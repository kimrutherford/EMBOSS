/****************************************************************************
** 
** @source embsig.h
**
** Data structures and algorithms for use with sparse sequence signatures.
** Hit, Hitlist, Sigpos, Sigdat and Signature objects.
** 
** Copyright (c) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
** @version 1.0 
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
**
****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embsig_h
#define embsig_h





/* @data EmbPSigpos **********************************************************
**
** Nucleus Sigpos object.
**
** Holds data for compiled signature position
**
** EmbPSigpos is implemented as a pointer to a C data structure.
**
** @alias EmbSSigpos
** @alias EmbOSigpos
**
**
** 

** @attr  gsiz    [ajuint*] Gap sizes 
** @attr  gpen    [float*]  Gap penalties 
** @attr  subs    [float*]  Residue match values 
** @attr  ngaps   [ajuint]  No. of gaps 
** @attr  Padding [char[4]] Padding to alignment boundary
** 
** @new embSigposNew Default Sigdat object constructor
** @delete embSigposDel Default Sigdat object destructor
** @@
****************************************************************************/
typedef struct EmbSSigpos
{
    ajuint   *gsiz;       
    float   *gpen;       
    float   *subs;       
    ajuint  ngaps;      
    char    Padding[4];
} EmbOSigpos;
#define EmbPSigpos EmbOSigpos*





/* @data EmbPSigdat **********************************************************
**
** Nucleus Sigdat object.
**
** Holds empirical data for an (uncompiled) signature position. 
** Important: Functions which manipulate this structure rely on the data in 
** the gap arrays (gsiz and grfq) being filled in order of increasing gap 
** size.
**
** EmbPSigdat is implemented as a pointer to a C data structure.
**
** @alias EmbSSigdat
** @alias EmbOSigdat
**
**
**
** @attr  rids [AjPChar]   Residue id's 
** @attr  rfrq [AjPUint]   Residue frequencies 
** 
** @attr  nres [ajuint]    No. diff. types of residue
** @attr  nenv [ajuint]    No. diff. types of environment
** @attr  eids [AjPStr*]   Environment id's
** @attr  efrq [AjPUint]   Environment frequencies 
**
** @attr  gsiz [AjPUint]   Gap sizes 
** @attr  gfrq [AjPUint]   Frequencies of gaps of each size
** @attr  ngap [ajuint]    No. diff. sizes of empirical gap
** @attr  wsiz [ajuint]    Window size for this gap 
**
** @new embSigdatNew Default Sigdat object constructor
** @delete embSigdatDel Default Sigdat object destructor
** @@
****************************************************************************/
typedef struct EmbSSigdat
{

    AjPChar      rids;
    AjPUint      rfrq;         
    ajuint       nres;
    ajuint       nenv;         
    AjPStr      *eids;
    AjPUint      efrq;         


    AjPUint      gsiz;         
    AjPUint      gfrq;         

    ajuint       ngap;         
    ajuint       wsiz;         
} EmbOSigdat;
#define EmbPSigdat EmbOSigdat*





/* @data EmbPSignature *******************************************************
**
** Nucleus Signature object.
**
** EmbPSignature is implemented as a pointer to a C data structure.
**
** @alias EmbSSignature
** @alias EmbOSignature
**
** 
**
** @attr  Type         [ajuint]       Type, either ajSCOP (1) or ajCATH (2)
** for domain signatures, or ajLIGAND (3) for ligand signatures.
** @attr  Typesig      [ajuint]       Type, either aj1D (1) or aj3D (2)
** for sequence or structure-based signatures respectively. 
** @attr  Class        [AjPStr]      SCOP classification.
** @attr  Architecture [AjPStr]      CATH classification.
** @attr  Topology     [AjPStr]      CATH classification.
** @attr  Fold         [AjPStr]      SCOP classification.
** @attr  Superfamily  [AjPStr]      SCOP classification.
** @attr  Family       [AjPStr]      SCOP classification.
** @attr  Sunid_Family [ajuint]       SCOP sunid for family. 
** @attr  npos         [ajuint]       No. of signature positions.
** @attr  pos          [EmbPSigpos*]  Array of derived data for puropses of 
**                                   alignment.
** @attr  dat          [EmbPSigdat*]  Array of empirical data.
**
** @attr  Id    [AjPStr]   Protein id code. 
** @attr  Domid [AjPStr]   Domain id code. 
** @attr  Ligid [AjPStr]   Ligand id code. 
** @attr  Desc  [AjPStr]   Description of ligand (ajLIGAND only)
** @attr  ns    [ajuint]    No. of sites (ajLIGAND only)
** @attr  sn    [ajuint]    Site number (ajLIGAND only)
** @attr  np    [ajuint]    No. of patches (ajLIGAND only)
** @attr  pn    [ajuint]    Patch number (ajLIGAND only)
** @attr  minpatch  [ajuint]   Max. patch size (residues) (ajLIGAND only)
** @attr  maxgap   [ajuint]    Min. gap distance (residues) (ajLIGAND only)
** @new    embSignatureNew Default Signature constructor
** @delete embSignatureDel Default Signature destructor
** @output embSignatureWrite Write signature to file.
** @input  embSignatureReadNew Construct a Signature object from reading a 
**         file in embl-like format (see documentation for the DOMAINATRIX
**         "sigscan" application).
** @output embSignatureWrite Write a Signature object to a file in embl-like 
**         format (see documentation for the DOMAINATRIX "sigscan" 
**         application).
** @input  embSignatureHitsRead Construct a Hitlist object from reading a 
**         signature hits file (see documentation for the DOMAINATRIX 
**         "sigscan" application). 
** @output embSignatureHitsWrite Writes a list of Hit objects to a 
**         signature hits file (see documentation for the DOMAINATRIX 
**         "sigscan" application). 
** @modify embSignatureCompile Compiles a Signature object.  The signature 
**         must first have been allocated by using the embSignatureNew 
**         function.
** @use    embSignatureAlignSeq Performs an alignment of a signature to a 
**         protein sequence. The signature must have first been compiled by 
**         calling embSignatureCompile.  Write a Hit object with the result.
** @use    embSignatureAlignSeqall Performs an alignment of a signature to
**         protein sequences. The signature must have first been compiled by 
**         calling embSignatureCompile.  Write a list of Hit objects with 
**         the result.
** @@
****************************************************************************/

typedef struct EmbSSignature
{
    ajuint      Type;
    ajuint      Typesig;
    AjPStr      Class;
    AjPStr      Architecture;
    AjPStr      Topology;
    AjPStr      Fold;
    AjPStr      Superfamily;
    AjPStr      Family;
    ajuint      Sunid_Family; 
    ajuint      npos;       
    EmbPSigpos *pos;        
    EmbPSigdat *dat;        

    AjPStr    Id;     
    AjPStr    Domid;     
    AjPStr    Ligid;     
    AjPStr    Desc;
    ajuint    ns;
    ajuint    sn;
    ajuint    np;
    ajuint    pn;
    ajuint    minpatch;
    ajuint    maxgap;
} EmbOSignature;
#define EmbPSignature EmbOSignature*






/* @data EmbPHit *************************************************************
**
** Nucleus hit object.
**
** Holds data associated with a protein / domain sequence that is generated 
** and or manipulated by the EMBOSS applications seqsearch, seqsort, and 
** sigscan.
**
** EmbPHit is implemented as a pointer to a C data structure.
**
** @alias EmbSHit
** @alias EmbOHit
**
**
**
** @attr  Seq	   [AjPStr]  Sequence as string.
** @attr  Start    [ajuint]   Start of sequence or signature alignment relative
**	           	     to full length swissprot sequence, this is an 
**		             index so starts at 0. 
** @attr  End      [ajuint]   End of sequence or signature alignment relative
**		             to full length swissprot sequence, this is an
**         		     index so starts at 0. 
** @attr  Acc      [AjPStr]  Accession number of sequence entry.  
** @attr  Spr      [AjPStr]  Swissprot code of sequence entry. 
** @attr  Dom      [AjPStr]  SCOP or CATH database identifier code of entry. 
** @attr  Rank     [ajuint]   Rank order of hit 	
** @attr  Score    [float]   Score of hit 
** @attr  Eval     [float]   E-value of hit 
** @attr  Pval     [float]   p-value of hit 
**  
** @attr  Typeobj  [AjPStr]  Primary (objective) classification of hit.
** @attr  Typesbj  [AjPStr]  Secondary (subjective) classification of hit 
** @attr  Model    [AjPStr]  String for model type if used, one of 
**  PSIBLAST, HMMER, SAM, SPARSE, HENIKOFF or GRIBSKOV
**
** @attr  Alg      [AjPStr]  Alignment, e.g. of a signature to the sequence 
** @attr  Group    [AjPStr]  Grouping of hit, e.g. 'REDUNDANT' or 
**                           'NON_REDUNDANT' 
** @attr  Target   [AjBool]  Used for garbage collection.
** @attr  Target2  [AjBool]  Also used for garbage collection.
** @attr  Sig      [EmbPSignature] Pointer to signature object for which hit
** @attr  Priority [AjBool]  Also used for garbage collection.
** @attr  Padding  [char[4]]  Padding to alignment boundary
** was generated. Used as a pointer only - memory is never freed or allocated
** to it.
**
**
** 
** @new    embHitNew Default Hit constructor
** @new    embHitReadFasta  Construct Hit object from reading the next entry
**         from a file in extended FASTA format (see documentation for the 
**         DOMAINATRIX "seqsearch" application). 
** @delete embHitDel Default Hit destructor
** @assign embHitMerge Create new Hit from merging two Hit objects
** @use    embMatchScore Sort Hit objects by Score element.
** @use    embMatchinvScore Sort (inverted order) Hit objects by Score 
**         element.
** @use    embMatchLigid Sort Hit objects by Ligid element in Sig element.
** @use    embMatch Sort Hit objects by Ligid element in Sig element.

** @use    embHitsOverlap Checks for overlap between two Hit objects.
** 
** @@
****************************************************************************/

typedef struct EmbSHit
{
  AjPStr  Seq;	
  ajuint  Start;      
  ajuint  End;        
  AjPStr  Acc;           
  AjPStr  Spr;        
  AjPStr  Dom;        
  ajuint  Rank;       
  float   Score;      
  float  Eval;       
  float  Pval;       

  AjPStr  Typeobj;    
  AjPStr  Typesbj;    
  AjPStr  Model;      
  AjPStr  Alg;        
  AjPStr  Group;      
  AjBool  Target;     
  AjBool  Target2;    

  EmbPSignature Sig;
  AjBool  Priority;
  char    Padding[4];
} EmbOHit;
#define EmbPHit EmbOHit*






/* @data EmbPHitlist *********************************************************
**
** Nucleus hitlist object.
**
** Holds an array of hit structures and associated SCOP classification 
** records.
**
** EmbPHitlist is implemented as a pointer to a C data structure.
**
** @alias EmbSHitlist
** @alias EmbOHitlist
**
** 
**
** @attr  Class         [AjPStr]    SCOP classification.
** @attr  Architecture  [AjPStr]    CATH classification.
** @attr  Topology      [AjPStr]    CATH classification.
** @attr  Fold          [AjPStr]    SCOP classification.
** @attr  Superfamily   [AjPStr]    SCOP classification.
** @attr  Family        [AjPStr]    SCOP classification.
** @attr  Model         [AjPStr]    SCOP classification.
** @attr  Sunid_Family  [ajuint]     SCOP sunid for family. 
** @attr  Priority      [AjBool]    True if the Hitlist is high priority. 

** @attr  hits          [EmbPHit*]  Array of hits. 
** @attr  Type          [ajuint]     Domain type, either ajSCOP (1) or
**                                  ajCATH (2).
** @attr  N             [ajuint]    No. of hits. 
**
** @new    embHitlistNew Default Hitlist constructor
** @delete embHitlistDel Default Hitlist destructor
** @use    embHitlistMatchFold Sort Hitlist objects by Fold element
** @input  embHitlistRead Construct Hitlist object from reading the next entry
**         from a file in embl-like format (see documentation for the 
**         DOMAINATRIX "seqsearch" application). 
** @new    embHitlistReadFasta Construct Hitlist object from reading
**         the next entry
**         from a file in extended FASTA format (see documentation for the 
**         DOMAINATRIX "seqsearch" application). 
** @input  embHitlistReadNode Construct Hitlist object from reading a specific
**         entry from a file in embl-like format (see documentation for the 
**         DOMAINATRIX "seqsearch" application). 
** @new    embHitlistReadNodeFasta Construct Hitlist object from reading
**         a specific entry from a file in extended FASTA format
**         (see documentation for the DOMAINATRIX "seqsearch" application). 
** @output embHitlistWrite Write Hitlist to file in embl-like format (see 
**         documentation for the DOMAINATRIX "seqsearch" application). 
** @output embHitlistWriteSubset Write a subset of a Hitlist to file in 
**         embl-like format (see documentation for the DOMAINATRIX "seqsearch"
**         application). 
** @output embHitlistWriteFasta Write Hitlist to file in extended FASTA format 
**         (see documentation for the DOMAINATRIX "seqsearch" application). 
** @output embHitlistWriteSubsetFasta Write a subset of a Hitlist to file in 
**         extended FASTA format (see documentation for the DOMAINATRIX
**         "seqsearch" application). 
** @output embHitlistWriteHitFasta Write a single Hit from a Hitlist to file 
**         in extended FASTA format (see documentation for the DOMAINATRIX 
**         "seqsearch" application). 
** @use    embHitlistClassify Classifies a list of signature-sequence hits 
**         (held in a Hitlist object) according to list of target sequences 
**         (a list of Hitlist objects).
** @@
****************************************************************************/

typedef struct EmbSHitlist
{
    AjPStr   Class;
    AjPStr   Architecture;
    AjPStr   Topology;
    AjPStr   Fold;
    AjPStr   Superfamily;
    AjPStr   Family;
    AjPStr   Model;
    ajuint   Sunid_Family;
    AjBool   Priority;     
    EmbPHit *hits;         
    ajuint   Type;
    ajuint   N;            
} EmbOHitlist;
#define EmbPHitlist EmbOHitlist*





/*
** Prototype definitions
*/


/* ======================================================================= */
/* =========================== Sigdat object ============================= */
/* ======================================================================= */
EmbPSigdat   embSigdatNew(ajuint nres, ajuint ngap);
void         embSigdatDel(EmbPSigdat *pthis);




/* ======================================================================= */
/* =========================== Sigpos object ============================= */
/* ======================================================================= */
EmbPSigpos   embSigposNew(ajuint ngap);
void         embSigposDel(EmbPSigpos *thys);




/* ======================================================================= */
/* ========================== Signature object =========================== */
/* ======================================================================= */
EmbPSignature embSignatureNew(ajuint n);
void          embSignatureDel(EmbPSignature *ptr);
EmbPSignature embSignatureReadNew(AjPFile inf);
AjBool        embSignatureWrite(AjPFile outf, const EmbPSignature obj);
AjBool        embSignatureCompile(EmbPSignature *S, float gapo, float gape,
				  const AjPMatrixf matrix);
AjBool        embSignatureAlignSeq(const EmbPSignature S, const AjPSeq seq,
				   EmbPHit *hit, 
				   ajuint nterm);
AjBool        embSignatureAlignSeqall(const EmbPSignature sig, AjPSeqall db, 
				      ajuint n, EmbPHitlist *hitlist, 
				      ajuint nterm);
AjBool        embSignatureHitsWrite(AjPFile outf, const EmbPSignature sig, 
				    const EmbPHitlist hitlist, ajuint n);
EmbPHitlist   embSignatureHitsRead(AjPFile inf);





/* ======================================================================= */
/* ============================= Hit object ============================== */
/* ======================================================================= */
EmbPHit       embHitNew(void);

EmbPHit       embHitReadFasta(AjPFile inf);

void          embHitDel(EmbPHit *ptr);

EmbPHit       embHitMerge(const EmbPHit hit1, 
			  const EmbPHit hit2);

AjBool        embHitsOverlap(const EmbPHit hit1, 
			     const EmbPHit hit2, 
			     ajuint n);

ajint         embMatchScore(const void *hit1, 
			    const void *hit2);

ajint         embMatchinvScore(const void *hit1, 
			       const void *hit2);

ajint         embMatchLigid(const void *hit1, 
			    const void *hit2);

ajint         embMatchSN(const void *hit1, 
			 const void *hit2);


/* ======================================================================= */
/* =========================== Hitlist object ============================ */
/* ======================================================================= */

EmbPHitlist   embHitlistNew(ajuint n);

void          embHitlistDel(EmbPHitlist *ptr);

EmbPHitlist   embHitlistRead(AjPFile inf);

EmbPHitlist   embHitlistReadFasta(AjPFile inf);

AjBool        embHitlistWrite(AjPFile outf, 
			      const EmbPHitlist obj);

AjBool        embHitlistWriteSubset(AjPFile outf, 
				    const EmbPHitlist obj, 
				    const AjPUint ok);

AjBool        embHitlistWriteFasta(AjPFile outf, 
				   const EmbPHitlist obj);

AjBool        embHitlistWriteSubsetFasta(AjPFile outf, 
					 const EmbPHitlist obj, 
					 const AjPUint ok);

AjBool        embHitlistWriteHitFasta(AjPFile outf, 
				      ajuint n, 
				      const EmbPHitlist obj);

AjPList       embHitlistReadNode(AjPFile inf, 
				 const AjPStr fam, 
				 const AjPStr sfam, 	
				 const AjPStr fold, 
				 const AjPStr klass);

AjPList       embHitlistReadNodeFasta(AjPFile inf, 
				      const AjPStr fam, 
				      const AjPStr sfam, 
				      const AjPStr fold, 
				      const AjPStr klass);

AjBool        embHitlistClassify(EmbPHitlist hits, 
				 const AjPList targets, 
				 ajuint thresh);

ajint         embHitlistMatchFold(const void *hit1, 
				  const void *hit2);



/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif














/****************************************************************************
**
** @source ajpdbio.c
**
** Data structures and functions for reading and writing PDB format files.
** Includes functions for writing a Pdb object (defined in ajpdb.h).
** 
** Copyright (c) 2004 Jon Ison
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
**
** Notes
** Look for JONNEW for latest edits.
**
****************************************************************************/

/* ======================================================================= */
/* ============================ include files ============================ */
/* ======================================================================= */

#include "ajax.h"
#include <math.h>




/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */




/* @datastatic AjPElement *****************************************************
**
** Nucleus Element object.
**
** Object for holding a single secondary structure element when parsing PDB.
**
** AjPElement is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSElement
** @alias AjOElement
**
**
**
** @attr elementId [AjPStr]    Element identifier (columns 12 - 14) 
**                             SHEET ('E') or TURN ('T') 
** @attr initResName [AjPStr]  Name of first residue in each element (columns 
**	    	               16 - 18 (HELIX & TURN) or 18 - 20 (SHEET) ) 
** @attr initSeqNum [AjPStr]   Residue number (including insertion code) 
**		               of first residue in each element
**                             (columns 22 - 26 (HELIX), 23 - 27 (SHEET)
**                             or 21 - 25 (TURN) )
** @attr endResName [AjPStr]   Name of last residue in each element
**                             (columns 28 - 30 (HELIX), 29 - 31 (SHEET)
**                             or  27 - 29 (TURN) ) 
** @attr endSeqNum [AjPStr]    Residue number (including insertion code) of 
**		               last residue in each element
**                             (columns 34 - 38 (HELIX and SHEET)
**                             or 32 - 36 (TURN)  )
** @attr helixClass [ajint]    Classes of helices (columns 39 - 40),
**                             an int from 1-10 from
** @attr elementNum [ajint]    Serial number of the element (columns 8 - 10)
** @attr elementType [char]    Element type COIL ('C'), HELIX ('H'),
** @attr chainId [char]        Chain identifiers for chains containing the
**                             elements (column 20 (HELIX & TURN)
**                             or 22 (SHEET) )
** @attr Padding [char[6]]     Padding to alignment boundary
**
** http://www.rcsb.org/pdb/docs/format/pdbguide2.2/guide2.2_frame.html<br>
** 
** TYPE OF HELIX             CLASS NUMBER (COLUMNS 39 - 40)<br>
** --------------------------------------------------------------<br>
** Right-handed alpha (default)                1<br>
** Right-handed omega                          2<br>
** Right-handed pi                             3<br>
** Right-handed gamma                          4<br>
** Right-handed 310                            5<br>
** Left-handed alpha                           6<br>
** Left-handed omega                           7<br>
** Left-handed gamma                           8<br>
** 27 ribbon/helix                             9<br>
** Polyproline                                10<br>
** 
** @@
****************************************************************************/
typedef struct AjSElement
{
    AjPStr   elementId;  
    AjPStr   initResName;     
				
    AjPStr   initSeqNum;    
    AjPStr   endResName;    
    AjPStr   endSeqNum;     
    ajint    helixClass;    
    ajint    elementNum; 

    char     elementType; 
    char     chainId;
    char     Padding[6];
}AjOElement;
#define AjPElement AjOElement*





/* @datastatic AjPElements ****************************************************
**
** Nucleus Elements object.
**
** Object for holding secondary structure elements from a PDB file
**
** AjPElements is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSElements
** @alias AjOElements
**
**
**
** @attr elms [AjPElement*] Secondary structure element array
** @attr n [ajint] Total no. of secondary structure elements
**                       (helices, strands or turns) 
** @attr Padding [char[4]] Padding to alignment boundary
** @@
****************************************************************************/
typedef struct AjSElements
{
    AjPElement *elms;        
    ajint      n;            
    char       Padding[4];
}
AjOElements;
#define AjPElements AjOElements*




/* @datastatic AjPPdbfile *****************************************************
**
** Nucleus Pdbfile object.
**
** Holds a pdb file for parsing.
**
** AjPPdbfile is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSPdbfile
** @alias AjOPdbfile
**
**
**
** @attr pdbid [AjPStr]       4 character pdb id code 
** @attr tercnt [ajint]       The number of TER records in the pdb file 
** @attr toofewter [AjBool]   True if the file contained too few TER records 
** @attr modcnt [ajint]       The number of MODEL records in the pdb file
**	                      (does not count duplicate MODEL records
**                            that are masked out)
** @attr nomod [AjBool]       True if the file contained no MODEL records 
** @attr compnd [AjPStr]      Text from COMPND records 
** @attr source [AjPStr]      Text from SOURCE records 
** @attr reso [float]         Resolution of structure 
** @attr method [ajint]       Structural method,  either ajXRAY or ajNMR 
** @attr gpid [AjPChar]       Array of chain (group) id's for groups that
**                            cannot be associated with a chain in the SEQRES
**                            section 
** @attr idxfirst [ajint]     Index in <lines> of first ATOM, HETATM or MODEL
**                            line 
**
** @attr nchains [ajint]      Number of chains (from SEQRES record)
**                            for sizes of following attribute arrays
** @attr seqres [AjPStr*]     Array of sequences taken from the SEQRES records
** @attr seqresful [AjPStr*]  Array of sequences using 3 letter codes taken
**                            from the SEQRES records
** @attr nres [ajint*]        Number of residues in each chain 
** @attr chainok [AjBool*]    Array of flags which are True if a chain in the
**                            SEQRES record contains >= minimum no. of amino
**                            acids and has a  unique chain identifier
** @attr resn1ok [AjBool*]    Bool's for each chain which are TRUE if resn1
**                            was used to derive resni, i.e. gave correct
**                            alignment to seqres sequence.
**                            If False then resn2 was used. 
** @attr nligands [ajint*]    Number of ligands for each chain.  A ligand is a
**                            non-protein group associated with a chain 
**                            in the SEQRES section.
** @attr numHelices [ajint*]  No. of helices in each chain 
** @attr numStrands [ajint*]  No. of strands in each chain 
** @attr numSheets [ajint*]   No. of sheets in each chain 
** @attr numTurns [ajint*]    No. of turns in each chain 
** @attr chid [AjPChar]       Array of chain id's for chains from SEQRES
**                            records
** 
** @attr lines [AjPStr*]      Array of lines in the pdb file 
** @attr linetype [ajint*]    Array of int's describing the lines, have values
**	                      of PDBPARSE_IGNORE (do not consider this line
**	                      when parsing coordinates from the file),
**                            PDBPARSE_COORD (coordinate line (ATOM or HETATM
**                            record) for protein atoms, PDBPARSE_COORDHET
**                            (coordinate line for non-protein atoms), 
**                            PDBPARSE_COORDGP (coordinate line for groups
**                            that could not be associated with a SEQRES
                              chain), PDBPARSE_COORDWAT (coordinate line for
**                            water), PDBPARSE_TER (it is a TER record) or
**                            PDBPARSE_MODEL (it is a MODEL record).
** @attr chnn [ajint*]        Array of chain numbers for each PDBPARSE_COORD & 
**                            PDBPARSE_COORDHET line.
** @attr gpn [ajint*]         Array of group numbers for each line. Each group 
**	                      (heterogen) is given a group number, that is
**                            either relative to a chain or the whole file
**                            (for groups that could  not be associated with
**                            a chain from the SEQRES records) 
** @attr modn [ajint*]        Array of model numbers for each PDBPARSE_COORD
**                            line 
**
** @attr resni [ajint*]       Residue numbers for each PDBPARSE_COORD line.
**	                      These give the correct index into the 'seqres'
**                            sequences 
** @attr resn1 [ajint*]       Array of residue numbers for each PDBPARSE_COORD
**                            line. This is pdbn converted to a sequential 
**	                      integer where alternative residue numbering is 
**	                      presumed for lines where line[26] is used
**	                      (residues for which oddnum==True are considered).
** @attr resn2 [ajint*]       Array of residue numbers for each PDBPARSE_COORD
**	                      line. This is pdbn converted to a sequential 
**	                      integer where heterogeneity is presumed for 
**	                      lines where line[26] is used (residues where
**                            oddnum==True are ignored).
** @attr pdbn [AjPStr*]       Array with a residue number for each line
**	                      for which 'coord' == ajTrue. This is the
**	                      original residue number string (including
**                            insertion code) from the pdb file
** @attr oddnum [AjBool*]     Bool's for each line which are TRUE for
**                            duplicate residues of heterogenous positions
**	                      (e.g. if 2 different residues are both numbered
**                            '8' or one is '8' and the other '8A'
**    	                      for example then <oddnum> would be set True for
**	                      the second residue. Heterogeneity is indicated 
**                            by a character in position lines[26] (the same 
**	                      position used to indicate alternative residue
**                            numbering schemes).    
** @attr atype [AjPStr*]      Atom type for each line 
** @attr rtype [AjPStr*]      Residue type for each line 
** @attr x [float*]           x-coordinate for each line 
** @attr y [float*]           y-coordinate for each line 
** @attr z [float*]           z-coordinate for each line 
** @attr o [float*]           occupancy for each line 
** @attr b [float*]           thermal factor for each line 
**
** @attr elementNum [ajint*]  Serial number of the secondary structure element 
**                            (columns 8 - 10) 
** @attr elementId [AjPStr*]  Secondary structure element identifier (columns
**                            12 - 14) 
** @attr elementType [char*]  Secondary structure element type COIL ('C'), 
**                            HELIX ('H'), SHEET ('E') or TURN ('T') 
** @attr helixClass [ajint*]   Classes of helices (columns 39 - 40)  from 
**	 http://www.rcsb.org/pdb/docs/format/pdbguide2.2/guide2.2_frame.html 
**                             (see below). Has a value of 0 (printed out as 
**                             '.') for non-helical elements.
** @attr nlines [ajint]       Number of lines in the pdb file and size of the
**                            following arrays
** @attr ngroups [ajint]      Number of groups (non-protein groups that
**                            could not be associated with a chain in the
**                            SEQRES section 
** @@
******************************************************************************/

typedef struct AjSPdbfile
{
    AjPStr    pdbid;    
    ajint     tercnt;   
    AjBool    toofewter;   
    ajint     modcnt;   
    AjBool    nomod;       
    AjPStr    compnd;      
    AjPStr    source;	   
    float     reso;	   
    ajint     method;      

    AjPChar   gpid;	       
    ajint     idxfirst;   


    ajint     nchains;    
    AjPStr    *seqres;      
    AjPStr    *seqresful;      
    ajint     *nres;            
    AjBool    *chainok;     
    AjBool    *resn1ok; 
    ajint     *nligands;      
    ajint    *numHelices;  
    ajint    *numStrands;  
    ajint    *numSheets;   
    ajint    *numTurns;    
    AjPChar    chid;	  


    AjPStr    *lines;    
    ajint     *linetype; 
    ajint     *chnn;
    ajint     *gpn;         
    ajint     *modn;    
    ajint     *resni;   
    ajint     *resn1;   
    ajint     *resn2;   
    AjPStr    *pdbn;    
    AjBool    *oddnum;  
    AjPStr    *atype;      
    AjPStr    *rtype;      
    float     *x;          
    float     *y;          
    float     *z;          
    float     *o;          
    float     *b;          
    ajint    *elementNum;  
    AjPStr   *elementId;   
    char     *elementType; 
    ajint    *helixClass;  
    ajint    nlines;
    ajint    ngroups;   
}AjOPdbfile;
#define AjPPdbfile AjOPdbfile*




#define POS_CHID         21   /* Position in ATOM line of chain id.        */
#define PDBPARSE_IGNORE   0   /* Ignore this line when parsing coordinates.*/  
#define PDBPARSE_COORD    1   /* Coordinate line.                          */
#define PDBPARSE_COORDHET 2   /* Coordinate line for non-protein atoms.    */
#define PDBPARSE_TER      3   /* TER record.                               */
#define PDBPARSE_MODEL    4   /* MODEL record.                             */ 
#define PDBPARSE_COORDGP  5   /* Coordinate line for groups that could not 
			         be associated with a SEQRES chain.        */
#define PDBPARSE_COORDWAT 6   /* Coordinate line for water.                */
#define PDBPARSE_ENDMDL   7   /* ENDMDL record.                            */ 

#define MAXMISSNTERM      3   /* A number of residues may be missing from the 
				 N-terminus of the SEQRES records relative to 
				 the ATOM records (e.g. MET and ACE often do 
				 not appear).  The parser will search and 
				 correct for such cases. MAXMISSNTERM is the 
				 maximum number of such missing residues that 
				 can be accounted for. */

/* Flags for printing in PDB format */
enum flags
{ 
    ajHEADER_DOMAIN = 1, ajSEQRES_DOMAIN, ajATOMPDB_DOMAIN, ajATOMIDX_DOMAIN, 
    ajSEQRES_CHAIN, ajATOMPDB_CHAIN, ajATOMIDX_CHAIN, ajHETEROGEN, 
    ajHEADER, ajTITLE, ajCOMPND, ajSOURCE, ajEMPTYREMARK, ajRESOLUTION 
};




/*THIS_DIAGNOSTIC */ 
  AjPStr  tempstr;    
  AjPFile tempfile;




/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */

/* These functions are for parsing PDB files */
/*THIS_DIAGNOSTIC   static void diagnostic(AjPPdbfile *pdbfile, ajint n); */
static void         PdbfileDel(AjPPdbfile *pthis);
static AjPPdbfile   ReadLines(AjPFile inf);
static AjPPdbfile   PdbfileNew(ajint nlines, ajint nchains);
static AjBool       FirstPass(AjPPdbfile pdbfile, AjPFile flog, 
			      AjPElements *elms, AjBool camask);
static AjBool       CheckChains(AjPPdbfile pdbfile, AjPFile flog, 
				ajint min_chain_size);
static AjBool       SeqresToSequence(const AjPStr seqres, AjPStr *seq, 
				     AjBool camask, ajint *len);
static AjBool       CheckTer(AjPPdbfile pdbfile, AjPFile flog);
static AjBool       NumberChains(AjPPdbfile pdbfile, AjPFile flog);
static AjBool       NoMoreAtoms(AjPPdbfile pdbfile, ajint linen);
static AjBool       MaskChains(AjPPdbfile pdbfile, AjPFile flog, 
			       ajint min_chain_size, AjBool camask, 
			       AjBool camask1, AjBool atommask);
static AjBool       StandardiseNumbering(AjPPdbfile pdbfile, 
					 AjPFile flog);
static AjBool       AlignNumbering(AjPPdbfile pdbfile, AjPFile flog, 
				   ajuint lim, ajuint lim2);
static AjBool       PdbfileToPdb(AjPPdb *ret, AjPPdbfile pdb);
static ajint        PdbfileFindLine(const AjPPdbfile pdb, ajint chn, 
				    ajint which, ajint pos);

/* Functions for Element object */
static AjPElements  ElementsNew(ajint nchains);
static void         ElementsDel(AjPElements *ptr);
static AjPElement   ElementNew(void);
static void         ElementDel(AjPElement *ptr);
static AjBool       PdbfileChain(char id, const AjPPdbfile pdb, ajint *chn);
static AjBool       WriteElementData(AjPPdbfile pdbfile, AjPFile flog, 
				     const AjPElements elms);


/* These functions are called by ajPdbWriteDomainRecordRaw */
static AjBool       WriteHeaderScop(AjPFile outf, const AjPScop scop);
static AjBool       WriteSeqresDomain(AjPFile errf, AjPFile outf, 
				      const AjPPdb pdb, const AjPScop scop);
static AjBool       WriteAtomDomainPdb(AjPFile errf, AjPFile outf, 
				       const AjPPdb pdb, const AjPScop scop,
				       ajint mod);
static AjBool       WriteAtomDomainIdx(AjPFile errf, AjPFile outf, 
				       const AjPPdb pdb, const AjPScop scop,
				       ajint mod);
static AjBool       WriteAtomDomain(AjPFile errf, AjPFile outf,
				    const AjPPdb pdb,
				    const AjPScop scop, ajint mod, ajint mode);


/* These functions are called by ajPdbWriteRecordRaw */
static AjBool       WriteSeqresChain(AjPFile errf, AjPFile outf,
				     const AjPPdb pdb, ajint chn);
static AjBool       WriteAtomChain(AjPFile outf, const AjPPdb pdb, ajint mod, 
				   ajint chn, ajint mode);
static AjBool       WriteHeterogen(AjPFile outf, const AjPPdb pdb, ajint mod);
static AjBool       WriteHeader(AjPFile outf, const AjPPdb pdb);
static AjBool       WriteTitle(AjPFile outf, const AjPPdb pdb);
static AjBool       WriteCompnd(AjPFile outf, const AjPPdb pdb);
static AjBool       WriteSource(AjPFile outf, const AjPPdb pdb);
static AjBool       WriteEmptyRemark(AjPFile outf, const AjPPdb pdb);
static AjBool       WriteResolution(AjPFile outf, const AjPPdb pdb);

/* Others */
static AjBool       WriteText(AjPFile outf, const AjPStr str,
			      const char *prefix);




/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */




/* @funcstatic WriteSeqresChain *********************************************
**
** Writes sequence for a protein chain to an output file in pdb format 
** (SEQRES records).  Sequence is taken from a Pdb structure.
**
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
** @param [r] chn  [ajint] chain number, beginning at 1
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteSeqresChain(AjPFile errf, AjPFile outf, const AjPPdb pdb, 
				ajint chn)
{
    ajint last_rn = 0;  
    ajint this_rn;
    ajint x;
    ajint y;
    ajint len;
    const char  *p;

    AjPStr   tmp1 = NULL;
    AjPStr   tmp2 = NULL;
    AjIList  iter = NULL;
    AjPAtom  atm  = NULL;

    tmp1 = ajStrNew();
    tmp2 = ajStrNew();


    iter=ajListIterNewread(pdb->Chains[chn-1]->Atoms);	


    /* Iterate through list of atoms */
    while((atm=(AjPAtom)ajListIterGet(iter)))
    {
	/*
	** Hard-coded to work on model 1
	** Break if a non-protein atom is found or model no. !=1
	** Continue / break if a non-protein atom is found or model no. !=1
	*/
	    if(atm->Mod!=1)
		break;

	    if(atm->Type!='P') 
		continue;

	/* If we are onto a new residue */
	this_rn = atm->Idx;
	if(this_rn!=last_rn)
	{
	    /* Assign sequence for residues missing from the linked list */
	    for(x=last_rn; x<this_rn-1; x++)
	    {	
		/* Check that position x is in range for the sequence */
		if(!ajResidueToTriplet(
                       ajStrGetCharPos(pdb->Chains[chn-1]->Seq, x),
                       &tmp2))
		{
		    ajWarn("Index out of range in WriteSeqresChain");
		    ajFmtPrintF(errf, "//\n%S\nERROR Index out "
				"of range in WriteSeqresChain\n", 
				pdb->Pdb);

		    ajStrDel(&tmp1);
		    ajStrDel(&tmp2);
		    ajListIterDel(&iter);	

		    return ajFalse;
		}
		
		else
		{
		    ajStrAppendS(&tmp1, tmp2);
		    ajStrAppendC(&tmp1, " ");
		}	
	    }

	    ajStrAppendS(&tmp1, atm->Id3);
	    ajStrAppendC(&tmp1, " ");

	    last_rn=this_rn;
	}
    }

    
    /* Assign sequence for residues missing from end of linked list */
    for(x=last_rn; x<pdb->Chains[chn-1]->Nres; x++)
	if(!ajResidueToTriplet(
               ajStrGetCharPos(pdb->Chains[chn-1]->Seq, x),
               &tmp2))
	    { 
		ajStrDel(&tmp1);
		ajStrDel(&tmp2);
		ajListIterDel(&iter);	
		ajWarn("Index out of range in WriteSeqresChain");
		ajFmtPrintF(errf, "//\n%S\nERROR Index out of range "
			    "in WriteSeqresChain\n", pdb->Pdb);

		return ajFalse;
	    }
    
	else
	{
	    ajStrAppendS(&tmp1, tmp2);
	    ajStrAppendC(&tmp1, " ");
	}	

    
    /* Print out SEQRES records */
    for(p=ajStrGetPtr(tmp1), len=ajStrGetLen(tmp1), x=0, y=1; 
	x<len; 
	x+=52, y++, p+=52)
	ajFmtPrintF(outf, "SEQRES%4d %c%5d  %-61.52s\n", 
		    y, 
		    pdb->Chains[chn-1]->Id, 
		    pdb->Chains[chn-1]->Nres, 
		    p);


    /* Tidy up */
    ajStrDel(&tmp1);
    ajStrDel(&tmp2);
    ajListIterDel(&iter);	

    return ajTrue;
}




/* @funcstatic WriteSeqresDomain ********************************************
**
** Writes sequence for a SCOP domain to an output file in pdb format (SEQRES 
** records). Sequence is taken from a Pdb structure, domain definition is 
** taken from a Scop structure.  Where coordinates for multiple models (e.g. 
** NMR structures) are given, data for model 1 are written.
**
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
** @param [r] scop [const AjPScop] Scop object
**
** @return [AjBool] True on success
** @@
****************************************************************************/
static AjBool WriteSeqresDomain(AjPFile errf, AjPFile outf, const AjPPdb pdb, 
				 const AjPScop scop)
{
    ajint last_rn = 0;  
    ajint this_rn;
    ajint x;
    ajint y;
    ajint z;
    ajint rcnt = 0;
    ajint len;
    ajint chn  = -1;
    const char  *p;
    char  id;

    AjPStr      tmp1        = NULL;
    AjPStr      tmp2        = NULL;
    AjBool      found_start = ajFalse;
    AjBool      found_end   = ajFalse;
    AjBool      nostart     = ajFalse;
    AjBool      noend       = ajFalse;
    AjIList     iter        = NULL;
    AjPAtom     atm         = NULL;
    AjPStr      tmpstr      = NULL;
/*    AjPResidue *resarr      = NULL; */
    

    /* Allocate strings etc */
    tmp1   = ajStrNew();
    tmp2   = ajStrNew();
    tmpstr = ajStrNew();

    

    /* Loop for each chain in the domain */
    for(z=0;z<scop->N;z++,found_start=ajFalse, found_end=ajFalse, 
	last_rn=0)
    {
	/* Check for error in chain id */
	if(!ajPdbChnidToNum(scop->Chain[z], pdb, &chn))
	{
	    ajListIterDel(&iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajStrDel(&tmpstr);

	    ajWarn("Chain incompatibility error in "
		   "WriteSeqresDomain");		

	    ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
			"error in WriteSeqresDomain\n", 
			scop->Entry);

	    return ajFalse;
	}

/*	if(resarr)
	    AJFREE(resarr);
	ajListToarray(pdb->Chains[chn-1]->Residues, (void ***) &resarr);  */


	/* Initialise iterator for list of atoms */
	iter=ajListIterNewread(pdb->Chains[chn-1]->Atoms);	
	

	/* Start of chain not specified */
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else
	    nostart = ajFalse;

	
	/* End of chain not specified */
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else	
	    noend = ajFalse;
	

	/* Iterate through list of atoms */
	while((atm=(AjPAtom)ajListIterGet(iter)))
	{
	    /* Hard-coded to work on model 1 */	
	    /*
	    ** Continue / break if a non-protein atom is found or
	    ** model no. !=1
	    */
	    if(atm->Mod!=1)
		break;

	    if(atm->Type!='P') 
		continue;
	
	    
	    /* If there is a new residue */
	    this_rn = atm->Idx;

	    if(this_rn!=last_rn)
	    {	
		/*
		** The start position was specified, but has not 
		** been found yet
		*/
		if(!found_start && !nostart)
		{
		    ajStrAssignS(&tmpstr, scop->Start[z]);
		    ajStrAppendK(&tmpstr, '*');
		    

		    /* Start position found */
		    /* if(!ajStrCmpCaseS(atm->Pdb, scop->Start[z])) */
		    if(ajStrMatchWildS(atm->Pdb, tmpstr))		     
		    /* if(ajStrMatchWildS(resarr[atm->Idx-1]->Pdb, tmpstr)) */
		    {
			if(!ajStrMatchS(atm->Pdb, scop->Start[z]))
			/* if(!ajStrMatchS(resarr[atm->Idx-1]->Pdb,
			   scop->Start[z])) */
			{
			    ajWarn("Domain start found by wildcard match "
				   "only in WriteSeqresDomain");
			    ajFmtPrintF(errf, "//\n%S\nERROR Domain start "
					"found "
					"by wildcard match only in "
					"WriteSeqresDomain\n",
					scop->Entry);
			}	

			last_rn = this_rn;
			found_start = ajTrue;	
		    }
		    else	
		    {
			last_rn = this_rn;
			continue;
		    }
		    
		}
	    

		/*
		** Assign sequence for residues missing from the linked list
		** of atoms of known structure
		*/
		for(x=last_rn; x<this_rn-1; x++)
		{	
		    /* Check that position x is in range for the sequence */
		    if(!ajResidueToTriplet(
                           ajStrGetCharPos(pdb->Chains[chn-1]->Seq, x), 
                           &tmp2))
		    {
			ajListIterDel(&iter);			
			ajStrDel(&tmp1);
			ajStrDel(&tmp2);
			ajStrDel(&tmpstr);

			ajWarn("Index out of range in "
			       "WriteSeqresDomain");		
			ajFmtPrintF(errf, "//\n%S\nERROR Index out of range "
				    "in WriteSeqresDomain\n",
				    scop->Entry);

			return ajFalse;
		    }
		    else
		    {	
			ajStrAppendS(&tmp1, tmp2);
			ajStrAppendC(&tmp1, " ");
			rcnt++;
		    }	
		}

		last_rn = this_rn;

		
		/* Append the residue to the sequence */
		ajStrAppendS(&tmp1, atm->Id3);
		ajStrAppendC(&tmp1, " ");
		rcnt++;
		

		/* The end position was specified, but has not 
		       been found yet */
		if(!found_end && !noend)
		{
		    ajStrAssignS(&tmpstr, scop->End[z]);
		    ajStrAppendK(&tmpstr, '*');
		    

		    /* End found */
		    /* if(!ajStrCmpCaseS(atm->Pdb, scop->End[z])) */
		    if(ajStrMatchWildS(atm->Pdb, tmpstr))
			/* if(ajStrMatchWildS(resarr[atm->Idx-1]->Pdb,
			   tmpstr)) */
		    {
			 if(!ajStrMatchS(atm->Pdb, scop->End[z])) 
			 /* if(!ajStrMatchS(resarr[atm->Idx-1]->Pdb,
			    scop->End[z])) */
			{
			    ajWarn("Domain end found by wildcard match only "
				   "in WriteSeqresDomain");
			    ajFmtPrintF(errf, "//\n%S\nERROR Domain end found "
					"by wildcard match only in "
					"WriteSeqresDomain\n",
					scop->Entry);
			}


                         found_end = ajTrue;       
			break;
		    }
		}	
	    }
	}
	
	
	/* Domain start specified but not found */
	if(!found_start && !nostart)
	{
	    ajListIterDel(&iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajStrDel(&tmpstr);

	    ajWarn("Domain start not found in WriteSeqresDomain");
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain start not found "
			"in WriteSeqresDomain\n", scop->Entry);

	    return ajFalse;
	}
	

	/* Domain end specified but not found */
	if(!found_end && !noend)
	{
	    ajListIterDel(&iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajStrDel(&tmpstr);

	    ajWarn("Domain end not found in WriteSeqresDomain");
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain end not found "
			"in WriteSeqresDomain\n", scop->Entry);

	    return ajFalse;
	}


	/*
	** Assign sequence for residues missing from end of linked list
	** Only needs to be done where the end of the domain is not specified
	*/
	if(noend)
	{	    
	    for(x=last_rn; x<pdb->Chains[chn-1]->Nres; x++)    
		if(!ajResidueToTriplet(
                       ajStrGetCharPos(pdb->Chains[chn-1]->Seq, x),
                       &tmp2))
		{	 
		    ajStrDel(&tmp1);
		    ajStrDel(&tmp2);
		    ajStrDel(&tmpstr);

		    ajListIterDel(&iter);	
    		    ajWarn("Index out of range in WriteSeqresDomain");
		    ajFmtPrintF(errf, "//\n%S\nERROR Index out of "
				"range in WriteSeqresDomain\n", 
				scop->Entry);

		    return ajFalse;
		}
		else
		{
		    ajStrAppendS(&tmp1, tmp2);
		    ajStrAppendC(&tmp1, " ");
		    rcnt++;
		}	
	}

	ajListIterDel(&iter);	 		
    }
    

    /*
    ** If the domain was composed of more than once chain then a '.' is
    ** given as the chain identifier
    */
    if(scop->N > 1)
	id = '.';
    else 
	id = pdb->Chains[chn-1]->Id;
       
 
    /* Print out SEQRES records */
    for(p=ajStrGetPtr(tmp1), len=ajStrGetLen(tmp1), x=0, y=1; 
	x<len; 
	x+=52, y++, p+=52)
	ajFmtPrintF(outf, "SEQRES%4d %c%5d  %-61.52s\n", 
		    y, 
		    id, 
		    rcnt,
		    p);

/*    if(resarr)
	AJFREE(resarr); */
    ajStrDel(&tmp1);
    ajStrDel(&tmp2);
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @funcstatic WriteAtomChain ***********************************************
**
** Writes coordinates for a protein chain to an output file in pdb format 
** (ATOM records). Coordinates are taken from a Pdb structure. The model 
** number argument should have a value of 1 for x-ray structures.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] mod  [ajint]   Model number, beginning at 1
** @param [r] chn  [ajint]   Chain number, beginning at 1
** @param [r] mode [ajint]   Either ajPDB or ajIDX if the original or 
**                           corrected residue number is to be used. 
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteAtomChain(AjPFile outf, const AjPPdb pdb,
			     ajint mod, ajint chn, 
			     ajint mode)
{
    AjBool  doneter = ajFalse;
    AjIList iter    = NULL;
    AjPAtom atm     = NULL;
    AjPAtom atm2    = NULL;
    ajint acnt;
/*    AjPResidue *resarr      = NULL; */


    /* Check args are not NULL */
    if(!outf || !pdb || mod<1 || chn<1)
	return ajFalse;
    

/*    ajListToarray(pdb->Chains[chn-1]->Residues, (void ***) &resarr);  */

    doneter = ajFalse;
    iter = ajListIterNewread(pdb->Chains[chn-1]->Atoms);	

    while((atm=(AjPAtom)ajListIterGet(iter)))
	if(atm->Mod==mod)
	    break;
  
    for(acnt=1; atm; atm=(AjPAtom)ajListIterGet(iter)) 	
    {
	/* Break if on a new model */
	if(atm->Mod!=mod)
	    break;
		
	
	/* End of protein atoms - so write a TER record */
	if(atm->Type!='P' && (!doneter))
	{
	    if(mode == ajIDX)
	    {
		ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
			    "TER", 
			    acnt++, 
			    atm2->Id3, 
			    pdb->Chains[chn-1]->Id, 
			    atm2->Idx, 
			    " ");
	    }
	    else if(mode == ajPDB)
	    {
		ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4S%54s\n", 
			    "TER", 
			    acnt++, 
			    atm2->Id3, 
			    pdb->Chains[chn-1]->Id, 
			    atm2->Pdb, 
			    /* resarr[atm2->Idx-1]->Pdb,  */
			    " ");
	    }
	    else
		ajFatal("Invalid mode in WriteAtomChain");

	    doneter = ajTrue;
	}

	
	/* Write out ATOM or HETATM line */
	if(atm->Type=='P')
	    ajFmtPrintF(outf, "%-6s", "ATOM");
	else
	    ajFmtPrintF(outf, "%-6s", "HETATM");

	if(mode == ajIDX)
	{
	    ajFmtPrintF(outf, "%5d  %-4S%-4S%c%4d%12.3f%8.3f%8.3f"
			"%6.2f%6.2f%11s%-3c\n", 
			acnt++, 
			atm->Atm, 
			atm->Id3, 
			pdb->Chains[chn-1]->Id, 
			atm->Idx, 
			atm->X, 
			atm->Y, 
			atm->Z, 
			atm->O,
			atm->B,
			" ", 
			*ajStrGetPtr(atm->Atm));
	}
	else if(mode == ajPDB)
	{
	    ajFmtPrintF(outf, "%5d  %-4S%-4S%c%4S%12.3f%8.3f%8.3f"
			"%6.2f%6.2f%11s%-3c\n", 
			acnt++, 
			atm->Atm, 
			atm->Id3, 
			pdb->Chains[chn-1]->Id, 
			atm->Pdb, 
			/* resarr[atm->Idx-1]->Pdb,  */
			atm->X, 
			atm->Y, 
			atm->Z, 
			atm->O,
			atm->B,
			" ", 
			*ajStrGetPtr(atm->Atm));
	}
	else
	    ajFatal("Invalid mode in WriteAtomChain");
	atm2 = atm;
    }

    
    /* Write TER record if its not already done */
    if(!doneter)
    {
	ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
		    "TER", 
		    acnt++, 
		    atm2->Id3, 
		    pdb->Chains[chn-1]->Id, 
		    atm2->Idx, 
		    " ");
	doneter=ajTrue;
    }
    ajListIterDel(&iter);				    


    
/*    if(resarr)
	AJFREE(resarr); */
    return ajTrue;
}




/* @funcstatic WriteAtomDomain ************************************************
**
** Writes coordinates for a SCOP domain to an output file in pdb format (ATOM 
** records).  Coordinates are taken from a Pdb structure, domain definition is 
** taken from a Scop structure. The model number argument should have a value
** of 1 for x-ray structures. Coordinates for heterogens are NOT written to 
** file.  The corrected residue numbers are given (these give an index into 
** the SEQRES sequence.  
**
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
** @param [r] scop [const AjPScop] Scop object
** @param [r] mod  [ajint] Model number, beginning at 1
** @param [r] mode [ajint] Either ajPDB or ajIDX if the original or corrected
**                         residue number is to be used. 
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteAtomDomain(AjPFile errf, AjPFile outf, const AjPPdb pdb, 
			      const AjPScop scop, ajint mod, ajint mode)
{
    /*
    ** rn_mod is a modifier to the residue number to give correct residue 
    ** numbering for the domain
    */
    ajint acnt    = 1;
    ajint rn_mod  = 0;  
    ajint z;
    ajint finalrn = 0;
    ajint chn;
    char  id      = '\0';

    AjBool   found_start = ajFalse;
    AjBool   found_end   = ajFalse;
    AjBool   nostart     = ajFalse;
    AjBool   noend       = ajFalse;
    AjIList  iter        = NULL;
    AjPAtom  atm         = NULL;
    AjPAtom  atm2        = NULL;
    AjPStr   tmpstr      = NULL;
/*    AjPResidue *resarr      = NULL; */



    if(!errf || !outf || !pdb || !scop)
      ajFatal("Bad args. passed to WriteAtomDomain");

    
    /* Allocate strings etc */
    tmpstr = ajStrNew();
    


    /* Loop for each chain in the domain */
    for(z=0;z<scop->N;z++,found_start=ajFalse, found_end=ajFalse)
    {
	/* Check for chain error */
	if(!ajPdbChnidToNum(scop->Chain[z], pdb, &chn))
	    {
		ajListIterDel(&iter);	
		ajWarn("Chain incompatibility error in "
		       "WriteAtomDomain");		
		ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
			    "error in WriteAtomDomain\n", 
			    scop->Entry);
		ajStrDel(&tmpstr);

		return ajFalse;
	    }
	
	
/*	ajListToarray(pdb->Chains[chn-1]->Residues, (void ***) &resarr);  */




	/* Iterate up to the correct model */
	iter=ajListIterNewread(pdb->Chains[chn-1]->Atoms);	
	
	while((atm = (AjPAtom)ajListIterGet(iter)))
	    if(atm->Mod==mod)
		break;

	if(!atm)
	  ajFatal("Unexpected error (atm == NULL) in WriteAtomDomain\n");
	

	/* Increment res. counter from last chain if appropriate */
	if(noend)
	    rn_mod += atm2->Idx;
	else	 
	    rn_mod += finalrn;


	/* Start of chain was not specified */
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else 
	    nostart = ajFalse;
	

			    
	/* End of chain was not specified */
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else
	    noend = ajFalse;
	

	/* If the domain was composed of more than once chain then a '.' is
	   given as the chain identifier */
	if(scop->N > 1)
	    id = '.';
	else 
	    id = pdb->Chains[chn-1]->Id;


	
	for(; atm; atm=(AjPAtom)ajListIterGet(iter)) 	
	{
	    /*
	    ** Continue / break if a non-protein atom is found or
	    ** model no. is incorrect
	    */
	    if(atm->Mod!=mod)  
		break;

	    if(atm->Type!='P')
		continue;

	    /*
	    ** The start position was specified, but has not 
	    ** been found yet
	    */
	    if(!found_start && !nostart)
	    {
		ajStrAssignS(&tmpstr, scop->Start[z]);
		ajStrAppendK(&tmpstr, '*');

		/* Start position found */
		/* if(!ajStrCmpCaseS(atm->Pdb, scop->Start[z])) */
		 if(ajStrMatchWildS(atm->Pdb, tmpstr)) 
		     /* if(ajStrMatchWildS(resarr[atm->Idx-1]->Pdb, tmpstr)) */
		{
		     if(!ajStrMatchS(atm->Pdb, scop->Start[z])) 
			 /* if(!ajStrMatchS(resarr[atm->Idx-1]->Pdb,
			    scop->Start[z])) */
		    {
			ajWarn("Domain start found by wildcard match only "
			       "in WriteAtomDomain");
			ajFmtPrintF(errf, "//\n%S\nERROR Domain start found "
				    "by wildcard match only in "
				    "WriteAtomDomain\n", scop->Entry);
		    }

		    rn_mod -= atm->Idx-1;
		    found_start = ajTrue;	
		}
		else	
		    continue;
	    }	
	    


	    /* The end position was specified, but has not 
		       been found yet */
	    if(!found_end && !noend)
	    {
		ajStrAssignS(&tmpstr, scop->End[z]);
		ajStrAppendK(&tmpstr, '*');

		/* End position found */
		/* if(!ajStrCmpCaseS(atm->Pdb, scop->End[z])) */
		 if(ajStrMatchWildS(atm->Pdb, tmpstr)) 
		     /* if(ajStrMatchWildS(resarr[atm->Idx-1]->Pdb, tmpstr)) */
		{
		     if(!ajStrMatchS(atm->Pdb, scop->End[z])) 
			 /* if(!ajStrMatchS(resarr[atm->Idx-1]->Pdb,
			    scop->End[z])) */
		    {
			ajWarn("Domain end found by wildcard match only "
			       "in WriteAtomDomain");
			ajFmtPrintF(errf, "//\n%S\nERROR Domain end found "
				    "by wildcard match only in "
				    "WriteAtomDomain\n", scop->Entry);
		    }

		    found_end = ajTrue;     
		    finalrn   = atm->Idx;
		}
	    }	
	    else if(atm->Idx != finalrn && !noend)
		break;


	    /* Write out ATOM line to pdb file */
	    if(mode == ajIDX)
		ajFmtPrintF(outf, "%-6s%5d  %-4S%-4S%c%4d%12.3f%8.3f"
			    "%8.3f%6.2f%6.2f%11s%-3c\n", 
			    "ATOM", 
			    acnt++, 
			    atm->Atm, 
			    atm->Id3, 
			    id,
			    atm->Idx+rn_mod, 
			    atm->X, 
			    atm->Y, 
			    atm->Z, 
			    atm->O, 
			    atm->B, 
			    " ", 
			    *ajStrGetPtr(atm->Atm));
	    else
		ajFmtPrintF(outf, "%-6s%5d  %-4S%-4S%c%4S%12.3f%8.3f"
			    "%8.3f%6.2f%6.2f%11s%-3c\n", 
			    "ATOM", 
			    acnt++, 
			    atm->Atm, 
			    atm->Id3, 
			    id,
			    atm->Pdb,  
			    /* resarr[atm->Idx-1]->Pdb,  */
			    atm->X, 
			    atm->Y, 
			    atm->Z, 
			    atm->O, 
			    atm->B, 
			    " ", 
			    *ajStrGetPtr(atm->Atm));

	    /* Assign pointer for this chain */
	    atm2=atm;
	}


	/* Diagnostic if start was specified but not found */
	if(!found_start && !nostart)
        {
            ajListIterDel(&iter);	
            ajWarn("Domain start not found in WriteAtomDomain");
            ajFmtPrintF(errf, "//\n%S\nERROR Domain start not "
                        "found in WriteAtomDomain\n", scop->Entry);
            ajStrDel(&tmpstr);

            return ajFalse;
        }
	

	/* Diagnostic if end was specified but not found */
	if(!found_end && !noend)
        {
            ajListIterDel(&iter);	
            ajWarn("Domain end not found in WriteAtomDomain");
            ajFmtPrintF(errf, "//\n%S\nERROR Domain end not "
                        "found in WriteAtomDomain\n", scop->Entry);
            ajStrDel(&tmpstr);

            return ajFalse;
        }
	

	ajListIterDel(&iter);	
    }
    
    if(!atm2)
      ajFatal("Unexpected error (atm2 == NULL) in WriteAtomDomain\n"); 

    /* Write the TER record to the pdb file */
    ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
		"TER", 
		acnt++, 
		atm2->Id3, 
		id,	
		atm2->Idx+rn_mod, 
		" ");
    
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @funcstatic WriteAtomDomainPdb *******************************************
**
** Writes coordinates for a SCOP domain to an output file in pdb format (ATOM 
** records).  Coordinates are taken from a Pdb structure, domain definition 
** is taken from a Scop structure. 
** Coordinates for heterogens are NOT written to 
** file.  The original (pdb) residue numbers are given (these do NOT give an
** index into the SEQRES sequence.  Use WriteAtomDomainIdx if you 
** need an index into the SEQRES sequence.  
**
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
** @param [r] scop [const AjPScop] Scop object
** @param [r] mod  [ajint] Model number, beginning at 1
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteAtomDomainPdb(AjPFile errf, AjPFile outf, const AjPPdb pdb, 
				 const AjPScop scop, ajint mod)
{
    if(WriteAtomDomain(errf, outf, pdb, scop, mod, ajPDB))
	return ajTrue;
    
    return ajFalse;
}




/* @funcstatic WriteAtomDomainIdx *******************************************
**
** Writes coordinates for a SCOP domain to an output file in pdb format (ATOM 
** records).  Coordinates are taken from a Pdb structure, domain definition 
** is taken from a Scop structure. The model number argument should have a 
** value of 1 for x-ray structures. Coordinates for heterogens are NOT 
** written to file.  The corrected residue numbers are given (these give an 
** index into the SEQRES sequence).  Use WriteAtomDomainPdb if you 
** wish to maintain the original residue number. 
**
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] scop [const AjPScop] Scop object
** @param [r] mod  [ajint]   Model number, beginning at 1
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteAtomDomainIdx(AjPFile errf, AjPFile outf,const  AjPPdb pdb, 
				 const AjPScop scop, ajint mod)
{
    if(WriteAtomDomain(errf, outf, pdb, scop, mod, ajIDX))
	return ajTrue;
    
    return ajFalse;
}




/* @funcstatic WriteHeterogen ***********************************************
**
** Writes coordinates for heterogens that could not be uniquely associated 
** with a chain to an output file in pdb format (HETATM records). Coordinates 
** are taken from a Pdb structure. The model number argument should have a 
** value of 1 for x-ray structures.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] mod  [ajint]   Model number, beginning at 1
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteHeterogen(AjPFile outf, const AjPPdb pdb, ajint mod)
{
    AjIList  iter = NULL;
    AjPAtom  atm  = NULL;
    ajint acnt;
    

    /* Check args are not NULL */
    if(!outf || !pdb || mod<1)
	return ajFalse;
    

        iter=ajListIterNewread(pdb->Groups);	

    while((atm = (AjPAtom)ajListIterGet(iter)))
	if(atm->Mod==mod)
	    break;
  
    for(acnt=1; atm; atm=(AjPAtom)ajListIterGet(iter)) 	
    {
	/* Break if on t0 a new model */
	if(atm->Mod!=mod)
	    break;
	
	/* Write out HETATM line */

	if(atm->Type == 'H')
	    ajFmtPrintF(outf, "%-6s%5d  %-4S%-4S%c%4d%12.3f%8.3f%8.3f"
			"%6.2f%6.2f%11s%-3c\n", 
			"HETATM", 
			acnt++, 
			atm->Atm, 
			atm->Id3, 
			ajChararrGet(pdb->gpid, atm->Gpn-1),
			atm->Idx, 
			atm->X, 
			atm->Y, 
			atm->Z, 
			atm->O,
			atm->B,
			" ", 
			*ajStrGetPtr(atm->Atm));
	else
	    ajFmtPrintF(outf, "%-6s%5d  %-4S%-4S%c%4d%12.3f%8.3f%8.3f"
			"%6.2f%6.2f%11s%-3c\n", 
			"HETATM", 
			acnt++, 
			atm->Atm, 
			atm->Id3, 
			' ',
			atm->Idx, 
			atm->X, 
			atm->Y, 
			atm->Z, 
			atm->O,
			atm->B,
			" ", 
			*ajStrGetPtr(atm->Atm));
/*	atm2 = atm; Unused variable */
    }

    
    ajListIterDel(&iter);				    

    return ajTrue;
}




/* @funcstatic WriteText ****************************************************
**
** Writes text to file in the format of pdb records
** 
** @param [w] outf   [AjPFile] Output file stream
** @param [r] str    [const AjPStr]  Text to print out
** @param [r] prefix [const char *]  pdb record (e.g. "HEADER")
**
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteText(AjPFile outf, const AjPStr str, const char *prefix)
{
    ajint n = 0;
    ajint l = 0;
    ajint c = 0;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr tmp       = NULL;
    
    if(!outf)
	return ajFalse;



    /* Initialise strings */    
    token = ajStrNew();
    tmp   = ajStrNewC("");
    

    handle = ajStrTokenNewC(str," \t\r\n");
    
    while(ajStrTokenNextParse(&handle,&token))
    {
	if(!c)
	    ajFmtPrintF(outf,"%-11s",prefix);
	
	if((l=n+ajStrGetLen(token)) < 68)
	{
	    if(c++)
		ajStrAppendC(&tmp," ");
	    ajStrAppendS(&tmp,token);
	    n = ++l;
	}
	else
	{
	    ajFmtPrintF(outf,"%-*S\n",69, tmp);

	    ajStrAssignS(&tmp,token);
	    ajStrAppendC(&tmp," ");
	    n = ajStrGetLen(token);
	    c = 0;
	}
    }

    if(c)
	ajFmtPrintF(outf,"%-*S\n",69, tmp);
    

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);
    
    return ajTrue;
}




/* @funcstatic WriteHeader **************************************************
**
** Writes the Pdb element of a Pdb structure to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteHeader(AjPFile outf, const AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11sCLEANED-UP PDB FILE FOR %-45S\n", 
		    "HEADER", 
		    pdb->Pdb);    

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic WriteHeaderScop **********************************************
**
** Writes the Entry element of a Scop structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] scop [const AjPScop] Scop object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteHeaderScop(AjPFile outf, const AjPScop scop)
{
    if(scop && outf)
    {
	ajFmtPrintF(outf, "%-11sCLEANED-UP PDB FILE FOR SCOP DOMAIN %-33S\n", 
		    "HEADER", 
		    scop->Entry);    

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic WriteTitle ***************************************************
**
** Writes a TITLE record to an output file in pdb format
** The text is hard-coded.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteTitle(AjPFile outf, const AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11sTHIS FILE IS MISSING MOST RECORDS FROM THE "
		    "ORIGINAL PDB FILE%9s\n", 
		    "TITLE", " ");

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic WriteCompnd **************************************************
**
** Writes the Compnd element of a Pdb structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
****************************************************************************/
static AjBool WriteCompnd(AjPFile outf, const AjPPdb pdb)
{
    if(pdb && outf)
    {
	WriteText(outf,pdb->Compnd,"COMPND");

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic WriteSource **************************************************
**
** Writes the Source element of a Pdb structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteSource(AjPFile outf, const AjPPdb pdb)
{
    if(pdb && outf)
    {
	WriteText(outf,pdb->Source,"SOURCE");

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic WriteEmptyRemark *********************************************
**
** Writes an empty REMARK record to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteEmptyRemark(AjPFile outf, const AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11s%-69s\n", "REMARK", " ");

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic WriteResolution **********************************************
**
** Writes the Reso element of a Pdb structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool WriteResolution(AjPFile outf, const AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11sRESOLUTION. %-6.2f%-51s\n", 
		    "REMARK", pdb->Reso, "ANGSTROMS.");

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic  ReadLines ******************************************
**
** Reads a pdb file and returns a pointer to a partially filled Pdbfile object.
** All of the lines from the pdb file are written to the <lines> array of the 
** object and the <nlines> element is written.
**
** Memory for the object itself and any arrays whose size is equal to the 
** number of lines is allocated:
** lines, linetype, chnn, gpn, modn, resni, resn1, resn2, pdbn, oddnum, atype, 
** rtype, x,y,z,o,b, elementNum, elementId, elementType & helixClass.
** 
** The following elements are written:
** nlines, lines.
** 
** The linetype array is set to default value of PDBPARSE_IGNORE
**
** @param [u] inf  [AjPFile] Pointer to pdb file
**
** @return [AjPPdbfile] Pdbfile object pointer, or NULL on failure.
** @@
****************************************************************************/

static AjPPdbfile ReadLines(AjPFile inf)
{
    AjPPdbfile ptr    =NULL;		/* pdbfile object to be returned */
    AjPList    list   =NULL;		/* List of lines in pdb file */
    AjPStr     line   =NULL;		/* A line from a pdb file */
    ajint      i      =0;
    




    /* Check args */
    if(!inf)
	return NULL;
    
    
    /* Allocate list and pdbfile object*/
    list = ajListstrNew();
    /* Don't know number of lines or chains yet */
    ptr  = PdbfileNew(0,0);	
    
    
    /* Read pdb file and append lines to list */ 
    line = ajStrNew();

    while(ajReadlineTrim(inf,&line))
    {
	ajListstrPushAppend(list, line);
	line = ajStrNew();
    }

    
    /* Convert list to array in pdbfile object */
    ptr->nlines = ajListstrToarray(list, &(ptr->lines));

    if(ptr->nlines==0)
    {
	ajStrDel(&line);
	ajListstrFree(&list);
	PdbfileDel(&ptr);

	return NULL;
    }
    
    
        
    /* Allocate memory for x,y,z,o,b, modn, chnn, linetype, ok, coord and pdbn 
       arrays in pdbfile object */
    AJCNEW0(ptr->x, ptr->nlines); 
    AJCNEW0(ptr->y, ptr->nlines); 
    AJCNEW0(ptr->z, ptr->nlines); 
    AJCNEW0(ptr->o, ptr->nlines); 
    AJCNEW0(ptr->b, ptr->nlines); 

    AJCNEW0(ptr->elementNum, ptr->nlines); 
    AJCNEW0(ptr->elementType, ptr->nlines); 
    AJCNEW0(ptr->helixClass, ptr->nlines); 
    AJCNEW0(ptr->resni, ptr->nlines); 
    AJCNEW0(ptr->resn1, ptr->nlines); 
    AJCNEW0(ptr->resn2, ptr->nlines); 
    AJCNEW0(ptr->modn, ptr->nlines);

    AJCNEW0(ptr->chnn, ptr->nlines);
    AJCNEW0(ptr->gpn, ptr->nlines);

    AJCNEW0(ptr->linetype, ptr->nlines);

    for(i=0;i<ptr->nlines;i++)
	ptr->linetype[i]=PDBPARSE_IGNORE;


    AJCNEW0(ptr->pdbn, ptr->nlines);

    for(i=0;i<ptr->nlines;i++)
	ptr->pdbn[i]=ajStrNew();

    AJCNEW0(ptr->elementId, ptr->nlines);

    for(i=0;i<ptr->nlines;i++)
	ptr->elementId[i]=ajStrNew();


    AJCNEW0(ptr->atype, ptr->nlines);

    for(i=0;i<ptr->nlines;i++)
	ptr->atype[i]=ajStrNewRes(4);

    AJCNEW0(ptr->rtype, ptr->nlines);

    for(i=0;i<ptr->nlines;i++)
	ptr->rtype[i]=ajStrNewRes(4);


    AJCNEW0(ptr->oddnum, ptr->nlines);

    for(i=0;i<ptr->nlines;i++)
	ptr->oddnum[i]=ajFalse;
    
/*DIAGNOSTIC 
//      for(i=0;i<ptr->nlines;i++)
//      {
//      ajFmtPrintF(tempfile, "%S\n", ptr->lines[i]);
//      fflush(tempfile->fp);
//      }
*/
    

    /* Tidy up and return */
    ajStrDel(&line);
    ajListstrFree(&list);

    return ptr;
}




/* @funcstatic  PdbfileDel **************************************************
**
** Destructor for pdbfile object.
**
** @param [d] pthis [AjPPdbfile*] Pdbfile object pointer
**
** @return [void]
** @@
****************************************************************************/

static void PdbfileDel(AjPPdbfile *pthis)
{
    ajint i;
    
    
    /* Check args */
    if(!(*pthis))
    {
	ajWarn("NULL arg passed to PdbfileDel.\n");

	return;
    }
    
    if((*pthis)->resn1ok)
	AJFREE((*pthis)->resn1ok);
    
    if((*pthis)->nres)
	AJFREE((*pthis)->nres);
    
    if((*pthis)->nligands)
	AJFREE((*pthis)->nligands);
    
    if((*pthis)->chainok)
	AJFREE((*pthis)->chainok);
    
    if((*pthis)->pdbid)
	ajStrDel(&(*pthis)->pdbid);
        
    if((*pthis)->compnd)
	ajStrDel(&(*pthis)->compnd);
    
    if((*pthis)->source)
	ajStrDel(&(*pthis)->source);
    
    
    if((*pthis)->modn)
	AJFREE((*pthis)->modn);
    
    if((*pthis)->x)
	AJFREE((*pthis)->x); 

    if((*pthis)->y)
	AJFREE((*pthis)->y); 

    if((*pthis)->z)
	AJFREE((*pthis)->z); 

    if((*pthis)->o)
	AJFREE((*pthis)->o); 

    if((*pthis)->b)
	AJFREE((*pthis)->b); 
    
    if((*pthis)->numHelices)
	AJFREE((*pthis)->numHelices); 

    if((*pthis)->numStrands)
	AJFREE((*pthis)->numStrands); 

    if((*pthis)->numSheets)
	AJFREE((*pthis)->numSheets); 

    if((*pthis)->numTurns)
	AJFREE((*pthis)->numTurns); 
    
    if((*pthis)->elementNum)
	AJFREE((*pthis)->elementNum); 

    if((*pthis)->elementType)
	AJFREE((*pthis)->elementType); 

    if((*pthis)->helixClass)
	AJFREE((*pthis)->helixClass); 
    
    if((*pthis)->resni)
	AJFREE((*pthis)->resni); 
    
    if((*pthis)->resn1)
	AJFREE((*pthis)->resn1); 
    
    if((*pthis)->resn2)
	AJFREE((*pthis)->resn2); 
    
    if((*pthis)->chnn)
	AJFREE((*pthis)->chnn);
    
    if((*pthis)->gpn)
	AJFREE((*pthis)->gpn);
    
    if((*pthis)->linetype)
	AJFREE((*pthis)->linetype);
    
    if((*pthis)->oddnum)
	AJFREE((*pthis)->oddnum);
    
    if((*pthis)->lines)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->lines[i]);

	AJFREE((*pthis)->lines);
    }
    
    if((*pthis)->pdbn)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->pdbn[i]);

	AJFREE((*pthis)->pdbn);
    }
    
    if((*pthis)->elementId)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->elementId[i]);

	AJFREE((*pthis)->elementId);
    }
    
    if((*pthis)->atype)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->atype[i]);

	AJFREE((*pthis)->atype);
    }
    
    if((*pthis)->rtype)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->rtype[i]);

	AJFREE((*pthis)->rtype);
    }
    
    if((*pthis)->seqres)
    {
	for(i=0;i<(*pthis)->nchains; i++)
	    ajStrDel(&(*pthis)->seqres[i]);

	AJFREE((*pthis)->seqres);
    }

    if((*pthis)->seqresful)
    {
	for(i=0;i<(*pthis)->nchains; i++)
	    ajStrDel(&(*pthis)->seqresful[i]);

	AJFREE((*pthis)->seqresful);
    }
    
    
    if((*pthis)->chid)
	ajChararrDel(&(*pthis)->chid);
    
    if((*pthis)->gpid)
	ajChararrDel(&(*pthis)->gpid);
    
    
    AJFREE(*pthis);
    *pthis=NULL;
    
    return;
}




/* @funcstatic  ElementsNew *************************************************
**
** Constructor for Elements object.
**
** @param [r] nelms    [ajint] no. of elements
**
** @return [AjPElements] Pointer to Elements object, or NULL on failure.
** @@
****************************************************************************/
static AjPElements ElementsNew(ajint nelms)
{
    AjPElements ret=NULL;
    ajint       x  =0;
    
    AJNEW0(ret);

    ret->n=nelms;
    
    if(nelms)
    {	
	AJCNEW0(ret->elms, nelms);

	for(x=0;x<nelms;x++)
	    ret->elms[x] = ElementNew();
    }
/*
    else
	ajWarn("Value of zero passed to ElementsNew");
  */  

    return ret;
}




/* @funcstatic  ElementsDel *************************************************
**
** Destructor for Elements object.
**
** @param [d] ptr [AjPElements*] Elements object pointer
**
** @return [void]
** @@
****************************************************************************/

static void ElementsDel(AjPElements *ptr)
{
    ajint x=0;

    
    if(!(*ptr))
    {
	ajWarn("NULL arg passed to ElementsDel");

	return;
    }
    
    if((*ptr)->elms)
    {
	for(x=0; x<(*ptr)->n; x++)
	    ElementDel(&(*ptr)->elms[x]);

	AJFREE((*ptr)->elms);
    }
    
    AJFREE((*ptr));
    *ptr=NULL;
    
    return;
}




/* @funcstatic  ElementNew **************************************************
**
** Constructor for Element object.
**
** @return [AjPElement] Pointer to Element object, or NULL on failure.
** @@
****************************************************************************/

static AjPElement ElementNew(void)
{
    AjPElement ret;

    AJNEW0(ret);
    
    ret->elementId   = ajStrNew();
    ret->initResName = ajStrNew();
    ret->initSeqNum  = ajStrNew();
    ret->endResName  = ajStrNew();
    ret->endSeqNum   = ajStrNew();

    ret->elementNum  = 0;
    ret->elementType = ' ';
    ret->helixClass  = 0;
    ret->chainId     = ' ';

    return ret;
}




/* @funcstatic  ElementDel **************************************************
**
** Destructor for Element object.
**
** @param [d] ptr [AjPElement*] Element object pointer
**
** @return [void]
** @@
****************************************************************************/

static void ElementDel(AjPElement *ptr)
{
    ajStrDel(&((*ptr)->elementId));
    ajStrDel(&((*ptr)->initResName));
    ajStrDel(&((*ptr)->initSeqNum));
    ajStrDel(&((*ptr)->endResName));    
    ajStrDel(&((*ptr)->endSeqNum));

    AJFREE((*ptr));
    *ptr=NULL;

    return;
}




/* @funcstatic  PdbfileNew **************************************************
**
** Constructor for Pdbfile object.
**
** @param [r] nlines  [ajint] No. of lines in pdb file
** @param [r] nchains [ajint] No. of chains in pdb file
**
** @return [AjPPdbfile] Pointer to pdbfile object, or NULL on failure.
** @@
****************************************************************************/

static AjPPdbfile PdbfileNew(ajint nlines, ajint nchains)
{
    AjPPdbfile ret  =NULL;
    ajint i=0;

    AJNEW0(ret);

    ret->pdbid = ajStrNew();
    ret->compnd = ajStrNew();
    ret->source = ajStrNew();

    ret->nomod=ajFalse;
    ret->toofewter=ajFalse;
    
    ret->nchains=nchains;

    if(nchains)
    {
	AJCNEW0(ret->resn1ok, nchains);

	for(i=0;i<nchains;i++)
	    ret->resn1ok[i]=ajTrue;


	AJCNEW0(ret->numHelices, nchains);

	for(i=0;i<nchains;i++)
	    ret->numHelices[i]=ajTrue;

	AJCNEW0(ret->numStrands, nchains);

	for(i=0;i<nchains;i++)
	    ret->numStrands[i]=ajTrue;

	AJCNEW0(ret->numSheets, nchains);

	for(i=0;i<nchains;i++)
	    ret->numSheets[i]=ajTrue;

	AJCNEW0(ret->numTurns, nchains);

	for(i=0;i<nchains;i++)
	    ret->numTurns[i]=ajTrue;


	AJCNEW0(ret->chainok, nchains);

	for(i=0;i<nchains;i++)
	    ret->chainok[i]=ajTrue;

	AJCNEW0(ret->nres, nchains);

	AJCNEW0(ret->nligands, nchains);
	
	AJCNEW0(ret->seqres, nchains);

	for(i=0;i<nchains;i++)
	    ret->seqres[i]=ajStrNew();

	AJCNEW0(ret->seqresful, nchains);

	for(i=0;i<nchains;i++)
	    ret->seqresful[i]=ajStrNew();


	ret->chid = ajChararrNewRes(nchains);
    }
    else
    {
	ret->chid = ajChararrNew();	
    }

    ret->gpid = ajChararrNew();
    

    ret->nlines = nlines;

    if(nlines)
    {
	AJCNEW0(ret->x, nlines); 
	AJCNEW0(ret->y, nlines); 
	AJCNEW0(ret->z, nlines); 
	AJCNEW0(ret->o, nlines); 
	AJCNEW0(ret->b, nlines); 

	AJCNEW0(ret->elementNum, nlines); 
	AJCNEW0(ret->elementType, nlines); 
	AJCNEW0(ret->helixClass, nlines); 

	AJCNEW0(ret->resni, nlines); 
	AJCNEW0(ret->resn1, nlines); 
	AJCNEW0(ret->resn2, nlines); 
	AJCNEW0(ret->modn, nlines);
	AJCNEW0(ret->chnn, nlines);
	AJCNEW0(ret->gpn, nlines);
	AJCNEW0(ret->linetype, nlines);
	AJCNEW0(ret->oddnum, nlines);
	AJCNEW0(ret->lines, nlines);
	AJCNEW0(ret->pdbn, nlines);
	AJCNEW0(ret->elementId, nlines);
	AJCNEW0(ret->atype, nlines);
	AJCNEW0(ret->rtype, nlines);

	for(i=0;i<nlines;i++)
	{
	    ret->linetype[i]=PDBPARSE_IGNORE;
	    ret->oddnum[i]=ajFalse;
	    ret->lines[i]=ajStrNew();
	    ret->pdbn[i]=ajStrNew();
	    ret->elementId[i]=ajStrNew();
	    ret->atype[i]=ajStrNewRes(4);
	    ret->rtype[i]=ajStrNewRes(4);
	}
    }
/*
    else
	ajWarn("Zero sized arg passed to PdbfileNew.\n");
*/
    
    return ret;
}




/* @funcstatic FirstPass ******************************************************
**
** The initial read of the pdb file as held in the <lines> array of a Pdbfile 
** object. Bibliographic information is parsed, the number of chains 
** determined and the sequences and chain ids from the SEQRES records are
** parsed. The line type (see below) of each line is determined, and for 
** lines with coordinates, the residue type is parsed. A count of the number
** of TER records and the location of the first coordinate line is also
** determined.
** Secondary structure information is also parsed and an AjPElements 
** object is written.
** 
** Memory for any arrays whose size is equal to the number of chains is 
** allocated:
** seqres, seqresful, nres, chainok, resn1ok, nligands, numHelices, 
** numStrands, numSheets, numTurns 
**
** The following arrays are written:
** seqres, seqresful, chid, pdbn, resn1 and resn2, rtype, linetype
** 
** The following elements are written:
** nchains, tercnt, modcnt, nomod, source, compnd, method, reso, idxfirst
**
** The chainok & resn1ok arrays are set to default values of ajTrue
** 
**
** Writing resn1/resn2 & pdbn arrays
** The pdbn array is the raw residue number (as a string) and is filled for
** lines for which <linetype>==PDBPARSE_COORD. The resn1/resn2 arrays are
** given initial values which at this stage are simply the integer component
** of pdbn. The values for resn1/resn2 are changed later in the program.
**
** Writing modcnt and nomod elements
** modcnt is a count of the number of MODEL records (excluding duplicate 
** records). However, if no MODEL records are found, modcnt is set to the
** minimum value of 1, and nomod is set to ajTrue.
**
** Writing linetype array  
** The linetype array is set as follows:
**
** PDBPARSE_COORD for ATOM or HETATM records which contain both atom and
** residue  identifier codes and which are not duplicate positions.
** Duplicate positions for  (i) whole residues or (ii) individual atoms 
** are presumed where a ATOM or HETATM record uses a value other than '1' or 
** 'A' in the (i) residue alternate location indicator (column 17) or (ii)
** the first column of the atom name (column 13) respectively.
**
** PDBPARSE_TER for TER records
**
** PDBPARSE_MODEL for MODEL records
**
** PDBPARSE_COORDWAT for HOH (should be HETATM records)
**
** For all other lines, it is left as the default of PDBPARSE_IGNORE (the value
** might change later in the program).
**
**
** @param [w] pdbfile  [AjPPdbfile]  Pdbfile object
** @param [u] flog     [AjPFile]       Log file (build diagnostics)
** @param [w] elms     [AjPElements*] Elements object pointer    
** @param [r] camask   [AjBool]        Whether to mask non-amino acid residues 
**                                    within protein chains which do not
**                                    have a C-alpha atom.
** 
** @return [AjBool]  True if file was parsed, False otherwise
** @@
****************************************************************************/

static AjBool FirstPass(AjPPdbfile pdbfile, AjPFile flog, AjPElements *elms, 
			AjBool camask)
{
    ajint    i=0;		       /* Loop counter */
    ajint    j=0;		       /* Loop counter */
    ajint    k=0;		       /* Loop counter */
    AjBool   donefirstatom=ajFalse;    /* Flag for finding first ATOM or 
				          HETATM line */
    char     pdbn[6];		       /* Residue number */
    AjBool   resolfound=ajFalse;       /* Flag for finding RESOLUTION record */
    AjBool   seqresfound=ajFalse;      /* Flag for finding SEQRES record */
    AjPStr   tmpstr=NULL;	       /* A temp. string */
    AjPStr   seqres=NULL;	       /* Sequence from SEQRES records */
    ajint    seqreslen=0;	       /* Indicated length of sequence from 
				          SEQRES records */
    ajint    lenful=0;                 /* Length of SEQRES sequence including
				          ACE, FOR & NH2 groups that might be 
				          discarded by the call to 
				          SeqresToSequence */
    char     last_id=' ';	       /* CHain id of last SEQRES line read*/
    AjPStr   tmpseq=NULL;	       /* A temp. string for a sequence */
    AjPStr   tmpseqful=NULL;	       /* A temp. string for a sequence */
    AjPList  listseqs=NULL;	       /* For list of sequences from SEQRES 
				          records*/
    AjPList  listseqsful=NULL;	       /* For list of sequences (using 
				          3-letter codes) from SEQRES 
				          records*/
    AjBool   done_msg=ajFalse;	       /* Flag for error messaging */
    AjPList  listelms=NULL;	       /* Temp. list of secondary structure 
				          elements (from HELIX, SHEET and TURN 
				          records) */
    AjPElement  elm=NULL;	       /* Temp. Element object pointer */
    AjPElement  FirstStrand=NULL;     /* Temp. pointer to first strand of 
				          each sheet*/
    AjBool     doneFirstStrand=ajFalse;/* Flag for parsing first strand of 
				          each sheet */
    AjPStr     LastSheetId=NULL;       /* Sheet identifier of the last sheet 
				          read in */
    




    /* Check args */
    if(!pdbfile || !flog || !(*elms))
    {
	ajWarn("Bad args passed to FirstPass\n");

	return ajFalse;
    }
    
    
    /* Allocate memory etc*/
    tmpstr=ajStrNew();
    seqres=ajStrNew();
    LastSheetId=ajStrNew();
    
    listseqs=ajListstrNew();
    listseqsful=ajListstrNew();
    listelms=ajListNew();

    
    /* Start of main loop */
    for(i=0;i<pdbfile->nlines;i++)
    {
	if((ajStrPrefixC(pdbfile->lines[i],"ATOM")) || 
	   (ajStrPrefixC(pdbfile->lines[i],"HETATM")))
	{ 
	    /*In instances where >1 residue positions are given, ignore 
	      all but position 'A' & '1'
	      In instances where >1 atom positions are given, ignore all 
	      but position '1'*/
	    if(((ajStrGetCharPos(pdbfile->lines[i], 16) != ' ')    && 
		((ajStrGetCharPos(pdbfile->lines[i], 16) != 'A')   &&
		 (ajStrGetCharPos(pdbfile->lines[i], 16) != '1'))) ||
	       ((ajStrGetCharPos(pdbfile->lines[i], 12) != ' ')    && 
		((ajStrGetCharPos(pdbfile->lines[i], 12) != '1')   &&
		 (isdigit((int)ajStrGetCharPos(pdbfile->lines[i], 12))))))
	    {
		if(!done_msg)
		{
		    ajFmtPrintF(flog, "%-15s%d\n", "DUPATOMRES", i+1); 
		    done_msg=ajTrue;
		}
		

	    }
	    /*In instances where no atom or residue identity is given, 
	      ignore line */
	    else if((ajStrGetCharPos(pdbfile->lines[i], 12)==' '  && 
		     ajStrGetCharPos(pdbfile->lines[i], 13)==' '  && 
		     ajStrGetCharPos(pdbfile->lines[i], 14)==' ') ||
		    (ajStrGetCharPos(pdbfile->lines[i], 17)==' '  &&
		     ajStrGetCharPos(pdbfile->lines[i], 19)==' '))
	    {

		ajFmtPrintF(flog, "%-15s%d\n", "NOATOMRESID", i+1); 

	    }
	    else 
	    {

		pdbfile->linetype[i]=PDBPARSE_COORD;
		
               
		if(!donefirstatom)
		{
		    donefirstatom=ajTrue;
		    pdbfile->idxfirst=i;
		}

		
		/*Write residue number for the line */
		for(k=22, j=0; k<=26; k++)
		    if((isalnum((int)ajStrGetCharPos(pdbfile->lines[i], k)))
		       ||
		       ajStrGetCharPos(pdbfile->lines[i], k)=='-')
			pdbn[j++] = ajStrGetCharPos(pdbfile->lines[i], k);

		pdbn[j]='\0';
		
		ajStrAssignC(&(pdbfile->pdbn[i]), pdbn);


		
		if(!ajFmtScanS(pdbfile->pdbn[i], "%d", 
			       &(pdbfile->resn1[i])))
		{
		    ajFmtPrintF(flog, "%-15s%d\n", "ATOMNONUM", i+1);
		    pdbfile->linetype[i]=PDBPARSE_IGNORE;		    
		}
		else
		{
		    pdbfile->resn2[i]=pdbfile->resn1[i];

		    /* Assign residue type */ 
		    ajStrAssignSubS(&pdbfile->rtype[i], 
				pdbfile->lines[i], 17, 19);
		    ajStrRemoveWhite(&pdbfile->rtype[i]);

		    /* JONNEW */
		    if(ajStrMatchC(pdbfile->rtype[i], "HOH"))
			pdbfile->linetype[i]=PDBPARSE_COORDWAT;
		}
	    }
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"SEQRES"))
	{
	    seqresfound = ajTrue;
	    
	    /* Read first SEQRES line */
	    ajStrAssignSubS(&seqres, pdbfile->lines[i], 14, 18);

	    if(!ajFmtScanS(seqres, "%d", &seqreslen))
		ajFmtPrintF(flog, "%-15s%d (%c)\n", "SEQRESLEN", 
			    pdbfile->nchains, 
			    ajStrGetCharPos(pdbfile->lines[i], 11));


	    ajStrAssignSubS(&seqres, pdbfile->lines[i], 19, 70);
	    /* Append a ' ' in case this is missing from the PDB file,
	       e.g. pdb1iie.ent */
	    ajStrAppendK(&seqres, ' ');
	    

	    ajChararrPut(&(pdbfile->chid), 0, 
			 (last_id=ajStrGetCharPos(pdbfile->lines[i], 11))); 

	    
	    pdbfile->nchains++;
	    

	    /* Read subsequent SEQRES lines */
	    for(i++;i<pdbfile->nlines;i++)	    
		if(ajStrPrefixC(pdbfile->lines[i],"SEQRES"))
		{
		    /* Still on same chain */
		    if(ajStrGetCharPos(pdbfile->lines[i], 11) == last_id)
		    {
			ajStrAppendSubS(&seqres, pdbfile->lines[i], 19, 70);
			/* Append a ' ' in case this is missing from
			   the PDB file, e.g. pdb1iie.ent */
			ajStrAppendK(&seqres, ' ');
		    }
		    
		    /* On new chain */
		    else
		    {	
			tmpseq=ajStrNew();


			/* Process last chain */
			if(!SeqresToSequence(seqres, &tmpseq, camask, 
						      &lenful))
			{
			    ajWarn("Sequence conversion error in"
				   " FirstPass\nEmail jison@hgmp.mrc.ac.uk\n");
			    ajStrDel(&tmpseq);
			    continue;
			}
			
			tmpseqful=ajStrNew();			
			ajStrAssignS(&tmpseqful, seqres);
			


			

			/* Check length of sequence vs indicated length */
			if(lenful != seqreslen)
			{
			    ajFmtPrintF(flog, "%-15s%d (%c)\n", 
					"SEQRESLENDIF", 
					pdbfile->nchains, 
					last_id);
			    
			}		

			/* Push sequences onto lists */
			ajListstrPushAppend(listseqs, tmpseq);
			ajListstrPushAppend(listseqsful, tmpseqful);

			
			/* Read first SEQRES line of new chain */
			ajStrAssignSubS(&seqres, pdbfile->lines[i], 14, 18);

			if(!ajFmtScanS(seqres, "%d", &seqreslen))
			    ajFmtPrintF(flog, "%-15s%d (%c)\n", "SEQRESLEN", 
					pdbfile->nchains, 
					ajStrGetCharPos(pdbfile->lines[i],
							11));



			ajStrAssignSubS(&seqres, pdbfile->lines[i], 19, 70);
			/* Append a ' ' in case this is missing from the
			   PDB file, e.g. pdb1iie.ent */
			ajStrAppendK(&seqres, ' ');
			
			ajChararrPut(&(pdbfile->chid), pdbfile->nchains, 
				     (last_id
				      =ajStrGetCharPos(pdbfile->lines[i],
						       11)));

			/*
			   if((last_id=ajStrGetCharPos(pdbfile->lines[i],
                                                  11))==' ')
			   ajChararrPut(&(pdbfile->chid), 
			   pdbfile->nchains, '.');
			   else
			   ajChararrPut(&(pdbfile->chid), 
			   pdbfile->nchains, last_id); 
			   */
			


			pdbfile->nchains++;
		    }
		}
		else
		{
		    tmpseq=ajStrNew();

		    /* Process last chain */
		    if(!SeqresToSequence(seqres, &tmpseq, camask, 
						  &lenful))
		    {
			ajWarn("Sequence conversion error in "
			       "FirstPass\nEmail jison@hgmp.mrc.ac.uk\n");
			ajStrDel(&tmpseq);
			continue;
		    }

		    tmpseqful=ajStrNew();			
		    ajStrAssignS(&tmpseqful, seqres);


		    /* Check length of sequence vs indicated length */
		    if(lenful != seqreslen)
                    {
                        ajFmtPrintF(flog, "%-15s%d (%c)\n", 
                                    "SEQRESLENDIF", 
                                    pdbfile->nchains, 
                                    last_id);
                    }			
		    
		    /* Push sequences onto lists */
		    ajListstrPushAppend(listseqs, tmpseq);
		    ajListstrPushAppend(listseqsful, tmpseqful);
		    

		    /* Convert lists to arrays in pdbfile object and delete 
		       list */
		    ajListstrToarray(listseqs, &(pdbfile->seqres));
		    ajListFree(&listseqs);

		    ajListstrToarray(listseqsful, &(pdbfile->seqresful));
		    ajListFree(&listseqsful);
		    
		    /* i will get incremented in main loop */
		    i--;		
		    break;
		}
	    
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"COMPND"))
	{	
	    /* Read first COMPND line */
	    ajStrAssignSubS(&(pdbfile->compnd), 
			pdbfile->lines[i], 10, 71);
	    
	    /* Read subsequent COMPND lines */
	    for(i++;i<pdbfile->nlines;i++)	    
		if(ajStrPrefixC(pdbfile->lines[i],"COMPND"))
		{
		    ajStrAppendSubS(&(pdbfile->compnd), 
				pdbfile->lines[i], 10, 71);
		}
		else	
		{
		    ajStrRemoveWhiteExcess(&(pdbfile->compnd));
		    /* i will get incremented in main loop */
		    i--;		
		    break;
		}
	    
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"SOURCE"))
	{	
	    /* Read first SOURCE line */
	    ajStrAssignSubS(&(pdbfile->source), 
			pdbfile->lines[i], 10, 71);
	    
	    /* Read subsequent SOURCE lines */
	    for(i++;i<pdbfile->nlines;i++)	    
		if(ajStrPrefixC(pdbfile->lines[i],"SOURCE"))
		{
		    ajStrAppendSubS(&(pdbfile->source), 
				pdbfile->lines[i], 10, 71);
		}
		else	
		{
		    ajStrRemoveWhiteExcess(&(pdbfile->source));
		    /* i will get incremented in main loop */
		    i--;		
		    break;
		}
	    
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"TER")) 
	{
	    /* By default ok == ajTrue 
	       pdbfile->ok[i]=ajTrue; */
	    pdbfile->tercnt++;
	    pdbfile->linetype[i]=PDBPARSE_TER;
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"MODEL"))
	{
	    pdbfile->modcnt++;
	    pdbfile->linetype[i]=PDBPARSE_MODEL;
	    
	    if(!donefirstatom)
	    {
		donefirstatom=ajTrue;
		pdbfile->idxfirst=i;
	    }
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"ENDMDL"))
	{
	    pdbfile->linetype[i]=PDBPARSE_ENDMDL;
	}
	
	else if((!resolfound) && (ajStrPrefixC(pdbfile->lines[i],
					       "REMARK")))
 	{
	    /*Assign method and resolution */
	    ajFmtScanS(pdbfile->lines[i], "%*s %*d %S", &tmpstr);
	    	    
	    if(!ajStrCmpLenC(tmpstr, "RESOLUTION", 10))
	    {
		resolfound = ajTrue;
		  
		if(isdigit((int)ajStrGetCharPos(pdbfile->lines[i], 23)))
		{
		    if( (ajFmtScanS(pdbfile->lines[i], 
				    "%*s %*d %*s %f", 
				    &(pdbfile->reso))) != 1)
			ajFmtPrintF(flog, "%-15s\n", "RESOLUNK");
		    
		    pdbfile->method=ajXRAY;
		}
		else
		{
		    pdbfile->reso=0;
		    pdbfile->method=ajNMR;
		}
	    }
	    
	}


	else if(ajStrPrefixC(pdbfile->lines[i],"HELIX"))
	{
	    doneFirstStrand=ajFalse;
	    

	    elm = ElementNew();

	    ajStrAssignSubS(&tmpstr, pdbfile->lines[i], 7, 9);
	    ajStrRemoveWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->elementNum);
	    
	    ajStrAssignSubS(&elm->elementId, pdbfile->lines[i], 11, 13);
	    ajStrRemoveWhite(&elm->elementId);

	    elm->elementType = 'H';

	    ajStrAssignSubS(&elm->initResName, pdbfile->lines[i], 15, 17);
	    ajStrRemoveWhite(&elm->initResName);

	    ajStrAssignSubS(&elm->initSeqNum, pdbfile->lines[i], 21, 25);
	    ajStrRemoveWhite(&elm->initSeqNum);

	    ajStrAssignSubS(&elm->endResName, pdbfile->lines[i], 27, 29);
	    ajStrRemoveWhite(&elm->endResName);

	    ajStrAssignSubS(&elm->endSeqNum, pdbfile->lines[i], 33, 37);
	    ajStrRemoveWhite(&elm->endSeqNum);

	    elm->chainId = ajStrGetCharPos(pdbfile->lines[i], 19);
	    
	    if(elm->chainId != ajStrGetCharPos(pdbfile->lines[i], 31))
	    {
		ajFmtPrintF(flog, "%-15s%c %c\n", "SECTWOCHN", elm->chainId,
			    ajStrGetCharPos(pdbfile->lines[i], 31));
		ElementDel(&elm);
		continue;	
	    }
	    
	    
	    ajStrAssignSubS(&tmpstr, pdbfile->lines[i], 38, 39);
	    ajStrRemoveWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->helixClass);
	    

	    /* Check that all records are present and flag an error if 
	       they're not */
	    if(MAJSTRGETLEN(elm->initResName)
	       && MAJSTRGETLEN(elm->initSeqNum) 
	       && MAJSTRGETLEN(elm->endResName) 
	       && MAJSTRGETLEN(elm->endSeqNum) && 
	       MAJSTRGETLEN(elm->elementId))
	    {
		ajListPushAppend(listelms, elm);
	    }
	    else	 
	    {
		ajFmtPrintF(flog, "%-15s%d\n", "SECMISS", i+1);
		ElementDel(&elm);
		continue;	
	    }
	    
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"SHEET"))
	{
	    elm = ElementNew();

	    ajStrAssignSubS(&tmpstr, pdbfile->lines[i], 7, 9);
	    ajStrRemoveWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->elementNum);

	    ajStrAssignSubS(&elm->elementId, pdbfile->lines[i], 11, 13);
	    ajStrRemoveWhite(&elm->elementId);

	    if(!ajStrMatchS(elm->elementId, LastSheetId))
		doneFirstStrand=ajFalse;		

	    elm->elementType = 'E';

	    ajStrAssignSubS(&elm->initResName, pdbfile->lines[i], 17, 19);
	    ajStrRemoveWhite(&elm->initResName);

	    ajStrAssignSubS(&elm->initSeqNum, pdbfile->lines[i], 22, 26);
	    ajStrRemoveWhite(&elm->initSeqNum);

	    ajStrAssignSubS(&elm->endResName, pdbfile->lines[i], 28, 30);
	    ajStrRemoveWhite(&elm->endResName);

	    ajStrAssignSubS(&elm->endSeqNum, pdbfile->lines[i], 33, 37);
	    ajStrRemoveWhite(&elm->endSeqNum);

	    elm->chainId = ajStrGetCharPos(pdbfile->lines[i], 21);

	    if(elm->chainId != ajStrGetCharPos(pdbfile->lines[i], 32))
	    {
		ajFmtPrintF(flog, "%-15s%c %c\n", "SECTWOCHN", elm->chainId,
			    ajStrGetCharPos(pdbfile->lines[i], 32));
		ElementDel(&elm);
		continue;	
	    }
	    
	    
	    /* Check for beta-barrels - where the first and last strands 
	       are identical requiring us to ignore the last strand */
	    
	    if(doneFirstStrand)
	    {
		if(ajStrMatchS(elm->initResName, FirstStrand->initResName) &&
		   ajStrMatchS(elm->endResName, FirstStrand->endResName)   &&
		   ajStrMatchS(elm->initSeqNum, FirstStrand->initSeqNum)   &&
		   ajStrMatchS(elm->endSeqNum, FirstStrand->endSeqNum))
		{			
		    ElementDel(&elm);
		    continue;
		}
	    }
	    
	    /* Check that all records are present and flag an error if 
	       they're not */
	    if(MAJSTRGETLEN(elm->initResName) && MAJSTRGETLEN(elm->initSeqNum)
	       && 
	       MAJSTRGETLEN(elm->endResName) && MAJSTRGETLEN(elm->endSeqNum)
	       && 
	       MAJSTRGETLEN(elm->elementId))	    
	    {
		ajListPushAppend(listelms, elm); 
	    }
	    else
	    {	
		ajFmtPrintF(flog, "%-15s%d\n", "SECMISS", i+1);
		ElementDel(&elm);
		continue;	

	    }
	    
	    ajStrAssignS(&LastSheetId, elm->elementId);
	    FirstStrand=elm;
	    doneFirstStrand=ajTrue;
	}
	else if(ajStrPrefixC(pdbfile->lines[i],"TURN"))
	{
	    doneFirstStrand=ajFalse;

	    elm = ElementNew();

	    ajStrAssignSubS(&tmpstr, pdbfile->lines[i], 7, 9);
	    ajStrRemoveWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->elementNum);

	    ajStrAssignSubS(&elm->elementId, pdbfile->lines[i], 11, 13);
	    ajStrRemoveWhite(&elm->elementId);

	    elm->elementType = 'T';

	    ajStrAssignSubS(&elm->initResName, pdbfile->lines[i], 15, 17);
	    ajStrRemoveWhite(&elm->initResName);

	    ajStrAssignSubS(&elm->initSeqNum, pdbfile->lines[i], 20, 24);
	    ajStrRemoveWhite(&elm->initSeqNum);

	    ajStrAssignSubS(&elm->endResName, pdbfile->lines[i], 26, 28);
	    ajStrRemoveWhite(&elm->endResName);

	    ajStrAssignSubS(&elm->endSeqNum, pdbfile->lines[i], 31, 35);
	    ajStrRemoveWhite(&elm->endSeqNum);

	    elm->chainId = ajStrGetCharPos(pdbfile->lines[i], 19);

	    if(elm->chainId != ajStrGetCharPos(pdbfile->lines[i], 30))
	    {
		ajFmtPrintF(flog, "%-15s%c %c\n", "SECTWOCHN", elm->chainId,
			    ajStrGetCharPos(pdbfile->lines[i], 30));
		ElementDel(&elm);
		continue;	
	    }	


	    /* Check that all records are present and flag an error if 
	       they're not */
	    if(MAJSTRGETLEN(elm->initResName) && MAJSTRGETLEN(elm->initSeqNum) 
	       && MAJSTRGETLEN(elm->endResName) && MAJSTRGETLEN(elm->endSeqNum)
	       && MAJSTRGETLEN(elm->elementId))	    
	    {
		ajListPushAppend(listelms, elm);
	    }
	    else	 
	    {
		ajFmtPrintF(flog, "%-15s%d\n", "SECMISS", i+1);
		ElementDel(&elm);
		continue;	
	    }
	}
    }



    
    /* Write array in Elements structure */
    (*elms)->n=ajListToarray(listelms, (void ***) &(*elms)->elms);
        

    /* Generate diagnostics and set defaults */
    if((ajStrGetLen(pdbfile->compnd) == 0))
    {
	ajStrAssignC(&pdbfile->compnd, ".");
	ajFmtPrintF(flog, "%-15s\n", "NOCOMPND");  
    }

    if((ajStrGetLen(pdbfile->source) == 0))
    {
	ajStrAssignC(&pdbfile->source, ".");
	ajFmtPrintF(flog, "%-15s\n", "NOSOURCE");
    }
    
    if((pdbfile->method==ajNMR) && (pdbfile->modcnt==0))
	ajFmtPrintF(flog, "%-15s\n", "NOMODEL");


    if(!E_FPZERO(pdbfile->reso,U_FEPS) && pdbfile->modcnt)
    {
	ajFmtPrintF(flog, "%-15s\n", "RESOLMOD");
	pdbfile->method=ajNMR;
    }
    


    /* Every pdb file is considered to have at least one model */
    if(pdbfile->modcnt==0)
    {
	pdbfile->modcnt=1;
	pdbfile->nomod=ajTrue;
    }
    


    if(!resolfound)
    {
	pdbfile->reso=0;
	pdbfile->method=ajNMR;
	ajFmtPrintF(flog, "%-15s\n","NORESOLUTION");
    }



    if(!seqresfound)
    {
	ajWarn("No SEQRES record found in raw pdb file");
	ajFmtPrintF(flog, "%-15s\n", "NOSEQRES");
	
	/* Free memory and return */
	ajListFree(&listelms);
	ajStrDel(&LastSheetId);
	ajStrDel(&tmpstr);
	ajStrDel(&seqres);

	return ajFalse;
    }


    

    if(!donefirstatom)    
    {
	ajWarn("No ATOM record found in raw pdb file");
	ajFmtPrintF(flog, "%-15s\n", "NOATOM");
	
	/* Free memory and return */
	ajListFree(&listelms);
	ajStrDel(&LastSheetId);    
	ajStrDel(&tmpstr);
	ajStrDel(&seqres);

	return ajFalse;
    }


    AJCNEW0(pdbfile->nligands, pdbfile->nchains);
    AJCNEW0(pdbfile->nres, pdbfile->nchains);
    AJCNEW0(pdbfile->numHelices, pdbfile->nchains);
    AJCNEW0(pdbfile->numStrands, pdbfile->nchains);
    AJCNEW0(pdbfile->numSheets, pdbfile->nchains);
    AJCNEW0(pdbfile->numTurns, pdbfile->nchains);
    AJCNEW0(pdbfile->chainok, pdbfile->nchains);
    AJCNEW0(pdbfile->resn1ok, pdbfile->nchains);
    


    for(i=0;i<pdbfile->nchains;i++)
    {
	pdbfile->chainok[i]=ajTrue;
	pdbfile->resn1ok[i]=ajTrue;
    }


    /* Free memory and return */
    ajStrDel(&LastSheetId);
    ajListFree(&listelms);

    ajStrDel(&tmpstr);
    ajStrDel(&seqres);

    return ajTrue;
}




/* @funcstatic  SeqresToSequence ********************************************
**
** Reads a string containing a SEQRES sequence  (e.g. "ALA ALA LEU" ) and 
** writes a string containing a normal sequence (e.g. "AAL").
**
** @param [r] seqres   [const AjPStr]   SEQRES sequence
** @param [w] seq      [AjPStr *] Output sequence
** @param [r] camask   [AjBool]   Whether to ignore residues which do not 
** have a C-alpha atom, these are defined as ACE, FOR and NH2 groups.
** @param [w] len      [ajint *]  Length of sequence INCLUDING ACE, FOR and 
**                                NH2 groups.
** 
** @return [AjBool] ajTrue on success, ajFalse otherwise.
** @@
****************************************************************************/

static AjBool SeqresToSequence(const AjPStr seqres,
			       AjPStr *seq, AjBool camask, 
			       ajint *len)
{
    const  AjPStr aa3  =NULL;
    char aa1;
    ajint nrem=0;  /* No. 'residues' that were removed */
    

    /* Check args */   
    if(!seqres || !seq)
    {
	ajWarn("Bad args passed to SeqresToSequence\n");

	return ajFalse;
    }

    /* Allocate memory */
    
    if((aa3=ajStrParseC(seqres, " \n")))
	/* Parse seqres string */
	do
	{
	    if(ajStrMatchC(aa3, "FOR") ||
	       ajStrMatchC(aa3, "ACE") ||
	       ajStrMatchC(aa3, "NH2"))
	    {
		if(camask)
		{	
		    nrem++;
		    continue;
		}
	    }
	    
	    ajResidueFromTriplet(aa3, &aa1);
	    ajStrAppendK(seq, aa1);
	}
	while((aa3=ajStrParseC(NULL, " \n")));
    else
	return ajFalse;
    
    *len = ajStrGetLen(*seq)+nrem;

    return ajTrue;
}




/* @funcstatic  CheckChains *************************************************
**
** Reads a Pdbfile object and checks whether chains from the SEQRES records 
** (i) use unique chain ids, (ii) do not use an id of a space (' ') alongside 
** non-space chain ids and (iii) contain at least the user-defined threshold 
** number of amino acid residues. If any of these conditions are not met then 
** the chain is discarded (chainok array is set to ajFalse).
**
** The chainok array is written.
** 
**
** @param [w] pdbfile         [AjPPdbfile]   Pdbfile object
** @param [u] flog            [AjPFile]      Pointer to log file (build 
**                                           diagnostics).
** @param [r] min_chain_size  [ajint]        Minimum number of amino acids in 
**                                           a chain.
** 
** @return [AjBool]  True if SEQRES records contained at least one protein 
** chain, False otherwise.
** @@
****************************************************************************/

static AjBool CheckChains(AjPPdbfile pdbfile, AjPFile flog, 
			  ajint min_chain_size)
{
    ajint    i=0;			/* Loop counter */
    ajint    j=0;			/* Loop counter */
    AjIStr   iter=NULL;			/* Iterator for sequence strings */
    ajint    aacnt=0;			
    /* Counter for no. of amino acids in sequence strings */

    AjBool   ok=ajFalse;
    /* Flag which is True if amino acid chains are found  in the 
       SEQRES records */

    char     id1=' ';			/* Chain id */
    char     id2=' ';			/* Chain id */
    AjBool   iderr=ajFalse;	
    /* ajTrue if both a space and a character are used as chain 
       id's in the same file */




    
    /* Check args */
    if(!pdbfile || !flog)
    {
	ajWarn("Bad args passed to CheckChains\n");

	return ajFalse;
    }
    
    
    /* Report problems with chain id's */
    for(i=0;i<pdbfile->nchains; i++)
    {
	id1 = ajChararrGet(pdbfile->chid, i);

	for(j=i+1; j<pdbfile->nchains; j++)
	{
	    if(id1==(id2=ajChararrGet(pdbfile->chid, j)))
	    {
		ajFmtPrintF(flog, "%-15s%d (%c) %d (%c)\n", "CHAINIDS", i+1, 
			    ajChararrGet(pdbfile->chid, i), j+1, 
			    ajChararrGet(pdbfile->chid, j));
		
		pdbfile->chainok[i] = ajFalse;
		pdbfile->chainok[j] = ajFalse;
	    }
	    
	    if((((id1 == ' ')&&(id2 != ' '))||((id2 == ' ')&&(id1 != ' '))) 
	       && (!(iderr)))
	    {
		ajFmtPrintF(flog, "%-15s\n", "CHAINIDSPC");
		iderr=ajTrue;
	    }
	    
	}
    }


    /* Report problems with non-protein chains */
    for(i=0;i<pdbfile->nchains; i++)
    {
	if(!pdbfile->chainok[i])
	    continue;
	

	aacnt=0;
	iter=ajStrIterNew(pdbfile->seqres[i]);

	if(toupper((int) ajStrIterGetK(iter)) != 'X')
	    ++aacnt;
	
	while(ajStrIterNext(iter))
	    if(toupper((int) ajStrIterGetK(iter)) != 'X')
		if(++aacnt >= min_chain_size)
		    break;

	ajStrIterDel(&iter);
	
	
	if(aacnt==0)
	{
	    ajFmtPrintF(flog, "%-15s%d (%c)\n", "SEQRESNOAA", i+1, 
			ajChararrGet(pdbfile->chid, i));
	    pdbfile->chainok[i] = ajFalse;
	}
	else if(aacnt < min_chain_size)
	{	
	    ajFmtPrintF(flog, "%-15s%d (%c)\n", "SEQRESFEWAA", i+1, 
			ajChararrGet(pdbfile->chid, i));
	    
	    pdbfile->chainok[i] = ajFalse;


	}
	else 
	{
	    ok = ajTrue;
	}
    }

    /* Return now if no protein chains are found */
    if(!ok)
    {
	ajWarn("No protein chains found in raw pdb file");
	ajFmtPrintF(flog, "%-15s\n", "NOPROTEINS");

	return ajFalse;
    }
    

    return ajTrue;
}




/* @funcstatic  CheckTer ****************************************************
**
** Reads a Pdbfile object and checks whether the expected number of TER 
** and MODEL records are present. Any unwanted records (e.g. TER records that
** delimit fragments of chain digests and duplicate MODEL records) are 
** discarded (the linetype array for the lines are set to PDBPARSE_IGNORE).
** 
** The linetype array and modcnt variable may be modified. The toofewter
** element is written.
** The value of modcnt is reduced by 1 for each MODEL record that was masked
** but this is not done for tercnt.
**
** @param [w] pdbfile [AjPPdbfile] Pdbfile object
** @param [u] flog    [AjPFile]     Pointer to log file (build diagnostics)
** 
** @return [AjBool]  True on success, False otherwise.
** @@
****************************************************************************/

static AjBool CheckTer(AjPPdbfile pdbfile, AjPFile flog)
{
    AjBool toomany=ajFalse;
    AjBool toofew =ajFalse;
    ajint    i=0;			/* Loop counter */
    AjPStr aa1=NULL;
    AjPStr aa2=NULL;
    char   aa=' ';




    
    /* Check args */
    if(!pdbfile || !flog)
    {
	ajWarn("Bad args passed to CheckTer\n");

	return ajFalse;
    }

    /* Allocate memory */
    aa1=ajStrNew();
    aa2=ajStrNew();
    
    
    /* Report problems with TER records */
    if(!pdbfile->tercnt)
	ajFmtPrintF(flog, "%-15s\n", "TERNONE");
    else
    {
	if(pdbfile->method==ajNMR)
	{
	    if(pdbfile->tercnt > (pdbfile->nchains * 
				     pdbfile->modcnt))
		toomany=ajTrue;
	    else if(pdbfile->tercnt < (pdbfile->nchains * 
					  pdbfile->modcnt))
		toofew=ajTrue;
	}
	else
	{
	    if(pdbfile->tercnt > pdbfile->nchains)
		toomany=ajTrue;
	    else if(pdbfile->tercnt < pdbfile->nchains)
		toofew=ajTrue;
	}
    }	


    /* Report diagnostics */
    if(toomany)
	ajFmtPrintF(flog, "%-15s\n", "TERTOOMANY");
    else if(toofew)
    {
	ajFmtPrintF(flog, "%-15s\n", "TERTOOFEW");
	pdbfile->toofewter=ajTrue;
    }
    

    /* Mask out the extra TER records */ 
    if(toomany)
    {
	for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
	{
	    /* This is a TER record. Mask it out if it is flanked
	       by ATOM or HETATM records for AMINO ACIDS and with 
	       identical chain ids. */
	    if(pdbfile->linetype[i]==PDBPARSE_TER)
	    {
		if((pdbfile->linetype[i-1]==PDBPARSE_COORD) && 
		   (pdbfile->linetype[i+1]==PDBPARSE_COORD) &&
		   (ajStrGetCharPos(pdbfile->lines[i-1], POS_CHID)
		    ==ajStrGetCharPos(pdbfile->lines[i+1], POS_CHID)))
		{
		    ajStrAssignSubS(&aa1, pdbfile->lines[i-1], 17, 19);
		    ajStrAssignSubS(&aa2, pdbfile->lines[i+1], 17, 19);

		    if((ajResidueFromTriplet(aa1, &aa)) && 
		       (ajResidueFromTriplet(aa2, &aa)))
		    {
			pdbfile->linetype[i]=PDBPARSE_IGNORE;

		    }
			
		}
		
	    }
	    
		
	}
    }


    /* Check for duplicate MODEL records */
    for(i=pdbfile->idxfirst+1;
	i<pdbfile->nlines;i++)
    {
	if((pdbfile->linetype[i-1]==PDBPARSE_MODEL) && 
	   (pdbfile->linetype[i]==PDBPARSE_MODEL))
	{
	    pdbfile->linetype[i-1]=PDBPARSE_IGNORE;
	    pdbfile->modcnt--;
	    ajFmtPrintF(flog, "%-15s%d\n", "MODELDUP", i+1);


	}
    }
    

    /*Tidy up and return */
    ajStrDel(&aa1);
    ajStrDel(&aa2);

    return ajTrue;
}




/* @funcstatic  NumberChains ************************************************
**
** Reads a Pdbfile object and assigns each ATOM or HETATM record to a specific
** chain and model. Lines containing coordinates for water molecules and 
** other non-protein groups ("heterogens") are identified.  Water molecules are
** uniquely associated with a whole model whereas other non-protein groups are 
** associated with a unique chain if possible. If this is not possible they 
** are assigned a unique group number. 
**
** The modn, chnn and gpn arrays are written. The linetype array is modified.
** The nligands array (count of groups associated with a chain), ngroups 
** element (count of groups not associated with a chain) and gpid (identifiers
** of these later groups) are written.
** 
** linetype array
** The linetype array is written with a value of PDBPARSE_COORDWAT for lines 
** containing coordinates for water, and to PDBPARSE_COORDHET or 
** PDBPARSE_COORDGP for non-protein groups that, respectively, could or could 
** not be uniquely associated with a chain.
** 
**
** @param [w] pdbfile  [AjPPdbfile] Pdbfile object
** @param [u] flog     [AjPFile]      Log file (build diagnostics)
** 
** @return [AjBool]  True on success, False otherwise
** @@
****************************************************************************/

static AjBool NumberChains(AjPPdbfile pdbfile, AjPFile flog)
{
    ajint  i=0;			/* Loop counter */
    ajint  j=0;			/* Loop counter */
    ajint  mod=0;		/* Model number */
    char   id=' ';		/* Chain id */

    AjBool done=ajFalse;  /*True if we have assigned a chain id for this 
			    line */
    ajint  this=0;	  /* Chain number of last line read in */
    ajint  chn=0;	  /* Chain number as index (starting from 0) */
    AjPInt gpns=NULL;	  /* Gives the correct group number 
			     for groups that could not be identified as 
			     belonging to a chain, in cases where a single 
			     chain only is present in the file */
    ajint  gpn=0;	  /* Current group number */
    ajint  offset=0;	  /* Offset for finding correct value for gpns (for 
			     use with files with a single chain only */
    AjBool *chndone=NULL;   /* Array whose elements are TRUE if we have 
			       already read a line in belonging to the 
			       appropriate chain for this model*/
    AjPStr *htype;   	    /* Array holding the residue type of the last 
			       heterogen read in for the appropriate chain */
    


    /* Check args */
    if(!pdbfile || !flog)
    {
	ajWarn("Bad args passed to NumberChains\n");

	return ajFalse;
    }

    /* Allocate memory */
    gpns = ajIntNew();
    
    AJCNEW0(htype, pdbfile->nchains);

    for(i=0;i<pdbfile->nchains; i++)
	htype[i]=ajStrNew();
    

    AJCNEW0(chndone, pdbfile->nchains);

    for(i=0;i<pdbfile->nchains; i++)
	chndone[i]=ajFalse;



    if(((pdbfile->method==ajNMR) && pdbfile->nomod) ||
       (pdbfile->method==ajXRAY))
	mod=1;
    
    

    for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	/* JONNEW Lines may already have been assigned to PDBPARSE_COORDWAT
	   in FirstPass function,
	   so we need to check here to ensure model number gets assigned */
	if((pdbfile->linetype[i]==PDBPARSE_COORD) ||
	   (pdbfile->linetype[i]==PDBPARSE_COORDWAT))
	{

	    pdbfile->modn[i]=mod;


	    /*Assign appropriate chain number to any ATOM or HETATM 	
	      line which has an id exhibited in the SEQRES records. */
	    done=ajFalse;
	    id=ajStrGetCharPos(pdbfile->lines[i], POS_CHID);

	    for(j=0;j<pdbfile->nchains; j++)
	    {
		if(ajChararrGet(pdbfile->chid, j) == id)
		{
		    pdbfile->chnn[i]=j+1;
		    chn=pdbfile->chnn[i]-1;
		    this=j+1;



		    if(chndone[this-1])
		    {
			/* Mark up water coordinates */
			if(ajStrMatchC(pdbfile->rtype[i], "HOH"))
			    pdbfile->linetype[i]=PDBPARSE_COORDWAT;
			else
			{
			    /* Mark up ligand coordinates */
			    pdbfile->linetype[i]=PDBPARSE_COORDHET;



			    /* New heterogen */
			    if(!ajStrMatchS(htype[chn], pdbfile->rtype[i]))
			    {
				offset++;
				pdbfile->nligands[chn]++;
				pdbfile->gpn[i] = pdbfile->nligands[chn];
				ajStrAssignS(&htype[chn],
					     pdbfile->rtype[i]);
			    }
			    /* More atoms of the same heterogen */
			    else
			    {
				pdbfile->gpn[i] = pdbfile->nligands[chn];
			    }
			}	
		    }
/* Unused
		    else 
			doneter=ajFalse; 
*/

		    done=ajTrue;
		    break;
		}
	    }
	    
	    
	    if(!done)
	    {		
		/* Any ATOM or HETATM
		   record with a whitespace as chain id and which has not
		   already been assigned belongs to the chain immediately 
		   preceding it. Assign these lines as NON_PROTEIN_CHAIN
		   lines.  */

		if(id==' ')
		{
		    /* This won't be set until we've read in at least one
		       coordinate line for protein chain, so if this==0, 
		       set it to 1 (first chain). This prevents problems
		       for 1qjh.pxyz*/
		  
		  if(this==0)
		    this=1;

		    pdbfile->chnn[i]=this;
		    chn=pdbfile->chnn[i]-1;
		  
		    /* Mark up water coordinates */
		    if(ajStrMatchC(pdbfile->rtype[i], "HOH"))
			pdbfile->linetype[i]=PDBPARSE_COORDWAT;
		    else
		    {
			/* Mark up ligand coordinates */
			  
			pdbfile->linetype[i]=PDBPARSE_COORDHET;



    
			/* New ligand */
			if(!ajStrMatchS(htype[chn], pdbfile->rtype[i]))
			{

			    offset++;
			    pdbfile->nligands[chn]++;
			    pdbfile->gpn[i] = pdbfile->nligands[chn];
			    ajStrAssignS(&htype[chn], pdbfile->rtype[i]);
			}	
			/* More atoms of the same heterogen */
			else
			{
			    pdbfile->gpn[i] = pdbfile->nligands[chn];
			}
		    }
		}
		else
		    /* Assign any ATOM or HETATM records with a non-
		       whitespace chain id that does not appear in the 
		       SEQRES records as a  NON_PROTEIN_CHAIN. Record 
		       the chain id's used and assign a GROUP NUMBER 
		       as appropriate.*/
		{
		    /* Mark up water coordinates */
		    if(ajStrMatchC(pdbfile->rtype[i], "HOH"))
			pdbfile->linetype[i]=PDBPARSE_COORDWAT;
		    else
		    {
			/* If there is a single chain only then the group is
			   of course associated with that chain */
			if(pdbfile->nchains==1)
			{
			    /* Mark up ligand coordinates */
			    pdbfile->linetype[i]=PDBPARSE_COORDHET;
			    pdbfile->chnn[i]=1;
			    

			    for(done=ajFalse, j=0;j<pdbfile->ngroups; j++)
				if(ajChararrGet(pdbfile->gpid, j) == id)
				{
				    pdbfile->gpn[i]=ajIntGet(gpns, j);
				    done=ajTrue;
				    break;
				}

			    if(!done)
			    {
				ajIntPut(&gpns, j, (gpn=j+1+offset));
				pdbfile->gpn[i]=gpn;

				/* NO - this code would be used only if the
				   group could not be associated with a chain.
				ajChararrPut(&(pdbfile->gpid), 
					     pdbfile->ngroups, id);
				pdbfile->ngroups++;
				*/

				/* Increment number of ligands and reset last 
				   ligand type read in */
				pdbfile->nligands[0]++;
				ajStrAssignC(&htype[0], " ");
			    }
			}
			else
			{
			    /* Mark up ligand coordinates */
			    pdbfile->linetype[i]=PDBPARSE_COORDGP;
			
			    for(done=ajFalse, j=0;j<pdbfile->ngroups; j++)
				if(ajChararrGet(pdbfile->gpid, j) == id)
				{
				    pdbfile->gpn[i]=j+1;
				    done=ajTrue;
				    break;
				}

			    if(!done)
			    {
				ajChararrPut(&(pdbfile->gpid), 
					     pdbfile->ngroups, id);
				pdbfile->ngroups++;
				pdbfile->gpn[i]=j+1;
			    }
			}
		    }
		}
	    }
	}
	else if(pdbfile->linetype[i]==PDBPARSE_MODEL)
	{
	    mod++;

/* 	    doneter=ajFalse;  Unused variable */
	    for(j=0;j<pdbfile->nchains; j++)
		chndone[j]=ajFalse;

/*	    doneoneter=ajFalse; Unused variable */
	}
	else if(pdbfile->linetype[i]==PDBPARSE_TER)
	{
	    chndone[this-1]=ajTrue;
	    

/*	    prev = this; Unused variable */
/*	    doneoneter=ajTrue; Unused variable */
/*	    doneter=ajTrue; Unused variable */
	}	

	

	/* 
	   Check for missing TER records.
	   Where chains are not separated by TER records
	   (the chain id changes from line to line without an intervening TER 
	   record and both chain id's are not whitespace).
	   Where ATOM and HETATM groups are not separated 
	   by TER records (a chain id is given on one line, a whitespace chain
	   id is given on the next line, and there is no intervening TER 
	   record).
	   
	   This code is identical to a fragment from CheckTer.
	   
	   Note that chndone only is modified. If the code in the function 
	   was made to use prev, doneoneter, doneter then the code below would 
	   also have to be modified
	   
	   Only do this now where there aren't enough TER records in the file.
	   Without this check, it was failing for cases where the order of 
	   chains is inconsistent (see around line 4095 of pdb1cm4.ent)
	   */

	
	if(pdbfile->toofewter)
	{
	    
	    if(i>pdbfile->idxfirst)
		if(pdbfile->linetype[i-1]==PDBPARSE_COORD  	&& 
		   ((pdbfile->linetype[i]==PDBPARSE_COORD)||
		    (pdbfile->linetype[i]==PDBPARSE_COORDHET)) &&
		   ((ajStrGetCharPos(pdbfile->lines[i-1], POS_CHID)) !=
		    ajStrGetCharPos(pdbfile->lines[i], POS_CHID)))
		{
		    for(j=0;j<pdbfile->nchains; j++)
			if(ajChararrGet(pdbfile->chid, j)
			   ==ajStrGetCharPos(pdbfile->lines[i-1], POS_CHID))
			{	
			    chndone[j]=ajTrue;	    
			    break;
			}
		}
	}
    }	


    /* The above code cannot cope for cases where the ATOM and HETATM records 
       use the same (or no) chain identifier and are not separated by a TER 
       record (e.g. 1rbp) 
       
       For files with less than the expected number of TER records, 
       check again for COORDHET lines, which are identified as 
       (i)  a line beginning with a HETATM record with the same chain 
       identifier but lower residue number than the preceding line, or
       JONNEW
       (ii) a line beginning with a HETATM record which is not followed
       anywhere in the file by an ATOM record with the same chain identifier 
       (from the PDB record) or number (assigned by parser)
       */

    for(i=0;i<pdbfile->nchains;i++)
	ajStrAssignC(&htype[i], "\0");
    

    if(pdbfile->tercnt < (pdbfile->nchains * pdbfile->modcnt))
	for(i=pdbfile->idxfirst+1;i<pdbfile->nlines;i++)
	{
	    chn=pdbfile->chnn[i]-1;

	    if(pdbfile->linetype[i-1]==PDBPARSE_COORD  	&& 
	       pdbfile->linetype[i]==PDBPARSE_COORD &&
	       ((ajStrGetCharPos(pdbfile->lines[i-1], POS_CHID)) ==
		ajStrGetCharPos(pdbfile->lines[i], POS_CHID)))
		if(ajStrPrefixC(pdbfile->lines[i],"HETATM"))
		    if((pdbfile->resn1[i]<pdbfile->resn1[i-1]) ||
		       NoMoreAtoms(pdbfile, i))
		    /* if(pdbfile->resn1[i]<pdbfile->resn1[i-1]) */
			while((ajStrPrefixC(pdbfile->lines[i],"HETATM")))
			{			
			    if(ajStrMatchC(pdbfile->rtype[i], "HOH"))
				pdbfile->linetype[i]=PDBPARSE_COORDWAT;
			    else
			    {
				pdbfile->linetype[i]=PDBPARSE_COORDHET;

				/* New heterogen */
				if(!ajStrMatchS(htype[chn], 
					       pdbfile->rtype[i]))
				{
				    offset++;
				    pdbfile->nligands[chn]++;
				    pdbfile->gpn[i] 
					= pdbfile->nligands[chn];
				    ajStrAssignS(&htype[chn], 
					      pdbfile->rtype[i]);
				}	
				/* More atoms of the same heterogen */
				else
				{
				    pdbfile->gpn[i] 
					= pdbfile->nligands[chn];
				}
			    }
			    i++;
			}
	}
    
    /* For files with a single chain only, set the number of groups
       that could not be associated with a chain to zero */
    if(pdbfile->nchains==1)
	{
	    /* We might need to add ngroups to nligands[0] */
	    if(pdbfile->ngroups != 0)
		ajFatal("Must check ngroups versus nligands in the file");
	    pdbfile->ngroups = 0;
	}
    


    /* Tidy up and return */
    for(i=0;i<pdbfile->nchains; i++)
	ajStrDel(&htype[i]);

    AJFREE(htype);
    ajIntDel(&gpns);
    AJFREE(chndone);

    return ajTrue;
}




/* @funcstatic  NoMoreAtoms ************************************************
**
** This function is called by function <NumberChains> to identify ligands 
** (COORDHET lines) in files with less than the expected number of TER 
** records.
** These are identified here by a line beginning with a HETATM record which
** is not followed by an ATOM record with the same chain identifier (from 
** the PDB record) or number (assigned by parser). The function returns if
** a line of a different chain is found or at the first non-ATOM/HETAM line. 
** Additional processing is done in <NumberChains> itself.
**
** @param [w] pdbfile  [AjPPdbfile] Pdbfile object
** @param [r] linen    [ajint] Line number
** 
** @return [AjBool]  True (no more atoms), False otherwise
** @@
****************************************************************************/

static AjBool NoMoreAtoms(AjPPdbfile pdbfile, ajint linen)
{
    ajint i=0;
    
    for(i=linen+1;i<pdbfile->nlines;i++)
    {
	if((ajStrPrefixC(pdbfile->lines[i],"ATOM")))
	{
	    /* Same chain */
	    if((ajStrGetCharPos(pdbfile->lines[linen], POS_CHID) ==
		ajStrGetCharPos(pdbfile->lines[i], POS_CHID)) ||
	       (pdbfile->chnn[linen] == pdbfile->chnn[i]))
		return ajFalse;
	    else
		/* Different chain */
		return ajTrue;
	}
	else if((ajStrPrefixC(pdbfile->lines[i],"HETATM")))
	{
	    /* Different chain */
	    if((ajStrGetCharPos(pdbfile->lines[linen], POS_CHID) !=
		ajStrGetCharPos(pdbfile->lines[i], POS_CHID)) ||
	       (pdbfile->chnn[linen] != pdbfile->chnn[i]))
		return ajTrue;
	}
	else
	    /* Different chain or near EOF */
	    return ajTrue;
    }
    
    return ajTrue;
}




/* @funcstatic MaskChains *****************************************************
**
** Reads a Pdbfile object and checks to see whether the ATOM records for 
** each chain contain sufficient amino acids. Any chains with insufficient 
** amino acids either in the SEQRES or ATOM records, or with ambiguous chain 
** id's are discarded. Optionally, amino acid residues and non-amino 
** acid groups (e.g. ACE, NH2 etc) in protein chains with no CA atom are also 
** discarded (the linetype array for the lines are set to PDBPARSE_IGNORE).  
** For non-amino acid groups,  the corresponding characters are removed from 
** the sequence derived from the SEQRES records.  Coordinate data and atom 
** type are parsed for each atom.  Optionally, amino acids or groups in 
** protein chains with a single atom only are also discarded.
**
** 
** Checks whether chains from the ATOM records contain at least the
** user-defined threshold number of amino acid residues. If not then the chain
** is discarded  (chainok array is set to ajFalse). If NO chains with 
** sufficient residues are found, a "NOPROTEINS" error is generated and
** ajFalse is returned.
** 
** Writes the x,y,z,o,b and atype elements of a Pdbfile object.  The linetype,
** and possibly seqres, seqresful and nres arrays are modified.
** 
** linetype array
** Coordinate data are extracted for lines of linetype PDBPARSE_COORD, 
** PDBPARSE_COORDHET and PDBPARSE_COORDGP.
**
** seqres & seqresful arrays
** Three-letter codes of any groups that are (i) not standard amino acids and 
** (ii) which do not contain a CA atom are removed from the seqres sequence 
** if the <camask> is set. The seqresful array is an intermediate array to 
** achieve this.
** 
** @param [w] pdbfile    [AjPPdbfile] Pdbfile object
** @param [u] flog       [AjPFile]     Log file (build diagnostics)
** @param [r] min_chain_size  [ajint]  Min. no. of amino acids in a chain
** @param [r] camask          [AjBool] Whether to mask non-amino acid 
**                                     residues within protein chains which 
**                                     do not have a C-alpha atom (remove them
**                                     from the seqres sequence and set the 
**                                     linetype array for the lines 
**                                     to PDBPARSE_IGNORE).
** @param [r] camask1         [AjBool]  Whether to mask amino acid residues 
**                                     within protein chains which do not have 
**                                     a C-alpha atom (set the linetype 
**                                     array for the lines to PDBPARSE_IGNORE).
** @param [r] atommask        [AjBool] Whether to mask residues or groups 
**                                     with a single atom only.
** 
** @return [AjBool]  True on success, False otherwise
** @@
****************************************************************************/

static AjBool MaskChains(AjPPdbfile pdbfile, AjPFile flog, 
			 ajint min_chain_size, 
			 AjBool camask, AjBool camask1, 
			 AjBool atommask)
{
    ajint  i=0;			/*Loop counter */
    ajint  j=0;			/*Loop counter */
    AjPStr aa3=NULL;		/*Amino acid */
    ajint  rcnt=0;		/* Residue count */
    ajint  acnt=0;		/* Atom count */
    ajint  modcnt=0;		/*Count of MODEL records */
    ajint  lastatm=0;		/*Line number of last coordinate line read in*/
    ajint  firstatm=0;		/*Line number of coordinate line for first 
				  atom of residue */
    AjBool noca=ajFalse;	/*True if this residue does not contain a 
				  CA atom */
    ajint  lastchn=0;		/*Chain number of last line read in*/
    AjBool *chainok;		/*Array of flags which are True if a chain 
				  in the SEQRES records is found in the ATOM 
				  records */
    char aa1=' ';		/*Amino acid id */
    AjPStr lastrn=NULL;		/*Number of last residue read in */
    AjBool msgdone=ajFalse;	/* Flag for message reporting */
    AjPStr sub=NULL;
    AjPStr tmpseq=NULL;
    ajint    lenful=0;          /* Length of SEQRES sequence including
				   ACE, FOR & NH2 groups that might be 
				   discarded by the call to 
				   SeqresToSequence */
    ajuint ipos = 0;
    char   tmp=' ';             
    AjBool odd=ajFalse;         /* Whether the current residue / group
				   is of unknown type */
    AjBool ok =ajFalse;         /* True if the file, after processing
				   by this function, is found
				   to contain at least one chain for which
				   chainok == ajTrue */
    
    



    /* Check args */
    if(!pdbfile || !flog)
    {
	ajWarn("Bad args passed to MaskChains\n");

	return ajFalse;
    }
    
    /* Allocate memory */
    AJCNEW0(chainok, pdbfile->nchains);
    for(i=0;i<pdbfile->nchains;i++)
	chainok[i]=ajFalse;
    
    aa3=ajStrNew();
    lastrn=ajStrNew();
    sub=ajStrNew();
    ajStrAssignClear(&sub);
    
    
    firstatm=lastatm=pdbfile->idxfirst;
    
    
    for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	if((pdbfile->linetype[i]==PDBPARSE_COORD)    ||
	   (pdbfile->linetype[i]==PDBPARSE_COORDHET) ||
	   (pdbfile->linetype[i]==PDBPARSE_COORDGP)  ||
	   (pdbfile->linetype[i]==PDBPARSE_COORDWAT))
	{	

	    /* Assign x, y, z, o, b data */
	    /* Replace this with code so that internals of structure 
	       are not probed (when code becomes available)*/
	    if( (sscanf(&(pdbfile->lines[i]->Ptr[28]), "%f %f %f %f", 
			&(pdbfile->x[i]), 
			&(pdbfile->y[i]), 
			&(pdbfile->z[i]), 
			&(pdbfile->o[i]))) != 4)
		ajFatal("Scan error in MaskChains\n"
			"Email jison@hgmp.mrc.ac.uk");

	    if(!sscanf(&(pdbfile->lines[i]->Ptr[60]), "%f", 
		       &(pdbfile->b[i])))
		ajFatal("Scan error in MaskChains\n"
			"Email jison@hgmp.mrc.ac.uk");
	    	    
	    /* Usually position 12 is used for the alternative position 
	       indicator (taken in the code below to be indicated by a 
	       number) for atoms, but occasionally can be incorrectly 
	       used for the atom type itself (indicated in the code below 
	       by a character in pos 12). This code copes for both cases */
	    /* Assign atom type */
	    if(isalpha((int)pdbfile->lines[i]->Ptr[12]))
	    {
		ajStrAssignSubS(&pdbfile->atype[i], 
			    pdbfile->lines[i], 12, 15);
		ajStrRemoveWhite(&pdbfile->atype[i]);
		if(!msgdone)
		{
		    ajFmtPrintF(flog, "%-15s%d\n", "ATOMCOL12", i+1);
		    msgdone=ajTrue;
		}
	    }
	    else
	    {
		ajStrAssignSubS(&pdbfile->atype[i], 
			    pdbfile->lines[i], 13, 15);
		ajStrRemoveWhite(&pdbfile->atype[i]);
	    }
	   
	}
	
	


	if(pdbfile->linetype[i]==PDBPARSE_COORD)
	{	
	    /* Check whether CA atom is present */
	    if(!(ajStrCmpC(pdbfile->atype[i],  "CA")))
		noca=ajFalse;


	    if(pdbfile->chnn[i]<lastchn)
		ajFmtPrintF(flog, "%-15s%d\n", "CHAINORDER", i+1);

	    if(pdbfile->chnn[i]!=lastchn)
	    {
		rcnt=0;
		lastchn=pdbfile->chnn[i];
	    }
	    

	    /* If this is a new residue */
	    if(!ajStrMatchS(pdbfile->pdbn[i], lastrn))
	    {
		/* Mask coordinate lines where there are only a single atom */
		if(acnt==1)
		{
		    ajFmtPrintF(flog, "%-15s%d\n", "ATOMONEONLY", lastatm+1);

		    if(atommask)
			    pdbfile->linetype[lastatm]=PDBPARSE_IGNORE;
		}
		
		/* Mask coordinate lines for residues lacking a CA atom */
		if(noca)
		{
		    odd = (!(ajResidueFromTriplet(pdbfile->rtype[lastatm-1],
                                                  &tmp)));
		    
		    if((camask  && odd) ||
		       (camask1 && !odd))
			for(j=firstatm; j<=lastatm; j++)
			    pdbfile->linetype[j]=PDBPARSE_IGNORE; 

			/* Remove residues from SEQRES records */
		    if((camask  && odd))
		    {
			ipos = pdbfile->chnn[firstatm] - 1;
			ajStrExchangeSS(&pdbfile->seqresful[ipos], 
					pdbfile->rtype[firstatm], sub);
		    }
		    
		    if(firstatm == lastatm)
                        ajFmtPrintF(flog, "%-15s%d\n", "ATOMNOCA", 
                                    firstatm+1);
		    else
			ajFmtPrintF(flog, "%-15s%d %d\n", "ATOMNOCA", 
				    firstatm+1, lastatm+1);
		}
		
		
		/* Increment the residue counter if the code is recognised */
		if(ajResidueFromTriplet(pdbfile->rtype[i], &aa1))
		    rcnt++;	
		
		if(rcnt>=min_chain_size)
		    chainok[pdbfile->chnn[i]-1]=ajTrue;
		
		ajStrAssignS(&lastrn, pdbfile->pdbn[i]);
		
		/* Set count of atoms to zero, set the position of the first
		   atom and set flag for recognising CA atom*/
		acnt=1;
		firstatm=i;	
		
		if(!(ajStrCmpC(pdbfile->atype[i],  "CA")))
		    noca=ajFalse;
		else
		    noca=ajTrue;
	    }


	    /* Set the position for the last atom read in */
	    lastatm=i;
	    
	    
	    /* Increment the atom counter */
	    acnt++;
	}	
    	else if(pdbfile->linetype[i]==PDBPARSE_MODEL)
	{
	    rcnt=0;
	    lastchn=0;
	    
	    modcnt++;

	    if(modcnt!=1)
	    {
		for(j=0;j<pdbfile->nchains;j++)
		{	
		    /* Only bother reporting error messages if a message
		       about the SEQRES records not containing enough aa's
		       has not already been reported */
		    /* If pdbfile->chainok is False, leave it so  */
		    if(!pdbfile->chainok[j])
			continue;
		    else
		    {
			if(!chainok[j])
			{
			    pdbfile->chainok[j]=chainok[j];
			    ajFmtPrintF(flog, "%-15s%d (%c) %d\n", 
					"ATOMFEWAA", j+1,
					(ajChararrGet(pdbfile->chid, j)),
					modcnt);
			    			    
			}
		    }
		}
	    }

	}
    }

    /* Ensure that C-terminal residues are masked if necessary */
    /*	else if( (pdbfile->linetype[i]==PDBPARSE_TER)||
	(pdbfile->linetype[i]==PDBPARSE_ENDMDL)) 
	{*/
    if(noca)
    {
	odd = (!(ajResidueFromTriplet(pdbfile->rtype[lastatm-1],&tmp)));
	
	
	if((camask  && odd) ||
	   (camask1 && !odd))
	    for(j=firstatm; j<=lastatm; j++)
		pdbfile->linetype[j]=PDBPARSE_IGNORE; 
	
	/* Remove residues from SEQRES records */
	if((camask  && odd))
	{
	    ipos = pdbfile->chnn[firstatm] - 1;
	    ajStrExchangeSS(&pdbfile->seqresful[ipos], 
			    pdbfile->rtype[firstatm], sub);
	}
	
	if(firstatm == lastatm)
	{		
	    ajFmtPrintF(flog, "%-15s%d\n", "ATOMNOCA", firstatm+1);
	}	

	else
	    ajFmtPrintF(flog, "%-15s%d %d\n", "ATOMNOCA", 
			firstatm+1, lastatm+1);
    }
    
    /*	}	 */





    /* Write the new (masked) seqres sequences if necessary */
    if(camask)
    {
	for(i=0;i<pdbfile->nchains;i++)
	{	
	    tmpseq=ajStrNew();	 
	    
	    if(!SeqresToSequence(pdbfile->seqresful[i], 
				 &tmpseq, camask, &lenful))
		ajFatal("Sequence conversion error in "
			"FirstPass\nEmail jison@hgmp.mrc.ac.uk\n");

	    ajStrAssignS(&pdbfile->seqres[i], tmpseq);	
	    pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);
	    
	    ajStrDel(&tmpseq);
	}
    }
    
    

    /* Set modcnt to 1 for xray structures */
    if(!modcnt)
	modcnt=1;
    
    
    /*Check for xray structures or last model of nmr structures */
    for(i=0;i<pdbfile->nchains;i++)
    {	
	/* Only bother reporting error messages if a message
	   about the SEQRES records not containing enough aa's
	   has not already been reported */
	/* If pdbfile->chainok is False, leave it so  */
	if(!pdbfile->chainok[i])
	    continue;
	else
	{
	    if(!chainok[i])
	    {
		pdbfile->chainok[i]=chainok[i];
		ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "ATOMFEWAA", 
			    i+1,(ajChararrGet(pdbfile->chid, i)),modcnt);

	    }
	}
    }


    /* Mask out any chains with insufficient amino acids either in the 
       SEQRES or ATOM records */
    for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
	if(((pdbfile->linetype[i]==PDBPARSE_COORD)    ||
	    (pdbfile->linetype[i]==PDBPARSE_COORDHET)) &&
	   (!pdbfile->chainok[pdbfile->chnn[i]-1]))
	{
	    pdbfile->linetype[i]=PDBPARSE_IGNORE;


	}		
    
    
    
    /* Check for missing TER records.
       Where chains are not separated by TER records
       (the chain id changes from line to line without an intervening TER 
       record and both chain id's are not whitespace).
       Where ATOM and HETATM groups are not separated 
       by TER records (a chain id is given on one line, a whitespace chain
       id is given on the next line, and there is no intervening TER 
       record) */
    
    
    for(i=pdbfile->idxfirst+1;
	i<pdbfile->nlines;i++)
    {
	if((pdbfile->linetype[i-1]==PDBPARSE_COORD) && 
	   (pdbfile->linetype[i]==PDBPARSE_COORD) &&
	   pdbfile->chnn[i-1] != pdbfile->chnn[i])
	    ajFmtPrintF(flog, "%-15s%d %d\n", "TERMISSCHN", i, i+1);
	else if((pdbfile->linetype[i-1]==PDBPARSE_COORD) && 
		((pdbfile->linetype[i]==PDBPARSE_COORDHET) || 
		 (pdbfile->linetype[i]==PDBPARSE_COORDWAT))&&
		pdbfile->chnn[i-1] == pdbfile->chnn[i])
	    ajFmtPrintF(flog, "%-15s%d %d\n", "TERMISSHET", i, i+1);
    }
    

    /* Tidy up  */
    AJFREE(chainok);
    ajStrDel(&aa3);
    ajStrDel(&lastrn);
    ajStrDel(&sub);


    /* Report problems with non-protein chains */
    for(i=0;i<pdbfile->nchains; i++)
	if(pdbfile->chainok[i])
	{
	    ok = ajTrue;
	    break;
	}		
    
    /* Return now if no protein chains are found */
    if(!ok)
    {
	ajWarn("No protein chains found in raw pdb file");
	ajFmtPrintF(flog, "%-15s\n", "NOPROTEINS");
	return ajFalse;
    }
    
    return ajTrue;
}




/* @funcstatic  StandardiseNumbering ****************************************
**
** Reads a Pdbfile object and standardises the two sets of residue numbers 
** (resn1 & resn2 arrays) derived from the raw residue numbers. The residue 
** numbering is corrected for zero or negative residue numbers, non-standard 
** numbering schemes and any other cases of non-sequentiality (e.g. where the
** next residue number is lower than the previous one, see 1pca).
** resn1 gives the sequence presuming an alternative numbering scheme, resn2 
** gives the sequence presuming heterogeneity. Heterogeneity is indicated by 
** a character in position lines[26] (the same position used to indicate 
** alternative residue numbering schemes).
**
** The resn1 & resn2 arrays of a Pdbfile object are modified.  The oddnum 
** array is written. 
** 
** oddnum array
** This is an array of Bool's which are TRUE for duplicate residues of 
** heterogenous positions (e.g. if 2 different residues are both numbered '8' 
** or one is '8' and the other '8A' for example then <oddnum> would be set 
** True for the second residue. 
**
** @param [w] pdbfile [AjPPdbfile] Pdbfile object
** @param [u] flog    [AjPFile]      Pointer to log file (build diagnostics)
** 
** @return [AjBool]  True on success, False otherwise
** @@
****************************************************************************/

static AjBool StandardiseNumbering(AjPPdbfile pdbfile, AjPFile flog)
{	
    ajint   i=0;
    ajint   lastchn=-1;		    /* Chain number of last line read in */
    AjBool  first=ajFalse;	    /* True if we have processed the first 
				       residue in a chain */
    ajint   first_num=0;	    /* Number of first residue in chain */
    ajint   modrn=0;		    /* Corrected first residue number */
    AjBool  neg=ajFalse;	    /* True if first residue number is 
				       negative */
    AjBool  zer=ajFalse;	    /* True if first residue number is zero */
    AjBool  report_neg=ajFalse;	    /* True if we have reported an error 
				       that a residue number is negative 
				       for this chain*/
    AjBool  report_zer=ajFalse;	    /* True if we have reported an error 
				       that a residue number is zero 
				       for this chain*/
    ajint   add=0;		    /* An amount to add to the residue 
				       numbers to correct them */
    AjBool  ignore=ajFalse;
    AjBool  odd=ajFalse;
    ajint   rn=0;		    /* Current residue number */
    ajint   last_rn=0;		    /* Last residue number read in */    
    ajint   this_rn=0;		    /* Current residue number read in */    
    char    last=' ';		    /* Chain id of last chain */
    char    curr=' ';		    /* Chain id of current chain */
    AjPStr  last_rt=NULL;	    /* Type of previous residue */
    AjPStr  this_rt=NULL;	    /* Type of current residue */
    AjBool  report_nonstd=ajFalse;  /* True if we have reported an error 
				       that a non-standard
				       residue numbering scheme is used 
				       for this chain */
    AjBool  report_nonsqt=ajFalse;  /* True if we have reported an error 
				       that any other cases
				       of non-sequential numbering are 
				       found for this this chain */
    char     aa1=' ';		    /* Amino acid single character code */

    ajuint   ipos = 0;
    

    last_rt=ajStrNew();
    this_rt=ajStrNew();
    
 
    /* Check args */
    if(!pdbfile || !flog)
    {
	ajWarn("Bad args passed to StandardiseNumbering\n");

	return ajFalse;
    }


    /* Check whether the integer part of the original pdb 
       numbering (at this point in code held in resn1 and
       resn2) gives the correct index into the SEQRES sequence */
    for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    { 
	/* If model number has gone past 1 then we must have checked
	   all the chains, so break */
	if(pdbfile->modn[i] > 1)
	    break;

	if(pdbfile->linetype[i]!=PDBPARSE_COORD)
	    continue;
	

	/* If residue number is not negative, zero, or greater then the
	   length of the SEQRES sequence and if the residue matches then
	   continue */
	 
	if((pdbfile->resn1[i] <= 
	    pdbfile->nres[pdbfile->chnn[i]-1])||
	   (pdbfile->resn1[i] >=  1))
	{
	    ajResidueFromTriplet(pdbfile->rtype[i],&aa1);
	    ipos = pdbfile->chnn[i] - 1;

	    if(aa1 == ajStrGetCharPos(pdbfile->seqres[ipos], 
				pdbfile->resn1[i]-1))
		continue;
	}
	
	/* Otherwise flag an error for this chain and move to the 
	   end of the chain */

	ajFmtPrintF(flog, "%-15s%d (%c)\n", "BADINDEX",  
		    pdbfile->chnn[i], 
		    ajChararrGet(pdbfile->chid,pdbfile->chnn[i]-1));
	
	
	for(lastchn=pdbfile->chnn[i]; i<pdbfile->nlines;i++)
	{
	    if(pdbfile->linetype[i]!=PDBPARSE_COORD)
		continue;

	    if(pdbfile->modn[i] > 1)
		break;

	    if(pdbfile->chnn[i] != lastchn)
	    {
		i--;
		break;
	    }
	}
    }
    



    /* Fix for zero or negative residue numbers.
       This is done for both resn1 and resn2 arrays of 
       a Pdbfile object */
    
    for(first=ajFalse, i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	/* Coordinate line */
	if(pdbfile->linetype[i]==PDBPARSE_COORD)
	{
	    /* New chain */
	    if(pdbfile->chnn[i]!=lastchn)
	    {
		neg=ajFalse;
		zer=ajFalse;
		report_neg=ajFalse;
		report_zer=ajFalse;
		first=ajFalse;
		lastchn=pdbfile->chnn[i];
	    }
	    
	    rn=pdbfile->resn1[i];
	    

	    if(!first)
	    {
		first_num = rn;

		if(first_num > 0)
		{
		    /* Advance counter to next chain */
		    for( ;i<pdbfile->nlines;i++)
			if(((pdbfile->linetype[i]==
			     PDBPARSE_COORD)&&(pdbfile->chnn[i]!=lastchn))
			   || pdbfile->linetype[i]==PDBPARSE_MODEL)
			{
			    neg=ajFalse;
			    zer=ajFalse;
			    report_neg=ajFalse;
			    report_zer=ajFalse;
			    first=ajFalse;
			    lastchn=-1;
			    
			    break;
			}

		    /* i will get incremented in main loop above */
		    i--;		
		    continue;
		}
		
		first=ajTrue;	
	    }
	    if(rn < 0) 
	    {		
		neg=ajTrue;

		if(zer)
		    modrn=rn-(first_num-1);
		else			/* if(neg && !zer) */
		    modrn=rn-(first_num-1);

		if(!report_neg)
		{
		    ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "NEGNUM", 
				pdbfile->chnn[i], 
				ajChararrGet(pdbfile->chid, 
					     (pdbfile->chnn[i]-1)),i+1);
		    report_neg=ajTrue;
		}
	    }
	    else if (rn == 0)
	    {
		zer=ajTrue;

		if(neg)
		    modrn=rn-(first_num-1);
		else			/* if(!neg) */
		    modrn=rn+1;

		if(!report_zer)
		{	
		    ajFmtPrintF(flog, "%-15s%d (%c) %d\n", 
				"ZERNUM", pdbfile->chnn[i], 
				ajChararrGet(pdbfile->chid, 
					     (pdbfile->chnn[i]-1)),i+1);
		    report_zer=ajTrue;
		}
	    }
	    else			/*rn is (+ve)*/
	    {
		if(!neg && zer)
		    modrn=rn+1;
		else 	if(neg && zer)
		    modrn=rn-(first_num-1);
		else			/* if(neg && !zer) */
		    modrn=rn-(first_num);
	    }

	    pdbfile->resn1[i]=modrn;
	    pdbfile->resn2[i]=modrn;
	}
	/* New model */
	else if(pdbfile->linetype[i]==PDBPARSE_MODEL)
	{
	    neg=ajFalse;
	    zer=ajFalse;
	    report_neg=ajFalse;
	    report_zer=ajFalse;
	    first=ajFalse;
	    lastchn=-1;
	}	
    }
    

    
    /* Fix non-standard residue numbering scheme.
       This is done for resn1 array of a Pdbfile object only. */
    for(lastchn=-1, i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	/* Coordinate line */
	if(pdbfile->linetype[i]==PDBPARSE_COORD)
	{
	    /* New chain */
	    if(pdbfile->chnn[i]!=lastchn)
	    {
		add = 0;
		first=ajFalse;
		ignore=ajFalse;
		lastchn=pdbfile->chnn[i];
		report_nonstd=ajFalse;		
	    }

	    if(!first)
	    {
		/*Remove the chmyotrypsin numbering code*/
		last_rn = pdbfile->resn1[i];
		last=ajStrGetCharPos(pdbfile->lines[i], 26);
		first=ajTrue;
		continue;
	    }
	    
	    rn = pdbfile->resn1[i];
	    curr=ajStrGetCharPos(pdbfile->lines[i], 26);

	    if(curr!=last)
	    {
		if(rn == last_rn) 
		{
		    add++;
		    ignore=ajTrue;
		}
	    }

	    if(rn != last_rn) 
	    {
		ignore = ajFalse;
	    }


	      
	    last=curr;
	    last_rn = rn;

	    pdbfile->resn1[i]=rn+add;

	    if(ignore)
	    {
		pdbfile->oddnum[i]=ajTrue;

		if(!report_nonstd)
		{
		    ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "ODDNUM", 
				pdbfile->chnn[i], 
				ajChararrGet(pdbfile->chid, 
					     (pdbfile->chnn[i]-1)),i+1);
		    report_nonstd=ajTrue;		
		}
		    
	    }
	    
	}
	else if(pdbfile->linetype[i]==PDBPARSE_MODEL)
	{
	    add = 0;
	    first=ajFalse;
	    ignore=ajFalse;
	    lastchn=-1;
	    report_nonstd=ajFalse;		
	}
    }
    




    

    /* Fix remaining non-sequential residue numbering in resn1
       array of Pdbfile object*/
    for(lastchn=-1, i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	/* Coordinate line */
	if(pdbfile->linetype[i]==PDBPARSE_COORD)
	{
	    /* New chain */
	    if(pdbfile->chnn[i]!=lastchn)
	    {
		add = 0;
		first=ajFalse;
		ignore=ajFalse;
		lastchn=pdbfile->chnn[i];
		report_nonsqt=ajFalse;
	    }

	    if(!first)
	    {
		last_rn=pdbfile->resn1[i];
		ajStrAssignS(&last_rt, pdbfile->rtype[i]);

		first=ajTrue;
		continue;
	    }
	    
	    this_rn=pdbfile->resn1[i];
	    ajStrAssignS(&this_rt, pdbfile->rtype[i]);


	    /* A new residue is indicated if this ATOM is 'N' or 
	       if this is a different residue type*/
	    if( !(ajStrCmpC(pdbfile->atype[i],  "N")) || 
	       !(ajStrMatchS(this_rt, last_rt)))
	    {
		/* Check for duplicate residue numbers */
		if(this_rn == last_rn)
		{
		    add++;
		    ignore=ajTrue;
		    odd=ajTrue;
		}

		/* Check for drops in residue numbers, see 1pca */
		if(this_rn < last_rn)
		{
		    add+=(last_rn-this_rn+1);
		    ignore=ajTrue;
		    odd=ajFalse;
		}
	    }


	    if(this_rn > last_rn)
		ignore = ajFalse;

	    pdbfile->resn1[i]=this_rn+add;


	    ajStrAssignS(&last_rt, this_rt);
	    last_rn = this_rn;
	    

	    if(ignore)
	    {
		if(odd)
		    pdbfile->oddnum[i]=ajTrue;
		
		if(!report_nonsqt)
		{
		    ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "NONSQNTL", 
				pdbfile->chnn[i], 
				ajChararrGet(pdbfile->chid, 
					     (pdbfile->chnn[i]-1)),i+1);
		    report_nonsqt=ajTrue;		
		}
	    }
	}
	
	else if(pdbfile->linetype[i]==PDBPARSE_MODEL)    
	{
	    add = 0;
	    first=ajFalse;
	    ignore=ajFalse;
	    lastchn=-1;
	    report_nonsqt=ajFalse;
	}
    }
    


    /* Fix remaining non-sequential residue numbering in resn2
       array of Pdbfile object (duplicate lines for presumed 
       heterogenous residues positions are ignored)*/
    for(lastchn=-1, i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	/* Coordinate line */
	if((pdbfile->linetype[i]==PDBPARSE_COORD) && 
	   (!pdbfile->oddnum[i]))
	{
	    /* New chain */
	    if(pdbfile->chnn[i]!=lastchn)
	    {
		add = 0;
		first=ajFalse;
		lastchn=pdbfile->chnn[i];
	    }

	    if(!first)
	    {
		last_rn=pdbfile->resn2[i];
		ajStrAssignS(&last_rt, pdbfile->rtype[i]);

		first=ajTrue;
		continue;
	    }

	    this_rn=pdbfile->resn2[i];
	    ajStrAssignS(&this_rt, pdbfile->rtype[i]);


	    /* A new residue is indicated if this ATOM is 'N' or 
	       if this is a different residue type*/
	    if(!(ajStrCmpC(pdbfile->atype[i],  "N")) || 
	       !(ajStrMatchS(this_rt, last_rt)))
	    {
		/* Check for duplicate residue numbers */
		if(this_rn == last_rn)
		    add++;

		/* Check for drops in residue numbers, see 1pca */
		if(this_rn < last_rn)
		    add+=(last_rn-this_rn+1);
	    }

	    pdbfile->resn2[i]=this_rn+add;
	    ajStrAssignS(&last_rt, this_rt);
	    last_rn = this_rn;
	}
	else if (pdbfile->linetype[i]==PDBPARSE_MODEL)   
	{
	    add = 0;
	    first=ajFalse;
	    lastchn=-1;
	}
    }


    /* Tidy up and return */
    ajStrDel(&last_rt);
    ajStrDel(&this_rt);

    return ajTrue;
}




/* @funcstatic AlignNumbering *************************************************
**
** Reads a Pdbfile object and determines for each chain a set of residue 
** numbers (the resni array) that give the correct index into the full length 
** (SEQRES) sequence for residues listed in the ATOM records.    
**
** The resni and resn1ok arrays of a Pdbfile object are written. 
** The seqres and nres elements may be modified for any missing N-terminal 
** residues.
**
** resn1ok array
** This array contains Bool's for each chain which are TRUE if resn1 was 
** used to derive resni, i.e. gave correct alignment to the full-length 
** (SEQRES) sequence.  If False then resn2 was used ( resn1 gives the 
** sequence presuming an alternative numbering scheme, resn2 gives the 
** sequence presuming heterogeneity).
**
** @param [w] pdbfile [AjPPdbfile] Pdbfile object
** @param [u] flog    [AjPFile]      Pointer to log file (build diagnostics)
** @param [r] lim     [ajuint]        Max. no. permissible mismatches between
**                                   the ATOM & SEQRES sequences.
** @param [r] lim2     [ajuint]       Max. no. residues to trim when checking
**                                   for missing C-terminal SEQRES residues.
** 
** @return [AjBool]  True on success, False otherwise
** @@
** Must comment what diagnostics this writes!
** This now writes its diagnostics to flog, rather than tempfile.
****************************************************************************/

static AjBool AlignNumbering(AjPPdbfile pdbfile, AjPFile flog, ajuint lim, 
			     ajuint lim2)
{
    /* Sequence and residue number data are given for each unique chain 
       (data for the first model only is considered when assigning residue
       numbers) */
       

    /* Rather than use, e.g. seq1, seq2 & seq, we could use a
       single 2 dimensional array, but we would need new code for 2d
       arrays of characters*/
    

    ajuint       a=0;		/* Loop counter */
    ajuint       b=0;		/* Loop counter */
    ajuint    maxb=0;		/* Max value of b */
    ajint       i=0;		/* Loop counter */
    ajint       j=0;		/* Loop counter */
    ajint       k=0;		/* Loop counter */
    ajint       x=0;		/* Loop counter */
    ajint       y=0;		/* Loop counter */
    ajint       z=0;		/* Loop counter */


    AjPStr  *seq1=NULL;		/* Sequences of residues from ATOM 
				   records (all residues) */
    AjPStr  *seq2=NULL;		/* Sequences of residues from ATOM 
				   records (excluding residues
				   for which oddnum array in Pdbfile 
				   object is True) */
    AjPStr  *seq=NULL;		/* Pointer to seq1 or seq2 */
    AjPStr  seqbit=NULL;	/* Subsequence of seq (real copy) */    
    ajlong   lenseqbit=0;        /* Length of seqbit */
    

    ajint   *nres1=NULL;	/* No. residues for seq1/arr1 */
    ajint   *nres2=NULL;	/* No. residues for seq2/arr2 */
    ajint   *nres=NULL;		/* Pointer to nres1 or nres2 */
    

    AjPInt  *num1=NULL;		/* Residue numbers for seq1 (from resn1 
				   element of the Pdbfile object) */
    AjPInt  *num2=NULL;		/* Residue numbers for seq2 (from resn2 
				   element of the Pdbfile object) */
    AjPInt  *num=NULL;		/* Pointer to num1 or num2 */
    
    AjPInt  *idx=NULL;		/* Gives correct index into seqres sequence 
				   (from Pdbfile object) for the 
				   current sequence. These are residue numbers 
				   and therefore idx would 
				   have a value of 1 for the first seqres 
				   residue.*/
    AjPInt  *idx_full=NULL;	/* As idx but with empty array elements 
				   replacing missing residues
				   so that we can index into idx_full using 
				   residue numbers from num */


    ajint    last1=-1000;	/* Number of last residue for seq1/arr1*/
    ajint    last2=-1000;	/* Number of last residue for seq2/arr2*/
    
    char     aa1=' ';		/* Amino acid single character code*/
    ajint    c=0;		/* No. of current chain */
    
    AjBool   done=ajFalse;	/* True if we have found the correct residue 
				   numbering */
    
    char    *insert=NULL;	/* String from N-terminus of ATOM sequence to 
				   insert at N-terminus of SEQRES sequence in 
				   case of the later missing residues */
    AjPStr   tmpseqres=NULL;	/* Temp. string for seqres sequence from 
				   Pdbfile object */
    AjPStr   bit=NULL;	        /* Temp. string for a bit of sequence */
    ajuint   nmismatches=0;	/* No. of mismatches between ATOM and SEQRES 
				   sequence */
    ajlong    loc=0L;		/* Location of ATOM sequence in SEQRES sequence
				   (if applicable) */
    ajint    len=0;		/* Length of seqres sequence from Pdbfile 
				   object*/
    AjBool   err=ajFalse;	/* True if a residue number from the ATOM 
				   records would cause
				   an array boundary error in the seqres 
				   sequence */
    ajint siz_substr=0;		/* Size of substring for alignment of ATOM and 
				   SEQRES sequences */
    const char *atm_ptr=NULL; 	/* Pointer to ATOM sequence */
    const char *seqres_ptr=NULL;/* Pointer to SEQRES sequence */
    const char *loc_ptr=NULL;   /* Pointer for location of match of substring
				   to SEQRES sequence*/
    AjPStr substr=NULL;		/* Substring of ATOM sequence */
    AjPStr substr2=NULL;	/* Substring of ATOM sequence */
    ajint  atm_idx=0;		/* Index into ATOM sequence */
    ajint  seqres_idx=0;	/* Index into SEQRES sequence */
    ajint  seqres_idx_last=0;	/* Index into SEQRES sequence for C-terminal 
				   residue of substring*/		
    char   aa_last=' ';         /* Amino acid residue code of C-terminal 
				   residue of substring*/		
    AjBool fixed=ajFalse;       /* Whether the mismatch residue of the 
				   substring was later aligned correctly */
    AjBool done_end=ajFalse;	/* True if we have aligned the terminus of the 
				   ATOM sequence */
    AjBool founderr=ajFalse;    /* Match of substring of ATOM sequence to 
				   SEQRES found with potential mismatched
				   residue */
    AjPStr  msgstr=NULL;	/* A string to hold a message */
    AjPStr  msgbit=NULL;	/* A temp. string to hold part of a message */
    ajint  idx_misfit_atm=0;    /* Index into ATOM sequence (seq) for first 
				   residue that does not match SEQRES
				   sequence */
    
    ajint  idx_misfit_seqres =0;/* Index into SEQRES sequence for first
				   residue that does not match ATOM sequence */
    AjPStr aa_misfit=NULL;	/* Original (PDB) residue number for first 
				   residue mismatch between ATOM and SEQRES 
				   sequences */
    ajint  this_num = 0;	/* Current residue number */
/*DIAGNOSTIC    ajint  max = 0; */            /* Used in diagnostics code */
    
    
    
    /* Check args */
    if(!pdbfile || !flog)
    {
	ajWarn("Bad args passed to AlignNumbering\n");

	return ajFalse;
    }
    
    
    
    /* Allocate memory for arrays etc*/
    aa_misfit=ajStrNew();
    msgstr=ajStrNew();
    msgbit=ajStrNew();
    seqbit=ajStrNew();    

    insert=ajCharNewRes(MAXMISSNTERM);
    tmpseqres=ajStrNew();
    bit=ajStrNew();
    substr=ajStrNew();    
    substr2=ajStrNew();    
    
    
    AJCNEW0(seq1, pdbfile->nchains);
    AJCNEW0(seq2, pdbfile->nchains);
    
    
    AJCNEW0(num1, pdbfile->nchains);
    AJCNEW0(num2, pdbfile->nchains);
    
    AJCNEW0(idx, pdbfile->nchains);
    AJCNEW0(idx_full,  pdbfile->nchains);
    
    AJCNEW0(nres1, pdbfile->nchains);
    AJCNEW0(nres2, pdbfile->nchains);
    
    
    
    for(i=0;i<pdbfile->nchains; i++)
    {
	if(!pdbfile->chainok[i])
	    continue;

	seq1[i]=ajStrNew();
	seq2[i]=ajStrNew();
	
	num1[i]=ajIntNew();
	num2[i]=ajIntNew();
    }

    
    /* Assign arrays */
    for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	/* Coordinate line */
	if(pdbfile->linetype[i]==PDBPARSE_COORD)
	{
	    /* Break if we are no longer in the first model */
	    if(pdbfile->modn[i] != 1)
		break;
	    else
		c=pdbfile->chnn[i]-1;
	       

/*	    ajFmtPrint("%S\n", pdbfile->lines[i]); */
	    


	    if(pdbfile->resn1[i] != last1)
	    {
		ajResidueFromTriplet(pdbfile->rtype[i],&aa1);
		ajStrAppendK(&seq1[c], aa1);
		
		ajIntPut(&num1[c], nres1[c],pdbfile->resn1[i]);
		last1=pdbfile->resn1[i];
		nres1[c]++;
	    }

	    if((pdbfile->resn2[i] != last2) && (!pdbfile->oddnum[i]))
	    {
		ajResidueFromTriplet(pdbfile->rtype[i],&aa1);
		ajStrAppendK(&seq2[c], aa1);
		
		ajIntPut(&num2[c], nres2[c],pdbfile->resn2[i]);
		last2=pdbfile->resn2[i];
		nres2[c]++;
	    }
	}
    }
    
    

    /* Allocate memory for arrays of residue numbers */
    for(i=0;i<pdbfile->nchains; i++)
    {
	if(!pdbfile->chainok[i])
	    continue;


	/* Array must be big enough to cope with either sequence */
	if(nres1[i] > nres2[i])
	    idx[i]=ajIntNewRes(nres1[i]);
	else
	    idx[i]=ajIntNewRes(nres2[i]);

	

	/* Array must be big enough to cope with highest the residue 
	   number from either array */
	if(ajIntGet(num1[i], nres1[i]-1) > ajIntGet(num2[i], nres2[i]-1))
	    idx_full[i]=ajIntNewRes(ajIntGet(num1[i], nres1[i]-1)+1);
	else
	    idx_full[i]=ajIntNewRes(ajIntGet(num2[i], nres2[i]-1)+1);
    }
    
    

    
    /* Loop for each chain */
    for(i=0;i<pdbfile->nchains; i++)
    {
	
	/* Skip this chain if necessary */
	if(!(pdbfile->chainok[i]))
	    continue;
	else
	    ajStrAssignS(&tmpseqres, pdbfile->seqres[i]);


	/* Loop for checking for missing residues from N-term of SEQRES 
	   sequence */
	for(done=ajFalse, j=0;j<MAXMISSNTERM+1;j++)
	{
	    /* Loop for the 2 sequences derived from the ATOM records */
	    for(x=0;x<2;x++)
	    {
		if(x==0)
		{
		    seq=seq1;
		    nres=nres1;
		    num=num1;
		}
		else
		{
		    seq=seq2;
		    nres=nres2;
		    num=num2;
		}	

		
		/* Restore the original seqres sequence */
		ajStrAssignS(&(pdbfile->seqres[i]), tmpseqres);
		pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);
		
		
		/* Modify the seqres sequence in the Pdbfile object 
		   by adding the first j residues from the N-terminus
		   of the ATOM sequence to the N-terminus of <seqres>. */
		
		for(k=0;(k<j)&&(k<nres[i]);k++)
		    insert[k]=ajStrGetCharPos(seq[i], k);

		insert[k]='\0';

		ajStrInsertC(&(pdbfile->seqres[i]), 0, insert);
		pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);


		/* DIAGNOSTIC CODE  
		ajFmtPrintF(flog, "\nChainMod %d"
		   " (seq %d)\n%S\n%S\n\n\n", 
		   i+1,x+1,
		   seq[i], 
		   pdbfile->seqres[i]);
		   */
		    

		/***********************************************/
		/******************* STEP 1 ********************/
		/***********************************************/
		/*DIAGNOSTIC 
		  ajFmtPrintF(flog, "STEP1 tmpseqres: %S\n", tmpseqres); 
		  
		  ajFmtPrintF(flog, "chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i], 
		  pdbfile->seqres[i]); 
		 ajFmtPrintF(flog, "\n");
		  if(ajStrMatchS(seq1[i], seq2[i]))
		 ajFmtPrintF(flog, "seq1 and seq2 match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		 ajFmtPrintF(flog, "seq1 and seqres match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		 ajFmtPrintF(flog, "\n");
		*/

		/* Check whether sequences are identical length*/
		if(nres[i]==pdbfile->nres[i])
		{
		    /* Sequences are identical - assign residue 
		       numbers 'by hand' */		    
		    if(ajStrMatchS(seq[i], pdbfile->seqres[i]))
		    {
			for(k=0;k<nres[i];k++)
			    ajIntPut(&idx[i], k, k+1);
			
			if(x==0)
			    pdbfile->resn1ok[i]=ajTrue;
			else
			    pdbfile->resn1ok[i]=ajFalse;		       
			
			done=ajTrue;

			/* DIAGNOSTIC ajFmtPrintF(flog, "STEP1 OK\n");  */
			
			break;
		    }
		    /* Sequence are same length but contain mismatches */
		    else
		    {	
			for(ajStrAssignClear(&msgstr), nmismatches=0, k=0;
			    k<nres[i];k++)
			    if(ajStrGetCharPos(seq[i], k) != 
			       ajStrGetCharPos(pdbfile->seqres[i], k))
			    {
				nmismatches++;
				/* Correct the seqres sequence.
				   Replace this with appropriate library 
				   call once available so we don't have to 
				   probe the internals of the structure */


				/* a will give the number of the first
				   coordinate line for the mismatch residue 
				   from the ATOM records*/
				a = PdbfileFindLine(pdbfile, i+1, x, 
						    ajIntGet(num[i], k));
								

				/* Get the id of the mismatch residue in the 
				   SEQRES sequence.  */
				ajResidueToTriplet(pdbfile->seqres[i]->Ptr[k], 
					       &aa_misfit);

				/* To give correct index into SEQRES records 
				   in original PDB file, subtract j to account
				   for modifications to the N-terminus
				   that were made for missing residues relative
				   to ATOM sequence.
				   A further 1 is added to give a number 
				   starting from 1 (rather than 0)*/

				ajFmtPrintS(&msgbit,  "%S%S:%S%d.    ", 
					    pdbfile->rtype[a], 
					    pdbfile->pdbn[a], 
					    aa_misfit, k-j+1);


				ajStrAppendS(&msgstr, msgbit);
				    
				    
				pdbfile->seqres[i]->Ptr[k]=
				    ajStrGetCharPos(seq[i], k);

			    }
			
			
			/* Sequences are same length (acceptable number of 
			   mismatches) */
			if(nmismatches<=lim)
			{
			    if(nmismatches)
				ajFmtPrintF(flog, "%-15s%d (%c) %d %S\n", 
					    "MISMATCH",  i+1,  
					    ajChararrGet(pdbfile->chid, i),
					    nmismatches, msgstr);
			    

			    for(k=0;k<nres[i];k++)
				ajIntPut(&idx[i], k, k+1);
			
			    if(x==0)
				pdbfile->resn1ok[i]=ajTrue;
			    else
				pdbfile->resn1ok[i]=ajFalse;		       
			
			    done=ajTrue;
			    /*DIAGNOSTIC		 
			   ajFmtPrintF(flog, "STEP1 OK %d mismatches\n",
                                       nmismatches); */

			    break;
			}
			else
			{
			    /* Otherwise, sequences are same length 
			       (unacceptable number of mismatches) 
			       Restore the original seqres sequence */
			    ajStrAssignS(&(pdbfile->seqres[i]), tmpseqres);
			   pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);
			    /*DIAGNOSTIC 
			     ajFmtPrintF(flog,
                                         "STEP1 **NOT** OK %d mismatches\n", 
			                 nmismatches);  */
			    
			}
		    }
		}


		
				    
		/***********************************************/
		/******************* STEP 2 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		ajFmtPrintF(flog, "STEP2 tmpseqres: %S\n", tmpseqres);
		
		ajFmtPrintF(flog, "chnn : %d\n"
			    "seq1 : %S\n"
			    "seq2 : %S\n"
			    "seqr : %S\n", i+1, seq1[i], seq2[i], 
			    pdbfile->seqres[i]); 
		ajFmtPrintF(flog, "\n");
		if(ajStrMatchS(seq1[i], seq2[i]))
		    ajFmtPrintF(flog, "seq1 and seq2 match\n");
		else
		    ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		
		if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		    ajFmtPrintF(flog, "seq1 and seqres match\n");
		else
		    ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		ajFmtPrintF(flog, "\n");
		 */


		/* JONNEW The code below replaces 'ORIGINAL' code block */
		maxb=lim2;

		if(maxb >= MAJSTRGETLEN(seq[i]))
		    maxb = MAJSTRGETLEN(seq[i]) - 1;

		/*DIAGNOSTIC  
		ajFmtPrintF(flog, "maxb = %d\nlim2 = %d\n");
		ajDebug("strlen: %d\n",
			    maxb, lim2, MAJSTRGETLEN(seq[i]));*/
		

		/* First pass through loop is full-length sequence */
		for(b=0; b< maxb+1; b++)
		{
		    lenseqbit = MAJSTRGETLEN(seq[i]) -b;
		    ajStrAssignSubS(&seqbit, seq[i], 0, (lenseqbit -1));
		    ajStrAssignSubS(&bit, seq[i], (lenseqbit), -1);
		    
		    /*DIAGNOSTIC  
		    ajFmtPrintF(flog, "Trying ATOM substring %S\n"
				"versus SEQRES        %S\n",
                                seqbit, pdbfile->seqres[i]);*/
		    
		    
		    /* Check whether ATOM is substring of SEQRES sequence */
		    if((loc=ajStrFindS(pdbfile->seqres[i], seqbit))!=-1)
		    {
			/* Check to ensure that the last substring
			   residue is aligned to the last residue of
			   the SEQRES residue, otherwise, problems
			   would arise in cases where SEQRES sequence
			   had C-terminal residues that were absent
			   from the ATOM (& therefore also substring)
			   sequence. */
			if((loc+lenseqbit) != pdbfile->nres[i])
			    break;
			
			/* ATOM is substring of SEQRES sequence -
			   assign residue numbers 'by hand' */

			for(k=0;k<nres[i];k++)
			  ajIntPut(&idx[i], k, k+(ajint)loc+1);
		    
			if(x==0)
			    pdbfile->resn1ok[i]=ajTrue;
			else
			    pdbfile->resn1ok[i]=ajFalse;		       
		    

			/* SEQRES sequence is missing C-terminal ATOM
                           residues */
			if(b)
			{
			    ajFmtPrintF(flog, "%-15s%d (%c) %d\n",
					"MISSCTERM", i+1,
					 ajChararrGet(pdbfile->chid, i),b);
			    
			    ajStrAppendS(&(pdbfile->seqres[i]), bit);
			   pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);
			}
			
			done=ajTrue;
			/*DIAGNOSTIC  
			ajFmtPrintF(flog, "STEP2 OK\n");  */
			break;
		    }
		}

		if(done)
		    break;
		else
		{
		    /* Otherwise, ATOM is NOT a substring of SEQRES sequence */
		    /*DIAGNOSTIC
		    ajFmtPrintF(flog, "STEP2 **NOT** OK\n"); 	*/ 	
		}
		
		    /* ORIGINAL
		       if((loc=ajStrFindS(pdbfile->seqres[i], seq[i]))!=-1)
		       {
		       for(k=0;k<nres[i];k++)
		       ajIntPut(&idx[i], k, k+loc+1);
		       
		       if(x==0)
		       pdbfile->resn1ok[i]=ajTrue;
		       else
		       pdbfile->resn1ok[i]=ajFalse;		       
		       
		       done=ajTrue;
		       ajFmtPrintF(flog, "STEP2 OK\n");  
		       break;
		       }
		       */



		/***********************************************/
		/******************* STEP 3 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrintF(flog, "STEP3 tmpseqres: %S\n", tmpseqres);
		  
		  ajFmtPrintF(flog, "chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i], 
		  pdbfile->seqres[i]); 
		 ajFmtPrintF(flog, "\n");
		  if(ajStrMatchS(seq1[i], seq2[i]))
		 ajFmtPrintF(flog, "seq1 and seq2 match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		 ajFmtPrintF(flog, "seq1 and seqres match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		 ajFmtPrintF(flog, "\n");
		 */


		/* Check whether SEQRES is substring of ATOM sequence */
		/* This will only find omissions from the SEQRES
		   sequence where the ATOM sequence would align to it
		   without gaps, and where the SEQRES sequence does not
		   have extra N-terminal residues relative to ATOM
		   (such cases are caught in STEP 2) */
		if((loc = ajStrFindS(seq[i],pdbfile->seqres[i])) !=-1)
		{
		    /* SEQRES is substring of ATOM sequence - correct for
		       residues missing from SEQRES sequence and assign 
		       residue numbers 'by hand' */
		    
		    /* N-terminal insertion needed*/
		    if(loc != 0)
		    {
			ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "MISSNTERM", 
				    i+1, ajChararrGet(pdbfile->chid, i), 
				    loc);
						

			ajStrAssignSubS(&bit, seq[i], 0, loc-1);
			ajStrInsertS(&(pdbfile->seqres[i]), 0, bit); 
			pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);
		    }
		    
		    /* C-terminal insertion needed */
		    if(pdbfile->nres[i]!=nres[i])
		    {
			ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "MISSCTERM", 
				    i+1, ajChararrGet(pdbfile->chid, i),
				    ( nres[i] - pdbfile->nres[i]));
			

			ajStrAssignSubS(&bit, seq[i], pdbfile->nres[i], 
				    nres[i]-1);
			ajStrAppendS(&(pdbfile->seqres[i]), bit);
			pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);
		    }

		    for(k=0;k<nres[i];k++)
			ajIntPut(&idx[i], k, k+1);
		    
		    if(x==0)
			pdbfile->resn1ok[i]=ajTrue;
		    else
			pdbfile->resn1ok[i]=ajFalse;		       

		    /*DIAGNOSTIC   ajFmtPrintF(flog, "STEP3 OK\n");  */

		    done=ajTrue;
		    break;
		}

		/* Otherwise, SEQRES is NOT a substring of the ATOM 
		   sequence */
		/* DIAGNOSTIC   ajFmtPrintF(flog, "STEP3 **NOT** OK\n");  */

		

		/***********************************************/
		/******************* STEP 4 ********************/
		/***********************************************/
		/*DIAGNOSTIC 
		  ajFmtPrintF(flog, "STEP4.1 tmpseqres: %S\n", tmpseqres);
		  
		  ajFmtPrintF(flog, "chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i], 
		  pdbfile->seqres[i]); 
		 ajFmtPrintF(flog, "\n");
		  if(ajStrMatchS(seq1[i], seq2[i]))
		 ajFmtPrintF(flog, "seq1 and seq2 match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		 ajFmtPrintF(flog, "seq1 and seqres match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		 ajFmtPrintF(flog, "\n");
		  
		  ajFmtPrintF(flog, "STEP4.2 tmpseqres: %S\n", tmpseqres);
		   */

		/* Check whether residue numbering is correct (and count 
		   the number of mismatches) */
		for(err=ajFalse, ajStrAssignClear(&msgstr), nmismatches=0, 
		    k=0;k<nres[i];k++)
		{
		    this_num = ajIntGet(num[i], k);

		    /* Check to prevent array boundary error */
		    if(this_num <= pdbfile->nres[i])
		    {
			if(ajStrGetCharPos(seq[i], k) != 
			   ajStrGetCharPos(pdbfile->seqres[i], this_num-1))
			{
			    nmismatches++;
			    /* Correct the seqres sequence.
			       Replace this with appropriate library 
			       call once available so we don't have to probe 
			       the internals of the structure */

			    
			    /* a will give the number of the first coordinate
			       line for the mismatch residue from the ATOM 
			       records */
			    a = PdbfileFindLine(pdbfile, i+1, x, 
						ajIntGet(num[i], k));
			    
			    /* Get the id of the mismatch residue in the 
			       SEQRES sequence.  */
			    
			    ajResidueToTriplet(
                                pdbfile->seqres[i]->Ptr[this_num-1], 
                                &aa_misfit);

			    ajFmtPrintS(&msgbit,  "%S%S:%S%d.    ", 
					pdbfile->rtype[a], pdbfile->pdbn[a], 
					aa_misfit, this_num-j);
			    

			    ajStrAppendS(&msgstr, msgbit);
				    
			    
			    pdbfile->seqres[i]->Ptr[this_num-1]=
				ajStrGetCharPos(seq[i], k); 

			}
		    }
		    else
		    {
			err=ajTrue;
			break;
		    }
		}
		

		/*DIAGNOSTIC	
		  ajFmtPrintF(flog, "STEP4.3 tmpseqres: %S\n", tmpseqres); */

		if(!err)
		{
		    /* Residue numbering is correct (no or acceptable number 
		       of mismatches) */		
		    if(nmismatches<=lim)
		    {
			if(nmismatches)
			    ajFmtPrintF(flog, "%-15s%d (%c) %d %S\n", 
					"MISMATCH",
					i+1,  
					ajChararrGet(pdbfile->chid, i),
					nmismatches, msgstr);


			

			for(k=0;k<nres[i];k++)  
			    ajIntPut(&idx[i], k, ajIntGet(num[i], k));
			
			if(x==0)
			    pdbfile->resn1ok[i]=ajTrue;
			else
			    pdbfile->resn1ok[i]=ajFalse;		       

			done=ajTrue;
			
			/*DIAGNOSTIC
			 ajFmtPrintF(flog, "STEP4 OK %d mismatches\n",
			  nmismatches);  	*/
			
 			break;
		    }
		    else
		    {
			/* Otherwise, residue numbering is incorrect 
			   (unacceptable number of mismatches) 
			   Restore the original seqres sequence*/
			ajStrAssignS(&(pdbfile->seqres[i]), tmpseqres);
			pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);

			
			/*DIAGNOSTIC
			 ajFmtPrintF(flog, "STEP4 **NOT** OK %d mismatches\n", 
			  nmismatches); 		*/
		    }
		}
		else
		{
		    /* Otherwise, residue numbering is incorrect (residue 
		       number is out of range) 
		       Restore the original seqres sequence*/
		    ajStrAssignS(&(pdbfile->seqres[i]), tmpseqres);
		    pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);



		    /*DIAGNOSTIC
		     ajFmtPrintF(flog, "STEP4 **NOT** OK out_of_range\n");
		     */
		}





		/***********************************************/
		/******************* STEP 5 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrintF(flog, "STEP5 tmpseqres: %S\n", tmpseqres); 
		  
		  ajFmtPrintF(flog, "chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i], 
		  pdbfile->seqres[i]); 
		 ajFmtPrintF(flog, "\n");
		  if(ajStrMatchS(seq1[i], seq2[i]))
		 ajFmtPrintF(flog, "seq1 and seq2 match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		 ajFmtPrintF(flog, "seq1 and seqres match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		 ajFmtPrintF(flog, "\n");
		 */



		/* None of the measures above could find the correct residue 
		   numbering so try by alignment.
		   
		   Align the SEQRES sequence to the ATOM sequence by taking 
		   progressively decreasing sized substrings from the ATOM 
		   sequence.  It returns 0 if any of the alignments would not 
		   leave enough space to be able to align the rest of the ATOM 
		   sequence to the SEQRES sequence - i.e. alignments giving an 
		   overspill of the ATOM sequence past the C-terminus of the 
		   SEQRES sequence are NOT allowed. 
		   
		   NO Mismatches are allowed at this stage 
		   */


		for(done_end=ajFalse,
		    len=pdbfile->nres[i],
		    siz_substr=nres[i], 
		    atm_ptr=ajStrGetPtr(seq[i]), 
		    seqres_ptr=ajStrGetPtr(pdbfile->seqres[i]); 
		    siz_substr>0; )
		{
		    ajStrAssignSubC(&substr, atm_ptr, 0, siz_substr-1);
		    
		    if((loc_ptr = strstr(seqres_ptr, 
					 ajStrGetPtr(substr)))==NULL)
		    {
			siz_substr--;
			continue;
		    }
		    else
		    {
			atm_idx= (int) ((atm_ptr-ajStrGetPtr(seq[i]))
					/sizeof(char));
			seqres_idx= (int) 
			    ((loc_ptr-ajStrGetPtr(pdbfile->seqres[i]))
			     /sizeof(char));

			


			/*CHECK TO SEE IF THERE IS SPACE TO FIT THE
			  REMAINER OF THE ATOM SEQUENCE IN THE SEQRES
			  SEQUENCE GIVEN THIS ALIGNMENT */
			if((nres[i] - atm_idx) > (len - seqres_idx))
			    break;
			
			for(k=0, y=atm_idx, z=seqres_idx; k<siz_substr; k++, 
			    y++, z++)
			    ajIntPut(&idx[i], y, z+1);

			
			
			/*Mark up last SEQRES residue as having been done */
			if(y==nres[i])
			    done_end = ajTrue;
		    }
      
		    atm_ptr+=siz_substr;
		    seqres_ptr=loc_ptr+siz_substr;
		    siz_substr=nres[i]-(atm_idx+siz_substr);
		}
		
		
		/*Check to ensure that position for last residue has been 
		  worked out */
		if(done_end)
		{
		    
		    /*DIAGNOSTIC		ajFmtPrintF(flog, "chnn : %d\n"
		      "seq1 : %S\n"
		      "seq2 : %S\n"
		      "seqr : %S\n", i+1, seq1[i], seq2[i], 
		      pdbfile->seqres[i]); 
		     ajFmtPrintF(flog, "\n");
		      if(ajStrMatchS(seq1[i], seq2[i]))
		     ajFmtPrintF(flog, "seq1 and seq2 match\n");
		      else
		     ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		      
		      if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		     ajFmtPrintF(flog, "seq1 and seqres match\n");
		      else
		     ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		     ajFmtPrintF(flog, "\n");
						*/

		    /* Residue numbering is correct after alignment 
		       (no mismatches) */
		    if(x==0)
			pdbfile->resn1ok[i]=ajTrue;
		    else
			pdbfile->resn1ok[i]=ajFalse;		       

		    ajFmtPrintF(flog, "%-15s%d (%c)\n", "GAPPEDOK", i+1,  
				ajChararrGet(pdbfile->chid, i));
		    		    
		    

		    /*DIAGNOSTIC
		    ajFmtPrintF(flog, "STEP5 OK\n");  */
		    
		    done=ajTrue;
		    break;
		}


		/*DIAGNOSTIC
		  ajFmtPrintF(flog, "STEP5 **NOT** OK\n"); */

		/* Otherwise, agreement could not be found */

		/* array might contain junk values
		   now but this should not matter as the array should
		   be overwritten */



		
		/***********************************************/
		/******************* STEP 6 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrintF(flog, "STEP6 tmpseqres: %S\n", 
		  tmpseqres);
		  
		  ajFmtPrintF(flog, "chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i], 
		  pdbfile->seqres[i]); 
		 ajFmtPrintF(flog, "\n");
		  if(ajStrMatchS(seq1[i], seq2[i]))
		 ajFmtPrintF(flog, "seq1 and seq2 match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatchS(seq1[i], pdbfile->seqres[i]))
		 ajFmtPrintF(flog, "seq1 and seqres match\n");
		  else
		 ajFmtPrintF(flog, "seq1 and seqres DO NOT match\n");
		 ajFmtPrintF(flog, "\n");
		  */


		/* Try again by alignment.
		   Mismatches ARE allowed at this stage 
		   Must change:
		   */
		

		for(ajStrAssignClear(&msgstr), 
		    nmismatches=0,
		    done_end=ajFalse,  
		    len=pdbfile->nres[i],
		    siz_substr=nres[i], 
		    atm_ptr=ajStrGetPtr(seq[i]), 
		    seqres_idx_last=-1,
		    seqres_ptr=ajStrGetPtr(pdbfile->seqres[i]); 
		    siz_substr>0; )
		{	
		    /* On the first pass, atm_ptr and seqres_ptr point to 
		       the start of the ATOM and SEQRES sequences 
		       respectively. */


		    founderr=ajFalse;

		    /* Copy block (of progressively decreasing size) from 
		       ATOM sequence to substring1  */
		    ajStrAssignSubC(&substr, atm_ptr, 0, siz_substr-1);
      

		    /*DIAGNOSTIC		
		      ajFmtPrintF(flog, "\n***\n%12s\n%s\n%12s\n%s\n", 
		      "Aligning ", ajStrGetPtr(substr), "To ", seqres_ptr);  */
		    
		    
		    /* Set loc_ptr to point to the first occurrence of
		       substring1 in SEQRES sequence */
		    /* If substring1 does not occur in SEQRES sequence */
		    if( (loc_ptr = strstr(seqres_ptr, 
					  ajStrGetPtr(substr)))==NULL)
		    {
			/*See if there is an error in the residue id at the 
			  start of <substr> Create a new substring from the 
			  ATOM records but omit the first character.
			  There is a special case if substr is only 1
			  character long.*/
			
			if(siz_substr == 1)
			{
			    loc_ptr = seqres_ptr;
			    nmismatches++;
			    founderr=ajTrue;
			}
			else
			{
			    /* Copy substring1 to substring2 but omit the 
			       first character */
			    ajStrAssignSubS(&substr2, substr, 1, -1);
			


			    /*DIAGNOSTIC
			      ajFmtPrintF(flog, 
			      "\n!!!\n%12s\n%s\n%12s\n%s\n", 
			      "Aligning ", ajStrGetPtr(substr2), "To ", 
			      seqres_ptr); */


			    /* Set loc_ptr to point to the first occurrence of
			       substring1 in SEQRES sequence */
			    /* If substring2 does not occur in the SEQRES 
			       sequence, continue with a smaller substring */
			    loc_ptr = strstr(seqres_ptr, 
					     ajStrGetPtr(substr2));
			    if(!loc_ptr)
			    {
				siz_substr--;
				continue;
			    }
			    /* substring2 is found in the SEQRES sequence */
			    else
			    {
				/* If there is not enough space to accommodate
				   the 'missing' residue continue (with a 
				   smaller substring) */
				if(loc_ptr == seqres_ptr)
				{	
				    siz_substr--;
				    continue;
				}


				/*DIAGNOSTIC 
				  ajFmtPrintF(flog, "\n\n\n";
				  ajFmtPrintF(flog, "nmismatches = "
				  "%d\n%12s%s\n%12s%s\n%12s%s\n%12s%s\n", 
				  nmismatches, 
				  "atm_ptr:",
				  atm_ptr, 
				  "seqres_ptr:",
				  seqres_ptr, 
				  "substr:",
				  substr, 
				  "substr2", 
				  substr2);  */

			    
				/*DIAGNOSTIC 
				  ajFmtPrintF(flog, 
				  "MISMATCH FOUND OK\n");  */

				/* There is enough space to accommodate 
				   substring2 and the 'missing' 
				   (mismatch) residue */
				nmismatches++;
				founderr=ajTrue;
			    }
			}
		    }
		    /*DIAGNOSTIC 
		    else
			ajFmtPrintF(flog, "ALIGNMENT FOUND OK\n");  */
		    
		    
		    /* atm_idx and seqres_idx are set to give the
		       index into ATOM and SEQRES sequences
		       respectively for the position of match of
		       N-terminal residue of substring (if founderr is
		       True this will be the position of the
		       N-terminal mismatch residue) */
		    atm_idx= (int) ((atm_ptr-ajStrGetPtr(seq[i]))/sizeof(char));

		    if(founderr)
			seqres_idx= (int) (((loc_ptr-1)-
					    ajStrGetPtr(pdbfile->seqres[i]))
					   /sizeof(char));
		    else
			seqres_idx= (int) ((loc_ptr-
					    ajStrGetPtr(pdbfile->seqres[i]))
					   /sizeof(char));
		    
		    /*DIAGNOSTIC
		      ajFmtPrintF(flog, "seqres_idx : %d\n", seqres_idx); */
		    


		    /* If there was a mismatch residue, idx_misfit_atm and 
		       idx_misfit_seqres will give the index into the ATOM 
		       and SEQRES sequences respectively for its position */
		    if(founderr)
		    {
			idx_misfit_atm    = atm_idx;
			idx_misfit_seqres = seqres_idx;
		    }
		    

      
		    /*CHECK TO SEE IF THERE IS SPACE TO FIT THE REMAINDER OF 
		      THE ATOM SEQUENCE IN THE SEQRES SEQUENCE GIVEN THIS 
		      ALIGNMENT */
		    if((nres[i] - atm_idx) > (len - seqres_idx))
			break;
		    
		    /**************************************************/
		    /* This will have to change for 1st residue       */
		    /**************************************************/
		    
		    /* Try and find an exact match within the gap for the 
		       mismatch residue */
		    fixed = ajFalse;

		    if(founderr)
		    {
			/*DIAGNOSTIC 
			ajFmtPrintF(flog,
			            "About to try (seqres_idx_last: %d,  "
				    "seqres_idx: %d) ...\n", seqres_idx_last, 
				    seqres_idx); */
			
			
			aa_last = ajStrGetCharFirst(substr);

			for(z=seqres_idx_last+1; z<seqres_idx; z++)
			{
			    /*DIAGNOSTIC 
			     ajFmtPrintF(flog, "Trying ATOM:SEQRES  %c:%c\n", 
			      aa_last,pdbfile->seqres[i]->Ptr[z]);*/
			    
			    if(pdbfile->seqres[i]->Ptr[z] == aa_last)
			    {
				nmismatches--;
				founderr=ajFalse;
				fixed = ajTrue;
				

				/* Assign residue number */
				ajIntPut(&idx[i], atm_idx, z+1);

				for(k=0,  y=atm_idx+1,  z=seqres_idx+1; 
				    k<siz_substr-1; k++,  y++,  z++)
				    ajIntPut(&idx[i], y, z+1);

				break;
			    }
			}
		    }
		    
		    if(!fixed)
		    {
			/*DIAGNOSTIC 
			 ajFmtPrintF(flog, "FAILED TO FIX\n"); */
			/* Assign residue number */
			for(k=0, y=atm_idx,  z=seqres_idx; k<siz_substr; 
			    k++,  y++,  z++)
			    ajIntPut(&idx[i], y, z+1);

		    }
		    


		    /*Mark up last SEQRES residue as having been done */
		    if(y==nres[i])
			done_end = ajTrue;





		    /* If the substring matched but with a residue
		       mismatch for the 1st residue */
		    /**************************************************/
		    /* This block should only be called if we         */
		    /* can't fit the mismatch residue in somewhere.   */
		    /**************************************************/
		    if(founderr)
		    {
			/* a will give the number of the first coordinate line 
			   for the mismatch residue from the ATOM records*/
			
			a = PdbfileFindLine(pdbfile, i+1, x, 
					    ajIntGet(num[i], 
						     idx_misfit_atm));

			/* Get the id of the mismatch residue 
			   in the SEQRES sequence.  */
			ajResidueToTriplet(
                            pdbfile->seqres[i]->Ptr[idx_misfit_seqres], 
                            &aa_misfit);
			
			
			/* To give correct index into SEQRES records in 
			   original PDB file, subtract j to account for 
			   modifications to the N-terminus that were made 
			   for missing residues relative to ATOM sequence.
			   A further 1 is added to give a number starting 
			   from 1 (rather than 0)*/
			
			/*DIAGNOSTIC 
			  ajFmtPrintF(flog, "a : %d\n". a);
			      ajFmtPrintF(flog, "pdbfile->rtype[a] : %S\n", pdbfile->rtype[a]);
			      ajFmtPrintF(flog, "pdbfile->pdbn[a] : %S\n",
					pdbfile->pdbn[a]); */
			  
			
			ajFmtPrintS(&msgbit,  "%S %S %S %d;    ", 
				    pdbfile->rtype[a], pdbfile->pdbn[a], 
				    aa_misfit, idx_misfit_seqres-j+1);
			

			/*DIAGNOSTIC 
			  ajFmtPrintS(&msgbit, "ATOM residue %d (%c) vs "
			  "SEQRES residue %d (%c).   ", 
			  ajIntGet(num[i], atm_idx), ajStrGetCharFirst(substr), 
			  seqres_idx+1, pdbfile->seqres[i]->Ptr[seqres_idx]); */


			ajStrAppendS(&msgstr, msgbit);

			
			pdbfile->seqres[i]->Ptr[seqres_idx] =
                            ajStrGetCharFirst(substr);
		    }
      
		    
		    /* atm_ptr and seqres_ptr now point to 1 residue past
		       the end of the match of the substring in the ATOM 
		       and SEQRES sequences respectively. */

		    atm_ptr+=siz_substr;
      
		    if(founderr)
			seqres_ptr=(loc_ptr-1)+siz_substr;
		    else
			seqres_ptr=loc_ptr+siz_substr;
      
		    siz_substr=nres[i]-(atm_idx+siz_substr);



		    /**************************************************/
		    /* Must assign index into SEQRES for              */
		    /* C-terminal residue of substring                */
		    /**************************************************/


		    /* seqres_idx_last is set to give the index into SEQRES 
		       sequence for the position of match of C-terminal 
		       residue of substring + 1 */
		    seqres_idx_last = (int) z-1;
		}
		
		
		
		
		
		/*Check to ensure that position for last residue has been 
		  worked out */
		if((done_end) && (nmismatches<=lim))
		{

		    
		    if(nmismatches)
			ajFmtPrintF(flog, "%-15s%d (%c) %d %S\n", "GAPPED", 
				    i+1,  
				    ajChararrGet(pdbfile->chid, i),
				    nmismatches, msgstr);
		    else
			ajFmtPrintF(flog, "%-15s%d (%c)\n", "GAPPEDOK", i+1,  
				    ajChararrGet(pdbfile->chid, i));
		    		    
		    
		    /* Residue numbering is correct after alignment 
		       (acceptable number of mismatches) */
		    if(x==0)
			pdbfile->resn1ok[i]=ajTrue;
		    else
			pdbfile->resn1ok[i]=ajFalse;		       
		    
		    /*DIAGNOSTIC		 
		     ajFmtPrintF(flog,
		     "STEP6 OK %d mismatches\n", nmismatches);   */

		    done=ajTrue;
		    break;
		}
		
		
		/*DIAGNOSTIC	 
		ajFmtPrintF(flog, "STEP6 **NOT** OK %d mismatches\n",
		            nmismatches); */


		/* Otherwise, agreement could not be found - unacceptable 
		   number of mismatches.
		   Restore the original seqres sequence*/
		ajStrAssignS(&(pdbfile->seqres[i]), tmpseqres);
		pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);

	    } /* for(x=0;x<2;x++) */

	    if(done)
		break;
	}
	
	
	
		
	/* After trying 6 different alignment steps with (potentially 
	   modified) seqres sequences and 2 (possibly different) sequences
	   derived from the ATOM records an alignment with agreement in 
	   residue numbering still cannot be found. 
	   Use data from ATOM records only - use seq1 (all residues) and 
	   presume there are no missing residues.*/
	if(!done)
	{
	    ajFmtPrintF(flog, "%-15s%d (%c)\n", "NOMATCH", i+1, 
			ajChararrGet(pdbfile->chid, i));


	    ajStrAssignS(&(pdbfile->seqres[i]), seq1[i]);
	    pdbfile->nres[i]=ajStrGetLen(pdbfile->seqres[i]);

	    for(k=0;k<nres1[i];k++)
		ajIntPut(&idx[i], k, k+1);

	    pdbfile->resn1ok[i]=ajTrue;

	    
	}
	else
	{
	    if(j)
		ajFmtPrintF(flog, "%-15s%d (%c) %d\n", "MISSNTERM", i+1, 
			    ajChararrGet(pdbfile->chid, i), j);
	    

	    if(!ajStrMatchS(seq1[i], seq2[i]))
	    {  
		if(x==0)
		    ajFmtPrintF(flog, "%-15s%d\n", "ALTERNOK", i+1);   
		else	
		    ajFmtPrintF(flog, "%-15s%d\n", "HETEROK", i+1);   
		
	    }
	}
    }
    

    
    
    /* Write the index arrays */
    for(i=0;i<pdbfile->nchains; i++)
    {
	if(!pdbfile->chainok[i])
	    continue;


	if(pdbfile->resn1ok[i])
	    for(j=0;j<nres1[i];j++)
		ajIntPut(&idx_full[i], ajIntGet(num1[i], j), 
			 ajIntGet(idx[i], j));
	else
	    for(j=0;j<nres2[i];j++)
		ajIntPut(&idx_full[i], ajIntGet(num2[i], j), 
			 ajIntGet(idx[i], j));
    }
    
    

    
    /* Write the resni element of the Pdbfile object.
       These are the residue numbers that give the correct index into 
       the finalised seqres sequence */
    
    
    
    for(i=pdbfile->idxfirst;i<pdbfile->nlines;i++)
    {
	if(pdbfile->linetype[i]==PDBPARSE_COORD)
	{	
	    if(pdbfile->resn1ok[pdbfile->chnn[i]-1])
	    {
		pdbfile->resni[i]=
		    ajIntGet(idx_full[pdbfile->chnn[i]-1], 
			     pdbfile->resn1[i]);	
		/*DIAGNOSTIC 
		 ajFmtPrintF(flog, "Got position %d (%d) ok\n", 
		  pdbfile->resn1[i], 
		  ajIntGet(idx_full[pdbfile->chnn[i]-1], 
		  pdbfile->resn1[i]));  */

	    }

	    else
	    {
		pdbfile->resni[i]=
		    ajIntGet(idx_full[pdbfile->chnn[i]-1], 
			     pdbfile->resn2[i]);
		/*DIAGNOSTIC ajFmtPrintF(flog, "Got position %d (%d) ok\n", 
		  pdbfile->resn2[i], 
		  ajIntGet(idx_full[pdbfile->chnn[i]-1], 
		  pdbfile->resn2[i]));  */
	    }
	}
    }
    

    
    
    /* DIAGNOSTIC 
       
       ajFmtPrintF(flog, "\n\n\n"); 
       
       seq=seq1;
       
       for(i=0;i<pdbfile->nchains; i++)
       {
       if(!pdbfile->chainok[i])
       {
       ajFmtPrintF(flog, 
       "Chain %d\nSEQRES %S\nCHAIN NOT OK\n\n\n", 
       i+1,
       pdbfile->seqres[i]);
       
       continue;
       }
       
       ajFmtPrintF(flog, 
       "Chain %d\nSEQRES %S\nSEQ__1 %S\nSEQ__2 %S\n\n\n", 
       i+1,
       pdbfile->seqres[i],
       seq1[i],
       seq2[i]);
       
       }
       
       
       for(i=0;i<pdbfile->nchains; i++)
       {
       if(!pdbfile->chainok[i])
       continue;	
       
       if(nres1[i] > nres2[i])
       max = nres1[i];
       else
       max = nres2[i];
       
       ajFmtPrintF(flog, "CHAIN %d\n", i+1);
       ajFmtPrintF(flog, "seqres %S\n", pdbfile->seqres[i]);
       
       
       ajFmtPrintF(flog, "%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n", 
       "RES", "NUM", "SEQ1", "RESN1", "SEQ2", "RESN2", "IDX");
       
       for(j=0;j<max;j++)
       {
       if(j<nres1[i] && j<nres2[i])
       {
       
       ajFmtPrintF(flog, "%-6s%-6d%-6c%-6d%-6c%-6d%-6d\n", 
       "RES", 
       j+1,
       ajStrGetCharPos(seq1[i], j),
       ajIntGet(num1[i], j),     
       ajStrGetCharPos(seq2[i], j),
       ajIntGet(num2[i], j),	
       ajIntGet(idx[i], j));
       }
       
       else if (j<nres1[i])
       {
       ajFmtPrintF(flog, "%-6s%-6d%-6c%-6d%-6c%-6c", 
       "RES", 
       j+1,
       ajStrGetCharPos(seq1[i], j), 
       ajIntGet(num1[i], j),     
       '.', '.');
       
       
       if(pdbfile->resn1ok[i])
       {
       ajFmtPrintF(flog, "%-6d\n", ajIntGet(idx[i], j));
       }
       
       else
       {
       ajFmtPrintF(flog, "%-6c\n", '.');
       }
       
       
       }
       
       else 
       {
       ajFmtPrintF(flog, "%-6s%-6d%-6c%-6c%-6c%-6d", 
       "RES", 
       j+1,
       '.', '.',
       ajStrGetCharPos(seq2[i], j), 
       ajIntGet(num2[i], j));
       
       if(!pdbfile->resn1ok[i])
       {
       ajFmtPrintF(flog, "%-6d\n", ajIntGet(idx[i], j));
       }
       
       else
       {
       ajFmtPrintF(flog, "%-6c\n", '.');
       }
       
       
       }
       
       }
       
       }
       
       */
    
    
    
    /* Tidy up and return */
    ajStrDel(&aa_misfit);
    ajStrDel(&seqbit);
    ajStrDel(&msgstr);
    ajStrDel(&msgbit);
    AJFREE(insert);
    ajStrDel(&tmpseqres);
    ajStrDel(&bit);
    ajStrDel(&substr);    
    ajStrDel(&substr2);    
    
    for(i=0;i<pdbfile->nchains; i++)
    {
	if(!pdbfile->chainok[i])
	    continue;


	ajStrDel(&seq1[i]);
	ajStrDel(&seq2[i]);


	ajIntDel(&num1[i]);
	ajIntDel(&num2[i]);
	
	ajIntDel(&idx[i]);
	ajIntDel(&idx_full[i]);

    }
    
    AJFREE(seq1);
    AJFREE(seq2);
    
    
    AJFREE(num1);
    AJFREE(num2);
    
    AJFREE(idx);
    AJFREE(idx_full);
    
    AJFREE(nres1);
    AJFREE(nres2);
    
    
    return ajTrue;
}




/* #funcstatic diagnostic  **************************************************
**
** For printing out diagnostics for pdbparse build
** 
**
** #param [r] pdbfile [AjPPdbfile]  Pdbfile object
** #param [r] n       [ajint ]        Flag for controlling output
** 
** #return [AjBool]  True on success, False otherwise
** ##
****************************************************************************/
/*THIS_DIAGNOSTIC 
//static void diagnostic(AjPPdbfile pdbfile, ajint n)
//{
//    ajint i;
//    i=0;
//    
//    
//
//         
//       
//       if(n==0)
//       {
//       ajFmtPrintF(tempfile, "nchains: %d\n", pdbfile->nchains);
//       for(i=0;i<pdbfile->nchains; i++)
//       ajFmtPrintF(tempfile, "chid: %c\n", 
//		   ajChararrGet(pdbfile->chid, i));
//       for(i=0;i<pdbfile->nchains; i++)
//       {
//       ajFmtPrintF(tempfile, "seqres %d: %S\n", i, pdbfile->seqres[i]);
//       }
//       
//       
//       ajFmtPrintF(tempfile, "tercnt: %d\n", pdbfile->tercnt);
//       ajFmtPrintF(tempfile, "COMPND: %S\n", pdbfile->compnd);
//       ajFmtPrintF(tempfile, "SOURCE: %S\n", pdbfile->source);
//       ajFmtPrintF(tempfile, "reso: %f\n", pdbfile->reso);
//       if(pdbfile->method == ajXRAY)
//       ajFmtPrintF(tempfile, "method: ajXRAY\n");
//       else
//       ajFmtPrintF(tempfile, "method: ajNMR\n");
//       }
//       else if(n==1)
//       {
//       for(i=0;i<pdbfile->nlines;i++)
//       {
//       ajFmtPrintF(tempfile, "%-5S",  pdbfile->pdbid);
//       
//       if(pdbfile->linetype[i]==PDBPARSE_IGNORE)
//       ajFmtPrintF(tempfile, "%-10s",  "IGNORE");
//       else if(pdbfile->linetype[i]==PDBPARSE_COORD)
//       ajFmtPrintF(tempfile, "%-10s",  "COORD");
//       else if(pdbfile->linetype[i]==PDBPARSE_COORDHET)
//       ajFmtPrintF(tempfile, "%-10s",  "COORDHET");
//       else if(pdbfile->linetype[i]==PDBPARSE_COORDGP)
//       ajFmtPrintF(tempfile, "%-10s",  "COORDGP");
//       else if(pdbfile->linetype[i]==PDBPARSE_COORDWAT)
//       ajFmtPrintF(tempfile, "%-10s",  "COORDWAT");
//       else if(pdbfile->linetype[i]==PDBPARSE_TER)
//       ajFmtPrintF(tempfile, "%-10s",  "TER");
//       else if(pdbfile->linetype[i]==PDBPARSE_MODEL)
//       ajFmtPrintF(tempfile, "%-10s",  "MODEL");
//       
//       
//       
//       ajFmtPrintF(tempfile, 
//		   "M%-2dC%-2d%-6S%-5d%-5d%-4B%-4S%-4S%-7.3f%-7.3f"
//       "%-7.3f%-6.3f%-6.3f\n", 
//       pdbfile->modn[i],
//       pdbfile->chnn[i],
//       pdbfile->pdbn[i], 
//       pdbfile->resn1[i],
//       pdbfile->resn2[i],
//       pdbfile->oddnum[i], 
//       pdbfile->atype[i], 
//       pdbfile->rtype[i], 
//       pdbfile->x[i], 
//       pdbfile->y[i], 
//       pdbfile->z[i], 
//       pdbfile->o[i], 
//       pdbfile->b[i]);
//       
//       }	
//       }
//       
//
//}
*/




/* @funcstatic  PdbfileToPdb ************************************************
**
** Reads data from a Pdbfile object and writes a Pdb object.
** Chains that did not contain at least the user-defined threshold number of
** amino acid residues are discarded, i.e. are NOT copied and will NOT appear
** in the output file that is eventually generated.
** 
**
** @param [w] ret     [AjPPdb *]     Pdb object pointer
** @param [u] pdb     [AjPPdbfile]  Pdbfile object
** 
** @return [AjBool]  True on success, False otherwise
** @@
****************************************************************************/

static AjBool PdbfileToPdb(AjPPdb *ret, AjPPdbfile pdb)
{
    ajint      i    =0;          /* Loop counter */
    ajint      idx  =0;          /* Index into chain array */
    ajint      j    =0;		 /* Loop counter */
    AjPAtom    atm  =NULL;	 /* Atom object */
    AjPResidue res  =NULL;	 /* Residue object */
    ajint      nchn =0;          /* No. chains that have min. no. of aa's */
    AjPInt     lookup;           /* Array of chain numbers for chains in 
				    ret for all chains in pdb.A '0' is 
				    given for chains with < threshold no.
				    of aa's */
    ajint    chn=0;
    ajint    rn_last = -100000;
    ajint    mn_last = -100000;
    
    ajint    eNum;  
    AjPStr   eId=NULL;   
    char     eType;    
    ajint    eClass;   
    
    
    if( !ret || !pdb)
    {
	ajWarn("Bad args passed to PdbfileToPdb");

	return ajFalse;
    }

    if((*ret))
    {
	ajWarn("Bad args passed to PdbfileToPdb - PDB object exists");

	return ajFalse;
    }
    

    eId = ajStrNew();
    
    
    lookup = ajIntNewRes(pdb->nchains);
    ajIntPut(&lookup, pdb->nchains-1, 0);

    for(nchn=0, i=0;i<pdb->nchains;i++)
	if((pdb)->chainok[i])
	{
	    nchn++;
	    ajIntPut(&lookup, i, nchn);
	}
    
    *ret=ajPdbNew(nchn);
    (*ret)->Nchn = nchn;
    

    ajStrAssignS(&((*ret)->Pdb), pdb->pdbid);
    ajStrAssignS(&((*ret)->Compnd), pdb->compnd);
    ajStrAssignS(&((*ret)->Source), pdb->source);    
    (*ret)->Method = pdb->method;
    (*ret)->Reso   = pdb->reso;
    (*ret)->Nmod   = pdb->modcnt;
    (*ret)->Ngp    = pdb->ngroups;
    /*    (*ret)->Nchn   = pdb->nchains; */

    for(i=0; i<pdb->ngroups; i++)
	ajChararrPut(&((*ret)->gpid), i, ajChararrGet(pdb->gpid, i));
    

    for(idx=-1, i=0;i<pdb->nchains;i++)
    {
	if((pdb)->chainok[i])
	    idx++;
	else
	    continue;
	
	(*ret)->Chains[idx]->Id   = ajChararrGet((pdb)->chid, i);


	/* These counts are no longer made from the PDB records. They 
	   are only made if the file is annotated with stride secondary
	   structure info by using pdbstride */
	/*
	(*ret)->Chains[idx]->numHelices = (pdb)->numHelices[i];
	(*ret)->Chains[idx]->numStrands = (pdb)->numStrands[i];
	(*ret)->Chains[idx]->numSheets  = (pdb)->numSheets[i];
	(*ret)->Chains[idx]->numTurns   = (pdb)->numTurns[i];
	*/

	(*ret)->Chains[idx]->Nres = (pdb)->nres[i];
	(*ret)->Chains[idx]->Nlig = (pdb)->nligands[i];
	ajStrAssignS(&((*ret)->Chains[idx]->Seq), (pdb)->seqres[i]);
    }
    
    
    for(j=(pdb)->idxfirst; j<(pdb)->nlines; j++)
    {
	if(((pdb)->linetype[j]==PDBPARSE_COORD) ||
	   ((pdb)->linetype[j]==PDBPARSE_COORDHET) ||
	   ((pdb)->linetype[j]==PDBPARSE_COORDGP) ||
	   ((pdb)->linetype[j]==PDBPARSE_COORDWAT))
	{
	    /* Skip this line if it for a heterogenous (duplicate) 
	       position  */
	    if( (!(pdb)->resn1ok[(pdb)->chnn[j]-1]) && (pdb)->oddnum[j])
		continue;
		
	    atm = ajAtomNew();

	    atm->Mod = (pdb)->modn[j];
	    /* atm->Chn = (pdb)->chnn[j]; */
	    atm->Chn = ajIntGet(lookup, (pdb)->chnn[j]-1);
	    
	    

	    atm->Gpn = (pdb)->gpn[j];

	    if((pdb)->linetype[j]==PDBPARSE_COORDHET)
		atm->Type = 'H';
	    else if((pdb)->linetype[j]==PDBPARSE_COORDGP)
		atm->Type = 'H';
	    else if((pdb)->linetype[j]==PDBPARSE_COORD)
		atm->Type = 'P';
	    else if((pdb)->linetype[j]==PDBPARSE_COORDWAT)
		atm->Type = 'W';

	    atm->Idx = (pdb)->resni[j];
	    
	    ajStrAssignS(&atm->Pdb, (pdb)->pdbn[j]);
	    
		
	    if(((pdb)->linetype[j]==PDBPARSE_COORDHET) ||
	       ((pdb)->linetype[j]==PDBPARSE_COORDGP) ||
	       ((pdb)->linetype[j]==PDBPARSE_COORDWAT))
		atm->Id1='.';
	    else
		ajResidueFromTriplet((pdb)->rtype[j],&atm->Id1);
		
	    ajStrAssignS(&atm->Id3, (pdb)->rtype[j]);
	    ajStrAssignS(&atm->Atm, (pdb)->atype[j]);
	    atm->X=(pdb)->x[j];
	    atm->Y=(pdb)->y[j];
	    atm->Z=(pdb)->z[j];
	    atm->O=(pdb)->o[j];
	    atm->B=(pdb)->b[j];


	    ajStrAssignS(&eId, (pdb)->elementId[j]);
	    eNum   = (pdb)->elementNum[j];
	    eType  = (pdb)->elementType[j];
	    eClass = (pdb)->helixClass[j];
	    

	    if((pdb)->linetype[j]==PDBPARSE_COORDGP)
		ajListPushAppend((*ret)->Groups, atm);
	    else if((pdb)->linetype[j]==PDBPARSE_COORDWAT)
		ajListPushAppend((*ret)->Water, atm);
	    else
	    {
		if((pdb)->chainok[(pdb)->chnn[j]-1])
		{
		    /* ajListPushAppend((*ret)->Chains[(pdb)->chnn[j]-1]->Atoms,
		       atm); */
		    /* ajListPushAppend((*ret)->Chains[ajIntGet(lookup,
		       (pdb)->chnn[j]-1)-1]->Atoms, atm); */
		    chn = ajIntGet(lookup, (pdb)->chnn[j]-1)-1;
		    
		    ajListPushAppend((*ret)->Chains[chn]->Atoms, atm); 

		    /* Write residue object */
		    if(atm->Type=='P')
		    {
			/* New model */
			if(atm->Mod != mn_last)
			{
			    rn_last = -100000;
			    mn_last = atm->Mod;
			}

			/* New residue */
			if(atm->Idx != rn_last)
			{
			    res = ajResidueNew();

			    res->Mod     = atm->Mod;
			    res->Chn     = atm->Chn;
			    res->Idx     = atm->Idx;
			    ajStrAssignS(&res->Pdb, atm->Pdb);
			    res->Id1     = atm->Id1;
			    ajStrAssignS(&res->Id3, atm->Id3);

			    res->eNum    = eNum;  
			    ajStrAssignS(&res->eId, eId);
			    res->eType   = eType;    
			    res->eClass  = eClass;   

			    ajListPushAppend((*ret)->Chains[chn]->Residues,
					     (void *) res);
			    rn_last = atm->Idx;
			}
		    }
		}
		else
		    ajAtomDel(&atm);
	    }
	}
	else
            continue;
    }
    
    ajIntDel(&lookup);
    ajStrDel(&eId);

    return ajTrue;
}




/* @funcstatic PdbfileFindLine ************************************************
**
** Returns the line number of the first instance of a line with a specified 
** residue and chain number. 	
** 
** @param [r] pdb     [const AjPPdbfile] Pdbfile object pointer
** @param [r] chn     [ajint] Chain number
** @param [r] which   [ajint] 0 or 1, refer to resn1 or resn2 residue
** @param [r] pos     [ajint] Residue number
** 
** @return [ajint]  Line number (index, i.e. starts from 0).
** @@
****************************************************************************/

static ajint  PdbfileFindLine(const AjPPdbfile pdb, ajint chn, ajint which, 
			      ajint pos)
{
    ajint a=0;
    /* a will give the number of the first coordinate line 
       for the mismatch residue from the ATOM records*/
			



    for(a=pdb->idxfirst;a<pdb->nlines;a++)
	if(pdb->linetype[a]==PDBPARSE_COORD &&
	   pdb->chnn[a] == chn)
	    /* First sequence (all residues) derived for atom records */
	    /* OR Second sequence (excluding certain residues) derived for
	       atom records */	
	    if(((which==0) && (pos == pdb->resn1[a])) ||
	       ((which==1) && (pos == pdb->resn2[a])))
		break;
			
			
    if(a==pdb->nlines)
	ajFatal("Unexpected loop failure in PdbfileFindLine. "
		"Email jison@hgmp.mrc.ac.uk\n");

    return a;
}




/* @funcstatic  PdbfileChain ************************************************
**
** Finds the chain number for a given chain identifier in a pdbfile structure
**
** @param [r] id  [char]        Chain identifier
** @param [r] pdb [const AjPPdbfile] Pdbfile object
** @param [w] chn [ajint *]     Chain number
**
** @return [AjBool] True on success
** @@
****************************************************************************/

static AjBool PdbfileChain(char id, const AjPPdbfile pdb, ajint *chn)
{
    ajint a;
 
    for(a=0;a<pdb->nchains;a++)
    {
	if(toupper((int) ajChararrGet(pdb->chid, a)) == toupper((int) id))
	{
	    *chn=a+1;

	    return ajTrue;
	}

	/* Cope with chain id's of ' ' (which might be given as '.' in 
	   the Pdbfile object) */
	if((id==' ')&&(ajChararrGet(pdb->chid, a)=='.'))
	{
	    *chn=a+1;

	    return ajTrue;
	}
    }

    /* A '.' may be given as the id for domains comprising more than one
       chain*/
    if(id=='.')
    {
	*chn=1;

	return ajTrue;
    }
    
	
    return ajFalse;
}




/* @funcstatic  WriteElementData ********************************************
**
** Reads the secondary structure information from an Elements object 
** and writes equivalent variables in an Pdbfile object.
** 
** @param [w] pdbfile [AjPPdbfile] Pdbfile object
** @param [u] flog    [AjPFile] Pointer to log file (build diagnostics)
** @param [r] elms    [const AjPElements] Elements object pointer
** 
** @return [AjBool]  True on success, False otherwise
** @@
****************************************************************************/

static AjBool WriteElementData(AjPPdbfile pdbfile, AjPFile flog, 
			       const AjPElements elms)
{
    ajint x           =0;
    ajint y           =0;
    ajint z           =0;
    ajint modn        =0;	/* Model number */
    
    ajint idx         =0;	/* idx into lines in Pdbfile object */
    ajint idx_start   =0;	/* Line index of start of element */
    ajint idx_end     =0;	/* Line index of end of element */
    ajint idx_last    =0;	/* Line index of last line to try */
    ajint idx_tmp     =0;	/* Temp. line index */
    
    ajint chn         =0;	/* Chain id of current element as integer */
    
    AjPInt nsheets    =NULL;	/* Number of sheets in each chain */
    AjPStr *lastids   =NULL;	/* Last sheet identifier read in for each 
				   chain */
    AjBool found_start=ajFalse;	/* Whether start residue of the
				   current element has been found yet */
    AjBool found_end  =ajFalse;	/* Whether the end residue of the current 
				   element has been found yet */
    ajint  n_unknown  =0;	/* No. of unknown chain ids */
    AjPChar unknowns  =NULL;	/* Unknown chain ids */
    AjBool found      =ajFalse;	/* True if we have already reported an error 
				   message for the unknown chain id */


    
    /* Check args */
    if(!pdbfile || !(elms) || !(flog))
    {
	ajWarn("NULL arg passed to WriteElementData");

	return ajFalse;
    }
    
    
    /* Assign default values for secondary structure fields in 
       Pdbfile object */
    for(x=0; x<pdbfile->nlines; x++)
    {
	pdbfile->elementType[x]='.';
	ajStrAssignC(&pdbfile->elementId[x], ".");
    }
    
    
    
    /* Allocate memory */
    nsheets = ajIntNewRes(pdbfile->nchains); 
    ajIntPut(&nsheets, pdbfile->nchains, 0);
    
    
    unknowns = ajChararrNew();
    
    
    AJCNEW0(lastids, pdbfile->nchains);

    for(x=0;x<pdbfile->nchains;x++)
    {
	lastids[x]=ajStrNew();
	/* Assign a silly value for starters */
	ajStrAssignC(&lastids[x], "?????");
    }
    
    
    
/*   ajFmtPrint("LOOK HERE x:%d elms->n:%d\n", x, elms->n); */
    
    for(modn=1; modn<=pdbfile->modcnt; modn++)	{
	
	/* Loop for each element. 
	   Set the current line to the first line in the file*/
	for(idx=pdbfile->idxfirst, 
	    x=0;x<elms->n; x++)
	{
	    /* Find the chain number of the current element */
	    if(!PdbfileChain(elms->elms[x]->chainId, pdbfile, &chn))
	    {
		/* Only report errors once for each unknown id */
		if(modn==1)
		{
		    for(found=ajFalse, y=0; y<n_unknown; y++)
			if(ajChararrGet(unknowns, y)==elms->elms[x]->chainId)
			{
			    found=ajTrue;
			    break;
			}
		
		    if(!found)
		    {
			ajFmtPrintF(flog, "%-15s%c %d\n", "SECCHAIN", 
				    elms->elms[x]->chainId, idx);
			ajChararrPut(&unknowns, n_unknown, 
				     elms->elms[x]->chainId);
			n_unknown++;
		    }
		
		}
	
		continue;
	    }
	    
	    
	    /* Only want to do this once (for the first model ) */
	    if(modn==1)
	    {
	    

		/* Make a count of the number of beta sheets */
		if(elms->elms[x]->elementType == 'E')
		    if(!ajStrMatchS(lastids[chn-1], elms->elms[x]->elementId))
		    {
			ajIntInc(&nsheets, chn-1); 
			ajStrAssignS(&lastids[chn-1], elms->elms[x]->elementId);
		    }

	    }
	    
	    
	    /* Loop for two passes.  z is for efficiency, if z==0 it
	       will check from the current position up to the last
	       coordinate line, if z==1 it will check from the first
	       coordinate line up to the last position checked*/
	    for(found_start=ajFalse, found_end=ajFalse,
		z=0; z<2; z++)
	    {
		if(z==0)
		    idx_last = pdbfile->nlines;
		else
		{
		    idx=pdbfile->idxfirst;
		    idx_last = idx_tmp;
		}
	    

		/* Find the start and end of the current element (as an 
		   index into the line array) */
		for(;idx<idx_last; idx++)
		{
		    /* Find the correct chain and skip lines that are not for 
		       amino acids*/
		    if((pdbfile->chnn[idx] != chn) || 
		       (pdbfile->linetype[idx]!=PDBPARSE_COORD) || 
		       (pdbfile->modn[idx] != modn))
			continue;

		    /* We have not found the start residue yet */
		    if(!found_start)
			if(ajStrMatchS(elms->elms[x]->initSeqNum, 
				      pdbfile->pdbn[idx]))
			    if(ajStrMatchS(elms->elms[x]->initResName, 
					  pdbfile->rtype[idx]))
			    {
				/* Residue number for start found and residue
				   type matches */
				idx_start = idx;
				/* printf("found_start !\n"); */
				
				found_start=ajTrue;
			    }
		
		    if(ajStrMatchS(elms->elms[x]->endSeqNum, 
				  pdbfile->pdbn[idx]))
			if(ajStrMatchS(elms->elms[x]->endResName, 
				      pdbfile->rtype[idx]))
			{
			    /* Residue number for end found and residue 
			       type matches */
			    idx_end = idx;

			
/*			    printf("idx_end: %d ...", idx_end); */
			    

			    /* Set the index to the LAST atom of the residue */

			    for(; idx_end < pdbfile->nlines; idx_end++)
			    {
				if(pdbfile->linetype[idx_end]!=
				   PDBPARSE_COORD)
				    continue;
				
				if(!ajStrMatchS(elms->elms[x]->endSeqNum, 
					       pdbfile->pdbn[idx_end]) ||
				   !ajStrMatchS(elms->elms[x]->endResName,
					       pdbfile->rtype[idx_end])||
				   pdbfile->chnn[idx_end] != chn ||
				   pdbfile->modn[idx_end] != modn)
				    break;
			    }
			    
			    idx_end--;
			    
			    /*
			    printf(" %d\n", idx_end);

			    ajFmtPrint("found_end !\n"
			    "elms->elms[x]->endSeqNum   "
			    ": pdbfile->pdbn[idx_end]  ===  %S : %S\n"
			    "elms->elms[x]->endResName  "
			    ": pdbfile->rtype[idx_end] ===  %S : %S\n"
			    "pdbfile->chnn[idx_end]  "
			    ": chn                        ===  %d : %d\n"
			    "pdbfile->modn[idx_end]  "
			    ": modn                       ===  %d : %d\n", 
			    elms->elms[x]->endSeqNum,
			    pdbfile->pdbn[idx_end+1],	
			    elms->elms[x]->endResName,
			    pdbfile->rtype[idx_end+1],
			    pdbfile->chnn[idx_end+1],
			    chn,
			    pdbfile->modn[idx_end+1],
			    modn);
			    */

			    found_end=ajTrue;
			    idx_tmp = idx;
			    break;
			}
		
		}
	    
		if(found_start && found_end)
		    break; 
	    }
	    
	    
	    if(!found_start || !found_end)
	    {
		if(!found_start && !found_end)
		{
		    ajFmtPrintF(flog, "%-15s%d %d %S %S %S %S\n", "SECBOTH", 
				chn, modn, elms->elms[x]->initResName, 
				elms->elms[x]->initSeqNum, 
				elms->elms[x]->endResName, 
				elms->elms[x]->endSeqNum);
		
		}
		else if(!found_start)
		{
		    ajFmtPrintF(flog, "%-15s%d %d %S %S\n", "SECSTART", 
				chn, modn, elms->elms[x]->initResName, 
				elms->elms[x]->initSeqNum);
		}
		else if(!found_end)
		{
		    ajFmtPrintF(flog, "%-15s%d %d %S %S\n", "SECEND", chn, 
				modn, elms->elms[x]->endResName, 
				elms->elms[x]->endSeqNum);
		}
	    }
	    
	    
	    
	    
	
	
	    /* Assign secondary structure fields in Pdbfile object */
	    for(idx=idx_start; idx<=idx_end; idx++)
	    {
		pdbfile->elementNum[idx] = elms->elms[x]->elementNum;
		pdbfile->elementType[idx] = elms->elms[x]->elementType;

		if(elms->elms[x]->elementType == 'H')
		    pdbfile->helixClass[idx] = elms->elms[x]->helixClass;

		ajStrAssignS(&pdbfile->elementId[idx], 
			  elms->elms[x]->elementId);
	    }


	    /* Only want to do this once */
	    if(modn==1)
	    {
		if(elms->elms[x]->elementType == 'H')
		    pdbfile->numHelices[chn-1]++;	
		else if(elms->elms[x]->elementType == 'E')
		    pdbfile->numStrands[chn-1]++;	
		else if(elms->elms[x]->elementType == 'T')
		    pdbfile->numTurns[chn-1]++;	
	    }
	}
    }

    

    
    /* Assign number of sheets */
    if(modn==1)
	for(x=0; x<pdbfile->nchains; x++)
	    pdbfile->numSheets[x] = ajIntGet(nsheets, x);
    
    
    /* Tidy up and return */
    ajIntDel(&nsheets);

    for(x=0;x<pdbfile->nchains;x++)
	ajStrDel(&lastids[x]);

    AJFREE(lastids);
    
    ajChararrDel(&unknowns);


    return ajTrue;
}




/* ======================================================================= */
/* =========================== constructors ============================== */
/* ======================================================================= */




/* @section Constructors ****************************************************
**
** All constructors return a pointer to a new instance. It is the 
** responsibility of the user to first destroy any previous instance. The 
** target pointer does not need to be initialised to NULL, but it is good 
** programming practice to do so anyway.
**
****************************************************************************/




/* @func  ajPdbReadRawNew **************************************************
**
** Reads a pdb file and returns a pointer to a filled Pdb object. 
** 
** The pdb id is derived from the file name and extension of the pdb file 
** (these are passed in by argument).
** 
** @param [u] inf            [AjPFile] Pointer to pdb file 
** @param [r] pdbid          [const AjPStr]  PDB id code of pdb file
** @param [r] min_chain_size [ajint]  Minimum number of amino acids in a chain 
** @param [r] max_mismatch   [ajint]  Maximum number of permissible mismatches 
**                                    between the ATOM and SEQRES sequences 
** @param [r] max_trim       [ajint]  Max. no. residues to trim when checking
**                                    for missing N- or C-terminal ATOM or 
**                                    SEQRES sequences.
** @param [r] camask         [AjBool]  Whether to mask non-amino acid groups
** within protein chains which do not have a C-alpha atom.
** @param [r] camask1        [AjBool]  Whether to mask amino acid residues
** within protein chains which do not have a C-alpha atom.
** @param [r] atommask       [AjBool]  Whether to mask residues or groups 
** in protein chains with a single atom only.
** @param [u] flog           [AjPFile] Pointer to log file (build diagnostics)
**
** @return [AjPPdb] pdb object pointer, or NULL on failure.
** @@
****************************************************************************/

AjPPdb ajPdbReadRawNew(AjPFile inf, const AjPStr pdbid, ajint min_chain_size, 
		       ajint max_mismatch, ajint max_trim, AjBool camask, 
		       AjBool camask1, AjBool atommask, AjPFile flog)
{
    AjPPdbfile pdbfile      =NULL; /* Pdbfile structure (for raw data)    */
    AjPPdb     ret          =NULL;  /* Pdb structure (for parsed data)     */
    AjPElements elms=NULL;         /* Elements structure (for parsed data)*/

    

    if(!inf || !flog)
    {
	ajWarn("Null arg passed to ajPdbReadRawNew");

	return NULL;
    }
    


    /* Write pdbfile structure */
    if(!(pdbfile=ReadLines(inf)))
	return NULL;
    
    /* Allocate Elements object */
    elms = ElementsNew(0); 
    
    
    ajStrAssignS(&(pdbfile->pdbid), pdbid);
    ajStrFmtLower(&(pdbfile->pdbid));
    
    
    /* Initial read of pdb file, read sequences for chains from 
       SEQRES records, mark lines up to ignore or as coordinate 
       lines, assigning initial residue numbers, read bibliographic
       information etc. */
    if(!FirstPass(pdbfile, flog, &elms, camask))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);

	return NULL;
    }

    /*DIAGNOSTIC
      diagnostic(&pdbfile, 0);        
      diagnostic(&pdbfile, 1);*/
    
    
    /* Check that SEQRES records contain protein chains. Check 
       that chain id's are unique */
    if(!CheckChains(pdbfile, flog, min_chain_size))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);

	return NULL;
    }


    /* Check for correct number of TER records. Mask unwanted TER
       records */
    if(!CheckTer(pdbfile, flog))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);

	return NULL;
    }


    /* Assign model and chain number to each coordinate line. Mark
       up non-protein coordinates */
    if(!NumberChains(pdbfile, flog))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);

	return NULL;
    }

    /* Mask any ATOM or HETATM records with chain id's of chains of
       non-proteins or chains that have non-unique id's (chainok==ajFalse).
       Check that ATOM records contain protein chains.
       */
    if(!MaskChains(pdbfile, flog, min_chain_size, camask, 
		   camask1, atommask))
    {
	ElementsDel(&elms);
	PdbfileDel(&pdbfile);

	return NULL;

    }

    /* DIAGNOSTIC      
    diagnostic(pdbfile, 0);        
    diagnostic(pdbfile, 1);      */
  
    
   
    /* Standardise residue numbering */
    if(!StandardiseNumbering(pdbfile, flog))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);

	return NULL;
    }


    /* Find correct residue numbering */
    if(!AlignNumbering(pdbfile, flog, max_mismatch, max_trim))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);
	return NULL;
    }

    if(!WriteElementData(pdbfile, flog, elms))
    {
	PdbfileDel(&pdbfile);
	ElementsDel(&elms);

	return NULL;	
    }

    ElementsDel(&elms);

    /* Copy data from Pdbfile object to Pdb object.
       PdbfileToPdb creates the Pdb object (ret) */
    if(!PdbfileToPdb(&ret, pdbfile))
    {
	PdbfileDel(&pdbfile);
	ajPdbDel(&ret);

	return NULL;	
    }


    /* Tidy up and return */
    PdbfileDel(&pdbfile);
    
    return ret;
}




/* ======================================================================= */
/* =========================== destructors =============================== */
/* ======================================================================= */




/* @section Structure Destructors *******************************************
**
** All destructor functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
****************************************************************************/




/* ======================================================================= */
/* ============================ Assignments ============================== */
/* ======================================================================= */




/* @section Assignments *****************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
****************************************************************************/




/* ======================================================================= */
/* ============================= Modifiers =============================== */
/* ======================================================================= */




/* @section Modifiers *******************************************************
**
** These functions use the contents of an instance and update them.
**
****************************************************************************/




/* ======================================================================= */
/* ========================== Operators ===================================*/
/* ======================================================================= */




/* @section Operators *******************************************************
**
** These functions use the contents of an instance but do not make any 
** changes.
**
****************************************************************************/




/* ======================================================================= */
/* ============================== Casts ===================================*/
/* ======================================================================= */




/* @section Casts ***********************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. They are provided for programming convenience
** but should be used with caution.
**
****************************************************************************/




/* ======================================================================= */
/* =========================== Reporters ==================================*/
/* ======================================================================= */




/* @section Reporters *******************************************************
**
** These functions return the contents of an instance but do not make any 
** changes.
**
****************************************************************************/




/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */




/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/




/* @func ajPdbWriteDomainRecordRaw *****************************************
**
** Writes lines to a PDB file.  What is written depends upon the mode:
** ajHEADER_DOMAIN  Header line for domain PDB file.
** ajSEQRES_DOMAIN  SEQRES records for domain.
** ajATOMPDB_DOMAIN ATOM records for domain using original residue numbers.
** ajATOMIDX_DOMAIN ATOM records for domain using residues numbers that give 
**                  correct index into SEQRES sequence.
**
** @param [r] mode [ajint]   Mode that controls what is printed: one of 
**                           ajHEADER_DOMAIN, ajSEQRES_DOMAIN, 
**                           ajATOMPDB_DOMAIN, ajATOMIDX_DOMAIN
**                           
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] mod  [ajint]   Model number
** @param [r] scop [const AjPScop] Scop object for domain
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool  ajPdbWriteDomainRecordRaw(ajint mode, const AjPPdb pdb, ajint mod,
				  const AjPScop scop,
				  AjPFile outf, AjPFile errf)
{
    /* Check args */
    if(!outf || !scop)
	ajFatal("Invalid args passed to ajPdbWriteDomainRecordRaw");
    

    if(mode==ajHEADER_DOMAIN)
    {
	if(!WriteHeaderScop(outf, scop))
	    return ajFalse;
    }
    else if(mode==ajSEQRES_DOMAIN)
    {
	if(!errf || !pdb)
	    ajFatal("Invalid args passed to ajPdbWriteDomainRecordRaw");

	if(!WriteSeqresDomain(errf, outf, pdb, scop))
	    return ajFalse;
    }
    else if(mode==ajATOMPDB_DOMAIN)
    {
	if(!errf || !pdb)
	    ajFatal("Invalid args passed to ajPdbWriteDomainRecordRaw");

	if(!WriteAtomDomainPdb(errf, outf, pdb, scop, mod))
	    return ajFalse;
    }
    else if(mode==ajATOMIDX_DOMAIN)
    {
	if(!errf || !pdb)
	    ajFatal("Invalid args passed to ajPdbWriteDomainRecordRaw");

	if(!WriteAtomDomainIdx(errf, outf, pdb, scop, mod))
	    return ajFalse;
    }
    else
	ajFatal("Invalid mode in ajPdbWriteDomainRecordRaw");

    return ajTrue;
}




/* @func ajPdbWriteRecordRaw ************************************************
**
** Writes lines in pdb format to a PDB file.  What is written depends upon 
** the mode:
** ajSEQRES_CHAIN     SEQRES records for a chain.
** ajATOMPDB_CHAIN    ATOM records for chain using original residue numbers.
** ajATOMIDX_CHAIN    ATOM records for domain using residues numbers that 
**                    give correct index into SEQRES sequence.
** ajHETEROGEN        ATOM line for a heterogen (small ligand).
** ajHEADER           Header line.
** ajTITLE            Title line.
** ajCOMPND           COMPND records (info. on compound)
** ajSOURCE           SOURCE records (info. on protein source)
** ajEMPTYREMARK      An empty REMARK record.
** ajRESOLUTION       Record with resolution of the structure.
**
** @param [r] mode  [ajint]  Mode that controls what is printed: one of 
**                           ajSEQRES_CHAIN, ajATOMPDB_CHAIN, ajATOMIDX_CHAIN, 
**                           ajHETEROGEN, ajHEADER, ajTITLE,ajCOMPND,ajSOURCE, 
**                           ajEMPTYREMARK,ajRESOLUTION.
** @param [r] pdb   [const AjPPdb]  Pdb object
** @param [r] mod   [ajint]   Model number.
** @param [r] chn   [ajint]   Chain number.
** @param [w] outf  [AjPFile] Output file stream
** @param [w] errf  [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool  ajPdbWriteRecordRaw(ajint mode, const AjPPdb pdb, ajint mod, 
			    ajint chn, AjPFile outf, AjPFile errf)
{
    /* Check args */
    if(!outf || !pdb)
	ajFatal("Invalid args passed to ajPdbWriteRecordRaw");
    

    if(mode==ajSEQRES_CHAIN)
    {
	if(!errf)
	    ajFatal("Invalid args passed to ajPdbWriteRecordRaw");

	if(!WriteSeqresChain(errf, outf, pdb, chn))
	    return ajFalse;
    }
    else if(mode==ajATOMPDB_CHAIN)
    {
	if(!WriteAtomChain(outf,  pdb,  mod,  chn, 
			   ajPDB))
	    return ajFalse;
    }
    else if(mode==ajATOMIDX_CHAIN)
    {
	if(!WriteAtomChain(outf, pdb, mod, chn, ajIDX))
	    return ajFalse;
    }
    else if(mode==ajHETEROGEN)
    {
	if(!WriteHeterogen(outf,  pdb,  mod))
	    return ajFalse;
    }
    else if(mode==ajHEADER)
    {
	if(!WriteHeader(outf, pdb))
	    return ajFalse;
    }
    else if(mode==ajTITLE)
    {
	if(!WriteTitle(outf, pdb))
	    return ajFalse;
    }
    else if(mode==ajCOMPND)
    {
	if(!WriteCompnd(outf,  pdb))
	    return ajFalse;
    }
    else if(mode==ajSOURCE)
    {
	if(!WriteSource(outf,  pdb))
	    return ajFalse;
    }
    else if(mode==ajEMPTYREMARK)
    {
	if(!WriteEmptyRemark(outf,  pdb))
	    return ajFalse;
    }
    else if(mode==ajRESOLUTION)
    {
	if(!WriteResolution(outf,  pdb))
	    return ajFalse;
    }
    else
	ajFatal("Invalid mode in ajPdbWriteRecordRaw");

    return ajTrue;
}




/* @func ajPdbWriteAllRaw **************************************************
**
** Writes a pdb file for a protein.
**
** @param [r] mode [ajint]   Either ajPdb or ajIDX if the original or 
**                           corrected residue number is to be used. 
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajPdbWriteAllRaw(ajint mode, const AjPPdb pdb,
			AjPFile outf, AjPFile errf)
{
    ajint x;
    ajint y;
    
    
    /* Write bibliographic info. */
    ajPdbWriteRecordRaw(ajHEADER, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajTITLE, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajCOMPND, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajSOURCE, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajEMPTYREMARK, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajRESOLUTION, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajEMPTYREMARK, pdb, 0, 0, outf, NULL);
    
    
    /* Write SEQRES records */
    for(x=0;x<pdb->Nchn;x++)
	if(!ajPdbWriteRecordRaw(ajSEQRES_CHAIN, pdb, 0, x+1, outf, errf))
	{
	    ajWarn("Error writing file in ajPdbWriteAllRaw"); 

	    return ajFalse;
	}
    
    
    /* Loop for each model */
    for(y=0;y<pdb->Nmod;y++)
    {
	/* Write the MODEL record */
	if(pdb->Method == ajNMR)
	    ajFmtPrintF(outf, "MODEL%9d%66s\n", y+1, " ");

	
	/* Write ATOM/HETATM records */
	for(x=0;x<pdb->Nchn;x++)
	{
	    if(mode == ajPDB)
	    {
		if(!ajPdbWriteRecordRaw(ajATOMPDB_CHAIN, pdb, y+1, x+1, 
					 outf, NULL))
		{
		    ajWarn("Error writing file in ajPdbWriteAllRaw"); 

		    return ajFalse;
		}
	    }
	    else if(mode == ajIDX)
	    {
		if(!ajPdbWriteRecordRaw(ajATOMIDX_CHAIN, pdb, y+1, x+1, 
					 outf, NULL))
		{
		    ajWarn("Error writing file in ajPdbWriteAllRaw"); 

		    return ajFalse;
		}
	    }
	    else
		ajFatal("Invalid mode in ajPdbWriteAllRaw");
	    
	
	    if(!ajPdbWriteRecordRaw(ajHETEROGEN, pdb, y+1, 0, outf, NULL))
	    {
		ajWarn("Error writing file in ajPdbWriteAllRaw"); 

		return ajFalse;
	    }
	

	    /* Write ENDMDL record */
	    if(pdb->Method == ajNMR)
		ajFmtPrintF(outf, "%-80s\n", "ENDMDL");
	}

    }	

    /* Write END record */
    ajFmtPrintF(outf, "%-80s\n", "END");
    
    return ajTrue;

}




/* @func ajPdbWriteDomainRaw ***********************************************
**
** Writes a pdb file for a SCOP domain. Where coordinates for multiple 
** models (e.g. NMR structures) are given, data for model 1 are written. 
** Coordinates are taken from a Pdb structure, domain definition is taken 
** from a Scop structure.
** In the pdb file, the coordinates are presented as belonging to a single 
** chain regardless of how many chains the domain comprised.
** Coordinates for heterogens are NOT written to file.
** 
** @param [r] mode [ajint]   Either ajPDB or ajIDX if the original or 
**                           corrected residue number is to be used. 
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] scop [const AjPScop] Scop object
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajPdbWriteDomainRaw(ajint mode, const AjPPdb pdb, const AjPScop scop, 
			    AjPFile outf, AjPFile errf)
{
    ajint z;     /* A counter */
    ajint chn;   /* No. of the chain in the pdb structure */


    if(!pdb || !scop || !outf || !errf)
      ajFatal("Bad args passed to ajPdbWriteDomainRaw");


    /* Check for errors in chain identifier and length */
    for(z=0;z<scop->N;z++)
	if(!ajPdbChnidToNum(scop->Chain[z], pdb, &chn))
	{
	    ajWarn("Chain incompatibility error in "
		   "ajPdbWriteDomainRaw");			
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility error "
			"in ajPdbWriteDomainRaw\n", scop->Entry);
		
	    return ajFalse;
	}
	else if(pdb->Chains[chn-1]->Nres==0)
	{		
	    ajWarn("Chain length zero");			
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n",
			scop->Entry);
	    
	    return ajFalse;
	}
    


    /* Write bibliographic info. */
    ajPdbWriteDomainRecordRaw(ajHEADER_DOMAIN, NULL, 0, scop, outf, NULL);
    ajPdbWriteRecordRaw(ajTITLE, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajCOMPND, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajSOURCE, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajEMPTYREMARK, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajRESOLUTION, pdb, 0, 0, outf, NULL);
    ajPdbWriteRecordRaw(ajEMPTYREMARK, pdb, 0, 0, outf, NULL);
    

    /* Write SEQRES records */
    if(!ajPdbWriteDomainRecordRaw(ajSEQRES_DOMAIN, pdb, 0, scop, outf, 
				   errf))
    {
	ajWarn("Error writing file in ajPdbWriteDomainRaw"); 

	return ajFalse;
    }


    /* Write MODEL record, if appropriate */
    if(pdb->Method == ajNMR)
	ajFmtPrintF(outf, "MODEL%9d%66s\n", 1, " ");


    /* Write ATOM/HETATM records */
    if(!WriteAtomDomain(errf, outf, pdb, scop, 1, mode))
    {
	ajWarn("Error writing file in ajPdbWriteDomainRaw"); 

	return ajFalse;
    }

    
    /* Write END/ENDMDL records */
    if(pdb->Method == ajNMR)
	ajFmtPrintF(outf, "%-80s\n", "ENDMDL");

    ajFmtPrintF(outf, "%-80s\n", "END");

    return ajTrue;
}

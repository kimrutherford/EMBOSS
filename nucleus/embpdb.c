/****************************************************************************
** @source embpdb.c
**
** Algorithms for handling protein structural data.
** For use with the Atom, Chain and Pdb objects defined in ajpdb.h
** Also for use with Hetent, Het, Vdwres, Vdwall, Cmap and Pdbtosp objects 
** (also in ajpdb.h).
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
****************************************************************************/

#include "emboss.h"
#include <math.h>

/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */






/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */






/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */





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

/* @func embPdbidToSp *******************************************************
**
** Read a pdb identifier code and writes the equivalent swissprot identifier
** code.  Relies on list of Pdbtosp objects sorted by PDB code, which is 
** usually obtained by a call to ajPdbtospReadAllNew.
** 
** @param [r] pdb  [const AjPStr]   Pdb  identifier code
** @param [w] spr  [AjPStr*]  Swissprot identifier code
** @param [r] list [const AjPList]  Sorted list of Pdbtosp objects
**
** @return [AjBool]  True if a swissprot identifier code was found
**                   for the Pdb code.
** @@
****************************************************************************/

AjBool embPdbidToSp(const AjPStr pdb, AjPStr *spr, const AjPList list)
{
    AjPPdbtosp *arr = NULL;  /* Array derived from list */
    ajint dim = 0;           /* Size of array */
    ajint idx = 0;           /* Index into array for the Pdb code */

    
    if(!pdb || !list)
    {
	ajWarn("Bad args passed to embPdbidToSp");
	return ajFalse;
    }
    

    dim = ajListToarray(list, (void ***) &(arr));
    if(!dim)
    {
	ajWarn("Empty list passed to embPdbidToSp");
	return ajFalse;
    }


    if( (idx = ajPdbtospArrFindPdbid(arr, dim, pdb))==-1)
	return ajFalse;
    else
    {
	ajStrAssignS(spr, arr[idx]->Spr[0]);
	return ajTrue;
    }
}





/* @func embPdbidToAcc ******************************************************
**
** Read a pdb identifier code and writes the equivalent accession number.
** Relies on list of Pdbtosp objects sorted by PDB code, which is usually 
** obtained by a call to ajPdbtospReadAllNew.
** 
** @param [r] pdb  [const AjPStr]   Pdb  identifier code
** @param [w] acc  [AjPStr*]  Accession number
** @param [r] list [const AjPList]  Sorted list of Pdbtosp objects
**
** @return [AjBool]  True if a swissprot identifier code was found for the
**                   Pdb code.
** @@
****************************************************************************/

AjBool embPdbidToAcc(const AjPStr pdb, AjPStr *acc, const AjPList list)
{
    AjPPdbtosp *arr = NULL;  /* Array derived from list */
    AjPPdbtosp *arrfree = NULL;
    ajint dim = 0;           /* Size of array */
    ajint idx = 0;           /* Index into array for the Pdb code */

    
    if(!pdb || !list)
    {
	ajWarn("Bad args passed to embPdbidToAcc");
	return ajFalse;
    }
    

    dim = ajListToarray(list, (void ***) &(arr));
    if(!dim)
    {
	ajWarn("Empty list passed to embPdbidToAcc");
	return ajFalse;
    }


    if( (idx = ajPdbtospArrFindPdbid(arr, dim, pdb))==-1)
    {
        arrfree = (AjPPdbtosp*) arr;
	AJFREE(arrfree);
	return ajFalse;
    }
    
    else
    {
	ajStrAssignS(acc, arr[idx]->Acc[0]);
        arrfree = (AjPPdbtosp*) arr;
	AJFREE(arrfree);
	return ajTrue;
    }
}





/* @func  embPdbidToScop ****************************************************
**
** Writes a list of scop identifier codes for the domains that a Pdb object
** contains.  The domain data is taken from a list of scop objects.
**
** @param [r] pdb             [const AjPPdb]   Pointer to pdb object
** @param [r] list_allscop    [const AjPList]  Pointer to SCOP list of SCOP 
**                                       classification objects  
** @param [w] list_pdbscopids [AjPList*] Pointer to list of scop domain ids
**                                       in the current pdb object
** 
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

AjBool embPdbidToScop(const AjPPdb pdb, const AjPList list_allscop,
		      AjPList *list_pdbscopids)
{
  
    AjIList iter    = NULL; /* List iterator for SCOP classification list */
    AjPScop ptr     = NULL;
    AjPStr tmpPdbId = NULL;
    AjPStr tmpDomId = NULL;
    ajint found     = 0;

    iter=ajListIterNewread(list_allscop);


    while((ptr=(AjPScop)ajListIterGet(iter)))
    {
	ajStrAssignS(&tmpPdbId, ptr->Pdb);
	ajStrFmtLower(&tmpPdbId);

	if(ajStrMatchS(pdb->Pdb, tmpPdbId))
	{
	    ajStrAssignS(&tmpDomId, ptr->Entry);
	    ajStrFmtLower(&tmpDomId);
	    ajListPushAppend(*list_pdbscopids, tmpDomId);
	    tmpDomId = NULL;
	    found = 1;
	}
    }
    ajListIterDel(&iter);
    ajStrDel(&tmpPdbId);
    ajStrDel(&tmpDomId);
  
    if(found==1) 
	return ajTrue;

    return ajFalse;
}





/* @func embAtomInContact ***************************************************
**
** Determines whether two atoms are in physical contact  
**
** @param [r] atm1   [const AjPAtom]     Atom 1 object
** @param [r] atm2   [const AjPAtom]     Atom 1 object
** @param [r] thresh [float]       Threshold contact distance
** @param [r] vdw    [const AjPVdwall]   Vdwall object
**
** Contact between two atoms is defined as when the van der Waals surface of 
** the first atom comes within the threshold contact distance (thresh) of 
** the van der Waals surface of the second atom.
**
** @return [AjBool] True if the two atoms form contact
** @@
**
****************************************************************************/

AjBool embAtomInContact(const AjPAtom atm1, const AjPAtom atm2, float thresh,
			const AjPVdwall vdw)
{
    float val  = 0.0;
    float val1 = 0.0;



    /* Check args */
    if(!atm1 || !atm2 || !vdw)
    {
	ajWarn("Bad args passed to embAtomInContact");
	return ajFalse;
    }
    
    
    val=((atm1->X -  atm2->X) * (atm1->X -  atm2->X)) +
	((atm1->Y -  atm2->Y) * (atm1->Y -  atm2->Y)) +
	    ((atm1->Z -  atm2->Z) * (atm1->Z -  atm2->Z));


    /*  This calculation uses square root 
    if((sqrt(val) - embVdwRad(atm1, vdw) -
	embVdwRad(atm2, vdw)) <= thresh)
	return ajTrue;
	*/


    /* Same calculation avoiding square root */
    val1 = embVdwRad(atm1, vdw) + embVdwRad(atm2, vdw) + thresh;
    
    if(val <= (val1*val1))
	return ajTrue;


    return ajFalse;
} 




/* @func embAtomDistance ****************************************************
**
** Returns the distance (Angstroms) between two atoms.
**
** @param [r] atm1   [const AjPAtom]     Atom 1 object
** @param [r] atm2   [const AjPAtom]     Atom 1 object
** @param [r] vdw    [const AjPVdwall]   Vdwall object
**
** Returns the distance (Angstroms) between the van der Waals surface of two
** atoms.
**
** @return [float] Distance (Angstroms) between two atoms.
** @@
**
****************************************************************************/

float embAtomDistance(const AjPAtom atm1, const AjPAtom atm2,
		      const AjPVdwall vdw)
{
    float val  = 0.0;
    float val1 = 0.0;

    
    val=((atm1->X -  atm2->X) * (atm1->X -  atm2->X)) +
	((atm1->Y -  atm2->Y) * (atm1->Y -  atm2->Y)) +
	    ((atm1->Z -  atm2->Z) * (atm1->Z -  atm2->Z));


    /*  This calculation uses square root */
    val1= (float) (sqrt(val) - embVdwRad(atm1, vdw) - embVdwRad(atm2, vdw));
        
    return val1;
} 




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

/* @func embVdwRad **********************************************************
**
** Returns the van der Waals radius of an atom. Returns 1.2 as default.
**
** @param [r] atm    [const AjPAtom]     Atom object
** @param [r] vdw    [const AjPVdwall]   Vdwall object
**
** @return [float] van der Waals radius of the atom
** @@
**
****************************************************************************/

float embVdwRad(const AjPAtom atm, const AjPVdwall vdw)
{
    ajint x = 0;
    ajint y = 0;
    
    for(x=0;x<vdw->N;x++)
	for(y=0;y<vdw->Res[x]->N;y++)
	    if(ajStrMatchS(atm->Atm, vdw->Res[x]->Atm[y]))
		return(vdw->Res[x]->Rad[y]);	 
    
    return((float)1.2);
}





/* @func embPdbToIdx ********************************************************
**
** Reads a Pdb object and writes an integer which gives the index into the 
** protein sequence for a residue with a specified pdb residue number and a 
** specified chain number.
** 
** @param [w] idx [ajint*]  Residue number (index into sequence)
** @param [r] pdb [const AjPPdb]  Pdb object
** @param [r] res [const AjPStr]  Residue number (PDB numbering)
** @param [r] chn [ajint]   Chain number
**
** @return [AjBool] True on succcess (res was found in pdb object)
** @@
****************************************************************************/
AjBool embPdbToIdx(ajint *idx, const AjPPdb pdb, const AjPStr res, ajint chn)
{
    AjIList  iter = NULL;
    AjPResidue  residue  = NULL;
    
    
    if(!pdb || !(res) || !(idx))
    {
	ajWarn("Bad arg's passed to embPdbToIdx");
	return ajFalse;
    }
    
    if((chn > pdb->Nchn) || (!pdb->Chains) || (chn<1))
    {
	ajWarn("Bad arg's passed to embPdbToIdx");
	return ajFalse;
    }
    

    /* Initialise the iterator */
    iter=ajListIterNewread(pdb->Chains[chn-1]->Residues);


    /* Iterate through the list of residues */
    while((residue = (AjPResidue)ajListIterGet(iter)))
    {
	if(residue->Chn!=chn)
	    continue;
	
	/*
	** Hard-coded to work on model 1
	** Continue / break if a non-protein residue is found or model no. !=1
	*/
	if(residue->Mod!=1)
	    break;

	/* if(residue->Type!='P') 
	    continue; */

	/* If we have found the residue */
	if(ajStrMatchS(res, residue->Pdb))
	{
	    ajListIterDel(&iter);		
	    *idx = residue->Idx;
	    return ajTrue;
	}
    }
        
    ajWarn("Residue number not found in embPdbToIdx");
    ajListIterDel(&iter);		

    return ajFalse;
}





/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/





/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */

/* @section Miscellaneous ***************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories. 
**
****************************************************************************/

/* @func embPdbListHeterogens ***********************************************
** 
** Function to create a list of arrays of Atom objects for ligands in the 
** current Pdb object (a single array for each ligand).  An array of int's
** giving the number of Atom objects in each array, is also written. The
** number of ligands is also written.
** 
** @param [r] pdb             [const AjPPdb]   Pointer to pdb object 
** @param [w] list_heterogens [AjPList*] Pointer to list of heterogen Atom
**                                       arrays 
** @param [w] siz_heterogens  [AjPInt*]  Pointer to integer array of sizes
**                                       (number of Atom objects in each 
**                                       array).
** @param [w] nhet            [ajint*]   Number of arrays in the list that
**                                       was written. 
** @param [u] logfile         [AjPFile]  Log file for error messages
**
** @return [AjBool] ajTrue on success
** @@ 
****************************************************************************/

AjBool embPdbListHeterogens(const AjPPdb pdb, AjPList *list_heterogens, 
			    AjPInt *siz_heterogens, ajint *nhet, 
			    AjPFile logfile)
{ 
  /*
  ** NOTE: EVERYTHING IN THE CLEAN PDB FILES IS CURRENTLY CHAIN
  ** ASSOCIATED!  THIS WILL BE CHANGED IN FUTURE
  */
  AjIList iter  = NULL;	/* Iterator for atoms in current pdb object */
  AjPAtom hetat = NULL;		/* Pointer to current Atom object */
  ajint i=0;			/* Counter for chains */
  ajint prev_gpn = -10000; 	/* Group number of atom object from
				   previous iteration */
  AjPList GrpAtmList = NULL; 	/* List to hold atoms from the current 
				   group */
  AjPAtom *AtmArray  = NULL;	/* Array of atom objects */
  ajint n=0;			/* number of elements in AtmArray */
  ajint grp_count = 0;		/* No. of groups */
  ajint arr_count = 0;          /* Index for siz_heterogens */
    
  /* Check args */
  if((pdb==NULL)||(list_heterogens==NULL)||(siz_heterogens==NULL))
  {
      ajWarn("Bad args passed to embPdbListHeterogens\n");
      return ajFalse;
  }
  
  if((!(*list_heterogens))||(!(*siz_heterogens)))
  {
      ajWarn("Bad args passed to embPdbListHeterogens\n");
      return ajFalse;
  }
  
  if(pdb->Ngp>0)
      ajFmtPrintF(logfile, "\tNGP:%d\n", pdb->Ngp);
  
  if(pdb->Nchn>0)
  {      
      for(i=0;i<pdb->Nchn;++i) 
      {
	  prev_gpn=-100000;	   /* Reset prev_gpn for each chain */
	  /* initialise iterator for pdb->Chains[i]->Atoms */
	  iter=ajListIterNewread(pdb->Chains[i]->Atoms);
	  /* Iterate through list of Atom objects */
	  while((hetat=(AjPAtom)ajListIterGet(iter)))
	  {		
	      /* check for type  */
	      if(hetat->Type != 'H')
		  continue;

	      /* TEST FOR A NEW GROUP */
	      if(prev_gpn != hetat->Gpn) 
	      {
		  grp_count++;
		  if(GrpAtmList)
		  {
		      n=(ajListToarray(GrpAtmList, (void ***) &AtmArray));
		      ajListPushAppend(*list_heterogens, AtmArray);
		      /* So that ajListToArray doesn't try and free the non-NULL pointer */
		      AtmArray=NULL;
		      ajIntPut(siz_heterogens, arr_count, n);
		      (*nhet)++;
		      ajListFree(&GrpAtmList);
		      GrpAtmList=NULL;
		      arr_count++;
		  }		    		    
		  GrpAtmList=ajListNew();
		  prev_gpn=hetat->Gpn;
	      } /* End of new group loop */
	      ajListPushAppend(GrpAtmList, (AjPAtom) hetat);
	  } /* End of list iteration loop */

	  /* Free list iterator */
	  ajListIterDel(&iter);
	    
      } /* End of chain for loop */

      if(GrpAtmList)
      {
	  n=(ajListToarray(GrpAtmList, (void ***) &AtmArray));
	  ajListPushAppend(*list_heterogens, AtmArray);
	  /* So that ajListToArray doesn't try and free the non-NULL pointer */
	  AtmArray=NULL; 
	  ajIntPut(siz_heterogens, arr_count, n);
	  (*nhet)++;
	  ajListFree(&GrpAtmList);
	  GrpAtmList=NULL;
      }
	
      GrpAtmList = NULL;
      prev_gpn   = -10000;  

  } /* End of chain loop */
  
  
  return ajTrue;
}





/* @func embPdbResidueIndexI ***************************************************
**
** Reads a Pdb object and writes an integer array which gives the index into 
** the protein sequence for structured residues (residues for which electron
** density was determined) for a given chain. The array length is of course
** equal to the number of structured residues. 
**
** @param [r] pdb [const AjPPdb] Pdb object
** @param [r] chn [ajint] Chain number
** @param [w] idx [AjPInt*] Index array
**
** @return [AjBool] True on succcess
** @@
****************************************************************************/

AjBool embPdbResidueIndexI(const AjPPdb pdb, ajint chn, AjPInt *idx)
{
    AjIList  iter = NULL;
    AjPResidue  res  = NULL;
/*    ajint this_rn = 0;
    ajint last_rn = -1000; */
    ajint resn    = 0;  
    
    
    if(!pdb || !(*idx))
    {
	ajWarn("Bad arg's passed to embPdbResidueIndexI");
	return ajFalse;
    }
    
    if((chn > pdb->Nchn) || (!pdb->Chains))
    {
	ajWarn("Bad arg's passed to embPdbResidueIndexI");
	return ajFalse;
    }
    

    /* Initialise the iterator */
    iter=ajListIterNewread(pdb->Chains[chn-1]->Residues);


    /* Iterate through the list of residues */
    while((res=(AjPResidue)ajListIterGet(iter)))
    {
	if(res->Chn!=chn)
	    continue;
	
	/* Hard-coded to work on model 1 */
	/* Continue / break if a non-protein residue is found or 
	   model no. !=1 */
	if(res->Mod!=1)
	    break;
	/*
	   if(res->Type!='P') 
	   continue; */


	/* If we are onto a new residue */
	/*
	this_rn=res->Idx;
	if(this_rn!=last_rn)
	{
	    ajIntPut(&(*idx), resn++, res->Idx);
	    last_rn=this_rn;
	}*/

	ajIntPut(&(*idx), resn++, res->Idx);

    }
        
    if(resn==0)
    {
	ajWarn("Chain not found in embPdbResidueIndexI");
	ajListIterDel(&iter);		
	return ajFalse;
    }
    	
    ajListIterDel(&iter);		

    return ajTrue;
}




/* @func embPdbResidueIndexC ***************************************************
**
** Reads a Pdb object and writes an integer array which gives the index into 
** the protein sequence for structured residues (residues for which electron
** density was determined) for a given chain.  The array length is of course 
** equal to the number of structured residues. 
**
** @param [r] pdb [const AjPPdb]  Pdb object
** @param [r] chn [char]  Chain identifier
** @param [w] idx [AjPInt*] Index array
**
** @return [AjBool] True on succcess
** @@
****************************************************************************/

AjBool embPdbResidueIndexC(const AjPPdb pdb, char chn, AjPInt *idx)
{
    ajint chnn;
    
    if(!ajPdbChnidToNum(chn, pdb, &chnn))
    {
	ajWarn("Chain not found in embPdbResidueIndexC");
	return ajFalse;
    }
    
    if(!embPdbResidueIndexI(pdb, chnn, idx))
	return ajFalse;

    return ajTrue;
}





/* @func embPdbResidueIndexICA ************************************************
**
** Reads a Pdb object and writes an integer array which gives the index into 
** the protein sequence for structured residues (residues for which electron
** density was determined) for a given chain, EXCLUDING those residues for 
** which CA atoms are missing. The array length is of course equal to the 
** number of structured residues. 
**
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] chn  [ajint]   Chain number
** @param [w] idx  [AjPUint*] Index array
** @param [w] nres [ajint*]  Array length 
**
** @return [AjBool] True on succcess
** @@
****************************************************************************/

AjBool embPdbResidueIndexICA(const AjPPdb pdb,
			  ajint chn, AjPUint *idx, ajint *nres)
{
    AjIList iter  = NULL;
    AjPAtom atm   = NULL;
    ajint this_rn = 0;
    ajint last_rn = -1000;
    ajint resn    = 0;     /* Sequential count of residues */
    
    if(!pdb || !(*idx))
    {
	ajWarn("Bad arg's passed to embPdbResidueIndexICA");
	return ajFalse;
    }
    
    if((chn > pdb->Nchn) || (!pdb->Chains))
    {
	ajWarn("Bad arg's passed to embPdbResidueIndexICA");
	return ajFalse;
    }
    

    /* Initialise the iterator */
    iter=ajListIterNewread(pdb->Chains[chn-1]->Atoms);


    /* Iterate through the list of atoms */
    while((atm=(AjPAtom)ajListIterGet(iter)))
    {
	if(atm->Chn!=chn)
	    continue;
	
	/*
	** Hard-coded to work on model 1
	** Continue / break if a non-protein atom is found or model no. !=1
	*/
	if(atm->Mod!=1)
	    break;
	if(atm->Type!='P') 
	    continue;

	/* If we are onto a new residue */
	this_rn=atm->Idx;
	if(this_rn!=last_rn && ajStrMatchC(atm->Atm,  "CA"))
	{
	    ajUintPut(&(*idx), resn++, atm->Idx);
	    last_rn=this_rn;
	}
    }

        
    if(resn==0)
    {
	ajWarn("Chain not found in embPdbResidueIndexICA");
	ajListIterDel(&iter);		
	return ajFalse;
    }
    	
    *nres=resn;
    
    ajListIterDel(&iter);		

    return ajTrue;
}





/* @func embPdbResidueIndexCCA *************************************************
**
** Reads a Pdb object and writes an integer array which gives the index into 
** the protein sequence for structured residues (residues for which electron
** density was determined) for a given chain, EXCLUDING those residues for 
** which CA atoms are missing. The array length is of course equal to the 
** number of structured residues. 
**
** @param [r] pdb [const AjPPdb]  Pdb object
** @param [r] chn [char]    Chain identifier
** @param [w] idx [AjPUint*] Index array
** @param [w] nres [ajint*] Array length 
**
** @return [AjBool] True on succcess
** @@
****************************************************************************/

AjBool embPdbResidueIndexCCA(const AjPPdb pdb, char chn,
			     AjPUint *idx, ajint *nres)
{
    ajint chnn;
    
    if(!ajPdbChnidToNum(chn, pdb, &chnn))
    {
	ajWarn("Chain not found in embPdbResidueIndexCCA");
	return ajFalse;
    }
    
    if(!embPdbResidueIndexICA(pdb, chnn, idx, nres))
	return ajFalse;


    return ajTrue;
}



/* @func embStrideToThree ***************************************************
**
** Reads a string that contains an 8-state STRIDE secondary structure 
** assignment and writes a string with the corresponding 3-state assignment.
** The 8 states used in STRIDE are 'H' (alpha helix), 'G' (3-10 helix), 
** 'I' (Pi-helix), 'E' (extended conformation), 'B' or 'b' (isolated bridge),
** 'T' (turn) or 'C' (coil, i.e. none of the above).  The 3 states used
** are 'H' (STRIDE 'H', 'G' or 'I'), 'E' (STRIDE 'E', 'B' or 'b') and 'C'
** (STRIDE 'T' or 'C'). The string is allocated if necessary.
**
** @param [w] to [AjPStr*]  String to write
** @param [r] from [const AjPStr]  String to read
**
** @return [AjBool] True on succcess
** @@
****************************************************************************/
AjBool       embStrideToThree(AjPStr *to, const AjPStr from)
{
    if(!from)
    {
	ajWarn("Bad args passed to embStrideToThree");
	return ajFalse;
    }
    else
	ajStrAssignS(to, from);

    ajStrExchangeKK(to, 'G', 'H');
    ajStrExchangeKK(to, 'I', 'H');
    ajStrExchangeKK(to, 'B', 'E');
    ajStrExchangeKK(to, 'b', 'E');
    ajStrExchangeKK(to, 'T', 'C');

    return ajTrue;
}





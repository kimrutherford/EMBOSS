/* @source psiphi application
**
** Calculates the psi and phi torsion angles around the alpha carbons (CA)
**  in (a specified stretch of) a specified chain of a protein structure
**  from co-ordinates of the mainchain atoms in the two planes around it
**
** Outputs 360 degrees if an angle cannot be calculated
**
** @author Copyright (C) Damian Counsell
** @@
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
******************************************************************************/

#include "emboss.h"
#include <math.h>



static ajint psiphi_chain_index(ajint selected,
			 ajint highest,
			 ajint lowest);

static ajint psiphi_first_residue_number(const AjPPdb pdb,
				  ajint myindex,
				  ajint startres);

static ajint psiphi_last_residue_number(const AjPPdb pdb,
				 ajint myindex,
				 ajint startres,
				 ajint finishres);

static AjBool psiphi_phi_calculable(const AjBool* known);

static AjBool psiphi_psi_calculable(const AjBool* known);

static float psiphi_phival(AjPAtom const *atoms);

static float psiphi_psival(AjPAtom const *atoms);

static AjBool psiphi_load_previous_residue(AjPAtom current,
				    AjPAtom* atoms,
				    AjBool* known);

static AjBool psiphi_load_next_residue(AjPAtom current,
				AjPAtom* atoms,
				AjBool* known);

static AjBool psiphi_load_current_residue(AjPAtom current,
				   AjPAtom* atoms,
				   AjBool* known);

static AjPFeature psiphi_write_psi_phi(AjPFeattable angletab,
				ajint resnum,
				float phi,
				float psi);

static void psiphi_shift_residues(AjPAtom* atoms,
			   AjBool* known);


/* constant window size and enumerated indexes to atoms in window */
const ajint windowsize = 9;
enum enumAtomWindowPoint
{
    ENPrev,
    ECAlphaPrev,
    ECPrimePrev,
    ENCurr,
    ECAlphaCurr,
    ECPrimeCurr,
    ENNext,
    ECAlphaNext,
    ECPrimeNext
};

/* for unavailable angles (360 deg is an impossible torsion angle) */
const float FUnavailableAngle = 360.0;

/* @prog psiphi **************************************************************
** 
** protein structure C-alpha psi and phi angles given neighbour co-ordinates
**
******************************************************************************/

int main( int argc , char **argv )
{
    float phi = 0.0;
    float psi = 0.0;

    /*
     * coordinates from atoms in at least
     * THREE residues are required
     * to calculate psi and phi angles
     */
    ajint firstres;
    ajint secondres;
    ajint thirdres;

    /* declare position counters and limits */
    /* ...for residues */
    ajint cpos          = 0;
    ajint resnum          = 0;
    ajint prevres  = 0;
    ajint lastres     = 0;

    /* ...for chains */
    ajint myindex             = 0; /* ...into structure object */
    ajint lowest      = 0; /* ...in structure file     */
    ajint highest     = 0; /* ...in structure file     */

    /* variables for (user-specified) chain and residue numbers */
    ajint selected;
    ajint startres;
    ajint finishres;
    
    /* window of AjPAtoms for co-ords */
    AjPAtom* atoms = NULL;
    /* window of AjBools for presence or absence  */
    AjBool* known   = NULL;

    AjPStr header = NULL;
    /* DDDDEBUG: string for report footer */
    /*     AjPStr ajpStrReportTail      = NULL; */

    /* cleaned-up structure */
    AjPFile pdbfile     = NULL; /* file           */
    AjPPdb  pdb         = NULL; /* object         */
    AjPSeq  seq         = NULL; /* current chain  */
    AjIList atomlist    = NULL; /* list of atoms  */
    AjPAtom inlist      = NULL; /* current atom   */

    /* output report file for torsion angles */
    AjPReport report    = NULL;
    AjPFeattable angles = NULL;

    embInit( "psiphi", argc ,argv );

    /* get protein structure from ACD */
    pdbfile = ajAcdGetInfile("infile" );
    /* get angle output file from ACD */
    report = ajAcdGetReport("outfile");
    /* get chain to be scanned from ACD */
    selected = ajAcdGetInt("chainnumber");
    /* get first residue to be scanned from ACD */
    startres = ajAcdGetInt("startresiduenumber");
    /* get last residue to be scanned from ACD */
    finishres = ajAcdGetInt("finishresiduenumber");

    /* reserve memory for and read in structure */
    /* JISON */    pdb = ajPdbReadNew(pdbfile,0);
    
    /* check and set number of chain to be analysed */
    highest = pdb->Nchn;
    myindex = psiphi_chain_index(selected,
			       highest,
			       lowest);

    /* check and set range of residues to be analysed */
    firstres = 
	psiphi_first_residue_number(pdb,
				    myindex,
				    startres);    
    lastres = psiphi_last_residue_number(pdb,
					 myindex,
					 startres,
					 finishres);
    secondres = firstres+1;
    thirdres = firstres+2;

    /*
     * start loop over atoms in chain at
     * firstres and
     * finish at lastres
     */

    /* obtain iterator for list of atoms in chain */
    atomlist = 
	ajListIterNewread(pdb->Chains[myindex]->Atoms);

    ajDebug("psiphi pdb Pdb '%S' chain %d Id '%c'\n",
	    pdb->Pdb, myindex, pdb->Chains[myindex]->Id);

    /* obtain sequence from residues in chain */
    seq =
	ajSeqNewNameS(pdb->Chains[myindex]->Seq, pdb->Pdb);

    resnum = 0;

    /* create feature table for torsion angle output */
    angles = ajFeattableNewSeq(seq);    

    /* chain info for head of report */
    ajFmtPrintS(&header, "Chain: %d", (myindex+1));
    ajReportSetHeaderS(report, header);

    /* BEGIN ANALYSIS OF CHAIN HERE */
    /* loop through list until first residue in window reached */
    do
    {
	/* do nothing until you reach the start residue */
	inlist = ajListIterGet(atomlist);
	resnum = inlist->Idx;
    }
    while(resnum < firstres);

    prevres = resnum;

    /* create and initialize AjPAtom window array */
    atoms =
	(AjPAtom *) AJALLOC(windowsize*sizeof(AjPAtom));
    atoms[ENPrev]    = NULL;
    atoms[ECAlphaPrev] = NULL;
    atoms[ECPrimePrev] = NULL;
    atoms[ENCurr]     = NULL;
    atoms[ECAlphaCurr]  = NULL;
    atoms[ECPrimeCurr]  = NULL;
    atoms[ENNext]        = NULL;
    atoms[ECAlphaNext]     = NULL;
    atoms[ECPrimeNext]     = NULL;
    /* create and initialize AjBool window array */
    known =
	(AjBool *) AJALLOC(windowsize*sizeof(AjBool));
    known[ENPrev]    = AJFALSE;
    known[ECAlphaPrev] = AJFALSE;
    known[ECPrimePrev] = AJFALSE;
    known[ENCurr]     = AJFALSE;
    known[ECAlphaCurr]  = AJFALSE;
    known[ECPrimeCurr]  = AJFALSE;
    known[ENNext]        = AJFALSE;
    known[ECAlphaNext]     = AJFALSE;
    known[ECPrimeNext]     = AJFALSE;
    
    /* loop through list until window is full */
    do
    {
	resnum = inlist->Idx;

	/* load window with atom co-ordinates */
	/* get previous N, CA and C' */
	if(resnum == firstres)
	    psiphi_load_previous_residue(inlist,
					 atoms,
					 known);
	/* get current N, CA and C' */
	else if(resnum == secondres)
	    psiphi_load_current_residue(inlist,
					atoms,
					known);
	/* get new next N, CA and C'  */
	else if(resnum == thirdres)
	{
	    /* cpos residue no. for which angles calc'd */
	    cpos = resnum-1;
	    psiphi_load_next_residue(inlist,
				     atoms,
				     known);
	}
	else
	    break;
    }while((inlist = ajListIterGet(atomlist)));

    /* analyse first residue */
    if(psiphi_phi_calculable(known))
	phi = psiphi_phival(atoms);
    else
	phi = FUnavailableAngle;
    if(psiphi_psi_calculable(known))
	psi = psiphi_psival(atoms);
    else
	psi = FUnavailableAngle;
    psiphi_write_psi_phi(angles,cpos,phi,psi);

    /* loop through list until last residue to be analysed */
    prevres = resnum;
    psiphi_shift_residues(atoms, known);
    do
    {
	resnum = inlist->Idx;
	/* cpos residue no. for which angles calc'd */

	/* new residue? */
	if(resnum > prevres)
	{
	    /* analyse previous previous residue */
	    if(psiphi_phi_calculable(known))
		phi = psiphi_phival(atoms);
	    else
		phi = FUnavailableAngle;
	    if(psiphi_psi_calculable(known))
		psi = psiphi_psival(atoms);
	    else
		psi = FUnavailableAngle;
	    psiphi_write_psi_phi(angles,cpos,phi,psi);

	    psiphi_shift_residues(atoms, known);
	}
	/* not finished? get new next N, CA and C'  */
	if(resnum <= lastres)
	{
	    /* conditional is kludge for bad residue numbers at termini */
	    if( resnum > 1 )
		cpos = resnum-1;
	    psiphi_load_next_residue(inlist,
				     atoms,
				     known);
	}
	else
	    break;
	prevres = resnum;
    }
    while((inlist = ajListIterGet(atomlist)));

    /* conditional is kludge for bad residue numbers at chain termini */
    if( resnum > 1 )
	cpos = resnum-1;

    /* analyse penultimate residue */
    if(cpos < lastres)
    {

	if(psiphi_phi_calculable(known))
	    phi = psiphi_phival(atoms);
	else
	    phi = FUnavailableAngle;
	if(psiphi_psi_calculable(known))
	    psi = psiphi_psival(atoms);
	else
	    psi = FUnavailableAngle;

	psiphi_write_psi_phi(angles,
			     cpos,
			     phi,
			     psi);
	cpos++;
	psiphi_shift_residues(atoms,
			      known);
    }
    /* analyse last residue */
    if((cpos < lastres ) &&
       (psiphi_phi_calculable(known)))
    {
	phi = psiphi_phival(atoms);
	psi = FUnavailableAngle;
	psiphi_write_psi_phi(angles,
			     cpos,
			     phi,
			     psi);
    }
    /* END ANALYSIS OF CHAIN HERE */

    /* DDDDEBUG TEST INFO FOR TAIL OF REPORT */
    /*     ajFmtPrintS(&ajpStrReportTail, "This is some tail text"); */
    /*     ajReportSetTailS(report, ajpStrReportTail); */

    /* write the report to the output file */
    ajReportWrite(report,
		  angles,
		  seq);

    AJFREE(atoms);
    AJFREE(known);

    ajFeattableDel(&angles);
    ajListIterDel(&atomlist);
    ajFileClose(&pdbfile);
    ajReportDel(&report);
    ajPdbDel(&pdb);
    ajSeqDel(&seq);

    ajStrDel(&header);

    embExit();

    return 0;
}




/* @funcstatic psiphi_chain_index ********************************************
**
** check selected protein chain number present in structure file; return index
**
** @param [r] selected [ajint] number of chain selected by user
** @param [r] highest [ajint] number of highest chain in
**                                            structure
** @param [r] lowest [ajint] number of lowest chain in
**                                           structure
** @return [ajint] index
** @@
******************************************************************************/
static ajint psiphi_chain_index(ajint selected,
				ajint highest,
				ajint lowest)
{
    /* ERROR: chain number too high */ 
    if(selected > highest)
	ajDie("There is no chain %d---highest chain number: %d.",
	       selected, highest );
    /* ERROR: chain number too low */ 
    if(selected < lowest )
	ajWarn("There is no chain %d---lowest chain number %d.",
	       selected, lowest );
    return selected-1;
}




/* @funcstatic psiphi_first_residue_number ***********************************
**
** check selected lower residue within chain's range and return 1st window res
**
** @param [r] pdb [const AjPPdb] cleaned AjPPdb structure
** @param [r] myindex [ajint] number of user-selected chain in
**                                    structure
** @param [r] startres [ajint] user-selected lower residue
**                                            number
** @return [ajint] First window residue number
** @@
******************************************************************************/
static ajint psiphi_first_residue_number (const AjPPdb pdb,
				   ajint myindex,
				   ajint startres)
{
    ajint firstres  = 0;
    ajint lowestres = 0;

    AjPAtom inlist   = NULL;

    if(!ajListGetLength(pdb->Chains[myindex]->Atoms))
	ajFatal("Chain %d has no atoms",myindex+1);
    
    /* read first atom in list into memory, but keep it on list */
    ajListPeek(pdb->Chains[myindex]->Atoms,
	       (void**)&inlist);

    /* get number of lowest residue available in chain */
    lowestres = inlist->Idx;
    
    /* ERROR: start residue too low */ 
    if(startres < lowestres)
    {	
	ajWarn("No residue %d---number of lowest residue in chain %d is %d.",
	       startres, myindex,
	       lowestres );
    }

    /* use user-specified starting position... */
    if(startres > lowestres)
	firstres = startres-1;
    /* ...or use start of chain */
    else
    {
	firstres = lowestres-1;
	
    }

    return firstres;
}




/* @funcstatic psiphi_last_residue_number *************************************
**
** check selected upper protein residue within chain's range and return limit
**
** @param [r] pdb [const AjPPdb] cleaned AjPPdb structure
** @param [r] myindex [ajint] number of user-selected chain in
**                                    structure
** @param [r] startres [ajint] user-selected lower residue
**                                             number
** @param [r] finishres [ajint] user-selected upper residue
**                                             number
** @return [ajint] Last residue number
** @@
******************************************************************************/
static ajint psiphi_last_residue_number(const AjPPdb pdb,
					ajint myindex,
					ajint startres,
					ajint finishres)
{
    ajint lastres   = 0;
    ajint highres = 0;

    /* get number of highest residue available in chain */
    highres = 
	pdb->Chains[myindex]->Nres;

    /* last residue defaults to end of chain... */
    if(finishres == 1)
	lastres = highres+1;
    /* ERROR: finish residue too low */ 
    else if(finishres < startres)
	ajDie("Residue %d too low---number of lowest residue you chose is %d.",
	      finishres, startres );
    /* any other legitimate choice used as given */
    else if(finishres < highres)
	    lastres = finishres+1;
    else
	/* ERROR: finish residue too high */ 
	ajDie("No residue %d---number of highest residue in chain %d is %d.",
	      finishres, myindex,
	      highres );
    
    return lastres;
}




/* @funcstatic psiphi_phi_calculable ******************************************
**
** are all necessary atoms present to calculate phi torsion angle?
**
** @param [r] known [const AjBool*] corresponding array of AjBools
** @return [AjBool] ajTrue if calculable
** @@
******************************************************************************/
static AjBool psiphi_phi_calculable(const AjBool* known)
{
    AjBool phicalc = AJFALSE;
    /*
     * check for a complete set of atoms needed to calculate PHI
     */
    if(known[ECPrimePrev] &&
       known[ENCurr] &&
       known[ECAlphaCurr] &&
       known[ECPrimeCurr])
	phicalc = AJTRUE;

    return phicalc;
}




/* @funcstatic psiphi_psi_calculable ******************************************
**
** are all necessary atoms present to calculate psi torsion angle?
**
** @param [r] known [const AjBool*] corresponding array of AjBools
** @return [AjBool] ajTrue if calculable
** @@
******************************************************************************/
static AjBool psiphi_psi_calculable(const AjBool* known)
{
    AjBool psicalc = AJFALSE;
    /*
     * If you've got a complete set of the relevant
     * torsion atoms then calculate the PSI angle
     */
    if(known[ENCurr] &&
       known[ECAlphaCurr] &&
       known[ECPrimeCurr] &&
       known[ENNext])
    {
	psicalc = AJTRUE;
    }
    return psicalc;
}




/* @funcstatic psiphi_phival **************************************************
**
** returns the phi torsion angle between a specified set of AjPAtoms
**
** @param [r] atoms [AjPAtom const *] window of nine mainchain atoms
** @return [float] phi torsion angle
** @@
******************************************************************************/

static float psiphi_phival (AjPAtom const * atoms)
{
    float phi;
    
    AjP3dVector vec1To2 = NULL;
    AjP3dVector vec3To2 = NULL;
    AjP3dVector vec3To4 = NULL;

    /* construct vectors between four atoms relevant to torsion angles */
    vec1To2 = aj3dVectorNew();
    vec3To2 = aj3dVectorNew();
    vec3To4 = aj3dVectorNew();

    /* calculate PHI angle for current window */
    aj3dVectorBetweenPoints(vec1To2,
			    atoms[ECPrimePrev]->X,
			    atoms[ECPrimePrev]->Y,
			    atoms[ECPrimePrev]->Z,
			    atoms[ENCurr]->X,
			    atoms[ENCurr]->Y,
			    atoms[ENCurr]->Z);
    aj3dVectorBetweenPoints(vec3To2,
			    atoms[ECAlphaCurr]->X,
			    atoms[ECAlphaCurr]->Y,
			    atoms[ECAlphaCurr]->Z,
			    atoms[ENCurr]->X,
			    atoms[ENCurr]->Y,
			    atoms[ENCurr]->Z);
    aj3dVectorBetweenPoints(vec3To4,
			    atoms[ECAlphaCurr]->X,
			    atoms[ECAlphaCurr]->Y,
			    atoms[ECAlphaCurr]->Z,
			    atoms[ECPrimeCurr]->X,
			    atoms[ECPrimeCurr]->Y,
			    atoms[ECPrimeCurr]->Z);
		
    phi = (float) -1.0 *
	aj3dVectorDihedralAngle(vec1To2,
				vec3To2,
				vec3To4);
    /* clean up vectors */
    aj3dVectorDel(&vec1To2);
    aj3dVectorDel(&vec3To2);
    aj3dVectorDel(&vec3To4);
    
    return phi;
}




/* @funcstatic psiphi_psival **************************************************
**
** returns the psi torsion angle between a specified set of AjPAtoms
**
** @param [r] atoms [AjPAtom const *] window of nine mainchain atoms
** @return [float]  psi torsion angle
** @@
******************************************************************************/

static float psiphi_psival (AjPAtom const * atoms)
{
    float psi;
    
    AjP3dVector vec1To2 = NULL;
    AjP3dVector vec3To2 = NULL;
    AjP3dVector vec3To4 = NULL;

    /* construct vectors between four atoms relevant to torsion angles */
    vec1To2 = aj3dVectorNew();
    vec3To2 = aj3dVectorNew();
    vec3To4 = aj3dVectorNew();

    /* calculate PSI angle for current window */
    aj3dVectorBetweenPoints(vec1To2,
			    atoms[ENCurr]->X,
			    atoms[ENCurr]->Y,
			    atoms[ENCurr]->Z,
			    atoms[ECAlphaCurr]->X,
			    atoms[ECAlphaCurr]->Y,
			    atoms[ECAlphaCurr]->Z);
		
    aj3dVectorBetweenPoints(vec3To2,
			    atoms[ECPrimeCurr]->X,
			    atoms[ECPrimeCurr]->Y,
			    atoms[ECPrimeCurr]->Z,
			    atoms[ECAlphaCurr]->X,
			    atoms[ECAlphaCurr]->Y,
			    atoms[ECAlphaCurr]->Z);
		
    aj3dVectorBetweenPoints(vec3To4,
			    atoms[ECPrimeCurr]->X,
			    atoms[ECPrimeCurr]->Y,
			    atoms[ECPrimeCurr]->Z,
			    atoms[ENNext]->X,
			    atoms[ENNext]->Y,
			    atoms[ENNext]->Z);
    psi = (float) -1.0 *
	aj3dVectorDihedralAngle(vec1To2,
				vec3To2,
				vec3To4);
    /* clean up vectors */
    aj3dVectorDel(&vec1To2);
    aj3dVectorDel(&vec3To2);
    aj3dVectorDel(&vec3To4);

    return psi;
}




/* @funcstatic psiphi_load_previous_residue ***********************************
**
** checks and/or loads one mainchain AjPAtom from into window of AjPAtoms
** returns AJFALSE if non-residue atom
**
** @param [u] ajpAtom [AjPAtom] current AjPAtom from ajPPdb object
** @param [u] atoms [AjPAtom*] array of nine mainchain atoms
** @param [u] known [AjBool*] corresponding array of AjBools
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool psiphi_load_previous_residue(AjPAtom ajpAtom,
					   AjPAtom* atoms,
					   AjBool* known)
{
    AjBool isres = AJFALSE;

    if(ajpAtom->Id1 == 'X')
    {
	/* do nothing: this is not a residue */
	isres = AJFALSE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "N"))
    {
	known[ENPrev] = AJTRUE;
	atoms[ENPrev]  = ajpAtom;
	isres = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "CA"))
    {
	known[ECAlphaPrev] = AJTRUE;
	atoms[ECAlphaPrev]  = ajpAtom;
	isres = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "C"))
    {
	known[ECPrimePrev] = AJTRUE;
	atoms[ECPrimePrev]  = ajpAtom;
	isres = AJTRUE;
    }
    
    return isres;
}




/* @funcstatic psiphi_load_current_residue ************************************
**
** check and/or loads one mainchain AjPAtom into window of AjPAtoms
** returns AJFALSE if non-residue atom
**
** @param [u] ajpAtom [AjPAtom] current AjPAtom from ajPPdb object
** @param [u] atoms [AjPAtom*] array of nine mainchain atoms
** @param [u] known [AjBool*] corresponding array of AjBools
** @return [AjBool]  ajTrue on success
** @@
******************************************************************************/

static AjBool psiphi_load_current_residue(AjPAtom ajpAtom,
					  AjPAtom* atoms,
					  AjBool* known)
{
    AjBool isres = AJFALSE;
    
    if(ajpAtom->Id1 == 'X')
    {
	/* do nothing: this is not a residue */
	isres = AJFALSE;
	
    }
    else if(ajStrMatchC(ajpAtom->Atm, "N"))
    {
	known[ENCurr]  = AJTRUE;
	atoms[ENCurr] = ajpAtom;
	isres = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "CA"))
    {
	known[ECAlphaCurr]  = AJTRUE;
	atoms[ECAlphaCurr] = ajpAtom;
	isres = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "C"))
    {
	known[ECPrimeCurr]  = AJTRUE;
	atoms[ECPrimeCurr] = ajpAtom;
	isres = AJTRUE;
    }
    
    return isres;
}




/* @funcstatic psiphi_load_next_residue ***************************************
**
** loads AjPAtoms from next residue into window; returns AJTRUE if window full
** returns AJFALSE if non-residue atom
**
** @param [u] ajpAtom [AjPAtom] current AjPAtom from ajPPdb object
** @param [u] atoms [AjPAtom*] array of nine mainchain atoms
** @param [u] known [AjBool*] corresponding array of AjBools
** @return [AjBool]  ajTrue on success
** @@
******************************************************************************/

static AjBool psiphi_load_next_residue(AjPAtom ajpAtom,
				       AjPAtom* atoms,
				       AjBool* known)
{
    AjBool isres = AJFALSE;

    if(ajpAtom->Id1 == 'X')
    {
	/* do nothing: this is not a residue */
	;
	
    }
    else if(ajStrMatchC(ajpAtom->Atm, "N"))
    {
	known[ENNext]  = AJTRUE;
	atoms[ENNext] = ajpAtom;
	isres = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "CA"))
    {
	known[ECAlphaNext]  = AJTRUE;
	atoms[ECAlphaNext] = ajpAtom;
	isres = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "C"))
    {
	known[ECPrimeNext]  = AJTRUE;
	atoms[ECPrimeNext] = ajpAtom;
	isres = AJTRUE;
    }

    return isres;
}




/* @funcstatic psiphi_write_psi_phi *******************************************
**
** writes torsion angle features to a feature table and returns new feature  
**
** @param [u] angletab [AjPFeattable] table to write torsion
**                                                     angle to
** @param [r] resnum [ajint] residue that angle belongs to
** @param [r] phi [float] phi torsion angle for residue
** @param [r] psi [float] psi torsion angle for residue
** @return [AjPFeature] New feature stored in feature table
** @@
******************************************************************************/

static AjPFeature psiphi_write_psi_phi (AjPFeattable angletab,
					ajint resnum,
					float phi,
					float psi)
{
    AjPFeature angleft;
    AjPStr feattmp;

    feattmp = ajStrNew();

    /* create feature for torsion angles and write psi/phi */
    angleft = ajFeatNewII(angletab,
			  resnum,
			  resnum);
    ajFmtPrintS(&feattmp, "*phi %7.2f", phi);
    ajFeatTagAdd(angleft, NULL, feattmp);
    ajFmtPrintS(&feattmp, "*psi %7.2f", psi);
    ajFeatTagAdd(angleft, NULL, feattmp);

    ajStrDel(&feattmp);
    
    return angleft;
}




/* @funcstatic psiphi_shift_residues ***************************************
**
** moves AjPAtoms one residue along an array of mainchain AjPAtoms
**
** @param [u] atoms [AjPAtom*] array of nine mainchain atoms
** @param [u] known [AjBool*] corresponding array of AjBools
** @return [void]
** @@
******************************************************************************/

static void psiphi_shift_residues(AjPAtom* atoms,
				  AjBool* known)
{
    /* move previous atoms */
    if(known[ENCurr])
    {
	/* use old current N as new previous N */
	known[ENPrev]=
	    known[ENCurr];
	atoms[ENPrev] =
	    atoms[ENCurr];
    }
    if(known[ECAlphaCurr])
    {
	/* use old current CA as new previous CA */
	known[ECAlphaPrev] =
	    known[ECAlphaCurr];
	atoms[ECAlphaPrev] =
	    atoms[ECAlphaCurr];
    }
    if(known[ECPrimeCurr])
    {
	/* use old current C' as new previous C' */
	known[ECPrimePrev] =
	    known[ECPrimeCurr];
	atoms[ECPrimePrev] =
	    atoms[ECPrimeCurr];
    }
    if(known[ENNext])
    { 
	/* use old next N as new current N */
	known[ENCurr] =
	    known[ENNext];
	atoms[ENCurr] =
	    atoms[ENNext];
    }
    if(known[ECAlphaNext])
    {    
	/* use old next CA as new current CA */
	known[ECAlphaCurr] =
	    known[ECAlphaNext];
	atoms[ECAlphaCurr] =
	    atoms[ECAlphaNext];
    }
    if(known[ECPrimeNext])
    {	    
	/* use old next C as new current C */
	known[ECPrimeCurr] =
	    known[ECPrimeNext];
	atoms[ECPrimeCurr] =
	    atoms[ECPrimeNext];
    }
    /* clear next atoms */
    known[ECPrimeNext] = AJFALSE;
    known[ENNext]    = AJFALSE;
    known[ECAlphaNext] = AJFALSE;
    
    return;
}

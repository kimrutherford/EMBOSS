/****************************************************************************
**
** @source embdomain.c
**
** Algorithms for handling protein domain data.
** For use with Scop and Cath objects defined in ajdomain.h
** 
** Copyright (c) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
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




/* ======================================================================= */
/* ============================ Private data ============================= */
/* ======================================================================= */




/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */




/* ======================================================================= */
/* ========================== Private functions ========================== */
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




/* @func embScopToPdbid *****************************************************
**
** Read a scop identifier code and writes the equivalent pdb identifier code
**
** @param [r] scop [const AjPStr]   Scop identifier code
** @param [w] pdb  [AjPStr*]  Pdb identifier code
**
** @return [AjPStr] Pointer to pdb identifier code.
** @@
****************************************************************************/

AjPStr embScopToPdbid(const AjPStr scop, AjPStr *pdb)
{
    ajStrAssignSubS(pdb, scop, 1, 4);

    return *pdb;
}




/* @func embScopToSp ********************************************************
**
** Read a scop identifier code and writes the equivalent swissprot 
** identifier code.  Relies on a list of Pdbtosp objects sorted by PDB
** code, which is usually obtained by a call to ajPdbtospReadAllNew.
** 
** @param [r] scop  [const AjPStr]  Scop domain identifier code
** @param [w] spr   [AjPStr*]  Swissprot identifier code
** @param [r] list  [const AjPList]  Sorted list of Pdbtosp objects
**
** @return [AjBool]  True if a swissprot identifier code was
**                   found for the Scop code.
** @@
****************************************************************************/

AjBool embScopToSp(const AjPStr scop, AjPStr *spr, const AjPList list)
{
    AjPStr pdb = NULL;
    
    pdb = ajStrNew();
    
    if(embPdbidToSp(embScopToPdbid(scop, &pdb), spr, list))
    {
	ajStrDel(&pdb);

	return ajTrue;
    }

    ajStrDel(&pdb);

    return ajFalse;
}




/* @func embScopToAcc *******************************************************
**
** Read a scop identifier code and writes the equivalent accession number.
** Relies on a list of Pdbtosp objects sorted by PDB code, which is usually
** obtained by a call to ajPdbtospReadAllNew.
** 
** @param [r] scop  [const AjPStr]  Scop domain identifier code
** @param [w] acc   [AjPStr*]  Accession number
** @param [r] list  [const AjPList]  Sorted list of Pdbtosp objects
**
** @return [AjBool]  True if a swissprot identifier code was found for the
**                   Scop code.
** @@
****************************************************************************/

AjBool embScopToAcc(const AjPStr scop, AjPStr *acc, const AjPList list)
{
    AjPStr pdb = NULL;
    
    pdb = ajStrNew();
    
    if(embPdbidToAcc(embScopToPdbid(scop, &pdb), acc, list))
    {
	ajStrDel(&pdb);

	return ajTrue;
    }

    ajStrDel(&pdb);
    
    return ajFalse;
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

/****************************************************************************
**
** @source embpdb.h
** 
** Algorithms for handling protein structural data.   
** For use with Atom, Chain and Pdb objects defined in ajpdb.h
** Also for use with Hetent, Het, Vdwres, Vdwall, Cmap and Pdbtosp objects 
** (also in ajpdb.h)
** 
** @author: Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
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
****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embpdb_h
#define embpdb_h





/****************************************************************************
**
** @source embpdb.h
** 
** @source Algorithms for reading and writing ccf (clean coordinate file)
** format and for handling protein structural data.   
** For Atom, Chain and Pdb objects defined in ajpdb.h
** For Hetent, Het, Vdwres, Vdwall, Cmap and Pdbtosp objects (also in ajpdb.h)
** 
** @author: Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
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
***************************************************************************/




/*
** Prototype definitions
*/


/* ======================================================================= */
/* =========================== Pdb object ================================ */
/* ======================================================================= */
AjBool       embAtomInContact(const AjPAtom atm1, const AjPAtom atm2,
			      float thresh,
			      const AjPVdwall vdw);
float        embAtomDistance(const AjPAtom atm1, const AjPAtom atm2,
			     const AjPVdwall vdw);
AjBool       embPdbResidueIndexI(const AjPPdb pdb, ajint chn, AjPInt *idx);
AjBool       embPdbResidueIndexC(const AjPPdb pdb, char chn, AjPInt *idx);
AjBool       embPdbResidueIndexICA(const AjPPdb pdb, ajint chn, AjPUint *idx, 
				ajint *nres);
AjBool       embPdbResidueIndexCCA(const AjPPdb pdb, char chn, AjPUint *idx, 
				ajint *nres);
AjBool       embPdbToIdx(ajint *idx, const AjPPdb pdb, const AjPStr res,
			 ajint chn);
AjBool       embPdbListHeterogens(const AjPPdb pdb, AjPList *list_heterogens, 
				  AjPInt *siz_heterogens, ajint *nhet, 
				  AjPFile logfile);
AjBool       embPdbidToSp(const AjPStr pdb, AjPStr *spr, const AjPList list);
AjBool       embPdbidToAcc(const AjPStr pdb, AjPStr *acc, const AjPList list);
AjBool       embPdbidToScop(const AjPPdb pdb, const AjPList list_allscop, 
			    AjPList *list_pdbscopids);
float        embVdwRad(const AjPAtom atm, const AjPVdwall vdw);
AjBool       embStrideToThree(AjPStr *to, const AjPStr from);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif








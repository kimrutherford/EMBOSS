/****************************************************************************
**
** @source ajpdbio.h
** 
** Prototypes of functions for reading and writing PDB format.  
** For use with Pdb object defined in ajpdb.h
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

#ifndef ajpdbio_h
#define ajpdbio_h




/*
** Prototype definitions
*/

/* ======================================================================= */
/* =========================== Pdb object ================================ */
/* ======================================================================= */
AjPPdb ajPdbReadRawNew(AjPFile inf, const AjPStr pdbid, ajint min_chain_size, 
		       ajint max_mismatch, ajint max_trim, AjBool camask, 
		       AjBool camask1, AjBool atommask, AjPFile logf);

AjBool  ajPdbWriteAllRaw(ajint mode, const AjPPdb pdb, AjPFile outf, 
			  AjPFile errf);

AjBool  ajPdbWriteDomainRaw(ajint mode, const AjPPdb pdb, const AjPScop scop, 
			     AjPFile outf, AjPFile errf); 

AjBool  ajPdbWriteRecordRaw(ajint mode, const AjPPdb pdb, ajint mod, 
			     ajint chn, AjPFile outf, AjPFile errf); 

AjBool  ajPdbWriteDomainRecordRaw(ajint mode, const AjPPdb pdb, ajint mod,
				  const AjPScop scop,
				  AjPFile outf, AjPFile errf);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif


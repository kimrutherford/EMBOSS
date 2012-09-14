/* @include ajpdbio ***********************************************************
**
** Data structures and functions for reading and writing PDB format files.
** Includes functions for writing a Pdb object (defined in ajpdb.h).
**
** @author Copyright (c) 2004 Jon Ison
** @version $Revision: 1.12 $
** @modified $Date: 2012/04/12 20:38:09 $ by $Author: mks $
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

#ifndef AJPDBIO_H
#define AJPDBIO_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajdomain.h"
#include "ajpdb.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* @enum AjEPdbioMode *********************************************************
**
** AJAX PDB Input Output Mode enumeration for writing in PDB format
**
** @value ajEPdbioModeHeaderDomain
** Header line for domain PDB file
** @value ajEPdbioModeSeqresDomain
** SEQRES records for domain
** @value ajEPdbioModeAtomPdbDomain
** ATOM records for domain using original residue numbers
** @value ajEPdbioModeAtomIdxDomain
** ATOM records for domain using residues numbers that give  correct index
** into SEQRES sequence
** @value ajEPdbioModeSeqResChain
** SEQRES records for a chain
** @value ajEPdbioModeAtomPdbChain
** ATOM records for chain using original residue numbers
** @value ajEPdbioModeAtomIdxChain
** ATOM records for domain using residues numbers that  give correct index
** into SEQRES sequence
** @value ajEPdbioModeHeterogen
** ATOM line for a heterogen (small ligand)
** @value ajEPdbioModeHeader
** Header line
** @value ajEPdbioModeTitle
** Title line
** @value ajEPdbioModeCompnd
** COMPND records (info. on compound)
** @value ajEPdbioModeSource
** SOURCE records (info. on protein source)
** @value ajEPdbioModeEmptyRemark
** An empty REMARK record
** @value ajEPdbioModeResolution
** Record with resolution of the structure
** @@
******************************************************************************/

typedef enum AjOPdbioMode
{
    ajEPdbioModeHeaderDomain,
    ajEPdbioModeSeqresDomain,
    ajEPdbioModeAtomPdbDomain,
    ajEPdbioModeAtomIdxDomain,
    ajEPdbioModeSeqResChain,
    ajEPdbioModeAtomPdbChain,
    ajEPdbioModeAtomIdxChain,
    ajEPdbioModeHeterogen,
    ajEPdbioModeHeader,
    ajEPdbioModeTitle,
    ajEPdbioModeCompnd,
    ajEPdbioModeSource,
    ajEPdbioModeEmptyRemark,
    ajEPdbioModeResolution
} AjEPdbioMode;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

/* ======================================================================= */
/* =========================== Pdb object ================================ */
/* ======================================================================= */

AjPPdb ajPdbReadRawNew(AjPFile inf, const AjPStr pdbid, ajint min_chain_size,
                       ajint max_mismatch, ajint max_trim, AjBool camask,
                       AjBool camask1, AjBool atommask, AjPFile flog);

AjBool ajPdbWriteAllRaw(AjEPdbMode mode, const AjPPdb pdb, AjPFile outf,
                        AjPFile errf);

AjBool ajPdbWriteDomainRaw(AjEPdbMode mode, const AjPPdb pdb,
                           const AjPScop scop, AjPFile outf, AjPFile errf);

AjBool ajPdbWriteRecordRaw(AjEPdbioMode mode, const AjPPdb pdb, ajint mod,
                           ajint chn, AjPFile outf, AjPFile errf);

AjBool ajPdbWriteDomainRecordRaw(AjEPdbioMode mode, const AjPPdb pdb,
                                 ajint mod, const AjPScop scop,
                                 AjPFile outf, AjPFile errf);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJPDBIO_H */

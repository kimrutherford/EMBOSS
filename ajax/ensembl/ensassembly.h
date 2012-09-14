/* @include ensassembly *******************************************************
**
** Ensembl Assembly functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.12 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:23 $ by $Author: mks $
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

#ifndef ENSASSEMBLY_H
#define ENSASSEMBLY_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajax.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPAssembly *********************************************************
**
** Ensembl Assembly
**
** An Ensembl Assembly object encapsulates assembly information how an
** assembled Ensembl Sequence Region is assembled from a particular
** component Ensembl Sequence Region.
**
** @alias EnsSAssembly
** @alias EnsOAssembly
**
** @attr Use [ajuint] Use counter
** @attr AssembledIdentifier [ajuint] Assembled Ensembl Sequence Region
** identifier
** @attr AssembledStart [ajuint] Assembled Ensembl Sequence Region
** start coordinate
** @attr AssembledEnd [ajuint] Assembled Ensembl Sequence Region
** end coordinate
** @attr ComponentIdentifier [ajuint] Component Ensembl Sequence Region
** identifier
** @attr ComponentStart [ajuint] Component Ensembl Sequence Region
** start coordinate
** @attr ComponentEnd [ajuint] Component Ensembl Sequence Region
** end coordinate
** @attr Orientation [ajint] Relative orientation
** @@
******************************************************************************/

typedef struct EnsSAssembly
{
    ajuint Use;
    ajuint AssembledIdentifier;
    ajuint AssembledStart;
    ajuint AssembledEnd;
    ajuint ComponentIdentifier;
    ajuint ComponentStart;
    ajuint ComponentEnd;
    ajint Orientation;
} EnsOAssembly;

#define EnsPAssembly EnsOAssembly*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Assembly */

EnsPAssembly ensAssemblyNewCpy(const EnsPAssembly assembly);

EnsPAssembly ensAssemblyNewIni(ajuint asmsrid,
                               ajuint asmstart,
                               ajuint asmend,
                               ajuint cmpsrid,
                               ajuint cmpstart,
                               ajuint cmpend,
                               ajint orientation);

EnsPAssembly ensAssemblyNewRef(EnsPAssembly assembly);

void ensAssemblyDel(EnsPAssembly *Passembly);

ajuint ensAssemblyGetAssembledEnd(const EnsPAssembly assembly);

ajuint ensAssemblyGetAssembledIdentifier(const EnsPAssembly assembly);

ajuint ensAssemblyGetAssembledStart(const EnsPAssembly assembly);

ajuint ensAssemblyGetComponentEnd(const EnsPAssembly assembly);

ajuint ensAssemblyGetComponentIdentifier(const EnsPAssembly assembly);

ajuint ensAssemblyGetComponentStart(const EnsPAssembly assembly);

ajint ensAssemblyGetOrientation(const EnsPAssembly assembly);

AjBool ensAssemblyTrace(const EnsPAssembly assembly, ajuint level);

size_t ensAssemblyCalculateMemsize(const EnsPAssembly assembly);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSASSEMBLY_H */

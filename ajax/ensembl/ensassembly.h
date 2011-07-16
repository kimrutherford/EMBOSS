
#ifndef ENSASSEMBLY_H
#define ENSASSEMBLY_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

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




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

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

void ensAssemblyDel(EnsPAssembly* Passembly);

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

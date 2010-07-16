#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensassembly_h
#define ensassembly_h

#include "ajax.h"




/* @data EnsPAssembly *********************************************************
**
** Ensembl Assembly.
** An Ensembl Assembly object encapsulates assembly information how an
** assembled Ensembl Sequence Region is assembled from a particular
** component Ensembl Sequence Region.
**
** @alias EnsSAssembly
** @alias EnsOAssembly
**
** @attr Use [ajuint] Use counter
** @attr AssembledSeqregionId [ajuint] Assembled Ensembl Sequence Region
**                                     identifier
** @attr AssembledStart [ajuint] Assembled Ensembl Sequence Region
**                               start coordinate
** @attr AssembledEnd [ajuint] Assembled Ensembl Sequence Region
**                             end coordinate
** @attr ComponentSeqregionId [ajuint] Component Ensembl Sequence Region
**                                     identifier
** @attr ComponentStart [ajuint] Component Ensembl Sequence Region
**                               start coordinate
** @attr ComponentEnd [ajuint] Component Ensembl Sequence Region
**                             end coordinate
** @attr Orientation [ajint] Relative orientation
** @@
******************************************************************************/

typedef struct EnsSAssembly
{
    ajuint Use;
    ajuint AssembledSeqregionId;
    ajuint AssembledStart;
    ajuint AssembledEnd;
    ajuint ComponentSeqregionId;
    ajuint ComponentStart;
    ajuint ComponentEnd;
    ajint Orientation;
} EnsOAssembly;

#define EnsPAssembly EnsOAssembly*




/*
** Prototype definitions
*/

/* Ensembl Assembly */

EnsPAssembly ensAssemblyNew(ajuint asmsrid,
                            ajuint asmstart,
                            ajuint asmend,
                            ajuint cmpsrid,
                            ajuint cmpstart,
                            ajuint cmpend,
                            ajint orientation);

EnsPAssembly ensAssemblyNewObj(const EnsPAssembly object);

EnsPAssembly ensAssemblyNewRef(EnsPAssembly assembly);

void ensAssemblyDel(EnsPAssembly* Passembly);

ajuint ensAssemblyGetAssembledSeqregionId(const EnsPAssembly assembly);

ajuint ensAssemblyGetAssembledStart(const EnsPAssembly assembly);

ajuint ensAssemblyGetAssembledEnd(const EnsPAssembly assembly);

ajuint ensAssemblyGetComponentSeqregionId(const EnsPAssembly assembly);

ajuint ensAssemblyGetComponentStart(const EnsPAssembly assembly);

ajuint ensAssemblyGetComponentEnd(const EnsPAssembly assembly);

ajint ensAssemblyGetOrientation(const EnsPAssembly assembly);

AjBool ensAssemblyTrace(const EnsPAssembly assembly, ajuint level);

ajulong ensAssemblyGetMemsize(const EnsPAssembly assembly);

/*
** End of prototype definitions
*/




#endif /* ensassembly_h */

#ifdef __cplusplus
}
#endif

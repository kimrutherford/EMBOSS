#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensprojectionsegment_h
#define ensprojectionsegment_h

#include "ensslice.h"




/* @data EnsPProjectionsegment ************************************************
**
** Ensembl Projection Segment.
**
** Holds information about a genome sequence slice.
**
** @alias EnsSProjectionsegment
** @alias EnsOProjectionsegment
**
** @attr SrcStart [ajuint] Source start coordinate.
** @attr SrcEnd [ajuint] Source end coordinate.
** @attr TrgSlice [EnsPSlice] Target Ensembl Slice.
** @attr Use [ajuint] Use counter.
** @attr Padding [ajuint] Padding to alignment boundary.
** @@
******************************************************************************/

typedef struct EnsSProjectionsegment
{
    ajuint SrcStart;
    ajuint SrcEnd;
    EnsPSlice TrgSlice;
    ajuint Use;
    ajuint Padding;
} EnsOProjectionsegment;

#define EnsPProjectionsegment EnsOProjectionsegment*




/*
** Prototype definitions
*/

EnsPProjectionsegment ensProjectionsegmentNew(ajuint srcstart, ajuint srcend,
                                              EnsPSlice trgslice);

EnsPProjectionsegment ensProjectionsegmentNewRef(EnsPProjectionsegment ps);

void ensProjectionsegmentDel(EnsPProjectionsegment* Pps);

ajuint ensProjectionsegmentGetSrcStart(const EnsPProjectionsegment ps);

ajuint ensProjectionsegmentGetSrcEnd(const EnsPProjectionsegment ps);

EnsPSlice ensProjectionsegmentGetTrgSlice(const EnsPProjectionsegment ps);

AjBool ensProjectionsegmentTrace(const EnsPProjectionsegment ps, ajuint level);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

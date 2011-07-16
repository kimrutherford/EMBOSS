
#ifndef ENSPROJECTIONSEGMENT_H
#define ENSPROJECTIONSEGMENT_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensslice.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPProjectionsegment ************************************************
**
** Ensembl Projection Segment
**
** @alias EnsSProjectionsegment
** @alias EnsOProjectionsegment
**
** @attr SourceStart [ajuint] Source start coordinate
** @attr SourceEnd [ajuint] Source end coordinate
** @attr TargetSlice [EnsPSlice] Target Ensembl Slice
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSProjectionsegment
{
    ajuint SourceStart;
    ajuint SourceEnd;
    EnsPSlice TargetSlice;
    ajuint Use;
    ajuint Padding;
} EnsOProjectionsegment;

#define EnsPProjectionsegment EnsOProjectionsegment*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

EnsPProjectionsegment ensProjectionsegmentNewCpy(
    const EnsPProjectionsegment ps);

EnsPProjectionsegment ensProjectionsegmentNewIni(ajuint srcstart,
                                                 ajuint srcend,
                                                 EnsPSlice trgslice);

EnsPProjectionsegment ensProjectionsegmentNewRef(EnsPProjectionsegment ps);

void ensProjectionsegmentDel(EnsPProjectionsegment* Pps);

ajuint ensProjectionsegmentGetSourceStart(const EnsPProjectionsegment ps);

ajuint ensProjectionsegmentGetSourceEnd(const EnsPProjectionsegment ps);

EnsPSlice ensProjectionsegmentGetTargetSlice(const EnsPProjectionsegment ps);

AjBool ensProjectionsegmentTrace(const EnsPProjectionsegment ps, ajuint level);

size_t ensProjectionsegmentCalculateMemsize(const EnsPProjectionsegment ps);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSPROJECTIONSEGMENT_H */

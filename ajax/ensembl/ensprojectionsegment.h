/* @include ensprojectionsegment **********************************************
**
** Ensembl Projection Segment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.9 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:24 $ by $Author: mks $
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

#ifndef ENSPROJECTIONSEGMENT_H
#define ENSPROJECTIONSEGMENT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensslice.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

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




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

EnsPProjectionsegment ensProjectionsegmentNewCpy(
    const EnsPProjectionsegment ps);

EnsPProjectionsegment ensProjectionsegmentNewIni(ajuint srcstart,
                                                 ajuint srcend,
                                                 EnsPSlice trgslice);

EnsPProjectionsegment ensProjectionsegmentNewRef(EnsPProjectionsegment ps);

void ensProjectionsegmentDel(EnsPProjectionsegment *Pps);

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

/* @source Ensembl Projection Segment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.17 $
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
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensprojectionsegment.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensprojectionsegment ******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPProjectionsegment] Ensembl Projection Segment ************
**
** @nam2rule Projectionsegment Functions for manipulating
** Ensembl Projection Segment objects
**
** @cc Bio::EnsEMBL::ProjectionSegment
** @cc CVS Revision: 1.7
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Projection Segment by pointer.
** It is the responsibility of the user to first destroy any previous
** Projection Segment. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ps [const EnsPProjectionsegment] Ensembl Projection Segment
** @argrule Ini srcstart [ajuint] Source start coordinate
** @argrule Ini srcend [ajuint] Source end coordinate
** @argrule Ini trgslice [EnsPSlice] Target Ensembl Slice
** @argrule Ref ps [EnsPProjectionsegment] Ensembl Projection Segment
**
** @valrule * [EnsPProjectionsegment] Ensembl Projection Segment or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensProjectionsegmentNewCpy *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment or NULL
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewCpy(
    const EnsPProjectionsegment ps)
{
    EnsPProjectionsegment pthis = NULL;

    if(!ps)
        return NULL;

    AJNEW0(pthis);

    pthis->SourceStart = ps->SourceStart;
    pthis->SourceEnd   = ps->SourceEnd;
    pthis->TargetSlice = ensSliceNewRef(ps->TargetSlice);
    pthis->Use         = 1;

    return pthis;
}




/* @func ensProjectionsegmentNewIni *******************************************
**
** Constructor for an Ensembl Projection Segment with initial values.
**
** @param [r] srcstart [ajuint] Source start coordinate
** @param [r] srcend [ajuint] Source end coordinate
** @param [u] trgslice [EnsPSlice] Target Ensembl Slice
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment or NULL
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewIni(ajuint srcstart,
                                                 ajuint srcend,
                                                 EnsPSlice trgslice)
{
    EnsPProjectionsegment ps = NULL;

    if(ajDebugTest("ensProjectionsegmentNewIni"))
    {
        ajDebug("ensProjectionsegmentNewIni\n"
                "  srcstart %u\n"
                "  srcend %u\n"
                "  trgslice %p\n",
                srcstart,
                srcend,
                trgslice);

        ensSliceTrace(trgslice, 1);
    }

    if(!trgslice)
        return NULL;

    AJNEW0(ps);

    ps->SourceStart = srcstart;
    ps->SourceEnd   = srcend;
    ps->TargetSlice = ensSliceNewRef(trgslice);
    ps->Use         = 1;

    return ps;
}




/* @func ensProjectionsegmentNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ps [EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewRef(EnsPProjectionsegment ps)
{
    if(!ps)
        return NULL;

    ps->Use++;

    return ps;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Del Destroy (free) an Ensembl Projection Segment object
**
** @argrule * Pps [EnsPProjectionsegment*] Ensembl Projection Segment
**                                         object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProjectionsegmentDel **********************************************
**
** Default destructor for an Ensembl Projection Segment.
**
** @param [d] Pps [EnsPProjectionsegment*] Ensembl Projection Segment
**                                         object address
**
** @return [void]
** @@
******************************************************************************/

void ensProjectionsegmentDel(EnsPProjectionsegment* Pps)
{
    EnsPProjectionsegment pthis = NULL;

    if(!Pps)
        return;

    if(!*Pps)
        return;

    pthis = *Pps;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pps = NULL;

        return;
    }

    ensSliceDel(&pthis->TargetSlice);

    AJFREE(pthis);

    *Pps = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Projection Segment.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Get Get an attribute of an Ensembl Projection Segment
** @nam4rule Source Get source members
** @nam5rule End Get source end element
** @nam5rule Start Get source start element
** @nam4rule Target Get tagret members
** @nam5rule Slice Get the target Ensembl Slice
**
** @argrule * ps [const EnsPProjectionsegment] Projection Segment object
** address
**
** @valrule SourceEnd [ajuint] Source end or 0
** @valrule SourceStart [ajuint] Source start or 0
** @valrule TargetSlice [EnsPSlice] Target Ensembl Slice or NULL
**
** @fcategory delete
******************************************************************************/




/* @func ensProjectionsegmentGetSourceEnd *************************************
**
** Get the source end element of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [ajuint] Source end coordinate or 0
** @@
******************************************************************************/

ajuint ensProjectionsegmentGetSourceEnd(const EnsPProjectionsegment ps)
{
    if(!ps)
        return 0;

    return ps->SourceEnd;
}




/* @func ensProjectionsegmentGetSourceStart ***********************************
**
** Get the source start element of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [ajuint] Source start coordinate or 0
** @@
******************************************************************************/

ajuint ensProjectionsegmentGetSourceStart(const EnsPProjectionsegment ps)
{
    if(!ps)
        return 0;

    return ps->SourceStart;
}




/* @func ensProjectionsegmentGetTargetSlice ***********************************
**
** Get the target Ensembl Slice element of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [EnsPSlice] Target Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPSlice ensProjectionsegmentGetTargetSlice(const EnsPProjectionsegment ps)
{
    if(!ps)
        return NULL;

    return ps->TargetSlice;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Trace Report Ensembl Projection Segment elements to debug file
**
** @argrule Trace ps [const EnsPProjectionsegment] Ensembl Projection Segment
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensProjectionsegmentTrace ********************************************
**
** Trace an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProjectionsegmentTrace(const EnsPProjectionsegment ps, ajuint level)
{
    AjPStr indent = NULL;

    if(!ps)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensProjectionsegmentTrace %p\n"
            "%S  SourceStart %d\n"
            "%S  SourceEnd %d\n"
            "%S  TargetSlice %p\n"
            "%S  Use %u\n",
            indent, ps,
            indent, ps->SourceStart,
            indent, ps->SourceEnd,
            indent, ps->TargetSlice,
            indent, ps->Use);

    ensSliceTrace(ps->TargetSlice, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Calculate Calculate Ensembl Projection Segment values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensProjectionsegmentCalculateMemsize *********************************
**
** Calculate the memory size in bytes of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensProjectionsegmentCalculateMemsize(const EnsPProjectionsegment ps)
{
    size_t size = 0;

    if(!ps)
        return 0;

    size += sizeof (EnsOProjectionsegment);

    size += ensSliceCalculateMemsize(ps->TargetSlice);

    return size;
}

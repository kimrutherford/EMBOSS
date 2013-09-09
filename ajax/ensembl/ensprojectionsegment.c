/* @source ensprojectionsegment ***********************************************
**
** Ensembl Projection Segment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.30 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensprojectionsegment.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




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
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-68
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewCpy(
    const EnsPProjectionsegment ps)
{
    EnsPProjectionsegment pthis = NULL;

    if (!ps)
        return NULL;

    AJNEW0(pthis);

    pthis->SourceStart = ps->SourceStart;
    pthis->SourceEnd   = ps->SourceEnd;
    pthis->TargetSlice = ensSliceNewRef(ps->TargetSlice);
    pthis->Use         = 1U;

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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewIni(ajuint srcstart,
                                                 ajuint srcend,
                                                 EnsPSlice trgslice)
{
    EnsPProjectionsegment ps = NULL;

    if (ajDebugTest("ensProjectionsegmentNewIni"))
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

    if (!trgslice)
        return NULL;

    AJNEW0(ps);

    ps->SourceStart = srcstart;
    ps->SourceEnd   = srcend;
    ps->TargetSlice = ensSliceNewRef(trgslice);
    ps->Use         = 1U;

    return ps;
}




/* @func ensProjectionsegmentNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ps [EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewRef(EnsPProjectionsegment ps)
{
    if (!ps)
        return NULL;

    ps->Use++;

    return ps;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Del Destroy (free) an Ensembl Projection Segment
**
** @argrule * Pps [EnsPProjectionsegment*] Ensembl Projection Segment address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProjectionsegmentDel **********************************************
**
** Default destructor for an Ensembl Projection Segment.
**
** @param [d] Pps [EnsPProjectionsegment*] Ensembl Projection Segment address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensProjectionsegmentDel(EnsPProjectionsegment *Pps)
{
    EnsPProjectionsegment pthis = NULL;

    if (!Pps)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensProjectionsegmentDel"))
    {
        ajDebug("ensProjectionsegmentDel\n"
                "  *Pps %p\n",
                *Pps);

        ensProjectionsegmentTrace(*Pps, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pps) || --pthis->Use)
    {
        *Pps = NULL;

        return;
    }

    ensSliceDel(&pthis->TargetSlice);

    ajMemFree((void **) Pps);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Projection Segment.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Get Get an attribute of an Ensembl Projection Segment
** @nam4rule Source Get source members
** @nam5rule End Get source end member
** @nam5rule Start Get source start member
** @nam4rule Target Get tagret members
** @nam5rule Slice Get the target Ensembl Slice
**
** @argrule * ps [const EnsPProjectionsegment] Projection Segment object
** address
**
** @valrule SourceEnd [ajuint] Source end or 0U
** @valrule SourceStart [ajuint] Source start or 0U
** @valrule TargetSlice [EnsPSlice] Target Ensembl Slice or NULL
**
** @fcategory delete
******************************************************************************/




/* @func ensProjectionsegmentGetSourceEnd *************************************
**
** Get the source end member of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [ajuint] Source end coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensProjectionsegmentGetSourceEnd(const EnsPProjectionsegment ps)
{
    return (ps) ? ps->SourceEnd : 0U;
}




/* @func ensProjectionsegmentGetSourceStart ***********************************
**
** Get the source start member of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [ajuint] Source start coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensProjectionsegmentGetSourceStart(const EnsPProjectionsegment ps)
{
    return (ps) ? ps->SourceStart : 0U;
}




/* @func ensProjectionsegmentGetTargetSlice ***********************************
**
** Get the target Ensembl Slice member of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment
**
** @return [EnsPSlice] Target Ensembl Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSlice ensProjectionsegmentGetTargetSlice(const EnsPProjectionsegment ps)
{
    return (ps) ? ps->TargetSlice : NULL;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Trace Report Ensembl Projection Segment members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProjectionsegmentTrace(const EnsPProjectionsegment ps, ajuint level)
{
    AjPStr indent = NULL;

    if (!ps)
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
** Functions for calculating information from an
** Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
**
** @nam3rule Calculate Calculate Ensembl Projection Segment information
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
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensProjectionsegmentCalculateMemsize(const EnsPProjectionsegment ps)
{
    size_t size = 0;

    if (!ps)
        return 0;

    size += sizeof (EnsOProjectionsegment);

    size += ensSliceCalculateMemsize(ps->TargetSlice);

    return size;
}

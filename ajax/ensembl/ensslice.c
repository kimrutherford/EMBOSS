/* @source ensslice ***********************************************************
**
** Ensembl Slice functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.68 $
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

#include "enscache.h"
#include "ensassemblyexception.h"
#include "ensassemblymapper.h"
#include "ensattribute.h"
#include "ensmapper.h"
#include "ensprojectionsegment.h"
#include "enssequence.h"
#include "enssequenceedit.h"
#include "ensslice.h"
#include "ensrepeat.h"
#include "enstable.h"
#include "enstranslation.h"




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

/* @conststatic sliceKSequenceeditCode ****************************************
**
** Ensembl Sequence Edit objects for Ensembl Slice objects are a sub-set of
** Ensembl Attribute objects that provide information about modifications of
** the Slice sequence. Ensembl Attribute objects with the following codes are
** Sequence Edit objects on the Slice-level.
**
** _rna_edit: General sequence edit
**
******************************************************************************/

static const char *sliceKSequenceeditCode[] =
{
    "_rna_edit",
    (const char *) NULL
};




/* @conststatic sliceKType ****************************************************
**
** The Ensembl Slice Type member is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsESliceType.
**
******************************************************************************/

static const char *const sliceKType[] =
{
    "",
    "Linear",
    "Circular",
    "LocusReferenceGenome",
    (const char *) NULL
};





/* @conststatic sliceadaptorKCacheMaxBytes ************************************
**
** Maximum memory size in bytes the Ensembl Slice Adaptor-internal
** Ensembl Cache can use.
**
** 1 << 26 = 64 MiB
**
******************************************************************************/

static const size_t sliceadaptorKCacheMaxBytes = 1U << 26U;




/* @conststatic sliceadaptorKCacheMaxCount ************************************
**
** Maximum number of Ensembl Slice objects the Ensembl Slice Adaptor-internal
** Ensembl Cache can hold.
**
** 1 << 16 = 64 Ki
**
******************************************************************************/

static const ajuint sliceadaptorKCacheMaxCount = 1U << 16U;




/* @conststatic sliceadaptorKCacheMaxSize *************************************
**
** Maximum memory size in bytes of an Ensembl Slice to be allowed into the
** Ensembl Slice Adaptor-internal Ensembl Cache.
**
******************************************************************************/

static const size_t sliceadaptorKCacheMaxSize = 0U;




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static EnsPProjectionsegment sliceConstrain(EnsPSlice slice);

static AjBool sliceProject(EnsPSlice slice,
                           EnsPCoordsystem trgcs,
                           AjPList pss);

static int listSliceCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listSliceCompareIdentifierDescending(
    const void *item1,
    const void *item2);

static int listSliceCompareNameAscending(
    const void *item1,
    const void *item2);

static int listSliceCompareNameDescending(
    const void *item1,
    const void *item2);

static void sliceadaptorCacheDelete(void **Pvalue);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensslice ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSlice] Ensembl Slice *************************************
**
** @nam2rule Slice Functions for manipulating Ensembl Slice objects
**
** @cc Bio::EnsEMBL::Slice
** @cc CVS Revision: 1.301
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::CircularSlice
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Slice by pointer.
** It is the responsibility of the user to first destroy any previous
** Slice. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSlice]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
** @nam4rule Seq Constructor with a sequence
**
** @argrule Cpy slice [const EnsPSlice] Ensembl Slice
** @argrule Ini sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @argrule Ini sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Ini start [ajint] Ensembl Sequence Region start
** @argrule Ini end [ajint] Ensembl Sequence Region end
** @argrule Ini strand [ajint] Ensembl Sequence Region strand
** @argrule Ref slice [EnsPSlice] Ensembl Slice
** @argrule Seq sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @argrule Seq sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Seq start [ajint] Ensembl Sequence Region start
** @argrule Seq end [ajint] Ensembl Sequence Region end
** @argrule Seq strand [ajint] Ensembl Sequence Region strand
** @argrule Seq sequence [AjPStr] Sequence (optional)
**
** @valrule * [EnsPSlice] Ensembl Slice or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensSliceNewCpy *******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPSlice] Ensembl Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSlice ensSliceNewCpy(const EnsPSlice slice)
{
    EnsPSlice pthis = NULL;

    if (!slice)
        return NULL;

    AJNEW0(pthis);

    pthis->Adaptor   = slice->Adaptor;
    pthis->Seqregion = ensSeqregionNewRef(slice->Seqregion);

    if (slice->Sequence)
        pthis->Sequence = ajStrNewRef(slice->Sequence);

    pthis->Topology = slice->Topology;
    pthis->Type     = slice->Type;
    pthis->Start    = slice->Start;
    pthis->End      = slice->End;
    pthis->Strand   = slice->Strand;
    pthis->Use      = 1U;

    return pthis;
}




/* @func ensSliceNewIni *******************************************************
**
** Constructor for an Ensembl Slice with initial values.
**
** @cc Bio::EnsEMBL::Slice::new
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajint] Ensembl Sequence Region start
** @param [r] end [ajint] Ensembl Sequence Region end
** @param [r] strand [ajint] Ensembl Sequence Region strand
**
** @return [EnsPSlice] Ensembl Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSlice ensSliceNewIni(EnsPSliceadaptor sla,
                         EnsPSeqregion sr,
                         ajint start,
                         ajint end,
                         ajint strand)
{
    EnsPSlice slice = NULL;

    if (!sr)
    {
        ajDebug("ensSliceNewIni requires an Ensembl Sequence Region.\n");

        return NULL;
    }

    if (ensCoordsystemGetToplevel(ensSeqregionGetCoordsystem(sr)))
    {
        ajDebug("ensSliceNewIni cannot create a Slice on a Sequence Region "
                "with a top-level Coordinate System.\n");

        return NULL;
    }

    if ((strand != 1) && (strand != -1))
    {
        ajDebug("ensSliceNewIni requires a strand of either 1 or -1 "
                "not %d.\n", strand);

        return NULL;
    }

    AJNEW0(slice);

    slice->Adaptor   = sla;
    slice->Seqregion = ensSeqregionNewRef(sr);
    slice->Sequence  = NULL;
    slice->Topology  = ensESliceTopologyNULL;
    slice->Type      = ensSliceTypeFromSeqregion(sr);
    slice->Start     = start;
    slice->End       = end;
    slice->Strand    = strand;
    slice->Use       = 1U;

    return slice;
}




/* @func ensSliceNewRef *******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsPSlice] Ensembl Slice or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSlice ensSliceNewRef(EnsPSlice slice)
{
    if (!slice)
        return NULL;

    slice->Use++;

    return slice;
}




/* @func ensSliceNewSeq *******************************************************
**
** Construct a new Ensembl Slice with sequence information.
**
** @cc Bio::EnsEMBL::Slice::new
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajint] Ensembl Sequence Region start
** @param [r] end [ajint] Ensembl Sequence Region end
** @param [r] strand [ajint] Ensembl Sequence Region strand
** @param [u] sequence [AjPStr] Sequence (optional)
**
** @return [EnsPSlice] Ensembl Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSlice ensSliceNewSeq(EnsPSliceadaptor sla,
                         EnsPSeqregion sr,
                         ajint start,
                         ajint end,
                         ajint strand,
                         AjPStr sequence)
{
    EnsPSlice slice = NULL;

    if (!sr)
    {
        ajDebug("ensSliceNewSeq requires an Ensembl Sequence Region.\n");

        return NULL;
    }

    if (ensCoordsystemGetToplevel(ensSeqregionGetCoordsystem(sr)))
    {
        ajDebug("ensSliceNewSeq cannot create a Slice on a Sequence Region "
                "with a top-level Coordinate System.\n");

        return NULL;
    }

    if (!start)
        start = 1;

    if (!end)
        end = ensSeqregionGetLength(sr);

    if (start > (end + 1))
    {
        ajDebug("ensSliceNewSeq requires that the start coordinate %d is "
                "less than or equal to the end coordinate %d + 1.\n",
                start, end);

        return NULL;
    }

    if (!strand)
        strand = 1;

    if ((strand != 1) && (strand != -1))
    {
        ajDebug("ensSliceNewSeq requires a strand of either 1 or -1 "
                "not %d.\n", strand);

        return NULL;
    }

    if (sequence &&
        (ajStrGetLen(sequence) != (ajuint) ensSeqregionGetLength(sr)))
    {
        ajDebug("ensSliceNewSeq requires that the Sequence Region length %d "
                "matches the length of the Sequence string %u.\n",
                ensSeqregionGetLength(sr), ajStrGetLen(sequence));

        return NULL;
    }

    AJNEW0(slice);

    slice->Adaptor = sla;

    slice->Seqregion = ensSeqregionNewRef(sr);

    if (sequence)
        slice->Sequence = ajStrNewRef(sequence);

    slice->Topology = ensESliceTopologyNULL;
    slice->Type     = ensSliceTypeFromSeqregion(sr);
    slice->Start    = start;
    slice->End      = end;
    slice->Strand   = strand;
    slice->Use      = 1U;

    return slice;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Slice object.
**
** @fdata [EnsPSlice]
**
** @nam3rule Del Destroy (free) an Ensembl Slice
**
** @argrule * Pslice [EnsPSlice*] Ensembl Slice address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSliceDel **********************************************************
**
** Default destructor for an Ensembl Slice.
**
** @param [d] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensSliceDel(EnsPSlice *Pslice)
{
    EnsPSlice pthis = NULL;

    if (!Pslice)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensSliceDel"))
    {
        ajDebug("ensSliceDel\n"
                "  *Pslice %p\n",
                *Pslice);

        ensSliceTrace(*Pslice, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pslice) || --pthis->Use)
    {
        *Pslice = NULL;

        return;
    }

    ensSeqregionDel(&pthis->Seqregion);

    ajStrDel(&pthis->Sequence);

    ajMemFree((void **) Pslice);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
**
** @nam3rule Get Return Slice attribute(s)
** @nam4rule Adaptor Return the Ensembl Slice Adaptor
** @nam4rule End Return the end
** @nam4rule Seqregion Return the Ensembl Sequence Region
** @nam4rule Sequence Return the sequence
** @nam4rule Start Return the start
** @nam4rule Strand Return the strand
**
** @argrule * slice [const EnsPSlice] Slice
**
** @valrule Adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor or NULL
** @valrule End [ajint] End or 0
** @valrule Seqregion [EnsPSeqregion] Ensembl Sequence Region or NULL
** @valrule Sequence [const AjPStr] Sequence or NULL
** @valrule Start [ajint] Start or 0
** @valrule Strand [ajint] Strand or 0
**
** @fcategory use
******************************************************************************/




/* @func ensSliceGetAdaptor ***************************************************
**
** Get the Ensembl Slice Adaptor member an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::adaptor
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPSliceadaptor] Ensembl Slice Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSliceadaptor ensSliceGetAdaptor(const EnsPSlice slice)
{
    return (slice) ? slice->Adaptor : NULL;
}




/* @func ensSliceGetEnd *******************************************************
**
** Get the end coordinate member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::end
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] End coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensSliceGetEnd(const EnsPSlice slice)
{
    return (slice) ? slice->End : 0;
}




/* @func ensSliceGetSeqregion *************************************************
**
** Get the Ensembl Sequence Region member of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPSeqregion] Ensembl Sequence Region or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSeqregion ensSliceGetSeqregion(const EnsPSlice slice)
{
    return (slice) ? slice->Seqregion : NULL;
}




/* @func ensSliceGetSequence **************************************************
**
** Get the sequence member of an Ensembl Slice.
**
** NOTE: This function returns only the sequence member that has been set for
** an Ensembl Slice object that is not based on an Ensembl Sequence Region.
** For Ensembl Slice objects based on Ensembl Sequence Region objects the
** sequence is fetched dynamically via ensSliceFetchSequence functions.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensSliceGetSequence(const EnsPSlice slice)
{
    return (slice) ? slice->Sequence : NULL;
}




/* @func ensSliceGetStart *****************************************************
**
** Get the start coordinate member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::start
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Start coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensSliceGetStart(const EnsPSlice slice)
{
    return (slice) ? slice->Start : 0;
}




/* @func ensSliceGetStrand ****************************************************
**
** Get the strand member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::strand
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Strand information or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensSliceGetStrand(const EnsPSlice slice)
{
    return (slice) ? slice->Strand : 0;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Slice object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPSlice]
**
** @nam3rule Load Return Ensembl Slice attribute(s) loaded on demand
** @nam4rule Topology Return the topology
**
** @argrule * slice [EnsPSlice] Ensembl Slice
**
** @valrule Topology [EnsESliceTopology] Topology or ensESliceTopologyNULL
**
** @fcategory use
******************************************************************************/




/* @func ensSliceLoadTopology *************************************************
**
** Load the topology member of an Ensembl Slice.
**
** This is not a simple accessor function, since it will attempt fetching
** Ensembl Attribute objects from the Ensembl database associated with the
** Ensembl Sequence Region.
**
** @cc Bio::EnsEMBL::Slice::is_circular
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsESliceTopology] Topology or ensESliceTopologyNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsESliceTopology ensSliceLoadTopology(EnsPSlice slice)
{
    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute attribute = NULL;

    if (!slice)
        return ensESliceTopologyNULL;

    if (slice->Topology == ensESliceTopologyNULL)
    {
        if (slice->Seqregion == NULL)
            return ensESliceTopologyNULL;

        code = ajStrNewC("circular_seq");

        attributes = ajListNew();

        ensSliceFetchAllAttributes(slice, code, attributes);

        if (ajListGetLength(attributes) > 0)
            slice->Topology = ensESliceTopologyCircular;
        else
            slice->Topology = ensESliceTopologyLinear;

        while (ajListPop(attributes, (void **) &attribute))
            ensAttributeDel(&attribute);

        ajListFree(&attributes);

        ajStrDel(&code);
    }

    return slice->Topology;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
**
** @nam3rule Set Set one member of a Slice
** @nam4rule Adaptor Set the Ensembl Slice Adaptor
** @nam4rule Sequence Set the sequence
** @nam4rule Topology Set the topology
**
** @argrule * slice [EnsPSlice] Ensembl Slice object
** @argrule Adaptor sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @argrule Sequence sequence [AjPStr] Sequence
** @argrule Topology sltp [EnsESliceTopology] Ensembl Slice Topology
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSliceSetAdaptor ***************************************************
**
** Set the Ensembl Slice Adaptor member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceSetAdaptor(EnsPSlice slice, EnsPSliceadaptor sla)
{
    if (!slice)
        return ajFalse;

    slice->Adaptor = sla;

    return ajTrue;
}




/* @func ensSliceSetSequence **************************************************
**
** Set the sequence member of an Ensembl Slice.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] sequence [AjPStr] Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceSetSequence(EnsPSlice slice, AjPStr sequence)
{
    if (!slice)
        return ajFalse;

    ajStrDel(&slice->Sequence);

    if (sequence)
    {
        if (ajStrGetLen(sequence) == ensSliceCalculateLength(slice))
            slice->Sequence = ajStrNewRef(sequence);
        else
            ajFatal("ensSliceSetSequence got sequence of length %u, "
                    "which does not match the length of the Slice %u.\n",
                    ajStrGetLen(sequence),
                    ensSliceCalculateLength(slice));
    }

    return ajTrue;
}




/* @func ensSliceSetTopology **************************************************
**
** Set the Ensembl Slice Topology member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::is_circular
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] sltp [EnsESliceTopology] Ensembl Slice Topology
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceSetTopology(EnsPSlice slice, EnsESliceTopology sltp)
{
    if (!slice)
        return ajFalse;

    slice->Topology = sltp;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
**
** @nam3rule Trace Report Ensembl Slice members to debug file
**
** @argrule Trace slice [const EnsPSlice] Ensembl Slice
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceTrace ********************************************************
**
** Trace an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceTrace(const EnsPSlice slice, ajuint level)
{
    AjPStr indent = NULL;

    if (!slice)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSliceTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Seqregion %p\n"
            "%S  Sequence %p\n"
            "%S  Topology %d\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Strand %d\n"
            "%S  Use %u\n",
            indent, slice,
            indent, slice->Adaptor,
            indent, slice->Seqregion,
            indent, slice->Sequence,
            indent, slice->Topology,
            indent, slice->Start,
            indent, slice->End,
            indent, slice->Strand,
            indent, slice->Use);

    ensSeqregionTrace(slice->Seqregion, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Slice convenience functions
**
** @fdata [EnsPSlice]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Coordsystem Get Ensembl Coordinate System member(s)
** @nam5rule Name Get the name
** @nam5rule Object Get the Ensembl Coordinate System object
** @nam5rule Version Get the version
** @nam4rule Seqregion Get Ensembl Sequence Region member(s)
** @nam5rule Identifier Get the SQL database-internal identifier
** @nam5rule Length Get the length
** @nam5rule Name Get the name
** @nam4rule Translation Get an AJAX Translation
**
** @argrule Coordsystem slice [const EnsPSlice] Ensembl Slice
** @argrule Seqregion slice [const EnsPSlice] Ensembl Slice
** @argrule Translation slice [EnsPSlice] Ensembl Slice
**
** @valrule CoordsystemObject [EnsPCoordsystem] Ensembl Coordinate System or
** NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [const AjPStr] Name or NULL
** @valrule Length [ajint] Length or 0
** @valrule Version [const AjPStr] Version or NULL
** @valrule Translation [const AjPTrn] AJAX Translation or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSliceGetCoordsystemName *******************************************
**
** Get the name member of the Ensembl Coordinate System member of the
** Ensembl Sequence Region member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::coord_system_name
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Ensembl Coordinate System name
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensSliceGetCoordsystemName(const EnsPSlice slice)
{
    return (slice) ?
        ensCoordsystemGetName(
            ensSeqregionGetCoordsystem(slice->Seqregion)) : NULL;
}




/* @func ensSliceGetCoordsystemObject *****************************************
**
** Get the Ensembl Coordinate System member of the
** Ensembl Sequence Region member of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::coord_system
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPCoordsystem] Ensembl Coordinate System
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPCoordsystem ensSliceGetCoordsystemObject(const EnsPSlice slice)
{
    return (slice) ? ensSeqregionGetCoordsystem(slice->Seqregion) : NULL;
}




/* @func ensSliceGetCoordsystemVersion ****************************************
**
** Get the version member of the Ensembl Coordinate System member of the
** Ensembl Sequence Region member of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Ensembl Coordinate System version or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensSliceGetCoordsystemVersion(const EnsPSlice slice)
{
    return (slice) ?
        ensCoordsystemGetVersion(
            ensSeqregionGetCoordsystem(slice->Seqregion)) : NULL;
}




/* @func ensSliceGetSeqregionIdentifier ***************************************
**
** Get the identifier member of the Ensembl Sequence Region member of an
** Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajuint] Ensembl Sequence Region identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensSliceGetSeqregionIdentifier(const EnsPSlice slice)
{
    return (slice) ? ensSeqregionGetIdentifier(slice->Seqregion) : 0U;
}




/* @func ensSliceGetSeqregionLength *******************************************
**
** Calculate the length of the Ensembl Sequence Region member of an
** Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Ensembl Sequence Region length or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensSliceGetSeqregionLength(const EnsPSlice slice)
{
    return (slice) ? ensSeqregionGetLength(slice->Seqregion) : 0;
}




/* @func ensSliceGetSeqregionName *********************************************
**
** Get the name member of the Ensembl Sequence Region member of an
** Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Ensembl Sequence Region name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensSliceGetSeqregionName(const EnsPSlice slice)
{
    return (slice) ? ensSeqregionGetName(slice->Seqregion) : NULL;
}




/* @func ensSliceGetTranslation ***********************************************
**
** Get an AJAX Translation for an Ensembl Slice.
**
** The AJAX Translation will match the codon table defined as an
** Ensembl Attribute of code "codon_table" associated with an Ensembl Slice.
** If no Attribute is associated with this Slice, an AJAX Translation based on
** codon table 0, the standard code with translation start at AUG only,
** will be returned.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [const AjPTrn] AJAX Translation or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

const AjPTrn ensSliceGetTranslation(EnsPSlice slice)
{
    ajuint codontable = 0U;

    AjPList attributes = NULL;

    AjPStr code  = NULL;
    AjPStr value = NULL;

    EnsPAttribute attribute = NULL;

    if (!slice)
        return NULL;

    code = ajStrNewC("codon_table");

    attributes = ajListNew();

    ensSliceFetchAllAttributes(slice, code, attributes);

    ajStrDel(&code);

    while (ajListPop(attributes, (void **) &attribute))
    {
        value = ensAttributeGetValue(attribute);

        if (value && ajStrGetLen(value))
        {
            if (!ajStrToUint(value, &codontable))
                ajWarn("ensSliceGetTranslation Could not parse "
                       "Ensembl Attribute value '%S' into an "
                       "unsigned integer value.",
                       value);
        }
        else
        {
            ajDebug("ensSliceGetTranslation got Ensembl Attribute %p with an "
                    "empty value.",
                    attribute);

            ensAttributeTrace(attribute, 1);
        }

        ensAttributeDel(&attribute);
    }

    ajListFree(&attributes);

    return ensTranslationCacheGet(codontable);
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an Ensembl Slice object.
**
** @fdata [EnsPSlice]
**
** @nam3rule Calculate   Calculate Ensembl Slice information
** @nam4rule Centrepoint Calculate the centre point
** @nam4rule Length      Calculate the length
** @nam4rule Memsize     Calculate the memory size in bytes
** @nam4rule Region      Calculate the length of a region
**
** @argrule Centrepoint slice [EnsPSlice] Ensembl Slice
** @argrule Length      slice [EnsPSlice] Ensembl Slice
** @argrule Memsize     slice [const EnsPSlice] Ensembl Slice
** @argrule Region      slice [EnsPSlice] Ensembl Slice
** @argrule Region      start [ajint] Start
** @argrule Region      end   [ajint] End
**
** @valrule Centrepoint [ajint] Centre point or 0
** @valrule Length      [ajuint] Length or 0U
** @valrule Memsize     [size_t] Memory size in bytes or 0
** @valrule Region      [ajuint] Length or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceCalculateCentrepoint *****************************************
**
** Calculate the centre point coordinate of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::centrepoint
** @cc Bio::EnsEMBL::CircularSlice::centrepoint
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [ajint] Centre point coordinate or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensSliceCalculateCentrepoint(EnsPSlice slice)
{
    ajint mpoint  = 0;

    if (!slice)
        return 0;

    mpoint = slice->Start + ensSliceCalculateLength(slice) / 2;

    if (mpoint > ensSeqregionGetLength(slice->Seqregion))
        mpoint -= ensSeqregionGetLength(slice->Seqregion);

    return mpoint;
}




/* @func ensSliceCalculateLength **********************************************
**
** Calculate the length of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::length
** @cc Bio::EnsEMBL::CircularSlice::length
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [ajuint] Ensembl Slice length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensSliceCalculateLength(EnsPSlice slice)
{
    return (slice) ?
        ensSliceCalculateRegion(slice, slice->Start, slice->End) : 0U;
}




/* @func ensSliceCalculateMemsize *********************************************
**
** Calculate the memory size in bytes of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensSliceCalculateMemsize(const EnsPSlice slice)
{
    size_t size = 0;

    if (!slice)
        return 0;

    size += sizeof (EnsOSlice);

    size += ensSeqregionCalculateMemsize(slice->Seqregion);

    if (slice->Sequence)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(slice->Sequence);
    }

    return size;
}




/* @func ensSliceCalculateRegion **********************************************
**
** Calculate the length of a region spanning a (circular) Ensembl Slice.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start
** @param [r] end [ajint] End
**
** @return [ajuint] Length or 0U
**
** @release 6.4.0
** @@
** Linear Slice:
**   length = start - end + 1
** Circular Slice (start > end): in two sections
**   length = (Sequence Region Length - start + 1) + (end - 1 + 1)
**   length =  Sequence Region Length - start + 1 + end
******************************************************************************/

ajuint ensSliceCalculateRegion(EnsPSlice slice, ajint start, ajint end)
{
    ajuint length = 0U;

    AjBool circular = AJFALSE;

    if (!slice)
        return 0U;

    if (start > end)
    {
        if (ensSliceIsCircular(slice, &circular))
        {
            if (circular == ajTrue)
                length = ensSliceGetSeqregionLength(slice) - start + 1U + end;
            else
                ajFatal("ensSliceCalculateRegion cannot calculate the length "
                        "of a region on a linear Ensembl Slice, where the "
                        "start coordinate (%d) is greater than the "
                        "end coordinate (%d).", start, end);
        }
        else
            ajFatal("ensSliceCalculateSpan cannot determine the "
                    "Ensembl Slice topology.");
    }
    else
        length = end - start + 1U;

    /*
    ** NOTE: This could be rewritten to:
    **
    ** length = end - start + 1U;
    **
    ** if ((start > end) && (circular == ajTrue))
    **     length += ensSliceGetSeqregionLength(slice);
    */

    return length;
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
**
** @nam3rule Fetch Fetch Ensembl Slice objects
** @nam4rule All             Fetch all objects
** @nam5rule Attributes      Fetch all Ensembl Attribute objects
** @nam5rule Repeatfeatures  Fetch all Repeat Feature objects
** @nam5rule Sequenceedits   Fetch all Sequence Edit objects
** @nam4rule Name            Fetch the name
** @nam4rule Sequence        Fetch the sequence
** @nam5rule All             Fetch the complete sequence
** @nam5rule Sub             Fetch a sub-sequence
** @nam6rule Seq             Fetch as AJAX Sequence object
** @nam6rule Str             Fetch as AJAX String object
** @nam4rule Sliceexpanded   Fetch an expanded Ensembl Slice
** @nam4rule Sliceinverted   Fetch an inverted Ensembl Slice
** @nam4rule Slicesub        Fetch a sub-Slice
**
** @argrule AllAttributes slice [EnsPSlice] Ensembl Slice
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllRepeatfeatures slice [EnsPSlice] Ensembl Slice
** @argrule AllRepeatfeatures anname [const AjPStr] Ensembl Analysis name
** @argrule AllRepeatfeatures rctype [const AjPStr]
** Ensembl Repeat Consensus type
** @argrule AllRepeatfeatures rcclass [const AjPStr]
** Ensembl Repeat Consensus class
** @argrule AllRepeatfeatures rcname [const AjPStr]
** Ensembl Repeat Consensus name
** @argrule AllRepeatfeatures rfs [AjPList]
** AJAX List of Ensembl Repeat Feature objects
** @argrule AllSequenceedits slice [EnsPSlice] Ensembl Slice
** @argrule AllSequenceedits ses [AjPList]
** AJAX List of Ensembl Sequence Edit objects
** @argrule Name slice [const EnsPSlice] Ensembl Slice
** @argrule Name Pname [AjPStr*] Name
** @argrule Sequence slice [EnsPSlice] Ensembl Slice
** @argrule Sub start [ajint] Start coordinate
** @argrule Sub end [ajint] End coordinate
** @argrule Sub strand [ajint] Strand information
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence address
** @argrule Str Psequence [AjPStr*] AJAX String address
** @argrule Sliceinverted slice [EnsPSlice] Ensembl Slice
** @argrule Sliceinverted Pslice [EnsPSlice*] Sliceinverted Ensembl Slice
** @argrule Sliceexpanded slice [EnsPSlice] Ensembl Slice
** @argrule Sliceexpanded five [ajint]
** Number of bases to expand the 5' region
** @argrule Sliceexpanded three [ajint]
** Number of bases to expand the 3' region
** @argrule Sliceexpanded force [AjBool] Force Slice contraction
** @argrule Sliceexpanded Pfshift [ajint*]
** Maximum possible 5' shift when "force" is set
** @argrule Sliceexpanded Ptshift [ajint*]
** Maximum possible 3' shift when "force" is set
** @argrule Sliceexpanded Pslice [EnsPSlice*] Ensembl Slice address
** @argrule Slicesub slice [EnsPSlice] Ensembl Slice
** @argrule Slicesub start [ajint] Start coordinate
** @argrule Slicesub end [ajint] End coordinate
** @argrule Slicesub strand [ajint] Strand information
** @argrule Slicesub Pslice [EnsPSlice*] Ensembl Slice address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceFetchAllAttributes *******************************************
**
** Fetch all Ensembl Attribute objects for an Ensembl Slice.
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::get_all_Attributes
** @cc Bio::EnsEMBL::CircularSlice::get_all_Attributes
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceFetchAllAttributes(EnsPSlice slice,
                                  const AjPStr code,
                                  AjPList attributes)
{
    if (!slice)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    if (!slice->Seqregion)
    {
        ajDebug("ensSliceFetchAllAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Slice without an "
                "Ensembl Sequence Region.\n");

        return ajFalse;
    }

    ensSeqregionFetchAllAttributes(slice->Seqregion, code, attributes);

    return ajTrue;
}




/* @func ensSliceFetchAllRepeatfeatures ***************************************
**
** Fetch all Ensembl Repeat Feature objects on an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Repeat Feture objects
** before deleting the AJAX List.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] rctype [const AjPStr] Ensembl Repeat Consensus type
** @param [r] rcclass [const AjPStr] Ensembl Repeat Consensus class
** @param [r] rcname [const AjPStr] Ensembl Repeat Consensus name
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceFetchAllRepeatfeatures(EnsPSlice slice,
                                      const AjPStr anname,
                                      const AjPStr rctype,
                                      const AjPStr rcclass,
                                      const AjPStr rcname,
                                      AjPList rfs)
{
    if (ajDebugTest("ensSliceFetchAllRepeatfeatures"))
        ajDebug("ensSliceFetchAllRepeatfeatures\n"
                "  slice %p\n"
                "  anname '%S'\n"
                "  rctype '%S'\n"
                "  rcclass '%S'\n"
                "  rcname '%S'\n"
                "  rfs %p\n",
                slice,
                anname,
                rctype,
                rcclass,
                rcname,
                rfs);

    if (!slice)
        return ajFalse;

    if (!slice->Adaptor)
    {
        ajDebug("ensSliceFetchAllRepeatfeatures cannot get Repeat Feature "
                "objects without a Slice Adaptor attached to the Slice.\n");

        return ajFalse;
    }

    return ensRepeatfeatureadaptorFetchAllbySlice(
        ensRegistryGetRepeatfeatureadaptor(
            ensSliceadaptorGetDatabaseadaptor(slice->Adaptor)),
        slice,
        anname,
        rctype,
        rcclass,
        rcname,
        rfs);
}




/* @func ensSliceFetchAllSequenceedits ****************************************
**
** Fetch all Ensembl Sequence Edit objects of an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Sequence Edit objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::SequenceAdaptor::_rna_edit
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] ses [AjPList] AJAX List of Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** Ensembl Sequence Edit objects are Ensembl Attribute objects, which codes
** have to be defined in the static const char *sliceKSequenceeditCode array.
******************************************************************************/

AjBool ensSliceFetchAllSequenceedits(EnsPSlice slice,
                                     AjPList ses)
{
    register ajuint i = 0U;

    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute at = NULL;

    EnsPSequenceedit se = NULL;

    if (!slice)
        return ajFalse;

    if (!ses)
        return ajFalse;

    code = ajStrNew();

    attributes = ajListNew();

    for (i = 0U; sliceKSequenceeditCode[i]; i++)
    {
        ajStrAssignC(&code, sliceKSequenceeditCode[i]);

        ensSliceFetchAllAttributes(slice, code, attributes);
    }

    while (ajListPop(attributes, (void **) &at))
    {
        se = ensSequenceeditNewAttribute(at);

        ajListPushAppend(ses, (void *) se);

        ensAttributeDel(&at);
    }

    ajListFree(&attributes);

    ajStrDel(&code);

    return ajTrue;
}




/* @func ensSliceFetchName ****************************************************
**
** Fetch the name of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::name
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [wP] Pname [AjPStr*] Name String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceFetchName(const EnsPSlice slice, AjPStr *Pname)
{
    EnsPCoordsystem cs = NULL;

    if (!slice)
        return ajFalse;

    if (!Pname)
        return ajFalse;

    cs = ensSeqregionGetCoordsystem(slice->Seqregion);

    if (*Pname)
        *Pname = ajFmtPrintS(
            Pname,
            "%S:%S:%S:%d:%d:%d",
            ensCoordsystemGetName(cs),
            ensCoordsystemGetVersion(cs),
            ensSeqregionGetName(slice->Seqregion),
            slice->Start,
            slice->End,
            slice->Strand);
    else
        *Pname = ajFmtStr(
            "%S:%S:%S:%d:%d:%d",
            ensCoordsystemGetName(cs),
            ensCoordsystemGetVersion(cs),
            ensSeqregionGetName(slice->Seqregion),
            slice->Start,
            slice->End,
            slice->Strand);

    return ajTrue;
}




/* @func ensSliceFetchSequenceAllSeq ******************************************
**
** Fetch the sequence of an Ensembl Slice as AJAX Sequence.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceFetchSequenceAllSeq(EnsPSlice slice, AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!slice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    ensSliceFetchName(slice, &name);
    ensSliceFetchSequenceAllStr(slice, &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
        *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensSliceFetchSequenceAllStr ******************************************
**
** Fetch the sequence of an Ensembl Slice as AJAX String.
**
** @cc Bio::EnsEMBL::Slice::seq
** @cc Bio::EnsEMBL::CircularSlice::seq
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPStr*] Sequence String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceFetchSequenceAllStr(EnsPSlice slice, AjPStr *Psequence)
{
    AjBool circular = AJFALSE;

    AjPStr sequence1 = NULL;
    AjPStr sequence2 = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSequenceadaptor sqa = NULL;

    EnsPSlice slice1 = NULL;
    EnsPSlice slice2 = NULL;

    if (!slice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(ensSliceCalculateLength(slice) + 1);

    /* Special case for "in-between" (insertion) coordinates. */

    if (slice->Start == (slice->End + 1))
        return ajTrue;

    if (slice->Sequence)
    {
        /*
        ** Since the Slice has sequence attached, check whether its
        ** Slice length matches its sequence length.
        */

        if (ajStrGetLen(slice->Sequence) != ensSliceCalculateLength(slice))
            ajFatal("ensSliceFetchSequenceAllStr got a Slice, "
                    "which sequence length %u does not match its length %u.\n",
                    ajStrGetLen(slice->Sequence),
                    ensSliceCalculateLength(slice));

        ajStrAssignS(Psequence, slice->Sequence);
    }
    else if (slice->Adaptor)
    {
        /*
        ** Since the Slice has a Slice Adaptor attached, it is possible to
        ** retrieve the sequence from the database.
        */

        dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

        sqa = ensRegistryGetSequenceadaptor(dba);

        ensSliceIsCircular(slice, &circular);

        if ((circular == ajTrue) && (slice->Start > slice->End))
        {
            slice1 = ensSliceNewCpy(slice);
            slice2 = ensSliceNewCpy(slice);

            slice1->End   = ensSeqregionGetLength(slice1->Seqregion);
            slice2->Start = 1;

            /*
            ** FIXME: The Perl API uses two linear Slice objects here
            ** and passes in undef for the end. The above copies may
            ** therefore not work.
            */

            /*
            ** It is sligtly more efficient, if undefined AJAX String objects
            ** are directly allocated by the following functions to their
            ** final size.
            */

            ensSequenceadaptorFetchSliceAllStr(sqa, slice1, &sequence1);
            ensSequenceadaptorFetchSliceAllStr(sqa, slice2, &sequence2);

            ajStrAssignS(Psequence, sequence1);
            ajStrAppendS(Psequence, sequence2);

            ajStrDel(&sequence1);
            ajStrDel(&sequence2);

            ensSliceDel(&slice1);
            ensSliceDel(&slice2);
        }
        else
            ensSequenceadaptorFetchSliceAllStr(sqa, slice, Psequence);
    }
    else
    {
        /*
        ** The Slice has no sequence and no Slice Adaptor attached,
        ** so just return Ns.
        */

        ajStrAppendCountK(Psequence, 'N', ensSliceCalculateLength(slice));
    }

    if (ajStrGetLen(*Psequence) != ensSliceCalculateLength(slice))
        ajWarn("ensSliceFetchSequenceAllStr got sequence of length %u "
               "for Ensembl Slice of length %u.\n",
               ajStrGetLen(*Psequence),
               ensSliceCalculateLength(slice));

    return ajTrue;
}




/* @func ensSliceFetchSequenceSubSeq ******************************************
**
** Fetch a sub-sequence of an Ensembl Slice as AJAX Sequence.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPSeq*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceFetchSequenceSubSeq(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPSeq *Psequence)
{
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!slice)
        return ajFalse;

    if (!strand)
        strand = 1;

    if (!Psequence)
        return ajFalse;

    /*
    ** Construct the Slice name, but convert relative Slice coordinates into
    ** absolute Sequence Region coordinates.
    */

    if (slice->Strand >= 0)
    {
        srstart = slice->Start + start - 1;
        srend   = slice->Start + end   - 1;
    }
    else
    {
        srstart = slice->End - end   + 1;
        srend   = slice->End - start + 1;
    }

    srstrand = slice->Strand * strand;

    name = ajFmtStr("%S:%S:%S:%d:%d:%d",
                    ensSliceGetCoordsystemName(slice),
                    ensSliceGetCoordsystemVersion(slice),
                    ensSliceGetSeqregionName(slice),
                    srstart,
                    srend,
                    srstrand);

    /*
    ** Fetch the Slice sequence.
    **
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    ensSliceFetchSequenceSubStr(slice, start, end, strand, &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
        *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensSliceFetchSequenceSubStr ******************************************
**
** Fetch a sub-sequence of an Ensembl Slice as AJAX String
** in releative coordinates.
**
** @cc Bio::EnsEMBL::Slice::subseq
** @cc Bio::EnsEMBL::CircularSlice::subseq
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceFetchSequenceSubStr(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPStr *Psequence)
{
    ajint region1 = 0;
    ajint region2 = 0;
    ajint slength = 0;

    ajuint rlength = 0U;

    AjBool circular = AJFALSE;

    AjPStr sequence1 = NULL;
    AjPStr sequence2 = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSequenceadaptor sqa = NULL;

    if (ajDebugTest("ensSliceFetchSequenceSubStr"))
        ajDebug("ensSliceFetchSequenceSubStr\n"
                "  slice %p\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  Psequence %p\n",
                slice,
                start,
                end,
                strand,
                Psequence);

    if (!slice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /* For "in-between" (insertion) coordinates return an empty string. */

    if (start == (end + 1))
        return ajTrue;

    ensSliceIsCircular(slice, &circular);

    if ((circular == ajFalse) && (start > (end + 1)))
    {
        ajDebug("ensSliceFetchSequenceSubStr requires for linear Slices that "
                "the start coordinate %d is less than the "
                "end coordinate %d + 1.\n",
                start, end);

        return ajFalse;
    }

    if (!strand)
        strand = 1;

    if ((circular == ajTrue) && (start > end))
    {
        region1 = ensSeqregionGetLength(slice->Seqregion) - start;

        region2 = end;

        rlength = region1 + region2 + 1;
    }
    else
        rlength = end - start + 1;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(rlength + 1);

    if (slice->Sequence)
    {
        /*
        ** Since the Slice has sequence attached, check whether
        ** Slice length and sequence length match.
        */

        if (ajStrGetLen(slice->Sequence) != ensSliceCalculateLength(slice))
            ajFatal("ensSliceFetchSequenceSubStr got a Slice, "
                    "which sequence length (%Lu) does not match its "
                    "length (%u).\n",
                    ajStrGetLen(slice->Sequence),
                    ensSliceCalculateLength(slice));
        /* FIXME: size_t can be shorter than ajulong */

        /* Relative Slice coordinates range from 1 to length. */

        /* Check for a gap at the beginning and pad it with Ns. */

        if (start < 1)
        {
            ajStrAppendCountK(Psequence, 'N', 1 - start);

            start = 1;
        }

        ajStrAppendSubS(Psequence,
                        slice->Sequence,
                        start - 1,
                        end - start + 1);

        /* Check that the Slice is within signed integer range. */

        if (ensSliceCalculateLength(slice) <= INT_MAX)
            slength = (ajint) ensSliceCalculateLength(slice);
        else
            ajFatal("ensSliceFetchSequenceSubStr got an "
                    "Ensembl Slice, which length (%u) exceeds the "
                    "maximum integer limit (%d).\n",
                    ensSliceCalculateLength(slice), INT_MAX);

        /* Check for a gap at the end and pad it again with Ns. */

        if (end > slength)
            ajStrAppendCountK(Psequence, 'N', (ajuint) (end - slength));

        if (strand < 0)
            ajSeqstrReverse(Psequence);
    }
    else if (slice->Adaptor)
    {
        /*
        ** Since the Slice has a Slice Adaptor attached, it is possible to
        ** retrieve the sequence from the database.
        */

        dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

        sqa = ensRegistryGetSequenceadaptor(dba);

        if ((circular == ajTrue) && (start > end))
        {
            sequence1 = ajStrNewRes(rlength);
            sequence2 = ajStrNewRes(rlength);

            ensSequenceadaptorFetchSliceSubStr(
                sqa,
                slice,
                start,
                ensSeqregionGetLength(slice->Seqregion),
                strand,
                &sequence1);

            ensSequenceadaptorFetchSliceSubStr(
                sqa,
                slice,
                1,
                end,
                strand,
                &sequence2);

            ajStrAppendS(Psequence, sequence1);
            ajStrAppendS(Psequence, sequence2);

            ajStrDel(&sequence1);
            ajStrDel(&sequence2);
        }
        else
            ensSequenceadaptorFetchSliceSubStr(sqa,
                                               slice,
                                               start,
                                               end,
                                               strand,
                                               Psequence);
    }
    else
    {
        /*
        ** The Slice has no sequence and no Slice Adaptor attached,
        ** so just return Ns.
        */

        ajStrAppendCountK(Psequence, 'N', rlength);
    }

    if (ajStrGetLen(*Psequence) != rlength)
        ajWarn("ensSliceFetchSequenceSubStr got sequence of length %Lu "
               "for region of length %u.\n",
               ajStrGetLen(*Psequence),
               rlength);
    /* FIXME: size_t can be shorter than ajulong */

    return ajTrue;
}




/* @func ensSliceFetchSliceexpanded *******************************************
**
** Fetch an expanded copy of an Ensembl Slice, which remains unchanged.
** The start and end are moved outwards from the centre of the Slice if
** positive values are provided and moved inwards if negative values are
** provided. A Slice may not be contracted below 1 base pair but may grow
** to be arbitrarily large.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::expand
** @cc Bio::EnsEMBL::CircularSlice::expand
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] five [ajint] Number of bases to expand the 5' region
**            Positive values expand the Slice, negative values contract it
** @param [r] three [ajint] Number of bases to expand the 3' region
**            Positive values expand the Slice, negative values contract it
** @param [r] force [AjBool] Force Slice contraction, even when shifts for
**                          "five" and "three" overlap. In that case "five" and
**                          "three" will be set to a maximum possible number
**                           resulting in a Slice, which would have only 2 base
**                           pairs.
** @param [w] Pfshift [ajint*] Maximum possible 5' shift when "force" is set
** @param [w] Ptshift [ajint*] Maximum possible 3' shift when "force" is set
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: Currently, this function fetches the expanded Slice via a
** Slice Adaptor, if one is set for this Slice. The Slice Adaptor registers
** Slice objects in the internal cache, which minimises memory requirements.
** However, registering many Slice objects, the LRU cache may drop important
** Slice objects.
******************************************************************************/

AjBool ensSliceFetchSliceexpanded(EnsPSlice slice,
                                  ajint five,
                                  ajint three,
                                  AjBool force,
                                  ajint *Pfshift,
                                  ajint *Ptshift,
                                  EnsPSlice *Pslice)
{
    ajint sshift = 0;
    ajint eshift = 0;

    ajint srstart = 0;
    ajint srend   = 0;

    AjBool circular = AJFALSE;

    if (ajDebugTest("ensSliceFetchSliceexpanded"))
        ajDebug("ensSliceFetchSliceexpanded\n"
                "  slice %p\n"
                "  five %d\n"
                "  three %d\n"
                "  force '%B'\n"
                "  Pfshift %p\n"
                "  Ptshift %p\n"
                "  Pslice %p\n",
                slice,
                five,
                three,
                force,
                Pfshift,
                Ptshift,
                Pslice);

    if (!slice)
        return ajFalse;

    if (!Pfshift)
        return ajFalse;

    if (!Ptshift)
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    if (slice->Sequence)
    {
        ajDebug("ensSliceFetchSliceexpanded cannot expand an Ensembl Slice "
                "with an attached sequence.\n");

        return ajFalse;
    }

    if (slice->Strand > 0)
    {
        sshift = five;
        eshift = three;
    }
    else
    {
        sshift = three;
        eshift = five;
    }

    srstart = slice->Start - sshift;
    srend   = slice->End   + eshift;

    ensSliceIsCircular(slice, &circular);

    if (
        (
            (srstart <= 0)
            ||
            (srstart > ensSliceGetSeqregionLength(slice))
            ||
            (srend <= 0)
            ||
            (srend > ensSliceGetSeqregionLength(slice))
         )
        &&
        (circular == ajTrue)
        )
    {
        if (srstart <= 0)
            srstart += ensSliceGetSeqregionLength(slice);

        if (srstart > ensSliceGetSeqregionLength(slice))
            srstart -= ensSliceGetSeqregionLength(slice);

        if (srend <= 0)
            srend += ensSliceGetSeqregionLength(slice);

        if (srend > ensSliceGetSeqregionLength(slice))
            srend -= ensSliceGetSeqregionLength(slice);
    }

    if ((srstart > srend)  && (circular == ajFalse))
    {
        if (force)
        {
            /* Apply the maximal possible shift, if force is set. */

            if (sshift < 0)
            {
                /*
                ** If we are contracting the Slice from the start,
                ** move the start just before the end.
                */

                srstart = srend - 1;

                sshift = slice->Start - srstart;
            }
            else
            {
                /*
                ** If the Slice still has a negative length,
                ** try to move the end.
                */

                if (eshift < 0)
                {
                    srend = srstart + 1;

                    eshift = srend - slice->End;
                }

                *Pfshift = (slice->Strand >= 0) ? eshift : sshift;
                *Ptshift = (slice->Strand >= 0) ? sshift : eshift;
            }
        }

        if (srstart > srend)
        {
            /* If the Slice still has a negative length, return NULL. */

            ajDebug("ensSliceFetchSliceexpanded requires the Slice "
                    "start %d to be less than the Slice end %d "
                    "coordinate.\n",
                    srstart, srend);

            return ajFalse;
        }
    }

    if (slice->Adaptor)
        ensSliceadaptorFetchBySeqregionIdentifier(
            slice->Adaptor,
            ensSeqregionGetIdentifier(slice->Seqregion),
            srstart,
            srend,
            slice->Strand,
            Pslice);
    else
        *Pslice = ensSliceNewIni(slice->Adaptor,
                                 slice->Seqregion,
                                 srstart,
                                 srend,
                                 slice->Strand);

    return ajTrue;
}




/* @func ensSliceFetchSliceinverted *******************************************
**
** Fetch an inverted Slice from an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::invert
** @cc Bio::EnsEMBL::CircularSlice::invert
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
** @see ensSliceadaptorFetchBySlice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: Currently, this function fetches the expanded Slice via a
** Slice Adaptor, if one is set for this Slice. The Slice Adaptor registers
** Slice objects in the internal cache, which minimises memory requirements.
** However, registering many Slice objects, the LRU cache may drop important
** Slice objects.
******************************************************************************/

AjBool ensSliceFetchSliceinverted(EnsPSlice slice, EnsPSlice *Pslice)
{
    AjPStr sequence = NULL;

    if (!slice)
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    /*
    ** Retrieve the inverted Slice via the Slice Adaptor if one is attached.
    ** This will automatically register the new Slice in the
    ** Slice Adaptor-internal cache and will also take care of
    ** an eventual Slice-internal sequence.
    ** Use relative Slice coordinates, which range from 1 to length.
    */

    if (slice->Adaptor)
    {
        ensSliceadaptorFetchBySlice(slice->Adaptor,
                                    slice,
                                    1,
                                    (ajint) ensSliceCalculateLength(slice),
                                    -1,
                                    Pslice);
        return ajTrue;
    }

    if (slice->Sequence)
    {
        /* Reverse (and complement) the sequence if one has been defined. */

        sequence = ajStrNewS(slice->Sequence);

        ajSeqstrReverse(&sequence);

        *Pslice = ensSliceNewSeq(slice->Adaptor,
                                 slice->Seqregion,
                                 slice->Start,
                                 slice->End,
                                 slice->Strand * -1,
                                 sequence);

        ajStrDel(&sequence);
    }
    else
        *Pslice = ensSliceNewIni(slice->Adaptor,
                                 slice->Seqregion,
                                 slice->Start,
                                 slice->End,
                                 slice->Strand * -1);

    return ajTrue;
}




/* @func ensSliceFetchSlicesub ************************************************
**
** Fetch a Sub-Slice from an Ensembl Slice.
**
** If a Slice is requested, which lies outside the boundaries of this Slice,
** this function will return NULL. This means that the behaviour will be
** consistent whether or not the Slice is attached to the database
** (i.e. if there is attached sequence to the Slice).
** Alternatively, the ensSliceFetchSliceexpanded or
** ensSliceAdaptorFetchByRegion functions could be used instead.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::sub_Slice
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
** @see ensSliceadaptorFetchBySlice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: Currently, this function fetches the expanded Slice via a
** Slice Adaptor, if one is set for this Slice. The Slice Adaptor registers
** Slice objects in the internal cache, which minimises memory requirements.
** However, registering many Slice objects, the LRU cache may drop important
** Slice objects.
******************************************************************************/

AjBool ensSliceFetchSlicesub(EnsPSlice slice,
                             ajint start,
                             ajint end,
                             ajint strand,
                             EnsPSlice *Pslice)
{
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    AjPStr sequence = NULL;

    if (!slice)
    {
        ajDebug("ensSliceGetSubSlice requires an Ensembl Slice.\n");

        return ajFalse;
    }

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    if ((start < 1) || (start > slice->End))
    {
        ajDebug("ensSliceGetSubSlice requires the start coordinate %d "
                "to be greater than or equal to 1 and less than or equal to "
                "the end coordinate %d of the Ensembl Slice.\n",
                start, slice->End);

        return ajFalse;
    }

    if ((end < start) || (end > slice->End))
    {
        ajDebug("ensSliceGetSubSlice requires the end coordinate %d "
                "to be greater than or equal to the start coordinate %d "
                "and to be less than or equal to the "
                "end coordinate %d of the Ensembl Slice.\n",
                end, start, slice->End);

        return ajFalse;
    }

    if (!strand)
        strand = 1;

    /*
    ** Retrieve the Sub-Slice via the Slice Adaptor if one is attached.
    ** This will automatically register the new Slice in the
    ** Slice Adaptor-internal cache and will also take care of
    ** an eventual Slice-internal sequence.
    */

    if (slice->Adaptor)
    {
        ensSliceadaptorFetchBySlice(slice->Adaptor,
                                    slice,
                                    start,
                                    end,
                                    strand,
                                    Pslice);

        return ajTrue;
    }

    /* Transform relative into absolute Slice coordinates. */

    if (slice->Strand >= 0)
    {
        srstart = slice->Start + start - 1;
        srend   = slice->Start + end   - 1;
    }
    else
    {
        srstart = slice->End - end   + 1;
        srend   = slice->End - start + 1;
    }

    srstrand = slice->Strand * strand;

    if (slice->Sequence)
    {
        sequence = ajStrNewRes(end - start + 1);

        ensSliceFetchSequenceSubStr(slice,
                                    start,
                                    end,
                                    strand,
                                    &sequence);

        *Pslice = ensSliceNewSeq(slice->Adaptor,
                                 slice->Seqregion,
                                 srstart,
                                 srend,
                                 srstrand,
                                 sequence);

        ajStrDel(&sequence);
    }
    else
        *Pslice = ensSliceNewIni(slice->Adaptor,
                                 slice->Seqregion,
                                 srstart,
                                 srend,
                                 srstrand);

    return ajTrue;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Slice objects.
**
** @fdata [EnsPSlice]
**
** @nam3rule Compare Functions for comparing Ensembl Slice objects
** @nam4rule Identifier Compare the SQL database-internal identifier
** @nam5rule Ascending  Sort in ascending order
** @nam5rule Descending Sort in descending order
**
** @argrule * slice1 [const EnsPSlice] Ensembl Slice
** @argrule * slice2 [const EnsPSlice] Ensembl Slice
**
** @valrule * [int] Integer less than, equal or greater than 0
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceCompareIdentifierAscending ***********************************
**
** Comparison function to sort Ensembl Slice objects by their
** Ensembl Sequence Region identifier in ascending order.
**
** Ensembl Slice objects without Ensembl Sequence Region objects sort towards
** the end of the AJAX List.
**
** @param [r] slice1 [const EnsPSlice] Ensembl Slice 1
** @param [r] slice2 [const EnsPSlice] Ensembl Slice 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.3.0
** @@
******************************************************************************/

int ensSliceCompareIdentifierAscending(const EnsPSlice slice1,
                                       const EnsPSlice slice2)
{
    int result = 0;

    ajuint srid1 = 0U;
    ajuint srid2 = 0U;

    /* Sort empty values towards the end of the AJAX List. */

    if (slice1 && (!slice2))
        return -1;

    if ((!slice1) && (!slice2))
        return 0;

    if ((!slice1) && slice2)
        return +1;

    srid1 = ensSliceGetSeqregionIdentifier(slice1);
    srid2 = ensSliceGetSeqregionIdentifier(slice2);

    if (srid1 < srid2)
        result = -1;

    if (srid1 > srid2)
        result = +1;

    return result;
}




/* @section comparison ********************************************************
**
** Functions for matching Ensembl Slice objects.
**
** @fdata [EnsPSlice]
**
** @nam3rule Match Functions for matching Ensembl Slice objects
** @nam3rule Similarity Functions for matching Ensembl Slice objects
**
** @argrule * slice1 [const EnsPSlice] Ensembl Slice
** @argrule * slice2 [const EnsPSlice] Ensembl Slice
**
** @valrule * [AjBool] ajTrue if the Slice objects match
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceMatch ********************************************************
**
** Test for matching two Ensembl Slice objects.
**
** @param [r] slice1 [const EnsPSlice] First Ensembl Slice
** @param [r] slice2 [const EnsPSlice] Second Ensembl Slice
**
** @return [AjBool] ajTrue if the Slice objects are equal
**
** @release 6.2.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, the Coordinate System, the Sequence Region, as well as the Slice
** start, end and strand members are compared. In case the Slice has a
** sequence String attached, it is compared as well.
******************************************************************************/

AjBool ensSliceMatch(const EnsPSlice slice1, const EnsPSlice slice2)
{
    if (ajDebugTest("ensSliceMatch"))
    {
        ajDebug("ensSliceMatch\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);

        ensSliceTrace(slice1, 1);
        ensSliceTrace(slice2, 1);
    }

    if (!slice1)
        return ajFalse;

    if (!slice2)
        return ajFalse;

    /* Try a direct pointer comparison first. */

    if (slice1 == slice2)
        return ajTrue;

    if (!ensSeqregionMatch(slice1->Seqregion, slice2->Seqregion))
        return ajFalse;

    if (slice1->Start != slice2->Start)
        return ajFalse;

    if (slice1->End != slice2->End)
        return ajFalse;

    if (slice1->Strand != slice2->Strand)
        return ajFalse;

    if ((slice1->Sequence || slice2->Sequence) &&
        (!ajStrMatchS(slice1->Sequence, slice2->Sequence)))
        return ajFalse;

    return ajTrue;
}




/* @func ensSliceSimilarity ***************************************************
**
** Test two Ensembl Slice objects for similarity.
**
** For similarity Ensembl Slice objects have to be defined on the same
** Ensembl Sequence Region, but can have different start end and strand
** coordinates. If a sequence has been set, it has to match perfectly.
**
** @param [r] slice1 [const EnsPSlice] First Ensembl Slice
** @param [r] slice2 [const EnsPSlice] Second Ensembl Slice
**
** @return [AjBool] ajTrue if the Slice objects are equal
**
** @release 6.4.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, the Ensembl Sequence Region objects are compared. In case one of the
** Ensembl Slice objects has a sequence attached, it is compared as well.
******************************************************************************/

AjBool ensSliceSimilarity(const EnsPSlice slice1, const EnsPSlice slice2)
{
    if (ajDebugTest("ensSliceSimilarity"))
    {
        ajDebug("ensSliceSimilarity\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);

        ensSliceTrace(slice1, 1);
        ensSliceTrace(slice2, 1);
    }

    if (!slice1)
        return ajFalse;

    if (!slice2)
        return ajFalse;

    /* Try a direct pointer comparison first. */

    if (slice1 == slice2)
        return ajTrue;

    if (!ensSeqregionMatch(slice1->Seqregion, slice2->Seqregion))
        return ajFalse;

    /* Sequence members are optional. */

    if (((slice1->Sequence != NULL) || (slice2->Sequence != NULL)) &&
        (ajStrMatchS(slice1->Sequence, slice2->Sequence) == ajFalse))
        return ajFalse;

    return ajTrue;
}




/* @section query *************************************************************
**
** Functions for querying the properties of an Ensembl Slice.
**
** @fdata [EnsPSlice]
**
** @nam3rule Is Check whether an Ensembl Slice represents a certain property
** @nam4rule Circular Check for a circular Ensembl Slice
** @nam4rule Nonreference Check for a non-reference Ensembl Slice
** @nam4rule Toplevel Check for a top-level Ensembl Slice
**
** @argrule * slice [EnsPSlice] Ensembl Slice
** @argrule * Presult [AjBool*] Boolean result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSliceIsCircular ***************************************************
**
** Check whether an Ensembl Slice is circular based on an Ensembl Sequence
** Region, which has an Ensembl Attribute of code "circular_seq" set.
**
** @cc Bio::EnsEMBL::Slice::is_circular
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] Presult [AjBool*] ajTrue if the Ensembl Sequence Region has an
**                              Ensembl Attribute of code "circular_seq" set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: In this implementation, the "circular" property relies on
** Ensembl Attribute objects are linked to Ensembl Sequence Region objects
** that are at the base of Ensembl Slice objects. Although Ensembl Sequence
** Region objects cache all Ensembl Attribute objects, one SQL query is still
** needed for each new Ensembl Sequence Region object.
** The Perl API caches the information for all Ensembl Sequence Region objects
** at once, but does not formalise the information within
** Ensembl Attribute objects.
******************************************************************************/

AjBool ensSliceIsCircular(EnsPSlice slice, AjBool *Presult)
{
    EnsESliceTopology sltp = ensESliceTopologyNULL;

    if (!slice)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    sltp = ensSliceLoadTopology(slice);

    if (sltp == ensESliceTopologyCircular)
        *Presult = ajTrue;
    else
        *Presult = ajFalse;

    return ajTrue;
}




/* @func ensSliceIsNonreference ***********************************************
**
** Check whether an Ensembl Slice is based on an Ensembl Sequence Region, which
** has an Ensembl Attribute of code "non_ref" set.
**
** @cc Bio::EnsEMBL::Slice::is_reference
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] Presult [AjBool*] ajTrue if the Ensembl Sequence Region has an
**                              Ensembl Attribute of code "non_ref" set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceIsNonreference(EnsPSlice slice, AjBool *Presult)
{
    if (!slice)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    if (!slice->Seqregion)
    {
        ajDebug("ensSliceIsNonreference requires the Ensembl Slice to have an "
                "Ensembl Sequence region attached.\n");

        return ajFalse;
    }

    return ensSeqregionIsNonreference(slice->Seqregion, Presult);
}




/* @func ensSliceIsToplevel ***************************************************
**
** Check whether an Ensembl Slice is based on an Ensembl Sequence Region, which
** has an Ensembl Attribute of code "toplevel" set.
**
** @cc Bio::EnsEMBL::Slice::is_toplevel
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] Presult [AjBool*] ajTrue if the Ensembl Sequence Region has an
**                              Ensembl Attribute of code "toplevel" set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceIsToplevel(EnsPSlice slice, AjBool *Presult)
{
    if (!slice)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    if (!slice->Seqregion)
    {
        ajDebug("ensSliceIsToplevel requires the Ensembl Slice to have an "
                "Ensembl Sequence region attached.\n");

        return ajFalse;
    }

    return ensSeqregionIsToplevel(slice->Seqregion, Presult);
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Slice objects.
**
** @fdata [EnsPSlice]
**
** @nam3rule Project Project an Ensembl Slice onto another
** Ensembl Coordinate System
** @nam3rule Projectslice Project an Ensembl Slice onto another
** Ensembl Slice
**
** @argrule Project slice [EnsPSlice] Ensembl Slice
** @argrule Project csname [const AjPStr] Ensembl Coordinate System name
** @argrule Project csversion [const AjPStr] Ensembl Coordinate System version
** @argrule Project pss [AjPList] AJAX List of
** Ensembl Projection Segment objects
** @argrule Projectslice srcslice [EnsPSlice] Source Ensembl Slice
** @argrule Projectslice trgslice [EnsPSlice] Target Ensembl Slice
** @argrule Projectslice pss [AjPList] AJAX List of
** Ensembl Projection Segment objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic sliceConstrain *************************************************
**
** Constrain an Ensembl Slice to its Ensembl Sequence Region member.
**
** The caller is responsible for deleting the Ensembl Projection Segment.
**
** @cc Bio::EnsEMBL::Slice::_constrain_to_region
** @cc Bio::EnsEMBL::CircularSlice::_constrain_to_region
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

static EnsPProjectionsegment sliceConstrain(EnsPSlice slice)
{
    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice nslice = NULL;

    if (!slice)
    {
        ajDebug("sliceConstrain requires an Ensembl Slice.\n");

        return NULL;
    }

    /* Return NULL, if this Slice does not overlap with its Sequence Region. */

    if ((slice->Start > ensSeqregionGetLength(slice->Seqregion)) ||
        (slice->End < 1))
        return NULL;

    /*
    ** If the Slice has negative coordinates or coordinates exceeding the
    ** length of the Sequence Region the Slice needs shrinking to the
    ** defined Sequence Region.
    */

    if (slice->Start < 1)
        five = slice->Start - 1;

    if (slice->End > ensSeqregionGetLength(slice->Seqregion))
        three = ensSeqregionGetLength(slice->Seqregion) - slice->End;

    if (five || three)
    {
        if (slice->Strand >= 0)
            ensSliceFetchSliceexpanded(slice,
                                       five,
                                       three,
                                       ajFalse,
                                       &fshift,
                                       &tshift,
                                       &nslice);
        else
            ensSliceFetchSliceexpanded(slice,
                                       three,
                                       five,
                                       ajFalse,
                                       &fshift,
                                       &tshift,
                                       &nslice);
    }
    else
        nslice = ensSliceNewRef(slice);

    ps = ensProjectionsegmentNewIni(1 - five,
                                    ensSliceCalculateLength(slice) + three,
                                    nslice);

    ensSliceDel(&nslice);

    return ps;
}




/* @funcstatic sliceProject ***************************************************
**
** Ensembl Slice project helper function.
**
** The caller is responsible for deleting the Ensembl Projection Segment
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::project
** @cc Bio::EnsEMBL::CircularSlice::project
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] trgcs [EnsPCoordsystem] Ensembl Coordinate System
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool sliceProject(EnsPSlice slice,
                           EnsPCoordsystem trgcs,
                           AjPList pss)
{
    ajint end    = 0;
    ajint start  = 1;
    ajint length = 0;

    AjBool debug    = AJFALSE;
    AjBool circular = AJFALSE;

    AjPList nrmpss = NULL;
    AjPList mrs    = NULL;

    EnsPAssemblymapper        am  = NULL;
    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystem srccs  = NULL;
    EnsPCoordsystem nrmcs  = NULL;
    EnsPCoordsystem  mrcs  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment nrmps = NULL;
    EnsPProjectionsegment    ps = NULL;

    EnsPMapperresult mr = NULL;

    const EnsPSlice nrmslice = NULL;
    EnsPSlice       newslice = NULL;

    /* Deliberately connect debug requests to ensSliceProject. */

    debug = ajDebugTest("ensSliceProject");

    if (debug)
    {
        ajDebug("sliceProject\n"
                "  slice %p\n"
                "  trgcs %p\n"
                "  pss %p\n",
                slice,
                trgcs,
                pss);

        ensSliceTrace(slice, 1);

        ensCoordsystemTrace(trgcs, 1);
    }

    if (!slice)
        return ajFalse;

    if (!trgcs)
        return ajFalse;

    if (!pss)
        return ajFalse;

    if (!slice->Adaptor)
    {
        ajDebug("sliceProject requires that an Ensembl Slice Adaptor has "
                "been set in the Ensembl Slice.\n");

        return ajFalse;
    }

    dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

    /*
    ** Get the source Coordinate System, which is the
    ** Ensembl Coordinate System member of the
    ** Ensembl Sequence Region member of this Ensembl Slice.
    */

    if (!slice->Seqregion)
    {
        ajDebug("sliceProject requires that an Ensembl Sequence Region "
                "has been set in the Ensembl Slice.\n");

        return ajFalse;
    }

    srccs = ensSeqregionGetCoordsystem(slice->Seqregion);

    if (!srccs)
    {
        ajDebug("sliceProject requires that an Ensembl Coordinate System "
                "member has been set in the Ensembl Sequence Region member "
                "of the Ensembl Slice.\n");

        return ajFalse;
    }

    /*
    ** No mapping is needed if the requested Ensembl Coordinate System is the
    ** one this Ensembl Slice is based upon, but we do need to check if some
    ** of the Slice is outside of defined regions.
    */

    if (ensCoordsystemMatch(srccs, trgcs))
    {
        ajListPushAppend(pss, (void *) sliceConstrain(slice));

        return ajTrue;
    }

    /*
    ** Decompose this Slice into its symlinked components, which allows
    ** handling of haplotypes (HAPs) and pseudo-autosomal region (PARs).
    */

    ama = ensRegistryGetAssemblymapperadaptor(dba);

    nrmpss = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(slice->Adaptor,
                                                slice,
                                                nrmpss);

    while (ajListPop(nrmpss, (void **) &nrmps))
    {
        nrmslice = ensProjectionsegmentGetTargetSlice(nrmps);

        nrmcs = ensSeqregionGetCoordsystem(nrmslice->Seqregion);

        ensAssemblymapperadaptorFetchByCoordsystems(ama, nrmcs, trgcs, &am);

        mrs = ajListNew();

        if (am)
            ensAssemblymapperMapSlice(am, nrmslice, ajFalse, mrs);
        else
        {
            if (debug)
                ajDebug("sliceProject could not fetch an "
                        "Ensembl Assembly Mapper between "
                        "Ensembl Coordinate System objects "
                        "'%S:%S' and '%S:%S', which implies an "
                        "Ensembl Mapper Result of type "
                        "ensEMapperresultTypeGap.\n",
                        ensCoordsystemGetName(nrmcs),
                        ensCoordsystemGetVersion(nrmcs),
                        ensCoordsystemGetName(trgcs),
                        ensCoordsystemGetVersion(trgcs));

            mr = ensMapperresultNewGap(nrmslice->Start, nrmslice->End, 0);

            ajListPushAppend(mrs, (void *) mr);
        }

        ensAssemblymapperDel(&am);

        /* Construct a projection from the mapping results and return it. */

        while (ajListPop(mrs, (void **) &mr))
        {
            switch (ensMapperresultGetType(mr))
            {
                case ensEMapperresultTypeCoordinate:

                    /*
                    ** Calculate the Ensembl Mapper Result length for circular
                    ** or linear Ensembl Slice objects.
                    */

                    if (ensMapperresultGetCoordinateStart(mr) >
                        ensMapperresultGetCoordinateEnd(mr))
                        length
                            = ensSliceGetSeqregionLength(nrmslice)
                            + ensMapperresultGetCoordinateStart(mr)
                            + ensMapperresultGetCoordinateEnd(mr)
                            + 1;
                    else
                        length
                            = ensMapperresultGetCoordinateEnd(mr)
                            - ensMapperresultGetCoordinateStart(mr)
                            + 1;

                    mrcs = ensMapperresultGetCoordsystem(mr);

                    /*
                    ** If the normalised projection just ended up mapping to
                    ** the same Coordinate System we were already in then we
                    ** should just return the original region. This can happen
                    ** for example, if we were on a PAR region on Y, which
                    ** referred to X and a projection to "toplevel" was
                    ** requested.
                    */

                    if (ensCoordsystemMatch(mrcs, nrmcs))
                    {
                        /* Trim off regions, which are not defined. */

                        ajListPushAppend(pss, (void *) sliceConstrain(slice));

                        /*
                        ** Delete this Ensembl Mapper Result and the rest of
                        ** the Ensembl Mapper Results including the AJAX List.
                        */

                        ensMapperresultDel(&mr);

                        while (ajListPop(mrs, (void **) &mr))
                            ensMapperresultDel(&mr);

                        ajListFree(&mrs);

                        /*
                        ** Delete this normalised Projection Segment and the
                        ** rest of the normalised Projection Segment objects
                        ** including the AJAX List.
                        */

                        ensProjectionsegmentDel(&nrmps);

                        while (ajListPop(nrmpss, (void **) &nrmps))
                            ensProjectionsegmentDel(&nrmps);

                        ajListFree(&nrmpss);

                        return ajTrue;
                    }
                    else
                    {
                        /* Create a Slice in the target Coordinate System. */

                        ensSliceadaptorFetchByMapperresult(slice->Adaptor,
                                                           mr,
                                                           &newslice);

                        end = start + length - 1;

                        ensSliceIsCircular(newslice, &circular);

                        if ((circular == ajTrue) &&
                            (end > ensSliceGetSeqregionLength(newslice)))
                            end -= ensSliceGetSeqregionLength(newslice);

                        ps = ensProjectionsegmentNewIni(start, end, newslice);

                        ajListPushAppend(pss, (void *) ps);

                        ensSliceDel(&newslice);
                    }

                    break;

                case ensEMapperresultTypeGap:

                    /*
                    ** Skip gaps, but calculate the Ensembl Mapper Result
                    ** length for circular or linear Ensembl Slice objects.
                    */

                    if (ensMapperresultGetGapStart(mr) >
                        ensMapperresultGetGapEnd(mr))
                        length
                            = ensSliceGetSeqregionLength(nrmslice)
                            + ensMapperresultGetGapStart(mr)
                            + ensMapperresultGetGapEnd(mr)
                            + 1;
                    else
                        length
                            = ensMapperresultGetGapEnd(mr)
                            - ensMapperresultGetGapStart(mr)
                            +1;

                    break;

                case ensEMapperresultTypeInDel:

                    /*
                    ** Calculate the Ensembl Mapper Result length for circular
                    ** or linear Ensembl Slice objects.
                    ** NOTE: The Ensembl Mapper Result Coordinate member is
                    ** most likely the one to be used here. The Perl API just
                    ** specifies code for Bio::EnsEMBL::Mapper::Coordinate
                    ** objects.
                    */

                    if (ensMapperresultGetCoordinateStart(mr) >
                        ensMapperresultGetCoordinateEnd(mr))
                        length
                            = ensSliceGetSeqregionLength(nrmslice)
                            + ensMapperresultGetCoordinateStart(mr)
                            + ensMapperresultGetCoordinateEnd(mr)
                            + 1;
                    else
                        length
                            = ensMapperresultGetCoordinateEnd(mr)
                            - ensMapperresultGetCoordinateStart(mr)
                            + 1;

                    ajWarn("sliceProject got an unexpected "
                           "Ensembl Mapper Result of type "
                           "ensEMapperresultTypeInDel.");

                    break;

                default:

                    ajWarn("sliceProject got an unexpected "
                           "Ensembl Mapper Result of type %d.",
                           ensMapperresultGetType(mr));
            }

            start += length;

            ensMapperresultDel(&mr);
        }

        ajListFree(&mrs);

        ensProjectionsegmentDel(&nrmps);
    }

    ajListFree(&nrmpss);

    return ajTrue;
}




/* @func ensSliceProject ******************************************************
**
** Project an Ensembl Slice onto another Ensembl Coordinate System.
**
** Projecting an Ensembl Slice onto an Ensembl Coordinate System that the
** Slice is assembled from is analogous to retrieving a tiling path.
** This method may also be used to project up-wards to a higher-level
** Ensembl Coordinate System.
**
** The caller is responsible for deleting the Ensembl Projection Segment
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::project
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceProject(EnsPSlice slice,
                       const AjPStr csname,
                       const AjPStr csversion,
                       AjPList pss)
{
    AjBool circular = AJFALSE;
    AjBool result   = AJFALSE;

    EnsPCoordsystem        cs  = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSlice slice1 = NULL;
    EnsPSlice slice2 = NULL;

    if (ajDebugTest("ensSliceProject"))
    {
        ajDebug("ensSliceProject\n"
                "  slice %p\n"
                "  csname '%S'\n"
                "  cvsversion '%S'\n"
                "  pss %p\n",
                slice,
                csname,
                csversion,
                pss);

        ensSliceTrace(slice, 1);
    }

    if (!slice)
    {
        ajDebug("ensSliceProject requires an Ensembl Slice.\n");

        return ajFalse;
    }

    if (!csname)
    {
        ajDebug("ensSliceProject requires an "
                "Ensembl Coordinate System name.\n");

        return ajFalse;
    }

    /*
    ** A Coordinate System version is not strictly required,
    ** since ensCoordsystemadaptorFetchByName does not require one.
    */

    if (!pss)
    {
        ajDebug("ensSliceProject requires an AJAX List of "
                "Ensembl Projection Segment objects.\n");

        return ajFalse;
    }

    if (!slice->Adaptor)
    {
        ajDebug("ensSliceProject requires that an Ensembl Slice Adaptor has "
                "been set in the Ensembl Slice.\n");

        return ajFalse;
    }

    if (!slice->Seqregion)
    {
        ajDebug("ensSliceProject requires that an Ensembl Sequence Region "
                "has been set in the Ensembl Slice.\n");

        return ajFalse;
    }

    if (!ensSliceIsCircular(slice, &circular))
        return ajFalse;

    /* Fetch the target Coordinate System. */

    dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);

    if (!cs)
    {
        ajDebug("ensSliceProject cannot project to an unknown "
                "Ensembl Coordinate System '%S:%S'.\n", csname, csversion);

        return ajFalse;
    }

    if ((circular == ajTrue) && (slice->Start > slice->End))
    {
        slice1 = ensSliceNewCpy(slice);
        slice2 = ensSliceNewCpy(slice);

        slice1->End   = ensSeqregionGetLength(slice1->Seqregion);
        slice2->Start = 1;

        if (!sliceProject(slice1, cs, pss))
            result = ajFalse;

        if (!sliceProject(slice2, cs, pss))
            result = ajFalse;

        ensSliceDel(&slice1);
        ensSliceDel(&slice2);
    }
    else
    {
        if (!sliceProject(slice, cs, pss))
            result = ajFalse;
    }

    ensCoordsystemDel(&cs);

    return result;
}




/* @func ensSliceProjectslice *************************************************
**
** Project an Ensembl Slice onto another Ensembl Slice.
**
** Needed for cases where multiple assembly mappings exist and a specific
** mapping is specified.
**
** The caller is responsible for deleting the Ensembl Projection Segment
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::project_to_slice
** @param [u] srcslice [EnsPSlice] Ensembl Slice
** @param [u] trgslice [EnsPSlice] Target Ensembl Slice
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceProjectslice(EnsPSlice srcslice,
                            EnsPSlice trgslice,
                            AjPList pss)
{
    ajint start  = 0;
    ajint length = 0;

    ajuint last = 0U;

    AjPList nrmpss = NULL;
    AjPList mrs    = NULL;

    EnsPAssemblymapper        am  = NULL;
    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem trgcs = NULL;
    EnsPCoordsystem nrmcs = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment nrmps = NULL;
    EnsPProjectionsegment    ps = NULL;

    EnsPMapperresult mr = NULL;

    const EnsPSlice nrmslice = NULL;
    EnsPSlice       newslice = NULL;

    if (!srcslice)
        return ajFalse;

    if (!trgslice)
        return ajFalse;

    dba = ensSliceadaptorGetDatabaseadaptor(srcslice->Adaptor);

    ama = ensRegistryGetAssemblymapperadaptor(dba);

    /*
    ** Get the source Ensembl Coordinate System, which is the
    ** Ensembl Coordinate System member of the
    ** Ensembl Sequence Region member of this Ensembl Slice.
    */

    srccs = ensSeqregionGetCoordsystem(srcslice->Seqregion);

    if (!srccs)
    {
        ajDebug("ensSliceProjectslice requires that an "
                "Ensembl Coordinate System member has been set in the "
                "Ensembl Sequence Region member of the source "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    /*
    ** Get the target Coordinate System, which is the
    ** Ensembl Coordinate System member of the
    ** Ensembl Sequence Region member of this Ensembl Slice.
    */

    trgcs = ensSeqregionGetCoordsystem(trgslice->Seqregion);

    if (!trgcs)
    {
        ajDebug("ensSliceProjectslice requires that an "
                "Ensembl Coordinate System member has been set in the "
                "Ensembl Sequence Region member of the target "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    /*
    ** Decompose this Slice into its symlinked components, which allows
    ** handling of haplotypes (HAPs) and pseudo-autosomal region (PARs).
    */

    nrmpss = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(srcslice->Adaptor,
                                                srcslice,
                                                nrmpss);

    while (ajListPop(nrmpss, (void **) &nrmps))
    {
        nrmslice = ensProjectionsegmentGetTargetSlice(nrmps);

        nrmcs = ensSeqregionGetCoordsystem(nrmslice->Seqregion);

        ensAssemblymapperadaptorFetchByCoordsystems(ama, nrmcs, trgcs, &am);

        mrs = ajListNew();

        if (am)
            ensAssemblymapperMapToSlice(am, nrmslice, trgslice, ajFalse, mrs);
        else
        {
            ajDebug("ensSliceProjectslice got no Ensembl Assemblymapper -> "
                    "Ensembl Mapper Result of type gap\n");

            mr = ensMapperresultNewGap(nrmslice->Start, nrmslice->End, 0);

            ajListPushAppend(mrs, (void *) mr);
        }

        ensAssemblymapperDel(&am);

        /* Construct a projection from the mapping results and return it. */

        while (ajListPop(mrs, (void **) &mr))
        {
            length = ensMapperresultCalculateLengthResult(mr);

            /*
            ** Reset the start coordinate, if a new target Sequence Region
            ** was encountered.
            */

            if (last != ensMapperresultGetRank(mr))
                start = 1;

            last = ensMapperresultGetRank(mr);

            /* Skip gaps. */

            if (ensMapperresultGetType(mr) == ensEMapperresultTypeCoordinate)
            {
                /* For multiple mappings only get the correct one. */

                if (ensMapperresultGetObjectidentifier(mr) ==
                    ensSliceGetSeqregionIdentifier(trgslice))
                {
                    /* Create a Slice in the target Coordinate System. */

                    ensSliceadaptorFetchByMapperresult(srcslice->Adaptor,
                                                       mr,
                                                       &newslice);

                    ps = ensProjectionsegmentNewIni(start,
                                                    start + length - 1,
                                                    newslice);

                    ajListPushAppend(pss, (void *) ps);

                    ensSliceDel(&newslice);
                }
            }

            start += length;

            ensMapperresultDel(&mr);
        }

        ajListFree(&mrs);

        ensProjectionsegmentDel(&nrmps);
    }

    ajListFree(&nrmpss);

    ensCoordsystemDel(&trgcs);

    /*
    ** Delete the Ensembl Assembly Mapper Adaptor cache,
    ** as the next mapping may include a different set.
    */

    ensAssemblymapperadaptorClear(ama);

    return ajTrue;
}




/* @datasection [EnsESliceType] Ensembl Slice Type ****************************
**
** @nam2rule Slice Functions for manipulating
** Ensembl Slice objects
** @nam3rule SliceType Functions for manipulating
** Ensembl Slice Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Slice Type enumeration.
**
** @fdata [EnsESliceType]
**
** @nam4rule From Ensembl Slice Type query
** @nam5rule Seqregion Ensembl Sequence Region
** @nam5rule Str  AJAX String object query
**
** @argrule Seqregion sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Str  type [const AjPStr] Type string
**
** @valrule * [EnsESliceType] Ensembl Slice Type enumeration or
**                            ensESliceTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceTypeFromSeqregion ********************************************
**
** Convert an Ensembl Sequence Region into an Ensembl Slice Type enumeration.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsESliceType] Ensembl Slice Type enumeration or
**                         ensESliceTypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsESliceType ensSliceTypeFromSeqregion(EnsPSeqregion sr)
{
    AjBool result = AJFALSE;

    if (!sr)
        return ensESliceTypeNULL;

    ensSeqregionIsLocusreferencegenomic(sr, &result);

    if (result == ajTrue)
        return ensESliceTypeLrg;
    else
        return ensESliceTypeLinear;
}




/* @func ensSliceTypeFromStr **************************************************
**
** Convert an AJAX String into an Ensembl Slice Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsESliceType] Ensembl Slice Type enumeration or
**                         ensESliceTypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsESliceType ensSliceTypeFromStr(const AjPStr type)
{
    register EnsESliceType i = ensESliceTypeNULL;

    EnsESliceType ste = ensESliceTypeNULL;

    for (i = ensESliceTypeNULL;
         sliceKType[i];
         i++)
        if (ajStrMatchC(type, sliceKType[i]))
            ste = i;

    if (!ste)
        ajDebug("ensSliceTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return ste;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Slice Type enumeration.
**
** @fdata [EnsESliceType]
**
** @nam4rule To   Return Ensembl Slice Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To ste [EnsESliceType] Ensembl Slice Type enumeration
**
** @valrule Char [const char*] Ensembl Slice Type C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensSliceTypeToChar ***************************************************
**
** Convert an Ensembl Slice Type enumeration into a C-type (char *) string.
**
** @param [u] ste [EnsESliceType] Ensembl Slice Type enumeration
**
** @return [const char*] Ensembl Slice Type C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensSliceTypeToChar(EnsESliceType ste)
{
    register EnsESliceType i = ensESliceTypeNULL;

    for (i = ensESliceTypeNULL;
         sliceKType[i] && (i < ste);
         i++);

    if (!sliceKType[i])
        ajDebug("ensSliceTypeToChar "
                "encountered an out of boundary error on "
                "Ensembl Slice Type "
                "enumeration %d.\n",
                ste);

    return sliceKType[i];
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Slice Functions for manipulating AJAX List objects of
** Ensembl Slice objects
** @nam4rule Remove Remove functions
** @nam5rule Duplications Remove duplications
** @nam4rule Sort Sort functions
** @nam5rule Identifier Sort by Ensembl Sequence Region identifier member
** @nam5rule Name Sort by Ensembl Sequence Region name member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * slices [AjPList] AJAX List of Ensembl Slice objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListSliceRemoveDuplications ***************************************
**
** Process an AJAX List of Ensembl Slice objects and remove duplicate regions.
**
** This function checks each Ensembl Slice object on the AJAX List for
** duplicate regions and if positive, replaces the Ensembl Slice object by one
** or more Ensembl Slice objects representing the unique sequence.
**
** Ensembl Slice objects of complete mammalian Y chromosomes contain duplictaed
** regions where the pseudo autosomal regions (PARs) on both chromosome ends
** represent unique sequence, while the bulk of the chromosome is copied
** (duplicated) from the X chromosome. Therefore, this function would remove an
** Ensembl Slice spanning the Y chromosome and replace it with two Ensembl
** Slice objects representing just the PAR regions.
**
** The caller is responsible for deleting the Ensembl Slice objects before
** deleting the AJAX List.
**
** @param [u] slices [AjPList] AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSliceRemoveDuplications(AjPList slices)
{
    AjIList iter = NULL;

    AjPList aes = NULL;
    AjPList pss = NULL;

    EnsPAssemblyexception        ae  = NULL;
    EnsPAssemblyexceptionadaptor aea = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice psslice = NULL;
    EnsPSlice srslice = NULL;

    if (!slices)
        return ajFalse;

    aes = ajListNew();
    pss = ajListNew();

    iter = ajListIterNew(slices);

    while (!ajListIterDone(iter))
    {
        srslice = (EnsPSlice) ajListIterGet(iter);

        dba = ensSliceadaptorGetDatabaseadaptor(srslice->Adaptor);

        aea = ensRegistryGetAssemblyexceptionadaptor(dba);

        ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion(
            aea,
            ensSliceGetSeqregionIdentifier(srslice),
            aes);

        if (ajListGetLength(aes))
        {
            /*
            ** This Ensembl Slice may have duplicate regions, so de-reference
            ** symlinked assembly regions and remove any Ensembl Slice objects,
            ** which have a symlink, because these are duplicates.
            ** Replace them with any symlinked Ensembl Slice objects based on
            ** the same Ensembl Sequence Region and Ensembl Coordinate System
            ** as the original Ensembl Slice.
            */

            ajListIterRemove(iter);

            ensSliceadaptorRetrieveNormalisedprojection(srslice->Adaptor,
                                                        srslice,
                                                        pss);

            while (ajListPop(pss, (void **) &ps))
            {
                psslice = ensProjectionsegmentGetTargetSlice(ps);

                if (ensSeqregionMatch(psslice->Seqregion,
                                      srslice->Seqregion))
                {
                    ajListIterInsert(iter, (void *) ensSliceNewRef(psslice));

                    /* Advance the AJAX List Iterator after the insert. */

                    (void) ajListIterGet(iter);
                }

                ensProjectionsegmentDel(&ps);
            }

            ensSliceDel(&srslice);
        }

        while (ajListPop(aes, (void **) &ae))
            ensAssemblyexceptionDel(&ae);
    }

    ajListIterDel(&iter);

    ajListFree(&aes);
    ajListFree(&pss);

    return ajTrue;
}




/* @funcstatic listSliceCompareIdentifierAscending ****************************
**
** AJAX List of Ensembl Slice objects comparison function to sort by
** Ensembl Sequence Region identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Slice address 1
** @param [r] item2 [const void*] Ensembl Slice address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listSliceCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    ajuint srid1 = 0U;
    ajuint srid2 = 0U;

    EnsPSlice slice1 = *(EnsOSlice *const *) item1;
    EnsPSlice slice2 = *(EnsOSlice *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSliceCompareIdentifierAscending"))
        ajDebug("listSliceCompareIdentifierAscending\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (slice1 && (!slice2))
        return -1;

    if ((!slice1) && (!slice2))
        return 0;

    if ((!slice1) && slice2)
        return +1;

    srid1 = ensSliceGetSeqregionIdentifier(slice1);
    srid2 = ensSliceGetSeqregionIdentifier(slice2);

    if (srid1 && (!srid2))
        return -1;

    if ((!srid1) && (!srid2))
        return 0;

    if ((!srid1) && srid2)
        return +1;

    if (srid1 < srid2)
        return -1;

    if (srid1 > srid2)
        return +1;

    return 0;
}




/* @func ensListSliceSortIdentifierAscending **********************************
**
** Sort an AJAX List of Ensembl Slice objects by their
** Ensembl Sequence Region identifier in ascending order.
**
** @param [u] slices [AjPList] AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSliceSortIdentifierAscending(AjPList slices)
{
    if (!slices)
        return ajFalse;

    ajListSort(slices, &listSliceCompareIdentifierAscending);

    return ajTrue;
}




/* @funcstatic listSliceCompareIdentifierDescending ***************************
**
** AJAX List of Ensembl Slice objects comparison function to sort by
** Ensembl Sequence Region identifier in descending order.
**
** @param [r] item1 [const void*] Ensembl Slice address 1
** @param [r] item2 [const void*] Ensembl Slice address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listSliceCompareIdentifierDescending(
    const void *item1,
    const void *item2)
{
    ajuint srid1 = 0U;
    ajuint srid2 = 0U;

    EnsPSlice slice1 = *(EnsOSlice *const *) item1;
    EnsPSlice slice2 = *(EnsOSlice *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSliceCompareIdentifierDescending"))
        ajDebug("listSliceCompareIdentifierDescending\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (slice1 && (!slice2))
        return -1;

    if ((!slice1) && (!slice2))
        return 0;

    if ((!slice1) && slice2)
        return +1;

    srid1 = ensSliceGetSeqregionIdentifier(slice1);
    srid2 = ensSliceGetSeqregionIdentifier(slice2);

    if (srid1 && (!srid2))
        return -1;

    if ((!srid1) && (!srid2))
        return 0;

    if ((!srid1) && srid2)
        return +1;

    if (srid1 > srid2)
        return -1;

    if (srid1 < srid2)
        return +1;

    return 0;
}




/* @func ensListSliceSortIdentifierDescending *********************************
**
** Sort an AJAX List of Ensembl Slice objects by their
** Ensembl Sequence Region identifier in descending order.
**
** @param [u] slices [AjPList] AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSliceSortIdentifierDescending(AjPList slices)
{
    if (!slices)
        return ajFalse;

    ajListSort(slices, &listSliceCompareIdentifierDescending);

    return ajTrue;
}




/* @funcstatic listSliceCompareNameAscending **********************************
**
** AJAX List of Ensembl Slice objects comparison function to sort by
** Ensembl Sequence Region name in ascending order.
**
** @param [r] item1 [const void*] Ensembl Slice address 1
** @param [r] item2 [const void*] Ensembl Slice address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listSliceCompareNameAscending(
    const void *item1,
    const void *item2)
{
    const AjPStr srname1 = NULL;
    const AjPStr srname2 = NULL;

    EnsPSlice slice1 = *(EnsOSlice *const *) item1;
    EnsPSlice slice2 = *(EnsOSlice *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSliceCompareNameAscending"))
        ajDebug("listSliceCompareNameAscending\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (slice1 && (!slice2))
        return -1;

    if ((!slice1) && (!slice2))
        return 0;

    if ((!slice1) && slice2)
        return +1;

    srname1 = ensSliceGetSeqregionName(slice1);
    srname2 = ensSliceGetSeqregionName(slice2);

    if (srname1 && (!srname2))
        return -1;

    if ((!srname1) && (!srname2))
        return 0;

    if ((!srname1) && srname2)
        return +1;

    return ajStrCmpS(srname1, srname2);
}




/* @func ensListSliceSortNameAscending ****************************************
**
** Sort an AJAX List of Ensembl Slice objects by their
** Ensembl Sequence Region name in ascending order.
**
** @param [u] slices [AjPList] AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSliceSortNameAscending(AjPList slices)
{
    if (!slices)
        return ajFalse;

    ajListSort(slices, &listSliceCompareNameAscending);

    return ajTrue;
}




/* @funcstatic listSliceCompareNameDescending *********************************
**
** AJAX List of Ensembl Slice objects comparison function to sort by
** Ensembl Sequence Region name in descending order.
**
** @param [r] item1 [const void*] Ensembl Slice address 1
** @param [r] item2 [const void*] Ensembl Slice address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listSliceCompareNameDescending(
    const void *item1,
    const void *item2)
{
    const AjPStr srname1 = NULL;
    const AjPStr srname2 = NULL;

    EnsPSlice slice1 = *(EnsOSlice *const *) item1;
    EnsPSlice slice2 = *(EnsOSlice *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSliceCompareNameDescending"))
        ajDebug("listSliceCompareNameDescending\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (slice1 && (!slice2))
        return -1;

    if ((!slice1) && (!slice2))
        return 0;

    if ((!slice1) && slice2)
        return +1;

    srname1 = ensSliceGetSeqregionName(slice1);
    srname2 = ensSliceGetSeqregionName(slice2);

    if (srname1 && (!srname2))
        return -1;

    if ((!srname1) && (!srname2))
        return 0;

    if ((!srname1) && srname2)
        return +1;

    return -1 * ajStrCmpS(srname1, srname2);
}




/* @func ensListSliceSortNameDescending ***************************************
**
** Sort an AJAX List of Ensembl Slice objects by their
** Ensembl Sequence Region name in descending order.
**
** @param [u] slices [AjPList] AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSliceSortNameDescending(AjPList slices)
{
    if (!slices)
        return ajFalse;

    ajListSort(slices, &listSliceCompareNameDescending);

    return ajTrue;
}




/* @datasection [EnsPSliceadaptor] Ensembl Slice Adaptor **********************
**
** @nam2rule Sliceadaptor Functions for manipulating
** Ensembl Slice Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SliceAdaptor
** @cc CVS Revision: 1.134
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic sliceadaptorCacheDelete ****************************************
**
** Wrapper function to delete an Ensembl Slice from an Ensembl Cache.
**
** @param [d] Pvalue [void**] Ensembl Slice address
** @see ensCacheNew
**
** @return [void]
**
** @release 6.3.0
** @@
** When deleting from the Cache, this function also removes and deletes the
** Ensembl Slice from the name cache, which is based on a
** conventional AJAX Table.
******************************************************************************/

static void sliceadaptorCacheDelete(void **Pvalue)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjPStr key = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPSlice oldslice = NULL;
    EnsPSlice newslice = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    /*
    ** Synchronise the deletion of this Sequence Region from the
    ** identifier cache, which is based on an Ensembl (LRU) Cache,
    ** with the name cache, based on a conventional AJAX Table,
    ** both in the Sequence Adaptor.
    */

    newslice = *((EnsPSlice *) Pvalue);

    if (newslice->Adaptor && newslice->Adaptor->CacheByName)
    {
        cs = ensSeqregionGetCoordsystem(newslice->Seqregion);

        start  = newslice->Start;
        end    = newslice->End;
        strand = newslice->Strand;

        if ((start == 1) &&
            (end == ensSeqregionGetLength(newslice->Seqregion)) &&
            (strand == 1))
        {
            start  = 0;
            end    = 0;
            strand = 0;
        }

        /* Remove from the name cache. */

        key = ajFmtStr("%S:%S:%S:%d:%d:%d",
                       ensCoordsystemGetName(cs),
                       ensCoordsystemGetVersion(cs),
                       ensSeqregionGetName(newslice->Seqregion),
                       start, end, strand);

        oldslice = (EnsPSlice) ajTableRemove(
            newslice->Adaptor->CacheByName,
            (const void *) key);

        ensSliceDel(&oldslice);

        ajStrDel(&key);
    }

    ensSliceDel((EnsPSlice *) Pvalue);

    return;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Slice Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Slice Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSliceadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPSliceadaptor] Ensembl Slice Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensSliceadaptorNew ***************************************************
**
** Default constructor for an Ensembl Slice Adaptor.
**
** Ensembl Object Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Object Adaptor connected to a particular database is
** sufficient to instantiate any number of Ensembl Objects from the database.
** Each Ensembl Object will have a weak reference to the Object Adaptor that
** instantiated it. Therefore, Ensembl Object Adaptors should not be
** instantiated directly, but rather obtained from the Ensembl Registry,
** which will in turn call this function if neccessary.
**
** @see ensRegistryGetDatabaseadaptor
** @see ensRegistryGetSliceadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSliceadaptor] Ensembl Slice Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSliceadaptor ensSliceadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return NULL;

    AJNEW0(sla);

    sla->Adaptor = dba;

    sla->CacheByIdentifier = ensCacheNew(
        ensECacheTypeAlphaNumeric,
        sliceadaptorKCacheMaxBytes,
        sliceadaptorKCacheMaxCount,
        sliceadaptorKCacheMaxSize,
        (void *(*)(void *)) &ensSliceNewRef,
        &sliceadaptorCacheDelete,
        (size_t (*)(const void *)) &ensSliceCalculateMemsize,
        (void *(*)(const void *key)) NULL,
        (AjBool (*)(const void *value)) NULL,
        ajFalse,
        "Slice");

    sla->CacheByName = ajTablestrNew(0U);

    ajTableSetDestroyvalue(sla->CacheByName, (void (*)(void **)) &ensSliceDel);

    return sla;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Slice Adaptor object.
**
** @fdata [EnsPSliceadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Slice Adaptor
**
** @argrule * Psla [EnsPSliceadaptor*] Ensembl Slice Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSliceadaptorDel ***************************************************
**
** Default destructor for an Ensembl Slice Adaptor.
**
** This function also clears the internal Sequence Region caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Psla [EnsPSliceadaptor*] Ensembl Slice Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensSliceadaptorDel(EnsPSliceadaptor *Psla)
{
    EnsPSliceadaptor pthis = NULL;

    if (!Psla)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensSliceadaptorDel"))
        ajDebug("ensSliceadaptorDel\n"
                "  *Psla %p\n",
                *Psla);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Psla))
        return;

    /*
    ** Clear the identifier cache, which is based on an Ensembl LRU Cache.
    ** Clearing the Ensembl LRU Cache automatically clears the name cache
    ** via sliceadaptorCacheDelete.
    */

    ensCacheDel(&pthis->CacheByIdentifier);

    ajTableDel(&pthis->CacheByName);

    ajMemFree((void **) Psla);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Slice Adaptor object.
**
** @fdata [EnsPSliceadaptor]
**
** @nam3rule Get Return Ensembl Slice Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * sla [EnsPSliceadaptor] Ensembl Slice Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSliceadaptorGetDatabaseadaptor ************************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Slice Adaptor.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSliceadaptorGetDatabaseadaptor(
    EnsPSliceadaptor sla)
{
    return (sla) ? sla->Adaptor : NULL;
}




/* @section member retrieval **************************************************
**
** Functions for manipulating an Ensembl Slice Adaptor cache.
**
** @fdata [EnsPSliceadaptor]
**
** @nam3rule Cache Manupulate an Ensembl Slice Adaptor cache
** @nam4rule Insert Insert an Ensembl Slice
**
** @argrule * sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @argrule Insert Pslice [EnsPSlice*] Ensembl Slice
** address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSliceadaptorCacheInsert *******************************************
**
** Insert an Ensembl Slice into the Slice Adaptor-internal cache.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceadaptorCacheInsert(EnsPSliceadaptor sla, EnsPSlice *Pslice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjPStr ikey = NULL;
    AjPStr nkey = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPSlice slice1 = NULL;
    EnsPSlice slice2 = NULL;

    if (!sla)
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    if (ajDebugTest("ensSliceadaptorCacheInsert"))
    {
        ajDebug("ensSliceadaptorCacheInsert\n"
                "  sla %p\n"
                "  *Pslice %p\n",
                sla,
                *Pslice);

        ensSliceTrace(*Pslice, 1);
    }

    if (!*Pslice)
        return ajFalse;

    cs = ensSeqregionGetCoordsystem((*Pslice)->Seqregion);

    start  = (*Pslice)->Start;
    end    = (*Pslice)->End;
    strand = (*Pslice)->Strand;

    /*
    ** For Ensembl Slice objects that cover an entire Ensembl Sequence Region,
    ** zero all coordinates.
    */

    if ((start == 1) &&
        (end == ensSeqregionGetLength((*Pslice)->Seqregion)) &&
        (strand == 1))
    {
        start  = 0;
        end    = 0;
        strand = 0;
    }

    /* Search the identifier cache. */

    ikey = ajFmtStr("%u:%d:%d:%d",
                    ensSeqregionGetIdentifier((*Pslice)->Seqregion),
                    start, end, strand);

    ensCacheFetch(sla->CacheByIdentifier,
                  (void *) ikey,
                  (void **) &slice1);

    /* Search the name cache. */

    nkey = ajFmtStr("%S:%S:%S:%d:%d:%d",
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs),
                    ensSeqregionGetName((*Pslice)->Seqregion),
                    start, end, strand);

    slice2 = (EnsPSlice) ajTableFetchmodS(sla->CacheByName, nkey);

    if ((!slice1) && (!slice2))
    {
        /*
        ** None of the caches returned an identical Ensembl Slice so add this
        ** one to both caches. The Ensembl LRU Cache automatically references
        ** the Ensembl Slice via the sliceadaptorCacheReference function,
        ** while the AJAX Table-based cache needs manual referencing.
        */

        ensCacheStore(sla->CacheByIdentifier,
                      (void *) ikey,
                      (void **) Pslice);

        ajTablePut(sla->CacheByName,
                   (void *) ajStrNewS(nkey),
                   (void *) ensSliceNewRef(*Pslice));
    }

    if (slice1 && slice2 && (slice1 == slice2))
    {
        /*
        ** Both caches returned the same Ensembl Slice so delete it and
        ** return a pointer to the one already in the cache.
        */

        ensSliceDel(Pslice);

        *Pslice = ensSliceNewRef(slice2);
    }

    if (slice1 && slice2 && (slice1 != slice2))
        ajDebug("ensSliceadaptorCacheInsert detected Slice objects in the "
                "identifier '%S' and name '%S' cache with "
                "different addresses (%p and %p).\n",
                ikey, nkey, slice1, slice2);

    if (slice1 && (!slice2))
        ajDebug("ensSliceadaptorCacheInsert detected a Slice in "
                "the identifier, but not in the name cache.\n");

    if ((!slice1) && slice2)
        ajDebug("ensSliceadaptorCacheInsert detected a Slice in "
                "the name, but not in the identifier cache.\n");

    ensSliceDel(&slice1);

    ajStrDel(&ikey);
    ajStrDel(&nkey);

    return ajTrue;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Slice objects from an
** Ensembl SQL database.
**
** @fdata [EnsPSliceadaptor]
**
** @nam3rule Fetch Fetch Ensembl Slice object(s)
** @nam4rule All   Fetch all Ensembl Slice objects
** @nam4rule Allby Fetch all Ensembl Slice objects matching a criterion
** @nam5rule Regionunique Fetch all Ensembl Slice objects representing the
** unique parts of an Ensembl Sequence Region
** @nam4rule By Fetch one Ensembl Slice object matching a criterion
** @nam5rule Feature Fetch by an Ensembl Feature
** @nam5rule Location Fetch by a top-level location
** @nam5rule Mapperresult Fetch by an Ensembl Mapper Result
** @nam5rule Name Fetch by a name
** @nam5rule Region Fetch by a region
** @nam5rule Seqregion Fetch by an Ensembl Sequence Region member
** @nam6rule Identifier Fetch by an SQL database-internal identifier
** @nam6rule Name Fetch by a name
** @nam5rule Slice Fetch by an Ensembl Slice
**
** @argrule * sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @argrule All csname [const AjPStr] Ensembl Coordinate System name
** @argrule All csversion [const AjPStr] Ensembl Coordinate System version
** @argrule All nonreference [AjBool]
** Include non-reference Ensembl Sequence Region objects
** @argrule All duplicates [AjBool]
** Include duplicate Ensembl Sequence Region objects
** @argrule All lrg [AjBool] Include Locus Reference Genomic
** @argrule All slices [AjPList] AJAX List of Ensembl Slice objects
** @argrule AllbyRegionunique csname [const AjPStr]
** Ensembl Coordinate System name
** @argrule AllbyRegionunique csversion [const AjPStr]
** Ensembl Coordinate System version
** @argrule AllbyRegionunique srname [const AjPStr]
** Ensembl Sequence Region name
** @argrule AllbyRegionunique srstart [ajint] Start coordinate
** @argrule AllbyRegionunique srend [ajint] End coordinate
** @argrule AllbyRegionunique srstrand [ajint] Strand information
** @argrule Allby slices [AjPList] AJAX List of Ensembl Slice objects
** @argrule ByLocation location [const AjPStr] Top-level location
** @argrule ByMapperresult mr [const EnsPMapperresult] Ensembl Mapper Result
** @argrule ByName name [const AjPStr] Ensembl Slice name
** @argrule ByRegion csname [const AjPStr] Ensembl Coordinate System name
** @argrule ByRegion csversion [const AjPStr] Ensembl Coordinate System version
** @argrule ByRegion srname [const AjPStr] Ensembl Sequence Region name
** @argrule ByRegion srstart [ajint] Start coordinate
** @argrule ByRegion srend [ajint] End coordinate
** @argrule ByRegion srstrand [ajint] Strand information
** @argrule BySeqregionIdentifier srid [ajuint]
** Ensembl Sequence Region identifier
** @argrule BySeqregionIdentifier srstart [ajint] Start coordinate
** @argrule BySeqregionIdentifier srend [ajint] End coordinate
** @argrule BySeqregionIdentifier srstrand [ajint] Strand information
** @argrule BySeqregionName csname [const AjPStr]
** Ensembl Coordinate System name
** @argrule BySeqregionName csversion [const AjPStr]
** Ensembl Coordinate System version
** @argrule BySeqregionName srname [const AjPStr] Ensembl Sequence Region name
** @argrule BySlice slice [EnsPSlice] Ensembl Slice
** @argrule BySlice start [ajint] Start coordinate
** @argrule BySlice end [ajint] End coordinate
** @argrule BySlice strand [ajint] Strand information
** @argrule ByFeature feature [const EnsPFeature] Ensembl Feature
** @argrule ByFeature flank [ajint] Flanking region in base pair coordinates
** @argrule By Pslice [EnsPSlice*] Ensembl Slice address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSliceadaptorFetchAll **********************************************
**
** Fetch all Ensembl Slice objects representing Ensembl Sequence Region objects
** of a given Ensembl Coordinate System.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [r] nonreference [AjBool]
** Include non-reference Ensembl Sequence Region objects
** @param [r] duplicates [AjBool]
** Include duplicate Ensembl Sequence Region objects
** @param [r] lrg [AjBool] Include Locus Reference Genomic
** @param [u] slices [AjPList] An AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
** The Coordinate System name may be the name of an actual Coordinate System
** or an alias like "seqlevel" or "toplevel".
** If the Coordinate System name provided is "toplevel", all non-redundant,
** top-level Slice objects are returned. The Coordinate System version will be
** ignored in that case.
** Non-reference Slice objects reflect alternative haplotype assemblies.
** Examples for human NCBI36 would be c5_H2, c6_COX, c6_QBL and c22_H2.
** If the duplicate option is not set and a pseudo-autosomal region (PAR) is
** at the beginning of the Sequence Region, then the resulting Slice will not
** start at position 1, so that coordinates retrieved from this Slice might
** not be what was expected.
******************************************************************************/

AjBool ensSliceadaptorFetchAll(EnsPSliceadaptor sla,
                               const AjPStr csname,
                               const AjPStr csversion,
                               AjBool nonreference,
                               AjBool duplicates,
                               AjBool lrg,
                               AjPList slices)
{
    AjBool debug  = AJFALSE;
    AjBool result = AJFALSE;

    AjPList srs = NULL;

    EnsPCoordsystem        cs  = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSeqregion        sr  = NULL;
    EnsPSeqregionadaptor sra = NULL;

    EnsPSlice slice = NULL;

    debug = ajDebugTest("ensSliceadaptorFetchAll");

    if (debug)
        ajDebug("ensSliceadaptorFetchAll\n"
                "  sla %p\n"
                "  csname '%S'\n"
                "  csversion '%S'\n"
                "  nonreference '%B'\n"
                "  duplicates '%B'\n"
                "  lrg '%B'\n"
                "  slices %p\n",
                sla,
                csname,
                csversion,
                nonreference,
                duplicates,
                lrg,
                slices);

    if (!sla)
        return ajFalse;

    if (!csname)
        return ajFalse;

    if (!slices)
        return ajFalse;

    dba = ensSliceadaptorGetDatabaseadaptor(sla);

    csa = ensRegistryGetCoordsystemadaptor(dba);
    sra = ensRegistryGetSeqregionadaptor(dba);

    ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);

    if (!cs)
    {
        ajWarn("ensSliceadaptorFetchAll could not retrieve Coordinate System "
               "for name '%S' and version '%S'.\n", csname, csversion);

        return ajTrue;
    }

    /* Fetch the Ensembl Sequence Region objects from the database. */

    srs = ajListNew();

    ensSeqregionadaptorFetchAllbyCoordsystem(sra, cs, srs);

    while (ajListPop(srs, (void **) &sr))
    {
        /* Check for non-reference Ensembl Sequence Region objects. */

        if (nonreference == ajFalse)
        {
            if (!ensSeqregionIsNonreference(sr, &result))
            {
                ajDebug("ensSliceadaptorFetchAll could not call "
                        "ensSeqregionIsNonreference successfully.\n");

                ensSeqregionTrace(sr, 1);

                ensSeqregionDel(&sr);

                continue;
            }

            if (result == ajTrue)
            {
                if (debug)
                {
                    ajDebug("ensSliceadaptorFetchAll removed non-reference "
                            "Ensembl Sequence Region %p.\n", sr);

                    ensSeqregionTrace(sr, 1);
                }

                ensSeqregionDel(&sr);

                continue;
            }
        }

        /*
        ** Check for Locus Reference Genomic (LRG)
        ** Ensembl Sequence Region objects.
        */

        if (lrg == ajFalse)
        {
            if (!ensSeqregionIsLocusreferencegenomic(sr, &result))
            {
                ajDebug("ensSliceadaptorFetchAll could not call "
                        "ensSeqregionIsLocusreferencegenomic successfully.\n");

                ensSeqregionTrace(sr, 1);

                ensSeqregionDel(&sr);

                continue;
            }

            if (result == ajTrue)
            {
                if (debug)
                {
                    ajDebug("ensSliceadaptorFetchAll removed LRG "
                            "Ensembl Sequence Region %p.\n", sr);

                    ensSeqregionTrace(sr, 1);
                }

                ensSeqregionDel(&sr);

                continue;
            }
        }

        /* Create a new Slice spanning the entire Sequence Region. */

        slice = ensSliceNewIni(sla, sr, 1, ensSeqregionGetLength(sr), 1);

        ajListPushAppend(slices, (void *) slice);

        ensSeqregionDel(&sr);
    }

    if (duplicates == ajFalse)
        ensListSliceRemoveDuplications(slices);

    ajListFree(&srs);

    ensCoordsystemDel(&cs);

    return ajTrue;
}




/* @func ensSliceadaptorFetchAllbyRegionunique ********************************
**
** Fetch Ensembl Slice objects representing the unique parts of an
** Ensembl Sequence Region. At a minimum the name of an
** Ensembl Sequence Region must be provided. If no Coordinate System name is
** provided then a Slice of the highest ranked Coordinate System with a
** matching Sequence Region name will be returned.
**
** The caller is responsible for deleting the Ensembl Slice objects before
** deleting the AJAX List.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [rN] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [r] srname [const AjPStr] Ensembl Sequence Region name
** @param [r] srstart [ajint] Start coordinate
** @param [r] srend [ajint] End coordinate
** @param [r] srstrand [ajint] Strand information
** @param [u] slices [AjPList] AJAX List of Ensembl Slice objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** NOTE: The Ensembl Core Perl API performs some fuzzy matching, if no exact
** match for the provided Sequence Region name can be found. This allows clones
** to be fetched even when their sequence version is not known.
** For example ensSliceadaptorFetchByRegion(..., "clone", "AC008066", ...)
** will retrieve the Sequence Region with name "AC008066.4".
**
** The fuzzy matching can be turned off by setting the $no_fuzz argument to a
** true value.
**
** This has not been implemented here.
******************************************************************************/

AjBool ensSliceadaptorFetchAllbyRegionunique(EnsPSliceadaptor sla,
                                             const AjPStr csname,
                                             const AjPStr csversion,
                                             const AjPStr srname,
                                             ajint srstart,
                                             ajint srend,
                                             ajint srstrand,
                                             AjPList slices)
{
    AjBool result = AJFALSE;

    EnsPSlice slice = NULL;

    if (!sla)
        return ajFalse;

    if (!(srname && ajStrGetLen(srname)))
        return ajFalse;

    if (!slices)
        return ajFalse;

    result = ensSliceadaptorFetchByRegion(
        sla,
        csname,
        csversion,
        srname,
        srstart,
        srend,
        srstrand,
        &slice);

    ajListPushAppend(slices, (void *) slice);

    ensListSliceRemoveDuplications(slices);

    return result;
}




/* @func ensSliceadaptorFetchByFeature ****************************************
**
** Fetch an Ensembl Slice around an Ensembl Feature.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [r] flank [ajint] Flanking region in base pair coordinates
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
** All this really does is return a resized version of the Slice that the
** Feature is already on. Note that Slice objects returned from this function
** are always on the forward strand of the Sequence Region regardless of the
** strandedness of the Feature passed in.
******************************************************************************/

AjBool ensSliceadaptorFetchByFeature(EnsPSliceadaptor sla,
                                     const EnsPFeature feature,
                                     ajint flank,
                                     EnsPSlice *Pslice)
{
    ajuint srid   = 0U;
    ajint srstart = 0;
    ajint srend   = 0;

    AjBool result = AJFALSE;

    EnsPSlice slice = NULL;

    if (ajDebugTest("ensSliceadaptorFetchByFeature"))
    {
        ajDebug("ensSliceadaptorFetchByFeature\n"
                "  sla %p\n"
                "  feature %p\n"
                "  flank %d\n"
                "  Pslice %p\n",
                sla,
                feature,
                flank,
                Pslice);

        ensFeatureTrace(feature, 1);
    }

    if (!sla)
    {
        ajDebug("ensSliceadaptorFetchByFeature requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if (!feature)
    {
        ajDebug("ensSliceadaptorFetchByFeature requires an "
                "Ensembl Feature.\n");

        return ajFalse;
    }

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    slice = ensFeatureGetSlice(feature);

    if (!slice)
    {
        ajDebug("ensSliceadaptorFetchByFeature requires an "
                "Ensembl Slice attached to the Ensembl Feature.\n");

        return ajFalse;
    }

    srid = ensSliceGetSeqregionIdentifier(slice);

    /* Convert the Feature Slice coordinates to Sequence Region coordinates. */

    if (slice->Strand >= 0)
    {
        srstart = slice->Start + ensFeatureGetStart(feature) - 1;
        srend   = slice->Start + ensFeatureGetEnd(feature)   - 1;
    }
    else
    {
        srstart = slice->End - ensFeatureGetEnd(feature)   + 1;
        srend   = slice->End - ensFeatureGetStart(feature) + 1;
    }

    result = ensSliceadaptorFetchBySeqregionIdentifier(
        sla,
        srid,
        srstart - flank,
        srend   + flank,
        1,
        Pslice);

    return result;
}




/* @func ensSliceadaptorFetchByLocation ***************************************
**
** Fetch an Ensembl Slice based on a top-level location.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] location [const AjPStr] Top-leve location
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchByLocation(EnsPSliceadaptor sla,
                                      const AjPStr location,
                                      EnsPSlice *Pslice)
{
    ajint srstart = 0;
    ajint srend   = 0;

    AjBool result = AJFALSE;

    AjPRegexp re = NULL;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr localstr  = NULL;
    AjPStr srname    = NULL;
    AjPStr number    = NULL;

    if (!sla)
        return ajFalse;

    if ((!location) && (!ajStrGetLen(location)))
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    localstr = ajStrNewS(location);

    /* Cleanup any nomenclature like 1_000 or 1 000 or 1,000 */

    ajStrRemoveSetC(&localstr, " _,");

    /* NOTE: This needs adjusting to more complex patterns. */

    re = ajRegCompC("^(\\w+):?(\\d+)?(?:-|[.]{2})?(\\d+)?$");

    if (ajRegExec(re, localstr))
    {
        srname = ajStrNew();

        ajRegSubI(re, 1, &srname);

        if (ajRegLenI(re, 2))
        {
            number = ajStrNew();
            ajRegSubI(re, 2, &number);
            ajStrToInt(number, &srstart);
            ajStrDel(&number);
        }

        if (ajRegLenI(re, 3))
        {
            number = ajStrNew();
            ajRegSubI(re, 3, &number);
            ajStrToInt(number, &srend);
            ajStrDel(&number);
        }

        csname    = ajStrNewC("toplevel");
        csversion = ajStrNew();

        result = ensSliceadaptorFetchByRegion(
            sla,
            csname,
            csversion,
            srname,
            srstart,
            srend,
            1,
            Pslice);

        ajStrDel(&csname);
        ajStrDel(&csversion);
        ajStrDel(&srname);
    }

    ajRegFree(&re);

    ajStrDel(&localstr);

    return result;
}




/* @func ensSliceadaptorFetchByMapperresult ***********************************
**
** Fetch an Ensembl Slice based on an Ensembl Mapper Result.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchByMapperresult(EnsPSliceadaptor sla,
                                          const EnsPMapperresult mr,
                                          EnsPSlice *Pslice)
{
    if (!sla)
        return ajFalse;

    if (!mr)
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    return ensSliceadaptorFetchBySeqregionIdentifier(
        sla,
        ensMapperresultGetObjectidentifier(mr),
        ensMapperresultGetCoordinateStart(mr),
        ensMapperresultGetCoordinateEnd(mr),
        ensMapperresultGetCoordinateStrand(mr),
        Pslice);
}




/* @func ensSliceadaptorFetchByName *******************************************
**
** Fetch an Ensembl Slice by name, which consists of the following
** colon-separated fields:
**   Coordinate System name,
**   Coordinate System version,
**   Sequence Region start,
**   Sequence Region end and
**   Sequence Region strand
**
** csname:csversion:srname:srstart:srend:srstrand
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] name [const AjPStr] Ensembl Slice name
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchByName(EnsPSliceadaptor sla,
                                  const AjPStr name,
                                  EnsPSlice *Pslice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjBool debug  = AJFALSE;
    AjBool error  = AJFALSE;
    AjBool result = AJFALSE;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr srstart   = NULL;
    AjPStr srend     = NULL;
    AjPStr srstrand  = NULL;

    AjPStrTok token = NULL;

    debug = ajDebugTest("ensSliceadaptorFetchByName");

    if (debug)
        ajDebug("ensSliceadaptorFetchByName\n"
                "  sla %p\n"
                "  name '%S'\n"
                "  Pslice %p\n",
                sla,
                name,
                Pslice);

    if (!sla)
        return ajFalse;

    if (!(name && ajStrGetLen(name)))
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    /* Check the Slice Adaptor-internal cache for the name, first. */

    *Pslice = (EnsPSlice) ajTableFetchmodS(sla->CacheByName, name);

    if (*Pslice)
    {
        /*
        ** For any object returned by the AJAX Table the reference counter
        ** needs to be incremented manually.
        */

        ensSliceNewRef(*Pslice);

        return ajTrue;
    }

    token = ajStrTokenNewC(name, ":");

    csname    = ajStrNew();
    csversion = ajStrNew();
    srname    = ajStrNew();
    srstart   = ajStrNew();
    srend     = ajStrNew();
    srstrand  = ajStrNew();

    if (ajStrTokenNextParseNoskip(token, &csname) &&
        ajStrTokenNextParseNoskip(token, &csversion) &&
        ajStrTokenNextParseNoskip(token, &srname) &&
        ajStrTokenNextParseNoskip(token, &srstart) &&
        ajStrTokenNextParseNoskip(token, &srend) &&
        ajStrTokenNextParseNoskip(token, &srstrand))
    {
        /*
        ** Convert AJAX String objects into AJAX integers.
        ** Empty AJAX String objects are not converted and default to 0, to
        ** discriminate them from out of range and other errors reported by
        ** ajStrToInt.
        */

        if (ajStrGetLen(srstart))
            if (!ajStrToInt(srstart, &start))
            {
                ajWarn("ensSliceadaptorFetchByName could not convert '%S' "
                       "into an integer.",
                       srstart);

                error = ajTrue;
            }

        if (ajStrGetLen(srend))
            if (!ajStrToInt(srend, &end))
            {
                ajWarn("ensSliceadaptorFetchByName could not convert '%S' "
                       "into an integer.",
                       srend);

                error = ajTrue;
            }

        if (ajStrGetLen(srstrand))
            if (!ajStrToInt(srstrand, &strand))
            {
                ajWarn("ensSliceadaptorFetchByName could not convert '%S' "
                       "into an integer.",
                       srstrand);

                error = ajTrue;
            }

        if (debug)
            ajDebug("ensSliceadaptorFetchByName parsed name '%S' into:\n"
                    "  csname    '%S'\n"
                    "  csversion '%S'\n"
                    "  srname    '%S'\n"
                    "  srstart   '%S' %d\n"
                    "  srend     '%S' %d\n"
                    "  srstrand  '%S' %d\n",
                    name,
                    csname,
                    csversion,
                    srname,
                    srstart,  start,
                    srend,    end,
                    srstrand, strand);

        if (error == ajFalse)
        {
            if (start == 0 && end == 0)
                result = ensSliceadaptorFetchBySeqregionName(
                    sla,
                    csname,
                    csversion,
                    srname,
                    Pslice);
            else
                result = ensSliceadaptorFetchByRegion(
                    sla,
                    csname,
                    csversion,
                    srname,
                    start,
                    end,
                    strand,
                    Pslice);
        }
    }
    else
        ajDebug("ensSliceadaptorFetchByName got a malformed Ensembl Slice "
                "name '%S', should be something like "
                "chromosome:NCBI36:X:1000000:2000000:1\n"
                "  csname    '%S'\n"
                "  csversion '%S'\n"
                "  srname    '%S'\n"
                "  srstart   '%S'\n"
                "  srend     '%S'\n"
                "  srstrand  '%S'\n",
                name,
                csname,
                csversion,
                srname,
                srstart,
                srend,
                srstrand);

    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    ajStrDel(&srstart);
    ajStrDel(&srend);
    ajStrDel(&srstrand);

    ajStrTokenDel(&token);

    return result;
}




/* @func ensSliceadaptorFetchByRegion *****************************************
**
** Fetch an Ensembl Slice by region information. At a minimum the name of an
** Ensembl Sequence Region must be provided. If no Coordinate System name is
** provided then a Slice of the highest ranked Coordinate System with a
** matching Sequence Region name will be returned.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [rN] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [r] srname [const AjPStr] Ensembl Sequence Region name
** @param [r] srstart [ajint] Start coordinate
** @param [r] srend [ajint] End coordinate
** @param [r] srstrand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
**
** NOTE: The Ensembl Core Perl API performs some fuzzy matching, if no exact
** match for the provided Sequence Region name can be found. This allows clones
** to be fetched even when their sequence version is not known.
** For example ensSliceadaptorFetchByRegion(..., "clone", "AC008066", ...)
** will retrieve the Sequence Region with name "AC008066.4".
**
** The fuzzy matching can be turned off by setting the $no_fuzz argument to a
** true value.
**
** This has not been implemented here.
******************************************************************************/

AjBool ensSliceadaptorFetchByRegion(EnsPSliceadaptor sla,
                                    const AjPStr csname,
                                    const AjPStr csversion,
                                    const AjPStr srname,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    EnsPSlice *Pslice)
{
    AjPStr key = NULL;

    EnsPCoordsystem        cs  = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSeqregion        sr  = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if (ajDebugTest("ensSliceadaptorFetchByRegion"))
        ajDebug("ensSliceadaptorFetchByRegion\n"
                "  sla %p\n"
                "  csname '%S'\n"
                "  csversion '%S'\n"
                "  srname '%S'\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  Pslice %p\n",
                sla,
                csname,
                csversion,
                srname,
                srstart,
                srend,
                srstrand,
                Pslice);

    if (!sla)
    {
        ajDebug("ensSliceadaptorFetchByRegion requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if ((srname == NULL) || (ajStrGetLen(srname) == 0))
    {
        ajDebug("ensSliceadaptorFetchByRegion requires an "
                "Ensembl Sequence Region name.\n");

        return ajFalse;
    }

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    /*
    ** Initially, search the cache, which can only return an Ensembl Slice,
    ** which is associated with a name and version of a regular Ensembl
    ** Coordinate System. For requests specifying the top-level Coordinate
    ** System or no Coordinate System at all the database needs to be queried
    ** for the Sequence Region associated with the Coordinate System of the
    ** highest rank. However, all Slice objects will be inserted into the cache
    ** with their true Coordinate System, keeping at least the memory
    ** requirements minimal.
    */

    key = ajFmtStr("%S:%S:%S:%d:%d:%d",
                   csname,
                   csversion,
                   srname,
                   srstart,
                   srend,
                   srstrand);

    *Pslice = (EnsPSlice) ajTableFetchmodS(sla->CacheByName, key);

    ajStrDel(&key);

    if (*Pslice)
    {
        /*
        ** For any object returned by the AJAX Table the reference counter
        ** needs to be incremented manually.
        */

        ensSliceNewRef(*Pslice);

        return ajTrue;
    }

    dba = ensSliceadaptorGetDatabaseadaptor(sla);

    /* Load the Ensembl Coordinate System if a name has been provided. */

    if (csname && ajStrGetLen(csname))
    {
        csa = ensRegistryGetCoordsystemadaptor(dba);

        ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);

        if (!cs)
        {
            ajDebug("ensSliceadaptorFetchByRegion could not load an Ensembl "
                    "Coordinate System for name '%S' and version '%S'.\n",
                    csname, csversion);

            return ajTrue;
        }
    }

    /* Load the Ensembl Sequence Region by name. */

    sra = ensRegistryGetSeqregionadaptor(dba);

    ensSeqregionadaptorFetchByName(sra, cs, srname, &sr);

    if (!sr)
    {
        ajDebug("ensSliceadaptorFetchByRegion could not load an Ensembl "
                "Sequence Region for name '%S' and Coordinate System "
                "identifier %d.\n", srname, ensCoordsystemGetIdentifier(cs));

        ensCoordsystemTrace(cs, 1);

        /* NOTE: The Perl implementation also performs a fuzzy search. */

        ensSeqregionadaptorFetchBySynonym(sra, srname, &sr);
    }

    if (!sr)
    {
        ajDebug("ensSliceadaptorFetchByRegion could not load an Ensembl "
                "Sequence Region for synonym '%S'.\n",
                srname);
    }

    if (srstrand == 0)
        srstrand = 1;

    /*
    ** NOTE: The Perl API tests the Coordinate System name whether it matches
    ** a meta table entry with meta_key = 'LRG' to create a Bio::EnsEMBL::Slice
    ** or Bio::EnsEMBL::LRGSlice object. This implementation leaves this to the
    ** ensSliceNewIni function.
    */

    if (sr)
    {
        *Pslice = ensSliceNewIni(sla, sr, srstart, srend, srstrand);

        /*
        ** This implentation does not have a
        ** Bio::EnsEMBL::CircularSlice object.
        */

        if (srstart > (srend + 1))
            ensSliceSetTopology(*Pslice, ensESliceTopologyCircular);

        ensSliceadaptorCacheInsert(sla, Pslice);
    }

    ensCoordsystemDel(&cs);

    ensSeqregionDel(&sr);

    return ajTrue;
}




/* @func ensSliceadaptorFetchBySeqregionIdentifier ****************************
**
** Fetch an Ensembl Slice by an Ensembl Sequence Region identifier.
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] srid [ajuint] Ensembl Sequence Region identifier
** @param [r] srstart [ajint] Start coordinate
** @param [r] srend [ajint] End coordinate
** @param [r] srstrand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchBySeqregionIdentifier(EnsPSliceadaptor sla,
                                                 ajuint srid,
                                                 ajint srstart,
                                                 ajint srend,
                                                 ajint srstrand,
                                                 EnsPSlice *Pslice)
{
    AjPStr key = NULL;

    EnsPSeqregion sr = NULL;

    if (ajDebugTest("ensSliceadaptorFetchBySeqregionIdentifier"))
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier\n"
                "  sla %p\n"
                "  srid %u\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  Pslice %p\n",
                sla,
                srid,
                srstart,
                srend,
                srstrand,
                Pslice);

    if (!sla)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if (!srid)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier requires an "
                "Ensembl Sequence Region identifier.\n");

        return ajFalse;
    }

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    /* Check the cache first. */

    key = ajFmtStr("%u:%d:%d:%d", srid, srstart, srend, srstrand);

    ensCacheFetch(sla->CacheByIdentifier, (void *) key, (void **) Pslice);

    ajStrDel(&key);

    if (*Pslice)
        return ajTrue;

    ensSeqregionadaptorFetchByIdentifier(
        ensRegistryGetSeqregionadaptor(
            ensSliceadaptorGetDatabaseadaptor(sla)),
        srid,
        &sr);

    if (!sr)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier could not load an "
                "Ensembl Sequence Region for identifier %d.\n", srid);

        return ajTrue;
    }

    if (!srstart)
        srstart = 1;

    if (!srend)
        srend = ensSeqregionGetLength(sr);

    if (srstart > (srend + 1))
        ajFatal("ensSliceadaptorFetchBySeqregionIdentifier requires the Slice "
                "start %d to be less than or equal to the end %d + 1 for "
                "Ensembl Sequence Region identifier %d.",
                srstart, srend, srid);

    if (!srstrand)
        srstrand = 1;

    *Pslice = ensSliceNewIni(sla, sr, srstart, srend, srstrand);

    ensSliceadaptorCacheInsert(sla, Pslice);

    ensSeqregionDel(&sr);

    return ajTrue;
}




/* @func ensSliceadaptorFetchBySeqregionName **********************************
**
** Fetch an Ensembl Slice by an Ensembl Sequence Region name, which must be
** provided at a minimum. If no Coordinate System name has been specified,
** an Ensembl Slice of the highest ranked Ensembl Coordinate System with a
** matching Sequence Region name will be returned.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [rN] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [r] srname [const AjPStr] Ensembl Sequence Region name
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** NOTE: The Ensembl Core Perl API performs some fuzzy matching, if no exact
** match for the provided Sequence Region name can be found. This allows clones
** to be fetched even when their sequence version is not known.
** For example ensSliceadaptorFetchByRegion(..., "clone", "AC008066", ...)
** will retrieve the Sequence Region with name "AC008066.4".
**
** The fuzzy matching can be turned off by setting the $no_fuzz argument to a
** true value.
**
** This has not been implemented here.
******************************************************************************/

AjBool ensSliceadaptorFetchBySeqregionName(EnsPSliceadaptor sla,
                                           const AjPStr csname,
                                           const AjPStr csversion,
                                           const AjPStr srname,
                                           EnsPSlice *Pslice)
{
    AjPStr key = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSeqregion sr = NULL;

    if (ajDebugTest("ensSliceadaptorFetchBySeqregionName"))
        ajDebug("ensSliceadaptorFetchBySeqregionName\n"
                "  sla %p\n"
                "  csname '%S'\n"
                "  csversion '%S'\n"
                "  srname '%S'\n"
                "  Pslice %p\n",
                sla,
                csname,
                csversion,
                srname,
                Pslice);

    if (!sla)
        return ajFalse;

    if (!(srname && ajStrGetLen(srname)))
        return ajFalse;

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    /*
    ** Initially, search the cache, which can only return an Ensembl Slice,
    ** which is associated with a name and version of a regular Ensembl
    ** Coordinate System. For requests specifying the top-level Coordinate
    ** System or no Coordinate System at all the database needs to be queried
    ** for the Sequence Region associated with the Coordinate System of the
    ** highest rank. However, all Slice objects will be inserted into the cache
    ** with their true Coordinate System, keeping at least the memory
    ** requirements minimal.
    */

    key = ajFmtStr("%S:%S:%S:0:0:0",
                   csname,
                   csversion,
                   srname);

    *Pslice = (EnsPSlice) ajTableFetchmodS(sla->CacheByName, key);

    ajStrDel(&key);

    if (*Pslice)
    {
        /*
        ** For any object returned by the AJAX Table the reference counter
        ** needs to be incremented manually.
        */

        ensSliceNewRef(*Pslice);

        return ajTrue;
    }

    dba = ensSliceadaptorGetDatabaseadaptor(sla);

    /* Load the Ensembl Coordinate System if a name has been provided. */

    if (csname && ajStrGetLen(csname))
    {
        ensCoordsystemadaptorFetchByName(
            ensRegistryGetCoordsystemadaptor(dba),
            csname,
            csversion,
            &cs);

        if (!cs)
        {
            ajDebug("ensSliceadaptorFetchBySeqregionName could not load an "
                    "Ensembl Coordinate System for name '%S' and "
                    "version '%S'.\n",
                    csname, csversion);

            return ajTrue;
        }
    }

    /* Load the Ensembl Sequence Region. */

    ensSeqregionadaptorFetchByName(
        ensRegistryGetSeqregionadaptor(dba),
        cs,
        srname,
        &sr);

    if (!sr)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionName could not load an "
                "Ensembl Sequence Region for name '%S' and Coordinate System "
                "identifier %d.\n", srname, ensCoordsystemGetIdentifier(cs));

        ensCoordsystemTrace(cs, 1);

        ensCoordsystemDel(&cs);

        return ajTrue;
    }

    /*
    ** NOTE: The Perl API tests the Coordinate System name whether it matches
    ** a meta table entry with meta_key = 'LRG' to create a Bio::EnsEMBL::Slice
    ** or Bio::EnsEMBL::LRGSlice object. This implementation leaves this to the
    ** ensSliceNewIni function.
    */

    *Pslice = ensSliceNewIni(sla, sr, 1, ensSeqregionGetLength(sr), 1);

    ensSliceadaptorCacheInsert(sla, Pslice);

    ensCoordsystemDel(&cs);

    ensSeqregionDel(&sr);

    return ajTrue;
}




/* @func ensSliceadaptorFetchBySlice ******************************************
**
** Fetch an Ensembl Slice by an Ensembl Slice in relative coordinates.
**
** This function is experimental, but the advantage of going through the
** Slice Adaptor would be that a (Sub-)Slice would be registered in the
** Slice Adaptor-internal cache, which in turn reduces memory consumption.
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchBySlice(EnsPSliceadaptor sla,
                                   EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   EnsPSlice *Pslice)
{
    ajuint srid    = 0U;
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    if (!sla)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (start > (end + 1))
    {
        ajDebug("ensSliceadaptorFetchBySlice requires the start coordinate %d "
                "to be less than the end coordinate %d + 1.\n",
                start, end);

        return ajFalse;
    }

    if (!strand)
        strand = 1;

    if (!Pslice)
        return ajFalse;

    *Pslice = NULL;

    /* Transform relative into absolute Sequence Region coordinates. */

    /*
    **
    **           s           e
    **     SS     \    +1   /     SE
    **  1    \     |-------|     /    length
    **   \    |------Slice------|    /
    **    |--------Seqregion--------|
    **        |------Slice------|
    **       /     |-------|     \
    **     SS     /    -1   \     SE
    **           e           s
    **
    */

    srid = ensSeqregionGetIdentifier(slice->Seqregion);

    if (slice->Strand >= 0)
    {
        srstart = slice->Start + start - 1;
        srend   = slice->Start + end   - 1;
    }
    else
    {
        srstart = slice->End   - end   + 1;
        srend   = slice->End   - start + 1;
    }

    srstrand = slice->Strand * strand;

    ensSliceadaptorFetchBySeqregionIdentifier(sla,
                                              srid,
                                              srstart,
                                              srend,
                                              srstrand,
                                              Pslice);

    if (!*Pslice)
        return ajFalse;

    /* Adjust the sequence in case the Slice has one associated. */

    if (slice->Sequence)
        ensSliceFetchSequenceSubStr(slice,
                                    start,
                                    end,
                                    strand,
                                    &((*Pslice)->Sequence));

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Slice objects
** from an Ensembl SQL database.
**
** @fdata [EnsPSliceadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Slice-releated object(s)
** @nam4rule Normalisedprojection Retrieve all Ensembl Projection Segment
** objects for a normalised projection
**
** @argrule * sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @argrule Normalisedprojection slice [EnsPSlice] Ensembl Slice
** @argrule Normalisedprojection pss [AjPList] AJAX List of
** Ensembl Projection Segment objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSliceadaptorRetrieveNormalisedprojection **************************
**
** Retrieve all Ensembl Projection Segment objects required for a normalised
** projection of an Ensembl Slice.
**
** This function returns an AJAX List of Ensembl Projection Segment objects
** where symbolic links exist for the given Ensembl Slice.
**
** The caller is resposible for deleting the Ensembl Projection Segment objects
** before deleting the AJAX List.
**
** @param [u] sla [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSliceadaptorRetrieveNormalisedprojection(EnsPSliceadaptor sla,
                                                   EnsPSlice slice,
                                                   AjPList pss)
{
    ajint difference = 0;

    ajuint excsrid   = 0U;
    ajuint excstart  = 1U;
    ajuint excend    = 0U;
    ajuint exclength = 0U;

    ajuint refsrid   = 0U;
    ajuint refstart  = 1U;
    ajuint refend    = 0U;
    ajuint reflength = 0U;

    ajuint start     = 1U;
    /* ajuint srid      = 0U; */

    AjBool debug = AJFALSE;

    AjPList aes  = NULL;
    AjPList haps = NULL;
    AjPList pars = NULL;
    AjPList syms = NULL;
    AjPList mrs  = NULL;

    AjPStr source = NULL;
    AjPStr target = NULL;

    EnsPAssemblyexception        ae  = NULL;
    EnsPAssemblyexceptionadaptor aea = NULL;

    EnsPMapper mapper = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice excslice = NULL;
    EnsPSlice prjslice = NULL;
    EnsPSlice refslice = NULL;

    debug = ajDebugTest("ensSliceadaptorRetrieveNormalisedprojection");

    if (debug)
    {
        ajDebug("ensSliceadaptorRetrieveNormalisedprojection\n"
                "  sla %p\n"
                "  slice %p\n"
                "  pss %p\n",
                sla,
                slice,
                pss);

        ensSliceTrace(slice, 1);
    }

    if (!sla)
    {
        ajDebug("ensSliceadaptorRetrieveNormalisedprojection requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if (!slice)
    {
        ajDebug("ensSliceadaptorRetrieveNormalisedprojection requires an "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    if (!pss)
    {
        ajDebug("ensSliceadaptorRetrieveNormalisedprojection requires an "
                "AJAX List of Ensembl Projection Segment objects.\n");

        return ajFalse;
    }

    aea = ensRegistryGetAssemblyexceptionadaptor(
        ensSliceadaptorGetDatabaseadaptor(sla));

    refsrid = ensSliceGetSeqregionIdentifier(slice);

    haps = ajListNew();
    pars = ajListNew();
    aes  = ajListNew();

    ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion(aea, refsrid, aes);

    while (ajListPop(aes, (void **) &ae))
    {
        /*
        ** Select all overlapping pseudo-autosomal regions (PARs) and all
        ** haplotype regions (HAPs) if any.
        */

        switch (ensAssemblyexceptionGetType(ae))
        {
            case ensEAssemblyexceptionTypePAR:

                if (((ajint) ensAssemblyexceptionGetReferenceStart(ae)
                     <= slice->End) &&
                    ((ajint) ensAssemblyexceptionGetReferenceEnd(ae)
                     >= slice->Start))
                    ajListPushAppend(pars, (void *) ae);
                else
                    ensAssemblyexceptionDel(&ae);

                break;

            default:

                ajListPushAppend(haps, (void *) ae);
        }
    }

    ajListFree(&aes);

    if ((!ajListGetLength(haps)) && (!ajListGetLength(pars)))
    {
        /*
        ** There are no haplotypes and no pseudo-autosomal regions,
        ** so return just a Projection Segment for this Slice.
        */

        if (debug)
            ajDebug("ensSliceadaptorRetrieveNormalisedprojection "
                    "found no Ensembl Assembly Exception objects overlapping "
                    "this Ensembl Slice.\n");

        ajListFree(&haps);
        ajListFree(&pars);

        ajListPushAppend(
            pss,
            (void *) ensProjectionsegmentNewIni(
                1,
                ensSliceCalculateLength(slice),
                slice));

        return ajTrue;
    }

    syms = ajListNew();

    if (ajListGetLength(haps) > 0)
    {
        /*
        ** We want the region of the haplotype inverted, which means that we
        ** want the two regions of the Slice that are not covered by the
        ** haplotype as Projection Segment objects.
        **
        ** Haplotype:                          *******
        ** Reference:                    -------------------
        ** Projection Segment objects:   ++++++       ++++++
        */

        ensListAssemblyexceptionSortReferenceEndAscending(haps);

        ajListPeekFirst(haps, (void **) &ae);

        ensSliceadaptorFetchBySeqregionIdentifier(
            sla,
            ensAssemblyexceptionGetReferenceSeqregion(ae),
            0,
            0,
            0,
            &refslice);

        ensSliceadaptorFetchBySeqregionIdentifier(
            sla,
            ensAssemblyexceptionGetExceptionSeqregion(ae),
            0,
            0,
            0,
            &excslice);

        excsrid   = ensAssemblyexceptionGetExceptionSeqregion(ae);
        excstart  = 1;
        excend    = 0;
        exclength = ensSliceCalculateLength(excslice);

        refstart  = 1;
        refend    = 0;
        reflength = ensSliceCalculateLength(refslice);

        ensSliceDel(&refslice);
        ensSliceDel(&excslice);

        while (ajListPop(haps, (void **) &ae))
        {
            excend = ensAssemblyexceptionGetReferenceStart(ae) - 1;
            refend = ensAssemblyexceptionGetExceptionStart(ae) - 1;

            /*
            ** Add an Ensembl Assembly Exception for a haplotype (HAP) at
            ** the start or the end of the reference Slice.
            */

            if ((excend > 0) && (excstart < reflength))
                ajListPushAppend(
                    syms,
                    (void *) ensAssemblyexceptionNewIni(
                        (EnsPAssemblyexceptionadaptor) NULL,
                        0,
                        ensAssemblyexceptionGetReferenceSeqregion(ae),
                        excstart,
                        excend,
                        ensAssemblyexceptionGetExceptionSeqregion(ae),
                        refstart,
                        refend,
                        1,
                        ensEAssemblyexceptionTypeNULL));

            refstart
                = refend
                + ensAssemblyexceptionCalculateExceptionLength(ae)
                + 1;

            excstart
                = excend
                + ensAssemblyexceptionCalculateReferenceLength(ae)
                + 1;

            ensAssemblyexceptionDel(&ae);
        }

        /*
        ** For the last segment from the end of the
        ** Ensembl Assembly Exception to the end of the Ensembl Slice objects.
        */

        excend = reflength;
        refend = exclength;

        difference = (excend - excstart) - (refend - refstart);

        if (difference > 0)
            ajListPushAppend(
                syms,
                ensAssemblyexceptionNewIni(
                    (EnsPAssemblyexceptionadaptor) NULL,
                    0,
                    refsrid,
                    excstart,
                    excend,
                    excsrid,
                    refstart,
                    refend + difference,
                    1,
                    ensEAssemblyexceptionTypeNULL));
        else if (difference < 0)
            ajListPushAppend(
                syms,
                ensAssemblyexceptionNewIni(
                    (EnsPAssemblyexceptionadaptor) NULL,
                    0,
                    refsrid,
                    excstart,
                    excend - difference,
                    excsrid,
                    refstart,
                    refend,
                    1,
                    ensEAssemblyexceptionTypeNULL));
        else
            ajListPushAppend(
                syms,
                ensAssemblyexceptionNewIni(
                    (EnsPAssemblyexceptionadaptor) NULL,
                    0,
                    refsrid,
                    excstart,
                    excend,
                    excsrid,
                    refstart,
                    refend,
                    1,
                    ensEAssemblyexceptionTypeNULL));
    }

    ajListFree(&haps);

    /*
    ** The ajListPushlist function adds all nodes from the second AJAX List to
    ** the first and deletes the second.
    */

    ajListPushlist(syms, &pars);

    source = ajStrNewC("sym");
    target = ajStrNewC("org");

    mapper = ensMapperNewIni(source,
                             target,
                             ensSliceGetCoordsystemObject(slice),
                             ensSliceGetCoordsystemObject(slice));

    while (ajListPop(syms, (void **) &ae))
    {
        ensMapperAddCoordinates(mapper,
                                ensAssemblyexceptionGetReferenceSeqregion(ae),
                                ensAssemblyexceptionGetReferenceStart(ae),
                                ensAssemblyexceptionGetReferenceEnd(ae),
                                1,
                                ensAssemblyexceptionGetExceptionSeqregion(ae),
                                ensAssemblyexceptionGetExceptionStart(ae),
                                ensAssemblyexceptionGetExceptionEnd(ae));

        ensAssemblyexceptionDel(&ae);
    }

    ajListFree(&syms);

    mrs = ajListNew();

    ensMapperMap(mapper,
                 refsrid,
                 slice->Start,
                 slice->End,
                 slice->Strand,
                 source,
                 mrs);

    ajStrDel(&source);
    ajStrDel(&target);

    while (ajListPop(mrs, (void **) &mr))
    {
        switch (ensMapperresultGetType(mr))
        {
            case ensEMapperresultTypeCoordinate:

                if (debug)
                {
                    ajDebug("ensSliceadaptorRetrieveNormalisedprojection "
                            " got Ensembl Mapper Result %p.\n", mr);

                    ensMapperresultTrace(mr, 1);
                }

                ensSliceadaptorFetchByMapperresult(sla, mr, &prjslice);

                ajListPushAppend(
                    pss,
                    (void *) ensProjectionsegmentNewIni(
                        start,
                        start + ensMapperresultCalculateLengthResult(mr) - 1,
                        prjslice));

                ensSliceDel(&prjslice);

                break;

            case ensEMapperresultTypeGap:

                if (debug)
                    ajDebug("ensSliceadaptorRetrieveNormalisedprojection\n"
                            "  Ensembl Mapper Result gap %d:%d\n",
                            ensMapperresultGetGapStart(mr),
                            ensMapperresultGetGapEnd(mr));

                prjslice = ensSliceNewIni(sla,
                                          slice->Seqregion,
                                          ensMapperresultGetGapStart(mr),
                                          ensMapperresultGetGapEnd(mr),
                                          slice->Strand);

                ajListPushAppend(
                    pss,
                    (void *) ensProjectionsegmentNewIni(
                        start,
                        start + ensMapperresultCalculateLengthGap(mr) - 1,
                        prjslice));

                ensSliceDel(&prjslice);

                break;

            default:

                ajWarn("ensSliceadaptorRetrieveNormalisedprojection got "
                       "an unexpected Ensembl Mapper Result type %d.\n",
                       ensMapperresultGetType(mr));
        }

        start += ensMapperresultCalculateLengthResult(mr);

        ensMapperresultDel(&mr);
    }

    ajListFree(&mrs);

    ensMapperDel(&mapper);

    return ajTrue;
}




/* @datasection [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice ***********
**
** @nam2rule Repeatmaskedslice Functions for manipulating
** Ensembl Repeat-Masked Slice objects
**
** @cc Bio::EnsEMBL::RepeatMaskedSlice
** @cc CVS Revision: 1.21
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Repeat-Masked Slice by pointer.
** It is the responsibility of the user to first destroy any previous
** Repeat-Masked Slice. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPRepeatmaskedslice]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy rmslice [const EnsPRepeatmaskedslice] Ensembl Repeat-Masked
** Slice
** @argrule Ini slice [EnsPSlice] Ensembl Slice
** @argrule Ini annames [AjPList]
** AJAX List of AJAX String (Ensembl Analysis name) objects
** @argrule Ini masking [AjPTable]
** AJAX Table of AJAX String key data and
** Ensembl Repeat Mask Type enumeration value data
** to override masking types
** @argrule Ref rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @valrule * [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatmaskedsliceNewCpy *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] rmslice [const EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @return [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewCpy(
    const EnsPRepeatmaskedslice rmslice)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    EnsERepeatMaskType *Pmsktyp = NULL;

    AjIList iter = NULL;

    AjPStr key = NULL;

    EnsPRepeatmaskedslice pthis = NULL;

    if (!rmslice)
        return NULL;

    AJNEW0(pthis);

    pthis->Slice = ensSliceNewRef(rmslice->Slice);

    /* Copy the AJAX List of (Ensembl Analysis name) AJAX String objects. */

    if (rmslice->Analysisnames)
    {
        pthis->Analysisnames = ajListstrNew();

        iter = ajListIterNew(rmslice->Analysisnames);

        while (!ajListIterDone(iter))
        {
            key = (AjPStr) ajListIterGet(iter);

            if (key && ajStrGetLen(key))
                ajListPushAppend(pthis->Analysisnames,
                                 (void *) ajStrNewS(key));
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX Table of AJAX String key and AJAX Enum masking types. */

    if (rmslice->Masking)
    {
        pthis->Masking = ajTablestrNew(0U);

        ajTableSetDestroyvalue(pthis->Masking, &ajMemFree);

        ajTableToarrayKeysValues(rmslice->Masking, &keyarray, &valarray);

        for (i = 0U; keyarray[i]; i++)
        {
            key = ajStrNewS((AjPStr) keyarray[i]);

            AJNEW0(Pmsktyp);

            *Pmsktyp = *((EnsERepeatMaskType *) valarray[i]);

            ajTablePut(pthis->Masking, (void *) key, (void *) Pmsktyp);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    pthis->Use = 1U;

    return pthis;
}




/* @func ensRepeatmaskedsliceNewIni *******************************************
**
** Constructor for an Ensembl Repeat-Masked Slice with initial values.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [uN] annames [AjPList]
** AJAX List of AJAX String (Ensembl Analysis name) objects
** @param [uN] masking [AjPTable]
** AJAX Table of AJAX String key data and
** Ensembl Repeat Mask Type enumeration value data
** to override masking types
**
** @return [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewIni(EnsPSlice slice,
                                                 AjPList annames,
                                                 AjPTable masking)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    EnsERepeatMaskType *Pmsktyp = NULL;

    AjIList iter = NULL;

    AjPStr key = NULL;

    EnsPRepeatmaskedslice rmslice = NULL;

    if (ajDebugTest("ensRepeatmaskedsliceNewIni"))
    {
        ajDebug("ensRepeatmaskedsliceNewIni\n"
                "  slice %p\n"
                "  annames %p\n"
                "  masking %p\n",
                slice,
                annames,
                masking);

        ensSliceTrace(slice, 1);
    }

    if (!slice)
        return NULL;

    AJNEW0(rmslice);

    rmslice->Slice = ensSliceNewRef(slice);

    /* Copy the AJAX List of (Ensembl Analysis name) AJAX String objects. */

    if (annames)
    {
        rmslice->Analysisnames = ajListstrNew();

        iter = ajListIterNew(annames);

        while (!ajListIterDone(iter))
        {
            key = (AjPStr) ajListIterGet(iter);

            if (key && ajStrGetLen(key))
                ajListPushAppend(rmslice->Analysisnames,
                                 (void *) ajStrNewS(key));
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX Table of AJAX String key and AJAX Enum masking types. */

    if (masking)
    {
        rmslice->Masking = ajTablestrNew(0U);

        ajTableSetDestroyvalue(rmslice->Masking, &ajMemFree);

        ajTableToarrayKeysValues(masking, &keyarray, &valarray);

        for (i = 0U; keyarray[i]; i++)
        {
            key = ajStrNewS((AjPStr) keyarray[i]);

            AJNEW0(Pmsktyp);

            *Pmsktyp = *((EnsERepeatMaskType *) valarray[i]);

            ajTablePut(rmslice->Masking, (void *) key, (void *) Pmsktyp);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    rmslice->Use = 1U;

    return rmslice;
}




/* @func ensRepeatmaskedsliceNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @return [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewRef(
    EnsPRepeatmaskedslice rmslice)
{
    if (!rmslice)
        return NULL;

    rmslice->Use++;

    return rmslice;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Repeat-Masked Slice object.
**
** @fdata [EnsPRepeatmaskedslice]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat-Masked Slice
**
** @argrule * Prmslice [EnsPRepeatmaskedslice*]
** Ensembl Repeat-Masked Slice address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatmaskedsliceDel **********************************************
**
** Default destructor for an Ensembl Repeat-Masked Slice.
**
** @param [d] Prmslice [EnsPRepeatmaskedslice*]
** Ensembl Repeat-Masked Slice address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensRepeatmaskedsliceDel(EnsPRepeatmaskedslice *Prmslice)
{
    EnsPRepeatmaskedslice pthis = NULL;

    if (!Prmslice)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("ensRepeatmaskedsliceDel"))
    {
        ajDebug("ensRepeatmaskedsliceDel\n"
                "  *Prmslice %p\n",
                *Prmslice);

        ensRepeatmaskedsliceTrace(*Prmslice, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Prmslice) || --pthis->Use)
    {
        *Prmslice = NULL;

        return;
    }

    ensSliceDel(&pthis->Slice);

    /*
    ** Clear and delete the AJAX List of
    ** (Ensembl Analysis name) AJAX String objects.
    */

    ajListstrFreeData(&pthis->Analysisnames);

    ajTableDel(&pthis->Masking);

    ajMemFree((void **) Prmslice);

    return;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Repeat-Masked Slice object.
**
** @fdata [EnsPRepeatmaskedslice]
**
** @nam3rule Trace Report Ensembl Repeat-Masked Slice members to debug file
**
** @argrule Trace rmslice [const EnsPRepeatmaskedslice]
** Ensembl Repeat-Masked Slice
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensRepeatmaskedsliceTrace ********************************************
**
** Trace an Ensembl Repeat-Masked Slice.
**
** @param [r] rmslice [const EnsPRepeatmaskedslice]
** Ensembl Repeat-Masked Slice
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatmaskedsliceTrace(const EnsPRepeatmaskedslice rmslice,
                                 ajuint level)
{
    AjPStr indent = NULL;

    if (!rmslice)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensRepeatmaskedsliceTrace %p\n"
            "%S  Slice %p\n"
            "%S  Analysisnames %p\n"
            "%S  Masking %p\n"
            "%S  Use %u\n",
            indent, rmslice,
            indent, rmslice->Slice,
            indent, rmslice->Analysisnames,
            indent, rmslice->Masking,
            indent, rmslice->Use);

    ensSliceTrace(rmslice->Slice, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an Ensembl Repeat-Masked Slice object.
**
** @fdata [EnsPRepeatmaskedslice]
**
** @nam3rule Fetch Fetch Ensembl Repeat-Masked Slice objects
** @nam4rule All Fetch all objects
** @nam4rule Sequence Fetch the sequence
** @nam5rule Seq Fetch as AJAX Sequence object
** @nam5rule Str Fetch as AJAX String object
**
** @argrule Sequence rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked
** Slice
** @argrule Sequence mtype [EnsERepeatMaskType] Repeat masking type
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence address
** @argrule Str Psequence [AjPStr*] AJAX String address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensRepeatmaskedsliceFetchSequenceSeq *********************************
**
** Fetch the (masked) sequence of an Ensembl Repeat-Masked Slice as
** AJAX Sequence.
**
** @param [u] rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
** @param [u] mtype [EnsERepeatMaskType] Repeat masking type
** @param [wP] Psequence [AjPSeq*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatmaskedsliceFetchSequenceSeq(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!rmslice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    name = ajStrNew();

    ensSliceFetchName(rmslice->Slice, &name);

    sequence = ajStrNew();

    ensRepeatmaskedsliceFetchSequenceStr(rmslice, mtype, &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignSeqS(*Psequence, sequence);
        ajSeqAssignNameS(*Psequence, name);
    }
    else
        *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensRepeatmaskedsliceFetchSequenceStr *********************************
**
** Fetch the (masked) sequence for an Ensembl Repeat-Masked Slice as
** AJAX String.
**
** @cc Bio::EnsEMBL::RepeatMaskedSlice::seq
** @cc Bio::EnsEMBL::Slice::_mask_features
** @param [u] rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
** @param [u] mtype [EnsERepeatMaskType] Repeat masking type
** @param [wP] Psequence [AjPStr*] Sequence String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
** NOTE: The Bio::EnsEMBL::RepeatMaskedSlice::subseq method has not been
** implemented, because the Ensembl Base Adaptor has no global Ensembl Feature
** cache. Handling of polymorphic Bio::EnsEMBL::Feature subclasses is simple in
** object-oriented Perl, but quite complex in ANSI C. The Perl method
** retrieves Bio::EnsEMBL::RepeatFeature objects for larger blocks
** (2^18 or 256 Ki base pairs) for more efficient caching of subsequent
** requests.
** However, the performance hit should be minimal, as the relational database
** management system (RDBMS) caches the underlying queries. It is also possible
** to fetch the (masked) sequence for the whole Repeat-Masked Slice and
** subsequently extract the sub sequence with correspoinding AJAX functions.
** Therefore, the Bio::EnsEMBL::RepeatMaskedSlice::subseq method offers
** only small benefits over Bio::EnsEMBL::RepeatMaskedSlice::seq.
** As a consequence, the private Bio::EnsEMBL::Slice::_mask_features method
** has been merged into this function to simplify parameter checking if both,
** sequence and Repeat Feature objects are fetched from the same Slice.
******************************************************************************/

AjBool ensRepeatmaskedsliceFetchSequenceStr(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPStr *Psequence)
{
    ajint start = 0;
    ajint end   = 0;

    EnsERepeatMaskType   msktyp = ensERepeatMaskTypeNULL;
    EnsERepeatMaskType *Pmsktyp = NULL;

    AjIList iter = NULL;
    AjPList rfs  = NULL;

    AjPStr key = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPRepeatconsensus rc = NULL;

    EnsPRepeatfeature rf         = NULL;
    EnsPRepeatfeatureadaptor rfa = NULL;

    EnsPSliceadaptor sla = NULL;

    if (ajDebugTest("ensRepeatmaskedsliceFetchSequenceStr"))
        ajDebug("ensRepeatmaskedsliceFetchSequenceStr\n"
                "  rmslice %p\n"
                "  mtype %d\n"
                "  *Psequence %p\n",
                rmslice,
                mtype,
                *Psequence);

    if (!rmslice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    if ((mtype <= ensERepeatMaskTypeNULL) || (mtype > ensERepeatMaskTypeHard))
    {
        ajDebug("ensRepeatmaskedsliceFetchSequenceStr got unsupported "
                "masking type %d\n", mtype);

        return ajFalse;
    }

    if (!rmslice->Slice)
    {
        ajDebug("ensRepeatmaskedsliceFetchSequenceStr got an "
                "Ensembl Repeat-Masked Slice without an "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    ensSliceFetchSequenceAllStr(rmslice->Slice, Psequence);

    /* Sanity check */

    if (ensSliceCalculateLength(rmslice->Slice) != ajStrGetLen(*Psequence))
        ajWarn("ensRepeatmaskedsliceFetchSequenceStr got Slice of length %u, "
               "but DNA sequence of length %u.\n",
               ensSliceCalculateLength(rmslice->Slice),
               ajStrGetLen(*Psequence));

    sla = ensSliceGetAdaptor(rmslice->Slice);

    dba = ensSliceadaptorGetDatabaseadaptor(sla);

    rfa = ensRegistryGetRepeatfeatureadaptor(dba);

    rfs = ajListNew();

    if (rmslice->Analysisnames)
    {
        iter = ajListIterNew(rmslice->Analysisnames);

        while (!ajListIterDone(iter))
        {
            key = (AjPStr) ajListIterGet(iter);

            ensRepeatfeatureadaptorFetchAllbySlice(rfa,
                                                   rmslice->Slice,
                                                   key,
                                                   (AjPStr) NULL,
                                                   (AjPStr) NULL,
                                                   (AjPStr) NULL,
                                                   rfs);
        }

        ajListIterDel(&iter);
    }
    else
        ensRepeatfeatureadaptorFetchAllbySlice(rfa,
                                               rmslice->Slice,
                                               (AjPStr) NULL,
                                               (AjPStr) NULL,
                                               (AjPStr) NULL,
                                               (AjPStr) NULL,
                                               rfs);

    while (ajListPop(rfs, (void **) &rf))
    {
        /*
        ** NOTE: Since this function merges methods from the RepeatMaskedSlice
        ** and Slice classes the Repeat Feature objects are automatically
        ** fetched from the same Slice and do not need transforming back onto
        ** the Slice that has been passed into the second method. It is also
        ** not necessary to check whether the Feature objects overlap with the
        ** Slice.
        **
        ** For the AJAX String masking functions, the start and end coordinates
        ** of Repeat Feature objects partly outside the Slice need to be
        ** trimmed.
        */

        feature = ensRepeatfeatureGetFeature(rf);

        start = (ensFeatureGetStart(feature) >= 1) ?
            ensFeatureGetStart(feature) :
            1;

        end = (ensFeatureGetEnd(feature) <=
               (ajint) ensSliceCalculateLength(rmslice->Slice)) ?
            ensFeatureGetEnd(feature) :
            (ajint) ensSliceCalculateLength(rmslice->Slice);

        if (rmslice->Masking)
        {
            rc = ensRepeatfeatureGetRepeatconsensus(rf);

            /* Get the masking type for the Repeat Consensus type. */

            key = ajFmtStr("repeat_type_%S", ensRepeatconsensusGetType(rc));

            Pmsktyp = (EnsERepeatMaskType *) ajTableFetchmodV(
                rmslice->Masking,
                (const void *) key);

            if (Pmsktyp)
                msktyp = *Pmsktyp;

            ajStrDel(&key);

            /* Get the masking type for the Repeat Consensus class. */

            key = ajFmtStr("repeat_class_%S", ensRepeatconsensusGetClass(rc));

            Pmsktyp = (EnsERepeatMaskType *) ajTableFetchmodV(
                rmslice->Masking,
                (const void *) key);

            if (Pmsktyp)
                msktyp = *Pmsktyp;

            ajStrDel(&key);

            /* Get the masking type for the Repeat Consensus name. */

            key = ajFmtStr("repeat_name_%S", ensRepeatconsensusGetName(rc));

            Pmsktyp = (EnsERepeatMaskType *) ajTableFetchmodV(
                rmslice->Masking,
                (const void *) key);

            if (Pmsktyp)
                msktyp = *Pmsktyp;

            ajStrDel(&key);
        }

        /* Set the default masking type, if no other type has been defined. */

        if (!msktyp)
            msktyp = mtype;

        /* Mask the DNA sequence. */

        switch (msktyp)
        {
            case ensERepeatMaskTypeNULL:

                break;

            case ensERepeatMaskTypeSoft:

                ajStrFmtLowerSub(Psequence, start - 1, end - 1);

                break;

            case ensERepeatMaskTypeHard:

                ajStrMaskRange(Psequence, start - 1, end - 1, 'N');

                break;

            default:

                ajDebug("ensRepeatmaskedsliceFetchSequenceStr got unsupported "
                        "Ensembl Repeat Mask Type enumeration %d\n", msktyp);
        }

        ensRepeatfeatureDel(&rf);
    }

    ajListFree(&rfs);

    return ajTrue;
}

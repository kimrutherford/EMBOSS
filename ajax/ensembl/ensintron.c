/* @source ensintron **********************************************************
**
** Ensembl Intron functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.26 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#include "ensintron.h"
#include "ensexon.h"




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




/* @filesection ensintron *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library.
**
******************************************************************************/




/* @datasection [EnsPIntron] Ensembl Intron ***********************************
**
** @nam2rule Intron Functions for manipulating Ensembl Intron objects
**
** @cc Bio::EnsEMBL::Intron
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Intron by pointer.
** It is the responsibility of the user to first destroy any previous
** Intron. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPIntron]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the use counter
**
** @argrule Cpy intron [const EnsPIntron] Ensembl Intron
** @argrule Ini exon1 [EnsPExon] Ensembl Exon 1
** @argrule Ini exon2 [EnsPExon] Ensembl Exon 2
** @argrule Ref intron [EnsPIntron] Ensembl Intron
**
** @valrule * [EnsPIntron] Ensembl Intron or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensIntronNewCpy ******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPIntron ensIntronNewCpy(const EnsPIntron intron)
{
    EnsPIntron pthis = NULL;

    if (!intron)
        return NULL;

    AJNEW0(pthis);

    pthis->Feature      = ensFeatureNewRef(intron->Feature);
    pthis->PreviousExon = ensExonNewRef(intron->PreviousExon);
    pthis->NextExon     = ensExonNewRef(intron->NextExon);
    pthis->Use          = 1U;

    return pthis;
}




/* @func ensIntronNewIni ******************************************************
**
** Default constructor for an Ensembl Intron with initial values.
**
** @cc Bio::EnsEMBL::Intron::new
** @param [u] exon1 [EnsPExon] Ensembl Exon 1
** @param [u] exon2 [EnsPExon] Ensembl Exon 2
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPIntron ensIntronNewIni(EnsPExon exon1, EnsPExon exon2)
{
    ajint strand = 0;

    ajuint start = 0U;
    ajuint end   = 0U;

    AjPStr seqname1 = NULL;
    AjPStr seqname2 = NULL;

    EnsPFeature feature  = NULL;
    EnsPFeature feature1 = NULL;
    EnsPFeature feature2 = NULL;

    EnsPIntron intron = NULL;

    EnsPSlice slice1 = NULL;
    EnsPSlice slice2 = NULL;

    if (ajDebugTest("ensIntronNewExons"))
    {
        ajDebug("ensIntronNewExons\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);

        ensExonTrace(exon1, 1);
        ensExonTrace(exon2, 1);
    }

    if (!exon1)
        return NULL;

    if (!exon2)
        return NULL;

    feature1 = ensExonGetFeature(exon1);
    feature2 = ensExonGetFeature(exon2);

    slice1 = ensFeatureGetSlice(feature1);
    slice2 = ensFeatureGetSlice(feature2);

    seqname1 = ensFeatureGetSequencename(feature1);
    seqname2 = ensFeatureGetSequencename(feature2);

    /* Both Exon objects have to be on the same Slice or sequence name. */

    if (!((slice1 && slice2) || (seqname1 && seqname2)))
    {
        ajDebug("ensIntronNewExons got Ensembl Exon objects on both, an "
                "Ensembl Slice and sequence name.\n");

        return NULL;
    }

    if (slice1 && slice2 && (!ensSliceMatch(slice1, slice2)))
    {
        ajDebug("ensIntronNewExons got Ensembl Exon objects on different "
                "Ensembl Slice objects.\n");

        return NULL;
    }

    if (seqname1 && seqname2 && (!ajStrMatchCaseS(seqname1, seqname2)))
    {
        ajDebug("ensIntronNewExons got Ensembl Exon objects on different "
                "sequence names.\n");

        return NULL;
    }

    if (ensFeatureGetStrand(feature1) != ensFeatureGetStrand(feature2))
    {
        ajDebug("ensIntronNewExons got Ensembl Exon objects on different "
                "strands.\n");

        return NULL;
    }

    if (ensFeatureGetStrand(feature1) >= 0)
    {
        start = ensFeatureGetEnd(feature1)   + 1;
        end   = ensFeatureGetStart(feature2) - 1;
    }
    else
    {
        start = ensFeatureGetEnd(feature2)   + 1;
        end   = ensFeatureGetStart(feature1) - 1;
    }

    if (start > (end + 1))
    {
        ajDebug("ensIntronNewExons requires that the start coordinate %u "
                "is less than the end coordinate %u + 1 ", start, end);

        return NULL;
    }

    strand = ensFeatureGetStrand(feature1);

    if (slice1)
        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    slice1,
                                    start,
                                    end,
                                    strand);

    if (seqname1)
        feature = ensFeatureNewIniN((EnsPAnalysis) NULL,
                                    seqname1,
                                    start,
                                    end,
                                    strand);

    if (feature)
    {
        AJNEW0(intron);

        intron->Feature      = feature;
        intron->PreviousExon = ensExonNewRef(exon1);
        intron->NextExon     = ensExonNewRef(exon2);
        intron->Use          = 1U;
    }
    else
        ajDebug("ensIntronNewExons could not create an Ensembl Feature.\n");

    return intron;
}




/* @func ensIntronNewRef ******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] intron [EnsPIntron] Ensembl Intron
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPIntron ensIntronNewRef(EnsPIntron intron)
{
    if (!intron)
        return NULL;

    intron->Use++;

    return intron;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Del Destroy (free) an Ensembl Intron
**
** @argrule * Pintron [EnsPIntron*] Ensembl Intron address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensIntronDel *********************************************************
**
** Default destructor for an Ensembl Intron.
**
** @param [d] Pintron [EnsPIntron*] Ensembl Intron address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensIntronDel(EnsPIntron *Pintron)
{
    EnsPIntron pthis = NULL;

    if (!Pintron)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensIntronDel"))
    {
        ajDebug("ensIntronDel\n"
                "  *Pintron %p\n",
                *Pintron);

        ensIntronTrace(*Pintron, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pintron)
        return;

    pthis = *Pintron;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pintron = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensExonDel(&pthis->PreviousExon);
    ensExonDel(&pthis->NextExon);

    AJFREE(pthis);

    *Pintron = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Get Return Intron attribute(s)
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Previousexon Return the previous Ensembl Exon
** @nam4rule Nextexon Return the next Ensembl Exon
**
** @argrule * intron [const EnsPIntron] Intron
**
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Nextexon [EnsPExon] Ensembl Exon or NULL
** @valrule Previousexon [EnsPExon] Ensembl Exon or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensIntronGetFeature **************************************************
**
** Get the Ensembl Feature member of an Ensembl Intron.
**
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensIntronGetFeature(const EnsPIntron intron)
{
    return (intron) ? intron->Feature : NULL;
}




/* @func ensIntronGetNextexon *************************************************
**
** Get the next Ensembl Exon member of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::next_Exon
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPExon] Ensembl Exon or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExon ensIntronGetNextexon(const EnsPIntron intron)
{
    return (intron) ? intron->NextExon : NULL;
}




/* @func ensIntronGetPreviousexon *********************************************
**
** Get the previous Ensembl Exon member of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::prev_Exon
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPExon] Ensembl Exon or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExon ensIntronGetPreviousexon(const EnsPIntron intron)
{
    return (intron) ? intron->PreviousExon : NULL;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Trace Report Intron members to debug file
**
** @argrule Trace intron [const EnsPIntron] Ensembl Intron
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronTrace *******************************************************
**
** Trace an Ensembl Intron.
**
** @param [r] intron [const EnsPIntron] Ensembl Intron
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIntronTrace(const EnsPIntron intron, ajuint level)
{
    AjPStr indent = NULL;

    if (!intron)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensIntronTrace %p\n"
            "%S  Feature %p\n"
            "%S  PreviousExon %p\n"
            "%S  NextExon %p\n"
            "%S  Use %u\n",
            indent, intron,
            indent, intron->Feature,
            indent, intron->PreviousExon,
            indent, intron->NextExon,
            indent, intron->Use);

    ensFeatureTrace(intron->Feature, level + 1);

    ensExonTrace(intron->PreviousExon, level + 1);

    ensExonTrace(intron->NextExon, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Calculate Calculate Ensembl Intron values
** @nam4rule Length  Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * intron [const EnsPIntron] Ensembl Intron
**
** @valrule Length [ajuint] Length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronCalculateLength *********************************************
**
** Calculate the length of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::length
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [ajuint] Length or 0U
**
** @release 6.4.0
** @@
** NOTE: The Bio::EnsEMBL::Intron::length method seems to override
** Bio::EnsEMBL::Feature::length to allow for zero-length introns.
******************************************************************************/

ajuint ensIntronCalculateLength(const EnsPIntron intron)
{
    if (!intron)
        return 0U;

    return ensFeatureGetEnd(intron->Feature)
        - ensFeatureGetStart(intron->Feature) + 1U;
}




/* @func ensIntronCalculateMemsize ********************************************
**
** Calculate the memory size in bytes of an Ensembl Intron.
**
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensIntronCalculateMemsize(const EnsPIntron intron)
{
    size_t size = 0;

    if (!intron)
        return 0;

    size += sizeof (EnsOIntron);

    size += ensFeatureCalculateMemsize(intron->Feature);

    size += ensExonCalculateMemsize(intron->PreviousExon);
    size += ensExonCalculateMemsize(intron->NextExon);

    return size;
}

/* @source ensintron **********************************************************
**
** Ensembl Intron functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.28 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:07:37 $ by $Author: mks $
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
#include "enstranscript.h"




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

/* @conststatic intronsupportingevidenceKType *********************************
**
** The Ensembl Intron Supporting Evidence type member is enumerated in both,
** the SQL table definition and the data structure. The following strings are
** used for conversion in database operations and correspond to
** EnsEIntronsupportingevidenceType and the
** 'intron_supporting_evidence.score_type' field.
**
******************************************************************************/

static const char *const intronsupportingevidenceKType[] =
{
    "",
    "NONE",
    "DEPTH",
    (const char *) NULL
};




/* @conststatic intronsupportingevidenceadaptorKTablenames ********************
**
** Array of Ensembl Intron Supporting Evidence Adaptor SQL table names
**
******************************************************************************/

static const char *const intronsupportingevidenceadaptorKTablenames[] =
{
    "intron_supporting_evidence",
    (const char *) NULL
};




/* @conststatic intronsupportingevidenceadaptorKColumnnames *******************
**
** Array of Ensembl Intron Supporting Evidence Adaptor SQL column names
**
******************************************************************************/

static const char *const intronsupportingevidenceadaptorKColumnnames[] =
{
    "intron_supporting_evidence.intron_supporting_evidence_id",
    "intron_supporting_evidence.seq_region_id",
    "intron_supporting_evidence.seq_region_start",
    "intron_supporting_evidence.seq_region_end",
    "intron_supporting_evidence.seq_region_strand",
    "intron_supporting_evidence.analysis_id",
    "intron_supporting_evidence.hit_name",
    "intron_supporting_evidence.score",
    "intron_supporting_evidence.score_type",
    "intron_supporting_evidence.is_splice_canonical",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static EnsPIntron intronNewCpyFeatures(EnsPIntron intron);

static AjBool intronsupportingevidenceadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ises);




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
** @cc CVS Revision: 1.22
** @cc CVS Tag: branch-ensembl-68
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
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
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
** @param [uN] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPIntron ensIntronNewIni(EnsPExon exon1,
                           EnsPExon exon2,
                           EnsPAnalysis analysis)
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

    if (ajDebugTest("ensIntronNewIni"))
    {
        ajDebug("ensIntronNewIni\n"
                "  exon1 %p\n"
                "  exon2 %p\n"
                "  analysis %p\n",
                exon1,
                exon2,
                analysis);

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
        ajDebug("ensIntronNewIni got Ensembl Exon objects on both, an "
                "Ensembl Slice and sequence name.\n");

        return NULL;
    }

    if (slice1 && slice2 && (!ensSliceMatch(slice1, slice2)))
    {
        ajDebug("ensIntronNewIni got Ensembl Exon objects on different "
                "Ensembl Slice objects.\n");

        return NULL;
    }

    if (seqname1 && seqname2 && (!ajStrMatchCaseS(seqname1, seqname2)))
    {
        ajDebug("ensIntronNewIni got Ensembl Exon objects on different "
                "sequence names.\n");

        return NULL;
    }

    if (ensFeatureGetStrand(feature1) != ensFeatureGetStrand(feature2))
    {
        ajDebug("ensIntronNewIni got Ensembl Exon objects on different "
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
        ajDebug("ensIntronNewIni requires that the start coordinate %u "
                "is less than the end coordinate %u + 1 ", start, end);

        return NULL;
    }

    strand = ensFeatureGetStrand(feature1);

    if (slice1)
        feature = ensFeatureNewIniS(analysis,
                                    slice1,
                                    start,
                                    end,
                                    strand);

    if (seqname1)
        feature = ensFeatureNewIniN(analysis,
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
        ajDebug("ensIntronNewIni could not create an Ensembl Feature.\n");

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




/* @funcstatic intronNewCpyFeatures *******************************************
**
** Returns a new copy of an Ensembl Intron, but in addition to the shallow
** copy provided by ensIntronNewCpy, also copies all Ensembl Intron-
** internal Ensembl Objects based on the Ensembl Feature class. This is useful
** in preparation of ensIntronTransform and ensIntronTransfer, which
** return an independent Ensembl Intron object and therefore, require
** independent mapping of all internal Feature objects to the new
** Ensembl Coordinate System or Ensembl Slice.
**
** @param [u] intron [EnsPIntron] Ensembl Intron
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

static EnsPIntron intronNewCpyFeatures(EnsPIntron intron)
{
    EnsPExon newexon = NULL;

    EnsPIntron newintron = NULL;

    if (!intron)
        return NULL;

    newintron = ensIntronNewCpy(intron);

    if (!newintron)
        return NULL;

    newexon = ensExonNewCpy(intron->PreviousExon);
    ensExonDel(&intron->PreviousExon);
    intron->PreviousExon = newexon;

    newexon = ensExonNewCpy(intron->NextExon);
    ensExonDel(&intron->NextExon);
    intron->NextExon = newexon;

    return newintron;
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

    if (!(pthis = *Pintron) || --pthis->Use)
    {
        *Pintron = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensExonDel(&pthis->PreviousExon);
    ensExonDel(&pthis->NextExon);

    ajMemFree((void **) Pintron);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Get Return Ensembl Intron attribute(s)
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
** @cc Bio::EnsEMBL::Feature
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




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Set Set one member of an Ensembl Intron
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Nextexon Set the next Ensembl Exon
** @nam4rule Previousexon Set the previous Ensembl Exon
**
** @argrule * intron [EnsPIntron] Ensembl Intron
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Nextexon exon [EnsPExon] Next Ensembl Exon
** @argrule Previousexon exon [EnsPExon] Previous Ensembl Exon
**
** @fcategory modify
******************************************************************************/





/* @func ensIntronSetFeature **************************************************
**
** Set the Ensembl Feature member of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Feature
** @param [u] intron [EnsPIntron] Ensembl Intron
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronSetFeature(EnsPIntron intron,
                           EnsPFeature feature)
{
    EnsPExon newexon = NULL;

    EnsPSlice slice = NULL;

    if (!intron)
        return ajFalse;

    /* Replace the current Feature. */

    ensFeatureDel(&intron->Feature);

    if (feature)
    {
        intron->Feature = ensFeatureNewRef(feature);

        slice = ensFeatureGetSlice(intron->Feature);

        /*
        ** Transfer Ensembl Exon objects onto the new Ensembl Slice
        ** linked to this Ensembl Feature.
        */

        newexon = ensExonTransfer(intron->PreviousExon, slice);
        ensExonDel(&intron->PreviousExon);
        intron->PreviousExon = newexon;

        newexon = ensExonTransfer(intron->NextExon, slice);
        ensExonDel(&intron->NextExon);
        intron->NextExon = newexon;
    }

    return ajTrue;
}




/* @func ensIntronSetNextexon *************************************************
**
** Set the next Ensembl Exon member of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::next_Exon
** @param [u] intron [EnsPIntron] Ensembl Intron
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
** NOTE: The Perl API has no set method and does not transfer the Ensembl Exon
** object onto the Ensembl Slice of the Ensembl Intron object.
******************************************************************************/

AjBool ensIntronSetNextexon(EnsPIntron intron,
                            EnsPExon exon)
{
    if (!intron)
        return ajFalse;

    ensExonDel(&intron->NextExon);

    intron->NextExon = ensExonTransfer(
        exon,
        ensFeatureGetSlice(intron->Feature));

    if (!intron->NextExon)
        ajWarn("ensIntronSetNextexon could not transfer "
               "Ensembl Exon %u onto the "
               "Ensembl Slice of this "
               "Ensembl Intron.\n",
               ensExonGetIdentifier(exon));

    return ajTrue;
}




/* @func ensIntronSetPreviousexon *********************************************
**
** Set the previous Ensembl Exon member of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::prev_Exon
** @param [u] intron [EnsPIntron] Ensembl Intron
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
** NOTE: The Perl API has no set method and does not transfer the Ensembl Exon
** object onto the Ensembl Slice of the Ensembl Intron object.
******************************************************************************/

AjBool ensIntronSetPreviousexon(EnsPIntron intron,
                                EnsPExon exon)
{
    if (!intron)
        return ajFalse;

    ensExonDel(&intron->PreviousExon);

    intron->PreviousExon = ensExonTransfer(
        exon,
        ensFeatureGetSlice(intron->Feature));

    if (!intron->NextExon)
        ajWarn("ensIntronSetPreviousexon could not transfer "
               "Ensembl Exon '%u' onto the "
               "Ensembl Slice of this "
               "Ensembl Intron.\n",
               ensExonGetIdentifier(exon));

    return ajTrue;
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
** Functions for calculating information from an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Calculate Calculate Ensembl Intron information
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
        - ensFeatureGetStart(intron->Feature)
        + 1U;
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




/* @section fetch *************************************************************
**
** Functions for fetching information from an Ensembl Intron object.
**
** @fdata [EnsPIntron]
**
** @nam3rule Fetch Fetch Ensembl Intron information
** @nam4rule All   Fetch all Ensembl Intron information
** @nam4rule Splicesequences
** Fetch splice site sequences
**
** @argrule * intron [const EnsPIntron] Ensembl Intron
** @argrule Splicesequences Pdonor [AjPStr*]
** AJAX String (splice donor sequence) address
** @argrule Splicesequences Pacceptor [AjPStr*]
** AJAX String (splice acceptor sequence) address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronFetchSplicesequences ****************************************
**
** Fetch splice site sequences of an Ensembl Intron.
**
** The caller is responsible for deleting the
** AJAX String objects.
**
** @cc Bio::EnsEMBL::Intron::splice_seq
** @param [r] intron [const EnsPIntron] Ensembl Intron
** @param [wP] Pdonor [AjPStr*]
** AJAX String (splice donor sequence) address
** @param [wP] Pacceptor [AjPStr*]
** AJAX String (splice acceptor sequence) address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronFetchSplicesequences(const EnsPIntron intron,
                                     AjPStr *Pdonor,
                                     AjPStr *Pacceptor)
{
    ajuint length = 0U;

    AjBool result = AJFALSE;

    EnsPSlice      slice = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!intron)
        return ajFalse;

    if (!*Pdonor)
        return ajFalse;

    if (!*Pacceptor)
        return ajFalse;

    if (*Pdonor)
        ajStrAssignClear(Pdonor);

    if (*Pacceptor)
        ajStrAssignClear(Pacceptor);

    sla = ensRegistryGetSliceadaptor(
        ensExonadaptorGetDatabaseadaptor(
            ensExonGetAdaptor(intron->PreviousExon)));

    result = ensSliceadaptorFetchByFeature(sla,
                                           intron->Feature,
                                           0,
                                           &slice);

    if (!result)
        return result;

    length = ensSliceCalculateLength(slice);

    result = ensSliceFetchSequenceSubStr(slice, 1, 2, 1, Pdonor);

    if (!result)
        return result;

    result = ensSliceFetchSequenceSubStr(slice,
                                         length - 1,
                                         length,
                                         1,
                                         Pacceptor);

    return result;
}




/* @section test **************************************************************
**
** Functions for testing Ensembl Intron objects
**
** @fdata [EnsPIntron]
**
** @nam3rule Is Ensembl Intron has a property
** @nam4rule Canonical
** Test, whether an Ensembl Intron has canonical splice sites
**
** @argrule * intron [const EnsPIntron] Ensembl Intron
** @argrule Canonical Presult [AjBool*] AJAX Boolean address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensIntronIsCanonical *************************************************
**
** Test, whether an Ensembl Intron has canonical splice sites.
**
** @cc Bio::EnsEMBL::Intron::is_splice_canonical
** @param [r] intron [const EnsPIntron] Ensembl Intron
** @param [w] Presult [AjBool*] AJAX Boolean address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronIsCanonical(const EnsPIntron intron, AjBool *Presult)
{
    AjPStr both     = NULL;
    AjPStr donor    = NULL;
    AjPStr acceptor = NULL;

    if (!intron)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    *Presult = AJFALSE;

    ensIntronFetchSplicesequences(intron, &donor, &acceptor);

    both = ajStrNew();
    ajStrAppendS(&both, donor);
    ajStrAppendS(&both, acceptor);

    if (ajStrMatchCaseC(both, "GTAG"))
        *Presult = ajTrue;

    if (ajStrMatchCaseC(both, "ATAC"))
        *Presult = ajTrue;

    if (ajStrMatchCaseC(both, "GCAG"))
        *Presult = ajTrue;

    ajStrDel(&both);
    ajStrDel(&donor);
    ajStrDel(&acceptor);

    return ajTrue;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Intron objects
**
** @fdata [EnsPIntron]
**
** @nam3rule Match      Test Ensembl Intron objects for identity
** @nam3rule Overlap    Test Ensembl Intron objects for overlap
** @nam3rule Similarity Test Ensembl Intron objects for similarity
**
** @argrule * intron1 [const EnsPIntron] Ensembl Intron
** @argrule * intron2 [const EnsPIntron] Ensembl Intron
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensIntronMatch *******************************************************
**
** Test Ensembl Intron objects for identity.
**
** @param [r] intron1 [const EnsPIntron] Ensembl Intron
** @param [r] intron2 [const EnsPIntron] Ensembl Intron
**
** @return [AjBool] ajTrue if the Ensembl Intron objects are equal
**
** @release 6.5.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, individual members are compared.
******************************************************************************/

AjBool ensIntronMatch(const EnsPIntron intron1,
                      const EnsPIntron intron2)
{
    if (!intron1)
        return ajFalse;

    if (!intron2)
        return ajFalse;

    if (!ensFeatureMatch(intron1->Feature, intron2->Feature))
        return ajFalse;

    if (!ensExonMatch(intron1->PreviousExon, intron2->PreviousExon))
        return ajFalse;

    if (!ensExonMatch(intron1->NextExon, intron2->NextExon))
        return ajFalse;

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Intron objects between
** Ensembl Coordinate System objects.
**
** @fdata [EnsPIntron]
**
** @nam3rule Transfer Transfer an Ensembl Intron
** @nam3rule Transform Transform an Ensembl Intron
**
** @argrule * intron [EnsPIntron] Ensembl Intron
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr]
** Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr]
** Ensembl Coordinate System version
**
** @valrule * [EnsPIntron] Ensembl Intron or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronTransfer ****************************************************
**
** Transfer an Ensembl Intron onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Intron::transfer
** @param [u] intron [EnsPIntron] Ensembl Intron
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntron ensIntronTransfer(EnsPIntron intron,
                             EnsPSlice slice)
{
    EnsPFeature newif = NULL;

    EnsPIntron newintron = NULL;

    if (ajDebugTest("ensIntronTransfer"))
        ajDebug("ensIntronTransfer\n"
                "  intron %p\n"
                "  slice %p\n",
                intron,
                slice);

    if (!intron)
        return NULL;

    if (!slice)
        return NULL;

    if (!intron->Feature)
        ajFatal("ensIntronTransfer cannot transfer an Ensembl Intron "
                "without an Ensembl Feature.\n");

    newif = ensFeatureTransfer(intron->Feature, slice);

    if (!newif)
        return NULL;

    newintron = intronNewCpyFeatures(intron);

    ensIntronSetFeature(newintron, newif);

    ensFeatureDel(&newif);

    return newintron;
}




/* @func ensIntronTransform ***************************************************
**
** Transform an Ensembl Intron into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Intron::transform
** @param [u] intron [EnsPIntron] Ensembl Intron
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntron ensIntronTransform(EnsPIntron intron,
                              const AjPStr csname,
                              const AjPStr csversion)
{
    EnsPFeature newif = NULL;

    EnsPIntron newintron = NULL;

    if (!intron)
        return NULL;

    if (!csname)
        return NULL;

    newif = ensFeatureTransform(intron->Feature,
                                csname,
                                csversion,
                                (EnsPSlice) NULL);

    if (!newif)
        return NULL;

    newintron = intronNewCpyFeatures(intron);

    ensIntronSetFeature(newintron, newif);

    ensFeatureDel(&newif);

    return newintron;
}




/* @datasection [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @nam2rule Intronsupportingevidence
** Functions for manipulating Ensembl Intron Supporting Evidence objects
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence
** @cc CVS Revision: 1.4
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Intron Supporting Evidence by pointer.
** It is the responsibility of the user to first destroy any previous
** Intron Supporting Evidence. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the use counter
**
** @argrule Cpy ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule Ini isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @argrule Ini identifier [ajuint] SQL database internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini intron [EnsPIntron] Ensembl Intron
** @argrule Ini hitname [AjPStr] Hit name
** @argrule Ini canonical [AjBool] Canonical splice sites
** @argrule Ini score [double] Score
** @argrule Ini type [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration
** @argrule Ref ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @valrule * [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensIntronsupportingevidenceNewCpy ************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidence ensIntronsupportingevidenceNewCpy(
    const EnsPIntronsupportingevidence ise)
{
    EnsPIntronsupportingevidence pthis = NULL;

    if (!ise)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = ise->Identifier;
    pthis->Adaptor    = ise->Adaptor;
    pthis->Feature    = ensFeatureNewRef(ise->Feature);
    pthis->Intron     = ensIntronNewRef(ise->Intron);

    if (ise->Hitname)
        pthis->Hitname = ajStrNewRef(ise->Hitname);

    pthis->Canonical = ise->Canonical;
    pthis->Score     = ise->Score;
    pthis->Type      = ise->Type;

    return pthis;
}




/* @func ensIntronsupportingevidenceNewIni ************************************
**
** Default constructor for an Ensembl Intron Supporting Evidence
** with initial values.
**
** @cc Bio::EnsEMBL::IntronSupportingtEvidence::new
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] intron [EnsPIntron] Ensembl Intron
** @param [u] hitname [AjPStr] Hit name
** @param [r] canonical [AjBool] Canonical splice sites
** @param [r] score [double] Score
** @param [u] type [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration
**
** @return [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidence ensIntronsupportingevidenceNewIni(
    EnsPIntronsupportingevidenceadaptor isea,
    ajuint identifier,
    EnsPFeature feature,
    EnsPIntron intron,
    AjPStr hitname,
    AjBool canonical,
    double score,
    EnsEIntronsupportingevidenceType type)
{
    EnsPIntronsupportingevidence ise = NULL;

    if (!intron)
        return NULL;

    AJNEW0(ise);

    ise->Use        = 1U;
    ise->Identifier = identifier;
    ise->Adaptor    = isea;

    if (intron)
        ise->Feature = ensFeatureNewCpy(ensIntronGetFeature(intron));
    else
        ise->Feature = ensFeatureNewRef(feature);

    ise->Intron = ensIntronNewRef(intron);

    if (hitname)
        ise->Hitname = ajStrNewRef(hitname);

    if (intron)
        ensIntronIsCanonical(intron, &ise->Canonical);
    else
        ise->Canonical = canonical;

    ise->Score = score;
    ise->Type  = type;

    return ise;
}




/* @func ensIntronsupportingevidenceNewRef ************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidence ensIntronsupportingevidenceNewRef(
    EnsPIntronsupportingevidence ise)
{
    if (!ise)
        return NULL;

    ise->Use++;

    return ise;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Intron Supporting Evidence object.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Del Destroy (free) an Ensembl Intron Supporting Evidence
**
** @argrule * Pise [EnsPIntronsupportingevidence*]
** Ensembl Intron Supporting Evidence address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensIntronsupportingevidenceDel ***************************************
**
** Default destructor for an Ensembl Intron Supporting Evidence.
**
** @param [d] Pise [EnsPIntronsupportingevidence*]
** Ensembl Intron Supporting Evidence address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensIntronsupportingevidenceDel(EnsPIntronsupportingevidence *Pise)
{
    EnsPIntronsupportingevidence pthis = NULL;

    if (!Pise)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensIntronsupportingevidenceDel"))
    {
        ajDebug("ensIntronsupportingevidenceDel\n"
                "  *Pise %p\n",
                *Pise);

        ensIntronsupportingevidenceTrace(*Pise, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pise) || --pthis->Use)
    {
        *Pise = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);
    ensIntronDel(&pthis->Intron);

    ajStrDel(&pthis->Hitname);

    ajMemFree((void **) Pise);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Intron Supporting Evidence object.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Get Return Ensembl Intron Supporting Evidence attribute(s)
** @nam4rule Adaptor Return the Ensembl Intron Supporting Evidence Adaptor
** @nam4rule Canonical Return the canonical splice site member
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Hitname Return the hit name
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Intron Return the Ensembl Intron
** @nam4rule Score Return the score
** @nam4rule Type Return the
** Ensembl Intron Supporting Evidence Type enumeration
**
** @argrule * ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @valrule Adaptor [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor or NULL
** @valrule Canonical [AjBool] Canonical splice sites or AJFALSE
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Hitname [AjPStr] Hit name
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Intron [EnsPIntron] Ensembl Intron or NULL
** @valrule Score [double] Score or 0.0
** @valrule Type [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration or
** ensEIntronsupportingevidenceTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensIntronsupportingevidenceGetAdaptor ********************************
**
** Get the Ensembl Intron Supporting Evidence Adaptor member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidenceadaptor ensIntronsupportingevidenceGetAdaptor(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Adaptor : NULL;
}




/* @func ensIntronsupportingevidenceGetCanonical ******************************
**
** Get the canonical splice sites member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::is_splice_canonical
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [AjBool] Canonical splice sites or AJFALSE
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceGetCanonical(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Canonical : AJFALSE;
}




/* @func ensIntronsupportingevidenceGetFeature ********************************
**
** Get the Ensembl Feature member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Feature
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeature ensIntronsupportingevidenceGetFeature(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Feature : NULL;
}




/* @func ensIntronsupportingevidenceGetHitname ********************************
**
** Get the hit name member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::hit_name
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [AjPStr] Hit name or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensIntronsupportingevidenceGetHitname(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Hitname : NULL;
}




/* @func ensIntronsupportingevidenceGetIdentifier *****************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensIntronsupportingevidenceGetIdentifier(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Identifier : 0U;
}




/* @func ensIntronsupportingevidenceGetIntron *********************************
**
** Get the Ensembl Intron member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::get_Intron
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [EnsPIntron] Ensembl Intron or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntron ensIntronsupportingevidenceGetIntron(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Intron : NULL;
}




/* @func ensIntronsupportingevidenceGetScore **********************************
**
** Get the score member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::score
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [double] Score or 0.0
**
** @release 6.5.0
** @@
******************************************************************************/

double ensIntronsupportingevidenceGetScore(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Score : 0.0;
}




/* @func ensIntronsupportingevidenceGetType ***********************************
**
** Get the Ensembl Intron Supporting Evidence Type enumeration member of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::score_type
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration or
** ensEIntronsupportingevidenceTypeNULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsEIntronsupportingevidenceType ensIntronsupportingevidenceGetType(
    const EnsPIntronsupportingevidence ise)
{
    return (ise) ? ise->Type : ensEIntronsupportingevidenceTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Intron Supporting Evidence object.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Set Set one member of an Ensembl Intron Supporting Evidence
** @nam4rule Feature Set the Ensembl Feature
**
** @argrule * ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Feature feature [EnsPFeature] Ensembl Feature
**
** @fcategory modify
******************************************************************************/





/* @func ensIntronsupportingevidenceSetFeature ********************************
**
** Set the Ensembl Feature member of an Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Feature
** @param [u] ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceSetFeature(
    EnsPIntronsupportingevidence ise,
    EnsPFeature feature)
{
    if (!ise)
        return ajFalse;

    ensFeatureDel(&ise->Feature);

    ise->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Intron Supporting Evidence object.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Trace Report Ensembl Intron Supporting Evidence members
** to debug file
**
** @argrule Trace ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronsupportingevidenceTrace *************************************
**
** Trace an Ensembl Intron Supporting Evidence.
**
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceTrace(
    const EnsPIntronsupportingevidence ise,
    ajuint level)
{
    AjPStr indent = NULL;

    if (!ise)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%ensIntronsupportingevidenceTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Intron %p\n"
            "%S  Hitname '%S'\n"
            "%S  Canonical '%B'\n"
            "%S  Score %f.3\n"
            "%S  Type '%s'\n",
            indent, ise,
            indent, ise->Use,
            indent, ise->Identifier,
            indent, ise->Adaptor,
            indent, ise->Feature,
            indent, ise->Intron,
            indent, ise->Hitname,
            indent, ise->Canonical,
            indent, ise->Score,
            indent, ensIntronsupportingevidenceTypeToChar(ise->Type));

    ensFeatureTrace(ise->Feature, level + 1);
    ensIntronTrace(ise->Intron, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Intron Supporting Evidence object.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Calculate Calculate Ensembl Intron Supporting Evidence information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronsupportingevidenceCalculateMemsize **************************
**
** Calculate the memory size in bytes of an
** Ensembl Intron Supporting Evidence.
**
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensIntronsupportingevidenceCalculateMemsize(
    const EnsPIntronsupportingevidence ise)
{
    size_t size = 0;

    if (!ise)
        return 0;

    size += sizeof (EnsOIntronsupportingevidence);

    size += ensFeatureCalculateMemsize(ise->Feature);
    size += ensIntronCalculateMemsize(ise->Intron);

    if (ise->Hitname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ise->Hitname);
    }

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching information from an
** Ensembl Intron Supporting Evidence object.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Fetch Fetch Ensembl Intron Supporting Evidence information
** @nam4rule All   Fetch all Ensembl Intron Supporting Evidence information
** @nam4rule Exons Fetch Ensembl Exon objects
**
** @argrule * ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule Exons transcript [EnsPTranscript] Ensembl Transcript
** @argrule Exons Pprevious [EnsPExon*] Previous Ensembl Exon
** @argrule Exons Pnext [EnsPExon*] Next Ensembl Exon
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronsupportingevidenceFetchExons ********************************
**
** Fetch Ensembl Exon objects for an Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Intron::find_previous_Exon
** @cc Bio::EnsEMBL::Intron::find_next_Exon
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Pprevious [EnsPExon*] Previous Ensembl Exon address
** @param [wP] Pnext [EnsPExon*] Next Ensembl Exon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceFetchExons(
    const EnsPIntronsupportingevidence ise,
    EnsPTranscript transcript,
    EnsPExon *Pprevious,
    EnsPExon *Pnext)
{
    ajuint previous = 0U;
    ajuint next     = 0U;

    const AjPList exons = NULL;
    AjIList iter = NULL;

    EnsPExon exon = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature ifeature = NULL;

    if (!ise)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!Pprevious)
        return ajFalse;

    if (!Pnext)
        return ajFalse;

    *Pprevious = NULL;
    *Pnext    = NULL;

    ifeature = ensIntronsupportingevidenceGetFeature(ise);

    if (ise->Adaptor)
        ensIntronsupportingevidenceadaptorRetrieveExonidentifiers(
            ise->Adaptor,
            ise,
            transcript,
            &previous,
            &next);

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if (previous && next)
        {
            /* For perstent objects stored in a database. */

            if (ensExonGetIdentifier(exon) == previous)
                *Pprevious = ensExonNewRef(exon);

            if (ensExonGetIdentifier(exon) == next)
                *Pnext = ensExonNewRef(exon);
        }
        else
        {
            efeature = ensExonGetFeature(exon);

            if (ensFeatureGetStrand(ifeature) >= 0)
            {
                if (ensFeatureGetEnd(efeature) ==
                    (ensFeatureGetStart(ifeature) - 1))
                    *Pprevious = ensExonNewRef(exon);

                if (ensFeatureGetStart(efeature) ==
                    (ensFeatureGetEnd(ifeature) + 1))
                    *Pnext = ensExonNewRef(exon);
            }
            else
            {
                if (ensFeatureGetStart(efeature) ==
                    (ensFeatureGetEnd(ifeature) + 1))
                    *Pprevious = ensExonNewRef(exon);

                if (ensFeatureGetEnd(efeature) ==
                    (ensFeatureGetStart(ifeature) - 1))
                    *Pnext = ensExonNewRef(exon);

            }
        }
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Intron Supporting Evidence objects
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Match      Test for identity
** @nam3rule Overlap    Test for overlap
** @nam3rule Similarity Test for similarity
**
** @argrule * ise1 [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule * ise2 [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensIntronsupportingevidenceMatch *************************************
**
** Test Ensembl Intron Supporting Evidence objects for identity.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::equals
** @param [r] ise1 [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [r] ise2 [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [AjBool] ajTrue if the Ensembl Intron Supporting Evidence objects
** are equal
**
** @release 6.5.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, individual members are compared.
******************************************************************************/

AjBool ensIntronsupportingevidenceMatch(
    const EnsPIntronsupportingevidence ise1,
    const EnsPIntronsupportingevidence ise2)
{
    if (!ise1)
        return ajFalse;

    if (!ise2)
        return ajFalse;

    if (ise1 == ise2)
        return ajTrue;

    if (ise1->Identifier != ise2->Identifier)
        return ajFalse;

    if ((ise1->Adaptor && ise2->Adaptor) && (ise1->Adaptor != ise2->Adaptor))
        return ajFalse;

    if (!ensFeatureMatch(ise1->Feature, ise2->Feature))
        return ajFalse;

    if ((ise1->Intron && ise2->Intron) &&
        !ensIntronMatch(ise1->Intron, ise2->Intron))
        return ajFalse;

    if (!ajStrMatchCaseS(ise1->Hitname, ise2->Hitname))
        return ajFalse;

    if (ise1->Canonical != ise2->Canonical)
        return ajFalse;

    if (ise1->Score != ise2->Score)
        return ajFalse;

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Intron Supporting Evidence objects between
** Ensembl Coordinate System objects.
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Transfer Transfer an Ensembl Intron Supporting Evidence
** @nam3rule Transform Transform an Ensembl Intron Supporting Evidence
**
** @argrule * ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr]
** Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr]
** Ensembl Coordinate System version
**
** @valrule * [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronsupportingevidenceTransfer **********************************
**
** Transfer an Ensembl Intron Supporting Evidence onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::IntronSupportingEvidence::transfer
** @param [u] ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidence ensIntronsupportingevidenceTransfer(
    EnsPIntronsupportingevidence ise,
    EnsPSlice slice)
{
    EnsPFeature newfeature = NULL;

    EnsPIntronsupportingevidence newise = NULL;

    if (!ise)
        return NULL;

    if (!slice)
        return NULL;

    newfeature = ensFeatureTransfer(ise->Feature, slice);

    if (!newfeature)
        return NULL;

    newise = ensIntronsupportingevidenceNewCpy(ise);

    ensIntronsupportingevidenceSetFeature(newise, newfeature);

    return newise;
}




/* @func ensIntronsupportingevidenceTransform *********************************
**
** Transform an Ensembl Intron Supporting Feature into another
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::IntronSupportingFeature::transform
** @param [u] ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidence ensIntronsupportingevidenceTransform(
    EnsPIntronsupportingevidence ise,
    const AjPStr csname,
    const AjPStr csversion)
{
    EnsPFeature newfeature = NULL;

    EnsPIntronsupportingevidence newise = NULL;

    if (!ise)
        return NULL;

    if (!csname)
        return NULL;

    newfeature = ensFeatureTransform(ise->Feature,
                                     csname,
                                     csversion,
                                     (EnsPSlice) NULL);

    if (!newfeature)
        return NULL;

    newise = ensIntronsupportingevidenceNewCpy(ise);

    ensIntronsupportingevidenceSetFeature(newise, newfeature);

    return newise;
}




/* @section test **************************************************************
**
** Functions for testing Ensembl Intron Supporting Evidence objects
**
** @fdata [EnsPIntronsupportingevidence]
**
** @nam3rule Has Ensembl Intron Supporting Evidence has a property
** @nam4rule Transcripts
** Test, whether an Ensembl Intron Supporting Evidnce has
** Ensembl Transcript objects linked
**
** @argrule * ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule Transcripts Presult [AjBool*] AJAX Boolean address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensIntronsupportingevidenceHasTranscripts ****************************
**
** Test, whether an Ensembl Intron has Ensembl Transcript objects linked.
**
** @cc Bio::EnsEMBL::Intron::is_splice_canonical
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [w] Presult [AjBool*] AJAX Boolean address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceHasTranscripts(
    const EnsPIntronsupportingevidence ise,
    AjBool *Presult)
{
    AjBool result = AJFALSE;

    AjPList identifiers = NULL;

    if (!ise)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    *Presult = ajFalse;

    result = ensIntronsupportingevidenceadaptorRetrieveAllTranscriptidentifiers(
        ise->Adaptor,
        ise,
        identifiers);

    if (ajListGetLength(identifiers))
        *Presult = ajTrue;
    else
        *Presult = ajFalse;

    ajListFreeData(&identifiers);

    return result;
}




/* @datasection [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type
**
** @nam2rule Intronsupportingevidence Functions for manipulating
** Ensembl Intron Supporting Evidence objects
** @nam3rule IntronsupportingevidenceType Functions for manipulating
** Ensembl Intron Supporting Evidence Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl Intron Supporting Evidence Type enumeration.
**
** @fdata [EnsEIntronsupportingevidenceType]
**
** @nam4rule From Ensembl Intron Supporting Evidence Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration or
** ensEIntronsupportingevidenceTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronsupportingevidenceTypeFromStr *******************************
**
** Return an Ensembl Intron Supporting Evidence Type enumeration
** from an AJAX String.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration or
** ensEIntronsupportingevidenceTypeNULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsEIntronsupportingevidenceType ensIntronsupportingevidenceTypeFromStr(
    const AjPStr type)
{
    register EnsEIntronsupportingevidenceType i =
        ensEIntronsupportingevidenceTypeNULL;

    EnsEIntronsupportingevidenceType iset =
        ensEIntronsupportingevidenceTypeNULL;

    for (i = ensEIntronsupportingevidenceTypeNULL;
         intronsupportingevidenceKType[i];
         i++)
        if (ajStrMatchC(type, intronsupportingevidenceKType[i]))
            iset = i;

    if (!iset)
        ajDebug("ensIntronsupportingevidenceTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return iset;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Intron Supporting Evidence Type enumeration.
**
** @fdata [EnsEIntronsupportingevidenceType]
**
** @nam4rule To   Return Ensembl Intron Supporting Evidence Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To iset [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration
**
** @valrule Char [const char*] Status
**
** @fcategory cast
******************************************************************************/




/* @func ensIntronsupportingevidenceTypeToChar ********************************
**
** Cast an Ensembl Intron Supporting Evidence Type enumeration
** into a C-type (char *) string.
**
** @param [u] iset [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration
**
** @return [const char*]
** Ensembl Intron Supporting Evidence Type C-type (char *) string
**
** @release 6.5.0
** @@
******************************************************************************/

const char* ensIntronsupportingevidenceTypeToChar(
    EnsEIntronsupportingevidenceType iset)
{
    register EnsEIntronsupportingevidenceType i =
        ensEIntronsupportingevidenceTypeNULL;

    for (i = ensEIntronsupportingevidenceTypeNULL;
         intronsupportingevidenceKType[i] && (i < iset);
         i++);

    if (!intronsupportingevidenceKType[i])
        ajDebug("ensIntronsupportingevidenceTypeToChar "
                "encountered an out of boundary error on "
                "Ensembl Intron Supporting Evidence Type "
                "enumeration %d.\n",
                iset);

    return intronsupportingevidenceKType[i];
}




/* @datasection [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
**
** @nam2rule Intronsupportingevidenceadaptor Functions for manipulating
** Ensembl Intron Supporting Evidence Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::IntronSupportingEvidenceAdaptor
** @cc CVS Revision: 1.2
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic intronsupportingevidenceadaptorFetchAllbyStatement *************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Intron Supporting Evidence objects.
**
** @cc Bio::EnsEMBL::DBSQL::IntronSupportingEvidenceAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] ises [AjPList] AJAX List of
** Ensembl Intron Supporting Evidence objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool intronsupportingevidenceadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ises)
{
    double score = 0.0;

    ajuint identifier = 0U;

    ajuint srid       = 0U;
    ajuint srstart    = 0U;
    ajuint srend      = 0U;
    ajint  srstrand   = 0;
    ajuint analysisid = 0U;

    AjBool canonical = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr hitname = NULL;
    AjPStr type    = NULL;

    EnsPFeature feature = NULL;

    EnsPIntron intron = NULL;

    EnsPIntronsupportingevidence        ise  = NULL;
    EnsPIntronsupportingevidenceadaptor isea = NULL;

    EnsEIntronsupportingevidenceType iset =
        ensEIntronsupportingevidenceTypeNULL;

    EnsPDatabaseadaptor dba = NULL;

    if (ajDebugTest("intronsupportingevidenceadaptorFetchAllbyStatement"))
        ajDebug("intronsupportingevidenceadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  ises %p\n",
                ba,
                statement,
                am,
                slice,
                ises);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!ises)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    isea = ensRegistryGetIntronsupportingevidenceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0U;
        srid       = 0U;
        srstart    = 0U;
        srend      = 0U;
        srstrand   = 0;
        analysisid = 0U;
        hitname    = ajStrNew();
        score      = 0.0;
        type       = ajStrNew();
        canonical  = AJFALSE;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &hitname);
        ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToBool(sqlr, &canonical);

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&hitname);
            ajStrDel(&type);

            continue;
        }

        iset = ensIntronsupportingevidenceTypeFromStr(type);

        if (!iset)
            ajDebug("intronsupportingevidenceadaptorFetchAllbyStatement "
                    "encountered unexpected string '%S' in the "
                    "'intron_supporting_evidence.score_type' field.\n",
                    type);

        ise = ensIntronsupportingevidenceNewIni(
            isea,
            identifier,
            feature,
            (EnsPIntron) NULL,
            hitname,
            canonical,
            score,
            iset);

        ajListPushAppend(ises, (void *) ise);

        ensFeatureDel(&feature);
        ensIntronDel(&intron);

        ajStrDel(&hitname);
        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Intron Supporting Evidence Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Intron Supporting Evidence Adaptor.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPIntronsupportingevidenceadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensIntronsupportingevidenceadaptorNew ********************************
**
** Default constructor for an Ensembl Intron Supporting Evidence Adaptor.
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
** @see ensRegistryGetIntronsupportingevidenceadaptor
**
** @cc Bio::EnsEMBL::DBSQL::IntronSupportingEvidenceAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPIntronsupportingevidenceadaptor ensIntronsupportingevidenceadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        intronsupportingevidenceadaptorKTablenames,
        intronsupportingevidenceadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &intronsupportingevidenceadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensIntronsupportingevidenceNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) NULL,
        (size_t (*)(const void *)) &ensIntronsupportingevidenceCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensIntronsupportingevidenceGetFeature,
        "Intronsupportingevidence");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Intron Supporting Evidence Adaptor object.
**
** @fdata [EnsPIntronsupportingevidenceadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Intron Supporting Evidence Adaptor
**
** @argrule * Pisea [EnsPIntronsupportingevidenceadaptor*]
** Ensembl Intron Supporting Evidence Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensIntronsupportingevidenceadaptorDel ********************************
**
** Default destructor for an Ensembl Intron Supporting Evidence Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pisea [EnsPIntronsupportingevidenceadaptor*]
** Ensembl Intron Supporting Evidence Adaptor address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensIntronsupportingevidenceadaptorDel(
    EnsPIntronsupportingevidenceadaptor *Pisea)
{
    ensFeatureadaptorDel(Pisea);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Intron Supporting Evidence Adaptor object.
**
** @fdata [EnsPIntronsupportingevidenceadaptor]
**
** @nam3rule Get Return Ensembl Intron Supporting Evidence Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensIntronsupportingevidenceadaptorGetBaseadaptor *********************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Intron Supporting Evidence Adaptor.
**
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensIntronsupportingevidenceadaptorGetBaseadaptor(
    EnsPIntronsupportingevidenceadaptor isea)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensIntronsupportingevidenceadaptorGetFeatureadaptor(isea));
}




/* @func ensIntronsupportingevidenceadaptorGetDatabaseadaptor *****************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Intron Supporting Evidence Adaptor.
**
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensIntronsupportingevidenceadaptorGetDatabaseadaptor(
    EnsPIntronsupportingevidenceadaptor isea)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensIntronsupportingevidenceadaptorGetFeatureadaptor(isea));
}




/* @func ensIntronsupportingevidenceadaptorGetFeatureadaptor ******************
**
** Get the Ensembl Feature Adaptor member of an
** Ensembl Intron Supporting Evidence Adaptor.
**
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensIntronsupportingevidenceadaptorGetFeatureadaptor(
    EnsPIntronsupportingevidenceadaptor isea)
{
    return isea;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Intron Supporting Evidence objects from an
** Ensembl SQL database.
**
** @fdata [EnsPIntronsupportingevidenceadaptor]
**
** @nam3rule Fetch  Fetch Ensembl Intron Supporting Evidence object(s)
** @nam4rule All    Fetch all Ensembl Intron Supporting Evidence objects
** @nam4rule Allby  Fetch all Ensembl Intron Supporting Evidence objects
** matching a criterion
** @nam5rule Transcript Fetch all by an Ensembl Transcript
** @nam4rule By     Fetch one Ensembl Intron Supporting Evidence object
** matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @argrule All ises [AjPList]
** AJAX List of Ensembl Intron Supporting Evidence objects
** @argrule AllbyTranscript transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule Allby ises [AjPList]
** AJAX List of Ensembl Intron Supporting Evidence objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pise [EnsPIntronsupportingevidence*]
** Ensembl Intron Supporting Evidence address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensIntronsupportingevidenceadaptorFetchAllbyTranscript ***************
**
** Fetch all Ensembl Intron Supporting Evidence objects via an
** Ensembl Transcript object.
**
** The caller is responsible for deleting the
** Ensembl Intron Supporting Evidence objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::IntronSupportingEvidenceAdaptor::
** fetch_all_by_Transcript
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] ises [AjPList]
** AJAX List of Ensembl Intron Supporting Evidence objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceadaptorFetchAllbyTranscript(
    EnsPIntronsupportingevidenceadaptor isea,
    const EnsPTranscript transcript,
    AjPList ises)
{
    void **valarray = NULL;

    register ajuint i = 0U;

    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable table = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!isea)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!ises)
        return ajFalse;

    dba = ensIntronsupportingevidenceadaptorGetDatabaseadaptor(isea);

    statement = ajFmtStr(
        "SELECT "
        "intron_supporting_evidence_id "
        "FROM "
        "transcript_intron_supporting_evidence "
        "WHERE "
        "intron_supporting_evidence_id.transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    ajStrDel(&statement);

    table = ajTableuintNew(ajSqlstatementGetSelectedrows(sqls));

    ajTableSetDestroyvalue(
        table,
        (void (*)(void **)) &ensIntronsupportingevidenceDel);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajTablePut(table, (void *) Pidentifier, NULL);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ensBaseadaptorFetchAllbyIdentifiers(
        ensIntronsupportingevidenceadaptorGetBaseadaptor(isea),
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensIntronsupportingevidenceGetIdentifier,
        table);

    ajTableToarrayValues(table, &valarray);

    for (i = 0U; valarray[i]; i++)
        ajListPushAppend(ises,
                         (void *) ensIntronsupportingevidenceNewRef(
                             (EnsPIntronsupportingevidence) valarray[i]));

    AJFREE(valarray);

    ajTableDel(&table);

    return ajTrue;
}




/* @func ensIntronsupportingevidenceadaptorFetchByIdentifier ******************
**
** Fetch an Ensembl Intron Supporting Evidence via its
** SQL database-internal identifier.
**
** The caller is responsible for deleting the
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Map::DBSQL::IntronSupportingEvidenceAdaptor::fetch_by_dbID
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pise [EnsPIntronsupportingevidence*]
** Ensembl Intron Supporting Evidence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceadaptorFetchByIdentifier(
    EnsPIntronsupportingevidenceadaptor isea,
    ajuint identifier,
    EnsPIntronsupportingevidence *Pise)
{
    return ensBaseadaptorFetchByIdentifier(
        ensIntronsupportingevidenceadaptorGetBaseadaptor(isea),
        identifier,
        (void **) Pise);
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to
** Ensembl Intron Supporting Evidence objects
** from an Ensembl SQL database.
**
** @fdata [EnsPIntronsupportingevidenceadaptor]
**
** @nam3rule Retrieve
** Retrieve Ensembl Intron Supporting Evidence-releated object(s)
** @nam4rule All
** Retrieve all Ensembl Intron Supporting Evidence-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
** @nam5rule Transcriptidentifiers
** Retrieve all AJAX unsigned integer (Ensembl Transcript identifier) objects
** @nam4rule Exonidentifiers
** Retrive AJAX unsigned integer (Ensembl Exon identifier) objects
**
** @argrule * isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer
** (Ensembl Intron Supporting Evidence identifier) objects
** @argrule AllTranscriptidentifiers ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule AllTranscriptidentifiers transcriptids [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Transcript identifier) objects
** @argrule Exonidentifiers ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @argrule Exonidentifiers transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule Exonidentifiers Previous [ajuint*]
** Previous Ensembl Exon identifier
** @argrule Exonidentifiers Pnext [ajuint*]
** Next Ensembl Exon identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensIntronsupportingevidenceadaptorRetrieveAllTranscriptidentifiers ***
**
** Retrieve all Ensembl Transcript identifier objects of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Map::DBSQL::IntronSupportingEvidenceAdaptor::
** list_linked_transcript_ids
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [u] transcriptids [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Transcript identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceadaptorRetrieveAllTranscriptidentifiers(
    EnsPIntronsupportingevidenceadaptor isea,
    const EnsPIntronsupportingevidence ise,
    AjPList transcriptids)
{
    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!isea)
        return ajFalse;

    if (!ise)
        return ajFalse;

    if (!transcriptids)
        return ajFalse;

    dba = ensIntronsupportingevidenceadaptorGetDatabaseadaptor(isea);

    statement = ajFmtStr(
        "SELECT "
        "transcript_intron_supporting_evidence.transcript_id "
        "FROM "
        "transcript_intron_supporting_evidence "
        "WHERE "
        "transcript_intron_supporting_evidence."
        "intron_supporting_evidence_id = %u",
        ensIntronsupportingevidenceGetIdentifier(ise));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(transcriptids, (void *) Pidentifier);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensIntronsupportingevidenceadaptorRetrieveExonidentifiers ************
**
** Retrieve Ensembl Exon identifiers of an
** Ensembl Intron Supporting Evidence.
**
** @cc Bio::EnsEMBL::Map::DBSQL::IntronSupportingEvidenceAdaptor::
** fetch_flanking_exon_ids
** @param [u] isea [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @param [r] ise [const EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [wP] Previous [ajuint*] Previous Ensembl Exon identifier
** @param [wP] Pnext [ajuint*] Next Ensembl Exon identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensIntronsupportingevidenceadaptorRetrieveExonidentifiers(
    EnsPIntronsupportingevidenceadaptor isea,
    const EnsPIntronsupportingevidence ise,
    const EnsPTranscript transcript,
    ajuint *Previous,
    ajuint *Pnext)
{
    ajuint previousid = 0U;
    ajuint nextid     = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!isea)
        return ajFalse;

    if (!ise)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!Previous)
        return ajFalse;

    if (!Pnext)
        return ajFalse;

    *Previous = 0U;
    *Pnext    = 0U;

    dba = ensIntronsupportingevidenceadaptorGetDatabaseadaptor(isea);

    statement = ajFmtStr(
        "SELECT "
        "transcript_intron_supporting_evidence.previous_exon_id, "
        "transcript_intron_supporting_evidence.next_exon_id "
        "FROM "
        "transcript_intron_supporting_evidence "
        "WHERE "
        "transcript_intron_supporting_evidence."
        "transcript_id = %u "
        "AND "
        "transcript_intron_supporting_evidence."
        "intron_supporting_evidence_id = %u",
        ensTranscriptGetIdentifier(transcript),
        ensIntronsupportingevidenceGetIdentifier(ise));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        previousid = 0U;
        nextid     = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &previousid);
        ajSqlcolumnToUint(sqlr, &nextid);

        if (!*Previous)
        {
            *Previous = previousid;
            *Pnext    = nextid;
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

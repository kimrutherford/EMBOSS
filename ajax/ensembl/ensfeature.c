/* @source ensfeature *********************************************************
**
** Ensembl Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.72 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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

#include "ensexternaldatabase.h"
#include "ensfeature.h"
#include "ensmetacoordinate.h"
#include "ensmetainformation.h"
#include "ensprojectionsegment.h"
#include "enstable.h"




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

/* @conststatic featureadaptorKMaxSplitQuerySeqregions ************************
**
** Maximum number of Ensembl Mapper Result objects for which multiple regional
** constraints for Feature objects on Sequence Regions are used. Above this
** number of regions, it is faster to limit by start and end coordinates.
**
******************************************************************************/

static const ajuint featureadaptorKMaxSplitQuerySeqregions = 3U;




/* @conststatic featureadaptorKCacheMaxBytes **********************************
**
** Maximum memory size in bytes the Ensembl Feature Adaptor-internal
** Ensembl Cache can use.
**
** 1 << 26 = 64 Mi
**
******************************************************************************/

static const size_t featureadaptorKCacheMaxBytes = 1U << 26U;




/* @conststatic featureadaptorKCacheMaxCount **********************************
**
** Maximum number of objects based on the Ensembl Feature class the
** Ensembl Feature Adaptor-internal Ensembl Cache can hold.
**
** 1 << 16 = 64 ki
**
******************************************************************************/

static const ajuint featureadaptorKCacheMaxCount = 1U << 16U;




/* @conststatic featureadaptorKCacheMaxSize ***********************************
**
** Maximum memory size in bytes of an object based on the Ensembl Feature class
** to be allowed into the Ensembl Feature Adaptor-internal Ensembl Cache.
**
******************************************************************************/

static const size_t featureadaptorKCacheMaxSize = 0U;




/* @conststatic assemblyexceptionfeatureadaptorKCacheMaxBytes *****************
**
** Maximum memory size in bytes the Ensembl Assembly Exception Adaptor-internal
** Ensembl Cache can use.
**
** 1 << 26 = 64 Mi
**
******************************************************************************/

static const size_t assemblyexceptionfeatureadaptorKCacheMaxBytes = 1U << 26U;




/* @conststatic assemblyexceptionfeatureadaptorKCacheMaxCount *****************
**
** Maximum number of Ensembl Assembly Exception Feature objects the
** Ensembl Assembly Exception Feature Adaptor-internal Ensembl Cache can hold.
**
** 1 << 16 = 64 ki
**
******************************************************************************/

static const ajuint assemblyexceptionfeatureadaptorKCacheMaxCount = 1U << 16U;




/* @conststatic assemblyexceptionfeatureadaptorKCacheMaxSize ******************
**
** Maximum memory size in bytes of an Ensembl Assembly Exception Feature to be
** allowed into the Ensembl Assembly Exception Feature Adaptor-internal
** Ensembl Cache.
**
******************************************************************************/

static const size_t assemblyexceptionfeatureadaptorKCacheMaxSize = 0U;




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listFeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listFeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listFeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listFeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool featureadaptorRemap(EnsPFeatureadaptor fa,
                                  AjPList objects,
                                  EnsPAssemblymapper am,
                                  EnsPSlice slice);

static AjBool featureadaptorSliceFetch(EnsPFeatureadaptor fa,
                                       EnsPSlice slice,
                                       AjPStr constraint,
                                       AjPList objects);

static int listFeaturepairCompareSourceEndAscending(
    const void *item1,
    const void *item2);

static int listFeaturepairCompareSourceEndDescending(
    const void *item1,
    const void *item2);

static int listFeaturepairCompareSourceStartAscending(
    const void *item1,
    const void *item2);

static int listFeaturepairCompareSourceStartDescending(
    const void *item1,
    const void *item2);

static AjBool assemblyexceptionfeatureadaptorCacheInit(
    EnsPAssemblyexceptionfeatureadaptor aefa);

static AjBool assemblyexceptionfeatureadaptorRemap(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    AjPList aefs,
    EnsPAssemblymapper am,
    EnsPSlice slice);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensfeature ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPFeature] Ensembl Feature *********************************
**
** @nam2rule Feature Functions for manipulating Ensembl Feature objects
**
** @cc Bio::EnsEMBL::Feature
** @cc CVS Revision: 1.66
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPFeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @suffix N Constructor with a sequence name
** @suffix S Constructor with an Ensembl Slice
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy feature [const EnsPFeature] Ensembl Feature
** @argrule IniN analysis [EnsPAnalysis] Ensembl Analysis
** @argrule IniN seqname [AjPStr] Sequence name
** @argrule IniN start [ajint] Start coordinate
** @argrule IniN end [ajint] End coordinate
** @argrule IniN strand [ajint] Strand orientation
** @argrule IniS analysis [EnsPAnalysis] Ensembl Analysis
** @argrule IniS slice [EnsPSlice] Ensembl Slice
** @argrule IniS start [ajint] Start coordinate
** @argrule IniS end [ajint] End coordinate
** @argrule IniS strand [ajint] Strand orientation
** @argrule Ref feature [EnsPFeature] Ensembl Feature
**
** @valrule * [EnsPFeature] Ensembl Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensFeatureNewCpy *****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeature ensFeatureNewCpy(const EnsPFeature feature)
{
    EnsPFeature pthis = NULL;

    if (!feature)
        return NULL;

    AJNEW0(pthis);

    pthis->Analysis = ensAnalysisNewRef(feature->Analysis);
    pthis->Slice    = ensSliceNewRef(feature->Slice);

    if (feature->Sequencename)
        pthis->Sequencename = ajStrNewRef(feature->Sequencename);

    pthis->Start  = feature->Start;
    pthis->End    = feature->End;
    pthis->Strand = feature->Strand;
    pthis->Use    = 1U;

    return pthis;
}




/* @func ensFeatureNewIniN ****************************************************
**
** Constructor for an Ensembl Feature with a sequence name.
**
** This is useful for Ensembl Feature objects that are not annotated on a
** (genome sequence) Ensembl Slice, such as Ensembl Protein Feature objects.
**
** @cc Bio::EnsEMBL::Feature::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] seqname [AjPStr] Sequence name
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand orientation
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeature ensFeatureNewIniN(EnsPAnalysis analysis,
                              AjPStr seqname,
                              ajint start,
                              ajint end,
                              ajint strand)
{
    EnsPFeature feature = NULL;

    if (!seqname)
    {
        ajDebug("ensFeatureNewIniN requires a sequence name.\n");

        return NULL;
    }

    if (start && end && (start > (end + 1)))
    {
        ajDebug("ensFeatureNewIniN start (%u) must be less than or equal to "
                "end (%u) + 1.\n", start, end);

        return NULL;
    }

    if ((strand < -1) || (strand > 1))
    {
        ajDebug("ensFeatureNewIniN strand (%d) must be +1, 0 or -1.\n",
                strand);

        return NULL;
    }

    AJNEW0(feature);

    feature->Analysis = ensAnalysisNewRef(analysis);

    feature->Slice = NULL;

    if (seqname)
        feature->Sequencename = ajStrNewRef(seqname);

    feature->Start  = start;
    feature->End    = end;
    feature->Strand = strand;
    feature->Use    = 1U;

    return feature;
}




/* @func ensFeatureNewIniS ****************************************************
**
** Constructor for an Ensembl Feature with an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand orientation
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeature ensFeatureNewIniS(EnsPAnalysis analysis,
                              EnsPSlice slice,
                              ajint start,
                              ajint end,
                              ajint strand)
{
    EnsPFeature feature = NULL;

    if (!slice)
    {
        ajDebug("ensFeatureNewIniS reqires an Ensembl Slice.\n");

        return NULL;
    }

    if (start && end && (start > (end + 1)))
    {
        ajDebug("ensFeatureNewIniS start (%u) must be less than or equal to "
                "end (%u) + 1.\n", start, end);

        return NULL;
    }

    if ((strand < -1) || (strand > 1))
    {
        ajDebug("ensFeatureNewIniS strand (%d) must be +1, 0 or -1.\n",
                strand);

        return NULL;
    }

    AJNEW0(feature);

    feature->Analysis     = ensAnalysisNewRef(analysis);
    feature->Slice        = ensSliceNewRef(slice);
    feature->Sequencename = NULL;
    feature->Start        = start;
    feature->End          = end;
    feature->Strand       = strand;
    feature->Use          = 1U;

    return feature;
}




/* @func ensFeatureNewRef *****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensFeatureNewRef(EnsPFeature feature)
{
    if (!feature)
        return NULL;

    feature->Use++;

    return feature;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Feature object.
**
** @fdata [EnsPFeature]
**
** @nam3rule Del Destroy (free) an Ensembl Feature
**
** @argrule * Pfeature [EnsPFeature*] Ensembl Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensFeatureDel ********************************************************
**
** Default destructor for an Ensembl Feature.
**
** @param [d] Pfeature [EnsPFeature*] Ensembl Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensFeatureDel(EnsPFeature *Pfeature)
{
    EnsPFeature pthis = NULL;

    if (!Pfeature)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensFeatureDel"))
    {
        ajDebug("ensFeatureDel\n"
                "  *Pfeature %p\n",
                *Pfeature);

        ensFeatureTrace(*Pfeature, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pfeature)
        return;

    pthis = *Pfeature;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pfeature = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ensSliceDel(&pthis->Slice);

    ajStrDel(&pthis->Sequencename);

    AJFREE(pthis);

    *Pfeature = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
**
** @nam3rule Get Return Feature attribute(s)
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule End Return the end
** @nam4rule Sequencename Return the sequence name
** @nam4rule Slice Return the Ensembl Slice
** @nam4rule Start Return the start
** @nam4rule Strand Return the strand
**
** @argrule * feature [const EnsPFeature] Feature
**
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule End [ajint] End or 0
** @valrule Sequencename [AjPStr] Sequence name or NULL
** @valrule Slice [EnsPSlice] Ensembl Slice or NULL
** @valrule Start [ajint] Start or 0
** @valrule Strand [ajint] Strand or 0
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureGetAnalysis ************************************************
**
** Get the Ensembl Analysis member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::analysis
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensFeatureGetAnalysis(const EnsPFeature feature)
{
    return (feature) ? feature->Analysis : NULL;
}




/* @func ensFeatureGetEnd *****************************************************
**
** Get the end coordinate member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::end
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] End coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensFeatureGetEnd(const EnsPFeature feature)
{
    return (feature) ? feature->End : 0;
}




/* @func ensFeatureGetSequencename ********************************************
**
** Get the sequence name member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seqname
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [AjPStr] Sequence name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensFeatureGetSequencename(const EnsPFeature feature)
{
    return (feature) ? feature->Sequencename : NULL;
}




/* @func ensFeatureGetSlice ***************************************************
**
** Get the Ensembl Slice member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::slice
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [EnsPSlice] Ensembl Slice or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSlice ensFeatureGetSlice(const EnsPFeature feature)
{
    return (feature) ? feature->Slice : NULL;
}




/* @func ensFeatureGetStart ***************************************************
**
** Get the start coordinate member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::start
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Start coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensFeatureGetStart(const EnsPFeature feature)
{
    return (feature) ? feature->Start : 0;
}




/* @func ensFeatureGetStrand **************************************************
**
** Get the strand orientation member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::strand
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Strand orientation or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensFeatureGetStrand(const EnsPFeature feature)
{
    return (feature) ? feature->Strand : 0;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
**
** @nam3rule Set Set one member of a Feature
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule End Set the end
** @nam4rule Sequencename Set the sequence name
** @nam4rule Slice Set the Ensembl Slice
** @nam4rule Start Set the start
** @nam4rule Strand Set the strand
**
** @argrule * feature [EnsPFeature] Ensembl Feature object
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule End end [ajint] End coordinate
** @argrule Sequencename seqname [AjPStr] Sequence name
** @argrule Slice slice [EnsPSlice] Ensembl Slice
** @argrule Start start [ajint] Start coordinate
** @argrule Strand strand [ajint] Strand orientation
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensFeatureSetAnalysis ************************************************
**
** Set the Ensembl Analysis member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::analysis
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureSetAnalysis(EnsPFeature feature, EnsPAnalysis analysis)
{
    if (!feature)
        return ajFalse;

    ensAnalysisDel(&feature->Analysis);

    feature->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensFeatureSetEnd *****************************************************
**
** Set the end coordinate member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::end
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] end [ajint] End coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureSetEnd(EnsPFeature feature, ajint end)
{
    if (!feature)
        return ajFalse;

    feature->End = end;

    return ajTrue;
}




/* @func ensFeatureSetSequencename ********************************************
**
** Set the sequence name member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seqname
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] seqname [AjPStr] Sequence name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureSetSequencename(EnsPFeature feature, AjPStr seqname)
{
    if (!feature)
        return ajFalse;

    ajStrDel(&feature->Sequencename);

    feature->Sequencename = ajStrNewRef(seqname);

    return ajTrue;
}




/* @func ensFeatureSetSlice ***************************************************
**
** Set the Ensembl Slice member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::slice
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureSetSlice(EnsPFeature feature, EnsPSlice slice)
{
    if (!feature)
        return ajFalse;

    ensSliceDel(&feature->Slice);

    feature->Slice = ensSliceNewRef(slice);

    return ajTrue;
}




/* @func ensFeatureSetStart ***************************************************
**
** Set the start coordinate member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::start
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] start [ajint] Start coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureSetStart(EnsPFeature feature, ajint start)
{
    if (!feature)
        return ajFalse;

    feature->Start = start;

    return ajTrue;
}




/* @func ensFeatureSetStrand **************************************************
**
** Set the strand orientation member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::strand
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] strand [ajint] Strand orientation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureSetStrand(EnsPFeature feature, ajint strand)
{
    if (!feature)
        return ajFalse;

    feature->Strand = strand;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
**
** @nam3rule Trace Report Ensembl Feature members to debug file
**
** @argrule Trace feature [const EnsPFeature] Ensembl Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensFeatureTrace ******************************************************
**
** Trace an Ensembl Feature.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureTrace(const EnsPFeature feature, ajuint level)
{
    AjPStr indent = NULL;

    if (!feature)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensFeatureTrace %p\n"
            "%S  Slice %p\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Strand %d\n"
            "%S  Analysis %p\n"
            "%S  Sequencename '%S'\n"
            "%S  Use %u\n",
            indent, feature,
            indent, feature->Slice,
            indent, feature->Start,
            indent, feature->End,
            indent, feature->Strand,
            indent, feature->Analysis,
            indent, feature->Sequencename,
            indent, feature->Use);

    ensSliceTrace(feature->Slice, level + 1);

    ensAnalysisTrace(feature->Analysis, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
**
** @nam3rule Calculate Calculate Ensembl Feature values
** @nam4rule Length  Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Strand  Calculate the strand
**
** @argrule * feature [const EnsPFeature] Ensembl Feature
**
** @valrule Length [ajuint] Length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule Strand [char] Strand '+', '\0' or '-'
**
** @fcategory misc
******************************************************************************/




/* @func ensFeatureCalculateLength ********************************************
**
** Calculate the length of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::length
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajuint] Length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensFeatureCalculateLength(const EnsPFeature feature)
{
    if (!feature)
        return 0U;

    if (feature->Slice != NULL)
        return ensSliceCalculateRegion(feature->Slice,
                                       feature->Start,
                                       feature->End);

    if (feature->Start > feature->End)
        ajFatal("ensFeatureCalculateLength cannot calculate the length of an "
                "Ensembl Feature presumably on a circular Ensembl Slice, "
                "without an Ensembl Slice.");

    return feature->End - feature->Start + 1U;
}




/* @func ensFeatureCalculateMemsize *******************************************
**
** Calculate the memory size in bytes of an Ensembl Feature.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensFeatureCalculateMemsize(const EnsPFeature feature)
{
    size_t size = 0;

    if (!feature)
        return 0;

    size += sizeof (EnsOFeature);

    size += ensSliceCalculateMemsize(feature->Slice);

    size += ensAnalysisCalculateMemsize(feature->Analysis);

    if (feature->Sequencename)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(feature->Sequencename);
    }

    return size;
}




/* @func ensFeatureCalculateStrand ********************************************
**
** Calculate the strand of an Ensembl Feature.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [char] Strand '+', '\0' or '-'
**
** @release 6.5.0
** @@
******************************************************************************/

char ensFeatureCalculateStrand(const EnsPFeature feature)
{
    if (!feature)
        return '\0';

    if (feature->Strand > 0)
        return '+';

    if (feature->Strand < 0)
        return '-';

    return '\0';
}




/* @section convenience functions *********************************************
**
** Ensembl Feature convenience functions
**
** @fdata [EnsPFeature]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Seqregion Get an Ensembl Sequence Region of an Ensembl Slice
** @nam5rule End Get an Ensembl Sequence Region end
** @nam5rule Length Get an Ensembl Sequence Region length
** @nam5rule Name Get an Ensembl Sequence region name
** @nam5rule Object Get an Ensembl Sequence Region object
** @nam5rule Start Get an Ensembl Sequence Region start
** @nam5rule Strand Get an Ensembl Sequence Region strand
**
** @argrule * feature [const EnsPFeature] Ensembl Feature
**
** @valrule SeqregionEnd [ajint] Ensembl Sequence Region end or 0
** @valrule SeqregionLength [ajuint] Ensembl Sequence Region length or 0U
** @valrule SeqregionName [const AjPStr] Ensembl Sequence Region name or NULL
** @valrule SeqregionObject [const EnsPSeqregion] Ensembl Sequence Region or
** NULL
** @valrule SeqregionStart [ajint] Ensembl Sequence Region start or 0
** @valrule SeqregionStrand [ajint] Ensembl Sequence Region strand or 0
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureGetSeqregionEnd ********************************************
**
** Get the end coordinate of an Ensembl Feature relative to the
** Ensembl Sequence Region member of the Ensembl Slice member of an
** Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_end
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] End coordinate on an Ensembl Sequence Region or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensFeatureGetSeqregionEnd(const EnsPFeature feature)
{
    ajint srend    = 0;
    ajint srlength = 0;

    AjBool circular = AJFALSE;

    if (!feature)
        return 0;

    if (!feature->Slice)
        return 0;

    if (ensSliceIsCircular(feature->Slice, &circular) == ajFalse)
        return 0;

    srlength = ensSliceGetSeqregionLength(feature->Slice);

    if (ensSliceGetStrand(feature->Slice) >= 0)
        srend = ensSliceGetStart(feature->Slice) + feature->End   - 1;
    else
        srend = ensSliceGetEnd(feature->Slice)   - feature->Start + 1;

    if ((srend > srlength) && (circular == ajTrue))
        srend -= srlength;

    return srend;
}




/* @func ensFeatureGetSeqregionLength *****************************************
**
** Get the length member of the Ensembl Sequence Region member of the
** Ensembl Slice member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_length
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajuint] Ensembl Sequence Region length or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensFeatureGetSeqregionLength(const EnsPFeature feature)
{
    return (feature &&
            feature->Slice) ?
        (ajuint) ensSliceGetSeqregionLength(feature->Slice) : 0U;
}




/* @func ensFeatureGetSeqregionName *******************************************
**
** Get the name member of the Ensembl Sequence Region member of the
** Ensembl Slice member of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_name
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [const AjPStr] Ensembl Sequence Region name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensFeatureGetSeqregionName(const EnsPFeature feature)
{
    return (feature &&
            feature->Slice) ?
        ensSliceGetSeqregionName(feature->Slice) : NULL;
}




/* @func ensFeatureGetSeqregionObject *****************************************
**
** Get the Ensembl Sequence Region member of the
** Ensembl Slice member of an Ensembl Feature.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [const EnsPSeqregion] Ensembl Sequence Region or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const EnsPSeqregion ensFeatureGetSeqregionObject(const EnsPFeature feature)
{
    return (feature &&
            feature->Slice) ?
        ensSliceGetSeqregion(feature->Slice) : NULL;
}




/* @func ensFeatureGetSeqregionStart ******************************************
**
** Get the start coordinate of an Ensembl Feature relative to the
** Ensembl Sequence Region member of the Ensembl Slice member of an
** Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_start
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Start coordinate on an Ensembl Sequence Region or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensFeatureGetSeqregionStart(const EnsPFeature feature)
{
    ajint srstart  = 0;
    ajint srlength = 0;

    AjBool circular = AJFALSE;

    if (!feature)
        return 0;

    if (!feature->Slice)
        return 0;

    if (ensSliceIsCircular(feature->Slice, &circular) == ajFalse)
        return 0;

    srlength = ensSliceGetSeqregionLength(feature->Slice);

    if (ensSliceGetStrand(feature->Slice) >= 0)
    {
        if ((feature->Start < 0) && (circular == ajTrue))
            srstart = srlength + feature->Start;
        else
            srstart = ensSliceGetStart(feature->Slice) + feature->Start - 1;
    }
    else
        srstart = ensSliceGetEnd(feature->Slice) - feature->End + 1;

    if ((srstart > srlength) && (circular == ajTrue))
        srstart -= srlength;

    return srstart;
}




/* @func ensFeatureGetSeqregionStrand *****************************************
**
** Get the strand information of an Ensembl Feature relative to the
** Ensembl Sequence Region member of the Ensembl Slice member of an
** Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_strand
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Strand information on an Ensembl Sequence Region or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensFeatureGetSeqregionStrand(const EnsPFeature feature)
{
    return (feature &&
            feature->Slice) ?
        (ensSliceGetStrand(feature->Slice) * feature->Strand) : 0;
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an
** Ensembl Feature object.
**
** @fdata [EnsPFeature]
**
** @nam3rule Fetch Fetch object from an Ensembl Feature
** @nam4rule All
** @nam5rule Alternativelocations Fetch all alternative locations
** @nam4rule Sequencename Fetch the sequence name
**
** @argrule Alternativelocations feature [EnsPFeature] Ensembl Feature
** @argrule Alternativelocations all [AjBool] Fetch all Ensembl Feature objects
** @argrule Alternativelocations features [AjPList] AJAX List of
**                                                  Ensembl Feature objects
** @argrule Sequencename feature [const EnsPFeature] Ensembl Feature
** @argrule Sequencename Pname [AjPStr*] Sequence name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensFeatureFetchAllAlternativelocations *******************************
**
** Fetch all alternative locations of an Ensembl Feature on other symlinked
** Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Feature objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Feature::get_all_alt_locations
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] all [AjBool] Fetch all Ensembl Feature objects
** @param [u] features [AjPList] AJAX List of Ensembl Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureFetchAllAlternativelocations(EnsPFeature feature,
                                              AjBool all,
                                              AjPList features)
{
    ajint alength = 0;
    ajint rlength = 0;

    ajuint srid = 0U;

    AjPList aefs = NULL;
    AjPList haps = NULL;
    AjPList alts = NULL;

    EnsPAssemblyexceptionfeature aef         = NULL;
    EnsPAssemblyexceptionfeature naef        = NULL;
    EnsPAssemblyexceptionfeatureadaptor aefa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature afeature = NULL;
    EnsPFeature nfeature = NULL;

    EnsPSlice fslice    = NULL;
    EnsPSlice rslice    = NULL;
    EnsPSlice aslice    = NULL;
    EnsPSlice nslice    = NULL;
    EnsPSliceadaptor sa = NULL;

    if (!feature)
        return ajFalse;

    fslice = feature->Slice;

    if (!fslice)
        return ajTrue;

    sa = ensSliceGetAdaptor(fslice);

    if (!sa)
        return ajTrue;

    /*
    ** Fetch all Ensembl Assembly Exception Feature objects for the
    ** full-length Slice of the Ensembl Sequence Region member.
    */

    dba = ensSliceadaptorGetDatabaseadaptor(sa);

    aefa = ensRegistryGetAssemblyexceptionfeatureadaptor(dba);

    srid = ensSliceGetSeqregionIdentifier(fslice);

    ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &rslice);

    aefs = ajListNew();

    ensAssemblyexceptionfeatureadaptorFetchAllbySlice(aefa, rslice, aefs);

    /*
    ** Group Ensembl Assembly Exception Feature objects based on their
    ** Ensembl Assembly Exception Type enumeration member into
    ** haplotypes (HAPs) and pseudo-autosomal regions (PARs) initially.
    */

    haps = ajListNew();
    alts = ajListNew();

    while (ajListPop(aefs, (void **) &aef))
    {
        switch (ensAssemblyexceptionfeatureGetType(aef))
        {
            case ensEAssemblyexceptionTypeHAP:           /* fall through */
            case ensEAssemblyexceptionTypePatchFix:      /* fall through */
            case ensEAssemblyexceptionTypePatchNovel:    /* fall through */

                ajListPushAppend(haps, (void *) aef);

                break;

            case ensEAssemblyexceptionTypePAR:

                ajListPushAppend(alts, (void *) aef);

                break;

            case ensEAssemblyexceptionTypeHAPRef:        /* fall through */
            case ensEAssemblyexceptionTypePatchNovelRef: /* fall through */
            case ensEAssemblyexceptionTypePatchFixRef:   /* fall through */

                if (all)
                    ajListPushAppend(haps, (void *) aef);

                break;

            default:

                ajDebug("ensFeatureFetchAllAlternativelocations got unknown "
                        "EnsEAssemblyexceptionType (%d).\n",
                        ensAssemblyexceptionfeatureGetType(aef));
        }
    }

    ajListFree(&aefs);

    /*
    ** Regions surrounding haplotypes are those of interest, not the haplotype
    ** itself. Convert haplotype Assembly Exception Feature objects to regions
    ** around haplotypes instead.
    */

    while (ajListPop(haps, (void **) &aef))
    {
        afeature = ensAssemblyexceptionfeatureGetFeature(aef);

        aslice = ensAssemblyexceptionfeatureGetExceptionSlice(aef);

        if ((ensFeatureGetStart(afeature) > 1) &&
            (ensSliceGetStart(aslice) > 1))
        {
            /* Copy the Feature and re-set the start and end cordinates. */

            nfeature = ensFeatureNewCpy(afeature);

            ensFeatureSetStart(nfeature, 1);

            ensFeatureSetEnd(nfeature, ensFeatureGetStart(afeature) - 1);

            ensSliceadaptorFetchBySeqregionIdentifier(
                sa,
                ensSliceGetSeqregionIdentifier(aslice),
                1,
                ensSliceGetStart(aslice) - 1,
                ensSliceGetStrand(aslice),
                &nslice);

            naef = ensAssemblyexceptionfeatureNewIni(
                aefa,
                0,
                nfeature,
                nslice,
                ensAssemblyexceptionfeatureGetType(aef));

            ajListPushAppend(alts, (void *) naef);

            ensSliceDel(&nslice);

            ensFeatureDel(&nfeature);
        }

        /* Check that Slice lengths are within range. */

        if (ensSliceGetSeqregionLength(rslice) <= INT_MAX)
            rlength = ensSliceGetSeqregionLength(rslice);
        else
            ajFatal("ensFeatureFetchAllAlternativelocations got "
                    "Sequence Region length (%u) exceeding MAX_INT (%d).\n",
                    ensSliceGetSeqregionLength(rslice),
                    INT_MAX);

        if (ensSliceGetSeqregionLength(aslice) <= INT_MAX)
            alength = ensSliceGetSeqregionLength(aslice);
        else
            ajFatal("ensFeatureFetchAllAlternativelocations got "
                    "Sequence Region length (%u) exceeding MAX_INT (%d).\n",
                    ensSliceGetSeqregionLength(aslice),
                    INT_MAX);

        if ((ensFeatureGetEnd(afeature) < rlength) &&
            (ensSliceGetEnd(aslice) < alength))
        {
            /* Copy the Feature and re-set the start and end cordinates. */

            nfeature = ensFeatureNewCpy(afeature);

            ensFeatureSetStart(nfeature, ensFeatureGetEnd(afeature) + 1);

            ensFeatureSetEnd(nfeature, ensFeatureGetSeqregionLength(afeature));

            ensSliceadaptorFetchBySeqregionIdentifier(
                sa,
                ensSliceGetSeqregionIdentifier(aslice),
                ensSliceGetEnd(aslice) + 1,
                ensSliceGetSeqregionLength(aslice),
                ensSliceGetStrand(aslice),
                &nslice);

            naef = ensAssemblyexceptionfeatureNewIni(
                aefa,
                0,
                nfeature,
                nslice,
                ensAssemblyexceptionfeatureGetType(aef));

            ajListPushAppend(alts, (void *) naef);

            ensSliceDel(&nslice);
        }

        ensAssemblyexceptionfeatureDel(&aef);
    }

    ajListFree(&haps);

    /* Check if exception regions contain our Feature. */

    while (ajListPop(alts, (void **) &aef))
    {
        afeature = ensAssemblyexceptionfeatureGetFeature(aef);

        aslice = ensAssemblyexceptionfeatureGetExceptionSlice(aef);

        /* Ignore the other region if the Feature is not entirely on it. */

        if ((ensFeatureGetSeqregionStart(feature)
             < ensFeatureGetStart(afeature)) ||
            (ensFeatureGetSeqregionEnd(feature)
             > ensFeatureGetEnd(afeature)))
        {
            ensAssemblyexceptionfeatureDel(&aef);

            continue;
        }

        nfeature = ensFeatureNewCpy(feature);

        /* Position the Feature on the entire Slice of the other region. */

        nfeature->Start = ensFeatureGetSeqregionStart(nfeature)
            - ensFeatureGetStart(afeature)
            + ensSliceGetStart(aslice);

        nfeature->End = ensFeatureGetSeqregionEnd(nfeature)
            - ensFeatureGetStart(afeature)
            + ensSliceGetStart(aslice);

        nfeature->Strand *= ensSliceGetStrand(aslice);

        /*
        ** Place the new Feature objects on the full-length Slice of the
        ** Ensembl Sequence Region member.
        */

        ensSliceadaptorFetchBySeqregionIdentifier(
            sa,
            ensSliceGetSeqregionIdentifier(aslice),
            0,
            0,
            0,
            &nslice);

        ensFeatureSetSlice(nfeature, nslice);

        ajListPushAppend(features, (void *) nfeature);

        ensSliceDel(&nslice);
    }

    ajListFree(&alts);

    ensSliceDel(&fslice);

    return ajTrue;
}




/* @func ensFeatureFetchSequencename ******************************************
**
** Fetch the name of the sequence, on which an Ensembl Feature is annotated.
** The name is the Ensembl Feature sequence name member or, if not available,
** the name of the underlying Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::seqname
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [wP] Pname [AjPStr*] Sequence name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureFetchSequencename(const EnsPFeature feature, AjPStr *Pname)
{
    if (!feature)
        return ajFalse;

    if (!Pname)
        return ajFalse;

    if (*Pname)
        ajStrAssignClear(Pname);
    else
        *Pname = ajStrNew();

    if (feature->Sequencename && ajStrGetLen(feature->Sequencename))
        ajStrAssignS(Pname, feature->Sequencename);
    else if (feature->Slice)
        ensSliceFetchName(feature->Slice, Pname);

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Feature objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPFeature]
**
** @nam3rule Move Move an Ensembl Feature
** @nam3rule Project Project an Ensembl Feature
** @nam3rule Projectslice Project an Ensembl Feature onto an Ensembl Slice
** @nam3rule Transfer Transfer an Ensembl Feature
** @nam3rule Transform Transform an Ensembl Feature
**
** @argrule Move feature [EnsPFeature] Ensembl Feature
** @argrule Move start [ajint] Start coordinate
** @argrule Move end [ajint] End coordinate
** @argrule Move strand [ajint] Strand orientation
** @argrule Project feature [const EnsPFeature] Ensembl Feature
** @argrule Project csname [const AjPStr] Ensembl Coordinate System name
** @argrule Project csversion [const AjPStr] Ensembl Coordinate System version
** @argrule Project pss [AjPList] AJAX List of
** Ensembl Projection Segment objects
** @argrule Projectslice feature [const EnsPFeature] Ensembl Feature
** @argrule Projectslice slice [EnsPSlice] Ensembl Slice
** @argrule Projectslice pss [AjPList] AJAX List of
** Ensembl Projection Segment objects
** @argrule Transfer feature [EnsPFeature] Ensembl Feature
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform feature [EnsPFeature] Ensembl Feature
** @argrule Transform csname [const AjPStr]
** Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr]
** Ensembl Coordinate System version
** @argrule Transform slice [EnsPSlice] Ensembl Slice
**
** @valrule Move [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Project [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Projectslice [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Transfer [EnsPFeature] Ensembl Feature or NULL
** @valrule Transform [EnsPFeature] Ensembl Feature or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensFeatureMove *******************************************************
**
** Move an Ensembl Feature on its Slice. This function sets the start and end
** coordinate, as well as the strand orientation simultaneously.
**
** @cc Bio::EnsEMBL::Feature::move
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand orientation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureMove(EnsPFeature feature,
                      ajint start,
                      ajint end,
                      ajint strand)
{
    if (!feature)
        return ajFalse;

    if (start && end && (end < start))
    {
        ajDebug("ensFeatureMove start (%u) must be less than or equal to the "
                "end coordinate (%u).\n", start, end);

        return ajFalse;
    }

    if ((strand < -1) || (strand > 1))
    {
        ajDebug("ensFeatureMove strand (%d) must be +1, 0 or -1.\n", strand);

        return ajFalse;
    }

    feature->Start  = start;
    feature->End    = end;
    feature->Strand = strand;

    return ajTrue;
}




/* @func ensFeatureProject ****************************************************
**
** Project an Ensembl Feature into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Feature::project
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
** This projection function does not move a Feature to another Slice, but it
** provides a definition of where a Feature lies in another Coordinate System.
** This is especially useful to see where a Feature would lie in a
** Coordinate System in which it crosses (Sequence Region) boundaries.
**
** Ensembl Projection Segment objects contain source coordinates releative to
** the Feature start and a target Slice spanning the region in the requested
** Coordinate System this Feature projects to.
**
** If the Feature projects entirely into a gap an empty AJAX List will be
** returned.
******************************************************************************/

AjBool ensFeatureProject(const EnsPFeature feature,
                         const AjPStr csname,
                         const AjPStr csversion,
                         AjPList pss)
{
    ajint strand = 0;

    EnsPSlice fslice     = NULL;
    EnsPSlice nslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!feature)
    {
        ajDebug("ensFeatureProject requires an Ensembl Feature.\n");

        return ajFalse;
    }

    if (!csname)
    {
        ajDebug("ensFeatureProject requires an "
                "Ensembl Coordinate System name.\n");

        return ajFalse;
    }

    /* A Coordinate System version is not strictly required. */

    if (!pss)
    {
        ajDebug("ensFeatureProject requires an AJAX List.\n");

        return ajFalse;
    }

    if (!feature->Slice)
    {
        ajWarn("ensFeatureProject requires an Ensembl Feature with "
               "an Ensembl Slice attached to it.\n");

        return ajFalse;
    }

    /*
    ** Use the Ensembl Database Adaptor of the Slice as this Feature may not
    ** yet be stored in the database and may not have its own Adaptor.
    */

    sla = ensSliceGetAdaptor(feature->Slice);

    if (!sla)
    {
        ajWarn("ensFeatureProject requires an Ensembl Feature with "
               "an Ensembl Slice Adaptor member attached to the "
               "Ensembl Slice member.\n");

        return ajFalse;
    }

    strand = feature->Strand * ensSliceGetStrand(feature->Slice);

    /*
    ** The ensSliceadaptorFetchByFeature function always returns a
    ** forward-strand Slice.
    */

    /*
    ** FIXME: So far this is the only instance of ensSliceFetchSliceinverted.
    ** Wouldn't ensSliceadaptorFetchByFeature be better if it allowed the
    ** specification of a strand?
    */

    ensSliceadaptorFetchByFeature(sla, feature, 0, &fslice);

    if (strand < 0)
        ensSliceFetchSliceinverted(fslice, &nslice);
    else
        nslice = ensSliceNewRef(fslice);

    ensSliceDel(&fslice);

    ensSliceProject(nslice, csname, csversion, pss);

    ensSliceDel(&nslice);

    return ajTrue;
}




/* @func ensFeatureProjectslice ***********************************************
**
** Project an Ensembl Feature onto an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::project_to_slice
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** This projection function does not move a Feature to another Slice, but it
** provides a definition of where a Feature lies in another Coordinate System.
** This is especially useful to see where a Feature would lie on a
** Slice on which it crosses (Sequence Region) boundaries.
**
** Ensembl Projection Segment objects contain source coordinates releative to
** the Feature start and a target Slice spanning the region in the requested
** Coordinate System this Feature projects to.
**
** If the Feature projects entirely into a gap an empty AJAX List will be
** returned.
******************************************************************************/

AjBool ensFeatureProjectslice(const EnsPFeature feature,
                              EnsPSlice slice,
                              AjPList pss)
{
    ajint strand = 0;

    EnsPSlice fslice     = NULL;
    EnsPSlice nslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!feature)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!pss)
        return ajFalse;

    if (!feature->Slice)
    {
        ajWarn("ensFeatureProjectslice requires an Ensembl Feature with "
               "an Ensembl Slice attached to it.\n");

        return ajFalse;
    }

    /*
    ** Use the Ensembl Database Adaptor of the Slice as this Feature may not
    ** yet be stored in the database and may not have its own Adaptor.
    */

    sla = ensSliceGetAdaptor(feature->Slice);

    if (!sla)
    {
        ajWarn("ensFeatureProjectslice requires an Ensembl Feature with "
               "an Ensembl Slice Adaptor member attached to the "
               "Ensembl Slice member.\n");

        return ajFalse;
    }

    strand = feature->Strand * ensSliceGetStrand(feature->Slice);

    /*
    ** The ensSliceadaptorFetchByFeature function always returns a
    ** forward-strand Slice.
    */

    /*
    ** FIXME: So far this is the only instance of ensSliceFetchSliceinverted.
    ** Wouldn't ensSliceadaptorFetchByFeature be better if it allowed the
    ** specification of a strand?
    */

    ensSliceadaptorFetchByFeature(sla, feature, 0, &fslice);

    if (strand < 0)
        ensSliceFetchSliceinverted(fslice, &nslice);
    else
        nslice = ensSliceNewRef(fslice);

    ensSliceDel(&fslice);

    ensSliceProjectslice(nslice, slice, pss);

    ensSliceDel(&nslice);

    return ajTrue;
}




/* @func ensFeatureTransfer ***************************************************
**
** Transfer an Ensembl Feature onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::transfer
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
** Returns a copy of this Feature, which has been shifted onto another Slice.
**
** If the new Slice is in a different Coordinate System, the Feature is
** transformed first and then placed on the Slice.
**
** If the Feature would be split across a Coordinate System boundary or mapped
** to a gap NULL will be returned instead.
**
** If the Feature cannot be placed on the provided Slice because it maps to an
** entirely different Sequence Region, NULL will be returned instead.
******************************************************************************/

EnsPFeature ensFeatureTransfer(EnsPFeature feature, EnsPSlice slice)
{
    ajint nfstart = 0;
    ajint nfend   = 0;

    const EnsPCoordsystem srccs = NULL;
    const EnsPCoordsystem trgcs = NULL;

    EnsPFeature nf = NULL;

    if (!feature)
    {
        ajDebug("ensFeatureTransfer requires an Ensembl Feature.\n");

        return NULL;
    }

    if (!slice)
    {
        ajDebug("ensFeatureTransfer requires an Ensembl Slice.\n");

        return NULL;
    }

    if (!feature->Slice)
    {
        ajDebug("ensFeatureTransfer requires an Ensembl Feature with "
                "an Ensembl Slice attached.");

        return NULL;
    }

    srccs = ensSliceGetCoordsystemObject(feature->Slice);
    trgcs = ensSliceGetCoordsystemObject(slice);

    /*
    ** If the Coordinate System objects of Feature and Slice are identical,
    ** adjust only coordinates. In case they are not, the Feature needs
    ** transforming into the target Cordinate System first.
    */

    if (ensCoordsystemMatch(srccs, trgcs))
        nf = ensFeatureNewCpy(feature);
    else
    {
        nf = ensFeatureTransform(feature,
                                 ensCoordsystemGetName(trgcs),
                                 ensCoordsystemGetVersion(trgcs),
                                 slice);

        if (!nf)
        {
            ajDebug("ensFeatureTransfer got no Feature from "
                    "ensFeatureTransform.\n");

            return NULL;
        }
    }

    /*
    ** Discard Feature objects that were transformed to an entirely different
    ** Sequence Region than the one underlying the requested Slice.
    */

    if (!ensSeqregionMatch(ensSliceGetSeqregion(nf->Slice),
                           ensSliceGetSeqregion(slice)))
    {
        ajDebug("ensFeatureTransfer transformed Ensembl Feature %p onto "
                "Sequence Region '%S:%S:%S', which is different from the "
                "requested Ensembl Slice '%S:%S:%S'.\n",
                nf,
                ensSliceGetCoordsystemName(nf->Slice),
                ensSliceGetCoordsystemVersion(nf->Slice),
                ensSliceGetSeqregionName(nf->Slice),
                ensSliceGetCoordsystemName(slice),
                ensSliceGetCoordsystemVersion(slice),
                ensSliceGetSeqregionName(slice));

        ensFeatureTrace(nf, 1);

        ensFeatureDel(&nf);

        return NULL;
    }

    /*
    ** Convert Feature coordinates from Slice to Sequence Region coordinates.
    ** NOTE: Instead of testing first, this implementation always converts,
    ** which should be faster.
    */

    nfstart = nf->Start;
    nfend   = nf->End;

    if (ensSliceGetStrand(nf->Slice) >= 0)
    {
        nf->Start = nfstart + ensSliceGetStart(nf->Slice) - 1;
        nf->End   = nfend   + ensSliceGetStart(nf->Slice) - 1;
    }
    else
    {
        nf->Start   = ensSliceGetEnd(nf->Slice) - nfend   + 1;
        nf->End     = ensSliceGetEnd(nf->Slice) - nfstart + 1;
        nf->Strand *= -1;
    }

    /*
    ** Convert Feature coordinates from Sequence Region to
    ** target Slice coordinates.
    */

    nfstart = nf->Start;
    nfend   = nf->End;

    if (ensSliceGetStrand(slice) >= 0)
    {
        nf->Start = nfstart - ensSliceGetStart(slice) + 1;
        nf->End   = nfend   - ensSliceGetStart(slice) + 1;
    }
    else
    {
        nf->Start   = ensSliceGetEnd(slice) - nfend   + 1;
        nf->End     = ensSliceGetEnd(slice) - nfstart + 1;
        nf->Strand *= -1;
    }

    ensFeatureSetSlice(nf, slice);

    return nf;
}




/* @func ensFeatureTransform **************************************************
**
** Transform an Ensembl Feature into another Ensembl Coordinate System.
**
** May simply return a reference copy
**
** @cc Bio::EnsEMBL::Feature::transform
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [uN] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
** Returns a copy of this Feature converted to a different Coordinate System.
**
** The converted Feature will be placed on a Slice which spans an entire
** Sequence Region of the new Coordinate System. If the requested Coordinate
** System is the same Coordinate System it is simply placed on a Slice, which
** spans the entire Sequence Region as opposed to the original Slice, which may
** have only partially covered the Sequence Region.
**
** If a Feature spans a (Sequence Region) boundary in the new
** Coordinate System, NULL will be returned instead.
**
** For example, transforming an Exon in contig coordinates to one in
** chromosome coordinates will place the Exon on a Slice of an entire
** chromosome.
******************************************************************************/

EnsPFeature ensFeatureTransform(EnsPFeature feature,
                                const AjPStr csname,
                                const AjPStr csversion,
                                EnsPSlice slice)
{
    AjBool match = AJFALSE;

    AjIList iterator = NULL;
    AjPList pss      = NULL;

    EnsPCoordsystem        cs  = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature nfeature = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice       nslice = NULL;
    const EnsPSlice pslice = NULL;
    EnsPSliceadaptor sla   = NULL;

    if (!feature)
    {
        ajDebug("ensFeatureTransform requires an Ensembl Feature.\n");

        return NULL;
    }

    if (!csname)
    {
        ajDebug("ensFeatureTransform requires a Coordinate System name.\n");

        return NULL;
    }

    /* A Coordinate System version is not strictly required. */

    if (!feature->Slice)
    {
        ajWarn("ensFeatureTransform requires an Ensembl Feature with "
               "an Ensembl Slice attached to it.\n");

        return NULL;
    }

    /*
    ** Use the Ensembl Database Adaptor of the Slice as this Feature may not
    ** yet be stored in the database and may not have its own Adaptor.
    */

    sla = ensSliceGetAdaptor(feature->Slice);

    if (!sla)
    {
        ajWarn("ensFeatureTransform requires an Ensembl Feature with "
               "an Ensembl Slice Adaptor member attached to the "
               "Ensembl Slice member.\n");

        return NULL;
    }

    if (!ensSliceGetCoordsystemObject(feature->Slice))
    {
        ajWarn("ensFeatureTransform requires an Ensembl Feature with "
               "an Ensembl Coordinate System member attached to the "
               "Ensembl Slice member.\n");

        return NULL;
    }

    dba = ensSliceadaptorGetDatabaseadaptor(sla);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);

    if (!cs)
        ajFatal("ensFeatureTransform cannot transform to an unknown "
                "Ensembl Coordinate System '%S:%S'.", csname, csversion);

    /*
    ** If the Ensembl Feature is already in the requested Coordinate System,
    ** and on a regular Sequence Region Slice, we can simply return a copy
    ** of this Feature.
    */

    /*
    ** FIXME: Shouldn't this test also check for the correct Sequence Region
    ** length as the end point of the Slice? Although coordinates would not be
    ** affected in case a Slice was shorter at its end, shouldn't the Slice
    ** cover the entire Sequence Region for consistency?
    ** ensSliceadaptorFetchBySeqregionIdentifier below would return a Slice
    ** covering the full Sequence Region.
    */

    if (ensCoordsystemMatch(cs, ensSliceGetCoordsystemObject(feature->Slice))
        && (ensSliceGetStart(feature->Slice) == 1)
        && (ensSliceGetStrand(feature->Slice) >= 0))
    {
        nfeature = ensFeatureNewRef(feature);

        ensCoordsystemDel(&cs);

        return nfeature;
    }

    /*
    ** If a Coordinate System different from the Feature Coordinate System was
    ** requested, project the Feature into this other Coordinate System, but
    ** place the Feature only, if it projects in one piece i.e does not span
    ** Sequence Region boundaries.
    */

    pss = ajListNew();

    if (slice)
        ensFeatureProjectslice(feature, slice, pss);
    else
        ensFeatureProject(feature, csname, csversion, pss);

    /*
    ** For Ensembl Feature objects that project more than once,
    ** an Ensembl Slice should be specified.
    */

    if (ajListGetLength(pss) <= 1)
    {
        ajListPeekFirst(pss, (void **) &ps);

        if (ps)
            match = ajTrue;
    }
    else if (slice)
    {
        iterator = ajListIterNew(pss);

        while (!ajListIterDone(iterator))
        {
            ps = (EnsPProjectionsegment) ajListIterGet(iterator);

            pslice = ensProjectionsegmentGetTargetSlice(ps);

            if (ensSliceGetSeqregionIdentifier(pslice) ==
                ensSliceGetSeqregionIdentifier(slice))
            {
                match = ajTrue;

                break;
            }
        }

        ajListIterDel(&iterator);
    }

    if (match)
    {
        ensSliceadaptorFetchBySeqregionIdentifier(
            sla,
            ensSliceGetSeqregionIdentifier(pslice),
            0,
            0,
            1,
            &nslice);

        nfeature = ensFeatureNewCpy(feature);

        nfeature->Start  = ensSliceGetStart(pslice);
        nfeature->End    = ensSliceGetEnd(pslice);
        nfeature->Strand = (feature->Strand == 0)
            ? 0 : ensSliceGetStrand(pslice);

        ensFeatureSetSlice(nfeature, nslice);

        ensSliceDel(&nslice);
    }

    if ((match == ajFalse) && (ajListGetLength(pss) > 1))
        ajWarn("ensFeatureTransform got %d Ensembl Projection Segment "
               "objects, but no Ensembl Slice was specified.",
               ajListGetLength(pss));

    while (ajListPop(pss, (void **) &ps))
        ensProjectionsegmentDel(&ps);

    ajListFree(&pss);

    ensCoordsystemDel(&cs);

    return nfeature;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Feature objects
**
** @fdata [EnsPFeature]
**
** @nam3rule Match      Test Ensembl Feature objects for identity
** @nam3rule Overlap    Test Ensembl Feature objects for overlap
** @nam3rule Similarity Test Ensembl Feature objects for similarity
**
** @argrule * feature1 [const EnsPFeature] Ensembl Feature
** @argrule * feature2 [const EnsPFeature] Ensembl Feature
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureMatch ******************************************************
**
** Test two Ensembl Feature objects for identity.
**
** @param [r] feature1 [const EnsPFeature] Ensembl Feature
** @param [r] feature2 [const EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue if the Ensembl Feature objects are equal
**
** @release 6.3.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-sensitive string comparison of the sequence name and
** comparisons of other members are performed.
******************************************************************************/

AjBool ensFeatureMatch(const EnsPFeature feature1,
                       const EnsPFeature feature2)
{
    if (!feature1)
        return ajFalse;

    if (!feature2)
        return ajFalse;

    if (feature1 == feature2)
        return ajTrue;

    /* Ensembl Analysis objects are optional. */

    if ((feature1->Analysis || feature2->Analysis)
        && (!ensAnalysisMatch(feature1->Analysis, feature2->Analysis)))
        return ajFalse;

    if (!ensSliceMatch(feature1->Slice, feature2->Slice))
        return ajFalse;

    /* Sequence names are optional. */

    if ((feature1->Sequencename || feature2->Sequencename)
        && (!ajStrMatchS(feature1->Sequencename, feature2->Sequencename)))
        return ajFalse;

    if (feature1->Start != feature2->Start)
        return ajFalse;

    if (feature1->End != feature2->End)
        return ajFalse;

    if (feature1->Strand != feature2->Strand)
        return ajFalse;

    return ajTrue;
}




/* @func ensFeatureOverlap ****************************************************
**
** Tests two Ensembl Feature objects for overlap.
**
** @cc Bio::EnsEMBL::Feature::overlaps
** @param [r] feature1 [const EnsPFeature] Ensembl Feature
** @param [r] feature2 [const EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue if the Ensembl Feature objects overlap on the same
**                  Ensembl Sequence Region, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureOverlap(const EnsPFeature feature1,
                         const EnsPFeature feature2)
{
    AjPStr name1 = NULL;
    AjPStr name2 = NULL;

    if (!feature1)
        return ajFalse;

    if (!feature2)
        return ajFalse;

    name1 = ajStrNew();
    name2 = ajStrNew();

    ensFeatureFetchSequencename(feature1, &name1);
    ensFeatureFetchSequencename(feature2, &name2);

    if ((name1 && name2) && (!ajStrMatchCaseS(name1, name2)))
    {
        ajDebug("ensFeatureOverlap got Feature objects on different "
                "Ensembl Sequence Regions.\n");

        ajStrDel(&name1);
        ajStrDel(&name2);

        return ajFalse;
    }

    ajStrDel(&name1);
    ajStrDel(&name2);

    return ((ensFeatureGetSeqregionEnd(feature1) >=
             ensFeatureGetSeqregionStart(feature2))
            &&
            (ensFeatureGetSeqregionStart(feature1) <=
             ensFeatureGetSeqregionEnd(feature2)));
}




/* @func ensFeatureSimilarity *************************************************
**
** Test Ensembl Feature objects for similarity.
**
** @cc Bio::EnsEMBL::Feature::equals
** @param [r] feature1 [const EnsPFeature] Ensembl Feature
** @param [r] feature2 [const EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue if the Ensembl Feature objects are equal
**
** @release 6.4.0
** @@
** NOTE: This function is similar to the Bio::EnsEMBL::Feature::equals method,
** but not identical.
** The comparison is based on an initial pointer equality test and if that
** fails, the Ensembl Slice objects are compared for similarity.
** The Ensembl Feature coordinates are compared as absolute Ensembl Sequence
** Region coordinates. If Ensembl Analysis objects have been set, they are
** matched. If sequence names have been set, they are compared
** in a case-sensitive manner.
******************************************************************************/

AjBool ensFeatureSimilarity(const EnsPFeature feature1,
                            const EnsPFeature feature2)
{
    if (!feature1)
        return ajFalse;

    if (!feature2)
        return ajFalse;

    if (feature1 == feature2)
        return ajTrue;

    /* Ensembl Analysis objects are optional. */

    if ((feature1->Analysis || feature2->Analysis)
        && (!ensAnalysisMatch(feature1->Analysis, feature2->Analysis)))
        return ajFalse;

    if (!ensSliceSimilarity(feature1->Slice, feature2->Slice))
        return ajFalse;

    /* Sequence names are optional. */

    if ((feature1->Sequencename || feature2->Sequencename)
        && (!ajStrMatchS(feature1->Sequencename, feature2->Sequencename)))
        return ajFalse;

    /* Compare absolute Ensembl Sequence Region coordinates. */

    if (ensFeatureGetSeqregionStart(feature1) !=
        ensFeatureGetSeqregionStart(feature2))
        return ajFalse;

    if (ensFeatureGetSeqregionEnd(feature1) !=
        ensFeatureGetSeqregionEnd(feature2))
        return ajFalse;

    if (ensFeatureGetSeqregionStrand(feature1) !=
        ensFeatureGetSeqregionStrand(feature2))
        return ajFalse;

    return ajTrue;
}




/* @section comparing *********************************************************
**
** Functions for comparing Ensembl Feature objects
**
** @fdata [EnsPFeature]
**
** @nam3rule Compare    Compare two Ensembl Feature objects
** @nam4rule End        Compare by Ensembl Feature end members
** @nam4rule Start      Compare by Ensembl Feature start members
** @nam5rule Ascending  Compare in ascending order
** @nam5rule Descending Compare in descending order
**
** @argrule * feature1 [const EnsPFeature] Ensembl Feature 1
** @argrule * feature2 [const EnsPFeature] Ensembl Feature 2
**
** @valrule * [int] The comparison function returns an integer less than,
**                  equal to, or greater than zero if the first argument is
**                  considered to be respectively less than, equal to, or
**                  greater than the second.
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureCompareEndAscending ****************************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** end member in ascending order.
**
** Ensembl Feature objects based on Ensembl Slice objects sort before
** Ensembl Feature objects based on sequence names.
** Ensembl Feature objects without Ensembl Slice objects or sequence names
** sort towards the end of the AJAX List.
**
** @param [r] feature1 [const EnsPFeature] Ensembl Feature 1
** @param [r] feature2 [const EnsPFeature] Ensembl Feature 2
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

int ensFeatureCompareEndAscending(const EnsPFeature feature1,
                                  const EnsPFeature feature2)
{
    int result = 0;

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    /*
    ** Ensembl Feature objects based on Ensembl Slices sort before
    ** Ensembl Feature objects based on sequence names.
    ** For Ensembl Feature objects based on identical Slice objects or
    ** sequence names evaluate start coordinates.
    */

    if (feature1->Slice && feature2->Sequencename)
        return -1;

    if (feature1->Slice && feature2->Slice &&
        (result = ensSliceCompareIdentifierAscending(feature1->Slice,
                                                     feature2->Slice)))
        return result;

    if (feature1->Sequencename && feature2->Sequencename &&
        (result = ajStrCmpS(feature1->Sequencename,
                            feature2->Sequencename)))
        return result;

    if (feature1->Sequencename && feature2->Slice)
        return +1;

    /* No decision yet, evaluate Feature end coordinates. */

    if (feature1->End < feature2->End)
        return -1;

    if (feature1->End > feature2->End)
        return +1;

#if AJFALSE
    /* No decision yet, evaluate Feature start coordinates. */

    if (feature1->Start < feature2->Start)
        return -1;

    if (feature1->Start > feature2->Start)
        return +1;
#endif /* AJFALSE */

    return 0;
}




/* @func ensFeatureCompareEndDescending ***************************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** end member in descending order.
**
** Ensembl Feature objects based on Ensembl Slice objects sort before
** Ensembl Feature objects based on sequence names.
** Ensembl Feature objects without Ensembl Slice objects or sequence names
** sort towards the end of the AJAX List.
**
** @param [r] feature1 [const EnsPFeature] Ensembl Feature 1
** @param [r] feature2 [const EnsPFeature] Ensembl Feature 2
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

int ensFeatureCompareEndDescending(const EnsPFeature feature1,
                                   const EnsPFeature feature2)
{
    int result = 0;

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    /*
    ** Ensembl Feature objects based on Ensembl Slice objects sort before
    ** Ensembl Feature objects based on sequence names.
    ** For Ensembl Feature objects based on identical Slice objects or
    ** sequence names evaluate start coordinates.
    */

    if (feature1->Slice && feature2->Sequencename)
        return -1;

    if (feature1->Slice && feature2->Slice &&
        (result = ensSliceCompareIdentifierAscending(feature1->Slice,
                                                     feature2->Slice)))
        return result;

    if (feature1->Sequencename && feature2->Sequencename &&
        (result = ajStrCmpS(feature1->Sequencename,
                            feature2->Sequencename)))
        return result;

    if (feature1->Sequencename && feature2->Slice)
        return +1;

    /* No decision yet, evaluate Feature end coordinates. */

    if (feature1->End < feature2->End)
        return +1;

    if (feature1->End < feature2->End)
        return -1;

#if AJFALSE
    /* No decision yet, evaluate Feature start coordinates. */

    if (feature1->Start < feature2->Start)
        return +1;

    if (feature1->Start < feature2->Start)
        return -1;
#endif /* AJFALSE */

    return 0;
}




/* @func ensFeatureCompareStartAscending **************************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** start member in ascending order.
**
** Ensembl Feature objects based on Ensembl Slice objects sort before
** Ensembl Feature objects based on sequence names.
** Ensembl Feature objects without Ensembl Slice objects or sequence names
** sort towards the end of the AJAX List.
**
** @param [r] feature1 [const EnsPFeature] Ensembl Feature 1
** @param [r] feature2 [const EnsPFeature] Ensembl Feature 2
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

int ensFeatureCompareStartAscending(const EnsPFeature feature1,
                                    const EnsPFeature feature2)
{
    int result = 0;

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    /*
    ** Ensembl Feature objects based on Ensembl Slices sort before
    ** Ensembl Feature objects based on sequence names.
    ** For Ensembl Feature objects based on identical Slice objects or
    ** sequence names evaluate start coordinates.
    */

    if (feature1->Slice && feature2->Sequencename)
        return -1;

    if (feature1->Slice && feature2->Slice &&
        (result = ensSliceCompareIdentifierAscending(feature1->Slice,
                                                     feature2->Slice)))
        return result;

    if (feature1->Sequencename && feature2->Sequencename &&
        (result = ajStrCmpS(feature1->Sequencename,
                            feature2->Sequencename)))
        return result;

    if (feature1->Sequencename && feature2->Slice)
        return +1;

    /* No decision yet, evaluate Feature start coordinates. */

    if (feature1->Start < feature2->Start)
        return -1;

    if (feature1->Start > feature2->Start)
        return +1;

#if AJFALSE
    /* No decision yet, evaluate Feature end coordinates. */

    if (feature1->End < feature2->End)
        return -1;

    if (feature1->End > feature2->End)
        return +1;
#endif /* AJFALSE */

    return 0;
}




/* @func ensFeatureCompareStartDescending *************************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** start member in descending order.
**
** Ensembl Feature objects based on Ensembl Slice objects sort before
** Ensembl Feature objects based on sequence names.
** Ensembl Feature objects without Ensembl Slice objects or sequence names
** sort towards the end of the AJAX List.
**
** @param [r] feature1 [const EnsPFeature] Ensembl Feature 1
** @param [r] feature2 [const EnsPFeature] Ensembl Feature 2
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

int ensFeatureCompareStartDescending(const EnsPFeature feature1,
                                     const EnsPFeature feature2)
{
    int result = 0;

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    /*
    ** Ensembl Feature objects based on Ensembl Slice objects sort before
    ** Ensembl Feature objects based on sequence names.
    ** For Ensembl Feature objects based on identical Slice objects or
    ** sequence names evaluate start coordinates.
    */

    if (feature1->Slice && feature2->Sequencename)
        return -1;

    if (feature1->Slice && feature2->Slice &&
        (result = ensSliceCompareIdentifierAscending(feature1->Slice,
                                                     feature2->Slice)))
        return result;

    if (feature1->Sequencename && feature2->Sequencename &&
        (result = ajStrCmpS(feature1->Sequencename,
                            feature2->Sequencename)))
        return result;

    if (feature1->Sequencename && feature2->Slice)
        return +1;

    /* No decision yet, evaluate Feature start coordinates. */

    if (feature1->Start < feature2->Start)
        return +1;

    if (feature1->Start > feature2->Start)
        return -1;

#if AJFALSE
    /* No decision yet, evaluate Feature end coordinates. */

    if (feature1->End < feature2->End)
        return +1;

    if (feature1->End > feature2->End)
        return -1;
#endif /* AJFALSE */

    return 0;
}




/* @funcstatic listFeatureCompareEndAscending *********************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Feature address 1
** @param [r] item2 [const void*] Ensembl Feature address 2
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

static int listFeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPFeature feature1 = *(EnsOFeature *const *) item1;
    EnsPFeature feature2 = *(EnsOFeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listFeatureCompareEndAscending"))
    {
        ajDebug("listFeatureCompareEndAscending\n"
                "  feature1 %p\n"
                "  feature2 %p\n",
                feature1,
                feature2);

        ensFeatureTrace(feature1, 1);
        ensFeatureTrace(feature2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    return ensFeatureCompareEndAscending(feature1, feature2);
}




/* @funcstatic listFeatureCompareEndDescending ********************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Feature address 1
** @param [r] item2 [const void*] Ensembl Feature address 2
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

static int listFeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPFeature feature1 = *(EnsOFeature *const *) item1;
    EnsPFeature feature2 = *(EnsOFeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listFeatureCompareEndDescending"))
    {
        ajDebug("listFeatureCompareEndDescending\n"
                "  feature1 %p\n"
                "  feature2 %p\n",
                feature1,
                feature2);

        ensFeatureTrace(feature1, 1);
        ensFeatureTrace(feature2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    return ensFeatureCompareEndDescending(feature1, feature2);
}




/* @funcstatic listFeatureCompareStartAscending *******************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Feature address 1
** @param [r] item2 [const void*] Ensembl Feature address 2
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

static int listFeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPFeature feature1 = *(EnsOFeature *const *) item1;
    EnsPFeature feature2 = *(EnsOFeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listFeatureCompareStartAscending"))
    {
        ajDebug("listFeatureCompareStartAscending\n"
                "  feature1 %p\n"
                "  feature2 %p\n",
                feature1,
                feature2);

        ensFeatureTrace(feature1, 1);
        ensFeatureTrace(feature2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    return ensFeatureCompareStartAscending(feature1, feature2);
}




/* @funcstatic listFeatureCompareStartDescending ******************************
**
** AJAX List of Ensembl Feature objects comparison function to sort by
** start coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Feature address 1
** @param [r] item2 [const void*] Ensembl Feature address 2
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

static int listFeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPFeature feature1 = *(EnsOFeature *const *) item1;
    EnsPFeature feature2 = *(EnsOFeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listFeatureCompareStartDescending"))
    {
        ajDebug("listFeatureCompareStartDescending\n"
                "  feature1 %p\n"
                "  feature2 %p\n",
                feature1,
                feature2);

        ensFeatureTrace(feature1, 1);
        ensFeatureTrace(feature2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (feature1 && (!feature2))
        return -1;

    if ((!feature1) && (!feature2))
        return 0;

    if ((!feature1) && feature2)
        return +1;

    return ensFeatureCompareStartDescending(feature1, feature2);
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
** @nam3rule Feature Functions for manipulating AJAX List objects of
** Ensembl Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * features [AjPList]
** AJAX List of Ensembl Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListFeatureSortEndAscending ***************************************
**
** Sort an AJAX List of Ensembl Feature objects by their end member in
** ascending order.
**
** @param [u] features [AjPList] AJAX List of Ensembl Feature objects
** @see ensFeatureCompareEndAscending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeatureSortEndAscending(AjPList features)
{
    if (!features)
        return ajFalse;

    ajListSortTwo(features,
                  &listFeatureCompareEndAscending,
                  &listFeatureCompareStartAscending);

    return ajTrue;
}




/* @func ensListFeatureSortEndDescending **************************************
**
** Sort an AJAX List of Ensembl Feature objects by their end member in
** descending order.
**
** @param [u] features [AjPList] AJAX List of Ensembl Feature objects
** @see ensFeatureCompareEndDescending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeatureSortEndDescending(AjPList features)
{
    if (!features)
        return ajFalse;

    ajListSortTwo(features,
                  &listFeatureCompareEndDescending,
                  &listFeatureCompareStartDescending);

    return ajTrue;
}




/* @func ensListFeatureSortStartAscending *************************************
**
** Sort an AJAX List of Ensembl Feature objects by their start member in
** ascending order.
**
** @param [u] features [AjPList] AJAX List of Ensembl Feature objects
** @see ensFeatureCompareStartAscending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeatureSortStartAscending(AjPList features)
{
    if (!features)
        return ajFalse;

    ajListSortTwo(features,
                  &listFeatureCompareStartAscending,
                  &listFeatureCompareEndAscending);

    return ajTrue;
}




/* @func ensListFeatureSortStartDescending ************************************
**
** Sort an AJAX List of Ensembl Feature objects by their start member in
** descending order.
**
** @param [u] features [AjPList] AJAX List of Ensembl Feature objects
** @see ensFeatureCompareStartDescending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeatureSortStartDescending(AjPList features)
{
    if (!features)
        return ajFalse;

    ajListSortTwo(features,
                  &listFeatureCompareStartDescending,
                  &listFeatureCompareEndDescending);

    return ajTrue;
}




/* @datasection [EnsPFeatureadaptor] Ensembl Feature Adaptor ******************
**
** @nam2rule Featureadaptor Functions for manipulating
** Ensembl Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor
** @cc CVS Revision: 1.119
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Feature Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule New Ptables [const char* const*] Table names
** @argrule New Pcolumns [const char* const*] Column names
** @argrule New leftjoin [const EnsPBaseadaptorLeftjoin]
** Ensembl Base Adaptor Left Join conditions
** @argrule New condition [const char*] SQL SELECT default condition
** @argrule New final [const char*] SQL SELECT final condition
** @argrule New Fstatement [AjBool function] Statement function address
** @argrule New Fread [void* function] Read function address
** @argrule New Freference [void* function] Reference function address
** @argrule New Fwrite [AjBool function] Write function
** @argrule New Fdelete [void function] Delete function address
** @argrule New Fsize [size_t function] Size function address
** @argrule New Fgetfeature [EnsPFeature function] Get Feature function address
** @argrule New label [const char*] Ensembl Cache label
**
** @valrule * [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensFeatureadaptorNew *************************************************
**
** Default constructor for an Ensembl Feature Adaptor.
**
** For Ensembl Collection Core databases storing information about multiple
** species, only Ensembl Feature objects for a particular species encoded in
** the Ensembl Database Adaptor need to be selected.
** This is achieved by automatically adding additional joins to the
** 'seq_region' and the 'coord_system' table, which contains the
** 'coord_system.species_id' field to the default SQL condition.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] Ptables [const char* const*] Table names
** @param [r] Pcolumns [const char* const*] Column names
** @param [rN] leftjoin [const EnsPBaseadaptorLeftjoin]
** Ensembl Base Adaptor Left Join conditions
** @param [rN] condition [const char*] SQL SELECT default condition
** @param [rN] final [const char*] SQL SELECT final condition
** @param [f] Fstatement [AjBool function] Statement function address
** @param [fN] Fread [void* function] Read function address
** @param [fN] Freference [void* function] Reference function address
** @param [fN] Fwrite [AjBool function] Write function address
** @param [fN] Fdelete [void function] Delete function address
** @param [fN] Fsize [size_t function] Size function address
** @param [f] Fgetfeature [EnsPFeature function] Get Feature function address
** @param [r] label [const char*] Ensembl Cache label
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.2.0
** @@
** NOTE: For Ensembl Feature objects stored in Ensembl Core databases with
** multiple species, the Sequence Region in a '*_feature' table needs joining
** to the 'seq_region' table and the 'coord_system' table, which holds the
** species identifier field.
** TODO: The Perl API adds the constraint as extra default where clause in the
** Base Adaptor. This is clearly not the right place as not every Object the
** Base Adaptor fetches is a Feature. Feature objects are those objects that
** are associated with a Sequence Region so that the Sequence Region constraint
** is more natural here.
** TODO: Suggest to the Ensembl Core team!
** TODO: Separate this function into an ensFeatureadaptorNew and
** ensFeatureadaptorNewCache function, since not all Ensembl Feature Adaptors
** require an Ensembl Cache.
******************************************************************************/

EnsPFeatureadaptor ensFeatureadaptorNew(
    EnsPDatabaseadaptor dba,
    const char* const* Ptables,
    const char* const* Pcolumns,
    const EnsPBaseadaptorLeftjoin leftjoin,
    const char *condition,
    const char *final,
    AjBool (*Fstatement) (EnsPBaseadaptor ba,
                          const AjPStr statement,
                          EnsPAssemblymapper am,
                          EnsPSlice slice,
                          AjPList objects),
    void* (*Fread) (const void *key),
    void* (*Freference) (void *value),
    AjBool (*Fwrite) (const void *value),
    void (*Fdelete) (void **Pvalue),
    size_t (*Fsize) (const void *value),
    EnsPFeature (*Fgetfeature) (const void *object),
    const char *label)
{
    register ajuint i = 0U;

    EnsPFeatureadaptor fa = NULL;

    if (ajDebugTest("ensFeatureadaptorNew"))
        ajDebug("ensFeatureadaptorNew\n"
                "  dba %p\n"
                "  Ptables %p\n"
                "  Pcolumns %p\n"
                "  leftjoin %p\n"
                "  condition %p\n"
                "  final %p\n"
                "  Fstatement %p\n"
                "  Fread %p\n"
                "  Freference %p\n"
                "  Fwrite %p\n"
                "  Fdelete %p\n"
                "  Fsize %p\n"
                "  Fgetfeature %p\n"
                "  label '%s'\n",
                dba,
                Ptables,
                Pcolumns,
                leftjoin,
                condition,
                final,
                Fstatement,
                Fread,
                Freference,
                Fwrite,
                Fdelete,
                Fsize,
                Fgetfeature,
                label);

    if (!dba)
        return NULL;

    if (!Ptables)
        return NULL;

    if (!Pcolumns)
        return NULL;

    if (!Fstatement)
        return NULL;

    if (!Fgetfeature)
        return NULL;

    AJNEW0(fa);

    if (ensDatabaseadaptorGetMultispecies(dba))
    {
        /*
        ** For Ensembl collection (multi-species) databases, allocate an array
        ** of SQL table names extended with 'seq_region' and 'coord_system'.
        ** This array, instead of the one provided as parameter
        ** (const char *const *Ptables) will then be passed into the
        ** Ensembl Base Adaptor via the ensBaseadaptorNew function.
        */

        for (i = 0U; Ptables[i]; i++);

        fa->Tables = AJCALLOC0(i + 1U + 2U, sizeof (char *));

        for (i = 0U; Ptables[i]; i++)
            fa->Tables[i] = ajCharNewC(Ptables[i]);

        fa->Tables[i] = ajCharNewC("seq_region");
        i++;
        fa->Tables[i] = ajCharNewC("coord_system");
        i++;
        fa->Tables[i] = (char *) NULL;

        Ptables = (const char* const *) fa->Tables;

        /*
        ** Allocate the default SQL condition and extend for 'seq_region' and
        ** 'coord_system' conditions. This character string, instead of the one
        ** provided here (const char *condition) will then be used by the
        ** Ensembl Base Adaptor via ensBaseadaptorNew.
        */

        if (condition)
            fa->Condition = ajFmtString(
                "%s "
                "AND "
                "%s.seq_region_id = seq_region.seq_region_id "
                "AND "
                "seq_region.coord_system_id = coord_system.coord_system_id "
                "AND "
                "coord_system.species_id = %u",
                condition,
                Ptables[0],
                ensDatabaseadaptorGetIdentifier(dba));
        else
            fa->Condition = ajFmtString(
                "%s.seq_region_id = "
                "seq_region.seq_region_id "
                "AND "
                "seq_region.coord_system_id = coord_system.coord_system_id "
                "AND "
                "coord_system.species_id = %u",
                Ptables[0],
                ensDatabaseadaptorGetIdentifier(dba));

        condition = (const char *) fa->Condition;
    }

    fa->Adaptor = ensBaseadaptorNew(
        dba,
        Ptables,
        Pcolumns,
        leftjoin,
        condition,
        final,
        Fstatement);

    fa->Cache = ensCacheNew(
        ensECacheTypeNumeric,
        featureadaptorKCacheMaxBytes,
        featureadaptorKCacheMaxCount,
        featureadaptorKCacheMaxSize,
        Freference,
        Fdelete,
        Fsize,
        Fread,
        Fwrite,
        ajFalse,
        label);

    fa->FobjectGetFeature = Fgetfeature;
    fa->Freference        = Freference;
    fa->Fdelete           = Fdelete;

    fa->Startequalsend = ajFalse;
    fa->Maximumlength  = 0;

    return fa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Feature Adaptor object.
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Feature Adaptor
**
** @argrule * Pfa [EnsPFeatureadaptor*] Ensembl Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensFeatureadaptorDel *************************************************
**
** Default destructor for an Ensembl Feature Adaptor.
**
** @param [d] Pfa [EnsPFeatureadaptor*] Ensembl Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensFeatureadaptorDel(EnsPFeatureadaptor *Pfa)
{
    register ajuint i = 0U;

    EnsPFeatureadaptor pthis = NULL;

    if (!Pfa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensFeatureadaptorDel"))
        ajDebug("ensFeatureadaptorDel\n"
                "  *Pfa %p\n",
                *Pfa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pfa)
        return;

    pthis = *Pfa;

    ensBaseadaptorDel(&pthis->Adaptor);

    ensCacheDel(&pthis->Cache);

    /* Clear the array of SQL table names. */

    if (pthis->Tables)
    {
        for (i = 0U; pthis->Tables[i]; i++)
            ajCharDel(&pthis->Tables[i]);

        AJFREE(pthis->Tables);
    }

    /* Clear the default SQL condition. */

    if (pthis->Condition)
        ajCharDel(&pthis->Condition);

    AJFREE(pthis);

    *Pfa = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Feature Adaptor object.
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule Get Return Ensembl Feature Adaptor attribute(s)
** @nam4rule Baseadaptor      Return the Ensembl Base Adaptor
** @nam4rule Cache            Return the Ensembl Cache
** @nam4rule Databaseadaptor  Return the Ensembl Database Adaptor
** @nam4rule Maximumlength    Return the maximum Feature length
** @nam4rule Startequalsend   Return the start-equals-end flag
**
** @argrule * fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Cache [EnsPCache] Ensembl Cache or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Maximumlength [ajint] Maximum Ensembl Feature length or 0
** @valrule Startequalsend [AjBool] Start-equals-end flag or ajFalse
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureadaptorGetBaseadaptor **************************************
**
** Get the Ensembl Base Adaptor member of an Ensembl Feature Adaptor.
**
** @param [r] fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBaseadaptor ensFeatureadaptorGetBaseadaptor(
    const EnsPFeatureadaptor fa)
{
    return (fa) ? fa->Adaptor : NULL;
}




/* @func ensFeatureadaptorGetCache ********************************************
**
** Get the Ensembl Cache member of an Ensembl Feature Adaptor.
**
** @param [r] fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [EnsPCache] Ensembl Cache or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPCache ensFeatureadaptorGetCache(
    const EnsPFeatureadaptor fa)
{
    return (fa) ? fa->Cache : NULL;
}




/* @func ensFeatureadaptorGetDatabaseadaptor **********************************
**
** Get the Ensembl Database Adaptor member of the
** Ensembl Base Adaptor member of an Ensembl Feature Adaptor.
**
** @param [r] fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensFeatureadaptorGetDatabaseadaptor(
    const EnsPFeatureadaptor fa)
{
    return (fa) ? ensBaseadaptorGetDatabaseadaptor(fa->Adaptor) : NULL;
}




/* @func ensFeatureadaptorGetMaximumlength ************************************
**
** Get the maximum length member of an Ensembl Feature Adaptor.
**
** @param [r] fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [ajint] Maximum length or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensFeatureadaptorGetMaximumlength(
    const EnsPFeatureadaptor fa)
{
    return (fa) ? fa->Maximumlength : 0;
}




/* @func ensFeatureadaptorGetStartequalsend ***********************************
**
** Get the start-equals-end flag member of an Ensembl Feature Adaptor.
**
** @param [r] fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [AjBool] Start-equals-end flag or ajFalse
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorGetStartequalsend(
    const EnsPFeatureadaptor fa)
{
    return (fa) ? fa->Startequalsend : ajFalse;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Feature Adaptor object.
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule Set Set one member of an Ensembl Feature Adaptor
** @nam4rule Columns Set the column names
** @nam4rule Defaultcondition Set the SQL SELECT default condition
** @nam4rule Finalcondition Set the SQL SELECT final condition
** @nam4rule Maximumlength Set the maximum Feature length
** @nam4rule Startequalsend Set the start-equals-end flag
** @nam4rule Tables Set the table names
**
** @argrule * fa [EnsPFeatureadaptor] Ensembl Feature Adaptor object
** @argrule Columns Pcolumns [const char* const*] Column names
** @argrule Defaultcondition condition [const char*]
** SQL SELECT default condition
** @argrule Finalcondition final [const char*]
** SQL SELECT final condition
** @argrule Maximumlength length [ajint] Maximum length
** @argrule Startequalsend flag [AjBool] Start-equals-end flag
** @argrule Tables Ptables [const char* const*] Table names
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensFeatureadaptorSetColumns ******************************************
**
** Set the columns member of the Ensembl Base Adaptor member of an
** Ensembl Feature Adaptor.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] Pcolumns [const char* const*] Column names
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetColumns(EnsPFeatureadaptor fa,
                                   const char* const* Pcolumns)
{
    if (!fa)
        return ajFalse;

    if (!Pcolumns)
        return ajFalse;

    return ensBaseadaptorSetColumns(fa->Adaptor, Pcolumns);
}




/* @func ensFeatureadaptorSetDefaultcondition *********************************
**
** Set the SQL SELECT default condition member of the
** Ensembl Base Adaptor member of an Ensembl Feature Adaptor.
**
** For Ensembl Collection Core databases storing information about multiple
** species, only Ensembl Feature objects for a particular species encoded in
** the Ensembl Database Adaptor need to be selected.
** This is achieved by automatically adding additional joins to the
** 'seq_region' and the 'coord_system' table, which contains the
** 'coord_system.species_id' field to the default SQL condition.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] condition [const char*] SQL SELECT default condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetDefaultcondition(EnsPFeatureadaptor fa,
                                            const char *condition)
{
    EnsPDatabaseadaptor dba = NULL;

    if (!fa)
        return ajFalse;

    if (ensDatabaseadaptorGetMultispecies(dba))
    {
        /* Clear the SQL SELECT default condition. */

        if (fa->Condition)
            ajCharDel(&fa->Condition);

        /*
        ** Allocate the SQL SELECT default condition and extend for
        ** 'seq_region' and 'coord_system' conditions. This character string,
        ** instead of the one provided here (const char *condition) will then
        ** be set in the Ensembl Base Adaptor via
        ** ensBaseadaptorSetDefaultcondition.
        */

        if (condition)
            fa->Condition = ajFmtString(
                "%s "
                "AND "
                "%s.seq_region_id = seq_region.seq_region_id "
                "AND "
                "seq_region.coord_system_id = coord_system.coord_system_id "
                "AND "
                "coord_system.species_id = %u",
                condition,
                ensBaseadaptorGetPrimarytable(fa->Adaptor),
                ensDatabaseadaptorGetIdentifier(dba));
        else
            fa->Condition = ajFmtString(
                "%s.seq_region_id = seq_region.seq_region_id "
                "AND "
                "seq_region.coord_system_id = coord_system.coord_system_id "
                "AND "
                "coord_system.species_id = %u",
                ensBaseadaptorGetPrimarytable(fa->Adaptor),
                ensDatabaseadaptorGetIdentifier(dba));

        condition = (const char *) fa->Condition;
    }

    return ensBaseadaptorSetDefaultcondition(fa->Adaptor, condition);
}




/* @func ensFeatureadaptorSetFinalcondition ***********************************
**
** Set the final condition (SQL SELECT) member of the
** Ensembl Base Adaptor member of an Ensembl Base Adaptor.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] final [const char*] Final SQL condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetFinalcondition(EnsPFeatureadaptor fa,
                                          const char *final)
{
    if (!fa)
        return ajFalse;

    return ensBaseadaptorSetFinalcondition(fa->Adaptor, final);
}




/* @func ensFeatureadaptorSetMaximumlength ************************************
**
** Set the maximum length member of an Ensembl Feature Adaptor.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] length [ajint] Maximum length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetMaximumlength(EnsPFeatureadaptor fa,
                                         ajint length)
{
    if (!fa)
        return ajFalse;

    fa->Maximumlength = length;

    return ajTrue;
}




/* @func ensFeatureadaptorSetStartequalsend ***********************************
**
** Set the start-equals-end flag member of an Ensembl Feature Adaptor.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] flag [AjBool] Start-equals-end flag
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetStartequalsend(EnsPFeatureadaptor fa,
                                          AjBool flag)
{
    if (!fa)
        return ajFalse;

    fa->Startequalsend = flag;

    return ajTrue;
}




/* @func ensFeatureadaptorSetTables *******************************************
**
** Set the tables member of the Ensembl Base Adaptor member of an
** Ensembl Feature Adaptor.
**
** For Ensembl Collection Core databases storing information about multiple
** species, only Ensembl Feature objects for a particular species encoded in
** the Ensembl Database Adaptor need to be selected.
** This is achieved by automatically adding additional joins to the
** 'seq_region' and the 'coord_system' table, which contains the
** 'coord_system.species_id' field to the SQL SELECT default condition.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] Ptables [const char* const*] Table names
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetTables(EnsPFeatureadaptor fa,
                                  const char* const* Ptables)
{
    register ajuint i = 0U;

    EnsPDatabaseadaptor dba = NULL;

    if (!fa)
        return ajFalse;

    if (!Ptables)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(fa);

    if (ensDatabaseadaptorGetMultispecies(dba))
    {
        /* Clear the array of table names. */

        if (fa->Tables)
        {
            for (i = 0U; fa->Tables[i]; i++)
                ajCharDel(&fa->Tables[i]);

            AJFREE(fa->Tables);
        }

        /*
        ** Allocate an array of SQL table names extended for 'seq_region' and
        ** 'coord_system' tables. This array, instead of the one provided here
        ** (const char *const *Ptables) will then be set in the
        ** Ensembl Base Adaptor via ensBaseadaptorSetTables.
        */

        for (i = 0U; Ptables[i]; i++);

        fa->Tables = AJCALLOC0(i + 1U + 2U, sizeof (char *));

        for (i = 0U; Ptables[i]; i++)
            fa->Tables[i] = ajCharNewC(Ptables[i]);

        fa->Tables[i] = ajCharNewC("seq_region");
        i++;
        fa->Tables[i] = ajCharNewC("coord_system");
        i++;
        fa->Tables[i] = (char *) NULL;

        Ptables = (const char* const *) fa->Tables;
    }

    return ensBaseadaptorSetTables(fa->Adaptor, Ptables);
}




/* @section SQL character escaping ********************************************
**
** Ensembl Feature Adaptor SQL character escaping convenience functions
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule Escape Escape strings
** @nam4rule C Escape an AJAX String
** @nam4rule S Escape a C-type character string
**
** @argrule * fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @argrule EscapeC Ptxt [char**] Address of the (new) SQL-escaped C string
** @argrule EscapeC str [const AjPStr] AJAX String to be escaped
** @argrule EscapeS Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @argrule EscapeS str [const AjPStr] AJAX String to be escaped
**
** @valrule EscapeC [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule EscapeS [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureadaptorEscapeC *********************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return a C-type character string.
**
** The caller is responsible for deleting the escaped C-type character string.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorEscapeC(EnsPFeatureadaptor fa,
                                char **Ptxt,
                                const AjPStr str)
{
    if (!fa)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensFeatureadaptorEscapeC"))
        ajDebug("ensFeatureadaptorEscapeC\n"
                "  fa %p\n"
                "  Ptxt %p\n"
                "  str '%S'\n",
                fa,
                Ptxt,
                str);

    return ensBaseadaptorEscapeC(fa->Adaptor, Ptxt, str);
}




/* @func ensFeatureadaptorEscapeS *********************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return an AJAX String.
**
** The caller is responsible for deleting the escaped AJAX String.
**
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorEscapeS(EnsPFeatureadaptor fa,
                                AjPStr *Pstr,
                                const AjPStr str)
{
    if (!fa)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensFeatureadaptorEscapeS"))
        ajDebug("ensFeatureadaptorEscapeS\n"
                "  fa %p\n"
                "  Pstr %p\n"
                "  str '%S'\n",
                fa,
                Pstr,
                str);

    return ensBaseadaptorEscapeS(fa->Adaptor, Pstr, str);
}




/* @section SQL statement constraint ******************************************
**
** Ensembl Feature Adaptor SQL statement constraint functions
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule Constraint Manipulate SQL statement constraints
** @nam4rule Append Append a condition to an SQL statement constraint
** @nam5rule Analysisname Ensembl Analysis name
**
** @argrule * fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
** @argrule Constraint Pconstraint [AjPStr*] SQL constraint address
** @argrule Analysisname anname [const AjPStr] Ensembl Analysis name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureadaptorConstraintAppendAnalysisname ************************
**
** Append an Ensembl Analysis condition to an SQL statement constraint via an
** Ensembl Analysis name.
**
** The caller is responsible for deleting the SQL statement constraint.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::_logic_name_to_constraint
** @param [r] fa [const EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [uN] Pconstraint [AjPStr*] SQL statement constraint address
** @param [rN] anname [const AjPStr] Ensembl Analysis name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** Given an Ensembl Analysis name and an existing constraint this will
** add an 'analysis' table constraint to the Feature. Note that if no
** 'analysis_id' exists in the columns of the primary table then no
** constraint is added at all.
******************************************************************************/

AjBool ensFeatureadaptorConstraintAppendAnalysisname(
    const EnsPFeatureadaptor fa,
    AjPStr *Pconstraint,
    const AjPStr anname)
{
    const char *const *columns = NULL;
    const char        *table   = NULL;

    register ajuint i = 0U;

    AjBool match = AJFALSE;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!fa)
        return ajFalse;

    if (!Pconstraint)
        return ajFalse;

    if (!anname)
        return ajTrue;

    if (ajDebugTest("ensFeatureadaptorConstraintAppendAnalysisname"))
        ajDebug("ensFeatureadaptorConstraintAppendAnalysisname\n"
                "  fa %p\n"
                "  *Pconstraint '%S'\n"
                "  anname '%S'\n",
                fa,
                *Pconstraint,
                anname);

    /*
    ** Check that the primary table, which is the first one in the list of
    ** tables (fa->Adaptor->Tables[0]), actually contains an 'analysis_id'
    ** column.
    */

    columns = ensBaseadaptorGetColumns(fa->Adaptor);

    table = ensBaseadaptorGetPrimarytable(fa->Adaptor);

    while (columns[i])
    {
        if (ajCharPrefixC(columns[i], table) &&
            ajCharSuffixC(columns[i], ".analysis_id"))
            match = ajTrue;

        i++;
    }

    if (!match)
    {
        ajWarn("ensFeatureadaptorConstraintAppendAnalysisname called for an "
               "Ensembl Feature, which is not associated with an "
               "Ensembl Analysis. Ignoring Analysis name argument '%S'.\n",
               anname);

        return ajFalse;
    }

    dba = ensFeatureadaptorGetDatabaseadaptor(fa);

    aa = ensRegistryGetAnalysisadaptor(dba);

    ensAnalysisadaptorFetchByName(aa, anname, &analysis);

    if (!analysis)
        return ajFalse;

    if (*Pconstraint && ajStrGetLen(*Pconstraint))
        ajStrAppendC(Pconstraint, " AND ");
    else
        *Pconstraint = ajStrNew();

    ajFmtPrintAppS(Pconstraint,
                   "%s.analysis_id = %u",
                   table,
                   ensAnalysisGetIdentifier(analysis));

    ensAnalysisDel(&analysis);

    return ajTrue;
}




/* @funcstatic featureadaptorRemap ********************************************
**
** Remap Ensembl Objects based on Ensembl Feature objects onto an
** Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::_remap
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Feature objects
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool featureadaptorRemap(EnsPFeatureadaptor fa,
                                  AjPList objects,
                                  EnsPAssemblymapper am,
                                  EnsPSlice slice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    ajuint srid = 0U;

    void *Pobject = NULL;

    AjBool debug = AJFALSE;

    AjIList iter = NULL;
    AjPList mrs  = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    debug = ajDebugTest("featureadaptorRemap");

    if (debug)
        ajDebug("featureadaptorRemap\n"
                "  fa %p\n"
                "  objects %p\n"
                "  am %p\n"
                "  slice %p\n",
                fa,
                objects,
                am,
                slice);

    if (!fa)
        return ajFalse;

    if (!objects)
        return ajFalse;

    if (!slice)
        return ajFalse;

    /*
    ** Remapping is not required, if the AJAX List is empty or the Slice
    ** attached to the first Feature is already identical to the Slice
    ** the Feature objects should be mapped to.
    */

    if (!ajListGetLength(objects))
        return ajTrue;

    ajListPeekFirst(objects, (void **) &Pobject);

    feature = (*fa->FobjectGetFeature) (Pobject);

    if (ensSliceMatch(ensFeatureGetSlice(feature), slice))
        return ajTrue;

    /* Remapping has not been done, we have to do our own conversion. */

    mrs = ajListNew();

    iter = ajListIterNew(objects);

    while (!ajListIterDone(iter))
    {
        Pobject = ajListIterGet(iter);

        feature = (*fa->FobjectGetFeature) (Pobject);

        /*
        ** Since Ensembl Feature objects were obtained in contig coordinates,
        ** the attached Sequence Region is a contig.
        */

        if (!feature->Slice)
            ajFatal("featureadaptorRemap got an Ensembl Feature (%p) "
                    "without an Ensembl Slice.\n", feature);

        if (ensCoordsystemMatch(ensSliceGetCoordsystemObject(slice),
                                ensSliceGetCoordsystemObject(feature->Slice)))
        {
            /*
            ** The Slice attached to the Feature is in the same
            ** Coordinate System as the target Slice, therefore remapping and
            ** an Ensembl Assembly Mapper are not required. Nevertheless,
            ** coordinates need still adjusting to the Slice.
            */

            srid   = ensSliceGetSeqregionIdentifier(feature->Slice);
            start  = feature->Start;
            end    = feature->End;
            strand = feature->Strand;
        }
        else
        {
            /*
            ** The Slice attached to the Feature is in a different
            ** Coordinate System, therefore remapping is required.
            */

            if (!am)
                ajFatal("featureadaptorRemap requires an "
                        "Ensembl Assembly Mapper, when "
                        "Coordinate Systems of Feature objects and Slice "
                        "differ.\n");

            ensAssemblymapperFastmap(am,
                                     ensSliceGetSeqregion(feature->Slice),
                                     feature->Start,
                                     feature->End,
                                     feature->Strand,
                                     mrs);

            /*
            ** The ensMapperFastmap function returns at maximum one Ensembl
            ** Mapper Result. An empty AJAX List of Ensembl Mapper Results
            ** means a gap, so remove the Ensembl Object from the AJAX List
            ** of Ensembl Objects and delete it.
            */

            if (ajListGetLength(mrs))
            {
                ajListPeekFirst(mrs, (void **) &mr);

                srid   = ensMapperresultGetObjectidentifier(mr);
                start  = ensMapperresultGetCoordinateStart(mr);
                end    = ensMapperresultGetCoordinateEnd(mr);
                strand = ensMapperresultGetCoordinateStrand(mr);

                while (ajListPop(mrs, (void **) &mr))
                    ensMapperresultDel(&mr);
            }
            else
            {
                if (debug)
                {
                    ajDebug("featureadaptorRemap deleted Ensembl Object (%p), "
                            "which associated Ensembl Feature (%p) maps into "
                            "a gap.\n", Pobject, feature);

                    ensFeatureTrace(feature, 1);
                }

                ajListIterRemove(iter);

                (*fa->Fdelete) (&Pobject);

                continue;
            }
        }

        /*
        ** If the Ensembl Feature maps to a region outside the desired area,
        ** remove the Ensembl Object from the AJAX List of Ensembl Objects
        ** and delete it.
        */

        if ((srid != ensSliceGetSeqregionIdentifier(slice)) ||
            (start > ensSliceGetEnd(slice)) ||
            (end   < ensSliceGetStart(slice)))
        {
            if (debug)
            {
                ajDebug("featureadaptorRemap deleted Ensembl Object (%p), "
                        "which associated Ensembl Feature (%p:%u:%d:%d:%d) "
                        "maps outside the requested region %u:%d:%d:%d.\n",
                        Pobject, feature, srid, start, end, strand,
                        ensSliceGetSeqregionIdentifier(slice),
                        ensSliceGetStart(slice),
                        ensSliceGetEnd(slice),
                        ensSliceGetStrand(slice));

                ensFeatureTrace(feature, 1);
            }

            ajListIterRemove(iter);

            (*fa->Fdelete) (&Pobject);

            continue;
        }

        /* Shift the Feature start, end and strand in one call. */

        if (ensSliceGetStrand(slice) > 0)
            ensFeatureMove(feature,
                           start - ensSliceGetStart(slice) + 1,
                           end   - ensSliceGetStart(slice) + 1,
                           +strand);
        else
            ensFeatureMove(feature,
                           ensSliceGetEnd(slice) - end   + 1,
                           ensSliceGetEnd(slice) - start + 1,
                           -strand);

        ensFeatureSetSlice(feature, slice);
    }

    ajListIterDel(&iter);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic featureadaptorSliceFetch ***************************************
**
** Helper function used by ensFeatureadaptorFetchAllbySlice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::_slice_fetch
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] constraint [AjPStr] SQL statement constraint
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool featureadaptorSliceFetch(EnsPFeatureadaptor fa,
                                       EnsPSlice slice,
                                       AjPStr constraint,
                                       AjPList objects)
{
    const char *table = NULL;

    ajuint extsrid   = 0U;
    ajuint intsrid   = 0U;
    ajuint mrslength = 0U;

    AjBool circular = AJFALSE;
    AjBool toplevel = AJFALSE;

    AjIList iter = NULL;

    AjPList css = NULL; /* Ensembl Coordinate System objects */
    AjPList mis = NULL; /* Ensembl Meta-Information objects */
    AjPList mrs = NULL; /* Ensembl Mapper Result objects */
    AjPList pos = NULL; /* Primary Objects */

    AjPStr key       = NULL;
    AjPStr tmpconstr = NULL;
    AjPStr srids     = NULL;

    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMapperresult mr = NULL;

    EnsPMetainformation mi         = NULL;
    EnsPMetainformationadaptor mia = NULL;

    EnsPMetacoordinateadaptor mca = NULL;

    if (ajDebugTest("featureadaptorSliceFetch"))
        ajDebug("featureadaptorSliceFetch\n"
                "  fa %p\n"
                "  slice %p\n"
                "  constraint '%S'\n"
                "  objects %p\n",
                fa,
                slice,
                constraint,
                objects);

    if (!fa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!constraint)
        return ajFalse;

    if (!objects)
        return ajFalse;

    if (!fa->Adaptor)
    {
        ajDebug("featureadaptorSliceFetch got Ensembl Feature Adaptor "
                "without an Ensembl Base Adaptor.\n");

        return ajFalse;
    }

    dba = ensFeatureadaptorGetDatabaseadaptor(fa);

    if (!dba)
    {
        ajDebug("featureadaptorSliceFetch got Ensembl Feature Adaptor "
                "without an Ensembl Database Adaptor.\n");

        return ajFalse;
    }

    if (!ensSliceIsCircular(slice, &circular))
    {
        ajDebug("featureadaptorSliceFetch could not call ensSliceIsCircular "
                "successfully.\n");

        ensSliceTrace(slice, 1);

        return ajFalse;
    }

    if (!ensSliceIsToplevel(slice, &toplevel))
    {
        ajDebug("featureadaptorSliceFetch could not call ensSliceIsToplevel "
                "successfully.\n");

        ensSliceTrace(slice, 1);

        return ajFalse;
    }

    ama = ensRegistryGetAssemblymapperadaptor(dba);
    csa = ensRegistryGetCoordsystemadaptor(dba);
    mia = ensRegistryGetMetainformationadaptor(dba);
    mca = ensRegistryGetMetacoordinateadaptor(dba);

    /*
    ** Fetch the *build.level Ensembl Meta-Information entry, which specifies
    ** in which Coordinate System this particular Feature has been annotated.
    */

    table = ensBaseadaptorGetPrimarytable(fa->Adaptor);

    key = ajFmtStr("%sbuild.level", table);

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(mia, key, mis);

    ajStrAssignC(&key, table);

    css = ajListNew();

    if (ajListGetLength(mis) && toplevel)
        ajListPushAppend(
            css,
            (void *) ensCoordsystemNewRef(
                ensSliceGetCoordsystemObject(slice)));
    else
        ensMetacoordinateadaptorFetchAllCoordsystems(mca, key, css);

    while (ajListPop(mis, (void **) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    ajStrDel(&key);

    /*
    ** Fetch the Feature objects for each Coordinate System they are stored in.
    ** This may require projecting the Slice for which the Feature objects
    ** have been requested into the Coordinate System they have been
    ** annotated in.
    */

    while (ajListPop(css, (void **) &cs))
    {
        if (ensCoordsystemMatch(cs, ensSliceGetCoordsystemObject(slice)))
        {
            /*
            ** No mapping is required as the Coordinate Systems of Feature
            ** and Slice are identical.
            */

            if (!fa->Maximumlength)
            {
                key = ajStrNewC(table);

                fa->Maximumlength =
                    ensMetacoordinateadaptorGetMaximumlength(mca, cs, key);

                ajStrDel(&key);
            }

            /* Use external Ensembl Sequence Region identifiers if present. */

            srids = ajStrNew();

            intsrid = ensSliceGetSeqregionIdentifier(slice);

            ajFmtPrintAppS(&srids, "%u, ", intsrid);

            while (1)
            {
                extsrid = ensCoordsystemadaptorGetSeqregionidentifierExternal(
                    csa,
                    intsrid);

                if (intsrid == extsrid)
                    break;

                ajFmtPrintAppS(&srids, "%u, ", extsrid);

                intsrid = extsrid;
            }

            /* Remove last comma and space. */

            ajStrCutEnd(&srids, 2);

            tmpconstr = ajStrNewS(constraint);

            if (ajStrGetLen(tmpconstr))
                ajStrAppendC(&tmpconstr, " AND ");

            ajFmtPrintAppS(&tmpconstr,
                           "%s.seq_region_id IN (%S) AND ",
                           table,
                           srids);

            ajStrDel(&srids);

            if (fa->Startequalsend &&
                (ensSliceGetStart(slice) == ensSliceGetEnd(slice)))
                ajFmtPrintAppS(
                    &tmpconstr,
                    "%s.seq_region_start = %d "
                    "AND "
                    "%s.seq_region_end = %d",
                    table, ensSliceGetEnd(slice),
                    table, ensSliceGetStart(slice));
            else
            {
                if (circular == ajFalse)
                {
                    ajFmtPrintAppS(
                        &tmpconstr,
                        "%s.seq_region_start <= %d "
                        "AND "
                        "%s.seq_region_end >= %d",
                        table, ensSliceGetEnd(slice),
                        table, ensSliceGetStart(slice));

                    if (fa->Maximumlength != 0)
                        ajFmtPrintAppS(
                            &tmpconstr,
                            " AND "
                            "%s.seq_region_start >= %d",
                            table,
                            ensSliceGetStart(slice)
                            - fa->Maximumlength);
                }
                else
                {
                    /* Deal with the case of a circular chromosome. */

                    if (ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                        ajFmtPrintAppS(
                            &tmpconstr,
                            "("
                            "%s.seq_region_start >= %d "
                            "OR "
                            "%s.seq_region_start <= %d "
                            "OR "
                            "%s.seq_region_end >= %d "
                            "OR "
                            "%s.seq_region_end <= %d "
                            "OR "
                            "%s.seq_region_start > %s.seq_region_end"
                            ")",
                            table, ensSliceGetStart(slice),
                            table, ensSliceGetEnd(slice),
                            table, ensSliceGetStart(slice),
                            table, ensSliceGetEnd(slice),
                            table, table);
                    else
                        ajFmtPrintAppS(
                            &tmpconstr,
                            "("
                            "("
                            "%s.seq_region_start <= %d "
                            "AND "
                            "%s.seq_region_end >= %d"
                            ") "
                            "OR "
                            "("
                            "%s.seq_region_start > %s.seq_region_end "
                            "AND "
                            "("
                            "%s.seq_region_start <= %d "
                            "OR "
                            "%s.seq_region_end >= %d"
                            ")"
                            ")"
                            ")",
                            table, ensSliceGetEnd(slice),
                            table, ensSliceGetStart(slice),
                            table, table,
                            table, ensSliceGetEnd(slice),
                            table, ensSliceGetStart(slice));
                }
            }

            pos = ajListNew();

            ensBaseadaptorFetchAllbyConstraint(fa->Adaptor,
                                               tmpconstr,
                                               (EnsPAssemblymapper) NULL,
                                               slice,
                                               pos);

            /*
            ** Feature objects may still have to have coordinates made relative
            ** to the Slice start.
            */

            /*
            ** FIXME: The following mapper does not seem to be defined in the
            ** Perl API, when no mapping is needed. Why is there no error?
            ** _remap tests whether Coordinate Systems are not equal and only
            ** executes the mapper code when the Coordinate Systems are not
            ** equal, but that is exactly not the case when no mapping is
            ** required.
            */

            featureadaptorRemap(fa,
                                pos,
                                (EnsPAssemblymapper) NULL,
                                slice);

            ajListPushlist(objects, &pos);

            ajStrDel(&tmpconstr);
        }
        else
        {
            /*
            ** Mapping is required as the Coordinate Systems of Feature and
            ** Slice Coordinate Systems differ.
            */

            ensAssemblymapperadaptorFetchByCoordsystems(
                ama,
                cs,
                ensSliceGetCoordsystemObject(slice),
                &am);

            if (!am)
            {
                ensCoordsystemDel(&cs);

                continue;
            }

            mrs = ajListNew();

            /*
            ** Get the list of coordinates and corresponding internal
            ** identifiers for the regions the Slice spans.
            */

            ensAssemblymapperMapSlice(am, slice, mrs);

            iter = ajListIterNew(mrs);

            while (!ajListIterDone(iter))
            {
                mr = (EnsPMapperresult) ajListIterGet(iter);

                /* Remove all Ensembl Mapper Results that represent gaps. */

                if (ensMapperresultGetType(mr) == ensEMapperresultTypeGap)
                {
                    ajListIterRemove(iter);

                    ensMapperresultDel(&mr);
                }
            }

            ajListIterDel(&iter);

            mrslength = (ajuint) ajListGetLength(mrs);

            if (!mrslength)
            {
                ensCoordsystemDel(&cs);

                continue;
            }

            /*
            ** When regions are large and only partially spanned by a Slice
            ** it is faster to limit the query with start and end constraints.
            ** Take a simple approach and use regional constraints if there
            ** are less than a specific number of regions covered.
            */

            if (mrslength > featureadaptorKMaxSplitQuerySeqregions)
            {
                tmpconstr = ajStrNewS(constraint);

                if (ajStrGetLen(tmpconstr))
                    ajStrAppendC(&tmpconstr, " AND ");

                srids = ajStrNew();

                while (ajListPop(mrs, (void **) &mr))
                {
                    ajFmtPrintAppS(&srids,
                                   "%u, ",
                                   ensMapperresultGetObjectidentifier(mr));

                    ensMapperresultDel(&mr);
                }

                /* Remove last comma and space. */

                ajStrCutEnd(&srids, 2);

                ajFmtPrintAppS(&tmpconstr,
                               "%s.seq_region_id IN (%S)",
                               table,
                               srids);

                ajStrDel(&srids);

                pos = ajListNew();

                ensBaseadaptorFetchAllbyConstraint(fa->Adaptor,
                                                   tmpconstr,
                                                   am,
                                                   slice,
                                                   pos);

                featureadaptorRemap(fa, pos, am, slice);

                ajListPushlist(objects, &pos);

                ajStrDel(&tmpconstr);
            }
            else
            {
                /*
                ** Run multiple split queries using
                ** start and end constraints.
                */

                if (!fa->Maximumlength)
                {
                    key = ajStrNewC(table);

                    fa->Maximumlength =
                        ensMetacoordinateadaptorGetMaximumlength(mca,
                                                                 cs,
                                                                 key);

                    ajStrDel(&key);
                }

                while (ajListPop(mrs, (void **) &mr))
                {
                    tmpconstr = ajStrNewS(constraint);

                    if (ajStrGetLen(tmpconstr))
                        ajStrAppendC(&tmpconstr, " AND ");

                    ajFmtPrintAppS(&tmpconstr,
                                   "%s.seq_region_id = %u "
                                   "AND "
                                   "%s.seq_region_start <= %d "
                                   "AND "
                                   "%s.seq_region_end >= %d",
                                   table,
                                   ensMapperresultGetObjectidentifier(mr),
                                   table,
                                   ensMapperresultGetCoordinateStart(mr),
                                   table,
                                   ensMapperresultGetCoordinateEnd(mr));

                    if (fa->Maximumlength)
                        ajFmtPrintAppS(&tmpconstr,
                                       " AND "
                                       "%s.seq_region_start >= %d",
                                       table,
                                       ensMapperresultGetCoordinateStart(mr)
                                       - fa->Maximumlength);

                    pos = ajListNew();

                    ensBaseadaptorFetchAllbyConstraint(fa->Adaptor,
                                                       tmpconstr,
                                                       am,
                                                       slice,
                                                       pos);

                    ajStrDel(&tmpconstr);

                    featureadaptorRemap(fa, pos, am, slice);

                    ajListPushlist(objects, &pos);

                    ensMapperresultDel(&mr);
                }
            }

            ajListFree(&mrs);

            ensAssemblymapperDel(&am);
        }

        ensCoordsystemDel(&cs);
    }

    ajListFree(&css);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPFeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Feature object(s)
** @nam4rule All   Fetch all Ensembl Feature objects
** @nam4rule Allby Fetch all Ensembl Feature objects matching a criterion
** @nam5rule Analysisname Fetch all by an Ensembl Analysis name
** @nam5rule Slice Fetch by an Ensembl Slice
** @nam5rule Slicescore Fetch by an Ensembl Slice and a score
** @nam4rule By    Fetch one Ensembl Feature object matching a criterion
**
** @argrule * fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @argrule FetchAll objects [AjPList] AJAX List of Ensembl Objects
**                                     based on Ensembl Feature objects
** @argrule AllbyAnalysisname anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyAnalysisname objects [AjPList] AJAX List of Ensembl Objects
**                                              based on Ensembl Feature
**                                              objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice objects [AjPList] AJAX List of Ensembl Objects
**                                       based on Ensembl Feature objects
** @argrule AllbySlicescore slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicescore score [double] Score
** @argrule AllbySlicescore anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicescore objects [AjPList] AJAX List of Ensembl Objects
**                                            based on Ensembl Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureadaptorFetchAllbyAnalysisname ******************************
**
** Fetch all Ensembl Objects based on Ensembl Feature objects by an
** Ensembl Analysis name.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_logic_name
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorFetchAllbyAnalysisname(EnsPFeatureadaptor fa,
                                               const AjPStr anname,
                                               AjPList objects)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!fa)
        return ajFalse;

    if (!anname)
        return ajFalse;

    constraint = ajStrNew();

    if (!ensFeatureadaptorConstraintAppendAnalysisname(fa, &constraint, anname))
    {
        ajStrDel(&constraint);

        return ajFalse;
    }

    result = ensBaseadaptorFetchAllbyConstraint(fa->Adaptor,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                objects);

    ajStrDel(&constraint);

    return result;
}




/* @func ensFeatureadaptorFetchAllbySlice *************************************
**
** Fetch all Ensembl Objects based on Ensembl Feature objects matching an
** SQL constraint on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice_constraint
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] constraint [const AjPStr] SQL constraint
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: This implementation of the Ensembl Base Feature Adaptor does not
** cache Ensembl Feature objects indexed by complete SQL queries in their
** Slice context. The class polymorhism required to do this is difficult to
** implement in ANSI C. Additionally, SQL queries are cached by the RDBMS
** instance.
******************************************************************************/

AjBool ensFeatureadaptorFetchAllbySlice(EnsPFeatureadaptor fa,
                                        EnsPSlice slice,
                                        const AjPStr constraint,
                                        const AjPStr anname,
                                        AjPList objects)
{
    ajint boundary = 0;

    ajuint srid   = 0U;
    ajuint pssrid = 0U;

    void *Pobject = NULL;

    AjBool debug = AJFALSE;

    AjIList ftiter = NULL;
    AjIList psiter = NULL;

    AjPList slpss = NULL; /* Slice Projection Segment objects */
    AjPList srpss = NULL; /* Sequence Region Projection Segment objects */

    AjPStr constr = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPProjectionsegment slps = NULL; /* Slice Projection Segment */
    EnsPProjectionsegment srps = NULL; /* Sequence Region Projection Segment */

    EnsPSlice psslice   = NULL;
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    debug = ajDebugTest("ensFeatureadaptorFetchAllbySlice");

    if (debug)
    {
        ajDebug("ensFeatureadaptorFetchAllbySlice\n"
                "  fa %p\n"
                "  slice %p\n"
                "  constraint '%S'\n"
                "  anname '%S'\n"
                "  objects %p\n",
                fa,
                slice,
                constraint,
                anname,
                objects);

        ensSliceTrace(slice, 1);
    }

    if (!fa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!fa->Adaptor)
    {
        ajDebug("ensFeatureadaptorFetchAllbySlice got an "
                "Ensembl Feature Adaptor without an "
                "Ensembl Base Adaptor.\n");

        return ajFalse;
    }

    dba = ensFeatureadaptorGetDatabaseadaptor(fa);

    if (!dba)
    {
        ajDebug("ensFeatureadaptorFetchAllbySlice got an "
                "Ensembl Feature Adaptor without an "
                "Ensembl Database Adaptor.\n");

        return ajFalse;
    }

    if (constraint && ajStrGetLen(constraint))
        constr = ajStrNewS(constraint);
    else
        constr = ajStrNew();

    if (!ensFeatureadaptorConstraintAppendAnalysisname(fa, &constr, anname))
    {
        ajStrDel(&constr);

        return ajFalse;
    }

    /*
    ** Retrieve normalised, non-symlinked Slices to support pseudo-automsomal
    ** regions (PARs) and haplotypes (HAPs).
    */

    sa = ensRegistryGetSliceadaptor(dba);

    slpss = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(sa, slice, slpss);

    if (!ajListGetLength(slpss))
        ajFatal("ensFeatureadaptorFetchAllbySlice could not get "
                "normalised Slices. "
                "The Ensembl Core database seems to contain incorrect "
                "information in the 'assembly_exception' table.\n");

    /*
    ** Get Feature objects on the full original Slice, as well as any
    ** sym-linked Slice objects. Filter out partial Slice objects from
    ** Projection Segment objects that are based on the same Sequence Region
    ** as the original Slice. A Projection Segment representing the original
    ** Slice is added later on.
    */

    srid = ensSliceGetSeqregionIdentifier(slice);

    psiter = ajListIterNew(slpss);

    while (!ajListIterDone(psiter))
    {
        slps = (EnsPProjectionsegment) ajListIterGet(psiter);

        psslice = ensProjectionsegmentGetTargetSlice(slps);

        pssrid = ensSliceGetSeqregionIdentifier(psslice);

        if (pssrid == srid)
        {
            ajListIterRemove(psiter);

            ensProjectionsegmentDel(&slps);
        }
    }

    ajListIterDel(&psiter);

    /* Add back a Projection Segment representing the original Slice. */

    slps = ensProjectionsegmentNewIni(1, ensSliceCalculateLength(slice), slice);

    ajListPushAppend(slpss, (void *) slps);

    /*
    ** Construct an AJAX List of HAP and PAR boundaries for a Slice spanning
    ** the entire Sequence Region.
    */

    ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                              srid,
                                              0,
                                              0,
                                              ensSliceGetStrand(slice),
                                              &srslice);

    srpss = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(sa, srslice, srpss);

    ensSliceDel(&srslice);

    /*
    ** Exclude the first Projection Segment, since it indicates just the
    ** Slice start coordinate.
    **
    ** 'chromosome:NCBI36:c22_H2:1:49691432:1'
    **
    ** srcstart:srcend    trgslice
    **        1:40992945 'chromosome:NCBI36:22:1:40992945:1'
    ** 40992946:41056606 'chromosome:NCBI36:c22_H2:40992946:41056606:1'
    ** 41056607:49691432 'chromosome:NCBI36:22:41056607:49691432:1'
    */

    ajListPop(srpss, (void **) &srps);

    ensProjectionsegmentDel(&srps);

    /* Fetch Feature objects for the primary Slice and all symlinked Slices. */

    while (ajListPop(slpss, (void **) &slps))
    {
        featureadaptorSliceFetch(fa,
                                 ensProjectionsegmentGetTargetSlice(slps),
                                 constr,
                                 objects);

        if (!ensSliceMatch(slice, ensProjectionsegmentGetTargetSlice(slps)))
        {
            /*
            ** Feature objects returned on symlinked Slices need checking that
            ** they do not cross Slice boundaries.
            */

            /*
            ** FIXME: All Objects are pushed onto the same AJAX List and their
            ** Feature objects are rechecked further below. Should a separate
            ** AJAX List be used to fetch Objects for all Projection Segment
            ** Slices, then checked and pushed onto a separate AJAX List?
            */

            ftiter = ajListIterNew(objects);

            while (!ajListIterDone(ftiter))
            {
                Pobject = ajListIterGet(ftiter);

                feature = (*fa->FobjectGetFeature) (Pobject);

                feature->Start += ensProjectionsegmentGetSourceStart(slps) - 1;
                feature->End   += ensProjectionsegmentGetSourceStart(slps) - 1;

                psiter = ajListIterNewread(srpss);

                while (!ajListIterDone(psiter))
                {
                    srps = (EnsPProjectionsegment) ajListIterGet(psiter);

                    boundary = ensProjectionsegmentGetSourceStart(srps) -
                        ensSliceGetStart(slice) + 1;

                    if ((feature->Start < boundary) &&
                        (feature->End >= boundary))
                    {
                        ajListIterRemove(ftiter);

                        if (debug)
                        {
                            ajDebug(
                                "ensFeatureadaptorFetchAllbySlice "
                                "got an Ensembl Object (%p), which "
                                "Feature (%p) crosses the normalised Slice "
                                "boundary at %d.\n",
                                Pobject,
                                feature,
                                boundary);

                            ensFeatureTrace(feature, 1);
                        }

                        (*fa->Fdelete) (&Pobject);

                        feature = (EnsPFeature) NULL;

                        break;
                    }
                }

                ajListIterDel(&psiter);

                if (feature)
                    ensFeatureSetSlice(feature, slice);
            }

            ajListIterDel(&ftiter);
        }

        ensProjectionsegmentDel(&slps);
    }

    ajListFree(&slpss);

    while (ajListPop(srpss, (void **) &srps))
        ensProjectionsegmentDel(&srps);

    ajListFree(&srpss);

    ajStrDel(&constr);

    return ajTrue;
}




/* @func ensFeatureadaptorFetchAllbySlicescore ********************************
**
** Fetch all Ensembl Objects based on Ensembl Feature objects on an
** Ensembl Slice above a threshold score.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [u] fa [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] score [double] Score
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeatureadaptorFetchAllbySlicescore(EnsPFeatureadaptor fa,
                                             EnsPSlice slice,
                                             double score,
                                             const AjPStr anname,
                                             AjPList objects)
{
    const char *table = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!fa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    table = ensBaseadaptorGetPrimarytable(fa->Adaptor);

    constraint = ajFmtStr("%s.score > %lf", table, score);

    result = ensFeatureadaptorFetchAllbySlice(fa,
                                              slice,
                                              constraint,
                                              anname,
                                              objects);

    ajStrDel(&constraint);

    return result;
}




/* @datasection [EnsPFeaturepair] Ensembl Feature Pair ************************
**
** @nam2rule Featurepair Functions for manipulating
** Ensembl Feature Pair objects
**
** @cc Bio::EnsEMBL::FeaturePair
** @cc CVS Revision: 1.68
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Feature Pair by pointer.
** It is the responsibility of the user to first destroy any previous
** Feature Pair. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy fp [const EnsPFeaturepair] Ensembl Feature Pair
** @argrule Ini srcfeature [EnsPFeature] Source Ensembl Feature
** @argrule Ini trgfeature [EnsPFeature] Target Ensembl Feature
** @argrule Ini edb [EnsPExternaldatabase] Ensembl External Database
** @argrule Ini extra [AjPStr] Extra data
** @argrule Ini srcspecies [AjPStr] Source species name
** @argrule Ini trgspecies [AjPStr] Target species name
** @argrule Ini groupid [ajuint] Group id
** @argrule Ini levelid [ajuint] Level id
** @argrule Ini evalue [double] e- or p-value
** @argrule Ini score [double] Score
** @argrule Ini srccoverage [float] Source coverage in percent
** @argrule Ini trgcoverage [float] Target coverage in percent
** @argrule Ini identity [float] Sequence identity in percent
** @argrule Ref fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @valrule * [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensFeaturepairNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairNewCpy(const EnsPFeaturepair fp)
{
    EnsPFeaturepair pthis = NULL;

    if (!fp)
        return NULL;

    AJNEW0(pthis);

    pthis->SourceFeature    = ensFeatureNewRef(fp->SourceFeature);
    pthis->TargetFeature    = ensFeatureNewRef(fp->TargetFeature);
    pthis->Externaldatabase = ensExternaldatabaseNewRef(fp->Externaldatabase);

    if (fp->Extradata)
        pthis->Extradata = ajStrNewRef(fp->Extradata);

    if (fp->SourceSpecies)
        pthis->SourceSpecies = ajStrNewRef(fp->SourceSpecies);

    if (fp->TargetSpecies)
        pthis->TargetSpecies = ajStrNewRef(fp->TargetSpecies);

    pthis->Use             = 1U;
    pthis->Evalue          = fp->Evalue;
    pthis->Score           = fp->Score;
    pthis->Groupidentifier = fp->Groupidentifier;
    pthis->Levelidentifier = fp->Levelidentifier;
    pthis->SourceCoverage  = fp->SourceCoverage;
    pthis->TargetCoverage  = fp->TargetCoverage;

    return pthis;
}




/* @func ensFeaturepairNewIni *************************************************
**
** Constructor of an Ensembl Feature Pair with initial values.
**
** @cc Bio::EnsEMBL::Feature::new
** @param [u] srcfeature [EnsPFeature] Source Ensembl Feature
** @cc Bio::EnsEMBL::FeaturePair::new
** @param [u] trgfeature [EnsPFeature] Target Ensembl Feature
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] extra [AjPStr] Extra data
** @param [u] srcspecies [AjPStr] Source species name
** @param [u] trgspecies [AjPStr] Target species name
** @param [r] groupid [ajuint] Group id
** @param [r] levelid [ajuint] Level id
** @param [r] evalue [double] e- or p-value
** @param [r] score [double] Score
** @param [r] srccoverage [float] Source coverage in percent
** @param [r] trgcoverage [float] Target coverage in percent
** @param [r] identity [float] Sequence identity in percent
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairNewIni(EnsPFeature srcfeature,
                                     EnsPFeature trgfeature,
                                     EnsPExternaldatabase edb,
                                     AjPStr extra,
                                     AjPStr srcspecies,
                                     AjPStr trgspecies,
                                     ajuint groupid,
                                     ajuint levelid,
                                     double evalue,
                                     double score,
                                     float srccoverage,
                                     float trgcoverage,
                                     float identity)
{
    EnsPFeaturepair fp = NULL;

    if (!srcfeature)
        return NULL;

    if (!trgfeature)
        return NULL;

    AJNEW0(fp);

    fp->SourceFeature = ensFeatureNewRef(srcfeature);

    fp->TargetFeature = ensFeatureNewRef(trgfeature);

    fp->Externaldatabase = ensExternaldatabaseNewRef(edb);

    if (extra)
        fp->Extradata = ajStrNewRef(extra);

    if (srcspecies)
        fp->SourceSpecies = ajStrNewRef(srcspecies);

    if (trgspecies)
        fp->TargetSpecies = ajStrNewRef(trgspecies);

    fp->Use             = 1U;
    fp->Evalue          = evalue;
    fp->Score           = score;
    fp->Groupidentifier = groupid;
    fp->Levelidentifier = levelid;
    fp->SourceCoverage  = srccoverage;
    fp->TargetCoverage  = trgcoverage;
    fp->Identity        = identity;

    return fp;
}




/* @func ensFeaturepairNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairNewRef(EnsPFeaturepair fp)
{
    if (!fp)
        return NULL;

    fp->Use++;

    return fp;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Del Destroy (free) an Ensembl Feature Pair
**
** @argrule * Pfp [EnsPFeaturepair*] Ensembl Feature Pair address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensFeaturepairDel ****************************************************
**
** Default destructor for an Ensembl Feature Pair.
**
** @param [d] Pfp [EnsPFeaturepair*] Ensembl Feature Pair address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensFeaturepairDel(EnsPFeaturepair *Pfp)
{
    EnsPFeaturepair pthis = NULL;

    if (!Pfp)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensFeaturepairDel"))
    {
        ajDebug("ensFeaturepairDel\n"
                "  *Pfp %p\n",
                *Pfp);

        ensFeaturepairTrace(*Pfp, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pfp)
        return;

    pthis = *Pfp;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pfp = NULL;

        return;
    }

    ensFeatureDel(&pthis->SourceFeature);
    ensFeatureDel(&pthis->TargetFeature);

    ensExternaldatabaseDel(&pthis->Externaldatabase);

    ajStrDel(&pthis->Extradata);
    ajStrDel(&pthis->SourceSpecies);
    ajStrDel(&pthis->TargetSpecies);

    AJFREE(pthis);

    *Pfp = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Get Return Feature Pair attribute(s)
** @nam4rule Evalue Return the e-value
** @nam4rule Externaldatabase Return the Ensembl External Database
** @nam4rule Extradata Return the extra data
** @nam4rule Groupidentifier Return the group identifier
** @nam4rule Identity Return the sequence identity
** @nam4rule Levelidentifier Return the level identifier
** @nam4rule Score Return the score
** @nam4rule Source Return source member(s)
** @nam5rule SourceCoverage Return the source coverage
** @nam5rule SourceFeature Return the source Ensembl Feature
** @nam5rule SourceSpecies Return the source species name
** @nam4rule Target Return target member(s)
** @nam5rule TargetCoverage Return the target coverage
** @nam5rule TargetFeature Return the target Ensembl Feature
** @nam5rule TargetSpecies Return the target species name
**
** @argrule * fp [const EnsPFeaturepair] Feature Pair
**
** @valrule Evalue [double] E-value or 0.0
** @valrule Externaldatabase [EnsPExternaldatabase] Ensembl External Database
** or NULL
** @valrule Extradata [AjPStr] Extra data or NULL
** @valrule Groupidentifier [ajuint] Group identifier or 0U
** @valrule Identity [float] Sequence identity od 0.0F
** @valrule Levelidentifier [ajuint] Level identifier or 0U
** @valrule Score [double] Score or 0.0
** @valrule SourceCoverage [float] Source coverage or 0.0F
** @valrule SourceFeature [EnsPFeature] Source Ensembl Feature or NULL
** @valrule SourceSpecies [AjPStr] Source species name or NULL
** @valrule TargetCoverage [float] Target coverage or 0.0F
** @valrule TargetFeature [EnsPFeature] Target Ensembl Feature or NULL
** @valrule TargetSpecies [AjPStr] Target species name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensFeaturepairGetEvalue **********************************************
**
** Get the e-value member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::p_value
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [double] E-value or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensFeaturepairGetEvalue(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Evalue : 0.0;
}




/* @func ensFeaturepairGetExternaldatabase ************************************
**
** Get the Ensembl External Database member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternaldatabase ensFeaturepairGetExternaldatabase(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Externaldatabase : NULL;
}




/* @func ensFeaturepairGetExtradata *******************************************
**
** Get the extra data member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::extra_data
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjPStr] Extra data or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensFeaturepairGetExtradata(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Extradata : NULL;
}




/* @func ensFeaturepairGetGroupidentifier *************************************
**
** Get the group identifier member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::group_id
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajuint] Group identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensFeaturepairGetGroupidentifier(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Groupidentifier : 0U;
}




/* @func ensFeaturepairGetIdentity ********************************************
**
** Get the sequence identity member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::percent_id
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [float] Sequence identity or 0.0F
**
** @release 6.4.0
** @@
******************************************************************************/

float ensFeaturepairGetIdentity(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Identity : 0.0F;
}




/* @func ensFeaturepairGetLevelidentifier *************************************
**
** Get the level identifier member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::level_id
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajuint] Level identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensFeaturepairGetLevelidentifier(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Levelidentifier : 0U;
}




/* @func ensFeaturepairGetScore ***********************************************
**
** Get the score member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::score
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [double] Score or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensFeaturepairGetScore(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->Evalue : 0.0;
}




/* @func ensFeaturepairGetSourceCoverage **************************************
**
** Get the source coverage member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::coverage
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [float] Source coverage or 0.0F
**
** @release 6.2.0
** @@
******************************************************************************/

float ensFeaturepairGetSourceCoverage(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->SourceCoverage : 0.0F;
}




/* @func ensFeaturepairGetSourceFeature ***************************************
**
** Get the source Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeature] Source Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensFeaturepairGetSourceFeature(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->SourceFeature : NULL;
}




/* @func ensFeaturepairGetSourceSpecies ***************************************
**
** Get the source species name member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::species
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjPStr] Source species or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensFeaturepairGetSourceSpecies(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->SourceSpecies : NULL;
}




/* @func ensFeaturepairGetTargetCoverage **************************************
**
** Get the target coverage member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hcoverage
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [float] Target coverage or 0.0F
**
** @release 6.2.0
** @@
******************************************************************************/

float ensFeaturepairGetTargetCoverage(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->TargetCoverage : 0.0F;
}




/* @func ensFeaturepairGetTargetFeature ***************************************
**
** Get the target Ensembl Feature member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hslice
** @cc Bio::EnsEMBL::FeaturePair::hseqname
** @cc Bio::EnsEMBL::FeaturePair::hstart
** @cc Bio::EnsEMBL::FeaturePair::hend
** @cc Bio::EnsEMBL::FeaturePair::hstrand
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeature] Target Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensFeaturepairGetTargetFeature(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->TargetFeature : NULL;
}




/* @func ensFeaturepairGetTargetSpecies ***************************************
**
** Get the target species name member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hspecies
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjPStr] Target species or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensFeaturepairGetTargetSpecies(
    const EnsPFeaturepair fp)
{
    return (fp) ? fp->TargetSpecies : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Set Set one member of an Ensembl Feature Pair
** @nam4rule Evalue Set the e-value
** @nam4rule Externaldatabase Set the Ensembl External Database
** @nam4rule Extradata Set the extra data
** @nam4rule Identity Set the sequence identity
** @nam4rule Groupidentifier Set the group identifier
** @nam4rule Levelidentifier Set the level identifier
** @nam4rule Score Set the score
** @nam4rule Source Set source member(s)
** @nam5rule SourceCoverage Set the source coverage
** @nam5rule SourceFeature Set the source Ensembl Feature
** @nam5rule SourceSpecies Set the source species name
** @nam4rule Target Set target member(s)
** @nam5rule TargetCoverage Set the target coverage
** @nam5rule TargetFeature Set the target Ensembl Feature
** @nam5rule TargetSpecies Set the target species name
**
** @argrule * fp [EnsPFeaturepair] Ensembl Feature Pair object
** @argrule Evalue evalue [double] E-value
** @argrule Externaldatabase edb [EnsPExternaldatabase]
** Ensembl External Database
** @argrule Extradata extra [AjPStr] Extra data
** @argrule Groupidentifier groupid [ajuint] Group identifier
** @argrule Identity identity [float] Sequence identity
** @argrule Levelidentifier levelid [ajuint] Level identifier
** @argrule Score score [double] Score
** @argrule SourceCoverage coverage [float] Source coverage
** @argrule SourceFeature feature [EnsPFeature] Source Ensembl Feature
** @argrule SourceSpecies species [AjPStr] Source species
** @argrule TargetCoverage coverage [float] Target coverage
** @argrule TargetFeature feature [EnsPFeature] Target Ensembl Feature
** @argrule TargetSpecies tspecies [AjPStr] Target species
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensFeaturepairSetEvalue **********************************************
**
** Set the e-value member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::p_value
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] evalue [double] E-value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetEvalue(EnsPFeaturepair fp,
                               double evalue)
{
    if (!fp)
        return ajFalse;

    fp->Evalue = evalue;

    return ajTrue;
}




/* @func ensFeaturepairSetExternaldatabase ************************************
**
** Set the Ensembl External Database member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::external_db_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetExternaldatabase(EnsPFeaturepair fp,
                                         EnsPExternaldatabase edb)
{
    if (!fp)
        return ajFalse;

    ensExternaldatabaseDel(&fp->Externaldatabase);

    fp->Externaldatabase = ensExternaldatabaseNewRef(edb);

    return ajTrue;
}




/* @func ensFeaturepairSetExtradata *******************************************
**
** Set the extra data member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::extra_data
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] extra [AjPStr] Extra data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetExtradata(EnsPFeaturepair fp,
                                  AjPStr extra)
{
    if (!fp)
        return ajFalse;

    ajStrDel(&fp->Extradata);

    if (extra)
        fp->Extradata = ajStrNewRef(extra);

    return ajTrue;
}




/* @func ensFeaturepairSetGroupidentifier *************************************
**
** Set the group identifier member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::group_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] groupid [ajuint] Group identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetGroupidentifier(EnsPFeaturepair fp,
                                        ajuint groupid)
{
    if (!fp)
        return ajFalse;

    fp->Groupidentifier = groupid;

    return ajTrue;
}




/* @func ensFeaturepairSetIdentity ********************************************
**
** Set the sequence identity member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::percent_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] identity [float] Sequence identity
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetIdentity(EnsPFeaturepair fp,
                                 float identity)
{
    if (!fp)
        return ajFalse;

    fp->Identity = identity;

    return ajTrue;
}




/* @func ensFeaturepairSetLevelidentifier *************************************
**
** Set the level identifier member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::level_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] levelid [ajuint] Level identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetLevelidentifier(EnsPFeaturepair fp,
                                        ajuint levelid)
{
    if (!fp)
        return ajFalse;

    fp->Levelidentifier = levelid;

    return ajTrue;
}




/* @func ensFeaturepairSetScore ***********************************************
**
** Set the score member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::score
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetScore(EnsPFeaturepair fp,
                              double score)
{
    if (!fp)
        return ajFalse;

    fp->Score = score;

    return ajTrue;
}




/* @func ensFeaturepairSetSourceCoverage **************************************
**
** Set the source coverage member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::coverage
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] coverage [float] Source coverage
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetSourceCoverage(EnsPFeaturepair fp,
                                       float coverage)
{
    if (!fp)
        return ajFalse;

    fp->SourceCoverage = coverage;

    return ajTrue;
}




/* @func ensFeaturepairSetSourceFeature ***************************************
**
** Set the source Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] feature [EnsPFeature] Source Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetSourceFeature(EnsPFeaturepair fp,
                                      EnsPFeature feature)
{
    if (ajDebugTest("ensFeaturepairSetSourceFeature"))
    {
        ajDebug("ensFeaturepairSetSourceFeature\n"
                "  fp %p\n"
                "  feature %p\n",
                fp,
                feature);

        ensFeaturepairTrace(fp, 1);

        ensFeatureTrace(feature, 1);
    }

    if (!fp)
        return ajFalse;

    /* Replace the current Feature. */

    ensFeatureDel(&fp->SourceFeature);

    fp->SourceFeature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensFeaturepairSetSourceSpecies ***************************************
**
** Set the source species member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::species
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] species [AjPStr] Source species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetSourceSpecies(EnsPFeaturepair fp,
                                      AjPStr species)
{
    if (!fp)
        return ajFalse;

    ajStrDel(&fp->SourceSpecies);

    if (species)
        fp->SourceSpecies = ajStrNewRef(species);

    return ajTrue;
}




/* @func ensFeaturepairSetTargetCoverage **************************************
**
** Set the target coverage member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hcoverage
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] coverage [float] Target coverage
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetTargetCoverage(EnsPFeaturepair fp,
                                       float coverage)
{
    if (!fp)
        return ajFalse;

    fp->TargetCoverage = coverage;

    return ajTrue;
}




/* @func ensFeaturepairSetTargetFeature ***************************************
**
** Set the target Ensembl Feature member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hslice
** @cc Bio::EnsEMBL::FeaturePair::hseqname
** @cc Bio::EnsEMBL::FeaturePair::hstart
** @cc Bio::EnsEMBL::FeaturePair::hend
** @cc Bio::EnsEMBL::FeaturePair::hstrand
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] feature [EnsPFeature] Target Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetTargetFeature(EnsPFeaturepair fp,
                                      EnsPFeature feature)
{
    if (ajDebugTest("ensFeaturepairSetTargetFeature"))
    {
        ajDebug("ensFeaturepairSetTargetFeature\n"
                "  fp %p\n"
                "  feature %p\n",
                fp,
                feature);

        ensFeaturepairTrace(fp, 1);

        ensFeatureTrace(feature, 1);
    }

    if (!fp)
        return ajFalse;

    /* Replace the current Feature. */

    ensFeatureDel(&fp->TargetFeature);

    fp->TargetFeature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensFeaturepairSetTargetSpecies ***************************************
**
** Set the target species member of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hspecies
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] tspecies [AjPStr] Target species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairSetTargetSpecies(EnsPFeaturepair fp,
                                      AjPStr tspecies)
{
    if (!fp)
        return ajFalse;

    ajStrDel(&fp->TargetSpecies);

    if (tspecies)
        fp->TargetSpecies = ajStrNewRef(tspecies);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Trace Report Ensembl Feature Pair members to debug file
**
** @argrule Trace fp [const EnsPFeaturepair] Ensembl Feature Pair
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensFeaturepairTrace **************************************************
**
** Trace an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensFeaturepairTrace(const EnsPFeaturepair fp, ajuint level)
{
    AjPStr indent = NULL;

    if (!fp)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("ensFeaturepairTrace %p\n"
            "%S  SourceFeature %p\n"
            "%S  TargetFeature %p\n"
            "%S  Externaldatabase %p\n"
            "%S  Extradata %p\n"
            "%S  SourceSpecies '%S'\n"
            "%S  TargetSpecies '%S'\n"
            "%S  Groupidentifier %d\n"
            "%S  Levelidentifier %d\n"
            "%S  Use %u\n"
            "%S  Evalue %f\n"
            "%S  Score %f\n"
            "%S  SourceCoverage %f\n"
            "%S  TargetCoverage %f\n"
            "%S  Identity %f\n",
            indent, fp,
            indent, fp->SourceFeature,
            indent, fp->TargetFeature,
            indent, fp->Externaldatabase,
            indent, fp->Extradata,
            indent, fp->SourceSpecies,
            indent, fp->TargetSpecies,
            indent, fp->Groupidentifier,
            indent, fp->Levelidentifier,
            indent, fp->Use,
            indent, fp->Evalue,
            indent, fp->Score,
            indent, fp->SourceCoverage,
            indent, fp->TargetCoverage,
            indent, fp->Identity);

    ensFeatureTrace(fp->SourceFeature, level + 1);

    ensFeatureTrace(fp->TargetFeature, level + 1);

    ensExternaldatabaseTrace(fp->Externaldatabase, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Calculate Calculate Ensembl Feature Pair values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensFeaturepairCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensFeaturepairCalculateMemsize(const EnsPFeaturepair fp)
{
    size_t size = 0;

    if (!fp)
        return 0;

    size += sizeof (EnsOFeaturepair);

    size += ensFeatureCalculateMemsize(fp->SourceFeature);
    size += ensFeatureCalculateMemsize(fp->TargetFeature);

    if (fp->Extradata)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(fp->Extradata);
    }

    if (fp->SourceSpecies)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(fp->SourceSpecies);
    }

    if (fp->TargetSpecies)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(fp->TargetSpecies);
    }

    return size;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Feature Pair objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Transfer Transfer an Ensembl Feature Pair
** @nam3rule Transform Transform an Ensembl Feature Pair
**
** @argrule Transfer fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform fp [const EnsPFeaturepair] Ensembl Feature Pair
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
**
** @valrule * [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensFeaturepairTransfer ***********************************************
**
** Transfer an Ensembl Feature Pair onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::transfer
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairTransfer(EnsPFeaturepair fp,
                                       EnsPSlice slice)
{
    EnsPFeature newfeature = NULL;
    EnsPFeaturepair newfp  = NULL;

    if (!fp)
        return NULL;

    if (!slice)
        return NULL;

    newfeature = ensFeatureTransfer(fp->SourceFeature, slice);

    if (!newfeature)
        return NULL;

    newfp = ensFeaturepairNewCpy(fp);

    ensFeaturepairSetSourceFeature(newfp, newfeature);

    ensFeatureDel(&newfeature);

    return newfp;
}




/* @func ensFeaturepairTransform **********************************************
**
** Transform an Ensembl Feature Pair into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Feature::transform
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairTransform(const EnsPFeaturepair fp,
                                        const AjPStr csname,
                                        const AjPStr csversion)
{
    EnsPFeature nfeature = NULL;
    EnsPFeaturepair nfp  = NULL;

    if (!fp)
        return NULL;

    if (!csname)
        return NULL;

    if (!csversion)
        return NULL;

    nfeature = ensFeatureTransform(fp->SourceFeature,
                                   csname,
                                   csversion,
                                   (EnsPSlice) NULL);

    if (!nfeature)
        return NULL;

    nfp = ensFeaturepairNewCpy(fp);

    ensFeaturepairSetSourceFeature(nfp, nfeature);

    ensFeatureDel(&nfeature);

    return nfp;
}




/* @section convenience functions *********************************************
**
** Ensembl Feature Pair convenience functions
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Source Get Ensembl Feature member(s) for the source
** @nam4rule Target Get Ensembl Feature member(s) for the target
** @nam5rule End Get the Ensembl Feature end
** @nam5rule Start Get the Ensembl Feature start
** @nam5rule Strand Get the Ensembl Feature strand
**
** @argrule * fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @valrule End [ajint] Ensembl Feature end or 0
** @valrule Start [ajint] Ensembl Feature start or 0
** @valrule Strand [ajint] Ensembl Feature strand or 0
**
** @fcategory use
******************************************************************************/




/* @func ensFeaturepairGetSourceEnd *******************************************
**
** Get the end member of the
** source Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajint] Ensembl Feature end or 0
** @@
******************************************************************************/

ajint ensFeaturepairGetSourceEnd(const EnsPFeaturepair fp)
{
    return (fp && fp->SourceFeature) ?
        ensFeatureGetEnd(fp->SourceFeature) : 0;
}




/* @func ensFeaturepairGetSourceStart *****************************************
**
** Get the start member of the
** source Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajint] Ensembl Feature start or 0
** @@
******************************************************************************/

ajint ensFeaturepairGetSourceStart(const EnsPFeaturepair fp)
{
    return (fp && fp->SourceFeature) ?
        ensFeatureGetStart(fp->SourceFeature) : 0;
}




/* @func ensFeaturepairGetSourceStrand ****************************************
**
** Get the strand member of the
** source Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajint] Ensembl Feature strand or 0
** @@
******************************************************************************/

ajint ensFeaturepairGetSourceStrand(const EnsPFeaturepair fp)
{
    return (fp && fp->SourceFeature) ?
        ensFeatureGetStrand(fp->SourceFeature) : 0;
}




/* @func ensFeaturepairGetTargetEnd *******************************************
**
** Get the end member of the
** target Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajint] Ensembl Feature end or 0
** @@
******************************************************************************/

ajint ensFeaturepairGetTargetEnd(const EnsPFeaturepair fp)
{
    return (fp && fp->TargetFeature) ?
        ensFeatureGetEnd(fp->TargetFeature) : 0;
}




/* @func ensFeaturepairGetTargetStart *****************************************
**
** Get the start member of the
** target Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajint] Ensembl Feature start or 0
** @@
******************************************************************************/

ajint ensFeaturepairGetTargetStart(const EnsPFeaturepair fp)
{
    return (fp && fp->TargetFeature) ?
        ensFeatureGetStart(fp->TargetFeature) : 0;
}




/* @func ensFeaturepairGetTargetStrand ****************************************
**
** Get the strand member of the
** target Ensembl Feature member of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajint] Ensembl Feature strand or 0
** @@
******************************************************************************/

ajint ensFeaturepairGetTargetStrand(const EnsPFeaturepair fp)
{
    return (fp && fp->TargetFeature) ?
        ensFeatureGetStrand(fp->TargetFeature) : 0;
}




/* @section comparing *********************************************************
**
** Functions for comparing Ensembl Feature Pair objects
**
** @fdata [EnsPFeaturepair]
**
** @nam3rule Compare Compare two Ensembl Feature Pair objects
** @nam4rule Source  Compare by source Ensembl Feature object members
** @nam4rule Target  Compare by target Ensembl Feature object members
** @nam5rule End     Compare by Ensembl Feature end members
** @nam5rule Start   Compare by Ensembl Feature start members
** @nam6rule Ascending  Compare in ascending order
** @nam6rule Descending Compare in descending order
**
** @argrule * fp1 [const EnsPFeaturepair] Ensembl Feature Pair 1
** @argrule * fp2 [const EnsPFeaturepair] Ensembl Feature Pair 2
**
** @valrule * [int] The comparison function returns an integer less than,
**                  equal to, or greater than zero if the first argument is
**                  considered to be respectively less than, equal to, or
**                  greater than the second.
**
** @fcategory use
******************************************************************************/




/* @func ensFeaturepairCompareSourceEndAscending ******************************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** Source Ensembl Feature end member in ascending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature sort
** towards the end of the AJAX List.
**
** @param [r] fp1 [const EnsPFeaturepair] Ensembl Feature Pair 1
** @param [r] fp2 [const EnsPFeaturepair] Ensembl Feature Pair 2
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

int ensFeaturepairCompareSourceEndAscending(const EnsPFeaturepair fp1,
                                            const EnsPFeaturepair fp2)
{
    if (ajDebugTest("ensFeaturepairCompareSourceEndAscending"))
    {
        ajDebug("ensFeaturepairCompareSourceEndAscending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeatureCompareEndAscending(fp1->SourceFeature,
                                         fp2->SourceFeature);
}




/* @func ensFeaturepairCompareSourceEndDescending *****************************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** source Ensembl Feature end member in descending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature sort
** towards the end of the AJAX List.
**
** @param [r] fp1 [const EnsPFeaturepair] Ensembl Feature Pair 1
** @param [r] fp2 [const EnsPFeaturepair] Ensembl Feature Pair 2
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

int ensFeaturepairCompareSourceEndDescending(const EnsPFeaturepair fp1,
                                             const EnsPFeaturepair fp2)
{
    if (ajDebugTest("ensFeaturepairCompareSourceEndDescending"))
    {
        ajDebug("ensFeaturepairCompareSourceEndDescending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeatureCompareEndDescending(fp1->SourceFeature,
                                          fp2->SourceFeature);
}




/* @func ensFeaturepairCompareSourceStartAscending ****************************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** Source Ensembl Feature start member in ascending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature sort
** towards the end of the AJAX List.
**
** @param [r] fp1 [const EnsPFeaturepair] Ensembl Feature Pair 1
** @param [r] fp2 [const EnsPFeaturepair] Ensembl Feature Pair 2
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

int ensFeaturepairCompareSourceStartAscending(const EnsPFeaturepair fp1,
                                              const EnsPFeaturepair fp2)
{
    if (ajDebugTest("ensFeaturepairCompareSourceStartAscending"))
    {
        ajDebug("ensFeaturepairCompareSourceStartAscending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeatureCompareStartAscending(fp1->SourceFeature,
                                           fp2->SourceFeature);
}




/* @func ensFeaturepairCompareSourceStartDescending ***************************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** source Ensembl Feature start member in descending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature sort
** towards the end of the AJAX List.
**
** @param [r] fp1 [const EnsPFeaturepair] Ensembl Feature Pair 1
** @param [r] fp2 [const EnsPFeaturepair] Ensembl Feature Pair 2
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

int ensFeaturepairCompareSourceStartDescending(const EnsPFeaturepair fp1,
                                               const EnsPFeaturepair fp2)
{
    if (ajDebugTest("ensFeaturepairCompareSourceStartDescending"))
    {
        ajDebug("ensFeaturepairCompareSourceStartDescending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeatureCompareStartDescending(fp1->SourceFeature,
                                            fp2->SourceFeature);
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listFeaturepairCompareSourceEndAscending ***********************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** source Ensembl Feature end member in ascending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature object sort
** towards the end of the AJAX List.
**
** @param [r] item1 [const void*] Ensembl Feature Pair address 1
** @param [r] item2 [const void*] Ensembl Feature Pair address 2
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

static int listFeaturepairCompareSourceEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPFeaturepair fp1 = *(EnsOFeaturepair *const *) item1;
    EnsPFeaturepair fp2 = *(EnsOFeaturepair *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("ensFeaturepairCompareSourceEndAscending"))
    {
        ajDebug("ensFeaturepairCompareSourceEndAscending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeaturepairCompareSourceEndAscending(fp1, fp2);
}




/* @funcstatic listFeaturepairCompareSourceEndDescending **********************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** source Ensembl Feature end member in descending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature object sort
** towards the end of the AJAX List.
**
** @param [r] item1 [const void*] Ensembl Feature Pair address 1
** @param [r] item2 [const void*] Ensembl Feature Pair address 2
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

static int listFeaturepairCompareSourceEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPFeaturepair fp1 = *(EnsOFeaturepair *const *) item1;
    EnsPFeaturepair fp2 = *(EnsOFeaturepair *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("ensFeaturepairCompareSourceEndDescending"))
    {
        ajDebug("ensFeaturepairCompareSourceEndDescending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeaturepairCompareSourceEndDescending(fp1, fp2);
}




/* @funcstatic listFeaturepairCompareSourceStartAscending *********************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** source Ensembl Feature start member in ascending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature object sort
** towards the end of the AJAX List.
**
** @param [r] item1 [const void*] Ensembl Feature Pair address 1
** @param [r] item2 [const void*] Ensembl Feature Pair address 2
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

static int listFeaturepairCompareSourceStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPFeaturepair fp1 = *(EnsOFeaturepair *const *) item1;
    EnsPFeaturepair fp2 = *(EnsOFeaturepair *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("ensFeaturepairCompareSourceStartAscending"))
    {
        ajDebug("ensFeaturepairCompareSourceStartAscending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeaturepairCompareSourceStartAscending(fp1, fp2);
}




/* @funcstatic listFeaturepairCompareSourceStartDescending ********************
**
** AJAX List of Ensembl Feature Pair objects comparison function to sort by
** source Ensembl Feature start member in descending order.
**
** Ensembl Feature Pair objects without a source Ensembl Feature object sort
** towards the end of the AJAX List.
**
** @param [r] item1 [const void*] Ensembl Feature Pair address 1
** @param [r] item2 [const void*] Ensembl Feature Pair address 2
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

static int listFeaturepairCompareSourceStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPFeaturepair fp1 = *(EnsOFeaturepair *const *) item1;
    EnsPFeaturepair fp2 = *(EnsOFeaturepair *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("ensFeaturepairCompareSourceStartDescending"))
    {
        ajDebug("ensFeaturepairCompareSourceStartDescending\n"
                "  fp1 %p\n"
                "  fp2 %p\n",
                fp1,
                fp2);

        ensFeaturepairTrace(fp1, 1);
        ensFeaturepairTrace(fp2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (fp1 && (!fp2))
        return -1;

    if ((!fp1) && (!fp2))
        return 0;

    if ((!fp1) && fp2)
        return +1;

    return ensFeaturepairCompareSourceStartDescending(fp1, fp2);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Featurepair Functions for manipulating AJAX List objects of
** Ensembl Feature Pair objects
** @nam4rule Sort       Sort functions
** @nam5rule Source     Sort by source Ensembl Feature member
** @nam5rule Target     Sort by target Ensembl Feature member
** @nam6rule End        Sort by Ensembl Feature end member
** @nam6rule Start      Sort by Ensembl Feature start member
** @nam7rule Ascending  Sort in ascending order
** @nam7rule Descending Sort in descending order
**
** @argrule * fps [AjPList]  AJAX List of Ensembl Feature Pair objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListFeaturepairSortSourceEndAscending *****************************
**
** Sort an AJAX List of Ensembl Feature Pair objects by their source
** Ensembl Feature end coordinate in ascending order.
**
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
** @see ensFeaturepairCompareSourceEndAscending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeaturepairSortSourceEndAscending(AjPList fps)
{
    if (!fps)
        return ajFalse;

    ajListSortTwo(fps,
                  &listFeaturepairCompareSourceEndAscending,
                  &listFeaturepairCompareSourceStartAscending);

    return ajTrue;
}




/* @func ensListFeaturepairSortSourceEndDescending ****************************
**
** Sort an AJAX List of Ensembl Feature Pair objects by their
** source Ensembl Feature end coordinate in descending order.
**
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
** @see ensFeaturepairCompareSourceEndDescending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeaturepairSortSourceEndDescending(AjPList fps)
{
    if (!fps)
        return ajFalse;

    ajListSortTwo(fps,
                  &listFeaturepairCompareSourceEndDescending,
                  &listFeaturepairCompareSourceStartDescending);

    return ajTrue;
}




/* @func ensListFeaturepairSortSourceStartAscending ***************************
**
** Sort an AJAX List of Ensembl Feature Pair objects by their source
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
** @see ensFeaturepairCompareSourceStartAscending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeaturepairSortSourceStartAscending(AjPList fps)
{
    if (!fps)
        return ajFalse;

    ajListSortTwo(fps,
                  &listFeaturepairCompareSourceStartAscending,
                  &listFeaturepairCompareSourceEndAscending);

    return ajTrue;
}




/* @func ensListFeaturepairSortSourceStartDescending **************************
**
** Sort an AJAX List of Ensembl Feature Pair objects by their
** source Ensembl Feature start coordinate in descending order.
**
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
** @see ensFeaturepairCompareSourceStartDescending
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListFeaturepairSortSourceStartDescending(AjPList fps)
{
    if (!fps)
        return ajFalse;

    ajListSortTwo(fps,
                  &listFeaturepairCompareSourceStartDescending,
                  &listFeaturepairCompareSourceEndDescending);

    return ajTrue;
}




/* @datasection [EnsPAssemblyexceptionfeature] Ensembl Assembly Exception
** Feature
**
** @nam2rule Assemblyexceptionfeature Functions for manipulating
** Ensembl Assembly Exception Feature objects
**
** @cc Bio::EnsEMBL::AssemblyExceptionFeature
** @cc CVS Revision: 1.9
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly Exception Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly Exception Feature. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @argrule Ini aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini slice [EnsPSlice] Exception Ensembl Slice
** @argrule Ini type [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
** @argrule Ref aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @valrule * [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyexceptionfeatureNewCpy ************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewCpy(
    const EnsPAssemblyexceptionfeature aef)
{
    EnsPAssemblyexceptionfeature pthis = NULL;

    AJNEW0(pthis);

    pthis->Use            = 1U;
    pthis->Identifier     = aef->Identifier;
    pthis->Adaptor        = aef->Adaptor;
    pthis->Feature        = ensFeatureNewRef(aef->Feature);
    pthis->ExceptionSlice = ensSliceNewRef(aef->ExceptionSlice);
    pthis->Type           = aef->Type;

    return pthis;
}




/* @func ensAssemblyexceptionfeatureNewIni ************************************
**
** Constructor for an Ensembl Assembly Exception Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::AssemblyExceptionFeature::new
** @param [u] slice [EnsPSlice] Alternative Slice
** @param [u] type [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewIni(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPSlice slice,
    EnsEAssemblyexceptionType type)
{
    EnsPAssemblyexceptionfeature aef = NULL;

    if (!slice)
        return NULL;

    if (!type)
        return NULL;

    AJNEW0(aef);

    aef->Use            = 1U;
    aef->Identifier     = identifier;
    aef->Adaptor        = aefa;
    aef->Feature        = feature;
    aef->ExceptionSlice = ensSliceNewRef(slice);
    aef->Type           = type;

    return aef;
}




/* @func ensAssemblyexceptionfeatureNewRef ************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewRef(
    EnsPAssemblyexceptionfeature aef)
{
    if (!aef)
        return NULL;

    aef->Use++;

    return aef;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Exception Feature
**
** @argrule * Paef [EnsPAssemblyexceptionfeature*]
** Ensembl Assembly Exception Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblyexceptionfeatureDel ***************************************
**
** Default destructor for an Ensembl Assembly Exception Feature.
**
** @param [d] Paef [EnsPAssemblyexceptionfeature*]
** Ensembl Assembly Exception Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAssemblyexceptionfeatureDel(EnsPAssemblyexceptionfeature *Paef)
{
    EnsPAssemblyexceptionfeature pthis = NULL;

    if (!Paef)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAssemblyexceptionfeatureDel"))
    {
        ajDebug("ensAssemblyexceptionfeatureDel\n"
                "  *Paef %p\n",
                *Paef);

        ensAssemblyexceptionfeatureTrace(*Paef, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Paef)
        return;

    pthis = *Paef;

    pthis->Use--;

    if (pthis->Use)
    {
        *Paef = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensSliceDel(&pthis->ExceptionSlice);

    AJFREE(pthis);

    *Paef = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule Get Return Assembly Exception Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Assembly Exception Feature Adaptor
** @nam4rule Exception Return exeption attribute(s)
** @nam5rule Slice Return the exception Ensembl Slice
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Type Return the Ensembl Assembly Exception Type enumeration
**
** @argrule * aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @valrule Adaptor [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor or NULL
** @valrule ExceptionSlice [EnsPSlice]
** Alternative Ensembl Slice or NULL
** @valrule Feature [EnsPFeature]
** Ensembl Feature or NULL
** @valrule Identifier [ajuint]
** SQL database-internal identifier or 0
** @valrule Type [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type or ensEAssemblyexceptionTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionfeatureGetAdaptor ********************************
**
** Get the Ensembl Assembly Exception Feature Adaptor member of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureGetAdaptor(
    const EnsPAssemblyexceptionfeature aef)
{
    return (aef) ? aef->Adaptor : NULL;
}




/* @func ensAssemblyexceptionfeatureGetExceptionSlice *************************
**
** Get the alternate Ensembl Slice member of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::AssemblyExceptionFeature::alternate_slice
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [EnsPSlice] Alternate Ensembl Slice or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSlice ensAssemblyexceptionfeatureGetExceptionSlice(
    const EnsPAssemblyexceptionfeature aef)
{
    return (aef) ? aef->ExceptionSlice : NULL;
}




/* @func ensAssemblyexceptionfeatureGetFeature ********************************
**
** Get the Ensembl Feature member of an Ensembl Assembly Exception Feature.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensAssemblyexceptionfeatureGetFeature(
    const EnsPAssemblyexceptionfeature aef)
{
    return (aef) ? aef->Feature : NULL;
}




/* @func ensAssemblyexceptionfeatureGetIdentifier *****************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionfeatureGetIdentifier(
    const EnsPAssemblyexceptionfeature aef)
{
    return (aef) ? aef->Identifier : 0U;
}




/* @func ensAssemblyexceptionfeatureGetType ***********************************
**
** Get the Ensembl Assembly Exception Type enumeration member of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::AssemblyExceptionFeature::type
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration or ensEAssemblyexceptionTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEAssemblyexceptionType ensAssemblyexceptionfeatureGetType(
    const EnsPAssemblyexceptionfeature aef)
{
    return (aef) ? aef->Type : ensEAssemblyexceptionTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule Set Set one member of an Assembly Exception Feature
** @nam4rule Adaptor Set the Ensembl Assembly Exception Feature Adaptor
** @nam4rule Exception Set exception attribute(s)
** @nam5rule Slice Set the exception Ensembl Slice
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Type Set the Ensembl Assembly Exception Type enumeration
**
** @argrule * aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @argrule Adaptor aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @argrule ExceptionSlice slice [EnsPSlice]
** Exception Ensembl Slice
** @argrule Feature feature [EnsPFeature]
** Ensembl Feature
** @argrule Identifier identifier [ajuint]
** SQL database-internal identifier
** @argrule Type type [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensAssemblyexceptionfeatureSetAdaptor ********************************
**
** Set the Ensembl Assembly Exception Feature Adaptor member of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetAdaptor(
    EnsPAssemblyexceptionfeature aef,
    EnsPAssemblyexceptionfeatureadaptor aefa)
{
    if (!aef)
        return ajFalse;

    aef->Adaptor = aefa;

    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetExceptionSlice *************************
**
** Set the alternate Ensembl Slice member of an
** Ensembl Assembly Exception Feature.
**
** @param [u] aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [u] slice [EnsPSlice] Exception Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetExceptionSlice(
    EnsPAssemblyexceptionfeature aef,
    EnsPSlice slice)
{
    if (!aef)
        return ajFalse;

    ensSliceDel(&aef->ExceptionSlice);

    aef->ExceptionSlice = ensSliceNewRef(slice);

    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetFeature ********************************
**
** Set the Ensembl Feature member of an Ensembl Assembly Exception Feature.
**
** @param [u] aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetFeature(
    EnsPAssemblyexceptionfeature aef,
    EnsPFeature feature)
{
    if (!aef)
        return ajFalse;

    ensFeatureDel(&aef->Feature);

    aef->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetIdentifier *****************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetIdentifier(
    EnsPAssemblyexceptionfeature aef,
    ajuint identifier)
{
    if (!aef)
        return ajFalse;

    aef->Identifier = identifier;

    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetType ***********************************
**
** Set the Ensembl Assembly Exception Type enumeration member of an
** Ensembl Assembly Exception Feature.
**
** @param [u] aef [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [u] type [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetType(EnsPAssemblyexceptionfeature aef,
                                          EnsEAssemblyexceptionType type)
{
    if (!aef)
        return ajFalse;

    aef->Type = type;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule Trace Report Ensembl Assembly Exception Feature members to
**                 debug file
**
** @argrule Trace aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyexceptionfeatureTrace *************************************
**
** Trace an Ensembl Assembly Exception Feature.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureTrace(const EnsPAssemblyexceptionfeature aef,
                                        ajuint level)
{
    AjPStr indent = NULL;

    if (!aef)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("ensAssemblyexceptionfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  EnsPAssemblyexceptionfeatureadaptor %p\n"
            "%S  Feature %p\n"
            "%S  ExceptionSlice %p\n"
            "%S  Type %d\n",
            indent, aef,
            indent, aef->Use,
            indent, aef->Identifier,
            indent, aef->Adaptor,
            indent, aef->Feature,
            indent, aef->ExceptionSlice,
            indent, aef->Type);

    ensFeatureTrace(aef->Feature, level + 1);

    ensSliceTrace(aef->ExceptionSlice, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule Calculate Calculate Ensembl Assembly Exception Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyexceptionfeatureCalculateMemsize **************************
**
** Calculate the memory size in bytes of an Ensembl Assembly Exception Feature.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensAssemblyexceptionfeatureCalculateMemsize(
    const EnsPAssemblyexceptionfeature aef)
{
    size_t size = 0;

    if (!aef)
        return 0;

    size += sizeof (EnsOAssemblyexceptionfeature);

    size += ensFeatureCalculateMemsize(aef->Feature);

    size += ensSliceCalculateMemsize(aef->ExceptionSlice);

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an
** Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
**
** @nam3rule Fetch Fetch object from an Ensembl Assembly Exception Feature
** @nam4rule Displayidentifier Fetch the display identifier
**
** @argrule * aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @argrule Displayidentifier Pidentifier [AjPStr*] Display identifier address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyexceptionfeatureFetchDisplayidentifier ********************
**
** Fetch the display identifier of an Ensembl Assembly Exception Feature.
**
** The caller is responsible for deletiung the AJAX String.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature
** @param [wP] Pidentifier [AjPStr*] Display identifier String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureFetchDisplayidentifier(
    const EnsPAssemblyexceptionfeature aef,
    AjPStr *Pidentifier)
{
    if (!aef)
        return ajFalse;

    if (!Pidentifier)
        return ajFalse;

    if (!aef->ExceptionSlice)
        return ajFalse;

    if (*Pidentifier)
        ajStrAssignS(
            Pidentifier,
            ensSliceGetSeqregionName(aef->ExceptionSlice));
    else
        *Pidentifier = ajStrNewS(
            ensSliceGetSeqregionName(aef->ExceptionSlice));

    return ajTrue;
}




/* @datasection [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
** Exception Feature Adaptor
**
** @nam2rule Assemblyexceptionfeatureadaptor Functions for manipulating
** Ensembl Assembly Exception Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyExceptionFeatureAdaptor
** @cc CVS Revision: 1.21
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic assemblyexceptionfeatureadaptorCacheInit ***********************
**
** Initialise an Ensembl Assembly Exception Feature Adaptor-internal
** Ensembl Assembly Exception Feature cache.
**
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool assemblyexceptionfeatureadaptorCacheInit(
    EnsPAssemblyexceptionfeatureadaptor aefa)
{
    ajuint erid    = 0U;
    ajuint srid    = 0U;
    ajuint erstart = 0U;
    ajuint srstart = 0U;
    ajuint erend   = 0U;
    ajuint srend   = 0U;

    ajuint *Pidentifier = NULL;

    AjPList aes = NULL;

    EnsPAssemblyexception ae         = NULL;
    EnsPAssemblyexceptionadaptor aea = NULL;

    EnsPAssemblyexceptionfeature refaef = NULL;
    EnsPAssemblyexceptionfeature excaef = NULL;

    EnsPFeature feature = NULL;

    EnsPSlice excslice  = NULL;
    EnsPSlice refslice  = NULL;
    EnsPSliceadaptor sa = NULL;

    if (!aefa)
        return ajFalse;

    if (!aefa->CacheByIdentifier)
    {
        ajDebug("assemblyexceptionfeatureadaptorCacheInit CacheByIdentifier "
                "not initialised!\n");

        return ajFalse;
    }

    aea = ensRegistryGetAssemblyexceptionadaptor(aefa->Adaptor);

    sa = ensRegistryGetSliceadaptor(aefa->Adaptor);

    aes = ajListNew();

    ensAssemblyexceptionadaptorFetchAll(aea, aes);

    while (ajListPop(aes, (void **) &ae))
    {
        srid = ensAssemblyexceptionGetReferenceSeqregion(ae);

        srstart = ensAssemblyexceptionGetReferenceStart(ae);

        srend = ensAssemblyexceptionGetReferenceEnd(ae);

        erid = ensAssemblyexceptionGetExceptionSeqregion(ae);

        erstart = ensAssemblyexceptionGetExceptionStart(ae);

        erend = ensAssemblyexceptionGetExceptionEnd(ae);

        /*
        ** Each Ensembl Assembly Exception creates two
        ** Ensembl Assembly Exception Feature objects, each of which has
        ** an alternative Slice pointing to the "other" one. Thereby, the
        ** Feature is annotated on the Slice spanning the entire
        ** Ensembl Sequence Region. The alternate Slice spans only the
        ** exception region.
        */

        /* For the reference Slice ... */

        ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                  srid,
                                                  0,
                                                  0,
                                                  0,
                                                  &refslice);

        ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                  erid,
                                                  erstart,
                                                  erend,
                                                  0,
                                                  &excslice);

        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    refslice,
                                    srstart,
                                    srend,
                                    1);

        refaef = ensAssemblyexceptionfeatureNewIni(
            aefa,
            ensAssemblyexceptionGetIdentifier(ae),
            feature,
            excslice,
            ensAssemblyexceptionGetType(ae));

        ensFeatureDel(&feature);

        ensSliceDel(&excslice);

        ensSliceDel(&refslice);

        /* Insert the (reference) Assembly Exception Feature into the cache. */

        ajListPushAppend(aefa->Cache, (void *) refaef);

        AJNEW0(Pidentifier);

        *Pidentifier = refaef->Identifier;

        ajTablePut(aefa->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensAssemblyexceptionfeatureNewRef(refaef));

        /* For the exception Slice ... */

        ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                  erid,
                                                  0,
                                                  0,
                                                  0,
                                                  &excslice);

        ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                  srid,
                                                  srstart,
                                                  srend,
                                                  0,
                                                  &refslice);

        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    excslice,
                                    erstart,
                                    erend,
                                    1);

        excaef = ensAssemblyexceptionfeatureNewIni(
            aefa,
            ensAssemblyexceptionGetIdentifier(ae),
            feature,
            refslice,
            ensAssemblyexceptionGetType(ae));

        ensFeatureDel(&feature);

        ensSliceDel(&excslice);

        ensSliceDel(&refslice);

        /* Insert the (exception) Assembly Exception Feature into the cache. */

        ajListPushAppend(aefa->Cache, (void *) excaef);

        ensAssemblyexceptionDel(&ae);
    }

    ajListFree(&aes);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly Exception Feature Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly Exception Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway
**
** @fdata [EnsPAssemblyexceptionfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature Adaptor
** @argrule Ref object [EnsPAssemblyexceptionfeature]
** Ensembl Assembly Exception Feature Adaptor
**
** @valrule * [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyexceptionfeatureadaptorNew ********************************
**
** Default constructor for an Ensembl Assembly Exception Feature Adaptor.
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
** @see ensRegistryGetAssemblyexceptionfeatureadaptor
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyExceptionFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAssemblyexceptionfeatureadaptor aefa = NULL;

    if (!dba)
        return NULL;

    AJNEW0(aefa);

    aefa->Adaptor = dba;

    aefa->Cache = ajListNew();

    aefa->CacheByIdentifier = ajTableuintNew(0);

    ajTableSetDestroyvalue(
        aefa->CacheByIdentifier,
        (void (*)(void **)) &ensAssemblyexceptionfeatureDel);

    aefa->CacheBySlice = ensCacheNew(
        ensECacheTypeAlphaNumeric,
        assemblyexceptionfeatureadaptorKCacheMaxBytes,
        assemblyexceptionfeatureadaptorKCacheMaxCount,
        assemblyexceptionfeatureadaptorKCacheMaxSize,
        (void *(*)(void *value)) NULL,
        (void (*)(void **Pvalue)) NULL,
        (size_t (*)(const void *value)) NULL,
        (void *(*)(const void *key)) NULL,
        (AjBool (*)(const void *value)) NULL,
        ajFalse,
        "Assembly Exception Feature");

    assemblyexceptionfeatureadaptorCacheInit(aefa);

    return aefa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Assembly Exception Feature Adaptor object.
**
** @fdata [EnsPAssemblyexceptionfeatureadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Assembly Exception Feature Adaptor
**
** @argrule * Paefa [EnsPAssemblyexceptionfeatureadaptor*]
** Ensembl Assembly Exception Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblyexceptionfeatureadaptorDel ********************************
**
** Default destructor for an Ensembl Assembly Exception Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Paefa [EnsPAssemblyexceptionfeatureadaptor*]
** Ensembl Assembly Exception Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAssemblyexceptionfeatureadaptorDel(
    EnsPAssemblyexceptionfeatureadaptor *Paefa)
{
    EnsPAssemblyexceptionfeature        aef   = NULL;
    EnsPAssemblyexceptionfeatureadaptor pthis = NULL;

    if (!Paefa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAssemblyexceptionfeatureadaptorDel"))
        ajDebug("ensAssemblyexceptionfeatureadaptorDel\n"
                "  *Paefa %p\n",
                *Paefa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Paefa)
        return;

    pthis = *Paefa;

    while (ajListPop(pthis->Cache, (void **) &aef))
        ensAssemblyexceptionfeatureDel(&aef);

    ajTableClearDelete(pthis->CacheByIdentifier);

    AJFREE(pthis);

    *Paefa = NULL;

    return;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Assembly Exception Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPAssemblyexceptionfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Assembly Exception Feature object(s)
** @nam4rule All   Fetch all Ensembl Assembly Exception Feature objects
** @nam4rule Allby Fetch all Ensembl Assembly Exception Feature objects
**                 matching a criterion
** @nam5rule Slice Fetch by an Ensembl Slice
** @nam4rule By    Fetch one Ensembl Assembly Exception Feature object
**                 matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @argrule All aefs [AjPList]
** AJAX List of Ensembl Assembly Exception Feature objects
** @argrule AllbySlice slice [EnsPSlice]
** Ensembl Slice
** @argrule AllbySlice aefs [AjPList]
** AJAX List of Ensembl Assembly Exception Feature objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Paef [EnsPAssemblyexceptionfeature*]
** Ensembl Assembly Exception Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionfeatureadaptorFetchAll ***************************
**
** Fetch all Ensembl Assembly Exception Feature objects.
**
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @param [u] aefs [AjPList]
** AJAX List of Ensembl Assembly Exception Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureadaptorFetchAll(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    AjPList aefs)
{
    AjIList iter = NULL;

    EnsPAssemblyexceptionfeature aef = NULL;

    if (!aefa)
        return ajFalse;

    if (!aefs)
        return ajFalse;

    iter = ajListIterNewread(aefa->Cache);

    while (!ajListIterDone(iter))
    {
        aef = (EnsPAssemblyexceptionfeature) ajListIterGet(iter);

        ajListPushAppend(aefs,
                         (void *) ensAssemblyexceptionfeatureNewRef(aef));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic assemblyexceptionfeatureadaptorRemap ***************************
**
** Remap Ensembl Assembly Exception Feature objects onto an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyExceptionFeatureAdaptor::_remap
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @param [u] aefs [AjPList]
** AJAX List of Ensembl Assembly Exception Feature objects
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool assemblyexceptionfeatureadaptorRemap(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    AjPList aefs,
    EnsPAssemblymapper am,
    EnsPSlice slice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    ajuint srid = 0U;

    AjIList iter = NULL;
    AjPList mrs  = NULL;

    EnsPAssemblyexceptionfeature aef = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    if (!aefa)
        return ajFalse;

    if (!aefs)
        return ajFalse;

    if (!slice)
        return ajFalse;

    /*
    ** Remapping is not required, if the AJAX List is empty or the Slice
    ** attached to the first Feature is already identical to the Slice
    ** the Feature objects should be mapped to.
    */

    if (!ajListGetLength(aefs))
        return ajTrue;

    ajListPeekFirst(aefs, (void **) &aef);

    feature = aef->Feature;

    if (ensSliceMatch(ensFeatureGetSlice(feature), slice))
        return ajTrue;

    /* Remapping has not been done, we have to do our own conversion. */

    mrs = ajListNew();

    iter = ajListIterNew(aefs);

    while (!ajListIterDone(iter))
    {
        aef = (EnsPAssemblyexceptionfeature) ajListIterGet(iter);

        feature = aef->Feature;

        /*
        ** Since Feature objects were obtained in contig coordinates, the
        ** attached Sequence Region is a contig.
        */

        if (!feature->Slice)
            ajFatal("assemblyexceptionfeatureadaptorRemap got an "
                    "Ensembl Feature (%p) without an Ensembl Slice.\n",
                    feature);

        if (ensCoordsystemMatch(ensSliceGetCoordsystemObject(slice),
                                ensSliceGetCoordsystemObject(feature->Slice)))
        {
            /*
            ** The Slice attached to the Feature is in the same
            ** Coordinate System as the target Slice, therefore remapping and
            ** an Ensembl Assembly Mapper are not required. Nevertheless,
            ** coordinates need still adjusting to the Slice.
            */

            srid   = ensSliceGetSeqregionIdentifier(feature->Slice);
            start  = feature->Start;
            end    = feature->End;
            strand = feature->Strand;
        }
        else
        {
            /*
            ** The Slice attached to the Feature is in a different
            ** Coordinate System, therefore remapping is required.
            */

            if (!am)
                ajFatal("assemblyexceptionfeatureadaptorRemap requires an "
                        "Ensembl Assembly Mapper, when "
                        "Coordinate System objects of Feature objects and "
                        "Slice differ.\n");

            ensAssemblymapperFastmap(am,
                                     ensSliceGetSeqregion(feature->Slice),
                                     feature->Start,
                                     feature->End,
                                     feature->Strand,
                                     mrs);

            /*
            ** The ensMapperFastmap function returns at maximum one Ensembl
            ** Mapper Result. An empty AJAX List of Ensembl Mapper Results
            ** means a gap, so remove the Ensembl Object from the AJAX List
            ** of Ensembl Objects and delete it.
            */

            if (ajListGetLength(mrs))
            {
                ajListPeekFirst(mrs, (void **) &mr);

                srid   = ensMapperresultGetObjectidentifier(mr);
                start  = ensMapperresultGetCoordinateStart(mr);
                end    = ensMapperresultGetCoordinateEnd(mr);
                strand = ensMapperresultGetCoordinateStrand(mr);

                while (ajListPop(mrs, (void **) &mr))
                    ensMapperresultDel(&mr);
            }
            else
            {
                ajListIterRemove(iter);

                ensAssemblyexceptionfeatureDel(&aef);

                continue;
            }
        }

        if ((srid != ensSliceGetSeqregionIdentifier(slice)) ||
            (start > ensSliceGetEnd(slice)) ||
            (end   < ensSliceGetStart(slice)))
        {
            /*
            ** Since the Feature maps to a region outside the desired area,
            ** remove the Ensembl Object from the AJAX List and delete it.
            */

            ajListIterRemove(iter);

            ensAssemblyexceptionfeatureDel(&aef);

            continue;
        }

        /*
        ** FIXME: In contrast to the Perl API the Assembly Exception Feature
        ** Adaptor does currently not use a Slice cache. While the Perl API
        ** keeps Feature objects on Slice objects spanning the entire
        ** Sequence Region, a new copy of the Feature needs to be placed on
        ** the requested Slice.
        ** Since we are currently not using a Slice cache, the Feature can be
        ** remapped in place.
        ** FIXME: This means in fact that this function is now again
        ** completely identical to the ensFeatureadaptorRemap function.
        */

        /* Shift the Feature start, end and strand in one call. */

        if (ensSliceGetStrand(slice) > 0)
            ensFeatureMove(feature,
                           start - ensSliceGetStart(slice) + 1,
                           end   - ensSliceGetStart(slice) + 1,
                           +strand);
        else
            ensFeatureMove(feature,
                           ensSliceGetEnd(slice) - end   + 1,
                           ensSliceGetEnd(slice) - start + 1,
                           -strand);

        ensFeatureSetSlice(feature, slice);
    }

    ajListIterDel(&iter);

    ajListFree(&mrs);

    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureadaptorFetchAllbySlice ********************
**
** Fetch all Ensembl Assembly Exception Feature objects via an Ensembl Slice.
**
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] aefs [AjPList]
** AJAX List of Ensembl Assembly Exception Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureadaptorFetchAllbySlice(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    EnsPSlice slice,
    AjPList aefs)
{
    AjPList css = NULL;

    AjPStr name = NULL;

    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystem mcs = NULL;
    EnsPCoordsystem scs = NULL;

    EnsPMetacoordinateadaptor mca = NULL;

    if (!aefa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!aefs)
        return ajFalse;

    /* Return Feature objects from the Slice cache if present. */

    /*
    ** FIXME: The Perl API cache uses the Slice name as the key.
    ** If Feature objects are requested for a Slice that is based on the same
    ** Sequence Region, but has differnt start end coordinates the Feature
    ** cache does not work, as the Slice name would be differnt.
    ** Therefore, the same set of Feature objects could be cached under
    ** different Slice names.
    ** Wouldn't it be better to use the Sequence Region identifier as the
    ** cache key and do the re-mapping from there?
    */

    /* TODO: Implement Slice cache.
       my $key= uc($slice->name());
    */

    /* TODO: Implement Slice cache.
       if (exists($self->{'_aexc_slice_cache'}->{$key})) {
       return $self->{'_aexc_slice_cache'}->{$key};
       }
    */

    ensAssemblyexceptionfeatureadaptorFetchAll(aefa, aefs);

    mca = ensRegistryGetMetacoordinateadaptor(aefa->Adaptor);

    name = ajStrNewC("assembly_exception");

    css = ajListNew();

    ensMetacoordinateadaptorFetchAllCoordsystems(mca, name, css);

    ama = ensRegistryGetAssemblymapperadaptor(aefa->Adaptor);

    scs = ensSliceGetCoordsystemObject(slice);

    while (ajListPop(css, (void **) &mcs))
    {
        if (ensCoordsystemMatch(mcs, scs))
            am = NULL;
        else
            ensAssemblymapperadaptorFetchByCoordsystems(ama, mcs, scs, &am);

        /*
        ** FIXME: assemblyexceptionfeatureadaptorRemap is completely identical
        ** to featureadaptorRemap.
        */
        assemblyexceptionfeatureadaptorRemap(aefa, aefs, am, slice);

        ensCoordsystemDel(&mcs);
    }

    /* TODO: Implement Slice cache.
       $self->{'_aexc_slice_cache'}->{$key} =\@ features;
    */

    ajListFree(&css);

    ajStrDel(&name);

    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureadaptorFetchByIdentifier ******************
**
** Fetch all Ensembl Assembly Exception Feature objects.
**
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Paef [EnsPAssemblyexceptionfeature*]
** Ensembl Assembly Exception Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureadaptorFetchByIdentifier(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    ajuint identifier,
    EnsPAssemblyexceptionfeature *Paef)
{
    if (!aefa)
        return ajFalse;

    *Paef = (EnsPAssemblyexceptionfeature) ajTableFetchmodV(
        aefa->CacheByIdentifier,
        (const void *) &identifier);

    ensAssemblyexceptionfeatureNewRef(*Paef);

    return ajTrue;
}

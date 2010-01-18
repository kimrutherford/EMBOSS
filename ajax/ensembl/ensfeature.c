/******************************************************************************
** @source Ensembl Feature functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.5 $
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

#include "ensfeature.h"




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPAnalysisadaptor
ensRegistryGetAnalysisadaptor(EnsPDatabaseadaptor dba);

extern EnsPAssemblyexceptionadaptor
ensRegistryGetAssemblyexceptionadaptor(EnsPDatabaseadaptor dba);

extern EnsPAssemblyexceptionfeatureadaptor
ensRegistryGetAssemblyexceptionfeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPAssemblymapperadaptor
ensRegistryGetAssemblymapperadaptor(EnsPDatabaseadaptor dba);

extern EnsPCoordsystemadaptor
ensRegistryGetCoordsystemadaptor(EnsPDatabaseadaptor dba);

extern EnsPDNAAlignFeatureadaptor
ensRegistryGetDNAAlignFeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPExternaldatabaseadaptor
ensRegistryGetExternaldatabaseadaptor(EnsPDatabaseadaptor dba);

extern EnsPMetainformationadaptor
ensRegistryGetMetainformationadaptor(EnsPDatabaseadaptor dba);

extern EnsPMetacoordinateadaptor
ensRegistryGetMetacoordinateadaptor(EnsPDatabaseadaptor dba);

extern EnsPProteinalignfeatureadaptor
ensRegistryGetProteinalignfeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPProteinfeatureadaptor
ensRegistryGetProteinfeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPSimplefeatureadaptor
ensRegistryGetSimplefeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static AjBool featureAdaptorAnalysisNameToConstraint(
    const EnsPFeatureadaptor adaptor,
    AjPStr* Pconstraint,
    const AjPStr anname);

static AjBool featureAdaptorRemap(EnsPFeatureadaptor adaptor,
                                  AjPList objects,
                                  EnsPAssemblymapper am,
                                  EnsPSlice slice);

static AjBool featureAdaptorSliceFetch(EnsPFeatureadaptor adaptor,
                                       EnsPSlice slice,
                                       AjPStr constraint,
                                       AjPList objects);

static int baseAlignFeatureCompareSourceFeatureStart(const void* P1,
                                                     const void* P2);

static AjBool baseAlignFeatureParseFeatures(EnsPBasealignfeature baf,
                                            AjPList objects);

static AjBool baseAlignFeatureParseCigar(const EnsPBasealignfeature baf,
                                         AjPList fps);

static void *baseAlignFeatureadaptorCacheReference(void *value);

static void baseAlignFeatureadaptorCacheDelete(void **value);

static ajuint baseAlignFeatureadaptorCacheSize(const void *value);

static EnsPFeature baseAlignFeatureadaptorGetFeature(const void *value);

static AjBool dnaAlignFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement,
                                                  EnsPAssemblymapper mapper,
                                                  EnsPSlice slice,
                                                  AjPList bafs);

static AjBool proteinAlignFeatureadaptorFetchAllBySQL(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper mapper,
    EnsPSlice slice,
    AjPList bafs);

static AjBool proteinFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                 const AjPStr statement,
                                                 EnsPAssemblymapper mapper,
                                                 EnsPSlice slice,
                                                 AjPList pfs);

static AjBool simpleFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                const AjPStr statement,
                                                EnsPAssemblymapper mapper,
                                                EnsPSlice slice,
                                                AjPList sfs);

static void *simpleFeatureadaptorCacheReference(void *value);

static void simpleFeatureadaptorCacheDelete(void **value);

static ajuint simpleFeatureadaptorCacheSize(const void *value);

static EnsPFeature simpleFeatureadaptorGetFeature(const void *value);

static AjBool assemblyExceptionFeatureadaptorCacheInit(
    EnsPAssemblyexceptionfeatureadaptor aefa);

static AjBool assemblyExceptionFeatureadaptorCacheClear(
    EnsPAssemblyexceptionfeatureadaptor adaptor);

static AjBool assemblyExceptionFeatureadaptorRemap(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    AjPList aefs,
    EnsPAssemblymapper am,
    EnsPSlice slice);




static ajuint ensFeatureadaptorMaxSplitQuerySeqregions = 3;




/* @filesection ensfeature ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPFeature] Feature *****************************************
**
** Functions for manipulating Ensembl Feature objects
**
** @cc Bio::EnsEMBL::Feature CVS Revision: 1.32
**
** @nam2rule Feature
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
** @fnote None
**
** @nam3rule New Constructor
** @suffix S Constructor with an Ensembl Slice
** @suffix N Constructor with a sequence name
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPFeature] Ensembl Feature
** @argrule Ref object [EnsPFeature] Ensembl Feature
**
** @valrule * [EnsPFeature] Ensembl Feature
**
** @fcategory new
******************************************************************************/




/* @func ensFeatureNewS *******************************************************
**
** Default Ensembl Feature constructor based on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand orientation
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensFeatureNewS(EnsPAnalysis analysis,
                           EnsPSlice slice,
                           ajint start,
                           ajint end,
                           ajint strand)
{
    EnsPFeature feature = NULL;
    
    if(!slice)
    {
        ajDebug("ensFeatureNewS reqires an Ensembl Slice.\n");
	
	return NULL;
    }
    
    if(start && end && (start > (end + 1)))
    {
        ajDebug("ensFeatureNewS start (%u) must be less than or equal to "
                "end (%u) + 1.\n", start, end);
	
        return NULL;
    }
    
    if((strand < -1) || (strand > 1))
    {
        ajDebug("ensFeatureNewS strand (%d) must be +1, 0 or -1.\n",
                strand);
	
        return NULL;
    }
    
    AJNEW0(feature);
    
    feature->Analysis     = ensAnalysisNewRef(analysis);
    feature->Slice        = ensSliceNewRef(slice);
    feature->SequenceName = NULL;
    feature->Start        = start;
    feature->End          = end;
    feature->Strand       = strand;
    feature->Use          = 1;
    
    return feature;
}




/* @func ensFeatureNewN *******************************************************
**
** Alternative Ensembl Feature constructor based on a sequence name.
**
** This is useful for Features that are not annotated on a (genome sequence)
** Ensembl Slice, such as Protein Features.
**
** @cc Bio::EnsEMBL::Feature::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] seqname [AjPStr] Sequence name
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand orientation
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensFeatureNewN(EnsPAnalysis analysis,
                           AjPStr seqname,
                           ajint start,
                           ajint end,
                           ajint strand)
{
    EnsPFeature feature = NULL;
    
    if(!seqname)
    {
        ajDebug("ensFeatureNewN requires a sequence name.\n");
	
	return NULL;
    }
    
    if(start && end && (start > (end + 1)))
    {
        ajDebug("ensFeatureNewN start (%u) must be less than or equal to "
                "end (%u) + 1.\n", start, end);
	
        return NULL;
    }
    
    if((strand < -1) || (strand > 1))
    {
        ajDebug("ensFeatureNewN strand (%d) must be +1, 0 or -1.\n",
                strand);
	
        return NULL;
    }
    
    AJNEW0(feature);
    
    feature->Analysis = ensAnalysisNewRef(analysis);
    
    feature->Slice = NULL;
    
    if(seqname)
        feature->SequenceName = ajStrNewRef(seqname);
    
    feature->Start  = start;
    feature->End    = end;
    feature->Strand = strand;
    feature->Use    = 1;
    
    return feature;
}




/* @func ensFeatureNewObj *****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPFeature] Ensembl Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensFeatureNewObj(const EnsPFeature object)
{
    EnsPFeature feature = NULL;
    
    AJNEW0(feature);
    
    feature->Analysis = ensAnalysisNewRef(object->Analysis);
    feature->Slice    = ensSliceNewRef(object->Slice);
    
    if(object->SequenceName)
        feature->SequenceName = ajStrNewRef(object->SequenceName);
    
    feature->Start  = object->Start;
    feature->End    = object->End;
    feature->Strand = object->Strand;
    
    feature->Use = 1;
    
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
** @@
******************************************************************************/

EnsPFeature ensFeatureNewRef(EnsPFeature feature)
{
    if(!feature)
	return NULL;
    
    feature->Use++;
    
    return feature;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Feature.
**
** @fdata [EnsPFeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Feature object
**
** @argrule * Pfeature [EnsPFeature*] Feature object address
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
** @@
******************************************************************************/

void ensFeatureDel(EnsPFeature *Pfeature)
{
    EnsPFeature pthis = NULL;
    
    if(!Pfeature)
        return;
    
    if(!*Pfeature)
        return;

    pthis = *Pfeature;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pfeature = NULL;
	
	return;
    }
    
    ensAnalysisDel(&pthis->Analysis);
    
    ensSliceDel(&pthis->Slice);
    
    ajStrDel(&pthis->SequenceName);
    
    AJFREE(pthis);

    *Pfeature = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
** @fnote None
**
** @nam3rule Get Return Feature attribute(s)
** @nam4rule GetAnalysis Return the Ensembl Analysis
** @nam4rule GetSlice Return the Ensembl Slice
** @nam4rule GetSequenceName Return the sequence name
** @nam4rule GetStart Return the start
** @nam4rule GetEnd Return the end
** @nam4rule GetStrand Return the strand
**
** @argrule * feature [const EnsPFeature] Feature
**
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis
** @valrule Slice [EnsPSlice] Ensembl Slice
** @valrule SequenceName [AjPStr] Sequence name
** @valrule Start [ajint] Start
** @valrule End [ajint] End
** @valrule Strand [ajint] Strand
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureGetAnalysis ************************************************
**
** Get the Ensembl Analysis element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::analysis
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
** @@
******************************************************************************/

EnsPAnalysis ensFeatureGetAnalysis(const EnsPFeature feature)
{
    if(!feature)
        return NULL;
    
    return feature->Analysis;
}




/* @func ensFeatureGetSlice ***************************************************
**
** Get the Ensembl Slice element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::slice
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [EnsPSlice] Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPSlice ensFeatureGetSlice(const EnsPFeature feature)
{
    if(!feature)
        return NULL;
    
    return feature->Slice;
}




/* @func ensFeatureGetSequenceName ********************************************
**
** Get the sequence name element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seqname
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [AjPStr*] Sequence name or NULL
** @@
******************************************************************************/

AjPStr ensFeatureGetSequenceName(const EnsPFeature feature)
{
    if(!feature)
        return NULL;
    
    return feature->SequenceName;
}




/* @func ensFeatureGetStart ***************************************************
**
** Get the start coordinate element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::start
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Start coordinate or 0
** @@
******************************************************************************/

ajint ensFeatureGetStart(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    return feature->Start;
}




/* @func ensFeatureGetEnd *****************************************************
**
** Get the end coordinate element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::end
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] End coordinate or 0
** @@
******************************************************************************/

ajint ensFeatureGetEnd(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    return feature->End;
}




/* @func ensFeatureGetStrand **************************************************
**
** Get the strand orientation element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::strand
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Strand orientation or 0
** @@
******************************************************************************/

ajint ensFeatureGetStrand(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    return feature->Strand;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
** @fnote None
**
** @nam3rule Set Set one element of a Feature
** @nam4rule SetAnalysis Set the Ensembl Analysis
** @nam4rule SetSlice Set the Ensembl Slice
** @nam4rule SetSequenceName Set the sequence name
** @nam4rule SetStart Set the start
** @nam4rule SetEnd Set the end
** @nam4rule SetStrand Set the strand
**
** @argrule * feature [EnsPFeature] Ensembl Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensFeatureSetAnalysis ************************************************
**
** Set the Ensembl Analysis element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::analysis
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureSetAnalysis(EnsPFeature feature, EnsPAnalysis analysis)
{
    if(!feature)
        return ajFalse;
    
    ensAnalysisDel(&feature->Analysis);
    
    feature->Analysis = ensAnalysisNewRef(analysis);
    
    return ajTrue;
}




/* @func ensFeatureSetSlice ***************************************************
**
** Set the Ensembl Slice element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::slice
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureSetSlice(EnsPFeature feature, EnsPSlice slice)
{
    if(!feature)
        return ajFalse;
    
    ensSliceDel(&feature->Slice);
    
    feature->Slice = ensSliceNewRef(slice);
    
    return ajTrue;
}




/* @func ensFeatureSetSequenceName ********************************************
**
** Set the sequence name element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seqname
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] seqname [AjPStr] Sequence name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureSetSequenceName(EnsPFeature feature, AjPStr seqname)
{
    if(!feature)
        return ajFalse;
    
    ajStrDel(&feature->SequenceName);
    
    feature->SequenceName = ajStrNewRef(seqname);
    
    return ajTrue;
}




/* @func ensFeatureSetStart ***************************************************
**
** Set the start coordinate element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::start
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] start [ajint] Start coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureSetStart(EnsPFeature feature, ajint start)
{
    if(!feature)
        return ajFalse;
    
    feature->Start = start;
    
    return ajTrue;
}




/* @func ensFeatureSetEnd *****************************************************
**
** Set the end coordinate element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::end
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] end [ajint] End coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureSetEnd(EnsPFeature feature, ajint end)
{
    if(!feature)
        return ajFalse;
    
    feature->End = end;
    
    return ajTrue;
}




/* @func ensFeatureSetStrand **************************************************
**
** Set the strand orientation element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::strand
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] strand [ajint] Strand orientation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureSetStrand(EnsPFeature feature, ajint strand)
{
    if(!feature)
        return ajFalse;
    
    feature->Strand = strand;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Feature object.
**
** @fdata [EnsPFeature]
** @nam3rule Trace Report Ensembl Feature elements to debug file
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
** @@
******************************************************************************/

AjBool ensFeatureTrace(const EnsPFeature feature, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!feature)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensFeatureTrace %p\n"
	    "%S  Slice %p\n"
	    "%S  Start %d\n"
	    "%S  End %d\n"
	    "%S  Strand %d\n"
	    "%S  Analysis %p\n"
	    "%S  SequenceName '%S'\n"
	    "%S  Use %u\n",
	    indent, feature,
	    indent, feature->Slice,
	    indent, feature->Start,
	    indent, feature->End,
	    indent, feature->Strand,
	    indent, feature->Analysis,
	    indent, feature->SequenceName,
	    indent, feature->Use);
    
    ensSliceTrace(feature->Slice, level + 1);
    
    ensAnalysisTrace(feature->Analysis, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensFeatureGetLength **************************************************
**
** Get the length of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::length
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajuint] Length
** @@
******************************************************************************/

ajuint ensFeatureGetLength(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    return (feature->End - feature->Start + 1);
}




/* @func ensFeatureGetMemSize *************************************************
**
** Get the memory size in bytes of an Ensembl Feature.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensFeatureGetMemSize(const EnsPFeature feature)
{
    ajuint size = 0;
    
    if(!feature)
	return 0;
    
    size += (ajuint) sizeof (EnsOFeature);
    
    size += ensSliceGetMemSize(feature->Slice);
    
    size += ensAnalysisGetMemSize(feature->Analysis);
    
    if (feature->SequenceName)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(feature->SequenceName);
    }
    
    return size;
}




/* @func ensFeatureGetSeqregion ***********************************************
**
** Get the Ensembl Sequence Region element of the
** Ensembl Slice element of an Ensembl Feature.
**
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [const EnsPSeqregion] Ensembl Sequence Region
** @@
******************************************************************************/

const EnsPSeqregion ensFeatureGetSeqregion(const EnsPFeature feature)
{
    if(!feature)
        return NULL;
    
    if(!feature->Slice)
	return NULL;
    
    return ensSliceGetSeqregion(feature->Slice);
}




/* @func ensFeatureGetSeqregionName *******************************************
**
** Get the name element of the Ensembl Sequence Region element of the
** Ensembl Slice element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_name
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [const AjPStr] Ensembl Sequence Region name
** @@
******************************************************************************/

const AjPStr ensFeatureGetSeqregionName(const EnsPFeature feature)
{
    if(!feature)
        return NULL;
    
    if(!feature->Slice)
	return NULL;
    
    return ensSliceGetSeqregionName(feature->Slice);
}




/* @func ensFeatureGetSeqregionStart ******************************************
**
** Get the start coordinate of an Ensembl Feature relative to the
** Ensembl Sequence Region element of the Ensembl Slice element of an
** Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_start
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Start coordinate on an Ensembl Sequence Region
** @@
******************************************************************************/

ajint ensFeatureGetSeqregionStart(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    if(!feature->Slice)
	return 0;
    
    if(ensSliceGetStrand(feature->Slice) >= 0)
	return ensSliceGetStart(feature->Slice) + feature->Start - 1;

    return ensSliceGetEnd(feature->Slice) - feature->End + 1;
}




/* @func ensFeatureGetSeqregionEnd ********************************************
**
** Get the end coordinate of an Ensembl Feature relative to the
** Ensembl Sequence Region element of the Ensembl Slice element of an
** Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_end
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] End coordinate on an Ensembl Sequence Region
** @@
******************************************************************************/

ajint ensFeatureGetSeqregionEnd(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    if(!feature->Slice)
	return 0;
    
    if(ensSliceGetStrand(feature->Slice) >= 0)
	return ensSliceGetStart(feature->Slice) + feature->End - 1;
    
    return ensSliceGetStart(feature->Slice) - feature->Start + 1;
}




/* @func ensFeatureGetSeqregionStrand *****************************************
**
** Get the strand information of an Ensembl Feature relative to the
** Ensembl Sequence Region element of the Ensembl Slice element of an
** Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_strand
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajint] Strand information on an Ensembl Sequence Region
** @@
******************************************************************************/

ajint ensFeatureGetSeqregionStrand(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    if(!feature->Slice)
	return 0;
    
    return ensSliceGetStrand(feature->Slice) * feature->Strand;
}




/* @func ensFeatureGetSeqregionLength *****************************************
**
** Get the length element of the Ensembl Sequence Region element of the
** Ensembl Slice element of an Ensembl Feature.
**
** @cc Bio::EnsEMBL::Feature::seq_region_length
** @param [r] feature [const EnsPFeature] Ensembl Feature
**
** @return [ajuint] Ensembl Sequence Region length
** @@
******************************************************************************/

ajuint ensFeatureGetSeqregionLength(const EnsPFeature feature)
{
    if(!feature)
        return 0;
    
    if(!feature->Slice)
	return 0;
    
    return ensSliceGetSeqregionLength(feature->Slice);
}




/* @func ensFeatureMove *******************************************************
**
** Move an Ensembl Feature on its Slice. This function sets the start and end
** coordinate, as well as the strand orientation simultaneously.
**
** @cc Bio::EnsEMBL::Feature::move
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] start [ajint] Start coodinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand orientation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureMove(EnsPFeature feature,
                      ajint start,
                      ajint end,
                      ajint strand)
{
    if(!feature)
        return ajFalse;
    
    if(start && end && (end < start))
    {
        ajDebug("ensFeatureMove start (%u) must be less than or equal to the "
                "end coordinate (%u).\n", start, end);
	
        return ajFalse;
    }
    
    if((strand < -1) || (strand > 1))
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
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @param [u] pslist [AjPList] AJAX List of Ensembl Projection Segments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This projection function does not move a Feature to another Slice, but it
** provides a definition of where a Feature lies in another Coordinate System.
** This is especially useful to see where a Feature would lie in a
** Coordinate System in which it crosses (Sequence Region) boundaries.
**
** Ensembl Projection Segments contain source coordinates releative to the
** Feature start and a target Slice spanning the region in the requested
** Coordinate System this Feature projects to.
**
** If the Feature projects entirely into a gap an empty AJAX List will be
** returned.
******************************************************************************/

AjBool ensFeatureProject(const EnsPFeature feature,
                         const AjPStr csname,
                         const AjPStr csversion,
                         AjPList pslist)
{
    ajint strand = 0;
    
    EnsPSlice fslice    = NULL;
    EnsPSlice nslice    = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!feature)
    {
	ajDebug("ensFeatureProject requires an Ensembl Feature.\n");
	
	return ajFalse;
    }
    
    /* A Coordinate System name and version is not mandatory. */
    
    if(!pslist)
    {
	ajDebug("ensFeatureProject requires an AJAX List.\n");
	
	return ajFalse;
    }
    
    if(!feature->Slice)
    {
	ajWarn("ensFeatureProject requires an Ensembl Feature with "
	       "an Ensembl Slice attached to it.\n");
	
	return ajFalse;
    }
    
    /*
    ** Use the Ensembl Database Adaptor of the Slice as this Feature may not
    ** yet be stored in the database and may not have its own Adaptor.
    */
    
    sa = ensSliceGetAdaptor(feature->Slice);
    
    if(!sa)
    {
	ajWarn("ensFeatureProject requires an Ensembl Feature with "
	       "an Ensembl Slice Adaptor element attached to the "
	       "Ensembl Slice element.\n");
	
	return ajFalse;
    }
    
    strand = feature->Strand * ensSliceGetStrand(feature->Slice);
    
    /*
    ** The ensSliceadaptorFetchByFeature function always returns a
    ** forward-strand Slice.
    */
    
    /*
    ** FIXME: So far this is the only instance of ensSliceFetchInvertedSlice.
    ** Wouldn't ensSliceadaptorFetchByFeature be better if it allowed the
    ** specification of a strand?
    ** FIXME: Also, ensSliceadaptorFetchByFeature should be separated into
    ** ensSliceadaptorFetchByFeatureFlank and
    ** ensSliceadaptorFetchByFeaturePercentage.
    */
    
    ensSliceadaptorFetchByFeature(sa, feature, 0, &fslice);
    
    if(strand < 0)
	ensSliceFetchInvertedSlice(fslice, &nslice);
    else
	nslice = ensSliceNewRef(fslice);
    
    ensSliceDel(&fslice);
    
    ensSliceProject(nslice, csname, csversion, pslist);
    
    ensSliceDel(&nslice);
    
    return ajTrue;
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
**
** @return [EnsPFeature] Ensembl Feature or NULL
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
** chromosome coodinates will place the Exon on a Slice of an entire
** chromosome.
******************************************************************************/

EnsPFeature ensFeatureTransform(EnsPFeature feature,
                                const AjPStr csname,
                                const AjPStr csversion)
{
    AjPList pslist = NULL;
    
    const AjPStr pcsname    = NULL;
    const AjPStr pcsversion = NULL;
    const AjPStr psrname    = NULL;
    
    EnsPCoordsystem cs         = NULL;
    const EnsPCoordsystem pcs  = NULL;
    EnsPCoordsystemadaptor csa = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPFeature nfeature = NULL;
    
    EnsPProjectionsegment ps = NULL;
    
    EnsPSlice nslice       = NULL;
    const EnsPSlice pslice = NULL;
    EnsPSliceadaptor sa    = NULL;
    
    if(!feature)
    {
	ajDebug("ensFeatureTransform requires an Ensembl Feature.\n");
	
	return NULL;
    }
    
    if(!csname)
    {
	ajDebug("ensFeatureTransform requires a Coordinate System name.\n");
	
	return NULL;
    }
    
    /* FIXME: A Coordinate System version is not strictly required??? */
#if AJFALSE
    if (! csversion)
    {
        ajDebug("ensFeatureTransform requires a Coordinate System version.\n");
        
        return NULL;
    }
#endif
    
    if(!feature->Slice)
    {
	ajWarn("ensFeatureTransform requires an Ensembl Feature with "
	       "an Ensembl Slice attached to it.\n");
	
	return NULL;
    }
    
    /*
    ** Use the Ensembl Database Adaptor of the Slice as this Feature may not
    ** yet be stored in the database and may not have its own Adaptor.
    */
    
    sa = ensSliceGetAdaptor(feature->Slice);
    
    if(!sa)
    {
	ajWarn("ensFeatureTransform requires an Ensembl Feature with "
	       "an Ensembl Slice Adaptor element attached to the "
	       "Ensembl Slice element.\n");
	
	return NULL;
    }
    
    if(!ensSliceGetCoordsystem(feature->Slice))
    {
	ajWarn("ensFeatureTransform requires an Ensembl Feature with "
	       "an Ensembl Coordinate System element attached to the "
	       "Ensembl Slice element.\n");
	
	return NULL;
    }
    
    dba = ensSliceadaptorGetDatabaseadaptor(sa);
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);
    
    if(!cs)
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
    ** cover the entire Sequence Region for consitency?
    ** ensSliceadaptorFetchByRegion below would return a Slice covering the
    ** full Sequence Region.
    */
    
    if(ensCoordsystemMatch(cs, ensSliceGetCoordsystem(feature->Slice)) &&
       (ensSliceGetStart(feature->Slice) == 1) &&
       (ensSliceGetStrand(feature->Slice) >= 0))
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
    
    pslist = ajListNew();
    
    ensFeatureProject(feature, csname, csversion, pslist);
    
    if(ajListGetLength(pslist) == 1)
    {
	pslice = ensProjectionsegmentGetTrgSlice(ps);
	
	pcs = ensSliceGetCoordsystem(pslice);
	
	pcsname = ensCoordsystemGetName(pcs);
	
	pcsversion = ensCoordsystemGetVersion(pcs);
	
	psrname = ensSliceGetSeqregionName(pslice);
	
	/*
	** TODO: This could all be simpler if we had an
	** ensSliceadaptorFetchBySeqregion function.
	** We could create the Slice directly from the Sequence Region, but
	** going through the Adaptor has the big advantage that the Slice
	** would be inserted into the Adaptor-internal cache, which would
        ** reduce the overall memory footprint.
        */
	
	ensSliceadaptorFetchByRegion(sa,
				     pcsname,
				     pcsversion,
				     psrname,
				     0,
				     0,
				     1,
				     &nslice);
	
	nfeature = ensFeatureNewObj(feature);
	
	nfeature->Start = ensSliceGetStart(pslice);
	
	nfeature->End = ensSliceGetEnd(pslice);
	
	nfeature->Strand =
	    (feature->Strand == 0) ? 0 : ensSliceGetStrand(pslice);
	
	ensFeatureSetSlice(nfeature, nslice);
	
	ensSliceDel(&nslice);
    }
    
    while(ajListPop(pslist, (void **) &ps))
	ensProjectionsegmentDel(&ps);
    
    ajListFree(&pslist);
    
    ensCoordsystemDel(&cs);
    
    return nfeature;
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
    ajint nfend = 0;
    
    const EnsPCoordsystem srccs = NULL;
    const EnsPCoordsystem trgcs = NULL;
    
    EnsPFeature nf = NULL;
    
    if(!feature)
    {
	ajDebug("ensFeatureTransfer requires an Ensembl Feature.\n");
	
	return NULL;
    }
    
    if(!feature->Slice)
    {
	ajDebug("ensFeatureTransfer requires an Ensembl Feature with "
		"an Ensembl Slice attached.");
	
	return NULL;
    }
    
    if(!slice)
    {
	ajDebug("ensFeatureTransfer requires an Ensembl Slice.\n");
	
	return NULL;
    }
    
    srccs = ensSliceGetCoordsystem(feature->Slice);
    
    trgcs = ensSliceGetCoordsystem(slice);
    
    /*
    ** If the Coordinate Systems of the Feature and the Slice are identical,
    ** adjust only coordinates. In case they are not, the Feature needs
    ** transforming into the target Cordinate System first.
    */
    
    if(ensCoordsystemMatch(srccs, trgcs))
	nf = ensFeatureNewObj(feature);
    else
    {
	nf = ensFeatureTransform(feature,
				 ensCoordsystemGetName(trgcs),
				 ensCoordsystemGetVersion(trgcs));
	
	if(!nf)
	{
	    ajDebug("ensFeatureTransfer got no Feature from "
		    "ensFeatureTransform.\n");
	    
	    return NULL;
	}
    }
    
    /*
    ** Discard Features that were placed on an entirely different
    ** Sequence Region.
    **
    ** FIXME: Shouldn't this test first take the Identifier into account and
    ** in case it was not available from both use the name for comparison?
    ** Sequence Regions in different coordinate systems may have the same
    ** name e.g. clone and contig.
    **
    ** We should really use ensSeqregionMatch for this.
    **
    ** But why is this test there at all?
    ** The Perl API method description claims that this method can transform
    ** into another Coordinate System. In another Coordinate System however,
    ** the Sequence Region is very likely to have a different name!
    ** So why are we testing for it?
    **
    ** Shouldn't this test only be performed in case the source and the target
    ** Coordinate Systems are identical? Then it would make sense to check
    ** for identical Sequence Region elements.
    **
    ** Also if the second Slice is based on the same Sequence Region but
    ** shorter, we would end up with Features with negative coordinates.
    ** Test this!
    */
    
    if(!ajStrMatchCaseS(ensSliceGetSeqregionName(nf->Slice),
                        ensSliceGetSeqregionName(slice)))
    {
	ajDebug("ensFeatureTransfer got a Feature on a different "
		"Sequence Region '%S' vs '%S'.\n",
		ensSliceGetSeqregionName(nf->Slice),
		ensSliceGetSeqregionName(slice));
	
	ensFeatureDel(&nf);
	
	return NULL;
    }
    
    /*
    ** If the current Feature positions are not relative to the start of the
    ** Sequence Region, convert them so that they are.
    */
    
    if((ensSliceGetStart(nf->Slice) != 1) ||
       (ensSliceGetStrand(nf->Slice) < 0))
    {
	nfstart = nf->Start;
	
	nfend = nf->End;
	
	if(ensSliceGetStrand(nf->Slice) >= 0)
	{
	    nf->Start = nfstart + ensSliceGetStart(nf->Slice) - 1;
	    
	    nf->End = nfend + ensSliceGetStart(nf->Slice) - 1;
	}
	else
	{
	    nf->Start = ensSliceGetEnd(nf->Slice) - nfend + 1;
	    
	    nf->End = ensSliceGetEnd(nf->Slice) - nfstart + 1;
	    
	    nf->Strand *= -1;
	}
    }
    
    /* Convert to target Slice coordinates. */
    
    nfstart = nf->Start;
    
    nfend = nf->End;
    
    if(ensSliceGetStrand(slice) >= 0)
    {
	nf->Start = nfstart - ensSliceGetStart(slice) + 1;
	
	nf->End = nfend - ensSliceGetStart(slice) + 1;
    }
    else
    {
	nf->Start = ensSliceGetEnd(slice) - nfend + 1;
	
	nf->End = ensSliceGetEnd(slice) - nfstart + 1;
	
	nf->Strand *= -1;
    }
    
    ensFeatureSetSlice(nf, slice);
    
    return nf;
}




/* @func ensFeatureFetchAllAlternativeLocations *******************************
**
** Fetch all alternative locations of an Ensembl Feature on other symlinked
** Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Features before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Feature::get_all_alt_locations
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] all [AjBool] Fetch all Features
** @param [u] features [AjPList] AJAX List of Ensembl Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

/*
** TODO: The Perl API requires a second parameter $return_all,
** which is not documented!
*/

AjBool ensFeatureFetchAllAlternativeLocations(EnsPFeature feature,
                                              AjBool all,
                                              AjPList features)
{
    ajint alength = 0;
    ajint rlength = 0;
    
    ajuint srid = 0;
    
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
    
    if(!feature)
	return ajFalse;
    
    fslice = feature->Slice;
    
    if(!fslice)
	return ajTrue;
    
    sa = ensSliceGetAdaptor(fslice);
    
    if(!sa)
	return ajTrue;
    
    /*
    ** Fetch all Ensembl Assembly Exception Features for the full-length Slice
    ** of the Ensembl Sequence Region element.
    */
    
    dba = ensSliceadaptorGetDatabaseadaptor(sa);
    
    aefa = ensRegistryGetAssemblyexceptionfeatureadaptor(dba);
    
    srid = ensSliceGetSeqregionIdentifier(fslice);
    
    ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &rslice);
    
    aefs = ajListNew();
    
    ensAssemblyexceptionfeatureadaptorFetchAllBySlice(aefa, rslice, aefs);
    
    /*
    ** Group Ensembl Assembly Exception Features based on their type element
    ** into haplotypes (HAPs) and pseudo-autosomal regions (PARs) initially.
    */
    
    haps = ajListNew();
    
    alts = ajListNew();
    
    while(ajListPop(aefs, (void **) &aef))
    {
	switch(ensAssemblyexceptionfeatureGetType(aef))
	{
	    case ensEAssemblyexceptionTypeHAP:
		
		ajListPushAppend(haps, (void *) aef);
		
		break;
		
	    case ensEAssemblyexceptionTypePAR:
		
		ajListPushAppend(alts, (void *) aef);
		
		break;
		
	    case ensEAssemblyexceptionTypeHAPREF:
		
		if(all)
		    ajListPushAppend(haps, (void *) aef);
		
		break;
		
	    default:
		
		ajDebug("ensFeatureFetchAllAlternativeLocations got unknown "
			"EnsEAssemblyexceptionType (%d).\n",
			ensAssemblyexceptionfeatureGetType(aef));
	}
    }
    
    ajListFree(&aefs);
    
    /*
    ** Regions surrounding haplotypes are those of interest, not the haplotype
    ** itself. Convert haplotype Assembly Exception Features to regions
    ** around haplotypes instead.
    */
    
    while (ajListPop(haps, (void **) &aef))
    {
	afeature = ensAssemblyexceptionfeatureGetFeature(aef);
	
	aslice = ensAssemblyexceptionfeatureGetAlternateSlice(aef);
	
	if((ensFeatureGetStart(afeature) > 1) &&
           (ensSliceGetStart(aslice) > 1))
	{
	    /*
            ** TODO: Rewrite this, as it could be much simpler using
	    ** ensAssemblyexceptionfeatureNewObj and then
	    ** ensAssemblyexceptionfeatureGetFeature
	    ** ensFeatureNewObj
	    ** ensFeatureSetStart
	    ** ensFeatureSetEnd
	    ** ensAssemblyexceptionfeatureGetAlternateSlice
	    ** ensSliceNewObj
	    ** ensSliceSetStart
	    ** ensSliceSetEnd
	    ** ensAssemblyexceptionfeatureSetFeature
	    ** ensAssemblyexceptionfeatureSetAlternateSlice
	    ** ensFeatureDel
	    ** ensSliceDel
	    **
	    ** TODO: There could be an ensSliceadaptorFetchBySlice function,
	    ** which maintains the Sequence Region and the Coordinate System
	    ** and just allows for altering the start end and strand.
	    ** The benefit would be that this function could inset the Slice
	    ** into the Adaptor-internal cache. This function could also be
	    ** used internally by ensSliceFetchInvertedSlice and other
	    ** Slice functions.
	    */
	    
	    /* Copy the Feature and re-set the start and end cordinates. */
	    
	    nfeature = ensFeatureNewObj(afeature);
	    
	    ensFeatureSetStart(nfeature, 1);
	    
	    ensFeatureSetEnd(nfeature, ensFeatureGetStart(afeature) - 1);
	    
	    ensSliceadaptorFetchByRegion(sa,
					 ensSliceGetCoordsystemName(aslice),
					 ensSliceGetCoordsystemVersion(aslice),
					 ensSliceGetSeqregionName(aslice),
					 1,
					 ensSliceGetStart(aslice) - 1,
					 ensSliceGetStrand(aslice),
					 &nslice);
	    
	    naef = ensAssemblyexceptionfeatureNew(
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
	
	if(ensSliceGetSeqregionLength(rslice) <= INT_MAX)
	    rlength = ensSliceGetSeqregionLength(rslice);
	else
	    ajFatal("ensFeatureFetchAllAlternativeLocations got "
		    "Sequence Region length (%u) exceeding MAX_INT (%d).\n",
		    ensSliceGetSeqregionLength(rslice),
		    INT_MAX);
	
	if(ensSliceGetSeqregionLength(aslice) <= INT_MAX)
	    alength = ensSliceGetSeqregionLength(aslice);
	else
	    ajFatal("ensFeatureFetchAllAlternativeLocations got "
		    "Sequence Region length (%u) exceeding MAX_INT (%d).\n",
		    ensSliceGetSeqregionLength(aslice),
		    INT_MAX);
	
	if((ensFeatureGetEnd(afeature) < rlength) &&
	   (ensSliceGetEnd(aslice) < alength))
	{
	    /* Copy the Feature and re-set the start and end cordinates. */
	    
	    nfeature = ensFeatureNewObj(afeature);
	    
	    ensFeatureSetStart(nfeature, ensFeatureGetEnd(afeature) + 1);
	    
	    ensFeatureSetEnd(nfeature, ensFeatureGetSeqregionLength(afeature));
	    
	    ensSliceadaptorFetchByRegion(sa,
					 ensSliceGetCoordsystemName(aslice),
					 ensSliceGetCoordsystemVersion(aslice),
					 ensSliceGetSeqregionName(aslice),
					 ensSliceGetEnd(aslice) + 1,
					 ensSliceGetSeqregionLength(aslice),
					 ensSliceGetStrand(aslice),
					 &nslice);
	    
	    naef = ensAssemblyexceptionfeatureNew(
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
    
    while(ajListPop(alts, (void **) &aef))
    {
	afeature = ensAssemblyexceptionfeatureGetFeature(aef);
	
	aslice = ensAssemblyexceptionfeatureGetAlternateSlice(aef);
	
	/* Ignore the other region if the Feature is not entirely on it. */
	
	if((ensFeatureGetSeqregionStart(feature) <
	    ensFeatureGetStart(afeature)) ||
	   (ensFeatureGetSeqregionEnd(feature) > ensFeatureGetEnd(afeature)))
	{
	    ensAssemblyexceptionfeatureDel(&aef);
	    
	    continue;
	}
	
	nfeature = ensFeatureNewObj(feature);
	
	/* Position the Feature on the entire Slice of the other region. */
	
	nfeature->Start =
	    ensFeatureGetSeqregionStart(nfeature) -
	    ensFeatureGetStart(afeature) +
	    ensSliceGetStart(aslice);
	
	nfeature->End =
	    ensFeatureGetSeqregionEnd(nfeature) -
	    ensFeatureGetStart(afeature) +
	    ensSliceGetStart(aslice);
	
	nfeature->Strand *= ensSliceGetStrand(aslice);
	
	/*
	** Place the new Features on the full-length Slice of the
	** Ensembl Sequence Region element.
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




/* @func ensFeatureOverlap ****************************************************
**
** Tests whether two Ensembl Features overlap.
**
** @cc Bio::EnsEMBL::Feature::overlaps
** @param [r] feature1 [const EnsPFeature] First Ensembl Feature
** @param [r] feature2 [const EnsPFeature] Second Ensembl Feature
**
** @return [AjBool] ajTrue if the Features overlap on the same
**                  Ensembl Sequence Region, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureOverlap(const EnsPFeature feature1,
                         const EnsPFeature feature2)
{
    AjPStr name1 = NULL;
    AjPStr name2 = NULL;
    
    if(!feature1)
	return ajFalse;
    
    if(!feature2)
	return ajFalse;
    
    name1 = ajStrNew();
    name2 = ajStrNew();
    
    ensFeatureFetchSeqregionName(feature1, &name1);
    ensFeatureFetchSeqregionName(feature2, &name2);
    
    if((name1 && name2) && (! ajStrMatchCaseS(name1, name2)))
    {
	ajDebug("ensFeatureOverlap got Features on different "
	        "Ensembl Sequence Regions.\n");
	
	ajStrDel(&name1);
	ajStrDel(&name2);
	
	return ajFalse;
    }
    
    ajStrDel(&name1);
    ajStrDel(&name2);
    
    return ((feature1->End >= feature2->Start) &&
	    (feature1->Start <= feature2->End));
}




/* @func ensFeatureFetchSeqregionName *****************************************
**
** Fetch the Ensembl Sequence Region name, which can be the sequence name
** element set in the Ensembl Feature or, if not available, the
** Ensembl Slice name.
**
** @cc Bio::EnsEMBL::Feature::seqname
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [wP] Pname [AjPStr*] Ensembl Sequence Region name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureFetchSeqregionName(const EnsPFeature feature, AjPStr* Pname)
{
    if(!feature)
        return ajFalse;
    
    if(!Pname)
	return ajFalse;
    
    if(feature->SequenceName && ajStrGetLen(feature->SequenceName))
	ajStrAssignS(Pname, feature->SequenceName);
    else if(feature->Slice)
	ensSliceFetchName(feature->Slice, Pname);
    
    return ajTrue;
}




/* @datasection [EnsPFeatureadaptor] Feature Adaptor **************************
**
** Functions for manipulating Ensembl Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor CVS Revision: 1.86
**
** @nam2rule Featureadaptor
**
******************************************************************************/

/* 1 << 26 = 64 Mi */
static ajuint featureAdaptorCacheMaxBytes = 1 << 26;

/* 1 << 16 = 64 ki */
static ajuint featureAdaptorCacheMaxCount = 1 << 16;

static ajuint featureAdaptorCacheMaxSize = 0;




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Feature Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPFeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @argrule Ref object [EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @valrule * [EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensFeatureadaptorNew *************************************************
**
** Default Ensembl Feature Adaptor constructor.
**
** For Ensembl Collection Core databases storing information about multiple
** species, only Ensembl Features for a particular species encoded in the
** Ensembl Database Adaptor need to be selected.
** This is achieved by automatically adding additional joins to the
** 'seq_region' and the 'coord_system' table, which contains the
** 'coord_system.species_id' field to the default SQL condition.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] Ptables [const char**] Table names
** @param [r] Pcolumns [const char**] Column names
** @param [r] left_join [EnsOBaseadaptorLeftJoin*] Ensembl Base Adaptor
**                                                 Left Join Statements
** @param [r] condition [const char *] Default SQL condition
** @param [r] final [const char *] Final SQL condition
** @param [f] Fquery [AjBool function] Function address to handle the
**                              Ensembl Object-specific SQL statement
** @param [f] Fread [void* function] Function address to read further value
**                                   data not yet cached
** @param [f] Freference [void* function] Function address to reference value
**                                        data, i.e. increment an internal use
**                                        counter
** @param [f] Fwrite [AjBool function] Function address to write back modified
**                                     cache entries
** @param [f] Fdelete [void function] Function address to delete objects
** @param [f] Fsize [ajuint function] Function address to determine the
**                             value data (or object) memory size
** @param [f] Fgetfeature [EnsPFeature function] Pointer to an Ensembl Object
**                                               specific function to get an
**                                               Ensembl Feature
** @param [r] label [const char*] Cache label included in statistics output
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
** @@
** NOTE: For Ensembl Features stored in Ensembl Core databases with multiple
** species, the Sequence Region in a '*_feature' table needs joining to
** the 'seq_region' table and the 'coord_system' table, which holds the
** species identifier field.
** TODO: The Perl API adds the constraint as extra default where clause in the
** Base Adaptor. This is clearly not the right place as not every Object the
** Base Adaptor fetches is a Feature. Features are those objects that are
** associated with a Sequence Region so that the Sequence Region constraint
** is more natural here.
** TODO: Suggest to the Ensembl Core team!
******************************************************************************/

EnsPFeatureadaptor ensFeatureadaptorNew(
    EnsPDatabaseadaptor dba,
    const char **Ptables,
    const char **Pcolumns,
    EnsOBaseadaptorLeftJoin *left_join,
    const char *condition,
    const char *final,
    AjBool Fquery(EnsPDatabaseadaptor dba,
                  const AjPStr statement,
                  EnsPAssemblymapper mapper,
                  EnsPSlice slice,
                  AjPList objects),
    void* Fread(const void* key),
    void* Freference(void* value),
    AjBool Fwrite(const void* value),
    void Fdelete(void** value),
    ajuint Fsize(const void* value),
    EnsPFeature Fgetfeature(const void *object),
    const char *label)
{
    register ajuint i = 0;
    
    EnsPFeatureadaptor fa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(fa);
    
    if(ensDatabaseadaptorGetMultiSpecies(dba))
    {
	/* Extend the array of table names. */
	
	AJCNEW0(fa->Tables, sizeof (Ptables) + 2);
	
	for(i = 0; Ptables[i]; i++)
	    fa->Tables[i] = ajCharNewC(Ptables[i]);
	
	fa->Tables[i] = ajCharNewC("seq_region");
	
	i++;
	
	fa->Tables[i] = ajCharNewC("coord_system");
	
	i++;
	
	fa->Tables[i] = (char *) NULL;
	
	Ptables = (const char **) fa->Tables;
	
	/* Extend the default SQL condition. */
	
	if(condition)
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
			     
	condition = fa->Condition;
    }
    
    fa->Adaptor = ensBaseadaptorNew(dba,
				    Ptables,
				    Pcolumns,
				    left_join,
				    condition,
				    final,
				    Fquery);
    
    fa->Cache = ensCacheNew(ensECacheTypeNumeric,
			    featureAdaptorCacheMaxBytes,
			    featureAdaptorCacheMaxCount,
			    featureAdaptorCacheMaxSize,
			    Freference,
			    Fdelete,
			    Fsize,
			    Fread,
			    Fwrite,
			    ajFalse,
			    label);
    
    fa->MaxFeatureLength = 0;
    
    fa->GetFeature = Fgetfeature;
    
    fa->Reference = Freference;
    
    fa->Delete = Fdelete;
    
    return fa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Feature Adaptor.
**
** @fdata [EnsPFeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Feature Adaptor object
**
** @argrule * Padaptor [EnsPFeatureadaptor*] Ensembl Feature Adaptor
**                                           object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensFeatureadaptorDel *************************************************
**
** Default destructor for an Ensembl Feature Adaptor.
**
** @param [d] Padaptor [EnsPFeatureadaptor*] Ensembl Feature Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensFeatureadaptorDel(EnsPFeatureadaptor *Padaptor)
{
    register ajuint i = 0;
    EnsPFeatureadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    ensBaseadaptorDel(&pthis->Adaptor);
    
    ensCacheDel(&pthis->Cache);
    
    /* Clear the array of table names. */
    
    if(pthis->Tables)
    {
	for(i = 0; (*Padaptor)->Tables[i]; i++)
	    ajCharDel(&((*Padaptor)->Tables[i]));
	
	AJFREE((*Padaptor)->Tables);
    }
    
    /* Clear the default SQL condition. */
    
    if(pthis->Condition)
    {
	ajCharDel(&pthis->Condition);
    }
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Feature Adaptor object.
**
** @fdata [EnsPFeatureadaptor]
** @fnote None
**
** @nam3rule Get Return Ensembl Feature Adaptor attribute(s)
** @nam4rule GetBaseadaptor Return the Ensembl Base Adaptor
** @nam4rule GetCache Return the Ensembl Cache
** @nam4rule GetMaxFeatureLength Return the maximum Feature length
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * adaptor [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @valrule Cache [EnsPCache] Ensembl Cache
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureadaptorGetBaseadaptor **************************************
**
** Get the Ensembl Base Adaptor element of an Ensembl Feature Adaptor.
**
** @param [r] adaptor [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensFeatureadaptorGetBaseadaptor(
    const EnsPFeatureadaptor adaptor)
{
    if(!adaptor)
	return NULL;
    
    return adaptor->Adaptor;
}




/* @func ensFeatureadaptorGetCache ********************************************
**
** Get the Ensembl Cache element of an Ensembl Feature Adaptor.
**
** @param [r] adaptor [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [EnsPCache] Ensembl Cache or NULL
** @@
******************************************************************************/

EnsPCache ensFeatureadaptorGetCache(const EnsPFeatureadaptor adaptor)
{
    if(!adaptor)
	return NULL;
    
    return adaptor->Cache;
}




/* @func ensFeatureadaptorGetMaxFeatureLength *********************************
**
** Get the maximum Feature length element of an Ensembl Feature Adaptor.
**
** @param [r] adaptor [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [ajuint] Maximum Feature length or 0
** @@
******************************************************************************/

ajuint ensFeatureadaptorGetMaxFeatureLength(const EnsPFeatureadaptor adaptor)
{
    if(!adaptor)
	return 0;
    
    return adaptor->MaxFeatureLength;
}




/* @func ensFeatureadaptorGetDatabaseadaptor **********************************
**
** Get the Ensembl Database Adaptor element of the
** Ensembl Base Adaptor element of an Ensembl Feature Adaptor.
**
** @param [r] adaptor [const EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensFeatureadaptorGetDatabaseadaptor(
    const EnsPFeatureadaptor adaptor)
{
    if(!adaptor)
	return NULL;
    
    return ensBaseadaptorGetDatabaseadaptor(adaptor->Adaptor);
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Feature Adaptor object.
**
** @fdata [EnsPFeatureadaptor]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Feature Adaptor
** @nam4rule SetMaxFeatureLength Set the maximum Feature length
** @nam4rule SetTables Set the tables
** @nam4rule SetColumns Set the columns
** @nam4rule SetDefaultCondition Set the default SQL condition
** @nam4rule SetFinalCondition Set the final SQL condition
**
** @argrule * adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensFeatureadaptorSetMaximumFeatureLength *****************************
**
** Set the maximum Feature length element of an Ensembl Feature Adaptor.
**
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] length [ajuint] Maximum Feature length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetMaximumFeatureLength(EnsPFeatureadaptor adaptor,
                                                ajuint length)
{
    if(!adaptor)
	return ajFalse;
    
    adaptor->MaxFeatureLength = length;
    
    return ajTrue;
}




/* @func ensFeatureadaptorSetTables *******************************************
**
** Set the tables element of the Ensembl Base Adaptor element of an
** Ensembl Feature Adaptor.
**
** For Ensembl Collection Core databases storing information about multiple
** species, only Ensembl Features for a particular species encoded in the
** Ensembl Database Adaptor need to be selected.
** This is achieved by automatically adding additional joins to the
** 'seq_region' and the 'coord_system' table, which contains the
** 'coord_system.species_id' field to the default SQL condition.
**
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] Ptables [const char**] Table names
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetTables(EnsPFeatureadaptor adaptor,
                                  const char **Ptables)
{
    register ajuint i = 0;
    
    EnsPDatabaseadaptor dba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!Ptables)
	return ajFalse;
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor);
    
    if(ensDatabaseadaptorGetMultiSpecies(dba))
    {
	/* Clear the array of table names. */
	
	if(adaptor->Tables)
	{
	    for(i = 0; adaptor->Tables[i]; i++)
		ajCharDel(&(adaptor->Tables[i]));
	    
	    AJFREE(adaptor->Tables);
	}
	
	/* Extend the array of table names. */
	
	AJCNEW0(adaptor->Tables, sizeof (Ptables) + 2);
	
	for(i = 0; Ptables[i]; i++)
	    adaptor->Tables[i] = ajCharNewC(Ptables[i]);
	
	adaptor->Tables[i] = ajCharNewC("seq_region");
	
	i++;
	
	adaptor->Tables[i] = ajCharNewC("coord_system");
	
	i++;
	
	adaptor->Tables[i] = (char *) NULL;
	
	Ptables = (const char **) adaptor->Tables;
    }
    
    return ensBaseadaptorSetTables(adaptor->Adaptor, Ptables);
}




/* @func ensFeatureadaptorSetColumns ******************************************
**
** Set the columns element of the Ensembl Base Adaptor element of an
** Ensembl Feature Adaptor.
**
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] Pcolumns [const char**] Column names
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetColumns(EnsPFeatureadaptor adaptor,
                                   const char **Pcolumns)
{
    if(!adaptor)
	return ajFalse;
    
    if(!Pcolumns)
	return ajFalse;
    
    return ensBaseadaptorSetColumns(adaptor->Adaptor, Pcolumns);
}




/* @func ensFeatureadaptorSetDefaultCondition *********************************
**
** Set the default SQL condition element of the
** Ensembl Base Adaptor element of an Ensembl Feature Adaptor.
**
** For Ensembl Collection Core databases storing information about multiple
** species, only Ensembl Features for a particular species encoded in the
** Ensembl Database Adaptor need to be selected.
** This is achieved by automatically adding additional joins to the
** 'seq_region' and the 'coord_system' table, which contains the
** 'coord_system.species_id' field to the default SQL condition.
**
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] condition [const char *] Default SQL condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetDefaultCondition(EnsPFeatureadaptor adaptor,
                                            const char *condition)
{
    EnsPDatabaseadaptor dba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(ensDatabaseadaptorGetMultiSpecies(dba))
    {
	/* Clear the default SQL condition. */
	
	if(adaptor->Condition)
	    ajCharDel(&(adaptor->Condition));
	
	/* Extend the default SQL condition. */
	
	if(condition)
	    adaptor->Condition = ajFmtString(
                "%s "
                "AND "
                "%s.seq_region_id = seq_region.seq_region_id "
                "AND "
                "seq_region.coord_system_id = coord_system.coord_system_id "
                "AND "
                "coord_system.species_id = %u",
                condition,
                ensBaseadaptorGetPrimaryTable(adaptor->Adaptor),
                ensDatabaseadaptorGetIdentifier(dba));
	else
	    adaptor->Condition = ajFmtString(
                "%s.seq_region_id = seq_region.seq_region_id "
                "AND "
                "seq_region.coord_system_id = coord_system.coord_system_id "
                "AND "
                "coord_system.species_id = %u",
                ensBaseadaptorGetPrimaryTable(adaptor->Adaptor),
                ensDatabaseadaptorGetIdentifier(dba));
	
	condition = adaptor->Condition;
    }
    
    return ensBaseadaptorSetDefaultCondition(adaptor->Adaptor, condition);
}




/* @func ensFeatureadaptorSetFinalCondition ***********************************
**
** Set the final SQL condition element of the
** Ensembl Base Adaptor element of an Ensembl Base Adaptor.
**
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] final [const char *] Final SQL condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorSetFinalCondition(EnsPFeatureadaptor adaptor,
                                          const char *final)
{
    if(!adaptor)
	return ajFalse;
    
    return ensBaseadaptorSetFinalCondition(adaptor->Adaptor, final);
}




/* @func ensFeatureadaptorEscapeC *********************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the C-type character string.
**
** @param [u] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorEscapeC(EnsPFeatureadaptor adaptor,
                                char **Ptxt,
                                const AjPStr str)
{
    if(!adaptor)
        return ajFalse;
    
    if(!str)
	return ajFalse;
    
#if AJFALSE
    ajDebug("ensFeatureadaptorEscapeC\n"
            "  adaptor %p\n"
            "  Ptxt %p\n"
            "  str '%S'\n",
            adaptor,
            Ptxt,
            str);
    
    ensFeatureadaptorTrace(adaptor, 1);
#endif
    
    return ensBaseadaptorEscapeC(adaptor->Adaptor, Ptxt, str);
}




/* @func ensFeatureadaptorEscapeS *********************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the AJAX String.
**
** @param [u] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorEscapeS(EnsPFeatureadaptor adaptor,
                                AjPStr *Pstr,
                                const AjPStr str)
{
    if(!adaptor)
        return ajFalse;
    
    if(!str)
	return ajFalse;
    
#if AJFALSE
    ajDebug("ensFeatureadaptorEscapeS\n"
            "  adaptor %p\n"
            "  Pstr %p\n"
            "  str '%S'\n",
            adaptor,
            Pstr,
            str);
    
    ensFeatureadaptorTrace(adaptor, 1);
#endif
    
    return ensBaseadaptorEscapeS(adaptor->Adaptor, Pstr, str);
}




/* @funcstatic featureAdaptorAnalysisNameToConstraint *************************
**
** Build an SQL statement constraint from an Ensembl Analysis name.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::_logic_name_to_constraint
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [u] Pconstraint [AjPStr*] Address of an AJAX String constraint that
**                                  will be enhanced by Analysis-specific
**                                  conditions
** @param [rN] anname [const AjPStr] Ensembl Analysis name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Given an Ensembl Analysis name and an existing constraint this will
** add an 'analysis' table constraint to the Feature. Note that if no
** 'analysis_id' exists in the columns of the primary table then no
** constraint is added at all.
******************************************************************************/

static AjBool featureAdaptorAnalysisNameToConstraint(
    const EnsPFeatureadaptor adaptor,
    AjPStr* Pconstraint,
    const AjPStr anname)
{
    const char **columns = NULL;
    const char *table    = NULL;
    
    register ajuint i = 0;
    
    AjBool match = AJFALSE;
    
    EnsPAnalysis analysis = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
#if AJFALSE
    ajDebug("featureAdaptorAnalysisNameToConstraint\n"
            "  adaptor %p\n"
            "  *Pconstraint '%S'\n"
            "  anname '%S'\n",
            adaptor,
            *Pconstraint,
            anname);
#endif
    
    if(!adaptor)
	return ajFalse;
    
    if(!Pconstraint)
	return ajFalse;
    
    if(!anname)
	return ajTrue;
    
    /*
    ** Check that the primary table, which is the first one in the list of
    ** tables (fa->Adaptor->Tables[0]), actually contains an 'analysis_id'
    ** column.
    */
    
    columns = ensBaseadaptorGetColumns(adaptor->Adaptor);
    
    table = ensBaseadaptorGetPrimaryTable(adaptor->Adaptor);
    
    while(columns[i])
    {
	if(ajCharPrefixC(columns[i], table) &&
	   ajCharSuffixC(columns[i], ".analysis_id"))
	    match = ajTrue;
	
	i++;
    }
    
    if(!match)
    {
	ajWarn("featureAdaptorAnalysisNameToConstraint this Ensembl Feature "
	       "is not associated with an Ensembl Analysis. "
	       "Ignoring Analysis name argument '%S'.\n", anname);
	
	return ajFalse;
    }
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor);
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    ensAnalysisadaptorFetchByName(aa, anname, &analysis);
    
    if(!analysis)
	return ajFalse;
    
    if(*Pconstraint)
    {
	if(ajStrGetLen(*Pconstraint))
            ajStrAppendC(Pconstraint, " AND ");
    }
    else
	*Pconstraint = ajStrNew();
    
    ajFmtPrintAppS(Pconstraint,
		   "%s.analysis_id = %u",
		   table,
		   ensAnalysisGetIdentifier(analysis));
    
    ensAnalysisDel(&analysis);
    
    return ajTrue;
}




/* @funcstatic featureAdaptorRemap ********************************************
**
** Remap Ensembl Objects based on Ensembl Features onto an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::_remap
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Features
** @param [rN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool featureAdaptorRemap(EnsPFeatureadaptor adaptor,
                                  AjPList objects,
                                  EnsPAssemblymapper am,
                                  EnsPSlice slice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;
    
    ajuint srcsrid = 0;
    ajuint trgsrid = 0;
    
    void *Pobject = NULL;
    
    AjIList iter = NULL;
    AjPList mrs  = NULL;
    
    EnsPFeature feature = NULL;
    EnsPMapperresult mr = NULL;
    
#if AJFALSE
    ajDebug("featureAdaptorRemap\n"
            "  adaptor %p\n"
            "  objects %p\n"
            "  am %p\n"
            "  slice %p\n",
            adaptor,
            objects,
            am,
            slice);
#endif
    
    if(!adaptor)
	return ajFalse;
    
    if(!objects)
	return ajFalse;
    
    /* FIXME: An Ensembl Assembly Mapper is not strictly required??? */
#if AJFALSE
    if (! am)
        return ajFalse;
#endif
    
    if(!slice)
	return ajFalse;
    
    /* If the List contains no Features remapping is not required. */
    
    if(!ajListGetLength(objects))
	return ajTrue;
    
    /*
    ** If the Slice attached to the first Feature is identical to the Slice
    ** the Features should be mapped to, remapping is not required.
    */
    
    ajListPeekFirst(objects, (void **) &Pobject);
    
    feature = (*adaptor->GetFeature)(Pobject);
    
#if AJFALSE
    ajDebug("featureAdaptorRemap got Ensembl Feature (%p)\n", feature);
     
    ensFeatureTrace(feature, 1);
#endif
    
    if(ensSliceMatch(ensFeatureGetSlice(feature), slice))
	return ajTrue;
    
    /* Remapping has not been done, we have to do our own conversion. */
    
    trgsrid = ensSliceGetSeqregionIdentifier(slice);
    
    iter = ajListIterNew(objects);
    
    while(!ajListIterDone(iter))
    {
	Pobject = ajListIterGet(iter);
	
	feature = (*adaptor->GetFeature)(Pobject);
	
	/*
        ** Since Features were obtained in contig coordinates, the attached
        ** Sequence Region is a contig.
        */
	
	if(!feature->Slice)
	    ajFatal("featureAdaptorRemap Feature (%p) does not have a "
		    "Slice attached.\n", feature);
	
	if(ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                               ensSliceGetCoordsystem(feature->Slice)))
	{
	    /*
	    ** The Slice attached to the Feature is in the same
	    ** Coordinate System as the target Slice, therefore remapping and
	    ** an Ensembl Assembly Mapper are not required. Nevertheless,
	    ** coordinates need still adjusting to the Slice.
	    */
	    
	    start = feature->Start;
	    
	    end = feature->End;
	    
	    strand = feature->Strand;
	    
	    srcsrid = ensSliceGetSeqregionIdentifier(feature->Slice);
	}
	else
	{
	    /*
            ** The Slice attached to the Feature is in a different
	    ** Coordinate System, therefore remapping is required.
	    */
	    
	    if(!am)
		ajFatal("featureAdaptorRemap requires an "
			"Ensembl Assembly Mapper, when "
			"Coordinate Systems of Features and Slice differ.\n");
	    
	    mrs = ajListNew();
	    
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(feature->Slice),
				     feature->Start,
				     feature->End,
				     feature->Strand,
				     mrs);
	    
	    /*
	    ** For successful mapping, the ensMapperFastMap function returns
	    ** at maximum one Mapper Result List element.
	    ** An empty List means a Gap so remove the Ensembl Object from
	    ** the List and de-reference (delete) it.
	    */
	    
	    if(ajListGetLength(mrs))
		ajListPop(mrs, (void **) &mr);
	    else
	    {
		ajListIterRemove(iter);
		
		(*adaptor->Delete)(&Pobject);
		
		continue;
	    }
	    
	    start = ensMapperresultGetStart(mr);
	    
	    end = ensMapperresultGetEnd(mr);
	    
	    strand = ensMapperresultGetStrand(mr);
	    
	    srcsrid = ensMapperresultGetObjectIdentifier(mr);
	    
	    ensMapperresultDel(&mr);
	    
	    ajListFree(&mrs);
	}
	
	if((srcsrid != trgsrid) ||
           (start > ensSliceGetEnd(slice)) ||
           (end < ensSliceGetStart(slice)))
	{
	    /*
	    ** Since the Feature maps to a region outside the desired area,
	    ** remove the Ensembl Object from the AJAX List and delete it.
	    */
	    
	    ajListIterRemove(iter);
	    
	    (*adaptor->Delete)(&Pobject);
	    
	    continue;
	}
	
	/* Shift the Feature start, end and strand in one call. */
	
	if(ensSliceGetStrand(slice) > 0)
	    ensFeatureMove(feature,
			   start - ensSliceGetStart(slice) + 1,
			   end - ensSliceGetStart(slice) + 1,
			   strand);
	else
	    ensFeatureMove(feature,
			   ensSliceGetEnd(slice) - end + 1,
			   ensSliceGetEnd(slice) - start + 1,
			   strand * -1);
	
	ensFeatureSetSlice(feature, slice);
    }
    
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @funcstatic featureAdaptorSliceFetch ***************************************
**
** Helper function used by ensFeatureadaptorFetchAllBySliceConstraint.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::_slice_fetch
** @param [u] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] constraint [AjPStr] SQL statement constraint
** @param [r] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool featureAdaptorSliceFetch(EnsPFeatureadaptor adaptor,
                                       EnsPSlice slice,
                                       AjPStr constraint,
                                       AjPList objects)
{
    const char *table = NULL;
    
    ajuint mrslength = 0;
    ajuint srid = 0;
    
    AjIList iter = NULL;
    
    AjPList cslist = NULL;
    AjPList mis    = NULL; /* Meta-Informations */
    AjPList mrs    = NULL; /* Mapper Results */
    AjPList pos    = NULL; /* Primary Objects */
    
    AjPStr key       = NULL;
    AjPStr tmpconstr = NULL;
    AjPStr tmpstr    = NULL;
    
    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPCoordsystem ftcs       = NULL;
    EnsPCoordsystemadaptor csa = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPMetainformation mi         = NULL;
    EnsPMetainformationadaptor mia = NULL;
    
    EnsPMetacoordinateadaptor mca = NULL;
    
#if AJFALSE
    ajDebug("featureAdaptorSliceFetch\n"
            "  adaptor %p\n"
            "  slice %p\n"
            "  constraint '%S'\n"
            "  objects %p\n",
            adaptor,
            slice,
            constraint,
            objects);
#endif
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!constraint)
	return ajFalse;
    
    if(!objects)
	return ajFalse;
    
    if(!adaptor->Adaptor)
    {
	ajDebug("featureAdaptorSliceFetch got Ensembl Feature Adaptor "
		"without an Ensembl Base Adaptor.\n");
	
	return ajFalse;
    }
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor);
    
    if(!dba)
    {
	ajDebug("featureAdaptorSliceFetch got Ensembl Feature Adaptor "
		"without an Ensembl Database Adaptor.\n");
	
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
    
    table = ensBaseadaptorGetPrimaryTable(adaptor->Adaptor);
    
    key = ajFmtStr("%sbuild.level", table);
    
    mis = ajListNew();
    
    ensMetainformationadaptorFetchAllByKey(mia, key, mis);
    
    ajStrAssignC(&key, table);
    
    cslist = ajListNew();
    
    if(ajListGetLength(mis) && ensSliceIsTopLevel(slice))
	ajListPushAppend(cslist, (void *)
			 ensCoordsystemNewRef(ensSliceGetCoordsystem(slice)));
    else
	ensMetacoordinateadaptorFetchAllCoordsystems(mca, key, cslist);
    
    while(ajListPop(mis, (void **) &mi))
	ensMetainformationDel(&mi);
    
    ajListFree(&mis);
    
    ajStrDel(&key);
    
    /*
    ** Fetch the Features for each Coordinate System they are stored in.
    ** This may require projecting the Slice for which the Features have been
    ** requested into the Coordinate System they have been annotated in.
    */
    
    while(ajListPop(cslist, (void **) &ftcs))
    {
	if(ensCoordsystemMatch(ftcs, ensSliceGetCoordsystem(slice)))
	{
	    /*
	    ** No mapping is required as the Coordinate Systems of Feature
	    ** and Slice are identical.
	    */
	    
	    if(! adaptor->MaxFeatureLength)
	    {
		key = ajStrNewC(table);
		
		adaptor->MaxFeatureLength =
		    ensMetacoordinateadaptorGetMaximumLength(mca, ftcs, key);
		
		ajStrDel(&key);
	    }
	    
	    /* Use external Ensembl Sequence Region identifiers if present. */
	    
	    srid = ensSliceGetSeqregionIdentifier(slice);
	    
	    srid = ensCoordsystemadaptorGetExternalSeqregionIdentifier(csa,
								       srid);
	    
	    tmpconstr = ajStrNewS(constraint);
	    
	    if(ajStrGetLen(tmpconstr))
		ajStrAppendC(&tmpconstr, " AND ");
	    
	    ajFmtPrintAppS(&tmpconstr,
			   "%s.seq_region_id = %u "
			   "AND "
			   "%s.seq_region_start <= %d "
			   "AND "
			   "%s.seq_region_end >= %d",
			   table, srid,
			   table, ensSliceGetEnd(slice),
			   table, ensSliceGetStart(slice));
	    
	    if(adaptor->MaxFeatureLength)
		ajFmtPrintAppS(&tmpconstr,
			       " AND "
			       "%s.seq_region_start >= %d",
			       table,
			       ensSliceGetStart(slice) -
			       (ajint) adaptor->MaxFeatureLength);
	    
	    /* FIXME: (ajint) adaptor->MaxFeatureLength remains a problem! */
	    
	    pos = ajListNew();
	    
	    ensBaseadaptorGenericFetch(adaptor->Adaptor,
				       tmpconstr,
				       (EnsPAssemblymapper) NULL,
				       slice,
				       pos);
	    
	    /*
	    ** Features may still have to have coordinates made relative to
	    ** the Slice start.
	    */
	    
	    /*
	    ** FIXME: The following mapper does not seem to be defined in the
	    ** Perl API, when no mapping is needed. Why is there no error?
	    ** _remap tests whether Coordinate Systems are not equal and only
	    ** executes the mapper code when the Coordinate Systems are not
	    ** equal, but that is exactly not the case when no mapping is
	    ** required.
	    */
	    
	    featureAdaptorRemap(adaptor,
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
	    
	    am = ensAssemblymapperadaptorFetchByCoordsystems(
                ama,
                ftcs,
                ensSliceGetCoordsystem(slice));
	    
	    if(!am)
	    {
		ensCoordsystemDel(&ftcs);
		
		continue;
	    }
	    
	    mrs = ajListNew();
	    
	    /*
	    ** Get the list of coordinates and corresponding internal
	    ** identifiers for the regions the Slice spans.
	    */
	    
	    ensAssemblymapperMap(am,
				 ensSliceGetSeqregion(slice),
				 ensSliceGetStart(slice),
				 ensSliceGetEnd(slice),
				 ensSliceGetStrand(slice),
				 mrs);
	    
	    iter = ajListIterNew(mrs);
	    
	    while(!ajListIterDone(iter))
	    {
		mr = (EnsPMapperresult) ajListIterGet(iter);
		
		/* Remove all Ensembl Mapper Results that represent gaps. */
		
		if(ensMapperresultGetType(mr) == ensEMapperresultGap)
		{
		    ajListIterRemove(iter);
		    
		    ensMapperresultDel(&mr);
		}
	    }
	    
	    ajListIterDel(&iter);
	    
	    mrslength = ajListGetLength(mrs);
	    
	    if(!mrslength)
	    {
		ensCoordsystemDel(&ftcs);
		
		continue;
	    }
	    
	    /*
	    ** When regions are large and only partially spanned by a Slice
	    ** it is faster to limit the query with start and end constraints.
	    ** Take a simple approach and use regional constraints if there
	    ** are less than a specific number of regions covered.
	    */
	    
	    if(mrslength > ensFeatureadaptorMaxSplitQuerySeqregions)
	    {
		tmpconstr = ajStrNewS(constraint);
		
		if(ajStrGetLen(tmpconstr))
		    ajStrAppendC(&tmpconstr, " AND ");
		
		tmpstr = ajStrNew();
		
		while(ajListPop(mrs, (void **) &mr))
		{
		    ajFmtPrintAppS(&tmpstr,
				   "%u, ",
				   ensMapperresultGetObjectIdentifier(mr));
		    
		    ensMapperresultDel(&mr);
		}
		
		/* Remove last comma and space. */
		
		ajStrCutEnd(&tmpstr, 2);
		
		ajFmtPrintAppS(&tmpconstr,
			       "%s.seq_region_id IN (%S)",
			       table,
			       tmpstr);
		
		ajStrDel(&tmpstr);
		
		pos = ajListNew();
		
		ensBaseadaptorGenericFetch(adaptor->Adaptor,
					   tmpconstr,
					   am,
					   slice,
					   pos);
		
		featureAdaptorRemap(adaptor, pos, am, slice);
		
		ajListPushlist(objects, &pos);
		
		ajStrDel(&tmpconstr);
	    }
	    else
	    {
		/*
		** Run multiple split queries using
		** start and end constraints.
		*/
		
		if(!adaptor->MaxFeatureLength)
		{
		    key = ajStrNewC(table);
		    
		    adaptor->MaxFeatureLength =
			ensMetacoordinateadaptorGetMaximumLength(mca,
								 ftcs,
								 key);
		    
		    ajStrDel(&key);
		}
		
		while(ajListPop(mrs, (void **) &mr))
		{
		    tmpconstr = ajStrNewS(constraint);
		    
		    if(ajStrGetLen(tmpconstr))
			ajStrAppendC(&tmpconstr, " AND ");
		    
		    ajFmtPrintAppS(&tmpconstr,
				   "%s.seq_region_id = %u "
				   "AND "
				   "%s.seq_region_start <= %d "
				   "AND "
				   "%s.seq_region_end >= %d",
				   table,
				   ensMapperresultGetObjectIdentifier(mr),
				   table,
				   ensMapperresultGetStart(mr),
				   table,
				   ensMapperresultGetEnd(mr));
		    
		    if(adaptor->MaxFeatureLength)
			ajFmtPrintAppS(&tmpconstr,
				       " AND "
				       "%s.seq_region_start >= %d",
				       table,
				       ensMapperresultGetStart(mr) -
				       (ajint) adaptor->MaxFeatureLength);
		    
		    /*
                    ** FIXME: (ajint) adaptor->MaxFeatureLength remains
                    ** a problem!
                    */
		    
		    pos = ajListNew();
		    
		    ensBaseadaptorGenericFetch(adaptor->Adaptor,
					       tmpconstr,
					       am,
					       slice,
					       pos);
		    
		    ajStrDel(&tmpconstr);
		    
		    featureAdaptorRemap(adaptor, pos, am, slice);
		    
		    ajListPushlist(objects, &pos);
		    
		    ensMapperresultDel(&mr);
		}
	    }
	    
	    ajListFree(&mrs);
	    
	    ensAssemblymapperDel(&am);
	}
	
	ensCoordsystemDel(&ftcs);
    }
    
    ajListFree(&cslist);
    
    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPFeatureadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Feature object(s)
** @nam4rule FetchAll Retrieve all Ensembl Feature objects
** @nam5rule FetchAllBy Retrieve all Ensembl Feature objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Feature object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPFeatureadaptor] Ensembl Feature Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Objects based on
**                             Ensembl Features
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensFeatureadaptorFetchAllBySliceConstraint ***************************
**
** Fetch all Ensembl Objects based on Ensembl Features matching an
** SQL constraint on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice_constraint
** @param [u] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] constraint [const AjPStr] SQL constraint
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorFetchAllBySliceConstraint(EnsPFeatureadaptor adaptor,
                                                  EnsPSlice slice,
                                                  const AjPStr constraint,
                                                  const AjPStr anname,
                                                  AjPList objects)
{
    ajuint srid    = 0;
    ajuint pssrid = 0;
    
    ajint *Pboundary = NULL;
    
    void *Pobject = NULL;
    
    /* AjIList iter = NULL; */
    AjIList bditer = NULL;
    AjIList ftiter = NULL;
    AjIList psiter = NULL;
    
    AjPList list       = NULL;
    AjPList pslist     = NULL;
    AjPList srpslist   = NULL;
    AjPList boundaries = NULL;
    
    AjPStr constr = NULL;
    AjPStr key    = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPFeature ft = NULL;
    
    EnsPProjectionsegment ps = NULL;
    
    EnsPSlice psslice   = NULL;
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
#if AJFALSE
    ajDebug("ensFeatureadaptorFetchAllBySliceConstraint\n"
            "  adaptor %p\n"
            "  slice %p\n"
            "  constraint '%S'\n"
            "  anname '%S'\n"
            "  objects %p\n",
            adaptor,
            slice,
            constraint,
            anname,
            objects);
     
    ensSliceTrace(slice, 1);
#endif
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!adaptor->Adaptor)
    {
	ajDebug("ensFeatureadaptorFetchAllBySliceConstraint got "
		"Ensembl Feature Adaptor without an Ensembl Base Adaptor.\n");
	
	return ajFalse;
    }
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor);
    
    if(!dba)
    {
	ajDebug("ensFeatureadaptorFetchAllBySliceConstraint got "
		"Ensembl Feature Adaptor without an "
		"Ensembl Database Adaptor.\n");
	
	return ajFalse;
    }
    
    constr = ajStrNew();
    
    if(constraint)
	ajStrAssignS(&constr, constraint);
    
    if(!featureAdaptorAnalysisNameToConstraint(adaptor, &constr, anname))
    {
	ajStrDel(&constr);
	
	return ajFalse;
    }
    
    /* Check the cache if this query has been run before. */
    
    key = ajStrNew();
    
    ensSliceFetchName(slice, &key);
    
    ajFmtPrintAppS(&key, ":%S", constr);
    
    /*
    ** FIXME: The Cache must have a List as value???
    **
    ** FIXME: This very crude cache should possibly be removed and the code
    ** could possibly just rely on the SQL server to have queries that have
    ** been run previously cached.
    ** Does that make sense?
    */
    
    list = (AjPList) ensCacheFetch(adaptor->Cache, (void *) key);
    
    /*
    ** TODO: Copy Features from one List to the other, but increase the
    ** reference count. This will be a bit tricky to solve for a generic case.
    ** We have already a pointer to an object-specific referencing function
    ** in the Feature Adaptor.
    */
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    pslist = ajListNew();
    
    /* HAP and PAR support. Retrieve normalised, non-symlinked Slices. */
    
    ensSliceadaptorFetchNormalisedSliceProjection(sa, slice, pslist);
    
    if(!ajListGetLength(pslist))
	ajFatal("ensFeatureadaptorFetchAllBySlice could not get normalised "
		"Slices. Ensembl Core database contains incorrect information "
		"in the 'assembly_exception' table.\n");
    
    /*
    ** Get Features on the full original Slice, as well as any sym-linked
    ** Slices. Filter out partial Slices from Projection Segments that are
    ** based on the same Sequence Region as the original Slice.
    ** A Projection Segment representing the original Slice is added later on.
    */
    
    srid = ensSliceGetSeqregionIdentifier(slice);
    
    psiter = ajListIterNew(pslist);
    
    while(!ajListIterDone(psiter))
    {
	ps = (EnsPProjectionsegment) ajListIterGet(psiter);
	
	psslice = ensProjectionsegmentGetTrgSlice(ps);
	
	pssrid = ensSliceGetSeqregionIdentifier(psslice);
	
	if(pssrid == srid)
	{
	    ajListIterRemove(psiter);
	    
	    ensProjectionsegmentDel(&ps);
	}
    }
    
    ajListIterDel(&psiter);
    
    /* Add back a Projection Segment representing the original Slice. */
    
    ps = ensProjectionsegmentNew(1, ensSliceGetLength(slice), slice);
    
    ajListPushAppend(pslist, (void *) ps);
    
    /*
    ** Construct a list of HAP/PAR boundaries for a Slice spanning the entire
    ** Sequence Region.
    */
    
    /*
    ** FIXME: The question is, do we need to move the boundaries to a separate
    ** List, or could we just keep the Projection Segment List and use them as
    ** they are?
    ** Alternatively, this could be solved with a simple array.
    */
    
    ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);
    
    if(ensSliceGetStrand(slice) >= 0)
	psslice = ensSliceNewRef(srslice);
    else
	ensSliceFetchInvertedSlice(srslice, &psslice);
    
    ensSliceDel(&srslice);
    
    srpslist = ajListNew();
    
    ensSliceadaptorFetchNormalisedSliceProjection(sa, psslice, srpslist);
    
    ensSliceDel(&psslice);
    
    boundaries = ajListNew();
    
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
    
    ajListPop(srpslist, (void **) &ps);
    
    ensProjectionsegmentDel(&ps);
    
    while(ajListPop(srpslist, (void **) &ps))
    {
	AJNEW0(Pboundary);
	
	*Pboundary =
	    ensProjectionsegmentGetSrcStart(ps) - ensSliceGetStart(slice) + 1;
	
	ajListPushAppend(boundaries, (void *) Pboundary);
	
	ensProjectionsegmentDel(&ps);
    }
    
    ajListFree(&srpslist);
    
    /* Fetch Features for the primary Slice and all symlinked Slices. */
    
    while(ajListPop(pslist, (void **) &ps))
    {
	featureAdaptorSliceFetch(adaptor,
				 ensProjectionsegmentGetTrgSlice(ps),
				 constr,
				 objects);
	
	if(!ensSliceMatch(slice, ensProjectionsegmentGetTrgSlice(ps)))
	{
	    /*
            ** Features returned on symlinked Slices need checking that they
            ** do not cross Slice boundaries.
            */
	    
	    ftiter = ajListIterNew(objects);
	    
	    while(!ajListIterDone(ftiter))
	    {
		Pobject = ajListIterGet(ftiter);
		
		ft = (*adaptor->GetFeature)(Pobject);
		
		if(ensProjectionsegmentGetSrcStart(ps) != 1)
		{
		    ft->Start += ensProjectionsegmentGetSrcStart(ps) - 1;
		    
		    ft->End += ensProjectionsegmentGetSrcStart(ps) - 1;
		}
		
		/*
                ** Discard boundary crossing Features from symlinked regions.
                */
		
		bditer = ajListIterNew(boundaries);
		
		while(!ajListIterDone(bditer))
		{
		    Pboundary = (ajint *) ajListIterGet(bditer);
		    
		    if((ft->Start < *Pboundary) && (ft->End >= *Pboundary))
		    {
			ajListIterRemove(ftiter);
			
			ajDebug("ensFeatureadaptorFetchAllBySliceConstraint "
				"got Object (%p) crossing normalised Slice "
				"boundaries.\n", Pobject);
			
			(*adaptor->Delete)(&Pobject);
			
			ft = NULL;
			
			break;
		    }
		}
		
		ajListIterDel(&bditer);
		
		if(ft)
		    ensFeatureSetSlice(ft, slice);
	    }
	    
	    ajListIterDel(&ftiter);
	}
	
	ensProjectionsegmentDel(&ps);
    }
    
    /*
    ** TODO: Push all Objects into the cache, which again may be tricky for
    ** a general case.
    */
    
    ajListFree(&pslist);
    
    /* Delete the boundaries from the List and the AJAX List. */
    
    while(ajListPop(boundaries, (void **) &Pboundary))
	AJFREE(Pboundary);
    
    ajListFree(&boundaries);
    
    ajStrDel(&constr);
    ajStrDel(&key);
    
    return ajTrue;
}




/* @func ensFeatureadaptorFetchAllBySlice *************************************
**
** Fetch all Ensembl Objects based on Ensembl Features on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorFetchAllBySlice(EnsPFeatureadaptor adaptor,
                                        EnsPSlice slice,
                                        const AjPStr anname,
                                        AjPList objects)
{
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    return ensFeatureadaptorFetchAllBySliceConstraint(adaptor,
						      slice,
						      (const AjPStr) NULL,
						      anname,
						      objects);
}




/* @func ensFeatureadaptorFetchAllBySliceScore ********************************
**
** Fetch all Ensembl Objects based on Ensembl Features on an
** Ensembl Slice above a threshold score.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] score [double] Score
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorFetchAllBySliceScore(EnsPFeatureadaptor adaptor,
                                             EnsPSlice slice,
                                             double score,
                                             const AjPStr anname,
                                             AjPList objects)
{
    const char *table = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if (! slice)
	return ajFalse;
    
    table = ensBaseadaptorGetPrimaryTable(adaptor->Adaptor);
    
    constraint = ajFmtStr("%s.score > %lf", table, score);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor,
						       slice,
						       constraint,
						       anname,
						       objects);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensFeatureadaptorFetchAllByAnalysisName ******************************
**
** Fetch all Ensembl Objects based on Ensembl Features by an
** Ensembl Analysis name.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_logic_name
** @param [r] adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeatureadaptorFetchAllByAnalysisName(EnsPFeatureadaptor adaptor,
                                               const AjPStr anname,
                                               AjPList objects)
{
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!anname)
	return ajFalse;
    
    constraint = ajStrNew();
    
    if(!featureAdaptorAnalysisNameToConstraint(adaptor, &constraint, anname))
    {
	ajStrDel(&constraint);
	
	return ajFalse;
    }
    
    value = ensBaseadaptorGenericFetch(adaptor->Adaptor,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       objects);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @datasection [EnsPFeaturepair] Feature Pair ********************************
**
** Functions for manipulating Ensembl Feature Pair objects
**
** @cc Bio::EnsEMBL::FeaturePair CVS Revision: 1.61
**
** @nam2rule Featurepair
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPFeaturepair] Ensembl Feature Pair
** @argrule Ref object [EnsPFeaturepair] Ensembl Feature Pair
**
** @valrule * [EnsPFeaturepair] Ensembl Feature Pair
**
** @fcategory new
******************************************************************************/




/* @func ensFeaturepairNew ****************************************************
**
** Default Ensembl Feature Pair constructor.
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
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairNew(EnsPFeature srcfeature,
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
    
    if(!srcfeature)
	return NULL;
    
    if(!trgfeature)
	return NULL;
    
    AJNEW0(fp);
    
    fp->SourceFeature = ensFeatureNewRef(srcfeature);
    
    fp->TargetFeature = ensFeatureNewRef(trgfeature);
    
    fp->Externaldatabase = ensExternaldatabaseNewRef(edb);
    
    if(extra)
	fp->ExtraData = ajStrNewRef(extra);
    
    if(srcspecies)
	fp->SourceSpecies = ajStrNewRef(srcspecies);
    
    if(trgspecies)
	fp->TargetSpecies = ajStrNewRef(trgspecies);
    
    fp->Use = 1;
    
    fp->Evalue = evalue;
    
    fp->Score = score;
    
    fp->GroupIdentifier = groupid;
    
    fp->LevelIdentifier = levelid;
    
    fp->SourceCoverage = srccoverage;
    
    fp->TargetCoverage = trgcoverage;
    
    fp->SequenceIdentity = identity;
    
    return fp;
}




/* @func ensFeaturepairNewObj *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairNewObj(const EnsPFeaturepair object)
{
    EnsPFeaturepair fp = NULL;
    
    AJNEW0(fp);
    
    fp->SourceFeature    = ensFeatureNewRef(object->SourceFeature);
    fp->TargetFeature    = ensFeatureNewRef(object->TargetFeature);
    fp->Externaldatabase = ensExternaldatabaseNewRef(object->Externaldatabase);
    
    if(object->ExtraData)
	fp->ExtraData = ajStrNewRef(object->ExtraData);
    
    if(object->SourceSpecies)
	fp->SourceSpecies = ajStrNewRef(object->SourceSpecies);
    
    if(object->TargetSpecies)
	fp->TargetSpecies = ajStrNewRef(object->TargetSpecies);
    
    fp->GroupIdentifier = object->GroupIdentifier;
    fp->LevelIdentifier = object->LevelIdentifier;
    
    fp->Use = 1;
    
    fp->Evalue         = object->Evalue;
    fp->Score          = object->Score;
    fp->SourceCoverage = object->SourceCoverage;
    fp->TargetCoverage = object->TargetCoverage;
    
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
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairNewRef(EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    fp->Use++;
    
    return fp;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Feature Pair.
**
** @fdata [EnsPFeaturepair]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Feature Pair object
**
** @argrule * Pfp [EnsPFeaturepair*] Ensembl Feature Pair object address
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
** @@
******************************************************************************/

void ensFeaturepairDel(EnsPFeaturepair *Pfp)
{
    EnsPFeaturepair pthis = NULL;
    
    if(!Pfp)
	return;
    
    if(!*Pfp)
	return;

    pthis = *Pfp;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pfp = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->SourceFeature);
    ensFeatureDel(&pthis->TargetFeature);
    
    ensExternaldatabaseDel(&pthis->Externaldatabase);
    
    ajStrDel(&pthis->ExtraData);
    ajStrDel(&pthis->SourceSpecies);
    ajStrDel(&pthis->TargetSpecies);
    
    AJFREE(pthis);

    *Pfp = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
** @fnote None
**
** @nam3rule Get Return Feature Pair attribute(s)
** @nam4rule GetSourceFeature Return the source Ensembl Feature
** @nam4rule GetTargetFeature Return the target Ensembl Feature
** @nam4rule GetExternaldatabase Return the Ensembl External Database
** @nam4rule GetExtraData Return the extra data
** @nam4rule GetSourceSpecies Return the source species name
** @nam4rule GetTargetSpecies Return the target species name
** @nam4rule GetGroupIdentifier Return the group identifier
** @nam4rule GetLevelIdentifier Return the level identifier
** @nam4rule GetEvalue Return the e-value
** @nam4rule GetScore Return the score
** @nam4rule GetSourceCoverage Return the source coverage
** @nam4rule GetTargetCoverage Return the target coverage
** @nam4rule GetSequenceIdentity Return the sequence identity
**
** @argrule * fp [const EnsPFeaturepair] Feature Pair
**
** @valrule SourceFeature [EnsPFeature] Source Ensembl Feature
** @valrule TargetFeature [EnsPFeature] Target Ensembl Feature
** @valrule Externaldatabase [EnsPExternaldatabase] Ensembl External Database
** @valrule ExtraData [AjPStr] Extra data
** @valrule SourceSpecies [AjPStr] Source species name
** @valrule TargetSpecies [AjPStr] Target species name
** @valrule GroupIdentifier [ajuint] Group identifier
** @valrule LevelIdentifier [ajuint] Level identifier
** @valrule Evalue [double] E-value
** @valrule Score [double] Score
** @valrule GetSourceCoverage [float] Source coverage
** @valrule GetTargetCoverage [float] Target coverage
** @valrule GetSequenceIdentity [float] Sequence identity
**
** @fcategory use
******************************************************************************/




/* @func ensFeaturepairGetSourceFeature ***************************************
**
** Get the source Ensembl Feature element of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeature] Source Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensFeaturepairGetSourceFeature(const EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    return fp->SourceFeature;
}




/* @func ensFeaturepairGetTargetFeature ***************************************
**
** Get the target Ensembl Feature element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hslice
** @cc Bio::EnsEMBL::FeaturePair::hseqname
** @cc Bio::EnsEMBL::FeaturePair::hstart
** @cc Bio::EnsEMBL::FeaturePair::hend
** @cc Bio::EnsEMBL::FeaturePair::hstrand
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPFeature] Target Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensFeaturepairGetTargetFeature(const EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    return fp->TargetFeature;
}




/* @func ensFeaturepairGetExternaldatabase ************************************
**
** Get the Ensembl External Database element of an Ensembl Feature Pair.
**
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
** @@
******************************************************************************/

EnsPExternaldatabase ensFeaturepairGetExternaldatabase(
    const EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    return fp->Externaldatabase;
}




/* @func ensFeaturepairGetExtraData *******************************************
**
** Get the extra data element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::extra_data
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjPStr] Extra data or NULL
** @@
******************************************************************************/

AjPStr ensFeaturepairGetExtraData(const EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    return fp->ExtraData;
}




/* @func ensFeaturepairGetSourceSpecies ***************************************
**
** Get the source species name element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::species
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjPStr] Source species or NULL
** @@
******************************************************************************/

AjPStr ensFeaturepairGetSourceSpecies(const EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    return fp->SourceSpecies;
}




/* @func ensFeaturepairGetTargetSpecies ***************************************
**
** Get the target species name element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hspecies
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjPStr] Target species or NULL
** @@
******************************************************************************/

AjPStr ensFeaturepairGetTargetSpecies(const EnsPFeaturepair fp)
{
    if(!fp)
	return NULL;
    
    return fp->TargetSpecies;
}




/* @func ensFeaturepairGetGroupIdentifier *************************************
**
** Get the group identifier element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::group_id
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajuint] Group identifier or 0
** @@
******************************************************************************/

ajuint ensFeaturepairGetGroupIdentifier(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->GroupIdentifier;
}




/* @func ensFeaturepairGetLevelIdentifier *************************************
**
** Get the level identifier element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::level_id
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajuint] Level identifier or 0
** @@
******************************************************************************/

ajuint ensFeaturepairGetLevelIdentifier(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->LevelIdentifier;
}




/* @func ensFeaturepairGetEvalue **********************************************
**
** Get the e-value element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::p_value
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [double] E-value or 0
** @@
******************************************************************************/

double ensFeaturepairGetEvalue(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->Evalue;
}




/* @func ensFeaturepairGetScore ***********************************************
**
** Get the score element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::score
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [double] Score or 0
** @@
******************************************************************************/

double ensFeaturepairGetScore(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->Evalue;
}




/* @func ensFeaturepairGetSourceCoverage **************************************
**
** Get the source coverage element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::coverage
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [float] Source coverage or 0
** @@
******************************************************************************/

float ensFeaturepairGetSourceCoverage(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->SourceCoverage;
}




/* @func ensFeaturepairGetTargetCoverage **************************************
**
** Get the target coverage element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hcoverage
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [float] Target coverage or 0
** @@
******************************************************************************/

float ensFeaturepairGetTargetCoverage(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->TargetCoverage;
}




/* @func ensFeaturepairGetSequenceIdentity ************************************
**
** Get the sequence identity element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::percent_id
** @param [r] fp [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [float] Sequence identity or 0
** @@
******************************************************************************/

float ensFeaturepairGetSequenceIdentity(const EnsPFeaturepair fp)
{
    if(!fp)
	return 0;
    
    return fp->SequenceIdentity;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
** @fnote None
**
** @nam3rule Set Set one element of a Feature
** @nam4rule SetSourceFeature Set the source Ensembl Feature
** @nam4rule SetTargetFeature Set the target Ensembl Feature
** @nam4rule SetExternaldatabase Set the Ensembl External Database
** @nam4rule SetExtraData Set the extra data
** @nam4rule SetSourceSpecies Set the source species name
** @nam4rule SetTargetSpecies Set the target species name
** @nam4rule SetGroupIdentifier Set the group identifier
** @nam4rule SetLevelIdentifier Set the level identifier
** @nam4rule SetEvalue Set the e-value
** @nam4rule SetScore Set the score
** @nam4rule SetSourceCoverage Set the source coverage
** @nam4rule SetTargetCoverage Set the target coverage
** @nam4rule SetSequenceIdentity Set the sequence identity
**
** @argrule * fp [EnsPFeaturepair] Ensembl Feature Pair object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensFeaturepairSetSourceFeature ***************************************
**
** Set the source Ensembl Feature element of an Ensembl Feature Pair.
**
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] feature [EnsPFeature] Source Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetSourceFeature(EnsPFeaturepair fp, EnsPFeature feature)
{
#if AJFALSE
    ajDebug("ensFeaturepairSetSourceFeature\n"
	    "  fp %p\n"
	    "  feature %p\n",
	    fp,
	    feature);
    
    ensFeaturepairTrace(fp, 1);
    
    ensFeatureTrace(feature, 1);
#endif
    
    if(!fp)
	return ajFalse;
    
    /* Replace the current Feature. */
    
    ensFeatureDel(&fp->SourceFeature);
    
    fp->SourceFeature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensFeaturepairSetTargetFeature ***************************************
**
** Set the target Ensembl Feature element of an Ensembl Feature Pair.
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
** @@
******************************************************************************/

AjBool ensFeaturepairSetTargetFeature(EnsPFeaturepair fp, EnsPFeature feature)
{
#if AJFALSE
    ajDebug("ensFeaturepairSetTargetFeature\n"
	    "  fp %p\n"
	    "  feature %p\n",
	    fp,
	    feature);
    
    ensFeaturepairTrace(fp, 1);
    
    ensFeatureTrace(feature, 1);
#endif
    
    if(!fp)
	return ajFalse;
    
    /* Replace the current Feature. */
    
    ensFeatureDel(&fp->TargetFeature);
    
    fp->TargetFeature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensFeaturepairSetExternaldatabase ************************************
**
** Set the Ensembl External Database element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::external_db_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetExternaldatabase(EnsPFeaturepair fp,
                                         EnsPExternaldatabase edb)
{
    if(!fp)
	return ajFalse;
    
    ensExternaldatabaseDel(&fp->Externaldatabase);
    
    fp->Externaldatabase = ensExternaldatabaseNewRef(edb);
    
    return ajTrue;
}




/* @func ensFeaturepairSetExtraData *******************************************
**
** Set the extra data element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::extra_data
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] extra [AjPStr] Extra data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetExtraData(EnsPFeaturepair fp, AjPStr extra)
{
    if(!fp)
	return ajFalse;
    
    ajStrDel(&fp->ExtraData);
    
    if(extra)
	fp->ExtraData = ajStrNewRef(extra);
    
    return ajTrue;
}




/* @func ensFeaturepairSetSourceSpecies ***************************************
**
** Set the source species element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::species
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] species [AjPStr] Source species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetSourceSpecies(EnsPFeaturepair fp, AjPStr species)
{
    if(!fp)
	return ajFalse;
    
    ajStrDel(&fp->SourceSpecies);
    
    if(species)
	fp->SourceSpecies = ajStrNewRef(species);
    
    return ajTrue;
}




/* @func ensFeaturepairSetTargetSpecies ***************************************
**
** Set the target species element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hspecies
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [u] tspecies [AjPStr] Target species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetTargetSpecies(EnsPFeaturepair fp, AjPStr tspecies)
{
    if(!fp)
	return ajFalse;
    
    ajStrDel(&fp->TargetSpecies);
    
    if(tspecies)
	fp->TargetSpecies = ajStrNewRef(tspecies);
    
    return ajTrue;
}




/* @func ensFeaturepairSetGroupIdentifier *************************************
**
** Set the group identifier element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::group_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] groupid [ajuint] Group identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetGroupIdentifier(EnsPFeaturepair fp, ajuint groupid)
{
    if(!fp)
	return ajFalse;
    
    fp->GroupIdentifier = groupid;
    
    return ajTrue;
}




/* @func ensFeaturepairSetLevelIdentifier *************************************
**
** Set the level identifier element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::level_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] levelid [ajuint] Level identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetLevelIdentifier(EnsPFeaturepair fp, ajuint levelid)
{
    if(!fp)
	return ajFalse;
    
    fp->LevelIdentifier = levelid;
    
    return ajTrue;
}




/* @func ensFeaturepairSetEvalue **********************************************
**
** Set the e-value element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::p_value
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] evalue [double] E-value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetEvalue(EnsPFeaturepair fp, double evalue)
{
    if(!fp)
	return ajFalse;
    
    fp->Evalue = evalue;
    
    return ajTrue;
}




/* @func ensFeaturepairSetScore ***********************************************
**
** Set the score element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::score
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetScore(EnsPFeaturepair fp, double score)
{
    if(!fp)
	return ajFalse;
    
    fp->Score = score;
    
    return ajTrue;
}




/* @func ensFeaturepairSetSourceCoverage **************************************
**
** Set the source coverage element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::coverage
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] coverage [float] Source coverage
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetSourceCoverage(EnsPFeaturepair fp, float coverage)
{
    if(!fp)
	return ajFalse;
    
    fp->SourceCoverage = coverage;
    
    return ajTrue;
}




/* @func ensFeaturepairSetTargetCoverage **************************************
**
** Set the target coverage element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::hcoverage
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] coverage [float] Target coverage
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetTargetCoverage(EnsPFeaturepair fp, float coverage)
{
    if(!fp)
	return ajFalse;
    
    fp->TargetCoverage = coverage;
    
    return ajTrue;
}




/* @func ensFeaturepairSetSequenceIdentity ************************************
**
** Set the sequence identity element of an Ensembl Feature Pair.
**
** @cc Bio::EnsEMBL::FeaturePair::percent_id
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @param [r] identity [float] Sequence identity
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensFeaturepairSetSequenceIdentity(EnsPFeaturepair fp, float identity)
{
    if(!fp)
	return ajFalse;
    
    fp->SequenceIdentity = identity;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Feature Pair object.
**
** @fdata [EnsPFeaturepair]
** @nam3rule Trace Report Ensembl Feature Pair elements to debug file
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
** @@
******************************************************************************/

AjBool ensFeaturepairTrace(const EnsPFeaturepair fp, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!fp)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("ensFeaturepairTrace %p\n"
	    "%S  SourceFeature %p\n"
	    "%S  TargetFeature %p\n"
	    "%S  Externaldatabase %p\n"
	    "%S  ExtraData %p\n"
	    "%S  SourceSpecies '%S'\n"
	    "%S  TargetSpecies '%S'\n"
	    "%S  GroupIdentifier %d\n"
	    "%S  LevelIdentifier %d\n"
	    "%S  Use %u\n"
	    "%S  Evalue %f\n"
	    "%S  Score %f\n"
	    "%S  SourceCoverage %f\n"
	    "%S  TargetCoverage %f\n"
	    "%S  SequenceIdentity %f\n",
	    indent, fp,
	    indent, fp->SourceFeature,
	    indent, fp->TargetFeature,
	    indent, fp->Externaldatabase,
	    indent, fp->ExtraData,
	    indent, fp->SourceSpecies,
	    indent, fp->TargetSpecies,
	    indent, fp->GroupIdentifier,
	    indent, fp->LevelIdentifier,
	    indent, fp->Use,
	    indent, fp->Evalue,
	    indent, fp->Score,
	    indent, fp->SourceCoverage,
	    indent, fp->TargetCoverage,
	    indent, fp->SequenceIdentity);
    
    ensFeatureTrace(fp->SourceFeature, level + 1);
    
    ensFeatureTrace(fp->TargetFeature, level + 1);
    
    ensExternaldatabaseTrace(fp->Externaldatabase, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensFeaturepairGetMemSize *********************************************
**
** Get the memory size in bytes of an Ensembl Feature Pair.
**
** @param [r] baf [const EnsPFeaturepair] Ensembl Feature Pair
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensFeaturepairGetMemSize(const EnsPFeaturepair fp)
{
    ajuint size = 0;
    
    if(!fp)
	return 0;
    
    size += (ajuint) sizeof (EnsOFeaturepair);
    
    size += ensFeatureGetMemSize(fp->SourceFeature);
    
    size += ensFeatureGetMemSize(fp->TargetFeature);
    
    if(fp->ExtraData)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(fp->ExtraData);
    }
    
    if(fp->SourceSpecies)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(fp->SourceSpecies);
    }
    
    if(fp->TargetSpecies)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(fp->TargetSpecies);
    }
    
    return size;
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
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairTransform(const EnsPFeaturepair fp,
                                        const AjPStr csname,
                                        const AjPStr csversion)
{
    EnsPFeature nfeature = NULL;
    EnsPFeaturepair nfp  = NULL;
    
    if(!fp)
	return NULL;
    
    if(!csname)
	return NULL;
    
    if(!csversion)
	return NULL;
    
    nfeature = ensFeatureTransform(fp->SourceFeature, csname, csversion);
    
    if(!nfeature)
	return NULL;
    
    nfp = ensFeaturepairNewObj(fp);
    
    ensFeaturepairSetSourceFeature(nfp, nfeature);
    
    ensFeatureDel(&nfeature);
    
    return nfp;
}




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
** @@
******************************************************************************/

EnsPFeaturepair ensFeaturepairTransfer(EnsPFeaturepair fp, EnsPSlice slice)
{
    EnsPFeature newfeature = NULL;
    EnsPFeaturepair newfp  = NULL;
    
    if(!fp)
	return NULL;
    
    if(!slice)
	return NULL;
    
    newfeature = ensFeatureTransfer(fp->SourceFeature, slice);
    
    if(!newfeature)
	return NULL;
    
    newfp = ensFeaturepairNewObj(fp);
    
    ensFeaturepairSetSourceFeature(newfp, newfeature);
    
    ensFeatureDel(&newfeature);
    
    return newfp;
}




/* @datasection [EnsPBasealignfeature] Base Align Feature *********************
**
** Functions for manipulating Ensembl Base Align Feature objects
**
** @cc Bio::EnsEMBL::BaseAlignFeature CVS Revision: 1.58
** @cc Bio::EnsEMBL::DnaDnaAlignFeature CVS Revision: 1.22
** @cc Bio::EnsEMBL::DnaPepAlignFeature CVS Revision: 1.12
** @cc Bio::EnsEMBL::PepDnaAlignFeature CVS Revision: 1.9
**
** @nam2rule Basealignfeature
**
******************************************************************************/




/* @funcstatic baseAlignFeatureCompareSourceFeatureStart **********************
**
** Comparison function to sort Ensembl Feature Pairs by the start ccordinate of
** their source Ensembl Feature elements in ascending order on the forward
** strand and descending order on the reverse strand.
**
** @param [r] P1 [const void*] Ensembl Base Align Feature address 1
** @param [r] P2 [const void*] Ensembl Base Align Feature address 2
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int baseAlignFeatureCompareSourceFeatureStart(const void* P1,
                                                     const void* P2)
{
    int value = 0;
    
    EnsPFeaturepair fp1 = NULL;
    EnsPFeaturepair fp2 = NULL;
    
    fp1 = *(EnsPFeaturepair const *) P1;
    
    fp2 = *(EnsPFeaturepair const *) P2;
    
#if AJFALSE
    ajDebug("baseAlignFeatureCompareSourceFeatureStart\n"
            "  fp1 %p\n"
            "  fp2 %p\n",
            fp1,
            fp2);
    
    ensFeaturepairTrace(fp1, 1);
    ensFeaturepairTrace(fp2, 1);
#endif
    
    if(!fp1)
    {
	ajDebug("baseAlignFeatureCompareSourceFeatureStart got empty "
		"Feature Pair 1.\n");
	
	return 0;
    }
    
    if(!fp2)
    {
	ajDebug("baseAlignFeatureCompareSourceFeatureStart got empty "
		"Feature Pair 2.\n");
	
	return 0;
    }
    
    if(!fp1->SourceFeature)
    {
	ajDebug("baseAlignFeatureCompareSourceFeatureStart got a "
		"Feature Pair 1 without a source Feature.\n");
	
	return 0;
    }
    
    if(!fp2->SourceFeature)
    {
	ajDebug("baseAlignFeatureCompareSourceFeatureStart got a "
		"Feature Pair 2 without a source Feature.\n");
	
	return 0;
    }
    
    if(fp1->SourceFeature->Start < fp2->SourceFeature->Start)
        value = -1;
    
    if(fp1->SourceFeature->Start == fp2->SourceFeature->Start)
        value = 0;
    
    if(fp1->SourceFeature->Start > fp2->SourceFeature->Start)
        value = +1;
    
    /*
    ** For Feature Pairs with a Feature on the reverse strand,
    ** reverse also the sorting order.
    */
    
    if(fp1->SourceFeature->Strand < 0)
	value *= -1;
    
    return value;
}




/* @funcstatic baseAlignFeatureParseFeatures **********************************
**
** Parse Ensembl Base Align Features from an AJAX List of Ensembl Objects based
** upon Ensembl Feature Pairs. This creates an internal CIGAR string and sets
** internal alignment coordinates.
**
** @param [r] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [r] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Feature Pairs
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

/*
** FIXME: The Perl API assigns to $start1 $start2 and $ori but never uses
** these values.
*/

static AjBool baseAlignFeatureParseFeatures(EnsPBasealignfeature baf,
                                            AjPList objects)
{
    void *Pobject = NULL;
    
    ajint srcstart = 0;
    ajint trgstart = 0;
    
    ajint srcend = 0;
    ajint trgend = 0;
    
    ajint srcgap = 0;
    ajint trggap = 0;
    
    ajint srclength = 0;
    ajint trglength = 0;
    
    ajint srcpos = 0; /* where last feature q part ended */
    ajint trgpos = 0; /* where last feature s part ended */
    
    ajint match = 0;
    
    ajuint srcunit = 0;
    ajuint trgunit = 0;
    
    AjBool insertion = AJFALSE;
    AjBool warning = AJFALSE;
    
    AjIList iter = NULL;
    AjPList fps  = NULL;
    
    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;
    
    EnsPFeaturepair firstfp = NULL;
    EnsPFeaturepair lastfp  = NULL;
    EnsPFeaturepair fp      = NULL;
    
    if(!baf)
	return ajFalse;
    
    if(!objects)
	return ajFalse;
    
    if(!ajListGetLength(objects))
	return ajTrue;
    
    srcunit = ensBasealignfeatureGetSourceUnit(baf);
    
    trgunit = ensBasealignfeatureGetTargetUnit(baf);
    
    if(baf->Cigar)
	ajStrAssignClear(&baf->Cigar);
    else
	baf->Cigar = ajStrNew();
    
    /* Extract Ensembl Feature Pair objects from Ensembl Objects. */
    
    iter = ajListIterNew(objects);
    
    while(!ajListIterDone(iter))
    {
	Pobject = ajListIterGet(iter);
	
	ajListPushAppend(fps, (void *) baf->GetFeaturepair(Pobject));
    }
    
    ajListIterDel(&iter);
    
    /*
    ** Sort the Ensembl Feature Pairs on their Feature start position in
    ** ascending order on positive strand, descending on negative strand.
    */
    
    ajListSort(fps, baseAlignFeatureCompareSourceFeatureStart);
    
    ajListPeekFirst(fps, (void **) &firstfp);
    
    ajListPeekLast(fps, (void **) &lastfp);
    
    /*
    ** Use strandedness info of source and target to make sure both sets of
    ** start and end coordinates are oriented the right way around.
    */
    
    if(firstfp->SourceFeature->Strand >= 0)
    {
	srcstart = firstfp->SourceFeature->Start;
	
	srcend = lastfp->SourceFeature->End;
    }
    else
    {
	srcstart = lastfp->SourceFeature->Start;
	
	srcend = firstfp->SourceFeature->End;
    }
    
    if(firstfp->TargetFeature->Strand >= 0)
    {
	trgstart = firstfp->TargetFeature->Start;
	
	trgend = lastfp->TargetFeature->End;
    }
    else
    {
	trgstart = lastfp->TargetFeature->Start;
	
	trgend = firstfp->TargetFeature->End;
    }
    
    /* Loop through each portion of alignment and construct cigar string */
    
    iter = ajListIterNew(fps);
    
    while(!ajListIterDone(iter))
    {
	fp = (EnsPFeaturepair) ajListIterGet(iter);
	
	/* Sanity checks */
	
	if(fp->SourceFeature->Strand != firstfp->SourceFeature->Strand)
	    ajFatal("Inconsistencies in the source Feature Strand element of "
		    "Ensembl Feature Pair list.\n");
	
	if(!ensSliceMatch(fp->SourceFeature->Slice,
                          firstfp->SourceFeature->Slice))
	    ajFatal("Inconsistencies in source Feature Slice element of "
		    "Ensembl Feature Pair list.\n");
	
	if(fp->SourceSpecies &&
	   firstfp->SourceSpecies &&
           (! ajStrMatchS(fp->SourceSpecies, firstfp->SourceSpecies)))
	    ajFatal("Inconsistencies in Feature Pair source species element "
		    "of Ensembl Feature Pair list.\n");
	
	if(fp->TargetSpecies &&
           firstfp->TargetSpecies &&
           (! ajStrMatchS(fp->TargetSpecies, firstfp->TargetSpecies)))
	    ajFatal("Inconsistencies in Feature Pair target species element "
		    "of Ensembl Feature Pair list.\n");
	
	if(fp->TargetFeature->Strand != firstfp->TargetFeature->Strand)
	    ajFatal("Inconsistencies in the target Feature strand element of "
		    "Ensembl Feature Pair list.\n");
	
	/*
	** FIXME: This currently only takes Features based on SequenceNames
	** into account. Should test Slice and seqeunce name based Features.
	*/
	
	if(!ajStrMatchCaseS(fp->TargetFeature->SequenceName,
                            firstfp->TargetFeature->SequenceName))
	    ajFatal("Inconsistencies in the target Feature sequence name "
		    "element of Ensembl Feature Pair list.\n");
	
	/*
	** FIXME: To test simple equality for floats and doubles is a bit
        ** dangerous.
	 
        if (defined($score) && $score ne $f->score) {
        throw("Inconsisent scores in feature array [$score - " .
        $f->score . "]");
        }
        if (defined($f->percent_id) && $percent ne $f->percent_id) {
        throw("Inconsistent pids in feature array [$percent - " .
        $f->percent_id . "]");
        }
        if(defined($pvalue) && $pvalue != $f->p_value()) {
        throw("Inconsistant p_values in feature arraw [$pvalue " .
        $f->p_value() . "]");
        }
        */
	
	if(fp->SourceFeature->SequenceName &&
           firstfp->SourceFeature->SequenceName &&
           (! ajStrMatchS(fp->SourceFeature->SequenceName,
                          firstfp->SourceFeature->SequenceName)))
	    ajFatal("Inconsistencies in source Feature sequence name element "
		    "of Ensembl Feature Pair list.\n");
	
	/* More sanity checking */
	
	if(srcpos)
	{
	    if(fp->SourceFeature->Strand >= 0)
	    {
		if(fp->SourceFeature->Start < srcpos)
		    ajFatal("Inconsistent coodinates in "
			    "Ensembl Feature Pair List (forward strand).\n"
			    "Start (%d) of current Feature Pair should be "
			    "greater than previous Feature Pair end (%d).\n",
			    fp->SourceFeature->Start, srcpos);
	    }
	    else
	    {
		if(fp->SourceFeature->End > srcpos)
		    ajFatal("Inconsistent coodinates in "
			    "Ensembl Feature Pair List (reverse strand).\n"
			    "End (%d) of current Feature Pair should be "
			    "less than previous Feature Pair start (%d).\n",
			    fp->SourceFeature->End, srcpos);
	    }
	}
	
	srclength = fp->SourceFeature->End - fp->SourceFeature->Start + 1;
	
	trglength = fp->TargetFeature->End - fp->TargetFeature->Start + 1;
	
	/*
	** using multiplication to avoid rounding errors, hence the
	** switch from source to target for the ratios
	*/
	
	/*
	** Yet more sanity checking
	*/
	
	if(srcunit > trgunit)
	{
	    /*
	    ** I am going to make the assumption here that this situation
	    ** will only occur with DnaPepAlignFeatures, this may not be true
	    */
	    
	    if((srclength / srcunit) != (trglength * trgunit))
		ajFatal("Feature Pair lengths not comparable "
			"Lengths: %d %d "
			"Ratios: %d %d.\n",
			srclength, trglength,
			srcunit, trgunit);
	}
	else
	{
	    if((srclength * trgunit) != (trglength * srcunit))
		ajFatal("Feature Pair lengths not comparable "
			"Lengths: %d %d "
			"Ratios: %d %d.\n",
			srclength, trglength,
			srcunit, trgunit);
	}
	
	/*
	** Check to see if there is an I type (insertion) gap:
	** If there is a space between the end of the last source sequence
	** alignment and the start of this one, then this is an insertion
	*/
	
	insertion = ajFalse;
	
	if(firstfp->SourceFeature->Strand >= 0)
	{
	    if(srcpos && (fp->SourceFeature->Start > (srcpos + 1)))
	    {
		/* there is an insertion */
		
		insertion = ajTrue;
		
		srcgap = fp->SourceFeature->Start - srcpos - 1;
		
		/* no need for a number if gap length is 1 */
		
		if(srcgap == 1)
		    ajStrAppendK(&(baf->Cigar), 'I');
		else
		    ajFmtPrintAppS(&(baf->Cigar), "%dI", srcgap);
	    }
	    
	    /* shift our position in the source sequence alignment */
	    
	    srcpos = fp->SourceFeature->End;
	}
	else
	{
	    if(srcpos && ((fp->SourceFeature->End + 1) < srcpos))
	    {
		/* there is an insertion */
		
		insertion = ajTrue;
		
		srcgap = srcpos - fp->SourceFeature->End - 1;
		
		/* no need for a number if gap length is 1 */
		
		if(srcgap == 1)
		    ajStrAppendK(&(baf->Cigar), 'I');
		else
		    ajFmtPrintAppS(&(baf->Cigar), "%dI", srcgap);
	    }
	    
	    /* shift our position in the source sequence alignment */
	    
	    srcpos = fp->SourceFeature->Start;
	}
	
	/*
	** Check to see if there is a D type (deletion) gap
	** There is a deletion gap if there is a space between the end of the
	** last portion of the hit sequence alignment and this one
	*/
	
	if(fp->TargetFeature->Strand >= 0)
	{
	    if(trgpos && (fp->TargetFeature->Start > (trgpos + 1)))
	    {
		/* there is a deletion */
		
		srcgap = fp->TargetFeature->Start - trgpos - 1;
		
		trggap = (ajint) (srcgap * srcunit / trgunit + 0.5);
		
		/* no need for a number if gap length is 1 */
		
		if(trggap == 1)
		    ajStrAppendK(&(baf->Cigar), 'D');
		else
		    ajFmtPrintAppS(&(baf->Cigar), "%dD", trggap);
		
		/* sanity check,  should not be an insertion and deletion */
		
		if(insertion)
		{
		    if(!warning)
		    {
			ajWarn("Should not be an deletion and insertion on "
			       "the same alignment region. "
			       "CIGAR line '%S'\n", baf->Cigar);
			
			warning = ajTrue;
		    }
		}
	    }
	    
	    /* shift our position in the target sequence alignment */
	    
	    trgpos = fp->TargetFeature->End;
	}
	else
	{
	    if(trgpos && ((fp->TargetFeature->End + 1) < trgpos))
	    {
		/* there is a deletion */
		
		srcgap = trgpos - fp->TargetFeature->End - 1;
		
		trggap = (ajint) (srcgap * srcunit / trgunit + 0.5);
		
		/* no need for a number if gap length is 1 */
		
		if(trggap == 1)
		    ajStrAppendK(&(baf->Cigar), 'D');
		else
		    ajFmtPrintAppS(&(baf->Cigar), "%dD", trggap);
		
		/* sanity check,  should not be an insertion and deletion */
		
		if(insertion)
		{
		    if(!warning)
		    {
			ajWarn("Should not be an deletion and insertion on "
			       "the same alignment region. "
			       "target position %d "
			       "target end %d "
			       "CIGAR line '%S'\n",
			       trgpos,
			       fp->TargetFeature->End,
			       baf->Cigar);
			
			warning = ajTrue;
		    }
		}
	    }
	    
	    /* shift our position in the target sequence alignment */
	    
	    trgpos = fp->TargetFeature->Start;
	}
	
	match = fp->SourceFeature->End - fp->SourceFeature->Start + 1;
	
	if(match == 1)
	    ajStrAppendK(&(baf->Cigar), 'M');
	else
	    ajFmtPrintAppS(&(baf->Cigar), "%dM", match);
    }
    
    /* Replace the Feature Pair in the Base Align Feature. */
    
    ensFeaturepairDel(&(baf->Featurepair));
    
    /* Clone the source Feature and set the new coordinates. */
    
    srcfeature = ensFeatureNewObj(firstfp->SourceFeature);
    
    ensFeatureSetStart(srcfeature, srcstart);
    
    ensFeatureSetEnd(srcfeature, srcend);
    
    /* Clone the target Feature and set the new coordinates. */
    
    trgfeature = ensFeatureNewObj(firstfp->TargetFeature);
    
    ensFeatureSetStart(trgfeature, trgstart);
    
    ensFeatureSetEnd(trgfeature, trgend);
    
    /* Clone the Feature Pair and set the new source and target Features. */
    
    baf->Featurepair = ensFeaturepairNewObj(firstfp);
    
    ensFeaturepairSetSourceFeature(baf->Featurepair, srcfeature);
    
    ensFeaturepairSetTargetFeature(baf->Featurepair, trgfeature);
    
    /* Delete the source and target Features. */
    
    ensFeatureDel(&srcfeature);
    
    ensFeatureDel(&trgfeature);
    
    return ajTrue;
}




/* @funcstatic baseAlignFeatureParseCigar *************************************
**
** Convert an Ensembl Base Align Feature CIGAR line into a
** List of Ensembl Feature Pairs.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pairs
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool baseAlignFeatureParseCigar(const EnsPBasealignfeature baf,
                                         AjPList fps)
{
    ajint fpsrcstart = 0;
    ajint fptrgstart = 0;
    
    ajint srcstart = 0;
    ajint trgstart = 0;
    
    ajint srcend = 0;
    ajint trgend = 0;
    
    ajint tlength = 0;
    ajint mlength = 0;
    
    ajuint srcunit = 0;
    ajuint trgunit = 0;
    
    const AjPStr token = NULL;
    
    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;
    
    EnsPFeaturepair fp = NULL;
    
    if(!baf)
	return ajFalse;
    
    if((! baf->Cigar) && (! ajStrGetLen(baf->Cigar)))
	ajFatal("baseAlignFeatureParseCigar no CIGAR string defined!\n");
    
    if(! baf->Featurepair)
	ajFatal("baseAlignFeatureParseCigar no Feature Pair defined in the "
		"Base Align Feature!\n");
    
    if(!baf->Featurepair->SourceFeature)
	ajFatal("baseAlignFeatureParseCigar no Feature defined in the "
		"Feature Pair of the Base Align Feature!\n");
    
    srcunit = ensBasealignfeatureGetSourceUnit(baf);
    
    trgunit = ensBasealignfeatureGetTargetUnit(baf);
    
    if(baf->Featurepair->SourceFeature->Strand > 0)
	fpsrcstart = baf->Featurepair->SourceFeature->Start;
    else
	fpsrcstart = baf->Featurepair->SourceFeature->End;
    
    if(baf->Featurepair->TargetFeature->Strand > 0)
	fptrgstart = baf->Featurepair->TargetFeature->Start;
    else
	fptrgstart = baf->Featurepair->TargetFeature->End;
    
    /* Construct ungapped blocks as Feature Pair objects for each MATCH. */
    
    while((token = ajStrParseC(baf->Cigar, "MDI")))
    {
	ajStrToInt(token, &tlength);
	
	tlength = tlength ? tlength : 1;
	
	if((srcunit == 1) && (trgunit == 3))
	    mlength = tlength * 3;
	else if((srcunit == 3) && (trgunit == 1))
	    mlength = tlength / 3;
	else if((srcunit == 1) && (trgunit == 1))
	    mlength = tlength;
	else
	    ajFatal("baseAlignFeatureParseCigar got "
		    "Base Align Feature source unit %d "
		    "Base Align Feature target unit %d, "
		    "but currently only 1 or 3 are allowed.\n",
		    srcunit,
		    trgunit);
	
	if(ajStrMatchC(token, "M"))
	{
	    /* MATCH */
	    
	    if(baf->Featurepair->SourceFeature->Strand > 0)
	    {
		srcstart = fpsrcstart;
		
		srcend = fpsrcstart + tlength - 1;
		
		fpsrcstart = srcend + 1;
	    }
	    else
	    {
		srcend = fpsrcstart;
		
		srcstart = fpsrcstart - tlength + 1;
		
		fpsrcstart = srcstart - 1;
	    }
	    
	    if(baf->Featurepair->TargetFeature->Strand > 0)
	    {
		trgstart = fptrgstart;
		
		trgend = fptrgstart + mlength - 1;
		
		fptrgstart = trgend + 1;
	    }
	    else
	    {
		trgend = fptrgstart;
		
		trgstart = fptrgstart - mlength + 1;
		
		fptrgstart = trgstart - 1;
	    }
	    
	    /* Clone the source Feature and set the new coordinates. */
	    
	    srcfeature = ensFeatureNewObj(baf->Featurepair->SourceFeature);
	    
	    ensFeatureSetStart(srcfeature, srcstart);
	    
	    ensFeatureSetEnd(srcfeature, srcend);
	    
	    /* Clone the target Feature and set the new coordinates. */
	    
	    trgfeature = ensFeatureNewObj(baf->Featurepair->TargetFeature);
	    
	    ensFeatureSetStart(trgfeature, trgstart);
	    
	    ensFeatureSetEnd(trgfeature, trgend);
	    
	    /*
	    ** Clone the Feature Pair and set the new source and target
	    ** Features.
	    */
	    
	    fp = ensFeaturepairNewObj(baf->Featurepair);
	    
	    ensFeaturepairSetSourceFeature(baf->Featurepair, srcfeature);
	    
	    ensFeaturepairSetTargetFeature(baf->Featurepair, trgfeature);
	    
	    /* Delete the source and target Features. */
	    
	    ensFeatureDel(&srcfeature);
	    
	    ensFeatureDel(&trgfeature);
	    
	    ajListPushAppend(fps, (void *) fp);
	}
	else if(ajStrMatchC(token, "I"))
	{
	    /* INSERT */
	    
	    if(baf->Featurepair->SourceFeature->Strand > 0)
		fpsrcstart += tlength;
	    else
		fpsrcstart -= tlength;
	}
	
	else if (ajStrMatchC(token, "D"))
	{
	    /* DELETION */
	    
	    if(baf->Featurepair->TargetFeature->Strand > 0)
		fptrgstart += mlength;
	    else
		fptrgstart -= mlength;
	}
	else
	    ajFatal("baseAlignFeatureParseCigar "
		    "illegal CIGAR line token '%S'\n",
		    token);
    }
    
    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Base Align Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Base Align Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPBasealignfeature]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @suffix D Constructor for a Base Align Feature of type 'DNA'
** @suffix P Constructor for a Base Align Feature of type 'Protein'
**
** @argrule Obj object [EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Ref object [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @valrule * [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @fcategory new
******************************************************************************/




/* @func ensBasealignfeatureNewD **********************************************
**
** Ensembl Base Align Feature constructor for type 'DNA'.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
** @param [r] pair [ajuint] Pair DNA Align Feature identifier
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewD(
    EnsPDNAAlignFeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar,
    ajuint pair)
{
    EnsPBasealignfeature baf = NULL;
    
    if(!fp)
	return NULL;
    
    if(!cigar)
	return NULL;
    
    AJNEW0(baf);
    
    baf->Use = 1;
    
    baf->Identifier = identifier;
    
    baf->DNAAlignFeatureadaptor = adaptor;
    
    baf->Featurepair = ensFeaturepairNewRef(fp);
    
    if(cigar)
	baf->Cigar = ajStrNewRef(cigar);
    
    baf->Type = ensEBasealignfeatureTypeDNA;
    
    baf->PairDNAAlignFeatureIdentifier = pair;
    
    return baf;
}




/* @func ensBasealignfeatureNewP **********************************************
**
** Ensembl Base Align Feature constructor for type 'Protein'.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewP(
    EnsPProteinalignfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar)
{
    EnsPBasealignfeature baf = NULL;
    
    if(!fp)
	return NULL;
    
    if(!cigar)
	return NULL;
    
    AJNEW0(baf);
    
    baf->Use = 1;
    
    baf->Identifier = identifier;
    
    baf->Proteinalignfeatureadaptor = adaptor;
    
    baf->Featurepair = ensFeaturepairNewRef(fp);
    
    if(cigar)
	baf->Cigar = ajStrNewRef(cigar);
    
    baf->Type = ensEBasealignfeatureTypeProtein;
    
    baf->PairDNAAlignFeatureIdentifier = 0;
    
    return baf;
}




/*
** FIXME: We should split this into two separate Functions.
** ensBasealignfeatureNewC (cigar)
** ensBasealignfeatureNewF (feature pairs).
*/

/* @func ensBasealignfeatureNew ***********************************************
**
** Default Ensembl Base Align Feature constructor.
**
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
** @param [r] fps [AjPList] AJAX List of Ensembl Feature Pairs
** @param [r] type [AjEnum] Ensembl Base Align Feature type
** @param [r] pair [ajuint] Pair DNA Align Feature identifier
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNew(EnsPFeaturepair fp,
                                            AjPStr cigar,
                                            AjPList fps,
                                            AjEnum type,
                                            ajuint pair)
{
    EnsPBasealignfeature baf = NULL;
    
    if(!fp)
	return NULL;
    
    if((type < ensEBasealignfeatureTypeDNA) ||
       (type > ensEBasealignfeatureTypeProtein))
    {
	ajDebug("ensBasealignfeatureNew got illegal type (%d).\n", type);
	
	return NULL;
    }
    
    AJNEW0(baf);
    
    baf->Use = 1;
    
    baf->Featurepair = ensFeaturepairNewRef(fp);
    
    if(cigar && ajStrGetLen(cigar) && fps && ajListGetLength(fps))
    {
	ajDebug("ensBasealignfeatureNew requires a CIGAR String or a "
		"List of Ensembl Feature Pairs, not both.\n");
	
	AJFREE(baf);
	
	return NULL;
    }
    else if (cigar && ajStrGetLen(cigar))
	baf->Cigar = ajStrNewRef(cigar);
    else if (fps && ajListGetLength(fps))
    {
	fps = ajListNew();
	
	baseAlignFeatureParseFeatures(baf, fps);
    }
    else
	ajDebug("ensBasealignfeatureNew requirs either a CIGAR String or a "
		"List of Ensembl Feature Pairs.\n");
    
    baf->Type = type;
    
    baf->PairDNAAlignFeatureIdentifier = pair;
    
    return baf;
}




/* @func ensBasealignfeatureNewObj ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewObj(
    const EnsPBasealignfeature object)
{
    EnsPBasealignfeature baf = NULL;
    
    AJNEW0(baf);
    
    baf->Use = 1;
    
    baf->Identifier                 = object->Identifier;
    baf->DNAAlignFeatureadaptor     = object->DNAAlignFeatureadaptor;
    baf->Proteinalignfeatureadaptor = object->Proteinalignfeatureadaptor;
    
    baf->Featurepair = ensFeaturepairNewRef(object->Featurepair);
    
    if(object->Cigar)
	baf->Cigar = ajStrNewRef(object->Cigar);
    
    baf->Type = object->Type;
    
    baf->AlignmentLength = object->AlignmentLength;
    
    baf->PairDNAAlignFeatureIdentifier = object->PairDNAAlignFeatureIdentifier;
    
    return baf;
}




/* @func ensBasealignfeatureNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewRef(EnsPBasealignfeature baf)
{
    if(!baf)
	return NULL;
    
    baf->Use++;
    
    return baf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Base Align Feature.
**
** @fdata [EnsPBasealignfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Base Align Feature object
**
** @argrule * Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature
**                                         object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensBasealignfeatureDel ***********************************************
**
** Default destructor for an Ensembl Base Align Feature.
**
** @param [d] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [void]
** @@
******************************************************************************/

void ensBasealignfeatureDel(EnsPBasealignfeature *Pbaf)
{
    EnsPBasealignfeature pthis = NULL;
    
    if(!Pbaf)
	return;
    
    if(!*Pbaf)
	return;

    pthis = *Pbaf;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pbaf = NULL;
	
	return;
    }
    
    ensFeaturepairDel(&pthis->Featurepair);
    
    AJFREE(pthis);

    *Pbaf = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
** @fnote None
**
** @nam3rule Get Return Base Align Feature attribute(s)
** @nam4rule GetDNAAlignFeatureadaptor
**           Return the Ensembl DNA Align Feature Adaptor
** @nam4rule GetProteinalignfeatureadaptor
**           Return the Ensembl Protein Align Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeaturepair Return the Ensembl Feature Pair
** @nam4rule GetCigar Return the CIGAR line
** @nam4rule GetType Return the type
** @nam4rule GetAlignmentLength Return the alignment length
** @nam4rule GetPairDNAAlignFeatureIdentifier Return the pair DNA align feature
**                                            identifier
**
** @argrule * baf [const EnsPBasealignfeature] Base Align Feature
**
** @valrule DNAAlignFeatureadaptor [EnsPDNAAlignFeatureadaptor]
**                                  Ensembl DNA Align Feature Adaptor
** @valrule Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
**                                      Ensembl Protein Align Feature Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @valrule Cigar [AjPStr] CIGAR line
** @valrule Type [AjEnum] Type
** @valrule AlignmentLength [ajuint] Alignment length
** @valrule PairDNAAlignFeatureIdentifier [ajuint] Pair DNA Align Feature
**                                                 identifier
**
** @fcategory use
******************************************************************************/




/* @func ensBasealignfeatureGetDNAAlignFeatureadaptor *************************
**
** Get the DNA Align Feature Adaptor element of an Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align Feature Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPDNAAlignFeatureadaptor ensBasealignfeatureGetDNAAlignFeatureadaptor(
    const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;
    
    return baf->DNAAlignFeatureadaptor;
}




/* @func ensBasealignfeatureGetProteinalignfeatureadaptor *********************
**
** Get the Protein Align Feature Adaptor element of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                          Feature Adaptor
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensBasealignfeatureGetProteinalignfeatureadaptor(
    const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;
    
    return baf->Proteinalignfeatureadaptor;
}




/* @func ensBasealignfeatureGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetIdentifier(const EnsPBasealignfeature baf)
{
    if(!baf)
        return 0;
    
    return baf->Identifier;
}




/* @func ensBasealignfeatureGetFeaturepair ************************************
**
** Get the Ensembl Feature Pair element of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPFeaturepair] Ensembl Feature Pair
** @@
******************************************************************************/

EnsPFeaturepair ensBasealignfeatureGetFeaturepair(
    const EnsPBasealignfeature baf)
{
    if(!baf)
	return NULL;
    
    return baf->Featurepair;
}




/*
** FIXME: The alignment length could already be calculated when the CIGAR line
** is parsed the first time in baseAlignFeatureParseCigar.
*/

ajuint ensBasealignfeatureGetAlignmentLength(EnsPBasealignfeature baf)
{
    ajint tlength = 0;
    
    const AjPStr token = NULL;
    
    if(!baf)
	return 0;
    
    if(!baf->AlignmentLength && baf->Cigar)
    {
	
	while((token = ajStrParseC(baf->Cigar, "MDI")))
	{
	    ajStrToInt(token, &tlength);
	    
	    tlength = tlength ? tlength : 1;
	    
	    baf->AlignmentLength += tlength;
	}
    }
    
    return baf->AlignmentLength;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
** @fnote None
**
** @nam3rule Set Set one element of a Base Align Feature
** @nam4rule SetFeaturepair Set the Ensembl Feature Pair
**
** @argrule * baf [EnsPBasealignfeature] Ensembl Base Align Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensBasealignfeatureSetFeaturepair ************************************
**
** Set the Ensembl Feature Pair element of an Ensembl Base Align Feature.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBasealignfeatureSetFeaturepair(EnsPBasealignfeature baf,
                                         EnsPFeaturepair fp)
{
#if AJFALSE
    ajDebug("ensBasealignfeatureSetFeaturepair\n"
	    "  baf %p\n"
	    "  fp %p\n",
	    baf,
	    fp);
    
    ensBasealignfeatureTrace(baf, 1);
    
    ensFeaturepairTrace(fp, 1);
#endif
    
    if(!baf)
	return ajFalse;
    
    if(!fp)
	return ajFalse;
    
    /* Replace the current Feature Pair. */
    
    if(baf->Featurepair)
	ensFeaturepairDel(&(baf->Featurepair));
    
    baf->Featurepair = ensFeaturepairNewRef(fp);
    
    return ajTrue;
}




/* @func ensBasealignfeatureGetFeature ****************************************
**
** Get the Ensembl Feature element of an Ensembl Feature Pair element of an
** Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensBasealignfeatureGetFeature(const EnsPBasealignfeature baf)
{
    if(!baf)
	return NULL;
    
    return ensFeaturepairGetSourceFeature(baf->Featurepair);
}




/* @func ensBasealignfeatureGetAdaptor ****************************************
**
** Get the Adaptor element of an Ensembl Base Align Feature.
** This will return an EnsPDNAAlignFeatureadaptor for a feature of type 'DNA'
** and an EnsPProteinalignfeatureadaptor for a feature of type 'protein'.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [void *] Ensembl DNA or Protein Align Feature Adaptor
** @@
******************************************************************************/

void *ensBasealignfeatureGetAdaptor(const EnsPBasealignfeature baf)
{
    void *Padaptor = NULL;
    
    if(!baf)
        return NULL;

    switch(baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:
          
            Padaptor = (void *) baf->DNAAlignFeatureadaptor;

            break;
          
        case ensEBasealignfeatureTypeProtein:

            Padaptor = (void *) baf->Proteinalignfeatureadaptor;

            break;

        default:
            
            ajWarn("ensBasealignfeatureGetAdaptor got an "
                   "Ensembl Base Align Feature with unexpected type %d.",
                   baf->Type);
    }
    
    return Padaptor;
}




/* @func ensBasealignfeatureGetSourceUnit *************************************
**
** Get the alignment unit for the source part of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Alignment unit for the source part
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetSourceUnit(const EnsPBasealignfeature baf)
{
    ajuint unit = 0;
    
    if(!baf)
	return 0;

    switch(baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            unit = 1;

            break;

        case ensEBasealignfeatureTypeProtein:

            unit = 3;

            break;
            
        default:
            
            ajWarn("ensBasealignfeatureGetSourceUnit got an "
                   "Ensembl Base Align Feature with unexpected type %d.",
                   baf->Type);
    }
    
    return unit;
}




/* @func ensBasealignfeatureGetTargetUnit *************************************
**
** Get the alignment unit for the target part of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Alignment unit for the target part
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetTargetUnit(const EnsPBasealignfeature baf)
{
    ajuint unit = 0;
    
    if(!baf)
	return 0;
    
    switch(baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            unit = 1;

            break;

        case ensEBasealignfeatureTypeProtein:

            unit = 1;

            break;
            
        default:
            
            ajWarn("ensBasealignfeatureGetTargetUnit got an "
                   "Ensembl Base Align Feature with unexpected type %d.",
                   baf->Type);
    }
    
    return unit;
}




/* @func ensBasealignfeatureGetMemSize ****************************************
**
** Get the memory size in bytes of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetMemSize(const EnsPBasealignfeature baf)
{
    ajuint size = 0;
    
    if(!baf)
	return 0;
    
    size += (ajuint) sizeof (EnsOBasealignfeature);
    
    size += ensFeaturepairGetMemSize(baf->Featurepair);
    
    if(baf->Cigar)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(baf->Cigar);
    }
    
    return size;
}




/* @func ensBasealignfeatureTransform *****************************************
**
** Transform an Ensembl Base Align Feature into another
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Feature::transform
** @param [u] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureTransform(
    const EnsPBasealignfeature baf,
    const AjPStr csname,
    const AjPStr csversion)
{
    EnsPFeaturepair nfp = NULL;
    
    EnsPBasealignfeature nbaf = NULL;
    
    if(!baf)
	return NULL;
    
    if(!csname)
	return NULL;
    
    if(!csversion)
	return NULL;
    
    nfp = ensFeaturepairTransform(baf->Featurepair, csname, csversion);
    
    if(!nfp)
	return NULL;
    
    nbaf = ensBasealignfeatureNewObj(baf);
    
    ensBasealignfeatureSetFeaturepair(nbaf, nfp);
    
    ensFeaturepairDel(&nfp);
    
    return nbaf;
}




/* @func ensBasealignfeatureTransfer ******************************************
**
** Transfer an Ensembl Base Align Feature onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::transfer
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureTransfer(EnsPBasealignfeature baf,
                                                 EnsPSlice slice)
{
    EnsPBasealignfeature newbaf = NULL;
    
    EnsPFeaturepair newfp = NULL;
    
    if(!baf)
	return NULL;
    
    if(!slice)
	return NULL;
    
    newfp = ensFeaturepairTransfer(baf->Featurepair, slice);
    
    if(!newfp)
	return NULL;
    
    newbaf = ensBasealignfeatureNewObj(baf);
    
    ensBasealignfeatureSetFeaturepair(newbaf, newfp);
    
    ensFeaturepairDel(&newfp);
    
    return newbaf;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
** @nam3rule Trace Report Ensembl Base Align Feature elements to debug file
**
** @argrule Trace baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensBasealignfeatureTrace *********************************************
**
** Trace an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBasealignfeatureTrace(const EnsPBasealignfeature baf, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!baf)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("ensBasealignfeatureTrace %p\n"
	    "%S  DNAAlignFeatureadaptor %p\n"
	    "%S  Proteinalignfeatureadaptor %p\n"
	    "%S  Identifier %u\n"
	    "%S  Featurepair %p\n"
	    "%S  Cigar '%S'\n"
	    "%S  (*GetFeaturepair) %p\n"
	    "%S  Type %d\n"
	    "%S  AlignmentLength %u\n"
	    "%S  PairDNAAlignFeatureIdentifier %u\n"
	    "%S  Use %u\n",
	    indent, baf,
	    indent, baf->DNAAlignFeatureadaptor,
	    indent, baf->Proteinalignfeatureadaptor,
	    indent, baf->Identifier,
	    indent, baf->Featurepair,
	    indent, baf->Cigar,
	    indent, baf->GetFeaturepair,
	    indent, baf->Type,
	    indent, baf->AlignmentLength,
	    indent, baf->PairDNAAlignFeatureIdentifier,
	    indent, baf->Use);
    
    ensFeaturepairTrace(baf->Featurepair, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensBasealignfeatureFetchAllFeaturepairs ******************************
**
** Fetch all (ungapped) Ensembl Feature Pairs of an
** Ensembl Base Align Feature.
**
** The caller is responsible for deleting the Ensembl Feature Pairs before
** deleting the AJAX List.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pairs
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBasealignfeatureFetchAllFeaturepairs(const EnsPBasealignfeature baf,
                                               AjPList fps)
{
    if(!baf)
	return ajFalse;
    
    if(!fps)
	return ajFalse;
    
    return baseAlignFeatureParseCigar(baf, fps);
}




/* @funcstatic baseAlignFeatureadaptorCacheReference **************************
**
** Wrapper function to reference an Ensembl Base Align Feature
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Base Align Feature
**
** @return [void *] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

static void *baseAlignFeatureadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensBasealignfeatureNewRef((EnsPBasealignfeature) value);
}




/* @funcstatic baseAlignFeatureadaptorCacheDelete *****************************
**
** Wrapper function to delete an Ensembl Base Align Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Base Align Feature address
**
** @return [void]
** @@
******************************************************************************/

static void baseAlignFeatureadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensBasealignfeatureDel((EnsPBasealignfeature *) value);
    
    return;
}




/* @funcstatic baseAlignFeatureadaptorCacheSize *******************************
**
** Wrapper function to determine the memory size of an
** Ensembl Base Align Feature from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Base Align Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

static ajuint baseAlignFeatureadaptorCacheSize(const void *value)
{
    if (!value)
	return 0;
    
    return ensBasealignfeatureGetMemSize((const EnsPBasealignfeature) value);
}




/* @funcstatic baseAlignFeatureadaptorGetFeature ******************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Base Align Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Base Align Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature baseAlignFeatureadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensBasealignfeatureGetFeature((const EnsPBasealignfeature) value);
}




/* @datasection [EnsPDNAAlignFeatureadaptor] DNA Align Feature Adaptor ********
**
** Functions for manipulating Ensembl DNA Align Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DNAAlignFeatureAdaptor CVS Revision: 1.67
** @cc Bio::EnsEMBL::DBSQL::BaseAlignFeatureAdaptor CVS Revision: 1.30
**
** @nam2rule DNAAlignFeatureadaptor
**
******************************************************************************/

static const char *dnaAlignFeatureadaptorTables[] =
{
    "dna_align_feature",
    /* FIXME: The External Database Adaptor has an internal cache.
       "external_db",
    */
    NULL
};




static const char *dnaAlignFeatureadaptorColumns[] =
{
    "dna_align_feature.dna_align_feature_id",
    "dna_align_feature.seq_region_id",
    "dna_align_feature.seq_region_start",
    "dna_align_feature.seq_region_end",
    "dna_align_feature.seq_region_strand",
    "dna_align_feature.hit_start",
    "dna_align_feature.hit_end",
    "dna_align_feature.hit_strand",
    "dna_align_feature.hit_name",
    "dna_align_feature.analysis_id",
    "dna_align_feature.cigar_line",
    "dna_align_feature.score",
    "dna_align_feature.evalue",
    "dna_align_feature.perc_ident",
    "dna_align_feature.external_db_id",
    "dna_align_feature.hcoverage",
    "dna_align_feature.external_data",
    "dna_align_feature.pair_dna_align_feature_id",
    /* FIXME: The Ensembl External Database Adaptor has an internal cache.
       "external_db.db_name",
       "external_db.db_display_name",
    */
    NULL
};




static EnsOBaseadaptorLeftJoin dnaAlignFeatureadaptorLeftJoin[] =
{
    /* FIXME: The Ensembl External Database Adaptor has an internal cache.
       {
       "external_db",
       "dna_align_feature.external_db_id = "
       "external_db.external_db_id"
       },
    */
    {NULL, NULL}
};

static const char *dnaAlignFeatureadaptorDefaultCondition = NULL;

static const char *dnaAlignFeatureadaptorFinalCondition = NULL;




/* @funcstatic dnaAlignFeatureadaptorFetchAllBySQL ****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Base Align Features.
**
** The caller is responsible for deleting the Ensembl Base Align Features
** before deleting the AJAX List.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [r] mapper [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool dnaAlignFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement,
                                                  EnsPAssemblymapper mapper,
                                                  EnsPSlice slice,
                                                  AjPList bafs)
{
    double evalue = 0;
    double score  = 0;
    
    float identity    = 0;
    float hitcoverage = 0;
    
    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint edbid      = 0;
    ajuint pair       = 0;
    
    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;
    
    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    
    ajuint hitstart = 0;
    ajuint hitend   = 0;
    ajint hitstrand = 0;
    
    AjPList mrs = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr hitname = NULL;
    AjPStr cigar   = NULL;
    AjPStr extra   = NULL;
    
    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPBasealignfeature baf = NULL;
    
    EnsPDNAAlignFeatureadaptor dafa = NULL;
    
    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;
    
    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;
    
    EnsPFeaturepair fp = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

#if AJFALSE
    ajDebug("dnaAlignFeatureadaptorFetchAllBySQL\n"
	    "  dba %p\n"
	    "  statement %p\n"
	    "  mapper %p\n"
	    "  slice %p\n"
	    "  bafs %p\n",
	    dba,
	    statement,
	    mapper,
	    slice,
	    bafs);
#endif
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    dafa = ensRegistryGetDNAAlignFeatureadaptor(dba);
    
    edba = ensRegistryGetExternaldatabaseadaptor(dba);
    
    sa = (slice) ? ensSliceGetAdaptor(slice) : ensRegistryGetSliceadaptor(dba);
    
    mrs = ajListNew();
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier  = 0;
	srid        = 0;
	srstart     = 0;
	srend       = 0;
	srstrand    = 0;
	hitstart    = 0;
	hitend      = 0;
	hitstrand   = 0;
	hitname     = ajStrNew();
	analysisid  = 0;
	cigar       = ajStrNew();
	score       = 0;
	evalue      = 0;
	identity    = 0;
	edbid       = 0;
	hitcoverage = 0;
	extra       = ajStrNew();
	pair        = 0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToUint(sqlr, &hitstart);
	ajSqlcolumnToUint(sqlr, &hitend);
	ajSqlcolumnToInt(sqlr, &hitstrand);
	ajSqlcolumnToStr(sqlr, &hitname);
	ajSqlcolumnToUint(sqlr, &analysisid);
	ajSqlcolumnToStr(sqlr, &cigar);
	ajSqlcolumnToDouble(sqlr, &score);
	ajSqlcolumnToDouble(sqlr, &evalue);
	ajSqlcolumnToFloat(sqlr, &identity);
	ajSqlcolumnToUint(sqlr, &edbid);
	ajSqlcolumnToFloat(sqlr, &hitcoverage);
	ajSqlcolumnToStr(sqlr, &extra);
	ajSqlcolumnToUint(sqlr, &pair);
	
	/*
	** Since the Ensembl SQL schema defines Sequence Region start and end
	** coordinates as unsigned integers for all Features, the range needs
	** checking.
	*/
	
	if(srstart <= INT_MAX)
	    slstart = (ajint) srstart;
	else
	    ajFatal("dnaAlignFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("dnaAlignFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region end coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srend, INT_MAX);
	
	slstrand = srstrand;
	
	/* Fetch a Slice spanning the entire Sequence Region. */
	
	ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);
	
	/*
	** Obtain an Ensembl Assembly Mapper if none was defined, but a
	** destination Slice was.
	*/
	
	if(mapper)
	    am = ensAssemblymapperNewRef(mapper);
	
	if((! mapper) &&
	   slice &&
	   (! ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                                  ensSliceGetCoordsystem(srslice))))
	    am =
		ensAssemblymapperadaptorFetchByCoordsystems(
                    ama,
		    ensSliceGetCoordsystem(slice),
		    ensSliceGetCoordsystem(srslice));
	
	/*
	** Remap the Feature coordinates to another Ensembl Coordinate System
	** if an Ensembl Mapper was provided.
	*/
	
	if(am)
	{
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(srslice),
				     slstart,
				     slend,
				     slstrand,
				     mrs);
	    
	    /*
	    ** The ensAssemblymapperFastMap function returns at best one
	    ** Ensembl Mapper Result.
	    */
	    
	    ajListPop(mrs, (void **) &mr);
	    
	    /*
	    ** Skip Features that map to gaps or
	    ** Coordinate System boundaries.
	    */
	    
	    if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
	    {
		/* Load the next Feature but destroy first! */
		
		ajStrDel(&hitname);
		
		ajStrDel(&cigar);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		ensMapperresultDel(&mr);
		
		continue;
	    }
	    
	    srid = ensMapperresultGetObjectIdentifier(mr);
	    
	    slstart = ensMapperresultGetStart(mr);
	    
	    slend = ensMapperresultGetEnd(mr);
	    
	    slstrand = ensMapperresultGetStrand(mr);
	    
	    /*
	    ** FIXME: In contrast to the Bio::EnsEMBL::DBSQL::ExonAdaptor
	    ** code, a construct to get a Slice from the cache is not
	    ** commented out from the
	    ** Bio::EnsEMBL::DBSQL::DnaAlignFeatureAdaptor of the Perl API.
	    ** See CVS versions 1.49 and 1.49.14.1 for details.
	    */
	    
	    /*
	    ** Delete the Sequence Region Slice and fetch a Slice in the
	    ** Coordinate System we just mapped to.
	    */
	    
	    ensSliceDel(&srslice);
	    
	    ensSliceadaptorFetchBySeqregionIdentifier(sa,
						      srid,
						      0,
						      0,
						      0,
						      &srslice);
	    
	    ensMapperresultDel(&mr);
	}
	
	/*
	** Convert Sequence Region Slice coordinates to destination Slice
	** coordinates, if a destination Slice has been provided.
	*/
	
	if(slice)
	{
	    /* Check that the length of the Slice is within range. */
	    
	    if(ensSliceGetLength(slice) <= INT_MAX)
		sllength = (ajint) ensSliceGetLength(slice);
	    else
		ajFatal("dnaAlignFeatureadaptorFetchAllBySQL got a Slice, "
			"which length (%u) exceeds the "
			"maximum integer limit (%d).",
			ensSliceGetLength(slice), INT_MAX);
	    
	    /*
	    ** Nothing needs to be done if the destination Slice starts at 1
	    ** and is on the forward strand.
	    */
	    
	    if((ensSliceGetStart(slice) != 1) ||
               (ensSliceGetStrand(slice) < 0))
	    {
		if(ensSliceGetStrand(slice) >= 0)
		{
		    slstart = slstart - ensSliceGetStart(slice) + 1;
		    
		    slend = slend - ensSliceGetStart(slice) + 1;
		}
		else
		{
		    slend = ensSliceGetEnd(slice) - slstart + 1;
		    
		    slstart = ensSliceGetEnd(slice) - slend + 1;
		    
		    slstrand *= -1;
		}
	    }
	    
	    /*
	    ** Throw away Features off the end of the requested Slice or on
	    ** any other than the requested Slice.
	    */
	    
	    if((slend < 1) ||
               (slstart > sllength) ||
	       (srid =! ensSliceGetSeqregionIdentifier(slice)))
	    {
		/* Next feature but destroy first! */
		
		ajStrDel(&hitname);
		
		ajStrDel(&cigar);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		continue;
	    }
	    
	    /* Delete the Sequence Region Slice and set the requested Slice. */
	    
	    ensSliceDel(&srslice);
	    
	    srslice = ensSliceNewRef(slice);
	}
	
	ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);
	
	ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);
	
	srcfeature = ensFeatureNewS(analysis,
				    srslice,
				    slstart,
				    slend,
				    slstrand);
	
	trgfeature = ensFeatureNewN((EnsPAnalysis) NULL,
				    hitname,
				    hitstart,
				    hitend,
				    hitstrand);
	
	fp = ensFeaturepairNew(srcfeature,
			       trgfeature,
			       edb,
			       extra,
			       (AjPStr) NULL, /* srcspecies */
			       (AjPStr) NULL, /* trgspecies */
			       0, /* groupid */
			       0, /* levelid */
			       evalue,
			       score,
			       0, /* srccoverage */
			       hitcoverage,
			       identity);
	
	/*
	** Finally, create a Base Align Feature object of type
	** ensEBasealignfeatureTypeDNA.
	*/
	
	baf = ensBasealignfeatureNewD(dafa, identifier, fp, cigar, pair);
	
	ajListPushAppend(bafs, (void *) baf);	
	
	ensFeaturepairDel(&fp);
	
	ensFeatureDel(&srcfeature);
	
	ensFeatureDel(&trgfeature);
	
	ensExternaldatabaseDel(&edb);
	
	ensAnalysisDel(&analysis);
	
	ajStrDel(&hitname);
	
	ajStrDel(&cigar);
	
	ensSliceDel(&srslice);
	
	ensAssemblymapperDel(&am);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl DNA Align Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** DNA Align Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDNAAlignFeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                  Feature Adaptor
** @argrule Ref object [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                  Feature Adaptor
**
** @valrule * [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensDNAAlignFeatureadaptorNew *****************************************
**
** Default Ensembl DNA Align Feature Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align Feature Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPDNAAlignFeatureadaptor ensDNAAlignFeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPDNAAlignFeatureadaptor dafa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(dafa);
    
    dafa->Adaptor = ensFeatureadaptorNew(
        dba,
        dnaAlignFeatureadaptorTables,
        dnaAlignFeatureadaptorColumns,
        dnaAlignFeatureadaptorLeftJoin,
        dnaAlignFeatureadaptorDefaultCondition,
        dnaAlignFeatureadaptorFinalCondition,
        dnaAlignFeatureadaptorFetchAllBySQL,
        (void* (*)(const void* key)) NULL, /* Fread */
        baseAlignFeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL, /* Fwrite */
        baseAlignFeatureadaptorCacheDelete,
        baseAlignFeatureadaptorCacheSize,
        baseAlignFeatureadaptorGetFeature,
        "DNA Align Feature");
    
    return dafa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl DNA Align Feature Adaptor.
**
** @fdata [EnsPDNAAlignFeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl DNA Align Feature Adaptor object
**
** @argrule * Padaptor [EnsPDNAAlignFeatureadaptor*] Ensembl DNA Align
**                                                   Feature Adaptor
**                                                   object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDNAAlignFeatureadaptorDel *****************************************
**
** Default destructor for an Ensembl DNA Align Feature Adaptor.
**
** @param [d] Padaptor [EnsPDNAAlignFeatureadaptor*] Ensembl DNA Align Feature
**                                                   Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensDNAAlignFeatureadaptorDel(EnsPDNAAlignFeatureadaptor *Padaptor)
{
    EnsPDNAAlignFeatureadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if (! *Padaptor)
	return;

    pthis = *Padaptor;
    
    ensFeatureadaptorDel(&pthis->Adaptor);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Base Align Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPDNAAlignFeatureadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Base Align Feature object(s)
** @nam4rule FetchAll Retrieve all Ensembl Base Align Feature objects
** @nam5rule FetchAllBy Retrieve all Ensembl Base Align Feature objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Base Align Feature object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPDNAAlignFeatureadaptor] Ensembl Base Align
**                                                       Feature Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDNAAlignFeatureadaptorFetchByIdentifier ***************************
**
** Fetch an Ensembl DNA Align Feature via its SQL database-internal identifier.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchByIdentifier(
    EnsPDNAAlignFeatureadaptor adaptor,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf)
{
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Pbaf)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    *Pbaf = (EnsPBasealignfeature)
	ensBaseadaptorFetchByIdentifier(ba, identifier);
    
    return ajTrue;
}




/* @func ensDNAAlignFeatureadaptorFetchAllBySliceIdentity *********************
**
** Fetch Ensembl DNA Align Features via an Ensembl Slice and
** an alignment identity threshold.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] identity [float] Alignment identity threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchAllBySliceIdentity(
    EnsPDNAAlignFeatureadaptor adaptor,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    if(identity > 0)
	constraint = ajFmtStr("dna_align_feature.perc_ident > %f", identity);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDNAAlignFeatureadaptorFetchAllBySliceCoverage *********************
**
** Fetch Ensembl DNA Align Features via an Ensembl Slice and
** an alignment target coverage.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] coverage [float] Alignment coverage threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchAllBySliceCoverage(
    EnsPDNAAlignFeatureadaptor adaptor,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    if(coverage > 0)
	constraint = ajFmtStr("dna_align_feature.hcoverage > %f", coverage);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDNAAlignFeatureadaptorFetchAllBySliceExternaldatabase *************
**
** Fetch Ensembl DNA Align Features via an Ensembl Slice and
** an Ensembl External Database name.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] edbname [const AjPStr] Ensembl External Database name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchAllBySliceExternaldatabase(
    EnsPDNAAlignFeatureadaptor adaptor,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txtname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!edbname)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    ensFeatureadaptorEscapeC(adaptor->Adaptor, &txtname, edbname);
    
    constraint = ajFmtStr("external_db.db_name = %s", txtname);
    
    ajCharDel(&txtname);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDNAAlignFeatureadaptorFetchAllByHitName ***************************
**
** Fetch Ensembl DNA Align Features via a hit name.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (Target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchAllByHitName(
    EnsPDNAAlignFeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txthitname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!hitname)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    ensBaseadaptorEscapeC(ba, &txthitname, hitname);
    
    constraint = ajFmtStr("dna_align_feature.hit_name = '%s'", txthitname);
    
    ajCharDel(&txthitname);
    
    /* Add the Ensembl Analysis name constraint. */
    
    if(anname && ajStrGetLen(anname))
	featureAdaptorAnalysisNameToConstraint(adaptor->Adaptor,
					       &constraint,
					       anname);
    
    value = ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       bafs);
    
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDNAAlignFeatureadaptorFetchAllByHitUnversioned ********************
**
** Fetch Ensembl DNA Align Features via an unversioned hit name.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (Target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchAllByHitUnversioned(
    EnsPDNAAlignFeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txthitname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!hitname)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    ensBaseadaptorEscapeC(ba, &txthitname, hitname);
    
    constraint = ajFmtStr("dna_align_feature.hit_name LIKE '%s.%'",
                          txthitname);
    
    ajCharDel(&txthitname);
    
    /* Add the Ensembl Analysis name constraint. */
    
    if(anname && ajStrGetLen(anname))
	featureAdaptorAnalysisNameToConstraint(adaptor->Adaptor,
					       &constraint,
					       anname);
    
    value = ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDNAAlignFeatureadaptorFetchAllIdentifiers *************************
**
** Fetch all SQL database-internal identifiers of Ensembl DNA Align Features.
**
** @param [u] adaptor [EnsPDNAAlignFeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @param [u] idlist [AjPList] AJAX List of ajuint identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDNAAlignFeatureadaptorFetchAllIdentifiers(
    EnsPDNAAlignFeatureadaptor adaptor,
    AjPList idlist)
{
    AjBool value = AJFALSE;
    
    AjPStr table = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!idlist)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    table = ajStrNewC("dna_align_feature");
    
    value = ensBaseadaptorFetchAllIdentifiers(ba,
					      table,
					      (AjPStr) NULL,
					      idlist);
    
    ajStrDel(&table);
    
    return value;
}




/* @datasection [EnsPProteinalignfeatureadaptor] Protein Align Feature Adaptor
**
** Functions for manipulating Ensembl Protein Align Feature Adaptor objects
**
** @cc Bio::EnsEMBL::BBSQL::ProteinAlignFeatureAdaptor CVS Revision: 1.46
** @cc Bio::EnsEMBL::DBSQL::BaseAlignFeatureAdaptor CVS Revision: 1.30
**
** @nam2rule Proteinalignfeatureadaptor
**
******************************************************************************/

static const char *proteinAlignFeatureadaptorTables[] =
{
    "protein_align_feature",
    /* FIXME: The External Database Adaptor has an internal cache.
       "external_db",
    */
    NULL
};




static const char *proteinAlignFeatureadaptorColumns[] =
{
    "protein_align_feature.protein_align_feature_id",
    "protein_align_feature.seq_region_id",
    "protein_align_feature.seq_region_start",
    "protein_align_feature.seq_region_end",
    "protein_align_feature.seq_region_strand",
    "protein_align_feature.hit_start",
    "protein_align_feature.hit_end",
    "protein_align_feature.hit_name",
    "protein_align_feature.analysis_id",
    "protein_align_feature.cigar_line",
    "protein_align_feature.score",
    "protein_align_feature.evalue",
    "protein_align_feature.perc_ident",
    "protein_align_feature.external_db_id",
    "protein_align_feature.hcoverage",
    "protein_align_feature.external_data",
    /*
      "protein_align_feature.pair_protein_align_feature_id",
    */
    /* FIXME: The External Database Adaptor has an internal cache.
       "external_db.db_name",
       "external_db.db_display_name",
    */
    NULL
};




static EnsOBaseadaptorLeftJoin proteinAlignFeatureadaptorLeftJoin[] =
{
    /* FIXME: The External Database Adaptor has an internal cache.
       {
       "external_db",
       "protein_align_feature.external_db_id = "
       "external_db.external_db_id"
       },
    */
    {NULL, NULL}
};




static const char *proteinAlignFeatureadaptorDefaultCondition = NULL;

static const char *proteinAlignFeatureadaptorFinalCondition = NULL;




/* @funcstatic proteinAlignFeatureadaptorFetchAllBySQL ************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Protein Align Features.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [r] mapper [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool proteinAlignFeatureadaptorFetchAllBySQL(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper mapper,
    EnsPSlice slice,
    AjPList bafs)
{
    double evalue = 0;
    double score  = 0;
    
    float identity    = 0;
    float hitcoverage = 0;
    
    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint edbid      = 0;
    
    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    
    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    
    ajuint hitstart = 0;
    ajuint hitend   = 0;
    ajint srstrand  = 0;
    
    AjPList mrs = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr hitname = NULL;
    AjPStr cigar   = NULL;
    
    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPBasealignfeature baf = NULL;
    
    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;
    
    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;
    
    EnsPFeaturepair fp = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPProteinalignfeatureadaptor pafa = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

#if AJFALSE
    ajDebug("proteinAlignFeatureadaptorFetchAllBySQL\n"
	    "  dba %p\n"
	    "  statement %p\n"
	    "  mapper %p\n"
	    "  slice %p\n"
	    "  bafs %p\n",
	    dba,
	    statement,
	    mapper,
	    slice,
	    bafs);
#endif
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    edba = ensRegistryGetExternaldatabaseadaptor(dba);
    
    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier  = 0;
	srid        = 0;
	srstart     = 0;
	srend       = 0;
	srstrand    = 0;
	hitstart    = 0;
	hitend      = 0;
	hitname     = ajStrNew();
	analysisid  = 0;
	cigar       = ajStrNew();
	score       = 0;
	evalue      = 0;
	identity    = 0;
	edbid       = 0;
	hitcoverage = 0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToUint(sqlr, &hitstart);
	ajSqlcolumnToUint(sqlr, &hitend);
	ajSqlcolumnToStr(sqlr, &hitname);
	ajSqlcolumnToUint(sqlr, &analysisid);
	ajSqlcolumnToStr(sqlr, &cigar);
	ajSqlcolumnToDouble(sqlr, &score);
	ajSqlcolumnToDouble(sqlr, &evalue);
	ajSqlcolumnToFloat(sqlr, &identity);
	ajSqlcolumnToUint(sqlr, &edbid);
	ajSqlcolumnToFloat(sqlr, &hitcoverage);
	
	/*
	** Since the Ensembl SQL schema defines Sequence Region start and end
	** coordinates as unsigned integers for all Features, the range needs
	** checking.
	*/
	
	if(srstart <= INT_MAX)
	    slstart = (ajint) srstart;
	else
	    ajFatal("proteinAlignFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("proteinAlignFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region end coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srend, INT_MAX);
	
	slstrand = srstrand;
	
	/* Fetch a Slice spanning the entire Sequence Region. */
	
	ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);
	
	/*
	** Obtain an Ensembl Assembly Mapper if none was defined, but a
	** destination Slice was.
	*/
	
	if(mapper)
	    am = ensAssemblymapperNewRef(mapper);
	
	if((! mapper) &&
	   slice &&
	   (! ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                                  ensSliceGetCoordsystem(srslice))))
	    am = ensAssemblymapperadaptorFetchByCoordsystems(
                ama,
                ensSliceGetCoordsystem(slice),
                ensSliceGetCoordsystem(srslice));
	
	/*
	** Remap the Feature coordinates to another Ensembl Coordinate System
	** if an Ensembl Mapper was provided.
	*/
	
	if(am)
	{
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(srslice),
				     slstart,
				     slend,
				     slstrand,
				     mrs);
	    
	    /*
	    ** The ensAssemblymapperFastMap function returns at best one
	    ** Ensembl Mapper Result.
	    */
	    
	    ajListPop(mrs, (void **) &mr);
	    
	    /*
	    ** Skip Features that map to gaps or
	    ** Coordinate System boundaries.
	    */
	    
	    if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
	    {
		/* Load the next Feature but destroy first! */
		
		ajStrDel(&hitname);
		
		ajStrDel(&cigar);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		ensMapperresultDel(&mr);
		
		continue;
	    }
	    
	    srid = ensMapperresultGetObjectIdentifier(mr);
	    
	    slstart = ensMapperresultGetStart(mr);
	    
	    slend = ensMapperresultGetEnd(mr);
	    
	    slstrand = ensMapperresultGetStrand(mr);
	    
	    /*
	    ** FIXME: Like in the Bio::EnsEMBL::DBSQL::ExonAdaptor
	    ** code, a construct to get a Slice from the cache is
	    ** commented out from the
	    ** Bio::EnsEMBL::DBSQL::ProteinAlignFeatureAdaptor of the Perl API.
	    ** See CVS versions 1.39 and 1.39.14.1 for details.
	    */
	    
	    /*
	    ** Delete the Sequence Region Slice and fetch a Slice in the
	    ** Coordinate System we just mapped to.
	    */
	    
	    ensSliceDel(&srslice);
	    
	    ensSliceadaptorFetchBySeqregionIdentifier(sa,
						      srid,
						      0,
						      0,
						      0,
						      &srslice);
	    
	    ensMapperresultDel(&mr);
	}
	
	/*
	** Convert Sequence Region Slice coordinates to destination Slice
	** coordinates, if a destination Slice has been provided.
	*/
	
	if(slice)
	{
	    /* Check that the length of the Slice is within range. */
	    
	    if(ensSliceGetLength(slice) <= INT_MAX)
		sllength = (ajint) ensSliceGetLength(slice);
	    else
		ajFatal("proteinAlignFeatureadaptorFetchAllBySQL got a Slice, "
			"which length (%u) exceeds the "
			"maximum integer limit (%d).",
			ensSliceGetLength(slice), INT_MAX);
	    
	    /*
	    ** Nothing needs to be done if the destination Slice starts at 1
	    ** and is on the forward strand.
	    */
	    
	    if((ensSliceGetStart(slice) != 1) ||
               (ensSliceGetStrand(slice) < 0))
	    {
		if(ensSliceGetStrand(slice) >= 0)
		{
		    slstart = slstart - ensSliceGetStart(slice) + 1;
		    
		    slend = slend - ensSliceGetStart(slice) + 1;
		}
		else
		{
		    slend = ensSliceGetEnd(slice) - slstart + 1;
		    
		    slstart = ensSliceGetEnd(slice) - slend + 1;
		    
		    slstrand *= -1;
		}
	    }
	    
	    /*
	    ** Throw away Features off the end of the requested Slice or on
	    ** any other than the requested Slice.
	    */
	    
	    if((slend < 1) ||
               (slstart > sllength) ||
	       (srid =! ensSliceGetSeqregionIdentifier(slice)))
	    {
		/* Next feature but destroy first! */
		
		ajStrDel(&hitname);
		
		ajStrDel(&cigar);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		continue;
	    }
	    
	    /* Delete the Sequence Region Slice and set the requested Slice. */
	    
	    ensSliceDel(&srslice);
	    
	    srslice = ensSliceNewRef(slice);
	}
	
	ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);
	
	ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);
	
	srcfeature = ensFeatureNewS(analysis,
				    srslice,
				    slstart,
				    slend,
				    slstrand);
	
	trgfeature = ensFeatureNewN((EnsPAnalysis) NULL,
				    hitname,
				    hitstart,
				    hitend,
				    1);
	
	fp = ensFeaturepairNew(srcfeature,
			       trgfeature,
			       edb,
			       (AjPStr) NULL, /* extra data */
			       (AjPStr) NULL, /* srcspecies */
			       (AjPStr) NULL, /* trgspecies */
			       0, /* groupid */
			       0, /* levelid */
			       evalue,
			       score,
			       0, /* srccoverage */
			       0, /* trgcoverage */
			       identity);
	
	/*
	** Finally, create a Base Align Feature object of type
	** ensEBasealignfeatureTypeProtein.
	*/
	
	baf = ensBasealignfeatureNewP(pafa, identifier, fp, cigar);
	
	ajListPushAppend(bafs, (void *) baf);	
	
	ensFeaturepairDel(&fp);
	
	ensFeatureDel(&srcfeature);
	
	ensFeatureDel(&trgfeature);
	
	ensExternaldatabaseDel(&edb);
	
	ensAnalysisDel(&analysis);
	
	ajStrDel(&hitname);
	
	ajStrDel(&cigar);
	
	ensSliceDel(&srslice);
	
	ensAssemblymapperDel(&am);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Protein Align Feature Adaptor by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Protein Align Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProteinalignfeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                      Feature Adaptor
** @argrule Ref object [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                      Feature Adaptor
**
** @valrule * [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                             Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensProteinalignfeatureadaptorNew *************************************
**
** Default Ensembl Protein Align Feature Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPProteinalignfeatureadaptor] Ensembl Protein Align Feature
**                                          Adaptor or NULL
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensProteinalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPProteinalignfeatureadaptor pafa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(pafa);
    
    pafa->Adaptor = ensFeatureadaptorNew(
        dba,
        proteinAlignFeatureadaptorTables,
        proteinAlignFeatureadaptorColumns,
        proteinAlignFeatureadaptorLeftJoin,
        proteinAlignFeatureadaptorDefaultCondition,
        proteinAlignFeatureadaptorFinalCondition,
        proteinAlignFeatureadaptorFetchAllBySQL,
        (void* (*)(const void* key)) NULL, /* Fread */
        baseAlignFeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL, /* Fwrite */
        baseAlignFeatureadaptorCacheDelete,
        baseAlignFeatureadaptorCacheSize,
        baseAlignFeatureadaptorGetFeature,
        "Protein Align Feature");
    
    return pafa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Protein Align Feature Adaptor.
**
** @fdata [EnsPProteinalignfeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Protein Align Feature Adaptor object
**
** @argrule * Padaptor [EnsPProteinalignfeatureadaptor*] Ensembl Protein Align
**                                                       Feature Adaptor
**                                                       object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProteinalignfeatureadaptorDel *************************************
**
** Default destructor for an Ensembl Protein Align Feature Adaptor.
**
** @param [d] Padaptor [EnsPProteinalignfeatureadaptor*] Ensembl Protein Align
**                                                       Feature Adaptor
**                                                       address
**
** @return [void]
** @@
******************************************************************************/

void ensProteinalignfeatureadaptorDel(EnsPProteinalignfeatureadaptor *Padaptor)
{
    EnsPProteinalignfeatureadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    ensFeatureadaptorDel(&pthis->Adaptor);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @func ensProteinalignfeatureadaptorFetchByIdentifier ***********************
**
** Fetch an Ensembl Protein Align Feature via its SQL database-internal
** identifier.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchByIdentifier(
    EnsPProteinalignfeatureadaptor adaptor,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf)
{
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Pbaf)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    *Pbaf = (EnsPBasealignfeature)
	ensBaseadaptorFetchByIdentifier(ba, identifier);
    
    return ajTrue;
}




/* @func ensProteinalignfeatureadaptorFetchAllBySliceIdentity *****************
**
** Fetch Ensembl Protein Align Features via an Ensembl Slice and
** an alignment identity threshold.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] identity [float] Alignment identity threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllBySliceIdentity(
    EnsPProteinalignfeatureadaptor adaptor,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    if(identity > 0)
	constraint = ajFmtStr("protein_align_feature.perc_ident > %f",
                              identity);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensProteinalignfeatureadaptorFetchAllBySliceCoverage *****************
**
** Fetch Ensembl Protein Align Features via an Ensembl Slice and
** an alignment target coverage.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] coverage [float] Alignment target coverage threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllBySliceCoverage(
    EnsPProteinalignfeatureadaptor adaptor,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    if(coverage > 0)
	constraint = ajFmtStr("protein_align_feature.hcoverage > %f",
                              coverage);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensProteinalignfeatureadaptorFetchAllBySliceExternaldatabase *********
**
** Fetch Ensembl Protein Align Features via an Ensembl Slice and
** an Ensembl External Database name.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] edbname [const AjPStr] Ensembl External Database name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllBySliceExternaldatabase(
    EnsPProteinalignfeatureadaptor adaptor,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txtname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!edbname)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    ensFeatureadaptorEscapeC(adaptor->Adaptor, &txtname, edbname);
    
    constraint = ajFmtStr("external_db.db_name = %s", txtname);
    
    ajCharDel(&txtname);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensProteinalignfeatureadaptorFetchAllByHitName ***********************
**
** Fetch Ensembl Protein Align Features via a hit name.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (Target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllByHitName(
    EnsPProteinalignfeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txthitname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!hitname)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    ensBaseadaptorEscapeC(ba, &txthitname, hitname);
    
    constraint = ajFmtStr("protein_align_feature.hit_name = '%s'", txthitname);
    
    ajCharDel(&txthitname);
    
    /* Add the Ensembl Analysis name constraint. */
    
    if(anname && ajStrGetLen(anname))
	featureAdaptorAnalysisNameToConstraint(adaptor->Adaptor,
					       &constraint,
					       anname);
    
    value = ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       bafs);
    
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensProteinalignfeatureadaptorFetchAllByHitUnversioned ****************
**
** Fetch Ensembl Protein Align Features via an unversioned hit name.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (Target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllByHitUnversioned(
    EnsPProteinalignfeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txthitname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!hitname)
	return ajFalse;
    
    if(!bafs)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    ensBaseadaptorEscapeC(ba, &txthitname, hitname);
    
    constraint = ajFmtStr("protein_align_feature.hit_name LIKE '%s.%'",
                          txthitname);
    
    ajCharDel(&txthitname);
    
    /* Add the Ensembl Analysis name constraint. */
    
    if(anname && ajStrGetLen(anname))
	featureAdaptorAnalysisNameToConstraint(adaptor->Adaptor,
					       &constraint,
					       anname);
    
    value = ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       bafs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensProteinalignfeatureadaptorFetchAllIdentifiers *********************
**
** Fetch all SQL database-internal identifiers of
** Ensembl Protein Align Features.
**
** @param [u] adaptor [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
** @param [u] idlist [AjPList] AJAX List of ajuint identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllIdentifiers(
    EnsPProteinalignfeatureadaptor adaptor,
    AjPList idlist)
{
    AjBool value = AJFALSE;
    
    AjPStr table = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!idlist)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    table = ajStrNewC("protein_align_feature");
    
    value = ensBaseadaptorFetchAllIdentifiers(ba,
					      table,
					      (AjPStr) NULL,
					      idlist);
    
    ajStrDel(&table);
    
    return value;
}




/* @datasection [EnsPProteinfeature] Protein Feature **************************
**
** Functions for manipulating Ensembl Protein Feature objects
**
** @cc Bio::EnsEMBL::ProteinFeature CVS Revision: 1.12
**
** @nam2rule Proteinfeature
**
******************************************************************************/





/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Protein Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Protein Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProteinfeature]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPProteinfeature] Ensembl Protein Feature
** @argrule Ref object [EnsPProteinfeature] Ensembl Protein Feature
**
** @valrule * [EnsPProteinfeature] Ensembl Protein Feature
**
** @fcategory new
******************************************************************************/




/* @func ensProteinfeatureNew *************************************************
**
** Default Ensembl Protein Feature constructor.
**
** @cc Bio::EnsEMBL::Storable
** @param [r] adaptor [EnsPProteinfeatureadaptor] Ensembl Protein
**                                                Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [r] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::ProteinFeature
** @param [r] accession [AjPStr] (InterPro) Accession
** @param [r] description [AjPStr] (InterPro) Description
**
** @return [EnsPProteinfeature] Ensembl Protein Feature or NULL
** @@
******************************************************************************/

/*
** FIXME: CVS Revision 1.11 introduces a translation_id method, but the
** POD is a copy of the idesc.
** TODO: Report to the Ensembl Core team.
*/

EnsPProteinfeature ensProteinfeatureNew(EnsPProteinfeatureadaptor adaptor,
                                        ajuint identifier,
                                        EnsPFeaturepair fp,
                                        AjPStr accession,
                                        AjPStr description)
{
    EnsPProteinfeature pf = NULL;
    
    AJNEW0(pf);
    
    pf->Use = 1;
    
    pf->Identifier = identifier;
    
    pf->Adaptor = adaptor;
    
    pf->Featurepair = ensFeaturepairNewRef(fp);
    
    if(accession)
	pf->Accession = ajStrNewRef(accession);
    
    if(description)
	pf->Description = ajStrNewRef(description);
    
    return pf;
}




/* @func ensProteinfeatureNewObj **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPProteinfeature] Ensembl Protein Feature or NULL
** @@
******************************************************************************/

EnsPProteinfeature ensProteinfeatureNewObj(const EnsPProteinfeature object)
{
    EnsPProteinfeature pf = NULL;
    
    AJNEW0(pf);
    
    pf->Use = 1;
    
    pf->Adaptor = object->Adaptor;
    
    pf->Featurepair = ensFeaturepairNewRef(object->Featurepair);
    
    if(object->Accession)
        pf->Accession = ajStrNewRef(object->Accession);
    
    if(object->Description)
        pf->Description = ajStrNewRef(object->Description);
    
    return pf;
}




/* @func ensProteinfeatureNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] baf [EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPProteinfeature] Ensembl Protein Feature or NULL
** @@
******************************************************************************/

EnsPProteinfeature ensProteinfeatureNewRef(EnsPProteinfeature pf)
{
    if(!pf)
	return NULL;
    
    pf->Use++;
    
    return pf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Protein Feature.
**
** @fdata [EnsPProteinfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Protein Feature object
**
** @argrule * Ppf [EnsPProteinfeature*] Protein Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProteinfeatureDel *************************************************
**
** Default destructor for an Ensembl Protein Feature.
**
** @param [d] Ppf [EnsPProteinfeature*] Ensembl Protein Feature address
**
** @return [void]
** @@
******************************************************************************/

void ensProteinfeatureDel(EnsPProteinfeature *Ppf)
{
    EnsPProteinfeature pthis = NULL;
    
    if(!Ppf)
	return;
    
    if(!*Ppf)
	return;

    pthis = *Ppf;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Ppf = NULL;
	
	return;
    }
    
    ensFeaturepairDel(&pthis->Featurepair);
    
    ajStrDel(&pthis->Accession);
    ajStrDel(&pthis->Description);
    
    AJFREE(pthis);

    *Ppf = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
** @fnote None
**
** @nam3rule Get Return Protein Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Protein Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeaturepair Return the Ensembl Feature Pair
** @nam4rule GetAccession Return the accession
** @nam4rule GetDescription Return the description
**
** @argrule * pf [const EnsPProteinfeature] Protein Feature
**
** @valrule Adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @valrule Identifier [ajuint] SQL databse-internal identifier
** @valrule Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @valrule Accession [AjPStr] Accession
** @valrule Description [AjPStr] Description
**
** @fcategory use
******************************************************************************/




/* @func ensProteinfeatureGetAdaptor ******************************************
**
** Get the Ensembl Protein Feature Adaptor element of an
** Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @@
******************************************************************************/

EnsPProteinfeatureadaptor ensProteinfeatureGetAdaptor(
    const EnsPProteinfeature pf)
{
    if(!pf)
        return NULL;
    
    return pf->Adaptor;
}




/* @func ensProteinfeatureGetIdentifier ***************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensProteinfeatureGetIdentifier(const EnsPProteinfeature pf)
{
    if(!pf)
        return 0;
    
    return pf->Identifier;
}




/* @func ensProteinfeatureGetFeaturepair **************************************
**
** Get the Ensembl Feature Pair element of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPFeaturepair] Ensembl Feature Pair
** @@
******************************************************************************/

EnsPFeaturepair ensProteinfeatureGetFeaturepair(const EnsPProteinfeature pf)
{
    if(!pf)
	return NULL;
    
    return pf->Featurepair;
}




/* @func ensProteinfeatureGetAccession ****************************************
**
** Get the (InterPro) accession element of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [AjPStr] (InterPro) accession
** @@
******************************************************************************/

AjPStr ensProteinfeatureGetAccession(const EnsPProteinfeature pf)
{
    if(!pf)
	return NULL;
    
    return pf->Accession;
}




/* @func ensProteinfeatureGetDescription **************************************
**
** Get the (InterPro) description element of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [AjPStr] (InterPro) description
** @@
******************************************************************************/

AjPStr ensProteinfeatureGetDescription(const EnsPProteinfeature pf)
{
    if(!pf)
	return NULL;
    
    return pf->Description;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
** @fnote None
**
** @nam3rule Set Set one element of a Protein Feature
** @nam4rule SetAdaptor Set the Ensembl Protein Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeaturepair Set the Ensembl Feature Pair
** @nam4rule SetAccession Set the accession
** @nam4rule SetDescription Set the description
**
** @argrule * pf [EnsPProteinfeature] Ensembl Protein Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensProteinfeatureSetAdaptor ******************************************
**
** Set the Ensembl Protein Feature Adaptor element of an
** Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [r] adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature
**                                                Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureSetAdaptor(EnsPProteinfeature pf,
                                   EnsPProteinfeatureadaptor adaptor)
{
    if(!pf)
        return ajFalse;
    
    pf->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensProteinfeatureSetIdentifier ***************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureSetIdentifier(EnsPProteinfeature pf, ajuint identifier)
{
    if(!pf)
        return ajFalse;
    
    pf->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensProteinfeatureSetFeaturepair **************************************
**
** Set the Ensembl Feature Pair element of an Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureSetFeaturepair(EnsPProteinfeature pf,
                                       EnsPFeaturepair fp)
{
    if(!pf)
        return ajFalse;
    
    ensFeaturepairDel(&pf->Featurepair);
    
    pf->Featurepair = ensFeaturepairNewRef(fp);
    
    return ajTrue;
}




/* @func ensProteinfeatureSetAccession ****************************************
**
** Set the accession element of an Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] accession [AjPStr] Accession
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureSetAccession(EnsPProteinfeature pf, AjPStr accession)
{
    if(!pf)
        return ajFalse;
    
    ajStrDel(&pf->Accession);
    
    pf->Accession = ajStrNewRef(accession);
    
    return ajTrue;
}




/* @func ensProteinfeatureSetDescription **************************************
**
** Set the description element of an Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureSetDescription(EnsPProteinfeature pf,
                                       AjPStr description)
{
    if(!pf)
        return ajFalse;
    
    ajStrDel(&pf->Description);
    
    pf->Description = ajStrNewRef(description);
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
** @nam3rule Trace Report Ensembl Protein Feature elements to debug file
**
** @argrule Trace pf [const EnsPProteinfeature] Ensembl Protein Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensProteinfeatureTrace ***********************************************
**
** Trace an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureTrace(const EnsPProteinfeature pf, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!pf)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensProteinfeatureTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Featurepair %p\n"
	    "%S  Accession '%S'\n"
	    "%S  Description '%S'\n",
	    indent, pf,
	    indent, pf->Use,
	    indent, pf->Identifier,
	    indent, pf->Adaptor,
	    indent, pf->Featurepair,
	    indent, pf->Accession,
	    indent, pf->Description);
    
    ensFeaturepairTrace(pf->Featurepair, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensProteinfeatureGetMemSize ******************************************
**
** Get the memory size in bytes of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensProteinfeatureGetMemSize(const EnsPProteinfeature pf)
{
    ajuint size = 0;
    
    if(!pf)
	return 0;
    
    size += (ajuint) sizeof (EnsOProteinfeature);
    
    size += ensFeaturepairGetMemSize(pf->Featurepair);
    
    if(pf->Accession)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(pf->Accession);
    }
    
    if(pf->Description)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(pf->Description);
    }
    
    return size;
}




/* @datasection [EnsPProteinfeatureadaptor] Protein Feature Adaptor ***********
**
** Functions for manipulating Ensembl Protein Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::ProteinFeatureAdaptor CVS Revision: 1.29
**
** @nam2rule Proteinfeatureadaptor
**
******************************************************************************/

static const char *proteinFeatureadaptorTables[] =
{
    "protein_feature",
    NULL
};




static const char *proteinFeatureadaptorColumns[] =
{
    "protein_feature.protein_feature_id",
    "protein_feature.translation_id",
    "protein_feature.seq_start",
    "protein_feature.seq_end",
    "protein_feature.hit_name",
    "protein_feature.hit_start",
    "protein_feature.hit_end",
    "protein_feature.analysis_id",
    "protein_feature.score",
    "protein_feature.evalue",
    "protein_feature.perc_ident",
    "interpro.interpro_ac",
    "xref.display_label",
    NULL
};




static EnsOBaseadaptorLeftJoin proteinFeatureadaptorLeftJoin[] =
{
    {"interpro", "protein_feature.hit_name = interpro.id"},
    {"xref", "interpro.interpro_ac = xref.dbprimary_acc"},
    {NULL, NULL}
};




static const char *proteinFeatureadaptorDefaultCondition = NULL;

static const char *proteinFeatureadaptorFinalCondition = NULL;




/* @funcstatic proteinFeatureadaptorFetchAllBySQL *****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Protein Features.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [r] mapper [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] pfs [AjPList] AJAX List of Ensembl Protein Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool proteinFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                 const AjPStr statement,
                                                 EnsPAssemblymapper mapper,
                                                 EnsPSlice slice,
                                                 AjPList pfs)
{
    double evalue  = 0;
    double score   = 0;
    float identity = 0;
    
    ajuint identifier = 0;
    ajuint tlstart    = 0;
    ajuint tlend      = 0;
    ajuint analysisid = 0;
    ajuint hitstart   = 0;
    ajuint hitend     = 0;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr tlid        = NULL;
    AjPStr hitname     = NULL;
    AjPStr accession   = NULL;
    AjPStr description = NULL;
    
    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;
    
    EnsPFeaturepair fp = NULL;
    
    EnsPProteinfeature pf         = NULL;
    EnsPProteinfeatureadaptor pfa = NULL;
    
    ajDebug("proteinFeatureadaptorFetchAllBySQL\n"
	    "  dba %p\n"
	    "  statement %p\n"
	    "  mapper %p\n"
	    "  slice %p\n"
	    "  pfs %p\n",
	    dba,
	    statement,
	    mapper,
	    slice,
	    pfs);
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!pfs)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    pfa = ensRegistryGetProteinfeatureadaptor(dba);
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier  = 0;
	tlid        = ajStrNew();
	tlstart     = 0;
	tlend       = 0;
	hitname     = ajStrNew();
	hitstart    = 0;
	hitend      = 0;
	analysisid  = 0;
	score       = 0;
	evalue      = 0;
	identity    = 0;
	accession   = ajStrNew();
	description = ajStrNew();
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToStr(sqlr, &tlid);
	ajSqlcolumnToUint(sqlr, &tlstart);
	ajSqlcolumnToUint(sqlr, &tlend);
	ajSqlcolumnToStr(sqlr, &hitname);
	ajSqlcolumnToUint(sqlr, &hitstart);
	ajSqlcolumnToUint(sqlr, &hitend);
	ajSqlcolumnToUint(sqlr, &analysisid);
	ajSqlcolumnToDouble(sqlr, &score);
	ajSqlcolumnToDouble(sqlr, &evalue);
	ajSqlcolumnToFloat(sqlr, &identity);
	ajSqlcolumnToStr(sqlr, &accession);
	ajSqlcolumnToStr(sqlr, &description);
	
	ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);
	
	/*
	** FIXME: The translation_id is currently handled as an AJAX String
	** to fit into the seqname element of an Ensembl Feature. Let's see
	** how this goes.
	*/
	
	srcfeature = ensFeatureNewN(analysis, tlid, tlstart, tlend, 1);
	
	trgfeature = ensFeatureNewN((EnsPAnalysis) NULL,
				    hitname,
				    hitstart,
				    hitend,
				    1);
	
	fp = ensFeaturepairNew(srcfeature,
			       trgfeature,
			       (EnsPExternaldatabase) NULL, /* edb */
			       (AjPStr) NULL, /* extra data */
			       (AjPStr) NULL, /* srcspecies */
			       (AjPStr) NULL, /* trgspecies */
			       0, /* groupid */
			       0, /* levelid */
			       evalue,
			       score,
			       0, /* srccoverage */
			       0, /* trgcoverage */
			       identity);
	
	/* Create a Protein Feature object. */
	
	pf = ensProteinfeatureNew(pfa, identifier, fp, accession, description);
	
	ajListPushAppend(pfs, (void *) pf);	
	
	ensFeaturepairDel(&fp);
	
	ensFeatureDel(&srcfeature);
	ensFeatureDel(&trgfeature);
	
	ajStrDel(&tlid);
	ajStrDel(&hitname);
	ajStrDel(&accession);
	ajStrDel(&description);
	
	ensAnalysisDel(&analysis);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Protein Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Protein Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProteinfeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPProteinfeatureadaptor] Ensembl Protein
**                                                 Feature Adaptor
** @argrule Ref object [EnsPProteinfeatureadaptor] Ensembl Protein
**                                                 Feature Adaptor
**
** @valrule * [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensProteinfeatureadaptorNew ******************************************
**
** Default Ensembl Protein Feature Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPProteinfeatureadaptor ensProteinfeatureadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPProteinfeatureadaptor pfa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(pfa);
    
    pfa->Adaptor = ensBaseadaptorNew(
        dba,
        proteinFeatureadaptorTables,
        proteinFeatureadaptorColumns,
        proteinFeatureadaptorLeftJoin,
        proteinFeatureadaptorDefaultCondition,
        proteinFeatureadaptorFinalCondition,
        proteinFeatureadaptorFetchAllBySQL);
    
    return pfa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Protein Feature Adaptor.
**
** @fdata [EnsPProteinfeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Protein Feature Adaptor object
**
** @argrule * Padaptor [EnsPProteinfeatureadaptor*] Ensembl Protein Feature
**                                                  Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProteinfeatureadaptorDel ******************************************
**
** Default destructor for an Ensembl Protein Feature Adaptor.
**
** @param [d] Padaptor [EnsPProteinfeatureadaptor*] Ensembl Protein Feature
**                                                  Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensProteinfeatureadaptorDel(EnsPProteinfeatureadaptor *Padaptor)
{
    EnsPProteinfeatureadaptor pthis = NULL;

    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    ensBaseadaptorDel(&pthis->Adaptor);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Protein Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPProteinfeatureadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Protein Feature object(s)
** @nam4rule FetchAll Retrieve all Ensembl Protein Feature objects
** @nam5rule FetchAllBy Retrieve all Ensembl Protein Feature objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Protein Feature object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPProteinfeatureadaptor] Ensembl Protein Feature
**                                                      Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Protein Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensProteinfeatureadaptorFetchAllByTranslationIdentifier **************
**
** Fetch an Ensembl Protein Feature via an Ensembl Translation identifier.
** The caller is responsible for deleting the Ensembl Protein Feature.
**
** @param [r] adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature
*                                                 Adaptor
** @param [r] tlid [ajuint] Ensembl Translation identifier
** @param [u] pfs [AjPList] AJAX List of Ensembl Protein Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinfeatureadaptorFetchAllByTranslationIdentifier(
    EnsPProteinfeatureadaptor adaptor,
    ajuint tlid,
    AjPList pfs)
{
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!tlid)
	return ajFalse;
    
    if(!pfs)
	return ajFalse;
    
    constraint = ajFmtStr("protein_feature.translation_id = %u", tlid);
    
    ensBaseadaptorGenericFetch(adaptor->Adaptor,
			       constraint,
			       (EnsPAssemblymapper) NULL,
			       (EnsPSlice) NULL,
			       pfs);
    
    ajStrDel(&constraint);
    
    return ajTrue;
}




/* @func ensProteinfeatureadaptorFetchByIdentifier ****************************
**
** Fetch an Ensembl Protein Feature by its SQL database-internal identifier.
**
** @param [r] adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature
**                                                Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ppf [EnsPProteinfeature*] Ensembl Protein Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/


AjBool ensProteinfeatureadaptorFetchByIdentifier(
    EnsPProteinfeatureadaptor adaptor,
    ajuint identifier,
    EnsPProteinfeature *Ppf)
{
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Ppf)
	return ajFalse;
    
    *Ppf = (EnsPProteinfeature)
	ensBaseadaptorFetchByIdentifier(adaptor->Adaptor, identifier);
    
    return ajTrue;
}




/* @datasection [EnsPSimplefeature] Simple Feature ****************************
**
** Functions for manipulating Ensembl Simple Feature objects
**
** @cc Bio::EnsEMBL::SimpleFeature CVS Revision: 1.9
**
** @nam2rule Simplefeature
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Simple Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Simple Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSimplefeature]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPSimplefeature] Ensembl Simple Feature
** @argrule Ref object [EnsPSimplefeature] Ensembl Simple Feature
**
** @valrule * [EnsPSimplefeature] Ensembl Simple Feature
**
** @fcategory new
******************************************************************************/




/* @func ensSimplefeatureNew **************************************************
**
** Default Ensembl Simple Feature constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::SimpleFeature::new
** @param [u] label [AjPStr] Display label
** @param [r] score [double] Score
**
** @return [EnsPSimplefeature] Ensembl Simple Feature or NULL
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNew(EnsPSimplefeatureadaptor adaptor,
                                      ajuint identifier,
                                      EnsPFeature feature,
                                      AjPStr label,
                                      double score)
{
    EnsPSimplefeature sf = NULL;
    
    if(!feature)
        return NULL;
    
    AJNEW0(sf);
    
    sf->Use = 1;
    
    sf->Identifier = identifier;
    sf->Adaptor    = adaptor;
    sf->Feature    = ensFeatureNewRef(feature);
    
    if(label)
	sf->DisplayLabel = ajStrNewRef(label);
    
    sf->Score = score;
    
    return sf;
}




/* @func ensSimplefeatureNewObj ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeature] Ensembl Simple Feature or NULL
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewObj(const EnsPSimplefeature object)
{
    EnsPSimplefeature sf = NULL;
    
    AJNEW0(sf);
    
    sf->Use = 1;
    
    sf->Identifier = object->Identifier;
    sf->Adaptor    = object->Adaptor;
    sf->Feature    = ensFeatureNewRef(object->Feature);
    
    if(object->DisplayLabel)
	sf->DisplayLabel = ajStrNewRef(object->DisplayLabel);
    
    sf->Score = object->Score;
    
    return sf;
}




/* @func ensSimplefeatureNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeature] Ensembl Simple Feature or NULL
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewRef(EnsPSimplefeature sf)
{
    if(!sf)
	return NULL;
    
    sf->Use++;
    
    return sf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Simple Features.
**
** @fdata [EnsPSimplefeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Simple Feature object
**
** @argrule * Psf [EnsPSimplefeature*] Simple Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSimplefeatureDel **************************************************
**
** Default destructor for an Ensembl Simple Feature.
**
** @param [d] Psf [EnsPSimplefeature*] Ensembl Simple Feature address
**
** @return [void]
** @@
******************************************************************************/

void ensSimplefeatureDel(EnsPSimplefeature *Psf)
{
    EnsPSimplefeature pthis = NULL;
    
    if(!Psf)
        return;
    
    if(!*Psf)
        return;

    pthis = *Psf;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Psf = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    ajStrDel(&pthis->DisplayLabel);
    
    AJFREE(pthis);

    *Psf = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
** @fnote None
**
** @nam3rule Get Return Simple Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Simple Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetDisplayLabel Return the display label
** @nam4rule GetScore Return the score
**
** @argrule * sf [const EnsPSimplefeature] Simple Feature
**
** @valrule Adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule DisplayLabel [AjPStr] Display label
** @valrule Score [double] Score
**
** @fcategory use
******************************************************************************/




/* @func ensSimplefeatureGetAdaptor *******************************************
**
** Get the Ensembl Simple Feature Adaptor element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensSimplefeatureGetAdaptor(const EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;
    
    return sf->Adaptor;
}




/* @func ensSimplefeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensSimplefeatureGetIdentifier(const EnsPSimplefeature sf)
{
    if(!sf)
        return 0;
    
    return sf->Identifier;
}




/* @func ensSimplefeatureGetFeature *******************************************
**
** Get the Ensembl Feature element of an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensSimplefeatureGetFeature(const EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;
    
    return sf->Feature;
}




/* @func ensSimplefeatureGetDisplayLabel **************************************
**
** Get the display label element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::display_label
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [AjPStr] Display label
** @@
******************************************************************************/

AjPStr ensSimplefeatureGetDisplayLabel(const EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;
    
    return sf->DisplayLabel;
}




/* @func ensSimplefeatureGetScore *********************************************
**
** Get the score element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::score
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [double] Score
** @@
******************************************************************************/

double ensSimplefeatureGetScore(const EnsPSimplefeature sf)
{
    if(!sf)
        return 0;
    
    return sf->Score;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
** @fnote None
**
** @nam3rule Set Set one element of a Simple Feature
** @nam4rule SetAdaptor Set the Ensembl Simple Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetDisplayLabel Set the display label
** @nam4rule SetScore Set the score
**
** @argrule * sf [EnsPSimplefeature] Ensembl Simple Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSimplefeatureSetAdaptor *******************************************
**
** Set the Ensembl Simple Feature Adaptor element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] adaptor [EnsPSimplefeatureadaptor] Ensembl Simple
**                                               Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetAdaptor(EnsPSimplefeature sf,
                                  EnsPSimplefeatureadaptor adaptor)
{
    if(!sf)
        return ajFalse;
    
    sf->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensSimplefeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetIdentifier(EnsPSimplefeature sf, ajuint identifier)
{
    if(!sf)
        return ajFalse;
    
    sf->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensSimplefeatureSetFeature *******************************************
**
** Set the Ensembl Feature element of an Ensembl Simple Feature.
**
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetFeature(EnsPSimplefeature sf, EnsPFeature feature)
{
    if(!sf)
        return ajFalse;
    
    ensFeatureDel(&sf->Feature);
    
    sf->Feature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensSimplefeatureSetDisplayLabel **************************************
**
** Set the display label element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::display_label
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] label [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetDisplayLabel(EnsPSimplefeature sf, AjPStr label)
{
    if(!sf)
        return ajFalse;
    
    ajStrDel(&sf->DisplayLabel);
    
    sf->DisplayLabel = ajStrNewRef(label);
    
    return ajTrue;
}




/* @func ensSimplefeatureSetScore *********************************************
**
** Set the score element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::score
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetScore(EnsPSimplefeature sf, double score)
{
    if(!sf)
        return ajFalse;
    
    sf->Score = score;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
** @nam3rule Trace Report Ensembl Simple Feature elements to debug file
**
** @argrule Trace sf [const EnsPSimplefeature] Ensembl Simple Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSimplefeatureTrace ************************************************
**
** Trace an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureTrace(const EnsPSimplefeature sf, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!sf)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensSimplefeatureTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Feature %p\n"
	    "%S  DisplayLabel '%S'\n"
	    "%S  Score %f\n",
	    indent, sf,
	    indent, sf->Use,
	    indent, sf->Identifier,
	    indent, sf->Adaptor,
	    indent, sf->Feature,
	    indent, sf->DisplayLabel,
	    indent, sf->Score);
    
    ensFeatureTrace(sf->Feature, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensSimplefeatureGetMemSize *******************************************
**
** Get the memory size in bytes of an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensSimplefeatureGetMemSize(const EnsPSimplefeature sf)
{
    ajuint size = 0;
    
    if(!sf)
	return 0;
    
    size += (ajuint) sizeof (EnsOSimplefeature);
    
    size += ensFeatureGetMemSize(sf->Feature);
    
    if(sf->DisplayLabel)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(sf->DisplayLabel);
    }
    
    return size;
}




/* @datasection [EnsPSimplefeatureadaptor] Simple Feature Adaptor *************
**
** Functions for manipulating Ensembl Simple Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor CVS Revision: 1.33
**
** @nam2rule Simplefeatureadaptor
**
******************************************************************************/

static const char *simpleFeatureadaptorTables[] =
{
    "simple_feature",
    NULL
};




static const char *simpleFeatureadaptorColumns[] =
{
    "simple_feature.simple_feature_id",
    "simple_feature.seq_region_id",
    "simple_feature.seq_region_start",
    "simple_feature.seq_region_end",
    "simple_feature.seq_region_strand",
    "simple_feature.analysis_id",
    "simple_feature.label",
    "simple_feature.score",
    NULL
};





static EnsOBaseadaptorLeftJoin simpleFeatureadaptorLeftJoin[] =
{
    {NULL, NULL}
};





static const char *simpleFeatureadaptorDefaultCondition = NULL;

static const char *simpleFeatureadaptorFinalCondition = NULL;




/* @funcstatic simpleFeatureadaptorFetchAllBySQL ******************************
**
** Fetch all Ensembl Simple Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor::_objs_from_sth
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [r] mapper [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool simpleFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                const AjPStr statement,
                                                EnsPAssemblymapper mapper,
                                                EnsPSlice slice,
                                                AjPList sfs)
{
    double score = 0;
    
    ajint srstrand = 0;
    
    ajuint identifier = 0;
    ajuint analysisid = 0;
    
    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    
    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    
    AjPList mrs = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr label = NULL;
    
    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPCoordsystemadaptor csa = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPSimplefeature sf         = NULL;
    EnsPSimplefeatureadaptor sfa = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

#if AJFALSE
    ajDebug("simpleFeatureadaptorFetchAllBySQL\n"
	    "  dba %p\n"
	    "  statement %p\n"
	    "  mapper %p\n"
	    "  slice %p\n"
	    "  sfs %p\n",
	    dba,
	    statement,
	    mapper,
	    slice,
	    sfs);
#endif
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!sfs)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    sfa = ensRegistryGetSimplefeatureadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    if(slice)
	ama = ensRegistryGetAssemblymapperadaptor(dba);
    
    mrs = ajListNew();
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier = 0;
	srid       = 0;
	srstart    = 0;
	srend      = 0;
	srstrand   = 0;
	label      = ajStrNew();
	score      = 0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToUint(sqlr, &analysisid);
	ajSqlcolumnToStr(sqlr, &label);
        ajSqlcolumnToDouble(sqlr, &score);
	
	/* Need to get the internal Ensembl Sequence Region identifier. */
	
	srid = ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);
	
	/*
	** Since the Ensembl SQL schema defines Sequence Region start and end
	** coordinates as unsigned integers for all Features, the range needs
	** checking.
	*/
	
	if(srstart <= INT_MAX)
	    slstart = (ajint) srstart;
	else
	    ajFatal("simpleFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("simpleFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region end coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srend, INT_MAX);
	
	slstrand = srstrand;
	
	/* Fetch a Slice spanning the entire Sequence Region. */
	
	ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);
	
	/*
	** Obtain an Ensembl Assembly Mapper if none was defined, but a
	** destination Slice was.
	*/
	
	if(mapper)
	    am = ensAssemblymapperNewRef(mapper);
	
	if((! mapper) &&
	   slice &&
	   (! ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                                  ensSliceGetCoordsystem(srslice))))
	    am =
		ensAssemblymapperadaptorFetchByCoordsystems(
                    ama,
		    ensSliceGetCoordsystem(slice),
		    ensSliceGetCoordsystem(srslice));
	
	/*
	** Remap the Feature coordinates to another Ensembl Coordinate System
	** if an Ensembl Assembly Mapper was provided.
	*/
	
	if(am)
	{
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(srslice),
				     slstart,
				     slend,
				     slstrand,
				     mrs);
	    
	    /*
	    ** The ensAssemblymapperFastMap function returns at best one
	    ** Ensembl Mapper Result.
	    */
	    
	    ajListPop(mrs, (void **) &mr);
	    
	    /*
	    ** Skip Features that map to gaps or
	    ** Coordinate System boundaries.
	    */
	    
	    if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
	    {
		/* Load the next Feature but destroy first! */
		
		ajStrDel(&label);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		ensMapperresultDel(&mr);
		
		continue;
	    }
	    
	    srid = ensMapperresultGetObjectIdentifier(mr);
	    
	    slstart = ensMapperresultGetStart(mr);
	    
	    slend = ensMapperresultGetEnd(mr);
	    
	    slstrand = ensMapperresultGetStrand(mr);
	    
	    /*
	    ** Delete the Sequence Region Slice and fetch a Slice in the
	    ** Coordinate System we just mapped to.
	    */
	    
	    ensSliceDel(&srslice);
	    
	    ensSliceadaptorFetchBySeqregionIdentifier(sa,
						      srid,
						      0,
						      0,
						      0,
						      &srslice);
	    
	    ensMapperresultDel(&mr);
	}
	
	/*
	** Convert Sequence Region Slice coordinates to destination Slice
	** coordinates, if a destination Slice has been provided.
	*/
	
	if(slice)
	{
	    /* Check that the length of the Slice is within range. */
	    
	    if(ensSliceGetLength(slice) <= INT_MAX)
		sllength = (ajint) ensSliceGetLength(slice);
	    else
		ajFatal("simpleFeatureadaptorFetchAllBySQL got a Slice, "
			"which length (%u) exceeds the "
			"maximum integer limit (%d).",
			ensSliceGetLength(slice), INT_MAX);
	    
	    /*
	    ** Nothing needs to be done if the destination Slice starts at 1
	    ** and is on the forward strand.
	    */
	    
	    if((ensSliceGetStart(slice) != 1) ||
               (ensSliceGetStrand(slice) < 0))
	    {
		if(ensSliceGetStrand(slice) >= 0)
		{
		    slstart = slstart - ensSliceGetStart(slice) + 1;
		    
		    slend = slend - ensSliceGetStart(slice) + 1;
		}
		else
		{
		    slend = ensSliceGetEnd(slice) - slstart + 1;
		    
		    slstart = ensSliceGetEnd(slice) - slend + 1;
		    
		    slstrand *= -1;
		}
	    }
	    
	    /*
	    ** Throw away Features off the end of the requested Slice or on
	    ** any other than the requested Slice.
	    */
	    
	    if((slend < 1) ||
	       (slstart > sllength) ||
	       (srid != ensSliceGetSeqregionIdentifier(slice)))
	    {
		/* Load the next Feature but destroy first! */
		
		ajStrDel(&label);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		continue;
	    }
	    
	    /* Delete the Sequence Region Slice and set the requested Slice. */
	    
	    ensSliceDel(&srslice);
	    
	    srslice = ensSliceNewRef(slice);
	}
	
	ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);
	
	/* Finally, create a new Ensembl Simple Feature. */
	
	feature = ensFeatureNewS(analysis,
				 srslice,
				 slstart,
				 slend,
				 slstrand);
	
	sf = ensSimplefeatureNew(sfa, identifier, feature, label, score);
	
	ajListPushAppend(sfs, (void *) sf);
	
	ensFeatureDel(&feature);
	
	ensAnalysisDel(&analysis);
	
	ensAssemblymapperDel(&am);
	
	ensSliceDel(&srslice);
	
	ajStrDel(&label);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @funcstatic simpleFeatureadaptorCacheReference *****************************
**
** Wrapper function to reference an Ensembl Simple Feature
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Simple Feature
**
** @return [void *] Ensembl Simple Feature or NULL
** @@
******************************************************************************/

static void *simpleFeatureadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensSimplefeatureNewRef((EnsPSimplefeature) value);
}




/* @funcstatic simpleFeatureadaptorCacheDelete ********************************
**
** Wrapper function to delete an Ensembl Simple Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Simple Feature address
**
** @return [void]
** @@
******************************************************************************/

static void simpleFeatureadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensSimplefeatureDel((EnsPSimplefeature *) value);
    
    return;
}




/* @funcstatic simpleFeatureadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Simple Feature
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Simple Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

static ajuint simpleFeatureadaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensSimplefeatureGetMemSize((const EnsPSimplefeature) value);
}




/* @funcstatic simpleFeatureadaptorGetFeature *********************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Simple Feature
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Simple Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature simpleFeatureadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensSimplefeatureGetFeature((const EnsPSimplefeature) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Simple Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Simple Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSimplefeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPSimplefeatureadaptor] Ensembl Simple
**                                                Feature Adaptor
** @argrule Ref object [EnsPSimplefeatureadaptor] Ensembl Simple
**                                                Feature Adaptor
**
** @valrule * [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensSimplefeatureadaptorNew *******************************************
**
** Default Ensembl Simple Feature Adaptor constructor.
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensSimplefeatureadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPSimplefeatureadaptor sfa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(sfa);
    
    sfa->Adaptor = ensFeatureadaptorNew(
        dba,
        simpleFeatureadaptorTables,
        simpleFeatureadaptorColumns,
        simpleFeatureadaptorLeftJoin,
        simpleFeatureadaptorDefaultCondition,
        simpleFeatureadaptorFinalCondition,
        simpleFeatureadaptorFetchAllBySQL,
        (void* (*)(const void* key)) NULL, /* Fread */
        simpleFeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL, /* Fwrite */
        simpleFeatureadaptorCacheDelete,
        simpleFeatureadaptorCacheSize,
        simpleFeatureadaptorGetFeature,
        "Simple Feature");
    
    return sfa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Simple Feature Adaptor.
**
** @fdata [EnsPSimplefeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Simple Feature Adaptor object
**
** @argrule * Padaptor [EnsPSimplefeatureadaptor*] Ensembl Simple Feature
**                                                 Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSimplefeatureadaptorDel *******************************************
**
** Default destructor for an Ensembl Simple Feature Adaptor.
**
** @param [d] Padaptor [EnsPSimplefeatureadaptor*] Ensembl Simple Feature
**                                                 Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensSimplefeatureadaptorDel(EnsPSimplefeatureadaptor *Padaptor)
{
    EnsPSimplefeatureadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    ensFeatureadaptorDel(&pthis->Adaptor);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @func ensSimplefeatureadaptorFetchAllBySlice *******************************
**
** Fetch all Ensembl Simple Features on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [r] adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature
**                                               Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] sfs [AjPList] AJAX List of Ensembl Simple Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllBySlice(EnsPSimplefeatureadaptor adaptor,
                                              EnsPSlice slice,
                                              const AjPStr anname,
                                              AjPList sfs)
{
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!sfs)
	return ajFalse;
    
    return ensFeatureadaptorFetchAllBySlice(adaptor->Adaptor,
					    slice,
					    anname,
					    sfs);
}




/* @func ensSimplefeatureadaptorFetchAllBySliceScore **************************
**
** Fetch all Ensembl Simple Features on an Ensembl Slice
** above a threshold score.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [r] adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature
**                                               Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] score [double] Score
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] sfs [AjPList] AJAX List of Ensembl Simple Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllBySliceScore(
    EnsPSimplefeatureadaptor adaptor,
    EnsPSlice slice,
    double score,
    const AjPStr anname,
    AjPList sfs)
{
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!sfs)
	return ajFalse;
    
    return ensFeatureadaptorFetchAllBySliceScore(adaptor->Adaptor,
						 slice,
						 score,
						 anname,
						 sfs);
}




/* @func ensSimplefeatureadaptorFetchAllByAnalysisName ************************
**
** Fetch all Ensembl Simple Features via an Ensembl Analysis name.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_logic_name
** @param [r] adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature
**                                               Adaptor
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] sfs [AjPList] AJAX List of Ensembl Simple Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllByAnalysisName(
    EnsPSimplefeatureadaptor adaptor,
    const AjPStr anname,
    AjPList sfs)
{
    if(!adaptor)
	return ajFalse;
    
    if(!anname)
	return ajFalse;
    
    if(!sfs)
	return ajFalse;
    
    return ensFeatureadaptorFetchAllByAnalysisName(adaptor->Adaptor,
						   anname,
						   sfs);
}




/* @datasection [EnsPAssemblyexceptionfeature] Assembly Exception Feature *****
**
** Functions for manipulating Ensembl Assembly Exception Feature objects
**
** @cc Bio::EnsEMBL::AssemblyExceptionFeature CVS Revision: 1.4
**
** @nam2rule Assemblyexceptionfeature
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                    Exception Feature
** @argrule Ref object [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                    Exception Feature
**
** @valrule * [EnsPAssemblyexceptionfeature] Ensembl Assembly Exception Feature
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyexceptionfeatureNew ***************************************
**
** Default Ensembl Assembly Exception Feature constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::AssemblyExceptionFeature::new
** @param [u] slice [EnsPSlice] Alternative Slice
** @param [r] type [AjEnum] Assembly Exception Feature type
**
** @return [EnsPAssemblyexceptionfeature] Ensembl Assembly Exception Feature
**                                        or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNew(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature,
    EnsPSlice slice,
    AjEnum type)
{
    EnsPAssemblyexceptionfeature aef = NULL;
    
    if(!slice)
	return NULL;
    
    if(!type)
	return NULL;
    
    AJNEW0(aef);
    
    aef->Use = 1;
    
    aef->Identifier     = identifier;
    aef->Adaptor        = adaptor;
    aef->Feature        = feature;
    aef->AlternateSlice = ensSliceNewRef(slice);
    aef->Type           = type;
    
    return aef;
}




/* @func ensAssemblyexceptionfeatureNewObj ************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                        Exception Feature
**
** @return [EnsPAssemblyexceptionfeature] Ensembl Assembly Exception Feature
**                                        or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewObj(
    const EnsPAssemblyexceptionfeature object)
{
    EnsPAssemblyexceptionfeature aef = NULL;
    
    AJNEW0(aef);
    
    aef->Use            = 1;
    aef->Identifier     = object->Identifier;
    aef->Adaptor        = object->Adaptor;
    aef->Feature        = ensFeatureNewRef(object->Feature);
    aef->AlternateSlice = ensSliceNewRef(object->AlternateSlice);
    aef->Type           = object->Type;
    
    return aef;
}




/* @func ensAssemblyexceptionfeatureNewRef ************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] aef [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                               Exception Feature
**
** @return [EnsPAssemblyexceptionfeature] Ensembl Assembly Exception Feature
**                                        or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewRef(
    EnsPAssemblyexceptionfeature aef)
{
    if(!aef)
	return NULL;
    
    aef->Use++;
    
    return aef;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Assembly Exception Feature.
**
** @fdata [EnsPAssemblyexceptionfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Feature object
**
** @argrule * Paef [EnsPAssemblyexceptionfeature*] Assembly Exception Feature
**                                                 object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblyexceptionfeatureDel ***************************************
**
** Default destructor for an Ensembl Assembly Exception Feature.
**
** @param [d] Paef [EnsPAssemblyexceptionfeature*] Ensembl Assembly
**                                                 Exception Feature address
**
** @return [void]
** @@
******************************************************************************/

void ensAssemblyexceptionfeatureDel(EnsPAssemblyexceptionfeature *Paef)
{
    EnsPAssemblyexceptionfeature pthis = NULL;
    
    if(!Paef)
	return;
    
    if(!*Paef)
	return;

    pthis = *Paef;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Paef = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    ensSliceDel(&pthis->AlternateSlice);
    
    AJFREE(pthis);

    *Paef = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
** @fnote None
**
** @nam3rule Get Return Assembly Exception Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Assembly Exception Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetAlternateSlice Return the alternate Ensembl Slice
** @nam4rule GetType Return the Assembly Exception type
**
** @argrule * aef [const EnsPAssemblyexceptionfeature] Assembly Exception
**                                                     Feature
**
** @valrule Adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                        Exception Feature
**                                                        Adaptor
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule AlternatSlice [EnsPSlice] Alternative Ensembl Slice
** @valrule Type [AjEnum] Assembly Exception Feature type
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionfeatureGetAdaptor ********************************
**
** Get the Ensembl Assembly Exception Feature Adaptor element of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
**
** @return [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                               Exception Feature Adaptor
** @@
******************************************************************************/

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureGetAdaptor(
    const EnsPAssemblyexceptionfeature aef)
{
    if(!aef)
	return NULL;
    
    return aef->Adaptor;
}




/* @func ensAssemblyexceptionfeatureGetIdentifier *****************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensAssemblyexceptionfeatureGetIdentifier(
    const EnsPAssemblyexceptionfeature aef)
{
    if(!aef)
	return 0;
    
    return aef->Identifier;
}




/* @func ensAssemblyexceptionfeatureGetFeature ********************************
**
** Get the Ensembl Feature element of an Ensembl Assembly Exception Feature.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensAssemblyexceptionfeatureGetFeature(
    const EnsPAssemblyexceptionfeature aef)
{
    if(!aef)
	return NULL;
    
    return aef->Feature;
}




/* @func ensAssemblyexceptionfeatureGetAlternateSlice *************************
**
** Get the alternate Ensembl Slice element of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::AssemblyExceptionFeature::alternate_slice
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
**
** @return [EnsPSlice] Alternate Ensembl Slice
** @@
******************************************************************************/

EnsPSlice ensAssemblyexceptionfeatureGetAlternateSlice(
    const EnsPAssemblyexceptionfeature aef)
{
    if(!aef)
	return NULL;
    
    return aef->AlternateSlice;
}




/* @func ensAssemblyexceptionfeatureGetType ***********************************
**
** Get the Ensembl Assembly Exception Feature type element of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::AssemblyExceptionFeature::type
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
**
** @return [AjEnum] Ensembl Assembly Exception Feature type
** @@
******************************************************************************/

AjEnum ensAssemblyexceptionfeatureGetType(
    const EnsPAssemblyexceptionfeature aef)
{
    if(!aef)
	return 0;
    
    return aef->Type;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
** @fnote None
**
** @nam3rule Set Set one element of an Assembly Exception Feature
** @nam4rule SetAdaptor Set the Ensembl Assembly Exception Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetAlternatSlice Set the alternate Ensembl Slice
** @nam4rule SetType Set the type
**
** @argrule * aef [EnsPAssemblyexceptionfeature] Ensembl Assembly Exception
**                                               Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensAssemblyexceptionfeatureSetAdaptor ********************************
**
** Set the Ensembl Assembly Exception Feature Adaptor element of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] aef [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                               Exception Feature
** @param [u] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetAdaptor(
    EnsPAssemblyexceptionfeature aef,
    EnsPAssemblyexceptionfeatureadaptor adaptor)
{
    if(!aef)
	return ajFalse;
    
    aef->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetIdentifier *****************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Assembly Exception Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] aef [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                               Exception Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetIdentifier(
    EnsPAssemblyexceptionfeature aef,
    ajuint identifier)
{
    if(!aef)
	return ajFalse;
    
    aef->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetFeature ********************************
**
** Set the Ensembl Feature element of an Ensembl Assembly Exception Feature.
**
** @param [u] aef [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                               Exception Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetFeature(
    EnsPAssemblyexceptionfeature aef,
    EnsPFeature feature)
{
    if(!aef)
	return ajFalse;
    
    ensFeatureDel(&aef->Feature);
    
    aef->Feature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetAlternateSlice *************************
**
** Set the alternate Ensembl Slice element of an
** Ensembl Assembly Exception Feature.
**
** @param [u] aef [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                               Exception Feature
** @param [u] altslice [EnsPSlice] Alternate Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetAlternateSlice(
    EnsPAssemblyexceptionfeature aef,
    EnsPSlice altslice)
{
    if(!aef)
	return ajFalse;
    
    ensSliceDel(&aef->AlternateSlice);
    
    aef->AlternateSlice = ensSliceNewRef(altslice);
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureSetType ***********************************
**
** Set the Ensembl Assembly Exception Feature type element of an
** Ensembl Assembly Exception Feature.
**
** @param [u] aef [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                               Exception Feature
** @param [r] type [AjEnum] Ensembl Assembly Exception Feature type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureSetType(EnsPAssemblyexceptionfeature aef,
                                          AjEnum type)
{
    if(!aef)
	return ajFalse;
    
    aef->Type = type;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Assembly Exception Feature object.
**
** @fdata [EnsPAssemblyexceptionfeature]
** @nam3rule Trace Report Ensembl Assembly Exception Feature elements to
**                 debug file
**
** @argrule Trace aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                         Exception Feature
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
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureTrace(const EnsPAssemblyexceptionfeature aef,
                                        ajuint level)
{
    AjPStr indent = NULL;
    
    if(!aef)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("ensAssemblyexceptionfeatureTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  EnsPAssemblyexceptionfeatureadaptor %p\n"
	    "%S  Feature %p\n"
	    "%S  AlternateSlice %p\n"
	    "%S  Type %d\n",
	    indent, aef,
	    indent, aef->Use,
	    indent, aef->Identifier,
	    indent, aef->Adaptor,
	    indent, aef->Feature,
	    indent, aef->AlternateSlice,
	    indent, aef->Type);
    
    ensFeatureTrace(aef->Feature, level + 1);
    
    ensSliceTrace(aef->AlternateSlice, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureFetchDisplayIdentifier ********************
**
** Fetch the display identifier of an Ensembl Assembly Exception Feature.
**
** @param [r] aef [const EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                     Exception Feature
** @param [wP] Pidentifier [AjPStr*] Display identifier String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureFetchDisplayIdentifier(
    const EnsPAssemblyexceptionfeature aef,
    AjPStr *Pidentifier)
{
    if(!aef)
	return ajFalse;
    
    if(!Pidentifier)
	return ajFalse;
    
    if(!aef->AlternateSlice)
	return ajFalse;
    
    ajStrAssignS(Pidentifier, ensSliceGetSeqregionName(aef->AlternateSlice));
    
    return ajTrue;
}




/* @datasection [EnsPAssemblyexceptionfeatureadaptor] Assembly Exception Feature Adaptor
**
** Functions for manipulating Ensembl Assembly Exception Feature Adaptor
** objects
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyExceptionFeatureAdaptor CVS Revision: 1.14
**
** @nam2rule Assemblyexceptionfeatureadaptor
**
******************************************************************************/

static ajuint assemblyExceptionFeatureadaptorCacheMaxBytes = 1 << 26;

static ajuint assemblyExceptionFeatureadaptorCacheMaxCount = 1 << 16;

static ajuint assemblyExceptionFeatureadaptorCacheMaxSize = 0;




/* @funcstatic assemblyExceptionFeatureadaptorCacheInit ***********************
**
** Initialise an Ensembl Assembly Exception Feature Adaptor-internal
** Ensembl Assembly Exception Feature cache.
**
** @param [u] aefa [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                       Exception Feature
**                                                       Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyExceptionFeatureadaptorCacheInit(
    EnsPAssemblyexceptionfeatureadaptor aefa)
{
    ajuint erid    = 0;
    ajuint srid    = 0;
    ajuint erstart = 0;
    ajuint srstart = 0;
    ajuint erend   = 0;
    ajuint srend   = 0;
    
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
    
    if(!aefa)
	return ajFalse;
    
    if(!aefa->CacheByIdentifier)
    {
	ajDebug("assemblyExceptionFeatureadaptorCacheInit CacheByIdentifier "
		"not initialised!\n");
	
	return ajFalse;
    }
    
    aea = ensRegistryGetAssemblyexceptionadaptor(aefa->Adaptor);
    
    sa = ensRegistryGetSliceadaptor(aefa->Adaptor);
    
    aes = ajListNew();
    
    ensAssemblyexceptionadaptorFetchAll(aea, aes);
    
    while(ajListPop(aes, (void **) &ae))
    {
	srid = ensAssemblyexceptionGetSeqregionIdentifier(ae);
	
	srstart = ensAssemblyexceptionGetSeqregionStart(ae);
	
	srend = ensAssemblyexceptionGetSeqregionEnd(ae);
	
	erid = ensAssemblyexceptionGetExcRegionIdentifier(ae);
	
	erstart = ensAssemblyexceptionGetExcRegionStart(ae);
	
	erend = ensAssemblyexceptionGetExcRegionEnd(ae);
	
	/*
	** Each Ensembl Assembly Exception creates two
	** Ensembl Assembly Exception Features, each of which has
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
	
	feature = ensFeatureNewS((EnsPAnalysis) NULL,
				 refslice,
				 srstart,
				 srend,
				 1);
	
	refaef = ensAssemblyexceptionfeatureNew(
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
	
	feature = ensFeatureNewS((EnsPAnalysis) NULL,
				 excslice,
				 erstart,
				 erend,
				 1);
	
	excaef = ensAssemblyexceptionfeatureNew(
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
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                    Exception Feature Adaptor
** @argrule Ref object [EnsPAssemblyexceptionfeature] Ensembl Assembly
**                                                    Exception Feature Adaptor
**
**
** @valrule * [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                  Exception Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyexceptionfeatureadaptorNew ********************************
**
** Default Ensembl Assembly Exception Feature Adaptor constructor.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyExceptionFeatureAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly Exception
**                                               Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAssemblyexceptionfeatureadaptor aefa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(aefa);
    
    aefa->Adaptor = dba;
    
    aefa->Cache = ajListNew();
    
    aefa->CacheByIdentifier =
	ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);
    
    aefa->CacheBySlice = ensCacheNew(
        ensECacheTypeAlphaNumeric,
        assemblyExceptionFeatureadaptorCacheMaxBytes,
        assemblyExceptionFeatureadaptorCacheMaxCount,
        assemblyExceptionFeatureadaptorCacheMaxSize,
        (void* (*)(void* value)) NULL, /* Freference */
        (void (*)(void** value)) NULL, /* Fdelete */
        (ajuint (*)(const void* value)) NULL, /* Fsize */
        (void* (*)(const void* key)) NULL, /* Fread */
        (AjBool (*)(const void* value)) NULL, /* Fwrite */
        ajFalse,
        "Assembly Exception Feature");
    
    assemblyExceptionFeatureadaptorCacheInit(aefa);
    
    return aefa;
}




/* @funcstatic assemblyExceptionFeatureadaptorCacheClear **********************
**
** Clear an Ensembl Assembly Exception Feature Adaptor-internal
** Ensembl Assembly Exception Feature cache.
**
** @param [u] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyExceptionFeatureadaptorCacheClear(
    EnsPAssemblyexceptionfeatureadaptor adaptor)
{
    void **keyarray = NULL;
    void **valarray = NULL;
    
    register ajuint i = 0;
    
    EnsPAssemblyexceptionfeature aef = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    while(ajListPop(adaptor->Cache, (void **) &aef))
	ensAssemblyexceptionfeatureDel(&aef);
    
    ajTableToarrayKeysValues(adaptor->CacheByIdentifier, &keyarray, &valarray);
    
    for(i = 0; keyarray[i]; i++)
    {
	ajTableRemove(adaptor->CacheByIdentifier, (const void *) keyarray[i]);
	
	/* Delete unsigned integer key data. */
	
	AJFREE(keyarray[i]);
	
	/* Delete the Ensembl Assembly Exception Feature. */
	
	ensAssemblyexceptionfeatureDel((EnsPAssemblyexceptionfeature *)
				       &valarray[i]);
    }
    
    AJFREE(keyarray);
    AJFREE(valarray);
    
    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Assembly Exception Feature Adaptor.
**
** @fdata [EnsPAssemblyexceptionfeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) a Feature object
**
** @argrule * Padaptor [EnsPAssemblyexceptionfeatureadaptor*] Assembly Exception
**                                                            Feature Adaptor
**                                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblyexceptionfeatureadaptorDel ********************************
**
** Default destructor for an Ensembl Assembly Exception Feature Adaptor.
**
** @param [d] Padaptor [EnsPAssemblyexceptionfeatureadaptor*] Ensembl Assembly
**                                                            Exception Feature
**                                                            Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensAssemblyexceptionfeatureadaptorDel(
    EnsPAssemblyexceptionfeatureadaptor *Padaptor)
{
    EnsPAssemblyexceptionfeatureadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    assemblyExceptionFeatureadaptorCacheClear(pthis);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Assembly Exception Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPAssemblyexceptionfeatureadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Assembly Exception Feature object(s)
** @nam4rule FetchAll Retrieve all Ensembl Assembly Exception Feature objects
** @nam5rule FetchAllBy Retrieve all Ensembl Assembly Exception Feature objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Assembly Exception Feature object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPAssemblyexceptionfeatureadaptor] Ensembl
**                     Assembly Exception Feature Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Assembly Exception Feature
**                             objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionfeatureadaptorFetchAll ***************************
**
** Fetch all Ensembl Assembly Exception Features.
**
** @param [r] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
** @param [u] aefs [AjPList] AJAX List of Ensembl Assembly Exception Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureadaptorFetchAll(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    AjPList aefs)
{
    AjIList iter = NULL;
    
    EnsPAssemblyexceptionfeature aef = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!aefs)
	return ajFalse;
    
    iter = ajListIterNewread(adaptor->Cache);
    
    while(!ajListIterDone(iter))
    {
	aef = (EnsPAssemblyexceptionfeature) ajListIterGet(iter);
	
	ajListPushAppend(aefs,
			 (void *) ensAssemblyexceptionfeatureNewRef(aef));
    }
    
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureadaptorFetchByIdentifier ******************
**
** Fetch all Ensembl Assembly Exception Features.
**
** @param [r] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Paef [EnsPAssemblyexceptionfeature*] Ensembl Assembly Exception
**                                                  Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureadaptorFetchByIdentifier(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    ajuint identifier,
    EnsPAssemblyexceptionfeature* Paef)
{
    if(!adaptor)
	return ajFalse;
    
    *Paef = (EnsPAssemblyexceptionfeature)
	ajTableFetch(adaptor->CacheByIdentifier, (const void *) &identifier);
    
    ensAssemblyexceptionfeatureNewRef(*Paef);
    
    return ajTrue;
}




/* @funcstatic assemblyExceptionFeatureadaptorRemap ***************************
**
** Remap Ensembl Assembly Exception Features onto an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyExceptionFeatureAdaptor::_remap
** @param [r] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
** @param [r] aefs [AjPList] AJAX List of Ensembl Assembly Exception Features
** @param [r] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyExceptionFeatureadaptorRemap(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    AjPList aefs,
    EnsPAssemblymapper am,
    EnsPSlice slice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;
    
    ajuint srcsrid = 0;
    ajuint trgsrid = 0;
    
    AjIList iter = NULL;
    AjPList mrs  = NULL;
    
    EnsPAssemblyexceptionfeature aef = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPMapperresult mr = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!aefs)
	return ajFalse;
    
    /*
      if (! am)
      return ajFalse;
    */
    
    if(!slice)
	return ajFalse;
    
    /* Check if remapping is actually required. */
    
    /*
    ** If the List contains no Assembly Exception Features, remapping is
    ** not required.
    */
    
    if(!ajListGetLength(aefs))
	return ajTrue;
    
    /*
    ** If the Slice attached to the first Feature is identical to the Slice
    ** the Features should be mapped to, remapping is not required.
    */
    
    ajListPeekFirst(aefs, (void **) &aef);
    
    feature = aef->Feature;
    
    if(ensSliceMatch(ensFeatureGetSlice(feature), slice))
	return ajTrue;
    
    /* Remapping has not been done, we have to do our own conversion. */
    
    trgsrid = ensSliceGetSeqregionIdentifier(slice);
    
    iter = ajListIterNew(aefs);
    
    while(!ajListIterDone(iter))
    {
	aef = (EnsPAssemblyexceptionfeature) ajListIterGet(iter);
	
	feature = aef->Feature;
	
	/*
	** Since Features were obtained in contig coordinates, the attached
	** Sequence Region is a contig.
	*/
	
	if(!feature->Slice)
	    ajFatal("assemblyExceptionFeatureadaptorRemap Feature (%p) "
		    "does not have a Slice attached.\n", feature);
	
	if(ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                               ensSliceGetCoordsystem(feature->Slice)))
	{
	    /*
	    ** The Slice attached to the Feature is in the same
	    ** Coordinate System as the target Slice, therefore remapping and
	    ** an Ensembl Assembly Mapper are not required. Nevertheless,
	    ** coordinates need still adjusting to the Slice.
	    */
	    
	    start = feature->Start;
	    
	    end = feature->End;
	    
	    strand = feature->Strand;
	    
	    srcsrid = ensSliceGetSeqregionIdentifier(feature->Slice);
	}
	else
	{
	    /*
	    ** The Slice attached to the Feature is in a different
	    ** Coordinate System, therefore remapping is required.
	    */
	    
	    if(!am)
		ajFatal("featureAdaptorRemap requires an "
			"Ensembl Assembly Mapper, when "
			"Coordinate Systems of Features and Slice differ.\n");
	    
	    mrs = ajListNew();
	    
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(feature->Slice),
				     feature->Start,
				     feature->End,
				     feature->Strand,
				     mrs);
	    
	    /*
	    ** For successful mapping, the ensMapperFastMap function returns
	    ** at maximum one Mapper Result List element.
	    ** An empty List means a Gap so remove the Ensembl Object from
	    ** the List and de-reference (delete) it.
	    */
	    
	    if(ajListGetLength(mrs))
		ajListPop(mrs, (void **) &mr);
	    else
	    {
		ajListIterRemove(iter);
		
		ensAssemblyexceptionfeatureDel(&aef);
		
		continue;
	    }
	    
	    start = ensMapperresultGetStart(mr);
	    
	    end = ensMapperresultGetEnd(mr);
	    
	    strand = ensMapperresultGetStrand(mr);
	    
	    srcsrid = ensMapperresultGetObjectIdentifier(mr);
	    
	    ensMapperresultDel(&mr);
	    
	    ajListFree(&mrs);
	}
	
	if((srcsrid != trgsrid) ||
	   (start > ensSliceGetEnd(slice)) ||
	   (end < ensSliceGetStart(slice)))
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
	** FIXME: In constrast to the Perl API the Assembly Exception Feature
	** Adaptor does currently not use a Slice cache. While the Perl API
	** keeps Features on Slices spanning the entire Sequence Region, a new
	** copy of the Feature needs to be placed on the requested Slice.
	** Since we are currently not using a Slice cache, the Feature can be
	** remapped in place.
	** This means in fact that this function is now again identical to the
	** ensFeatureadaptorRemap function.
	*/
	
	/* Shift the Feature start, end and strand in one call. */
	
	if(ensSliceGetStrand(slice) > 0)
	    ensFeatureMove(feature,
			   start - ensSliceGetStart(slice) + 1,
			   end - ensSliceGetStart(slice) + 1,
			   strand);
	else
	    ensFeatureMove(feature,
			   ensSliceGetEnd(slice) - end + 1,
			   ensSliceGetEnd(slice) - start + 1,
			   strand * -1);
	
	ensFeatureSetSlice(feature, slice);
    }
    
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @func ensAssemblyexceptionfeatureadaptorFetchAllBySlice ********************
**
** Fetch all Ensembl Assembly Exception Features via an Ensembl Slice.
**
** @param [r] adaptor [EnsPAssemblyexceptionfeatureadaptor] Ensembl Assembly
**                                                          Exception Feature
**                                                          Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] aefs [AjPList] AJAX List of Ensembl Assembly Exception Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionfeatureadaptorFetchAllBySlice(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    EnsPSlice slice,
    AjPList aefs)
{
    AjPList cslist = NULL;
    
    AjPStr name = NULL;
    
    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPCoordsystem mcs = NULL;
    EnsPCoordsystem scs = NULL;
    
    EnsPMetacoordinateadaptor mca = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!aefs)
	return ajFalse;
    
    /* Return Features from the Slice cache if present. */
    
    /*
    ** FIXME: The Perl API cache uses the Slice name as the key.
    ** If Features are requested for a Slice that is based on the same
    ** Sequence Region, but has differnt start end coordinates the Feature
    ** cache does not work, as the Slice name would be differnt.
    ** Therefore, the same set of Features could be cached under different
    ** Slice names.
    ** Wouldn't it be better to use the Sequence Region identifier as the
    ** cache key and do the re-mapping from there?
    */
    
    /* TODO: Implement Slice cache.
       my $key= uc($slice->name());
    */
    
    /* TODO: Implement Slice cache.
       if(exists($self->{'_aexc_slice_cache'}->{$key})) {
       return $self->{'_aexc_slice_cache'}->{$key};
       }
    */
    
    ensAssemblyexceptionfeatureadaptorFetchAll(adaptor, aefs);
    
    mca = ensRegistryGetMetacoordinateadaptor(adaptor->Adaptor);
    
    name = ajStrNewC("assembly_exception");
    
    cslist = ajListNew();
    
    ensMetacoordinateadaptorFetchAllCoordsystems(mca, name, cslist);
    
    ama = ensRegistryGetAssemblymapperadaptor(adaptor->Adaptor);
    
    scs = ensSliceGetCoordsystem(slice);
    
    while(ajListPop(cslist, (void **) &mcs))
    {
	if(ensCoordsystemMatch(mcs, scs))
	    am = NULL;
	else
	    am = ensAssemblymapperadaptorFetchByCoordsystems(ama, mcs, scs);
	
	assemblyExceptionFeatureadaptorRemap(adaptor, aefs, am, slice);
	
	ensCoordsystemDel(&mcs);
    }
    
    /* TODO: Implement Slice cache.
       $self->{'_aexc_slice_cache'}->{$key} =\@ features;
    */
    
    ajListFree(&cslist);
    
    ajStrDel(&name);
    
    return ajTrue;
}

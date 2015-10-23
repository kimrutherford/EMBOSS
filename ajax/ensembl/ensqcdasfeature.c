/* @source ensqcdasfeature ****************************************************
**
** Ensembl Quality Check DAS Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.19 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:11 $ by $Author: mks $
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

#include "ensqcdasfeature.h"




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

/* @conststatic qcdasfeatureKCategory *****************************************
**
** The Ensembl Quality Check DAS Feature category member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcdasfeatureCategory.
**
******************************************************************************/

static const char *qcdasfeatureKCategory[] =
{
    "",
    "unknown",
    "transcript:perfect",
    "transcript:tolerance",
    "transcript:partial'",
    "transcript:missing",
    "transcript",
    "translation:perfect",
    "translation:tolerance",
    "translation:partial",
    "translation:missing",
    "translation",
    (const char *) NULL
};




/* @conststatic qcdasfeatureKType *********************************************
**
** The Ensembl Quality Check DAS Feature type member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcdasfeatureType.
**
******************************************************************************/

static const char *qcdasfeatureKType[] =
{
    "",
    "unknown",
    "exon:perfect",
    "exon:partial",
    "exon:missing",
    "exon:frameshift",
    "exon:gap",
    "exon",
    (const char *) NULL
};




/* @conststatic qcdasfeatureadaptorKTablenames ********************************
**
** Array of Ensembl Quality Check DAS Feature Adaptor SQL table names
**
******************************************************************************/

static const char *qcdasfeatureadaptorKTablenames[] =
{
    "das_feature",
    (const char *) NULL
};




/* @conststatic qcdasfeatureadaptorKColumnnames *******************************
**
** Array of Ensembl Quality Check DAS Feature Adaptor SQL column names
**
******************************************************************************/

static const char *qcdasfeatureadaptorKColumnnames[] =
{
    "das_feature.das_feature_id",
    "das_feature.analysis_id",
    "das_feature.alignment_id",
    "das_feature.feature_id",
    "das_feature.feature_start",
    "das_feature.feature_end",
    "das_feature.segment_id",
    "das_feature.segment_start",
    "das_feature.segment_end",
    "das_feature.segment_strand",
    "das_feature.phase",
    "das_feature.category",
    "das_feature.type",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool qcdasfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcdasfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensqc *********************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcdasfeature] Ensembl Quality Check DAS Feature **********
**
** @nam2rule Qcdasfeature Functions for manipulating
** Ensembl Quality Check (QC) Distributed Annotation System (DAS) Feature
** objects
**
** @cc Bio::EnsEMBL::QC::DASFeature
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check DAS Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Quality Check DAS Feature. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcdasfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy qcdasf [const EnsPQcdasfeature]
** Ensembl Quality Check DAS Feature
** @argrule Ini qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini segment [EnsPQcsequence] Segment Ensembl Quality Check Sequence
** @argrule Ini segstart [ajuint] Segment start
** @argrule Ini segend [ajuint] Segment end
** @argrule Ini segstrand [ajint] Segment strand
** @argrule Ini feature [EnsPQcsequence] Feature Ensembl Quality Check Sequence
** @argrule Ini fstart [ajuint] Feature start
** @argrule Ini fend [ajuint] Feature end
** @argrule Ini phase [ajint] Phase
** @argrule Ini category [EnsEQcdasfeatureCategory] Category
** @argrule Ini type [EnsEQcdasfeatureType] Type
** @argrule Ref qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @valrule * [EnsPQcdasfeature] Ensembl Quality Check DAS Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcdasfeatureNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPQcdasfeature] Ensembl Quality Check DAS Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcdasfeature ensQcdasfeatureNewCpy(const EnsPQcdasfeature qcdasf)
{
    EnsPQcdasfeature pthis = NULL;

    if (!qcdasf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = qcdasf->Identifier;
    pthis->Adaptor    = qcdasf->Adaptor;

    if (qcdasf->Qcalignment)
        pthis->Qcalignment = ensQcalignmentNewRef(qcdasf->Qcalignment);

    if (qcdasf->Analysis)
        pthis->Analysis = ensAnalysisNewRef(qcdasf->Analysis);

    if (qcdasf->SegmentSequence)
        pthis->SegmentSequence = ensQcsequenceNewRef(qcdasf->SegmentSequence);

    if (qcdasf->FeatureSequence)
        pthis->FeatureSequence = ensQcsequenceNewRef(qcdasf->FeatureSequence);

    pthis->SegmentStart  = qcdasf->SegmentStart;
    pthis->SegmentEnd    = qcdasf->SegmentEnd;
    pthis->SegmentStrand = qcdasf->SegmentStrand;
    pthis->FeatureStart  = qcdasf->FeatureStart;
    pthis->FeatureEnd    = qcdasf->FeatureEnd;
    pthis->Phase         = qcdasf->Phase;
    pthis->Category      = qcdasf->Category;
    pthis->Type          = qcdasf->Type;

    return pthis;
}




/* @func ensQcdasfeatureNewIni ************************************************
**
** Constructor for an Ensembl Quality Check DAS Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::DASFeature::new
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] segment [EnsPQcsequence] Segment Ensembl Quality Check Sequence
** @param [r] segstart [ajuint] Segment start
** @param [r] segend [ajuint] Segment end
** @param [r] segstrand [ajint] Segment strand
** @param [u] feature [EnsPQcsequence] Feature Ensembl Quality Check Sequence
** @param [r] fstart [ajuint] Feature start
** @param [r] fend [ajuint] Feature end
** @param [r] phase [ajint] Phase
** @param [u] category [EnsEQcdasfeatureCategory] Category
** @param [u] type [EnsEQcdasfeatureType] Type
**
** @return [EnsPQcdasfeature] Ensembl Quality Check DAS Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcdasfeature ensQcdasfeatureNewIni(EnsPQcdasfeatureadaptor qcdasfa,
                                       ajuint identifier,
                                       EnsPQcalignment qca,
                                       EnsPAnalysis analysis,
                                       EnsPQcsequence segment,
                                       ajuint segstart,
                                       ajuint segend,
                                       ajint segstrand,
                                       EnsPQcsequence feature,
                                       ajuint fstart,
                                       ajuint fend,
                                       ajint phase,
                                       EnsEQcdasfeatureCategory category,
                                       EnsEQcdasfeatureType type)
{
    EnsPQcdasfeature qcdasf = NULL;

    if (!qca)
        return NULL;

    if (!analysis)
        return NULL;

    if (!segment)
        return NULL;

    if (!feature)
        return NULL;

    AJNEW0(qcdasf);

    qcdasf->Use             = 1U;
    qcdasf->Identifier      = identifier;
    qcdasf->Adaptor         = qcdasfa;
    qcdasf->Qcalignment     = ensQcalignmentNewRef(qca);
    qcdasf->Analysis        = ensAnalysisNewRef(analysis);
    qcdasf->SegmentSequence = ensQcsequenceNewRef(segment);
    qcdasf->SegmentStart    = segstart;
    qcdasf->SegmentEnd      = segend;
    qcdasf->SegmentStrand   = segstrand;
    qcdasf->FeatureSequence = ensQcsequenceNewRef(feature);
    qcdasf->FeatureStart    = fstart;
    qcdasf->FeatureEnd      = fend;
    qcdasf->Phase           = phase;
    qcdasf->Category        = category;
    qcdasf->Type            = type;

    return qcdasf;
}




/* @func ensQcdasfeatureNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPQcdasfeature] Ensembl Quality Check DAS Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcdasfeature ensQcdasfeatureNewRef(EnsPQcdasfeature qcdasf)
{
    if (!qcdasf)
        return NULL;

    qcdasf->Use++;

    return qcdasf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check DAS Feature object.
**
** @fdata [EnsPQcdasfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check DAS Feature
**
** @argrule * Pqcdasf [EnsPQcdasfeature*]
** Ensembl Quality Check DAS Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcdasfeatureDel ***************************************************
**
** Default destructor for an Ensembl Quality Check DAS Feature.
**
** @param [d] Pqcdasf [EnsPQcdasfeature*]
** Ensembl Quality Check DAS Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcdasfeatureDel(EnsPQcdasfeature *Pqcdasf)
{
    EnsPQcdasfeature pthis = NULL;

    if (!Pqcdasf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensQcdasfeatureDel"))
    {
        ajDebug("ensQcdasfeatureDel\n"
                "  *Pqcdasf %p\n",
                *Pqcdasf);

        ensQcdasfeatureTrace(*Pqcdasf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pqcdasf) || --pthis->Use)
    {
        *Pqcdasf = NULL;

        return;
    }

    ensQcalignmentDel(&pthis->Qcalignment);

    ensAnalysisDel(&pthis->Analysis);

    ensQcsequenceDel(&pthis->SegmentSequence);
    ensQcsequenceDel(&pthis->FeatureSequence);

    ajMemFree((void **) Pqcdasf);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check DAS Feature object.
**
** @fdata [EnsPQcdasfeature]
**
** @nam3rule Get Return Quality Check DAS Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl DAS Feature Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Category Return the category
** @nam4rule Feature Return Feature attribute{s}
** @nam5rule End Return the feature end
** @nam5rule Sequence Return the feature Ensembl Quality Check Sequence
** @nam5rule Start Return the feature start
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Phase Return the phase
** @nam4rule Qcalignment Return the Ensembl Quality Check Alignment
** @nam4rule Segment Return segment attribute(s)
** @nam5rule End Return the segment end
** @nam5rule Sequence Return the segment Ensembl Quality Check Sequence
** @nam5rule Start Return the segment start
** @nam5rule Strand Return the segment strand
** @nam4rule Type Return the type
**
** @argrule * qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @valrule Adaptor [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Category [EnsEQcdasfeatureCategory] Category or
** ensEQcdasfeatureCategoryNULL
** @valrule FeatureEnd [ajuint] Feature end or 0U
** @valrule FeatureSequence [EnsPQcsequence]
** Feature Ensembl Quality Check Sequence or NULL
** @valrule FeatureStart [ajuint] Feature start or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Phase [ajint] Phase or 0
** @valrule Qcalignment [EnsPQcalignment]
** Ensembl Quality Check Alignment or NULL
** @valrule SegmentSequence [EnsPQcsequence]
** Segment Ensembl Quality Check Sequence or NULL
** @valrule SegmentEnd [ajuint] Segment end or 0U
** @valrule SegmentStart [ajuint] Segment start or 0U
** @valrule SegmentStrand [ajint] Segment strand or 0
** @valrule Type [EnsEQcdasfeatureType] Type or ensEQcdasfeatureTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcdasfeatureGetAdaptor ********************************************
**
** Get the Ensembl Quality Check DAS Feature Adaptor member of an
** Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPQcdasfeatureadaptor] Ensembl Quality Check DAS Feature Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcdasfeatureadaptor ensQcdasfeatureGetAdaptor(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Adaptor : NULL;
}




/* @func ensQcdasfeatureGetAnalysis *******************************************
**
** Get the Ensembl Analysis member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensQcdasfeatureGetAnalysis(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Analysis : NULL;
}




/* @func ensQcdasfeatureGetCategory *******************************************
**
** Get the category member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsEQcdasfeatureCategory] Category or ensEQcdasfeatureCategoryNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdasfeatureCategory ensQcdasfeatureGetCategory(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Category : ensEQcdasfeatureCategoryNULL;
}




/* @func ensQcdasfeatureGetFeatureEnd *****************************************
**
** Get the feature end member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajuint] Feature end or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcdasfeatureGetFeatureEnd(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->FeatureEnd : 0U;
}




/* @func ensQcdasfeatureGetFeatureSequence ************************************
**
** Get the feature Ensembl Quality Check Sequence member of an
** Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPQcsequence] Ensembl Quality Check Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcsequence ensQcdasfeatureGetFeatureSequence(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->FeatureSequence : NULL;
}




/* @func ensQcdasfeatureGetFeatureStart ***************************************
**
** Get the feature start member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajuint] Feature start or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcdasfeatureGetFeatureStart(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->FeatureStart : 0U;
}




/* @func ensQcdasfeatureGetIdentifier *****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcdasfeatureGetIdentifier(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Identifier : 0U;
}




/* @func ensQcdasfeatureGetPhase **********************************************
**
** Get the phase member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajint] Phase or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensQcdasfeatureGetPhase(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Phase : 0;
}




/* @func ensQcdasfeatureGetQcalignment ****************************************
**
** Get the Ensembl Quality Check Alignment member of an
** Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPQcalignment] Ensembl Quality Check Alignment or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcalignment ensQcdasfeatureGetQcalignment(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Qcalignment : NULL;
}




/* @func ensQcdasfeatureGetSegmentEnd *****************************************
**
** Get the segment end member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajuint] Segment end or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcdasfeatureGetSegmentEnd(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->SegmentEnd : 0U;
}




/* @func ensQcdasfeatureGetSegmentSequence ************************************
**
** Get the segment Ensembl Quality Check Sequence member of an
** Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsPQcsequence] Ensembl Quality Check Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcsequence ensQcdasfeatureGetSegmentSequence(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->SegmentSequence : NULL;
}




/* @func ensQcdasfeatureGetSegmentStart ***************************************
**
** Get the segment start member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajuint] Segment start or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcdasfeatureGetSegmentStart(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->SegmentStart : 0U;
}




/* @func ensQcdasfeatureGetSegmentStrand **************************************
**
** Get the segment strand member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [ajint] Segment strand or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensQcdasfeatureGetSegmentStrand(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->SegmentStrand : 0;
}




/* @func ensQcdasfeatureGetType ***********************************************
**
** Get the type member of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [EnsEQcdasfeatureType] Type or ensEQcdasfeatureTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdasfeatureType ensQcdasfeatureGetType(
    const EnsPQcdasfeature qcdasf)
{
    return (qcdasf) ? qcdasf->Type : ensEQcdasfeatureTypeNULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an
** Ensembl Quality Check DAS Feature object.
**
** @fdata [EnsPQcdasfeature]
**
** @nam3rule Set Set one member of an Ensembl Quality Check DAS Feature
** @nam4rule Adaptor Set the Ensembl Quality Check DAS Feature Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Category Set the category
** @nam4rule Feature Set Feature members
** @nam5rule End Set the feature end
** @nam5rule Sequence Set the feature Ensembl Quality Check Sequence
** @nam5rule Start Set the feature start
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Phase Set the phase
** @nam4rule Qcalignment Set the Ensembl Quality Check Alignment
** @nam4rule Segment Set Segment members
** @nam5rule End Set the segment end
** @nam5rule Sequence Set the segment Ensembl Quality Check Sequence
** @nam5rule Start Set the segment start
** @nam5rule Strand Set the segment strand
** @nam4rule Type Set the type
**
** @argrule * qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @argrule Adaptor qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Category category [EnsEQcdasfeatureCategory] Category
** @argrule Phase phase [ajint] Phase
** @argrule FeatureEnd end [ajuint] Feature end
** @argrule FeatureSequence qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule FeatureStart start [ajuint] Feature start
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Qcalignment qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule SegmentEnd end [ajuint] Segment end
** @argrule SegmentSequence qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule SegmentStart start [ajuint] Segment start
** @argrule SegmentStrand strand [ajint] Segment strand
** @argrule Type type [EnsEQcdasfeatureType] Type
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcdasfeatureSetAdaptor ********************************************
**
** Set the Ensembl Quality Check DAS Feature Adaptor member of an
** Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetAdaptor(EnsPQcdasfeature qcdasf,
                                 EnsPQcdasfeatureadaptor qcdasfa)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->Adaptor = qcdasfa;

    return ajTrue;
}




/* @func ensQcdasfeatureSetAnalysis *******************************************
**
** Set the Ensembl Analysis member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetAnalysis(EnsPQcdasfeature qcdasf,
                                  EnsPAnalysis analysis)
{
    if (!qcdasf)
        return ajFalse;

    ensAnalysisDel(&qcdasf->Analysis);

    qcdasf->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcdasfeatureSetCategory *******************************************
**
** Set the category member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] category [EnsEQcdasfeatureCategory] Category
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetCategory(EnsPQcdasfeature qcdasf,
                                  EnsEQcdasfeatureCategory category)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->Category = category;

    return ajTrue;
}




/* @func ensQcdasfeatureSetFeatureEnd *****************************************
**
** Set the feature end member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] end [ajuint] Feature end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetFeatureEnd(EnsPQcdasfeature qcdasf,
                                    ajuint end)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->FeatureEnd = end;

    return ajTrue;
}




/* @func ensQcdasfeatureSetFeatureSequence ************************************
**
** Set the feature Ensembl Quality Check Sequence member of an
** Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetFeatureSequence(EnsPQcdasfeature qcdasf,
                                         EnsPQcsequence qcs)
{
    if (!qcdasf)
        return ajFalse;

    ensQcsequenceDel(&qcdasf->FeatureSequence);

    qcdasf->FeatureSequence = ensQcsequenceNewRef(qcs);

    return ajTrue;
}




/* @func ensQcdasfeatureSetFeatureStart ***************************************
**
** Set the feature start member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] start [ajuint] Feature start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetFeatureStart(EnsPQcdasfeature qcdasf,
                                      ajuint start)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->FeatureStart = start;

    return ajTrue;
}




/* @func ensQcdasfeatureSetIdentifier *****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetIdentifier(EnsPQcdasfeature qcdasf,
                                    ajuint identifier)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcdasfeatureSetPhase **********************************************
**
** Set the phase member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] phase [ajint] Phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetPhase(EnsPQcdasfeature qcdasf,
                               ajint phase)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->Phase = phase;

    return ajTrue;
}




/* @func ensQcdasfeatureSetQcalignment ****************************************
**
** Set the Ensembl Quality Check Alignment member of an
** Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetQcalignment(EnsPQcdasfeature qcdasf,
                                     EnsPQcalignment qca)
{
    if (!qcdasf)
        return ajFalse;

    ensQcalignmentDel(&qcdasf->Qcalignment);

    qcdasf->Qcalignment = ensQcalignmentNewRef(qca);

    return ajTrue;
}




/* @func ensQcdasfeatureSetSegmentEnd *****************************************
**
** Set the segment end member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] end [ajuint] Segment end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetSegmentEnd(EnsPQcdasfeature qcdasf,
                                    ajuint end)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->SegmentEnd = end;

    return ajTrue;
}




/* @func ensQcdasfeatureSetSegmentSequence ************************************
**
** Set the segment Ensembl Quality Check Sequence member of an
** Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetSegmentSequence(EnsPQcdasfeature qcdasf,
                                         EnsPQcsequence qcs)
{
    if (!qcdasf)
        return ajFalse;

    ensQcsequenceDel(&qcdasf->SegmentSequence);

    qcdasf->SegmentSequence = ensQcsequenceNewRef(qcs);

    return ajTrue;
}




/* @func ensQcdasfeatureSetSegmentStart ***************************************
**
** Set the segment start member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] start [ajuint] Segment start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetSegmentStart(EnsPQcdasfeature qcdasf,
                                      ajuint start)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->SegmentStart = start;

    return ajTrue;
}




/* @func ensQcdasfeatureSetSegmentStrand **************************************
**
** Set the segment strand member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] strand [ajint] Segment strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetSegmentStrand(EnsPQcdasfeature qcdasf,
                                       ajint strand)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->SegmentStrand = strand;

    return ajTrue;
}




/* @func ensQcdasfeatureSetType ***********************************************
**
** Set the type member of an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [u] type [EnsEQcdasfeatureType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureSetType(EnsPQcdasfeature qcdasf,
                              EnsEQcdasfeatureType type)
{
    if (!qcdasf)
        return ajFalse;

    qcdasf->Type = type;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Quality Check DAS Feature object.
**
** @fdata [EnsPQcdasfeature]
**
** @nam3rule Trace Report Ensembl Quality Check DAS Feature members to
** debug file
**
** @argrule Trace qcdasf [const EnsPQcdasfeature]
** Ensembl Quality Check DAS Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdasfeatureTrace *************************************************
**
** Trace an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureTrace(const EnsPQcdasfeature qcdasf, ajuint level)
{
    AjPStr indent = NULL;

    if (!qcdasf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensQcdasfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Qcalignment %p\n"
            "%S  Analysis %p\n"
            "%S  SegmentSequence %p\n"
            "%S  FeatureSequence %p\n"
            "%S  SegmentStart %u\n"
            "%S  SegmentEnd %u\n"
            "%S  SegmentStrand %d\n"
            "%S  FeatureStart %u\n"
            "%S  FeatureEnd %u\n"
            "%S  Phase %d\n"
            "%S  Category '%s'\n"
            "%S  Type '%s'\n",
            indent, qcdasf,
            indent, qcdasf->Use,
            indent, qcdasf->Identifier,
            indent, qcdasf->Adaptor,
            indent, qcdasf->Qcalignment,
            indent, qcdasf->Analysis,
            indent, qcdasf->SegmentSequence,
            indent, qcdasf->FeatureSequence,
            indent, qcdasf->SegmentStart,
            indent, qcdasf->SegmentEnd,
            indent, qcdasf->SegmentStrand,
            indent, qcdasf->FeatureStart,
            indent, qcdasf->FeatureEnd,
            indent, qcdasf->Phase,
            indent, ensQcdasfeatureCategoryToChar(qcdasf->Category),
            indent, ensQcdasfeatureTypeToChar(qcdasf->Type));

    ensAnalysisTrace(qcdasf->Analysis, 1);

    ensQcalignmentTrace(qcdasf->Qcalignment, 1);

    ensQcsequenceTrace(qcdasf->SegmentSequence, 1);
    ensQcsequenceTrace(qcdasf->FeatureSequence, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Quality Check DAS Feature object.
**
** @fdata [EnsPQcdasfeature]
**
** @nam3rule Calculate Calculate Ensembl Quality Check DAS Feature information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdasfeatureCalculateMemsize **************************************
**
** Get the memory size in bytes of an Ensembl Quality Check DAS Feature.
**
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensQcdasfeatureCalculateMemsize(const EnsPQcdasfeature qcdasf)
{
    size_t size = 0;

    if (!qcdasf)
        return 0;

    size += sizeof (EnsOQcdasfeature);

    size += ensAnalysisCalculateMemsize(qcdasf->Analysis);

    size += ensQcalignmentCalculateMemsize(qcdasf->Qcalignment);

    size += ensQcsequenceCalculateMemsize(qcdasf->SegmentSequence);
    size += ensQcsequenceCalculateMemsize(qcdasf->FeatureSequence);

    return size;
}




/* @datasection [EnsEQcdasfeatureCategory] Ensembl Quality Check DAS Feature
** Category
**
** @nam2rule Qcdasfeature Functions for manipulating
** Ensembl Quality Check DAS Feature objects
** @nam3rule QcdasfeatureCategory Functions for manipulating
** Ensembl Quality Check DAS Feature Category enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl Quality Check DAS Feature Category enumeration.
**
** @fdata [EnsEQcdasfeatureCategory]
**
** @nam4rule From Ensembl Quality Check DAS Feature Category query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  category  [const AjPStr] Category string
**
** @valrule * [EnsEQcdasfeatureCategory]
** Ensembl Quality Check DAS Feature Category enumeration or
** ensEQcdasfeatureCategoryNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdasfeatureCategoryFromStr ***************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check DAS Feature Category enumeration.
**
** @param [r] category [const AjPStr] Category string
**
** @return [EnsEQcdasfeatureCategory]
** Ensembl Quality Check DAS Feature Category or ensEQcdasfeatureCategoryNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdasfeatureCategory ensQcdasfeatureCategoryFromStr(const AjPStr category)
{
    register EnsEQcdasfeatureCategory i = ensEQcdasfeatureCategoryNULL;

    EnsEQcdasfeatureCategory ecategory = ensEQcdasfeatureCategoryNULL;

    for (i = ensEQcdasfeatureCategoryNULL;
         qcdasfeatureKCategory[i];
         i++)
        if (ajStrMatchCaseC(category, qcdasfeatureKCategory[i]))
            ecategory = i;

    if (!ecategory)
        ajDebug("ensQcdasfeatureCategoryFromStr encountered "
                "unexpected string '%S'.\n", category);

    return ecategory;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check DAS Feature Category enumeration.
**
** @fdata [EnsEQcdasfeatureCategory]
**
** @nam4rule To   Return Ensembl Quality Check DAS Feature Category enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcdasfc [EnsEQcdasfeatureCategory]
** Ensembl Quality Check DAS Feature Category enumeration
**
** @valrule Char [const char*] Category or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcdasfeatureCategoryToChar ****************************************
**
** Convert an Ensembl Quality Check DAS Feature Category enumeration into a
** C-type (char *) string.
**
** @param [u] qcdasfc [EnsEQcdasfeatureCategory]
** Ensembl Quality Check DAS Feature Category enumeration
**
** @return [const char*]
** Ensembl Quality Check DAS Feature Category C-type (char *) string or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcdasfeatureCategoryToChar(EnsEQcdasfeatureCategory qcdasfc)
{
    register EnsEQcdasfeatureCategory i = ensEQcdasfeatureCategoryNULL;

    for (i = ensEQcdasfeatureCategoryNULL;
         qcdasfeatureKCategory[i] && (i < qcdasfc);
         i++);

    if (!qcdasfeatureKCategory[i])
        ajDebug("ensQcdasfeatureCategoryToChar "
                "encountered an out of boundary error on "
                "Ensembl Quality Check DAS Feature Category "
                "enumeration %d.\n",
                qcdasfc);

    return qcdasfeatureKCategory[i];
}




/* @datasection [EnsEQcdasfeatureType] Ensembl Quality Check DAS Feature Type
**
** @nam2rule Qcdasfeature Functions for manipulating
** Ensembl Quality Check DAS Feature objects
** @nam3rule QcdasfeatureType Functions for manipulating
** Ensembl Quality Check DAS Feature Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl Quality Check DAS Feature Type enumeration.
**
** @fdata [EnsEQcdasfeatureType]
**
** @nam4rule From Ensembl Quality Check DAS Feature Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEQcdasfeatureType]
** Ensembl Quality Check DAS Feature Type enumeration or
** ensEQcdasfeatureTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdasfeatureTypeFromStr *******************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check DAS Feature Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEQcdasfeatureType]
** Ensembl Quality Check DAS Feature Type enumeration or
** ensEQcdasfeatureTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdasfeatureType ensQcdasfeatureTypeFromStr(const AjPStr type)
{
    register EnsEQcdasfeatureType i = ensEQcdasfeatureTypeNULL;

    EnsEQcdasfeatureType etype = ensEQcdasfeatureTypeNULL;

    for (i = ensEQcdasfeatureTypeNULL;
         qcdasfeatureKType[i];
         i++)
        if (ajStrMatchCaseC(type, qcdasfeatureKType[i]))
            etype = i;

    if (!etype)
        ajDebug("ensQcdasfeatureTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check DAS Feature Type enumeration.
**
** @fdata [EnsEQcdasfeatureType]
**
** @nam4rule To   Return Ensembl Quality Check DAS Feature Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcdasft [EnsEQcdasfeatureType]
** Ensembl Quality Check DAS Feature Type enumeration
**
** @valrule Char [const char*] Type or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcdasfeatureTypeToChar ********************************************
**
** Convert an Ensembl Quality Check DAS Feature Type enumeration into a
** C-type (char *) string.
**
** @param [u] qcdasft [EnsEQcdasfeatureType]
** Ensembl Quality Check DAS Feature Type
**
** @return [const char*]
** Ensembl Quality Check DAS Feature Type C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcdasfeatureTypeToChar(EnsEQcdasfeatureType qcdasft)
{
    register EnsEQcdasfeatureType i = ensEQcdasfeatureTypeNULL;

    for (i = ensEQcdasfeatureTypeNULL;
         qcdasfeatureKType[i] && (i < qcdasft);
         i++);

    if (!qcdasfeatureKType[i])
        ajDebug("ensQcdasfeatureTypeToChar "
                "encountered an out of boundary error on "
                "Ensembl Quality Check DAS Feature Type "
                "enumeration %d.\n",
                qcdasft);

    return qcdasfeatureKType[i];
}




/* @datasection [EnsPQcdasfeatureadaptor] Ensembl Quality Check DAS Feature
** Adaptor
**
** @nam2rule Qcdasfeatureadaptor Functions for manipulating
** Ensembl Quality Check (QC) Distributed Annotation System (DAS) Feature
** Adaptor objects
**
** @cc Bio::EnsEMBL::QC::DBSQL::DASFeatureadaptor
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @funcstatic qcdasfeatureadaptorFetchAllbyStatement *************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Quality Check DAS Feature objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcdasfs [AjPList]
** AJAX List of Ensembl Quality Check DAS Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool qcdasfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcdasfs)
{
    ajint tstrand = 0;
    ajint phase   = 0;

    ajuint identifier  = 0U;
    ajuint analysisid  = 0U;
    ajuint alignmentid = 0U;
    ajuint qsid        = 0U;
    ajuint qstart      = 0U;
    ajuint qend        = 0U;
    ajuint tsid        = 0U;
    ajuint tstart      = 0U;
    ajuint tend        = 0U;

    EnsEQcdasfeatureCategory ecategory = ensEQcdasfeatureCategoryNULL;
    EnsEQcdasfeatureType etype         = ensEQcdasfeatureTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr category = NULL;
    AjPStr type     = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPQcalignment qca         = NULL;
    EnsPQcalignmentadaptor qcaa = NULL;

    EnsPQcdasfeature qcdasf         = NULL;
    EnsPQcdasfeatureadaptor qcdasfa = NULL;

    EnsPQcsequence qsequence   = NULL;
    EnsPQcsequence tsequence   = NULL;
    EnsPQcsequenceadaptor qcsa = NULL;

    if (ajDebugTest("qcdasfeatureadaptorFetchAllbyStatement"))
        ajDebug("qcdasfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcdasfs %p\n",
                ba,
                statement,
                am,
                slice,
                qcdasfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!qcdasfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    aa      = ensRegistryGetAnalysisadaptor(dba);
    qcaa    = ensRegistryGetQcalignmentadaptor(dba);
    qcdasfa = ensRegistryGetQcdasfeatureadaptor(dba);
    qcsa    = ensRegistryGetQcsequenceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        analysisid  = 0;
        alignmentid = 0;
        qsid        = 0;
        qstart      = 0;
        qend        = 0;
        tsid        = 0;
        tstart      = 0;
        tend        = 0;
        tstrand     = 0;
        phase       = 0;
        category    = ajStrNew();
        type        = ajStrNew();
        ecategory   = ensEQcdasfeatureCategoryNULL;
        etype       = ensEQcdasfeatureTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToUint(sqlr, &alignmentid);
        ajSqlcolumnToUint(sqlr, &qsid);
        ajSqlcolumnToUint(sqlr, &qstart);
        ajSqlcolumnToUint(sqlr, &qend);
        ajSqlcolumnToUint(sqlr, &tsid);
        ajSqlcolumnToUint(sqlr, &tstart);
        ajSqlcolumnToUint(sqlr, &tend);
        ajSqlcolumnToInt(sqlr, &tstrand);
        ajSqlcolumnToInt(sqlr, &phase);
        ajSqlcolumnToStr(sqlr, &category);
        ajSqlcolumnToStr(sqlr, &type);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        ensQcalignmentadaptorFetchByIdentifier(qcaa, alignmentid, &qca);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, qsid, &qsequence);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, tsid, &tsequence);

        ecategory = ensQcdasfeatureCategoryFromStr(category);

        etype = ensQcdasfeatureTypeFromStr(type);

        qcdasf = ensQcdasfeatureNewIni(qcdasfa,
                                       identifier,
                                       qca,
                                       analysis,
                                       qsequence,
                                       qstart,
                                       qend,
                                       tstrand,
                                       tsequence,
                                       tstart,
                                       tend,
                                       phase,
                                       ecategory,
                                       etype);

        ajListPushAppend(qcdasfs, (void *) qcdasf);

        ensQcsequenceDel(&qsequence);
        ensQcsequenceDel(&tsequence);

        ensAnalysisDel(&analysis);

        ensQcalignmentDel(&qca);

        ajStrDel(&category);
        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check DAS Feature Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Quality Check DAS Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcdasfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcdasfeatureadaptorNew ********************************************
**
** Default constructor for an Ensembl Quality Check DAS Feature Adaptor.
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
** @see ensRegistryGetQcdasfeatureadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcdasfeatureadaptor ensQcdasfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensBaseadaptorNew(
        dba,
        qcdasfeatureadaptorKTablenames,
        qcdasfeatureadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &qcdasfeatureadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check DAS Feature Adaptor object.
**
** @fdata [EnsPQcdasfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check DAS Feature Adaptor
**
** @argrule * Pqcdasfa [EnsPQcdasfeatureadaptor*]
** Ensembl Quality Check DAS Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcdasfeatureadaptorDel ********************************************
**
** Default destructor for an Ensembl Quality Check DAS Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcdasfa [EnsPQcdasfeatureadaptor*]
** Ensembl Quality Check DAS Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcdasfeatureadaptorDel(EnsPQcdasfeatureadaptor *Pqcdasfa)
{
    ensBaseadaptorDel(Pqcdasfa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check DAS Feature Adaptor object.
**
** @fdata [EnsPQcdasfeatureadaptor]
**
** @nam3rule Get Return Ensembl Quality Check DAS Feature Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcdasfeatureadaptorGetBaseadaptor *********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Quality Check DAS Feature Adaptor.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensQcdasfeatureadaptorGetBaseadaptor(
    EnsPQcdasfeatureadaptor qcdasfa)
{
    return qcdasfa;
}




/* @func ensQcdasfeatureadaptorGetDatabaseadaptor *****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Quality Check DAS Feature Adaptor.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensQcdasfeatureadaptorGetDatabaseadaptor(
    EnsPQcdasfeatureadaptor qcdasfa)
{
    return ensBaseadaptorGetDatabaseadaptor(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa));
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Quality Check DAS Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPQcdasfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Quality Check DAS Feature object(s)
** @nam4rule All   Fetch all Ensembl Quality Check DAS Feature objects
** @nam4rule Allby Fetch all Ensembl Quality Check DAS Feature objects matching
**                 a criterion
** @nam5rule Qcalignment Fetch all by an Ensembl Quality Check Alignment
** @nam5rule Qcsequence Fetch all by an Ensembl Quality Check Sequence
** @nam6rule Feature    Fetch all by an Ensembl Quality Check Sequence
** as feature
** @nam6rule Pair       Fetch all by an Ensembl Quality Check Sequence pair
** @nam6rule Segment    Fetch all by an Ensembl Quality Check Sequence
** as segment
** @nam5rule Region     Fetch all by a region
** @nam4rule By Fetch one Ensembl Quality Check DAS Feature object matching a
** criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
**
** @argrule * qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @argrule All qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
** @argrule AllbyQcalignment qca [const EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule AllbyQcalignment qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
** @argrule AllbyQcsequenceFeature analysis [const EnsPAnalysis]
** Ensembl Analysis
** @argrule AllbyQcsequenceFeature feature [const EnsPQcsequence]
** Feature Ensembl Quality Check Sequence
** @argrule AllbyQcsequenceFeature qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
** @argrule AllbyQcsequencePair analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcsequencePair feature [const EnsPQcsequence]
** Feature Ensembl Quality Check Sequence
** @argrule AllbyQcsequencePair segment [const EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @argrule AllbyQcsequencePair qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
** @argrule AllbyQcsequenceSegment analysis [const EnsPAnalysis]
** Ensembl Analysis
** @argrule AllbyQcsequenceSegment segment [const EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @argrule AllbyQcsequenceSegment qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
** @argrule AllbyRegion analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyRegion segment [const EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @argrule AllbyRegion start [ajuint] Start
** @argrule AllbyRegion end [ajuint] End
** @argrule AllbyRegion qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pqcdasf [EnsPQcdasfeature*]
** Ensembl Quality Check DAS Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcdasfeatureadaptorFetchAllbyQcalignment **************************
**
** Fetch all Ensembl Quality Check DAS Feature objects by an
** Ensembl Quality Check Alignment.
**
** The caller is responsible for deleting the Ensembl Quality Check DAS Feature
** objects before deleting the AJAX List.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @param [u] qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorFetchAllbyQcalignment(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPQcalignment qca,
    AjPList qcdasfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!qca)
        return ajFalse;

    if (!qcdasfs)
        return ajFalse;

    constraint = ajFmtStr("das_feature.alignment_id = %u",
                          ensQcalignmentGetIdentifier(qca));

    result = ensBaseadaptorFetchAllbyConstraint(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        qcdasfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensQcdasfeatureadaptorFetchAllbyQcsequenceFeature ********************
**
** Fetch all Ensembl Quality Check DAS Feature objects by an
** Ensembl Quality Check Sequence representing an
** Ensembl Quality Check DAS Feature feature member.
**
** The caller is responsible for deleting the Ensembl Quality Check DAS Feature
** objects before deleting the AJAX List.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] feature [const EnsPQcsequence]
** Feature Ensembl Quality Check Sequence
** @param [u] qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorFetchAllbyQcsequenceFeature(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence feature,
    AjPList qcdasfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!feature)
        return ajFalse;

    if (!qcdasfs)
        return ajFalse;

    constraint = ajFmtStr("das_feature.feature_id = %u",
                          ensQcsequenceGetIdentifier(feature));

    if (analysis)
        ajFmtPrintAppS(&constraint,
                       " AND das_feature.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    result = ensBaseadaptorFetchAllbyConstraint(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        qcdasfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensQcdasfeatureadaptorFetchAllbyQcsequencePair ***********************
**
** Fetch all Ensembl Quality Check DAS Feature objects by an
** Ensembl Analysis and an Ensembl Quality Check Sequence pair representing
** Ensembl Quality Check DAS Feature feature and segment.
**
** The caller is responsible for deleting the Ensembl Quality Check DAS Feature
** objects before deleting the AJAX List.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] feature [const EnsPQcsequence]
** Feature Ensembl Quality Check Sequence
** @param [r] segment [const EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @param [u] qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorFetchAllbyQcsequencePair(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence feature,
    const EnsPQcsequence segment,
    AjPList qcdasfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!analysis)
        return ajFalse;

    if (!feature)
        return ajFalse;

    if (!segment)
        return ajFalse;

    constraint = ajFmtStr("das_feature.analysis_id = %u "
                          "AND "
                          "das_feature.feature_id = %u "
                          "AND "
                          "das_feature.segment_id = %u",
                          ensAnalysisGetIdentifier(analysis),
                          ensQcsequenceGetIdentifier(feature),
                          ensQcsequenceGetIdentifier(segment));

    result = ensBaseadaptorFetchAllbyConstraint(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        qcdasfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensQcdasfeatureadaptorFetchAllbyQcsequenceSegment ********************
**
** Fetch all Ensembl Quality Check DAS Feature objects by an
** Ensembl Quality Check Sequence representing an
** Ensembl Quality Check DAS Feature segment.
**
** The caller is responsible for deleting the Ensembl Quality Check DAS Feature
** objects before deleting the AJAX List.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] segment [const EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @param [u] qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorFetchAllbyQcsequenceSegment(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence segment,
    AjPList qcdasfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!segment)
        return ajFalse;

    if (!qcdasfs)
        return ajFalse;

    constraint = ajFmtStr("das_feature.segment_id = %u",
                          ensQcsequenceGetIdentifier(segment));

    if (analysis)
        ajFmtPrintAppS(&constraint,
                       " AND das_feature.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    result = ensBaseadaptorFetchAllbyConstraint(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        qcdasfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensQcdasfeatureadaptorFetchAllbyRegion *******************************
**
** Fetch all Ensembl Quality Check DAS Feature objects that fall into a region
** on the segment.
**
** The caller is responsible for deleting the Ensembl Quality Check DAS Feature
** objects before deleting the AJAX List.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] segment [const EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @param [r] start [ajuint] Start
** @param [r] end [ajuint] End
** @param [u] qcdasfs [AjPList] AJAX List of
** Ensembl Quality Check DAS Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorFetchAllbyRegion(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence segment,
    ajuint start,
    ajuint end,
    AjPList qcdasfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!analysis)
        return ajFalse;

    if (!segment)
        return ajFalse;

    if (!qcdasfs)
        return ajFalse;

    constraint = ajFmtStr(
        "das.analysis_id = %u "
        "AND "
        "das.segment_id = %u "
        "AND "
        "("
        "(das.segment_start >= %u AND das.segment_start <= %u) "
        "OR "
        "(das.segment_end >= %u AND das.segment_end <= %u) "
        "OR "
        "(das.segment_start <= %u AND das.segment_end >= %u)"
        ")",
        ensAnalysisGetIdentifier(analysis),
        ensQcsequenceGetIdentifier(segment),
        start,
        end,
        start,
        end,
        start,
        end);

    result = ensBaseadaptorFetchAllbyConstraint(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        qcdasfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensQcdasfeatureadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Quality Check DAS Feature via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal
** Ensembl Quality Check DAS Feature identifier
** @param [wP] Pqcdasf [EnsPQcdasfeature*]
** Ensembl Quality Check DAS Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorFetchByIdentifier(
    EnsPQcdasfeatureadaptor qcdasfa,
    ajuint identifier,
    EnsPQcdasfeature *Pqcdasf)
{
    return ensBaseadaptorFetchByIdentifier(
        ensQcdasfeatureadaptorGetBaseadaptor(qcdasfa),
        identifier,
        (void **) Pqcdasf);
}




/* @section object access *****************************************************
**
** Functions for accessing Ensembl Quality Check DAS Feature objects in an
** Ensembl SQL database.
**
** @fdata [EnsPQcdasfeatureadaptor]
**
** @nam3rule Delete Delete Ensembl Quality Check DAS Feature object(s)
** @nam3rule Store  Store Ensembl Quality Check DAS Feature object(s)
** @nam3rule Update Update Ensembl Quality Check DAS Feature object(s)
**
** @argrule * qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @argrule Delete qcdasf [EnsPQcdasfeature]
** Ensembl Quality Check DAS Feature
** @argrule Store qcdasf [EnsPQcdasfeature]
** Ensembl Quality Check DAS Feature
** @argrule Update qcdasf [const EnsPQcdasfeature]
** Ensembl Quality Check DAS Feature
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcdasfeatureadaptorDelete *****************************************
**
** Delete an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorDelete(EnsPQcdasfeatureadaptor qcdasfa,
                                    EnsPQcdasfeature qcdasf)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!qcdasf)
        return ajFalse;

    if (!ensQcdasfeatureGetIdentifier(qcdasf))
        return ajFalse;

    dba = ensQcdasfeatureadaptorGetDatabaseadaptor(qcdasfa);

    statement = ajFmtStr(
        "DELETE FROM "
        "das_feature "
        "WHERE "
        "das_feature.das_feature_id = %u",
        qcdasf->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        qcdasf->Adaptor    = NULL;
        qcdasf->Identifier = 0U;

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcdasfeatureadaptorStore ******************************************
**
** Store an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [u] qcdasf [EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorStore(EnsPQcdasfeatureadaptor qcdasfa,
                                   EnsPQcdasfeature qcdasf)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!qcdasf)
        return ajFalse;

    if (ensQcdasfeatureGetAdaptor(qcdasf) &&
        ensQcdasfeatureGetIdentifier(qcdasf))
        return ajFalse;

    dba = ensQcdasfeatureadaptorGetDatabaseadaptor(qcdasfa);

    statement = ajFmtStr(
        "INSERT IGNORE INTO "
        "das_feature "
        "SET "
        "das_feature.alignment_id = %u, "
        "das_feature.analysis_id = %u, "
        "das_feature.feature_id = %u, "
        "das_feature.feature_start = %u, "
        "das_feature.feature_end = %u, "
        "das_feature.segment_id = %u, "
        "das_feature.segment_start = %u, "
        "das_feature.segment_end = %u, "
        "das_feature.segment_strand = %d, "
        "das_feature.phase = %d, "
        "das_feature.category = '%s', "
        "das_feature.type = '%s'",
        ensQcalignmentGetIdentity(qcdasf->Qcalignment),
        ensAnalysisGetIdentifier(qcdasf->Analysis),
        ensQcsequenceGetIdentifier(qcdasf->FeatureSequence),
        qcdasf->FeatureStart,
        qcdasf->FeatureEnd,
        ensQcsequenceGetIdentifier(qcdasf->SegmentSequence),
        qcdasf->SegmentStart,
        qcdasf->SegmentEnd,
        qcdasf->SegmentStrand,
        qcdasf->Phase,
        ensQcdasfeatureCategoryToChar(qcdasf->Category),
        ensQcdasfeatureTypeToChar(qcdasf->Type));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcdasfeatureSetIdentifier(qcdasf,
                                     ajSqlstatementGetIdentifier(sqls));

        ensQcdasfeatureSetAdaptor(qcdasf, qcdasfa);

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcdasfeatureadaptorUpdate *****************************************
**
** Update an Ensembl Quality Check DAS Feature.
**
** @param [u] qcdasfa [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @param [r] qcdasf [const EnsPQcdasfeature] Ensembl Quality Check DAS Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdasfeatureadaptorUpdate(EnsPQcdasfeatureadaptor qcdasfa,
                                    const EnsPQcdasfeature qcdasf)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcdasfa)
        return ajFalse;

    if (!qcdasf)
        return ajFalse;

    if (!ensQcdasfeatureGetIdentifier(qcdasf))
        return ajFalse;

    dba = ensQcdasfeatureadaptorGetDatabaseadaptor(qcdasfa);

    statement = ajFmtStr(
        "UPDATE IGNORE "
        "das_feature "
        "SET "
        "das_feature.alignment_id = %u, "
        "das_feature.analysis_id = %u, "
        "das_feature.feature_id = %u, "
        "das_feature.feature_start = %u, "
        "das_feature.feature_end = %u, "
        "das_feature.segment_id = %u, "
        "das_feature.segment_start = %u, "
        "das_feature.segment_end = %u, "
        "das_feature.segment_strand = %d, "
        "das_feature.phase = %d, "
        "das_feature.category = '%s', "
        "das_feature.type = '%s' "
        "WHERE "
        "das_feature.das_feature_id = %u",
        ensQcalignmentGetIdentity(qcdasf->Qcalignment),
        ensAnalysisGetIdentifier(qcdasf->Analysis),
        ensQcsequenceGetIdentifier(qcdasf->FeatureSequence),
        qcdasf->FeatureStart,
        qcdasf->FeatureEnd,
        ensQcsequenceGetIdentifier(qcdasf->SegmentSequence),
        qcdasf->SegmentStart,
        qcdasf->SegmentEnd,
        qcdasf->SegmentStrand,
        qcdasf->Phase,
        ensQcdasfeatureCategoryToChar(qcdasf->Category),
        ensQcdasfeatureTypeToChar(qcdasf->Type),
        ensQcdasfeatureGetIdentifier(qcdasf));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
        result = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}

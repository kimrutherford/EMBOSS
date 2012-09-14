/* @include ensqcdasfeature ***************************************************
**
** Ensembl Quality Check DAS Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.7 $
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

#ifndef ENSQCDASFEATURE_H
#define ENSQCDASFEATURE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensqcalignment.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPQcdasfeatureadaptor *********************************************
**
** Ensembl Quality Check DAS Feature Adaptor
**
******************************************************************************/

#define EnsPQcdasfeatureadaptor EnsPBaseadaptor




/* @enum EnsEQcdasfeatureCategory *********************************************
**
** Ensembl Quality Check DAS Feature Category enumeration
**
** @value ensEQcdasfeatureCategoryNULL
** @value ensEQcdasfeatureCategoryUnknown
** @value ensEQcdasfeatureCategoryTranscriptPerfect
** @value ensEQcdasfeatureCategoryTranscriptTolerance
** @value ensEQcdasfeatureCategoryTranscriptPartial
** @value ensEQcdasfeatureCategoryTranscriptMissing
** @value ensEQcdasfeatureCategoryTranscript
** @value ensEQcdasfeatureCategoryTranslationPerfect
** @value ensEQcdasfeatureCategoryTranslationTolerance
** @value ensEQcdasfeatureCategoryTranslationPartial
** @value ensEQcdasfeatureCategoryTranslationMissing
** @value ensEQcdasfeatureCategoryTranslation
** @@
******************************************************************************/

typedef enum EnsOQcdasfeatureCategory
{
    ensEQcdasfeatureCategoryNULL,
    ensEQcdasfeatureCategoryUnknown,
    ensEQcdasfeatureCategoryTranscriptPerfect,
    ensEQcdasfeatureCategoryTranscriptTolerance,
    ensEQcdasfeatureCategoryTranscriptPartial,
    ensEQcdasfeatureCategoryTranscriptMissing,
    ensEQcdasfeatureCategoryTranscript,
    ensEQcdasfeatureCategoryTranslationPerfect,
    ensEQcdasfeatureCategoryTranslationTolerance,
    ensEQcdasfeatureCategoryTranslationPartial,
    ensEQcdasfeatureCategoryTranslationMissing,
    ensEQcdasfeatureCategoryTranslation
} EnsEQcdasfeatureCategory;




/* @enum EnsEQcdasfeatureType *************************************************
**
** Ensembl Quality Check DAS Feature Type enumeration
**
** @value ensEQcdasfeatureTypeNULL
** @value ensEQcdasfeatureTypeUnknown
** @value ensEQcdasfeatureTypeExonPerfect
** @value ensEQcdasfeatureTypeExonPartial
** @value ensEQcdasfeatureTypeExonMissing
** @value ensEQcdasfeatureTypeExonFrameshift
** @value ensEQcdasfeatureTypeExonGap
** @value ensEQcdasfeatureTypeExon
** @@
******************************************************************************/

typedef enum EnsOQcdasfeatureType
{
    ensEQcdasfeatureTypeNULL,
    ensEQcdasfeatureTypeUnknown,
    ensEQcdasfeatureTypeExonPerfect,
    ensEQcdasfeatureTypeExonPartial,
    ensEQcdasfeatureTypeExonMissing,
    ensEQcdasfeatureTypeExonFrameshift,
    ensEQcdasfeatureTypeExonGap,
    ensEQcdasfeatureTypeExon
} EnsEQcdasfeatureType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPQcdasfeature *****************************************************
**
** Ensembl Quality Check DAS Feature
**
** @alias EnsSQcdasfeature
** @alias EnsOQcdasfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @cc Bio::EnsEMBL::QC::DASFeature
** @cc 'das_feature' SQL table
** @attr Qcalignment [EnsPQcalignment] Ensembl Quality Check Alignment
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr SegmentSequence [EnsPQcsequence]
** Segment Ensembl Quality Check Sequence
** @attr FeatureSequence [EnsPQcsequence]
** Feature Ensembl Quality Check Sequence
** @attr SegmentStart [ajuint] Segment start
** @attr SegmentEnd [ajuint] Segment end
** @attr SegmentStrand [ajint] Segment strand
** @attr FeatureStart [ajuint] Feature start
** @attr FeatureEnd [ajuint] Feature end
** @attr Phase [ajint] Phase
** @attr Category [EnsEQcdasfeatureCategory] Category
** @attr Type [EnsEQcdasfeatureType] Type
** @@
******************************************************************************/

typedef struct EnsSQcdasfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcdasfeatureadaptor Adaptor;
    EnsPQcalignment Qcalignment;
    EnsPAnalysis Analysis;
    EnsPQcsequence SegmentSequence;
    EnsPQcsequence FeatureSequence;
    ajuint SegmentStart;
    ajuint SegmentEnd;
    ajint SegmentStrand;
    ajuint FeatureStart;
    ajuint FeatureEnd;
    ajint Phase;
    EnsEQcdasfeatureCategory Category;
    EnsEQcdasfeatureType Type;
} EnsOQcdasfeature;

#define EnsPQcdasfeature EnsOQcdasfeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Quality Check DAS Feature */

EnsPQcdasfeature ensQcdasfeatureNewCpy(const EnsPQcdasfeature qcdasf);

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
                                       EnsEQcdasfeatureType type);

EnsPQcdasfeature ensQcdasfeatureNewRef(EnsPQcdasfeature qcdasf);

void ensQcdasfeatureDel(EnsPQcdasfeature *Pqcdasf);

EnsPQcdasfeatureadaptor ensQcdasfeatureGetAdaptor(
    const EnsPQcdasfeature qcdasf);

EnsPAnalysis ensQcdasfeatureGetAnalysis(
    const EnsPQcdasfeature qcdasf);

EnsEQcdasfeatureCategory ensQcdasfeatureGetCategory(
    const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetFeatureEnd(
    const EnsPQcdasfeature qcdasf);

EnsPQcsequence ensQcdasfeatureGetFeatureSequence(
    const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetFeatureStart(
    const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetIdentifier(
    const EnsPQcdasfeature qcdasf);

ajint ensQcdasfeatureGetPhase(
    const EnsPQcdasfeature qcdasf);

EnsPQcalignment ensQcdasfeatureGetQcalignment(
    const EnsPQcdasfeature qcdasf);

EnsPQcsequence ensQcdasfeatureGetSegmentSequence(
    const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetSegmentStart(
    const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetSegmentEnd(
    const EnsPQcdasfeature qcdasf);

ajint ensQcdasfeatureGetSegmentStrand(
    const EnsPQcdasfeature qcdasf);

EnsEQcdasfeatureType ensQcdasfeatureGetType(
    const EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureSetAdaptor(EnsPQcdasfeature qcdasf,
                                 EnsPQcdasfeatureadaptor qcdasfa);

AjBool ensQcdasfeatureSetAnalysis(EnsPQcdasfeature qcdasf,
                                  EnsPAnalysis analysis);

AjBool ensQcdasfeatureSetCategory(EnsPQcdasfeature qcdasf,
                                  EnsEQcdasfeatureCategory category);

AjBool ensQcdasfeatureSetFeatureEnd(EnsPQcdasfeature qcdasf,
                                    ajuint end);

AjBool ensQcdasfeatureSetFeatureSequence(EnsPQcdasfeature qcdasf,
                                         EnsPQcsequence qcs);

AjBool ensQcdasfeatureSetFeatureStart(EnsPQcdasfeature qcdasf,
                                      ajuint start);

AjBool ensQcdasfeatureSetIdentifier(EnsPQcdasfeature qcdasf,
                                    ajuint identifier);

AjBool ensQcdasfeatureSetPhase(EnsPQcdasfeature qcdasf,
                               ajint phase);

AjBool ensQcdasfeatureSetQcalignment(EnsPQcdasfeature qcdasf,
                                     EnsPQcalignment qca);

AjBool ensQcdasfeatureSetSegmentEnd(EnsPQcdasfeature qcdasf,
                                    ajuint end);

AjBool ensQcdasfeatureSetSegmentSequence(EnsPQcdasfeature qcdasf,
                                         EnsPQcsequence qcs);

AjBool ensQcdasfeatureSetSegmentStart(EnsPQcdasfeature qcdasf,
                                      ajuint start);

AjBool ensQcdasfeatureSetSegmentStrand(EnsPQcdasfeature qcdasf,
                                       ajint strand);

AjBool ensQcdasfeatureSetType(EnsPQcdasfeature qcdasf,
                              EnsEQcdasfeatureType type);

AjBool ensQcdasfeatureTrace(const EnsPQcdasfeature qcdasf, ajuint level);

size_t ensQcdasfeatureCalculateMemsize(const EnsPQcdasfeature qcdasf);

EnsEQcdasfeatureCategory ensQcdasfeatureCategoryFromStr(const AjPStr category);

const char *ensQcdasfeatureCategoryToChar(EnsEQcdasfeatureCategory qcdasfc);

EnsEQcdasfeatureType ensQcdasfeatureTypeFromStr(const AjPStr type);

const char *ensQcdasfeatureTypeToChar(EnsEQcdasfeatureType qcdasft);

/* Ensembl Quality Check DAS Feature Adaptor */

EnsPQcdasfeatureadaptor ensRegistryGetQcdasfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcdasfeatureadaptor ensQcdasfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcdasfeatureadaptorDel(EnsPQcdasfeatureadaptor *Pqcdasfa);

EnsPBaseadaptor ensQcdasfeatureadaptorGetBaseadaptor(
    EnsPQcdasfeatureadaptor qcdasfa);

EnsPDatabaseadaptor ensQcdasfeatureadaptorGetDatabaseadaptor(
    EnsPQcdasfeatureadaptor qcdasfa);

AjBool ensQcdasfeatureadaptorFetchAllbyQcalignment(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPQcalignment qca,
    AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllbyQcsequenceFeature(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence feature,
    AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllbyQcsequencePair(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence feature,
    const EnsPQcsequence segment,
    AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllbyQcsequenceSegment(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence segment,
    AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllbyRegion(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence segment,
    ajuint start,
    ajuint end,
    AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchByIdentifier(
    EnsPQcdasfeatureadaptor qcdasfa,
    ajuint identifier,
    EnsPQcdasfeature *Pqcdasf);

AjBool ensQcdasfeatureadaptorDelete(EnsPQcdasfeatureadaptor qcdasfa,
                                    EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureadaptorStore(EnsPQcdasfeatureadaptor qcdasfa,
                                   EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureadaptorUpdate(EnsPQcdasfeatureadaptor qcdasfa,
                                    const EnsPQcdasfeature qcdasf);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSQCDASFEATURE_H */

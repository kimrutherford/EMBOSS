/* @include ensprediction *****************************************************
**
** Ensembl Prediction functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.24 $
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

#ifndef ENSPREDICTION_H
#define ENSPREDICTION_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPPredictionexonadaptor *******************************************
**
** Ensembl Prediction Exon Adaptor
**
** Bio::EnsEMBL::DBSQL::PredictionExonAdaptor
**
******************************************************************************/

#define EnsPPredictionexonadaptor EnsPFeatureadaptor




/* @const EnsPPredictiontranscriptadaptor *************************************
**
** Ensembl Prediction Transcript Adaptor
**
** Bio::EnsEMBL::DBSQL::PredictionTranscriptAdaptor
**
******************************************************************************/

#define EnsPPredictiontranscriptadaptor EnsPFeatureadaptor




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPPredictionexon ***************************************************
**
** Ensembl Prediction Exon.
**
** @alias EnsSPredictionexon
** @alias EnsOPredictionexon
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::PredictionExon
** @cc 'prediction_exon' SQL table
** @attr Score [double] Score
** @attr Pvalue [double] P-value
** @attr PhaseStart [ajint] Start phase
** @attr Padding [char[4]] Padding to alignment boundary
** @attr SequenceCache [AjPStr] Sequence cache
** @@
******************************************************************************/

typedef struct EnsSPredictionexon
{
    ajuint Use;
    ajuint Identifier;
    EnsPPredictionexonadaptor Adaptor;
    EnsPFeature Feature;
    double Score;
    double Pvalue;
    ajint PhaseStart;
    char Padding[4];
    AjPStr SequenceCache;
} EnsOPredictionexon;

#define EnsPPredictionexon EnsOPredictionexon*




/* @data EnsPPredictiontranscript *********************************************
**
** Ensembl Prediction Transcript.
**
** @alias EnsSPredictiontranscript
** @alias EnsOPredictiontranscript
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                 Transcript Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::PredictionTranscript
** @attr Displaylabel [AjPStr] Display label
** @attr Predictionexons [AjPList] AJAX List of Ensembl Prediction Exon objects
** @@
******************************************************************************/

typedef struct EnsSPredictiontranscript
{
    ajuint Use;
    ajuint Identifier;
    EnsPPredictiontranscriptadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr Displaylabel;
    AjPList Predictionexons;
} EnsOPredictiontranscript;

#define EnsPPredictiontranscript EnsOPredictiontranscript*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Prediction Exon */

EnsPPredictionexon ensPredictionexonNewCpy(const EnsPPredictionexon pe);

EnsPPredictionexon ensPredictionexonNewIni(EnsPPredictionexonadaptor pea,
                                           ajuint identifier,
                                           EnsPFeature feature,
                                           ajint sphase,
                                           double score,
                                           double pvalue);

EnsPPredictionexon ensPredictionexonNewRef(EnsPPredictionexon pe);

void ensPredictionexonDel(EnsPPredictionexon *Ppe);

EnsPPredictionexonadaptor ensPredictionexonGetAdaptor(
    const EnsPPredictionexon pe);

EnsPFeature ensPredictionexonGetFeature(const EnsPPredictionexon pe);

ajuint ensPredictionexonGetIdentifier(const EnsPPredictionexon pe);

ajint ensPredictionexonGetPhaseStart(const EnsPPredictionexon pe);

double ensPredictionexonGetPvalue(const EnsPPredictionexon pe);

double ensPredictionexonGetScore(const EnsPPredictionexon pe);

AjBool ensPredictionexonSetAdaptor(EnsPPredictionexon pe,
                                   EnsPPredictionexonadaptor pea);

AjBool ensPredictionexonSetFeature(EnsPPredictionexon pe,
                                   EnsPFeature feature);

AjBool ensPredictionexonSetIdentifier(EnsPPredictionexon pe,
                                      ajuint identifier);

AjBool ensPredictionexonSetPhaseStart(EnsPPredictionexon pe,
                                      ajint sphase);

AjBool ensPredictionexonSetPvalue(EnsPPredictionexon pe,
                                  double pvalue);

AjBool ensPredictionexonSetScore(EnsPPredictionexon pe,
                                 double score);

AjBool ensPredictionexonTrace(const EnsPPredictionexon pe, ajuint level);

ajint ensPredictionexonGetPhaseEnd(const EnsPPredictionexon pe);

size_t ensPredictionexonCalculateMemsize(const EnsPPredictionexon pe);

EnsPPredictionexon ensPredictionexonTransfer(EnsPPredictionexon pe,
                                             EnsPSlice slice);

EnsPPredictionexon ensPredictionexonTransform(EnsPPredictionexon pe,
                                              const AjPStr csname,
                                              const AjPStr csversion);

AjBool ensPredictionexonFetchSequenceSeq(EnsPPredictionexon pe,
                                         AjPSeq *Psequence);

AjBool ensPredictionexonFetchSequenceStr(EnsPPredictionexon pe,
                                         AjPStr *Psequence);

/* AJAX List of Ensembl Prediction Exon objects */

AjBool ensListPredictionexonSortEndAscending(AjPList pes);

AjBool ensListPredictionexonSortEndDescending(AjPList pes);

AjBool ensListPredictionexonSortIdentifierAscending(AjPList pes);

AjBool ensListPredictionexonSortStartAscending(AjPList pes);

AjBool ensListPredictionexonSortStartDescending(AjPList pes);

/* Ensembl Prediction Exon Adaptor */

EnsPPredictionexonadaptor ensRegistryGetPredictionexonadaptor(
    EnsPDatabaseadaptor dba);

EnsPPredictionexonadaptor ensPredictionexonadaptorNew(
    EnsPDatabaseadaptor dba);

void ensPredictionexonadaptorDel(EnsPPredictionexonadaptor *Ppea);

EnsPDatabaseadaptor ensPredictionexonadaptorGetDatabaseadaptor(
    EnsPPredictionexonadaptor pea);

EnsPFeatureadaptor ensPredictionexonadaptorGetFeatureadaptor(
    EnsPPredictionexonadaptor pea);

AjBool ensPredictionexonadaptorFetchAllbyPredictiontranscript(
    EnsPPredictionexonadaptor pea,
    const EnsPPredictiontranscript pt,
    AjPList pes);

/* Ensembl Prediction Transcript */

EnsPPredictiontranscript ensPredictiontranscriptNewCpy(
    const EnsPPredictiontranscript pt);

EnsPPredictiontranscript ensPredictiontranscriptNewIni(
    EnsPPredictiontranscriptadaptor pta,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr label);

EnsPPredictiontranscript ensPredictiontranscriptNewRef(
    EnsPPredictiontranscript pt);

void ensPredictiontranscriptDel(
    EnsPPredictiontranscript *Ppt);

EnsPPredictiontranscriptadaptor ensPredictiontranscriptGetAdaptor(
    const EnsPPredictiontranscript pt);

AjPStr ensPredictiontranscriptGetDisplaylabel(
    const EnsPPredictiontranscript pt);

EnsPFeature ensPredictiontranscriptGetFeature(
    const EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetIdentifier(
    const EnsPPredictiontranscript pt);

const AjPList ensPredictiontranscriptLoadPredictionexons(
    EnsPPredictiontranscript pt);

AjBool ensPredictiontranscriptSetAdaptor(
    EnsPPredictiontranscript pt,
    EnsPPredictiontranscriptadaptor pta);

AjBool ensPredictiontranscriptSetDisplaylabel(
    EnsPPredictiontranscript pt,
    AjPStr label);

AjBool ensPredictiontranscriptSetFeature(
    EnsPPredictiontranscript pt,
    EnsPFeature feature);

AjBool ensPredictiontranscriptSetIdentifier(
    EnsPPredictiontranscript pt,
    ajuint identifier);

AjBool ensPredictiontranscriptTrace(
    const EnsPPredictiontranscript pt,
    ajuint level);

size_t ensPredictiontranscriptCalculateMemsize(
    const EnsPPredictiontranscript pt);

ajint ensPredictiontranscriptCalculateSliceCodingEnd(
    const EnsPPredictiontranscript pt);

ajint ensPredictiontranscriptCalculateSliceCodingStart(
    const EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptCalculateTranscriptCodingEnd(
    EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptCalculateTranscriptCodingStart(
    const EnsPPredictiontranscript pt);

AjBool ensPredictiontranscriptFetchSequenceTranscriptSeq(
    EnsPPredictiontranscript pt,
    AjPSeq *Psequence);

AjBool ensPredictiontranscriptFetchSequenceTranscriptStr(
    EnsPPredictiontranscript pt,
    AjPStr *Psequence);

AjBool ensPredictiontranscriptFetchSequenceTranslationSeq(
    EnsPPredictiontranscript pt,
    AjPSeq *Psequence);

AjBool ensPredictiontranscriptFetchSequenceTranslationStr(
    EnsPPredictiontranscript pt,
    AjPStr *Psequence);

EnsPPredictiontranscript ensPredictiontranscriptTransfer(
    EnsPPredictiontranscript pt,
    EnsPSlice slice);

EnsPPredictiontranscript ensPredictiontranscriptTransform(
    EnsPPredictiontranscript pt,
    const AjPStr csname,
    const AjPStr csversion);

/* AJAX List of Ensembl Prediction Transcript objects */

AjBool ensListPredictiontranscriptSortEndAscending(AjPList pts);

AjBool ensListPredictiontranscriptSortEndDescending(AjPList pts);

AjBool ensListPredictiontranscriptSortIdentifierAscending(AjPList pts);

AjBool ensListPredictiontranscriptSortStartAscending(AjPList pts);

AjBool ensListPredictiontranscriptSortStartDescending(AjPList pts);

/* Ensembl Prediction Transcript Adaptor */

EnsPPredictiontranscriptadaptor ensRegistryGetPredictiontranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPPredictiontranscriptadaptor ensPredictiontranscriptadaptorNew(
    EnsPDatabaseadaptor dba);

void ensPredictiontranscriptadaptorDel(
    EnsPPredictiontranscriptadaptor *Ppta);

EnsPDatabaseadaptor ensPredictiontranscriptadaptorGetDatabaseadaptor(
    EnsPPredictiontranscriptadaptor pta);

EnsPFeatureadaptor ensPredictiontranscriptadaptorGetFeatureadaptor(
    EnsPPredictiontranscriptadaptor pta);

AjBool ensPredictiontranscriptadaptorFetchByIdentifier(
    EnsPPredictiontranscriptadaptor pta,
    ajuint identifier,
    EnsPPredictiontranscript *Ppt);

AjBool ensPredictiontranscriptadaptorFetchByStableidentifier(
    EnsPPredictiontranscriptadaptor pta,
    const AjPStr stableid,
    EnsPPredictiontranscript *Ppt);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSPREDICTION_H */

/* @include ensexon ***********************************************************
**
** Ensembl Exon functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.26 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 10:45:57 $ by $Author: mks $
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

#ifndef ENSEXON_H
#define ENSEXON_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPExoncoordinates **************************************************
**
** Ensembl Exon Coordinates.
**
** Holds Ensembl Exon coordinates relative to a particular associated
** Ensembl Transcript, as well as coding region coordinates relative to an
** Ensembl Slice and a particular associated Translation.
**
** @alias EnsSExoncoordinates
** @alias EnsOExoncoordinates
**
** @attr TranscriptStart [ajuint] Exon start on Transcript
** @attr TranscriptEnd [ajuint] Exon end on Transcript
** @attr TranscriptCodingStart [ajuint] Coding region start on Transcript
** @attr TranscriptCodingEnd [ajuint] Coding region end on Transcript
** @attr SliceCodingStart [ajint] Coding region start on Slice
** @attr SliceCodingEnd [ajint] Coding region end on Slice
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSExoncoordinates
{
    ajuint TranscriptStart;
    ajuint TranscriptEnd;
    ajuint TranscriptCodingStart;
    ajuint TranscriptCodingEnd;
    ajint SliceCodingStart;
    ajint SliceCodingEnd;
    ajuint Use;
    ajuint Padding;
} EnsOExoncoordinates;

#define EnsPExoncoordinates EnsOExoncoordinates*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Excon Coordinates */

EnsPExoncoordinates ensExoncoordinatesNewCpy(const EnsPExoncoordinates ec);

EnsPExoncoordinates ensExoncoordinatesNewIni(EnsPExon exon,
                                             EnsPTranscript transcript,
                                             EnsPTranslation translation);

EnsPExoncoordinates ensExoncoordinatesNewRef(EnsPExoncoordinates ec);

void ensExoncoordinatesDel(EnsPExoncoordinates *Pec);

ajint ensExoncoordinatesGetSliceCodingEnd(
    const EnsPExoncoordinates ec);

ajint ensExoncoordinatesGetSliceCodingStart(
    const EnsPExoncoordinates ec);

ajuint ensExoncoordinatesGetTranscriptCodingEnd(
    const EnsPExoncoordinates ec);

ajuint ensExoncoordinatesGetTranscriptCodingStart(
    const EnsPExoncoordinates ec);

ajuint ensExoncoordinatesGetTranscriptEnd(
    const EnsPExoncoordinates ec);

ajuint ensExoncoordinatesGetTranscriptStart(
    const EnsPExoncoordinates ec);

AjBool ensExoncoordinatesTrace(const EnsPExoncoordinates ec, ajuint level);

size_t ensExoncoordinatesCalculateMemsize(const EnsPExoncoordinates ec);

/* Ensembl Exon */

EnsPExon ensExonNewCpy(const EnsPExon object);

EnsPExon ensExonNewIni(EnsPExonadaptor ea,
                       ajuint identifier,
                       EnsPFeature feature,
                       ajint sphase,
                       ajint ephase,
                       AjBool current,
                       AjBool constitutive,
                       AjPStr stableid,
                       ajuint version,
                       AjPStr cdate,
                       AjPStr mdate);

EnsPExon ensExonNewRef(EnsPExon exon);

void ensExonDel(EnsPExon *Pexon);

EnsPExonadaptor ensExonGetAdaptor(const EnsPExon exon);

AjBool ensExonGetConstitutive(const EnsPExon exon);

AjBool ensExonGetCurrent(const EnsPExon exon);

AjPStr ensExonGetDateCreation(const EnsPExon exon);

AjPStr ensExonGetDateModification(const EnsPExon exon);

EnsPFeature ensExonGetFeature(const EnsPExon exon);

ajuint ensExonGetIdentifier(const EnsPExon exon);

ajint ensExonGetPhaseEnd(const EnsPExon exon);

ajint ensExonGetPhaseStart(const EnsPExon exon);

AjPStr ensExonGetStableidentifier(const EnsPExon exon);

ajuint ensExonGetVersion(const EnsPExon exon);

const AjPList ensExonLoadSupportingfeatures(EnsPExon exon);

AjBool ensExonSetAdaptor(EnsPExon exon, EnsPExonadaptor ea);

AjBool ensExonSetConstitutive(EnsPExon exon, AjBool constitutive);

AjBool ensExonSetCurrent(EnsPExon exon, AjBool current);

AjBool ensExonSetDateCreation(EnsPExon exon, AjPStr cdate);

AjBool ensExonSetDateModification(EnsPExon exon, AjPStr mdate);

AjBool ensExonSetFeature(EnsPExon exon, EnsPFeature feature);

AjBool ensExonSetIdentifier(EnsPExon exon, ajuint identifier);

AjBool ensExonSetPhaseEnd(EnsPExon exon, ajint ephase);

AjBool ensExonSetPhaseStart(EnsPExon exon, ajint sphase);

AjBool ensExonSetStableidentifier(EnsPExon exon, AjPStr stableid);

AjBool ensExonSetVersion(EnsPExon exon, ajuint version);

AjBool ensExonTrace(const EnsPExon exon, ajuint level);

ajint ensExonCalculateFrame(const EnsPExon exon);

size_t ensExonCalculateMemsize(const EnsPExon exon);

ajint ensExonCalculateSliceCodingEnd(EnsPExon exon,
                                     EnsPTranscript transcript,
                                     EnsPTranslation translation);

ajint ensExonCalculateSliceCodingStart(EnsPExon exon,
                                       EnsPTranscript transcript,
                                       EnsPTranslation translation);

ajuint ensExonCalculateTranscriptCodingEnd(EnsPExon exon,
                                           EnsPTranscript transcript,
                                           EnsPTranslation translation);

ajuint ensExonCalculateTranscriptCodingStart(EnsPExon exon,
                                             EnsPTranscript transcript,
                                             EnsPTranslation translation);

ajuint ensExonCalculateTranscriptStart(EnsPExon exon,
                                       EnsPTranscript transcript);

ajuint ensExonCalculateTranscriptEnd(EnsPExon exon,
                                     EnsPTranscript transcript);

EnsPExon ensExonTransfer(EnsPExon exon, EnsPSlice slice);

EnsPExon ensExonTransform(EnsPExon exon,
                          const AjPStr csname,
                          const AjPStr csversion);

AjBool ensExonFetchDisplayidentifier(const EnsPExon exon, AjPStr *Pidentifier);

AjBool ensExonFetchSequenceSliceSeq(EnsPExon exon, AjPSeq *Psequence);

AjBool ensExonFetchSequenceSliceStr(EnsPExon exon, AjPStr *Psequence);

AjBool ensExonFetchSequenceTranslationSeq(EnsPExon exon,
                                          EnsPTranscript transcript,
                                          EnsPTranslation translation,
                                          AjPSeq *Psequence);

AjBool ensExonFetchSequenceTranslationStr(EnsPExon exon,
                                          EnsPTranscript transcript,
                                          EnsPTranslation translation,
                                          AjPStr *Psequence);

AjBool ensExonMatch(const EnsPExon exon1, const EnsPExon exon2);

AjBool ensExonSimilarity(const EnsPExon exon1, const EnsPExon exon2);

/* AJAX List of Ensembl Exon objects */

AjBool ensListExonSortEndAscending(AjPList exons);

AjBool ensListExonSortEndDescending(AjPList exons);

AjBool ensListExonSortIdentifierAscending(AjPList exons);

AjBool ensListExonSortStartAscending(AjPList exons);

AjBool ensListExonSortStartDescending(AjPList exons);

AjBool ensSequenceAddFeatureExon(AjPSeq seq,
                                 EnsPExon exon,
                                 ajint rank,
                                 AjPFeature *Pfeature);

/* Ensembl Exon Adaptor */

EnsPExonadaptor ensRegistryGetExonadaptor(
    EnsPDatabaseadaptor dba);

EnsPExonadaptor ensExonadaptorNew(
    EnsPDatabaseadaptor dba);

void ensExonadaptorDel(EnsPExonadaptor *Pea);

EnsPBaseadaptor ensExonadaptorGetBaseadaptor(EnsPExonadaptor ea);

EnsPDatabaseadaptor ensExonadaptorGetDatabaseadaptor(EnsPExonadaptor ea);

EnsPFeatureadaptor ensExonadaptorGetFeatureadaptor(EnsPExonadaptor ea);

AjBool ensExonadaptorFetchAll(EnsPExonadaptor ea,
                              AjPList exons);

AjBool ensExonadaptorFetchAllbySlice(EnsPExonadaptor ea,
                                     EnsPSlice slice,
                                     const AjPStr constraint,
                                     AjPList exons);

AjBool ensExonadaptorFetchAllbyStableidentifier(EnsPExonadaptor ea,
                                                const AjPStr stableid,
                                                AjPList exons);

AjBool ensExonadaptorFetchAllbyTranscript(EnsPExonadaptor ea,
                                          const EnsPTranscript transcript,
                                          AjPList exons);

AjBool ensExonadaptorFetchByIdentifier(EnsPExonadaptor ea,
                                       ajuint identifier,
                                       EnsPExon *Pexon);

AjBool ensExonadaptorFetchByStableidentifier(EnsPExonadaptor ea,
                                             const AjPStr stableid,
                                             ajuint version,
                                             EnsPExon *Pexon);

AjBool ensExonadaptorRetrieveAllIdentifiers(EnsPExonadaptor ea,
                                            AjPList identifiers);

AjBool ensExonadaptorRetrieveAllStableidentifiers(EnsPExonadaptor ea,
                                                  AjPList stableids);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSEXON_H */

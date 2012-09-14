/* @include ensalign **********************************************************
**
** Ensembl Alignment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.10 $
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

#ifndef ENSALIGN_H
#define ENSALIGN_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPDnaalignfeatureadaptor ******************************************
**
** Ensembl DNA Align Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::DnaAlignFeatureAdaptor
** ##
******************************************************************************/

#define EnsPDnaalignfeatureadaptor EnsPFeatureadaptor




/* @const EnsPProteinalignfeatureadaptor **************************************
**
** Ensembl Protein Align Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::ProteinAlignFeatureAdaptor
** ##
******************************************************************************/

#define EnsPProteinalignfeatureadaptor EnsPFeatureadaptor




/* @enum EnsEBasealignfeatureType *********************************************
**
** Ensembl Base Align Feature Type enumeration
**
** @value ensEBasealignfeatureTypeNULL Null
** @value ensEBasealignfeatureTypeDNA DNA alignment
** @value ensEBasealignfeatureTypeProtein Protein alignment
** @@
******************************************************************************/

typedef enum EnsOBasealignfeatureType
{
    ensEBasealignfeatureTypeNULL,
    ensEBasealignfeatureTypeDNA,
    ensEBasealignfeatureTypeProtein
} EnsEBasealignfeatureType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPBasealignfeature *************************************************
**
** Ensembl Base Align Feature.
**
** @alias EnsSBasealignfeature
** @alias EnsOBasealignfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Dnaalignfeatureadaptor [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @attr Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @cc Bio::EnsEMBL::Featurepair
** @attr Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::Basealignfeature
** @attr FobjectGetFeaturepair [EnsPFeaturepair function]
** Ensembl Object Get Ensembl Feature Pair function
** @attr Cigar [AjPStr] CIGAR line
** @attr Type [EnsEBasealignfeatureType] Type
** @attr Alignmentlength [ajuint] Target component alignment length
** @attr Pairdnaalignfeatureidentifier [ajuint]
** Pair DNA Align Feature identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSBasealignfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPDnaalignfeatureadaptor Dnaalignfeatureadaptor;
    EnsPProteinalignfeatureadaptor Proteinalignfeatureadaptor;
    EnsPFeaturepair Featurepair;
    EnsPFeaturepair (*FobjectGetFeaturepair) (const void *object);
    AjPStr Cigar;
    EnsEBasealignfeatureType Type;
    ajuint Alignmentlength;
    ajuint Pairdnaalignfeatureidentifier;
    ajuint Padding;
} EnsOBasealignfeature;

#define EnsPBasealignfeature EnsOBasealignfeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Base Align Feature */

EnsPBasealignfeature ensBasealignfeatureNewCpy(
    const EnsPBasealignfeature baf);

EnsPBasealignfeature ensBasealignfeatureNewFps(
    EnsPFeaturepair fp,
    AjPStr cigar,
    AjPList fps,
    EnsEBasealignfeatureType type,
    ajuint pair);

EnsPBasealignfeature ensBasealignfeatureNewIniD(
    EnsPDnaalignfeatureadaptor dafa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar,
    ajuint pair);

EnsPBasealignfeature ensBasealignfeatureNewIniP(
    EnsPProteinalignfeatureadaptor pafa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar);

EnsPBasealignfeature ensBasealignfeatureNewRef(EnsPBasealignfeature baf);

void ensBasealignfeatureDel(EnsPBasealignfeature *Pbaf);

AjPStr ensBasealignfeatureGetCigar(
    const EnsPBasealignfeature baf);

EnsPDnaalignfeatureadaptor ensBasealignfeatureGetDnaalignfeatureadaptor(
    const EnsPBasealignfeature baf);

EnsPFeaturepair ensBasealignfeatureGetFeaturepair(
    const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetIdentifier(
    const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetPairdnaalignfeatureidentifier(
    const EnsPBasealignfeature baf);

EnsPProteinalignfeatureadaptor ensBasealignfeatureGetProteinalignfeatureadaptor(
    const EnsPBasealignfeature baf);

EnsEBasealignfeatureType ensBasealignfeatureGetType(
    const EnsPBasealignfeature baf);

AjBool ensBasealignfeatureSetFeaturepair(EnsPBasealignfeature baf,
                                         EnsPFeaturepair fp);

EnsPFeature ensBasealignfeatureGetFeature(const EnsPBasealignfeature baf);

void *ensBasealignfeatureCalculateAdaptor(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureCalculateAlignmentlength(EnsPBasealignfeature baf);

size_t ensBasealignfeatureCalculateMemsize(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureCalculateUnitSource(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureCalculateUnitTarget(const EnsPBasealignfeature baf);

EnsPBasealignfeature ensBasealignfeatureTransfer(
    EnsPBasealignfeature baf,
    EnsPSlice slice);

EnsPBasealignfeature ensBasealignfeatureTransform(
    EnsPBasealignfeature baf,
    const AjPStr csname,
    const AjPStr csversion);

AjBool ensBasealignfeatureTrace(const EnsPBasealignfeature baf, ajuint level);

AjBool ensBasealignfeatureFetchAllFeaturepairs(const EnsPBasealignfeature baf,
                                               AjPList fps);

/* AJAX List of Ensembl Base Align Feature objects */

AjBool ensListBasealignfeatureSortSourceEndAscending(AjPList bafs);

AjBool ensListBasealignfeatureSortSourceEndDescending(AjPList bafs);

AjBool ensListBasealignfeatureSortSourceStartAscending(AjPList bafs);

AjBool ensListBasealignfeatureSortSourceStartDescending(AjPList bafs);

/* Ensembl DNA Align Feature Adaptor */

EnsPDnaalignfeatureadaptor ensRegistryGetDnaalignfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPDnaalignfeatureadaptor ensDnaalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDnaalignfeatureadaptorDel(EnsPDnaalignfeatureadaptor *Pdafa);

EnsPDatabaseadaptor ensDnaalignfeatureadaptorGetDatabaseadaptor(
    EnsPDnaalignfeatureadaptor dafa);

AjBool ensDnaalignfeatureadaptorFetchAllbyHitname(
    EnsPDnaalignfeatureadaptor dafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDnaalignfeatureadaptorFetchAllbyHitunversioned(
    EnsPDnaalignfeatureadaptor dafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDnaalignfeatureadaptorFetchAllbySlicecoverage(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDnaalignfeatureadaptorFetchAllbySliceidentity(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDnaalignfeatureadaptorFetchAllbySliceexternaldatabasename(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDnaalignfeatureadaptorFetchByIdentifier(
    EnsPDnaalignfeatureadaptor dafa,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf);

AjBool ensDnaalignfeatureadaptorRetrieveAllIdentifiers(
    EnsPDnaalignfeatureadaptor dafa,
    AjPList identifiers);

/* Ensembl Protein Align Feature Adaptor */

EnsPProteinalignfeatureadaptor ensRegistryGetProteinalignfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPProteinalignfeatureadaptor ensProteinalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensProteinalignfeatureadaptorDel(EnsPProteinalignfeatureadaptor *Ppafa);

EnsPDatabaseadaptor ensProteinalignfeatureadaptorGetDatabaseadaptor(
    EnsPProteinalignfeatureadaptor pafa);

AjBool ensProteinalignfeatureadaptorFetchAllbyHitname(
    EnsPProteinalignfeatureadaptor pafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllbyHitunversioned(
    EnsPProteinalignfeatureadaptor pafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllbySlicecoverage(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllbySliceexternaldatabasename(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllbySliceidentity(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchByIdentifier(
    EnsPProteinalignfeatureadaptor pafa,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf);

AjBool ensProteinalignfeatureadaptorRetrieveAllIdentifiers(
    EnsPProteinalignfeatureadaptor pafa,
    AjPList identifiers);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSALIGN_H */

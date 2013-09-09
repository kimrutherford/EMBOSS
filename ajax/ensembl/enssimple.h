/* @include enssimple *********************************************************
**
** Ensembl Simple Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.8 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:31:15 $ by $Author: mks $
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

#ifndef ENSSIMPLE_H
#define ENSSIMPLE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPSimplefeatureadaptor ********************************************
**
** Ensembl Simple Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
**
** #cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor
** ##
******************************************************************************/

#define EnsPSimplefeatureadaptor EnsPFeatureadaptor




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPSimplefeature ****************************************************
**
** Ensembl Simple Feature.
**
** @alias EnsSSimplefeature
** @alias EnsOSimplefeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Simplefeature
** @attr Displaylabel [AjPStr] Display label
** @attr Score [double] Score
** @@
******************************************************************************/

typedef struct EnsSSimplefeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPSimplefeatureadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr Displaylabel;
    double Score;
} EnsOSimplefeature;

#define EnsPSimplefeature EnsOSimplefeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Simple Feature */

EnsPSimplefeature ensSimplefeatureNewCpy(const EnsPSimplefeature object);

EnsPSimplefeature ensSimplefeatureNewIni(EnsPSimplefeatureadaptor pfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         AjPStr label,
                                         double score);

EnsPSimplefeature ensSimplefeatureNewRef(EnsPSimplefeature sf);

void ensSimplefeatureDel(EnsPSimplefeature *Psf);

EnsPSimplefeatureadaptor ensSimplefeatureGetAdaptor(
    const EnsPSimplefeature sf);

AjPStr ensSimplefeatureGetDisplaylabel(
    const EnsPSimplefeature sf);

EnsPFeature ensSimplefeatureGetFeature(
    const EnsPSimplefeature sf);

ajuint ensSimplefeatureGetIdentifier(
    const EnsPSimplefeature sf);

double ensSimplefeatureGetScore(
    const EnsPSimplefeature sf);

AjBool ensSimplefeatureSetAdaptor(EnsPSimplefeature sf,
                                  EnsPSimplefeatureadaptor sfa);

AjBool ensSimplefeatureSetDisplaylabel(EnsPSimplefeature sf, AjPStr label);

AjBool ensSimplefeatureSetFeature(EnsPSimplefeature sf, EnsPFeature feature);

AjBool ensSimplefeatureSetIdentifier(EnsPSimplefeature sf, ajuint identifier);

AjBool ensSimplefeatureSetScore(EnsPSimplefeature sf, double score);

AjBool ensSimplefeatureTrace(const EnsPSimplefeature sf, ajuint level);

size_t ensSimplefeatureCalculateMemsize(const EnsPSimplefeature sf);

/* AJAX List of Ensembl Simple Feature objects */

AjBool ensListSimplefeatureSortEndAscending(AjPList sfs);

AjBool ensListSimplefeatureSortEndDescending(AjPList sfs);

AjBool ensListSimplefeatureSortIdentifierAscending(AjPList sfs);

AjBool ensListSimplefeatureSortStartAscending(AjPList sfs);

AjBool ensListSimplefeatureSortStartDescending(AjPList sfs);

/* Ensembl Simple Feature Adaptor */

EnsPSimplefeatureadaptor ensRegistryGetSimplefeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPSimplefeatureadaptor ensSimplefeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSimplefeatureadaptorDel(EnsPSimplefeatureadaptor *Psfa);

EnsPBaseadaptor ensSimplefeatureadaptorGetBaseadaptor(
    EnsPSimplefeatureadaptor sfa);

EnsPDatabaseadaptor ensSimplefeatureadaptorGetDatabaseadaptor(
    EnsPSimplefeatureadaptor sfa);

EnsPFeatureadaptor ensSimplefeatureadaptorGetFeatureadaptor(
    EnsPSimplefeatureadaptor sfa);

AjBool ensSimplefeatureadaptorFetchAllbyAnalysisname(
    EnsPSimplefeatureadaptor sfa,
    const AjPStr anname,
    AjPList sfs);

AjBool ensSimplefeatureadaptorFetchAllbySlice(EnsPSimplefeatureadaptor sfa,
                                              EnsPSlice slice,
                                              const AjPStr anname,
                                              AjPList sfs);

AjBool ensSimplefeatureadaptorFetchAllbySlicescore(
    EnsPSimplefeatureadaptor sfa,
    EnsPSlice slice,
    double score,
    const AjPStr anname,
    AjPList sfs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSIMPLE_H */

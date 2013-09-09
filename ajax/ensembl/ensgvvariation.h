/* @include ensgvvariation ****************************************************
**
** Ensembl Genetic Variation Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.15 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:03:26 $ by $Author: mks $
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

#ifndef ENSGVVARIATION_H
#define ENSGVVARIATION_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensgvdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Variation */

EnsPGvvariation ensGvvariationNewCpy(const EnsPGvvariation gvv);

EnsPGvvariation ensGvvariationNewIni(EnsPGvvariationadaptor gvva,
                                     ajuint identifier,
                                     EnsPGvattribute gvaclass,
                                     EnsPGvattribute gvaclinical,
                                     EnsPGvsource gvsource,
                                     AjPStr name,
                                     AjPStr ancestralallele,
                                     AjPList gvalleles,
                                     AjPList gvsynonyms,
                                     AjPStr validationstates,
                                     AjPStr moltype,
                                     AjPStr flankfive,
                                     AjPStr flankthree,
                                     AjBool flankexists,
                                     AjBool flipped,
                                     AjBool somatic,
                                     AjPStr maallele,
                                     ajuint macount,
                                     float mafrequency);

EnsPGvvariation ensGvvariationNewRef(EnsPGvvariation gvv);

void ensGvvariationDel(EnsPGvvariation *Pgvv);

EnsPGvvariationadaptor ensGvvariationGetAdaptor(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetAncestralallele(const EnsPGvvariation gvv);

EnsPGvattribute ensGvvariationGetClass(const EnsPGvvariation gvv);

EnsPGvattribute ensGvvariationGetClinical(const EnsPGvvariation gvv);

AjBool ensGvvariationGetFlankExists(const EnsPGvvariation gvv);

AjBool ensGvvariationGetFlipped(const EnsPGvvariation gvv);

const AjPList ensGvvariationGetGvalleles(const EnsPGvvariation gvv);

EnsPGvsource ensGvvariationGetGvsource(const EnsPGvvariation gvv);

const AjPList ensGvvariationGetGvsynonyms(const EnsPGvvariation gvv);

ajuint ensGvvariationGetIdentifier(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetMinoralleleAllele(const EnsPGvvariation gvv);

ajuint ensGvvariationGetMinoralleleCount(const EnsPGvvariation gvv);

float ensGvvariationGetMinoralleleFrequency(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetMoleculetype(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetName(const EnsPGvvariation gvv);

AjBool ensGvvariationGetSomatic(const EnsPGvvariation gvv);

ajuint ensGvvariationGetValidations(const EnsPGvvariation gvv);

const AjPList ensGvvariationLoadAllFaileddescriptions(EnsPGvvariation gvv);

const AjPList ensGvvariationLoadAllGvalleles(EnsPGvvariation gvv);

AjPStr ensGvvariationLoadFlankFive(EnsPGvvariation gvv);

AjPStr ensGvvariationLoadFlankThree(EnsPGvvariation gvv);

AjBool ensGvvariationSetAdaptor(EnsPGvvariation gvv,
                                EnsPGvvariationadaptor gvva);

AjBool ensGvvariationSetAncestralallele(EnsPGvvariation gvv,
                                        AjPStr ancestralallele);

AjBool ensGvvariationSetClass(EnsPGvvariation gvv,
                              EnsPGvattribute gvaclass);

AjBool ensGvvariationSetClinical(EnsPGvvariation gvv,
                                 EnsPGvattribute gvaclinical);

AjBool ensGvvariationSetFlankExists(EnsPGvvariation gvv,
                                    AjBool flankexists);

AjBool ensGvvariationSetFlankFive(EnsPGvvariation gvv,
                                  AjPStr flankfive);

AjBool ensGvvariationSetFlankThree(EnsPGvvariation gvv,
                                   AjPStr flankthree);

AjBool ensGvvariationSetFlipped(EnsPGvvariation gvv,
                                AjBool flipped);

AjBool ensGvvariationSetGvsource(EnsPGvvariation gvv,
                                 EnsPGvsource gvs);

AjBool ensGvvariationSetIdentifier(EnsPGvvariation gvv,
                                   ajuint identifier);

AjBool ensGvvariationSetMinoralleleAllele(EnsPGvvariation gvv,
                                          AjPStr maallele);

AjBool ensGvvariationSetMinoralleleCount(EnsPGvvariation gvv,
                                         ajuint macount);

AjBool ensGvvariationSetMinoralleleFrequency(EnsPGvvariation gvv,
                                             float mafrequency);

AjBool ensGvvariationSetMoleculetype(EnsPGvvariation gvv,
                                     AjPStr moltype);

AjBool ensGvvariationSetName(EnsPGvvariation gvv,
                             AjPStr name);

AjBool ensGvvariationSetSomatic(EnsPGvvariation gvv,
                                AjBool somatic);

AjBool ensGvvariationTrace(const EnsPGvvariation gvv, ajuint level);

size_t ensGvvariationCalculateMemsize(const EnsPGvvariation gvv);

AjBool ensGvvariationAddFaileddescription(EnsPGvvariation gvv,
                                          AjPStr description);

AjBool ensGvvariationAddGvallele(EnsPGvvariation gvv,
                                 EnsPGvallele gva);

AjBool ensGvvariationAddGvsynonym(EnsPGvvariation gvv,
                                  EnsPGvsynonym gvs);

AjBool ensGvvariationAddValidation(EnsPGvvariation gvv,
                                   EnsEGvvariationValidation gvvv);

AjBool ensGvvariationFetchAllGenes(EnsPGvvariation gvv,
                                   ajint flank,
                                   AjPList genes);

AjBool ensGvvariationFetchAllGvsources(const EnsPGvvariation gvv,
                                       AjPList gvss);

AjBool ensGvvariationFetchAllGvsynonyms(const EnsPGvvariation gvv,
                                        const AjPStr gvsourcename,
                                        AjPList gvss);

AjBool ensGvvariationFetchAllGvvariationfeatures(EnsPGvvariation gvv,
                                                 AjPList gvvfs);

AjBool ensGvvariationHasFailedgvalleles(EnsPGvvariation gvv, AjBool *Presult);

AjBool ensGvvariationIsFailed(EnsPGvvariation gvv, AjBool *Presult);

EnsEGvvariationClass ensGvvariationClassFromStr(
    const AjPStr class);

const char *ensGvvariationClassToChar(
    EnsEGvvariationClass gvvc);

EnsEGvvariationValidation ensGvvariationValidationFromStr(
    const AjPStr validation);

const char *ensGvvariationValidationToChar(
    EnsEGvvariationValidation gvvv);

ajuint ensGvvariationValidationsFromSet(const AjPStr gvvvset);

AjBool ensGvvariationValidationsToSet(ajuint gvvvbf, AjPStr *Pgvvvset);

/* Ensembl Genetic Variation Variation Adaptor */

EnsPGvvariationadaptor ensRegistryGetGvvariationadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvvariationadaptor ensGvvariationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvvariationadaptorDel(EnsPGvvariationadaptor *Pgvva);

EnsPBaseadaptor ensGvvariationadaptorGetBaseadaptor(
    EnsPGvvariationadaptor gvva);

EnsPDatabaseadaptor ensGvvariationadaptorGetDatabaseadaptor(
    EnsPGvvariationadaptor gvva);

EnsPGvbaseadaptor ensGvvariationadaptorGetGvbaseadaptor(
    EnsPGvvariationadaptor gvva);

EnsPGvdatabaseadaptor ensGvvariationadaptorGetGvdatabaseadaptor(
    EnsPGvvariationadaptor gvva);

AjBool ensGvvariationadaptorFetchAll(EnsPGvvariationadaptor gvva,
                                     AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllbyGvpopulation(
    EnsPGvvariationadaptor gvva,
    const EnsPGvpopulation gvp,
    AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllbyGvpopulationfrequency(
    EnsPGvvariationadaptor gvva,
    const EnsPGvpopulation gvp,
    float frequency,
    AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllbyGvsourceType(
    EnsPGvvariationadaptor gvva,
    EnsEGvsourceType gvst,
    AjBool primary,
    AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllbyGvvariationset(
    EnsPGvvariationadaptor gvva,
    EnsPGvvariationset gvvset,
    AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllbyIdentifiers(
    EnsPGvvariationadaptor gvva,
    AjPTable gvvs);

AjBool ensGvvariationadaptorFetchAllbyNames(
    EnsPGvvariationadaptor gvva,
    AjPTable gvvs);

AjBool ensGvvariationadaptorFetchAllbySource(
    EnsPGvvariationadaptor gvva,
    const AjPStr source,
    AjBool primary,
    AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllsomatic(EnsPGvvariationadaptor gvva,
                                            AjPList gvvs);

AjBool ensGvvariationadaptorFetchByIdentifier(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    EnsPGvvariation *Pgvv);

AjBool ensGvvariationadaptorFetchByName(
    EnsPGvvariationadaptor gvva,
    const AjPStr name,
    const AjPStr source,
    EnsPGvvariation *Pgvv);

AjBool ensGvvariationadaptorFetchBySubidentifier(
    EnsPGvvariationadaptor gvva,
    const AjPStr name,
    EnsPGvvariation *Pgvv);

AjBool ensGvvariationadaptorFetchBySynonym(
    EnsPGvvariationadaptor gvva,
    const AjPStr synonym,
    const AjPStr source,
    EnsPGvvariation *Pgvv);

AjBool ensGvvariationadaptorRetrieveAllFaileddescriptions(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    AjPList fvs);

AjBool ensGvvariationadaptorRetrieveFlank(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    AjPStr *Pfiveseq,
    AjPStr *Pthreeseq);

/* Ensembl Genetic Variation Variation Feature */

EnsPGvvariationfeature ensGvvariationfeatureNewCpy(
    const EnsPGvvariationfeature gvvf);

EnsPGvvariationfeature ensGvvariationfeatureNewIdentifier(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPGvsource gvsource,
    EnsEGvvariationClass gvvc,
    ajuint gvvidentifier,
    AjPStr allele,
    AjPStr name,
    AjPStr validation,
    AjPStr consequence,
    ajuint mapweight);

EnsPGvvariationfeature ensGvvariationfeatureNewIni(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPGvsource gvsource,
    EnsEGvvariationClass gvvc,
    EnsPGvvariation gvv,
    AjPStr allele,
    AjPStr name,
    AjPStr validation,
    AjPStr consequence,
    ajuint mapweight);

EnsPGvvariationfeature ensGvvariationfeatureNewRef(
    EnsPGvvariationfeature gvvf);

void ensGvvariationfeatureDel(EnsPGvvariationfeature *Pgvvf);

EnsPGvvariationfeatureadaptor ensGvvariationfeatureGetAdaptor(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetAllele(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetConsequencetype(
    const EnsPGvvariationfeature gvvf);

EnsPFeature ensGvvariationfeatureGetFeature(
    const EnsPGvvariationfeature gvvf);

EnsPGvsource ensGvvariationfeatureGetGvsource(
    const EnsPGvvariationfeature gvvf);

EnsEGvvariationClass ensGvvariationfeatureGetGvvariationclass(
    const EnsPGvvariationfeature gvvf);

ajuint ensGvvariationfeatureGetIdentifier(
    const EnsPGvvariationfeature gvvf);

ajuint ensGvvariationfeatureGetMapweight(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetName(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetValidationcode(
    const EnsPGvvariationfeature gvvf);

EnsPGvvariation ensGvvariationfeatureLoadGvvariation(
    EnsPGvvariationfeature gvvf);

AjBool ensGvvariationfeatureSetAdaptor(EnsPGvvariationfeature gvvf,
                                       EnsPGvvariationfeatureadaptor gvvfa);

AjBool ensGvvariationfeatureSetAllele(EnsPGvvariationfeature gvvf,
                                      AjPStr allele);

AjBool ensGvvariationfeatureSetConsequencetype(EnsPGvvariationfeature gvvf,
                                               AjPStr consequencetype);

AjBool ensGvvariationfeatureSetFeature(EnsPGvvariationfeature gvvf,
                                       EnsPFeature feature);

AjBool ensGvvariationfeatureSetGvsource(EnsPGvvariationfeature gvvf,
                                        EnsPGvsource gvsource);

AjBool ensGvvariationfeatureSetGvvariation(EnsPGvvariationfeature gvvf,
                                           EnsPGvvariation gvv);

AjBool ensGvvariationfeatureSetGvvariationclass(EnsPGvvariationfeature gvvf,
                                                EnsEGvvariationClass gvvc);

AjBool ensGvvariationfeatureSetIdentifier(EnsPGvvariationfeature gvvf,
                                          ajuint identifier);

AjBool ensGvvariationfeatureSetMapweight(EnsPGvvariationfeature gvvf,
                                         ajuint mapweight);

AjBool ensGvvariationfeatureSetName(EnsPGvvariationfeature gvvf,
                                    AjPStr name);

AjBool ensGvvariationfeatureSetValidationcode(EnsPGvvariationfeature gvvf,
                                              AjPStr validationcode);

AjBool ensGvvariationfeatureTrace(const EnsPGvvariationfeature gvvf,
                                  ajuint level);

ajuint ensGvvariationfeatureCalculateLength(
    const EnsPGvvariationfeature gvvf);

size_t ensGvvariationfeatureCalculateMemsize(
    const EnsPGvvariationfeature gvvf);

AjBool ensGvvariationfeatureGetSomatic(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetSourcename(
    const EnsPGvvariationfeature gvvf);

AjBool ensGvvariationfeatureIsReference(EnsPGvvariationfeature gvvf,
                                        AjBool *Presult);

/* AJAX List of Ensembl Genetic Variation Variation Feature objects */

AjBool ensListGvvariationfeatureSortEndAscending(AjPList gvvfs);

AjBool ensListGvvariationfeatureSortEndDescending(AjPList gvvfs);

AjBool ensListGvvariationfeatureSortIdentifierAscending(AjPList gvvfs);

AjBool ensListGvvariationfeatureSortStartAscending(AjPList gvvfs);

AjBool ensListGvvariationfeatureSortStartDescending(AjPList gvvfs);

/* Ensembl Genetic Variation Feature Adaptor */

EnsPGvvariationfeatureadaptor ensRegistryGetGvvariationfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvvariationfeatureadaptor ensGvvariationfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvvariationfeatureadaptorDel(EnsPGvvariationfeatureadaptor *Pgvvfa);

EnsPBaseadaptor ensGvvariationfeatureadaptorGetBaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa);

EnsPDatabaseadaptor ensGvvariationfeatureadaptorGetDatabaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa);

EnsPFeatureadaptor ensGvvariationfeatureadaptorGetFeatureadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa);

EnsPGvdatabaseadaptor ensGvvariationfeatureadaptorGetGvdatabaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa);

AjBool ensGvvariationfeatureadaptorFetchAllbyFrequency(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvpopulation gvp,
    float frequency,
    EnsPSlice slice,
    AjPList gvvfs);

AjBool ensGvvariationfeatureadaptorFetchAllbyGvpopulation(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvpopulation gvp,
    EnsPSlice slice,
    AjPList gvvfs);

AjBool ensGvvariationfeatureadaptorFetchAllbyGvvariation(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvvariation gvv,
    AjPList gvvfs);

AjBool ensGvvariationfeatureadaptorFetchAllbySliceAnnotated(
    EnsPGvvariationfeatureadaptor gvvfa,
    const AjPStr vsource,
    const AjPStr psource,
    const AjPStr annotation,
    AjBool somatic,
    EnsPSlice slice,
    AjPList gvvfs);

AjBool ensGvvariationfeatureadaptorFetchAllbySliceConstraint(
    EnsPGvvariationfeatureadaptor gvvfa,
    EnsPSlice slice,
    const AjPStr constraint,
    AjBool somatic,
    AjPList gvvfs);

AjBool ensGvvariationfeatureadaptorFetchAllbySliceGenotyped(
    EnsPGvvariationfeatureadaptor gvvfa,
    EnsPSlice slice,
    AjPList gvvfs);

AjBool ensGvvariationfeatureadaptorFetchByIdentifier(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPGvvariationfeature *Pgvvf);

AjBool ensGvvariationfeatureadaptorRetrieveAllIdentifiers(
    EnsPGvvariationfeatureadaptor gvvfa,
    AjPList identifiers);

/* Ensembl Genetic Variation Variation Set */

EnsPGvvariationset ensGvvariationsetNewCpy(const EnsPGvvariationset gvvs);

EnsPGvvariationset ensGvvariationsetNewIni(EnsPGvvariationsetadaptor gvvsa,
                                           ajuint identifier,
                                           AjPStr name,
                                           AjPStr description,
                                           AjPStr shortname);

EnsPGvvariationset ensGvvariationsetNewRef(EnsPGvvariationset gvvs);

void ensGvvariationsetDel(EnsPGvvariationset *Pgvvs);

EnsPGvvariationsetadaptor ensGvvariationsetGetAdaptor(
    const EnsPGvvariationset gvvs);

AjPStr ensGvvariationsetGetDescription(
    const EnsPGvvariationset gvvs);

ajuint ensGvvariationsetGetIdentifier(
    const EnsPGvvariationset gvvs);

AjPStr ensGvvariationsetGetName(
    const EnsPGvvariationset gvvs);

AjPStr ensGvvariationsetGetShortname(
    const EnsPGvvariationset gvvs);

AjBool ensGvvariationsetSetAdaptor(EnsPGvvariationset gvvs,
                                   EnsPGvvariationsetadaptor gvvsa);

AjBool ensGvvariationsetSetDescription(EnsPGvvariationset gvvs,
                                       AjPStr description);

AjBool ensGvvariationsetSetIdentifier(EnsPGvvariationset gvvs,
                                      ajuint identifier);

AjBool ensGvvariationsetSetName(EnsPGvvariationset gvvs,
                                AjPStr name);

AjBool ensGvvariationsetSetShortname(EnsPGvvariationset gvvs,
                                     AjPStr shortname);

AjBool ensGvvariationsetTrace(const EnsPGvvariationset gvvs,
                              ajuint level);

size_t ensGvvariationsetCalculateMemsize(const EnsPGvvariationset gvvs);

AjBool ensGvvariationsetFetchAllGvvariations(
    EnsPGvvariationset gvvs,
    AjPList gvvss);

AjBool ensGvvariationsetFetchAllSub(EnsPGvvariationset gvvs,
                                    AjBool immediate,
                                    AjPList gvvss);

AjBool ensGvvariationsetFetchAllSuper(EnsPGvvariationset gvvs,
                                      AjBool immediate,
                                      AjPList gvvss);

/* Ensembl Genetic Variation Variation Set Adaptor */

EnsPGvvariationsetadaptor ensRegistryGetGvvariationsetadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvvariationsetadaptor ensGvvariationsetadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvvariationsetadaptorDel(EnsPGvvariationsetadaptor *Pgvvsa);

EnsPBaseadaptor ensGvvariationsetadaptorGetBaseadaptor(
    EnsPGvvariationsetadaptor gvvsa);

EnsPDatabaseadaptor ensGvvariationsetadaptorGetDatabaseadaptor(
    EnsPGvvariationsetadaptor gvvsa);

AjBool ensGvvariationsetadaptorFetchAllToplevel(
    EnsPGvvariationsetadaptor gvvsa,
    AjPList gvvss);

AjBool ensGvvariationsetadaptorFetchAllbyGvvariation(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariation gvv,
    AjPList gvvss);

AjBool ensGvvariationsetadaptorFetchAllbySub(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariationset gvvs,
    AjBool immediate,
    AjPList gvvss);

AjBool ensGvvariationsetadaptorFetchAllbySuper(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariationset gvvs,
    AjBool immediate,
    AjPList gvvss);

AjBool ensGvvariationsetadaptorFetchByIdentifier(
    EnsPGvvariationsetadaptor gvvsa,
    ajuint identifier,
    EnsPGvvariationset *Pgvvs);

AjBool ensGvvariationsetadaptorFetchByName(
    EnsPGvvariationsetadaptor gvvsa,
    const AjPStr name,
    EnsPGvvariationset *Pgvvs);

AjBool ensGvvariationsetadaptorFetchByShortname(
    EnsPGvvariationsetadaptor gvvsa,
    const AjPStr shortname,
    EnsPGvvariationset *Pgvvs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVVARIATION_H */

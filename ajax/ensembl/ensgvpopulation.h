/* @include ensgvpopulation ***************************************************
**
** Ensembl Genetic Variation Population functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.21 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/03/04 12:32:43 $ by $Author: mks $
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

#ifndef ENSGVPOPULATION_H
#define ENSGVPOPULATION_H

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

/* Ensembl Genetic Variation Population */

EnsPGvpopulation ensGvpopulationNewCpy(const EnsPGvpopulation gvp);

EnsPGvpopulation ensGvpopulationNewIni(EnsPGvpopulationadaptor gvpa,
                                       ajuint identifier,
                                       EnsPGvsample gvs,
                                       AjPList subpopulations);

EnsPGvpopulation ensGvpopulationNewRef(EnsPGvpopulation gvp);

void ensGvpopulationDel(EnsPGvpopulation *Pgvp);

EnsPGvpopulationadaptor ensGvpopulationGetAdaptor(const EnsPGvpopulation gvp);

EnsPGvsample ensGvpopulationGetGvsample(const EnsPGvpopulation gvp);

ajuint ensGvpopulationGetIdentifier(const EnsPGvpopulation gvp);

const AjPList ensGvpopulationGetSubgvpopulations(const EnsPGvpopulation gvp);

AjBool ensGvpopulationSetAdaptor(EnsPGvpopulation gvp,
                                 EnsPGvpopulationadaptor gvpa);

AjBool ensGvpopulationSetGvsample(EnsPGvpopulation gvp, EnsPGvsample gvs);

AjBool ensGvpopulationSetIdentifier(EnsPGvpopulation gvp, ajuint identifier);

AjBool ensGvpopulationAddSubgvpopulation(EnsPGvpopulation gvp,
                                         EnsPGvpopulation subgvp);

AjBool ensGvpopulationTrace(const EnsPGvpopulation gvp, ajuint level);

size_t ensGvpopulationCalculateMemsize(const EnsPGvpopulation gvp);

AjBool ensGvpopulationFetchAllGvindividuals(EnsPGvpopulation gvp,
                                            AjPList gvis);

AjBool ensGvpopulationFetchAllSynonyms(EnsPGvpopulation gvp,
                                       const AjPStr source,
                                       AjPList synonyms);

/* Ensembl Genetic Variation Population Adaptor */

EnsPGvpopulationadaptor ensRegistryGetGvpopulationadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvpopulationadaptor ensGvpopulationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvpopulationadaptorDel(EnsPGvpopulationadaptor *Pgvpa);

EnsPGvsampleadaptor ensGvpopulationadaptorGetBaseadaptor(
    EnsPGvpopulationadaptor gvpa);

EnsPDatabaseadaptor ensGvpopulationadaptorGetDatabaseadaptor(
    EnsPGvpopulationadaptor gvpa);

AjBool ensGvpopulationadaptorFetchAllHapmap(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllLd(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllTagged(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvvariationfeature gvvf,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllTags(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvvariationfeature gvvf,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllThousandgenomes(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbyGvindividual(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvindividual gvi,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbyGvindividuals(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvis,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbyGvpopulationSub(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbyGvpopulationSuper(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbyIdentifiers(
    EnsPGvpopulationadaptor gvpa,
    AjPTable gvps);

AjBool ensGvpopulationadaptorFetchAllbyNamesearch(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbySynonym(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchByIdentifier(
    EnsPGvpopulationadaptor gvpa,
    ajuint identifier,
    EnsPGvpopulation *Pgvp);

AjBool ensGvpopulationadaptorFetchByName(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    EnsPGvpopulation *Pgvp);

AjBool ensGvpopulationadaptorFetchDefaultld(
    EnsPGvpopulationadaptor gvpa,
    EnsPGvpopulation *Pgvp);

/* Ensembl Genetic Variation Population Genotype */

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewCpy(
    const EnsPGvpopulationgenotype gvpg);

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewIni(
    EnsPGvpopulationgenotypeadaptor gvpga,
    ajuint identifier,
    EnsPGvpopulation gvp,
    EnsPGvgenotype gvg,
    ajuint counter,
    float frequency);

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewRef(
    EnsPGvpopulationgenotype gvpg);

void ensGvpopulationgenotypeDel(EnsPGvpopulationgenotype *Pgvpg);

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeGetAdaptor(
    const EnsPGvpopulationgenotype gvpg);

ajuint ensGvpopulationgenotypeGetCounter(
    const EnsPGvpopulationgenotype gvpg);

float ensGvpopulationgenotypeGetFrequency(
    const EnsPGvpopulationgenotype gvpg);

EnsPGvgenotype ensGvpopulationgenotypeGetGvgenotype(
    const EnsPGvpopulationgenotype gvpg);

EnsPGvpopulation ensGvpopulationgenotypeGetGvpopulation(
    const EnsPGvpopulationgenotype gvpg);

ajuint ensGvpopulationgenotypeGetIdentifier(
    const EnsPGvpopulationgenotype gvpg);

AjBool ensGvpopulationgenotypeSetAdaptor(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulationgenotypeadaptor gvpga);

AjBool ensGvpopulationgenotypeSetCounter(
    EnsPGvpopulationgenotype gvpg,
    ajuint counter);

AjBool ensGvpopulationgenotypeSetFrequency(
    EnsPGvpopulationgenotype gvpg,
    float frequency);

AjBool ensGvpopulationgenotypeSetGvgenotype(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvgenotype gvg);

AjBool ensGvpopulationgenotypeSetGvpopulation(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulation gvp);

AjBool ensGvpopulationgenotypeSetIdentifier(
    EnsPGvpopulationgenotype gvpg,
    ajuint identifier);

AjBool ensGvpopulationgenotypeTrace(const EnsPGvpopulationgenotype gvpg,
                                    ajuint level);

AjPStr ensGvpopulationgenotypeGetSubhandle(
    const EnsPGvpopulationgenotype gvpg);

ajuint ensGvpopulationgenotypeGetSubidentifier(
    const EnsPGvpopulationgenotype gvpg);

size_t ensGvpopulationgenotypeCalculateMemsize(
    const EnsPGvpopulationgenotype gvpg);

/* Ensembl Genetic Variation Population Genotype Adaptor */

EnsPGvpopulationgenotypeadaptor ensRegistryGetGvpopulationgenotypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvpopulationgenotypeadaptorDel(
    EnsPGvpopulationgenotypeadaptor *Pgvpga);

AjBool ensGvpopulationgenotypeadaptorFetchAllbyGvpopulation(
    EnsPGvpopulationgenotypeadaptor gvpga,
    const EnsPGvpopulation gvp,
    AjPList gvpgs);

AjBool ensGvpopulationgenotypeadaptorFetchAllbyGvvariation(
    EnsPGvpopulationgenotypeadaptor gvpga,
    const EnsPGvvariation gvv,
    AjPList gvpgs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVPOPULATION_H */


#ifndef ENSGVPOPULATION_H
#define ENSGVPOPULATION_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensgvdata.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

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

void ensGvpopulationDel(EnsPGvpopulation* Pgvp);

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

AjBool ensTableGvpopulationClear(AjPTable table);

AjBool ensTableGvpopulationDelete(AjPTable* Ptable);

/* Ensembl Genetic Variation Population Adaptor */

EnsPGvpopulationadaptor ensRegistryGetGvpopulationadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvpopulationadaptor ensGvpopulationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvpopulationadaptorDel(EnsPGvpopulationadaptor* Pgvpa);

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

AjBool ensGvpopulationadaptorFetchAllThousandgenomes(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllbyGvindividual(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvindividual gvi,
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
    EnsPGvpopulation* Pgvp);

AjBool ensGvpopulationadaptorFetchByName(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    EnsPGvpopulation* Pgvp);

AjBool ensGvpopulationadaptorFetchDefaultld(
    EnsPGvpopulationadaptor gvpa,
    EnsPGvpopulation* Pgvp);

/* Ensembl Genetic Variation Population Genotype */

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewCpy(
    const EnsPGvpopulationgenotype gvpg);

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewIni(
    EnsPGvpopulationgenotypeadaptor gvpga,
    ajuint identifier,
    EnsPGvpopulation gvp,
    EnsPGvvariation gvv,
    AjPStr allele1,
    AjPStr allele2,
    ajuint counter,
    float frequency);

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewRef(
    EnsPGvpopulationgenotype gvpg);

void ensGvpopulationgenotypeDel(EnsPGvpopulationgenotype* Pgvpg);

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeGetAdaptor(
    const EnsPGvpopulationgenotype gvpg);

AjPStr ensGvpopulationgenotypeGetAllele1(
    const EnsPGvpopulationgenotype gvpg);

AjPStr ensGvpopulationgenotypeGetAllele2(
    const EnsPGvpopulationgenotype gvpg);

ajuint ensGvpopulationgenotypeGetCounter(
    const EnsPGvpopulationgenotype gvpg);

float ensGvpopulationgenotypeGetFrequency(
    const EnsPGvpopulationgenotype gvpg);

EnsPGvpopulation ensGvpopulationgenotypeGetGvpopulation(
    const EnsPGvpopulationgenotype gvpg);

EnsPGvvariation ensGvpopulationgenotypeGetGvvariation(
    const EnsPGvpopulationgenotype gvpg);

ajuint ensGvpopulationgenotypeGetIdentifier(
    const EnsPGvpopulationgenotype gvpg);

ajuint ensGvpopulationgenotypeGetSubidentifier(
    const EnsPGvpopulationgenotype gvpg);

AjBool ensGvpopulationgenotypeSetAdaptor(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulationgenotypeadaptor gvpga);

AjBool ensGvpopulationgenotypeSetAllele1(
    EnsPGvpopulationgenotype gvpg,
    AjPStr allele1);

AjBool ensGvpopulationgenotypeSetAllele2(
    EnsPGvpopulationgenotype gvpg,
    AjPStr allele2);

AjBool ensGvpopulationgenotypeSetCounter(
    EnsPGvpopulationgenotype gvpg,
    ajuint counter);

AjBool ensGvpopulationgenotypeSetFrequency(
    EnsPGvpopulationgenotype gvpg,
    float frequency);

AjBool ensGvpopulationgenotypeSetGvpopulation(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulation gvp);

AjBool ensGvpopulationgenotypeSetGvvariation(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvvariation gvv);

AjBool ensGvpopulationgenotypeSetIdentifier(
    EnsPGvpopulationgenotype gvpg,
    ajuint identifier);

AjBool ensGvpopulationgenotypeSetSubidentifier(
    EnsPGvpopulationgenotype gvpg,
    ajuint subidentifier);

AjBool ensGvpopulationgenotypeTrace(const EnsPGvpopulationgenotype gvpg,
                                    ajuint level);

size_t ensGvpopulationgenotypeCalculateMemsize(
    const EnsPGvpopulationgenotype gvpg);

/* Ensembl Genetic Variation Population Genotype Adaptor */

EnsPGvpopulationgenotypeadaptor ensRegistryGetGvpopulationgenotypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvpopulationgenotypeadaptorDel(
    EnsPGvpopulationgenotypeadaptor* Pgvpga);

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

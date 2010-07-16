#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensgvpopulation_h
#define ensgvpopulation_h

#include "ensgvdata.h"
#include "ensgvsample.h"




/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Population */

EnsPGvpopulation ensGvpopulationNew(EnsPGvpopulationadaptor gvpa,
                                    ajuint identifier,
                                    EnsPGvsample gvs,
                                    AjPList subpopulations);

EnsPGvpopulation ensGvpopulationNewObj(const EnsPGvpopulation object);

EnsPGvpopulation ensGvpopulationNewRef(EnsPGvpopulation gvp);

void ensGvpopulationDel(EnsPGvpopulation* Pgvp);

EnsPGvpopulationadaptor ensGvpopulationGetAdaptor(const EnsPGvpopulation gvp);

ajuint ensGvpopulationGetIdentifier(const EnsPGvpopulation gvp);

EnsPGvsample ensGvpopulationGetGvsample(const EnsPGvpopulation gvp);

const AjPList ensGvpopulationGetSubPopulations(const EnsPGvpopulation gvp);

AjBool ensGvpopulationSetAdaptor(EnsPGvpopulation gvp,
                                 EnsPGvpopulationadaptor gvpa);

AjBool ensGvpopulationSetIdentifier(EnsPGvpopulation gvp, ajuint identifier);

AjBool ensGvpopulationSetGvsample(EnsPGvpopulation gvp, EnsPGvsample gvs);

AjBool ensGvpopulationAddSubPopulation(EnsPGvpopulation gvp,
                                       EnsPGvpopulation subgvp);

ajulong ensGvpopulationGetMemsize(const EnsPGvpopulation gvp);

AjBool ensGvpopulationTrace(const EnsPGvpopulation gvp, ajuint level);

/* Ensembl Genetic Variation Population Adaptor */

EnsPGvpopulationadaptor ensRegistryGetGvpopulationadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvpopulationadaptor ensGvpopulationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvpopulationadaptorDel(EnsPGvpopulationadaptor *Pgvpa);

EnsPGvsampleadaptor ensGvpopulationadaptorGetAdaptor(
    EnsPGvpopulationadaptor gvpa);

AjBool ensGvpopulationadaptorFetchByIdentifier(
    const EnsPGvpopulationadaptor gvpa,
    ajuint identifier,
    EnsPGvpopulation *Pgvp);

AjBool ensGvpopulationadaptorFetchByName(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    EnsPGvpopulation* Pgvp);

AjBool ensGvpopulationadaptorFetchAllBySynonym(
    const EnsPGvpopulationadaptor gvpa,
    const AjPStr synonym,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllBySuperPopulation(
    const EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllBySubPopulation(
    const EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchAllByIndvividual(
    const EnsPGvpopulationadaptor gvpa,
    const EnsPGvindividual gvi,
    AjPList gvps);

AjBool ensGvpopulationadaptorFetchDefaultLDPopulation(
    const EnsPGvpopulationadaptor gvpa,
    EnsPGvpopulation *Pgvp);

AjBool ensGvpopulationadaptorFetchAllLDPopulations(
    const EnsPGvpopulationadaptor gvpa,
    AjPList gvps);

/*
** End of prototype definitions
*/




#endif /* ensgvpopulation_h */

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensgvindividual_h
#define ensgvindividual_h

#include "ensgvsample.h"




/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Individual */

EnsPGvindividual ensGvindividualNew(EnsPGvindividualadaptor gvia,
                                    ajuint identifier,
                                    EnsPGvsample gvs,
                                    EnsPGvindividual father,
                                    EnsPGvindividual mother,
                                    EnsEGvindividualGender gender,
                                    EnsEGvindividualType type,
                                    AjPStr description);

EnsPGvindividual ensGvindividualNewObj(const EnsPGvindividual object);

EnsPGvindividual ensGvindividualNewRef(EnsPGvindividual gvi);

void ensGvindividualDel(EnsPGvindividual* Pgvi);

EnsPGvindividualadaptor ensGvindividualGetAdaptor(const EnsPGvindividual gvi);

ajuint ensGvindividualGetIdentifier(const EnsPGvindividual gvi);

EnsPGvsample ensGvindividualGetGvsample(const EnsPGvindividual gvi);

EnsPGvindividual ensGvindividualGetFather(const EnsPGvindividual gvi);

EnsPGvindividual ensGvindividualGetMother(const EnsPGvindividual gvi);

EnsEGvindividualGender ensGvindividualGetGender(const EnsPGvindividual gvi);

EnsEGvindividualType ensGvindividualGetType(const EnsPGvindividual gvi);

AjPStr ensGvindividualGetDescription(const EnsPGvindividual gvi);

AjBool ensGvindividualSetAdaptor(EnsPGvindividual gvi,
                                 EnsPGvindividualadaptor gvia);

AjBool ensGvindividualSetIdentifier(EnsPGvindividual gvi,
                                    ajuint identifier);

AjBool ensGvindividualSetGvsample(EnsPGvindividual gvi,
                                  EnsPGvsample gvs);

AjBool ensGvindividualSetFather(EnsPGvindividual gvi,
                                EnsPGvindividual father);

AjBool ensGvindividualSetMother(EnsPGvindividual gvi,
                                EnsPGvindividual mother);

AjBool ensGvindividualSetGender(EnsPGvindividual gvi,
                                EnsEGvindividualGender gender);

AjBool ensGvindividualSetType(EnsPGvindividual gvi,
                              EnsEGvindividualType type);

AjBool ensGvindividualSetDescription(EnsPGvindividual gvi,
                                     AjPStr description);

EnsEGvindividualGender ensGvindividualGenderFromStr(const AjPStr gender);

EnsEGvindividualType ensGvindividualTypeFromStr(const AjPStr type);

const char *ensGvindividualGenderToChar(EnsEGvindividualGender gender);

const char *ensGvindividualTypeToChar(EnsEGvindividualType type);

ajulong ensGvindividualGetMemsize(const EnsPGvindividual gvi);

AjBool ensGvindividualTrace(const EnsPGvindividual gvi, ajuint level);

/* Ensembl Genetic Variation Individual Adaptor */

EnsPGvindividualadaptor ensRegistryGetGvindividualadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvindividualadaptor ensGvindividualadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvindividualadaptorDel(EnsPGvindividualadaptor *Pgvia);

EnsPGvsampleadaptor ensGvindividualadaptorGetAdaptor(
    EnsPGvindividualadaptor gvia);

AjBool ensGvindividualadaptorFetchByIdentifier(
    const EnsPGvindividualadaptor gvia,
    ajuint identifier,
    EnsPGvindividual *Pgvi);

AjBool ensGvindividualadaptorFetchAllByName(EnsPGvindividualadaptor gvia,
                                            const AjPStr name,
                                            AjPList gvis);

AjBool ensGvindividualadaptorFetchAllBySynonym(
    const EnsPGvindividualadaptor adaptor,
    const AjPStr synonym,
    AjPList individuals);

AjBool ensGvindividualadaptorFetchAllByPopulation(
    const EnsPGvindividualadaptor gvia,
    const EnsPGvpopulation gvp,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllByParentIndividual(
    const EnsPGvindividualadaptor gvia,
    const EnsPGvindividual parent,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllStrains(
    const EnsPGvindividualadaptor gvia,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchReferenceStrainName(
    const EnsPGvindividualadaptor gvia,
    AjPStr *Pname);

AjBool ensGvindividualadaptorFetchAllDefaultStrains(
    const EnsPGvindividualadaptor gvia,
    AjPList names);

AjBool ensGvindividualadaptorFetchAllDisplayStrains(
    const EnsPGvindividualadaptor gvia,
    AjPList individuals);

AjBool ensGvindividualadaptorFetchAllStrainsWithCoverage(
    const EnsPGvindividualadaptor gvia,
    AjPList idlist);

/*
** End of prototype definitions
*/




#endif /* ensgvindividual_h */

#ifdef __cplusplus
}
#endif

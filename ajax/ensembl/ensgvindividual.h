
#ifndef ENSGVINDIVIDUAL_H
#define ENSGVINDIVIDUAL_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensgvsample.h"

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

/* Ensembl Genetic Variation Individual */

EnsPGvindividual ensGvindividualNewCpy(const EnsPGvindividual gvi);

EnsPGvindividual ensGvindividualNewIni(EnsPGvindividualadaptor gvia,
                                       ajuint identifier,
                                       EnsPGvsample gvs,
                                       EnsPGvindividual father,
                                       EnsPGvindividual mother,
                                       EnsEGvindividualGender gender,
                                       EnsEGvindividualType type,
                                       AjPStr description);

EnsPGvindividual ensGvindividualNewRef(EnsPGvindividual gvi);

void ensGvindividualDel(EnsPGvindividual* Pgvi);

EnsPGvindividualadaptor ensGvindividualGetAdaptor(const EnsPGvindividual gvi);

AjPStr ensGvindividualGetDescription(const EnsPGvindividual gvi);

EnsPGvindividual ensGvindividualGetFather(const EnsPGvindividual gvi);

EnsEGvindividualGender ensGvindividualGetGender(const EnsPGvindividual gvi);

EnsPGvsample ensGvindividualGetGvsample(const EnsPGvindividual gvi);

ajuint ensGvindividualGetIdentifier(const EnsPGvindividual gvi);

EnsPGvindividual ensGvindividualGetMother(const EnsPGvindividual gvi);

EnsEGvindividualType ensGvindividualGetType(const EnsPGvindividual gvi);

AjBool ensGvindividualSetAdaptor(EnsPGvindividual gvi,
                                 EnsPGvindividualadaptor gvia);

AjBool ensGvindividualSetDescription(EnsPGvindividual gvi,
                                     AjPStr description);

AjBool ensGvindividualSetFather(EnsPGvindividual gvi,
                                EnsPGvindividual father);

AjBool ensGvindividualSetGender(EnsPGvindividual gvi,
                                EnsEGvindividualGender gender);

AjBool ensGvindividualSetGvsample(EnsPGvindividual gvi,
                                  EnsPGvsample gvs);

AjBool ensGvindividualSetIdentifier(EnsPGvindividual gvi,
                                    ajuint identifier);

AjBool ensGvindividualSetMother(EnsPGvindividual gvi,
                                EnsPGvindividual mother);

AjBool ensGvindividualSetType(EnsPGvindividual gvi,
                              EnsEGvindividualType type);

AjBool ensGvindividualTrace(const EnsPGvindividual gvi, ajuint level);

size_t ensGvindividualCalculateMemsize(const EnsPGvindividual gvi);

EnsEGvindividualGender ensGvindividualGenderFromStr(const AjPStr gender);

const char* ensGvindividualGenderToChar(EnsEGvindividualGender gvig);

EnsEGvindividualType ensGvindividualTypeFromStr(const AjPStr type);

const char* ensGvindividualTypeToChar(EnsEGvindividualType gvit);

/* Ensembl Genetic Variation Individual Adaptor */

EnsPGvindividualadaptor ensRegistryGetGvindividualadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvindividualadaptor ensGvindividualadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvindividualadaptorDel(EnsPGvindividualadaptor* Pgvia);

EnsPGvsampleadaptor ensGvindividualadaptorGetBaseadaptor(
    EnsPGvindividualadaptor gvia);

EnsPDatabaseadaptor ensGvindividualadaptorGetDatabaseadaptor(
    EnsPGvindividualadaptor gvia);

AjBool ensGvindividualadaptorFetchAllCoverage(
    EnsPGvindividualadaptor gvia,
    AjPList idlist);

AjBool ensGvindividualadaptorFetchAllDefault(
    EnsPGvindividualadaptor gvia,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllDisplayable(
    EnsPGvindividualadaptor gvia,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllStrains(
    EnsPGvindividualadaptor gvia,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllbyDisplay(
    EnsPGvindividualadaptor gvia,
    EnsEGvsampleDisplay display,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllbyGvpopulation(
    EnsPGvindividualadaptor gvia,
    const EnsPGvpopulation gvp,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllbyName(
    EnsPGvindividualadaptor gvia,
    const AjPStr name,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllbyParent(
    EnsPGvindividualadaptor gvia,
    const EnsPGvindividual parent,
    AjPList gvis);

AjBool ensGvindividualadaptorFetchAllbySynonym(
    EnsPGvindividualadaptor adaptor,
    const AjPStr synonym,
    const AjPStr source,
    AjPList individuals);

AjBool ensGvindividualadaptorFetchByIdentifier(
    EnsPGvindividualadaptor gvia,
    ajuint identifier,
    EnsPGvindividual* Pgvi);

AjBool ensGvindividualadaptorFetchReference(
    EnsPGvindividualadaptor gvia,
    EnsPGvindividual* Pgvi);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVINDIVIDUAL_H */

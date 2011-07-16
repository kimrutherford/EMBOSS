
#ifndef ENSREGISTRY_H
#define ENSREGISTRY_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensembl.h"

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

/* Ensembl Registry */

void ensRegistryClear(void);

void ensRegistryExit(void);

void ensRegistryInit(void);

AjBool ensRegistryLoadDatabaseconnection(EnsPDatabaseconnection dbc);

AjBool ensRegistryLoadServername(AjPStr servername);

AjBool ensRegistryRetrieveAllSpecies(AjPList species);

/* Ensembl Registry Alias */

AjBool ensRegistryAliasAdd(const AjPStr species, const AjPStr alias);

AjBool ensRegistryAliasClear(void);

AjBool ensRegistryAliasFetchAllbySpecies(const AjPStr species,
                                         AjPList aliases);

AjBool ensRegistryAliasLoadFile(const AjPStr filename);

AjBool ensRegistryAliasRemove(const AjPStr alias);

AjBool ensRegistryAliasResolve(const AjPStr alias, AjPStr* Pspecies);

AjBool ensRegistryAliasTrace(ajuint level);

/* Ensembl Registry Entry */

AjBool ensRegistryEntryClear(void);

AjBool ensRegistryEntryTrace(ajuint level);

/* Ensembl Registry Identifier */

AjBool ensRegistryIdentifierClear(void);

AjBool ensRegistryIdentifierLoadFile(const AjPStr filename);

AjBool ensRegistryIdentifierResolve(const AjPStr identifier,
                                    AjPStr* Pspecies,
                                    EnsEDatabaseadaptorGroup* Pdbag);

/* Ensembl Registry Source */

AjBool ensRegistrySourceTrace(ajuint level);

/* Ensembl Registry Database Adaptor */

AjBool ensRegistryAddDatabaseadaptor(EnsPDatabaseadaptor dba);

AjBool ensRegistryAddReferenceadaptor(EnsPDatabaseadaptor dba,
                                      EnsPDatabaseadaptor rsa);

AjBool ensRegistryAddStableidentifierprefix(EnsPDatabaseadaptor dba,
                                            const AjPStr prefix);

EnsPDatabaseadaptor ensRegistryNewDatabaseadaptor(
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier);

EnsPDatabaseadaptor ensRegistryNewReferenceadaptor(
    EnsPDatabaseadaptor dba,
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier);

AjBool ensRegistryRemoveDatabaseadaptor(EnsPDatabaseadaptor* Pdba);

AjBool ensRegistryGetAllDatabaseadaptors(EnsEDatabaseadaptorGroup dbag,
                                         const AjPStr alias,
                                         AjPList dbas);

AjPStr ensRegistryGetStableidentifierprefix(EnsPDatabaseadaptor dba);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSREGISTRY_H */

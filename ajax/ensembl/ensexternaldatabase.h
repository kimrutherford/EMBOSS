
#ifndef ENSEXTERNALDATABASE_H
#define ENSEXTERNALDATABASE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdata.h"

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

/* Ensembl External Database */

EnsPExternaldatabase ensExternaldatabaseNewCpy(
    const EnsPExternaldatabase edb);

EnsPExternaldatabase ensExternaldatabaseNewIni(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    AjPStr name,
    AjPStr release,
    AjPStr displayname,
    AjPStr secondaryname,
    AjPStr secondarytable,
    AjPStr description,
    EnsEExternaldatabaseStatus status,
    EnsEExternaldatabaseType type,
    ajint priority);

EnsPExternaldatabase ensExternaldatabaseNewRef(EnsPExternaldatabase edb);

void ensExternaldatabaseDel(EnsPExternaldatabase* Pedb);

EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetDescription(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetDisplayname(
    const EnsPExternaldatabase edb);

ajuint ensExternaldatabaseGetIdentifier(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetName(
    const EnsPExternaldatabase edb);

ajint ensExternaldatabaseGetPriority(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetRelease(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondaryname(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondarytable(
    const EnsPExternaldatabase edb);

EnsEExternaldatabaseStatus ensExternaldatabaseGetStatus(
    const EnsPExternaldatabase edb);

EnsEExternaldatabaseType ensExternaldatabaseGetType(
    const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseSetAdaptor(EnsPExternaldatabase edb,
                                     EnsPExternaldatabaseadaptor edba);

AjBool ensExternaldatabaseSetDescription(EnsPExternaldatabase edb,
                                         AjPStr description);

AjBool ensExternaldatabaseSetDisplayname(EnsPExternaldatabase edb,
                                         AjPStr displayname);

AjBool ensExternaldatabaseSetIdentifier(EnsPExternaldatabase edb,
                                        ajuint identifier);

AjBool ensExternaldatabaseSetName(EnsPExternaldatabase edb,
                                  AjPStr name);

AjBool ensExternaldatabaseSetPriority(EnsPExternaldatabase edb,
                                      ajint priority);

AjBool ensExternaldatabaseSetRelease(EnsPExternaldatabase edb,
                                     AjPStr release);

AjBool ensExternaldatabaseSetSecondaryname(EnsPExternaldatabase edb,
                                           AjPStr secondaryname);

AjBool ensExternaldatabaseSetSecondarytable(EnsPExternaldatabase edb,
                                            AjPStr secondarytable);

AjBool ensExternaldatabaseSetStatus(EnsPExternaldatabase edb,
                                    EnsEExternaldatabaseStatus status);

AjBool ensExternaldatabaseSetType(EnsPExternaldatabase edb,
                                  EnsEExternaldatabaseType type);

AjBool ensExternaldatabaseTrace(const EnsPExternaldatabase edb, ajuint level);

size_t ensExternaldatabaseCalculateMemsize(const EnsPExternaldatabase edb);

EnsEExternaldatabaseStatus ensExternaldatabaseStatusFromStr(
    const AjPStr status);

const char* ensExternaldatabaseStatusToChar(EnsEExternaldatabaseStatus edbs);

EnsEExternaldatabaseType ensExternaldatabaseTypeFromStr(
    const AjPStr type);

const char* ensExternaldatabaseTypeToChar(EnsEExternaldatabaseType edbt);

/* Ensembl External Database Adaptor */

EnsPExternaldatabaseadaptor ensRegistryGetExternaldatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPExternaldatabaseadaptor ensExternaldatabaseadaptorNew(
    EnsPDatabaseadaptor dba);

void ensExternaldatabaseadaptorDel(
    EnsPExternaldatabaseadaptor* Pedba);

EnsPBaseadaptor ensExternaldatabaseadaptorGetBaseadaptor(
    const EnsPExternaldatabaseadaptor edba);

EnsPDatabaseadaptor ensExternaldatabaseadaptorGetDatabaseadaptor(
    const EnsPExternaldatabaseadaptor edba);

AjBool ensExternaldatabaseadaptorFetchAll(
    EnsPExternaldatabaseadaptor edba,
    AjPList edbs);

AjBool ensExternaldatabaseadaptorFetchByIdentifier(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    EnsPExternaldatabase* Pedb);

AjBool ensExternaldatabaseadaptorFetchByName(
    EnsPExternaldatabaseadaptor edba,
    const AjPStr name,
    EnsPExternaldatabase* Pedb);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSEXTERNALDATABASE_H */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensexternaldatabase_h
#define ensexternaldatabase_h

#include "ensdata.h"




/*
** Prototype definitions
*/

/* Ensembl External Database */

EnsPExternaldatabase ensExternaldatabaseNew(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    AjPStr name,
    AjPStr release,
    AjPStr displayname,
    AjPStr secondaryname,
    AjPStr secondarytable,
    AjPStr description,
    AjBool primarylinkable,
    AjBool displaylinkable,
    EnsEExternaldatabaseStatus status,
    EnsEExternaldatabaseType type,
    ajint priority);

EnsPExternaldatabase ensExternaldatabaseNewObj(
    const EnsPExternaldatabase object);

EnsPExternaldatabase ensExternaldatabaseNewRef(EnsPExternaldatabase edb);

void ensExternaldatabaseDel(EnsPExternaldatabase *Pedb);

const EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb);

ajuint ensExternaldatabaseGetIdentifier(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetName(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetRelease(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondaryName(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondaryTable(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetDescription(
    const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseGetPrimaryIdIsLinkable(
    const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseGetDisplayIdIsLinkable(
    const EnsPExternaldatabase edb);

EnsEExternaldatabaseStatus ensExternaldatabaseGetStatus(
    const EnsPExternaldatabase edb);

EnsEExternaldatabaseType ensExternaldatabaseGetType(
    const EnsPExternaldatabase edb);

ajint ensExternaldatabaseGetPriority(const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseSetAdaptor(EnsPExternaldatabase edb,
                                     EnsPExternaldatabaseadaptor edba);

AjBool ensExternaldatabaseSetIdentifier(EnsPExternaldatabase edb,
                                        ajuint identifier);

AjBool ensExternaldatabaseSetName(EnsPExternaldatabase edb,
                                  AjPStr name);

AjBool ensExternaldatabaseSetRelease(EnsPExternaldatabase edb,
                                     AjPStr release);

AjBool ensExternaldatabaseSetSecondaryName(EnsPExternaldatabase edb,
                                           AjPStr secondaryname);

AjBool ensExternaldatabaseSetSecondaryTable(EnsPExternaldatabase edb,
                                            AjPStr secondarytable);

AjBool ensExternaldatabaseSetDescription(EnsPExternaldatabase edb,
                                         AjPStr description);

AjBool ensExternaldatabaseSetPrimaryIdIsLinkable(EnsPExternaldatabase edb,
                                                 AjBool primarylinkable);

AjBool ensExternaldatabaseSetDisplayIdIsLinkable(EnsPExternaldatabase edb,
                                                 AjBool displaylinkable);

AjBool ensExternaldatabaseSetStatus(EnsPExternaldatabase edb,
                                    EnsEExternaldatabaseStatus status);

AjBool ensExternaldatabaseSetType(EnsPExternaldatabase edb,
                                  EnsEExternaldatabaseType type);

AjBool ensExternaldatabaseSetPriority(EnsPExternaldatabase edb,
                                      ajint priority);

AjBool ensExternaldatabaseTrace(const EnsPExternaldatabase edb, ajuint level);

EnsEExternaldatabaseStatus ensExternaldatabaseStatusFromStr(
    const AjPStr status);

EnsEExternaldatabaseType ensExternaldatabaseTypeFromStr(
    const AjPStr type);

const char *ensExternaldatabaseStatusToChar(EnsEExternaldatabaseStatus status);

const char *ensExternaldatabaseTypeToChar(EnsEExternaldatabaseType type);

ajulong ensExternaldatabaseGetMemsize(const EnsPExternaldatabase edb);

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
    EnsPExternaldatabase *Pedb);

AjBool ensExternaldatabaseadaptorFetchByName(
    EnsPExternaldatabaseadaptor edba,
    const AjPStr name,
    EnsPExternaldatabase *Pedb);

/*
** End of prototype definitions
*/




#endif /* ensexternaldatabase_h */

#ifdef __cplusplus
}
#endif

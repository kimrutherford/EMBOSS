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

EnsPExternaldatabase ensExternaldatabaseNew(EnsPExternaldatabaseadaptor adaptor,
                                            ajuint identifier,
                                            AjPStr name,
                                            AjPStr release,
                                            AjPStr displayname,
                                            AjPStr secondaryname,
                                            AjPStr secondarytable,
                                            AjPStr description,
                                            AjBool primarylinkable,
                                            AjBool displaylinkable,
                                            AjEnum status,
                                            AjEnum type,
                                            ajint priority);

EnsPExternaldatabase ensExternaldatabaseNewObj(
    const EnsPExternaldatabase object);

EnsPExternaldatabase ensExternaldatabaseNewRef(EnsPExternaldatabase edb);

void ensExternaldatabaseDel(EnsPExternaldatabase *Pedb);

const EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb);

ajuint ensExternaldatabaseGetIdentifier(const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetName(const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetRelease(const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondaryName(const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondaryTable(const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetDescription(const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseGetPrimaryIdIsLinkable(
    const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseGetDisplayIdIsLinkable(
    const EnsPExternaldatabase edb);

AjEnum ensExternaldatabaseGetStatus(const EnsPExternaldatabase edb);

AjEnum ensExternaldatabaseGetType(const EnsPExternaldatabase edb);

ajint ensExternaldatabaseGetPriority(const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseSetAdaptor(EnsPExternaldatabase edb,
                                     EnsPExternaldatabaseadaptor adaptor);

AjBool ensExternaldatabaseSetIdentifier(EnsPExternaldatabase edb,
                                        ajuint identifier);

AjBool ensExternaldatabaseSetName(EnsPExternaldatabase edb, AjPStr name);

AjBool ensExternaldatabaseSetRelease(EnsPExternaldatabase edb, AjPStr release);

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

AjBool ensExternaldatabaseSetStatus(EnsPExternaldatabase edb, AjEnum status);

AjBool ensExternaldatabaseSetType(EnsPExternaldatabase edb, AjEnum type);

AjBool ensExternaldatabaseSetPriority(EnsPExternaldatabase edb, ajint priority);

AjBool ensExternaldatabaseTrace(const EnsPExternaldatabase edb, ajuint level);

AjEnum ensExternaldatabaseStatusFromStr(const AjPStr status);

AjEnum ensExternaldatabaseTypeFromStr(const AjPStr type);

const char *ensExternaldatabaseStatusToChar(const AjEnum status);

const char *ensExternaldatabaseTypeToChar(const AjEnum type);

ajuint ensExternaldatabaseGetMemSize(const EnsPExternaldatabase edb);

/* Ensembl External Database Adaptor */

EnsPExternaldatabaseadaptor ensExternaldatabaseadaptorNew(
    EnsPDatabaseadaptor dba);

void ensExternaldatabaseadaptorDel(EnsPExternaldatabaseadaptor* Pedba);

AjBool ensExternaldatabaseadaptorFetchByIdentifier(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    EnsPExternaldatabase *Pedb);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

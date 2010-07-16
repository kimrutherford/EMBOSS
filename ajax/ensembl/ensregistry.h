#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensregistry_h
#define ensregistry_h

#include "ajax.h"
#include "ensdatabaseadaptor.h"
#include "ensanalysis.h"
#include "ensassemblyexception.h"
#include "ensassemblymapper.h"
#include "enscoordsystem.h"
#include "ensdatabaseentry.h"
#include "ensdensity.h"
#include "ensditag.h"
#include "ensexon.h"
#include "ensexternaldatabase.h"
#include "ensgene.h"
#include "ensgvdata.h"
#include "ensgvsample.h"
#include "ensgvindividual.h"
#include "ensgvpopulation.h"
#include "enskaryotype.h"
#include "ensmarker.h"
#include "ensmetacoordinate.h"
#include "ensmetainformation.h"
#include "ensmiscellaneous.h"
#include "ensprediction.h"
#include "ensqc.h"
#include "ensrepeat.h"
#include "enssequence.h"
#include "ensseqregion.h"
#include "ensslice.h"
#include "enstranscript.h"
#include "enstranslation.h"
#include "ensvariation.h"




/*
** Prototype definitions
*/

/* Ensembl Registry */

void ensRegistryInit(void);

void ensRegistryClear(void);

void ensRegistryExit(void);

/* Ensembl Registry Alias */

AjBool ensRegistryAddAlias(const AjPStr species, const AjPStr alias);

AjBool ensRegistryRemoveAlias(const AjPStr alias);

AjPStr ensRegistryGetSpecies(const AjPStr alias);

AjBool ensRegistryLoadAliasesFromFile(const AjPStr filename);

AjBool ensRegistryTraceAliases(ajuint level);

AjBool ensRegistryTraceEntries(ajuint level);

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
    EnsEDatabaseadaptorGroup group,
    AjBool multi,
    ajuint identifier);

EnsPDatabaseadaptor ensRegistryNewReferenceadaptor(
    EnsPDatabaseadaptor dba,
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup group,
    AjBool multi,
    ajuint identifier);

AjBool ensRegistryRemoveDatabaseadaptor(EnsPDatabaseadaptor *Pdba);

AjBool ensRegistryGetAllDatabaseadaptors(EnsEDatabaseadaptorGroup group,
                                         const AjPStr alias,
                                         AjPList dbas);

AjPStr ensRegistryGetStableidentifierprefix(EnsPDatabaseadaptor dba);

AjBool ensRegistryLoadFromServer(EnsPDatabaseconnection dbc);

AjBool ensRegistryLoadIdentifiers(void);

AjBool ensRegistryGetSpeciesGroup(const AjPStr identifier,
                                  AjPStr *Pspecies,
                                  EnsEDatabaseadaptorGroup *Pgroup);

/*
** End of prototype definitions
*/




#endif /* ensregistry_h */

#ifdef __cplusplus
}
#endif

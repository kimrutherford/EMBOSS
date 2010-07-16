#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensmetainformation_h
#define ensmetainformation_h

#include "ensdatabaseadaptor.h"
#include "enstable.h"




/* @data EnsPMetainformationadaptor *******************************************
**
** Ensembl Meta-Information Adaptor.
**
** @alias EnsSMetainformationadaptor
** @alias EnsOMetainformationadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @attr CacheByIdentifier [AjPTable] Cache by Meta-Information SQL identifier.
** @attr CacheByKey [AjPTable] Cache by Meta-Information key data.
** @@
******************************************************************************/

typedef struct EnsSMetainformationadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByKey;
} EnsOMetainformationadaptor;

#define EnsPMetainformationadaptor EnsOMetainformationadaptor*




/* @data EnsPMetainformation **************************************************
**
** Ensembl Meta-Information.
**
** @alias EnsSMetainformation
** @alias EnsOMetainformation
**
** @attr Use [ajuint] Use counter.
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL Database-internal identifier.
** @attr Adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor.
** @cc Bio::EnsEMBL::MetaContainer
** @attr Key [AjPStr] Key.
** @attr Value [AjPStr] Value.
** @attr Species [ajuint] Species identifier.
** @attr Padding [ajuint] Padding to alignment boundary.
** @@
******************************************************************************/

typedef struct EnsSMetainformation
{
    ajuint Use;
    ajuint Identifier;
    EnsPMetainformationadaptor Adaptor;
    AjPStr Key;
    AjPStr Value;
    ajuint Species;
    ajuint Padding;
} EnsOMetainformation;

#define EnsPMetainformation EnsOMetainformation*




/*
** Prototype definitions
*/

/* Ensembl Meta-Information */

EnsPMetainformation ensMetainformationNew(EnsPMetainformationadaptor mia,
                                          ajuint identifier,
                                          ajuint species,
                                          AjPStr key,
                                          AjPStr value);

EnsPMetainformation ensMetainformationNewObj(EnsPMetainformation object);

EnsPMetainformation ensMetainformationNewRef(EnsPMetainformation mi);

void ensMetainformationDel(EnsPMetainformation* Pmi);

EnsPMetainformationadaptor ensMetainformationGetAdaptor(
    const EnsPMetainformation mi);

ajuint ensMetainformationGetIdentifier(const EnsPMetainformation mi);

AjPStr ensMetainformationGetKey(const EnsPMetainformation mi);

AjPStr ensMetainformationGetValue(const EnsPMetainformation mi);

ajuint ensMetainformationGetSpecies(const EnsPMetainformation mi);

AjBool ensMetainformationSetAdaptor(EnsPMetainformation mi,
                                    EnsPMetainformationadaptor mia);

AjBool ensMetainformationSetIdentifier(EnsPMetainformation mi,
                                       ajuint identifier);

AjBool ensMetainformationSetKey(EnsPMetainformation mi, AjPStr key);

AjBool ensMetainformationSetValue(EnsPMetainformation mi, AjPStr value);

AjBool ensMetainformationSetSpecies(EnsPMetainformation mi, ajuint species);

AjBool ensMetainformationTrace(const EnsPMetainformation mi, ajuint level);

ajulong ensMetainformationGetMemsize(const EnsPMetainformation mi);

/* Ensembl Meta-Information Adaptor */

EnsPMetainformationadaptor ensRegistryGetMetainformationadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetainformationadaptor ensMetainformationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMetainformationadaptorDel(EnsPMetainformationadaptor* Pmia);

AjBool ensMetainformationadaptorFetchAllByKey(
    const EnsPMetainformationadaptor mia,
    const AjPStr key,
    AjPList mis);

AjBool ensMetainformationadaptorGetValueByKey(
    const EnsPMetainformationadaptor mia,
    const AjPStr key,
    AjPStr *Pvalue);

AjBool ensMetainformationadaptorGetGenebuildVersion(
    const EnsPMetainformationadaptor mia,
    AjPStr *Pvalue);

AjBool ensMetainformationadaptorGetSchemaVersion(
    const EnsPMetainformationadaptor mia,
    AjPStr *Pvalue);

AjBool ensMetainformationadaptorGetTaxonomyIdentifier(
    const EnsPMetainformationadaptor mia,
    AjPStr *Pvalue);

AjBool ensMetainformationadaptorFetchAllSpeciesNames(
    const EnsPMetainformationadaptor mia,
    AjPList names);

AjBool ensMetainformationadaptorKeyValueExists(
    const EnsPMetainformationadaptor mia,
    const AjPStr key,
    const AjPStr value);

/*
** End of prototype definitions
*/




#endif /* ensmetainformation_h */

#ifdef __cplusplus
}
#endif

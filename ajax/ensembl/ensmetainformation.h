
#ifndef ENSMETAINFORMATION_H
#define ENSMETAINFORMATION_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdatabaseadaptor.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPMetainformationadaptor *******************************************
**
** Ensembl Meta-Information Adaptor
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
** Ensembl Meta-Information
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




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Meta-Information */

EnsPMetainformation ensMetainformationNewCpy(const EnsPMetainformation mi);

EnsPMetainformation ensMetainformationNewIni(EnsPMetainformationadaptor mia,
                                             ajuint identifier,
                                             ajuint species,
                                             AjPStr key,
                                             AjPStr value);

EnsPMetainformation ensMetainformationNewRef(EnsPMetainformation mi);

void ensMetainformationDel(EnsPMetainformation* Pmi);

EnsPMetainformationadaptor ensMetainformationGetAdaptor(
    const EnsPMetainformation mi);

ajuint ensMetainformationGetIdentifier(const EnsPMetainformation mi);

AjPStr ensMetainformationGetKey(const EnsPMetainformation mi);

ajuint ensMetainformationGetSpecies(const EnsPMetainformation mi);

AjPStr ensMetainformationGetValue(const EnsPMetainformation mi);

AjBool ensMetainformationSetAdaptor(EnsPMetainformation mi,
                                    EnsPMetainformationadaptor mia);

AjBool ensMetainformationSetIdentifier(EnsPMetainformation mi,
                                       ajuint identifier);

AjBool ensMetainformationSetKey(EnsPMetainformation mi, AjPStr key);

AjBool ensMetainformationSetSpecies(EnsPMetainformation mi, ajuint species);

AjBool ensMetainformationSetValue(EnsPMetainformation mi, AjPStr value);

AjBool ensMetainformationTrace(const EnsPMetainformation mi, ajuint level);

size_t ensMetainformationCalculateMemsize(const EnsPMetainformation mi);

/* Ensembl Meta-Information Adaptor */

EnsPMetainformationadaptor ensRegistryGetMetainformationadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetainformationadaptor ensMetainformationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMetainformationadaptorDel(EnsPMetainformationadaptor* Pmia);

AjBool ensMetainformationadaptorFetchAllbyKey(
    EnsPMetainformationadaptor mia,
    const AjPStr key,
    AjPList mis);

AjBool ensMetainformationadaptorRetrieveAllSpeciesnames(
    EnsPMetainformationadaptor mia,
    AjPList names);

AjBool ensMetainformationadaptorRetrieveGenebuildversion(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveSchemaversion(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveSpeciesCommonname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveSpeciesProductionname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveSpeciesScientificname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveSpeciesShortname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveTaxonomyidentifier(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorRetrieveValue(
    EnsPMetainformationadaptor mia,
    const AjPStr key,
    AjPStr* Pvalue);

AjBool ensMetainformationadaptorCheck(
    EnsPMetainformationadaptor mia,
    const AjPStr key,
    const AjPStr value);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSMETAINFORMATION_H */

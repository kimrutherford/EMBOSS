
#ifndef ENSGVATTRIBUTE_H
#define ENSGVATTRIBUTE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensgvdata.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsEGvattributetypeCode *********************************************
**
** Ensembl Genetic Variation Attribute Type Code enumeration
**
******************************************************************************/

typedef enum EnsOGvattributetypeCode
{
    ensEGvattributetypeCodeNULL,
    ensEGvattributetypeCodeSoaccession,
    ensEGvattributetypeCodeSoterm,
    ensEGvattributetypeCodeDisplayterm
} EnsEGvattributetypeCode;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Attribute */

EnsPGvattribute ensGvattributeNewCpy(const EnsPGvattribute gva);

EnsPGvattribute ensGvattributeNewIni(EnsPGvattributeadaptor gvaa,
                                     ajuint identifier,
                                     EnsPAttributetype at,
                                     AjPStr value);

EnsPGvattribute ensGvattributeNewRef(EnsPGvattribute gva);

void ensGvattributeDel(EnsPGvattribute* Pgva);

EnsPGvattributeadaptor ensGvattributeGetAdaptor(const EnsPGvattribute gva);

EnsPAttributetype ensGvattributeGetAttributetype(const EnsPGvattribute gva);

ajuint ensGvattributeGetIdentifier(const EnsPGvattribute gva);

AjPStr ensGvattributeGetValue(const EnsPGvattribute gva);

AjBool ensGvattributeTrace(const EnsPGvattribute gva, ajuint level);

size_t ensGvattributeCalculateMemsize(const EnsPGvattribute gva);

AjPStr ensGvattributeGetCode(const EnsPGvattribute gva);

AjPStr ensGvattributeGetDescription(const EnsPGvattribute gva);

AjPStr ensGvattributeGetName(const EnsPGvattribute gva);

/* Ensembl Genetic Variation Attribute Adaptor */

EnsPGvattributeadaptor ensRegistryGetGvattributeadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvattributeadaptor ensGvattributeadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensGvattributeadaptorCacheClear(EnsPGvattributeadaptor gvaa);

void ensGvattributeadaptorDel(EnsPGvattributeadaptor* Pgvaa);

EnsPBaseadaptor ensGvattributeadaptorGetBaseadaptor(
    const EnsPGvattributeadaptor gvaa);

EnsPDatabaseadaptor ensGvattributeadaptorGetDatabaseadaptor(
    const EnsPGvattributeadaptor gvaa);

AjBool ensGvattributeadaptorFetchAllbyCode(EnsPGvattributeadaptor gvaa,
                                           const AjPStr code,
                                           const AjPStr value,
                                           AjPList gvas);

AjBool ensGvattributeadaptorFetchByIdentifier(EnsPGvattributeadaptor gvaa,
                                              ajuint identifier,
                                              EnsPGvattribute* Pgva);

const EnsPGvattribute ensGvattributeadaptorGetDisplaytermFromsoaccession(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soaccession);

const EnsPGvattribute ensGvattributeadaptorGetDisplaytermFromsoterm(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soterm);

const EnsPGvattribute ensGvattributeadaptorGetSoaccessionFromsoterm(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soterm);

const EnsPGvattribute ensGvattributeadaptorGetSotermFromsoaccession(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soaccession);

/* Ensembl Genetic Variation Attribute Code */

EnsEGvattributetypeCode ensGvattributetypeCodeFromStr(const AjPStr code);

const char* ensGvattributetypeCodeToChar(EnsEGvattributetypeCode gvatc);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVATTRIBUTE_H */

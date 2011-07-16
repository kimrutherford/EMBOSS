
#ifndef ENSGVSYNONYM_H
#define ENSGVSYNONYM_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensgvdata.h"

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

/* Ensembl Genetic Variation Synonym */

EnsPGvsynonym ensGvsynonymNewCpy(const EnsPGvsynonym gvs);

EnsPGvsynonym ensGvsynonymNewIni(EnsPGvsynonymadaptor gvsa,
                                 ajuint identifier,
                                 EnsPGvsource gvsource,
                                 AjPStr name,
                                 AjPStr moleculetype,
                                 ajuint gvvidentifier,
                                 ajuint subidentifier);

EnsPGvsynonym ensGvsynonymNewRef(EnsPGvsynonym gvs);

void ensGvsynonymDel(EnsPGvsynonym* Pgvs);

EnsPGvsynonymadaptor ensGvsynonymGetAdaptor(const EnsPGvsynonym gvs);

EnsPGvsource ensGvsynonymGetGvsource(const EnsPGvsynonym gvs);

ajuint ensGvsynonymGetGvvariationidentifier(const EnsPGvsynonym gvs);

ajuint ensGvsynonymGetIdentifier(const EnsPGvsynonym gvs);

AjPStr ensGvsynonymGetMoleculetype(const EnsPGvsynonym gvs);

AjPStr ensGvsynonymGetName(const EnsPGvsynonym gvs);

ajuint ensGvsynonymGetSubidentifier(const EnsPGvsynonym gvs);

AjBool ensGvsynonymSetAdaptor(EnsPGvsynonym gvs,
                              EnsPGvsynonymadaptor gvsa);

AjBool ensGvsynonymSetIdentifier(EnsPGvsynonym gvs,
                                 ajuint identifier);

AjBool ensGvsynonymSetGvsource(EnsPGvsynonym gvs,
                               EnsPGvsource gvsource);

AjBool ensGvsynonymSetName(EnsPGvsynonym gvs,
                           AjPStr name);

AjBool ensGvsynonymSetMoleculetype(EnsPGvsynonym gvs,
                                   AjPStr moleculetype);

AjBool ensGvsynonymSetGvvariationidentifier(EnsPGvsynonym gvs,
                                            ajuint gvvidentifier);

AjBool ensGvsynonymSetSubidentifier(EnsPGvsynonym gvs,
                                    ajuint subidentifier);

size_t ensGvsynonymCalculateMemsize(const EnsPGvsynonym gvs);

AjBool ensGvsynonymTrace(const EnsPGvsynonym gvs, ajuint level);

AjBool ensTableGvsynonymClear(AjPTable table);

AjBool ensTableGvsynonymDelete(AjPTable* Ptable);

/* Ensembl Genetic Variation Synonym Adaptor */

EnsPGvsynonymadaptor ensRegistryGetGvsynonymadaptor(
    EnsPDatabaseadaptor dba);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVSYNONYM_H */

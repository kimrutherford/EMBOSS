
#ifndef ENSGVGENOTYPE_H
#define ENSGVGENOTYPE_H

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

/* Ensembl Genetic Variation Genotype */

EnsPGvgenotype ensGvgenotypeNewCpy(const EnsPGvgenotype gvg);

EnsPGvgenotype ensGvgenotypeNewIni(EnsPGvgenotypeadaptor gvga,
                                   ajuint identifier,
                                   AjPStr allele1,
                                   AjPStr allele2);

EnsPGvgenotype ensGvgenotypeNewRef(EnsPGvgenotype gvg);

void ensGvgenotypeDel(EnsPGvgenotype* Pgvg);

EnsPGvgenotypeadaptor ensGvgenotypeGetAdaptor(const EnsPGvgenotype gvg);

AjPStr ensGvgenotypeGetAllele1(const EnsPGvgenotype gvg);

AjPStr ensGvgenotypeGetAllele2(const EnsPGvgenotype gvg);

ajuint ensGvgenotypeGetIdentifier(const EnsPGvgenotype gvg);

AjBool ensGvgenotypeSetAdaptor(EnsPGvgenotype gvg, EnsPGvgenotypeadaptor gvga);

AjBool ensGvgenotypeSetAllele1(EnsPGvgenotype gvg, AjPStr allele1);

AjBool ensGvgenotypeSetAllele2(EnsPGvgenotype gvg, AjPStr allele2);

AjBool ensGvgenotypeSetIdentifier(EnsPGvgenotype gvg, ajuint identifier);

AjBool ensGvgenotypeTrace(const EnsPGvgenotype gvg, ajuint level);

size_t ensGvgenotypeCalculateMemsize(const EnsPGvgenotype gvg);

/* Ensembl Genetic Variation Genotype Adaptor */

EnsPGvgenotypeadaptor ensRegistryGetGvgenotypeadaptor(
    EnsPDatabaseadaptor dba);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVGENOTYPE_H */

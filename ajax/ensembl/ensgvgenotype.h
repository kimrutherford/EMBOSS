/* @include ensgvgenotype *****************************************************
**
** Ensembl Genetic Variation Genotype functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.9 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:07:17 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef ENSGVGENOTYPE_H
#define ENSGVGENOTYPE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensgvdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

AjBool ensUtilityCharSort(char *string);

char ensUtilityBaseAmbiguityFromString(const char *basestr);

const char* ensUtilityBaseAmbiguityToString(char ambiguitychar);

/* Ensembl Genetic Variation Genotype */

EnsPGvgenotype ensGvgenotypeNewCpy(const EnsPGvgenotype gvg);

EnsPGvgenotype ensGvgenotypeNewIni(EnsPGvgenotypeadaptor gvga,
                                   ajuint identifier,
                                   EnsPGvvariation gvv,
                                   AjPList alleles,
                                   AjPStr subhandle,
                                   ajuint subidentifier);

EnsPGvgenotype ensGvgenotypeNewRef(EnsPGvgenotype gvg);

void ensGvgenotypeDel(EnsPGvgenotype *Pgvg);

EnsPGvgenotypeadaptor ensGvgenotypeGetAdaptor(const EnsPGvgenotype gvg);

const AjPList ensGvgenotypeGetAlleles(const EnsPGvgenotype gvg);

EnsPGvvariation ensGvgenotypeGetGvvariation(const EnsPGvgenotype gvg);

ajuint ensGvgenotypeGetIdentifier(const EnsPGvgenotype gvg);

AjPStr ensGvgenotypeGetSubhandle(const EnsPGvgenotype gvg);

ajuint ensGvgenotypeGetSubidentifier(const EnsPGvgenotype gvg);

AjBool ensGvgenotypeSetAdaptor(EnsPGvgenotype gvg,
                               EnsPGvgenotypeadaptor gvga);

AjBool ensGvgenotypeSetGvvariation(EnsPGvgenotype gvg,
                                   EnsPGvvariation gvv);

AjBool ensGvgenotypeSetIdentifier(EnsPGvgenotype gvg,
                                  ajuint identifier);

AjBool ensGvgenotypeSetSubhandle(EnsPGvgenotype gvg,
                                 AjPStr subhandle);

AjBool ensGvgenotypeSetSubidentifier(EnsPGvgenotype gvg,
                                     ajuint subidentifier);

AjBool ensGvgenotypeAddAllele(EnsPGvgenotype gvg,
                              AjPStr allele);

AjBool ensGvgenotypeAddAllelenumber(EnsPGvgenotype gvg,
                                    ajuint number,
                                    AjPStr allele);

AjBool ensGvgenotypeAddGvgenotypecode(EnsPGvgenotype gvg,
                                      EnsPGvgenotypecode gvgc);

AjBool ensGvgenotypeTrace(const EnsPGvgenotype gvg, ajuint level);

const AjPStr ensGvgenotypeCalculateAllelenumber(const EnsPGvgenotype gvg,
                                                ajuint number);

size_t ensGvgenotypeCalculateMemsize(const EnsPGvgenotype gvg);

AjBool ensGvgenotypeFetchAlleleNumber(const EnsPGvgenotype gvg,
                                      ajuint number,
                                      AjPStr *Pallele);

AjBool ensGvgenotypeFetchAlleleString(const EnsPGvgenotype gvg,
                                      AjPStr *Pstring);

/* Ensembl Genetic Variation Genotype Adaptor */

EnsPGvgenotypeadaptor ensRegistryGetGvgenotypeadaptor(
    EnsPDatabaseadaptor dba);

/* Ensembl Genetic Variation Genotype Code */

EnsPGvgenotypecode ensGvgenotypecodeNewCpy(const EnsPGvgenotypecode gvgc);

EnsPGvgenotypecode ensGvgenotypecodeNewIni(EnsPGvgenotypecodeadaptor gvgca,
                                           ajuint identifier,
                                           AjPList alleles);

EnsPGvgenotypecode ensGvgenotypecodeNewRef(EnsPGvgenotypecode gvgc);

void ensGvgenotypecodeDel(EnsPGvgenotypecode *Pgvgc);

EnsPGvgenotypecodeadaptor ensGvgenotypecodeGetAdaptor(
    const EnsPGvgenotypecode gvgc);

const AjPList ensGvgenotypecodeGetAlleles(
    const EnsPGvgenotypecode gvgc);

ajuint ensGvgenotypecodeGetIdentifier(
    const EnsPGvgenotypecode gvgc);

AjBool ensGvgenotypecodeSetAdaptor(EnsPGvgenotypecode gvgc,
                                   EnsPGvgenotypecodeadaptor gvgca);

AjBool ensGvgenotypecodeSetIdentifier(EnsPGvgenotypecode gvgc,
                                      ajuint identifier);

AjBool ensGvgenotypecodeAddAllele(EnsPGvgenotypecode gvgc, AjPStr allele);

AjBool ensGvgenotypecodeTrace(const EnsPGvgenotypecode gvgc, ajuint level);

size_t ensGvgenotypecodeCalculateMemsize(const EnsPGvgenotypecode gvgc);

/* Ensembl Genetic Variation Genotype Code Adaptor */

EnsPGvgenotypecodeadaptor ensRegistryGetGvgenotypecodeadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvgenotypecodeadaptor ensGvgenotypecodeadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvgenotypecodeadaptorDel(
    EnsPGvgenotypecodeadaptor *Pgvgca);

EnsPBaseadaptor ensGvgenotypecodeadaptorGetBaseadaptor(
    EnsPGvgenotypecodeadaptor gvgca);

EnsPDatabaseadaptor ensGvgenotypecodeadaptorGetDatabaseadaptor(
    EnsPGvgenotypecodeadaptor gvgca);

EnsPGvbaseadaptor ensGvgenotypecodeadaptorGetGvbaseadaptor(
    EnsPGvgenotypecodeadaptor gvgca);

AjBool ensGvgenotypecodeadaptorFetchAllbyIdentifiers(
    EnsPGvgenotypecodeadaptor gvgca,
    AjPTable gvgcs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVGENOTYPE_H */

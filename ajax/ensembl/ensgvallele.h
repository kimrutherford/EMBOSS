/* @include ensgvallele *******************************************************
**
** Ensembl Genetic Variation Allele functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.11 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/20 22:09:19 $ by $Author: mks $
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

#ifndef ENSGVALLELE_H
#define ENSGVALLELE_H

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

/* Ensembl Genetic Variation Allele */

EnsPGvallele ensGvalleleNewCpy(const EnsPGvallele gva);

EnsPGvallele ensGvalleleNewIni(EnsPGvalleleadaptor gvaa,
                               ajuint identifier,
                               EnsPGvpopulation gvp,
                               AjPStr allele,
                               float frequency,
                               ajuint subidentifier,
                               ajuint counter,
                               ajuint gvvid);

EnsPGvallele ensGvalleleNewRef(EnsPGvallele gva);

void ensGvalleleDel(EnsPGvallele *Pgva);

EnsPGvalleleadaptor ensGvalleleGetAdaptor(const EnsPGvallele gva);

AjPStr ensGvalleleGetAllele(const EnsPGvallele gva);

ajuint ensGvalleleGetCounter(const EnsPGvallele gva);

EnsPGvpopulation ensGvalleleGetGvpopulation(const EnsPGvallele gva);

ajuint ensGvalleleGetGvvariationidentifier(const EnsPGvallele gva);

ajuint ensGvalleleGetIdentifier(const EnsPGvallele gva);

float ensGvalleleGetFrequency(const EnsPGvallele gva);

ajuint ensGvalleleGetSubidentifier(const EnsPGvallele gva);

const AjPList ensGvalleleLoadAllFaileddescriptions(EnsPGvallele gva);

AjPStr ensGvalleleLoadSubhandle(EnsPGvallele gva);

AjBool ensGvalleleSetAdaptor(EnsPGvallele gva, EnsPGvalleleadaptor gvaa);

AjBool ensGvalleleSetCounter(EnsPGvallele gva, ajuint counter);

AjBool ensGvalleleSetIdentifier(EnsPGvallele gva, ajuint identifier);

AjBool ensGvalleleSetGvpopulation(EnsPGvallele gva, EnsPGvpopulation gvp);

AjBool ensGvalleleSetGvvariationidentifier(EnsPGvallele gva, ajuint gvvid);

AjBool ensGvalleleSetAllele(EnsPGvallele gva, AjPStr allelestr);

AjBool ensGvalleleSetFrequency(EnsPGvallele gva, float frequency);

AjBool ensGvalleleSetSubhandle(EnsPGvallele gva, AjPStr subhandle);

AjBool ensGvalleleSetSubidentifier(EnsPGvallele gva, ajuint subidentifier);

AjBool ensGvalleleTrace(const EnsPGvallele gva, ajuint level);

size_t ensGvalleleCalculateMemsize(const EnsPGvallele gva);

AjBool ensGvalleleIsFailed(EnsPGvallele gva, AjBool *Presult);

/* Ensembl Genetic Variation Allele Adaptor */

EnsPGvalleleadaptor ensRegistryGetGvalleleadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvalleleadaptor ensGvalleleadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvalleleadaptorDel(EnsPGvalleleadaptor *Pgvaa);

EnsPDatabaseadaptor ensGvalleleadaptorGetDatabaseadaptor(
    const EnsPGvalleleadaptor gvaa);

EnsPGvdatabaseadaptor ensGvalleleadaptorGetGvdatabaseadaptor(
    const EnsPGvalleleadaptor gvaa);

AjBool ensGvalleleadaptorFetchAllbyGvvariation(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvvariation gvv,
    const EnsPGvpopulation gvp,
    AjPList gvas);

AjBool ensGvalleleadaptorFetchAllbySubidentifier(
    EnsPGvalleleadaptor gvaa,
    ajuint subidentifier,
    AjPList gvas);

AjBool ensGvalleleadaptorRetrieveAllFaileddescriptions(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvallele gva,
    AjPList descriptions);

AjBool ensGvalleleadaptorRetrieveSubhandle(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvallele gva,
    AjPStr *Psubhandle);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVALLELE_H */

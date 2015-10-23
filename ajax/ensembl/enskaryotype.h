/* @include enskaryotype ******************************************************
**
** Ensembl Karyotype Band functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.21 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:06:37 $ by $Author: mks $
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

#ifndef ENSKARYOTYPE_H
#define ENSKARYOTYPE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPKaryotypebandadaptor ********************************************
**
** Ensembl Karyotype Band Adaptor
** Defined as an alias in EnsPFeatureadaptor
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::KaryotypeBandAdaptor
** ##
******************************************************************************/

#define EnsPKaryotypebandadaptor EnsPFeatureadaptor




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPKaryotypeband ****************************************************
**
** Ensembl Karyotype Band
**
** @alias EnsSKaryotypeband
** @alias EnsOKaryotypeband
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @cc Bio::EnsEMBL::Karyotypeband
** @attr Feature [EnsPFeature] Ensembl Feature
** @attr Name [AjPStr] Name
** @attr Stain [AjPStr] Stain
** @@
******************************************************************************/

typedef struct EnsSKaryotypeband
{
    ajuint Use;
    ajuint Identifier;
    EnsPKaryotypebandadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr Name;
    AjPStr Stain;
} EnsOKaryotypeband;

#define EnsPKaryotypeband EnsOKaryotypeband*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Karyotype Band */

EnsPKaryotypeband ensKaryotypebandNewCpy(const EnsPKaryotypeband kb);

EnsPKaryotypeband ensKaryotypebandNewIni(EnsPKaryotypebandadaptor kba,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         AjPStr name,
                                         AjPStr stain);

EnsPKaryotypeband ensKaryotypebandNewRef(EnsPKaryotypeband kb);

void ensKaryotypebandDel(EnsPKaryotypeband *Pkb);

EnsPKaryotypebandadaptor ensKaryotypebandGetAdaptor(
    const EnsPKaryotypeband kb);

EnsPFeature ensKaryotypebandGetFeature(
    const EnsPKaryotypeband kb);

ajuint ensKaryotypebandGetIdentifier(
    const EnsPKaryotypeband kb);

AjPStr ensKaryotypebandGetName(
    const EnsPKaryotypeband kb);

AjPStr ensKaryotypebandGetStain(
    const EnsPKaryotypeband kb);

AjBool ensKaryotypebandSetAdaptor(EnsPKaryotypeband kb,
                                  EnsPKaryotypebandadaptor kba);

AjBool ensKaryotypebandSetFeature(EnsPKaryotypeband kb,
                                  EnsPFeature feature);

AjBool ensKaryotypebandSetIdentifier(EnsPKaryotypeband kb,
                                     ajuint identifier);

AjBool ensKaryotypebandSetName(EnsPKaryotypeband kb,
                               AjPStr name);

AjBool ensKaryotypebandSetStain(EnsPKaryotypeband kb,
                                AjPStr stain);

AjBool ensKaryotypebandTrace(const EnsPKaryotypeband kb,
                             ajuint level);

size_t ensKaryotypebandCalculateMemsize(const EnsPKaryotypeband kb);

/* AJAX List of Ensembl Karyotype Band objects */

AjBool ensListKaryotypebandSortEndAscending(AjPList kbs);

AjBool ensListKaryotypebandSortEndDescending(AjPList kbs);

AjBool ensListKaryotypebandSortIdentifierAscending(AjPList kbs);

AjBool ensListKaryotypebandSortStartAscending(AjPList kbs);

AjBool ensListKaryotypebandSortStartDescending(AjPList kbs);

/* Ensembl Karyoptype Band Adaptor */

EnsPKaryotypebandadaptor ensRegistryGetKaryotypebandadaptor(
    EnsPDatabaseadaptor dba);

EnsPKaryotypebandadaptor ensKaryotypebandadaptorNew(
    EnsPDatabaseadaptor dba);

void ensKaryotypebandadaptorDel(EnsPKaryotypebandadaptor *Padaptor);

EnsPBaseadaptor ensKaryotypebandadaptorGetBaseadaptor(
    EnsPKaryotypebandadaptor kba);

EnsPDatabaseadaptor ensKaryotypebandadaptorGetDatabaseadaptor(
    EnsPKaryotypebandadaptor kba);

EnsPFeatureadaptor ensKaryotypebandadaptorGetFeatureadaptor(
    EnsPKaryotypebandadaptor kba);

AjBool ensKaryotypebandadaptorFetchAllbyChromosomeband(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    const AjPStr band,
    AjPList kblist);

AjBool ensKaryotypebandadaptorFetchAllbyChromosomename(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    AjPList kblist);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSKARYOTYPE_H */

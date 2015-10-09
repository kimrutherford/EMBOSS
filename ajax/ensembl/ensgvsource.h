/* @include ensgvsource *******************************************************
**
** Ensembl Genetic Variation Source functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.7 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:23 $ by $Author: mks $
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

#ifndef ENSGVSOURCE_H
#define ENSGVSOURCE_H

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

/* Ensembl Genetic Variation Source */

EnsPGvsource ensGvsourceNewCpy(const EnsPGvsource gvs);

EnsPGvsource ensGvsourceNewIni(EnsPGvsourceadaptor gvsa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr version,
                               AjPStr description,
                               AjPStr url,
                               EnsEGvsourceSomatic gvss,
                               EnsEGvsourceType gvst);

EnsPGvsource ensGvsourceNewRef(EnsPGvsource gvs);

void ensGvsourceDel(EnsPGvsource *Pgvs);

EnsPGvsourceadaptor ensGvsourceGetAdaptor(const EnsPGvsource gvs);

AjPStr ensGvsourceGetDescription(const EnsPGvsource gvs);

ajuint ensGvsourceGetIdentifier(const EnsPGvsource gvs);

AjPStr ensGvsourceGetName(const EnsPGvsource gvs);

EnsEGvsourceSomatic ensGvsourceGetSomatic(const EnsPGvsource gvs);

EnsEGvsourceType ensGvsourceGetType(const EnsPGvsource gvs);

AjPStr ensGvsourceGetUrl(const EnsPGvsource gvs);

AjPStr ensGvsourceGetVersion(const EnsPGvsource gvs);

AjBool ensGvsourceSetAdaptor(EnsPGvsource gvs, EnsPGvsourceadaptor gvsa);

AjBool ensGvsourceSetDescription(EnsPGvsource gvs, AjPStr description);

AjBool ensGvsourceSetIdentifier(EnsPGvsource gvs, ajuint identifier);

AjBool ensGvsourceSetName(EnsPGvsource gvs, AjPStr name);

AjBool ensGvsourceSetSomatic(EnsPGvsource gvs, EnsEGvsourceSomatic gvss);

AjBool ensGvsourceSetType(EnsPGvsource gvs, EnsEGvsourceType gvst);

AjBool ensGvsourceSetUrl(EnsPGvsource gvs, AjPStr url);

AjBool ensGvsourceSetVersion(EnsPGvsource gvs, AjPStr version);

AjBool ensGvsourceTrace(const EnsPGvsource gvs, ajuint level);

size_t ensGvsourceCalculateMemsize(const EnsPGvsource gvs);

AjBool ensGvsourceMatch(const EnsPGvsource gvs1, const EnsPGvsource gvs2);

EnsEGvsourceSomatic ensGvsourceSomaticFromStr(
    const AjPStr somatic);

const char *ensGvsourceSomaticToChar(
    EnsEGvsourceSomatic gvss);

EnsEGvsourceType ensGvsourceTypeFromStr(
    const AjPStr type);

const char *ensGvsourceTypeToChar(
    EnsEGvsourceType gvst);

/* Ensembl Genetic Variation Source Adaptor */

EnsPGvsourceadaptor ensRegistryGetGvsourceadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsourceadaptor ensGvsourceadaptorNew(EnsPDatabaseadaptor dba);

void ensGvsourceadaptorDel(EnsPGvsourceadaptor *Pgvsa);

EnsPBaseadaptor ensGvsourceadaptorGetBaseadaptor(
    const EnsPGvsourceadaptor gvsa);

EnsPDatabaseadaptor ensGvsourceadaptorGetDatabaseadaptor(
    const EnsPGvsourceadaptor gvsa);

AjBool ensGvsourceadaptorFetchAll(
    EnsPGvsourceadaptor gvsa,
    AjPList gvss);

AjBool ensGvsourceadaptorFetchAllbyGvsourceType(
    EnsPGvsourceadaptor gvsa,
    EnsEGvsourceType gvst,
    AjPList gvss);

AjBool ensGvsourceadaptorFetchByIdentifier(
    EnsPGvsourceadaptor gvsa,
    ajuint identifier,
    EnsPGvsource *Pgvs);

AjBool ensGvsourceadaptorFetchByName(
    EnsPGvsourceadaptor gvsa,
    const AjPStr name,
    EnsPGvsource *Pgvs);

AjBool ensGvsourceadaptorFetchDefault(
    EnsPGvsourceadaptor gvsa,
    EnsPGvsource *Pgvs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVSOURCE_H */

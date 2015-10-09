/* @include ensexternaldatabase ***********************************************
**
** Ensembl External Database functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.19 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/26 06:38:02 $ by $Author: mks $
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

#ifndef ENSEXTERNALDATABASE_H
#define ENSEXTERNALDATABASE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdata.h"

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

/* Ensembl External Database */

EnsPExternaldatabase ensExternaldatabaseNewCpy(
    const EnsPExternaldatabase edb);

EnsPExternaldatabase ensExternaldatabaseNewIni(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    AjPStr name,
    AjPStr release,
    AjPStr displayname,
    AjPStr secondaryname,
    AjPStr secondarytable,
    AjPStr description,
    EnsEExternaldatabaseStatus status,
    EnsEExternaldatabaseType type,
    ajint priority);

EnsPExternaldatabase ensExternaldatabaseNewRef(EnsPExternaldatabase edb);

void ensExternaldatabaseDel(EnsPExternaldatabase *Pedb);

EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetDescription(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetDisplayname(
    const EnsPExternaldatabase edb);

ajuint ensExternaldatabaseGetIdentifier(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetName(
    const EnsPExternaldatabase edb);

ajint ensExternaldatabaseGetPriority(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetRelease(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondaryname(
    const EnsPExternaldatabase edb);

AjPStr ensExternaldatabaseGetSecondarytable(
    const EnsPExternaldatabase edb);

EnsEExternaldatabaseStatus ensExternaldatabaseGetStatus(
    const EnsPExternaldatabase edb);

EnsEExternaldatabaseType ensExternaldatabaseGetType(
    const EnsPExternaldatabase edb);

AjBool ensExternaldatabaseSetAdaptor(EnsPExternaldatabase edb,
                                     EnsPExternaldatabaseadaptor edba);

AjBool ensExternaldatabaseSetDescription(EnsPExternaldatabase edb,
                                         AjPStr description);

AjBool ensExternaldatabaseSetDisplayname(EnsPExternaldatabase edb,
                                         AjPStr displayname);

AjBool ensExternaldatabaseSetIdentifier(EnsPExternaldatabase edb,
                                        ajuint identifier);

AjBool ensExternaldatabaseSetName(EnsPExternaldatabase edb,
                                  AjPStr name);

AjBool ensExternaldatabaseSetPriority(EnsPExternaldatabase edb,
                                      ajint priority);

AjBool ensExternaldatabaseSetRelease(EnsPExternaldatabase edb,
                                     AjPStr release);

AjBool ensExternaldatabaseSetSecondaryname(EnsPExternaldatabase edb,
                                           AjPStr secondaryname);

AjBool ensExternaldatabaseSetSecondarytable(EnsPExternaldatabase edb,
                                            AjPStr secondarytable);

AjBool ensExternaldatabaseSetStatus(EnsPExternaldatabase edb,
                                    EnsEExternaldatabaseStatus status);

AjBool ensExternaldatabaseSetType(EnsPExternaldatabase edb,
                                  EnsEExternaldatabaseType type);

AjBool ensExternaldatabaseTrace(const EnsPExternaldatabase edb, ajuint level);

size_t ensExternaldatabaseCalculateMemsize(const EnsPExternaldatabase edb);

EnsEExternaldatabaseStatus ensExternaldatabaseStatusFromStr(
    const AjPStr status);

const char *ensExternaldatabaseStatusToChar(EnsEExternaldatabaseStatus edbs);

EnsEExternaldatabaseType ensExternaldatabaseTypeFromStr(
    const AjPStr type);

const char *ensExternaldatabaseTypeToChar(EnsEExternaldatabaseType edbt);

/* Ensembl External Database Adaptor */

EnsPExternaldatabaseadaptor ensRegistryGetExternaldatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPExternaldatabaseadaptor ensExternaldatabaseadaptorNew(
    EnsPDatabaseadaptor dba);

void ensExternaldatabaseadaptorDel(
    EnsPExternaldatabaseadaptor *Pedba);

EnsPBaseadaptor ensExternaldatabaseadaptorGetBaseadaptor(
    const EnsPExternaldatabaseadaptor edba);

EnsPDatabaseadaptor ensExternaldatabaseadaptorGetDatabaseadaptor(
    const EnsPExternaldatabaseadaptor edba);

AjBool ensExternaldatabaseadaptorFetchAll(
    EnsPExternaldatabaseadaptor edba,
    AjPList edbs);

AjBool ensExternaldatabaseadaptorFetchByIdentifier(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    EnsPExternaldatabase *Pedb);

AjBool ensExternaldatabaseadaptorFetchByName(
    EnsPExternaldatabaseadaptor edba,
    const AjPStr name,
    const AjPStr release,
    EnsPExternaldatabase *Pedb);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSEXTERNALDATABASE_H */

/* @include ensregistry *******************************************************
**
** Ensembl Registry functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.25 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/20 22:14:03 $ by $Author: mks $
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

#ifndef ENSREGISTRY_H
#define ENSREGISTRY_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensembl.h"

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

/* Ensembl Registry */

void ensRegistryClear(void);

void ensRegistryExit(void);

void ensRegistryInit(void);

ajuint ensRegistryLoadDatabaseconnection(EnsPDatabaseconnection dbc);

ajuint ensRegistryLoadServername(AjPStr servername);

AjBool ensRegistryRetrieveAllSpecies(AjPList species);

/* Ensembl Registry Alias */

AjBool ensRegistryAliasAdd(const AjPStr species, const AjPStr alias);

AjBool ensRegistryAliasClear(void);

AjBool ensRegistryAliasFetchAllbySpecies(const AjPStr species,
                                         AjPList aliases);

AjBool ensRegistryAliasLoadFile(const AjPStr filename);

AjBool ensRegistryAliasRemove(const AjPStr alias);

AjBool ensRegistryAliasResolve(const AjPStr alias, AjPStr *Pspecies);

AjBool ensRegistryAliasTrace(ajuint level);

/* Ensembl Registry Entry */

AjBool ensRegistryEntryClear(void);

AjBool ensRegistryEntryTrace(ajuint level);

/* Ensembl Registry Identifier */

AjBool ensRegistryIdentifierClear(void);

AjBool ensRegistryIdentifierLoadFile(const AjPStr filename);

AjBool ensRegistryIdentifierResolve(const AjPStr identifier,
                                    AjPStr *Pspecies,
                                    EnsEDatabaseadaptorGroup *Pdbag);

/* Ensembl Registry Source */

AjBool ensRegistrySourceTrace(ajuint level);

/* Ensembl Registry Database Adaptor */

AjBool ensRegistryAddDatabaseadaptor(EnsPDatabaseadaptor dba);

AjBool ensRegistryAddReferenceadaptor(EnsPDatabaseadaptor dba,
                                      EnsPDatabaseadaptor rsa);

AjBool ensRegistryAddStableidentifierprefix(EnsPDatabaseadaptor dba,
                                            const AjPStr prefix);

EnsPDatabaseadaptor ensRegistryNewDatabaseadaptor(
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier);

EnsPDatabaseadaptor ensRegistryNewReferenceadaptor(
    EnsPDatabaseadaptor dba,
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier);

AjBool ensRegistryRemoveDatabaseadaptor(EnsPDatabaseadaptor *Pdba);

AjBool ensRegistryGetAllDatabaseadaptors(EnsEDatabaseadaptorGroup dbag,
                                         AjPStr alias,
                                         AjPList dbas);

AjPStr ensRegistryGetStableidentifierprefix(EnsPDatabaseadaptor dba);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSREGISTRY_H */

/* @include ensbaseadaptor ****************************************************
**
** Ensembl Base Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.24 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:03:48 $ by $Author: mks $
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

#ifndef ENSBASEADAPTOR_H
#define ENSBASEADAPTOR_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensassemblymapper.h"
#include "ensdata.h"
#include "ensslice.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

extern const ajuint ensKBaseadaptorMaximumIdentifiers;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Base Adaptor */

EnsPBaseadaptor ensBaseadaptorNew(
    EnsPDatabaseadaptor dba,
    const char *const *Ptablenames,
    const char *const *Pcolumnnames,
    const EnsPBaseadaptorLeftjoin leftjoins,
    const char *defaultcondition,
    const char *finalcondition,
    AjBool (*Fstatement) (EnsPBaseadaptor ba,
                          const AjPStr statement,
                          EnsPAssemblymapper am,
                          EnsPSlice slice,
                          AjPList objects));

void ensBaseadaptorDel(EnsPBaseadaptor *Pba);

const char *const *ensBaseadaptorGetColumnnames(const EnsPBaseadaptor ba);

EnsPDatabaseadaptor ensBaseadaptorGetDatabaseadaptor(const EnsPBaseadaptor ba);

const char *const *ensBaseadaptorGetTablenames(const EnsPBaseadaptor ba);

AjBool ensBaseadaptorSetColumnnames(EnsPBaseadaptor ba,
                                    const char *const *Pcolumnnames);

AjBool ensBaseadaptorSetDefaultcondition(EnsPBaseadaptor ba,
                                         const char *defaultcondition);

AjBool ensBaseadaptorSetFinalcondition(EnsPBaseadaptor ba,
                                       const char *finalcondition);

AjBool ensBaseadaptorSetTablenames(EnsPBaseadaptor ba,
                                   const char *const *Ptablenames);

AjBool ensBaseadaptorEscapeC(EnsPBaseadaptor ba,
                             char **Ptxt,
                             const AjPStr str);

AjBool ensBaseadaptorEscapeS(EnsPBaseadaptor ba,
                             AjPStr *Pstr,
                             const AjPStr str);

AjBool ensBaseadaptorGetMultispecies(const EnsPBaseadaptor ba);

const char *ensBaseadaptorGetPrimarytable(const EnsPBaseadaptor ba);

ajuint ensBaseadaptorGetSpeciesidentifier(const EnsPBaseadaptor ba);

AjBool ensBaseadaptorCountAll(
    EnsPBaseadaptor ba,
    ajuint *Pcount);

AjBool ensBaseadaptorCountAllbyConstraint(
    EnsPBaseadaptor ba,
    const AjPStr constraint,
    ajuint *Pcount);

AjBool ensBaseadaptorFetchAll(
    EnsPBaseadaptor ba,
    AjPList objects);

AjBool ensBaseadaptorFetchAllbyConstraint(
    EnsPBaseadaptor ba,
    const AjPStr constraint,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList objects);

AjBool ensBaseadaptorFetchAllbyIdentifiers(
    EnsPBaseadaptor ba,
    EnsPSlice slice,
    ajuint (*FobjectGetIdentifier) (const void *object),
    AjPTable objects);

AjBool ensBaseadaptorFetchByIdentifier(
    EnsPBaseadaptor ba,
    ajuint identifier,
    void **Pobject);

AjBool ensBaseadaptorRetrieveAllIdentifiers(
    EnsPBaseadaptor ba,
    const AjPStr tablename,
    const AjPStr columnname,
    AjPList identifiers);

AjBool ensBaseadaptorRetrieveAllStrings(
    EnsPBaseadaptor ba,
    const AjPStr tablename,
    const AjPStr columnname,
    AjPList strings);

AjBool ensBaseadaptorRetrieveFeature(
    EnsPBaseadaptor ba,
    ajuint analysisid,
    ajuint srid,
    ajuint srstart,
    ajuint srend,
    ajint srstrand,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    EnsPFeature *Pfeature);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSBASEADAPTOR_H */

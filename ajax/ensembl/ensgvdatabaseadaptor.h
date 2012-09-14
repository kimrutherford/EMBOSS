/* @include ensgvdatabaseadaptor **********************************************
**
** Ensembl Genetic Variation Database Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.7 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/03/28 21:08:38 $ by $Author: mks $
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

#ifndef ENSGVDATABASEADAPTOR_H
#define ENSGVDATABASEADAPTOR_H

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

/* Ensembl Genetic Variation Database Adaptor */

EnsPGvdatabaseadaptor ensRegistryGetGvdatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvdatabaseadaptor ensGvdatabaseadaptorNewIni(EnsPDatabaseadaptor dba);

void ensGvdatabaseadaptorDel(EnsPGvdatabaseadaptor *Pgvdba);

EnsPDatabaseadaptor ensGvdatabaseadaptorGetDatabaseadaptor(
    const EnsPGvdatabaseadaptor gvdba);

AjBool ensGvdatabaseadaptorGetFailedvariations(
    const EnsPGvdatabaseadaptor gvdba);

AjBool ensGvdatabaseadaptorSetDatabaseadaptor(EnsPGvdatabaseadaptor gvdba,
                                              EnsPDatabaseadaptor dba);

AjBool ensGvdatabaseadaptorSetFailedvariations(EnsPGvdatabaseadaptor gvdba,
                                               AjBool fv);

AjBool ensGvdatabaseadaptorTrace(const EnsPGvdatabaseadaptor gvdba,
                                 ajuint level);

AjBool ensGvdatabaseadaptorFailedallelesconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr *Pconstraint);

AjBool ensGvdatabaseadaptorFailedstructuralsconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr *Pconstraint);

AjBool ensGvdatabaseadaptorFailedvariationsconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr *Pconstraint);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVDATABASEADAPTOR_H */

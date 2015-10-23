/* @include ensgvbaseadaptor **************************************************
**
** Ensembl Genetic Variation Base Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.12 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:16:34 $ by $Author: mks $
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

#ifndef ENSGVBASEADAPTOR_H
#define ENSGVBASEADAPTOR_H

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

/* Ensembl Genetic Variation Base Adaptor */

EnsPGvbaseadaptor ensGvbaseadaptorNew(
    EnsPGvdatabaseadaptor gvdba,
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

void ensGvbaseadaptorDel(EnsPGvbaseadaptor *Pgvba);

EnsPBaseadaptor ensGvbaseadaptorGetBaseadaptor(
    EnsPGvbaseadaptor gvba);

EnsPDatabaseadaptor ensGvbaseadaptorGetDatabaseadaptor(
    EnsPGvbaseadaptor gvba);

AjBool ensGvbaseadaptorGetFailedvariations(
    EnsPGvbaseadaptor gvba);

EnsPGvdatabaseadaptor ensGvbaseadaptorGetGvdatabaseadaptor(
    EnsPGvbaseadaptor gvba);

ajuint ensGvbaseadaptorLoadPloidy(
    EnsPGvbaseadaptor gvba);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVBASEADAPTOR_H */

/* @include ensgvsynonym ******************************************************
**
** Ensembl Genetic Variation Synonym functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.8 $
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

#ifndef ENSGVSYNONYM_H
#define ENSGVSYNONYM_H

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

/* Ensembl Genetic Variation Synonym */

EnsPGvsynonym ensGvsynonymNewCpy(const EnsPGvsynonym gvs);

EnsPGvsynonym ensGvsynonymNewIni(EnsPGvsynonymadaptor gvsa,
                                 ajuint identifier,
                                 EnsPGvsource gvsource,
                                 AjPStr name,
                                 AjPStr moleculetype,
                                 ajuint gvvidentifier,
                                 ajuint subidentifier);

EnsPGvsynonym ensGvsynonymNewRef(EnsPGvsynonym gvs);

void ensGvsynonymDel(EnsPGvsynonym *Pgvs);

EnsPGvsynonymadaptor ensGvsynonymGetAdaptor(const EnsPGvsynonym gvs);

EnsPGvsource ensGvsynonymGetGvsource(const EnsPGvsynonym gvs);

ajuint ensGvsynonymGetGvvariationidentifier(const EnsPGvsynonym gvs);

ajuint ensGvsynonymGetIdentifier(const EnsPGvsynonym gvs);

AjPStr ensGvsynonymGetMoleculetype(const EnsPGvsynonym gvs);

AjPStr ensGvsynonymGetName(const EnsPGvsynonym gvs);

ajuint ensGvsynonymGetSubidentifier(const EnsPGvsynonym gvs);

AjBool ensGvsynonymSetAdaptor(EnsPGvsynonym gvs,
                              EnsPGvsynonymadaptor gvsa);

AjBool ensGvsynonymSetIdentifier(EnsPGvsynonym gvs,
                                 ajuint identifier);

AjBool ensGvsynonymSetGvsource(EnsPGvsynonym gvs,
                               EnsPGvsource gvsource);

AjBool ensGvsynonymSetName(EnsPGvsynonym gvs,
                           AjPStr name);

AjBool ensGvsynonymSetMoleculetype(EnsPGvsynonym gvs,
                                   AjPStr moleculetype);

AjBool ensGvsynonymSetGvvariationidentifier(EnsPGvsynonym gvs,
                                            ajuint gvvidentifier);

AjBool ensGvsynonymSetSubidentifier(EnsPGvsynonym gvs,
                                    ajuint subidentifier);

size_t ensGvsynonymCalculateMemsize(const EnsPGvsynonym gvs);

AjBool ensGvsynonymTrace(const EnsPGvsynonym gvs, ajuint level);

/* Ensembl Genetic Variation Synonym Adaptor */

EnsPGvsynonymadaptor ensRegistryGetGvsynonymadaptor(
    EnsPDatabaseadaptor dba);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVSYNONYM_H */

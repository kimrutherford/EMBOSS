/* @include ensgvattribute ****************************************************
**
** Ensembl Genetic Variation Attribute functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.6 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 10:51:33 $ by $Author: mks $
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

#ifndef ENSGVATTRIBUTE_H
#define ENSGVATTRIBUTE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensgvdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @enum EnsEGvattributetypeCode **********************************************
**
** Ensembl Genetic Variation Attribute Type Code enumeration
**
** @value ensEGvattributetypeCodeNULL Null
** @value ensEGvattributetypeCodeSoaccession Sequence Ontology Accession
** @value ensEGvattributetypeCodeSoterm Sequence Ontology Term
** @value ensEGvattributetypeCodeDisplayterm Display Term
** @@
******************************************************************************/

typedef enum EnsOGvattributetypeCode
{
    ensEGvattributetypeCodeNULL,
    ensEGvattributetypeCodeSoaccession,
    ensEGvattributetypeCodeSoterm,
    ensEGvattributetypeCodeDisplayterm
} EnsEGvattributetypeCode;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Attribute */

EnsPGvattribute ensGvattributeNewCpy(const EnsPGvattribute gva);

EnsPGvattribute ensGvattributeNewIni(EnsPGvattributeadaptor gvaa,
                                     ajuint identifier,
                                     EnsPAttributetype at,
                                     AjPStr value);

EnsPGvattribute ensGvattributeNewRef(EnsPGvattribute gva);

void ensGvattributeDel(EnsPGvattribute *Pgva);

EnsPGvattributeadaptor ensGvattributeGetAdaptor(const EnsPGvattribute gva);

EnsPAttributetype ensGvattributeGetAttributetype(const EnsPGvattribute gva);

ajuint ensGvattributeGetIdentifier(const EnsPGvattribute gva);

AjPStr ensGvattributeGetValue(const EnsPGvattribute gva);

AjBool ensGvattributeTrace(const EnsPGvattribute gva, ajuint level);

size_t ensGvattributeCalculateMemsize(const EnsPGvattribute gva);

AjPStr ensGvattributeGetCode(const EnsPGvattribute gva);

AjPStr ensGvattributeGetDescription(const EnsPGvattribute gva);

AjPStr ensGvattributeGetName(const EnsPGvattribute gva);

/* Ensembl Genetic Variation Attribute Adaptor */

EnsPGvattributeadaptor ensRegistryGetGvattributeadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvattributeadaptor ensGvattributeadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensGvattributeadaptorCacheClear(EnsPGvattributeadaptor gvaa);

void ensGvattributeadaptorDel(EnsPGvattributeadaptor *Pgvaa);

EnsPBaseadaptor ensGvattributeadaptorGetBaseadaptor(
    EnsPGvattributeadaptor gvaa);

EnsPDatabaseadaptor ensGvattributeadaptorGetDatabaseadaptor(
    EnsPGvattributeadaptor gvaa);

AjBool ensGvattributeadaptorFetchAllbyCode(EnsPGvattributeadaptor gvaa,
                                           const AjPStr code,
                                           const AjPStr value,
                                           AjPList gvas);

AjBool ensGvattributeadaptorFetchByIdentifier(EnsPGvattributeadaptor gvaa,
                                              ajuint identifier,
                                              EnsPGvattribute *Pgva);

const EnsPGvattribute ensGvattributeadaptorGetDisplaytermFromsoaccession(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soaccession);

const EnsPGvattribute ensGvattributeadaptorGetDisplaytermFromsoterm(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soterm);

const EnsPGvattribute ensGvattributeadaptorGetSoaccessionFromsoterm(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soterm);

const EnsPGvattribute ensGvattributeadaptorGetSotermFromsoaccession(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soaccession);

/* Ensembl Genetic Variation Attribute Code */

EnsEGvattributetypeCode ensGvattributetypeCodeFromStr(const AjPStr code);

const char *ensGvattributetypeCodeToChar(EnsEGvattributetypeCode gvatc);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVATTRIBUTE_H */

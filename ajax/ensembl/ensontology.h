/* @include ensontology *******************************************************
**
** Ensembl Ontology functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.6 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:16:39 $ by $Author: mks $
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

#ifndef ENSONTOLOGY_H
#define ENSONTOLOGY_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensbaseadaptor.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPOntologytermadaptor *********************************************
**
** Ensembl Ontology Term Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPOntologytermadaptor EnsPBaseadaptor




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPOntologyadaptor **************************************************
**
** Ensembl Ontology Adaptor
**
** @alias EnsSOntologyadaptor
** @alias EnsOOntologyadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByNamespace [AjPTable] Name space cache
** @@
******************************************************************************/

typedef struct EnsSOntologyadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByNamespace;
} EnsOOntologyadaptor;

#define EnsPOntologyadaptor EnsOOntologyadaptor*




/* @data EnsPOntology *********************************************************
**
** Ensembl Ontology
**
** @alias EnsSOntology
** @alias EnsOOntology
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @cc Bio::EnsEMBL::OntologyTerm
** @attr Name [AjPStr] Name
** @attr Space [AjPStr] Name space
** @@
******************************************************************************/

typedef struct EnsSOntology
{
    ajuint Use;
    ajuint Identifier;
    EnsPOntologyadaptor Adaptor;
    AjPStr Name;
    AjPStr Space;
} EnsOOntology;

#define EnsPOntology EnsOOntology*




/* @data EnsPOntologysynonym **************************************************
**
** Ensembl Ontology Synonym
**
** @alias EnsSOntologysynonym
** @alias EnsOOntologysynonym
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @cc Bio::EnsEMBL::OntologyTerm
** @attr Name [AjPStr] Name
** @@
******************************************************************************/

typedef struct EnsSOntologysynonym
{
    ajuint Use;
    ajuint Identifier;
    EnsPOntologytermadaptor Adaptor;
    AjPStr Name;
} EnsOOntologysynonym;

#define EnsPOntologysynonym EnsOOntologysynonym*




/* @data EnsPOntologyterm *****************************************************
**
** Ensembl Ontology Term
**
** @alias EnsSOntologyterm
** @alias EnsOOntologyterm
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @cc Bio::EnsEMBL::OntologyTerm
** @attr Ontology [EnsPOntology] Ensembl Ontology
** @attr Accession [AjPStr] Accession
** @attr Definition [AjPStr] Definition
** @attr Name [AjPStr] Name
** @attr Subsets [AjPStr] Subsets
** @attr Children [AjPTable] AJAX Table of child Ensembl Ontology Term objects
** @attr Parents [AjPTable] AJAX Table of parent Ensembl Ontology Term objects
** @attr Ontologysynonyms [AjPList]
** AJAX List of Ensembl Ontology Synonym objects
** @@
******************************************************************************/

typedef struct EnsSOntologyterm
{
    ajuint Use;
    ajuint Identifier;
    EnsPOntologytermadaptor Adaptor;
    EnsPOntology Ontology;
    AjPStr Accession;
    AjPStr Definition;
    AjPStr Name;
    AjPStr Subsets;
    AjPTable Children;
    AjPTable Parents;
    AjPList Ontologysynonyms;
} EnsOOntologyterm;

#define EnsPOntologyterm EnsOOntologyterm*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Ontology */

EnsPOntology ensOntologyNewCpy(const EnsPOntology ontology);

EnsPOntology ensOntologyNewIni(EnsPOntologyadaptor oa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr space);

EnsPOntology ensOntologyNewRef(EnsPOntology ontology);

void ensOntologyDel(EnsPOntology *Pontology);

EnsPOntologyadaptor ensOntologyGetAdaptor(const EnsPOntology ontology);

ajuint ensOntologyGetIdentifier(const EnsPOntology ontology);

AjPStr ensOntologyGetName(const EnsPOntology ontology);

AjPStr ensOntologyGetSpace(const EnsPOntology ontology);

AjBool ensOntologySetAdaptor(EnsPOntology ontology,
                             EnsPOntologyadaptor oa);

AjBool ensOntologySetIdentifier(EnsPOntology ontology,
                                ajuint identifier);

AjBool ensOntologySetName(EnsPOntology ontology,
                          AjPStr name);

AjBool ensOntologySetSpace(EnsPOntology ontology,
                           AjPStr space);

AjBool ensOntologyTrace(const EnsPOntology ontology, ajuint level);

size_t ensOntologyCalculateMemsize(const EnsPOntology ontology);

/* Ensembl Ontology Adaptor */

EnsPOntologyadaptor ensRegistryGetOntologyadaptor(
    EnsPDatabaseadaptor dba);

EnsPOntologyadaptor ensOntologyadaptorNew(
    EnsPDatabaseadaptor dba);

void ensOntologyadaptorDel(EnsPOntologyadaptor *Poa);

EnsPBaseadaptor ensOntologyadaptorGetBaseadaptor(
    EnsPOntologyadaptor oa);

EnsPDatabaseadaptor ensOntologyadaptorGetDatabaseadaptor(
    EnsPOntologyadaptor oa);

AjBool ensOntologyadaptorFetchAll(EnsPOntologyadaptor oa,
                                  AjPList ontologies);

AjBool ensOntologyadaptorFetchAllbyName(EnsPOntologyadaptor oa,
                                        const AjPStr name,
                                        AjPList ontologies);

AjBool ensOntologyadaptorFetchByIdentifier(EnsPOntologyadaptor oa,
                                           ajuint identifier,
                                           EnsPOntology *Pontology);

AjBool ensOntologyadaptorFetchByName(EnsPOntologyadaptor oa,
                                     const AjPStr name,
                                     const AjPStr space,
                                     EnsPOntology *Pontology);

/* Ensembl Ontology Synonym */

EnsPOntologysynonym ensOntologysynonymNewCpy(const EnsPOntologysynonym os);

EnsPOntologysynonym ensOntologysynonymNewIni(EnsPOntologytermadaptor ota,
                                             ajuint identifier,
                                             AjPStr name);

EnsPOntologysynonym ensOntologysynonymNewRef(EnsPOntologysynonym os);

void ensOntologysynonymDel(EnsPOntologysynonym *Pos);

EnsPOntologytermadaptor ensOntologysynonymGetAdaptor(
    const EnsPOntologysynonym os);

ajuint ensOntologysynonymGetIdentifier(
    const EnsPOntologysynonym os);

AjPStr ensOntologysynonymGetName(
    const EnsPOntologysynonym os);

AjBool ensOntologysynonymSetAdaptor(EnsPOntologysynonym os,
                                    EnsPOntologytermadaptor ota);

AjBool ensOntologysynonymSetIdentifier(EnsPOntologysynonym os,
                                       ajuint identifier);

AjBool ensOntologysynonymSetName(EnsPOntologysynonym os,
                                 AjPStr name);

AjBool ensOntologysynonymTrace(const EnsPOntologysynonym os, ajuint level);

size_t ensOntologysynonymCalculateMemsize(const EnsPOntologysynonym os);

/* Ensembl Ontology Term */

EnsPOntologyterm ensOntologytermNewCpy(const EnsPOntologyterm ot);

EnsPOntologyterm ensOntologytermNewIni(EnsPOntologytermadaptor ota,
                                       ajuint identifier,
                                       EnsPOntology ontology,
                                       AjPStr accession,
                                       AjPStr description,
                                       AjPStr name,
                                       AjPStr subsets);

EnsPOntologyterm ensOntologytermNewRef(EnsPOntologyterm ot);

void ensOntologytermDel(EnsPOntologyterm *Pot);

AjPStr ensOntologytermGetAccession(const EnsPOntologyterm ot);

EnsPOntologytermadaptor ensOntologytermGetAdaptor(const EnsPOntologyterm ot);

AjPStr ensOntologytermGetDefinition(const EnsPOntologyterm ot);

ajuint ensOntologytermGetIdentifier(const EnsPOntologyterm ot);

AjPStr ensOntologytermGetName(const EnsPOntologyterm ot);

EnsPOntology ensOntologytermGetOntology(const EnsPOntologyterm ot);

AjPStr ensOntologytermGetSubsets(const EnsPOntologyterm ot);

const AjPList ensOntologytermLoadOntologysynonyms(EnsPOntologyterm ot);

AjBool ensOntologytermSetAccession(EnsPOntologyterm ot,
                                   AjPStr accession);

AjBool ensOntologytermSetAdaptor(EnsPOntologyterm ot,
                                 EnsPOntologytermadaptor ota);

AjBool ensOntologytermSetDefinition(EnsPOntologyterm ot,
                                    AjPStr description);

AjBool ensOntologytermSetIdentifier(EnsPOntologyterm ot,
                                    ajuint identifier);

AjBool ensOntologytermSetName(EnsPOntologyterm ot,
                              AjPStr name);

AjBool ensOntologytermSetOntology(EnsPOntologyterm ot,
                                  EnsPOntology ontology);

AjBool ensOntologytermSetSubsets(EnsPOntologyterm ot,
                                 AjPStr subsets);

AjBool ensOntologytermTrace(const EnsPOntologyterm ot, ajuint level);

size_t ensOntologytermCalculateMemsize(const EnsPOntologyterm ot);

/* Ensembl Ontology Term Adaptor */

EnsPOntologytermadaptor ensRegistryGetOntologytermadaptor(
    EnsPDatabaseadaptor dba);

EnsPOntologytermadaptor ensOntologytermadaptorNew(
    EnsPDatabaseadaptor dba);

void ensOntologytermadaptorDel(EnsPOntologytermadaptor *Pota);

EnsPBaseadaptor ensOntologytermadaptorGetBaseadaptor(
    EnsPOntologytermadaptor ota);

EnsPDatabaseadaptor ensOntologytermadaptorGetDatabaseadaptor(
    EnsPOntologytermadaptor ota);

AjBool ensOntologytermadaptorFetchAll(
    EnsPOntologytermadaptor ota,
    AjPList ots);

AjBool ensOntologytermadaptorFetchAllbyAncestor(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    AjPList ots);

AjBool ensOntologytermadaptorFetchAllbyChild(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    AjPList ots);

AjBool ensOntologytermadaptorFetchAllbyDescendant(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    const AjPStr subset,
    AjBool closest,
    AjBool zerodistance,
    AjPList ots);

AjBool ensOntologytermadaptorFetchAllbyName(
    EnsPOntologytermadaptor ota,
    const AjPStr termname,
    const AjPStr ontologyname,
    AjPList ots);

AjBool ensOntologytermadaptorFetchAllbyParent(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    AjPList ots);

AjBool ensOntologytermadaptorFetchByAccession(
    EnsPOntologytermadaptor ota,
    const AjPStr accession,
    EnsPOntologyterm *Pot);

AjBool ensOntologytermadaptorFetchByIdentifier(
    EnsPOntologytermadaptor ota,
    ajuint identifier,
    EnsPOntologyterm *Pot);

AjBool ensOntologytermadaptorRetrieveAllOntologysynonyms(
    EnsPOntologytermadaptor ota,
    const EnsPOntologyterm ot,
    AjPList oss);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSONTOLOGY_H */

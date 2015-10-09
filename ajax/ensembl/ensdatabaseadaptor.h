/* @include ensdatabaseadaptor ************************************************
**
** Ensembl Database Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.27 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/03/09 20:33:24 $ by $Author: mks $
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

#ifndef ENSDATABASEADAPTOR_H
#define ENSDATABASEADAPTOR_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdatabaseconnection.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

extern const char *ensKDatabaseadaptorRegCollection;
extern const char *ensKDatabaseadaptorRegMulti;
extern const char *ensKDatabaseadaptorRegSpecies;




/* @const EnsMDatabaseadaptorGroups *******************************************
**
** Number of Ensembl Database Adaptor groups
**
** The number of Ensembl Database Adaptor groups has to correlate with the
** EnsEDatabaseadaptorGroup enumeration list and the
** static const char *databaseadaptorGroup[] array. It is used by the
** ensregistry.[ch] module.
**
******************************************************************************/

#define EnsMDatabaseadaptorGroups 17




/* @enum EnsEDatabaseadaptorGroup *********************************************
**
** Ensembl Database Adaptor Group enumeration
**
** @value ensEDatabaseadaptorGroupNULL Null
** @value ensEDatabaseadaptorGroupCore Core
** @value ensEDatabaseadaptorGroupVega Vega
** @value ensEDatabaseadaptorGroupOtherFeatures Other features
** @value ensEDatabaseadaptorGroupCopyDNA DNA reference
** @value ensEDatabaseadaptorGroupGeneticVariation Genetic variation
** @value ensEDatabaseadaptorGroupFunctionalGenomics Functional genomics
** @value ensEDatabaseadaptorGroupComparativeGenomics Comparative genomics
** @value ensEDatabaseadaptorGroupOntology Ontology
** @value ensEDatabaseadaptorGroupQualityCheck Quality check
** @value ensEDatabaseadaptorGroupPipeline Pipleine
** @value ensEDatabaseadaptorGroupHive Hive
** @value ensEDatabaseadaptorGroupCoreExpressionEST Core expression EST
** @value ensEDatabaseadaptorGroupCoreExpressionGNF Core expression GNF
** @value ensEDatabaseadaptorGroupAncestral Ancestral
** @value ensEDatabaseadaptorGroupWebsite Webiste
** @value ensEDatabaseadaptorGroupProduction Production
** @@
******************************************************************************/

typedef enum EnsODatabaseadaptorGroup
{
    ensEDatabaseadaptorGroupNULL,
    ensEDatabaseadaptorGroupCore,
    ensEDatabaseadaptorGroupVega,
    ensEDatabaseadaptorGroupOtherFeatures,
    ensEDatabaseadaptorGroupCopyDNA,
    ensEDatabaseadaptorGroupGeneticVariation,
    ensEDatabaseadaptorGroupFunctionalGenomics,
    ensEDatabaseadaptorGroupComparativeGenomics,
    ensEDatabaseadaptorGroupOntology,
    ensEDatabaseadaptorGroupQualityCheck,
    ensEDatabaseadaptorGroupPipeline,
    ensEDatabaseadaptorGroupHive,
    ensEDatabaseadaptorGroupCoreExpressionEST,
    ensEDatabaseadaptorGroupCoreExpressionGNF,
    ensEDatabaseadaptorGroupAncestral,
    ensEDatabaseadaptorGroupWebsite,
    ensEDatabaseadaptorGroupProduction
} EnsEDatabaseadaptorGroup;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPDatabaseadaptor **************************************************
**
** Ensembl Database Adaptor.
**
** Holds the Ensembl Database Group, the species name and the Ensembl
** Database Connection object connected to this SQL database.
**
** @alias EnsSDatabaseadaptor
** @alias EnsODatabaseadaptor
**
** @alias EnsPAttributeadaptor
** @alias EnsPGvgenotypeadaptor
** @alias EnsPGvsynonymadaptor
**
** @attr Databaseconnection [EnsPDatabaseconnection]
** Ensembl Database Connection
** @attr Species [AjPStr] Species
** @attr SpeciesNames [AjPList]
** AJAX List of AJAX String (species name) objects
** @attr Group [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @attr Multispecies [AjBool] Multi-species database
** @attr Identifier [ajuint] Species identifier, defaults to 1U
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDatabaseadaptor
{
    EnsPDatabaseconnection Databaseconnection;
    AjPStr Species;
    AjPList SpeciesNames;
    EnsEDatabaseadaptorGroup Group;
    AjBool Multispecies;
    ajuint Identifier;
    ajuint Padding;
} EnsODatabaseadaptor;

#define EnsPDatabaseadaptor EnsODatabaseadaptor*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Database Adaptor */

EnsPDatabaseadaptor ensRegistryGetDatabaseadaptor(
    EnsEDatabaseadaptorGroup dbag,
    AjPStr alias);

EnsPDatabaseadaptor ensRegistryGetReferenceadaptor(EnsPDatabaseadaptor dba);

EnsPDatabaseadaptor ensDatabaseadaptorNewDatabasename(
    EnsPDatabaseconnection dbc,
    AjPStr database);

EnsPDatabaseadaptor ensDatabaseadaptorNewIni(EnsPDatabaseconnection dbc,
                                             AjPStr database,
                                             AjPStr species,
                                             EnsEDatabaseadaptorGroup dbag,
                                             AjBool multi,
                                             ajuint identifier);

EnsPDatabaseadaptor ensDatabaseadaptorNewUrl(const AjPStr url);

void ensDatabaseadaptorDel(EnsPDatabaseadaptor *Pdba);

EnsPDatabaseconnection ensDatabaseadaptorGetDatabaseconnection(
    const EnsPDatabaseadaptor dba);

EnsEDatabaseadaptorGroup ensDatabaseadaptorGetGroup(
    const EnsPDatabaseadaptor dba);

ajuint ensDatabaseadaptorGetIdentifier(const EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorGetMultispecies(const EnsPDatabaseadaptor dba);

AjPStr ensDatabaseadaptorGetSpecies(const EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorSetDatabaseconnection(EnsPDatabaseadaptor dba,
                                               EnsPDatabaseconnection dbc);

AjBool ensDatabaseadaptorSetGroup(EnsPDatabaseadaptor dba,
                                  EnsEDatabaseadaptorGroup dbag);

AjBool ensDatabaseadaptorSetIdentifier(EnsPDatabaseadaptor dba,
                                       ajuint identifier);

AjBool ensDatabaseadaptorSetMultispecies(EnsPDatabaseadaptor dba,
                                         AjBool multi);

AjBool ensDatabaseadaptorSetSpecies(EnsPDatabaseadaptor dba, AjPStr species);

AjBool ensDatabaseadaptorTrace(const EnsPDatabaseadaptor dba, ajuint level);

AjBool ensDatabaseadaptorMatch(const EnsPDatabaseadaptor dba1,
                               const EnsPDatabaseadaptor dba2);

AjBool ensDatabaseadaptorMatchcomponents(const EnsPDatabaseadaptor dba,
                                         const EnsPDatabaseconnection dbc,
                                         const AjPStr species,
                                         EnsEDatabaseadaptorGroup dbag,
                                         AjBool multi,
                                         ajuint identifier);

AjBool ensDatabaseadaptorDisconnect(EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorEscapeC(EnsPDatabaseadaptor dba,
                                 char **Ptxt,
                                 const AjPStr str);

AjBool ensDatabaseadaptorEscapeS(EnsPDatabaseadaptor dba,
                                 AjPStr *Pstr,
                                 const AjPStr str);

const AjPList ensDatabaseadaptorGetAllSpeciesnames(EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorSqlstatementDel(EnsPDatabaseadaptor dba,
                                         AjPSqlstatement *Psqls);

AjPSqlstatement ensDatabaseadaptorSqlstatementNew(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement);

AjBool ensDatabaseadaptorFetchSchemabuild(EnsPDatabaseadaptor dba,
                                          AjPStr *Pbuild);

EnsEDatabaseadaptorGroup ensDatabaseadaptorGroupFromStr(const AjPStr group);

const char *ensDatabaseadaptorGroupToChar(EnsEDatabaseadaptorGroup dbag);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSDATABASEADAPTOR_H */

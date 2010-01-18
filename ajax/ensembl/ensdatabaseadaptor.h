#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensdatabaseadaptor_h
#define ensdatabaseadaptor_h

#include "ensdatabaseconnection.h"




/******************************************************************************
**
** Ensembl Database Adaptor group enumeration.
**
******************************************************************************/

enum EnsEDatabaseadaptorGroup
{
    ensEDatabaseadaptorGroupNULL,
    ensEDatabaseadaptorGroupCore,
    ensEDatabaseadaptorGroupVega,
    ensEDatabaseadaptorGroupOtherFeatures,
    ensEDatabaseadaptorGroupCopyDNA,
    ensEDatabaseadaptorGroupGeneticVariation,
    ensEDatabaseadaptorGroupFunctionalGenomics,
    ensEDatabaseadaptorGroupComparativeGenomics,
    ensEDatabaseadaptorGroupGeneOntology,
    ensEDatabaseadaptorGroupQualityCheck,
    ensEDatabaseadaptorGroupPipeline,
    ensEDatabaseadaptorGroupHive,
    ensEDatabaseadaptorGroupCoreExpressionEST,
    ensEDatabaseadaptorGroupCoreExpressionGNF,
    ensEDatabaseadaptorGroupAncestral,
    ensEDatabaseadaptorGroupWebsite
};




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
** @attr Databaseconnection [EnsPDatabaseconnection] Ensembl Database
**                                                   Connection
** @attr Species [AjPStr] Species
** @attr Group [AjEnum] Group
** @attr MultiSpecies [AjBool] Multi-species database
** @attr Identifier [ajuint] Species identifier, defaults to 1
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDatabaseadaptor
{
    EnsPDatabaseconnection Databaseconnection;
    AjPStr Species;
    AjEnum Group;
    AjBool MultiSpecies;
    ajuint Identifier;
    ajuint Padding;
} EnsODatabaseadaptor;

#define EnsPDatabaseadaptor EnsODatabaseadaptor*




/*
** Prototype definitions
*/

EnsPDatabaseadaptor ensDatabaseadaptorNew(EnsPDatabaseconnection dbc,
                                          AjPStr database,
                                          AjPStr species,
                                          AjEnum group,
                                          AjBool multi,
                                          ajuint identifier);

void ensDatabaseadaptorDel(EnsPDatabaseadaptor* Pdba);

EnsPDatabaseconnection ensDatabaseadaptorGetDatabaseconnection(
    const EnsPDatabaseadaptor dba);

AjPStr ensDatabaseadaptorGetSpecies(const EnsPDatabaseadaptor dba);

AjEnum ensDatabaseadaptorGetGroup(const EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorGetMultiSpecies(const EnsPDatabaseadaptor dba);

ajuint ensDatabaseadaptorGetIdentifier(const EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorSetDatabaseconnection(EnsPDatabaseadaptor dba,
                                               EnsPDatabaseconnection dbc);

AjBool ensDatabaseadaptorSetSpecies(EnsPDatabaseadaptor dba, AjPStr species);

AjBool ensDatabaseadaptorSetGroup(EnsPDatabaseadaptor dba, AjEnum group);

AjBool ensDatabaseadaptorSetMultiSpecies(EnsPDatabaseadaptor dba, AjBool multi);

AjBool ensDatabaseadaptorSetIdentifier(EnsPDatabaseadaptor dba,
                                       ajuint identifier);

AjEnum ensDatabaseadaptorGroupFromStr(const AjPStr group);

const char *ensDatabaseadaptorGroupToChar(const AjEnum group);

AjBool ensDatabaseadaptorMatch(const EnsPDatabaseadaptor dba1,
			const EnsPDatabaseadaptor dba2);

AjBool ensDatabaseadaptorMatchComponents(const EnsPDatabaseadaptor dba,
                                         const EnsPDatabaseconnection dbc,
                                         const AjPStr species,
                                         AjEnum group,
                                         AjBool multi,
                                         ajuint identifier);

AjPSqlstatement ensDatabaseadaptorSqlstatementNew(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement);

AjBool ensDatabaseadaptorEscapeC(EnsPDatabaseadaptor dba,
                                  char **Ptxt,
                                  const AjPStr str);

AjBool ensDatabaseadaptorEscapeS(EnsPDatabaseadaptor dba,
                                  AjPStr *Pstr,
                                  const AjPStr str);

AjBool ensDatabaseadaptorTrace(const EnsPDatabaseadaptor dba, ajuint level);

AjBool ensDatabaseadaptorGetSchemaBuild(const EnsPDatabaseadaptor dba,
                                        AjPStr *Pbuild);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

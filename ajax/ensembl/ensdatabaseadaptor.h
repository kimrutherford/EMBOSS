#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensdatabaseadaptor_h
#define ensdatabaseadaptor_h

#include "ensdatabaseconnection.h"




/* EnsMDatabaseadaptorGroups **************************************************
**
** Number of Ensembl Database Adaptor groups.
**
** The number of Ensembl Database Adaptor groups has to correlate with the
** EnsEDatabaseadaptorGroup enumeration list and the
** static const char *databaseadaptorGroup[] array. It is used by the
** ensregistry.[ch] module.
**
******************************************************************************/

#define EnsMDatabaseadaptorGroups 17




/* EnsEDatabaseadaptorGroup ***************************************************
**
** Ensembl Database Adaptor group enumeration.
**
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
** @alias EnsPDitagadaptor
** @alias EnsPRepeatconsensusadaptor
** @alias EnsPGvalleleadaptor
** @alias EnsPGvgenotypeadaptor
** @alias EnsPGvvariationadaptor
**
** @attr Databaseconnection [EnsPDatabaseconnection] Ensembl Database
**                                                   Connection
** @attr Species [AjPStr] Species
** @attr SpeciesNames [AjPList] AJAX List of species name AJAX Strings
** @attr Group [EnsEDatabaseadaptorGroup] Group
** @attr MultiSpecies [AjBool] Multi-species database
** @attr Identifier [ajuint] Species identifier, defaults to 1
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDatabaseadaptor
{
    EnsPDatabaseconnection Databaseconnection;
    AjPStr Species;
    AjPList SpeciesNames;
    EnsEDatabaseadaptorGroup Group;
    AjBool MultiSpecies;
    ajuint Identifier;
    ajuint Padding;
} EnsODatabaseadaptor;

#define EnsPDatabaseadaptor EnsODatabaseadaptor*




/*
** Prototype definitions
*/

/* Ensembl Database Adaptor */

EnsPDatabaseadaptor ensRegistryGetDatabaseadaptor(
    EnsEDatabaseadaptorGroup group,
    const AjPStr alias);

EnsPDatabaseadaptor ensRegistryGetReferenceadaptor(EnsPDatabaseadaptor dba);

EnsPDatabaseadaptor ensDatabaseadaptorNew(EnsPDatabaseconnection dbc,
                                          AjPStr database,
                                          AjPStr species,
                                          EnsEDatabaseadaptorGroup group,
                                          AjBool multi,
                                          ajuint identifier);

void ensDatabaseadaptorDel(EnsPDatabaseadaptor* Pdba);

EnsPDatabaseconnection ensDatabaseadaptorGetDatabaseconnection(
    const EnsPDatabaseadaptor dba);

AjPStr ensDatabaseadaptorGetSpecies(const EnsPDatabaseadaptor dba);

EnsEDatabaseadaptorGroup ensDatabaseadaptorGetGroup(
    const EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorGetMultiSpecies(const EnsPDatabaseadaptor dba);

ajuint ensDatabaseadaptorGetIdentifier(const EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorSetDatabaseconnection(EnsPDatabaseadaptor dba,
                                               EnsPDatabaseconnection dbc);

AjBool ensDatabaseadaptorSetSpecies(EnsPDatabaseadaptor dba, AjPStr species);

AjBool ensDatabaseadaptorSetGroup(EnsPDatabaseadaptor dba,
                                  EnsEDatabaseadaptorGroup group);

AjBool ensDatabaseadaptorSetMultiSpecies(EnsPDatabaseadaptor dba,
                                         AjBool multi);

AjBool ensDatabaseadaptorSetIdentifier(EnsPDatabaseadaptor dba,
                                       ajuint identifier);

EnsEDatabaseadaptorGroup ensDatabaseadaptorGroupFromStr(const AjPStr group);

const char *ensDatabaseadaptorGroupToChar(EnsEDatabaseadaptorGroup group);

AjBool ensDatabaseadaptorMatch(const EnsPDatabaseadaptor dba1,
                               const EnsPDatabaseadaptor dba2);

AjBool ensDatabaseadaptorMatchComponents(const EnsPDatabaseadaptor dba,
                                         const EnsPDatabaseconnection dbc,
                                         const AjPStr species,
                                         EnsEDatabaseadaptorGroup group,
                                         AjBool multi,
                                         ajuint identifier);

AjPSqlstatement ensDatabaseadaptorSqlstatementNew(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement);

AjBool ensDatabaseadaptorSqlstatementDel(EnsPDatabaseadaptor dba,
                                         AjPSqlstatement *Psqls);

AjBool ensDatabaseadaptorDisconnect(EnsPDatabaseadaptor dba);

AjBool ensDatabaseadaptorEscapeC(EnsPDatabaseadaptor dba,
                                 char **Ptxt,
                                 const AjPStr str);

AjBool ensDatabaseadaptorEscapeS(EnsPDatabaseadaptor dba,
                                 AjPStr *Pstr,
                                 const AjPStr str);

AjBool ensDatabaseadaptorTrace(const EnsPDatabaseadaptor dba, ajuint level);

AjBool ensDatabaseadaptorGetSchemaBuild(const EnsPDatabaseadaptor dba,
                                        AjPStr *Pbuild);

const AjPList ensDatabaseadaptorGetAllSpeciesNames(EnsPDatabaseadaptor dba);

/*
** End of prototype definitions
*/




#endif /* ensdatabaseadaptor_h */

#ifdef __cplusplus
}
#endif

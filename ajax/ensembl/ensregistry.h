#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensregistry_h
#define ensregistry_h

#include "ajax.h"
#include "ensdatabaseadaptor.h"
#include "ensanalysis.h"
#include "ensassemblyexception.h"
#include "ensassemblymapper.h"
#include "enscoordsystem.h"
#include "ensdatabaseentry.h"
#include "ensdensity.h"
#include "ensditag.h"
#include "ensexon.h"
#include "ensexternaldatabase.h"
#include "ensgene.h"
#include "ensgvdata.h"
#include "ensgvsample.h"
#include "ensgvindividual.h"
#include "ensgvpopulation.h"
#include "enskaryotype.h"
#include "ensmarker.h"
#include "ensmetacoordinate.h"
#include "ensmetainformation.h"
#include "ensmiscellaneous.h"
#include "ensprediction.h"
#include "ensqc.h"
#include "ensrepeat.h"
#include "enssequence.h"
#include "ensseqregion.h"
#include "ensslice.h"
#include "enstranscript.h"
#include "enstranslation.h"
#include "ensvariation.h"




/*
** Prototype definitions
*/

/* Ensembl Registry */

void ensRegistryInit(void);

void ensRegistryClear(void);

void ensRegistryExit(void);

/* Ensembl Registry Aliases */

AjBool ensRegistryAddAlias(const AjPStr species, const AjPStr alias);

AjBool ensRegistryRemoveAlias(const AjPStr alias);

AjPStr ensRegistryGetSpecies(const AjPStr alias);

AjBool ensRegistryLoadAliases(void);

AjBool ensRegistryTraceAliases(ajuint level);

AjBool ensRegistryTraceEntries(ajuint level);

/* Ensembl Database Adaptors */

AjBool ensRegistryAddDatabaseadaptor(EnsPDatabaseconnection dbc,
                                     AjPStr database,
                                     AjPStr alias,
                                     AjEnum group,
                                     AjBool multi,
                                     ajuint identifier);

AjBool ensRegistryRemoveDatabaseadaptor(EnsPDatabaseadaptor *Pdba);

AjBool ensRegistryGetAllDatabaseadaptors(AjEnum group,
                                         const AjPStr alias,
                                         AjPList list);

EnsPDatabaseadaptor ensRegistryGetDatabaseadaptor(AjEnum group,
                                                  const AjPStr alias);



/* Excluded in C file */
/* EnsPDatabaseadaptor ensRegistryGetDnaadaptor(EnsPDatabaseadaptor dba); */

/* Ensembl Core-style Adaptors */
    
EnsPAnalysisadaptor ensRegistryGetAnalysisadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblyexceptionadaptor ensRegistryGetAssemblyexceptionadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblyexceptionfeatureadaptor ensRegistryGetAssemblyexceptionfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblymapperadaptor ensRegistryGetAssemblymapperadaptor(
    EnsPDatabaseadaptor dba);

EnsPCoordsystemadaptor ensRegistryGetCoordsystemadaptor(
    EnsPDatabaseadaptor dba);

EnsPDatabaseentryadaptor ensRegistryGetDatabaseentryadaptor(
    EnsPDatabaseadaptor dba);

EnsPDensityfeatureadaptor ensRegistryGetDensityfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPDensitytypeadaptor ensRegistryGetDensitytypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPDitagfeatureadaptor ensRegistryGetDitagfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPDNAAlignFeatureadaptor ensRegistryGetDNAAlignFeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPExonadaptor ensRegistryGetExonadaptor(
    EnsPDatabaseadaptor dba);

EnsPExternaldatabaseadaptor ensRegistryGetExternaldatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPGeneadaptor ensRegistryGetGeneadaptor(
    EnsPDatabaseadaptor dba);

EnsPKaryotypebandadaptor ensRegistryGetKaryotypebandadaptor(
    EnsPDatabaseadaptor dba);

EnsPMarkeradaptor ensRegistryGetMarkeradaptor(
    EnsPDatabaseadaptor dba);

EnsPMarkerfeatureadaptor ensRegistryGetMarkerfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetainformationadaptor ensRegistryGetMetainformationadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetacoordinateadaptor ensRegistryGetMetacoordinateadaptor(
    EnsPDatabaseadaptor dba);

EnsPMiscellaneousfeatureadaptor ensRegistryGetMiscellaneousfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPMiscellaneoussetadaptor ensRegistryGetMiscellaneoussetadaptor(
    EnsPDatabaseadaptor dba);

EnsPPredictionexonadaptor ensRegistryGetPredictionexonadaptor(
    EnsPDatabaseadaptor dba);

EnsPPredictiontranscriptadaptor ensRegistryGetPredictiontranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPProteinalignfeatureadaptor ensRegistryGetProteinalignfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPProteinfeatureadaptor ensRegistryGetProteinfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPRepeatfeatureadaptor ensRegistryGetRepeatfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPSeqregionadaptor ensRegistryGetSeqregionadaptor(
    EnsPDatabaseadaptor dba);

EnsPSequenceadaptor ensRegistryGetSequenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPSimplefeatureadaptor ensRegistryGetSimplefeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPSliceadaptor ensRegistryGetSliceadaptor(
    EnsPDatabaseadaptor dba);

EnsPTranscriptadaptor ensRegistryGetTranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPTranslationadaptor ensRegistryGetTranslationadaptor(
    EnsPDatabaseadaptor dba);

/* Ensembl Quality Check Adaptors */

EnsPQcdatabaseadaptor ensRegistryGetQcdatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcsequenceadaptor ensRegistryGetQcsequenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcalignmentadaptor ensRegistryGetQcalignmentadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcdasfeatureadaptor ensRegistryGetQcdasfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcvariationadaptor ensRegistryGetQcvariationadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcsubmissionadaptor ensRegistryGetQcsubmissionadaptor(
    EnsPDatabaseadaptor dba);

/* Ensembl Genetic Variation Adaptors */

EnsPGvalleleadaptor ensRegistryGetGvalleleadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvgenotypeadaptor ensRegistryGetGvgenotypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvindividualadaptor ensRegistryGetGvindividualadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvpopulationadaptor ensRegistryGetGvpopulationadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsampleadaptor ensRegistryGetGvsampleadaptor(
    EnsPDatabaseadaptor dba);
    
EnsPGvvariationadaptor ensRegistryGetGvvariationadaptor(
    EnsPDatabaseadaptor dba);

AjBool ensRegistryLoadFromServer(EnsPDatabaseconnection dbc);

AjBool ensRegistryLoadIdentifiers(void);

AjBool ensRegistryGetSpeciesGroup(const AjPStr identifier,
                                  AjPStr *Pspecies,
                                  AjEnum *Pgroup);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

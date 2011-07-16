/* @source Ensembl Registry functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/08 11:43:41 $ by $Author: mks $
** @version $Revision: 1.46 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensregistry.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* @datastatic RegistryPComparativeGenomics ***********************************
**
** Registry Comparative Genomics structure.
**
** Holds an Ensembl Database Adaptor and associated Ensembl Object Adaptors
** specific for Ensembl Comparative Genomics databases.
**
** @alias RegistrySComparativeGenomics
** @alias RegistryOComparativeGenomics
**
** @attr Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @@
******************************************************************************/

typedef struct RegistrySComparativeGenomics
{
    EnsPDatabaseadaptor Databaseadaptor;
} RegistryOComparativeGenomics;

#define RegistryPComparativeGenomics RegistryOComparativeGenomics*




/* @datastatic RegistryPCoreStyle *********************************************
**
** Registry Core-Style structure.
**
** Holds an Ensembl Database Adaptor and associated Ensembl Object Adaptors
** specific for Ensembl Core-Style databases.
**
** @alias RegistrySCoreStyle
** @alias RegistryOCoreStyle
**
** @attr Stableidentifierprefix [AjPStr]
** Ensembl Stable Identifier Prefix
** @attr Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @attr Referenceadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @attr Analysisadaptor [EnsPAnalysisadaptor]
** Ensembl Analysis Adaptor
** @attr Assemblyexceptionadaptor [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @attr Assemblyexceptionfeatureadaptor [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor
** @attr Assemblymapperadaptor [EnsPAssemblymapperadaptor]
** Ensembl Assembly Mapper Adaptor
** @attr Attributetypeadaptor [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor
** @attr Coordsystemadaptor [EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor
** @attr Databaseentryadaptor [EnsPDatabaseentryadaptor]
** Ensembl Database Entry Adaptor
** @attr Densityfeatureadaptor [EnsPDensityfeatureadaptor]
** Ensembl Density Feature Adaptor
** @attr Densitytypeadaptor [EnsPDensitytypeadaptor]
** Ensembl Density Type Adaptor
** @attr Ditagadaptor [EnsPDitagadaptor]
** Ensembl Ditag Adaptor
** @attr Ditagfeatureadaptor [EnsPDitagfeatureadaptor]
** Ensembl Ditag Feature Adaptor
** @attr Dnaalignfeatureadaptor [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Alignment Feature Adaptor
** @attr Exonadaptor [EnsPExonadaptor]
** Ensembl Exon Adaptor
** @attr Externaldatabaseadaptor [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
** @attr Geneadaptor [EnsPGeneadaptor]
** Ensembl Gene Adaptor
** @attr Karyotypebandadaptor [EnsPKaryotypebandadaptor]
** Ensembl Karyotype Band Adaptor
** @attr Markeradaptor [EnsPMarkeradaptor]
** Ensembl Marker Adaptor
** @attr Markerfeatureadaptor [EnsPMarkerfeatureadaptor]
** Ensembl Marker Feature Adaptor
** @attr Metacoordinateadaptor [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor
** @attr Metainformationadaptor [EnsPMetainformationadaptor]
** Ensembl Meta-Information Adaptor
** @attr Miscellaneousfeatureadaptor [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @attr Miscellaneoussetadaptor [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @attr Predictionexonadaptor [EnsPPredictionexonadaptor]
** Ensembl Prediction Exon Adaptor
** @attr Predictiontranscriptadaptor [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
** @attr Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Alignment Feature Adaptor
** @attr Proteinfeatureadaptor [EnsPProteinfeatureadaptor]
** Ensembl Protein Feature Adaptor
** @attr Repeatconsensusadaptor [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor
** @attr Repeatfeatureadaptor [EnsPRepeatfeatureadaptor]
** Ensembl Repeat Feature Adaptor
** @attr Seqregionadaptor [EnsPSeqregionadaptor]
** Ensembl Sequence Region Adaptor
** @attr Seqregionsynonymadaptor [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @attr Sequenceadaptor [EnsPSequenceadaptor]
** Ensembl Sequence Adaptor
** @attr Simplefeatureadaptor [EnsPSimplefeatureadaptor]
** Ensembl Simple Feature Adaptor
** @attr Sliceadaptor [EnsPSliceadaptor]
** Ensembl Slice Adaptor
** @attr Transcriptadaptor [EnsPTranscriptadaptor]
** Ensembl Transcript Adaptor
** @attr Translationadaptor [EnsPTranslationadaptor]
** Ensembl Translation Adaptor
** @@
******************************************************************************/

typedef struct RegistrySCoreStyle
{
    AjPStr Stableidentifierprefix;
    EnsPDatabaseadaptor Databaseadaptor;
    EnsPDatabaseadaptor Referenceadaptor;
    EnsPAnalysisadaptor Analysisadaptor;
    EnsPAssemblyexceptionadaptor Assemblyexceptionadaptor;
    EnsPAssemblyexceptionfeatureadaptor Assemblyexceptionfeatureadaptor;
    EnsPAssemblymapperadaptor Assemblymapperadaptor;
    EnsPAttributetypeadaptor Attributetypeadaptor;
    EnsPCoordsystemadaptor Coordsystemadaptor;
    EnsPDatabaseentryadaptor Databaseentryadaptor;
    EnsPDensityfeatureadaptor Densityfeatureadaptor;
    EnsPDensitytypeadaptor Densitytypeadaptor;
    EnsPDitagadaptor Ditagadaptor;
    EnsPDitagfeatureadaptor Ditagfeatureadaptor;
    EnsPDnaalignfeatureadaptor Dnaalignfeatureadaptor;
    EnsPExonadaptor Exonadaptor;
    EnsPExternaldatabaseadaptor Externaldatabaseadaptor;
    EnsPGeneadaptor Geneadaptor;
    EnsPKaryotypebandadaptor Karyotypebandadaptor;
    EnsPMarkeradaptor Markeradaptor;
    EnsPMarkerfeatureadaptor Markerfeatureadaptor;
    EnsPMetacoordinateadaptor Metacoordinateadaptor;
    EnsPMetainformationadaptor Metainformationadaptor;
    EnsPMiscellaneousfeatureadaptor Miscellaneousfeatureadaptor;
    EnsPMiscellaneoussetadaptor Miscellaneoussetadaptor;
    EnsPPredictionexonadaptor Predictionexonadaptor;
    EnsPPredictiontranscriptadaptor Predictiontranscriptadaptor;
    EnsPProteinalignfeatureadaptor Proteinalignfeatureadaptor;
    EnsPProteinfeatureadaptor Proteinfeatureadaptor;
    EnsPRepeatconsensusadaptor Repeatconsensusadaptor;
    EnsPRepeatfeatureadaptor Repeatfeatureadaptor;
    EnsPSeqregionadaptor Seqregionadaptor;
    EnsPSeqregionsynonymadaptor Seqregionsynonymadaptor;
    EnsPSequenceadaptor Sequenceadaptor;
    EnsPSimplefeatureadaptor Simplefeatureadaptor;
    EnsPSliceadaptor Sliceadaptor;
    EnsPTranscriptadaptor Transcriptadaptor;
    EnsPTranslationadaptor Translationadaptor;
} RegistryOCoreStyle;

#define RegistryPCoreStyle RegistryOCoreStyle*




/* @datastatic RegistryPEntry *************************************************
**
** Registry Entry structure.
**
** Holds Ensembl Database Adaptor Group-specific Registry objects.
**
** @alias RegistrySEntry
** @alias RegistryOEntry
**
** @attr Registry [void*[EnsMDatabaseadaptorGroups]] Array of database-specific
**                Registry Entry objects, indexed by the
**                Ensembl Database Adaptor Group enumeration.
** @@
******************************************************************************/

typedef struct RegistrySEntry
{
    void* Registry[EnsMDatabaseadaptorGroups];
} RegistryOEntry;

#define RegistryPEntry RegistryOEntry*




/* @datastatic RegistryPFunctionalGenomics ************************************
**
** Registry Functional Genomics structure.
**
** Holds an Ensembl Database Adaptors and associated Ensembl Object Adaptors
** specific for Ensembl Functional Genomics databases.
**
** @alias RegistrySFunctionalGenomics
** @alias RegistryOFunctionalGenomics
**
** @attr Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @@
******************************************************************************/

typedef struct RegistrySFunctionalGenomics
{
    EnsPDatabaseadaptor Databaseadaptor;
} RegistryOFunctionalGenomics;

#define RegistryPFunctionalGenomics RegistryOFunctionalGenomics*




/* @datastatic RegistryPGeneticVariation **************************************
**
** Registry Genetic Variation structure.
**
** Holds an Ensembl Database Adaptor and associated Ensembl Object Adaptors
** specific for Ensembl Genetic Variation databases.
**
** @alias RegistrySGeneticVariation
** @alias RegistryOGeneticVariation
**
** @attr Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @attr Gvalleleadaptor [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @attr Gvattributeadaptor [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @attr Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @attr Gvindividualadaptor [EnsPGvindividualadaptor]
** Ensembl Genetic Variation Individual Adaptor
** @attr Gvpopulationadaptor [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @attr Gvpopulationgenotypeadaptor [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor
** @attr Gvsampleadaptor [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor
** @attr Gvsourceadaptor [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
** @attr Gvtranscriptvariationadaptor [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
** @attr Gvvariationfeatureadaptor [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @attr Gvvariationsetadaptor [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @@
******************************************************************************/

typedef struct RegistrySGeneticVariation
{
    EnsPDatabaseadaptor Databaseadaptor;
    EnsPGvalleleadaptor Gvalleleadaptor;
    EnsPGvattributeadaptor Gvattributeadaptor;
    EnsPGvdatabaseadaptor Gvdatabaseadaptor;
    EnsPGvindividualadaptor Gvindividualadaptor;
    EnsPGvpopulationadaptor Gvpopulationadaptor;
    EnsPGvpopulationgenotypeadaptor Gvpopulationgenotypeadaptor;
    EnsPGvsampleadaptor Gvsampleadaptor;
    EnsPGvsourceadaptor Gvsourceadaptor;
    EnsPGvtranscriptvariationadaptor Gvtranscriptvariationadaptor;
    EnsPGvvariationfeatureadaptor Gvvariationfeatureadaptor;
    EnsPGvvariationsetadaptor Gvvariationsetadaptor;
} RegistryOGeneticVariation;

#define RegistryPGeneticVariation RegistryOGeneticVariation*




/* @datastatic RegistryPIdentifier ********************************************
**
** Registry Identifier structure.
**
** Holds regular expressions for matching Ensembl Gene, Transcript,
** Translation and Exon stable identifiers, as well as
** Ensembl Database Adaptor species and group information.
**
** @alias RegistrySIdentifier
** @alias RegistryOIdentifier
**
** @attr RegularExpression [AjPStr] Regular expression
** @attr SpeciesName [AjPStr] Ensembl Database Adaptor species element
** @attr Group [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct RegistrySIdentifier
{
    AjPStr RegularExpression;
    AjPStr SpeciesName;
    EnsEDatabaseadaptorGroup Group;
    ajuint Padding;
} RegistryOIdentifier;

#define RegistryPIdentifier RegistryOIdentifier*




/* @datastatic RegistryPOntology **********************************************
**
** Registry Ontology structure.
**
** Holds an Ensembl Database Adaptor and associated Ensembl Object Adaptors
** specific for Ensembl Ontology databases.
**
** @alias RegistrySOntology
** @alias RegistryOOntology
**
** @attr Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @@
******************************************************************************/

typedef struct RegistrySOntology
{
    EnsPDatabaseadaptor Databaseadaptor;
} RegistryOOntology;

#define RegistryPOntology RegistryOOntology*




/* @datastatic RegistryPQualityCheck ******************************************
**
** Registry Quality Check structure.
**
** Holds an Ensembl Database Adaptor and associated Ensembl Object Adaptors
** specific for Ensembl Quality Check databases.
**
** @alias RegistrySQualityCheck
** @alias RegistryOQualityCheck
**
** @attr Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @attr Qcalignmentadaptor [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @attr Qcdasfeatureadaptor [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor
** @attr Qcdatabaseadaptor [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @attr Qcsequenceadaptor [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @attr Qcsubmissionadaptor [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @attr Qcvariationadaptor [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @@
******************************************************************************/

typedef struct RegistrySQualityCheck
{
    EnsPDatabaseadaptor Databaseadaptor;
    EnsPQcalignmentadaptor Qcalignmentadaptor;
    EnsPQcdasfeatureadaptor Qcdasfeatureadaptor;
    EnsPQcdatabaseadaptor Qcdatabaseadaptor;
    EnsPQcsequenceadaptor Qcsequenceadaptor;
    EnsPQcsubmissionadaptor Qcsubmissionadaptor;
    EnsPQcvariationadaptor Qcvariationadaptor;
} RegistryOQualityCheck;

#define RegistryPQualityCheck RegistryOQualityCheck*




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic registryAliasMetaKey ******************************************
**
** Register species aliases from the following Ensembl Meta-Information keys.
**
******************************************************************************/

static const char* const registryAliasMetaKey[] =
{
    "assembly.name",
    "species.alias",
    "species.common_name",
    "species.stable_id_prefix",
    "species.taxonomy_id",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */

/* #varstatic registryAlias ***************************************************
**
** AJAX Table storing
** AJAX String object (alias) key data and
** AJAX String object (species name) value data.
**
**   key data:   AJAX String object (alias)
**   value data: AJAX String object (species name)
**
******************************************************************************/

static AjPTable registryAlias = NULL;




/* #varstatic registryEntry ***************************************************
**
** AJAX Table storing
** AJAX String object (species name) key data and
** Registry Entry object value data.
**
**   key data:   AJAX String object (species name)
**   value data: Registry Entry object
**
******************************************************************************/

static AjPTable registryEntry = NULL;




/* #varstatic registryIdentifier **********************************************
**
** AJAX List storing Registry Identifier objects.
**
******************************************************************************/

static AjPList registryIdentifier = NULL;




/* #varstatic registrySource **************************************************
**
** AJAX List storing AJAX String objects representing sources that have been
** used to populate the Ensembl Registry. Keeping track of sources prevents
** multiple attempts to initialise and potentially overwrite the
** Ensembl Registry.
**
** Valid sources are:
**
**   * AJAX String object representation of Ensembl Database Connection object
**     URLs used in ensRegistryLoadDatabaseconnection.
**
******************************************************************************/

static AjPList registrySource = NULL;




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void registryEntryDel(
    RegistryPEntry *Pentry);

static AjBool registryEntryTrace(
    const RegistryPEntry entry,
    ajuint level);

static RegistryPIdentifier registryIdentifierNew(
    AjPStr expression,
    AjPStr species,
    EnsEDatabaseadaptorGroup dbag);

static void registryIdentifierDel(
    RegistryPIdentifier *Pri);

static void registryCoreStyleDel(
    RegistryPCoreStyle *Prcs);

static AjBool registryCoreStyleTrace(
    const RegistryPCoreStyle rcs,
    ajuint level);

static void registryComparativeGenomicsDel(
    RegistryPComparativeGenomics *Prcg);

static AjBool registryComparativeGenomicsTrace(
    const RegistryPComparativeGenomics rcg,
    ajuint level);

static void registryFunctionalGenomicsDel(
    RegistryPFunctionalGenomics *Prfg);

static AjBool registryFunctionalGenomicsTrace(
    const RegistryPFunctionalGenomics rfg,
    ajuint level);

static void registryGeneticVariationDel(
    RegistryPGeneticVariation *Prgv);

static AjBool registryGeneticVariationTrace(
    const RegistryPGeneticVariation rgv,
    ajuint level);

static void registryOntologyDel(
    RegistryPOntology *Pro);

static AjBool registryOntologyTrace(
    const RegistryPOntology ro,
    ajuint level);

static void registryQualityCheckDel(
    RegistryPQualityCheck *Pqc);

static AjBool registryQualityCheckTrace(
    const RegistryPQualityCheck rqc,
    ajuint level);

static AjPStr registryAliasRegister(const AjPStr alias);

static AjBool registryAliasLoadDatabaseconnection(
    EnsPDatabaseconnection dbc,
    EnsPDatabaseadaptor dba);

static AjBool registryEntryLoadCollection(EnsPDatabaseconnection dbc,
                                          AjPStr dbname,
                                          EnsEDatabaseadaptorGroup dbag);

static void tableRegistryAliasClear(void** key,
                                    void** value,
                                    void* cl);

static void tableRegistryEntryClear(void** key,
                                    void** value,
                                    void* cl);

static AjBool registrySourceRegister(const AjPStr source,
                                     AjBool* Pregistered);

static int registryStringCompareCase(const void* P1, const void* P2);

static void registryStringDelete(void** PP1, void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensregistry ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
** @nam2rule Registry Functions for manipulating the Ensembl Registry
**
******************************************************************************/




/* @funcstatic registryComparativeGenomicsDel *********************************
**
** Default destructor for a Registry Comparative Genomics object.
**
** @param [d] Prcg [RegistryPComparativeGenomics*] Registry Comparative
**                                                 Genomics object address
**
** @return [void]
** @@
******************************************************************************/

static void registryComparativeGenomicsDel(
    RegistryPComparativeGenomics *Prcg)
{
    RegistryPComparativeGenomics pthis = NULL;

    if(!Prcg)
        return;

    if(!*Prcg)
        return;

    if(ajDebugTest("registryComparativeGenomicsDel"))
    {
        ajDebug("registryComparativeGenomicsDel\n"
                "  *Prcg %p\n",
                *Prcg);

        registryComparativeGenomicsTrace(*Prcg, 1);
    }

    pthis = *Prcg;

    /* Delete all Ensembl Object Adaptors based on the Database Adaptor. */

    /* Finally, delete the Ensembl Database Adaptor. */

    ensDatabaseadaptorDel(&pthis->Databaseadaptor);

    AJFREE(pthis);

    *Prcg = NULL;

    return;
}




/* @funcstatic registryComparativeGenomicsTrace *******************************
**
** Trace a Registry Comparative Genomics object.
**
** @param [r] rcg [const RegistryPComparativeGenomics]
** Registry Comparative Genomics object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryComparativeGenomicsTrace(
    const RegistryPComparativeGenomics rcg,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!rcg)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%registryComparativeGenomicsTrace %p\n"
            "%S  Databaseadaptor %p\n",
            indent, rcg,
            indent, rcg->Databaseadaptor);

    ensDatabaseadaptorTrace(rcg->Databaseadaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic registryCoreStyleDel *******************************************
**
** Default destructor for a Registry Core-Style object.
**
** @param [d] Prcs [RegistryPCoreStyle*] Registry Core-Style object address
**
** @return [void]
** @@
******************************************************************************/

static void registryCoreStyleDel(
    RegistryPCoreStyle *Prcs)
{
    RegistryPCoreStyle pthis = NULL;

    if(!Prcs)
        return;

    if(!*Prcs)
        return;

    if(ajDebugTest("registryCoreStyleDel"))
    {
        ajDebug("registryCoreStyleDel\n"
                "  *Prcs %p\n",
                *Prcs);

        registryCoreStyleTrace(*Prcs, 1);
    }

    pthis = *Prcs;

    ajStrDel(&pthis->Stableidentifierprefix);

    /* Delete all Ensembl Object Adaptors based on Database Adaptors. */

    ensAnalysisadaptorDel(
        &pthis->Analysisadaptor);

    ensAssemblyexceptionadaptorDel(
        &pthis->Assemblyexceptionadaptor);

    ensAssemblyexceptionfeatureadaptorDel(
        &pthis->Assemblyexceptionfeatureadaptor);

    ensAssemblymapperadaptorDel(
        &pthis->Assemblymapperadaptor);

    ensAttributetypeadaptorDel(
        &pthis->Attributetypeadaptor);

    ensCoordsystemadaptorDel(
        &pthis->Coordsystemadaptor);

    ensDatabaseentryadaptorDel(
        &pthis->Databaseentryadaptor);

    ensDensityfeatureadaptorDel(
        &pthis->Densityfeatureadaptor);

    ensDensitytypeadaptorDel(
        &pthis->Densitytypeadaptor);

    ensDitagadaptorDel(
        &pthis->Ditagadaptor);

    ensDitagfeatureadaptorDel(
        &pthis->Ditagfeatureadaptor);

    ensDnaalignfeatureadaptorDel(
        &pthis->Dnaalignfeatureadaptor);

    ensExonadaptorDel(
        &pthis->Exonadaptor);

    ensExternaldatabaseadaptorDel(
        &pthis->Externaldatabaseadaptor);

    ensGeneadaptorDel(
        &pthis->Geneadaptor);

    ensKaryotypebandadaptorDel(
        &pthis->Karyotypebandadaptor);

    ensMarkeradaptorDel(
        &pthis->Markeradaptor);

    ensMarkerfeatureadaptorDel(
        &pthis->Markerfeatureadaptor);

    ensMetainformationadaptorDel(
        &pthis->Metainformationadaptor);

    ensMetacoordinateadaptorDel(
        &pthis->Metacoordinateadaptor);

    ensMiscellaneousfeatureadaptorDel(
        &pthis->Miscellaneousfeatureadaptor);

    ensMiscellaneoussetadaptorDel(
        &pthis->Miscellaneoussetadaptor);

    ensPredictionexonadaptorDel(
        &pthis->Predictionexonadaptor);

    ensPredictiontranscriptadaptorDel(
        &pthis->Predictiontranscriptadaptor);

    ensProteinalignfeatureadaptorDel(
        &pthis->Proteinalignfeatureadaptor);

    ensProteinfeatureadaptorDel(
        &pthis->Proteinfeatureadaptor);

    ensRepeatconsensusadaptorDel(
        &pthis->Repeatconsensusadaptor);

    ensRepeatfeatureadaptorDel(
        &pthis->Repeatfeatureadaptor);

    ensSeqregionadaptorDel(
        &pthis->Seqregionadaptor);

    ensSeqregionsynonymadaptorDel(
        &pthis->Seqregionsynonymadaptor);

    ensSequenceadaptorDel(
        &pthis->Sequenceadaptor);

    ensSimplefeatureadaptorDel(
        &pthis->Simplefeatureadaptor);

    ensSliceadaptorDel(
        &pthis->Sliceadaptor);

    ensTranscriptadaptorDel(
        &pthis->Transcriptadaptor);

    ensTranslationadaptorDel(
        &pthis->Translationadaptor);

    /* Finally, delete the Ensembl Database Adaptors. */

    ensDatabaseadaptorDel(
        &pthis->Referenceadaptor);

    ensDatabaseadaptorDel(
        &pthis->Databaseadaptor);

    AJFREE(pthis);

    *Prcs = NULL;

    return;
}




/* @funcstatic registryCoreStyleTrace *****************************************
**
** Trace a Registry Core-Style object.
**
** @param [r] rcs [const RegistryPCoreStyle] Registry Core-Style object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryCoreStyleTrace(
    const RegistryPCoreStyle rcs,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!rcs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SregistryCoreStyleTrace %p\n"
            "%S  Stableidentifierprefix '%S'\n"
            "%S  Databaseadaptor %p\n"
            "%S  Referenceadaptor %p\n"
            "%S  Analysisadaptor %p\n"
            "%S  Assemblyexceptionadaptor %p\n"
            "%S  Assemblyexceptionfeatureadaptor %p\n"
            "%S  Assemblymapperadaptor %p\n"
            "%S  Coordsystemadaptor %p\n"
            "%S  Databaseentryadaptor %p\n"
            "%S  Dnaalignfeatureadaptor %p\n"
            "%S  Exonadaptor %p\n"
            "%S  Externaldatabaseadaptor %p\n"
            "%S  Geneadaptor %p\n"
            "%S  Karyotypebandadaptor %p\n"
            "%S  Metainformationadaptor %p\n"
            "%S  Metacoordinateadaptor %p\n"
            "%S  Miscellaneousfeatureadaptor %p\n"
            "%S  Miscellaneoussetadaptor %p\n"
            "%S  Proteinalignfeatureadaptor %p\n"
            "%S  Proteinfeatureadaptor %p\n"
            "%S  Repeatfeatureadaptor %p\n"
            "%S  Seqregionadaptor %p\n"
            "%S  Seqregionsynonymadaptor %p\n"
            "%S  Sequenceadaptor %p\n"
            "%S  Simplefeatureadaptor %p\n"
            "%S  Sliceadaptor %p\n"
            "%S  Transcriptadaptor %p\n"
            "%S  Translationadaptor %p\n",
            indent, rcs,
            indent, rcs->Stableidentifierprefix,
            indent, rcs->Databaseadaptor,
            indent, rcs->Referenceadaptor,
            indent, rcs->Analysisadaptor,
            indent, rcs->Assemblyexceptionadaptor,
            indent, rcs->Assemblyexceptionfeatureadaptor,
            indent, rcs->Assemblymapperadaptor,
            indent, rcs->Coordsystemadaptor,
            indent, rcs->Databaseentryadaptor,
            indent, rcs->Dnaalignfeatureadaptor,
            indent, rcs->Exonadaptor,
            indent, rcs->Externaldatabaseadaptor,
            indent, rcs->Geneadaptor,
            indent, rcs->Karyotypebandadaptor,
            indent, rcs->Metainformationadaptor,
            indent, rcs->Metacoordinateadaptor,
            indent, rcs->Miscellaneousfeatureadaptor,
            indent, rcs->Miscellaneoussetadaptor,
            indent, rcs->Proteinalignfeatureadaptor,
            indent, rcs->Proteinfeatureadaptor,
            indent, rcs->Repeatfeatureadaptor,
            indent, rcs->Seqregionadaptor,
            indent, rcs->Seqregionsynonymadaptor,
            indent, rcs->Sequenceadaptor,
            indent, rcs->Simplefeatureadaptor,
            indent, rcs->Sliceadaptor,
            indent, rcs->Transcriptadaptor,
            indent, rcs->Translationadaptor);

    ensDatabaseadaptorTrace(rcs->Databaseadaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic registryEntryDel ***********************************************
**
** Default destructor for a Registry Entry object.
**
** @param [d] Pentry [RegistryPEntry*] Registry Entry object address
**
** @return [void]
** @@
******************************************************************************/

static void registryEntryDel(
    RegistryPEntry *Pentry)
{
    RegistryPEntry pthis = NULL;

    if(!Pentry)
        return;

    if(!*Pentry)
        return;

    if(ajDebugTest("registryEntryDel"))
    {
        ajDebug("registryEntryDel\n"
                "  *Pentry %p\n",
                *Pentry);

        registryEntryTrace(*Pentry, 1);
    }

    pthis = *Pentry;

    registryCoreStyleDel(
        (RegistryPCoreStyle *)
        &pthis->Registry[ensEDatabaseadaptorGroupCore]);

    registryCoreStyleDel(
        (RegistryPCoreStyle *)
        &pthis->Registry[ensEDatabaseadaptorGroupVega]);

    registryCoreStyleDel(
        (RegistryPCoreStyle *)
        &pthis->Registry[ensEDatabaseadaptorGroupOtherFeatures]);

    registryCoreStyleDel(
        (RegistryPCoreStyle *)
        &pthis->Registry[ensEDatabaseadaptorGroupCopyDNA]);

    registryGeneticVariationDel(
        (RegistryPGeneticVariation *)
        &pthis->Registry[ensEDatabaseadaptorGroupGeneticVariation]);

    registryFunctionalGenomicsDel(
        (RegistryPFunctionalGenomics *)
        &pthis->Registry[ensEDatabaseadaptorGroupFunctionalGenomics]);

    registryComparativeGenomicsDel(
        (RegistryPComparativeGenomics *)
        &pthis->Registry[ensEDatabaseadaptorGroupComparativeGenomics]);

    registryOntologyDel(
        (RegistryPOntology *)
        &pthis->Registry[ensEDatabaseadaptorGroupOntology]);

    registryQualityCheckDel(
        (RegistryPQualityCheck *)
        &pthis->Registry[ensEDatabaseadaptorGroupQualityCheck]);

    AJFREE(pthis);

    *Pentry = NULL;

    return;
}




/* @funcstatic registryEntryTrace *********************************************
**
** Trace a Registry Entry object.
**
** @param [r] entry [const RegistryPEntry] Registry Entry object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryEntryTrace(
    const RegistryPEntry entry,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!entry)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SregistryEntryTrace %p\n"
            "%S  Registry[0] NULL %p\n"
            "%S  Registry[1] Core %p\n"
            "%S  Registry[2] Vega %p\n"
            "%S  Registry[3] Other Features %p\n"
            "%S  Registry[4] Copy DNA %p\n"
            "%S  Registry[5] Genetic Variation %p\n"
            "%S  Registry[6] Functional Genomics %p\n"
            "%S  Registry[7] Comparative Genomics %p\n"
            "%S  Registry[8] Gene Ontology %p\n"
            "%S  Registry[9] Quality Check %p\n",
            indent, entry,
            indent, entry->Registry[0],
            indent, entry->Registry[1],
            indent, entry->Registry[2],
            indent, entry->Registry[3],
            indent, entry->Registry[4],
            indent, entry->Registry[5],
            indent, entry->Registry[6],
            indent, entry->Registry[7],
            indent, entry->Registry[8],
            indent, entry->Registry[9]);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic registryFunctionalGenomicsDel **********************************
**
** Default destructor for a Registry Functional Genomics object.
**
** @param [d] Prfg [RegistryPFunctionalGenomics*] Registry Functional Genomics
**                                                object address
**
** @return [void]
** @@
******************************************************************************/

static void registryFunctionalGenomicsDel(
    RegistryPFunctionalGenomics *Prfg)
{
    RegistryPFunctionalGenomics pthis = NULL;

    if(!Prfg)
        return;

    if(!*Prfg)
        return;

    if(ajDebugTest("registryFunctionalGenomicsDel"))
    {
        ajDebug("registryFunctionalGenomicsDel\n"
                "  *Prfg %p\n",
                *Prfg);

        registryFunctionalGenomicsTrace(*Prfg, 1);
    }

    pthis = *Prfg;

    /* Delete all Ensembl Object Adaptors based on the Database Adaptor. */

    /* Finally, delete the Ensembl Database Adaptor. */

    ensDatabaseadaptorDel(&pthis->Databaseadaptor);

    AJFREE(pthis);

    *Prfg = NULL;

    return;
}




/* @funcstatic registryFunctionalGenomicsTrace ********************************
**
** Trace a Registry Functional Genomics object.
**
** @param [r] rfg [const RegistryPFunctionalGenomics]
** Registry Functional Genomics object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryFunctionalGenomicsTrace(
    const RegistryPFunctionalGenomics rfg,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!rfg)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%registryFunctionalGenomicsTrace %p\n"
            "%S  Databaseadaptor %p\n",
            indent, rfg,
            indent, rfg->Databaseadaptor);

    ensDatabaseadaptorTrace(rfg->Databaseadaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic registryGeneticVariationDel ************************************
**
** Default destructor for a Registry Genetic Variation object.
**
** @param [d] Prgv [RegistryPGeneticVariation*] Registry Genetic Variation
**                                              object address
**
** @return [void]
** @@
******************************************************************************/

static void registryGeneticVariationDel(
    RegistryPGeneticVariation *Prgv)
{
    RegistryPGeneticVariation pthis = NULL;

    if(!Prgv)
        return;

    if(!*Prgv)
        return;

    if(ajDebugTest("registryGeneticVariationDel"))
    {
        ajDebug("registryGeneticVariationDel\n"
                "  *Prgv %p\n",
                *Prgv);

        registryGeneticVariationTrace(*Prgv, 1);
    }

    pthis = *Prgv;

    /* Delete all Ensembl Object Adaptors based on the Database Adaptor. */

    ensGvalleleadaptorDel(
        &pthis->Gvalleleadaptor);

    ensGvattributeadaptorDel(
        &pthis->Gvattributeadaptor);

    ensGvindividualadaptorDel(
        &pthis->Gvindividualadaptor);

    ensGvpopulationadaptorDel(
        &pthis->Gvpopulationadaptor);

    ensGvpopulationgenotypeadaptorDel(
        &pthis->Gvpopulationgenotypeadaptor);

    ensGvsampleadaptorDel(
        &pthis->Gvsampleadaptor);

    ensGvsourceadaptorDel(
        &pthis->Gvsourceadaptor);

    ensGvvariationfeatureadaptorDel(
        &pthis->Gvvariationfeatureadaptor);

    ensGvvariationsetadaptorDel(
        &pthis->Gvvariationsetadaptor);

    /* Finally, delete the Ensembl Database Adaptor. */

    ensGvdatabaseadaptorDel(
        &pthis->Gvdatabaseadaptor);

    ensDatabaseadaptorDel(
        &pthis->Databaseadaptor);

    AJFREE(pthis);

    *Prgv = NULL;

    return;
}




/* @funcstatic registryGeneticVariationTrace **********************************
**
** Trace a Registry Genetic Variation object.
**
** @param [r] rgv [const RegistryPGeneticVariation] Registry Genetic Variation
** object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryGeneticVariationTrace(
    const RegistryPGeneticVariation rgv,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!rgv)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%registryGeneticVariationTrace %p\n"
            "%S  Databaseadaptor %p\n"
            "%S  Gvalleleadaptor %p\n"
            "%S  Gvdatabaseadaptor %p\n"
            "%S  Gvindividualadaptor %p\n"
            "%S  Gvpopulationadaptor %p\n"
            "%S  Gvpopulationgenotypeadaptor %p\n"
            "%S  Gvsampleadaptor %p\n"
            "%S  Gvsourceadaptor %p\n"
            "%S  Gvvariationfeatureadaptor %p\n"
            "%S  Gvvariationsetadaptor %p\n",
            indent, rgv,
            indent, rgv->Databaseadaptor,
            indent, rgv->Gvalleleadaptor,
            indent, rgv->Gvdatabaseadaptor,
            indent, rgv->Gvindividualadaptor,
            indent, rgv->Gvpopulationadaptor,
            indent, rgv->Gvpopulationgenotypeadaptor,
            indent, rgv->Gvsampleadaptor,
            indent, rgv->Gvsourceadaptor,
            indent, rgv->Gvvariationfeatureadaptor,
            indent, rgv->Gvvariationsetadaptor);

    ensDatabaseadaptorTrace(rgv->Databaseadaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic registryIdentifierNew ******************************************
**
** Default constructor for a Registry Identifier object.
**
** @param [u] expression [AjPStr] Regular expression
** @param [u] species [AjPStr] Ensembl Database Adaptor species
** @param [u] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
**
** @return [RegistryPIdentifier] Registry Identifier object or NULL
** @@
******************************************************************************/

static RegistryPIdentifier registryIdentifierNew(
    AjPStr expression,
    AjPStr species,
    EnsEDatabaseadaptorGroup dbag)
{
    RegistryPIdentifier ri = NULL;

    if(!expression)
        return NULL;

    if(!species)
        return NULL;

    if(!dbag)
        return NULL;

    AJNEW0(ri);

    ri->RegularExpression = ajStrNewS(expression);
    ri->SpeciesName       = ajStrNewS(species);
    ri->Group             = dbag;

    return ri;
}




/* @funcstatic registryIdentifierDel ******************************************
**
** Default destructor for a Registry Identifier object.
**
** @param [d] Pri [RegistryPIdentifier*] Registry Identifier object address
**
** @return [void]
** @@
******************************************************************************/

static void registryIdentifierDel(
    RegistryPIdentifier *Pri)
{
    RegistryPIdentifier pthis = NULL;

    if(!Pri)
        return;

    if(!*Pri)
        return;

    pthis = *Pri;

    ajStrDel(&pthis->RegularExpression);
    ajStrDel(&pthis->SpeciesName);

    AJFREE(pthis);

    *Pri = NULL;

    return;
}




/* @funcstatic registryOntologyDel ********************************************
**
** Default destructor for a Registry Ontology object.
**
** @param [d] Pro [RegistryPOntology*] Registry Ontology object address
**
** @return [void]
** @@
******************************************************************************/

static void registryOntologyDel(
    RegistryPOntology *Pro)
{
    RegistryPOntology pthis = NULL;

    if(!Pro)
        return;

    if(!*Pro)
        return;

    if(ajDebugTest("registryOntologyDel"))
    {
        ajDebug("registryOntologyDel\n"
                "  *Pro %p\n",
                *Pro);

        registryOntologyTrace(*Pro, 1);
    }

    pthis = *Pro;

    /* Delete all Ensembl Object Adaptors based on the Database Adaptor. */

    /* Finally, delete the Ensembl Database Adaptor. */

    ensDatabaseadaptorDel(&pthis->Databaseadaptor);

    AJFREE(pthis);

    *Pro = NULL;

    return;
}




/* @funcstatic registryOntologyTrace ******************************************
**
** Trace a Registry Ontology object.
**
** @param [r] ro [const RegistryPOntology] Registry Ontology object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryOntologyTrace(
    const RegistryPOntology ro,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!ro)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%registryOntologyTrace %p\n"
            "%S  Databaseadaptor %p\n",
            indent, ro,
            indent, ro->Databaseadaptor);

    ensDatabaseadaptorTrace(ro->Databaseadaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic registryQualityCheckDel ****************************************
**
** Default destructor for a Registry Quality Check object.
**
** @param [d] Prqc [RegistryPQualityCheck*] Registry Quality Check
**                                          object address
**
** @return [void]
** @@
******************************************************************************/

static void registryQualityCheckDel(
    RegistryPQualityCheck *Prqc)
{
    RegistryPQualityCheck pthis = NULL;

    if(!Prqc)
        return;

    if(!*Prqc)
        return;

    if(ajDebugTest("registryQualityCheckDel"))
    {
        ajDebug("registryQualityCheckDel\n"
                "  *Prqc %p\n",
                *Prqc);

        registryQualityCheckTrace(*Prqc, 1);
    }

    pthis = *Prqc;

    /* Delete all Ensembl Object Adaptors based on the Database Adaptor. */

    ensQcalignmentadaptorDel(
        &pthis->Qcalignmentadaptor);

    ensQcdasfeatureadaptorDel(
        &pthis->Qcdasfeatureadaptor);

    ensQcdatabaseadaptorDel(
        &pthis->Qcdatabaseadaptor);

    ensQcsequenceadaptorDel(
        &pthis->Qcsequenceadaptor);

    ensQcsubmissionadaptorDel(
        &pthis->Qcsubmissionadaptor);

    ensQcvariationadaptorDel(
        &pthis->Qcvariationadaptor);

    /* Finally, delete the Ensembl Database Adaptor. */

    ensDatabaseadaptorDel(
        &pthis->Databaseadaptor);

    AJFREE(pthis);

    *Prqc = NULL;

    return;
}




/* @funcstatic registryQualityCheckTrace **************************************
**
** Trace a Registry Quality Check object.
**
** @param [r] rqc [const RegistryPQualityCheck] Registry Quality Check object
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryQualityCheckTrace(
    const RegistryPQualityCheck rqc,
    ajuint level)
{
    AjPStr indent = NULL;

    if(!rqc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%registryQualityCheckTrace %p\n"
            "%S  Databaseadaptor %p\n"
            "%S  Qcalignmentadaptor %p\n"
            "%S  Qcdasfeatureadaptor %p\n"
            "%S  Qcdatabaseadaptor %p\n"
            "%S  Qcsequenceadaptor %p\n"
            "%S  Qcsubmissionadaptor %p\n"
            "%S  Qcvariationadaptor %p\n",
            indent, rqc,
            indent, rqc->Databaseadaptor,
            indent, rqc->Qcalignmentadaptor,
            indent, rqc->Qcdasfeatureadaptor,
            indent, rqc->Qcdatabaseadaptor,
            indent, rqc->Qcsequenceadaptor,
            indent, rqc->Qcsubmissionadaptor,
            indent, rqc->Qcvariationadaptor);

    ensDatabaseadaptorTrace(rqc->Databaseadaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [none] Ensembl Registry Internals *****************************
**
** Functions to control Ensembl Registry Internals
**
** @cc Bio::EnsEMBL::Registry
** @cc CVS Revision: 1.165
** @cc CVS Tag:
**
******************************************************************************/




/* @section Internals *********************************************************
**
** @fdata [none]
**
** @nam3rule Clear Clear Ensembl Registry internals
** @nam3rule Exit Clear and free Ensembl Registry internals
** @nam3rule Init Initialise Ensembl Registry internals
**
** @valrule * [void]
**
** @fcategory internals
******************************************************************************/




/* @func ensRegistryClear *****************************************************
**
** Clear Ensembl Registry internals.
**
** @return [void]
** @@
******************************************************************************/

void ensRegistryClear(void)
{
    ensRegistryAliasClear();

    ensRegistryEntryClear();

    ensRegistryIdentifierClear();

    return;
}




/* @func ensRegistryExit ******************************************************
**
** Clear and free Ensembl Registry internals.
**
** @return [void]
** @@
******************************************************************************/

void ensRegistryExit(void)
{
    ensRegistryClear();

    /* Free the AJAX Table of Registry Alias objects. */

    if(registryAlias)
        ajTableFree(&registryAlias);

    /* Clear and free the AJAX Table of Registry Entry objects. */

    if(registryEntry)
        ajTableFree(&registryEntry);

    /* Clear and free the AJAX List of Registry Identifier objects. */

    if(registryIdentifier)
        ajListFree(&registryIdentifier);

    /* Clear and free the AJAX List of Registry Source objects. */

    if(registrySource)
        ajListstrFreeData(&registrySource);

    return;
}




/* @func ensRegistryInit ******************************************************
**
** Initialise Ensembl Registry internals.
**
** @return [void]
** @@
******************************************************************************/

void ensRegistryInit(void)
{
    registryAlias = ensTablestrNewLen(0);

    registryEntry = ensTablestrNewLen(0);

    registryIdentifier = ajListNew();

    registrySource = ajListstrNew();

    return;
}




/* @section Load **************************************************************
**
** @fdata [none]
**
** @nam3rule Load Load Ensembl Registry internals
** @nam4rule Databaseconnection Load from an Ensembl Database Connection
** @nam4rule Filename Load from an EMBOSS data file
** @nam4rule Servername Load via an AJAX Server name
**
** @argrule Databaseconnection dbc [EnsPDatabaseconnection]
** Ensembl Database Connection
** @argrule Filename filename [const AjPStr] EMBOSS data file name
** @argrule Servername servername [AjPStr] AJAX Server name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory internals
******************************************************************************/




/* @func ensRegistryLoadDatabaseconnection ************************************
**
** Load Ensembl Registry internals from an Ensembl Database Connection.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: In this implementation the reference adaptor is not set during
** registration of core-sytle, but non-core databases (i.e. cdna,
** otherfeatures, vega databases). The ensRegistryGetReferenceadaptor function
** checks a hierarchy of Database Adaptors to return an appropriate Database
** Adaptor in case none has been explicitly set as reference adaptor.
** Bio::EnsEMBL::Utils::ConfigRegistry::load_core
** Bio::EnsEMBL::Utils::ConfigRegistry::load_adaptors
** Bio::EnsEMBL::Utils::ConfigRegistry::load_and_attach_dnadb_to_core
** TODO: This function could be re-written!
** New ensDatabaseadaptorNewName and ensDatabaseadaptorNewUrl functions have
** been added and use the Ensembl database naming schema to fill in all
** Ensembl Database Adaptor members.
******************************************************************************/

AjBool ensRegistryLoadDatabaseconnection(EnsPDatabaseconnection dbc)
{
    AjBool debug      = AJFALSE;
    AjBool registered = AJFALSE;

    AjPRegexp multire      = NULL;
    AjPRegexp speciesre    = NULL;
    AjPRegexp collectionre = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr dbname    = NULL;
    AjPStr group     = NULL;
    AjPStr prefix    = NULL;
    AjPStr swversion = NULL;
    AjPStr multi     = NULL;
    AjPStr source    = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    EnsPDatabaseadaptor dba = NULL;

    debug = ajDebugTest("ensRegistryLoadDatabaseconnection");

    if(debug)
    {
        ajDebug("ensRegistryLoadDatabaseconnection\n"
                "  dbc %p\n",
                dbc);

        ensDatabaseconnectionTrace(dbc, 1);

        ajDebug("ensRegistryLoadDatabaseconnection "
                "software version '%s'.\n",
                ensSoftwareVersion);
    }

    if(!dbc)
        return ajFalse;

    /* Check if this Ensembl Database Connection has been used before. */

    ensDatabaseconnectionFetchUrl(dbc, &source);

    registrySourceRegister(source, &registered);

    ajStrDel(&source);

    if(registered)
        return ajTrue;

    multi = ajStrNewC("default");

    collectionre =
        ajRegCompC("^\\w+_collection_([a-z]+)(?:_\\d+)?_(\\d+)_\\w+");

    multire =
        ajRegCompC("^ensembl_([a-z]+)(_\\w+?)*?(?:_\\d+)?_(\\d+)$");

    speciesre =
        ajRegCompC("^([a-z]+_[a-z0-9]+)_([a-z]+)(?:_\\d+)?_(\\d+)_\\w+");

    statement = ajStrNewC("SHOW DATABASES");

    sqls = ensDatabaseconnectionSqlstatementNew(dbc, statement);

    if(!sqls)
        ajFatal("ensRegistryLoadDatabaseconnection "
                "SQL statement failed.\n"
                "Please check the SQL server address '%S', "
                "your network connection or that any firewalls "
                "permit outgong TCP/IP connections on port '%S'.\n",
                ensDatabaseconnectionGetHostname(dbc),
                ensDatabaseconnectionGetHostport(dbc));

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        dbname = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &dbname);

        if(ajRegExec(collectionre, dbname))
        {
            /* Ensembl Collection databases have to be matched first. */

            group     = ajStrNew();
            swversion = ajStrNew();

            ajRegSubI(collectionre, 1, &group);
            ajRegSubI(collectionre, 2, &swversion);

            if(ajStrMatchCaseC(swversion, ensSoftwareVersion))
            {
                if(debug)
                    ajDebug("ensRegistryLoadDatabaseconnection '%S' "
                            "collection matched\n",
                            dbname);

                dbag = ensDatabaseadaptorGroupFromStr(group);

                if(dbag)
                    registryEntryLoadCollection(dbc, dbname, dbag);
                else
                    ajDebug("ensRegistryLoadDatabaseconnection got unexpected "
                            "group string '%S' for database name '%S'.\n",
                            group, dbname);
            }
            else
            {
                if(debug)
                    ajDebug("ensRegistryLoadDatabaseconnection '%S' "
                            "collection\n",
                            dbname);
            }

            ajStrDel(&group);
            ajStrDel(&swversion);
        }
        else if(ajRegExec(multire, dbname))
        {
            /* Multi-species databases */

            group     = ajStrNew();
            prefix    = ajStrNew();
            swversion = ajStrNew();

            ajRegSubI(multire, 1, &group);

            if(ajRegLenI(multire, 3))
            {
                ajRegSubI(multire, 2, &prefix);
                ajRegSubI(multire, 3, &swversion);

                /* Remove leading underscores from the species prefix. */

                if(ajStrPrefixC(prefix, "_"))
                    ajStrCutStart(&prefix, 1);
            }
            else
                ajRegSubI(multire, 2, &swversion);

            if(ajStrMatchCaseC(swversion, ensSoftwareVersion))
            {
                if(debug)
                    ajDebug("ensRegistryLoadDatabaseconnection '%S' "
                            "multi-species matched\n",
                            dbname);

                dbag = ensDatabaseadaptorGroupFromStr(group);

                if(dbag)
                    ensRegistryNewDatabaseadaptor(dbc,
                                                  dbname,
                                                  (ajStrGetLen(prefix))
                                                  ? prefix : multi,
                                                  dbag,
                                                  ajFalse,
                                                  0);
                else
                    ajDebug("ensRegistryLoadDatabaseconnection got unexpected "
                            "group string '%S' for database name '%S'.\n",
                            group, dbname);
            }
            else
            {
                if(debug)
                    ajDebug("ensRegistryLoadDatabaseconnection '%S' "
                            "multi-species\n",
                            dbname);
            }

            ajStrDel(&group);
            ajStrDel(&prefix);
            ajStrDel(&swversion);
        }
        else if(ajRegExec(speciesre, dbname))
        {
            /* Species-specific databases */

            prefix    = ajStrNew();
            group     = ajStrNew();
            swversion = ajStrNew();

            ajRegSubI(speciesre, 1, &prefix);
            ajRegSubI(speciesre, 2, &group);
            ajRegSubI(speciesre, 3, &swversion);

            if(ajStrMatchCaseC(swversion, ensSoftwareVersion))
            {
                if(debug)
                    ajDebug("ensRegistryLoadDatabaseconnection '%S' "
                            "species-specific matched\n",
                            dbname);

                dbag = ensDatabaseadaptorGroupFromStr(group);

                if(dbag)
                {
                    dba = ensRegistryNewDatabaseadaptor(dbc,
                                                        dbname,
                                                        prefix,
                                                        dbag,
                                                        ajFalse,
                                                        0);

                    registryAliasLoadDatabaseconnection(dbc, dba);
                }
                else
                    ajDebug("ensRegistryLoadDatabaseconnection got unexpected "
                            "group string '%S' for database name '%S'.\n",
                            group, dbname);
            }
            else
            {
                if(debug)
                    ajDebug("ensRegistryLoadDatabaseconnection '%S' "
                            "species-specific\n",
                            dbname);
            }

            ajStrDel(&prefix);
            ajStrDel(&group);
            ajStrDel(&swversion);
        }
        else
            ajDebug("ensRegistryLoadDatabaseconnection '%S' no match\n",
                    dbname);

        ajStrDel(&dbname);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseconnectionSqlstatementDel(dbc, &sqls);

    ajRegFree(&collectionre);
    ajRegFree(&multire);
    ajRegFree(&speciesre);

    ajStrDel(&statement);
    ajStrDel(&multi);

    if(debug)
    {
        ajDebug("ensRegistryLoadDatabaseconnection\n");

        ensRegistryEntryTrace(1);
    }

    return ajTrue;
}




/* @func ensRegistryLoadServername ********************************************
**
** Load Ensembl Registry internals via an AJAX Server name.
**
** @param [u] servername [AjPStr] AJAX Server name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryLoadServername(AjPStr servername)
{
    AjBool debug      = AJFALSE;
    AjBool registered = AJFALSE;

    AjIList dbniter  = NULL;
    AjIList svriter  = NULL;
    AjPList svrnames = NULL;
    AjPList dbnames  = NULL;

    AjPStr dbname  = NULL;
    AjPStr source  = NULL;
    AjPStr svrname = NULL;
    AjPStr value   = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseconnection dbc = NULL;

    debug = ajDebugTest("ensRegistryLoadServername");

    if(debug)
        ajDebug("ensRegistryLoadServername\n"
                "  servername '%S'\n",
                servername);

    /*
    ** Since the AJAX String objects on the AJAX String List objects for
    ** server and database names are not owned by the caller, the AJAX String
    ** List objects must be re-created and freed for each server.
    */

    value = ajStrNew();

    svrnames = ajListstrNew();

    if((servername != NULL) && (ajStrGetLen(servername) > 0))
        ajListstrPushAppend(svrnames, servername);
    else
        ajNamListListServers(svrnames);

    svriter = ajListIterNewread(svrnames);

    while(!ajListIterDone(svriter))
    {
        svrname = ajListstrIterGet(svriter);

        /* Only AJAX Server definitions of "method" "ensembl" are relevant. */

        ajNamSvrGetAttrC(svrname, "method", &value);

        if(debug)
            ajDebug("ensRegistryLoadServername got "
                    "server name '%S' method '%S'.\n",
                    svrname, value);

        if(ajStrMatchC(value, "ensembl"))
        {
            /* Register the AJAX Server as Registry Source entry via a URL. */

            ajNamSvrGetAttrC(svrname, "cachefile", &value);

            /*
            ** FIXME: It would be good to get the full path for this file.
            ** Does the AJAX Name module keep track of the server file it
            ** actually loaded?
            */
            source = ajFmtStr("file:///%S", value);

            if(debug)
                ajDebug("ensRegistryLoadServername register '%S' from "
                        "source '%S'.\n",
                        svrname, source);

            registrySourceRegister(source, &registered);

            ajStrDel(&source);

            if(registered == ajTrue)
            {
                if(debug)
                    ajDebug("ensRegistryLoadServername '%S' already "
                            "registered.\n",
                            svrname);

                continue;
            }

            /*
            ** Create an Ensembl Database Connection to the AJAX Server for
            ** stream-lining subsequent SQL requests.
            */

            ajNamSvrGetAttrC(svrname, "url", &value);

            dbc = ensDatabaseconnectionNewUrl(value);

            if(!dbc)
            {
                ajDebug("ensRegistryLoadServer could not create an "
                        "Ensembl Database Connection for server name '%S' "
                        "and URL '%S'.\n",
                        svrname, value);

                continue;
            }

            /* Get all database names for this AJAX Server. */

            dbnames = ajListstrNew();

            ajNamSvrListListDatabases(svrname, dbnames);

            dbniter = ajListIterNew(dbnames);

            while(!ajListIterDone(dbniter))
            {
                dbname = ajListstrIterGet(dbniter);

                ajNamSvrGetdbAttrC(svrname, dbname, "url", &value);

                dba = ensDatabaseadaptorNewUrl(value);

                if((ensDatabaseadaptorGetMultispecies(dba) == ajTrue) &&
                   (ensDatabaseadaptorGetIdentifier(dba) == 0))
                {
                    /*
                    ** Expand an Ensembl Database Adaptor representing a
                    ** collection database into species-specific
                    ** Ensembl Database Adaptor objects before registering.
                    ** Delete the Ensembl Database Adaptor for the collection.
                    */

                    registryEntryLoadCollection(
                        dbc,
                        ensDatabaseconnectionGetDatabasename(
                            ensDatabaseadaptorGetDatabaseconnection(dba)),
                        ensDatabaseadaptorGetGroup(dba));

                    ensDatabaseadaptorDel(&dba);
                }
                else
                {
                    /*
                    ** Register Ensembl Database Adaptor objects for
                    ** multi-species and species-specific databases.
                    ** Add the species as alias if registered or delete the
                    ** Ensembl Database Adaptor if not registered,
                    ** successfully.
                    */

                    if(ensRegistryAddDatabaseadaptor(dba))
                        ensRegistryAliasAdd(ensDatabaseadaptorGetSpecies(dba),
                                            ensDatabaseadaptorGetSpecies(dba));
                    else
                        ensDatabaseadaptorDel(&dba);
                }
            }

            ajListIterDel(&dbniter);
            ajListstrFree(&dbnames);

            ensDatabaseconnectionDel(&dbc);
        }
    }

    ajListIterDel(&svriter);
    ajListstrFree(&svrnames);

    ajStrDel(&value);

    if(debug)
    {
        ajDebug("ensRegistryLoadServername\n");

        ensRegistryEntryTrace(1);
        ensRegistryAliasTrace(1);
    }

    return ajTrue;
}




/* @section retrieve **********************************************************
**
** @fdata [none]
**
** @nam3rule Retrieve Retrieve objects from the Ensembl Registry
** @nam4rule All      Retrieve all objects
** @nam5rule Species  Retrieve all species names
**
** @argrule Species species [AjPList]
** AJAX List of AJAX String (species name) objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory internals
******************************************************************************/




/* @funcstatic registryStringCompareCase **************************************
**
** Comparison function to sort AJAX String objects case-insensitively in
** ascending order.
**
** @param [r] P1 [const void*] AJAX String 1
** @param [r] P2 [const void*] AJAX String 2
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int registryStringCompareCase(const void* P1, const void* P2)
{
    const AjPStr string1 = NULL;
    const AjPStr string2 = NULL;

    string1 = *(AjPStr const*) P1;
    string2 = *(AjPStr const*) P2;

#if AJFALSE
    if(ajDebugTest("registryStringCompareCase"))
        ajDebug("registryStringCompareCase\n"
                "  string1 %u\n"
                "  string2 %u\n",
                string1,
                string2);
#endif

    return ajStrCmpCaseS(string1, string2);
}




/* @funcstatic registryStringDelete *******************************************
**
** ajListSortUnique nodedelete function to delete AJAX String objects
** that are redundant.
**
** @param [r] PP1 [void**] AJAX String address 1
** @param [r] cl [void*] Standard. Passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
** @@
******************************************************************************/

static void registryStringDelete(void** PP1, void* cl)
{
    if(!PP1)
        return;

    (void) cl;

    ajStrDel((AjPStr*) PP1);

    *PP1 = NULL;

    return;
}




/* @func ensRegistryRetrieveAllSpecies ****************************************
**
** Retrieve all species names from the Ensembl Registry.
**
** @param [u] species [AjPList] AJAX List of AJAX String (species name) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryRetrieveAllSpecies(AjPList species)
{
    void** keyarray = NULL;

    register ajuint i = 0;

    if(!species)
        return ajFalse;

    ajTableToarrayKeys(registryEntry, &keyarray);

    for(i = 0; keyarray[i]; i++)
        ajListstrPushAppend(species, ajStrNewS((AjPStr) keyarray[i]));

    AJFREE(keyarray);

    ajListSortUnique(species, registryStringCompareCase, registryStringDelete);

    return ajTrue;
}




/* @section Alias *************************************************************
**
** @fdata [none]
**
** @nam3rule Alias Functions for manipulating Registry Alias entries
** @nam4rule Add Add a Registry Alias entry
** @nam4rule Clear Clear all Registry Alias entries
** @nam4rule Fetch Fetch aliases (AJAX String objects) from the
** Ensembl Registry
** @nam5rule Allby Fetch all aliases (AJAX String objects)) by a criterion
** @nam6rule Species Fetch all by a species name
** @nam4rule Load Load Registry Alias entries
** @nam5rule File Load from an EMBOSS data file
** @nam4rule Remove Remove a Registry Alias entry
** @nam4rule Resolve Resolve a Registry Alias entry into an
** Ensembl Database Adaptor species
** @nam4rule Trace Trace Registry Alias entries
**
** @argrule Add species [const AjPStr] Species name
** @argrule Add alias [const AjPStr] Alias name
** @argrule Species species [const AjPStr] Species name
** @argrule Allby aliases [AjPList] AJAX List of AJAX String (alias) objects
** @argrule LoadFile filename [const AjPStr] EMBOSS data file name
** @argrule Remove alias [const AjPStr] Alias name
** @argrule Resolve alias [const AjPStr] Alias name
** @argrule Resolve Pspecies [AjPStr*] Species name address
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory internals
******************************************************************************/




/* @funcstatic registryAliasRegister ******************************************
**
** Check, whether an alias name can be resolved directly or after replacing
** underscore characters into a species name. If not, register the alias
** without underscore characters and set the alias with underscores as alias.
**
** @param [r] alias [const AjPStr] Alias name
**
** @return [AjPStr] Species name or NULL
** @@
******************************************************************************/

static AjPStr registryAliasRegister(const AjPStr alias)
{
    AjPStr species = NULL;
    AjPStr unalias = NULL;

    if(!(alias && ajStrGetLen(alias)))
        return NULL;

    /* Resolve an eventual alias to the species name. */

    ensRegistryAliasResolve(alias, &species);

    if(!species)
    {
        /*
        ** If this alias has not been registered before, test if it has been
        ** registered without underscores.
        */

        unalias = ajStrNewS(alias);

        /*
        ** FIXME: Since EMBOSS DBNAMES cannot cope with spaces species names
        ** must include underscores.
        ajStrExchangeCC(&unalias, "_", " ");
        */

        ensRegistryAliasResolve(unalias, &species);

        if(!species)
        {
            /*
            ** If the alias without underscores has also not been registered
            ** before, register it as species before registering the one
            ** with underscores as alias.
            */

            ensRegistryAliasAdd(unalias, unalias);
            ensRegistryAliasAdd(unalias, alias);

            ensRegistryAliasResolve(alias, &species);
        }

        ajStrDel(&unalias);
    }

    return species;
}




/* @func ensRegistryAliasAdd **************************************************
**
** Add a Registry Alias entry to the Ensembl Registry.
**
** @param [r] species [const AjPStr] Species name
** @param [r] alias [const AjPStr] Alias name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasAdd(const AjPStr species, const AjPStr alias)
{
    AjBool debug = AJFALSE;

    AjPStr current = NULL;

    debug = ajDebugTest("ensRegistryAliasAdd");

    if(debug)
        ajDebug("ensRegistryAliasAdd\n"
                "  species '%S'\n"
                "  alias '%S'\n",
                species,
                alias);

    if(!species)
        return ajFalse;

    if(!alias)
        return ajFalse;

    current = (AjPStr) ajTableFetchmodV(registryAlias, (const void*) alias);

    if(current)
    {
        if(ajStrMatchCaseS(current, species))
        {
            if(debug)
                ajDebug("ensRegistryAliasAdd has already added alias '%S' for "
                        "this species '%S'.\n", alias, current);

            return ajTrue;
        }
        else
        {
            if(debug)
                ajDebug("ensRegistryAliasAdd has already added alias '%S' for "
                        "a different species '%S'.\n", alias, current);

            return ajFalse;
        }
    }

    ajTablePut(registryAlias,
               (void*) ajStrNewS(alias),
               (void*) ajStrNewS(species));

    if(debug)
        ajDebug("ensRegistryAliasAdd added alias '%S' for species '%S'.\n",
                alias, species);

    return ajTrue;
}




/* @funcstatic tableRegistryAliasClear ****************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX String key data and
** AJAX String value data.
**
** @param [u] key [void**] AJAX String address
** @param [u] value [void**] AJAX String address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableRegistryAliasClear(void** key,
                                    void** value,
                                    void* cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    ajStrDel((AjPStr*) key);
    ajStrDel((AjPStr*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensRegistryAliasClear ************************************************
**
** Clear all Registry Alias entries from the Ensembl Registry.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasClear(void)
{
    if(!registryAlias)
        return ajFalse;

    ajTableMapDel(registryAlias, tableRegistryAliasClear, NULL);

    return ajTrue;
}




/* @func ensRegistryAliasFetchAllbySpecies ************************************
**
** Fetch all aliases (AJAX String objects) from the Ensembl Registry by a
** species name.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @param [rN] species [const AjPStr] Species name
** @param [u] aliases [AjPList] AJAX List of AJAX String (alias) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasFetchAllbySpecies(const AjPStr species,
                                         AjPList aliases)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    register ajuint i = 0;

    if(!aliases)
        return ajFalse;

    ajTableToarrayKeysValues(registryAlias, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        /* If a species name has been passed in, skip non-matching ones. */
        if((species != NULL) &&
           (ajStrMatchS(species, (AjPStr) valarray[i]) == ajFalse))
            continue;

        /* Skip identical species and alias names. */
        if(ajStrMatchS((AjPStr) keyarray[i], (AjPStr) valarray[i]) == ajTrue)
            continue;

        ajListstrPushAppend(aliases, ajStrNewS((AjPStr) keyarray[i]));
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @funcstatic registryAliasLoadDatabaseconnection ****************************
**
** Load Registry Alias entries from an Ensembl Database Connection into the
** Ensembl Registry.
**
** @cc Bio::EnsEMBL::Registry::ind_and_add_aliases
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: This function uses the Ensembl Database Connection to avoid
** connecting and disconnecting for each database.
******************************************************************************/

static AjBool registryAliasLoadDatabaseconnection(
    EnsPDatabaseconnection dbc,
    EnsPDatabaseadaptor dba)
{
    char* txtdbname = NULL;

    register ajuint i = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr metavalue = NULL;
    AjPStr dbname    = NULL;
    AjPStr statement = NULL;

    if(!dbc)
        return ajFalse;

    if(!dba)
        return ajFalse;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            /* Database Adaptor groups with a 'meta' table. */

            dbname = ensDatabaseconnectionGetDatabasename(
                ensDatabaseadaptorGetDatabaseconnection(dba));

            ensDatabaseconnectionEscapeC(dbc, &txtdbname, dbname);

            for(i = 0; registryAliasMetaKey[i]; i++)
            {
                statement = ajFmtStr(
                    "SELECT "
                    "%s.meta.meta_value "
                    "FROM "
                    "%s.meta "
                    "WHERE "
                    "%s.meta.meta_key = '%s' "
                    "AND "
                    "%s.meta.species_id = %u",
                    txtdbname,
                    txtdbname,
                    txtdbname, registryAliasMetaKey[i],
                    txtdbname, ensDatabaseadaptorGetIdentifier(dba));

                sqls = ensDatabaseconnectionSqlstatementNew(dbc, statement);

                sqli = ajSqlrowiterNew(sqls);

                while(!ajSqlrowiterDone(sqli))
                {
                    metavalue = ajStrNew();

                    sqlr = ajSqlrowiterGet(sqli);

                    ajSqlcolumnToStr(sqlr, &metavalue);

                    if(ajCharMatchC(registryAliasMetaKey[i],
                                    "species.stable_id_prefix"))
                        ensRegistryAddStableidentifierprefix(dba, metavalue);
                    else
                        ensRegistryAliasAdd(ensDatabaseadaptorGetSpecies(dba),
                                            metavalue);

                    ajStrDel(&metavalue);
                }

                ajSqlrowiterDel(&sqli);

                ensDatabaseconnectionSqlstatementDel(dbc, &sqls);

                ajStrDel(&statement);
            }

            /* Load the Ensembl stable identifier prefix. */

            ajCharDel(&txtdbname);

            break;

        case ensEDatabaseadaptorGroupGeneticVariation:

        case ensEDatabaseadaptorGroupFunctionalGenomics:

        case ensEDatabaseadaptorGroupComparativeGenomics:

        case ensEDatabaseadaptorGroupOntology:

        case ensEDatabaseadaptorGroupQualityCheck:

        case ensEDatabaseadaptorGroupPipeline:

        case ensEDatabaseadaptorGroupHive:

        case ensEDatabaseadaptorGroupCoreExpressionEST:

        case ensEDatabaseadaptorGroupCoreExpressionGNF:

        case ensEDatabaseadaptorGroupAncestral:

        case ensEDatabaseadaptorGroupWebsite:

        case ensEDatabaseadaptorGroupProduction:

            /* Ensembl Database Adaptor groups without a 'meta' table. */

            break;

        default:

            ajWarn("registryLoadAlises got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return ajTrue;
}




/* @func ensRegistryAliasLoadFile *********************************************
**
** Load Registry Alias entries from an EMBOSS data file into the
** Ensembl Registry. If no EMBOSS data file name has been specified,
** the default is "EnsemblAliases.dat".
**
** @param [rN] filename [const AjPStr] EMBOSS data file name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasLoadFile(const AjPStr filename)
{
    const char* txtname = "EnsemblAliases.dat";

    AjBool block = AJFALSE;
    AjBool debug = AJFALSE;

    AjPFile infile = NULL;

    AjPStr line    = NULL;
    AjPStr species = NULL;

    debug = ajDebugTest("ensRegistryAliasLoadFile");

    if(debug)
        ajDebug("ensRegistryAliasLoadFile\n"
                "  filename: '%S'\n",
                filename);

    if(filename && ajStrGetLen(filename))
        infile = ajDatafileNewInNameS(filename);
    else
        infile = ajDatafileNewInNameC(txtname);

    if(!infile)
    {
        ajWarn("ensRegistryAliasLoadFile could not load "
               "Ensembl Registry Aliases data file '%s'.",
               (filename && ajStrGetLen(filename)) ?
               ajStrGetPtr(filename) : txtname);

        return ajFalse;
    }

    line = ajStrNew();

    species = ajStrNew();

    while(ajReadlineTrim(infile, &line))
    {
        /*
        ** Remove excess white space and ignore all lines that start with
        ** a '#' sign. Commented out lines, however, should not break blocks.
        */

        ajStrRemoveWhiteSpaces(&line);

        if(ajStrGetCharFirst(line) == '#')
            continue;

        /*
        ** Remove comments on the same line as an alias, before again removing
        ** excess white space. Finally, remove the quotes, if any, and
        ** register the remaining string as alias.
        */

        ajStrCutComments(&line);

        ajStrQuoteStrip(&line);

        ajStrRemoveWhiteSpaces(&line);

        if(!ajStrGetLen(line))
        {
            /* An empty line indicates the end of a block. */

            block = ajFalse;

            continue;
        }

        if(block)
        {
            /* This is a subsequent line of a block, which is an alias. */

            ensRegistryAliasAdd(species, line);
        }
        else
        {
            /* This is the first line of a block, which is the species. */

            ajStrAssignS(&species, line);

            ensRegistryAliasAdd(species, species);

            block = ajTrue;
        }
    }

    ajStrDel(&species);
    ajStrDel(&line);

    ajFileClose(&infile);

    if(debug)
    {
        ajDebug("ensRegistryAliasLoadFile\n");

        ensRegistryAliasTrace(1);
    }

    return ajTrue;
}




/* @func ensRegistryAliasRemove ***********************************************
**
** Remove a Registry Alias entry from the Ensembl Registry.
**
** @param [r] alias [const AjPStr] Alias name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasRemove(const AjPStr alias)
{
    AjPStr key = NULL;
    AjPStr val = NULL;

    if(!alias)
        return ajFalse;

    val = (AjPStr) ajTableRemoveKey(registryAlias,
                                    (const void*) alias,
                                    (void**) &key);

    ajStrDel(&key);
    ajStrDel(&val);

    return ajTrue;
}




/* @func ensRegistryAliasResolve **********************************************
**
** Resolve an alias name into an Ensembl Database Adaptor species.
**
** @param [r] alias [const AjPStr] Alias name
** @param [wP] Pspecies [AjPStr*] Species name adress or NULL
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasResolve(const AjPStr alias, AjPStr* Pspecies)
{
    if(!alias)
        return ajFalse;

    if(!Pspecies)
        return ajFalse;

    *Pspecies = (AjPStr) ajTableFetchmodV(registryAlias, (const void*) alias);

    return ajTrue;
}




/* @func ensRegistryAliasTrace ************************************************
**
** Trace Ensembl Registry Alias entries.
**
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAliasTrace(ajuint level)
{
    AjPStr indent = NULL;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensRegistryAliasTrace %p\n",
            indent, registryAlias);

    ajTablestrTrace(registryAlias);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section Identifier ********************************************************
**
** @fdata [none]
**
** @nam3rule Identifier Functions for manipulating Registry Identifier entries
** @nam4rule Add Add a Registry Identifier entry
** @nam4rule Clear Clear all Registry Identifier entries
** @nam4rule Load Load Registry Identifier entries
** @nam5rule File Load from an EMBOSS data file
** @nam4rule Remove Remove a Registry Identifier entry
** @nam4rule Resolve Resolve a Registry Identifier to an
** Ensembl Database Adaptor species and group
** @nam4rule Trace Trace Registry Identifier entries
**
** @argrule LoadFile filename [const AjPStr] EMBOSS data file name
** @argrule Resolve identifier [const AjPStr] Ensembl stable identifier
** @argrule Resolve Pspecies [AjPStr*] Ensembl Database Adaptor species
** @argrule Resolve Pdbag [EnsEDatabaseadaptorGroup*]
** Ensembl Database Adaptor Group enumeration address
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory internals
******************************************************************************/




/* @func ensRegistryIdentifierClear *******************************************
**
** Clear all Registry Identifier entries from the Ensembl Registry.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryIdentifierClear(void)
{
    RegistryPIdentifier ri = NULL;

    if(!registryIdentifier)
        return ajFalse;

    while(ajListPop(registryIdentifier, (void**) &ri))
        registryIdentifierDel(&ri);

    return ajTrue;
}




/* @func ensRegistryIdentifierLoadFile ****************************************
**
** Load Registry Identifier entries from an EMBOSS data file into the
** Ensembl Registry. If no EMBOSS data file name has been specified,
** the default is "EnsemblIdentifiers.dat".
**
** @param [rN] filename [const AjPStr] EMBOSS data file name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryIdentifierLoadFile(const AjPStr filename)
{
    const char* txtname = "EnsemblIdentifiers.dat";

    AjBool debug = AJFALSE;

    AjPFile infile = NULL;

    AjPStr line       = NULL;
    AjPStr expression = NULL;
    AjPStr alias      = NULL;
    AjPStr group      = NULL;
    AjPStr space      = NULL;
    AjPStr species    = NULL;

    AjPStrTok token = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    RegistryPIdentifier ri = NULL;

    debug = ajDebugTest("ensRegistryIdentifierLoadFile");

    if(debug)
        ajDebug("ensRegistryIdentifierLoadFile\n");

    if(filename && ajStrGetLen(filename))
        infile = ajDatafileNewInNameS(filename);
    else
        infile = ajDatafileNewInNameC(txtname);

    if(!infile)
    {
        ajWarn("ensRegistryIdentifierLoadFile could not load "
               "Ensembl Registry Identifiers data file '%s'.",
               (filename && ajStrGetLen(filename)) ?
               ajStrGetPtr(filename) : txtname);

        return ajFalse;
    }

    line = ajStrNew();

    expression = ajStrNew();

    alias = ajStrNew();
    group = ajStrNew();
    space = ajStrNew();

    while(ajReadlineTrim(infile, &line))
    {
        if(debug)
            ajDebug("ensRegistryIdentifierLoadFile original line '%S'\n",
                    line);

        if(!ajStrCutComments(&line))
            continue;

        ajStrTokenAssignC(&token, line, "\"");

        if(!ajStrTokenNextFind(&token, &expression))
            ajWarn("ensRegistryIdentifierLoadFile could not parse "
                   "regular expression from line '%S'.\n", line);

        if(!ajStrTokenNextFind(&token, &space))
            ajWarn("ensRegistryIdentifierLoadFile could not parse "
                   "begin of species from line '%S'.\n", line);

        if(!ajStrTokenNextFind(&token, &alias))
            ajWarn("ensRegistryIdentifierLoadFile could not parse "
                   "species from line '%S'.\n", line);

        if(!ajStrTokenNextFind(&token, &space))
            ajWarn("ensRegistryIdentifierLoadFile could not parse "
                   "begin of group from line '%S'.\n", line);

        if(!ajStrTokenNextFind(&token, &group))
            ajWarn("ensRegistryIdentifierLoadFile could not parse "
                   "group from line '%S'.\n", line);

        ajStrTokenDel(&token);

        if(debug)
            ajDebug("ensRegistryIdentifierLoadFile "
                    "regular expression '%S' "
                    "alias '%S' "
                    "group '%S'\n",
                    expression,
                    alias,
                    group);

        ensRegistryAliasResolve(alias, &species);

        if(!species)
        {
            ajWarn("ensRegistryIdentifierLoadFile could not resolve '%S' "
                   "to valid species name.\n", alias);

            continue;
        }

        dbag = ensDatabaseadaptorGroupFromStr(group);

        if(!dbag)
        {
            ajWarn("ensRegistryIdentifierLoadFile could not get group for "
                   "string '%S'.\n", group);

            continue;
        }

        ri = registryIdentifierNew(expression, species , dbag);

        ajListPushAppend(registryIdentifier, (void*) ri);
    }

    ajStrDel(&expression);
    ajStrDel(&alias);
    ajStrDel(&group);
    ajStrDel(&space);
    ajStrDel(&line);

    ajFileClose(&infile);

    return ajTrue;
}




/* @func ensRegistryIdentifierResolve *****************************************
**
** Resolve an Ensembl stable identifier into Ensembl Database Adaptor species
** and group members.
**
** @param [r] identifier [const AjPStr] Ensembl stable identifier
** @param [u] Pspecies [AjPStr*] Ensembl Database Adaptor species
** @param [w] Pdbag [EnsEDatabaseadaptorGroup*]
** Ensembl Database Adaptor Group enumeration address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryIdentifierResolve(const AjPStr identifier,
                                    AjPStr* Pspecies,
                                    EnsEDatabaseadaptorGroup* Pdbag)
{
    AjIList iter = NULL;

    AjPRegexp re = NULL;

    RegistryPIdentifier ri = NULL;

    if(!identifier)
        return ajFalse;

    if(!Pspecies)
        return ajFalse;

    if(!Pdbag)
        return ajFalse;

    ajStrAssignClear(Pspecies);

    *Pdbag = ensEDatabaseadaptorGroupNULL;

    iter = ajListIterNew(registryIdentifier);

    while(!ajListIterDone(iter))
    {
        ri = (RegistryPIdentifier) ajListIterGet(iter);

        re = ajRegComp(ri->RegularExpression);

        if(ajRegExec(re, identifier))
        {
            ajStrAssignS(Pspecies, ri->SpeciesName);

            *Pdbag = ri->Group;

            ajRegFree(&re);

            break;
        }

        ajRegFree(&re);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section Entry *************************************************************
**
** @fdata [none]
**
** @nam3rule Entry Functions for manipulating Registry Entry objects
** @nam4rule Clear Clear Registry Entry objects
** @nam4rule Trace Trace Registry Entry objects
**
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory internals
******************************************************************************/




/* @funcstatic tableRegistryEntryClear ****************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX String key data and
** Registry Entry object value data.
**
** @param [u] key [void**] AJAX String address
** @param [u] value [void**] Registry Entry object address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableRegistryEntryClear(void** key,
                                    void** value,
                                    void* cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    ajStrDel((AjPStr*) key);

    registryEntryDel((RegistryPEntry *) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensRegistryEntryClear ************************************************
**
** Clear all Registry Entry objects from the Ensembl Registry.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryEntryClear(void)
{
    if(!registryEntry)
        return ajFalse;

    ajTableMapDel(registryEntry, tableRegistryEntryClear, NULL);

    return ajTrue;
}




/* @funcstatic registryEntryLoadCollection ************************************
**
** Load Registry Entry and Registry Alias objects from meta table entries and
** instantiate Ensembl Database Adaptor objects for each species in an Ensembl
** Collection database.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] dbname [AjPStr] Database name
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registryEntryLoadCollection(EnsPDatabaseconnection dbc,
                                          AjPStr dbname,
                                          EnsEDatabaseadaptorGroup dbag)
{
    char* txtdbname = NULL;

    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr species   = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!dbc)
        return ajFalse;

    ensDatabaseconnectionEscapeC(dbc, &txtdbname, dbname);

    statement = ajFmtStr(
        "SELECT "
        "%s.meta.species_id, "
        "%s.meta.meta_value "
        "FROM "
        "%s.meta "
        "WHERE "
        "%s.meta.meta_key = 'species.db_name'",
        txtdbname,
        txtdbname,
        txtdbname,
        txtdbname);

    ajCharDel(&txtdbname);

    sqls = ensDatabaseconnectionSqlstatementNew(dbc, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        species = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &species);

        ensRegistryAliasAdd(species, species);

        dba = ensRegistryNewDatabaseadaptor(dbc,
                                            dbname,
                                            species,
                                            dbag,
                                            ajTrue,
                                            identifier);

        registryAliasLoadDatabaseconnection(dbc, dba);

        ajStrDel(&species);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseconnectionSqlstatementDel(dbc, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensRegistryEntryTrace ************************************************
**
** Trace Ensembl Registry Entry objects.
**
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryEntryTrace(ajuint level)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    register ajuint i = 0;

    AjPStr indent = NULL;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensRegistryEntryTrace %p\n",
            indent, registryEntry);

    ajTableToarrayKeysValues(registryEntry, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajDebug("%S  Species '%S'\n", indent, (AjPStr) keyarray[i]);

        registryEntryTrace((RegistryPEntry) valarray[i], level + 2);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section Source ************************************************************
**
** @fdata [none]
**
** @nam3rule Source Functions for manipulating Registry Source entries
** @nam4rule Add Add a Registry Source entry
** @nam4rule Clear Clear all Registry Source entries
** @nam4rule Register Register a Registy Source entry
** @nam4rule Remove Remove a Registry Source entry
** @nam4rule Trace Trace Registry Source entries
**
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory internals
******************************************************************************/




/* @funcstatic registrySourceRegister *****************************************
**
** Check, whether a source has been registered before and if this was not the
** case, automatically register it.
**
** Sources are AJAX String representations of file names or Ensembl Database
** Connection URLs that have been used to initialise the Ensembl Registry.
** Tracking source avoids multiple initialisations from the same source.
**
** @param [r] source [const AjPStr] Ensembl Registry source
** @param [w] Pregistered [AjBool*] Registered boolean
**                                  ajTrue if registered before, ajFalse if not
**
** @return [AjBool] ajTrue opon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool registrySourceRegister(const AjPStr source,
                                     AjBool* Pregistered)
{
    AjIList iterator = NULL;

    AjPStr entry = NULL;

    if(!Pregistered)
        return ajFalse;

    *Pregistered = ajFalse;

    if(!(source && ajStrGetLen(source)))
        return ajFalse;

    iterator = ajListIterNew(registrySource);

    while(!ajListIterDone(iterator))
    {
        entry = ajListstrIterGet(iterator);

        if(ajStrMatchCaseS(entry, source))
        {
            *Pregistered = ajTrue;

            break;
        }
    }

    ajListIterDel(&iterator);

    /* If this source has not been seen before, add it to the AJAX List. */

    if(!*Pregistered)
        ajListstrPushAppend(registrySource, ajStrNewS(source));

    return ajTrue;
}




/* @func ensRegistrySourceTrace ***********************************************
**
** Trace Ensembl Registry Source objects.
**
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistrySourceTrace(ajuint level)
{
    AjIList iterator = NULL;

    AjPStr entry = NULL;
    AjPStr indent = NULL;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%ensRegistrySourceTrace %p\n",
            indent, registrySource);

    iterator = ajListIterNew(registrySource);

    while(!ajListIterDone(iterator))
    {
        entry = ajListstrIterGet(iterator);

        ajDebug("%S  Source '%S'\n", indent, (AjPStr) entry);
    }

    ajListIterDel(&iterator);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPDatabaseadaptor] Ensembl Registry Database Adaptor *******
**
** Functions for manipulating Ensembl Database Adaptor objects via the
** Ensembl Registry.
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Database Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Database Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule New Constructor
** @nam4rule Databaseadaptor Constructor for an Ensembl Database Adaptor
** @nam4rule Referenceadaptor Constructor for an Ensembl Database Adaptor as
** reference sequence adaptor
**
** @argrule Databaseadaptor dbc [EnsPDatabaseconnection]
** Ensembl Database Connection
** @argrule Databaseadaptor database [AjPStr] Database name
** @argrule Databaseadaptor alias [AjPStr] Species name or alias name
** @argrule Databaseadaptor dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule Databaseadaptor multi [AjBool] Multiple species
** @argrule Databaseadaptor identifier [ajuint] Species identifier
** @argrule Referenceadaptor dba [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @argrule Referenceadaptor dbc [EnsPDatabaseconnection]
** Ensembl Database Connection
** @argrule Referenceadaptor database [AjPStr] Database name
** @argrule Referenceadaptor alias [AjPStr] Species name or alias name
** @argrule Referenceadaptor dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule Referenceadaptor multi [AjBool] Multiple species
** @argrule Referenceadaptor identifier [ajuint] Species identifier
**
** @valrule * [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensRegistryNewDatabaseadaptor ****************************************
**
** Create an Ensembl Database Adaptor and add it to the Ensembl Registry.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] database [AjPStr] Database name (optional). If not provided, the
**                              database name in the Database Connection will
**                              be used.
** @param [u] alias [AjPStr] Species name or alias name
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
** This function aims to resolve an eventual alias to a valid species name.
** If the alias has not been registered before, it will strip underscores from
** the alias to permit searches for an Ensembl database name prefix, such as
** "homo_sapiens". If an alias without underscores has also not been registered
** before, it will register the alias without underscores as species and the
** the prefix with underscores as alias.
** Ideally aliases have already been loaded from an Ensembl Database
** Connection or an Ensembl data file (EnsemblAliases.dat) before.
******************************************************************************/

EnsPDatabaseadaptor ensRegistryNewDatabaseadaptor(
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier)
{
    AjPStr species = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(ajDebugTest("ensRegistryNewDatabaseadaptor"))
    {
        ajDebug("ensRegistryNewDatabaseadaptor\n"
                "  dbc %p\n"
                "  database '%S'\n"
                "  alias '%S'\n"
                "  dbag '%s'\n"
                "  multi %B\n"
                "  identifier %u\n",
                dbc,
                database,
                alias,
                ensDatabaseadaptorGroupToChar(dbag),
                multi,
                identifier);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dbc)
        return NULL;

    if(!database)
        return NULL;

    if(!alias)
        return NULL;

    species = registryAliasRegister(alias);

    dba = ensDatabaseadaptorNewIni(dbc,
                                   database,
                                   species,
                                   dbag,
                                   multi,
                                   identifier);

    if(!ensRegistryAddDatabaseadaptor(dba))
    {
        ensDatabaseadaptorDel(&dba);

        dba = NULL;
    }

    return dba;
}




/* @func ensRegistryNewReferenceadaptor ***************************************
**
** Create an Ensembl Database Adaptor and add it to the Ensembl Registry
** as reference sequence adaptor.
**
** This function will only return successfully, if the Ensembl Database
** Adaptor could be created and has not been registered before.
**
** @cc Bio::EnsEMBL::Registry::add_DNAAdaptor
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] database [AjPStr] Database name (optional). If not provided, the
**                              database name in the Database Connection will
**                              be used.
** @param [u] alias [AjPStr] Species name or alias name
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
** This function aims to resolve an eventual alias to a valid species name.
** If the alias has not been registered before, it will strip underscores from
** the alias to permit searches for an Ensembl database name prefix, such as
** "homo_sapiens". If an alias without underscores has also not been registered
** before, it will register the alias without underscores as species and the
** the prefix with underscores as alias.
** Ideally aliases have already been loaded from an Ensembl Database
** Connection or an Ensembl data file (EnsemblAliases.dat) before.
******************************************************************************/

EnsPDatabaseadaptor ensRegistryNewReferenceadaptor(
    EnsPDatabaseadaptor dba,
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr alias,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier)
{
    AjPStr species = NULL;

    EnsPDatabaseadaptor rsa = NULL;

    if(ajDebugTest("ensRegistryNewReferenceadaptor"))
    {
        ajDebug("ensRegistryNewReferenceadaptor\n"
                "  dba %p\n"
                "  dbc %p\n"
                "  database '%S'\n"
                "  alias '%S'\n"
                "  dbag '%s'\n"
                "  multi %B\n"
                "  identifier %u\n",
                dba,
                dbc,
                database,
                alias,
                ensDatabaseadaptorGroupToChar(dbag),
                multi,
                identifier);

        ensDatabaseadaptorTrace(dba, 1);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dba)
        return NULL;

    if(!dbc)
        return NULL;

    if(!database)
        return NULL;

    if(!alias)
        return NULL;

    species = registryAliasRegister(alias);

    rsa = ensDatabaseadaptorNewIni(dbc,
                                   database,
                                   species,
                                   dbag,
                                   multi,
                                   identifier);

    if(!ensRegistryAddReferenceadaptor(dba, rsa))
    {
        ensDatabaseadaptorDel(&rsa);

        rsa = NULL;
    }

    return rsa;
}




/* @section element addition **************************************************
**
** Functions for adding members to the Ensembl Registry.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Add Add one object to the Ensembl Registry
** @nam4rule Databaseadaptor Add an Ensembl Database Adaptor
** @nam4rule Referenceadaptor Add an Ensembl Database Adaptor as reference
** @nam4rule Stableidentifierprefix Add an Ensembl stable identifier prefix
**
** @argrule Databaseadaptor dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Referenceadaptor dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Referenceadaptor rsa [EnsPDatabaseadaptor] Reference adaptor
** @argrule Stableidentifierprefix dba [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @argrule Stableidentifierprefix prefix [const AjPStr]
** Stable identifier prefix
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensRegistryAddDatabaseadaptor ****************************************
**
** Add an Ensembl Database Adaptor to the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAddDatabaseadaptor(EnsPDatabaseadaptor dba)
{
    AjBool result = AJFALSE;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    RegistryPEntry             entry = NULL;
    RegistryPCoreStyle           rcs = NULL;
    RegistryPGeneticVariation    rgv = NULL;
    RegistryPFunctionalGenomics  rfg = NULL;
    RegistryPComparativeGenomics rcg = NULL;
    RegistryPOntology            ro  = NULL;

    if(ajDebugTest("ensRegistryAddDatabaseadaptor"))
    {
        ajDebug("ensRegistryAddDatabaseadaptor\n"
                "  dba %p\n",
                dba);

        ensDatabaseadaptorTrace(dba, 1);
    }

    if(!dba)
        return ajFalse;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
    {
        AJNEW0(entry);

        ajTablePut(registryEntry,
                   (void*) ajStrNewRef(ensDatabaseadaptorGetSpecies(dba)),
                   (void*) entry);
    }

    dbag = ensDatabaseadaptorGetGroup(dba);

    switch(dbag)
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            if(entry->Registry[dbag])
                rcs = (RegistryPCoreStyle) entry->Registry[dbag];
            else
            {
                AJNEW0(rcs);

                entry->Registry[dbag] = (void*) rcs;
            }

            if(rcs->Databaseadaptor)
                ajWarn("ensRegistryAddDatabaseadaptor already has an "
                       "Ensembl Database Adaptor of the same "
                       "species '%S' and group '%s' registered.",
                       ensDatabaseadaptorGetSpecies(dba),
                       ensDatabaseadaptorGroupToChar(dbag));
            else
            {
                rcs->Databaseadaptor = dba;

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupGeneticVariation:

            if(entry->Registry[dbag])
                rgv = (RegistryPGeneticVariation) entry->Registry[dbag];
            else
            {
                AJNEW0(rgv);

                entry->Registry[dbag] = (void*) rgv;
            }

            if(rgv->Databaseadaptor)
                ajWarn("ensRegistryAddDatabaseadaptor already has an "
                       "Ensembl Database Adaptor of the same "
                       "species '%S' and group '%s' registered.",
                       ensDatabaseadaptorGetSpecies(dba),
                       ensDatabaseadaptorGroupToChar(dbag));
            else
            {
                rgv->Databaseadaptor = dba;

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupFunctionalGenomics:

            if(entry->Registry[dbag])
                rfg = (RegistryPFunctionalGenomics) entry->Registry[dbag];
            else
            {
                AJNEW0(rfg);

                entry->Registry[dbag] = (void*) rfg;
            }

            if(rfg->Databaseadaptor)
                ajWarn("ensRegistryAddDatabaseadaptor already has an "
                       "Ensembl Database Adaptor of the same "
                       "species '%S' and group '%s' registered.",
                       ensDatabaseadaptorGetSpecies(dba),
                       ensDatabaseadaptorGroupToChar(dbag));
            else
            {
                rfg->Databaseadaptor = dba;

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupComparativeGenomics:

            if(entry->Registry[dbag])
                rcg = (RegistryPComparativeGenomics) entry->Registry[dbag];
            else
            {
                AJNEW0(rcg);

                entry->Registry[dbag] = (void*) rcg;
            }

            if(rcg->Databaseadaptor)
                ajWarn("ensRegistryAddDatabaseadaptor already has an "
                       "Ensembl Database Adaptor of the same "
                       "species '%S' and group '%s' registered.",
                       ensDatabaseadaptorGetSpecies(dba),
                       ensDatabaseadaptorGroupToChar(dbag));
            else
            {
                rcg->Databaseadaptor = dba;

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupOntology:

            if(entry->Registry[dbag])
                ro = (RegistryPOntology) entry->Registry[dbag];
            else
            {
                AJNEW0(ro);

                entry->Registry[dbag] = (void*) ro;
            }

            if(ro->Databaseadaptor)
                ajWarn("ensRegistryAddDatabaseadaptor already has an "
                       "Ensembl Database Adaptor of the same "
                       "species '%S' and group '%s' registered.",
                       ensDatabaseadaptorGetSpecies(dba),
                       ensDatabaseadaptorGroupToChar(dbag));
            else
            {
                ro->Databaseadaptor = dba;

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupQualityCheck:

        case ensEDatabaseadaptorGroupPipeline:

        case ensEDatabaseadaptorGroupHive:

        case ensEDatabaseadaptorGroupCoreExpressionEST:

        case ensEDatabaseadaptorGroupCoreExpressionGNF:

        case ensEDatabaseadaptorGroupAncestral:

        case ensEDatabaseadaptorGroupWebsite:

        case ensEDatabaseadaptorGroupProduction:

            break;

        default:

            ajWarn("ensRegistryAddDatabaseadaptor got a request for an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n", dbag);
    }

    return result;
}




/* @func ensRegistryAddReferenceadaptor ***************************************
**
** Add an Ensembl Database Adaptor for the reference sequence to the
** Ensembl Registry.
**
** This function will only return successfully, if the Ensembl Database
** Adaptor has been registered before and if its species and group match the
** reference sequence adaptor. Also, the reference sequence adaptor must not
** have been added before.
**
** @cc Bio::EnsEMBL::Registry::add_DNAAdaptor
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] rsa [EnsPDatabaseadaptor] Reference sequence adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAddReferenceadaptor(EnsPDatabaseadaptor dba,
                                      EnsPDatabaseadaptor rsa)
{
    AjBool result = AJFALSE;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle ecs = NULL;

    if(ajDebugTest("ensRegistryAddReferenceAdaptor"))
    {
        ajDebug("ensRegistryAddReferenceAdaptor\n"
                "  dba %p\n"
                "  rsa %p\n",
                dba,
                rsa);

        ensDatabaseadaptorTrace(dba, 1);
        ensDatabaseadaptorTrace(rsa, 1);
    }

    if(!dba)
        return result;

    if(!rsa)
        return result;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return result;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            ecs = entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if((ecs != NULL) && (ecs->Referenceadaptor == NULL))
            {
                ecs->Referenceadaptor = rsa;

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupQualityCheck:

        case ensEDatabaseadaptorGroupPipeline:

        case ensEDatabaseadaptorGroupHive:

        case ensEDatabaseadaptorGroupCoreExpressionEST:

        case ensEDatabaseadaptorGroupCoreExpressionGNF:

        case ensEDatabaseadaptorGroupAncestral:

        case ensEDatabaseadaptorGroupWebsite:

        case ensEDatabaseadaptorGroupProduction:

            break;

        default:

            ajWarn("ensRegistryAddReferenceadaptor got a request for an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return result;
}




/* @func ensRegistryAddStableidentifierprefix *********************************
**
** Add an Ensembl stable identifier prefix for an Ensembl Database Adaptor
** to the Ensembl Registry.
**
** This function will only return successfully, if the Ensembl Database
** Adaptor has been registered before.
**
** @cc Bio::EnsEMBL::Registry::*
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] prefix [const AjPStr] Stable identifier prefix
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryAddStableidentifierprefix(EnsPDatabaseadaptor dba,
                                            const AjPStr prefix)
{
    AjBool result = AJFALSE;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle ecs = NULL;

    if(ajDebugTest("ensRegistryAddStableidentifierprefix"))
    {
        ajDebug("ensRegistryAddStableidentifierprefix\n"
                "  dba %p\n"
                "  prefix '%S'\n",
                dba,
                prefix);

        ensDatabaseadaptorTrace(dba, 1);
    }

    if(!dba)
        return result;

    if(!(prefix && ajStrGetLen(prefix)))
        return result;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return result;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            ecs = entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(ecs != NULL)
            {
                if(ecs->Stableidentifierprefix == NULL)
                    ecs->Stableidentifierprefix = ajStrNewS(prefix);
                else
                    ajStrAssignS(&ecs->Stableidentifierprefix, prefix);

                result = ajTrue;
            }

            break;

        case ensEDatabaseadaptorGroupQualityCheck:

        case ensEDatabaseadaptorGroupPipeline:

        case ensEDatabaseadaptorGroupHive:

        case ensEDatabaseadaptorGroupCoreExpressionEST:

        case ensEDatabaseadaptorGroupCoreExpressionGNF:

        case ensEDatabaseadaptorGroupAncestral:

        case ensEDatabaseadaptorGroupWebsite:

        case ensEDatabaseadaptorGroupProduction:

            break;

        default:

            ajWarn("ensRegistryAddStableidentifierprefix got a request for an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return result;
}




/* @section element removal ***************************************************
**
** Functions for removing members from the Ensembl Registry.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Remove Remove one object from the Ensembl Registry
** @nam4rule Databaseadaptor Remove an Ensembl Database Adaptor
**
** @argrule Databaseadaptor Pdba [EnsPDatabaseadaptor*]
** Ensembl Database Adaptor address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensRegistryRemoveDatabaseadaptor *************************************
**
** Remove an Ensembl Database Adaptor from the Ensembl Registry and delete it.
**
** @param [d] Pdba [EnsPDatabaseadaptor*] Ensembl Database Adaptor address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryRemoveDatabaseadaptor(EnsPDatabaseadaptor* Pdba)
{
    AjBool registered = AJFALSE;

    AjPStr key     = NULL;
    AjPStr truekey = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    RegistryPEntry entry             = NULL;
    RegistryPCoreStyle rcs           = NULL;
    RegistryPGeneticVariation rgv    = NULL;
    RegistryPFunctionalGenomics rfg  = NULL;
    RegistryPComparativeGenomics rcg = NULL;
    RegistryPOntology ro             = NULL;

    if(!Pdba)
        return ajFalse;

    if(!*Pdba)
        return ajFalse;

    key = ajStrNewS(ensDatabaseadaptorGetSpecies(*Pdba));

    entry = (RegistryPEntry) ajTableFetchmodV(registryEntry,
                                              (const void*) key);

    if(!entry)
    {
        ajWarn("ensRegistryRemoveDatabaseadaptor could not get "
               "a Registry Entry for species '%S'.\n",
               ensDatabaseadaptorGetSpecies(*Pdba));

        *Pdba = (EnsPDatabaseadaptor) NULL;

        return ajTrue;
    }

    dbag = ensDatabaseadaptorGetGroup(*Pdba);

    switch(dbag)
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle) entry->Registry[dbag];

            if(rcs)
            {
                if(rcs->Databaseadaptor == *Pdba)
                    registryCoreStyleDel((RegistryPCoreStyle*)
                                         &entry->Registry[dbag]);
            }
            else
                ajWarn("ensRegistryRemoveDatabaseadaptor got "
                       "an Ensembl Database Adaptor, which is not "
                       "registered?\n");

            break;

        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation) entry->Registry[dbag];

            if(rgv)
            {
                if(rgv->Databaseadaptor == *Pdba)
                    registryGeneticVariationDel((RegistryPGeneticVariation *)
                                                &entry->Registry[dbag]);
            }
            else
                ajWarn("ensRegistryRemoveDatabaseadaptor got "
                       "an Ensembl Database Adaptor, which is not "
                       "registered?\n");

            break;

        case ensEDatabaseadaptorGroupFunctionalGenomics:

            rfg = (RegistryPFunctionalGenomics) entry->Registry[dbag];

            if(rfg)
            {
                if(rfg->Databaseadaptor == *Pdba)
                    registryFunctionalGenomicsDel(
                        (RegistryPFunctionalGenomics *)
                        &entry->Registry[dbag]);
            }
            else
                ajWarn("ensRegistryRemoveDatabaseadaptor got "
                       "an Ensembl Database Adaptor, which is not "
                       "registered?\n");

            break;

        case ensEDatabaseadaptorGroupComparativeGenomics:

            rcg = (RegistryPComparativeGenomics) entry->Registry[dbag];

            if(rcg)
            {
                if(rcg->Databaseadaptor == *Pdba)
                    registryComparativeGenomicsDel(
                        (RegistryPComparativeGenomics *)
                        &entry->Registry[dbag]);
            }
            else
                ajWarn("ensRegistryRemoveDatabaseadaptor got "
                       "an Ensembl Database Adaptor, which is not "
                       "registered?\n");

            break;

        case ensEDatabaseadaptorGroupOntology:

            ro = (RegistryPOntology) entry->Registry[dbag];

            if(ro)
            {
                if(ro->Databaseadaptor == *Pdba)
                    registryOntologyDel((RegistryPOntology *)
                                        &entry->Registry[dbag]);
            }
            else
                ajWarn("ensRegistryRemoveDatabaseadaptor got "
                       "an Ensembl Database Adaptor, which is not "
                       "registered?\n");

            break;

        case ensEDatabaseadaptorGroupQualityCheck:

        case ensEDatabaseadaptorGroupPipeline:

        case ensEDatabaseadaptorGroupHive:

        case ensEDatabaseadaptorGroupCoreExpressionEST:

        case ensEDatabaseadaptorGroupCoreExpressionGNF:

        case ensEDatabaseadaptorGroupAncestral:

        case ensEDatabaseadaptorGroupWebsite:

        case ensEDatabaseadaptorGroupProduction:

            break;

        default:

            ajWarn("ensRegistryRemoveDatabaseadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n", dbag);
    }

    /*
    ** Check if this Registry Entry remains completely empty and if so,
    ** remove it from the AJAX Table.
    */

    for(dbag = ensEDatabaseadaptorGroupCore;
        dbag < EnsMDatabaseadaptorGroups;
        dbag++)
        if(entry->Registry[dbag])
            registered = ajTrue;

    if(!registered)
    {
        ajTableRemoveKey(registryEntry,
                         (const void*) key,
                         (void**) &truekey);

        registryEntryDel(&entry);

        ajStrDel(&key);
        ajStrDel(&truekey);
    }

    /* Clear the Ensembl Database Adaptor pointer. */

    *Pdba = (EnsPDatabaseadaptor) NULL;

    return ajTrue;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements from the
** Ensembl Registry.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Get Return Registry member(s)
** @nam4rule All Return all Registry members
** @nam5rule Databaseadaptors Return all Ensembl Database Adaptors
** @nam4rule Databaseadaptor Return the
**             Ensembl Database Adaptor
** @nam4rule Referenceadaptor Return the
**             Ensembl Reference Adaptor
**
** @argrule AllDatabaseadaptors dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule AllDatabaseadaptors alias [const AjPStr]
** Ensembl Database Adaptor alias name or species name
** @argrule AllDatabaseadaptors dbas [AjPList] AJAX List of
** Ensembl Database Adaptor objects
** @argrule Databaseadaptor dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule Databaseadaptor alias [const AjPStr]
** Ensembl Database Adaptor alias name or species name
** @argrule Referenceadaptor dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule Databaseadaptors [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Referenceadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensRegistryGetAllDatabaseadaptors ************************************
**
** Get all Ensembl Database Adaptors from the Ensembl Registry
** and optionally filter them by species or group elements.
**
** The caller is responsible for deleting the AJAX List, but *must not* delete
** the Ensembl Database Adaptor objects.
**
** @param [uN] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
** @param [rN] alias [const AjPStr] Ensembl Database Adaptor alias name or
** species name
** @param [u] dbas [AjPList] AJAX List of Ensembl Database Adaptor objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRegistryGetAllDatabaseadaptors(EnsEDatabaseadaptorGroup dbag,
                                         const AjPStr alias,
                                         AjPList dbas)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    register ajuint i = 0;
    register EnsEDatabaseadaptorGroup j = ensEDatabaseadaptorGroupNULL;

    AjBool debug = AJFALSE;

    AjPStr species = NULL;

    EnsPDatabaseadaptor dba = NULL;

    RegistryPCoreStyle rcs           = NULL;
    RegistryPGeneticVariation rgv    = NULL;
    RegistryPFunctionalGenomics rfg  = NULL;
    RegistryPComparativeGenomics rcg = NULL;
    RegistryPOntology ro             = NULL;

    debug = ajDebugTest("ensRegistryGetAllDatabaseadaptors");

    if(debug)
        ajDebug("ensRegistryGetAllDatabaseadaptors\n"
                "  dbag %d\n"
                "  alias '%S'\n"
                "  dbas %p\n",
                dbag,
                alias,
                dbas);

    if(!dbas)
        return ajFalse;

    ensRegistryAliasResolve(alias, &species);

    ajTableToarrayKeysValues(registryEntry, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        for(j = ensEDatabaseadaptorGroupCore;
            j < EnsMDatabaseadaptorGroups;
            j++)
        {
            if(dbag && (dbag != j))
                continue;

            switch(j)
            {
                case ensEDatabaseadaptorGroupCore:

                case ensEDatabaseadaptorGroupVega:

                case ensEDatabaseadaptorGroupOtherFeatures:

                case ensEDatabaseadaptorGroupCopyDNA:

                    rcs = (RegistryPCoreStyle)
                        (((RegistryPEntry) valarray[i])->Registry[j]);

                    if(rcs && rcs->Databaseadaptor)
                    {
                        dba = rcs->Databaseadaptor;

                        if(alias)
                        {
                            if(ajStrMatchS(species,
                                           ensDatabaseadaptorGetSpecies(dba)))
                                ajListPushAppend(dbas, (void*) dba);
                        }
                        else
                            ajListPushAppend(dbas, (void*) dba);
                    }

                    break;

                case ensEDatabaseadaptorGroupGeneticVariation:

                    rgv = (RegistryPGeneticVariation)
                        (((RegistryPEntry) valarray[i])->Registry[j]);

                    if(rgv && rgv->Databaseadaptor)
                    {
                        dba = rgv->Databaseadaptor;

                        if(alias)
                        {
                            if(ajStrMatchS(species,
                                           ensDatabaseadaptorGetSpecies(dba)))
                                ajListPushAppend(dbas, (void*) dba);
                        }
                        else
                            ajListPushAppend(dbas, (void*) dba);
                    }

                    break;

                case ensEDatabaseadaptorGroupFunctionalGenomics:

                    rfg = (RegistryPFunctionalGenomics)
                        (((RegistryPEntry) valarray[i])->Registry[j]);

                    if(rfg && rfg->Databaseadaptor)
                    {
                        dba = rfg->Databaseadaptor;

                        if(alias)
                        {
                            if(ajStrMatchS(species,
                                           ensDatabaseadaptorGetSpecies(dba)))
                                ajListPushAppend(dbas, (void*) dba);
                        }
                        else
                            ajListPushAppend(dbas, (void*) dba);
                    }

                    break;

                case ensEDatabaseadaptorGroupComparativeGenomics:

                    rcg = (RegistryPComparativeGenomics)
                        (((RegistryPEntry) valarray[i])->Registry[j]);

                    if(rcg && rcg->Databaseadaptor)
                    {
                        dba = rcg->Databaseadaptor;

                        if(alias)
                        {
                            if(ajStrMatchS(species,
                                           ensDatabaseadaptorGetSpecies(dba)))
                                ajListPushAppend(dbas, (void*) dba);
                        }
                        else
                            ajListPushAppend(dbas, (void*) dba);
                    }

                    break;

                case ensEDatabaseadaptorGroupOntology:

                    ro = (RegistryPOntology)
                        (((RegistryPEntry) valarray[i])->Registry[j]);

                    if(ro && ro->Databaseadaptor)
                    {
                        dba = ro->Databaseadaptor;

                        if(alias)
                        {
                            if(ajStrMatchS(species,
                                           ensDatabaseadaptorGetSpecies(dba)))
                                ajListPushAppend(dbas, (void*) dba);
                        }
                        else
                            ajListPushAppend(dbas, (void*) dba);
                    }

                    break;

                case ensEDatabaseadaptorGroupQualityCheck:

                case ensEDatabaseadaptorGroupPipeline:

                case ensEDatabaseadaptorGroupHive:

                case ensEDatabaseadaptorGroupCoreExpressionEST:

                case ensEDatabaseadaptorGroupCoreExpressionGNF:

                case ensEDatabaseadaptorGroupAncestral:

                case ensEDatabaseadaptorGroupWebsite:

                case ensEDatabaseadaptorGroupProduction:

                    break;

                default:

                    ajWarn("ensRegistryGetAllDatabaseadaptors got an "
                           "unexpected group %d.\n", j);
            }
        }
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @func ensRegistryGetDatabaseadaptor ****************************************
**
** Get an Ensembl Database Adaptor from the Ensembl Registry.
**
** @cc Bio::EnsEMBL::Registry::get_DBAdaptor
** @param [u] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
** @param [r] alias [const AjPStr] Scientific species name or alias name
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensRegistryGetDatabaseadaptor(
    EnsEDatabaseadaptorGroup dbag,
    const AjPStr alias)
{
    AjBool debug = AJFALSE;

    AjPStr species = NULL;

    RegistryPEntry entry             = NULL;
    RegistryPCoreStyle rcs           = NULL;
    RegistryPGeneticVariation rgv    = NULL;
    RegistryPFunctionalGenomics rfg  = NULL;
    RegistryPComparativeGenomics rcg = NULL;
    RegistryPOntology ro             = NULL;

    debug = ajDebugTest("ensRegistryGetDatabaseadaptor");

    if(debug)
        ajDebug("ensRegistryGetDatabaseadaptor\n"
                "  dbag %d\n"
                "  alias '%S'\n",
                dbag,
                alias);

    if(!dbag)
        return NULL;

    if(!alias)
        return NULL;

    ensRegistryAliasResolve(alias, &species);

    if(debug)
        ajDebug("ensRegistryGetDatabaseadaptor alias '%S' -> species '%S'\n",
                alias, species);

    entry = (RegistryPEntry) ajTableFetchmodV(registryEntry,
                                              (const void*) species);

    if(!entry)
    {
        ajDebug("ensRegistryGetDatabaseadaptor could not get a "
                "Registry Entry for group '%s' and species '%S'.\n",
                ensDatabaseadaptorGroupToChar(dbag), species);

        return NULL;
    }

    switch(dbag)
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle) entry->Registry[dbag];

            if(rcs)
                return rcs->Databaseadaptor;
            else
            {
                ajDebug("ensRegistryGetDatabaseadaptor could not get an "
                        "Ensembl Database Adaptor for group '%s' and "
                        "species '%S'.\n",
                        ensDatabaseadaptorGroupToChar(dbag), species);

                return NULL;
            }

            break;

        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation) entry->Registry[dbag];

            if(rgv)
                return rgv->Databaseadaptor;
            else
            {
                ajDebug("ensRegistryGetDatabaseadaptor could not get an "
                        "Ensembl Database Adaptor for group '%s' and "
                        "species '%S'.\n",
                        ensDatabaseadaptorGroupToChar(dbag), species);

                return NULL;
            }

            break;

        case ensEDatabaseadaptorGroupFunctionalGenomics:

            rfg = (RegistryPFunctionalGenomics) entry->Registry[dbag];

            if(rfg)
                return rfg->Databaseadaptor;
            else
            {
                ajDebug("ensRegistryGetDatabaseadaptor could not get an "
                        "Ensembl Database Adaptor for group '%s' and "
                        "species '%S'.\n",
                        ensDatabaseadaptorGroupToChar(dbag), species);

                return NULL;
            }

            break;

        case ensEDatabaseadaptorGroupComparativeGenomics:

            rcg = (RegistryPComparativeGenomics) entry->Registry[dbag];

            if(rcg)
                return rcg->Databaseadaptor;
            else
            {
                ajDebug("ensRegistryGetDatabaseadaptor could not get an "
                        "Ensembl Database Adaptor for group '%s' and "
                        "species '%S'.\n",
                        ensDatabaseadaptorGroupToChar(dbag), species);

                return NULL;
            }

            break;

        case ensEDatabaseadaptorGroupOntology:

            ro = (RegistryPOntology) entry->Registry[dbag];

            if(ro)
                return ro->Databaseadaptor;
            else
            {
                ajDebug("ensRegistryGetDatabaseadaptor could not get an "
                        "Ensembl Database Adaptor for group '%s' and "
                        "species '%S'.\n",
                        ensDatabaseadaptorGroupToChar(dbag), species);

                return NULL;
            }

            break;

        case ensEDatabaseadaptorGroupQualityCheck:

        case ensEDatabaseadaptorGroupPipeline:

        case ensEDatabaseadaptorGroupHive:

        case ensEDatabaseadaptorGroupCoreExpressionEST:

        case ensEDatabaseadaptorGroupCoreExpressionGNF:

        case ensEDatabaseadaptorGroupAncestral:

        case ensEDatabaseadaptorGroupWebsite:

        case ensEDatabaseadaptorGroupProduction:

            break;

        default:

            ajWarn("ensRegistryGetDatabaseadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n", dbag);
    }

    return NULL;
}




/* @func ensRegistryGetReferenceadaptor ***************************************
**
** Get an Ensembl Reference Adaptor from the Ensembl Registry.
**
** An Ensembl Reference Adaptor is an Ensembl Database Adaptor that is
** connected to an Ensembl core-style database that serves as the reference
** for a particular species.
** Reference databases have the rather large sequence and assembly tables
** populated, backing the following adaptors. In the set of release databases,
** the core database serve as the reference. Other core-style databases
** (e.g. cdna, otherfeatures, vega) are linked to the reference database by
** setting the Reference Adaptor and thus avoiding redundant storage of large
** data sets.
**
** This function will return the following Database Adaptors ordered by
** precedence.
**   * Reference Adaptor of this group
**   * Reference Adaptor of the core group
**   * Database  Adaptor of the core group
**   * Database  Adaptor of this group
**
** @see ensRegistryGetAssemblyexceptionadaptor
** @see ensRegistryGetAssemblyexceptionfeatureadaptor
** @see ensRegistryGetAssemblymapperadaptor
** @see ensRegistryGetCoordsystemadaptor
** @see ensRegistryGetKaryotypebandadaptor
** @see ensRegistryGetRepeatconsensusadaptor
** @see ensRegistryGetRepeatfeatureadaptor
** @see ensRegistryGetSequenceadaptor
**
** @cc Bio::EnsEMBL::Registry::get_DNAAdaptor
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensRegistryGetReferenceadaptor(EnsPDatabaseadaptor dba)
{
    AjBool debug = AJFALSE;

    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry          = NULL;
    RegistryPCoreStyle rcs        = NULL;
    RegistryPGeneticVariation rgv = NULL;

    debug = ajDebugTest("ensRegistryGetReferenceadaptor");

    if(debug)
    {
        ajDebug("ensRegistryGetReferenceadaptor\n"
                "  dba %p\n",
                dba);

        ensDatabaseadaptorTrace(dba, 1);
    }

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            rsa = (rcs->Referenceadaptor) ?
                rcs->Referenceadaptor :
                rcs->Databaseadaptor;

            break;

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(rcs->Referenceadaptor)
                rsa = rcs->Referenceadaptor;
            else
            {
                /*
                ** If a Reference Adaptor has not been set for this group,
                ** return the Reference Adaptor for the core group.
                */

                rcs = (RegistryPCoreStyle)
                    entry->Registry[ensEDatabaseadaptorGroupCore];

                if(rcs)
                {
                    rsa = ensRegistryGetReferenceadaptor(rcs->Databaseadaptor);

                    if(!rsa)
                    {
                        /*
                        ** If neither Reference nor Database Adaptor for the
                        ** core group has been set, return the Database
                        ** Adaptor for this group and hope for the best.
                        */

                        rcs = (RegistryPCoreStyle)
                            entry->Registry[ensDatabaseadaptorGetGroup(dba)];

                        if(rcs)
                            rsa = rcs->Databaseadaptor;
                    }
                }
            }

            break;

        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensEDatabaseadaptorGroupCore];

            if(rcs)
                rsa = ensRegistryGetReferenceadaptor(rcs->Databaseadaptor);

        default:

            ajWarn("ensRegistryGetReferenceadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    if(debug)
    {
        ajDebug("ensRegistryGetReferenceadaptor dba %p rsa %p\n", dba, rsa);

        ensDatabaseadaptorTrace(rsa, 1);
    }

    return rsa;
}




/* @section Ensembl Core-Style Adaptor retrieval ******************************
**
** Functions for returning Ensembl Core-Style Adaptor objects from the
** Ensembl Registry.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Get Return Registry attribute(s)
** @nam4rule Analysisadaptor
** Return the Ensembl Analysis Adaptor
** @nam4rule Assemblyexceptionadaptor
** Return the Ensembl Assembly Exception Adaptor
** @nam4rule Assemblyexceptionfeatureadaptor
** Return the Ensembl Assembly Exception Feature Adaptor
** @nam4rule Assemblymapperadaptor
** Return the Ensembl Assembly Mapper Adaptor
** @nam4rule Attributeadaptor
** Return the Ensembl Attribute Adaptor
** @nam4rule Attributetypeadaptor
** Return the Ensembl Attribute Type Adaptor
** @nam4rule Coordsystemadaptor
** Return the Ensembl Coordinate System Adaptor
** @nam4rule Databaseentryadaptor
** Return the Ensembl Database Entry Adaptor
** @nam4rule Densityfeatureadaptor
** Return the Ensembl Density Feature Adaptor
** @nam4rule Densitytypeadaptor
** Return the Ensembl Density Type Adaptor
** @nam4rule Ditagadaptor
** Return the Ensembl Ditag Adaptor
** @nam4rule Ditagfeatureadaptor
** Return the Ensembl Ditag Feature Adaptor
** @nam4rule Dnaalignfeatureadaptor
** Return the Ensembl DNA Align Feature Adaptor
** @nam4rule Exonadaptor
** Return the Ensembl Exon Adaptor
** @nam4rule Externaldatabaseadaptor
** Return the Ensembl External Database Adaptor
** @nam4rule Geneadaptor
** Return the Ensembl Gene Adaptor
** @nam4rule Karyotypebandadaptor
** Return the Ensembl Karyotype Band Adaptor
** @nam4rule Markeradaptor
** Return the Ensembl Marker Adaptor
** @nam4rule Markerfeatureadaptor
** Return the Ensembl Marker Feature Adaptor
** @nam4rule Metacoordinateadaptor
** Return the Ensembl Meta-Coordinate Adaptor
** @nam4rule Metainformationadaptor
** Return the Ensembl Meta-Information Adaptor
** @nam4rule Miscellaneousfeatureadaptor
** Return the Ensembl Miscellaneous Feature Adaptor
** @nam4rule Miscellaneoussetadaptor
** Return the Ensembl Miscellaneous Set Adaptor
** @nam4rule Predictionexonadaptor
** Return the Ensembl Prediction Exon Adaptor
** @nam4rule Predictiontranscriptadaptor
** Return the Ensembl Prediction Transcript Adaptor
** @nam4rule Proteinalignfeatureadaptor
** Return the Ensembl Protein Align Feature Adaptor
** @nam4rule Proteinfeatureadaptor
** Return the Ensembl Protein Feature Adaptor
** @nam4rule Repeatconsensusadaptor
** Return the Ensembl Repeat Consensus Adaptor
** @nam4rule Repeatfeatureadaptor
** Return the Ensembl Repeat Feature Adaptor
** @nam4rule Seqregionadaptor
** Return the Ensembl Sequence Region Adaptor
** @nam4rule Seqregionsynonymadaptor
** Return the Ensembl Sequence Region Synonym Adaptor
** @nam4rule Sequenceadaptor
** Return the Ensembl Sequence Adaptor
** @nam4rule Simplefeatureadaptor
** Return the Ensembl Simple Feature Adaptor
** @nam4rule Sliceadaptor
** Return the Ensembl Slice Adaptor
** @nam4rule Stableidentifierprefix
** Return the Ensembl stable identifier prefix
** @nam4rule Transcriptadaptor
** Return the Ensembl Transcript Adaptor
** @nam4rule Translationadaptor
** Return the Ensembl Translation Adaptor
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule Analysisadaptor [EnsPAnalysisadaptor]
** Ensembl Analysis Adaptor or NULL
** @valrule Assemblyexceptionadaptor [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor or NULL
** @valrule Assemblyexceptionfeatureadaptor [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor or NULL
** @valrule Assemblymapperadaptor [EnsPAssemblymapperadaptor]
** Ensembl Assembly Mapper Adaptor or NULL
** @valrule Attributeadaptor [EnsPAttributeadaptor]
** Ensembl Attribute Adaptor or NULL
** @valrule Attributetypeadaptor [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor or NULL
** @valrule Coordsystemadaptor [EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor or NULL
** @valrule Databaseentryadaptor [EnsPDatabaseentryadaptor]
** Ensembl Database Entry Adaptor or NULL
** @valrule Densityfeatureadaptor [EnsPDensityfeatureadaptor]
** Ensembl Density Feature Adaptor or NULL
** @valrule Densitytypeadaptor [EnsPDensitytypeadaptor]
** Ensembl Density Type Adaptor or NULL
** @valrule Ditagadaptor [EnsPDitagadaptor]
** Ensembl Ditag Adaptor or NULL
** @valrule Ditagfeatureadaptor [EnsPDitagfeatureadaptor]
** Ensembl Ditag Feature Adaptor or NULL
** @valrule Dnaalignfeatureadaptor [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
** @valrule Exonadaptor [EnsPExonadaptor]
** Ensembl Exon Adaptor or NULL
** @valrule Externaldatabaseadaptor [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor or NULL
** @valrule Geneadaptor [EnsPGeneadaptor]
** Ensembl Gene Adaptor or NULL
** @valrule Karyotypebandadaptor [EnsPKaryotypebandadaptor]
** Ensembl Karyotype Band Adaptor or NULL
** @valrule Markeradaptor [EnsPMarkeradaptor]
** Ensembl Marker Adaptor or NULL
** @valrule Markerfeatureadaptor [EnsPMarkerfeatureadaptor]
** Ensembl Marker Feature Adaptor or NULL
** @valrule Metacoordinateadaptor [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor or NULL
** @valrule Metainformationadaptor [EnsPMetainformationadaptor]
** Ensembl Meta-Information Adaptor or NULL
** @valrule Miscellaneousfeatureadaptor [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor or NULL
** @valrule Miscellaneoussetadaptor [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor or NULL
** @valrule Predictionexonadaptor [EnsPPredictionexonadaptor]
** Ensembl Prediction Exon Adaptor or NULL
** @valrule Predictiontranscriptadaptor [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor or NULL
** @valrule Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
** @valrule Proteinfeatureadaptor [EnsPProteinfeatureadaptor]
** Ensembl Protein Feature Adaptor or NULL
** @valrule Repeatconsensusadaptor [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor or NULL
** @valrule Repeatfeatureadaptor [EnsPRepeatfeatureadaptor]
** Ensembl Repeat Feature Adaptor or NULL
** @valrule Seqregionadaptor [EnsPSeqregionadaptor]
** Ensembl Sequence Region Adaptor or NULL
** @valrule Seqregionsynonymadaptor [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor or NULL
** @valrule Sequenceadaptor [EnsPSequenceadaptor]
** Ensembl Sequence Adaptor or NULL
** @valrule Simplefeatureadaptor [EnsPSimplefeatureadaptor]
** Ensembl Simple Feature Adaptor or NULL
** @valrule Sliceadaptor [EnsPSliceadaptor]
** Ensembl Slice Adaptor or NULL
** @valrule Stableidentifierprefix [AjPStr]
** Ensembl stable identifier prefix or NULL
** @valrule Transcriptadaptor [EnsPTranscriptadaptor]
** Ensembl Transcript Adaptor or NULL
** @valrule Translationadaptor [EnsPTranslationadaptor]
** Ensembl Translation Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensRegistryGetAnalysisadaptor ****************************************
**
** Get an Ensembl Analysis Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAnalysisadaptor]
** Ensembl Analysis Adaptor or NULL
** @@
******************************************************************************/

EnsPAnalysisadaptor ensRegistryGetAnalysisadaptor(EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Analysisadaptor)
                rcs->Analysisadaptor =
                    ensAnalysisadaptorNew(dba);

            return rcs->Analysisadaptor;

            break;

        default:

            ajWarn("ensRegistryGetAnalysisadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetAssemblyexceptionadaptor *******************************
**
** Get an Ensembl Assembly Exception Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionadaptor ensRegistryGetAssemblyexceptionadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Assemblyexceptionadaptor)
                rcs->Assemblyexceptionadaptor =
                    ensAssemblyexceptionadaptorNew(rsa);

            return rcs->Assemblyexceptionadaptor;

            break;

        default:

            ajWarn("ensRegistryGetAssemblyexceptionadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetAssemblyexceptionfeatureadaptor ************************
**
** Get an Ensembl Assembly Exception Feature Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblyexceptionfeatureadaptor]
** Ensembl Assembly Exception Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionfeatureadaptor ensRegistryGetAssemblyexceptionfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Assemblyexceptionfeatureadaptor)
                rcs->Assemblyexceptionfeatureadaptor =
                    ensAssemblyexceptionfeatureadaptorNew(rsa);

            return rcs->Assemblyexceptionfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetAssemblyexceptionfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetAssemblymapperadaptor **********************************
**
** Get an Ensembl Assembly Mapper Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblymapperadaptor]
** Ensembl Assembly Mapper Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblymapperadaptor ensRegistryGetAssemblymapperadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Assemblymapperadaptor)
                rcs->Assemblymapperadaptor =
                    ensAssemblymapperadaptorNew(rsa);

            return rcs->Assemblymapperadaptor;

            break;

        default:

            ajWarn("ensRegistryGetAssemblymapperadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetAttributeadaptor ***************************************
**
** Get an Ensembl Attribute Adaptor from the Ensembl Registry.
** The Ensembl Attribute Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Core-style database.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAttributeadaptor]
** Ensembl Attribute Adaptor or NULL
** @@
******************************************************************************/

EnsPAttributeadaptor ensRegistryGetAttributeadaptor(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            return dba;

            break;

        default:

            ajWarn("ensRegistryGetAttributeadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetAttributetypeadaptor ***********************************
**
** Get an Ensembl Attribute Type Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor or NULL
** @@
******************************************************************************/

EnsPAttributetypeadaptor ensRegistryGetAttributetypeadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

        case ensEDatabaseadaptorGroupGeneticVariation:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Attributetypeadaptor)
                rcs->Attributetypeadaptor =
                    ensAttributetypeadaptorNew(dba);

            return rcs->Attributetypeadaptor;

            break;

        default:

            ajWarn("ensRegistryGetAttributetypeadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetCoordsystemadaptor *************************************
**
** Get an Ensembl Coordinate System Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor or NULL
** @@
******************************************************************************/

EnsPCoordsystemadaptor ensRegistryGetCoordsystemadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Coordsystemadaptor)
                rcs->Coordsystemadaptor =
                    ensCoordsystemadaptorNew(rsa);

            return rcs->Coordsystemadaptor;

            break;

        default:

            ajWarn("ensRegistryGetCoordsystemadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetDatabaseentryadaptor ***********************************
**
** Get an Ensembl Database Entry Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDatabaseentryadaptor]
** Ensembl Database Entry Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseentryadaptor ensRegistryGetDatabaseentryadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Databaseentryadaptor)
                rcs->Databaseentryadaptor =
                    ensDatabaseentryadaptorNew(dba);

            return rcs->Databaseentryadaptor;

            break;

        default:

            ajWarn("ensRegistryGetDatabaseentryadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetDensityfeatureadaptor **********************************
**
** Get an Ensembl Density Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDensityfeatureadaptor]
** Ensembl Density Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDensityfeatureadaptor ensRegistryGetDensityfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Densityfeatureadaptor)
                rcs->Densityfeatureadaptor =
                    ensDensityfeatureadaptorNew(dba);

            return rcs->Densityfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetDensityfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetDensitytypeadaptor *************************************
**
** Get an Ensembl Density Type Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDensitytypeadaptor]
** Ensembl Density Type Adaptor or NULL
** @@
******************************************************************************/

EnsPDensitytypeadaptor ensRegistryGetDensitytypeadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Densitytypeadaptor)
                rcs->Densitytypeadaptor =
                    ensDensitytypeadaptorNew(dba);

            return rcs->Densitytypeadaptor;

            break;

        default:

            ajWarn("ensRegistryGetDensitytypeadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetDitagadaptor *******************************************
**
** Get an Ensembl Ditag Adaptor from the Ensembl Registry.
** The Ensembl Ditag Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Core-style database.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDitagadaptor]
** Ensembl Ditag Adaptor or NULL
** @@
******************************************************************************/

EnsPDitagadaptor ensRegistryGetDitagadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Ditagadaptor)
                rcs->Ditagadaptor =
                    ensDitagadaptorNew(dba);

            return rcs->Ditagadaptor;

            break;

        default:

            ajWarn("ensRegistryGetDitagadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetDitagfeatureadaptor ************************************
**
** Get an Ensembl Ditag Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDitagfeatureadaptor]
** Ensembl Ditag Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensRegistryGetDitagfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Ditagfeatureadaptor)
                rcs->Ditagfeatureadaptor =
                    ensDitagfeatureadaptorNew(dba);

            return rcs->Ditagfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetDitagfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetDnaalignfeatureadaptor *********************************
**
** Get an Ensembl DNA Align Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDnaalignfeatureadaptor ensRegistryGetDnaalignfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Dnaalignfeatureadaptor)
                rcs->Dnaalignfeatureadaptor =
                    ensDnaalignfeatureadaptorNew(dba);

            return rcs->Dnaalignfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetDnaalignfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetExonadaptor ********************************************
**
** Get an Ensembl Exon Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPExonadaptor]
** Ensembl Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPExonadaptor ensRegistryGetExonadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Exonadaptor)
                rcs->Exonadaptor =
                    ensExonadaptorNew(dba);

            return rcs->Exonadaptor;

            break;

        default:

            ajWarn("ensRegistryGetExonadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetExternaldatabaseadaptor ********************************
**
** Get an Ensembl External Database Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor or NULL
** @@
******************************************************************************/

EnsPExternaldatabaseadaptor ensRegistryGetExternaldatabaseadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Externaldatabaseadaptor)
                rcs->Externaldatabaseadaptor =
                    ensExternaldatabaseadaptorNew(dba);

            return rcs->Externaldatabaseadaptor;

            break;

        default:

            ajWarn("ensRegistryGetExternaldatabaseadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGeneadaptor ********************************************
**
** Get an Ensembl Gene Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGeneadaptor]
** Ensembl Gene Adaptor or NULL
** @@
******************************************************************************/

EnsPGeneadaptor ensRegistryGetGeneadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Geneadaptor)
                rcs->Geneadaptor =
                    ensGeneadaptorNew(dba);

            return rcs->Geneadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGeneadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetKaryotypebandadaptor ***********************************
**
** Get an Ensembl Karyotype Band Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPKaryotypebandadaptor]
** Ensembl Karyotype Band Adaptor or NULL
** @@
******************************************************************************/

EnsPKaryotypebandadaptor ensRegistryGetKaryotypebandadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Karyotypebandadaptor)
                rcs->Karyotypebandadaptor =
                    ensKaryotypebandadaptorNew(rsa);

            return rcs->Karyotypebandadaptor;

            break;

        default:

            ajWarn("ensRegistryGetKaryotypebandadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetMarkeradaptor ******************************************
**
** Get an Ensembl Marker Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMarkeradaptor]
** Ensembl Marker Adaptor or NULL
** @@
******************************************************************************/

EnsPMarkeradaptor ensRegistryGetMarkeradaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Markeradaptor)
                rcs->Markeradaptor =
                    ensMarkeradaptorNew(dba);

            return rcs->Markeradaptor;

            break;

        default:

            ajWarn("ensRegistryGetMarkeradaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetMarkerfeatureadaptor ***********************************
**
** Get an Ensembl Marker Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMarkerfeatureadaptor]
** Ensembl Marker Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPMarkerfeatureadaptor ensRegistryGetMarkerfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Markerfeatureadaptor)
                rcs->Markerfeatureadaptor =
                    ensMarkerfeatureadaptorNew(dba);

            return rcs->Markerfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetMarkerfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetMetacoordinateadaptor **********************************
**
** Get an Ensembl Meta-Coordinate Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor or NULL
** @@
******************************************************************************/

EnsPMetacoordinateadaptor ensRegistryGetMetacoordinateadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Metacoordinateadaptor)
                rcs->Metacoordinateadaptor =
                    ensMetacoordinateadaptorNew(dba);

            return rcs->Metacoordinateadaptor;

            break;

        default:

            ajWarn("ensRegistryGetMetacoordinateadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetMetainformationadaptor *********************************
**
** Get an Ensembl Meta-Information Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMetainformationadaptor]
** Ensembl Meta-Information Adaptor or NULL
** @@
******************************************************************************/

EnsPMetainformationadaptor ensRegistryGetMetainformationadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Metainformationadaptor)
                rcs->Metainformationadaptor =
                    ensMetainformationadaptorNew(dba);

            return rcs->Metainformationadaptor;

            break;

        default:

            ajWarn("ensRegistryGetMetainformationadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetMiscellaneousfeatureadaptor ****************************
**
** Get an Ensembl Miscellaneous Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensRegistryGetMiscellaneousfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Miscellaneousfeatureadaptor)
                rcs->Miscellaneousfeatureadaptor =
                    ensMiscellaneousfeatureadaptorNew(dba);

            return rcs->Miscellaneousfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetMiscellaneousfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetMiscellaneoussetadaptor ********************************
**
** Get an Ensembl Miscellaneous Set Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor or NULL
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensRegistryGetMiscellaneoussetadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Miscellaneoussetadaptor)
                rcs->Miscellaneoussetadaptor =
                    ensMiscellaneoussetadaptorNew(dba);

            return rcs->Miscellaneoussetadaptor;

            break;

        default:

            ajWarn("ensRegistryGetMiscellaneoussetadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetPredictionexonadaptor **********************************
**
** Get an Ensembl Prediction Exon Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPPredictionexonadaptor]
** Ensembl Prediction Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictionexonadaptor ensRegistryGetPredictionexonadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Predictionexonadaptor)
                rcs->Predictionexonadaptor =
                    ensPredictionexonadaptorNew(dba);

            return rcs->Predictionexonadaptor;

            break;

        default:

            ajWarn("ensRegistryGetPredictionexonadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetPredictiontranscriptadaptor ****************************
**
** Get an Ensembl Prediction Transcript Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictiontranscriptadaptor ensRegistryGetPredictiontranscriptadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Predictiontranscriptadaptor)
                rcs->Predictiontranscriptadaptor =
                    ensPredictiontranscriptadaptorNew(dba);

            return rcs->Predictiontranscriptadaptor;

            break;

        default:

            ajWarn("ensRegistryGetPredictiontranscriptadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetProteinalignfeatureadaptor *****************************
**
** Get an Ensembl Protein Align Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensRegistryGetProteinalignfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Proteinalignfeatureadaptor)
                rcs->Proteinalignfeatureadaptor =
                    ensProteinalignfeatureadaptorNew(dba);

            return rcs->Proteinalignfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetProteinalignfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetProteinfeatureadaptor **********************************
**
** Get an Ensembl Protein Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPProteinfeatureadaptor]
** Ensembl Protein Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPProteinfeatureadaptor ensRegistryGetProteinfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Proteinfeatureadaptor)
                rcs->Proteinfeatureadaptor =
                    ensProteinfeatureadaptorNew(dba);

            return rcs->Proteinfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetProteinfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetRepeatconsensusadaptor *********************************
**
** Get an Ensembl Repeat Consensus Adaptor from the Ensembl Registry.
** The Ensembl Repeat Consensus Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Core-style database.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor or NULL
** @@
******************************************************************************/

EnsPRepeatconsensusadaptor ensRegistryGetRepeatconsensusadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Repeatconsensusadaptor)
                rcs->Repeatconsensusadaptor =
                    ensRepeatconsensusadaptorNew(rsa);

            return rcs->Repeatconsensusadaptor;

            break;

        default:

            ajWarn("ensRegistryGetRepeatconsensusadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetRepeatfeatureadaptor ***********************************
**
** Get an Ensembl Repeat Feature Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPRepeatfeatureadaptor]
** Ensembl Repeat Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPRepeatfeatureadaptor ensRegistryGetRepeatfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Repeatfeatureadaptor)
                rcs->Repeatfeatureadaptor =
                    ensRepeatfeatureadaptorNew(rsa);

            return rcs->Repeatfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetRepeatfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetSeqregionadaptor ***************************************
**
** Get an Ensembl Sequence Region Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSeqregionadaptor]
** Ensembl Sequence Region Adaptor or NULL
** @@
******************************************************************************/

EnsPSeqregionadaptor ensRegistryGetSeqregionadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Seqregionadaptor)
                rcs->Seqregionadaptor =
                    ensSeqregionadaptorNew(dba);

            return rcs->Seqregionadaptor;

            break;

        default:

            ajWarn("ensRegistryGetSeqregionadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetSeqregionsynonymadaptor ********************************
**
** Get an Ensembl Sequence Region Synonym Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor or NULL
** @@
******************************************************************************/

EnsPSeqregionsynonymadaptor ensRegistryGetSeqregionsynonymadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Seqregionsynonymadaptor)
                rcs->Seqregionsynonymadaptor =
                    ensSeqregionsynonymadaptorNew(dba);

            return rcs->Seqregionsynonymadaptor;

            break;

        default:

            ajWarn("ensRegistryGetSeqregionsynonymadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetSequenceadaptor ****************************************
**
** Get an Ensembl Sequence Adaptor from the Ensembl Registry.
**
** If an Ensembl Reference Adaptor has been set for this database, the adaptor
** returned will be based on the Ensembl Reference Adaptor rather than the
** Ensembl Database Adaptor that was passed in.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSequenceadaptor]
** Ensembl Sequence Adaptor or NULL
** @@
******************************************************************************/

EnsPSequenceadaptor ensRegistryGetSequenceadaptor(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseadaptor rsa = NULL;

    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    rsa = ensRegistryGetReferenceadaptor(dba);

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(rsa));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(rsa))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(rsa)];

            if(!rcs)
                break;

            if(!rcs->Sequenceadaptor)
                rcs->Sequenceadaptor =
                    ensSequenceadaptorNew(rsa);

            return rcs->Sequenceadaptor;

            break;

        default:

            ajWarn("ensRegistryGetSequenceadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(rsa));
    }

    return NULL;
}




/* @func ensRegistryGetSimplefeatureadaptor ***********************************
**
** Get an Ensembl Simple Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSimplefeatureadaptor]
** Ensembl Simple Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensRegistryGetSimplefeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Simplefeatureadaptor)
                rcs->Simplefeatureadaptor =
                    ensSimplefeatureadaptorNew(dba);

            return rcs->Simplefeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetSimplefeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetSliceadaptor *******************************************
**
** Get an Ensembl Slice Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSliceadaptor]
** Ensembl Slice Adaptor or NULL
** @@
******************************************************************************/

EnsPSliceadaptor ensRegistryGetSliceadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Sliceadaptor)
                rcs->Sliceadaptor =
                    ensSliceadaptorNew(dba);

            return rcs->Sliceadaptor;

            break;

        default:

            ajWarn("ensRegistryGetSliceadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetStableidentifierprefix *********************************
**
** Get an Ensembl stable identifier prefix from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjPStr]
** Ensembl stable identifier prefix or NULL
** @@
******************************************************************************/

AjPStr ensRegistryGetStableidentifierprefix(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            return rcs->Stableidentifierprefix;

            break;

        default:

            ajWarn("ensRegistryGetStableidentifierprefix got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetTranscriptadaptor **************************************
**
** Get an Ensembl Transcript Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPTranscriptadaptor]
** Ensembl Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPTranscriptadaptor ensRegistryGetTranscriptadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Transcriptadaptor)
                rcs->Transcriptadaptor =
                    ensTranscriptadaptorNew(dba);

            return rcs->Transcriptadaptor;

            break;

        default:

            ajWarn("ensRegistryGetTranscriptadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetTranslationadaptor *************************************
**
** Get an Ensembl Translation Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPTranslationadaptor]
** Ensembl Translation Adaptor or NULL
** @@
******************************************************************************/

EnsPTranslationadaptor ensRegistryGetTranslationadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry   = NULL;
    RegistryPCoreStyle rcs = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupCore:

        case ensEDatabaseadaptorGroupVega:

        case ensEDatabaseadaptorGroupOtherFeatures:

        case ensEDatabaseadaptorGroupCopyDNA:

            rcs = (RegistryPCoreStyle)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rcs)
                break;

            if(!rcs->Translationadaptor)
                rcs->Translationadaptor =
                    ensTranslationadaptorNew(dba);

            return rcs->Translationadaptor;

            break;

        default:

            ajWarn("ensRegistryGetTranslationadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @section Ensembl Quality Check Adaptor retrieval ***************************
**
** Functions for returning Ensembl Quality Check Adaptor objects from the
** Ensembl Registry.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Get Return Registry attribute(s)
** @nam4rule Qcdatabaseadaptor
** Return the Ensembl Quality Check Database Adaptor
** @nam4rule Qcsequenceadaptor
** Return the Ensembl Quality Check Sequence Adaptor
** @nam4rule Qcalignmentadaptor
** Return the Ensembl Quality Check Alignment Adaptor
** @nam4rule Qcdasfeatureadaptor
** Return the Ensembl Quality Check DAS Feature Adaptor
** @nam4rule Qcsubmissionadaptor
** Return the Ensembl Quality Check Submission Adaptor
** @nam4rule Qcvariationadaptor
** Return the Ensembl Quality Check Variation Adaptor
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule Qcdatabaseadaptor [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor or NULL
** @valrule Qcsequenceadaptor [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor or NULL
** @valrule Qcalignmentadaptor [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor or NULL
** @valrule Qcdasfeatureadaptor [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor or NULL
** @valrule Qcsubmissionadaptor [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor or NULL
** @valrule Qcvariationadaptor [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensRegistryGetQcalignmentadaptor *************************************
**
** Get an Ensembl Quality Check Alignment Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor or NULL
** @@
******************************************************************************/

EnsPQcalignmentadaptor ensRegistryGetQcalignmentadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry      = NULL;
    RegistryPQualityCheck rqc = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupQualityCheck:

            rqc = (RegistryPQualityCheck)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rqc)
                break;

            if(!rqc->Qcalignmentadaptor)
                rqc->Qcalignmentadaptor =
                    ensQcalignmentadaptorNew(dba);

            return rqc->Qcalignmentadaptor;

            break;

        default:

            ajWarn("ensRegistryGetQcalignmentadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetQcdasfeatureadaptor ************************************
**
** Get an Ensembl Quality Check DAS Feature Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcdasfeatureadaptor]
** Ensembl Quality Check DAS Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPQcdasfeatureadaptor ensRegistryGetQcdasfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry      = NULL;
    RegistryPQualityCheck rqc = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupQualityCheck:

            rqc = (RegistryPQualityCheck)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rqc)
                break;

            if(!rqc->Qcdasfeatureadaptor)
                rqc->Qcdasfeatureadaptor =
                    ensQcdasfeatureadaptorNew(dba);

            return rqc->Qcdasfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetQcdasfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetQcdatabaseadaptor **************************************
**
** Get an Ensembl Quality Check Database Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor or NULL
** @@
******************************************************************************/

EnsPQcdatabaseadaptor ensRegistryGetQcdatabaseadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry      = NULL;
    RegistryPQualityCheck rqc = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupQualityCheck:

            rqc = (RegistryPQualityCheck)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rqc)
                break;

            if(!rqc->Databaseadaptor)
                rqc->Qcdatabaseadaptor =
                    ensQcdatabaseadaptorNew(dba);

            return rqc->Qcdatabaseadaptor;

            break;

        default:

            ajWarn("ensRegistryGetQcdatabaseadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetQcsequenceadaptor **************************************
**
** Get an Ensembl Quality Check Sequence Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor or NULL
** @@
******************************************************************************/

EnsPQcsequenceadaptor ensRegistryGetQcsequenceadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry      = NULL;
    RegistryPQualityCheck rqc = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupQualityCheck:

            rqc = (RegistryPQualityCheck)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rqc)
                break;

            if(!rqc->Qcsequenceadaptor)
                rqc->Qcsequenceadaptor =
                    ensQcsequenceadaptorNew(dba);

            return rqc->Qcsequenceadaptor;

            break;

        default:

            ajWarn("ensRegistryGetQcsequenceadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetQcsubmissionadaptor ************************************
**
** Get an Ensembl Quality Check Submission Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor or NULL
** @@
******************************************************************************/

EnsPQcsubmissionadaptor ensRegistryGetQcsubmissionadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry      = NULL;
    RegistryPQualityCheck rqc = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupQualityCheck:

            rqc = (RegistryPQualityCheck)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rqc)
                break;

            if(!rqc->Qcsubmissionadaptor)
                rqc->Qcsubmissionadaptor =
                    ensQcsubmissionadaptorNew(dba);

            return rqc->Qcsubmissionadaptor;

            break;

        default:

            ajWarn("ensRegistryGetQcsubmissionadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetQcvariationadaptor *************************************
**
** Get an Ensembl Quality Check Variation Adaptor from the Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor or NULL
** @@
******************************************************************************/

EnsPQcvariationadaptor ensRegistryGetQcvariationadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry      = NULL;
    RegistryPQualityCheck rqc = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupQualityCheck:

            rqc = (RegistryPQualityCheck)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rqc)
                break;

            if(!rqc->Qcvariationadaptor)
                rqc->Qcvariationadaptor =
                    ensQcvariationadaptorNew(dba);

            return rqc->Qcvariationadaptor;

            break;

        default:

            ajWarn("ensRegistryGetQcvariationadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @section Ensembl Genetic Variation Adaptor retrieval ***********************
**
** Functions for returning Ensembl Genetic Variation Adaptor objects from the
** Ensembl Registry.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Get Return Registry attribute(s)
** @nam4rule Gvalleleadaptor
** Return the Ensembl Genetic Variation Allele Adaptor
** @nam4rule Gvattributeadaptor
** Return the Ensembl Genetic Variation Attribute Adaptor
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
** @nam4rule Gvgenotypeadaptor
** Return the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule Gvindividualadaptor
** Return the Ensembl Genetic Variation Individual Adaptor
** @nam4rule Gvpopulationadaptor
** Return the Ensembl Genetic Variation Population Adaptor
** @nam4rule Gvpopulationgenotypeadaptor
** Return the Ensembl Genetic Variation Population Genotype Adaptor
** @nam4rule Gvsampleadaptor
** Return the Ensembl Genetic Variation Sample Adaptor
** @nam4rule Gvsourceadaptor
** Return the Ensembl Genetic Variation Source Adaptor
** @nam4rule Gvsynonymadaptor
** Return the Ensembl Genetic Variation Synonym Adaptor
** @nam4rule Gvtranscriptvariationadaptor
** Return the Ensembl Genetic Variation Transcript Variation Adaptor
** @nam4rule Gvvariationadaptor
** Return the Ensembl Genetic Variation Variation Adaptor
** @nam4rule Gvvariationfeatureadaptor
** Return the Ensembl Genetic Variation Variation Feature Adaptor
** @nam4rule Gvvariationsetadaptor
** Return the Ensembl Genetic Variation Variation Set Adaptor
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule Gvalleleadaptor [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor or NULL
** @valrule Gvattributeadaptor [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor or NULL
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
** @valrule Gvgenotypeadaptor [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor or NULL
** @valrule Gvindividualadaptor [EnsPGvindividualadaptor]
** Ensembl Genetic Variation Individual Adaptor or NULL
** @valrule Gvpopulationadaptor [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor or NULL
** @valrule Gvpopulationgenotypeadaptor [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor or NULL
** @valrule Gvsampleadaptor [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor or NULL
** @valrule Gvsourceadaptor [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor or NULL
** @valrule Gvsynonymadaptor [EnsPGvsynonymadaptor]
** Ensembl Genetic Variation Synonym Adaptor or NULL
** @valrule Gvtranscriptvariationadaptor [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor or NULL
** @valrule Gvvariationadaptor [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor or NULL
** @valrule Gvvariationfeatureadaptor [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor or NULL
** @valrule Gvvariationsetadaptor [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensRegistryGetGvalleleadaptor ****************************************
**
** Get an Ensembl Genetic Variation Allele Adaptor from the
** Ensembl Registry.
** The Ensembl Genetic Variation Allele Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Variation database.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor or NULL
** @@
******************************************************************************/

EnsPGvalleleadaptor ensRegistryGetGvalleleadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvalleleadaptor)
                rgv->Gvalleleadaptor =
                    ensGvalleleadaptorNew(dba);

            return rgv->Gvalleleadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvalleleadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvattributeadaptor *************************************
**
** Get an Ensembl Genetic Variation Attribute Adaptor from the
** Ensembl Registry.
** The Ensembl Genetic Variation Attribute Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Variation database.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor or NULL
** @@
******************************************************************************/

EnsPGvattributeadaptor ensRegistryGetGvattributeadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvattributeadaptor)
                rgv->Gvattributeadaptor =
                    ensGvattributeadaptorNew(dba);

            return rgv->Gvattributeadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvattributeadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvdatabaseadaptor **************************************
**
** Get an Ensembl Genetic Variation Database Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensRegistryGetGvdatabaseadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvdatabaseadaptor)
                rgv->Gvdatabaseadaptor =
                    ensGvdatabaseadaptorNewIni(dba);

            return rgv->Gvdatabaseadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvdatabaseadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvgenotypeadaptor **************************************
**
** Get an Ensembl Genetic Variation Genotype Adaptor from the
** Ensembl Registry.
** The Ensembl Genetic Variation Genotype Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Variation database.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor or NULL
** @@
******************************************************************************/

EnsPGvgenotypeadaptor ensRegistryGetGvgenotypeadaptor(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            return dba;

            break;

        default:

            ajWarn("ensRegistryGetGvgenotypeAdaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvindividualadaptor ************************************
**
** Get an Ensembl Genetic Variation Individual Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvindividualadaptor]
** Ensembl Genetic Variation Individual Adaptor or NULL
** @@
******************************************************************************/

EnsPGvindividualadaptor ensRegistryGetGvindividualadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvindividualadaptor)
                rgv->Gvindividualadaptor =
                    ensGvindividualadaptorNew(dba);

            return rgv->Gvindividualadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvindividualadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvpopulationadaptor ************************************
**
** Get an Ensembl Genetic Variation Population Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensRegistryGetGvpopulationadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvpopulationadaptor)
                rgv->Gvpopulationadaptor =
                    ensGvpopulationadaptorNew(dba);

            return rgv->Gvpopulationadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvpopulationadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvpopulationgenotypeadaptor ****************************
**
** Get an Ensembl Genetic Variation Population Genotype Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationgenotypeadaptor ensRegistryGetGvpopulationgenotypeadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvpopulationgenotypeadaptor)
                rgv->Gvpopulationgenotypeadaptor =
                    ensGvpopulationgenotypeadaptorNew(dba);

            return rgv->Gvpopulationgenotypeadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvpopulationgenotypeadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvsampleadaptor ****************************************
**
** Get an Ensembl Genetic Variation Sample Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor or NULL
** @@
******************************************************************************/

EnsPGvsampleadaptor ensRegistryGetGvsampleadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvsampleadaptor)
                rgv->Gvsampleadaptor =
                    ensGvsampleadaptorNew(dba);

            return rgv->Gvsampleadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvsampleadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvsourceadaptor ****************************************
**
** Get an Ensembl Genetic Variation Source Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor or NULL
** @@
******************************************************************************/

EnsPGvsourceadaptor ensRegistryGetGvsourceadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvsourceadaptor)
                rgv->Gvsourceadaptor =
                    ensGvsourceadaptorNew(dba);

            return rgv->Gvsourceadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvsourceadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvsynonymadaptor ***************************************
**
** Get an Ensembl Genetic Variation Synonym Adaptor from the
** Ensembl Registry.
** The Ensembl Genetic Variation Synonym Adaptor is an alias for an
** Ensembl Database Adaptor connected to an Ensembl Variation database.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvsynonymadaptor]
** Ensembl Genetic Variation Synonym Adaptor or NULL
** @@
******************************************************************************/

EnsPGvsynonymadaptor ensRegistryGetGvsynonymadaptor(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            return dba;

            break;

        default:

            ajWarn("ensRegistryGetGvsynonymadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvtranscriptvariationadaptor ***************************
**
** Get an Ensembl Genetic Variation Transcript Variation Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor or NULL
** @@
******************************************************************************/

EnsPGvtranscriptvariationadaptor ensRegistryGetGvtranscriptvariationadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvtranscriptvariationadaptor)
                rgv->Gvtranscriptvariationadaptor =
                    ensGvtranscriptvariationadaptorNew(dba);

            return rgv->Gvtranscriptvariationadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvtranscriptvariationadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvvariationadaptor *************************************
**
** Get an Ensembl Genetic Variation Variation Adaptor from the
** Ensembl Registry.
** The Ensembl Genetic Variation Variation Adaptor is an alias for an
** Ensembl Genetic Variation Database Adaptor.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationadaptor ensRegistryGetGvvariationadaptor(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            return ensRegistryGetGvdatabaseadaptor(dba);

            break;

        default:

            ajWarn("ensRegistryGetGvvariationadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvvariationfeatureadaptor ******************************
**
** Get an Ensembl Genetic Variation Variation Feature Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationfeatureadaptor ensRegistryGetGvvariationfeatureadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvvariationfeatureadaptor)
                rgv->Gvvariationfeatureadaptor =
                    ensGvvariationfeatureadaptorNew(dba);

            return rgv->Gvvariationfeatureadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvvariationfeatureadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}




/* @func ensRegistryGetGvvariationsetadaptor **********************************
**
** Get an Ensembl Genetic Variation Variation Set Adaptor from the
** Ensembl Registry.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationsetadaptor ensRegistryGetGvvariationsetadaptor(
    EnsPDatabaseadaptor dba)
{
    RegistryPEntry entry          = NULL;
    RegistryPGeneticVariation rgv = NULL;

    if(!dba)
        return NULL;

    entry = (RegistryPEntry) ajTableFetchmodV(
        registryEntry,
        (const void*) ensDatabaseadaptorGetSpecies(dba));

    if(!entry)
        return NULL;

    switch(ensDatabaseadaptorGetGroup(dba))
    {
        case ensEDatabaseadaptorGroupGeneticVariation:

            rgv = (RegistryPGeneticVariation)
                entry->Registry[ensDatabaseadaptorGetGroup(dba)];

            if(!rgv)
                break;

            if(!rgv->Gvvariationsetadaptor)
                rgv->Gvvariationsetadaptor =
                    ensGvvariationsetadaptorNew(dba);

            return rgv->Gvvariationsetadaptor;

            break;

        default:

            ajWarn("ensRegistryGetGvvariationsetadaptor got an "
                   "Ensembl Database Adaptor "
                   "with an unexpected group %d.\n",
                   ensDatabaseadaptorGetGroup(dba));
    }

    return NULL;
}

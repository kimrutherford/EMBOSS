/* @source ensdatabaseentry ***************************************************
**
** Ensembl Database Entry functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.52 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdatabaseentry.h"
#include "ensgene.h"
#include "enstranscript.h"
#include "enstranslation.h"
#include "enstable.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic externalreferenceKInfotype ************************************
**
** The Ensembl External Reference Information Type member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to the
** EnsEExternalreferenceInfotype enumeration.
**
******************************************************************************/

static const char *externalreferenceKInfotype[] =
{
    "",
    "PROJECTION",
    "MISC",
    "DEPENDENT",
    "DIRECT",
    "SEQUENCE_MATCH",
    "INFERRED_PAIR",
    "PROBE",
    "UNMAPPED",
    "COORDINATE_OVERLAP",
    (const char *) NULL
};




/* @conststatic externalreferenceObjecttype ***********************************
**
** The Ensembl External Reference Object Type member is enumerated in both,
** the SQL table definition and the data structure. The following strings are
** used for conversion in database operations and correspond to the
** EnsEExternalreferenceObjecttype enumeration.
**
******************************************************************************/

static const char *externalreferenceObjecttype[] =
{
    "",
    "RawContig",
    "Transcript",
    "Gene",
    "Translation",
    (const char *) NULL
};




/* @conststatic ontologylinkageType *******************************************
**
** The Ensembl Ontology Linkage Type member is enumerated in both,
** the SQL table definition and the data structure. The following strings are
** used for conversion in database operations and correspond to
** EnsEOntologylinkageType.
**
** http://www.geneontology.org/GO.evidence.shtml
******************************************************************************/

static const char *ontologylinkageType[] =
{
    "",
    "IC",  /* Inferred by Curator */
    "IDA", /* Inferred from Direct Assay */
    "IEA", /* Inferred from Electronic Annotation */
    "IEP", /* Inferred from Expression Pattern */
    "IGI", /* Inferred from Genetic Interaction */
    "IMP", /* Inferred from Mutant Phenotype */
    "IPI", /* Inferred from Physical Interaction */
    "ISS", /* Inferred from Sequence or Structural Similarity */
    "NAS", /* Non-traceable Author Statement */
    "ND",  /* No biological Data available */
    "TAS", /* Traceable Author Statement */
    "NR",  /* Not Recorded */
    "RCA", /* Inferred from Reviewed Computational Analysis */
    "EXP", /* Inferred from Experiment */
    "ISO", /* Inferred from Sequence Orthology */
    "ISA", /* Inferred from Sequence Alignment */
    "ISM", /* Inferred from Sequence Model */
    "IGC", /* Inferred from Genomic Context */
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool databaseentryadaptorFetchAllbyStatement(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr statement,
    AjPList dbes);

static int databaseentryadaptorCompareIdentifier(
    const void *item1,
    const void *item2);

static void databaseentryadaptorDeleteIdentifier(void **Pitem, void *cl);

static AjBool databaseentryadaptorFetchAllDependenciesByObject(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr constraint,
    const AjPStr objecttype,
    ajuint objectidentifier,
    AjPList dbes);

static AjBool databaseentryadaptorRetrieveAllIdentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr ensembltype,
    const AjPStr extratype,
    const AjPStr dbname,
    AjPList idlist);

static AjBool databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    const AjPStr ensembltype,
    const AjPStr extratype,
    AjPList idlist);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensdatabaseentry **********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPExternalreference] Ensembl External Reference ************
**
** @nam2rule Externalreference Functions for manipulating
** Ensembl External Reference objects
**
** In this implmentation the Ensembl External Reference class has been split
** out of the Bio::EnsEMBL::DBEntry class and covers the "xref" SQL table.
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl External Reference by pointer.
** It is the responsibility of the user to first destroy any previous
** External Reference. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExternalreference]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy er [const EnsPExternalreference] Ensembl External Reference
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini edb [EnsPExternaldatabase] Ensembl External Database
** @argrule Ini primaryid [AjPStr] Primary identifier
** @argrule Ini displayid [AjPStr] Display identifier
** @argrule Ini version [AjPStr] Version
** @argrule Ini description [AjPStr] Description
** @argrule Ini linkageannotation [AjPStr] Linkage annotation
** @argrule Ini infotext [AjPStr] Information text
** @argrule Ini erit [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
** @argrule Ini erot [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
** @argrule Ini objectid [ajuint] Ensembl Object identifier
** @argrule Ref er [EnsPExternalreference] Ensembl External Reference
**
** @valrule * [EnsPExternalreference] Ensembl External Reference or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensExternalreferenceNewCpy *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternalreference] Ensembl External Reference or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExternalreference ensExternalreferenceNewCpy(
    const EnsPExternalreference er)
{
    EnsPExternalreference pthis = NULL;

    if (!er)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = er->Identifier;

    /*
      pthis->Adaptor = er->Adaptor;
    */

    pthis->Analysis = ensAnalysisNewRef(er->Analysis);

    pthis->Externaldatabase = ensExternaldatabaseNewRef(er->Externaldatabase);

    if (er->Primaryidentifier)
        pthis->Primaryidentifier = ajStrNewRef(er->Primaryidentifier);

    if (er->Displayidentifier)
        pthis->Displayidentifier = ajStrNewRef(er->Displayidentifier);

    if (er->Version)
        pthis->Version = ajStrNewRef(er->Version);

    if (er->Description)
        pthis->Description = ajStrNewRef(er->Description);

    if (er->Linkageannotation)
        pthis->Linkageannotation = ajStrNewRef(er->Linkageannotation);

    if (er->Infotext)
        pthis->Infotext = ajStrNewRef(er->Infotext);

    pthis->Infotype         = er->Infotype;
    pthis->Objecttype       = er->Objecttype;
    pthis->Objectidentifier = er->Objectidentifier;

    return pthis;
}




/* @func ensExternalreferenceNewIni *******************************************
**
** Constructor for an Ensembl External Reference with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @cc Bio::EnsEMBL::DBEntry::new
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] primaryid [AjPStr] Primary identifier
** @param [u] displayid [AjPStr] Display identifier
** @param [u] version [AjPStr] Version
** @param [u] description [AjPStr] Description
** @param [u] linkageannotation [AjPStr] Linkage annotation
** @param [u] infotext [AjPStr] Information text
** @param [u] erit [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
** @param [u] erot [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
** @param [r] objectid [ajuint] Ensembl Object identifier
**
** @return [EnsPExternalreference] Ensembl External Reference or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExternalreference ensExternalreferenceNewIni(
    ajuint identifier,
    EnsPAnalysis analysis,
    EnsPExternaldatabase edb,
    AjPStr primaryid,
    AjPStr displayid,
    AjPStr version,
    AjPStr description,
    AjPStr linkageannotation,
    AjPStr infotext,
    EnsEExternalreferenceInfotype erit,
    EnsEExternalreferenceObjecttype erot,
    ajuint objectid)
{
    EnsPExternalreference er = NULL;

    if (ajDebugTest("ensExternalreferenceNewIni"))
    {
        ajDebug("ensExternalreferenceNewIni\n"
                "  identifier %u\n"
                "  analysis %p\n"
                "  edb %p\n"
                "  primaryid '%S'\n"
                "  displayid '%S'\n"
                "  version '%S'\n"
                "  description '%S'\n"
                "  linkageannotation '%S'\n"
                "  infotext '%S'\n"
                "  erit '%s'\n"
                "  erot '%s'\n"
                "  objectid %u\n",
                identifier,
                analysis,
                edb,
                primaryid,
                displayid,
                version,
                description,
                linkageannotation,
                infotext,
                ensExternalreferenceInfotypeToChar(erit),
                ensExternalreferenceObjecttypeToChar(erot),
                objectid);

        ensAnalysisTrace(analysis, 1);

        ensExternaldatabaseTrace(edb, 1);
    }

    if (!edb)
        return NULL;

    if (!primaryid)
        return NULL;

    AJNEW0(er);

    er->Use = 1U;

    er->Identifier = identifier;

    er->Analysis = ensAnalysisNewRef(analysis);

    er->Externaldatabase = ensExternaldatabaseNewRef(edb);

    if (primaryid)
        er->Primaryidentifier = ajStrNewRef(primaryid);

    if (displayid)
        er->Displayidentifier = ajStrNewRef(displayid);

    if (version)
        er->Version = ajStrNewRef(version);

    if (description)
        er->Description = ajStrNewRef(description);

    if (linkageannotation)
        er->Linkageannotation = ajStrNewRef(linkageannotation);

    if (infotext)
        er->Infotext = ajStrNewRef(infotext);

    er->Infotype         = erit;
    er->Objecttype       = erot;
    er->Objectidentifier = objectid;

    return er;
}




/* @func ensExternalreferenceNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] er [EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternalreference] Ensembl External Reference or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternalreference ensExternalreferenceNewRef(
    EnsPExternalreference er)
{
    if (!er)
        return NULL;

    er->Use++;

    return er;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
**
** @nam3rule Del Destroy (free) an EEnsembl xternal Reference
**
** @argrule * Per [EnsPExternalreference*] Ensembl External Reference address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExternalreferenceDel **********************************************
**
** Default destructor for an Ensembl External Reference.
**
** @param [d] Per [EnsPExternalreference*] Ensembl External Reference address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensExternalreferenceDel(
    EnsPExternalreference *Per)
{
    EnsPExternalreference pthis = NULL;

    if (!Per)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensExternalreferenceDel"))
    {
        ajDebug("ensExternalreferenceDel\n"
                "  *Per %p\n",
                *Per);

        ensExternalreferenceTrace(*Per, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Per)
        return;

    pthis = *Per;

    pthis->Use--;

    if (pthis->Use)
    {
        *Per = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ensExternaldatabaseDel(&pthis->Externaldatabase);

    ajStrDel(&pthis->Primaryidentifier);
    ajStrDel(&pthis->Displayidentifier);
    ajStrDel(&pthis->Version);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->Linkageannotation);
    ajStrDel(&pthis->Infotext);

    AJFREE(pthis);

    *Per = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
**
** @nam3rule Get Return External Reference attribute(s)
** @nam4rule Adaptor Return the Ensembl External Reference Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Description Return the description
** @nam4rule Displayidentifier Return the display identifier
** @nam4rule Externaldatabase Return the Ensembl External Database
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Infotext Return the information text
** @nam4rule Infotype Return the
** Ensembl External Reference Information Type enumeration
** @nam4rule Objectidentifier Return the Ensembl Object identifier
** @nam4rule Objecttype Return the
** Ensembl External Reference Object Type enumeration
** @nam4rule Primaryidentifier Return the primary identifier
** @nam4rule Version Return the version
** @nam4rule Linkageannotation Return the linkage annotation
**
** @argrule * er [const EnsPExternalreference] External Reference
**
** @valrule Adaptor [EnsPExternalreferenceadaptor]
** Ensembl External Reference Adaptor or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayidentifier [AjPStr] Display identifier or NULL
** @valrule Externaldatabase [EnsPExternaldatabase] External Database or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Infotext [AjPStr] Information text or NULL
** @valrule Infotype [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration or
** ensEExternalreferenceInfotypeNULL
** @valrule Objectidentifier [ajuint] Ensembl Object identifier or 0U
** @valrule Objecttype [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration or
** ensEExternalreferenceObjecttypeNULL
** @valrule Primaryidentifier [AjPStr] Primary identifier or NULL
** @valrule Version [AjPStr] Version or NULL
** @valrule Linkageannotation [AjPStr] Linkage annotation or NULL
**
** @fcategory use
******************************************************************************/




#if AJFALSE
/* @func ensExternalreferenceGetAdaptor ***************************************
**
** Get the Ensembl External Reference Adaptor member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternalreferenceadaptor] Ensembl External Reference Adaptor
**                                        or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternalreferenceadaptor ensExternalreferenceGetAdaptor(
    const EnsPExternalreference er)
{
    return (er) ? er->Adaptor : NULL;
}
#endif /* AJFALSE */




/* @func ensExternalreferenceGetAnalysis **************************************
**
** Get the Ensembl Analysis member of an Ensembl External Reference.
**
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensExternalreferenceGetAnalysis(
    const EnsPExternalreference er)
{
    return (er) ? er->Analysis : NULL;
}




/* @func ensExternalreferenceGetDescription ***********************************
**
** Get the description member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetDescription(
    const EnsPExternalreference er)
{
    return (er) ? er->Description : NULL;
}




/* @func ensExternalreferenceGetDisplayidentifier *****************************
**
** Get the display identifier member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::display_id
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Display identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetDisplayidentifier(
    const EnsPExternalreference er)
{
    return (er) ? er->Displayidentifier : NULL;
}




/* @func ensExternalreferenceGetExternaldatabase ******************************
**
** Get the Ensembl External Database member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @cc Bio::EnsEMBL::DBEntry::release
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @cc Bio::EnsEMBL::DBEntry::description
** @cc Bio::EnsEMBL::DBEntry::status
** @cc Bio::EnsEMBL::DBEntry::type
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternaldatabase ensExternalreferenceGetExternaldatabase(
    const EnsPExternalreference er)
{
    return (er) ? er->Externaldatabase : NULL;
}




/* @func ensExternalreferenceGetIdentifier ************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensExternalreferenceGetIdentifier(
    const EnsPExternalreference er)
{
    return (er) ? er->Identifier : 0U;
}




/* @func ensExternalreferenceGetInfotext **************************************
**
** Get the information text member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_text
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Information text or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetInfotext(
    const EnsPExternalreference er)
{
    return (er) ? er->Infotext : NULL;
}




/* @func ensExternalreferenceGetInfotype **************************************
**
** Get the information type member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_type
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type or
** ensEExternalreferenceInfotypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEExternalreferenceInfotype ensExternalreferenceGetInfotype(
    const EnsPExternalreference er)
{
    return (er) ? er->Infotype : ensEExternalreferenceInfotypeNULL;
}




/* @func ensExternalreferenceGetLinkageannotation *****************************
**
** Get the linkage annotation member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::linkage_annotation
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Linkage annotation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetLinkageannotation(
    const EnsPExternalreference er)
{
    return (er) ? er->Linkageannotation : NULL;
}




/* @func ensExternalreferenceGetObjectidentifier ******************************
**
** Get the Ensembl Object identifier member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::ensembl_id
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [ajuint] Ensembl Object identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensExternalreferenceGetObjectidentifier(
    const EnsPExternalreference er)
{
    return (er) ? er->Objectidentifier : 0U;
}




/* @func ensExternalreferenceGetObjecttype ************************************
**
** Get the Ensembl External Refernece Object Type member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::ensembl_object_type
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type or
** ensEExternalreferenceObjecttypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEExternalreferenceObjecttype ensExternalreferenceGetObjecttype(
    const EnsPExternalreference er)
{
    return (er) ? er->Objecttype : ensEExternalreferenceObjecttypeNULL;
}




/* @func ensExternalreferenceGetPrimaryidentifier *****************************
**
** Get the primary identifier member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Primary identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetPrimaryidentifier(
    const EnsPExternalreference er)
{
    return (er) ? er->Primaryidentifier : NULL;
}




/* @func ensExternalreferenceGetVersion ***************************************
**
** Get the version member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::version
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Version or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetVersion(
    const EnsPExternalreference er)
{
    return (er) ? er->Version : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
**
** @nam3rule Set Set one member of an Ensembl External Reference
** @nam4rule Adaptor Set the Ensembl External Database Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Description Set the description
** @nam4rule Displayidentifier Set the display identifier
** @nam4rule Externaldatabase Set the Ensembl External Database
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Linkageannotation Set the linkage annotation
** @nam4rule Objectidentifier Set the Ensembl Object identifier
** @nam4rule Objecttype Set the
** Ensembl External Reference Object Type enumeration
** @nam4rule Primaryidentifier Set the primary identifier
** @nam4rule Version Set the version
** @nam4rule Infotext Set the information text
** @nam4rule Infotype Set the information type
**
** @argrule * er [EnsPExternalreference] Ensembl External Reference object
** @argrule Adaptor adaptor [EnsPExternalreferenceadaptor] Ensembl External
** Reference Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Description description [AjPStr] Description
** @argrule Displayidentifier displayid [AjPStr] Display identifier
** @argrule Externaldatabase edb [EnsPExternaldatabase] Ensembl External
** Database
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Infotext infotext [AjPStr] Information text
** @argrule Infotype erit [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
** @argrule Linkageannotation linkageannotation [AjPStr] Linkage annotation
** @argrule Objectidentifier objectid [ajuint] Ensembl Object identifier
** @argrule Objecttype erot [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
** @argrule Primaryidentifier primaryid [AjPStr] Primary identifier
** @argrule Version version [AjPStr] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




#if AJFALSE
/* @func ensExternalreferenceSetAdaptor ***************************************
**
** Set the Ensembl External Reference Adaptor member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] adaptor [EnsPExternalreferenceadaptor] Ensembl External
**                                                   Reference Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetAdaptor(EnsPExternalreference er,
                                      EnsPExternalreferenceadaptor adaptor)
{
    if (!er)
        return ajFalse;

    er->Adaptor = adaptor;

    return ajTrue;
}
#endif /* AJFALSE */




/* @func ensExternalreferenceSetAnalysis **************************************
**
** Set the Ensembl Analysis member of an Ensembl External Reference.
**
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetAnalysis(EnsPExternalreference er,
                                       EnsPAnalysis analysis)
{
    if (!er)
        return ajFalse;

    ensAnalysisDel(&er->Analysis);

    er->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensExternalreferenceSetDescription ***********************************
**
** Set the description member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetDescription(EnsPExternalreference er,
                                          AjPStr description)
{
    if (!er)
        return ajFalse;

    ajStrDel(&er->Description);

    er->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensExternalreferenceSetDisplayidentifier *****************************
**
** Set the display identifier member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::display_id
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] displayid [AjPStr] Display identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetDisplayidentifier(EnsPExternalreference er,
                                                AjPStr displayid)
{
    if (!er)
        return ajFalse;

    ajStrDel(&er->Displayidentifier);

    er->Displayidentifier = ajStrNewRef(displayid);

    return ajTrue;
}




/* @func ensExternalreferenceSetExternaldatabase ******************************
**
** Set the Ensembl External Database member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @cc Bio::EnsEMBL::DBEntry::release
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @cc Bio::EnsEMBL::DBEntry::description
** @cc Bio::EnsEMBL::DBEntry::status
** @cc Bio::EnsEMBL::DBEntry::type
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetExternaldatabase(EnsPExternalreference er,
                                               EnsPExternaldatabase edb)
{
    if (!er)
        return ajFalse;

    ensExternaldatabaseDel(&er->Externaldatabase);

    er->Externaldatabase = ensExternaldatabaseNewRef(edb);

    return ajTrue;
}




/* @func ensExternalreferenceSetIdentifier ************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetIdentifier(EnsPExternalreference er,
                                         ajuint identifier)
{
    if (!er)
        return ajFalse;

    er->Identifier = identifier;

    return ajTrue;
}




/* @func ensExternalreferenceSetInfotext **************************************
**
** Set the information text member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_text
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] infotext [AjPStr] Information text
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetInfotext(EnsPExternalreference er,
                                       AjPStr infotext)
{
    if (!er)
        return ajFalse;

    ajStrDel(&er->Infotext);

    er->Infotext = ajStrNewRef(infotext);

    return ajTrue;
}




/* @func ensExternalreferenceSetInfotype **************************************
**
** Set the Ensembl External Reference Information Type member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_type
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] erit [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetInfotype(EnsPExternalreference er,
                                       EnsEExternalreferenceInfotype erit)
{
    if (!er)
        return ajFalse;

    er->Infotype = erit;

    return ajTrue;
}




/* @func ensExternalreferenceSetLinkageannotation *****************************
**
** Set the linkage annotation member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::linkage_annotation
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] linkageannotation [AjPStr] Linkage annotation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetLinkageannotation(EnsPExternalreference er,
                                                AjPStr linkageannotation)
{
    if (!er)
        return ajFalse;

    ajStrDel(&er->Linkageannotation);

    er->Linkageannotation = ajStrNewRef(linkageannotation);

    return ajTrue;
}




/* @func ensExternalreferenceSetObjectidentifier ******************************
**
** Set the Ensembl Object identifier member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::ensembl_id
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [r] objectid [ajuint] Ensembl Object identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetObjectidentifier(EnsPExternalreference er,
                                               ajuint objectid)
{
    if (!er)
        return ajFalse;

    er->Objectidentifier = objectid;

    return ajTrue;
}




/* @func ensExternalreferenceSetObjecttype ************************************
**
** Set the Ensembl External Reference Object Type member of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::ensembl_object_type
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] erot [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetObjecttype(EnsPExternalreference er,
                                         EnsEExternalreferenceObjecttype erot)
{
    if (!er)
        return ajFalse;

    er->Objecttype = erot;

    return ajTrue;
}




/* @func ensExternalreferenceSetPrimaryidentifier *****************************
**
** Set the primary identifier member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] primaryid [AjPStr] Primary identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetPrimaryidentifier(EnsPExternalreference er,
                                                AjPStr primaryid)
{
    if (!er)
        return ajFalse;

    ajStrDel(&er->Primaryidentifier);

    er->Primaryidentifier = ajStrNewRef(primaryid);

    return ajTrue;
}




/* @func ensExternalreferenceSetVersion ***************************************
**
** Set the version member of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::version
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] version [AjPStr] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceSetVersion(EnsPExternalreference er,
                                      AjPStr version)
{
    if (!er)
        return ajFalse;

    ajStrDel(&er->Version);

    er->Version = ajStrNewRef(version);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
**
** @nam3rule Trace Report Ensembl External Reference members to debug file
**
** @argrule Trace er [const EnsPExternalreference] Ensembl External Reference
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensExternalreferenceTrace ********************************************
**
** Trace an Ensembl External Reference.
**
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternalreferenceTrace(const EnsPExternalreference er, ajuint level)
{
    AjPStr indent = NULL;

    if (!er)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensExternalreferenceTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
#if AJFALSE
            "%S  Adaptor %p\n"
#endif /* AJFALSE */
            "%S  Analysis %p\n"
            "%S  Externaldatabase %p\n"
            "%S  Primaryidentifier '%S'\n"
            "%S  Displayidentifier '%S'\n"
            "%S  Version '%S'\n"
            "%S  Description '%S'\n"
            "%S  Linkageannotation '%S'\n"
            "%S  Infotext '%S'\n"
            "%S  Infotype '%s'\n"
            "%S  Objecttype '%s'\n"
            "%S  Objectidentifier %u\n",
            indent, er,
            indent, er->Use,
            indent, er->Identifier,
#if AJFALSE
            indent, er->Adaptor,
#endif /* AJFALSE */
            indent, er->Analysis,
            indent, er->Externaldatabase,
            indent, er->Primaryidentifier,
            indent, er->Displayidentifier,
            indent, er->Version,
            indent, er->Description,
            indent, er->Linkageannotation,
            indent, er->Infotext,
            indent, ensExternalreferenceInfotypeToChar(er->Infotype),
            indent, ensExternalreferenceObjecttypeToChar(er->Objecttype),
            indent, er->Objectidentifier);

    ensAnalysisTrace(er->Analysis, level + 1);

    ensExternaldatabaseTrace(er->Externaldatabase, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
**
** @nam3rule Calculate Calculate Ensembl External Reference values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * er [const EnsPExternalreference] Ensembl External Reference
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensExternalreferenceCalculateMemsize *********************************
**
** Calculate the memory size in bytes of an Ensembl External Reference.
**
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensExternalreferenceCalculateMemsize(const EnsPExternalreference er)
{
    size_t size = 0;

    if (!er)
        return 0;

    size += sizeof (EnsOExternalreference);

    size += ensAnalysisCalculateMemsize(er->Analysis);

    size += ensExternaldatabaseCalculateMemsize(er->Externaldatabase);

    if (er->Primaryidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Primaryidentifier);
    }

    if (er->Displayidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Displayidentifier);
    }

    if (er->Version)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Version);
    }

    if (er->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Description);
    }

    if (er->Linkageannotation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Linkageannotation);
    }

    if (er->Infotext)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Infotext);
    }

    return size;
}




/* @datasection [EnsEExternalreferenceInfotype] Ensembl External Reference
** Information Type
**
** @nam2rule Externalreference Functions for manipulating
** Ensembl External Reference objects
** @nam3rule ExternalreferenceInfotype Functions for manipulating
** Ensembl External Reference Information Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl External Reference Information Type enumeration.
**
** @fdata [EnsEExternalreferenceInfotype]
**
** @nam4rule From Ensembl External Reference Information Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  infotype  [const AjPStr] Infotype string
**
** @valrule * [EnsEExternalreferenceInfotype] Ensembl External Reference
** Information Type enumeration or ensEExternalreferenceInfotypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExternalreferenceInfotypeFromStr **********************************
**
** Convert an AJAX String into an Ensembl External Reference Information Type
** enumeration.
**
** @param [r] infotype [const AjPStr] Information Type string
**
** @return [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration or
** ensEExternalreferenceInfotypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEExternalreferenceInfotype ensExternalreferenceInfotypeFromStr(
    const AjPStr infotype)
{
    register EnsEExternalreferenceInfotype i =
        ensEExternalreferenceInfotypeNULL;

    EnsEExternalreferenceInfotype erit =
        ensEExternalreferenceInfotypeNULL;

    for (i = ensEExternalreferenceInfotypeNULL;
         externalreferenceKInfotype[i];
         i++)
        if (ajStrMatchC(infotype, externalreferenceKInfotype[i]))
            erit = i;

    if (!erit)
        ajDebug("ensExternalreferenceInfotypeFromStr encountered "
                "unexpected string '%S'.\n", infotype);

    return erit;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl External Reference Information Type enumeration.
**
** @fdata [EnsEExternalreferenceInfotype]
**
** @nam4rule To   Return Ensembl External Reference Information Type
**                enumeration
** @nam5rule Char Return C character string value
**
** @argrule To erit [EnsEExternalreferenceInfotype] Ensembl External
** Reference Information Type enumeration
**
** @valrule Char [const char*]  Ensembl External Reference Information Type
**                              C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensExternalreferenceInfotypeToChar ***********************************
**
** Convert an Ensembl External Reference Information Type enumeration into a
** C-type (char *) string.
**
** @param [u] erit [EnsEExternalreferenceInfotype] Ensembl External
** Reference Information Type enumeration
**
** @return [const char*] Ensembl External Reference Information Type
**                       C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensExternalreferenceInfotypeToChar(
    EnsEExternalreferenceInfotype erit)
{
    register EnsEExternalreferenceInfotype i =
        ensEExternalreferenceInfotypeNULL;

    for (i = ensEExternalreferenceInfotypeNULL;
         externalreferenceKInfotype[i] && (i < erit);
         i++);

    if (!externalreferenceKInfotype[i])
        ajDebug("ensExternalreferenceInfotypeToChar encountered an "
                "out of boundary error on Ensembl External Reference "
                "Information Type enumeration %d.\n",
                erit);

    return externalreferenceKInfotype[i];
}




/* @datasection [EnsEExternalreferenceObjecttype] Ensembl External Reference
** Object Type
**
** @nam2rule Externalreference Functions for manipulating
** Ensembl External Reference objects
** @nam3rule ExternalreferenceObjecttype Functions for manipulating
** Ensembl External Reference Object Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl External Reference Object Type enumeration.
**
** @fdata [EnsEExternalreferenceObjecttype]
**
** @nam4rule From Ensembl External Reference Object Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  objecttype [const AjPStr] Object Type string
**
** @valrule * [EnsEExternalreferenceObjecttype] Ensembl External Reference
** Object Type enumeration or ensEExternalreferenceObjecttypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExternalreferenceObjecttypeFromStr ********************************
**
** Convert an AJAX String into an Ensembl External Reference Object Type
** enumeration.
**
** @param [r] objecttype [const AjPStr] Object Type string
**
** @return [EnsEExternalreferenceObjecttype] Ensembl External Reference Object
** Type enumeration or ensEExternalreferenceObjecttypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEExternalreferenceObjecttype ensExternalreferenceObjecttypeFromStr(
    const AjPStr objecttype)
{
    register EnsEExternalreferenceObjecttype i =
        ensEExternalreferenceObjecttypeNULL;

    EnsEExternalreferenceObjecttype erot =
        ensEExternalreferenceObjecttypeNULL;

    for (i = ensEExternalreferenceObjecttypeNULL;
         externalreferenceObjecttype[i];
         i++)
        if (ajStrMatchC(objecttype, externalreferenceObjecttype[i]))
            erot = i;

    if (!erot)
        ajDebug("ensExternalreferenceObjecttypeFromStr encountered "
                "unexpected string '%S'.\n", objecttype);

    return erot;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl External Reference Object Type enumeration.
**
** @fdata [EnsEExternalreferenceObjecttype]
**
** @nam4rule To   Return Ensembl External Reference Object Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To erot [EnsEExternalreferenceObjecttype] Ensembl External
** Reference Object Type enumeration
**
** @valrule Char [const char*] Ensembl External Reference Object Type
**
** @fcategory cast
******************************************************************************/




/* @func ensExternalreferenceObjecttypeToChar *********************************
**
** Convert an Ensembl External Reference Object Type enumeration into a
** C-type (char *) string.
**
** @param [u] erot [EnsEExternalreferenceObjecttype] Ensembl External
** Reference Object Type enumeration
**
** @return [const char*] Ensembl External Reference Object Type
**                       C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensExternalreferenceObjecttypeToChar(
    EnsEExternalreferenceObjecttype erot)
{
    register EnsEExternalreferenceObjecttype i =
        ensEExternalreferenceObjecttypeNULL;

    for (i = ensEExternalreferenceObjecttypeNULL;
         externalreferenceObjecttype[i] && (i < erot);
         i++);

    if (!externalreferenceObjecttype[i])
        ajDebug("ensExternalreferenceObjecttypeToChar encountered an "
                "out of boundary error on "
                "Ensembl External Reference Object Type enumeration %d.\n",
                erot);

    return externalreferenceObjecttype[i];
}




/* @datasection [EnsPIdentityreference] Ensembl Identity Reference ************
**
** @nam2rule Identityreference Functions for manipulating
** Ensembl Identity Reference objects
**
** @cc Bio::EnsEMBL::IdentityXref
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Identity Reference by pointer.
** It is the responsibility of the user to first destroy any previous
** Identity Reference. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPIdentityreference]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ir [const EnsPIdentityreference] Ensembl Identity Reference
** @argrule Ini cigar [AjPStr] Cigar line
** @argrule Ini qstart [ajint] Query start coordinate
** @argrule Ini qend [ajint] Query end coordinate
** @argrule Ini qidentity [ajint] Query sequence identity
** @argrule Ini tstart [ajint] Target start coordinate
** @argrule Ini tend [ajint] Target end coordinate
** @argrule Ini tidentity [ajint] Target sequence identity
** @argrule Ini score [double] Alignment score
** @argrule Ini evalue [double] Expectation value
** @argrule Ref ir [EnsPIdentityreference] Ensembl Identity Reference
**
** @valrule * [EnsPIdentityreference] Ensembl Identity Reference or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensIdentityreferenceNewCpy *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [EnsPIdentityreference] Ensembl Identity Reference or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPIdentityreference ensIdentityreferenceNewCpy(
    const EnsPIdentityreference ir)
{
    EnsPIdentityreference pthis = NULL;

    if (!ir)
        return NULL;

    AJNEW0(pthis);

    if (ir->Cigar)
        pthis->Cigar = ajStrNewRef(ir->Cigar);

    pthis->QueryStart     = ir->QueryStart;
    pthis->QueryEnd       = ir->QueryEnd;
    pthis->QueryIdentity  = ir->QueryIdentity;
    pthis->TargetStart    = ir->TargetStart;
    pthis->TargetEnd      = ir->TargetEnd;
    pthis->TargetIdentity = ir->TargetIdentity;
    pthis->Use            = 1U;
    pthis->Evalue         = ir->Evalue;
    pthis->Score          = ir->Score;

    return pthis;
}




/* @func ensIdentityreferenceNewIni *******************************************
**
** Constructor for an Ensembl Identity Reference with initial values.
**
** @cc Bio::EnsEMBL::IdentityXref::new
** @param [u] cigar [AjPStr] Cigar line
** @param [r] qstart [ajint] Query start coordinate
** @param [r] qend [ajint] Query end coordinate
** @param [r] qidentity [ajint] Query sequence identity
** @param [r] tstart [ajint] Target start coordinate
** @param [r] tend [ajint] Target end coordinate
** @param [r] tidentity [ajint] Target sequence identity
** @param [r] score [double] Alignment score
** @param [r] evalue [double] Expectation value
**
** @return [EnsPIdentityreference] Ensembl Identity Reference or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPIdentityreference ensIdentityreferenceNewIni(AjPStr cigar,
                                                 ajint qstart,
                                                 ajint qend,
                                                 ajint qidentity,
                                                 ajint tstart,
                                                 ajint tend,
                                                 ajint tidentity,
                                                 double score,
                                                 double evalue)
{
    EnsPIdentityreference ir = NULL;

    AJNEW0(ir);

    if (cigar)
        ir->Cigar = ajStrNewRef(cigar);
    else
        ir->Cigar = ajStrNew();

    ir->QueryStart     = qstart;
    ir->QueryEnd       = qend;
    ir->QueryIdentity  = qidentity;
    ir->TargetStart    = tstart;
    ir->TargetEnd      = tend;
    ir->TargetIdentity = tidentity;
    ir->Score          = score;
    ir->Evalue         = evalue;

    return ir;
}




/* @func ensIdentityreferenceNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
**
** @return [EnsPIdentityreference] Ensembl Identity Reference or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPIdentityreference ensIdentityreferenceNewRef(
    EnsPIdentityreference ir)
{
    if (!ir)
        return NULL;

    ir->Use++;

    return ir;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
**
** @nam3rule Del Destroy (free) an Ensembl Identity Reference
**
** @argrule * Pir [EnsPIdentityreference*] Ensembl Identity Reference address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensIdentityreferenceDel **********************************************
**
** Default destructor for an Ensembl Identity Reference.
**
** @param [d] Pir [EnsPIdentityreference*] Ensembl Identity Reference address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensIdentityreferenceDel(EnsPIdentityreference *Pir)
{
    EnsPIdentityreference pthis = NULL;

    if (!Pir)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensIdentityreferenceDel"))
    {
        ajDebug("ensIdentityreferenceDel\n"
                "  *Pir %p\n",
                *Pir);

        ensIdentityreferenceTrace(*Pir, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pir)
        return;

    pthis = *Pir;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pir = NULL;

        return;
    }

    ajStrDel(&pthis->Cigar);

    AJFREE(pthis);

    *Pir = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
**
** @nam3rule Get Get attribute
** @nam4rule Cigar Return the CIGAR line
** @nam4rule Evalue Return the e-value
** @nam4rule Query Return query members
** @nam5rule End Return the query end
** @nam5rule Identity Return the query identity
** @nam4rule Score Return the score
** @nam4rule Target Return target members
** @nam5rule Start Return the query start
** @nam5rule End Return the target end
** @nam5rule Identity Return the target identity
** @nam5rule Start Return the target start
**
** @argrule * ir [const EnsPIdentityreference] Identity Reference
**
** @valrule Cigar [AjPStr] Cigar line or NULL
** @valrule Evalue [double] E-value or 0.0
** @valrule QueryEnd [ajint] Query end or 0
** @valrule QueryIdentity [ajint] Query identity or 0
** @valrule QueryStart [ajint] Query start or 0
** @valrule Score [double] Score or 0.0
** @valrule TargetEnd [ajint] Target end or 0
** @valrule TargetIdentity [ajint] Target identity or 0
** @valrule TargetStart [ajint] Target start or 0
**
** @fcategory use
******************************************************************************/




/* @func ensIdentityreferenceGetCigar *****************************************
**
** Get the CIGAR line member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::cigar_line
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [AjPStr] CIGAR line or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensIdentityreferenceGetCigar(const EnsPIdentityreference ir)
{
    return (ir) ? ir->Cigar : NULL;
}




/* @func ensIdentityreferenceGetEvalue ****************************************
**
** Get the e-value member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::evalue
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [double] E-value or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensIdentityreferenceGetEvalue(const EnsPIdentityreference ir)
{
    return (ir) ? ir->Evalue : 0.0;
}





/* @func ensIdentityreferenceGetQueryEnd **************************************
**
** Get the query end member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_end
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Query end or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensIdentityreferenceGetQueryEnd(const EnsPIdentityreference ir)
{
    return (ir) ? ir->QueryEnd : 0;
}




/* @func ensIdentityreferenceGetQueryIdentity *********************************
**
** Get the query identity member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_identity
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Query identity or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensIdentityreferenceGetQueryIdentity(const EnsPIdentityreference ir)
{
    return (ir) ? ir->QueryIdentity : 0;
}




/* @func ensIdentityreferenceGetQueryStart ************************************
**
** Get the query start member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_start
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Query start or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensIdentityreferenceGetQueryStart(const EnsPIdentityreference ir)
{
    return (ir) ? ir->QueryStart : 0;
}




/* @func ensIdentityreferenceGetScore *****************************************
**
** Get the score member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::score
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [double] Score or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensIdentityreferenceGetScore(const EnsPIdentityreference ir)
{
    return (ir) ? ir->Score : 0.0;
}




/* @func ensIdentityreferenceGetTargetEnd *************************************
**
** Get the target end member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_end
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Target end or 0
**
** @release 6.3.0
** @@
******************************************************************************/

ajint ensIdentityreferenceGetTargetEnd(const EnsPIdentityreference ir)
{
    return (ir) ? ir->TargetEnd : 0;
}




/* @func ensIdentityreferenceGetTargetIdentity ********************************
**
** Get the target identity member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_identity
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Target identity or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensIdentityreferenceGetTargetIdentity(const EnsPIdentityreference ir)
{
    return (ir) ? ir->TargetIdentity : 0;
}




/* @func ensIdentityreferenceGetTargetStart ***********************************
**
** Get the target start member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_start
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Target start or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensIdentityreferenceGetTargetStart(const EnsPIdentityreference ir)
{
    return (ir) ? ir->TargetStart : 0;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
**
** @nam3rule Set Set one member of an Ensembl Identity Reference
** @nam4rule Cigar Set the CIGAR line
** @nam4rule Evalue Set the e-value
** @nam4rule Query Set query members
** @nam5rule End Set the query end
** @nam5rule Identity Set the query identity
** @nam5rule Start Set the query start
** @nam4rule Score Set the score
** @nam4rule Target Set target members
** @nam5rule End Set the target end
** @nam5rule Identity Set the target identity
** @nam5rule Start Set the target start
**
** @argrule * ir [EnsPIdentityreference] Ensembl Identity Reference object
** @argrule Cigar cigar [AjPStr] CIGAR-line
** @argrule Evalue evalue [double] E-value
** @argrule QueryEnd qend [ajint] Query end
** @argrule QueryIdentity qidentity [ajint] Query identity
** @argrule QueryStart qstart [ajint] Query start
** @argrule Score score [double] Score
** @argrule TargetEnd tend [ajint] Target end
** @argrule TargetIdentity tidentity [ajint] Target identity
** @argrule TargetStart tstart [ajint] Target start
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensIdentityreferenceSetCigar *****************************************
**
** Set the CIGAR-line member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::cigar_line
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [u] cigar [AjPStr] CIGAR-line
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetCigar(EnsPIdentityreference ir,
                                    AjPStr cigar)
{
    if (!ir)
        return ajFalse;

    ajStrDel(&ir->Cigar);

    ir->Cigar = ajStrNewRef(cigar);

    return ajTrue;
}




/* @func ensIdentityreferenceSetEvalue ****************************************
**
** Set the e-value member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::evalue
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] evalue [double] E-value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetEvalue(EnsPIdentityreference ir,
                                     double evalue)
{
    if (!ir)
        return ajFalse;

    ir->Evalue = evalue;

    return ajTrue;
}




/* @func ensIdentityreferenceSetQueryEnd **************************************
**
** Set the query end member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_end
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] qend [ajint] Query end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetQueryEnd(EnsPIdentityreference ir,
                                       ajint qend)
{
    if (!ir)
        return ajFalse;

    ir->QueryEnd = qend;

    return ajTrue;
}




/* @func ensIdentityreferenceSetQueryIdentity *********************************
**
** Set the query identity member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_identity
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] qidentity [ajint] Query identity
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetQueryIdentity(EnsPIdentityreference ir,
                                            ajint qidentity)
{
    if (!ir)
        return ajFalse;

    ir->QueryIdentity = qidentity;

    return ajTrue;
}




/* @func ensIdentityreferenceSetQueryStart ************************************
**
** Set the query start member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_start
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] qstart [ajint] Query start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetQueryStart(EnsPIdentityreference ir,
                                         ajint qstart)
{
    if (!ir)
        return ajFalse;

    ir->QueryStart = qstart;

    return ajTrue;
}




/* @func ensIdentityreferenceSetScore *****************************************
**
** Set the score member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::score
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetScore(EnsPIdentityreference ir,
                                    double score)
{
    if (!ir)
        return ajFalse;

    ir->Score = score;

    return ajTrue;
}




/* @func ensIdentityreferenceSetTargetEnd *************************************
**
** Set the target end member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_end
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] tend [ajint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetTargetEnd(EnsPIdentityreference ir,
                                        ajint tend)
{
    if (!ir)
        return ajFalse;

    ir->TargetEnd = tend;

    return ajTrue;
}




/* @func ensIdentityreferenceSetTargetIdentity ********************************
**
** Set the target identity member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_identity
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] tidentity [ajint] Target identity
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetTargetIdentity(EnsPIdentityreference ir,
                                             ajint tidentity)
{
    if (!ir)
        return ajFalse;

    ir->TargetIdentity = tidentity;

    return ajTrue;
}




/* @func ensIdentityreferenceSetTargetStart ***********************************
**
** Set the target start member of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_start
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] tstart [ajint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetTargetStart(EnsPIdentityreference ir,
                                          ajint tstart)
{
    if (!ir)
        return ajFalse;

    ir->TargetStart = tstart;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
**
** @nam3rule Trace Report Ensembl Identity Reference members to debug file
**
** @argrule Trace ir [const EnsPIdentityreference] Ensembl Identity Reference
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensIdentityreferenceTrace ********************************************
**
** Trace an Ensembl Identity Reference.
**
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensIdentityreferenceTrace(const EnsPIdentityreference ir, ajuint level)
{
    AjPStr indent = NULL;

    if (!ir)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensIdentityreferenceTrace %p\n"
            "%S  Cigar '%S'\n"
            "%S  QueryStart %d\n"
            "%S  QueryEnd %d\n"
            "%S  QueryIdentity %d\n"
            "%S  TargetStart %d\n"
            "%S  TargetEnd %d\n"
            "%S  TargetIdentity %d\n",
            "%S  Use %u\n"
            "%S  Evalue %f\n"
            "%S  Score %f\n",
            indent, ir,
            indent, ir->Cigar,
            indent, ir->QueryStart,
            indent, ir->QueryEnd,
            indent, ir->QueryIdentity,
            indent, ir->TargetStart,
            indent, ir->TargetEnd,
            indent, ir->TargetIdentity,
            indent, ir->Use,
            indent, ir->Evalue,
            indent, ir->Score);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
**
** @nam3rule Calculate Calculate Ensembl Identity Reference values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensIdentityreferenceCalculateMemsize *********************************
**
** Calculate the memory size in bytes of an Ensembl Identity Reference.
**
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensIdentityreferenceCalculateMemsize(const EnsPIdentityreference ir)
{
    size_t size = 0;

    if (!ir)
        return 0;

    size += sizeof (EnsOIdentityreference);

    if (ir->Cigar)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ir->Cigar);
    }

    return size;
}




/* @datasection [EnsPOntologylinkage] Ensembl Ontology Linkage ****************
**
** @nam2rule Ontologylinkage Functions for manipulating
** Ensembl Ontology Linkage objects
**
** @cc Bio::EnsEMBL::OntologyXref
** @cc CVS Revision: 1.5
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ontology Linkage by pointer.
** It is the responsibility of the user to first destroy any previous
** Ontology Linkage. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPOntologylinkage]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ol [const EnsPOntologylinkage] Ensembl Ontology Linkage
** @argrule Ini linkagetype [AjPStr] Linkage type
** @argrule Ini source [EnsPDatabaseentry] Source Ensembl Database Entry
** @argrule Ref ol [EnsPOntologylinkage] Ensembl Ontology Linkage
**
** @valrule * [EnsPOntologylinkage] Ensembl Ontology Linkage or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOntologylinkageNewCpy *********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ol [const EnsPOntologylinkage] Ensembl Ontology Linkage
**
** @return [EnsPOntologylinkage] Ensembl Ontology Linkage or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPOntologylinkage ensOntologylinkageNewCpy(
    const EnsPOntologylinkage ol)
{
    EnsPOntologylinkage pthis = NULL;

    if (!ol)
        return NULL;

    AJNEW0(pthis);

    pthis->LinkageType = ajStrNewRef(ol->LinkageType);
    pthis->Source      = ensDatabaseentryNewCpy(ol->Source);

    pthis->Use = 1U;

    return pthis;
}




/* @func ensOntologylinkageNewIni *********************************************
**
** Constructor for an Ensembl Ontology Linkage with initial values.
**
** @param [u] linkagetype [AjPStr] Linkage type
** @param [u] source [EnsPDatabaseentry] Source Ensembl Database Entry
**
** @return [EnsPOntologylinkage] Ensembl Ontology Linkage or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPOntologylinkage ensOntologylinkageNewIni(
    AjPStr linkagetype,
    EnsPDatabaseentry source)
{
    EnsPOntologylinkage ol = NULL;

    if (ajDebugTest("ensOntologylinkageNewIni"))
    {
        ajDebug("ensOntologylinkageNewIni\n"
                "  linkagetype '%S'\n"
                "  source %p\n",
                linkagetype,
                source);

        ensDatabaseentryTrace(source, 1);
    }

    if (!linkagetype)
        return NULL;

    AJNEW0(ol);

    ol->LinkageType = ajStrNewRef(linkagetype);
    ol->Source      = ensDatabaseentryNewRef(source);

    ol->Use = 1U;

    return ol;
}




/* @func ensOntologylinkageNewRef *********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ol [EnsPOntologylinkage] Ensembl Ontology Linkage
**
** @return [EnsPOntologylinkage] Ensembl Ontology Linkage or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPOntologylinkage ensOntologylinkageNewRef(
    EnsPOntologylinkage ol)
{
    if (!ol)
        return NULL;

    ol->Use++;

    return ol;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ontology Linkage object.
**
** @fdata [EnsPOntologylinkage]
**
** @nam3rule Del Destroy (free) an Ensembl Ontology Linkage
**
** @argrule * Pol [EnsPOntologylinkage*] Ensembl Ontology Linkage address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOntologylinkageDel ************************************************
**
** Default destructor for an Ensembl Ontology Linkage.
**
** @param [d] Pol [EnsPOntologylinkage*] Ensembl Ontology Linkage address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensOntologylinkageDel(EnsPOntologylinkage *Pol)
{
    EnsPOntologylinkage pthis = NULL;

    if (!Pol)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOntologylinkageDel"))
        ajDebug("ensOntologylinkageDel\n"
                " *Pol %p\n",
                *Pol);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pol)
        return;

    pthis = *Pol;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pol = NULL;

        return;
    }

    ajStrDel(&pthis->LinkageType);

    ensDatabaseentryDel(&pthis->Source);

    AJFREE(pthis);

    *Pol = NULL;

    return;
}




/* @datasection [EnsEOntologylinkageType] Ensembl Ontology Linkage Type *******
**
** @nam2rule Ontologylinkage Functions for manipulating
** Ensembl Ontology Linkage objects
** @nam3rule OntologylinkageType Functions for manipulating
** Ensembl Ontology Linkage Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Ontology Linkage Type enumeration.
**
** @fdata [EnsEOntologylinkageType]
**
** @nam4rule From Ensembl Ontology Linkage Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Ensembl Ontology Linkage Type string
**
** @valrule * [EnsEOntologylinkageType] Ensembl Ontology Linkage Type
** enumeration or ensEOntologylinkageNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologylinkageTypeFromStr ****************************************
**
** Convert an AJAX String into an Ensembl Ontology Linkage Type enumeration.
**
** @param [r] type [const AjPStr] Ensembl Ontology Linkage Type string
**
** @return [EnsEOntologylinkageType] Ensembl Ontology Linkage Type
** enumeration or ensEOntologylinkageNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEOntologylinkageType ensOntologylinkageTypeFromStr(const AjPStr type)
{
    register EnsEOntologylinkageType i = ensEOntologylinkageTypeNULL;

    EnsEOntologylinkageType olt = ensEOntologylinkageTypeNULL;

    for (i = ensEOntologylinkageTypeNULL;
         ontologylinkageType[i];
         i++)
        if (ajStrMatchC(type, ontologylinkageType[i]))
            olt = i;

    if (!olt)
        ajDebug("ensOntologylinkageTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return olt;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Ontology Linkage Type enumeration.
**
** @fdata [EnsEOntologylinkageType]
**
** @nam4rule To   Return Ensembl Ontology Linkage Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To olt [EnsEOntologylinkageType]
** Ensembl Ontology Linkage Type enumeration
**
** @valrule Char [const char*] Ensembl Ontology Linkage Type C-type (char *)
** string
**
** @fcategory cast
******************************************************************************/




/* @func ensOntologylinkageTypeToChar *****************************************
**
** Convert an Ensembl Ontology Linkage Type enumeration into a
** C-type (char *) string.
**
** @param [u] olt [EnsEOntologylinkageType] Ensembl Ontology Linkage Type
** enumeration
**
** @return [const char*] Ensembl Ontology Linkage Type C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensOntologylinkageTypeToChar(EnsEOntologylinkageType olt)
{
    register EnsEOntologylinkageType i = ensEOntologylinkageTypeNULL;

    for (i = ensEOntologylinkageTypeNULL;
         ontologylinkageType[i] && (i < olt);
         i++);

    if (!ontologylinkageType[i])
        ajDebug("ensOntologylinkageTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Ontology Linkage Type enumeration %d.\n",
                olt);

    return ontologylinkageType[i];
}




/* @datasection [EnsPDatabaseentry] Ensembl Database Entry ********************
**
** @nam2rule Databaseentry Functions for manipulating
** Ensembl Database Entry objects
**
** @cc Bio::EnsEMBL::DBEntry
** @cc CVS Revision: 1.51
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Database Entry by pointer.
** It is the responsibility of the user to first destroy any previous
** Database Entry. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @argrule Ini dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini edb [EnsPExternaldatabase] Ensembl External Database
** @argrule Ini primaryid [AjPStr] Primary identifier
** @argrule Ini displayid [AjPStr] Display identifier
** @argrule Ini version [AjPStr] Version
** @argrule Ini description [AjPStr] Description
** @argrule Ini linkageannotation [AjPStr] Linkage annotation
** @argrule Ini infotext [AjPStr] Information text
** @argrule Ini erit [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
** @argrule Ini erot [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
** @argrule Ini objectid [ajuint] Ensembl Object identifier
** @argrule Ref dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @valrule * [EnsPDatabaseentry] Ensembl Database Entry or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseentryNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseentry ensDatabaseentryNewCpy(const EnsPDatabaseentry dbe)
{
    AjIList iter = NULL;

    AjPStr synonym = NULL;

    EnsPDatabaseentry pthis = NULL;

    EnsPOntologylinkage ol = NULL;

    if (!dbe)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = dbe->Identifier;

    pthis->Adaptor = dbe->Adaptor;

    pthis->Externalreference =
        ensExternalreferenceNewCpy(dbe->Externalreference);

    pthis->Identityreference =
        ensIdentityreferenceNewCpy(dbe->Identityreference);

    /* Copy the AJAX List of (synonym) AJAX String objects. */

    pthis->Synonyms = ajListstrNew();

    iter = ajListIterNew(dbe->Synonyms);

    while (!ajListIterDone(iter))
    {
        synonym = ajListstrIterGet(iter);

        ajListstrPushAppend(pthis->Synonyms, ajStrNewRef(synonym));
    }

    ajListIterDel(&iter);

    /* Copy the AJAX List of Ensembl Ontology Linkage objects. */

    pthis->Ontologylinkages = ajListNew();

    iter = ajListIterNew(dbe->Ontologylinkages);

    while (!ajListIterDone(iter))
    {
        ol = (EnsPOntologylinkage) ajListIterGet(iter);

        ajListPushAppend(pthis->Ontologylinkages,
                         (void *) ensOntologylinkageNewRef(ol));
    }

    ajListIterDel(&iter);

    return pthis;
}




/* @func ensDatabaseentryNewIni ***********************************************
**
** Constructor for an Ensembl Database Entry with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::DBEntry::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] primaryid [AjPStr] Primary identifier
** @param [u] displayid [AjPStr] Display identifier
** @param [u] version [AjPStr] Version
** @param [u] description [AjPStr] Description
** @param [u] linkageannotation [AjPStr] Linkage annotation
** @param [u] infotext [AjPStr] Information text
** @param [u] erit [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
** @param [u] erot [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
** @param [r] objectid [ajuint] Ensembl Object identifier
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseentry ensDatabaseentryNewIni(
    EnsPDatabaseentryadaptor dbea,
    ajuint identifier,
    EnsPAnalysis analysis,
    EnsPExternaldatabase edb,
    AjPStr primaryid,
    AjPStr displayid,
    AjPStr version,
    AjPStr description,
    AjPStr linkageannotation,
    AjPStr infotext,
    EnsEExternalreferenceInfotype erit,
    EnsEExternalreferenceObjecttype erot,
    ajuint objectid)
{
    EnsPDatabaseentry dbe = NULL;

    EnsPExternalreference er = NULL;

    if (ajDebugTest("ensDatabaseentryNewIni"))
    {
        ajDebug("ensDatabaseentryNewIni\n"
                "  dbea %p\n"
                "  identifier %u\n"
                "  analysis %p\n"
                "  edb %p\n"
                "  primaryid '%S'\n"
                "  displayid '%S'\n"
                "  version '%S'\n"
                "  description '%S'\n"
                "  linkageannotation '%S'\n"
                "  infotext '%S'\n"
                "  erit %d\n"
                "  erot %d\n"
                "  objectid %u\n",
                dbea,
                identifier,
                analysis,
                edb,
                primaryid,
                displayid,
                version,
                description,
                linkageannotation,
                infotext,
                erit,
                erot,
                objectid);

        ensAnalysisTrace(analysis, 1);

        ensExternaldatabaseTrace(edb, 1);
    }

    if (!edb)
        return NULL;

    er = ensExternalreferenceNewIni(identifier,
                                    analysis,
                                    edb,
                                    primaryid,
                                    displayid,
                                    version,
                                    description,
                                    linkageannotation,
                                    infotext,
                                    erit,
                                    erot,
                                    objectid);

    if (er)
    {
        AJNEW0(dbe);

        dbe->Use               = 1U;
        dbe->Identifier        = identifier;
        dbe->Adaptor           = dbea;
        dbe->Externalreference = er;
        dbe->Identityreference = NULL;
        dbe->Synonyms          = ajListstrNew();
        dbe->Ontologylinkages  = ajListNew();
    }
    else
        ajDebug("ensDatabaseentryNewIni could not create an "
                "External Reference.\n");

    return dbe;
}




/* @func ensDatabaseentryNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseentry ensDatabaseentryNewRef(EnsPDatabaseentry dbe)
{
    if (!dbe)
        return NULL;

    dbe->Use++;

    return dbe;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Del Destroy (free) an Ensembl Database Entry
**
** @argrule * Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseentryDel **************************************************
**
** Default destructor for an Ensembl Database Entry.
**
** @param [d] Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensDatabaseentryDel(EnsPDatabaseentry *Pdbe)
{
    EnsPOntologylinkage ol = NULL;

    EnsPDatabaseentry pthis = NULL;

    if (!Pdbe)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensDatabaseentryDel"))
    {
        ajDebug("ensDatabaseentryDel\n"
                "  *Pdbe %p\n",
                *Pdbe);

        ensDatabaseentryTrace(*Pdbe, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pdbe)
        return;

    pthis = *Pdbe;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pdbe = NULL;

        return;
    }

    ensExternalreferenceDel(&pthis->Externalreference);

    ensIdentityreferenceDel(&pthis->Identityreference);

    ajListstrFreeData(&pthis->Synonyms);

    while (ajListPop(pthis->Ontologylinkages, (void **) &ol))
        ensOntologylinkageDel(&ol);

    ajListFree(&pthis->Ontologylinkages);

    AJFREE(pthis);

    *Pdbe = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Get Return Database Entry attribute(s)
** @nam4rule Adaptor Return the Ensembl Database Entry Adaptor
** @nam4rule Ontologylinkages Return the Ensembl Ontology Linkage objects
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Synonyms Return synonyms
**
** @argrule * dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @valrule Adaptor [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @valrule Ontologylinkages [const AjPList] AJAX List of Ensembl
** Ontology Linkage objects
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Synonyms [AjPList] AJAX List of AJAX String (synonym) objecs
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseentryGetAdaptor *******************************************
**
** Get the Ensembl Database Entry Adaptor member of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseentryadaptor ensDatabaseentryGetAdaptor(
    const EnsPDatabaseentry dbe)
{
    return (dbe) ? dbe->Adaptor : NULL;
}




/* @func ensDatabaseentryGetIdentifier ****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [ajuint] SQL database-internal identifier  or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensDatabaseentryGetIdentifier(
    const EnsPDatabaseentry dbe)
{
    return (dbe) ? dbe->Identifier : 0U;
}




/* @func ensDatabaseentryGetOntologylinkages **********************************
**
** Get Ensembl Ontology Linkage objects of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::OntologyXref::get_all_linkage_info
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [const AjPList] AJAX List of Ensembl Ontology Linkage objects
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensDatabaseentryGetOntologylinkages(
    const EnsPDatabaseentry dbe)
{
    return (dbe) ? dbe->Ontologylinkages : NULL;
}




/* @func ensDatabaseentryGetSynonyms ******************************************
**
** Get the synonyms member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_synonyms
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPList] AJAX List of AJAX String (symnonym) objects
**
** @release 6.2.0
** @@
******************************************************************************/

AjPList ensDatabaseentryGetSynonyms(
    const EnsPDatabaseentry dbe)
{
    return (dbe) ? dbe->Synonyms : NULL;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Trace Report Ensembl Database Entry members to debug file
**
** @argrule Trace dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseentryTrace ************************************************
**
** Trace an Ensembl Database Entry.
**
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseentryTrace(const EnsPDatabaseentry dbe, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr synonym = NULL;
    AjPStr linkage = NULL;

    if (!dbe)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensDatabaseentryTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Externalreference %p\n"
            "%S  Identityreference %p\n"
            "%S  Synonyms %p\n"
            "%S  Ontologylinkages %p\n",
            indent, dbe,
            indent, dbe->Use,
            indent, dbe->Identifier,
            indent, dbe->Adaptor,
            indent, dbe->Externalreference,
            indent, dbe->Identityreference,
            indent, dbe->Synonyms,
            indent, dbe->Ontologylinkages);

    ensExternalreferenceTrace(dbe->Externalreference, level + 1);

    ensIdentityreferenceTrace(dbe->Identityreference, level + 1);

    /* Trace the AJAX List of (synonym) AJAX String objects. */

    if (dbe->Synonyms)
    {
        ajDebug("%S    AJAX List %p of (synonym) AJAX String objects:\n",
                indent, dbe->Synonyms);

        iter = ajListIterNewread(dbe->Synonyms);

        while (!ajListIterDone(iter))
        {
            synonym = ajListstrIterGet(iter);

            ajDebug("%S        '%S'\n", indent, synonym);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of (Ontology Linkage Type) AJAX String objects. */

    if (dbe->Ontologylinkages)
    {
        ajDebug("%S    AJAX List %p of (Ontology Linkage Type) "
                "AJAX String objects:\n",
                indent, dbe->Ontologylinkages);

        iter = ajListIterNewread(dbe->Ontologylinkages);

        while (!ajListIterDone(iter))
        {
            linkage = (AjPStr) ajListIterGet(iter);

            ajDebug("%S        '%S'\n", indent, linkage);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Calculate Calculate Ensembl Database Entry values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseentryCalculateMemsize *************************************
**
** Calculate the memory size in bytes of an Ensembl Database Entry.
**
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensDatabaseentryCalculateMemsize(const EnsPDatabaseentry dbe)
{
    size_t size = 0;

    AjIList iter = NULL;

    AjPStr synonym = NULL;
    AjPStr linkage = NULL;

    if (!dbe)
        return 0;

    size += sizeof (EnsODatabaseentry);

    size += ensExternalreferenceCalculateMemsize(dbe->Externalreference);

    size += ensIdentityreferenceCalculateMemsize(dbe->Identityreference);

    /* Summarise the AJAX List of (synonym) AJAX String objects. */

    if (dbe->Synonyms)
    {
        size += sizeof (AjOList);

        iter = ajListIterNew(dbe->Synonyms);

        while (!ajListIterDone(iter))
        {
            synonym = ajListstrIterGet(iter);

            if (synonym)
            {
                size += sizeof (AjOStr);

                size += ajStrGetRes(synonym);
            }
        }

        ajListIterDel(&iter);
    }

    /*
    ** Summarise the AJAX List of (Ontology Linkage Type)
    ** AJAX String objects.
    */

    if (dbe->Ontologylinkages)
    {
        size += sizeof (AjOList);

        iter = ajListIterNew(dbe->Ontologylinkages);

        while (!ajListIterDone(iter))
        {
            linkage = (AjPStr) ajListIterGet(iter);

            if (linkage)
            {
                size += sizeof (AjOStr);

                size += ajStrGetRes(linkage);
            }
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @section convenience functions *********************************************
**
** Ensembl Database Entry convenience functions
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Db Return External Database members
** @nam5rule Displayname Return the database display name
** @nam5rule Name Return the database name
** @nam5rule Release Return the database release
** @nam4rule Description Return the description
** @nam4rule Displayidentifier Return the display identifier
** @nam4rule Infotext Return the information text
** @nam4rule Infotype Return the
** Ensembl External Refernece Information Type enumeration
** @nam4rule Linkageannotation Return the linkage annotation
** @nam4rule Objectidentifier Return the Ensembl Object identifier
** @nam4rule Objecttype Return the
** Ensembl External Database Object Type enumeration
** @nam4rule Primaryidentifier Return the primary identifier
** @nam4rule Priority Return the priority
** @nam4rule Status Return the status
** @nam4rule Type Return the type
** @nam4rule Version Return the version
**
** @argrule * dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @valrule DbDisplayname [AjPStr] Database display name or NULL
** @valrule DbName [AjPStr] Database name or NULL
** @valrule DbRelease [AjPStr] Database release or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayidentifier [AjPStr] Display identifier or NULL
** @valrule Infotext [AjPStr] Information text or NULL
** @valrule Infotype [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration or
** ensEExternalreferenceInfotypeNULL
** @valrule Linkageannotation [AjPStr] Linkage annotation or NULL
** @valrule Objectidentifier [ajuint] Ensembl Object identifier or 0U
** @valrule Objecttype [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration or
** ensEExternalreferenceObjecttypeNULL
** @valrule Primaryidentifier [AjPStr] Primary identifier or NULL
** @valrule Priority [ajint] Priority
** @valrule Status [EnsEExternaldatabaseStatus] Status or
** ensEExternaldatabaseStatusNULL
** @valrule Type [EnsEExternaldatabaseType] Type or
** ensEExternaldatabaseTypeNULL
** @valrule Version [AjPStr] Version or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseentryGetDbDisplayname *************************************
**
** Get the database display name member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::db_display_name
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Database display name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDbDisplayname(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Externaldatabase &&
            dbe->Externalreference->Externaldatabase->Displayname) ?
        dbe->Externalreference->Externaldatabase->Displayname : NULL;
}




/* @func ensDatabaseentryGetDbName ********************************************
**
** Get the database name member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Database name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDbName(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Externaldatabase &&
            dbe->Externalreference->Externaldatabase->Name) ?
        dbe->Externalreference->Externaldatabase->Name : NULL;
}




/* @func ensDatabaseentryGetDbRelease *****************************************
**
** Get the database release member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Database release or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDbRelease(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Externaldatabase &&
            dbe->Externalreference->Externaldatabase->Release) ?
        dbe->Externalreference->Externaldatabase->Release : NULL;
}




/* @func ensDatabaseentryGetDescription ***************************************
**
** Get the description member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDescription(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Description) ?
        dbe->Externalreference->Description : NULL;
}




/* @func ensDatabaseentryGetDisplayidentifier *********************************
**
** Get the display identifier member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::display_id
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Display identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDisplayidentifier(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Displayidentifier) ?
        dbe->Externalreference->Displayidentifier : NULL;
}




/* @func ensDatabaseentryGetInfotext ******************************************
**
** Get the information text member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::info_text
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Information text or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetInfotext(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Infotext) ?
        dbe->Externalreference->Infotext : NULL;
}




/* @func ensDatabaseentryGetInfotype ******************************************
**
** Get the Ensembl External Refernce Information Type enumeration member of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::info_type
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration or
** ensEExternalreferenceInfotypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEExternalreferenceInfotype ensDatabaseentryGetInfotype(
    const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Infotype) ?
        dbe->Externalreference->Infotype : ensEExternalreferenceInfotypeNULL;
}




/* @func ensDatabaseentryGetLinkageannotation *********************************
**
** Get the linkage annotation member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::linkage_annotation
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Linkage annotation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetLinkageannotation(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Linkageannotation) ?
        dbe->Externalreference->Linkageannotation : NULL;
}




/* @func ensDatabaseentryGetObjectidentifier **********************************
**
** Get the Ensembl Object identifier member of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::ensembl_id
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [ajuint] Ensembl Object identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensDatabaseentryGetObjectidentifier(
    const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Objectidentifier) ?
        dbe->Externalreference->Objectidentifier : 0U;
}




/* @func ensDatabaseentryGetObjecttype ****************************************
**
** Get the Ensembl External Refernce Object Type enumeration member of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::ensembl_object_type
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration or
** ensEExternalreferenceObjecttypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEExternalreferenceObjecttype ensDatabaseentryGetObjecttype(
    const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Objecttype) ?
        dbe->Externalreference->Objecttype : ensEExternalreferenceObjecttypeNULL;
}




/* @func ensDatabaseentryGetPrimaryidentifier *********************************
**
** Get the primary identifier member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Primary identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetPrimaryidentifier(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Primaryidentifier) ?
        dbe->Externalreference->Primaryidentifier : NULL;
}




/* @func ensDatabaseentryGetPriority ******************************************
**
** Get the priority member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [ajint] Priortity or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensDatabaseentryGetPriority(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Externaldatabase &&
            dbe->Externalreference->Externaldatabase->Priority) ?
        dbe->Externalreference->Externaldatabase->Priority : 0;
}




/* @func ensDatabaseentryGetStatus ********************************************
**
** Get the status member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternaldatabaseStatus] Status or
**                                      ensEExternaldatabaseStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensDatabaseentryGetStatus(
    const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Externaldatabase &&
            dbe->Externalreference->Externaldatabase->Status) ?
        dbe->Externalreference->Externaldatabase->Status :
        ensEExternaldatabaseStatusNULL;
}




/* @func ensDatabaseentryGetType **********************************************
**
** Get the type member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::type
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternaldatabaseType] Type or ensEExternaldatabaseTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEExternaldatabaseType ensDatabaseentryGetType(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Externaldatabase &&
            dbe->Externalreference->Externaldatabase->Type) ?
        dbe->Externalreference->Externaldatabase->Type :
        ensEExternaldatabaseTypeNULL;
}




/* @func ensDatabaseentryGetVersion *******************************************
**
** Get the version member of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::version
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Version or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetVersion(const EnsPDatabaseentry dbe)
{
    return (dbe &&
            dbe->Externalreference &&
            dbe->Externalreference->Version) ?
        dbe->Externalreference->Version : NULL;
}




/* @section member addition **************************************************
**
** Functions for adding members to an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Add Add one object to an Ensembl Database Entry
** @nam4rule Ontologylinkage Add an Ensembl Ontology Linkage
**
** @argrule * dbe [EnsPDatabaseentry] Ensembl Database Entry object
** @argrule Ontologylinkage linkagetype [AjPStr] Linkage type
** @argrule Ontologylinkage source [EnsPDatabaseentry] Ensembl Database
** Entry
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDatabaseentryAddOntologylinkage ***********************************
**
** Add an Ensembl Ontology Linkage to an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::OntologyXref::add_linkage_type
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] linkagetype [AjPStr] Linkage type
** @param [u] source [EnsPDatabaseentry] Source Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryAddOntologylinkage(EnsPDatabaseentry dbe,
                                          AjPStr linkagetype,
                                          EnsPDatabaseentry source)
{
    EnsPOntologylinkage ol = NULL;

    if (!dbe)
        return ajFalse;

    ol = ensOntologylinkageNewIni(linkagetype, source);

    if (ol)
    {
        ajListPushAppend(dbe->Ontologylinkages, (void *) ol);

        return ajTrue;
    }

    return ajFalse;
}




/* @section clear *************************************************************
**
** Functions for clearing internals of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Clear Clear Ensembl Database Entry values
** @nam4rule Ontologylinkages Clear Ensembl Ontology Linkage objects
**
** @argrule * dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseentryClearOntologylinkages ********************************
**
** Clear all Ensembl Ontology Linkage objects of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::OntologyXref::flush_linkage_types
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryClearOntologylinkages(EnsPDatabaseentry dbe)
{
    EnsPOntologylinkage ol = NULL;

    if (!dbe)
        return ajFalse;

    while (ajListPop(dbe->Ontologylinkages, (void **) &ol))
        ensOntologylinkageDel(&ol);

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
**
** @nam3rule Fetch Fetch Ensembl Database Entry values
** @nam4rule All Fetch all objects
** @nam5rule Dependents   Fetch all Ensembl Database Entry objects,
**                        which depend on an Ensembl Database Entry
** @nam5rule Linkagetypes Fetch all Ontology Linkage types
** @nam5rule Masters      Fetch all Ensembl Database Entry objects,
**                        which are masters of an Ensembl Database Entry
** @nam6rule By           Fetch all by a criterion
** @nam7rule Gene         Fetch all by an Ensembl Gene
** @nam7rule Transcript   Fetch all by an Ensembl Transcript
** @nam7rule Translation  Fetch all by an Ensembl Translation
**
** @argrule AllDependents dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule AllLinkagetypes dbe [const EnsPDatabaseentry]
** Ensembl Database Entry
** @argrule AllMasters dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule ByGene gene [const EnsPGene]
** Ensembl Gene
** @argrule ByTranscript transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule ByTranslation translation [const EnsPTranslation]
** Ensembl Translation
** @argrule AllDependents dbes [AjPList] AJAX List of
** Ensembl Database Entry objects
** @argrule AllLinkagetypes types [AjPList] AJAX List of AJAX String objects
**                          (linkage types i.e. Ontology Evidence Codes)
** @argrule AllMasters dbes [AjPList] AJAX List of
** Ensembl Database Entry objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseentryFetchAllDependents ***********************************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_dependents
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllDependents(
    EnsPDatabaseentry dbe,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllDependents(dbe->Adaptor,
                                                     dbe,
                                                     dbes);
}




/* @func ensDatabaseentryFetchAllDependentsByGene *****************************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry and linked to an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_dependents
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllDependentsByGene(
    EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllDependentsByGene(
        dbe->Adaptor,
        dbe,
        gene,
        dbes);
}




/* @func ensDatabaseentryFetchAllDependentsByTranscript ***********************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry and linked to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_dependents
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllDependentsByTranscript(
    EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllDependentsByTranscript(
        dbe->Adaptor,
        dbe,
        transcript,
        dbes);
}




/* @func ensDatabaseentryFetchAllDependentsByTranslation **********************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry and linked to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_dependents
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllDependentsByTranslation(
    EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllDependentsByTranslation(
        dbe->Adaptor,
        dbe,
        translation,
        dbes);
}




/* @func ensDatabaseentryFetchAllLinkagetypes *********************************
**
** Fetch all Ensembl Ontology Linkage objects of an Ensembl Database Entry.
**
** The caller is responsible for deleting the AJAX String linkage types
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::OntologyXref::get_all_linkage_types
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [u] types [AjPList] AJAX List of AJAX String linkage types
**                           (i.e. Ontology Evidence Codes)
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllLinkagetypes(const EnsPDatabaseentry dbe,
                                            AjPList types)
{
    AjBool match = AJFALSE;

    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    AjPStr type = NULL;

    EnsPOntologylinkage ol = NULL;

    if (!dbe)
        return ajFalse;

    if (!types)
        return ajFalse;

    iter1 = ajListIterNew(dbe->Ontologylinkages);

    iter2 = ajListIterNew(types);

    while (!ajListIterDone(iter1))
    {
        ol = (EnsPOntologylinkage) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        match = ajFalse;

        while (!ajListIterDone(iter2))
        {
            type = (AjPStr) ajListIterGet(iter2);

            if (ajStrMatchS(ol->LinkageType, type))
            {
                match = ajTrue;

                break;
            }
        }

        if (!match)
            ajListPushAppend(types, (void *) ajStrNewRef(type));
    }

    ajListIterDel(&iter1);
    ajListIterDel(&iter2);

    return ajTrue;
}




/* @func ensDatabaseentryFetchAllMasters **************************************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_masters
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllMasters(
    EnsPDatabaseentry dbe,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllMasters(
        dbe->Adaptor,
        dbe,
        dbes);
}




/* @func ensDatabaseentryFetchAllMastersByGene ********************************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry and linked to an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_masters
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllMastersByGene(
    EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllMastersByGene(
        dbe->Adaptor,
        dbe,
        gene,
        dbes);
}




/* @func ensDatabaseentryFetchAllMastersByTranscript **************************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry and linked to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_masters
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllMastersByTranscript(
    EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllMastersByTranscript(
        dbe->Adaptor,
        dbe,
        transcript,
        dbes);
}




/* @func ensDatabaseentryFetchAllMastersByTranslation *************************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry and linked to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_masters
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryFetchAllMastersByTranslation(
    EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes)
{
    if (!dbe)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    return ensDatabaseentryadaptorFetchAllMastersByTranslation(
        dbe->Adaptor,
        dbe,
        translation,
        dbes);
}




/* @datasection [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor *****
**
** @nam2rule Databaseentryadaptor Functions for manipulating
** Ensembl Database Entry Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor
** @cc CVS Revision: 1.165
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic databaseentryadaptorFetchAllbyStatement ************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Database Entry objects.
**
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool databaseentryadaptorFetchAllbyStatement(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr statement,
    AjPList dbes)
{
    double score  = 0.0;
    double evalue = 0.0;

    ajint ierqryidt = 0;
    ajint iertrgidt = 0;
    ajint ierqrysrt = 0;
    ajint ierqryend = 0;
    ajint iertrgsrt = 0;
    ajint iertrgend = 0;

    ajuint xrefid     = 0U;
    ajuint edbid      = 0U;
    ajuint objectid   = 0U;
    ajuint objxrfid   = 0U;
    ajuint analysisid = 0U;
    ajuint sourceid   = 0U;

    ajuint *Pidentifier = NULL;

    AjBool debug = AJFALSE;
    AjBool ident = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr primaryid         = NULL;
    AjPStr displayid         = NULL;
    AjPStr version           = NULL;
    AjPStr description       = NULL;
    AjPStr linkageannotation = NULL;
    AjPStr infotext          = NULL;
    AjPStr infotype          = NULL;
    AjPStr objecttype        = NULL;

    AjPStr synonym = NULL;

    AjPStr cigar    = NULL;
    AjPStr olinkage = NULL;

    AjPTable dbetable = NULL;
    AjPTable linkages = NULL;
    AjPTable synonyms = NULL;

    EnsEExternalreferenceInfotype   erit = ensEExternalreferenceInfotypeNULL;
    EnsEExternalreferenceObjecttype erot = ensEExternalreferenceObjecttypeNULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseentry dbe       = NULL;
    EnsPDatabaseentry sourcedbe = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    debug = ajDebugTest("databaseentryadaptorFetchAllbyStatement");

    if (debug)
        ajDebug("databaseentryadaptorFetchAllbyStatement\n"
                "  dbea %p\n"
                "  statement '%S'\n"
                "  dbes %p\n",
                dbea,
                statement,
                dbes);

    if (!dbea)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dbea->Adaptor);

    edba = ensRegistryGetExternaldatabaseadaptor(dbea->Adaptor);

    dbetable = ajTableuintNew(0U);

    ajTableSetDestroyvalue(
        dbetable,
        (void (*)(void **)) &ensDatabaseentryDel);

    linkages = ensTableuintliststrNewLen(0U);
    synonyms = ensTableuintliststrNewLen(0U);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    if (ajSqlrowGetColumns(sqlr) >= 10U)
        ident = ajTrue;
    else
        ident = ajFalse;

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        /* "xref" SQL table */
        xrefid      = 0U;
        edbid       = 0U;
        primaryid   = ajStrNew();
        displayid   = ajStrNew();
        version     = ajStrNew();
        description = ajStrNew();
        infotype    = ajStrNew();
        erit        = ensEExternalreferenceInfotypeNULL;
        infotext    = ajStrNew();
        /* "external_synonym" SQL table */
        synonym     = ajStrNew();

        if (ident)
        {
            /* "object_xref" SQL table */
            objxrfid   = 0U;
            objectid   = 0U;
            objecttype = ajStrNew();
            erot       = ensEExternalreferenceObjecttypeNULL;
            linkageannotation = ajStrNew();
            analysisid = 0U;
            /* "identity_xref" SQL table */
            ierqryidt  = 0;
            iertrgidt  = 0;
            ierqrysrt  = 0;
            ierqryend  = 0;
            iertrgsrt  = 0;
            iertrgend  = 0;
            cigar      = ajStrNew();
            score      = 0.0;
            evalue     = 0.0;
            /* "ontology_xref" SQL table */
            olinkage   = ajStrNew();
            sourceid   = 0U;
        }

        sqlr = ajSqlrowiterGet(sqli);

        /* "xref" SQL table */
        ajSqlcolumnToUint(sqlr, &xrefid);
        ajSqlcolumnToUint(sqlr, &edbid);
        ajSqlcolumnToStr(sqlr, &primaryid);
        ajSqlcolumnToStr(sqlr, &displayid);
        ajSqlcolumnToStr(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &infotype);
        ajSqlcolumnToStr(sqlr, &infotext);
        /* "external_synonym SQL table */
        ajSqlcolumnToStr(sqlr, &synonym);

        if (ident)
        {
            /* "object_xref" SQL table */
            ajSqlcolumnToUint(sqlr, &objxrfid);
            ajSqlcolumnToUint(sqlr, &objectid);
            ajSqlcolumnToStr(sqlr, &objecttype);
            ajSqlcolumnToStr(sqlr, &linkageannotation);
            ajSqlcolumnToUint(sqlr, &analysisid);
            /* "identity_xref" SQL table */
            ajSqlcolumnToInt(sqlr, &ierqryidt);
            ajSqlcolumnToInt(sqlr, &iertrgidt);
            ajSqlcolumnToInt(sqlr, &ierqrysrt);
            ajSqlcolumnToInt(sqlr, &ierqryend);
            ajSqlcolumnToInt(sqlr, &iertrgsrt);
            ajSqlcolumnToInt(sqlr, &iertrgend);
            ajSqlcolumnToStr(sqlr, &cigar);
            ajSqlcolumnToDouble(sqlr, &score);
            ajSqlcolumnToDouble(sqlr, &evalue);
            /* "ontology_xref" SQL table */
            ajSqlcolumnToStr(sqlr, &olinkage);
            ajSqlcolumnToUint(sqlr, &sourceid);
        }

        dbe = (EnsPDatabaseentry) ajTableFetchmodV(dbetable,
                                                   (const void *) &xrefid);

        if (debug)
        {
            ajDebug("databaseentryadaptorFetchAllbyStatement "
                    "xrefid %u dbe %p\n",
                    xrefid, dbe);

            ensDatabaseentryTrace(dbe, 1U);
        }

        if (!dbe)
        {
            erit = ensExternalreferenceInfotypeFromStr(infotype);

#if AJFALSE
            /* The info_type field can be NULL. */
            if (!erit)
                ajFatal("databaseentryadaptorFetchAllbyStatement "
                        "encountered unexpected string '%S' in the "
                        "'xref.infotype' field.\n",
                        infotype);
#endif /* AJFALSE */

            if (ident)
            {
                erot = ensExternalreferenceObjecttypeFromStr(objecttype);

                if (!erot)
                    ajFatal("databaseentryadaptorFetchAllbyStatement "
                            "encountered unexpected string '%S' in the "
                            "'object_xref.ensembl_object_type' field.\n",
                            objecttype);
            }

            ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

            ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

            dbe = ensDatabaseentryNewIni(dbea,
                                         xrefid,
                                         analysis,
                                         edb,
                                         primaryid,
                                         displayid,
                                         version,
                                         description,
                                         linkageannotation,
                                         infotext,
                                         erit,
                                         erot,
                                         objectid);

            ajListPushAppend(dbes, (void *) dbe);

            ensAnalysisDel(&analysis);

            ensExternaldatabaseDel(&edb);

            AJNEW0(Pidentifier);

            *Pidentifier = xrefid;

            ajTablePut(dbetable,
                       (void *) Pidentifier,
                       (void *) ensDatabaseentryNewRef(dbe));

            /* An external reference with a similarity score. */

            if (ierqryidt)
            {
                dbe->Identityreference =
                    ensIdentityreferenceNewIni(cigar,
                                               ierqrysrt,
                                               ierqryend,
                                               ierqryidt,
                                               iertrgsrt,
                                               iertrgend,
                                               iertrgidt,
                                               score,
                                               evalue);
            }
        }

        if (debug)
        {
            ajDebug("databaseentryadaptorFetchAllbyStatement "
                    "xrefid %u dbe %p\n",
                    xrefid, dbe);

            ensDatabaseentryTrace(dbe, 1U);
        }

        if (ajStrGetLen(synonym) &&
            (!ensTableuintliststrRegister(synonyms, xrefid, synonym)))
            ajListstrPushAppend(dbe->Synonyms, ajStrNewRef(synonym));

        if (ident && ajStrGetLen(olinkage) &&
            (!ensTableuintliststrRegister(linkages, xrefid, olinkage)))
        {
            ensDatabaseentryadaptorFetchByIdentifier(dbea,
                                                     sourceid,
                                                     &sourcedbe);

            ensDatabaseentryAddOntologylinkage(dbe, olinkage, sourcedbe);

            ensDatabaseentryDel(&sourcedbe);
        }

        ajStrDel(&primaryid);
        ajStrDel(&displayid);
        ajStrDel(&version);
        ajStrDel(&description);
        ajStrDel(&infotype);
        ajStrDel(&infotext);
        ajStrDel(&synonym);

        if (ident)
        {
            ajStrDel(&linkageannotation);
            ajStrDel(&objecttype);
            ajStrDel(&cigar);
            ajStrDel(&olinkage);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dbea->Adaptor, &sqls);

    ajTableDel(&dbetable);

    ensTableuintliststrDelete(&linkages);
    ensTableuintliststrDelete(&synonyms);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Database Entry Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Database Entry Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDatabaseentryadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseentryadaptorNew *******************************************
**
** Default constructor for an Ensembl Database Entry Adaptor.
**
** Ensembl Object Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Object Adaptor connected to a particular database is
** sufficient to instantiate any number of Ensembl Objects from the database.
** Each Ensembl Object will have a weak reference to the Object Adaptor that
** instantiated it. Therefore, Ensembl Object Adaptors should not be
** instantiated directly, but rather obtained from the Ensembl Registry,
** which will in turn call this function if neccessary.
**
** @see ensRegistryGetDatabaseadaptor
** @see ensRegistryGetDatabaseentryadaptor
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseentryadaptor ensDatabaseentryadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseentryadaptor dbea = NULL;

    if (!dba)
        return NULL;

    AJNEW0(dbea);

    dbea->Adaptor = dba;

    return dbea;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Database Entry Adaptor object.
**
** @fdata [EnsPDatabaseentryadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Database Entry Adaptor
**
** @argrule * Pdbea [EnsPDatabaseentryadaptor*]
** Ensembl Database Entry Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseentryadaptorDel *******************************************
**
** Default destructor for an Ensembl Database Entry Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pdbea [EnsPDatabaseentryadaptor*]
** Ensembl Database Entry Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensDatabaseentryadaptorDel(EnsPDatabaseentryadaptor *Pdbea)
{
    EnsPDatabaseentryadaptor pthis = NULL;

    if (!Pdbea)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensDatabaseentryadaptorDel"))
        ajDebug("ensDatabaseentryadaptorDel\n"
                "  *Pdbea %p\n",
                *Pdbea);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pdbea)
        return;

    pthis = *Pdbea;

    AJFREE(pthis);

    *Pdbea = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Database Entry Adaptor
** object.
**
** @fdata [EnsPDatabaseentryadaptor]
**
** @nam3rule Get Return Ensembl Database Entry Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * dbea [const EnsPDatabaseentryadaptor]
** Ensembl Database Entry Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseentryadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Database Entry Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::db
** @param [r] dbea [const EnsPDatabaseentryadaptor] Ensembl Database
**                                                  Entry Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseentryadaptorGetDatabaseadaptor(
    const EnsPDatabaseentryadaptor dbea)
{
    if (!dbea)
        return NULL;

    return dbea->Adaptor;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Database Entry objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDatabaseentryadaptor]
**
** @nam3rule Fetch       Fetch Ensembl Database Entry object(s)
** @nam4rule All         Fetch all Ensembl Database Entry objects
** @nam5rule Dependents  Fetch all Ensembl Database Entry objects,
**                       which depend on an Ensembl Database Entry
** @nam5rule Masters     Fetch all Ensembl Database Entry objects,
**                       which are masters of an Ensembl Database Entry
** @nam6rule By          Fetch all by a criterion
** @nam7rule Gene        Fetch all by an Ensembl Gene
** @nam7rule Transcript  Fetch all by an Ensembl Transcript
** @nam7rule Translation Fetch all by an Ensembl Translation
** @nam4rule Allby       Fetch all Ensembl Database Entry objects
**                       matching a criterion
** @nam5rule Description Featch all by a description
** @nam5rule Gene        Fetch all by an Ensembl Gene
** @nam5rule Name        Fetch all by a name
** @nam5rule Object      Fetch all by an Ensembl Object
** @nam5rule Source      Fetch all by an Ensembl External Database name
** @nam5rule Transcript  Fetch all by an Ensembl Transcript
** @nam5rule Translation Fetch all by an Ensembl Translation
** @nam4rule By          Fetch one Ensembl Database Entry object
**                       matching a criterion
** @nam5rule Accession   Fetch by an accession
** @nam5rule Identifier  Fetch by an SQL database-internal identifier
**
** @argrule * dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @argrule Dependents dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @argrule Masters dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @argrule ByGene gene [const EnsPGene]
** Ensembl Gene
** @argrule ByTranscript transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule ByTranslation translation [const EnsPTranslation]
** Ensembl Translation
** @argrule All dbes [AjPList] AJAX List of Ensembl Database Entry objects
** @argrule AllbyDescription description [const AjPStr] Description
** @argrule AllbyDescription dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyGene gene [const EnsPGene]
** Ensembl Gene
** @argrule AllbyGene dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyGene dbtype [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllbyName name [const AjPStr]
** Name
** @argrule AllbyName dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyObject objectidentifier [ajuint]
** Ensembl Object identifier
** @argrule AllbyObject objecttype [const AjPStr]
** Ensembl Object type
** @argrule AllbyObject dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyObject dbtype [EnsEExternaldatabaseType]
** Ensembl External Database Type
** @argrule AllbySource source [const AjPStr] Source
** @argrule AllbyTranscript transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule AllbyTranscript dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyTranscript dbtype [EnsEExternaldatabaseType]
** Ensembl External Database Type
** @argrule AllbyTranslation translation [const EnsPTranslation]
** Ensembl Translation
** @argrule AllbyTranslation dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyTranslation dbtype [EnsEExternaldatabaseType]
** Ensembl External Database Type
** @argrule Allby dbes [AjPList] AJAX List of Ensembl Database Entry objects
** @argrule ByAccession dbname [const AjPStr] Ensembl Exernal Database name
** @argrule ByAccession accession [const AjPStr] Ensembl External Reference
** primary identifier
** @argrule ByAccession Pdbe [EnsPDatabaseentry*] Ensembl Database Entry
** address
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pdbe [EnsPDatabaseentry*] Ensembl Database Entry
** address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic databaseentryadaptorFetchAllDependenciesByObject ***************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_get_all_dm
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_get_all_dm_sth
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_get_all_dm_loc_sth
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] constraint [const AjPStr] SQL constraint
** @param [r] objecttype [const AjPStr] Ensembl Object type
**                                      (Gene, Transcript or Translation)
** @param [r] objectidentifier [ajuint] Ensembl Object identifier
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool databaseentryadaptorFetchAllDependenciesByObject(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr constraint,
    const AjPStr objecttype,
    ajuint objectidentifier,
    AjPList dbes)
{
    char *txtobjecttype = NULL;

    AjBool result = AJFALSE;

    AjPStr statement = NULL;

    if (!dbea)
        return ajFalse;

    if ((constraint == NULL) || (ajStrGetLen(constraint) == 0))
        return ajFalse;

    if (!dbes)
        return ajFalse;

    if ((objecttype != NULL) && (ajStrGetLen(objecttype) > 0))
    {
        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtobjecttype, objecttype);

        statement = ajFmtStr(
            "SELECT "
            "xref.xref_id, "
            "xref.external_db_id, "
            "xref.dbprimary_acc, "
            "xref.display_label, "
            "xref.version, "
            "xref.description, "
            "xref.info_type, "
            "xref.info_text, "
            "external_synonym.synonym "
            "FROM "
            "(xref, dependent_xref, object_xref) "
            "LEFT JOIN "
            "external_synonym "
            "ON "
            "xref.xref_id = external_synonym.xref_id "
            "WHERE "
            "xref.xref_id = object_xref.xref_id "
            "AND "
            "object_xref.ensembl_object_type = '%S' "
            "AND "
            "object_xref.ensembl_id = %u "
            "AND ",
            txtobjecttype,
            objectidentifier);

        ajCharDel(&txtobjecttype);
    }
    else
        statement = ajStrNewC(
            "SELECT "
            "xref.xref_id, "
            "xref.external_db_id, "
            "xref.dbprimary_acc, "
            "xref.display_label, "
            "xref.version, "
            "xref.description, "
            "xref.info_type, "
            "xref.info_text, "
            "external_synonym.synonym "
            "FROM "
            "(xref, dependent_xref) "
            "LEFT JOIN "
            "external_synonym "
            "ON "
            "xref.xref_id = external_synonym.xref_id "
            "WHERE ");

    ajStrAppendS(&statement, constraint);

    result = databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllDependents ****************************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_dependents
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllDependents(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.dependent_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        (AjPStr) NULL,
        0,
        dbes);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllDependentsByGene **********************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry and linked to an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_dependents
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllDependentsByGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("gene");

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.dependent_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        objecttype,
        ensGeneGetIdentifier(gene),
        dbes);

    ajStrDel(&constraint);
    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllDependentsByTranscript ****************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry and linked to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_dependents
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllDependentsByTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("transcript");

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.dependent_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        objecttype,
        ensTranscriptGetIdentifier(transcript),
        dbes);

    ajStrDel(&constraint);
    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllDependentsByTranslation ***************
**
** Fetch all Ensembl Database Entry objects which are dependent on an
** Ensembl Database Entry and linked to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_dependents
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllDependentsByTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("translation");

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.dependent_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        objecttype,
        ensTranslationGetIdentifier(translation),
        dbes);

    ajStrDel(&constraint);
    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllMasters *******************************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_masters
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllMasters(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (dbes)
        return ajFalse;

    constraint = ajFmtStr("dependent_xref.dependent_xref_id = $%u "
                          "AND "
                          "dependent_xref.master_xref_id = xref.xref_id",
                          dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(dbea,
                                                              constraint,
                                                              (AjPStr) NULL,
                                                              0,
                                                              dbes);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllMastersByGene *************************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry and linked to an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_masters
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllMastersByGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("gene");

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.master_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        objecttype,
        ensGeneGetIdentifier(gene),
        dbes);

    ajStrDel(&constraint);
    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllMastersByTranscript *******************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry and linked to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_masters
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllMastersByTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("transcript");

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.master_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        objecttype,
        ensTranscriptGetIdentifier(transcript),
        dbes);

    ajStrDel(&constraint);
    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllMastersByTranslation ******************
**
** Fetch all Ensembl Database Entry objects which are masters of an
** Ensembl Database Entry and linked to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::get_all_masters
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllMastersByTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("translation");

    constraint = ajFmtStr(
        "dependent_xref.master_xref_id = %u "
        "AND "
        "dependent_xref.master_xref_id = xref.xref_id",
        dbe->Identifier);

    result = databaseentryadaptorFetchAllDependenciesByObject(
        dbea,
        constraint,
        objecttype,
        ensTranslationGetIdentifier(translation),
        dbes);

    ajStrDel(&constraint);
    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllbyDescription *************************
**
** Fetch all Ensembl Database Entry objects via a description.
**
** The caller is responsible for deleting the Ensembl Database Entry objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_description
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] description [const AjPStr] Description
** @param [rN] dbname [const AjPStr] External Database name
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbyDescription(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr description,
    const AjPStr dbname,
    AjPList dbes)
{
    char *txtdescription = NULL;
    char *txtdbname      = NULL;

    AjPStr statement = NULL;

    if (!dbea)
        return ajFalse;

    if (!(description && ajStrGetLen(description)))
        return ajFalse;

    if (!dbes)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdescription, description);

    statement = ajFmtStr(
        "SELECT "
        "xref.xref_id, "
        "xref.external_db_id, "
        "xref.dbprimary_acc, "
        "xref.display_label, "
        "xref.version, "
        "xref.description, "
        "xref.info_type, "
        "xref.info_text, "
        "external_synonym.synonym "
        "FROM "
        "(xref, external_db) "
        "LEFT JOIN "
        "external_synonym "
        "ON "
        "xref.xref_id = external_synonym.xref_id "
        "WHERE "
        "xref.external_db_id = external_db.external_db_id "
        "AND "
        "xref.description LIKE '%s'",
        txtdescription);

    ajCharDel(&txtdescription);

    if (dbname && ajStrGetLen(dbname))
    {
        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

        ajFmtPrintAppS(&statement, " AND exDB.db_name = '%s'", txtdbname);

        ajCharDel(&txtdbname);
    }

    databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllbyGene ********************************
**
** Fetch all Ensembl Database Entry objects by an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_Gene
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [uN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbyGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPGene gene,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("Gene");

    result = ensDatabaseentryadaptorFetchAllbyObject(
        dbea,
        ensGeneGetIdentifier(gene),
        objecttype,
        dbname,
        dbtype,
        dbes);

    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllbyName ********************************
**
** Fetch all Ensembl Database Entry objects by a name.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_name
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbyName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList dbes)
{
    char *txtdbname = NULL;
    char *txtname   = NULL;

    AjPStr statement = NULL;

    if (!dbea)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtname, name);

    statement = ajFmtStr(
        "SELECT "
        "xref.xref_id, "
        "xref.external_db_id, "
        "xref.dbprimary_acc, "
        "xref.display_label, "
        "xref.version, "
        "xref.description, "
        "xref.info_type, "
        "xref.info_text, "
        "external_synonym.synonym "
        "FROM "
        "(xref, external_db) "
        "LEFT JOIN "
        "external_synonym "
        "ON "
        "xref.xref_id = external_synonym.xref_id "
        "WHERE "
        "xref.external_db_id = external_db.external_db_id "
        "AND "
        "(xref.dbprimary_acc = '%s' or xref.display_label = '%s')",
        txtname,
        txtname);

    if (dbname && ajStrGetLen(dbname))
    {
        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

        if (ajStrFindAnyK(dbname, '%') > 0)
            ajFmtPrintAppS(&statement,
                           " AND external_db.db_name like '%s'",
                           txtdbname);
        else
            ajFmtPrintAppS(&statement,
                           " AND external_db.db_name = '%s'",
                           txtdbname);

        ajCharDel(&txtdbname);
    }

    databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    if (!ajListGetLength(dbes))
    {
        ajDebug("ensDatabaseentryadaptorFetchAllbyName did not get an "
                "Ensembl Database Entry for database name '%S' and "
                "name '%S'.\n", dbname, name);

        /*
        ** This is a minor hack that means that results still come back even
        ** when a mistake was made and no InterPro accessions were loaded into
        ** the xref table. This has happened in the past and had the result of
        ** breaking Ensembl DomainView.
        */

        if (ajStrMatchC(dbname, "interpro"))
        {
            statement = ajFmtStr(
                "SELECT "
                "'0', "                /* xref.xref_id */
                "'0', "                /* xref.external_db_id */
                "interpro.interpro_ac, " /* xref.dbprimary_acc */
                "interpro.id, "        /* xref.display_label */
                "NULL, "               /* xref.version */
                "NULL, "               /* xref.description */
                "NULL, "               /* xref.info_type */
                "NULL, "               /* xref.info_text */
                "NULL "                /* external_synonym.synonym */
                "FROM "
                "interpro "
                "WHERE "
                "interpro.interpro_ac = '%s'",
                txtname);

            databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

            ajStrDel(&statement);
        }
    }

    ajCharDel(&txtname);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllbyObject ******************************
**
** Fetch all Ensembl Database Entry objects by an Ensembl Object.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_fetch_by_object_type
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] objectidentifier [ajuint] Ensembl Object identifier
** @param [r] objecttype [const AjPStr] Ensembl Object type
** @param [rN] dbname [const AjPStr] Ensembl External Database name, which may
** be an SQL pattern containing "%" matching any number of characters
** @param [uN] dbtype [EnsEExternaldatabaseType] Ensembl External Database Type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbyObject(
    EnsPDatabaseentryadaptor dbea,
    ajuint objectidentifier,
    const AjPStr objecttype,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    char *txtobjecttype = NULL;
    char *txtdbname = NULL;

    AjPStr statement = NULL;

    if (!dbea)
        return ajFalse;

    if (!objectidentifier)
        return ajFalse;

    if (!objecttype)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtobjecttype, objecttype);

    statement = ajFmtStr(
        "SELECT "
        "xref.xref_id, "
        "xref.external_db_id, "
        "xref.dbprimary_acc, "
        "xref.display_label, "
        "xref.version, "
        "xref.description, "
        "xref.info_type, "
        "xref.info_text, "
        "external_synonym.synonym, "
        "object_xref.object_xref_id, "
        "object_xref.ensembl_id, "
        "object_xref.ensembl_object_type, "
        "object_xref.linkage_annotation, "
        "object_xref.analysis_id, "
        "identity_xref.xref_identity, "
        "identity_xref.ensembl_identity, "
        "identity_xref.xref_start, "
        "identity_xref.xref_end, "
        "identity_xref.ensembl_start, "
        "identity_xref.ensembl_end, "
        "identity_xref.cigar_line, "
        "identity_xref.score, "
        "identity_xref.evalue, "
        "ontology_xref.linkage_type, "
        "ontology_xref.source_xref_id "
        "FROM "
        "(xref, external_db, object_xref) "
        "LEFT JOIN "
        "external_synonym "
        "ON "
        "xref.xref_id = external_synonym.xref_id "
        "LEFT JOIN "
        "identity_xref "
        "ON "
        "object_xref.object_xref_id = "
        "identity_xref.object_xref_id "
        "LEFT JOIN "
        "ontology_xref "
        "ON "
        "object_xref.object_xref_id = "
        "ontology_xref.object_xref_id "
        "WHERE "
        "xref.external_db_id = external_db.external_db_id "
        "AND "
        "xref.xref_id = object_xref.xref_id "
        "AND "
        "object_xref.ensembl_id = %u "
        "AND "
        "object_xref.ensembl_object_type = '%s'",
        objectidentifier,
        txtobjecttype);

    ajCharDel(&txtobjecttype);

    if (dbname && ajStrGetLen(dbname))
    {
        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

        if (ajStrFindAnyK(dbname, '%') > 0)
            ajFmtPrintAppS(&statement,
                           " AND external_db.db_name like '%s'",
                           txtdbname);
        else
            ajFmtPrintAppS(&statement,
                           " AND external_db.db_name = '%s'",
                           txtdbname);

        ajCharDel(&txtdbname);
    }

    if (dbtype)
        ajFmtPrintAppS(&statement,
                       " AND external_db.type = '%s'",
                       ensExternaldatabaseTypeToChar(dbtype));

    databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllbySource ******************************
**
** Fetch all Ensembl Database Entry objects via an
** Ensembl External Database name.
**
** The caller is responsible for deleting the Ensembl Database Entry objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_source
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] source [const AjPStr] Source
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbySource(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr source,
    AjPList dbes)
{
    char *txtsource = NULL;

    AjPStr statement = NULL;

    if (!dbea)
        return ajFalse;

    if (!(source && ajStrGetLen(source)))
        return ajFalse;

    if (!dbes)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtsource, source);

    statement = ajFmtStr(
        "SELECT "
        "xref.xref_id, "
        "xref.external_db_id, "
        "xref.dbprimary_acc, "
        "xref.display_label, "
        "xref.version, "
        "xref.description, "
        "xref.info_type, "
        "xref.info_text, "
        "external_synonym.synonym "
        "FROM "
        "(xref, external_db) "
        "LEFT JOIN "
        "external_synonym "
        "ON "
        "xref.xref_id = external_synonym.xref_id "
        "WHERE "
        "xref.external_db_id = external_db.external_db_id "
        "AND "
        "external_db.db_name LIKE '%s'",
        txtsource);

    ajCharDel(&txtsource);

    databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllbyTranscript **************************
**
** Fetch all Ensembl Database Entry objects by an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_Transcript
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [uN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbyTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranscript transcript,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("Transcript");

    result = ensDatabaseentryadaptorFetchAllbyObject(
        dbea,
        ensTranscriptGetIdentifier(transcript),
        objecttype,
        dbname,
        dbtype,
        dbes);

    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchAllbyTranslation *************************
**
** Fetch all Ensembl Database Entry objects by an Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_Translation
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [uN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllbyTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranslation translation,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    AjBool result = AJFALSE;

    AjPStr objecttype = NULL;

    if (!dbea)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    objecttype = ajStrNewC("Translation");

    result = ensDatabaseentryadaptorFetchAllbyObject(
        dbea,
        ensTranslationGetIdentifier(translation),
        objecttype,
        dbname,
        dbtype,
        dbes);

    ajStrDel(&objecttype);

    return result;
}




/* @func ensDatabaseentryadaptorFetchByAccession ******************************
**
** Fetch an Ensembl Database Entry via its database name and accession number.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_by_db_accession
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbname [const AjPStr] Ensembl Exernal Database name
** @param [r] accession [const AjPStr] Ensembl External Reference primary
** identifier
** @param [wP] Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchByAccession(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    const AjPStr accession,
    EnsPDatabaseentry *Pdbe)
{
    char *txtdbname = NULL;
    char *txtaccession = NULL;

    AjPList dbes = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseentry dbe = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!accession)
        return ajFalse;

    if (!Pdbe)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtaccession, accession);

    statement = ajFmtStr(
        "SELECT "
        "xref.xref_id, "
        "xref.external_db_id, "
        "xref.dbprimary_acc, "
        "xref.display_label, "
        "xref.version, "
        "xref.description, "
        "xref.info_type, "
        "xref.info_text, "
        "external_synonym.synonym "
        "FROM (xref) "
        "LEFT JOIN "
        "external_synonym "
        "ON "
        "xref.xref_id = external_synonym.xref_id "
        "WHERE "
        "xref.dbprimary_acc = '%s' "
        "AND "
        "external_db.db_name = '%s'",
        txtaccession,
        txtdbname);

    dbes = ajListNew();

    databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    if (!ajListGetLength(dbes))
    {
        ajDebug("ensDatabaseentryadaptorFetchByAccession did not get an "
                "Ensembl Database Entry for database name '%S' and "
                "accession '%S'.\n", dbname, accession);

        /*
        ** This is a minor hack that means that results still come back even
        ** when a mistake was made and no InterPro accessions were loaded into
        ** the xref table. This has happened in the past and had the result of
        ** breaking Ensembl DomainView.
        */

        if (ajStrMatchC(dbname, "interpro"))
        {
            statement = ajFmtStr(
                "SELECT "
                "'0', "                /* xref.xref_id */
                "'0', "                /* xref.external_db_id */
                "interpro.interpro_ac, " /* xref.dbprimary_acc */
                "interpro.id, "        /* xref.display_label */
                "NULL, "               /* xref.version */
                "NULL, "               /* xref.description */
                "NULL, "               /* xref.info_type */
                "NULL, "               /* xref.info_text */
                "NULL "                /* external_synonym.synonym */
                "FROM "
                "interpro "
                "WHERE "
                "interpro.interpro_ac = '%s'",
                txtaccession);

            databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

            ajStrDel(&statement);
        }
    }

    if (ajListGetLength(dbes) > 1)
        ajDebug("ensDatabaseentryadaptorFetchByAccession got more than "
                "one Ensembl Database Entry for database name '%S' and "
                "accession '%S'.\n", dbname, accession);

    ajListPop(dbes, (void **) Pdbe);

    while (ajListPop(dbes, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&dbes);

    ajCharDel(&txtdbname);
    ajCharDel(&txtaccession);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchByIdentifier *****************************
**
** Fetch an Ensembl Database Entry via its SQL database-internal identifier.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_by_dbID
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchByIdentifier(
    EnsPDatabaseentryadaptor dbea,
    ajuint identifier,
    EnsPDatabaseentry *Pdbe)
{
    AjPList dbes = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseentry dbe = NULL;

    if (!dbea)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pdbe)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "xref.xref_id, "
        "xref.external_db_id, "
        "xref.dbprimary_acc, "
        "xref.display_label, "
        "xref.version, "
        "xref.description, "
        "xref.info_type, "
        "xref.info_text, "
        "external_synonym.synonym "
        "FROM "
        "(xref) "
        "LEFT JOIN "
        "external_synonym "
        "ON "
        "xref.xref_id = external_synonym.xref_id "
        "WHERE "
        "xref.xref_id = %d",
        identifier);

    dbes = ajListNew();

    databaseentryadaptorFetchAllbyStatement(dbea, statement, dbes);

    ajStrDel(&statement);

    if (!ajListGetLength(dbes))
        ajDebug("ensDatabaseentryadaptorFetchById did not get an "
                "Ensembl Database Entry for identifier %u.\n",
                identifier);

    if (ajListGetLength(dbes) > 1)
        ajDebug("ensDatabaseentryadaptorFetchById got more than one (%u) "
                "Ensembl Database Entry for identifier %u.\n",
                ajListGetLength(dbes), identifier);

    ajListPop(dbes, (void **) Pdbe);

    while (ajListPop(dbes, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&dbes);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Database Entry objects
** from an Ensembl SQL database.
**
** @fdata [EnsPDatabaseentryadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Database Entry-releated object(s)
** @nam4rule All Retrieve all Ensembl Databse Entry-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
** @nam5rule Geneidentifiers Fetch all Ensembl Gene identifiers
** @nam6rule By Fetch by a criterion
** @nam7rule Externalname Fetch by an external name
** @nam7rule Externaldatabasename Fetch by an Ensembl external Database name
** @nam5rule Transcriptidentifiers Fetch all Ensembl Transcript identifiers
** @nam6rule By Fetch by a criterion
** @nam7rule Externalname Fetch by an external name
** @nam7rule Externaldatabasename Fetch by an Ensembl external Database name
** @nam5rule Translationidentifiers Fetch all Ensembl Translation identifiers
** @nam6rule By Fetch by a criterion
** @nam7rule Externalname Fetch by an external name
** @nam7rule Externaldatabasename Fetch by an Ensembl external Database name
**
** @argrule * dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
** @argrule AllGeneidentifiersByExternaldatabasename dbname [const AjPStr]
** External Database name
** @argrule AllGeneidentifiersByExternalname name [const AjPStr]
** External name
** @argrule AllGeneidentifiersByExternalname dbname [const AjPStr]
** External Database name
** @argrule AllTranscriptidentifiersByExternaldatabasename dbname
** [const AjPStr] External Database name
** @argrule AllTranscriptidentifiersByExternalname name [const AjPStr]
** External name
** @argrule AllTranscriptidentifiersByExternalname dbname [const AjPStr]
** External Database name
** @argrule AllTranslationidentifiersByExternaldatabasename dbname
** [const AjPStr] External Database name
** @argrule AllTranslationidentifiersByExternalname name [const AjPStr]
** External name
** @argrule AllTranslationidentifiersByExternalname dbname [const AjPStr]
** External Database name
** @argrule * idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic databaseentryadaptorCompareIdentifier **************************
**
** Comparison function to sort unsigned integer (SQL) identifiers in
** ascending order.
**
** @param [r] item1 [const void*] Unsigned integer address 1
** @param [r] item2 [const void*] Unsigned integer address 2
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.3.0
** @@
******************************************************************************/

static int databaseentryadaptorCompareIdentifier(
    const void *item1,
    const void *item2)
{
    int result = 0;

    ajuint *Pidentifier1 = *(ajuint *const *) item1;
    ajuint *Pidentifier2 = *(ajuint *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("databaseentryadaptorCompareIdentifier"))
        ajDebug("databaseentryadaptorCompareIdentifier\n"
                "  identifier1 %u\n"
                "  identifier2 %u\n",
                *Pidentifier1,
                *Pidentifier2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (Pidentifier1 && (!Pidentifier2))
        return -1;

    if ((!Pidentifier1) && (!Pidentifier2))
        return 0;

    if ((!Pidentifier1) && Pidentifier2)
        return +1;

    /* Evaluate identifiers */

    if (*Pidentifier1 < *Pidentifier2)
        result = -1;

    if (*Pidentifier1 > *Pidentifier2)
        result = +1;

    return result;
}




/* @funcstatic databaseentryadaptorDeleteIdentifier ***************************
**
** ajListSortUnique "itemdel" function to delete unsigned integer SQL
** identifiers that are redundant.
**
** @param [r] Pitem [void**] AJAX unsigned integer pointer address
** @param [r] cl [void*] Standard. Passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void databaseentryadaptorDeleteIdentifier(void **Pitem, void *cl)
{
    if (!Pitem)
        return;

    (void) cl;

    ajMemFree(Pitem);

    return;
}




/* @funcstatic databaseentryadaptorRetrieveAllIdentifiersByExternalname *******
**
** Fetch SQL database-internal Ensembl identifiers via an external name.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_type_by_external_id
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] name [const AjPStr] External name
** @param [r] ensembltype [const AjPStr] Ensembl Object type
** @param [rN] extratype [const AjPStr] Additional Ensembl Object type
** @param [rN] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool databaseentryadaptorRetrieveAllIdentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr ensembltype,
    const AjPStr extratype,
    const AjPStr dbname,
    AjPList idlist)
{
    char *txtname   = NULL;
    char *txtdbname = NULL;

    ajuint *Pidentifier = NULL;

    ajlong strpos = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr operator  = NULL;
    AjPStr statement = NULL;
    AjPStr sqlfrom   = NULL;
    AjPStr sqlwhere  = NULL;
    AjPStr sqlselect = NULL;
    AjPStr temporary = NULL;

    if (!dbea)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!ensembltype)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    /*
    ** Accept SQL wildcard characters from the second position only to
    ** avoid too generic queries, which use too many database resources.
    */

    strpos = ajStrFindAnyK(name, '%');

    if (strpos == -1)
    {
        operator = ajStrNewC("=");
    }
    else if (strpos <= 2)
    {
        ajWarn("databaseentryadaptorRetrieveAllIdentifiersByExternalname "
               "got a name '%S', which is too generic and will use "
               "SQL database resources.", name);

        return ajFalse;
    }
    else
    {
        operator = ajStrNewC("LIKE");
    }

    if (ajStrFindAnyK(name, '_') > 0)
        ajStrAssignC(&operator, "=");

    sqlselect = ajStrNewC("object_xref.ensembl_id");
    sqlfrom   = ajStrNew();
    sqlwhere  = ajStrNew();

    if (extratype && ajStrGetLen(extratype))
    {
        temporary = ajStrNewS(extratype);

        ajStrFmtLower(&temporary);

        if (ajStrMatchCaseC(temporary, "Translation"))
            ajStrAssignC(&sqlselect, "translation.translation_id");
        else
        {
            ajStrDel(&sqlselect);

            sqlselect = ajFmtStr("transcript.%S_id", temporary);
        }

        if (ajStrMatchCaseC(ensembltype, "Translation"))
        {
            ajStrAssignC(&sqlfrom, "transcript, translation");

            ajStrAssignC(&sqlwhere,
                         "transcript.is_current = 1 "
                         "AND "
                         "transcript.transcript_id = "
                         "translation.transcript_id "
                         "AND "
                         "translation.translation_id = "
                         "object_xref.ensembl_id");
        }
        else
        {
            ajStrAssignC(&sqlfrom, "transcript");

            ajStrDel(&sqlwhere);

            sqlwhere = ajFmtStr("transcript.is_current = 1 "
                                "AND "
                                "transcript.%S_id = object_xref.ensembl_id",
                                ensembltype);
        }

        ajStrDel(&temporary);
    }

    if (ajStrMatchC(ensembltype, "Gene"))
    {
        ajStrAssignC(&sqlfrom, "gene, seq_region, coord_system");

        ajFmtPrintAppS(&sqlwhere,
                       "gene.is_current = 1 "
                       "AND "
                       "gene.gene_id = object_xref.ensembl_id "
                       "AND "
                       "gene.seq_region_id = seq_region.seq_region_id "
                       "AND "
                       "seq_region.coord_system_id = "
                       "coord_system.coord_system_id "
                       "AND "
                       "coord_system.species_id = %u",
                       ensDatabaseadaptorGetIdentifier(dbea->Adaptor));
    }
    else if (ajStrMatchCaseC(ensembltype, "Transcript"))
    {
        ajStrAssignC(&sqlfrom, "transcript, seq_region, coord_system");

        ajFmtPrintAppS(&sqlwhere,
                       "transcript.is_current = 1 "
                       "AND "
                       "transcript.transcript_id = object_xref.ensembl_id "
                       "AND "
                       "transcript.seq_region_id = seq_region.seq_region_id "
                       "AND "
                       "seq_region.coord_system_id = "
                       "coord_system.coord_system_id "
                       "AND "
                       "coord_system.species_id = %u",
                       ensDatabaseadaptorGetIdentifier(dbea->Adaptor));
    }
    else if (ajStrMatchCaseC(ensembltype, "Translation"))
    {
        ajStrAssignC(&sqlfrom,
                     "transcript, translation, "
                     "seq_region, coord_system");

        ajFmtPrintAppS(&sqlwhere,
                       "transcript.is_current = 1 "
                       "AND "
                       "transcript.transcript_id = translation.transcript_id "
                       "AND "
                       "translation.translation_id = object_xref.ensembl_id "
                       "AND "
                       "transcript.seq_region_id = seq_region.seq_region_id "
                       "AND "
                       "seq_region.coord_system_id = "
                       "coord_system.coord_system_id "
                       "AND "
                       "coord_system.species_id = %u",
                       ensDatabaseadaptorGetIdentifier(dbea->Adaptor));
    }

    if (dbname && ajStrGetLen(dbname))
    {
        /*
        ** Involve the "external_db" SQL table to limit the hits to a
        ** particular external database.
        */

        ajStrAppendC(&sqlfrom, ", external_db");

        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

        ajFmtPrintAppS(&sqlwhere,
                       " AND "
                       "external_db.db_name LIKE '%s%%' "
                       "AND "
                       "external_db.external_db_id = xref.external_db_id",
                       txtdbname);

        ajCharDel(&txtdbname);
    }

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtname, name);

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "%S, "
        "object_xref, "
        "xref "
        "WHERE "
        "%S "
        "AND "
        "object_xref.xref_id = xref.xref_id "
        "AND "
        "object_xref.ensembl_object_type = '%S' "
        "AND "
        "(xref.dbprimary_acc %S '%s' OR xref.display_label %S '%s')",
        sqlselect,
        sqlfrom,
        sqlwhere,
        ensembltype,
        operator,
        txtname,
        operator,
        txtname);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(idlist, (void *) Pidentifier);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dbea->Adaptor, &sqls);

    ajStrDel(&statement);

    /*
    ** If an external database name has been passed in, the "xref" and the
    ** "object_xref" SQL tables need joining on "xref_id".
    */

    if (dbname && ajStrGetLen(dbname))
        statement = ajFmtStr(
            "SELECT "
            "%S "
            "FROM "
            "%S, "
            "external_synonym, "
            "object_xref, "
            "xref "
            "WHERE "
            "%S "
            "AND "
            "external_synonym.synonym %S '%s' "
            "AND "
            "external_synonym.xref_id = object_xref.xref_id "
            "AND "
            "object_xref.ensembl_object_type = '%%S' "
            "AND "
            "object_xref.xref_id = xref.xref_id ",
            sqlselect,
            sqlfrom,
            sqlwhere,
            operator,
            txtname,
            ensembltype);
    else
        statement = ajFmtStr(
            "SELECT "
            "%S "
            "FROM "
            "%S, "
            "external_synonym, "
            "object_xref "
            "WHERE "
            "%S "
            "AND "
            "external_synonym.synonym %S '%s' "
            "AND "
            "external_synonym.xref_id = object_xref.xref_id "
            "AND "
            "object_xref.ensembl_object_type = '%S'",
            sqlselect,
            sqlfrom,
            sqlwhere,
            operator,
            txtname,
            ensembltype);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(idlist, (void *) Pidentifier);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dbea->Adaptor, &sqls);

    ajStrDel(&statement);

    ajCharDel(&txtname);

    ajStrDel(&sqlselect);
    ajStrDel(&sqlfrom);
    ajStrDel(&sqlwhere);
    ajStrDel(&operator);

    ajListSortUnique(idlist,
                     &databaseentryadaptorCompareIdentifier,
                     &databaseentryadaptorDeleteIdentifier);

    return ajTrue;
}




/* @funcstatic databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename
**
** Fetch SQL database-internal Ensembl identifiers via an external database
** name.
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_type_by_external_db_id
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbname [const AjPStr] External Database name
** @param [r] ensembltype [const AjPStr] Ensembl Object type
** @param [rN] extratype [const AjPStr] Additional Ensembl Object type
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: This function requires an external database name and not an external
** database identifier as the Perl API implementation.
******************************************************************************/

static AjBool databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    const AjPStr ensembltype,
    const AjPStr extratype,
    AjPList idlist)
{
    char *txtdbname = NULL;

    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr sqlfrom   = NULL;
    AjPStr sqlwhere  = NULL;
    AjPStr sqlselect = NULL;
    AjPStr temporary = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!ensembltype)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    sqlselect = ajStrNewC("object_xref.ensembl_id");
    sqlfrom   = ajStrNew();
    sqlwhere  = ajStrNew();

    if (extratype && ajStrGetLen(extratype))
    {
        temporary = ajStrNewS(extratype);

        ajStrFmtLower(&temporary);

        if (ajStrMatchCaseC(temporary, "Translation"))
            ajStrAssignC(&sqlselect, "translation.translation_id");
        else
        {
            ajStrDel(&sqlselect);

            sqlselect = ajFmtStr("transcript.%S_id", temporary);
        }

        if (ajStrMatchCaseC(ensembltype, "Translation"))
        {
            ajStrAssignC(&sqlfrom, "transcript, translation");

            ajStrAssignC(&sqlwhere,
                         "transcript.is_current = 1 "
                         "AND "
                         "transcript.transcript_id = "
                         "translation.transcript_id "
                         "AND "
                         "translation.translation_id = "
                         "object_xref.ensembl_id");
        }
        else
        {
            ajStrAssignC(&sqlfrom,"transcript");

            ajStrDel(&sqlwhere);

            sqlwhere = ajFmtStr("transcript.is_current = 1 "
                                "AND "
                                "transcript.%S_id = object_xref.ensembl_id",
                                ensembltype);
        }

        ajStrDel(&temporary);
    }

    if (ajStrMatchC(ensembltype, "Gene"))
    {
        ajStrAssignC(&sqlfrom, "gene");

        ajStrAssignC(&sqlwhere,
                     "gene.is_current = 1 "
                     "AND "
                     "gene.gene_id = object_xref.ensembl_id");
    }
    else if (ajStrMatchCaseC(ensembltype, "Transcript"))
    {
        ajStrAssignC(&sqlfrom, "transcript");

        ajStrAssignC(&sqlwhere,
                     "transcript.is_current = 1 "
                     "AND "
                     "transcript.transcript_id = object_xref.ensembl_id");
    }
    else if (ajStrMatchCaseC(ensembltype, "Translation"))
    {
        ajStrAssignC(&sqlfrom, "transcript, translation");

        ajStrAssignC(&sqlwhere,
                     "transcript.is_current = 1 "
                     "AND "
                     "transcript.transcript_id = translation.transcript_id "
                     "AND "
                     "translation.translation_id = object_xref.ensembl_id");
    }

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "%S, "
        "object_xref, "
        "xref, "
        "external_db "
        "WHERE "
        "%S "
        "AND "
        "object_xref.xref_id = xref.xref_id "
        "AND "
        "object_xref.ensembl_object_type = '%S' "
        "AND "
        "xref.external_db_id = external_db.external_db_id "
        "AND "
        "external_db.db_name = '%s'",
        sqlselect,
        sqlfrom,
        sqlwhere,
        ensembltype,
        txtdbname);

    ajCharDel(&txtdbname);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(idlist, (void *) Pidentifier);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dbea->Adaptor, &sqls);

    ajStrDel(&statement);
    ajStrDel(&sqlselect);
    ajStrDel(&sqlfrom);
    ajStrDel(&sqlwhere);

    ajListSortUnique(idlist,
                     &databaseentryadaptorCompareIdentifier,
                     &databaseentryadaptorDeleteIdentifier);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorRetrieveAllGeneidentifiersByExternaldatabasename
**
** Fetch SQL database-internal Ensembl Gene identifiers via an
** External Database name.
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::list_gene_ids_by_external_db_id
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorRetrieveAllGeneidentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool result = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("gene");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
            dbea,
            dbname,
            ensembltype,
            extratype,
            idlist))
        result = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
            dbea,
            dbname,
            ensembltype,
            extratype,
            idlist))
        result = ajFalse;

    ajStrAssignC(&ensembltype, "Gene");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
            dbea,
            dbname,
            ensembltype,
            (AjPStr) NULL,
            idlist))
        result = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     &databaseentryadaptorCompareIdentifier,
                     &databaseentryadaptorDeleteIdentifier);

    return result;
}




/* @func ensDatabaseentryadaptorRetrieveAllGeneidentifiersByExternalname ******
**
** Fetch SQL database-internal Ensembl Gene identifiers via an
** external name.
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::list_gene_ids_by_extids
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] name [const AjPStr] External name
** @param [r] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorRetrieveAllGeneidentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool result = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if (!dbea)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("gene");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternalname(
            dbea,
            name,
            ensembltype,
            extratype,
            dbname,
            idlist))
        result = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternalname(
            dbea,
            name,
            ensembltype,
            extratype,
            dbname,
            idlist))
        result = ajFalse;

    ajStrAssignC(&ensembltype, "Gene");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternalname(
            dbea,
            name,
            ensembltype,
            (AjPStr) NULL,
            dbname,
            idlist))
        result = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     &databaseentryadaptorCompareIdentifier,
                     &databaseentryadaptorDeleteIdentifier);

    return result;
}




/* @func ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternaldatabasename
**
** Fetch SQL database-internal Ensembl Transcript identifiers via an
** External Database name.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::
**     list_transcript_ids_by_external_db_id
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool result = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("transcript");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
            dbea,
            dbname,
            ensembltype,
            extratype,
            idlist))
        result = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
            dbea,
            dbname,
            ensembltype,
            extratype,
            idlist))
        result = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     &databaseentryadaptorCompareIdentifier,
                     &databaseentryadaptorDeleteIdentifier);

    return result;
}




/* @func ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternalname*
**
** Fetch SQL database-internal Ensembl Transcript identifiers via an
** external name.
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::list_transcript_ids_by_extids
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] name [const AjPStr] External name
** @param [r] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool result = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if (!dbea)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("transcript");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternalname(
            dbea,
            name,
            ensembltype,
            extratype,
            dbname,
            idlist))
        result = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternalname(
            dbea,
            name,
            ensembltype,
            (AjPStr) NULL,
            dbname,
            idlist))
        result = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     &databaseentryadaptorCompareIdentifier,
                     &databaseentryadaptorDeleteIdentifier);

    return result;
}




/* @func ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternaldatabasename
**
** Fetch SQL database-internal Ensembl Translation identifiers via an
** External Database name.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::
**     list_translation_ids_by_external_db_id
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool result = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if (!dbea)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternaldatabasename(
            dbea,
            dbname,
            ensembltype,
            extratype,
            idlist))
        result = ajFalse;

    ajStrDel(&ensembltype);

    return result;
}




/* @func ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternalname
**
** Fetch SQL database-internal Ensembl Translation identifiers via an
** external name.
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::list_translation_ids_by_extids
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] name [const AjPStr] External name
** @param [r] dbname [const AjPStr] External Database name
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool result = AJTRUE;

    AjPStr ensembltype = NULL;

    if (!dbea)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    if (!databaseentryadaptorRetrieveAllIdentifiersByExternalname(
            dbea,
            name,
            ensembltype,
            (AjPStr) NULL,
            dbname,
            idlist))
        result = ajFalse;

    ajStrDel(&ensembltype);

    return result;
}

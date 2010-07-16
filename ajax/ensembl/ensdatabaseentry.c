/******************************************************************************
** @source Ensembl Database Entry functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.14 $
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

#include "ensdatabaseentry.h"
#include "ensgene.h"
#include "enstranscript.h"
#include "enstranslation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

static const char *externalreferenceInfoType[] =
{
    NULL,
    "PROJECTION",
    "MISC",
    "DEPENDENT",
    "DIRECT",
    "SEQUENCE_MATCH",
    "INFERRED_PAIR",
    "PROBE",
    "UNMAPPED",
    "COORDINATE_OVERLAP",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool databaseentryadaptorHasLinkage(AjPTable linkages,
                                             ajuint xrefid,
                                             AjPStr linkage);

static AjBool databaseentryadaptorHasSynonym(AjPTable synonyms,
                                             ajuint xrefid,
                                             AjPStr synonym);

static AjBool databaseentryadaptorCacheClear(AjPTable table);

static AjBool databaseentryadaptorTempClear(AjPTable table);

static AjBool databaseentryadaptorFetchAllBySQL(EnsPDatabaseentryadaptor dbea,
                                                const AjPStr statement,
                                                AjPList dbes);

static int databaseentryadaptorCompareIdentifier(const void *P1,
                                                 const void *P2);

static void databaseentryadaptorDeleteIdentifier(void **PP1, void *cl);

static AjBool databaseentryadaptorFetchAllIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr ensembltype,
    const AjPStr extratype,
    const AjPStr dbname,
    AjPList idlist);

static AjBool databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    const AjPStr ensembltype,
    const AjPStr extratype,
    AjPList idlist);




/* @filesection ensdatabaseentry **********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPExternalreference] External Reference ********************
**
** Functions for manipulating Ensembl External Reference objects
**
** @cc Bio::EnsEMBL::DBEntry CVS Revision: 1.43
**
** @nam2rule Externalreference
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPExternalreference] Ensembl External Reference
** @argrule Ref object [EnsPExternalreference] Ensembl External Reference
**
** @valrule * [EnsPExternalreference] Ensembl External Reference
**
** @fcategory new
******************************************************************************/




/* @func ensExternalreferenceNew **********************************************
**
** Default constructor for an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [r] analysis [EnsPAnalysis] Ensembl Analysis
** @cc Bio::EnsEMBL::DBEntry::new
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] primaryid [AjPStr] Primary identifier
** @param [u] displayid [AjPStr] Display identifier
** @param [u] version [AjPStr] Version
** @param [u] description [AjPStr] Description
** @param [u] linkageannotation [AjPStr] Linkage annotation
** @param [u] infotext [AjPStr] Information text
** @param [r] infotype [EnsEExternalreferenceInfoType] Information type
**
** @return [EnsPExternalreference] Ensembl External Reference or NULL
** @@
******************************************************************************/

EnsPExternalreference ensExternalreferenceNew(
    ajuint identifier,
    EnsPAnalysis analysis,
    EnsPExternaldatabase edb,
    AjPStr primaryid,
    AjPStr displayid,
    AjPStr version,
    AjPStr description,
    AjPStr linkageannotation,
    AjPStr infotext,
    EnsEExternalreferenceInfoType infotype)
{
    EnsPExternalreference er = NULL;

    if(ajDebugTest("ensExternalreferenceNew"))
    {
        ajDebug("ensExternalreferenceNew\n"
                "  identifier %u\n"
                "  analysis %p\n"
                "  edb %p\n"
                "  primaryid '%S'\n"
                "  displayid '%S'\n"
                "  version '%S'\n"
                "  description '%S'\n"
                "  linkageannotation '%S'\n"
                "  infotext '%S'\n"
                "  infotype %d\n",
                identifier,
                analysis,
                edb,
                primaryid,
                displayid,
                version,
                description,
                linkageannotation,
                infotext,
                infotype);

        ensAnalysisTrace(analysis, 1);

        ensExternaldatabaseTrace(edb, 1);
    }

    if(!edb)
        return NULL;

    if(!primaryid)
        return NULL;

    AJNEW0(er);

    er->Use = 1;

    er->Identifier = identifier;

    er->Analysis = ensAnalysisNewRef(analysis);

    er->Externaldatabase = ensExternaldatabaseNewRef(edb);

    if(primaryid)
        er->PrimaryIdentifier = ajStrNewRef(primaryid);

    if(displayid)
        er->DisplayIdentifier = ajStrNewRef(displayid);

    if(version)
        er->Version = ajStrNewRef(version);

    if(description)
        er->Description = ajStrNewRef(description);

    if(linkageannotation)
        er->LinkageAnnotation = ajStrNewRef(linkageannotation);

    if(infotext)
        er->InfoText = ajStrNewRef(infotext);

    er->InfoType = infotype;

    return er;
}




/* @func ensExternalreferenceNewObj *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternalreference] Ensembl External Reference or NULL
** @@
******************************************************************************/

EnsPExternalreference ensExternalreferenceNewObj(
    const EnsPExternalreference object)
{
    EnsPExternalreference er = NULL;

    if(!object)
        return NULL;

    AJNEW0(er);

    er->Use = 1;

    er->Identifier = object->Identifier;

    /*
      er->Adaptor = object->Adaptor;
    */

    er->Analysis = ensAnalysisNewRef(object->Analysis);

    er->Externaldatabase = ensExternaldatabaseNewRef(object->Externaldatabase);

    if(object->PrimaryIdentifier)
        er->PrimaryIdentifier = ajStrNewRef(object->PrimaryIdentifier);

    if(object->DisplayIdentifier)
        er->DisplayIdentifier = ajStrNewRef(object->DisplayIdentifier);

    if(object->Version)
        er->Version = ajStrNewRef(object->Version);

    if(object->Description)
        er->Description = ajStrNewRef(object->Description);

    if(object->LinkageAnnotation)
        er->LinkageAnnotation = ajStrNewRef(object->LinkageAnnotation);

    if(object->InfoText)
        er->InfoText = ajStrNewRef(object->InfoText);

    er->InfoType = object->InfoType;

    return er;
}




/* @func ensExternalreferenceNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] er [EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternalreference] Ensembl External Reference
** @@
******************************************************************************/

EnsPExternalreference ensExternalreferenceNewRef(
    EnsPExternalreference er)
{
    if(!er)
        return NULL;

    er->Use++;

    return er;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl External Reference.
**
** @fdata [EnsPExternalreference]
** @fnote None
**
** @nam3rule Del Destroy (free) an External Reference object
**
** @argrule * Per [EnsPExternalreference*] External Reference object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExternalreferenceDel **********************************************
**
** Default Ensembl External Reference destructor.
**
** @param [d] Per [EnsPExternalreference*] Ensembl External Reference address
**
** @return [void]
** @@
******************************************************************************/

void ensExternalreferenceDel(
    EnsPExternalreference *Per)
{
    EnsPExternalreference pthis = NULL;

    if(!Per)
        return;

    if(!*Per)
        return;

    pthis = *Per;

    pthis->Use--;

    if(pthis->Use)
    {
        *Per = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ensExternaldatabaseDel(&pthis->Externaldatabase);

    ajStrDel(&pthis->PrimaryIdentifier);
    ajStrDel(&pthis->DisplayIdentifier);
    ajStrDel(&pthis->Version);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->LinkageAnnotation);
    ajStrDel(&pthis->InfoText);

    AJFREE(pthis);

    *Per = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
** @fnote None
**
** @nam3rule Get Return External Reference attribute(s)
** @nam4rule GetAdaptor Return the Ensembl External Reference Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetAnalysis Return the Ensembl Analysis
** @nam4rule GetExternaldatabase Return the Ensembl External Database
** @nam4rule GetPrimaryIdentifier Return the primary identifier
** @nam4rule GetDisplayIdentifier Return the display identifier
** @nam4rule GetVersion Return the version
** @nam4rule GetDescription Return the description
** @nam4rule GetLinkageAnnotation Return the linkage annotation
** @nam4rule GetInfoText Return the information text
** @nam4rule GetInfoType Return the information type
**
** @argrule * er [const EnsPExternalreference] External Reference
**
** @valrule Adaptor [EnsPExternalreferenceadaptor] Ensembl External
**                                                 Reference Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis
** @valrule Externaldatabase [EnsPExternaldatabase] External Database
** @valrule PrimaryIdentifier [AjPStr] Primary identifier
** @valrule DisplayIdentifier [AjPStr] Display identifier
** @valrule Version [AjPStr] Version
** @valrule Description [AjPStr] Description
** @valrule LinkageAnnotation [AjPStr] Linkage annotation
** @valrule InfoText [AjPStr] Information text
** @valrule InfoType [EnsEExternalreferenceInfoType] Information type
**
** @fcategory use
******************************************************************************/




#if AJFALSE
/* @func ensExternalreferenceGetAdaptor ***************************************
**
** Get the Ensembl External Reference Adaptor element of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [const EnsPExternalreferenceadaptor] Ensembl External Reference
**                                              Adaptor
** @@
******************************************************************************/

const EnsPExternalreferenceadaptor ensExternalreferenceGetAdaptor(
    const EnsPExternalreference er)
{
    if(!er)
        return NULL;

    return er->Adaptor;
}

#endif




/* @func ensExternalreferenceGetIdentifier ************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensExternalreferenceGetIdentifier(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->Identifier;
}




/* @func ensExternalreferenceGetAnalysis **************************************
**
** Get the Ensembl Analysis element of an Ensembl External Reference.
**
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPAnalysis] Ensembl Analysis
** @@
******************************************************************************/

EnsPAnalysis ensExternalreferenceGetAnalysis(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->Analysis;
}




/* @func ensExternalreferenceGetExternaldatabase ******************************
**
** Get the Ensembl External Database element of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @cc Bio::EnsEMBL::DBEntry::release
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @cc Bio::EnsEMBL::DBEntry::description
** @cc Bio::EnsEMBL::DBEntry::primary_id_linkable
** @cc Bio::EnsEMBL::DBEntry::display_id_linkable
** @cc Bio::EnsEMBL::DBEntry::status
** @cc Bio::EnsEMBL::DBEntry::type
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsPExternaldatabase] Ensembl External Database
** @@
******************************************************************************/

EnsPExternaldatabase ensExternalreferenceGetExternaldatabase(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->Externaldatabase;
}




/* @func ensExternalreferenceGetPrimaryIdentifier *****************************
**
** Get the primary identifier element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Primary identifier
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetPrimaryIdentifier(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->PrimaryIdentifier;
}




/* @func ensExternalreferenceGetDisplayIdentifier *****************************
**
** Get the display identifier element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::display_id
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Display identifier
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetDisplayIdentifier(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->DisplayIdentifier;
}




/* @func ensExternalreferenceGetVersion ***************************************
**
** Get the version element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::version
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Version
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetVersion(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->Version;
}




/* @func ensExternalreferenceGetDescription ***********************************
**
** Get the description element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetDescription(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->Description;
}




/* @func ensExternalreferenceGetLinkageAnnotation *****************************
**
** Get the linkage annotation element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::linkage_annotation
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Linkage annotation
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetLinkageAnnotation(
    const EnsPExternalreference er)
{
    if(!er)
        return NULL;

    return er->LinkageAnnotation;
}




/* @func ensExternalreferenceGetInfoText **************************************
**
** Get the information text element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_text
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [AjPStr] Information text
** @@
******************************************************************************/

AjPStr ensExternalreferenceGetInfoText(
    const EnsPExternalreference er)
{
    if(!er)
        return 0;

    return er->InfoText;
}




/* @func ensExternalreferenceGetInfoType **************************************
**
** Get the information type element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_type
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [EnsEExternalreferenceInfoType] Information type
** @@
******************************************************************************/

EnsEExternalreferenceInfoType ensExternalreferenceGetInfoType(
    const EnsPExternalreference er)
{
    if(!er)
        return ensEExternalreferenceInfoTypeNULL;

    return er->InfoType;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl External Reference
** @nam4rule SetAdaptor Set the Ensembl External Database Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetAnalysis Set the Ensembl Analysis
** @nam4rule SetExternaldatabase Set the Ensembl External Database
** @nam4rule SetPrimaryIdentifier Set the primary identifier
** @nam4rule SetDisplayIdentifier Set the display identifier
** @nam4rule SetDescription Set the description
** @nam4rule SetVersion Set the version
** @nam4rule SetDescription Set the description
** @nam4rule SetLinkageAnnotation Set the linkage annotation
** @nam4rule SetInfoText Set the information text
** @nam4rule SetInfoType Set the information type
**
** @argrule * er [EnsPExternalreference] Ensembl External Reference object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




#if AJFALSE
/* @func ensExternalreferenceSetAdaptor ***************************************
**
** Set the Ensembl External Reference Adaptor element of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [r] adaptor [EnsPExternalreferenceadaptor] Ensembl External
**                                                   Reference Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetAdaptor(EnsPExternalreference er,
                                      EnsPExternalreferenceadaptor adaptor)
{
    if(!er)
        return ajFalse;

    er->Adaptor = adaptor;

    return ajTrue;
}

#endif




/* @func ensExternalreferenceSetIdentifier ************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl External Reference.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetIdentifier(EnsPExternalreference er,
                                         ajuint identifier)
{
    if(!er)
        return ajFalse;

    er->Identifier = identifier;

    return ajTrue;
}




/* @func ensExternalreferenceSetAnalysis **************************************
**
** Set the Ensembl Analysis element of an Ensembl External Reference.
**
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetAnalysis(EnsPExternalreference er,
                                       EnsPAnalysis analysis)
{
    if(!er)
        return ajFalse;

    ensAnalysisDel(&er->Analysis);

    er->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensExternalreferenceSetExternaldatabase ******************************
**
** Set the Ensembl External Database element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @cc Bio::EnsEMBL::DBEntry::release
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @cc Bio::EnsEMBL::DBEntry::description
** @cc Bio::EnsEMBL::DBEntry::primary_id_linkable
** @cc Bio::EnsEMBL::DBEntry::display_id_linkable
** @cc Bio::EnsEMBL::DBEntry::status
** @cc Bio::EnsEMBL::DBEntry::type
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetExternaldatabase(EnsPExternalreference er,
                                               EnsPExternaldatabase edb)
{
    if(!er)
        return ajFalse;

    ensExternaldatabaseDel(&er->Externaldatabase);

    er->Externaldatabase = ensExternaldatabaseNewRef(edb);

    return ajTrue;
}




/* @func ensExternalreferenceSetPrimaryIdentifier *****************************
**
** Set the primary identifier element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] primaryid [AjPStr] Primary identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetPrimaryIdentifier(EnsPExternalreference er,
                                                AjPStr primaryid)
{
    if(!er)
        return ajFalse;

    ajStrDel(&er->PrimaryIdentifier);

    er->PrimaryIdentifier = ajStrNewRef(primaryid);

    return ajTrue;
}




/* @func ensExternalreferenceSetDisplayIdentifier *****************************
**
** Set the display identifier element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::display_id
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] displayid [AjPStr] Display identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetDisplayIdentifier(EnsPExternalreference er,
                                                AjPStr displayid)
{
    if(!er)
        return ajFalse;

    ajStrDel(&er->DisplayIdentifier);

    er->DisplayIdentifier = ajStrNewRef(displayid);

    return ajTrue;
}




/* @func ensExternalreferenceSetVersion ***************************************
**
** Set the version element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::version
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] version [AjPStr] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetVersion(EnsPExternalreference er,
                                      AjPStr version)
{
    if(!er)
        return ajFalse;

    ajStrDel(&er->Version);

    er->Version = ajStrNewRef(version);

    return ajTrue;
}




/* @func ensExternalreferenceSetDescription ***********************************
**
** Set the description element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetDescription(EnsPExternalreference er,
                                          AjPStr description)
{
    if(!er)
        return ajFalse;

    ajStrDel(&er->Description);

    er->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensExternalreferenceSetLinkageAnnotation *****************************
**
** Set the linkage annotation element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::linkage_annotation
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] linkageannotation [AjPStr] Linkage annotation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetLinkageAnnotation(EnsPExternalreference er,
                                                AjPStr linkageannotation)
{
    if(!er)
        return ajFalse;

    ajStrDel(&er->LinkageAnnotation);

    er->LinkageAnnotation = ajStrNewRef(linkageannotation);

    return ajTrue;
}




/* @func ensExternalreferenceSetInfoText **************************************
**
** Set the information text element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_text
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [u] infotext [AjPStr] Information text
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetInfoText(EnsPExternalreference er,
                                       AjPStr infotext)
{
    if(!er)
        return ajFalse;

    ajStrDel(&er->InfoText);

    er->InfoText = ajStrNewRef(infotext);

    return ajTrue;
}




/* @func ensExternalreferenceSetInfoType **************************************
**
** Set the information type element of an Ensembl External Reference.
**
** @cc Bio::EnsEMBL::DBEntry::info_type
** @param [u] er [EnsPExternalreference] Ensembl External Reference
** @param [r] infotype [EnsEExternalreferenceInfoType] Information type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternalreferenceSetInfoType(EnsPExternalreference er,
                                       EnsEExternalreferenceInfoType infotype)
{
    if(!er)
        return ajFalse;

    er->InfoType = infotype;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl External Reference object.
**
** @fdata [EnsPExternalreference]
** @nam3rule Trace Report Ensembl External Reference elements to debug file
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
** @@
******************************************************************************/

AjBool ensExternalreferenceTrace(const EnsPExternalreference er, ajuint level)
{
    AjPStr indent = NULL;

    if(!er)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensExternalreferenceTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            /*
              "%S  Adaptor %p\n"
            */
            "%S  Analysis %p\n"
            "%S  Externaldatabase %p\n"
            "%S  PrimaryIdentifier '%S'\n"
            "%S  DisplayIdentifier '%S'\n"
            "%S  Version '%S'\n"
            "%S  Description '%S'\n"
            "%S  LinkageAnnotation '%S'\n"
            "%S  InfoText '%S'\n"
            "%S  InfoType %d\n",
            indent, er,
            indent, er->Use,
            indent, er->Identifier,
            /*
              indent, er->Adaptor,
            */
            indent, er->Analysis,
            indent, er->Externaldatabase,
            indent, er->PrimaryIdentifier,
            indent, er->DisplayIdentifier,
            indent, er->Version,
            indent, er->Description,
            indent, er->LinkageAnnotation,
            indent, er->InfoText,
            indent, er->InfoType);

    ensAnalysisTrace(er->Analysis, level + 1);

    ensExternaldatabaseTrace(er->Externaldatabase, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensExternalreferenceInfoTypeFromStr **********************************
**
** Convert an AJAX String into an Ensembl External Reference info type element.
**
** @param [r] infotype [const AjPStr] Info type string
**
** @return [EnsEExternalreferenceInfoType] Ensembl External Reference info type
**                                         or ensEExternalreferenceInfoTypeNULL
** @@
******************************************************************************/

EnsEExternalreferenceInfoType ensExternalreferenceInfoTypeFromStr(
    const AjPStr infotype)
{
    register EnsEExternalreferenceInfoType i =
        ensEExternalreferenceInfoTypeNULL;

    EnsEExternalreferenceInfoType einfotype =
        ensEExternalreferenceInfoTypeNULL;

    for(i = ensEExternalreferenceInfoTypeProjection;
        externalreferenceInfoType[i];
        i++)
        if(ajStrMatchC(infotype, externalreferenceInfoType[i]))
            einfotype = i;

    if(!einfotype)
        ajDebug("ensExternalreferenceInfoTypeFromStr encountered "
                "unexpected string '%S'.\n", infotype);

    return einfotype;
}




/* @func ensExternalreferenceInfoTypeToChar ***********************************
**
** Convert an Ensembl External Reference information type element into a
** C-type (char*) string.
**
** @param [r] type [EnsEExternalreferenceInfoType] External Reference
**                                                 information type
**
** @return [const char*] External Reference information type
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensExternalreferenceInfoTypeToChar(
    EnsEExternalreferenceInfoType type)
{
    register EnsEExternalreferenceInfoType i =
        ensEExternalreferenceInfoTypeNULL;

    if(!type)
        return NULL;

    for(i = ensEExternalreferenceInfoTypeProjection;
        externalreferenceInfoType[i] && (i < type);
        i++);

    if(!externalreferenceInfoType[i])
        ajDebug("ensExternalreferenceInfoTypeToChar encountered an "
                "out of boundary error on type %d.\n", type);

    return externalreferenceInfoType[i];
}




/* @func ensExternalreferenceGetMemsize ***************************************
**
** Get the memory size in bytes of an Ensembl External Reference.
**
** @param [r] er [const EnsPExternalreference] Ensembl External Reference
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensExternalreferenceGetMemsize(const EnsPExternalreference er)
{
    ajulong size = 0;

    if(!er)
        return 0;

    size += sizeof (EnsOExternalreference);

    size += ensAnalysisGetMemsize(er->Analysis);

    size += ensExternaldatabaseGetMemsize(er->Externaldatabase);

    if(er->PrimaryIdentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->PrimaryIdentifier);
    }

    if(er->DisplayIdentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->DisplayIdentifier);
    }

    if(er->Version)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Version);
    }

    if(er->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->Description);
    }

    if(er->LinkageAnnotation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->LinkageAnnotation);
    }

    if(er->InfoText)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(er->InfoText);
    }

    return size;
}




/* @datasection [EnsPIdentityreference] Identity Reference ********************
**
** Functions for manipulating Ensembl Identity Reference objects
**
** @cc Bio::EnsEMBL::IdentityXref CVS Revision: 1.16
**
** @nam2rule Identityreference
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPIdentityreference] Ensembl Identity Reference
** @argrule Ref object [EnsPIdentityreference] Ensembl Identity Reference
**
** @valrule * [EnsPIdentityreference] Ensembl Identity Reference
**
** @fcategory new
******************************************************************************/




/* @func ensIdentityreferenceNew **********************************************
**
** Default constructor for an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::new
** @param [r] cigar [AjPStr] Cigar line
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
** @@
******************************************************************************/

EnsPIdentityreference ensIdentityreferenceNew(AjPStr cigar,
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

    if(cigar)
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




/* @func ensIdentityreferenceNewObj *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [EnsPIdentityreference] Ensembl Identity Reference or NULL
** @@
******************************************************************************/

EnsPIdentityreference ensIdentityreferenceNewObj(
    const EnsPIdentityreference object)
{
    EnsPIdentityreference ir = NULL;

    if(!object)
        return NULL;

    AJNEW0(ir);

    if(object->Cigar)
        ir->Cigar = ajStrNewRef(object->Cigar);

    ir->QueryStart     = object->QueryStart;
    ir->QueryEnd       = object->QueryEnd;
    ir->QueryIdentity  = object->QueryIdentity;
    ir->TargetStart    = object->TargetStart;
    ir->TargetEnd      = object->TargetEnd;
    ir->TargetIdentity = object->TargetIdentity;
    ir->Use            = 1;
    ir->Evalue         = object->Evalue;
    ir->Score          = object->Score;

    return ir;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Identity Reference.
**
** @fdata [EnsPIdentityreference]
** @fnote None
**
** @nam3rule Del Destroy (free) an Identity Reference object
**
** @argrule * Pir [EnsPIdentityreference*] Identity Reference object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensIdentityreferenceDel **********************************************
**
** Default Ensembl Identity Reference destructor.
**
** @param [d] Pir [EnsPIdentityreference*] Ensembl Identity Reference address
**
** @return [void]
** @@
******************************************************************************/

void ensIdentityreferenceDel(EnsPIdentityreference *Pir)
{
    EnsPIdentityreference pthis = NULL;

    if(!Pir)
        return;

    if(!*Pir)
        return;

    pthis = *Pir;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pir = NULL;

        return;
    }

    ajStrDel(&pthis->Cigar);

    AJFREE(pthis);

    *Pir = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
** @fnote None
**
** @nam3rule Get Get attribute
** @nam4rule GetCigar Return the CIGAR line
** @nam4rule GetQueryStart Return the query start
** @nam4rule GetQueryEnd Return the query end
** @nam4rule GetQueryIdentity Return the query identity
** @nam4rule GetTargetStart Return the target start
** @nam4rule GetTargetEnd Return the target end
** @nam4rule GetTargetIdentity Return the target identity
** @nam4rule GetEvalue Return the e-value
** @nam4rule GetScore Return the score
**
** @argrule * ir [const EnsPIdentityreference] Identity Reference
**
** @valrule Cigar [AjPStr] Cigar line
** @valrule QueryStart [ajint] Query start
** @valrule QueryEnd [ajint] Query end
** @valrule QueryIdentity [ajint] Query identity
** @valrule TargetStart [ajint] Target start
** @valrule TargetEnd [ajint] Target end
** @valrule TargetIdentity [ajint] Target identity
** @valrule Evalue [double] E-value
** @valrule Score [double] Score
**
** @fcategory use
******************************************************************************/




/* @func ensIdentityreferenceGetCigar *****************************************
**
** Get the CIGAR line element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::cigar_line
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [AjPStr] CIGAR line
** @@
******************************************************************************/

AjPStr ensIdentityreferenceGetCigar(const EnsPIdentityreference ir)
{
    if(!ir)
        return NULL;

    return ir->Cigar;
}




/* @func ensIdentityreferenceGetQueryStart ************************************
**
** Get the query start element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_start
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Query start
** @@
******************************************************************************/

ajint ensIdentityreferenceGetQueryStart(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0;

    return ir->QueryStart;
}




/* @func ensIdentityreferenceGetQueryEnd **************************************
**
** Get the query end element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_end
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Query end
** @@
******************************************************************************/

ajint ensIdentityreferenceGetQueryEnd(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0;

    return ir->QueryEnd;
}




/* @func ensIdentityreferenceGetQueryIdentity *********************************
**
** Get the query identity element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_identity
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Query identity
** @@
******************************************************************************/

ajint ensIdentityreferenceGetQueryIdentity(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0;

    return ir->QueryIdentity;
}




/* @func ensIdentityreferenceGetTargetStart ***********************************
**
** Get the target start element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_start
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Target start
** @@
******************************************************************************/

ajint ensIdentityreferenceGetTargetStart(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0;

    return ir->TargetStart;
}




/* @func ensIdentityreferenceGetTargetEnd *************************************
**
** Get the target end element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_end
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Target end
** @@
******************************************************************************/

ajint ensIdentityreferenceGetTargetEnd(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0;

    return ir->TargetEnd;
}




/* @func ensIdentityreferenceGetTargetIdentity ********************************
**
** Get the target identity element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_identity
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajint] Target identity
** @@
******************************************************************************/

ajint ensIdentityreferenceGetTargetIdentity(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0;

    return ir->TargetIdentity;
}




/* @func ensIdentityreferenceGetEvalue ****************************************
**
** Get the e-value element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::evalue
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [double] E-value
** @@
******************************************************************************/

double ensIdentityreferenceGetEvalue(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0.0;

    return ir->Evalue;
}




/* @func ensIdentityreferenceGetScore *****************************************
**
** Get the score element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::score
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [double] Score
** @@
******************************************************************************/

double ensIdentityreferenceGetScore(const EnsPIdentityreference ir)
{
    if(!ir)
        return 0.0;

    return ir->Score;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Identity Reference
** @nam4rule SetCigar Set the CIGAR line
** @nam4rule SetQueryStart Set the query start
** @nam4rule SetQueryEnd Set the query end
** @nam4rule SetQueryIdentity Set the query identity
** @nam4rule SetTargetStart Set the target start
** @nam4rule SetTargetEnd Set the target end
** @nam4rule SetTargetIdentity Set the target identity
** @nam4rule SetEvalue Set the e-value
** @nam4rule SetScore Set the score
**
** @argrule * ir [EnsPIdentityreference] Ensembl Identity Reference object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensIdentityreferenceSetCigar *****************************************
**
** Set the CIGAR-line element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::cigar_line
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [u] cigar [AjPStr] CIGAR-line
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetCigar(EnsPIdentityreference ir,
                                    AjPStr cigar)
{
    if(!ir)
        return ajFalse;

    ajStrDel(&ir->Cigar);

    ir->Cigar = ajStrNewRef(cigar);

    return ajTrue;
}




/* @func ensIdentityreferenceSetQueryStart ************************************
**
** Set the query start element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_start
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] qstart [ajint] Query start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetQueryStart(EnsPIdentityreference ir,
                                         ajint qstart)
{
    if(!ir)
        return ajFalse;

    ir->QueryStart = qstart;

    return ajTrue;
}




/* @func ensIdentityreferenceSetQueryEnd **************************************
**
** Set the query end element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_end
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] qend [ajint] Query end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetQueryEnd(EnsPIdentityreference ir,
                                       ajint qend)
{
    if(!ir)
        return ajFalse;

    ir->QueryEnd = qend;

    return ajTrue;
}




/* @func ensIdentityreferenceSetQueryIdentity *********************************
**
** Set the query identity element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::xref_identity
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] qidentity [ajint] Query identity
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetQueryIdentity(EnsPIdentityreference ir,
                                            ajint qidentity)
{
    if(!ir)
        return ajFalse;

    ir->QueryIdentity = qidentity;

    return ajTrue;
}




/* @func ensIdentityreferenceSetTargetStart ***********************************
**
** Set the target start element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_start
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] tstart [ajint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetTargetStart(EnsPIdentityreference ir,
                                          ajint tstart)
{
    if(!ir)
        return ajFalse;

    ir->TargetStart = tstart;

    return ajTrue;
}




/* @func ensIdentityreferenceSetTargetEnd *************************************
**
** Set the target end element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_end
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] tend [ajint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetTargetEnd(EnsPIdentityreference ir,
                                        ajint tend)
{
    if(!ir)
        return ajFalse;

    ir->TargetEnd = tend;

    return ajTrue;
}




/* @func ensIdentityreferenceSetTargetIdentity ********************************
**
** Set the target identity element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::ensembl_identity
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] tidentity [ajint] Target identity
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetTargetIdentity(EnsPIdentityreference ir,
                                             ajint tidentity)
{
    if(!ir)
        return ajFalse;

    ir->TargetIdentity = tidentity;

    return ajTrue;
}




/* @func ensIdentityreferenceSetEvalue ****************************************
**
** Set the e-value element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::evalue
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] evalue [double] E-value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetEvalue(EnsPIdentityreference ir,
                                     double evalue)
{
    if(!ir)
        return ajFalse;

    ir->Evalue = evalue;

    return ajTrue;
}




/* @func ensIdentityreferenceSetScore *****************************************
**
** Set the score element of an Ensembl Identity Reference.
**
** @cc Bio::EnsEMBL::IdentityXref::score
** @param [u] ir [EnsPIdentityreference] Ensembl Identity Reference
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIdentityreferenceSetScore(EnsPIdentityreference ir,
                                    double score)
{
    if(!ir)
        return ajFalse;

    ir->Score = score;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Identity Reference object.
**
** @fdata [EnsPIdentityreference]
** @nam3rule Trace Report Ensembl Identity Reference elements to debug file
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
** @@
******************************************************************************/

AjBool ensIdentityreferenceTrace(const EnsPIdentityreference ir, ajuint level)
{
    AjPStr indent = NULL;

    if(!ir)
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




/* @func ensIdentityreferenceGetMemsize ***************************************
**
** Get the memory size in bytes of an Ensembl Identity Reference.
**
** @param [r] ir [const EnsPIdentityreference] Ensembl Identity Reference
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensIdentityreferenceGetMemsize(const EnsPIdentityreference ir)
{
    ajulong size = 0;

    if(!ir)
        return 0;

    size += sizeof (EnsOIdentityreference);

    if(ir->Cigar)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ir->Cigar);
    }

    return size;
}




/* @datasection [EnsPGeneontologylinkage] Gene Ontology Linkage ***************
**
** Functions for manipulating Ensembl Gene Ontology Linkage objects
**
** @cc Bio::EnsEMBL::DBEntry CVS Revision: 1.43
** @cc Bio::EnsEMBL::GoXref CVS Revision: 1.9
**
** @nam2rule Geneontologylinkage
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Gene Ontology Linkage by pointer.
** It is the responsibility of the user to first destroy any previous
** Gene Ontology Linkage. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGeneontologylinkage]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage
** @argrule Ref object [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage
**
** @valrule * [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage
**
** @fcategory new
******************************************************************************/




/* @func ensGeneontologylinkageNew ********************************************
**
** Default constructor for an Ensembl Gene Ontology Linkage.
**
** @param [u] linkagetype [AjPStr] Linkage type
** @param [u] source [EnsPDatabaseentry] Source Ensembl Database Entry
**
** @return [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage or NULL
** @@
******************************************************************************/

EnsPGeneontologylinkage ensGeneontologylinkageNew(AjPStr linkagetype,
                                                  EnsPDatabaseentry source)
{
    EnsPGeneontologylinkage gol = NULL;

    if(ajDebugTest("ensGeneontologylinkageNew"))
    {
        ajDebug("ensGeneontologylinkageNew\n"
                "  linkagetype '%S'\n"
                "  source %p\n",
                linkagetype,
                source);

        ensDatabaseentryTrace(source, 1);
    }

    if(!linkagetype)
        return NULL;

    AJNEW0(gol);

    gol->LinkageType = ajStrNewRef(linkagetype);
    gol->Source      = ensDatabaseentryNewRef(source);

    gol->Use = 1;

    return gol;
}




/* @func ensGeneontologylinkageNewObj *****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGeneontologylinkage] Ensembl Gene
**                                                   Ontology Linkage
**
** @return [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage or NULL
** @@
******************************************************************************/

EnsPGeneontologylinkage ensGeneontologylinkageNewObj(
    const EnsPGeneontologylinkage object)
{
    EnsPGeneontologylinkage gol = NULL;

    if(!object)
        return NULL;

    AJNEW0(gol);

    gol->LinkageType = ajStrNewRef(object->LinkageType);
    gol->Source      = ensDatabaseentryNewObj(object->Source);

    gol->Use = 1;

    return gol;
}




/* @func ensGeneontologylinkageNewRef *****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gol [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage
**
** @return [EnsPGeneontologylinkage] Ensembl Gene Ontology Linkage
** @@
******************************************************************************/

EnsPGeneontologylinkage ensGeneontologylinkageNewRef(
    EnsPGeneontologylinkage gol)
{
    if(!gol)
        return NULL;

    gol->Use++;

    return gol;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Gene Ontology Linkage.
**
** @fdata [EnsPGeneontologylinkage]
** @fnote None
**
** @nam3rule Del Destroy (free) a Gene Ontology Linkage object
**
** @argrule * Pgol [EnsPGeneontologylinkage*] Ensembl Gene Ontology Linkage
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGeneontologylinkageDel ********************************************
**
** Default Ensembl Gene Ontology Linkage destructor.
**
** @param [d] Pgol [EnsPGeneontologylinkage*] Ensembl Gene Ontology Linkage
**                                            address
**
** @return [void]
** @@
******************************************************************************/

void ensGeneontologylinkageDel(EnsPGeneontologylinkage *Pgol)
{
    EnsPGeneontologylinkage pthis = NULL;

    if(!Pgol)
        return;

    if(!*Pgol)
        return;

    pthis = *Pgol;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgol = NULL;

        return;
    }

    ajStrDel(&pthis->LinkageType);

    ensDatabaseentryDel(&pthis->Source);

    AJFREE(pthis);

    *Pgol = NULL;

    return;
}




/* @datasection [EnsPDatabaseentry] Database Entry ****************************
**
** Functions for manipulating Ensembl Database Entry objects
**
** @cc Bio::EnsEMBL::DBEntry CVS Revision: 1.43
** @cc Bio::EnsEMBL::GoXref CVS Revision: 1.9
**
** @nam2rule Databaseentry
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Ref object [EnsPDatabaseentry] Ensembl Database Entry
**
** @valrule * [EnsPDatabaseentry] Ensembl Database Entry
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseentryNew **************************************************
**
** Default constructor for an Ensembl Database Entry.
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
** @param [r] infotype [EnsEExternalreferenceInfoType] Information type
** @param [u] infotext [AjPStr] Information text
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @@
******************************************************************************/

EnsPDatabaseentry ensDatabaseentryNew(EnsPDatabaseentryadaptor dbea,
                                      ajuint identifier,
                                      EnsPAnalysis analysis,
                                      EnsPExternaldatabase edb,
                                      AjPStr primaryid,
                                      AjPStr displayid,
                                      AjPStr version,
                                      AjPStr description,
                                      AjPStr linkageannotation,
                                      EnsEExternalreferenceInfoType infotype,
                                      AjPStr infotext)
{
    EnsPDatabaseentry dbe = NULL;

    EnsPExternalreference er = NULL;

    if(ajDebugTest("ensDatabaseentryNew"))
    {
        ajDebug("ensDatabaseentryNew\n"
                "  dbea %p\n"
                "  identifier %u\n"
                "  analysis %p\n"
                "  edb %p\n"
                "  primaryid '%S'\n"
                "  displayid '%S'\n"
                "  version '%S'\n"
                "  description '%S'\n"
                "  linkageannotation '%S'\n"
                "  infotype %d\n"
                "  infotext '%S'\n",
                dbea,
                identifier,
                analysis,
                edb,
                primaryid,
                displayid,
                version,
                description,
                linkageannotation,
                infotype,
                infotext);

        ensAnalysisTrace(analysis, 1);

        ensExternaldatabaseTrace(edb, 1);
    }

    if(!edb)
        return NULL;

    er = ensExternalreferenceNew(identifier,
                                 analysis,
                                 edb,
                                 primaryid,
                                 displayid,
                                 version,
                                 description,
                                 linkageannotation,
                                 infotext,
                                 infotype);

    if(er)
    {
        AJNEW0(dbe);

        dbe->Use               = 1;
        dbe->Identifier        = identifier;
        dbe->Adaptor           = dbea;
        dbe->Externalreference = er;
        dbe->Identityreference = NULL;
        dbe->Synonyms          = ajListstrNew();
        dbe->GoLinkageTypes    = ajListNew();
    }
    else
        ajDebug("ensDatabaseentryNew could not create an "
                "External Reference.\n");

    return dbe;
}




/* @func ensDatabaseentryNewObj ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @@
******************************************************************************/

EnsPDatabaseentry ensDatabaseentryNewObj(const EnsPDatabaseentry object)
{
    AjIList iter = NULL;

    AjPStr synonym = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGeneontologylinkage gol = NULL;

    if(!object)
        return NULL;

    AJNEW0(dbe);

    dbe->Use = 1;

    dbe->Identifier = object->Identifier;

    dbe->Adaptor = object->Adaptor;

    dbe->Externalreference =
        ensExternalreferenceNewObj(object->Externalreference);

    dbe->Identityreference =
        ensIdentityreferenceNewObj(object->Identityreference);

    /* Copy the AJAX List of synonym AJAX Strings. */

    dbe->Synonyms = ajListstrNew();

    iter = ajListIterNew(object->Synonyms);

    while(!ajListIterDone(iter))
    {
        synonym = (AjPStr) ajListIterGet(iter);

        ajListPushAppend(dbe->Synonyms, (void *) ajStrNewRef(synonym));
    }

    ajListIterDel(&iter);

    /* Copy the AJAX List of Ensembl Gene Ontology Linkage objects. */

    dbe->GoLinkageTypes = ajListNew();

    iter = ajListIterNew(object->GoLinkageTypes);

    while(!ajListIterDone(iter))
    {
        gol = (EnsPGeneontologylinkage) ajListIterGet(iter);

        ajListPushAppend(dbe->GoLinkageTypes,
                         (void *) ensGeneontologylinkageNewRef(gol));
    }

    ajListIterDel(&iter);

    return dbe;
}




/* @func ensDatabaseentryNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsPDatabaseentry] Ensembl Database Entry
** @@
******************************************************************************/

EnsPDatabaseentry ensDatabaseentryNewRef(EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    dbe->Use++;

    return dbe;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Database Entry.
**
** @fdata [EnsPDatabaseentry]
** @fnote None
**
** @nam3rule Del Destroy (free) a Database Entry object
**
** @argrule * Pdbe [EnsPDatabaseentry*] Database Entry object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseentryDel **************************************************
**
** Default Ensembl Database Entry destructor.
**
** @param [d] Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @return [void]
** @@
******************************************************************************/

void ensDatabaseentryDel(EnsPDatabaseentry *Pdbe)
{
    EnsPGeneontologylinkage gol = NULL;
    EnsPDatabaseentry pthis = NULL;

    if(!Pdbe)
        return;

    if(!*Pdbe)
        return;

    pthis = *Pdbe;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pdbe = NULL;

        return;
    }

    ensExternalreferenceDel(&pthis->Externalreference);

    ensIdentityreferenceDel(&pthis->Identityreference);

    ajListstrFree(&pthis->Synonyms);

    while(ajListPop(pthis->GoLinkageTypes, (void **) &gol))
        ensGeneontologylinkageDel(&gol);

    ajListFree(&pthis->GoLinkageTypes);

    AJFREE(pthis);

    *Pdbe = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
** @fnote None
**
** @nam3rule Get Return Database Entry attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Database Entry Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetPrimaryIdentifier Return the primary identifier
** @nam4rule GetDisplayIdentifier Return the display identifier
** @nam4rule GetVersion Return the version
** @nam4rule GetDescription Return the description
** @nam4rule GetInfoText Return the information text
** @nam4rule GetInfoType Return the information type
** @nam4rule GetDbName Return the database name
** @nam4rule GetDbRelease Return the database release
** @nam4rule GetDbDisplayName Return the database display name
**
** @argrule * edb [const EnsPExternaldatabase] External Database
**
** @valrule Adaptor [EnsPExternaldatabaseadaptor] Ensembl External
**                                                Database Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Release [AjPStr] Release
** @valrule SecondaryName [AjPStr] Secondary name
** @valrule SecondaryTable [AjPStr] Secondary table
** @valrule PrimaryIdIsLinkable [AjBool] Primary identifier is linkable
** @valrule DisplayIdIsLinkable [AjBool] Display identifier is linkable
** @valrule Status [EnsEExternaldatabaseStatus] Status
** @valrule Type [EnsEExternaldatabaseType] Type
** @valrule Priority [ajint] Priority
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseentryGetAdaptor *******************************************
**
** Get the Ensembl Database Entry Adaptor element of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @@
******************************************************************************/

EnsPDatabaseentryadaptor ensDatabaseentryGetAdaptor(
    const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    return dbe->Adaptor;
}




/* @func ensDatabaseentryGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [ajuint] SQL database-internal identifier of an
**                  Ensembl Database Entry
** @@
******************************************************************************/

ajuint ensDatabaseentryGetIdentifier(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return 0;

    return dbe->Identifier;
}




/* @func ensDatabaseentryGetPrimaryIdentifier *********************************
**
** Get the primary identifier element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Primary identifier
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetPrimaryIdentifier(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    return dbe->Externalreference->PrimaryIdentifier;
}




/* @func ensDatabaseentryGetDisplayIdentifier *********************************
**
** Get the display identifier element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::display_id
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Display identifier
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDisplayIdentifier(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    return dbe->Externalreference->DisplayIdentifier;
}




/* @func ensDatabaseentryGetVersion *******************************************
**
** Get the version element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::version
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Version
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetVersion(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    return dbe->Externalreference->Version;
}




/* @func ensDatabaseentryGetDescription ***************************************
**
** Get the description element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDescription(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    return dbe->Externalreference->Description;
}




/* @func ensDatabaseentryGetLinkageAnnotation *********************************
**
** Get the linkage annotation element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::linkage_annotation
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Linkage annotation
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetLinkageAnnotation(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    return dbe->Externalreference->LinkageAnnotation;
}




/* @func ensDatabaseentryGetInfoText ******************************************
**
** Get the information text element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::info_text
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Information text
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetInfoText(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    return dbe->Externalreference->InfoText;
}




/* @func ensDatabaseentryGetInfoType ******************************************
**
** Get the information type element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::info_type
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternalreferenceInfoType] Information type or
**                                         ensEExternalreferenceInfoTypeNULL
** @@
******************************************************************************/

EnsEExternalreferenceInfoType ensDatabaseentryGetInfoType(
    const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return ensEExternalreferenceInfoTypeNULL;

    if(!dbe->Externalreference)
        return ensEExternalreferenceInfoTypeNULL;

    return dbe->Externalreference->InfoType;
}




/* @func ensDatabaseentryGetDbName ********************************************
**
** Get the database name element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Database name
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDbName(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    if(!dbe->Externalreference->Externaldatabase)
        return NULL;

    return dbe->Externalreference->Externaldatabase->Name;
}




/* @func ensDatabaseentryGetDbRelease *****************************************
**
** Get the database release element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Database release
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDbRelease(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    if(!dbe->Externalreference->Externaldatabase)
        return NULL;

    return dbe->Externalreference->Externaldatabase->Release;
}




/* @func ensDatabaseentryGetDbDisplayName *************************************
**
** Get the database display name element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::db_display_name
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPStr] Database display name
** @@
******************************************************************************/

AjPStr ensDatabaseentryGetDbDisplayName(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    if(!dbe->Externalreference)
        return NULL;

    if(!dbe->Externalreference->Externaldatabase)
        return NULL;

    return dbe->Externalreference->Externaldatabase->DisplayName;
}




/* @func ensDatabaseentryGetPrimaryIdIsLinkable *******************************
**
** Get the 'primary identifier is linkable' element of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id_linkable
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue if the primary identifier is linkable
** @@
******************************************************************************/

AjBool ensDatabaseentryGetPrimaryIdIsLinkable(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return ajFalse;

    if(!dbe->Externalreference)
        return ajFalse;

    if(!dbe->Externalreference->Externaldatabase)
        return ajFalse;

    return dbe->Externalreference->Externaldatabase->PrimaryIdIsLinkable;
}




/* @func ensDatabaseentryGetDisplayIdIsLinkable *******************************
**
** Get the 'display identifier is linkable' element of an
** Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::display_id_linkable
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue if the display identifier is linkable
** @@
******************************************************************************/

AjBool ensDatabaseentryGetDisplayIdIsLinkable(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return ajFalse;

    if(!dbe->Externalreference)
        return ajFalse;

    if(!dbe->Externalreference->Externaldatabase)
        return ajFalse;

    return dbe->Externalreference->Externaldatabase->DisplayIdIsLinkable;
}




/* @func ensDatabaseentryGetPriority ******************************************
**
** Get the priority element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [ajint] Priortity
** @@
******************************************************************************/

ajint ensDatabaseentryGetPriority(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return 0;

    if(!dbe->Externalreference)
        return 0;

    if(!dbe->Externalreference->Externaldatabase)
        return 0;

    return dbe->Externalreference->Externaldatabase->Priority;
}




/* @func ensDatabaseentryGetStatus ********************************************
**
** Get the status element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternaldatabaseStatus] Status or
**                                      ensEExternaldatabaseStatusNULL
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensDatabaseentryGetStatus(
    const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return ensEExternaldatabaseStatusNULL;

    if(!dbe->Externalreference)
        return ensEExternaldatabaseStatusNULL;

    if(!dbe->Externalreference->Externaldatabase)
        return ensEExternaldatabaseStatusNULL;

    return dbe->Externalreference->Externaldatabase->Status;
}




/* @func ensDatabaseentryGetType **********************************************
**
** Get the type element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::type
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [EnsEExternaldatabaseType] Type or ensEExternaldatabaseTypeNULL
** @@
******************************************************************************/

EnsEExternaldatabaseType ensDatabaseentryGetType(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return ensEExternaldatabaseTypeNULL;

    if(!dbe->Externalreference)
        return ensEExternaldatabaseTypeNULL;

    if(!dbe->Externalreference->Externaldatabase)
        return ensEExternaldatabaseTypeNULL;

    return dbe->Externalreference->Externaldatabase->Type;
}




/* @func ensDatabaseentryGetSynonyms ******************************************
**
** Get the synonyms element of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::DBEntry::get_all_synonyms
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjPList] AJAX List of AJAX Strings
** @@
******************************************************************************/

AjPList ensDatabaseentryGetSynonyms(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    return dbe->Synonyms;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Entry object.
**
** @fdata [EnsPDatabaseentry]
** @nam3rule Trace Report Ensembl Database Entry elements to debug file
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
** @@
******************************************************************************/

AjBool ensDatabaseentryTrace(const EnsPDatabaseentry dbe, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr synonym = NULL;
    AjPStr linkage = NULL;

    if(!dbe)
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
            "%S  GoLinkageTypes %p\n",
            indent, dbe,
            indent, dbe->Use,
            indent, dbe->Identifier,
            indent, dbe->Adaptor,
            indent, dbe->Externalreference,
            indent, dbe->Identityreference,
            indent, dbe->Synonyms,
            indent, dbe->GoLinkageTypes);

    ensExternalreferenceTrace(dbe->Externalreference, level + 1);

    ensIdentityreferenceTrace(dbe->Identityreference, level + 1);

    /* Trace the AJAX List of synonym AJAX Strings. */

    if(dbe->Synonyms)
    {
        ajDebug("%S    AJAX List %p of AJAX String synonyms\n",
                indent, dbe->Synonyms);

        iter = ajListIterNewread(dbe->Synonyms);

        while(!ajListIterDone(iter))
        {
            synonym = (AjPStr) ajListIterGet(iter);

            ajDebug("%S        '%S'\n", indent, synonym);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of GO linkage type AJAX Strings. */

    if(dbe->GoLinkageTypes)
    {
        ajDebug("%S    AJAX List %p of AJAX String GO linkage types\n",
                indent, dbe->GoLinkageTypes);

        iter = ajListIterNewread(dbe->GoLinkageTypes);

        while(!ajListIterDone(iter))
        {
            linkage = (AjPStr) ajListIterGet(iter);

            ajDebug("%S        '%S'\n", indent, linkage);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensDatabaseentryGetMemsize *******************************************
**
** Get the memory size in bytes of an Ensembl Database Entry.
**
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensDatabaseentryGetMemsize(const EnsPDatabaseentry dbe)
{
    ajulong size = 0;

    AjIList iter = NULL;

    AjPStr synonym = NULL;
    AjPStr linkage = NULL;

    if(!dbe)
        return 0;

    size += sizeof (EnsODatabaseentry);

    size += ensExternalreferenceGetMemsize(dbe->Externalreference);

    size += ensIdentityreferenceGetMemsize(dbe->Identityreference);

    /* Summarise the AJAX List of synonym AJAX Strings. */

    if(dbe->Synonyms)
    {
        size += sizeof (AjOList);

        iter = ajListIterNew(dbe->Synonyms);

        while(!ajListIterDone(iter))
        {
            synonym = (AjPStr) ajListIterGet(iter);

            if(synonym)
            {
                size += sizeof (AjOStr);

                size += ajStrGetRes(synonym);
            }
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of GO linkage type AJAX Strings. */

    if(dbe->GoLinkageTypes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNew(dbe->GoLinkageTypes);

        while(!ajListIterDone(iter))
        {
            linkage = (AjPStr) ajListIterGet(iter);

            if(linkage)
            {
                size += sizeof (AjOStr);

                size += ajStrGetRes(linkage);
            }
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @func ensDatabaseentryAddGeneontologylinkage *******************************
**
** Add an Ensembl Gene Ontology Linkage to an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::GoXref::add_linkage_type
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] linkagetype [AjPStr] Linkage type
** @param [u] source [EnsPDatabaseentry] Source Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryAddGeneontologylinkage(EnsPDatabaseentry dbe,
                                              AjPStr linkagetype,
                                              EnsPDatabaseentry source)
{
    EnsPGeneontologylinkage gol = NULL;

    if(!dbe)
        return ajFalse;

    gol = ensGeneontologylinkageNew(linkagetype, source);

    if(gol)
    {
        ajListPushAppend(dbe->GoLinkageTypes, (void *) gol);

        return ajTrue;
    }

    return ajFalse;
}




/* @func ensDatabaseentryGetGeneontologylinkages ******************************
**
** Get Ensembl Gene Ontology Linkages of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::GoXref::get_all_linkage_info
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
**
** @return [const AjPList] AJAX List of Ensembl Gene Ontology Linkages
** @@
******************************************************************************/

const AjPList ensDatabaseentryGetGeneontologylinkages(const EnsPDatabaseentry dbe)
{
    if(!dbe)
        return NULL;

    return dbe->GoLinkageTypes;
}




/* @func ensDatabaseentryGetAllLinkageTypes ***********************************
**
** Get Ensembl Gene Ontology Linkages of an Ensembl Database Entry.
**
** The caller is responsible for deleting the AJAX String linkage types
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::GoXref::get_all_linkage_types
** @param [r] dbe [const EnsPDatabaseentry] Ensembl Database Entry
** @param [u] types [AjPList] AJAX List of AJAX String linkage types
**                           (i.e. Gene Ontology Evidence Codes)
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryGetAllLinkageTypes(const EnsPDatabaseentry dbe,
                                          AjPList types)
{
    AjBool match = AJFALSE;

    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    AjPStr type = NULL;

    EnsPGeneontologylinkage gol = NULL;

    if(!dbe)
        return ajFalse;

    if(!types)
        return ajFalse;

    iter1 = ajListIterNew(dbe->GoLinkageTypes);

    iter2 = ajListIterNew(types);

    while(!ajListIterDone(iter1))
    {
        gol = (EnsPGeneontologylinkage) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        match = ajFalse;

        while(!ajListIterDone(iter2))
        {
            type = (AjPStr) ajListIterGet(iter2);

            if(ajStrMatchS(gol->LinkageType, type))
            {
                match = ajTrue;

                break;
            }
        }

        if(!match)
            ajListPushAppend(types, (void *) ajStrNewRef(type));
    }

    ajListIterDel(&iter1);

    ajListIterDel(&iter2);

    return ajTrue;
}




/* @func EnsDatabaseentryClearGeneontologylinkageTypes ************************
**
** Clear all Ensembl Gene Ontology Linkages of an Ensembl Database Entry.
**
** @cc Bio::EnsEMBL::GoXref::flush_linkage_types
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool EnsDatabaseentryClearGeneontologylinkageTypes(EnsPDatabaseentry dbe)
{
    EnsPGeneontologylinkage gol = NULL;

    if(!dbe)
        return ajFalse;

    while(ajListPop(dbe->GoLinkageTypes, (void **) &gol))
        ensGeneontologylinkageDel(&gol);

    return ajTrue;
}




/* @datasection [EnsPDatabaseentryadaptor] Database Entry Adaptor *************
**
** Functions for manipulating Ensembl Database Entry Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor CVS Revision: 1.143
**
** @nam2rule Databaseentryadaptor
**
******************************************************************************/

/*
  static const char *databaseentryadaptorTables[] =
  {
  "xref",
  "external_db",
  "external_synonym",
  "identity_xref",
  NULL
  };

  static const char *databaseentryadaptorColumns[] =
  {
  "xref.xref_id",
  "xref.external_db_id",
  "xref.dbprimary_acc",
  "xref.display_label",
  "xref.version",
  "xref.description",
  "xref.info_type",
  "xref.info_text",
  "external_db.external_db_id",
  "external_db.db_name",
  "external_db.db_release",
  "external_db.db_display_name",
  "external_db.secondary_db_name",
  "external_db.secondary_db_table",
  "external_db.dbprimary_acc_linkable",
  "external_db.display_label_linkable",
  "external_db.priority",
  "external_db.status",
  "external_db.type",
  "external_synonym.synonym",
  NULL
  };

  static EnsOBaseadaptorLeftJoin databaseentryadaptorLeftJoin[] =
  {
  {
  "external_db",
  "xref.external_db_id = external_db.external_db_id"
  },
  {
  "external_synonym",
  "external_synonym.xref_id = xref.xref_id"
  },
  {NULL, NULL}
  };

  static const char *databaseentryadaptorDefaultCondition = NULL;

  static const char *databaseentryadaptorFinalCondition = NULL;
*/




/* @funcstatic databaseentryadaptorHasLinkage *********************************
**
** Check whether a linkage String has already been indexed for a particular
** Ensembl External Reference identifier.
**
** @param [u] linkages [AjPTable] Linkage Table
** @param [r] xrefid [ajuint] External Reference identifier
** @param [u] linkage [AjPStr] Linkage string
**
** @return [AjBool] ajTrue: A particular linkage String has already
**                          been indexed.
**                  ajFalse: A particular linkage String has not been indexed
**                           before, but has been added now.
** @@
** This function keeps a primary AJAX Table of External References ajuint key
** data and secondary AJAX Tables as value data.
** The secondary AJAX Tables contain linkage AJAX String key data and AjBool
** value data.
******************************************************************************/

static AjBool databaseentryadaptorHasLinkage(AjPTable linkages,
                                             ajuint xrefid,
                                             AjPStr linkage)
{
    ajuint *Pidentifier = NULL;

    AjBool *Pbool = NULL;

    AjPTable table = NULL;

    if(!linkages)
        return ajFalse;

    if(!xrefid)
        return ajFalse;

    if(!linkage)
        return ajFalse;

    table = (AjPTable) ajTableFetch(linkages, (const void *) &xrefid);

    if(table)
    {
        if(ajTableFetch(table, (const void *) linkage))
            return ajTrue;
        else
        {
            AJNEW0(Pbool);

            *Pbool = ajTrue;

            ajTablePut(table, (void *) ajStrNewRef(linkage), (void *) Pbool);

            return ajFalse;
        }
    }
    else
    {
        AJNEW0(Pidentifier);

        *Pidentifier = xrefid;

        table = ajTablestrNewLen(0);

        ajTablePut(linkages, (void *) Pidentifier, (void *) table);

        AJNEW0(Pbool);

        *Pbool = ajTrue;

        ajTablePut(table, (void *) ajStrNewRef(linkage), (void *) Pbool);

        return ajFalse;
    }
}




/* @funcstatic databaseentryadaptorHasSynonym *********************************
**
** Check whether a synonym String has already been indexed for a particular
** Ensembl External Reference identifier.
**
** @param [u] synonyms [AjPTable] Synonym Table
** @param [r] xrefid [ajuint] External Reference identifier
** @param [u] synonym [AjPStr] Synonym string
**
** @return [AjBool] ajTrue: A particular synonym String has already
**                          been indexed.
**                  ajFalse: A particular synonym String has not been indexed
**                           before, but has been added now.
** @@
** This function keeps a primary AJAX Table of External References ajuint key
** data and secondary AJAX Tables as value data.
** The secondary AJAX Tables contain synonym AJAX String key data and AjBool
** value data.
******************************************************************************/

static AjBool databaseentryadaptorHasSynonym(AjPTable synonyms,
                                             ajuint xrefid,
                                             AjPStr synonym)
{
    ajuint *Pidentifier = NULL;

    AjBool *Pbool = NULL;

    AjPTable table = NULL;

    if(!synonyms)
        return ajFalse;

    if(!xrefid)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    table = (AjPTable) ajTableFetch(synonyms, (const void *) &xrefid);

    if(table)
    {
        if(ajTableFetch(table, (const void *) synonym))
            return ajTrue;
        else
        {
            AJNEW0(Pbool);

            *Pbool = ajTrue;

            ajTablePut(table, (void *) ajStrNewRef(synonym), (void *) Pbool);

            return ajFalse;
        }
    }
    else
    {
        AJNEW0(Pidentifier);

        *Pidentifier = xrefid;

        table = ajTablestrNewLen(0);

        ajTablePut(synonyms, (void *) Pidentifier, (void *) table);

        AJNEW0(Pbool);

        *Pbool = ajTrue;

        ajTablePut(table, (void *) ajStrNewRef(synonym), (void *) Pbool);

        return ajFalse;
    }
}




/* @funcstatic databaseentryadaptorCacheClear *********************************
**
** Clear an Ensembl Database Entry Adaptor-internal cache.
**
** This function clears unsigned integer key and second-level AJAX Table value
** data from first-level AJAX Tables, as well as unsigned integer key and
** AJAX Boolean value data from the second-level AJAX Table.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool databaseentryadaptorCacheClear(AjPTable table)
{
    void **keyarray1 = NULL;
    void **valarray1 = NULL;
    void **keyarray2 = NULL;
    void **valarray2 = NULL;

    register ajuint i = 0;
    register ajuint j = 0;

    if(!table)
        return ajFalse;

    ajTableToarrayKeysValues(table, &keyarray1, &valarray1);

    for(i = 0; keyarray1[i]; i++)
    {
        ajTableRemove(table, (const void *) keyarray1[i]);

        /* Delete the first-level unsigned integer key data. */

        AJFREE(keyarray1[i]);

        /*
        ** Clear the second-level AJAX Tables with AJAX String key and
        ** AJAX Boolean value data.
        */

        ajTableToarrayKeysValues((AjPTable) valarray1[i],
                                 &keyarray2, &valarray2);

        for(j = 0; keyarray2[j]; j++)
        {
            ajTableRemove((AjPTable) valarray1[i],
                          (const void *) keyarray2[j]);

            /* Delete AJAX String keys and AJAX Boolean values. */

            ajStrDel((AjPStr *) &keyarray2[j]);

            AJFREE(valarray2[j]);
        }

        /* Delete the second-level AJAX Table as first-level value data. */

        ajTableFree((AjPTable *) &valarray1[i]);

        AJFREE(keyarray2);
        AJFREE(valarray2);
    }

    AJFREE(keyarray1);
    AJFREE(valarray1);

    return ajTrue;
}




/* @funcstatic databaseentryadaptorTempClear **********************************
**
** Clear an Ensembl Database Entry Adaptor-internal cache.
**
** This function clears unsigned integer key and Ensembl Database Entry
** value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool databaseentryadaptorTempClear(AjPTable table)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    if(!table)
        return ajFalse;

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajTableRemove(table, (const void *) keyarray[i]);

        /* Delete the first-level unsigned integer key data. */

        AJFREE(keyarray[i]);

        /* Delete the first-level Ensembl Database Entry value data. */

        ensDatabaseentryDel((EnsPDatabaseentry *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @funcstatic databaseentryadaptorFetchAllBySQL ******************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Database Entry objects.
**
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool databaseentryadaptorFetchAllBySQL(EnsPDatabaseentryadaptor dbea,
                                                const AjPStr statement,
                                                AjPList dbes)
{
    double score = 0;
    double evalue = 0;

    ajint ierqryidt = 0;
    ajint iertrgidt = 0;
    ajint ierqrysrt = 0;
    ajint ierqryend = 0;
    ajint iertrgsrt = 0;
    ajint iertrgend = 0;

    ajuint xrefid     = 0;
    ajuint edbid      = 0;
    ajuint objxrfid   = 0;
    ajuint analysisid = 0;
    ajuint sourceid   = 0;

    ajuint *Pidentifier = NULL;

    EnsEExternalreferenceInfoType einfotype =
        ensEExternalreferenceInfoTypeNULL;

    AjPTable detable  = NULL;
    AjPTable linkages = NULL;
    AjPTable synonyms = NULL;

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

    AjPStr synonym = NULL;

    AjPStr cigar     = NULL;
    AjPStr golinkage = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseentry dbe       = NULL;
    EnsPDatabaseentry sourcedbe = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    if(!dbea)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!dbes)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dbea->Adaptor);

    edba = ensRegistryGetExternaldatabaseadaptor(dbea->Adaptor);

    detable  = MENSTABLEUINTNEW(0);
    linkages = MENSTABLEUINTNEW(0);
    synonyms = MENSTABLEUINTNEW(0);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        /* 'xref' table */
        xrefid = 0;
        edbid  = 0;
        primaryid   = ajStrNew();
        displayid   = ajStrNew();
        version     = ajStrNew();
        description = ajStrNew();
        infotype    = ajStrNew();
        einfotype   = ensEExternalreferenceInfoTypeNULL;
        infotext    = ajStrNew();

        /* 'external_synonym' table */
        synonym     = ajStrNew();

        if(ajSqlrowGetColumns(sqlr) > 10)
        {
            /* 'object_xref' table */
            objxrfid = 0;
            linkageannotation = ajStrNew();
            analysisid = 0;
            /* 'identity_xref' table */
            ierqryidt = 0;
            iertrgidt = 0;
            ierqrysrt = 0;
            ierqryend = 0;
            iertrgsrt = 0;
            iertrgend = 0;
            cigar     = ajStrNew();
            score     = 0;
            evalue    = 0;
            /* 'go_xref' table */
            golinkage = ajStrNew();
            sourceid  = 0;
        }

        sqlr = ajSqlrowiterGet(sqli);

        /* 'xref' table */
        ajSqlcolumnToUint(sqlr, &xrefid);
        ajSqlcolumnToUint(sqlr, &edbid);
        ajSqlcolumnToStr(sqlr, &primaryid);
        ajSqlcolumnToStr(sqlr, &displayid);
        ajSqlcolumnToStr(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &infotype);
        ajSqlcolumnToStr(sqlr, &infotext);
        /* 'external_synonym' table */
        ajSqlcolumnToStr(sqlr, &synonym);

        if(ajSqlrowGetColumns(sqlr) > 10)
        {
            /* 'object_xref' table */
            ajSqlcolumnToUint(sqlr, &objxrfid);
            ajSqlcolumnToStr(sqlr, &linkageannotation);
            ajSqlcolumnToUint(sqlr, &analysisid);
            /* 'identity_xref' table */
            ajSqlcolumnToInt(sqlr, &ierqryidt);
            ajSqlcolumnToInt(sqlr, &iertrgidt);
            ajSqlcolumnToInt(sqlr, &ierqrysrt);
            ajSqlcolumnToInt(sqlr, &ierqryend);
            ajSqlcolumnToInt(sqlr, &iertrgsrt);
            ajSqlcolumnToInt(sqlr, &iertrgend);
            ajSqlcolumnToStr(sqlr, &cigar);
            ajSqlcolumnToDouble(sqlr, &score);
            ajSqlcolumnToDouble(sqlr, &evalue);
            /* 'go_xref' table */
            ajSqlcolumnToStr(sqlr, &golinkage);
            ajSqlcolumnToUint(sqlr, &sourceid);
        }

        dbe = (EnsPDatabaseentry) ajTableFetch(detable,
                                               (const void *) &xrefid);

        if(!dbe)
        {
            einfotype = ensExternalreferenceInfoTypeFromStr(infotype);

            if(!einfotype)
                ajFatal("databaseentryadaptorFetchAllBySQL encountered "
                        "unexpected string '%S' in the "
                        "'xref.infotype' field.\n", infotype);

            ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

            ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

            dbe = ensDatabaseentryNew(dbea,
                                      xrefid,
                                      analysis,
                                      edb,
                                      primaryid,
                                      displayid,
                                      version,
                                      description,
                                      linkageannotation,
                                      einfotype,
                                      infotext);

            ensAnalysisDel(&analysis);

            ensExternaldatabaseDel(&edb);

            AJNEW0(Pidentifier);

            *Pidentifier = xrefid;

            ajTablePut(detable, (void *) Pidentifier, (void *) dbe);

            /* An external reference with a similarity score. */

            if(ierqryidt)
            {
                dbe->Identityreference =
                    ensIdentityreferenceNew(cigar,
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

        if(synonym && ajStrGetLen(synonym) &&
           (!databaseentryadaptorHasSynonym(synonyms, xrefid, synonym)))
            ajListPushAppend(dbe->Synonyms, (void *) ajStrNewRef(synonym));

        if(golinkage && ajStrGetLen(golinkage) &&
           (!databaseentryadaptorHasLinkage(linkages, xrefid, golinkage)))
        {
            ensDatabaseentryadaptorFetchByIdentifier(dbea,
                                                     sourceid,
                                                     &sourcedbe);

            ensDatabaseentryAddGeneontologylinkage(dbe, golinkage, sourcedbe);

            ensDatabaseentryDel(&sourcedbe);
        }

        ajStrDel(&primaryid);
        ajStrDel(&displayid);
        ajStrDel(&version);
        ajStrDel(&description);
        ajStrDel(&linkageannotation);
        ajStrDel(&infotype);
        ajStrDel(&infotext);
        ajStrDel(&synonym);

        if(ajSqlrowGetColumns(sqlr) > 10)
        {
            ajStrDel(&cigar);
            ajStrDel(&golinkage);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dbea->Adaptor, &sqls);

    databaseentryadaptorCacheClear(linkages);
    databaseentryadaptorCacheClear(synonyms);

    databaseentryadaptorTempClear(detable);

    ajTableFree(&linkages);
    ajTableFree(&synonyms);
    ajTableFree(&detable);

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
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseentryadaptorNew *******************************************
**
** Default Ensembl Database Entry Adaptor constructor.
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseentryadaptor ensDatabaseentryadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPDatabaseentryadaptor dbea = NULL;

    if(!dba)
        return NULL;

    AJNEW0(dbea);

    dbea->Adaptor = dba;

    return dbea;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Database Entry Adaptor.
**
** @fdata [EnsPDatabaseentryadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Database Entry Adaptor object
**
** @argrule * Pdbea [EnsPDatabaseentryadaptor*] Ensembl Database Entry
**                                              Adaptor object address
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
** @param [d] Pdbea [EnsPDatabaseentryadaptor*] Ensembl Database Entry Adaptor
**                                              address
**
** @return [void]
** @@
******************************************************************************/

void ensDatabaseentryadaptorDel(EnsPDatabaseentryadaptor *Pdbea)
{
    if(!Pdbea)
        return;

    if(!*Pdbea)
        return;

    AJFREE(*Pdbea);

    *Pdbea = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Database Entry Adaptor
** object.
**
** @fdata [EnsPDatabaseentryadaptor]
** @fnote None
**
** @nam3rule Get Return Ensembl Database Entry Adaptor attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Database Adaptor
**
** @argrule * adaptor [const EnsPDatabaseentryadaptor] Ensembl Database
**                                                     Entry Adaptor
**
** @valrule Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseentryadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Database Entry Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::db
** @param [r] dbea [const EnsPDatabaseentryadaptor] Ensembl Database
**                                                  Entry Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseentryadaptorGetDatabaseadaptor(
    const EnsPDatabaseentryadaptor dbea)
{
    if(!dbea)
        return NULL;

    return dbea->Adaptor;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Database Entry objects from an
** Ensembl Core database.
**
** @fdata [EnsPDatabaseentryadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Database Entry object(s)
** @nam4rule FetchAll Retrieve all Ensembl Database Entry objects
** @nam5rule FetchAllBy Retrieve all Ensembl Database Entry objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Database Entry object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPDatabaseentryadaptor] Ensembl Database
**                                                     Entry Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Database Entry objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseentryadaptorFetchByIdentifier *****************************
**
** Fetch an Ensembl Database Entry via its internal SQL database identifier.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_by_dbID
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!dbea)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pdbe)
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

    databaseentryadaptorFetchAllBySQL(dbea, statement, dbes);

    ajStrDel(&statement);

    if(!ajListGetLength(dbes))
        ajDebug("ensDatabaseentryadaptorFetchById did not get an "
                "Ensembl Database Entry for identifier %u.\n",
                identifier);

    if(ajListGetLength(dbes) > 1)
        ajDebug("ensDatabaseentryadaptorFetchById got more than one (%u) "
                "Ensembl Database Entry for identifier %u.\n",
                ajListGetLength(dbes), identifier);

    ajListPop(dbes, (void **) Pdbe);

    while(ajListPop(dbes, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&dbes);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchByDbNameAccession ************************
**
** Fetch an Ensembl Database Entry via its database name and accession number.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_by_db_accession
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] dbname [const AjPStr] Ensembl Exernal Database name
** @param [r] accession [const AjPStr] Ensembl External Reference primary identifier
** @param [wP] Pdbe [EnsPDatabaseentry*] Ensembl Database Entry address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchByDbNameAccession(
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

    if(!dbea)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!accession)
        return ajFalse;

    if(!Pdbe)
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

    databaseentryadaptorFetchAllBySQL(dbea, statement, dbes);

    ajStrDel(&statement);

    if(!ajListGetLength(dbes))
    {
        ajDebug("ensDatabaseentryadaptorFetchByDbNameAccession did not get an "
                "Ensembl Database Entry for database name '%S' and "
                "accession '%S'.\n", dbname, accession);

        /*
        ** This is a minor hack that means that results still come back even
        ** when a mistake was made and no InterPro accessions were loaded into
        ** the xref table. This has happened in the past and had the result of
        ** breaking Ensembl DomainView.
        */

        if(ajStrMatchC(dbname, "interpro"))
        {
            statement = ajFmtStr(
                "SELECT "
                "'0', "                /* xref.xref_id */
                "'0', "                /* xref.external_db_id */
                "interpro.accession, " /* xref.dbprimary_acc */
                "interpro.id, "        /* xref.display_label */
                "NULL, "               /* xref.version */
                "NULL, "               /* xref.description */
                "NULL, "               /* xref.info_type */
                "NULL, "               /* xref.info_text */
                "NULL "                /* external_synonym.synonym */
                "FROM "
                "interpro "
                "WHERE "
                "interpro.accession = '%s'",
                txtaccession);

            databaseentryadaptorFetchAllBySQL(dbea, statement, dbes);

            ajStrDel(&statement);
        }
    }

    if(ajListGetLength(dbes) > 1)
        ajDebug("ensDatabaseentryadaptorFetchByDbNameAccession got more than "
                "one Ensembl Database Entry for database name '%S' and "
                "accession '%S'.\n", dbname, accession);

    ajListPop(dbes, (void **) Pdbe);

    while(ajListPop(dbes, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&dbes);

    ajCharDel(&txtdbname);
    ajCharDel(&txtaccession);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllByObjectType **************************
**
** Fetch all Ensembl Database Entries by an Ensembl Object type.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::_fetch_by_object_type
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] objid [ajuint] Ensembl Object identifier
** @param [r] objtype [const AjPStr] Ensembl Object type
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [rN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllByObjectType(
    EnsPDatabaseentryadaptor dbea,
    ajuint objid,
    const AjPStr objtype,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    char *txtobjtype = NULL;
    char *txtdbname = NULL;

    AjPStr statement = NULL;

    if(!dbea)
        return ajFalse;

    if(!objid)
        return ajFalse;

    if(!objtype)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtobjtype, objtype);

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
        "go_xref.linkage_type, "
        "go_xref.source_xref_id "
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
        "go_xref "
        "ON "
        "object_xref.object_xref_id = "
        "go_xref.object_xref_id "
        "WHERE "
        "xref.external_db_id = external_db.external_db_id "
        "AND "
        "xref.xref_id = object_xref.xref_id "
        "AND "
        "object_xref.ensembl_id = %u "
        "AND "
        "object_xref.ensembl_object_type = '%s'",
        objid,
        txtobjtype);

    ajCharDel(&txtobjtype);

    if(dbname && ajStrGetLen(dbname))
    {
        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

        ajFmtPrintAppS(&statement,
                       " AND external_db.db_name like '%s'",
                       txtdbname);

        ajCharDel(&txtdbname);
    }

    if(dbtype)
        ajFmtPrintAppS(&statement,
                       " AND external_db.type = '%s'",
                       ensExternaldatabaseTypeToChar(dbtype));

    databaseentryadaptorFetchAllBySQL(dbea, statement, dbes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllByGene ********************************
**
** Fetch all Ensembl Database Entries by an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_Gene
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [rN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllByGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPGene gene,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    AjBool value = AJFALSE;

    AjPStr objtype = NULL;

    if(!dbea)
        return ajFalse;

    if(!gene)
        return ajFalse;

    if(!dbes)
        return ajFalse;

    objtype = ajStrNewC("Gene");

    value = ensDatabaseentryadaptorFetchAllByObjectType(
        dbea,
        ensGeneGetIdentifier(gene),
        objtype,
        dbname,
        dbtype,
        dbes);

    ajStrDel(&objtype);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllByTranscript **************************
**
** Fetch all Ensembl Database Entries by an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_Transcript
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [rN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllByTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranscript transcript,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    AjBool value = AJFALSE;

    AjPStr objtype = NULL;

    if(!dbea)
        return ajFalse;

    if(!transcript)
        return ajFalse;

    if(!dbes)
        return ajFalse;

    objtype = ajStrNewC("Transcript");

    value = ensDatabaseentryadaptorFetchAllByObjectType(
        dbea,
        ensTranscriptGetIdentifier(transcript),
        objtype,
        dbname,
        dbtype,
        dbes);

    ajStrDel(&objtype);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllByTranslation *************************
**
** Fetch all Ensembl Database Entries by an Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_Translation
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [rN] dbtype [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllByTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranslation translation,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes)
{
    AjBool value = AJFALSE;

    AjPStr objtype = NULL;

    if(!dbea)
        return ajFalse;

    if(!translation)
        return ajFalse;

    if(!dbes)
        return ajFalse;

    objtype = ajStrNewC("Translation");

    value = ensDatabaseentryadaptorFetchAllByObjectType(
        dbea,
        ensTranslationGetIdentifier(translation),
        objtype,
        dbname,
        dbtype,
        dbes);

    ajStrDel(&objtype);

    return value;
}




/* @funcstatic databaseentryadaptorCompareIdentifier **************************
**
** Comparison function to sort unsigned integer (SQL) identifiers in
** ascending order.
**
** @param [r] P1 [const void*] Unsigned integer address 1
** @param [r] P2 [const void*] Unsigned integer address 2
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int databaseentryadaptorCompareIdentifier(const void *P1,
                                                 const void *P2)
{
    int value = 0;

    const ajuint *Pidentifier1 = NULL;
    const ajuint *Pidentifier2 = NULL;

    Pidentifier1 = *(ajuint * const *) P1;
    Pidentifier2 = *(ajuint * const *) P2;

    if(ajDebugTest("databaseentryadaptorCompareIdentifier"))
        ajDebug("databaseentryadaptorCompareIdentifier\n"
                "  identifier1 %u\n"
                "  identifier2 %u\n",
                *Pidentifier1,
                *Pidentifier2);

    /* Sort empty values towards the end of the AJAX List. */

    if(Pidentifier1 && (!Pidentifier2))
        return -1;

    if((!Pidentifier1) && (!Pidentifier2))
        return 0;

    if((!Pidentifier1) && Pidentifier2)
        return +1;

    /* Evaluate identifiers */

    if(*Pidentifier1 < *Pidentifier2)
        value = -1;

    if(*Pidentifier1 > *Pidentifier2)
        value = +1;

    return value;
}




/* @funcstatic databaseentryadaptorDeleteIdentifier ***************************
**
** ajListSortUnique nodedelete function to delete unsigned integer SQL
** identifiers that are redundant.
**
** @param [r] PP1 [void**] Unsigned integer pointer address 1
** @param [r] cl [void*] Standard. Passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
** @@
******************************************************************************/

static void databaseentryadaptorDeleteIdentifier(void **PP1, void *cl)
{
    if(!PP1)
        return;

    (void) cl;

    AJFREE(PP1);

    return;
}




/* @funcstatic databaseentryadaptorFetchAllIdentifiersByExternalName **********
**
** Fetch SQL database-internal Ensembl identifiers via an external name.
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
** @@
******************************************************************************/

static AjBool databaseentryadaptorFetchAllIdentifiersByExternalName(
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

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr sqlfrom   = NULL;
    AjPStr sqlwhere  = NULL;
    AjPStr sqlselect = NULL;
    AjPStr temporary = NULL;

    if(!dbea)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!ensembltype)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    sqlselect = ajStrNewC("object_xref.ensembl_id");
    sqlfrom   = ajStrNew();
    sqlwhere  = ajStrNew();

    if(extratype && ajStrGetLen(extratype))
    {
        temporary = ajStrNewS(extratype);

        ajStrFmtLower(&temporary);

        if(ajStrMatchCaseC(temporary, "Translation"))
            ajStrAssignC(&sqlselect, "translation.translation_id");
        else
        {
            ajStrDel(&sqlselect);

            sqlselect = ajFmtStr("transcript.%S_id", temporary);
        }

        if(ajStrMatchCaseC(ensembltype, "Translation"))
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

    if(ajStrMatchC(ensembltype, "Gene"))
    {
        ajStrAssignC(&sqlfrom, "gene");

        ajStrAssignC(&sqlwhere,
                     "gene.is_current = 1 "
                     "AND "
                     "gene.gene_id = object_xref.ensembl_id");
    }
    else if(ajStrMatchCaseC(ensembltype, "Transcript"))
    {
        ajStrAssignC(&sqlfrom, "transcript");

        ajStrAssignC(&sqlwhere,
                     "transcript.is_current = 1 "
                     "AND "
                     "transcript.transcript_id = object_xref.ensembl_id");
    }
    else if(ajStrMatchCaseC(ensembltype, "Translation"))
    {
        ajStrAssignC(&sqlfrom, "transcript, translation");

        ajStrAssignC(&sqlwhere,
                     "transcript.is_current = 1 "
                     "AND "
                     "transcript.transcript_id = translation.transcript_id "
                     "AND "
                     "translation.translation_id = object_xref.ensembl_id");
    }

    if(dbname && ajStrGetLen(dbname))
    {
        /*
        ** Involve the 'external_db' table to limit the hits to a particular
        ** external database.
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
        "(xref.dbprimary_acc = '%s' OR xref.display_label = '%s')",
        sqlselect,
        sqlfrom,
        sqlwhere,
        ensembltype,
        txtname,
        txtname);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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
    ** If an external database name has been passed in, the 'xref' and the
    ** 'object_xref' tables need joining on 'xref_id'.
    */

    if(dbname && ajStrGetLen(dbname))
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
            "external_synonym.synonym = '%s' "
            "AND "
            "external_synonym.xref_id = object_xref.xref_id "
            "AND "
            "object_xref.ensembl_object_type = '%%S' "
            "AND "
            "object_xref.xref_id = xref.xref_id ",
            sqlselect,
            sqlfrom,
            sqlwhere,
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
            "external_synonym.synonym = '%s' "
            "AND "
            "external_synonym.xref_id = object_xref.xref_id "
            "AND "
            "object_xref.ensembl_object_type = '%S'",
            sqlselect,
            sqlfrom,
            sqlwhere,
            txtname,
            ensembltype);

    sqls = ensDatabaseadaptorSqlstatementNew(dbea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

    ajListSortUnique(idlist,
                     databaseentryadaptorCompareIdentifier,
                     databaseentryadaptorDeleteIdentifier);

    return ajTrue;
}




/* @funcstatic databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName *
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
** @@
** NOTE: This function requires an external database name and not an external
** databse identifier as the Perl API implementation.
******************************************************************************/

static AjBool databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
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

    if(!dbea)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!ensembltype)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    sqlselect = ajStrNewC("object_xref.ensembl_id");
    sqlfrom   = ajStrNew();
    sqlwhere  = ajStrNew();

    if(extratype && ajStrGetLen(extratype))
    {
        temporary = ajStrNewS(extratype);

        ajStrFmtLower(&temporary);

        if(ajStrMatchCaseC(temporary, "Translation"))
            ajStrAssignC(&sqlselect, "translation.translation_id");
        else
        {
            ajStrDel(&sqlselect);

            sqlselect = ajFmtStr("transcript.%S_id", temporary);
        }

        if(ajStrMatchCaseC(ensembltype, "Translation"))
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

    if(ajStrMatchC(ensembltype, "Gene"))
    {
        ajStrAssignC(&sqlfrom, "gene");

        ajStrAssignC(&sqlwhere,
                     "gene.is_current = 1 "
                     "AND "
                     "gene.gene_id = object_xref.ensembl_id");
    }
    else if(ajStrMatchCaseC(ensembltype, "Transcript"))
    {
        ajStrAssignC(&sqlfrom, "transcript");

        ajStrAssignC(&sqlwhere,
                     "transcript.is_current = 1 "
                     "AND "
                     "transcript.transcript_id = object_xref.ensembl_id");
    }
    else if(ajStrMatchCaseC(ensembltype, "Translation"))
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

    while(!ajSqlrowiterDone(sqli))
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
                     databaseentryadaptorCompareIdentifier,
                     databaseentryadaptorDeleteIdentifier);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternalName *********
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
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool value = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if(!dbea)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("gene");

    if(!databaseentryadaptorFetchAllIdentifiersByExternalName(
           dbea,
           name,
           ensembltype,
           extratype,
           dbname,
           idlist))
        value = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if(!databaseentryadaptorFetchAllIdentifiersByExternalName(
           dbea,
           name,
           ensembltype,
           extratype,
           dbname,
           idlist))
        value = ajFalse;

    ajStrAssignC(&ensembltype, "Gene");

    if(!databaseentryadaptorFetchAllIdentifiersByExternalName(
           dbea,
           name,
           ensembltype,
           (AjPStr) NULL,
           dbname,
           idlist))
        value = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     databaseentryadaptorCompareIdentifier,
                     databaseentryadaptorDeleteIdentifier);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternalName ***
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
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool value = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if(!dbea)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("transcript");

    if(!databaseentryadaptorFetchAllIdentifiersByExternalName(
           dbea,
           name,
           ensembltype,
           extratype,
           dbname,
           idlist))
        value = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if(!databaseentryadaptorFetchAllIdentifiersByExternalName(
           dbea,
           name,
           ensembltype,
           (AjPStr) NULL,
           dbname,
           idlist))
        value = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     databaseentryadaptorCompareIdentifier,
                     databaseentryadaptorDeleteIdentifier);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternalName **
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
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool value = AJTRUE;

    AjPStr ensembltype = NULL;

    if(!dbea)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    if(!databaseentryadaptorFetchAllIdentifiersByExternalName(
           dbea,
           name,
           ensembltype,
           (AjPStr) NULL,
           dbname,
           idlist))
        value = ajFalse;

    ajStrDel(&ensembltype);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternaldatabaseName *
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
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool value = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if(!dbea)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("gene");

    if(!databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
           dbea,
           dbname,
           ensembltype,
           extratype,
           idlist))
        value = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if(!databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
           dbea,
           dbname,
           ensembltype,
           extratype,
           idlist))
        value = ajFalse;

    ajStrAssignC(&ensembltype, "Gene");

    if(!databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
           dbea,
           dbname,
           ensembltype,
           (AjPStr) NULL,
           idlist))
        value = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     databaseentryadaptorCompareIdentifier,
                     databaseentryadaptorDeleteIdentifier);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternaldatabaseName
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
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool value = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if(!dbea)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    extratype = ajStrNewC("transcript");

    if(!databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
           dbea,
           dbname,
           ensembltype,
           extratype,
           idlist))
        value = ajFalse;

    ajStrAssignC(&ensembltype, "Transcript");

    if(!databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
           dbea,
           dbname,
           ensembltype,
           extratype,
           idlist))
        value = ajFalse;

    ajStrDel(&ensembltype);
    ajStrDel(&extratype);

    ajListSortUnique(idlist,
                     databaseentryadaptorCompareIdentifier,
                     databaseentryadaptorDeleteIdentifier);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternaldatabaseName
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
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist)
{
    AjBool value = AJTRUE;

    AjPStr ensembltype = NULL;
    AjPStr extratype   = NULL;

    if(!dbea)
        return ajFalse;

    if(!dbname)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    ensembltype = ajStrNewC("Translation");

    if(!databaseentryadaptorFetchAllIdentifiersByExternaldatabaseName(
           dbea,
           dbname,
           ensembltype,
           extratype,
           idlist))
        value = ajFalse;

    ajStrDel(&ensembltype);

    return value;
}




/* @func ensDatabaseentryadaptorFetchAllByDescription *************************
**
** Fetch all Ensembl Database Entries via a description.
**
** The caller is responsible for deleting the Ensembl Database Entries before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_description
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] description [const AjPStr] Description
** @param [rN] dbname [const AjPStr] External Database name
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllByDescription(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr description,
    const AjPStr dbname,
    AjPList dbes)
{
    char *txtdescription = NULL;
    char *txtdbname      = NULL;

    AjPStr statement = NULL;

    if(!dbea)
        return ajFalse;

    if (!(description && ajStrGetLen(description)))
        return ajFalse;

    if(!dbes)
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

    if(dbname && ajStrGetLen(dbname))
    {
        ensDatabaseadaptorEscapeC(dbea->Adaptor, &txtdbname, dbname);

        ajFmtPrintAppS(&statement, " AND exDB.db_name = '%s'", txtdbname);

        ajCharDel(&txtdbname);
    }

    databaseentryadaptorFetchAllBySQL(dbea, statement, dbes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensDatabaseentryadaptorFetchAllBySource ******************************
**
** Fetch all Ensembl Database Entries via a source.
**
** The caller is responsible for deleting the Ensembl Database Entries before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor::fetch_all_by_source
** @param [u] dbea [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @param [r] source [const AjPStr] Source
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon sucess, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseentryadaptorFetchAllBySource(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr source,
    AjPList dbes)
{
    char *txtsource = NULL;

    AjPStr statement = NULL;

    if(!dbea)
        return ajFalse;

    if (!(source && ajStrGetLen(source)))
        return ajFalse;

    if(!dbes)
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

    databaseentryadaptorFetchAllBySQL(dbea, statement, dbes);

    ajStrDel(&statement);

    return ajTrue;
}

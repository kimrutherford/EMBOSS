/* @source ensoperon **********************************************************
**
** Ensembl Operon functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.2 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:11 $ by $Author: mks $
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




#include "ensattribute.h"
#include "ensdatabaseentry.h"
#include "ensgene.h"
#include "ensoperon.h"




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




/* @conststatic operonadaptorKTablenames **************************************
**
** Array of Ensembl Operon Adaptor SQL table names
**
******************************************************************************/

static const char *const operonadaptorKTablenames[] =
{
    "operon",
    (const char *) NULL
};




/* @conststatic operonadaptorKColumnnames *************************************
**
** Array of Ensembl Operon Adaptor SQL column names
**
******************************************************************************/

static const char *const operonadaptorKColumnnames[] =
{
    "operon.operon_id",
    "operon.seq_region_id",
    "operon.seq_region_start",
    "operon.seq_region_end",
    "operon.seq_region_strand",
    "operon.analysis_id",
    "operon.display_label",
    "operon.stable_id",
    "operon.version",
    "operon.created_date",
    "operon.modified_date",
    (const char *) NULL
};




/* @conststatic operontranscriptadaptorKTablenames ****************************
**
** Array of Ensembl Operon Transcript Adaptor SQL table names
**
******************************************************************************/

static const char *const operontranscriptadaptorKTablenames[] =
{
    "operon_transcript",
    (const char *) NULL
};




/* @conststatic operontranscriptadaptorKColumnnames ***************************
**
** Array of Ensembl Operon Adaptor SQL column names
**
******************************************************************************/

static const char *const operontranscriptadaptorKColumnnames[] =
{
    "operon_transcript.operon_transcript_id",
    "operon_transcript.seq_region_id",
    "operon_transcript.seq_region_start",
    "operon_transcript.seq_region_end",
    "operon_transcript.seq_region_strand",
    "operon_transcript.analysis_id",
    "operon_transcript.display_label",
    "operon_transcript.stable_id",
    "operon_transcript.version",
    "operon_transcript.created_date",
    "operon_transcript.modified_date",
    "operon_transcript.operon_id",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




static AjBool operonadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList operons);

static AjBool operontranscriptadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ots);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensoperon *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPOperon] Ensembl Operon ***********************************
**
** @nam2rule Operon Functions for manipulating Ensembl Operon objects
**
** @cc Bio::EnsEMBL::Operon
** @cc CVS Revision: 1.2
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Operon by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Operon.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOperon]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy operon [const EnsPOperon] Ensembl Operon
** @argrule Ini opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini displaylabel [AjPStr] Display label
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Verion
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ref operon [EnsPOperon] Ensembl Operon
**
** @valrule * [EnsPOperon] Ensembl Operon or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOperonNewCpy ******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [EnsPOperon] Ensembl Operon or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperon ensOperonNewCpy(const EnsPOperon operon)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPOperon pthis = NULL;

    EnsPOperontranscript ot = NULL;

    if (!operon)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = operon->Identifier;

    pthis->Adaptor = operon->Adaptor;

    pthis->Feature = ensFeatureNewRef(operon->Feature);

    if (operon->Displaylabel)
        pthis->Displaylabel = ajStrNewRef(operon->Displaylabel);

    if (operon->DateCreation)
        pthis->DateCreation = ajStrNewRef(operon->DateCreation);

    if (operon->DateModification)
        pthis->DateModification = ajStrNewRef(operon->DateModification);

    if (operon->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(operon->Stableidentifier);

    pthis->Version = operon->Version;

    /* Copy the AJAX List of Ensembl Attribute objects. */

    if (operon->Attributes && ajListGetLength(operon->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(operon->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void *) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Database Entry objects. */

    if (operon->Databaseentries
        && ajListGetLength(operon->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(operon->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ajListPushAppend(pthis->Databaseentries,
                             (void *) ensDatabaseentryNewRef(dbe));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Databaseentries = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Operon Transcript objects. */

    if (operon->Operontranscripts
        && ajListGetLength(operon->Operontranscripts))
    {
        pthis->Operontranscripts = ajListNew();

        iter = ajListIterNew(operon->Operontranscripts);

        while (!ajListIterDone(iter))
        {
            ot = (EnsPOperontranscript) ajListIterGet(iter);

            ajListPushAppend(pthis->Operontranscripts,
                             (void *) ensOperontranscriptNewRef(ot));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Operontranscripts = NULL;

    return pthis;
}




/* @func ensOperonNewIni ******************************************************
**
** Constructor for an Ensembl Operon with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Operon::new
** @param [u] displaylabel [AjPStr] Display label
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
**
** @return [EnsPOperon] Ensembl Operon or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperon ensOperonNewIni(EnsPOperonadaptor opa,
                           ajuint identifier,
                           EnsPFeature feature,
                           AjPStr displaylabel,
                           AjPStr stableid,
                           ajuint version,
                           AjPStr cdate,
                           AjPStr mdate)
{
    EnsPOperon operon = NULL;

    if (!feature)
        return NULL;

    AJNEW0(operon);

    operon->Use = 1U;

    operon->Identifier = identifier;

    operon->Adaptor = opa;

    operon->Feature = ensFeatureNewRef(feature);

    if (displaylabel)
        operon->Displaylabel = ajStrNewRef(displaylabel);

    if (stableid)
        operon->Stableidentifier = ajStrNewRef(stableid);

    if (cdate)
        operon->DateCreation = ajStrNewRef(cdate);

    if (mdate)
        operon->DateModification = ajStrNewRef(mdate);

    operon->Version = version;

    operon->Attributes = NULL;

    operon->Databaseentries = NULL;

    operon->Operontranscripts = NULL;

    return operon;
}




/* @func ensOperonNewRef ******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] operon [EnsPOperon] Ensembl Operon
**
** @return [EnsPOperon] Ensembl Operon or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperon ensOperonNewRef(EnsPOperon operon)
{
    if (!operon)
        return NULL;

    operon->Use++;

    return operon;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Del Destroy (free) an Ensembl Operon
**
** @argrule * Poperon [EnsPOperon*] Ensembl Operon address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOperonDel *********************************************************
**
** Default destructor for an Ensembl Operon.
**
** @param [d] Poperon [EnsPOperon*] Ensembl Operon address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOperonDel(EnsPOperon *Poperon)
{
    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPOperon pthis = NULL;

    EnsPOperontranscript ot = NULL;

    if (!Poperon)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOperonDel"))
    {
        ajDebug("ensOperonDel\n"
                "  *Poperon %p\n",
                *Poperon);

        ensOperonTrace(*Poperon, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Poperon) || --pthis->Use)
    {
        *Poperon = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->Displaylabel);
    ajStrDel(&pthis->Stableidentifier);
    ajStrDel(&pthis->DateCreation);
    ajStrDel(&pthis->DateModification);

    /* Clear and delete the AJAX List of Ensembl Attribute objects. */

    while (ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while (ajListPop(pthis->Databaseentries, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Operon Transcript objects. */

    while (ajListPop(pthis->Operontranscripts, (void **) &ot))
        ensOperontranscriptDel(&ot);

    ajListFree(&pthis->Operontranscripts);

    ajMemFree((void **) Poperon);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Get Return Ensembl Operon attribute(s)
** @nam4rule Adaptor Return the Ensembl Operon Adaptor
** @nam4rule Date Return a date
** @nam5rule DateCreation Return the creation date
** @nam5rule DateModification Return the modification date
** @nam4rule Displaylabel Return the display label
** @nam4rule Feature Return the Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Stableidentifier Return the stable identifier
** @nam4rule Version Return the version
**
** @argrule * operon [const EnsPOperon] Ensembl Operon
**
** @valrule Adaptor [EnsPOperonadaptor] Ensembl Operon Adaptor or NULL
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Modification date or NULL
** @valrule Displaylabel [AjPStr] Display label or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Version [ajuint] Version or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensOperonGetAdaptor **************************************************
**
** Get the Ensembl Operon Adaptor member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [EnsPOperonadaptor] Ensembl Operon Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperonadaptor ensOperonGetAdaptor(
    const EnsPOperon operon)
{
    return (operon) ? operon->Adaptor : NULL;
}




/* @func ensOperonGetDateCreation *********************************************
**
** Get the creation date member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::created_date
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperonGetDateCreation(
    const EnsPOperon operon)
{
    return (operon) ? operon->DateCreation : NULL;
}




/* @func ensOperonGetDateModification *****************************************
**
** Get the modification date member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::modified_date
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [AjPStr] Modification date or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperonGetDateModification(
    const EnsPOperon operon)
{
    return (operon) ? operon->DateModification : NULL;
}




/* @func ensOperonGetDisplaylabel *********************************************
**
** Get the display label member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::display_label
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [AjPStr] Display label or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperonGetDisplaylabel(
    const EnsPOperon operon)
{
    return (operon) ? operon->Displaylabel : NULL;
}




/* @func ensOperonGetFeature **************************************************
**
** Get the Ensembl Feature member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Feature
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeature ensOperonGetFeature(
    const EnsPOperon operon)
{
    return (operon) ? operon->Feature : NULL;
}




/* @func ensOperonGetIdentifier ***********************************************
**
** Get the SQL database-internal identifier member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOperonGetIdentifier(
    const EnsPOperon operon)
{
    return (operon) ? operon->Identifier : 0U;
}




/* @func ensOperonGetStableidentifier *****************************************
**
** Get the stable identifier member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::stable_id
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [AjPStr] Stable identifier or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperonGetStableidentifier(
    const EnsPOperon operon)
{
    return (operon) ? operon->Stableidentifier : NULL;
}




/* @func ensOperonGetVersion **************************************************
**
** Get the version member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::version
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [ajuint] Version or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOperonGetVersion(
    const EnsPOperon operon)
{
    return (operon) ? operon->Version : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Operon object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPOperon]
**
** @nam3rule Load Return Ensembl Operon attribute(s) loaded on demand
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Operontranscripts Return all Ensembl Operon Transcript objects
**
** @argrule * operon [EnsPOperon] Ensembl Operon
**
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule Operontranscripts [const AjPList]
** AJAX List of Ensembl Operon Transcript objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOperonLoadAttributes **********************************************
**
** Load all Ensembl Attribute objects of an Ensembl Operon.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Attribute objects from the Ensembl SQL database associated with the
** Ensembl Operon Adaptor in case the internal AJAX List is not defined.
**
** To filter Ensembl Attribute objects via their code, consider using
** ensOperonFetchAllAttributes.
**
** @cc Bio::EnsEMBL::Operon::get_all_Attributes
** @param [u] operon [EnsPOperon] Ensembl Operon
** @see ensOperonFetchAllAttributes
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensOperonLoadAttributes(EnsPOperon operon)
{
    EnsPDatabaseadaptor dba = NULL;

    if (!operon)
        return NULL;

    if (operon->Attributes)
        return operon->Attributes;

    if (!operon->Adaptor)
    {
        ajDebug("ensOperonLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Operon without an "
                "Ensembl Operon Adaptor.\n");

        return NULL;
    }

    dba = ensOperonadaptorGetDatabaseadaptor(operon->Adaptor);

    if (!dba)
    {
        ajDebug("ensOperonLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Operon without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Operon Adaptor.\n");

        return NULL;
    }

    operon->Attributes = ajListNew();

    ensAttributeadaptorFetchAllbyOperon(
        ensRegistryGetAttributeadaptor(dba),
        operon,
        (const AjPStr) NULL,
        operon->Attributes);

    return operon->Attributes;
}




/* @func ensOperonLoadDatabaseentries *****************************************
**
** Load all Ensembl Database Entry objects of an Ensembl Operon.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Database Entry objects from the Ensembl SQL database associated
** with the Ensembl Operon Adaptor in case the internal
** AJAX List is not defined.
**
** To filter Ensembl Database Entry objects via an Ensembl External Database
** name or type, consider using ensOperonFetchAllDatabaseentries.
**
** @cc Bio::EnsEMBL::Operon::get_all_DBEntries
** @param [u] operon [EnsPOperon] Ensembl Operon
** @see ensOperonFetchAllDatabaseentries
**
** @return [const AjPList] AJAX List of Ensembl Database Entry objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensOperonLoadDatabaseentries(EnsPOperon operon)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!operon)
        return NULL;

    if (operon->Databaseentries)
        return operon->Databaseentries;

    if (!operon->Adaptor)
    {
        ajDebug("ensOperonLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an "
                "Ensembl Operon without an "
                "Ensembl Operon Adaptor.\n");

        return NULL;
    }

    dba = ensOperonadaptorGetDatabaseadaptor(operon->Adaptor);

    if (!dba)
    {
        ajDebug("ensOperonLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an "
                "Ensembl Operon without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Operon Adaptor.\n");

        return NULL;
    }

    objtype = ajStrNewC("Operon");

    operon->Databaseentries = ajListNew();

    ensDatabaseentryadaptorFetchAllbyObject(
        ensRegistryGetDatabaseentryadaptor(dba),
        operon->Identifier,
        objtype,
        (AjPStr) NULL,
        ensEExternaldatabaseTypeNULL,
        operon->Databaseentries);

    ajStrDel(&objtype);

    return operon->Databaseentries;
}




/* @func ensOperonLoadOperontranscripts ***************************************
**
** Load all Ensembl Operon Transcript objects of an Ensembl Operon.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Operon Transcript objects from the Ensembl SQL database associated
** with the Ensembl Operon Adaptor in case the internal
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Operon::get_all_OperonTranscripts
** @param [u] operon [EnsPOperon] Ensembl Operon
**
** @return [const AjPList]
** AJAX List of Ensembl Operon Transcript objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensOperonLoadOperontranscripts(EnsPOperon operon)
{
    EnsPDatabaseadaptor dba = NULL;

    if (!operon)
        return NULL;

    if (operon->Operontranscripts)
        return operon->Operontranscripts;

    if (!operon->Adaptor)
    {
        ajDebug("ensOperonLoadOperontranscripts cannot fetch "
                "Ensembl Operon Transcript objects for an "
                "Ensembl Operon without an "
                "Ensembl Operon Adaptor.\n");

        return NULL;
    }

    dba = ensOperonadaptorGetDatabaseadaptor(operon->Adaptor);

    if (!dba)
    {
        ajDebug("ensOperonLoadOperontranscripts cannot fetch "
                "Ensembl Operon Transcript objects for an "
                "Ensembl Operon without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Operon Adaptor.\n");

        return NULL;
    }

    operon->Operontranscripts = ajListNew();

    ensOperontranscriptadaptorFetchAllbyOperon(
        ensRegistryGetOperontranscriptadaptor(dba),
        operon,
        operon->Operontranscripts);

    return operon->Operontranscripts;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Set Set one member of an Ensembl Operon
** @nam4rule Adaptor Set the Ensembl Operon Adaptor
** @nam4rule Date Set a date
** @nam5rule Creation Set the date of creation
** @nam5rule Modification Set the date of modification
** @nam4rule Displaylabel Set the display label
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Stableidentifier Set the stable identifier
** @nam4rule Version Set the version
**
** @argrule * operon [EnsPOperon] Ensembl Operon object
** @argrule Adaptor opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule DateModification mdate [AjPStr] Modification date
** @argrule Displaylabel displaylabel [AjPStr] Display label
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Stableidentifier stableid [AjPStr] Stable identifier
** @argrule Version version [ajuint] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOperonSetAdaptor **************************************************
**
** Set the Ensembl Operon Adaptor member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetAdaptor(EnsPOperon operon, EnsPOperonadaptor opa)
{
    if (!operon)
        return ajFalse;

    operon->Adaptor = opa;

    return ajTrue;
}




/* @func ensOperonSetDateCreation *********************************************
**
** Set the date of creation member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::created_date
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetDateCreation(EnsPOperon operon, AjPStr cdate)
{
    if (!operon)
        return ajFalse;

    ajStrDel(&operon->DateCreation);

    operon->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensOperonSetDateModification *****************************************
**
** Set the date of modification member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::modified_date
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetDateModification(EnsPOperon operon, AjPStr mdate)
{
    if (!operon)
        return ajFalse;

    ajStrDel(&operon->DateModification);

    operon->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensOperonSetDisplaylabel *********************************************
**
** Set the display label member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::display_label
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] displaylabel [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetDisplaylabel(EnsPOperon operon, AjPStr displaylabel)
{
    if (!operon)
        return ajFalse;

    ajStrDel(&operon->Displaylabel);

    operon->Displaylabel = ajStrNewRef(displaylabel);

    return ajTrue;
}




/* @func ensOperonSetFeature **************************************************
**
** Set the Ensembl Feature member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Feature::slice
** @cc Bio::EnsEMBL::Feature::start
** @cc Bio::EnsEMBL::Feature::end
** @cc Bio::EnsEMBL::Feature::strand
** @cc Bio::EnsEMBL::Feature::move
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetFeature(EnsPOperon operon, EnsPFeature feature)
{
    if (ajDebugTest("ensOperonSetFeature"))
    {
        ajDebug("ensOperonSetFeature\n"
                "  operon %p\n"
                "  feature %p\n",
                operon,
                feature);

        ensOperonTrace(operon, 1);

        ensFeatureTrace(feature, 1);
    }

    if (!operon)
        return ajFalse;

    if (!feature)
        return ajFalse;

    /* Replace the current Feature. */

    ensFeatureDel(&operon->Feature);

    operon->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensOperonSetIdentifier ***********************************************
**
** Set the SQL database-internal identifier member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetIdentifier(EnsPOperon operon, ajuint identifier)
{
    if (!operon)
        return ajFalse;

    operon->Identifier = identifier;

    return ajTrue;
}




/* @func ensOperonSetStableidentifier *****************************************
**
** Set the stable identifier member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::stable_id
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetStableidentifier(EnsPOperon operon, AjPStr stableid)
{
    if (!operon)
        return ajFalse;

    ajStrDel(&operon->Stableidentifier);

    operon->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensOperonSetVersion **************************************************
**
** Set the version member of an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::version
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonSetVersion(EnsPOperon operon, ajuint version)
{
    if (!operon)
        return ajFalse;

    operon->Version = version;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Trace Report Ensembl Operon members to debug file
**
** @argrule Trace operon [const EnsPOperon] Ensembl Operon
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOperonTrace *******************************************************
**
** Trace an Ensembl Operon.
**
** @param [r] operon [const EnsPOperon] Ensembl Operon
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonTrace(const EnsPOperon operon, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPOperontranscript ot = NULL;

    if (!operon)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensOperonTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Displaylabel '%S'\n"
            "%S  Stableidentifier '%S'\n"
            "%S  DateCreation '%S'\n"
            "%S  DateModification '%S'\n"
            "%S  Version %u\n"
            "%S  Attributes %p\n"
            "%S  Databaseentries %p\n"
            "%S  Operontranscripts %p\n",
            indent, operon,
            indent, operon->Use,
            indent, operon->Identifier,
            indent, operon->Adaptor,
            indent, operon->Feature,
            indent, operon->Displaylabel,
            indent, operon->Stableidentifier,
            indent, operon->DateCreation,
            indent, operon->DateModification,
            indent, operon->Version,
            indent, operon->Attributes,
            indent, operon->Databaseentries,
            indent, operon->Operontranscripts);

    ensFeatureTrace(operon->Feature, level + 1);

    /* Trace the AJAX List of Ensembl Attribute objects. */

    if (operon->Attributes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Attribute objects\n",
                indent, operon->Attributes);

        iter = ajListIterNewread(operon->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if (operon->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entry objects\n",
                indent, operon->Databaseentries);

        iter = ajListIterNewread(operon->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Operon Transcript objects. */

    if (operon->Operontranscripts)
    {
        ajDebug("%S    AJAX List %p of Ensembl Operon Transcript objects\n",
                indent, operon->Operontranscripts);

        iter = ajListIterNewread(operon->Operontranscripts);

        while (!ajListIterDone(iter))
        {
            ot = (EnsPOperontranscript) ajListIterGet(iter);

            ensOperontranscriptTrace(ot, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Calculate Calculate Ensembl Operon information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize operon [const EnsPOperon] Ensembl Operon
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensOperonCalculateMemsize ********************************************
**
** Calculate the memory size in bytes of an Ensembl Operon.
**
** @param [r] operon [const EnsPOperon] Ensembl Operon
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensOperonCalculateMemsize(const EnsPOperon operon)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPOperontranscript ot = NULL;

    if (!operon)
        return 0;

    size += sizeof (EnsOOperon);

    size += ensFeatureCalculateMemsize(operon->Feature);

    if (operon->Displaylabel)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(operon->Displaylabel);
    }

    if (operon->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(operon->Stableidentifier);
    }

    if (operon->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(operon->DateCreation);
    }

    if (operon->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(operon->DateModification);
    }

    /* Summarise the AJAX List of Ensembl Attribute objects. */

    if (operon->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(operon->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if (operon->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(operon->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Operon Transcript objects. */

    if (operon->Operontranscripts)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(operon->Operontranscripts);

        while (!ajListIterDone(iter))
        {
            ot = (EnsPOperontranscript) ajListIterGet(iter);

            size += ensOperontranscriptCalculateMemsize(ot);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Add Add one object to an Ensembl Operon
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Databaseentry Add an Ensembl Database Entry
** @nam4rule Operontranscript Add an Ensembl Operon Transcript
**
** @argrule * operon [EnsPOperon] Ensembl Operon object
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Databaseentry dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Operontranscript ot [EnsPOperontranscript]
** Ensembl Operon Transcript
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOperonAddAttribute ************************************************
**
** Add an Ensembl Attribute to an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::add_Attribute
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonAddAttribute(EnsPOperon operon,
                             EnsPAttribute attribute)
{
    if (!operon)
        return ajFalse;

    if (!attribute)
        return ajFalse;

    if (!operon->Attributes)
        operon->Attributes = ajListNew();

    ajListPushAppend(operon->Attributes,
                     (void *) ensAttributeNewRef(attribute));

    return ajTrue;
}




/* @func ensOperonAddDatabaseentry ********************************************
**
** Add an Ensembl Database Entry to an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::add_DBEntry
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonAddDatabaseentry(EnsPOperon operon,
                                 EnsPDatabaseentry dbe)
{
    if (!operon)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!operon->Databaseentries)
        operon->Databaseentries = ajListNew();

    ajListPushAppend(operon->Databaseentries,
                     (void *) ensDatabaseentryNewRef(dbe));

    return ajTrue;
}




/* @func ensOperonAddOperontranscript *****************************************
**
** Add an Ensembl Operon Transcript to an Ensembl Operon.
**
** @cc Bio::EnsEMBL::Operon::add_OperonTranscript
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonAddOperontranscript(EnsPOperon operon,
                                    EnsPOperontranscript ot)
{
    if (!operon)
        return ajFalse;

    if (!ot)
        return ajFalse;

    if (!operon->Operontranscripts)
        operon->Operontranscripts = ajListNew();

    ajListPushAppend(operon->Operontranscripts,
                     (void *) ensOperontranscriptNewRef(ot));

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching information from an Ensembl Operon object.
**
** @fdata [EnsPOperon]
**
** @nam3rule Fetch Fetch Ensembl Operon information
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Databaseentries Fetch all Ensembl Database Entry objects
**
** @argrule AllAttributes operon [EnsPOperon] Ensembl Operon
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllDatabaseentries operon [EnsPOperon] Ensembl Operon
** @argrule AllDatabaseentries name [const AjPStr]
** Ensembl External Database name
** @argrule AllDatabaseentries type [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllDatabaseentries dbes [AjPList]
** AJAX List of Ensembl Database Entry objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOperonFetchAllAttributes ******************************************
**
** Fetch all Ensembl Attribute objects of an Ensembl Operon and optionally
** filter via an Ensembl Attribute code. To get all Ensembl Attribute objects
** for this Ensembl Operon, consider using ensOperonLoadAttributes.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Operon::get_all_Attributes
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
** @see ensOperonLoadAttributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonFetchAllAttributes(EnsPOperon operon,
                                   const AjPStr code,
                                   AjPList attributes)
{
    AjBool match = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPAttribute attribute = NULL;

    if (!operon)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    list = ensOperonLoadAttributes(operon);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if (code)
        {
            if (ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if (match)
            ajListPushAppend(attributes,
                             (void *) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensOperonFetchAllDatabaseentries *************************************
**
** Fetch all Ensembl Database Entry objects of an Ensembl Operon and
** optionally filter via an Ensembl External Database name or type. To get all
** Ensembl External Database objects for this Ensembl Operon, consider
** using ensOperonGetExternalDatabaseentries.
**
** The caller is responsible for deleting the Ensembl Database Entry objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Operon::get_all_DBEntries
** @param [u] operon [EnsPOperon] Ensembl Operon
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [u] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry object
** @see ensOperonLoadDatabaseentries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonFetchAllDatabaseentries(EnsPOperon operon,
                                        const AjPStr name,
                                        EnsEExternaldatabaseType type,
                                        AjPList dbes)
{
    AjBool namematch = AJFALSE;
    AjBool typematch = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPDatabaseentry dbe = NULL;

    if (!operon)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    list = ensOperonLoadDatabaseentries(operon);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        dbe = (EnsPDatabaseentry) ajListIterGet(iter);

        if (name)
        {
            if (ajStrMatchCaseS(name, ensDatabaseentryGetDbName(dbe)))
                namematch = ajTrue;
            else
                namematch = ajFalse;
        }
        else
            namematch = ajTrue;

        if (type)
        {
            if (type == ensDatabaseentryGetType(dbe))
                typematch = ajTrue;
            else
                typematch = ajFalse;
        }

        else
            typematch = ajTrue;

        if (namematch && typematch)
            ajListPushAppend(dbes, (void *) ensDatabaseentryNewRef(dbe));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [EnsPOperonadaptor] Ensembl Operon Adaptor ********************
**
** @nam2rule Operonadaptor Functions for manipulating
** Ensembl Operon Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor
** @cc CVS Revision: 1.7
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic operonadaptorFetchAllbyStatement *******************************
**
** Fetch all Ensembl Operon objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] operons [AjPList] AJAX List of Ensembl Operon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool operonadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList operons)
{
    ajuint identifier = 0U;
    ajuint analysisid = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    ajuint version = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr displaylabel = NULL;
    AjPStr stableid     = NULL;
    AjPStr cdate        = NULL;
    AjPStr mdate        = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPOperon operon     = NULL;
    EnsPOperonadaptor opa = NULL;

    if (ajDebugTest("operonadaptorFetchAllbyStatement"))
        ajDebug("operonadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  operons %p\n",
                ba,
                statement,
                am,
                slice,
                operons);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!operons)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    opa = ensRegistryGetOperonadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier   = 0U;
        srid         = 0U;
        srstart      = 0U;
        srend        = 0U;
        srstrand     = 0;
        analysisid   = 0U;
        displaylabel = ajStrNew();
        stableid     = ajStrNew();
        version      = 0U;
        cdate        = ajStrNew();
        mdate        = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &displaylabel);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&displaylabel);
            ajStrDel(&stableid);
            ajStrDel(&cdate);
            ajStrDel(&mdate);

            continue;
        }

        /* Finally, create a new Ensembl Operon. */

        operon = ensOperonNewIni(opa,
                                 identifier,
                                 feature,
                                 displaylabel,
                                 stableid,
                                 version,
                                 cdate,
                                 mdate);

        ajListPushAppend(operons, (void *) operon);

        ensFeatureDel(&feature);

        ajStrDel(&displaylabel);
        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Operon Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Operon Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPOperonadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPOperonadaptor] Ensembl Operon Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOperonadaptorNew **************************************************
**
** Default constructor for an Ensembl Operon Adaptor.
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
** @see ensRegistryGetOperonadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPOperonadaptor] Ensembl Operon Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperonadaptor ensOperonadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        operonadaptorKTablenames,
        operonadaptorKColumnnames,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &operonadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensOperonNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensOperonDel,
        (size_t (*)(const void *)) &ensOperonCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensOperonGetFeature,
        "Operon");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Operon Adaptor object.
**
** @fdata [EnsPOperonadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Operon Adaptor
**
** @argrule * Popa [EnsPOperonadaptor*] Ensembl Operon Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOperonadaptorDel **************************************************
**
** Default destructor for an Ensembl Operon Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Popa [EnsPOperonadaptor*] Ensembl Operon Adaptor address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOperonadaptorDel(EnsPOperonadaptor *Popa)
{
    ensFeatureadaptorDel(Popa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Operon Adaptor object.
**
** @fdata [EnsPOperonadaptor]
**
** @nam3rule Get Return Ensembl Operon Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * opa [EnsPOperonadaptor] Ensembl Operon Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOperonadaptorGetBaseadaptor ***************************************
**
** Get the Ensembl Base Adaptor member of an Ensembl Operon Adaptor.
**
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensOperonadaptorGetBaseadaptor(
    EnsPOperonadaptor opa)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensOperonadaptorGetFeatureadaptor(opa));
}




/* @func ensOperonadaptorGetDatabaseadaptor ***********************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Operon Adaptor.
**
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensOperonadaptorGetDatabaseadaptor(
    EnsPOperonadaptor opa)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensOperonadaptorGetFeatureadaptor(opa));
}




/* @func ensOperonadaptorGetFeatureadaptor ************************************
**
** Get the Ensembl Feature Adaptor member of an Ensembl Operon Adaptor.
**
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensOperonadaptorGetFeatureadaptor(
    EnsPOperonadaptor opa)
{
    return opa;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Operon objects from an
** Ensembl SQL database.
**
** @fdata [EnsPOperonadaptor]
**
** @nam3rule Fetch Fetch Ensembl Operon object(s)
** @nam4rule All   Fetch all Ensembl Operon objects
** @nam4rule Allby Fetch all Ensembl Operon objects matching a criterion
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable identifier
** @nam4rule By Fetch one Ensembl Operon object matching a criterion
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Operontranscriptidentifier
** Fetch by an Ensembl Operon Transcript identifier
** @nam5rule Operontranscriptstableidentifier
** Fetch by an Ensembl Operon Transcript stable identifier
** @nam5rule Stableidentifier Fetch by a stable identifier
**
** @argrule * opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @argrule All operons [AjPList] AJAX List of Ensembl Operon objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule Allby operons [AjPList] AJAX List of Ensembl Operon objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByOperontranscriptidentifier otidentifier [ajuint]
** Ensembl Operon Transcript identifier
** @argrule ByOperontranscriptstableidentifier stableid [const AjPStr]
** Ensembl Operon Transcript stable identifier
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule By Poperon [EnsPOperon*] Ensembl Operon address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensOperonadaptorFetchAll *********************************************
**
** Fetch all Ensembl Operon objects.
**
** The caller is responsible for deleting the Ensembl Operon objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::fetch_all
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [u] operons [AjPList] AJAX List of Ensembl Operon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorFetchAll(
    EnsPOperonadaptor opa,
    AjPList operons)
{
    return ensBaseadaptorFetchAll(
        ensOperonadaptorGetBaseadaptor(opa),
        operons);
}




/* @func ensOperonadaptorFetchByDisplaylabel **********************************
**
** Fetch an Ensembl Operon via its display label (name).
**
** The caller is responsible for deleting the Ensembl Operon.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::fetch_by_name
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [r] label [const AjPStr] Display label
** @param [wP] Poperon [EnsPOperon*] Ensembl Operon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorFetchByDisplaylabel(
    EnsPOperonadaptor opa,
    const AjPStr label,
    EnsPOperon *Poperon)
{
    char *txtlabel = NULL;

    AjBool result = AJFALSE;

    AjPList operons = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPOperon operon = NULL;

    if (!opa)
        return ajFalse;

    if (!label)
        return ajFalse;

    if (!Poperon)
        return ajFalse;

    ba = ensOperonadaptorGetBaseadaptor(opa);

    ensBaseadaptorEscapeC(ba, &txtlabel, label);

    constraint = ajFmtStr("operon.display_label = '%s'", txtlabel);

    ajCharDel(&txtlabel);

    operons = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        operons);

    if (ajListGetLength(operons) > 1)
        ajDebug("ensOperonadaptorFetchByDisplaylabel got more than "
                "one Operon for display label '%S'.\n", label);

    ajListPop(operons, (void **) Poperon);

    while (ajListPop(operons, (void **) &operon))
        ensOperonDel(&operon);

    ajListFree(&operons);

    ajStrDel(&constraint);

    return result;
}




/* @func ensOperonadaptorFetchByOperontranscriptidentifier ********************
**
** Fetch an Ensembl Operon via its Ensembl Operon Transcript identifier.
**
** The caller is responsible for deleting the Ensembl Operon.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::fetch_by_operon_transcript_id
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [r] otidentifier [ajuint] Ensembl Operon Transcript identifier
** @param [wP] Poperon [EnsPOperon*] Ensembl Operon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorFetchByOperontranscriptidentifier(
    EnsPOperonadaptor opa,
    ajuint otidentifier,
    EnsPOperon *Poperon)
{
    ajuint operonid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!opa)
        return ajFalse;

    if (!otidentifier)
        return ajFalse;

    if (!Poperon)
        return ajFalse;

    *Poperon = NULL;

    dba = ensOperonadaptorGetDatabaseadaptor(opa);

    statement = ajFmtStr(
        "SELECT "
        "operon_transcript.operon_id "
        "FROM "
        "operon_transcript "
        "WHERE "
        "operon_transcript.operon_transcript_id = %u",
        otidentifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        operonid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &operonid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!operonid)
    {
        ajDebug("ensOperonadaptorFetchByOperontranscriptidentifier "
                "could not get an Ensembl Operon identifier for "
                "Ensembl Operon Transcript identifier %u.\n", otidentifier);

        return ajFalse;
    }

    return ensBaseadaptorFetchByIdentifier(
        ensOperontranscriptadaptorGetBaseadaptor(opa),
        operonid,
        (void **) Poperon);
}




/* @func ensOperonadaptorFetchByOperontranscriptstableidentifier **************
**
** Fetch an Ensembl Operon via its
** Ensembl Operon Transcript stable identifier.
**
** The caller is responsible for deleting the Ensembl Operon.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::fetch_by_operon_transcript_stable_id
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [r] stableid [const AjPStr]
** Ensembl Operon Transcript stable identifier
** @param [wP] Poperon [EnsPOperon*] Ensembl Operon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorFetchByOperontranscriptstableidentifier(
    EnsPOperonadaptor opa,
    const AjPStr stableid,
    EnsPOperon *Poperon)
{
    char *txtstableid = NULL;

    ajuint operonid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!opa)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Poperon)
        return ajFalse;

    *Poperon = NULL;

    dba = ensOperonadaptorGetDatabaseadaptor(opa);

    ensDatabaseadaptorEscapeC(dba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "operon_transcript.operon_id "
        "FROM "
        "operon_transcript "
        "WHERE "
        "operon_transcript.stable_id = '%s'",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        operonid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &operonid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!operonid)
    {
        ajDebug("ensOperonadaptorFetchByOperontranscriptstableidentifier "
                "could not get an Ensembl Operon identifier for "
                "Ensembl Operon Transcript stable identifier '%S'.\n",
                stableid);

        return ajFalse;
    }

    return ensBaseadaptorFetchByIdentifier(
        ensOperontranscriptadaptorGetBaseadaptor(opa),
        operonid,
        (void **) Poperon);
}




/* @func ensOperonadaptorFetchByStableidentifier ******************************
**
** Fetch an Ensembl Operon via its stable identifier and version.
** In case a version is not specified, the current Ensembl Operon
** will be returned.
** The caller is responsible for deleting the Ensembl Operon.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::fetch_by_stable_id
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Poperon [EnsPOperon*] Ensembl Operon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorFetchByStableidentifier(
    EnsPOperonadaptor opa,
    const AjPStr stableid,
    ajuint version,
    EnsPOperon *Poperon)
{
    char *txtstableid = NULL;

    AjBool result = AJFALSE;

    AjPList operons = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPOperon operon = NULL;

    if (!opa)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Poperon)
        return ajFalse;

    *Poperon = NULL;

    ba = ensOperonadaptorGetBaseadaptor(opa);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if (version)
        constraint = ajFmtStr(
            "operon.stable_id = '%s' "
            "AND "
            "operon.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
#if AJFALSE
            "operon.stable_id = '%s' "
            "AND "
            "operon.is_current = 1",
#endif /* AJFALSE */
            "operon.stable_id = '%s'",
            txtstableid);

    ajCharDel(&txtstableid);

    operons = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        operons);

    if (ajListGetLength(operons) > 1)
        ajDebug("ensOperonadaptorFetchByStableidentifier got more than one "
                "Ensembl Operon for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(operons, (void **) Poperon);

    while (ajListPop(operons, (void **) &operon))
        ensOperonDel(&operon);

    ajListFree(&operons);

    ajStrDel(&constraint);

    return result;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Operon objects
** from an Ensembl SQL database.
**
** @fdata [EnsPOperonadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Operon-releated object(s)
** @nam4rule All Retrieve all Ensembl Operon-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifiers
** @nam5rule Stableidentifiers Retrieve all stable identifiers
**
** @argrule * opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Operon identifier) objects
** @argrule AllStableidentifiers stableids [AjPList]
** AJAX List of AJAX String (Ensembl Operon stable identifier) objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensOperonadaptorRetrieveAllIdentifiers *******************************
**
** Retrieve all SQL database-internal identifier objects of
** Ensembl Operon objects.
**
** The caller is responsible for deleting the AJAX unsigned integer objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::list_dbIDs
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [u] identifiers [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Operon identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorRetrieveAllIdentifiers(
    EnsPOperonadaptor opa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!opa)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("operon");

    result = ensBaseadaptorRetrieveAllIdentifiers(
        ensOperonadaptorGetBaseadaptor(opa),
        table,
        (AjPStr) NULL,
        identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensOperonadaptorRetrieveAllStableidentifiers *************************
**
** Retrieve all stable identifier objects of Ensembl Operon objects.
**
** The caller is responsible for deleting the AJAX String objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonAdaptor::list_stable_ids
** @param [u] opa [EnsPOperonadaptor] Ensembl Operon Adaptor
** @param [u] stableids [AjPList]
** AJAX List of AJAX String (Ensembl Operon stable identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperonadaptorRetrieveAllStableidentifiers(
    EnsPOperonadaptor opa,
    AjPList stableids)
{
    AjBool result = AJFALSE;

    AjPStr primary = NULL;
    AjPStr table   = NULL;

    if (!opa)
        return ajFalse;

    if (!stableids)
        return ajFalse;

    table   = ajStrNewC("operon");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(
        ensOperonadaptorGetBaseadaptor(opa),
        table,
        primary,
        stableids);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}




/* @datasection [EnsPOperontranscript] Ensembl Operon Transcript **************
**
** @nam2rule Operontranscript
** Functions for manipulating Ensembl Operon Transcript objects
**
** @cc Bio::EnsEMBL::OperonTranscript
** @cc CVS Revision: 1.4
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Operon Transcript by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Operon Transcript.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ot [const EnsPOperontranscript] Ensembl Operon Transcript
** @argrule Ini ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini displaylabel [AjPStr] Display label
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Verion
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ref ot [EnsPOperontranscript] Ensembl Operon Transcript
**
** @valrule * [EnsPOperontranscript] Ensembl Operon Transcript or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOperontranscriptNewCpy ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [EnsPOperontranscript] Ensembl Operon Transcript or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperontranscript ensOperontranscriptNewCpy(const EnsPOperontranscript ot)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene gene = NULL;

    EnsPOperontranscript pthis = NULL;

    if (!ot)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = ot->Identifier;

    pthis->Adaptor = ot->Adaptor;

    pthis->Feature = ensFeatureNewRef(ot->Feature);

    if (ot->Displaylabel)
        pthis->Displaylabel = ajStrNewRef(ot->Displaylabel);

    if (ot->DateCreation)
        pthis->DateCreation = ajStrNewRef(ot->DateCreation);

    if (ot->DateModification)
        pthis->DateModification = ajStrNewRef(ot->DateModification);

    if (ot->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(ot->Stableidentifier);

    pthis->Version = ot->Version;

    /* Copy the AJAX List of Ensembl Attribute objects. */

    if (ot->Attributes && ajListGetLength(ot->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(ot->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void *) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Database Entry objects. */

    if (ot->Databaseentries && ajListGetLength(ot->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(ot->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ajListPushAppend(pthis->Databaseentries,
                             (void *) ensDatabaseentryNewRef(dbe));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Databaseentries = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Gene objects. */

    if (ot->Genes && ajListGetLength(ot->Genes))
    {
        pthis->Genes = ajListNew();

        iter = ajListIterNew(ot->Genes);

        while (!ajListIterDone(iter))
        {
            gene = (EnsPGene) ajListIterGet(iter);

            ajListPushAppend(pthis->Genes,
                             (void *) ensGeneNewRef(gene));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Genes = NULL;

    return pthis;
}




/* @func ensOperontranscriptNewIni ********************************************
**
** Constructor for an Ensembl Operon Transcript with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::OperonTranscript::new
** @param [u] displaylabel [AjPStr] Display label
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
**
** @return [EnsPOperontranscript] Ensembl Operon Transcript or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperontranscript ensOperontranscriptNewIni(
    EnsPOperontranscriptadaptor ota,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr displaylabel,
    AjPStr stableid,
    ajuint version,
    AjPStr cdate,
    AjPStr mdate)
{
    EnsPOperontranscript ot = NULL;

    if (!feature)
        return NULL;

    AJNEW0(ot);

    ot->Use = 1U;

    ot->Identifier = identifier;

    ot->Adaptor = ota;

    ot->Feature = ensFeatureNewRef(feature);

    if (displaylabel)
        ot->Displaylabel = ajStrNewRef(displaylabel);

    if (stableid)
        ot->Stableidentifier = ajStrNewRef(stableid);

    if (cdate)
        ot->DateCreation = ajStrNewRef(cdate);

    if (mdate)
        ot->DateModification = ajStrNewRef(mdate);

    ot->Version = version;

    ot->Attributes = NULL;

    ot->Databaseentries = NULL;

    ot->Genes = NULL;

    return ot;
}




/* @func ensOperontranscriptNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [EnsPOperontranscript] Ensembl Operon Transcript or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperontranscript ensOperontranscriptNewRef(EnsPOperontranscript ot)
{
    if (!ot)
        return NULL;

    ot->Use++;

    return ot;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Del Destroy (free) an Ensembl Operon Transcript
**
** @argrule * Pot [EnsPOperontranscript*] Ensembl Operon Transcript address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOperontranscriptDel ***********************************************
**
** Default destructor for an Ensembl Operon Transcript.
**
** @param [d] Pot [EnsPOperontranscript*] Ensembl Operon Transcript address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOperontranscriptDel(EnsPOperontranscript *Pot)
{
    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene gene = NULL;

    EnsPOperontranscript pthis = NULL;

    if (!Pot)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOperontranscriptDel"))
    {
        ajDebug("ensOperontranscriptDel\n"
                "  *Pot %p\n",
                *Pot);

        ensOperontranscriptTrace(*Pot, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pot) || --pthis->Use)
    {
        *Pot = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->Displaylabel);
    ajStrDel(&pthis->Stableidentifier);
    ajStrDel(&pthis->DateCreation);
    ajStrDel(&pthis->DateModification);

    /* Clear and delete the AJAX List of Ensembl Attribute objects. */

    while (ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while (ajListPop(pthis->Databaseentries, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Gene objects. */

    while (ajListPop(pthis->Genes, (void **) &gene))
        ensGeneDel(&gene);

    ajListFree(&pthis->Genes);

    ajMemFree((void **) Pot);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Get Return Ensembl Operon Transcript attribute(s)
** @nam4rule Adaptor Return the Ensembl Operon Transcript Adaptor
** @nam4rule Date Return a date
** @nam5rule DateCreation Return the creation date
** @nam5rule DateModification Return the modification date
** @nam4rule Displaylabel Return the display label
** @nam4rule Feature Return the Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Stableidentifier Return the stable identifier
** @nam4rule Version Return the version
**
** @argrule * ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @valrule Adaptor [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor or NULL
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Modification date or NULL
** @valrule Displaylabel [AjPStr] Display label or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Version [ajuint] Version or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensOperontranscriptGetAdaptor ****************************************
**
** Get the Ensembl Operon Transcript Adaptor member of an
** Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperontranscriptadaptor ensOperontranscriptGetAdaptor(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->Adaptor : NULL;
}




/* @func ensOperontranscriptGetDateCreation ***********************************
**
** Get the creation date member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::created_date
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperontranscriptGetDateCreation(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->DateCreation : NULL;
}




/* @func ensOperontranscriptGetDateModification *******************************
**
** Get the modification date member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::modified_date
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [AjPStr] Modification date or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperontranscriptGetDateModification(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->DateModification : NULL;
}




/* @func ensOperontranscriptGetDisplaylabel ***********************************
**
** Get the display label member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::display_label
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [AjPStr] Display label or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperontranscriptGetDisplaylabel(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->Displaylabel : NULL;
}




/* @func ensOperontranscriptGetFeature ****************************************
**
** Get the Ensembl Feature member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::Feature
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeature ensOperontranscriptGetFeature(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->Feature : NULL;
}




/* @func ensOperontranscriptGetIdentifier *************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOperontranscriptGetIdentifier(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->Identifier : 0U;
}




/* @func ensOperontranscriptGetStableidentifier *****************************************
**
** Get the stable identifier member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::stable_id
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [AjPStr] Stable identifier or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOperontranscriptGetStableidentifier(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->Stableidentifier : NULL;
}




/* @func ensOperontranscriptGetVersion ****************************************
**
** Get the version member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::version
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [ajuint] Version or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOperontranscriptGetVersion(
    const EnsPOperontranscript ot)
{
    return (ot) ? ot->Version : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Operon Transcript object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Load
** Return Ensembl Operon Transcript attribute(s) loaded on demand
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Genes Return all Ensembl Gene objects
**
** @argrule * ot [EnsPOperontranscript] Ensembl Operon Transcript
**
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule Genes [const AjPList]
** AJAX List of Ensembl Gene objects
**
** @fcategory use
******************************************************************************/




/* @func ensOperontranscriptLoadAttributes ************************************
**
** Load all Ensembl Attribute objects of an Ensembl Operon Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Attribute objects from the Ensembl SQL database associated with the
** Ensembl Operon Transcript Adaptor in case the internal
** AJAX List is not defined.
**
** To filter Ensembl Attribute objects via their code, consider using
** ensOperontranscriptFetchAllAttributes.
**
** @cc Bio::EnsEMBL::OperonTranscript::get_all_Attributes
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @see ensOperontranscriptFetchAllAttributes
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensOperontranscriptLoadAttributes(EnsPOperontranscript ot)
{
    EnsPDatabaseadaptor dba = NULL;

    if (!ot)
        return NULL;

    if (ot->Attributes)
        return ot->Attributes;

    if (!ot->Adaptor)
    {
        ajDebug("ensOperontranscriptLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Operon Transcript without an "
                "Ensembl Operon Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensOperontranscriptadaptorGetDatabaseadaptor(ot->Adaptor);

    if (!dba)
    {
        ajDebug("ensOperontranscriptLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Operon Transcript without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Operon Transcript Adaptor.\n");

        return NULL;
    }

    ot->Attributes = ajListNew();

    ensAttributeadaptorFetchAllbyOperontranscript(
        ensRegistryGetAttributeadaptor(dba),
        ot,
        (const AjPStr) NULL,
        ot->Attributes);

    return ot->Attributes;
}




/* @func ensOperontranscriptLoadDatabaseentries *******************************
**
** Load all Ensembl Database Entry objects of an Ensembl Operon Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Database Entry objects from the Ensembl SQL database associated
** with the Ensembl Operon Transcript Adaptor in case the internal
** AJAX List is not defined.
**
** To filter Ensembl Database Entry objects via an Ensembl External Database
** name or type, consider using ensOperontranscriptFetchAllDatabaseentries.
**
** @cc Bio::EnsEMBL::OperonTranscript::get_all_DBEntries
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @see ensOperontranscriptFetchAllDatabaseentries
**
** @return [const AjPList] AJAX List of Ensembl Database Entry objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensOperontranscriptLoadDatabaseentries(EnsPOperontranscript ot)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ot)
        return NULL;

    if (ot->Databaseentries)
        return ot->Databaseentries;

    if (!ot->Adaptor)
    {
        ajDebug("ensOperontranscriptLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an "
                "Ensembl Operon Transcript without an "
                "Ensembl Operon Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensOperontranscriptadaptorGetDatabaseadaptor(ot->Adaptor);

    if (!dba)
    {
        ajDebug("ensOperontranscriptLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an "
                "Ensembl Operon Transcript without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Operon Transcript Adaptor.\n");

        return NULL;
    }

    objtype = ajStrNewC("OperonTranscript");

    ot->Databaseentries = ajListNew();

    ensDatabaseentryadaptorFetchAllbyObject(
        ensRegistryGetDatabaseentryadaptor(dba),
        ot->Identifier,
        objtype,
        (AjPStr) NULL,
        ensEExternaldatabaseTypeNULL,
        ot->Databaseentries);

    ajStrDel(&objtype);

    return ot->Databaseentries;
}




/* @func ensOperontranscriptLoadGenes *****************************************
**
** Load all Ensembl Gene objects of an Ensembl Operon Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Gene objects from the Ensembl SQL database associated
** with the Ensembl Operon Transcript Adaptor in case the internal
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::OperonTranscript::get_all_Genes
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [const AjPList] AJAX List of Ensembl Gene objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensOperontranscriptLoadGenes(EnsPOperontranscript ot)
{
    EnsPDatabaseadaptor dba = NULL;

    if (!ot)
        return NULL;

    if (ot->Genes)
        return ot->Genes;

    if (!ot->Adaptor)
    {
        ajDebug("ensOperontranscriptLoadGenes cannot fetch "
                "Ensembl Gene objects for an "
                "Ensembl Operon Transcript without an "
                "Ensembl Operon Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensOperontranscriptadaptorGetDatabaseadaptor(ot->Adaptor);

    if (!dba)
    {
        ajDebug("ensOperontranscriptLoadGenes cannot fetch "
                "Ensembl Gene objects for an "
                "Ensembl Operon Transcript without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Operon Transcript Adaptor.\n");

        return NULL;
    }

    ot->Genes = ajListNew();

    ensOperontranscriptadaptorRetrieveAllGenesByOperontranscriptidentifier(
        ot->Adaptor,
        ot->Identifier,
        ot->Genes);

    return ot->Genes;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Set Set one member of an Ensembl Operon Transcript
** @nam4rule Adaptor Set the Ensembl Operon Transcript Adaptor
** @nam4rule Date Set a date
** @nam5rule Creation Set the date of creation
** @nam5rule Modification Set the date of modification
** @nam4rule Displaylabel Set the display label
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Stableidentifier Set the stable identifier
** @nam4rule Version Set the version
**
** @argrule * ot [EnsPOperontranscript] Ensembl Operon Transcript object
** @argrule Adaptor ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule DateModification mdate [AjPStr] Modification date
** @argrule Displaylabel displaylabel [AjPStr] Display label
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Stableidentifier stableid [AjPStr] Stable identifier
** @argrule Version version [ajuint] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOperontranscriptSetAdaptor ****************************************
**
** Set the Ensembl Operon Transcript Adaptor member of an
** Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetAdaptor(EnsPOperontranscript ot,
                                     EnsPOperontranscriptadaptor ota)
{
    if (!ot)
        return ajFalse;

    ot->Adaptor = ota;

    return ajTrue;
}




/* @func ensOperontranscriptSetDateCreation ***********************************
**
** Set the date of creation member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::created_date
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetDateCreation(EnsPOperontranscript ot,
                                          AjPStr cdate)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->DateCreation);

    ot->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensOperontranscriptSetDateModification *******************************
**
** Set the date of modification member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::modified_date
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetDateModification(EnsPOperontranscript ot,
                                              AjPStr mdate)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->DateModification);

    ot->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensOperontranscriptSetDisplaylabel ***********************************
**
** Set the display label member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::display_label
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] displaylabel [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetDisplaylabel(EnsPOperontranscript ot,
                                          AjPStr displaylabel)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->Displaylabel);

    ot->Displaylabel = ajStrNewRef(displaylabel);

    return ajTrue;
}




/* @func ensOperontranscriptSetFeature ****************************************
**
** Set the Ensembl Feature member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::Feature::slice
** @cc Bio::EnsEMBL::Feature::start
** @cc Bio::EnsEMBL::Feature::end
** @cc Bio::EnsEMBL::Feature::strand
** @cc Bio::EnsEMBL::Feature::move
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetFeature(EnsPOperontranscript ot,
                                     EnsPFeature feature)
{
    if (ajDebugTest("ensOperontranscriptSetFeature"))
    {
        ajDebug("ensOperontranscriptSetFeature\n"
                "  ot %p\n"
                "  feature %p\n",
                ot,
                feature);

        ensOperontranscriptTrace(ot, 1);

        ensFeatureTrace(feature, 1);
    }

    if (!ot)
        return ajFalse;

    if (!feature)
        return ajFalse;

    /* Replace the current Feature. */

    ensFeatureDel(&ot->Feature);

    ot->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensOperontranscriptSetIdentifier *************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetIdentifier(EnsPOperontranscript ot,
                                        ajuint identifier)
{
    if (!ot)
        return ajFalse;

    ot->Identifier = identifier;

    return ajTrue;
}




/* @func ensOperontranscriptSetStableidentifier *******************************
**
** Set the stable identifier member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::stable_id
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetStableidentifier(EnsPOperontranscript ot,
                                              AjPStr stableid)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->Stableidentifier);

    ot->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensOperontranscriptSetVersion ****************************************
**
** Set the version member of an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::version
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptSetVersion(EnsPOperontranscript ot,
                                     ajuint version)
{
    if (!ot)
        return ajFalse;

    ot->Version = version;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Trace Report Ensembl Operon Transcript members to debug file
**
** @argrule Trace ot [const EnsPOperontranscript] Ensembl Operon Transcript
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOperontranscriptTrace *********************************************
**
** Trace an Ensembl Operon Transcript.
**
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptTrace(const EnsPOperontranscript ot, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene gene = NULL;

    if (!ot)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensOperontranscriptTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Displaylabel '%S'\n"
            "%S  Stableidentifier '%S'\n"
            "%S  DateCreation '%S'\n"
            "%S  DateModification '%S'\n"
            "%S  Version %u\n"
            "%S  Attributes %p\n"
            "%S  Databaseentries %p\n"
            "%S  Genes %p\n",
            indent, ot,
            indent, ot->Use,
            indent, ot->Identifier,
            indent, ot->Adaptor,
            indent, ot->Feature,
            indent, ot->Displaylabel,
            indent, ot->Stableidentifier,
            indent, ot->DateCreation,
            indent, ot->DateModification,
            indent, ot->Version,
            indent, ot->Attributes,
            indent, ot->Databaseentries,
            indent, ot->Genes);

    ensFeatureTrace(ot->Feature, level + 1);

    /* Trace the AJAX List of Ensembl Attribute objects. */

    if (ot->Attributes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Attribute objects\n",
                indent, ot->Attributes);

        iter = ajListIterNewread(ot->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if (ot->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entry objects\n",
                indent, ot->Databaseentries);

        iter = ajListIterNewread(ot->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Gene objects. */

    if (ot->Genes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Gene objects\n",
                indent, ot->Genes);

        iter = ajListIterNewread(ot->Genes);

        while (!ajListIterDone(iter))
        {
            gene = (EnsPGene) ajListIterGet(iter);

            ensGeneTrace(gene, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Calculate Calculate Ensembl Operon Transcript information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensOperontranscriptCalculateMemsize **********************************
**
** Calculate the memory size in bytes of an Ensembl Operon Transcript.
**
** @param [r] ot [const EnsPOperontranscript] Ensembl Operon Transcript
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensOperontranscriptCalculateMemsize(const EnsPOperontranscript ot)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene gene = NULL;

    if (!ot)
        return 0;

    size += sizeof (EnsOOperontranscript);

    size += ensFeatureCalculateMemsize(ot->Feature);

    if (ot->Displaylabel)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->Displaylabel);
    }

    if (ot->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->Stableidentifier);
    }

    if (ot->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->DateCreation);
    }

    if (ot->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->DateModification);
    }

    /* Summarise the AJAX List of Ensembl Attribute objects. */

    if (ot->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(ot->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if (ot->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(ot->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Gene objects. */

    if (ot->Genes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(ot->Genes);

        while (!ajListIterDone(iter))
        {
            gene = (EnsPGene) ajListIterGet(iter);

            size += ensGeneCalculateMemsize(gene);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Add Add one object to an Ensembl Operon Transcript
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Databaseentry Add an Ensembl Database Entry
** @nam4rule Gene Add an Ensembl Gene
**
** @argrule * ot [EnsPOperontranscript] Ensembl Operon Transcript object
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Databaseentry dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Gene gene [EnsPGene] Ensembl Gene
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOperontranscriptAddAttribute **************************************
**
** Add an Ensembl Attribute to an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::add_Attribute
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptAddAttribute(EnsPOperontranscript ot,
                                       EnsPAttribute attribute)
{
    if (!ot)
        return ajFalse;

    if (!attribute)
        return ajFalse;

    if (!ot->Attributes)
        ot->Attributes = ajListNew();

    ajListPushAppend(ot->Attributes,
                     (void *) ensAttributeNewRef(attribute));

    return ajTrue;
}




/* @func ensOperontranscriptAddDatabaseentry **********************************
**
** Add an Ensembl Database Entry to an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::add_DBEntry
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptAddDatabaseentry(EnsPOperontranscript ot,
                                           EnsPDatabaseentry dbe)
{
    if (!ot)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!ot->Databaseentries)
        ot->Databaseentries = ajListNew();

    ajListPushAppend(ot->Databaseentries,
                     (void *) ensDatabaseentryNewRef(dbe));

    return ajTrue;
}




/* @func ensOperontranscriptAddGene *******************************************
**
** Add an Ensembl Gene to an Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::OperonTranscript::add_Gene
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptAddGene(EnsPOperontranscript ot,
                                  EnsPGene gene)
{
    if (!ot)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!ot->Genes)
        ot->Genes = ajListNew();

    ajListPushAppend(ot->Genes,
                     (void *) ensGeneNewRef(gene));

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching information from an Ensembl Operon Transcript object.
**
** @fdata [EnsPOperontranscript]
**
** @nam3rule Fetch Fetch Ensembl Operon Transcript information
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Databaseentries Fetch all Ensembl Database Entry objects
** @nam4rule Operon Fetch an Ensembl Operon
**
** @argrule AllAttributes ot [EnsPOperontranscript]
** Ensembl Operon Transcript
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllDatabaseentries ot [EnsPOperontranscript]
** Ensembl Operon Transcript
** @argrule AllDatabaseentries name [const AjPStr]
** Ensembl External Database name
** @argrule AllDatabaseentries type [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllDatabaseentries dbes [AjPList]
** AJAX List of Ensembl Database Entry objects
** @argrule Operon ot [EnsPOperontranscript] Ensembl Operon Transcript
** @argrule Operon Poperon [EnsPOperon*] Ensembl Operon address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOperontranscriptFetchAllAttributes ********************************
**
** Fetch all Ensembl Attribute objects of an Ensembl Operon Transcript and
** optionally filter via an Ensembl Attribute code. To get all
** Ensembl Attribute objects for this Ensembl Operon Transcript,
** consider using ensOperontranscriptLoadAttributes.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::OperonTranscript::get_all_Attributes
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
** @see ensOperontranscriptLoadAttributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptFetchAllAttributes(
    EnsPOperontranscript ot,
    const AjPStr code,
    AjPList attributes)
{
    AjBool match = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPAttribute attribute = NULL;

    if (!ot)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    list = ensOperontranscriptLoadAttributes(ot);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if (code)
        {
            if (ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if (match)
            ajListPushAppend(attributes,
                             (void *) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensOperontranscriptFetchAllDatabaseentries ***************************
**
** Fetch all Ensembl Database Entry objects of an Ensembl Operon Transcript and
** optionally filter via an Ensembl External Database name or type. To get all
** Ensembl External Database objects for this Ensembl Operon Transcript,
** consider using ensOperontranscriptGetExternalDatabaseentries.
**
** The caller is responsible for deleting the Ensembl Database Entry objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::OperonTranscript::get_all_DBEntries
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [u] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry object
** @see ensOperontranscriptLoadDatabaseentries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptFetchAllDatabaseentries(
    EnsPOperontranscript ot,
    const AjPStr name,
    EnsEExternaldatabaseType type,
    AjPList dbes)
{
    AjBool namematch = AJFALSE;
    AjBool typematch = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPDatabaseentry dbe = NULL;

    if (!ot)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    list = ensOperontranscriptLoadDatabaseentries(ot);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        dbe = (EnsPDatabaseentry) ajListIterGet(iter);

        if (name)
        {
            if (ajStrMatchCaseS(name, ensDatabaseentryGetDbName(dbe)))
                namematch = ajTrue;
            else
                namematch = ajFalse;
        }
        else
            namematch = ajTrue;

        if (type)
        {
            if (type == ensDatabaseentryGetType(dbe))
                typematch = ajTrue;
            else
                typematch = ajFalse;
        }

        else
            typematch = ajTrue;

        if (namematch && typematch)
            ajListPushAppend(dbes, (void *) ensDatabaseentryNewRef(dbe));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensOperontranscriptFetchOperon ***************************************
**
** Fetch the Ensembl Operon for an Ensembl Operon Transcript.
**
** The caller is responsible for deleting the Ensembl Operon.
**
** @cc Bio::EnsEMBL::OperonTranscript::operon
** @param [u] ot [EnsPOperontranscript] Ensembl Operon Transcript
** @param [wP] Poperon [EnsPOperon*] Ensembl Operon
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
** NOTE: Since the Ensembl Operon already caches Ensembl Operon Transcript
** objects and to avoid circular references, the Ensembl Operon is not
** cached in the Ensembl Operon Transcript object.
******************************************************************************/

AjBool ensOperontranscriptFetchOperon(
    EnsPOperontranscript ot,
    EnsPOperon *Poperon)
{
    if (!ot)
        return ajFalse;

    if (!Poperon)
        return ajFalse;

    return ensOperonadaptorFetchByOperontranscriptidentifier(
        ensRegistryGetOperonadaptor(
            ensOperontranscriptadaptorGetDatabaseadaptor(ot->Adaptor)),
        ensOperontranscriptGetIdentifier(ot),
        Poperon);
}




/* @datasection [EnsPOperontranscriptadaptor] Ensembl Operon Transcript Adaptor
**
** @nam2rule Operontranscriptadaptor Functions for manipulating
** Ensembl Operon Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor
** @cc CVS Revision: 1.7
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic operontranscriptadaptorFetchAllbyStatement *********************
**
** Fetch all Ensembl Operon Transcript objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] ots [AjPList] AJAX List of Ensembl Operon Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool operontranscriptadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ots)
{
    ajuint identifier = 0U;
    ajuint analysisid = 0U;
    ajuint operonid   = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    ajuint version = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr displaylabel = NULL;
    AjPStr stableid     = NULL;
    AjPStr cdate        = NULL;
    AjPStr mdate        = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPOperontranscript        ot  = NULL;
    EnsPOperontranscriptadaptor ota = NULL;

    if (ajDebugTest("operontranscriptadaptorFetchAllbyStatement"))
        ajDebug("operontranscriptadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  ots %p\n",
                ba,
                statement,
                am,
                slice,
                ots);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!ots)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ota = ensRegistryGetOperontranscriptadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier   = 0U;
        srid         = 0U;
        srstart      = 0U;
        srend        = 0U;
        srstrand     = 0;
        analysisid   = 0U;
        displaylabel = ajStrNew();
        stableid     = ajStrNew();
        version      = 0U;
        cdate        = ajStrNew();
        mdate        = ajStrNew();
        operonid     = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &displaylabel);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);
        ajSqlcolumnToUint(sqlr, &operonid);

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&displaylabel);
            ajStrDel(&stableid);
            ajStrDel(&cdate);
            ajStrDel(&mdate);

            continue;
        }

        /* Finally, create a new Ensembl Operon. */

        ot = ensOperontranscriptNewIni(ota,
                                       identifier,
                                       feature,
                                       displaylabel,
                                       stableid,
                                       version,
                                       cdate,
                                       mdate);

        ajListPushAppend(ots, (void *) ot);

        ensFeatureDel(&feature);

        ajStrDel(&displaylabel);
        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Operon Transcript Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Operon Transcript Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPOperontranscriptadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOperontranscriptadaptorNew ****************************************
**
** Default constructor for an Ensembl Operon Transcript Adaptor.
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
** @see ensRegistryGetOperontranscriptadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOperontranscriptadaptor ensOperontranscriptadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        operontranscriptadaptorKTablenames,
        operontranscriptadaptorKColumnnames,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &operontranscriptadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensOperontranscriptNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensOperontranscriptDel,
        (size_t (*)(const void *)) &ensOperontranscriptCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensOperontranscriptGetFeature,
        "Operontranscript");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Operon Transcript Adaptor object.
**
** @fdata [EnsPOperontranscriptadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Operon Transcript Adaptor
**
** @argrule * Pota [EnsPOperontranscriptadaptor*]
** Ensembl Operon Transcript Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOperontranscriptadaptorDel ****************************************
**
** Default destructor for an Ensembl Operon Transcript Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pota [EnsPOperontranscriptadaptor*]
** Ensembl Operon Transcript Adaptor address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOperontranscriptadaptorDel(EnsPOperontranscriptadaptor *Pota)
{
    ensFeatureadaptorDel(Pota);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Operon Transcript Adaptor object.
**
** @fdata [EnsPOperontranscriptadaptor]
**
** @nam3rule Get Return Ensembl Operon Transcript Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOperontranscriptadaptorGetBaseadaptor *****************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Operon Transcript Adaptor.
**
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensOperontranscriptadaptorGetBaseadaptor(
    EnsPOperontranscriptadaptor ota)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensOperontranscriptadaptorGetFeatureadaptor(ota));
}




/* @func ensOperontranscriptadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Operon Transcript Adaptor.
**
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensOperontranscriptadaptorGetDatabaseadaptor(
    EnsPOperontranscriptadaptor ota)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensOperontranscriptadaptorGetFeatureadaptor(ota));
}




/* @func ensOperontranscriptadaptorGetFeatureadaptor **************************
**
** Get the Ensembl Feature Adaptor member of an
** Ensembl Operon Transcript Adaptor.
**
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensOperontranscriptadaptorGetFeatureadaptor(
    EnsPOperontranscriptadaptor ota)
{
    return ota;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Operon Transcript objects from an
** Ensembl SQL database.
**
** @fdata [EnsPOperontranscriptadaptor]
**
** @nam3rule Fetch Fetch Ensembl Operon Transcript object(s)
** @nam4rule All   Fetch all Ensembl Operon Transcript objects
** @nam4rule Allby Fetch all Ensembl Operon Transcript objects
**                 matching a criterion
** @nam5rule Gene Fetch all by an Ensembl Gene
** @nam5rule Operon Fetch all by an Ensembl Operon
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable identifier
** @nam4rule By Fetch one Ensembl Operon Transcript object matching a criterion
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by a stable identifier
**
** @argrule * ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @argrule All ots [AjPList] AJAX List of Ensembl Operon Transcript objects
** @argrule AllbyGene gene [const EnsPGene] Ensembl Gene
** @argrule AllbyOperon operon [const EnsPOperon] Ensembl Operon
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule Allby ots [AjPList] AJAX List of Ensembl Operon Transcript objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule By Pot [EnsPOperontranscript*]
** Ensembl Operon Transcript address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensOperontranscriptadaptorFetchAllbyGene *****************************
**
** Fetch all Ensembl Operon Transcript objects by an Ensembl Gene.
**
** The caller is responsible for deleting the Ensembl Operon Transcript
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::
**     fetch_all_by_gene_id
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::
**     fetch_all_by_gene
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] ots [AjPList]
** AJAX List of Ensembl Operon Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorFetchAllbyGene(
    EnsPOperontranscriptadaptor ota,
    const EnsPGene gene,
    AjPList ots)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOperontranscript ot = NULL;

    if (!ota)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!ots)
        return ajFalse;

    dba = ensOperontranscriptadaptorGetDatabaseadaptor(ota);

    statement = ajFmtStr(
        "SELECT "
        "operon_transcript_gene.operon_transcript_id "
        "FROM "
        "operon_transcript_gene "
        "WHERE "
        "operon_transcript_gene.gene_id = %u",
        ensGeneGetIdentifier(gene));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);

        ensOperontranscriptadaptorFetchByIdentifier(ota, identifier, &ot);

        if (ot)
            ajListPushAppend(ots, (void *) ot);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensOperontranscriptadaptorFetchAllbyOperon ***************************
**
** Fetch all Ensembl Operon Transcript objects by an Ensembl Operon.
**
** The caller is responsible for deleting the Ensembl Operon Transcript
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::fetch_all_by_Operon
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] operon [const EnsPOperon] Ensembl Operon
** @param [u] ots [AjPList] AJAX List of Ensembl Operon Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorFetchAllbyOperon(
    EnsPOperontranscriptadaptor ota,
    const EnsPOperon operon,
    AjPList ots)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    constraint = ajFmtStr("operon_transcript.operon_id = %u",
                          ensOperonGetIdentifier(operon));

    result = ensBaseadaptorFetchAllbyConstraint(
        ensOperontranscriptadaptorGetBaseadaptor(ota),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        ots);

    ajStrDel(&constraint);

    return result;
}




/* @func ensOperontranscriptadaptorFetchByDisplaylabel ************************
**
** Fetch an Ensembl Operon Transcript via its display label (name).
**
** The caller is responsible for deleting the Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::fetch_by_name
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] label [const AjPStr] Display label
** @param [wP] Pot [EnsPOperontranscript*]
** Ensembl Operon Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorFetchByDisplaylabel(
    EnsPOperontranscriptadaptor ota,
    const AjPStr label,
    EnsPOperontranscript *Pot)
{
    char *txtlabel = NULL;

    AjBool result = AJFALSE;

    AjPList ots = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPOperontranscript ot = NULL;

    if (!ota)
        return ajFalse;

    if (!label)
        return ajFalse;

    if (!Pot)
        return ajFalse;

    *Pot = NULL;

    ba = ensOperonadaptorGetBaseadaptor(ota);

    ensBaseadaptorEscapeC(ba, &txtlabel, label);

    constraint = ajFmtStr("operon_transcript.display_label = '%s'", txtlabel);

    ajCharDel(&txtlabel);

    ots = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        ots);

    if (ajListGetLength(ots) > 1)
        ajDebug("ensOperontranscriptadaptorFetchByDisplaylabel got more than "
                "one Ensembl Operon Transcript for display label '%S'.\n",
                label);

    ajListPop(ots, (void **) Pot);

    while (ajListPop(ots, (void **) &ot))
        ensOperontranscriptDel(&ot);

    ajListFree(&ots);

    ajStrDel(&constraint);

    return result;
}




/* @func ensOperontranscriptadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl Operon Transcript via its identifier.
**
** The caller is responsible for deleting the Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] identifier [ajuint] Identifier
** @param [wP] Pot [EnsPOperontranscript*] Ensembl Operon Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorFetchByIdentifier(
    EnsPOperontranscriptadaptor ota,
    ajuint identifier,
    EnsPOperontranscript *Pot)
{
    return ensBaseadaptorFetchByIdentifier(
        ensOperontranscriptadaptorGetBaseadaptor(ota),
        identifier,
        (void **) Pot);
}




/* @func ensOperontranscriptadaptorFetchByStableidentifier ********************
**
** Fetch an Ensembl Operon Transcript via its stable identifier and version.
** In case a version is not specified, the current Ensembl Operon Transcript
** will be returned.
** The caller is responsible for deleting the Ensembl Operon Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::fetch_by_stable_id
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Pot [EnsPOperontranscript*] Ensembl Operon Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorFetchByStableidentifier(
    EnsPOperontranscriptadaptor ota,
    const AjPStr stableid,
    ajuint version,
    EnsPOperontranscript *Pot)
{
    char *txtstableid = NULL;

    AjBool result = AJFALSE;

    AjPList ots = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPOperontranscript ot = NULL;

    if (!ota)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Pot)
        return ajFalse;

    *Pot = NULL;

    ba = ensOperontranscriptadaptorGetBaseadaptor(ota);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if (version)
        constraint = ajFmtStr(
            "operon_transcript.stable_id = '%s' "
            "AND "
            "operon_transcript.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
#if AJFALSE
            "operon_transcript.stable_id = '%s' "
            "AND "
            "operon_transcript.is_current = 1",
#endif /* AJFALSE */
            "operon_transcript.stable_id = '%s'",
            txtstableid);

    ajCharDel(&txtstableid);

    ots = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        ots);

    if (ajListGetLength(ots) > 1)
        ajDebug("ensOperontranscriptadaptorFetchByStableidentifier "
                "got more than one Ensembl Operon Transcript "
                "for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(ots, (void **) Pot);

    while (ajListPop(ots, (void **) &ot))
        ensOperontranscriptDel(&ot);

    ajListFree(&ots);

    ajStrDel(&constraint);

    return result;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Operon Transcript
** objects from an Ensembl SQL database.
**
** @fdata [EnsPOperontranscriptadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Operon Transcript-releated object(s)
** @nam4rule All Retrieve all Ensembl Operon Transcript-releated objects
** @nam5rule Genes Retrieve all Ensembl Gene objects
** @nam6rule By Retrieve all Ensembl Operon Transcript-relarted objects
**           via a criterion
** @nam7rule Operontranscriptidentifier
** Retrieve via an Ensembl Operon Transcript identifier
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
** @nam5rule Stableidentifiers Retrieve all stable identifier objects
**
** @argrule * ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @argrule ByOperontranscriptidentifier identifier [ajuint]
** Ensembl Operon Transcript identifier
** @argrule AllGenes genes [AjPList]
** AJAX List of Ensembl Gene objects
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Operon Transcript identifier)
** objects
** @argrule AllStableidentifiers stableids [AjPList]
** AJAX List of AJAX String (Ensembl Operon Transcript stable identifier)
** objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/





/* @func ensOperontranscriptadaptorRetrieveAllGenesByOperontranscriptidentifier
**
** Retrieve all Ensembl Gene objects of an
** Ensembl Operon Transcript identifier.
**
** The caller is responsible for deleting the Ensembl Gene objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::
**     fetch_genes_by_operon_transcript_id
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::
**     fetch_genes_by_operon_transcript
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [r] identifier [ajuint]
** Ensembl Operon Transcript identifier
** @param [u] genes [AjPList]
** AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorRetrieveAllGenesByOperontranscriptidentifier(
    EnsPOperontranscriptadaptor ota,
    ajuint identifier,
    AjPList genes)
{
    ajuint geneid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    if (!ota)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!genes)
        return ajFalse;

    dba = ensOperontranscriptadaptorGetDatabaseadaptor(ota);

    ga = ensRegistryGetGeneadaptor(dba);

    statement = ajFmtStr(
        "SELECT "
        "operon_transcript_gene.gene_id "
        "FROM "
        "operon_transcript_gene "
        "WHERE "
        "operon_transcript_gene.operon_transcript_id = %u",
        identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        geneid = 0U;
        gene   = NULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &geneid);

        ensGeneadaptorFetchByIdentifier(ga, geneid, &gene);

        if (gene)
            ajListPushAppend(genes, (void *) gene);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensOperontranscriptadaptorRetrieveAllIdentifiers *********************
**
** Retrieve all SQL database-internal identifier objects of
** Ensembl Operon Transcript objects.
**
** The caller is responsible for deleting the AJAX unsigned integer objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::list_dbIDs
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [u] identifiers [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Operon Transcript identifier)
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorRetrieveAllIdentifiers(
    EnsPOperontranscriptadaptor ota,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!ota)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("operon_transcript");

    result = ensBaseadaptorRetrieveAllIdentifiers(
        ensOperontranscriptadaptorGetBaseadaptor(ota),
        table,
        (AjPStr) NULL,
        identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensOperontranscriptadaptorRetrieveAllStableidentifiers ***************
**
** Retrieve all stable identifier objects of Ensembl Operon Transcript objects.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor::list_stable_ids
** @param [u] ota [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @param [u] stableids [AjPList]
** AJAX List of AJAX String (Ensembl Operon Transcript stable identifier)
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOperontranscriptadaptorRetrieveAllStableidentifiers(
    EnsPOperontranscriptadaptor ota,
    AjPList stableids)
{
    AjBool result = AJFALSE;

    AjPStr primary = NULL;
    AjPStr table   = NULL;

    if (!ota)
        return ajFalse;

    if (!stableids)
        return ajFalse;

    table   = ajStrNewC("operon_transcript");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(
        ensOperontranscriptadaptorGetBaseadaptor(ota),
        table,
        primary,
        stableids);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}

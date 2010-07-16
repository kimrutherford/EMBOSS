/******************************************************************************
** @source Ensembl Exon functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.17 $
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

#include "ensexon.h"
#include "ensbaseadaptor.h"
#include "enstranscript.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* @datastatic ExonPCoordinates ***********************************************
**
** Ensembl Exon Coordinates.
**
** Holds Ensembl Exon coordinates relative to a particular associated
** Transcript, as well as coding region coordinates relative to a Slice and a
** particular associated Transcript.
**
** @alias ExonSCoordinates
** @alias ExonOCoordinates
**
** @attr TranscriptStart [ajint] Exon start on Transcript
** @attr TranscriptEnd [ajint] Exon end on Transcript
** @attr TranscriptCodingStart [ajint] Coding region start on Transcript
** @attr TranscriptCodingEnd [ajint] Coding region end on Transcript
** @attr SliceCodingStart [ajint] Coding region start on Slice
** @attr SliceCodingEnd [ajint] Coding region end on Slice
** @@
******************************************************************************/

typedef struct ExonSCoordinates
{
    ajint TranscriptStart;
    ajint TranscriptEnd;
    ajint TranscriptCodingStart;
    ajint TranscriptCodingEnd;
    ajint SliceCodingStart;
    ajint SliceCodingEnd;
} ExonOCoordinates;

#define ExonPCoordinates ExonOCoordinates*




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static ExonPCoordinates exonCoordinatesNew(void);

static ExonPCoordinates exonCoordinatesNewObj(ExonPCoordinates object);

static void exonCoordinatesDel(ExonPCoordinates* Pec);

static void exonCoordinatesCopy(const void *key, void **value, void *cl);

static AjBool exonCoordinatesClear(EnsPExon exon);

static ExonPCoordinates exonGetExonCoodinates(EnsPExon exon,
                                              EnsPTranscript transcript);

static int exonCompareStartAscending(const void* P1, const void* P2);

static int exonCompareStartDescending(const void* P1, const void* P2);

static AjBool exonadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                       const AjPStr statement,
                                       EnsPAssemblymapper am,
                                       EnsPSlice slice,
                                       AjPList exons);

static void *exonadaptorCacheReference(void *value);

static void exonadaptorCacheDelete(void **value);

static ajulong exonadaptorCacheSize(const void *value);

static EnsPFeature exonadaptorGetFeature(const void *value);




/* @funcstatic exonCoordinatesNew *********************************************
**
** Default Exon Coordinates constructor.
**
** @return [ExonPCoordinates] Exon Coordinates
** @@
******************************************************************************/

static ExonPCoordinates exonCoordinatesNew(void)
{
    ExonPCoordinates ec = NULL;

    AJNEW0(ec);

    return ec;
}




/* @funcstatic exonCoordinatesNewObj ******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [ExonPCoordinates] Exon Coordinates
**
** @return [ExonPCoordinates] Exon Coordinates
** @@
******************************************************************************/

static ExonPCoordinates exonCoordinatesNewObj(ExonPCoordinates object)
{
    ExonPCoordinates ec = NULL;

    if(!object)
        return NULL;

    AJNEW0(ec);

    ec->TranscriptStart       = object->TranscriptStart;
    ec->TranscriptEnd         = object->TranscriptEnd;
    ec->TranscriptCodingStart = object->TranscriptCodingStart;
    ec->TranscriptCodingEnd   = object->TranscriptCodingEnd;
    ec->SliceCodingStart      = object->SliceCodingStart;
    ec->SliceCodingEnd        = object->SliceCodingEnd;

    return ec;
}




/* @funcstatic exonCoordinatesDel *********************************************
**
** Default Exon Coordinates destructor.
**
** @param [d] Pec [ExonPCoordinates*] Exon Coordinates address
**
** @return [void]
** @@
******************************************************************************/

static void exonCoordinatesDel(ExonPCoordinates* Pec)
{
    if(!Pec)
        return;

    if(!*Pec)
        return;

    AJFREE(*Pec);

    *Pec = NULL;

    return;
}




/* @filesection ensexon *******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPExon] Exon ***********************************************
**
** Functions for manipulating Ensembl Exon objects
**
** @cc Bio::EnsEMBL::Exon CVS Revision: 1.162
**
** @nam2rule Exon
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Exon by pointer.
** It is the responsibility of the user to first destroy any previous
** Exon. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExon]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPExon] Ensembl Exon
** @argrule Ref object [EnsPExon] Ensembl Exon
**
** @valrule * [EnsPExon] Ensembl Exon
**
** @fcategory new
******************************************************************************/




/* @func ensExonNew ***********************************************************
**
** Default Ensembl Exon constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Exon::new
** @param [r] sphase [ajint] Start phase of Translation
** @param [r] ephase [ajint] End phase of Translation
** @param [r] current [AjBool] Current attribute
** @param [r] constitutive [AjBool] Constitutive attribute
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
**
** @return [EnsPExon] Ensembl Exon or NULL
** @@
******************************************************************************/

EnsPExon ensExonNew(EnsPExonadaptor ea,
                    ajuint identifier,
                    EnsPFeature feature,
                    ajint sphase,
                    ajint ephase,
                    AjBool current,
                    AjBool constitutive,
                    AjPStr stableid,
                    ajuint version,
                    AjPStr cdate,
                    AjPStr mdate)
{
    EnsPExon exon = NULL;

    if(ajDebugTest("ensExonNew"))
    {
        ajDebug("ensExonNew\n"
                "  ea %p\n"
                "  identifier %u\n"
                "  feature %p\n"
                "  sphase %d\n"
                "  ephase %d\n"
                "  current '%B'\n"
                "  constitutive '%B'\n"
                "  stableid '%S'\n"
                "  version %u\n"
                "  cdate '%S'\n"
                "  mdate '%S'\n",
                ea,
                identifier,
                feature,
                sphase,
                ephase,
                current,
                constitutive,
                stableid,
                version,
                cdate,
                mdate);

        ensFeatureTrace(feature, 1);
    }

    if(!feature)
        return NULL;

    if((sphase < -1) || (sphase > 2))
    {
        ajDebug("ensExonNew start phase must be 0, 1, 2 for coding regions "
                "or -1 for non-coding regions, not %d.\n", sphase);

        return NULL;
    }

    if((ephase < -1) || (ephase > 2))
    {
        ajDebug("ensExonNew end phase must be 0, 1, 2 for coding regions "
                "or -1 for non-coding regions, not %d.\n", ephase);

        return NULL;
    }

    AJNEW0(exon);

    exon->Use          = 1;
    exon->Identifier   = identifier;
    exon->Adaptor      = ea;
    exon->Feature      = ensFeatureNewRef(feature);
    exon->StartPhase   = sphase;
    exon->EndPhase     = ephase;
    exon->Current      = current;
    exon->Constitutive = constitutive;

    if(stableid)
        exon->StableIdentifier = ajStrNewRef(stableid);

    exon->Version = version;

    if(cdate)
        exon->CreationDate = ajStrNewRef(cdate);

    if(mdate)
        exon->ModificationDate = ajStrNewRef(mdate);

    exon->SequenceCache      = NULL;
    exon->Supportingfeatures = NULL;
    exon->Coordinates        = NULL;

    return exon;
}




/* @funcstatic exonCoordinatesCopy ********************************************
**
** An ajTableMap 'apply' function to copy Exon Coordinates objects.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] value [void**] Exon Coordinates value data address
** @param [u] cl [void*] AJAX Table of Exon Coordinates objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void exonCoordinatesCopy(const void *key, void **value, void *cl)
{
    ajuint *Pidentifier = NULL;

    ExonPCoordinates ec = NULL;

    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    /* Copy unsigned integer key data. */

    AJNEW0(Pidentifier);

    if(!Pidentifier)
        return;

    *Pidentifier = *((const ajuint *) key);

    /* Copy Exon Coordinates value data. */

    ec = exonCoordinatesNewObj(*((ExonPCoordinates *) value));

    if(!ec)
        return;

    ajTablePut((AjPTable) cl,
               (void *) Pidentifier,
               (void *) exonCoordinatesNewObj(*((ExonPCoordinates *) value)));

    return;
}




/* @func ensExonNewObj ********************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPExon] Ensembl Exon
**
** @return [EnsPExon] Ensembl Exon or NULL
** @@
******************************************************************************/

EnsPExon ensExonNewObj(const EnsPExon object)
{
    AjIList iter = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPExon exon = NULL;

    if(!object)
        return NULL;

    AJNEW0(exon);

    exon->Use          = 1;
    exon->Identifier   = object->Identifier;
    exon->Adaptor      = object->Adaptor;
    exon->Feature      = ensFeatureNewRef(object->Feature);
    exon->StartPhase   = object->StartPhase;
    exon->EndPhase     = object->EndPhase;
    exon->Current      = object->Current;
    exon->Constitutive = object->Constitutive;

    if(object->StableIdentifier)
        exon->StableIdentifier = ajStrNewRef(object->StableIdentifier);

    exon->Version = object->Version;

    if(object->CreationDate)
        exon->CreationDate = ajStrNewRef(object->CreationDate);

    if(object->ModificationDate)
        exon->ModificationDate = ajStrNewRef(object->ModificationDate);

    if(object->SequenceCache)
        exon->SequenceCache = ajStrNewRef(object->SequenceCache);

    /* Copy the AJAX Table of Exon Coordinates. */

    if(object->Coordinates)
    {
        exon->Coordinates = ajTableNewFunctionLen(0,
                                                  ensTableCmpUint,
                                                  ensTableHashUint);

        ajTableMap(object->Coordinates,
                   exonCoordinatesCopy,
                   (void *) exon->Coordinates);
    }

    /* Copy the AJAX List of supporting Ensembl Base Align Features. */

    if(object->Supportingfeatures &&
       ajListGetLength(object->Supportingfeatures))
    {
        exon->Supportingfeatures = ajListNew();

        iter = ajListIterNew(object->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListPushAppend(exon->Supportingfeatures,
                             (void *) ensBasealignfeatureNewRef(baf));
        }

        ajListIterDel(&iter);
    }

    return exon;
}




/* @func ensExonNewRef ********************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [EnsPExon] Ensembl Exon
** @@
******************************************************************************/

EnsPExon ensExonNewRef(EnsPExon exon)
{
    if(!exon)
        return NULL;

    exon->Use++;

    return exon;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Exon.
**
** @fdata [EnsPExon]
** @fnote None
**
** @nam3rule Del Destroy (free) an Exon object
**
** @argrule * Pexon [EnsPExon*] Ensembl Exon object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic exonCoordinatesClear *******************************************
**
** Clear the Exon Coordinate Table.
** This function clears the unsigned integer Transcript identifier key data
** and the Exon Coordinate value data.
**
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool exonCoordinatesClear(EnsPExon exon)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    if(!exon)
        return ajFalse;

    if(!exon->Coordinates)
        return ajTrue;

    /* Clear the first-level AJAX Table. */

    ajTableToarrayKeysValues(exon->Coordinates, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajTableRemove(exon->Coordinates, (const void *) keyarray[i]);

        /* Delete unsigned integer Transcript identifier key data. */

        AJFREE(keyarray[i]);

        /* Delete the Exon Coordinate value data. */

        exonCoordinatesDel((ExonPCoordinates*) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @func ensExonDel ***********************************************************
**
** Default destructor for an Ensembl Exon.
**
** @param [d] Pexon [EnsPExon*] Ensembl Exon address
**
** @return [void]
** @@
******************************************************************************/

void ensExonDel(EnsPExon *Pexon)
{
    EnsPBasealignfeature baf = NULL;
    EnsPExon pthis = NULL;

    if(!Pexon)
        return;

    if(!*Pexon)
        return;

    if(ajDebugTest("ensExonDel"))
    {
        ajDebug("ensExonDel\n"
                "  *Pexon %p\n",
                *Pexon);

        ensExonTrace(*Pexon, 1);
    }

    pthis = *Pexon;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pexon = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->StableIdentifier);
    ajStrDel(&pthis->CreationDate);
    ajStrDel(&pthis->ModificationDate);
    ajStrDel(&pthis->SequenceCache);

    /* Clear and delete the AJAX List of supporting Base Align Features. */

    while(ajListPop((*Pexon)->Supportingfeatures, (void **) &baf))
        ensBasealignfeatureDel(&baf);

    ajListFree(&pthis->Supportingfeatures);

    /* Clear and delete the AJAX Table of Exon coordinates. */

    exonCoordinatesClear(pthis);

    ajTableFree(&pthis->Coordinates);

    AJFREE(pthis);

    *Pexon = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Exon object.
**
** @fdata [EnsPExon]
** @fnote None
**
** @nam3rule Get Return Exon attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Exon Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetStartPhase Return the start phase
** @nam4rule GetEndPhase Return the end phase
** @nam4rule GetCurrent Return the current element
** @nam4rule GetConstitutive Return the constitutive element
** @nam4rule GetStableIdentifier Return the stable identifier
** @nam4rule GetVersion Return the version
** @nam4rule GetCreationDate Return the creation date
** @nam4rule GetModificationDate Return the modification date
** @nam4rule GetSupportingfeatures Return all supporting
**                                 Ensembl Base Align Features
**
** @argrule * exon [const EnsPExon] Exon
**
** @valrule Adaptor [EnsPExonadaptor] Ensembl Exon Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule StartPhase [ajint] Start phase
** @valrule EndPhase [ajint] End phase
** @valrule Current [AjBool] Current attribute
** @valrule Constitutive [AjBool] Constitutive attribute
** @valrule StableIdentifier [AjPStr] Stable identifier
** @valrule Version [ajuint] Version
** @valrule CreationDate [AjPStr] Creation date
** @valrule ModificationDate [AjPStr] Modified date
** @valrule Supportingfeatures [AjPList] AJAX List of
**                                       Ensembl Base Align Features
**
** @fcategory use
******************************************************************************/




/* @func ensExonGetAdaptor ****************************************************
**
** Get the Ensembl Exon Adaptor element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [EnsPExonadaptor] Ensembl Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPExonadaptor ensExonGetAdaptor(const EnsPExon exon)
{
    if(!exon)
        return NULL;

    return exon->Adaptor;
}




/* @func ensExonGetIdentifier *************************************************
**
** Get the SQL database-internal identifier element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensExonGetIdentifier(const EnsPExon exon)
{
    if(!exon)
        return 0;

    return exon->Identifier;
}




/* @func ensExonGetFeature ****************************************************
**
** Get the Ensembl Feature element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::slice
** @cc Bio::EnsEMBL::Exon::start
** @cc Bio::EnsEMBL::Exon::end
** @cc Bio::EnsEMBL::Exon::strand
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensExonGetFeature(const EnsPExon exon)
{
    if(!exon)
        return NULL;

    return exon->Feature;
}




/* @func ensExonGetStartPhase *************************************************
**
** Get the start phase element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::phase
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajint] Start phase
** @@
**
** Get or set the phase of the Exon, which tells the translation machinery,
** which makes a peptide from the DNA, where to start.
**
** The Ensembl phase convention can be thought of as "the number of bases of
** the first codon which are on the previous exon". It is therefore 0, 1 or 2
** or -1 if the exon is non-coding. In ASCII art, with alternate codons
** represented by '###' and '+++':
**
**       Previous Exon   Intron   This Exon
**    ...-------------            -------------...
**
**    5'                    Phase                3'
**    ...#+++###+++###          0 +++###+++###+...
**    ...+++###+++###+          1 ++###+++###++...
**    ...++###+++###++          2 +###+++###+++...
**
** Here is another explanation from Ewan:
**
** Phase means the place where the intron lands inside the codon - 0 between
** codons, 1 between the 1st and second base, 2 between the second and 3rd
** base. Exons therefore have a start phase and an end phase, but introns have
** just one phase.
******************************************************************************/

ajint ensExonGetStartPhase(const EnsPExon exon)
{
    if(!exon)
        return 0;

    return exon->StartPhase;
}




/* @func ensExonGetEndPhase ***************************************************
**
** Get the end phase element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::end_phase
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajint] End phase
** @@
******************************************************************************/

ajint ensExonGetEndPhase(const EnsPExon exon)
{
    if(!exon)
        return 0;

    return exon->EndPhase;
}




/* @func ensExonGetCurrent ****************************************************
**
** Get the current element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_current
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue if this Exon reflects the current state of
**                  annotation
** @@
******************************************************************************/

AjBool ensExonGetCurrent(const EnsPExon exon)
{
    if(!exon)
        return ajFalse;

    return exon->Current;
}




/* @func ensExonGetConstitutive ***********************************************
**
** Get the constitutive element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_constitutive
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue if this Exon is constitutive
** @@
******************************************************************************/

AjBool ensExonGetConstitutive(const EnsPExon exon)
{
    if(!exon)
        return ajFalse;

    return exon->Constitutive;
}




/* @func ensExonGetStableIdentifier *******************************************
**
** Get the stable identifier element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::stable_id
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjPStr] Stable identifier
** @@
******************************************************************************/

AjPStr ensExonGetStableIdentifier(const EnsPExon exon)
{
    if(!exon)
        return NULL;

    return exon->StableIdentifier;
}




/* @func ensExonGetVersion ****************************************************
**
** Get the version element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::version
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajuint] Version
** @@
******************************************************************************/

ajuint ensExonGetVersion(const EnsPExon exon)
{
    if(!exon)
        return 0;

    return exon->Version;
}




/* @func ensExonGetCreationDate ***********************************************
**
** Get the creation date element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::created_date
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjPStr] Creation date
** @@
******************************************************************************/

AjPStr ensExonGetCreationDate(const EnsPExon exon)
{
    if(!exon)
        return NULL;

    return exon->CreationDate;
}




/* @func ensExonGetModificationDate *******************************************
**
** Get the modification date element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::modified_date
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjPStr] Modification date
** @@
******************************************************************************/

AjPStr ensExonGetModificationDate(const EnsPExon exon)
{
    if(!exon)
        return NULL;

    return exon->ModificationDate;
}




/* @func ensExonGetSupportingfeatures *****************************************
**
** Get all Ensembl Supporting Features for this Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch
** Ensembl Base Align Features of type 'dna' and 'protein' from the
** Ensembl Core database in case the AJAX List is empty.
**
** @cc Bio::EnsEMBL::Exon::get_all_supporting_features
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [const AjPList] AJAX List of Ensembl Base Align Features
** @@
******************************************************************************/

const AjPList ensExonGetSupportingfeatures(EnsPExon exon)
{
    EnsPDatabaseadaptor dba = NULL;

    if(!exon)
        return NULL;

    if(exon->Supportingfeatures)
        return exon->Supportingfeatures;

    if(!exon->Adaptor)
    {
        ajDebug("ensExonGetSupportingfeatures cannot fetch "
                "Ensembl Base Align Features for an Exon without an "
                "Exon Adaptor.\n");

        return NULL;
    }

    dba = ensExonadaptorGetDatabaseadaptor(exon->Adaptor);

    exon->Supportingfeatures = ajListNew();

    ensSupportingfeatureadaptorFetchAllByExon(dba,
                                              exon,
                                              exon->Supportingfeatures);

    return exon->Supportingfeatures;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Exon object.
**
** @fdata [EnsPExon]
** @fnote None
**
** @nam3rule Set Set one element of an Exon
** @nam4rule SetAdaptor Set the Ensembl Exon Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetStartPhase Set the start phase
** @nam4rule SetEndPhase Set the end phase
** @nam4rule SetCurrent Set the current element
** @nam4rule SetConstitutive Set the constitutive element
** @nam4rule SetStableIdentifier Set the stable identifier
** @nam4rule SetVersion Set the version
** @nam4rule SetCreationDate Set the creation date
** @nam4rule SetModificationDate Set the modification date
**
** @argrule * exon [EnsPExon] Ensembl Exon object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensExonSetAdaptor ****************************************************
**
** Set the Ensembl Exon Adaptor element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetAdaptor(EnsPExon exon, EnsPExonadaptor ea)
{
    if(!exon)
        return ajFalse;

    exon->Adaptor = ea;

    return ajTrue;
}




/* @func ensExonSetIdentifier *************************************************
**
** Set the SQL database-internal identifier element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetIdentifier(EnsPExon exon, ajuint identifier)
{
    if(!exon)
        return ajFalse;

    exon->Identifier = identifier;

    return ajTrue;
}




/* @func ensExonSetFeature ****************************************************
**
** Set the Ensembl Feature element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::slice
** @cc Bio::EnsEMBL::Exon::start
** @cc Bio::EnsEMBL::Exon::end
** @cc Bio::EnsEMBL::Exon::strand
** @cc Bio::EnsEMBL::Exon::move
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetFeature(EnsPExon exon, EnsPFeature feature)
{
    AjIList iter = NULL;

    EnsPBasealignfeature baf  = NULL;
    EnsPBasealignfeature nbaf = NULL;

    EnsPSlice eslice = NULL;

    if(ajDebugTest("ensExonSetFeature"))
    {
        ajDebug("ensExonSetFeature\n"
                "  exon %p\n"
                "  feature %p\n",
                exon,
                feature);

        ensExonTrace(exon, 1);

        ensFeatureTrace(feature, 1);
    }

    if(!exon)
        return ajFalse;

    if(!feature)
        return ajFalse;

    /* Replace the current Feature. */

    if(exon->Feature)
        ensFeatureDel(&exon->Feature);

    exon->Feature = ensFeatureNewRef(feature);

    /* Clear the sequence cache. */

    ajStrDel(&exon->SequenceCache);

    /* Clear the Exon coordinate cache. */

    exonCoordinatesClear(exon);

    /* Transfer Base Align Features onto the new Feature Slice. */

    if(!exon->Supportingfeatures)
        return ajTrue;

    eslice = ensFeatureGetSlice(exon->Feature);

    iter = ajListIterNew(exon->Supportingfeatures);

    while(!ajListIterDone(iter))
    {
        baf = (EnsPBasealignfeature) ajListIterGet(iter);

        ajListIterRemove(iter);

        nbaf = ensBasealignfeatureTransfer(baf, eslice);

        if(!nbaf)
        {
            ajDebug("ensExonSetFeature could not transfer Base Align Feature "
                    "onto new Ensembl Feature Slice.");

            ensBasealignfeatureTrace(baf, 1);
        }

        ajListIterInsert(iter, (void *) nbaf);

        /* Advance the AJAX List Iterator after the insert. */

        (void) ajListIterGet(iter);

        ensBasealignfeatureDel(&baf);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensExonSetStartPhase *************************************************
**
** Set the start phase element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::phase
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] sphase [ajint] Start phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetStartPhase(EnsPExon exon, ajint sphase)
{
    if(!exon)
        return ajFalse;

    exon->StartPhase = sphase;

    return ajTrue;
}




/* @func ensExonSetEndPhase ***************************************************
**
** Set the end phase element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::end_phase
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] ephase [ajint] End phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetEndPhase(EnsPExon exon, ajint ephase)
{
    if(!exon)
        return ajFalse;

    exon->EndPhase = ephase;

    return ajTrue;
}




/* @func ensExonSetCurrent ****************************************************
**
** Set the current element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_current
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] current [AjBool] Current attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetCurrent(EnsPExon exon, AjBool current)
{
    if(!exon)
        return ajFalse;

    exon->Current = current;

    return ajTrue;
}




/* @func ensExonSetConstitutive ***********************************************
**
** Set the constitutive element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_constitutive
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] constitutive [AjBool] Constitutive attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetConstitutive(EnsPExon exon, AjBool constitutive)
{
    if(!exon)
        return ajFalse;

    exon->Constitutive = constitutive;

    return ajTrue;
}




/* @func ensExonSetStableIdentifier *******************************************
**
** Set the stable identifier element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::stable_id
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetStableIdentifier(EnsPExon exon, AjPStr stableid)
{
    if(!exon)
        return ajFalse;

    ajStrDel(&exon->StableIdentifier);

    exon->StableIdentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensExonSetVersion ****************************************************
**
** Set the version element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::version
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetVersion(EnsPExon exon, ajuint version)
{
    if(!exon)
        return ajFalse;

    exon->Version = version;

    return ajTrue;
}




/* @func ensExonSetCreationDate ***********************************************
**
** Set the creation date element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::created_date
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetCreationDate(EnsPExon exon, AjPStr cdate)
{
    if(!exon)
        return ajFalse;

    ajStrDel(&exon->CreationDate);

    exon->CreationDate = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensExonSetModificationDate *******************************************
**
** Set the modification date element of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::modified_date
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSetModificationDate(EnsPExon exon, AjPStr mdate)
{
    if(!exon)
        return ajFalse;

    ajStrDel(&exon->ModificationDate);

    exon->ModificationDate = ajStrNewRef(mdate);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Exon object.
**
** @fdata [EnsPExon]
** @nam3rule Trace Report Ensembl Exon elements to debug file
**
** @argrule Trace exon [const EnsPExon] Ensembl Exon
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensExonTrace *********************************************************
**
** Trace an Ensembl Exon.
**
** @param [r] exon [const EnsPExon] Ensembl Exon
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonTrace(const EnsPExon exon, ajuint level)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    ajuint *Poid = NULL;

    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr tmpstr = NULL;

    EnsPBasealignfeature baf = NULL;

    ExonPCoordinates ec = NULL;

    if(!exon)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensExonTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  StartPhase %d\n"
            "%S  EndPhase %d\n"
            "%S  Current '%B'\n"
            "%S  Constitutive '%B'\n"
            "%S  StableIdentifier '%S'\n"
            "%S  Version %u\n"
            "%S  CreationDate '%S'\n"
            "%S  ModificationDate '%S'\n"
            "%S  SequenceCache %p\n"
            "%S  Coordinates %p\n"
            "%S  Supportingfeatures %p\n",
            indent, exon,
            indent, exon->Use,
            indent, exon->Identifier,
            indent, exon->Adaptor,
            indent, exon->Feature,
            indent, exon->StartPhase,
            indent, exon->EndPhase,
            indent, exon->Current,
            indent, exon->Constitutive,
            indent, exon->StableIdentifier,
            indent, exon->Version,
            indent, exon->CreationDate,
            indent, exon->ModificationDate,
            indent, exon->SequenceCache,
            indent, exon->Coordinates,
            indent, exon->Supportingfeatures);

    ensFeatureTrace(exon->Feature, level + 1);

    if(exon->SequenceCache)
    {
        /*
        ** For sequences longer than 40 characters use a
        ** '20 ... 20' notation.
        */

        if(ajStrGetLen(exon->SequenceCache) > 40)
        {
            tmpstr = ajStrNew();

            ajStrAssignSubS(&tmpstr, exon->SequenceCache, 0, 19);

            ajStrAppendC(&tmpstr, " ... ");

            ajStrAppendSubS(&tmpstr, exon->SequenceCache, -20, -1);

            ajDebug("%S  Sequence cache: '%S'\n", indent, tmpstr);

            ajStrDel(&tmpstr);
        }
        else
            ajDebug("%S  Sequence cache: '%S'\n", indent, exon->SequenceCache);
    }

    if(exon->Coordinates)
    {
        ajDebug("%S  AJAX Table %p of Ensembl Exon coordinates\n",
                indent, exon->Coordinates);

        ajTableToarrayKeysValues(exon->Coordinates, &keyarray, &valarray);

        for(i = 0; valarray[i]; i++)
        {
            /* Unsigned integer key data. */

            Poid = (ajuint *) keyarray[i];

            ec = (ExonPCoordinates) valarray[i];

            ajDebug("%S    Ensembl Transcript identifier %u\n", indent, *Poid);

            ajDebug("%S        Transcript coordinates %u:%u\n", indent,
                    ec->TranscriptStart, ec->TranscriptEnd);

            ajDebug("%S        Coding sequence coordinates %u:%u\n", indent,
                    ec->TranscriptCodingStart, ec->TranscriptCodingEnd);

            ajDebug("%S        Slice coordinates %u:%u\n", indent,
                    ec->SliceCodingStart, ec->SliceCodingEnd);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    /* Trace the AJAX List of supporting Ensembl Base Align Features. */

    if(exon->Supportingfeatures)
    {
        ajDebug("%S    AJAX List %p of Ensembl Base Align Features\n",
                indent, exon->Supportingfeatures);

        iter = ajListIterNewread(exon->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ensBasealignfeatureTrace(baf, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensExonGetFrame ******************************************************
**
** Get the coding frame of an Ensembl Exon on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Exon:frame
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajint] Coding frame, which can be 0, 1, 2 or -1 for
**                 Exons that are not coding.
** @@
******************************************************************************/

ajint ensExonGetFrame(const EnsPExon exon)
{
    if(!exon)
        return 0;

    if(exon->StartPhase == -1)
        return -1;

    if(exon->StartPhase == 0)
        return (exon->Feature->Start % 3);

    if(exon->StartPhase == 1)
        return ((exon->Feature->Start + 2) % 3);

    if(exon->StartPhase == 2)
        return ((exon->Feature->Start + 1) % 3);

    ajDebug("ensExonGetFrame invalid start phase %d in exon %u.\n",
            exon->StartPhase, exon->Identifier);

    return 0;
}




/* @func ensExonGetMemsize ****************************************************
**
** Get the memory size in bytes of an Ensembl Exon.
**
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensExonGetMemsize(const EnsPExon exon)
{
    ajulong size = 0;

    AjIList iter = NULL;

    EnsPBasealignfeature baf = NULL;

    if(!exon)
        return 0;

    size += sizeof (EnsOExon);

    size += ensFeatureGetMemsize(exon->Feature);

    if(exon->StableIdentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->StableIdentifier);
    }

    if(exon->CreationDate)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->CreationDate);
    }

    if(exon->ModificationDate)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->ModificationDate);
    }

    if(exon->SequenceCache)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->SequenceCache);
    }

    /* Summarise the AJAX Table of Exon Coordinates. */

    if(exon->Coordinates)
    {
        size += sizeof (AjOTable);

        size += (sizeof (ajuint) + sizeof (ExonOCoordinates)) *
            ajTableGetLength(exon->Coordinates);

        /* NOTE: The AJAX Table-internal buckets are not considered here. */
    }

    /* Summarise the AJAX List of supporting Ensembl Base Align Features. */

    if(exon->Supportingfeatures)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(exon->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            size += ensBasealignfeatureGetMemsize(baf);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @func ensExonTransform *****************************************************
**
** Transform an Ensembl Exon into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Feature::transform
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPExon] Ensembl Exon in the defined
**                    Ensembl Coordinate System or NULL
** @@
******************************************************************************/

EnsPExon ensExonTransform(EnsPExon exon, const AjPStr csname,
                          const AjPStr csversion)
{
    EnsPExon nexon       = NULL;
    EnsPFeature nfeature = NULL;

    if(!exon)
        return NULL;

    if(!csname)
        return NULL;

    if(!csversion)
        return NULL;

    nfeature = ensFeatureTransform(exon->Feature, csname, csversion);

    if(!nfeature)
        return NULL;

    nexon = ensExonNewObj(exon);

    ensExonSetFeature(nexon, nfeature);

    /*
    ** NOTE: The Exon-internal supporting features have already been
    ** transfered to the new Slice and the sequence cache has already been
    ** cleared by the ensExonSetFeature function.
    */

    return nexon;
}




/* @func ensExonTransfer ******************************************************
**
** Transfer an Ensembl Exon onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::transfer
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPExon] Ensembl Exon on the defined Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPExon ensExonTransfer(EnsPExon exon, EnsPSlice slice)
{
    EnsPExon newexon = NULL;
    EnsPFeature newfeature = NULL;

    if(!exon)
        return NULL;

    if(!slice)
        return NULL;

    newfeature = ensFeatureTransfer(exon->Feature, slice);

    if(!newfeature)
        return NULL;

    newexon = ensExonNewObj(exon);

    ensExonSetFeature(newexon, newfeature);

    /*
    ** NOTE: The Exon-internal supporting features have already been
    ** transfered to the new Slice and the sequence cache has already been
    ** cleared by the ensExonSetFeature function.
    */

    ensFeatureDel(&newfeature);

    return newexon;
}




/* @func ensExonFetchDisplayIdentifier ****************************************
**
** Fetch the display identifier of an Ensembl Exon.
** This will return the stable identifier, the SQL database-internal identifier
** or the Exon memory address in this order.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Exon:display_id
** @param [r] exon [const EnsPExon] Ensembl Exon
** @param [wP] Pidentifier [AjPStr*] Display identifier address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonFetchDisplayIdentifier(const EnsPExon exon, AjPStr* Pidentifier)
{
    if(!exon)
        return ajFalse;

    if(!Pidentifier)
        return ajFalse;

    if(exon->StableIdentifier && ajStrGetLen(exon->StableIdentifier))
        *Pidentifier = ajStrNewS(exon->StableIdentifier);
    else if(exon->Identifier)
        *Pidentifier = ajFmtStr("%u", exon->Identifier);
    else
        *Pidentifier = ajFmtStr("%p", exon);

    return ajTrue;
}




/* @func ensExonFetchSequenceStr **********************************************
**
** Fetch the sequence of an Ensembl Exon as AJAX String.
** The caller is responsible for deleting the AJAX String.
**
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [wP] Psequence [AjPStr*] Ensembl Exon sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonFetchSequenceStr(EnsPExon exon, AjPStr* Psequence)
{
    EnsPFeature feature = NULL;

    EnsPSlice slice = NULL;

    if(!exon)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    feature = exon->Feature;

    if(!feature)
    {
        ajWarn("ensExonFetchSequenceStr cannot get sequence without an "
               "Ensembl Feature attached to the Ensembl Exon '%u:%S'.\n",
               exon->Identifier, exon->StableIdentifier);

        return ajFalse;
    }

    slice = ensFeatureGetSlice(feature);

    if(!slice)
    {
        ajWarn("ensExonFetchSequenceStr cannot get sequence without an "
               "Ensembl Slice attached to the Ensembl Feature in the "
               "Ensembl Exon '%u:%S'.\n",
               exon->Identifier, exon->StableIdentifier);

        return ajFalse;
    }

    if(!exon->SequenceCache)
        exon->SequenceCache = ajStrNew();

    if(!ajStrGetLen(exon->SequenceCache))
        ensSliceFetchSubSequenceStr(slice,
                                    feature->Start,
                                    feature->End,
                                    feature->Strand,
                                    &exon->SequenceCache);

    *Psequence = ajStrNewRef(exon->SequenceCache);

    return ajTrue;
}




/* @func ensExonFetchSequenceSeq **********************************************
**
** Fetch the sequence of an Ensembl Exon as AJAX Sequence.
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Exon:seq
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [wP] Psequence [AjPSeq*] Ensembl Exon sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonFetchSequenceSeq(EnsPExon exon, AjPSeq* Psequence)
{
    AjPStr name = NULL;
    AjPStr sequence = NULL;

    if(!exon)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    ensExonFetchSequenceStr(exon, &sequence);

    ensExonFetchDisplayIdentifier(exon, &name);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @funcstatic exonGetExonCoodinates ******************************************
**
** Get Exon Coordinates for an Ensembl Exon as part of an Ensembl Transcript.
**
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ExonPCoordinates] Exon Coordinates
** @@
******************************************************************************/

static ExonPCoordinates exonGetExonCoodinates(EnsPExon exon,
                                              EnsPTranscript transcript)
{
    ajint scstart = 0;
    ajint scend   = 0;
    ajint tcstart = 0;
    ajint tcend   = 0;

    ajuint identifier = 0;

    ajuint *Pidentifier = NULL;

    AjBool debug = AJFALSE;

    AjPList mrs = NULL;

    EnsPFeature feature = NULL;
    EnsPMapperresult mr = NULL;
    ExonPCoordinates ec = NULL;

    debug = ajDebugTest("exonGetExonCoodinates");

    if(debug)
        ajDebug("exonGetExonCoodinates\n"
                "  exon %p\n"
                "  transcript %p\n",
                exon,
                transcript);

    if(!exon)
        return NULL;

    if(!transcript)
        return NULL;

    identifier = ensTranscriptGetIdentifier(transcript);

    if(!exon->Coordinates)
        exon->Coordinates = MENSTABLEUINTNEW(0);

    ec = (ExonPCoordinates)
        ajTableFetch(exon->Coordinates, (const void*) &identifier);

    if(ec)
        return ec;

    AJNEW0(Pidentifier);

    *Pidentifier = identifier;

    ec = exonCoordinatesNew();

    ajTablePut(exon->Coordinates, (void *) Pidentifier, (void *) ec);

    /* Get the start and end of an Ensembl Exon in Transcript coordinates. */

    mrs = ajListNew();

    ensTranscriptMapperSliceToTranscript(transcript,
                                         ensFeatureGetStart(exon->Feature),
                                         ensFeatureGetEnd(exon->Feature),
                                         ensFeatureGetStrand(exon->Feature),
                                         mrs);

    if(!ajListGetLength(mrs))
    {
        ajDebug("exonGetExonCoodinates cannot map Exon.\n");

        ensExonTrace(exon, 1);
    }

    ajListPeekFirst(mrs, (void **) &mr);

    switch(ensMapperresultGetType(mr))
    {
        case ensEMapperresultCoordinate:

            ec->TranscriptStart = ensMapperresultGetStart(mr);
            ec->TranscriptEnd   = ensMapperresultGetEnd(mr);

            if(debug)
                ajDebug("exonGetExonCoodinates Exon '%S' "
                        "Transcript '%S:%d:%d'.\n",
                        ensExonGetStableIdentifier(exon),
                        ensTranscriptGetStableIdentifier(transcript),
                        ec->TranscriptStart,
                        ec->TranscriptEnd);

            break;

        case ensEMapperresultGap:

            if(debug)
            {
                ajDebug("exonGetExonCoodinates maps the first part of the "
                        "Exon into a gap %d:%d.\n",
                        ensMapperresultGetGapStart(mr),
                        ensMapperresultGetGapEnd(mr));

                ensExonTrace(exon, 1);
            }

            break;

        default:

            ajWarn("exonGetExonCoodinates got an Ensembl Mapper Result of "
                   "unexpected type %d.", ensMapperresultGetType(mr));
    }

    while(ajListPop(mrs, (void **) &mr))
        ensMapperresultDel(&mr);

    ajListFree(&mrs);

    /*
    ** Get the start and end of the coding region of the Exon in
    ** Transcript coordinates.
    */

    tcstart = ensTranscriptGetTranscriptCodingStart(transcript);

    tcend = ensTranscriptGetTranscriptCodingEnd(transcript);

    if(debug)
        ajDebug("exonGetExonCoodinates Transcript Coding %d:%d\n",
                tcstart, tcend);

    if(!tcstart)
    {
        /* This is a non-coding Transcript. */

        ec->TranscriptCodingStart = 0;

        ec->TranscriptCodingEnd = 0;
    }
    else
    {
        if(tcstart < ec->TranscriptStart)
        {
            /* The coding region starts up-stream of this Exon ... */

            if(tcend < ec->TranscriptStart)
            {
                /* ... and also ends up-stream of this Exon. */

                ec->TranscriptCodingStart = 0;

                ec->TranscriptCodingEnd = 0;
            }
            else
            {
                /* ... and does not end up-stream of this Exon, but ... */

                ec->TranscriptCodingStart = ec->TranscriptStart;

                if(tcend < ec->TranscriptEnd)
                {
                    /* ... ends in this Exon. */

                    ec->TranscriptCodingEnd = tcend;
                }
                else
                {
                    /* ... ends down-stream of this Exon. */

                    ec->TranscriptCodingEnd = ec->TranscriptEnd;
                }
            }
        }
        else
        {
            /*
            ** The coding region starts either within or down-stream of
            ** this Exon.
            */

            if(tcstart <= ec->TranscriptEnd)
            {
                /* The coding region starts within this Exon ... */

                ec->TranscriptCodingStart = tcstart;

                if(tcend < ec->TranscriptEnd)
                {
                    /* ... and also ends within this Exon. */

                    ec->TranscriptCodingEnd = tcend;
                }
                else
                {
                    /* ... and ends down-stream of this Exon. */

                    ec->TranscriptCodingEnd = ec->TranscriptEnd;
                }
            }
            else
            {
                /*
                ** The coding region starts and ends down-stream
                ** of this Exon.
                */

                ec->TranscriptCodingStart = 0;

                ec->TranscriptCodingEnd = 0;
            }
        }
    }

    /*
    ** Get the start and end of the coding region of the Exon in
    ** Slice coordinates.
    */

    scstart = ensTranscriptGetSliceCodingStart(transcript);

    scend = ensTranscriptGetSliceCodingEnd(transcript);

    if(!scstart)
    {
        /* This is a non-coding Transcript. */

        ec->SliceCodingStart = 0;

        ec->SliceCodingEnd = 0;
    }
    else
    {
        feature = ensExonGetFeature(exon);

        if(scstart < ensFeatureGetStart(feature))
        {
            /* The coding region starts up-stream this Exon ... */

            if(scend < ensFeatureGetStart(feature))
            {
                /* ... and also ends up-stream of this Exon. */

                ec->SliceCodingStart = 0;

                ec->SliceCodingEnd = 0;
            }
            else
            {
                /* ... and does not end up-stream of this Exon, but ... */

                ec->SliceCodingStart = ensFeatureGetStart(feature);

                if(scend < ensFeatureGetEnd(feature))
                {
                    /* ... ends in this Exon. */

                    ec->SliceCodingEnd = scend;
                }

                else
                {
                    /* ... ends down-stream of this Exon. */

                    ec->SliceCodingEnd = ensFeatureGetEnd(feature);
                }
            }
        }
        else
        {
            /*
            ** The coding region starts either within or down-stream of
            ** this Exon.
            */

            if(scstart <= ensFeatureGetEnd(feature))
            {
                /* The coding region starts within this Exon ... */

                ec->SliceCodingStart = scstart;

                if(scend < ensFeatureGetEnd(feature))
                {
                    /* ... and also ends within this Exon . */

                    ec->SliceCodingEnd = scend;
                }
                else
                {
                    /* ... and ends down-stream of this Exon. */

                    ec->SliceCodingEnd = ensFeatureGetEnd(feature);
                }
            }
            else
            {
                /*
                ** The coding region starts and ends down-stream
                ** of this Exon.
                */

                ec->SliceCodingStart = 0;

                ec->SliceCodingEnd = 0;
            }
        }
    }

    return ec;
}




/* @func ensExonGetTranscriptStart ********************************************
**
** Get the start coordinate of an Ensembl Exon in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Exon::cdna_start
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Exon start coordinate relative to the Transcript
** @@
******************************************************************************/

ajuint ensExonGetTranscriptStart(EnsPExon exon, EnsPTranscript transcript)
{
    ExonPCoordinates ec = NULL;

    if(!exon)
        return 0;

    if(!transcript)
        return 0;

    ec = exonGetExonCoodinates(exon, transcript);

    if(ec)
        return ec->TranscriptStart;

    return 0;
}




/* @func ensExonGetTranscriptEnd **********************************************
**
** Get the end coordinate of an Ensembl Exon in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Exon::cdna_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Exon end coordinate relative to the Transcript
** @@
******************************************************************************/

ajuint ensExonGetTranscriptEnd(EnsPExon exon, EnsPTranscript transcript)
{
    ExonPCoordinates ec = NULL;

    if(!exon)
        return 0;

    if(!transcript)
        return 0;

    ec = exonGetExonCoodinates(exon, transcript);

    if(ec)
        return ec->TranscriptEnd;

    return 0;
}




/* @func ensExonGetTranscriptCodingStart **************************************
**
** Get the start coordinate of the coding region of the Exon in
** Transcript coordinates.
**
** @cc Bio::EnsEMBL::Exon::cdna_coding_start
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Exon coding start coordinate relative to the Transcript
** @@
******************************************************************************/

ajuint ensExonGetTranscriptCodingStart(EnsPExon exon, EnsPTranscript transcript)
{
    ExonPCoordinates ec = NULL;

    if(!exon)
        return 0;

    if(!transcript)
        return 0;

    ec = exonGetExonCoodinates(exon, transcript);

    if(ec)
        return ec->TranscriptCodingStart;

    return 0;
}




/* @func ensExonGetTranscriptCodingEnd ****************************************
**
** Get the end coordinate of the coding region of the Exon in
** Transcript coordinates.
**
** @cc Bio::EnsEMBL::Exon::cdna_coding_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Exon coding end coordinate relative to the Transcript
** @@
******************************************************************************/

ajuint ensExonGetTranscriptCodingEnd(EnsPExon exon, EnsPTranscript transcript)
{
    ExonPCoordinates ec = NULL;

    if(!exon)
        return 0;

    if(!transcript)
        return 0;

    ec = exonGetExonCoodinates(exon, transcript);

    if(ec)
        return ec->TranscriptCodingEnd;

    return 0;
}




/* @func ensExonGetSliceCodingStart *******************************************
**
** Get the start coordinate of the coding region of the Exon in
** Slice coordinates.
**
** @cc Bio::EnsEMBL::Exon::coding_region_start
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Exon coding start coordinate relative to the Slice
** @@
******************************************************************************/

ajuint ensExonGetSliceCodingStart(EnsPExon exon, EnsPTranscript transcript)
{
    ExonPCoordinates ec = NULL;

    if(!exon)
        return 0;

    if(!transcript)
        return 0;

    ec = exonGetExonCoodinates(exon, transcript);

    if(ec)
        return ec->SliceCodingStart;

    return 0;
}




/* @func ensExonGetSliceCodingEnd *********************************************
**
** Get the end coordinate of the coding region of the Exon in
** Slice coordinates.
**
** @cc Bio::EnsEMBL::Exon::coding_region_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Exon coding end coordinate relative to the Slice
** @@
******************************************************************************/

ajuint ensExonGetSliceCodingEnd(EnsPExon exon, EnsPTranscript transcript)
{
    ExonPCoordinates ec = NULL;

    if(!exon)
        return 0;

    if(!transcript)
        return 0;

    ec = exonGetExonCoodinates(exon, transcript);

    if(ec)
        return ec->SliceCodingEnd;

    return 0;
}




/* @funcstatic exonCompareStartAscending **************************************
**
** Comparison function to sort Ensembl Exons by their Ensembl Feature start
** coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Exon address 1
** @param [r] P2 [const void*] Ensembl Exon address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int exonCompareStartAscending(const void* P1, const void* P2)
{
    const EnsPExon exon1 = NULL;
    const EnsPExon exon2 = NULL;

    exon1 = *(EnsPExon const *) P1;
    exon2 = *(EnsPExon const *) P2;

    if(ajDebugTest("exonCompareStartAscending"))
        ajDebug("exonCompareStartAscending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);

    /* Sort empty values towards the end of the AJAX List. */

    if(exon1 && (!exon2))
        return -1;

    if((!exon1) && (!exon2))
        return 0;

    if((!exon1) && exon2)
        return +1;

    return ensFeatureCompareStartAscending(exon1->Feature, exon2->Feature);
}




/* @func ensExonSortByStartAscending ******************************************
**
** Sort Ensembl Exons by their Ensembl Feature start coordinate
** in ascending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSortByStartAscending(AjPList exons)
{
    if(!exons)
        return ajFalse;

    ajListSort(exons, exonCompareStartAscending);

    return ajTrue;
}




/* @funcstatic exonCompareStartDescending *************************************
**
** Comparison function to sort Ensembl Exons by their Ensembl Feature start
** coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Exon address 1
** @param [r] P2 [const void*] Ensembl Exon address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int exonCompareStartDescending(const void* P1, const void* P2)
{
    const EnsPExon exon1 = NULL;
    const EnsPExon exon2 = NULL;

    exon1 = *(EnsPExon const *) P1;
    exon2 = *(EnsPExon const *) P2;

    if(ajDebugTest("exonCompareStartDescending"))
        ajDebug("exonCompareStartDescending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);

    /* Sort empty values towards the end of the AJAX List. */

    if(exon1 && (!exon2))
        return -1;

    if((!exon1) && (!exon2))
        return 0;

    if((!exon1) && exon2)
        return +1;

    return ensFeatureCompareStartDescending(exon1->Feature, exon2->Feature);
}




/* @func ensExonSortByStartDescending *****************************************
**
** Sort Ensembl Exons by their Ensembl Feature start coordinate
** in descending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonSortByStartDescending(AjPList exons)
{
    if(!exons)
        return ajFalse;

    ajListSort(exons, exonCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPExonadaptor] Exon Adaptor ********************************
**
** Functions for manipulating Ensembl Exon Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor CVS Revision: 1.101
**
** @nam2rule Exonadaptor
**
******************************************************************************/

static const char *exonadaptorTables[] =
{
    "exon",
    "exon_stable_id",
    NULL
};

static const char *exonTranscriptadaptorTables[] =
{
    "exon",
    "exon_stable_id",
    "exon_transcript",
    NULL
};

static const char *exonadaptorColumns[] =
{
    "exon.exon_id",
    "exon.seq_region_id",
    "exon.seq_region_start",
    "exon.seq_region_end",
    "exon.seq_region_strand",
    "exon.phase",
    "exon.end_phase",
    "exon.is_current",
    "exon.is_constitutive",
    "exon_stable_id.stable_id",
    "exon_stable_id.version",
    "exon_stable_id.created_date",
    "exon_stable_id.modified_date",
    NULL
};

static EnsOBaseadaptorLeftJoin exonadaptorLeftJoin[] =
{
    {"exon_stable_id", "exon_stable_id.exon_id = exon.exon_id"},
    {NULL, NULL}
};

static const char *exonadaptorDefaultCondition = NULL;

static const char *exonadaptorFinalCondition = NULL;

static const char *exonTranscriptadaptorFinalCondition =
    "ORDER BY "
    "exon_transcript.transcript_id, "
    "exon_transcript.rank";




/* @funcstatic exonadaptorFetchAllBySQL ***************************************
**
** Fetch all Ensembl Exon objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::_objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool exonadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                       const AjPStr statement,
                                       EnsPAssemblymapper am,
                                       EnsPSlice slice,
                                       AjPList exons)
{
    ajint sphase = 0;
    ajint ephase = 0;

    ajuint identifier = 0;
    ajuint version    = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    AjBool current      = AJFALSE;
    AjBool constitutive = AJFALSE;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr stableid = NULL;
    AjPStr cdate    = NULL;
    AjPStr mdate    = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPExon exon      = NULL;
    EnsPExonadaptor ea = NULL;

    EnsPFeature feature = NULL;
    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("exonadaptorFetchAllBySQL"))
        ajDebug("exonadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  exons %p\n",
                dba,
                statement,
                am,
                slice,
                exons);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!exons)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(dba);

    ea = ensRegistryGetExonadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier   = 0;
        srid         = 0;
        srstart      = 0;
        srend        = 0;
        srstrand     = 0;
        sphase       = 0;
        ephase       = 0;
        current      = ajFalse;
        constitutive = ajFalse;
        stableid     = ajStrNew();
        version      = 0;
        cdate        = ajStrNew();
        mdate        = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToInt(sqlr, &sphase);
        ajSqlcolumnToInt(sqlr, &ephase);
        ajSqlcolumnToBool(sqlr, &current);
        ajSqlcolumnToBool(sqlr, &constitutive);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);

        /* Need to get the internal Ensembl Sequence Region identifier. */

        srid = ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Features, the range needs
        ** checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("exonadaptorFetchAllBySQL got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("exonadaptorFetchAllBySQL got a "
                    "Sequence Region end coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srend, INT_MAX);

        slstrand = srstrand;

        /* Fetch a Slice spanning the entire Sequence Region. */

        ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);

        /*
        ** Increase the reference counter of the Ensembl Assembly Mapper if
        ** one has been specified, otherwise fetch it from the database if a
        ** destination Slice has been specified.
        */

        if(am)
            am = ensAssemblymapperNewRef(am);
        else if(slice && (!ensCoordsystemMatch(
                              ensSliceGetCoordsystem(slice),
                              ensSliceGetCoordsystem(srslice))))
            am = ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper is defined at this point.
        */

        if(am)
        {
            ensAssemblymapperFastMap(am,
                                     ensSliceGetSeqregion(srslice),
                                     slstart,
                                     slend,
                                     slstrand,
                                     mrs);

            /*
            ** The ensAssemblymapperFastMap function returns at best one
            ** Ensembl Mapper Result.
            */

            ajListPop(mrs, (void **) &mr);

            /*
            ** Skip Features that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&stableid);
                ajStrDel(&cdate);
                ajStrDel(&mdate);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                ensMapperresultDel(&mr);

                continue;
            }

            srid = ensMapperresultGetObjectIdentifier(mr);

            slstart = ensMapperresultGetStart(mr);

            slend = ensMapperresultGetEnd(mr);

            slstrand = ensMapperresultGetStrand(mr);

            /*
            ** Delete the Sequence Region Slice and fetch a Slice in the
            ** Coordinate System we just mapped to.
            */

            ensSliceDel(&srslice);

            ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                      srid,
                                                      0,
                                                      0,
                                                      0,
                                                      &srslice);

            ensMapperresultDel(&mr);
        }

        /*
        ** Convert Sequence Region Slice coordinates to destination Slice
        ** coordinates, if a destination Slice has been provided.
        */

        if(slice)
        {
            /* Check that the length of the Slice is within range. */

            if(ensSliceGetLength(slice) <= INT_MAX)
                sllength = (ajint) ensSliceGetLength(slice);
            else
                ajFatal("exonadaptorFetchAllBySQL got a Slice, "
                        "which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceGetLength(slice), INT_MAX);

            /*
            ** Nothing needs to be done if the destination Slice starts at 1
            ** and is on the forward strand.
            */

            if((ensSliceGetStart(slice) != 1) ||
               (ensSliceGetStrand(slice) < 0))
            {
                if(ensSliceGetStrand(slice) >= 0)
                {
                    slstart = slstart - ensSliceGetStart(slice) + 1;

                    slend = slend - ensSliceGetStart(slice) + 1;
                }
                else
                {
                    slend = ensSliceGetEnd(slice) - slstart + 1;

                    slstart = ensSliceGetEnd(slice) - slend + 1;

                    slstrand *= -1;
                }
            }

            /*
            ** Throw away Features off the end of the requested Slice or on
            ** any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&stableid);
                ajStrDel(&cdate);
                ajStrDel(&mdate);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        /* Finally, create a new Ensembl Exon. */

        feature = ensFeatureNewS((EnsPAnalysis) NULL,
                                 srslice,
                                 slstart,
                                 slend,
                                 slstrand);

        exon = ensExonNew(ea,
                          identifier,
                          feature,
                          sphase,
                          ephase,
                          current,
                          constitutive,
                          stableid,
                          version,
                          cdate,
                          mdate);

        ajListPushAppend(exons, (void *) exon);

        ensFeatureDel(&feature);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);

        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic exonadaptorCacheReference **************************************
**
** Wrapper function to reference an Ensembl Exon from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Exon
**
** @return [void*] Ensembl Exon or NULL
** @@
******************************************************************************/

static void* exonadaptorCacheReference(void *value)
{
    if(!value)
        return NULL;

    return (void *) ensExonNewRef((EnsPExon) value);
}




/* @funcstatic exonadaptorCacheDelete *****************************************
**
** Wrapper function to delete an Ensembl Exon from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Exon address
**
** @return [void]
** @@
******************************************************************************/

static void exonadaptorCacheDelete(void **value)
{
    if(!value)
        return;

    ensExonDel((EnsPExon *) value);

    return;
}




/* @funcstatic exonadaptorCacheSize *******************************************
**
** Wrapper function to determine the memory size of an Ensembl Exon
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Exon
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

static ajulong exonadaptorCacheSize(const void *value)
{
    if(!value)
        return 0;

    return ensExonGetMemsize((const EnsPExon) value);
}




/* @funcstatic exonadaptorGetFeature ******************************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Exon
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Exon
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature exonadaptorGetFeature(const void *value)
{
    if(!value)
        return NULL;

    return ensExonGetFeature((const EnsPExon) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Exon Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Exon Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExonadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPExonadaptor] Ensembl Exon Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensExonadaptorNew ****************************************************
**
** Default Ensembl Exon Adaptor constructor.
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
** @see ensRegistryGetExonadaptor
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPExonadaptor] Ensembl Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPExonadaptor ensExonadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPExonadaptor ea = NULL;

    if(!dba)
        return NULL;

    AJNEW0(ea);

    ea->Adaptor = ensFeatureadaptorNew(
        dba,
        exonadaptorTables,
        exonadaptorColumns,
        exonadaptorLeftJoin,
        exonadaptorDefaultCondition,
        exonadaptorFinalCondition,
        exonadaptorFetchAllBySQL,
        (void * (*)(const void *key)) NULL,
        exonadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        exonadaptorCacheDelete,
        exonadaptorCacheSize,
        exonadaptorGetFeature,
        "Exon");

    return ea;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Exon Adaptor.
**
** @fdata [EnsPExonadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Exon Adaptor object
**
** @argrule * Pea [EnsPExonadaptor*] Ensembl Exon Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExonadaptorDel ****************************************************
**
** Default destructor for an Ensembl Exon Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pea [EnsPExonadaptor*] Ensembl Exon Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensExonadaptorDel(EnsPExonadaptor *Pea)
{
    EnsPExonadaptor pthis = NULL;

    if(!Pea)
        return;

    if(!*Pea)
        return;

    pthis = *Pea;

    ensFeatureadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pea = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Exon Adaptor object.
**
** @fdata [EnsPExonadaptor]
** @fnote None
**
** @nam3rule Get Return Ensembl Exon Adaptor attribute(s)
** @nam3rule GetFeatureadaptor Return the Ensembl Feature Adaptor
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ea [const EnsPExonadaptor] Ensembl Exon Adaptor
**
** @valrule Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensExonadaptorGetFeatureadaptor **************************************
**
** Get the Ensembl Feature Adaptor element of an Ensembl Exon Adaptor.
**
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

EnsPFeatureadaptor ensExonadaptorGetFeatureadaptor(
    const EnsPExonadaptor ea)
{
    if(!ea)
        return NULL;

    return ea->Adaptor;
}




/* @func ensExonadaptorGetDatabaseadaptor *************************************
**
** Get the Ensembl Database Adaptor element of the
** Ensembl Feature Adaptor element of an Ensembl Exon Adaptor.
**
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensExonadaptorGetDatabaseadaptor(
    const EnsPExonadaptor ea)
{
    if(!ea)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(ea->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Exon objects from an
** Ensembl Core database.
**
** @fdata [EnsPExonadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Exon object(s)
** @nam4rule FetchAll Retrieve all Ensembl Exon objects.
** @nam5rule FetchAllBy Retrieve all Ensembl Exon objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Exon object
**                   matching a criterion
**
** @argrule * ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Exon objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensExonadaptorFetchAll ***********************************************
**
** Fetch all Ensembl Exons.
**
** The caller is responsible for deleting the Ensembl Exons before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAll(const EnsPExonadaptor ea, AjPList exons)
{
    EnsPBaseadaptor ba = NULL;

    if(!ea)
        return ajFalse;

    if(!exons)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Adaptor);

    return ensBaseadaptorFetchAll(ba, exons);
}




/* @func ensExonadaptorFetchAllBySlice ****************************************
**
** Fetch all Ensembl Exons on an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Exons before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllBySlice(const EnsPExonadaptor ea,
                                     EnsPSlice slice,
                                     AjPList exons)
{
    EnsPFeatureadaptor fa = NULL;

    if(!ea)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!exons)
        return ajFalse;

    fa = ensExonadaptorGetFeatureadaptor(ea);

    ensFeatureadaptorFetchAllBySlice(fa, slice, (AjPStr) NULL, exons);

    return ajTrue;
}




/* @func ensExonadaptorFetchAllBySliceConstraint ******************************
**
** Fetch all Ensembl Exons matching a SQL constraint on an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Exons before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice_constraint
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] constraint [const AjPStr] SQL constraint
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllBySliceConstraint(const EnsPExonadaptor ea,
                                               EnsPSlice slice,
                                               const AjPStr constraint,
                                               AjPList exons)
{
    EnsPFeatureadaptor fa = NULL;

    if(!ea)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!constraint)
        return ajFalse;

    if(!exons)
        return ajFalse;

    fa = ensExonadaptorGetFeatureadaptor(ea);

    ensFeatureadaptorFetchAllBySliceConstraint(fa,
                                               slice,
                                               constraint,
                                               (AjPStr) NULL,
                                               exons);

    return ajTrue;
}




/* @func ensExonadaptorFetchByIdentifier **************************************
**
** Fetch an Ensembl Exon via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Exon.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] identifier [ajuint] SQL database-internal Exon identifier
** @param [wP] Pexon [EnsPExon*] Ensembl Exon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchByIdentifier(const EnsPExonadaptor ea,
                                       ajuint identifier,
                                       EnsPExon *Pexon)
{
    EnsPBaseadaptor ba = NULL;

    if(!ea)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pexon)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Adaptor);

    *Pexon = (EnsPExon) ensBaseadaptorFetchByIdentifier(ba, identifier);

    return ajTrue;
}




/* @func ensExonadaptorFetchByStableIdentifier ********************************
**
** Fetch an Ensembl Exon via its stable identifier and version.
** In case a particular version is not specified,
** the current Exon will be returned.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::fetch_by_stable_id
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Pexon [EnsPExon*] Ensembl Exon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchByStableIdentifier(const EnsPExonadaptor ea,
                                             const AjPStr stableid,
                                             ajuint version,
                                             EnsPExon *Pexon)
{
    char *txtstableid = NULL;

    AjPList exons = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPExon exon = NULL;

    if(!ea)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!Pexon)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if(version)
        constraint = ajFmtStr(
            "exon_stable_id.stable_id = '%s' "
            "AND "
            "exon_stable_id.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "exon_stable_id.stable_id = '%s' "
            "AND "
            "exon.is_current = 1",
            txtstableid);

    ajCharDel(&txtstableid);

    exons = ajListNew();

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               exons);

    if(ajListGetLength(exons) > 1)
        ajDebug("ensExonadaptorFetchByStableId got more than one "
                "Exon for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(exons, (void **) Pexon);

    while(ajListPop(exons, (void **) &exon))
        ensExonDel(&exon);

    ajListFree(&exons);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensExonadaptorFetchAllByStableIdentifier *****************************
**
** Fetch all Ensembl Exon versions via a stable identifier.
**
** The caller is responsible for deleting the Ensembl Exons before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::fetch_all_versions_by_stable_id
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllByStableIdentifier(const EnsPExonadaptor ea,
                                                const AjPStr stableid,
                                                AjPList exons)
{
    char *txtstableid = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ea)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!exons)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("exon_stable_id.stable_id = '%s'", txtstableid);

    ajCharDel(&txtstableid);

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               exons);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensExonadaptorFetchAllByTranscript ***********************************
**
** Fetch all Ensembl Exons via an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Exons before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::fetch_all_by_Transcript
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllByTranscript(const EnsPExonadaptor ea,
                                          const EnsPTranscript transcript,
                                          AjPList exons)
{
    AjIList iter = NULL;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature tfeature = NULL;

    EnsPSlice eslice    = NULL;
    EnsPSlice tslice    = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("ensExonadaptorFetchAllByTranscript"))
    {
        ajDebug("ensExonadaptorFetchAllByTranscript\n"
                "  ea %p\n"
                "  transcript %p\n"
                "  exons %p\n",
                ea,
                transcript,
                exons);

        ensTranscriptTrace(transcript, 1);
    }

    if(!ea)
        return ajFalse;

    if(!transcript)
        return ajFalse;

    if(!exons)
        return ajFalse;

    tfeature = ensTranscriptGetFeature(transcript);

    tslice = ensFeatureGetSlice(tfeature);

    if(!tslice)
    {
        ajDebug("ensExonadaptorFetchAllByTranscript cannot fetch Exons "
                "for Transcript without Slice.\n");

        return ajFalse;
    }

    dba = ensFeatureadaptorGetDatabaseadaptor(ea->Adaptor);

    sa = ensRegistryGetSliceadaptor(dba);

    /* Get a Slice that spans just this Transcript to place Exons on them. */

    ensSliceadaptorFetchByFeature(sa, tfeature, 0, &eslice);

    /*
    ** Override the table and final condition element of the
    ** Ensembl Feature Adaptor this Exon Adaptor is based upon.
    ** FIXME: This is problematic in a multi-threaded environment.
    ** Create a separate Ensembl Exon Transcript Adaptor?
    */

    ensFeatureadaptorSetTables(ea->Adaptor, exonTranscriptadaptorTables);

    ensFeatureadaptorSetFinalCondition(ea->Adaptor,
                                       exonTranscriptadaptorFinalCondition);

    constraint = ajFmtStr(
        "exon_transcript.transcript_id = %u "
        "AND "
        "exon.exon_id = exon_transcript.exon_id",
        ensTranscriptGetIdentifier(transcript));

    ensFeatureadaptorFetchAllBySliceConstraint(ea->Adaptor,
                                               eslice,
                                               constraint,
                                               (const AjPStr) NULL,
                                               exons);

    /*
    ** Reset the table and final condition element of the
    ** Ensembl Feature Adaptor this Exon Adaptor is based upon.
    */

    ensFeatureadaptorSetTables(ea->Adaptor, exonadaptorTables);

    ensFeatureadaptorSetFinalCondition(ea->Adaptor,
                                       exonadaptorFinalCondition);

    /* Remap Exon coordinates if neccessary. */

    /*
    ** FIXME: What is the point of retrieving Exons first on a Slice that
    ** spans the Transcript only and then map them to the Slice the Transcript
    ** is on? Could this have something to do with haplotypes or PAR regions?
    */

    if(!ensSliceMatch(eslice, tslice))
    {
        iter = ajListIterNew(exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            efeature = ensFeatureTransfer(exon->Feature, tslice);

            ensExonSetFeature(exon, efeature);

            ensFeatureDel(&efeature);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&constraint);

    ensSliceDel(&eslice);

    return ajTrue;
}




/* @func ensExonadaptorFetchAllIdentifiers ************************************
**
** Fetch all SQL database-internal identifiers of Ensembl Exons.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::list_dbIDs
** @param [u] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllIdentifiers(const EnsPExonadaptor ea,
                                         AjPList identifiers)
{
    AjBool value = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ea)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Adaptor);

    table = ajStrNewC("exon");

    value = ensBaseadaptorFetchAllIdentifiers(ba,
                                              table,
                                              (AjPStr) NULL,
                                              identifiers);

    ajStrDel(&table);

    return value;
}




/* @func ensExonadaptorFetchAllStableIdentifiers ******************************
**
** Fetch all stable identifiers of Ensembl Exons.
**
** The caller is responsible for deleting the AJAX Strings before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::list_stable_ids
** @param [r] ea [const EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllStableIdentifiers(const EnsPExonadaptor ea,
                                               AjPList identifiers)
{
    AjBool value = AJFALSE;

    AjPStr table   = NULL;
    AjPStr primary = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ea)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Adaptor);

    table   = ajStrNewC("exon_stable_id");
    primary = ajStrNewC("stable_id");

    value = ensBaseadaptorFetchAllStrings(ba, table, primary, identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return value;
}




/* @datasection [EnsPDatabaseadaptor] Supporting Feature Adaptor **************
**
** Functions for manipulating Ensembl Supporting Feature Adaptor objects
**
** @nam2rule Supportingfeatureadaptor
**
******************************************************************************/




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Base Align Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPDatabaseadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Base Align Feature object(s)
** @nam4rule FetchAll Retrieve all Ensembl Base Align Feature objects
** @nam5rule FetchAllBy Retrieve all Ensembl Base Align Feature objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Base Align Feature object
**                   matching a criterion
**
** @argrule * dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSupportingfeatureadaptorFetchAllByExon ****************************
**
** Fetch Ensembl Supporting Features via an Ensembl Exon.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] exon [EnsPExon] Ensembl Exon
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSupportingfeatureadaptorFetchAllByExon(EnsPDatabaseadaptor dba,
                                                 EnsPExon exon,
                                                 AjPList bafs)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr type      = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature nfeature = NULL;
    EnsPFeature ofeature = NULL;

    EnsPSlice eslice = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDNAAlignFeatureadaptor dafa = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if(!dba)
        return ajFalse;

    if(!exon)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(ensExonGetIdentifier(exon) == 0)
    {
        ajDebug("ensSupportingfeatureadaptorFetchAllByExon cannot get "
                "Supporting Features for an Exon without an identifier.\n");

        return ajFalse;
    }

    efeature = ensExonGetFeature(exon);

    eslice = ensFeatureGetSlice(efeature);

    dafa = ensRegistryGetDNAAlignFeatureadaptor(dba);

    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    statement = ajFmtStr("SELECT "
                         "supporting_feature.feature_type, "
                         "supporting_feature.feature_id "
                         "FROM "
                         "supporting_feature "
                         "WHERE "
                         "supporting_feature.exon_id = %u",
                         ensExonGetIdentifier(exon));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        type = ajStrNew();
        identifier = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &identifier);

        if(ajStrMatchC(type, "dna_align_feature"))
            ensDNAAlignFeatureadaptorFetchByIdentifier(dafa,
                                                       identifier,
                                                       &baf);
        else if(ajStrMatchC(type, "protein_align_feature"))
            ensProteinalignfeatureadaptorFetchByIdentifier(pafa,
                                                           identifier,
                                                           &baf);
        else
            ajWarn("ensSupportingfeatureadaptorFetchAllByExon got "
                   "unexpected value in supporting_feature.feature_type "
                   "'%S'.\n", type);

        if(baf)
        {
            ofeature = ensFeaturepairGetSourceFeature(baf->Featurepair);

            nfeature = ensFeatureTransfer(ofeature, eslice);

            ensFeaturepairSetSourceFeature(baf->Featurepair, nfeature);

            ensFeatureDel(&nfeature);

            ajListPushAppend(bafs, (void *) baf);
        }
        else
            ajDebug("ensSupportingfeatureadaptorFetchAllByExon could not "
                    "retrieve Supporting feature of type '%S' and "
                    "identifier %u from database.\n", type, identifier);

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

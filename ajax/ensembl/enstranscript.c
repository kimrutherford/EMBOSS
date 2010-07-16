/******************************************************************************
**
** @source Ensembl Transcript functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.23 $
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

#include "enstranscript.h"
#include "ensattribute.h"
#include "ensexon.h"
#include "ensgene.h"
#include "ensintron.h"
#include "enssequenceedit.h"
#include "enstranslation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* @datastatic TranscriptPExonRank ********************************************
**
** Ensembl Transcript, Exon and rank associations.
**
** Holds associations between Ensembl Transcripts, Ensembl Exons and their rank
** in the Transcript.
**
** @alias TranscriptSExonRank
** @alias TranscriptOExonRank
**
** @attr TranscriptIdentifier [ajuint] Ensembl Transcript identifier
** @attr Rank [ajint] Ensembl Exon rank
** @@
******************************************************************************/

typedef struct TranscriptSExonRank
{
    ajuint TranscriptIdentifier;
    ajint Rank;
} TranscriptOExonRank;

#define TranscriptPExonRank TranscriptOExonRank*




/* transcriptStatus ***********************************************************
**
** The Ensembl Transcript status element is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsETranscriptStatus.
**
******************************************************************************/

static const char *transcriptStatus[] =
{
    NULL,
    "KNOWN",
    "NOVEL",
    "PUTATIVE",
    "PREDICTED",
    "KNOWN_BY_PROJECTION",
    "UNKNOWN",
    NULL
};




/* transcriptSequenceEditCode *************************************************
**
** Ensembl Sequence Edits for Ensembl Transcripts are a sub-set of
** Ensembl Attributes that provide information about post-transcriptional
** modifications of the Transcript sequence. Attributes with the following
** codes are Sequence Edits on the Transcript-level.
**
** _rna_edit: General cDNA, RNA or Transcript sequence edit
**
******************************************************************************/

static const char *transcriptSequenceEditCode[] =
{
    "_rna_edit",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @funcstatic transcriptExonRankNew **********************************
**
** Default constructor for an Ensembl Transcripts Exon rank association.
**
** @param [r] trid [ajuint] Ensembl Transcript identifier
** @param [r] rank [ajint] Ensembl Exon rank
**
** @return [TranscriptPExonRank] Transcript and Exon rank association
** @@
******************************************************************************/

static TranscriptPExonRank transcriptExonRankNew(ajuint trid, ajint rank)
{
    TranscriptPExonRank trex = NULL;

    AJNEW0(trex);

    trex->TranscriptIdentifier = trid;
    trex->Rank = rank;

    return trex;
}




/* @funcstatic transcriptExonRankDel **********************************
**
** Default destructor for an Ensembl Transcripts Exon rank association.
**
** @param [d] Ptrex [TranscriptPExonRank*] Ensembl Exon rank object address
**
** @return [void]
** @@
******************************************************************************/

static void transcriptExonRankDel(TranscriptPExonRank *Ptrex)
{
    if(!Ptrex)
        return;

    if(!*Ptrex)
        return;

    AJFREE(*Ptrex);

    return;
}




static int transcriptCompareStartAscending(const void* P1, const void* P2);

static int transcriptCompareStartDescending(const void* P1, const void* P2);

static AjBool transcriptadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList transcripts);

static void *transcriptadaptorCacheReference(void *value);

static void transcriptadaptorCacheDelete(void **value);

static ajulong transcriptadaptorCacheSize(const void *value);

static EnsPFeature transcriptadaptorGetFeature(const void *value);




/* @filesection enstranscript *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPTranscript] Transcript ***********************************
**
** Functions for manipulating Ensembl Transcript objects
**
** @cc Bio::EnsEMBL::Transcript CVS Revision: 1.292
**
** @nam2rule Transcript
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Transcript by pointer.
** It is the responsibility of the user to first destroy any previous
** Transcript. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPTranscript]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [const EnsPTranscript] Ensembl Transcript
** @argrule Ref object [EnsPTranscript] Ensembl Transcript
**
** @valrule * [EnsPTranscript] Ensembl Transcript
**
** @fcategory new
******************************************************************************/




/* @func ensTranscriptNew *****************************************************
**
** Default Ensembl Transcript constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Transcript::new
** @param [u] displaydbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] description [AjPStr] Description
** @param [u] biotype [AjPStr] Biotype
** @param [r] status [EnsETranscriptStatus] Status
** @param [r] current [AjBool] Current attribute
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNew(EnsPTranscriptadaptor tca,
                                ajuint identifier,
                                EnsPFeature feature,
                                EnsPDatabaseentry displaydbe,
                                AjPStr description,
                                AjPStr biotype,
                                EnsETranscriptStatus status,
                                AjBool current,
                                AjPStr stableid,
                                ajuint version,
                                AjPStr cdate,
                                AjPStr mdate,
                                AjPList exons)
{
    AjIList iter = NULL;

    EnsPExon exon = NULL;

    EnsPTranscript transcript = NULL;

    if(!feature)
        return NULL;

    AJNEW0(transcript);

    transcript->Use = 1;

    transcript->Identifier = identifier;

    transcript->Adaptor = tca;

    transcript->Feature = ensFeatureNewRef(feature);

    transcript->DisplayReference = ensDatabaseentryNewRef(displaydbe);

    if(description)
        transcript->Description = ajStrNewRef(description);

    if(biotype)
        transcript->BioType = ajStrNewRef(biotype);

    transcript->Status = status;

    transcript->Current = current;

    if(stableid)
        transcript->StableIdentifier = ajStrNewRef(stableid);

    if(cdate)
        transcript->CreationDate = ajStrNewRef(cdate);

    if(mdate)
        transcript->ModificationDate = ajStrNewRef(mdate);

    transcript->Version = version;

    transcript->GeneIdentifier = 0;

    transcript->AlternativeTranslations = NULL;

    transcript->Attributes = NULL;

    transcript->DatabaseEntries = NULL;

    if(exons && ajListGetLength(exons))
    {
        transcript->Exons = ajListNew();

        iter = ajListIterNew(exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ajListPushAppend(transcript->Exons,
                             (void *) ensExonNewRef(exon));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->Exons = NULL;

    transcript->Supportingfeatures = NULL;

    transcript->Translation = NULL;

    transcript->SliceCodingStart = 0;

    transcript->SliceCodingEnd = 0;

    transcript->TranscriptCodingStart = 0;

    transcript->TranscriptCodingEnd = 0;

    /*
    ** NOTE: Sequence Edits such as transcription and translation exceptions
    ** are applied by default. Use ensTranscriptGetEnableSequenceEdits and
    ** ensTranscriptSetEnableSequenceEdits to toggle Sequence Edit application.
    */

    transcript->EnableSequenceEdits = ajTrue;

    transcript->StartPhase = 0;

    transcript->ExonCoordMapper = NULL;

    return transcript;
}




/* @func ensTranscriptNewObj **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNewObj(const EnsPTranscript object)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPTranscript transcript = NULL;

    EnsPTranslation translation = NULL;

    if(!object)
        return NULL;

    AJNEW0(transcript);

    transcript->Use = 1;

    transcript->Identifier = object->Identifier;

    transcript->Adaptor = object->Adaptor;

    transcript->Feature = ensFeatureNewRef(object->Feature);

    transcript->DisplayReference = ensDatabaseentryNewRef(
        object->DisplayReference);

    if(object->Description)
        transcript->Description = ajStrNewRef(object->Description);

    if(object->BioType)
        transcript->BioType = ajStrNewRef(object->BioType);

    transcript->Status = object->Status;

    transcript->Current = object->Current;

    if(object->StableIdentifier)
        transcript->StableIdentifier = ajStrNewRef(object->StableIdentifier);

    if(object->CreationDate)
        transcript->CreationDate = ajStrNewRef(object->CreationDate);

    if(object->ModificationDate)
        transcript->ModificationDate = ajStrNewRef(object->ModificationDate);

    transcript->Version = object->Version;

    transcript->GeneIdentifier = object->GeneIdentifier;

    /* Copy the AJAX List of alternative Ensembl Translations. */

    if(object->AlternativeTranslations &&
       ajListGetLength(object->AlternativeTranslations))
    {
        transcript->AlternativeTranslations = ajListNew();

        iter = ajListIterNew(object->AlternativeTranslations);

        while(!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            ajListPushAppend(transcript->AlternativeTranslations,
                             (void *) ensTranslationNewRef(translation));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->AlternativeTranslations = NULL;

    /* Copy the AJAX List of Ensembl Attributes. */

    if(object->Attributes && ajListGetLength(object->Attributes))
    {
        transcript->Attributes = ajListNew();

        iter = ajListIterNew(object->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(transcript->Attributes,
                             (void *) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->Attributes = NULL;

    /* Copy the AJAX List of Ensembl Database Entries. */

    if(object->DatabaseEntries && ajListGetLength(object->DatabaseEntries))
    {
        transcript->DatabaseEntries = ajListNew();

        iter = ajListIterNew(object->DatabaseEntries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ajListPushAppend(transcript->DatabaseEntries,
                             (void *) ensDatabaseentryNewRef(dbe));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->DatabaseEntries = NULL;

    /* Copy the AJAX List of Ensembl Exons. */

    if(object->Exons && ajListGetLength(object->Exons))
    {
        transcript->Exons = ajListNew();

        iter = ajListIterNew(object->Exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ajListPushAppend(transcript->Exons, (void *) ensExonNewRef(exon));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->Exons = NULL;

    /* Copy the AJAX List of supporting Ensembl Base Align Features. */

    if(object->Supportingfeatures &&
       ajListGetLength(object->Supportingfeatures))
    {
        transcript->Supportingfeatures = ajListNew();

        iter = ajListIterNew(object->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListPushAppend(transcript->Supportingfeatures,
                             (void *) ensBasealignfeatureNewRef(baf));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->Supportingfeatures = NULL;

    transcript->Translation = ensTranslationNewRef(object->Translation);

    transcript->SliceCodingStart = object->SliceCodingStart;

    transcript->SliceCodingEnd = object->SliceCodingEnd;

    transcript->TranscriptCodingStart = object->TranscriptCodingStart;

    transcript->TranscriptCodingEnd = object->TranscriptCodingEnd;

    transcript->EnableSequenceEdits = object->EnableSequenceEdits;

    transcript->StartPhase = object->StartPhase;

    /*
    ** TODO: The ExonCoordMapper is missing, but how should the
    ** Ensembl Mapper be copied? Should we really copy it deeply?
    */

    transcript->ExonCoordMapper = NULL;

    return transcript;
}




/* @func ensTranscriptNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNewRef(EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    transcript->Use++;

    return transcript;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Transcript.
**
** @fdata [EnsPTranscript]
** @fnote None
**
** @nam3rule Del Destroy (free) a Transcript object
**
** @argrule * Ptranscript [EnsPTranscript*] Transcript object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTranscriptDel *****************************************************
**
** Default destructor for an Ensembl Transcript.
**
** @param [d] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [void]
** @@
******************************************************************************/

void ensTranscriptDel(EnsPTranscript *Ptranscript)
{
    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPTranscript pthis = NULL;

    EnsPTranslation translation = NULL;

    if(!Ptranscript)
        return;

    if(!*Ptranscript)
        return;

    if(ajDebugTest("ensTranscriptDel"))
        ajDebug("ensTranscriptDel\n"
                "  *Ptranscript %p\n",
                *Ptranscript);

    pthis = *Ptranscript;

    pthis->Use--;

    if(pthis->Use)
    {
        *Ptranscript = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensDatabaseentryDel(&pthis->DisplayReference);

    ajStrDel(&pthis->Description);

    ajStrDel(&pthis->BioType);

    ajStrDel(&pthis->StableIdentifier);

    ajStrDel(&pthis->CreationDate);

    ajStrDel(&pthis->ModificationDate);

    /* Clear and delete the AJAX List of alternative Ensembl Translations. */

    while(ajListPop(pthis->AlternativeTranslations, (void **) &translation))
        ensTranslationDel(&translation);

    ajListFree(&pthis->AlternativeTranslations);

    /* Clear and delete the AJAX List of Ensembl Attributes. */

    while(ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and delete the AJAX List of Ensembl Database Entries. */

    while(ajListPop(pthis->DatabaseEntries, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->DatabaseEntries);

    /* Clear and delete the AJAX List of Ensembl Exons. */

    while(ajListPop(pthis->Exons, (void **) &exon))
        ensExonDel(&exon);

    ajListFree(&pthis->Exons);

    /* Clear and delete the AJAX List of Ensembl Base Align Features. */

    while(ajListPop(pthis->Supportingfeatures, (void **) &baf))
        ensBasealignfeatureDel(&baf);

    ajListFree(&pthis->Supportingfeatures);

    ensTranslationDel(&pthis->Translation);

    ensMapperDel(&pthis->ExonCoordMapper);

    AJFREE(pthis);

    *Ptranscript = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
** @fnote None
**
** @nam3rule Get Return Transcript attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Transcript Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Feature
** @nam4rule GetDisplayReference Return the display Database Entry
** @nam4rule GetDescription Return the description
** @nam4rule GetBioType Return the biological type
** @nam4rule GetStatus Return the status
** @nam4rule GetCurrent Return the current element
** @nam4rule GetStableIdentifier Return the stable identifier
** @nam4rule GetVersion Return the version
** @nam4rule GetCreationDate Return the creation date
** @nam4rule GetModificationDate Return the modification date
** @nam4rule GetAlternativeTransaltions Return all alternative
**                                      Ensembl Translations
** @nam4rule GetAttributes Return all Ensembl Attributes
** @nam4rule GetDatabaseEntries Return all Ensembl Database Entries
** @nam4rule GetExons Return all Ensembl Exons
** @nam4rule GetSupportingfeatures Return the supporting Ensembl Base Align
**                                 Features
** @nam4rule GetTranslation Return the Ensembl Translation
** @nam4rule GetTranscriptCodingStart Return the Translation start coordinate
**                                    in Transcript coordinates
** @nam4rule GetTranscriptCodingEnd Return the Translation end coordinate
**                                  in Transcript coordinates
** @nam4rule GetSliceCodingStart Return the Translation start coordinate
**                               in Slice coordinates
** @nam4rule GetSliceCodingEnd Return the Translation end coordinate
**                             in Slice coordinates
** @nam4rule GetEnableSequenceEdits Return enable Ensembl Sequence Edits
**
** @argrule * transcript [const EnsPTranscript] Transcript
**
** @valrule Adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule DisplayReference [EnsPDatabaseentry] Ensembl Database Entry
** @valrule Description [AjPStr] Description
** @valrule BioType [ajuint] Biological type
** @valrule Status [EnsETranscriptStatus] Status
** @valrule GeneIdentifier [ajuint] Ensembl Gene identifier
** @valrule Current [AjBool] Current attribute
** @valrule StableIdentifier [AjPStr] Stable identifier
** @valrule Version [ajuint] Version
** @valrule CreationDate [AjPStr] Creation date
** @valrule ModificationDate [AjPStr] Modification date
** @valrule Attributes [const AjPList] AJAX List of Ensembl Attributes
** @valrule DatabaseEntries [const AjPList] AJAX List of
**                                          Ensembl Database Entries
** @valrule Exons [const AjPList] AJAX List of Ensembl Exons
** @valrule Supportingfeatures [const AjPList] AJAX List of Ensembl Base
**                                       Align Features
** @valrule Translation [EnsPTranslation] Ensembl Translation
** @valrule TranscriptCodingStart [ajuint] Translation start coordinate
** @valrule TranscriptCodingEnd [ajuint] Translation end coordinate
** @valrule SliceCodingStart [ajuint] Translation start coordinate
** @valrule SliceCodingEnd [ajuint] Translation end coordinate
** @valrule EnableSequenceEdits [AjBool] Enable Ensembl Sequence Edits
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptGetAdaptor **********************************************
**
** Get the Ensembl Transcript Adaptor element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptGetAdaptor(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Adaptor;
}




/* @func ensTranscriptGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensTranscriptGetIdentifier(const EnsPTranscript transcript)
{
    if(!transcript)
        return 0;

    return transcript->Identifier;
}




/* @func ensTranscriptGetFeature **********************************************
**
** Get the Ensembl Feature element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Feature
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensTranscriptGetFeature(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Feature;
}




/* @func ensTranscriptGetDisplayReference *************************************
**
** Get the display reference element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::display_xref
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPDatabaseentry] Ensembl Database Entry
** @@
******************************************************************************/

EnsPDatabaseentry ensTranscriptGetDisplayReference(
    const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->DisplayReference;
}




/* @func ensTranscriptGetDescription ******************************************
**
** Get the description element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::description
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensTranscriptGetDescription(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Description;
}




/* @func ensTranscriptGetBioType **********************************************
**
** Get the biological type element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::biotype
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Biological type
** @@
******************************************************************************/

AjPStr ensTranscriptGetBioType(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->BioType;
}




/* @func ensTranscriptGetStatus ***********************************************
**
** Get the status element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::status
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsETranscriptStatus] Status or ensETranscriptStatusNULL
** @@
******************************************************************************/

EnsETranscriptStatus ensTranscriptGetStatus(const EnsPTranscript transcript)
{
    if(!transcript)
        return ensETranscriptStatusNULL;

    return transcript->Status;
}




/* @func ensTranscriptGetGeneIdentifier ***************************************
**
** Get the SQL database-internal Ensembl Gene identifier element of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::???
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Internal database identifier of the Ensembl Gene this
** Transcript is a member of
** @@
******************************************************************************/

ajuint ensTranscriptGetGeneIdentifier(const EnsPTranscript transcript)
{
    if(!transcript)
        return 0;

    return transcript->GeneIdentifier;
}




/* @func ensTranscriptGetCurrent **********************************************
**
** Get the current element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_current
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue if this Transcript reflects the current state of
**                  annotation
** @@
******************************************************************************/

AjBool ensTranscriptGetCurrent(const EnsPTranscript transcript)
{
    if(!transcript)
        return ajFalse;

    return transcript->Current;
}




/* @func ensTranscriptGetStableIdentifier *************************************
**
** Get the stable identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::stable_id
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Stable identifier
** @@
******************************************************************************/

AjPStr ensTranscriptGetStableIdentifier(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->StableIdentifier;
}




/* @func ensTranscriptGetVersion **********************************************
**
** Get the version element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::version
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Version
** @@
******************************************************************************/

ajuint ensTranscriptGetVersion(const EnsPTranscript transcript)
{
    if(!transcript)
        return 0;

    return transcript->Version;
}




/* @func ensTranscriptGetCreationDate *****************************************
**
** Get the creation date element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::created_date
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Creation date
** @@
******************************************************************************/

AjPStr ensTranscriptGetCreationDate(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->CreationDate;
}




/* @func ensTranscriptGetModificationDate *************************************
**
** Get the modification date element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::modified_date
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Modification date
** @@
******************************************************************************/

AjPStr ensTranscriptGetModificationDate(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->ModificationDate;
}




/* @func ensTranscriptGetAlternativeTranslations ******************************
**
** Get all alternative Ensembl Translations of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch Ensembl Translations
** from the Ensembl Core database in case the AJAX List is empty.
**
**
** @cc Bio::EnsEMBL::Transcript::get_all_alternative_translations
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllAttributes
**
** @return [const AjPList] AJAX List of Ensembl Translations
** @@
******************************************************************************/

const AjPList ensTranscriptGetAlternativeTranslations(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranslationadaptor tla  = NULL;

    if(!transcript)
        return NULL;

    if(transcript->AlternativeTranslations)
        return transcript->AlternativeTranslations;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptGetAlternativeTranslations cannot fetch "
                "Ensembl Translations for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if(!dba)
    {
        ajDebug("ensTranscriptGetAlternativeTranslations cannot fetch "
                "Ensembl Translations for a Transcript without a "
                "Database Adaptor set in the Transcript Adaptor.\n");

        return NULL;
    }

    tla = ensRegistryGetTranslationadaptor(dba);

    transcript->AlternativeTranslations = ajListNew();

    ensTranslationadaptorFetchAllByTranscript(tla, transcript);

    return transcript->AlternativeTranslations;
}




/* @func ensTranscriptGetAttributes *******************************************
**
** Get all Ensembl Attributes of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch Ensembl Attributes
** from the Ensembl Core database in case the AJAX List is empty.
**
** To filter Ensembl Attributes via their code, consider using
** ensTranscriptFetchAllAttributes.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Attributes
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllAttributes
**
** @return [const AjPList] AJAX List of Ensembl Attributes
** @@
******************************************************************************/

const AjPList ensTranscriptGetAttributes(EnsPTranscript transcript)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Attributes)
        return transcript->Attributes;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptGetAttributes cannot fetch "
                "Ensembl Attributes for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if(!dba)
    {
        ajDebug("ensTranscriptGetAttributes cannot fetch "
                "Ensembl Attributes for a Transcript without a "
                "Database Adaptor set in the Transcript Adaptor.\n");

        return NULL;
    }

    ata = ensRegistryGetAttributeadaptor(dba);

    transcript->Attributes = ajListNew();

    ensAttributeadaptorFetchAllByTranscript(ata,
                                            transcript,
                                            (const AjPStr) NULL,
                                            transcript->Attributes);

    return transcript->Attributes;
}




/* @func ensTranscriptGetDatabaseEntries **************************************
**
** Get all Ensembl Database Entries of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Database Entries from the Ensembl Core database associated with the
** Transcript Adaptor.
**
** To filter Ensembl Database Entires via an Ensembl External Database name or
** type, consider using ensTranscriptFetchAllDatabaseEntries.
**
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllDatabaseEntries
**
** @return [const AjPList] AJAX List of Ensembl Database Entries
** @@
******************************************************************************/

const AjPList ensTranscriptGetDatabaseEntries(EnsPTranscript transcript)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentryadaptor dbea = NULL;

    if(!transcript)
        return NULL;

    if(transcript->DatabaseEntries)
        return transcript->DatabaseEntries;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptGetDatabaseEntries cannot fetch "
                "Ensembl Database Entries for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if(!dba)
    {
        ajDebug("ensTranscriptGetDatabaseEntries cannot fetch "
                "Ensembl Database Entries for a Transcript without a "
                "Database Adaptor set in the Transcript Adaptor.\n");

        return NULL;
    }

    dbea = ensRegistryGetDatabaseentryadaptor(dba);

    objtype = ajStrNewC("Transcript");

    transcript->DatabaseEntries = ajListNew();

    ensDatabaseentryadaptorFetchAllByObjectType(dbea,
                                                transcript->Identifier,
                                                objtype,
                                                (AjPStr) NULL,
                                                ensEExternaldatabaseTypeNULL,
                                                transcript->DatabaseEntries);

    ajStrDel(&objtype);

    return transcript->DatabaseEntries;
}




/* @func ensTranscriptGetExons ************************************************
**
** Get all Ensembl Exons of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt fetching the
** Exons from the Ensembl Core database associated with the
** Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Exons
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Exons
** @@
** NOTE: This implementation does not support the constitutive parameter.
** Please use the ensTranscriptFetchAllConstitutiveExons function instead.
******************************************************************************/

const AjPList ensTranscriptGetExons(EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPExonadaptor ea = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Exons)
        return transcript->Exons;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptGetExons cannot fetch "
                "Ensembl Exons for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    ea = ensRegistryGetExonadaptor(dba);

    transcript->Exons = ajListNew();

    ensExonadaptorFetchAllByTranscript(ea, transcript, transcript->Exons);

    return transcript->Exons;
}




/* @func ensTranscriptGetSupportingfeatures ***********************************
**
** Get all Ensembl Supporting Features of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** supporting Base Align Features from the Ensembl Core database associated
** with the Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::get_all_supporting_features
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Base Align Features
** @@
******************************************************************************/

const AjPList ensTranscriptGetSupportingfeatures(EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Supportingfeatures)
        return transcript->Supportingfeatures;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptGetSupportingfeatures cannot fetch "
                "Ensembl Base Align Features for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    transcript->Supportingfeatures = ajListNew();

    ensSupportingfeatureadaptorFetchAllByTranscript(
        dba,
        transcript,
        transcript->Supportingfeatures);

    return transcript->Supportingfeatures;
}




/* @func ensTranscriptGetTranslation ******************************************
**
** Get the Ensembl Translation of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Translation from the Ensembl Core database associated
** with the Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranslation] Ensembl Translations or NULL
** @@
******************************************************************************/

EnsPTranslation ensTranscriptGetTranslation(EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranslationadaptor tla = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Translation)
        return transcript->Translation;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptGetTranslation cannot fetch an "
                "Ensembl Translation for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    tla = ensRegistryGetTranslationadaptor(dba);

    ensTranslationadaptorFetchByTranscript(tla, transcript);

    return transcript->Translation;
}




/* @func ensTranscriptGetTranscriptCodingStart ********************************
**
** Get the start position of the coding region in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Transcript::cdna_coding_start
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Coding region start in Transcript coordinates or 0,
** if this Transcript has no Translation
** @@
******************************************************************************/

ajuint ensTranscriptGetTranscriptCodingStart(EnsPTranscript transcript)
{
    AjBool debug = AJFALSE;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceEdit se = NULL;

    EnsPTranslation translation = NULL;

    debug = ajDebugTest("ensTranscriptGetTranscriptCodingStart");

    if(debug)
        ajDebug("ensTranscriptGetTranscriptCodingStart\n"
                "  transcript %p\n",
                transcript);

    if(!transcript)
        return 0;

    if(transcript->TranscriptCodingStart)
        return transcript->TranscriptCodingStart;

    translation = ensTranscriptGetTranslation(transcript);

    if(!translation)
        return 0;

    /*
    ** Calculate the coding start relative to the start of the
    ** Translation in Transcript coordinates.
    */

    exons = ensTranscriptGetExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if(debug)
            ajDebug("ensTranscriptGetTranscriptCodingStart "
                    "exon %p start exon %p\n",
                    exon, ensTranslationGetStartExon(translation));

        if(exon == ensTranslationGetStartExon(translation))
        {
            /* Add the UTR portion of the first coding Exon. */

            transcript->TranscriptCodingStart +=
                ensTranslationGetStart(translation);

            break;
        }
        else
        {
            /* Add the entire length of this non-coding Exon. */

            feature = ensExonGetFeature(exon);

            transcript->TranscriptCodingStart +=
                ensFeatureGetLength(feature);
        }
    }

    ajListIterDel(&iter);

    /* Adjust Transcript coordinates if Sequence Edits are enabled. */

    if(transcript->EnableSequenceEdits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceEdits(transcript, ses);

        /*
        ** Sort in reverse order to avoid adjustment of down-stream
        ** Sequence Edits.
        */

        ensSequenceEditSortByStartDescending(ses);

        while(ajListPop(ses, (void **) &se))
        {
            if(ensSequenceEditGetStart(se) <
               transcript->TranscriptCodingStart)
                transcript->TranscriptCodingStart +=
                    ensSequenceEditGetLengthDifference(se);

            ensSequenceEditDel(&se);
        }

        ajListFree(&ses);
    }

    return transcript->TranscriptCodingStart;
}




/* @func ensTranscriptGetTranscriptCodingEnd **********************************
**
** Get the end position of the coding region in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Transcript::cdna_coding_end
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Coding region end in Transcript coordinates or 0,
** if this Transcript has no Translation
** @@
******************************************************************************/

ajuint ensTranscriptGetTranscriptCodingEnd(EnsPTranscript transcript)
{
    AjBool debug = AJFALSE;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceEdit se = NULL;

    EnsPTranslation translation = NULL;

    debug = ajDebugTest("ensTranscriptGetTranscriptCodingEnd");

    if(debug)
        ajDebug("ensTranscriptGetTranscriptCodingEnd\n"
                "  transcript %p\n",
                transcript);

    if(!transcript)
        return 0;

    if(transcript->TranscriptCodingEnd)
        return transcript->TranscriptCodingEnd;

    translation = ensTranscriptGetTranslation(transcript);

    if(!translation)
        return 0;

    /*
    ** Calculate the coding start relative to the start of the
    ** Translation in Transcript coordinates.
    */

    exons = ensTranscriptGetExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if(debug)
            ajDebug("ensTranscriptGetTranscriptCodingEnd "
                    "exon %p end exon %p\n",
                    exon, ensTranslationGetEndExon(translation));

        if(exon == ensTranslationGetEndExon(translation))
        {
            /* Add the coding portion of the last coding Exon. */

            transcript->TranscriptCodingEnd +=
                ensTranslationGetEnd(translation);

            break;
        }
        else
        {
            /* Add the entire length of this Exon. */

            feature = ensExonGetFeature(exon);

            transcript->TranscriptCodingEnd +=
                ensFeatureGetLength(feature);
        }
    }

    ajListIterDel(&iter);

    /* Adjust Transcript coordinates if Sequence Edits are enabled. */

    if(transcript->EnableSequenceEdits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceEdits(transcript, ses);

        /*
        ** Sort in reverse order to avoid adjustment of down-stream
        ** Sequence Edits.
        */

        ensSequenceEditSortByStartDescending(ses);

        while(ajListPop(ses, (void **) &se))
        {
            /*
            ** Use less than or equal to end + 1 so that the end of the
            ** CDS can be extended.
            */

            if(ensSequenceEditGetStart(se) <=
               transcript->TranscriptCodingEnd + 1)
                transcript->TranscriptCodingEnd +=
                    ensSequenceEditGetLengthDifference(se);

            ensSequenceEditDel(&se);
        }

        ajListFree(&ses);
    }

    return transcript->TranscriptCodingEnd;
}




/* @func ensTranscriptGetSliceCodingStart *************************************
**
** Get the start position of the coding region in Slice coordinates.
**
** @cc Bio::EnsEMBL::Transcript::coding_region_start
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Coding region start in Slice coordinates
** @@
******************************************************************************/

ajuint ensTranscriptGetSliceCodingStart(EnsPTranscript transcript)
{
    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return 0;

    if(transcript->SliceCodingStart)
        return transcript->SliceCodingStart;

    translation = ensTranscriptGetTranslation(transcript);

    if(translation)
    {
        /* Calulate the Slice coding start from the Translation. */

        exon = ensTranslationGetStartExon(translation);

        feature = ensExonGetFeature(exon);

        if(ensFeatureGetStrand(feature) >= 0)
        {
            exon = ensTranslationGetStartExon(translation);

            feature = ensExonGetFeature(exon);

            transcript->SliceCodingStart = ensFeatureGetStart(feature);

            transcript->SliceCodingStart +=
                (ensTranslationGetStart(translation) - 1);
        }
        else
        {
            exon = ensTranslationGetEndExon(translation);

            feature = ensExonGetFeature(exon);

            transcript->SliceCodingStart = ensFeatureGetEnd(feature);

            transcript->SliceCodingStart -=
                (ensTranslationGetEnd(translation) - 1);
        }
    }

    return transcript->SliceCodingStart;
}




/* @func ensTranscriptGetSliceCodingEnd ***************************************
**
** Get the end position of the coding region in Slice coordinates.
**
** @cc Bio::EnsEMBL::Transcript::coding_region_end
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Coding region end in Slice coordinates
** @@
******************************************************************************/

ajuint ensTranscriptGetSliceCodingEnd(EnsPTranscript transcript)
{
    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return 0;

    if(transcript->SliceCodingEnd)
        return transcript->SliceCodingEnd;

    translation = ensTranscriptGetTranslation(transcript);

    if(translation)
    {
        /* Calulate the Slice coding start from the Translation. */

        exon = ensTranslationGetStartExon(translation);

        feature = ensExonGetFeature(exon);

        if(ensFeatureGetStrand(feature) >= 0)
        {
            exon = ensTranslationGetEndExon(translation);

            feature = ensExonGetFeature(exon);

            transcript->SliceCodingEnd = ensFeatureGetStart(feature);

            transcript->SliceCodingEnd +=
                (ensTranslationGetEnd(translation) - 1);
        }
        else
        {
            exon = ensTranslationGetStartExon(translation);

            feature = ensExonGetFeature(exon);

            transcript->SliceCodingEnd = ensFeatureGetEnd(feature);

            transcript->SliceCodingEnd -=
                (ensTranslationGetStart(translation) - 1);
        }
    }

    return transcript->SliceCodingEnd;
}




/* @func ensTranscriptGetEnableSequenceEdits **********************************
**
** Get the enable Ensembl Sequence Edits element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::edits_enabled
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue: Enable Ensembl Sequence Edits to the sequence
** @@
******************************************************************************/

AjBool ensTranscriptGetEnableSequenceEdits(EnsPTranscript transcript)
{
    if(!transcript)
        return ajFalse;

    return transcript->EnableSequenceEdits;
}




/* @func ensTranscriptGetLength ***********************************************
**
** Get the length of an Ensembl Transcript, which is the sum of the length of
** all Ensembl Exons of this transcript.
**
** @cc Bio::EnsEMBL::Transcript::length
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Transcript (cDNA) length
** @@
******************************************************************************/

ajuint ensTranscriptGetLength(EnsPTranscript transcript)
{
    ajuint length = 0;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceEdit se = NULL;

    if(!transcript)
        return 0;

    exons = ensTranscriptGetExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        feature = ensExonGetFeature(exon);

        length += ensFeatureGetLength(feature);
    }

    ajListIterDel(&iter);

    /* Adjust the length if post-transcriptional Sequence Edits are enabled. */

    if(transcript->EnableSequenceEdits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceEdits(transcript, ses);

        while(ajListPop(ses, (void **) &se))
        {
            length += ensSequenceEditGetLengthDifference(se);

            ensSequenceEditDel(&se);
        }

        ajListFree(&ses);
    }

    return length;
}




/* @func ensTranscriptGetMemsize **********************************************
**
** Get the memory size in bytes of an Ensembl Transcript.
**
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensTranscriptGetMemsize(const EnsPTranscript transcript)
{
    ajulong size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return 0;

    size += sizeof (EnsOTranscript);

    size += ensFeatureGetMemsize(transcript->Feature);

    size += ensDatabaseentryGetMemsize(transcript->DisplayReference);

    if(transcript->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Description);
    }

    if(transcript->BioType)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->BioType);
    }

    if(transcript->StableIdentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->StableIdentifier);
    }

    if(transcript->CreationDate)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->CreationDate);
    }

    if(transcript->ModificationDate)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->ModificationDate);
    }

    /* Summarise the AJAX List of alternative Ensembl Translations. */

    if(transcript->AlternativeTranslations)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->AlternativeTranslations);

        while(!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            size += ensTranslationGetMemsize(translation);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Attributes. */

    if(transcript->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeGetMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entries. */

    if(transcript->DatabaseEntries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->DatabaseEntries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryGetMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Exons. */

    if(transcript->Exons)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            size += ensExonGetMemsize(exon);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of supporting Ensembl Base Align Features. */

    if(transcript->Supportingfeatures)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            size += ensBasealignfeatureGetMemsize(baf);
        }

        ajListIterDel(&iter);
    }

    size += ensTranslationGetMemsize(transcript->Translation);

    size += ensMapperGetMemsize(transcript->ExonCoordMapper);

    return size;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
** @fnote None
**
** @nam3rule Set Set one element of a Transcript
** @nam4rule SetAdaptor Set the Ensembl Transcript Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetDisplayReference Set the display Ensembl Database Entry
** @nam4rule SetDescription Set the description
** @nam4rule SetBioType Set the biological type
** @nam4rule SetStatus Set the status
** @nam4rule SetGeneIdentifier Set the Ensembl Gene identifier
** @nam4rule SetCurrent Set the current element
** @nam4rule SetStableIdentifier Set the stable identifier
** @nam4rule SetVersion Set the version
** @nam4rule SetCreationDate Set the creation date
** @nam4rule SetModificationDate Set the modification date
** @nam4rule SetTranslation Set the Ensembl Translation
** @nam4rule SetEnableSequenceEdits Set enable Ensembl Sequence Edits
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensTranscriptSetAdaptor **********************************************
**
** Set the Ensembl Transcript Adaptor element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetAdaptor(EnsPTranscript transcript,
                               EnsPTranscriptadaptor tca)
{
    if(!transcript)
        return ajFalse;

    transcript->Adaptor = tca;

    return ajTrue;
}




/* @func ensTranscriptSetIdentifier *******************************************
**
** Set the SQL database-internal identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetIdentifier(EnsPTranscript transcript, ajuint identifier)
{
    if(!transcript)
        return ajFalse;

    transcript->Identifier = identifier;

    return ajTrue;
}




/* @func ensTranscriptSetFeature **********************************************
**
** Set the Ensembl Feature element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Feature
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetFeature(EnsPTranscript transcript, EnsPFeature feature)
{
    AjIList iter = NULL;

    EnsPBasealignfeature oldbaf = NULL;
    EnsPBasealignfeature newbaf = NULL;

    EnsPExon oldexon = NULL;
    EnsPExon newexon = NULL;

    EnsPSlice slice = NULL;

    if(ajDebugTest("ensTranscriptSetFeature"))
    {
        ajDebug("ensTranscriptSetFeature\n"
                "  transcript %p\n"
                "  feature %p\n",
                transcript,
                feature);

        ensTranscriptTrace(transcript, 1);

        ensFeatureTrace(feature, 1);
    }

    if(!transcript)
        return ajFalse;

    if(!feature)
        return ajFalse;

    /* Replace the current Feature. */

    if(transcript->Feature)
        ensFeatureDel(&transcript->Feature);

    transcript->Feature = ensFeatureNewRef(feature);

    slice = ensFeatureGetSlice(transcript->Feature);

    /*
    ** Transfer Exons onto the new Feature Slice and thereby also adjust
    ** the start and end Exons of the Translation if it is defined.
    */

    if(transcript->Exons)
    {
        iter = ajListIterNew(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            oldexon = (EnsPExon) ajListIterGet(iter);

            ajListIterRemove(iter);

            newexon = ensExonTransfer(oldexon, slice);

            if(!newexon)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Exon onto new Ensembl Feature Slice.");

                ensExonTrace(oldexon, 1);
            }

            ajListIterInsert(iter, (void *) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            /* Re-assign the start and end Exons of the Ensembl Translation. */

            if(transcript->Translation)
            {
                if(oldexon ==
                   ensTranslationGetStartExon(transcript->Translation))
                    ensTranslationSetStartExon(transcript->Translation,
                                               newexon);

                if(oldexon ==
                   ensTranslationGetEndExon(transcript->Translation))
                    ensTranslationSetEndExon(transcript->Translation,
                                             newexon);
            }

            ensExonDel(&oldexon);
        }

        ajListIterDel(&iter);
    }

    /* Transfer Base Align Features onto the new Feature Slice. */

    if(transcript->Supportingfeatures)
    {
        iter = ajListIterNew(transcript->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListIterRemove(iter);

            newbaf = ensBasealignfeatureTransfer(oldbaf, slice);

            if(!newbaf)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Base Align Feature onto new Ensembl Feature Slice.");

                ensBasealignfeatureTrace(oldbaf, 1);
            }

            ajListIterInsert(iter, (void *) newbaf);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensBasealignfeatureDel(&oldbaf);
        }

        ajListIterDel(&iter);
    }

    /* Clear internal values that depend on Exon coordinates. */

    transcript->SliceCodingStart = 0;
    transcript->SliceCodingEnd   = 0;

    transcript->TranscriptCodingStart = 0;
    transcript->TranscriptCodingEnd   = 0;

    ensMapperDel(&transcript->ExonCoordMapper);

    return ajTrue;
}




/* @func ensTranscriptSetDisplayReference **************************************
**
** Set the display Ensembl Database Entry element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::display_xref
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] displaydbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetDisplayReference(EnsPTranscript transcript,
                                        EnsPDatabaseentry displaydbe)
{
    if(!transcript)
        return ajFalse;

    ensDatabaseentryDel(&transcript->DisplayReference);

    transcript->DisplayReference = ensDatabaseentryNewRef(displaydbe);

    return ajTrue;
}




/* @func ensTranscriptSetDescription ******************************************
**
** Set the description element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::description
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetDescription(EnsPTranscript transcript,
                                   AjPStr description)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->Description);

    if(description)
        transcript->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensTranscriptSetBioType **********************************************
**
** Set the biological type element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::biotype
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] biotype [AjPStr] Biological type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetBioType(EnsPTranscript transcript,
                               AjPStr biotype)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->BioType);

    if(biotype)
        transcript->Description = ajStrNewRef(biotype);

    return ajTrue;
}




/* @func ensTranscriptSetStatus ***********************************************
**
** Set the status element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::status
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] status [EnsETranscriptStatus] Status
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetStatus(EnsPTranscript transcript,
                              EnsETranscriptStatus status)
{
    if(!transcript)
        return ajFalse;

    transcript->Status = status;

    return ajTrue;
}




/* @func ensTranscriptSetGeneIdentifier ***************************************
**
** Set the Ensembl Gene identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::???
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] geneid [ajuint] Ensembl Gene identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetGeneIdentifier(EnsPTranscript transcript,
                                      ajuint geneid)
{
    if(!transcript)
        return ajFalse;

    transcript->GeneIdentifier = geneid;

    return ajTrue;
}




/* @func ensTranscriptSetCurrent **********************************************
**
** Set the current element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_current
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] current [AjBool] Current
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetCurrent(EnsPTranscript transcript,
                               AjBool current)
{
    if(!transcript)
        return ajFalse;

    transcript->Current = current;

    return ajTrue;
}




/* @func ensTranscriptSetStableIdentifier *************************************
**
** Set the stable identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::stable_id
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetStableIdentifier(EnsPTranscript transcript,
                                        AjPStr stableid)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->StableIdentifier);

    if(stableid)
        transcript->StableIdentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensTranscriptSetVersion **********************************************
**
** Set the version element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::version
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetVersion(EnsPTranscript transcript,
                               ajuint version)
{
    if(!transcript)
        return ajFalse;

    transcript->Version = version;

    return ajTrue;
}




/* @func ensTranscriptSetCreationDate *****************************************
**
** Set the creation date element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::created_date
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetCreationDate(EnsPTranscript transcript,
                                    AjPStr cdate)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->CreationDate);

    if(cdate)
        transcript->CreationDate = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensTranscriptSetModificationDate *************************************
**
** Set the modification date element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::modified_date
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetModificationDate(EnsPTranscript transcript,
                                        AjPStr mdate)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->ModificationDate);

    if(mdate)
        transcript->ModificationDate = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensTranscriptSetTranslation ******************************************
**
** Set the Ensembl Translation element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uE] translation [EnsPTranslation] Ensembl Translation
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetTranslation(EnsPTranscript transcript,
                                   EnsPTranslation translation)
{
    if(!transcript)
        return ajFalse;

    ensTranslationDel(&transcript->Translation);

    transcript->Translation = ensTranslationNewRef(translation);

    /* Clear internal values that depend on Translation coordinates. */

    transcript->SliceCodingStart = 0;

    transcript->SliceCodingEnd = 0;

    transcript->TranscriptCodingStart = 0;

    transcript->TranscriptCodingEnd = 0;

    return ajTrue;
}




/* @func ensTranscriptSetEnableSequenceEdits **********************************
**
** Set the enable Ensembl Sequence Edits element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::edits_enabled
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] enablese [AjBool] Enable Ensembl Sequence Edits
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetEnableSequenceEdits(EnsPTranscript transcript,
                                           AjBool enablese)
{
    if(!transcript)
        return ajFalse;

    transcript->EnableSequenceEdits = enablese;

    return ajTrue;
}




/* @func ensTranscriptAddAlternativeTranslation *******************************
**
** Add an alternative Ensembl Translation to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::add_alternative_translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptAddAlternativeTranslation(EnsPTranscript transcript,
                                              EnsPTranslation translation)
{
    if(!transcript)
        return ajFalse;

    if(!translation)
        return ajFalse;

    if(!transcript->AlternativeTranslations)
        transcript->AlternativeTranslations = ajListNew();

    ajListPushAppend(transcript->AlternativeTranslations,
                     (void *) ensTranslationNewRef(translation));

    return ajTrue;
}




/* @func ensTranscriptAddAttribute ********************************************
**
** Add an Ensembl Attribute to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::add_Attribute
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptAddAttribute(EnsPTranscript transcript,
                                 EnsPAttribute attribute)
{
    register ajuint i = 0;

    AjBool match = AJFALSE;

    if(!transcript)
        return ajFalse;

    if(!attribute)
        return ajFalse;

    if(!transcript->Attributes)
        transcript->Attributes = ajListNew();

    ajListPushAppend(transcript->Attributes,
                     (void *) ensAttributeNewRef(attribute));

    /* Check if this Attribute is associated with a Sequence Edit. */

    for(i = 0; transcriptSequenceEditCode[i]; i++)
    {
        if(ajStrMatchC(ensAttributeGetCode(attribute),
                       transcriptSequenceEditCode[i]))
            match = ajTrue;
    }

    if(match)
    {
        /*
        ** If the Attribute is a Sequence Edit, then clear internal values
        ** that depend on the edited Transcript sequence.
        */

        transcript->SliceCodingStart = 0;
        transcript->SliceCodingEnd   = 0;

        transcript->TranscriptCodingStart = 0;
        transcript->TranscriptCodingEnd   = 0;

        ensMapperDel(&transcript->ExonCoordMapper);
    }

    return ajTrue;
}




/* @func ensTranscriptAddDatabaseentry ****************************************
**
** Add an Ensembl Database Entry to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::add_DBEntry
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptAddDatabaseentry(EnsPTranscript transcript,
                                     EnsPDatabaseentry dbe)
{
    if(!transcript)
        return ajFalse;

    if(!dbe)
        return ajFalse;

    if(!transcript->DatabaseEntries)
        transcript->DatabaseEntries = ajListNew();

    ajListPushAppend(transcript->DatabaseEntries,
                     (void *) ensDatabaseentryNewRef(dbe));

    return ajTrue;
}




/* @func ensTranscriptAddExon *************************************************
**
** Add an Ensembl Exon to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::add_Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [rE] rank [ajint] Ensembl Exon rank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptAddExon(EnsPTranscript transcript,
                            EnsPExon exon,
                            ajint rank)
{
    register ajint i = 0;

    AjBool added = AJFALSE;

    AjIList iter = NULL;

    AjPStr message = NULL;

    EnsPExon lastexon = NULL;

    EnsPFeature feature     = NULL;
    EnsPFeature lastfeature = NULL;

    if(ajDebugTest("ensTranscriptAddExon"))
        ajDebug("ensTranscriptAddExon\n"
                "  transcript %p\n"
                "  exon %p\n"
                "  rank %d\n",
                transcript,
                exon,
                rank);

    if(!transcript)
        return ajFalse;

    if(!exon)
        return ajFalse;

    if(!transcript->Exons)
        transcript->Exons = ajListNew();

    if(rank > 0)
    {
        iter = ajListIterNew(transcript->Exons);

        i = 0;

        while(i < rank)
        {
            i++;

            if(ajListIterDone(iter))
            {
                /*
                ** If the AJAX List is too short, append the Exon if the rank
                ** matches this position or an empty node otherwise.
                */

                if(i == rank)
                {
                    ajListPushAppend(transcript->Exons,
                                     (void *) ensExonNewRef(exon));

                    break;
                }
                else
                    ajListPushAppend(transcript->Exons, NULL);
            }

            /* Advance one step. */

            lastexon = (EnsPExon) ajListIterGet(iter);

            if(i == rank)
            {
                /*
                ** Remove the current position, delete the previous Exon,
                ** if any, insert the new List node and advance the
                ** List Iterator one step. Remeber, inserts into AJAX Lists
                ** are applied ahead of the Iterator so that they will be seen
                ** in th enext iteration.
                */
                ajListIterRemove(iter);
                ensExonDel(&lastexon);
                ajListIterInsert(iter, (void *) ensExonNewRef(exon));
                lastexon = (EnsPExon) ajListIterGet(iter);
            }
        }

        ajListIterDel(&iter);

        return ajTrue;
    }

    feature = ensExonGetFeature(exon);

    if(ensFeatureGetStrand(feature) > 0)
    {
        ajListPeekLast(transcript->Exons, (void **) &lastexon);

        lastfeature = ensExonGetFeature(lastexon);

        if(ensFeatureGetStart(feature) > ensFeatureGetEnd(lastfeature))
        {
            /* Append at the end. */

            ajListPushAppend(transcript->Exons, (void *) ensExonNewRef(exon));

            added = ajTrue;
        }
        else
        {
            /* Insert at the correct position. */

            iter = ajListIterNew(transcript->Exons);

            while(!ajListIterDone(iter))
            {
                lastexon = (EnsPExon) ajListIterGet(iter);

                lastfeature = ensExonGetFeature(lastexon);

                if(ensFeatureGetEnd(feature) < ensFeatureGetStart(lastfeature))
                {
                    ajListIterInsert(iter, (void *) ensExonNewRef(exon));

                    added = ajTrue;

                    break;
                }
            }

            ajListIterDel(&iter);
        }
    }
    else
    {
        ajListPeekLast(transcript->Exons, (void **) &lastexon);

        lastfeature = ensExonGetFeature(lastexon);

        if(ensFeatureGetEnd(feature) < ensFeatureGetStart(lastfeature))
        {
            /* Append at the end. */

            ajListPushAppend(transcript->Exons, (void *) ensExonNewRef(exon));

            added = ajTrue;
        }
        else
        {
            /* Insert at the correct position. */

            iter = ajListIterNew(transcript->Exons);

            while(!ajListIterDone(iter))
            {
                lastexon = (EnsPExon) ajListIterGet(iter);

                lastfeature = ensExonGetFeature(lastexon);

                if(ensFeatureGetStart(feature) > ensFeatureGetEnd(lastfeature))
                {
                    ajListIterInsert(iter, (void *) ensExonNewRef(exon));

                    added = ajTrue;

                    break;
                }
            }

            ajListIterDel(&iter);
        }
    }

    /* Sanity check. */

    if(!added)
    {
        /*
        ** The Exon was not added because it has the same end coordinate as
        ** the start coordinate of another Exon.
        */

        message = ajStrNewC("ensTranscriptAddExon got an Exon, "
                            "which overlaps with another Exon in the same "
                            "Transcript.\n"
                            "Transcript Exons:\n");

        iter = ajListIterNew(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            lastexon = (EnsPExon) ajListIterGet(iter);

            lastfeature = ensExonGetFeature(lastexon);

            ajFmtPrintAppS(&message,
                           "  %S %d:%d:%d\n",
                           ensExonGetStableIdentifier(exon),
                           ensFeatureGetStart(lastfeature),
                           ensFeatureGetEnd(lastfeature),
                           ensFeatureGetStrand(lastfeature));
        }

        ajListIterDel(&iter);

        ajFmtPrintAppS(&message,
                       "This Exon:\n"
                       "  %S %d:%d:%d\n",
                       ensExonGetStableIdentifier(exon),
                       ensFeatureGetStart(feature),
                       ensFeatureGetEnd(feature),
                       ensFeatureGetStrand(feature));

        ajFatal(ajStrGetPtr(message));

        ajStrDel(&message);
    }

    ensTranscriptCalculateCoordinates(transcript);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
** @nam3rule Trace Report Ensembl Transcript elements to debug file
**
** @argrule Trace transcript [const EnsPTranscript] Ensembl Transcript
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensTranscriptTrace ***************************************************
**
** Trace an Ensembl Transcript.
**
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptTrace(const EnsPTranscript transcript, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTranscriptTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  DisplayReference %p\n"
            "%S  Description '%S'\n"
            "%S  BioType '%S'\n"
            "%S  Status %d\n"
            "%S  Current '%B'\n"
            "%S  StableIdentifier '%S'\n"
            "%S  CreationDate '%S'\n"
            "%S  ModificationDate '%S'\n"
            "%S  Version %u\n"
            "%S  GeneIdentifier %u\n"
            "%S  AlternativeTranslations %p\n"
            "%S  Attributes %p\n"
            "%S  DatabaseEntries %p\n"
            "%S  Exons %p\n"
            "%S  Supportingfeatures %p\n"
            "%S  Translation %p\n"
            "%S  SliceCodingStart %u\n"
            "%S  SliceCodingEnd %u\n"
            "%S  TranscriptCodingStart %u\n"
            "%S  TranscriptCodingEnd %u\n"
            "%S  EnableSequenceEdits '%B'\n"
            "%S  ExonCoordMapper %p\n"
            "%S  StartPhase %d\n",
            indent, transcript,
            indent, transcript->Use,
            indent, transcript->Identifier,
            indent, transcript->Adaptor,
            indent, transcript->Feature,
            indent, transcript->DisplayReference,
            indent, transcript->Description,
            indent, transcript->BioType,
            indent, transcript->Status,
            indent, transcript->Current,
            indent, transcript->StableIdentifier,
            indent, transcript->CreationDate,
            indent, transcript->ModificationDate,
            indent, transcript->Version,
            indent, transcript->GeneIdentifier,
            indent, transcript->AlternativeTranslations,
            indent, transcript->Attributes,
            indent, transcript->DatabaseEntries,
            indent, transcript->Exons,
            indent, transcript->Supportingfeatures,
            indent, transcript->Translation,
            indent, transcript->SliceCodingStart,
            indent, transcript->SliceCodingEnd,
            indent, transcript->TranscriptCodingStart,
            indent, transcript->TranscriptCodingEnd,
            indent, transcript->EnableSequenceEdits,
            indent, transcript->ExonCoordMapper,
            indent, transcript->StartPhase);

    ensFeatureTrace(transcript->Feature, level + 1);

    ensDatabaseentryTrace(transcript->DisplayReference, level + 1);

    /* Trace the AJAX List of alternative Ensembl Translations. */

    if(transcript->AlternativeTranslations)
    {
        ajDebug("%S    AJAX List %p of alternative Ensembl Translations\n",
                indent, transcript->AlternativeTranslations);

        iter = ajListIterNewread(transcript->AlternativeTranslations);

        while(!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            ensTranslationTrace(translation, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Attributes. */

    if(transcript->Attributes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Attributes\n",
                indent, transcript->Attributes);

        iter = ajListIterNewread(transcript->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entries. */

    if(transcript->DatabaseEntries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entries\n",
                indent, transcript->DatabaseEntries);

        iter = ajListIterNewread(transcript->DatabaseEntries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Exons. */

    if(transcript->Exons)
    {
        ajDebug("%S    AJAX List %p of Ensembl Exons\n",
                indent, transcript->Exons);

        iter = ajListIterNewread(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ensExonTrace(exon, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of supporting Ensembl Base Align Features. */

    if(transcript->Supportingfeatures)
    {
        ajDebug("%S    AJAX List %p of Ensembl Base Align Features\n",
                indent, transcript->Supportingfeatures);

        iter = ajListIterNewread(transcript->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ensBasealignfeatureTrace(baf, level + 2);
        }

        ajListIterDel(&iter);
    }

    ensTranslationTrace(transcript->Translation, level + 1);

    ensMapperTrace(transcript->ExonCoordMapper, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensTranscriptStatusFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Transcript status element.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsETranscriptStatus] Ensembl Transcript status or
**                                ensETranscriptStatusNULL
** @@
******************************************************************************/

EnsETranscriptStatus ensTranscriptStatusFromStr(const AjPStr status)
{
    register EnsETranscriptStatus i = ensETranscriptStatusNULL;

    EnsETranscriptStatus estatus = ensETranscriptStatusNULL;

    for(i = ensETranscriptStatusKnown; transcriptStatus[i]; i++)
        if(ajStrMatchC(status, transcriptStatus[i]))
            estatus = i;

    if(!estatus)
        ajDebug("ensTranscriptStatusFromStr encountered "
                "unexpected string '%S'.\n", status);

    return estatus;
}




/* @func ensTranscriptStatusToChar ********************************************
**
** Convert an Ensembl Transcript status element into a C-type (char*) string.
**
** @param [r] status [EnsETranscriptStatus] Transcript status
**
** @return [const char*] Transcript status C-type (char*) string
** @@
******************************************************************************/

const char* ensTranscriptStatusToChar(EnsETranscriptStatus status)
{
    register EnsETranscriptStatus i = ensETranscriptStatusNULL;

    if(!status)
        return NULL;

    for(i = ensETranscriptStatusKnown;
        transcriptStatus[i] && (i < status);
        i++);

    if(!transcriptStatus[i])
        ajDebug("ensTranscriptStatusToChar encountered an "
                "out of boundary error on status %d.\n", status);

    return transcriptStatus[i];
}




/* @func ensTranscriptCalculateCoordinates ************************************
**
** Calculate coordinates of an Ensembl Transcript.
** This function should be called, whenever an Exon of this Transcript has
** been changed.
**
** @cc Bio::EnsEMBL::Transcript::recalculate_coordinates
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptCalculateCoordinates(EnsPTranscript transcript)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjBool transsplicing = AJFALSE;

    AjIList iter       = NULL;
    const AjPList list = NULL;

    EnsPExon exon = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature tfeature = NULL;

    EnsPSlice slice = NULL;

    if(!transcript)
        return ajFalse;

    list = ensTranscriptGetExons(transcript);

    if(!ajListGetLength(list))
        return ajTrue;

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        /* Skip missing Exons. */

        if(!exon)
            continue;

        efeature = ensExonGetFeature(exon);

        /* Skip un-mapped Exons. */

        if(!ensFeatureGetStart(efeature))
            continue;

        slice = ensFeatureGetSlice(efeature);

        start = ensFeatureGetStart(efeature);

        end = ensFeatureGetEnd(efeature);

        strand = ensFeatureGetStrand(efeature);

        break;
    }

    ajListIterDel(&iter);

    /* Start loop after the first Exon with coordinates. */

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        /* Skip missing Exons. */

        if(!exon)
            continue;

        efeature = ensExonGetFeature(exon);

        /* Skip un-mapped Exons. */

        if(!ensFeatureGetStart(efeature))
            continue;

        if(!ensSliceMatch(ensFeatureGetSlice(efeature), slice))
            ajFatal("ensTranscriptCalculateCoordinates got Exons of one "
                    "Transcript on different Slices.\n");

        start = (ensFeatureGetStart(efeature) < start) ?
            ensFeatureGetStart(efeature) :
            start;

        end = (ensFeatureGetEnd(efeature) > end) ?
            ensFeatureGetEnd(efeature) :
            end;

        if(ensFeatureGetStrand(efeature) != strand)
            transsplicing = ajTrue;
    }

    ajListIterDel(&iter);

    if(transsplicing)
        ajWarn("ensTranscriptCalculateCoordinates got Transcript with "
               "trans-splicing event.\n");

    tfeature = ensTranscriptGetFeature(transcript);

    ensFeatureSetStart(tfeature, start);

    ensFeatureSetEnd(tfeature, end);

    ensFeatureSetStrand(tfeature, strand);

    ensFeatureSetSlice(tfeature, slice);

    /* Clear internal values that depend on Exon coordinates. */

    ensMapperClear(transcript->ExonCoordMapper);

    transcript->SliceCodingStart = 0;

    transcript->SliceCodingEnd = 0;

    transcript->TranscriptCodingStart = 0;

    transcript->TranscriptCodingEnd = 0;

    return ajTrue;
}




/* @func ensTranscriptTransform ***********************************************
**
** Transform an Ensembl Transcript into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Transcript::transform
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptTransform(EnsPTranscript transcript,
                                      const AjPStr csname,
                                      const AjPStr csversion)
{
    ajint lastnewstart  = 0;
    ajint lastnewstrand = 0;
    ajint lastoldstrand = 0;
    ajint trstart = 0;
    ajint trend   = 0;

    ajuint pslength = 0;

    AjBool first       = AJFALSE;
    AjBool ignoreorder = AJFALSE;
    AjBool orderbroken = AJFALSE;
    AjBool error       = AJFALSE;

    AjIList iter   = NULL;
    AjPList exons  = NULL;
    AjPList pslist = NULL;

    EnsPFeature newfeature = NULL;
    EnsPFeature oldfeature = NULL;
    EnsPFeature feature    = NULL;

    EnsPExon oldexon   = NULL;
    EnsPExon newexon   = NULL;
    EnsPExon startexon = NULL;
    EnsPExon endexon   = NULL;

    EnsPProjectionsegment ps = NULL;

    const EnsPSeqregion newsr     = NULL;
    const EnsPSeqregion lastnewsr = NULL;

    EnsPTranscript newtranscript = NULL;

    EnsPTranslation newtranslation = NULL;

    if(!transcript)
        return NULL;

    if(!csname)
        return NULL;

    feature = ensFeatureTransform(transcript->Feature, csname, csversion);

    if(!feature)
    {
        /*
        ** If the Transcript does not transform, test whether it
        ** projects, since its Exons may transform well.
        */

        pslist = ajListNew();

        ensFeatureProject(transcript->Feature, csname, csversion, pslist);

        pslength = ajListGetLength(pslist);

        while(ajListPop(pslist, (void **) &ps))
            ensProjectionsegmentDel(&ps);

        ajListFree(&pslist);

        /*
        ** If the List of Ensembl Projection Segments was empty, the
        ** Transcript did not project. If it projects, make sure
        ** all its Exons are defined.
        */

        if(!pslength)
            return NULL;
        else
            ensTranscriptGetExons(transcript);
    }

    exons = ajListNew();

    if(transcript->Exons)
    {
        first = ajTrue;

        iter = ajListIterNew(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            /*
            ** We want to check, whether the transform preserved the
            ** 5 prime to 3 prime order. No complaints on trans-splicing.
            */

            oldexon = (EnsPExon) ajListIterGet(iter);

            newexon = ensExonTransform(oldexon, csname, csversion);

            if(!newexon)
            {
                error = ajTrue;

                break;
            }

            oldfeature = ensExonGetFeature(oldexon);

            newfeature = ensExonGetFeature(newexon);

            if(!feature)
            {
                if(first)
                {
                    trstart = ensFeatureGetStart(newfeature);

                    trend = ensFeatureGetEnd(newfeature);

                    first = ajFalse;
                }
                else
                {
                    newsr = ensFeatureGetSeqregion(newfeature);

                    if(!ensSeqregionMatch(newsr, lastnewsr))
                    {
                        error = ajTrue;

                        break;
                    }

                    /*
                    ** Test for a trans-splicing event and ignore the Exon
                    ** order in those cases.
                    */

                    if(ensFeatureGetStrand(oldfeature) != lastoldstrand)
                        ignoreorder = ajTrue;

                    /* Check for correct Exon order. */

                    if((lastnewstrand >= 0) &&
                       (ensFeatureGetStart(newfeature) < lastnewstart))
                        orderbroken = ajTrue;

                    if((lastnewstrand < 0) &&
                       (ensFeatureGetStart(newfeature) > lastnewstart))
                        orderbroken = ajTrue;

                    trstart = (ensFeatureGetStart(newfeature) < trstart) ?
                        ensFeatureGetStart(newfeature) :
                        trstart;

                    trend = (ensFeatureGetEnd(newfeature) > trend) ?
                        ensFeatureGetEnd(newfeature) :
                        trend;
                }

                lastnewsr = ensFeatureGetSeqregion(newfeature);

                lastnewstart = ensFeatureGetStart(newfeature);

                lastnewstrand = ensFeatureGetStrand(newfeature);

                lastoldstrand = ensFeatureGetStrand(oldfeature);
            }

            if(transcript->Translation)
            {
                /* FIXME: Should this move into ensTranscriptSetFeature? */
                if(ensTranslationGetStartExon(transcript->Translation) ==
                   oldexon)
                    startexon = newexon;

                if(ensTranslationGetEndExon(transcript->Translation) ==
                   oldexon)
                    endexon = newexon;
            }

            ajListPushAppend(exons, (void *) newexon);
        }

        ajListIterDel(&iter);

        if(orderbroken && (!ignoreorder))
        {
            ajWarn("ensTranscriptTransform got Exons out of order in "
                   "transformation of Transcript with identifier %u.\n",
                   transcript->Identifier);

            error = ajTrue;
        }

        if(error)
        {
            /* Clean up in case we encountered an error. */

            ensFeatureDel(&feature);

            while(ajListPop(exons, (void **) &newexon))
                ensExonDel(&newexon);

            return NULL;
        }
    }

    if(!feature)
    {
        ajListPeekFirst(exons, (void **) &newexon);

        newfeature = ensExonGetFeature(newexon);

        feature = ensFeatureNewS(ensFeatureGetAnalysis(transcript->Feature),
                                 ensFeatureGetSlice(newfeature),
                                 trstart,
                                 trend,
                                 ensFeatureGetStrand(newfeature));
    }

    newtranscript = ensTranscriptNewObj(transcript);

    ensTranscriptSetFeature(newtranscript, feature);

    ensFeatureDel(&feature);

    if(transcript->Translation)
    {
        /* FIXME: Should this move into ensTranscriptSetFeature? */
        newtranslation = ensTranslationNewObj(transcript->Translation);

        ensTranslationSetStartExon(newtranslation, startexon);

        ensTranslationSetEndExon(transcript->Translation, endexon);

        ensTranscriptSetTranslation(newtranscript, newtranslation);

        ensTranslationDel(&newtranslation);
    }

    while(ajListPop(exons, (void **) &newexon))
        ensExonDel(&newexon);

    ajListFree(&exons);

    return newtranscript;
}




/* @func ensTranscriptTransfer ************************************************
**
** Transfer an Ensembl Transcript onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Transcript::transfer
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptTransfer(EnsPTranscript transcript,
                                     EnsPSlice slice)
{
    EnsPFeature newfeature = NULL;

    EnsPTranscript newtranscript = NULL;

    if(ajDebugTest("ensTranscriptTransfer"))
        ajDebug("ensTranscriptTransfer\n"
                "  transcript %p\n"
                "  slice %p\n",
                transcript,
                slice);

    if(!transcript)
        return NULL;

    if(!slice)
        return NULL;

    if(!transcript->Feature)
        ajFatal("ensTranscriptTransfer cannot transfer an Ensembl Transcript "
                "without an Ensembl Feature.\n");

    newfeature = ensFeatureTransfer(transcript->Feature, slice);

    if(!newfeature)
        return NULL;

    newtranscript = ensTranscriptNewObj(transcript);

    ensTranscriptSetFeature(newtranscript, newfeature);

    ensFeatureDel(&newfeature);

    return newtranscript;
}




/* @func ensTranscriptFetchAllAttributes **************************************
**
** Fetch all Ensembl Attributes of an Ensembl Transcript and optionally filter
** via an Ensembl Attribute code. To get all Ensembl Attributes for this
** Ensembl Transcript, consider using ensTranscriptGetAttributes.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Attributes
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [r] attributes [AjPList] AJAX List of Ensembl Attributes
** @see ensTranscriptGetAttributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllAttributes(EnsPTranscript transcript,
                                       const AjPStr code,
                                       AjPList attributes)
{
    AjBool match = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPAttribute attribute = NULL;

    if(!transcript)
        return ajFalse;

    if(!attributes)
        return ajFalse;

    list = ensTranscriptGetAttributes(transcript);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if(code)
        {
            if(ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if(match)
            ajListPushAppend(attributes,
                             (void *) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranscriptFetchAllConstitutiveExons *******************************
**
** Fetch all constitutive Ensembl Exons of an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Exons before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_constitutive_Exons
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllConstitutiveExons(EnsPTranscript transcript,
                                              AjPList exons)
{
    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPExon exon = NULL;

    if(!transcript)
        return ajFalse;

    if(!exons)
        return ajFalse;

    list = ensTranscriptGetExons(transcript);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if(ensExonGetConstitutive(exon))
            ajListPushAppend(exons, (void *) ensExonNewRef(exon));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranscriptFetchAllDatabaseEntries *********************************
**
** Fetch all Ensembl Database Entries of an Ensembl Transcript and optionally
** filter via an Ensembl External Database name or type. To get all Ensembl
** External Database objects for this Ensembl Transcript, consider using
** ensTranscriptGetExternalDatabaseEntries.
**
** The caller is responsible for deleting the Ensembl Database Entries before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [r] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
** @see ensTranscriptGetDatabaseEntries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllDatabaseEntries(EnsPTranscript transcript,
                                            const AjPStr name,
                                            EnsEExternaldatabaseType type,
                                            AjPList dbes)
{
    AjBool namematch = AJFALSE;
    AjBool typematch = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPDatabaseentry dbe = NULL;

    if(!transcript)
        return ajFalse;

    if(!dbes)
        return ajFalse;

    list = ensTranscriptGetDatabaseEntries(transcript);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        dbe = (EnsPDatabaseentry) ajListIterGet(iter);

        if(name)
        {
            if(ajStrMatchCaseS(name, ensDatabaseentryGetDbName(dbe)))
                namematch = ajTrue;
            else
                namematch = ajFalse;
        }
        else
            namematch = ajTrue;

        if(type)
        {
            if(type == ensDatabaseentryGetType(dbe))
                typematch = ajTrue;
            else
                typematch = ajFalse;
        }

        else
            typematch = ajTrue;

        if(namematch && typematch)
            ajListPushAppend(dbes, (void *) ensDatabaseentryNewRef(dbe));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranscriptFetchAllIntrons *****************************************
**
** Fetch all Ensembl Introns of an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Introns before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Introns
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] introns [AjPList] AJAX List of Ensembl Introns
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllIntrons(EnsPTranscript transcript, AjPList introns)
{
    register ajuint i = 0;

    const AjPList list = NULL;

    EnsPExon exon1 = NULL;
    EnsPExon exon2 = NULL;

    EnsPIntron intron = NULL;

    if(!transcript)
        return ajFalse;

    if(!introns)
        return ajFalse;

    list = ensTranscriptGetExons(transcript);

    for(i = 0; i < (ajListGetLength(list) - 1); i++)
    {
        ajListPeekNumber(list, i, (void **) &exon1);

        ajListPeekNumber(list, i + 1, (void **) &exon2);

        intron = ensIntronNewExons(exon1, exon2);

        ajListPushAppend(introns, (void *) intron);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchAllSequenceEdits ***********************************
**
** Fetch all Ensembl Sequence Edits of an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Sequence Edits before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_SeqEdits
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] ses [AjPList] AJAX List of Ensembl Sequence Edits
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Ensembl Sequence Edits are Ensembl Attributes, which codes have to be
** defined in the static const char *transcriptSequenceEditCode array.
******************************************************************************/

AjBool ensTranscriptFetchAllSequenceEdits(EnsPTranscript transcript,
                                          AjPList ses)
{
    register ajuint i = 0;

    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute at = NULL;

    EnsPSequenceEdit se = NULL;

    if(!transcript)
        return ajFalse;

    if(!ses)
        return ajFalse;

    code = ajStrNew();

    attributes = ajListNew();

    for(i = 0; transcriptSequenceEditCode[i]; i++)
    {
        ajStrAssignC(&code, transcriptSequenceEditCode[i]);

        ensTranscriptFetchAllAttributes(transcript, code, attributes);
    }

    while(ajListPop(attributes, (void **) &at))
    {
        se = ensSequenceEditNewA(at);

        ajListPushAppend(ses, (void *) se);

        ensAttributeDel(&at);
    }

    ajListFree(&attributes);

    ajStrDel(&code);

    return ajTrue;
}




/* @func ensTranscriptFetchDisplayIdentifier **********************************
**
** Fetch the display identifier of an Ensembl Transcript.
** This will return the stable identifier, the SQL database-internal identifier
** or the Transcript memory address in this descending priority.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::display_id
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [wP] Pidentifier [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchDisplayIdentifier(const EnsPTranscript transcript,
                                           AjPStr *Pidentifier)
{
    if(!transcript)
        return ajFalse;

    if(!Pidentifier)
        return ajFalse;

    if(transcript->StableIdentifier &&
       ajStrGetLen(transcript->StableIdentifier))
        *Pidentifier = ajStrNewS(transcript->StableIdentifier);
    else if(transcript->Identifier)
        *Pidentifier = ajFmtStr("%u", transcript->Identifier);
    else
        *Pidentifier = ajFmtStr("%p", transcript);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceSeq ****************************************
**
** Fetch the sequence of an Ensembl Transcript as AJAX Sequence.
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Transcript:spliced_seq
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceSeq(EnsPTranscript transcript,
                                     AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    ensTranscriptFetchSequenceStr(transcript, &sequence);

    ensTranscriptFetchDisplayIdentifier(transcript, &name);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceStr ****************************************
**
** Fetch the spliced sequence of an Ensembl Transcript as AJAX String.
**
** The sequence of all Ensembl Exons is concatenated and by default, all
** post-transcriptional Sequence Edits are applied. Applying Sequence Edits can
** be disabled by setting ensTranscriptSetEnableSequenceEdits to ajFalse.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::spliced_seq
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
** @see ensTranscriptSetEnableSequenceEdits
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceStr(EnsPTranscript transcript,
                                     AjPStr *Psequence)
{
    AjIList iter        = NULL;
    const AjPList exons = NULL;
    AjPList ses         = NULL;

    AjPStr exseq = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceEdit se = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    exons = ensTranscriptGetExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        ensExonFetchSequenceStr(exon, &exseq);

        if(exseq && ajStrGetLen(exseq))
            ajStrAppendS(Psequence, exseq);
        else
        {
            ajDebug("ensTranscriptFetchSequenceStr could not get sequence "
                    "for Exon. Transcript sequence may not be correct.\n");

            feature = ensExonGetFeature(exon);

            ajStrAppendCountK(Psequence, 'N', ensFeatureGetLength(feature));
        }

        ajStrDel(&exseq);
    }

    ajListIterDel(&iter);

    /* Apply post-transcriptional Sequence Edits if enabled. */

    if(transcript->EnableSequenceEdits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceEdits(transcript, ses);

        /*
        ** Sort Sequence Edits in reverse order to avoid the complication of
        ** adjusting down-stream Sequence Edit coordinates.
        */

        ensSequenceEditSortByStartDescending(ses);

        while(ajListPop(ses, (void **) &se))
        {
            ensSequenceEditApplyEdit(se, 0, Psequence);

            ensSequenceEditDel(&se);
        }

        ajListFree(&ses);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchTranslatableSequence *******************************
**
** Fetch the translatable sequence portion of an Ensembl Transcript as
** AJAX String. The sequence is padded with 'N's according to the start phase
** of the first coding Exon.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::translateable_seq
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchTranslatableSequence(EnsPTranscript transcript,
                                              AjPStr *Psequence)
{
    ajint sphase = 0;

    ajuint cdsstart = 0;
    ajuint cdsend = 0;

    AjPStr sequence = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /*
    ** Return empty string for non-coding Transcripts.
    ** The ensTranscriptGetTranslation function will attempt to load the
    ** Translation from the database.
    */

    translation = ensTranscriptGetTranslation(transcript);

    if(!translation)
    {
        ajDebug("ensTranscriptFetchTranslatableSequence got a Transcript "
                "without a Translation.\n");

        return ajTrue;
    }

    /* Calculate coding start and end coordinates. */

    cdsstart = ensTranscriptGetTranscriptCodingStart(transcript);

    if(!cdsstart)
        return ajTrue;

    cdsend = ensTranscriptGetTranscriptCodingEnd(transcript);

    if(!cdsend)
        return ajTrue;

    sphase = ensExonGetStartPhase(ensTranslationGetStartExon(translation));

    if(sphase > 0)
        ajStrAppendCountK(Psequence, 'N', sphase);

    ensTranscriptFetchSequenceStr(transcript, &sequence);

    ajStrAppendSubS(Psequence, sequence, cdsstart - 1, cdsend - 1);

    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranscriptFetchTranslationSequenceStr *****************************
**
** Fetch the sequence of the Ensembl Translation of an
** Ensembl Transcript as AJAX String.
**
** The sequence is based on ensTranscriptFetchTranslatableSequence and by
** default, all post-translational Sequence Edits are applied. Applying
** Sequence Edits can be disabled by setting
** ensTranscriptSetEnableSequenceEdits to ajFalse.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::translate
** @cc Bio::EnsEMBL::Translation::modify_translation
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
** @see ensTranscriptFetchTranslatableSequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchTranslationSequenceStr(EnsPTranscript transcript,
                                                AjPStr *Psequence)
{
    AjPList ses = NULL;

    AjPStr cdna = NULL;

    const AjPTrn atranslation = NULL;

    EnsPSequenceEdit se = NULL;

    EnsPSlice slice = NULL;

    EnsPTranslation etranslation = NULL;

    if(ajDebugTest("ensTranscriptFetchTranslationSequenceStr"))
        ajDebug("ensTranscriptFetchTranslationSequenceStr\n"
                "  transcript %p\n"
                "  Psequence %p\n",
                transcript,
                Psequence);

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /*
    ** Return empty string for non-coding Transcripts.
    ** The ensTranscriptGetTranslation function will attempt to load the
    ** Translation from the database.
    */

    etranslation = ensTranscriptGetTranslation(transcript);

    if(!etranslation)
    {
        ajDebug("ensTranscriptFetchTranslationSequenceStr got a Transcript "
                "without a Translation.\n");

        return ajTrue;
    }

    cdna = ajStrNew();

    ensTranscriptFetchTranslatableSequence(transcript, &cdna);

    if(ajStrGetLen(cdna) < 1)
        return ajTrue;

    slice = ensFeatureGetSlice(transcript->Feature);

    atranslation = ensSliceGetTranslation(slice);

    ajTrnSeqS(atranslation, cdna, Psequence);

    ajStrDel(&cdna);

    /*
    ** Remove the final stop codon from the mRNA if it is present, so that the
    ** resulting peptides do not end with a '*'. If a terminal stop codon is
    ** desired, call ensTranscriptFetchTranslatableSequence and translate it
    ** directly.
    ** NOTE: This test is simpler and hopefully more efficient than the one
    ** in the Perl API, which tests for a termination codon in a
    ** codon table-specifc manner and removes the last triplet from the cDNA.
    ** NOTE: This implementation does not use the 'complete5' and 'complete3'
    ** Sequence Region Attributes to modify the translated sequence. The
    ** initiator codon should be correctly translated by ajTrnSeqS based on
    ** the codon table and the stop codon, if present, is removed above.
    */

    if(ajStrGetCharLast(*Psequence) == '*')
        ajStrCutEnd(Psequence, 1);

    /* Apply post-translational Sequence Edits if enabled. */

    if(transcript->EnableSequenceEdits)
    {
        ses = ajListNew();

        ensTranslationFetchAllSequenceEdits(etranslation, ses);

        /*
        ** Sort Sequence Edits in reverse order to avoid the complication of
        ** adjusting down-stream Sequence Edit coordinates.
        */

        ensSequenceEditSortByStartDescending(ses);

        while(ajListPop(ses, (void **) &se))
        {
            ensSequenceEditApplyEdit(se, 0, Psequence);

            ensSequenceEditDel(&se);
        }

        ajListFree(&ses);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchTranslationSequenceSeq *****************************
**
** Fetch the sequence of the Ensembl Translation of an
** Ensembl Transcript as AJAX Sequence.
**
** The sequence is based on ensTranscriptFetchTranslatableSequence and by
** default, all post-translational Sequence Edits are applied. Applying
** Sequence Edits can be disabled by setting
** ensTranscriptSetEnableSequenceEdits to ajFalse.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Transcript::translate
** @cc Bio::EnsEMBL::Translation::modify_translation
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
** @see ensTranscriptFetchTranslatableSequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchTranslationSequenceSeq(EnsPTranscript transcript,
                                                AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    translation = ensTranscriptGetTranslation(transcript);

    if(!translation)
        return ajTrue;

    name     = ajStrNew();
    sequence = ajStrNew();

    ensTranslationFetchDisplayIdentifier(translation, &name);

    ensTranscriptFetchTranslationSequenceStr(transcript, &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetProt(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @funcstatic transcriptCompareStartAscending ********************************
**
** Comparison function to sort Ensembl Transcripts by their Ensembl Feature
** start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Transcript address 1
** @param [r] P2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int transcriptCompareStartAscending(const void* P1, const void* P2)
{
    const EnsPTranscript transcript1 = NULL;
    const EnsPTranscript transcript2 = NULL;

    transcript1 = *(EnsPTranscript const *) P1;
    transcript2 = *(EnsPTranscript const *) P2;

    if(ajDebugTest("transcriptCompareStartAscending"))
        ajDebug("transcriptCompareStartAscending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);

    /* Sort empty values towards the end of the AJAX List. */

    if(transcript1 && (!transcript2))
        return -1;

    if((!transcript1) && (!transcript2))
        return 0;

    if((!transcript1) && transcript2)
        return +1;

    return ensFeatureCompareStartAscending(transcript1->Feature,
                                           transcript2->Feature);
}




/* @func ensTranscriptSortByStartAscending ************************************
**
** Sort Ensembl Transcripts by their Ensembl Feature start coordinate
** in ascending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSortByStartAscending(AjPList transcripts)
{
    if(!transcripts)
        return ajFalse;

    ajListSort(transcripts, transcriptCompareStartAscending);

    return ajTrue;
}




/* @funcstatic transcriptCompareStartDescending *******************************
**
** Comparison function to sort Ensembl Transcripts by their Ensembl Feature
** start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Transcript address 1
** @param [r] P2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int transcriptCompareStartDescending(const void* P1, const void* P2)
{
    const EnsPTranscript transcript1 = NULL;
    const EnsPTranscript transcript2 = NULL;

    transcript1 = *(EnsPTranscript const *) P1;
    transcript2 = *(EnsPTranscript const *) P2;

    if(ajDebugTest("transcriptCompareStartDescending"))
        ajDebug("transcriptCompareStartDescending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);

    /* Sort empty values towards the end of the AJAX List. */

    if(transcript1 && (!transcript2))
        return -1;

    if((!transcript1) && (!transcript2))
        return 0;

    if((!transcript1) && transcript2)
        return +1;

    return ensFeatureCompareStartDescending(transcript1->Feature,
                                            transcript2->Feature);
}




/* @func ensTranscriptSortByStartDescending ***********************************
**
** Sort Ensembl Transcripts by their Ensembl Feature start coordinate
** in descending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSortByStartDescending(AjPList transcripts)
{
    if(!transcripts)
        return ajFalse;

    ajListSort(transcripts, transcriptCompareStartDescending);

    return ajTrue;
}




/* @section mapper ************************************************************
**
** Ensembl Transcript Mapper functions.
**
** @fdata [EnsPTranscript]
** @fnote None
**
** @cc Bio::EnsEMBL::TranscriptMapper CVS Revision: 1.6
**
** @nam3rule Mapper Ensembl Transcript Mapper functions
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory new
******************************************************************************/




/* @func ensTranscriptMapperInit **********************************************
**
** Initialise the Ensembl Transcript Mapper of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::TranscriptMapper::new
** @cc Bio::EnsEMBL::TranscriptMapper::_load_mapper
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Initialises a Transcript Mapper object which can be used to perform
** various coordinate transformations relating to Transcripts.
** Since the Transcript Mapper uses the Transcript state at the time of
** initialisation to perform the conversions, it must be re-initialised if the
** underlying Transcript is altered.
** 'Genomic' coordinates are in fact relative to the Slice that the Transcript
** is annotated on.
******************************************************************************/

AjBool ensTranscriptMapperInit(EnsPTranscript transcript)
{
    ajuint srid = 0;

    /* Current and previous Feature (Exon) Slice coordinates */

    ajuint curftrstart = 0;
    ajuint curftrend   = 0;
    ajint curftrstrand = 0;

    ajuint prvftrstart = 0;
    ajuint prvftrend   = 0;

    /* Current and previous Transcript (cDNA) coordinates */

    ajuint curtrcstart = 0;
    ajuint curtrcend   = 0;

    ajuint prvtrcstart  = 0;
    ajuint prvtrcend    = 0;
    ajuint prvtrclength = 0;

    ajuint editshift = 0;

    AjIList iter        = NULL;
    const AjPList exons = NULL;
    AjPList ses         = NULL;

    AjPStr src = NULL;
    AjPStr trg = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceEdit se = NULL;

    EnsPSlice slice = NULL;

    if(ajDebugTest("ensTranscriptMapperInit"))
        ajDebug("ensTranscriptMapperInit\n"
                "  transcript %p\n",
                transcript);

    if(!transcript)
        return ajFalse;

    if(transcript->ExonCoordMapper)
        ensMapperClear(transcript->ExonCoordMapper);
    else
    {
        src = ajStrNewC("transcript");
        trg = ajStrNewC("slice");

        cs = ensSliceGetCoordsystem(ensFeatureGetSlice(transcript->Feature));

        transcript->ExonCoordMapper = ensMapperNew(src, trg, cs, cs);

        ajStrDel(&src);
        ajStrDel(&trg);
    }

    /* Get the Sequence Region Identifier for this Transcript. */

    feature = ensTranscriptGetFeature(transcript);

    slice = ensFeatureGetSlice(feature);

    srid = ensSliceGetSeqregionIdentifier(slice);

    /* Get all Exons of this Transcript. */

    exons = ensTranscriptGetExons(transcript);

    if(ajListGetLength(exons))
    {
        ajListPeekFirst(exons, (void **) &exon);

        transcript->StartPhase = ensExonGetStartPhase(exon);
    }
    else
        transcript->StartPhase = -1;

    /* Load Mapper Bio::EnsEMBL::TranscriptMapper::_load_mapper */

    ses = ajListNew();

    if(transcript->EnableSequenceEdits)
    {
        ensTranscriptFetchAllSequenceEdits(transcript, ses);

        ensSequenceEditSortByStartAscending(ses);
    }

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        feature = ensExonGetFeature(exon);

        curftrstart = ensFeatureGetStart(feature);

        curftrend = ensFeatureGetEnd(feature);

        curftrstrand = ensFeatureGetStrand(feature);

        curtrcstart = curtrcend + 1;

        curtrcend = curtrcstart + ensFeatureGetLength(feature) - 1;

        /*
        ** Add deletions and insertions into Mapper Pairs when Sequence Edits
        ** are turned on and ignore mismatches, i.e. treat them as matches.
        */

        if(transcript->EnableSequenceEdits)
        {
            while(ajListPeekFirst(ses, (void **) &se) &&
                  (ensSequenceEditGetStart(se) + editshift <= curtrcend))
            {
                if(ensSequenceEditGetLengthDifference(se))
                {
                    /*
                    ** Break the Mapper Pair into two parts, finish the
                    ** first Mapper Pair just before the Sequence Edit.
                    */

                    prvtrcend = ensSequenceEditGetStart(se) + editshift - 1;

                    prvtrcstart = curtrcstart;

                    prvtrclength = prvtrcend - prvtrcstart + 1;

                    if(curftrstrand >= 0)
                    {
                        prvftrstart = curftrstart;

                        prvftrend = curftrstart + prvtrclength - 1;
                    }
                    else
                    {
                        prvftrstart = curftrend - prvtrclength + 1;

                        prvftrend = curftrend;
                    }

                    /*
                    ** Only create a Mapper Pair if this is not a boundary
                    ** case. Set the Ensembl Transcript identifier as source
                    ** object identifier and the Sequence Region identifier
                    ** of the Slice on which this Transcript is annotated
                    ** as the target object identifier.
                    */

                    if(prvtrclength > 0)
                        ensMapperAddCoordinates(transcript->ExonCoordMapper,
                                                transcript->Identifier,
                                                prvtrcstart,
                                                prvtrcend,
                                                curftrstrand,
                                                srid,
                                                prvftrstart,
                                                prvftrend);

                    curtrcstart = prvtrcend + 1;

                    if(curftrstrand >= 0)
                        curftrstart = prvftrend + 1;
                    else
                        curftrend = prvftrstart - 1;

                    curtrcend += ensSequenceEditGetLengthDifference(se);

                    if(ensSequenceEditGetLengthDifference(se) >= 0)
                    {
                        /*
                        ** Positive length difference means insertion into
                        ** Transcript.
                        ** Shift Transcript coordinates along.
                        */

                        curtrcstart += ensSequenceEditGetLengthDifference(se);
                    }
                    else
                    {
                        /*
                        ** Negative length difference means deletion from
                        ** Transcript and insertion into Slice.
                        ** Shift Slice coordinates along.
                        */

                        if(curftrstrand >= 0)
                            curftrstart -=
                                ensSequenceEditGetLengthDifference(se);
                        else
                            curftrend +=
                                ensSequenceEditGetLengthDifference(se);
                    }

                    editshift += ensSequenceEditGetLengthDifference(se);
                }

                /* At this stage remove the Sequence Edit from the List. */

                ajListPop(ses, (void **) &se);

                ensSequenceEditDel(&se);
            }
        }

        /*
        ** Set the Transcript identifier as the source object identifier and
        ** the Sequence Region identifier of the Slice on which this Transcript
        ** is annotated as the target object identifier.
        */

        if((curtrcend - curtrcstart + 1) > 0)
            ensMapperAddCoordinates(transcript->ExonCoordMapper,
                                    transcript->Identifier,
                                    curtrcstart,
                                    curtrcend,
                                    curftrstrand,
                                    srid,
                                    curftrstart,
                                    curftrend);
    }

    ajListIterDel(&iter);

    /* Delete any remaining Sequence Edits before deleting the List. */

    while(ajListPop(ses, (void **) &se))
        ensSequenceEditDel(&se);

    ajListFree(&ses);

    return ajTrue;
}




/* @func ensTranscriptMapperTranscriptToSlice *********************************
**
** Map Transcript (cDNA) coordinates of an Ensembl Transcript into Slice
** (genome) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::cdna2genomic
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start [ajuint] Transcript start coordinate
** @param [r] end [ajuint] Transcript end coordinate
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptMapperTranscriptToSlice(EnsPTranscript transcript,
                                            ajuint start,
                                            ajuint end,
                                            AjPList mrs)
{
    AjPStr src = NULL;

    if(!transcript)
        return ajFalse;

    if(!start)
    {
        ajDebug("ensTranscriptMapperTranscriptToSlice "
                "requires a start coordinate.\n");

        return ajFalse;
    }

    if(!end)
    {
        ajDebug("ensTranscriptMapperTranscriptToSlice "
                "requires an end coordinate.\n");

        return ajFalse;
    }

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperTranscriptToSlice "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    if(!transcript->ExonCoordMapper)
        ensTranscriptMapperInit(transcript);

    src = ajStrNewC("transcript");

    ensMapperMapCoordinates(transcript->ExonCoordMapper,
                            transcript->Identifier,
                            start,
                            end,
                            1,
                            src,
                            mrs);

    ajStrDel(&src);

    return ajTrue;
}




/* @func ensTranscriptMapperSliceToTranscript *********************************
**
** Map Slice (genome) coordinates of an Ensembl Transcript into
** Transcript (cDNA) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::genomic2cdna
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start [ajuint] Slice start coordinate
** @param [r] end [ajuint] Slice end coordinate
** @param [r] strand [ajint] Slice strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Converts Slice coordinates to Transcript coordinates. The return value is an
** AJAX List of Mapper Result coordinates and gaps. Gaps represent intronic
** or upstream/downstream regions which do not comprise this transcript's
** cDNA. Mapper Result Coordinate objects represent Slice regions which map
** to Exons (UTRs included).
******************************************************************************/

AjBool ensTranscriptMapperSliceToTranscript(EnsPTranscript transcript,
                                            ajuint start,
                                            ajuint end,
                                            ajint strand,
                                            AjPList mrs)
{
    ajuint srid = 0;

    AjPStr src = NULL;

    EnsPSlice slice = NULL;

    if(!transcript)
        return ajFalse;

    if(!start)
    {
        ajDebug("ensTranscriptMapperSliceToTranscript "
                "requires a start coordinate.\n");

        return ajFalse;
    }

    if(!end)
    {
        ajDebug("ensTranscriptMapperSliceToTranscript "
                "requires an end coordinate.\n");

        return ajFalse;
    }

    if(!strand)
    {
        ajDebug("ensTranscriptMapperSliceToTranscript "
                "requires strand information.\n");

        return ajFalse;
    }

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperSliceToTranscript "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    if(!transcript->ExonCoordMapper)
        ensTranscriptMapperInit(transcript);

    slice = ensFeatureGetSlice(transcript->Feature);

    srid = ensSliceGetSeqregionIdentifier(slice);

    src = ajStrNewC("slice");

    ensMapperMapCoordinates(transcript->ExonCoordMapper,
                            srid,
                            start,
                            end,
                            strand,
                            src,
                            mrs);

    ajStrDel(&src);

    return ajTrue;
}




/* @func ensTranscriptMapperTranslationToSlice ********************************
**
** Map Translation (Peptide) coordinates of an Ensembl Transcript into Slice
** (genome) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::pep2genomic
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start [ajuint] Transcript start coordinate
** @param [r] end [ajuint] Transcript end coordinate
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Converts Translation coordinates into Slice coordinates. The Mapper Result
** Coordinates returned are relative to the same slice that the Transcript
** used to initialise this Transcript Mapper was on.
******************************************************************************/

AjBool ensTranscriptMapperTranslationToSlice(EnsPTranscript transcript,
                                             ajuint start,
                                             ajuint end,
                                             AjPList mrs)
{
    if(!transcript)
        return ajFalse;

    if(!start)
    {
        ajDebug("ensTranscriptMapperTranslationToSlice "
                "requires a start coordinate.\n");

        return ajFalse;
    }

    if(!end)
    {
        ajDebug("ensTranscriptMapperTranslationToSlice "
                "requires an end coordinate.\n");

        return ajFalse;
    }

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperTranslationToSlice "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    /* Move start and end into cDNA coordinates. */

    start = 3 * start - 2 + (transcript->TranscriptCodingStart - 1);

    end = 3 * end + (transcript->TranscriptCodingStart - 1);

    return ensTranscriptMapperTranscriptToSlice(transcript, start, end, mrs);
}




/* @func ensTranscriptMapperSliceToCDS ****************************************
**
** Map Translation (Peptide) coordinates of an Ensembl Transcript into coding
** sequence coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::genomic2cds
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start  [ajuint] Transcript start coordinate
** @param [r] end    [ajuint] Transcript end coordinate
** @param [r] strand [ajint] Transcript strand
** @param [u] mrs    [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptMapperSliceToCDS(EnsPTranscript transcript,
                                     ajuint start,
                                     ajuint end,
                                     ajint strand,
                                     AjPList mrs)
{
    ajuint cdsstart = 0;
    ajuint cdsend   = 0;

    AjPList result = NULL;

    EnsPMapperresult mr     = NULL;
    EnsPMapperresult gcmr   = NULL;
    EnsPMapperresult endgap = NULL;

    if(!transcript)
        return ajFalse;

    if(!start)
    {
        ajDebug("ensTranscriptMapperSliceToCDS "
                "requires a start coordinate.\n");

        return ajFalse;
    }

    if(!end)
    {
        ajDebug("ensTranscriptMapperSliceToCDS "
                "requires an end coordinate.\n");

        return ajFalse;
    }

    if(!strand)
    {
        ajDebug("ensTranscriptMapperSliceToCDS "
                "requires strand information.\n");

        return ajFalse;
    }

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperSliceToCDS "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    if(start > (end + 1))
        ajFatal("ensTranscriptMapperSliceToCDS requires start %u to be "
                "less than end %u + 1.\n", start, end);

    /*
    ** For non-cooding Transcripts return a Mapper Result Gap as there is no
    ** coding sequence.
    */

    if(!transcript->TranscriptCodingStart)
    {
        mr = MENSMAPPERGAPNEW(start, end, 0);

        ajListPushAppend(mrs, (void *) mr);

        return ajTrue;
    }

    result = ajListNew();

    ensTranscriptMapperSliceToTranscript(transcript,
                                         start,
                                         end,
                                         strand,
                                         result);

    while(ajListPop(result, (void **) &gcmr))
    {
        if(ensMapperresultGetType(gcmr) == ensEMapperresultGap)
            ajListPushAppend(mrs, (void *) gcmr);
        else
        {
            if((ensMapperresultGetStrand(gcmr) < 0) ||
               (ensMapperresultGetEnd(gcmr) <
                (ajint) transcript->TranscriptCodingStart) ||
               (ensMapperresultGetStart(gcmr) >
                (ajint) transcript->TranscriptCodingEnd))
            {
                /* All gap - does not map to peptide. */

                mr = MENSMAPPERGAPNEW(start, end, 0);

                ajListPushAppend(mrs, (void *) mr);
            }
            else
            {
                /* We know area is at least partially overlapping CDS. */

                cdsstart = start - transcript->TranscriptCodingStart + 1;

                cdsend = end - transcript->TranscriptCodingStart + 1;

                if(start < transcript->TranscriptCodingStart)
                {
                    /* Start coordinate is in the 5' UTR. */

                    mr = MENSMAPPERGAPNEW(
                        start,
                        transcript->TranscriptCodingStart - 1,
                        0);

                    ajListPushAppend(mrs, (void *) mr);

                    /* Start is now relative to start of CDS. */

                    cdsstart = 1;
                }

                endgap = NULL;

                if(end > transcript->TranscriptCodingEnd)
                {
                    /* End coordinate is in the 3' UTR. */

                    endgap = MENSMAPPERGAPNEW(
                        transcript->TranscriptCodingEnd + 1,
                        end,
                        0);

                    /* Adjust end coordinate relative to CDS start. */

                    cdsend = transcript->TranscriptCodingEnd -
                        transcript->TranscriptCodingStart + 1;
                }

                /*
                ** Start and end are now entirely in CDS and relative
                ** to CDS start.
                */

                mr = MENSMAPPERCOORDINATENEW(
                    ensMapperresultGetObjectIdentifier(gcmr),
                    cdsstart,
                    cdsend,
                    ensMapperresultGetStrand(gcmr),
                    ensMapperresultGetCoordsystem(gcmr),
                    0);

                ajListPushAppend(mrs, (void *) mr);

                if(endgap)
                    ajListPushAppend(mrs, (void *) endgap);
            }

            ensMapperresultDel(&gcmr);
        }
    }

    ajListFree(&result);

    return ajTrue;
}




/* @func ensTranscriptMapperSliceToTranslation ********************************
**
** Map Slice (genome) coordinates of an Ensembl Transcript into
** Translation (peptide) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::genomic2pep
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start [ajuint] Slice start coordinate
** @param [r] end [ajuint] Slice end coordinate
** @param [r] strand [ajint] Slice strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptMapperSliceToTranslation(EnsPTranscript transcript,
                                             ajuint start,
                                             ajuint end,
                                             ajint strand,
                                             AjPList mrs)
{
    ajuint pepstart = 0;
    ajuint pepend   = 0;
    ajuint shift    = 0;

    AjPList result = NULL;

    EnsPMapperresult mr   = NULL;
    EnsPMapperresult gcmr = NULL;

    if(!transcript)
        return ajFalse;

    if(!start)
    {
        ajDebug("ensTranscriptMapperSliceToTranslation "
                "requires a start coordinate.\n");

        return ajFalse;
    }

    if(!end)
    {
        ajDebug("ensTranscriptMapperSliceToTranslation "
                "requires an end coordinate.\n");

        return ajFalse;
    }

    if(!strand)
    {
        ajDebug("ensTranscriptMapperSliceToTranslation "
                "requires strand information.\n");

        return ajFalse;
    }

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperSliceToTranslation "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    result = ajListNew();

    ensTranscriptMapperSliceToCDS(transcript, start, end, strand, result);

    /* Take possible N padding at beginning of CDS. */

    shift = (transcript->StartPhase > 0) ? transcript->StartPhase : 0;

    while(ajListPop(result, (void **) &gcmr))
    {
        if(ensMapperresultGetType(gcmr) == ensEMapperresultGap)
            ajListPushAppend(mrs, (void *) gcmr);
        else
        {
            /*
            ** Start and end coordinates are now entirely in CDS and
            ** relative to CDS start.
            ** Convert to peptide coordinates.
            */

            pepstart = (ajuint)
                ((ensMapperresultGetStart(gcmr) + shift + 2) / 3);

            pepend = (ajuint) ((ensMapperresultGetEnd(gcmr) + shift + 2) / 3);

            mr = MENSMAPPERCOORDINATENEW(
                ensMapperresultGetObjectIdentifier(gcmr),
                pepstart,
                pepend,
                ensMapperresultGetStrand(gcmr),
                ensMapperresultGetCoordsystem(gcmr),
                0);

            ajListPushAppend(mrs, (void *) mr);

            ensMapperresultDel(&gcmr);
        }

    }

    ajListFree(&result);

    return ajTrue;
}




/* @func ensSupportingfeatureadaptorFetchAllByTranscript **********************
**
** Fetch Ensembl Supporting Features via an Ensembl Transcript.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSupportingfeatureadaptorFetchAllByTranscript(
    EnsPDatabaseadaptor dba,
    EnsPTranscript transcript,
    AjPList bafs)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr type      = NULL;

    EnsPFeature tfeature = NULL;
    EnsPFeature nfeature = NULL;
    EnsPFeature ofeature = NULL;

    EnsPSlice tslice = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDNAAlignFeatureadaptor dafa = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if(!dba)
        return ajFalse;

    if(!transcript)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(!ensTranscriptGetIdentifier(transcript))
    {
        ajDebug("ensSupportingfeatureadaptorFetchAllByTranscript cannot get "
                "Supporting Features for a Transcript without an "
                "identifier.\n");

        return ajFalse;
    }

    tfeature = ensTranscriptGetFeature(transcript);

    tslice = ensFeatureGetSlice(tfeature);

    dafa = ensRegistryGetDNAAlignFeatureadaptor(dba);

    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    statement = ajFmtStr(
        "SELECT "
        "transcript_supporting_feature.feature_type, "
        "transcript_supporting_feature.feature_id "
        "FROM "
        "transcript_supporting_feature "
        "WHERE "
        "transcript_supporting_feature.transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

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
        {
            ensDNAAlignFeatureadaptorFetchByIdentifier(dafa,
                                                       identifier,
                                                       &baf);
        }
        else if(ajStrMatchC(type, "protein_align_feature"))
        {
            ensProteinalignfeatureadaptorFetchByIdentifier(pafa,
                                                           identifier,
                                                           &baf);
        }
        else
            ajWarn("ensSupportingfeatureadaptorFetchAllByTranscript got "
                   "unexpected value in "
                   "transcript_supporting_feature.feature_type '%S'.\n", type);

        if(baf)
        {
            ofeature = ensFeaturepairGetSourceFeature(baf->Featurepair);

            nfeature = ensFeatureTransfer(ofeature, tslice);

            ensFeaturepairSetSourceFeature(baf->Featurepair, nfeature);

            ensFeatureDel(&nfeature);

            ajListPushAppend(bafs, (void *) baf);
        }
        else
        {
            ajDebug("ensSupportingfeatureadaptorFetchAllByTranscript could "
                    "not retrieve Supporting feature of type '%S' and "
                    "identifier %u from database.\n", type, identifier);
        }

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPTranscriptadaptor] Transcript Adaptor ********************
**
** Functions for manipulating Ensembl Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor CVS Revision: 1.121
**
** @nam2rule Transcriptadaptor
**
******************************************************************************/

static const char *transcriptadaptorTables[] =
{
    "transcript",
    "transcript_stable_id",
    "xref",
    "external_db",
    NULL
};

static const char *transcriptadaptorColumns[] =
{
    "transcript.transcript_id",
    "transcript.seq_region_id",
    "transcript.seq_region_start",
    "transcript.seq_region_end",
    "transcript.seq_region_strand",
    "transcript.analysis_id",
    "transcript.display_xref_id",
    "transcript.description",
    "transcript.biotype",
    "transcript.status",
    "transcript.is_current",
    "transcript.gene_id",
    "transcript_stable_id.stable_id",
    "transcript_stable_id.version",
    "transcript_stable_id.created_date",
    "transcript_stable_id.modified_date",
    "xref.external_db_id",
    "xref.dbprimary_acc",
    "xref.display_label",
    "xref.version",
    "xref.description",
    "xref.info_type",
    "xref.info_text",
    NULL
};

static EnsOBaseadaptorLeftJoin transcriptadaptorLeftJoin[] =
{
    {
        "transcript_stable_id",
        "transcript_stable_id.transcript_id = transcript.transcript_id"
    },
    {"xref", "xref.xref_id = transcript.display_xref_id"},
    {"external_db", "external_db.external_db_id = xref.external_db_id"},
    {NULL, NULL}
};

static const char *transcriptadaptorDefaultCondition = NULL;

static const char *transcriptadaptorFinalCondition = NULL;




/* @funcstatic transcriptadaptorFetchAllBySQL *********************************
**
** Fetch all Ensembl Transcript objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool transcriptadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList transcripts)
{
    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint erid   = 0;
    ajuint geneid = 0;
    ajuint edbid  = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint version = 0;

    AjBool current = AJFALSE;

    EnsETranscriptStatus estatus =
        ensETranscriptStatusNULL;

    EnsEExternalreferenceInfoType einfotype =
        ensEExternalreferenceInfoTypeNULL;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr description   = NULL;
    AjPStr biotype       = NULL;
    AjPStr status        = NULL;
    AjPStr stableid      = NULL;
    AjPStr cdate         = NULL;
    AjPStr mdate         = NULL;
    AjPStr erprimaryid   = NULL;
    AjPStr erdisplayid   = NULL;
    AjPStr erversion     = NULL;
    AjPStr erdescription = NULL;
    AjPStr erinfotype    = NULL;
    AjPStr erinfotext    = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPFeature feature = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca  = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("transcriptadaptorFetchAllBySQL"))
        ajDebug("transcriptadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  transcripts %p\n",
                dba,
                statement,
                am,
                slice,
                transcripts);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    tca = ensRegistryGetTranscriptadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        srid        = 0;
        srstart     = 0;
        srend       = 0;
        srstrand    = 0;
        analysisid  = 0;
        erid        = 0;
        description = ajStrNew();
        biotype     = ajStrNew();
        status      = ajStrNew();
        current     = ajFalse;
        geneid      = 0;
        stableid    = ajStrNew();
        version     = 0;
        cdate       = ajStrNew();
        mdate       = ajStrNew();
        edbid       = 0;
        erprimaryid = ajStrNew();
        erdisplayid = ajStrNew();
        erversion   = ajStrNew();

        erdescription = ajStrNew();
        erinfotype    = ajStrNew();
        erinfotext    = ajStrNew();

        estatus   = ensETranscriptStatusNULL;
        einfotype = ensEExternalreferenceInfoTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToUint(sqlr, &erid);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &biotype);
        ajSqlcolumnToStr(sqlr, &status);
        ajSqlcolumnToBool(sqlr, &current);
        ajSqlcolumnToUint(sqlr, &geneid);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);
        ajSqlcolumnToUint(sqlr, &edbid);
        ajSqlcolumnToStr(sqlr, &erprimaryid);
        ajSqlcolumnToStr(sqlr, &erdisplayid);
        ajSqlcolumnToStr(sqlr, &erversion);
        ajSqlcolumnToStr(sqlr, &erdescription);
        ajSqlcolumnToStr(sqlr, &erinfotype);
        ajSqlcolumnToStr(sqlr, &erinfotext);

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
            ajFatal("transcriptadaptorFetchAllBySQL got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("transcriptadaptorFetchAllBySQL got a "
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

                ajStrDel(&description);
                ajStrDel(&biotype);
                ajStrDel(&status);
                ajStrDel(&stableid);
                ajStrDel(&cdate);
                ajStrDel(&mdate);
                ajStrDel(&erprimaryid);
                ajStrDel(&erdisplayid);
                ajStrDel(&erversion);
                ajStrDel(&erdescription);
                ajStrDel(&erinfotype);
                ajStrDel(&erinfotext);

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
                ajFatal("transcriptadaptorFetchAllBySQL got a Slice, "
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
                /* Next feature but destroy first! */

                ajStrDel(&description);
                ajStrDel(&biotype);
                ajStrDel(&status);
                ajStrDel(&stableid);
                ajStrDel(&cdate);
                ajStrDel(&mdate);
                ajStrDel(&erprimaryid);
                ajStrDel(&erdisplayid);
                ajStrDel(&erversion);
                ajStrDel(&erdescription);
                ajStrDel(&erinfotype);
                ajStrDel(&erinfotext);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        if(erid)
        {
            ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

            einfotype = ensExternalreferenceInfoTypeFromStr(erinfotype);

            if(!einfotype)
                ajDebug("transcriptadaptorFetchAllBySQL encountered "
                        "unexpected string '%S' in the "
                        "'xref.infotype' field.\n", erinfotype);

            dbe = ensDatabaseentryNew((EnsPDatabaseentryadaptor) NULL,
                                      erid,
                                      (EnsPAnalysis) NULL,
                                      edb,
                                      erprimaryid,
                                      erdisplayid,
                                      erversion,
                                      erdescription,
                                      (AjPStr) NULL,
                                      einfotype,
                                      erinfotext);

            ensExternaldatabaseDel(&edb);
        }
        else
            dbe = NULL;

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        /* Set the Transcript status. */

        estatus = ensTranscriptStatusFromStr(status);

        if(!estatus)
            ajFatal("transcriptadaptorFetchAllBySQL encountered "
                    "unexpected string '%S' in the "
                    "'transcript.status' field.\n", status);

        /* Finally, create a new Ensembl Transcript. */

        feature = ensFeatureNewS(analysis,
                                 srslice,
                                 slstart,
                                 slend,
                                 slstrand);

        transcript = ensTranscriptNew(tca,
                                      identifier,
                                      feature,
                                      dbe,
                                      description,
                                      biotype,
                                      estatus,
                                      current,
                                      stableid,
                                      version,
                                      cdate,
                                      mdate,
                                      (AjPList) NULL);

        ajListPushAppend(transcripts, (void *) transcript);

        ensFeatureDel(&feature);

        ajStrDel(&description);
        ajStrDel(&biotype);
        ajStrDel(&status);
        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
        ajStrDel(&erprimaryid);
        ajStrDel(&erdisplayid);
        ajStrDel(&erversion);
        ajStrDel(&erdescription);
        ajStrDel(&erinfotype);
        ajStrDel(&erinfotext);

        ensAnalysisDel(&analysis);

        ensDatabaseentryDel(&dbe);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic transcriptadaptorCacheReference ********************************
**
** Wrapper function to reference an Ensembl Transcript
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Transcript
**
** @return [void*] Ensembl Transcript or NULL
** @@
******************************************************************************/

static void* transcriptadaptorCacheReference(void *value)
{
    if(!value)
        return NULL;

    return (void *) ensTranscriptNewRef((EnsPTranscript) value);
}




/* @funcstatic transcriptadaptorCacheDelete ***********************************
**
** Wrapper function to delete an Ensembl Transcript
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Transcript address
**
** @return [void]
** @@
******************************************************************************/

static void transcriptadaptorCacheDelete(void **value)
{
    if(!value)
        return;

    ensTranscriptDel((EnsPTranscript *) value);

    return;
}




/* @funcstatic transcriptadaptorCacheSize *************************************
**
** Wrapper function to determine the memory size of an Ensembl Transcript
** via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Transcript
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

static ajulong transcriptadaptorCacheSize(const void *value)
{
    if(!value)
        return 0;

    return ensTranscriptGetMemsize((const EnsPTranscript) value);
}




/* @funcstatic transcriptadaptorGetFeature ************************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Transcript from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Transcript
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature transcriptadaptorGetFeature(const void *value)
{
    if(!value)
        return NULL;

    return ensTranscriptGetFeature((const EnsPTranscript) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Transcript Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Transcript Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPTranscriptadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensTranscriptadaptorNew **********************************************
**
** Default Ensembl Transcript Adaptor constructor.
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
** @see ensRegistryGetTranscriptadaptor
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPTranscriptadaptor tca = NULL;

    if(!dba)
        return NULL;

    AJNEW0(tca);

    tca->Adaptor = ensFeatureadaptorNew(
        dba,
        transcriptadaptorTables,
        transcriptadaptorColumns,
        transcriptadaptorLeftJoin,
        transcriptadaptorDefaultCondition,
        transcriptadaptorFinalCondition,
        transcriptadaptorFetchAllBySQL,
        (void* (*)(const void* key)) NULL,
        transcriptadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        transcriptadaptorCacheDelete,
        transcriptadaptorCacheSize,
        transcriptadaptorGetFeature,
        "Transcript");

    return tca;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Transcript Adaptor.
**
** @fdata [EnsPTranscriptadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Transcript Adaptor object.
**
** @argrule * Ptca [EnsPTranscriptadaptor*] Ensembl Transcript Adaptor
**                                          object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTranscriptadaptorDel **********************************************
**
** Default destructor for an Ensembl Transcript Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Ptca [EnsPTranscriptadaptor*] Ensembl Transcript Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensTranscriptadaptorDel(EnsPTranscriptadaptor *Ptca)
{
    EnsPTranscriptadaptor pthis = NULL;

    if(!Ptca)
        return;

    if(!*Ptca)
        return;

    pthis = *Ptca;

    ensFeatureadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Ptca = NULL;

    return;
}




/* @func ensTranscriptadaptorGetFeatureadaptor ********************************
**
** Get the Ensembl Feature Adaptor element of an Ensembl Transcript Adaptor.
**
** @param [r] tca [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

EnsPFeatureadaptor ensTranscriptadaptorGetFeatureadaptor(
    const EnsPTranscriptadaptor tca)
{
    if(!tca)
        return NULL;

    return tca->Adaptor;
}




/* @func ensTranscriptadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Transcript Adaptor.
**
** @param [r] tca [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensTranscriptadaptorGetDatabaseadaptor(
    const EnsPTranscriptadaptor tca)
{
    if(!tca)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(tca->Adaptor);
}




/* @func ensTranscriptadaptorFetchAll *****************************************
**
** Fetch all Ensembl Transcripts.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: In this implementation fetch all really fetches all. The Perl API
** adds the following constraint:
** "transcript.biotype != 'LRG_gene' and transcript.is_current = 1"
******************************************************************************/

AjBool ensTranscriptadaptorFetchAll(EnsPTranscriptadaptor tca,
                                    AjPList transcripts)
{
    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    return ensBaseadaptorFetchAll(ba, transcripts);
}




/* @func ensTranscriptadaptorFetchAllByGene ***********************************
**
** Fetch all Ensembl Transcripts via an Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_Gene
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] gene [EnsPGene] Ensembl Gene
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllByGene(EnsPTranscriptadaptor tca,
                                          EnsPGene gene,
                                          AjPList transcripts)
{
    AjPList list = NULL;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature gfeature = NULL;

    EnsPSlice gslice    = NULL;
    EnsPSlice tslice    = NULL;
    EnsPSliceadaptor sa = NULL;

    EnsPTranscript transcript    = NULL;
    EnsPTranscript newtranscript = NULL;

    if(!tca)
        return ajFalse;

    if(!gene)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    /*
    ** Use the fetch_all_by_Slice_constraint method because it
    ** handles the difficult Haps/PARs and coordinate remapping.
    **
    ** Get a Slice that entirely overlaps the gene.  This is because we
    ** want all Transcripts to be retrieved, not just ones overlapping
    ** the Slice the Gene is on, as the Gene may only partially overlap the
    ** Slice. For speed reasons, only use a different Slice if necessary
    ** though.
    */

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    if(!dba)
    {
        ajWarn("ensTranscriptadaptorFetchAllByGene cannot get Transcripts "
               "without an Ensembl Database Adaptor defined in the "
               "Transcript Adaptor.\n");

        return ajFalse;
    }

    gfeature = ensGeneGetFeature(gene);

    if(!gfeature)
    {
        ajWarn("ensTranscriptadaptorFetchAllByGene cannot get Transcripts "
               "without an Ensembl Feature defined in the Gene.\n");

        return ajFalse;
    }

    gslice = ensFeatureGetSlice(gfeature);

    if(!gslice)
    {
        ajWarn("ensTranscriptadaptorFetchAllByGene cannot get Transcripts "
               "without an Ensembl Slice defined in the Ensembl Feature of "
               "the Gene.\n");

        return ajFalse;
    }

    if((ensFeatureGetStart(gfeature) < 1) ||
       (ensFeatureGetEnd(gfeature) > (ajint) ensSliceGetLength(gslice)))
    {
        sa = ensRegistryGetSliceadaptor(dba);

        ensSliceadaptorFetchByFeature(sa, gfeature, 0, &tslice);
    }
    else
        tslice = ensSliceNewRef(gslice);

    constraint = ajFmtStr(
        "transcript.gene_id = %u",
        ensGeneGetIdentifier(gene));

    list = ajListNew();

    ensFeatureadaptorFetchAllBySliceConstraint(tca->Adaptor,
                                               tslice,
                                               constraint,
                                               (const AjPStr) NULL,
                                               list);

    while(ajListPop(list, (void **) &transcript))
    {
        if(ensSliceMatch(gslice, tslice))
            ajListPushAppend(transcripts, (void *) transcript);
        else
        {
            newtranscript = ensTranscriptTransfer(transcript, tslice);

            ajListPushAppend(transcripts, (void *) newtranscript);

            ensTranscriptDel(&transcript);
        }
    }

    ajListFree(&list);

    ajStrDel(&constraint);

    ensSliceDel(&tslice);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchAllBySlice **********************************
**
** Fetch all Ensembl Transcripts via an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Transcripts before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_Slice
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] constraint [const AjPStr] SQL constraint
** @param [r] loadexons [AjBool] Load Ensembl Exons
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllBySlice(EnsPTranscriptadaptor tca,
                                           EnsPSlice slice,
                                           const AjPStr anname,
                                           const AjPStr constraint,
                                           AjBool loadexons,
                                           AjPList transcripts)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    ajint start = INT_MAX;
    ajint end   = INT_MIN;
    ajint rank  = 0;

    register ajuint i = 0;

    ajuint exid = 0;
    ajuint trid = 0;

    ajuint *Pidentifier = NULL;

    AjIList iter  = NULL;
    AjPList exons = NULL;
    AjPList list  = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement    = NULL;
    AjPStr trconstraint = NULL;
    AjPStr csv          = NULL;

    AjPTable extable = NULL;
    AjPTable trtable = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon      = NULL;
    EnsPExon newexon   = NULL;
    EnsPExonadaptor ea = NULL;

    EnsPFeature feature = NULL;

    EnsPSlice newslice  = NULL;
    EnsPSliceadaptor sa = NULL;

    EnsPTranscript transcript = NULL;

    EnsPTranslationadaptor tla = NULL;

    TranscriptPExonRank trex = NULL;

    if(!tca)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    if(constraint && ajStrGetLen(constraint))
        trconstraint = ajFmtStr("transcript.is_current = 1 AND %S",
                                constraint);
    else
        trconstraint = ajStrNewC("transcript.is_current = 1");

    ensFeatureadaptorFetchAllBySliceConstraint(tca->Adaptor,
                                               slice,
                                               trconstraint,
                                               anname,
                                               transcripts);

    ajStrDel(&trconstraint);

    /* If there are 0 or 1 Transcripts, still do lazy-loading. */

    if(!loadexons || ajListGetLength(transcripts) < 2)
        return ajTrue;

    /*
    ** Preload all Ensembl Exons now, instead of lazy loading later, which is
    ** faster than one SQL query per Transcript.
    ** First check if the Exons are already preloaded.
    ** TODO: This should test all Exons.
    */

    ajListPeekFirst(transcripts, (void **) &transcript);

    if(transcript->Exons)
        return ajTrue;

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    ea = ensRegistryGetExonadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    tla = ensRegistryGetTranslationadaptor(dba);

    /*
    ** Get the extent of the region spanned by Transcripts, prepare a
    ** comma-separared list of Transcript identifiers and put Transcripts
    ** into an AJAX Table indexed by their identifier.
    */

    csv = ajStrNew();

    trtable = MENSTABLEUINTNEW(0);

    iter = ajListIterNew(transcripts);

    while(!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        feature = ensTranscriptGetFeature(transcript);

        start = (ensFeatureGetSeqregionStart(feature) < start) ?
            ensFeatureGetSeqregionStart(feature) : start;

        end = (ensFeatureGetSeqregionEnd(feature) > end) ?
            ensFeatureGetSeqregionEnd(feature) : end;

        ajFmtPrintAppS(&csv, "%u, ", ensTranscriptGetIdentifier(transcript));

        /*
        ** Put all Ensembl Transcripts into an AJAX Table indexed by their
        ** identifier.
        */

        AJNEW0(Pidentifier);

        *Pidentifier = ensTranscriptGetIdentifier(transcript);

        ajTablePut(trtable,
                   (void *) Pidentifier,
                   (void *) ensTranscriptNewRef(transcript));
    }

    ajListIterDel(&iter);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    if((start >= ensSliceGetStart(slice)) && (end <= ensSliceGetEnd(slice)))
        newslice = ensSliceNewRef(slice);
    else
        ensSliceadaptorFetchBySlice(sa,
                                    slice,
                                    start,
                                    end,
                                    ensSliceGetStrand(slice),
                                    &newslice);

    /* Associate Exon identifiers with Transcripts and Exon ranks. */

    statement = ajFmtStr(
        "SELECT "
        "exon_transcript.transcript_id, "
        "exon_transcript.exon_id, "
        "exon_transcript.rank "
        "FROM "
        "exon_transcript "
        "WHERE "
        "exon_transcript.transcript_id IN (%S)",
        csv);

    ajStrAssignClear(&csv);

    extable = MENSTABLEUINTNEW(0);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        trid = 0;
        exid = 0;
        rank = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
        ajSqlcolumnToUint(sqlr, &exid);
        ajSqlcolumnToInt(sqlr, &rank);

        list = (AjPList) ajTableFetch(extable, (void *) &exid);

        if(!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = exid;

            list = ajListNew();

            ajTablePut(extable, (void *) Pidentifier, (void *) list);
        }

        ajListPushAppend(list, (void *) transcriptExonRankNew(trid, rank));
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    /* Get all Exon identifiers as comma-separated values. */

    ajTableToarrayKeys(extable, &keyarray);

    for(i = 0; keyarray[i]; i++)
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint *) keyarray[i]));

    AJFREE(keyarray);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    trconstraint = ajFmtStr("exon.exon_id IN (%S)", csv);

    ajStrDel(&csv);

    exons = ajListNew();

    ensExonadaptorFetchAllBySliceConstraint(ea, newslice, trconstraint, exons);

    ajStrDel(&trconstraint);

    /* Transfer Exons onto Transcript Slice, and add them to Transcripts. */

    while(ajListPop(exons, (void **) &exon))
    {
        newexon = ensExonTransfer(exon, newslice);

        if(!newexon)
            ajFatal("ensTranscriptAdaptorFetchAllBySlice could not transfer "
                    "Exon onto new Slice.\n");

        exid = ensExonGetIdentifier(newexon);

        list = (AjPList) ajTableFetch(extable, &exid);

        iter = ajListIterNew(list);

        while(!ajListIterDone(iter))
        {
            trex = (TranscriptPExonRank) ajListIterGet(iter);

            transcript = (EnsPTranscript) ajTableFetch(
                trtable,
                &trex->TranscriptIdentifier);

            ensTranscriptAddExon(transcript, newexon, trex->Rank);
        }

        ajListIterDel(&iter);

        ensExonDel(&newexon);
        ensExonDel(&exon);
    }

    ajListFree(&exons);

    ensTranslationadaptorFetchAllByTranscriptTable(tla, trtable);

    /*
    ** Clear and delete the AJAX Table of unsigned integer key and
    ** Ensembl Transcript value data.
    */

    ajTableToarrayKeysValues(trtable, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        AJFREE(keyarray[i]);

        ensTranscriptDel((EnsPTranscript *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableFree(&trtable);

    /*
    ** Clear and detete the AJAX Table of unsigned integer key and
    ** AJAX List value data.
    */

    ajTableToarrayKeysValues(extable, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        AJFREE(keyarray[i]);

        while(ajListPop((AjPList) valarray[i], (void **) &trex))
            transcriptExonRankDel(&trex);

        ajListFree((AjPList *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableFree(&extable);

    ensSliceDel(&newslice);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchAllByStableIdentifier ***********************
**
** Fetch all Ensembl Transcript versions via a stable identifier.
**
** The caller is responsible for deleting the Ensembl Transcripts before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_versions_by_stable_id
** @param [r] tca [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllByStableIdentifier(
    const EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    AjPList transcripts)
{
    char *txtstableid = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("transcript_stable_id.stable_id = '%s'",
                          txtstableid);

    ajCharDel(&txtstableid);

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               transcripts);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl Transcript via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByIdentifier(EnsPTranscriptadaptor tca,
                                             ajuint identifier,
                                             EnsPTranscript *Ptranscript)
{
    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    *Ptranscript = (EnsPTranscript)
        ensBaseadaptorFetchByIdentifier(ba, identifier);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByStableIdentifier **************************
**
** Fetch an Ensembl Transcript via its stable identifier and version.
** In case a version is not specified, the current Ensembl Transcript
** will be returned.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_stable_id
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByStableIdentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    ajuint version,
    EnsPTranscript *Ptranscript)
{
    char *txtstableid = NULL;

    AjPList transcripts = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPTranscript transcript = NULL;

    if(!tca)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if(version)
        constraint = ajFmtStr(
            "transcript_stable_id.stable_id = '%s' "
            "AND "
            "transcript_stable_id.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "transcript_stable_id.stable_id = '%s' "
            "AND "
            "transcript.is_current = 1",
            txtstableid);

    ajCharDel(&txtstableid);

    transcripts = ajListNew();

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               transcripts);

    if(ajListGetLength(transcripts) > 1)
        ajDebug("ensTranscriptadaptorFetchByStableIdentifier got more than "
                "one Transcript for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(transcripts, (void **) Ptranscript);

    while(ajListPop(transcripts, (void **) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&transcripts);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByExonIdentifier ****************************
**
** Fetch an Ensembl Transcript via an Ensembl Exon identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_exon_id
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] Ensembl Exon identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByExonIdentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript* Ptranscript)
{
    ajuint trid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(tca->Adaptor);

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id "
        "FROM "
        "exon_transcript, "
        "transcript "
        "WHERE "
        "exon_transcript.exon_id = %u "
        "AND "
        "exon_transcript.transcript_id = transcript.transcript_id "
        "AND "
        "transcript.is_current = 1",
        identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        trid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if(!trid)
    {
        ajDebug("ensTranscriptadaptorFetchByExonIdentifier could not "
                "get Transcript identifier for Exon identifier %u.\n",
                identifier);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    *Ptranscript = (EnsPTranscript) ensBaseadaptorFetchByIdentifier(ba, trid);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByExonStableIdentifier **********************
**
** Fetch an Ensembl Transcript via an Ensembl Exon stable identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_exon_stable_id
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Ensembl Exon stable identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByExonStableIdentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript *Ptranscript)
{
    char *txtstableid = NULL;

    ajuint trid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!tca)
        return ajFalse;

    if(!stableid && !ajStrGetLen(stableid))
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(tca->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id "
        "FROM "
        "exon_stable_id, "
        "exon_transcript, "
        "transcript "
        "WHERE "
        "exon_stable_id.stable_id = '%s' "
        "AND "
        "exon_stable_id.exon_id = exon_transcript.exon_id "
        "AND "
        "exon_transcript.transcript_id = transcript.transcript_id "
        "AND "
        "transcript.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        trid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if(!trid)
    {
        ajDebug("ensTranscriptadaptorFetchByExonStableIdentifier "
                "could not get Transcript identifier for Exon "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    *Ptranscript = (EnsPTranscript) ensBaseadaptorFetchByIdentifier(ba, trid);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByTranslationIdentifier *********************
**
** Fetch an Ensembl Transcript via its Ensembl Translation identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_translation_id
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] Ensembl Translation identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationIdentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript* Ptranscript)
{
    ajuint trid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(tca->Adaptor);

    statement = ajFmtStr(
        "SELECT "
        "translation.transcript_id "
        "FROM "
        "translation "
        "WHERE "
        "translation.translation_id = %u",
        identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        trid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if(!trid)
    {
        ajDebug("ensTranscriptadaptorFetchByTranslationIdentifier could not "
                "get Transcript identifier for Translation identifier %u.\n",
                identifier);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    *Ptranscript = (EnsPTranscript) ensBaseadaptorFetchByIdentifier(ba, trid);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByTranslationStableIdentifier ***************
**
** Fetch an Ensembl Transcript via its Ensembl Translation stable identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_translation_stable_id
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Ensembl Translation stable identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationStableIdentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript* Ptranscript)
{
    char *txtstableid = NULL;

    ajuint trid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!tca)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(tca->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id "
        "FROM "
        "translation_stable_id, "
        "translation, "
        "transcript "
        "WHERE "
        "translation_stable_id.stable_id = '%s' "
        "AND "
        "translation_stable_id.translation_id = "
        "translation.translation_id "
        "AND "
        "translation.transcript_id = transcript.transcript_id "
        "AND "
        "transcript.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        trid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if(!trid)
    {
        ajDebug("ensTranscriptadaptorFetchByTranslationStableIdentifier "
                "could not get Transcript identifier for Translation "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    *Ptranscript = (EnsPTranscript) ensBaseadaptorFetchByIdentifier(ba, trid);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByDisplayLabel ******************************
**
** Fetch an Ensembl Transcript via its display label.
**
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_display_label
** @param [r] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] label [const AjPStr] Display label
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByDisplayLabel(EnsPTranscriptadaptor tca,
                                               const AjPStr label,
                                               EnsPTranscript *Ptranscript)
{
    char *txtlabel = NULL;

    AjPList transcripts = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPTranscript transcript = NULL;

    if(!tca)
        return ajFalse;

    if(!label)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtlabel, label);

    constraint = ajFmtStr(
        "transcript.is_current = 1 "
        "AND "
        "xref.display_label = '%s'",
        txtlabel);

    ajCharDel(&txtlabel);

    transcripts = ajListNew();

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               transcripts);

    if(ajListGetLength(transcripts) > 1)
        ajDebug("ensTranscriptadaptorFetchByDisplayLabel got more than one "
                "Transcript for display label '%S'.\n", label);

    ajListPop(transcripts, (void **) Ptranscript);

    while(ajListPop(transcripts, (void **) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&transcripts);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchAllIdentifiers ******************************
**
** Fetch all SQL database-internal identifiers of Ensembl Transcripts.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::list_dbIDs
** @param [u] tca [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllIdentifiers(
    const EnsPTranscriptadaptor tca,
    AjPList identifiers)
{
    AjBool value = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    table = ajStrNewC("transcript");

    value = ensBaseadaptorFetchAllIdentifiers(ba,
                                              table,
                                              (AjPStr) NULL,
                                              identifiers);

    ajStrDel(&table);

    return value;
}




/* @func ensTranscriptadaptorFetchAllStableIdentifiers ************************
**
** Fetch all stable identifiers of Ensembl Transcripts.
**
** The caller is responsible for deleting the AJAX Strings before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::list_stable_ids
** @param [r] tca [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllStableIdentifiers(
    const EnsPTranscriptadaptor tca,
    AjPList identifiers)
{
    AjBool value = AJFALSE;

    AjPStr table   = NULL;
    AjPStr primary = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca->Adaptor);

    table   = ajStrNewC("transcript_stable_id");
    primary = ajStrNewC("stable_id");

    value = ensBaseadaptorFetchAllStrings(ba, table, primary, identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return value;
}

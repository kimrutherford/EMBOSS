/******************************************************************************
**
** @source Ensembl Transcript functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.5 $
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




static const char *transcriptSequenceEditCode[] =
{
    "_rna_edit",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPAnalysisadaptor
ensRegistryGetAnalysisadaptor(EnsPDatabaseadaptor dba);

extern EnsPAssemblymapperadaptor
ensRegistryGetAssemblymapperadaptor(EnsPDatabaseadaptor dba);

extern EnsPCoordsystemadaptor
ensRegistryGetCoordsystemadaptor(EnsPDatabaseadaptor dba);

extern EnsPDatabaseentryadaptor
ensRegistryGetDatabaseentryadaptor(EnsPDatabaseadaptor dba);

extern EnsPDNAAlignFeatureadaptor
ensRegistryGetDNAAlignFeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPExternaldatabaseadaptor
ensRegistryGetExternaldatabaseadaptor(EnsPDatabaseadaptor dba);

extern EnsPExonadaptor
ensRegistryGetExonadaptor(EnsPDatabaseadaptor dba);

extern EnsPProteinalignfeatureadaptor
ensRegistryGetProteinalignfeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPTranscriptadaptor
ensRegistryGetTranscriptadaptor(EnsPDatabaseadaptor dba);

extern EnsPTranslationadaptor
ensRegistryGetTranslationadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static AjBool transcriptAdaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList transcripts);

static void *transcriptAdaptorCacheReference(void *value);

static void transcriptAdaptorCacheDelete(void **value);

static ajuint transcriptAdaptorCacheSize(const void *value);

static EnsPFeature transcriptAdaptorGetFeature(const void *value);




/* @filesection enstranscript *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/



/* @datasection [EnsPTranscript] Transcript ***********************************
**
** Functions for manipulating Ensembl Transcript objects
**
** @cc Bio::EnsEMBL::Transcript CVS Revision: 1.265
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
** @argrule Obj object [EnsPTranscript] Ensembl Transcript
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
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [r] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Transcript::new
** @param [r] displaydbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] description [AjPStr] Description
** @param [u] biotype [AjPStr] Biotype
** @param [r] status [AjEnum] Status
** @param [r] current [AjBool] Current attribute
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
** @param [r] exons [AjPList] AJAX List of Ensembl Exons
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNew(EnsPTranscriptadaptor adaptor,
                                ajuint identifier,
                                EnsPFeature feature,
                                EnsPDatabaseentry displaydbe,
                                AjPStr description,
                                AjPStr biotype,
                                AjEnum status,
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
    
    transcript->Adaptor = adaptor;
    
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
    
    transcript->Translations = ajListNew();
    
    transcript->SliceCodingStart = 0;
    
    transcript->SliceCodingEnd = 0;
    
    transcript->TranscriptCodingStart = 0;
    
    transcript->TranscriptCodingEnd = 0;
    
    /*
    ** FIXME: Like in the Perl API, this is set to ajTrue by default.
    ** This would need an independent parameter or it could be dropped
    ** completely to always apply Sequence Edits.
    */
    
    transcript->EditsEnabled = ajTrue;
    
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
    
    /* Since Translations are weakly linked, initialise a new AJAX List. */
    
    transcript->Translations = ajListNew();
    
    transcript->SliceCodingStart = object->SliceCodingStart;
    
    transcript->SliceCodingEnd = object->SliceCodingEnd;
    
    transcript->TranscriptCodingStart = object->TranscriptCodingStart;
    
    transcript->TranscriptCodingEnd = object->TranscriptCodingEnd;
    
    transcript->EditsEnabled = object->EditsEnabled;
    
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
    ajuint length = 0;
    
    EnsPAttribute attribute = NULL;
    
    EnsPBasealignfeature baf = NULL;
    
    EnsPDatabaseentry dbe = NULL;
    
    EnsPExon exon = NULL;

    EnsPTranscript pthis = NULL;
    
    if(!Ptranscript)
        return;
    
    if(!*Ptranscript)
        return;

    pthis = *Ptranscript;
    
#if AJFALSE
    ajDebug("ensTranscriptDel\n"
            "  *Ptranscript %p\n",
            *Ptranscript);
#endif
    
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
    
    /*
    ** Clear and delete the AJAX List of weak references to
    ** Ensembl Translations.
    ** Since Ensembl Translations keep strong references to a Transcript,
    ** thereby retaining the Transcript, at the stage of deletion this List
    ** should be empty. In any case Translations do *not* need deletion!
    */
    
    length = ajListGetLength((*Ptranscript)->Translations);
    
    if(length)
	ajWarn("ensTranscriptDel encountered %u remaining weak references "
	       "to Ensembl Translations.\n", length);
    
    ajListFree(&pthis->Translations);
    
    /* Delete the Ensembl Transcript Mapper. */
    
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
** @nam4rule GetAttributes Return all Ensembl Attributes
** @nam4rule GetDatabaseEntries Return all Ensembl Database Entries
** @nam4rule GetExons Return all Ensembl Exons
** @nam4rule GetSupportingfeatures Return the supporting Ensembl Base Align
**                                 Features
** @nam4rule GetTranslations Return (weak links to) all Ensembl Translations
** @nam4rule GetTranscriptCodingStart Return the Translation start coordinate
**                                    in Transcript coordinates
** @nam4rule GetTranscriptCodingEnd Return the Translation end coordinate
**                                  in Transcript coordinates
** @nam4rule GetSliceCodingStart Return the Translation start coordinate
**                               in Slice coordinates
** @nam4rule GetSliceCodingEnd Return the Translation end coordinate
**                             in Slice coordinates
**
** @argrule * translation [const EnsPTranslation] Translation
**
** @valrule Adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule DisplayReference [EnsPDatabaseentry] Ensembl Database Entry
** @valrule Description [AjPStr] Description
** @valrule BioType [ajuint] Biological type
** @valrule Status [AjEnum] Status
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
** @valrule Translations [const AjPList] AJAX List of Ensembl Translations
** @valrule TranscriptCodingStart [ajuint] Translation start coordinate
** @valrule TranscriptCodingEnd [ajuint] Translation end coordinate
** @valrule SliceCodingStart [ajuint] Translation start coordinate
** @valrule SliceCodingEnd [ajuint] Translation end coordinate
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
** @return [AjEnum] Status
** @@
******************************************************************************/

AjEnum ensTranscriptGetStatus(const EnsPTranscript transcript)
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




/* @func ensTranscriptGetAttributes *******************************************
**
** Get all Ensembl Attributes of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch Ensembl Attributes
** from the Ensembl Core database in case the AJAX List is empty.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Attributes
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Attributes
** @@
******************************************************************************/

const AjPList ensTranscriptGetAttributes(EnsPTranscript transcript)
{
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
    
    transcript->Attributes = ajListNew();
    
    ensAttributeadaptorFetchAllByTranscript(dba,
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
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Database Entries
** @@
******************************************************************************/

/*
** FIXME: The Perl API documentation does not document the two
** additional arguments to this function.
*/

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
						0,
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




/* @func ensTranscriptGetTranslations *****************************************
**
** Get (weak references for) all Ensembl Translations of an Ensembl Transcript.
**
** Although an Ensembl Transcript has conceptually only one Translation, the
** Translation object may be cloned via ensTranslationNewObj, thereby retaining
** the Transcript it is based on, via a strong reference. Since each
** Translation has a strong reference for its Transcript, the corresponding
** Transcript needs to have weak references to each of its (cloned) Translation
** objects.
**
** Please note, since this is a List of weak references to Translation objects,
** the List will be empty if a Translation is not owned by any other object.
** Please use ensTranscriptFetchTranslation to own the Translation.
**
** @cc Bio::EnsEMBL::Transcript::???
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Translations or NULL
** @@
******************************************************************************/

const AjPList ensTranscriptGetTranslations(const EnsPTranscript transcript)
{
    if(!transcript)
	return NULL;
    
    return transcript->Translations;
}




/* @func ensTranscriptGetTranscriptCodingStart ********************************
**
** Get the start position of the coding region in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Transcript::cdna_coding_start
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Coding region start in Transcript coordinates
** @@
******************************************************************************/

ajuint ensTranscriptGetTranscriptCodingStart(EnsPTranscript transcript)
{
    AjIList iter = NULL;
    
    const AjPList exons = NULL;
    AjPList selist      = NULL;
    
    EnsPExon exon = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPSequenceEdit se = NULL;
    
    EnsPTranslation translation = NULL;
    
    if(!transcript)
	return 0;
    
    if(transcript->TranscriptCodingStart)
	return transcript->TranscriptCodingStart;
    
    translation = ensTranscriptFetchTranslation(transcript);
    
    if(translation)
    {
	/*
	** Calculate the coding start relative from the start of the
	** Translation in Transcript coordinates.
	*/
	
	exons = ensTranscriptGetExons(transcript);
	
	iter = ajListIterNewread(exons);
	
	while(!ajListIterDone(iter))
	{
	    exon = (EnsPExon) ajListIterGet(iter);
	    
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
	
	if(transcript->EditsEnabled)
	{
	    selist = ajListNew();
	    
	    ensTranscriptFetchAllSequenceEdits(transcript, selist);
	    
	    /*
	    ** Sort in reverse order to avoid adjustment of down-stream
	    ** Sequence Edits.
	    */
	    
	    ajListSort(selist, ensSequenceEditCompareStartDescending);
	    
	    while(ajListPop(selist, (void **) &se))
	    {
		if(ensSequenceEditGetStart(se) <
                   transcript->TranscriptCodingStart)
		    transcript->TranscriptCodingStart +=
			ensSequenceEditGetLengthDifference(se);
		
		ensSequenceEditDel(&se);
	    }
	    
	    ajListFree(&selist);
	}
    }
    
    ensTranslationDel(&translation);
    
    return transcript->TranscriptCodingStart;
}




/* @func ensTranscriptGetTranscriptCodingEnd **********************************
**
** Get the end position of the coding region in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Transcript::cdna_coding_end
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Coding region end in Transcript coordinates
** @@
******************************************************************************/

ajuint ensTranscriptGetTranscriptCodingEnd(EnsPTranscript transcript)
{
    AjIList iter = NULL;
    
    const AjPList exons = NULL;
    AjPList selist      = NULL;
    
    EnsPExon exon = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPSequenceEdit se = NULL;
    
    EnsPTranslation translation = NULL;
    
    if(!transcript)
	return 0;
    
    if(transcript->TranscriptCodingEnd)
	return transcript->TranscriptCodingEnd;
    
    translation = ensTranscriptFetchTranslation(transcript);
    
    if(translation)
    {
	/*
	** Calculate the coding start relative from the start of the
	** Translation in Transcript coordinates.
	*/
	
	exons = ensTranscriptGetExons(transcript);
	
	iter = ajListIterNewread(exons);
	
	while(!ajListIterDone(iter))
	{
	    exon = (EnsPExon) ajListIterGet(iter);
	    
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
	
	if(transcript->EditsEnabled)
	{
	    selist = ajListNew();
	    
	    ensTranscriptFetchAllSequenceEdits(transcript, selist);
	    
	    /*
            ** Sort in reverse order to avoid adjustment of down-stream
            ** Sequence Edits.
            */
	    
	    ajListSort(selist, ensSequenceEditCompareStartDescending);
	    
	    while(ajListPop(selist, (void **) &se))
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
	    
	    ajListFree(&selist);
	}
    }
    
    ensTranslationDel(&translation);
    
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
    
    translation = ensTranscriptFetchTranslation(transcript);
    
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
    
    ensTranslationDel(&translation);
    
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
    
    translation = ensTranscriptFetchTranslation(transcript);
    
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
    
    ensTranslationDel(&translation);
    
    return transcript->SliceCodingEnd;
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
    AjPList selist      = NULL;
    
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
    
    /* Adjust the length if post-transcriptional Sequence Edits are defined. */
    
    if(transcript->EditsEnabled)
    {
	selist = ajListNew();
	
	ensTranscriptFetchAllSequenceEdits(transcript, selist);
	
	while(ajListPop(selist, (void **) &se))
	{
	    length += ensSequenceEditGetLengthDifference(se);
	    
	    ensSequenceEditDel(&se);
	}
	
	ajListFree(&selist);
    }
    
    return length;
}




/* @func ensTranscriptGetMemSize **********************************************
**
** Get the memory size in bytes of an Ensembl Transcript.
**
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensTranscriptGetMemSize(const EnsPTranscript transcript)
{
    ajuint size = 0;
    
    AjIList iter = NULL;
    
    EnsPAttribute attribute = NULL;
    
    EnsPBasealignfeature baf = NULL;
    
    EnsPDatabaseentry dbe = NULL;
    
    EnsPExon exon = NULL;
    
    EnsPTranslation translation = NULL;
    
    if(!transcript)
	return 0;
    
    size += (ajuint) sizeof (EnsOTranscript);
    
    size += ensFeatureGetMemSize(transcript->Feature);
    
    size += ensDatabaseentryGetMemSize(transcript->DisplayReference);
    
    if(transcript->Description)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(transcript->Description);
    }
    
    if(transcript->BioType)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(transcript->BioType);
    }
    
    if(transcript->StableIdentifier)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(transcript->StableIdentifier);
    }
    
    if(transcript->CreationDate)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(transcript->CreationDate);
    }
    
    if(transcript->ModificationDate)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(transcript->ModificationDate);
    }
    
    /* Summarise the AJAX List of Ensembl Attributes. */
    
    if(transcript->Attributes)
    {
	size += (ajuint) sizeof (AjOList);
	
	iter = ajListIterNewread(transcript->Attributes);
	
	while(!ajListIterDone(iter))
	{
	    attribute = (EnsPAttribute) ajListIterGet(iter);
	    
	    size += ensAttributeGetMemSize(attribute);
	}
	
	ajListIterDel(&iter);
    }
    
    /* Summarise the AJAX List of Ensembl Database Entries. */
    
    if(transcript->DatabaseEntries)
    {
	size += (ajuint) sizeof (AjOList);
	
	iter = ajListIterNewread(transcript->DatabaseEntries);
	
	while(!ajListIterDone(iter))
	{
	    dbe = (EnsPDatabaseentry) ajListIterGet(iter);
	    
	    size += ensDatabaseentryGetMemSize(dbe);
	}
	
	ajListIterDel(&iter);
    }
    
    /* Summarise the AJAX List of Ensembl Exons. */
    
    if(transcript->Exons)
    {
	size += (ajuint) sizeof (AjOList);
	
	iter = ajListIterNewread(transcript->Exons);
	
	while(!ajListIterDone(iter))
	{
	    exon = (EnsPExon) ajListIterGet(iter);
	    
	    size += ensExonGetMemSize(exon);
	}
	
	ajListIterDel(&iter);
    }
    
    /* Summarise the AJAX List of supporting Ensembl Base Align Features. */
    
    if(transcript->Supportingfeatures)
    {
	size += (ajuint) sizeof (AjOList);
	
	iter = ajListIterNewread(transcript->Supportingfeatures);
	
	while(!ajListIterDone(iter))
	{
	    baf = (EnsPBasealignfeature) ajListIterGet(iter);
	    
	    size += ensBasealignfeatureGetMemSize(baf);
	}
	
	ajListIterDel(&iter);
    }
    
    /* Summarise the AJAX List of Ensembl Translations. */
    
    if(transcript->Translations)
    {
	size += (ajuint) sizeof (AjOList);
	
	iter = ajListIterNewread(transcript->Translations);
	
	while(!ajListIterDone(iter))
	{
	    translation = (EnsPTranslation) ajListIterGet(iter);
	    
	    size += ensTranslationGetMemSize(translation);
	}
	
	ajListIterDel(&iter);
    }
    
    size += ensMapperGetMemSize(transcript->ExonCoordMapper);
    
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
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetAdaptor(EnsPTranscript transcript,
                               EnsPTranscriptadaptor adaptor)
{
    if(!transcript)
        return ajFalse;
    
    transcript->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensTranscriptSetIdentifier *******************************************
**
** Set the SQL database-internal identifier element of an Ensembl Transcript.
**
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
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetFeature(EnsPTranscript transcript, EnsPFeature feature)
{
    AjIList iter = NULL;
    AjIList titer = NULL;
    
    EnsPBasealignfeature oldbaf = NULL;
    EnsPBasealignfeature newbaf = NULL;
    
    EnsPExon oldexon = NULL;
    EnsPExon newexon = NULL;
    
    EnsPSlice slice = NULL;
    
    EnsPTranslation translation = NULL;
    
#if AJFALSE
    ajDebug("ensTranscriptSetFeature\n"
            "  transcript %p\n"
            "  feature %p\n",
            transcript,
            feature);
    
    ensTranscriptTrace(transcript, 1);
    
    ensFeatureTrace(feature, 1);
#endif
    
    if(!transcript)
	return ajFalse;
    
    if(!feature)
	return ajFalse;
    
    /* Replace the current Feature. */
    
    if(transcript->Feature)
	ensFeatureDel(&(transcript->Feature));
    
    transcript->Feature = ensFeatureNewRef(feature);
    
    slice = ensFeatureGetSlice(transcript->Feature);
    
    /*
    ** Transfer Exons onto the new Feature Slice and thereby also adjust
    ** the start and end Exons of Translation(s) if they are defined.
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
	    
	    /*
	    ** Re-assign the start and end Exons of weakly linked
	    ** Translation(s).
	    */
	    
	    titer = ajListIterNew(transcript->Translations);
	    
	    while(!ajListIterDone(titer))
	    {
		translation = (EnsPTranslation) ajListIterGet(titer);
		
		if(oldexon == ensTranslationGetStartExon(translation))
		    ensTranslationSetStartExon(translation, newexon);
		
		if(oldexon == ensTranslationGetEndExon(translation))
		    ensTranslationSetEndExon(translation, newexon);
	    }
	    
	    ajListIterDel(&titer);
	    
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
    
    ensMapperDel(&(transcript->ExonCoordMapper));
    
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
	** that depend on the Transcript sequence.
	*/
	
	/*
	** FIXME: The Perl API deletes the Exon Sequence caches? Why?
	** A Sequence Edit on the Transcript level should have *no* influence
	** on the Exon sequence?
	*/
	
	transcript->SliceCodingStart = 0;
	transcript->SliceCodingEnd   = 0;
	
	transcript->TranscriptCodingStart = 0;
	transcript->TranscriptCodingEnd   = 0;
	
	ensMapperDel(&(transcript->ExonCoordMapper));
	
	/*
	** FIXME: Get all weakly referenced Translations and clear their
	** Sequence cache?
	*/
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




/* @func ensTranscriptLinkTranslation *****************************************
**
** Link a weak reference for an Ensembl Translation of an Ensembl Transcript.
**
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] translation [EnsPTranslation] Ensembl Translation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptLinkTranslation(EnsPTranscript transcript,
                                    EnsPTranslation translation)
{
    if(!transcript)
	return ajFalse;
    
    if(!translation)
	return ajFalse;
    
    /*
    ** Since the aim is to establish a weak reference, the reference counter
    ** is not incremented.
    */
    
    ajListPushAppend(transcript->Translations, (void *) translation);
    
    return ajTrue;
}




/* @func ensTranscriptUnlinkTranslation ***************************************
**
** Unlink a weak reference for an Ensembl Translation of an Ensembl Transcript.
**
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] translation [EnsPTranslation] Ensembl Translation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptUnlinkTranslation(EnsPTranscript transcript,
                                      EnsPTranslation translation)
{
    AjIList iter = NULL;
    
    EnsPTranslation temporary = NULL;
    
    if(!transcript)
	return ajFalse;
    
    if(!translation)
	return ajFalse;
    
    /*
    ** Since this is a weak reference, the reference counter
    ** is not decremented.
    */
    
    iter = ajListIterNew(transcript->Translations);
    
    while(!ajListIterDone(iter))
    {
	temporary = (EnsPTranslation) ajListIterGet(iter);
	
	if(temporary == translation)
	    ajListIterRemove(iter);
    }
    
    ajListIterDel(&iter);
    
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
	    "%S  Attributes %p\n"
	    "%S  DatabaseEntries %p\n"
	    "%S  Exons %p\n"
	    "%S  Supportingfeatures %p\n"
	    "%S  Translations %p\n"
	    "%S  SliceCodingStart %u\n"
	    "%S  SliceCodingEnd %u\n"
	    "%S  TranscriptCodingStart %u\n"
	    "%S  TranscriptCodingEnd %u\n"
	    "%S  EditsEnabled '%B'\n"
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
	    indent, transcript->Attributes,
	    indent, transcript->DatabaseEntries,
	    indent, transcript->Exons,
	    indent, transcript->Supportingfeatures,
	    indent, transcript->Translations,
	    indent, transcript->SliceCodingStart,
	    indent, transcript->SliceCodingEnd,
	    indent, transcript->TranscriptCodingStart,
	    indent, transcript->TranscriptCodingEnd,
	    indent, transcript->EditsEnabled,
	    indent, transcript->ExonCoordMapper,
	    indent, transcript->StartPhase);
    
    ensFeatureTrace(transcript->Feature, level + 1);
    
    ensDatabaseentryTrace(transcript->DisplayReference, level + 1);
    
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
    
    /* Trace the AJAX List of Ensembl Translations. */
    
    if(transcript->Translations)
    {
	ajDebug("%S    AJAX List %p of Ensembl Translations\n",
		indent, transcript->Translations);
	
	iter = ajListIterNewread(transcript->Translations);
	
	while(!ajListIterDone(iter))
	{
	    translation = (EnsPTranslation) ajListIterGet(iter);
	    
	    ensTranslationTrace(translation, level + 2);
	}
	
	ajListIterDel(&iter);
    }
    
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
** @return [AjEnum] Ensembl Transcript status element or
**                  ensETranscriptStatusNULL
** @@
******************************************************************************/

AjEnum ensTranscriptStatusFromStr(const AjPStr status)
{
    register ajint i = 0;
    
    AjEnum estatus = ensETranscriptStatusNULL;
    
    for(i = 1; transcriptStatus[i]; i++)
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
** @param [r] status [const AjEnum] Transcript status enumerator
**
** @return [const char*] Transcript status C-type (char*) string
** @@
******************************************************************************/

const char *ensTranscriptStatusToChar(const AjEnum status)
{
    register ajint i = 0;
    
    if(!status)
	return NULL;
    
    for(i = 1; transcriptStatus[i] && (i < status); i++);
    
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
	
	start =
	    (ensFeatureGetStart(efeature) < start) ?
	    ensFeatureGetStart(efeature) :
	    start;
	
	end =
	    (ensFeatureGetEnd(efeature) > end) ?
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
    
    /*
    ** FIXME: Should we re-calculate the following values already here?
    ** The Perl API keeps those separate although we would need a loop over
    ** all Exons and fetching of Sequence Edits for each of those values.
    */
    
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
    ajint lastnewend    = 0;
    ajint lastnewstrand = 0;
    ajint lastoldstrand = 0;
    ajint trstart = 0;
    ajint trend   = 0;
    
    ajuint pslength = 0;
    
    AjBool first       = AJFALSE;
    AjBool ignoreorder = AJFALSE;
    AjBool orderbroken = AJFALSE;
    AjBool error = AJFALSE;
    
    AjIList iter   = NULL;
    AjPList exons  = NULL;
    AjPList pslist = NULL;
    
    EnsPFeature newfeature = NULL;
    EnsPFeature oldfeature = NULL;
    EnsPFeature feature    = NULL;
    
    EnsPExon oldexon = NULL;
    EnsPExon newexon = NULL;
    
    EnsPProjectionsegment ps = NULL;
    
    const EnsPSeqregion newsr     = NULL;
    const EnsPSeqregion lastnewsr = NULL;
    
    EnsPTranscript newtranscript = NULL;
    
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
    
    if(transcript->Exons)
    {
	exons = ajListNew();
	
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
		    
		    /*
		    ** FIXME: The Perl API tests for equality of Sequence
		    ** Region names. This may however be too little, as there
		    ** are now clones and contigs with identical names, yet
		    ** different Coordinate Systems.
		    **
		    ** Here we test for identical Sequence Regions, which
		    ** includes a test for matching Coordinate Systems.
		    */
		    
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
		    
		    trstart =
			(ensFeatureGetStart(newfeature) < trstart) ?
			ensFeatureGetStart(newfeature) :
			trstart;
		    
		    trend =
			(ensFeatureGetEnd(newfeature) > trend) ?
			ensFeatureGetEnd(newfeature) :
			trend;
		}
		
		lastnewsr = ensFeatureGetSeqregion(newfeature);
		
		lastnewstart = ensFeatureGetStart(newfeature);
		
		lastnewend = ensFeatureGetEnd(newfeature);
		
		lastnewstrand = ensFeatureGetStrand(newfeature);
		
		lastoldstrand = ensFeatureGetStrand(oldfeature);
	    }
	    
	    /* Handle Translations. */
	    
	    /*
	    ** FIXME: In this implementation, Translations are only weak
	    ** references!
	    ** Not sure what to do here!
	    ** Translations are certainly not copied to the newly transformed
	    ** Transcript. As we don't have copies or strong references, it
	    ** is probably not neccessary to handle them at all???
	    */
	    
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
	ajListPeekNumber(exons, 0, (void **) &newexon);
	
	newfeature = ensExonGetFeature(newexon);
	
	feature =
	    ensFeatureNewS(ensFeatureGetAnalysis(transcript->Feature),
			   ensFeatureGetSlice(newfeature),
			   trstart,
			   trend,
			   ensFeatureGetStrand(newfeature));
    }
    
    newtranscript = ensTranscriptNewObj(transcript);
    
    /*
    ** FIXME: There is a problem here.
    ** ensTranscriptSetFeature transfers Exons, Supporting Features, etc onto
    ** the new Slice. We need to *transform* them here, but since
    ** ensFeatureTransfer calls ensFeatureTransform internally, in case the
    ** Coordinate Systems are not identical, this should be fine.
    ** The only problem is that we have done much of the work already above,
    ** but this is required to find out whether the Exons transform.
    */
    
    ensTranscriptSetFeature(newtranscript, feature);
    
    ensFeatureDel(&feature);
    
    while(ajListPop(exons, (void **) &newexon))
	ensExonDel(&newexon);
    
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
** Fetch all Ensembl Attributes of an Ensembl Transcript.
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Attributes
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [r] attributes [AjPList] AJAX List of Ensembl Attributes
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




/* @func ensTranscriptFetchAllDatabaseEntries *********************************
**
** Fetch all Ensembl Database Entries of an Ensembl Transcript.
** The caller is responsible for deleting the Ensembl Database Entries before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [r] type [AjEnum] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

/*
** FIXME: The Perl API documentation does not document the two additional
** arguments to this function.
*/

AjBool ensTranscriptFetchAllDatabaseEntries(EnsPTranscript transcript,
                                            const AjPStr name,
                                            AjEnum type,
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
	
	ajListPeekNumber(list, i, (void **) &exon2);
	
	intron = ensIntronNewExons(exon1, exon2);
	
	ajListPushAppend(introns, (void *) intron);
    }
    
    return ajTrue;
}




/* @func ensTranscriptFetchAllSequenceEdits ***********************************
**
** Fetch all Ensembl Sequence Edits of an Ensembl Transcript.
**
** The corresponding Ensembl Attribute codes have to be defined in the
** static const char *transcriptSequenceEditCode array.
** The caller is responsible for deleting the Ensembl Sequence Edits before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_SeqEdits
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] selist [AjPList] AJAX List of Ensembl Sequence Edits
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllSequenceEdits(EnsPTranscript transcript,
                                          AjPList selist)
{
    register ajuint i = 0;
    
    AjPList attributes = NULL;
    
    AjPStr code = NULL;
    
    EnsPAttribute at = NULL;
    
    EnsPSequenceEdit se = NULL;
    
    if(!transcript)
	return ajFalse;
    
    if(!selist)
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
	
	ajListPushAppend(selist, (void *) se);
	
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
                                           AjPStr* Pidentifier)
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
                                     AjPSeq* Psequence)
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
    
    ajStrDel(&name);
    ajStrDel(&sequence);
    
    return ajTrue;
}




/* @func ensTranscriptFetchSequenceStr ****************************************
**
** Fetch the spliced sequence of an Ensembl Transcript as AJAX String.
** The sequence of all Ensembl Exons is concatenated before
** post-transcriptional Sequence Edits are applied.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::spliced_seq
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceStr(EnsPTranscript transcript,
                                     AjPStr *Psequence)
{
    AjIList iter        = NULL;
    const AjPList exons = NULL;
    AjPList selist      = NULL;
    
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
    
    /*
    ** FIXME: There should be a sequence cache like in Exons and Translations,
    ** although that may take quite some memory. Some Transcripts are long ...
    */
    
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
    
    /* Apply post-transcriptional Sequence Edits. */
    
    if(transcript->EditsEnabled)
    {
	selist = ajListNew();
	
	ensTranscriptFetchAllSequenceEdits(transcript, selist);
	
	/*
	** Sort Sequence Edits in reverse order to avoid the complication of
	** adjusting down-stream Sequence Edits.
	*/
	
	ajListSort(selist, ensSequenceEditCompareStartDescending);
	
	while(ajListPop(selist, (void **) &se))
	{
	    ensSequenceEditApplyEdit(se, Psequence);
	    
	    ensSequenceEditDel(&se);
	}
	
	ajListFree(&selist);
    }
    
    return ajTrue;
}




/* @func ensTranscriptFetchTranslatableSequence *******************************
**
** Fetch the translatable sequence portion of an Ensembl Transcript as
** AJAX String. The sequence is padded with 'N's according to the start phase
** of the first coding Exon.
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
    ** The ensTranscriptFetchTranslation function will attempt to load the
    ** Translation from the database.
    */
    
    translation = ensTranscriptFetchTranslation(transcript);
    
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
    
    ensTranslationDel(&translation);
    
    return ajTrue;
}




/* @func ensTranscriptFetchTranslation ****************************************
**
** Fetch the Ensembl Translation of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch the
** Ensembl Translation from the Ensembl Core database in case it is
** not defined.
**
** This function will at most return one Translation, eventhough a Translation
** could have been cloned with ensTranslationNewObj. In case the complete
** AJAX List of weak references is required ensTranscriptGetTranslations
** should be used.
**
** The caller is responsible for deleting the ENsembl Translation.
**
** @cc Bio::EnsEMBL::Transcript::translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranslation] Ensembl Translation or NULL
** @@
******************************************************************************/

EnsPTranslation ensTranscriptFetchTranslation(EnsPTranscript transcript)
{
    ajuint length = 0;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPTranslation translation = NULL;
    EnsPTranslationadaptor ta   = NULL;
    
    if(!transcript)
	return NULL;
    
    length = ajListGetLength(transcript->Translations);
    
    /*
    ** Fetch the Translation from the database, in case the List of
    ** weak references is empty.
    */
    
    if(length == 0)
    {
	if(!transcript->Adaptor)
	{
	    ajDebug("ensTranscriptFetchTranslation cannot fetch an "
		    "Ensembl Translation for a Transcript without a "
		    "Transcript Adaptor.\n");
	    
	    return NULL;
	}
	
	dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);
	
	ta = ensRegistryGetTranslationadaptor(dba);
	
	ensTranslationadaptorFetchByTranscript(ta, transcript, &translation);
	
	return translation;
    }
    
    if(length > 1)
	ajWarn("ensTranscriptFetchTranslation returned the first out of %u "
	       "Translations.\n", length);
    
    ajListPeekNumber(transcript->Translations, 0, (void **) &translation);
    
    /*
    ** Since the List contains only weak references the reference counter of
    ** the Translation needs to be increased.
    */
    
    return ensTranslationNewRef(translation);
}




/* @func ensTranscriptTranslateStr ********************************************
**
** Translate an Ensembl Transcript and return the sequence as AJAX String.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::translate
** @param [r] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

/*
** FIXME: This should probably move to ensTranslationFetchSequenceStr.
*/

AjBool ensTranscriptTranslateStr(EnsPTranscript transcript, AjPStr* Psequence)
{
    ajuint codontable = 0;
    
    AjPList attributes = NULL;
    
    AjPStr cdna  = NULL;
    AjPStr code  = NULL;
    AjPStr value = NULL;
    
    AjPTrn atranslation = NULL;
    
    EnsPAttribute attribute = NULL;
    
    EnsPSlice slice = NULL;
    
    EnsPTranslation etranslation = NULL;
    
#if AJFALSE
    ajDebug("ensTranscriptTranslateStr\n"
            "  transcript %p\n"
            "  Psequence %p\n",
            transcript,
            Psequence);
#endif
    
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
    ** The ensTranscriptFetchTranslation function will attempt to load the
    ** Translation from the database.
    */
    
    etranslation = ensTranscriptFetchTranslation(transcript);
    
    if(!etranslation)
    {
	ajDebug("ensTranscriptTranslateStr got a Transcript without "
		"a Translation.\n");
	
	return ajTrue;
    }
    
    ensTranscriptFetchTranslatableSequence(transcript, &cdna);
    
    if(ajStrGetLen(cdna) < 1)
	return ajTrue;
    
    slice = ensFeatureGetSlice(transcript->Feature);
    
    code = ajStrNewC("codon_table");
    
    ensSliceFetchAllAttributes(slice, code, attributes);
    
    ajStrDel(&code);
    
    while(ajListPop(attributes, (void **) &attribute))
    {
	value = ensAttributeGetValue(attribute);
	
	ajStrToUint(value, &codontable);
	
	ensAttributeDel(&attribute);
    }
    
    ajListFree(&attributes);
    
    atranslation = ensTranslationCacheGetTranslation(codontable);
    
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
    */
    
    if (ajStrGetCharLast(*Psequence) == '*')
	ajStrCutEnd(Psequence, 1);
    
    if(transcript->EditsEnabled)
	ensTranslationModify(etranslation, Psequence);
    
    ensTranslationDel(&etranslation);
    
    return ajTrue;
}




/* @datasection [EnsPTranscriptadaptor] Transcript Adaptor ********************
**
** Functions for manipulating Ensembl Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor CVS Revision: 1.82
**
** @nam2rule Transcriptadaptor
**
******************************************************************************/

static const char *transcriptAdaptorTables[] =
{
    "transcript",
    "transcript_stable_id",
    "xref",
    "external_db",
    NULL
};




static const char *transcriptAdaptorColumns[] =
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




static EnsOBaseadaptorLeftJoin transcriptAdaptorLeftJoin[] =
{
    {
	"transcript_stable_id",
	"transcript_stable_id.transcript_id = transcript.transcript_id"
    },
    {"xref", "xref.xref_id = transcript.display_xref_id"},
    {"external_db", "external_db.external_db_id = xref.external_db_id"},
    {NULL, NULL}
};




static const char *transcriptAdaptorDefaultCondition = NULL;




static const char *transcriptAdaptorFinalCondition = NULL;




/* @funcstatic transcriptAdaptorFetchAllBySQL *********************************
**
** Fetch all Ensembl Transcript objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool transcriptAdaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
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
    
    AjEnum estatus   = ensETranscriptStatusNULL;
    AjEnum einfotype = ensEExternalreferenceInfoTypeNULL;
    
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
    EnsPTranscriptadaptor ta  = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
#if AJFALSE
    ajDebug("transcriptAdaptorFetchAllBySQL\n"
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
#endif
    
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
    
    ta = ensRegistryGetTranscriptadaptor(dba);
    
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
	
	estatus = ensETranscriptStatusNULL;
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
	/* FIXME: The gene_id does not seem to be used by the Perl API!!! */
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
	    ajFatal("transcriptAdaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("transcriptAdaptorFetchAllBySQL got a "
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
	
	if((!am) &&
           slice &&
           (!ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                                 ensSliceGetCoordsystem(srslice))))
	    am =
		ensAssemblymapperadaptorFetchByCoordsystems(
                    ama,
                    ensSliceGetCoordsystem(slice),
                    ensSliceGetCoordsystem(srslice));
	
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
	    ** FIXME: In contrast to the Bio::EnsEMBL::DBSQL::ExonAdaptor
	    ** code, a construct to get a Slice from the cache is not
	    ** commented out from the Bio::EnsEMBL::DBSQL::TranscriptAdaptor
	    ** of the Perl API.
	    ** See CVS versions 1.68 and 1.68.2.1 for details.
	    */
	    
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
		ajFatal("transcriptAdaptorFetchAllBySQL got a Slice, "
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
	    
	    /* FIXME: The homo_sapiens_core_51_36m database has NULL here.
               if(!einfotype)
               ajFatal("transcriptAdaptorFetchAllBySQL encountered "
               "unexpected string '%S' in the "
               "'xref.infotype' field.\n", erinfotype);
	    */
	    
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
	    ajFatal("transcriptAdaptorFetchAllBySQL encountered "
		    "unexpected string '%S' in the "
		    "'transcript.status' field.\n", status);
	
	/* Finally, create a new Ensembl Transcript. */
	
	feature = ensFeatureNewS(analysis,
				 srslice,
				 slstart,
				 slend,
				 slstrand);
	
	transcript = ensTranscriptNew(ta,
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
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @funcstatic transcriptAdaptorCacheReference ********************************
**
** Wrapper function to reference an Ensembl Transcript
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Transcript
**
** @return [void *] Ensembl Transcript or NULL
** @@
******************************************************************************/

static void *transcriptAdaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensTranscriptNewRef((EnsPTranscript) value);
}




/* @funcstatic transcriptAdaptorCacheDelete ***********************************
**
** Wrapper function to delete an Ensembl Transcript
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Transcript address
**
** @return [void]
** @@
******************************************************************************/

static void transcriptAdaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensTranscriptDel((EnsPTranscript *) value);
    
    return;
}




/* @funcstatic transcriptAdaptorCacheSize *************************************
**
** Wrapper function to determine the memory size of an Ensembl Transcript
** via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Transcript
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

static ajuint transcriptAdaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensTranscriptGetMemSize((const EnsPTranscript) value);
}




/* @funcstatic transcriptAdaptorGetFeature ************************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Transcript from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Transcript
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature transcriptAdaptorGetFeature(const void *value)
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
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule Ref object [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @valrule * [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensTranscriptadaptorNew **********************************************
**
** Default Ensembl Transcript Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPTranscriptadaptor adaptor = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(adaptor);
    
    adaptor->Adaptor = ensFeatureadaptorNew(
        dba,
        transcriptAdaptorTables,
        transcriptAdaptorColumns,
        transcriptAdaptorLeftJoin,
        transcriptAdaptorDefaultCondition,
        transcriptAdaptorFinalCondition,
        transcriptAdaptorFetchAllBySQL,
        (void* (*)(const void* key)) NULL, /* Fread */
        transcriptAdaptorCacheReference,
        (AjBool (*)(const void* value)) NULL, /* Fwrite */
        transcriptAdaptorCacheDelete,
        transcriptAdaptorCacheSize,
        transcriptAdaptorGetFeature,
        "Transcript");
    
    return adaptor;
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
** @argrule * Padaptor [EnsPTranscriptadaptor*] Ensembl Transcript Adaptor
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTranscriptadaptorDel **********************************************
**
** Default destructor for an Ensembl Transcript Adaptor.
**
** @param [d] Padaptor [EnsPTranscriptadaptor*] Ensembl Transcript Adaptor
**                                              address
**
** @return [void]
** @@
******************************************************************************/

void ensTranscriptadaptorDel(EnsPTranscriptadaptor *Padaptor)
{
    EnsPTranscriptadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    ensFeatureadaptorDel(&pthis->Adaptor);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @func ensTranscriptadaptorGetFeatureadaptor ********************************
**
** Get the Ensembl Feature Adaptor element of an Ensembl Transcript Adaptor.
**
** @param [r] adaptor [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

EnsPFeatureadaptor ensTranscriptadaptorGetFeatureadaptor(
    const EnsPTranscriptadaptor adaptor)
{
    if(!adaptor)
	return NULL;
    
    return adaptor->Adaptor;
}




/* @func ensTranscriptadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Transcript Adaptor.
**
** @param [r] adaptor [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensTranscriptadaptorGetDatabaseadaptor(
    const EnsPTranscriptadaptor adaptor)
{
    if(!adaptor)
	return NULL;
    
    return ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
}




/* @func ensTranscriptadaptorFetchAll *****************************************
**
** Fetch all Ensembl Transcripts.
**
** @param [r] adaptor [const EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAll(EnsPTranscriptadaptor adaptor,
                                    AjPList transcripts)
{
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!transcripts)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    return ensBaseadaptorFetchAll(ba, transcripts);
}




/* @func ensTranscriptadaptorFetchAllByGene ***********************************
**
** Fetch all Ensembl Transcripts via an Ensembl Gene.
**
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] gene [EnsPGene] Ensembl Gene
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllByGene(EnsPTranscriptadaptor adaptor,
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
    
    if(!adaptor)
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
    
    dba = ensTranscriptadaptorGetDatabaseadaptor(adaptor);
    
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
    
    constraint =
	ajFmtStr("transcript.gene_id = %u", ensGeneGetIdentifier(gene));
    
    list = ajListNew();
    
    ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
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




/* @func ensTranscriptadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl Transcript via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByIdentifier(EnsPTranscriptadaptor adaptor,
                                             ajuint identifier,
                                             EnsPTranscript *Ptranscript)
{
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Ptranscript)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
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
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByStableIdentifier(
    EnsPTranscriptadaptor adaptor,
    const AjPStr stableid,
    ajuint version,
    EnsPTranscript *Ptranscript)
{
    char *txtstableid = NULL;
    
    AjPList transcripts = NULL;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    EnsPTranscript transcript = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!stableid)
	return ajFalse;
    
    if(!Ptranscript)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
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




/* @func ensTranscriptadaptorFetchByTranslationIdentifier *********************
**
** Fetch an Ensembl Transcript via its Ensembl Translation identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] Ensembl Translation identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationIdentifier(
    EnsPTranscriptadaptor adaptor,
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
    
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Ptranscript)
	return ajFalse;
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
    
    statement = ajFmtStr("SELECT "
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
    
    ajSqlstatementDel(&sqls);
    
    ajStrDel(&statement);
    
    if(!trid)
    {
	ajDebug("ensTranscriptadaptorFetchByTranslationIdentifier could not "
		"get Transcript identifier for Translation identifier %u.\n",
		identifier);
	
	return ajFalse;
    }
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    *Ptranscript = (EnsPTranscript) ensBaseadaptorFetchByIdentifier(ba, trid);
    
    /*
    ** FIXME: Calling the function above directly is a short-cut.
     
    value = ensTranscriptadaptorFetchByIdentifier(ta, trid, Ptranscript);
    */
    
    return ajTrue;
}




/* @func ensTranscriptadaptorFetchByTranslationStableIdentifier ***************
**
** Fetch an Ensembl Transcript via its Ensembl Translation stable identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @param [r] adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Ensembl Translation stable identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationStableIdentifier(
    EnsPTranscriptadaptor adaptor,
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
    
    if(!adaptor)
	return ajFalse;
    
    if(!stableid)
	return ajFalse;
    
    if(!Ptranscript)
	return ajFalse;
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
    
    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);
    
    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id "
        "FROM "
        "translation_stable_id, "
        "transcript, "
        "translation "
        "WHERE "
        "translation_stable_id.stable_id = '%s' "
        "AND "
        "translation.translation_id = "
        "translation_stable_id.translation_id "
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
    
    ajSqlstatementDel(&sqls);
    
    ajStrDel(&statement);
    
    if(!trid)
    {
	ajDebug("ensTranscriptadaptorFetchByTranslationStableIdentifier "
		"could not get Transcript identifier for Translation "
		"stable identifier '%S'.\n", stableid);
	
	return ajFalse;
    }
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    *Ptranscript = (EnsPTranscript) ensBaseadaptorFetchByIdentifier(ba, trid);
    
    /*
    ** FIXME: Calling the function above directly is a short-cut.
     
    value = ensTranscriptadaptorFetchByIdentifier(ta, trid, Ptranscript);
    */
    
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
** Note that the Transcript Mapper uses the Transcript state at the
** time of initialisation to perform the conversions and that the
** Transcript Mapper must be re-initialised if the Transcript is altered.
** 'Genomic' coordinates are coordinates which are relative to the
** Slice that the Transcript is on.
******************************************************************************/

AjBool ensTranscriptMapperInit(EnsPTranscript transcript)
{
    ajuint srid = 0;
    
    /* Current and previous Feature (Exon) genomic coordinates */
    
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
    AjPList selist      = NULL;
    
    AjPStr src = NULL;
    AjPStr trg = NULL;
    
    EnsPCoordsystem cs = NULL;
    
    EnsPExon exon = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPSequenceEdit se = NULL;
    
    EnsPSlice slice = NULL;
    
#if AJFALSE
    ajDebug("ensTranscriptMapperInit\n"
            "  transcript %p\n",
            transcript);
#endif
    
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
    
    selist = ajListNew();
    
    if(transcript->EditsEnabled)
    {
	ensTranscriptFetchAllSequenceEdits(transcript, selist);
	
	ajListSort(selist, ensSequenceEditCompareStartAscending);
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
	** Add deletions and insertions into pairs wnen Sequence Edits are
	** turned on ignore mismatches (i.e. treat as mismatches)
	*/
	
	if(transcript->EditsEnabled)
	{
	    while(ajListPop(selist, (void **) &se) &&
                  (ensSequenceEditGetStart(se) + editshift <= curtrcend))
	    {
		if(ensSequenceEditGetLengthDifference(se))
		{
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
		    
		    if(prvtrclength > 0)
		    {
			/*
			** Only create a Mapper Pair if this is not a
			** boundary case.
			*/
			
			/*
			** FIXME: ensMapperAddCoordinates requires
			** Source and Target Sequence Region identifiers.
			** Currently, source is set as transcript->Identifier
		        ** and the target as the Sequence Region identifier
			** of the Slice on which this transcript is annotated.
			*/
			
			ensMapperAddCoordinates(transcript->ExonCoordMapper,
						transcript->Identifier,
						prvtrcstart,
						prvtrcend,
						curftrstrand,
						srid,
						prvftrstart,
						prvftrend);
		    }
		    
		    curtrcstart = prvtrcend + 1;
		    
		    if(curftrstrand >= 0)
			curftrstart = prvftrend + 1;
		    else
			curftrend = prvftrstart - 1;
		    
		    curtrcend += ensSequenceEditGetLengthDifference(se);
		    
		    if(ensSequenceEditGetLengthDifference(se))
		    {
			/* Insert in cDNA, shift coordinates along. */
			
			curtrcstart += ensSequenceEditGetLengthDifference(se);
		    }
		    else
		    {
			/*
			** Delete in cDNA (insert in genomic),
			** shift Slice coordinates along.
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
		
		ensSequenceEditDel(&se);
	    }
	}
	
	/*
	** FIXME: ensMapperAddCoordinates requires
	** Source and Target Sequence Region identifiers.
	** Currently, source is set as transcript->Identifier
	** and the target as the Sequence Region identifier
	** of the Slice on which this transcript is annotated.
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
    
    while(ajListPop(selist, (void **) &se))
	ensSequenceEditDel(&se);
    
    ajListFree(&selist);
    
    return ajTrue;
}




/* @func ensTranscriptMapperTranscript2Slice **********************************
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

AjBool ensTranscriptMapperTranscript2Slice(EnsPTranscript transcript,
                                           ajuint start,
                                           ajuint end,
                                           AjPList mrs)
{
    AjPStr src = NULL;
    
    if(!transcript)
	return ajFalse;
    
    if(!start)
    {
	ajDebug("ensTranscriptMapperTranscript2Slice "
		"requires a start coordinate.\n");
	
	return ajFalse;
    }
    
    if(!end)
    {
	ajDebug("ensTranscriptMapperTranscript2Slice "
		"requires an end coordinate.\n");
	
	return ajFalse;
    }
    
    if(!mrs)
    {
	ajDebug("ensTranscriptMapperTranscript2Slice "
		"requires an AJAX List.\n");
	
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




/* @func ensTranscriptMapperSlice2Transcript **********************************
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

AjBool ensTranscriptMapperSlice2Transcript(EnsPTranscript transcript,
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
	ajDebug("ensTranscriptMapperSlice2Transcript "
		"requires a start coordinate.\n");
	
	return ajFalse;
    }
    
    if(!end)
    {
	ajDebug("ensTranscriptMapperSlice2Transcript "
		"requires an end coordinate.\n");
	
	return ajFalse;
    }
    
    if(!strand)
    {
	ajDebug("ensTranscriptMapperSlice2Transcript "
		"requires strand information.\n");
	
	return ajFalse;
    }
    
    if(!mrs)
    {
	ajDebug("ensTranscriptMapperSlice2Transcript "
		"requires an AJAX List.\n");
	
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




/* @func ensTranscriptMapperTranslation2Slice *********************************
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

AjBool ensTranscriptMapperTranslation2Slice(EnsPTranscript transcript,
                                            ajuint start,
                                            ajuint end,
                                            AjPList mrs)
{
    if(!transcript)
	return ajFalse;
    
    if(!start)
    {
	ajDebug("ensTranscriptMapperTranslation2Slice "
		"requires a start coordinate.\n");
	
	return ajFalse;
    }
    
    if(!end)
    {
	ajDebug("ensTranscriptMapperTranslation2Slice "
		"requires an end coordinate.\n");
	
	return ajFalse;
    }
    
    if(!mrs)
    {
	ajDebug("ensTranscriptMapperTranslation2Slice "
		"requires an AJAX List.\n");
	
	return ajFalse;
    }
    
    /* Move start and end into cDNA coordinates. */
    
    start = 3 * start - 2 + (transcript->TranscriptCodingStart - 1);
    
    end = 3 * end + (transcript->TranscriptCodingStart - 1);
    
    return ensTranscriptMapperTranscript2Slice(transcript, start, end, mrs);
}




/* @func ensTranscriptMapperSlice2CDS *****************************************
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

AjBool ensTranscriptMapperSlice2CDS(EnsPTranscript transcript,
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
	ajDebug("ensTranscriptMapperSlice2CDS "
		"requires a start coordinate.\n");
	
	return ajFalse;
    }
    
    if(!end)
    {
	ajDebug("ensTranscriptMapperSlice2CDS "
		"requires an end coordinate.\n");
	
	return ajFalse;
    }
    
    if(!strand)
    {
	ajDebug("ensTranscriptMapperSlice2CDS "
		"requires strand information.\n");
	
	return ajFalse;
    }
    
    if(!mrs)
    {
	ajDebug("ensTranscriptMapperSlice2CDS "
		"requires an AJAX List.\n");
	
	return ajFalse;
    }
    
    if(start > (end + 1))
	ajFatal("ensTranscriptMapperSlice2CDS requires start %u to be "
		"less than end %u + 1.\n", start, end);
    
    /*
    ** For non-cooding Transcripts return a Mapper Result Gap as there is no
    ** coding sequence.
    */
    
    if(!transcript->TranscriptCodingStart)
    {
	mr = MENSMAPPERGAPNEW(start, end);
	
	ajListPushAppend(mrs, (void *) mr);
	
	return ajTrue;
    }
    
    result = ajListNew();
    
    ensTranscriptMapperSlice2Transcript(transcript,
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
		
		mr = MENSMAPPERGAPNEW(start, end);
		
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
                        transcript->TranscriptCodingStart - 1);
		    
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
                        end);
		    
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
                    ensMapperresultGetCoordsystem(gcmr));
		
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




/* @func ensTranscriptMapperSlice2Translation *********************************
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

AjBool ensTranscriptMapperSlice2Translation(EnsPTranscript transcript,
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
	ajDebug("ensTranscriptMapperSlice2Translation "
		"requires a start coordinate.\n");
	
	return ajFalse;
    }
    
    if(!end)
    {
	ajDebug("ensTranscriptMapperSlice2Translation "
		"requires an end coordinate.\n");
	
	return ajFalse;
    }
    
    if(!strand)
    {
	ajDebug("ensTranscriptMapperSlice2Translation "
		"requires strand information.\n");
	
	return ajFalse;
    }
    
    if(!mrs)
    {
	ajDebug("ensTranscriptMapperSlice2Translation "
		"requires an AJAX List.\n");
	
	return ajFalse;
    }
    
    result = ajListNew();
    
    ensTranscriptMapperSlice2CDS(transcript, start, end, strand, result);
    
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
                ensMapperresultGetCoordsystem(gcmr));
	    
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
** @param [r] exon [EnsPTranscript] Ensembl Transcript
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
    
    statement = ajFmtStr("SELECT "
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
    
    ajSqlstatementDel(&sqls);
    
    ajStrDel(&statement);
    
    return ajTrue;
}

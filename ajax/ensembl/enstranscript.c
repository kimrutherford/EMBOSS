/* @source Ensembl Transcript functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:29 $ by $Author: mks $
** @version $Revision: 1.57 $
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

#include "ensalign.h"
#include "ensattribute.h"
#include "ensdatabaseentry.h"
#include "ensexon.h"
#include "ensgene.h"
#include "ensintron.h"
#include "ensprojectionsegment.h"
#include "enssequenceedit.h"
#include "enstable.h"
#include "enstranscript.h"
#include "enstranslation.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* @datastatic TranscriptPExonRank ********************************************
**
** Ensembl Transcript, Exon and rank associations.
**
** Holds associations between Ensembl Transcript objects, Ensembl Exon objects
** and their rank in the Ensembl Transcript.
**
** @alias TranscriptSExonRank
** @alias TranscriptOExonRank
**
** @attr Transcriptidentifier [ajuint] Ensembl Transcript identifier
** @attr Rank [ajint] Ensembl Exon rank
** @@
******************************************************************************/

typedef struct TranscriptSExonRank
{
    ajuint Transcriptidentifier;
    ajint Rank;
} TranscriptOExonRank;

#define TranscriptPExonRank TranscriptOExonRank*




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic transcriptSequenceeditCode ************************************
**
** Ensembl Sequence Edit objects for Ensembl Transcript objects are a sub-set
** of Ensembl Attribute objects that provide information about
** post-transcriptional modifications of the Transcript sequence.
** Attribute objects with the following codes are Sequence Edit objects
** on the Transcript-level.
**
** _rna_edit: General cDNA, RNA or Transcript sequence edit
**
******************************************************************************/

static const char* const transcriptSequenceeditCode[] =
{
    "_rna_edit",
    (const char*) NULL
};




/* @conststatic transcriptStatus **********************************************
**
** The Ensembl Transcript status element is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsETranscriptStatus
** and the 'transcript.status' field.
**
******************************************************************************/

static const char* const transcriptStatus[] =
{
    "",
    "KNOWN",
    "NOVEL",
    "PUTATIVE",
    "PREDICTED",
    "KNOWN_BY_PROJECTION",
    "UNKNOWN",
    (const char*) NULL
};




/* @conststatic transcriptadaptorTables ***************************************
**
** Array of Ensembl Transcript Adaptor SQL table names
**
******************************************************************************/

static const char* const transcriptadaptorTables[] =
{
    "transcript",
    "transcript_stable_id",
    "xref",
    "external_db",
    (const char*) NULL
};




/* @conststatic transcriptadaptorColumns **************************************
**
** Array of Ensembl Transcript Adaptor SQL column names
**
******************************************************************************/

static const char* const transcriptadaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic transcriptadaptorLeftjoin *************************************
**
** Array of Ensembl Transcript Adaptor SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin transcriptadaptorLeftjoin[] =
{
    {
        "transcript_stable_id",
        "transcript_stable_id.transcript_id = transcript.transcript_id"
    },
    {"xref", "xref.xref_id = transcript.display_xref_id"},
    {"external_db", "external_db.external_db_id = xref.external_db_id"},
    {(const char*) NULL, (const char*) NULL}
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static TranscriptPExonRank transcriptExonRankNew(ajuint trid, ajint rank);

static void transcriptExonRankDel(TranscriptPExonRank *Ptrex);

static EnsPTranscript transcriptNewCpyFeatures(EnsPTranscript transcript);

static int listTranscriptCompareStartAscending(const void* P1,
                                               const void* P2);

static int listTranscriptCompareStartDescending(const void* P1,
                                                const void* P2);

static void tableTranscriptClear(void** key,
                                 void** value,
                                 void* cl);

static AjBool transcriptadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList transcripts);

static void* transcriptadaptorCacheReference(void* value);

static void transcriptadaptorCacheDelete(void** value);

static size_t transcriptadaptorCacheSize(const void* value);

static EnsPFeature transcriptadaptorGetFeature(const void* value);

static void transcriptadaptorClearTranscriptExonRankList(void** key,
                                                         void** value,
                                                         void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection enstranscript *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @funcstatic transcriptExonRankNew ******************************************
**
** Default constructor for a Transcript Exon Rank object.
**
** @param [r] trid [ajuint] Ensembl Transcript identifier
** @param [r] rank [ajint] Ensembl Exon rank
**
** @return [TranscriptPExonRank] Transcript Exon Rank object
** @@
******************************************************************************/

static TranscriptPExonRank transcriptExonRankNew(ajuint trid, ajint rank)
{
    TranscriptPExonRank trex = NULL;

    AJNEW0(trex);

    trex->Transcriptidentifier = trid;
    trex->Rank = rank;

    return trex;
}




/* @funcstatic transcriptExonRankDel ******************************************
**
** Default destructor for a Transcript Exon Rank object.
**
** @param [d] Ptrex [TranscriptPExonRank*] Transcript Exon Rank object address
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




/* @datasection [EnsPTranscript] Ensembl Transcript ***************************
**
** @nam2rule Transcript Functions for manipulating Ensembl Transcript objects
**
** @cc Bio::EnsEMBL::Transcript
** @cc CVS Revision: 1.317
** @cc CVS Tag: branch-ensembl-62
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy transcript [const EnsPTranscript] Ensembl Transcript
** @argrule Ini tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini displaydbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Ini description [AjPStr] Description
** @argrule Ini biotype [AjPStr] Biotype
** @argrule Ini status [EnsETranscriptStatus] Status
** @argrule Ini current [AjBool] Current attribute
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Version
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ini exons [AjPList] AJAX List of Ensembl Exon objects
** @argrule Ref transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule * [EnsPTranscript] Ensembl Transcript
**
** @fcategory new
******************************************************************************/




/* @func ensTranscriptNewCpy **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNewCpy(const EnsPTranscript transcript)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPTranscript pthis = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = transcript->Identifier;

    pthis->Adaptor = transcript->Adaptor;

    pthis->Feature = ensFeatureNewRef(transcript->Feature);

    pthis->Displayreference = ensDatabaseentryNewRef(
        transcript->Displayreference);

    if(transcript->Description)
        pthis->Description = ajStrNewRef(transcript->Description);

    if(transcript->Biotype)
        pthis->Biotype = ajStrNewRef(transcript->Biotype);

    pthis->Status = transcript->Status;

    pthis->Current = transcript->Current;

    if(transcript->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(transcript->Stableidentifier);

    if(transcript->DateCreation)
        pthis->DateCreation = ajStrNewRef(transcript->DateCreation);

    if(transcript->DateModification)
        pthis->DateModification = ajStrNewRef(transcript->DateModification);

    pthis->Version = transcript->Version;

    pthis->Geneidentifier = transcript->Geneidentifier;

    /* NOTE: Copy the AJAX List of alternative Ensembl Translation objects. */

    if(transcript->Alternativetranslations
       && ajListGetLength(transcript->Alternativetranslations))
    {
        pthis->Alternativetranslations = ajListNew();

        iter = ajListIterNew(transcript->Alternativetranslations);

        while(!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            ajListPushAppend(pthis->Alternativetranslations,
                             (void*) ensTranslationNewRef(translation));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Alternativetranslations = NULL;

    /* Copy the AJAX List of Ensembl Attributes. */

    if(transcript->Attributes && ajListGetLength(transcript->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(transcript->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void*) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Database Entry objects. */

    if(transcript->Databaseentries
       && ajListGetLength(transcript->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(transcript->Databaseentries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ajListPushAppend(pthis->Databaseentries,
                             (void*) ensDatabaseentryNewRef(dbe));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Databaseentries = NULL;

    /* Copy the AJAX List of Ensembl Exon objects. */

    if(transcript->Exons && ajListGetLength(transcript->Exons))
    {
        pthis->Exons = ajListNew();

        iter = ajListIterNew(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ajListPushAppend(pthis->Exons, (void*) ensExonNewRef(exon));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Exons = NULL;

    /*
    ** NOTE: Copy the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    if(transcript->Supportingfeatures &&
       ajListGetLength(transcript->Supportingfeatures))
    {
        pthis->Supportingfeatures = ajListNew();

        iter = ajListIterNew(transcript->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListPushAppend(pthis->Supportingfeatures,
                             (void*) ensBasealignfeatureNewRef(baf));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Supportingfeatures = NULL;

    pthis->Translation = ensTranslationNewRef(transcript->Translation);

    pthis->Sequenceedits = transcript->Sequenceedits;

    /*
    ** NOTE: The ExonCoordMapper is not copied here, as the new Transcript
    ** object is most likely placed onto a new Slice, which required
    ** subsequent deletion of the Ensembl Mapper.
    */

    pthis->ExonCoordMapper = NULL;

    return pthis;
}




/* @func ensTranscriptNewIni **************************************************
**
** Constructor for an Ensembl Transcript with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Transcript::new
** @param [u] displaydbe [EnsPDatabaseentry] Ensembl Database Entry
** @param [u] description [AjPStr] Description
** @param [u] biotype [AjPStr] Biotype
** @param [u] status [EnsETranscriptStatus] Status
** @param [r] current [AjBool] Current attribute
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNewIni(EnsPTranscriptadaptor tca,
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

    transcript->Displayreference = ensDatabaseentryNewRef(displaydbe);

    if(description)
        transcript->Description = ajStrNewRef(description);

    if(biotype)
        transcript->Biotype = ajStrNewRef(biotype);

    transcript->Status = status;

    transcript->Current = current;

    if(stableid)
        transcript->Stableidentifier = ajStrNewRef(stableid);

    if(cdate)
        transcript->DateCreation = ajStrNewRef(cdate);

    if(mdate)
        transcript->DateModification = ajStrNewRef(mdate);

    transcript->Version = version;

    transcript->Geneidentifier = 0;

    transcript->Alternativetranslations = NULL;

    transcript->Attributes = NULL;

    transcript->Databaseentries = NULL;

    if(exons && ajListGetLength(exons))
    {
        transcript->Exons = ajListNew();

        iter = ajListIterNew(exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ajListPushAppend(transcript->Exons,
                             (void*) ensExonNewRef(exon));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->Exons = NULL;

    transcript->Supportingfeatures = NULL;

    transcript->Translation = NULL;

    /*
    ** NOTE: Sequence Edit objects such as transcription and translation
    ** exceptions are applied by default. Use ensTranscriptGetSequenceedits and
    ** ensTranscriptSetSequenceedits to toggle Sequence Edit application.
    */

    transcript->Sequenceedits = ajTrue;

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




/* @funcstatic transcriptNewCpyFeatures ***************************************
**
** Returns a new copy of an Ensembl Transcript, but in addition to the shallow
** copy provided by ensTranscriptNewCpy, also copies all Ensembl Transcript-
** internal Ensembl Objects based on the Ensembl Feature class. This is useful
** in preparation of ensTranscriptTransform and ensTranscriptTransfer, which
** return an independent Ensembl Transcript object and therefore, require
** independent mapping of all internal Feature objects to the new
** Ensembl Coordinate System or Ensembl Slice.
**
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
** @@
******************************************************************************/

static EnsPTranscript transcriptNewCpyFeatures(EnsPTranscript transcript)
{
    AjIList iter = NULL;

    EnsPBasealignfeature newbaf = NULL;
    EnsPBasealignfeature oldbaf = NULL;

    EnsPExon newexon = NULL;
    EnsPExon oldexon = NULL;

    EnsPTranscript newtranscript = NULL;

    EnsPTranslation newtranslation = NULL;
    EnsPTranslation oldtranslation = NULL;

    if(!transcript)
        return NULL;

    newtranscript = ensTranscriptNewCpy(transcript);

    if(!newtranscript)
        return NULL;

    /* Copy the AJAX List of alternative Ensembl Translation objects. */

    if(newtranscript->Alternativetranslations)
    {
        iter = ajListIterNew(newtranscript->Alternativetranslations);

        while(!ajListIterDone(iter))
        {
            oldtranslation = (EnsPTranslation) ajListIterGet(iter);

            ajListIterRemove(iter);

            newtranslation = ensTranslationNewCpy(oldtranslation);

            ajListIterInsert(iter, (void*) newtranslation);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensTranslationDel(&oldtranslation);
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of Ensembl Exon objects. */

    if(newtranscript->Exons)
    {
        iter = ajListIterNew(newtranscript->Exons);

        while(!ajListIterDone(iter))
        {
            oldexon = (EnsPExon) ajListIterGet(iter);

            ajListIterRemove(iter);

            newexon = ensExonNewCpy(oldexon);

            ajListIterInsert(iter, (void*) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensExonDel(&oldexon);
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of supporting Ensembl Base Align Feature objects. */

    if(newtranscript->Supportingfeatures)
    {
        iter = ajListIterNew(newtranscript->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListIterRemove(iter);

            newbaf = ensBasealignfeatureNewCpy(oldbaf);

            ajListIterInsert(iter, (void*) newbaf);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensBasealignfeatureDel(&oldbaf);
        }

        ajListIterDel(&iter);
    }

    /* Copy the canonical Ensembl Translation. */

    newtranslation = ensTranslationNewCpy(newtranscript->Translation);

    ensTranslationDel(&newtranscript->Translation);

    newtranscript->Translation = newtranslation;

    return newtranscript;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Del Destroy (free) an Ensembl Transcript object
**
** @argrule * Ptranscript [EnsPTranscript*] Ensembl Transcript object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTranscriptDel *****************************************************
**
** Default destructor for an Ensembl Transcript.
**
** @param [d] Ptranscript [EnsPTranscript*] Ensembl Transcript object address
**
** @return [void]
** @@
******************************************************************************/

void ensTranscriptDel(EnsPTranscript* Ptranscript)
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

    ensDatabaseentryDel(&pthis->Displayreference);

    ajStrDel(&pthis->Description);

    ajStrDel(&pthis->Biotype);

    ajStrDel(&pthis->Stableidentifier);

    ajStrDel(&pthis->DateCreation);

    ajStrDel(&pthis->DateModification);

    /*
    ** Clear and delete the AJAX List of alternative
    ** Ensembl Translation objects.
    */

    while(ajListPop(pthis->Alternativetranslations, (void**) &translation))
        ensTranslationDel(&translation);

    ajListFree(&pthis->Alternativetranslations);

    /* Clear and delete the AJAX List of Ensembl Attributes. */

    while(ajListPop(pthis->Attributes, (void**) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while(ajListPop(pthis->Databaseentries, (void**) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Exon objects. */

    while(ajListPop(pthis->Exons, (void**) &exon))
        ensExonDel(&exon);

    ajListFree(&pthis->Exons);

    /* Clear and delete the AJAX List of Ensembl Base Align Feature objects. */

    while(ajListPop(pthis->Supportingfeatures, (void**) &baf))
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
**
** @nam3rule Get Return Transcript attribute(s)
** @nam4rule Adaptor Return the Ensembl Transcript Adaptor
** @nam4rule Biotype Return the biological type
** @nam4rule Canonical Return the canonical element
** @nam4rule Current Return the current element
** @nam4rule Date Return a date
** @nam5rule DateCreation Return the creation date
** @nam5rule DateModification Return the modification date
** @nam4rule Description Return the description
** @nam4rule Displayreference Return the display Database Entry
** @nam4rule Feature Return the Feature
** @nam4rule Geneidentifier Return the Ensembl Gene identifier
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Sequenceedits Return the enable Ensembl Sequence Edit flag
** @nam4rule Stableidentifier Return the stable identifier
** @nam4rule Status Return the status
** @nam4rule Version Return the version
**
** @argrule * transcript [const EnsPTranscript] Transcript
**
** @valrule Adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
** @valrule Biotype [AjPStr] Biological type or NULL
** @valrule Canonical [AjBool] Canonical atrribute or ajFalse
** @valrule Current [AjBool] Current attribute or ajFalse
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Modification date or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayreference [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Geneidentifier [ajuint] Ensembl Gene identifier or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Sequenceedits [AjBool] Enable Ensembl Sequence Edit flag ro ajFalse
** @valrule Status [EnsETranscriptStatus] Status or NULL
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Version [ajuint] Version or 0
** @valrule Exons [const AjPList] AJAX List of Ensembl Exon objects or NULL
** @valrule Supportingfeatures [const AjPList] AJAX List of Ensembl Base
** Align Feature objects or NULL
** @valrule Translation [EnsPTranslation] Ensembl Translation or NULL
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
** @return [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptGetAdaptor(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Adaptor;
}




/* @func ensTranscriptGetBiotype **********************************************
**
** Get the biological type element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::biotype
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Biological type or NULL
** @@
******************************************************************************/

AjPStr ensTranscriptGetBiotype(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Biotype;
}




/* @func ensTranscriptGetCanonical ********************************************
**
** Get the canonical element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_canonical
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue if this is a canonical Transcript
** @@
******************************************************************************/

AjBool ensTranscriptGetCanonical(const EnsPTranscript transcript)
{
    if(!transcript)
        return ajFalse;

    return transcript->Canonical;
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




/* @func ensTranscriptGetDateCreation *****************************************
**
** Get the creation date element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::created_date
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Creation date or NULL
** @@
******************************************************************************/

AjPStr ensTranscriptGetDateCreation(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->DateCreation;
}




/* @func ensTranscriptGetDateModification *************************************
**
** Get the modification date element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::modified_date
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Modification date or NULL
** @@
******************************************************************************/

AjPStr ensTranscriptGetDateModification(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->DateModification;
}




/* @func ensTranscriptGetDescription ******************************************
**
** Get the description element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::description
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensTranscriptGetDescription(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Description;
}




/* @func ensTranscriptGetDisplayreference *************************************
**
** Get the display reference element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::display_xref
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @@
******************************************************************************/

EnsPDatabaseentry ensTranscriptGetDisplayreference(
    const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Displayreference;
}




/* @func ensTranscriptGetFeature **********************************************
**
** Get the Ensembl Feature element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Feature
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensTranscriptGetFeature(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Feature;
}




/* @func ensTranscriptGetGeneidentifier ***************************************
**
** Get the SQL database-internal Ensembl Gene identifier element of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::???
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] SQL database-internal identifier of the Ensembl Gene or 0
** @@
******************************************************************************/

ajuint ensTranscriptGetGeneidentifier(const EnsPTranscript transcript)
{
    if(!transcript)
        return 0;

    return transcript->Geneidentifier;
}




/* @func ensTranscriptGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensTranscriptGetIdentifier(const EnsPTranscript transcript)
{
    if(!transcript)
        return 0;

    return transcript->Identifier;
}




/* @func ensTranscriptGetSequenceedits ****************************************
**
** Get the enable Ensembl Sequence Edit objects element of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::edits_enabled
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue: Apply Ensembl Sequence Edit objects to the sequence
** @@
******************************************************************************/

AjBool ensTranscriptGetSequenceedits(const EnsPTranscript transcript)
{
    if(!transcript)
        return ajFalse;

    return transcript->Sequenceedits;
}




/* @func ensTranscriptGetStableidentifier *************************************
**
** Get the stable identifier element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::stable_id
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Stable identifier or NULL
** @@
******************************************************************************/

AjPStr ensTranscriptGetStableidentifier(const EnsPTranscript transcript)
{
    if(!transcript)
        return NULL;

    return transcript->Stableidentifier;
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




/* @func ensTranscriptGetVersion **********************************************
**
** Get the version element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::version
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Version or 0
** @@
******************************************************************************/

ajuint ensTranscriptGetVersion(const EnsPTranscript transcript)
{
    if(!transcript)
        return 0;

    return transcript->Version;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Transcript object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Load Return Ensembl Transcript attribute(s) loaded on demand
** @nam4rule Alternativetranslations Return all alternative Ensembl Translation
**                                   objects
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Exons Return all Ensembl Exon objects
** @nam4rule Supportingfeatures Return the supporting Ensembl Base Align
**                              Feature objects
** @nam4rule Translation Return the Ensembl Translation
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule Alternativetranslations [const AjPList] AJAX List of alternative
** Ensembl Translation objects or NULL
** @valrule Attributes [const AjPList] AJAX List of Ensembl Attribute objects
** or NULL
** @valrule Databaseentries [const AjPList] AJAX List of
** Ensembl Database Entry objects or NULL
** @valrule Exons [const AjPList] AJAX List of Ensembl Exon objects or NULL
** @valrule Supportingfeatures [const AjPList] AJAX List of supporting
** Ensembl Base Align Feature objects or NULL
** @valrule Translation [EnsPTranslation] Ensembl Translation or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptLoadAlternativetranslations *****************************
**
** Load all alternative Ensembl Translation objects of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch Ensembl Translation
** objects from the Ensembl SQL database in case the AJAX List is not defined.
**
**
** @cc Bio::EnsEMBL::Transcript::get_all_alternative_translations
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllAttributes
**
** @return [const AjPList] AJAX List of Ensembl Translation objects or NULL
** @@
******************************************************************************/

const AjPList ensTranscriptLoadAlternativetranslations(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranslationadaptor tla  = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Alternativetranslations)
        return transcript->Alternativetranslations;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadAlternativetranslations cannot fetch "
                "Ensembl Translation objects for an Ensembl Transcript "
                "without an Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if(!dba)
    {
        ajDebug("ensTranscriptLoadAlternativetranslations cannot fetch "
                "Ensembl Translation objects for an Ensembl Transcript "
                "without an Ensembl Database Adaptor set in the "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    tla = ensRegistryGetTranslationadaptor(dba);

    transcript->Alternativetranslations = ajListNew();

    ensTranslationadaptorFetchAllbyTranscript(tla, transcript);

    return transcript->Alternativetranslations;
}




/* @func ensTranscriptLoadAttributes ******************************************
**
** Load all Ensembl Attribute objects of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch Ensembl Attribute
** objects from an Ensembl SQL database in case the AJAX List is empty.
**
** To filter Ensembl Attribute objects via their code, consider using
** ensTranscriptFetchAllAttributes.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Attributes
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllAttributes
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
** @@
******************************************************************************/

const AjPList ensTranscriptLoadAttributes(EnsPTranscript transcript)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Attributes)
        return transcript->Attributes;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadAttributes cannot fetch "
                "Ensembl Attributes for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if(!dba)
    {
        ajDebug("ensTranscriptLoadAttributes cannot fetch "
                "Ensembl Attributes for a Transcript without a "
                "Database Adaptor set in the Transcript Adaptor.\n");

        return NULL;
    }

    ata = ensRegistryGetAttributeadaptor(dba);

    transcript->Attributes = ajListNew();

    ensAttributeadaptorFetchAllbyTranscript(ata,
                                            transcript,
                                            (const AjPStr) NULL,
                                            transcript->Attributes);

    return transcript->Attributes;
}




/* @func ensTranscriptLoadDatabaseentries *************************************
**
** Load all Ensembl Database Entry objects of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Database Entry objects from the Ensembl SQL database associated with the
** Transcript Adaptor.
**
** To filter Ensembl Database Entry objects via an Ensembl External Database
** name or type, consider using ensTranscriptFetchAllDatabaseentries.
**
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllDatabaseentries
**
** @return [const AjPList] AJAX List of Ensembl Database Entry objects or NULL
** @@
******************************************************************************/

const AjPList ensTranscriptLoadDatabaseentries(EnsPTranscript transcript)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentryadaptor dbea = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Databaseentries)
        return transcript->Databaseentries;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Transcript "
                "without an Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if(!dba)
    {
        ajDebug("ensTranscriptLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Transcript "
                "without an Ensembl Database Adaptor set in the "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dbea = ensRegistryGetDatabaseentryadaptor(dba);

    objtype = ajStrNewC("Transcript");

    transcript->Databaseentries = ajListNew();

    ensDatabaseentryadaptorFetchAllbyObject(dbea,
                                            transcript->Identifier,
                                            objtype,
                                            (AjPStr) NULL,
                                            ensEExternaldatabaseTypeNULL,
                                            transcript->Databaseentries);

    ajStrDel(&objtype);

    return transcript->Databaseentries;
}




/* @func ensTranscriptLoadExons ***********************************************
**
** Load all Ensembl Exon objects of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt fetching the
** Exons from the Ensembl Core database associated with the
** Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Exons
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Exon objects or NULL
** @@
** NOTE: This implementation does not support the constitutive parameter.
** Please use the ensTranscriptFetchAllConstitutiveexons function instead.
******************************************************************************/

const AjPList ensTranscriptLoadExons(EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPExonadaptor ea = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Exons)
        return transcript->Exons;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadExons cannot fetch "
                "Ensembl Exon objects for an "
                "Ensembl Transcript without an "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    ea = ensRegistryGetExonadaptor(dba);

    transcript->Exons = ajListNew();

    ensExonadaptorFetchAllbyTranscript(ea, transcript, transcript->Exons);

    return transcript->Exons;
}




/* @func ensTranscriptLoadSupportingfeatures **********************************
**
** Load all Ensembl Supporting Feature objects of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** supporting Ensembl Base Align Feature objects from the Ensembl SQL database
** associated with the Ensembl Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::get_all_supporting_features
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Base Align Feature objects
** or NULL
** @@
******************************************************************************/

const AjPList ensTranscriptLoadSupportingfeatures(EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Supportingfeatures)
        return transcript->Supportingfeatures;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadSupportingfeatures cannot fetch "
                "Ensembl Base Align Feature objects for an Ensembl Transcript "
                "without an Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    transcript->Supportingfeatures = ajListNew();

    ensSupportingfeatureadaptorFetchAllbyTranscript(
        dba,
        transcript,
        transcript->Supportingfeatures);

    return transcript->Supportingfeatures;
}




/* @func ensTranscriptLoadTranslation *****************************************
**
** Load the Ensembl Translation of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Translation from the Ensembl Core database associated
** with the Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranslation] Ensembl Translation objects or NULL
** @@
******************************************************************************/

EnsPTranslation ensTranscriptLoadTranslation(EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranslationadaptor tla = NULL;

    if(!transcript)
        return NULL;

    if(transcript->Translation)
        return transcript->Translation;

    if(!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadTranslation cannot fetch an "
                "Ensembl Translation for a Transcript without a "
                "Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    tla = ensRegistryGetTranslationadaptor(dba);

    ensTranslationadaptorFetchByTranscript(tla, transcript);

    return transcript->Translation;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Set Set one element of a Transcript
** @nam4rule Adaptor Set the Ensembl Transcript Adaptor
** @nam4rule Biotype Set the biological type
** @nam4rule Canonical Set the canonical attribute
** @nam4rule Current Set the current attribute
** @nam4rule Date Set a date
** @nam5rule DateCreation Set the creation date
** @nam5rule DateModification Set the modification date
** @nam4rule Description Set the description
** @nam4rule Displayreference Set the display Ensembl Database Entry
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Geneidentifier Set the Ensembl Gene identifier
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Stableidentifier Set the stable identifier
** @nam4rule Status Set the status
** @nam4rule Version Set the version
** @nam4rule Sequenceedits Set enable Ensembl Sequence Edit objects
** @nam4rule Translation Set the Ensembl Translation
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Adaptor tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule Biotype biotype [AjPStr] Biological type
** @argrule Canonical canonical [AjBool] Canonical attribute
** @argrule Current current [AjBool] Current attribute
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule DateModification mdate [AjPStr] Modification date
** @argrule Description description [AjPStr] Description
** @argrule Displayreference displaydbe [EnsPDatabaseentry] Ensembl Database
** Entry
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Geneidentifier geneid [ajuint] Ensembl Gene identifier
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Sequenceedits enablese [AjBool] Enable Ensembl Sequence Edit
** objects
** @argrule Stableidentifier stableid [AjPStr] Stable identifier
** @argrule Status status [EnsETranscriptStatus] Status
** @argrule Translation translation [EnsPTranslation] Ensembl Translation
** @argrule Version version [ajuint] Version
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




/* @func ensTranscriptSetBiotype **********************************************
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

AjBool ensTranscriptSetBiotype(EnsPTranscript transcript,
                               AjPStr biotype)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->Biotype);

    if(biotype)
        transcript->Description = ajStrNewRef(biotype);

    return ajTrue;
}




/* @func ensTranscriptSetCanonical ********************************************
**
** Set the canonical element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_canonical
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] canonical [AjBool] Canonical attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetCanonical(EnsPTranscript transcript,
                                 AjBool canonical)
{
    if(!transcript)
        return ajFalse;

    transcript->Canonical = canonical;

    return ajTrue;
}




/* @func ensTranscriptSetCurrent **********************************************
**
** Set the current element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_current
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] current [AjBool] Current attribute
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




/* @func ensTranscriptSetDateCreation *****************************************
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

AjBool ensTranscriptSetDateCreation(EnsPTranscript transcript,
                                    AjPStr cdate)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->DateCreation);

    if(cdate)
        transcript->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensTranscriptSetDateModification *************************************
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

AjBool ensTranscriptSetDateModification(EnsPTranscript transcript,
                                        AjPStr mdate)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->DateModification);

    if(mdate)
        transcript->DateModification = ajStrNewRef(mdate);

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




/* @func ensTranscriptSetDisplayreference *************************************
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

AjBool ensTranscriptSetDisplayreference(EnsPTranscript transcript,
                                        EnsPDatabaseentry displaydbe)
{
    if(!transcript)
        return ajFalse;

    ensDatabaseentryDel(&transcript->Displayreference);

    transcript->Displayreference = ensDatabaseentryNewRef(displaydbe);

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
    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    EnsPBasealignfeature oldbaf = NULL;
    EnsPBasealignfeature newbaf = NULL;

    EnsPExon oldexon = NULL;
    EnsPExon newexon = NULL;

    EnsPSlice slice = NULL;

    EnsPTranslation translation = NULL;

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

    ensFeatureDel(&transcript->Feature);

    transcript->Feature = ensFeatureNewRef(feature);

    slice = ensFeatureGetSlice(transcript->Feature);

    /*
    ** Transfer Ensembl Exon objects onto the new Ensembl Feature Slice and
    ** thereby also adjust the start and end Exon objects of the
    ** Ensembl Translation if it is defined.
    */

    if(transcript->Exons)
    {
        iter1 = ajListIterNew(transcript->Exons);

        while(!ajListIterDone(iter1))
        {
            oldexon = (EnsPExon) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newexon = ensExonTransfer(oldexon, slice);

            if(!newexon)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Exon onto new Ensembl Feature Slice.");

                ensExonTrace(oldexon, 1);
            }

            ajListIterInsert(iter1, (void*) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            /*
            ** Re-assign the start and end Ensembl Exon objects of the
            ** Ensembl Translation.
            */

            if(transcript->Translation)
            {
                if(ensExonMatch(
                       oldexon,
                       ensTranslationGetStartexon(transcript->Translation)))
                    ensTranslationSetStartexon(transcript->Translation,
                                               newexon);

                if(ensExonMatch(
                       oldexon,
                       ensTranslationGetEndexon(transcript->Translation)))
                    ensTranslationSetEndexon(transcript->Translation,
                                             newexon);
            }

            /*
            ** Reassign also the start and end Exon for each alternative
            ** Translation of this Transcript.
            */

            if(transcript->Alternativetranslations)
            {
                iter2 = ajListIterNew(transcript->Alternativetranslations);

                while(!ajListIterDone(iter2))
                {
                    translation = (EnsPTranslation) ajListIterGet(iter2);

                    if(ensExonMatch(
                           oldexon,
                           ensTranslationGetStartexon(translation)))
                        ensTranslationSetStartexon(translation, newexon);

                    if(ensExonMatch(
                           oldexon,
                           ensTranslationGetEndexon(translation)))
                        ensTranslationSetEndexon(translation, newexon);
                }

                ajListIterDel(&iter2);
            }

            ensExonDel(&oldexon);
        }

        ajListIterDel(&iter1);
    }

    /*
    ** Transfer all Ensembl Base Align Feature objects onto the new
    ** Ensembl Feature Slice.
    */

    if(transcript->Supportingfeatures)
    {
        iter1 = ajListIterNew(transcript->Supportingfeatures);

        while(!ajListIterDone(iter1))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newbaf = ensBasealignfeatureTransfer(oldbaf, slice);

            if(!newbaf)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Base Align Feature onto new Ensembl Feature Slice.");

                ensBasealignfeatureTrace(oldbaf, 1);
            }

            ajListIterInsert(iter1, (void*) newbaf);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            ensBasealignfeatureDel(&oldbaf);
        }

        ajListIterDel(&iter1);
    }

    /* Clear internal values that depend on Exon coordinates. */

    ensMapperClear(transcript->ExonCoordMapper);

    return ajTrue;
}




/* @func ensTranscriptSetGeneidentifier ***************************************
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

AjBool ensTranscriptSetGeneidentifier(EnsPTranscript transcript,
                                      ajuint geneid)
{
    if(!transcript)
        return ajFalse;

    transcript->Geneidentifier = geneid;

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




/* @func ensTranscriptSetSequenceedits ****************************************
**
** Set the enable Ensembl Sequence Edit object element of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::edits_enabled
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] enablese [AjBool] Enable Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptSetSequenceedits(EnsPTranscript transcript,
                                     AjBool enablese)
{
    if(!transcript)
        return ajFalse;

    transcript->Sequenceedits = enablese;

    return ajTrue;
}




/* @func ensTranscriptSetStableidentifier *************************************
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

AjBool ensTranscriptSetStableidentifier(EnsPTranscript transcript,
                                        AjPStr stableid)
{
    if(!transcript)
        return ajFalse;

    ajStrDel(&transcript->Stableidentifier);

    if(stableid)
        transcript->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensTranscriptSetStatus ***********************************************
**
** Set the status element of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::status
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] status [EnsETranscriptStatus] Status
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

    /*
    ** NOTE: The Perl API clears the Bio::EnsEMBL::TranscriptMapper,
    ** although it is independent of a Bio::EnsEMBL::Translation.
    ** It also sets the Bio::EnsEMBL::Transcript in the
    ** Bio::EnsEMBL::Translation object via weak references,
    ** which is not implemented here.
    */

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




/* @section element addition **************************************************
**
** Functions for adding elements to an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Add Add one object to an Ensembl Transcript
** @nam4rule Alternativetranslation Add an alternative Ensembl Translation
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Databaseentry Add an Ensembl Database Entry
** @nam4rule Exon Add an Ensembl Exon
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript object
** @argrule Alternativetranslation translation [EnsPTranslation]
**                                                         Ensembl Translation
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Databaseentry dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Exon exon [EnsPExon] Ensembl Exon
** @argrule Exon rank [ajint] Ensembl Exon rank
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensTranscriptAddAlternativetranslation *******************************
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

AjBool ensTranscriptAddAlternativetranslation(EnsPTranscript transcript,
                                              EnsPTranslation translation)
{
    if(!transcript)
        return ajFalse;

    if(!translation)
        return ajFalse;

    if(!transcript->Alternativetranslations)
        transcript->Alternativetranslations = ajListNew();

    ajListPushAppend(transcript->Alternativetranslations,
                     (void*) ensTranslationNewRef(translation));

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
                     (void*) ensAttributeNewRef(attribute));

    /*
    ** Check if this Attribute is associated with a Sequence Edit and clear
    ** internal values that depend on the now modified Transcript sequence.
    */

    for(i = 0; transcriptSequenceeditCode[i]; i++)
        if(ajStrMatchC(ensAttributeGetCode(attribute),
                       transcriptSequenceeditCode[i]))
            match = ajTrue;

    if(match)
        ensMapperClear(transcript->ExonCoordMapper);

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

    if(!transcript->Databaseentries)
        transcript->Databaseentries = ajListNew();

    ajListPushAppend(transcript->Databaseentries,
                     (void*) ensDatabaseentryNewRef(dbe));

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
                                     (void*) ensExonNewRef(exon));

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
                ** in the next iteration.
                */

                ajListIterRemove(iter);

                ensExonDel(&lastexon);

                ajListIterInsert(iter, (void*) ensExonNewRef(exon));

                /* Advance the AJAX List Iterator after the insert. */

                lastexon = (EnsPExon) ajListIterGet(iter);
            }
        }

        ajListIterDel(&iter);

        return ajTrue;
    }

    feature = ensExonGetFeature(exon);

    if(ensFeatureGetStrand(feature) > 0)
    {
        ajListPeekLast(transcript->Exons, (void**) &lastexon);

        lastfeature = ensExonGetFeature(lastexon);

        if(ensFeatureGetStart(feature) > ensFeatureGetEnd(lastfeature))
        {
            /* Append at the end. */

            ajListPushAppend(transcript->Exons, (void*) ensExonNewRef(exon));

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

                if(ensFeatureGetStart(feature) <
                   ensFeatureGetStart(lastfeature))
                {
                    /* Ensembl Exon objects that overlap are not added. */

                    if(ensFeatureGetEnd(feature) >=
                       ensFeatureGetStart(lastfeature))
                        break;

                    ajListIterInsert(iter, (void*) ensExonNewRef(exon));

                    added = ajTrue;

                    break;
                }
            }

            ajListIterDel(&iter);
        }
    }
    else
    {
        ajListPeekLast(transcript->Exons, (void**) &lastexon);

        lastfeature = ensExonGetFeature(lastexon);

        if(ensFeatureGetEnd(feature) < ensFeatureGetStart(lastfeature))
        {
            /* Append at the end. */

            ajListPushAppend(transcript->Exons, (void*) ensExonNewRef(exon));

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

                if(ensFeatureGetEnd(feature) > ensFeatureGetEnd(lastfeature))
                {
                    /* Ensembl Exon objects that overlap are not added. */

                    if(ensFeatureGetStart(feature) <=
                       ensFeatureGetEnd(lastfeature))
                        break;

                    ajListIterInsert(iter, (void*) ensExonNewRef(exon));

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
        /* The Exon was not added because it overlaps with another Exon. */

        message = ajStrNewC("ensTranscriptAddExon got an Exon, "
                            "which overlaps with another Exon in the same "
                            "Transcript.\n"
                            "  Transcript Exons:\n");

        iter = ajListIterNew(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            lastexon = (EnsPExon) ajListIterGet(iter);

            lastfeature = ensExonGetFeature(lastexon);

            ajFmtPrintAppS(&message,
                           "    %S %d:%d:%d\n",
                           ensExonGetStableidentifier(exon),
                           ensFeatureGetStart(lastfeature),
                           ensFeatureGetEnd(lastfeature),
                           ensFeatureGetStrand(lastfeature));
        }

        ajListIterDel(&iter);

        ajFmtPrintAppS(&message,
                       "  This Exon:\n"
                       "    %S %d:%d:%d\n",
                       ensExonGetStableidentifier(exon),
                       ensFeatureGetStart(feature),
                       ensFeatureGetEnd(feature),
                       ensFeatureGetStrand(feature));

        ajFatal(ajStrGetPtr(message));

        ajStrDel(&message);
    }

    /*
    ** Recalculate the Transcript coordinates, delete the
    ** Ensembl Transcript Mapper, as well as internal cache values.
    */

    ensTranscriptCalculateCoordinates(transcript);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
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
            "%S  Displayreference %p\n"
            "%S  Description '%S'\n"
            "%S  Biotype '%S'\n"
            "%S  Status %d\n"
            "%S  Current '%B'\n"
            "%S  Stableidentifier '%S'\n"
            "%S  DateCreation '%S'\n"
            "%S  DateModification '%S'\n"
            "%S  Version %u\n"
            "%S  Geneidentifier %u\n"
            "%S  Alternativetranslations %p\n"
            "%S  Attributes %p\n"
            "%S  Databaseentries %p\n"
            "%S  Exons %p\n"
            "%S  Supportingfeatures %p\n"
            "%S  Translation %p\n"
            "%S  Sequenceedits '%B'\n"
            "%S  Canonical '%B'\n"
            "%S  ExonCoordMapper %p\n",
            indent, transcript,
            indent, transcript->Use,
            indent, transcript->Identifier,
            indent, transcript->Adaptor,
            indent, transcript->Feature,
            indent, transcript->Displayreference,
            indent, transcript->Description,
            indent, transcript->Biotype,
            indent, transcript->Status,
            indent, transcript->Current,
            indent, transcript->Stableidentifier,
            indent, transcript->DateCreation,
            indent, transcript->DateModification,
            indent, transcript->Version,
            indent, transcript->Geneidentifier,
            indent, transcript->Alternativetranslations,
            indent, transcript->Attributes,
            indent, transcript->Databaseentries,
            indent, transcript->Exons,
            indent, transcript->Supportingfeatures,
            indent, transcript->Translation,
            indent, transcript->Sequenceedits,
            indent, transcript->Canonical,
            indent, transcript->ExonCoordMapper);

    ensFeatureTrace(transcript->Feature, level + 1);

    ensDatabaseentryTrace(transcript->Displayreference, level + 1);

    /* Trace the AJAX List of alternative Ensembl Translation objects. */

    if(transcript->Alternativetranslations)
    {
        ajDebug("%S    AJAX List %p of alternative "
                "Ensembl Translation objects:\n",
                indent, transcript->Alternativetranslations);

        iter = ajListIterNewread(transcript->Alternativetranslations);

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

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if(transcript->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entry objects\n",
                indent, transcript->Databaseentries);

        iter = ajListIterNewread(transcript->Databaseentries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Exon objects. */

    if(transcript->Exons)
    {
        ajDebug("%S    AJAX List %p of Ensembl Exon objects\n",
                indent, transcript->Exons);

        iter = ajListIterNewread(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ensExonTrace(exon, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of supporting Ensembl Base Align Feature objects. */

    if(transcript->Supportingfeatures)
    {
        ajDebug("%S    AJAX List %p of Ensembl Base Align Feature objects:\n",
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




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Calculate Calculate Ensembl Transcript values
** @nam4rule Coordinates Calculate coordinates
** @nam4rule Length  Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Phase   Calculate the phase
** @nam4rule Slice   Calculate Ensembl Transcript coordinates relative to
**                   an Ensembl Slice
** @nam5rule Coding  Calculate Ensembl Transcript coding coordinates
** @nam6rule End     Calculate the Ensembl Transcript coding end coordinate
** @nam6rule Start   Calculate the Ensembl Transcript coding start coordinate
** @nam4rule Transcript Calculate Ensembl Transcript coordinates relative to
**                      an Ensembl Transcript
** @nam5rule Coding  Calculate Ensembl Transcript coding coordinates
** @nam6rule End     Calculate the Ensembl Transcript coding end coordinate
** @nam6rule Start   Calculate the Ensembl Transcript coding start coordinate
** @nam5rule End     Calculate the Ensembl Transcript end coordinate
** @nam5rule Start   Calculate the Ensembl Transcript start coordinate
**
** @argrule Coordinates transcript [EnsPTranscript] Ensembl Transcript
** @argrule Length transcript [EnsPTranscript] Ensembl Transcript
** @argrule Memsize transcript [const EnsPTranscript] Ensembl Transcript
** @argrule Phase transcript [EnsPTranscript] Ensembl Transcript
** @argrule CodingEnd transcript [EnsPTranscript] Ensembl Transcript
** @argrule CodingEnd translation [EnsPTranslation] Ensembl Translation
** @argrule CodingStart transcript [EnsPTranscript] Ensembl Transcript
** @argrule CodingStart translation [EnsPTranslation] Ensembl Translation
** @argrule TranscriptEnd exon [EnsPExon] Ensembl Exon
** @argrule TranscriptEnd transcript [EnsPTranscript] Ensembl Transcript
** @argrule TranscriptStart exon [EnsPExon] Ensembl Exon
** @argrule TranscriptStart transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule Coordinates [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Length [ajuint] Transcript (cDNA) length or 0
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule PhaseStart [ajint] Start phase or -1
** @valrule SliceCodingEnd [ajint] End coordinate or 0
** @valrule SliceCodingStart [ajint] Start coordinate or 0
** @valrule TranscriptCodingEnd [ajuint] End coordinate or 0
** @valrule TranscriptCodingStart [ajuint] Start coordinate or 0
** @valrule TranscriptEnd [ajuint] End coordinate or 0
** @valrule TranscriptStart [ajuint] Start coordinate or 0
**
** @fcategory misc
******************************************************************************/




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

    list = ensTranscriptLoadExons(transcript);

    if(!ajListGetLength(list))
        return ajTrue;

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        /* Skip missing Exon objects. */

        if(!exon)
            continue;

        efeature = ensExonGetFeature(exon);

        /* Skip un-mapped Exon objects. */

        if(!ensFeatureGetStart(efeature))
            continue;

        slice  = ensFeatureGetSlice(efeature);
        start  = ensFeatureGetStart(efeature);
        end    = ensFeatureGetEnd(efeature);
        strand = ensFeatureGetStrand(efeature);

        break;
    }

    ajListIterDel(&iter);

    /* Start loop after the first Exon with coordinates. */

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        /* Skip missing Exon objects. */

        if(!exon)
            continue;

        efeature = ensExonGetFeature(exon);

        /* Skip un-mapped Exon objects. */

        if(!ensFeatureGetStart(efeature))
            continue;

        if(!ensSliceMatch(ensFeatureGetSlice(efeature), slice))
            ajFatal("ensTranscriptCalculateCoordinates got "
                    "Ensembl Exon objects of one "
                    "Ensembl Transcript on different "
                    "Ensembl Slice objects.\n");

        start = (ensFeatureGetStart(efeature) < start)
            ? ensFeatureGetStart(efeature) : start;

        end = (ensFeatureGetEnd(efeature) > end)
            ? ensFeatureGetEnd(efeature) : end;

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

    return ajTrue;
}




/* @func ensTranscriptCalculateLength *****************************************
**
** Calculate the length of an Ensembl Transcript, which is the sum of the
** length of all Ensembl Exon objects.
**
** @cc Bio::EnsEMBL::Transcript::length
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Transcript (cDNA) length or 0
** @@
******************************************************************************/

ajuint ensTranscriptCalculateLength(EnsPTranscript transcript)
{
    ajuint length = 0;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    if(!transcript)
        return 0;

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        feature = ensExonGetFeature(exon);

        length += ensFeatureCalculateLength(feature);
    }

    ajListIterDel(&iter);

    /*
    ** Adjust the length if post-transcriptional Sequence Edit objects are
    ** applied.
    */

    if(transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        while(ajListPop(ses, (void**) &se))
        {
            length += ensSequenceeditCalculateDifference(se);

            ensSequenceeditDel(&se);
        }

        ajListFree(&ses);
    }

    return length;
}




/* @func ensTranscriptCalculateMemsize ****************************************
**
** Calculate the memory size in bytes of an Ensembl Transcript.
**
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensTranscriptCalculateMemsize(const EnsPTranscript transcript)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return 0;

    size += sizeof (EnsOTranscript);

    size += ensFeatureCalculateMemsize(transcript->Feature);

    size += ensDatabaseentryCalculateMemsize(transcript->Displayreference);

    if(transcript->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Description);
    }

    if(transcript->Biotype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Biotype);
    }

    if(transcript->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Stableidentifier);
    }

    if(transcript->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->DateCreation);
    }

    if(transcript->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->DateModification);
    }

    /* Summarise the AJAX List of alternative Ensembl Translation objects. */

    if(transcript->Alternativetranslations)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Alternativetranslations);

        while(!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            size += ensTranslationCalculateMemsize(translation);
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

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if(transcript->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Databaseentries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Exon objects. */

    if(transcript->Exons)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            size += ensExonCalculateMemsize(exon);
        }

        ajListIterDel(&iter);
    }

    /*
    ** Summarise the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    if(transcript->Supportingfeatures)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Supportingfeatures);

        while(!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            size += ensBasealignfeatureCalculateMemsize(baf);
        }

        ajListIterDel(&iter);
    }

    size += ensTranslationCalculateMemsize(transcript->Translation);

    size += ensMapperCalculateMemsize(transcript->ExonCoordMapper);

    return size;
}




/* @func ensTranscriptCalculatePhaseStart *************************************
**
** Calculate the start phase of an Ensembl Transcript,
** which is the start phase of the first Ensembl Exon object.
**
** This function will load the Ensembl Exon objects in case they have not
** been loaded before.
**
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajint] Start phase or -1
** @@
******************************************************************************/

ajint ensTranscriptCalculatePhaseStart(EnsPTranscript transcript)
{
    const AjPList exons = NULL;

    EnsPExon exon = NULL;

    if(!transcript)
        return -1;

    exons = ensTranscriptLoadExons(transcript);

    ajListPeekFirst(exons, (void**) &exon);

    if(!exon)
        return -1;

    return ensExonGetPhaseStart(exon);
}




/* @func ensTranscriptCalculateSliceCodingEnd *********************************
**
** Calculate the end position of the coding region in Slice coordinates.
**
** @cc Bio::EnsEMBL::Transcript::coding_region_end
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajint] Coding region end in Slice coordinates or 0
** @@
******************************************************************************/

ajint ensTranscriptCalculateSliceCodingEnd(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajint scend = 0;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    if(!transcript)
        return 0;

    if(!translation)
        return 0;

    /* Calculate the Slice coding start from the Translation. */

    exon = ensTranslationGetStartexon(translation);

    feature = ensExonGetFeature(exon);

    if(ensFeatureGetStrand(feature) >= 0)
    {
        exon = ensTranslationGetEndexon(translation);

        feature = ensExonGetFeature(exon);

        scend = ensFeatureGetStart(feature)
            + (ensTranslationGetEnd(translation) - 1);
    }
    else
    {
        exon = ensTranslationGetStartexon(translation);

        feature = ensExonGetFeature(exon);

        scend = ensFeatureGetEnd(feature)
            - (ensTranslationGetStart(translation) - 1);
    }

    return scend;
}




/* @func ensTranscriptCalculateSliceCodingStart *******************************
**
** Calculate the start position of the coding region in Slice coordinates.
**
** @cc Bio::EnsEMBL::Transcript::coding_region_start
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajint] Coding region start in Slice coordinates or 0
** @@
******************************************************************************/

ajint ensTranscriptCalculateSliceCodingStart(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajint scstart = 0;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    if(!transcript)
        return 0;

    if(!translation)
        return 0;

    /* Calculate the Slice coding start from the Translation. */

    exon = ensTranslationGetStartexon(translation);

    feature = ensExonGetFeature(exon);

    if(ensFeatureGetStrand(feature) >= 0)
    {
        exon = ensTranslationGetStartexon(translation);

        feature = ensExonGetFeature(exon);

        scstart = ensFeatureGetStart(feature)
            + (ensTranslationGetStart(translation) - 1);
    }
    else
    {
        exon = ensTranslationGetEndexon(translation);

        feature = ensExonGetFeature(exon);

        scstart = ensFeatureGetEnd(feature)
            - (ensTranslationGetEnd(translation) - 1);
    }

    return scstart;
}




/* @func ensTranscriptCalculateTranscriptCodingEnd ****************************
**
** Calculate the end position of the coding region in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Transcript::cdna_coding_end
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Coding region end in Transcript coordinates or 0,
** if this Transcript has no Translation
** @@
******************************************************************************/

ajuint ensTranscriptCalculateTranscriptCodingEnd(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajuint tcend = 0;

    AjBool debug = AJFALSE;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    debug = ajDebugTest("ensTranscriptCalculateTranscriptCodingEnd");

    if(debug)
    {
        ajDebug("ensTranscriptCalculateTranscriptCodingEnd\n"
                "  transcript %p\n"
                "  translation %p\n",
                transcript,
                translation);

        ensTranscriptTrace(transcript, 1);

        ensTranslationTrace(translation, 1);
    }

    if(!transcript)
        return 0;

    if(!translation)
        return 0;

    /*
    ** Calculate the coding start relative to the start of the
    ** Translation in Transcript coordinates.
    */

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if(debug)
            ajDebug("ensTranscriptCalculateTranscriptCodingEnd "
                    "exon %p (Identifier %u) end exon %p (Identifier %u)\n",
                    exon, ensExonGetIdentifier(exon),
                    ensTranslationGetEndexon(translation),
                    ensExonGetIdentifier(
                        ensTranslationGetEndexon(translation)));

        if(ensExonMatch(exon, ensTranslationGetEndexon(translation)))
        {
            /* Add the coding portion of the last coding Exon. */

            tcend += ensTranslationGetEnd(translation);

            break;
        }
        else
        {
            /* Add the entire length of this Exon. */

            feature = ensExonGetFeature(exon);

            tcend += ensFeatureCalculateLength(feature);
        }
    }

    ajListIterDel(&iter);

    /* Adjust Transcript coordinates if Sequence Edit objects are enabled. */

    if(transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        /*
        ** Sort in reverse order to avoid adjustment of down-stream
        ** Sequence Edit objects.
        */

        ensListSequenceeditSortStartDescending(ses);

        while(ajListPop(ses, (void**) &se))
        {
            /*
            ** Use less than or equal to end + 1 so that the end of the
            ** CDS can be extended.
            */

            if(ensSequenceeditGetStart(se) <= tcend + 1)
                tcend += ensSequenceeditCalculateDifference(se);

            ensSequenceeditDel(&se);
        }

        ajListFree(&ses);
    }

    return tcend;
}




/* @func ensTranscriptCalculateTranscriptCodingStart **************************
**
** Calculate the start position of the coding region in Transcript coordinates.
**
** @cc Bio::EnsEMBL::Transcript::cdna_coding_start
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Coding region start in Transcript coordinates or 0,
** if this Transcript has no Translation
** @@
******************************************************************************/

ajuint ensTranscriptCalculateTranscriptCodingStart(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajuint tcstart = 0;

    AjBool debug = AJFALSE;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    debug = ajDebugTest("ensTranscriptCalculateTranscriptCodingStart");

    if(debug)
    {
        ajDebug("ensTranscriptCalculateTranscriptCodingStart\n"
                "  transcript %p\n"
                "  translation %p\n",
                transcript,
                translation);

        ensTranscriptTrace(transcript, 1);

        ensTranslationTrace(translation, 1);
    }

    if(!transcript)
        return 0;

    /*
    ** Calculate the coding start relative to the start of the
    ** Translation in Transcript coordinates.
    */

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if(debug)
            ajDebug("ensTranscriptCalculateTranscriptCodingStart "
                    "exon %p (Identifier %u) start exon %p (Identifier %u)\n",
                    exon, ensExonGetIdentifier(exon),
                    ensTranslationGetStartexon(translation),
                    ensExonGetIdentifier(
                        ensTranslationGetStartexon(translation)));

        if(ensExonMatch(exon, ensTranslationGetStartexon(translation)))
        {
            /* Add the UTR portion of the first coding Exon. */

            tcstart += ensTranslationGetStart(translation);

            break;
        }
        else
        {
            /* Add the entire length of this non-coding Exon. */

            feature = ensExonGetFeature(exon);

            tcstart += ensFeatureCalculateLength(feature);
        }
    }

    ajListIterDel(&iter);

    /* Adjust Transcript coordinates if Sequence Edit objects are enabled. */

    if(transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        /*
        ** Sort in reverse order to avoid adjustment of down-stream
        ** Sequence Edit objects.
        */

        ensListSequenceeditSortStartDescending(ses);

        while(ajListPop(ses, (void**) &se))
        {
            if(ensSequenceeditGetStart(se) < tcstart)
                tcstart += ensSequenceeditCalculateDifference(se);

            ensSequenceeditDel(&se);
        }

        ajListFree(&ses);
    }

    return tcstart;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Transcript objects
**
** @fdata [EnsPTranscript]
**
** @nam3rule Match      Test Ensembl Transcript objects for identity
** @nam3rule Overlap    Test Ensembl Transcript objects for overlap
** @nam3rule Similarity Test Ensembl Transcript objects for similarity
**
** @argrule * transcript1 [EnsPTranscript] Ensembl Transcript
** @argrule * transcript2 [EnsPTranscript] Ensembl Transcript
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptSimilarity **********************************************
**
** Test Ensembl Transcript objects for similarity.
**
** @cc Bio::EnsEMBL::Transcript::equals
** @param [u] transcript1 [EnsPTranscript] Ensembl Transcript
** @param [u] transcript2 [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue if the Ensembl Transcript objects are similar
** @@
** NOTE: This function is similar to Bio::EnsEMBL::Transcript::equals, but not
** completely identical. The Bio::EnsEMBL::Exon objects are not sorted before
** comparison, as this would interfere with the internal AJAX List and their
** biological order in the Ensembl Transcript.
******************************************************************************/

AjBool ensTranscriptSimilarity(EnsPTranscript transcript1,
                               EnsPTranscript transcript2)
{
    AjBool mismatch = AJFALSE;
    AjBool similarity = AJFALSE;

    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    const AjPList exons1 = NULL;
    const AjPList exons2 = NULL;

    EnsPExon exon1 = NULL;
    EnsPExon exon2 = NULL;

    if(!transcript1)
        return ajFalse;

    if(!transcript2)
        return ajFalse;

    if(transcript1 == transcript2)
        return ajTrue;

    if(!ensFeatureSimilarity(transcript1->Feature, transcript2->Feature))
        return ajFalse;

    if(!ajStrMatchCaseS(transcript1->Biotype, transcript2->Biotype))
        return ajFalse;

    if((transcript1->Stableidentifier && transcript2->Stableidentifier) &&
       (!ajStrMatchCaseS(transcript1->Stableidentifier,
                         transcript2->Stableidentifier)))
        return ajFalse;

    exons1 = ensTranscriptLoadExons(transcript1);
    exons2 = ensTranscriptLoadExons(transcript2);

    if(ajListGetLength(exons1) != ajListGetLength(exons2))
        return ajFalse;

    iter1 = ajListIterNewread(exons1);
    iter2 = ajListIterNewread(exons2);

    while(!ajListIterDone(iter1))
    {
        exon1 = (EnsPExon) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        similarity = ajFalse;

        while(!ajListIterDone(iter2))
        {
            exon2 = (EnsPExon) ajListIterGet(iter2);

            if(ensExonSimilarity(exon1, exon2))
            {
                similarity = ajTrue;
                break;
            }
        }

        if(similarity == ajFalse)
        {
            mismatch = ajTrue;
            break;
        }
    }

    ajListIterDel(&iter1);
    ajListIterDel(&iter2);

    if(mismatch == ajTrue)
        return ajFalse;

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Transcript objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Transfer Transfer an Ensembl Transcript
** @nam3rule Transform Transform an Ensembl Transcript
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
**
** @valrule * [EnsPTranscript] Ensembl Transcript or NULL
**
** @fcategory misc
******************************************************************************/




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
    EnsPFeature newtf = NULL;

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

    newtf = ensFeatureTransfer(transcript->Feature, slice);

    if(!newtf)
        return NULL;

    newtranscript = transcriptNewCpyFeatures(transcript);

    ensTranscriptSetFeature(newtranscript, newtf);

    ensFeatureDel(&newtf);

    return newtranscript;
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
    ajint minstart      = INT_MAX;
    ajint maxend        = INT_MIN;

    ajuint psslength = 0;

    AjBool first       = AJFALSE;
    AjBool ignoreorder = AJFALSE;
    AjBool orderbroken = AJFALSE;
    AjBool error       = AJFALSE;

    AjIList iter1 = NULL;
    AjIList iter2 = NULL;
    AjPList pss   = NULL;

    EnsPBasealignfeature oldbaf = NULL;
    EnsPBasealignfeature newbaf = NULL;

    EnsPFeature newef = NULL;
    EnsPFeature oldef = NULL;
    EnsPFeature newtf = NULL;

    EnsPExon oldexon = NULL;
    EnsPExon newexon = NULL;

    EnsPProjectionsegment ps = NULL;

    const EnsPSeqregion     newsr = NULL;
    const EnsPSeqregion lastnewsr = NULL;

    EnsPTranscript newtranscript = NULL;

    EnsPTranslation translation = NULL;

    if(!transcript)
        return NULL;

    if(!csname)
        return NULL;

    newtf = ensFeatureTransform(transcript->Feature,
                                csname,
                                csversion,
                                (EnsPSlice) NULL);

    if(!newtf)
    {
        /*
        ** If the Ensembl Transcript does not transform, test whether it
        ** projects, since its Ensembl Exon objects may transform well.
        */

        pss = ajListNew();

        ensFeatureProject(transcript->Feature, csname, csversion, pss);

        psslength = ajListGetLength(pss);

        while(ajListPop(pss, (void**) &ps))
            ensProjectionsegmentDel(&ps);

        ajListFree(&pss);

        /*
        ** If the AJAX List of Ensembl Projection Segment objects is empty,
        ** the Transcript did not project. If it projects, make sure
        ** all its Ensembl Exon objects are loaded.
        */

        if(psslength == 0)
            return NULL;
        else
            ensTranscriptLoadExons(transcript);
    }

    /*
    ** Copy the Ensembl Transcript object and also its internal
    ** Ensembl Feature class-based members.
    */

    newtranscript = transcriptNewCpyFeatures(transcript);

    if(newtranscript->Exons)
    {
        first = ajTrue;

        iter1 = ajListIterNew(newtranscript->Exons);

        while(!ajListIterDone(iter1))
        {
            /*
            ** We want to check, whether the transform preserved the
            ** 5 prime to 3 prime order. No complaints on trans-splicing.
            */

            oldexon = (EnsPExon) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newexon = ensExonTransform(oldexon, csname, csversion);

            if(!newexon)
            {
                error = ajTrue;

                break;
            }

            ajListIterInsert(iter1, (void*) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            if(!newtf)
            {
                oldef = ensExonGetFeature(oldexon);
                newef = ensExonGetFeature(newexon);

                if(first)
                {
                    minstart = ensFeatureGetStart(newef);
                    maxend   = ensFeatureGetEnd(newef);

                    first = ajFalse;
                }
                else
                {
                    newsr = ensFeatureGetSeqregionObject(newef);

                    if(!ensSeqregionMatch(newsr, lastnewsr))
                    {
                        error = ajTrue;

                        break;
                    }

                    /*
                    ** Test for a trans-splicing event and ignore the Exon
                    ** order in those cases.
                    */

                    if(ensFeatureGetStrand(oldef) != lastoldstrand)
                        ignoreorder = ajTrue;

                    /* Check for correct Exon order. */

                    if((lastnewstrand >= 0) &&
                       (ensFeatureGetStart(newef) < lastnewstart))
                        orderbroken = ajTrue;

                    if((lastnewstrand < 0) &&
                       (ensFeatureGetStart(newef) > lastnewstart))
                        orderbroken = ajTrue;

                    minstart = (ensFeatureGetStart(newef) < minstart)
                        ? ensFeatureGetStart(newef) : minstart;

                    maxend   = (ensFeatureGetEnd(newef) > maxend)
                        ? ensFeatureGetEnd(newef) : maxend;
                }

                lastnewsr     = ensFeatureGetSeqregionObject(newef);
                lastnewstart  = ensFeatureGetStart(newef);
                lastnewstrand = ensFeatureGetStrand(newef);
                lastoldstrand = ensFeatureGetStrand(oldef);
            }

            if(newtranscript->Translation)
            {
                if(ensExonMatch(
                       oldexon,
                       ensTranslationGetStartexon(newtranscript->Translation)))
                    ensTranslationSetStartexon(newtranscript->Translation,
                                               newexon);

                if(ensExonMatch(
                       oldexon,
                       ensTranslationGetEndexon(newtranscript->Translation)))
                    ensTranslationSetEndexon(newtranscript->Translation,
                                             newexon);
            }

            if(newtranscript->Alternativetranslations)
            {
                iter2 = ajListIterNew(newtranscript->Alternativetranslations);

                while(!ajListIterDone(iter2))
                {
                    translation = (EnsPTranslation) ajListIterGet(iter2);

                    if(ensExonMatch(
                           oldexon,
                           ensTranslationGetStartexon(translation)))
                        ensTranslationSetStartexon(translation, newexon);

                    if(ensExonMatch(
                           oldexon,
                           ensTranslationGetEndexon(translation)))
                        ensTranslationSetEndexon(translation, newexon);
                }

                ajListIterDel(&iter2);
            }

            ensExonDel(&oldexon);
        }

        ajListIterDel(&iter1);

        if(orderbroken && (!ignoreorder))
        {
            ajWarn("ensTranscriptTransform got Ensembl Exon objects out of "
                   "order in the transformation of Ensembl Transcript with "
                   "identifier %u.\n",
                   transcript->Identifier);

            error = ajTrue;
        }

        if(error)
        {
            ensFeatureDel(&newtf);

            ensTranscriptDel(&newtranscript);

            return NULL;
        }
    }

    if(!newtf)
    {
        ajListPeekFirst(newtranscript->Exons, (void**) &newexon);

        newef = ensExonGetFeature(newexon);

        newtf = ensFeatureNewIniS(ensFeatureGetAnalysis(transcript->Feature),
                                  ensFeatureGetSlice(newef),
                                  minstart,
                                  maxend,
                                  ensFeatureGetStrand(newef));
    }

    /*
    ** Set the Feature directly, since ensTranscriptSetFeature transfers all
    ** internal Ensembl Objects based on the Feature class onto the new
    ** Ensembl Feature Slice, which duplicates the work already done here.
    */

    ensFeatureDel(&newtranscript->Feature);

    newtranscript->Feature = newtf;

    /*
    ** Transfer all Ensembl Base Align Feature objects onto the new
    ** Ensembl Feature Slice.
    */

    if(newtranscript->Supportingfeatures)
    {
        iter1 = ajListIterNew(newtranscript->Supportingfeatures);

        while(!ajListIterDone(iter1))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newbaf = ensBasealignfeatureTransform(oldbaf, csname, csversion);

            if(!newbaf)
            {
                ajDebug("ensTranscriptTransform could not transfer "
                        "Base Align Feature onto new Ensembl Feature Slice.");

                ensBasealignfeatureTrace(oldbaf, 1);
            }

            ajListIterInsert(iter1, (void*) newbaf);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            ensBasealignfeatureDel(&oldbaf);
        }

        ajListIterDel(&iter1);
    }

    /* Clear internal values that depend on Exon coordinates. */

    ensMapperClear(newtranscript->ExonCoordMapper);

    return newtranscript;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Fetch Fetch Ensembl Transcript values
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Constitutiveexons Fetch all constitutive Ensembl Exon objects
** @nam5rule Databaseentries Fetch all Ensembl Database Entry objects
** @nam5rule Introns Fetch all Ensembl Intron objects
** @nam5rule Sequenceedits Fetch all Ensembl Sequence Edit objects
** @nam4rule Displayidentifier Fetch the display identifier
** @nam4rule Sequence    Fetch the sequence
** @nam5rule Coding      Fetch the coding sequence
** @nam5rule Transcript  Fetch the Ensembl Transcript (cDNA) sequence
** @nam6rule Seq         Fetch as AJAX Sequence object
** @nam6rule Str         Fetch as AJAX String object
** @nam5rule Translation Fetch the Ensembl Translation sequence
** @nam6rule Seq         Fetch as AJAX Sequence object
** @nam6rule Str         Fetch as AJAX String object
**
** @argrule AllAttributes transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList] AJAX List of
** Ensembl Attribute objects
** @argrule AllConstitutiveexons transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllConstitutiveexons exons [AjPList] AJAX List of
** Ensembl Exon objects
** @argrule AllDatabaseentries transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllDatabaseentries name [const AjPStr]
** Ensembl External Database name
** @argrule AllDatabaseentries type [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllDatabaseentries dbes [AjPList] AJAX List of
** Ensembl Database Entry objects
** @argrule AllIntrons transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllIntrons introns [AjPList] AJAX List of Ensembl Intron objects
** @argrule AllSequenceedits transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllSequenceedits ses [AjPList] AJAX List of
** Ensembl Sequence Edit objects
** @argrule Displayidentifier transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule Displayidentifier Pidentifier [AjPStr*] AJAX String address
** @argrule Sequence transcript [EnsPTranscript] Ensembl Transcript
** @argrule SequenceCoding translation [EnsPTranslation] Ensembl Translation
** @argrule SequenceTranslation translation [EnsPTranslation]
** Ensembl Translation
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence address
** @argrule Str Psequence [AjPStr*] AJAX String address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensTranscriptFetchAllAttributes **************************************
**
** Fetch all Ensembl Attribute objects of an Ensembl Transcript and optionally
** filter via an Ensembl Attribute code. To get all Ensembl Attribute objects
** for this Ensembl Transcript, consider using ensTranscriptLoadAttributes.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Attributes
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
** @see ensTranscriptLoadAttributes
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

    list = ensTranscriptLoadAttributes(transcript);

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
                             (void*) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranscriptFetchAllConstitutiveexons *******************************
**
** Fetch all constitutive Ensembl Exon objects of an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Exon objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_constitutive_Exons
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllConstitutiveexons(EnsPTranscript transcript,
                                              AjPList exons)
{
    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPExon exon = NULL;

    if(!transcript)
        return ajFalse;

    if(!exons)
        return ajFalse;

    list = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if(ensExonGetConstitutive(exon))
            ajListPushAppend(exons, (void*) ensExonNewRef(exon));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranscriptFetchAllDatabaseentries *********************************
**
** Fetch all Ensembl Database Entry objects of an Ensembl Transcript and
** optionally filter via an Ensembl External Database name or type. To get all
** Ensembl External Database objects for this Ensembl Transcript, consider
** using ensTranscriptGetExternalDatabaseentries.
**
** The caller is responsible for deleting the Ensembl Database Entry objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [u] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry object
** @see ensTranscriptLoadDatabaseentries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllDatabaseentries(EnsPTranscript transcript,
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

    list = ensTranscriptLoadDatabaseentries(transcript);

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
            ajListPushAppend(dbes, (void*) ensDatabaseentryNewRef(dbe));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranscriptFetchAllIntrons *****************************************
**
** Fetch all Ensembl Intron objects of an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Intron objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Introns
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] introns [AjPList] AJAX List of Ensembl Intron objects
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

    list = ensTranscriptLoadExons(transcript);

    for(i = 0; i < (ajListGetLength(list) - 1); i++)
    {
        ajListPeekNumber(list, i, (void**) &exon1);

        ajListPeekNumber(list, i + 1, (void**) &exon2);

        intron = ensIntronNewIni(exon1, exon2);

        ajListPushAppend(introns, (void*) intron);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchAllSequenceedits ***********************************
**
** Fetch all Ensembl Sequence Edit objects of an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Sequence Edit objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Transcript::get_all_SeqEdits
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] ses [AjPList] AJAX List of Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Ensembl Sequence Edit objects are Ensembl Attribute objects, which codes
** have to be defined in the
** static const char* const transcriptSequenceeditCode[] array.
******************************************************************************/

AjBool ensTranscriptFetchAllSequenceedits(EnsPTranscript transcript,
                                          AjPList ses)
{
    register ajuint i = 0;

    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute at = NULL;

    EnsPSequenceedit se = NULL;

    if(!transcript)
        return ajFalse;

    if(!ses)
        return ajFalse;

    code = ajStrNew();

    attributes = ajListNew();

    for(i = 0; transcriptSequenceeditCode[i]; i++)
    {
        ajStrAssignC(&code, transcriptSequenceeditCode[i]);

        ensTranscriptFetchAllAttributes(transcript, code, attributes);
    }

    while(ajListPop(attributes, (void**) &at))
    {
        se = ensSequenceeditNewAttribute(at);

        ajListPushAppend(ses, (void*) se);

        ensAttributeDel(&at);
    }

    ajListFree(&attributes);

    ajStrDel(&code);

    return ajTrue;
}




/* @func ensTranscriptFetchDisplayidentifier **********************************
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

AjBool ensTranscriptFetchDisplayidentifier(const EnsPTranscript transcript,
                                           AjPStr* Pidentifier)
{
    if(!transcript)
        return ajFalse;

    if(!Pidentifier)
        return ajFalse;

    if(transcript->Stableidentifier &&
       ajStrGetLen(transcript->Stableidentifier))
        *Pidentifier = ajStrNewS(transcript->Stableidentifier);
    else if(transcript->Identifier)
        *Pidentifier = ajFmtStr("%u", transcript->Identifier);
    else
        *Pidentifier = ajFmtStr("%p", transcript);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceCodingStr **********************************
**
** Fetch the coding (translatable) sequence portion of an Ensembl Transcript as
** AJAX String. The sequence is padded with 'N's according to the start phase
** of the first coding Exon.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::translateable_seq
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceCodingStr(EnsPTranscript transcript,
                                           EnsPTranslation translation,
                                           AjPStr* Psequence)
{
    ajint sphase = 0;

    ajuint cdsstart = 0;
    ajuint cdsend = 0;

    AjPStr sequence = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /*
    ** Return an empty string for non-coding Ensembl Transcript objects.
    ** The ensTranscriptLoadTranslation function will attempt to load the
    ** Translation from the database.
    */

    if(!translation)
    {
        ajDebug("ensTranscriptFetchSequenceCodingStr got a Transcript "
                "without a Translation.\n");

        return ajTrue;
    }

    /* Calculate coding start and end coordinates. */

    cdsstart = ensTranscriptCalculateTranscriptCodingStart(transcript,
                                                           translation);

    if(!cdsstart)
    {
        ajDebug("ensTranscriptFetchSequenceCodingStr got a Transcript and "
                "Translation pair with an invalid cdsstart %u.\n", cdsstart);

        return ajTrue;
    }

    cdsend = ensTranscriptCalculateTranscriptCodingEnd(transcript,
                                                       translation);

    if(!cdsend)
    {
        ajDebug("ensTranscriptFetchSequenceCodingStr got a Transcript and "
                "Translation pair with an invalid cdsend %u.\n", cdsend);

        return ajTrue;
    }

    sphase = ensExonGetPhaseStart(ensTranslationGetStartexon(translation));

    if(sphase > 0)
        ajStrAppendCountK(Psequence, 'N', sphase);

    ensTranscriptFetchSequenceTranscriptStr(transcript, &sequence);

    ajStrAppendSubS(Psequence, sequence, cdsstart - 1, cdsend - 1);

    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceTranscriptSeq ******************************
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

AjBool ensTranscriptFetchSequenceTranscriptSeq(EnsPTranscript transcript,
                                               AjPSeq* Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    ensTranscriptFetchSequenceTranscriptStr(transcript, &sequence);

    ensTranscriptFetchDisplayidentifier(transcript, &name);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceTranscriptStr ******************************
**
** Fetch the spliced sequence of an Ensembl Transcript as AJAX String.
**
** The sequence of all Ensembl Exon objects is concatenated and by default, all
** post-transcriptional Sequence Edit objects are applied, but can be disabled
** by setting ensTranscriptSetSequenceedits to ajFalse.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::spliced_seq
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
** @see ensTranscriptSetSequenceedits
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranscriptStr(EnsPTranscript transcript,
                                               AjPStr* Psequence)
{
    AjIList iter        = NULL;
    const AjPList exons = NULL;
    AjPList ses         = NULL;

    AjPStr exseq = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    if(!transcript)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        ensExonFetchSequenceSliceStr(exon, &exseq);

        if(exseq && ajStrGetLen(exseq))
            ajStrAppendS(Psequence, exseq);
        else
        {
            ajDebug("ensTranscriptFetchSequenceTranscriptStr could not get "
                    "sequence for Exon %p. Transcript sequence may not be "
                    "correct.\n",
                    exon);

            ensExonTrace(exon, 1);

            feature = ensExonGetFeature(exon);

            ajStrAppendCountK(Psequence,
                              'N',
                              ensFeatureCalculateLength(feature));
        }

        ajStrDel(&exseq);
    }

    ajListIterDel(&iter);

    /* Apply post-transcriptional Sequence Edit objects if enabled. */

    if(transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        /*
        ** Sort Sequence Edit objects in reverse order to avoid the
        ** complication of adjusting down-stream Sequence Edit coordinates.
        */

        ensListSequenceeditSortStartDescending(ses);

        while(ajListPop(ses, (void**) &se))
        {
            ensSequenceeditApplyString(se, 0, Psequence);

            ensSequenceeditDel(&se);
        }

        ajListFree(&ses);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceTranslationSeq *****************************
**
** Fetch the sequence of the Ensembl Translation of an
** Ensembl Transcript as AJAX Sequence.
**
** The sequence is based on ensTranscriptFetchSequenceCodingStr and by
** default, all post-translational Sequence Edit objects are applied, but
** can be disabled by setting ensTranscriptSetSequenceedits to ajFalse.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Transcript::translate
** @cc Bio::EnsEMBL::Translation::modify_translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
** @see ensTranscriptFetchSequenceCodingStr
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranslationSeq(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPSeq* Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!transcript)
        return ajFalse;

    if(!translation)
        return ajTrue;

    if(!Psequence)
        return ajFalse;

    name     = ajStrNew();
    sequence = ajStrNew();

    ensTranslationFetchDisplayidentifier(translation, &name);

    ensTranscriptFetchSequenceTranslationStr(transcript,
                                             translation,
                                             &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetProt(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceTranslationStr *****************************
**
** Fetch the sequence of the Ensembl Translation of an
** Ensembl Transcript as AJAX String.
**
** The sequence is based on ensTranscriptFetchSequenceCodingStr and by
** default, all post-translational Sequence Edit objects are applied,
** but can be disabled by setting ensTranscriptSetSequenceedits to ajFalse.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::translate
** @cc Bio::EnsEMBL::Translation::modify_translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPStr*] AJAX String address
** @see ensTranscriptFetchSequenceCodingStr
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranslationStr(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPStr* Psequence)
{
    AjPList ses = NULL;

    AjPStr cdna = NULL;

    const AjPTrn trn = NULL;

    EnsPSequenceedit se = NULL;

    EnsPSlice slice = NULL;

    if(ajDebugTest("ensTranscriptFetchSequenceTranslationStr"))
        ajDebug("ensTranscriptFetchSequenceTranslationStr\n"
                "  transcript %p\n"
                "  translation %p\n"
                "  Psequence %p\n",
                transcript,
                translation,
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
    ** Return an empty string for non-coding Ensembl Transcript objects.
    ** The ensTranscriptLoadTranslation function will attempt to load the
    ** Translation from the database.
    */

    if(!translation)
    {
        ajDebug("ensTranscriptFetchSequenceTranslationStr got a Transcript "
                "without a Translation.\n");

        return ajTrue;
    }

    cdna = ajStrNew();

    ensTranscriptFetchSequenceCodingStr(transcript, translation, &cdna);

    if(ajStrGetLen(cdna) < 1)
        return ajTrue;

    slice = ensFeatureGetSlice(transcript->Feature);

    trn = ensSliceGetTranslation(slice);

    ajTrnSeqS(trn, cdna, Psequence);

    ajStrDel(&cdna);

    /*
    ** Remove the final stop codon from the mRNA if it is present, so that the
    ** resulting peptides do not end with a '*'. If a terminal stop codon is
    ** desired, call ensTranscriptFetchSequenceCodingStr and translate it
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

    /* Apply post-translational Sequence Edit objects if enabled. */

    if(transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranslationFetchAllSequenceedits(translation, ses);

        /*
        ** Sort Sequence Edit objects in reverse order to avoid the
        ** complication of adjusting down-stream Sequence Edit coordinates.
        */

        ensListSequenceeditSortStartDescending(ses);

        while(ajListPop(ses, (void**) &se))
        {
            ensSequenceeditApplyString(se, 0, Psequence);

            ensSequenceeditDel(&se);
        }

        ajListFree(&ses);
    }

    return ajTrue;
}




/* @section mapper ************************************************************
**
** Ensembl Transcript Mapper functions.
**
** @fdata [EnsPTranscript]
**
** @cc Bio::EnsEMBL::TranscriptMapper
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-62
**
** @nam3rule Mapper Ensembl Transcript Mapper functions
** @nam4rule Init          Initialise an Ensembl Transcript Mapper
** @nam4rule Slice         Map from Ensembl Slice coordinates
** @nam5rule Tocoding      Map to coding coordinates
** @nam5rule Totranscript  Map to transcript coordinates
** @nam5rule Totranslation Map to translation coordinates
** @nam4rule Transcript    Map from Ensembl Transcript coordinates
** @nam5rule Toslice       Map to Ensembl Slice coordinates
** @nam4rule Translation   Map from Ensembl Translation coordinates
** @nam5rule Toslice       Map to Ensembl Slice coordinates
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
** @argrule Init force [AjBool] force re-initialisation
** @argrule Tocoding translation [EnsPTranslation] Ensembl Translation
** @argrule Totranslation translation [EnsPTranslation] Ensembl Translation
** @argrule Translation translation [EnsPTranslation] Ensembl Translation
** @argrule Slice start  [ajint] Ensembl Slice start coordinate
** @argrule Slice end    [ajint] Ensembl Slice end coordinate
** @argrule Slice strand [ajint] Ensembl Slice strand information
** @argrule Slice mrs [AjPList] AJAX List of Ensembl Mapper Result objects
** @argrule TranscriptToslice start [ajuint] Transcript start coordinate
** @argrule TranscriptToslice end   [ajuint] Transcript end coordinate
** @argrule TranscriptToslice mrs   [AjPList] AJAX List of
**                                            Ensembl Mapper Result objects
** @argrule TranslationToslice start [ajuint] Translation start coordinate
** @argrule TranslationToslice end   [ajuint] Translation end coordinate
** @argrule TranslationToslice mrs   [AjPList] AJAX List of
**                                             Ensembl Mapper Result objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory new
******************************************************************************/




/* @func ensTranscriptMapperInit **********************************************
**
** Initialise or re-initialise an Ensembl Transcript Mapper of an
** Ensembl Transcript.
**
** If the force parameter is set to ajTrue the Ensembl Transcript Mapper will
** be re-initialised regardless whetner it has been initialised before,
** i.e. if it already contains Ensembl Mapper Pair objects.
**
** @cc Bio::EnsEMBL::TranscriptMapper::new
** @cc Bio::EnsEMBL::TranscriptMapper::_load_mapper
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] force [AjBool] force re-initialisation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Initialises a Transcript Mapper object which can be used to perform
** various coordinate transformations relating to Ensembl Transcript objects.
** Since the Transcript Mapper uses the Transcript state at the time of
** initialisation to perform the conversions, it must be re-initialised if the
** underlying Transcript is altered.
** All 'Genomic' coordinates in the Perl API are in fact relative to the
** Slice on which the particular Transcript is annotated.
******************************************************************************/

AjBool ensTranscriptMapperInit(EnsPTranscript transcript, AjBool force)
{
    ajuint srid = 0;

    /* Current and previous Feature (Exon) Slice coordinates */

    ajint curftrstart  = 0;
    ajint curftrend    = 0;
    ajint curftrstrand = 0;

    ajint prvftrstart = 0;
    ajint prvftrend   = 0;

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

    EnsPSequenceedit se = NULL;

    EnsPSlice slice = NULL;

    if(ajDebugTest("ensTranscriptMapperInit"))
        ajDebug("ensTranscriptMapperInit\n"
                "  transcript %p\n",
                transcript);

    if(!transcript)
        return ajFalse;

    if(transcript->ExonCoordMapper)
    {
        if(force == ajTrue)
            ensMapperClear(transcript->ExonCoordMapper);

        if(ensMapperGetCount(transcript->ExonCoordMapper) > 0)
            return ajTrue;
    }
    else
    {
        src = ajStrNewC("transcript");
        trg = ajStrNewC("slice");

        cs = ensSliceGetCoordsystemObject(
            ensFeatureGetSlice(transcript->Feature));

        transcript->ExonCoordMapper = ensMapperNewIni(src, trg, cs, cs);

        ajStrDel(&src);
        ajStrDel(&trg);
    }

    /* Get the Sequence Region Identifier for this Transcript. */

    feature = ensTranscriptGetFeature(transcript);

    slice = ensFeatureGetSlice(feature);

    srid = ensSliceGetSeqregionIdentifier(slice);

    /* Get all Ensembl Exon objects of this Ensembl Transcript. */

    exons = ensTranscriptLoadExons(transcript);

    /* Load Mapper Bio::EnsEMBL::TranscriptMapper::_load_mapper */

    ses = ajListNew();

    if(transcript->Sequenceedits)
    {
        ensTranscriptFetchAllSequenceedits(transcript, ses);

        ensListSequenceeditSortStartAscending(ses);
    }

    iter = ajListIterNewread(exons);

    while(!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        feature = ensExonGetFeature(exon);

        curftrstart  = ensFeatureGetStart(feature);
        curftrend    = ensFeatureGetEnd(feature);
        curftrstrand = ensFeatureGetStrand(feature);

        curtrcstart = curtrcend   + 1;
        curtrcend   = curtrcstart + ensFeatureCalculateLength(feature) - 1;

        /*
        ** Add deletions and insertions into Mapper Pair objects when
        ** Sequence Edit objects are applied and ignore mismatches,
        ** i.e. treat them as matches.
        */

        if(transcript->Sequenceedits)
        {
            while(ajListPeekFirst(ses, (void**) &se) &&
                  (ensSequenceeditGetStart(se) + editshift <= curtrcend))
            {
                if(ensSequenceeditCalculateDifference(se))
                {
                    /*
                    ** Break the Mapper Pair into two parts, finish the
                    ** first Mapper Pair just before the Sequence Edit.
                    */

                    prvtrcend    = ensSequenceeditGetStart(se) + editshift - 1;
                    prvtrcstart  = curtrcstart;
                    prvtrclength = prvtrcend - prvtrcstart + 1;

                    if(curftrstrand >= 0)
                    {
                        prvftrstart = curftrstart;
                        prvftrend   = curftrstart + prvtrclength - 1;
                    }
                    else
                    {
                        prvftrstart = curftrend - prvtrclength + 1;
                        prvftrend   = curftrend;
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
                        curftrstart = prvftrend   + 1;
                    else
                        curftrend   = prvftrstart - 1;

                    curtrcend += ensSequenceeditCalculateDifference(se);

                    if(ensSequenceeditCalculateDifference(se) >= 0)
                    {
                        /*
                        ** Positive length difference means insertion into
                        ** Transcript.
                        ** Shift Transcript coordinates along.
                        */

                        curtrcstart += ensSequenceeditCalculateDifference(se);
                    }
                    else
                    {
                        /*
                        ** Negative length difference means deletion from
                        ** Transcript and insertion into Slice.
                        ** Shift Slice coordinates along.
                        */

                        if(curftrstrand >= 0)
                            curftrstart
                                -= ensSequenceeditCalculateDifference(se);
                        else
                            curftrend
                                += ensSequenceeditCalculateDifference(se);
                    }

                    editshift += ensSequenceeditCalculateDifference(se);
                }

                /* At this stage remove the Sequence Edit from the List. */

                ajListPop(ses, (void**) &se);

                ensSequenceeditDel(&se);
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

    /*
    ** Delete any remaining Sequence Edit objects before deleting the
    ** AJAX List.
    */

    while(ajListPop(ses, (void**) &se))
        ensSequenceeditDel(&se);

    ajListFree(&ses);

    return ajTrue;
}




/* @func ensTranscriptMapperSliceTocoding *************************************
**
** Map Slice (genome) coordinates of an Ensembl Transcript into coding
** sequence coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::genomic2cds
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [r] start  [ajint] Ensembl Slice start coordinate
** @param [r] end    [ajint] Ensembl Slice end coordinate
** @param [r] strand [ajint] Ensembl Slice strand information
** @param [u] mrs    [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptMapperSliceTocoding(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        ajint start,
                                        ajint end,
                                        ajint strand,
                                        AjPList mrs)
{
    ajuint cdsstart = 0;
    ajuint cdsend   = 0;
    ajuint tcstart  = 0;
    ajuint tcend    = 0;

    AjPList result = NULL;

    EnsPMapperresult mr     = NULL;
    EnsPMapperresult gcmr   = NULL;
    EnsPMapperresult endgap = NULL;

    if(!transcript)
        return ajFalse;

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperSliceTocoding "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    if(start > (end + 1))
        ajFatal("ensTranscriptMapperSliceTocoding requires start %d to be "
                "less than end %d + 1.\n", start, end);

    /*
    ** For non-coding Ensembl Transcript objects return an
    ** Ensembl Mapper Result of type ensEMapperresultTypeGap,
    ** as there is no coding sequence.
    */

    if(!translation)
    {
        mr = ensMapperresultNewGap(start, end, 0);

        ajListPushAppend(mrs, (void*) mr);

        return ajTrue;
    }

    tcstart = ensTranscriptCalculateTranscriptCodingStart(transcript,
                                                          translation);

    tcend   = ensTranscriptCalculateTranscriptCodingEnd(transcript,
                                                        translation);

    result = ajListNew();

    ensTranscriptMapperSliceTotranscript(transcript,
                                         start,
                                         end,
                                         strand,
                                         result);

    while(ajListPop(result, (void**) &gcmr))
    {
        if(ensMapperresultGetType(gcmr) == ensEMapperresultTypeGap)
            ajListPushAppend(mrs, (void*) gcmr);
        else
        {
            if((ensMapperresultGetCoordinateStrand(gcmr) < 0) ||
               (ensMapperresultGetCoordinateEnd(gcmr)    < (ajint) tcstart) ||
               (ensMapperresultGetCoordinateStart(gcmr)  > (ajint) tcend))
            {
                /* All gap - does not map to peptide. */

                mr = ensMapperresultNewGap(start, end, 0);

                ajListPushAppend(mrs, (void*) mr);
            }
            else
            {
                /* We know area is at least partially overlapping CDS. */

                cdsstart = ensMapperresultGetCoordinateStart(gcmr)
                    - tcstart + 1;

                cdsend   = ensMapperresultGetCoordinateEnd(gcmr)
                    - tcstart + 1;

                if(ensMapperresultGetCoordinateStart(gcmr) < (ajint) tcstart)
                {
                    /* Start coordinate is in the 5' UTR. */

                    mr = ensMapperresultNewGap(
                        ensMapperresultGetCoordinateStart(gcmr),
                        tcstart - 1,
                        0);

                    ajListPushAppend(mrs, (void*) mr);

                    /* Start is now relative to start of CDS. */

                    cdsstart = 1;
                }

                endgap = NULL;

                if(ensMapperresultGetCoordinateEnd(gcmr) > (ajint) tcend)
                {
                    /* End coordinate is in the 3' UTR. */

                    endgap = ensMapperresultNewGap(
                        tcend + 1,
                        ensMapperresultGetCoordinateEnd(gcmr),
                        0);

                    /* Adjust end coordinate relative to CDS start. */

                    cdsend = tcend - tcstart + 1;
                }

                /*
                ** Start and end are now entirely in CDS and relative
                ** to CDS start.
                */

                mr = ensMapperresultNewCoordinate(
                    ensMapperresultGetObjectidentifier(gcmr),
                    cdsstart,
                    cdsend,
                    ensMapperresultGetCoordinateStrand(gcmr),
                    ensMapperresultGetCoordsystem(gcmr),
                    0);

                ajListPushAppend(mrs, (void*) mr);

                if(endgap)
                    ajListPushAppend(mrs, (void*) endgap);
            }

            ensMapperresultDel(&gcmr);
        }
    }

    ajListFree(&result);

    return ajTrue;
}




/* @func ensTranscriptMapperSliceTotranscript *********************************
**
** Map Slice (genome) coordinates of an Ensembl Transcript into
** Transcript (cDNA) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::genomic2cdna
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start [ajint] Ensembl Slice start coordinate
** @param [r] end [ajint] Ensembl Slice end coordinate
** @param [r] strand [ajint] Ensembl Slice strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Converts Slice coordinates to Transcript coordinates. The return value is an
** AJAX List of Mapper Result coordinates and gaps. Gaps represent intronic
** or upstream/downstream regions which do not comprise this transcript's
** cDNA. Mapper Result Coordinate objects represent Slice regions which map
** to Ensembl Exon objects (UTRs included).
******************************************************************************/

AjBool ensTranscriptMapperSliceTotranscript(EnsPTranscript transcript,
                                            ajint start,
                                            ajint end,
                                            ajint strand,
                                            AjPList mrs)
{
    ajuint srid = 0;

    AjPStr src = NULL;

    EnsPSlice slice = NULL;

    if(!transcript)
        return ajFalse;

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperSliceTotranscript "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    ensTranscriptMapperInit(transcript, ajFalse);

    slice = ensFeatureGetSlice(transcript->Feature);

    srid = ensSliceGetSeqregionIdentifier(slice);

    src = ajStrNewC("slice");

    ensMapperMap(transcript->ExonCoordMapper,
                 srid,
                 start,
                 end,
                 strand,
                 src,
                 mrs);

    ajStrDel(&src);

    return ajTrue;
}




/* @func ensTranscriptMapperSliceTotranslation ********************************
**
** Map Slice (genome) coordinates of an Ensembl Transcript into
** Translation (peptide) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::genomic2pep
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [r] start [ajint] Ensembl Slice start coordinate
** @param [r] end [ajint] Ensembl Slice end coordinate
** @param [r] strand [ajint] Ensembl Slice strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptMapperSliceTotranslation(EnsPTranscript transcript,
                                             EnsPTranslation translation,
                                             ajint start,
                                             ajint end,
                                             ajint strand,
                                             AjPList mrs)
{
    ajint tlstart = 0;
    ajint tlend   = 0;
    ajint shift   = 0;

    AjPList result = NULL;

    EnsPMapperresult mr   = NULL;
    EnsPMapperresult gcmr = NULL;

    if(!transcript)
        return ajFalse;

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperSliceTotranslation "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    result = ajListNew();

    ensTranscriptMapperSliceTocoding(transcript,
                                     translation,
                                     start,
                                     end,
                                     strand,
                                     result);

    /* Take possible N padding at beginning of CDS. */

    shift = ensTranscriptCalculatePhaseStart(transcript);
    shift = (shift >= 0) ? shift : 0;

    while(ajListPop(result, (void**) &gcmr))
    {
        if(ensMapperresultGetType(gcmr) == ensEMapperresultTypeGap)
            ajListPushAppend(mrs, (void*) gcmr);
        else
        {
            /*
            ** Start and end coordinates are now entirely in CDS and
            ** relative to CDS start.
            ** Convert to peptide coordinates.
            */

            tlstart = (ensMapperresultGetCoordinateStart(gcmr) + shift + 2)
                / 3;

            tlend =   (ensMapperresultGetCoordinateEnd(gcmr)   + shift + 2)
                / 3;

            mr = ensMapperresultNewCoordinate(
                ensMapperresultGetObjectidentifier(gcmr),
                tlstart,
                tlend,
                ensMapperresultGetCoordinateStrand(gcmr),
                ensMapperresultGetCoordsystem(gcmr),
                0);

            ajListPushAppend(mrs, (void*) mr);

            ensMapperresultDel(&gcmr);
        }
    }

    ajListFree(&result);

    return ajTrue;
}




/* @func ensTranscriptMapperTranscriptToslice *********************************
**
** Map Transcript (cDNA) coordinates of an Ensembl Transcript into Slice
** (genome) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::cdna2genomic
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] start [ajuint] Ensembl Transcript start coordinate
** @param [r] end [ajuint] Ensembl Transcript end coordinate
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptMapperTranscriptToslice(EnsPTranscript transcript,
                                            ajuint start,
                                            ajuint end,
                                            AjPList mrs)
{
    AjPStr src = NULL;

    if(!transcript)
        return ajFalse;

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperTranscriptToslice "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    ensTranscriptMapperInit(transcript, ajFalse);

    src = ajStrNewC("transcript");

    ensMapperMap(transcript->ExonCoordMapper,
                 transcript->Identifier,
                 start,
                 end,
                 1,
                 src,
                 mrs);

    ajStrDel(&src);

    return ajTrue;
}




/* @func ensTranscriptMapperTranslationToslice ********************************
**
** Map Translation (Peptide) coordinates of an Ensembl Transcript into Slice
** (genome) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::pep2genomic
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [r] start [ajuint] Ensembl Translation start coordinate
** @param [r] end [ajuint] Ensembl Translation end coordinate
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Converts Translation coordinates into Slice coordinates. The Mapper Result
** Coordinates returned are relative to the same slice that the Transcript
** used to initialise this Transcript Mapper was on.
******************************************************************************/

AjBool ensTranscriptMapperTranslationToslice(EnsPTranscript transcript,
                                             EnsPTranslation translation,
                                             ajuint start,
                                             ajuint end,
                                             AjPList mrs)
{
    ajint tcstart = 0;
    ajint shift   = 0;

    if(!transcript)
        return ajFalse;

    if(!mrs)
    {
        ajDebug("ensTranscriptMapperTranslationToslice "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    tcstart = ensTranscriptCalculateTranscriptCodingStart(transcript,
                                                          translation);

    /* Take possible N-padding at the beginning of the CDS into account. */

    shift = ensTranscriptCalculatePhaseStart(transcript);
    shift = (shift >= 0) ? shift : 0;

    /* Move start and end into cDNA coordinates. */

    start = 3 * start - 2 + (tcstart - 1) - shift;
    end   = 3 * end       + (tcstart - 1) - shift;

    return ensTranscriptMapperTranscriptToslice(transcript, start, end, mrs);
}




/* @datasection [EnsETranscriptStatus] Ensembl Transcript Status **************
**
** @nam2rule Transcript Functions for manipulating
** Ensembl Transcript objects
** @nam3rule TranscriptStatus Functions for manipulating
** Ensembl Transcript Status enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Transcript Status enumeration.
**
** @fdata [EnsETranscriptStatus]
**
** @nam4rule From Ensembl Transcript Status query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  status  [const AjPStr] Status string
**
** @valrule * [EnsETranscriptStatus] Ensembl Transcript Status enumeration or
**                                   ensETranscriptStatusNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensTranscriptStatusFromStr *******************************************
**
** Return an Ensembl Transcript Status enumeration from an AJAX String.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsETranscriptStatus] Ensembl Transcript Status enumeration or
**                                ensETranscriptStatusNULL
** @@
******************************************************************************/

EnsETranscriptStatus ensTranscriptStatusFromStr(const AjPStr status)
{
    register EnsETranscriptStatus i = ensETranscriptStatusNULL;

    EnsETranscriptStatus estatus = ensETranscriptStatusNULL;

    for(i = ensETranscriptStatusNULL;
        transcriptStatus[i];
        i++)
        if(ajStrMatchC(status, transcriptStatus[i]))
            estatus = i;

    if(!estatus)
        ajDebug("ensTranscriptStatusFromStr encountered "
                "unexpected string '%S'.\n", status);

    return estatus;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Transcript Status enumeration.
**
** @fdata [EnsETranscriptStatus]
**
** @nam4rule To   Return Ensembl Transcript Status enumeration
** @nam5rule Char Return C character string value
**
** @argrule To status [EnsETranscriptStatus]
** Ensembl Transcript Status enumeration
**
** @valrule Char [const char*] Status
**
** @fcategory cast
******************************************************************************/




/* @func ensTranscriptStatusToChar ********************************************
**
** Cast an Ensembl Transcript Status enumeration into a C-type (char*) string.
**
** @param [u] status [EnsETranscriptStatus]
** Ensembl Transcript Status enumeration
**
** @return [const char*] Ensembl Transcript status C-type (char*) string
** @@
******************************************************************************/

const char* ensTranscriptStatusToChar(EnsETranscriptStatus status)
{
    register EnsETranscriptStatus i = ensETranscriptStatusNULL;

    for(i = ensETranscriptStatusNULL;
        transcriptStatus[i] && (i < status);
        i++);

    if(!transcriptStatus[i])
        ajDebug("ensTranscriptStatusToChar encountered an "
                "out of boundary error on "
                "Ensembl Transcript Status enumeration %d.\n",
                status);

    return transcriptStatus[i];
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Transcript Functions for manipulating AJAX List objects of
** Ensembl Transcript objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending transcripts [AjPList] AJAX List of Ensembl Transcript
**                                          objects
** @argrule Descending transcripts [AjPList] AJAX List of Ensembl Transcript
**                                           objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listTranscriptCompareStartAscending ****************************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** Ensembl Feature start element in ascending order.
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

static int listTranscriptCompareStartAscending(const void* P1,
                                               const void* P2)
{
    const EnsPTranscript transcript1 = NULL;
    const EnsPTranscript transcript2 = NULL;

    transcript1 = *(EnsPTranscript const*) P1;
    transcript2 = *(EnsPTranscript const*) P2;

    if(ajDebugTest("listTranscriptCompareStartAscending"))
        ajDebug("listTranscriptCompareStartAscending\n"
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




/* @func ensListTranscriptSortStartAscending **********************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** Ensembl Feature start element in ascending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListTranscriptSortStartAscending(AjPList transcripts)
{
    if(!transcripts)
        return ajFalse;

    ajListSort(transcripts, listTranscriptCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listTranscriptCompareStartDescending ***************************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** Ensembl Feature start element in descending order.
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

static int listTranscriptCompareStartDescending(const void* P1,
                                                const void* P2)
{
    const EnsPTranscript transcript1 = NULL;
    const EnsPTranscript transcript2 = NULL;

    transcript1 = *(EnsPTranscript const*) P1;
    transcript2 = *(EnsPTranscript const*) P2;

    if(ajDebugTest("listTranscriptCompareStartDescending"))
        ajDebug("listTranscriptCompareStartDescending\n"
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




/* @func ensListTranscriptSortStartDescending *********************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** Ensembl Feature start element in descending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListTranscriptSortStartDescending(AjPList transcripts)
{
    if(!transcripts)
        return ajFalse;

    ajListSort(transcripts, listTranscriptCompareStartDescending);

    return ajTrue;
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Table Functions for manipulating AJAX Table objects
**
******************************************************************************/




/* @section table *************************************************************
**
** Functions for processing AJAX Table objects of
** Ensembl Transcript objects.
**
** @fdata [AjPTable]
**
** @nam3rule Transcript AJAX Table of AJAX unsigned integer key data and
**                      Ensembl Transcript value data
** @nam4rule Clear Clear an AJAX Table
** @nam4rule Delete Delete an AJAX Table
**
** @argrule Clear table [AjPTable] AJAX Table
** @argrule Delete Ptable [AjPTable*] AJAX Table address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic tableTranscriptClear *******************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Transcript value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Transcript address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableTranscriptClear(void** key,
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

    AJFREE(*key);

    ensTranscriptDel((EnsPTranscript*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableTranscriptClear **********************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Transcript value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableTranscriptClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableTranscriptClear, NULL);

    return ajTrue;
}




/* @func ensTableTranscriptDelete *********************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Transcript value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableTranscriptDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableTranscriptClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [EnsPDatabaseadaptor] Ensembl Supporting Feature Adaptor ******
**
** @nam2rule Supportingfeatureadaptor Functions for manipulating
** Ensembl Supporting Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SupportingFeatureAdaptor
** @cc CVS Revision: 1.21
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Base Align Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Fetch Fetch Ensembl Base Align Feature object(s)
** @nam4rule FetchAll Fetch all Ensembl Base Align Feature objects
** @nam4rule FetchAllby Fetch all Ensembl Base Align Feature objects
**                      matching a criterion
** @nam5rule Transcript Fetch all Ensembl Base Align Feature objects matching
**                      an Ensembl Transcript
** @nam4rule FetchBy Fetch one Ensembl Base Align Feature object
**                   matching a criterion
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule AllbyTranscript transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllbyTranscript bafs [AjPList] AJAX List of Ensembl Base Align
**                                         Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSupportingfeatureadaptorFetchAllbyTranscript **********************
**
** Fetch Ensembl Supporting Feature objects via an Ensembl Transcript.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSupportingfeatureadaptorFetchAllbyTranscript(
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

    EnsPDnaalignfeatureadaptor dafa = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if(!dba)
        return ajFalse;

    if(!transcript)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(!ensTranscriptGetIdentifier(transcript))
    {
        ajDebug("ensSupportingfeatureadaptorFetchAllbyTranscript cannot get "
                "Ensembl Supporting Feature objects for an Ensembl Transcript "
                "without an identifier.\n");

        return ajFalse;
    }

    tfeature = ensTranscriptGetFeature(transcript);

    tslice = ensFeatureGetSlice(tfeature);

    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);

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
            ensDnaalignfeatureadaptorFetchByIdentifier(dafa,
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
            ajWarn("ensSupportingfeatureadaptorFetchAllbyTranscript got "
                   "unexpected value in "
                   "transcript_supporting_feature.feature_type '%S'.\n", type);

        if(baf)
        {
            ofeature = ensFeaturepairGetSourceFeature(baf->Featurepair);

            nfeature = ensFeatureTransfer(ofeature, tslice);

            ensFeaturepairSetSourceFeature(baf->Featurepair, nfeature);

            ensFeatureDel(&nfeature);

            ajListPushAppend(bafs, (void*) baf);
        }
        else
        {
            ajDebug("ensSupportingfeatureadaptorFetchAllbyTranscript could "
                    "not fetch Supporting feature of type '%S' and "
                    "identifier %u from database.\n", type, identifier);
        }

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPTranscriptadaptor] Ensembl Transcript Adaptor ************
**
** @nam2rule Transcriptadaptor Functions for manipulating
** Ensembl Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor
** @cc CVS Revision: 1.135
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic transcriptadaptorFetchAllbyStatement ***************************
**
** Fetch all Ensembl Transcript objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool transcriptadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
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
    ajint tmpstart = 0;

    ajuint version = 0;

    AjBool circular = AJFALSE;
    AjBool current  = AJFALSE;

    EnsETranscriptStatus estatus =
        ensETranscriptStatusNULL;

    EnsEExternalreferenceInfotype erit = ensEExternalreferenceInfotypeNULL;

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

    if(ajDebugTest("transcriptadaptorFetchAllbyStatement"))
        ajDebug("transcriptadaptorFetchAllbyStatement\n"
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

        estatus = ensETranscriptStatusNULL;
        erit    = ensEExternalreferenceInfotypeNULL;

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

        srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa, srid);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Ensembl Feature objects,
        ** the range needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("transcriptadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("transcriptadaptorFetchAllbyStatement got a "
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
                              ensSliceGetCoordsystemObject(slice),
                              ensSliceGetCoordsystemObject(srslice))))
            ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice, &am);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper is defined at this point.
        */

        if(am)
        {
            ensAssemblymapperFastmap(am,
                                     ensSliceGetSeqregion(srslice),
                                     slstart,
                                     slend,
                                     slstrand,
                                     mrs);

            /*
            ** The ensAssemblymapperFastmap function returns at best one
            ** Ensembl Mapper Result.
            */

            ajListPop(mrs, (void**) &mr);

            /*
            ** Skip Ensembl Feature objects that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
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

            srid     = ensMapperresultGetObjectidentifier(mr);
            slstart  = ensMapperresultGetCoordinateStart(mr);
            slend    = ensMapperresultGetCoordinateEnd(mr);
            slstrand = ensMapperresultGetCoordinateStrand(mr);

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

            if(ensSliceCalculateLength(slice) <= INT_MAX)
                sllength = (ajint) ensSliceCalculateLength(slice);
            else
                ajFatal("transcriptadaptorFetchAllbyStatement got a Slice, "
                        "which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceCalculateLength(slice), INT_MAX);

            ensSliceIsCircular(slice, &circular);

            if(ensSliceGetStrand(slice) >= 0)
            {
                /* On the positive strand ... */

                slstart = slstart - ensSliceGetStart(slice) + 1;
                slend   = slend   - ensSliceGetStart(slice) + 1;

                if(circular == ajTrue)
                {
                    if(slstart > slend)
                    {
                        /* A Feature overlapping the chromsome origin. */

                        /* Region in the beginning of the chromosome. */
                        if(slend > ensSliceGetStart(slice))
                            slstart -= sllength;

                        if(slend < 0)
                            slend += sllength;
                    }
                    else
                    {
                        if((ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                           && (slend < 0))
                        {
                            /*
                            ** A region overlapping the chromosome origin
                            ** and a Feature, which is at the beginning of
                            ** the chromosome.
                            */

                            slstart += sllength;
                            slend   += sllength;
                        }
                    }
                }
            }
            else
            {
                /* On the negative strand ... */

                if((circular == ajTrue) && (slstart > slend))
                {
                    /* Handle circular chromosomes. */

                    if(ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                    {
                        tmpstart = slstart;
                        slstart = ensSliceGetEnd(slice) - slend + 1;
                        slend   = ensSliceGetEnd(slice) + sllength
                            - tmpstart + 1;
                    }
                    else
                    {

                        if(slend > ensSliceGetStart(slice))
                        {
                            /*
                            ** Looking at the region in the beginning of the
                            ** chromosome.
                            */

                            slstart = ensSliceGetEnd(slice) - slend + 1;
                            slend   = slend - sllength - slstart + 1;
                        }
                        else
                        {
                            tmpstart = slstart;
                            slstart  = ensSliceGetEnd(slice) - slend
                                - sllength + 1;
                            slend    = slend - tmpstart + 1;
                        }
                    }
                }
                else
                {
                    /* Non-circular Ensembl Slice objects... */

                    slend   = ensSliceGetEnd(slice) - slstart + 1;
                    slstart = ensSliceGetEnd(slice) - slend   + 1;
                }

                slstrand *= -1;
            }

            /*
            ** Throw away Ensembl Feature objects off the end of the requested
            ** Slice or on any other than the requested Slice.
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

            erit = ensExternalreferenceInfotypeFromStr(erinfotype);

            if(!erit)
                ajDebug("transcriptadaptorFetchAllbyStatement encountered "
                        "unexpected string '%S' in the "
                        "'xref.infotype' field.\n", erinfotype);

            dbe = ensDatabaseentryNewIni(
                (EnsPDatabaseentryadaptor) NULL,
                erid,
                (EnsPAnalysis) NULL,
                edb,
                erprimaryid,
                erdisplayid,
                erversion,
                erdescription,
                (AjPStr) NULL,
                erinfotext,
                erit,
                ensEExternalreferenceObjectypeTranscript,
                identifier);

            ensExternaldatabaseDel(&edb);
        }
        else
            dbe = NULL;

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        /* Set the Transcript status. */

        estatus = ensTranscriptStatusFromStr(status);

        if(!estatus)
            ajFatal("transcriptadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'transcript.status' field.\n", status);

        /* Finally, create a new Ensembl Transcript. */

        feature = ensFeatureNewIniS(analysis,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

        transcript = ensTranscriptNewIni(tca,
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

        ajListPushAppend(transcripts, (void*) transcript);

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

static void* transcriptadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensTranscriptNewRef((EnsPTranscript) value);
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

static void transcriptadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensTranscriptDel((EnsPTranscript*) value);

    return;
}




/* @funcstatic transcriptadaptorCacheSize *************************************
**
** Wrapper function to determine the memory size of an Ensembl Transcript
** via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Transcript
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t transcriptadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensTranscriptCalculateMemsize((const EnsPTranscript) value);
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

static EnsPFeature transcriptadaptorGetFeature(const void* value)
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
** Default constructor for an Ensembl Transcript Adaptor.
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        transcriptadaptorTables,
        transcriptadaptorColumns,
        transcriptadaptorLeftjoin,
        (const char*) NULL,
        (const char*) NULL,
        transcriptadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        transcriptadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        transcriptadaptorCacheDelete,
        transcriptadaptorCacheSize,
        transcriptadaptorGetFeature,
        "Transcript");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Transcript Adaptor object.
**
** @fdata [EnsPTranscriptadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Transcript Adaptor object
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

void ensTranscriptadaptorDel(EnsPTranscriptadaptor* Ptca)
{
    if(!Ptca)
        return;

    if(!*Ptca)
        return;

    ensFeatureadaptorDel(Ptca);

    *Ptca = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Transcript Adaptor object.
**
** @fdata [EnsPTranscriptadaptor]
**
** @nam3rule Get Return Ensembl Transcript Adaptor attribute(s)
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @valrule Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Transcript Adaptor.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensTranscriptadaptorGetDatabaseadaptor(
    EnsPTranscriptadaptor tca)
{
    if(!tca)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(tca);
}




/* @func ensTranscriptadaptorGetFeatureadaptor ********************************
**
** Get the Ensembl Feature Adaptor element of an Ensembl Transcript Adaptor.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPFeatureadaptor ensTranscriptadaptorGetFeatureadaptor(
    EnsPTranscriptadaptor tca)
{
    if(!tca)
        return NULL;

    return tca;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Transcript objects from an
** Ensembl SQL database.
**
** @fdata [EnsPTranscriptadaptor]
**
** @nam3rule Fetch Fetch Ensembl Transcript object(s)
** @nam4rule All   Fetch all Ensembl Transcript objects
** @nam4rule Allby Fetch all Ensembl Transcript objects matching a criterion
** @nam5rule Gene  Fetch all by an Ensembl Gene
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable Ensembl Transcript
** identifier
** @nam4rule By Fetch one Ensembl Transcript object matching a criterion
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Exonidentifier Fetch by an Ensembl Exon identifier
** @nam5rule Exonstableidentifier Fetch by an Ensembl Exon stable identifier
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by stable Ensembl Transcript identifier
** @nam5rule Translationidentifier Fetch by an Ensembl Translation identifier
** @nam5rule Translationstableidentifier Fetch by an Ensembl Translation
** stable identifier
**
** @argrule * tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule All transcripts [AjPList] AJAX List of Ensembl Transcript objects
** @argrule AllbyGene gene [EnsPGene] Ensembl Gene
** @argrule AllbyGene transcripts [AjPList] AJAX List of
** Ensembl Transcript objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice loadexons [AjBool] Load Ensembl Exon objects
** @argrule AllbySlice transcripts [AjPList] AJAX List of
** Ensembl Transcript objects
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule AllbyStableidentifier transcripts [AjPList] AJAX List of
** Ensembl Transcript objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByDisplaylabel Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule ByExonidentifier identifier [ajuint] Ensembl Exon identifier
** @argrule ByExonidentifier Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule ByExonstableidentifier stableid [const AjPStr]
** Ensembl Exon stable identifier
** @argrule ByExonstableidentifier Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule ByStableidentifier Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule ByTranslationidentifier identifier [ajuint]
** Ensembl Translation identifier
** @argrule ByTranslationidentifier Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule ByTranslationstableidentifier stableid [const AjPStr]
** Ensembl Translation stable identifier
** @argrule ByTranslationstableidentifier Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptadaptorFetchAll *****************************************
**
** Fetch all Ensembl Transcript objects.
**
** The caller is responsible for deleting the Ensembl Transcript objects before
** deleting the AJAX List.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAll(
    EnsPTranscriptadaptor tca,
    AjPList transcripts)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    constraint = ajStrNewC(
        "transcript.biotype != 'LRG_gene' "
        "AND "
        "transcript.is_current = 1");

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                transcripts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranscriptadaptorFetchAllbyGene ***********************************
**
** Fetch all Ensembl Transcript objects via an Ensembl Gene.
**
** The caller is responsible for deleting the Ensembl Transcript objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_Gene
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyGene(
    EnsPTranscriptadaptor tca,
    EnsPGene gene,
    AjPList transcripts)
{
    AjBool circular = AJFALSE;

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
    ** Get a Slice that entirely overlaps the gene. This is because we
    ** want all Transcript objects to be fetched, not just ones overlapping
    ** the Slice the Gene is on, as the Gene may only partially overlap the
    ** Slice. For speed reasons, only use a different Slice if necessary
    ** though.
    */

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    if(!dba)
    {
        ajWarn("ensTranscriptadaptorFetchAllbyGene cannot fetch "
               "Ensembl Transcript objects without an "
               "Ensembl Database Adaptor defined in the "
               "Ensembl Transcript Adaptor.\n");

        return ajFalse;
    }

    gfeature = ensGeneGetFeature(gene);

    if(!gfeature)
    {
        ajWarn("ensTranscriptadaptorFetchAllbyGene cannot fetch "
               "Ensembl Transcript objects without an "
               "Ensembl Feature defined in the Ensembl Gene.\n");

        return ajFalse;
    }

    gslice = ensFeatureGetSlice(gfeature);

    if(!gslice)
    {
        ajWarn("ensTranscriptadaptorFetchAllbyGene cannot fetch "
               "Ensembl Transcript objects without an Ensembl Slice defined "
               "in the Ensembl Feature of the Ensembl Gene.\n");

        return ajFalse;
    }

    if(!ensSliceIsCircular(gslice, &circular))
        return ajFalse;

    if((ensFeatureGetStart(gfeature) < 1) ||
       (ensFeatureGetEnd(gfeature) > (ajint) ensSliceCalculateLength(gslice)))
    {
        if(circular == ajTrue)
            tslice = ensSliceNewRef(gslice);
        else
        {
            sa = ensRegistryGetSliceadaptor(dba);

            ensSliceadaptorFetchByFeature(sa, gfeature, 0, &tslice);
        }
    }
    else
        tslice = ensSliceNewRef(gslice);

    constraint = ajFmtStr(
        "transcript.gene_id = %u",
        ensGeneGetIdentifier(gene));

    list = ajListNew();

    ensFeatureadaptorFetchAllbySlice(tca,
                                     tslice,
                                     constraint,
                                     (const AjPStr) NULL,
                                     list);

    while(ajListPop(list, (void**) &transcript))
    {
        if(ensSliceMatch(gslice, tslice))
            ajListPushAppend(transcripts, (void*) transcript);
        else
        {
            newtranscript = ensTranscriptTransfer(transcript, tslice);

            ajListPushAppend(transcripts, (void*) newtranscript);

            ensTranscriptDel(&transcript);
        }
    }

    ajListFree(&list);

    ajStrDel(&constraint);

    ensSliceDel(&tslice);

    return ajTrue;
}




/* @funcstatic transcriptadaptorClearTranscriptExonRankList *******************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** AJAX List value data of Transcript Exon Rank objects.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] AJAX List address of Transcript Exon Rank objects
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void transcriptadaptorClearTranscriptExonRankList(void** key,
                                                         void** value,
                                                         void* cl)
{
    TranscriptPExonRank trex = NULL;

    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);

    while(ajListPop(*((AjPList*) value), (void**) &trex))
        transcriptExonRankDel(&trex);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTranscriptadaptorFetchAllbySlice **********************************
**
** Fetch all Ensembl Transcript objects via an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Transcript objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_Slice
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] constraint [const AjPStr] SQL constraint
** @param [r] loadexons [AjBool] Load Ensembl Exon objects
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbySlice(
    EnsPTranscriptadaptor tca,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr constraint,
    AjBool loadexons,
    AjPList transcripts)
{
    void** keyarray = NULL;

    ajint start = INT_MAX;
    ajint end   = INT_MIN;
    ajint rank  = 0;

    register ajuint i = 0;

    ajuint exid = 0;
    ajuint trid = 0;

    ajuint* Pidentifier = NULL;

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

    ensFeatureadaptorFetchAllbySlice(tca,
                                     slice,
                                     trconstraint,
                                     anname,
                                     transcripts);

    ajStrDel(&trconstraint);

    /* If there is no or just one Ensembl Transcript, still do lazy-loading. */

    if((!loadexons) || (ajListGetLength(transcripts) == 0))
        return ajTrue;

    /*
    ** Preload all Ensembl Exon objects now, instead of lazy loading later,
    ** which is faster than one SQL query per Transcript.
    ** First check if the Ensembl Exon objects are already preloaded.
    ** TODO: This should test all Ensembl Exon objects.
    */

    ajListPeekFirst(transcripts, (void**) &transcript);

    if(transcript->Exons)
        return ajTrue;

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    ea = ensRegistryGetExonadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    tla = ensRegistryGetTranslationadaptor(dba);

    /*
    ** Get the extent of the region spanned by Ensembl Transcript objects,
    ** prepare a comma-separared list of Transcript identifiers and put
    ** Ensembl Transcript objects into an AJAX Table indexed by their
    ** identifier.
    */

    csv = ajStrNew();

    trtable = ensTableuintNewLen(0);

    iter = ajListIterNew(transcripts);

    while(!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        trid = ensTranscriptGetIdentifier(transcript);

        feature = ensTranscriptGetFeature(transcript);

        start = (ensFeatureGetSeqregionStart(feature) < start) ?
            ensFeatureGetSeqregionStart(feature) : start;

        end = (ensFeatureGetSeqregionEnd(feature) > end) ?
            ensFeatureGetSeqregionEnd(feature) : end;

        ajFmtPrintAppS(&csv, "%u, ", trid);

        /*
        ** Put all Ensembl Transcript objects into an AJAX Table indexed by
        ** their identifier.
        */

        if(ajTableMatchV(trtable, (const void*) &trid))
            ajDebug("ensTranscriptadaptorFetchAllbySlice got duplicate "
                    "Ensembl Transcript with identifier %u.\n", trid);
        else
        {
            AJNEW0(Pidentifier);

            *Pidentifier = trid;

            ajTablePut(trtable,
                       (void*) Pidentifier,
                       (void*) ensTranscriptNewRef(transcript));
        }
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

    /* Associate Exon identifiers with Transcript objects and Exon ranks. */

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

    extable = ensTableuintNewLen(0);

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

        list = (AjPList) ajTableFetchmodV(extable, (const void*) &exid);

        if(!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = exid;

            list = ajListNew();

            ajTablePut(extable, (void*) Pidentifier, (void*) list);
        }

        ajListPushAppend(list, (void*) transcriptExonRankNew(trid, rank));
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    /* Get all Exon identifiers as comma-separated values. */

    ajTableToarrayKeys(extable, &keyarray);

    for(i = 0; keyarray[i]; i++)
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint*) keyarray[i]));

    AJFREE(keyarray);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    trconstraint = ajFmtStr("exon.exon_id IN (%S)", csv);

    ajStrDel(&csv);

    exons = ajListNew();

    ensExonadaptorFetchAllbySlice(ea, newslice, trconstraint, exons);

    ajStrDel(&trconstraint);

    /*
    ** Transfer Ensembl Exon objects onto the Ensembl Transcript Slice and add
    ** them to Ensembl Transcript objects.
    */

    while(ajListPop(exons, (void**) &exon))
    {
        newexon = ensExonTransfer(exon, newslice);

        if(!newexon)
            ajFatal("ensTranscriptAdaptorFetchAllbySlice could not transfer "
                    "Exon onto new Slice.\n");

        exid = ensExonGetIdentifier(newexon);

        list = (AjPList) ajTableFetchmodV(extable, (const void*) &exid);

        iter = ajListIterNew(list);

        while(!ajListIterDone(iter))
        {
            trex = (TranscriptPExonRank) ajListIterGet(iter);

            transcript = (EnsPTranscript) ajTableFetchmodV(
                trtable,
                (const void*) &trex->Transcriptidentifier);

            ensTranscriptAddExon(transcript, newexon, trex->Rank);
        }

        ajListIterDel(&iter);

        ensExonDel(&newexon);
        ensExonDel(&exon);
    }

    ajListFree(&exons);

    ensTranslationadaptorFetchAllbyTranscriptsTable(tla, trtable);

    /*
    ** Clear and delete the AJAX Table of
    ** AJAX unsigned integer key and
    ** Ensembl Transcript value data.
    */

    ensTableTranscriptDelete(&trtable);

    /*
    ** Clear and detete the AJAX Table of
    ** AJAX unsigned integer key and
    ** AJAX List value data.
    ** Also delete the transcriptExonRank objects from the AJAX Lists.
    */

    ajTableMapDel(extable, transcriptadaptorClearTranscriptExonRankList, NULL);

    ajTableFree(&extable);

    ensSliceDel(&newslice);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchAllbyStableidentifier ***********************
**
** Fetch all Ensembl Transcript versions via a stable identifier.
**
** The caller is responsible for deleting the Ensembl Transcript objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_versions_by_stable_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    AjPList transcripts)
{
    char* txtstableid = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!transcripts)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("transcript_stable_id.stable_id = '%s'",
                          txtstableid);

    ajCharDel(&txtstableid);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                transcripts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranscriptadaptorFetchByDisplaylabel ******************************
**
** Fetch an Ensembl Transcript via its display label.
**
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_display_label
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] label [const AjPStr] Display label
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByDisplaylabel(
    EnsPTranscriptadaptor tca,
    const AjPStr label,
    EnsPTranscript* Ptranscript)
{
    char* txtlabel = NULL;

    AjBool result = AJFALSE;

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

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtlabel, label);

    constraint = ajFmtStr(
        "transcript.is_current = 1 "
        "AND "
        "xref.display_label = '%s'",
        txtlabel);

    ajCharDel(&txtlabel);

    transcripts = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                transcripts);

    if(ajListGetLength(transcripts) > 1)
        ajDebug("ensTranscriptadaptorFetchByDisplaylabel got more than one "
                "Transcript for display label '%S'.\n", label);

    ajListPop(transcripts, (void**) Ptranscript);

    while(ajListPop(transcripts, (void**) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&transcripts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranscriptadaptorFetchByExonidentifier ****************************
**
** Fetch an Ensembl Transcript via an Ensembl Exon identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_exon_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] Ensembl Exon identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByExonidentifier(
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

    dba = ensFeatureadaptorGetDatabaseadaptor(tca);

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
        ajDebug("ensTranscriptadaptorFetchByExonidentifier could not "
                "get Transcript identifier for Exon identifier %u.\n",
                identifier);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    return ensBaseadaptorFetchByIdentifier(ba, trid, (void**) Ptranscript);
}




/* @func ensTranscriptadaptorFetchByExonstableidentifier **********************
**
** Fetch an Ensembl Transcript via an Ensembl Exon stable identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_exon_stable_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Ensembl Exon stable identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByExonstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript* Ptranscript)
{
    char* txtstableid = NULL;

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

    dba = ensFeatureadaptorGetDatabaseadaptor(tca);

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
        ajDebug("ensTranscriptadaptorFetchByExonstableidentifier "
                "could not get Transcript identifier for Exon "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    return ensBaseadaptorFetchByIdentifier(ba, trid, (void**) Ptranscript);
}




/* @func ensTranscriptadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl Transcript via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByIdentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript* Ptranscript)
{
    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    return ensBaseadaptorFetchByIdentifier(ba,
                                           identifier,
                                           (void**) Ptranscript);
}




/* @func ensTranscriptadaptorFetchByStableidentifier **************************
**
** Fetch an Ensembl Transcript via its stable identifier and version.
** In case a version is not specified, the current Ensembl Transcript
** will be returned.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_stable_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    ajuint version,
    EnsPTranscript* Ptranscript)
{
    char* txtstableid = NULL;

    AjBool result = AJFALSE;

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

    ba = ensFeatureadaptorGetBaseadaptor(tca);

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

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                transcripts);

    if(ajListGetLength(transcripts) > 1)
        ajDebug("ensTranscriptadaptorFetchByStableidentifier got more than "
                "one Transcript for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(transcripts, (void**) Ptranscript);

    while(ajListPop(transcripts, (void**) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&transcripts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranscriptadaptorFetchByTranslationidentifier *********************
**
** Fetch an Ensembl Transcript via its Ensembl Translation identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_translation_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] Ensembl Translation identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationidentifier(
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

    dba = ensFeatureadaptorGetDatabaseadaptor(tca);

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
        ajDebug("ensTranscriptadaptorFetchByTranslationidentifier could not "
                "get Transcript identifier for Translation identifier %u.\n",
                identifier);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    return ensBaseadaptorFetchByIdentifier(ba, trid, (void**) Ptranscript);
}




/* @func ensTranscriptadaptorFetchByTranslationstableidentifier ***************
**
** Fetch an Ensembl Transcript via its Ensembl Translation stable identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_by_translation_stable_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Ensembl Translation stable identifier
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript* Ptranscript)
{
    char* txtstableid = NULL;

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

    dba = ensFeatureadaptorGetDatabaseadaptor(tca);

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
        ajDebug("ensTranscriptadaptorFetchByTranslationstableidentifier "
                "could not get Transcript identifier for Translation "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    return ensBaseadaptorFetchByIdentifier(ba, trid, (void**) Ptranscript);
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Transcript objects
** from an Ensembl SQL database.
**
** @fdata [EnsPTranscriptadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Transcript-releated object(s)
** @nam4rule All Retrieve all Ensembl Transcript-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifiers
** @nam5rule Stableidentifiers Retrieve all stable Ensembl Transcript
** identifiers
**
** @argrule * tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of
** AJAX unsigned integer objects
** @argrule AllStableidentifiers identifiers [AjPList] AJAX List of
** AJAX String (Ensembl Transcript stable identifier) objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptadaptorRetrieveAllIdentifiers ***************************
**
** Fetch all SQL database-internal identifiers of Ensembl Transcript objects.
**
** The caller is responsible for deleting the AJAX unsigned integer objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::list_dbIDs
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integer objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorRetrieveAllIdentifiers(
    EnsPTranscriptadaptor tca,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    table = ajStrNewC("transcript");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensTranscriptadaptorRetrieveAllStableidentifiers *********************
**
** Fetch all stable identifiers of Ensembl Transcript objects.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::list_stable_ids
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTranscriptadaptorRetrieveAllStableidentifiers(
    EnsPTranscriptadaptor tca,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table   = NULL;
    AjPStr primary = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!tca)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(tca);

    table   = ajStrNewC("transcript_stable_id");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(ba, table, primary, identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}

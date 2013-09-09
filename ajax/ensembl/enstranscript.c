/* @source enstranscript ******************************************************
**
** Ensembl Transcript functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.81 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:09:58 $ by $Author: mks $
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




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */

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




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic transcriptKSequenceeditCode ***********************************
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

static const char *const transcriptKSequenceeditCode[] =
{
    "_rna_edit",
    (const char *) NULL
};




/* @conststatic transcriptKStatus *********************************************
**
** The Ensembl Transcript status member is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsETranscriptStatus
** and the 'transcript.status' field.
**
******************************************************************************/

static const char *const transcriptKStatus[] =
{
    "",
    "KNOWN",
    "NOVEL",
    "PUTATIVE",
    "PREDICTED",
    "KNOWN_BY_PROJECTION",
    "UNKNOWN",
    "ANNOTATED",
    (const char *) NULL
};




/* @conststatic transcriptadaptorKTablenames **********************************
**
** Array of Ensembl Transcript Adaptor SQL table names
**
******************************************************************************/

static const char *const transcriptadaptorKTablenames[] =
{
    "transcript",
    "xref",
    "external_db",
    (const char *) NULL
};




/* @conststatic transcriptadaptorKColumnnames *********************************
**
** Array of Ensembl Transcript Adaptor SQL column names
**
******************************************************************************/

static const char *const transcriptadaptorKColumnnames[] =
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
    "transcript.stable_id",
    "transcript.version",
    "transcript.created_date",
    "transcript.modified_date",
    "xref.external_db_id",
    "xref.dbprimary_acc",
    "xref.display_label",
    "xref.version",
    "xref.description",
    "xref.info_type",
    "xref.info_text",
    (const char *) NULL
};




/* @conststatic transcriptadaptorKLeftjoins ***********************************
**
** Array of Ensembl Transcript Adaptor SQL LEFT JOIN conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin transcriptadaptorKLeftjoins[] =
{
    {"xref", "xref.xref_id = transcript.display_xref_id"},
    {"external_db", "external_db.external_db_id = xref.external_db_id"},
    {(const char *) NULL, (const char *) NULL}
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static TranscriptPExonRank transcriptExonRankNew(ajuint trid, ajint rank);

static void transcriptExonRankDel(TranscriptPExonRank *Ptrex);

static EnsPTranscript transcriptNewCpyFeatures(EnsPTranscript transcript);

static int listTranscriptCompareEndAscending(
    const void *item1,
    const void *item2);

static int listTranscriptCompareEndDescending(
    const void *item1,
    const void *item2);

static int listTranscriptCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listTranscriptCompareStartAscending(
    const void *item1,
    const void *item2);

static int listTranscriptCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool transcriptadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList transcripts);

static void transcriptadaptorListTranscriptExonRankValdel(void **Pvalue);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




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
**
** @release 6.3.0
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
**
** @release 6.3.0
** @@
******************************************************************************/

static void transcriptExonRankDel(TranscriptPExonRank *Ptrex)
{
    ajMemFree((void **) Ptrex);

    return;
}




/* @datasection [EnsPTranscript] Ensembl Transcript ***************************
**
** @nam2rule Transcript Functions for manipulating Ensembl Transcript objects
**
** @cc Bio::EnsEMBL::Transcript
** @cc CVS Revision: 1.336
** @cc CVS Tag: branch-ensembl-68
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
** @valrule * [EnsPTranscript] Ensembl Transcript or NULL
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNewCpy(const EnsPTranscript transcript)
{
    AjBool *Pcanonical = NULL;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPIntronsupportingevidence ise = NULL;

    EnsPTranscript pthis = NULL;

    EnsPTranslation translation = NULL;

    if (!transcript)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = transcript->Identifier;

    pthis->Adaptor = transcript->Adaptor;

    pthis->Feature = ensFeatureNewRef(transcript->Feature);

    pthis->Displayreference = ensDatabaseentryNewRef(
        transcript->Displayreference);

    if (transcript->Description)
        pthis->Description = ajStrNewRef(transcript->Description);

    if (transcript->Biotype)
        pthis->Biotype = ajStrNewRef(transcript->Biotype);

    pthis->Status = transcript->Status;

    pthis->Current = transcript->Current;

    if (transcript->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(transcript->Stableidentifier);

    if (transcript->DateCreation)
        pthis->DateCreation = ajStrNewRef(transcript->DateCreation);

    if (transcript->DateModification)
        pthis->DateModification = ajStrNewRef(transcript->DateModification);

    pthis->Version = transcript->Version;

    pthis->Geneidentifier = transcript->Geneidentifier;

    /* NOTE: Copy the AJAX List of alternative Ensembl Translation objects. */

    if (transcript->Alternativetranslations
        && ajListGetLength(transcript->Alternativetranslations))
    {
        pthis->Alternativetranslations = ajListNew();

        iter = ajListIterNew(transcript->Alternativetranslations);

        while (!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            ajListPushAppend(pthis->Alternativetranslations,
                             (void *) ensTranslationNewRef(translation));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Alternativetranslations = NULL;

    /* Copy the AJAX List of Ensembl Attribute objects. */

    if (transcript->Attributes && ajListGetLength(transcript->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(transcript->Attributes);

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

    if (transcript->Databaseentries
        && ajListGetLength(transcript->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(transcript->Databaseentries);

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

    /* Copy the AJAX List of Ensembl Exon objects. */

    if (transcript->Exons && ajListGetLength(transcript->Exons))
    {
        pthis->Exons = ajListNew();

        iter = ajListIterNew(transcript->Exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ajListPushAppend(pthis->Exons, (void *) ensExonNewRef(exon));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Exons = NULL;

    /* Copy the AJAX List of Ensembl Intron Supporting Evidence objects. */

    if (transcript->Intronsupportingevidences
        && ajListGetLength(transcript->Intronsupportingevidences))
    {
        pthis->Intronsupportingevidences = ajListNew();

        iter = ajListIterNew(transcript->Intronsupportingevidences);

        while (!ajListIterDone(iter))
        {
            ise = (EnsPIntronsupportingevidence) ajListIterGet(iter);

            ajListPushAppend(pthis->Intronsupportingevidences,
                             (void *) ensIntronsupportingevidenceNewRef(ise));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Intronsupportingevidences = NULL;

    /*
    ** NOTE: Copy the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    if (transcript->Supportingfeatures &&
        ajListGetLength(transcript->Supportingfeatures))
    {
        pthis->Supportingfeatures = ajListNew();

        iter = ajListIterNew(transcript->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListPushAppend(pthis->Supportingfeatures,
                             (void *) ensBasealignfeatureNewRef(baf));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Supportingfeatures = NULL;

    pthis->Translation = ensTranslationNewRef(transcript->Translation);

    pthis->Sequenceedits = transcript->Sequenceedits;

    if (transcript->Canonical)
    {
        AJNEW0(Pcanonical);

        pthis->Canonical = Pcanonical;

        *pthis->Canonical = *transcript->Canonical;
    }

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
**
** @release 6.4.0
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

    if (!feature)
        return NULL;

    AJNEW0(transcript);

    transcript->Use = 1U;

    transcript->Identifier = identifier;

    transcript->Adaptor = tca;

    transcript->Feature = ensFeatureNewRef(feature);

    transcript->Displayreference = ensDatabaseentryNewRef(displaydbe);

    if (description)
        transcript->Description = ajStrNewRef(description);

    if (biotype)
        transcript->Biotype = ajStrNewRef(biotype);

    transcript->Status = status;

    transcript->Current = current;

    if (stableid)
        transcript->Stableidentifier = ajStrNewRef(stableid);

    if (cdate)
        transcript->DateCreation = ajStrNewRef(cdate);

    if (mdate)
        transcript->DateModification = ajStrNewRef(mdate);

    transcript->Version = version;

    transcript->Geneidentifier = 0;

    transcript->Alternativetranslations = NULL;

    transcript->Attributes = NULL;

    transcript->Databaseentries = NULL;

    if (exons && ajListGetLength(exons))
    {
        transcript->Exons = ajListNew();

        iter = ajListIterNew(exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ajListPushAppend(transcript->Exons,
                             (void *) ensExonNewRef(exon));
        }

        ajListIterDel(&iter);
    }
    else
        transcript->Exons = NULL;

    transcript->Intronsupportingevidences = NULL;

    transcript->Supportingfeatures = NULL;

    transcript->Translation = NULL;

    /*
    ** NOTE: Sequence Edit objects such as transcription and translation
    ** exceptions are applied by default. Use ensTranscriptGetSequenceedits and
    ** ensTranscriptSetSequenceedits to toggle Sequence Edit application.
    */

    transcript->Sequenceedits = ajTrue;

    transcript->Canonical = NULL;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranscript ensTranscriptNewRef(EnsPTranscript transcript)
{
    if (!transcript)
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
**
** @release 6.4.0
** @@
******************************************************************************/

static EnsPTranscript transcriptNewCpyFeatures(EnsPTranscript transcript)
{
    AjIList iter = NULL;

    EnsPBasealignfeature newbaf = NULL;
    EnsPBasealignfeature oldbaf = NULL;

    EnsPExon newexon = NULL;
    EnsPExon oldexon = NULL;

    EnsPIntronsupportingevidence newise = NULL;
    EnsPIntronsupportingevidence oldise = NULL;

    EnsPTranscript newtranscript = NULL;

    EnsPTranslation newtranslation = NULL;
    EnsPTranslation oldtranslation = NULL;

    if (!transcript)
        return NULL;

    newtranscript = ensTranscriptNewCpy(transcript);

    if (!newtranscript)
        return NULL;

    /* Copy the AJAX List of alternative Ensembl Translation objects. */

    if (newtranscript->Alternativetranslations)
    {
        iter = ajListIterNew(newtranscript->Alternativetranslations);

        while (!ajListIterDone(iter))
        {
            oldtranslation = (EnsPTranslation) ajListIterGet(iter);

            ajListIterRemove(iter);

            newtranslation = ensTranslationNewCpy(oldtranslation);

            ajListIterInsert(iter, (void *) newtranslation);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensTranslationDel(&oldtranslation);
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of Ensembl Exon objects. */

    if (newtranscript->Exons)
    {
        iter = ajListIterNew(newtranscript->Exons);

        while (!ajListIterDone(iter))
        {
            oldexon = (EnsPExon) ajListIterGet(iter);

            ajListIterRemove(iter);

            newexon = ensExonNewCpy(oldexon);

            ajListIterInsert(iter, (void *) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensExonDel(&oldexon);
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of Ensembl Intron Supporting Evidence objects. */

    if (newtranscript->Intronsupportingevidences)
    {
        iter = ajListIterNew(newtranscript->Intronsupportingevidences);

        while (!ajListIterDone(iter))
        {
            oldise = (EnsPIntronsupportingevidence) ajListIterGet(iter);

            ajListIterRemove(iter);

            newise = ensIntronsupportingevidenceNewCpy(oldise);

            ajListIterInsert(iter, (void *) newise);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensIntronsupportingevidenceDel(&oldise);
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of supporting Ensembl Base Align Feature objects. */

    if (newtranscript->Supportingfeatures)
    {
        iter = ajListIterNew(newtranscript->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListIterRemove(iter);

            newbaf = ensBasealignfeatureNewCpy(oldbaf);

            ajListIterInsert(iter, (void *) newbaf);

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




/* @section clear *************************************************************
**
** Clear internal data structures and frees the memory
** allocated for an Ensembl Transcript object internals.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Clear Clear an Ensembl Transcript
** @nam4rule Intronsupportingevidences
** Clear Ensembl Intron Supporting Evidence objects
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensTranscriptClearIntronsupportingevidences **************************
**
** Clear Ensembl Intron Supporting Evidence objects of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::flush_IntronSupportingEvidence
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptClearIntronsupportingevidences(EnsPTranscript transcript)
{
    EnsPIntronsupportingevidence ise = NULL;

    if (!transcript)
        return ajFalse;

    /*
    ** Clear and delete the AJAX List of
    ** Ensembl Intron Supporting Evidence objects.
    */

    while (ajListPop(transcript->Intronsupportingevidences, (void **) &ise))
        ensIntronsupportingevidenceDel(&ise);

    ajListFree(&transcript->Intronsupportingevidences);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Del Destroy (free) an Ensembl Transcript
**
** @argrule * Ptranscript [EnsPTranscript*] Ensembl Transcript address
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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensTranscriptDel(EnsPTranscript *Ptranscript)
{
    EnsPAttribute attribute = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPIntronsupportingevidence ise = NULL;

    EnsPTranscript pthis = NULL;

    EnsPTranslation translation = NULL;

    if (!Ptranscript)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensTranscriptDel"))
    {
        ajDebug("ensTranscriptDel\n"
                "  *Ptranscript %p\n",
                *Ptranscript);

        ensTranscriptTrace(*Ptranscript, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Ptranscript) || --pthis->Use)
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

    while (ajListPop(pthis->Alternativetranslations, (void **) &translation))
        ensTranslationDel(&translation);

    ajListFree(&pthis->Alternativetranslations);

    /* Clear and delete the AJAX List of Ensembl Attribute objects. */

    while (ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while (ajListPop(pthis->Databaseentries, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Exon objects. */

    while (ajListPop(pthis->Exons, (void **) &exon))
        ensExonDel(&exon);

    ajListFree(&pthis->Exons);

    /*
    ** Clear and delete the AJAX List of
    ** Ensembl Intron Supporting Evidence objects.
    */

    while (ajListPop(pthis->Intronsupportingevidences, (void **) &ise))
        ensIntronsupportingevidenceDel(&ise);

    ajListFree(&pthis->Intronsupportingevidences);

    /* Clear and delete the AJAX List of Ensembl Base Align Feature objects. */

    while (ajListPop(pthis->Supportingfeatures, (void **) &baf))
        ensBasealignfeatureDel(&baf);

    ajListFree(&pthis->Supportingfeatures);

    ensTranslationDel(&pthis->Translation);

    ajMemFree((void **) &pthis->Canonical);

    ensMapperDel(&pthis->ExonCoordMapper);

    ajMemFree((void **) Ptranscript);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Get Return Transcript attribute(s)
** @nam4rule Adaptor Return the Ensembl Transcript Adaptor
** @nam4rule Biotype Return the biological type
** @nam4rule Canonical Return the canonical flag
** @nam4rule Current Return the current flag
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
** @valrule Canonical [const AjBool*] Canonical atrribute or NULL
** @valrule Current [AjBool] Current attribute or ajFalse
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Modification date or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayreference [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Geneidentifier [ajuint] Ensembl Gene identifier or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Sequenceedits [AjBool] Enable Ensembl Sequence Edit flag ro ajFalse
** @valrule Status [EnsETranscriptStatus] Status or NULL
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Version [ajuint] Version or 0U
** @valrule Exons [const AjPList] AJAX List of Ensembl Exon objects or NULL
** @valrule Supportingfeatures [const AjPList] AJAX List of Ensembl Base
** Align Feature objects or NULL
** @valrule Translation [EnsPTranslation] Ensembl Translation or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptGetAdaptor **********************************************
**
** Get the Ensembl Transcript Adaptor member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptGetAdaptor(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Adaptor : NULL;
}




/* @func ensTranscriptGetBiotype **********************************************
**
** Get the biological type member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::biotype
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Biological type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranscriptGetBiotype(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Biotype : NULL;
}




/* @func ensTranscriptGetCanonical ********************************************
**
** Get the canonical member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_canonical
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [const AjBool*] Canonical attribute or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjBool* ensTranscriptGetCanonical(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Canonical : NULL;
}




/* @func ensTranscriptGetCurrent **********************************************
**
** Get the current member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_current
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue if this Transcript reflects the current state of
**                  annotation
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptGetCurrent(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Current : ajFalse;
}




/* @func ensTranscriptGetDateCreation *****************************************
**
** Get the creation date member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::created_date
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranscriptGetDateCreation(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->DateCreation : NULL;
}




/* @func ensTranscriptGetDateModification *************************************
**
** Get the modification date member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::modified_date
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Modification date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranscriptGetDateModification(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->DateModification : NULL;
}




/* @func ensTranscriptGetDescription ******************************************
**
** Get the description member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::description
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensTranscriptGetDescription(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Description : NULL;
}




/* @func ensTranscriptGetDisplayreference *************************************
**
** Get the display reference member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::display_xref
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseentry ensTranscriptGetDisplayreference(
    const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Displayreference : NULL;
}




/* @func ensTranscriptGetFeature **********************************************
**
** Get the Ensembl Feature member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Feature
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensTranscriptGetFeature(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Feature : NULL;
}




/* @func ensTranscriptGetGeneidentifier ***************************************
**
** Get the SQL database-internal Ensembl Gene identifier member of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::???
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] SQL database-internal identifier of the Ensembl Gene or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensTranscriptGetGeneidentifier(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Geneidentifier : 0U;
}




/* @func ensTranscriptGetIdentifier *******************************************
**
** Get the SQL database-internal identifier member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensTranscriptGetIdentifier(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Identifier : 0U;
}




/* @func ensTranscriptGetSequenceedits ****************************************
**
** Get the enable Ensembl Sequence Edit objects member of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::edits_enabled
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue: Apply Ensembl Sequence Edit objects to the sequence
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptGetSequenceedits(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Sequenceedits : ajFalse;
}




/* @func ensTranscriptGetStableidentifier *************************************
**
** Get the stable identifier member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::stable_id
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [AjPStr] Stable identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranscriptGetStableidentifier(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Stableidentifier : NULL;
}




/* @func ensTranscriptGetStatus ***********************************************
**
** Get the status member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::status
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [EnsETranscriptStatus] Status or ensETranscriptStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsETranscriptStatus ensTranscriptGetStatus(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Status : ensETranscriptStatusNULL;
}




/* @func ensTranscriptGetVersion **********************************************
**
** Get the version member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::version
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Version or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensTranscriptGetVersion(const EnsPTranscript transcript)
{
    return (transcript) ? transcript->Version : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Transcript object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Load Return Ensembl Transcript attribute(s) loaded on demand
** @nam4rule Alternativetranslations
** Return all alternative Ensembl Translation objects
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Canonical Return the canonical flag
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Exons Return all Ensembl Exon objects
** @nam4rule Intronsupportingevidences
** Return all Ensembl Intron Supporting Evidence objects
** @nam4rule Supportingfeatures
** Return the supporting Ensembl Base Align Feature objects
** @nam4rule Translation Return the Ensembl Translation
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule Alternativetranslations [const AjPList]
** AJAX List of alternative Ensembl Translation objects or NULL
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Canonical [const AjBool*] Canonical flag
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule Exons [const AjPList]
** AJAX List of Ensembl Exon objects or NULL
** @valrule Intronsupportingevidences [const AjPList]
** AJAX List of Ensembl Intron Supporting Evidence objects or NULL
** @valrule Supportingfeatures [const AjPList]
** AJAX List of supporting Ensembl Base Align Feature objects or NULL
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
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranscriptLoadAlternativetranslations(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranslationadaptor tla  = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Alternativetranslations)
        return transcript->Alternativetranslations;

    if (!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadAlternativetranslations cannot fetch "
                "Ensembl Translation objects for an "
                "Ensembl Transcript without an "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if (!dba)
    {
        ajDebug("ensTranscriptLoadAlternativetranslations cannot fetch "
                "Ensembl Translation objects for an "
                "Ensembl Transcript without an "
                "Ensembl Database Adaptor set in the "
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
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranscriptLoadAttributes(
    EnsPTranscript transcript)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Attributes)
        return transcript->Attributes;

    if (!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Transcript without an "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if (!dba)
    {
        ajDebug("ensTranscriptLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an "
                "Ensembl Transcript without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Transcript Adaptor.\n");

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




/* @func ensTranscriptLoadCanonical *******************************************
**
** Load the canonical member of an Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch the canonical flag
** from an Ensembl SQL database in case the flag is not defined.
**
** @cc Bio::EnsEMBL::Transcript::is_canonical
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptadaptorRetrieveCanonical
**
** @return [const AjBool*] Canonical attribute or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjBool* ensTranscriptLoadCanonical(
    EnsPTranscript transcript)
{
    AjBool canonical = AJFALSE;

    if (!transcript)
        return NULL;

    if (transcript->Canonical)
        return transcript->Canonical;

    if (!transcript->Canonical)
    {
        ajDebug("ensTranscriptLoadCanonical cannot fetch "
                "the canonical flag for an "
                "Ensembl Transcript without an "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    ensTranscriptadaptorRetrieveCanonical(transcript->Adaptor,
                                          transcript,
                                          &canonical);

    return transcript->Canonical;
}




/* @func ensTranscriptLoadDatabaseentries *************************************
**
** Load all Ensembl Database Entry objects of an Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt loading the
** Ensembl Database Entry objects from the Ensembl SQL database associated with
** the Ensembl Transcript Adaptor.
**
** To filter Ensembl Database Entry objects via an Ensembl External Database
** name or type, consider using ensTranscriptFetchAllDatabaseentries.
**
** @cc Bio::EnsEMBL::Transcript::get_all_DBEntries
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @see ensTranscriptFetchAllDatabaseentries
**
** @return [const AjPList] AJAX List of Ensembl Database Entry objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranscriptLoadDatabaseentries(
    EnsPTranscript transcript)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentryadaptor dbea = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Databaseentries)
        return transcript->Databaseentries;

    if (!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an "
                "Ensembl Transcript without an "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    if (!dba)
    {
        ajDebug("ensTranscriptLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an "
                "Ensembl Transcript without an "
                "Ensembl Database Adaptor set in the "
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
** Ensembl Exon objects from the Ensembl Core database associated with the
** Ensembl Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::get_all_Exons
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList] AJAX List of Ensembl Exon objects or NULL
**
** @release 6.4.0
** @@
** NOTE: This implementation does not support the constitutive parameter.
** Please use the ensTranscriptFetchAllConstitutiveexons function instead.
******************************************************************************/

const AjPList ensTranscriptLoadExons(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPExonadaptor ea = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Exons)
        return transcript->Exons;

    if (!transcript->Adaptor)
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




/* @func ensTranscriptLoadIntronsupportingevidences ***************************
**
** Load all Ensembl Intron Suppoorting Eidence objects of an
** Ensembl Transcript.
**
** This is not a simple accessor function, since it will attempt fetching the
** Ensembl Intron Suppoorting Evidence objects from the
** Ensembl Core database associated with the Ensembl Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::get_all_IntronSupportingEvidence
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [const AjPList]
** AJAX List of Ensembl Intron Supporting Evidence objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensTranscriptLoadIntronsupportingevidences(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPIntronsupportingevidenceadaptor isea = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Intronsupportingevidences)
        return transcript->Intronsupportingevidences;

    if (!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadExons cannot fetch "
                "Ensembl Intron Suppoorting Evidence objects for an "
                "Ensembl Transcript without an "
                "Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    isea = ensRegistryGetIntronsupportingevidenceadaptor(dba);

    transcript->Intronsupportingevidences = ajListNew();

    ensIntronsupportingevidenceadaptorFetchAllbyTranscript(
        isea,
        transcript,
        transcript->Intronsupportingevidences);

    return transcript->Intronsupportingevidences;
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
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranscriptLoadSupportingfeatures(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPSupportingfeatureadaptor sfa = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Supportingfeatures)
        return transcript->Supportingfeatures;

    if (!transcript->Adaptor)
    {
        ajDebug("ensTranscriptLoadSupportingfeatures cannot fetch "
                "Ensembl Base Align Feature objects for an Ensembl Transcript "
                "without an Ensembl Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    sfa = ensRegistryGetSupportingfeatureadaptor(dba);

    transcript->Supportingfeatures = ajListNew();

    ensSupportingfeatureadaptorFetchAllbyTranscript(
        sfa,
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
** with the Ensembl Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Transcript::translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [EnsPTranslation] Ensembl Translation objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPTranslation ensTranscriptLoadTranslation(
    EnsPTranscript transcript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranslationadaptor tla = NULL;

    if (!transcript)
        return NULL;

    if (transcript->Translation)
        return transcript->Translation;

    if (!transcript->Adaptor)
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




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Set Set one member of an Ensembl Transcript
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
** Set the Ensembl Transcript Adaptor member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptSetAdaptor(EnsPTranscript transcript,
                               EnsPTranscriptadaptor tca)
{
    if (!transcript)
        return ajFalse;

    transcript->Adaptor = tca;

    return ajTrue;
}




/* @func ensTranscriptSetBiotype **********************************************
**
** Set the biological type member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::biotype
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] biotype [AjPStr] Biological type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetBiotype(EnsPTranscript transcript,
                               AjPStr biotype)
{
    if (!transcript)
        return ajFalse;

    ajStrDel(&transcript->Biotype);

    if (biotype)
        transcript->Description = ajStrNewRef(biotype);

    return ajTrue;
}




/* @func ensTranscriptSetCanonical ********************************************
**
** Set the canonical member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_canonical
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] canonical [AjBool] Canonical attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetCanonical(EnsPTranscript transcript,
                                 AjBool canonical)
{
    AjBool *Pcanonical = NULL;

    if (!transcript)
        return ajFalse;

    if (!transcript->Canonical)
    {
        AJNEW0(Pcanonical);

        transcript->Canonical = Pcanonical;
    }

    *transcript->Canonical = canonical;

    return ajTrue;
}




/* @func ensTranscriptSetCurrent **********************************************
**
** Set the current member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::is_current
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] current [AjBool] Current attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensTranscriptSetCurrent(EnsPTranscript transcript,
                               AjBool current)
{
    if (!transcript)
        return ajFalse;

    transcript->Current = current;

    return ajTrue;
}




/* @func ensTranscriptSetDateCreation *****************************************
**
** Set the creation date member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::created_date
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetDateCreation(EnsPTranscript transcript,
                                    AjPStr cdate)
{
    if (!transcript)
        return ajFalse;

    ajStrDel(&transcript->DateCreation);

    if (cdate)
        transcript->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensTranscriptSetDateModification *************************************
**
** Set the modification date member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::modified_date
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetDateModification(EnsPTranscript transcript,
                                        AjPStr mdate)
{
    if (!transcript)
        return ajFalse;

    ajStrDel(&transcript->DateModification);

    if (mdate)
        transcript->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensTranscriptSetDescription ******************************************
**
** Set the description member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::description
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensTranscriptSetDescription(EnsPTranscript transcript,
                                   AjPStr description)
{
    if (!transcript)
        return ajFalse;

    ajStrDel(&transcript->Description);

    if (description)
        transcript->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensTranscriptSetDisplayreference *************************************
**
** Set the display Ensembl Database Entry member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::display_xref
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] displaydbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetDisplayreference(EnsPTranscript transcript,
                                        EnsPDatabaseentry displaydbe)
{
    if (!transcript)
        return ajFalse;

    ensDatabaseentryDel(&transcript->Displayreference);

    transcript->Displayreference = ensDatabaseentryNewRef(displaydbe);

    return ajTrue;
}




/* @func ensTranscriptSetFeature **********************************************
**
** Set the Ensembl Feature member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Feature
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptSetFeature(EnsPTranscript transcript,
                               EnsPFeature feature)
{
    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    EnsPBasealignfeature oldbaf = NULL;
    EnsPBasealignfeature newbaf = NULL;

    EnsPExon oldexon = NULL;
    EnsPExon newexon = NULL;

    EnsPIntronsupportingevidence oldise = NULL;
    EnsPIntronsupportingevidence newise = NULL;

    EnsPSlice slice = NULL;

    EnsPTranslation translation = NULL;

    if (ajDebugTest("ensTranscriptSetFeature"))
    {
        ajDebug("ensTranscriptSetFeature\n"
                "  transcript %p\n"
                "  feature %p\n",
                transcript,
                feature);

        ensTranscriptTrace(transcript, 1);

        ensFeatureTrace(feature, 1);
    }

    if (!transcript)
        return ajFalse;

    if (!feature)
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

    if (transcript->Exons)
    {
        iter1 = ajListIterNew(transcript->Exons);

        while (!ajListIterDone(iter1))
        {
            oldexon = (EnsPExon) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newexon = ensExonTransfer(oldexon, slice);

            if (!newexon)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Ensembl Exon onto new "
                        "Ensembl Feature Slice.");

                ensExonTrace(oldexon, 1);
            }

            ajListIterInsert(iter1, (void *) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            /*
            ** Re-assign the start and end Ensembl Exon objects of the
            ** Ensembl Translation.
            */

            if (transcript->Translation)
            {
                if (ensExonMatch(
                        oldexon,
                        ensTranslationGetStartexon(transcript->Translation)))
                    ensTranslationSetStartexon(transcript->Translation,
                                               newexon);

                if (ensExonMatch(
                        oldexon,
                        ensTranslationGetEndexon(transcript->Translation)))
                    ensTranslationSetEndexon(transcript->Translation,
                                             newexon);
            }

            /*
            ** Reassign also the start and end Exon for each alternative
            ** Translation of this Transcript.
            */

            if (transcript->Alternativetranslations)
            {
                iter2 = ajListIterNew(transcript->Alternativetranslations);

                while (!ajListIterDone(iter2))
                {
                    translation = (EnsPTranslation) ajListIterGet(iter2);

                    if (ensExonMatch(
                            oldexon,
                            ensTranslationGetStartexon(translation)))
                        ensTranslationSetStartexon(translation, newexon);

                    if (ensExonMatch(
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
    ** Transfer all Ensembl Intron Supporting Evidence objects onto the new
    ** Ensembl Feature Slice.
    */

    if (transcript->Intronsupportingevidences)
    {
        iter1 = ajListIterNew(transcript->Intronsupportingevidences);

        while (!ajListIterDone(iter1))
        {
            oldise = (EnsPIntronsupportingevidence) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newise = ensIntronsupportingevidenceTransfer(oldise, slice);

            if (!newise)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Ensembl Intron Supporting Evidence onto new "
                        "Ensembl Feature Slice.");

                ensIntronsupportingevidenceTrace(oldise, 1);
            }

            ajListIterInsert(iter1, (void *) newise);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            ensIntronsupportingevidenceDel(&oldise);
        }

        ajListIterDel(&iter1);
    }

    /*
    ** Transfer all Ensembl Base Align Feature objects onto the new
    ** Ensembl Feature Slice.
    */

    if (transcript->Supportingfeatures)
    {
        iter1 = ajListIterNew(transcript->Supportingfeatures);

        while (!ajListIterDone(iter1))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newbaf = ensBasealignfeatureTransfer(oldbaf, slice);

            if (!newbaf)
            {
                ajDebug("ensTranscriptSetFeature could not transfer "
                        "Ensembl Base Align Feature onto new "
                        "Ensembl Feature Slice.");

                ensBasealignfeatureTrace(oldbaf, 1);
            }

            ajListIterInsert(iter1, (void *) newbaf);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            ensBasealignfeatureDel(&oldbaf);
        }

        ajListIterDel(&iter1);
    }

    /* Clear internal members that depend on Ensembl Exon coordinates. */

    ensMapperClear(transcript->ExonCoordMapper);

    return ajTrue;
}




/* @func ensTranscriptSetGeneidentifier ***************************************
**
** Set the Ensembl Gene identifier member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::???
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] geneid [ajuint] Ensembl Gene identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetGeneidentifier(EnsPTranscript transcript,
                                      ajuint geneid)
{
    if (!transcript)
        return ajFalse;

    transcript->Geneidentifier = geneid;

    return ajTrue;
}




/* @func ensTranscriptSetIdentifier *******************************************
**
** Set the SQL database-internal identifier member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptSetIdentifier(EnsPTranscript transcript, ajuint identifier)
{
    if (!transcript)
        return ajFalse;

    transcript->Identifier = identifier;

    return ajTrue;
}




/* @func ensTranscriptSetSequenceedits ****************************************
**
** Set the enable Ensembl Sequence Edit object member of an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::edits_enabled
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] enablese [AjBool] Enable Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetSequenceedits(EnsPTranscript transcript,
                                     AjBool enablese)
{
    if (!transcript)
        return ajFalse;

    transcript->Sequenceedits = enablese;

    return ajTrue;
}




/* @func ensTranscriptSetStableidentifier *************************************
**
** Set the stable identifier member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::stable_id
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptSetStableidentifier(EnsPTranscript transcript,
                                        AjPStr stableid)
{
    if (!transcript)
        return ajFalse;

    ajStrDel(&transcript->Stableidentifier);

    if (stableid)
        transcript->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensTranscriptSetStatus ***********************************************
**
** Set the status member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::status
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] status [EnsETranscriptStatus] Status
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensTranscriptSetStatus(EnsPTranscript transcript,
                              EnsETranscriptStatus status)
{
    if (!transcript)
        return ajFalse;

    transcript->Status = status;

    return ajTrue;
}




/* @func ensTranscriptSetTranslation ******************************************
**
** Set the Ensembl Translation member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::translation
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uE] translation [EnsPTranslation] Ensembl Translation
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensTranscriptSetTranslation(EnsPTranscript transcript,
                                   EnsPTranslation translation)
{
    if (!transcript)
        return ajFalse;

    ensTranslationDel(&transcript->Translation);

    transcript->Translation = ensTranslationNewRef(translation);

    /* Clear internal members that depend on Ensembl Translation coordinates. */

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
** Set the version member of an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::version
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensTranscriptSetVersion(EnsPTranscript transcript,
                               ajuint version)
{
    if (!transcript)
        return ajFalse;

    transcript->Version = version;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Add Add one object to an Ensembl Transcript
** @nam4rule Alternativetranslation Add an alternative Ensembl Translation
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Databaseentry Add an Ensembl Database Entry
** @nam4rule Exon Add an Ensembl Exon
** @nam4rule Intronsupportingevidence Add an Ensembl Intron Supporting Evidence
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript object
** @argrule Alternativetranslation translation [EnsPTranslation]
**                                                         Ensembl Translation
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Databaseentry dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Exon exon [EnsPExon] Ensembl Exon
** @argrule Exon rank [ajint] Ensembl Exon rank
** @argrule Intronsupportingevidence ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptAddAlternativetranslation(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!transcript->Alternativetranslations)
        transcript->Alternativetranslations = ajListNew();

    ajListPushAppend(transcript->Alternativetranslations,
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptAddAttribute(
    EnsPTranscript transcript,
    EnsPAttribute attribute)
{
    register ajuint i = 0U;

    AjBool match = AJFALSE;

    if (!transcript)
        return ajFalse;

    if (!attribute)
        return ajFalse;

    if (!transcript->Attributes)
        transcript->Attributes = ajListNew();

    ajListPushAppend(transcript->Attributes,
                     (void *) ensAttributeNewRef(attribute));

    /*
    ** Check if this Attribute is associated with a Sequence Edit and clear
    ** internal members that depend on the now modified Transcript sequence.
    */

    for (i = 0U; transcriptKSequenceeditCode[i]; i++)
        if (ajStrMatchC(ensAttributeGetCode(attribute),
                        transcriptKSequenceeditCode[i]))
            match = ajTrue;

    if (match)
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptAddDatabaseentry(
    EnsPTranscript transcript,
    EnsPDatabaseentry dbe)
{
    if (!transcript)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!transcript->Databaseentries)
        transcript->Databaseentries = ajListNew();

    ajListPushAppend(transcript->Databaseentries,
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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensTranscriptAddExon(
    EnsPTranscript transcript,
    EnsPExon exon,
    ajint rank)
{
    register ajint i = 0;

    AjBool added = AJFALSE;

    AjIList iter = NULL;

    AjPStr message = NULL;

    EnsPExon curexon = NULL; /* Current Ensembl Exon */

    EnsPFeature feature    = NULL;
    EnsPFeature curfeature = NULL; /* Current Ensembl Feature */
    EnsPFeature prvfeature = NULL; /* Previous Ensembl Feature */

    if (ajDebugTest("ensTranscriptAddExon"))
        ajDebug("ensTranscriptAddExon\n"
                "  transcript %p\n"
                "  exon %p\n"
                "  rank %d\n",
                transcript,
                exon,
                rank);

    if (!transcript)
        return ajFalse;

    if (!exon)
        return ajFalse;

    if (!transcript->Exons)
        transcript->Exons = ajListNew();

    if (rank > 0)
    {
        iter = ajListIterNew(transcript->Exons);

        i = 0;

        while (i < rank)
        {
            i++;

            if (ajListIterDone(iter))
            {
                /*
                ** If the AJAX List is too short, append the Exon if the rank
                ** matches this position or an empty node otherwise.
                */

                if (i == rank)
                {
                    ajListPushAppend(transcript->Exons,
                                     (void *) ensExonNewRef(exon));

                    break;
                }
                else
                    ajListPushAppend(transcript->Exons, NULL);
            }

            /* Advance one step. */

            curexon = (EnsPExon) ajListIterGet(iter);

            if (i == rank)
            {
                /*
                ** Remove the current position, delete the previous
                ** Ensembl Exon, if any, insert the new Ensembl Exon and
                ** advance the AJAX List Iterator one step. Remember, inserts
                ** into AJAX List objects are applied ahead of the
                ** AJAX List Iterator so that they will be seen in the
                ** next iteration.
                */

                ajListIterRemove(iter);

                ensExonDel(&curexon);

                ajListIterInsert(iter, (void *) ensExonNewRef(exon));

                /* Advance the AJAX List Iterator after the insert. */

                curexon = (EnsPExon) ajListIterGet(iter);
            }
        }

        ajListIterDel(&iter);

        return ajTrue;
    }

    feature = ensExonGetFeature(exon);

    if (ensFeatureGetStrand(feature) > 0)
    {
        ajListPeekLast(transcript->Exons, (void **) &curexon);

        curfeature = ensExonGetFeature(curexon);

        if (ensFeatureGetStart(feature) > ensFeatureGetEnd(curfeature))
        {
            /* Append at the end. */

            ajListPushAppend(transcript->Exons, (void *) ensExonNewRef(exon));

            added = ajTrue;
        }
        else
        {
            /* Insert at the correct position. */

            prvfeature = NULL;

            iter = ajListIterNew(transcript->Exons);

            while (!ajListIterDone(iter))
            {
                curexon = (EnsPExon) ajListIterGet(iter);

                curfeature = ensExonGetFeature(curexon);

                if (ensFeatureGetStart(feature) <
                    ensFeatureGetStart(curfeature))
                {
                    /* Ensembl Exon objects that overlap are not added. */

                    if (ensFeatureGetEnd(feature) >=
                        ensFeatureGetStart(curfeature))
                        break;

                    if ((prvfeature != NULL) &&
                        (ensFeatureGetStart(feature) <=
                         ensFeatureGetEnd(prvfeature)))
                        break;

                    ajListIterInsert(iter, (void *) ensExonNewRef(exon));

                    added = ajTrue;

                    break;
                }

                prvfeature = curfeature;
            }

            ajListIterDel(&iter);
        }
    }
    else
    {
        ajListPeekLast(transcript->Exons, (void **) &curexon);

        curfeature = ensExonGetFeature(curexon);

        if (ensFeatureGetEnd(feature) < ensFeatureGetStart(curfeature))
        {
            /* Append at the end. */

            ajListPushAppend(transcript->Exons, (void *) ensExonNewRef(exon));

            added = ajTrue;
        }
        else
        {
            /* Insert at the correct position. */

            prvfeature = NULL;

            iter = ajListIterNew(transcript->Exons);

            while (!ajListIterDone(iter))
            {
                curexon = (EnsPExon) ajListIterGet(iter);

                curfeature = ensExonGetFeature(curexon);

                if (ensFeatureGetEnd(feature) > ensFeatureGetEnd(curfeature))
                {
                    /* Ensembl Exon objects that overlap are not added. */

                    if (ensFeatureGetStart(feature) <=
                        ensFeatureGetEnd(curfeature))
                        break;

                    if ((prvfeature != NULL) &&
                        (ensFeatureGetEnd(feature) >=
                         ensFeatureGetStart(prvfeature)))
                        ajListIterInsert(iter, (void *) ensExonNewRef(exon));

                    added = ajTrue;

                    break;
                }

                prvfeature = curfeature;
            }

            ajListIterDel(&iter);
        }
    }

    /* Sanity check. */

    if (!added)
    {
        /* The Exon was not added because it overlaps with another Exon. */

        message = ajStrNewC("ensTranscriptAddExon got an Exon, "
                            "which overlaps with another Exon in the same "
                            "Transcript.\n"
                            "  Transcript Exons:\n");

        iter = ajListIterNew(transcript->Exons);

        while (!ajListIterDone(iter))
        {
            curexon = (EnsPExon) ajListIterGet(iter);

            curfeature = ensExonGetFeature(curexon);

            ajFmtPrintAppS(&message,
                           "    %S %d:%d:%d\n",
                           ensExonGetStableidentifier(exon),
                           ensFeatureGetStart(curfeature),
                           ensFeatureGetEnd(curfeature),
                           ensFeatureGetStrand(curfeature));
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
    ** Ensembl Transcript Mapper, as well as internal members.
    */

    ensTranscriptCalculateCoordinates(transcript);

    return ajTrue;
}




/* @func ensTranscriptAddIntronsupportingevidence *****************************
**
** Add an Ensembl Intron Supporting Evidence to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Transcript::add_IntronSupportingEvidence
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] ise [EnsPIntronsupportingevidence]
** Ensembl Intron Supporting Evidence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptAddIntronsupportingevidence(
    EnsPTranscript transcript,
    EnsPIntronsupportingevidence ise)
{
    AjBool match = AJFALSE;

    AjIList iter = NULL;

    EnsPIntronsupportingevidence oldise = NULL;

    if (!transcript)
        return ajFalse;

    if (!ise)
        return ajFalse;

    if (!transcript->Intronsupportingevidences)
        transcript->Intronsupportingevidences = ajListNew();

    iter = ajListIterNewread(transcript->Intronsupportingevidences);

    while (!ajListIterDone(iter))
    {
        oldise = (EnsPIntronsupportingevidence) ajListIterGet(iter);

        if ((match = ensIntronsupportingevidenceMatch(ise, oldise)))
            break;
    }

    ajListIterDel(&iter);

    if (match)
        return ajFalse;

    ajListPushAppend(transcript->Intronsupportingevidences,
                     (void *) ensIntronsupportingevidenceNewRef(ise));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Trace Report Ensembl Transcript members to debug file
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
**
** @release 6.2.0
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

    EnsPIntronsupportingevidence ise = NULL;

    EnsPTranslation translation = NULL;

    if (!transcript)
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
            "%S  Intronsupportingevidences %p\n"
            "%S  Supportingfeatures %p\n"
            "%S  Translation %p\n"
            "%S  Sequenceedits '%B'\n"
            "%S  Canonical %p\n"
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
            indent, transcript->Intronsupportingevidences,
            indent, transcript->Supportingfeatures,
            indent, transcript->Translation,
            indent, transcript->Sequenceedits,
            indent, transcript->Canonical,
            indent, transcript->ExonCoordMapper);

    ensFeatureTrace(transcript->Feature, level + 1);

    ensDatabaseentryTrace(transcript->Displayreference, level + 1);

    /* Trace the AJAX List of alternative Ensembl Translation objects. */

    if (transcript->Alternativetranslations)
    {
        ajDebug("%S    AJAX List %p of alternative "
                "Ensembl Translation objects:\n",
                indent, transcript->Alternativetranslations);

        iter = ajListIterNewread(transcript->Alternativetranslations);

        while (!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            ensTranslationTrace(translation, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Attribute objects. */

    if (transcript->Attributes)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Attribute objects\n",
                indent, transcript->Attributes);

        iter = ajListIterNewread(transcript->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if (transcript->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Database Entry objects\n",
                indent, transcript->Databaseentries);

        iter = ajListIterNewread(transcript->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Exon objects. */

    if (transcript->Exons)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Exon objects\n",
                indent, transcript->Exons);

        iter = ajListIterNewread(transcript->Exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            ensExonTrace(exon, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Intron Supporting Evidence objects. */

    if (transcript->Intronsupportingevidences)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Intron Supporting Evidence objects\n",
                indent, transcript->Intronsupportingevidences);

        iter = ajListIterNewread(transcript->Intronsupportingevidences);

        while (!ajListIterDone(iter))
        {
            ise = (EnsPIntronsupportingevidence) ajListIterGet(iter);

            ensIntronsupportingevidenceTrace(ise, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of supporting Ensembl Base Align Feature objects. */

    if (transcript->Supportingfeatures)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Base Align Feature objects\n",
                indent, transcript->Supportingfeatures);

        iter = ajListIterNewread(transcript->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ensBasealignfeatureTrace(baf, level + 2);
        }

        ajListIterDel(&iter);
    }

    ensTranslationTrace(transcript->Translation, level + 1);

    if (transcript->Canonical)
        ajDebug("%S    AJAX Boolean Canonical %p '%B'\n",
                indent, transcript->Canonical, *transcript->Canonical);

    ensMapperTrace(transcript->ExonCoordMapper, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Calculate Calculate Ensembl Transcript information
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
** @valrule Length [ajuint] Transcript (cDNA) length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule PhaseStart [ajint] Start phase or -1
** @valrule SliceCodingEnd [ajint] End coordinate or 0
** @valrule SliceCodingStart [ajint] Start coordinate or 0
** @valrule TranscriptCodingEnd [ajuint] End coordinate or 0U
** @valrule TranscriptCodingStart [ajuint] Start coordinate or 0U
** @valrule TranscriptEnd [ajuint] End coordinate or 0U
** @valrule TranscriptStart [ajuint] Start coordinate or 0U
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
**
** @release 6.2.0
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

    if (!transcript)
        return ajFalse;

    list = ensTranscriptLoadExons(transcript);

    if (!ajListGetLength(list))
        return ajTrue;

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        /* Skip missing Exon objects. */

        if (!exon)
            continue;

        efeature = ensExonGetFeature(exon);

        /* Skip un-mapped Exon objects. */

        if (!ensFeatureGetStart(efeature))
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

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        /* Skip missing Exon objects. */

        if (!exon)
            continue;

        efeature = ensExonGetFeature(exon);

        /* Skip un-mapped Exon objects. */

        if (!ensFeatureGetStart(efeature))
            continue;

        if (!ensSliceMatch(ensFeatureGetSlice(efeature), slice))
            ajFatal("ensTranscriptCalculateCoordinates got "
                    "Ensembl Exon objects of one "
                    "Ensembl Transcript on different "
                    "Ensembl Slice objects.\n");

        start = (ensFeatureGetStart(efeature) < start)
            ? ensFeatureGetStart(efeature) : start;

        end = (ensFeatureGetEnd(efeature) > end)
            ? ensFeatureGetEnd(efeature) : end;

        if (ensFeatureGetStrand(efeature) != strand)
            transsplicing = ajTrue;
    }

    ajListIterDel(&iter);

    if (transsplicing)
        ajWarn("ensTranscriptCalculateCoordinates got Transcript with "
               "trans-splicing event.\n");

    tfeature = ensTranscriptGetFeature(transcript);

    ensFeatureSetStart(tfeature, start);
    ensFeatureSetEnd(tfeature, end);
    ensFeatureSetStrand(tfeature, strand);
    ensFeatureSetSlice(tfeature, slice);

    /* Clear internal members that depend on Ensembl Exon coordinates. */

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
** @return [ajuint] Transcript (cDNA) length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensTranscriptCalculateLength(EnsPTranscript transcript)
{
    ajuint length = 0U;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    if (!transcript)
        return 0U;

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
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

    if (transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        while (ajListPop(ses, (void **) &se))
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
**
** @release 6.4.0
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

    EnsPIntronsupportingevidence ise = NULL;

    EnsPTranslation translation = NULL;

    if (!transcript)
        return 0;

    size += sizeof (EnsOTranscript);

    size += ensFeatureCalculateMemsize(transcript->Feature);

    size += ensDatabaseentryCalculateMemsize(transcript->Displayreference);

    if (transcript->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Description);
    }

    if (transcript->Biotype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Biotype);
    }

    if (transcript->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->Stableidentifier);
    }

    if (transcript->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->DateCreation);
    }

    if (transcript->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(transcript->DateModification);
    }

    /* Summarise the AJAX List of alternative Ensembl Translation objects. */

    if (transcript->Alternativetranslations)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Alternativetranslations);

        while (!ajListIterDone(iter))
        {
            translation = (EnsPTranslation) ajListIterGet(iter);

            size += ensTranslationCalculateMemsize(translation);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Attribute objects. */

    if (transcript->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if (transcript->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Exon objects. */

    if (transcript->Exons)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            size += ensExonCalculateMemsize(exon);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Intron Supporting Feature objects. */

    if (transcript->Intronsupportingevidences)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Intronsupportingevidences);

        while (!ajListIterDone(iter))
        {
            ise = (EnsPIntronsupportingevidence) ajListIterGet(iter);

            size += ensIntronsupportingevidenceCalculateMemsize(ise);
        }

        ajListIterDel(&iter);
    }

    /*
    ** Summarise the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    if (transcript->Supportingfeatures)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(transcript->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            size += ensBasealignfeatureCalculateMemsize(baf);
        }

        ajListIterDel(&iter);
    }

    size += ensTranslationCalculateMemsize(transcript->Translation);

    if (transcript->Canonical)
        size += sizeof (AjBool);

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
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensTranscriptCalculatePhaseStart(EnsPTranscript transcript)
{
    const AjPList exons = NULL;

    EnsPExon exon = NULL;

    if (!transcript)
        return -1;

    exons = ensTranscriptLoadExons(transcript);

    ajListPeekFirst(exons, (void **) &exon);

    if (!exon)
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
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensTranscriptCalculateSliceCodingEnd(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajint scend = 0;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    if (!transcript)
        return 0;

    if (!translation)
        return 0;

    /* Calculate the Slice coding start from the Translation. */

    exon = ensTranslationGetStartexon(translation);

    feature = ensExonGetFeature(exon);

    if (ensFeatureGetStrand(feature) >= 0)
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
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensTranscriptCalculateSliceCodingStart(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajint scstart = 0;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    if (!transcript)
        return 0;

    if (!translation)
        return 0;

    /* Calculate the Slice coding start from the Translation. */

    exon = ensTranslationGetStartexon(translation);

    feature = ensExonGetFeature(exon);

    if (ensFeatureGetStrand(feature) >= 0)
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
** @return [ajuint] Coding region end in Transcript coordinates or 0U,
** if this Transcript has no Translation
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensTranscriptCalculateTranscriptCodingEnd(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajuint tcend = 0U;

    AjBool debug = AJFALSE;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    debug = ajDebugTest("ensTranscriptCalculateTranscriptCodingEnd");

    if (debug)
    {
        ajDebug("ensTranscriptCalculateTranscriptCodingEnd\n"
                "  transcript %p\n"
                "  translation %p\n",
                transcript,
                translation);

        ensTranscriptTrace(transcript, 1);

        ensTranslationTrace(translation, 1);
    }

    if (!transcript)
        return 0U;

    if (!translation)
        return 0U;

    /*
    ** Calculate the coding start relative to the start of the
    ** Translation in Transcript coordinates.
    */

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if (debug)
            ajDebug("ensTranscriptCalculateTranscriptCodingEnd "
                    "exon %p (Identifier %u) end exon %p (Identifier %u)\n",
                    exon, ensExonGetIdentifier(exon),
                    ensTranslationGetEndexon(translation),
                    ensExonGetIdentifier(
                        ensTranslationGetEndexon(translation)));

        if (ensExonMatch(exon, ensTranslationGetEndexon(translation)))
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

    if (transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        /*
        ** Sort in reverse order to avoid adjustment of down-stream
        ** Sequence Edit objects.
        */

        ensListSequenceeditSortStartDescending(ses);

        while (ajListPop(ses, (void **) &se))
        {
            /*
            ** Use less than or equal to end + 1 so that the end of the
            ** CDS can be extended.
            */

            if (ensSequenceeditGetStart(se) <= tcend + 1)
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
** @return [ajuint] Coding region start in Transcript coordinates or 0U,
** if this Transcript has no Translation
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensTranscriptCalculateTranscriptCodingStart(
    EnsPTranscript transcript,
    EnsPTranslation translation)
{
    ajuint tcstart = 0U;

    AjBool debug = AJFALSE;

    AjIList iter = NULL;

    const AjPList exons = NULL;
    AjPList ses         = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    debug = ajDebugTest("ensTranscriptCalculateTranscriptCodingStart");

    if (debug)
    {
        ajDebug("ensTranscriptCalculateTranscriptCodingStart\n"
                "  transcript %p\n"
                "  translation %p\n",
                transcript,
                translation);

        ensTranscriptTrace(transcript, 1);

        ensTranslationTrace(translation, 1);
    }

    if (!transcript)
        return 0U;

    /*
    ** Calculate the coding start relative to the start of the
    ** Translation in Transcript coordinates.
    */

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if (debug)
            ajDebug("ensTranscriptCalculateTranscriptCodingStart "
                    "exon %p (Identifier %u) start exon %p (Identifier %u)\n",
                    exon, ensExonGetIdentifier(exon),
                    ensTranslationGetStartexon(translation),
                    ensExonGetIdentifier(
                        ensTranslationGetStartexon(translation)));

        if (ensExonMatch(exon, ensTranslationGetStartexon(translation)))
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

    if (transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        /*
        ** Sort in reverse order to avoid adjustment of down-stream
        ** Sequence Edit objects.
        */

        ensListSequenceeditSortStartDescending(ses);

        while (ajListPop(ses, (void **) &se))
        {
            if (ensSequenceeditGetStart(se) < tcstart)
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
**
** @release 6.4.0
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

    if (!transcript1)
        return ajFalse;

    if (!transcript2)
        return ajFalse;

    if (transcript1 == transcript2)
        return ajTrue;

    if (!ensFeatureSimilarity(transcript1->Feature, transcript2->Feature))
        return ajFalse;

    if (!ajStrMatchCaseS(transcript1->Biotype, transcript2->Biotype))
        return ajFalse;

    if ((transcript1->Stableidentifier && transcript2->Stableidentifier) &&
        (!ajStrMatchCaseS(transcript1->Stableidentifier,
                          transcript2->Stableidentifier)))
        return ajFalse;

    exons1 = ensTranscriptLoadExons(transcript1);
    exons2 = ensTranscriptLoadExons(transcript2);

    if (ajListGetLength(exons1) != ajListGetLength(exons2))
        return ajFalse;

    iter1 = ajListIterNewread(exons1);
    iter2 = ajListIterNewread(exons2);

    while (!ajListIterDone(iter1))
    {
        exon1 = (EnsPExon) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        similarity = ajFalse;

        while (!ajListIterDone(iter2))
        {
            exon2 = (EnsPExon) ajListIterGet(iter2);

            if (ensExonSimilarity(exon1, exon2))
            {
                similarity = ajTrue;
                break;
            }
        }

        if (similarity == ajFalse)
        {
            mismatch = ajTrue;
            break;
        }
    }

    ajListIterDel(&iter1);
    ajListIterDel(&iter2);

    if (mismatch == ajTrue)
        return ajFalse;

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Transcript objects between
** Ensembl Coordinate System objects.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Transfer Transfer an Ensembl Transcript
** @nam3rule Transform Transform an Ensembl Transcript
**
** @argrule * transcript [EnsPTranscript] Ensembl Transcript
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr]
** Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr]
** Ensembl Coordinate System version
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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranscript ensTranscriptTransfer(EnsPTranscript transcript,
                                     EnsPSlice slice)
{
    EnsPFeature newtf = NULL;

    EnsPTranscript newtranscript = NULL;

    if (ajDebugTest("ensTranscriptTransfer"))
        ajDebug("ensTranscriptTransfer\n"
                "  transcript %p\n"
                "  slice %p\n",
                transcript,
                slice);

    if (!transcript)
        return NULL;

    if (!slice)
        return NULL;

    if (!transcript->Feature)
        ajFatal("ensTranscriptTransfer cannot transfer an Ensembl Transcript "
                "without an Ensembl Feature.\n");

    newtf = ensFeatureTransfer(transcript->Feature, slice);

    if (!newtf)
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
**
** @release 6.2.0
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

    ajuint psslength = 0U;

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

    if (!transcript)
        return NULL;

    if (!csname)
        return NULL;

    newtf = ensFeatureTransform(transcript->Feature,
                                csname,
                                csversion,
                                (EnsPSlice) NULL);

    if (!newtf)
    {
        /*
        ** If the Ensembl Transcript does not transform, test whether it
        ** projects, since its Ensembl Exon objects may transform well.
        */

        pss = ajListNew();

        ensFeatureProject(transcript->Feature, csname, csversion, pss);

        psslength = (ajuint) ajListGetLength(pss);

        while (ajListPop(pss, (void **) &ps))
            ensProjectionsegmentDel(&ps);

        ajListFree(&pss);

        /*
        ** If the AJAX List of Ensembl Projection Segment objects is empty,
        ** the Transcript did not project. If it projects, make sure
        ** all its Ensembl Exon objects are loaded.
        */

        if (psslength == 0)
            return NULL;
        else
            ensTranscriptLoadExons(transcript);
    }

    /*
    ** Copy the Ensembl Transcript object and also its internal
    ** Ensembl Feature class-based members.
    */

    newtranscript = transcriptNewCpyFeatures(transcript);

    if (newtranscript->Exons)
    {
        first = ajTrue;

        iter1 = ajListIterNew(newtranscript->Exons);

        while (!ajListIterDone(iter1))
        {
            /*
            ** We want to check, whether the transform preserved the
            ** 5 prime to 3 prime order. No complaints on trans-splicing.
            */

            oldexon = (EnsPExon) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newexon = ensExonTransform(oldexon, csname, csversion);

            if (!newexon)
            {
                error = ajTrue;

                break;
            }

            ajListIterInsert(iter1, (void *) newexon);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            if (!newtf)
            {
                oldef = ensExonGetFeature(oldexon);
                newef = ensExonGetFeature(newexon);

                if (first)
                {
                    minstart = ensFeatureGetStart(newef);
                    maxend   = ensFeatureGetEnd(newef);

                    first = ajFalse;
                }
                else
                {
                    newsr = ensFeatureGetSeqregionObject(newef);

                    if (!ensSeqregionMatch(newsr, lastnewsr))
                    {
                        error = ajTrue;

                        break;
                    }

                    /*
                    ** Test for a trans-splicing event and ignore the Exon
                    ** order in those cases.
                    */

                    if (ensFeatureGetStrand(oldef) != lastoldstrand)
                        ignoreorder = ajTrue;

                    /* Check for correct Exon order. */

                    if ((lastnewstrand >= 0) &&
                        (ensFeatureGetStart(newef) < lastnewstart))
                        orderbroken = ajTrue;

                    if ((lastnewstrand < 0) &&
                        (ensFeatureGetStart(newef) > lastnewstart))
                        orderbroken = ajTrue;

                    /*
                    ** Additional check that if Ensembl Exon objects were on
                    ** the same strand previously, they should be again.
                    */
                    if ((lastoldstrand == ensFeatureGetStrand(oldef)) &&
                        !(lastnewstrand == ensFeatureGetStrand(newef)))
                    {
                        error = ajTrue;

                        break;
                    }

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

            if (newtranscript->Translation)
            {
                if (ensExonMatch(
                        oldexon,
                        ensTranslationGetStartexon(newtranscript->Translation)))
                    ensTranslationSetStartexon(newtranscript->Translation,
                                               newexon);

                if (ensExonMatch(
                        oldexon,
                        ensTranslationGetEndexon(newtranscript->Translation)))
                    ensTranslationSetEndexon(newtranscript->Translation,
                                             newexon);
            }

            if (newtranscript->Alternativetranslations)
            {
                iter2 = ajListIterNew(newtranscript->Alternativetranslations);

                while (!ajListIterDone(iter2))
                {
                    translation = (EnsPTranslation) ajListIterGet(iter2);

                    if (ensExonMatch(
                            oldexon,
                            ensTranslationGetStartexon(translation)))
                        ensTranslationSetStartexon(translation, newexon);

                    if (ensExonMatch(
                            oldexon,
                            ensTranslationGetEndexon(translation)))
                        ensTranslationSetEndexon(translation, newexon);
                }

                ajListIterDel(&iter2);
            }

            ensExonDel(&oldexon);
        }

        ajListIterDel(&iter1);

        if (orderbroken && (!ignoreorder))
        {
            ajWarn("ensTranscriptTransform got Ensembl Exon objects out of "
                   "order in the transformation of Ensembl Transcript with "
                   "identifier %u.\n",
                   transcript->Identifier);

            error = ajTrue;
        }

        if (error)
        {
            ensFeatureDel(&newtf);

            ensTranscriptDel(&newtranscript);

            return NULL;
        }
    }

    if (!newtf)
    {
        ajListPeekFirst(newtranscript->Exons, (void **) &newexon);

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

    if (newtranscript->Supportingfeatures)
    {
        iter1 = ajListIterNew(newtranscript->Supportingfeatures);

        while (!ajListIterDone(iter1))
        {
            oldbaf = (EnsPBasealignfeature) ajListIterGet(iter1);

            ajListIterRemove(iter1);

            newbaf = ensBasealignfeatureTransform(oldbaf, csname, csversion);

            if (!newbaf)
            {
                ajDebug("ensTranscriptTransform could not transfer "
                        "Base Align Feature onto new Ensembl Feature Slice.");

                ensBasealignfeatureTrace(oldbaf, 1);
            }

            ajListIterInsert(iter1, (void *) newbaf);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter1);

            ensBasealignfeatureDel(&oldbaf);
        }

        ajListIterDel(&iter1);
    }

    /* Clear internal members that depend on Ensembl Exon coordinates. */

    ensMapperClear(newtranscript->ExonCoordMapper);

    return newtranscript;
}




/* @section fetch *************************************************************
**
** Functions for fetching information from an Ensembl Transcript object.
**
** @fdata [EnsPTranscript]
**
** @nam3rule Fetch Fetch Ensembl Transcript information
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Constitutiveexons Fetch all constitutive Ensembl Exon objects
** @nam5rule Databaseentries Fetch all Ensembl Database Entry objects
** @nam5rule Introns Fetch all Ensembl Intron objects
** @nam5rule Sequenceedits Fetch all Ensembl Sequence Edit objects
** @nam4rule Displayidentifier Fetch the display identifier
** @nam4rule Feature     Fetch the Ensembl Feature
** @nam4rule Gene        Fetch the Ensembl Gene
** @nam4rule Sequence    Fetch the sequence
** @nam5rule Coding      Fetch the coding sequence
** @nam5rule Transcript  Fetch the Ensembl Transcript (cDNA) sequence
** @nam5rule Utrfive     Fetch the five-prime untranslated region
** @nam5rule Utrthree    Fetch the three-prime untranslated region
** @nam6rule Seq         Fetch as AJAX Sequence object
** @nam6rule Str         Fetch as AJAX String object
** @nam5rule Translation Fetch the Ensembl Translation sequence
** @nam6rule Seq         Fetch as AJAX Sequence object
** @nam6rule Str         Fetch as AJAX String object
**
** @argrule AllAttributes transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllConstitutiveexons transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllConstitutiveexons exons [AjPList]
** AJAX List of Ensembl Exon objects
** @argrule AllDatabaseentries transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllDatabaseentries name [const AjPStr]
** Ensembl External Database name
** @argrule AllDatabaseentries type [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllDatabaseentries dbes [AjPList]
** AJAX List of Ensembl Database Entry objects
** @argrule AllIntrons transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllIntrons introns [AjPList] AJAX List of Ensembl Intron objects
** @argrule AllSequenceedits transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllSequenceedits ses [AjPList]
** AJAX List of Ensembl Sequence Edit objects
** @argrule Displayidentifier transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule Displayidentifier Pidentifier [AjPStr*] AJAX String address
** @argrule Feature transcript [EnsPTranscript] Ensembl Transcript
** @argrule Feature translation [EnsPTranslation] Ensembl Translation
** @argrule Feature Pfeature [EnsPFeature*] Ensembl Feature address
** @argrule Gene transcript [const EnsPTranscript] Ensembl Transcript
** @argrule Gene Pgene [EnsPGene*] Ensembl Gene address
** @argrule Sequence transcript [EnsPTranscript] Ensembl Transcript
** @argrule SequenceCoding translation [EnsPTranslation] Ensembl Translation
** @argrule SequenceTranslation translation [EnsPTranslation]
** Ensembl Translation
** @argrule SequenceUtrfive translation [EnsPTranslation]
** Ensembl Translation
** @argrule SequenceUtrthree translation [EnsPTranslation]
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
** @param [rN] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
** @see ensTranscriptLoadAttributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
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

    if (!transcript)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    list = ensTranscriptLoadAttributes(transcript);

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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllConstitutiveexons(EnsPTranscript transcript,
                                              AjPList exons)
{
    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPExon exon = NULL;

    if (!transcript)
        return ajFalse;

    if (!exons)
        return ajFalse;

    list = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        if (ensExonGetConstitutive(exon))
            ajListPushAppend(exons, (void *) ensExonNewRef(exon));
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
** @param [rN] name [const AjPStr] Ensembl External Database name
** @param [uN] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry object
** @see ensTranscriptLoadDatabaseentries
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
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

    if (!transcript)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    list = ensTranscriptLoadDatabaseentries(transcript);

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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchAllIntrons(EnsPTranscript transcript,
                                    AjPList introns)
{
    register ajuint i = 0U;

    const AjPList list = NULL;

    EnsPExon exon1 = NULL;
    EnsPExon exon2 = NULL;

    EnsPIntron intron = NULL;

    if (!transcript)
        return ajFalse;

    if (!introns)
        return ajFalse;

    list = ensTranscriptLoadExons(transcript);

    for (i = 0U; i < (ajListGetLength(list) - 1); i++)
    {
        ajListPeekNumber(list, i,     (void **) &exon1);
        ajListPeekNumber(list, i + 1, (void **) &exon2);

        intron = ensIntronNewIni(exon1, exon2, (EnsPAnalysis) NULL);

        ajListPushAppend(introns, (void *) intron);
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
**
** @release 6.4.0
** @@
** Ensembl Sequence Edit objects are Ensembl Attribute objects, which codes
** have to be defined in the
** static const char *const transcriptKSequenceeditCode[] array.
******************************************************************************/

AjBool ensTranscriptFetchAllSequenceedits(EnsPTranscript transcript,
                                          AjPList ses)
{
    register ajuint i = 0U;

    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute at = NULL;

    EnsPSequenceedit se = NULL;

    if (!transcript)
        return ajFalse;

    if (!ses)
        return ajFalse;

    code = ajStrNew();

    attributes = ajListNew();

    for (i = 0U; transcriptKSequenceeditCode[i]; i++)
    {
        ajStrAssignC(&code, transcriptKSequenceeditCode[i]);

        ensTranscriptFetchAllAttributes(transcript, code, attributes);
    }

    while (ajListPop(attributes, (void **) &at))
    {
        se = ensSequenceeditNewAttribute(at);

        ajListPushAppend(ses, (void *) se);

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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchDisplayidentifier(const EnsPTranscript transcript,
                                           AjPStr *Pidentifier)
{
    if (!transcript)
        return ajFalse;

    if (!Pidentifier)
        return ajFalse;

    if (transcript->Stableidentifier &&
        ajStrGetLen(transcript->Stableidentifier))
    {
        if (*Pidentifier)
            ajStrAssignS(Pidentifier, transcript->Stableidentifier);
        else
            *Pidentifier = ajStrNewS(transcript->Stableidentifier);
    }
    else if (transcript->Identifier)
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier,
                                       "%u", transcript->Identifier);
        else
            *Pidentifier = ajFmtStr("%u", transcript->Identifier);
    }
    else
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier, "%p", transcript);
        else
            *Pidentifier = ajFmtStr("%p", transcript);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchFeatureUtrfive *************************************
**
** Fetch an Ensembl Feature spanning the five prime untranslated region (UTR)
** of an Ensembl Translation of an Ensembl Transcript an Ensembl Feature.
**
** The caller is responsible for deleting the Ensembl Feature.
**
** @cc Bio::EnsEMBL::Transcript::five_prime_utr_Feature
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Pfeature [EnsPFeature*] Ensembl Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchFeatureUtrfive(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        EnsPFeature *Pfeature)
{
    ajint ftstart  = 0;
    ajint ftend    = 0;

    ajint slcstart = 0;

    EnsPFeature feature = NULL;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!Pfeature)
        return ajFalse;

    *Pfeature = NULL;

    feature = transcript->Feature;

    slcstart = ensTranscriptCalculateSliceCodingStart(transcript, translation);

    if (ensFeatureGetStrand(feature) >= 0)
    {
        ftstart = ensFeatureGetStart(feature);
        ftend   = slcstart - 1;
    }
    else
    {
        ftstart = slcstart + 1;
        ftend   = ensFeatureGetEnd(feature);
    }

    if (ftstart < ftend)
        *Pfeature = ensFeatureNewIniS(
            ensFeatureGetAnalysis(feature),
            ensFeatureGetSlice(feature),
            ftstart,
            ftend,
            ensFeatureGetStrand(feature));

    return ajTrue;
}




/* @func ensTranscriptFetchFeatureUtrthree ************************************
**
** Fetch an Ensembl Feature spanning the three prime untranslated region (UTR)
** of an Ensembl Translation of an Ensembl Transcript an Ensembl Feature.
**
** The caller is responsible for deleting the Ensembl Feature.
**
** @cc Bio::EnsEMBL::Transcript::three_prime_utr_Feature
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Pfeature [EnsPFeature*] Ensembl Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchFeatureUtrthree(EnsPTranscript transcript,
                                         EnsPTranslation translation,
                                         EnsPFeature *Pfeature)
{
    ajint ftstart  = 0;
    ajint ftend    = 0;

    ajint slcend = 0;

    EnsPFeature feature = NULL;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!Pfeature)
        return ajFalse;

    *Pfeature = NULL;

    feature = transcript->Feature;

    slcend = ensTranscriptCalculateSliceCodingEnd(transcript, translation);

    if (ensFeatureGetStrand(feature) >= 0)
    {
        ftstart = slcend + 1;
        ftend   = ensFeatureGetEnd(feature);
    }
    else
    {
        ftstart = ensFeatureGetStart(feature);
        ftend   = slcend - 1;
    }

    if (ftstart < ftend)
        *Pfeature = ensFeatureNewIniS(
            ensFeatureGetAnalysis(feature),
            ensFeatureGetSlice(feature),
            ftstart,
            ftend,
            ensFeatureGetStrand(feature));

    return ajTrue;
}




/* @func ensTranscriptFetchGene ***********************************************
**
** Fetch the Ensembl Gene of an Ensembl Transcript.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::Transcript::get_Gene
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchGene(const EnsPTranscript transcript,
                              EnsPGene *Pgene)
{
    if (!transcript)
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    *Pgene = NULL;

    return ensGeneadaptorFetchByTranscriptidentifier(
        ensRegistryGetGeneadaptor(
            ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor)),
        transcript->Identifier,
        Pgene);
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceCodingStr(EnsPTranscript transcript,
                                           EnsPTranslation translation,
                                           AjPStr *Psequence)
{
    ajint sphase = 0;

    ajuint cdsstart = 0U;
    ajuint cdsend   = 0U;

    AjPStr sequence = NULL;

    if (!transcript)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /*
    ** Return an empty string for non-coding Ensembl Transcript objects.
    ** The ensTranscriptLoadTranslation function will attempt to load the
    ** Translation from the database.
    */

    if (!translation)
    {
        ajDebug("ensTranscriptFetchSequenceCodingStr got a Transcript "
                "without a Translation.\n");

        return ajTrue;
    }

    /* Calculate coding start and end coordinates. */

    cdsstart = ensTranscriptCalculateTranscriptCodingStart(transcript,
                                                           translation);

    if (!cdsstart)
    {
        ajDebug("ensTranscriptFetchSequenceCodingStr got a Transcript and "
                "Translation pair with an invalid cdsstart %u.\n", cdsstart);

        return ajTrue;
    }

    cdsend = ensTranscriptCalculateTranscriptCodingEnd(transcript,
                                                       translation);

    if (!cdsend)
    {
        ajDebug("ensTranscriptFetchSequenceCodingStr got a Transcript and "
                "Translation pair with an invalid cdsend %u.\n", cdsend);

        return ajTrue;
    }

    sphase = ensExonGetPhaseStart(ensTranslationGetStartexon(translation));

    if (sphase > 0)
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranscriptSeq(EnsPTranscript transcript,
                                               AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!transcript)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    ensTranscriptFetchDisplayidentifier(transcript, &name);
    ensTranscriptFetchSequenceTranscriptStr(transcript, &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranscriptStr(EnsPTranscript transcript,
                                               AjPStr *Psequence)
{
    AjIList iter        = NULL;
    const AjPList exons = NULL;
    AjPList ses         = NULL;

    AjPStr sequence = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPSequenceedit se = NULL;

    if (!transcript)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    sequence = ajStrNew();

    exons = ensTranscriptLoadExons(transcript);

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        ensExonFetchSequenceSliceStr(exon, &sequence);

        if (sequence && ajStrGetLen(sequence))
            ajStrAppendS(Psequence, sequence);
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
    }

    ajListIterDel(&iter);

    ajStrDel(&sequence);

    /* Apply post-transcriptional Sequence Edit objects if enabled. */

    if (transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranscriptFetchAllSequenceedits(transcript, ses);

        /*
        ** Sort Sequence Edit objects in reverse order to avoid the
        ** complication of adjusting down-stream Sequence Edit coordinates.
        */

        ensListSequenceeditSortStartDescending(ses);

        while (ajListPop(ses, (void **) &se))
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranslationSeq(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajTrue;

    if (!Psequence)
        return ajFalse;

    name     = ajStrNew();
    sequence = ajStrNew();

    ensTranslationFetchDisplayidentifier(translation, &name);
    ensTranscriptFetchSequenceTranslationStr(transcript,
                                             translation,
                                             &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceTranslationStr(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPStr *Psequence)
{
    AjPList ses = NULL;

    AjPStr cdna = NULL;

    const AjPTrn trn = NULL;

    EnsPSequenceedit se = NULL;

    EnsPSlice slice = NULL;

    if (ajDebugTest("ensTranscriptFetchSequenceTranslationStr"))
        ajDebug("ensTranscriptFetchSequenceTranslationStr\n"
                "  transcript %p\n"
                "  translation %p\n"
                "  Psequence %p\n",
                transcript,
                translation,
                Psequence);

    if (!transcript)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /*
    ** Return an empty string for non-coding Ensembl Transcript objects.
    ** The ensTranscriptLoadTranslation function will attempt to load the
    ** Translation from the database.
    */

    if (!translation)
    {
        ajDebug("ensTranscriptFetchSequenceTranslationStr got a Transcript "
                "without a Translation.\n");

        return ajTrue;
    }

    cdna = ajStrNew();

    ensTranscriptFetchSequenceCodingStr(transcript, translation, &cdna);

    if (ajStrGetLen(cdna) < 1)
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
    ** Sequence Region Attribute objects to modify the translated sequence.
    ** The initiator codon should be correctly translated by ajTrnSeqS based
    ** on the codon table and the stop codon, if present, is removed above.
    */

    if (ajStrGetCharLast(*Psequence) == '*')
        ajStrCutEnd(Psequence, 1);

    /* Apply post-translational Sequence Edit objects if enabled. */

    if (transcript->Sequenceedits)
    {
        ses = ajListNew();

        ensTranslationFetchAllSequenceedits(translation, ses);

        /*
        ** Sort Sequence Edit objects in reverse order to avoid the
        ** complication of adjusting down-stream Sequence Edit coordinates.
        */

        ensListSequenceeditSortStartDescending(ses);

        while (ajListPop(ses, (void **) &se))
        {
            ensSequenceeditApplyString(se, 0, Psequence);

            ensSequenceeditDel(&se);
        }

        ajListFree(&ses);
    }

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceUtrfiveStr *********************************
**
** Fetch the sequence of the five prime untranslated region of an
** Ensembl Translation of an Ensembl Transcript as AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::five_prime_utr
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceUtrfiveStr(EnsPTranscript transcript,
                                            EnsPTranslation translation,
                                            AjPStr *Psequence)
{
    ajuint trcstart = 0U;

    AjPStr sequence = NULL;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);

    ensTranscriptFetchSequenceTranscriptStr(transcript, &sequence);

    trcstart = ensTranscriptCalculateTranscriptCodingStart(transcript,
                                                           translation);

    ajStrAssignSubS(Psequence, sequence, 0, trcstart - 1);

    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranscriptFetchSequenceUtrthreeStr ********************************
**
** Fetch the sequence of the three prime untranslated region of an
** Ensembl Translation of an Ensembl Transcript as AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Transcript::three_prime_utr
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptFetchSequenceUtrthreeStr(EnsPTranscript transcript,
                                             EnsPTranslation translation,
                                             AjPStr *Psequence)
{
    ajuint trcend = 0U;

    AjPStr sequence = NULL;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);

    ensTranscriptFetchSequenceTranscriptStr(transcript, &sequence);

    trcend = ensTranscriptCalculateTranscriptCodingEnd(transcript,
                                                       translation);

    ajStrAssignSubS(Psequence, sequence, trcend + 1, ajStrGetLen(sequence));

    ajStrDel(&sequence);

    return ajTrue;
}




/* @section mapper ************************************************************
**
** Ensembl Transcript Mapper functions.
**
** @fdata [EnsPTranscript]
**
** @cc Bio::EnsEMBL::TranscriptMapper
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-68
**
** @nam3rule Mapper Ensembl Transcript Mapper functions
** @nam4rule Init          Initialise an Ensembl Transcript Mapper
** @nam4rule Coding        Map from coding coordinates
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
** @argrule Coding  translation [EnsPTranslation] Ensembl Translation
** @argrule Tocoding translation [EnsPTranslation] Ensembl Translation
** @argrule Totranslation translation [EnsPTranslation] Ensembl Translation
** @argrule Translation translation [EnsPTranslation] Ensembl Translation
** @argrule Slice start  [ajint] Ensembl Slice start coordinate
** @argrule Slice end    [ajint] Ensembl Slice end coordinate
** @argrule Slice strand [ajint] Ensembl Slice strand information
** @argrule Slice mrs [AjPList] AJAX List of Ensembl Mapper Result objects
** @argrule CodingToslice start [ajint] Transcript start coordinate
** @argrule CodingToslice end   [ajint] Transcript end coordinate
** @argrule CodingToslice mrs   [AjPList] AJAX List of
**                                            Ensembl Mapper Result objects
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




/* @func ensTranscriptMapperCodingToslice *************************************
**
** Map coding sequence coordinates of an Ensembl Transcript into
** Slice (genome) coordinates.
**
** @cc Bio::EnsEMBL::TranscriptMapper::cds2genomic
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
** @param [r] start  [ajint] Ensembl Slice start coordinate
** @param [r] end    [ajint] Ensembl Slice end coordinate
** @param [u] mrs    [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptMapperCodingToslice(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        ajint start,
                                        ajint end,
                                        AjPList mrs)
{
    ajuint tcs = 0U;

    if (!transcript)
        return ajFalse;

    if (!mrs)
        return ajFalse;

    tcs = ensTranscriptCalculateTranscriptCodingStart(transcript, translation);

    /* Move start end into translate cDNA coordinates now. */

    return ensTranscriptMapperTranscriptToslice(transcript,
                                                start + tcs - 1,
                                                end   + tcs - 1,
                                                mrs);
}




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
**
** @release 6.2.0
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
    ajuint srid = 0U;

    /* Current and previous Feature (Exon) Slice coordinates */

    ajint curftrstart  = 0;
    ajint curftrend    = 0;
    ajint curftrstrand = 0;

    ajint prvftrstart = 0;
    ajint prvftrend   = 0;

    /* Current and previous Transcript (cDNA) coordinates */

    ajuint curtrcstart = 0U;
    ajuint curtrcend   = 0U;

    ajuint prvtrcstart  = 0U;
    ajuint prvtrcend    = 0U;
    ajuint prvtrclength = 0U;

    ajuint editshift = 0U;

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

    if (ajDebugTest("ensTranscriptMapperInit"))
        ajDebug("ensTranscriptMapperInit\n"
                "  transcript %p\n",
                transcript);

    if (!transcript)
        return ajFalse;

    if (transcript->ExonCoordMapper)
    {
        if (force == ajTrue)
            ensMapperClear(transcript->ExonCoordMapper);

        if (ensMapperGetCount(transcript->ExonCoordMapper) > 0)
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

    if (transcript->Sequenceedits)
    {
        ensTranscriptFetchAllSequenceedits(transcript, ses);

        ensListSequenceeditSortStartAscending(ses);
    }

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
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

        if (transcript->Sequenceedits)
        {
            while (ajListPeekFirst(ses, (void **) &se) &&
                   (ensSequenceeditGetStart(se) + editshift <= curtrcend))
            {
                if (ensSequenceeditCalculateDifference(se))
                {
                    /*
                    ** Break the Mapper Pair into two parts, finish the
                    ** first Mapper Pair just before the Sequence Edit.
                    */

                    prvtrcend    = ensSequenceeditGetStart(se) + editshift - 1;
                    prvtrcstart  = curtrcstart;
                    prvtrclength = prvtrcend - prvtrcstart + 1;

                    if (curftrstrand >= 0)
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

                    if (prvtrclength > 0)
                        ensMapperAddCoordinates(transcript->ExonCoordMapper,
                                                transcript->Identifier,
                                                prvtrcstart,
                                                prvtrcend,
                                                curftrstrand,
                                                srid,
                                                prvftrstart,
                                                prvftrend);

                    curtrcstart = prvtrcend + 1;

                    if (curftrstrand >= 0)
                        curftrstart = prvftrend   + 1;
                    else
                        curftrend   = prvftrstart - 1;

                    curtrcend += ensSequenceeditCalculateDifference(se);

                    if (ensSequenceeditCalculateDifference(se) >= 0)
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

                        if (curftrstrand >= 0)
                            curftrstart
                                -= ensSequenceeditCalculateDifference(se);
                        else
                            curftrend
                                += ensSequenceeditCalculateDifference(se);
                    }

                    editshift += ensSequenceeditCalculateDifference(se);
                }

                /* At this stage remove the Sequence Edit from the List. */

                ajListPop(ses, (void **) &se);

                ensSequenceeditDel(&se);
            }
        }

        /*
        ** Set the Transcript identifier as the source object identifier and
        ** the Sequence Region identifier of the Slice on which this Transcript
        ** is annotated as the target object identifier.
        */

        if ((curtrcend - curtrcstart + 1) > 0)
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

    while (ajListPop(ses, (void **) &se))
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptMapperSliceTocoding(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        ajint start,
                                        ajint end,
                                        ajint strand,
                                        AjPList mrs)
{
    ajuint cdsstart = 0U;
    ajuint cdsend   = 0U;
    ajuint tcstart  = 0U;
    ajuint tcend    = 0U;

    AjPList result = NULL;

    EnsPMapperresult mr     = NULL;
    EnsPMapperresult gcmr   = NULL;
    EnsPMapperresult endgap = NULL;

    if (!transcript)
        return ajFalse;

    if (!mrs)
    {
        ajDebug("ensTranscriptMapperSliceTocoding "
                "requires an AJAX List of Ensembl Mapper Results.\n");

        return ajFalse;
    }

    if (start > (end + 1))
        ajFatal("ensTranscriptMapperSliceTocoding requires start %d to be "
                "less than end %d + 1.\n", start, end);

    /*
    ** For non-coding Ensembl Transcript objects return an
    ** Ensembl Mapper Result of type ensEMapperresultTypeGap,
    ** as there is no coding sequence.
    */

    if (!translation)
    {
        mr = ensMapperresultNewGap(start, end, 0);

        ajListPushAppend(mrs, (void *) mr);

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

    while (ajListPop(result, (void **) &gcmr))
    {
        if (ensMapperresultGetType(gcmr) == ensEMapperresultTypeGap)
            ajListPushAppend(mrs, (void *) gcmr);
        else
        {
            if ((ensMapperresultGetCoordinateStrand(gcmr) < 0) ||
                (ensMapperresultGetCoordinateEnd(gcmr)    < (ajint) tcstart) ||
                (ensMapperresultGetCoordinateStart(gcmr)  > (ajint) tcend))
            {
                /* All gap - does not map to peptide. */

                mr = ensMapperresultNewGap(start, end, 0);

                ajListPushAppend(mrs, (void *) mr);
            }
            else
            {
                /* We know area is at least partially overlapping CDS. */

                cdsstart = ensMapperresultGetCoordinateStart(gcmr)
                    - tcstart + 1;

                cdsend   = ensMapperresultGetCoordinateEnd(gcmr)
                    - tcstart + 1;

                if (ensMapperresultGetCoordinateStart(gcmr) < (ajint) tcstart)
                {
                    /* Start coordinate is in the 5' UTR. */

                    mr = ensMapperresultNewGap(
                        ensMapperresultGetCoordinateStart(gcmr),
                        tcstart - 1,
                        0);

                    ajListPushAppend(mrs, (void *) mr);

                    /* Start is now relative to start of CDS. */

                    cdsstart = 1;
                }

                endgap = NULL;

                if (ensMapperresultGetCoordinateEnd(gcmr) > (ajint) tcend)
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

                ajListPushAppend(mrs, (void *) mr);

                if (endgap)
                    ajListPushAppend(mrs, (void *) endgap);
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
**
** @release 6.4.0
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
    ajuint srid = 0U;

    AjPStr src = NULL;

    EnsPSlice slice = NULL;

    if (!transcript)
        return ajFalse;

    if (!mrs)
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
**
** @release 6.4.0
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

    if (!transcript)
        return ajFalse;

    if (!mrs)
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

    while (ajListPop(result, (void **) &gcmr))
    {
        if (ensMapperresultGetType(gcmr) == ensEMapperresultTypeGap)
            ajListPushAppend(mrs, (void *) gcmr);
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

            ajListPushAppend(mrs, (void *) mr);

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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptMapperTranscriptToslice(EnsPTranscript transcript,
                                            ajuint start,
                                            ajuint end,
                                            AjPList mrs)
{
    AjPStr src = NULL;

    if (!transcript)
        return ajFalse;

    if (!mrs)
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
**
** @release 6.4.0
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

    if (!transcript)
        return ajFalse;

    if (!mrs)
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
** @valrule * [EnsETranscriptStatus]
** Ensembl Transcript Status enumeration or ensETranscriptStatusNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensTranscriptStatusFromStr *******************************************
**
** Return an Ensembl Transcript Status enumeration from an AJAX String.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsETranscriptStatus]
** Ensembl Transcript Status enumeration or ensETranscriptStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsETranscriptStatus ensTranscriptStatusFromStr(const AjPStr status)
{
    register EnsETranscriptStatus i = ensETranscriptStatusNULL;

    EnsETranscriptStatus estatus = ensETranscriptStatusNULL;

    for (i = ensETranscriptStatusNULL;
         transcriptKStatus[i];
         i++)
        if (ajStrMatchC(status, transcriptKStatus[i]))
            estatus = i;

    if (!estatus)
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
** Cast an Ensembl Transcript Status enumeration into a C-type (char *) string.
**
** @param [u] status [EnsETranscriptStatus]
** Ensembl Transcript Status enumeration
**
** @return [const char*]
** Ensembl Transcript status C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensTranscriptStatusToChar(EnsETranscriptStatus status)
{
    register EnsETranscriptStatus i = ensETranscriptStatusNULL;

    for (i = ensETranscriptStatusNULL;
         transcriptKStatus[i] && (i < status);
         i++);

    if (!transcriptKStatus[i])
        ajDebug("ensTranscriptStatusToChar "
                "encountered an out of boundary error on "
                "Ensembl Transcript Status "
                "enumeration %d.\n",
                status);

    return transcriptKStatus[i];
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listTranscriptCompareEndAscending ******************************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** Ensembl Feature end member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Transcript address 1
** @param [r] item2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listTranscriptCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPTranscript transcript1 = *(EnsOTranscript *const *) item1;
    EnsPTranscript transcript2 = *(EnsOTranscript *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listTranscriptCompareEndAscending"))
        ajDebug("listTranscriptCompareEndAscending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (transcript1 && (!transcript2))
        return -1;

    if ((!transcript1) && (!transcript2))
        return 0;

    if ((!transcript1) && transcript2)
        return +1;

    return ensFeatureCompareEndAscending(transcript1->Feature,
                                         transcript2->Feature);
}




/* @funcstatic listTranscriptCompareEndDescending *****************************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** Ensembl Feature end member in descending order.
**
** @param [r] item1 [const void*] Ensembl Transcript address 1
** @param [r] item2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listTranscriptCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPTranscript transcript1 = *(EnsOTranscript *const *) item1;
    EnsPTranscript transcript2 = *(EnsOTranscript *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listTranscriptCompareEndDescending"))
        ajDebug("listTranscriptCompareEndDescending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (transcript1 && (!transcript2))
        return -1;

    if ((!transcript1) && (!transcript2))
        return 0;

    if ((!transcript1) && transcript2)
        return +1;

    return ensFeatureCompareEndDescending(transcript1->Feature,
                                          transcript2->Feature);
}




/* @funcstatic listTranscriptCompareIdentifierAscending ***********************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Transcript address 1
** @param [r] item2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listTranscriptCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPTranscript transcript1 = *(EnsOTranscript *const *) item1;
    EnsPTranscript transcript2 = *(EnsOTranscript *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listTranscriptCompareIdentifierAscending"))
        ajDebug("listTranscriptCompareIdentifierAscending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (transcript1 && (!transcript2))
        return -1;

    if ((!transcript1) && (!transcript2))
        return 0;

    if ((!transcript1) && transcript2)
        return +1;

    if (transcript1->Identifier < transcript2->Identifier)
        return -1;

    if (transcript1->Identifier > transcript2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listTranscriptCompareStartAscending ****************************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** Ensembl Feature start member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Transcript address 1
** @param [r] item2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listTranscriptCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPTranscript transcript1 = *(EnsOTranscript *const *) item1;
    EnsPTranscript transcript2 = *(EnsOTranscript *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listTranscriptCompareStartAscending"))
        ajDebug("listTranscriptCompareStartAscending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (transcript1 && (!transcript2))
        return -1;

    if ((!transcript1) && (!transcript2))
        return 0;

    if ((!transcript1) && transcript2)
        return +1;

    return ensFeatureCompareStartAscending(transcript1->Feature,
                                           transcript2->Feature);
}




/* @funcstatic listTranscriptCompareStartDescending ***************************
**
** AJAX List of Ensembl Transcript objects comparison function to sort by
** Ensembl Feature start member in descending order.
**
** @param [r] item1 [const void*] Ensembl Transcript address 1
** @param [r] item2 [const void*] Ensembl Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listTranscriptCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPTranscript transcript1 = *(EnsOTranscript *const *) item1;
    EnsPTranscript transcript2 = *(EnsOTranscript *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listTranscriptCompareStartDescending"))
        ajDebug("listTranscriptCompareStartDescending\n"
                "  transcript1 %p\n"
                "  transcript2 %p\n",
                transcript1,
                transcript2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (transcript1 && (!transcript2))
        return -1;

    if ((!transcript1) && (!transcript2))
        return 0;

    if ((!transcript1) && transcript2)
        return +1;

    return ensFeatureCompareStartDescending(transcript1->Feature,
                                            transcript2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Transcript Functions for manipulating AJAX List objects of
** Ensembl Transcript objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListTranscriptSortEndAscending ************************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** Ensembl Feature end member in ascending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListTranscriptSortEndAscending(AjPList transcripts)
{
    if (!transcripts)
        return ajFalse;

    ajListSortTwoThree(transcripts,
                       &listTranscriptCompareEndAscending,
                       &listTranscriptCompareStartAscending,
                       &listTranscriptCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListTranscriptSortEndDescending ***********************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** Ensembl Feature end member in descending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListTranscriptSortEndDescending(AjPList transcripts)
{
    if (!transcripts)
        return ajFalse;

    ajListSortTwoThree(transcripts,
                       &listTranscriptCompareEndDescending,
                       &listTranscriptCompareStartDescending,
                       &listTranscriptCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListTranscriptSortIdentifierAscending *****************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** identifier member in ascending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListTranscriptSortIdentifierAscending(AjPList transcripts)
{
    if (!transcripts)
        return ajFalse;

    ajListSort(transcripts, &listTranscriptCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListTranscriptSortStartAscending **********************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** Ensembl Feature start member in ascending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListTranscriptSortStartAscending(AjPList transcripts)
{
    if (!transcripts)
        return ajFalse;

    ajListSortTwoThree(transcripts,
                       &listTranscriptCompareStartAscending,
                       &listTranscriptCompareEndAscending,
                       &listTranscriptCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListTranscriptSortStartDescending *********************************
**
** Sort an AJAX List of Ensembl Transcript objects by their
** Ensembl Feature start member in descending order.
**
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListTranscriptSortStartDescending(AjPList transcripts)
{
    if (!transcripts)
        return ajFalse;

    ajListSortTwoThree(transcripts,
                       &listTranscriptCompareStartDescending,
                       &listTranscriptCompareEndDescending,
                       &listTranscriptCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [AjPSeq] AJAX Sequence ****************************************
**
** @nam2rule Sequence Functions for manipulating AJAX Sequence objects
**
******************************************************************************/




/* @section add ***************************************************************
**
** Functions for manipulating AJAX Sequence objects.
**
** @fdata [AjPSeq]
**
** @nam3rule Add Add to an AJAX Sequence
** @nam4rule Feature Add an AJAX Feature
** @nam5rule Transcript Convert an Ensembl Transcript into an AJAX Feature
**
** @argrule * seq [AjPSeq] AJAX Sequence
** @argrule Transcript transcript [EnsPTranscript] Ensembl Transcript
** @argrule Transcript Pfeature [AjPFeature*] AJAX Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceAddFeatureTranscript **************************************
**
** Convert an Ensembl Transcript into an AJAX Feature and add it to the
** AJAX Feature Table of an AJAX Sequence. Also convert and add all
** Ensembl Exon objects associated with the Ensembl Transcript.
**
** @param [u] seq [AjPSeq] AJAX Sequence
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [wP] Pfeature [AjPFeature*] AJAX Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensSequenceAddFeatureTranscript(AjPSeq seq,
                                       EnsPTranscript transcript,
                                       AjPFeature *Pfeature)
{
    ajint rank = 0;

    AjPFeature feature = NULL;

    AjIList iter = NULL;
    const AjPList exons = NULL;

    AjPStr label = NULL;
    AjPStr type  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExon exon = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPTranscript newtranscript = NULL;

    if (!seq)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!Pfeature)
        return ajFalse;

    *Pfeature = NULL;

    /*
    ** NOTE: An AJAX Sequence accessor function returning the
    ** AJAX Feature Table in modifiable form is missing.
    */

    if (!seq->Fttable)
        return ajFalse;

    /*
    ** Get the Ensembl Slice covering the AJAX Sequence object and transfer
    ** the Ensembl Exon object onto it.
    */

    dba = ensTranscriptadaptorGetDatabaseadaptor(transcript->Adaptor);

    sla = ensRegistryGetSliceadaptor(dba);

    ensSliceadaptorFetchByName(sla, ajSeqGetNameS(seq), &slice);

    if (!slice)
    {
        ajDebug("ensSeqAddFeatureTranscript could not fetch an "
                "Ensembl Slice for AJAX Sequence name '%S'.\n",
                ajSeqGetNameS(seq));

        return ajFalse;
    }

    newtranscript = ensTranscriptTransfer(transcript, slice);

    if (!newtranscript)
    {
        ajDebug("ensSeqAddFeatureTranscript could not transfer "
                "Ensembl Transcript %p onto "
                "Ensembl Slice %p.\n", transcript, slice);

        ensTranscriptTrace(transcript, 1);
        ensSliceTrace(slice, 1);

        ensSliceDel(&slice);

        return ajFalse;
    }

    /* Convert the Ensembl Transcript into a parent AJAX Feature. */

    /* FIXME: This needs to take biotypes into account. */
    type = ajStrNewC("mRNA");

    *Pfeature = ajFeatNewNucFlags(
        seq->Fttable,
        ensAnalysisGetName(ensFeatureGetAnalysis(newtranscript->Feature)),
        type,
        ensFeatureGetStart(newtranscript->Feature),
        ensFeatureGetEnd(newtranscript->Feature),
        0.0F, /* Score */
        ensFeatureCalculateStrand(newtranscript->Feature),
        0, /* Frame */
        0, /* Exon number */
        0, /* Start 2 */
        0, /* End 2 */
        (AjPStr) NULL, /* Remote Identifier */
        (AjPStr) NULL, /* Label */
        AJFEATFLAG_MULTIPLE);

    ensTranscriptFetchDisplayidentifier(newtranscript, &label);

    ajFeatTagAddCS(*Pfeature,
                   "standard_name",
                   label);

    dbe = ensTranscriptGetDisplayreference(newtranscript);

    if (dbe)
        ajFeatTagAddCS(*Pfeature,
                       "locus_tag",
                       ensDatabaseentryGetDisplayidentifier(dbe));

    /* Add sub-AJAX Feature objects on the basis of Ensembl Exon objects. */

    exons = ensTranscriptLoadExons(newtranscript);

    iter = ajListIterNewread(exons);

    while (!ajListIterDone(iter))
    {
        exon = (EnsPExon) ajListIterGet(iter);

        rank++;

        feature = ajFeatNewNucFlagsSub(
            seq->Fttable,
            *Pfeature,
            ensAnalysisGetName(ensFeatureGetAnalysis(newtranscript->Feature)),
            type,
            ensFeatureGetStart(ensExonGetFeature(exon)),
            ensFeatureGetEnd(ensExonGetFeature(exon)),
            0.0F,
            ensFeatureCalculateStrand(ensExonGetFeature(exon)),
            ensExonCalculateFrame(exon), /* Frame */
            rank, /* Exon */
            0, /* Start 2 */
            0, /* End 2 */
            (AjPStr) NULL, /* Remote identifier */
            (AjPStr) NULL, /* Label */
            0);

        ensSequenceAddFeatureExon(seq, exon, rank, &feature);
    }

    ajListIterDel(&iter);

    ajStrDel(&label);
    ajStrDel(&type);

    ensTranscriptDel(&newtranscript);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @datasection [EnsPTranscriptadaptor] Ensembl Transcript Adaptor ************
**
** @nam2rule Transcriptadaptor Functions for manipulating
** Ensembl Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor
** @cc CVS Revision: 1.149
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic transcriptadaptorFetchAllbyStatement ***************************
**
** Fetch all Ensembl Transcript objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool transcriptadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList transcripts)
{
    ajuint identifier = 0U;
    ajuint analysisid = 0U;
    ajuint erid       = 0U;
    ajuint geneid     = 0U;
    ajuint edbid      = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    ajuint version = 0U;

    AjBool current = AJFALSE;

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

    EnsETranscriptStatus estatus =
        ensETranscriptStatusNULL;

    EnsEExternalreferenceInfotype erit = ensEExternalreferenceInfotypeNULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPFeature feature = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca  = NULL;

    if (ajDebugTest("transcriptadaptorFetchAllbyStatement"))
        ajDebug("transcriptadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  transcripts %p\n",
                ba,
                statement,
                am,
                slice,
                transcripts);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);
    tca  = ensRegistryGetTranscriptadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0U;
        srid        = 0U;
        srstart     = 0U;
        srend       = 0U;
        srstrand    = 0;
        analysisid  = 0U;
        erid        = 0U;
        description = ajStrNew();
        biotype     = ajStrNew();
        status      = ajStrNew();
        current     = ajFalse;
        geneid      = 0U;
        stableid    = ajStrNew();
        version     = 0U;
        cdate       = ajStrNew();
        mdate       = ajStrNew();
        edbid       = 0U;
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

            continue;
        }

        if (erid)
        {
            ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

            erit = ensExternalreferenceInfotypeFromStr(erinfotype);

            if (!erit)
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

        /* Set the Transcript status. */

        estatus = ensTranscriptStatusFromStr(status);

        if (!estatus)
            ajFatal("transcriptadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'transcript.status' field.\n", status);

        /* Finally, create a new Ensembl Transcript. */

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

        ajListPushAppend(transcripts, (void *) transcript);

        ensFeatureDel(&feature);

        ensDatabaseentryDel(&dbe);

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
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
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
** @valrule * [EnsPTranscriptadaptor] Ensembl Transcript Adaptor or NULL
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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranscriptadaptor ensTranscriptadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        transcriptadaptorKTablenames,
        transcriptadaptorKColumnnames,
        transcriptadaptorKLeftjoins,
        (const char *) NULL,
        (const char *) NULL,
        &transcriptadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensTranscriptNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensTranscriptDel,
        (size_t (*)(const void *)) &ensTranscriptCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensTranscriptGetFeature,
        "Transcript");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Transcript Adaptor object.
**
** @fdata [EnsPTranscriptadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Transcript Adaptor
**
** @argrule * Ptca [EnsPTranscriptadaptor*] Ensembl Transcript Adaptor address
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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensTranscriptadaptorDel(EnsPTranscriptadaptor *Ptca)
{
    ensFeatureadaptorDel(Ptca);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Transcript Adaptor object.
**
** @fdata [EnsPTranscriptadaptor]
**
** @nam3rule Get Return Ensembl Transcript Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
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




/* @func ensTranscriptadaptorGetBaseadaptor ***********************************
**
** Get the Ensembl Base Adaptor member of an Ensembl Transcript Adaptor.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensTranscriptadaptorGetBaseadaptor(
    EnsPTranscriptadaptor tca)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensTranscriptadaptorGetFeatureadaptor(tca));
}




/* @func ensTranscriptadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Transcript Adaptor.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensTranscriptadaptorGetDatabaseadaptor(
    EnsPTranscriptadaptor tca)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensTranscriptadaptorGetFeatureadaptor(tca));
}




/* @func ensTranscriptadaptorGetFeatureadaptor ********************************
**
** Get the Ensembl Feature Adaptor member of an Ensembl Transcript Adaptor.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensTranscriptadaptorGetFeatureadaptor(
    EnsPTranscriptadaptor tca)
{
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
** @nam5rule Biotype
** Fetch all by a biological type
** @nam5rule Exonidentifier
** Fetch all by an Ensembl Exon identifier
** @nam5rule Exonstableidentifier
** Fetch all by an Ensembl Exon stable identifier
** @nam5rule Externaldatabasename
** Fetch all by an Ensembl External Database name
** @nam5rule Externalname
** Fetch all by an Ensembl Database Entry name
** @nam5rule Gene Fetch all by an Ensembl Gene
** @nam5rule Identifiers Fetch all by an AJAX Table
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable identifier
** @nam4rule By Fetch one Ensembl Transcript object matching a criterion
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by a stable identifier
** @nam5rule Translationidentifier
** Fetch by an Ensembl Translation identifier
** @nam5rule Translationstableidentifier
** Fetch by an Ensembl Translation stable identifier
**
** @argrule * tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule All transcripts [AjPList] AJAX List of Ensembl Transcript objects
** @argrule AllbyBiotype biotype [const AjPStr]
** Biological type
** @argrule AllbyBiotype transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyExonidentifier identifier [ajuint]
** Ensembl Exon identifier
** @argrule AllbyExonidentifier transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyExonstableidentifier stableid [const AjPStr]
** Ensembl Exon stable identifier
** @argrule AllbyExonstableidentifier transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyExternaldatabasename dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyExternaldatabasename transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyExternalname name [const AjPStr]
** Ensembl Database Entry name
** @argrule AllbyExternalname dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyExternalname override [AjBool]
** Override optimisation of '_' SQL any
** @argrule AllbyExternalname transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyGene gene [EnsPGene] Ensembl Gene
** @argrule AllbyGene transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyIdentifiers transcripts [AjPTable]
** AJAX Table of Ensembl Transcript objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice loadexons [AjBool] Load Ensembl Exon objects
** @argrule AllbySlice transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule AllbyStableidentifier transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule ByTranslationidentifier identifier [ajuint]
** Ensembl Translation identifier
** @argrule ByTranslationstableidentifier stableid [const AjPStr]
** Ensembl Translation stable identifier
** @argrule By Ptranscript [EnsPTranscript*]
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
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAll(
    EnsPTranscriptadaptor tca,
    AjPList transcripts)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!tca)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    constraint = ajStrNewC(
        "transcript.biotype != 'LRG_gene' "
        "AND "
        "transcript.is_current = 1");

    result = ensBaseadaptorFetchAllbyConstraint(
        ensTranscriptadaptorGetBaseadaptor(tca),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        transcripts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranscriptadaptorFetchAllbyBiotype ********************************
**
** Fetch all Ensembl Transcript objects via a biotype.
**
** The caller is responsible for deleting the Ensembl Transcript objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_biotype
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] biotype [const AjPStr] Biotype
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyBiotype(
    EnsPTranscriptadaptor tca,
    const AjPStr biotype,
    AjPList transcripts)
{
    char *txtbiotype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!tca)
        return ajFalse;

    if (!biotype)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    ba = ensTranscriptadaptorGetBaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtbiotype, biotype);

    constraint = ajFmtStr(
        "transcript.is_current = 1 "
        "AND "
        "transcript.biotype = '%s'",
        txtbiotype);

    ajCharDel(&txtbiotype);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        transcripts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranscriptadaptorFetchAllbyExonidentifier *************************
**
** Fetch an Ensembl Transcript via an Ensembl Exon identifier.
** The caller is responsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_exon_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] identifier [ajuint] Ensembl Exon identifier
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyExonidentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    AjPList transcripts)
{
    ajuint trid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscript transcript = NULL;

    if (!tca)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    ba  = ensTranscriptadaptorGetBaseadaptor(tca);
    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

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

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);

        ensBaseadaptorFetchByIdentifier(ba, trid, (void **) &transcript);

        if (transcript)
            ajListPushAppend(transcripts, (void *) transcript);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchAllbyExonstableidentifier *******************
**
** Fetch all Ensembl Transcript objects via an Ensembl Exon stable identifier.
** The caller is responsible for deleting the Ensembl Transcript objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_exon_stable_id
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] stableid [const AjPStr] Ensembl Exon stable identifier
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyExonstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    AjPList transcripts)
{
    char *txtstableid = NULL;

    ajuint trid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscript transcript = NULL;

    if (!tca)
        return ajFalse;

    if (!stableid && !ajStrGetLen(stableid))
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    ba  = ensTranscriptadaptorGetBaseadaptor(tca);
    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id "
        "FROM "
        "exon, "
        "exon_transcript, "
        "transcript "
        "WHERE "
        "exon.stable_id = '%s' "
        "AND "
        "exon.exon_id = exon_transcript.exon_id "
        "AND "
        "exon_transcript.transcript_id = transcript.transcript_id "
        "AND "
        "transcript.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0U;
        transcript = NULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);

        ensBaseadaptorFetchByIdentifier(ba, trid, (void **) &transcript);

        if (transcript)
            ajListPushAppend(transcripts, (void *) transcript);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensTranscriptadaptorFetchAllbyExternaldatabasename *******************
**
** Fetch all Ensembl Transcript objects via an Ensembl External Database name.
**
** The caller is responsible for deleting the Ensembl Transcript objects
** before deleting the AJAX List.
**
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] dbname [const AjPStr] Ensembl External Database name
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.6.0
** @@
** NOTE: The Ensembl Core API has no implementation of
** Bio::EnsEMBL::DBSQL::DBEntryAdaptor::list_transcript_ids_by_external_dbid
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyExternaldatabasename(
    EnsPTranscriptadaptor tca,
    const AjPStr dbname,
    AjPList transcripts)
{
    AjBool result = AJFALSE;

    AjPTable table = NULL;

    if (!tca)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    table = ajTableuintNew(0U);

    ajTableSetDestroyvalue(table, (void (*)(void **)) &ensTranscriptDel);

    result = ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternaldatabasename(
        ensRegistryGetDatabaseentryadaptor(
            ensTranscriptadaptorGetDatabaseadaptor(tca)),
        dbname,
        table);

    result = ensTranscriptadaptorFetchAllbyIdentifiers(tca, table);

    ensTableuintToList(table, transcripts);

    ajTableFree(&table);

    return result;
}




/* @func ensTranscriptadaptorFetchAllbyExternalname ***************************
**
** Fetch all Ensembl Transcript objects via an Ensembl Database Entry name and
** Ensembl External Database name.
**
** The caller is responsible for deleting the Ensembl Transcript objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::fetch_all_by_external_name
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [r] name [const AjPStr] Ensembl Database Entry name
** @param [rN] dbname [const AjPStr] Ensembl External Database name
** @param [r] override [AjBool] Override optimisation of '_' SQL any
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyExternalname(
    EnsPTranscriptadaptor tca,
    const AjPStr name,
    const AjPStr dbname,
    AjBool override,
    AjPList transcripts)
{
    AjBool result = AJFALSE;

    AjPTable table = NULL;

    if (!tca)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    table = ajTableuintNew(0U);

    ajTableSetDestroyvalue(table, (void (*)(void **)) &ensTranscriptDel);

    result = ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternalname(
        ensRegistryGetDatabaseentryadaptor(
            ensTranscriptadaptorGetDatabaseadaptor(tca)),
        name,
        dbname,
        override,
        table);

    result = ensTranscriptadaptorFetchAllbyIdentifiers(tca, table);

    ensTableuintToList(table, transcripts);

    ajTableFree(&table);

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
**
** @release 6.4.0
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

    EnsPSlice gslice     = NULL;
    EnsPSlice tslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPTranscript transcript    = NULL;
    EnsPTranscript newtranscript = NULL;

    if (!tca)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!transcripts)
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

    if (!dba)
    {
        ajWarn("ensTranscriptadaptorFetchAllbyGene cannot fetch "
               "Ensembl Transcript objects without an "
               "Ensembl Database Adaptor defined in the "
               "Ensembl Transcript Adaptor.\n");

        return ajFalse;
    }

    gfeature = ensGeneGetFeature(gene);

    if (!gfeature)
    {
        ajWarn("ensTranscriptadaptorFetchAllbyGene cannot fetch "
               "Ensembl Transcript objects without an "
               "Ensembl Feature defined in the Ensembl Gene.\n");

        return ajFalse;
    }

    gslice = ensFeatureGetSlice(gfeature);

    if (!gslice)
    {
        ajWarn("ensTranscriptadaptorFetchAllbyGene cannot fetch "
               "Ensembl Transcript objects without an Ensembl Slice defined "
               "in the Ensembl Feature of the Ensembl Gene.\n");

        return ajFalse;
    }

    if (!ensSliceIsCircular(gslice, &circular))
        return ajFalse;

    if ((ensFeatureGetStart(gfeature) < 1) ||
        (ensFeatureGetEnd(gfeature) > (ajint) ensSliceCalculateLength(gslice)))
    {
        if (circular == ajTrue)
            tslice = ensSliceNewRef(gslice);
        else
        {
            sla = ensRegistryGetSliceadaptor(dba);

            ensSliceadaptorFetchByFeature(sla, gfeature, 0, &tslice);
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

    while (ajListPop(list, (void **) &transcript))
    {
        if (ensGeneGetCanonicaltranscriptidentifier(gene) ==
            ensTranscriptGetIdentifier(transcript))
            ensTranscriptSetCanonical(transcript, ajTrue);

        if (ensSliceMatch(gslice, tslice))
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




/* @func ensTranscriptadaptorFetchAllbyIdentifiers ****************************
**
** Fetch all Ensembl Transcript objects by an AJAX Table of
** AJAX unsigned integer key data and assign them as value data.
**
** The caller is responsible for deleting the AJAX unsigned integer key and
** Ensembl Transcript value data before deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] transcripts [AjPTable]
** AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Transcript value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyIdentifiers(
    EnsPTranscriptadaptor tca,
    AjPTable transcripts)
{
    return ensBaseadaptorFetchAllbyIdentifiers(
        ensTranscriptadaptorGetBaseadaptor(tca),
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensTranscriptGetIdentifier,
        transcripts);
}




/* @funcstatic transcriptadaptorListTranscriptExonRankValdel ******************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Exon Transcript Rank objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void transcriptadaptorListTranscriptExonRankValdel(void **Pvalue)
{
    TranscriptPExonRank trex = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &trex))
        transcriptExonRankDel(&trex);

    ajListFree((AjPList *) Pvalue);

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
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [rN] constraint [const AjPStr] SQL constraint
** @param [r] loadexons [AjBool] Load Ensembl Exon objects
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
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
    void **keyarray = NULL;

    ajint start = INT_MAX;
    ajint end   = INT_MIN;
    ajint rank  = 0;

    register ajuint i = 0U;

    ajuint exid = 0U;
    ajuint trid = 0U;

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

    EnsPSlice newslice   = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPTranscript transcript = NULL;

    EnsPTranslationadaptor tla = NULL;

    TranscriptPExonRank trex = NULL;

    if (!tca)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    if (constraint && ajStrGetLen(constraint))
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

    if ((!loadexons) || (ajListGetLength(transcripts) == 0))
        return ajTrue;

    /*
    ** Preload all Ensembl Exon objects now, instead of lazy loading later,
    ** which is faster than one SQL query per Transcript.
    ** First check if the Ensembl Exon objects are already preloaded.
    ** TODO: This should test all Ensembl Exon objects.
    */

    ajListPeekFirst(transcripts, (void **) &transcript);

    if (transcript->Exons)
        return ajTrue;

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    ea  = ensRegistryGetExonadaptor(dba);
    sla = ensRegistryGetSliceadaptor(dba);
    tla = ensRegistryGetTranslationadaptor(dba);

    /*
    ** Get the extent of the region spanned by Ensembl Transcript objects,
    ** prepare a comma-separared list of Ensembl Transcript identifier
    ** instances and put Ensembl Transcript objects into an AJAX Table
    ** indexed by their identifier.
    */

    csv = ajStrNew();

    trtable = ajTableuintNew(0U);

    ajTableSetDestroyvalue(trtable, (void (*)(void **)) &ensTranscriptDel);

    iter = ajListIterNew(transcripts);

    while (!ajListIterDone(iter))
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

        if (ajTableMatchV(trtable, (const void *) &trid))
            ajDebug("ensTranscriptadaptorFetchAllbySlice got duplicate "
                    "Ensembl Transcript with identifier %u.\n", trid);
        else
        {
            AJNEW0(Pidentifier);

            *Pidentifier = trid;

            ajTablePut(trtable,
                       (void *) Pidentifier,
                       (void *) ensTranscriptNewRef(transcript));
        }
    }

    ajListIterDel(&iter);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    if ((start >= ensSliceGetStart(slice)) && (end <= ensSliceGetEnd(slice)))
        newslice = ensSliceNewRef(slice);
    else
        ensSliceadaptorFetchBySlice(sla,
                                    slice,
                                    start,
                                    end,
                                    ensSliceGetStrand(slice),
                                    &newslice);

    /*
    ** Associate Ensembl Exon identifiers with Ensembl Transcript objects and
    ** Ensembl Exon ranks.
    */

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

    extable = ajTableuintNew(0U);

    ajTableSetDestroyvalue(
        extable,
        (void (*)(void **)) &transcriptadaptorListTranscriptExonRankValdel);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0U;
        exid = 0U;
        rank = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
        ajSqlcolumnToUint(sqlr, &exid);
        ajSqlcolumnToInt(sqlr, &rank);

        list = (AjPList) ajTableFetchmodV(extable, (const void *) &exid);

        if (!list)
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

    /* Get all Ensembl Exon identifiers as comma-separated values. */

    ajTableToarrayKeys(extable, &keyarray);

    for (i = 0U; keyarray[i]; i++)
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint *) keyarray[i]));

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

    while (ajListPop(exons, (void **) &exon))
    {
        newexon = ensExonTransfer(exon, newslice);

        if (!newexon)
            ajFatal("ensTranscriptAdaptorFetchAllbySlice could not transfer "
                    "Exon onto new Slice.\n");

        exid = ensExonGetIdentifier(newexon);

        list = (AjPList) ajTableFetchmodV(extable, (const void *) &exid);

        iter = ajListIterNew(list);

        while (!ajListIterDone(iter))
        {
            trex = (TranscriptPExonRank) ajListIterGet(iter);

            transcript = (EnsPTranscript) ajTableFetchmodV(
                trtable,
                (const void *) &trex->Transcriptidentifier);

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

    ajTableDel(&trtable);

    /*
    ** Clear and detete the AJAX Table of
    ** AJAX unsigned integer key and
    ** AJAX List value data.
    ** Also delete the transcriptExonRank objects from the AJAX List objects.
    */

    ajTableDel(&extable);

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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchAllbyStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    AjPList transcripts)
{
    char *txtstableid = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!tca)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    ba = ensTranscriptadaptorGetBaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("transcript.stable_id = '%s'",
                          txtstableid);

    ajCharDel(&txtstableid);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByDisplaylabel(
    EnsPTranscriptadaptor tca,
    const AjPStr label,
    EnsPTranscript *Ptranscript)
{
    char *txtlabel = NULL;

    AjBool result = AJFALSE;

    AjPList transcripts = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPTranscript transcript = NULL;

    if (!tca)
        return ajFalse;

    if (!label)
        return ajFalse;

    if (!Ptranscript)
        return ajFalse;

    *Ptranscript = NULL;

    ba = ensTranscriptadaptorGetBaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtlabel, label);

    constraint = ajFmtStr(
        "transcript.is_current = 1 "
        "AND "
        "xref.display_label = '%s'",
        txtlabel);

    ajCharDel(&txtlabel);

    transcripts = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        transcripts);

    if (ajListGetLength(transcripts) > 1)
        ajDebug("ensTranscriptadaptorFetchByDisplaylabel got more than one "
                "Transcript for display label '%S'.\n", label);

    ajListPop(transcripts, (void **) Ptranscript);

    while (ajListPop(transcripts, (void **) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&transcripts);

    ajStrDel(&constraint);

    return result;
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByIdentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript *Ptranscript)
{
    return ensBaseadaptorFetchByIdentifier(
        ensTranscriptadaptorGetBaseadaptor(tca),
        identifier,
        (void **) Ptranscript);
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    ajuint version,
    EnsPTranscript *Ptranscript)
{
    char *txtstableid = NULL;

    AjBool result = AJFALSE;

    AjPList transcripts = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPTranscript transcript = NULL;

    if (!tca)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Ptranscript)
        return ajFalse;

    *Ptranscript = NULL;

    ba = ensTranscriptadaptorGetBaseadaptor(tca);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if (version)
        constraint = ajFmtStr(
            "transcript.stable_id = '%s' "
            "AND "
            "transcript.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "transcript.stable_id = '%s' "
            "AND "
            "transcript.is_current = 1",
            txtstableid);

    ajCharDel(&txtstableid);

    transcripts = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        transcripts);

    if (ajListGetLength(transcripts) > 1)
        ajDebug("ensTranscriptadaptorFetchByStableidentifier "
                "got more than one "
                "Ensembl Transcript for stable identifier '%S' and "
                "version %u.\n",
                stableid, version);

    ajListPop(transcripts, (void **) Ptranscript);

    while (ajListPop(transcripts, (void **) &transcript))
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationidentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript *Ptranscript)
{
    ajuint trid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!tca)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Ptranscript)
        return ajFalse;

    *Ptranscript = NULL;

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

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

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!trid)
    {
        ajDebug("ensTranscriptadaptorFetchByTranslationidentifier could not "
                "get Transcript identifier for Translation identifier %u.\n",
                identifier);

        return ajFalse;
    }

    return ensBaseadaptorFetchByIdentifier(
        ensTranscriptadaptorGetBaseadaptor(tca),
        trid,
        (void **) Ptranscript);
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorFetchByTranslationstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript *Ptranscript)
{
    char *txtstableid = NULL;

    ajuint trid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!tca)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Ptranscript)
        return ajFalse;

    *Ptranscript = NULL;

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    ensDatabaseadaptorEscapeC(dba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id "
        "FROM "
        "translation, "
        "transcript "
        "WHERE "
        "translation.stable_id = '%s' "
        "AND "
        "translation.transcript_id = transcript.transcript_id "
        "AND "
        "transcript.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!trid)
    {
        ajDebug("ensTranscriptadaptorFetchByTranslationstableidentifier "
                "could not get an Ensembl Transcript identifier for "
                "Ensembl Translation stable identifier '%S'.\n",
                stableid);

        return ajFalse;
    }

    return ensBaseadaptorFetchByIdentifier(
        ensTranscriptadaptorGetBaseadaptor(tca),
        trid,
        (void **) Ptranscript);
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
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
** @nam5rule Stableidentifiers Retrieve all stable identifier objects
** @nam4rule Canonical Retrieve the canonical flag
**
** @argrule * tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Transcript identifier) objects
** @argrule AllStableidentifiers stableids [AjPList]
** AJAX List of AJAX String (Ensembl Transcript stable identifier) objects
** @argrule Canonical transcript [EnsPTranscript] Ensembl Transcript
** @argrule Canonical Pcanonical [AjBool*] Canonical flag
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensTranscriptadaptorRetrieveAllIdentifiers ***************************
**
** Retrieve all SQL database-internal identifier objects of
** Ensembl Transcript objects.
**
** The caller is responsible for deleting the AJAX unsigned integer objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::list_dbIDs
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] identifiers [AjPList]
** AJAX List of AJAX unsigned integer (Ensembl Transcript identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorRetrieveAllIdentifiers(
    EnsPTranscriptadaptor tca,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!tca)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("transcript");

    result = ensBaseadaptorRetrieveAllIdentifiers(
        ensTranscriptadaptorGetBaseadaptor(tca),
        table,
        (AjPStr) NULL,
        identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensTranscriptadaptorRetrieveAllStableidentifiers *********************
**
** Retrieve all stable identifier objects of Ensembl Transcript objects.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::list_stable_ids
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] stableids [AjPList]
** AJAX List of AJAX String (Ensembl Transcript stable identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorRetrieveAllStableidentifiers(
    EnsPTranscriptadaptor tca,
    AjPList stableids)
{
    AjBool result = AJFALSE;

    AjPStr primary = NULL;
    AjPStr table   = NULL;

    if (!tca)
        return ajFalse;

    if (!stableids)
        return ajFalse;

    table   = ajStrNewC("transcript");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(
        ensTranscriptadaptorGetBaseadaptor(tca),
        table,
        primary,
        stableids);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}




/* @func ensTranscriptadaptorRetrieveCanonical ********************************
**
** Retrieve the canonical attribute of an Ensembl Transcript.
**
** The caller is responsible for deleting the AJAX Boolean object.
**
** @cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor::is_Transcript_canonical
** @param [u] tca [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] Pcanonical [AjBool*] Canonical attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensTranscriptadaptorRetrieveCanonical(
    EnsPTranscriptadaptor tca,
    EnsPTranscript transcript,
    AjBool *Pcanonical)
{
    ajuint count = 0U;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!tca)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!Pcanonical)
        return ajFalse;

    dba = ensTranscriptadaptorGetDatabaseadaptor(tca);

    statement = ajFmtStr(
        "SELECT "
        "count(*) "
        "FROM "
        "gene "
        "WHERE "
        "gene.canonical_transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        count = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &count);

        if (count)
        {
            *Pcanonical = ajTrue;

            ensTranscriptSetCanonical(transcript, ajTrue);
        }
        else
        {
            *Pcanonical = ajFalse;

            ensTranscriptSetCanonical(transcript, ajFalse);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}

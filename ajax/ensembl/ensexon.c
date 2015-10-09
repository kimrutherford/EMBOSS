/* @source ensexon ************************************************************
**
** Ensembl Exon functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.65 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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
#include "ensexon.h"
#include "enstable.h"
#include "enstranscript.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */

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
** @attr TranscriptStart [ajuint] Exon start on Transcript
** @attr TranscriptEnd [ajuint] Exon end on Transcript
** @attr TranscriptCodingStart [ajuint] Coding region start on Transcript
** @attr TranscriptCodingEnd [ajuint] Coding region end on Transcript
** @attr SliceCodingStart [ajint] Coding region start on Slice
** @attr SliceCodingEnd [ajint] Coding region end on Slice
** @@
******************************************************************************/

typedef struct ExonSCoordinates
{
    ajuint TranscriptStart;
    ajuint TranscriptEnd;
    ajuint TranscriptCodingStart;
    ajuint TranscriptCodingEnd;
    ajint SliceCodingStart;
    ajint SliceCodingEnd;
} ExonOCoordinates;

#define ExonPCoordinates ExonOCoordinates*




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic exonadaptorKTables ********************************************
**
** Array of Ensembl Exon Adaptor SQL table names
**
******************************************************************************/

static const char *const exonadaptorKTables[] =
{
    "exon",
    (const char *) NULL
};




/* @conststatic exonadaptorKColumns *******************************************
**
** Array of Ensembl Exon Adaptor SQL column names
**
******************************************************************************/

static const char *const exonadaptorKColumns[] =
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
    "exon.stable_id",
    "exon.version",
    "exon.created_date",
    "exon.modified_date",
    (const char *) NULL
};




/* @conststatic exontranscriptadaptorKTables **********************************
**
** Array of Ensembl Exon-Transcript Adaptor SQL table names
**
******************************************************************************/

static const char *const exontranscriptadaptorKTables[] =
{
    "exon",
    "exon_transcript",
    (const char *) NULL
};




/* @conststatic exontranscriptadaptorKDefaultcondition ************************
**
** Ensembl Exon-Transcript Adaptor default SQL condition
**
******************************************************************************/

static const char *exontranscriptadaptorKDefaultcondition =
    "exon.exon_id = exon_transcript.exon_id";




/* @conststatic exontranscriptadaptorKFinalcondition **************************
**
** Ensembl Exon-Transcript Adaptor final SQL condition
**
******************************************************************************/

static const char *exontranscriptadaptorKFinalcondition =
    "ORDER BY "
    "exon_transcript.transcript_id, "
    "exon_transcript.rank";




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static ExonPCoordinates exonCoordinatesNewIni(EnsPExon exon,
                                              EnsPTranscript transcript,
                                              EnsPTranslation translation);

static void exonCoordinatesDel(ExonPCoordinates *Pec);

static AjBool exonMergeCoordinates(AjPList mrs);

static int listExonCompareEndAscending(
    const void *item1,
    const void *item2);

static int listExonCompareEndDescending(
    const void *item1,
    const void *item2);

static int listExonCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listExonCompareStartAscending(
    const void *item1,
    const void *item2);

static int listExonCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool exonadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList exons);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensexon *******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @funcstatic exonCoordinatesNewIni ******************************************
**
** Exon Coordinates constructor with initial values.
**
** The function calculates Ensembl Exon coordinates based on an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Exon::coding_region_start
** @cc Bio::EnsEMBL::Exon::coding_region_end
** @cc Bio::EnsEMBL::Exon::cdna_coding_start
** @cc Bio::EnsEMBL::Exon::cdna_coding_end
** @cc Bio::EnsEMBL::Exon::cdna_start
** @cc Bio::EnsEMBL::Exon::cdna_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] translation [EnsPTranslation] Ensembl Translation
**
** @return [ExonPCoordinates] Exon Coordinates or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

static ExonPCoordinates exonCoordinatesNewIni(EnsPExon exon,
                                              EnsPTranscript transcript,
                                              EnsPTranslation translation)
{
    ajint  scstart = 0;  /* Ensembl Slice coding start */
    ajint  scend   = 0;  /* Ensembl Slice coding end   */

    ajuint tcstart = 0U; /* Ensembl Transcript coding start */
    ajuint tcend   = 0U; /* Ensembl Transcript coding end   */

    AjBool debug = AJFALSE;

    AjPList mrs = NULL;

    EnsPFeature feature = NULL;
    EnsPMapperresult mr = NULL;
    ExonPCoordinates ec = NULL;

    debug = ajDebugTest("exonCoordinatesNewIni");

    if (debug)
        ajDebug("exonCoordinatesNewIni\n"
                "  exon %p\n"
                "  transcript %p\n"
                "  translation %p\n",
                exon,
                transcript,
                translation);

    if (!exon)
        return NULL;

    if (!transcript)
        return NULL;

    AJNEW0(ec);

    /*
    ** Calculate the start and end of this Ensembl Exon in
    ** Ensembl Transcript coordinates.
    */

    mrs = ajListNew();

    ensTranscriptMapperSliceTotranscript(transcript,
                                         ensFeatureGetStart(exon->Feature),
                                         ensFeatureGetEnd(exon->Feature),
                                         ensFeatureGetStrand(exon->Feature),
                                         mrs);

    if (!ajListGetLength(mrs))
    {
        ajDebug("exonCoordinatesNewIni cannot map Ensembl Exon %p from "
                "its Slice onto Transcript %p.\n", exon);

        ensExonTrace(exon, 1);

        ensTranscriptTrace(transcript, 1);
    }

    ajListPeekFirst(mrs, (void **) &mr);

    switch (ensMapperresultGetType(mr))
    {
        case ensEMapperresultTypeCoordinate:

            ec->TranscriptStart = ensMapperresultGetCoordinateStart(mr);
            ec->TranscriptEnd   = ensMapperresultGetCoordinateEnd(mr);

            if (debug)
                ajDebug("exonCoordinatesNewIni Exon '%S' "
                        "Transcript '%S:%d:%d'.\n",
                        ensExonGetStableidentifier(exon),
                        ensTranscriptGetStableidentifier(transcript),
                        ec->TranscriptStart,
                        ec->TranscriptEnd);

            break;

        case ensEMapperresultTypeGap:

            if (debug)
            {
                ajDebug("exonCoordinatesNewIni maps the first part of "
                        "Ensembl Exon %p into a gap %d:%d for "
                        "Transcript %p.\n",
                        exon,
                        ensMapperresultGetGapStart(mr),
                        ensMapperresultGetGapEnd(mr),
                        transcript);

                ensExonTrace(exon, 1);

                ensTranscriptTrace(transcript, 1);
            }

            break;

        default:

            ajWarn("exonCoordinatesNewIni got an Ensembl Mapper Result of "
                   "unexpected type %d.", ensMapperresultGetType(mr));
    }

    while (ajListPop(mrs, (void **) &mr))
        ensMapperresultDel(&mr);

    ajListFree(&mrs);

    /*
    ** Calculate the start and end of the coding region of this Ensembl Exon
    ** in Ensembl Transcript coordinates.
    */

    if (!translation)
        return ec;

    tcstart = ensTranscriptCalculateTranscriptCodingStart(transcript,
                                                          translation);

    tcend   = ensTranscriptCalculateTranscriptCodingEnd(transcript,
                                                        translation);

    if (debug)
        ajDebug("exonCoordinatesNewIni Transcript Coding %d:%d\n",
                tcstart, tcend);

    if (!tcstart)
    {
        /* This is a non-coding Transcript. */

        ec->TranscriptCodingStart = 0;
        ec->TranscriptCodingEnd   = 0;
    }
    else
    {
        if (tcstart < ec->TranscriptStart)
        {
            /* The coding region starts up-stream of this Exon ... */

            if (tcend < ec->TranscriptStart)
            {
                /* ... and also ends up-stream of this Exon. */

                ec->TranscriptCodingStart = 0;
                ec->TranscriptCodingEnd   = 0;
            }
            else
            {
                /* ... and does not end up-stream of this Exon, but ... */

                ec->TranscriptCodingStart = ec->TranscriptStart;

                if (tcend < ec->TranscriptEnd)
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

            if (tcstart <= ec->TranscriptEnd)
            {
                /* The coding region starts within this Exon ... */

                ec->TranscriptCodingStart = tcstart;

                if (tcend < ec->TranscriptEnd)
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
                ec->TranscriptCodingEnd   = 0;
            }
        }
    }

    /*
    ** Calculate the start and end of the coding region of this Ensembl Exon
    ** in Ensembl Slice coordinates.
    */

    scstart = ensTranscriptCalculateSliceCodingStart(transcript, translation);
    scend   = ensTranscriptCalculateSliceCodingEnd(transcript, translation);

    if (!scstart)
    {
        /* This is a non-coding Transcript. */

        ec->SliceCodingStart = 0;
        ec->SliceCodingEnd   = 0;
    }
    else
    {
        feature = ensExonGetFeature(exon);

        if (scstart < ensFeatureGetStart(feature))
        {
            /* The coding region starts up-stream of this Exon ... */

            if (scend < ensFeatureGetStart(feature))
            {
                /* ... and also ends up-stream of this Exon. */

                ec->SliceCodingStart = 0;
                ec->SliceCodingEnd   = 0;
            }
            else
            {
                /* ... and does not end up-stream of this Exon, but ... */

                ec->SliceCodingStart = ensFeatureGetStart(feature);

                if (scend < ensFeatureGetEnd(feature))
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

            if (scstart <= ensFeatureGetEnd(feature))
            {
                /* The coding region starts within this Exon ... */

                ec->SliceCodingStart = scstart;

                if (scend < ensFeatureGetEnd(feature))
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
                ec->SliceCodingEnd   = 0;
            }
        }
    }

    return ec;
}




/* @funcstatic exonCoordinatesDel *********************************************
**
** Default destructor for an Exon Coordinates object.
**
** @param [d] Pec [ExonPCoordinates*] Exon Coordinates address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void exonCoordinatesDel(ExonPCoordinates *Pec)
{
    ajMemFree((void **) Pec);

	return;
}




/* @datasection [EnsPExon] Ensembl Exon ***************************************
**
** @nam2rule Exon Functions for manipulating Ensembl Exon objects
**
** @cc Bio::EnsEMBL::Exon
** @cc CVS Revision: 1.182
** @cc CVS Tag: branch-ensembl-66
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
**
** @nam4rule Cpy Constructor with existing object
** @nam3rule New Constructor
** @nam4rule Ini Constructor with initial values
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Cpy exon [const EnsPExon] Ensembl Exon
** @argrule Ini ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini sphase [ajint] Start phase of Translation
** @argrule Ini ephase [ajint] End phase of Translation
** @argrule Ini current [AjBool] Current attribute
** @argrule Ini constitutive [AjBool] Constitutive attribute
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Version
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ref exon [EnsPExon] Ensembl Exon
**
** @valrule * [EnsPExon] Ensembl Exon or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensExonNewCpy ********************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [EnsPExon] Ensembl Exon or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExon ensExonNewCpy(const EnsPExon exon)
{
    AjIList iter = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPExon pthis = NULL;

    if (!exon)
        return NULL;

    AJNEW0(pthis);

    pthis->Use          = 1U;
    pthis->Identifier   = exon->Identifier;
    pthis->Adaptor      = exon->Adaptor;
    pthis->Feature      = ensFeatureNewRef(exon->Feature);
    pthis->PhaseStart   = exon->PhaseStart;
    pthis->PhaseEnd     = exon->PhaseEnd;
    pthis->Current      = exon->Current;
    pthis->Constitutive = exon->Constitutive;

    if (exon->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(exon->Stableidentifier);

    pthis->Version = exon->Version;

    if (exon->DateCreation)
        pthis->DateCreation = ajStrNewRef(exon->DateCreation);

    if (exon->DateModification)
        pthis->DateModification = ajStrNewRef(exon->DateModification);

    if (exon->SequenceCache)
        pthis->SequenceCache = ajStrNewRef(exon->SequenceCache);

    /*
    ** NOTE: Copy the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    if (exon->Supportingfeatures &&
        ajListGetLength(exon->Supportingfeatures))
    {
        pthis->Supportingfeatures = ajListNew();

        iter = ajListIterNew(exon->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ajListPushAppend(pthis->Supportingfeatures,
                             (void *) ensBasealignfeatureNewRef(baf));
        }

        ajListIterDel(&iter);
    }

    return pthis;
}




/* @func ensExonNewIni ********************************************************
**
** Ensembl Exon constructor with initial values.
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExon ensExonNewIni(EnsPExonadaptor ea,
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

    if (ajDebugTest("ensExonNewIni"))
    {
        ajDebug("ensExonNewIni\n"
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

    if (!feature)
        return NULL;

    if ((sphase < -1) || (sphase > 2))
    {
        ajDebug("ensExonNewIni start phase must be 0, 1, 2 for coding regions "
                "or -1 for non-coding regions, not %d.\n", sphase);

        return NULL;
    }

    if ((ephase < -1) || (ephase > 2))
    {
        ajDebug("ensExonNewIni end phase must be 0, 1, 2 for coding regions "
                "or -1 for non-coding regions, not %d.\n", ephase);

        return NULL;
    }

    AJNEW0(exon);

    exon->Use          = 1U;
    exon->Identifier   = identifier;
    exon->Adaptor      = ea;
    exon->Feature      = ensFeatureNewRef(feature);
    exon->PhaseStart   = sphase;
    exon->PhaseEnd     = ephase;
    exon->Current      = current;
    exon->Constitutive = constitutive;

    if (stableid)
        exon->Stableidentifier = ajStrNewRef(stableid);

    exon->Version = version;

    if (cdate)
        exon->DateCreation = ajStrNewRef(cdate);

    if (mdate)
        exon->DateModification = ajStrNewRef(mdate);

    exon->SequenceCache      = NULL;
    exon->Supportingfeatures = NULL;

    return exon;
}




/* @func ensExonNewRef ********************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [EnsPExon] Ensembl Exon or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExon ensExonNewRef(EnsPExon exon)
{
    if (!exon)
        return NULL;

    exon->Use++;

    return exon;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Exon object.
**
** @fdata [EnsPExon]
**
** @nam3rule Del Destroy (free) an Ensembl Exon
**
** @argrule * Pexon [EnsPExon*] Ensembl Exon address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExonDel ***********************************************************
**
** Default destructor for an Ensembl Exon.
**
** @param [d] Pexon [EnsPExon*] Ensembl Exon address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensExonDel(EnsPExon *Pexon)
{
    EnsPBasealignfeature baf = NULL;

    EnsPExon pthis = NULL;

    if (!Pexon)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensExonDel"))
    {
        ajDebug("ensExonDel\n"
                "  *Pexon %p\n",
                *Pexon);

        ensExonTrace(*Pexon, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pexon)
        return;

    pthis = *Pexon;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pexon = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->Stableidentifier);
    ajStrDel(&pthis->DateCreation);
    ajStrDel(&pthis->DateModification);
    ajStrDel(&pthis->SequenceCache);

    /*
    ** Clear and delete the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    while (ajListPop(pthis->Supportingfeatures, (void **) &baf))
        ensBasealignfeatureDel(&baf);

    ajListFree(&pthis->Supportingfeatures);

    AJFREE(pthis);

    *Pexon = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Exon object.
**
** @fdata [EnsPExon]
**
** @nam3rule Get Return Exon attribute(s)
** @nam4rule Adaptor Return the Ensembl Exon Adaptor
** @nam4rule Constitutive Return the constitutive flag
** @nam4rule Current Return the current flag
** @nam4rule Date Return a date
** @nam5rule Creation Return the date of creation
** @nam5rule Modification Return the date of modification
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Phase Return a phase
** @nam5rule End Return the phase at end
** @nam5rule Start Return the phase at start
** @nam4rule Stableidentifier Return the stable identifier
** @nam4rule Version Return the version
**
** @argrule * exon [const EnsPExon] Exon
**
** @valrule Adaptor [EnsPExonadaptor] Ensembl Exon Adaptor or NULL
** @valrule Constitutive [AjBool] Constitutive attribute or ajFalse
** @valrule Current [AjBool] Current attribute or ajFalse
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Modification date or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule PhaseEnd [ajint] Phase at end or 0
** @valrule PhaseStart [ajint] Phase at start or 0
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Version [ajuint] Version or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensExonGetAdaptor ****************************************************
**
** Get the Ensembl Exon Adaptor member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [EnsPExonadaptor] Ensembl Exon Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExonadaptor ensExonGetAdaptor(const EnsPExon exon)
{
    return (exon) ? exon->Adaptor : NULL;
}




/* @func ensExonGetConstitutive ***********************************************
**
** Get the constitutive flag member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_constitutive
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue if this Exon is constitutive
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonGetConstitutive(const EnsPExon exon)
{
    return (exon) ? exon->Constitutive : ajFalse;
}




/* @func ensExonGetCurrent ****************************************************
**
** Get the current flag member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_current
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue if this Exon reflects the current state of
**                  annotation
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonGetCurrent(const EnsPExon exon)
{
    return (exon) ? exon->Current : ajFalse;
}




/* @func ensExonGetDateCreation ***********************************************
**
** Get the date of creation member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::created_date
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExonGetDateCreation(const EnsPExon exon)
{
    return (exon) ? exon->DateCreation : NULL;
}




/* @func ensExonGetDateModification *******************************************
**
** Get the date of modification member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::modified_date
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjPStr] Modification date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExonGetDateModification(const EnsPExon exon)
{
    return (exon) ? exon->DateModification : NULL;
}




/* @func ensExonGetFeature ****************************************************
**
** Get the Ensembl Feature member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::slice
** @cc Bio::EnsEMBL::Exon::start
** @cc Bio::EnsEMBL::Exon::end
** @cc Bio::EnsEMBL::Exon::strand
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensExonGetFeature(const EnsPExon exon)
{
    return (exon) ? exon->Feature : NULL;
}




/* @func ensExonGetIdentifier *************************************************
**
** Get the SQL database-internal identifier member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensExonGetIdentifier(const EnsPExon exon)
{
    return (exon) ? exon->Identifier : 0U;
}




/* @func ensExonGetPhaseEnd ***************************************************
**
** Get the phase at end member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::end_phase
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajint] End phase or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensExonGetPhaseEnd(const EnsPExon exon)
{
    return (exon) ? exon->PhaseEnd : 0;
}




/* @func ensExonGetPhaseStart *************************************************
**
** Get the phase at start member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::phase
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajint] Start phase or 0
**
** @release 6.4.0
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

ajint ensExonGetPhaseStart(const EnsPExon exon)
{
    return (exon) ? exon->PhaseStart : 0;
}




/* @func ensExonGetStableidentifier *******************************************
**
** Get the stable identifier member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::stable_id
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [AjPStr] Stable identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExonGetStableidentifier(const EnsPExon exon)
{
    return (exon) ? exon->Stableidentifier : NULL;
}




/* @func ensExonGetVersion ****************************************************
**
** Get the version member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::version
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajuint] Version or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensExonGetVersion(const EnsPExon exon)
{
    return (exon) ? exon->Version : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Exon object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPExon]
**
** @nam3rule Load Return Ensembl Exon attribute(s) loaded on demand
** @nam4rule Supportingfeatures Return all supporting
**                              Ensembl Base Align Feature objects
**
** @argrule * exon [EnsPExon] Exon
**
** @valrule Supportingfeatures [const AjPList] AJAX List of
** Ensembl Base Align Feature objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensExonLoadSupportingfeatures ****************************************
**
** Load all Ensembl Supporting Feature objects for this Ensembl Transcript.
**
** This is not a simple accessor function, it will fetch
** Ensembl Base Align Feature objects of type 'dna' and 'protein' from the
** Ensembl Core database in case the AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Exon::get_all_supporting_features
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [const AjPList] AJAX List of Ensembl Base Align Feature objects
** or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensExonLoadSupportingfeatures(EnsPExon exon)
{
    EnsPDatabaseadaptor dba = NULL;

    if (!exon)
        return NULL;

    if (exon->Supportingfeatures)
        return exon->Supportingfeatures;

    if (!exon->Adaptor)
    {
        ajDebug("ensExonLoadSupportingfeatures cannot fetch "
                "Ensembl Base Align Features object for an Ensembl Exon "
                "without an Ensembl Exon Adaptor.\n");

        return NULL;
    }

    dba = ensExonadaptorGetDatabaseadaptor(exon->Adaptor);

    exon->Supportingfeatures = ajListNew();

    ensSupportingfeatureadaptorFetchAllbyExon(dba,
                                              exon,
                                              exon->Supportingfeatures);

    return exon->Supportingfeatures;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Exon object.
**
** @fdata [EnsPExon]
**
** @nam3rule Set Set one member of an Ensembl Exon
** @nam4rule Adaptor Set the Ensembl Exon Adaptor
** @nam4rule Constitutive Set the constitutive flag
** @nam4rule Current Set the current flag
** @nam4rule Date Set a date
** @nam5rule Creation Set the date of creation
** @nam5rule Modification Set the date of modification
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Phase Set a phase
** @nam5rule PhaseEnd Set the phase at end
** @nam5rule PhaseStart Set the phase at start
** @nam4rule Stableidentifier Set the stable identifier
** @nam4rule Version Set the version
**
** @argrule * exon [EnsPExon] Ensembl Exon object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Adaptor ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @argrule Constitutive constitutive [AjBool] Constitutive attribute
** @argrule Current current [AjBool] Current attribute
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule DateModification mdate [AjPStr] Modification date
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule PhaseEnd ephase [ajint] Phase at end
** @argrule PhaseStart sphase [ajint] Phase at start
** @argrule Stableidentifier stableid [AjPStr] Stable identifier
** @argrule Version version [ajuint] Version
**
** @fcategory modify
******************************************************************************/




/* @func ensExonSetAdaptor ****************************************************
**
** Set the Ensembl Exon Adaptor member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonSetAdaptor(EnsPExon exon, EnsPExonadaptor ea)
{
    if (!exon)
        return ajFalse;

    exon->Adaptor = ea;

    return ajTrue;
}




/* @func ensExonSetConstitutive ***********************************************
**
** Set the constitutive member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_constitutive
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] constitutive [AjBool] Constitutive attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonSetConstitutive(EnsPExon exon, AjBool constitutive)
{
    if (!exon)
        return ajFalse;

    exon->Constitutive = constitutive;

    return ajTrue;
}




/* @func ensExonSetCurrent ****************************************************
**
** Set the current member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::is_current
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] current [AjBool] Current attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonSetCurrent(EnsPExon exon, AjBool current)
{
    if (!exon)
        return ajFalse;

    exon->Current = current;

    return ajTrue;
}




/* @func ensExonSetDateCreation ***********************************************
**
** Set the date of creation member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::created_date
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonSetDateCreation(EnsPExon exon, AjPStr cdate)
{
    if (!exon)
        return ajFalse;

    ajStrDel(&exon->DateCreation);

    exon->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensExonSetDateModification *******************************************
**
** Set the date of modification member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::modified_date
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonSetDateModification(EnsPExon exon, AjPStr mdate)
{
    if (!exon)
        return ajFalse;

    ajStrDel(&exon->DateModification);

    exon->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensExonSetFeature ****************************************************
**
** Set the Ensembl Feature member of an Ensembl Exon.
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonSetFeature(EnsPExon exon, EnsPFeature feature)
{
    AjIList iter = NULL;

    EnsPBasealignfeature baf  = NULL;
    EnsPBasealignfeature nbaf = NULL;

    EnsPSlice eslice = NULL;

    if (ajDebugTest("ensExonSetFeature"))
    {
        ajDebug("ensExonSetFeature\n"
                "  exon %p\n"
                "  feature %p\n",
                exon,
                feature);

        ensExonTrace(exon, 1);

        ensFeatureTrace(feature, 1);
    }

    if (!exon)
        return ajFalse;

    if (!feature)
        return ajFalse;

    /* Replace the current Feature. */

    if (exon->Feature)
        ensFeatureDel(&exon->Feature);

    exon->Feature = ensFeatureNewRef(feature);

    /* Clear the sequence cache. */

    ajStrDel(&exon->SequenceCache);

    /*
    ** Transfer Ensembl Base Align Feature objects onto the new
    ** Feature Slice.
    */

    if (!exon->Supportingfeatures)
        return ajTrue;

    eslice = ensFeatureGetSlice(exon->Feature);

    iter = ajListIterNew(exon->Supportingfeatures);

    while (!ajListIterDone(iter))
    {
        baf = (EnsPBasealignfeature) ajListIterGet(iter);

        ajListIterRemove(iter);

        nbaf = ensBasealignfeatureTransfer(baf, eslice);

        if (!nbaf)
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




/* @func ensExonSetIdentifier *************************************************
**
** Set the SQL database-internal identifier member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonSetIdentifier(EnsPExon exon, ajuint identifier)
{
    if (!exon)
        return ajFalse;

    exon->Identifier = identifier;

    return ajTrue;
}




/* @func ensExonSetPhaseEnd ***************************************************
**
** Set the phase at end member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::end_phase
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] ephase [ajint] End phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonSetPhaseEnd(EnsPExon exon, ajint ephase)
{
    if (!exon)
        return ajFalse;

    exon->PhaseEnd = ephase;

    return ajTrue;
}




/* @func ensExonSetPhaseStart *************************************************
**
** Set the phase at start member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::phase
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] sphase [ajint] Start phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonSetPhaseStart(EnsPExon exon, ajint sphase)
{
    if (!exon)
        return ajFalse;

    exon->PhaseStart = sphase;

    return ajTrue;
}




/* @func ensExonSetStableidentifier *******************************************
**
** Set the stable identifier member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::stable_id
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonSetStableidentifier(EnsPExon exon, AjPStr stableid)
{
    if (!exon)
        return ajFalse;

    ajStrDel(&exon->Stableidentifier);

    exon->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensExonSetVersion ****************************************************
**
** Set the version member of an Ensembl Exon.
**
** @cc Bio::EnsEMBL::Exon::version
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonSetVersion(EnsPExon exon, ajuint version)
{
    if (!exon)
        return ajFalse;

    exon->Version = version;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Exon object.
**
** @fdata [EnsPExon]
**
** @nam3rule Trace Report Ensembl Exon members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonTrace(const EnsPExon exon, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr tmpstr = NULL;

    EnsPBasealignfeature baf = NULL;

    if (!exon)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensExonTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  PhaseStart %d\n"
            "%S  PhaseEnd %d\n"
            "%S  Current '%B'\n"
            "%S  Constitutive '%B'\n"
            "%S  Stableidentifier '%S'\n"
            "%S  Version %u\n"
            "%S  DateCreation '%S'\n"
            "%S  DateModification '%S'\n"
            "%S  SequenceCache %p\n"
            "%S  Supportingfeatures %p\n",
            indent, exon,
            indent, exon->Use,
            indent, exon->Identifier,
            indent, exon->Adaptor,
            indent, exon->Feature,
            indent, exon->PhaseStart,
            indent, exon->PhaseEnd,
            indent, exon->Current,
            indent, exon->Constitutive,
            indent, exon->Stableidentifier,
            indent, exon->Version,
            indent, exon->DateCreation,
            indent, exon->DateModification,
            indent, exon->SequenceCache,
            indent, exon->Supportingfeatures);

    ensFeatureTrace(exon->Feature, level + 1);

    if (exon->SequenceCache)
    {
        /*
        ** For sequences longer than 40 characters use a
        ** '20 ... 20' notation.
        */

        if (ajStrGetLen(exon->SequenceCache) > 40)
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

    /* Trace the AJAX List of supporting Ensembl Base Align Feature objects. */

    if (exon->Supportingfeatures)
    {
        ajDebug("%S    AJAX List %p of Ensembl Base Align Feature objects:\n",
                indent, exon->Supportingfeatures);

        iter = ajListIterNewread(exon->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            ensBasealignfeatureTrace(baf, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Exon object.
**
** @fdata [EnsPExon]
**
** @nam3rule Calculate Calculate Ensembl Exon values
** @nam4rule Frame Calculate the coding frame
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Slice      Calculate Ensembl Exon coordinates relative to an
**                      Ensembl Slice
** @nam5rule Coding Calculate Ensembl Exon coding coordinates
** @nam6rule End    Calculate the Ensembl Exon coding start coordinate
** @nam6rule Start  Calculate the Ensembl Exon coding end coordinate
** @nam4rule Transcript Calculate Ensembl Exon coordinates relative to an
**                      Ensembl Transcript
** @nam5rule Coding Calculate Ensembl Exon coding coordinates
** @nam6rule End    Calculate the Ensembl Exon coding start coordinate
** @nam6rule Start  Calculate the Ensembl Exon coding end coordinate
** @nam5rule End    Calculate the Ensembl Exon end coordinate
** @nam5rule Start  Calculate the Ensembl Exon start coordinate
**
** @argrule Frame exon [const EnsPExon] Ensembl Exon
** @argrule Memsize exon [const EnsPExon] Ensembl Exon
** @argrule SliceCoding exon [EnsPExon] Ensembl Exon
** @argrule SliceCoding transcript [EnsPTranscript] Ensembl Transcript
** @argrule SliceCoding translation [EnsPTranslation] Ensembl Translation
** @argrule TranscriptCoding exon [EnsPExon] Ensembl Exon
** @argrule TranscriptCoding transcript [EnsPTranscript] Ensembl Transcript
** @argrule TranscriptCoding translation [EnsPTranslation] Ensembl Translation
** @argrule TranscriptEnd exon [EnsPExon] Ensembl Exon
** @argrule TranscriptEnd transcript [EnsPTranscript] Ensembl Transcript
** @argrule TranscriptStart exon [EnsPExon] Ensembl Exon
** @argrule TranscriptStart transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule Frame [ajint] Coding frame or 0
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule SliceCodingEnd [ajint] End coordinate or 0
** @valrule SliceCodingStart [ajint] Start coordinate or 0
** @valrule TranscriptCodingEnd [ajuint] End coordinate or 0U
** @valrule TranscriptCodingStart [ajuint] Start coordinate or 0U
** @valrule TranscriptEnd [ajuint] End coordinate or 0U
** @valrule TranscriptStart [ajuint] Start coordinate or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensExonCalculateFrame ************************************************
**
** Calculate the coding frame of an Ensembl Exon on an Ensembl Slice.
** The coding frame can be 0, 1, 2 or -1 for Exon objects that are not coding.
**
** @cc Bio::EnsEMBL::Exon:frame
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [ajint] Coding frame or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensExonCalculateFrame(const EnsPExon exon)
{
    if (!exon)
        return 0;

    if (exon->PhaseStart == -1)
        return -1;

    if (exon->PhaseStart == 0)
        return (exon->Feature->Start % 3);

    if (exon->PhaseStart == 1)
        return ((exon->Feature->Start + 2) % 3);

    if (exon->PhaseStart == 2)
        return ((exon->Feature->Start + 1) % 3);

    ajDebug("ensExonCalculateFrame invalid start phase %d in exon %u.\n",
            exon->PhaseStart, exon->Identifier);

    return 0;
}




/* @func ensExonCalculateMemsize **********************************************
**
** Calculate the memory size in bytes of an Ensembl Exon.
**
** @param [r] exon [const EnsPExon] Ensembl Exon
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensExonCalculateMemsize(const EnsPExon exon)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPBasealignfeature baf = NULL;

    if (!exon)
        return 0;

    size += sizeof (EnsOExon);

    size += ensFeatureCalculateMemsize(exon->Feature);

    if (exon->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->Stableidentifier);
    }

    if (exon->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->DateCreation);
    }

    if (exon->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->DateModification);
    }

    if (exon->SequenceCache)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(exon->SequenceCache);
    }

    /*
    ** Summarise the AJAX List of supporting
    ** Ensembl Base Align Feature objects.
    */

    if (exon->Supportingfeatures)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(exon->Supportingfeatures);

        while (!ajListIterDone(iter))
        {
            baf = (EnsPBasealignfeature) ajListIterGet(iter);

            size += ensBasealignfeatureCalculateMemsize(baf);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @func ensExonCalculateSliceCodingEnd ***************************************
**
** Calculate the end coordinate of the coding region of an Ensembl Exon
** relaive to an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Exon::coding_region_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajint] Coding end coordinate or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensExonCalculateSliceCodingEnd(EnsPExon exon,
                                     EnsPTranscript transcript,
                                     EnsPTranslation translation)
{
    ajint scend = 0;

    ExonPCoordinates ec = NULL;

    if (!exon)
        return 0;

    if (!transcript)
        return 0;

    ec = exonCoordinatesNewIni(exon, transcript, translation);

    if (ec)
        scend = ec->SliceCodingEnd;

    exonCoordinatesDel(&ec);

    return scend;
}




/* @func ensExonCalculateSliceCodingStart *************************************
**
** Calculate the start coordinate of the coding region of an Ensembl Exon
** relaive to an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Exon::coding_region_start
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajint] Coding start coordinate or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensExonCalculateSliceCodingStart(EnsPExon exon,
                                       EnsPTranscript transcript,
                                       EnsPTranslation translation)
{
    ajint scstart = 0;

    ExonPCoordinates ec = NULL;

    if (!exon)
        return 0;

    if (!transcript)
        return 0;

    ec = exonCoordinatesNewIni(exon, transcript, translation);

    if (ec)
        scstart = ec->SliceCodingStart;

    exonCoordinatesDel(&ec);

    return scstart;
}




/* @func ensExonCalculateTranscriptCodingEnd **********************************
**
** Calculate the end coordinate of the coding region of an Ensembl Exon
** relative to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Exon::cdna_coding_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Coding end coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensExonCalculateTranscriptCodingEnd(EnsPExon exon,
                                           EnsPTranscript transcript,
                                           EnsPTranslation translation)
{
    ajuint tcend = 0U;

    ExonPCoordinates ec = NULL;

    if (!exon)
        return 0U;

    if (!transcript)
        return 0U;

    ec = exonCoordinatesNewIni(exon, transcript, translation);

    if (ec)
        tcend = ec->TranscriptCodingEnd;

    exonCoordinatesDel(&ec);

    return tcend;
}




/* @func ensExonCalculateTranscriptCodingStart ********************************
**
** Calculate the start coordinate of the coding region of an Ensembl Exon
** relative to an Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Exon::cdna_coding_start
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Coding start coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensExonCalculateTranscriptCodingStart(EnsPExon exon,
                                             EnsPTranscript transcript,
                                             EnsPTranslation translation)
{
    ajuint tcstart = 0U;

    ExonPCoordinates ec = NULL;

    if (!exon)
        return 0U;

    if (!transcript)
        return 0U;

    ec = exonCoordinatesNewIni(exon, transcript, translation);

    if (ec)
        tcstart = ec->TranscriptCodingStart;

    exonCoordinatesDel(&ec);

    return tcstart;
}




/* @func ensExonCalculateTranscriptEnd ****************************************
**
** Calculate the end coordinate of an Ensembl Exon relative to an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Exon::cdna_end
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] End coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensExonCalculateTranscriptEnd(EnsPExon exon, EnsPTranscript transcript)
{
    ajuint tend = 0U;

    ExonPCoordinates ec = NULL;

    if (!exon)
        return 0U;

    if (!transcript)
        return 0U;

    ec = exonCoordinatesNewIni(exon, transcript, (EnsPTranslation) NULL);

    if (ec)
        tend = ec->TranscriptEnd;

    exonCoordinatesDel(&ec);

    return tend;
}




/* @func ensExonCalculateTranscriptStart **************************************
**
** Calculate the start coordinate of an Ensembl Exon relative to an
** Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Exon::cdna_start
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [ajuint] Start coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensExonCalculateTranscriptStart(EnsPExon exon,
                                       EnsPTranscript transcript)
{
    ajuint tstart = 0U;

    ExonPCoordinates ec = NULL;

    if (!exon)
        return 0U;

    if (!transcript)
        return 0U;

    ec = exonCoordinatesNewIni(exon, transcript, (EnsPTranslation) NULL);

    if (ec)
        tstart = ec->TranscriptStart;

    exonCoordinatesDel(&ec);

    return tstart;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Exon objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPExon]
**
** @nam3rule Transfer Transfer an Ensembl Exon
** @nam3rule Transform Transform an Ensembl Exon
**
** @argrule * exon [EnsPExon] Ensembl Exon
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
**
** @valrule * [EnsPExon] Ensembl Exon or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExonTransfer ******************************************************
**
** Transfer an Ensembl Exon onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::transfer
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPExon] Ensembl Exon or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExon ensExonTransfer(EnsPExon exon, EnsPSlice slice)
{
    EnsPExon newexon = NULL;
    EnsPFeature newfeature = NULL;

    if (!exon)
        return NULL;

    if (!slice)
        return NULL;

    newfeature = ensFeatureTransfer(exon->Feature, slice);

    if (!newfeature)
        return NULL;

    newexon = ensExonNewCpy(exon);

    ensExonSetFeature(newexon, newfeature);

    /*
    ** NOTE: The Exon-internal supporting features have already been
    ** transfered to the new Slice and the sequence cache has already been
    ** cleared by the ensExonSetFeature function.
    */

    ensFeatureDel(&newfeature);

    return newexon;
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
** @return [EnsPExon] Ensembl Exon or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExon ensExonTransform(EnsPExon exon,
                          const AjPStr csname,
                          const AjPStr csversion)
{
    EnsPExon nexon       = NULL;
    EnsPFeature nfeature = NULL;

    if (!exon)
        return NULL;

    if (!csname)
        return NULL;

    if (!csversion)
        return NULL;

    nfeature = ensFeatureTransform(exon->Feature,
                                   csname,
                                   csversion,
                                   (EnsPSlice) NULL);

    if (!nfeature)
        return NULL;

    nexon = ensExonNewCpy(exon);

    ensExonSetFeature(nexon, nfeature);

    /*
    ** NOTE: The Exon-internal supporting features have already been
    ** transfered to the new Slice and the sequence cache has already been
    ** cleared by the ensExonSetFeature function.
    */

    return nexon;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Exon object.
**
** @fdata [EnsPExon]
**
** @nam3rule Fetch Fetch Ensembl Exon values
** @nam4rule Displayidentifier Fetch the display identifier
** @nam4rule Sequence Fetch the sequence
** @nam5rule Slice Fetch the sequence of the Ensembl Slice
** covered by an Ensembl Exon
** @nam5rule Translation Fetch the sequence of the Ensembl Translation
** covered by an Ensembl Exon
** @nam6rule Seq Fetch the sequence as AJAX Sequence object
** @nam6rule Str Fetch the sequence as AJAX String object
**
** @argrule Displayidentifier exon [const EnsPExon] Ensembl Exon
** @argrule Displayidentifier Pidentifier [AjPStr*] Display identifier address
** @argrule Sequence exon [EnsPExon] Ensembl Exon
** @argrule Translation transcript [EnsPTranscript] Ensembl Transcript
** @argrule Translation translation [EnsPTranslation] Ensembl Translation
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence object address
** @argrule Str Psequence [AjPStr*] AJAX String object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensExonFetchDisplayidentifier ****************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonFetchDisplayidentifier(const EnsPExon exon, AjPStr *Pidentifier)
{
    if (!exon)
        return ajFalse;

    if (!Pidentifier)
        return ajFalse;

    if (exon->Stableidentifier && ajStrGetLen(exon->Stableidentifier))
    {
        if (*Pidentifier)
            ajStrAssignS(Pidentifier, exon->Stableidentifier);
        else
            *Pidentifier = ajStrNewS(exon->Stableidentifier);
    }
    else if (exon->Identifier)
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier, "%u", exon->Identifier);
        else
            *Pidentifier = ajFmtStr("%u", exon->Identifier);
    }
    else
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier, "%p", exon);
        else
            *Pidentifier = ajFmtStr("%p", exon);
    }

    return ajTrue;
}




/* @func ensExonFetchSequenceSliceSeq *****************************************
**
** Fetch the sequence of an Ensembl Slice covered by an Ensembl Exon as
** AJAX Sequence.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Exon:seq
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [wP] Psequence [AjPSeq*] AJAX Sequence object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonFetchSequenceSliceSeq(EnsPExon exon, AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!exon)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    ensExonFetchDisplayidentifier(exon, &name);
    ensExonFetchSequenceSliceStr(exon, &sequence);

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




/* @func ensExonFetchSequenceSliceStr *****************************************
**
** Fetch the sequence of an Ensembl Slice covered by an Ensembl Exon as
** AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [wP] Psequence [AjPStr*] AJAX String object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonFetchSequenceSliceStr(EnsPExon exon, AjPStr *Psequence)
{
    ajint mpoint = 0;

    AjBool circular = AJFALSE;

    AjPStr sequence1 = NULL;
    AjPStr sequence2 = NULL;

    EnsPFeature feature = NULL;

    EnsPSlice slice = NULL;

    if (!exon)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    feature = exon->Feature;

    if (!feature)
    {
        ajWarn("ensExonFetchSequenceSliceStr cannot get sequence without an "
               "Ensembl Feature attached to the Ensembl Exon '%u:%S'.\n",
               exon->Identifier, exon->Stableidentifier);

        return ajFalse;
    }

    slice = ensFeatureGetSlice(feature);

    if (!slice)
    {
        ajWarn("ensExonFetchSequenceSliceStr cannot get sequence without an "
               "Ensembl Slice attached to the Ensembl Feature in the "
               "Ensembl Exon '%u:%S'.\n",
               exon->Identifier, exon->Stableidentifier);

        return ajFalse;
    }

    /*
    ** It is sligtly more efficient, if undefined AJAX String
    ** objects are directly allocated by the following functions
    ** to their final size.
    */

    if ((exon->SequenceCache == NULL) ||
        (ajStrGetLen(exon->SequenceCache) == 0))
    {
        if (!ensSliceIsCircular(slice, &circular))
            return ajFalse;

        if (circular == ajTrue)
        {
            if (ensSliceGetStart(slice) > ensSliceGetEnd(slice))
            {
                /*
                ** Normally, Ensembl Exon objects overlapping the chromosome
                ** origin will have a negative Ensembl Feature start, but
                ** the Ensembl Slice will be from 1 to Ensembl Sequence Region
                ** length.
                ** In case the Exon is attached to a Sub-Slice try this ...
                */

                mpoint
                    = ensSliceGetSeqregionLength(slice)
                    - ensSliceGetStart(slice)
                    + 1;

                /*
                ** It is sligtly more efficient, if undefined AJAX String
                ** objects are directly allocated by the following functions
                ** to their final size.
                */

                ensSliceFetchSequenceSubStr(slice,
                                            ensFeatureGetStart(feature),
                                            mpoint,
                                            ensFeatureGetStrand(feature),
                                            &sequence1);

                ensSliceFetchSequenceSubStr(slice,
                                            mpoint + 1,
                                            ensFeatureGetEnd(feature),
                                            ensFeatureGetStrand(feature),
                                            &sequence2);

                if (ensFeatureGetStrand(feature) >= 0)
                {
                    ajStrAppendS(&exon->SequenceCache, sequence1);
                    ajStrAppendS(&exon->SequenceCache, sequence2);
                }
                else
                {
                    ajStrAppendS(&exon->SequenceCache, sequence2);
                    ajStrAppendS(&exon->SequenceCache, sequence1);
                }

                ajStrDel(&sequence1);
                ajStrDel(&sequence2);
            }
            else if ((ensFeatureGetStart(feature) < 0) ||
                     (ensFeatureGetStart(feature) > ensFeatureGetEnd(feature)))
            {
                /*
                ** Normally, Ensembl Exon objects overlapping the chromosome
                ** origin will be zero-based, and can have negative start
                ** coordinates. Going via a Sub-Slice converts into
                ** chromosome-based coordinates, i.e it will have start greater
                ** than end.
                */

                mpoint = ensSliceGetSeqregionLength(slice);

                /*
                ** It is sligtly more efficient, if undefined AJAX String
                ** objects are directly allocated by the following functions
                ** to their final size.
                */

                ensSliceFetchSequenceSubStr(slice,
                                            ensFeatureGetStart(feature),
                                            mpoint,
                                            ensFeatureGetStrand(feature),
                                            &sequence1);

                ensSliceFetchSequenceSubStr(slice,
                                            1,
                                            ensFeatureGetEnd(feature),
                                            ensFeatureGetStrand(feature),
                                            &sequence2);

                if (ensFeatureGetStrand(feature) >= 0)
                {
                    ajStrAppendS(&exon->SequenceCache, sequence1);
                    ajStrAppendS(&exon->SequenceCache, sequence2);
                }
                else
                {
                    ajStrAppendS(&exon->SequenceCache, sequence2);
                    ajStrAppendS(&exon->SequenceCache, sequence1);
                }

                ajStrDel(&sequence1);
                ajStrDel(&sequence2);
            }
            else
            {
                /* For Ensembl Exon objects not overlapping the origin. */

                ensSliceFetchSequenceSubStr(slice,
                                            ensFeatureGetStart(feature),
                                            ensFeatureGetEnd(feature),
                                            ensFeatureGetStrand(feature),
                                            &exon->SequenceCache);
            }
        }
        else
            ensSliceFetchSequenceSubStr(slice,
                                        ensFeatureGetStart(feature),
                                        ensFeatureGetEnd(feature),
                                        ensFeatureGetStrand(feature),
                                        &exon->SequenceCache);
    }

    if (*Psequence)
        ajStrAssignS(Psequence, exon->SequenceCache);
    else
        *Psequence = ajStrNewS(exon->SequenceCache);

    return ajTrue;
}




/* @func ensExonFetchSequenceTranslationSeq ***********************************
**
** Fetch the sequence of an Ensembl Translation covered by an Ensembl Exon as
** AJAX Sequence.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Exon:peptide
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPSeq*] AJAX Sequence object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonFetchSequenceTranslationSeq(EnsPExon exon,
                                          EnsPTranscript transcript,
                                          EnsPTranslation translation,
                                          AjPSeq *Psequence)
{
    AjPStr name = NULL;
    AjPStr sequence = NULL;

    if (!exon)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    ensExonFetchDisplayidentifier(exon, &name);
    ensExonFetchSequenceTranslationStr(exon,
                                       transcript,
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

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @funcstatic exonMergeCoordinates *******************************************
**
** Merge Ensembl Mapper Result objects if end and start coordinates overlap.
** If all adjacent Ensembl Mapper Result objects can be merged, this function
** will delete them from the AJAX List and push a new Ensembl Mapper Result
** spanning all onto the AJAX List. If a merge is not possible, the AJAX List
** of Ensembl Mapper Result objects remains unchanged.
**
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool exonMergeCoordinates(AjPList mrs)
{
    ajint lastend = 0;

    AjBool merged = AJTRUE;

    AjIList iter = NULL;

    EnsPMapperresult mr1 = NULL;
    EnsPMapperresult mr2 = NULL;

    if (!mrs)
        return ajFalse;

    iter = ajListIterNew(mrs);

    while (!ajListIterDone(iter))
    {
        mr1 = (EnsPMapperresult) ajListIterGet(iter);

        if (ensMapperresultGetType(mr1) != ensEMapperresultTypeCoordinate)
            continue;

        lastend = ensMapperresultGetCoordinateEnd(mr1);

        while (!ajListIterDone(iter))
        {
            mr2 = (EnsPMapperresult) ajListIterGet(iter);

            if (ensMapperresultGetType(mr2) != ensEMapperresultTypeCoordinate)
                continue;

            if ((lastend + 1) >= ensMapperresultGetCoordinateStart(mr2))
                lastend = ensMapperresultGetCoordinateEnd(mr2);
            else
            {
                merged = ajFalse;

                break;
            }
        }

        if (merged == ajFalse)
            break;
    }

    ajListIterDel(&iter);

    if (merged == ajTrue)
    {
        /*
        ** If all Ensembl Mapper Result objects could be merged successfuly,
        ** create a new copy of the first Ensembl Mapper Result and set the
        ** end coordinate to the last end. Clear the AJAX List of Ensembl
        ** Mapper Result objects and push the new one onto it.
        */

        mr2 = ensMapperresultNewCpy(mr1);

        ensMapperresultSetCoordinateEnd(mr2, lastend);

        while (ajListPop(mrs, (void **) &mr1))
            ensMapperresultDel(&mr1);

        ajListPushAppend(mrs, (void *) mr2);
    }

    return merged;
}




/* @func ensExonFetchSequenceTranslationStr ***********************************
**
** Fetch the sequence of an Ensembl Translation covered by an Ensembl Exon as
** AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Exon::peptide
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPStr*] AJAX String object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonFetchSequenceTranslationStr(EnsPExon exon,
                                          EnsPTranscript transcript,
                                          EnsPTranslation translation,
                                          AjPStr *Psequence)
{
    const char *Ptr = NULL;

    register ajint length = 0;

    ajint start = 0;
    ajint end   = 0;

    AjIList iter = NULL;

    AjPList mrs = NULL;

    AjPStr sequence = NULL;

    EnsPExon nexon = NULL;

    EnsPFeature tfeature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice tslice = NULL;

    if (!exon)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /* Convert Ensembl Exon coordinates to Ensembl Translation coordinates. */

    tfeature = ensTranscriptGetFeature(transcript);

    tslice = ensFeatureGetSlice(tfeature);

    nexon = ensExonTransfer(exon, tslice);

    if (!nexon)
        ajFatal("ensExonFetchSequenceTranslationStr could not transfer "
                "Ensembl Exon to Ensembl Transcript Slice.");

    mrs = ajListNew();

    ensTranscriptMapperSliceTotranslation(transcript,
                                          translation,
                                          ensFeatureGetStart(nexon->Feature),
                                          ensFeatureGetEnd(nexon->Feature),
                                          ensFeatureGetStrand(nexon->Feature),
                                          mrs);

    /*
    ** Filter out Ensembl Mapper Result objects of type
    ** ensEMapperresultTypeGap.
    */

    iter = ajListIterNew(mrs);

    while (!ajListIterDone(iter))
    {
        mr = (EnsPMapperresult) ajListIterGet(iter);

        if (ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
        {
            ajListIterRemove(iter);

            ensMapperresultDel(&mr);
        }
    }

    ajListIterDel(&iter);

    /* If this is UTR then the peptide will be empty string */

    if (ajListGetLength(mrs) > 1)
    {
        if (!exonMergeCoordinates(mrs))
            ajFatal("ensExonFetchSequenceTranslationStr got an Exon (%u), "
                    "which maps to multiple non-continuous locations in the "
                    "Ensembl Translation.",
                    exon->Identifier);
    }
    else if (ajListGetLength(mrs) == 1)
    {
        ajListPeekFirst(mrs, (void **) &mr);

        sequence = ajStrNew();

        ensTranscriptFetchSequenceTranslationStr(transcript,
                                                 translation,
                                                 &sequence);

        /*
        ** NOTE: Since ajStrGetLen returns size_t, which exceeds ajint,
        ** the sequence length needs to be determined here.
        */

        for (length = 0, Ptr = ajStrGetPtr(sequence);
             (Ptr && *Ptr);
             length++, Ptr++)
            if (length == INT_MAX)
                ajFatal("ensExonFetchSequenceTranslationStr exeeded INT_MAX.");

        end = (ensMapperresultGetCoordinateEnd(mr) > length)
            ? length : ensMapperresultGetCoordinateEnd(mr);

        start = (ensMapperresultGetCoordinateStart(mr) < end)
            ? ensMapperresultGetCoordinateStart(mr) : end;

        ajStrAssignSubS(Psequence, sequence, start, end);

        ajStrDel(&sequence);
    }

    ensExonDel(&nexon);

    return ajTrue;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Exon objects
**
** @fdata [EnsPExon]
**
** @nam3rule Match      Test Ensembl Exon objects for identity
** @nam3rule Overlap    Test Ensembl Exon objects for overlap
** @nam3rule Similarity Test Ensembl Exon objects for similarity
**
** @argrule * exon1 [const EnsPExon] Ensembl Exon
** @argrule * exon2 [const EnsPExon] Ensembl Exon
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensExonMatch *********************************************************
**
** Test Ensembl Exon objects for identity.
**
** @param [r] exon1 [const EnsPExon] Ensembl Exon
** @param [r] exon2 [const EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue if the Ensembl Exon objects are equal
**
** @release 6.4.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, individual members are compared.
******************************************************************************/

AjBool ensExonMatch(const EnsPExon exon1, const EnsPExon exon2)
{
    if (!exon1)
        return ajFalse;

    if (!exon2)
        return ajFalse;

    if (exon1 == exon2)
        return ajTrue;

    if (exon1->Identifier != exon2->Identifier)
        return ajFalse;

    if (exon1->Adaptor != exon2->Adaptor)
        return ajFalse;

    if (!ensFeatureMatch(exon1->Feature, exon2->Feature))
        return ajFalse;

    if (exon1->PhaseStart != exon2->PhaseStart)
        return ajFalse;

    if (exon1->PhaseEnd != exon2->PhaseEnd)
        return ajFalse;

    if (exon1->Current != exon2->Current)
        return ajFalse;

    if (exon1->Constitutive != exon2->Constitutive)
        return ajFalse;

    /* Stable identifiers are optional. */

    if ((exon1->Stableidentifier || exon2->Stableidentifier)
        && (!ajStrMatchS(exon1->Stableidentifier, exon2->Stableidentifier)))
        return ajFalse;

    if (exon1->Version != exon2->Version)
        return ajFalse;

    /* Creation dates are optional. */

    if ((exon1->DateCreation || exon2->DateCreation)
        && (!ajStrMatchS(exon1->DateCreation, exon2->DateCreation)))
        return ajFalse;

    /* Modification dates are optional. */

    if ((exon1->DateModification || exon2->DateModification)
        && (!ajStrMatchS(exon1->DateModification, exon2->DateModification)))
        return ajFalse;

    /*
    ** NOTE: The SequenceCache and Supportingfeatures members are currently
    ** not checked.
    */

    return ajTrue;
}




/* @func ensExonSimilarity ****************************************************
**
** Test Ensembl Exon objects for similarity.
**
** @cc Bio::EnsEMBL::Exon::equals
** @param [r] exon1 [const EnsPExon] Ensembl Exon
** @param [r] exon2 [const EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue if the Ensembl Exon objects are similar
**
** @release 6.4.0
** @@
** NOTE: This function is similar to Bio::EnsEMBL::Exon::equals, but not
** completely identical.
** The comparison is based on an initial pointer equality test and if that
** fails, individual members are compared.
******************************************************************************/

AjBool ensExonSimilarity(const EnsPExon exon1, const EnsPExon exon2)
{
    if (!exon1)
        return ajFalse;

    if (!exon2)
        return ajFalse;

    if (exon1 == exon2)
        return ajTrue;

    if (exon1->Identifier != exon2->Identifier)
        return ajFalse;

    if (exon1->Adaptor != exon2->Adaptor)
        return ajFalse;

    if (!ensFeatureSimilarity(exon1->Feature, exon2->Feature))
        return ajFalse;

    if (exon1->PhaseStart != exon2->PhaseStart)
        return ajFalse;

    if (exon1->PhaseEnd != exon2->PhaseEnd)
        return ajFalse;

    if (exon1->Current != exon2->Current)
        return ajFalse;

    if (exon1->Constitutive != exon2->Constitutive)
        return ajFalse;

    /* Stable identifiers are optional. */

    if ((exon1->Stableidentifier || exon2->Stableidentifier)
        && (!ajStrMatchS(exon1->Stableidentifier, exon2->Stableidentifier)))
        return ajFalse;

    if (exon1->Version != exon2->Version)
        return ajFalse;

    /* Creation dates are optional. */

    if ((exon1->DateCreation || exon2->DateCreation)
        && (!ajStrMatchS(exon1->DateCreation, exon2->DateCreation)))
        return ajFalse;

    /* Modification dates are optional. */

    if ((exon1->DateModification || exon2->DateModification)
        && (!ajStrMatchS(exon1->DateModification, exon2->DateModification)))
        return ajFalse;

    /*
    ** NOTE: The SequenceCache and Supportingfeatures members are currently
    ** not checked.
    */

    return ajTrue;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listExonCompareEndAscending ************************************
**
** AJAX List of Ensembl Exon objects comparison function to sort by
** Ensembl Feature end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Exon address 1
** @param [r] item2 [const void*] Ensembl Exon address 2
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

static int listExonCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPExon exon1 = *(EnsOExon *const *) item1;
    EnsPExon exon2 = *(EnsOExon *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listExonCompareEndAscending"))
        ajDebug("listExonCompareEndAscending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (exon1 && (!exon2))
        return -1;

    if ((!exon1) && (!exon2))
        return 0;

    if ((!exon1) && exon2)
        return +1;

    return ensFeatureCompareEndAscending(exon1->Feature, exon2->Feature);
}




/* @funcstatic listExonCompareEndDescending ***********************************
**
** AJAX List of Ensembl Exon objects comparison function to sort by
** Ensembl Feature end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Exon address 1
** @param [r] item2 [const void*] Ensembl Exon address 2
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

static int listExonCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPExon exon1 = *(EnsOExon *const *) item1;
    EnsPExon exon2 = *(EnsOExon *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listExonCompareEndDescending"))
        ajDebug("listExonCompareEndDescending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (exon1 && (!exon2))
        return -1;

    if ((!exon1) && (!exon2))
        return 0;

    if ((!exon1) && exon2)
        return +1;

    return ensFeatureCompareEndDescending(exon1->Feature, exon2->Feature);
}




/* @funcstatic listExonCompareIdentifierAscending *****************************
**
** AJAX List of Ensembl Exon objects comparison function to sort by
** identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Exon address 1
** @param [r] item2 [const void*] Ensembl Exon address 2
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

static int listExonCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPExon exon1 = *(EnsOExon *const *) item1;
    EnsPExon exon2 = *(EnsOExon *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listExonCompareIdentifierAscending"))
        ajDebug("listExonCompareIdentifierAscending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (exon1 && (!exon2))
        return -1;

    if ((!exon1) && (!exon2))
        return 0;

    if ((!exon1) && exon2)
        return +1;

    if (exon1->Identifier < exon2->Identifier)
        return -1;

    if (exon1->Identifier > exon2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listExonCompareStartAscending **********************************
**
** AJAX List of Ensembl Exon objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Exon address 1
** @param [r] item2 [const void*] Ensembl Exon address 2
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

static int listExonCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPExon exon1 = *(EnsOExon *const *) item1;
    EnsPExon exon2 = *(EnsOExon *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listExonCompareStartAscending"))
        ajDebug("listExonCompareStartAscending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (exon1 && (!exon2))
        return -1;

    if ((!exon1) && (!exon2))
        return 0;

    if ((!exon1) && exon2)
        return +1;

    return ensFeatureCompareStartAscending(exon1->Feature, exon2->Feature);
}




/* @funcstatic listExonCompareStartDescending *********************************
**
** AJAX List of Ensembl Exon objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Exon address 1
** @param [r] item2 [const void*] Ensembl Exon address 2
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

static int listExonCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPExon exon1 = *(EnsOExon *const *) item1;
    EnsPExon exon2 = *(EnsOExon *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listExonCompareStartDescending"))
        ajDebug("listExonCompareStartDescending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (exon1 && (!exon2))
        return -1;

    if ((!exon1) && (!exon2))
        return 0;

    if ((!exon1) && exon2)
        return +1;

    return ensFeatureCompareStartDescending(exon1->Feature, exon2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Exon Functions for manipulating AJAX List objects of
** Ensembl Exon objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by Ensembl Exon identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * exons [AjPList] AJAX List of Ensembl Exon objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListExonSortEndAscending ******************************************
**
** Sort an AJAX List of Ensembl Exon objects by their
** Ensembl Feature end coordinate in ascending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListExonSortEndAscending(AjPList exons)
{
    if (!exons)
        return ajFalse;

    ajListSortTwoThree(exons,
                       &listExonCompareEndAscending,
                       &listExonCompareStartAscending,
                       &listExonCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListExonSortEndDescending *****************************************
**
** Sort an AJAX List of Ensembl Exon objects by their
** Ensembl Feature end coordinate in descending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListExonSortEndDescending(AjPList exons)
{
    if (!exons)
        return ajFalse;

    ajListSortTwoThree(exons,
                       &listExonCompareEndDescending,
                       &listExonCompareStartDescending,
                       &listExonCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListExonSortIdentifierAscending ***********************************
**
** Sort an AJAX List of Ensembl Exon objects by their
** identifier in ascending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListExonSortIdentifierAscending(AjPList exons)
{
    if (!exons)
        return ajFalse;

    ajListSort(exons, &listExonCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListExonSortStartAscending ****************************************
**
** Sort an AJAX List of Ensembl Exon objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListExonSortStartAscending(AjPList exons)
{
    if (!exons)
        return ajFalse;

    ajListSortTwoThree(exons,
                       &listExonCompareStartAscending,
                       &listExonCompareEndAscending,
                       &listExonCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListExonSortStartDescending ***************************************
**
** Sort an AJAX List of Ensembl Exon objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListExonSortStartDescending(AjPList exons)
{
    if (!exons)
        return ajFalse;

    ajListSortTwoThree(exons,
                       &listExonCompareStartDescending,
                       &listExonCompareEndDescending,
                       &listExonCompareIdentifierAscending);

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
** @nam5rule Exon Convert an Ensembl Exon into an AJAX Feature
**
** @argrule * seq [AjPSeq] AJAX Sequence
** @argrule Exon exon [EnsPExon] Ensembl Exon
** @argrule Exon rank [ajint] Rank in an Ensembl Transcript
** @argrule Exon Pfeature [AjPFeature*] AJAX Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceAddFeatureExon ********************************************
**
** Convert an Ensembl Exon into an AJAX Feature and add it to the
** AJAX Feature Table of an AJAX Sequence.
**
** @param [u] seq [AjPSeq] AJAX Sequence
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [rN] rank [ajint] Rank in an Ensembl Transcript
** @param [wP] Pfeature [AjPFeature*] AJAX Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensSequenceAddFeatureExon(AjPSeq seq,
                                 EnsPExon exon,
                                 ajint rank,
                                 AjPFeature *Pfeature)
{
    AjPStr label = NULL;
    AjPStr type  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon newexon = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!seq)
        return ajFalse;

    if (!exon)
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

    dba = ensExonadaptorGetDatabaseadaptor(exon->Adaptor);

    sla = ensRegistryGetSliceadaptor(dba);

    ensSliceadaptorFetchByName(sla, ajSeqGetNameS(seq), &slice);

    if (!slice)
    {
        ajDebug("ensSequenceAddFeatureExon could not fetch an "
                "Ensembl Slice for AJAX Sequence name '%S'.\n",
                ajSeqGetNameS(seq));

        return ajFalse;
    }

    newexon = ensExonTransfer(exon, slice);

    if (!newexon)
    {
        ajDebug("ensSequenceAddFeatureExon could not transfer "
                "Ensembl Exon %p onto "
                "Ensembl Slice %p.\n", exon, slice);

        ensExonTrace(exon, 1);
        ensSliceTrace(slice, 1);

        ensSliceDel(&slice);

        return ajFalse;
    }

    /* Convert the Ensembl Exon into an AJAX Feature. */

    type = ajStrNewC("exon");

    *Pfeature = ajFeatNewNucFlags(
        seq->Fttable,
        ensAnalysisGetName(ensFeatureGetAnalysis(newexon->Feature)),
        type,
        ensFeatureGetStart(newexon->Feature),
        ensFeatureGetEnd(newexon->Feature),
        0.0F,
        ensFeatureCalculateStrand(newexon->Feature),
        ensExonCalculateFrame(newexon),
        rank,
        0, /* Start 2 */
        0, /* End 2 */
        (AjPStr) NULL, /* Remote Identifier */
        (AjPStr) NULL, /* Label */
        0);

    ensExonFetchDisplayidentifier(newexon, &label);

    ajFeatTagAddCS(*Pfeature, "standard_name", label);

    ajStrDel(&label);
    ajStrDel(&type);

    ensExonDel(&newexon);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @datasection [EnsPExonadaptor] Ensembl Exon Adaptor ************************
**
** @nam2rule Exonadaptor Functions for manipulating
** Ensembl Exon Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor
** @cc CVS Revision: 1.120
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic exonadaptorFetchAllbyStatement *********************************
**
** Fetch all Ensembl Exon objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool exonadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList exons)
{
    ajint sphase = 0;
    ajint ephase = 0;

    ajuint identifier = 0U;
    ajuint version    = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    AjBool current      = AJFALSE;
    AjBool constitutive = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr stableid = NULL;
    AjPStr cdate    = NULL;
    AjPStr mdate    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon      = NULL;
    EnsPExonadaptor ea = NULL;

    EnsPFeature feature = NULL;

    if (ajDebugTest("exonadaptorFetchAllbyStatement"))
        ajDebug("exonadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  exons %p\n",
                ba,
                statement,
                am,
                slice,
                exons);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!exons)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ea  = ensRegistryGetExonadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
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

        ensBaseadaptorRetrieveFeature(ba,
                                      0U,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&stableid);
            ajStrDel(&cdate);
            ajStrDel(&mdate);

            continue;
        }

        /* Finally, create a new Ensembl Exon. */

        exon = ensExonNewIni(ea,
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
** All constructors return a new Ensembl Exon Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Exon Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExonadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPExonadaptor] Ensembl Exon Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensExonadaptorNew ****************************************************
**
** Default constructor for an Ensembl Exon Adaptor.
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPExonadaptor] Ensembl Exon Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExonadaptor ensExonadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPExonadaptor ea = NULL;

    if (!dba)
        return NULL;

    AJNEW0(ea);

    ea->Exonadaptor = ensFeatureadaptorNew(
        dba,
        exonadaptorKTables,
        exonadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &exonadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensExonNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensExonDel,
        (size_t (*)(const void *)) &ensExonCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensExonGetFeature,
        "Exon");

    ea->Exontranscriptadaptor = ensFeatureadaptorNew(
        dba,
        exontranscriptadaptorKTables,
        exonadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        exontranscriptadaptorKDefaultcondition,
        exontranscriptadaptorKFinalcondition,
        &exonadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensExonNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensExonDel,
        (size_t (*)(const void *)) &ensExonCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensExonGetFeature,
        "Exontranscript");

    return ea;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Exon Adaptor object.
**
** @fdata [EnsPExonadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Exon Adaptor
**
** @argrule * Pea [EnsPExonadaptor*] Ensembl Exon Adaptor address
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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensExonadaptorDel(EnsPExonadaptor *Pea)
{
    EnsPExonadaptor pthis = NULL;

    if (!Pea)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensExonadaptorDel"))
        ajDebug("ensExonadaptorDel\n"
                "  *Pea %p\n",
                *Pea);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    if (!*Pea)
        return;

    pthis = *Pea;

    ensFeatureadaptorDel(&pthis->Exonadaptor);
    ensFeatureadaptorDel(&pthis->Exontranscriptadaptor);

    AJFREE(pthis);

    *Pea = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Exon Adaptor object.
**
** @fdata [EnsPExonadaptor]
**
** @nam3rule Get Return Ensembl Exon Adaptor attribute(s)
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ea [EnsPExonadaptor] Ensembl Exon Adaptor
**
** @valrule Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensExonadaptorGetDatabaseadaptor *************************************
**
** Get the Ensembl Database Adaptor member of the
** Ensembl Feature Adaptor member of an Ensembl Exon Adaptor.
**
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensExonadaptorGetDatabaseadaptor(EnsPExonadaptor ea)
{
    return (ea) ? ensFeatureadaptorGetDatabaseadaptor(ea->Exonadaptor) : NULL;
}




/* @func ensExonadaptorGetFeatureadaptor **************************************
**
** Get the Ensembl Feature Adaptor member of an Ensembl Exon Adaptor.
**
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensExonadaptorGetFeatureadaptor(EnsPExonadaptor ea)
{
    return (ea) ? ea->Exonadaptor : NULL;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Exon objects from an
** Ensembl SQL database.
**
** @fdata [EnsPExonadaptor]
**
** @nam3rule Fetch Fetch Ensembl Exon object(s)
** @nam4rule All   Fetch all Ensembl Exon objects
** @nam4rule Allby Fetch all Ensembl Exon objects matching a criterion
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable Ensembl Exon identifier
** @nam5rule Transcript       Fetch all by an Ensembl Transcript
** @nam4rule By    Fetch one Ensembl Exon object matching a criterion
** @nam5rule Identifier       Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by stable Ensembl Exon identifier
**
** @argrule * ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @argrule All exons [AjPList] AJAX List of Ensembl Exon objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice exons [AjPList] AJAX List of Ensembl Exon objects
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule AllbyStableidentifier exons [AjPList] AJAX List of
** Ensembl Exon objects
** @argrule AllbyTranscript transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule AllbyTranscript exons [AjPList] AJAX List of Ensembl Exon objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pexon [EnsPExon*] Ensembl Exon address
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule ByStableidentifier Pexon [EnsPExon*] Ensembl Exon address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensExonadaptorFetchAll ***********************************************
**
** Fetch all Ensembl Exon objects.
**
** The caller is responsible for deleting the Ensembl Exon objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAll(EnsPExonadaptor ea,
                              AjPList exons)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ea)
        return ajFalse;

    if (!exons)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Exonadaptor);

    constraint = ajStrNewC("exon.is_current = 1");

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                exons);

    ajStrDel(&constraint);

    return result;
}




/* @func ensExonadaptorFetchAllbySlice ****************************************
**
** Fetch all Ensembl Exon objects matching a SQL constraint on an
** Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Exon objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice_constraint
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] constraint [const AjPStr] SQL constraint
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllbySlice(EnsPExonadaptor ea,
                                     EnsPSlice slice,
                                     const AjPStr constraint,
                                     AjPList exons)
{
    if (!ea)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!exons)
        return ajFalse;

    ensFeatureadaptorFetchAllbySlice(ea->Exonadaptor,
                                     slice,
                                     constraint,
                                     (AjPStr) NULL,
                                     exons);

    return ajTrue;
}




/* @func ensExonadaptorFetchAllbyStableidentifier *****************************
**
** Fetch all Ensembl Exon versions via a stable identifier.
**
** The caller is responsible for deleting the Ensembl Exon objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::fetch_all_versions_by_stable_id
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllbyStableidentifier(EnsPExonadaptor ea,
                                                const AjPStr stableid,
                                                AjPList exons)
{
    char *txtstableid = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ea)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!exons)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Exonadaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("exon.stable_id = '%s'", txtstableid);

    ajCharDel(&txtstableid);

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       exons);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensExonadaptorFetchAllbyTranscript ***********************************
**
** Fetch all Ensembl Exon objects via an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Exon objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::fetch_all_by_Transcript
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonadaptorFetchAllbyTranscript(EnsPExonadaptor ea,
                                          const EnsPTranscript transcript,
                                          AjPList exons)
{
    AjBool circular = AJFALSE;

    AjIList iter = NULL;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature tfeature = NULL;

    EnsPSlice eslice    = NULL;
    EnsPSlice tslice    = NULL;
    EnsPSliceadaptor sa = NULL;

    if (ajDebugTest("ensExonadaptorFetchAllbyTranscript"))
    {
        ajDebug("ensExonadaptorFetchAllbyTranscript\n"
                "  ea %p\n"
                "  transcript %p\n"
                "  exons %p\n",
                ea,
                transcript,
                exons);

        ensTranscriptTrace(transcript, 1);
    }

    if (!ea)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!exons)
        return ajFalse;

    tfeature = ensTranscriptGetFeature(transcript);

    tslice = ensFeatureGetSlice(tfeature);

    if (!tslice)
    {
        ajDebug("ensExonadaptorFetchAllbyTranscript cannot fetch Exon objects "
                "for an Ensembl Transcript without an Ensembl Slice.\n");

        return ajFalse;
    }

    if (ensSliceIsCircular(tslice, &circular) == ajFalse)
        return ajFalse;

    if (circular == ajTrue)
        eslice = ensSliceNewRef(tslice);
    else
    {
        /*
        ** For Transcript objects on linear Slice objects fetch a Slice that
        ** spans just this Transcript for placing Exon objects.
        */

        dba = ensFeatureadaptorGetDatabaseadaptor(ea->Exontranscriptadaptor);

        sa = ensRegistryGetSliceadaptor(dba);

        ensSliceadaptorFetchByFeature(sa, tfeature, 0, &eslice);
    }

    /*
    ** Use the Exon Transcript Adaptor, which has the "exon_transcript" SQL
    ** table joined permanently via a default SQL condition.
    */

    constraint = ajFmtStr(
        "exon_transcript.transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

    ensFeatureadaptorFetchAllbySlice(ea->Exontranscriptadaptor,
                                     eslice,
                                     constraint,
                                     (const AjPStr) NULL,
                                     exons);

    /*
    ** Remap Exon coordinates if neccessary.
    ** NOTE: Ensembl Exon objects are fetched on an Ensembl Slice, which spans
    ** only the Ensembl Transcript, to work with haplotypes (HAPs) and
    ** pseudo-autosomal regions (PARs).
    */

    if (!ensSliceMatch(eslice, tslice))
    {
        iter = ajListIterNew(exons);

        while (!ajListIterDone(iter))
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




/* @func ensExonadaptorFetchByIdentifier **************************************
**
** Fetch an Ensembl Exon via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Exon.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pexon [EnsPExon*] Ensembl Exon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExonadaptorFetchByIdentifier(EnsPExonadaptor ea,
                                       ajuint identifier,
                                       EnsPExon *Pexon)
{
    EnsPBaseadaptor ba = NULL;

    if (!ea)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pexon)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Exonadaptor);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void **) Pexon);
}




/* @func ensExonadaptorFetchByStableidentifier ********************************
**
** Fetch an Ensembl Exon via its stable identifier and version.
** In case a particular version is not specified,
** the current Exon will be returned.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::fetch_by_stable_id
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Pexon [EnsPExon*] Ensembl Exon address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonadaptorFetchByStableidentifier(EnsPExonadaptor ea,
                                             const AjPStr stableid,
                                             ajuint version,
                                             EnsPExon *Pexon)
{
    char *txtstableid = NULL;

    AjPList exons = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPExon exon = NULL;

    if (!ea)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Pexon)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Exonadaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if (version)
        constraint = ajFmtStr(
            "exon.stable_id = '%s' "
            "AND "
            "exon.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "exon.stable_id = '%s' "
            "AND "
            "exon.is_current = 1",
            txtstableid);

    ajCharDel(&txtstableid);

    exons = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       exons);

    if (ajListGetLength(exons) > 1)
        ajDebug("ensExonadaptorFetchByStableId got more than one "
                "Exon for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(exons, (void **) Pexon);

    while (ajListPop(exons, (void **) &exon))
        ensExonDel(&exon);

    ajListFree(&exons);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Exon objects from an
** Ensembl SQL database.
**
** @fdata [EnsPExonadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Exon-releated object(s)
** @nam4rule All Retrieve all Ensembl Exon-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
** @nam5rule Stableidentifiers Fetch all stable Ensembl Exon identifiers
**
** @argrule * ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
** @argrule AllStableidentifiers identifiers [AjPList] AJAX List of AJAX String
**                                              stable Ensembl Exon identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensExonadaptorRetrieveAllIdentifiers *********************************
**
** Retrieve all SQL database-internal identifiers of Ensembl Exon objects.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::list_dbIDs
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonadaptorRetrieveAllIdentifiers(EnsPExonadaptor ea,
                                            AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ea)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Exonadaptor);

    table = ajStrNewC("exon");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensExonadaptorRetrieveAllStableidentifiers ***************************
**
** Retrieve all stable identifiers of Ensembl Exon objects.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::ExonAdaptor::list_stable_ids
** @param [u] ea [EnsPExonadaptor] Ensembl Exon Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExonadaptorRetrieveAllStableidentifiers(EnsPExonadaptor ea,
                                                  AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table   = NULL;
    AjPStr primary = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ea)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ea->Exonadaptor);

    table   = ajStrNewC("exon");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(ba, table, primary, identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}




/* @datasection [EnsPDatabaseadaptor] Ensembl Supporting Feature Adaptor ******
**
** @nam2rule Supportingfeatureadaptor Functions for manipulating
** Ensembl Supporting Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SupportingFeatureAdaptor
** @cc CVS Revision: 1.22
** @cc CVS Tag: branch-ensembl-66
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
** @nam5rule Exon       Fetch all Ensembl Base Align Feature objects matching
**                      an Ensembl Exon
** @nam4rule FetchBy Fetch one Ensembl Base Align Feature object
**                   matching a criterion
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule AllbyExon exon [EnsPExon] Ensembl Exon
** @argrule AllbyExon bafs [AjPList] AJAX List of Ensembl Base Align Feature
**                                  objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSupportingfeatureadaptorFetchAllbyExon ****************************
**
** Fetch all supporting Ensembl Base Align Feature objects via an Ensembl Exon.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSupportingfeatureadaptorFetchAllbyExon(EnsPDatabaseadaptor dba,
                                                 EnsPExon exon,
                                                 AjPList bafs)
{
    ajuint identifier = 0U;

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

    EnsPDnaalignfeatureadaptor dafa = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if (!dba)
        return ajFalse;

    if (!exon)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (ensExonGetIdentifier(exon) == 0)
    {
        ajDebug("ensSupportingfeatureadaptorFetchAllbyExon cannot get "
                "supporting Ensembl Base Align Feature objects for an "
                "Ensembl Exon without an identifier.\n");

        return ajFalse;
    }

    efeature = ensExonGetFeature(exon);

    eslice = ensFeatureGetSlice(efeature);

    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);

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

    while (!ajSqlrowiterDone(sqli))
    {
        type = ajStrNew();
        identifier = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &identifier);

        if (ajStrMatchC(type, "dna_align_feature"))
            ensDnaalignfeatureadaptorFetchByIdentifier(dafa,
                                                       identifier,
                                                       &baf);
        else if (ajStrMatchC(type, "protein_align_feature"))
            ensProteinalignfeatureadaptorFetchByIdentifier(pafa,
                                                           identifier,
                                                           &baf);
        else
            ajWarn("ensSupportingfeatureadaptorFetchAllbyExon got "
                   "unexpected value in supporting_feature.feature_type "
                   "'%S'.\n", type);

        if (baf)
        {
            ofeature = ensFeaturepairGetSourceFeature(baf->Featurepair);

            nfeature = ensFeatureTransfer(ofeature, eslice);

            ensFeaturepairSetSourceFeature(baf->Featurepair, nfeature);

            ensFeatureDel(&nfeature);

            ajListPushAppend(bafs, (void *) baf);
        }
        else
            ajDebug("ensSupportingfeatureadaptorFetchAllbyExon could not "
                    "retrieve Supporting feature of type '%S' and "
                    "identifier %u from database.\n", type, identifier);

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

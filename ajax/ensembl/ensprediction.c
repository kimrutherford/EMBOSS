/* @source Ensembl Prediction functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.38 $
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

#include "ensprediction.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic predictionexonadaptorTables ***********************************
**
** Array of Ensembl Prediction Exon Adaptor SQL table names
**
******************************************************************************/

static const char* const predictionexonadaptorTables[] =
{
    "prediction_exon",
    (const char*) NULL
};




/* @conststatic predictionexonadaptorColumns **********************************
**
** Array of Ensembl Prediction Exon Adaptor SQL column names
**
******************************************************************************/

static const char* const predictionexonadaptorColumns[] =
{
    "prediction_exon.prediction_exon_id",
    "prediction_exon.seq_region_id",
    "prediction_exon.seq_region_start",
    "prediction_exon.seq_region_end",
    "prediction_exon.seq_region_strand",
    "prediction_exon.start_phase",
    "prediction_exon.score",
    "prediction_exon.p_value",
    (const char*) NULL
};




/* @conststatic predictionexonadaptorFinalcondition ***************************
**
** Ensembl Prediction Exon Adaptor SQL final condition
**
******************************************************************************/

static const char* predictionexonadaptorFinalcondition =
    "ORDER BY "
    "prediction_exon.prediction_transcript_id, prediction_exon.exon_rank";




/* @conststatic predictiontranscriptadaptorTables *****************************
**
** Array of Ensembl Prediction Transcript Adaptor SQL table names
**
******************************************************************************/

static const char* const predictiontranscriptadaptorTables[] =
{
    "prediction_transcript",
    (const char*) NULL
};




/* @conststatic predictiontranscriptadaptorColumns ****************************
**
** Array of Ensembl Prediction Transcript Adaptor SQL column names
**
******************************************************************************/

static const char* const predictiontranscriptadaptorColumns[] =
{
    "prediction_transcript.prediction_transcript_id",
    "prediction_transcript.seq_region_id",
    "prediction_transcript.seq_region_start",
    "prediction_transcript.seq_region_end",
    "prediction_transcript.seq_region_strand",
    "prediction_transcript.analysis_id",
    "prediction_transcript.display_label",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static int listPredictionexonCompareStartAscending(const void* P1,
                                                   const void* P2);

static int listPredictionexonCompareStartDescending(const void* P1,
                                                    const void* P2);

static AjBool predictionexonadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList pes);

static void* predictionexonadaptorCacheReference(void* value);

static void predictionexonadaptorCacheDelete(void** value);

static size_t predictionexonadaptorCacheSize(const void* value);

static EnsPFeature predictionexonadaptorGetFeature(const void* value);

static EnsPPredictiontranscript predictiontranscriptNewCpyFeatures(
    EnsPPredictiontranscript pt);

static int listPredictiontranscriptCompareStartAscending(const void* P1,
                                                         const void* P2);

static int listPredictiontranscriptCompareStartDescending(const void* P1,
                                                          const void* P2);

static AjBool predictiontranscriptadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList pts);

static void* predictiontranscriptadaptorCacheReference(void* value);

static void predictiontranscriptadaptorCacheDelete(void** value);

static size_t predictiontranscriptadaptorCacheSize(const void* value);

static EnsPFeature predictiontranscriptadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensprediction *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPPredictionexon] Ensembl Prediction Exon ******************
**
** @nam2rule Predictionexon Functions for manipulating
** Ensembl Prediction Exon objects
**
** @cc Bio::EnsEMBL::PredictionExon
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Exon by pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Exon. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy pe [const EnsPPredictionexon] Ensembl Prediction Exon
** @argrule Ini pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini sphase [ajint] Start phase of Translation
** @argrule Ini score [double] Score
** @argrule Ini pvalue [double] P-value
** @argrule Ref pe [EnsPPredictionexon] Ensembl Prediction Exon
**
** @valrule * [EnsPPredictionexon] Ensembl Prediction Exon or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensPredictionexonNewCpy **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonNewCpy(const EnsPPredictionexon pe)
{
    EnsPPredictionexon pthis = NULL;

    if(!pe)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = pe->Identifier;

    pthis->Adaptor = pe->Adaptor;

    pthis->Feature = ensFeatureNewRef(pe->Feature);

    pthis->PhaseStart = pe->PhaseStart;

    pthis->Score = pe->Score;

    pthis->Pvalue = pe->Pvalue;

    if(pe->SequenceCache)
        pthis->SequenceCache = ajStrNewRef(pe->SequenceCache);

    return pthis;
}




/* @func ensPredictionexonNewIni **********************************************
**
** Constructor for an Ensembl Prediction Exon with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::PredictionExon::new
** @param [r] sphase [ajint] Start phase of Translation
** @param [r] score [double] Score
** @param [r] pvalue [double] P-value
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonNewIni(EnsPPredictionexonadaptor pea,
                                           ajuint identifier,
                                           EnsPFeature feature,
                                           ajint sphase,
                                           double score,
                                           double pvalue)
{
    EnsPPredictionexon pe = NULL;

    if(!feature)
        return NULL;

    AJNEW0(pe);

    pe->Use = 1;

    pe->Identifier = identifier;

    pe->Adaptor = pea;

    pe->Feature = ensFeatureNewRef(feature);

    pe->PhaseStart = sphase;

    pe->Score = score;

    pe->Pvalue = pvalue;

    return pe;
}




/* @func ensPredictionexonNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonNewRef(EnsPPredictionexon pe)
{
    if(!pe)
        return NULL;

    pe->Use++;

    return pe;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Del Destroy (free) an Ensembl Prediction Exon object
**
** @argrule * Ppe [EnsPPredictionexon*] Ensembl Prediction Exon object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictionexonDel *************************************************
**
** Default destructor for an Ensembl Prediction Exon.
**
** @param [d] Ppe [EnsPPredictionexon*] Ensembl Prediction Exon object address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictionexonDel(EnsPPredictionexon* Ppe)
{
    EnsPPredictionexon pthis = NULL;

    if(!Ppe)
        return;

    if(!*Ppe)
        return;

    pthis = *Ppe;

    pthis->Use--;

    if(pthis->Use)
    {
        *Ppe = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->SequenceCache);

    AJFREE(pthis);

    *Ppe = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Get Return Prediction Exon attribute(s)
** @nam4rule Adaptor Return the Ensembl Prediction Exon Adaptor
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Phase Return a phase
** @nam5rule Start Return the phase at start
** @nam4rule Pvalue Return the p-value
** @nam4rule Score Return the score
**
** @argrule * pe [const EnsPPredictionexon] Prediction Exon
**
** @valrule Adaptor [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**`or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule PhaseStart [ajint] Phase at start or 0
** @valrule Pvalue [double] P-value or 0.0
** @valrule Score [double] Score or 0.0
**
** @fcategory use
******************************************************************************/




/* @func ensPredictionexonGetAdaptor ******************************************
**
** Get the Ensembl Prediction Exon Adaptor element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictionexonadaptor ensPredictionexonGetAdaptor(
    const EnsPPredictionexon pe)
{
    if(!pe)
        return NULL;

    return pe->Adaptor;
}




/* @func ensPredictionexonGetFeature ******************************************
**
** Get the Ensembl Feature element of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensPredictionexonGetFeature(const EnsPPredictionexon pe)
{
    if(!pe)
        return NULL;

    return pe->Feature;
}




/* @func ensPredictionexonGetIdentifier ***************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensPredictionexonGetIdentifier(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0;

    return pe->Identifier;
}




/* @func ensPredictionexonGetPhaseStart ***************************************
**
** Get the phase at start element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::PredictionExon::phase
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajint] Start phase or 0
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

ajint ensPredictionexonGetPhaseStart(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0;

    return pe->PhaseStart;
}




/* @func ensPredictionexonGetPvalue *******************************************
**
** Get the p-value element of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [double] P-value or 0.0
** @@
******************************************************************************/

double ensPredictionexonGetPvalue(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0.0;

    return pe->Pvalue;
}




/* @func ensPredictionexonGetScore ********************************************
**
** Get the score element of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [double] Score or 0.0
** @@
******************************************************************************/

double ensPredictionexonGetScore(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0.0;

    return pe->Score;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Set Set one element of a Prediction Exon
** @nam4rule Adaptor Set the Ensembl Prediction Exon Adaptor
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Phase Set a phase
** @nam5rule Start Set the phase at start
** @nam4rule Pvalue Set the p-value
** @nam4rule Score Set the score
**
** @argrule * pe [EnsPPredictionexon] Ensembl Prediction Exon object
** @argrule Adaptor pea [EnsPPredictionexonadaptor]
** Ensembl Prediction Exon Adaptor
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule PhaseStart sphase [ajint] Start phase
** @argrule Pvalue pvalue [double] P-value
** @argrule Score score [double] Score
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensPredictionexonSetAdaptor ******************************************
**
** Set the Ensembl Prediction Exon Adaptor element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [u] pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetAdaptor(EnsPPredictionexon pe,
                                   EnsPPredictionexonadaptor pea)
{
    if(!pe)
        return ajFalse;

    pe->Adaptor = pea;

    return ajTrue;
}




/* @func ensPredictionexonSetFeature ******************************************
**
** Set the Ensembl Feature element of an Ensembl Prediction Exon.
**
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetFeature(EnsPPredictionexon pe,
                                   EnsPFeature feature)
{
    if(ajDebugTest("ensPredictionexonSetFeature"))
    {
        ajDebug("ensPredictionexonSetFeature\n"
                "  pe %p\n"
                "  feature %p\n",
                pe,
                feature);

        ensPredictionexonTrace(pe, 1);

        ensFeatureTrace(feature, 1);
    }

    if(!pe)
        return ajFalse;

    if(!feature)
        return ajFalse;

    /* Replace the current Feature. */

    if(pe->Feature)
        ensFeatureDel(&pe->Feature);

    pe->Feature = ensFeatureNewRef(feature);

    /* Clear the sequence cache. */

    ajStrDel(&pe->SequenceCache);

    return ajTrue;
}




/* @func ensPredictionexonSetIdentifier ***************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetIdentifier(EnsPPredictionexon pe,
                                      ajuint identifier)
{
    if(!pe)
        return ajFalse;

    pe->Identifier = identifier;

    return ajTrue;
}




/* @func ensPredictionexonSetPhaseStart ***************************************
**
** Set the phase at start element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::PredictionExon::phase
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] sphase [ajint] Start phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetPhaseStart(EnsPPredictionexon pe,
                                      ajint sphase)
{
    if(!pe)
        return ajFalse;

    pe->PhaseStart = sphase;

    return ajTrue;
}




/* @func ensPredictionexonSetPvalue *******************************************
**
** Set the p-value element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::PredictionExon::p_value
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] pvalue [double] P-value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetPvalue(EnsPPredictionexon pe,
                                  double pvalue)
{
    if(!pe)
        return ajFalse;

    pe->Pvalue = pvalue;

    return ajTrue;
}




/* @func ensPredictionexonSetScore ********************************************
**
** Set the score element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::PredictionExon::score
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetScore(EnsPPredictionexon pe,
                                 double score)
{
    if(!pe)
        return ajFalse;

    pe->Score = score;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Trace Report Ensembl Prediction Exon elements to debug file
**
** @argrule Trace pe [const EnsPPredictionexon] Ensembl Prediction Exon
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictionexonTrace ***********************************************
**
** Trace an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonTrace(const EnsPPredictionexon pe, ajuint level)
{
    AjPStr indent = NULL;

    if(!pe)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensPredictionexonTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  PhaseStart %d\n"
            "%S  Score %f\n"
            "%S  P-value %f\n",
            indent, pe,
            indent, pe->Use,
            indent, pe->Identifier,
            indent, pe->Adaptor,
            indent, pe->Feature,
            indent, pe->PhaseStart,
            indent, pe->Score,
            indent, pe->Pvalue);

    ensFeatureTrace(pe->Feature, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Calculate Calculate Ensembl Prediction Exon values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictionexonCalculateMemsize ************************************
**
** Calculate the memory size in bytes of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensPredictionexonCalculateMemsize(const EnsPPredictionexon pe)
{
    size_t size = 0;

    if(!pe)
        return 0;

    size += sizeof (EnsOPredictionexon);

    size += ensFeatureCalculateMemsize(pe->Feature);

    return size;
}




/* @section convenience functions *********************************************
**
** Ensembl Prediction Exon convenience functions
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Phase Get the phase
** @nam5rule End Get the end phase
**
** @argrule * pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @valrule PhaseEnd [ajint] End phase or 0
**
** @fcategory use
******************************************************************************/




/* @func ensPredictionexonGetPhaseEnd *****************************************
**
** Get the end phase element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::PredictionExon::end_phase
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajint] End phase or 0
** @@
******************************************************************************/

ajint ensPredictionexonGetPhaseEnd(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0;

    return (pe->PhaseStart + ensFeatureCalculateLength(pe->Feature)) % 3;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Prediction Exon objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Transfer Transfer an Ensembl Prediction Exon
** @nam3rule Transform Transform an Ensembl Prediction Exon
**
** @argrule * pe [EnsPPredictionexon] Ensembl Prediction Exon
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
**
** @valrule * [EnsPPredictionexon] Ensembl Prediction Exon or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictionexonTransfer ********************************************
**
** Transfer an Ensembl Prediction Exon onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::PredictionExon::transfer
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon on the defined
**                              Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonTransfer(EnsPPredictionexon pe,
                                             EnsPSlice slice)
{
    EnsPPredictionexon npe = NULL;
    EnsPFeature newfeature = NULL;

    if(!pe)
        return NULL;

    if(!slice)
        return NULL;

    newfeature = ensFeatureTransfer(pe->Feature, slice);

    if(!newfeature)
        return NULL;

    npe = ensPredictionexonNewCpy(pe);

    ensPredictionexonSetFeature(npe, newfeature);

    ensFeatureDel(&newfeature);

    return npe;
}




/* @func ensPredictionexonTransform *******************************************
**
** Transform an Ensembl Prediction Exon into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::PredictionExon::transform
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon in the defined
**                              Ensembl Coordinate System or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonTransform(EnsPPredictionexon pe,
                                              const AjPStr csname,
                                              const AjPStr csversion)
{
    EnsPPredictionexon npe = NULL;
    EnsPFeature nfeature   = NULL;

    if(!pe)
        return NULL;

    if(!csname)
        return NULL;

    if(!csversion)
        return NULL;

    nfeature = ensFeatureTransform(pe->Feature,
                                   csname,
                                   csversion,
                                   (EnsPSlice) NULL);

    if(!nfeature)
        return NULL;

    npe = ensPredictionexonNewCpy(pe);

    ensPredictionexonSetFeature(npe, nfeature);

    ensFeatureDel(&nfeature);

    return npe;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
**
** @nam3rule Fetch Fetch Ensembl Prediction Exon values
** @nam4rule Displayidentifier Fetch the display identifier
** @nam4rule Sequence Fetch the sequence
** @nam5rule Seq Fetch the sequence as AJAX Sequence object
** @nam5rule Str Fetch the sequence as AJAX String object
**
** @argrule Displayidentifier pe [const EnsPPredictionexon] Ensembl Prediction
** Exon
** @argrule Displayidentifier Pidentifier [AjPStr*] Display identifier address
** @argrule SequenceSeq pe [EnsPPredictionexon] Ensembl Prediction Exon
** @argrule SequenceSeq Psequence [AjPSeq*] AJAX Sequence object address
** @argrule SequenceStr pe [EnsPPredictionexon] Ensembl Prediction Exon
** @argrule SequenceStr Psequence [AjPStr*] AJAX String object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictionexonFetchSequenceSeq ************************************
**
** Fetch the sequence of an Ensembl Prediction Exon as AJAX Sequence.
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::PredictionExon:seq
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [wP] Psequence [AjPSeq*] Ensembl Prediction Exon sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonFetchSequenceSeq(EnsPPredictionexon pe,
                                         AjPSeq* Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!pe)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    ensPredictionexonFetchSequenceStr(pe, &sequence);

    name = ajFmtStr("%u", pe->Identifier);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensPredictionexonFetchSequenceStr ************************************
**
** Fetch the sequence of an Ensembl Prediction Exon as AJAX String.
** The caller is responsible for deleting the AJAX String.
**
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [wP] Psequence [AjPStr*] Ensembl Exon sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonFetchSequenceStr(EnsPPredictionexon pe,
                                         AjPStr* Psequence)
{
    EnsPFeature feature = NULL;

    EnsPSlice slice = NULL;

    if(!pe)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    feature = pe->Feature;

    if(!feature)
    {
        ajWarn("ensPredictionexonFetchSequenceStr cannot get sequence without "
               "an Ensembl Feature attached to the Ensembl Prediction Exon "
               "with idetifier %u.\n",
               pe->Identifier);

        return ajFalse;
    }

    slice = ensFeatureGetSlice(feature);

    if(!slice)
    {
        ajWarn("ensPredictionExonFetchSequenceStr cannot get sequence without "
               "an Ensembl Slice attached to the Ensembl Feature in the "
               "Ensembl Prediction Exon with identifier %u.\n",
               pe->Identifier);

        return ajFalse;
    }

    if(!pe->SequenceCache)
        pe->SequenceCache = ajStrNew();

    if(!ajStrGetLen(pe->SequenceCache))
        ensSliceFetchSequenceSubStr(slice,
                                    feature->Start,
                                    feature->End,
                                    feature->Strand,
                                    &pe->SequenceCache);

    *Psequence = ajStrNewRef(pe->SequenceCache);

    return ajTrue;
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
** @nam3rule Predictionexon Functions for manipulating AJAX List objects of
** Ensembl Prediction Exon objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending pes [AjPList] AJAX List of
** Ensembl Prediction Exon objects
** @argrule Descending pes [AjPList] AJAX List of
** Ensembl Prediction Exon objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listPredictionexonCompareStartAscending ************************
**
** AJAX List of Ensembl Prediction Exon objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Prediction Exon address 1
** @param [r] P2 [const void*] Ensembl Prediction Exon address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listPredictionexonCompareStartAscending(const void* P1,
                                                   const void* P2)
{
    const EnsPPredictionexon pe1 = NULL;
    const EnsPPredictionexon pe2 = NULL;

    pe1 = *(EnsPPredictionexon const*) P1;
    pe2 = *(EnsPPredictionexon const*) P2;

    if(ajDebugTest("listPredictionexonCompareStartAscending"))
        ajDebug("listPredictionexonCompareStartAscending\n"
                "  pe1 %p\n"
                "  pe2 %p\n",
                pe1,
                pe2);

    /* Sort empty values towards the end of the AJAX List. */

    if(pe1 && (!pe2))
        return -1;

    if((!pe1) && (!pe2))
        return 0;

    if((!pe1) && pe2)
        return +1;

    return ensFeatureCompareStartAscending(pe1->Feature, pe2->Feature);
}




/* @func ensListPredictionexonSortStartAscending ******************************
**
** Sort an AJAX List of Ensembl Prediction Exon objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] pes [AjPList] AJAX List of Ensembl Prediction Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListPredictionexonSortStartAscending(AjPList pes)
{
    if(!pes)
        return ajFalse;

    ajListSort(pes, listPredictionexonCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listPredictionexonCompareStartDescending ***********************
**
** AJAX List of Ensembl Exon objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Prediction Exon address 1
** @param [r] P2 [const void*] Ensembl Prediction Exon address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listPredictionexonCompareStartDescending(const void* P1,
                                                    const void* P2)
{
    const EnsPPredictionexon pe1 = NULL;
    const EnsPPredictionexon pe2 = NULL;

    pe1 = *(EnsPPredictionexon const*) P1;
    pe2 = *(EnsPPredictionexon const*) P2;

    if(ajDebugTest("listPredictionexonCompareStartDescending"))
        ajDebug("listPredictionexonCompareStartDescending\n"
                "  pe1 %p\n"
                "  pe2 %p\n",
                pe1,
                pe2);

    /* Sort empty values towards the end of the AJAX List. */

    if(pe1 && (!pe2))
        return -1;

    if((!pe1) && (!pe2))
        return 0;

    if((!pe1) && pe2)
        return +1;

    return ensFeatureCompareStartDescending(pe1->Feature, pe2->Feature);
}




/* @func ensListPredictionexonSortStartDescending *****************************
**
** Sort an AJAX List of Ensembl Prediction Exon objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] pes [AjPList] AJAX List of Ensembl Prediction Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListPredictionexonSortStartDescending(AjPList pes)
{
    if(!pes)
        return ajFalse;

    ajListSort(pes, listPredictionexonCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor ***
**
** @nam2rule Predictionexonadaptor Functions for manipulating
** Ensembl Prediction Exon Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::PredictionExonAdaptor
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic predictionexonadaptorFetchAllbyStatement ***********************
**
** Fetch all Ensembl Prediction Exon objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] pes [AjPList] AJAX List of Ensembl Prediction Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool predictionexonadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList pes)
{
    double score  = 0.0;
    double pvalue = 0.0;

    ajint sphase = 0;

    ajuint identifier = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPPredictionexon pe         = NULL;
    EnsPPredictionexonadaptor pea = NULL;

    EnsPFeature feature = NULL;
    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("predictionexonadaptorFetchAllbyStatement"))
        ajDebug("predictionexonadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  pes %p\n",
                dba,
                statement,
                am,
                slice,
                pes);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!pes)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(dba);

    pea = ensRegistryGetPredictionexonadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        srstrand   = 0;
        sphase     = 0;
        score      = 0.0;
        pvalue     = 0.0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToInt(sqlr, &sphase);
        ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToDouble(sqlr, &pvalue);

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
            ajFatal("predictionexonadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("predictionexonadaptorFetchAllbyStatement got a "
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
                ajFatal("exonAdaptorFetchAllbyStatement got a Slice, "
                        "which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceCalculateLength(slice), INT_MAX);

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
            ** Throw away Ensembl Feature objects off the end of the requested
            ** Slice or on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Load the next Feature but destroy first! */

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        /* Finally, create a new Ensembl Exon. */

        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

        pe = ensPredictionexonNewIni(pea,
                                     identifier,
                                     feature,
                                     sphase,
                                     score,
                                     pvalue);

        ajListPushAppend(pes, (void*) pe);

        ensFeatureDel(&feature);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic predictionexonadaptorCacheReference ****************************
**
** Wrapper function to reference an Ensembl Prediction Exon from an
** Ensembl Cache.
**
** @param [r] value [void*] Ensembl Prediction Exon
**
** @return [void*] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

static void* predictionexonadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensPredictionexonNewRef((EnsPPredictionexon) value);
}




/* @funcstatic predictionexonadaptorCacheDelete *******************************
**
** Wrapper function to delete an Ensembl Prediction Exon from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Prediction Exon address
**
** @return [void]
** @@
******************************************************************************/

static void predictionexonadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensPredictionexonDel((EnsPPredictionexon*) value);

    return;
}




/* @funcstatic predictionexonadaptorCacheSize *********************************
**
** Wrapper function to determine the memory size of an Ensembl Prediction Exon
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Prediction Exon
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t predictionexonadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensPredictionexonCalculateMemsize((const EnsPPredictionexon) value);
}




/* @funcstatic predictionexonadaptorGetFeature ********************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Prediction Exon
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Prediction Exon
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature predictionexonadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensPredictionexonGetFeature((const EnsPPredictionexon) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Exon Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Exon Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictionexonadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensPredictionexonadaptorNew ******************************************
**
** Default constructor for an Ensembl Prediction Exon Adaptor.
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
** @see ensRegistryGetPredictionexonadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictionexonadaptor ensPredictionexonadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        predictionexonadaptorTables,
        predictionexonadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        predictionexonadaptorFinalcondition,
        predictionexonadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        predictionexonadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        predictionexonadaptorCacheDelete,
        predictionexonadaptorCacheSize,
        predictionexonadaptorGetFeature,
        "Predictionexon");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Prediction Exon Adaptor object.
**
** @fdata [EnsPPredictionexonadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Prediction Exon Adaptor object
**
** @argrule * Ppea [EnsPPredictionexonadaptor*] Ensembl Prediction Exon Adaptor
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictionexonadaptorDel ******************************************
**
** Default destructor for an Ensembl Prediction Exon Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Ppea [EnsPPredictionexonadaptor*] Ensembl Prediction Exon Adaptor
**                                              object address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictionexonadaptorDel(EnsPPredictionexonadaptor* Ppea)
{
    if(!Ppea)
        return;

    ensFeatureadaptorDel(Ppea);

    *Ppea = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Prediction Exon Adaptor object.
**
** @fdata [EnsPPredictionexonadaptor]
**
** @nam3rule Get Return Ensembl Prediction Exon Adaptor attribute(s)
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**
** @valrule Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensPredictionexonadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor element of the
** Ensembl Feature Adaptor element of an Ensembl Prediction Exon Adaptor.
**
** @param [u] pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensPredictionexonadaptorGetDatabaseadaptor(
    EnsPPredictionexonadaptor pea)
{
    if(!pea)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(pea);
}




/* @func ensPredictionexonadaptorGetFeatureadaptor ****************************
**
** Get the Ensembl Feature Adaptor element of an
** Ensembl Prediction Exon Adaptor.
**
** @param [u] pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

EnsPFeatureadaptor ensPredictionexonadaptorGetFeatureadaptor(
    EnsPPredictionexonadaptor pea)
{
    if(!pea)
        return NULL;

    return pea;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Prediction Exon objects from an
** Ensembl SQL database.
**
** @fdata [EnsPPredictionexonadaptor]
**
** @nam3rule Fetch Fetch Ensembl Prediction Exon object(s)
** @nam4rule All   Fetch all Ensembl Prediction Exon objects
** @nam4rule Allby Fetch all Ensembl Prediction Exon objects
** matching a criterion
** @nam5rule Predictiontranscript Fetch all by an Ensembl Prediction Transcript
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam4rule By Fetch one Ensembl Exon object matching a criterion
** @nam5rule Identifier Fetch by SQL database-internal identifier
**
** @argrule * pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @argrule All pes [AjPList] AJAX List of Ensembl Prediction Exon objects
** @argrule AllbyPredictiontranscript pt [const EnsPPredictiontranscript]
** Ensembl Prediction Transcript
** @argrule AllbyPredictiontranscript pes [AjPList] AJAX List of
** Ensembl Prediction Exon objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice pes [AjPList] AJAX List of
** Ensembl Prediction Exon objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Ppe [EnsPPredictionexon*] Ensembl Prediction Exon
** address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensPredictionexonadaptorFetchAllbyPredictiontranscript ***************
**
** Fetch all Ensembl Prediction Exon objects via an
** Ensembl Prediction Transcript.
**
** The caller is responsible for deleting the Ensembl Prediction Exon objects
** before deleting the AJAX List.
**
** @param [u] pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] pes [AjPList] AJAX List of Ensembl Prediction Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonadaptorFetchAllbyPredictiontranscript(
    EnsPPredictionexonadaptor pea,
    const EnsPPredictiontranscript pt,
    AjPList pes)
{
    AjIList iter = NULL;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPPredictionexon pe = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature tfeature = NULL;

    EnsPSlice eslice = NULL;
    EnsPSlice tslice = NULL;

    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("ensPredictionexonadaptorFetchAllbyPredictiontranscript"))
    {
        ajDebug("ensPredictionexonadaptorFetchAllbyPredictiontranscript\n"
                "  pea %p\n"
                "  pt %p\n"
                "  pes %p\n",
                pea,
                pt,
                pes);

        ensPredictiontranscriptTrace(pt, 1);
    }

    if(!pea)
        return ajFalse;

    if(!pt)
        return ajFalse;

    if(!pes)
        return ajFalse;

    tfeature = ensPredictiontranscriptGetFeature(pt);

    tslice = ensFeatureGetSlice(tfeature);

    if(!tslice)
    {
        ajDebug("ensPredictionexonadaptorFetchAllbyPredictiontranscript "
                "cannot fetch Ensembl Prediction Exon objects for an "
                "Ensembl Prediction Transcript without an Ensembl Slice.\n");

        return ajFalse;
    }

    dba = ensFeatureadaptorGetDatabaseadaptor(pea);

    sa = ensRegistryGetSliceadaptor(dba);

    /*
    ** Get a Slice that spans just this Prediction Transcript to place
    ** Ensembl Prediction Exon objects on.
    */

    ensSliceadaptorFetchByFeature(sa, tfeature, 0, &eslice);

    constraint = ajFmtStr("prediction_exon.prediction_transcript_id = %u",
                          ensPredictiontranscriptGetIdentifier(pt));

    ensFeatureadaptorFetchAllbySlice(pea,
                                     eslice,
                                     constraint,
                                     (const AjPStr) NULL,
                                     pes);

    /* Remap Exon coordinates if neccessary. */

    if(!ensSliceMatch(eslice, tslice))
    {
        iter = ajListIterNew(pes);

        while(!ajListIterDone(iter))
        {
            pe = (EnsPPredictionexon) ajListIterGet(iter);

            efeature = ensFeatureTransfer(pe->Feature, tslice);

            ensPredictionexonSetFeature(pe, efeature);

            ensFeatureDel(&efeature);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&constraint);

    ensSliceDel(&eslice);

    return ajTrue;
}




/* @datasection [EnsPPredictiontranscript] Ensembl Prediction Transcript ******
**
** @nam2rule Predictiontranscript Functions for manipulating
** Ensembl Prediction Transcript objects
**
** @cc Bio::EnsEMBL::PredictionTranscript
** @cc CVS Revision: 1.49
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Transcript by pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Transcript. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy pt [const EnsPPredictiontranscript]
** Ensembl Prediction Transcript
** @argrule Ini pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini label [AjPStr] Display label
** @argrule Ref pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @valrule * [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensPredictiontranscriptNewCpy ****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptNewCpy(
    const EnsPPredictiontranscript pt)
{
    AjIList iter = NULL;

    EnsPPredictionexon pe = NULL;

    EnsPPredictiontranscript pthis = NULL;

    if(!pt)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = pt->Identifier;

    pthis->Adaptor = pt->Adaptor;

    pthis->Feature = ensFeatureNewRef(pt->Feature);

    if(pt->Displaylabel)
        pthis->Displaylabel = ajStrNewRef(pt->Displaylabel);

    /* Copy the AJAX List of Ensembl Prediction Exon objects. */

    if(pt->Predictionexons && ajListGetLength(pt->Predictionexons))
    {
        pthis->Predictionexons = ajListNew();

        iter = ajListIterNew(pt->Predictionexons);

        while(!ajListIterDone(iter))
        {
            pe = (EnsPPredictionexon) ajListIterGet(iter);

            ajListPushAppend(pthis->Predictionexons,
                             (void*) ensPredictionexonNewRef(pe));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Predictionexons = NULL;

    return pthis;
}




/* @func ensPredictiontranscriptNewIni ****************************************
**
** Constructor for an Ensembl Prediction Transcript with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] pta [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                  Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::PredictionTranscript::new
** @param [u] label [AjPStr] Display label
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptNewIni(
    EnsPPredictiontranscriptadaptor pta,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr label)
{
    EnsPPredictiontranscript pt = NULL;

    if(!feature)
        return NULL;

    AJNEW0(pt);

    pt->Use = 1;

    pt->Identifier = identifier;

    pt->Adaptor = pta;

    pt->Feature = ensFeatureNewRef(feature);

    if(label)
        pt->Displaylabel = ajStrNewRef(label);

    return pt;
}




/* @func ensPredictiontranscriptNewRef ****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptNewRef(
    EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;

    pt->Use++;

    return pt;
}




/* @funcstatic predictiontranscriptNewCpyFeatures *****************************
**
** Returns a new copy of an Ensembl Prediction Transcript, but in addition to
** the shallow copy provided by ensPredictiontranscriptNewCpy, also copies all
** Ensembl Prediction Transcript-internal Ensembl Objects based on the
** Ensembl Feature class. This is useful in preparation of
** ensPredictiontranscriptTransform and ensPredictiontranscriptTransfer, which
** return an independent Ensembl Prediction Transcript object and therefore,
** require independent mapping of all internal Feature objects to the new
** Ensembl Coordinate System or Ensembl Slice.
**
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

static EnsPPredictiontranscript predictiontranscriptNewCpyFeatures(
    EnsPPredictiontranscript pt)
{
    AjIList iter = NULL;

    EnsPPredictionexon newpe = NULL;
    EnsPPredictionexon oldpe = NULL;

    EnsPPredictiontranscript newpt = NULL;

    if(!pt)
        return NULL;

    newpt = ensPredictiontranscriptNewCpy(pt);

    if(!newpt)
        return NULL;

    /* Copy the AJAX List of Ensembl Prediction Exon objects. */

    if(newpt->Predictionexons)
    {
        iter = ajListIterNew(newpt->Predictionexons);

        while(!ajListIterDone(iter))
        {
            oldpe = (EnsPPredictionexon) ajListIterGet(iter);

            ajListIterRemove(iter);

            newpe = ensPredictionexonNewCpy(oldpe);

            ajListIterInsert(iter, (void*) newpe);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensPredictionexonDel(&oldpe);
        }

        ajListIterDel(&iter);
    }

    return newpt;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Del Destroy (free) an Ensembl Prediction Transcript object
**
** @argrule * Ppt [EnsPPredictiontranscript*] Ensembl Prediction Transcript
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictiontranscriptDel *******************************************
**
** Default destructor for an Ensembl Prediction Transcript.
**
** @param [d] Ppt [EnsPPredictiontranscript*] Ensembl Prediction Transcript
**                                            object address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictiontranscriptDel(
    EnsPPredictiontranscript* Ppt)
{
    EnsPPredictionexon pe = NULL;

    EnsPPredictiontranscript pthis = NULL;

    if(!Ppt)
        return;

    if(!*Ppt)
        return;

    pthis = *Ppt;

    pthis->Use--;

    if(pthis->Use)
    {
        *Ppt = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->Displaylabel);

    /* Clear and delete the AJAX List of Ensembl Prediction Exon objects. */

    while(ajListPop(pthis->Predictionexons, (void**) &pe))
        ensPredictionexonDel(&pe);

    ajListFree(&pthis->Predictionexons);

    AJFREE(pthis);

    *Ppt = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Get Return Prediction Transcript attribute(s)
** @nam4rule Adaptor Return the Ensembl Prediction Transcript Adaptor
** @nam4rule Displaylabel Return the display label
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
**
** @argrule * pt [const EnsPPredictiontranscript] Prediction Transcript
**
** @valrule Adaptor [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor or NULL
** @valrule Displaylabel [AjPStr] Display label or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensPredictiontranscriptGetAdaptor ************************************
**
** Get the Ensembl Prediction Transcript Adaptor element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictiontranscriptadaptor ensPredictiontranscriptGetAdaptor(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;

    return pt->Adaptor;
}




/* @func ensPredictiontranscriptGetDisplaylabel *******************************
**
** Get the display label element of an Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::PredictionTranscript::display_label
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [AjPStr] Display label or NULL
** @@
******************************************************************************/

AjPStr ensPredictiontranscriptGetDisplaylabel(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;

    return pt->Displaylabel;
}




/* @func ensPredictiontranscriptGetFeature ************************************
**
** Get the Ensembl Feature element of an Ensembl Prediction Transcript.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensPredictiontranscriptGetFeature(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;

    return pt->Feature;
}




/* @func ensPredictiontranscriptGetIdentifier *********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensPredictiontranscriptGetIdentifier(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return 0;

    return pt->Identifier;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Prediction Transcript object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Load Return Ensembl Prediction Transcript attribute(s)
**                loaded on demand
** @nam4rule Predictionexons Return all Ensembl Prediction Exon objects
**
** @argrule * pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @valrule Predictionexons [const AjPList] AJAX List of
** Ensembl Prediction Exon objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensPredictiontranscriptLoadPredictionexons ***************************
**
** Load all Ensembl Prediction Exon objects of an
** Ensembl Prediction Transcript.
**
** This is not a simple accessor function, since it will attempt fetching the
** Prediction Exon objects from the Ensembl Core database associated with the
** Prediction Transcript Adaptor.
**
** @cc Bio::EnsEMBL::PredictionTranscript::get_all_Exons
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [const AjPList] AJAX List of Ensembl Prediction Exon objects
** @@
******************************************************************************/

const AjPList ensPredictiontranscriptLoadPredictionexons(
    EnsPPredictiontranscript pt)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPPredictionexonadaptor pea = NULL;

    if(!pt)
        return NULL;

    if(pt->Predictionexons)
        return pt->Predictionexons;

    if(!pt->Adaptor)
    {
        ajDebug("ensPredictiontranscriptGetPredictionexons cannot fetch "
                "Ensembl Prediction Exon objects for an "
                "Ensembl Prediction Transcript without an "
                "Ensembl Prediction Transcript Adaptor.\n");

        return NULL;
    }

    dba = ensFeatureadaptorGetDatabaseadaptor(pt->Adaptor);

    pea = ensRegistryGetPredictionexonadaptor(dba);

    pt->Predictionexons = ajListNew();

    ensPredictionexonadaptorFetchAllbyPredictiontranscript(
        pea,
        pt,
        pt->Predictionexons);

    return pt->Predictionexons;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Set Set one element of a Prediction Transcript
** @nam4rule Adaptor Set the Ensembl Prediction Transcript Adaptor
** @nam4rule Displaylabel Set the display label
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @argrule Adaptor pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
** @argrule Displaylabel label [AjPStr] Display label
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensPredictiontranscriptSetAdaptor ************************************
**
** Set the Ensembl Prediction Transcript Adaptor element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetAdaptor(
    EnsPPredictiontranscript pt,
    EnsPPredictiontranscriptadaptor pta)
{
    if(!pt)
        return ajFalse;

    pt->Adaptor = pta;

    return ajTrue;
}




/* @func ensPredictiontranscriptSetDisplaylabel *******************************
**
** Set the display label element of an Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::PredictionTranscript::display_label
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] label [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetDisplaylabel(
    EnsPPredictiontranscript pt,
    AjPStr label)
{
    if(!pt)
        return ajFalse;

    ajStrDel(&pt->Displaylabel);

    pt->Displaylabel = ajStrNewRef(label);

    return ajTrue;
}




/* @func ensPredictiontranscriptSetFeature ************************************
**
** Set the Ensembl Feature element of an Ensembl Prediction Transcript.
**
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetFeature(
    EnsPPredictiontranscript pt,
    EnsPFeature feature)
{
    AjIList iter = NULL;

    EnsPPredictionexon oldpe = NULL;
    EnsPPredictionexon newpe = NULL;

    EnsPSlice slice = NULL;

    if(!pt)
        return ajFalse;

    ensFeatureDel(&pt->Feature);

    pt->Feature = ensFeatureNewRef(feature);

    slice = ensFeatureGetSlice(pt->Feature);

    /*
    ** Transfer Ensembl Prediction Exon objects onto the new Ensembl Feature
    ** Slice.
    */

    if(pt->Predictionexons)
    {
        iter = ajListIterNew(pt->Predictionexons);

        while(!ajListIterDone(iter))
        {
            oldpe = (EnsPPredictionexon) ajListIterGet(iter);

            ajListIterRemove(iter);

            newpe = ensPredictionexonTransfer(oldpe, slice);

            if(!newpe)
            {
                ajDebug("ensPredictiontranscriptSetFeature could not transfer "
                        "Prediction Exon onto new Ensembl Feature Slice.");

                ensPredictionexonTrace(oldpe, 1);
            }

            ajListIterInsert(iter, (void*) newpe);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensPredictionexonDel(&oldpe);
        }

        ajListIterDel(&iter);
    }

    return ajTrue;
}




/* @func ensPredictiontranscriptSetIdentifier *********************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetIdentifier(
    EnsPPredictiontranscript pt,
    ajuint identifier)
{
    if(!pt)
        return ajFalse;

    pt->Identifier = identifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Trace Report Ensembl Prediction Transcript elements to debug file
**
** @argrule Trace pt [const EnsPPredictiontranscript] Ensembl Prediction
**                                                    Transcript
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictiontranscriptTrace *****************************************
**
** Trace an Ensembl Prediction Transcript.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptTrace(
    const EnsPPredictiontranscript pt,
    ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPPredictionexon pe = NULL;

    if(!pt)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensPredictiontranscriptTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Displaylabel '%S'\n"
            "%S  Predictionexons %p\n",
            indent, pt,
            indent, pt->Use,
            indent, pt->Identifier,
            indent, pt->Adaptor,
            indent, pt->Feature,
            indent, pt->Displaylabel,
            indent, pt->Predictionexons);

    ensFeatureTrace(pt->Feature, level + 1);

    /* Trace the AJAX List of Ensembl Prediction Exon objects. */

    if(pt->Predictionexons)
    {
        ajDebug("%S    AJAX List %p of Ensembl Prediction Exon objects\n",
                indent, pt->Predictionexons);

        iter = ajListIterNewread(pt->Predictionexons);

        while(!ajListIterDone(iter))
        {
            pe = (EnsPPredictionexon) ajListIterGet(iter);

            ensPredictionexonTrace(pe, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Calculate Calculate Ensembl Prediction Transcript values
** @nam4rule Length  Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
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
** @argrule Memsize pt [const EnsPPredictiontranscript]
** Ensembl Prediction Transcript
** @argrule Slice pt [const EnsPPredictiontranscript]
** Ensembl Prediction Transcript
** @argrule TranscriptCodingEnd pt [EnsPPredictiontranscript]
** Ensembl Prediction Transcript
** @argrule TranscriptCodingStart pt [const EnsPPredictiontranscript]
** Ensembl Prediction Transcript
**
** @valrule Length [ajuint] Transcript (cDNA) length or 0
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule SliceCodingEnd [ajint] End coordinate or 0
** @valrule SliceCodingStart [ajint] Start coordinate or 0
** @valrule TranscriptCodingEnd [ajuint] End coordinate or 0
** @valrule TranscriptCodingStart [ajuint] Start coordinate or 0
** @valrule TranscriptEnd [ajuint] End coordinate or 0
** @valrule TranscriptStart [ajuint] Start coordinate or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictiontranscriptCalculateMemsize ******************************
**
** Calculate the memory size in bytes of an Ensembl Prediction Transcript.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensPredictiontranscriptCalculateMemsize(
    const EnsPPredictiontranscript pt)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPPredictionexon pe = NULL;

    if(!pt)
        return 0;

    size += sizeof (EnsOPredictiontranscript);

    size += ensFeatureCalculateMemsize(pt->Feature);

    if(pt->Displaylabel)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(pt->Displaylabel);
    }

    /* Summarise the AJAX List of Ensembl Prediction Exon objects. */

    if(pt->Predictionexons)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(pt->Predictionexons);

        while(!ajListIterDone(iter))
        {
            pe = (EnsPPredictionexon) ajListIterGet(iter);

            size += ensPredictionexonCalculateMemsize(pe);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @func ensPredictiontranscriptCalculateSliceCodingEnd ***********************
**
** Calculate the end position of the coding region in Slice coordinates.
**
** Calculates the end of the coding region of this Prediction Transcript in
** Slice coordinates. For Prediction Transcript objects this is always the same
** as the end since no UTRs are stored. By convention, the coding region end,
** returned by ensPredictiontranscriptCalculateSliceCodingEnd is always higher
** than the value returned by the
** ensPredictiontranscriptCalculateSliceCodingStart function. The value
** returned by this function is NOT the biological coding end, since on the
** reverse strand the biological coding end would be the lower genomic value.
**
** @cc Bio::EnsEMBL::PredictionTranscript::coding_region_end
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajint] Coding region end in Slice coordinates
** @@
******************************************************************************/

ajint ensPredictiontranscriptCalculateSliceCodingEnd(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return 0;

    return ensFeatureGetEnd(pt->Feature);
}




/* @func ensPredictiontranscriptCalculateSliceCodingStart *********************
**
** Calculate the start position of the coding region in Slice coordinates.
**
** Calculates the start of the coding region of this Prediction Transcript in
** Slice coordinates. For Prediction Transcript objects this is always the
** start of the transcript (i.e. there is no UTR). By convention, the coding
** region start returned by ensPredictiontranscriptCalculateSliceCodingStart
** is always lower than the value returned by the
** ensPredictiontranscriptCalculateSliceCodingEnd function.
** The value returned by this function is NOT the biological coding start,
** since on the reverse strand the biological coding start would be the higher
** genomic value.
**
** @cc Bio::EnsEMBL::PredictionTranscript::coding_region_start
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajint] Coding region start in Slice coordinates
** @@
******************************************************************************/

ajint ensPredictiontranscriptCalculateSliceCodingStart(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return 0;

    return ensFeatureGetStart(pt->Feature);
}




/* @func ensPredictiontranscriptCalculateTranscriptCodingEnd ******************
**
** Calculate the end position of the coding region in Prediction Transcript
** coordinates.
**
** @cc Bio::EnsEMBL::PredictionTranscript::cdna_coding_end
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajuint] Coding region end in Transcript coordinates or 0
** @@
******************************************************************************/

ajuint ensPredictiontranscriptCalculateTranscriptCodingEnd(
    EnsPPredictiontranscript pt)
{
    ajuint end = 0;

    AjIList iter = NULL;

    const AjPList pes = NULL;

    EnsPPredictionexon pe = NULL;

    EnsPFeature feature = NULL;

    if(!pt)
        return 0;

    pes = ensPredictiontranscriptLoadPredictionexons(pt);

    iter = ajListIterNewread(pes);

    while(!ajListIterDone(iter))
    {
        pe = (EnsPPredictionexon) ajListIterGet(iter);

        /* Add the entire length of this Prediction Exon. */

        feature = ensPredictionexonGetFeature(pe);

        end += ensFeatureCalculateLength(feature);
    }

    ajListIterDel(&iter);

    return end;
}




/* @func ensPredictiontranscriptCalculateTranscriptCodingStart ****************
**
** Calculate the start position of the coding region in Prediction Transcript
** coordinates.
**
** @cc Bio::EnsEMBL::PredictionTranscript::cdna_coding_start
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajuint] Coding region start in Transcript coordinates or 0
** @@
******************************************************************************/

ajuint ensPredictiontranscriptCalculateTranscriptCodingStart(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return 0;

    return 1;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Fetch Fetch Ensembl Prediction Transcript values
** @nam4rule All Fetch all objects
** @nam4rule Sequence Fetch the sequence
** @nam5rule Coding      Fetch the coding sequence
** @nam5rule Transcript  Fetch the Ensembl Transcript (cDNA) sequence
** @nam6rule Seq         Fetch as AJAX Sequence object
** @nam6rule Str         Fetch as AJAX String object
** @nam5rule Translation Fetch the Ensembl Translation sequence
** @nam6rule Seq         Fetch as AJAX Sequence object
** @nam6rule Str         Fetch as AJAX String object
**
** @argrule * pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence address
** @argrule Str Psequence [AjPStr*] AJAX String address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictiontranscriptFetchSequenceTranscriptSeq ********************
**
** Fetch the sequence of an Ensembl Prediction Transcript as AJAX Sequence.
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::PredictionTranscript:spliced_seq
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptFetchSequenceTranscriptSeq(
    EnsPPredictiontranscript pt,
    AjPSeq* Psequence)
{
    AjPStr sequence = NULL;

    if(!pt)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    ensPredictiontranscriptFetchSequenceTranscriptStr(pt, &sequence);

    *Psequence = ajSeqNewNameS(sequence, pt->Displaylabel);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensPredictiontranscriptFetchSequenceTranscriptStr ********************
**
** Fetch the spliced sequence of an Ensembl Prediction Transcript as
** AJAX String.
**
** The sequence of all Ensembl Prediction Exon objects is concatenated.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::PredictionTranscript::spliced_seq
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptFetchSequenceTranscriptStr(
    EnsPPredictiontranscript pt,
    AjPStr* Psequence)
{
    AjIList iter      = NULL;
    const AjPList pes = NULL;

    AjPStr peseq = NULL;

    EnsPPredictionexon pe = NULL;

    EnsPFeature feature = NULL;

    if(!pt)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    pes = ensPredictiontranscriptLoadPredictionexons(pt);

    iter = ajListIterNewread(pes);

    while(!ajListIterDone(iter))
    {
        pe = (EnsPPredictionexon) ajListIterGet(iter);

        ensPredictionexonFetchSequenceStr(pe, &peseq);

        if(peseq && ajStrGetLen(peseq))
            ajStrAppendS(Psequence, peseq);
        else
        {
            ajDebug("ensPredictiontranscriptFetchSequenceTranscriptStr could "
                    "not get sequence for Prediction Exon. The "
                    "Prediction Transcript sequence may not be correct.\n");

            feature = ensPredictionexonGetFeature(pe);

            ajStrAppendCountK(Psequence,
                              'N',
                              ensFeatureCalculateLength(feature));
        }

        ajStrDel(&peseq);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensPredictiontranscriptFetchSequenceTranslationSeq *******************
**
** Fetch the sequence of the Ensembl Prediction Translation of an
** Ensembl Prediction Transcript as AJAX Sequence.
**
** The sequence is based on ensPredictiontranscriptFetchSequenceTranslationStr.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::PredictionTranscript::translate
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptFetchSequenceTranslationSeq(
    EnsPPredictiontranscript pt,
    AjPSeq* Psequence)
{
    AjPStr sequence = NULL;

    if(!pt)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    sequence = ajStrNew();

    ensPredictiontranscriptFetchSequenceTranslationStr(pt, &sequence);

    *Psequence = ajSeqNewNameS(sequence, pt->Displaylabel);

    ajSeqSetProt(*Psequence);

    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensPredictiontranscriptFetchSequenceTranslationStr *******************
**
** Fetch the sequence of the Ensembl Prediction Translation of an
** Ensembl Prediction Transcript as AJAX String.
**
** The sequence is based on ensPredictiontranscriptFetchSequenceTranscriptStr.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::PredictionTranscript::translate
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [wP] Psequence [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptFetchSequenceTranslationStr(
    EnsPPredictiontranscript pt,
    AjPStr* Psequence)
{
    AjPStr cdna = NULL;

    const AjPTrn atranslation = NULL;

    EnsPSlice slice = NULL;

    if(ajDebugTest("ensPredictiontranscriptFetchSequenceTranslationStr"))
        ajDebug("ensPredictiontranscriptFetchSequenceTranslationStr\n"
                "  pt %p\n"
                "  Psequence %p\n",
                pt,
                Psequence);

    if(!pt)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    /*
    ** NOTE: Ensembl Prediction Transcript objects have no untranslated regions
    ** (UTRs), so that it is sufficient to fetch the Prediction Transcript
    ** sequence, rather than the translatable sequence, as in the case of an
    ** Ensembl Transcript.
    */

    cdna = ajStrNew();

    ensPredictiontranscriptFetchSequenceTranscriptStr(pt, &cdna);

    if(ajStrGetLen(cdna) < 1)
        return ajTrue;

    slice = ensFeatureGetSlice(pt->Feature);

    atranslation = ensSliceGetTranslation(slice);

    ajTrnSeqS(atranslation, cdna, Psequence);

    ajStrDel(&cdna);

    /*
    ** Remove the final stop codon from the mRNA if it is present, so that the
    ** resulting peptides do not end with a '*'. If a terminal stop codon is
    ** desired, call ensTranscriptFetchSequenceCodingStr and translate it
    ** directly.
    ** NOTE: This test is simpler and hopefully more efficient than the one
    ** in the Perl API, which tests for a termination codon in a
    ** codon table-specifc manner and removes the last triplet from the cDNA.
    */

    if(ajStrGetCharLast(*Psequence) == '*')
        ajStrCutEnd(Psequence, 1);

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Prediction Transcript objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPPredictiontranscript]
**
** @nam3rule Transfer Transfer an Ensembl Prediction Transcript
** @nam3rule Transform Transform an Ensembl Prediction Transcript
**
** @argrule * pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
**
** @valrule * [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictiontranscriptTransfer **************************************
**
** Transfer an Ensembl Prediction Transcript onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::PredictionTranscript::transfer
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptTransfer(
    EnsPPredictiontranscript pt,
    EnsPSlice slice)
{
    EnsPFeature newptf = NULL;

    EnsPPredictiontranscript newpt = NULL;

    if(ajDebugTest("ensPredictiontranscriptTransfer"))
        ajDebug("ensPredictiontranscriptTransfer\n"
                "  pt %p\n"
                "  slice %p\n",
                pt,
                slice);

    if(!pt)
        return NULL;

    if(!slice)
        return NULL;

    if(!pt->Feature)
        ajFatal("ensPredictiontranscriptTransfer cannot transfer an "
                "Ensembl Prediction Transcript "
                "without an Ensembl Feature.\n");

    newptf = ensFeatureTransfer(pt->Feature, slice);

    if(!newptf)
        return NULL;

    newpt = predictiontranscriptNewCpyFeatures(pt);

    ensPredictiontranscriptSetFeature(newpt, newptf);

    ensFeatureDel(&newptf);

    return newpt;
}




/* @func ensPredictiontranscriptTransform *************************************
**
** Transform an Ensembl Prediction Transcript into another
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::PredictionTranscript::transform
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptTransform(
    EnsPPredictiontranscript pt,
    const AjPStr csname,
    const AjPStr csversion)
{
    EnsPFeature newptf = NULL;

    EnsPPredictiontranscript newpt = NULL;

    if(ajDebugTest("ensPredictiontranscriptTransform"))
        ajDebug("ensPredictiontranscriptTransform\n"
                "  pt %p\n"
                "  csname '%S'\n"
                "  csversion '%S'\n",
                pt,
                csname,
                csversion);

    if(!pt)
        return NULL;

    if(!csname)
        return NULL;

    if(!pt->Feature)
        ajFatal("ensPredictiontranscriptTransfer cannot transfer an "
                "Ensembl Prediction Transcript "
                "without an Ensembl Feature.\n");

    newptf = ensFeatureTransform(pt->Feature,
                                 csname,
                                 csversion,
                                 (EnsPSlice) NULL);

    if(!newptf)
        return NULL;

    newpt = predictiontranscriptNewCpyFeatures(pt);

    ensPredictiontranscriptSetFeature(newpt, newptf);

    ensFeatureDel(&newptf);

    return newpt;
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
** @nam3rule Predictiontranscript Functions for manipulating AJAX List objects
** of Ensembl Prediction Transcript objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending pts [AjPList] AJAX List of
** Ensembl Prediction Transcript objects
** @argrule Descending pts [AjPList] AJAX List of
** Ensembl Prediction Transcript objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listPredictiontranscriptCompareStartAscending ******************
**
** AJAX List of Ensembl Prediction Transcript objects comparison function to
** sort by Ensembl Feature start element in ascending order.
**
** @param [r] P1 [const void*] Ensembl Prediction Transcript address 1
** @param [r] P2 [const void*] Ensembl Prediction Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listPredictiontranscriptCompareStartAscending(const void* P1,
                                                         const void* P2)
{
    const EnsPPredictiontranscript pt1 = NULL;
    const EnsPPredictiontranscript pt2 = NULL;

    pt1 = *(EnsPPredictiontranscript const*) P1;
    pt2 = *(EnsPPredictiontranscript const*) P2;

    if(ajDebugTest("listPredictiontranscriptCompareStartAscending"))
        ajDebug("listPredictiontranscriptCompareStartAscending\n"
                "  pt1 %p\n"
                "  pt2 %p\n",
                pt1,
                pt2);

    /* Sort empty values towards the end of the AJAX List. */

    if(pt1 && (!pt2))
        return -1;

    if((!pt1) && (!pt2))
        return 0;

    if((!pt1) && pt2)
        return +1;

    return ensFeatureCompareStartAscending(pt1->Feature, pt2->Feature);
}




/* @func ensListPredictiontranscriptSortStartAscending ************************
**
** Sort Ensembl Prediction Transcript objects by their Ensembl Feature start
** coordinate in ascending order.
**
** @param [u] pts [AjPList] AJAX List of Ensembl Prediction Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListPredictiontranscriptSortStartAscending(AjPList pts)
{
    if(!pts)
        return ajFalse;

    ajListSort(pts, listPredictiontranscriptCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listPredictiontranscriptCompareStartDescending *****************
**
** AJAX List of Ensembl Prediction Transcript objects comparison function to
** sort by Ensembl Feature start element in descending order.
**
** @param [r] P1 [const void*] Ensembl Prediction Transcript address 1
** @param [r] P2 [const void*] Ensembl Prediction Transcript address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listPredictiontranscriptCompareStartDescending(const void* P1,
                                                          const void* P2)
{
    const EnsPPredictiontranscript pt1 = NULL;
    const EnsPPredictiontranscript pt2 = NULL;

    pt1 = *(EnsPPredictiontranscript const*) P1;
    pt2 = *(EnsPPredictiontranscript const*) P2;

    if(ajDebugTest("listPredictiontranscriptCompareStartDescending"))
        ajDebug("predictionCompareStartDescending\n"
                "  pt1 %p\n"
                "  pt2 %p\n",
                pt1,
                pt2);

    /* Sort empty values towards the end of the AJAX List. */

    if(pt1 && (!pt2))
        return -1;

    if((!pt1) && (!pt2))
        return 0;

    if((!pt1) && pt2)
        return +1;

    return ensFeatureCompareStartDescending(pt1->Feature, pt2->Feature);
}




/* @func ensListPredictiontranscriptSortStartDescending ***********************
**
** Sort Ensembl Prediction Transcript objects by their Ensembl Feature start
** coordinate in descending order.
**
** @param [u] pts [AjPList] AJAX List of Ensembl Prediction Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListPredictiontranscriptSortStartDescending(AjPList pts)
{
    if(!pts)
        return ajFalse;

    ajListSort(pts, listPredictiontranscriptCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPPredictiontranscriptadaptor] Ensembl Prediction Transcript
** Adaptor
**
** @nam2rule Predictiontranscriptadaptor Functions for manipulating
** Ensembl Prediction Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::PredictionTranscriptAdaptor
** @cc CVS Revision: 1.51
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic predictiontranscriptadaptorFetchAllbyStatement *****************
**
** Fetch all Ensembl Prediction Transcript objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] pts [AjPList] AJAX List of Ensembl Prediction Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool predictiontranscriptadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList pts)
{
    ajuint identifier = 0;
    ajuint analysisid = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr label = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPPredictiontranscript pt         = NULL;
    EnsPPredictiontranscriptadaptor pta = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("predictiontranscriptadaptorFetchAllbyStatement"))
        ajDebug("predictiontranscriptadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  pts %p\n",
                dba,
                statement,
                am,
                slice,
                pts);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!pts)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    pta = ensRegistryGetPredictiontranscriptadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        srstrand   = 0;
        analysisid = 0;
        label      = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &label);

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
            ajFatal("predictiontranscriptadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("predictiontranscriptadaptorFetchAllbyStatement got a "
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

                ajStrDel(&label);

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
                ajFatal("transcriptAdaptorFetchAllbyStatement got a "
                        "Slice, which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceCalculateLength(slice), INT_MAX);

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
            ** Throw away Ensembl Feature objects off the end of the requested
            ** Slice or on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Next feature but destroy first! */

                ajStrDel(&label);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        /* Finally, create a new Ensembl Prediction Transcript. */

        feature = ensFeatureNewIniS(analysis,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

        pt = ensPredictiontranscriptNewIni(pta,
                                           identifier,
                                           feature,
                                           label);

        ajListPushAppend(pts, (void*) pt);

        ensFeatureDel(&feature);

        ajStrDel(&label);

        ensAnalysisDel(&analysis);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic predictiontranscriptadaptorCacheReference **********************
**
** Wrapper function to reference an Ensembl Prediction Transcript
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Prediction Transcript
**
** @return [void*] Ensembl Transcript or NULL
** @@
******************************************************************************/

static void* predictiontranscriptadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensPredictiontranscriptNewRef((EnsPPredictiontranscript)
                                                 value);
}




/* @funcstatic predictiontranscriptadaptorCacheDelete *************************
**
** Wrapper function to delete an Ensembl Prediction Transcript
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Prediction Transcript address
**
** @return [void]
** @@
******************************************************************************/

static void predictiontranscriptadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensPredictiontranscriptDel((EnsPPredictiontranscript*) value);

    return;
}




/* @funcstatic predictiontranscriptadaptorCacheSize ***************************
**
** Wrapper function to determine the memory size of an
** Ensembl Prediction Transcript via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Prediction Transcript
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t predictiontranscriptadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensPredictiontranscriptCalculateMemsize((const EnsPPredictiontranscript)
                                                   value);
}




/* @funcstatic predictiontranscriptadaptorGetFeature **************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Prediction Transcript from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Prediction Transcript
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature predictiontranscriptadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensPredictiontranscriptGetFeature((const EnsPPredictiontranscript)
                                             value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Transcript Adaptor by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Transcript Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictiontranscriptadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                              Transcript Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensPredictiontranscriptadaptorNew ************************************
**
** Default constructor for an Ensembl Prediction Transcript Adaptor.
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
** @see ensRegistryGetPredictiontranscriptadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                           Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictiontranscriptadaptor ensPredictiontranscriptadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        predictiontranscriptadaptorTables,
        predictiontranscriptadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        predictiontranscriptadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        predictiontranscriptadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        predictiontranscriptadaptorCacheDelete,
        predictiontranscriptadaptorCacheSize,
        predictiontranscriptadaptorGetFeature,
        "Predictiontranscript");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Prediction Transcript Adaptor object.
**
** @fdata [EnsPPredictiontranscriptadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Prediction Transcript Adaptor object
**
** @argrule * Ppta [EnsPPredictiontranscriptadaptor*]
** Ensembl Prediction Transcript Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictiontranscriptadaptorDel ************************************
**
** Default destructor for an Ensembl Prediction Transcript Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Ppta [EnsPPredictiontranscriptadaptor*] Ensembl Prediction
**                                                    Transcript Adaptor
**                                                    address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictiontranscriptadaptorDel(
    EnsPPredictiontranscriptadaptor* Ppta)
{
    if(!Ppta)
        return;

    ensFeatureadaptorDel(Ppta);

    *Ppta = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Prediction Transcript Adaptor object.
**
** @fdata [EnsPPredictiontranscriptadaptor]
**
** @nam3rule Get Return Ensembl Prediction Transcript Adaptor attribute(s)
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
**
** @valrule Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensPredictiontranscriptadaptorGetDatabaseadaptor *********************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Prediction Transcript Adaptor.
**
** @param [u] pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensPredictiontranscriptadaptorGetDatabaseadaptor(
    EnsPPredictiontranscriptadaptor pta)
{
    if(!pta)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(pta);
}




/* @func ensPredictiontranscriptadaptorGetFeatureadaptor **********************
**
** Get the Ensembl Feature Adaptor element of an
** Ensembl Prediction Transcript Adaptor.
**
** @param [u] pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPFeatureadaptor ensPredictiontranscriptadaptorGetFeatureadaptor(
    EnsPPredictiontranscriptadaptor pta)
{
    if(!pta)
        return NULL;

    return pta;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Prediction Transcript objects from an
** Ensembl SQL database.
**
** @fdata [EnsPPredictiontranscriptadaptor]
**
** @nam3rule Fetch Fetch Ensembl Prediction Transcript object(s)
** @nam4rule All   Fetch all Ensembl Prediction Transcript objects
** @nam4rule Allby Fetch all Ensembl Prediction Transcript objects
**                 matching a criterion
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable Ensembl Exon identifier
** @nam4rule By Fetch one Ensembl Exon object matching a criterion
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by stable Ensembl Exon identifier
**
** @argrule * pta [EnsPPredictiontranscriptadaptor]
** Ensembl Prediction Transcript Adaptor
** @argrule All pts [AjPList] AJAX List of
** Ensembl Prediction Transcript objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice loadexons [AjBool] Load Ensembl Prediction Exon objects
** @argrule AllbySlice pts [AjPList] AJAX List of Ensembl Prediction Transcript
** objects
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule AllbyStableidentifier transcripts [AjPList] AJAX List of Ensembl
**                                                      Transcript objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByDisplaylabel Ppt [EnsPPredictiontranscript*]
** Ensembl Prediction Transcript address
** @argrule ByExonidentifier identifier [ajuint] Ensembl Exon identifier
** @argrule ByExonidentifier Ppt [EnsPPredictiontranscript*]
** Ensembl Pediction Transcript address
** @argrule ByExonstableidentifier stableid [const AjPStr] Ensembl Exon
**                                                         stable identifier
** @argrule ByExonstableidentifier Ppt [EnsPPredictiontranscript*]
** Ensembl Prediction Transcript address
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Ppt [EnsPPredictiontranscript*]
** Ensembl Prediction Transcript address
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** Ensembl Prediction Transcript address
** @argrule ByStableidentifier Ppt [EnsPPredictiontranscript*]
** Ensembl Prediction Transcript address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensPredictiontranscriptadaptorFetchByIdentifier **********************
**
** Fetch an Ensembl Prediction Transcript via its SQL database-internal
** identifier.
** The caller is responsible for deleting the Ensembl Prediction Transcript.
**
** @param [u] pta [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                  Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ppt [EnsPPredictiontranscript*] Ensembl Prediction
**                                             Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptadaptorFetchByIdentifier(
    EnsPPredictiontranscriptadaptor pta,
    ajuint identifier,
    EnsPPredictiontranscript* Ppt)
{
    EnsPBaseadaptor ba = NULL;

    if(!pta)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Ppt)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(pta);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void**) Ppt);
}




/* @func ensPredictiontranscriptadaptorFetchByStableidentifier ****************
**
** Fetch an Ensembl Prediction Transcript via its stable identifier and
** version. This method is called FetchByStableidentifier for polymorphism with
** the Ensembl Transcript Adaptor. Prediction Transcript display labels are
** not necessarily stable in that the same identifier may be reused for a
** completely different Prediction Transcript in a subsequent database release.
**
** The caller is responsible for deleting the Ensembl Prediction Transcript.
**
** @param [u] pta [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                  Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [wP] Ppt [EnsPPredictiontranscript*] Ensembl Prediction
**                                             Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptadaptorFetchByStableidentifier(
    EnsPPredictiontranscriptadaptor pta,
    const AjPStr stableid,
    EnsPPredictiontranscript* Ppt)
{
    char* txtstableid = NULL;

    AjPList pts = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPPredictiontranscript pt = NULL;

    if(!pta)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!Ppt)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(pta);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint =
        ajFmtStr("prediction_transcript.display_label = '%s'", txtstableid);

    ajCharDel(&txtstableid);

    pts = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       pts);

    if(ajListGetLength(pts) > 1)
        ajDebug("ensPredictiontranscriptadaptorFetchByStableidentifier got "
                "more than one Prediction Transcript for stable identifier "
                "'%S'.\n",
                stableid);

    ajListPop(pts, (void**) Ppt);

    while(ajListPop(pts, (void**) &pt))
        ensPredictiontranscriptDel(&pt);

    ajListFree(&pts);

    ajStrDel(&constraint);

    return ajTrue;
}

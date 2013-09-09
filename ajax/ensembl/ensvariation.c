/* @source ensvariation *******************************************************
**
** Ensembl Genetic Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.57 $
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

#include "ensgvbaseadaptor.h"
#include "ensgvdatabaseadaptor.h"
#include "ensgvvariation.h"
#include "ensvariation.h"
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




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic gvconsequenceKType ********************************************
**
** Array of Ensembl Genetic Variation consequence codes
**
** The special consequence type, SARA, that only applies to the effect of the
** Allele objects, not Variation objects and is equivalent to Same As Reference
** Allele, meaning that the Allele is the same as in reference sequence, so has
** no effect but it is not stored anywhere in the database and need no
** conversion at all when creating the Ensembl Variation Feature object,
** thus the absence in the array.
**
******************************************************************************/

const char *const gvconsequenceKType[] =
{
    "ESSENTIAL_SPLICE_SITE",  /* 1 <<  0 =       1 */
    "STOP_GAINED",            /* 1 <<  1 =       2 */
    "STOP_LOST",              /* 1 <<  2 =       4 */
    "COMPLEX_INDEL",          /* 1 <<  3 =       8 */
    "FRAMESHIFT_CODING",      /* 1 <<  4 =      16 */
    "NON_SYNONYMOUS_CODING",  /* 1 <<  5 =      32 */
    "SPLICE_SITE",            /* 1 <<  6 =      64 */
    "PARTIAL_CODON",          /* 1 <<  7 =     128 */
    "SYNONYMOUS_CODING",      /* 1 <<  8 =     256 */
    "REGULATORY_REGION",      /* 1 <<  9 =     512 */
    "WITHIN_MATURE_miRNA",    /* 1 << 10 =    1024 */
    "5PRIME_UTR",             /* 1 << 11 =    2048 */
    "3PRIME_UTR",             /* 1 << 12 =    2094 */
    "UTR",                    /* 1 << 13 =    4096 */
    "INTRONIC",               /* 1 << 14 =    8192 */
    "NMD_TRANSCRIPT",         /* 1 << 15 =   16384 */
    "WITHIN_NON_CODING_GENE", /* 1 << 16 =   32768 */
    "UPSTREAM",               /* 1 << 17 =   65536 */
    "DOWNSTREAM",             /* 1 << 18 =  131072 */
    "HGMD_MUTATION",          /* 1 << 19 =  262144 */
    "NO_CONSEQUENCE",         /* 1 << 20 =  524288 */
    "INTERGENIC",             /* 1 << 21 = 1048576 */
    "_",                      /* 1 << 22 = 2097152 */
    (const char *) NULL
};




/* @conststatic gvconsequenceKDescription *************************************
**
** Array of Ensembl Genetic Variation consequence descriptions
**
******************************************************************************/

const char *const gvconsequenceKDescription[] =
{
    "In the first 2 or the last 2 basepairs of an intron",
    "In coding sequence, resulting in the gain of a stop codon",
    "In coding sequence, resulting in the loss of a stop codon",
    "Insertion or deletion that spans an exon/intron or coding "
    "sequence/UTR border",
    "In coding sequence, resulting in a frameshift",
    "In coding sequence and results in an amino acid change in the encoded "
    "peptide sequence",
    "1-3 bps into an exon or 3-8 bps into an intron",
    "Located within the final, incomplete codon of a transcript whose "
    "end coordinate is unknown",
    "In coding sequence, not resulting in an amino acid change "
    "(silent mutation)",
    "In regulatory region annotated by Ensembl",
    "Located within a microRNA",
    "In 5 prime untranslated region",
    "In 3 prime untranslated region",
    "In intron",
    "Located within a transcript predicted to undergo nonsense-mediated decay",
    "Located within a gene that does not code for a protein",
    "Within 5 kb upstream of the 5 prime end of a transcript",
    "Within 5 kb downstream of the 3 prime end of a transcript",
    "Mutation from the HGMD database - consequence unknown",
    "More than 5 kb either upstream or downstream of a transcript",
    (const char *) NULL
};




/* @conststatic gvconsequenceKTranslation *************************************
**
** Array of consequence types that have an influence on the Translation of a
** Transcript.
**
******************************************************************************/

const char *const gvconsequenceKTranslation[] =
{
    "STOP_GAINED",
    "STOP_LOST",
    "COMPLEX_INDEL",
    "FRAMESHIFT_CODING",
    "NON_SYNONYMOUS_CODING",
    "PARTIAL_CODON",
    NULL
};




/* @conststatic gvtranscriptvariationadaptorKTablenames ***********************
**
** Array of Ensembl Genetic Variation Transcript Variation Adaptor
** SQL table names
**
******************************************************************************/

static const char *const gvtranscriptvariationadaptorKTablenames[] =
{
    "transcript_variation",
    "variation_feature",
    "failed_variation",
    "source",
    (const char *) NULL
};




/* @conststatic gvtranscriptvariationadaptorKColumnnames **********************
**
** Array of Ensembl Genetic Variation Transcript Variation Adaptor
** SQL column names
**
******************************************************************************/

static const char *const gvtranscriptvariationadaptorKColumnnames[] =
{
    "transcript_variation.transcript_variation_id",
    "transcript_variation.transcript_stable_id",
    "transcript_variation.variation_feature_id",
    "transcript_variation.cdna_start",
    "transcript_variation.cdna_end",
    "transcript_variation.cds_start",
    "transcript_variation.cds_end",
    "transcript_variation.translation_start",
    "transcript_variation.translation_end",
    "transcript_variation.peptide_allele_string",
    "transcript_variation.consequence_type",
    (const char *) NULL
};




/* @conststatic gvtranscriptvariationadaptorKLeftjoins ************************
**
** Array of Ensembl Genetic Variation Transcript Variation Adaptor
** SQL LEFT JOIN conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin gvtranscriptvariationadaptorKLeftjoins[] =
{
    {
        "failed_variation",
        "variation_feature.variation_id = failed_variation.variation_id"
    },
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic gvtranscriptvariationadaptorKDefaultcondition *****************
**
** Ensembl Genetic Variation Transcript Variation Adaptor
** SQL SELECT default condition
**
******************************************************************************/

static const char *gvtranscriptvariationadaptorKDefaultcondition =
    "transcript_variation.variation_feature_id = "
    "variation_feature.variation_feature_id "
    "AND "
    "variation_feature.source_id = "
    "source.source_id";




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool gvtranscriptvariationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvtvs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensvariation **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvconsequence] Ensembl Genetic Variation Consequence *****
**
** @nam2rule Gvconsequence Functions for manipulating
** Ensembl Genetic Variation Consequence objects
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType
** @cc CVS Revision: 1.29
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Consequence by
** pointer. It is the responsibility of the user to first destroy any previous
** Genetic Variation Consequence. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvconsequence]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
** @argrule Ini transcriptid [ajuint] Ensembl Transcript identifier
** @argrule Ini gvvfid [ajuint] Ensembl Genetic Variation Variation Feature
** @argrule Ini start [ajint] Start
** @argrule Ini end [ajint] End
** @argrule Ini strand [ajint] Strand
** @argrule Ref gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
**
** @valrule * [EnsPGvconsequence] Ensembl Genetic Variation Consequence or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvconsequenceNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [EnsPGvconsequence] Ensembl Genetic Variation Consequence or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvconsequence ensGvconsequenceNewCpy(const EnsPGvconsequence gvc)
{
    EnsPGvconsequence pthis = NULL;

    if (!gvc)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Transcriptidentifier = gvc->Transcriptidentifier;

    pthis->Gvvariationfeatureidentifier = gvc->Gvvariationfeatureidentifier;

    pthis->Start = gvc->Start;

    pthis->End = gvc->End;

    pthis->Strand = gvc->Strand;

    pthis->Alleles = ajListstrNew();

    pthis->Types = ajListstrNew();

    return pthis;
}




/* @func ensGvconsequenceNewIni ***********************************************
**
** Constructor for an Ensembl Genetic Variation Consequence with initial
** values.
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType::new
** @param [r] transcriptid [ajuint] Ensembl Transcript identifier
** @param [r] gvvfid [ajuint]
** Ensembl Genetic Variation Variation Feature identifier
** @param [r] start [ajint] Start
** @param [r] end [ajint] End
** @param [r] strand [ajint] Strand
**
** @return [EnsPGvconsequence] Ensembl Genetic Variation Consequence or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvconsequence ensGvconsequenceNewIni(ajuint transcriptid,
                                         ajuint gvvfid,
                                         ajint start,
                                         ajint end,
                                         ajint strand)
{
    EnsPGvconsequence gvc = NULL;

    if (!transcriptid)
        return NULL;

    if (!gvvfid)
        return NULL;

    AJNEW0(gvc);

    gvc->Use = 1U;

    gvc->Transcriptidentifier = transcriptid;

    gvc->Gvvariationfeatureidentifier = gvvfid;

    gvc->Start = start;

    gvc->End = end;

    gvc->Strand = strand;

    return gvc;
}




/* @func ensGvconsequenceNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
**
** @return [EnsPGvconsequence] Ensembl Genetic Variation Consequence or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvconsequence ensGvconsequenceNewRef(EnsPGvconsequence gvc)
{
    if (!gvc)
        return NULL;

    gvc->Use++;

    return gvc;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Consequence object.
**
** @fdata [EnsPGvconsequence]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Consequence
**
** @argrule * Pgvc [EnsPGvconsequence*]
** Ensembl Genetic Variation Consequence address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvconsequenceDel **************************************************
**
** Default destructor for an Ensembl Genetic Variation Consequence.
**
** @param [d] Pgvc [EnsPGvconsequence*]
** Ensembl Genetic Variation Consequence address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvconsequenceDel(EnsPGvconsequence *Pgvc)
{
    EnsPGvconsequence pthis = NULL;

    if (!Pgvc)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvconsequenceDel"))
    {
        ajDebug("ensGvconsequenceDel\n"
                "  *Pgvvc %p\n",
                *Pgvc);

        ensGvconsequenceTrace(*Pgvc, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvc) || --pthis->Use)
    {
        *Pgvc = NULL;

        return;
    }

    ajListstrFreeData(&pthis->Alleles);
    ajListstrFreeData(&pthis->Types);

    ajMemFree((void **) Pgvc);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Consequence object.
**
** @fdata [EnsPGvconsequence]
**
** @nam3rule Get Return Genetic Variation Consequence attribute(s)
** @nam4rule Alleles Return the AJAX List of AJAX String (allele) objects
** @nam4rule End Return the end
** @nam4rule Gvvariationfeatureidentifier
** Return the Ensembl Genetic Variation Variation Feature identifier
** @nam4rule Start Return the start
** @nam4rule Strand Return the strand
** @nam4rule Transcriptidentifier Return the Ensembl Transcript identifier
** @nam4rule Types Return the AJAX List of AJAX String types
**
** @argrule * gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @valrule Alleles [AjPList]
** AJAX List of AJAX String (allele) objects or NULL
** @valrule End [ajint] End or 0
** @valrule Gvvariationfeatureidentifier [ajuint]
** Ensembl Genetic Variation Variation Feature identifier or 0U
** @valrule Strand [ajint] Strand or 0
** @valrule Start [ajint] Start or 0
** @valrule Transcriptidentifier [ajuint] Ensembl Transcript identifier or 0U
** @valrule Types [AjPList] AJAX List of AJAX String types or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvconsequenceGetEnd ***********************************************
**
** Get the end member of an Ensembl Genetic Variation Consequence.
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType::end
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [ajint] End or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensGvconsequenceGetEnd(
    const EnsPGvconsequence gvc)
{
    return (gvc) ? gvc->End : 0;
}




/* @func ensGvconsequenceGetGvvariationfeatureidentifier **********************
**
** Get the Ensembl Genetic Variation Variation Feature identifier member of an
** Ensembl Genetic Variation Consequence.
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType::variation_feature_id
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [ajuint]
** Ensembl Genetic Variation Variation Feature identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvconsequenceGetGvvariationfeatureidentifier(
    const EnsPGvconsequence gvc)
{
    return (gvc) ? gvc->Gvvariationfeatureidentifier : 0U;
}




/* @func ensGvconsequenceGetStart *********************************************
**
** Get the start member of an Ensembl Genetic Variation Consequence.
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType::start
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [ajint] Start or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensGvconsequenceGetStart(
    const EnsPGvconsequence gvc)
{
    return (gvc) ? gvc->Start : 0;
}




/* @func ensGvconsequenceGetStrand ********************************************
**
** Get the strand member of an Ensembl Genetic Variation Consequence.
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType::strand
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [ajint] Strand or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensGvconsequenceGetStrand(
    const EnsPGvconsequence gvc)
{
    return (gvc) ? gvc->Strand : 0;
}




/* @func ensGvconsequenceGetTranscriptidentifier ******************************
**
** Get the Ensembl Transcript identifier member of an
** Ensembl Genetic Variation Consequence.
**
** @cc Bio::EnsEMBL::Variation::ConsequenceType::transcript_id
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [ajuint] Ensembl Transcript identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvconsequenceGetTranscriptidentifier(
    const EnsPGvconsequence gvc)
{
    return (gvc) ? gvc->Transcriptidentifier : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Consequence object.
**
** @fdata [EnsPGvconsequence]
**
** @nam3rule Set Set one member of a Genetic Variation Consequence
** @nam4rule End Set the end
** @nam4rule Gvvariationfeatureidentifier
** Set the Ensembl Genetic Variation Variation Feature identifier
** @nam4rule Start Set the start
** @nam4rule Strand Set the strand
** @nam4rule Transcriptidentifier Set the Ensembl Transcript identifier
**
** @argrule * gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
** @argrule End end [ajint] End
** @argrule Gvvariationfeatureidentifier gvvfid [ajuint]
** Ensembl Genetic Variation Variation Feature
** @argrule Start start [ajint] Start
** @argrule Strand strand [ajint] Strand
** @argrule Transcriptidentifier transcriptid [ajuint]
** Ensembl Transcript identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvconsequenceSetEnd ***********************************************
**
** Set the end member of an
** Ensembl Genetic Variation Consequence.
**
** @param [u] gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
** @param [r] end [ajint] End
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceSetEnd(EnsPGvconsequence gvc,
                              ajint end)
{
    if (!gvc)
        return ajFalse;

    gvc->End = end;

    return ajTrue;
}




/* @func ensGvconsequenceSetGvvariationfeatureidentifier **********************
**
** Set the Ensembl Genetic Variation Variation Feature identifier member of an
** Ensembl Genetic Variation Consequence.
**
** @param [u] gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
** @param [r] gvvfid [ajuint]
** Ensembl Genetic Variation Variation Feature identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceSetGvvariationfeatureidentifier(EnsPGvconsequence gvc,
                                                       ajuint gvvfid)
{
    if (!gvc)
        return ajFalse;

    gvc->Gvvariationfeatureidentifier = gvvfid;

    return ajTrue;
}




/* @func ensGvconsequenceSetStart *********************************************
**
** Set the start member of an
** Ensembl Genetic Variation Consequence.
**
** @param [u] gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
** @param [r] start [ajint] Start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceSetStart(EnsPGvconsequence gvc,
                                ajint start)
{
    if (!gvc)
        return ajFalse;

    gvc->Start = start;

    return ajTrue;
}




/* @func ensGvconsequenceSetStrand ********************************************
**
** Set the strand member of an
** Ensembl Genetic Variation Consequence.
**
** @param [u] gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
** @param [r] strand [ajint] Strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceSetStrand(EnsPGvconsequence gvc,
                                 ajint strand)
{
    if (!gvc)
        return ajFalse;

    gvc->Strand = strand;

    return ajTrue;
}




/* @func ensGvconsequenceSetTranscriptidentifier ******************************
**
** Set the Ensembl Transcript identifier member of an
** Ensembl Genetic Variation Consequence.
**
** @param [u] gvc [EnsPGvconsequence] Ensembl Genetic Variation Consequence
** @param [r] transcriptid [ajuint] Ensembl Transcript identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceSetTranscriptidentifier(EnsPGvconsequence gvc,
                                               ajuint transcriptid)
{
    if (!gvc)
        return ajFalse;

    gvc->Transcriptidentifier = transcriptid;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Consequence object.
**
** @fdata [EnsPGvconsequence]
**
** @nam3rule Trace Report Ensembl Genetic Variation Consequence members
**                 to debug file
**
** @argrule Trace gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvconsequenceTrace ************************************************
**
** Trace an Ensembl Genetic Variation Consequence.
**
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceTrace(const EnsPGvconsequence gvc,
                             ajuint level)
{
    AjPStr indent = NULL;

    if (!gvc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvconsequenceTrace %p\n"
            "%S  Transcriptidentifier %u\n"
            "%S  Gvvariationfeatureidentifier %u\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Strand %d\n",
            indent, gvc,
            indent, gvc->Transcriptidentifier,
            indent, gvc->Gvvariationfeatureidentifier,
            indent, gvc->Start,
            indent, gvc->End,
            indent, gvc->Strand);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Consequence object.
**
** @fdata [EnsPGvconsequence]
**
** @nam3rule Calculate
** Calculate Ensembl Genetic Variation Consequence information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvconsequenceCalculateMemsize *************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Consequence.
**
** @param [r] gvc [const EnsPGvconsequence]
** Ensembl Genetic Variation Consequence
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvconsequenceCalculateMemsize(const EnsPGvconsequence gvc)
{
    size_t size = 0;

    if (!gvc)
        return 0;

    size += sizeof (EnsOGvconsequence);

    return size;
}




/* @datasection [EnsEGvconsequenceType] Ensembl Genetic Variation Consequence
** Type enumeration
**
** @nam2rule Gvconsequence Functions for manipulating
** Ensembl Genetic Variation Consequence objects
** @nam3rule GvconsequenceType Functions for manipulating
** Ensembl Genetic Variation Consequence Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Consequence Type
** enumeration.
**
** @fdata [EnsEGvconsequenceType]
**
** @nam4rule From Ensembl Genetic Variation Consequence Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  consequencetype [const AjPStr] Consequence type string
**
** @valrule * [EnsEGvconsequenceType]
** Ensembl Genetic Variation Consequence Type enumeration or
** ensEGvconsequenceTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvconsequenceTypeFromStr ******************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Consequence
** Type enumeration.
**
** @param [r] consequencetype [const AjPStr] Consequence type string
**
** @return [EnsEGvconsequenceType]
** Ensembl Genetic Variation Consequence Type enumeration or
** ensEGvconsequenceTypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvconsequenceType ensGvconsequenceTypeFromStr(
    const AjPStr consequencetype)
{
    register EnsEGvconsequenceType i =
        ensEGvconsequenceTypeNULL;

    EnsEGvconsequenceType gvct =
        ensEGvconsequenceTypeNULL;

    for (i = ensEGvconsequenceTypeNULL;
         gvconsequenceKType[i];
         i++)
        if (ajStrMatchC(consequencetype, gvconsequenceKType[i]))
            gvct = i;

    if (!gvct)
        ajDebug("ensGvconsequenceTypeFromStr encountered "
                "unexpected string '%S'.\n", consequencetype);

    return gvct;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Consequence Type enumeration.
**
** @fdata [EnsEGvconsequenceType]
**
** @nam4rule To   Return Ensembl Genetic Variation Consequence Type
**                       enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvct [EnsEGvconsequenceType]
** Ensembl Genetic Variation Consequence Type enumeration
**
** @valrule Char [const char*]
** Ensembl Genetic Variation Consequence Type C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvconsequenceTypeToChar *******************************************
**
** Convert an Ensembl Genetic Variation Consequence Type enumeration
** into a C-type (char *) string.
**
** @param [u] gvct [EnsEGvconsequenceType]
** Ensembl Genetic Variation Consequence Type enumeration
**
** @return [const char*]
** Ensembl Genetic Variation Consequence Type C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensGvconsequenceTypeToChar(
    EnsEGvconsequenceType gvct)
{
    register EnsEGvconsequenceType i =
        ensEGvconsequenceTypeNULL;

    for (i = ensEGvconsequenceTypeNULL;
         gvconsequenceKType[i] && (i < gvct);
         i++);

    if (!gvconsequenceKType[i])
        ajDebug("ensGvconsequenceTypeToChar "
                "encountered an out of boundary error on "
                "Ensembl Genetic Variation Consequence Type "
                "enumeration %d.\n",
                gvct);

    return gvconsequenceKType[i];
}




/* @datasection [none] C-types ************************************************
**
** @nam2rule Gvconsequence Functions for manipulating
** Ensembl Genetic Variation Consequence objects
** @nam3rule GvconsequenceTypes Functions for manipulating
** Ensembl Genetic Variation Consequence Type (AJAX unsigned integer)
** bit fields
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Consequence Type
** bit field.
**
** @fdata [none]
**
** @nam4rule From Ensembl Genetic Variation Consequence Type
** bit field query
** @nam5rule Set SQL set string query
**
** @argrule  Set gvctset [const AjPStr] Comma-separated SQL set string
**
** @valrule * [ajuint]
** Ensembl Genetic Variation Consequence Type bit field or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensGvconsequenceTypesFromSet *****************************************
**
** Convert an AJAX String representing a comma-separared SQL set of
** consequence types into an Ensembl Genetic Variation Consequence Type
** (AJAX unsigned integer) bit field.
**
** @param [r] gvctset [const AjPStr] SQL set
**
** @return [ajuint] Ensembl Genetic Variation Consequence Type bit field or 0U
** or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvconsequenceTypesFromSet(const AjPStr gvctset)
{
    ajuint gvctbf = 0U;

    AjPStr gvctstring = NULL;

    AjPStrTok token = NULL;

    if (!gvctset)
        return 0U;

    /* Split the comma-separated list of consequence types. */

    token = ajStrTokenNewC(gvctset, ",");

    gvctstring = ajStrNew();

    while (ajStrTokenNextParse(token, &gvctstring))
        gvctbf |= (1U << ensGvconsequenceTypeFromStr(gvctstring));

    ajStrDel(&gvctstring);

    ajStrTokenDel(&token);

    return gvctbf;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Consequence Type bit field.
**
** @fdata [none]
**
** @nam4rule To Cast an Ensembl Genetic Variation Consequence Type
** bit field
** @nam5rule Set Cast into an SQl set (comma-separated strings)
**
** @argrule To gvctbf [ajuint]
** Ensembl Genetic Variation Consequence Type bit field
** @argrule Set Pgvctset [AjPStr*] SQL set
**
** @valrule * [AjBool] True on success, false on failure
**
** @fcategory cast
******************************************************************************/




/* @func ensGvconsequenceTypesToSet *******************************************
**
** Cast an Ensembl Genetic Variation Consequence Type bit field
** (AJAX unsigned integer) into an SQL set (comma-separated) strings.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [r] gvctbf [ajuint]
** Ensembl Genetic Variation Consequence Type bit field
** @param [w] Pgvctset [AjPStr*] SQL set
**
** @return [AjBool] True on success, false on failure
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvconsequenceTypesToSet(ajuint gvctbf, AjPStr *Pgvctset)
{
    register ajuint i = 0U;

    if (*Pgvctset)
        ajStrAssignClear(Pgvctset);
    else
        *Pgvctset = ajStrNew();

    for (i = 1U; gvconsequenceKType[i]; i++)
        if (gvctbf & (1U << i))
        {
            ajStrAppendC(Pgvctset, gvconsequenceKType[i]);
            ajStrAppendC(Pgvctset, ",");
        }

    /* Remove the last comma if one exists. */

    if (ajStrGetLen(*Pgvctset) > 0U)
        ajStrCutEnd(Pgvctset, 1);

    return ajTrue;
}




/* @datasection [EnsPGvtranscriptvariation] Ensembl Genetic Variation
** Transcript Variation
**
** @nam2rule Gvtranscriptvariation Functions for manipulating
** Ensembl Genetic Variation Transcript Variation objects
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation
** @cc CVS Revision: 1.28
** @cc CVS Tag: branch-ensembl-61
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Transcript Variation
** by pointer. It is the responsibility of the user to first destroy any
** previous Genetic Variation Transcript Variation. The target pointer does not
** need to be initialised to NULL, but it is good programming practice to do
**  so anyway.
**
** @fdata [EnsPGvtranscriptvariation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvtv [const EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @argrule Ini gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini transcript [EnsPTranscript] Ensembl Transcript
** @argrule Ini gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
** @argrule Ini translationallele [AjPStr] Ensembl Translation allele
** @argrule Ini gvctset [AjPStr]
** Ensembl Genetic Variation Consequence Type SQL set
** @argrule Ini codingstart [ajuint] Coding start
** @argrule Ini codingend [ajuint] Coding end
** @argrule Ini transcriptstart [ajuint] Ensembl Transcript start
** @argrule Ini transcriptend [ajuint] Ensembl Transcript end
** @argrule Ini translationstart [ajuint] Ensembl Translation start
** @argrule Ini translationend [ajuint] Ensembl Translation end
** @argrule Ref gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
**
** @valrule * [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvtranscriptvariationNewCpy ***************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvtv [const EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
**
** @return [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvtranscriptvariation ensGvtranscriptvariationNewCpy(
    const EnsPGvtranscriptvariation gvtv)
{
    EnsPGvtranscriptvariation pthis = NULL;

    if (!gvtv)
        return NULL;

    AJNEW0(pthis);

    pthis->Use                = 1U;
    pthis->Identifier         = gvtv->Identifier;
    pthis->Adaptor            = gvtv->Adaptor;
    pthis->TranscriptObject   = ensTranscriptNewRef(gvtv->TranscriptObject);
    pthis->Gvvariationfeature = ensGvvariationfeatureNewRef(
        gvtv->Gvvariationfeature);

    if (gvtv->TranslationAllele)
        pthis->TranslationAllele  = ajStrNewRef(gvtv->TranslationAllele);

    pthis->CodingStart        = gvtv->CodingStart;
    pthis->CodingEnd          = gvtv->CodingEnd;
    pthis->TranscriptStart    = gvtv->TranscriptStart;
    pthis->TranscriptEnd      = gvtv->TranscriptEnd;
    pthis->TranslationStart   = gvtv->TranslationStart;
    pthis->TranslationEnd     = gvtv->TranslationEnd;
    pthis->GvconsequenceTypes = gvtv->GvconsequenceTypes;

    return pthis;
}




/* @func ensGvtranscriptvariationNewIni ***************************************
**
** Constructor for an Ensembl Genetic Variation Transcript Variation with
** initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::new
** @param [uN] transcript [EnsPTranscript] Ensembl Transcript
** @param [uN] gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
** @param [u] translationallele [AjPStr] Ensembl Translation allele
** @param [u] gvctset [AjPStr] Ensembl Genetic Variation Consequence Type
** SQL set
** @param [r] codingstart [ajuint] Coding start
** @param [r] codingend [ajuint] Coding end
** @param [r] transcriptstart [ajuint] Ensembl Transcript start
** @param [r] transcriptend [ajuint] Ensembl Transcript end
** @param [r] translationstart [ajuint] Ensembl Translation start
** @param [r] translationend [ajuint] Ensembl Translation end
**
** @return [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation or NULL
**
** @release 6.4.0
** @@
** NOTE: From branch-ensembl-61 the new method accepts a list of consequence
** type strings rather than a comma separated SQL set of quoted consequence
** type strings.
******************************************************************************/

EnsPGvtranscriptvariation ensGvtranscriptvariationNewIni(
    EnsPGvtranscriptvariationadaptor gvtva,
    ajuint identifier,
    EnsPTranscript transcript,
    EnsPGvvariationfeature gvvf,
    AjPStr translationallele,
    AjPStr gvctset,
    ajuint codingstart,
    ajuint codingend,
    ajuint transcriptstart,
    ajuint transcriptend,
    ajuint translationstart,
    ajuint translationend)
{
    register ajuint i = 0U;

    AjBool match = AJFALSE;

    AjPStr value = NULL;

    AjPStrTok token = NULL;

    EnsPGvtranscriptvariation gvtv = NULL;

    /* Check that all consequence types are valid. */

    if (gvctset && ajStrGetLen(gvctset))
    {
        token = ajStrTokenNewC(gvctset, ",");
        value = ajStrNew();

        while (ajStrTokenNextParse(token, &value))
        {
            match = ajFalse;

            for (i = 0U; gvconsequenceKType[i]; i++)
                if (ajStrMatchCaseC(value, gvconsequenceKType[i]))
                    match = ajTrue;

            if (match == ajFalse)
                ajFatal("ensGvtranscriptvariationNewIni got invalid "
                        "consequence type '%S'.", value);
        }

        ajStrTokenDel(&token);
        ajStrDel(&value);
    }

    AJNEW0(gvtv);

    gvtv->Use                = 1U;
    gvtv->Identifier         = identifier;
    gvtv->Adaptor            = gvtva;
    gvtv->TranscriptObject   = ensTranscriptNewRef(transcript);
    gvtv->Gvvariationfeature = ensGvvariationfeatureNewRef(gvvf);
    gvtv->TranslationAllele  = ajStrNewRef(translationallele);
    gvtv->CodingStart        = codingstart;
    gvtv->CodingEnd          = codingend;
    gvtv->TranscriptStart    = transcriptstart;
    gvtv->TranscriptEnd      = transcriptend;
    gvtv->TranslationStart   = translationstart;
    gvtv->TranslationEnd     = translationend;
    gvtv->GvconsequenceTypes = ensGvconsequenceTypesFromSet(gvctset);

    return gvtv;
}





/* @func ensGvtranscriptvariationNewRef ***************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvtv [EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                             Transcript Variation
**
** @return [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvtranscriptvariation ensGvtranscriptvariationNewRef(
    EnsPGvtranscriptvariation gvtv)
{
    if (!gvtv)
        return NULL;

    gvtv->Use++;

    return gvtv;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Transcript Variation object.
**
** @fdata [EnsPGvtranscriptvariation]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Transcript Variation
**
** @argrule * Pgvtv [EnsPGvtranscriptvariation*]
** Ensembl Genetic Variation Transcript Variation address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvtranscriptvariationDel ******************************************
**
** Default destructor for an Ensembl Genetic Variation Transcript Variation.
**
** @param [d] Pgvtv [EnsPGvtranscriptvariation*]
** Ensembl Genetic Variation Transcript Variation address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvtranscriptvariationDel(EnsPGvtranscriptvariation *Pgvtv)
{
    EnsPGvtranscriptvariation pthis = NULL;

    if (!Pgvtv)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvtranscriptvariationDel"))
    {
        ajDebug("ensGvtranscriptvariationDel\n"
                "  *Pgvtv %p\n",
                *Pgvtv);

        ensGvtranscriptvariationTrace(*Pgvtv, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvtv) || --pthis->Use)
    {
        *Pgvtv = NULL;

        return;
    }

    ensGvvariationfeatureDel(&pthis->Gvvariationfeature);

    ensTranscriptDel(&pthis->TranscriptObject);

    ajStrDel(&pthis->TranslationAllele);

    ajMemFree((void **) Pgvtv);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Transcript Variation object.
**
** @fdata [EnsPGvtranscriptvariation]
**
** @nam3rule Get Return Ensembl Genetic Variation Transcript Variation
** attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation
**                   Transcript Variation Adaptor
** @nam4rule Coding Return coding region members
** @nam4rule Gvvariationfeature Return the
** Ensembl Genetic Variation Variation Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Transcript Return Ensembl Transcript members
** @nam4rule Translation Return Ensembl Translation members
** @nam5rule Allele Return the allele string
** @nam5rule End Return the end coordinate
** @nam5rule Start Return the start coordinate
** @nam5rule Object Return an Ensembl Object
**
** @argrule * gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @valrule Adaptor [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor or NULL
** @valrule Gvvariationfeature [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Allele [AjPStr] Allele string or NULL
** @valrule End [ajuint] End coordinate or 0U
** @valrule Object [EnsPTranscript] Ensembl Transcript or NULL
** @valrule Start [ajuint] Start coordinate or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvtranscriptvariationGetAdaptor ***********************************
**
** Get the Ensembl Genetic Variation Transcript Variation Adaptor member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [EnsPGvtranscriptvariationadaptor] Ensembl Genetic Variation
** Transcript Variation Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvtranscriptvariationadaptor ensGvtranscriptvariationGetAdaptor(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->Adaptor : NULL;
}




/* @func ensGvtranscriptvariationGetCodingEnd *********************************
**
** Get the coding end member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cds_end
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] Coding end or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetCodingEnd(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->CodingEnd : 0U;
}




/* @func ensGvtranscriptvariationGetCodingStart *******************************
**
** Get the coding start member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cds_start
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] Coding start or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetCodingStart(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->CodingStart : 0U;
}




/* @func ensGvtranscriptvariationGetGvvariationfeature ************************
**
** Get the Ensembl Genetic Variation Variation Feature member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [EnsPGvvariationfeature] Ensembl Genetic Variation Variation Feature
** or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvtranscriptvariationGetGvvariationfeature(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->Gvvariationfeature : NULL;
}




/* @func ensGvtranscriptvariationGetIdentifier ********************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetIdentifier(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->Identifier : 0U;
}




/* @func ensGvtranscriptvariationGetTranscriptEnd *****************************
**
** Get the Ensembl Transcript end member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cdna_end
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] Ensembl Transcript end or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetTranscriptEnd(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->TranscriptEnd : 0U;
}




/* @func ensGvtranscriptvariationGetTranscriptObject **************************
**
** Get the Ensembl Transcript member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::transcript
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [EnsPTranscript] Ensembl Transcript or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPTranscript ensGvtranscriptvariationGetTranscriptObject(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->TranscriptObject : NULL;
}




/* @func ensGvtranscriptvariationGetTranscriptStart ***************************
**
** Get the Ensembl Transcript start member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cdna_start
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] Ensembl Transcript start or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetTranscriptStart(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->TranscriptStart : 0U;
}




/* @func ensGvtranscriptvariationGetTranslationAllele *************************
**
** Get the Ensembl Translation allele member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::pep_allele_string
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [AjPStr] Ensembl Translation allele or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvtranscriptvariationGetTranslationAllele(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->TranslationAllele : NULL;
}




/* @func ensGvtranscriptvariationGetTranslationEnd ****************************
**
** Get the Ensembl Translation end member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::translation_end
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] Ensembl Translation end or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetTranslationEnd(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->TranslationEnd : 0U;
}




/* @func ensGvtranscriptvariationGetTranslationStart **************************
**
** Get the Ensembl Translation start member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::translation_start
** @param [r] gvtv [const EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                                   Transcript Variation
**
** @return [ajuint] Ensembl Translation start or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvtranscriptvariationGetTranslationStart(
    const EnsPGvtranscriptvariation gvtv)
{
    return (gvtv) ? gvtv->TranslationStart : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Transcript Variation object.
**
** @fdata [EnsPGvtranscriptvariation]
**
** @nam3rule Set Set one member of an
** Ensembl Genetic Variation Transcript Variation
** @nam4rule Adaptor Set the Ensembl Genetic Variation
**                   Transcript Variation Adaptor
** @nam4rule Coding Set coding region members
** @nam4rule Gvvariationfeature Set the
** Ensembl Genetic Variation Variation Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Transcript Set Ensembl Transcript members
** @nam4rule Translation Set Ensembl Translation members
** @nam5rule Allele Set the allele
** @nam5rule End Set the end coordinate
** @nam5rule Object Set an Ensembl Object
** @nam5rule Start Set the start coordinate
**
** @argrule * gvtv [EnsPGvtranscriptvariation] Ensembl Genetic Variation
**                                             Transcript Variation object
** @argrule Adaptor gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
** @argrule CodingEnd codingend [ajuint] Coding region end
** @argrule CodingStart codingstart [ajuint] Coding region start
** @argrule Gvvariationfeature gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule TranscriptEnd transcriptend [ajuint] Ensembl Transcript end
** @argrule TranscriptObject transcript [EnsPTranscript] Ensembl Transcript
** @argrule TranscriptStart transcriptstart [ajuint] Ensembl Transcript start
** @argrule TranslationAllele translationallele [AjPStr]
** Ensembl Translation allele
** @argrule TranslationEnd translationend [ajuint] Ensembl Translation end
** @argrule TranslationStart translationstart [ajuint]
** Ensembl Translation start
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvtranscriptvariationSetAdaptor ***********************************
**
** Set the Ensembl Genetic Variation Transcript Variation Adaptor member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [u] gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetAdaptor(
    EnsPGvtranscriptvariation gvtv,
    EnsPGvtranscriptvariationadaptor gvtva)
{
    if (!gvtv)
        return ajFalse;

    gvtv->Adaptor = gvtva;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetCodingEnd *********************************
**
** Set the coding region end member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cds_end
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] codingend [ajuint] Coding region end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetCodingEnd(
    EnsPGvtranscriptvariation gvtv,
    ajuint codingend)
{
    if (!gvtv)
        return ajFalse;

    gvtv->CodingEnd = codingend;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetCodingStart *******************************
**
** Set the coding region start member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cds_start
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] codingstart [ajuint] Coding region start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetCodingStart(
    EnsPGvtranscriptvariation gvtv,
    ajuint codingstart)
{
    if (!gvtv)
        return ajFalse;

    gvtv->CodingStart = codingstart;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetGvvariationfeature ************************
**
** Set the Ensembl Genetic Variation Variation Feature member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::variation_feature
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [uN] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetGvvariationfeature(
    EnsPGvtranscriptvariation gvtv,
    EnsPGvvariationfeature gvvf)
{
    if (!gvtv)
        return ajFalse;

    ensGvvariationfeatureDel(&gvtv->Gvvariationfeature);

    gvtv->Gvvariationfeature = ensGvvariationfeatureNewRef(gvvf);

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetIdentifier ********************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetIdentifier(
    EnsPGvtranscriptvariation gvtv,
    ajuint identifier)
{
    if (!gvtv)
        return ajFalse;

    gvtv->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetTranscriptEnd *****************************
**
** Set the Ensembl Transcript end member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cdna_end
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] transcriptend [ajuint] Ensembl Transcript end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetTranscriptEnd(
    EnsPGvtranscriptvariation gvtv,
    ajuint transcriptend)
{
    if (!gvtv)
        return ajFalse;

    gvtv->TranscriptEnd = transcriptend;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetTranscriptObject **************************
**
** Set the Ensembl Transcript member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::transcript
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [uN] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetTranscriptObject(
    EnsPGvtranscriptvariation gvtv,
    EnsPTranscript transcript)
{
    if (!gvtv)
        return ajFalse;

    ensTranscriptDel(&gvtv->TranscriptObject);

    gvtv->TranscriptObject = ensTranscriptNewRef(transcript);

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetTranscriptStart ***************************
**
** Set the Ensembl Transcript start member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::cdna_start
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] transcriptstart [ajuint] Ensembl Transcript start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetTranscriptStart(
    EnsPGvtranscriptvariation gvtv,
    ajuint transcriptstart)
{
    if (!gvtv)
        return ajFalse;

    gvtv->TranscriptStart = transcriptstart;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetTranslationAllele *************************
**
** Set the Ensembl Translation allele member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::pep_allele_string
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [u] translationallele [AjPStr] Ensembl Translation allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetTranslationAllele(
    EnsPGvtranscriptvariation gvtv,
    AjPStr translationallele)
{
    if (!gvtv)
        return ajFalse;

    ajStrDel(&gvtv->TranslationAllele);

    if (translationallele)
        gvtv->TranslationAllele = ajStrNewRef(translationallele);

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetTranslationEnd ****************************
**
** Set the Ensembl Translation end member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::translation_end
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] translationend [ajuint] Ensembl Translation end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetTranslationEnd(
    EnsPGvtranscriptvariation gvtv,
    ajuint translationend)
{
    if (!gvtv)
        return ajFalse;

    gvtv->TranslationEnd = translationend;

    return ajTrue;
}




/* @func ensGvtranscriptvariationSetTranslationStart **************************
**
** Set the Ensembl Translation start member of an
** Ensembl Genetic Variation Transcript Variation.
**
** @cc Bio::EnsEMBL::Variation::TranscriptVariation::translation_start
** @param [u] gvtv [EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] translationstart [ajuint] Ensembl Translation start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationSetTranslationStart(
    EnsPGvtranscriptvariation gvtv,
    ajuint translationstart)
{
    if (!gvtv)
        return ajFalse;

    gvtv->TranslationStart = translationstart;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Transcript Variation object.
**
** @fdata [EnsPGvtranscriptvariation]
**
** @nam3rule Trace Report Ensembl Genetic Variation Transcript Variation
**                 members to debug file
**
** @argrule Trace gvtv [const EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvtranscriptvariationTrace ****************************************
**
** Trace an Ensembl Genetic Variation Transcript Variation.
**
** @param [r] gvtv [const EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvtranscriptvariationTrace(const EnsPGvtranscriptvariation gvtv,
                                     ajuint level)
{
    AjPStr indent = NULL;

    if (!gvtv)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvtranscriptvariationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  TranscriptObject %p\n"
            "%S  Gvvariationfeature %p\n"
            "%S  TranslationAllele '%S'\n"
            "%S  CodingStart %u\n"
            "%S  CodingEnd %u\n"
            "%S  TranscriptStart %u\n"
            "%S  TranscriptEnd %u\n"
            "%S  TranslationStart %u\n"
            "%S  TranslationEnd %u\n",
            indent, gvtv,
            indent, gvtv->Use,
            indent, gvtv->Identifier,
            indent, gvtv->Adaptor,
            indent, gvtv->TranscriptObject,
            indent, gvtv->Gvvariationfeature,
            indent, gvtv->TranslationAllele,
            indent, gvtv->CodingStart,
            indent, gvtv->CodingEnd,
            indent, gvtv->TranscriptStart,
            indent, gvtv->TranscriptEnd,
            indent, gvtv->TranslationStart,
            indent, gvtv->TranslationEnd);

    ensTranscriptTrace(gvtv->TranscriptObject, level + 1);

    ensGvvariationfeatureTrace(gvtv->Gvvariationfeature, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Transcript Variation object.
**
** @fdata [EnsPGvtranscriptvariation]
**
** @nam3rule Calculate
** Calculate Ensembl Genetic Variation Transcript Variation information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvtv [const EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvtranscriptvariationCalculateMemsize *****************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Transcript Variation.
**
** @param [r] gvtv [const EnsPGvtranscriptvariation]
** Ensembl Genetic Variation Transcript Variation
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvtranscriptvariationCalculateMemsize(
    const EnsPGvtranscriptvariation gvtv)
{
    size_t size = 0;

    if (!gvtv)
        return 0;

    size += sizeof (EnsOGvtranscriptvariation);

    size += ensTranscriptCalculateMemsize(gvtv->TranscriptObject);

    size += ensGvvariationfeatureCalculateMemsize(gvtv->Gvvariationfeature);

    if (gvtv->TranslationAllele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvtv->TranslationAllele);
    }

    return size;
}




/* @datasection [EnsPGvtranscriptvariationadaptor] Ensembl Genetic Variation
** Transcript Variation Adaptor
**
** @nam2rule Gvtranscriptvariationadaptor Functions for manipulating
** Ensembl Genetic Variation Transcript Variation Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::TranscriptVariationAdaptor
** @cc CVS Revision: 1.29
** @cc CVS Tag: branch-ensembl-61
**
******************************************************************************/




/* @funcstatic gvtranscriptvariationadaptorFetchAllbyStatement ****************
**
** Fetch all Ensembl Genetic Variation Transcript Variation objects
** via an SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::TranscriptVariationAdaptor::
** _objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvtvs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Transcript Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvtranscriptvariationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvtvs)
{
    ajuint lastgvvfid       = 0U;
    ajuint identifier       = 0U;
    ajuint gvvfid           = 0U;
    ajuint codingstart      = 0U;
    ajuint codingend        = 0U;
    ajuint transcriptstart  = 0U;
    ajuint transcriptend    = 0U;
    ajuint translationstart = 0U;
    ajuint translationend   = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr gvctset            = NULL;
    AjPStr transcriptstableid = NULL;
    AjPStr translationallele  = NULL;

    EnsPDatabaseadaptor csdba = NULL;
    EnsPDatabaseadaptor gvdba = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    EnsPGvtranscriptvariation        gvtv  = NULL;
    EnsPGvtranscriptvariationadaptor gvtva = NULL;

    EnsPGvvariationfeature        gvvf  = NULL;
    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    if (ajDebugTest("gvtranscriptvariationadaptorFetchAllbyStatement"))
        ajDebug("gvtranscriptvariationadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvtvs %p\n",
                ba,
                statement,
                am,
                slice,
                gvtvs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvtvs)
        return ajFalse;

    gvdba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvtva = ensRegistryGetGvtranscriptvariationadaptor(gvdba);
    gvvfa = ensRegistryGetGvvariationfeatureadaptor(gvdba);
    csdba = ensRegistryGetReferenceadaptor(gvdba);
    tca   = ensRegistryGetTranscriptadaptor(csdba);

    sqls = ensDatabaseadaptorSqlstatementNew(gvdba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier         = 0;
        transcriptstableid = ajStrNew();
        gvvfid             = 0;
        transcriptstart    = 0;
        transcriptend      = 0;
        codingstart        = 0;
        codingend          = 0;
        translationstart   = 0;
        translationend     = 0;
        translationallele  = ajStrNew();
        gvctset            = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &transcriptstableid);
        ajSqlcolumnToUint(sqlr, &gvvfid);
        ajSqlcolumnToUint(sqlr, &transcriptstart);
        ajSqlcolumnToUint(sqlr, &transcriptend);
        ajSqlcolumnToUint(sqlr, &codingstart);
        ajSqlcolumnToUint(sqlr, &codingend);
        ajSqlcolumnToUint(sqlr, &translationstart);
        ajSqlcolumnToUint(sqlr, &translationend);
        ajSqlcolumnToStr(sqlr, &translationallele);
        ajSqlcolumnToStr(sqlr, &gvctset);

        /*
        ** Skip multiple rows, because of the SQL LEFT JOIN condition to
        ** the "failed_variation" SQL table.
        */

        if (lastgvvfid == identifier)
        {
            ajStrDel(&transcriptstableid);
            ajStrDel(&translationallele);
            ajStrDel(&gvctset);

            continue;
        }
        else
            lastgvvfid = identifier;

        /*
        ** TODO: Ensembl Transcript and Ensembl Genetic Variation Variation
        ** Feature objects should be fetched from the database in bulk
        ** via AJAX Table fetch functions.
        ** TODO: The Perl implementation stores the Ensembl Genetic Variation
        ** Variation Feature identifier and loads the object from the SQL
        ** database on demand.
        */

        ensTranscriptadaptorFetchByStableidentifier(
            tca,
            transcriptstableid,
            0,
            &transcript);

        ensGvvariationfeatureadaptorFetchByIdentifier(
            gvvfa,
            identifier,
            &gvvf);

        gvtv = ensGvtranscriptvariationNewIni(
            gvtva,
            identifier,
            transcript,
            gvvf,
            translationallele,
            gvctset,
            codingstart,
            codingend,
            transcriptstart,
            transcriptend,
            translationstart,
            translationend);

        ajListPushAppend(gvtvs, (void *) gvtv);

        ensGvvariationfeatureDel(&gvvf);

        ensTranscriptDel(&transcript);

        ajStrDel(&transcriptstableid);
        ajStrDel(&translationallele);
        ajStrDel(&gvctset);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(gvdba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Transcript Variation
** Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Transcript Variation Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvtranscriptvariationadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvtranscriptvariationadaptorNew ***********************************
**
** Default constructor for an
** Ensembl Genetic Variation Transcript Variation Adaptor.
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
** @see ensRegistryGetGvtranscriptvariationadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::TranscriptVariationAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvtranscriptvariationadaptor ensGvtranscriptvariationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPBaseadaptor ba = NULL;

    EnsPGvtranscriptvariationadaptor gvtva = NULL;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensGvtranscriptvariationadaptorNew"))
        ajDebug("ensGvtranscriptvariationadaptorNew\n"
                "  dba %p\n",
                dba);

    ba = ensBaseadaptorNew(
        dba,
        gvtranscriptvariationadaptorKTablenames,
        gvtranscriptvariationadaptorKColumnnames,
        gvtranscriptvariationadaptorKLeftjoins,
        gvtranscriptvariationadaptorKDefaultcondition,
        (const char *) NULL,
        &gvtranscriptvariationadaptorFetchAllbyStatement);

    if (!ba)
        return NULL;

    AJNEW0(gvtva);

    gvtva->Adaptor     = ensRegistryGetGvdatabaseadaptor(dba);
    gvtva->Baseadaptor = ba;

    return gvtva;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Transcript Variation Adaptor
** object.
**
** @fdata [EnsPGvtranscriptvariationadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Transcript Variation Adaptor
**
** @argrule * Pgvtva [EnsPGvtranscriptvariationadaptor*]
** Ensembl Genetic Variation Transcript Variation Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvtranscriptvariationadaptorDel ***********************************
**
** Default destructor for an
** Ensembl Genetic Variation Transcript Variation Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvtva [EnsPGvtranscriptvariationadaptor*]
** Ensembl Genetic Variation Transcript Variation Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvtranscriptvariationadaptorDel(
    EnsPGvtranscriptvariationadaptor *Pgvtva)
{
    EnsPGvtranscriptvariationadaptor pthis = NULL;

    if (!Pgvtva)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvtranscriptvariationadaptorDel"))
        ajDebug("ensGvtranscriptvariationadaptorDel\n"
                "  *Pgvtva %p\n",
                *Pgvtva);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvtva))
        return;

    ensBaseadaptorDel(&pthis->Baseadaptor);

    ajMemFree((void **) Pgvtva);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Transcript Variation Adaptor object.
**
** @fdata [EnsPGvtranscriptvariationadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Transcript Variation Adaptor
** attribute(s)
** @nam4rule Baseadaptor
** Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor
** Return the Ensembl Database Adaptor
** @nam4rule Gvbaseadaptor
** Return the Ensembl Genetic Variation Base Adaptor
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
**
** @argrule * gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Gvbaseadaptor [EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor or NULL
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvtranscriptvariationadaptorGetBaseadaptor ************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Transcript Variation Adaptor.
**
** @param [u] gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvtranscriptvariationadaptorGetBaseadaptor(
    EnsPGvtranscriptvariationadaptor gvtva)
{
    return ensGvbaseadaptorGetBaseadaptor(
        ensGvtranscriptvariationadaptorGetGvbaseadaptor(gvtva));
}




/* @func ensGvtranscriptvariationadaptorGetDatabaseadaptor ********************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Transcript Variation Adaptor.
**
** @param [u] gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvtranscriptvariationadaptorGetDatabaseadaptor(
    EnsPGvtranscriptvariationadaptor gvtva)
{
    return ensGvbaseadaptorGetDatabaseadaptor(
        ensGvtranscriptvariationadaptorGetGvbaseadaptor(gvtva));
}




/* @func ensGvtranscriptvariationadaptorGetGvbaseadaptor **********************
**
** Get the Ensembl Genetic Variation Base Adaptor member of an
** Ensembl Genetic Variation Transcript Variation Adaptor.
**
** @param [u] gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
**
** @return [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvbaseadaptor ensGvtranscriptvariationadaptorGetGvbaseadaptor(
    EnsPGvtranscriptvariationadaptor gvtva)
{
    return gvtva;
}




/* @func ensGvtranscriptvariationadaptorGetGvdatabaseadaptor ******************
**
** Get the Ensembl Genetic Variation Database Adaptor member of an
** Ensembl Genetic Variation Transcript Variation Adaptor.
**
** @param [u] gvtva [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvtranscriptvariationadaptorGetGvdatabaseadaptor(
    EnsPGvtranscriptvariationadaptor gvtva)
{
    return ensGvbaseadaptorGetGvdatabaseadaptor(
        ensGvtranscriptvariationadaptorGetGvbaseadaptor(gvtva));
}

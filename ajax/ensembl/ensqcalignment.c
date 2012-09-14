/* @source ensqcalignment *****************************************************
**
** Ensembl Quality Check Alignment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.40 $
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

#include "ensqcalignment.h"




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

#if AJFALSE
/* @conststatic qcalignmentKQueryCoveragePropertiesDnaDna *********************
**
** DNA to DNA Query Coverage Scoring Schema
**
**          5'-terminus                         3'-terminus
**
**          2**n score                          2**n score
**            -     0   -                         -     0
**            0     1   alignment                 0     1
**            -     0   non-matching region       -     0
**            2     4   shorter region            1     2
**            4    16   longer region             3     8
**            6    64   non-matching edge         5    32
**            8   256   shorter edge              7   128
**           10  1024   longer edge               9   512
**           12  4096   perfect                  11  2048
**           13  8192   identity >= threshold    13  8192
**
******************************************************************************/

static const char *qcalignmentKQueryCoveragePropertiesDnaDna[] =
{
    "No Alignment",
    "Alignment",
    "3' shorter region",
    "5' shorter region",
    "3' longer region",
    "5' longer region",
    "3' non-matching edge",
    "5' non-matching edge",
    "3' shorter edge",
    "5' shorter edge",
    "3' longer edge",
    "5' longer edge",
    "3' perfect",
    "5' perfect",
    "Identity Threshold",
    "Sum",
    (const char *) NULL
};
#endif /* AJFALSE */




#if AJFALSE
/* @conststatic qcalignmentKQueryCoveragePropertiesDnaGenome ******************
**
** cDNA to Genome Query Coverage Scoring Schema
**
** The query coverage score reflects individual alignment properties by
** adding 2**n, where n increases with the quality of the alignment. The
** following alignment properties are currently scored for query
** sequences of type 'dna'.
**
**          5'-terminus                         3'-terminus
**
**          2**n score                          2**n score
**            -     0   alignment is missing      -     0
**            0     1   alignment is there        0     1
**            -     0   non-matching ends         -     0
**            2     4   shorter ends              1     2
**            4    16   edge <= threshold         3     8
**            6    64   perfect                   5    32
**            7   128   identity >= threshold     7   128
**
******************************************************************************/

static const char *qcalignmentKQueryCoveragePropertiesDnaGenome[] =
{
    "No Alignment",
    "Alignment",
    "3' shorter",
    "5' shorter",
    "3' tolerance",
    "5' tolerance",
    "3' perfect",
    "5' perfect",
    "Identity Threshold",
    "Coverage Threshold",
    "Sum",
    (const char *) NULL
};
#endif /* AJFALSE */




#if AJFALSE
/* @conststatic qcalignmentKQueryCoveragePropertiesProteinGenome **************
**
** Protein to Genome Query Coverage Scoring Schema
**
** The query coverage score reflects individual alignment properties by
** adding 2**n, where n increases with the quality of the alignment. The
** following alignment properties are currently scored for query
** sequences of type 'protein'.
**
**          N-terminus                          C-terminus
**
**          2**n score                          2**n score
**            -     0   -                         -     0
**            0     1   alignment                 0     1
**            -     0   non-matching region       -     0
**            2     4   shorter                   1     2
**            4    16   edge <= threshold         3     8
**            6    64   perfect                   5    32
**            7   128   identity >= threshold     7   128
**
******************************************************************************/

static const char *qcalignmentKQueryCoveragePropertiesProteinGenome[] =
{
    "No Alignment",
    "Alignment",
    "C shorter",
    "N shorter",
    "C tolerance",
    "N tolerance",
    "C perfect",
    "N perfect",
    "Identity Threshold",
    "Coverage Threshold",
    "Sum",
    (const char *) NULL
};
#endif /* AJFALSE */




#if AJFALSE
/* @conststatic qcalignmentKQueryCoveragePropertiesProteinProtein *************
**
** Protein to Protein Query Coverage Scoring Schema
**
** The query coverage score reflects individual alignment properties by
** adding 2**n, where n increases with the quality of the alignment. The
** following alignment properties are currently scored for query and
** target sequences of type 'protein'.
**
**          N-terminus                          C-terminus
**
**          2**n score                          2**n score
**            -     0   -                         -     0
**            0     1   alignment                 0     1
**            -     0   non-matching region       -     0
**            2     4   shorter region            1     2
**            4    16   longer region             3     8
**            6    64   non-matching start/stop   5    32
**            8   256   missing start/stop        7   128
**           10  1024   added start/stop          9   512
**           12  4096   perfect                  11  2048
**           13  8192   identity >= threshold    13  8192
**
******************************************************************************/

static const char *qcalignmentKQueryCoveragePropertiesProteinProtein[] =
{
    "No Alignment",
    "Alignment",
    "C shorter",
    "N shorter",
    "C longer",
    "N longer",
    "C non-matching stop",
    "N non-matching start",
    "C missing stop",
    "N missing start",
    "C added stop",
    "N added start",
    "C perfect",
    "N perfect",
    "Identity Threshold",
    "Sum",
    (const char *) NULL
};
#endif /* AJFALSE */




#if AJFALSE
/* @conststatic qcalignmentKQueryCoveragePropertiesQueryQuery *****************
**
** Query to Query Comparison Scoring Schema
**
** The query comparison score reflects individual alignment properties by
** adding 2**n, where n increases with the quality of the alignment. The
** following alignment properties are currently scored for query
** sequences of type 'protein'.
**
**          5'-terminus                         3'-terminus
**          N-terminus                          C-terminus
**
**          2**n score                          2**n score
**            -     0   Alignment is missing      -     0
**            0     1   First alignment           0     1
**            1     2   Second Alignment          1     2
**            3     8   first query shorter       2     4
**            5    32   first query longer        4    16
**            7   128   equal                     6    64
**            9   512   perfect                   8   256
**           10  1024   sum                      10  1024
**
******************************************************************************/

static const char *qcalignmentKQueryCoveragePropertiesQueryQuery[] =
{
    "No Alignment",
    "Genome Alignment",
    "Test Alignment",
    "3' shorter",
    "5' shorter",
    "3' longer",
    "5' longer",
    "3' equal",
    "5' equal",
    "3' perfect",
    "5' perfect",
    "Sum",
    (const char *) NULL
};
#endif /* AJFALSE */




/* @conststatic qcalignmentadaptorKTables *************************************
**
** Array of Ensembl Quality Check Alignment Adaptor SQL table names
**
******************************************************************************/

static const char *qcalignmentadaptorKTables[] =
{
    "alignment",
    (const char *) NULL
};




/* @conststatic qcalignmentadaptorKColumns ************************************
**
** Array of Ensembl Quality Check Alignment Adaptor SQL column names
**
******************************************************************************/

static const char *qcalignmentadaptorKColumns[] =
{
    "alignment.alignment_id",
    "alignment.analysis_id",
    "alignment.query_db_id",
    "alignment.query_id",
    "alignment.query_start",
    "alignment.query_end",
    "alignment.query_strand",
    "alignment.target_db_id",
    "alignment.target_id",
    "alignment.target_start",
    "alignment.target_end",
    "alignment.target_strand",
    "alignment.splice_strand",
    "alignment.score",
    "alignment.identity",
    "alignment.vulgar_line",
    "alignment.coverage",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool qcalignmentadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcas);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensqcalignment ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcalignment] Ensembl Quality Check Alignment *************
**
** @nam2rule Qcalignment Functions for manipulating
** Ensembl Quality Check Alignment objects
**
** @cc Bio::EnsEMBL::QC::Alignment
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Alignment by pointer.
** It is the responsibility of the user to first destroy any previous
** Quality Check Alignment. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Ini qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini qsequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @argrule Ini qstart [ajuint] Query start
** @argrule Ini qend [ajuint] Query end
** @argrule Ini qstrand [ajint] Query strand
** @argrule Ini tsequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule Ini tstart [ajuint] Target start
** @argrule Ini tend [ajuint] Target end
** @argrule Ini tstrand [ajint] Target strand
** @argrule Ini sstrand [ajint] Splice strand
** @argrule Ini coverage [ajuint] Coverage score
** @argrule Ini score [double] Score
** @argrule Ini identity [float] Identity
** @argrule Ini vulgar [AjPStr] Vulgar line
** @argrule Ref qca [EnsPQcalignment] Ensembl Quality Check Alignment
**
** @valrule * [EnsPQcalignment] Ensembl Quality Check Alignment or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcalignmentNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [EnsPQcalignment] Ensembl Quality Check Alignment or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcalignment ensQcalignmentNewCpy(const EnsPQcalignment qca)
{
    EnsPQcalignment pthis = NULL;

    if (!qca)
        return NULL;

    AJNEW0(pthis);

    pthis->Use              = 1U;
    pthis->Identifier       = qca->Identifier;
    pthis->Adaptor          = qca->Adaptor;
    pthis->Analysis         = ensAnalysisNewRef(qca->Analysis);
    pthis->QuerySequence    = ensQcsequenceNewRef(qca->QuerySequence);
    pthis->QueryStart       = qca->QueryStart;
    pthis->QueryEnd         = qca->QueryEnd;
    pthis->QueryStrand      = qca->QueryStrand;
    pthis->TargetSequence   = ensQcsequenceNewRef(qca->TargetSequence);
    pthis->TargetStart      = qca->TargetStart;
    pthis->TargetEnd        = qca->TargetEnd;
    pthis->TargetStrand     = qca->TargetStrand;
    pthis->Splicestrand     = qca->Splicestrand;
    pthis->Coverage         = qca->Coverage;
    pthis->Score            = qca->Score;
    pthis->Identity         = qca->Identity;

    if (qca->Vulgar)
        pthis->Vulgar = ajStrNewRef(qca->Vulgar);

    return pthis;
}




/* @func ensQcalignmentNewIni *************************************************
**
** Constructor for an Ensembl Quality Check Alignment with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::Alignment::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] qsequence [EnsPQcsequence] Query Ensembl Quality Check Sequence
** @param [r] qstart [ajuint] Query start
** @param [r] qend [ajuint] Query end
** @param [r] qstrand [ajint] Query strand
** @param [u] tsequence [EnsPQcsequence] Target Ensembl Quality Check Sequence
** @param [r] tstart [ajuint] Target start
** @param [r] tend [ajuint] Target end
** @param [r] tstrand [ajint] Target strand
** @param [r] sstrand [ajint] Splice strand
** @param [r] coverage [ajuint] Coverage score
** @param [r] score [double] Score
** @param [r] identity [float] Identity
** @param [u] vulgar [AjPStr] Vulgar line
**
** @return [EnsPQcalignment] Ensembl Quality Check Alignment or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcalignment ensQcalignmentNewIni(EnsPQcalignmentadaptor qcaa,
                                     ajuint identifier,
                                     EnsPAnalysis analysis,
                                     EnsPQcsequence qsequence,
                                     ajuint qstart,
                                     ajuint qend,
                                     ajint qstrand,
                                     EnsPQcsequence tsequence,
                                     ajuint tstart,
                                     ajuint tend,
                                     ajint tstrand,
                                     ajint sstrand,
                                     ajuint coverage,
                                     double score,
                                     float identity,
                                     AjPStr vulgar)
{
    EnsPQcalignment qca = NULL;

    if (!analysis)
        return NULL;

    if (!qsequence)
        return NULL;

    if (!tsequence)
        return NULL;

    AJNEW0(qca);

    qca->Use            = 1U;
    qca->Identifier     = identifier;
    qca->Adaptor        = qcaa;
    qca->Analysis       = ensAnalysisNewRef(analysis);
    qca->QuerySequence  = ensQcsequenceNewRef(qsequence);
    qca->QueryStart     = qstart;
    qca->QueryEnd       = qend;
    qca->QueryStrand    = qstrand;
    qca->TargetSequence = ensQcsequenceNewRef(tsequence);
    qca->TargetStart    = tstart;
    qca->TargetEnd      = tend;
    qca->TargetStrand   = tstrand;
    qca->Splicestrand   = sstrand;
    qca->Coverage       = coverage;
    qca->Score          = score;
    qca->Identity       = identity;
    qca->Vulgar         = ajStrNewRef(vulgar);

    return qca;
}




/* @func ensQcalignmentNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qca [EnsPQcalignment] Ensembl Alignment
**
** @return [EnsPQcalignment] Ensembl Quality Check Alignment or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcalignment ensQcalignmentNewRef(EnsPQcalignment qca)
{
    if (!qca)
        return NULL;

    qca->Use++;

    return qca;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check Alignment object.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Alignment
**
** @argrule * Pqca [EnsPQcalignment*] Ensembl Quality Check Alignment address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcalignmentDel ****************************************************
**
** Default destructor for an Ensembl Quality Check Alignment.
**
** @param [d] Pqca [EnsPQcalignment*] Ensembl Quality Check Alignment address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcalignmentDel(EnsPQcalignment *Pqca)
{
    EnsPQcalignment pthis = NULL;

    if (!Pqca)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensQcalignmentDel"))
    {
        ajDebug("ensQcalignmentDel\n"
                "  *Pqca %p\n",
                *Pqca);

        ensQcalignmentTrace(*Pqca, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pqca)
        return;

    pthis = *Pqca;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pqca = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ensQcsequenceDel(&pthis->QuerySequence);
    ensQcsequenceDel(&pthis->TargetSequence);

    ajStrDel(&pthis->Vulgar);

    AJFREE(pthis);

    *Pqca = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check Alignment object.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Get Return Quality Check Alignment attribute(s)
** @nam4rule Adaptor Return the Ensembl Quality Check Alignment Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Coverage Return the coverage
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Identity Return the identity
** @nam4rule Query Return query attribute(s)
** @nam5rule End Return the query end
** @nam5rule Sequence Return the query sequence
** @nam5rule Start Return the query start
** @nam5rule Strand Return the query strand
** @nam4rule Score Return the score
** @nam4rule Splicestrand Return the splice strand
** @nam4rule Target Return target attribute(s)
** @nam5rule End Return the query end
** @nam5rule Sequence Return the query sequence
** @nam5rule Start Return the query start
** @nam5rule Strand Return the query strand
** @nam4rule Vulgar Return the Vulgar line
**
** @argrule * qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @valrule Adaptor [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Coverage [ajuint] Coverage score or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Identity [float] Identity or 0.0F
** @valrule QueryEnd [ajuint] Query end or 0U
** @valrule QuerySequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence or NULL
** @valrule QueryStart [ajuint] Query start or 0U
** @valrule QueryStrand [ajint] Query strand or 0
** @valrule Score [double] Score or 0.0
** @valrule Splicestrand [ajint] Splice strand or 0
** @valrule TargetEnd [ajuint] Target end or 0U
** @valrule TargetSequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence or NULL
** @valrule TargetStart [ajuint] Target start or 0U
** @valrule TargetStrand [ajint] Target strand or 0
** @valrule Vulgar [AjPStr] Vulgar line or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentGetAdaptor *********************************************
**
** Get the Ensembl Quality Check Alignment Adaptor member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [EnsPQcalignmentadaptor] Ensembl Quality Check Alignment Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcalignmentadaptor ensQcalignmentGetAdaptor(const EnsPQcalignment qca)
{
    return (qca) ? qca->Adaptor : NULL;
}




/* @func ensQcalignmentGetAnalysis ********************************************
**
** Get the Ensembl Analysis member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::analysis
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensQcalignmentGetAnalysis(const EnsPQcalignment qca)
{
    return (qca) ? qca->Analysis : NULL;
}




/* @func ensQcalignmentGetCoverage ********************************************
**
** Get the coverage score member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::coverage
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajuint] Coverage score or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcalignmentGetCoverage(const EnsPQcalignment qca)
{
    return (qca) ? qca->Coverage : 0U;
}




/* @func ensQcalignmentGetIdentifier ******************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcalignmentGetIdentifier(const EnsPQcalignment qca)
{
    return (qca) ? qca->Identifier : 0U;
}




/* @func ensQcalignmentGetIdentity ********************************************
**
** Get the identity member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::identity
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [float] Identity or 0.0F
**
** @release 6.2.0
** @@
******************************************************************************/

float ensQcalignmentGetIdentity(const EnsPQcalignment qca)
{
    return (qca) ? qca->Identity : 0.0F;
}




/* @func ensQcalignmentGetQueryEnd ********************************************
**
** Get the query end member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_end
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajuint] Query end or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcalignmentGetQueryEnd(const EnsPQcalignment qca)
{
    return (qca) ? qca->QueryEnd : 0U;
}




/* @func ensQcalignmentGetQuerySequence ***************************************
**
** Get the query Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_sequence
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [EnsPQcsequence] Query Ensembl Quality Check Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcsequence ensQcalignmentGetQuerySequence(const EnsPQcalignment qca)
{
    return (qca) ? qca->QuerySequence : NULL;
}




/* @func ensQcalignmentGetQueryStart ******************************************
**
** Get the query start member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_start
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajuint] Query start or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcalignmentGetQueryStart(const EnsPQcalignment qca)
{
    return (qca) ? qca->QueryStart : 0U;
}




/* @func ensQcalignmentGetQueryStrand *****************************************
**
** Get the query strand member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_strand
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajint] Query strand or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensQcalignmentGetQueryStrand(const EnsPQcalignment qca)
{
    return (qca) ? qca->QueryStrand : 0;
}




/* @func ensQcalignmentGetScore ***********************************************
**
** Get the score member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::score
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [double] Score or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensQcalignmentGetScore(const EnsPQcalignment qca)
{
    return (qca) ? qca->Score : 0.0;
}




/* @func ensQcalignmentGetSplicestrand ****************************************
**
** Get the splice strand member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::splice_strand
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajint] Splice strand or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensQcalignmentGetSplicestrand(const EnsPQcalignment qca)
{
    return (qca) ? qca->Splicestrand : 0;
}




/* @func ensQcalignmentGetTargetEnd *******************************************
**
** Get the target end member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_end
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajuint] Target end or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcalignmentGetTargetEnd(const EnsPQcalignment qca)
{
    return (qca) ? qca->TargetEnd : 0U;
}




/* @func ensQcalignmentGetTargetSequence **************************************
**
** Get the target Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_sequence
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [EnsPQcsequence] Target Ensembl Quality Check Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcsequence ensQcalignmentGetTargetSequence(const EnsPQcalignment qca)
{
    return (qca) ? qca->TargetSequence : NULL;
}




/* @func ensQcalignmentGetTargetStart *****************************************
**
** Get the target start member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_start
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajuint] Target start or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcalignmentGetTargetStart(const EnsPQcalignment qca)
{
    return (qca) ? qca->TargetStart : 0U;
}




/* @func ensQcalignmentGetTargetStrand ****************************************
**
** Get the target strand member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_strand
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [ajint] Target strand or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensQcalignmentGetTargetStrand(const EnsPQcalignment qca)
{
    return (qca) ? qca->TargetStrand : 0;
}




/* @func ensQcalignmentGetVulgar **********************************************
**
** Get the Vulgar line member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::vulgar_line
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [AjPStr] Vulgar line or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensQcalignmentGetVulgar(const EnsPQcalignment qca)
{
    return (qca) ? qca->Vulgar : NULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an
** Ensembl Quality Check Alignment object.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Set Set one member of an Ensembl Quality Check Alignment
** @nam4rule Adaptor Set the Ensembl Quality Check Alignment Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Coverage Set the coverage score
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Identity Set the identity
** @nam4rule Query Set query attribute(s)
** @nam5rule End Set the query end
** @nam5rule Sequence Set the query Ensembl Quality Check Sequence
** @nam5rule Start Set the query start
** @nam5rule Strand Set the query strand
** @nam4rule Score Set the score
** @nam4rule Splicestrand Set the splice strand
** @nam4rule Target Set target member(s)
** @nam5rule End Set the target end
** @nam5rule Sequence Set the target Ensembl Quality Check Sequence
** @nam5rule Start Set the target start
** @nam5rule Strand Set the target strand
** @nam4rule Vulgar Set the Vulgar line
**
** @argrule * qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Adaptor qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Coverage coverage [ajuint] Coverage score
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Identity identity [float] Score
** @argrule QueryEnd qend [ajuint] Query end
** @argrule QuerySequence qsequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @argrule QueryStart qstart [ajuint] Query start
** @argrule QueryStrand qstrand [ajint] Query strand
** @argrule Score score [double] Score
** @argrule Splicestrand sstrand [ajint] Splice strand
** @argrule TargetEnd tend [ajuint] Target end
** @argrule TargetSequence tsequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule TargetStart tstart [ajuint] Target start
** @argrule TargetStrand tstrand [ajint] Target strand
** @argrule Vulgar vulgar [AjPStr] Vulgar line
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcalignmentSetAdaptor *********************************************
**
** Set the Ensembl Quality Check Alignment Adaptor member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetAdaptor(EnsPQcalignment qca,
                                EnsPQcalignmentadaptor qcaa)
{
    if (!qca)
        return ajFalse;

    qca->Adaptor = qcaa;

    return ajTrue;
}




/* @func ensQcalignmentSetAnalysis ********************************************
**
** Set the Ensembl Analysis member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::analysis
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [uN] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetAnalysis(EnsPQcalignment qca,
                                 EnsPAnalysis analysis)
{
    if (!qca)
        return ajFalse;

    ensAnalysisDel(&qca->Analysis);

    qca->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcalignmentSetCoverage ********************************************
**
** Set the coverage score member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::coverage
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] coverage [ajuint] Coverage score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetCoverage(EnsPQcalignment qca,
                                 ajuint coverage)
{
    if (!qca)
        return ajFalse;

    qca->Coverage = coverage;

    return ajTrue;
}




/* @func ensQcalignmentSetIdentifier ******************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetIdentifier(EnsPQcalignment qca,
                                   ajuint identifier)
{
    if (!qca)
        return ajFalse;

    qca->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcalignmentSetIdentity ********************************************
**
** Set the identity member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::identity
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identity [float] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetIdentity(EnsPQcalignment qca,
                                 float identity)
{
    if (!qca)
        return ajFalse;

    qca->Identity = identity;

    return ajTrue;
}




/* @func ensQcalignmentSetQueryEnd ********************************************
**
** Set the query end member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_end
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] qend [ajuint] Query end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetQueryEnd(EnsPQcalignment qca,
                                 ajuint qend)
{
    if (!qca)
        return ajFalse;

    qca->QueryEnd = qend;

    return ajTrue;
}




/* @func ensQcalignmentSetQuerySequence ***************************************
**
** Set the query Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_sequence
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [uN] qsequence [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetQuerySequence(EnsPQcalignment qca,
                                      EnsPQcsequence qsequence)
{
    if (!qca)
        return ajFalse;

    ensQcsequenceDel(&qca->QuerySequence);

    qca->QuerySequence = ensQcsequenceNewRef(qsequence);

    return ajTrue;
}




/* @func ensQcalignmentSetQueryStart ******************************************
**
** Set the query start member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_start
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] qstart [ajuint] Query start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetQueryStart(EnsPQcalignment qca,
                                   ajuint qstart)
{
    if (!qca)
        return ajFalse;

    qca->QueryStart = qstart;

    return ajTrue;
}




/* @func ensQcalignmentSetQueryStrand *****************************************
**
** Set the query strand member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_strand
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] qstrand [ajint] Query strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetQueryStrand(EnsPQcalignment qca,
                                    ajint qstrand)
{
    if (!qca)
        return ajFalse;

    qca->QueryStrand = qstrand;

    return ajTrue;
}




/* @func ensQcalignmentSetScore ***********************************************
**
** Set the score member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::score
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetScore(EnsPQcalignment qca, double score)
{
    if (!qca)
        return ajFalse;

    qca->Score = score;

    return ajTrue;
}




/* @func ensQcalignmentSetSplicestrand ****************************************
**
** Set the splice strand member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::splice_strand
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] sstrand [ajint] Splice strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetSplicestrand(EnsPQcalignment qca,
                                     ajint sstrand)
{
    if (!qca)
        return ajFalse;

    qca->Splicestrand = sstrand;

    return ajTrue;
}




/* @func ensQcalignmentSetTargetEnd *******************************************
**
** Set the target end member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_end
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] tend [ajuint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetEnd(EnsPQcalignment qca,
                                  ajuint tend)
{
    if (!qca)
        return ajFalse;

    qca->TargetEnd = tend;

    return ajTrue;
}




/* @func ensQcalignmentSetTargetSequence **************************************
**
** Set the target Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_sequence
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [u] tsequence [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetSequence(EnsPQcalignment qca,
                                       EnsPQcsequence tsequence)
{
    if (!qca)
        return ajFalse;

    ensQcsequenceDel(&qca->TargetSequence);

    qca->TargetSequence = ensQcsequenceNewRef(tsequence);

    return ajTrue;
}




/* @func ensQcalignmentSetTargetStart *****************************************
**
** Set the target start member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_start
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] tstart [ajuint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetStart(EnsPQcalignment qca,
                                    ajuint tstart)
{
    if (!qca)
        return ajFalse;

    qca->TargetStart = tstart;

    return ajTrue;
}




/* @func ensQcalignmentSetTargetStrand ****************************************
**
** Set the target strand member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_strand
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] tstrand [ajint] Target strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetStrand(EnsPQcalignment qca,
                                     ajint tstrand)
{
    if (!qca)
        return ajFalse;

    qca->TargetStrand = tstrand;

    return ajTrue;
}




/* @func ensQcalignmentSetVulgar **********************************************
**
** Set the Vulgar line member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::vulgar_line
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [uN] vulgar [AjPStr] Vulgar line
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentSetVulgar(EnsPQcalignment qca,
                               AjPStr vulgar)
{
    if (!qca)
        return ajFalse;

    ajStrDel(&qca->Vulgar);

    qca->Vulgar = ajStrNewRef(vulgar);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Quality Check Alignment object.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Trace Report Ensembl Quality Check Alignment members to
**                 debug file
**
** @argrule Trace qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcalignmentTrace **************************************************
**
** Trace an Ensembl Quality Check Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentTrace(const EnsPQcalignment qca, ajuint level)
{
    AjPStr indent = NULL;

    if (!qca)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensQcalignmentTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Analysis %p\n"
            "%S  QuerySequence %p\n"
            "%S  QueryStart %u\n"
            "%S  QueryEnd %u\n"
            "%S  QueryStrand %d\n"
            "%S  TargetSequence %p\n"
            "%S  TargetStart %u\n"
            "%S  TargetEnd %u\n"
            "%S  TargetStrand %d\n"
            "%S  Splicestrand %d\n"
            "%S  Coverage %u\n"
            "%S  Score %f\n"
            "%S  Identity %f\n"
            "%S  Vulgar '%S'\n",
            indent, qca,
            indent, qca->Use,
            indent, qca->Identifier,
            indent, qca->Adaptor,
            indent, qca->Analysis,
            indent, qca->QuerySequence,
            indent, qca->QueryStart,
            indent, qca->QueryEnd,
            indent, qca->QueryStrand,
            indent, qca->TargetSequence,
            indent, qca->TargetStart,
            indent, qca->TargetEnd,
            indent, qca->TargetStrand,
            indent, qca->Splicestrand,
            indent, qca->Coverage,
            indent, qca->Score,
            indent, qca->Identity,
            indent, qca->Vulgar);

    ensAnalysisTrace(qca->Analysis, 1);

    ensQcsequenceTrace(qca->QuerySequence, 1);
    ensQcsequenceTrace(qca->TargetSequence, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Quality Check Alignment object.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Calculate   Calculate Ensembl Quality Check Alignment values
** @nam4rule Memsize     Calculate the memory size in bytes
** @nam4rule Query       Calculate query values
** @nam5rule Coordinates Calculate query coordinates
** @nam5rule Coverage    Calculate the query coverage
** @nam6rule Dna         Calculate the query coverage DNA values
** @nam7rule Dna         Calculate the query coverage DNA to DNA
** @nam7rule Genome      Calculate the query coverage DNA to Genome
** @nam6rule Protein     Calculate the query coverage Protein values
** @nam7rule Protein     Calculate the query coverage Protein to Protein
** @nam6rule Query       Calculate the query coverage Query values
** @nam7rule Query       Calculate the query coverage Query to Query
** @nam7rule Target      Calculate the query coverage Query to Target
** @nam4rule Target      Calculate target values
** @nam5rule Coordinates Calculate target coordinates
**
** @argrule Memsize qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule QueryCoordinates qca [const EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule QueryCoordinates Pstart [ajint*] Start
** @argrule QueryCoordinates Pend [ajint*] End
** @argrule QueryCoordinates Pstrand [ajint*] Strand
** @argrule QueryCoordinates Plength [ajuint*] Length
** @argrule QueryCoverageDnaDna qca [EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule QueryCoverageDnaDna identity [float] Identity threshold
** @argrule QueryCoverageDnaDna edge [ajuint] Edge threshold
** @argrule QueryCoverageDnaGenome qca [EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule QueryCoverageDnaGenome identity [float] Identity threshold
** @argrule QueryCoverageDnaGenome edge [ajuint] Edge threshold
** @argrule QueryCoverageProteinGenome qca [EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule QueryCoverageProteinGenome identity [float] Identity threshold
** @argrule QueryCoverageProteinGenome edge [ajuint] Edge threshold
** @argrule QueryCoverageProteinProtein qca [EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule QueryCoverageProteinProtein identity [float] Identity threshold
** @argrule QueryCoverageQueryQuery qca1 [const EnsPQcalignment]
** First Ensembl Quality Check Alignment
** @argrule QueryCoverageQueryQuery qca2 [const EnsPQcalignment]
** Second Ensembl Quality Check Alignment
** @argrule QueryCoverageQueryQuery Pscore [ajuint*] Coverage score address
** @argrule QueryCoverageQueryTarget qca [EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule QueryCoverageQueryTarget identity [float] Identity threshold
** @argrule QueryCoverageQueryTarget edge [ajuint] Edge threshold
** @argrule TargetCoordinates qca [const EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule TargetCoordinates Pstart [ajint*] Start
** @argrule TargetCoordinates Pend [ajint*] End
** @argrule TargetCoordinates Pstrand [ajint*] Strand
** @argrule TargetCoordinates Plength [ajuint*] Length
**
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule QueryCoordinates [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule QueryCoverage [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule TargetCoordinates [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcalignmentCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an Ensembl Quality Check Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensQcalignmentCalculateMemsize(const EnsPQcalignment qca)
{
    size_t size = 0;

    if (!qca)
        return 0;

    size += sizeof (EnsOQcalignment);

    size += ensAnalysisCalculateMemsize(qca->Analysis);

    size += ensQcsequenceCalculateMemsize(qca->QuerySequence);
    size += ensQcsequenceCalculateMemsize(qca->TargetSequence);

    return size;
}




/* @func ensQcalignmentCalculateQueryCoordinates ******************************
**
** Calculate query coordinates of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_coordinates
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @param [wP] Pstart [ajint*] Start
** @param [wP] Pend [ajint*] End
** @param [wP] Pstrand [ajint*] Strand
** @param [wP] Plength [ajuint*] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoordinates(
    const EnsPQcalignment qca,
    ajint *Pstart,
    ajint *Pend,
    ajint *Pstrand,
    ajuint *Plength)
{
    if (!qca)
        return ajFalse;

    if (!Pstart)
        return ajFalse;

    if (!Pend)
        return ajFalse;

    if (!Pstrand)
        return ajFalse;

    if (!Plength)
        return ajFalse;

    if (qca->QueryStrand >= 0)
    {
        *Pstart = qca->QueryStart;
        *Pend   = qca->QueryEnd;
    }
    else
    {
        *Pstart = qca->QueryEnd;
        *Pend   = qca->QueryStart;
    }

    *Pstrand = qca->QueryStrand;

    /* In Exonerate coordinates the length is just end - start. */

    *Plength = *Pend - *Pstart;

    return ajTrue;
}




/* @func ensQcalignmentCalculateQueryCoverageDnaDna ***************************
**
** Calculates the alignment coverage score for a query of type 'dna'
** against a target of type 'dna' and sets the score in the coverage
** member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::d2d_query_coverage
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold to classify cases where only few
**                          residues at the edge are different.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** DNA to DNA Query Coverage Criteria
**
**   5'-Terminus Criteria and Scores            2**n score
**
**    QS==0 & TS==0 ==> perfect                  12  4096
**    QS==0 & TS<=E ==> longer edge              10  1024
**    QS==0 & TS>E  ==> longer region             4    16
**    QS<=E & TS==0 ==> shorter edge              8   256
**    QS<=E & TS<=E ==> non-matching edge         6    64
**    QS<=E & TS>E  ==> non-matching region       -     0
**    QS>E  & TS==0 ==> shorter region            2     4
**    QS>E  & TS>0  ==> non-matching region       -     0
**
**  3'-Terminus Criteria and Scores             2**n  score
**
**    QE>=QL   & TE>=TL   ==> perfect             11   2048
**    QE>=QL   & TE>=TL-E ==> longer edge          9    512
**    QE>=QL   & TE<TL-E  ==> longer region        3      8
**    QE>=QL-E & TE>=TL   ==> shorter edge         7    128
**    QE>=QL-E & TE>=TL-E ==> non-matching edge    5     32
**    QE>=QL-E & TE<TL-E  ==> non-matching region  -      0
**    QE<QL-E  & TE>=TL   ==> shorter region       1      2
**    QE<QL-E  & TE<TL    ==> non-matching         -      0
**
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoverageDnaDna(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    ajuint qlength = 0U;
    ajuint tlength = 0U;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;
    ajint tstart  = 0;
    ajint tend    = 0;
    ajint tstrand = 0;

    if (!qca)
        return ajFalse;

    /* An Alignment object without a target automatically has coverage 0. */

    qca->Coverage = 0U;

    if (!qca->TargetSequence)
        return ajTrue;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentCalculateQueryCoordinates(qca,
                                            &qstart,
                                            &qend,
                                            &qstrand,
                                            &qlength);

    ensQcalignmentCalculateTargetCoordinates(qca,
                                             &tstart,
                                             &tend,
                                             &tstrand,
                                             &tlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);
    tlength = ensQcsequenceGetLength(qca->TargetSequence);

    /* Correct sequence lengths for PolyA+ tails. */

    qlength -= ensQcsequenceGetPolya(qca->QuerySequence);
    tlength -= ensQcsequenceGetPolya(qca->TargetSequence);

    /* Alignment (1*2**0=1) */
    qca->Coverage += 1U;

    /* Test for completeness of the 5'-terminus. */

    if (qstart == 0)
    {
        /* Perfect 5'-terminus (1*2**12=4096) */
        if (tstart == 0)
            qca->Coverage += 4096U;
        /* Longer 5'-edge (1*2**10=1024) */
        else if (tstart <= (ajint) edge)
            qca->Coverage += 1024U;
        /* Longer 5'-region (1*2**4=16) */
        else if (tstart > (ajint) edge)
            qca->Coverage += 16U;
        else
            ajWarn("Error in 5'-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if (qstart <= (ajint) edge)
    {
        /* Shorter 5'-edge (1*2**8=256) */
        if (tstart == 0)
            qca->Coverage += 256U;
        /* Non-matching 5'-edge (1*2**6=64) */
        else if (tstart <= (ajint) edge)
            qca->Coverage += 64U;
        /* Non-matching 5'-region (0*2**0=0) */
        else if (tstart > (ajint) edge)
            qca->Coverage += 0U;
        else
            ajWarn("Error in 5'-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if (qstart > (ajint) edge)
    {
        /* Shorter 5'-region (1*2**2=4) */
        if (tstart == 0)
            qca->Coverage += 4U;
        /* Non-matching 5'-region (0*2**0=0) */
        else if (tstart > 0)
            qca->Coverage += 0U;
        else
            ajWarn("Error in 5'-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else
        ajWarn("Error in 5'-terminus query coverage scoring schema. "
               "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);

    /*
    ** Test for completeness of the 3'-terminus.  As the PolyA tail
    ** detection algorithm could truncate As from the end of an mRNA,
    ** which were still represented by the genome, a perfect query end
    ** could be longer than the artificially truncated query length.
    **
    ** The same is true for the target length so that for each query case
    ** another condition testing for a target end exceeding the
    ** target length is required. Currently, scores as in the case of
    ** perfect overlaps are set.
    */

    if (qend >= (ajint) qlength)
    {
        /*
        ** Perfect 3'-terminus, including over-estimated Poly A tails.
        ** (1*2**11=2048)
        */
        if (tend >= (ajint) tlength)
            qca->Coverage += 2048U;
        /* Longer 3'-edge (1*2**9=512) */
        else if (tend >= (ajint) (tlength - edge))
            qca->Coverage += 512U;
        /* Longer 3'-region (1*2**3=8) */
        else if (tend < (ajint) (tlength - edge))
            qca->Coverage += 8U;
        else
            ajWarn("Error in 3'-terminus query coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else if (qend >= (ajint) (qlength - edge))
    {
        /*
        ** Added 3'-edge, including over-estimated Poly A tails.
        ** (1*2**7=128)
        */
        if (tend >= (ajint) tlength)
            qca->Coverage += 128U;
        /* Non-matching 3'-edge (1*2**5=32) */
        else if (tend >= (ajint) (tlength - edge))
            qca->Coverage += 32U;
        /* Non-matching 3-'terminus (0*2**2=0) */
        else if (tend < (ajint) (tlength - edge))
            qca->Coverage += 0U;
        else
            ajWarn("Error in 3'-terminus query coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else if (qend < (ajint) (qlength - edge))
    {
        /* Shorter 3'-terminus (1*2**1=2) */
        if (tend >= (ajint) tlength)
            qca->Coverage += 2U;
        /* Non-matching 3'-terminus (0*2**0=0) */
        else if (tend < (ajint) tlength)
            qca->Coverage += 0U;
        else
            ajWarn("Error in 3'-terminus query coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else
        ajWarn("Error in 3'-terminus query coverage scoring schema. "
               "ID: %u QE: %d QL: %u TE: %d TL: %u",
               qca->Identifier, qend, qlength, tend, tlength);

    /*
    ** Test for sequence identity over the identity threshold value
    ** only for perfect 5' and 3'-termini.
    ** (1*2**12 + 1*2**11 = 4096 + 2048 = 6144)
    ** Identity threshold value (1*2**13=8192).
    */

    if ((qca->Coverage & 6144U) && (qca->Identity >= identity))
        qca->Coverage += 8192U;

    return ajTrue;
}




/* @func ensQcalignmentCalculateQueryCoverageDnaGenome ************************
**
** Calculates the alignment coverage score for a query of type 'dna'
** against a target of class 'genome' and sets the score in the coverage
** member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::d2g_query_coverage
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold to classify cases where only few
**                          residues at the edge fall into a separate exon.
**                          This is generally difficult to predict for
**                          alignment programs.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** cDNA to Genome Query Coverage Criteria
**
**  5'-Terminus Criteria and Scores            2**n score
**
**    QS=0         ==> perfect query              6    64
**    QS<=TE       ==> threshold edge             4    16
**    QS>TE        ==> shorter query              2     4
**
**  3'-Terminus Criteria and Scores            2**n score
**
**    QE=QL        ==> perfect target             5    32
**    QE=>QL-TE    ==> threshold edge             3     8
**    QE<QL        ==> shorter query              1     2
**
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoverageDnaGenome(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    ajuint qlength = 0U;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;

    if (!qca)
        return ajFalse;

    /* An Alignment object without a target automatically has coverage 0. */

    qca->Coverage = 0U;

    if (!qca->TargetSequence)
        return ajTrue;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentCalculateQueryCoordinates(qca,
                                            &qstart,
                                            &qend,
                                            &qstrand,
                                            &qlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);

    qlength -= ensQcsequenceGetPolya(qca->QuerySequence);

    /* Alignment (1*2**0=1) */
    qca->Coverage += 1U;

    /* Test for completeness of the 5' terminus. */

    /* Perfect 5' terminus (1*2**6=64) */
    if (qstart == 0)
        qca->Coverage += 64U;
    /* Edge threshold (1*2**4=16) */
    else if (qstart <= (ajint) edge)
        qca->Coverage += 16U;
    /* Shorter 5' terminus (1*2**2=4) */
    else if (qstart > (ajint) edge)
        qca->Coverage += 4U;
    else
        ajWarn("Error in 5'-terminus query coverage scoring schema. "
               "ID: %u QS: %d", qca->Identifier, qstart);

    /*
    ** Test for completeness of the 3' terminus.  If part of a PolyA tail
    ** aligns to the genome sequence it may appear longer so that the end
    ** of the alignment actually extends the clipped mRNA sequence.
    */

    /* Perfect 3' terminus (1*2**5=32) */
    if (qend >= (ajint) qlength)
        qca->Coverage += 32U;
    /* Edge threshold (1*2**3=8) */
    else if (qend >= (ajint) (qlength - edge))
        qca->Coverage += 8U;
    /* 3' terminus shorter (1*2**1=2) */
    else if (qend < (ajint) (qlength - edge))
        qca->Coverage += 2U;
    else
        ajWarn("Error in 3'-terminus coverage scoring schema. "
               "ID: %u QE: %d QL: %u",
               qca->Identifier, qend, qlength);

    /*
    ** Test for sequence identity over the 'threshold_identity' value
    ** only for perfect 5' and 3'-termini.
    ** (1*2**6 + 1*2**5 = 32 + 64 = 96)
    ** Identity threshold value (1*2**7=128).
    */

    if ((qca->Coverage & 96U) && (qca->Identity >= identity))
        qca->Coverage += 128U;

    return ajTrue;
}




/* @func ensQcalignmentCalculateQueryCoverageProteinGenome ********************
**
** Calculates the alignment coverage score for a query of type 'protein'
** against a target of class 'genome' and sets the score in the coverage
** member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::p2g_query_coverage
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** Protein to Genome Query Coverage Criteria
**
**   N-Terminus Criteria and Scores             2**n score
**
**     QS=0         ==> perfect                    6    64
**     QS<=TE       ==> edge threshold             4    16
**     QS>TE        ==> shorter                    2     4
**
**   C-Terminus Criteria and Scores             2**n score
**
**     QE=QL        ==> perfect                    5    32
**     QE=>QL-TE    ==> edge threshold             3     8
**     QE<QL        ==> shorter                    1     2
**
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoverageProteinGenome(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    ajuint qlength = 0U;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;

    if (!qca)
        return ajFalse;

    /* An Alignment object without a target automatically has coverage 0. */

    qca->Coverage = 0U;

    if (!qca->TargetSequence)
        return ajTrue;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentCalculateQueryCoordinates(qca,
                                            &qstart,
                                            &qend,
                                            &qstrand,
                                            &qlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);

    /* Alignment (1*2**0=1) */
    qca->Coverage += 1U;

    /* Test for completeness of the N-terminus. */

    /* Perfect N-terminus (1*2**6=64) */
    if (qstart == 0)
        qca->Coverage += 64U;
    /* Edge threshold (1*2**4=16) */
    else if (qstart <= (ajint) edge)
        qca->Coverage += 16U;
    /* Shorter N-terminus (1*2**2=4) */
    else if (qstart > (ajint) edge)
        qca->Coverage += 4U;
    else
        ajWarn("Error in N-terminus query coverage scoring schema. "
               "ID: %d QS: %d", qca->Identifier, qstart);

    /* Test for completeness of the C-terminus. */

    /* Perfect C-terminus (1*2**5=32) */
    if (qend == (ajint) qlength)
        qca->Coverage += 32U;
    /* Edge threshold (1*2**3=8) */
    else if (qend >= (ajint) (qlength - edge))
        qca->Coverage += 8U;
    /* Shorter C-terminus (1*2**1=2) */
    else if (qend < (ajint) (qlength - edge))
        qca->Coverage += 2U;
    else
        ajWarn("Error in C-terminus coverage scoring schema. "
               "ID: %u QE: %d QL: %u", qca->Identifier, qend, qlength);

    /*
    ** Test for sequence identity over the identity threshold value
    ** only for perfect N and C-termini.
    ** (1*2**6 + 1*2**5 = 64 + 32 = 96)
    ** Identity threshold value (1*2**7=128)
    */

    if ((qca->Coverage & 96U) && (qca->Identity >= identity))
        qca->Coverage += 128U;

    return ajTrue;
}




/* @func ensQcalignmentCalculateQueryCoverageProteinProtein *******************
**
** Calculates the alignment coverage score for a query of type 'protein'
** against a target of type 'protein' and sets the score in the coverage
** member of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::p2p_query_coverage
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identity [float] Identity threshold
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** Protein to Protein Query Coverage Criteria
**
**   N-Terminus Criteria and Scores             2**n score
**
**     QS==0   & TS==0  ==> perfect               12  4096
**     QS==0   & TS==1  ==> added start           10  1024
**     QS==0   & TS> 1  ==> longer region          4    16
**     QS==1   & TS==0  ==> missing start          8   256
**     QS==1   & TS==1  ==> non-matching start     6    64
**     QS==1   & TS> 1  ==> non-matching region    -     0
**     QS> 1   & TS==0  ==> shorter region         2     4
**     QS> 1   & TS> 0  ==> non-matching region    -     0
**
**   C-Terminus Criteria and Scores             2**n score
**
**     QE=QL   & TE=TL   ==> perfect              11  2048
**     QE=QL   & TE=TL-1 ==> added stop            9   512
**     QE=QL   & TE<TL-1 ==> longer region         3     8
**     QE=QL-1 & TE=TL   ==> missing stop          7   128
**     QE=QL-1 & TE=TL-1 ==> non-matching stop     5    32
**     QE=QL-1 & TE<TL-1 ==> non-matching region   -     0
**     QE<QL-1 & TE=TL   ==> shorter region        1     2
**     QE<QL-1 & TE<TL   ==> non-matching region   -     0
**
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoverageProteinProtein(
    EnsPQcalignment qca,
    float identity)
{
    ajuint qlength = 0U;
    ajuint tlength = 0U;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;
    ajint tstart  = 0;
    ajint tend    = 0;
    ajint tstrand = 0;

    if (!qca)
        return ajFalse;

    /* An Alignment object without a target automatically has coverage 0. */

    qca->Coverage = 0U;

    if (!qca->TargetSequence)
        return ajTrue;

    /* Alignment (1*2**0=1) */
    qca->Coverage += 1U;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentCalculateQueryCoordinates(qca,
                                            &qstart,
                                            &qend,
                                            &qstrand,
                                            &qlength);

    ensQcalignmentCalculateTargetCoordinates(qca,
                                             &tstart,
                                             &tend,
                                             &tstrand,
                                             &tlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);
    tlength = ensQcsequenceGetLength(qca->TargetSequence);

    /* Test for completeness of the N-terminus. */

    if (qstart == 0)
    {
        /* Perfect N-terminus (1*2**12=4096) */
        if (tstart == 0)
            qca->Coverage += 4096U;
        /* Added start codon (1*2**10=1024) */
        else if (tstart == 1)
            qca->Coverage += 1024U;
        /* Longer N-terminus (1*2**4=16) */
        else if (tstart > 1)
            qca->Coverage += 16U;
        else
            ajWarn("Error in N-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if (qstart == 1)
    {
        /* Missing start codon (1*2**8=256) */
        if (tstart == 0)
            qca->Coverage += 256U;
        /* Non-matching start codon (1*2**6=64) */
        else if (tstart == 1)
            qca->Coverage += 64U;
        /* Non-matching N-terminus (0*2**1=0) */
        else if (tstart > 1)
            qca->Coverage += 0U;
        else
            ajWarn("Error in N-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if (qstart > 1)
    {
        /* Shorter N-terminus (1*2**2=4) */
        if (tstart == 0)
            qca->Coverage += 4U;
        /* Non-matching N-terminus (0*2**1=0) */
        else if (tstart > 0)
            qca->Coverage += 0U;
        else
            ajWarn("Error in N-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else
        ajWarn("Error in N-terminus query coverage scoring schema. "
               "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);

    /* Test for completeness of the C-terminus. */

    if (qend == (ajint) qlength)
    {
        /* Perfect C-terminus (1*2**11=2048) */
        if (tend == (ajint) tlength)
            qca->Coverage += 2048U;
        /* Added stop codon (1*2**9=512) */
        else if (tend == (ajint) (tlength - 1U))
            qca->Coverage += 512U;
        /* Longer C-terminus (1*2**3=8) */
        else if (tend < (ajint) (tlength - 1U))
            qca->Coverage += 8U;
        else
            ajWarn("Error in C-terminus coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else if (qend == (ajint) (qlength - 1U))
    {
        /* Missing stop codon (1*2**7=128) */
        if (tend == (ajint) tlength)
            qca->Coverage += 128U;
        /* Non-matching stop codon (1*2**5=32) */
        else if (tend == (ajint) (tlength - 1U))
            qca->Coverage += 32U;
        /* Non-matching C-terminus (0*2**2=0) */
        else if (tend < (ajint) (tlength - 1U))
            qca->Coverage += 0U;
        else
            ajWarn("Error in C-terminus coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else if (qend < (ajint) (qlength - 1U))
    {
        /* Shorter C-terminus (1*2**1=2) */
        if (tend == (ajint) tlength)
            qca->Coverage += 2U;
        /* Non-matching C-terminus (0*2**2=0) */
        else if (tend < (ajint) tlength)
            qca->Coverage += 0U;
        else
            ajWarn("Error in C-terminus coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else
        ajWarn("Error in C-terminus coverage scoring schema. "
               "ID: %u QE: %d QL: %u TE: %d TL: %u",
               qca->Identifier, qend, qlength, tend, tlength);

    /*
    ** Test for sequence identity over the threshold identity value
    ** only for perfect N and C-termini.
    **
    ** (1*2**12 + 1*2**11 = 4096 + 2048 = 6144)
    ** Identity Threshold value (1*2**13=8192)
    */

    if ((qca->Coverage & 6144U) && (qca->Identity >= identity))
        qca->Coverage += 8192U;

    return ajTrue;
}




/* @func ensQcalignmentCalculateQueryCoverageQueryQuery ***********************
**
** Compares and scores two Ensembl Quality Check Alignment objects based on the
** coverage of the query sequences in the alignment. The score is the sum of
** 5' and 3' properties, which can be 'longer', 'equal' or 'shorter' in respect
** to the coverage of the query in the second alignment.
** The first alignment is usually the genome alignment the second one the
** reference to test alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::q2q_query_coverage
** @param [r] qca1 [const EnsPQcalignment]
** First Ensembl Quality Check Alignment
** @param [r] qca2 [const EnsPQcalignment]
** Second Ensembl Quality Check Alignment
** @param [u] Pscore [ajuint*] Coverage score address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** Query to Query Comparison Criteria
**
**   Start Coordinate Criteria and Scores   2**n  score
**
**     Q1S<Q2S      ==> first query longer       5    32
**     Q1S=Q2S      ==> queries are equal        7   128
**     Q1S>Q2S      ==> first query shorter      3     8
**
**   End Coordinate Criteria and Scores     2**n  score
**
**     Q1E>Q2E      ==> first query longer       4    16
**     Q1E=Q2E      ==> queries are equal        6    64
**     Q1E<Q2E      ==> first query shorter      2     4
**
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoverageQueryQuery(
    const EnsPQcalignment qca1,
    const EnsPQcalignment qca2,
    ajuint *Pscore)
{
    ajint end1   = 0;
    ajint end2   = 0;
    ajint start1 = 0;
    ajint start2 = 0;

    if (!qca1)
        return ajFalse;

    if (!Pscore)
        return ajFalse;

    *Pscore = 0;

    /*
    ** If either of the Alignment objects has no target assigned, the score is
    ** automatically 0.
    */

    if (!qca1->TargetSequence)
        return ajTrue;

    if (!qca2)
        return ajTrue;

    if (!qca2->TargetSequence)
        return ajTrue;

    /* The first Alignment object is always defined. (1*2**0=1) */

    *Pscore += 1U;

    if (qca2)
    {
        /*
        ** Check whether the query Ensembl Quality Check Sequence object is
        ** the same in both Ensembl Quality Check Alignment objects.
        */

        if (!ensQcsequenceMatch(qca1->QuerySequence, qca2->QuerySequence))
        {
            *Pscore = 0U;

            return ajTrue;
        }

        /* The second Alignment object is defined. (1*2**1=2) */

        *Pscore += 2U;

        /*
        ** Determine the relative orientation of the query sequences in the
        ** alignment.
        */

        if (qca1->QueryStrand == qca2->QueryStrand)
        {
            /* Parallel query sequences. */

            start1 = qca1->QueryStart;
            end1   = qca1->QueryEnd;

            start2 = qca2->QueryStart;
            end2   = qca2->QueryEnd;
        }
        else
        {
            /* Anti-parallel query sequences. */

            start1 = qca1->QueryStart;
            end1   = qca1->QueryEnd;

            start2 = qca2->QueryEnd;
            end2   = qca2->QueryStart;
        }

        /* Evaluate query start coordinates. */

        /* The first alignment is longer. (1*2**5=32) */
        if (start1 < start2)
            *Pscore += 32U;
        /* The first alignment is as long as the second. (1*2**7=128) */
        else if (start1 == start2)
            *Pscore += 128U;
        /* The first alignment is shorter. (1*2**3=8) */
        else if (start1 > start2)
            *Pscore += 8U;
        else
            ajWarn("Unexpected query start coordinate relationship.");

        /* Evaluate query end coordinates. */

        /* The first alignment is longer. (1*2**4=16) */
        if (end1 > end2)
            *Pscore += 16U;
        /* The first alignment is as long as the second. (1*2**6=64) */
        else if (end1 == end2)
            *Pscore += 64U;
        /* The first alignment is shorter. (1*2**2=4) */
        else if (end1 < end2)
            *Pscore += 4U;
        else
            ajWarn("Unexpected query end coordinate releationship.");
    }

    /*
    ** Test for perfect query coverage in genome alignments. See
    ** ensQcalignmentCalculateQueryCoverageProteinGenome and
    ** ensQcalignmentCalculateQueryCoverageDnaGenome for details.
    ** Since this method is called for the genome alignment, the test
    ** refers to the first alignment (qca1).
    */

    /* Perfect N- or 5'-terminus (1*2**6=64) */
    if (qca1->Coverage & 64U)
        *Pscore += 512U;

    /* Perfect C- or 3'-terminus (1*2**5=32) */
    if (qca1->Coverage & 32U)
        *Pscore += 256U;

    return ajTrue;
}




/* @func ensQcalignmentCalculateQueryCoverageQueryTarget **********************
**
** Calculates the alignment coverage score for the query sequence and sets it
** in the coverage attribute of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::query_coverage
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold to classify cases where only few
**                          residues at the edge are different.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
**
** Query Alignment Coverage Schema
**
** N-terminus and C-terminus are scored independently based on alignment
** coordinates of query and target sequences.
**
**
**                 0 QS              QE QL
**               0  \  \    Query   /  /    TL
**                \  |--|+++++++++++|--|   /
**                 |----|+++++++++++|------|
**                     /   Target   \
**                   TS              TE
**
**
******************************************************************************/

AjBool ensQcalignmentCalculateQueryCoverageQueryTarget(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    AjBool result = AJFALSE;

    EnsPQcdatabase qqcdb = NULL;
    EnsPQcdatabase tqcdb = NULL;

    if (!qca)
        return ajFalse;

    qqcdb = ensQcsequenceGetQcdatabase(qca->QuerySequence);
    tqcdb = ensQcsequenceGetQcdatabase(qca->TargetSequence);

    switch (ensQcdatabaseGetClass(tqcdb))
    {
        case ensEQcdatabaseClassGenome:

            switch (ensQcdatabaseGetType(qqcdb))
            {
                case ensEQcdatabaseTypeProtein:

                    result = ensQcalignmentCalculateQueryCoverageProteinGenome(
                        qca,
                        identity,
                        edge);

                    break;

                case ensEQcdatabaseTypeDNA:

                    result = ensQcalignmentCalculateQueryCoverageDnaGenome(
                        qca,
                        identity,
                        edge);

                    break;

                default:

                    ajWarn("Cannot calculate a query to genome alignment "
                           "coverage for an Ensembl Quality Check Database of "
                           "type '%s'.",
                           ensQcdatabaseTypeToChar(
                               ensQcdatabaseGetType(qqcdb)));
            }

            break;

        default:

            switch (ensQcdatabaseGetType(qqcdb))
            {
                case ensEQcdatabaseTypeProtein:

                    result = ensQcalignmentCalculateQueryCoverageProteinProtein(
                        qca,
                        identity);

                    break;

                case ensEQcdatabaseTypeDNA:

                    result = ensQcalignmentCalculateQueryCoverageDnaDna(
                        qca,
                        identity,
                        edge);

                    break;

                default:

                    ajWarn("Cannot calculate a query alignment "
                           "coverage for an Ensembl Quality Check Database of "
                           "type '%s'.",
                           ensQcdatabaseTypeToChar(
                               ensQcdatabaseGetType(qqcdb)));
            }
    }

    return result;
}




/* @func ensQcalignmentCalculateTargetCoordinates *****************************
**
** Caclulate target coordinates of an Ensembl Quality Check Alignment.
**
** @cc Bio::EnsEMBL::QC::Alignment::target_coordinates
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @param [wP] Pstart [ajint*] Start
** @param [wP] Pend [ajint*] End
** @param [wP] Pstrand [ajint*] Strand
** @param [wP] Plength [ajuint*] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentCalculateTargetCoordinates(const EnsPQcalignment qca,
                                                ajint *Pstart,
                                                ajint *Pend,
                                                ajint *Pstrand,
                                                ajuint *Plength)
{
    if (!qca)
        return ajFalse;

    if (!Pstart)
        return ajFalse;

    if (!Pend)
        return ajFalse;

    if (!Pstrand)
        return ajFalse;

    if (!Plength)
        return ajFalse;

    if (qca->TargetStrand >= 0)
    {
        *Pstart = qca->TargetStart;
        *Pend   = qca->TargetEnd;
    }
    else
    {
        *Pstart = qca->TargetEnd;
        *Pend   = qca->TargetStart;
    }

    *Pstrand = qca->TargetStrand;

    /* In Exonerate coordinates the length is just end - start. */

    *Plength = *Pend - *Pstart;

    return ajTrue;
}




/* @section check *************************************************************
**
** Check Ensembl Quality Check Alignment objects.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Check Check Ensembl Quality Check Alignment objects
** @nam4rule Target Check the target Ensembl Quality Check Sequence object
** @nam5rule Overlap Check for target Ensembl Quality Check Sequence overlap
**
** @argrule CheckTargetOverlap qca1 [const EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule CheckTargetOverlap qca2 [const EnsPQcalignment]
** Ensembl Quality Check Alignment
**
** @valrule * [AjBool] ajTrue on overlap, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentCheckTargetOverlap *************************************
**
** Check if two Ensembl Quality Check Alignment objects overlap on the target
** Ensembl Quality Check Sequence.
** The test checks for identical Ensembl Quality Check Sequences and takes
** strand information into account.
**
** @cc Bio::EnsEMBL::QC::Alignment::overlap_on_target
** @param [r] qca1 [const EnsPQcalignment]
** First Ensembl Quality Check Sequence
** @param [r] qca2 [const EnsPQcalignment]
** Second Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue on overlap, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentCheckTargetOverlap(const EnsPQcalignment qca1,
                                        const EnsPQcalignment qca2)
{
    ajint end1    = 0;
    ajint end2    = 0;
    ajint start1  = 0;
    ajint start2  = 0;
    ajint strand1 = 0;
    ajint strand2 = 0;

    ajuint length1 = 0U;
    ajuint length2 = 0U;

    if (!qca1)
        return ajFalse;

    if (!qca2)
        return ajFalse;

    /* Check for identical target sequence objects. */

    if (!ensQcsequenceMatch(qca1->TargetSequence, qca2->TargetSequence))
        return ajFalse;

    /* Check for identical target strands. */

    if (qca1->TargetStrand != qca2->TargetStrand)
        return 0;

    /*
    ** Determine the relative orientation of the target sequences in the
    ** alignment.
    */

    ensQcalignmentCalculateTargetCoordinates(qca1,
                                             &start1,
                                             &end1,
                                             &strand1,
                                             &length1);

    ensQcalignmentCalculateTargetCoordinates(qca2,
                                             &start2,
                                             &end2,
                                             &strand2,
                                             &length2);

    /* Overlap criterion */

    if ((start1 <= end2) && (end1 >= start2))
        return ajTrue;

    return ajFalse;
}




/* @section report ************************************************************
**
** Report Ensembl Quality Check Alignment objects.
**
** @fdata [EnsPQcalignment]
**
** @nam3rule Report Report Ensembl Quality Check Alignment objects
**
** @argrule Report qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Report Pstr [AjPStr*] AJAX String
** @argrule Report internalquery [AjBool] HTML document-internal query link
** @argrule Report internaltarget [AjBool] HTML document-internal target link
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentReport *************************************************
**
** Report an Ensembl Quality Check Alignment in the following way:
** query:start-end:strand (length)
** target:start-end:strand (length)
**
** @cc Bio::EnsEMBL::QC::Alignment::report
** @param [r] qca [const EnsPQcalignment]
** Ensembl Quality Check Alignment Adaptor
** @param [w] Pstr [AjPStr*] AJAX String
** @param [r] internalquery [AjBool] HTML document-internal query link
** @param [r] internaltarget [AjBool] HTML document-internal target link
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentReport(const EnsPQcalignment qca,
                            AjPStr *Pstr,
                            AjBool internalquery,
                            AjBool internaltarget)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    ajuint length = 0U;

    AjPStr anchor   = NULL;
    AjPStr exturl   = NULL;
    AjPStr location = NULL;

    EnsPQcdatabase qcdb = NULL;

    if (!qca)
        return ajFalse;

    /* Format the query part of the Alignment object. */

    ensQcalignmentCalculateQueryCoordinates(qca,
                                            &start,
                                            &end,
                                            &strand,
                                            &length);

    qcdb = ensQcsequenceGetQcdatabase(qca->QuerySequence);

    if (internalquery)
    {
        /* A HTML document-internal link is requested. */

        anchor = ajStrNew();

        ensQcsequenceFetchAnchorInternal(qca->QuerySequence, &anchor);

        ajFmtPrintAppS(Pstr, "%S:%d-%d:%d", anchor, start, end, strand);

        ajStrDel(&anchor);
    }
    else if (ensQcdatabaseGetUrlExternal(qcdb))
    {
        /* An URL has been set in the sequence database definition. */

        exturl = ajStrNewS(ensQcdatabaseGetUrlExternal(qcdb));

        if (ajStrFindC(exturl, "###LOCATION###"))
        {
            /* ###LOCATION### links to Location/View. */

            location = ajFmtStr("%S:%d-%d",
                                ensQcsequenceGetName(qca->QuerySequence),
                                start,
                                end);

            ajStrExchangeCS(&exturl, "###LOCATION###", location);

            ensHtmlEncodeEntities(&exturl);

            ajFmtPrintAppS(Pstr,
                           "<a href=\"%S\" target=\"external\">%S</a>:%d",
                           exturl,
                           location,
                           strand);

            ajStrDel(&location);
        }
        else
        {
            /* Conventional URL replacement. */

            anchor = ajStrNew();

            ensQcsequenceFetchAnchorExternal(qca->QuerySequence,
                                             ajFalse,
                                             &anchor);

            ajFmtPrintAppS(Pstr,
                           "%S:%d-%d:%d",
                           anchor,
                           start,
                           end,
                           strand);

            ajStrDel(&anchor);
        }

        ajStrDel(&exturl);
    }
    else
    {
        /* No URL just plain text. */

        ajFmtPrintAppS(Pstr,
                       "%S:%d-%d:%d",
                       ensQcsequenceGetName(qca->QuerySequence),
                       start,
                       end,
                       strand);
    }

    ajFmtPrintAppS(Pstr, " (%d)", length);

    if (!qca->TargetSequence)
        return ajTrue;

    /* Format the target part of the Alignment object. */

    ensQcalignmentCalculateTargetCoordinates(qca,
                                             &start,
                                             &end,
                                             &strand,
                                             &length);

    qcdb = ensQcsequenceGetQcdatabase(qca->TargetSequence);

    ajStrAppendC(Pstr, "\t");

    if (internaltarget)
    {
        /* A HTML document-internal link has been requested. */

        anchor = ajStrNew();

        ensQcsequenceFetchAnchorInternal(qca->TargetSequence, &anchor);

        ajFmtPrintAppS(Pstr, "%S:%d-%d:%d", anchor, start, end, strand);

        ajStrDel(&anchor);
    }
    else if (ensQcdatabaseGetUrlExternal(qcdb))
    {
        /* URL has been set in the sequence database definition. */

        exturl = ajStrNewS(ensQcdatabaseGetUrlExternal(qcdb));

        if (ajStrFindC(exturl, "###LOCATION###"))
        {

            /* ###LOCATION### links to Location/View. */

            location = ajFmtStr("%S:%d-%d",
                                ensQcsequenceGetName(qca->TargetSequence),
                                start,
                                end);

            ajStrExchangeCS(&exturl, "###LOCATION###", location);

            ensHtmlEncodeEntities(&exturl);

            ajFmtPrintAppS(Pstr,
                           "<a href=\"%S\" target=\"external\">%S</a>:%d",
                           exturl,
                           location,
                           strand);

            ajStrDel(&location);
        }
        else
        {
            /* Conventional URL replacement. */

            anchor = ajStrNew();

            ensQcsequenceFetchAnchorExternal(qca->TargetSequence,
                                             ajFalse,
                                             &anchor);

            ajFmtPrintAppS(Pstr,
                           "%S:%d-%d:%d",
                           anchor,
                           start,
                           end,
                           strand);

            ajStrDel(&anchor);
        }

        ajStrDel(&exturl);
    }
    else
    {
        /* No URL just plain text. */

        ajFmtPrintAppS(Pstr,
                       "%S:%d-%d:%d",
                       ensQcsequenceGetName(qca->TargetSequence),
                       start,
                       end,
                       strand);
    }

    ajFmtPrintAppS(Pstr, " (%d)", length);

    return ajTrue;
}




/* @datasection [EnsPQcalignmentadaptor] Ensembl Quality Check Alignment
** Adaptor
**
** @nam2rule Qcalignmentadaptor Functions for manipulating
** Ensembl Quality Check Alignment Adaptor objects
**
** @cc Bio::EnsEMBL::QC::DBSQL::Alignmentadaptor
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @funcstatic qcalignmentadaptorFetchAllbyStatement **************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Quality Check Alignment objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool qcalignmentadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcas)
{
    double score   = 0.0;
    float identity = 0.0F;

    ajint qstrand = 0;
    ajint tstrand = 0;
    ajint sstrand = 0;

    ajuint identifier = 0U;
    ajuint analysisid = 0U;
    ajuint coverage   = 0U;
    ajuint qdbid      = 0U;
    ajuint qsid       = 0U;
    ajuint qstart     = 0U;
    ajuint qend       = 0U;
    ajuint tdbid      = 0U;
    ajuint tsid       = 0U;
    ajuint tstart     = 0U;
    ajuint tend       = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr vulgar = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPQcalignment qca         = NULL;
    EnsPQcalignmentadaptor qcaa = NULL;

    EnsPQcsequence qsequence   = NULL;
    EnsPQcsequence tsequence   = NULL;
    EnsPQcsequenceadaptor qcsa = NULL;

    if (ajDebugTest("qcalignmentadaptorFetchAllbyStatement"))
        ajDebug("qcalignmentadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcas %p\n",
                ba,
                statement,
                am,
                slice,
                qcas);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!qcas)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    aa   = ensRegistryGetAnalysisadaptor(dba);
    qcaa = ensRegistryGetQcalignmentadaptor(dba);
    qcsa = ensRegistryGetQcsequenceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        analysisid = 0;
        qdbid      = 0;
        qsid       = 0;
        qstart     = 0;
        qend       = 0;
        qstrand    = 0;
        tdbid      = 0;
        tsid       = 0;
        tstart     = 0;
        tend       = 0;
        tstrand    = 0;
        sstrand    = 0;
        score      = 0.0;
        identity   = 0.0F;
        vulgar     = ajStrNew();
        coverage   = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToUint(sqlr, &qdbid);
        ajSqlcolumnToUint(sqlr, &qsid);
        ajSqlcolumnToUint(sqlr, &qstart);
        ajSqlcolumnToUint(sqlr, &qend);
        ajSqlcolumnToInt(sqlr, &qstrand);
        ajSqlcolumnToUint(sqlr, &tdbid);
        ajSqlcolumnToUint(sqlr, &tsid);
        ajSqlcolumnToUint(sqlr, &tstart);
        ajSqlcolumnToUint(sqlr, &tend);
        ajSqlcolumnToInt(sqlr, &tstrand);
        ajSqlcolumnToInt(sqlr, &sstrand);
        ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToFloat(sqlr, &identity);
        ajSqlcolumnToStr(sqlr, &vulgar);
        ajSqlcolumnToUint(sqlr, &coverage);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, qsid, &qsequence);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, tsid, &tsequence);

        qca = ensQcalignmentNewIni(qcaa,
                                   identifier,
                                   analysis,
                                   qsequence,
                                   qstart,
                                   qend,
                                   qstrand,
                                   tsequence,
                                   tstart,
                                   tend,
                                   tstrand,
                                   sstrand,
                                   coverage,
                                   score,
                                   identity,
                                   vulgar);

        ajListPushAppend(qcas, (void *) qca);

        ensQcsequenceDel(&qsequence);

        ensQcsequenceDel(&tsequence);

        ensAnalysisDel(&analysis);

        ajStrDel(&vulgar);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Alignment Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Quality Check Alignment Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcalignmentadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcalignmentadaptorNew *********************************************
**
** Default constructor for an Ensembl Quality Check Alignment Adaptor.
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
** @see ensRegistryGetQcalignmentadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcalignmentadaptor ensQcalignmentadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        qcalignmentadaptorKTables,
        qcalignmentadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &qcalignmentadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check Alignment Adaptor object.
**
** @fdata [EnsPQcalignmentadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Alignment Adaptor
**
** @argrule * Pqcaa [EnsPQcalignmentadaptor*]
** Ensembl Quality Check Alignment Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcalignmentadaptorDel *********************************************
**
** Default destructor for an Ensembl Quality Check Alignment Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcaa [EnsPQcalignmentadaptor*]
** Ensembl Quality Check Alignment Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcalignmentadaptorDel(EnsPQcalignmentadaptor *Pqcaa)
{
    ensBaseadaptorDel(Pqcaa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check Alignment Adaptor object.
**
** @fdata [EnsPQcalignmentadaptor]
**
** @nam3rule Get Return Ensembl Quality Check Alignment Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentadaptorGetBaseadaptor **********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Quality Check Alignment Adaptor.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensQcalignmentadaptorGetBaseadaptor(
    EnsPQcalignmentadaptor qcaa)
{
    return qcaa;
}




/* @func ensQcalignmentadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Quality Check Alignment Adaptor.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensQcalignmentadaptorGetDatabaseadaptor(
    EnsPQcalignmentadaptor qcaa)
{
    return ensBaseadaptorGetDatabaseadaptor(qcaa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Quality Check Alignment objects from an
** Ensembl SQL database.
**
** @fdata [EnsPQcalignmentadaptor]
**
** @nam3rule Fetch Fetch Ensembl Quality Check Alignment object(s)
** @nam4rule All   Fetch all Ensembl Quality Check Alignment objects
** @nam4rule Allby Fetch all Ensembl Quality Check Alignment objects matching a
**                 criterion
** @nam5rule Coverage    Fetch all by a coverage score
** @nam5rule Location    Fetch all by a location
** @nam6rule Target      Fetch all by a location on the target
** @nam5rule Qcdatabase  Fetch all by an Ensembl Quality Check Database
** @nam6rule Pair        Fetch all by an Ensembl Quality Check Database pair
** @nam6rule Query       Fetch all by an Ensembl Quality Check Database
**                       as query
** @nam6rule Target      Fetch all by an Ensembl Quality Check Database
**                       as target
** @nam5rule Region      Fetch all by a region
** @nam4rule By          Fetch one Ensembl Quality Check Alignment object
**                       matching a criterion
** @nam5rule Identifier  Fetch by an SQL database internal identifier
**
** @argrule * qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @argrule AllbyCoverage lower [ajuint] Lower coverage score
** @argrule AllbyCoverage upper [ajuint] Upper coverage score
** @argrule AllbyCoverage qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
** @argrule AllbyLocationTarget analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyLocationTarget tsequence [const EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule AllbyLocationTarget tstart [ajuint] Target start
** @argrule AllbyLocationTarget tend [ajuint] Target end
** @argrule AllbyLocationTarget qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
** @argrule AllbyQcdatabasePair analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcdatabasePair qdb [const EnsPQcdatabase]
** Query Ensembl Quality Check Database
** @argrule AllbyQcdatabasePair tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyQcdatabasePair qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
** @argrule AllbyQcdatabaseQuery analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcdatabaseQuery qdb [const EnsPQcdatabase]
** Query Ensembl Quality Check Database
** @argrule AllbyQcdatabaseQuery qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
** @argrule AllbyQcdatabaseTarget analysis [const EnsPAnalysis]
** Ensembl Analysis
** @argrule AllbyQcdatabaseTarget tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyQcdatabaseTarget qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
** @argrule AllbyRegion analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyRegion qsequence [const EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @argrule AllbyRegion tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyRegion tsequence [const EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule AllbyRegion tstart [ajuint] Target start
** @argrule AllbyRegion tend [ajuint] Target end
** @argrule AllbyRegion tstrand [ajint] Target strand
** @argrule AllbyRegion qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pqca [EnsPQcalignment*]
** Ensembl Quality Check Alignment address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentadaptorFetchAllbyCoverage ******************************
**
** Fetch all Ensembl Quality Check Alignment objects by a coverage score range.
** The caller is responsible for deleting the Ensembl Quality Check Alignment
** objects before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [r] lower [ajuint] Lower coverage score
** @param [r] upper [ajuint] Upper coverage score
** @param [u] qcas [AjPList] AJAX List of Ensembl Quality Check Alignment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllbyCoverage(EnsPQcalignmentadaptor qcaa,
                                               ajuint lower,
                                               ajuint upper,
                                               AjPList qcas)
{
    AjPStr constraint = NULL;

    if (!qcaa)
        return ajFalse;

    if (!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.coverage >= %u "
                          "AND "
                          "alignment.coverage <= %u",
                          lower, upper);

    ensBaseadaptorFetchAllbyConstraint(qcaa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllbyLocationTarget ************************
**
** Fetch all Ensembl Quality Check Alignment objects by a target location.
**
** The caller is responsible for deleting the Ensembl Quality Check Alignment
** objects before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] tsequence [const EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @param [r] tstart [ajuint] Target start
** @param [r] tend [ajuint] Target end
** @param [u] qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllbyLocationTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence tsequence,
    ajuint tstart,
    ajuint tend,
    AjPList qcas)
{
    AjPStr constraint = NULL;

    if (!qcaa)
        return ajFalse;

    if (!analysis)
        return ajFalse;

    if (!tsequence)
        return ajFalse;

    if (!qcas)
        return ajFalse;

    /*
    ** Sometimes things can get complicated. There are four scenarios possible.
    **
    ** 1. The feature start (fs) and feature end (fe) are both within
    **    target start (ss) and target end (se).
    **    This case is also covered by the combination of cases 2 and 3.
    **
    **    (fs>=ss AND fe<=se)
    **
    ** 2. Feature start is within target start and target end.
    **
    **    (fs>=ss AND fs<=se)
    **
    ** 3. Feature end is within target start and target end.
    **
    **    (fe>=ss AND fe<=se)
    **
    ** 4. The feature spans the target, but both ends are outside.
    **
    **    (fs<=ss AND fe>=se)
    **
    ** We want all features that fulfil either of the criteria.
    **
    ** Since Ensembl Quality Check Alignment object use the exonerate
    ** coordinate system, features on the reverse strand have a target start
    ** greater than the target end. Therefore, one SQL query per strand is
    ** required.
    */

    constraint = ajFmtStr(
        "alignment.analysis_id = %u "
        "AND "
        "alignment.target_id = %u "
        "AND "
        "alignment.target_strand >= 0 "
        "AND "
        "("
        "(alignment.target_start >= %u AND alignment.target_start <= %u) "
        "OR "
        "(alignment.target_end >= %u AND alignment.target_end <= %u) "
        "OR "
        "(alignment.target_start <= %u AND alignment.target_end >= %u)"
        ")",
        ensAnalysisGetIdentifier(analysis),
        ensQcsequenceGetIdentifier(tsequence),
        tstart,
        tend,
        tstart,
        tend,
        tstart,
        tend);

    ensBaseadaptorFetchAllbyConstraint(qcaa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcas);

    ajStrDel(&constraint);

    constraint = ajFmtStr(
        "alignment.analysis_id = %u "
        "AND "
        "alignment.target_id = %u "
        "AND "
        "alignment.target_strand < 0 "
        "AND "
        "("
        "(alignment.target_end >= %u AND alignment.target_end <= %u) "
        "OR "
        "(alignment.target_start >= %u AND alignment.target_start <= %u) "
        "OR "
        "(alignment.target_end <= %u AND alignment.target_start >= %u)"
        ")",
        ensAnalysisGetIdentifier(analysis),
        ensQcsequenceGetIdentifier(tsequence),
        tstart,
        tend,
        tstart,
        tend,
        tstart,
        tend);

    ensBaseadaptorFetchAllbyConstraint(qcaa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllbyQcdatabasePair ************************
**
** Fetch all Ensembl Quality Check Alignment objects by Ensembl an Analysis,
** as well as query and target Ensembl Sequences.
** The caller is responsible for deleting the Ensembl Quality Check Alignment
** objects before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl Quality Check Database
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl Quality Check Database
** @param [u] qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllbyQcdatabasePair(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcas)
{
    AjPStr constraint = NULL;

    if (!qcaa)
        return ajFalse;

    if (!analysis)
        return ajFalse;

    if (!qdb)
        return ajFalse;

    if (!tdb)
        return ajFalse;

    if (!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.analysis_id = %u "
                          "AND "
                          "alignment.query_db_id = %u "
                          "AND "
                          "alignment.target_db_id = %u",
                          ensAnalysisGetIdentifier(analysis),
                          ensQcdatabaseGetIdentifier(qdb),
                          ensQcdatabaseGetIdentifier(tdb));

    ensBaseadaptorFetchAllbyConstraint(qcaa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllbyQcdatabaseQuery ***********************
**
** Fetch all Ensembl Quality Check Alignment objects by a
** Query Ensembl Quality Check Sequence.
** The caller is responsible for deleting the Ensembl Quality Check Alignment
** objects before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl Quality Check Database
** @param [u] qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllbyQcdatabaseQuery(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcas)
{
    AjPStr constraint = NULL;

    if (!qcaa)
        return ajFalse;

    if (!qdb)
        return ajFalse;

    if (!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.query_db_id = %u",
                          ensQcdatabaseGetIdentifier(qdb));

    if (analysis)
        ajFmtPrintAppS(&constraint,
                       " AND alignment.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorFetchAllbyConstraint(qcaa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllbyQcdatabaseTarget **********************
**
** Fetch all Ensembl Quality Check Alignment objects by a
** Target Ensembl Quality Check Sequence.
** The caller is responsible for deleting the Ensembl Quality Check Alignment
** objects before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl Quality Check Database
** @param [u] qcas [AjPList] AJAX List of
** Ensembl Quality Check Alignment objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllbyQcdatabaseTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcas)
{
    AjPStr constraint = NULL;

    if (!qcaa)
        return ajFalse;

    if (!tdb)
        return ajFalse;

    if (!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.target_db_id = %u",
                          ensQcdatabaseGetIdentifier(tdb));

    if (analysis)
        ajFmtPrintAppS(&constraint,
                       " AND alignment.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorFetchAllbyConstraint(qcaa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Quality Check Alignment via its
** SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Quality Check Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [r] identifier [ajuint] SQL database-internal
** Ensembl Quality Check Alignment identifier
** @param [wP] Pqca [EnsPQcalignment*] Ensembl Quality Check Alignment address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchByIdentifier(EnsPQcalignmentadaptor qcaa,
                                              ajuint identifier,
                                              EnsPQcalignment *Pqca)
{
    if (!qcaa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pqca)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(qcaa, identifier, (void **) Pqca);
}




/* @section object access *****************************************************
**
** Functions for accessing Ensembl Quality Check Alignment objects in an
** Ensembl SQL database.
**
** @fdata [EnsPQcalignmentadaptor]
**
** @nam3rule Delete Delete Ensembl Quality Check Alignment object(s)
** @nam3rule Store  Store Ensembl Quality Check Alignment object(s)
** @nam3rule Update Update Ensembl Quality Check Alignment object(s)
**
** @argrule * qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @argrule Delete qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Store qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Update qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentadaptorDelete ******************************************
**
** Delete an Ensembl Quality Check Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorDelete(EnsPQcalignmentadaptor qcaa,
                                   EnsPQcalignment qca)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcaa)
        return ajFalse;

    if (!qca)
        return ajFalse;

    if (!ensQcalignmentGetIdentifier(qca))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcaa);

    statement = ajFmtStr(
        "DELETE FROM "
        "alignment "
        "WHERE "
        "alignment.alignment_id = %u",
        qca->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        qca->Adaptor    = (EnsPQcalignmentadaptor) NULL;
        qca->Identifier = 0;

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcalignmentadaptorStore *******************************************
**
** Store an Ensembl Quality Check Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorStore(EnsPQcalignmentadaptor qcaa,
                                  EnsPQcalignment qca)
{
    char *txtvulgar = NULL;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcaa)
        return ajFalse;

    if (!qca)
        return ajFalse;

    if (ensQcalignmentGetAdaptor(qca) &&
        ensQcalignmentGetIdentifier(qca))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcaa);

    ensDatabaseadaptorEscapeC(dba, &txtvulgar, qca->Vulgar);

    statement = ajFmtStr(
        "INSERT IGNORE INTO "
        "alignment "
        "SET "
        "alignment.analysis_id = %u, "
        "alignment.query_db_id = %u, "
        "alignment.query_id = %u, "
        "alignment.query_start = %u, "
        "alignment.query_end = %u, "
        "alignment.query_strand = %d, "
        "alignment.target_db_id = %u, "
        "alignment.target_id = %u, "
        "alignment.target_start = %u, "
        "alignment.target_end = %u, "
        "alignment.target_strand = %d, "
        "alignment.splice_strand = %d, "
        "alignment.score = %f, "
        "alignment.identity = %f, "
        "alignment.vulgar_line = '%s', "
        "alignment.coverage = %u",
        ensAnalysisGetIdentifier(qca->Analysis),
        ensQcsequenceGetQcdatabaseIdentifier(
            qca->QuerySequence),
        ensQcsequenceGetIdentifier(qca->QuerySequence),
        qca->QueryStart,
        qca->QueryEnd,
        qca->QueryStrand,
        ensQcsequenceGetQcdatabaseIdentifier(
            qca->TargetSequence),
        ensQcsequenceGetIdentifier(qca->TargetSequence),
        qca->TargetStart,
        qca->TargetEnd,
        qca->TargetStrand,
        qca->Splicestrand,
        qca->Score,
        qca->Identity,
        txtvulgar,
        qca->Coverage);

    ajCharDel(&txtvulgar);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcalignmentSetIdentifier(qca, ajSqlstatementGetIdentifier(sqls));

        ensQcalignmentSetAdaptor(qca, qcaa);

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcalignmentadaptorUpdate ******************************************
**
** Update an Ensembl Quality Check Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor]
** Ensembl Quality Check Alignment Adaptor
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorUpdate(EnsPQcalignmentadaptor qcaa,
                                   const EnsPQcalignment qca)
{
    char *txtvulgar = NULL;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcaa)
        return ajFalse;

    if (!qca)
        return ajFalse;

    if (!ensQcalignmentGetIdentifier(qca))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcaa);

    ensDatabaseadaptorEscapeC(dba, &txtvulgar, qca->Vulgar);

    statement = ajFmtStr(
        "UPDATE IGNORE "
        "alignment "
        "SET "
        "alignment.analysis_id = %u, "
        "alignment.query_db_id = %u, "
        "alignment.query_id = %u, "
        "alignment.query_start = %u, "
        "alignment.query_end = %u, "
        "alignment.query_strand = %d, "
        "alignment.target_db_id = %u, "
        "alignment.target_id = %u, "
        "alignment.target_start = %u, "
        "alignment.target_end = %u, "
        "alignment.target_strand = %d, "
        "alignment.splice_strand = %d, "
        "alignment.score = %f, "
        "alignment.identity = %f, "
        "alignment.vulgar_line = '%s', "
        "alignment.coverage = %u "
        "WHERE "
        "alignment.alignment_id = %u",
        ensAnalysisGetIdentifier(qca->Analysis),
        ensQcsequenceGetQcdatabaseIdentifier(
            qca->QuerySequence),
        ensQcsequenceGetIdentifier(qca->QuerySequence),
        qca->QueryStart,
        qca->QueryEnd,
        qca->QueryStrand,
        ensQcsequenceGetQcdatabaseIdentifier(
            qca->TargetSequence),
        ensQcsequenceGetIdentifier(qca->TargetSequence),
        qca->TargetStart,
        qca->TargetEnd,
        qca->TargetStrand,
        qca->Splicestrand,
        qca->Score,
        qca->Identity,
        txtvulgar,
        qca->Coverage,
        qca->Identifier);

    ajCharDel(&txtvulgar);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
        result = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}

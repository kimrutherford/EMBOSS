/* @source Ensembl Quality Check Submission functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/05/25 19:55:04 $ by $Author: mks $
** @version $Revision: 1.3 $
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

#include "ensqcsubmission.h"




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

/* @conststatic qcsubmissionadaptorTables *************************************
**
** Array of Ensembl Quality Check Submission Adaptor SQL table names
**
******************************************************************************/

static const char* qcsubmissionadaptorTables[] =
{
    "submission",
    (const char*) NULL
};




/* @conststatic qcalignmentadaptorColumns *************************************
**
** Array of Ensembl Quality Check Submission Adaptor SQL column names
**
******************************************************************************/

static const char* qcsubmissionadaptorColumns[] =
{
    "submission.submission_id",
    "submission.analysis_id",
    "submission.query_db_id",
    "submission.query_id"
    "submission.target_db_id"
    "submission.target_id",
    "submission.target_start",
    "submission.target_end",
    "submission.target_strand",
    "submission.analysis_job_id",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool qcsubmissionadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcss);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensqcsubmission ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcsubmission] Ensembl Quality Check Submission ***********
**
** @nam2rule Qcsubmission Functions for manipulating
** Ensembl Quality Check Submission objects
**
** @cc Bio::EnsEMBL::QC::Submission
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Submission by pointer.
** It is the responsibility of the user to first destroy any previous
** Quality Check Submission. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcsubmission]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
** @argrule Ini qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini qsequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @argrule Ini tsequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule Ini tstart [ajuint] Target start
** @argrule Ini tend [ajuint] Target end
** @argrule Ini tstrand [ajint] Target strand
** @argrule Ini analysisjobid [ajuint] Ensembl Hive Analysis Job identifier
** @argrule Ref qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
**
** @valrule * [EnsPQcsubmission] Ensembl Quality Check Submission or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcsubmissionNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [EnsPQcsubmission] Ensembl Quality Check Submission or NULL
** @@
******************************************************************************/

EnsPQcsubmission ensQcsubmissionNewCpy(const EnsPQcsubmission qcsb)
{
    EnsPQcsubmission pthis = NULL;

    if(!qcsb)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = qcsb->Identifier;

    pthis->Adaptor = qcsb->Adaptor;

    if(qcsb->Analysis)
        pthis->Analysis = ensAnalysisNewRef(qcsb->Analysis);

    pthis->QuerySequence  = ensQcsequenceNewRef(qcsb->QuerySequence);
    pthis->TargetSequence = ensQcsequenceNewRef(qcsb->TargetSequence);

    pthis->TargetStart  = qcsb->TargetStart;
    pthis->TargetEnd    = qcsb->TargetEnd;
    pthis->TargetStrand = qcsb->TargetStrand;

    pthis->Analysisjobidentifier = qcsb->Analysisjobidentifier;

    return pthis;
}




/* @func ensQcsubmissionNewIni ************************************************
**
** Constructor for an Ensembl Quality Check Submission with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::Submission::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] qsequence [EnsPQcsequence] Query Ensembl Quality Check Sequence
** @param [u] tsequence [EnsPQcsequence] Target Ensembl Quality Check Sequence
** @param [r] tstart [ajuint] Target start
** @param [r] tend [ajuint] Target end
** @param [r] tstrand [ajint] Target strand
** @param [r] analysisjobid [ajuint] Ensembl Hive Analysis Job identifier
**
** @return [EnsPQcsubmission] Ensembl Quality Check Submission or NULL
** @@
******************************************************************************/

EnsPQcsubmission ensQcsubmissionNewIni(EnsPQcsubmissionadaptor qcsba,
                                       ajuint identifier,
                                       EnsPAnalysis analysis,
                                       EnsPQcsequence qsequence,
                                       EnsPQcsequence tsequence,
                                       ajuint tstart,
                                       ajuint tend,
                                       ajint tstrand,
                                       ajuint analysisjobid)
{
    EnsPQcsubmission qcsb = NULL;

    if(!analysis)
        return NULL;

    if(!qsequence)
        return NULL;

    if(!tsequence)
        return NULL;

    AJNEW0(qcsb);

    qcsb->Use = 1;

    qcsb->Identifier = identifier;

    qcsb->Adaptor = qcsba;

    qcsb->Analysis = ensAnalysisNewRef(analysis);

    qcsb->QuerySequence  = ensQcsequenceNewRef(qsequence);
    qcsb->TargetSequence = ensQcsequenceNewRef(tsequence);

    qcsb->TargetStart  = tstart;
    qcsb->TargetEnd    = tend;
    qcsb->TargetStrand = tstrand;

    qcsb->Analysisjobidentifier = analysisjobid;

    return qcsb;
}




/* @func ensQcsubmissionNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Submission
**
** @return [EnsPQcsubmission] Ensembl Quality Check Submission or NULL
** @@
******************************************************************************/

EnsPQcsubmission ensQcsubmissionNewRef(EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return NULL;

    qcsb->Use++;

    return qcsb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Quality Check Submission object.
**
** @fdata [EnsPQcsubmission]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Submission object
**
** @argrule * Pqcsb [EnsPQcsubmission*] Ensembl Quality Check Submission
**                                      object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcsubmissionDel ***************************************************
**
** Default destructor for an Ensembl Quality Check Submission.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcsb [EnsPQcsubmission*] Ensembl Quality Check Submission
**                                      object address
**
** @return [void]
** @@
******************************************************************************/

void ensQcsubmissionDel(EnsPQcsubmission* Pqcsb)
{
    EnsPQcsubmission pthis = NULL;

    if(!Pqcsb)
        return;

    if(!*Pqcsb)
        return;

    if(ajDebugTest("ensQcsubmissionDel"))
    {
        ajDebug("ensQcsubmissionDel\n"
                "  *Pqcsb %p\n",
                *Pqcsb);

        ensQcsubmissionTrace(*Pqcsb, 1);
    }

    pthis = *Pqcsb;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pqcsb = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ensQcsequenceDel(&pthis->QuerySequence);
    ensQcsequenceDel(&pthis->TargetSequence);

    AJFREE(pthis);

    *Pqcsb = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Quality Check Submission object.
**
** @fdata [EnsPQcsubmission]
**
** @nam3rule Get Return Ensembl Quality Check Submission attribute(s)
** @nam4rule Adaptor Return the Ensembl Quality Check Submission Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Analysisjobidentifier Return the Hive Analysis Job Identifier
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Query Return query elements
** @nam5rule End Return the query end
** @nam5rule Sequence Return the query Ensembl Quality Check Sequence
** @nam5rule Start Return the query start
** @nam5rule Strand Return the query strand
** @nam4rule Target Return target elements
** @nam5rule End Return the target end
** @nam5rule Sequence Return the target Ensembl Quality Check Sequence
** @nam5rule Start Return the target start
** @nam5rule Strand Return the target strand
**
** @argrule * qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @valrule Adaptor [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Analysisjobidentifier [ajuint] Hive Analysis Job Identifier or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule QueryEnd [ajuint] Query end or 0
** @valrule QuerySequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence or NULL
** @valrule QueryStart [ajuint] Query start or 0
** @valrule QueryStrand [ajint] Query strand or 0
** @valrule TargetEnd [ajuint] Target end or 0
** @valrule TargetSequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence or NULL
** @valrule TargetStart [ajuint] Target start or 0
** @valrule TargetStrand [ajint] Target strand or 0
**
** @fcategory use
******************************************************************************/




/* @func ensQcsubmissionGetAdaptor ********************************************
**
** Get the Ensembl Quality Check Submission Adaptor element of an
** Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [EnsPQcsubmissionadaptor] Ensembl Quality Check Submission Adaptor
** or NULL
** @@
******************************************************************************/

EnsPQcsubmissionadaptor ensQcsubmissionGetAdaptor(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return NULL;

    return qcsb->Adaptor;
}




/* @func ensQcsubmissionGetAnalysis *******************************************
**
** Get the Ensembl Analysis element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
** @@
******************************************************************************/

EnsPAnalysis ensQcsubmissionGetAnalysis(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return NULL;

    return qcsb->Analysis;
}




/* @func ensQcsubmissionGetAnalysisjobidentifier ******************************
**
** Get the Hive Analysis Job identifier element of an
** Ensembl Quality Check Variation.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Variation
**
** @return [ajuint] Hive Analysis Job identifier or 0
** @@
******************************************************************************/

ajuint ensQcsubmissionGetAnalysisjobidentifier(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->Analysisjobidentifier;
}




/* @func ensQcsubmissionGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensQcsubmissionGetIdentifier(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->Identifier;
}




/* @func ensQcsubmissionGetQueryEnd *******************************************
**
** Get the query end element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajuint] Query end or 0
** @@
******************************************************************************/

ajuint ensQcsubmissionGetQueryEnd(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->QueryEnd;
}




/* @func ensQcsubmissionGetQuerySequence **************************************
**
** Get the query Ensembl Quality Check Sequence element of an
** Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [EnsPQcsequence] Query Ensembl Quality Check Sequence or NULL
** @@
******************************************************************************/

EnsPQcsequence ensQcsubmissionGetQuerySequence(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return NULL;

    return qcsb->QuerySequence;
}




/* @func ensQcsubmissionGetQueryStart *****************************************
**
** Get the query start element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajuint] Query start or 0
** @@
******************************************************************************/

ajuint ensQcsubmissionGetQueryStart(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->QueryStart;
}




/* @func ensQcsubmissionGetQueryStrand ****************************************
**
** Get the query strand element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajint] Query strand or 0
** @@
******************************************************************************/

ajint ensQcsubmissionGetQueryStrand(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->QueryStrand;
}




/* @func ensQcsubmissionGetTargetEnd ******************************************
**
** Get the target end element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajuint] Target end or 0
** @@
******************************************************************************/

ajuint ensQcsubmissionGetTargetEnd(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->TargetEnd;
}




/* @func ensQcsubmissionGetTargetSequence *************************************
**
** Get the target Ensembl Quality Check Sequence element of an
** Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [EnsPQcsequence] Target Ensembl Quality Check Sequence or NULL
** @@
******************************************************************************/

EnsPQcsequence ensQcsubmissionGetTargetSequence(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return NULL;

    return qcsb->TargetSequence;
}




/* @func ensQcsubmissionGetTargetStart ****************************************
**
** Get the target start element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajuint] Target start or 0
** @@
******************************************************************************/

ajuint ensQcsubmissionGetTargetStart(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->TargetStart;
}




/* @func ensQcsubmissionGetTargetStrand ***************************************
**
** Get the target strand element of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [ajint] Target strand or 0
** @@
******************************************************************************/

ajint ensQcsubmissionGetTargetStrand(const EnsPQcsubmission qcsb)
{
    if(!qcsb)
        return 0;

    return qcsb->TargetStrand;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an
** Ensembl Quality Check Submission object.
**
** @fdata [EnsPQcsubmission]
**
** @nam3rule Set Set one element of an Ensembl Quality Check Submission
** @nam4rule Adaptor Set the Ensembl Quality Check Submission Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Analysisjobidentifier Set the Analysis Job Identifier
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Query Set query elements
** @nam5rule End Set the query end
** @nam5rule Sequence Set the query Ensembl Quality Check Sequence
** @nam5rule Start Set the query start
** @nam5rule Strand Set the query strand
** @nam4rule Target Set target elements
** @nam5rule End Set the target end
** @nam5rule Sequence Set the target Ensembl Quality Check Sequence
** @nam5rule Start Set the target start
** @nam5rule Strand Set the target strand
**
** @argrule * qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @argrule Adaptor qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Analysisjobidentifier jobid [ajuint] Hive Analysis Job identifier
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule QueryEnd qend [ajuint] Query end
** @argrule QuerySequence qsequence [EnsPQcsequence]
** Ensembl Quality Check Sequence
** @argrule QueryStart qstart [ajuint] Query start
** @argrule QueryStrand qstrand [ajint] Query strand
** @argrule TargetEnd tend [ajuint] Target end
** @argrule TargetSequence tsequence [EnsPQcsequence]
** Ensembl Quality Check Sequence
** @argrule TargetStart tstart [ajuint] Target start
** @argrule TargetStrand tstrand [ajint] Target strand
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcsubmissionSetAdaptor ********************************************
**
** Set the Ensembl Database Adaptor element of an Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [uN] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetAdaptor(EnsPQcsubmission qcsb,
                                 EnsPQcsubmissionadaptor qcsba)
{
    if(!qcsb)
        return ajFalse;

    qcsb->Adaptor = qcsba;

    return ajTrue;
}




/* @func ensQcsubmissionSetAnalysis *******************************************
**
** Set the Ensembl Analysis element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [uN] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetAnalysis(EnsPQcsubmission qcsb,
                                  EnsPAnalysis analysis)
{
    if(!qcsb)
        return ajFalse;

    ensAnalysisDel(&qcsb->Analysis);

    qcsb->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcsubmissionSetAnalysisjobidentifier ******************************
**
** Set the Hive Analysis Job identifier element of an
** Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] jobid [ajuint] Hive Analysis Job identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetAnalysisjobidentifier(EnsPQcsubmission qcsb,
                                               ajuint jobid)
{
    if(!qcsb)
        return ajFalse;

    qcsb->Analysisjobidentifier = jobid;

    return ajTrue;
}




/* @func ensQcsubmissionSetIdentifier *****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetIdentifier(EnsPQcsubmission qcsb,
                                    ajuint identifier)
{
    if(!qcsb)
        return ajFalse;

    qcsb->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcsubmissionSetQueryEnd *******************************************
**
** Set the query end element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] qend [ajuint] Query end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetQueryEnd(EnsPQcsubmission qcsb,
                                  ajuint qend)
{
    if(!qcsb)
        return ajFalse;

    qcsb->QueryEnd = qend;

    return ajTrue;
}




/* @func ensQcsubmissionSetQuerySequence **************************************
**
** Set the query Ensembl Quality Check Sequence element of an
** Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [uN] qsequence [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetQuerySequence(EnsPQcsubmission qcsb,
                                       EnsPQcsequence qsequence)
{
    if(!qcsb)
        return ajFalse;

    ensQcsequenceDel(&qcsb->QuerySequence);

    qcsb->QuerySequence = ensQcsequenceNewRef(qsequence);

    return ajTrue;
}




/* @func ensQcsubmissionSetQueryStart *****************************************
**
** Set the query start element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] qstart [ajuint] Query start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetQueryStart(EnsPQcsubmission qcsb,
                                    ajuint qstart)
{
    if(!qcsb)
        return ajFalse;

    qcsb->QueryStart = qstart;

    return ajTrue;
}




/* @func ensQcsubmissionSetQueryStrand ****************************************
**
** Set the query strand element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] qstrand [ajint] Query strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetQueryStrand(EnsPQcsubmission qcsb,
                                     ajint qstrand)
{
    if(!qcsb)
        return ajFalse;

    qcsb->QueryStrand = qstrand;

    return ajTrue;
}




/* @func ensQcsubmissionSetTargetEnd ******************************************
**
** Set the target end element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] tend [ajuint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetTargetEnd(EnsPQcsubmission qcsb,
                                   ajuint tend)
{
    if(!qcsb)
        return ajFalse;

    qcsb->TargetEnd = tend;

    return ajTrue;
}




/* @func ensQcsubmissionSetTargetSequence *************************************
**
** Set the target Ensembl Quality Check Sequence element of an
** Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [uN] tsequence [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetTargetSequence(EnsPQcsubmission qcsb,
                                        EnsPQcsequence tsequence)
{
    if(!qcsb)
        return ajFalse;

    ensQcsequenceDel(&qcsb->TargetSequence);

    qcsb->TargetSequence = ensQcsequenceNewRef(tsequence);

    return ajTrue;
}




/* @func ensQcsubmissionSetTargetStart ****************************************
**
** Set the target start element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] tstart [ajuint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetTargetStart(EnsPQcsubmission qcsb,
                                     ajuint tstart)
{
    if(!qcsb)
        return ajFalse;

    qcsb->TargetStart = tstart;

    return ajTrue;
}




/* @func ensQcsubmissionSetTargetStrand ***************************************
**
** Set the target strand element of an Ensembl Quality Check Submission.
**
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] tstrand [ajint] Target strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionSetTargetStrand(EnsPQcsubmission qcsb,
                                      ajint tstrand)
{
    if(!qcsb)
        return ajFalse;

    qcsb->TargetStrand = tstrand;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Quality Check Submission object.
**
** @fdata [EnsPQcsubmission]
**
** @nam3rule Trace Report Ensembl Quality Check Submission elements to
**                 debug file
**
** @argrule Trace qcsb [const EnsPQcsubmission]
** Ensembl Quality Check Submission
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcsubmissionTrace *************************************************
**
** Trace an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionTrace(const EnsPQcsubmission qcsb, ajuint level)
{
    AjPStr indent = NULL;

    if(!qcsb)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensQcsubmissionTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Analysis %p\n"
            "%S  QuerySequence %p\n"
            /*
              "%S  QueryStart %u\n"
              "%S  QueryEnd %u\n"
              "%S  QueryStrand %d\n"
            */
            "%S  TargetSequence %p\n"
            "%S  TargetStart %u\n"
            "%S  TargetEnd %u\n"
            "%S  TargetStrand %d\n"
            "%S  Analysisjobidentifier %u\n",
            indent, qcsb,
            indent, qcsb->Use,
            indent, qcsb->Identifier,
            indent, qcsb->Adaptor,
            indent, qcsb->Analysis,
            indent, qcsb->QuerySequence,
            /*
              indent, qcsb->QueryStart,
              indent, qcsb->QueryEnd,
              indent, qcsb->QueryStrand,
            */
            indent, qcsb->TargetSequence,
            indent, qcsb->TargetStart,
            indent, qcsb->TargetEnd,
            indent, qcsb->TargetStrand,
            indent, qcsb->Analysisjobidentifier);

    ensAnalysisTrace(qcsb->Analysis, 1);

    ensQcsequenceTrace(qcsb->QuerySequence, 1);
    ensQcsequenceTrace(qcsb->TargetSequence, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Quality Check Submission object.
**
** @fdata [EnsPQcsubmission]
**
** @nam3rule Calculate Calculate Ensembl Quality Check Submission values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensQcsubmissionCalculateMemsize **************************************
**
** Calculate the memory size in bytes of an Ensembl Quality Check Submission.
**
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensQcsubmissionCalculateMemsize(const EnsPQcsubmission qcsb)
{
    size_t size = 0;

    if(!qcsb)
        return 0;

    size += sizeof (EnsOQcsubmission);

    size += ensAnalysisCalculateMemsize(qcsb->Analysis);

    size += ensQcsequenceCalculateMemsize(qcsb->QuerySequence);
    size += ensQcsequenceCalculateMemsize(qcsb->TargetSequence);

    return size;
}




/* @datasection [EnsPQcsubmissionadaptor] Ensembl Quality Check Submission
** Adaptor
**
** @nam2rule Qcsubmissionadaptor Functions for manipulating
** Ensembl Quality Check Submission Adaptor objects
**
** @cc Bio::EnsEMBL::QC::DBSQL::Submissionadaptor
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @funcstatic qcsubmissionadaptorFetchAllbyStatement *************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Quality Check Submission objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcsubmissionadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcsbs)
{
    ajint tstrand = 0;

    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint qdbid      = 0;
    ajuint qsid       = 0;
    ajuint tdbid      = 0;
    ajuint tsid       = 0;
    ajuint tstart     = 0;
    ajuint tend       = 0;

    ajuint analysisjobid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPQcsequence qsequence   = NULL;
    EnsPQcsequence tsequence   = NULL;
    EnsPQcsequenceadaptor qcsa = NULL;

    EnsPQcsubmission qcsb         = NULL;
    EnsPQcsubmissionadaptor qcsba = NULL;

    if(ajDebugTest("qcsubmissionadaptorFetchAllbyStatement"))
        ajDebug("qcsubmissionadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcsbs %p\n",
                dba,
                statement,
                am,
                slice,
                qcsbs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!qcsbs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    qcsa = ensRegistryGetQcsequenceadaptor(dba);

    qcsba = ensRegistryGetQcsubmissionadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier    = 0;
        analysisid    = 0;
        qdbid         = 0;
        qsid          = 0;
        tdbid         = 0;
        tsid          = 0;
        tstart        = 0;
        tend          = 0;
        tstrand       = 0;
        analysisjobid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToUint(sqlr, &qdbid);
        ajSqlcolumnToUint(sqlr, &qsid);
        ajSqlcolumnToUint(sqlr, &tdbid);
        ajSqlcolumnToUint(sqlr, &tsid);
        ajSqlcolumnToUint(sqlr, &tstart);
        ajSqlcolumnToUint(sqlr, &tend);
        ajSqlcolumnToInt(sqlr, &tstrand);
        ajSqlcolumnToUint(sqlr, &analysisjobid);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, qsid, &qsequence);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, tsid, &tsequence);

        qcsb = ensQcsubmissionNewIni(qcsba,
                                     identifier,
                                     analysis,
                                     qsequence,
                                     tsequence,
                                     tstart,
                                     tend,
                                     tstrand,
                                     analysisjobid);

        ajListPushAppend(qcsbs, (void*) qcsb);

        ensAnalysisDel(&analysis);

        ensQcsequenceDel(&qsequence);
        ensQcsequenceDel(&tsequence);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Submission Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Quality Check Submission Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcsubmissionadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcsubmissionadaptorNew ********************************************
**
** Default constructor for an Ensembl Quality Check Submission Adaptor.
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
** @see ensRegistryGetQcsubmissionadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcsubmissionadaptor] Ensembl Quality Check Submission Adaptor
** or NULL
** @@
******************************************************************************/

EnsPQcsubmissionadaptor ensQcsubmissionadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        qcsubmissionadaptorTables,
        qcsubmissionadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        qcsubmissionadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Quality Check Submission Adaptor object.
**
** @fdata [EnsPQcsubmissionadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Submission Adaptor
**               object
**
** @argrule * Pqcsba [EnsPQcsubmissionadaptor*]
** Ensembl Quality Check Submission Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcsubmissionadaptorDel ********************************************
**
** Default destructor for an Ensembl Quality Check Submission Adaptor.
**
** @param [d] Pqcsba [EnsPQcsubmissionadaptor*]
** Ensembl Quality Check Submission Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensQcsubmissionadaptorDel(EnsPQcsubmissionadaptor* Pqcsba)
{
    if(!Pqcsba)
        return;

    if(!*Pqcsba)
        return;

    if(ajDebugTest("ensQcsubmissionadaptorDel"))
        ajDebug("ensQcsubmissionadaptorDel\n"
                "  *Pqcsba %p\n",
                *Pqcsba);

    ensBaseadaptorDel(Pqcsba);

    *Pqcsba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Quality Check Submission Adaptor object.
**
** @fdata [EnsPQcsubmissionadaptor]
**
** @nam3rule Get Return Ensembl Quality Check Submission Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcsubmissionadaptorGetBaseadaptor *********************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Quality Check Submission Adaptor.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensQcsubmissionadaptorGetBaseadaptor(
    EnsPQcsubmissionadaptor qcsba)
{
    if(!qcsba)
        return NULL;

    return qcsba;
}




/* @func ensQcsubmissionadaptorGetDatabaseadaptor *****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Quality Check Submission Adaptor.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensQcsubmissionadaptorGetDatabaseadaptor(
    EnsPQcsubmissionadaptor qcsba)
{
    if(!qcsba)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(qcsba);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Quality Check Submission objects from an
** Ensembl SQL database.
**
** @fdata [EnsPQcsubmissionadaptor]
**
** @nam3rule Fetch Fetch Ensembl Quality Check Submission object(s)
** @nam4rule All   Fetch all Ensembl Quality Check Submission objects
** @nam4rule Allby Fetch all Ensembl Quality Check Submission objects matching
**                 a criterion
** @nam5rule Qcalignment Fetch all by an Ensembl Quality Check Alignment
** @nam5rule Qcdatabase  Fetch all by an Ensembl Quality Check Database
** @nam6rule Pair        Fetch all by an Ensembl Quality Check Database pair
** @nam6rule Query       Fetch all by an Ensembl Quality Check Database
**                       as query
** @nam6rule Target      Fetch all by an Ensembl Quality Check Database
**                       as target
** @nam5rule Region      Fetch all by a region
** @nam4rule By          Fetch one Ensembl Quality Check Submission object
**                       matching a criterion
** @nam5rule Identifier  Fetch by an SQL database internal identifier
**
** @argrule * qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @argrule AllbyQcalignment qca [const EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule AllbyQcalignment qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
** @argrule AllbyQcdatabasePair analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcdatabasePair qdb [const EnsPQcdatabase]
** Query Ensembl Quality Check Database
** @argrule AllbyQcdatabasePair tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyQcdatabasePair qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
** @argrule AllbyQcdatabaseQuery analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcdatabaseQuery qdb [const EnsPQcdatabase]
** Query Ensembl Quality Check Database
** @argrule AllbyQcdatabaseQuery qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
** @argrule AllbyQcdatabaseTarget analysis [const EnsPAnalysis]
** Ensembl Analysis
** @argrule AllbyQcdatabaseTarget tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyQcdatabaseTarget qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
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
** Ensembl Quality Check Submission objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal
** Ensembl Quality Check Submission identifier
** @argrule ByIdentifier Pqcsb [EnsPQcsubmission*]
** Ensembl Quality Check Submission address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcsubmissionadaptorFetchAllbyQcdatabasePair ***********************
**
** Fetch all Ensembl Quality Check Submission objects by an Ensembl Analysis,
** as well as Ensembl Quality Check Database objects representing query and
** target.
**
** The caller is responsible for deleting the Ensembl Quality Check Submission
** objects before deleting the AJAX List.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl Quality Check Database
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl Quality Check Database
** @param [u] qcsbs [AjPList] AJAX List of Ensembl Quality Check Submissions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorFetchAllbyQcdatabasePair(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcsbs)
{
    AjPStr constraint = NULL;

    if(!qcsba)
        return ajFalse;

    if(!analysis)
        return ajFalse;

    if(!qdb)
        return ajFalse;

    if(!tdb)
        return ajFalse;

    if(!qcsbs)
        return ajFalse;

    constraint = ajFmtStr("submission.analysis_id = %u "
                          "AND "
                          "submission.query_db_id = %u "
                          "AND "
                          "submission.target_db_id = %u",
                          ensAnalysisGetIdentifier(analysis),
                          ensQcdatabaseGetIdentifier(qdb),
                          ensQcdatabaseGetIdentifier(tdb));

    ensBaseadaptorFetchAllbyConstraint(qcsba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcsbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsubmissionadaptorFetchAllbyQcdatabaseQuery **********************
**
** Fetch all Ensembl Quality Check Submission objects by an
** Ensembl Quality Check Sequence representing the query.
**
** The caller is responsible for deleting the Ensembl Quality Check Submission
** objects before deleting the AJAX List.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl Quality Check Database
** @param [u] qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorFetchAllbyQcdatabaseQuery(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcsbs)
{
    AjPStr constraint = NULL;

    if(!qcsba)
        return ajFalse;

    if(!qdb)
        return ajFalse;

    if(!qcsbs)
        return ajFalse;

    constraint = ajFmtStr("submission.query_db_id = %u",
                          ensQcdatabaseGetIdentifier(qdb));

    if(analysis)
        ajFmtPrintAppS(&constraint,
                       " AND submission.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorFetchAllbyConstraint(qcsba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcsbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsubmissionadaptorFetchAllbyQcdatabaseTarget *********************
**
** Fetch all Ensembl Quality Check Submission objects by an
** Ensembl Quality Check Database representing the target.
**
** The caller is responsible for deleting the Ensembl Quality Check Submission
** objects before deleting the AJAX List.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl Quality Check Database
** @param [u] qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorFetchAllbyQcdatabaseTarget(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcsbs)
{
    AjPStr constraint = NULL;

    if(!qcsba)
        return ajFalse;

    if(!tdb)
        return ajFalse;

    if(!qcsbs)
        return ajFalse;

    constraint = ajFmtStr("submission.target_db_id = %u",
                          ensQcdatabaseGetIdentifier(tdb));

    if(analysis)
        ajFmtPrintAppS(&constraint,
                       " AND submission.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorFetchAllbyConstraint(qcsba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcsbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsubmissionadaptorFetchAllbyRegion *******************************
**
** Fetch all Ensembl Quality Check Submission objects by an Ensembl Analysis,
** as well as Ensembl Quality Check Sequence objects representing query and
** target.
**
** The caller is responsible for deleting the Ensembl Quality Check Submission
** objects before deleting the AJAX List.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qsequence [const EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @param [r] tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @param [r] tsequence [const EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @param [rN] tstart [ajuint] Target start
** @param [rN] tend [ajuint] Target end
** @param [rN] tstrand [ajint] Target strand
** @param [u] qcsbs [AjPList] AJAX List of
** Ensembl Quality Check Submission objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorFetchAllbyRegion(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcsequence qsequence,
    const EnsPQcdatabase tdb,
    const EnsPQcsequence tsequence,
    ajuint tstart,
    ajuint tend,
    ajint tstrand,
    AjPList qcsbs)
{
    AjPStr constraint = NULL;

    if(!qcsba)
        return ajFalse;

    if(!analysis)
        return ajFalse;

    if(!qsequence)
        return ajFalse;

    if(!tdb)
        return ajFalse;

    if(!qcsbs)
        return ajFalse;

    constraint = ajFmtStr("submission.analysis_id = %u "
                          "AND "
                          "submission.query_db_id = %u "
                          "AND "
                          "submission.target_db_id = %u "
                          "AND "
                          "submission.query_id = %u",
                          ensAnalysisGetIdentifier(analysis),
                          ensQcsequenceGetQcdatabaseIdentifier(qsequence),
                          ensQcdatabaseGetIdentifier(tdb),
                          ensQcsequenceGetIdentifier(qsequence));

    if(tsequence)
        ajFmtPrintAppS(&constraint,
                       " AND submission.target_id = %u",
                       ensQcsequenceGetIdentifier(tsequence));

    if(tstart && tend)
        ajFmtPrintAppS(&constraint,
                       " AND"
                       " submission.target_start >= %u"
                       " AND"
                       " submission.target_end <= %u"
                       " AND"
                       " submission.target_strand = %d",
                       tstart,
                       tend,
                       tstrand);

    ensBaseadaptorFetchAllbyConstraint(qcsba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcsbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsubmissionadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Quality Check Submission via its
** SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Quality Check Submission.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [r] identifier [ajuint] SQL database-internal
** Ensembl Quality Check Submission identifier
** @param [wP] Pqcsb [EnsPQcsubmission*]
** Ensembl Quality Check Submission address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorFetchByIdentifier(
    EnsPQcsubmissionadaptor qcsba,
    ajuint identifier,
    EnsPQcsubmission* Pqcsb)
{
    if(!qcsba)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pqcsb)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(qcsba, identifier, (void**) Pqcsb);
}




/* @section object access *****************************************************
**
** Functions for accessing Ensembl Quality Check Submission objects in an
** Ensembl SQL database.
**
** @fdata [EnsPQcsubmissionadaptor]
**
** @nam3rule Delete Delete Ensembl Quality Check Submission object(s)
** @nam3rule Store  Store Ensembl Quality Check Submission object(s)
** @nam3rule Update Update Ensembl Quality Check Submission object(s)
**
** @argrule * qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @argrule Delete qcsb [EnsPQcsubmission]
** Ensembl Quality Check Submission
** @argrule Store qcsb [EnsPQcsubmission]
** Ensembl Quality Check Submission
** @argrule Update qcsb [const EnsPQcsubmission]
** Ensembl Quality Check Submission
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcsubmissionadaptorDelete *****************************************
**
** Delete an Ensembl Quality Check Submission.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorDelete(EnsPQcsubmissionadaptor qcsba,
                                    EnsPQcsubmission qcsb)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcsba)
        return ajFalse;

    if(!qcsb)
        return ajFalse;

    if(!ensQcsubmissionGetIdentifier(qcsb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcsba);

    statement = ajFmtStr(
        "DELETE FROM "
        "submission "
        "WHERE "
        "submission.submission_id = %u",
        qcsb->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        qcsb->Adaptor    = (EnsPQcsubmissionadaptor) NULL;
        qcsb->Identifier = 0;

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcsubmissionadaptorStore ******************************************
**
** Store an Ensembl Quality Check Submission.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [u] qcsb [EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorStore(EnsPQcsubmissionadaptor qcsba,
                                   EnsPQcsubmission qcsb)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcsba)
        return ajFalse;

    if(!qcsb)
        return ajFalse;

    if(ensQcsubmissionGetAdaptor(qcsb) && ensQcsubmissionGetIdentifier(qcsb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcsba);

    statement = ajFmtStr(
        "INSERT IGNORE INTO "
        "submission "
        "SET "
        "submission.analysis_id = %u, "
        "submission.query_db_id = %u, "
        "submission.query_id = %u, "
        "submission.target_db_id = %u, "
        "submission.target_id = %u, "
        "submission.target_start = %u, "
        "submission.target_end = %u, "
        "submission.target_strand = %d, "
        "submission.analysis_job_id = %u",
        ensAnalysisGetIdentifier(qcsb->Analysis),
        ensQcsequenceGetQcdatabaseIdentifier(qcsb->QuerySequence),
        ensQcsequenceGetIdentifier(qcsb->QuerySequence),
        ensQcsequenceGetQcdatabaseIdentifier(qcsb->TargetSequence),
        ensQcsequenceGetIdentifier(qcsb->TargetSequence),
        qcsb->TargetStart,
        qcsb->TargetEnd,
        qcsb->TargetStrand,
        qcsb->Analysisjobidentifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcsubmissionSetIdentifier(qcsb, ajSqlstatementGetIdentifier(sqls));

        ensQcsubmissionSetAdaptor(qcsb, qcsba);

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcsubmissionadaptorUpdate *****************************************
**
** Update an Ensembl Quality Check Submission.
**
** @param [u] qcsba [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @param [r] qcsb [const EnsPQcsubmission] Ensembl Quality Check Submission
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsubmissionadaptorUpdate(EnsPQcsubmissionadaptor qcsba,
                                    const EnsPQcsubmission qcsb)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcsba)
        return ajFalse;

    if(!qcsb)
        return ajFalse;

    if(!ensQcsubmissionGetIdentifier(qcsb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcsba);

    statement = ajFmtStr(
        "UPDATE IGNORE "
        "submission "
        "SET "
        "submission.analysis_id = %u, "
        "submission.query_db_id = %u, "
        "submission.query_id = %u, "
        "submission.target_db_id = %u, "
        "submission.target_id = %u, "
        "submission.target_start = %u, "
        "submission.target_end = %u, "
        "submission.target_strand = %d, "
        "submission.analysis_job_id = %u "
        "WHERE "
        "submission.submission_id = %u",
        ensAnalysisGetIdentifier(qcsb->Analysis),
        ensQcsequenceGetQcdatabaseIdentifier(qcsb->QuerySequence),
        ensQcsequenceGetIdentifier(qcsb->QuerySequence),
        ensQcsequenceGetQcdatabaseIdentifier(qcsb->TargetSequence),
        ensQcsequenceGetIdentifier(qcsb->TargetSequence),
        qcsb->TargetStart,
        qcsb->TargetEnd,
        qcsb->TargetStrand,
        qcsb->Analysisjobidentifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
        result = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}

/******************************************************************************
** @source Ensembl Quality Check Alignment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.12 $
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

#include "ensqcalignment.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool qcalignmentadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                              const AjPStr statement,
                                              EnsPAssemblymapper am,
                                              EnsPSlice slice,
                                              AjPList qcas);




/* @filesection ensqcalignment ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcalignment] QC Alignment ********************************
**
** Functions for manipulating Ensembl QC Alignment objects
**
** Bio::EnsEMBL::QC::Alignment CVS Revision:
**
** @nam2rule Qcalignment
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl QC Alignment by pointer.
** It is the responsibility of the user to first destroy any previous
** QC Alignment. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcalignment]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPQcalignment] Ensembl QC Alignment
** @argrule Ref object [EnsPQcalignment] Ensembl QC Alignment
**
** @valrule * [EnsPQcalignment] Ensembl QC Alignment
**
** @fcategory new
******************************************************************************/




/* @func ensQcalignmentNew ****************************************************
**
** Default constructor for an Ensembl QC Alignment.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::Alignment::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] qsequence [EnsPQcsequence] Query Ensembl QC Sequence
** @param [r] qstart [ajuint] Query start
** @param [r] qend [ajuint] Query end
** @param [r] qstrand [ajint] Query strand
** @param [u] tsequence [EnsPQcsequence] Target Ensembl QC Sequence
** @param [r] tstart [ajuint] Target start
** @param [r] tend [ajuint] Target end
** @param [r] tstrand [ajint] Target strand
** @param [r] sstrand [ajint] Splice strand
** @param [r] coverage [ajuint] Coverage score
** @param [r] score [double] Score
** @param [r] identity [float] Identity
** @param [u] vulgar [AjPStr] VULGAR line
**
** @return [EnsPQcalignment] Ensembl QC Alignment or NULL
** @@
******************************************************************************/

EnsPQcalignment ensQcalignmentNew(EnsPQcalignmentadaptor qcaa,
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

    if(!analysis)
        return NULL;

    if(!qsequence)
        return NULL;

    if(!tsequence)
        return NULL;

    AJNEW0(qca);

    qca->Use = 1;

    qca->Identifier = identifier;

    qca->Adaptor = qcaa;

    qca->Analysis = ensAnalysisNewRef(analysis);

    qca->QuerySequence = ensQcsequenceNewRef(qsequence);

    qca->QueryStart = qstart;

    qca->QueryEnd = qend;

    qca->QueryStrand = qstrand;

    qca->TargetSequence = ensQcsequenceNewRef(tsequence);

    qca->TargetStart = tstart;

    qca->TargetEnd = tend;

    qca->TargetStrand = tstrand;

    qca->SpliceStrand = sstrand;

    qca->Coverage = coverage;

    qca->Score = score;

    qca->Identity = identity;

    qca->VULGAR = ajStrNewRef(vulgar);

    return qca;
}




/* @func ensQcalignmentNewObj *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [EnsPQcalignment] Ensembl QC Alignment or NULL
** @@
******************************************************************************/

EnsPQcalignment ensQcalignmentNewObj(const EnsPQcalignment object)
{
    EnsPQcalignment qca = NULL;

    if(!object)
        return NULL;

    AJNEW0(qca);

    qca->Use = 1;

    qca->Identifier = object->Identifier;

    qca->Adaptor = object->Adaptor;

    if(object->Analysis)
        qca->Analysis = ensAnalysisNewRef(object->Analysis);

    qca->QuerySequence = ensQcsequenceNewRef(object->QuerySequence);

    qca->QueryStart = object->QueryStart;

    qca->QueryEnd = object->QueryEnd;

    qca->QueryStrand = object->QueryStrand;

    qca->TargetSequence = ensQcsequenceNewRef(object->TargetSequence);

    qca->TargetStart = object->TargetStart;

    qca->TargetEnd = object->TargetEnd;

    qca->TargetStrand = object->TargetStrand;

    qca->SpliceStrand = object->SpliceStrand;

    qca->Coverage = object->Coverage;

    qca->Score = object->Score;

    qca->Identity = object->Identity;

    if(object->VULGAR)
        qca->VULGAR = ajStrNewRef(object->VULGAR);

    return qca;
}




/* @func ensQcalignmentNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qca [EnsPQcalignment] Ensembl Alignment
**
** @return [EnsPQcalignment] Ensembl QC Alignment or NULL
** @@
******************************************************************************/

EnsPQcalignment ensQcalignmentNewRef(EnsPQcalignment qca)
{
    if(!qca)
        return NULL;

    qca->Use++;

    return qca;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl QC Alignment.
**
** @fdata [EnsPQcalignment]
** @fnote None
**
** @nam3rule Del Destroy (free) a QC Alignment object
**
** @argrule * Pqca [EnsPQcalignment*] QC Alignment object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcalignmentDel ****************************************************
**
** Default destructor for an Ensembl QC Alignment.
**
** @param [d] Pqca [EnsPQcalignment*] Ensembl QC Alignment address
**
** @return [void]
** @@
******************************************************************************/

void ensQcalignmentDel(EnsPQcalignment *Pqca)
{
    EnsPQcalignment pthis = NULL;

    if(!Pqca)
        return;

    if(!*Pqca)
        return;

    if(ajDebugTest("ensQcalignmentDel"))
    {
        ajDebug("ensQcalignmentDel\n"
                "  *Pqca %p\n",
                *Pqca);

        ensQcalignmentTrace(*Pqca, 1);
    }

    pthis = *Pqca;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pqca = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ensQcsequenceDel(&pthis->QuerySequence);

    ensQcsequenceDel(&pthis->TargetSequence);

    ajStrDel(&pthis->VULGAR);

    AJFREE(pthis);

    *Pqca = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl QC Alignment object.
**
** @fdata [EnsPQcalignment]
** @fnote None
**
** @nam3rule Get Return QC Alignment attribute(s)
** @nam4rule GetAdaptor Return the Ensembl QC Alignment Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
**
** @argrule * qca [const EnsPQcalignment] QC Alignment
**
** @valrule Adaptor [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis
** @valrule QuerySequence [AjPStr] Query Ensembl QC Sequence
** @valrule QueryStart [ajuint] Query start
** @valrule QueryEnd [ajuint] Query end
** @valrule QueryStrand [ajint] Query strand
** @valrule TargetSequence [AjPStr] Target Ensembl QC Sequence
** @valrule TargetStart [ajuint] Target start
** @valrule TargetEnd [ajuint] Target end
** @valrule TargetStrand [ajint] Target strand
** @valrule SpliceStrand [ajint] Splice strand
** @valrule Coverage [ajuint] Coverage score
** @valrule Score [double] Score
** @valrule Identity [float] Identity
** @valrule VULGAR [AjPStr] VULGAR line
**
** @fcategory use
******************************************************************************/




/* @func ensQcalignmentGetAdaptor *********************************************
**
** Get the Ensembl QC Alignment Adaptor element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @@
******************************************************************************/

EnsPQcalignmentadaptor ensQcalignmentGetAdaptor(const EnsPQcalignment qca)
{
    if(!qca)
        return NULL;

    return qca->Adaptor;
}




/* @func ensQcalignmentGetIdentifier ***********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensQcalignmentGetIdentifier(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->Identifier;
}




/* @func ensQcalignmentGetAnalysis ********************************************
**
** Get the Ensembl Analysis element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [const EnsPAnalysis] Ensembl Analysis
** @@
******************************************************************************/

const EnsPAnalysis ensQcalignmentGetAnalysis(const EnsPQcalignment qca)
{
    if(!qca)
        return NULL;

    return qca->Analysis;
}




/* @func ensQcalignmentGetQuerySequence ***************************************
**
** Get the query Ensembl QC Sequence element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [EnsPQcsequence] Query Ensembl QC Sequence
** @@
******************************************************************************/

EnsPQcsequence ensQcalignmentGetQuerySequence(const EnsPQcalignment qca)
{
    if(!qca)
        return NULL;

    return qca->QuerySequence;
}




/* @func ensQcalignmentGetQueryStart ******************************************
**
** Get the query start element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajuint] Query start
** @@
******************************************************************************/

ajuint ensQcalignmentGetQueryStart(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->QueryStart;
}




/* @func ensQcalignmentGetQueryEnd ********************************************
**
** Get the query end element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajuint] Query end
** @@
******************************************************************************/

ajuint ensQcalignmentGetQueryEnd(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->QueryEnd;
}




/* @func ensQcalignmentGetQueryStrand *****************************************
**
** Get the query strand element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajint] Query strand
** @@
******************************************************************************/

ajint ensQcalignmentGetQueryStrand(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->QueryStrand;
}




/* @func ensQcalignmentGetTargetSequence **************************************
**
** Get the target Ensembl QC Sequence element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [EnsPQcsequence] Target Ensembl QC Sequence
** @@
******************************************************************************/

EnsPQcsequence ensQcalignmentGetTargetSequence(const EnsPQcalignment qca)
{
    if(!qca)
        return NULL;

    return qca->TargetSequence;
}




/* @func ensQcalignmentGetTargetStart *****************************************
**
** Get the target start element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajuint] Target start
** @@
******************************************************************************/

ajuint ensQcalignmentGetTargetStart(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->TargetStart;
}




/* @func ensQcalignmentGetTargetEnd *******************************************
**
** Get the target end element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajuint] Target end
** @@
******************************************************************************/

ajuint ensQcalignmentGetTargetEnd(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->TargetEnd;
}




/* @func ensQcalignmentGetTargetStrand ****************************************
**
** Get the target strand element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajint] Target strand
** @@
******************************************************************************/

ajint ensQcalignmentGetTargetStrand(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->TargetStrand;
}




/* @func ensQcalignmentGetSpliceStrand ****************************************
**
** Get the splice strand element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajint] Splice strand
** @@
******************************************************************************/

ajint ensQcalignmentGetSpliceStrand(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->SpliceStrand;
}




/* @func ensQcalignmentGetCoverage ********************************************
**
** Get the coverage score element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajuint] Coverage score
** @@
******************************************************************************/

ajuint ensQcalignmentGetCoverage(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->Coverage;
}




/* @func ensQcalignmentGetScore ***********************************************
**
** Get the score element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [double] Score
** @@
******************************************************************************/

double ensQcalignmentGetScore(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->Score;
}




/* @func ensQcalignmentGetIdentity ********************************************
**
** Get the identity element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [float] Identity
** @@
******************************************************************************/

float ensQcalignmentGetIdentity(const EnsPQcalignment qca)
{
    if(!qca)
        return 0;

    return qca->Identity;
}




/* @func ensQcalignmentGetVULGAR **********************************************
**
** Get the VULGAR line element of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [AjPStr] VULGAR line or NULL
** @@
******************************************************************************/

AjPStr ensQcalignmentGetVULGAR(const EnsPQcalignment qca)
{
    if(!qca)
        return NULL;

    return qca->VULGAR;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl QC Alignment object.
**
** @fdata [EnsPQcalignment]
** @fnote None
**
** @nam3rule Set Set one element of a QC Alignment
** @nam4rule SetAdaptor Set the Ensembl QC Alignment Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetQuerySequence Set the query Ensembl QC Sequence
** @nam4rule SetQueryStart Set the query start
** @nam4rule SetQueryEnd Set the query end
** @nam4rule SetQueryStrand Set the query strand
** @nam4rule SetTargetSequence Set the target Ensembl QC Sequence
** @nam4rule SetTargetStart Set the target start
** @nam4rule SetTargetEnd Set the target end
** @nam4rule SetTargetStrand Set the target strand
** @nam4rule SetSpliceStrand Set the splice strand
** @nam4rule SetCoverage Set the coverage score
** @nam4rule SetScore Set the score
** @nam4rule SetIdentity Set the identity
** @nam4rule SetVULGAR Set the VULGAR line
**
** @argrule * qca [EnsPQcalignment] Ensembl QC Alignment
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcalignmentSetAdaptor *********************************************
**
** Set the Ensembl QC Alignment Adaptor element of an QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetAdaptor(EnsPQcalignment qca,
                                EnsPQcalignmentadaptor qcaa)
{
    if(!qca)
        return ajFalse;

    qca->Adaptor = qcaa;

    return ajTrue;
}




/* @func ensQcalignmentSetIdentifier ******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetIdentifier(EnsPQcalignment qca,
                                   ajuint identifier)
{
    if(!qca)
        return ajFalse;

    qca->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcalignmentSetAnalysis ********************************************
**
** Set the Ensembl Analysis element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [uN] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetAnalysis(EnsPQcalignment qca,
                                 EnsPAnalysis analysis)
{
    if(!qca)
        return ajFalse;

    ensAnalysisDel(&qca->Analysis);

    qca->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcalignmentSetQuerySequence ***************************************
**
** Set the query Ensembl QC Sequence element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [uN] qsequence [EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetQuerySequence(EnsPQcalignment qca,
                                      EnsPQcsequence qsequence)
{
    if(!qca)
        return ajFalse;

    ensQcsequenceDel(&qca->QuerySequence);

    qca->QuerySequence = ensQcsequenceNewRef(qsequence);

    return ajTrue;
}




/* @func ensQcalignmentSetQueryStart ******************************************
**
** Set the query start element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] qstart [ajuint] Query start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetQueryStart(EnsPQcalignment qca,
                                   ajuint qstart)
{
    if(!qca)
        return ajFalse;

    qca->QueryStart = qstart;

    return ajTrue;
}




/* @func ensQcalignmentSetQueryEnd ********************************************
**
** Set the query end element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] qend [ajuint] Query end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetQueryEnd(EnsPQcalignment qca,
                                 ajuint qend)
{
    if(!qca)
        return ajFalse;

    qca->QueryEnd = qend;

    return ajTrue;
}




/* @func ensQcalignmentSetQueryStrand *****************************************
**
** Set the query strand element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] qstrand [ajint] Query strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetQueryStrand(EnsPQcalignment qca,
                                    ajint qstrand)
{
    if(!qca)
        return ajFalse;

    qca->QueryStrand = qstrand;

    return ajTrue;
}




/* @func ensQcalignmentSetTargetSequence **************************************
**
** Set the target Ensembl QC Sequence element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [u] tsequence [EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetSequence(EnsPQcalignment qca,
                                       EnsPQcsequence tsequence)
{
    if(!qca)
        return ajFalse;

    ensQcsequenceDel(&qca->TargetSequence);

    qca->TargetSequence = ensQcsequenceNewRef(tsequence);

    return ajTrue;
}




/* @func ensQcalignmentSetTargetStart *****************************************
**
** Set the target start element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] tstart [ajuint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetStart(EnsPQcalignment qca,
                                    ajuint tstart)
{
    if(!qca)
        return ajFalse;

    qca->TargetStart = tstart;

    return ajTrue;
}




/* @func ensQcalignmentSetTargetEnd *******************************************
**
** Set the target end element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] tend [ajuint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetEnd(EnsPQcalignment qca,
                                  ajuint tend)
{
    if(!qca)
        return ajFalse;

    qca->TargetEnd = tend;

    return ajTrue;
}




/* @func ensQcalignmentSetTargetStrand ****************************************
**
** Set the target strand element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] tstrand [ajint] Target strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetTargetStrand(EnsPQcalignment qca,
                                     ajint tstrand)
{
    if(!qca)
        return ajFalse;

    qca->TargetStrand = tstrand;

    return ajTrue;
}




/* @func ensQcalignmentSetSpliceStrand ****************************************
**
** Set the splice strand element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] sstrand [ajint] Splice strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetSpliceStrand(EnsPQcalignment qca,
                                     ajint sstrand)
{
    if(!qca)
        return ajFalse;

    qca->SpliceStrand = sstrand;

    return ajTrue;
}




/* @func ensQcalignmentSetCoverage ********************************************
**
** Set the coverage score element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] coverage [ajuint] Coverage score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetCoverage(EnsPQcalignment qca,
                                 ajuint coverage)
{
    if(!qca)
        return ajFalse;

    qca->Coverage = coverage;

    return ajTrue;
}




/* @func ensQcalignmentSetScore ***********************************************
**
** Set the score element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetScore(EnsPQcalignment qca, double score)
{
    if(!qca)
        return ajFalse;

    qca->Score = score;

    return ajTrue;
}




/* @func ensQcalignmentSetIdentity ********************************************
**
** Set the identity element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identity [float] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetIdentity(EnsPQcalignment qca,
                                 float identity)
{
    if(!qca)
        return ajFalse;

    qca->Identity = identity;

    return ajTrue;
}




/* @func ensQcalignmentSetVULGAR **********************************************
**
** Set the VULGAR line element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [uN] vulgar [AjPStr] VULGAR line
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentSetVULGAR(EnsPQcalignment qca,
                               AjPStr vulgar)
{
    if(!qca)
        return ajFalse;

    ajStrDel(&qca->VULGAR);

    qca->VULGAR = ajStrNewRef(vulgar);

    return ajTrue;
}




/* @func ensQcalignmentGetMemsize *********************************************
**
** Get the memory size in bytes of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensQcalignmentGetMemsize(const EnsPQcalignment qca)
{
    ajulong size = 0;

    if(!qca)
        return 0;

    size += sizeof (EnsOQcalignment);

    size += ensAnalysisGetMemsize(qca->Analysis);

    size += ensQcsequenceGetMemsize(qca->QuerySequence);

    size += ensQcsequenceGetMemsize(qca->TargetSequence);

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl QC Alignment object.
**
** @fdata [EnsPQcalignment]
** @nam3rule Trace Report Ensembl QC Alignment elements to debug file
**
** @argrule Trace qca [const EnsPQcalignment] Ensembl QC Alignment
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcalignmentTrace **************************************************
**
** Trace an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentTrace(const EnsPQcalignment qca, ajuint level)
{
    AjPStr indent = NULL;

    if(!qca)
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
            "%S  SpliceStrand %d\n"
            "%S  Coverage %u\n"
            "%S  Score %f\n"
            "%S  Identity %f\n"
            "%S  VULGAR '%S'\n",
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
            indent, qca->SpliceStrand,
            indent, qca->Coverage,
            indent, qca->Score,
            indent, qca->Identity,
            indent, qca->VULGAR);

    ensAnalysisTrace(qca->Analysis, 1);

    ensQcsequenceTrace(qca->QuerySequence, 1);

    ensQcsequenceTrace(qca->TargetSequence, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensQcalignmentGetQueryCoordinates ************************************
**
** Return query coordinates of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
** @param [wP] Pstart [ajint*] Start
** @param [wP] Pend [ajint*] End
** @param [wP] Pstrand [ajint*] Strand
** @param [wP] Plength [ajuint*] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentGetQueryCoordinates(const EnsPQcalignment qca,
                                         ajint *Pstart,
                                         ajint *Pend,
                                         ajint *Pstrand,
                                         ajuint *Plength)
{
    if(!qca)
        return ajFalse;

    if(!Pstart)
        return ajFalse;

    if(!Pend)
        return ajFalse;

    if(!Pstrand)
        return ajFalse;

    if(!Plength)
        return ajFalse;

    if(qca->QueryStrand >= 0)
    {
        *Pstart = qca->QueryStart;

        *Pend = qca->QueryEnd;
    }
    else
    {
        *Pstart = qca->QueryEnd;

        *Pend = qca->QueryStart;
    }

    *Pstrand = qca->QueryStrand;

    /* In Exonerate coordinates the length is just end - start. */

    *Plength = *Pend - *Pstart;

    return ajTrue;
}




/* @func ensQcalignmentGetTargetCoordinates ***********************************
**
** Return target coordinates of an Ensembl QC Alignment.
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
** @param [wP] Pstart [ajint*] Start
** @param [wP] Pend [ajint*] End
** @param [wP] Pstrand [ajint*] Strand
** @param [wP] Plength [ajuint*] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentGetTargetCoordinates(const EnsPQcalignment qca,
                                          ajint *Pstart,
                                          ajint *Pend,
                                          ajint *Pstrand,
                                          ajuint *Plength)
{
    if(!qca)
        return ajFalse;

    if(!Pstart)
        return ajFalse;

    if(!Pend)
        return ajFalse;

    if(!Pstrand)
        return ajFalse;

    if(!Plength)
        return ajFalse;

    if(qca->TargetStrand >= 0)
    {
        *Pstart = qca->TargetStart;

        *Pend = qca->TargetEnd;
    }
    else
    {
        *Pstart = qca->TargetEnd;

        *Pend = qca->TargetStart;
    }

    *Pstrand = qca->TargetStrand;

    /* In Exonerate coordinates the length is just end - start. */

    *Plength = *Pend - *Pstart;

    return ajTrue;
}




/* @func ensQcalignmentOverlapOnTarget ***************************************
**
** Test if two Ensembl QC Alignments overlap on the target Ensembl QC Sequence.
** The test checks for identical Ensembl QC Sequences and takes strand
** information into account.
**
** @param [r] qca1 [const EnsPQcalignment] First Ensembl QC Sequence
** @param [r] qca2 [const EnsPQcalignment] Second Ensembl QC Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentOverlapOnTarget(const EnsPQcalignment qca1,
                                     const EnsPQcalignment qca2)
{
    ajint start1 = 0;
    ajint start2 = 0;
    ajint end1 = 0;
    ajint end2 = 0;
    ajint strand1 = 0;
    ajint strand2 = 0;

    ajuint length1 = 0;
    ajuint length2 = 0;

    if(!qca1)
        return ajFalse;

    if(!qca2)
        return ajFalse;

    /* Check for identical target sequence objects. */

    if(!ensQcsequenceMatch(qca1->TargetSequence, qca2->TargetSequence))
        return ajFalse;

    /* Check for identical target strands. */

    if(qca1->TargetStrand != qca2->TargetStrand)
        return 0;

    /*
    ** Determine the relative orientation of the target sequences in the
    ** alignment.
    */

    ensQcalignmentGetTargetCoordinates(qca1,
                                       &start1,
                                       &end1,
                                       &strand1,
                                       &length1);

    ensQcalignmentGetTargetCoordinates(qca2,
                                       &start2,
                                       &end2,
                                       &strand2,
                                       &length2);

    /* Overlap criterion. */

    if((start1 <= end2) && (end1 >= start2))
        return ajTrue;

    return ajFalse;
}




/* @func ensQcalignmentCalculateQueryCoverage ********************************
**
** Calculates the alignment coverage score for the query sequence and sets it
** in the coverage attribute of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold to classify cases where only few
**                          residues at the edge are different.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

AjBool ensQcalignmentCalculateQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    AjBool value = AJFALSE;

    EnsPQcdatabase qqcdb = NULL;
    EnsPQcdatabase tqcdb = NULL;

    if(!qca)
        return ajFalse;

    qqcdb = ensQcsequenceGetQcdatabase(qca->QuerySequence);

    tqcdb = ensQcsequenceGetQcdatabase(qca->TargetSequence);

    switch(ensQcdatabaseGetClass(tqcdb))
    {
        case ensEQcdatabaseClassGenome:

            switch(ensQcdatabaseGetType(qqcdb))
            {
                case ensEQcdatabaseTypeProtein:
                    value =
                        ensQcalignmentCalculateProteinToGenomeQueryCoverage(
                            qca,
                            identity,
                            edge);

                    break;

                case ensEQcdatabaseTypeDNA:
                    value =
                        ensQcalignmentCalculateDNAToGenomeQueryCoverage(
                            qca,
                            identity,
                            edge);

                    break;

                default:
                    ajWarn("Cannot calculate a query to genome alignment "
                           "coverage "
                           "for a database of type '%s'.",
                           ensQcdatabaseTypeToChar(
                               ensQcdatabaseGetType(qqcdb)));
            }

            break;

        default:

            switch(ensQcdatabaseGetType(qqcdb))
            {
                case ensEQcdatabaseTypeProtein:
                    value =
                        ensQcalignmentCalculateProteinToProteinQueryCoverage(
                            qca,
                            identity);

                    break;

                case ensEQcdatabaseTypeDNA:
                    value =
                        ensQcalignmentCalculateDNAToDNAQueryCoverage(
                            qca,
                            identity,
                            edge);

                    break;

                default:
                    ajWarn("Cannot calculate a query alignment coverage for "
                           "a database of type '%s'.",
                           ensQcdatabaseTypeToChar(
                               ensQcdatabaseGetType(qqcdb)));
            }
    }

    return value;
}




/* Protein to Protein Query Coverage Scoring Schema ***************************
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

static const char *qcalignmentProteinToProteinQueryCoverageProperties[] =
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




/* @func ensQcalignmentCalculateProteinToProteinQueryCoverage *****************
**
** Calculates the alignment coverage score for a query of type 'protein'
** against a target of type 'protein' and sets the score in the coverage
** element of an Ensembl QC Alignment.
**
** @param [r] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identity [float] Identity threshold
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

AjBool ensQcalignmentCalculateProteinToProteinQueryCoverage(
    EnsPQcalignment qca,
    float identity)
{
    ajuint qlength = 0;
    ajuint tlength = 0;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;
    ajint tstart  = 0;
    ajint tend    = 0;
    ajint tstrand = 0;

    if(!qca)
        return ajFalse;

    qca->Score = 0;

    /* An Alignment object without a target automatically has coverage 0. */

    if(!qca->TargetSequence)
        return ajTrue;

    /*
    ** A query to target alignment is there, which is already good.
    ** (1*2**0=1)
    */

    qca->Coverage += 1;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentGetQueryCoordinates(qca,
                                      &qstart,
                                      &qend,
                                      &qstrand,
                                      &qlength);

    ensQcalignmentGetTargetCoordinates(qca,
                                       &tstart,
                                       &tend,
                                       &tstrand,
                                       &tlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);

    tlength = ensQcsequenceGetLength(qca->TargetSequence);

    /* Test for completeness of the N-terminus. */

    if(qstart == 0)
    {
        /* Perfect N-terminus (1*2**12=4096) */
        if(tstart == 0)
            qca->Coverage += 4096;
        /* Added start codon (1*2**10=1024) */
        else if(tstart == 1)
            qca->Coverage += 1024;
        /* Longer N-terminus (1*2**4=16) */
        else if(tstart > 1)
            qca->Coverage += 16;
        else
            ajWarn("Error in N-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if(qstart == 1)
    {
        /* Missing start codon (1*2**8=256) */
        if(tstart == 0)
            qca->Coverage += 256;
        /* Non-matching start codon (1*2**6=64) */
        else if(tstart == 1)
            qca->Coverage += 64;
        /* Non-matching N-terminus (0*2**1=0) */
        else if(tstart > 1)
            qca->Coverage += 0;
        else
            ajWarn("Error in N-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if(qstart > 1)
    {
        /* Shorter N-terminus (1*2**2=4) */
        if(tstart == 0)
            qca->Coverage += 4;
        /* Non-matching N-terminus (0*2**1=0) */
        else if(tstart > 0)
            qca->Coverage += 0;
        else
            ajWarn("Error in N-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else
        ajWarn("Error in N-terminus query coverage scoring schema. "
               "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);

    /* Test for completeness of the C-terminus. */

    if(qend == (ajint) qlength)
    {
        /* Perfect C-terminus (1*2**11=2048) */
        if(tend == (ajint) tlength)
            qca->Coverage += 2048;
        /* Added stop codon (1*2**9=512) */
        else if(tend == (ajint) (tlength - 1))
            qca->Coverage += 512;
        /* Longer C-terminus (1*2**3=8) */
        else if(tend < (ajint) (tlength - 1))
            qca->Coverage += 8;
        else
            ajWarn("Error in C-terminus coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else if(qend == (ajint) (qlength - 1))
    {

        /* Missing stop codon (1*2**7=128) */
        if(tend == (ajint) tlength)
            qca->Coverage += 128;
        /* Non-matching stop codon (1*2**5=32) */
        else if(tend == (ajint) (tlength - 1))
            qca->Coverage += 32;
        /* Non-matching C-terminus (0*2**2=0) */
        else if(tend < (ajint) (tlength - 1))
            qca->Coverage += 0;
        else
            ajWarn("Error in C-terminus coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);

    }
    else if(qend < (ajint) (qlength - 1))
    {
        /* Shorter C-terminus (1*2**1=2) */
        if(tend == (ajint) tlength)
            qca->Coverage += 2;
        /* Non-matching C-terminus (0*2**2=0) */
        else if(tend < (ajint) tlength)
            qca->Coverage += 0;
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

    if((qca->Coverage & 6144) && (qca->Identity >= identity))
        qca->Coverage += 8192;

    return ajTrue;
}




/* Protein to Genome Query Coverage Scoring Schema ****************************
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

static const char *qcalignmentProteinToGenomeQueryCoverageProperties[] =
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




/* @func ensQcalignmentCalculateProteinToGenomeQueryCoverage ******************
**
** Calculates the alignment coverage score for a query of type 'protein'
** against a target of class 'genome' and sets the score in the coverage
** element of an Ensembl QC Alignment.
**
** @param [r] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

AjBool ensQcalignmentCalculateProteinToGenomeQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    ajuint qlength = 0;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;

    if(!qca)
        return ajFalse;

    qca->Coverage = 0;

    /* An Alignment object without a target automatically has coverage 0. */

    if(!qca->TargetSequence)
        return ajTrue;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentGetQueryCoordinates(qca,
                                      &qstart,
                                      &qend,
                                      &qstrand,
                                      &qlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);

    /*
    ** A query to target alignment is there, which is already good.
    ** (1*2**0=1)
    */

    qca->Coverage += 1;

    /* Test for completeness of the N-terminus. */

    /* Perfect N-terminus (1*2**6=64) */
    if(qstart == 0)
        qca->Coverage += 64;
    /* Edge threshold (1*2**4=16) */
    else if(qstart <= (ajint) edge)
        qca->Coverage += 16;
    /* Shorter N-terminus (1*2**2=4) */
    else if(qstart > (ajint) edge)
        qca->Coverage += 4;
    else
        ajWarn("Error in N-terminus query coverage scoring schema. "
               "ID: %d QS: %d", qca->Identifier, qstart);

    /* Test for completeness of the C-terminus. */

    /* Perfect C-terminus (1*2**5=32) */
    if(qend == (ajint) qlength)
        qca->Coverage += 32;
    /* Edge threshold (1*2**3=8) */
    else if(qend >= (ajint) (qlength - edge))
        qca->Coverage += 8;
    /* Shorter C-terminus (1*2**1=2) */
    else if(qend < (ajint) (qlength - edge))
        qca->Coverage += 2;
    else
        ajWarn("Error in C-terminus coverage scoring schema. "
               "ID: %u QE: %d QL: %u", qca->Identifier, qend, qlength);

    /*
    ** Test for sequence identity over the identity threshold value
    ** only for perfect N and C-termini.
    ** (1*2**6 + 1*2**5 = 64 + 32 = 96)
    ** Identity threshold value (1*2**7=128)
    */

    if((qca->Coverage & 96) && (qca->Identity >= identity))
        qca->Coverage += 128;

    return ajTrue;
}




/* DNA to DNA Query Coverage Scoring Schema ***********************************
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

static const char *qcalignmentDNAToDNAQueryCoverageProperties[] =
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




/* @func ensQcalignmentCalculateDNAToDNAQueryCoverage *************************
**
** Calculates the alignment coverage score for a query of type 'dna'
** against a target of type 'dna' and sets the score in the coverage
** element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold to classify cases where only few
**                          residues at the edge are different.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

AjBool ensQcalignmentCalculateDNAToDNAQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    ajuint qlength = 0;
    ajuint tlength = 0;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;
    ajint tstart  = 0;
    ajint tend    = 0;
    ajint tstrand = 0;

    if(!qca)
        return ajFalse;

    qca->Coverage = 0;

    /* An Alignment object without a target automatically has coverage 0. */

    if(!qca->TargetSequence)
        return ajTrue;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentGetQueryCoordinates(qca,
                                      &qstart,
                                      &qend,
                                      &qstrand,
                                      &qlength);

    ensQcalignmentGetTargetCoordinates(qca,
                                       &tstart,
                                       &tend,
                                       &tstrand,
                                       &tlength);

    /* Correct sequence lengths for PolyA+ tails. */

    qlength = ensQcsequenceGetLength(qca->QuerySequence);

    qlength -= ensQcsequenceGetPolyA(qca->QuerySequence);

    tlength = ensQcsequenceGetLength(qca->TargetSequence);

    tlength -= ensQcsequenceGetPolyA(qca->TargetSequence);

    /*
    ** A query to target alignment is there, which is already good.
    ** (1*2**0=1)
    */

    qca->Coverage += 1;

    /* Test for completeness of the 5'-terminus. */

    if(qstart == 0)
    {
        /* Perfect 5'-terminus (1*2**12=4096) */
        if(tstart == 0)
            qca->Coverage += 4096;
        /* Longer 5'-edge (1*2**10=1024) */
        else if(tstart <= (ajint) edge)
            qca->Coverage += 1024;
        /* Longer 5'-region (1*2**4=16) */
        else if(tstart > (ajint) edge)
            qca->Coverage += 16;
        else
            ajWarn("Error in 5'-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if(qstart <= (ajint) edge)
    {
        /* Shorter 5'-edge (1*2**8=256) */
        if(tstart == 0)
            qca->Coverage += 256;
        /* Non-matching 5'-edge (1*2**6=64) */
        else if(tstart <= (ajint) edge)
            qca->Coverage += 64;
        /* Non-matching 5'-region (0*2**0=0) */
        else if(tstart > (ajint) edge)
            qca->Coverage += 0;
        else
            ajWarn("Error in 5'-terminus query coverage scoring schema. "
                   "ID: %u QS: %d TS: %d", qca->Identifier, qstart, tstart);
    }
    else if(qstart > (ajint) edge)
    {
        /* Shorter 5'-region (1*2**2=4) */
        if(tstart == 0)
            qca->Coverage += 4;
        /* Non-matching 5'-region (0*2**0=0) */
        else if(tstart > 0)
            qca->Coverage += 0;
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
    ** could be longer than the artificially truncated qlength.
    **
    ** The same is true for the target length so that for each query case
    ** another condition testing for a target_end exceeding the
    ** tlength is required.  Currently, scores as in the case of
    ** perfect overlaps are set.
    **
    ** FIXME???
    */

    if(qend >= (ajint) qlength)
    {
        /*
        ** Perfect 3'-terminus, including over-estimated Poly A tails.
        ** (1*2**11=2048)
        */
        if(tend >= (ajint) tlength)
            qca->Coverage += 2048;
        /* Longer 3'-edge (1*2**9=512) */
        else if(tend >= (ajint) (tlength - edge))
            qca->Coverage += 512;
        /* Longer 3'-region (1*2**3=8) */
        else if(tend < (ajint) (tlength - edge))
            qca->Coverage += 8;
        else
            ajWarn("Error in 3'-terminus query coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);
    }
    else if(qend >= (ajint) (qlength - edge))
    {
        /* Added 3'-edge, including over-estimated Poly A tails. (1*2**7=128) */
        if(tend >= (ajint) tlength)
            qca->Coverage += 128;
        /* Non-matching 3'-edge (1*2**5=32) */
        else if(tend >= (ajint) (tlength - edge))
            qca->Coverage += 32;
        /* Non-matching 3-'terminus (0*2**2=0) */
        else if(tend < (ajint) (tlength - edge))
            qca->Coverage += 0;
        else
            ajWarn("Error in 3'-terminus query coverage scoring schema. "
                   "ID: %u QE: %d QL: %u TE: %d TL: %u",
                   qca->Identifier, qend, qlength, tend, tlength);

    }
    else if(qend < (ajint) (qlength - edge))
    {
        /* Shorter 3'-terminus (1*2**1=2) */
        if(tend >= (ajint) tlength)
            qca->Coverage += 2;
        /* Non-matching 3'-terminus (0*2**0=0) */
        else if(tend < (ajint) tlength)
            qca->Coverage += 0;
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

    if((qca->Coverage & 6144) && (qca->Identity >= identity))
        qca->Coverage += 8192;

    return ajTrue;
}




/* cDNA to Genome Query Coverage Scoring Schema *******************************
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

static const char *qcalignmentDNAToGenomeQueryCoverageProperties[] =
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




/* @func ensQcalignmentCalculateDNAToGenomeQueryCoverage **********************
**
** Calculates the alignment coverage score for a query of type 'dna'
** against a target of class 'genome' and sets the score in the coverage
** element of an Ensembl QC Alignment.
**
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
** @param [r] identity [float] Identity threshold
** @param [r] edge [ajuint] Edge threshold to classify cases where only few
**                          residues at the edge fall into a separate exon.
**                          This is generally difficult to predict for
**                          alignment programs.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

AjBool ensQcalignmentCalculateDNAToGenomeQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge)
{
    ajuint qlength = 0;

    ajint qstart  = 0;
    ajint qend    = 0;
    ajint qstrand = 0;

    if(!qca)
        return ajFalse;

    qca->Coverage = 0;

    /* An Alignment object without a target automatically has coverage 0. */

    if(!qca->TargetSequence)
        return ajTrue;

    /* Assign alignment coordinates strand-dependent. */

    ensQcalignmentGetQueryCoordinates(qca,
                                      &qstart,
                                      &qend,
                                      &qstrand,
                                      &qlength);

    qlength = ensQcsequenceGetLength(qca->QuerySequence);

    qlength -= ensQcsequenceGetPolyA(qca->QuerySequence);

    /*
    ** A query to target alignment is there, which is already good.
    ** (1*2**0=1)
    */

    qca->Coverage += 1;

    /* Test for completeness of the 5' terminus. */

    /* Perfect 5' terminus (1*2**6=64) */
    if(qstart == 0)
        qca->Coverage += 64;
    /* Edge threshold (1*2**4=16) */
    else if(qstart <= (ajint) edge)
        qca->Coverage += 16;
    /* Shorter 5' terminus (1*2**2=4) */
    else if(qstart > (ajint) edge)
        qca->Coverage += 4;
    else
        ajWarn("Error in 5'-terminus query coverage scoring schema. "
               "ID: %u QS: %d", qca->Identifier, qstart);

    /*
    ** Test for completeness of the 3' terminus.  If part of a PolyA tail
    ** aligns to the genome sequence it may appear longer so that the end
    ** of the alignment actually extends the clipped mRNA sequence.
    */

    /* Perfect 3' terminus (1*2**5=32) */
    if(qend >= (ajint) qlength)
        qca->Coverage += 32;
    /* Edge threshold (1*2**3=8) */
    else if(qend >= (ajint) (qlength - edge))
        qca->Coverage += 8;
    /* 3' terminus shorter (1*2**1=2) */
    else if(qend < (ajint) (qlength - edge))
        qca->Coverage += 2;
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

    if((qca->Coverage & 96) && (qca->Identity >= identity))
        qca->Coverage += 128;

    return ajTrue;
}




/* Query to Query Comparison Scoring Schema ***********************************
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

static const char* qcalignmentQueryToQueryCoverageProperties[] =
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




/* @func ensQcalignmentCalculateQueryToQueryCoverage **************************
**
** Compares and scores two Ensembl QC Alignment objects based on the coverage
** of the query sequences in the alignment. The score is the sum of 5' and 3'
** properties, which can be 'longer', 'equal' or 'shorter' in respect to the
** coverage of the query in the second alignment.
** The first alignment is usually the genome alignment the second one the
** reference to test alignment.
**
** @param [r] qca1 [const EnsPQcalignment] First Ensembl QC Alignment
** @param [r] qca2 [const EnsPQcalignment] Second Ensembl QC Alignment
**
** @return [ajuint] Score
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

ajuint ensQcalignmentCalculateQueryToQueryCoverage(
    const EnsPQcalignment qca1,
    const EnsPQcalignment qca2)
{
    ajuint score = 0;

    ajint first_start = 0;
    ajint first_end   = 0;

    ajint second_start = 0;
    ajint second_end   = 0;

    if(!qca1)
        return ajFalse;

    /*
    ** If either of the Alignment objects has no target assigned, the score is
    ** automatically 0.
    */

    if(!qca1->TargetSequence)
        return score;

    if(!qca2)
        return score;

    if(!qca2->TargetSequence)
        return score;

    /* The first Alignment object is always defined. (1*2**0=1) */

    score += 1;

    if(qca2)
    {
        /* Check whether the query sequence is the same in both alignments. */

        /* FIXME: Should this use ensQcsequenceMatch? */
        if(qca1->QuerySequence != qca2->QuerySequence)
            return 0;

        /* The second Alignment object is defined. (1*2**1=2) */

        score += 2;

        /*
        ** Determine the relative orientation of the query sequences in the
        ** alignment.
        */

        if(qca1->QueryStrand == qca2->QueryStrand)
        {
            /* Parallel query sequences. */

            first_start  = qca1->QueryStart;
            first_end    = qca1->QueryEnd;

            second_start = qca2->QueryStart;
            second_end   = qca2->QueryEnd;
        }
        else
        {
            /* Anti-parallel query sequences. */

            first_start  = qca1->QueryStart;
            first_end    = qca1->QueryEnd;

            second_start = qca2->QueryEnd;
            second_end   = qca2->QueryStart;
        }

        /* Evaluate query start coordinates. */

        /* The first alignment is longer. (1*2**5=32) */
        if(first_start < second_start)
            score += 32;
        /* The first alignment is as long as the second. (1*2**7=128) */
        else if(first_start == second_start)
            score += 128;
        /* The first alignment is shorter. (1*2**3=8) */
        else if(first_start > second_start)
            score += 8;
        else
            ajWarn("Unexpected query start coordinate relationship.");

        /* Evaluate query end coordinates. */

        /* The first alignment is longer. (1*2**4=16) */
        if(first_end > second_end)
            score += 16;
        /* The first alignment is as long as the second. (1*2**6=64) */
        else if(first_end == second_end)
            score += 64;
        /* The first alignment is shorter. (1*2**2=4) */
        else if(first_end < second_end)
            score += 4;
        else
            ajWarn("Unexpected query end coordinate releationship.");
    }

    /*
    ** Test for perfect query coverage in genome alignments. See
    ** ensQcalignmentCalculateProteinToGenomeQueryCoverage and
    ** ensQcalignmentCalculateDNAToGenomeQueryCoverage for details.
    ** Since this method is called for the genome alignment, the test
    ** refers to the first alignment (qca1).
    */

    /* Perfect N- or 5'-terminus (1*2**6=64) */
    if(qca1->Coverage & 64)
        score += 512;

    /* Perfect C- or 3'-terminus (1*2**5=32) */
    if(qca1->Coverage & 32)
        score += 256;

    return score;
}




/* @func ensQcalignmentReport *************************************************
**
** Report an Ensembl QC Alignment in the following way:
** query:start-end:strand (length)
** target:start-end:strand (length)
**
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment Adaptor
** @param [w] Pstr [AjPStr *] AJAX String
** @param [r] internalquery [AjBool] HTML document-internal query link
** @param [r] internaltarget [AjBool] HTML document-internal target link
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    ajuint length = 0;

    AjPStr anchor   = NULL;
    AjPStr exturl   = NULL;
    AjPStr location = NULL;

    EnsPQcdatabase qcdb = NULL;

    if(!qca)
        return ajFalse;

    /* Format the query part of the Alignment object. */

    ensQcalignmentGetQueryCoordinates(qca,
                                      &start,
                                      &end,
                                      &strand,
                                      &length);

    qcdb = ensQcsequenceGetQcdatabase(qca->QuerySequence);

    if(internalquery)
    {
        /* A HTML document-internal link is requested. */

        anchor = ajStrNew();

        ensQcsequenceFetchInternalAnchor(qca->QuerySequence, &anchor);

        ajFmtPrintAppS(Pstr, "%S:%d-%d:%d", anchor, start, end, strand);

        ajStrDel(&anchor);
    }
    else if(ensQcdatabaseGetExternalURL(qcdb))
    {
        /* An URL has been set in the sequence database definition. */

        exturl = ajStrNewS(ensQcdatabaseGetExternalURL(qcdb));

        if(ajStrFindC(exturl, "###LOCATION###"))
        {
            /* ###LOCATION### links to Location/View. */

            location = ajFmtStr("%S:%d-%d",
                                ensQcsequenceGetName(qca->QuerySequence),
                                start,
                                end);

            ajStrExchangeCS(&exturl, "###LOCATION###", location);

            ensHTMLEncodeEntities(&exturl);

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

            ensQcsequenceFetchExternalAnchor(qca->QuerySequence,
                                             &anchor,
                                             ajFalse);

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

    if(!qca->TargetSequence)
        return ajTrue;

    /* Format the target part of the Alignment object. */

    ensQcalignmentGetTargetCoordinates(qca,
                                       &start,
                                       &end,
                                       &strand,
                                       &length);

    qcdb = ensQcsequenceGetQcdatabase(qca->TargetSequence);

    ajStrAppendC(Pstr, "\t");

    if(internaltarget)
    {
        /* A HTML document-internal link has been requested. */

        anchor = ajStrNew();

        ensQcsequenceFetchInternalAnchor(qca->TargetSequence, &anchor);

        ajFmtPrintAppS(Pstr, "%S:%d-%d:%d", anchor, start, end, strand);

        ajStrDel(&anchor);
    }
    else if(ensQcdatabaseGetExternalURL(qcdb))
    {
        /* URL has been set in the sequence database definition. */

        exturl = ajStrNewS(ensQcdatabaseGetExternalURL(qcdb));

        if(ajStrFindC(exturl, "###LOCATION###"))
        {

            /* ###LOCATION### links to Location/View. */

            location = ajFmtStr("%S:%d-%d",
                                ensQcsequenceGetName(qca->TargetSequence),
                                start,
                                end);

            ajStrExchangeCS(&exturl, "###LOCATION###", location);

            ensHTMLEncodeEntities(&exturl);

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

            ensQcsequenceFetchExternalAnchor(qca->TargetSequence,
                                             &anchor,
                                             ajFalse);

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




/* @datasection [EnsPQcalignmentadaptor] QC Alignment Adaptor *****************
**
** Functions for manipulating Ensembl QC Alignment Adaptor objects
**
** Bio::EnsEMBL::QC::DBSQL::Alignmentadaptor CVS Revision:
**
** @nam2rule Qcalignmentadaptor
**
******************************************************************************/

static const char *qcalignmentadaptorTables[] =
{
    "alignment",
    (const char *) NULL
};




static const char *qcalignmentadaptorColumns[] =
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




static EnsOBaseadaptorLeftJoin qcalignmentadaptorLeftJoin[] =
{
    {(const char*) NULL, (const char*) NULL}
};




static const char *qcalignmentadaptorDefaultCondition =
    (const char*) NULL;

static const char *qcalignmentadaptorFinalCondition =
    (const char *) NULL;




/* @funcstatic qcalignmentadaptorFetchAllBySQL ********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl QC Alignment objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcas [AjPList] AJAX List of Ensembl QC Alignments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcalignmentadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                              const AjPStr statement,
                                              EnsPAssemblymapper am,
                                              EnsPSlice slice,
                                              AjPList qcas)
{
    double score   = 0.0;
    float identity = 0.0;

    ajint qstrand = 0;
    ajint tstrand = 0;
    ajint sstrand = 0;

    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint coverage   = 0;
    ajuint qdbid  = 0;
    ajuint qsid   = 0;
    ajuint qstart = 0;
    ajuint qend   = 0;
    ajuint tdbid  = 0;
    ajuint tsid   = 0;
    ajuint tstart = 0;
    ajuint tend   = 0;


    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr vulgar = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPQcalignment qca         = NULL;
    EnsPQcalignmentadaptor qcaa = NULL;

    EnsPQcsequence qsequence   = NULL;
    EnsPQcsequence tsequence   = NULL;
    EnsPQcsequenceadaptor qcsa = NULL;

    if(ajDebugTest("qcalignmentadaptorFetchAllBySQL"))
        ajDebug("qcalignmentadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcas %p\n",
                dba,
                statement,
                am,
                slice,
                qcas);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!qcas)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    qcaa = ensRegistryGetQcalignmentadaptor(dba);

    qcsa = ensRegistryGetQcsequenceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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
        identity   = 0.0;
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

        qca = ensQcalignmentNew(qcaa,
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
** All constructors return a new Ensembl QC Alignment Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** QC Alignment Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcalignmentadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensQcalignmentadaptorNew *********************************************
**
** Default constructor for an Ensembl QC Alignment Adaptor.
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor or NULL
** @@
******************************************************************************/

EnsPQcalignmentadaptor ensQcalignmentadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        qcalignmentadaptorTables,
        qcalignmentadaptorColumns,
        qcalignmentadaptorLeftJoin,
        qcalignmentadaptorDefaultCondition,
        qcalignmentadaptorFinalCondition,
        qcalignmentadaptorFetchAllBySQL);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl QC Alignment Adaptor.
**
** @fdata [EnsPQcalignmentadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) a QC Alignment Adaptor object
**
** @argrule * Pqcaa [EnsPQcalignmentadaptor*] QC Alignment Adaptor
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcalignmentadaptorDel *********************************************
**
** Default destructor for an Ensembl QC Alignment Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcaa [EnsPQcalignmentadaptor*] Ensembl QC Alignment Adaptor
**                                            address
**
** @return [void]
** @@
******************************************************************************/

void ensQcalignmentadaptorDel(EnsPQcalignmentadaptor *Pqcaa)
{
    if(!Pqcaa)
        return;

    if(!*Pqcaa)
        return;

    if(ajDebugTest("ensQcalignmentadaptorDel"))
        ajDebug("ensQcalignmentadaptorDel\n"
                "  *Pqcaa %p\n",
                *Pqcaa);

    ensBaseadaptorDel(Pqcaa);

    return;
}




/* @func ensQcalignmentadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl QC Alignment via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl QC Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [r] identifier [ajuint] SQL database-internal QC Alignment identifier
** @param [wP] Pqca [EnsPQcalignment*] Ensembl QC Alignment address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchByIdentifier(EnsPQcalignmentadaptor qcaa,
                                              ajuint identifier,
                                              EnsPQcalignment *Pqca)
{
    if(!qcaa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pqca)
        return ajFalse;

    *Pqca = (EnsPQcalignment)
        ensBaseadaptorFetchByIdentifier(qcaa, identifier);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllByAnalysisQueryTarget *******************
**
** Fetch all Ensembl QC Alignments by Ensembl an Analysis, as well as
** Query and Target Ensembl Sequences.
** The caller is responsible for deleting the Ensembl QC Alignments
** before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl QC Database
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl QC Database
** @param [u] qcas [AjPList] AJAX List of Ensembl QC Alignments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllByAnalysisQueryTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcas)
{
    AjPStr constraint = NULL;

    if(!qcaa)
        return ajFalse;

    if(!analysis)
        return ajFalse;

    if(!qdb)
        return ajFalse;

    if(!tdb)
        return ajFalse;

    if(!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.analysis_id = %u "
                          "AND "
                          "alignment.query_db_id = %u "
                          "AND "
                          "alignment.target_db_id = %u",
                          ensAnalysisGetIdentifier(analysis),
                          ensQcdatabaseGetIdentifier(qdb),
                          ensQcdatabaseGetIdentifier(tdb));

    ensBaseadaptorGenericFetch(qcaa,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllByQuery *********************************
**
** Fetch all Ensembl QC Alignments by a Query Ensembl QC Sequence.
** The caller is responsible for deleting the Ensembl QC Alignments
** before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl QC Database
** @param [u] qcas [AjPList] AJAX List of Ensembl QC Alignments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllByQuery(EnsPQcalignmentadaptor qcaa,
                                            const EnsPAnalysis analysis,
                                            const EnsPQcdatabase qdb,
                                            AjPList qcas)
{
    AjPStr constraint = NULL;

    if(!qcaa)
        return ajFalse;

    if(!qdb)
        return ajFalse;

    if(!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.query_db_id = %u",
                          ensQcdatabaseGetIdentifier(qdb));

    if(analysis)
        ajFmtPrintAppS(&constraint,
                       " AND alignment.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorGenericFetch(qcaa,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllByTarget ********************************
**
** Fetch all Ensembl QC Alignments by a Target Ensembl QC Sequence.
** The caller is responsible for deleting the Ensembl QC Alignments
** before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl QC Database
** @param [u] qcas [AjPList] AJAX List of Ensembl QC Alignments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllByTarget(EnsPQcalignmentadaptor qcaa,
                                             const EnsPAnalysis analysis,
                                             const EnsPQcdatabase tdb,
                                             AjPList qcas)
{
    AjPStr constraint = NULL;

    if(!qcaa)
        return ajFalse;

    if(!tdb)
        return ajFalse;

    if(!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.target_db_id = %u",
                          ensQcdatabaseGetIdentifier(tdb));

    if(analysis)
        ajFmtPrintAppS(&constraint,
                       " AND alignment.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorGenericFetch(qcaa,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllByCoverage ******************************
**
** Fetch all Ensembl QC Alignments by a coverage score range.
** The caller is responsible for deleting the Ensembl QC Alignments
** before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [r] lower [ajuint] Lower coverage score
** @param [r] upper [ajuint] Upper coverage score
** @param [u] qcas [AjPList] AJAX List of Ensembl QC Alignments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllByCoverage(EnsPQcalignmentadaptor qcaa,
                                               ajuint lower,
                                               ajuint upper,
                                               AjPList qcas)
{
    AjPStr constraint = NULL;

    if(!qcaa)
        return ajFalse;

    if(!qcas)
        return ajFalse;

    constraint = ajFmtStr("alignment.coverage >= %u "
                          "AND "
                          "alignment.coverage <= %u",
                          lower, upper);

    ensBaseadaptorGenericFetch(qcaa,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorFetchAllByTargetLocation ************************
**
** Fetch all Ensembl QC Alignments by a target location.
** The caller is responsible for deleting the Ensembl QC Alignments
** before deleting the AJAX List.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] tsequence [const EnsPQcsequence] Target Ensembl QC Sequence
** @param [r] tstart [ajuint] Target start
** @param [r] tend [ajuint] Target end
** @param [u] qcas [AjPList] AJAX List of Ensembl QC Alignments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorFetchAllByTargetLocation(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence tsequence,
    ajuint tstart,
    ajuint tend,
    AjPList qcas)
{
    AjPStr constraint = NULL;

    if(!qcaa)
        return ajFalse;

    if(!analysis)
        return ajFalse;

    if(!tsequence)
        return ajFalse;

    if(!qcas)
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
    ** Since Ensembl QC Alignment object use the exonerate coordinate
    ** system, features on the reverse strand have a target start greater
    ** than the target end. Therefore, one SQL query per strand is
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

    ensBaseadaptorGenericFetch(qcaa,
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

    ensBaseadaptorGenericFetch(qcaa,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcalignmentadaptorStore *******************************************
**
** Store an Ensembl QC Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorStore(EnsPQcalignmentadaptor qcaa,
                                  EnsPQcalignment qca)
{
    char *txtvulgar = NULL;

    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcaa)
        return ajFalse;

    if(!qca)
        return ajFalse;

    if(ensQcalignmentGetAdaptor(qca) &&
       ensQcalignmentGetIdentifier(qca))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcaa);

    ensDatabaseadaptorEscapeC(dba, &txtvulgar, qca->VULGAR);

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
        qca->SpliceStrand,
        qca->Score,
        qca->Identity,
        txtvulgar,
        qca->Coverage);

    ajCharDel(&txtvulgar);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcalignmentSetIdentifier(qca, ajSqlstatementGetIdentifier(sqls));

        ensQcalignmentSetAdaptor(qca, qcaa);

        value = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcalignmentadaptorUpdate ******************************************
**
** Update an Ensembl QC Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [r] qca [const EnsPQcalignment] Ensembl QC Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorUpdate(EnsPQcalignmentadaptor qcaa,
                                   const EnsPQcalignment qca)
{
    char *txtvulgar = NULL;

    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcaa)
        return ajFalse;

    if(!qca)
        return ajFalse;

    if(!ensQcalignmentGetIdentifier(qca))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcaa);

    ensDatabaseadaptorEscapeC(dba, &txtvulgar, qca->VULGAR);

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
        qca->SpliceStrand,
        qca->Score,
        qca->Identity,
        txtvulgar,
        qca->Coverage,
        qca->Identifier);

    ajCharDel(&txtvulgar);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
        value = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcalignmentadaptorDelete ******************************************
**
** Delete an Ensembl QC Alignment.
**
** @param [u] qcaa [EnsPQcalignmentadaptor] Ensembl QC Alignment Adaptor
** @param [u] qca [EnsPQcalignment] Ensembl QC Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcalignmentadaptorDelete(EnsPQcalignmentadaptor qcaa,
                                   EnsPQcalignment qca)
{
    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcaa)
        return ajFalse;

    if(!qca)
        return ajFalse;

    if(!ensQcalignmentGetIdentifier(qca))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcaa);

    statement = ajFmtStr(
        "DELETE FROM "
        "alignment "
        "WHERE "
        "alignment.alignment_id = %u",
        qca->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        qca->Adaptor    = (EnsPQcalignmentadaptor) NULL;
        qca->Identifier = 0;

        value = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcalignmentDummyFunction ******************************************
**
** Dummy function to catch all unused functions defined in the
** ensqcalignment source file.
**
** @return [void]
**
******************************************************************************/

void ensQcalignmentDummyFunction(void)
{
    (void) qcalignmentProteinToProteinQueryCoverageProperties[0];
    (void) qcalignmentProteinToGenomeQueryCoverageProperties[0];
    (void) qcalignmentDNAToDNAQueryCoverageProperties[0];
    (void) qcalignmentDNAToGenomeQueryCoverageProperties[0];
    (void) qcalignmentQueryToQueryCoverageProperties[0];

    return;
}

/* @source ensqcvariation *****************************************************
**
** Ensembl Quality Check Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.16 $
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

#include "ensqcvariation.h"




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

/* @conststatic qcvariationKClass *********************************************
**
** The Ensembl Quality Check Variation class member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcvariationClass.
**
******************************************************************************/

static const char *qcvariationKClass[] =
{
    "",
    "none",
    "simple",
    "splice",
    "exon",
    (const char *) NULL
};




/* @conststatic qcvariationKType **********************************************
**
** The Ensembl Quality Check Variation type member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcvariationType.
**
******************************************************************************/

static const char *qcvariationKType[] =
{
    "",
    "none",
    "single",
    "multi",
    (const char *) NULL
};




/* @conststatic qcvariationKState *********************************************
**
** The Ensembl Quality Check Variation state member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcvariationState.
**
******************************************************************************/

static const char *qcvariationKState[] =
{
    "",
    "none",
    "match",
    "gap",
    "frameshift",
    "5'ss",
    "3'ss",
    "split",
    (const char *) NULL
};




/* @conststatic qcvariationadaptorKTables *************************************
**
** Array of Ensembl Quality Check Variation Adaptor SQL table names
**
******************************************************************************/

static const char *qcvariationadaptorKTables[] =
{
    "variation",
    (const char *) NULL
};




/* @conststatic qcvariationadaptorKColumns ************************************
**
** Array of Ensembl Quality Check Variation Adaptor SQL column names
**
******************************************************************************/

static const char *qcvariationadaptorKColumns[] =
{
    "variation.variation_id",
    "variation.analysis_id",
    "variation.alignment_id",
    "variation.query_db_id",
    "variation.query_id"
    "variation.query_start",
    "variation.query_end",
    "variation.query_seq",
    "variation.target_db_id"
    "variation.target_id",
    "variation.target_start",
    "variation.target_end",
    "variation.target_seq",
    "variation.class",
    "variation.type",
    "variation.state",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool qcvariationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcvs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensqc *********************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcvariation] Ensembl Quality Check Variation *************
**
** @nam2rule Qcvariation Functions for manipulating
** Ensembl Quality Check Variation objects
**
** @cc Bio::EnsEMBL::QC::Variation
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Variation by pointer.
** It is the responsibility of the user to first destroy any previous
** Quality Check Variation. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcvariation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy qcv [const EnsPQcvariation] Ensembl Quality Check Variation
** @argrule Ini qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini qsequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @argrule Ini qstart [ajuint] Query start
** @argrule Ini qend [ajuint] Query end
** @argrule Ini qstring [AjPStr] Query string
** @argrule Ini tsequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule Ini tstart [ajuint] Target start
** @argrule Ini tend [ajuint] Target end
** @argrule Ini tstring [AjPStr] Target string
** @argrule Ini class [EnsEQcvariationClass] Class
** @argrule Ini type [EnsEQcvariationType] Type
** @argrule Ini state [EnsEQcvariationState] State
** @argrule Ref qcv [EnsPQcvariation] Ensembl Quality Check Variation
**
** @valrule * [EnsPQcvariation] Ensembl Quality Check Variation or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcvariationNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsPQcvariation] Ensembl Quality Check Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcvariation ensQcvariationNewCpy(const EnsPQcvariation qcv)
{
    EnsPQcvariation pthis = NULL;

    if (!qcv)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = qcv->Identifier;

    pthis->Adaptor = qcv->Adaptor;

    pthis->Qcalignment = ensQcalignmentNewRef(qcv->Qcalignment);

    pthis->Analysis = ensAnalysisNewRef(qcv->Analysis);

    pthis->QuerySequence = ensQcsequenceNewRef(qcv->QuerySequence);
    pthis->QueryStart    = qcv->QueryStart;
    pthis->QueryEnd      = qcv->QueryEnd;

    if (qcv->QueryString)
        pthis->QueryString = ajStrNewRef(qcv->QueryString);

    pthis->TargetSequence = ensQcsequenceNewRef(qcv->TargetSequence);
    pthis->TargetStart    = qcv->TargetStart;
    pthis->TargetEnd      = qcv->TargetEnd;

    if (qcv->TargetString)
        pthis->TargetString = ajStrNewRef(qcv->TargetString);

    pthis->Class = qcv->Class;
    pthis->Type  = qcv->Type;
    pthis->State = qcv->State;

    return pthis;
}




/* @func ensQcvariationNewIni *************************************************
**
** Constructor for an Ensembl Quality Check Variation with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::Variation::new
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] qsequence [EnsPQcsequence] Query Ensembl Quality Check Sequence
** @param [r] qstart [ajuint] Query start
** @param [r] qend [ajuint] Query end
** @param [u] qstring [AjPStr] Query string
** @param [u] tsequence [EnsPQcsequence] Target Ensembl Quality Check Sequence
** @param [r] tstart [ajuint] Target start
** @param [r] tend [ajuint] Target end
** @param [u] tstring [AjPStr] Target string
** @param [u] class [EnsEQcvariationClass] Class
** @param [u] type [EnsEQcvariationType] Type
** @param [u] state [EnsEQcvariationState] State
**
** @return [EnsPQcvariation] Ensembl Quality Check Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcvariation ensQcvariationNewIni(EnsPQcvariationadaptor qcva,
                                     ajuint identifier,
                                     EnsPQcalignment qca,
                                     EnsPAnalysis analysis,
                                     EnsPQcsequence qsequence,
                                     ajuint qstart,
                                     ajuint qend,
                                     AjPStr qstring,
                                     EnsPQcsequence tsequence,
                                     ajuint tstart,
                                     ajuint tend,
                                     AjPStr tstring,
                                     EnsEQcvariationClass class,
                                     EnsEQcvariationType type,
                                     EnsEQcvariationState state)
{
    EnsPQcvariation qcv = NULL;

    if (!qca)
        return NULL;

    if (!analysis)
        return NULL;

    if (!qsequence)
        return NULL;

    if (!tsequence)
        return NULL;

    AJNEW0(qcv);

    qcv->Use = 1U;

    qcv->Identifier = identifier;

    qcv->Adaptor = qcva;

    qcv->Qcalignment = ensQcalignmentNewRef(qca);

    qcv->Analysis = ensAnalysisNewRef(analysis);

    qcv->QuerySequence = ensQcsequenceNewRef(qsequence);
    qcv->QueryStart    = qstart;
    qcv->QueryEnd      = qend;

    if (qstring)
        qcv->QueryString = ajStrNewRef(qstring);

    qcv->TargetSequence = ensQcsequenceNewRef(tsequence);
    qcv->TargetStart    = tstart;
    qcv->TargetEnd      = tend;

    if (tstring)
        qcv->TargetString = ajStrNewRef(tstring);

    qcv->Class = class;
    qcv->Type  = type;
    qcv->State = state;

    return qcv;
}




/* @func ensQcvariationNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Variation
**
** @return [EnsPQcvariation] Ensembl Quality Check Variation or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcvariation ensQcvariationNewRef(EnsPQcvariation qcv)
{
    if (!qcv)
        return NULL;

    qcv->Use++;

    return qcv;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check Variation object.
**
** @fdata [EnsPQcvariation]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Variation
**
** @argrule * Pqcv [EnsPQcvariation*] Ensembl Quality Check Variation address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcvariationDel ****************************************************
**
** Default destructor for an Ensembl Quality Check Variation.
**
** @param [d] Pqcv [EnsPQcvariation*] Ensembl Quality Check Variation address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcvariationDel(EnsPQcvariation *Pqcv)
{
    EnsPQcvariation pthis = NULL;

    if (!Pqcv)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensQcvariationDel"))
    {
        ajDebug("ensQcvariationDel\n"
                "  *Pqcv %p\n",
                *Pqcv);

        ensQcvariationTrace(*Pqcv, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pqcv)
        return;

    pthis = *Pqcv;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pqcv = NULL;

        return;
    }

    ensQcalignmentDel(&pthis->Qcalignment);

    ensAnalysisDel(&pthis->Analysis);

    ensQcsequenceDel(&pthis->QuerySequence);
    ensQcsequenceDel(&pthis->TargetSequence);

    ajStrDel(&pthis->QueryString);
    ajStrDel(&pthis->TargetString);

    AJFREE(pthis);

    *Pqcv = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check Variation object.
**
** @fdata [EnsPQcvariation]
**
** @nam3rule Get Return Quality Check Variation attribute(s)
** @nam4rule Adaptor Return the Ensembl Quality Check Variation Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Class Return the class
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Qcalignment Return the Ensembl Quality Check Alignment
** @nam4rule Query Return query members
** @nam5rule End Return the query end
** @nam5rule Sequence Return the query Ensembl Quality Check Sequence
** @nam5rule Start Return the query start
** @nam5rule String Return the query string
** @nam4rule State Return the state
** @nam4rule Target Return target members
** @nam5rule Sequence Return the target Ensembl Quality Check Sequence
** @nam5rule Start Return the target start
** @nam5rule End Return the target end
** @nam5rule String Return the target string
** @nam4rule Type Return the type
**
** @argrule * qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @valrule Adaptor [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Class [EnsEQcvariationClass] Class or ensEQcvariationClassNULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Qcalignment [EnsPQcalignment]
** Ensembl Quality Check Alignment or NULL
** @valrule QueryEnd [ajuint] Query end or 0U
** @valrule QuerySequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence or NULL
** @valrule QueryStart [ajuint] Query start or 0U
** @valrule QueryString [AjPStr] Query string or NULL
** @valrule State [EnsEQcvariationState] State or ensEQcvariationStateNULL
** @valrule TargetSequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence or NULL
** @valrule TargetStart [ajuint] Target start or 0U
** @valrule TargetEnd [ajuint] Target end or 0U
** @valrule TargetString [AjPStr] Target string or NULL
** @valrule Type [EnsEQcvariationType] Type or ensEQcvariationTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcvariationGetAdaptor *********************************************
**
** Get the Ensembl Quality Check Variation Adaptor member of an
** Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsPQcvariationadaptor] Ensembl Quality Check Variation Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcvariationadaptor ensQcvariationGetAdaptor(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->Adaptor : NULL;
}




/* @func ensQcvariationGetAnalysis ********************************************
**
** Get the Ensembl Analysis member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensQcvariationGetAnalysis(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->Analysis : NULL;
}




/* @func ensQcvariationGetClass ***********************************************
**
** Get the class member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsEQcvariationClass] Class or ensEQcvariationClassNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcvariationClass ensQcvariationGetClass(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->Class : ensEQcvariationClassNULL;
}




/* @func ensQcvariationGetIdentifier ******************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcvariationGetIdentifier(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->Identifier : 0U;
}




/* @func ensQcvariationGetQcalignment *****************************************
**
** Get the Ensembl Quality Check Alignment member of an
** Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsPQcalignment] Ensembl Quality Check Alignment or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcalignment ensQcvariationGetQcalignment(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->Qcalignment : NULL;
}




/* @func ensQcvariationGetQueryEnd ********************************************
**
** Get the query end member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [ajuint] Query end or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcvariationGetQueryEnd(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->QueryEnd : 0U;
}




/* @func ensQcvariationGetQuerySequence ***************************************
**
** Get the query Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsPQcsequence] Query Ensembl Quality Check Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcsequence ensQcvariationGetQuerySequence(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->QuerySequence : NULL;
}




/* @func ensQcvariationGetQueryStart ******************************************
**
** Get the query start member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [ajuint] Query start or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcvariationGetQueryStart(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->QueryStart : 0U;
}




/* @func ensQcvariationGetQueryString *****************************************
**
** Get the query string member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [AjPStr] Query string or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcvariationGetQueryString(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->QueryString : NULL;
}




/* @func ensQcvariationGetState ***********************************************
**
** Get the state member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsEQcvariationState] State or ensEQcvariationStateNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcvariationState ensQcvariationGetState(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->State : ensEQcvariationStateNULL;
}




/* @func ensQcvariationGetTargetEnd *******************************************
**
** Get the target end member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [ajuint] Target end or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcvariationGetTargetEnd(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->TargetEnd : 0U;
}




/* @func ensQcvariationGetTargetSequence **************************************
**
** Get the target Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsPQcsequence] Target Ensembl Quality Check Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcsequence ensQcvariationGetTargetSequence(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->TargetSequence : NULL;
}




/* @func ensQcvariationGetTargetStart *****************************************
**
** Get the target start member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [ajuint] Target start or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcvariationGetTargetStart(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->TargetStart : 0U;
}




/* @func ensQcvariationGetTargetString ****************************************
**
** Get the target string member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [AjPStr] Target string or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcvariationGetTargetString(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->TargetString : NULL;
}




/* @func ensQcvariationGetType ************************************************
**
** Get the type member of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [EnsEQcvariationType] Type or ensEQcvariationTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcvariationType ensQcvariationGetType(const EnsPQcvariation qcv)
{
    return (qcv) ? qcv->Type : ensEQcvariationTypeNULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an
** Ensembl Quality Check Variation object.
**
** @fdata [EnsPQcvariation]
**
** @nam3rule Set Set one member of a Quality Check Variation
** @nam4rule Adaptor Set the Ensembl Quality Check Variation Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Class Set the class
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Qcalignment Set the Ensembl Quality Check Alignment
** @nam4rule Query Set query members
** @nam5rule End Set the query end
** @nam5rule Sequence Set the query Ensembl Quality Check Sequence
** @nam5rule Start Set the query start
** @nam5rule String Set the query string
** @nam4rule State Set the state
** @nam4rule Target Set target members
** @nam5rule End Set the target end
** @nam5rule Sequence Set the target Ensembl Quality Check Sequence
** @nam5rule Start Set the target start
** @nam5rule String Set the target string
** @nam4rule Type Set the type
**
** @argrule * qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @argrule Adaptor qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Class class [EnsEQcvariationClass] Class
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Qcalignment qca [EnsPQcalignment] Ensembl Quality Check Alignment
** @argrule QueryEnd qend [ajuint] Query end
** @argrule QuerySequence qsequence [EnsPQcsequence]
** Query Ensembl Quality Check Sequence
** @argrule QueryStart qstart [ajuint] Query start
** @argrule QueryString qstring [AjPStr] Query string
** @argrule TargetEnd tend [ajuint] Target end
** @argrule TargetSequence tsequence [EnsPQcsequence]
** Target Ensembl Quality Check Sequence
** @argrule TargetStart tstart [ajuint] Target start
** @argrule TargetString tstring [AjPStr] Target string
** @argrule Type type [EnsEQcvariationType] Type
** @argrule State state [EnsEQcvariationState] State
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcvariationSetAdaptor *********************************************
**
** Set the Ensembl Database Adaptor member of an
** Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetAdaptor(EnsPQcvariation qcv,
                                EnsPQcvariationadaptor qcva)
{
    if (!qcv)
        return ajFalse;

    qcv->Adaptor = qcva;

    return ajTrue;
}




/* @func ensQcvariationSetAnalysis ********************************************
**
** Set the Ensembl Analysis member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetAnalysis(EnsPQcvariation qcv,
                                 EnsPAnalysis analysis)
{
    if (!qcv)
        return ajFalse;

    ensAnalysisDel(&qcv->Analysis);

    qcv->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcvariationSetClass ***********************************************
**
** Set the class member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] class [EnsEQcvariationClass] Class
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetClass(EnsPQcvariation qcv,
                              EnsEQcvariationClass class)
{
    if (!qcv)
        return ajFalse;

    qcv->Class = class;

    return ajTrue;
}




/* @func ensQcvariationSetIdentifier ******************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetIdentifier(EnsPQcvariation qcv,
                                   ajuint identifier)
{
    if (!qcv)
        return ajFalse;

    qcv->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcvariationSetQcalignment *****************************************
**
** Set the Ensembl Quality Check Alignment member of an
** Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] qca [EnsPQcalignment] Ensembl Quality Check Alignment
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetQcalignment(EnsPQcvariation qcv,
                                    EnsPQcalignment qca)
{
    if (!qcv)
        return ajFalse;

    ensQcalignmentDel(&qcv->Qcalignment);

    qcv->Qcalignment = ensQcalignmentNewRef(qca);

    return ajTrue;
}




/* @func ensQcvariationSetQueryEnd ********************************************
**
** Set the query end member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [r] qend [ajuint] Query end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetQueryEnd(EnsPQcvariation qcv,
                                 ajuint qend)
{
    if (!qcv)
        return ajFalse;

    qcv->QueryEnd = qend;

    return ajTrue;
}




/* @func ensQcvariationSetQuerySequence ***************************************
**
** Set the query Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] qsequence [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetQuerySequence(EnsPQcvariation qcv,
                                      EnsPQcsequence qsequence)
{
    if (!qcv)
        return ajFalse;

    ensQcsequenceDel(&qcv->QuerySequence);

    qcv->QuerySequence = ensQcsequenceNewRef(qsequence);

    return ajTrue;
}




/* @func ensQcvariationSetQueryStart ******************************************
**
** Set the query start member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [r] qstart [ajuint] Query start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetQueryStart(EnsPQcvariation qcv,
                                   ajuint qstart)
{
    if (!qcv)
        return ajFalse;

    qcv->QueryStart = qstart;

    return ajTrue;
}




/* @func ensQcvariationSetQueryString *****************************************
**
** Set the query string member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] qstring [AjPStr] Query string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetQueryString(EnsPQcvariation qcv,
                                    AjPStr qstring)
{
    if (!qcv)
        return ajFalse;

    ajStrDel(&qcv->QueryString);

    if (qcv->QueryString)
        qcv->QueryString = ajStrNewRef(qstring);

    return ajTrue;
}




/* @func ensQcvariationSetState ***********************************************
**
** Set the state member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] state [EnsEQcvariationState] State
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetState(EnsPQcvariation qcv,
                              EnsEQcvariationState state)
{
    if (!qcv)
        return ajFalse;

    qcv->State = state;

    return ajTrue;
}




/* @func ensQcvariationSetTargetEnd *******************************************
**
** Set the target end member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [r] tend [ajuint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetTargetEnd(EnsPQcvariation qcv,
                                  ajuint tend)
{
    if (!qcv)
        return ajFalse;

    qcv->TargetEnd = tend;

    return ajTrue;
}




/* @func ensQcvariationSetTargetSequence **************************************
**
** Set the target Ensembl Quality Check Sequence member of an
** Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] tsequence [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetTargetSequence(EnsPQcvariation qcv,
                                       EnsPQcsequence tsequence)
{
    if (!qcv)
        return ajFalse;

    ensQcsequenceDel(&qcv->TargetSequence);

    qcv->TargetSequence = ensQcsequenceNewRef(tsequence);

    return ajTrue;
}




/* @func ensQcvariationSetTargetStart *****************************************
**
** Set the target start member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [r] tstart [ajuint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetTargetStart(EnsPQcvariation qcv,
                                    ajuint tstart)
{
    if (!qcv)
        return ajFalse;

    qcv->TargetStart = tstart;

    return ajTrue;
}




/* @func ensQcvariationSetTargetString ****************************************
**
** Set the target string member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] tstring [AjPStr] Target string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetTargetString(EnsPQcvariation qcv,
                                     AjPStr tstring)
{
    if (!qcv)
        return ajFalse;

    ajStrDel(&qcv->TargetString);

    if (tstring)
        qcv->TargetString = ajStrNewRef(tstring);

    return ajTrue;
}




/* @func ensQcvariationSetType ************************************************
**
** Set the type member of an Ensembl Quality Check Variation.
**
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @param [u] type [EnsEQcvariationType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationSetType(EnsPQcvariation qcv,
                             EnsEQcvariationType type)
{
    if (!qcv)
        return ajFalse;

    qcv->Type = type;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Quality Check Variation object.
**
** @fdata [EnsPQcvariation]
**
** @nam3rule Trace Report Quality Check Variation members to debug file
**
** @argrule Trace qcv [const EnsPQcvariation] Ensembl Quality Check Variation
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcvariationTrace **************************************************
**
** Trace an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationTrace(const EnsPQcvariation qcv, ajuint level)
{
    AjPStr indent = NULL;

    if (!qcv)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensQcvariationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Qcalignment %p\n"
            "%S  Analysis %p\n"
            "%S  QuerySequence %p\n"
            "%S  QueryStart %u\n"
            "%S  QueryEnd %u\n"
            "%S  QueryString '%S'\n"
            "%S  TargetSequence %p\n"
            "%S  TargetStart %u\n"
            "%S  TargetEnd %u\n"
            "%S  TargetString '%S'\n"
            "%S  Class '%s'\n"
            "%S  Type '%s'\n"
            "%S  State '%s'\n",
            indent, qcv,
            indent, qcv->Use,
            indent, qcv->Identifier,
            indent, qcv->Adaptor,
            indent, qcv->Qcalignment,
            indent, qcv->Analysis,
            indent, qcv->QuerySequence,
            indent, qcv->QueryStart,
            indent, qcv->QueryEnd,
            indent, qcv->QueryString,
            indent, qcv->TargetSequence,
            indent, qcv->TargetStart,
            indent, qcv->TargetEnd,
            indent, qcv->TargetString,
            indent, ensQcvariationClassToChar(qcv->Class),
            indent, ensQcvariationTypeToChar(qcv->Type),
            indent, ensQcvariationStateToChar(qcv->State));

    ensQcalignmentTrace(qcv->Qcalignment, level + 1);

    ensAnalysisTrace(qcv->Analysis, level + 1);

    ensQcsequenceTrace(qcv->QuerySequence, level + 1);
    ensQcsequenceTrace(qcv->TargetSequence, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Quality Check Variation object.
**
** @fdata [EnsPQcvariation]
**
** @nam3rule Calculate Calculate Ensembl Quality Check Variation values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensQcvariationCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an Ensembl Quality Check Variation.
**
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensQcvariationCalculateMemsize(const EnsPQcvariation qcv)
{
    size_t size = 0;

    if (!qcv)
        return 0;

    size += sizeof (EnsOQcvariation);

    size += ensQcalignmentCalculateMemsize(qcv->Qcalignment);

    size += ensAnalysisCalculateMemsize(qcv->Analysis);

    size += ensQcsequenceCalculateMemsize(qcv->QuerySequence);
    size += ensQcsequenceCalculateMemsize(qcv->TargetSequence);

    if (qcv->QueryString)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcv->QueryString);
    }

    if (qcv->TargetString)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcv->TargetString);
    }

    return size;
}




/* @datasection [EnsEQcvariationClass] Ensembl Quality Check Variation Class
** enumeration
**
** @nam2rule Qcvariation Functions for manipulating
** Ensembl Quality Check Variation objects
** @nam3rule QcvariationClass Functions for manipulating
** Ensembl Quality Check Variation Class enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl Quality Check Variation Class enumeration.
**
** @fdata [EnsEQcvariationClass]
**
** @nam4rule From Ensembl Quality Check Variation Class query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  vclass  [const AjPStr] Class string
**
** @valrule * [EnsEQcvariationClass]
** Ensembl Quality Check Variation Class enumeration or
** ensEQcvariationClassNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcvariationClassFromStr *******************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check Variation Class enumeration.
**
** @param [r] vclass [const AjPStr] Class string
**
** @return [EnsEQcvariationClass]
** Ensembl Quality Check Variation Class enumeration or
** ensEQcvariationClassNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcvariationClass ensQcvariationClassFromStr(const AjPStr vclass)
{
    register EnsEQcvariationClass i = ensEQcvariationClassNULL;

    EnsEQcvariationClass eclass = ensEQcvariationClassNULL;

    for (i = ensEQcvariationClassNULL;
         qcvariationKClass[i];
         i++)
        if (ajStrMatchCaseC(vclass, qcvariationKClass[i]))
            eclass = i;

    if (!eclass)
        ajDebug("ensQcvariationClassFromStr encountered "
                "unexpected string '%S'.\n", vclass);

    return eclass;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check Variation Class enumeration.
**
** @fdata [EnsEQcvariationClass]
**
** @nam4rule To   Return Ensembl Quality Check Variation Class enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcvc [EnsEQcvariationClass]
** Ensembl Quality Check Variation Class enumeration
**
** @valrule Char [const char*] Class or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcvariationClassToChar ********************************************
**
** Convert an Ensembl Quality Check Variation Class enumeration into a
** C-type (char *) string.
**
** @param [u] qcvc [EnsEQcvariationClass]
** Ensembl Quality Check Variation Class enumeration
**
** @return [const char*]
** Ensembl Quality Check Variation Class C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcvariationClassToChar(EnsEQcvariationClass qcvc)
{
    register EnsEQcvariationClass i = ensEQcvariationClassNULL;

    for (i = ensEQcvariationClassNULL;
         qcvariationKClass[i] && (i < qcvc);
         i++);

    if (!qcvariationKClass[i])
        ajDebug("ensQcvariationClassToChar encountered an "
                "out of boundary error on "
                "Ensembl Quality Check Variation Class enumeration %d.\n",
                qcvc);

    return qcvariationKClass[i];
}




/* @datasection [EnsEQcvariationState] Ensembl Quality Check Variation State
** enumeration
**
** @nam2rule Qcvariation Functions for manipulating
** Ensembl Quality Check Variation objects
** @nam3rule QcvariationState Functions for manipulating
** Ensembl Quality Check Variation State enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl Quality Check Variation State enumeration.
**
** @fdata [EnsEQcvariationState]
**
** @nam4rule From Ensembl Quality Check Variation State query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  state  [const AjPStr] State string
**
** @valrule * [EnsEQcvariationState]
** Ensembl Quality Check Variation State enumeration or
** ensEQcvariationStateNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcvariationStateFromStr *******************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check Variation State enumeration.
**
** @param [r] state [const AjPStr] State string
**
** @return [EnsEQcvariationState]
** Ensembl Quality Check Variation State or ensEQcvariationStateNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcvariationState ensQcvariationStateFromStr(const AjPStr state)
{
    register EnsEQcvariationState i = ensEQcvariationStateNULL;

    EnsEQcvariationState estate = ensEQcvariationStateNULL;

    for (i = ensEQcvariationStateNULL;
         qcvariationKState[i];
         i++)
        if (ajStrMatchCaseC(state, qcvariationKState[i]))
            estate = i;

    if (!estate)
        ajDebug("ensQcvariationStateFromStr encountered "
                "unexpected string '%S'.\n", state);

    return estate;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check Variation State enumeration.
**
** @fdata [EnsEQcvariationState]
**
** @nam4rule To   Return Ensembl Quality Check Variation State enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcvs [EnsEQcvariationState]
** Ensembl Quality Check Variation State enumeration
**
** @valrule Char [const char*] State or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcvariationStateToChar ********************************************
**
** Convert an Ensembl Quality Check Variation State enumeration into a
** C-type (char *) string.
**
** @param [u] qcvs [EnsEQcvariationState] Ensembl Quality Check Variation State
**
** @return [const char*]
** Ensembl Quality Check Variation State C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcvariationStateToChar(EnsEQcvariationState qcvs)
{
    register EnsEQcvariationState i = ensEQcvariationStateNULL;

    for (i = ensEQcvariationStateNULL;
         qcvariationKState[i] && (i < qcvs);
         i++);

    if (!qcvariationKState[i])
        ajDebug("ensQcvariationStateToChar encountered an "
                "out of boundary error on "
                "Ensembl Quality Check Variation State enumeration %d.\n",
                qcvs);

    return qcvariationKState[i];
}




/* @datasection [EnsEQcvariationType] Ensembl Quality Check Variation Type
** enumeration
**
** @nam2rule Qcvariation Functions for manipulating
** Ensembl Quality Check Variation objects
** @nam3rule QcvariationType Functions for manipulating
** Ensembl Quality Check Variation Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Quality Check Variation Type enumeration.
**
** @fdata [EnsEQcvariationType]
**
** @nam4rule From Ensembl Quality Check Variation Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEQcvariationType]
** Ensembl Quality Check Variation Type enumeration or ensEQcvariationTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcvariationTypeFromStr ********************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check Variation Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEQcvariationType]
** Ensembl Quality Check Variation type or ensEQcvariationTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcvariationType ensQcvariationTypeFromStr(const AjPStr type)
{
    register EnsEQcvariationType i = ensEQcvariationTypeNULL;

    EnsEQcvariationType etype = ensEQcvariationTypeNULL;

    for (i = ensEQcvariationTypeNULL;
         qcvariationKType[i];
         i++)
        if (ajStrMatchCaseC(type, qcvariationKType[i]))
            etype = i;

    if (!etype)
        ajDebug("ensQcvariationTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check Variation Type enumeration.
**
** @fdata [EnsEQcvariationType]
**
** @nam4rule To   Return Ensembl Quality Check Variation Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcvt [EnsEQcvariationType]
** Ensembl Quality Check Variation Type enumeration
**
** @valrule Char [const char*] Type or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcvariationTypeToChar *********************************************
**
** Convert an Ensembl Quality Check Variation Type enumeration into a
** C-type (char *) string.
**
** @param [u] qcvt [EnsEQcvariationType]
** Ensembl Quality Check Variation Type enumeration
**
** @return [const char*]
** Ensembl Quality Check Variation type C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcvariationTypeToChar(EnsEQcvariationType qcvt)
{
    register EnsEQcvariationType i = ensEQcvariationTypeNULL;

    for (i = ensEQcvariationTypeNULL;
         qcvariationKType[i] && (i < qcvt);
         i++);

    if (!qcvariationKType[i])
        ajDebug("ensQcvariationTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Quality Check Variation Type enumeration %d.\n",
                qcvt);

    return qcvariationKType[i];
}




/* @datasection [EnsPQcvariationadaptor] Ensembl Quality Check Variation
** Adaptor
**
** @nam2rule Qcvariationadaptor Functions for manipulating
** Ensembl Quality Check Variation Adaptor objects
**
** @cc Bio::EnsEMBL::QC::DBSQL::Variationadaptor
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @funcstatic qcvariationadaptorFetchAllbyStatement **************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Quality Check Variation objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcvs [AjPList] AJAX List of Ensembl Quality Check Variations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool qcvariationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcvs)
{
    ajuint identifier  = 0U;
    ajuint analysisid  = 0U;
    ajuint alignmentid = 0U;

    ajuint qdbid  = 0U;
    ajuint qsid   = 0U;
    ajuint qstart = 0U;
    ajuint qend   = 0U;
    ajuint tdbid  = 0U;
    ajuint tsid   = 0U;
    ajuint tstart = 0U;
    ajuint tend   = 0U;

    EnsEQcvariationClass eclass = ensEQcvariationClassNULL;
    EnsEQcvariationType etype   = ensEQcvariationTypeNULL;
    EnsEQcvariationState estate = ensEQcvariationStateNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr qstring = NULL;
    AjPStr tstring = NULL;
    AjPStr class   = NULL;
    AjPStr type    = NULL;
    AjPStr state   = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPQcalignment qca         = NULL;
    EnsPQcalignmentadaptor qcaa = NULL;

    EnsPQcsequence qsequence   = NULL;
    EnsPQcsequence tsequence   = NULL;
    EnsPQcsequenceadaptor qcsa = NULL;

    EnsPQcvariation qcv         = NULL;
    EnsPQcvariationadaptor qcva = NULL;

    if (ajDebugTest("qcvariationadaptorFetchAllbyStatement"))
        ajDebug("qcvariationadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcvs %p\n",
                ba,
                statement,
                am,
                slice,
                qcvs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!qcvs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    aa   = ensRegistryGetAnalysisadaptor(dba);
    qcaa = ensRegistryGetQcalignmentadaptor(dba);
    qcva = ensRegistryGetQcvariationadaptor(dba);
    qcsa = ensRegistryGetQcsequenceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        analysisid  = 0;
        alignmentid = 0;
        qdbid       = 0;
        qsid        = 0;
        qstart      = 0;
        qend        = 0;
        qstring     = ajStrNew();
        tdbid       = 0;
        tsid        = 0;
        tstart      = 0;
        tend        = 0;
        tstring     = ajStrNew();
        class       = ajStrNew();
        type        = ajStrNew();
        state       = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToUint(sqlr, &alignmentid);
        ajSqlcolumnToUint(sqlr, &qdbid);
        ajSqlcolumnToUint(sqlr, &qsid);
        ajSqlcolumnToUint(sqlr, &qstart);
        ajSqlcolumnToUint(sqlr, &qend);
        ajSqlcolumnToStr(sqlr, &qstring);
        ajSqlcolumnToUint(sqlr, &tdbid);
        ajSqlcolumnToUint(sqlr, &tsid);
        ajSqlcolumnToUint(sqlr, &tstart);
        ajSqlcolumnToUint(sqlr, &tend);
        ajSqlcolumnToStr(sqlr, &tstring);
        ajSqlcolumnToStr(sqlr, &class);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToStr(sqlr, &state);

        ensQcalignmentadaptorFetchByIdentifier(qcaa, alignmentid, &qca);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        ensQcsequenceadaptorFetchByIdentifier(qcsa, qsid, &qsequence);
        ensQcsequenceadaptorFetchByIdentifier(qcsa, tsid, &tsequence);

        eclass = ensQcvariationClassFromStr(class);

        etype = ensQcvariationTypeFromStr(type);

        estate = ensQcvariationStateFromStr(state);

        qcv = ensQcvariationNewIni(qcva,
                                   identifier,
                                   qca,
                                   analysis,
                                   qsequence,
                                   qstart,
                                   qend,
                                   qstring,
                                   tsequence,
                                   tstart,
                                   tend,
                                   tstring,
                                   eclass,
                                   etype,
                                   estate);

        ajListPushAppend(qcvs, (void *) qcv);

        ensQcsequenceDel(&qsequence);
        ensQcsequenceDel(&tsequence);

        ensAnalysisDel(&analysis);

        ensQcalignmentDel(&qca);

        ajStrDel(&qstring);
        ajStrDel(&tstring);
        ajStrDel(&class);
        ajStrDel(&type);
        ajStrDel(&state);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Variation Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Quality Check Variation Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcvariationadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcvariationadaptorNew *********************************************
**
** Default constructor for an Ensembl Quality Check Variation Adaptor.
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
** @see ensRegistryGetQcvariationadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcvariationadaptor ensQcvariationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        qcvariationadaptorKTables,
        qcvariationadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &qcvariationadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check Variation Adaptor object.
**
** @fdata [EnsPQcvariationadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Variation Adaptor
**
** @argrule * Pqcva [EnsPQcvariationadaptor*]
** Ensembl Quality Check Variation Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcvariationadaptorDel *********************************************
**
** Default destructor for an Ensembl Quality Check Variation Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcva [EnsPQcvariationadaptor*]
** Ensembl Quality Check Variation Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcvariationadaptorDel(EnsPQcvariationadaptor *Pqcva)
{
#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensQcvariationadaptorDel"))
        ajDebug("ensQcvariationadaptorDel\n"
                "  *Pqcva %p\n",
                *Pqcva);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    ensBaseadaptorDel(Pqcva);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check Variation Adaptor object.
**
** @fdata [EnsPQcvariationadaptor]
**
** @nam3rule Get Return Ensembl Quality Check Variation Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcvariationadaptorGetBaseadaptor **********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Quality Check Variation Adaptor.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensQcvariationadaptorGetBaseadaptor(
    EnsPQcvariationadaptor qcva)
{
    return qcva;
}




/* @func ensQcvariationadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Quality Check Variation Adaptor.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensQcvariationadaptorGetDatabaseadaptor(
    EnsPQcvariationadaptor qcva)
{
    return ensBaseadaptorGetDatabaseadaptor(qcva);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Quality Check Variation objects from an
** Ensembl SQL database.
**
** @fdata [EnsPQcvariationadaptor]
**
** @nam3rule Fetch Fetch Ensembl Quality Check Variation object(s)
** @nam4rule All   Fetch all Ensembl Quality Check Variation objects
** @nam4rule Allby Fetch all Ensembl Quality Check Variation objects matching a
**                 criterion
** @nam5rule Qcalignment Fetch all by an Ensembl Quality Check Alignment
** @nam5rule Qcdatabase  Fetch all by an Ensembl Quality Check Database
** @nam6rule Pair        Fetch all by an Ensembl Quality Check Database pair
** @nam6rule Query       Fetch all by an Ensembl Quality Check Database
**                       as query
** @nam6rule Target      Fetch all by an Ensembl Quality Check Database
**                       as target
** @nam4rule By Fetch one Ensembl Quality Check Variation object matching a
** criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
**
** @argrule * qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @argrule AllbyQcalignment qca [const EnsPQcalignment]
** Ensembl Quality Check Alignment
** @argrule AllbyQcalignment qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation
** objects
** @argrule AllbyQcdatabasePair analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcdatabasePair qdb [const EnsPQcdatabase]
** Query Ensembl Quality Check Database
** @argrule AllbyQcdatabasePair tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyQcdatabasePair qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
** @argrule AllbyQcdatabaseQuery analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule AllbyQcdatabaseQuery qdb [const EnsPQcdatabase]
** Query Ensembl Quality Check Database
** @argrule AllbyQcdatabaseQuery qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
** @argrule AllbyQcdatabaseTarget analysis [const EnsPAnalysis]
** Ensembl Analysis
** @argrule AllbyQcdatabaseTarget tdb [const EnsPQcdatabase]
** Target Ensembl Quality Check Database
** @argrule AllbyQcdatabaseTarget qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal
** Ensembl Quality Check Variation identifier
** @argrule ByIdentifier Pqcv [EnsPQcvariation*]
** Ensembl Quality Check Variation address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcvariationadaptorFetchAllbyQcalignment ***************************
**
** Fetch all Ensembl Quality Check Variation objects by an
** Ensembl Quality Check Alignment.
**
** The caller is responsible for deleting the Ensembl Quality Check Variation
** objects before deleting the AJAX List.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [r] qca [const EnsPQcalignment] Ensembl Quality Check Alignment
** @param [u] qcvs [AjPList] AJAX List of Ensembl Quality Check Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorFetchAllbyQcalignment(
    EnsPQcvariationadaptor qcva,
    const EnsPQcalignment qca,
    AjPList qcvs)
{
    AjPStr constraint = NULL;

    if (!qcva)
        return ajFalse;

    if (!qca)
        return ajFalse;

    if (!qcvs)
        return ajFalse;

    constraint = ajFmtStr("variation.alignment_id = %u",
                          ensQcalignmentGetIdentifier(qca));

    ensBaseadaptorFetchAllbyConstraint(qcva,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcvs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcvariationadaptorFetchAllbyQcdatabasePair ************************
**
** Fetch all Ensembl Quality Check Variation objects by an Ensembl Analysis,
** as well as Ensembl Quality Check Database objects representing query and
** target.
**
** The caller is responsible for deleting the Ensembl Quality Check Variation
** objects before deleting the AJAX List.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl Quality Check Database
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl Quality Check Database
** @param [u] qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorFetchAllbyQcdatabasePair(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcvs)
{
    AjPStr constraint = NULL;

    if (!qcva)
        return ajFalse;

    if (!analysis)
        return ajFalse;

    if (!qdb)
        return ajFalse;

    if (!tdb)
        return ajFalse;

    if (!qcvs)
        return ajFalse;

    constraint = ajFmtStr("variation.analysis_id = %u "
                          "AND "
                          "variation.query_db_id = %u "
                          "AND "
                          "variation.target_db_id = %u",
                          ensAnalysisGetIdentifier(analysis),
                          ensQcdatabaseGetIdentifier(qdb),
                          ensQcdatabaseGetIdentifier(tdb));

    ensBaseadaptorFetchAllbyConstraint(qcva,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcvs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcvariationadaptorFetchAllbyQcdatabaseQuery ***********************
**
** Fetch all Ensembl Quality Check Variation objects by an
** Ensembl Quality Check Database representing the query.
**
** The caller is responsible for deleting the Ensembl Quality Check Variation
** objects before deleting the AJAX List.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] qdb [const EnsPQcdatabase] Query Ensembl Quality Check Database
** @param [u] qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorFetchAllbyQcdatabaseQuery(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcvs)
{
    AjPStr constraint = NULL;

    if (!qcva)
        return ajFalse;

    if (!qdb)
        return ajFalse;

    if (!qcvs)
        return ajFalse;

    constraint = ajFmtStr("variation.query_db_id = %u",
                          ensQcdatabaseGetIdentifier(qdb));

    if (analysis)
        ajFmtPrintAppS(&constraint,
                       " AND variation.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorFetchAllbyConstraint(qcva,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcvs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcvariationadaptorFetchAllbyQcdatabaseTarget **********************
**
** Fetch all Ensembl Quality Check Variation objects by an
** Ensembl Quality Check Database representing the target.
**
** The caller is responsible for deleting the Ensembl Quality Check Variation
** objects before deleting the AJAX List.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [rN] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] tdb [const EnsPQcdatabase] Target Ensembl Quality Check Database
** @param [u] qcvs [AjPList] AJAX List of
** Ensembl Quality Check Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorFetchAllbyQcdatabaseTarget(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcvs)
{
    AjPStr constraint = NULL;

    if (!qcva)
        return ajFalse;

    if (!tdb)
        return ajFalse;

    if (!qcvs)
        return ajFalse;

    constraint = ajFmtStr("variation.target_db_id = %u",
                          ensQcdatabaseGetIdentifier(tdb));

    if (analysis)
        ajFmtPrintAppS(&constraint,
                       " AND variation.analysis_id = %u",
                       ensAnalysisGetIdentifier(analysis));

    ensBaseadaptorFetchAllbyConstraint(qcva,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcvs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcvariationadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Quality Check Variation via its
** SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Quality Check Variation.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal
** Ensembl Quality Check Variation identifier
** @param [wP] Pqcv [EnsPQcvariation*] Ensembl Quality Check Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorFetchByIdentifier(
    EnsPQcvariationadaptor qcva,
    ajuint identifier,
    EnsPQcvariation *Pqcv)
{
    if (!qcva)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pqcv)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(qcva, identifier, (void **) Pqcv);
}




/* @section object access *****************************************************
**
** Functions for accessing Ensembl Quality Check Variation objects in an
** Ensembl SQL database.
**
** @fdata [EnsPQcvariationadaptor]
**
** @nam3rule Delete Delete Ensembl Quality Check Variation object(s)
** @nam3rule Store  Store Ensembl Quality Check Variation object(s)
** @nam3rule Update Update Ensembl Quality Check Variation object(s)
**
** @argrule * qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @argrule Delete qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @argrule Store qcv [EnsPQcvariation] Ensembl Quality Check Variation
** @argrule Update qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcvariationadaptorDelete ******************************************
**
** Delete an Ensembl Quality Check Variation.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorDelete(EnsPQcvariationadaptor qcva,
                                   EnsPQcvariation qcv)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcva)
        return ajFalse;

    if (!qcv)
        return ajFalse;

    if (!ensQcvariationGetIdentifier(qcv))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcva);

    statement = ajFmtStr(
        "DELETE FROM "
        "variation "
        "WHERE "
        "variation.variation_id = %u",
        qcv->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        qcv->Adaptor    = (EnsPQcvariationadaptor) NULL;
        qcv->Identifier = 0;

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcvariationadaptorStore *******************************************
**
** Store an Ensembl Quality Check Variation.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [u] qcv [EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorStore(EnsPQcvariationadaptor qcva,
                                  EnsPQcvariation qcv)
{
    char *txtqstr = NULL;
    char *txttstr = NULL;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcva)
        return ajFalse;

    if (!qcv)
        return ajFalse;

    if (ensQcvariationGetAdaptor(qcv) && ensQcvariationGetIdentifier(qcv))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcva);

    ensDatabaseadaptorEscapeC(dba, &txtqstr, qcv->QueryString);

    ensDatabaseadaptorEscapeC(dba, &txttstr, qcv->TargetString);

    statement = ajFmtStr(
        "INSERT IGNORE INTO "
        "variation "
        "SET "
        "variation.analysis_id = %u, "
        "variation.alignment_id = %u, "
        "variation.query_db_id = %u, "
        "variation.query_id = %u, "
        "variation.query_start = %u, "
        "variation.query_end = %u, "
        "variation.query_seq = '%s', "
        "variation.target_db_id = %u, "
        "variation.target_id = %u, "
        "variation.target_start = %u, "
        "variation.target_end = %u, "
        "variation.target_seq = '%s', "
        "variation.class = '%s', "
        "variation.type = '%s', "
        "variation.state = '%s'",
        ensAnalysisGetIdentifier(qcv->Analysis),
        ensQcalignmentGetIdentity(qcv->Qcalignment),
        ensQcsequenceGetQcdatabaseIdentifier(qcv->QuerySequence),
        ensQcsequenceGetIdentifier(qcv->QuerySequence),
        qcv->QueryStart,
        qcv->QueryEnd,
        txtqstr,
        ensQcsequenceGetQcdatabaseIdentifier(qcv->TargetSequence),
        ensQcsequenceGetIdentifier(qcv->TargetSequence),
        qcv->TargetStart,
        qcv->TargetEnd,
        txttstr,
        ensQcvariationClassToChar(qcv->Class),
        ensQcvariationTypeToChar(qcv->Type),
        ensQcvariationStateToChar(qcv->State));

    ajCharDel(&txtqstr);
    ajCharDel(&txttstr);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcvariationSetIdentifier(qcv, ajSqlstatementGetIdentifier(sqls));

        ensQcvariationSetAdaptor(qcv, qcva);

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcvariationadaptorUpdate ******************************************
**
** Update an Ensembl Quality Check Variation.
**
** @param [u] qcva [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @param [r] qcv [const EnsPQcvariation] Ensembl Quality Check Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcvariationadaptorUpdate(EnsPQcvariationadaptor qcva,
                                   const EnsPQcvariation qcv)
{
    char *txtqstr = NULL;
    char *txttstr = NULL;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcva)
        return ajFalse;

    if (!qcv)
        return ajFalse;

    if (!ensQcvariationGetIdentifier(qcv))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcva);

    ensDatabaseadaptorEscapeC(dba, &txtqstr, qcv->QueryString);

    ensDatabaseadaptorEscapeC(dba, &txttstr, qcv->TargetString);

    statement = ajFmtStr(
        "UPDATE IGNORE "
        "variation "
        "SET "
        "variation.analysis_id = %u, "
        "variation.alignment_id = %u, "
        "variation.query_db_id = %u, "
        "variation.query_id = %u, "
        "variation.query_start = %u, "
        "variation.query_end = %u, "
        "variation.query_seq = '%s', "
        "variation.target_db_id = %u, "
        "variation.target_id = %u, "
        "variation.target_start = %u, "
        "variation.target_end = %u, "
        "variation.target_seq = '%s', "
        "variation.class = '%s', "
        "variation.type = '%s', "
        "variation.state = '%s' "
        "WHERE "
        "variation.variation_id = %u",
        ensAnalysisGetIdentifier(qcv->Analysis),
        ensQcalignmentGetIdentity(qcv->Qcalignment),
        ensQcsequenceGetQcdatabaseIdentifier(qcv->QuerySequence),
        ensQcsequenceGetIdentifier(qcv->QuerySequence),
        qcv->QueryStart,
        qcv->QueryEnd,
        txtqstr,
        ensQcsequenceGetQcdatabaseIdentifier(qcv->TargetSequence),
        ensQcsequenceGetIdentifier(qcv->TargetSequence),
        qcv->TargetStart,
        qcv->TargetEnd,
        txttstr,
        ensQcvariationClassToChar(qcv->Class),
        ensQcvariationTypeToChar(qcv->Type),
        ensQcvariationStateToChar(qcv->State),
        qcv->Identifier);

    ajCharDel(&txtqstr);
    ajCharDel(&txttstr);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
        result = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}

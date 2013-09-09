/* @source ensrepeat **********************************************************
**
** Ensembl Repeat functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.59 $
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

#include "ensrepeat.h"
#include "enstable.h"




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

/* @conststatic repeatconsensusadaptorKTablenames *****************************
**
** Array of Ensembl Repeat Consensus Adaptor SQL table names
**
******************************************************************************/

static const char *repeatconsensusadaptorKTablenames[] =
{
    "repeat_consensus",
    (const char *) NULL
};




/* @conststatic repeatconsensusadaptorKColumnnames ****************************
**
** Array of Ensembl Repeat Consensus Adaptor SQL column names
**
******************************************************************************/

static const char *repeatconsensusadaptorKColumnnames[] =
{
    "repeat_consensus.repeat_consensus_id",
    "repeat_consensus.repeat_name",
    "repeat_consensus.repeat_class",
    "repeat_consensus.repeat_type",
    "repeat_consensus.repeat_consensus",
    (const char *) NULL
};




/* @conststatic repeatfeatureadaptorKTablenames *******************************
**
** Array of Ensembl Repeat Feature Adaptor SQL table names
**
******************************************************************************/

static const char *repeatfeatureadaptorKTablenames[] =
{
    "repeat_feature",
    "repeat_consensus",
    (const char *) NULL
};




/* @conststatic repeatfeatureadaptorKColumnnames ******************************
**
** Array of Ensembl Repeat Feature Adaptor SQL column names
**
******************************************************************************/

static const char *repeatfeatureadaptorKColumnnames[] =
{
    "repeat_feature.repeat_feature_id",
    "repeat_feature.seq_region_id",
    "repeat_feature.seq_region_start",
    "repeat_feature.seq_region_end",
    "repeat_feature.seq_region_strand",
    "repeat_feature.repeat_consensus_id",
    "repeat_feature.repeat_start",
    "repeat_feature.repeat_end",
    "repeat_feature.analysis_id",
    "repeat_feature.score",
    (const char *) NULL
};




/* @conststatic repeatfeatureadaptorKDefaultcondition *************************
**
** Ensembl Repeat Feature Adaptor SQL SELECT default condition
**
******************************************************************************/

static const char *repeatfeatureadaptorKDefaultcondition =
    "repeat_feature.repeat_consensus_id = "
    "repeat_consensus.repeat_consensus_id";




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool repeatconsensusadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rci);

static int listRepeatfeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listRepeatfeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listRepeatfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listRepeatfeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listRepeatfeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static void repeatfeatureadaptorLinkRepeatconsensus(void **Pkey,
                                                    void **Pvalue,
                                                    void *cl);

static AjBool repeatfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensrepeat *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPRepeatconsensus] Ensembl Repeat Consensus ****************
**
** @nam2rule Repeatconsensus Functions for manipulating
** Ensembl Repeat Consensus objects
**
** @cc Bio::EnsEMBL::RepeatConsensus
** @cc CVS Revision: 1.14
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Repeat Consensus by pointer.
** It is the responsibility of the user to first destroy any previous
** Repeat Consensus. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
** @argrule Ini rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus
** Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Repeat Consensus name
** @argrule Ini class [AjPStr] Repeat Consensus class
** @argrule Ini type [AjPStr] Repeat Consensus type
** @argrule Ini consensus [AjPStr] Repeat Consensus sequence
** @argrule Ini length [ajuint] Repeat Consensus sequence length
** @argrule Ref rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @valrule * [EnsPRepeatconsensus] Ensembl Repeat Consensus or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatconsensusNewCpy *********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNewCpy(const EnsPRepeatconsensus rc)
{
    EnsPRepeatconsensus pthis = NULL;

    if (!rc)
        return NULL;

    AJNEW0(pthis);

    pthis->Adaptor = rc->Adaptor;

    pthis->Identifier = rc->Identifier;

    if (rc->Name)
        pthis->Name = ajStrNewRef(rc->Name);

    if (rc->Class)
        pthis->Class = ajStrNewRef(rc->Class);

    if (rc->Type)
        pthis->Type = ajStrNewRef(rc->Type);

    if (rc->Consensus)
        pthis->Consensus = ajStrNewRef(rc->Consensus);

    pthis->Length = rc->Length;

    pthis->Use = 1U;

    return pthis;
}




/* @func ensRepeatconsensusNewIni *********************************************
**
** Constructor for an Ensembl Repeat Consensus with initial values.
**
** @cc Bio::EnsEMBL::RepeatConsensus::new
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] name [AjPStr] Repeat Consensus name
** @param [u] class [AjPStr] Repeat Consensus class
** @param [u] type [AjPStr] Repeat Consensus type
** @param [u] consensus [AjPStr] Repeat Consensus sequence
** @param [r] length [ajuint] Repeat Consensus sequence length
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNewIni(EnsPRepeatconsensusadaptor rca,
                                             ajuint identifier,
                                             AjPStr name,
                                             AjPStr class,
                                             AjPStr type,
                                             AjPStr consensus,
                                             ajuint length)
{
    EnsPRepeatconsensus rc = NULL;

    AJNEW0(rc);

    rc->Adaptor    = rca;
    rc->Identifier = identifier;

    if (name)
        rc->Name = ajStrNewRef(name);

    if (class)
        rc->Class = ajStrNewRef(class);

    if (type)
        rc->Type = ajStrNewRef(type);

    if (consensus)
        rc->Consensus = ajStrNewRef(consensus);

    rc->Length = length;

    rc->Use = 1U;

    return rc;
}




/* @func ensRepeatconsensusNewRef *********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNewRef(EnsPRepeatconsensus rc)
{
    if (!rc)
        return NULL;

    rc->Use++;

    return rc;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Consensus
**
** @argrule * Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatconsensusDel ************************************************
**
** Default destructor for an Ensembl Repeat Consensus.
**
** @param [d] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensRepeatconsensusDel(EnsPRepeatconsensus *Prc)
{
    EnsPRepeatconsensus pthis = NULL;

    if (!Prc)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensRepeatconsensusDel"))
    {
        ajDebug("ensRepeatconsensusDel\n"
                "  *Prc %p\n",
                *Prc);

        ensRepeatconsensusTrace(*Prc, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Prc) || --pthis->Use)
    {
        *Prc = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Class);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Consensus);

    ajMemFree((void **) Prc);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Get Return Repeat Consensus attribute(s)
** @nam4rule Adaptor Return the Ensembl Repeat Consensus Adaptor
** @nam4rule Class Return the class
** @nam4rule Consensus Return the consensus sequence
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Length Return the length
** @nam4rule Name Return the name
** @nam4rule Type Return the type
**
** @argrule * rc [const EnsPRepeatconsensus] Repeat Consensus
**
** @valrule Adaptor [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor or NULL
** @valrule Class [AjPStr] Class or NULL
** @valrule Consensus [AjPStr] Consensus sequence or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Length [ajuint] Length or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Type [AjPStr] Type or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusGetAdaptor *****************************************
**
** Get the Ensembl Repeat Consensus Adaptor member of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatconsensusadaptor ensRepeatconsensusGetAdaptor(
    const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Adaptor : NULL;
}




/* @func ensRepeatconsensusGetClass *******************************************
**
** Get the class member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_class
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Class or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetClass(const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Class : NULL;
}




/* @func ensRepeatconsensusGetConsensus ***************************************
**
** Get the consensus sequence member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_consensus
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Consensus sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetConsensus(const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Consensus : NULL;
}




/* @func ensRepeatconsensusGetIdentifier **************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensRepeatconsensusGetIdentifier(const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Identifier : 0U;
}




/* @func ensRepeatconsensusGetLength ******************************************
**
** Get the length member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::length
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [ajuint] Length or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensRepeatconsensusGetLength(const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Length : 0U;
}




/* @func ensRepeatconsensusGetName ********************************************
**
** Get the name member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::name
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetName(const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Name : NULL;
}




/* @func ensRepeatconsensusGetType ********************************************
**
** Get the type member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_type
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Type or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetType(const EnsPRepeatconsensus rc)
{
    return (rc) ? rc->Type : NULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Set Set one member of a Repeat Consensus
** @nam4rule Adaptor Set the Ensembl Repeat Consensus Adaptor
** @nam4rule Class Set the class
** @nam4rule Consensus Set the consensus sequence
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Length Set the length
** @nam4rule Name Set the name
** @nam4rule Type Set the type
**
** @argrule * rc [EnsPRepeatconsensus] Ensembl Repeat Consensus object
** @argrule Adaptor rca [EnsPRepeatconsensusadaptor] Ensembl Repeat
**                                                   Consensus Adaptor
** @argrule Class class [AjPStr] Class
** @argrule Consensus consensus [AjPStr] Consensus sequence
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Length length [ajuint] Length
** @argrule Name name [AjPStr] Name
** @argrule Type type [AjPStr] Type
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensRepeatconsensusSetAdaptor *****************************************
**
** Set the Ensembl Repeat Consensus Adaptor member of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetAdaptor(EnsPRepeatconsensus rc,
                                    EnsPRepeatconsensusadaptor rca)
{
    if (!rc)
        return ajFalse;

    rc->Adaptor = rca;

    return ajTrue;
}




/* @func ensRepeatconsensusSetClass *******************************************
**
** Set the class member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_class
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] class [AjPStr] Class
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetClass(EnsPRepeatconsensus rc,
                                  AjPStr class)
{
    if (!rc)
        return ajFalse;

    ajStrDel(&rc->Class);

    if (class)
        rc->Class = ajStrNewRef(class);

    return ajTrue;
}




/* @func ensRepeatconsensusSetConsensus ***************************************
**
** Set the consensus sequence member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_consensus
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] consensus [AjPStr] Consensus sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetConsensus(EnsPRepeatconsensus rc,
                                      AjPStr consensus)
{
    if (!rc)
        return ajFalse;

    ajStrDel(&rc->Consensus);

    if (consensus)
        rc->Consensus = ajStrNewRef(consensus);

    return ajTrue;
}




/* @func ensRepeatconsensusSetIdentifier **************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetIdentifier(EnsPRepeatconsensus rc,
                                       ajuint identifier)
{
    if (!rc)
        return ajFalse;

    rc->Identifier = identifier;

    return ajTrue;
}




/* @func ensRepeatconsensusSetLength ******************************************
**
** Set the length member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::length
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] length [ajuint] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetLength(EnsPRepeatconsensus rc,
                                   ajuint length)
{
    if (!rc)
        return ajFalse;

    rc->Length = length;

    return ajTrue;
}




/* @func ensRepeatconsensusSetName ********************************************
**
** Set the name member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::name
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetName(EnsPRepeatconsensus rc,
                                 AjPStr name)
{
    if (!rc)
        return ajFalse;

    ajStrDel(&rc->Name);

    if (name)
        rc->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensRepeatconsensusSetType ********************************************
**
** Set the type member of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_type
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] type [AjPStr] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetType(EnsPRepeatconsensus rc,
                                 AjPStr type)
{
    if (!rc)
        return ajFalse;

    ajStrDel(&rc->Type);

    if (type)
        rc->Type = ajStrNewRef(type);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Trace Report Ensembl Repeat Consensus members to debug file
**
** @argrule Trace rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensRepeatconsensusTrace **********************************************
**
** Trace an Ensembl Repeat Consensus.
**
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusTrace(const EnsPRepeatconsensus rc, ajuint level)
{
    AjPStr indent = NULL;

    if (!rc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensRepeatconsensusTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Identifier %u\n"
            "%S  Name '%S'\n"
            "%S  Class '%S'\n"
            "%S  Type '%S'\n"
            "%S  Consensus '%S'\n"
            "%S  Length %u\n"
            "%S  Use %u\n",
            indent, rc,
            indent, rc->Adaptor,
            indent, rc->Identifier,
            indent, rc->Name,
            indent, rc->Class,
            indent, rc->Type,
            indent, rc->Consensus,
            indent, rc->Length,
            indent, rc->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Calculate Calculate Ensembl Repeat Consensus information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensRepeatconsensusCalculateMemsize ***********************************
**
** Calculate the memory size in bytes of an Ensembl Repeat Consensus.
**
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensRepeatconsensusCalculateMemsize(const EnsPRepeatconsensus rc)
{
    size_t size = 0;

    if (!rc)
        return 0;

    size += sizeof (EnsORepeatconsensus);

    if (rc->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Name);
    }

    if (rc->Class)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Class);
    }

    if (rc->Type)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Type);
    }

    if (rc->Consensus)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Consensus);
    }

    return size;
}




/* @datasection [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @nam2rule Repeatconsensusadaptor Functions for manipulating
** Ensembl Repeat Consensus Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor
** @cc CVS Revision: 1.25
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic repeatconsensusadaptorFetchAllbyStatement **********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Repeat Consensus objects.
** The caller is responsible for deleting the Ensembl Repeat Consensi before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::_generic_fetch
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] rci [AjPList] AJAX List of Ensembl Repeat Consensi
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool repeatconsensusadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rci)
{
    const char *Ptr = NULL;

    register ajuint i = 0U;

    ajuint identifier = 0U;
    ajuint length     = 0U;

    AjPRegexp rp = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr class     = NULL;
    AjPStr type      = NULL;
    AjPStr consensus = NULL;
    AjPStr number    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPRepeatconsensus rc         = NULL;
    EnsPRepeatconsensusadaptor rca = NULL;

    if (ajDebugTest("repeatconsensusadaptorFetchAllbyStatement"))
        ajDebug("repeatconsensusadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  rci %p\n",
                ba,
                statement,
                am,
                slice,
                rci);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!rci)
        return ajFalse;

    rp = ajRegCompC("^(\\d+)\\(N\\)$");

    number = ajStrNew();

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    rca = ensRegistryGetRepeatconsensusadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0U;
        name       = ajStrNew();
        class      = ajStrNew();
        type       = ajStrNew();
        consensus  = ajStrNew();
        length     = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &class);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToStr(sqlr, &consensus);

        /*
        ** Check the Ensembl Repeat consus whether it obeys the ^\\d+\\(N\\)$
        ** format or alternatively, use the string length, which should
        ** conveniently fit into an unsigned integer.
        */

        if (ajRegExec(rp, consensus))
        {
            ajRegSubI(rp, 1, &number);

            if (!ajStrToUint(number, &length))
                ajWarn("Could not convert repeat consensus '%S', which obeys "
                       "the \\d+\\(N\\) format, into an unsigned integer.\n",
                       consensus);
        }
        else
        {
            /*
            ** NOTE: Since ajStrGetLen returns size_t, which exceeds ajuint,
            ** the length of the consensus sequence needs to be determined
            ** here.
            **
            ** length = (ajuint) ajStrGetLen(consensus);
            */

            for (i = 0U, Ptr = ajStrGetPtr(consensus);
                 (Ptr && *Ptr);
                 i++, Ptr++)
                if (i == UINT_MAX)
                    ajFatal("repeatconsensusadaptorFetchAllbyStatement got a "
                            "consensus, which length exeeded UINT_MAX.");

            length = i;
        }

        rc = ensRepeatconsensusNewIni(rca,
                                      identifier,
                                      name,
                                      class,
                                      type,
                                      consensus,
                                      length);

        ajListPushAppend(rci, (void *) rc);

        ajStrDel(&name);
        ajStrDel(&class);
        ajStrDel(&type);
        ajStrDel(&consensus);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajRegFree(&rp);

    ajStrDel(&number);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Repeat Consensus Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Repeat Consensus Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatconsensusadaptorNew *****************************************
**
** Default constructor for an Ensembl Repeat Consensus Adaptor.
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
** @see ensRegistryGetRepeatconsensusadaptor
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPRepeatconsensusadaptor]
** Ensembl Repat Consensus Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatconsensusadaptor ensRepeatconsensusadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensBaseadaptorNew(
        dba,
        repeatconsensusadaptorKTablenames,
        repeatconsensusadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &repeatconsensusadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Repeat Consensus Adaptor object.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Consensus Adaptor
**
** @argrule * Prca [EnsPRepeatconsensusadaptor*]
** Ensembl Repeat Consensus Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatconsensusadaptorDel *****************************************
**
** Default destructor for an Ensembl Repeat Consensus Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Prca [EnsPRepeatconsensusadaptor*]
** Ensembl Repeat Consensus Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensRepeatconsensusadaptorDel(EnsPRepeatconsensusadaptor *Prca)
{
    ensBaseadaptorDel(Prca);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Repeat Consensus Adaptor
** object.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule Get Return Repeat Consensus Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusadaptorGetBaseadaptor ******************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Repeat Consensus Adaptor.
**
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensRepeatconsensusadaptorGetBaseadaptor(
    EnsPRepeatconsensusadaptor rca)
{
    return rca;
}




/* @func ensRepeatconsensusadaptorGetDatabaseadaptor **************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Repeat Consensus Adaptor.
**
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensRepeatconsensusadaptorGetDatabaseadaptor(
    EnsPRepeatconsensusadaptor rca)
{
    return ensBaseadaptorGetDatabaseadaptor(
        ensRepeatconsensusadaptorGetBaseadaptor(rca));
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Repeat Consensus objects from an
** Ensembl SQL database.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule Fetch Fetch Ensembl Repeat Consensus object(s)
** @nam4rule All Fetch all Ensembl Repeat Consensus objects
** @nam4rule Allby Fetch all Ensembl Repeat Consensus objects
**                 matching a criterion
** @nam5rule Classconsensus Fetch all by class and consensus sequence
** @nam5rule Identifiers Fetch all by an AJAX Table
** @nam4rule By Fetch one Ensembl Repeat Consensus object
**              matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
** @nam5rule Name Fetch by name and class
**
** @argrule * rca [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor
** @argrule AllbyClassconsensus class [const AjPStr]
** Class
** @argrule AllbyClassconsensus consensus [const AjPStr]
** Consensus sequence
** @argrule AllbyClassconsensus rci [AjPList]
** AJAX List of Ensembl Repeat Consensus objects
** @argrule AllbyIdentifiers rcit [AjPTable]
** AJAX Table of Ensembl Repeat Consensus objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Prc [EnsPRepeatconsensus*]
** Ensembl Repeat Consensus address
** @argrule ByName name [const AjPStr]
** Name
** @argrule ByName class [const AjPStr]
** Class
** @argrule ByName Prc [EnsPRepeatconsensus*]
** Ensembl Repeat Consensus address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusadaptorFetchAllbyClassconsensus ********************
**
** Fetch all Ensembl Repeat Consensus objects by a class and
** consensus sequence.
**
** The caller is responsible for deleting the Ensembl Repeat Consensus objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_all_by_class_seq
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] class [const AjPStr] Class
** @param [r] consensus [const AjPStr] Consensus sequence
** @param [u] rci [AjPList] AJAX List of Ensembl Repeat Consensi
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchAllbyClassconsensus(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr class,
    const AjPStr consensus,
    AjPList rci)
{
    char *txtclass     = NULL;
    char *txtconsensus = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!rca)
        return ajFalse;

    if (!class)
        return ajFalse;

    if (!consensus)
        return ajFalse;

    if (!rci)
        return ajFalse;

    ba = ensRepeatconsensusadaptorGetBaseadaptor(rca);

    ensBaseadaptorEscapeC(ba, &txtclass, class);
    ensBaseadaptorEscapeC(ba, &txtconsensus, consensus);

    constraint = ajFmtStr(
        "repeat_consensus.repeat_class = '%s' "
        "AND "
        "repeat_consensus.repeat_consensus = '%s'",
        txtclass,
        txtconsensus);

    ajCharDel(&txtclass);
    ajCharDel(&txtconsensus);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        rci);

    ajStrDel(&constraint);

    return result;
}




/* @func ensRepeatconsensusadaptorFetchAllbyIdentifiers ***********************
**
** Fetch all Ensembl Repeat Consensus objects by an AJAX Table of
** AJAX unsigned integer key data and assign them as value data.
**
** The caller is responsible for deleting the AJAX unsigned integer key and
** Ensembl Repeat Consensus value data before deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] rca [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor
** @param [u] rcit [AjPTable]
** AJAX Table of
** AJAX unsigned integer (identifier) key data and
** Ensembl Repeat Consensus value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchAllbyIdentifiers(
    EnsPRepeatconsensusadaptor rca,
    AjPTable rcit)
{
    return ensBaseadaptorFetchAllbyIdentifiers(
        ensRepeatconsensusadaptorGetBaseadaptor(rca),
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensRepeatconsensusGetIdentifier,
        rcit);
}




/* @func ensRepeatconsensusadaptorFetchByIdentifier ***************************
**
** Fetch an Ensembl Repeat Consensus by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_by_dbID
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByIdentifier(
    EnsPRepeatconsensusadaptor rca,
    ajuint identifier,
    EnsPRepeatconsensus *Prc)
{
    return ensBaseadaptorFetchByIdentifier(
        ensRepeatconsensusadaptorGetBaseadaptor(rca),
        identifier,
        (void **) Prc);
}




/* @func ensRepeatconsensusadaptorFetchByName *********************************
**
** Fetch an Ensembl Repeat Consensus by its name and optionally class.
**
** The caller is responsible for deleting the Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_by_name
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_by_name_class
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] class [const AjPStr] Class
** @param [u] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByName(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr name,
    const AjPStr class,
    EnsPRepeatconsensus *Prc)
{
    char *txtname  = NULL;
    char *txtclass = NULL;

    AjBool result = AJFALSE;

    AjPList rci = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPRepeatconsensus rc = NULL;

    if (!rca)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!Prc)
        return ajFalse;

    ba = ensRepeatconsensusadaptorGetBaseadaptor(rca);

    ensBaseadaptorEscapeC(ba, &txtname, name);

    constraint = ajFmtStr("repeat_consensus.repeat_name = '%s'", txtname);

    ajCharDel(&txtname);

    if (class && ajStrGetLen(class))
    {
        ensBaseadaptorEscapeC(ba, &txtclass, class);

        ajFmtPrintAppS(&constraint,
                       " AND repeat_consensus.repeat_class = '%s'",
                       txtclass);

        ajCharDel(&txtclass);
    }

    rci = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        rci);

    if (ajListGetLength(rci) > 1)
        ajFatal("ensRepeatconsensusadaptorFetchByName got more than one "
                "Repeat Consensus from the database for name '%S' and "
                "class '%S'.\n", name, class);

    ajListPop(rci, (void **) Prc);

    while (ajListPop(rci, (void **) &rc))
        ensRepeatconsensusDel(&rc);

    ajListFree(&rci);

    ajStrDel(&constraint);

    return result;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to
** Ensembl Repeat Consensus objects from an Ensembl SQL database.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Repeat Consensus-releated object(s)
** @nam4rule All Retrieve all Ensembl Repeat Consensus-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
** @nam5rule Types Retrieve all Ensembl Repeat Consensus types
**
** @argrule * rca [EnsPRepeatconsensusadaptor]
** Ensembl Repeat Consensus Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer objects
** @argrule AllTypes types [AjPList]
** AJAX List of AJAX String (Ensembl Repeat Consensus type) objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusadaptorRetrieveAllTypes ****************************
**
** Retrieve all Ensembl Repeat Consensus types of
** Ensembl Repeat Consensus objects.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_all_repeat_types
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [u] types [AjPList]
** AJAX List of AJAX String (Ensembl Repeat Consensus type) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorRetrieveAllTypes(
    EnsPRepeatconsensusadaptor rca,
    AjPList types)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr type      = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!rca)
        return ajFalse;

    if (!types)
        return ajFalse;

    dba = ensRepeatconsensusadaptorGetDatabaseadaptor(rca);

    statement = ajStrNewC(
        "SELECT "
        "DISTINCT "
        "repeat_consensus.repeat_type "
        "FROM "
        "repeat_consensus");

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        type = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &type);

        ajListstrPushAppend(types, type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPRepeatfeature] Ensembl Repeat Feature ********************
**
** @nam2rule Repeatfeature Functions for manipulating
** Ensembl Repeat Feature objects
**
** @cc Bio::EnsEMBL::RepeatFeature
** @cc CVS Revision: 1.29
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Repeat Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Repeat Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy rf [const EnsPRepeatfeature] Ensembl Repeat Feature
** @argrule Ini rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @argrule Ini hstart [ajint] Repeat Consensus hit start
** @argrule Ini hend [ajint] Repeat Consensus hit end
** @argrule Ini score [double] Repeat Consensus score
** @argrule Ref rf [EnsPRepeatfeature] Ensembl Repeat Feature
**
** @valrule * [EnsPRepeatfeature] Ensembl Repeat Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatfeatureNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatfeature] Ensembl Repeat Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNewCpy(const EnsPRepeatfeature rf)
{
    EnsPRepeatfeature pthis = NULL;

    if (!rf)
        return NULL;

    AJNEW0(pthis);

    pthis->Adaptor = rf->Adaptor;

    pthis->Identifier = rf->Identifier;

    pthis->Feature = ensFeatureNewRef(rf->Feature);

    pthis->Repeatconsensus = ensRepeatconsensusNewRef(rf->Repeatconsensus);

    pthis->HitStart = rf->HitStart;

    pthis->HitEnd = rf->HitEnd;

    pthis->Score = rf->Score;

    pthis->Use = 1U;

    return pthis;
}




/* @func ensRepeatfeatureNewIni ***********************************************
**
** Constructor for an Ensembl Repeat Feature with initial values.
**
** @cc Bio::EnsEMBL::RepeatFeature::new
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [uN] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] hstart [ajint] Repeat Consensus hit start
** @param [r] hend [ajint] Repeat Consensus hit end
** @param [r] score [double] Repeat Consensus score
**
** @return [EnsPRepeatfeature] Ensembl Repeat Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNewIni(EnsPRepeatfeatureadaptor rfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         EnsPRepeatconsensus rc,
                                         ajint hstart,
                                         ajint hend,
                                         double score)
{
    EnsPRepeatfeature rf = NULL;

    if (!feature)
        return NULL;

    AJNEW0(rf);

    rf->Adaptor = rfa;

    rf->Identifier = identifier;

    rf->Feature = ensFeatureNewRef(feature);

    rf->Repeatconsensus = ensRepeatconsensusNewRef(rc);

    rf->HitStart = hstart;

    rf->HitEnd = hend;

    rf->Score = score;

    rf->Use = 1U;

    return rf;
}




/* @func ensRepeatfeatureNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatfeature] Ensembl Repeat Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNewRef(EnsPRepeatfeature rf)
{
    if (!rf)
        return NULL;

    rf->Use++;

    return rf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Feature
**
** @argrule * Prf [EnsPRepeatfeature*] Ensembl Repeat Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatfeatureDel **************************************************
**
** Default destructor for an Ensembl Repeat Feature.
**
** @param [d] Prf [EnsPRepeatfeature*] Ensembl Repeat Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensRepeatfeatureDel(EnsPRepeatfeature *Prf)
{
    EnsPRepeatfeature pthis = NULL;

    if (!Prf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensRepeatfeatureDel"))
    {
        ajDebug("ensRepeatfeatureDel\n"
                "  *Prf %p\n",
                *Prf);

        ensRepeatfeatureTrace(*Prf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Prf) || --pthis->Use)
    {
        *Prf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensRepeatconsensusDel(&pthis->Repeatconsensus);

    ajMemFree((void **) Prf);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Get Return Repeat Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Repeat Feature Adaptor
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Hit Return hit members
** @nam5rule Start Return the hit start
** @nam5rule End Return the hit end
** @nam5rule Strand Return the hit strand
** @nam4rule Repeatconsensus Return the Ensembl Repeat Consensus
** @nam4rule Score Return the score
**
** @argrule * rf [const EnsPRepeatfeature] Repeat Feature
**
** @valrule Adaptor [EnsPRepeatfeatureadaptor]
** Ensembl Repeat Feature Adaptor or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule HitEnd [ajint] Hit end or 0
** @valrule HitStart [ajint] Hit start or 0
** @valrule HitStrand [ajint] Hit strand or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Repeatconsensus [EnsPRepeatconsensus]
** Ensembl Repeat Consensus or NULL
** @valrule Score [double] Score or 0.0
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatfeatureGetAdaptor *******************************************
**
** Get the Object Adaptor member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatfeatureadaptor ensRepeatfeatureGetAdaptor(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->Adaptor : NULL;
}




/* @func ensRepeatfeatureGetFeature *******************************************
**
** Get the Ensembl Feature member of an Ensembl Repeat Feature.
**
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensRepeatfeatureGetFeature(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->Feature : NULL;
}




/* @func ensRepeatfeatureGetHitEnd ********************************************
**
** Get the hit end member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hend
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajint] Hit end coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensRepeatfeatureGetHitEnd(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->HitEnd : 0;
}




/* @func ensRepeatfeatureGetHitStart ******************************************
**
** Get the hit start member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hstart
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajint] Hit start coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensRepeatfeatureGetHitStart(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->HitStart : 0;
}




/* @func ensRepeatfeatureGetHitStrand *****************************************
**
** Get the hit strand member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hstrand
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajint] Hit strand or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensRepeatfeatureGetHitStrand(
    const EnsPRepeatfeature rf)
{
    return (rf) ? 1 : 0;
}




/* @func ensRepeatfeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensRepeatfeatureGetIdentifier(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->Identifier : 0U;
}




/* @func ensRepeatfeatureGetRepeatconsensus ***********************************
**
** Get the Ensembl Repeat Consensus member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::repeat_consensus
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatfeatureGetRepeatconsensus(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->Repeatconsensus : NULL;
}




/* @func ensRepeatfeatureGetScore *********************************************
**
** Get the score member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::score
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [double] Score or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensRepeatfeatureGetScore(
    const EnsPRepeatfeature rf)
{
    return (rf) ? rf->Score : 0.0;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Set Set one member of a Repeat Feature
** @nam4rule Adaptor Set the Ensembl Repeat Feature Adaptor
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Hit Set hit members
** @nam5rule End Set the hit end
** @nam5rule Start Set the hit start
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Repeatconsensus Set the Ensembl Repeat Consensus
** @nam4rule Score Set the score
**
** @argrule * rf [EnsPRepeatfeature] Ensembl Repeat Feature object
** @argrule Adaptor rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat
**                                                 Feature Adaptor
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule HitEnd hend [ajuint] Hit end
** @argrule HitStart hstart [ajuint] Hit start
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Repeatconsensus rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @argrule Score score [double] Score
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensRepeatfeatureSetAdaptor *******************************************
**
** Set the Ensembl Database Adaptor member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetAdaptor(EnsPRepeatfeature rf,
                                  EnsPRepeatfeatureadaptor rfa)
{
    if (!rf)
        return ajFalse;

    rf->Adaptor = rfa;

    return ajTrue;
}




/* @func ensRepeatfeatureSetFeature *******************************************
**
** Set the Ensembl Feature member of an Ensembl Repeat Feature.
**
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetFeature(EnsPRepeatfeature rf,
                                  EnsPFeature feature)
{
    if (!rf)
        return ajFalse;

    ensFeatureDel(&rf->Feature);

    rf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensRepeatfeatureSetHitEnd ********************************************
**
** Set the hit end member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hend
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] hend [ajuint] Hit end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetHitEnd(EnsPRepeatfeature rf,
                                 ajuint hend)
{
    if (!rf)
        return ajFalse;

    rf->HitEnd = hend;

    return ajTrue;
}




/* @func ensRepeatfeatureSetHitStart ******************************************
**
** Set the hit start member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hstart
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] hstart [ajuint] Hit start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetHitStart(EnsPRepeatfeature rf,
                                   ajuint hstart)
{
    if (!rf)
        return ajFalse;

    rf->HitStart = hstart;

    return ajTrue;
}




/* @func ensRepeatfeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetIdentifier(EnsPRepeatfeature rf,
                                     ajuint identifier)
{
    if (!rf)
        return ajFalse;

    rf->Identifier = identifier;

    return ajTrue;
}




/* @func ensRepeatfeatureSetRepeatconsensus ***********************************
**
** Set the Ensembl Repeat Consensus member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::repeat_consensus
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetRepeatconsensus(EnsPRepeatfeature rf,
                                          EnsPRepeatconsensus rc)
{
    if (!rf)
        return ajFalse;

    ensRepeatconsensusDel(&rf->Repeatconsensus);

    rf->Repeatconsensus = ensRepeatconsensusNewRef(rc);

    return ajTrue;
}




/* @func ensRepeatfeatureSetScore *********************************************
**
** Set the score member of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::score
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetScore(EnsPRepeatfeature rf,
                                double score)
{
    if (!rf)
        return ajFalse;

    rf->Score = score;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Trace Report Ensembl Repeat Feature members to debug file
**
** @argrule Trace rf [const EnsPRepeatfeature] Ensembl Repeat Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensRepeatfeatureTrace ************************************************
**
** Trace an Ensembl Repeat Feature.
**
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureTrace(const EnsPRepeatfeature rf, ajuint level)
{
    AjPStr indent = NULL;

    if (!rf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensRepeatfeatureTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Identifier %u\n"
            "%S  Feature %p\n"
            "%S  Repeatconsensus %p\n"
            "%S  HitStart %d\n"
            "%S  HitEnd %d\n"
            "%S  Score %f\n"
            "%S  Use %u\n",
            indent, rf,
            indent, rf->Adaptor,
            indent, rf->Identifier,
            indent, rf->Feature,
            indent, rf->Repeatconsensus,
            indent, rf->HitStart,
            indent, rf->HitEnd,
            indent, rf->Score,
            indent, rf->Use);

    ensFeatureTrace(rf->Feature, level + 1);

    ensRepeatconsensusTrace(rf->Repeatconsensus, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Calculate Calculate Ensembl Repeat Feature information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensRepeatfeatureCalculateMemsize *************************************
**
** Calculate the memory size in bytes of an Ensembl Repeat Feature.
**
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensRepeatfeatureCalculateMemsize(const EnsPRepeatfeature rf)
{
    size_t size = 0;

    if (!rf)
        return 0;

    size += sizeof (EnsORepeatfeature);

    size += ensFeatureCalculateMemsize(rf->Feature);

    size += ensRepeatconsensusCalculateMemsize(rf->Repeatconsensus);

    return size;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listRepeatfeatureCompareEndAscending ***************************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** Ensembl Feature end member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Repeat Feature address 1
** @param [r] item2 [const void*] Ensembl Repeat Feature address 2
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

static int listRepeatfeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPRepeatfeature rf1 = *(EnsORepeatfeature *const *) item1;
    EnsPRepeatfeature rf2 = *(EnsORepeatfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listRepeatfeatureCompareEndAscending"))
        ajDebug("listRepeatfeatureCompareEndAscending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (rf1 && (!rf2))
        return -1;

    if ((!rf1) && (!rf2))
        return 0;

    if ((!rf1) && rf2)
        return +1;

    return ensFeatureCompareEndAscending(rf1->Feature, rf2->Feature);
}




/* @funcstatic listRepeatfeatureCompareEndDescending **************************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** Ensembl Feature end member in descending order.
**
** @param [r] item1 [const void*] Ensembl Repeat Feature address 1
** @param [r] item2 [const void*] Ensembl Repeat Feature address 2
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

static int listRepeatfeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPRepeatfeature rf1 = *(EnsORepeatfeature *const *) item1;
    EnsPRepeatfeature rf2 = *(EnsORepeatfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listRepeatfeatureCompareEndDescending"))
        ajDebug("listRepeatfeatureCompareEndDescending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (rf1 && (!rf2))
        return -1;

    if ((!rf1) && (!rf2))
        return 0;

    if ((!rf1) && rf2)
        return +1;

    return ensFeatureCompareEndDescending(rf1->Feature, rf2->Feature);
}




/* @funcstatic listRepeatfeatureCompareIdentifierAscending ********************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** identifier member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Repeat Feature address 1
** @param [r] item2 [const void*] Ensembl Repeat Feature address 2
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

static int listRepeatfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPRepeatfeature rf1 = *(EnsORepeatfeature *const *) item1;
    EnsPRepeatfeature rf2 = *(EnsORepeatfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listRepeatfeatureCompareIdentifierAscending"))
        ajDebug("listRepeatfeatureCompareIdentifierAscending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (rf1 && (!rf2))
        return -1;

    if ((!rf1) && (!rf2))
        return 0;

    if ((!rf1) && rf2)
        return +1;

    if (rf1->Identifier < rf2->Identifier)
        return -1;

    if (rf1->Identifier > rf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listRepeatfeatureCompareStartAscending *************************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** Ensembl Feature start member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Repeat Feature address 1
** @param [r] item2 [const void*] Ensembl Repeat Feature address 2
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

static int listRepeatfeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPRepeatfeature rf1 = *(EnsORepeatfeature *const *) item1;
    EnsPRepeatfeature rf2 = *(EnsORepeatfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listRepeatfeatureCompareStartAscending"))
        ajDebug("listRepeatfeatureCompareStartAscending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (rf1 && (!rf2))
        return -1;

    if ((!rf1) && (!rf2))
        return 0;

    if ((!rf1) && rf2)
        return +1;

    return ensFeatureCompareStartAscending(rf1->Feature, rf2->Feature);
}




/* @funcstatic listRepeatfeatureCompareStartDescending ************************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** Ensembl Feature start member in descending order.
**
** @param [r] item1 [const void*] Ensembl Repeat Feature address 1
** @param [r] item2 [const void*] Ensembl Repeat Feature address 2
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

static int listRepeatfeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPRepeatfeature rf1 = *(EnsORepeatfeature *const *) item1;
    EnsPRepeatfeature rf2 = *(EnsORepeatfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listRepeatfeatureCompareStartDescending"))
        ajDebug("listRepeatfeatureCompareStartDescending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (rf1 && (!rf2))
        return -1;

    if ((!rf1) && (!rf2))
        return 0;

    if ((!rf1) && rf2)
        return +1;

    return ensFeatureCompareStartDescending(rf1->Feature, rf2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Repeatfeature Functions for manipulating AJAX List objects of
** Ensembl Repeat Feature objects
** @nam4rule            Sort Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * rfs [AjPList]
** AJAX List of Ensembl Repeat Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListRepeatfeatureSortEndAscending *********************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** Ensembl Feature end member in ascending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortEndAscending(AjPList rfs)
{
    if (!rfs)
        return ajFalse;

    ajListSortTwoThree(rfs,
                       &listRepeatfeatureCompareEndAscending,
                       &listRepeatfeatureCompareStartAscending,
                       &listRepeatfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListRepeatfeatureSortEndDescending ********************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** Ensembl Feature end member in descending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortEndDescending(AjPList rfs)
{
    if (!rfs)
        return ajFalse;

    ajListSortTwoThree(rfs,
                       &listRepeatfeatureCompareEndDescending,
                       &listRepeatfeatureCompareStartDescending,
                       &listRepeatfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListRepeatfeatureSortIdentifierAscending **************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** identifier member in ascending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortIdentifierAscending(AjPList rfs)
{
    if (!rfs)
        return ajFalse;

    ajListSort(rfs, &listRepeatfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListRepeatfeatureSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** Ensembl Feature start member in ascending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortStartAscending(AjPList rfs)
{
    if (!rfs)
        return ajFalse;

    ajListSortTwoThree(rfs,
                       &listRepeatfeatureCompareStartAscending,
                       &listRepeatfeatureCompareEndAscending,
                       &listRepeatfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListRepeatfeatureSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** Ensembl Feature start member in descending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortStartDescending(AjPList rfs)
{
    if (!rfs)
        return ajFalse;

    ajListSortTwoThree(rfs,
                       &listRepeatfeatureCompareStartDescending,
                       &listRepeatfeatureCompareEndDescending,
                       &listRepeatfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor *****
**
** @nam2rule Repeatfeatureadaptor Functions for manipulating
** Ensembl Repeat Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor
** @cc CVS Revision: 1.63
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic repeatfeatureadaptorLinkRepeatconsensus ************************
**
** An ajTableMapDel "apply" function to link Ensembl Repeat Feature objects to
** Ensembl Repeat Consensus objects.
** This function also deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Repeat Feature objects after association.
**
** @param [d] Pkey [void**] AJAX unsigned integer key data address
** @param [d] Pvalue [void**] AJAX List of Ensembl Repeat Feature objects
** @param [u] cl [void*] AJAX Table of Ensembl Repeat Consensus objects,
**                       passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void repeatfeatureadaptorLinkRepeatconsensus(void **Pkey,
                                                    void **Pvalue,
                                                    void *cl)
{
    EnsPRepeatconsensus rc = NULL;

    EnsPRepeatfeature rf = NULL;

    if (!Pkey)
        return;

    if (!*Pkey)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    rc = (EnsPRepeatconsensus) ajTableFetchmodV(cl, *Pkey);

    /*
    ** The Ensembl Repeat Feature objects can be deleted after associating
    ** them with Ensembl Repeat Consensus objects, because this AJAX Table
    ** holds independent references for these objects.
    */

    while (ajListPop(*((AjPList *) Pvalue), (void **) &rf))
    {
        ensRepeatfeatureSetRepeatconsensus(rf, rc);

        ensRepeatfeatureDel(&rf);
    }

    ajMemFree(Pkey);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic repeatfeatureadaptorFetchAllbyStatement ************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Repeat Features.
** The caller is responsible for deleting the Ensembl Repeat Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool repeatfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rfs)
{
    ajuint *Pidentifier = NULL;

    ajuint anid  = 0U;
    ajuint rcid  = 0U;
    ajuint rfid  = 0U;
    ajuint srid  = 0U;

    ajuint rpstart = 0U;
    ajuint rpend   = 0U;

    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    double score = 0;

    AjBool debug = AJFALSE;

    AjPList list = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPTable rci     = NULL;
    AjPTable rcitorf = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPRepeatconsensusadaptor rca = NULL;

    EnsPRepeatfeature        rf  = NULL;
    EnsPRepeatfeatureadaptor rfa = NULL;

    debug = ajDebugTest("repeatfeatureadaptorFetchAllbyStatement");

    if (debug)
        ajDebug("repeatfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  rfs %p\n",
                ba,
                statement,
                am,
                slice,
                rfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!rfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    rca = ensRegistryGetRepeatconsensusadaptor(dba);
    rfa = ensRegistryGetRepeatfeatureadaptor(dba);

    rci     = ajTableuintNew(0U);
    rcitorf = ajTableuintNew(0U);

    ajTableSetDestroyvalue(rci, (void (*)(void **)) &ensRepeatconsensusDel);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        rfid     = 0;
        srid     = 0;
        srstart  = 0;
        srend    = 0;
        srstrand = 0;
        rcid     = 0;
        rpstart  = 0;
        rpend    = 0;
        anid     = 0;
        score    = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &rfid);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &rcid);
        ajSqlcolumnToUint(sqlr, &rpstart);
        ajSqlcolumnToUint(sqlr, &rpend);
        ajSqlcolumnToUint(sqlr, &anid);
        ajSqlcolumnToDouble(sqlr, &score);

        ensBaseadaptorRetrieveFeature(ba,
                                      anid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
            continue;

        /* Finally, create a new Ensembl Repeat Feature. */

        rf = ensRepeatfeatureNewIni(rfa,
                                    rfid,
                                    feature,
                                    (EnsPRepeatconsensus) NULL,
                                    rpstart,
                                    rpend,
                                    score);

        ajListPushAppend(rfs, (void *) rf);

        /*
        ** Populate two AJAX Table objects to fetch Ensembl Repeat Consensus
        ** objects from the database and associate them with
        ** Ensembl Repeat Feature objects.
        **
        ** rci
        **   key data:   AJAX unsigned integer
        **               (Ensembl Repeat Consensus identifier) objects
        **   value data: Ensembl Repeat Consensus objects fetched by
        **               ensRepeatconsensusadaptorFetchAllbyIdentifiers
        **
        ** rcitorf
        **   key data:   AJAX unsigned integer
        **               (Ensembl Repeat Consensus identifier) objects
        **   value data: AJAX List objects of Ensembl Repeat Feature objects
        **               that need to be associated with Ensembl Repeat
        **               Consensus objects once they have been fetched
        **               from the database
        */

        if (!ajTableMatchV(rci, (const void *) &rcid))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = rcid;

            ajTablePut(rci, (void *) Pidentifier, NULL);
        }

        list = (AjPList) ajTableFetchmodV(rcitorf, (const void *) &rcid);

        if (!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = rcid;

            list = ajListNew();

            ajTablePut(rcitorf, (void *) Pidentifier, (void *) list);
        }

        ajListPushAppend(list, (void *) ensRepeatfeatureNewRef(rf));

        ensFeatureDel(&feature);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ensRepeatconsensusadaptorFetchAllbyIdentifiers(rca, rci);

    /*
    ** Link
    ** Ensembl Repeat Consensus objects to
    ** Ensembl Repeat Feature objects.
    */

    ajTableMapDel(rcitorf,
                  &repeatfeatureadaptorLinkRepeatconsensus,
                  (void *) rci);

    ajTableFree(&rcitorf);

    /* Delete the utility AJAX Table. */

    ajTableDel(&rci);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Repeat Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Repeat Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPRepeatfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPRepeatfeatureadaptor]
** Ensembl Repeat Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatfeatureadaptorNew *******************************************
**
** Default constructor for an Ensembl Repeat Feature Adaptor.
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
** @see ensRegistryGetRepeatfeatureadaptor
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPRepeatfeatureadaptor ensRepeatfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        repeatfeatureadaptorKTablenames,
        repeatfeatureadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        repeatfeatureadaptorKDefaultcondition,
        (const char *) NULL,
        &repeatfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensRepeatfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensRepeatfeatureDel,
        (size_t (*)(const void *)) &ensRepeatfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensRepeatfeatureGetFeature,
        "Repeatfeature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Repeat Feature Adaptor object.
**
** @fdata [EnsPRepeatfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Feature Adaptor
**
** @argrule * Prfa [EnsPRepeatfeatureadaptor*]
** Ensembl Repeat Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatfeatureadaptorDel *******************************************
**
** Default destructor for an Ensembl Repeat Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Prfa [EnsPRepeatfeatureadaptor*]
** Ensembl Repeat Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensRepeatfeatureadaptorDel(EnsPRepeatfeatureadaptor *Prfa)
{
    ensFeatureadaptorDel(Prfa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Repeat Feature Adaptor
** object.
**
** @fdata [EnsPRepeatfeatureadaptor]
**
** @nam3rule Get Return Repeat Feature Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
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




/* @func ensRepeatfeatureadaptorGetBaseadaptor ********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Repeat Feature Adaptor.
**
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensRepeatfeatureadaptorGetBaseadaptor(
    EnsPRepeatfeatureadaptor rfa)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensRepeatfeatureadaptorGetFeatureadaptor(rfa));
}




/* @func ensRepeatfeatureadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Repeat Feature Adaptor.
**
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensRepeatfeatureadaptorGetDatabaseadaptor(
    EnsPRepeatfeatureadaptor rfa)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensRepeatfeatureadaptorGetFeatureadaptor(rfa));
}




/* @func ensRepeatfeatureadaptorGetFeatureadaptor *****************************
**
** Get the Ensembl Feature Adaptor member of an
** Ensembl Repeat Feature Adaptor.
**
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensRepeatfeatureadaptorGetFeatureadaptor(
    EnsPRepeatfeatureadaptor rfa)
{
    return rfa;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Repeat Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPRepeatfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Repeat Feature object(s)
** @nam4rule All Fetch all Ensembl Repeat Feature objects
** @nam4rule Allby Fetch all Ensembl Repeat Feature objects
**                 matching a criterion
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam4rule By Fetch one Ensembl Repeat Feature object
**              matching a criterion
**
** @argrule * rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice rctype [const AjPStr] Ensembl Repeat Consensus type
** @argrule AllbySlice rcclass [const AjPStr] Ensembl Repeat Consensus class
** @argrule AllbySlice rcname [const AjPStr] Ensembl Repeat Consensus name
** @argrule AllbySlice rfs [AjPList]
** AJAX List of Ensembl Repeat Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatfeatureadaptorFetchAllbySlice *******************************
**
** Fetch all Ensembl Repeat Feature objects on an Ensembl Slice.
** The caller is responsible for deleting the Ensembl Repeat Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor::fetch_all_by_Slice
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [rN] rctype [const AjPStr] Ensembl Repeat Consensus type
** @param [rN] rcclass [const AjPStr] Ensembl Repeat Consensus class
** @param [rN] rcname [const AjPStr] Ensembl Repeat Consensus name
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensRepeatfeatureadaptorFetchAllbySlice(
    EnsPRepeatfeatureadaptor rfa,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr rctype,
    const AjPStr rcclass,
    const AjPStr rcname,
    AjPList rfs)
{
    char *txtrcclass = NULL;
    char *txtrcname = NULL;
    char *txtrctype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPFeatureadaptor fa = NULL;

    if (ajDebugTest("ensRepeatfeatureadaptorFetchAllbySlice"))
        ajDebug("ensRepeatfeatureadaptorFetchAllbySlice\n"
                "  rfa %p\n"
                "  slice %p\n"
                "  anname '%S'\n"
                "  rctype '%S'\n"
                "  rcclass '%S'\n"
                "  rcname '%S'\n"
                "  rfs %p\n",
                rfa,
                slice,
                anname,
                rctype,
                rcclass,
                rcname,
                rfs);

    if (!rfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!rfs)
        return ajFalse;

    fa = ensRepeatfeatureadaptorGetFeatureadaptor(rfa);

    if (rctype && ajStrGetLen(rctype))
    {
        ensFeatureadaptorEscapeC(fa, &txtrctype, rctype);

        constraint = ajFmtStr("repeat_consensus.repeat_type = '%s'",
                              txtrctype);

        ajCharDel(&txtrctype);
    }

    if (rcclass && ajStrGetLen(rcclass))
    {
        ensFeatureadaptorEscapeC(fa, &txtrcclass, rcclass);

        if (constraint)
            ajFmtPrintAppS(&constraint,
                           " AND repeat_consensus.repeat_class = '%s'",
                           txtrcclass);
        else
            constraint = ajFmtStr("repeat_consensus.repeat_class = '%s'",
                                  txtrcclass);

        ajCharDel(&txtrcclass);
    }

    if (rcname && ajStrGetLen(rcname))
    {
        ensFeatureadaptorEscapeC(fa, &txtrcname, rcname);

        if (constraint)
            ajFmtPrintAppS(&constraint,
                           " AND repeat_consensus.repeat_name = '%s'",
                           txtrcname);
        else
            constraint = ajFmtStr("repeat_consensus.repeat_name = '%s'",
                                  txtrcname);

        ajCharDel(&txtrcname);
    }

    result = ensFeatureadaptorFetchAllbySlice(
        fa,
        slice,
        constraint,
        anname,
        rfs);

    ajStrDel(&constraint);

    return result;
}

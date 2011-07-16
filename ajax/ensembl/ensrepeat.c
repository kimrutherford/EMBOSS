/* @source Ensembl Repeat functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.39 $
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

#include "ensrepeat.h"
#include "enstable.h"




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

/* @conststatic repeatconsensusadaptorTables **********************************
**
** Array of Ensembl Repeat Consensus Adaptor SQL table names
**
******************************************************************************/

static const char* repeatconsensusadaptorTables[] =
{
    "repeat_consensus",
    (const char*) NULL
};




/* @conststatic repeatconsensusadaptorColumns *********************************
**
** Array of Ensembl Repeat Consensus Adaptor SQL column names
**
******************************************************************************/

static const char* repeatconsensusadaptorColumns[] =
{
    "repeat_consensus.repeat_consensus_id",
    "repeat_consensus.repeat_name",
    "repeat_consensus.repeat_class",
    "repeat_consensus.repeat_type",
    "repeat_consensus.repeat_consensus",
    (const char*) NULL
};




/* @conststatic repeatfeatureadaptorTables ************************************
**
** Array of Ensembl Repeat Feature Adaptor SQL table names
**
******************************************************************************/

static const char* repeatfeatureadaptorTables[] =
{
    "repeat_feature",
    "repeat_consensus",
    (const char*) NULL
};




/* @conststatic repeatfeatureadaptorColumns ***********************************
**
** Array of Ensembl Repeat Feature Adaptor SQL column names
**
******************************************************************************/

static const char* repeatfeatureadaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic repeatfeatureadaptorDefaultcondition **************************
**
** Ensembl Repeat Feature Adaptor SQL default condition
**
******************************************************************************/

static const char* repeatfeatureadaptorDefaultcondition =
    "repeat_feature.repeat_consensus_id = "
    "repeat_consensus.repeat_consensus_id";




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void tableRepeatconsensusClear(void** key,
                                      void** value,
                                      void* cl);

static AjBool repeatconsensusadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rci);

static int listRepeatfeatureCompareStartAscending(const void* P1,
                                                  const void* P2);

static int listRepeatfeatureCompareStartDescending(const void* P1,
                                                   const void* P2);

static void* repeatfeatureadaptorCacheReference(void* value);

static void repeatfeatureadaptorCacheDelete(void** value);

static size_t repeatfeatureadaptorCacheSize(const void* value);

static EnsPFeature repeatfeatureadaptorGetFeature(const void* rf);

static void repeatfeatureadaptorLinkRepeatconsensus(void** key,
                                                    void** value,
                                                    void* cl);

static AjBool repeatfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rfs);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-62
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
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNewCpy(const EnsPRepeatconsensus rc)
{
    EnsPRepeatconsensus pthis = NULL;

    if(!rc)
        return NULL;

    AJNEW0(pthis);

    pthis->Adaptor = rc->Adaptor;

    pthis->Identifier = rc->Identifier;

    if(rc->Name)
        pthis->Name = ajStrNewRef(rc->Name);

    if(rc->Class)
        pthis->Class = ajStrNewRef(rc->Class);

    if(rc->Type)
        pthis->Type = ajStrNewRef(rc->Type);

    if(rc->Consensus)
        pthis->Consensus = ajStrNewRef(rc->Consensus);

    pthis->Length = rc->Length;

    pthis->Use = 1;

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

    if(name)
        rc->Name = ajStrNewRef(name);

    if(class)
        rc->Class = ajStrNewRef(class);

    if(type)
        rc->Type = ajStrNewRef(type);

    if(consensus)
        rc->Consensus = ajStrNewRef(consensus);

    rc->Length = length;

    rc->Use = 1;

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
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNewRef(EnsPRepeatconsensus rc)
{
    if(!rc)
        return NULL;

    rc->Use++;

    return rc;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Consensus object
**
** @argrule * Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus
**                                       object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatconsensusDel ************************************************
**
** Default destructor for an Ensembl Repeat Consensus.
**
** @param [d] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus
**                                       object address
**
** @return [void]
** @@
******************************************************************************/

void ensRepeatconsensusDel(EnsPRepeatconsensus* Prc)
{
    EnsPRepeatconsensus pthis = NULL;

    if(!Prc)
        return;

    if(!*Prc)
        return;

    if(ajDebugTest("ensRepeatconsensusDel"))
        ajDebug("ensRepeatconsensusDel\n"
                "  *Prc %p\n",
                *Prc);

    pthis = *Prc;

    pthis->Use--;

    if(pthis->Use)
    {
        *Prc = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Class);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Consensus);

    AJFREE(pthis);

    *Prc = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Repeat Consensus object.
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
** @valrule Adaptor [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus
**                                               Adaptor
** @valrule Class [AjPStr] Class
** @valrule Consensus [AjPStr] Consensus sequence
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Length [ajuint] Length
** @valrule Name [AjPStr] Name
** @valrule Type [AjPStr] Type
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusGetAdaptor *****************************************
**
** Get the Ensembl Repeat Consensus Adaptor element of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @@
******************************************************************************/

EnsPRepeatconsensusadaptor ensRepeatconsensusGetAdaptor(
    const EnsPRepeatconsensus rc)
{
    if(!rc)
        return NULL;

    return rc->Adaptor;
}




/* @func ensRepeatconsensusGetClass *******************************************
**
** Get the class element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_class
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Class
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetClass(const EnsPRepeatconsensus rc)
{
    if(!rc)
        return NULL;

    return rc->Class;
}




/* @func ensRepeatconsensusGetConsensus ***************************************
**
** Get the consensus sequence element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_consensus
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Consensus sequence
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetConsensus(const EnsPRepeatconsensus rc)
{
    if(!rc)
        return NULL;

    return rc->Consensus;
}




/* @func ensRepeatconsensusGetIdentifier **************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensRepeatconsensusGetIdentifier(const EnsPRepeatconsensus rc)
{
    if(!rc)
        return 0;

    return rc->Identifier;
}




/* @func ensRepeatconsensusGetLength ******************************************
**
** Get the length element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::length
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [ajuint] Length
** @@
******************************************************************************/

ajuint ensRepeatconsensusGetLength(const EnsPRepeatconsensus rc)
{
    if(!rc)
        return 0;

    return rc->Length;
}




/* @func ensRepeatconsensusGetName ********************************************
**
** Get the name element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::name
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetName(const EnsPRepeatconsensus rc)
{
    if(!rc)
        return NULL;

    return rc->Name;
}




/* @func ensRepeatconsensusGetType ********************************************
**
** Get the type element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_type
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjPStr] Type
** @@
******************************************************************************/

AjPStr ensRepeatconsensusGetType(const EnsPRepeatconsensus rc)
{
    if(!rc)
        return NULL;

    return rc->Type;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Set Set one element of a Repeat Consensus
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
** Set the Ensembl Repeat Consensus Adaptor element of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetAdaptor(EnsPRepeatconsensus rc,
                                    EnsPRepeatconsensusadaptor rca)
{
    if(!rc)
        return ajFalse;

    rc->Adaptor = rca;

    return ajTrue;
}




/* @func ensRepeatconsensusSetClass *******************************************
**
** Set the class element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_class
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] class [AjPStr] Class
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetClass(EnsPRepeatconsensus rc,
                                  AjPStr class)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Class);

    if(class)
        rc->Class = ajStrNewRef(class);

    return ajTrue;
}




/* @func ensRepeatconsensusSetConsensus ***************************************
**
** Set the consensus sequence element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_consensus
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] consensus [AjPStr] Consensus sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetConsensus(EnsPRepeatconsensus rc,
                                      AjPStr consensus)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Consensus);

    if(consensus)
        rc->Consensus = ajStrNewRef(consensus);

    return ajTrue;
}




/* @func ensRepeatconsensusSetIdentifier **************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetIdentifier(EnsPRepeatconsensus rc,
                                       ajuint identifier)
{
    if(!rc)
        return ajFalse;

    rc->Identifier = identifier;

    return ajTrue;
}




/* @func ensRepeatconsensusSetLength ******************************************
**
** Set the length element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::length
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] length [ajuint] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetLength(EnsPRepeatconsensus rc,
                                   ajuint length)
{
    if(!rc)
        return ajFalse;

    rc->Length = length;

    return ajTrue;
}




/* @func ensRepeatconsensusSetName ********************************************
**
** Set the name element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::name
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetName(EnsPRepeatconsensus rc,
                                 AjPStr name)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Name);

    if(name)
        rc->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensRepeatconsensusSetType ********************************************
**
** Set the type element of an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::repeat_type
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [u] type [AjPStr] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusSetType(EnsPRepeatconsensus rc,
                                 AjPStr type)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Type);

    if(type)
        rc->Type = ajStrNewRef(type);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Trace Report Ensembl Repeat Consensus elements to debug file
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
** @@
******************************************************************************/

AjBool ensRepeatconsensusTrace(const EnsPRepeatconsensus rc, ajuint level)
{
    AjPStr indent = NULL;

    if(!rc)
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
** Functions for calculating values of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
**
** @nam3rule Calculate Calculate Ensembl Repeat Consensus values
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
** @@
******************************************************************************/

size_t ensRepeatconsensusCalculateMemsize(const EnsPRepeatconsensus rc)
{
    size_t size = 0;

    if(!rc)
        return 0;

    size += sizeof (EnsORepeatconsensus);

    if(rc->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Name);
    }

    if(rc->Class)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Class);
    }

    if(rc->Type)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Type);
    }

    if(rc->Consensus)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(rc->Consensus);
    }

    return size;
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Table Functions for manipulating AJAX Table objects
**
******************************************************************************/




/* @section table *************************************************************
**
** Functions for manipulating AJAX Table objects.
**
** @fdata [AjPTable]
**
** @nam3rule Repeatconsensus AJAX Table of AJAX unsigned integer key data and
**                           Ensembl Repeat Consensus value data
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




/* @funcstatic tableRepeatconsensusClear **************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Repeat Consensus value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Repeat Consensus address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableRepeatconsensusClear(void** key,
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

    ensRepeatconsensusDel((EnsPRepeatconsensus*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableRepeatconsensusClear *****************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Repeat Consensus value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableRepeatconsensusClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableRepeatconsensusClear, NULL);

    return ajTrue;
}




/* @func ensTableRepeatconsensusDelete ****************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Repeat Consensus value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableRepeatconsensusDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableRepeatconsensusClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @nam2rule Repeatconsensusadaptor Functions for manipulating
** Ensembl Repeat Consensus Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor
** @cc CVS Revision: 1.23
** @cc CVS Tag: branch-ensembl-62
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] rci [AjPList] AJAX List of Ensembl Repeat Consensi
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool repeatconsensusadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rci)
{
    const char* Ptr = NULL;

    register ajuint i = 0;

    ajuint identifier = 0;
    ajuint length     = 0;

    AjPRegexp rp = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr class     = NULL;
    AjPStr type      = NULL;
    AjPStr consensus = NULL;
    AjPStr number    = NULL;

    EnsPRepeatconsensus rc         = NULL;
    EnsPRepeatconsensusadaptor rca = NULL;

    if(ajDebugTest("repeatconsensusadaptorFetchAllbyStatement"))
        ajDebug("repeatconsensusadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  rci %p\n",
                dba,
                statement,
                am,
                slice,
                rci);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!rci)
        return ajFalse;

    rp = ajRegCompC("^(\\d+)\\(N\\)$");

    number = ajStrNew();

    rca = ensRegistryGetRepeatconsensusadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name       = ajStrNew();
        class      = ajStrNew();
        type       = ajStrNew();
        consensus  = ajStrNew();
        length     = 0;

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

        if(ajRegExec(rp, consensus))
        {
            ajRegSubI(rp, 1, &number);

            if(!ajStrToUint(number, &length))
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

            for(i = 0, Ptr = ajStrGetPtr(consensus); (Ptr && *Ptr); i++, Ptr++)
                if(i == UINT_MAX)
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

        ajListPushAppend(rci, (void*) rc);

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
** @valrule * [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
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
** @return [EnsPRepeatconsensusadaptor] Ensembl Repat Consensus Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPRepeatconsensusadaptor ensRepeatconsensusadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    if(ajDebugTest("ensRepeatconsensusadaptorNew"))
        ajDebug("ensRepeatconsensusadaptorNew\n"
                "  dba %p\n",
                dba);

    return ensBaseadaptorNew(
        dba,
        repeatconsensusadaptorTables,
        repeatconsensusadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        repeatconsensusadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Repeat Consensus Adaptor object.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Consensus Adaptor object
**
** @argrule * Prca [EnsPRepeatconsensusadaptor*]
** Ensembl Repeat Consensus Adaptor object address
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
** Ensembl Repeat Consensus Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensRepeatconsensusadaptorDel(EnsPRepeatconsensusadaptor* Prca)
{
    if(!Prca)
        return;

    if(!*Prca)
        return;

    ensBaseadaptorDel(Prca);

    *Prca = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Repeat Consensus Adaptor
** object.
**
** @fdata [EnsPRepeatconsensusadaptor]
**
** @nam3rule Get Return Repeat Consensus Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusadaptorGetDatabaseadaptor **************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Repeat Consensus Adaptor.
**
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensRepeatconsensusadaptorGetDatabaseadaptor(
    EnsPRepeatconsensusadaptor rca)
{
    if(!rca)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(rca);
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
** @argrule * rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @argrule AllbyClassconsensus class [const AjPStr] Class
** @argrule AllbyClassconsensus consensus [const AjPStr] Consensus sequence
** @argrule AllbyClassconsensus rci [AjPList] AJAX List of
**                                            Ensembl Repeat Consensus objects
** @argrule AllbyIdentifiers rcit [AjPTable] AJAX Table of
**                                           Ensembl Repeat Consensus objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus
**                                                  address
** @argrule ByName name [const AjPStr] Name
** @argrule ByName class [const AjPStr] Class
** @argrule ByName Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus
**                                            address
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
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchAllbyClassconsensus(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr class,
    const AjPStr consensus,
    AjPList rci)
{
    char* txtclass     = NULL;
    char* txtconsensus = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!rca)
        return ajFalse;

    if(!class)
        return ajFalse;

    if(!consensus)
        return ajFalse;

    if(!rci)
        return ajFalse;

    ensBaseadaptorEscapeC(rca, &txtclass, class);
    ensBaseadaptorEscapeC(rca, &txtconsensus, consensus);

    constraint = ajFmtStr(
        "repeat_consensus.repeat_class = '%s' "
        "AND "
        "repeat_consensus.repeat_consensus = '%s'",
        txtclass,
        txtconsensus);

    ajCharDel(&txtclass);
    ajCharDel(&txtconsensus);

    result = ensBaseadaptorFetchAllbyConstraint(rca,
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
** The caller is responsible for deleting the AJAX unisgned integer key and
** Ensembl Repeat Consensus value data before deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus
**                                             Adaptor
** @param [u] rcit [AjPTable] AJAX Table of AJAX unsigned integer (identifier)
**                            key data and Ensembl Repeat Consensus value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchAllbyIdentifiers(
    EnsPRepeatconsensusadaptor rca,
    AjPTable rcit)
{
    void** keyarray = NULL;

    const char* template = "repeat_consensus.repeat_consensus_id IN (%S)";

    register ajuint i = 0;

    ajuint identifier = 0;

    ajuint* Pidentifier = NULL;

    AjPList lrci = NULL;

    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    EnsPRepeatconsensus rc = NULL;

    if(!rca)
        return ajFalse;

    if(!rcit)
        return ajFalse;

    lrci = ajListNew();

    csv = ajStrNew();

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifiers configured in the Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    ajTableToarrayKeys(rcit, &keyarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint*) keyarray[i]));

        /* Run the statement if the maximum chunk size is exceed. */

        if(((i + 1) % ensBaseadaptorMaximumIdentifiers) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            constraint = ajFmtStr(template, csv);

            ensBaseadaptorFetchAllbyConstraint(rca,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               lrci);

            ajStrDel(&constraint);

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
    {
        constraint = ajFmtStr(template, csv);

        ensBaseadaptorFetchAllbyConstraint(rca,
                                           constraint,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           lrci);

        ajStrDel(&constraint);
    }

    ajStrDel(&csv);

    /*
    ** Move Ensembl Genetic Variation Population objects from the AJAX List
    ** to the AJAX Table.
    */

    while(ajListPop(lrci, (void**) &rc))
    {
        identifier = ensRepeatconsensusGetIdentifier(rc);

        if(ajTableMatchV(rcit, (const void*) &identifier))
            ajTablePut(rcit, (void*) &identifier, (void*) rc);
        else
        {
            /*
            ** This should not happen, because the keys should have been in
            ** the AJAX Table in the first place.
            */

            AJNEW0(Pidentifier);

            *Pidentifier = ensRepeatconsensusGetIdentifier(rc);

            ajTablePut(rcit, (void*) Pidentifier, (void*) rc);
        }
    }

    ajListFree(&lrci);

    return ajTrue;
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
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByIdentifier(
    EnsPRepeatconsensusadaptor rca,
    ajuint identifier,
    EnsPRepeatconsensus* Prc)
{
    if(!rca)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Prc)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(rca, identifier, (void**) Prc);
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
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByName(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr name,
    const AjPStr class,
    EnsPRepeatconsensus* Prc)
{
    char* txtname  = NULL;
    char* txtclass = NULL;

    AjPList rci = NULL;

    AjPStr constraint = NULL;

    EnsPRepeatconsensus rc = NULL;

    if(!rca)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Prc)
        return ajFalse;

    ensBaseadaptorEscapeC(rca, &txtname, name);

    constraint = ajFmtStr("repeat_consensus.repeat_name = '%s'", txtname);

    ajCharDel(&txtname);

    if(class && ajStrGetLen(class))
    {
        ensBaseadaptorEscapeC(rca, &txtclass, class);

        ajFmtPrintAppS(&constraint,
                       " AND repeat_consensus.repeat_class = '%s'",
                       txtclass);

        ajCharDel(&txtclass);
    }

    rci = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(rca,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       rci);

    if(ajListGetLength(rci) > 1)
        ajFatal("ensRepeatconsensusadaptorFetchByName got more than one "
                "Repeat Consensus from the database for name '%S' and "
                "class '%S'.\n", name, class);

    ajListPop(rci, (void**) Prc);

    while(ajListPop(rci, (void**) &rc))
        ensRepeatconsensusDel(&rc);

    ajListFree(&rci);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @datasection [EnsPRepeatfeature] Ensembl Repeat Feature ********************
**
** @nam2rule Repeatfeature Functions for manipulating
** Ensembl Repeat Feature objects
**
** @cc Bio::EnsEMBL::RepeatFeature
** @cc CVS Revision: 1.27
** @cc CVS Tag: branch-ensembl-62
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
** @valrule * [EnsPRepeatfeature] Ensembl Repeat Feature
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
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNewCpy(const EnsPRepeatfeature rf)
{
    EnsPRepeatfeature pthis = NULL;

    if(!rf)
        return NULL;

    AJNEW0(pthis);

    pthis->Adaptor = rf->Adaptor;

    pthis->Identifier = rf->Identifier;

    pthis->Feature = ensFeatureNewRef(rf->Feature);

    pthis->Repeatconsensus = ensRepeatconsensusNewRef(rf->Repeatconsensus);

    pthis->HitStart = rf->HitStart;

    pthis->HitEnd = rf->HitEnd;

    pthis->Score = rf->Score;

    pthis->Use = 1;

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
** @return [EnsPRepeatfeature] Ensembl Repeat Feature
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

    if(!feature)
        return NULL;

    AJNEW0(rf);

    rf->Adaptor = rfa;

    rf->Identifier = identifier;

    rf->Feature = ensFeatureNewRef(feature);

    rf->Repeatconsensus = ensRepeatconsensusNewRef(rc);

    rf->HitStart = hstart;

    rf->HitEnd = hend;

    rf->Score = score;

    rf->Use = 1;

    return rf;
}




/* @func ensRepeatfeatureNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatfeature] Ensembl Repeat Feature
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNewRef(EnsPRepeatfeature rf)
{
    if(!rf)
        return NULL;

    rf->Use++;

    return rf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Feature object
**
** @argrule * Prf [EnsPRepeatfeature*] Ensembl Repeat Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensRepeatfeatureDel **************************************************
**
** Default destructor for an Ensembl Repeat Feature.
**
** @param [d] Prf [EnsPRepeatfeature*] Ensembl Repeat Feature object address
**
** @return [void]
** @@
******************************************************************************/

void ensRepeatfeatureDel(EnsPRepeatfeature* Prf)
{
    EnsPRepeatfeature pthis = NULL;

    if(!Prf)
        return;

    if(!*Prf)
        return;

    if(ajDebugTest("ensRepeatfeatureDel"))
        ajDebug("ensRepeatfeatureDel\n"
                "  *Prf %p\n",
                *Prf);

    pthis = *Prf;

    pthis->Use--;

    if(pthis->Use)
    {
        *Prf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensRepeatconsensusDel(&pthis->Repeatconsensus);

    AJFREE(pthis);

    *Prf = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Repeat Feature object.
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
** @valrule Adaptor [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule HitEnd [ajint] Hit end
** @valrule HitStart [ajint] Hit start
** @valrule HitStrand [ajint] Hit strand
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Repeatconsensus [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @valrule Score [double] Score
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatfeatureGetAdaptor *******************************************
**
** Get the Object Adaptor element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @@
******************************************************************************/

EnsPRepeatfeatureadaptor ensRepeatfeatureGetAdaptor(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return NULL;

    return rf->Adaptor;
}




/* @func ensRepeatfeatureGetFeature *******************************************
**
** Get the Ensembl Feature element of an Ensembl Repeat Feature.
**
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensRepeatfeatureGetFeature(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return NULL;

    return rf->Feature;
}




/* @func ensRepeatfeatureGetHitEnd ********************************************
**
** Get the hit end element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hend
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajint] Hit end coordinate
** @@
******************************************************************************/

ajint ensRepeatfeatureGetHitEnd(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->HitEnd;
}




/* @func ensRepeatfeatureGetHitStart ******************************************
**
** Get the hit start element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hstart
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajint] Hit start coordinate
** @@
******************************************************************************/

ajint ensRepeatfeatureGetHitStart(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->HitStart;
}




/* @func ensRepeatfeatureGetHitStrand *****************************************
**
** Get the hit strand element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hstrand
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajint] Hit strand
** @@
******************************************************************************/

ajint ensRepeatfeatureGetHitStrand(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return 1;
}




/* @func ensRepeatfeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensRepeatfeatureGetIdentifier(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->Identifier;
}




/* @func ensRepeatfeatureGetRepeatconsensus ***********************************
**
** Get the Ensembl Repeat Consensus element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::repeat_consensus
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatfeatureGetRepeatconsensus(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return NULL;

    return rf->Repeatconsensus;
}




/* @func ensRepeatfeatureGetScore *********************************************
**
** Get the score element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::score
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [double] Score
** @@
******************************************************************************/

double ensRepeatfeatureGetScore(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->Score;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Set Set one element of a Repeat Feature
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
** Set the Ensembl Database Adaptor element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetAdaptor(EnsPRepeatfeature rf,
                                  EnsPRepeatfeatureadaptor rfa)
{
    if(!rf)
        return ajFalse;

    rf->Adaptor = rfa;

    return ajTrue;
}




/* @func ensRepeatfeatureSetFeature *******************************************
**
** Set the Ensembl Feature element of an Ensembl Repeat Feature.
**
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetFeature(EnsPRepeatfeature rf,
                                  EnsPFeature feature)
{
    if(!rf)
        return ajFalse;

    ensFeatureDel(&rf->Feature);

    rf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensRepeatfeatureSetHitEnd ********************************************
**
** Set the hit end element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hend
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] hend [ajuint] Hit end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetHitEnd(EnsPRepeatfeature rf,
                                 ajuint hend)
{
    if(!rf)
        return ajFalse;

    rf->HitEnd = hend;

    return ajTrue;
}




/* @func ensRepeatfeatureSetHitStart ******************************************
**
** Set the hit start element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::hstart
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] hstart [ajuint] Hit start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetHitStart(EnsPRepeatfeature rf,
                                   ajuint hstart)
{
    if(!rf)
        return ajFalse;

    rf->HitStart = hstart;

    return ajTrue;
}




/* @func ensRepeatfeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetIdentifier(EnsPRepeatfeature rf,
                                     ajuint identifier)
{
    if(!rf)
        return ajFalse;

    rf->Identifier = identifier;

    return ajTrue;
}




/* @func ensRepeatfeatureSetRepeatconsensus ***********************************
**
** Set the Ensembl Repeat Consensus element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::repeat_consensus
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetRepeatconsensus(EnsPRepeatfeature rf,
                                          EnsPRepeatconsensus rc)
{
    if(!rf)
        return ajFalse;

    ensRepeatconsensusDel(&rf->Repeatconsensus);

    rf->Repeatconsensus = ensRepeatconsensusNewRef(rc);

    return ajTrue;
}




/* @func ensRepeatfeatureSetScore *********************************************
**
** Set the score element of an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::score
** @param [u] rf [EnsPRepeatfeature] Ensembl Repeat Feature
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSetScore(EnsPRepeatfeature rf,
                                double score)
{
    if(!rf)
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
** @nam3rule Trace Report Ensembl Repeat Feature elements to debug file
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
** @@
******************************************************************************/

AjBool ensRepeatfeatureTrace(const EnsPRepeatfeature rf, ajuint level)
{
    AjPStr indent = NULL;

    if(!rf)
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
** Functions for calculating values of an Ensembl Repeat Feature object.
**
** @fdata [EnsPRepeatfeature]
**
** @nam3rule Calculate Calculate Ensembl Repeat Feature values
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
** @@
******************************************************************************/

size_t ensRepeatfeatureCalculateMemsize(const EnsPRepeatfeature rf)
{
    size_t size = 0;

    if(!rf)
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




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Repeatfeature Functions for manipulating AJAX List objects of
** Ensembl Repeat Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending rfs [AjPList] AJAX List of
**                                  Ensembl Repeat Feature objects
** @argrule Descending rfs [AjPList] AJAX List of
**                                   Ensembl Repeat Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listRepeatfeatureCompareStartAscending *************************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** Ensembl Feature start element in ascending order.
**
** @param [r] P1 [const void*] Ensembl Repeat Feature address 1
** @param [r] P2 [const void*] Ensembl Repeat Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listRepeatfeatureCompareStartAscending(const void* P1,
                                                  const void* P2)
{
    const EnsPRepeatfeature rf1 = NULL;
    const EnsPRepeatfeature rf2 = NULL;

    rf1 = *(EnsPRepeatfeature const*) P1;
    rf2 = *(EnsPRepeatfeature const*) P2;

    if(ajDebugTest("listRepeatfeatureCompareStartAscending"))
        ajDebug("listRepeatfeatureCompareStartAscending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(rf1 && (!rf2))
        return -1;

    if((!rf1) && (!rf2))
        return 0;

    if((!rf1) && rf2)
        return +1;

    return ensFeatureCompareStartAscending(rf1->Feature, rf2->Feature);
}




/* @func ensListRepeatfeatureSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** Ensembl Feature start element in ascending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortStartAscending(AjPList rfs)
{
    if(!rfs)
        return ajFalse;

    ajListSort(rfs, listRepeatfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listRepeatfeatureCompareStartDescending ************************
**
** AJAX List of Ensembl Repeat Feature objects comparison function to sort by
** Ensembl Feature start element in descending order.
**
** @param [r] P1 [const void*] Ensembl Repeat Feature address 1
** @param [r] P2 [const void*] Ensembl Repeat Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listRepeatfeatureCompareStartDescending(const void* P1,
                                                   const void* P2)
{
    const EnsPRepeatfeature rf1 = NULL;
    const EnsPRepeatfeature rf2 = NULL;

    rf1 = *(EnsPRepeatfeature const*) P1;
    rf2 = *(EnsPRepeatfeature const*) P2;

    if(ajDebugTest("listRepeatfeatureCompareStartDescending"))
        ajDebug("listRepeatfeatureCompareStartDescending\n"
                "  rf1 %p\n"
                "  rf2 %p\n",
                rf1,
                rf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(rf1 && (!rf2))
        return -1;

    if((!rf1) && (!rf2))
        return 0;

    if((!rf1) && rf2)
        return +1;

    return ensFeatureCompareStartDescending(rf1->Feature, rf2->Feature);
}




/* @func ensListRepeatfeatureSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Repeat Feature objects by their
** Ensembl Feature start element in descending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListRepeatfeatureSortStartDescending(AjPList rfs)
{
    if(!rfs)
        return ajFalse;

    ajListSort(rfs, listRepeatfeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor *****
**
** @nam2rule Repeatfeatureadaptor Functions for manipulating
** Ensembl Repeat Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor
** @cc CVS Revision: 1.59
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic repeatfeatureadaptorCacheReference *****************************
**
** Wrapper function to reference an Ensembl Repeat Feature
** from an Ensembl Cache.
**
** @param [u] value [void*] Ensembl Repeat Feature
**
** @return [void*] Ensembl Repeat Feature or NULL
** @@
******************************************************************************/

static void* repeatfeatureadaptorCacheReference(void*value)
{
    if(ajDebugTest("repeatfeatureadaptorCacheReference"))
        ajDebug("repeatfeatureadaptorCacheReference\n"
                "  value %p\n",
                value);

    if(!value)
        return NULL;

    return (void*) ensRepeatfeatureNewRef((EnsPRepeatfeature) value);
}




/* @funcstatic repeatfeatureadaptorCacheDelete ********************************
**
** Wrapper function to delete an Ensembl Repeat Feature from an Ensembl Cache.
**
** @param [u] value [void**] Ensembl Repeat Feature address
**
** @return [void]
** @@
******************************************************************************/

static void repeatfeatureadaptorCacheDelete(void** value)
{
    if(ajDebugTest("repeatfeatureadaptorCacheDelete"))
        ajDebug("repeatfeatureadaptorCacheDelete\n"
                "  value %p\n",
                value);

    if(!value)
        return;

    ensRepeatfeatureDel((EnsPRepeatfeature*) value);

    return;
}




/* @funcstatic repeatfeatureadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Repeat Feature
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Repeat Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t repeatfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensRepeatfeatureCalculateMemsize((const EnsPRepeatfeature) value);
}




/* @funcstatic repeatfeatureadaptorGetFeature *********************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Repeat Feature from an Ensembl Feature Adaptor.
**
** @param [r] rf [const void*] Ensembl Repeat Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature repeatfeatureadaptorGetFeature(const void* rf)
{
    if(!rf)
        return 0;

    return ensRepeatfeatureGetFeature((const EnsPRepeatfeature) rf);
}




/* @funcstatic repeatfeatureadaptorLinkRepeatconsensus ************************
**
** An ajTableMapDel "apply" function to link Ensembl Repeat Feature objects to
** Ensembl Repeat Consensus objects.
** This function also deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Repeat Feature objects after association.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX Lists of Ensembl Repeat Feature objects
** @param [u] cl [void*] AJAX Table of Ensembl Repeat Consensus objects,
**                       passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void repeatfeatureadaptorLinkRepeatconsensus(void** key,
                                                    void** value,
                                                    void* cl)
{
    EnsPRepeatconsensus rc = NULL;

    EnsPRepeatfeature rf = NULL;

    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    rc = (EnsPRepeatconsensus) ajTableFetchmodV(cl, *key);

    /*
    ** The Ensembl Repeat Feature objects can be deleted after associating
    ** them with Ensembl Repeat Consensus objects, because this AJAX Table
    ** holds independent references for these objects.
    */

    while(ajListPop(*((AjPList*) value), (void**) &rf))
    {
        ensRepeatfeatureSetRepeatconsensus(rf, rc);

        ensRepeatfeatureDel(&rf);
    }

    AJFREE(*key);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool repeatfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList rfs)
{
    ajuint* Pidentifier = NULL;

    ajuint anid  = 0;
    ajuint rcid  = 0;
    ajuint rfid  = 0;
    ajuint srid  = 0;

    ajuint rpstart = 0;
    ajuint rpend   = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    double score = 0;

    AjBool debug = AJFALSE;

    AjPList list = NULL;
    AjPList mrs  = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPTable rci     = NULL;
    AjPTable rcitorf = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPRepeatconsensusadaptor rca = NULL;

    EnsPRepeatfeature        rf  = NULL;
    EnsPRepeatfeatureadaptor rfa = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    debug = ajDebugTest("repeatfeatureadaptorFetchAllbyStatement");

    if(debug)
        ajDebug("repeatfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  rfs %p\n",
                dba,
                statement,
                am,
                slice,
                rfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!rfs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    rca = ensRegistryGetRepeatconsensusadaptor(dba);

    rfa = ensRegistryGetRepeatfeatureadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    rci     = ensTableuintNewLen(0);
    rcitorf = ensTableuintNewLen(0);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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
            ajFatal("repeatfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("repeatfeatureadaptorFetchAllbyStatement got a "
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
                if(debug)
                {
                    ajDebug("repeatfeatureadaptorFetchAllbyStatement mapped "
                            "Repeat Feature %u on Sequence Region %u "
                            "start %u end %u strand %d to gap.\n",
                            rfid,
                            srid,
                            srstart,
                            srend,
                            srstrand);

                    ensMapperresultTrace(mr, 1);
                }

                ensSliceDel(&srslice);

                ensAnalysisDel(&analysis);

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
                ajFatal("repeatfeatureadaptorFetchAllbyStatement got a "
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
                    slend   = slend   - ensSliceGetStart(slice) + 1;
                }
                else
                {
                    slend     = ensSliceGetEnd(slice) - slstart + 1;
                    slstart   = ensSliceGetEnd(slice) - slend   + 1;
                    slstrand *= -1;
                }
            }

            /*
            ** Throw away Ensembl Feature objects off the end of the
            ** requested Slice.
            */

            if((slstart > sllength) ||
               (slend < 1) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                ensSliceDel(&srslice);

                ensAnalysisDel(&analysis);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        /* Finally, create a new Ensembl Repeat Feature. */

        ensAnalysisadaptorFetchByIdentifier(aa, anid, &analysis);

        feature = ensFeatureNewIniS(analysis,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

        rf = ensRepeatfeatureNewIni(rfa,
                                    rfid,
                                    feature,
                                    (EnsPRepeatconsensus) NULL,
                                    rpstart,
                                    rpend,
                                    score);

        ajListPushAppend(rfs, (void*) rf);

        /*
        ** Populate two AJAX Table objects to fetch Ensembl Repeat Consensus
        ** objects from the database and associate them with
        ** Ensembl Repeat Feature objects.
        **
        ** rci
        **   key data:   Ensembl Repeat Consensus identifiers
        **   value data: Ensembl Repeat Consensus objects fetched by
        **               ensRepeatconsensusadaptorFetchAllbyIdentifiers
        **
        ** rcitorf
        **   key data:   Ensembl Repeat Consensus identifiers
        **   value data: AJAX List objects of Ensembl Repeat Feature objects
        **               that need to be associated with Ensembl Repeat
        **               Consensus objects once they have been fetched
        **               from the database
        */

        if(!ajTableMatchV(rci, (const void*) &rcid))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = rcid;

            ajTablePut(rci, (void*) Pidentifier, (void*) NULL);
        }

        list = (AjPList) ajTableFetchmodV(rcitorf, (const void*) &rcid);

        if(!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = rcid;

            list = ajListNew();

            ajTablePut(rcitorf,
                       (void*) Pidentifier,
                       (void*) list);
        }

        ajListPushAppend(list, (void*) ensRepeatfeatureNewRef(rf));

        ensFeatureDel(&feature);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);

        ensAnalysisDel(&analysis);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    ensRepeatconsensusadaptorFetchAllbyIdentifiers(rca, rci);

    /*
    ** Associate
    ** Ensembl Repeat Consensus objects with
    ** Ensembl Repeat Feature objects.
    */

    ajTableMapDel(rcitorf,
                  repeatfeatureadaptorLinkRepeatconsensus,
                  (void*) rci);

    ajTableFree(&rcitorf);

    /* Delete the utility AJAX Table objects. */

    ensTableRepeatconsensusDelete(&rci);

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
** @valrule * [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
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
** @@
******************************************************************************/

EnsPRepeatfeatureadaptor ensRepeatfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        repeatfeatureadaptorTables,
        repeatfeatureadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        repeatfeatureadaptorDefaultcondition,
        (const char*) NULL,
        repeatfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        repeatfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        repeatfeatureadaptorCacheDelete,
        repeatfeatureadaptorCacheSize,
        repeatfeatureadaptorGetFeature,
        "Repeatfeature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Repeat Feature Adaptor object.
**
** @fdata [EnsPRepeatfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Repeat Feature Adaptor object
**
** @argrule * Prfa [EnsPRepeatfeatureadaptor*]
** Ensembl Repeat Feature Adaptor object address
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
** Ensembl Repeat Feature Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensRepeatfeatureadaptorDel(EnsPRepeatfeatureadaptor* Prfa)
{
    if(!Prfa)
        return;

    if(!*Prfa)
        return;

    ensFeatureadaptorDel(Prfa);

    *Prfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Repeat Feature Adaptor
** object.
**
** @fdata [EnsPRepeatfeatureadaptor]
**
** @nam3rule Get Return Repeat Feature Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatfeatureadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Repeat Feature Adaptor.
**
** @param [u] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensRepeatfeatureadaptorGetDatabaseadaptor(
    EnsPRepeatfeatureadaptor rfa)
{
    if(!rfa)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(rfa);
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
    char* txtrcclass = NULL;
    char* txtrcname = NULL;
    char* txtrctype = NULL;

    AjPStr constraint = NULL;

    if(ajDebugTest("ensRepeatfeatureadaptorFetchAllbySlice"))
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

    if(!rfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!rfs)
        return ajFalse;

    if(rctype && ajStrGetLen(rctype))
    {
        ensFeatureadaptorEscapeC(rfa, &txtrctype, rctype);

        constraint = ajFmtStr("repeat_consensus.repeat_type = '%s'",
                              txtrctype);

        ajCharDel(&txtrctype);
    }

    if(rcclass && ajStrGetLen(rcclass))
    {
        ensFeatureadaptorEscapeC(rfa, &txtrcclass, rcclass);

        if(constraint)
            ajFmtPrintAppS(&constraint,
                           " AND repeat_consensus.repeat_class = '%s'",
                           txtrcclass);
        else
            constraint = ajFmtStr("repeat_consensus.repeat_class = '%s'",
                                  txtrcclass);

        ajCharDel(&txtrcclass);
    }

    if(rcname && ajStrGetLen(rcname))
    {
        ensFeatureadaptorEscapeC(rfa, &txtrcname, rcname);

        if(constraint)
            ajFmtPrintAppS(&constraint,
                           " AND repeat_consensus.repeat_name = '%s'",
                           txtrcname);
        else
            constraint = ajFmtStr("repeat_consensus.repeat_name = '%s'",
                                  txtrcname);

        ajCharDel(&txtrcname);
    }

    ensFeatureadaptorFetchAllbySlice(rfa,
                                     slice,
                                     constraint,
                                     anname,
                                     rfs);

    ajStrDel(&constraint);

    return ajTrue;
}

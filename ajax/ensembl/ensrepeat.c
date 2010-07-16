/******************************************************************************
** @source Ensembl Repeat functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.13 $
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




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool repeatconsensusadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement,
                                                  AjPList rci);

static int repeatfeatureCompareStartAscending(const void* P1, const void* P2);

static int repeatfeatureCompareStartDescending(const void* P1, const void* P2);

static void *repeatfeatureadaptorCacheReference(void *value);

static void repeatfeatureadaptorCacheDelete(void **value);

static ajulong repeatfeatureadaptorCacheSize(const void *value);

static EnsPFeature repeatfeatureadaptorGetFeature(const void *rf);

static AjBool repeatfeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                const AjPStr statement,
                                                EnsPAssemblymapper am,
                                                EnsPSlice slice,
                                                AjPList rfs);




/* @filesection ensrepeat *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPRepeatconsensus] Repeat Consensus ************************
**
** Functions for manipulating Ensembl Repeat Consensus objects
**
** @cc Bio::EnsEMBL::RepeatConsensus CVS Revision: 1.9
**
** @nam2rule Repeatconsensus
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @argrule Ref object [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @valrule * [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatconsensusNew ************************************************
**
** Default constructor for an Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::RepeatConsensus::new
** @param [r] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] identifier [ajuint] SQL databaseinternal identifier
** @param [r] name [AjPStr] Repeat Consensus name
** @param [r] class [AjPStr] Repeat Consensus class
** @param [r] type [AjPStr] Repeat Consensus type
** @param [r] consensus [AjPStr] Repeat Consensus sequence
** @param [r] length [ajuint] Repeat Consensus sequence length
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNew(EnsPRepeatconsensusadaptor rca,
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




/* @func ensRepeatconsensusNewObj *********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [EnsPRepeatconsensus] Ensembl Repeat Consensus or NULL
** @@
******************************************************************************/

EnsPRepeatconsensus ensRepeatconsensusNewObj(EnsPRepeatconsensus object)
{
    EnsPRepeatconsensus rc = NULL;

    AJNEW0(rc);

    rc->Adaptor = object->Adaptor;

    rc->Identifier = object->Identifier;

    if(object->Name)
        rc->Name = ajStrNewRef(object->Name);

    if(object->Class)
        rc->Class = ajStrNewRef(object->Class);

    if(object->Type)
        rc->Type = ajStrNewRef(object->Type);

    if(object->Consensus)
        rc->Consensus = ajStrNewRef(object->Consensus);

    rc->Length = object->Length;

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
** memory allocated for the Ensembl Repeat Consensus.
**
** @fdata [EnsPRepeatconsensus]
** @fnote None
**
** @nam3rule Del Destroy (free) a Repeat Consensus object
**
** @argrule * Prc [EnsPRepeatconsensus*] Repeat Consensus object address
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
** @@
******************************************************************************/

void ensRepeatconsensusDel(EnsPRepeatconsensus *Prc)
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
** @fnote None
**
** @nam3rule Get Return Repeat Consensus attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Repeat Consensus Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetClass Return the class
** @nam4rule GetType Return the type
** @nam4rule GetConsensus Return the consensus sequence
** @nam4rule GetLength Return the length
**
** @argrule * rc [const EnsPRepeatconsensus] Repeat Consensus
**
** @valrule Adaptor [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus
**                                               Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Class [AjPStr] Class
** @valrule Type [AjPStr] Type
** @valrule Consensus [AjPStr] Consensus sequence
** @valrule Length [ajuint] Length
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




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
** @fnote None
**
** @nam3rule Set Set one element of a Repeat Consensus
** @nam4rule SetAdaptor Set the Ensembl Repeat Consensus Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetClass Set the class
** @nam4rule SetType Set the type
** @nam4rule SetConsensus Set the consensus sequence
** @nam4rule SetType Set the type
**
** @argrule * rc [EnsPRepeatconsensus] Ensembl Repeat Consensus object
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
** @param [r] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
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

AjBool ensRepeatconsensusSetName(EnsPRepeatconsensus rc, AjPStr name)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Name);

    if(name)
        rc->Name = ajStrNewRef(name);

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

AjBool ensRepeatconsensusSetClass(EnsPRepeatconsensus rc, AjPStr class)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Class);

    if(class)
        rc->Class = ajStrNewRef(class);

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

AjBool ensRepeatconsensusSetType(EnsPRepeatconsensus rc, AjPStr type)
{
    if(!rc)
        return ajFalse;

    ajStrDel(&rc->Type);

    if(type)
        rc->Type = ajStrNewRef(type);

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

AjBool ensRepeatconsensusSetLength(EnsPRepeatconsensus rc, ajuint length)
{
    if(!rc)
        return ajFalse;

    rc->Length = length;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Repeat Consensus object.
**
** @fdata [EnsPRepeatconsensus]
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




/* @func ensRepeatconsensusGetMemsize *****************************************
**
** Get the memory size in bytes of an Ensembl Repeat Consensus.
**
** @param [r] rc [const EnsPRepeatconsensus] Ensembl Repeat Consensus
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensRepeatconsensusGetMemsize(const EnsPRepeatconsensus rc)
{
    ajulong size = 0;

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




/* @datasection [EnsPRepeatconsensusadaptor] Repeat Consensus Adaptor *********
**
** Functions for manipulating Ensembl Repeat Consensus Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor CVS Revision: 1.20
**
** @nam2rule Repeatconsensusadaptor
**
******************************************************************************/




/* @funcstatic repeatconsensusadaptorFetchAllBySQL ****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Repeat Consensus objects.
** The caller is responsible for deleting the Ensembl Repeat Consensi before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::_generic_fetch
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] rci [AjPList] AJAX List of Ensembl Repeat Consensi
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool repeatconsensusadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement,
                                                  AjPList rci)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr class     = NULL;
    AjPStr type      = NULL;
    AjPStr consensus = NULL;

    EnsPRepeatconsensus rc         = NULL;
    EnsPRepeatconsensusadaptor rca = NULL;

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!rci)
        return ajFalse;

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

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &class);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToStr(sqlr, &consensus);

        rc = ensRepeatconsensusNew(rca,
                                   identifier,
                                   name,
                                   class,
                                   type,
                                   consensus,
                                   0);

        ajListPushAppend(rci, (void *) rc);

        ajStrDel(&name);
        ajStrDel(&class);
        ajStrDel(&type);
        ajStrDel(&consensus);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @func ensRepeatconsensusadaptorGetAdaptor **********************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Repeat Consensus Adaptor.
**
** @param [u] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
** NOTE: At the moment, the Ensembl Repeat Consensus Adaptor is just an alias
** for the Ensembl Database Adaptor, but eventually an Ensembl Base Adaptor
** could be used.
******************************************************************************/

EnsPDatabaseadaptor ensRepeatconsensusadaptorGetAdaptor(
    EnsPRepeatconsensusadaptor rca)
{
    if(!rca)
        return NULL;

    return rca;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Repeat Consensus objects from an
** Ensembl Core database.
**
** @fdata [EnsPRepeatconsensusadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Repeat Consensus object(s)
** @nam4rule FetchAll Retrieve all Ensembl Repeat Consensus objects
** @nam5rule FetchAllBy Retrieve all Ensembl Repeat Consensus objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Repeat Consensus object
**                   matching a criterion
**
** @argrule * rca [const EnsPRepeatconsensusadaptor] Ensembl Repeat
**                                                   Consensus Adaptor
** @argrule FetchAll rci [AjPList] AJAX List of
**                                 Ensembl Repeat Consensus objects
** @argrule FetchBy Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensRepeatconsensusadaptorFetchByIdentifier ***************************
**
** Fetch an Ensembl Repeat Consensus by its internal SQL database identifier.
** The caller is responsible for deleting the Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_by_dbID
** @param [r] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [w] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByIdentifier(
    EnsPRepeatconsensusadaptor rca,
    ajuint identifier,
    EnsPRepeatconsensus *Prc)
{
    AjPList rci      = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPRepeatconsensus rc = NULL;

    if(!rca)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Prc)
        return ajFalse;

    dba = ensRepeatconsensusadaptorGetAdaptor(rca);

    statement = ajFmtStr(
        "SELECT "
        "repeat_consensus.repeat_consensus_id, "
        "repeat_consensus.repeat_name, "
        "repeat_consensus.repeat_class, "
        "repeat_consensus.repeat_type, "
        "repeat_consensus.repeat_consensus "
        "FROM "
        "repeat_consensus "
        "WHERE "
        "repeat_consensus.repeat_consensus_id = %u",
        identifier);

    rci = ajListNew();

    repeatconsensusadaptorFetchAllBySQL(dba, statement, rci);

    if(ajListGetLength(rci) > 1)
        ajFatal("ensRepeatconsensusadaptorFetchByIdentifier got more than one "
                "Repeat Consensus for (PRIMARY KEY) identifier %u.\n",
                identifier);

    ajListPop(rci, (void **) Prc);

    while(ajListPop(rci, (void **) &rc))
        ensRepeatconsensusDel(&rc);

    ajListFree(&rci);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensRepeatconsensusadaptorFetchByName *********************************
**
** Fetch an Ensembl Repeat Consensus by its name.
** The caller is responsible for deleting the Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_by_name
** @param [r] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] name [const AjPStr] Name
** @param [w] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByName(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr name,
    EnsPRepeatconsensus *Prc)
{
    char *txtname;

    AjPList rci = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPRepeatconsensus rc = NULL;

    if(!dba)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Prc)
        return ajFalse;

    dba = ensRepeatconsensusadaptorGetAdaptor(rca);

    ensDatabaseadaptorEscapeC(dba, &txtname, name);

    statement = ajFmtStr(
        "SELECT "
        "repeat_consensus.repeat_consensus_id, "
        "repeat_consensus.repeat_name, "
        "repeat_consensus.repeat_class, "
        "repeat_consensus.repeat_type, "
        "repeat_consensus.repeat_consensus "
        "FROM "
        "repeat_consensus "
        "WHERE "
        "repeat_consensus.repeat_name = '%s'",
        txtname);

    ajCharDel(&txtname);

    rci = ajListNew();

    repeatconsensusadaptorFetchAllBySQL(dba, statement, rci);

    if(ajListGetLength(rci) > 1)
        ajFatal("ensRepeatconsensusadaptorFetchByName got more than one "
                "Repeat Consensus from the database for name '%S'.\n", name);

    ajListPop(rci, (void **) Prc);

    while(ajListPop(rci, (void **) &rc))
        ensRepeatconsensusDel(&rc);

    ajListFree(&rci);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensRepeatconsensusadaptorFetchByNameClass ****************************
**
** Fetch an Ensembl Repeat Consensus by its name and class.
** The caller is responsible for deleting the Ensembl Repeat Consensus.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_by_name_class
** @param [r] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] name [const AjPStr] Name
** @param [r] class [const AjPStr] Class
** @param [w] Prc [EnsPRepeatconsensus*] Ensembl Repeat Consensus address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchByNameClass(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr name,
    const AjPStr class,
    EnsPRepeatconsensus *Prc)
{
    char *txtname  = NULL;
    char *txtclass = NULL;

    AjPList rci = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPRepeatconsensus rc = NULL;

    if(!dba)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!class)
        return ajFalse;

    if(!Prc)
        return ajFalse;

    dba = ensRepeatconsensusadaptorGetAdaptor(rca);

    ensDatabaseadaptorEscapeC(dba, &txtname, name);

    ensDatabaseadaptorEscapeC(dba, &txtclass, class);

    statement = ajFmtStr(
        "SELECT "
        "repeat_consensus.repeat_consensus_id, "
        "repeat_consensus.repeat_name, "
        "repeat_consensus.repeat_class, "
        "repeat_consensus.repeat_type, "
        "repeat_consensus.repeat_consensus "
        "FROM "
        "repeat_consensus "
        "WHERE "
        "repeat_consensus.repeat_name = '%s' "
        "AND "
        "repeat_consensus.repeat_class = '%s'",
        txtname,
        txtclass);

    ajCharDel(&txtname);

    ajCharDel(&txtclass);

    rci = ajListNew();

    repeatconsensusadaptorFetchAllBySQL(dba, statement, rci);

    if(ajListGetLength(rci) > 1)
        ajFatal("ensRepeatconsensusadaptorFetchByNameClass got more than one "
                "Repeat Consensus from the database for name '%S' and "
                "class '%S'.\n", name, class);

    ajListPop(rci, (void **) Prc);

    while(ajListPop(rci, (void **) &rc))
        ensRepeatconsensusDel(&rc);

    ajListFree(&rci);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensRepeatconsensusadaptorFetchAllByClassConsensus ********************
**
** Fetch all Ensembl Repeat Consensi by their class and
** consensus sequence.
** The caller is responsible for deleting the Ensembl Repeat Consensi before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatConsensusAdaptor::fetch_all_by_class_seq
** @param [r] rca [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @param [r] class [const AjPStr] Class
** @param [r] consensus [const AjPStr] Consensus sequence
** @param [u] rci [AjPList] AJAX List of Ensembl Repeat Consensi
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatconsensusadaptorFetchAllByClassConsensus(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr class,
    const AjPStr consensus,
    AjPList rci)
{
    char *txtclass     = NULL;
    char *txtconsensus = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!dba)
        return ajFalse;

    if(!class)
        return ajFalse;

    if(!consensus)
        return ajFalse;

    if(!rci)
        return ajFalse;

    dba = ensRepeatconsensusadaptorGetAdaptor(rca);

    ensDatabaseadaptorEscapeC(dba, &txtclass, class);
    ensDatabaseadaptorEscapeC(dba, &txtconsensus, consensus);

    statement = ajFmtStr(
        "SELECT "
        "repeat_consensus.repeat_consensus_id, "
        "repeat_consensus.repeat_name, "
        "repeat_consensus.repeat_class, "
        "repeat_consensus.repeat_type, "
        "repeat_consensus.repeat_consensus "
        "FROM "
        "repeat_consensus "
        "WHERE "
        "repeat_consensus.repeat_class = '%s' "
        "AND "
        "repeat_consensus.repeat_consensus = '%s'",
        txtclass,
        txtconsensus);

    ajCharDel(&txtclass);
    ajCharDel(&txtconsensus);

    repeatconsensusadaptorFetchAllBySQL(dba, statement, rci);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPRepeatfeature] Repeat Feature ****************************
**
** Functions for manipulating Ensembl Repeat Feature objects
**
** @cc Bio::EnsEMBL::RepeatFeature CVS Revision: 1.24
**
** @nam2rule Repeatfeature
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPRepeatfeature] Ensembl Repeat Feature
** @argrule Ref object [EnsPRepeatfeature] Ensembl Repeat Feature
**
** @valrule * [EnsPRepeatfeature] Ensembl Repeat Feature
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatfeatureNew **************************************************
**
** Default constructor for an Ensembl Repeat Feature.
**
** @cc Bio::EnsEMBL::RepeatFeature::new
** @param [r] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] rc [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @param [r] hstart [ajint] Repeat Consensus hit start
** @param [r] hend [ajint] Repeat Consensus hit end
** @param [r] score [double] Repeat Consensus score
**
** @return [EnsPRepeatfeature] Ensembl Repeat Feature
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNew(EnsPRepeatfeatureadaptor rfa,
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




/* @func ensRepeatfeatureNewObj ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [EnsPRepeatfeature] Ensembl Repeat Feature or NULL
** @@
******************************************************************************/

EnsPRepeatfeature ensRepeatfeatureNewObj(EnsPRepeatfeature object)
{
    EnsPRepeatfeature rf = NULL;

    AJNEW0(rf);

    rf->Adaptor = object->Adaptor;

    rf->Identifier = object->Identifier;

    rf->Feature = ensFeatureNewRef(object->Feature);

    rf->Repeatconsensus = ensRepeatconsensusNewRef(object->Repeatconsensus);

    rf->HitStart = object->HitStart;

    rf->HitEnd = object->HitEnd;

    rf->Score = object->Score;

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
** memory allocated for the Ensembl Repeat Features.
**
** @fdata [EnsPRepeatfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Repeat Feature object
**
** @argrule * Prf [EnsPRepeatfeature*] Repeat Feature object address
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
** @@
******************************************************************************/

void ensRepeatfeatureDel(EnsPRepeatfeature *Prf)
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
** @fnote None
**
** @nam3rule Get Return Repeat Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Repeat Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetRepeatconsensus Return the Ensembl Repeat Consensus
** @nam4rule GetHitStart Return the hit start
** @nam4rule GetHitEnd Return the hit end
** @nam4rule GetHitStrand Return the hit strand
** @nam4rule GetScore Return the score
**
** @argrule * rf [const EnsPRepeatfeature] Repeat Feature
**
** @valrule Adaptor [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule Repeatconsensus [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @valrule HitStart [ajint] Hit start
** @valrule HitEnd [ajint] Hit end
** @valrule HitStrand [ajint] Hit strand
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
** @return [const EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @@
******************************************************************************/

const EnsPRepeatfeatureadaptor ensRepeatfeatureGetAdaptor(
    const EnsPRepeatfeature rf)
{
    if(!rf)
        return NULL;

    return rf->Adaptor;
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

ajuint ensRepeatfeatureGetIdentifier(const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->Identifier;
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

EnsPFeature ensRepeatfeatureGetFeature(const EnsPRepeatfeature rf)
{
    if(!rf)
        return NULL;

    return rf->Feature;
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

ajint ensRepeatfeatureGetHitStart(const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->HitStart;
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

ajint ensRepeatfeatureGetHitEnd(const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return rf->HitEnd;
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

ajint ensRepeatfeatureGetHitStrand(const EnsPRepeatfeature rf)
{
    if(!rf)
        return 0;

    return 1;
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

double ensRepeatfeatureGetScore(const EnsPRepeatfeature rf)
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
** @fnote None
**
** @nam3rule Set Set one element of a Repeat Feature
** @nam4rule SetAdaptor Set the Ensembl Repeat Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetRepeatconsensus Set the Ensembl Repeat Consensus
** @nam4rule SetHitStart Set the hit start
** @nam4rule SetHitEnd Set the hit end
** @nam4rule SetScore Set the score
**
** @argrule * rf [EnsPRepeatfeature] Ensembl Repeat Feature object
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
** @param [r] rfa [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
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

AjBool ensRepeatfeatureSetIdentifier(EnsPRepeatfeature rf, ajuint identifier)
{
    if(!rf)
        return ajFalse;

    rf->Identifier = identifier;

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

AjBool ensRepeatfeatureSetFeature(EnsPRepeatfeature rf, EnsPFeature feature)
{
    if(!rf)
        return ajFalse;

    ensFeatureDel(&rf->Feature);

    rf->Feature = ensFeatureNewRef(feature);

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

AjBool ensRepeatfeatureSetHitStart(EnsPRepeatfeature rf, ajuint hstart)
{
    if(!rf)
        return ajFalse;

    rf->HitStart = hstart;

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

AjBool ensRepeatfeatureSetHitEnd(EnsPRepeatfeature rf, ajuint hend)
{
    if(!rf)
        return ajFalse;

    rf->HitEnd = hend;

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

AjBool ensRepeatfeatureSetScore(EnsPRepeatfeature rf, double score)
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




/* @func ensRepeatfeatureGetMemsize *******************************************
**
** Get the memory size in bytes of an Ensembl Repeat Feature.
**
** @param [r] rf [const EnsPRepeatfeature] Ensembl Repeat Feature
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensRepeatfeatureGetMemsize(const EnsPRepeatfeature rf)
{
    ajulong size = 0;

    if(!rf)
        return 0;

    size += sizeof (EnsORepeatfeature);

    size += ensFeatureGetMemsize(rf->Feature);

    size += ensRepeatconsensusGetMemsize(rf->Repeatconsensus);

    return size;
}




/* @funcstatic repeatfeatureCompareStartAscending *****************************
**
** Comparison function to sort Ensembl Repeat Features by their
** Ensembl Feature start coordinate in ascending order.
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

static int repeatfeatureCompareStartAscending(const void* P1, const void* P2)
{
    const EnsPRepeatfeature rf1 = NULL;
    const EnsPRepeatfeature rf2 = NULL;

    rf1 = *(EnsPRepeatfeature const *) P1;
    rf2 = *(EnsPRepeatfeature const *) P2;

    if(ajDebugTest("repeatfeatureCompareStartAscending"))
        ajDebug("repeatfeatureCompareStartAscending\n"
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




/* @func ensRepeatfeatureSortByStartAscending *********************************
**
** Sort Ensembl Repeat Features by their Ensembl Feature start coordinate
** in ascending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSortByStartAscending(AjPList rfs)
{
    if(!rfs)
        return ajFalse;

    ajListSort(rfs, repeatfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic repeatfeatureCompareStartDescending ****************************
**
** Comparison function to sort Ensembl Repeat Features by their
** Ensembl Feature start coordinate in descending order.
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

static int repeatfeatureCompareStartDescending(const void* P1, const void* P2)
{
    const EnsPRepeatfeature rf1 = NULL;
    const EnsPRepeatfeature rf2 = NULL;

    rf1 = *(EnsPRepeatfeature const *) P1;
    rf2 = *(EnsPRepeatfeature const *) P2;

    if(ajDebugTest("repeatfeatureCompareStartDescending"))
        ajDebug("repeatfeatureCompareStartDescending\n"
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




/* @func ensRepeatfeatureSortByStartDescending ********************************
**
** Sort Ensembl Repeat Features by their Ensembl Feature start coordinate
** in descending order.
**
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureSortByStartDescending(AjPList rfs)
{
    if(!rfs)
        return ajFalse;

    ajListSort(rfs, repeatfeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPRepeatfeatureadaptor] Repeat Feature Adaptor *************
**
** Functions for manipulating Ensembl Repeat Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor CVS Revision: 1.55
**
** @nam2rule Repeatfeatureadaptor
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

static void* repeatfeatureadaptorCacheReference(void *value)
{
    if(ajDebugTest("repeatfeatureadaptorCacheReference"))
        ajDebug("repeatfeatureadaptorCacheReference\n"
                "  value %p\n",
                value);

    if(!value)
        return NULL;

    return (void *) ensRepeatfeatureNewRef((EnsPRepeatfeature) value);
}




/* @funcstatic repeatfeatureadaptorCacheDelete ********************************
**
** Wrapper function to delete or de-reference an Ensembl Repeat Feature
** from an Ensembl Cache.
**
** @param [u] value [void**] Ensembl Repeat Feature address
**
** @return [void]
** @@
******************************************************************************/

static void repeatfeatureadaptorCacheDelete(void **value)
{
    if(ajDebugTest("repeatfeatureadaptorCacheDelete"))
        ajDebug("repeatfeatureadaptorCacheDelete\n"
                "  value %p\n",
                value);

    if(!value)
        return;

    ensRepeatfeatureDel((EnsPRepeatfeature *) value);

    return;
}




/* @funcstatic repeatfeatureadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Repeat Feature
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Repeat Feature
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

static ajulong repeatfeatureadaptorCacheSize(const void *value)
{
    if(!value)
        return 0;

    return ensRepeatfeatureGetMemsize((const EnsPRepeatfeature) value);
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

static EnsPFeature repeatfeatureadaptorGetFeature(const void *rf)
{
    if(!rf)
        return 0;

    return ensRepeatfeatureGetFeature((const EnsPRepeatfeature) rf);
}




static const char *repeatfeatureadaptorTables[] =
{
    "repeat_feature",
    "repeat_consensus",
    NULL
};




static const char *repeatfeatureadaptorColumns[] =
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
    "repeat_consensus.repeat_name",
    "repeat_consensus.repeat_class",
    "repeat_consensus.repeat_type",
    "repeat_consensus.repeat_consensus",
    NULL
};




static EnsOBaseadaptorLeftJoin repeatfeatureadaptorLeftJoin[] =
{
    {NULL, NULL}
};




static const char *repeatfeatureadaptorDefaultCondition =
    "repeat_feature.repeat_consensus_id = "
    "repeat_consensus.repeat_consensus_id";




static const char *repeatfeatureadaptorFinalCondition = NULL;




/* @funcstatic repeatfeatureadaptorFetchAllBySQL ******************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Repeat Features.
** The caller is responsible for deleting the Ensembl Repeat Features before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor::_objs_from_sth
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool repeatfeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                const AjPStr statement,
                                                EnsPAssemblymapper am,
                                                EnsPSlice slice,
                                                AjPList rfs)
{
    ajuint anid = 0;
    ajuint rcid = 0;
    ajuint rfid = 0;
    ajuint srid = 0;

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

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr rcname  = NULL;
    AjPStr rcclass = NULL;
    AjPStr rctype  = NULL;
    AjPStr rccons  = NULL;

    EnsPAnalysis analysis = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPRepeatconsensus rc         = NULL;
    EnsPRepeatconsensusadaptor rca = NULL;

    EnsPRepeatfeature rf         = NULL;
    EnsPRepeatfeatureadaptor rfa = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    debug = ajDebugTest("repeatfeatureadaptorFetchAllBySQL");

    if(debug)
        ajDebug("repeatfeatureadaptorFetchAllBySQL\n"
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
        rcname   = ajStrNew();
        rcclass  = ajStrNew();
        rctype   = ajStrNew();
        rccons   = ajStrNew();

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
        ajSqlcolumnToStr(sqlr, &rcname);
        ajSqlcolumnToStr(sqlr, &rcclass);
        ajSqlcolumnToStr(sqlr, &rctype);
        ajSqlcolumnToStr(sqlr, &rccons);

        /* Need to get the internal Ensembl Sequence Region identifier. */

        srid = ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Features, the range needs
        ** checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("repeatfeatureadaptorFetchAllBySQL got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("repeatfeatureadaptorFetchAllBySQL got a "
                    "Sequence Region end coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srend, INT_MAX);

        slstrand = srstrand;

        /* Create an Ensembl Repeat Consensus. */

        rc = ensRepeatconsensusNew(rca,
                                   rcid,
                                   rcname,
                                   rcclass,
                                   rctype,
                                   rccons,
                                   ajStrGetLen(rccons));

        ajStrDel(&rcname);
        ajStrDel(&rcclass);
        ajStrDel(&rctype);
        ajStrDel(&rccons);

        /* Fetch the corresponding Ensembl Analysis. */

        ensAnalysisadaptorFetchByIdentifier(aa, anid, &analysis);

        /* Fetch a Slice spanning the entire Sequence Region. */

        ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);

        /*
        ** FIXME: There is a difference to the Exon Adaptor.
        ** The Exonadaptor fetches an Assembly Mapper in case a Slice but no
        ** Assembly Mapper has been provided.
        **
        ** FIXME: There are big problems with signedess to resolve!
        ** srstart from database is ajuint, ensSliceGetStart is ajint!
        ** we need a second set of variables that is signed and
        ** we should probably test before the assignment whether the value of
        ** the unsigned is out of the range of the signed???
        */

        /*
        ** Increase the reference counter of the Ensembl Assembly Mapper if
        ** one has been specified, otherwise fetch it from the database if a
        ** destination Slice has been specified.
        */

        if(am)
            am = ensAssemblymapperNewRef(am);
        else if(slice && (!ensCoordsystemMatch(
                              ensSliceGetCoordsystem(slice),
                              ensSliceGetCoordsystem(srslice))))
            am = ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper is defined at this point.
        */

        if(am)
        {
            ensAssemblymapperFastMap(am,
                                     ensSliceGetSeqregion(srslice),
                                     slstart,
                                     slend,
                                     slstrand,
                                     mrs);

            /*
            ** The ensAssemblymapperFastMap function returns at best one
            ** Ensembl Mapper Result.
            */

            ajListPop(mrs, (void **) &mr);

            /*
            ** Skip Features that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
            {
                if(debug)
                {
                    ajDebug("repeatfeatureadaptorFetchAllBySQL mapped "
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

                ensRepeatconsensusDel(&rc);

                ensAssemblymapperDel(&am);

                ensMapperresultDel(&mr);

                continue;
            }

            srid = ensMapperresultGetObjectIdentifier(mr);

            slstart = ensMapperresultGetStart(mr);

            slend = ensMapperresultGetEnd(mr);

            slstrand = ensMapperresultGetStrand(mr);

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

            if(ensSliceGetLength(slice) <= INT_MAX)
                sllength = (ajint) ensSliceGetLength(slice);
            else
                ajFatal("repeatfeatureadaptorFetchAllBySQL got a Slice, "
                        "which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceGetLength(slice), INT_MAX);

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
            ** Throw away Repeat Features off the end of the
            ** requested Slice.
            */

            if((slstart > sllength) ||
               (slend < 1) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                ensSliceDel(&srslice);

                ensAnalysisDel(&analysis);

                ensRepeatconsensusDel(&rc);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        /* Finally, create a new Ensembl Repeat Feature. */

        feature = ensFeatureNewS(analysis,
                                 srslice,
                                 slstart,
                                 slend,
                                 slstrand);

        rf = ensRepeatfeatureNew(rfa,
                                 rfid,
                                 feature,
                                 rc,
                                 rpstart,
                                 rpend,
                                 score);

        ajListPushAppend(rfs, (void *) rf);

        ensFeatureDel(&feature);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);

        ensAnalysisDel(&analysis);

        ensRepeatconsensusDel(&rc);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

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
** @fnote None
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
** Default Ensembl Repeat Feature Adaptor constructor.
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPRepeatfeatureadaptor ensRepeatfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPRepeatfeatureadaptor rfa = NULL;

    if(!dba)
        return NULL;

    AJNEW0(rfa);

    rfa->Adaptor = ensFeatureadaptorNew(
        dba,
        repeatfeatureadaptorTables,
        repeatfeatureadaptorColumns,
        repeatfeatureadaptorLeftJoin,
        repeatfeatureadaptorDefaultCondition,
        repeatfeatureadaptorFinalCondition,
        repeatfeatureadaptorFetchAllBySQL,
        (void* (*)(const void* key)) NULL,
        repeatfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        repeatfeatureadaptorCacheDelete,
        repeatfeatureadaptorCacheSize,
        repeatfeatureadaptorGetFeature,
        "Repeatfeature");

    return rfa;
}




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
** @param [d] Prfa [EnsPRepeatfeatureadaptor*] Ensembl Repeat Feature
**                                             Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensRepeatfeatureadaptorDel(EnsPRepeatfeatureadaptor *Prfa)
{
    EnsPRepeatfeatureadaptor pthis = NULL;

    if(!Prfa)
        return;

    if(!*Prfa)
        return;

    pthis = *Prfa;

    ensFeatureadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Prfa = NULL;

    return;
}




/* @func ensRepeatfeatureadaptorFetchAllBySlice *******************************
**
** Fetch all Ensembl Repeat Features on an Ensembl Slice.
** The caller is responsible for deleting the Ensembl Repeat Features before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::RepeatFeatureAdaptor::fetch_all_by_Slice
** @param [r] rfa [const EnsPRepeatfeatureadaptor] Ensembl Repeat
**                                                 Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [rN] rctype [const AjPStr] Ensembl Repeat Consensus type
** @param [rN] rcclass [const AjPStr] Ensembl Repeat Consensus class
** @param [rN] rcname [const AjPStr] Ensembl Repeat Consensus name
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatfeatureadaptorFetchAllBySlice(
    const EnsPRepeatfeatureadaptor rfa,
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

    AjPStr constraint = NULL;

    if(ajDebugTest("ensRepeatfeatureadaptorFetchAllBySlice"))
        ajDebug("ensRepeatfeatureadaptorFetchAllBySlice\n"
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
        ensFeatureadaptorEscapeC(rfa->Adaptor, &txtrctype, rctype);

        constraint =
            ajFmtStr("repeat_consensus.repeat_type = '%s'", txtrctype);

        ajCharDel(&txtrctype);
    }

    if(rcclass && ajStrGetLen(rcclass))
    {
        ensFeatureadaptorEscapeC(rfa->Adaptor, &txtrcclass, rcclass);

        if(constraint)
            ajFmtPrintAppS(&constraint,
                           " AND repeat_consensus.repeat_class = '%s'",
                           txtrcclass);
        else
            constraint =
                ajFmtStr("repeat_consensus.repeat_class = '%s'", txtrcclass);

        ajCharDel(&txtrcclass);
    }

    if(rcname && ajStrGetLen(rcname))
    {
        ensFeatureadaptorEscapeC(rfa->Adaptor, &txtrcname, rcname);

        if(constraint)
            ajFmtPrintAppS(&constraint,
                           " AND repeat_consensus.repeat_name = '%s'",
                           txtrcname);
        else
            constraint =
                ajFmtStr("repeat_consensus.repeat_name = '%s'", txtrcname);

        ajCharDel(&txtrcname);
    }

    ensFeatureadaptorFetchAllBySliceConstraint(rfa->Adaptor,
                                               slice,
                                               constraint,
                                               anname,
                                               rfs);

    ajStrDel(&constraint);

    return ajTrue;
}

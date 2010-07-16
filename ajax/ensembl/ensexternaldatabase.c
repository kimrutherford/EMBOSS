/******************************************************************************
** @source Ensembl External Database functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.15 $
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

#include "ensexternaldatabase.h"
#include "ensbaseadaptor.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* externaldatabaseStatus *****************************************************
**
** The Ensembl External Database status is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to
** EnsEExternaldatabaseStatus.
**
** KNOWNXREF:
** KNOWN:
** XREF:
** PRED:
** ORTH:
** PSEUDO:
**
******************************************************************************/

static const char *externaldatabaseStatus[] =
{
    NULL,
    "KNOWNXREF",
    "KNOWN",
    "XREF",
    "PRED",
    "ORTH",
    "PSEUDO",
    NULL
};




/* externaldatabaseType *******************************************************
**
** The Ensembl External Database type is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to
** EnsEExternaldatabaseType.
**
** ARRAY:
** ALT_TRANS:
** MISC:
** LIT:
** PRIMARY_DB_SYNONYM:
** ENSEMBL:
**
******************************************************************************/

static const char *externaldatabaseType[] =
{
    NULL,
    "ARRAY",
    "ALT_TRANS",
    "MISC",
    "LIT",
    "PRIMARY_DB_SYNONYM",
    "ENSEMBL",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool externaldatabaseadaptorFetchAllBySQL(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList edbs);

static AjBool externaldatabaseadaptorCacheInsert(
    EnsPExternaldatabaseadaptor edba,
    EnsPExternaldatabase *Pedb);

static AjBool externaldatabaseadaptorCacheInit(
    EnsPExternaldatabaseadaptor edba);

static void externaldatabaseadaptorCacheClearIdentifier(void **key,
                                                        void **value,
                                                        void *cl);

static void externaldatabaseadaptorCacheClearName(void **key,
                                                  void **value,
                                                  void *cl);

static AjBool externaldatabaseadaptorCacheExit(
    EnsPExternaldatabaseadaptor edba);

static void externaldatabaseadaptorFetchAll(const void *key,
                                            void **value,
                                            void *cl);




/* @filesection ensexternaldatabase *******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPExternaldatabase] External Database **********************
**
** Functions for manipulating Ensembl External Database objects
**
** @cc Bio::EnsEMBL::DbEntry CVS Revision: 1.43
**
** @nam2rule Externaldatabase
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl External Database by pointer.
** It is the responsibility of the user to first destroy any previous
** External Database. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExternaldatabase]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPExternaldatabase] Ensembl External Database
** @argrule Ref object [EnsPExternaldatabase] Ensembl External Database
**
** @valrule * [EnsPExternaldatabase] Ensembl External Database
**
** @fcategory new
******************************************************************************/




/* @func ensExternaldatabaseNew ***********************************************
**
** Default constructor for an Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] edba [EnsPExternaldatabaseadaptor] External Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::DBEntry::new
** @param [u] name [AjPStr] Database name
** @param [u] release [AjPStr] Database release
** @param [u] displayname [AjPStr] Database display name
** @param [u] secondaryname [AjPStr] Secondary name
** @param [u] secondarytable [AjPStr] Secondary table
** @param [u] description [AjPStr] Description
** @param [r] primarylinkable [AjBool] Primary identifier is linkable
** @param [r] displaylinkable [AjBool] Display identifier is linkable
** @param [r] status [EnsEExternaldatabaseStatus] Status
** @param [r] type [EnsEExternaldatabaseType] Type
** @param [r] priority [ajint] Display priority
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNew(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    AjPStr name,
    AjPStr release,
    AjPStr displayname,
    AjPStr secondaryname,
    AjPStr secondarytable,
    AjPStr description,
    AjBool primarylinkable,
    AjBool displaylinkable,
    EnsEExternaldatabaseStatus status,
    EnsEExternaldatabaseType type,
    ajint priority)
{
    EnsPExternaldatabase edb = NULL;

    if(!name)
        return NULL;

    AJNEW0(edb);

    edb->Use        = 1;
    edb->Identifier = identifier;
    edb->Adaptor    = edba;

    if(name)
        edb->Name = ajStrNewRef(name);

    if(release)
        edb->Release = ajStrNewRef(release);

    if(displayname)
        edb->DisplayName = ajStrNewRef(displayname);

    if(secondaryname)
        edb->SecondaryName = ajStrNewRef(secondaryname);

    if(secondarytable)
        edb->SecondaryTable = ajStrNewRef(secondarytable);

    if(description)
        edb->Description = ajStrNewRef(description);

    edb->PrimaryIdIsLinkable = primarylinkable;
    edb->DisplayIdIsLinkable = displaylinkable;
    edb->Status              = status;
    edb->Type                = type;
    edb->Priority            = priority;

    return edb;
}




/* @func ensExternaldatabaseNewObj ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNewObj(
    const EnsPExternaldatabase object)
{
    EnsPExternaldatabase edb = NULL;

    if(!object)
        return NULL;

    AJNEW0(edb);

    edb->Use        = 1;
    edb->Identifier = object->Identifier;
    edb->Adaptor    = object->Adaptor;

    if(object->Name)
        edb->Name = ajStrNewRef(object->Name);

    if(object->Release)
        edb->Release = ajStrNewRef(object->Release);

    if(object->DisplayName)
        edb->DisplayName = ajStrNewRef(object->DisplayName);

    if(object->SecondaryName)
        edb->SecondaryName = ajStrNewRef(object->SecondaryName);

    if(object->SecondaryTable)
        edb->SecondaryTable = ajStrNewRef(object->SecondaryTable);

    if(object->Description)
        edb->Description = ajStrNewRef(object->Description);

    edb->PrimaryIdIsLinkable = object->PrimaryIdIsLinkable;
    edb->DisplayIdIsLinkable = object->DisplayIdIsLinkable;

    edb->Status = object->Status;
    edb->Type   = object->Type;

    edb->Priority = object->Priority;

    return edb;
}




/* @func ensExternaldatabaseNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsPExternaldatabase] Ensembl External Database
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNewRef(EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    edb->Use++;

    return edb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl External Database.
**
** @fdata [EnsPExternaldatabase]
** @fnote None
**
** @nam3rule Del Destroy (free) an External Database object
**
** @argrule * Pedb [EnsPExternaldatabase*] External Database object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExternaldatabaseDel ***********************************************
**
** Default Ensembl External Database destructor.
**
** @param [d] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [void]
** @@
******************************************************************************/

void ensExternaldatabaseDel(EnsPExternaldatabase *Pedb)
{
    EnsPExternaldatabase pthis = NULL;

    if(!Pedb)
        return;

    if(!*Pedb)
        return;

    pthis = *Pedb;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pedb = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Release);
    ajStrDel(&pthis->DisplayName);
    ajStrDel(&pthis->SecondaryName);
    ajStrDel(&pthis->SecondaryTable);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pedb = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
** @fnote None
**
** @nam3rule Get Return External Database attribute(s)
** @nam4rule GetAdaptor Return the Ensembl External Database Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetRelease Return the release
** @nam4rule GetSecondaryName Return the secondary name
** @nam4rule GetSecondaryTable Return the secondary table
** @nam4rule GetDescription Return the description
** @nam4rule GetPrimaryIdIsLinkable Return the 'primary id is linkable'
** @nam4rule GetDisplayIdIsLinkable Return the 'display id is linkable'
** @nam4rule GetStatus Return the status
** @nam4rule GetType Return the type
** @nam4rule GetPriority Return the priority
**
** @argrule * edb [const EnsPExternaldatabase] External Database
**
** @valrule Adaptor [EnsPExternaldatabaseadaptor] Ensembl External
**                                                Database Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Release [AjPStr] Release
** @valrule SecondaryName [AjPStr] Secondary name
** @valrule SecondaryTable [AjPStr] Secondary table
** @valrule Description [AjPStr] Description
** @valrule PrimaryIdIsLinkable [AjBool] Primary identifier is linkable
** @valrule DisplayIdIsLinkable [AjBool] Display identifier is linkable
** @valrule Status [EnsEExternaldatabaseStatus] Status
** @valrule Type [EnsEExternaldatabaseType] Type
** @valrule Priority [ajint] Priority
**
** @fcategory use
******************************************************************************/




/* @func ensExternaldatabaseGetAdaptor ****************************************
**
** Get the Ensembl External Database Adaptor element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [const EnsPExternaldatabaseadaptor] Ensembl External Database
**                                             Adaptor
** @@
******************************************************************************/

const EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Adaptor;
}




/* @func ensExternaldatabaseGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensExternaldatabaseGetIdentifier(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return 0;

    return edb->Identifier;
}




/* @func ensExternaldatabaseGetName *******************************************
**
** Get the name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetName(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Name;
}




/* @func ensExternaldatabaseGetRelease ****************************************
**
** Get the release element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Release
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetRelease(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Release;
}




/* @func ensExternaldatabaseGetSecondaryName **********************************
**
** Get the secondary name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Secondary name
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetSecondaryName(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->SecondaryName;
}




/* @func ensExternaldatabaseGetSecondaryTable *********************************
**
** Get the secondary table element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Secondary table
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetSecondaryTable(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->SecondaryTable;
}




/* @func ensExternaldatabaseGetDescription ************************************
**
** Get the description element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetDescription(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Description;
}




/* @func ensExternaldatabaseGetPrimaryIdIsLinkable ****************************
**
** Get the 'primary is linkable' element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id_linkable
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] Primary identifier is linkable
** @@
******************************************************************************/

AjBool ensExternaldatabaseGetPrimaryIdIsLinkable(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return ajFalse;

    return edb->PrimaryIdIsLinkable;
}




/* @func ensExternaldatabaseGetDisplayIdIsLinkable ****************************
**
** Get the 'display is linkable' element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::display_id_linkable
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] Display identifier is linkable
** @@
******************************************************************************/

AjBool ensExternaldatabaseGetDisplayIdIsLinkable(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return ajFalse;

    return edb->DisplayIdIsLinkable;
}




/* @func ensExternaldatabaseGetStatus *****************************************
**
** Get the status element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsEExternaldatabaseStatus] Status or
**                                      ensEExternaldatabaseStatusNULL
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensExternaldatabaseGetStatus(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return ensEExternaldatabaseStatusNULL;

    return edb->Status;
}




/* @func ensExternaldatabaseGetType *******************************************
**
** Get the type element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::type
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsEExternaldatabaseType] Type or ensEExternaldatabaseTypeNULL
** @@
******************************************************************************/

EnsEExternaldatabaseType ensExternaldatabaseGetType(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return ensEExternaldatabaseTypeNULL;

    return edb->Type;
}




/* @func ensExternaldatabaseGetPriority ***************************************
**
** Get the priority element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajint] Priority
** @@
******************************************************************************/

ajint ensExternaldatabaseGetPriority(const EnsPExternaldatabase edb)
{
    if(!edb)
        return 0;

    return edb->Priority;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl External Database
** @nam4rule SetAdaptor Set the Ensembl External Database Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetRelease Set the release
** @nam4rule SetSecondaryName Set the secondary name
** @nam4rule SetSecondaryTable Set the secondary table
** @nam4rule SetDescription Set the description
** @nam4rule SetPrimaryIdIsLinkable Set the primary identifier is linkable
** @nam4rule SetDisplayIdIsLinkable Set the display identifier is linkable
** @nam4rule SetStatus Set the status
** @nam4rule SetType Set the type
** @nam4rule SetPriority Set the priority
**
** @argrule * edb [EnsPExternaldatabase] Ensembl External Database object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensExternaldatabaseSetAdaptor ****************************************
**
** Set the Ensembl External Database Adaptor element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetAdaptor(EnsPExternaldatabase edb,
                                     EnsPExternaldatabaseadaptor edba)
{
    if(!edb)
        return ajFalse;

    edb->Adaptor = edba;

    return ajTrue;
}




/* @func ensExternaldatabaseSetIdentifier *************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetIdentifier(EnsPExternaldatabase edb,
                                        ajuint identifier)
{
    if(!edb)
        return ajFalse;

    edb->Identifier = identifier;

    return ajTrue;
}




/* @func ensExternaldatabaseSetName *******************************************
**
** Set the name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetName(EnsPExternaldatabase edb,
                                  AjPStr name)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->Name);

    edb->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensExternaldatabaseSetRelease ****************************************
**
** Set the release element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] release [AjPStr] Release
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetRelease(EnsPExternaldatabase edb,
                                     AjPStr release)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->Release);

    edb->Release = ajStrNewRef(release);

    return ajTrue;
}




/* @func ensExternaldatabaseSetSecondaryName **********************************
**
** Set the secondary name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] secondaryname [AjPStr] Secondary name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetSecondaryName(EnsPExternaldatabase edb,
                                           AjPStr secondaryname)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->SecondaryName);

    edb->SecondaryName = ajStrNewRef(secondaryname);

    return ajTrue;
}




/* @func ensExternaldatabaseSetSecondaryTable *********************************
**
** Set the secondary table element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] secondarytable [AjPStr] Secondary table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetSecondaryTable(EnsPExternaldatabase edb,
                                            AjPStr secondarytable)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->SecondaryTable);

    edb->SecondaryTable = ajStrNewRef(secondarytable);

    return ajTrue;
}




/* @func ensExternaldatabaseSetDescription ************************************
**
** Set the description element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetDescription(EnsPExternaldatabase edb,
                                         AjPStr description)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->Description);

    edb->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensExternaldatabaseSetPrimaryIdIsLinkable ****************************
**
** Set the primary identifier is linkble element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::primary_id_linkable
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] primarylinkable [AjBool] Primary identifier is linkable
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetPrimaryIdIsLinkable(EnsPExternaldatabase edb,
                                                 AjBool primarylinkable)
{
    if(!edb)
        return ajFalse;

    edb->PrimaryIdIsLinkable = primarylinkable;

    return ajTrue;
}




/* @func ensExternaldatabaseSetDisplayIdIsLinkable ****************************
**
** Set the display identifier is linkble element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::display_id_linkable
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] displaylinkable [AjBool] Display identifier is linkable
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetDisplayIdIsLinkable(EnsPExternaldatabase edb,
                                                 AjBool displaylinkable)
{
    if(!edb)
        return ajFalse;

    edb->DisplayIdIsLinkable = displaylinkable;

    return ajTrue;
}




/* @func ensExternaldatabaseSetStatus *****************************************
**
** Set the status element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] status [EnsEExternaldatabaseStatus] Status
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetStatus(EnsPExternaldatabase edb,
                                    EnsEExternaldatabaseStatus status)
{
    if(!edb)
        return ajFalse;

    edb->Status = status;

    return ajTrue;
}




/* @func ensExternaldatabaseSetType *******************************************
**
** Set the type element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::type
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] type [EnsEExternaldatabaseType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetType(EnsPExternaldatabase edb,
                                  EnsEExternaldatabaseType type)
{
    if(!edb)
        return ajFalse;

    edb->Type = type;

    return ajTrue;
}




/* @func ensExternaldatabaseSetPriority ***************************************
**
** Set the priority element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] priority [ajint] Priority
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetPriority(EnsPExternaldatabase edb,
                                      ajint priority)
{
    if(!edb)
        return ajFalse;

    edb->Priority = priority;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
** @nam3rule Trace Report Ensembl External Database elements to debug file
**
** @argrule Trace edb [const EnsPExternaldatabase] Ensembl External Database
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensExternaldatabaseTrace *********************************************
**
** Trace an Ensembl External Database.
**
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseTrace(const EnsPExternaldatabase edb, ajuint level)
{
    AjPStr indent = NULL;

    if(!edb)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensExternaldatabaseTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Release '%S'\n"
            "%S  DisplayName '%S'\n"
            "%S  SecondaryName '%S'\n"
            "%S  SecondaryTable '%S'\n"
            "%S  Description '%S'\n"
            "%S  PrimaryIdIsLinkable '%B'\n"
            "%S  DisplayIdIsLinkable '%B'\n"
            "%S  Status %d\n"
            "%S  Priority %d\n",
            indent, edb,
            indent, edb->Use,
            indent, edb->Identifier,
            indent, edb->Adaptor,
            indent, edb->Name,
            indent, edb->Release,
            indent, edb->DisplayName,
            indent, edb->SecondaryName,
            indent, edb->SecondaryTable,
            indent, edb->Description,
            indent, edb->PrimaryIdIsLinkable,
            indent, edb->DisplayIdIsLinkable,
            indent, edb->Status,
            indent, edb->Priority);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [AjPStr] to datatype conversion functions **********************
**
** Functions for manipulating Functions for manipulating Ensembl
** External Database objects using strings
**
** @nam2rule Externaldatabase
**
*/




/* @section external database from datatype conversion functions **************
**
** Functions for converting External Database elements to other datatypes.
**
** @fdata [AjPStr]
** @fnote None
**
** @nam3rule Status Convert string value to an Ensembl External Database
**                Status element
** @nam3rule Type Convert string value to an Ensembl External Database
**                Type element
** @nam4rule FromStr Convert from string
**
** @argrule Status status [const AjPStr] Status as string
** @argrule Type   type   [const AjPStr] Type as string
**
** @valrule Status [EnsEExternaldatabaseStatus] Ensembl External Database
**                                              Status
** @valrule Status [EnsEExternaldatabaseType] Ensembl External Database Type
** @fcategory cast
******************************************************************************/




/* @func ensExternaldatabaseStatusFromStr *************************************
**
** Convert an AJAX String into an Ensembl External Database status element.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsEExternaldatabaseStatus] Ensembl External Database status or
**                                      ensEExternaldatabaseStatusNULL
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensExternaldatabaseStatusFromStr(
    const AjPStr status)
{
    register EnsEExternaldatabaseStatus i = ensEExternaldatabaseStatusNULL;

    EnsEExternaldatabaseStatus estatus = ensEExternaldatabaseStatusNULL;

    for(i = ensEExternaldatabaseStatusKnownXref;
        externaldatabaseStatus[i];
        i++)
        if(ajStrMatchC(status, externaldatabaseStatus[i]))
            estatus = i;

    if(!estatus)
        ajDebug("ensExternaldatabaseStatusFromStr encountered "
                "unexpected string '%S'.\n", status);

    return estatus;
}




/* @func ensExternaldatabaseTypeFromStr ***************************************
**
** Convert an AJAX String into an Ensembl External Database type element.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEExternaldatabaseType] Ensembl External Database type or
**                                    ensEExternalDatabaseTypeNULL
** @@
******************************************************************************/

EnsEExternaldatabaseType ensExternaldatabaseTypeFromStr(
    const AjPStr type)
{
    register EnsEExternaldatabaseType i = ensEExternaldatabaseTypeNULL;

    EnsEExternaldatabaseType etype = ensEExternaldatabaseTypeNULL;

    for(i = ensEExternaldatabaseTypeArray;
        externaldatabaseType[i];
        i++)
        if(ajStrMatchC(type, externaldatabaseType[i]))
            etype = i;

    if(!etype)
        ajDebug("ensExternaldatabaseTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @datasection [none] to datatype conversion functions ************************
**
** Functions for manipulating Functions for converting Ensembl
** External Database object status as strings
**
** @nam2rule Externaldatabase
**
*/




/* @section external database to datatype conversion functions ****************
**
** Functions for converting External Database elements to other datatypes.
**
** @fdata [none]
** @fnote None
**
** @nam3rule To Convert string value to a different datatype
** @nam4rule ToChar Convert to char*
**
** @argrule * status [EnsEExternaldatabaseStatus] Ensembl External Database
**                                                Status
** @argrule * type [EnsEExternaldatabaseType] Ensembl External Database Type
** @argrule ToChar val [char*] C-type (char*)  string
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory cast
******************************************************************************/




/* @func ensExternaldatabaseStatusToChar **************************************
**
** Convert an Ensembl External Database status element into a
** C-type (char*) string.
**
** @param [r] status [EnsEExternaldatabaseStatus] External Database status
**
** @return [const char*] External Database status C-type (char*) string
** @@
******************************************************************************/

const char* ensExternaldatabaseStatusToChar(EnsEExternaldatabaseStatus status)
{
    register EnsEExternaldatabaseStatus i = ensEExternaldatabaseStatusNULL;

    if(!status)
        return NULL;

    for(i = ensEExternaldatabaseStatusKnownXref;
        externaldatabaseStatus[i] && (i < status);
        i++);

    if(!externaldatabaseStatus[i])
        ajDebug("ensExternaldatabaseStatusToChar encountered an "
                "out of boundary error on status %d.\n", status);

    return externaldatabaseStatus[i];
}




/* @func ensExternaldatabaseTypeToChar ****************************************
**
** Convert an Ensembl External Database type element into a
** C-type (char*) string.
**
** @param [r] type [EnsEExternaldatabaseType] External Database type
**
** @return [const char*] External Database type C-type (char*) string
** @@
******************************************************************************/

const char* ensExternaldatabaseTypeToChar(EnsEExternaldatabaseType type)
{
    register EnsEExternaldatabaseType i = ensEExternaldatabaseTypeNULL;

    if(!type)
        return NULL;

    for(i = ensEExternaldatabaseTypeArray;
        externaldatabaseType[i] && (i < type);
        i++);

    if(!externaldatabaseType[i])
        ajDebug("ensExternaldatabaseTypeToChar encountered an "
                "out of boundary error on type %d.\n", type);

    return externaldatabaseType[i];
}




/* @func ensExternaldatabaseGetMemsize ****************************************
**
** Get the memory size in bytes of an Ensembl External Database.
**
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensExternaldatabaseGetMemsize(const EnsPExternaldatabase edb)
{
    ajulong size = 0;

    if(!edb)
        return 0;

    size += sizeof (EnsOExternaldatabase);

    if(edb->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Name);
    }

    if(edb->Release)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Release);
    }

    if(edb->DisplayName)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->DisplayName);
    }

    if(edb->SecondaryName)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->SecondaryName);
    }

    if(edb->SecondaryTable)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->SecondaryTable);
    }

    if(edb->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Description);
    }

    return size;
}




/* @datasection [EnsPExternaldatabaseadaptor] External Database Adaptor *******
**
** Functions for manipulating Ensembl External Database Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor CVS Revision: 1.143
**
** @nam2rule Externaldatabaseadaptor
**
******************************************************************************/

static const char *externaldatabaseadaptorTables[] =
{
    "external_db",
    NULL
};

static const char *externaldatabaseadaptorColumns[] =
{
    "external_db.external_db_id",
    "external_db.db_name",
    "external_db.db_release",
    "external_db.db_display_name",
    "external_db.secondary_db_name",
    "external_db.secondary_db_table",
    "external_db.description",
    "external_db.dbprimary_acc_linkable",
    "external_db.display_label_linkable",
    "external_db.priority",
    "external_db.status",
    "external_db.type",
    NULL
};

static EnsOBaseadaptorLeftJoin externaldatabaseadaptorLeftJoin[] =
{
    {NULL, NULL}
};

static const char *externaldatabaseadaptorDefaultCondition = NULL;

static const char *externaldatabaseadaptorFinalCondition = NULL;




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl External Database Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** External Database Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExternaldatabaseadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**
** @fcategory new
******************************************************************************/




/* @funcstatic externaldatabaseadaptorFetchAllBySQL ***************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl External Database objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] edbs [AjPList] AJAX List of Ensembl External Databases
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorFetchAllBySQL(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList edbs)
{
    ajint priority    = 0;
    ajuint identifier = 0;

    AjBool primarylinkable = AJFALSE;
    AjBool displaylinkable = AJFALSE;

    EnsEExternaldatabaseStatus estatus = ensEExternaldatabaseStatusNULL;
    EnsEExternaldatabaseType etype     = ensEExternaldatabaseTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name           = NULL;
    AjPStr release        = NULL;
    AjPStr displayname    = NULL;
    AjPStr secondaryname  = NULL;
    AjPStr secondarytable = NULL;
    AjPStr description    = NULL;
    AjPStr status         = NULL;
    AjPStr type           = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    if(ajDebugTest("externaldatabaseadaptorFetchAllBySQL"))
        ajDebug("externaldatabaseadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement '%S'\n"
                "  am %p\n"
                "  slice %p\n"
                "  edbs %p\n",
                dba,
                statement,
                am,
                slice,
                edbs);

    if(!edba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!edbs)
        return ajFalse;

    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier      = 0;
        name            = ajStrNew();
        release         = ajStrNew();
        displayname     = ajStrNew();
        secondaryname   = ajStrNew();
        secondarytable  = ajStrNew();
        description     = ajStrNew();
        primarylinkable = ajFalse;
        displaylinkable = ajFalse;
        priority        = 0;
        status          = ajStrNew();
        type            = ajStrNew();
        estatus         = ensEExternaldatabaseStatusNULL;
        etype           = ensEExternaldatabaseTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &release);
        ajSqlcolumnToStr(sqlr, &displayname);
        ajSqlcolumnToStr(sqlr, &secondaryname);
        ajSqlcolumnToStr(sqlr, &secondarytable);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToBool(sqlr, &primarylinkable);
        ajSqlcolumnToBool(sqlr, &displaylinkable);
        ajSqlcolumnToInt(sqlr, &priority);
        ajSqlcolumnToStr(sqlr, &status);
        ajSqlcolumnToStr(sqlr, &type);

        estatus = ensExternaldatabaseStatusFromStr(status);

        if(!estatus)
            ajFatal("externaldatabaseadaptorFetchAllBySQL encountered "
                    "unexpected string '%S' in the "
                    "'external_db.status' field.\n", status);

        etype = ensExternaldatabaseTypeFromStr(type);

        if(!etype)
            ajFatal("externaldatabaseadaptorFetchAllBySQL encountered "
                    "unexpected string '%S' in the "
                    "'external_db.type' field.\n", type);

        edb = ensExternaldatabaseNew(edba,
                                     identifier,
                                     name,
                                     release,
                                     displayname,
                                     secondaryname,
                                     secondarytable,
                                     description,
                                     primarylinkable,
                                     displaylinkable,
                                     estatus,
                                     etype,
                                     priority);

        ajListPushAppend(edbs, (void *) edb);

        ajStrDel(&name);
        ajStrDel(&release);
        ajStrDel(&displayname);
        ajStrDel(&secondaryname);
        ajStrDel(&secondarytable);
        ajStrDel(&description);
        ajStrDel(&status);
        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic externaldatabaseadaptorCacheInsert *****************************
**
** Insert an Ensembl External Database into the External Database
** Adaptor-internal cache.
** If an External Database with the same name element is already present in the
** adaptor cache, the External Database is deleted and a pointer to the
** cached External Database is returned.
**
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
** @param [u] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheInsert(
    EnsPExternaldatabaseadaptor edba,
    EnsPExternaldatabase *Pedb)
{
    ajuint *Pidentifier = NULL;

    EnsPExternaldatabase edb1 = NULL;
    EnsPExternaldatabase edb2 = NULL;

    if(!edba)
        return ajFalse;

    if(!edba->CacheByIdentifier)
        return ajFalse;

    if(!edba->CacheByName)
        return ajFalse;

    if(!Pedb)
        return ajFalse;

    if(!*Pedb)
        return ajFalse;

    /* Search the identifer cache. */

    edb1 = (EnsPExternaldatabase) ajTableFetch(
        edba->CacheByIdentifier,
        (const void *) &((*Pedb)->Identifier));

    /* Search the name cache. */

    edb2 = (EnsPExternaldatabase) ajTableFetch(
        edba->CacheByName,
        (const void *) (*Pedb)->Name);

    if((!edb1) && (!edb2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pedb)->Identifier;

        ajTablePut(edba->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensExternaldatabaseNewRef(*Pedb));

        /* Insert into the name cache. */

        ajTablePut(edba->CacheByName,
                   (void *) ajStrNewS((*Pedb)->Name),
                   (void *) ensExternaldatabaseNewRef(*Pedb));
    }

    if(edb1 && edb2 && (edb1 == edb2))
    {
        ajDebug("externaDatabaseadaptorCacheInsert replaced External Database "
                "%p with one already cached %p\n", *Pedb, edb1);

        ensExternaldatabaseDel(Pedb);

        Pedb = &edb1;
    }

    if(edb1 && edb2 && (edb1 != edb2))
        ajDebug("externaldatabaseadaptorCacheInsert detected External "
                "Databases in the identifier and name cache with identical "
                "names ('%S' and '%S') but differnt addresses "
                "(%p and %p).\n",
                edb1->Name, edb2->Name, edb1, edb2);

    if(edb1 && (!edb2))
        ajDebug("externaldatabaseadaptorCacheInsert detected an "
                "Ensembl External Database "
                "in the identifier, but not in the name cache.\n");

    if((!edb1) && edb2)
        ajDebug("externaldatabaseadaptorCacheInsert detected and "
                "Ensembl External Database "
                "in the name, but not in the identifier cache.\n");

    return ajTrue;
}




#if AJFALSE
/* @funcstatic externaldatabaseadaptorCacheRemove *****************************
**
** Remove an Ensembl External database from an
** External Database Adaptor-internal cache.
**
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheRemove(
    EnsPExternaldatabaseadaptor edba,
    const EnsPExternaldatabase edb)
{
    ajuint *Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPExternaldatabase edb1 = NULL;
    EnsPExternaldatabase edb2 = NULL;

    if(!edba)
        return ajFalse;

    if(!edb)
        return ajFalse;

    /* Remove the table nodes. */

    edb1 = (EnsPExternaldatabase) ajTableRemoveKey(
        edba->CacheByIdentifier,
        (const void *) &edb->Identifier,
        (void **) &Pidentifier);

    edb2 = (EnsPExternaldatabase) ajTableRemoveKey(
        edba->CacheByName,
        (const void *) edb->Name,
        (void **) &key);

    if(edb1 && (!edb2))
        ajWarn("externaldatabaseadaptorCacheRemove could remove "
               "External Database with identifier %u '%S' "
               "only from the identifier cache.\n",
               edb->Identifier, edb->Name);

    if((!edb1) && edb2)
        ajWarn("externaldatabaseadaptorCacheRemove could remove "
               "External Database with identifier %u '%S' "
               "only from the name cache.\n",
               edb->Identifier, edb->Name);

    /* Delete the keys. */

    AJFREE(Pidentifier);

    ajStrDel(&key);

    /* Delete (or at least de-reference) the Ensembl External Databases. */

    ensExternaldatabaseDel(&edb1);
    ensExternaldatabaseDel(&edb2);

    return ajTrue;
}

#endif




/* @funcstatic externaldatabaseadaptorCacheInit *******************************
**
** Initialise the internal External Database cache of an
** Ensembl External Database Adaptor.
**
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheInit(EnsPExternaldatabaseadaptor edba)
{
    AjPList edbs = NULL;

    EnsPExternaldatabase edb = NULL;

    if(!edba)
        return ajFalse;

    if(edba->CacheByIdentifier)
        return ajFalse;
    else
        edba->CacheByIdentifier =
            ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    if(edba->CacheByName)
        return ajFalse;
    else
        edba->CacheByName = ajTablestrNewLen(0);

    edbs = ajListNew();

    ensBaseadaptorGenericFetch(edba->Adaptor,
                               (AjPStr) NULL,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               edbs);

    while(ajListPop(edbs, (void **) &edb))
    {
        externaldatabaseadaptorCacheInsert(edba, &edb);

        /*
        ** Both caches hold internal references to the
        ** External Database objects.
        */

        ensExternaldatabaseDel(&edb);
    }

    ajListFree(&edbs);

    return ajTrue;
}




/* @func ensExternaldatabaseadaptorNew ****************************************
**
** Default constructor for an Ensembl External Database Adaptor.
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
** @see ensRegistryGetExternaldatabaseadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**                                       or NULL
** @@
******************************************************************************/

EnsPExternaldatabaseadaptor ensExternaldatabaseadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPExternaldatabaseadaptor edba = NULL;

    if(!dba)
        return NULL;

    AJNEW0(edba);

    edba->Adaptor = ensBaseadaptorNew(
        dba,
        externaldatabaseadaptorTables,
        externaldatabaseadaptorColumns,
        externaldatabaseadaptorLeftJoin,
        externaldatabaseadaptorDefaultCondition,
        externaldatabaseadaptorFinalCondition,
        externaldatabaseadaptorFetchAllBySQL);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** externaldatabaseadaptorCacheInit function calls
    ** ensBaseadaptorGenericFetch, which calls
    ** externaldatabaseadaptorFetchAllBySQL, which calls
    ** ensRegistryGetExternaldatabaseadaptor. At that point, however, the
    ** External Database Adaptor has not been stored in the Registry.
    ** Therefore, each ensExternaldatabaseadaptorFetch function has to test
    ** the presence of the adaptor-internal cache and eventually initialise
    ** before accessing it.
    **
    ** externaldatabaseadaptorCacheInit(edba);
    */

    return edba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl External Database Adaptor.
**
** @fdata [EnsPExternaldatabaseadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl External Database Adaptor object
**
** @argrule * Pedba [EnsPExternaldatabaseadaptor*] Ensembl External Database
**                                                 Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic externaldatabaseadaptorCacheClearIdentifier ********************
**
** An ajTableMapDel 'apply' function to clear the Ensembl External Database
** Adaptor-internal External Database cache. This function deletes the unsigned
** integer identifier key and the Ensembl External Database value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl External Database value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void externaldatabaseadaptorCacheClearIdentifier(void **key,
                                                        void **value,
                                                        void *cl)
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

    ensExternaldatabaseDel((EnsPExternaldatabase *) value);

    return;
}




/* @funcstatic externaldatabaseadaptorCacheClearName **************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl External Database
** Adaptor-internal External Database cache. This function deletes the name
** AJAX String key data and the Ensembl External Database value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl External Database value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void externaldatabaseadaptorCacheClearName(void **key,
                                                  void **value,
                                                  void *cl)
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

    ajStrDel((AjPStr *) key);

    ensExternaldatabaseDel((EnsPExternaldatabase *) value);

    return;
}




/* @funcstatic externaldatabaseadaptorCacheExit *******************************
**
** Clears the internal External Database cache of an
** Ensembl External Database Adaptor.
**
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheExit(
    EnsPExternaldatabaseadaptor edba)
{
    if(!edba)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(edba->CacheByIdentifier,
                  externaldatabaseadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&edba->CacheByIdentifier);

    /* Clear and delete the name cache. */

    ajTableMapDel(edba->CacheByName,
                  externaldatabaseadaptorCacheClearName,
                  NULL);

    ajTableFree(&edba->CacheByName);

    return ajTrue;
}




/* @func ensExternaldatabaseadaptorDel ****************************************
**
** Default destructor for an Ensembl External Database Adaptor.
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pedba [EnsPExternaldatabaseadaptor*] Ensembl External Database
**                                                 Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensExternaldatabaseadaptorDel(
    EnsPExternaldatabaseadaptor *Pedba)
{
    EnsPExternaldatabaseadaptor pthis = NULL;

    if(!Pedba)
        return;

    if(!*Pedba)
        return;

    pthis = *Pedba;

    externaldatabaseadaptorCacheExit(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pedba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl External Database Adaptor
** object.
**
** @fdata [EnsPExternaldatabaseadaptor]
** @fnote None
**
** @nam3rule Get Return Ensembl External Database Adaptor attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Base Adaptor
**
** @argrule * aa [const EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @valrule Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensExternaldatabaseadaptorGetBaseadaptor *****************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl External Database Adaptor.
**
** @param [r] edba [const EnsPExternaldatabaseadaptor] Ensembl External
**                                                     Database Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensExternaldatabaseadaptorGetBaseadaptor(
    const EnsPExternaldatabaseadaptor edba)
{
    if(!edba)
        return NULL;

    return edba->Adaptor;
}




/* @func ensExternaldatabaseadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl External Database Adaptor.
**
** @param [r] edba [const EnsPExternaldatabaseadaptor] Ensembl External
**                                                     Database Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensExternaldatabaseadaptorGetDatabaseadaptor(
    const EnsPExternaldatabaseadaptor edba)
{
    if(!edba)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(edba->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl External Database objects from an
** Ensembl Core database.
**
** @fdata [EnsPExternaldatabaseadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl External Database object(s)
** @nam4rule FetchAll Retrieve all Ensembl External Database objects
** @nam5rule FetchAllBy Retrieve all Ensembl External Database objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl External Database object
**                   matching a criterion
**
** @argrule * edba [const EnsPExternaldatabaseadaptor] Ensembl External
**                                                     Database Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl External Database objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic externaldatabaseadaptorFetchAll ********************************
**
** An ajTableMap 'apply' function to return all External Database objects from
** the Ensembl External Database Adaptor-internal cache.
**
** @param [u] key [const void *] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl External Database value data address
** @param [u] cl [void*] AJAX List of Ensembl External Database objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void externaldatabaseadaptorFetchAll(const void *key,
                                            void **value,
                                            void *cl)
{
    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    ajListPushAppend(
        (AjPList) cl,
        (void *) ensExternaldatabaseNewRef(
            *((EnsPExternaldatabase *) value)));

    return;
}




/* @func ensExternaldatabaseadaptorFetchAll ***********************************
**
** Fetch all Ensembl External Databases.
**
** The caller is responsible for deleting the Ensembl External Databases
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
** @param [u] edbs [AjPList] AJAX List of Ensembl External Databases
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseadaptorFetchAll(
    EnsPExternaldatabaseadaptor edba,
    AjPList edbs)
{
    if(!edba)
        return ajFalse;

    if(!edbs)
        return ajFalse;

    if(!edba->CacheByIdentifier)
        externaldatabaseadaptorCacheInit(edba);

    ajTableMap(edba->CacheByIdentifier,
               externaldatabaseadaptorFetchAll,
               (void *) edbs);

    return ajTrue;
}




/* @func ensExternaldatabaseadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl External Database by its internal SQL database identifier.
**
** The caller is responsible for deleting the Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [r] edba [EnsPExternaldatabaseadaptor] Ensembl External Database
**                                               Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseadaptorFetchByIdentifier(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    EnsPExternaldatabase *Pedb)
{
    AjPList edbs = NULL;

    AjPStr constraint = NULL;

    EnsPExternaldatabase edb = NULL;

    if(!edba)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pedb)
        return ajFalse;

    /*
    ** Initally, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!edba->CacheByIdentifier)
        externaldatabaseadaptorCacheInit(edba);

    *Pedb = (EnsPExternaldatabase) ajTableFetch(edba->CacheByIdentifier,
                                                (const void *) &identifier);

    if(*Pedb)
    {
        ensExternaldatabaseNewRef(*Pedb);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    constraint = ajFmtStr("external_db.external_db_id = %u", identifier);

    edbs = ajListNew();

    ensBaseadaptorGenericFetch(edba->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               edbs);

    if(ajListGetLength(edbs) > 1)
        ajWarn("ensExternaldatabaseadaptorFetchByIdentifier got more "
               "than one Ensembl External Database for (PRIMARY KEY) "
               "identifier %u.\n", identifier);

    ajListPop(edbs, (void **) Pedb);

    externaldatabaseadaptorCacheInsert(edba, Pedb);

    while(ajListPop(edbs, (void **) &edb))
    {
        externaldatabaseadaptorCacheInsert(edba, &edb);

        ensExternaldatabaseDel(&edb);
    }

    ajListFree(&edbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensExternaldatabaseadaptorFetchByName ********************************
**
** Fetch an Ensembl External Database by its name.
**
** The caller is responsible for deleting the Ensembl External Database.
**
** @param [r] edba [EnsPExternaldatabaseadaptor] Ensembl External Database
**                                               Adaptor
** @param [r] name [const AjPStr] Database name
** @param [wP] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseadaptorFetchByName(
    EnsPExternaldatabaseadaptor edba,
    const AjPStr name,
    EnsPExternaldatabase *Pedb)
{
    char *txtname = NULL;

    AjPList edbs = NULL;

    AjPStr constraint = NULL;

    EnsPExternaldatabase edb = NULL;

    if(!edba)
        return ajFalse;

    if(!(name && ajStrGetLen(name)))
        return ajFalse;

    if(!Pedb)
        return ajFalse;

    /*
    ** Initally, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!edba->CacheByName)
        externaldatabaseadaptorCacheInit(edba);

    *Pedb = (EnsPExternaldatabase) ajTableFetch(edba->CacheByName,
                                                (const void *) name);

    if(*Pedb)
    {
        ensExternaldatabaseNewRef(*Pedb);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ensBaseadaptorEscapeC(edba->Adaptor, &txtname, name);

    constraint = ajFmtStr("external_db.db_name = '%s'", txtname);

    ajCharDel(&txtname);

    edbs = ajListNew();

    ensBaseadaptorGenericFetch(edba->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               edbs);

    if(ajListGetLength(edbs) > 1)
        ajWarn("ensExternaldatabaseadaptorFetchByName got more "
               "than one Ensembl External Database for name '%S'.\n",
               name);

    ajListPop(edbs, (void **) Pedb);

    externaldatabaseadaptorCacheInsert(edba, Pedb);

    while(ajListPop(edbs, (void **) &edb))
    {
        externaldatabaseadaptorCacheInsert(edba, &edb);

        ensExternaldatabaseDel(&edb);
    }

    ajListFree(&edbs);

    ajStrDel(&constraint);

    return ajTrue;
}

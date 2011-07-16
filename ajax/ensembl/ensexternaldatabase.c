/* @source Ensembl External Database functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:52:56 $ by $Author: mks $
** @version $Revision: 1.35 $
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

#include "ensbaseadaptor.h"
#include "ensexternaldatabase.h"
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

/* @conststatic externaldatabaseStatus ****************************************
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

static const char* externaldatabaseStatus[] =
{
    "",
    "KNOWNXREF",
    "KNOWN",
    "XREF",
    "PRED",
    "ORTH",
    "PSEUDO",
    (const char*) NULL
};




/* @conststatic externaldatabaseType ******************************************
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

static const char* externaldatabaseType[] =
{
    "",
    "ARRAY",
    "ALT_TRANS",
    "MISC",
    "LIT",
    "PRIMARY_DB_SYNONYM",
    "ENSEMBL",
    (const char*) NULL
};




/* @conststatic externaldatabaseadaptorTables *********************************
**
** Array of Ensembl External Database Adaptor SQL table names
**
******************************************************************************/

static const char* externaldatabaseadaptorTables[] =
{
    "external_db",
    (const char*) NULL
};




/* @conststatic externaldatabaseadaptorColumns ********************************
**
** Array of Ensembl External Database Adaptor SQL column names
**
******************************************************************************/

static const char* externaldatabaseadaptorColumns[] =
{
    "external_db.external_db_id",
    "external_db.db_name",
    "external_db.db_release",
    "external_db.db_display_name",
    "external_db.secondary_db_name",
    "external_db.secondary_db_table",
    "external_db.description",
    "external_db.priority",
    "external_db.status",
    "external_db.type",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool externaldatabaseadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList edbs);

static AjBool externaldatabaseadaptorCacheInsert(
    EnsPExternaldatabaseadaptor edba,
    EnsPExternaldatabase* Pedb);

static AjBool externaldatabaseadaptorCacheInit(
    EnsPExternaldatabaseadaptor edba);

static void externaldatabaseadaptorCacheClearIdentifier(void** key,
                                                        void** value,
                                                        void* cl);

static void externaldatabaseadaptorCacheClearName(void** key,
                                                  void** value,
                                                  void* cl);

static AjBool externaldatabaseadaptorCacheExit(
    EnsPExternaldatabaseadaptor edba);

static void externaldatabaseadaptorFetchAll(const void* key,
                                            void** value,
                                            void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensexternaldatabase *******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPExternaldatabase] Ensembl External Database **************
**
** @nam2rule Externaldatabase Functions for manipulating
** Ensembl External Database objects
**
** @cc Bio::EnsEMBL::DBEntry
** @cc CVS Revision: 1.49
** @cc CVS Tag: branch-ensembl-62
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy edb [const EnsPExternaldatabase] Ensembl External Database
** @argrule Ini edba [EnsPExternaldatabaseadaptor] External Database Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Database name
** @argrule Ini release [AjPStr] Database release
** @argrule Ini displayname [AjPStr] Database display name
** @argrule Ini secondaryname [AjPStr] Secondary name
** @argrule Ini secondarytable [AjPStr] Secondary table
** @argrule Ini description [AjPStr] Description
** @argrule Ini status [EnsEExternaldatabaseStatus] Status
** @argrule Ini type [EnsEExternaldatabaseType] Type
** @argrule Ini priority [ajint] Display priority
** @argrule Ref edb [EnsPExternaldatabase] Ensembl External Database
**
** @valrule * [EnsPExternaldatabase] Ensembl External Database or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensExternaldatabaseNewCpy ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNewCpy(
    const EnsPExternaldatabase edb)
{
    EnsPExternaldatabase pthis = NULL;

    if(!edb)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1;
    pthis->Identifier = edb->Identifier;
    pthis->Adaptor    = edb->Adaptor;

    if(edb->Name)
        pthis->Name = ajStrNewRef(edb->Name);

    if(edb->Release)
        pthis->Release = ajStrNewRef(edb->Release);

    if(edb->Displayname)
        pthis->Displayname = ajStrNewRef(edb->Displayname);

    if(edb->Secondaryname)
        pthis->Secondaryname = ajStrNewRef(edb->Secondaryname);

    if(edb->Secondarytable)
        pthis->Secondarytable = ajStrNewRef(edb->Secondarytable);

    if(edb->Description)
        pthis->Description = ajStrNewRef(edb->Description);

    pthis->Status = edb->Status;
    pthis->Type   = edb->Type;

    pthis->Priority = edb->Priority;

    return pthis;
}




/* @func ensExternaldatabaseNewIni ********************************************
**
** Constructor for an Ensembl External Database with initial values.
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
** @param [u] status [EnsEExternaldatabaseStatus] Status
** @param [u] type [EnsEExternaldatabaseType] Type
** @param [r] priority [ajint] Display priority
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNewIni(
    EnsPExternaldatabaseadaptor edba,
    ajuint identifier,
    AjPStr name,
    AjPStr release,
    AjPStr displayname,
    AjPStr secondaryname,
    AjPStr secondarytable,
    AjPStr description,
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
        edb->Displayname = ajStrNewRef(displayname);

    if(secondaryname)
        edb->Secondaryname = ajStrNewRef(secondaryname);

    if(secondarytable)
        edb->Secondarytable = ajStrNewRef(secondarytable);

    if(description)
        edb->Description = ajStrNewRef(description);

    edb->Status   = status;
    edb->Type     = type;
    edb->Priority = priority;

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
** memory allocated for an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
**
** @nam3rule Del Destroy (free) an Ensembl External Database object
**
** @argrule * Pedb [EnsPExternaldatabase*] Ensembl External Database
**                                         object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExternaldatabaseDel ***********************************************
**
** Default destructor for an Ensembl External Database.
**
** @param [d] Pedb [EnsPExternaldatabase*] Ensembl External Database
**                                         object address
**
** @return [void]
** @@
******************************************************************************/

void ensExternaldatabaseDel(EnsPExternaldatabase* Pedb)
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
    ajStrDel(&pthis->Displayname);
    ajStrDel(&pthis->Secondaryname);
    ajStrDel(&pthis->Secondarytable);
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
**
** @nam3rule Get Return External Database attribute(s)
** @nam4rule Adaptor Return the Ensembl External Database Adaptor
** @nam4rule Description Return the description
** @nam4rule Displayname Return the display name
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Priority Return the priority
** @nam4rule Release Return the release
** @nam4rule Secondaryname Return the secondary name
** @nam4rule Secondarytable Return the secondary table
** @nam4rule Status Return the status
** @nam4rule Type Return the type
**
** @argrule * edb [const EnsPExternaldatabase] External Database
**
** @valrule Adaptor [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayname [AjPStr] Display name or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Priority [ajint] Priority or 0
** @valrule Release [AjPStr] Release or NULL
** @valrule Secondaryname [AjPStr] Secondary name or NULL
** @valrule Secondarytable [AjPStr] Secondary table or NULL
** @valrule Status [EnsEExternaldatabaseStatus] Status or
** ensEExternaldatabaseStatusNULL
** @valrule Type [EnsEExternaldatabaseType] Type or
** ensEExternaldatabaseTypeNULL
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
** @return [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**                                       or NULL
** @@
******************************************************************************/

EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Adaptor;
}




/* @func ensExternaldatabaseGetDescription ************************************
**
** Get the description element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetDescription(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Description;
}




/* @func ensExternaldatabaseGetDisplayname ************************************
**
** Get the display name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::db_display_name
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Display name or NULL
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetDisplayname(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return ajFalse;

    return edb->Displayname;
}




/* @func ensExternaldatabaseGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajuint] SQL database-internal identifier or 0
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
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetName(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Name;
}




/* @func ensExternaldatabaseGetPriority ***************************************
**
** Get the priority element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajint] Priority or 0
** @@
******************************************************************************/

ajint ensExternaldatabaseGetPriority(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return 0;

    return edb->Priority;
}




/* @func ensExternaldatabaseGetRelease ****************************************
**
** Get the release element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Release or NULL
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetRelease(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Release;
}




/* @func ensExternaldatabaseGetSecondaryname **********************************
**
** Get the secondary name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Secondary name or NULL
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetSecondaryname(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Secondaryname;
}




/* @func ensExternaldatabaseGetSecondarytable *********************************
**
** Get the secondary table element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Secondary table or NULL
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetSecondarytable(
    const EnsPExternaldatabase edb)
{
    if(!edb)
        return NULL;

    return edb->Secondarytable;
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




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
**
** @nam3rule Set Set one element of an Ensembl External Database
** @nam4rule Adaptor Set the Ensembl External Database Adaptor
** @nam4rule Description Set the description
** @nam4rule Displayname Set the display name
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Priority Set the priority
** @nam4rule Release Set the release
** @nam4rule Secondaryname Set the secondary name
** @nam4rule Secondarytable Set the secondary table
** @nam4rule Status Set the status
** @nam4rule Type Set the type
**
** @argrule * edb [EnsPExternaldatabase] Ensembl External Database object
** @argrule Adaptor edba [EnsPExternaldatabaseadaptor] Ensembl External
** @argrule Description description [AjPStr] Description
** @argrule Displayname displayname [AjPStr] Display name
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Priority priority [ajint] Priority
** @argrule Release release [AjPStr] Release
** @argrule Secondaryname secondaryname [AjPStr] Secondary name
** @argrule Secondarytable secondarytable [AjPStr] Secondary table
** @argrule Status status [EnsEExternaldatabaseStatus] Status
** @argrule Type type [EnsEExternaldatabaseType] Type
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
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
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




/* @func ensExternaldatabaseSetDisplayname ************************************
**
** Set the display name element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::db_display_name
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] displayname [AjPStr] Display name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetDisplayname(EnsPExternaldatabase edb,
                                         AjPStr displayname)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->Displayname);

    edb->Displayname = ajStrNewRef(displayname);

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




/* @func ensExternaldatabaseSetSecondaryname **********************************
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

AjBool ensExternaldatabaseSetSecondaryname(EnsPExternaldatabase edb,
                                           AjPStr secondaryname)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->Secondaryname);

    edb->Secondaryname = ajStrNewRef(secondaryname);

    return ajTrue;
}




/* @func ensExternaldatabaseSetSecondarytable *********************************
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

AjBool ensExternaldatabaseSetSecondarytable(EnsPExternaldatabase edb,
                                            AjPStr secondarytable)
{
    if(!edb)
        return ajFalse;

    ajStrDel(&edb->Secondarytable);

    edb->Secondarytable = ajStrNewRef(secondarytable);

    return ajTrue;
}




/* @func ensExternaldatabaseSetStatus *****************************************
**
** Set the status element of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] status [EnsEExternaldatabaseStatus] Status
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
** @param [u] type [EnsEExternaldatabaseType] Type
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




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
**
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
            "%S  Displayname '%S'\n"
            "%S  Secondaryname '%S'\n"
            "%S  Secondarytable '%S'\n"
            "%S  Description '%S'\n"
            "%S  Status %d\n"
            "%S  Priority %d\n",
            indent, edb,
            indent, edb->Use,
            indent, edb->Identifier,
            indent, edb->Adaptor,
            indent, edb->Name,
            indent, edb->Release,
            indent, edb->Displayname,
            indent, edb->Secondaryname,
            indent, edb->Secondarytable,
            indent, edb->Description,
            indent, edb->Status,
            indent, edb->Priority);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
**
** @nam3rule Calculate Calculate Ensembl External Database values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * edb [const EnsPExternaldatabase] Ensembl External Database
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensExternaldatabaseCalculateMemsize **********************************
**
** Calculate the memory size in bytes of an Ensembl External Database.
**
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensExternaldatabaseCalculateMemsize(const EnsPExternaldatabase edb)
{
    size_t size = 0;

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

    if(edb->Displayname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Displayname);
    }

    if(edb->Secondaryname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Secondaryname);
    }

    if(edb->Secondarytable)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Secondarytable);
    }

    if(edb->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Description);
    }

    return size;
}




/* @datasection [EnsEExternaldatabaseStatus] Ensembl External Database Status *
**
** @nam2rule Externaldatabase Functions for manipulating
** Ensembl External Database objects
** @nam3rule ExternaldatabaseStatus Functions for manipulating
** Ensembl External Database Status enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl External Database Status enumeration.
**
** @fdata [EnsEExternaldatabaseStatus]
**
** @nam4rule From Ensembl External Database Status query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  status  [const AjPStr] Status string
**
** @valrule * [EnsEExternaldatabaseStatus] Ensembl External Database Status
** enumeration or ensEExternaldatabaseStatusNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExternaldatabaseStatusFromStr *************************************
**
** Convert an AJAX String into an Ensembl External Database Status enumeration.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsEExternaldatabaseStatus] Ensembl External Database Status
** enumeration or ensEExternaldatabaseStatusNULL
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensExternaldatabaseStatusFromStr(
    const AjPStr status)
{
    register EnsEExternaldatabaseStatus i = ensEExternaldatabaseStatusNULL;

    EnsEExternaldatabaseStatus estatus = ensEExternaldatabaseStatusNULL;

    for(i = ensEExternaldatabaseStatusNULL;
        externaldatabaseStatus[i];
        i++)
        if(ajStrMatchC(status, externaldatabaseStatus[i]))
            estatus = i;

    if(!estatus)
        ajDebug("ensExternaldatabaseStatusFromStr encountered "
                "unexpected string '%S'.\n", status);

    return estatus;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl External Database Status enumeration.
**
** @fdata [EnsEExternaldatabaseStatus]
**
** @nam4rule To   Cast an Ensembl External Database Status enumeration
** @nam5rule Char Return C character string value
**
** @argrule To edbs [EnsEExternaldatabaseStatus] Ensembl External Database
**                                               Status enumeration
**
** @valrule Char [const char*] Ensembl External Database Status
**
** @fcategory cast
******************************************************************************/




/* @func ensExternaldatabaseStatusToChar **************************************
**
** Convert an Ensembl External Database status element into a
** C-type (char*) string.
**
** @param [u] edbs [EnsEExternaldatabaseStatus] Ensembl External Database
**                                              Status enumeration
**
** @return [const char*] Ensembl External Database Status C-type (char*) string
** @@
******************************************************************************/

const char* ensExternaldatabaseStatusToChar(EnsEExternaldatabaseStatus edbs)
{
    register EnsEExternaldatabaseStatus i = ensEExternaldatabaseStatusNULL;

    for(i = ensEExternaldatabaseStatusNULL;
        externaldatabaseStatus[i] && (i < edbs);
        i++);

    if(!externaldatabaseStatus[i])
        ajDebug("ensExternaldatabaseStatusToChar encountered an "
                "out of boundary error on "
                "Ensembl External Database Status enumeration %d.\n",
                edbs);

    return externaldatabaseStatus[i];
}




/* @datasection [EnsEExternaldatabaseType] Ensembl External Database Type *****
**
** @nam2rule Externaldatabase Functions for manipulating
** Ensembl External Database objects
** @nam3rule ExternaldatabaseType Functions for manipulating
** Ensembl External Database Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl External Database Type enumeration.
**
** @fdata [EnsEExternaldatabaseType]
**
** @nam4rule From Ensembl External Database Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEExternaldatabaseType] Ensembl External Database Type
** enumeration or ensEExternaldatabaseTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExternaldatabaseTypeFromStr ***************************************
**
** Convert an AJAX String into an Ensembl External Database Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEExternaldatabaseType] Ensembl External Database Type
** enumeration or ensEExternaldatabaseTypeNULL
** @@
******************************************************************************/

EnsEExternaldatabaseType ensExternaldatabaseTypeFromStr(
    const AjPStr type)
{
    register EnsEExternaldatabaseType i = ensEExternaldatabaseTypeNULL;

    EnsEExternaldatabaseType etype = ensEExternaldatabaseTypeNULL;

    for(i = ensEExternaldatabaseTypeNULL;
        externaldatabaseType[i];
        i++)
        if(ajStrMatchC(type, externaldatabaseType[i]))
            etype = i;

    if(!etype)
        ajDebug("ensExternaldatabaseTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl External Database Type enumeration.
**
** @fdata [EnsEExternaldatabaseType]
**
** @nam4rule To   Cast an Ensembl External Database Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To edbt [EnsEExternaldatabaseType] Ensembl External Database
**                                             Type enumeration
**
** @valrule Char [const char*] Ensembl External Database Type or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensExternaldatabaseTypeToChar ****************************************
**
** Convert an Ensembl External Database type element into a
** C-type (char*) string.
**
** @param [u] edbt [EnsEExternaldatabaseType] Ensembl External Database Type
** enumeration
**
** @return [const char*] Ensembl External Database Type C-type (char*) string
** or NULL
** @@
******************************************************************************/

const char* ensExternaldatabaseTypeToChar(EnsEExternaldatabaseType edbt)
{
    register EnsEExternaldatabaseType i = ensEExternaldatabaseTypeNULL;

    for(i = ensEExternaldatabaseTypeNULL;
        externaldatabaseType[i] && (i < edbt);
        i++);

    if(!externaldatabaseType[i])
        ajDebug("ensExternaldatabaseTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl External Database Type enumeration %d.\n",
                edbt);

    return externaldatabaseType[i];
}




/* @datasection [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**
** @nam2rule Externaldatabaseadaptor Functions for manipulating
** Ensembl External Database Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor
** @cc CVS Revision: 1.161
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl External Database Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** External Database Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPExternaldatabaseadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**
** @fcategory new
******************************************************************************/




/* @funcstatic externaldatabaseadaptorFetchAllbyStatement *********************
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

static AjBool externaldatabaseadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList edbs)
{
    ajint priority    = 0;
    ajuint identifier = 0;

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

    if(ajDebugTest("externaldatabaseadaptorFetchAllbyStatement"))
        ajDebug("externaldatabaseadaptorFetchAllbyStatement\n"
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
        ajSqlcolumnToInt(sqlr, &priority);
        ajSqlcolumnToStr(sqlr, &status);
        ajSqlcolumnToStr(sqlr, &type);

        estatus = ensExternaldatabaseStatusFromStr(status);

        if(!estatus)
            ajFatal("externaldatabaseadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'external_db.status' field.\n", status);

        etype = ensExternaldatabaseTypeFromStr(type);

        if(!etype)
            ajFatal("externaldatabaseadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'external_db.type' field.\n", type);

        edb = ensExternaldatabaseNewIni(edba,
                                        identifier,
                                        name,
                                        release,
                                        displayname,
                                        secondaryname,
                                        secondarytable,
                                        description,
                                        estatus,
                                        etype,
                                        priority);

        ajListPushAppend(edbs, (void*) edb);

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
    EnsPExternaldatabase* Pedb)
{
    ajuint* Pidentifier = NULL;

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

    edb1 = (EnsPExternaldatabase) ajTableFetchmodV(
        edba->CacheByIdentifier,
        (const void*) &((*Pedb)->Identifier));

    /* Search the name cache. */

    edb2 = (EnsPExternaldatabase) ajTableFetchmodV(
        edba->CacheByName,
        (const void*) (*Pedb)->Name);

    if((!edb1) && (!edb2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pedb)->Identifier;

        ajTablePut(edba->CacheByIdentifier,
                   (void*) Pidentifier,
                   (void*) ensExternaldatabaseNewRef(*Pedb));

        /* Insert into the name cache. */

        ajTablePut(edba->CacheByName,
                   (void*) ajStrNewS((*Pedb)->Name),
                   (void*) ensExternaldatabaseNewRef(*Pedb));
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
    ajuint* Pidentifier = NULL;

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
        (const void*) &edb->Identifier,
        (void**) &Pidentifier);

    edb2 = (EnsPExternaldatabase) ajTableRemoveKey(
        edba->CacheByName,
        (const void*) edb->Name,
        (void**) &key);

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

    AJFREE(Pidentifier);

    ajStrDel(&key);

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

static AjBool externaldatabaseadaptorCacheInit(
    EnsPExternaldatabaseadaptor edba)
{
    AjPList edbs = NULL;

    EnsPExternaldatabase edb = NULL;

    if(!edba)
        return ajFalse;

    if(edba->CacheByIdentifier)
        return ajFalse;
    else
        edba->CacheByIdentifier = ensTableuintNewLen(0);

    if(edba->CacheByName)
        return ajFalse;
    else
        edba->CacheByName = ensTablestrNewLen(0);

    edbs = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(edba->Adaptor,
                                       (AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       edbs);

    while(ajListPop(edbs, (void**) &edb))
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
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        externaldatabaseadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** externaldatabaseadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint, which calls
    ** externaldatabaseadaptorFetchAllbyStatement, which calls
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
** memory allocated for an Ensembl External Database Adaptor object.
**
** @fdata [EnsPExternaldatabaseadaptor]
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
** An ajTableMapDel "apply" function to clear the Ensembl External Database
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

static void externaldatabaseadaptorCacheClearIdentifier(void** key,
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

    ensExternaldatabaseDel((EnsPExternaldatabase*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic externaldatabaseadaptorCacheClearName **************************
**
** An ajTableMapDel "apply" function to clear the Ensembl External Database
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

static void externaldatabaseadaptorCacheClearName(void** key,
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

    ajStrDel((AjPStr*) key);

    ensExternaldatabaseDel((EnsPExternaldatabase*) value);

    *key   = NULL;
    *value = NULL;

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
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pedba [EnsPExternaldatabaseadaptor*] Ensembl External Database
**                                                 Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensExternaldatabaseadaptorDel(
    EnsPExternaldatabaseadaptor* Pedba)
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
**
** @nam3rule Get Return Ensembl External Database Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * edba [const EnsPExternaldatabaseadaptor] Ensembl External
**                                                     Database Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
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
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
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
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
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
** Functions for fetching Ensembl External Database objects from an
** Ensembl SQL database.
**
** @fdata [EnsPExternaldatabaseadaptor]
**
** @nam3rule Fetch Fetch Ensembl External Database object(s)
** @nam4rule All   Fetch all Ensembl External Database objects
** @nam5rule Allby Fetch all Ensembl External Database objects
**                 matching a criterion
** @nam4rule By    Fetch one Ensembl External Database object
**                 matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
** @nam5rule Name  Fetch by a name
**
** @argrule * edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
** @argrule All edbs [AjPList] AJAX List of Ensembl External Database objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pedb [EnsPExternaldatabase*] Ensembl External Database
**                                                    address
** @argrule ByName name [const AjPStr] Name
** @argrule ByName Pedb [EnsPExternaldatabase*] Ensembl External Database
**                                              address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic externaldatabaseadaptorFetchAll ********************************
**
** An ajTableMap "apply" function to return all External Database objects from
** the Ensembl External Database Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl External Database value data address
** @param [u] cl [void*] AJAX List of Ensembl External Database objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void externaldatabaseadaptorFetchAll(const void* key,
                                            void** value,
                                            void* cl)
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
        (void*) ensExternaldatabaseNewRef(
            *((EnsPExternaldatabase*) value)));

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
               (void*) edbs);

    return ajTrue;
}




/* @func ensExternaldatabaseadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl External Database by its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External Database
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
    EnsPExternaldatabase* Pedb)
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
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!edba->CacheByIdentifier)
        externaldatabaseadaptorCacheInit(edba);

    *Pedb = (EnsPExternaldatabase) ajTableFetchmodV(
        edba->CacheByIdentifier,
        (const void*) &identifier);

    if(*Pedb)
    {
        ensExternaldatabaseNewRef(*Pedb);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    constraint = ajFmtStr("external_db.external_db_id = %u", identifier);

    edbs = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(edba->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       edbs);

    if(ajListGetLength(edbs) > 1)
        ajWarn("ensExternaldatabaseadaptorFetchByIdentifier got more "
               "than one Ensembl External Database for (PRIMARY KEY) "
               "identifier %u.\n", identifier);

    ajListPop(edbs, (void**) Pedb);

    externaldatabaseadaptorCacheInsert(edba, Pedb);

    while(ajListPop(edbs, (void**) &edb))
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
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External Database
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
    EnsPExternaldatabase* Pedb)
{
    char* txtname = NULL;

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
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!edba->CacheByName)
        externaldatabaseadaptorCacheInit(edba);

    *Pedb = (EnsPExternaldatabase) ajTableFetchmodV(edba->CacheByName,
                                                    (const void*) name);

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

    ensBaseadaptorFetchAllbyConstraint(edba->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       edbs);

    if(ajListGetLength(edbs) > 1)
        ajWarn("ensExternaldatabaseadaptorFetchByName got more "
               "than one Ensembl External Database for name '%S'.\n",
               name);

    ajListPop(edbs, (void**) Pedb);

    externaldatabaseadaptorCacheInsert(edba, Pedb);

    while(ajListPop(edbs, (void**) &edb))
    {
        externaldatabaseadaptorCacheInsert(edba, &edb);

        ensExternaldatabaseDel(&edb);
    }

    ajListFree(&edbs);

    ajStrDel(&constraint);

    return ajTrue;
}

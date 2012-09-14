/* @source ensexternaldatabase ************************************************
**
** Ensembl External Database functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.55 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/26 06:37:49 $ by $Author: mks $
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

#include "ensbaseadaptor.h"
#include "ensexternaldatabase.h"
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

/* @conststatic externaldatabaseKStatus ***************************************
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

static const char *externaldatabaseKStatus[] =
{
    "",
    "KNOWNXREF",
    "KNOWN",
    "XREF",
    "PRED",
    "ORTH",
    "PSEUDO",
    (const char *) NULL
};




/* @conststatic externaldatabaseKType *****************************************
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
** IGNORE: This is in the Ensembl Genomes schema only.
**
******************************************************************************/

static const char *externaldatabaseKType[] =
{
    "",
    "ARRAY",
    "ALT_TRANS",
    "ALT_GENE",
    "MISC",
    "LIT",
    "PRIMARY_DB_SYNONYM",
    "ENSEMBL",
    "IGNORE",
    (const char *) NULL
};




/* @conststatic externaldatabaseadaptorKTables ********************************
**
** Array of Ensembl External Database Adaptor SQL table names
**
******************************************************************************/

static const char *externaldatabaseadaptorKTables[] =
{
    "external_db",
    (const char *) NULL
};




/* @conststatic externaldatabaseadaptorKColumns *******************************
**
** Array of Ensembl External Database Adaptor SQL column names
**
******************************************************************************/

static const char *externaldatabaseadaptorKColumns[] =
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
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool externaldatabaseadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList edbs);

static AjBool externaldatabaseadaptorCacheInsert(
    EnsPExternaldatabaseadaptor edba,
    EnsPExternaldatabase *Pedb);

static void externaldatabaseadaptorCacheByNameValdel(void **Pvalue);

static AjBool externaldatabaseadaptorCacheInit(
    EnsPExternaldatabaseadaptor edba);

static void externaldatabaseadaptorFetchAll(const void *key,
                                            void **Pvalue,
                                            void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




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
** @cc CVS Revision: 1.51
** @cc CVS Tag: branch-ensembl-66
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNewCpy(
    const EnsPExternaldatabase edb)
{
    EnsPExternaldatabase pthis = NULL;

    if (!edb)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = edb->Identifier;
    pthis->Adaptor    = edb->Adaptor;

    if (edb->Name)
        pthis->Name = ajStrNewRef(edb->Name);

    if (edb->Release)
        pthis->Release = ajStrNewRef(edb->Release);

    if (edb->Displayname)
        pthis->Displayname = ajStrNewRef(edb->Displayname);

    if (edb->Secondaryname)
        pthis->Secondaryname = ajStrNewRef(edb->Secondaryname);

    if (edb->Secondarytable)
        pthis->Secondarytable = ajStrNewRef(edb->Secondarytable);

    if (edb->Description)
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
**
** @release 6.4.0
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

    if (!name)
        return NULL;

    AJNEW0(edb);

    edb->Use        = 1U;
    edb->Identifier = identifier;
    edb->Adaptor    = edba;

    if (name)
        edb->Name = ajStrNewRef(name);

    if (release)
        edb->Release = ajStrNewRef(release);

    if (displayname)
        edb->Displayname = ajStrNewRef(displayname);

    if (secondaryname)
        edb->Secondaryname = ajStrNewRef(secondaryname);

    if (secondarytable)
        edb->Secondarytable = ajStrNewRef(secondarytable);

    if (description)
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
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternaldatabase ensExternaldatabaseNewRef(EnsPExternaldatabase edb)
{
    if (!edb)
        return NULL;

    edb->Use++;

    return edb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
**
** @nam3rule Del Destroy (free) an Ensembl External Database
**
** @argrule * Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensExternaldatabaseDel ***********************************************
**
** Default destructor for an Ensembl External Database.
**
** @param [d] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensExternaldatabaseDel(EnsPExternaldatabase *Pedb)
{
    EnsPExternaldatabase pthis = NULL;

    if (!Pedb)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensExternaldatabaseDel"))
    {
        ajDebug("ensExternaldatabaseDel\n"
                "  *Pedb %p\n",
                *Pedb);

        ensExternaldatabaseTrace(*Pedb, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pedb)
        return;

    pthis = *Pedb;

    pthis->Use--;

    if (pthis->Use)
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




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl External Database object.
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
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
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
** Get the Ensembl External Database Adaptor member of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**                                       or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternaldatabaseadaptor ensExternaldatabaseGetAdaptor(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Adaptor : NULL;
}




/* @func ensExternaldatabaseGetDescription ************************************
**
** Get the description member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetDescription(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Description : NULL;
}




/* @func ensExternaldatabaseGetDisplayname ************************************
**
** Get the display name member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::db_display_name
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Display name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetDisplayname(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Displayname : NULL;
}




/* @func ensExternaldatabaseGetIdentifier *************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensExternaldatabaseGetIdentifier(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Identifier : 0U;
}




/* @func ensExternaldatabaseGetName *******************************************
**
** Get the name member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetName(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Name : NULL;
}




/* @func ensExternaldatabaseGetPriority ***************************************
**
** Get the priority member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [ajint] Priority or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensExternaldatabaseGetPriority(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Priority : 0;
}




/* @func ensExternaldatabaseGetRelease ****************************************
**
** Get the release member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Release or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetRelease(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Release : NULL;
}




/* @func ensExternaldatabaseGetSecondaryname **********************************
**
** Get the secondary name member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Secondary name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetSecondaryname(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Secondaryname : NULL;
}




/* @func ensExternaldatabaseGetSecondarytable *********************************
**
** Get the secondary table member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [AjPStr] Secondary table or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensExternaldatabaseGetSecondarytable(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Secondarytable : NULL;
}




/* @func ensExternaldatabaseGetStatus *****************************************
**
** Get the status member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsEExternaldatabaseStatus] Status or
**                                      ensEExternaldatabaseStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensExternaldatabaseGetStatus(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Status : ensEExternaldatabaseStatusNULL;
}




/* @func ensExternaldatabaseGetType *******************************************
**
** Get the type member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::type
** @param [r] edb [const EnsPExternaldatabase] Ensembl External Database
**
** @return [EnsEExternaldatabaseType] Type or ensEExternaldatabaseTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEExternaldatabaseType ensExternaldatabaseGetType(
    const EnsPExternaldatabase edb)
{
    return (edb) ? edb->Type : ensEExternaldatabaseTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl External Database object.
**
** @fdata [EnsPExternaldatabase]
**
** @nam3rule Set Set one member of an Ensembl External Database
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
** Set the Ensembl External Database Adaptor member of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] edba [EnsPExternaldatabaseadaptor] Ensembl External
**                                               Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetAdaptor(EnsPExternaldatabase edb,
                                     EnsPExternaldatabaseadaptor edba)
{
    if (!edb)
        return ajFalse;

    edb->Adaptor = edba;

    return ajTrue;
}




/* @func ensExternaldatabaseSetDescription ************************************
**
** Set the description member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::description
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetDescription(EnsPExternaldatabase edb,
                                         AjPStr description)
{
    if (!edb)
        return ajFalse;

    ajStrDel(&edb->Description);

    edb->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensExternaldatabaseSetDisplayname ************************************
**
** Set the display name member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::db_display_name
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] displayname [AjPStr] Display name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetDisplayname(EnsPExternaldatabase edb,
                                         AjPStr displayname)
{
    if (!edb)
        return ajFalse;

    ajStrDel(&edb->Displayname);

    edb->Displayname = ajStrNewRef(displayname);

    return ajTrue;
}




/* @func ensExternaldatabaseSetIdentifier *************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl External Database.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetIdentifier(EnsPExternaldatabase edb,
                                        ajuint identifier)
{
    if (!edb)
        return ajFalse;

    edb->Identifier = identifier;

    return ajTrue;
}




/* @func ensExternaldatabaseSetName *******************************************
**
** Set the name member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::dbname
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetName(EnsPExternaldatabase edb,
                                  AjPStr name)
{
    if (!edb)
        return ajFalse;

    ajStrDel(&edb->Name);

    edb->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensExternaldatabaseSetPriority ***************************************
**
** Set the priority member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::priority
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [r] priority [ajint] Priority
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetPriority(EnsPExternaldatabase edb,
                                      ajint priority)
{
    if (!edb)
        return ajFalse;

    edb->Priority = priority;

    return ajTrue;
}




/* @func ensExternaldatabaseSetRelease ****************************************
**
** Set the release member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::release
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] release [AjPStr] Release
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetRelease(EnsPExternaldatabase edb,
                                     AjPStr release)
{
    if (!edb)
        return ajFalse;

    ajStrDel(&edb->Release);

    edb->Release = ajStrNewRef(release);

    return ajTrue;
}




/* @func ensExternaldatabaseSetSecondaryname **********************************
**
** Set the secondary name member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_name
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] secondaryname [AjPStr] Secondary name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetSecondaryname(EnsPExternaldatabase edb,
                                           AjPStr secondaryname)
{
    if (!edb)
        return ajFalse;

    ajStrDel(&edb->Secondaryname);

    edb->Secondaryname = ajStrNewRef(secondaryname);

    return ajTrue;
}




/* @func ensExternaldatabaseSetSecondarytable *********************************
**
** Set the secondary table member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::secondary_db_table
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] secondarytable [AjPStr] Secondary table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetSecondarytable(EnsPExternaldatabase edb,
                                            AjPStr secondarytable)
{
    if (!edb)
        return ajFalse;

    ajStrDel(&edb->Secondarytable);

    edb->Secondarytable = ajStrNewRef(secondarytable);

    return ajTrue;
}




/* @func ensExternaldatabaseSetStatus *****************************************
**
** Set the status member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::status
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] status [EnsEExternaldatabaseStatus] Status
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetStatus(EnsPExternaldatabase edb,
                                    EnsEExternaldatabaseStatus status)
{
    if (!edb)
        return ajFalse;

    edb->Status = status;

    return ajTrue;
}




/* @func ensExternaldatabaseSetType *******************************************
**
** Set the type member of an Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBEntry::type
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] type [EnsEExternaldatabaseType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseSetType(EnsPExternaldatabase edb,
                                  EnsEExternaldatabaseType type)
{
    if (!edb)
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
** @nam3rule Trace Report Ensembl External Database members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseTrace(const EnsPExternaldatabase edb, ajuint level)
{
    AjPStr indent = NULL;

    if (!edb)
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
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensExternaldatabaseCalculateMemsize(const EnsPExternaldatabase edb)
{
    size_t size = 0;

    if (!edb)
        return 0;

    size += sizeof (EnsOExternaldatabase);

    if (edb->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Name);
    }

    if (edb->Release)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Release);
    }

    if (edb->Displayname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Displayname);
    }

    if (edb->Secondaryname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Secondaryname);
    }

    if (edb->Secondarytable)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(edb->Secondarytable);
    }

    if (edb->Description)
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
** @valrule * [EnsEExternaldatabaseStatus]
** Ensembl External Database Status enumeration or
** ensEExternaldatabaseStatusNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExternaldatabaseStatusFromStr *************************************
**
** Convert an AJAX String into an Ensembl External Database Status enumeration.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsEExternaldatabaseStatus]
** Ensembl External Database Status enumeration or
** ensEExternaldatabaseStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEExternaldatabaseStatus ensExternaldatabaseStatusFromStr(
    const AjPStr status)
{
    register EnsEExternaldatabaseStatus i = ensEExternaldatabaseStatusNULL;

    EnsEExternaldatabaseStatus estatus = ensEExternaldatabaseStatusNULL;

    for (i = ensEExternaldatabaseStatusNULL;
         externaldatabaseKStatus[i];
         i++)
        if (ajStrMatchC(status, externaldatabaseKStatus[i]))
            estatus = i;

    if (!estatus)
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
** @argrule To edbs [EnsEExternaldatabaseStatus]
** Ensembl External Database Status enumeration
**
** @valrule Char [const char*] Ensembl External Database Status
**
** @fcategory cast
******************************************************************************/




/* @func ensExternaldatabaseStatusToChar **************************************
**
** Convert an Ensembl External Database status member into a
** C-type (char *) string.
**
** @param [u] edbs [EnsEExternaldatabaseStatus]
** Ensembl External Database Status enumeration
**
** @return [const char*]
** Ensembl External Database Status C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensExternaldatabaseStatusToChar(EnsEExternaldatabaseStatus edbs)
{
    register EnsEExternaldatabaseStatus i = ensEExternaldatabaseStatusNULL;

    for (i = ensEExternaldatabaseStatusNULL;
         externaldatabaseKStatus[i] && (i < edbs);
         i++);

    if (!externaldatabaseKStatus[i])
        ajDebug("ensExternaldatabaseStatusToChar encountered an "
                "out of boundary error on "
                "Ensembl External Database Status enumeration %d.\n",
                edbs);

    return externaldatabaseKStatus[i];
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
** @valrule * [EnsEExternaldatabaseType]
** Ensembl External Database Type enumeration or ensEExternaldatabaseTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensExternaldatabaseTypeFromStr ***************************************
**
** Convert an AJAX String into an Ensembl External Database Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEExternaldatabaseType]
** Ensembl External Database Type enumeration or ensEExternaldatabaseTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEExternaldatabaseType ensExternaldatabaseTypeFromStr(
    const AjPStr type)
{
    register EnsEExternaldatabaseType i = ensEExternaldatabaseTypeNULL;

    EnsEExternaldatabaseType etype = ensEExternaldatabaseTypeNULL;

    for (i = ensEExternaldatabaseTypeNULL;
         externaldatabaseKType[i];
         i++)
        if (ajStrMatchC(type, externaldatabaseKType[i]))
            etype = i;

    if (!etype)
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
** @argrule To edbt [EnsEExternaldatabaseType]
** Ensembl External Database Type enumeration
**
** @valrule Char [const char*] Ensembl External Database Type or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensExternaldatabaseTypeToChar ****************************************
**
** Convert an Ensembl External Database type member into a
** C-type (char *) string.
**
** @param [u] edbt [EnsEExternaldatabaseType]
** Ensembl External Database Type enumeration
**
** @return [const char*]
** Ensembl External Database Type C-type (char *) string or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensExternaldatabaseTypeToChar(EnsEExternaldatabaseType edbt)
{
    register EnsEExternaldatabaseType i = ensEExternaldatabaseTypeNULL;

    for (i = ensEExternaldatabaseTypeNULL;
         externaldatabaseKType[i] && (i < edbt);
         i++);

    if (!externaldatabaseKType[i])
        ajDebug("ensExternaldatabaseTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl External Database Type enumeration %d.\n",
                edbt);

    return externaldatabaseKType[i];
}




/* @datasection [EnsPExternaldatabaseadaptor] Ensembl External Database Adaptor
**
** @nam2rule Externaldatabaseadaptor Functions for manipulating
** Ensembl External Database Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor
** @cc CVS Revision: 1.165
** @cc CVS Tag: branch-ensembl-66
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
** @valrule * [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @funcstatic externaldatabaseadaptorFetchAllbyStatement *********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl External Database objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] edbs [AjPList] AJAX List of Ensembl External Databases
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList edbs)
{
    ajint priority    = 0;
    ajuint identifier = 0U;

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

    EnsPDatabaseadaptor dba = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    if (ajDebugTest("externaldatabaseadaptorFetchAllbyStatement"))
        ajDebug("externaldatabaseadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement '%S'\n"
                "  am %p\n"
                "  slice %p\n"
                "  edbs %p\n",
                ba,
                statement,
                am,
                slice,
                edbs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!edbs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
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

        if (!estatus)
            ajFatal("externaldatabaseadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'external_db.status' field.\n", status);

        etype = ensExternaldatabaseTypeFromStr(type);

        if (!etype)
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
** If an External Database with the same name member is already present in the
** adaptor cache, the External Database is deleted and a pointer to the
** cached External Database is returned.
**
** @param [u] edba [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
** @param [u] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheInsert(
    EnsPExternaldatabaseadaptor edba,
    EnsPExternaldatabase *Pedb)
{
    ajuint *Pidentifier = NULL;

    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPExternaldatabase edb0 = NULL;
    EnsPExternaldatabase edb1 = NULL;
    EnsPExternaldatabase edb2 = NULL;

    if (!edba)
        return ajFalse;

    if (!edba->CacheByIdentifier)
        return ajFalse;

    if (!edba->CacheByName)
        return ajFalse;

    if (!Pedb)
        return ajFalse;

    if (!*Pedb)
        return ajFalse;

    /* Search the identifer cache. */

    edb1 = (EnsPExternaldatabase) ajTableFetchmodV(
        edba->CacheByIdentifier,
        (const void *) &((*Pedb)->Identifier));

    /* Search the name cache. */

    list = (AjPList) ajTableFetchmodS(edba->CacheByName, (*Pedb)->Name);

    if (list)
    {
        iter = ajListIterNewread(list);

        while (!ajListIterDone(iter))
        {
            edb0 = (EnsPExternaldatabase) ajListIterGet(iter);

            if (ajStrMatchS(edb0->Release, (*Pedb)->Release))
            {
                edb2 = edb0;

                break;
            }
        }

        ajListIterDel(&iter);
    }

    if ((!edb1) && (!edb2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pedb)->Identifier;

        ajTablePut(edba->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensExternaldatabaseNewRef(*Pedb));

        /* Insert into the name cache. */

        if (!list)
        {
            list = ajListNew();

            ajTablePut(edba->CacheByName,
                       (void *) ajStrNewS((*Pedb)->Name),
                       (void *) list);
        }

        ajListPushAppend(list,
                         (void *) ensExternaldatabaseNewRef(*Pedb));
    }

    if (edb1 && edb2 && (edb1 == edb2))
    {
        ajDebug("externaDatabaseadaptorCacheInsert replaced External Database "
                "%p with one already cached %p\n", *Pedb, edb1);

        ensExternaldatabaseDel(Pedb);

        Pedb = &edb1;
    }

    if (edb1 && edb2 && (edb1 != edb2))
        ajDebug("externaldatabaseadaptorCacheInsert detected External "
                "Databases (%u and %u) in the identifier and name cache with "
                "identical names ('%S' and '%S') and releases ('%S' and '%S') "
                "but differnt addresses (%p and %p).\n",
                edb1->Identifier, edb2->Identifier,
                edb1->Name, edb2->Name,
                edb1->Release, edb2->Release,
                edb1, edb2);

    if (edb1 && (!edb2))
        ajDebug("externaldatabaseadaptorCacheInsert detected "
                "Ensembl External Database %u name '%S' release '%S' "
                "in the identifier, but not in the name cache.\n",
                edb1->Identifier, edb1->Name, edb1->Release);

    if ((!edb1) && edb2)
        ajDebug("externaldatabaseadaptorCacheInsert detected "
                "Ensembl External Database %u name '%S' release '%S' "
                "in the name, but not in the identifier cache.\n",
                edb2->Identifier, edb2->Name, edb2->Release);

    return ajTrue;
}




#if AJFALSE
/* @funcstatic externaldatabaseadaptorCacheRemove *****************************
**
** Remove an Ensembl External database from an
** External Database Adaptor-internal cache.
**
** @param [u] edba [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
** @param [r] edb [const EnsPExternaldatabase]
** Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheRemove(
    EnsPExternaldatabaseadaptor edba,
    const EnsPExternaldatabase edb)
{
    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPExternaldatabase ebd0 = NULL;
    EnsPExternaldatabase edb1 = NULL;
    EnsPExternaldatabase edb2 = NULL;

    if (!edba)
        return ajFalse;

    if (!edb)
        return ajFalse;

    edb1 = (EnsPExternaldatabase) ajTableRemove(
        edba->CacheByIdentifier,
        (const void *) &edb->Identifier);

    list = (AjPList) ajTableFetchmodS(edba->CacheByName, edb->Name);

    if (list)
    {
        iter = ajListIterNew(list);

        while (!ajListIterDone(iter))
        {
            edb0 = (EnsPExternaldatabase) ajListIterGet(iter);

            if (ajStrMatchS(edb0->Release, edb->Release))
            {
                edb2 = edb0;

                ajListIterRemove(iter);

                break;
            }
        }

        ajListIterDel(&iter);

        /* If the AJAX List is now empty, remove it from the AJAX Table. */

        if (!ajListGetLength(list))
        {
            ajTableRemove(edba->CacheByName, edb->Name);

            ajListFreeData(&list);
        }

    }

    if (edb1 && (!edb2))
        ajWarn("externaldatabaseadaptorCacheRemove could remove "
               "External Database with identifier %u '%S' "
               "only from the identifier cache.\n",
               edb->Identifier, edb->Name);

    if ((!edb1) && edb2)
        ajWarn("externaldatabaseadaptorCacheRemove could remove "
               "External Database with identifier %u '%S' "
               "only from the name cache.\n",
               edb->Identifier, edb->Name);

    ensExternaldatabaseDel(&edb1);
    ensExternaldatabaseDel(&edb2);

    return ajTrue;
}

#endif /* AJFALSE */




/* @funcstatic externaldatabaseadaptorCacheByNameValdel ***********************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl External Database objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void externaldatabaseadaptorCacheByNameValdel(void **Pvalue)
{
    EnsPExternaldatabase edb = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &edb))
        ensExternaldatabaseDel(&edb);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic externaldatabaseadaptorCacheInit *******************************
**
** Initialise the internal External Database cache of an
** Ensembl External Database Adaptor.
**
** @param [u] edba [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool externaldatabaseadaptorCacheInit(
    EnsPExternaldatabaseadaptor edba)
{
    AjPList edbs = NULL;

    EnsPExternaldatabase edb = NULL;

    if (!edba)
        return ajFalse;

    if (edba->CacheByIdentifier)
        return ajFalse;
    else
    {
        edba->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            edba->CacheByIdentifier,
            (void (*)(void **)) &ensExternaldatabaseDel);
    }

    if (edba->CacheByName)
        return ajFalse;
    else
    {
        edba->CacheByName = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            edba->CacheByName,
            (void (*)(void **)) &externaldatabaseadaptorCacheByNameValdel);
    }

    edbs = ajListNew();

    ensBaseadaptorFetchAll(edba->Adaptor, edbs);

    while (ajListPop(edbs, (void **) &edb))
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
** @return [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPExternaldatabaseadaptor ensExternaldatabaseadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPExternaldatabaseadaptor edba = NULL;

    if (!dba)
        return NULL;

    AJNEW0(edba);

    edba->Adaptor = ensBaseadaptorNew(
        dba,
        externaldatabaseadaptorKTables,
        externaldatabaseadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &externaldatabaseadaptorFetchAllbyStatement);

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
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl External Database Adaptor object.
**
** @fdata [EnsPExternaldatabaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl External Database Adaptor
**
** @argrule * Pedba [EnsPExternaldatabaseadaptor*]
** Ensembl External Database Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensExternaldatabaseadaptorDel(
    EnsPExternaldatabaseadaptor *Pedba)
{
    EnsPExternaldatabaseadaptor pthis = NULL;

    if (!Pedba)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensExternaldatabaseadaptorDel"))
        ajDebug("ensExternaldatabaseadaptorDel\n"
                "  *Pedba %p\n",
                *Pedba);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pedba)
        return;

    pthis = *Pedba;

    ajTableDel(&pthis->CacheByIdentifier);
    ajTableDel(&pthis->CacheByName);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pedba = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl External Database Adaptor
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
** Get the Ensembl Base Adaptor member of an
** Ensembl External Database Adaptor.
**
** @param [r] edba [const EnsPExternaldatabaseadaptor] Ensembl External
**                                                     Database Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPBaseadaptor ensExternaldatabaseadaptorGetBaseadaptor(
    const EnsPExternaldatabaseadaptor edba)
{
    return (edba) ? edba->Adaptor : NULL;
}




/* @func ensExternaldatabaseadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl External Database Adaptor.
**
** @param [r] edba [const EnsPExternaldatabaseadaptor] Ensembl External
**                                                     Database Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensExternaldatabaseadaptorGetDatabaseadaptor(
    const EnsPExternaldatabaseadaptor edba)
{
    return (edba) ? ensBaseadaptorGetDatabaseadaptor(edba->Adaptor) : NULL;
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
** @argrule * edba [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
** @argrule All edbs [AjPList] AJAX List of Ensembl External Database objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pedb [EnsPExternaldatabase*]
** Ensembl External Database address
** @argrule ByName name [const AjPStr] Database name
** @argrule ByName release [const AjPStr] Database release
** @argrule ByName Pedb [EnsPExternaldatabase*]
** Ensembl External Database address
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
** @param [u] Pvalue [void**] Ensembl External Database value data address
** @param [u] cl [void*] AJAX List of Ensembl External Database objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void externaldatabaseadaptorFetchAll(const void *key,
                                            void **Pvalue,
                                            void *cl)
{
    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    ajListPushAppend(
        (AjPList) cl,
        (void *) ensExternaldatabaseNewRef(
            *((EnsPExternaldatabase *) Pvalue)));

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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseadaptorFetchAll(
    EnsPExternaldatabaseadaptor edba,
    AjPList edbs)
{
    if (!edba)
        return ajFalse;

    if (!edbs)
        return ajFalse;

    if (!edba->CacheByIdentifier)
        externaldatabaseadaptorCacheInit(edba);

    ajTableMap(edba->CacheByIdentifier,
               &externaldatabaseadaptorFetchAll,
               (void *) edbs);

    return ajTrue;
}




/* @func ensExternaldatabaseadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl External Database by its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl External Database.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] edba [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
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

    if (!edba)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pedb)
        return ajFalse;

    if (ajDebugTest("ensExternaldatabaseadaptorFetchByIdentifier"))
        ajDebug("ensExternaldatabaseadaptorFetchByIdentifier\n"
                "  edba %p\n"
                "  identifier %u\n"
                "  Pedb %p\n",
                edba,
                identifier,
                Pedb);

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!edba->CacheByIdentifier)
        externaldatabaseadaptorCacheInit(edba);

    *Pedb = (EnsPExternaldatabase) ajTableFetchmodV(
        edba->CacheByIdentifier,
        (const void *) &identifier);

    if (*Pedb)
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

    if (ajListGetLength(edbs) > 1)
        ajWarn("ensExternaldatabaseadaptorFetchByIdentifier got more "
               "than one Ensembl External Database for (PRIMARY KEY) "
               "identifier %u.\n", identifier);

    ajListPop(edbs, (void **) Pedb);

    externaldatabaseadaptorCacheInsert(edba, Pedb);

    while (ajListPop(edbs, (void **) &edb))
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
** @param [r] release [const AjPStr] Database release
** @param [wP] Pedb [EnsPExternaldatabase*] Ensembl External Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensExternaldatabaseadaptorFetchByName(
    EnsPExternaldatabaseadaptor edba,
    const AjPStr name,
    const AjPStr release,
    EnsPExternaldatabase *Pedb)
{
    char *txtname    = NULL;
    char *txtrelease = NULL;

    AjIList iter = NULL;
    AjPList edbs = NULL;
    AjPList list = NULL;

    AjPStr constraint = NULL;

    EnsPExternaldatabase edb = NULL;

    if (!edba)
        return ajFalse;

    if (!(name && ajStrGetLen(name)))
        return ajFalse;

    if (!(release && ajStrGetLen(release)))
        return ajFalse;

    if (!Pedb)
        return ajFalse;

    *Pedb = (EnsPExternaldatabase) NULL;

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!edba->CacheByName)
        externaldatabaseadaptorCacheInit(edba);

    list = (AjPList) ajTableFetchmodS(edba->CacheByName, name);

    if (list)
    {
        iter = ajListIterNew(list);

        while (!ajListIterDone(iter))
        {
            edb = (EnsPExternaldatabase) ajListIterGet(iter);

            if (ajStrMatchS(edb->Release, release))
            {
                *Pedb = edb;

                break;
            }
        }

        ajListIterDel(&iter);
    }

    if (*Pedb)
    {
        ensExternaldatabaseNewRef(*Pedb);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ensBaseadaptorEscapeC(edba->Adaptor, &txtname, name);
    ensBaseadaptorEscapeC(edba->Adaptor, &txtrelease, release);

    constraint = ajFmtStr("external_db.db_name = '%s' "
                          "AND "
                          "external_db.db_release = '%s'",
                          txtname,
                          txtrelease);

    ajCharDel(&txtname);
    ajCharDel(&txtrelease);

    edbs = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(edba->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       edbs);

    if (ajListGetLength(edbs) > 1)
        ajWarn("ensExternaldatabaseadaptorFetchByName got more "
               "than one Ensembl External Database for name '%S' "
               "and release '%S'.\n",
               name, release);

    ajListPop(edbs, (void **) Pedb);

    externaldatabaseadaptorCacheInsert(edba, Pedb);

    while (ajListPop(edbs, (void **) &edb))
    {
        externaldatabaseadaptorCacheInsert(edba, &edb);

        ensExternaldatabaseDel(&edb);
    }

    ajListFree(&edbs);

    ajStrDel(&constraint);

    return ajTrue;
}

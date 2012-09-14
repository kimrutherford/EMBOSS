/* @source ensstorable ********************************************************
**
** Ensembl Storable functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.28 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:17 $ by $Author: mks $
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

#include "ensstorable.h"




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




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensstorable ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPStorable] Ensembl Storable *******************************
**
** @nam2rule Storable Functions for manipulating Ensembl Storable objects
**
** @cc Bio::EnsEMBL::Storable
** @cc CVS Revision: 1.26
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Storable by pointer.
** It is the responsibility of the user to first destroy any previous
** Storable. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPStorable]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy storable [const EnsPStorable] Ensembl Storable
** @argrule Ini type [EnsEStorableType] Ensembl Storable Type
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini adaptor [void*] Corresponding Ensembl Object Adaptor
** @argrule Ref storable [EnsPStorable] Ensembl Storable
**
** @valrule * [EnsPStorable] Ensembl Storable or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensStorableNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [EnsPStorable] Ensembl Storable or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPStorable ensStorableNewCpy(const EnsPStorable storable)
{
    EnsPStorable pthis = NULL;

    if (!storable)
        return NULL;

    AJNEW0(pthis);

    pthis->Type = storable->Type;

    pthis->Adaptor = storable->Adaptor;

    pthis->Identifier = storable->Identifier;

    pthis->Use = 1U;

    return pthis;
}




/* @func ensStorableNewIni ****************************************************
**
** Constructor for an Ensembl Storable with initial values.
**
** @param [u] type [EnsEStorableType] Ensembl Storable Type
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] adaptor [void*] Corresponding Ensembl Object Adaptor
**
** @return [EnsPStorable] Ensembl Storable or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPStorable ensStorableNewIni(EnsEStorableType type,
                               ajuint identifier,
                               void *adaptor)
{
    EnsPStorable storable = NULL;

    AJNEW0(storable);

    storable->Type = type;

    storable->Identifier = identifier;

    storable->Adaptor = adaptor;

    storable->Use = 1U;

    return storable;
}




/* @func ensStorableNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] storable [EnsPStorable] Ensembl Storable
**
** @return [EnsPStorable] Ensembl Storable or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPStorable ensStorableNewRef(EnsPStorable storable)
{
    if (!storable)
        return NULL;

    storable->Use++;

    return storable;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Storable object.
**
** @fdata [EnsPStorable]
**
** @nam3rule Del Destroy (free) an Ensembl Storable
**
** @argrule * Pstorable [EnsPStorable*] Ensembl Storable address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensStorableDel *******************************************************
**
** Default destructor for an Ensembl Storable.
**
** @param [d] Pstorable [EnsPStorable*] Ensembl Storable address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensStorableDel(EnsPStorable *Pstorable)
{
    EnsPStorable pthis = NULL;

    if (!Pstorable)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensStorableDel"))
        ajDebug("ensStorableDel\n"
                "  *Pstorable %p\n",
                *Pstorable);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pstorable)
        return;

    pthis = *Pstorable;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pstorable = NULL;

        return;
    }

    AJFREE(pthis);

    *Pstorable = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Storable object.
**
** @fdata [EnsPStorable]
**
** @nam3rule Get Return Storable attribute(s)
** @nam4rule Adaptor Return the Ensembl Object Adaptor
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Type Return the type
**
** @argrule * storable [const EnsPStorable] Ensembl Storable
**
** @valrule Adaptor [void*] Ensembl Object Adaptor or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Type [EnsEStorableType]
** Ensembl Storable Type or ensEStorableTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensStorableGetAdaptor ************************************************
**
** Get the Ensembl Object Adaptor member of an Ensembl Storable.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [void*] Ensembl Object Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

void* ensStorableGetAdaptor(const EnsPStorable storable)
{
    return (storable) ? storable->Adaptor : NULL;
}




/* @func ensStorableGetIdentifier *********************************************
**
** Get the SQL database-internal identifier member of an Ensembl Storable.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [ajuint] SQL database-internal identifier (primary key) or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensStorableGetIdentifier(const EnsPStorable storable)
{
    return (storable) ? storable->Identifier : 0U;
}




/* @func ensStorableGetType ***************************************************
**
** Get the Ensembl Storable Object type member of an Ensembl Storable.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [EnsEStorableType]
** Ensembl Storable Type enumeration or ensEStorableTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEStorableType ensStorableGetType(const EnsPStorable storable)
{
    return (storable) ? storable->Type : ensEStorableTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Storable object.
**
** @fdata [EnsPStorable]
**
** @nam3rule Set Set one member of an Ensembl Storable
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Adaptor Set the Ensembl Object Adaptor
** @nam4rule Type Set the type
**
** @argrule * storable [EnsPStorable] Ensembl Storable object
** @argrule Adaptor adaptor [void*] Ensembl Object Adaptor
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Type type [EnsEStorableType] Ensembl Storable Type
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensStorableSetAdaptor ************************************************
**
** Set the Ensembl Object Adaptor member of an Ensembl Storable.
**
** @param [u] storable [EnsPStorable] Ensembl Storable
** @param [u] adaptor [void*] Ensembl Object Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensStorableSetAdaptor(EnsPStorable storable, void *adaptor)
{
    if (!storable)
        return ajFalse;

    if (!adaptor)
        return ajFalse;

    storable->Adaptor = adaptor;

    return ajTrue;
}




/* @func ensStorableSetIdentifier *********************************************
**
** Set the SQL database-internal identifier member of an Ensembl Storable.
**
** @param [u] storable [EnsPStorable] Ensembl Storable
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensStorableSetIdentifier(EnsPStorable storable, ajuint identifier)
{
    if (!storable)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    storable->Identifier = identifier;

    return ajTrue;
}




/* @section query *************************************************************
**
** Functions for querying the properties of Ensembl Storable objects.
**
** @fdata [EnsPStorable]
**
** @nam3rule Is     Check whether an Ensembl Storable is in a particular state
** @nam4rule Stored Check whether an Ensembl Storable is stored
**
** @argrule * storable [const EnsPStorable] Ensembl Storable
** @argrule Stored dbc [const EnsPDatabaseconnection] Ensembl Database
**                                                    Connection
** @valrule Stored [AjBool] ajTrue if the Ensembl Storable is alread stored
**
** @fcategory use
******************************************************************************/




/* @func ensStorableIsStored **************************************************
**
** Test whether an Ensembl Storable is stored in a database defined by an
** Ensembl Database Connection.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue if the Ensembl Storable is alread stored in the
**                  SQL database
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensStorableIsStored(const EnsPStorable storable,
                           const EnsPDatabaseconnection dbc)
{
    if (!storable)
        return ajFalse;

    if (!dbc)
        return ajFalse;

    if (storable->Identifier && (!storable->Adaptor))
        return ajFalse;

    if (storable->Adaptor && (!storable->Identifier))
        return ajFalse;

    if ((!storable->Identifier) && (!storable->Adaptor))
        return ajFalse;

    /*
    ** TODO: Compare host, port and dbname of the Database Connection.
    ** How to get at the Database Connection?
    ** We would need a type-specific GetDatabaseconnection function for each
    ** object adaptor, or we use direct object access.
    ** Objectadaptor->Databaseadaptor->Databaseconnection
    */

    return ajFalse;
}

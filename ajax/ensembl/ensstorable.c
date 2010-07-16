/******************************************************************************
** @source Ensembl Storable functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.6 $
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

#include "ensstorable.h"




/* @filesection ensstorable ***************************************************
**
** @nam1rule ens Function belongs to the AJAX Ensembl library
**
******************************************************************************/




/* @datasection [EnsPStorable] Ensembl Storable *******************************
**
** Functions for Ensembl Storables
**
** @cc Bio::EnsEMBL::Storable CVS Revision: 1.19
**
** @nam2rule Storable Ensembl Storable objects
**
******************************************************************************/




/* @section functions *********************************************************
**
** @fdata [EnsPStorable]
** @fcategory misc
**
******************************************************************************/




/* @func ensStorableNew *******************************************************
**
** Default Ensembl Storable constructor.
**
** @param [r] type [EnsEStorableType] Ensembl Storable Object Type
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [r] adaptor [void*] Corresponding Ensembl Object Adaptor
**
** @return [EnsPStorable] Ensembl Storable
** @@
******************************************************************************/

EnsPStorable ensStorableNew(EnsEStorableType type,
                            ajuint identifier,
                            void* adaptor)
{
    EnsPStorable storable = NULL;

    AJNEW0(storable);

    storable->Type = type;

    storable->Identifier = identifier;

    storable->Adaptor = adaptor;

    storable->Use = 1;

    return storable;
}




/* @func ensStorableNewObj ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPStorable] Ensembl Storable
**
** @return [EnsPStorable] Ensembl Storable or NULL
** @@
******************************************************************************/

EnsPStorable ensStorableNewObj(const EnsPStorable object)
{
    EnsPStorable storable = NULL;

    AJNEW0(storable);

    storable->Type = object->Type;

    storable->Adaptor = object->Adaptor;

    storable->Identifier = object->Identifier;

    storable->Use = 1;

    return storable;
}




/* @func ensStorableNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] storable [EnsPStorable] Ensembl Storable
**
** @return [EnsPStorable] Ensembl Storable
** @@
******************************************************************************/

EnsPStorable ensStorableNewRef(EnsPStorable storable)
{
    if(!storable)
        return NULL;

    storable->Use++;

    return storable;
}




/* @func ensStorableDel *******************************************************
**
** Default Ensembl Storable destructor.
**
** @param [d] Pstorable [EnsPStorable*] Ensembl Storable address
**
** @return [void]
** @@
******************************************************************************/

void ensStorableDel(EnsPStorable *Pstorable)
{
    EnsPStorable pthis = NULL;

    if(!Pstorable)
        return;

    if(!*Pstorable)
        return;

    pthis = *Pstorable;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pstorable = NULL;

        return;
    }

    AJFREE(pthis);

    *Pstorable = NULL;

    return;
}




/* @func ensStorableGetType ***************************************************
**
** Get the Ensembl Storable Object type element of an Ensembl Storable.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [EnsEStorableType] Ensembl Storable Object type
** @@
******************************************************************************/

EnsEStorableType ensStorableGetType(const EnsPStorable storable)
{
    if(!storable)
        return ensEStorableTypeNULL;

    return storable->Type;
}




/* @func ensStorableGetAdaptor ************************************************
**
** Get the Ensembl Object Adaptor element of an Ensembl Storable.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [void*] Ensembl Object Adaptor
** @@
******************************************************************************/

void* ensStorableGetAdaptor(const EnsPStorable storable)
{
    if(!storable)
        return NULL;

    return storable->Adaptor;
}




/* @func ensStorableGetIdentifier *********************************************
**
** Get the SQL database-internal identifier element of an Ensembl Storable.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
**
** @return [ajuint] SQL database-internal identifier (primary key)
** @@
******************************************************************************/

ajuint ensStorableGetIdentifier(const EnsPStorable storable)
{
    if(!storable)
        return 0;

    return storable->Identifier;
}




/* @func ensStorableSetAdaptor ************************************************
**
** Set the Ensembl Object Adaptor element of an Ensembl Storable.
**
** @param [u] storable [EnsPStorable] Ensembl Storable
** @param [r] adaptor [void*] Ensembl Object Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensStorableSetAdaptor(EnsPStorable storable, void* adaptor)
{
    if(!storable)
        return ajFalse;

    if(!adaptor)
        return ajFalse;

    storable->Adaptor = adaptor;

    return ajTrue;
}




/* @func ensStorableSetIdentifier *********************************************
**
** Set the SQL database-internal identifier element of an Ensembl Storable.
**
** @param [u] storable [EnsPStorable] Ensembl Storable
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensStorableSetIdentifier(EnsPStorable storable, ajuint identifier)
{
    if(!storable)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    storable->Identifier = identifier;

    return ajTrue;
}




/* @func ensStorableIsStored **************************************************
**
** Test whether an Ensembl Storable is stored in a database defined by an
** Ensembl Database Connection.
**
** @param [r] storable [const EnsPStorable] Ensembl Storable
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue if the Storable is alread stored in the database
** @@
******************************************************************************/

AjBool ensStorableIsStored(const EnsPStorable storable,
                           const EnsPDatabaseconnection dbc)
{
    if(!storable)
        return ajFalse;

    if(!dbc)
        return ajFalse;

    if(storable->Identifier && (!storable->Adaptor))
        return ajFalse;

    if(storable->Adaptor && (!storable->Identifier))
        return ajFalse;

    if((!storable->Identifier) && (!storable->Adaptor))
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

/******************************************************************************
** @source Ensembl Database Connection functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified $Date: 2009/10/13 09:23:15 $ by $Author: rice $
** @version $Revision: 1.3 $
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

#include "ensdatabaseconnection.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensdatabaseconnection *****************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/



/* @datasection [EnsPDatabaseconnection] Database Connection ******************
**
** Functions for manipulating Ensembl Database Connection objects
**
** @cc Bio::EnsEMBL::DBSQL::DbConnection CVS Revision: 1.51
**
** @nam2rule Databaseconnection
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Database Connection by pointer.
** It is the responsibility of the user to first destroy any previous
** Database Connection. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDatabaseconnection]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPDatabaseconnection] Ensembl Database Connection
** @argrule Ref object [EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule * [EnsPDatabaseconnection] Ensembl Database Connection
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseconnectionNew *********************************************
**
** Default Ensembl Database Connection constructor.
**
** @param [r] client [AjEnum] AJAX SQL client type
** @param [u] user [AjPStr] SQL user name
** @param [u] password [AjPStr] SQL password
** @param [u] host [AjPStr] SQL host name
** @param [u] port [AjPStr] SQL host port
** @param [u] socket [AjPStr] UNIX socket file
** @param [u] database [AjPStr] SQL database name
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNew(AjEnum client,
                                                AjPStr user,
                                                AjPStr password,
                                                AjPStr host,
                                                AjPStr port,
                                                AjPStr socket,
                                                AjPStr database)
{
    EnsPDatabaseconnection dbc = NULL;
    
    if(!client)
	return NULL;
    
    /*
     ajDebug("ensDatabaseconnectionNew\n"
	     "  client %d\n"
	     "  user '%S'\n"
	     "  password '***'\n"
	     "  host '%S'\n"
	     "  port '%S'\n"
	     "  socket '%S'\n"
	     "  database '%S'\n",
	     client,
	     user,
	     host,
	     port,
	     socket,
	     database);
     */
    
    AJNEW0(dbc);
    
    dbc->SqlClientType = client;
    
    if(user)
	dbc->UserName = ajStrNewRef(user);
    
    if(password)
	dbc->Password = ajStrNewRef(password);
    
    if(host)
	dbc->HostName = ajStrNewRef(host);
    
    if(port)
	dbc->HostPort = ajStrNewRef(port);
    
    if(socket)
	dbc->Socket = ajStrNewRef(socket);
    
    if(database)
	dbc->DatabaseName = ajStrNewRef(database);
    
    dbc->Use = 1;
    
    return dbc;
}




/* @func ensDatabaseconnectionNewC ********************************************
**
** Construct an Ensembl Database Connection on an already existing connection.
** Optionally, a database name may be provided to connect to a different
** database on the same SQL server using the same SQL account information.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] database [AjPStr] SQL database name (optional)
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNewC(
    EnsPDatabaseconnection dbc, AjPStr database)
{
    EnsPDatabaseconnection newdbc = NULL;
    
    if(!dbc)
        return NULL;
    
    /*
     ajDebug("ensDatabaseconnectionNewC\n"
	     "  dbc %p\n"
	     "  database '%S'\n",
	     dbc,
	     database);
     
     ensDatabaseconnectionDebug(dbc, 1);
     */
    
    AJNEW0(newdbc);
    
    newdbc->SqlClientType = dbc->SqlClientType;
    
    if(dbc->UserName)
	newdbc->UserName = ajStrNewRef(dbc->UserName);
    
    if(dbc->Password)
	newdbc->Password = ajStrNewRef(dbc->Password);
    
    if(dbc->HostName)
	newdbc->HostName = ajStrNewRef(dbc->HostName);
    
    if(dbc->HostPort)
	newdbc->HostPort = ajStrNewRef(dbc->HostPort);
    
    if(dbc->Socket)
	newdbc->Socket = ajStrNewRef(dbc->Socket);
    
    if(database && ajStrGetLen(database))
        newdbc->DatabaseName = ajStrNewRef(database);
    else
    {
	if(dbc->DatabaseName)
	    newdbc->DatabaseName = ajStrNewRef(dbc->DatabaseName);
    }
    
    newdbc->Use = 1;
    
    return newdbc;
}




/* @func ensAnalysisNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNewRef(EnsPDatabaseconnection dbc)
{
    if(!dbc)
	return NULL;
    
    dbc->Use++;
    
    return dbc;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Database Connection.
**
** @fdata [EnsPDatabaseconnection]
** @fnote None
**
** @nam3rule Del Destroy (free) an Database Connection object
**
** @argrule * Pdbc [EnsPDatabaseconnection*] Ensembl Database Connection
**                                           object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseconnectionDel *********************************************
**
** Default Ensembl Database Connection destructor. Before freeing memory the
** connection to the SQL server is dropped.
**
** @param [d] Pdbc [EnsPDatabaseconnection*] Ensembl Database Connection
**                                           address
**
** @return [void]
** @@
******************************************************************************/

void ensDatabaseconnectionDel(EnsPDatabaseconnection* Pdbc)
{
    EnsPDatabaseconnection pthis = NULL;
    
    if(!Pdbc)
        return;
    
    if(!*Pdbc)
        return;

    pthis = *Pdbc;
    
    /*
     ajDebug("ensDatabaseconnectionDel\n"
	     "  *Pdbc %p\n",
	     *Pdbc);
     */
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pdbc = NULL;
	
	return;
    }
    
    ajSqlconnectionDel(&pthis->Sqlconnection);
    
    ajStrDel(&pthis->UserName);
    
    ajStrDel(&pthis->Password);
    
    ajStrDel(&pthis->HostName);
    
    ajStrDel(&pthis->HostPort);
    
    ajStrDel(&pthis->Socket);
    
    ajStrDel(&pthis->DatabaseName);
    
    AJFREE(pthis);

    *Pdbc = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
** @fnote None
**
** @nam3rule Get Return Database Connection attribute(s)
** @nam4rule GetSqlconnection Return the AJAX SQL Connection
** @nam4rule GetUserName Return the user name
** @nam4rule GetPassword Return the password
** @nam4rule GetHostName Return the host name
** @nam4rule GetHostPort Return host port
** @nam4rule GetSocket Return the UNIX socket
** @nam4rule GetDatabaseName Return the database name
** @nam4rule GetSqlClientType Return the SQL client type
**
** @argrule * dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule Sqlconnection [AjPSqlconnection] AJAX SQL Connection
** @valrule UserName [AjPStr] User name
** @valrule Password [AjPStr] Password
** @valrule HostName [AjPStr] Host name
** @valrule HostPort [AjPStr] Host port
** @valrule Socket [AjPStr] UNIX socket
** @valrule DatabaseName [AjPStr] Database name
** @valrule SqlClientType [AjEnum] SQL client type
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseconnectionGetSqlconnection ********************************
**
** Get the AJAX SQL Connection element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPSqlconnection] AJAX SQL Connection
** @@
******************************************************************************/

AjPSqlconnection ensDatabaseconnectionGetSqlconnection(
    const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->Sqlconnection;
}




/* @func ensDatabaseconnectionGetUserName *************************************
**
** Get the user name element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] User name
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetUserName(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->UserName;
}




/* @func ensDatabaseconnectionGetPassword *************************************
**
** Get the password element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Password
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetPassword(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->Password;
}




/* @func ensDatabaseconnectionGetHostName *************************************
**
** Get the host name element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Host name
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetHostName(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->HostName;
}




/* @func ensDatabaseconnectionGetHostPort *************************************
**
** Get the host port element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Host port
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetHostPort(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->HostPort;
}




/* @func ensDatabaseconnectionGetSocket ***************************************
**
** Get the UNIX socket element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] UNIX socket
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetSocket(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->Socket;
}




/* @func ensDatabaseconnectionGetDatabaseName *********************************
**
** Get the database name element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Database name
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetDatabaseName(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;
    
    return dbc->DatabaseName;
}




/* @func ensDatabaseconnectionGetSqlClientType ********************************
**
** Get the SQL client type element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjEnum] SQL client type
** @@
******************************************************************************/

AjEnum ensDatabaseconnectionGetSqlClientType(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return ajESqlClientNULL;
    
    return dbc->SqlClientType;
}




/* @func ensDatabaseconnectionMatch *******************************************
**
** Tests for matching two Ensembl Database Connections.
**
** @param [r] dbc1 [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] dbc2 [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue if the Ensembl Database Connections are equal
** @@
** The comparison is based on initial pointer equality and if that fails a
** direct comparison of Ensembl Database Connection elements.
******************************************************************************/

AjBool ensDatabaseconnectionMatch(const EnsPDatabaseconnection dbc1,
                                  const EnsPDatabaseconnection dbc2)
{
    if(!dbc1)
	return ajFalse;
    
    if(!dbc2)
	return ajFalse;
    
    if(dbc1 == dbc2)
	return ajTrue;
    
    /*
    ** The AJAX SQL Connection is not tested as it can be disconnected at any
    ** time and the database name is most likely to be different and
    ** therefore tested first. String matches are rather expensive...
    */
    
    if(!ajStrMatchS(dbc1->DatabaseName, dbc2->DatabaseName))
	return ajFalse;
    
    if(!ajStrMatchS(dbc1->UserName, dbc2->UserName))
	return ajFalse;
    
    if(!ajStrMatchS(dbc1->Password, dbc2->Password))
	return ajFalse;
    
    if(!ajStrMatchS(dbc1->HostName, dbc2->HostName))
	return ajFalse;
    
    if(!ajStrMatchS(dbc1->HostPort, dbc2->HostPort))
	return ajFalse;
    
    if(!ajStrMatchS(dbc1->Socket, dbc2->Socket))
	return ajFalse;
    
    if(dbc1->SqlClientType != dbc2->SqlClientType)
	return ajFalse;
    
    return ajTrue;
}




/* @func ensDatabaseconnectionConnect *****************************************
**
** Connect an Ensembl Database Connection to the specified SQL database.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionConnect(EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return ajFalse;
    
    if(dbc->Sqlconnection)
        return ajTrue;
    
    /*
     ajDebug("ensDatabaseconnectionConnect\n"
	     "  dbc %p\n",
	     dbc);
     */
    
    dbc->Sqlconnection = ajSqlconnectionNewData(dbc->SqlClientType,
                                                dbc->UserName,
                                                dbc->Password,
                                                dbc->HostName,
                                                dbc->HostPort,
                                                dbc->Socket,
                                                dbc->DatabaseName);
    
    if(!dbc->Sqlconnection)
    {
        ajWarn("Could not establish an SQL connection for user '%S' "
               "to host '%S' at port '%S' for database '%S'.\n",
	       dbc->UserName,
	       dbc->HostName,
               dbc->HostPort,
	       dbc->DatabaseName);
	
        return ajFalse;
    }
    
    return ajTrue;
}




/* @func ensDatabaseconnectionDisconnect **************************************
**
** Disconnect an Ensembl Database Connection from its SQL database.
**
** @param [u] Pdbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [void]
** @@
******************************************************************************/

void ensDatabaseconnectionDisconnect(EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return;
    
    /*
     ajDebug("ensDatabaseconnectionDisconnect\n"
	     "  dbc %p\n",
	     dbc);
     
     ensDatabaseconnectionDebug(dbc, 1);
     */
    
    ajSqlconnectionDel(&(dbc->Sqlconnection));
    
    return;
}




/* @func ensDatabaseconnectionIsConnected *************************************
**
** Test whether an Ensembl Database Connection has an active
** AJAX SQL Connection assigned.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue if the Ensembl Database Connection has an active
**                  AJAX SQL Connection assigned
** @@
******************************************************************************/

AjBool ensDatabaseconnectionIsConnected(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return ajFalse;
    
    if(dbc->Sqlconnection)
        return ajTrue;
    
    return ajFalse;
}




/* @func ensDatabaseconnectionSqlstatementNew *********************************
**
** Run an SQL statement against an Ensembl Database Connection.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] statement [const AjPStr] SQL statement
**
** @return [AjPSqlstatement] AJAX SQL Statement
** @@
******************************************************************************/

AjPSqlstatement ensDatabaseconnectionSqlstatementNew(
    EnsPDatabaseconnection dbc,
    const AjPStr statement)
{
    if(!dbc)
        return NULL;
    
    /*
     ajDebug("ensDatabaseconnectionSqlstatementNew\n"
	     "  dbc %p\n"
	     "  statement '%S'\n",
	     dbc,
	     statement);
     
     ensDatabaseconnectionTrace(dbc, 1);
     */
    
    if(!ensDatabaseconnectionIsConnected(dbc))
        if(!ensDatabaseconnectionConnect(dbc))
            return NULL;
    
    return ajSqlstatementNewRun(dbc->Sqlconnection, statement);
}




/* @func ensDatabaseconnectionEscapeC ****************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the char string at the returned
** address.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionEscapeC(EnsPDatabaseconnection dbc,
                                     char **Ptxt,
                                     const AjPStr str)
{
    if(!dbc)
        return ajFalse;
    
    if(!str)
	return ajFalse;
    
    /*
     ajDebug("ensDatabaseconnectionEscapeC\n"
	     "  dbc %p\n"
	     "  Ptxt %p\n"
	     "  str '%S'\n",
	     dbc,
	     Ptxt,
	     str);
     
     ensDatabaseconnectionTrace(dbc, 1);
     */
    
    if(!ensDatabaseconnectionIsConnected(dbc))
        if(!ensDatabaseconnectionConnect(dbc))
            return ajFalse;
    
    return ajSqlconnectionEscapeC(dbc->Sqlconnection, Ptxt, str);
}




/* @func ensDatabaseconnectionEscapeS ****************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the AJAX String at the returned
** address.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionEscapeS(EnsPDatabaseconnection dbc,
                                    AjPStr *Pstr,
                                    const AjPStr str)
{
    if(!dbc)
        return ajFalse;
    
    if(!str)
	return ajFalse;
    
    /*
     ajDebug("ensDatabaseconnectionEscapeS\n"
	     "  dbc %p\n"
	     "  Pstr %p\n"
	     "  str '%S'\n",
	     dbc,
	     Pstr,
	     str);
     
     ensDatabaseconnectionTrace(dbc, 1);
     */
    
    if(!ensDatabaseconnectionIsConnected(dbc))
        if(!ensDatabaseconnectionConnect(dbc))
            return ajFalse;
    
    return ajSqlconnectionEscapeS(dbc->Sqlconnection, Pstr, str);
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
** @nam3rule Trace Report Ensembl Database Connection elements to debug file
**
** @argrule Trace dbc [const EnsPDatabaseconnection] Ensembl Database
**                                                   Connection
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseconnectionTrace *******************************************
**
** Trace an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionTrace(const EnsPDatabaseconnection dbc,
                                  ajuint level)
{
    AjPStr indent = NULL;
    
    if(!dbc)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensDatabaseconnectionTrace %p\n"
	    "%S  Sqlconnection %p\n"
	    "%S  SqlClientType %d\n"
	    "%S  UserName '%S'\n"
	    "%S  Password '***'\n"
	    "%S  HostName '%S'\n"
	    "%S  HostPort '%S'\n"
	    "%S  Socket '%S'\n"
	    "%S  DatabaseName '%S'\n"
	    "%S  Use %u\n",
	    indent, dbc,
	    indent, dbc->Sqlconnection,
	    indent, dbc->SqlClientType,
	    indent, dbc->UserName,
	    indent,
	    indent, dbc->HostName,
	    indent, dbc->HostPort,
	    indent, dbc->Socket,
	    indent, dbc->DatabaseName,
	    indent, dbc->Use);
    
    ajSqlconnectionTrace(dbc->Sqlconnection, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}

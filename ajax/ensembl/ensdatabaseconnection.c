/******************************************************************************
** @source Ensembl Database Connection functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified $Date: 2010/06/16 20:58:43 $ by $Author: mks $
** @version $Revision: 1.12 $
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection CVS Revision: 1.51
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::new
** @param [r] client [AjESqlconnectionClient] AJAX SQL Connection client
** @param [u] user [AjPStr] User name
** @param [u] password [AjPStr] Password
** @param [u] host [AjPStr] Host name or IP address
** @param [u] port [AjPStr] Host TCP/IP port
** @param [u] socketfile [AjPStr] UNIX socket file
** @param [u] database [AjPStr] SQL database name
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNew(AjESqlconnectionClient client,
                                                AjPStr user,
                                                AjPStr password,
                                                AjPStr host,
                                                AjPStr port,
                                                AjPStr socketfile,
                                                AjPStr database)
{
    EnsPDatabaseconnection dbc = NULL;

    if(!client)
        return NULL;

    if(ajDebugTest("ensDatabaseconnectionNew"))
        ajDebug("ensDatabaseconnectionNew\n"
                "  client %d\n"
                "  user '%S'\n"
                "  password '***'\n"
                "  host '%S'\n"
                "  port '%S'\n"
                "  socketfile '%S'\n"
                "  database '%S'\n",
                client,
                user,
                host,
                port,
                socketfile,
                database);

    AJNEW0(dbc);

    dbc->SqlconnectionClient = client;

    if(user)
        dbc->UserName = ajStrNewRef(user);

    if(password)
        dbc->Password = ajStrNewRef(password);

    if(host)
        dbc->HostName = ajStrNewRef(host);

    if(port)
        dbc->HostPort = ajStrNewRef(port);

    if(socketfile)
        dbc->SocketFile = ajStrNewRef(socketfile);

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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::new
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

    if(ajDebugTest("ensDatabaseconnectionNewC"))
    {
        ajDebug("ensDatabaseconnectionNewC\n"
                "  dbc %p\n"
                "  database '%S'\n",
                dbc,
                database);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    AJNEW0(newdbc);

    newdbc->SqlconnectionClient = dbc->SqlconnectionClient;

    if(dbc->UserName)
        newdbc->UserName = ajStrNewRef(dbc->UserName);

    if(dbc->Password)
        newdbc->Password = ajStrNewRef(dbc->Password);

    if(dbc->HostName)
        newdbc->HostName = ajStrNewRef(dbc->HostName);

    if(dbc->HostPort)
        newdbc->HostPort = ajStrNewRef(dbc->HostPort);

    if(dbc->SocketFile)
        newdbc->SocketFile = ajStrNewRef(dbc->SocketFile);

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




/* @func ensDatabaseconnectionNewRef ******************************************
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




/* @func ensDatabaseconnectionNewUrl ******************************************
**
** Construct an Ensembl Database Connection from a Uniform Resource Locator
** with the following schema where brackets indicate optional components.
**
** client://username[:password]@host[:port][/databasename]
**
** The following URL would be an example for the public Ensembl MySQL instance.
**
** mysql://anonymous@ensembldb.ensembl.org:5306/
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::*
** @param [r] url [const AjPStr] Uniform Resource Locator
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNewUrl(const AjPStr url)
{
    AjPRegexp urlre = NULL;

    AjESqlconnectionClient eclient = ajESqlconnectionClientNULL;

    EnsPDatabaseconnection dbc = NULL;

    AjPStr client   = NULL;
    AjPStr user     = NULL;
    AjPStr password = NULL;
    AjPStr host     = NULL;
    AjPStr port     = NULL;
    AjPStr database = NULL;

    if(ajDebugTest("ensDatabaseconnectionNewUrl"))
        ajDebug("ensDatabaseconnectionNewUrl\n"
                "  url '%S'\n",
                url);

    if(!(url && ajStrGetLen(url)))
        return NULL;

    urlre = ajRegCompC("([^:]+)://(?:([^@:]+)(?:\\:([^@]*))?@)?"
                       "([^:/]+)(?:\\:)?(\\d+)?(?:\\/(\\w+))?");

    if(ajRegExec(urlre, url))
    {
        client   = ajStrNew();
        user     = ajStrNew();
        password = ajStrNew();
        host     = ajStrNew();
        port     = ajStrNew();
        database = ajStrNew();

        ajRegSubI(urlre, 1, &client);
        ajRegSubI(urlre, 2, &user);
        ajRegSubI(urlre, 3, &password);
        ajRegSubI(urlre, 4, &host);
        ajRegSubI(urlre, 5, &port);
        ajRegSubI(urlre, 6, &database);

        eclient = ajSqlconnectionClientFromStr(client);

        if(!eclient)
            ajDebug("ensDatabaseconnectionNewUrl encountered "
                    "unexpected string '%S' in the "
                    "client part of the URL '%S'.\n",
                    client, url);

        dbc = ensDatabaseconnectionNew(eclient,
                                       user,
                                       password,
                                       host,
                                       port,
                                       (AjPStr) NULL,
                                       database);

        ajStrDel(&client);
        ajStrDel(&user);
        ajStrDel(&password);
        ajStrDel(&host);
        ajStrDel(&port);
        ajStrDel(&database);
    }

    ajRegFree(&urlre);

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

    if(ajDebugTest("ensDatabaseconnectionDel"))
        ajDebug("ensDatabaseconnectionDel\n"
                "  *Pdbc %p\n",
                *Pdbc);

    pthis = *Pdbc;

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
    ajStrDel(&pthis->SocketFile);
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
** @nam4rule GetSocketFile Return the UNIX socket file
** @nam4rule GetDatabaseName Return the database name
** @nam4rule GetSqlconnectionClient Return the AJAX SQL Connection client
**
** @argrule * dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule Sqlconnection [AjPSqlconnection] AJAX SQL Connection
** @valrule UserName [AjPStr] User name
** @valrule Password [AjPStr] Password
** @valrule HostName [AjPStr] Host name
** @valrule HostPort [AjPStr] Host port
** @valrule SocketFile [AjPStr] UNIX socket file
** @valrule DatabaseName [AjPStr] Database name
** @valrule SqlconnectionClient [AjESqlconnectionClient] AJAX SQL
**                                                       Connection client
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::username
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::password
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::host
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::port
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




/* @func ensDatabaseconnectionGetSocketFile ***********************************
**
** Get the UNIX socket file element of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] UNIX socket file
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetSocketFile(const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return NULL;

    return dbc->SocketFile;
}




/* @func ensDatabaseconnectionGetDatabaseName *********************************
**
** Get the database name element of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::dbname
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




/* @func ensDatabaseconnectionGetAutoDisconnect *******************************
**
** Get the auto disconnect element of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::disconnect_when_inactive
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] Auto disconnect flag
**                  ajTrue: The Ensembl Database Connection will automatically
**                          disconnect, i.e. delete the AJAX SQL Connection,
**                          if no AJAX SQL Statement is active.
**                  ajFalse: No automatic disconnects will occur.
** @@
******************************************************************************/

AjBool ensDatabaseconnectionGetAutoDisconnect(
    const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return ajFalse;

    return dbc->AutoDisconnect;
}




/* @func ensDatabaseconnectionGetSqlconnectionClient **************************
**
** Get the AJAX SQL Connection client element of an
** Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::driver
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjESqlconnectionClient] AJAX SQL Connection client
** @@
******************************************************************************/

AjESqlconnectionClient ensDatabaseconnectionGetSqlconnectionClient(
    const EnsPDatabaseconnection dbc)
{
    if(!dbc)
        return ajESqlconnectionClientNULL;

    return dbc->SqlconnectionClient;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Database Connection
** @nam4rule SetAutoDisconnect Set the automatic disconnect flag
**
** @argrule * dbc [EnsPDatabaseconnection] Ensembl Database Connection object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDatabaseconnectionSetAutoDisconnect *******************************
**
** Set the auto disconnect element of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::disconnect_when_inactive
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] autodisconnect [AjBool] Auto disconnect flag
**
**                  ajTrue: The Ensembl Database Connection will automatically
**                          disconnect, i.e. delete the AJAX SQL Connection,
**                          if no AJAX SQL Statement is active.
**
**                  ajFalse: No automatic disconnects will occur.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionSetAutoDisconnect(EnsPDatabaseconnection dbc,
                                              AjBool autodisconnect)
{
    if(!dbc)
        return ajFalse;

    dbc->AutoDisconnect = autodisconnect;

    return ajTrue;
}




/* @func ensDatabaseconnectionMatch *******************************************
**
** Tests for matching two Ensembl Database Connections.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::equals
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

    if(!ajStrMatchS(dbc1->SocketFile, dbc2->SocketFile))
        return ajFalse;

    if(dbc1->SqlconnectionClient != dbc2->SqlconnectionClient)
        return ajFalse;

    return ajTrue;
}




/* @func ensDatabaseconnectionConnect *****************************************
**
** Connect an Ensembl Database Connection to the specified SQL database.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::connect
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

    if(ajDebugTest("ensDatabaseconnectionConnect"))
        ajDebug("ensDatabaseconnectionConnect\n"
                "  dbc %p\n",
                dbc);

    dbc->Sqlconnection = ajSqlconnectionNewData(dbc->SqlconnectionClient,
                                                dbc->UserName,
                                                dbc->Password,
                                                dbc->HostName,
                                                dbc->HostPort,
                                                dbc->SocketFile,
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
** This function will only disconnect, if no other AJAX SQL Statement holds a
** reference to the AJAX SQL Connection, i.e. no AJAX SQL Statement is active.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::disconnect_if_idle
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionDisconnect(EnsPDatabaseconnection dbc)
{
    if(ajDebugTest("ensDatabaseconnectionDisconnect"))
    {
        ajDebug("ensDatabaseconnectionDisconnect\n"
                "  dbc %p\n",
                dbc);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dbc)
        return ajFalse;

    if(!dbc->Sqlconnection)
        return ajTrue;

    /*
    ** Disconnect if no other AJAX SQL Statement has a reference to the
    ** AJAX SQL Connection, except this Ensembl Database Connection object.
    */

    if(ajSqlconnectionGetUse(dbc->Sqlconnection) == 1)
        ajSqlconnectionDel(&dbc->Sqlconnection);

    return ajTrue;
}




/* @func ensDatabaseconnectionIsConnected *************************************
**
** Test whether an Ensembl Database Connection has an active
** AJAX SQL Connection assigned.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::connected
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
** @cc Bio::EnsEMBL::DBSQL::DBConnection::prepare
** @cc Bio::EnsEMBL::DBSQL::DBConnection::do
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
    if(ajDebugTest("ensDatabaseconnectionSqlstatementNew"))
    {
        ajDebug("ensDatabaseconnectionSqlstatementNew\n"
                "  dbc %p\n"
                "  statement '%S'\n",
                dbc,
                statement);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dbc)
        return NULL;

    if(!ensDatabaseconnectionIsConnected(dbc))
        if(!ensDatabaseconnectionConnect(dbc))
            return NULL;

    return ajSqlstatementNewRun(dbc->Sqlconnection, statement);
}




/* @func ensDatabaseconnectionSqlstatementDel *********************************
**
** Delete an AJAX SQL Statement associated with an
** Ensembl Database Connection.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [d] Psqls [AjPSqlstatement*] AJAX SQL Statement address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseconnectionSqlstatementDel(
    EnsPDatabaseconnection dbc,
    AjPSqlstatement *Psqls)
{
    if(!dbc)
        return ajFalse;

    if(!Psqls)
        return ajFalse;

    if(ajDebugTest("ensDatabaseconnectionSqlstatementDel"))
        ajDebug("ensDatabaseconnectionSqlstatementDel\n"
                "  dbc %p\n"
                "  Psqls %p\n",
                dbc,
                Psqls);

    ajSqlstatementDel(Psqls);

    *Psqls = NULL;

    if(dbc->AutoDisconnect)
        ensDatabaseconnectionDisconnect(dbc);

    return ajTrue;
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

    if(!Ptxt)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(ajDebugTest("ensDatabaseconnectionEscapeC"))
    {
        ajDebug("ensDatabaseconnectionEscapeC\n"
                "  dbc %p\n"
                "  Ptxt %p\n"
                "  str '%S'\n",
                dbc,
                Ptxt,
                str);

        ensDatabaseconnectionTrace(dbc, 1);
    }

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

    if(!Pstr)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(ajDebugTest("ensDatabaseconnectionEscapeS"))
    {
        ajDebug("ensDatabaseconnectionEscapeS\n"
                "  dbc %p\n"
                "  Pstr %p\n"
                "  str '%S'\n",
                dbc,
                Pstr,
                str);

        ensDatabaseconnectionTrace(dbc, 1);
    }

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
            "%S  SqlconnectionClient '%s'\n"
            "%S  UserName '%S'\n"
            "%S  Password '***'\n"
            "%S  HostName '%S'\n"
            "%S  HostPort '%S'\n"
            "%S  SocketFile '%S'\n"
            "%S  DatabaseName '%S'\n"
            "%S  Use %u\n",
            indent, dbc,
            indent, dbc->Sqlconnection,
            indent, ajSqlconnectionClientToChar(dbc->SqlconnectionClient),
            indent, dbc->UserName,
            indent,
            indent, dbc->HostName,
            indent, dbc->HostPort,
            indent, dbc->SocketFile,
            indent, dbc->DatabaseName,
            indent, dbc->Use);

    ajSqlconnectionTrace(dbc->Sqlconnection, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}

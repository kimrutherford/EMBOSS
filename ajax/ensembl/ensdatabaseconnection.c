/* @source ensdatabaseconnection **********************************************
**
** Ensembl Database Connection functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.37 $
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
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

#include "ensdatabaseconnection.h"




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




/* @filesection ensdatabaseconnection *****************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPDatabaseconnection] Ensembl Database Connection **********
**
** @nam2rule Databaseconnection Functions for manipulating
** Ensembl Database Connection objects
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection
** @cc CVS Revision: 1.79
** @cc CVS Tag: branch-ensembl-68
**
** NOTE: The Perl API also supports the ORACLE, ODBC and Sybase SQL client
** libraries.
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Database Connection by pointer.
** It is the responsibility of the user to first destroy any previous
** Database Connection. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
** @nam4rule Url Constructor with a Uniform Resource Locator
**
** @argrule Cpy dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @argrule Cpy database [AjPStr] SQL database name (optional)
** @argrule Ini client [AjESqlconnectionClient] AJAX SQL Connection client
** @argrule Ini user [AjPStr] User name
** @argrule Ini password [AjPStr] Password
** @argrule Ini host [AjPStr] Host name or IP address
** @argrule Ini port [AjPStr] Host TCP/IP port
** @argrule Ini socketfile [AjPStr] UNIX socket file
** @argrule Ini database [AjPStr] SQL database name
** @argrule Ini dbctimeout [ajuint]
** Timeout in seconds for idle, non-interactive connections
** @argrule Ref dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @argrule Url url [const AjPStr] Uniform Resource Locator
**
** @valrule * [EnsPDatabaseconnection] Ensembl Database Connection or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseconnectionNewCpy ******************************************
**
** Construct an Ensembl Database Connection on an already existing connection.
** Optionally, a database name may be provided to connect to a different
** database on the same SQL server using the same SQL account information.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::new
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [uN] database [AjPStr] SQL database name (optional)
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNewCpy(
    EnsPDatabaseconnection dbc,
    AjPStr database)
{
    EnsPDatabaseconnection pthis = NULL;

    if (!dbc)
        return NULL;

    if (ajDebugTest("ensDatabaseconnectionNewCpy"))
    {
        ajDebug("ensDatabaseconnectionNewCpy\n"
                "  dbc %p\n"
                "  database '%S'\n",
                dbc,
                database);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    AJNEW0(pthis);

    pthis->Sqlconnectionclient = dbc->Sqlconnectionclient;

    if (dbc->Username)
        pthis->Username = ajStrNewRef(dbc->Username);

    if (dbc->Password)
        pthis->Password = ajStrNewRef(dbc->Password);

    if (dbc->Hostname)
        pthis->Hostname = ajStrNewRef(dbc->Hostname);

    if (dbc->Hostport)
        pthis->Hostport = ajStrNewRef(dbc->Hostport);

    if (dbc->Socketfile)
        pthis->Socketfile = ajStrNewRef(dbc->Socketfile);

    if (database && ajStrGetLen(database))
        pthis->Databasename = ajStrNewRef(database);
    else
    {
        if (dbc->Databasename)
            pthis->Databasename = ajStrNewRef(dbc->Databasename);
    }

    pthis->Timeout = dbc->Timeout;

    pthis->Use = 1U;

    return pthis;
}




/* @func ensDatabaseconnectionNewIni ******************************************
**
** Constructor for an Ensembl Database Connection with initial values.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::new
** @param [u] client [AjESqlconnectionClient] AJAX SQL Connection client
** @param [uN] user [AjPStr] User name
** @param [uN] password [AjPStr] Password
** @param [uN] host [AjPStr] Host name or IP address
** @param [uN] port [AjPStr] Host TCP/IP port
** @param [uN] socketfile [AjPStr] UNIX socket file
** @param [uN] database [AjPStr] SQL database name
** @param [rN] dbctimeout [ajuint]
** Timeout in seconds for idle, non-interactive connections
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNewIni(
    AjESqlconnectionClient client,
    AjPStr user,
    AjPStr password,
    AjPStr host,
    AjPStr port,
    AjPStr socketfile,
    AjPStr database,
    ajuint dbctimeout)
{
    EnsPDatabaseconnection dbc = NULL;

    if (!client)
        return NULL;

    if (ajDebugTest("ensDatabaseconnectionNewIni"))
        ajDebug("ensDatabaseconnectionNewIni\n"
                "  client %d\n"
                "  user '%S'\n"
                "  password '***'\n"
                "  host '%S'\n"
                "  port '%S'\n"
                "  socketfile '%S'\n"
                "  database '%S'\n"
                "  dbctimeout %u\n",
                client,
                user,
                host,
                port,
                socketfile,
                database,
                dbctimeout);

    AJNEW0(dbc);

    dbc->Sqlconnectionclient = client;

    if (user)
        dbc->Username = ajStrNewRef(user);

    if (password)
        dbc->Password = ajStrNewRef(password);

    if (host)
        dbc->Hostname = ajStrNewRef(host);

    if (port)
        dbc->Hostport = ajStrNewRef(port);

    if (socketfile)
        dbc->Socketfile = ajStrNewRef(socketfile);

    if (database)
        dbc->Databasename = ajStrNewRef(database);

    dbc->Timeout = dbctimeout;

    dbc->Use = 1U;

    return dbc;
}




/* @func ensDatabaseconnectionNewRef ******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseconnectionNewRef(EnsPDatabaseconnection dbc)
{
    if (!dbc)
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
**
** @release 6.3.0
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

    if (ajDebugTest("ensDatabaseconnectionNewUrl"))
        ajDebug("ensDatabaseconnectionNewUrl\n"
                "  url '%S'\n",
                url);

    if ((url == NULL) || (ajStrGetLen(url) == 0))
        return NULL;

    urlre = ajRegCompC("([^:]+)://(?:([^@:]+)(?:\\:([^@]*))?@)?"
                       "([^:/]+)(?:\\:)?(\\d+)?(?:\\/(\\w+))?");

    if (ajRegExec(urlre, url))
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

        if (!eclient)
            ajDebug("ensDatabaseconnectionNewUrl encountered "
                    "unexpected string '%S' in the "
                    "client part of the URL '%S'.\n",
                    client, url);

        dbc = ensDatabaseconnectionNewIni(eclient,
                                          user,
                                          password,
                                          host,
                                          port,
                                          (AjPStr) NULL,
                                          database,
                                          0);

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
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Del Destroy (free) an Ensembl Database Connection
**
** @argrule * Pdbc [EnsPDatabaseconnection*]
** Ensembl Database Connection address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseconnectionDel *********************************************
**
** Default destructor for an Ensembl Database Connection.
**
** Before freeing memory, the AJAX SQL Connection to the SQL RDBMS instance
** is dropped.
**
** @param [d] Pdbc [EnsPDatabaseconnection*]
** Ensembl Database Connection address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensDatabaseconnectionDel(EnsPDatabaseconnection *Pdbc)
{
    EnsPDatabaseconnection pthis = NULL;

    if (!Pdbc)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensDatabaseconnectionDel"))
    {
        ajDebug("ensDatabaseconnectionDel\n"
                "  *Pdbc %p\n",
                *Pdbc);

        ensDatabaseconnectionTrace(*Pdbc, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pdbc) || --pthis->Use)
    {
        *Pdbc = NULL;

        return;
    }

    ajSqlconnectionDel(&pthis->Sqlconnection);

    ajStrDel(&pthis->Username);
    ajStrDel(&pthis->Password);
    ajStrDel(&pthis->Hostname);
    ajStrDel(&pthis->Hostport);
    ajStrDel(&pthis->Socketfile);
    ajStrDel(&pthis->Databasename);

    ajMemFree((void **) Pdbc);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Get Return Database Connection attribute(s)
** @nam4rule Autodisconnect Return the auto disconnect flag
** @nam4rule Databasename Return the database name
** @nam4rule Hostname Return the host name
** @nam4rule Hostport Return host port
** @nam4rule Password Return the password
** @nam4rule Socketfile Return the UNIX socket file
** @nam4rule Sqlconnection Return the AJAX SQL Connection
** @nam4rule Sqlconnectionclient Return the AJAX SQL Connection client
** @nam4rule Timeout Return the timeout
** @nam4rule Username Return the user name
**
** @argrule * dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule Autodisconnect [AjBool] Auto disconnect flag or ajFalse
** @valrule Databasename [AjPStr] Database name or NULL
** @valrule Hostname [AjPStr] Host name or NULL
** @valrule Hostport [AjPStr] Host port or NULL
** @valrule Password [AjPStr] Password or NULL
** @valrule Socketfile [AjPStr] UNIX socket file or NULL
** @valrule Sqlconnection [AjPSqlconnection] AJAX SQL Connection or NULL
** @valrule Sqlconnectionclient [AjESqlconnectionClient]
** AJAX SQL Connection Client enumeration or ajESqlconnectionClientNULL
** @valrule Timeout [ajuint] Timeout or 0U
** @valrule Username [AjPStr] User name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseconnectionGetAutodisconnect *******************************
**
** Get the auto disconnect member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::disconnect_when_inactive
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] Auto disconnect or ajFalse
**                  ajTrue: The Ensembl Database Connection will automatically
**                          disconnect, i.e. delete the AJAX SQL Connection,
**                          if no AJAX SQL Statement is active.
**                  ajFalse: No automatic disconnects will occur.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionGetAutodisconnect(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Autodisconnect : ajFalse;
}




/* @func ensDatabaseconnectionGetDatabasename *********************************
**
** Get the database name member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::dbname
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Database name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetDatabasename(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Databasename : NULL;
}




/* @func ensDatabaseconnectionGetHostname *************************************
**
** Get the host name member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::host
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Host name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetHostname(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Hostname : NULL;
}




/* @func ensDatabaseconnectionGetHostport *************************************
**
** Get the host port member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::port
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Host port or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetHostport(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Hostport : NULL;
}




/* @func ensDatabaseconnectionGetPassword *************************************
**
** Get the password member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::password
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] Password or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetPassword(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Password : NULL;
}




/* @func ensDatabaseconnectionGetSocketfile ***********************************
**
** Get the UNIX socket file member of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] UNIX socket file or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetSocketfile(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Socketfile : NULL;
}




/* @func ensDatabaseconnectionGetSqlconnection ********************************
**
** Get the AJAX SQL Connection member of an Ensembl Database Connection.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPSqlconnection] AJAX SQL Connection or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPSqlconnection ensDatabaseconnectionGetSqlconnection(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Sqlconnection : NULL;
}




/* @func ensDatabaseconnectionGetSqlconnectionclient **************************
**
** Get the AJAX SQL Connection Client enumeration member of an
** Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::driver
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjESqlconnectionClient]
** AJAX SQL Connection Client enumeration or ajESqlconnectionClientNULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjESqlconnectionClient ensDatabaseconnectionGetSqlconnectionclient(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Sqlconnectionclient : ajESqlconnectionClientNULL;
}




/* @func ensDatabaseconnectionGetTimeout **************************************
**
** Get the timeout member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::timeout
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [ajuint] Timeout or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensDatabaseconnectionGetTimeout(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Timeout : 0U;
}




/* @func ensDatabaseconnectionGetUsername *************************************
**
** Get the user name member of an Ensembl Database Connection.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::username
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjPStr] User name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensDatabaseconnectionGetUsername(
    const EnsPDatabaseconnection dbc)
{
    return (dbc) ? dbc->Username : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Set Set one member of an Ensembl Database Connection
** @nam4rule Autodisconnect Set the automatic disconnect flag
**
** @argrule * dbc [EnsPDatabaseconnection] Ensembl Database Connection object
** @argrule Autodisconnect autodisconnect [AjBool] Auto disconnect flag
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDatabaseconnectionSetAutodisconnect *******************************
**
** Set the auto disconnect member of an Ensembl Database Connection.
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionSetAutodisconnect(
    EnsPDatabaseconnection dbc,
    AjBool autodisconnect)
{
    if (!dbc)
        return ajFalse;

    dbc->Autodisconnect = autodisconnect;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Connection object.
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Trace Report Ensembl Database Connection members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionTrace(const EnsPDatabaseconnection dbc,
                                  ajuint level)
{
    AjPStr indent = NULL;

    if (!dbc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensDatabaseconnectionTrace %p\n"
            "%S  Sqlconnection %p\n"
            "%S  Sqlconnectionclient '%s'\n"
            "%S  Username '%S'\n"
            "%S  Password '***'\n"
            "%S  Hostname '%S'\n"
            "%S  Hostport '%S'\n"
            "%S  Socketfile '%S'\n"
            "%S  Databasename '%S'\n"
            "%S  Use %u\n",
            indent, dbc,
            indent, dbc->Sqlconnection,
            indent, ajSqlconnectionClientToChar(dbc->Sqlconnectionclient),
            indent, dbc->Username,
            indent,
            indent, dbc->Hostname,
            indent, dbc->Hostport,
            indent, dbc->Socketfile,
            indent, dbc->Databasename,
            indent, dbc->Use);

    ajSqlconnectionTrace(dbc->Sqlconnection, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section test **************************************************************
**
** Functions for testing Ensembl Database Connection objects
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Is Ensembl Database Connection has a property
** @nam4rule Connected Test whether an active AJAX SQL Connection exists
**
** @argrule * dbc [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule Connected [AjBool] ajTrue if connected
**
** @fcategory use
******************************************************************************/




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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionIsConnected(const EnsPDatabaseconnection dbc)
{
    return (dbc && dbc->Sqlconnection) ? ajTrue : ajFalse;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Database Connections
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Match Compare two Ensembl Database Connections
**
** @argrule * dbc1 [const EnsPDatabaseconnection] Ensembl Database Connection
** @argrule * dbc2 [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule * [AjBool] ajTrue on success
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseconnectionMatch *******************************************
**
** Tests for matching two Ensembl Database Connections.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::equals
** @param [r] dbc1 [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] dbc2 [const EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue if the Ensembl Database Connection objects match
**
** @release 6.2.0
** @@
** The comparison is based on initial pointer equality and if that fails a
** direct comparison of Ensembl Database Connection members.
******************************************************************************/

AjBool ensDatabaseconnectionMatch(const EnsPDatabaseconnection dbc1,
                                  const EnsPDatabaseconnection dbc2)
{
    if (!dbc1)
        return ajFalse;

    if (!dbc2)
        return ajFalse;

    if (dbc1 == dbc2)
        return ajTrue;

    /*
    ** The AJAX SQL Connection is not tested as it can be disconnected at any
    ** time and the database name is most likely to be different and
    ** therefore tested first. String matches are rather expensive...
    */

    if (!ajStrMatchS(dbc1->Databasename, dbc2->Databasename))
        return ajFalse;

    if (!ajStrMatchS(dbc1->Username, dbc2->Username))
        return ajFalse;

    if (!ajStrMatchS(dbc1->Password, dbc2->Password))
        return ajFalse;

    if (!ajStrMatchS(dbc1->Hostname, dbc2->Hostname))
        return ajFalse;

    if (!ajStrMatchS(dbc1->Hostport, dbc2->Hostport))
        return ajFalse;

    if (!ajStrMatchS(dbc1->Socketfile, dbc2->Socketfile))
        return ajFalse;

    if (dbc1->Sqlconnectionclient != dbc2->Sqlconnectionclient)
        return ajFalse;

    return ajTrue;
}




/* @section connection ********************************************************
**
** Functions for connecting and disconnecting Ensembl Database Connections
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Connect Connect an Ensembl Database Connection
** @nam3rule Disconnect Disconnect an Ensembl Database Connection
**
** @argrule * dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @valrule * [AjBool] True on success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseconnectionConnect *****************************************
**
** Connect an Ensembl Database Connection to the specified SQL database.
**
** @cc Bio::EnsEMBL::DBSQL::DBConnection::connect
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionConnect(EnsPDatabaseconnection dbc)
{
    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    if (!dbc)
        return ajFalse;

    if (dbc->Sqlconnection)
        return ajTrue;

    if (ajDebugTest("ensDatabaseconnectionConnect"))
        ajDebug("ensDatabaseconnectionConnect\n"
                "  dbc %p\n",
                dbc);

    dbc->Sqlconnection = ajSqlconnectionNewData(dbc->Sqlconnectionclient,
                                                dbc->Username,
                                                dbc->Password,
                                                dbc->Hostname,
                                                dbc->Hostport,
                                                dbc->Socketfile,
                                                dbc->Databasename);

    if (!dbc->Sqlconnection)
    {
        ajWarn("Could not establish an SQL connection for user '%S' "
               "to host '%S' at port '%S' for database '%S'.\n",
               dbc->Username,
               dbc->Hostname,
               dbc->Hostport,
               dbc->Databasename);

        return ajFalse;
    }

    /* NOTE: Setting a connection timeout is MySQL-specific for the moment. */

    if ((dbc->Timeout > 0)
        &&
        (dbc->Sqlconnectionclient == ajESqlconnectionClientMySQL))
    {
        statement = ajFmtStr("SET SESSION wait_timeout=%u", dbc->Timeout);

        sqls = ajSqlstatementNewRun(dbc->Sqlconnection, statement);

        ajSqlstatementDel(&sqls);

        ajStrDel(&statement);
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionDisconnect(EnsPDatabaseconnection dbc)
{
    if (ajDebugTest("ensDatabaseconnectionDisconnect"))
    {
        ajDebug("ensDatabaseconnectionDisconnect\n"
                "  dbc %p\n",
                dbc);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if (!dbc)
        return ajFalse;

    if (!dbc->Sqlconnection)
        return ajTrue;

    /*
    ** Disconnect if no other AJAX SQL Statement has a reference to the
    ** AJAX SQL Connection, except this Ensembl Database Connection object.
    */

    if (ajSqlconnectionGetUse(dbc->Sqlconnection) == 1)
        ajSqlconnectionDel(&dbc->Sqlconnection);

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Database Connection convenience functions
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Escape Escape a string
** @nam4rule C Escape to an AJAX String
** @nam4rule S Escape to a C-type character string
** @nam3rule Sqlstatement AJAX SQL Statement
** @nam4rule Del Delete an AJAX SQL Statement
** @nam4rule New Run a new AJAX SQL statement
**
** @argrule * dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @argrule EscapeC Ptxt [char**] Address of the (new) SQL-escaped C string
** @argrule EscapeC str [const AjPStr] AJAX String to be escaped
** @argrule EscapeS Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @argrule EscapeS str [const AjPStr] AJAX String to be escaped
** @argrule SqlstatementDel Psqls [AjPSqlstatement*] AJAX SQL Statement address
** @argrule SqlstatementNew statement [const AjPStr] SQL statement
**
** @valrule EscapeC [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule EscapeS [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule SqlstatementDel [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule SqlstatementNew [AjPSqlstatement] AJAX SQL Statement
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseconnectionEscapeC *****************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return a C-type character string.
**
** The caller is responsible for deleting the escaped C-type character string.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionEscapeC(EnsPDatabaseconnection dbc,
                                    char **Ptxt,
                                    const AjPStr str)
{
    if (!dbc)
        return ajFalse;

    if (!Ptxt)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensDatabaseconnectionEscapeC"))
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

    if (!ensDatabaseconnectionIsConnected(dbc))
        if (!ensDatabaseconnectionConnect(dbc))
            return ajFalse;

    return ajSqlconnectionEscapeC(dbc->Sqlconnection, Ptxt, str);
}




/* @func ensDatabaseconnectionEscapeS *****************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return an AJAX String.
**
** The caller is responsible for deleting the escaped AJAX String.
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionEscapeS(EnsPDatabaseconnection dbc,
                                    AjPStr *Pstr,
                                    const AjPStr str)
{
    if (!dbc)
        return ajFalse;

    if (!Pstr)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensDatabaseconnectionEscapeS"))
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

    if (!ensDatabaseconnectionIsConnected(dbc))
        if (!ensDatabaseconnectionConnect(dbc))
            return ajFalse;

    return ajSqlconnectionEscapeS(dbc->Sqlconnection, Pstr, str);
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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionSqlstatementDel(
    EnsPDatabaseconnection dbc,
    AjPSqlstatement *Psqls)
{
    if (!dbc)
        return ajFalse;

    if (!Psqls)
        return ajFalse;

    if (ajDebugTest("ensDatabaseconnectionSqlstatementDel"))
        ajDebug("ensDatabaseconnectionSqlstatementDel\n"
                "  dbc %p\n"
                "  Psqls %p\n",
                dbc,
                Psqls);

    ajSqlstatementDel(Psqls);

    *Psqls = NULL;

    if (dbc->Autodisconnect)
        ensDatabaseconnectionDisconnect(dbc);

    return ajTrue;
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjPSqlstatement ensDatabaseconnectionSqlstatementNew(
    EnsPDatabaseconnection dbc,
    const AjPStr statement)
{
    if (ajDebugTest("ensDatabaseconnectionSqlstatementNew"))
    {
        ajDebug("ensDatabaseconnectionSqlstatementNew\n"
                "  dbc %p\n"
                "  statement '%S'\n",
                dbc,
                statement);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if (!dbc)
        return NULL;

    if (!ensDatabaseconnectionIsConnected(dbc))
        if (!ensDatabaseconnectionConnect(dbc))
            return NULL;

    return ajSqlstatementNewRun(dbc->Sqlconnection, statement);
}




/* @section retrieval *********************************************************
**
** Ensembl Database Connection retrieval functions
**
** @fdata [EnsPDatabaseconnection]
**
** @nam3rule Fetch Fetch an object from an Ensembl Database Connection
** @nam4rule Url Fetch a Uniform Resource Locator representation
**
** @argrule * dbc [const EnsPDatabaseconnection] Ensembl Database Connection
** @argrule FetchUrl Purl [AjPStr*] Uniform Resource Locator
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseconnectionFetchUrl ****************************************
**
** Fetch a Uniform Resource Locator representation of an
** Ensembl Database Connection.
**
** NOTE: The URL contains the password in text form so fetching the URL is a
** security risk.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [wP] Purl [AjPStr*] Uniform Resource Locator
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseconnectionFetchUrl(const EnsPDatabaseconnection dbc,
                                     AjPStr *Purl)
{
    if (!dbc)
        return ajFalse;

    if (!Purl)
        return ajFalse;

    if (*Purl)
        ajStrAssignClear(Purl);
    else
        *Purl = ajStrNew();

    if (dbc->Socketfile && ajStrGetLen(dbc->Socketfile))
    {
        ajStrAppendC(Purl, "file:///");
        ajStrAppendS(Purl, dbc->Socketfile);
    }
    else
    {
        ajStrAppendC(Purl,
                     ajSqlconnectionClientToChar(dbc->Sqlconnectionclient));
        ajStrAppendC(Purl, "://");

        if ((dbc->Username != NULL) && (ajStrGetLen(dbc->Username) > 0))
        {
            ajStrAppendS(Purl, dbc->Username);

            if ((dbc->Password != NULL) && (ajStrGetLen(dbc->Password) > 0))
            {
                ajStrAppendC(Purl, ":");
                ajStrAppendS(Purl, dbc->Password);
            }

            ajStrAppendC(Purl, "@");
        }

        ajStrAppendS(Purl, dbc->Hostname);

        if ((dbc->Hostport != NULL) && (ajStrGetLen(dbc->Hostport) > 0))
        {
            ajStrAppendC(Purl, ":");
            ajStrAppendS(Purl, dbc->Hostport);
        }

        ajStrAppendC(Purl, "/");

        if ((dbc->Databasename != NULL) && (ajStrGetLen(dbc->Databasename) > 0))
            ajStrAppendS(Purl, dbc->Databasename);
    }

    return ajTrue;
}

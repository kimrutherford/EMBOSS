/******************************************************************************
** @source AJAX SQL functions
**
** @author Copyright (C) 2006 Michael K. Schuster
** @version 1.0
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

#include "ajax.h"

#ifdef HAVE_MYSQL
#ifdef WIN32
#include <windows.h>
#endif /* WIN32 */
#include "mysql.h"
#endif /* HAVE_MYSQL */

#ifdef HAVE_POSTGRESQL
#include "libpq-fe.h"
#endif /* HAVE_POSTGRESQL */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

/* sqlInit ********************************************************************
**
** Private boolean variable to ascertain that ajSqlInit has been called once
** and only once.
**
******************************************************************************/

static AjBool sqlInit = AJFALSE;




/* sqlconectionClient *********************************************************
**
** AJAX SQL Connection client library enumeration. The following strings are
** used for conversion in database operations and correspond to
** AjESqlconnectionClient.
**
******************************************************************************/

static const char *sqlconnectionClient[] =
{
    NULL,
    "mysql",
    "postgresql",
    NULL
};




#ifdef AJ_SAVESTATS

static ajlong sqlconnectionTotalCount = 0;
static ajlong sqlconnectionFreeCount  = 0;
static ajlong sqlconnectionErrorCount = 0;
static ajlong sqlstatementTotalCount  = 0;
static ajlong sqlstatementFreeCount   = 0;
static ajlong sqlstatementErrorCount  = 0;

#endif /* AJ_SAVESTATS */

#ifdef HAVE_MYSQL

static AjPSqlconnection sqlconnectionMysqlNewData(
    const AjPStr user,
    const AjPStr password,
    const AjPStr host,
    const AjPStr port,
    const AjPStr socketfile,
    const AjPStr database,
    AjBool debug);

static AjPSqlstatement sqlstatementMysqlNewRun(AjPSqlconnection sqlc,
                                               const AjPStr statement,
                                               AjBool debug);

#endif /* HAVE_MYSQL */

#ifdef HAVE_POSTGRESQL

static AjPSqlconnection sqlconnectionPostgresqlNewData(
    const AjPStr user,
    const AjPStr password,
    const AjPStr host,
    const AjPStr port,
    const AjPStr socketfile,
    const AjPStr database,
    AjBool debug);

static AjPSqlstatement sqlstatementPostgresqlNewRun(AjPSqlconnection sqlc,
                                                    const AjPStr statement,
                                                    AjBool debug);

#endif /* HAVE_POSTGRESQL */

static AjBool arrVoidResize(AjPVoid *thys, ajuint size);




/* @filesection ajsql *********************************************************
**
** @nam1rule aj Function belongs to the AJAX library
** @nam2rule Sql SQL interface to MySQL or Postgres
**
******************************************************************************/




/* @datasection [none] Internals **********************************************
**
** Function is for setup or control of internals
**
**
******************************************************************************/




/* @section initialization ****************************************************
**
** @fdata [none]
** @fcategory internals
**
** @nam3rule Init Initialize SQL internals
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSqlInit ************************************************************
**
** Initialises implementation-specific SQL client libraries.
**
** @return [AjBool] ajTrue if the initialisation was successful.
** @@
******************************************************************************/

AjBool ajSqlInit(void)
{
    if(sqlInit)
        return ajTrue;

#ifdef HAVE_MYSQL

    if(mysql_library_init(0, (char **) NULL, (char **) NULL))
    {
        ajDebug("ajSqlInit MySQL initialisation failed.\n");

        return ajFalse;
    }
    else
        ajDebug("ajSqlInit MySQL client library %s\n",
                mysql_get_client_info());

#endif /* HAVE_MYSQL */

    sqlInit = ajTrue;

    return ajTrue;
}




/* @section exiting ***********************************************************
**
** @fdata [none]
** @fcategory internals
**
** @nam3rule Exit Initialize SQL internals
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSqlExit ************************************************************
**
** Finalises implementation-specific SQL client libraries.
**
** @return [void]
** @@
******************************************************************************/

void ajSqlExit(void)
{
    if(!sqlInit)
        return;

#ifdef HAVE_MYSQL

    mysql_library_end();

#endif /* HAVE_MYSQL */

#ifdef AJ_SAVESTATS

    ajDebug("SQL Connection usage: "
            "%Ld opened, %Ld closed, %Ld in use, %Ld failed\n",
            sqlconnectionTotalCount,  sqlconnectionFreeCount,
            sqlconnectionTotalCount - sqlconnectionFreeCount,
            sqlconnectionErrorCount);

    ajDebug("SQL Statement usage: "
            "%Ld opened, %Ld closed, %Ld in use, %Ld failed\n",
            sqlstatementTotalCount,  sqlstatementFreeCount,
            sqlstatementTotalCount - sqlstatementFreeCount,
            sqlstatementErrorCount);

#endif /* AJ_SAVESTATS */

    sqlInit = ajFalse;

    return;
}




/* @datasection [AjPSqlconnection] SQL Connection *****************************
**
** Functions for manipulating AJAX SQL Connections.
**
** @nam2rule Sqlconnection Functions for manipulating AJAX SQL Connections.
**
******************************************************************************/




#ifdef HAVE_MYSQL
/* @funcstatic sqlconnectionMysqlNewData **************************************
**
** MySQL client library-specific AJAX SQL Connection constructor, which also
** constructs a client library-specific (MYSQL *) connection object.
**
** Configuration options will be read from the [client] and [EMBOSS] groups
** of the default my.cnf options file.
**
** A connection to a MySQL server is established.
**
** @param [r] user [const AjPStr] SQL account user name
** @param [r] password [const AjPStr] SQL account password
** @param [r] host [const AjPStr] SQL server hostname or IP address
** @param [r] port [const AjPStr] SQL server port number
** @param [r] socketfile [const AjPStr] SQL server UNIX socket file name
**                   MySQL: Absolute path to the socket file.
** @param [r] database [const AjPStr] SQL database name
** @param [r] debug [AjBool] Debug mode
**
** @return [AjPSqlconnection] AJAX SQL Connection or NULL
** @@
******************************************************************************/

static AjPSqlconnection sqlconnectionMysqlNewData(
    const AjPStr user,
    const AjPStr password,
    const AjPStr host,
    const AjPStr port,
    const AjPStr socketfile,
    const AjPStr database,
    AjBool debug)
{
    unsigned long clientflag = 0;

    ajuint portnumber = 0;

    AjPSqlconnection sqlc = NULL;

    MYSQL *Pmysql = NULL;

    debug |= ajDebugTest("sqlconnectionMysqlNewData");

    if(!ajStrToUint(port, &portnumber))
    {
        ajWarn("sqlconnectionMysqlNewData could not parse port '%S' into an "
               "AJAX unsigned integer value.", port);

        return NULL;
    }

    Pmysql = mysql_init(Pmysql);

    if(Pmysql == NULL)
    {
        ajWarn("sqlconnectionMysqlNewData MySQL connection object "
               "initialisation via mysql_init failed.\n");

        return NULL;
    }

    /*
    ** Read options from the [client] and [EMBOSS] groups of the
    ** default my.cnf options file.
    */

    mysql_options(Pmysql, MYSQL_READ_DEFAULT_GROUP, "EMBOSS");

    if(mysql_real_connect(Pmysql,
                          ajStrGetPtr(host),
                          ajStrGetPtr(user),
                          ajStrGetPtr(password),
                          ajStrGetPtr(database),
                          (unsigned int) portnumber,
                          ajStrGetPtr(socketfile),
                          clientflag))
    {
        /* The connection was successful. */

        AJNEW0(sqlc);

        sqlc->Pconnection = (void *) Pmysql;

        sqlc->Client = ajESqlconnectionClientMySQL;

        sqlc->Use = 1;

        if(debug)
            ajDebug("sqlconnectionMysqlNewData established a "
                    "MySQL connection to server '%S' on port '%S' (%d) "
                    "as user '%S' for database '%S'.\n",
                    host, port, portnumber, user, database);
    }
    else
    {
        /* The connection was not successful. */

        ajDebug("sqlconnectionMysqlNewData could not establish a "
                "MySQL connection to server '%S' on port '%S' (%d) "
                "as user '%S' for database '%S'.\n"
                "  MySQL error: %s\n",
                host, port, portnumber, user, database,
                mysql_error(Pmysql));

        mysql_close(Pmysql);

#ifdef AJ_SAVESTATS

        sqlconnectionErrorCount++;

#endif /* AJ_SAVESTATS */
    }

    return sqlc;
}

#endif /* HAVE_MYSQL */




#ifdef HAVE_POSTGRESQL
/* @funcstatic sqlconnectionPostgresqlNewData *********************************
**
** PostgreSQL client library-specific AJAX SQL Connection constructor, which
** also constructs a client library-specific (PGconn *) connection object.
**
** A connection to a PostgreSQL server is established.
**
** @param [r] user [const AjPStr] SQL account user name
** @param [r] password [const AjPStr] SQL account password
** @param [r] host [const AjPStr] SQL server hostname or IP address
** @param [r] port [const AjPStr] SQL server port number
** @param [r] socketfile [const AjPStr] SQL server UNIX socket file
**                   PostgreSQL: Absolute path to the socket directory only.
**                     Socket file names are then generated from this directory
**                     information and the port number above.
**                     See "%s/.s.PGSQL.%d" in macro UNIXSOCK_PATH in source
**                     file pgsql/src/include/libpq/pqcomm.h
** @param [r] database [const AjPStr] SQL database name
** @param [r] debug [AjBool] Debug mode
**
** @return [AjPSqlconnection] AJAX SQL Connection or NULL
** @@
******************************************************************************/

static AjPSqlconnection sqlconnectionPostgresqlNewData(
    const AjPStr user,
    const AjPStr password,
    const AjPStr host,
    const AjPStr port,
    const AjPStr socketfile,
    const AjPStr database,
    AjBool debug)
{
    AjPSqlconnection sqlc = NULL;

    AjPStr conninfo = NULL;
    AjPStr safeinfo = NULL;

    PGconn *Ppgconn = NULL;

    debug |= ajDebugTest("sqlconnectionPostgresqlNewData");

    conninfo = ajStrNew();
    safeinfo = ajStrNew();

    /* PostgreSQL needs escaping of ' and \ to \' and \\. */

    if(ajStrGetLen(user))
    {
        ajStrAssignS(&safeinfo, user);
        ajStrExchangeCC(&safeinfo, "'", "\'");
        ajStrExchangeCC(&safeinfo, "\\", "\\\\");

        ajFmtPrintAppS(&conninfo, "user = '%S' ", safeinfo);
    }

    if(ajStrGetLen(password))
    {
        ajStrAssignS(&safeinfo, password);
        ajStrExchangeCC(&safeinfo, "'", "\'");
        ajStrExchangeCC(&safeinfo, "\\", "\\\\");

        ajFmtPrintAppS(&conninfo, "password = '%S' ", safeinfo);
    }

    if(ajStrGetLen(host))
    {
        ajStrAssignS(&safeinfo, host);
        ajStrExchangeCC(&safeinfo, "'", "\'");
        ajStrExchangeCC(&safeinfo, "\\", "\\\\");

        ajFmtPrintAppS(&conninfo, "host = '%S' ", safeinfo);
    }

    if(ajStrGetLen(socketfile))
    {
        ajStrAssignS(&safeinfo, socketfile);
        ajStrExchangeCC(&safeinfo, "'", "\'");
        ajStrExchangeCC(&safeinfo, "\\", "\\\\");

        ajFmtPrintAppS(&conninfo, "host = '%S' ", safeinfo);
    }

    if(ajStrGetLen(port))
    {
        ajStrAssignS(&safeinfo, port);
        ajStrExchangeCC(&safeinfo, "'", "\'");
        ajStrExchangeCC(&safeinfo, "\\", "\\\\");

        ajFmtPrintAppS(&conninfo, "port = '%S' ", safeinfo);
    }

    if(ajStrGetLen(database))
    {
        ajStrAssignS(&safeinfo, database);
        ajStrExchangeCC(&safeinfo, "'", "\'");
        ajStrExchangeCC(&safeinfo, "\\", "\\\\");

        ajFmtPrintAppS(&conninfo, "dbname = '%S' ", safeinfo);
    }

    /*
    ** Other PQconnectdb conninfo parameters:
    **
    ** hostaddr:        Numeric IPv4 or IPv6 address of host to connect to.
    ** connect_timeout: Maximum wait for connection, in seconds.
    ** options:         Command line options to be sent to the server.
    ** sslmode:         disable, allow, prefer, require
    ** requiressl:      Deprecated use sslmode option instead.
    ** krbsrvname:      Kerberos5 service name.
    ** service:         Service name to use for additional parameters.
    */

    /*
    ** FIXME: sslmode should probably be configurable
    */
    ajFmtPrintAppS(&conninfo, "sslmode = 'disable'");

    ajStrDel(&safeinfo);

    Ppgconn = PQconnectdb(ajStrGetPtr(conninfo));

    ajStrDel(&conninfo);

    if(Ppgconn == NULL)
    {
        ajWarn("sqlconnectionPostgresqlNewData PostgreSQL connection object "
               "initialisation via PQconnectdb failed.\n");

        return NULL;
    }

    switch(PQstatus(Ppgconn))
    {
        case CONNECTION_OK:

            AJNEW0(sqlc);

            sqlc->Pconnection = (void *) Ppgconn;

            sqlc->Client = ajESqlconnectionClientPostgreSQL;

            sqlc->Use = 1;

            if(debug)
                ajDebug("sqlconnectionPostgresqlNewData established a "
                        "PostgreSQL connection to server '%S' on port '%S' "
                        "as user '%S' for database '%S'\n",
                        host, port, user, database);

            break;

        case CONNECTION_BAD:

            ajDebug("sqlconnectionPostgresqlNewData could not establish a "
                    "PostgreSQL connection to server '%S' on port '%S' "
                    "as user '%S' for database '%S'.\n"
                    "  PostgreSQL error: %s\n",
                    host, port, user, database,
                    PQerrorMessage(Ppgconn));

            PQfinish(Ppgconn);

#ifdef AJ_SAVESTATS

            sqlconnectionErrorCount++;

#endif /* AJ_SAVESTATS */

            break;

        default:

            ajDebug("sqlconnectionPostgresqlNewData got unexpected "
                    "PQstatus return value %d.\n", PQstatus(Ppgconn));
    }

    return sqlc;
}

#endif /* HAVE_POSTGRESQL */




/* @section constructors ******************************************************
**
** Functions for constructing AJAX SQL Connection objects.
**
** @fdata [AjPSqlconnection]
**
** @nam3rule New Construct a new AJAX SQL Connection
** @nam4rule NewData Constructor with set of initial values
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule NewData client [AjESqlconnectionClient] SQL client
** @argrule NewData user [const AjPStr] SQL account user name
** @argrule NewData password [const AjPStr] SQL account password
** @argrule NewData host [const AjPStr] SQL server hostname or IP address
** @argrule NewData port [const AjPStr] SQL server port number
** @argrule NewData socketfile [const AjPStr] SQL server UNIX socket file
**                   MySQL: Absolute path to the socket file.
**                   PostgreSQL: Absolute path to the socket directory only.
**                     Socket file names are then generated from this directory
**                     information and the port number above.
**                     See "%s/.s.PGSQL.%d" in macro UNIXSOCK_PATH in source
**                     file pgsql/src/include/libpq/pqcomm.h
** @argrule NewData database [const AjPStr] SQL database name
** @argrule NewRef  sqlc [AjPSqlconnection] AJAX SQL Connection
**
** @valrule * [AjPSqlconnection] AJAX SQL Connection
**
** @fcategory new
******************************************************************************/




/* @func ajSqlconnectionNewData ***********************************************
**
** Default AJAX SQL Connection constructor, which also allocates a client
** library-specific connection object.
**
** A connection to an SQL server is established.
**
** For MySQL clients options will be read from the [client] and [EMBOSS] groups
** of the default my.cnf options file.
**
** @param [u] client [AjESqlconnectionClient] SQL client
** @param [r] user [const AjPStr] SQL account user name
** @param [r] password [const AjPStr] SQL account password
** @param [r] host [const AjPStr] SQL server hostname or IP address
** @param [r] port [const AjPStr] SQL server port number
** @param [r] socketfile [const AjPStr] SQL server UNIX socket file
**                   MySQL: Absolute path to the socket file.
**                   PostgreSQL: Absolute path to the socket directory only.
**                     Socket file names are then generated from this directory
**                     information and the port number above.
**                     See "%s/.s.PGSQL.%d" in macro UNIXSOCK_PATH in source
**                     file pgsql/src/include/libpq/pqcomm.h
** @param [r] database [const AjPStr] SQL database name
**
** @return [AjPSqlconnection] AJAX SQL Connection or NULL
** @@
******************************************************************************/

AjPSqlconnection ajSqlconnectionNewData(AjESqlconnectionClient client,
                                        const AjPStr user,
                                        const AjPStr password,
                                        const AjPStr host,
                                        const AjPStr port,
                                        const AjPStr socketfile,
                                        const AjPStr database)
{
    AjBool debug = AJFALSE;

    AjPSqlconnection sqlc = NULL;

    debug = ajDebugTest("ajSqlconnectionNewData");

    if(debug)
        ajDebug("ajSqlconnectionNewData\n"
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

    (void) password;

    ajSqlInit();

    switch(client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            sqlc = sqlconnectionMysqlNewData(user,
                                             password,
                                             host,
                                             port,
                                             socketfile,
                                             database,
                                             debug);

#else

            ajDebug("ajSqlconnectionNewData EMBOSS AJAX library built without "
                    "MySQL client support.\n");

#endif /* HAVE_MYSQL */

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            sqlc = sqlconnectionPostgresqlNewData(user,
                                                  password,
                                                  host,
                                                  port,
                                                  socketfile,
                                                  database,
                                                  debug);

#else

            ajDebug("ajSqlconnectionNewData EMBOSS AJAX library built without "
                    "PostgreSQL client support.\n");

#endif /* HAVE_POSTGRESQL */

            break;

        default:

            ajDebug("ajSqlconnectionNewData SQL Connection client %d "
                    "not supported.\n",
                    client);
    }

#ifdef AJ_SAVESTATS

    if(sqlc)
        sqlconnectionTotalCount++;

#endif /* AJ_SAVESTATS */

    if(debug)
    {
        if(sqlc)
            ajDebug("ajSqlconnectionNewData connected.\n");
        else
            ajDebug("ajSqlconnectionNewData not connected.\n");
    }

    return sqlc;
}




/* @func ajSqlconnectionNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] sqlc [AjPSqlconnection] AJAX SQL Connection
**
** @return [AjPSqlconnection] AJAX SQL Connection or NULL
** @@
******************************************************************************/

AjPSqlconnection ajSqlconnectionNewRef(AjPSqlconnection sqlc)
{
    if(!sqlc)
        return NULL;

    sqlc->Use++;

    return sqlc;
}




/* @section destructors *******************************************************
**
** Functions for destruction of AJAX SQL Connection objects.
**
** @fdata [AjPSqlconnection]
**
** @nam3rule Del Destroy (free) an existing AJAX SQL Connection
**
** @argrule Del Psqlc [AjPSqlconnection*] AJAX SQL Connection address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ajSqlconnectionDel ***************************************************
**
** Default AJAX SQL Connection destructor, which disconnects from the
** SQL server first, frees the client library-specific connection object and
** finally the AJAX SQL Connection object.
**
** @param [d] Psqlc [AjPSqlconnection*] AJAX SQL Connection address
**
** @return [void]
** @@
******************************************************************************/

void ajSqlconnectionDel(AjPSqlconnection *Psqlc)
{
    AjBool debug = AJFALSE;

    AjPSqlconnection pthis = NULL;

    if(!Psqlc)
        return;

    if(!*Psqlc)
        return;

    debug = ajDebugTest("ajSqlconnectionDel");

    if(debug)
        ajDebug("ajSqlconnectionDel"
                "  *Psqlc %p\n",
                *Psqlc);

    pthis = *Psqlc;

    pthis->Use--;

    if(pthis->Use)
    {
        *Psqlc = NULL;

        return;
    }

    switch(pthis->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            mysql_close((MYSQL *) pthis->Pconnection);

            if(debug)
                ajDebug("ajSqlconnectionDel deleted MySQL connection.\n");

#else

            ajDebug("ajSqlconnectionDel got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_MYSQL */

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            PQfinish((PGconn *) pthis->Pconnection);

            if(debug)
                ajDebug("ajSqlconnectionDel deleted PostgreSQL connection.\n");

#else

            ajDebug("ajSqlconnectionDel got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_POSTGRESQL */

            break;

        default:

            ajDebug("ajSqlconnectionDel SQL Connection client %d "
                    "not supported.\n",
                    pthis->Client);
    }

    AJFREE(pthis);

    *Psqlc = NULL;

#ifdef AJ_SAVESTATS

    sqlconnectionFreeCount++;

#endif /* AJ_SAVESTATS */

    return;
}




/* @section Cast **************************************************************
**
** Functions for returning elements of an AJAX SQL Connection object.
**
** @fdata [AjPSqlconnection]
**
** @nam3rule Get Return AJAX SQL Connection elements
** @nam4rule Client Return client element
** @nam4rule Use Return the use counter element
** @nam3rule Escape Escape an AJAX String based on an AJAX SQL Connection
** @suffix C Return a char* escaped string
** @suffix S Return an AjPStr escaped string
**
** @argrule * sqlc [const AjPSqlconnection] AJAX SQL Connection
** @argrule C Ptxt [char**] Address of the (new) SQL-escaped C-type string
** @argrule S Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @argrule Escape str [const AjPStr] AJAX String to be escaped
**
** @valrule Client [AjESqlconnectionClient] Client library enumeration
** @valrule Use [ajuint] Use counter
** @valrule Escape [AjBool] True on success
**
** @fcategory cast
******************************************************************************/




/* @func ajSqlconnectionEscapeC ***********************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return a C-type character string.
**
** The caller is responsible for deleting the escaped C-type character string.
**
** @param [r] sqlc [const AjPSqlconnection] AJAX SQL Connection
** @param [w] Ptxt [char**] Address of the (new) SQL-escaped C-type string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlconnectionEscapeC(const AjPSqlconnection sqlc,
                              char **Ptxt,
                              const AjPStr str)
{

#ifdef HAVE_POSTGRESQL

    int error = 0;

#endif /* HAVE_POSTGRESQL */

#if defined(HAVE_MYSQL) || defined(HAVE_POSTGRESQL)

    size_t length = 0;

#endif /* defined(HAVE_MYSQL) || defined(HAVE_POSTGRESQL) */

    if(!sqlc)
        return ajFalse;

    if(!Ptxt)
        return ajFalse;

    if(!str)
        return ajFalse;

    switch(sqlc->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            /*
            ** At maximum the escaped string could be 2 * n + 1
            ** characters long.
            */

            length = ajStrGetLen(str);

            if(length >= LONG_MAX)
                ajFatal("ajSqlconnectionEscapeC exceeded the maximum length.");
            
            *Ptxt = ajCharNewRes(2 * length + 1);

            length = mysql_real_escape_string((MYSQL *) sqlc->Pconnection,
                                              *Ptxt,
                                              ajStrGetPtr(str),
                                              length);

#else

            ajDebug("ajSqlconnectionEscapeC got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_MYSQL */

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            /*
            ** At maximum the escaped string could be 2 * n + 1
            ** characters long.
            */

            length = ajStrGetLen(str);

            if(length >= LONG_MAX)
                ajFatal("ajSqlconnectionEscapeC exceeded the maximum length.");
            
            *Ptxt = ajCharNewRes(2 * length + 1);

            length = PQescapeStringConn((PGconn *) sqlc->Pconnection,
                                        *Ptxt,
                                        ajStrGetPtr(str),
                                        length,
                                        &error);

            if(error)
                ajDebug("ajSqlconnectionEscapeC PostgreSQL client encountered "
                        "an error calling PQescapeStringConn.\n"
                        "  PostgreSQL error: %s",
                        PQerrorMessage((PGconn *) sqlc->Pconnection));

#else

            ajDebug("ajSqlconnectionEscapeC got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_POSTGRESQL */

            break;

        default:

            ajDebug("ajSqlconnectionEscapeC SQL Connection client %d "
                    "not supported.\n",
                    sqlc->Client);
    }

    return ajTrue;
}




/* @func ajSqlconnectionEscapeS ***********************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return an AJAX String.
**
** The caller is responsible for deleting the escaped AJAX String.
**
** @param [r] sqlc [const AjPSqlconnection] AJAX SQL Connection
** @param [w] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlconnectionEscapeS(const AjPSqlconnection sqlc,
                              AjPStr *Pstr,
                              const AjPStr str)
{
    char *Ptxt = NULL;

    if(!sqlc)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(*Pstr)
        ajStrAssignClear(Pstr);
    else
        *Pstr = ajStrNew();

    ajSqlconnectionEscapeC(sqlc, &Ptxt, str);

    ajStrAssignC(Pstr, Ptxt);

    ajCharDel(&Ptxt);

    return ajTrue;
}




/* @func ajSqlconnectionGetClient *********************************************
**
** Get the client element of an AJAX SQL Connection.
**
** @param [r] sqlc [const AjPSqlconnection] AJAX SQL Connection
**
** @return [AjESqlconnectionClient] AJAX SQL Connection client or
**                                  ajESqlconnectionClientNULL
** @@
******************************************************************************/

AjESqlconnectionClient ajSqlconnectionGetClient(const AjPSqlconnection sqlc)
{
    if(!sqlc)
        return ajESqlconnectionClientNULL;

    return sqlc->Client;
}




/* @func ajSqlconnectionGetUse ************************************************
**
** Get the use counter element of an AJAX SQL Connection.
**
** @param [r] sqlc [const AjPSqlconnection] AJAX SQL Connection
**
** @return [ajuint] Use counter
** @@
******************************************************************************/

ajuint ajSqlconnectionGetUse(const AjPSqlconnection sqlc)
{
    if(!sqlc)
        return 0;

    return sqlc->Use;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an AJAX SQL Connection object.
**
** @fdata [AjPSqlconnection]
** @nam3rule Trace Report AJAX SQL Connection elements to debug file
**
** @argrule Trace sqlc [const AjPSqlconnection] AJAX SQL Connection
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ajSqlconnectionTrace *************************************************
**
** Trace an AJAX SQL Connection.
**
** @param [r] sqlc [const AjPSqlconnection] AJAX SQL Connection
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlconnectionTrace(const AjPSqlconnection sqlc, ajuint level)
{
    AjPStr indent = NULL;

    if(!sqlc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SajSqlconnectionTrace %p\n"
            "%S  Pconnection %p\n"
            "%S  Client %d\n"
            "%S  Use %u\n",
            indent, sqlc,
            indent, sqlc->Pconnection,
            indent, sqlc->Client,
            indent, sqlc->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [AjESqlconnectionClient] SQL Connection Client ****************
**
** Functions for manipulating AJAX SQL Connection clients.
**
** @nam3rule SqlconnectionClient Functions for manipulating AJAX SQL Connection
** clients.
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning a AJAX SQL Connection Client enumeration
**
** @fdata [AjESqlconnectionClient]
**
** @nam4rule From  AJAX SQL Connection Client query
** @nam5rule Str   String object query
**
** @argrule  Str   client  [const AjPStr] Client name
**
** @valrule * [AjESqlconnectionClient] AJAX SQL Connection Client enumeration
**
** @fcategory misc
******************************************************************************/




/* @func ajSqlconnectionClientFromStr *****************************************
**
** Convert an AJAX String into an AJAX SQL Connection client element.
**
** @param [r] client [const AjPStr] Client string
**
** @return [AjESqlconnectionClient] AJAX SQL Connection client or
**                                  ajESqlconnectionClientNULL
** @@
******************************************************************************/

AjESqlconnectionClient ajSqlconnectionClientFromStr(const AjPStr client)
{
    register AjESqlconnectionClient i = ajESqlconnectionClientNULL;

    AjESqlconnectionClient eclient = ajESqlconnectionClientNULL;

    for(i = ajESqlconnectionClientMySQL; sqlconnectionClient[i]; i++)
        if(ajStrMatchC(client, sqlconnectionClient[i]))
            eclient = i;

    if(!eclient)
        ajDebug("ajSqlconnectionClientFromStr encountered "
                "unexpected string '%S'.\n", client);

    return eclient;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an AJAX SQL Connection client
** enumeration
**
** @fdata [AjESqlconnectionClient]
**
** @nam4rule To Return AJAX SQL Connection Client enumeration
** @nam5rule Char Return C character string value
**
** @argrule To client [AjESqlconnectionClient] AJAX SQL Connection
**
** @valrule * [const char*] Client name
** @valrule *Char [const char*] Client name
**
** @fcategory cast
******************************************************************************/




/* @func ajSqlconnectionClientToChar ******************************************
**
** Convert an AJAX SQL Connection client element into a C-type (char*) string.
**
** @param [u] client [AjESqlconnectionClient] SQL Connection client
**
** @return [const char*] SQL Connection client C-type (char*) string
** @@
******************************************************************************/

const char* ajSqlconnectionClientToChar(AjESqlconnectionClient client)
{
    register AjESqlconnectionClient i = ajESqlconnectionClientNULL;

    if(!client)
        return NULL;

    for(i = ajESqlconnectionClientMySQL;
        sqlconnectionClient[i] && (i < client);
        i++);

    if(!sqlconnectionClient[i])
        ajDebug("ajSqlconnectionClientToChar encountered an "
                "out of boundary error on client %d.\n", client);

    return sqlconnectionClient[i];
}




/* @datasection [AjPSqlstatement] AJAX SQL Statement **************************
**
** Functions for manipulating AJAX SQL Statements.
**
** @nam2rule Sqlstatement Functions for manipulating AJAX SQL Statements.
**
******************************************************************************/




#ifdef HAVE_MYSQL
/* @funcstatic sqlstatementMysqlNewRun ****************************************
**
** MySQL client library-specific AJAX SQL Statement constructor.
** Upon construction the SQL Statement is run against the MySQL server
** specified in the AJAX SQL Connection.
** Eventual results of the SQL Statement are then stored inside this object.
**
** @param [u] sqlc [AjPSqlconnection] AJAX SQL Connection
** @param [r] statement [const AjPStr] SQL statement
** @param [r] debug [AjBool] Debug mode
**
** @return [AjPSqlstatement] AJAX SQL Statement or NULL
** @@
******************************************************************************/

static AjPSqlstatement sqlstatementMysqlNewRun(AjPSqlconnection sqlc,
                                               const AjPStr statement,
                                               AjBool debug)
{
    AjPSqlstatement sqls = NULL;

    MYSQL *Pmysql        = NULL;
    MYSQL_RES *Pmysqlres = NULL;

    debug |= ajDebugTest("sqlstatementMysqlNewRun");

    if(!sqlc)
        return NULL;

    if(!statement)
        return NULL;

    Pmysql = (MYSQL *) sqlc->Pconnection;

    if(!Pmysql)
        ajFatal("sqlstatementMysqlNewRun got an AJAX SQL Connection without "
                "a MYSQL client library-specific (MYSQL *) connection "
                "object.");

    if(mysql_real_query(Pmysql,
                        ajStrGetPtr(statement),
                        (unsigned long) ajStrGetLen(statement)))
    {
        ajWarn("sqlstatementMysqlNewRun encountered an "
               "error upon calling mysql_real_query.\n"
               "  statement: %S\n"
               "  MySQL error: %s\n",
               statement,
               mysql_error(Pmysql));

#ifdef AJ_SAVESTATS

        sqlstatementErrorCount++;

#endif /* AJ_SAVESTATS */

        return NULL;
    }

    /*
    ** The SQL statement was successful, now process any data returned.
    ** The mysql_store_result function retrieves all rows from the
    ** server and stores them in the client immediately. This is can be
    ** memory intensive, but reduces the server load.
    **/

    Pmysqlres = mysql_store_result(Pmysql);

    if(Pmysqlres)
    {
        /*
        ** The SQL statement has returned a (MYSQL_RES*) result, hence
        ** create an AJAX SQL Statement object and set the number of rows
        ** selected by the SQL statement.
        */

        AJNEW0(sqls);

        sqls->Sqlconnection = ajSqlconnectionNewRef(sqlc);

        sqls->Presult = (void *) Pmysqlres;

        sqls->AffectedRows = 0;

        sqls->SelectedRows = (ajulong) mysql_num_rows(Pmysqlres);

        sqls->Columns = (ajuint) mysql_num_fields(Pmysqlres);

        sqls->Use = 1;

        if(debug)
            ajDebug("ajSqlstatementNewRun MySQL selected "
                    "%Lu rows and %u columns.\n",
                    sqls->SelectedRows,
                    sqls->Columns);
    }
    else
    {
        /*
        ** Since the SQL statement has not returnd a (MYSQL_RES*) result,
        ** check whether it should have returned one.
        */

        if(mysql_field_count(Pmysql))
        {
            /*
            ** The SQL statement was expected to return a (MYSQL_RES*)
            ** result.
            */

            ajWarn("sqlstatementMysqlNewRun encountered an "
                   "error upon calling mysql_store_result.\n"
                   "  statement: %S\n"
                   "  MySQL error: %s\n",
                   statement,
                   mysql_error(Pmysql));

#ifdef AJ_SAVESTATS

            sqlstatementErrorCount++;

#endif /* AJ_SAVESTATS */
        }
        else
        {
            /*
            ** The SQL statement was not expected to return a (MYSQL_RES*)
            ** result, hence create an AJAX SQL Statement object and set the
            ** number rows affected by the SQL statement.
            */

            AJNEW0(sqls);

            sqls->Sqlconnection = ajSqlconnectionNewRef(sqlc);

            sqls->Presult = NULL;

            sqls->AffectedRows = (ajulong) mysql_affected_rows(Pmysql);

            sqls->SelectedRows = 0;

            sqls->Columns = 0;

            sqls->Use = 1;

            if(debug)
                ajDebug("ajSqlstatementNewRun MySQL affected "
                        "%Lu rows.\n",
                        sqls->AffectedRows);
        }
    }

    return sqls;
}

#endif /* HAVE_MYSQL */




#ifdef HAVE_POSTGRESQL
/* @funcstatic sqlstatementPostgresqlNewRun ***********************************
**
** PostgreSQL client library-specific AJAX SQL Statement constructor.
** Upon construction, the SQL Statement is run against the PostgreSQL server
** specified in the AJAX SQL Connection.
** Eventual results of the SQL Statement are then stored inside this object.
**
** @param [u] sqlc [AjPSqlconnection] AJAX SQL Connection
** @param [r] statement [const AjPStr] SQL statement
** @param [r] debug [AjBool] Debug mode
**
** @return [AjPSqlstatement] AJAX SQL Statement or NULL
** @@
******************************************************************************/

static AjPSqlstatement sqlstatementPostgresqlNewRun(AjPSqlconnection sqlc,
                                                    const AjPStr statement,
                                                    AjBool debug)
{
    AjPSqlstatement sqls = NULL;

    AjPStr affected = NULL;

    PGconn *Ppgconn     = NULL;
    PGresult *Ppgresult = NULL;

    debug |= ajDebugTest("sqlstatementPostgresqlNewRun");

    if(!sqlc)
        return NULL;

    if(!statement)
        return NULL;

    Ppgconn = (PGconn *) sqlc->Pconnection;

    if(!Ppgconn)
        ajFatal("sqlstatementPostgresqlNewRun got AJAX SQL Connection without "
                "PostgreSQL client library-specific (PGconn *) connection "
                "object.");

    Ppgresult = PQexec(Ppgconn, ajStrGetPtr(statement));

    if(Ppgresult)
    {
        switch (PQresultStatus(Ppgresult))
        {
            case PGRES_EMPTY_QUERY:

                ajDebug("sqlstatementPostgresqlNewRun PostgreSQL reported an "
                        "empty statement string.\n"
                        "  statement: %S\n",
                        statement);

                PQclear(Ppgresult);

#ifdef AJ_SAVESTATS

                sqlstatementErrorCount++;

#endif /* AJ_SAVESTATS */

                break;

            case PGRES_COMMAND_OK:

                /*
                ** PGRES_COMMAND_OK for statements that return no result
                ** rows but do affect rows.
                */

                affected = ajStrNewC(PQcmdTuples(Ppgresult));

                AJNEW0(sqls);

                sqls->Sqlconnection = ajSqlconnectionNewRef(sqlc);

                sqls->Presult = (void *) Ppgresult;

                if(!ajStrToUlong(affected, &sqls->AffectedRows))
                    ajWarn("sqlstatementPostgresqlNewRun could not parse "
                           "'%S' into an AJAX unsigned long integer.",
                           affected);

                sqls->SelectedRows = 0;

                sqls->Columns = 0;

                sqls->Use = 1;

                ajStrDel(&affected);

                if(debug)
                    ajDebug("ajSqlstatementNewRun PostgreSQL affected "
                            "%Lu rows.\n",
                            sqls->AffectedRows);

                break;

            case PGRES_TUPLES_OK:

                /* PGRES_TUPLES_OK for commands that return result rows. */

                AJNEW0(sqls);

                sqls->Sqlconnection = ajSqlconnectionNewRef(sqlc);

                sqls->Presult = (void *) Ppgresult;

                sqls->AffectedRows = 0;

                sqls->SelectedRows = (ajulong) PQntuples(Ppgresult);

                sqls->Columns = (ajuint) PQnfields(Ppgresult);

                sqls->Use = 1;

                if(debug)
                    ajDebug("ajSqlstatementNewRun PostgreSQL selected "
                            "%Lu rows and %u columns.\n",
                            sqls->SelectedRows,
                            sqls->Columns);

                break;

            case PGRES_FATAL_ERROR:

                ajWarn("sqlstatementPostgresqlNewRun encountered an "
                       "error upon calling PQexec.\n"
                       "  statement: %S\n"
                       "  PostgreSQL error: %s\n",
                       statement,
                       PQresultErrorMessage(Ppgresult));

                PQclear(Ppgresult);

#ifdef AJ_SAVESTATS

                sqlstatementErrorCount++;

#endif /* AJ_SAVESTATS */

                break;

            default:

                ajDebug("sqlstatementPostgresqlNewRun encountered an "
                        "unexpected status upon calling PQexec.\n"
                        "  PostgreSQL status: %s\n",
                        PQresStatus(PQresultStatus(Ppgresult)));

                PQclear(Ppgresult);

#ifdef AJ_SAVESTATS

                sqlstatementErrorCount++;

#endif /* AJ_SAVESTATS */
        }
    }
    else
    {
        ajWarn("sqlstatementPostgresqlNewRun encountered an "
               "error upon calling PQexec.\n"
               "  statement: %S"
               "  PostgreSQL error: %s\n",
               statement,
               PQerrorMessage(Ppgconn));

#ifdef AJ_SAVESTATS

        sqlstatementErrorCount++;

#endif /* AJ_SAVESTATS */
    }

    return sqls;
}

#endif /* HAVE_POSTGRESQL */




/* @section constructors ******************************************************
**
** Functions for constructing AJAX SQL Statement objects.
**
** @fdata [AjPSqlstatement]
**
** @nam3rule New Construct a new AJAX SQL Statement
** @nam4rule Ref Constructor by incrementing the reference counter
** @nam4rule Run Constructor by running a SQL statement
**
** @argrule Ref sqls [AjPSqlstatement] Undocumented
** @argrule Run sqlc [AjPSqlconnection] Undocumented
** @argrule Run statement [const AjPStr] Undocumented
**
** @valrule * [AjPSqlstatement] AJAX SQL Statement
**
** @fcategory new
******************************************************************************/




/* @func ajSqlstatementNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] sqls [AjPSqlstatement] AJAX SQL Statement
**
** @return [AjPSqlstatement] AJAX SQL Statement or NULL
** @@
******************************************************************************/

AjPSqlstatement ajSqlstatementNewRef(AjPSqlstatement sqls)
{
    if(!sqls)
        return NULL;

    sqls->Use++;

    return sqls;
}




/* @func ajSqlstatementNewRun *************************************************
**
** Default AJAX SQL Statement constructor. Upon construction the SQL Statement
** is run against the SQL server specified in the AJAX SQL Connection.
** Eventual results of the SQL Statement are then stored inside this object.
**
** @param [u] sqlc [AjPSqlconnection] AJAX SQL Connection
** @param [r] statement [const AjPStr] SQL statement
**
** @return [AjPSqlstatement] AJAX SQL Statement or NULL
** @@
******************************************************************************/

AjPSqlstatement ajSqlstatementNewRun(AjPSqlconnection sqlc,
                                     const AjPStr statement)
{
    AjBool debug = AJFALSE;

    AjPSqlstatement sqls = NULL;

    debug = ajDebugTest("ajSqlstatementNewRun");

    if(!sqlc)
        return NULL;

    if(!statement)
        return NULL;

    if(debug)
        ajDebug("ajSqlstatementNewRun %S\n", statement);

    switch(sqlc->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            sqls = sqlstatementMysqlNewRun(sqlc, statement, debug);

#else

            ajDebug("ajSqlstatementNewRun got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_MYSQL */

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            sqls = sqlstatementPostgresqlNewRun(sqlc, statement, debug);

#else

            ajDebug("ajSqlstatementNewRun got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_POSTGRESQL */

            break;

        default:

            ajDebug("ajSqlstatementNewRun AJAX SQL Connection client %d "
                    "not supported.\n",
                    sqlc->Client);
    }

#ifdef AJ_SAVESTATS

    if(sqls)
        sqlstatementTotalCount++;

#endif /* AJ_SAVESTATS */

    return sqls;
}




/* @section destructors *******************************************************
**
** Functions for destruction of AJAX SQL Statement objects.
**
** @fdata [AjPSqlstatement]
**
** @nam3rule Del Destroy (free) an existing AJAX SQL Statement
**
** @argrule Del Psqls [AjPSqlstatement*] AJAX SQL Statement address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ajSqlstatementDel ****************************************************
**
** Default AJAX SQL Statement destructor, which also destroys the SQL client
** library-specific result object.
**
** @param [d] Psqls [AjPSqlstatement*] AJAX SQL Statement address
**
** @return [void]
** @@
******************************************************************************/

void ajSqlstatementDel(AjPSqlstatement *Psqls)
{
    AjPSqlstatement pthis = NULL;

    if(!Psqls)
        return;

    if(!*Psqls)
        return;

    pthis = *Psqls;

    pthis->Use--;

    if(pthis->Use)
    {
        *Psqls = NULL;

        return;
    }

    switch(pthis->Sqlconnection->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            mysql_free_result((MYSQL_RES *) pthis->Presult);

#else

            ajDebug("ajSqlstatementDel got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_MYSQL */

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            PQclear((PGresult *) pthis->Presult);

#else

            ajDebug("ajSqlstatementDel got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif /* HAVE_POSTGRESQL */

            break;

        default:

            ajDebug("ajSqlstatementDel AJAX SQL Connection client %d "
                    "not supported.\n",
                    pthis->Sqlconnection->Client);

    }

    ajSqlconnectionDel(&pthis->Sqlconnection);

    AJFREE(pthis);

    *Psqls = NULL;

#ifdef AJ_SAVESTATS

    sqlstatementFreeCount++;

#endif /* AJ_SAVESTATS */

    return;
}





/* @section cast **************************************************************
**
** @fdata [AjPSqlstatement]
**
** @nam3rule Get return elements of a SQL statement
** @nam4rule Affectedrows return number of rows affected
** @nam4rule Columns return number of columns returned
** @nam4rule Identifier return identifier of a row inserted
** @nam4rule Selectedrows return number of rows selected
**
** @argrule Get sqls [const AjPSqlstatement] Undocumented
**
** @valrule Affectedrows [ajulong] Number of affected rows
** @valrule Columns [ajuint] Number of columns returned
** @valrule Identifier [ajuint] Identifier of a row inserted
** @valrule Selectedrows [ajulong] Number of selected rows
**
** @fcategory cast
**
******************************************************************************/




/* @func ajSqlstatementGetAffectedrows ****************************************
**
** Get the number of rows affected by a non-SELECT SQL statement.
**
** @param [r] sqls [const AjPSqlstatement] AJAX SQL Statement
**
** @return [ajulong] Number of affected rows
** @@
******************************************************************************/

ajulong ajSqlstatementGetAffectedrows(const AjPSqlstatement sqls)
{
    if(!sqls)
        return 0;

    return sqls->AffectedRows;
}




/* @func ajSqlstatementGetColumns *********************************************
**
** Get the number of columns returned by a SELECT-like SQL statement.
**
** @param [r] sqls [const AjPSqlstatement] AJAX SQL Statement
**
** @return [ajuint] Number of selected columns
** @@
******************************************************************************/

ajuint ajSqlstatementGetColumns(const AjPSqlstatement sqls)
{
    if(!sqls)
        return 0;

    return sqls->Columns;
}




/* @func ajSqlstatementGetIdentifier ******************************************
**
** Get the identifier of a row inserted by the last INSERT-like SQL statement.
**
** @param [r] sqls [const AjPSqlstatement] AJAX SQL Statement
**
** @return [ajuint] Identifier of last inserted row
** @@
******************************************************************************/

ajuint ajSqlstatementGetIdentifier(const AjPSqlstatement sqls)
{
    ajuint identifier = 0;

#ifdef HAVE_MYSQL

    MYSQL *Pmysql = NULL;

#endif /* HAVE_MYSQL */

#ifdef HAVE_POSTGRESQL

    PGresult *Ppgresult = NULL;

#endif /* HAVE_POSTGRESQL */

    if(!sqls)
        return 0;

    switch(sqls->Sqlconnection->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            Pmysql = (MYSQL *) sqls->Sqlconnection->Pconnection;

            identifier = (ajuint) mysql_insert_id(Pmysql);

#else

            ajDebug("ajSqlstatementGetIdentifier got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            Ppgresult = (PGresult *) sqls->Presult;

            identifier = (ajuint) PQoidValue(Ppgresult);

#else

            ajDebug("ajSqlstatementGetIdentifier got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif
            break;

        default:

            ajDebug("ajSqlstatementNewRun AJAX SQL Connection client %d "
                    "not supported.\n",
                    sqls->Sqlconnection->Client);
    }

    return identifier;
}




/* @func ajSqlstatementGetSelectedrows ****************************************
**
** Get the number of rows selected by a SELECT-like SQL statement.
**
** @param [r] sqls [const AjPSqlstatement] AJAX SQL Statement
**
** @return [ajulong] Number of selected rows
** @@
******************************************************************************/

ajulong ajSqlstatementGetSelectedrows(const AjPSqlstatement sqls)
{
    if(!sqls)
        return 0;

    return sqls->SelectedRows;
}




/* @datasection [AjISqlrow] AJAX SQL Row Iterator *****************************
**
** Functions for manipulating AJAX SQL Row Iterators.
**
** @nam2rule Sqlrowiter Functions for manipulating AJAX SQL Row Iterators.
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Functions for constructing AJAX SQL Row Iterator objects.
**
** @fdata [AjISqlrow]
**
** @nam3rule New Construct a new AJAX SQL Row Iterator
**
** @argrule New sqls [AjPSqlstatement] Undocumented
**
** @valrule * [AjISqlrow] AJAX SQL Row Iterator
**
** @fcategory new
******************************************************************************/




/* @func ajSqlrowiterNew ******************************************************
**
** Default AJAX SQL Row Iterator constructor.
**
** @param [u] sqls [AjPSqlstatement] AJAX SQL Statement
**
** @return [AjISqlrow] AJAX SQL Row Iterator or NULL if the
**                     AJAX SQL Statement did not return rows and columns
** @@
******************************************************************************/

AjISqlrow ajSqlrowiterNew(AjPSqlstatement sqls)
{
    AjISqlrow sqli = NULL;

    if(!sqls)
        return NULL;

    if(!sqls->SelectedRows)
        return NULL;

    if(!sqls->Columns)
        return NULL;

    AJNEW0(sqli);

    sqli->Sqlstatement = ajSqlstatementNewRef(sqls);

    sqli->Sqlrow = ajSqlrowNew(sqls->Columns);

    sqli->Current = 0;

    return sqli;
}




/* @section destructors *******************************************************
**
** Functions for destruction of AJAX SQL Row Iterator objects.
**
** @fdata [AjISqlrow]
**
** @nam3rule Del Destroy (free) an existing AJAX SQL Row Iterator
**
** @argrule Del Psqli [AjISqlrow*] AJAX SQL Row Iterator address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ajSqlrowiterDel ******************************************************
**
** Default AJAX SQL Row Iterator destructor.
**
** @param [d] Psqli [AjISqlrow*] AJAX SQL Row Iterator address
**
** @return [void]
** @@
******************************************************************************/

void ajSqlrowiterDel(AjISqlrow *Psqli)
{
    AjISqlrow pthis = NULL;

    if(!Psqli)
        return;

    if(!*Psqli)
        return;

    pthis = *Psqli;

    ajSqlstatementDel(&pthis->Sqlstatement);

    ajSqlrowDel(&pthis->Sqlrow);

    AJFREE(pthis);

    *Psqli = NULL;

    return;
}




/* @section tests *************************************************************
**
** @fdata [AjISqlrow]
**
** @nam3rule Done Check whether iteration has ended (no more AJAX SQL Rows).
**
** @argrule Done sqli [const AjISqlrow] AJAX SQL Row Iterator
**
** @valrule * [AjBool] Result of test
**
** @fcategory use
******************************************************************************/




/* @func ajSqlrowiterDone *****************************************************
**
** Tests whether an AJAX SQL Row Iterator has completed yet.
**
** @param [r] sqli [const AjISqlrow] AJAX SQL Row Iterator
**
** @return [AjBool] ajTrue if the iterator is exhausted
** @@
******************************************************************************/

AjBool ajSqlrowiterDone(const AjISqlrow sqli)
{
    if(!sqli)
        return ajTrue;

    if(sqli->Current < sqli->Sqlstatement->SelectedRows)
        return ajFalse;

    return ajTrue;
}




/* @section stepping **********************************************************
**
** @fdata [AjISqlrow]
**
** @nam3rule Get Next AJAX SQL Row.
**
** @argrule * sqli [AjISqlrow] AJAX SQL Row Iterator
**
** @valrule * [AjPSqlrow] AJAX SQL Row
**
** @fcategory modify
******************************************************************************/




/* @func ajSqlrowiterGet ******************************************************
**
** Returns the next AJAX SQL Row using the iterator, or steps off the end.
**
** @param [u] sqli [AjISqlrow] AJAX SQL Row Iterator
**
** @return [AjPSqlrow] AJAX SQL Row or NULL
** @@
******************************************************************************/

AjPSqlrow ajSqlrowiterGet(AjISqlrow sqli)
{
    register ajuint i = 0;

    AjBool debug = AJFALSE;

    AjPSqlrow sqlr = NULL;

#ifdef HAVE_MYSQL

    unsigned long *lengths;

    MYSQL *Pmysql = NULL;
    MYSQL_ROW mysqlrow;

#endif

    debug = ajDebugTest("ajSqlrowiterGet");

    if(!sqli)
        return NULL;

    (void) i;

    /* Check that the Iterator is within a valid range. */

    if(sqli->Current >= sqli->Sqlstatement->SelectedRows)
    {
        if(debug)
            ajDebug("ajSqlrowiterGet got no more AJAX SQL Rows to fetch.\n");

        return NULL;
    }

    /*
    ** If an AJAX SQL Row already exits, reset the current column value,
    ** which can be used for iterating over columns of a row, otherwise
    ** construct a new AJAX SQL Row with the correct number of columns.
    */

    if(sqli->Sqlrow)
        sqli->Sqlrow->Current = 0;
    else
        sqli->Sqlrow = ajSqlrowNew(sqli->Sqlstatement->Columns);

    switch(sqli->Sqlstatement->Sqlconnection->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            mysqlrow = mysql_fetch_row(
                (MYSQL_RES *) sqli->Sqlstatement->Presult);

            if(mysqlrow)
            {
                lengths = mysql_fetch_lengths(
                    (MYSQL_RES *) sqli->Sqlstatement->Presult);

                for(i = 0; i < sqli->Sqlstatement->Columns; i++)
                {
                    ajVoidPut(&sqli->Sqlrow->Values, i, (void *) mysqlrow[i]);

                    ajLongPut(&sqli->Sqlrow->Lengths, i, (ajlong) lengths[i]);
                }

                sqli->Current++;

                sqlr = sqli->Sqlrow;
            }
            else
            {
                Pmysql =
                    (MYSQL *) sqli->Sqlstatement->Sqlconnection->Pconnection;

                if(mysql_errno(Pmysql))
                    ajDebug("ajSqlrowiterGet encountered an error.\n"
                            "  MySQL error: %s", mysql_error(Pmysql));
                else
                    ajDebug("ajSqlrowiterGet got no more MySQL rows "
                            "to fetch?\n");
            }

#else

            ajDebug("ajSqlrowiterGet got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            for(i = 0; i < sqli->Sqlstatement->Columns; i++)
            {
                ajVoidPut(&sqli->Sqlrow->Values, i,
                          (void *) PQgetvalue(
                              (PGresult *) sqli->Sqlstatement->Presult,
                              (int) sqli->Current,
                              (int) i));

                ajLongPut(&sqli->Sqlrow->Lengths, i,
                          (ajlong) PQgetlength(
                              (PGresult *) sqli->Sqlstatement->Presult,
                              (int) sqli->Current,
                              (int) i));
            }

            sqli->Current++;

            sqlr = sqli->Sqlrow;

#else

            ajDebug("ajSqlrowiterGet got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif

            break;

        default:

            ajDebug("ajSqlrowiterGet AJAX SQL Connection client %d "
                    "not supported.\n",
                    sqli->Sqlstatement->Sqlconnection->Client);
    }

    return sqlr;
}




/* @section modifiers *********************************************************
**
** @fdata [AjISqlrow]
**
** @nam3rule Rewind Rewind to start
**
** @argrule * sqli [AjISqlrow] SQL Row Iterator
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
******************************************************************************/




/* @func ajSqlrowiterRewind ***************************************************
**
** Rewind an AJAX SQL Row Iterator to the start position.
**
** @param [u] sqli [AjISqlrow] AJAX SQL Row Iterator
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlrowiterRewind(AjISqlrow sqli)
{
    AjBool value = AJFALSE;

    if(!sqli)
        return ajFalse;


    switch(sqli->Sqlstatement->Sqlconnection->Client)
    {
        case ajESqlconnectionClientMySQL:

#ifdef HAVE_MYSQL

            /* For MySQL, the cursor and the current row need resetting. */

            sqli->Current = 0;

            mysql_data_seek((MYSQL_RES *) sqli->Sqlstatement->Presult, 0);

            value = ajTrue;

#else

            ajDebug("ajSqlrowiterRewind got an AJAX SQL Connection, "
                    "which claims a MySQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif

            break;

        case ajESqlconnectionClientPostgreSQL:

#ifdef HAVE_POSTGRESQL

            /* For PostgreSQL, just the current row needs resetting. */

            sqli->Current = 0;

            value = ajTrue;

#else

            ajDebug("ajSqlrowiterRewind got an AJAX SQL Connection, "
                    "which claims a PostgreSQL connection, but support "
                    "for this client has not been built into the "
                    "EMBOSS AJAX library.\n");

#endif

            break;

        default:

            ajDebug("ajSqlrowiterRewind AJAX SQL Connection client %d "
                    "not supported.\n",
                    sqli->Sqlstatement->Sqlconnection->Client);
    }

    return value;
}




/* @datasection [AjPSqlrow] AJAX SQL Row **************************************
**
** Functions for manipulating AJAX SQL Rows.
**
** @nam2rule Sqlrow Functions for manipulating AJAX SQL Rows.
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Functions for constructing AJAX SQL Row objects.
**
** @fdata [AjPSqlrow]
**
** @nam3rule New Construct a new AJAX SQL Row
** @nam3rule NewRef Constructor by incrementing the reference counter
**
** @argrule New columns [ajuint] Number of columns per AJAX SQL Row
**
** @valrule * [AjPSqlrow] AJAX SQL Row
**
** @fcategory new
******************************************************************************/




/* @func ajSqlrowNew **********************************************************
**
** Default AJAX SQL Row constructor.
**
** @param [r] columns [ajuint] Number of columns per AJAX SQL Row
**
** @return [AjPSqlrow] AJAX SQL Row
** @@
******************************************************************************/

AjPSqlrow ajSqlrowNew(ajuint columns)
{
    AjPSqlrow sqlr = NULL;

    AJNEW0(sqlr);

    sqlr->Values = ajVoidNewRes(columns);

    sqlr->Lengths = ajLongNewRes(columns);

    sqlr->Columns = columns;

    sqlr->Current = 0;

    return sqlr;
}




/* @section destructors *******************************************************
**
** Functions for destruction of AJAX SQL Row objects.
**
** @fdata [AjPSqlrow]
**
** @nam3rule Del Destroy (free) an existing AJAX SQL Row
**
** @argrule Del Psqlr [AjPSqlrow*] AJAX SQL Row address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ajSqlrowDel **********************************************************
**
** Default AJAX SQL Row destructor.
**
** @param [d] Psqlr [AjPSqlrow*] AJAX SQL Row address
**
** @return [void]
** @@
******************************************************************************/

void ajSqlrowDel(AjPSqlrow *Psqlr)
{
    AjPSqlrow pthis = NULL;

    if(!Psqlr)
        return;

    if(!*Psqlr)
        return;

    pthis = *Psqlr;

    ajVoidDel(&pthis->Values);

    ajLongDel(&pthis->Lengths);

    AJFREE(pthis);

    *Psqlr = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an AJAX SQL Row object.
**
** @fdata [AjPSqlrow]
** @fnote None
**
** @nam3rule Get Return SQL Row attribute(s)
** @nam4rule Lengths Return the AJAX Long Array of column value lengths
** @nam4rule Columns Return the number of columns
** @nam4rule Current Return the number of the current column in iteration
** @nam4rule Values Return the AJAX Void Pointer Array of column values
**
** @argrule * sqlr [const AjPSqlrow] SQL Row
**
** @valrule Columns [ajuint] Number of columns
** @valrule Current [ajuint] Current column
** @valrule Lengths [AjPLong] Column value lengths
** @valrule Values [AjPVoid] Column values
**
** @fcategory use
******************************************************************************/




/* @func ajSqlrowGetColumns ***************************************************
**
** Get the number of columns in an AJAX SQL Row.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
**
** @return [ajuint] Number of columns in an AJAX SQL Row
** @@
******************************************************************************/

ajuint ajSqlrowGetColumns(const AjPSqlrow sqlr)
{
    if(!sqlr)
        return 0;

    return sqlr->Columns;
}




/* @func ajSqlrowGetCurrent ***************************************************
**
** Get the number of the current column in column iterations of an
** AJAX SQL Row.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
**
** @return [ajuint] Number of current column of an AJAX SQL Row
** @@
******************************************************************************/

ajuint ajSqlrowGetCurrent(const AjPSqlrow sqlr)
{
    if(!sqlr)
        return 0;

    return sqlr->Current;
}




/* @func ajSqlrowGetLengths ***************************************************
**
** Get the AJAX Long Integer Array of column value lengths in an AJAX SQL Row.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
**
** @return [AjPLong] AJAX Long Integer Array
** @@
******************************************************************************/

AjPLong ajSqlrowGetLengths(const AjPSqlrow sqlr)
{
    if(!sqlr)
        return NULL;

    return sqlr->Lengths;
}




/* @func ajSqlrowGetValues ****************************************************
**
** Get the AJAX Void Pointer Array of column values in an AJAX SQL Row.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
**
** @return [AjPVoid] AJAX Void Pointer Array
** @@
******************************************************************************/

AjPVoid ajSqlrowGetValues(const AjPSqlrow sqlr)
{
    if(!sqlr)
        return NULL;

    return sqlr->Values;
}




/* @datasection [AjPSqlrow] AJAX SQL Column ***********************************
**
** Functions for manipulating AJAX SQL Row Columns
**
** @nam2rule Sqlcolumn Functions for manipulating AJAX SQL Row COlumns
**
******************************************************************************/




/* @section column modifiers **************************************************
**
** @fdata [AjPSqlrow]
**
** @nam3rule Rewind Rewind an AJAX SQL Column Iterator
**
** @argrule Rewind sqlr [AjPSqlrow] AJAX SQL Row
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ajSqlcolumnRewind ****************************************************
**
** Rewind an AJAX SQL Column Iterator to the start position.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnRewind(AjPSqlrow sqlr)
{
    if(!sqlr)
        return ajFalse;

    sqlr->Current = 0;

    return ajTrue;
}




/* @section column retrieval **************************************************
**
** @fdata [AjPSqlrow]
**
** @nam3rule Get column element
** @nam4rule Value Get the value of the next column
**
** @argrule Get sqlr [AjPSqlrow] AJAX SQL Row
** @argrule Value Pvalue [void**] Value address
** @argrule Value Plength [ajulong*] Value length address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ajSqlcolumnGetValue **************************************************
**
** Get the value of the next column of an AJAX SQL Row.
** Calling this function also advances the column iterator.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [u] Pvalue [void**] Value address
** @param [u] Plength [ajulong*] Value length address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnGetValue(AjPSqlrow sqlr,
                            void **Pvalue, ajulong *Plength)
{
    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(!Plength)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    *Pvalue = ajVoidGet(sqlr->Values, sqlr->Current);

    *Plength = ajLongGet(sqlr->Lengths, sqlr->Current);

    sqlr->Current++;

    return ajTrue;
}




/* @section column to datatype conversion functions ***************************
**
** Functions for converting SQL Column values to other datatypes.
**
** @fdata [AjPSqlrow]
** @fnote None
**
** @nam3rule To Convert to another type
** @nam4rule ToBool Convert to AJAX Boolean
** @nam4rule ToDouble Convert to C-type double
** @nam4rule ToFloat Convert to C-type float
** @nam4rule ToInt Convert to AJAX Signed Integer
** @nam4rule ToLong Convert to AJAX Signed Long Integer
** @nam4rule ToStr Convert to AJAX String
** @nam4rule ToTime Conver to AJAX Time
** @nam4rule ToUint Convert ot AJAX Unsigned Integer
** @nam4rule ToUlong Convert to AJAX Unsigned Long Integer
**
** @argrule * sqlr [AjPSqlrow] SQL Row
** @argrule ToBool Pvalue [AjBool*] AJAX Bool address
** @argrule ToDouble Pvalue [double*] C-type double address
** @argrule ToFloat Pvalue [float*] C-type float address
** @argrule ToInt Pvalue [ajint*] AJAX Signed Integer address
** @argrule ToLong Pvalue [ajlong*] AJAX Signed Long Integer address
** @argrule ToStr Pvalue [AjPStr*] AJAX String address
** @argrule ToTime Pvalue [AjPTime*] AJAX Time address
** @argrule ToUint Pvalue [ajuint*] AJAX Unsigned Integer address
** @argrule ToUlong Pvalue [ajulong*] AJAX Unsigned Long Integer address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ajSqlcolumnToBool ****************************************************
**
** Converts the value in the next column of an AJAX SQL Row into an
** AJAX Boolean value.
**
** This function uses ajStrToBool to convert the AJAX String representing the
** column value into an AJAX boolean value. The function converts 'yes' and
** 'true', as well as 'no' and 'false' into its corresponding AJAX Bool values.
** It also assumes any numeric value as true and 0 as false.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [AjBool*] AJAX Boolean address
** @see ajStrToBool
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToBool(AjPSqlrow sqlr, AjBool *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToBool(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToDouble **************************************************
**
** Converts the value in the next column of an AJAX SQL Row into a
** C-type double value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [double*] C-type double address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToDouble(AjPSqlrow sqlr, double *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToDouble(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToFloat ***************************************************
**
** Converts the value in the next column of an AJAX SQL Row into a
** C-type float value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [float*] C-type float address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToFloat(AjPSqlrow sqlr, float *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToFloat(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToInt *****************************************************
**
** Converts the value in the next column of an AJAX SQL Row into an
** AJAX Integer value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [ajint*] AJAX Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToInt(AjPSqlrow sqlr, ajint *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToInt(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToLong ****************************************************
**
** Converts the value in the next column of an AJAX SQL Row into an
** AJAX Long Integer value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [ajlong*] AJAX Long Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToLong(AjPSqlrow sqlr, ajlong *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToLong(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToStr *****************************************************
**
** Converts the value of the next column of an AJAX SQL Row into an
** AJAX String value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToStr(AjPSqlrow sqlr, AjPStr *Pvalue)
{
    void *value = NULL;

    ajulong length = 0;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    ajStrAssignClear(Pvalue);

    if(ajSqlcolumnGetValue(sqlr, &value, &length))
    {
        if(value == NULL)
            return ajFalse;

        if(length > UINT_MAX)
            return ajFalse;

        ajStrAssignLenC(Pvalue, (char *) value, (size_t) length);

        return ajTrue;
    }

    return ajFalse;
}




/* @func ajSqlcolumnToTime ****************************************************
**
** Converts the value in the next column of an AJAX SQL Row into an
** AJAX Time value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [AjPTime*] AJAX Time address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToTime(AjPSqlrow sqlr, AjPTime *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(!*Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajTimeSetS(*Pvalue, str);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToUint ****************************************************
**
** Converts the value in the next column of an AJAX SQL Row into an
** AJAX Unsigned Integer value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [ajuint*] AJAX Unsigned Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToUint(AjPSqlrow sqlr, ajuint *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToUint(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnToUlong ***************************************************
**
** Converts the value in the next column of an AJAX SQL Row into an
** AJAX Unsigned Long Integer value.
**
** @param [u] sqlr [AjPSqlrow] AJAX SQL Row
** @param [w] Pvalue [ajulong*] AJAX Unsigned Long Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnToUlong(AjPSqlrow sqlr, ajulong *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(sqlr->Current >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnToStr(sqlr, &str);

    if(bool)
        ajStrToUlong(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @section column retrieval **************************************************
**
** @fdata [AjPSqlrow]
**
** @nam3rule Number Select a particular column of an AJAX SQL Row.
** @nam4rule Get Get an element
** @nam5rule Value Get the value of a particular column
**
** @argrule * sqlr [const AjPSqlrow] AJAX SQL Row
** @argrule * column [ajuint] Column number
**
** @argrule GetValue Pvalue [void**] Value address
** @argrule GetValue Plength [ajulong*] Value length address
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ajSqlcolumnNumberGetValue ********************************************
**
** Get the value of a particular column of an AJAX SQL Row.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [u] Pvalue [void**] Value address
** @param [u] Plength [ajulong*] Value length address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberGetValue(const AjPSqlrow sqlr,
                                 ajuint column,
                                 void **Pvalue,
                                 ajulong *Plength)
{
    if(!sqlr)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(!Plength)
        return ajFalse;

    *Pvalue = ajVoidGet(sqlr->Values, column);

    *Plength = ajLongGet(sqlr->Lengths, column);

    return ajTrue;
}




/* @section column to datatype conversion functions ***************************
**
** Functions for converting SQL Column values to other datatypes.
**
** @fdata [AjPSqlrow]
** @fnote None
**
** @nam3rule Number Convert a particular column to a differnt datatype
** @nam4rule To Convert to another type
** @nam5rule ToBool Convert to AJAX Boolean
** @nam5rule ToDouble Convert to C-type double
** @nam5rule ToFloat Convert to C-type float
** @nam5rule ToInt Convert to AJAX Signed Integer
** @nam5rule ToLong Convert to AJAX Signed Long Integer
** @nam5rule ToStr Convert to AJAX String
** @nam5rule ToTime Conver to AJAX Time
** @nam5rule ToUint Convert ot AJAX Unsigned Integer
** @nam5rule ToUlong Convert to AJAX Unsigned Long Integer
**
** @argrule * sqlr [const AjPSqlrow] SQL Row
** @argrule * column [ajuint] Column number
** @argrule ToBool    Pvalue [AjBool*] AJAX Bool address
** @argrule ToDouble  Pvalue [double*] C-type double address
** @argrule ToFloat   Pvalue [float*] C-type float address
** @argrule ToInt     Pvalue [ajint*] AJAX Signed Integer address
** @argrule ToLong    Pvalue [ajlong*] AJAX Signed Long Integer address
** @argrule ToTime    Pvalue [AjPTime*] AJAX Time address
** @argrule ToStr     Pvalue [AjPStr*] AJAX String address
** @argrule ToUint    Pvalue [ajuint*] AJAX Unsigned Integer address
** @argrule ToUlong   Pvalue [ajulong*] AJAX Unsigned Long Integer address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ajSqlcolumnNumberToBool **********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX Boolean value.
**
** This function uses ajStrToBool to convert the AJAX String representing the
** column value into an AJAX boolean value. The function converts 'yes' and
** 'true', as well as 'no' and 'false' into its corresponding AJAX Bool values.
** It also assumes any numeric value as true and 0 as false.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [AjBool*] AJAX Boolean address
** @see ajStrToBool
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToBool(const AjPSqlrow sqlr, ajuint column,
                               AjBool *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToBool(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToDouble ********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into a
** C-type double value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [double*] C-type double address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToDouble(const AjPSqlrow sqlr, ajuint column,
                                 double *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToDouble(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToFloat *********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into a
** C-type float value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [float*] C-type float address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToFloat(const AjPSqlrow sqlr, ajuint column,
                                float *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToFloat(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToInt ***********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX Integer value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [ajint*] AJAX Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToInt(const AjPSqlrow sqlr, ajuint column,
                              ajint *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToInt(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToLong **********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX Long Integer value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [ajlong*] AJAX Long Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToLong(const AjPSqlrow sqlr, ajuint column,
                               ajlong *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToLong(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToStr ***********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX String value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToStr(const AjPSqlrow sqlr, ajuint column,
                              AjPStr *Pvalue)
{
    void *value = NULL;

    ajulong length = 0;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    ajStrAssignClear(Pvalue);

    if(ajSqlcolumnNumberGetValue(sqlr, column, &value, &length))
    {
        if(value == NULL)
            return ajFalse;

        if(length > UINT_MAX)
            return ajFalse;

        ajStrAssignLenC(Pvalue, (char *) value, (size_t) length);

        return ajTrue;
    }

    return ajFalse;
}




/* @func ajSqlcolumnNumberToTime **********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX Time value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [AjPTime*] AJAX Time address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToTime(const AjPSqlrow sqlr, ajuint column,
                               AjPTime *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(!*Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajTimeSetS(*Pvalue, str);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToUint **********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX Unsigned Integer value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [ajuint*] AJAX Unsigned Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToUint(const AjPSqlrow sqlr, ajuint column,
                               ajuint *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToUint(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @func ajSqlcolumnNumberToUlong *********************************************
**
** Converts the value in a particular column of an AJAX SQL Row into an
** AJAX Unsigned Long Integer value.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
** @param [w] Pvalue [ajulong*] AJAX Unsigned Long Integer address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberToUlong(const AjPSqlrow sqlr, ajuint column,
                                ajulong *Pvalue)
{
    AjBool bool = ajFalse;

    AjPStr str = NULL;

    if(!sqlr)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    str = ajStrNew();

    bool = ajSqlcolumnNumberToStr(sqlr, column, &str);

    if(bool)
        ajStrToUlong(str, Pvalue);

    ajStrDel(&str);

    return bool;
}




/* @section tests *************************************************************
**
** @fdata [AjPSqlrow]
**
** @nam3rule Number Column number
** @nam4rule Is Property of culumn
** @nam5rule IsDefined Test if the value in a particular column of
**                                 an AJAX SQL Row is defined.
**
** @argrule IsDefined sqlr [const AjPSqlrow] AJAX SQL Row
** @argrule IsDefined column [ajuint] Column number
**
** @valrule IsDefined [AjBool] Result of test
**
** @fcategory use
******************************************************************************/




/* @func ajSqlcolumnNumberIsDefined *******************************************
**
** Test if the value in a particular column of an AJAX SQL Row is defined.
**
** @param [r] sqlr [const AjPSqlrow] AJAX SQL Row
** @param [r] column [ajuint] Column number
**
** @return [AjBool] ajTrue if the value is defined,
**                  ajFalse if the value is undefined (\N)
** @@
******************************************************************************/

AjBool ajSqlcolumnNumberIsDefined(const AjPSqlrow sqlr, ajuint column)
{
    if(!sqlr)
        return ajFalse;

    if(column >= sqlr->Columns)
        return ajFalse;

    if(ajVoidGet(sqlr->Values, column) == NULL)
        return ajFalse;

    return ajTrue;
}




/*
** FIXME: Move the AJAX Void Pointer Array into ajarr.c.
**
** FIXME: Should we rename this to AjPArrayVoid? All the other arrays should be
** renamed as well? AjPLong should be an alias for ajlong *, while
** AjPArrayLong would be the AJAX Array holding AJAX Long Integer values??
*/

#define RESERVED_SIZE 32




/* @datasection [AjPVoid] Void pointer *****************************************
**
** Functions for void pointer arrays
**
** @nam2rule Void Void pointer array function
**
******************************************************************************/




/* @section constructors ******************************************************
**
** @fdata [AjPVoid]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule Res length specified
**
** @argrule Res size [ajuint] Size of array
**
** @valrule * [AjPVoid] void pointer array
**
******************************************************************************/




/* @func ajVoidNew ************************************************************
**
** Default constructor for empty AJAX Void Pointer Arrays.
**
** @return [AjPVoid] Pointer to an empty Pointer Array structure
** @category new [AjPVoid] Default constructor
** @@
******************************************************************************/

AjPVoid ajVoidNew(void)
{
    AjPVoid thys = NULL;

    AJNEW0(thys);

    thys->Ptr = AJALLOC0(RESERVED_SIZE * sizeof(void *));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    /*
    ** FIXME: This works only in ajarr.c
    arrTotal++;
    arrAlloc += RESERVED_SIZE * sizeof(void *);
    */

    return thys;
}




/* @func ajVoidNewRes *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPVoid] Pointer to an empty Pointer Array struct
**                   of specified size
** @category new [AjPVoid] Constructor with reserved size
** @@
******************************************************************************/

AjPVoid ajVoidNewRes(ajuint size)
{
    AjPVoid thys = NULL;

    size = ajRound(size, RESERVED_SIZE);

    AJNEW0(thys);

    thys->Ptr = AJALLOC0(size * sizeof(void *));
    thys->Len = 0;
    thys->Res = size;

    /*
    ** FIXME: This works only in ajarr.c
    arrTotal++;
    arrAlloc += size * sizeof(void *);
    */

    return thys;
}




/* @section destructors *******************************************************
**
** Functions for destruction of void pointer arrays.
**
** @fdata [AjPVoid]
**
** @nam3rule Del Destroy (free) an existing AjPVoid
**
** @argrule Del thys [AjPVoid*] Void pointer array
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ajVoidDel ************************************************************
**
** Default destructor for AJAX Pointer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPVoid*] Pointer to the Pointer Array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPVoid] Default destructor
** @@
******************************************************************************/

void ajVoidDel(AjPVoid *thys)
{
    if(!thys || !*thys)
        return;

    /*ajDebug("ajVoidDel Len %u Res %u\n",
      (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    /*
    ** FIXME: This works only in ajarr.c
    arrFreeCount++;
    */

    return;
}




/* @section Cast **************************************************************
**
** Functions for returning elements of an AJAX SQL Connection object.
**
** @fdata [AjPVoid]
**
** @nam3rule Get Return AJAX SQL Connection elements
** @nam3rule Len Return the length
**
** @argrule * thys [const AjPVoid] AJAX void pointer array
** @argrule Get elem [ajuint] Element number
**
** @valrule Get [void*] Void array
** @valrule Len [ajuint] Length
**
** @fcategory cast
******************************************************************************/




/* @func ajVoidGet ************************************************************
**
** Retrieve an element from an AJAX Pointer Array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPVoid] Pointer to the Pointer array
** @param  [r] elem [ajuint] array element
**
** @return [void*] contents of array element
** @category cast [AjPVoid] Retrieve an address from an array
** @@
******************************************************************************/

void* ajVoidGet(const AjPVoid thys, ajuint elem)
{
    if(!thys || elem >= thys->Len)
        ajErr("Attempt to access bad Pointer array index %d\n", elem);

    return thys->Ptr[elem];
}




/* @func ajVoidLen ************************************************************
**
** Get length of dynamic 1d AJAX Pointer Array.
**
** @param [r] thys [const AjPVoid] AJAX Pointer Array
** @return [ajuint] length
** @@
******************************************************************************/

ajuint ajVoidLen(const AjPVoid thys)
{
    return thys->Len;
}




/* @section modifiers *********************************************************
**
** @fdata [AjPVoid]
**
** @nam3rule Put Put a value in an array
** @nam3rule Resize Resize array
**
** @argrule * thys [AjPVoid*] Void pointer array
** @argrule Resize size [ajuint] New size of array
** @argrule Put elem [ajuint] Element number
** @argrule Put v [void*] Value to load
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
******************************************************************************/




/* @func ajVoidPut ************************************************************
**
** Load a void pointer array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPVoid*] Pointer to the void pointer array
** @param  [r] elem [ajuint] array element
** @param  [r] v [void*] value to load
**
** @return [AjBool] true if the array was extended
** @category modify [AjPChar] Load a character array element
** @@
******************************************************************************/

AjBool ajVoidPut(AjPVoid *thys, ajuint elem, void *v)
{
    if(!thys || !*thys)
        ajErr("Attempt to write to illegal array value %d\n", elem);

    if(elem < (*thys)->Res)
    {
        if(elem >= (*thys)->Len)
            (*thys)->Len = elem + 1;

        (*thys)->Ptr[elem] = v;

        return ajFalse;
    }

    arrVoidResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}





/* @funcstatic arrVoidResize **************************************************
**
** Resize a void pointer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPVoid*] Pointer to the void pointer array
** @param  [r] size [ajuint] new size
**
** @return [AjBool] true if the array was extended
** @@
******************************************************************************/

static AjBool arrVoidResize(AjPVoid *thys, ajuint size)
{
    AjPVoid p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
        ajErr("Illegal attempt to resize void pointer array");

    clen = ajRound((*thys)->Len - 1, RESERVED_SIZE);
    s = ajRound(size + 1, RESERVED_SIZE);
    if(s <= clen)
        return ajFalse;

    /*ajDebug("ajVoidResize %d (%d) -> %d (%d)\n",
      (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajVoidNewRes(s);

    if(size < p->Len)
        limit = size + 1;
    else
        limit = p->Len;

    memmove((*thys)->Ptr, p->Ptr, limit * sizeof(void *));

    (*thys)->Len = size + 1;

    ajVoidDel(&p);

    /*
    ** FIXME: This works only in ajarr.c
    arrResize++;
    */

    return ajTrue;
}

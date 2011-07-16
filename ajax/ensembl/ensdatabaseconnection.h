
#ifndef ENSDATABASECONNECTION_H
#define ENSDATABASECONNECTION_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPDatabaseconnection ***********************************************
**
** Ensembl Database Connection.
**
** Holds information to connect to a particular SQL database on a
** relational database management system (RDBMS) instance.
**
** @alias EnsSDatabaseconnection
** @alias EnsODatabaseconnection
**
** @attr Sqlconnection [AjPSqlconnection] AJAX SQL Connection
** @attr Username [AjPStr] User name
** @attr Password [AjPStr] Password
** @attr Hostname [AjPStr] Host name or IP address
** @attr Hostport [AjPStr] Host TCP/IP port
** @attr Socketfile [AjPStr] UNIX socket file
** @attr Databasename [AjPStr] SQL database name
** @attr Autodisconnect [AjBool] Automatic disconnections
** @attr Sqlconnectionclient [AjESqlconnectionClient] AJAX SQL
**                                                    Connection client
** @attr Timeout [ajuint] Timeout in seconds for closing idle, non-interactive
**                        connections
** @attr Use [ajuint] Use counter
**
** @@
******************************************************************************/

typedef struct EnsSDatabaseconnection
{
    AjPSqlconnection Sqlconnection;
    AjPStr Username;
    AjPStr Password;
    AjPStr Hostname;
    AjPStr Hostport;
    AjPStr Socketfile;
    AjPStr Databasename;
    AjBool Autodisconnect;
    AjESqlconnectionClient Sqlconnectionclient;
    ajuint Timeout;
    ajuint Use;
} EnsODatabaseconnection;

#define EnsPDatabaseconnection EnsODatabaseconnection*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

EnsPDatabaseconnection ensDatabaseconnectionNewCpy(
    EnsPDatabaseconnection dbc,
    AjPStr database);

EnsPDatabaseconnection ensDatabaseconnectionNewIni(
    AjESqlconnectionClient client,
    AjPStr user,
    AjPStr password,
    AjPStr host,
    AjPStr port,
    AjPStr socketfile,
    AjPStr database,
    ajuint dbctimeout);

EnsPDatabaseconnection ensDatabaseconnectionNewRef(EnsPDatabaseconnection dbc);

EnsPDatabaseconnection ensDatabaseconnectionNewUrl(const AjPStr url);

void ensDatabaseconnectionDel(EnsPDatabaseconnection* Pdbc);

AjBool ensDatabaseconnectionGetAutodisconnect(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetDatabasename(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetHostname(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetHostport(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetPassword(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetSocketfile(
    const EnsPDatabaseconnection dbc);

AjPSqlconnection ensDatabaseconnectionGetSqlconnection(
    const EnsPDatabaseconnection dbc);

AjESqlconnectionClient ensDatabaseconnectionGetSqlconnectionclient(
    const EnsPDatabaseconnection dbc);

ajuint ensDatabaseconnectionGetTimeout(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetUsername(
    const EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionSetAutodisconnect(
    EnsPDatabaseconnection dbc,
    AjBool autodisconnect);

AjBool ensDatabaseconnectionTrace(const EnsPDatabaseconnection dbc,
                                  ajuint level);

AjBool ensDatabaseconnectionIsConnected(const EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionMatch(const EnsPDatabaseconnection dbc1,
                                  const EnsPDatabaseconnection dbc2);

AjBool ensDatabaseconnectionConnect(EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionDisconnect(EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionEscapeC(EnsPDatabaseconnection dbc,
                                    char** Ptxt,
                                    const AjPStr str);

AjBool ensDatabaseconnectionEscapeS(EnsPDatabaseconnection dbc,
                                    AjPStr* Pstr,
                                    const AjPStr str);

AjBool ensDatabaseconnectionSqlstatementDel(
    EnsPDatabaseconnection dbc,
    AjPSqlstatement* Psqls);

AjPSqlstatement ensDatabaseconnectionSqlstatementNew(
    EnsPDatabaseconnection dbc,
    const AjPStr statement);

AjBool ensDatabaseconnectionFetchUrl(const EnsPDatabaseconnection dbc,
                                     AjPStr* Purl);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSDATABASECONNECTION_H */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensdatabaseconnection_h
#define ensdatabaseconnection_h

#include "ajax.h"




/* @data EnsPDatabaseconnection ***********************************************
**
** Ensembl Database Connection.
**
** Holds information to connect to an SQL database.
**
** @alias EnsSDatabaseconnection
** @alias EnsODatabaseconnection
**
** @attr Sqlconnection [AjPSqlconnection] AJAX SQL Connection
** @attr UserName [AjPStr] SQL user name
** @attr Password [AjPStr] SQL password
** @attr HostName [AjPStr] SQL host name
** @attr HostPort [AjPStr] SQL host port
** @attr Socket [AjPStr] SQL host UNIX socket
** @attr DatabaseName [AjPStr] SQL database name
** @attr SqlClientType [AjEnum] AJAX SQL client type
** @attr Use [ajuint] Use counter
**
** @@
******************************************************************************/

typedef struct EnsSDatabaseconnection
{
    AjPSqlconnection Sqlconnection;
    AjPStr UserName;
    AjPStr Password;
    AjPStr HostName;
    AjPStr HostPort;
    AjPStr Socket;
    AjPStr DatabaseName;
    AjEnum SqlClientType;
    ajuint Use;
} EnsODatabaseconnection;

#define EnsPDatabaseconnection EnsODatabaseconnection*




/*
** Prototype definitions
*/

EnsPDatabaseconnection ensDatabaseconnectionNew(AjEnum client,
                                                AjPStr user,
                                                AjPStr password,
                                                AjPStr host,
                                                AjPStr port,
                                                AjPStr socket,
                                                AjPStr database);

EnsPDatabaseconnection ensDatabaseconnectionNewC(EnsPDatabaseconnection dbc,
                                                 AjPStr database);

EnsPDatabaseconnection ensDatabaseconnectionNewRef(EnsPDatabaseconnection dbc);

void ensDatabaseconnectionDel(EnsPDatabaseconnection* Pdbc);

AjPSqlconnection ensDatabaseconnectionGetSqlconnection(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetUserName(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetPassword(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetHostName(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetHostPort(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetSocket(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetDatabaseName(const EnsPDatabaseconnection dbc);

AjEnum ensDatabaseconnectionGetSqlClientType(const EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionMatch(const EnsPDatabaseconnection dbc1,
                                  const EnsPDatabaseconnection dbc2);

AjBool ensDatabaseconnectionConnect(EnsPDatabaseconnection dbc);

void ensDatabaseconnectionDisconnect(EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionIsConnected(const EnsPDatabaseconnection dbc);

AjPSqlstatement ensDatabaseconnectionSqlstatementNew(EnsPDatabaseconnection dbc,
                                                     const AjPStr statement);

AjBool ensDatabaseconnectionEscapeC(EnsPDatabaseconnection dbc,
                                     char **Ptxt,
                                     const AjPStr str);

AjBool ensDatabaseconnectionEscapeS(EnsPDatabaseconnection dbc,
                                     AjPStr *Pstr,
                                     const AjPStr str);

AjBool ensDatabaseconnectionTrace(const EnsPDatabaseconnection dbc,
                                  ajuint level);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

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
** Holds information to connect to a particular SQL database on a
** relational database management system (RDBMS) instance.
**
** @alias EnsSDatabaseconnection
** @alias EnsODatabaseconnection
**
** @attr Sqlconnection [AjPSqlconnection] AJAX SQL Connection
** @attr UserName [AjPStr] User name
** @attr Password [AjPStr] Password
** @attr HostName [AjPStr] Host name or IP address
** @attr HostPort [AjPStr] Host TCP/IP port
** @attr SocketFile [AjPStr] UNIX socket file
** @attr DatabaseName [AjPStr] SQL database name
** @attr AutoDisconnect [AjBool] Automatic disconnections
** @attr SqlconnectionClient [AjESqlconnectionClient] AJAX SQL
**                                                    Connection client
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
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
    AjPStr SocketFile;
    AjPStr DatabaseName;
    AjBool AutoDisconnect;
    AjESqlconnectionClient SqlconnectionClient;
    ajuint Use;
    ajuint Padding;
} EnsODatabaseconnection;

#define EnsPDatabaseconnection EnsODatabaseconnection*




/*
** Prototype definitions
*/

EnsPDatabaseconnection ensDatabaseconnectionNew(AjESqlconnectionClient client,
                                                AjPStr user,
                                                AjPStr password,
                                                AjPStr host,
                                                AjPStr port,
                                                AjPStr socketfile,
                                                AjPStr database);

EnsPDatabaseconnection ensDatabaseconnectionNewC(EnsPDatabaseconnection dbc,
                                                 AjPStr database);

EnsPDatabaseconnection ensDatabaseconnectionNewRef(EnsPDatabaseconnection dbc);

EnsPDatabaseconnection ensDatabaseconnectionNewUrl(const AjPStr url);

void ensDatabaseconnectionDel(EnsPDatabaseconnection* Pdbc);

AjPSqlconnection ensDatabaseconnectionGetSqlconnection(
    const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetUserName(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetPassword(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetHostName(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetHostPort(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetSocketFile(const EnsPDatabaseconnection dbc);

AjPStr ensDatabaseconnectionGetDatabaseName(const EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionGetAutoDisconnect(
    const EnsPDatabaseconnection dbc);

AjESqlconnectionClient ensDatabaseconnectionGetSqlconnectionClient(
    const EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionSetAutoDisconnect(EnsPDatabaseconnection dbc,
                                              AjBool autodisconnect);

AjBool ensDatabaseconnectionMatch(const EnsPDatabaseconnection dbc1,
                                  const EnsPDatabaseconnection dbc2);

AjBool ensDatabaseconnectionConnect(EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionDisconnect(EnsPDatabaseconnection dbc);

AjBool ensDatabaseconnectionIsConnected(const EnsPDatabaseconnection dbc);

AjPSqlstatement ensDatabaseconnectionSqlstatementNew(
    EnsPDatabaseconnection dbc,
    const AjPStr statement);

AjBool ensDatabaseconnectionSqlstatementDel(
    EnsPDatabaseconnection dbc,
    AjPSqlstatement *Psqls);

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




#endif /* ensdatabaseconnection_h */

#ifdef __cplusplus
}
#endif

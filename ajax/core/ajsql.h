/* @include ajsql *************************************************************
**
** AJAX SQL functions
**
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.14 $
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#ifndef AJSQL_H
#define AJSQL_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajarr.h"
#include "ajtime.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPVoid **************************************************************
**
** Ajax address object.
**
** Holds a void array with additional data.
** The length is known and held internally.
**
** Saves on length calculation, and allows growth in reserved memory without
** changing the pointer in the calling routine.
**
** AjPVoid is implemented as a pointer to a C data structure.
**
** @alias AjSVoid
** @alias AjOVoid
**
** @new    ajVoidNew Default constructor
** @new    ajVoidNewRes Constructor with reserved size
** @delete ajVoidDel Default destructor
** @cast   ajVoidGet Retrieve a pointer from an array
** @modify ajVoidPut Load a pointer array element
**
** @attr Res [ajuint] Reserved space in case of extension
** @attr Len [ajuint] Actual length used
** @attr Ptr [void**] Array of void pointers
** @@
******************************************************************************/

typedef struct AjSVoid
{
    ajuint Res;
    ajuint Len;
    void **Ptr;
} AjOVoid;

#define AjPVoid AjOVoid*




/* @enum AjESqlconnectionClient ***********************************************
**
** AJAX SQL Connection client library enumeration.
**
** @alias AjOSqlconnectionClient
**
** @value ajESqlconnectionClientNULL       Null
** @value ajESqlconnectionClientMySQL      MySQL client
** @value ajESqlconnectionClientPostgreSQL PostgreSQL client
******************************************************************************/

typedef enum AjOSqlconnectionClient
{
    ajESqlconnectionClientNULL,
    ajESqlconnectionClientMySQL,
    ajESqlconnectionClientPostgreSQL
} AjESqlconnectionClient;




/* @data AjPSqlconnection *****************************************************
**
** AJAX SQL Connection.
**
** Holds the client type and a pointer to a SQL client library-specific
** connection object.
**
** @alias AjSSqlconnection
** @alias AjOSqlconnection
**
** @attr Pconnection [void*]  SQL client library-specific connection object
**                            (MYSQL*) for the MySQL client library
**                            (PGconn*) for the PostgreSQL client library
** @attr Client [AjESqlconnectionClient] Client library
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct AjSSqlconnection
{
    void *Pconnection;
    AjESqlconnectionClient Client;
    ajuint Use;
} AjOSqlconnection;

#define AjPSqlconnection AjOSqlconnection*




/* @data AjPSqlstatement ******************************************************
**
** AJAX SQL Statement.
**
** Holds pointers to an AJAX SQL Connection and to a client library-specific
** result object.
**
** @alias AjSSqlstatement
** @alias AjOSqlstatement
**
** @attr Sqlconnection [AjPSqlconnection] AJAX SQL Connection.
** @attr Presult [void*] SQL client library-specific result object
**                        (MYSQL_RES*) for the MySQL client library
**                        (PGresult*) for the PostgreSQL client library
** @attr AffectedRows [ajulong] Number of rows affected by non-SELECT SQL
**                              statements
** @attr SelectedRows [ajulong] Number of rows selected by SELECT-like SQL
**                              statements
** @attr Columns [ajuint] Number of columns returned by SELECT-like statements
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct AjSSqlstatement
{
    AjPSqlconnection Sqlconnection;
    void *Presult;
    ajulong AffectedRows;
    ajulong SelectedRows;
    ajuint Columns;
    ajuint Use;
} AjOSqlstatement;

#define AjPSqlstatement AjOSqlstatement*




/* @data AjPSqlrow ************************************************************
**
** AJAX SQL Result Row.
**
** Holds an AjPChar array of C-type character strings and an AjLong array of
** data lengths for each column data values in SQL client library-specific
** result objects.
**
** @alias AjSSqlrow
** @alias AjOSqlrow
**
** @attr Values [AjPVoid] AJAX Character Array of SQL column values
** @attr Lengths [AjPLong] AJAX Long Integer Array of SQL column value lengths
** @attr Columns [ajuint] Number of columns per row
** @attr Current [ajuint] Current column in column interactions
** @@
******************************************************************************/

typedef struct AjSSqlrow
{
    AjPVoid Values;
    AjPLong Lengths;
    ajuint Columns;
    ajuint Current;
} AjOSqlrow;

#define AjPSqlrow AjOSqlrow*




/* @data AjISqlrow ************************************************************
**
** AJAX SQL Row Iterator.
**
** Allows iteration over AJAX SQL Rows of an AJAX SQL Statement.
**
** @alias AjSSqlrowiter
** @alias AjOSqlrowiter
** @alias AjPSqlrowiter
**
** @attr Sqlstatement [AjPSqlstatement] AJAX SQL Statement
** @attr Sqlrow [AjPSqlrow] AJAX SQL Result Row
** @attr Current [ajulong] Current row number
** @@
******************************************************************************/

typedef struct AjSSqlrowiter
{
    AjPSqlstatement Sqlstatement;
    AjPSqlrow Sqlrow;
    ajulong Current;
} AjOSqlrowiter;

#define AjISqlrow AjOSqlrowiter*
#define AjPSqlrowiter AjOSqlrowiter*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjBool ajSqlInit(void);

void ajSqlExit(void);

/* AJAX SQL Connection */

AjPSqlconnection ajSqlconnectionNewData(AjESqlconnectionClient client,
                                        const AjPStr user,
                                        const AjPStr password,
                                        const AjPStr host,
                                        const AjPStr port,
                                        const AjPStr socketfile,
                                        const AjPStr database);

AjPSqlconnection ajSqlconnectionNewRef(AjPSqlconnection sqlc);

void ajSqlconnectionDel(AjPSqlconnection* Psqlc);

AjESqlconnectionClient ajSqlconnectionGetClient(const AjPSqlconnection sqlc);

ajuint ajSqlconnectionGetUse(const AjPSqlconnection sqlc);

AjBool ajSqlconnectionTrace(const AjPSqlconnection sqlc, ajuint level);

AjBool ajSqlconnectionEscapeC(const AjPSqlconnection sqlc,
                              char **Ptxt,
                              const AjPStr str);

AjBool ajSqlconnectionEscapeS(const AjPSqlconnection sqlc,
                              AjPStr *Pstr,
                              const AjPStr str);

AjESqlconnectionClient ajSqlconnectionClientFromStr(const AjPStr client);

const char* ajSqlconnectionClientToChar(AjESqlconnectionClient client);

/* AJAX SQL Statement */

AjPSqlstatement ajSqlstatementNewRun(AjPSqlconnection sqlc,
                                     const AjPStr statement);

AjPSqlstatement ajSqlstatementNewRef(AjPSqlstatement sqls);

void ajSqlstatementDel(AjPSqlstatement* Psqls);

ajulong ajSqlstatementGetAffectedrows(const AjPSqlstatement sqls);

ajulong ajSqlstatementGetSelectedrows(const AjPSqlstatement sqls);

ajuint ajSqlstatementGetColumns(const AjPSqlstatement sqls);

ajuint ajSqlstatementGetIdentifier(const AjPSqlstatement sqls);

/* AJAX SQL Row Iterator */

AjISqlrow ajSqlrowiterNew(AjPSqlstatement sqls);

void ajSqlrowiterDel(AjISqlrow *Psqli);

AjBool ajSqlrowiterDone(const AjISqlrow sqli);

AjPSqlrow ajSqlrowiterGet(AjISqlrow sqli);

AjBool ajSqlrowiterRewind(AjISqlrow sqli);

/* AJAX SQL Row */

AjPSqlrow ajSqlrowNew(ajuint columns);

void ajSqlrowDel(AjPSqlrow *Psqlr);

AjPVoid ajSqlrowGetValues(const AjPSqlrow sqlr);

AjPLong ajSqlrowGetLengths(const AjPSqlrow sqlr);

ajuint ajSqlrowGetColumns(const AjPSqlrow sqlr);

ajuint ajSqlrowGetCurrent(const AjPSqlrow sqlr);

AjBool ajSqlcolumnGetValue(AjPSqlrow sqlr, void **Pvalue,
                           ajulong *Plength);

AjBool ajSqlcolumnToStr(AjPSqlrow sqlr, AjPStr *Pvalue);

AjBool ajSqlcolumnToInt(AjPSqlrow sqlr, ajint *Pvalue);

AjBool ajSqlcolumnToUint(AjPSqlrow sqlr, ajuint *Pvalue);

AjBool ajSqlcolumnToLong(AjPSqlrow sqlr, ajlong *Pvalue);

AjBool ajSqlcolumnToUlong(AjPSqlrow sqlr, ajulong *Pvalue);

AjBool ajSqlcolumnToFloat(AjPSqlrow sqlr, float *Pvalue);

AjBool ajSqlcolumnToDouble(AjPSqlrow sqlr, double *Pvalue);

AjBool ajSqlcolumnToBool(AjPSqlrow sqlr, AjBool *Pvalue);

AjBool ajSqlcolumnToTime(AjPSqlrow sqlr, AjPTime *Pvalue);

AjBool ajSqlcolumnRewind(AjPSqlrow sqlr);

AjBool ajSqlcolumnNumberGetValue(const AjPSqlrow sqlr,
                                 ajuint column,
                                 void **Pvalue,
                                 ajulong *Plength);

AjBool ajSqlcolumnNumberToStr(const AjPSqlrow sqlr, ajuint column,
                              AjPStr *Pvalue);

AjBool ajSqlcolumnNumberToInt(const AjPSqlrow sqlr, ajuint column,
                              ajint *Pvalue);

AjBool ajSqlcolumnNumberToUint(const AjPSqlrow sqlr, ajuint column,
                               ajuint *Pvalue);

AjBool ajSqlcolumnNumberToLong(const AjPSqlrow sqlr, ajuint column,
                               ajlong *Pvalue);

AjBool ajSqlcolumnNumberToFloat(const AjPSqlrow sqlr, ajuint column,
                                float *Pvalue);

AjBool ajSqlcolumnNumberToDouble(const AjPSqlrow sqlr, ajuint column,
                                 double *Pvalue);

AjBool ajSqlcolumnNumberToBool(const AjPSqlrow sqlr, ajuint column,
                               AjBool *Pvalue);

AjBool ajSqlcolumnNumberToTime(const AjPSqlrow sqlr, ajuint column,
                               AjPTime *Pvalue);

AjBool ajSqlcolumnNumberToUlong(const AjPSqlrow sqlr, ajuint column,
                                ajulong *Pvalue);

AjBool ajSqlcolumnNumberIsDefined(const AjPSqlrow sqlr, ajuint column);

/* AJAX Void Array */

AjPVoid ajVoidNew(void);

AjPVoid ajVoidNewRes(ajuint size);

void ajVoidDel(AjPVoid *thys);

void *ajVoidGet(const AjPVoid thys, ajuint elem);

ajuint ajVoidLen(const AjPVoid thys);

AjBool ajVoidPut(AjPVoid *thys, ajuint elem, void *v);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJSQL_H */

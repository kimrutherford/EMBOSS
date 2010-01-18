/******************************************************************************
** @source sqltest application
** Alphabetically minimises tandem repeat sequences.
** @author Copyright (C) Michael K. Schuster
** Department of Medical Biochemistry University Vienna
** @author Copyright (C) Martin Grabner
** (martin.grabner@univie.ac.at) EMBnet Austria
** @version 1.0
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"




/* @prog sqltest **************************************************************
** Test a connection to an SQL server via AJAX ajsql library functions.
******************************************************************************/

int main(int argc, char **argv)
{
    void *value = NULL;
    
    ajulong length = 0;
    ajulong i = 0;
    ajuint j = 0;
    
    AjEnum client = ajESqlClientMySQL;
    
    AjPFile outf = NULL;
    
    AjPStr user = NULL;
    AjPStr password = NULL;
    AjPStr host = NULL;
    AjPStr port = NULL;
    AjPStr socket = NULL;
    AjPStr dbname = NULL;
    AjPStr statement = NULL;
    AjPStr result = NULL;
    
    AjPSqlconnection sqlc = NULL;
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli = NULL;
    AjPSqlrow row = NULL;
    
    embInit("sqltest", argc, argv);
    
    user = ajStrNewC("anonymous");
    password = ajStrNew();
    host = ajStrNewC("ensembldb.ensembl.org");
    port = ajStrNewC("3306");
    socket = ajStrNew();
    dbname = ajStrNew();
    statement = ajStrNewC("SHOW PROCESSLIST");
    result = ajStrNew();
    outf = ajAcdGetOutfile("outfile");
    
    if(!ajSqlInit())
	ajFatal("Library initialisation failed.");
    
    sqlc =
	ajSqlconnectionNewData(client, user, password, host, port, socket,
                               dbname);
    
    if(!sqlc)
	ajFatal("Could not connect as user '%S' to server '%S' at port '%S' "
                "to database '%S'", user, host, port, dbname);
    
    ajFmtPrintF(outf, "Connection to client %d\n",
		ajSqlconnectionGetClient(sqlc));
    
    sqls = ajSqlstatementNewRun(sqlc, statement);
    
    if(!sqls)
	ajFatal("SQL Statement did not complete: %S", statement);
    
    ajFmtPrintF(outf,
		"SQL Statement: %S\n",
		statement);
    
    ajFmtPrintF(outf,
		"Number of selected rows: %Lu\n",
		ajSqlstatementGetSelectedrows(sqls));
    
    ajFmtPrintF(outf,
		"Number of columns: %u\n",
		ajSqlstatementGetColumns(sqls));
    
    sqli = ajSqlrowiterNew(sqls);
    
    if(!sqli)
	ajFatal("Query Iterator not defined.");
    
    while(!ajSqlrowiterDone(sqli))
    {
	row = ajSqlrowiterGet(sqli);
	
	if(row)
	{
	    ajDebug("main SQL Row %Lu Columns %u Current %u\n",
		    i, row->Columns, row->Current);
	    
	    for(j = 0; j < ajSqlstatementGetColumns(sqls); j++)
	    {
		ajStrAssignClear(&result);
		
		if(ajSqlcolumnNumberGetValue(row, j, &value, &length))
		    ajDebug("main SQL Row %Lu Column %u Value %p Length %d\n",
			    i, j, value, length);
		
		ajSqlcolumnNumberToStr(row, j, &result);
		
		ajDebug("main SQL Row %Lu Column %u String '%S'\n",
			i, j, result);
		
		ajFmtPrintF(outf, "Row: %Lu Column: %u String: '%S'\n", i, j,
                            result);
	    }
	}
	
	else
	    ajDebug("main No row returned!\n");
	
	i++;
    }
    
    ajSqlrowiterDel(&sqli);
    ajSqlstatementDel(&sqls);
    ajSqlconnectionDel(&sqlc);
    
    ajFileClose(&outf);
    
    ajStrDel(&user);
    ajStrDel(&password);
    ajStrDel(&host);
    ajStrDel(&port);
    ajStrDel(&socket);
    ajStrDel(&dbname);
    ajStrDel(&statement);
    ajStrDel(&result);
    
    embExit();
    
    return 0;
}

/* @source dbfetchtest application
 **
 ** Run example queries of EBI WS/Dbfetch databases
 **
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
#include "ajsoap.h"



/* @datastatic dbinfo *********************************************************
**
** Database information for cachedbfetch
**
** @attr name [AjPStr] Name on server
** @attr displayname [AjPStr] Displayed name
** @attr format [AjPStr] Format
** @attr dbtype [AjPStr] Database type
** @attr description [AjPStr] Description
** @attr example [AjPStr] Example
******************************************************************************/

typedef struct dbinfo
{
    AjPStr name;
    AjPStr displayname;
    AjPStr format;
    AjPStr dbtype;
    AjPStr description;
    AjPStr example;
} dbOinfo;

#define dbPinfo dbOinfo*




static AjBool  dbfetchtest_Test(const AjPStr server,
                                const char* db, const char* id);
static AjPList dbfetchtest_GetDbList(const AjPStr servername);




/* @prog dbfetchtest *******************************************************
 **
 ** Experimental application to explore wsdbfetch databases.
 ** Can be used for testing as well; -runtestqueries option.
 **
 *****************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf = NULL;

    AjPStr query = NULL;
    AjPList l    = NULL;
    AjIList iter = NULL;
    dbPinfo db   = NULL;

    AjPStr servername   = NULL;

    AjPStr url = NULL;

    ajint tested   = 0;
    ajint noseqret = 0;

    AjBool runtestqueries = ajTrue;

    embInit("dbfetchtest", argc, argv);

    servername = ajAcdGetString("servername");
    outf   = ajAcdGetOutfile("outfile");

    runtestqueries  = ajAcdGetBoolean("runtestqueries");

    if(runtestqueries)
    {

	if(!ajNamServer(servername))
	{
	    ajWarn("following ws/dbfetch server is required to be defined "
		    "for test queries...");
	    ajUser("\nSERVER %S [\n"
		    "   type: \"sequence, features\"\n"
		    "   method: \"ws/dbfetch\"\n"
		    "   url: \"http://www.ebi.ac.uk/"
		    "ws/services/WSDbfetchDoclit\"\n"
		    "]\n",servername);
	    ajWarn("ignoring -runtestqueries option...");
	    runtestqueries = ajFalse;
	}
    }

    l= dbfetchtest_GetDbList(servername);

    if (l)
    {

	iter = ajListIterNew(l);

	while(!ajListIterDone(iter))
	{
	    db = (dbPinfo)ajListIterGet(iter);

	    ajFmtPrintF(outf,"%S\t%S\n",db->name, db->description);

	    if( runtestqueries && ajStrFindC(db->format, "fasta") != -1)
	    {
		const AjPStr id = ajStrParseC(db->example, ", ");

		tested++;

		if(!dbfetchtest_Test(servername, ajStrGetPtr(db->name),
                                     ajStrGetPtr(id)))
                    noseqret++;
	    }

	    ajStrDel(&db->format);
	    ajStrDel(&db->description);
	    ajStrDel(&db->displayname);
	    ajStrDel(&db->example);
	    ajStrDel(&db->name);
	    ajStrDel(&db->dbtype);
	    AJFREE(db);
	}

	ajFmtPrintF(outf,
		"#dbs: %Lu    #tested: %d    #no-sequence-returned: %d\n",
		ajListGetLength(l), tested, noseqret);

	ajListFree(&l);
	ajListIterDel(&iter);
    }


    ajFileClose(&outf);

    ajStrDel(&query);
    ajStrDel(&url);

    ajStrDel(&servername);

    embExit();

    return 0;
}




/* @funcstatic dbfetchtest_Test ***********************************************
**
** Makes wsdbfetch sequence query for the given database and query identifier.
**
** @param [r] server [const AjPStr] wsdbfetch server name
** @param [r] db [const char*] wsdbfetch database name
** @param [r] id [const char*] entry identifier for the wsdbfetch database
** @return [AjBool] returns true if the query returned any sequence
** @@
******************************************************************************/

static AjBool dbfetchtest_Test(const AjPStr server, const char* db, const char* id)
{
    AjPStr idqry = NULL;
    AjPSeq seq   = NULL;

    AjBool ret = ajTrue;

    /* todo: test various formats and data types */

    ajFmtPrintS(&idqry,"fasta::%S:%s:%s", server, db,id);

    ajUser("query USA: '%S'\n",idqry);

    seq = ajSeqNew();

    ajSeqGetFromUsa(idqry, ajFalse, seq);

    ajUser("legth of sequence returned: %d\n",
	    ajSeqGetLen(seq));

    ajStrDel(&idqry);

    if(!ajSeqGetLen(seq))
	ret = ajFalse;
    else if(ajSeqGetLen(seq)>100)
	ajUser("sequence returned (last 100 bases): %s\n",
	       ajSeqGetSeqC(seq)+seq->Seq->Len-100);
    else
	ajUser("sequence returned: %s\n", ajSeqGetSeqC(seq));

    ajSeqDel(&seq);

    return ret;
}




/* @funcstatic dbfetchtest_GetDbList ***************************************
**
** Parses the dbfetch metadata query response
**
** @param [r] servername [const AjPStr] Server name
** @return [AjPList] list of identifiers
** @@
******************************************************************************/

static AjPList dbfetchtest_GetDbList(const AjPStr servername)
{
    AjPList dbnames = NULL;
    AjIList dbniter = NULL;
    AjPList dblist  = NULL;
    dbPinfo db = NULL;

    AjPStr dbname = NULL;
    AjPStr value  = NULL;

    dbnames = ajListstrNew();
    dblist  = ajListNew();

    ajNamSvrListListDatabases(servername, dbnames);

    dbniter = ajListIterNew(dbnames);

    while(!ajListIterDone(dbniter))
    {
        dbname = ajListstrIterGet(dbniter);

    	AJNEW0(db);

	ajStrAssignS(&db->name, dbname);

	ajNamSvrGetdbAttrC(servername, dbname, "format", &value);
	ajStrAssignS(&db->format, value);

	ajNamSvrGetdbAttrC(servername, dbname, "example", &value);
	ajStrAssignS(&db->example, value);

	ajNamSvrGetdbAttrC(servername, dbname, "comment", &value);
	ajStrAssignS(&db->description, value);

	ajStrDel(&value);
	ajListPushAppend(dblist,db);
    }

    ajListIterDel(&dbniter);
    ajListFree(&dbnames);

    return dblist;
}


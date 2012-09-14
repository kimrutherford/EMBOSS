/* @source cachedbfetch application
 **
 ** Return list of EBI WSDbfetch databases with other available information
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

#define DBFETCH_DBS "http://www.ebi.ac.uk/Tools/dbfetch/dbfetch/dbfetch.databases?style=xml"

/* @datastatic dbinfo *********************************************************
**
** Database information for cachedbfetch
**
** @attr name [AjPStr] Name on server
** @attr displayname [AjPStr] Displayed name
** @attr format [AjPStr] Format
** @attr fmtterms [AjPStr] Format terms
** @attr dataterms [AjPStr] Data terms
** @attr tpcterms [AjPStr] EDAM topic terms
** @attr dbtype [AjPStr] Database type
** @attr description [AjPStr] Description
** @attr example [AjPStr] Example
******************************************************************************/

typedef struct dbinfo
{
    AjPStr name;
    AjPStr displayname;
    AjPStr format;
    AjPStr fmtterms;
    AjPStr dataterms;
    AjPStr tpcterms;
    AjPStr dbtype;
    AjPStr description;
    AjPStr example;
} dbOinfo;
#define dbPinfo dbOinfo*


static const char* idlists[] = {"accessionList", "entryVersionList",
                                "idList", "nameList", "sequenceVersionList",
                                NULL};



static AjBool cachedbfetch_GetDbtypes(const char* format, AjPTable dbtypes);
static void cachedbfetch_PrintCachefile(const AjPList dbs, AjPFile cachef,
	                                AjBool usedbfetch);
static void cachedbfetch_WriteDBdefinition(AjPFile cachef,
                                           const dbPinfo dbinfo,
                                           AjBool usedbfetch);
static void    cachedbfetch_SetDbtype(dbPinfo dbinfo, AjPTable dbtypes);


#ifdef HAVE_AXIS2C

static AjPList cachedbfetch_wsGetDbList(axiom_node_t *wsResult,
                                        const axutil_env_t *env);
static AjPList cachedbfetch_wsGetDBListInfoCall(const AjPStr url);
static AjPStr cachedbfetch_wsGetExamples(const axutil_env_t *env,
                                         axiom_element_t* elm,
                                         axiom_node_t* node);
static AjPStr cachedbfetch_wsGetFormats(const axutil_env_t *env,
                                        axiom_element_t* elm,
                                        axiom_node_t* node,
                                        AjPTable dbtypes, dbPinfo db);

static void cachedbfetch_wsGetEdamTerms(const axutil_env_t *env,
					axiom_element_t* elm,
					axiom_node_t* dbattnode,
					AjPStr * dbatt);

#endif

static AjPList cachedbfetch_GetDbList(AjPFilebuff buff);
static AjPList cachedbfetch_GetDBListInfoCall(const AjPStr url);
static AjPStr cachedbfetch_GetExamples(AjPDomDocument doc, AjPDomElement elm);
static AjPStr cachedbfetch_GetFormats(AjPDomDocument doc,  AjPDomElement e,
                                      AjPTable dbtypes, dbPinfo db);




/* @prog cachedbfetch *********************************************************
 **
 ** Experimental application to explore wsdbfetch databases.
 ** Can be used for testing as well; -runtestqueries option.
 **
 *****************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf   = NULL;
    AjPFile cachef = NULL;

    AjPStr query = NULL;
    AjPList l    = NULL;
    AjPStr methodentry = NULL;
    AjPStr servername  = NULL;

    AjIList i  = NULL;
    AjPStr url = NULL;

    AjBool usedbfetch = ajTrue;

    embInit("cachedbfetch", argc, argv);

    servername = ajAcdGetString("servername");
    outf   = ajAcdGetOutfile("outfile");
    cachef = ajAcdGetOutfile("cachefile");


    if(!ajNamServer(servername))
    {
	ajWarn("following wsdbfetch server is required to be defined "
		"for test queries...");
	ajUser("\nSERVER %S [\n"
		"   type: \"sequence, features\"\n"
		"   method: \"wsdbfetch\"\n"
		"   url: \"http://www.ebi.ac.uk/"
		"ws/services/WSDbfetchDoclit\"\n"
		"]\n",servername);
    }

    ajNamSvrGetAttrC(servername, "methodentry", &methodentry);

    if (!ajStrMatchC(methodentry, "dbfetch"))
	usedbfetch = ajFalse;

    if(usedbfetch)
    {
	    ajStrAssignC(&url, DBFETCH_DBS);
	    l= cachedbfetch_GetDBListInfoCall(url);
    }
#ifdef HAVE_AXIS2C
    else
    {

	if(!ajNamSvrGetUrl(servername, &url))
	    url = ajStrNewC(WSDBFETCH_EP);

	l = cachedbfetch_wsGetDBListInfoCall(url);

	if(l==NULL)
	    ajErr("wsdbfetch webservices call returned null");
    }
#endif

    cachedbfetch_PrintCachefile(l, cachef, usedbfetch);

    if (l)
    {
	i = ajListIterNew(l);

	while(!ajListIterDone(i))
	{
	    dbPinfo dbinfo = ajListIterGet(i);
	    ajStrDel(&dbinfo->format);
	    ajStrDel(&dbinfo->description);
	    ajStrDel(&dbinfo->displayname);
	    ajStrDel(&dbinfo->example);
	    ajStrDel(&dbinfo->name);
	    ajStrDel(&dbinfo->dbtype);
	    ajStrDel(&dbinfo->dataterms);
	    ajStrDel(&dbinfo->fmtterms);
	    ajStrDel(&dbinfo->tpcterms);
	    AJFREE(dbinfo);
	}

	ajListFree(&l);
	ajListIterDel(&i);
    }

    ajFileClose(&outf);
    ajFileClose(&cachef);

    ajStrDel(&query);
    ajStrDel(&url);

    ajStrDel(&servername);
    ajStrDel(&methodentry);

    embExit();

    return 0;
}




#ifdef HAVE_AXIS2C
/* @funcstatic cachedbfetch_wsGetDBListInfoCall *******************************
**
** Makes WSdbfetch "getDatabaseInfoList" call and returns list of dbinfo objs.
**
** @param [r] url [const AjPStr] endpoint for the wsdbfetch webservice
** @return [AjPList] list of dbinfo objs
** @@
******************************************************************************/

static AjPList cachedbfetch_wsGetDBListInfoCall(const AjPStr url)
{
    AjPList ret = NULL;

    axiom_node_t *payload = NULL;
    axiom_namespace_t *ns = NULL;
    const axis2_char_t* address = NULL;

    axis2_svc_client_t* client  = NULL;
    axiom_node_t* result        = NULL;

    axutil_env_t * env;

    address = ajStrGetPtr(url);

    ajDebug("wsdbfetch URL: %s\n", address);

    env = ajSoapAxis2GetEnv();
    client = ajSoapAxis2GetClient(env, address);


    ns = axiom_namespace_create(env, WSDBFETCH_NS, "wsdbfetch");
    axiom_element_create(env, NULL, "getDatabaseInfoList", ns, &payload);


    result = ajSoapAxis2Call(client, env, payload);

    if(result != NULL)
    {
	ret = cachedbfetch_wsGetDbList(result, env);
    }

    axis2_svc_client_free(client, env);
    axutil_env_free(env);

    return ret;
}




/* @funcstatic cachedbfetch_wsGetDbList ***************************************
**
** Parses the results of WSdbfetch "getDatabaseInfoList" calls
**
** @param [u] wsResult [axiom_node_t*] axis2 OM object
** 				       obtained by wsdbfetch webservice call
** @param [r] env [const axutil_env_t*] axis2 environment
** @return [AjPList] list of identifiers
** @@
******************************************************************************/

static AjPList cachedbfetch_wsGetDbList(axiom_node_t *wsResult,
                                        const axutil_env_t *env)
{
    AjPList dblist       = NULL;
    AjPTable dbtypes     = NULL;
    axis2_char_t* id     = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;
    axiom_node_t* dbattnode   = NULL;
    axiom_children_iterator_t* dbObjIter  = NULL;
    axiom_children_iterator_t* dbAttrIter = NULL;

    dbPinfo db = NULL;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "getDatabaseInfoListResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return NULL;
    }

    dbtypes = ajTablecharNewConst(10);

    dbObjIter = axiom_element_get_children(elm, env, wsResult);

    if(dbObjIter != NULL)
    {
	dblist = ajListNew();

	while(axiom_children_iterator_has_next(dbObjIter, env))
	{
	    node = axiom_children_iterator_next(dbObjIter, env);

	    if(axiom_node_get_node_type(node, env) == AXIOM_ELEMENT)
	    {
		elm = axiom_node_get_data_element(node, env);

		name = axiom_element_get_localname(elm, env);

		dbAttrIter = axiom_element_get_children(elm, env, node);

		AJNEW0(db);
		db->tpcterms = ajStrNew();
		db->dataterms = ajStrNew();
		db->fmtterms = ajStrNew();

		while(axiom_children_iterator_has_next(dbAttrIter, env))
		{

		    dbattnode = axiom_children_iterator_next(dbAttrIter, env);
		    elm = axiom_node_get_data_element(dbattnode, env);
		    name = axiom_element_get_localname(elm, env);

		    if (ajCharMatchCaseC(name, "displayName"))
		    {
			id = axiom_element_get_text(elm, env, dbattnode);
			ajStrAssignC(&db->displayname, id);
		    }
		    else if (ajCharMatchCaseC(name, "name"))
		    {
			id = axiom_element_get_text(elm, env, dbattnode);
			ajStrAssignC(&db->name, id);
		    }
		    else if (ajCharMatchCaseC(name, "description"))
		    {
			id = axiom_element_get_text(elm, env, dbattnode);
			ajStrAssignC(&db->description, id);
		    }
		    else if (ajCharMatchCaseC(name, "exampleIdentifiers"))
		    {
			db->example = cachedbfetch_wsGetExamples(env, elm,
                                                                 dbattnode);
		    }
		    else if (ajCharMatchCaseC(name, "formatInfoList"))
		    {
			ajTableClear(dbtypes);
			db->format = cachedbfetch_wsGetFormats(env, elm,
                                                               dbattnode,
                                                               dbtypes, db);
			cachedbfetch_SetDbtype(db, dbtypes);
		    }
		    else if (ajCharMatchCaseC(name, "databaseTerms"))
		    {
			cachedbfetch_wsGetEdamTerms(env, elm, dbattnode,
			                            &db->tpcterms);
		    }

		}

		ajListPushAppend(dblist,db);
	    }
	}
    }

    ajTableDel(&dbtypes);

    return dblist;
}




/* @funcstatic cachedbfetch_wsGetEdamTerms ************************************
**
** Extracts EDAM identifiers from given axiom element/node
**
** @param [r] env [const axutil_env_t*] axis2 environment obj
** @param [u] elm [axiom_element_t*] EDAM terms element
** @param [u] dbattnode [axiom_node_t*] EDAM terms node
** @param [u] dbatt [AjPStr*] String for storing extracted EDAM terms
** @return [void]
** @@
******************************************************************************/

static void cachedbfetch_wsGetEdamTerms(const axutil_env_t *env,
					axiom_element_t* elm,
					axiom_node_t* dbattnode, AjPStr* dbatt)
{
    axiom_child_element_iterator_t* i = NULL;
    axiom_node_t* n  = NULL;
    axis2_char_t* id     = NULL;

    i = axiom_element_get_child_elements(elm,env,dbattnode);

    if(!i)
	return;

    while(axiom_child_element_iterator_has_next(i, env))
    {
	n = axiom_child_element_iterator_next(i, env);
	elm = axiom_node_get_data_element(n, env);
	id = axiom_element_get_text(elm, env, n);

	if (ajStrFindC(*dbatt, id) == -1)
	{
	    if(ajStrGetLen(*dbatt)>0)
		ajStrAppendC(dbatt, ", ");

	    ajStrAppendC(dbatt, id);
	}
    }

    ajDebug("edam terms: %S\n", *dbatt);

    return;
}




/* @funcstatic cachedbfetch_wsGetExamples *************************************
**
** Parses the results of WSdbfetch "getDatabaseInfoList" calls
** for test query identifiers
**
** @param [r] env [const axutil_env_t*] axis2 environment obj
** @param [u] elm [axiom_element_t*] "exampleIdentifiers" element
** @param [u] node [axiom_node_t*] "exampleIdentifiers" node
** @return [AjPStr] list of identifiers
** @@
******************************************************************************/

static AjPStr cachedbfetch_wsGetExamples(const axutil_env_t *env,
                                         axiom_element_t* elm,
                                         axiom_node_t* node)
{
    AjPStr ret = NULL;
    axiom_node_t* n  = NULL;
    axis2_char_t* id = NULL;
    axutil_qname_t* qname = NULL;
    axiom_child_element_iterator_t* i = NULL;
    int j=0;

    ret = ajStrNew();

    while (idlists[j] != NULL)
    {
	qname = axutil_qname_create(env, idlists[j++], WSDBFETCH_NS, "");
	elm = axiom_element_get_first_child_with_qname(elm, env, qname,
	                                               node, &n);
	i = axiom_element_get_child_elements(elm,env,n);

	if(i)
	{
	    while(axiom_child_element_iterator_has_next(i, env))
	    {
		n = axiom_child_element_iterator_next(i, env);
		elm = axiom_node_get_data_element(n, env);
		id = axiom_element_get_text(elm, env, n);

		if(ajStrGetLen(ret)>0)
		    ajStrAppendC(&ret, ", ");

		ajStrAppendC(&ret, id);

	    }
	}

	axutil_qname_free(qname, env);
    }

    return ret;
}




/* @funcstatic cachedbfetch_wsGetFormats **************************************
**
** Parses the results of WSdbfetch "getDatabaseInfoList" calls
** for format names
**
** @param [r] env [const axutil_env_t*] axis2 environment obj
** @param [u] elm [axiom_element_t*] "formatInfoList" element
** @param [u] node [axiom_node_t*] "formatInfoList" node
** @param [u] dbtypes [AjPTable] set of db-types
** @param [u] db [dbPinfo] database metadata
** @return [AjPStr] supported formats in comma separated form
** @@
******************************************************************************/

static AjPStr cachedbfetch_wsGetFormats(const axutil_env_t *env,
                                        axiom_element_t* elm,
                                        axiom_node_t* node,
                                        AjPTable dbtypes, dbPinfo db)
{
    AjPStr ret = NULL;
    axiom_node_t* n  = NULL;
    axis2_char_t* format = NULL;
    axutil_qname_t* qnamename = NULL;
    axutil_qname_t* qnamedataterms = NULL;
    axutil_qname_t* qnamesyntxterms = NULL;
    axiom_child_element_iterator_t* i = NULL;
    axiom_element_t* dataterms = NULL;
    axiom_element_t* syntxterms = NULL;

    ret = ajStrNew();

    i = axiom_element_get_child_elements(elm,env,node);

    if(!i)
	return ret;

    qnamename = axutil_qname_create(env, "name", WSDBFETCH_NS, "");
    qnamedataterms = axutil_qname_create(env, "dataTerms", WSDBFETCH_NS, "");
    qnamesyntxterms = axutil_qname_create(env, "syntaxTerms", WSDBFETCH_NS, "");

    while(axiom_child_element_iterator_has_next(i, env))
    {
	node = axiom_child_element_iterator_next(i, env);
	dataterms = axiom_element_get_first_child_with_qname(elm, env,
	                                                     qnamedataterms,
	                                                     node, &n);

	if(dataterms)
	{
	    dataterms = axiom_node_get_data_element(n, env);
	    cachedbfetch_wsGetEdamTerms(env, dataterms, n, &db->dataterms);
	}

	syntxterms = axiom_element_get_first_child_with_qname(elm, env,
	                                                      qnamesyntxterms,
	                                                      node, &n);

	if(syntxterms)
	{
	    syntxterms = axiom_node_get_data_element(n, env);
	    cachedbfetch_wsGetEdamTerms(env, syntxterms, n, &db->fmtterms);
	}

	elm = axiom_element_get_first_child_with_qname(elm, env, qnamename,
	                                               node, &n);
	elm = axiom_node_get_data_element(n, env);
	format = axiom_element_get_text(elm, env, n);

	if (!format || ajCharMatchC(format, "default"))
	    continue;

	/* adds the format only if it is supported by one of the db types */
	if(cachedbfetch_GetDbtypes(format, dbtypes))
	{
	    if(ajStrGetLen(ret)>0)
		ajStrAppendC(&ret, ", ");

	    ajStrAppendC(&ret, format);
	}
    }

    axutil_qname_free(qnamename, env);
    axutil_qname_free(qnamedataterms, env);
    axutil_qname_free(qnamesyntxterms, env);

    if(!ajStrGetLen(ret))
	ajStrAssignC(&ret,"unknown");

    return ret;
}
#endif




/* @funcstatic cachedbfetch_GetDBListInfoCall *********************************
**
** Makes dbfetch metadata query and returns list of dbinfo objs.
**
** @param [r] url [const AjPStr] url for the dbfetch metadata query
** @return [AjPList] list of dbinfo objs
** @@
******************************************************************************/

static AjPList cachedbfetch_GetDBListInfoCall(const AjPStr url)
{
    AjPList ret = NULL;

    AjPStr host  = NULL;
    AjPStr path  = NULL;

    AjPStr svrhttpver = NULL;

    ajint port = 80;

    AjPFilebuff buff = NULL;

    ajHttpUrlDeconstruct(url, &port, &host, &path);

    buff = ajHttpRead(svrhttpver, NULL, NULL,
                      host, port, path);

    if(buff != NULL)
    {
	ajFilebuffHtmlNoheader(buff);
	ret = cachedbfetch_GetDbList(buff);
    }

    ajFilebuffDel(&buff);
    ajStrDel(&host);
    ajStrDel(&path);

    return ret;
}




/* @funcstatic cachedbfetch_GetDbList *****************************************
**
** Parses the dbfetch metadata query response
**
** @param [u] buff [AjPFilebuff] buffer for the metadata query response
** @return [AjPList] list of identifiers
** @@
******************************************************************************/

static AjPList cachedbfetch_GetDbList(AjPFilebuff buff)
{
    AjPList dblist   = NULL;
    AjPTable dbtypes = NULL;

    AjPDomDocument doc     = NULL;
    AjPDomNodeList dbnodes = NULL;
    AjPDomNode dbnode      = NULL;
    AjPDomElement e    = NULL;

    AjPStr dbterm = NULL;

    dbPinfo db = NULL;

    int i;
    int j;

    doc = ajDomImplementationCreateDocument(NULL,NULL,NULL);

    if (ajDomReadFilebuff(doc,buff) == -1)
    {
	ajDomDocumentDestroyNode(doc,doc);
	return AJFALSE;
    }

    ajFilebuffClear(buff, 0);

    dbnodes = ajDomDocumentGetElementsByTagNameC(doc, "databaseInfo");

    if(dbnodes==NULL || ajDomNodeListGetLen(dbnodes) < 1)
    {
	ajDomDocumentDestroyNodeList(doc,dbnodes,AJDOMKEEP);
	ajDomDocumentDestroyNode(doc,doc);
	return AJFALSE;
    }

    dbtypes = ajTablecharNewConst(10);

    dblist = ajListNew();

    for (i=0; i< ajDomNodeListGetLen(dbnodes); i++)
    {
	dbnode = ajDomNodeListItem(dbnodes, i);
	AJNEW0(db);

	e = ajDomElementGetFirstChildByTagNameC(doc,dbnode,"displayName");
	ajStrAssignS(&db->displayname, ajDomElementGetText(e));

	for(j=0; j<ajDomNodeListGetLen(dbnode->childnodes); j++)
	{
	    AjPDomNode child = ajDomNodeListItem(dbnode->childnodes, j);
	    if(ajStrMatchC(child->name, "name"))
		ajStrAssignS(&db->name, ajDomElementGetText(child));
	}

	e = ajDomElementGetFirstChildByTagNameC(doc,dbnode,"description");
	ajStrAssignS(&db->description, ajDomElementGetText(e));

	e = ajDomElementGetFirstChildByTagNameC(doc,dbnode,
	                                        "exampleIdentifiers");

	db->example = cachedbfetch_GetExamples(doc, e);

	e = ajDomElementGetFirstChildByTagNameC(doc,dbnode, "formatInfoList");
	ajTableClear(dbtypes);
	db->fmtterms = ajStrNew();
	db->dataterms = ajStrNew();
	db->tpcterms = ajStrNew();
	db->format = cachedbfetch_GetFormats(doc, e, dbtypes, db);
	cachedbfetch_SetDbtype(db, dbtypes);

	e = ajDomElementGetFirstChildByTagNameC(doc,dbnode, "databaseTerms");
	e = ajDomElementGetFirstChildByTagNameC(doc, e, "databaseTerm");
	ajStrAssignS(&dbterm, ajDomElementGetText(e));
	ajDebug( "dbterm: %S\n", dbterm);

	if( ajStrGetLen(dbterm)> 0  &&
		ajStrFindS(db->tpcterms, dbterm) == -1)
	{
	    if(ajStrGetLen(db->tpcterms)>0)
		ajStrAppendC(&db->tpcterms, ", ");

	    ajStrAppendS(&db->tpcterms, dbterm);
	}

	ajListPushAppend(dblist,db);
	ajStrDel(&dbterm);
    }

    ajTableDel(&dbtypes);
    ajDomDocumentDestroyNodeList(doc,dbnodes,AJDOMKEEP);
    ajDomDocumentDestroyNode(doc,doc);

    return dblist;
}




/* @funcstatic cachedbfetch_GetExamples ***************************************
**
** Parses the results of dbfetch metadata query response
** for test query identifiers
**
** @param [u] doc [AjPDomDocument] ajdom document obj
** @param [u] elm [AjPDomElement] ajdom element obj for examples list
** @return [AjPStr] list of example identifiers
** @@
******************************************************************************/

static AjPStr cachedbfetch_GetExamples(AjPDomDocument doc, AjPDomElement elm)
{
    AjPStr ret = NULL;
    AjPStr id  = NULL;
    AjPDomElement e = NULL;
    AjPDomNode ex;
    int j=0;
    int i=0;

    ret = ajStrNew();

    while (idlists[j] != NULL)
    {
	e = ajDomElementGetFirstChildByTagNameC(doc, elm, idlists[j++]);

	if(e)
	{
	    for(i=0; i<ajDomNodeListGetLen(e->childnodes);i++)
	    {
		ex = ajDomNodeListItem(e->childnodes, i);
		id = ajDomElementGetText(ex);

		if(!id)
		    continue; /* skip #text elements */

		if(ajStrGetLen(ret)>0)
		    ajStrAppendC(&ret, ", ");

		ajStrAppendS(&ret, id);

	    }
	}

    }

    return ret;
}




/* @funcstatic cachedbfetch_GetFormats ****************************************
**
** Parses the dbfetch metadata query response for format names
**
** @param [u] doc [AjPDomDocument] ajdom document obj
** @param [u] e [AjPDomElement] ajdom element obj for format-info list
** @param [u] dbtypes [AjPTable] set of db-types
** @param [u] db [dbPinfo] database metadata
** @return [AjPStr] supported formats in comma separated form
** @@
******************************************************************************/

static AjPStr cachedbfetch_GetFormats(AjPDomDocument doc, AjPDomElement e,
                                      AjPTable dbtypes, dbPinfo db)
{
    AjPStr ret = NULL;
    AjPStr format = NULL;
    AjPStr dataterm = NULL;
    AjPStr syntaxterm = NULL;
    AjPDomElement formatinfo = NULL;
    AjPDomElement dataterms = NULL;
    AjPDomElement datatermelm = NULL;
    AjPDomElement syntaxterms = NULL;
    AjPDomElement syntaxtermelm = NULL;
    AjPDomElement nameelm = NULL;
    int i=0;

    ret = ajStrNew();

    for(i=0; i<ajDomNodeListGetLen(e->childnodes);i++)
    {

	formatinfo = ajDomNodeListItem(e->childnodes, i);

	nameelm = ajDomElementGetFirstChildByTagNameC(doc, formatinfo, "name");

	if(!nameelm)
	    continue;

	format = ajDomElementGetText(nameelm);

	if (ajStrMatchC(format, "default"))
	    continue;

	/* adds the format only if it is supported by one of the db types */
	if(cachedbfetch_GetDbtypes(ajStrGetPtr(format), dbtypes))
	{
	    if(ajStrGetLen(ret)>0)
		ajStrAppendC(&ret, ", ");

	    ajStrAppendS(&ret, format);
	}

	/* data terms */

	dataterms = ajDomElementGetFirstChildByTagNameC(doc, formatinfo,
	                                                "dataTerms");

	datatermelm = ajDomElementGetFirstChildByTagNameC(doc, dataterms,
	                                               "dataTerm");

	ajStrAssignS(&dataterm, ajDomElementGetText(datatermelm));
	ajDebug( "%S\n", dataterm);

	if( ajStrGetLen(dataterm)> 0  &&
		ajStrFindS(db->dataterms, dataterm) == -1)
	{
	    if(ajStrGetLen(db->dataterms)>0)
		ajStrAppendC(&db->dataterms, ", ");

	    ajStrAppendS(&db->dataterms, dataterm);
	}

	/* syntax terms */

	syntaxterms = ajDomElementGetFirstChildByTagNameC(doc, formatinfo,
	                                                  "syntaxTerms");

	syntaxtermelm = ajDomElementGetFirstChildByTagNameC(doc, syntaxterms,
	                                                    "syntaxTerm");

	ajStrAssignS(&syntaxterm, ajDomElementGetText(syntaxtermelm));
	ajDebug( "syntax term: %S\n", syntaxterm);

	if( ajStrGetLen(syntaxterm)> 0  &&
		ajStrFindS(db->fmtterms, syntaxterm) == -1)
	{
	    if(ajStrGetLen(db->fmtterms)>0)
		ajStrAppendC(&db->fmtterms, ", ");

	    ajStrAppendS(&db->fmtterms, syntaxterm);
	}

	ajStrDel(&dataterm);
	ajStrDel(&syntaxterm);

    }

    if(!ajStrGetLen(ret))
	ajStrAssignC(&ret,"unknown");

    return ret;
}




/* @funcstatic cachedbfetch_GetDbtypes ****************************************
**
** Finds db-types supporting the given format
**
** @param [r] format [const char*] format name
** @param [u] dbtypes [AjPTable] set of db-types
** @return [AjBool] Returns true if any supporting db-types found
** @@
******************************************************************************/

static AjBool cachedbfetch_GetDbtypes(const char* format, AjPTable dbtypes)
{
    AjBool found = ajFalse;
    AjPStr formatS = ajStrNewC(format);
    static char assembly[] = "assembly";
    static char features[] = "features";
    static char obo[]      = "obo";
    static char resource[] = "resource";
    static char sequence[] = "sequence";
    static char taxon[]    = "taxon";
    static char text[]     = "text";

    if(ajAsseminformatTest(formatS))
    {
	ajTablePut(dbtypes, assembly, NULL);
	found=ajTrue;
    }

    if(ajFeattabinformatTest(formatS))
    {
	ajTablePut(dbtypes, features, NULL);
	found=ajTrue;
    }

    if(ajOboinformatTest(formatS))
    {
	ajTablePut(dbtypes, obo, NULL);
	found=ajTrue;
    }

    if(ajResourceinformatTest(formatS))
    {
	ajTablePut(dbtypes, resource, NULL);
	found=ajTrue;
    }

    if(ajSeqinformatTest(formatS))
    {
	ajTablePut(dbtypes, sequence, NULL);
	found=ajTrue;
    }

    if(ajTaxinformatTest(formatS))
    {
	ajTablePut(dbtypes, taxon, NULL);
	found=ajTrue;
    }

    if(ajTextinformatTest(formatS))
    {
	ajTablePut(dbtypes, text, NULL);
	found=ajTrue;
    }

    if(!found)
	ajWarn("No dbtype found supporting format %s", format);

    ajStrDel(&formatS);

    return found;
}




/* @funcstatic cachedbfetch_SetDbtype *****************************************
**
** Finds db-types supporting the given format
**
** @param [u] dbinfo [dbPinfo] database info obj
** @param [u] dbtypes [AjPTable] set of db-types
** @return [void]
** @@
******************************************************************************/

static void cachedbfetch_SetDbtype(dbPinfo dbinfo, AjPTable dbtypes)
{
    char** keys = NULL;
    ajuint i;
    ajuint n;


    ajTableToarrayKeys(dbtypes, (void***)&keys);

    n = (ajuint) ajTableGetLength(dbtypes);

    for(i=0;i<n;i++)
    {
	if (i>0)
	    ajStrAppendC(&dbinfo->dbtype, ", ");

	ajStrAppendC(&dbinfo->dbtype, keys[i]);
    }

    if(!n)
	ajWarn("No dbtype found for db %S", dbinfo->displayname);

    AJFREE(keys);

    return;
}




/* @funcstatic cachedbfetch_PrintCachefile **********************************
**
** Prints DB definition for the specified list of wsdbfetch databases
**
** @param [r] dbs [const AjPList] list of wsdbfetch databases
** @param [w] cachef [AjPFile] server cachefile to write DB definitions
** @param [r] usedbfetch [AjBool] use dbfetch access method
**                                rather than wsdbfetch
** @return [void]
** @@
******************************************************************************/

static void cachedbfetch_PrintCachefile(const AjPList dbs, AjPFile cachef,
	                                  AjBool usedbfetch)
{
    AjPTime today = NULL;
    AjIList iter  = NULL;
    AjPStr fname  = NULL;

    dbPinfo source = NULL;


    fname  = ajStrNewS(ajFileGetNameS(cachef));

    ajFilenameTrimPath(&fname);

    today =  ajTimeNewTodayFmt("cachefile");

    ajFmtPrintF(cachef,"# %S %D\n\n", fname, today);

    ajStrDel(&fname);
    ajTimeDel(&today);

    if(!dbs)
	return;

    iter = ajListIterNewread(dbs);

    while(!ajListIterDone(iter))
    {
	source = ajListIterGet(iter);

	cachedbfetch_WriteDBdefinition(cachef, source, usedbfetch);
    }

    ajListIterDel(&iter);

    return;
}




/* @funcstatic cachedbfetch_WriteDBdefinition *******************************
**
** Writes a WsDbfetch DB definition to the specified cache-file
**
** @param [u] cachef [AjPFile] cache file
** @param [r] dbinfo [const dbPinfo] ws/dbfetch database object
** @param [r] usedbfetch [AjBool] use dbfetch access method
**                                rather than wsdbfetch
** @return [void]
** @@
******************************************************************************/

static void cachedbfetch_WriteDBdefinition(AjPFile cachef,
                                           const dbPinfo dbinfo,
                                           AjBool usedbfetch)
{
    AjPStr dbtype    = NULL;
    AjIList iter   = NULL;
    AjPStr comment = NULL;

    comment = ajStrNewS(dbinfo->description);

    ajStrExchangeKK(&comment, '"', '\'');
    ajStrRemoveWhiteExcess(&comment);

    if(ajStrGetLen(dbinfo->dbtype))
	ajStrAssignS(&dbtype, dbinfo->dbtype);
    else
	dbtype = ajStrNewC("text");

    ajFmtPrintF(cachef,
	    "DB %S [\n"
	    "  method: %s\n"
	    "  hasacc: N\n"
	    "  format: \"%S\"\n"
	    "  edamfmt: \"%S\"\n"
	    "  edamdat: \"%S\"\n"
	    "  edamtpc: \"%S\"\n"
	    "  type: \"%S\"\n"
	    "  example: \"%S\"\n"
	    "  comment: \"%S\"\n"
	    "]\n\n",
	    dbinfo->name,
	    (usedbfetch ? "dbfetch" : "wsdbfetch"),
	    dbinfo->format,
	    dbinfo->fmtterms,
	    dbinfo->dataterms,
	    dbinfo->tpcterms,
	    dbtype,
	    dbinfo->example,
	    comment
	    );

    ajListIterDel(&iter);

    ajStrDel(&comment);
    ajStrDel(&dbtype);

    return;
}

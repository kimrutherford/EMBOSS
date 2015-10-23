/* @source cacheebeyesearch application
 **
 ** Prepares EMBOSS server cachefile for EBI EB-eye search domains.
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


/* @datastatic ebeyePdbinfo ***************************************************
**
** Database information for cacheebeyesearch
**
** @attr name [AjPStr] Name on server
** @attr searchableFields [AjPStr] Searchable fields
** @attr retrievableFields [AjPStr] Retrievable fields
** @attr description [AjPStr] Description
** @attr dbalias [AjPStr] Database alias
******************************************************************************/

typedef struct dbinfo
{
    AjPStr name;
    AjPStr searchableFields;
    AjPStr retrievableFields;
    AjPStr description;
    AjPStr dbalias;
} ebeyeOdbinfo;
#define ebeyePdbinfo ebeyeOdbinfo*

/* todo: description available through getDomainsHierarchy() call */



static AjPList cacheebeyesearch_ListDomainsCall(const AjPStr url);
static void    cacheebeyesearch_PrintCachefile(const AjPList dbs,
                                               AjPFile cachef);
static void    cacheebeyesearch_PrintDBdefinition(AjPFile cachef,
                                                  const ebeyePdbinfo dbinfo);

#ifdef HAVE_AXIS2C

static void  cacheebeyesearch_ListFieldsCall(const AjPStr url,
                                             ebeyePdbinfo domainInfo);

static void  cacheebeyesearch_GetFields(axiom_node_t *wsResult,
                                        const axutil_env_t *env,
                                        ebeyePdbinfo domainInfo);

static AjPList cacheebeyesearch_GetDomainList(axiom_node_t *wsResult,
                                              const axutil_env_t *env);

#endif




/* @prog cacheebeyesearch *****************************************************
 **
 ** Prepares EMBOSS server cachefile for EBI EB-eye search domains
 **
 *****************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf = NULL;
    AjPFile cachef = NULL;

    AjPStr query = NULL;
    AjPList l    = NULL;
    ebeyePdbinfo dbinfo   = NULL;

    AjIList i;
    AjPStr url = NULL;

    embInit("cacheebeyesearch", argc, argv);

    outf   = ajAcdGetOutfile("outfile");
    cachef = ajAcdGetOutfile("cachefile");

    l = cacheebeyesearch_ListDomainsCall(url);

    if(l==NULL)
	ajErr("ebeye webservices call returned null");

    if(cachef)
	cacheebeyesearch_PrintCachefile(l, cachef);


    if (l)
    {
	i = ajListIterNew(l);

	while(!ajListIterDone(i))
	{
	    dbinfo = ajListIterGet(i);
	    ajStrDel(&dbinfo->searchableFields);
	    ajStrDel(&dbinfo->retrievableFields);
	    ajStrDel(&dbinfo->description);
	    ajStrDel(&dbinfo->dbalias);
	    ajStrDel(&dbinfo->name);
	    AJFREE(dbinfo);
	}

	ajListFree(&l);
	ajListIterDel(&i);
    }

    ajFileClose(&outf);
    ajFileClose(&cachef);

    ajStrDel(&query);
    ajStrDel(&url);

    embExit();

    return 0;
}




/* @funcstatic cacheebeyesearch_ListDomainsCall *******************************
**
** Makes EBeye "listDomains" call and returns list of domains
**
** @param [r] url [const AjPStr] endpoint for the EBeye webservice
** @return [AjPList] list of dbinfo objs
** @@
******************************************************************************/

static AjPList cacheebeyesearch_ListDomainsCall(const AjPStr url)
{
    AjPList ret = NULL;

#ifdef HAVE_AXIS2C

    axiom_node_t *payload = NULL;
    axiom_namespace_t *ns = NULL;
    const axis2_char_t* address = NULL;

    axis2_svc_client_t* client  = NULL;
    axiom_node_t* result        = NULL;

    axutil_env_t * env;

    if (url == NULL)
	address = EBEYE_EP;
    else
        address = ajStrGetPtr(url);

    ajDebug("ebeye URL: %s\n", address);

    env = ajSoapAxis2GetEnv();
    client = ajSoapAxis2GetClient(env, address);


    ns = axiom_namespace_create(env, EBEYE_NS, "ebeye");
    axiom_element_create(env, NULL, "listDomains", ns, &payload);


    result = ajSoapAxis2Call(client, env, payload);

    if(result != NULL)
    {
	ret = cacheebeyesearch_GetDomainList(result, env);
	axiom_node_free_tree(result, env);
    }

    axis2_svc_client_free(client, env);
    client = NULL;
    axutil_env_free(env);
    env = NULL;

#else
    (void) url;
    ajWarn("Cannot make ebeye webservices calls without Axis2C enabled");
#endif


    return ret;
}




#ifdef HAVE_AXIS2C
/* @funcstatic cacheebeyesearch_ListFieldsCall ********************************
**
** Makes EBeye "listFieldsInformation" call
**
** @param [r] url [const AjPStr] EBeye webservice endpoint address
** @param [u] domainInfo [ebeyePdbinfo] domain info
** @return [void]
** @@
******************************************************************************/

static void cacheebeyesearch_ListFieldsCall(const AjPStr url,
                                            ebeyePdbinfo domainInfo)
{
    AjPStr domain;

    axiom_node_t *payload = NULL;
    axiom_element_t *elm  = NULL;
    axiom_node_t *child   = NULL;
    axiom_namespace_t *ns = NULL;
    const axis2_char_t* address = NULL;

    axis2_svc_client_t* client  = NULL;
    axiom_node_t* result        = NULL;

    axutil_env_t * env;

    domain = domainInfo->name;

    if (url == NULL)
	address = EBEYE_EP;
    else
        address = ajStrGetPtr(url);

    env = ajSoapAxis2GetEnv();
    client = ajSoapAxis2GetClient(env, address);

    ns = axiom_namespace_create(env, EBEYE_NS, "ebeye");
    elm = axiom_element_create(env, NULL, "listFieldsInformation", ns,
                               &payload);

    elm = axiom_element_create(env, payload, "domain", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(domain), child);


    result = ajSoapAxis2Call(client, env, payload);

    if(result != NULL)
    {
	cacheebeyesearch_GetFields(result, env, domainInfo);
	axiom_node_free_tree(result, env);
    }

    axis2_svc_client_free(client, env);
    axutil_env_free(env);

    return;
}




/* @funcstatic cacheebeyesearch_GetFields *************************************
**
** Parses the results of EBeye "listFieldsInformation" calls
**
** @param [u] wsResult [axiom_node_t*] axis2 OM object
** 				       obtained by EBeye webservice call
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [u] dbInfo [ebeyePdbinfo] domain info
** @return [void]
** @@
******************************************************************************/

static void cacheebeyesearch_GetFields(axiom_node_t *wsResult,
                                       const axutil_env_t *env,
                                       ebeyePdbinfo dbInfo)
{
    AjPStr field         = NULL;

    axis2_char_t* text   = NULL;
    axis2_char_t* name   = NULL;

    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;
    axiom_node_t* nodeFieldInfo   = NULL;

    axiom_children_iterator_t* arrayOfFieldInformation  = NULL;
    axiom_children_iterator_t* iterFieldInfo = NULL;
    axiom_children_iterator_t* attsFieldInfo = NULL;

    AjBool retrievable = ajFalse;
    AjBool searchable = ajFalse;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "listFieldsInformationResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return;
    }

    arrayOfFieldInformation = axiom_element_get_children(elm, env, wsResult);

    if(arrayOfFieldInformation != NULL)
    {
	while(axiom_children_iterator_has_next(arrayOfFieldInformation, env))
	{
	    node = axiom_children_iterator_next(arrayOfFieldInformation, env);

	    if(axiom_node_get_node_type(node, env) == AXIOM_ELEMENT)
	    {
		elm = axiom_node_get_data_element(node, env);

		iterFieldInfo = axiom_element_get_children(elm, env, node);

		while(axiom_children_iterator_has_next(iterFieldInfo, env))
		{

		    nodeFieldInfo = axiom_children_iterator_next(iterFieldInfo,
		                                                 env);

		    elm = axiom_node_get_data_element(nodeFieldInfo, env);
		    name = axiom_element_get_localname(elm, env);

		    attsFieldInfo = axiom_element_get_children(elm, env,
		                                               nodeFieldInfo);
		    field = ajStrNew();

		    while(axiom_children_iterator_has_next(attsFieldInfo, env))
		    {

			node = axiom_children_iterator_next(attsFieldInfo, env);
			elm = axiom_node_get_data_element(node, env);
			name = axiom_element_get_localname(elm, env);
			text = axiom_element_get_text(elm, env, node);

			ajDebug("%s: %s\n", name, text);

			if(!text)
			    continue;

			if (ajCharMatchCaseC(name, "id"))
			    ajStrAssignC(&field, text);

			else if (ajCharMatchCaseC(name, "name"))
			{}
			else if (ajCharMatchCaseC(name, "description"))
			{}
			else if (ajCharMatchCaseC(name, "retrievable") &&
				ajCharMatchCaseC(text, "true") )

			    retrievable = ajTrue;

			else if (ajCharMatchCaseC(name, "searchable"))

			    searchable = ajTrue;

		    }

		    if (searchable)
		    {
			if(ajStrGetLen(dbInfo->searchableFields)>0)
			    ajStrAppendC(&dbInfo->searchableFields, ", ");

			ajStrAppendS(&dbInfo->searchableFields,field);
		    }

		    if (retrievable)
		    {
			if(ajStrGetLen(dbInfo->retrievableFields)>0)
			    ajStrAppendC(&dbInfo->retrievableFields, ", ");
			ajStrAppendS(&dbInfo->retrievableFields,field);
			retrievable = ajFalse;
		    }

		    ajStrDel(&field);
		}
	    }
	}
    }

    ajStrDel(&field);
    
    return;
}




/* @funcstatic cacheebeyesearch_GetDomainList *********************************
**
** Parses the results of EBeye "listDomains" calls
**
** @param [u] wsResult [axiom_node_t*] axis2 OM object
** 				       obtained by EBeye webservice call
** @param [r] env [const axutil_env_t*] axis2 environment
** @return [AjPList] list of domains
** @@
******************************************************************************/

static AjPList cacheebeyesearch_GetDomainList(axiom_node_t *wsResult,
                                              const axutil_env_t *env)
{
    AjPList dblist       = NULL;
    axis2_char_t* id     = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;
    axiom_node_t* dbattnode   = NULL;
    axiom_children_iterator_t* dbObjIter  = NULL;
    axiom_children_iterator_t* dbAttrIter = NULL;

    ebeyePdbinfo db = NULL;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "listDomainsResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return NULL;
    }

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

		while(axiom_children_iterator_has_next(dbAttrIter, env))
		{

		    dbattnode = axiom_children_iterator_next(dbAttrIter, env);

		    elm = axiom_node_get_data_element(dbattnode, env);

		    name = axiom_element_get_localname(elm, env);

		    if (ajCharMatchCaseC(name, "string"))
		    {
			AJNEW0(db);
			id = axiom_element_get_text(elm, env, dbattnode);
			ajDebug("domain: %s\n", id);
			ajStrAssignC(&db->name, id);

			ajStrAssignC(&db->dbalias, id);

			cacheebeyesearch_ListFieldsCall(NULL, db);

			if(ajStrMatchCaseC(db->name, "2can"))
			    ajStrAssignC(&db->name, "toucan");

			ajStrExchangeSetCC(&db->name, "-", "_");
			ajStrTrimC(&db->name, "_");
			ajStrTrimStartC(&db->name, "0123456789");


			ajListPushAppend(dblist,db);
		    }
		}
	    }
	}
    }

    return dblist;
}


#endif




/* @funcstatic cacheebeyesearch_PrintCachefile ********************************
**
** Prints EMBOSS DB definitions for the specified EBeye domains
**
** @param [r] dbs [const AjPList] list of EBeye domains
** @param [w] cachef [AjPFile] server cachefile to write DB definitions
** @return [void]
** @@
******************************************************************************/

static void cacheebeyesearch_PrintCachefile(const AjPList dbs, AjPFile cachef)
{
    AjPTime today = NULL;
    AjIList iter  = NULL;
    AjPStr fname  = NULL;

    ebeyePdbinfo source = NULL;


    fname  = ajStrNewS(ajFileGetNameS(cachef));

    ajFilenameTrimPath(&fname);

    today =  ajTimeNewTodayFmt("cachefile");

    ajFmtPrintF(cachef,"# %S %D\n\n", fname, today);

    ajStrDel(&fname);
    ajTimeDel(&today);

    iter = ajListIterNewread(dbs);


    while(!ajListIterDone(iter))
    {
	source = ajListIterGet(iter);

	cacheebeyesearch_PrintDBdefinition(cachef, source);
    }
    ajListIterDel(&iter);
}




/* @funcstatic cacheebeyesearch_PrintDBdefinition *****************************
**
** Prints EMBOSS DB definition for the specified EBeye domain
**
** @param [u] cachef [AjPFile] cache file
** @param [r] dbinfo [const ebeyePdbinfo] EBeye database object
** @return [void]
** @@
******************************************************************************/

static void cacheebeyesearch_PrintDBdefinition(AjPFile cachef,
                                               const ebeyePdbinfo dbinfo)
{
    AjPStr comment = NULL;

    comment = ajStrNewS(dbinfo->description);

    ajStrExchangeKK(&comment, '"', '\'');
    ajStrRemoveWhiteExcess(&comment);

    ajFmtPrintF(cachef,
	    "DBNAME %S [\n"
	    "  method: \"ebeye\"\n"
	    "  dbalias: \"%S\"\n"
	    "  fields: \"%S\"\n"
	    "  return: \"%S\"\n"
	    "  format: \"text\"\n"
	    "  hasaccession: \"N\"\n"
	    "]\n\n",
	    dbinfo->name,
	    dbinfo->dbalias,
	    dbinfo->searchableFields,
	    dbinfo->retrievableFields
	    );

    ajStrDel(&comment);

    return;
}

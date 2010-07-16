/******************************************************************************
** @source AJAX Mart (database) functions
**
** These functions control all aspects of AJAX sequence database access
**
** @author Copyright (C) 2009 Alan Bleasby
** @version 1.0
** @modified Nov 23 ajb First version
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

#include "ajax.h"
#include "expat.h"
#include "ajseqdb.h"
#include "ajmart.h"


#define REGTABGUESS 100
#define MART_READ_AHEAD 5

#define DEFAULT_BIOMART_REG_HOST "www.biomart.org"
#define DEFAULT_BIOMART_REG_PATH "/biomart/martservice"
#define DEFAULT_BIOMART_REG_PORT 80

#define DEFAULT_BIOMART_MART_HOST "www.biomart.org"
#define DEFAULT_BIOMART_MART_PATH "/biomart/martservice"
#define DEFAULT_BIOMART_MART_PORT 80

static const char* martNucTerms[] =
{
    "3utr", "5utr", "cdna", "coding", "gene_exon", "gene_flank",
    "upstream_flank", "downstream_flank", "gene_exon_intron",
    "transcript_flank", "coding_gene_flank", "transcript_exon_intron",
    "coding_transcript_flank", "snp", "upstream_intergenic_raw",
    NULL
};

static const char* martProtTerms[] =
{
    "peptide",
    NULL
};




static void martRegistryElementend(void *userData, const XML_Char *name);
static void martRegistryElementstart(void *userData, const XML_Char *name,
                                     const XML_Char **atts);
static void martConvertLocToArrays(AjPMartLoc loc);
static AjPTable martGetMarttable(const AjPSeqin seqin, const AjPStr mart);
static AjBool martBuffIsXML(AjPFilebuff buff);
static ajint martTabToToken(AjPStr *token, const AjPStr s, ajint loc);
static AjBool martParseTabbedDataset(AjPSeqin seqin);
static AjBool martParseTabbedAttributes(AjPSeqin seqin);
static AjBool martParseTabbedFilters(AjPSeqin seqin);
static void martTablePush(AjPTable table, const char *name,
                          const AjPStr token);
static void martStrdel(void** str, void* cl);
static int martAttcmp(const void* str, const void* str2);
static AjBool martHttpEncode(AjPStr *str);

static AjBool martMatchAttribute(const AjPStr name,
                                 const AjPMartAttribute atts);
static AjBool martMatchFilter(const AjPStr name,
                              const  AjPMartFilter filts);

static const char *martGetVirtualSchema(const AjPStr dataset);
static void martSpacesToHex(AjPStr *s);
static int martAttributePageCompar(const void *a, const void *b);




/*
** This array is a kludge. It is to hold the dataset and schema names
** for cases where the schema name isn't "default" i.e. it
** is a lookup table.
** The virtual schema name could be retrieved from a dataset query but
** this table saves going on the net for that.
** This table is a recommendation from the Biomart developers who
** say that virtualSchemas are going to be removed from biomart queries
** soon.
*/

static const char *dataset_schemas[] =
{
    "dna",     "dictyMart",
    "est",     "dictyMart",
    "gene",    "dictyMart",
    "dictygo", "dictyMart",
    "COSMIC46", "cosmic",
    "hm27_variation_yri", "rel27_NCBI_Build36",
    "hm27_variation_tsi", "rel27_NCBI_Build36",
    "hm27_gene",          "rel27_NCBI_Build36",
    "hm27_variation_chd", "rel27_NCBI_Build36",
    "hm27_variation",     "rel27_NCBI_Build36",
    "hm27_variation_chb", "rel27_NCBI_Build36",
    "hm27_variation_ceu", "rel27_NCBI_Build36",
    "hm27_variation_asw", "rel27_NCBI_Build36",
    "hm27_variation_mkk", "rel27_NCBI_Build36",
    "hm27_variation_lwk", "rel27_NCBI_Build36",
    "hm27_encode",        "rel27_NCBI_Build36",
    "hm27_variation_jpt", "rel27_NCBI_Build36",
    "hm27_variation_mex", "rel27_NCBI_Build36",
    "hm27_variation_gih", "rel27_NCBI_Build36",
    "phytozome_structure", "zome_mart",
    "phytozome", "zome_mart",
    "phytozome_clusters", "zome_mart",
    NULL, NULL
};




/* @func ajStrUrlNew *********************************************************
**
** Initialise a URL components object
**
** @return [AjPUrl] URL Components
******************************************************************************/

AjPUrl ajStrUrlNew(void)
{
    AjPUrl ret = NULL;

    AJNEW0(ret);
    
    ret->Method   = ajStrNew();
    ret->Host     = ajStrNew();
    ret->Port     = ajStrNew();
    ret->Absolute = ajStrNew();
    ret->Relative = ajStrNew();
    ret->Fragment = ajStrNew();
    ret->Username = ajStrNew();
    ret->Password = ajStrNew();

    return ret;
}




/* @func ajStrUrlDel *********************************************************
**
** Delete URL components object
**
** @param [u] thys [AjPUrl*] URL components object
** @return [void]
******************************************************************************/

void ajStrUrlDel(AjPUrl *thys)
{
    AjPUrl pthis = NULL;

    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    ajStrDel(&pthis->Method);
    ajStrDel(&pthis->Host);
    ajStrDel(&pthis->Port);
    ajStrDel(&pthis->Absolute);
    ajStrDel(&pthis->Relative);
    ajStrDel(&pthis->Fragment);
    ajStrDel(&pthis->Username);
    ajStrDel(&pthis->Password);

    AJFREE(pthis);
    
    *thys = NULL;
    
    return;
}




/* @func ajStrUrlParseC ******************************************************
**
** Parse an IPV4/6 URL into its components
**
** @param [u] parts [AjPUrl*] URL components object
** @param [u] url [const char*] URL
** @return [void]
******************************************************************************/

void ajStrUrlParseC(AjPUrl *parts, const char *url)
{
    char *ucopy = NULL;
    char *p = NULL;
    char *post = NULL;

    char *dest = NULL;
    char *src  = NULL;
    
    AjPUrl comp = NULL;
    
    char *pmethod = NULL;
    char *phost   = NULL;
    char *pabs    = NULL;
    char *prel    = NULL;
    
    if(!parts || !url)
        return;
    
    if(!*parts)
        return;
    
    ucopy = ajCharNewC(url);

    post = ucopy;
    comp = *parts;
    
    /* Get any fragment */
    if ((p = strchr(ucopy, '#')))
    {
	*p++ = '\0';
        ajStrAssignC(&comp->Fragment,p);
    }

    if((p = strchr(ucopy, ' ')))
        *p++ = '\0';


    for(p = ucopy; *p; ++p)
    {
	if (isspace((int) *p))
        {
            dest = p;
            src  = p+1;
            
	    while ((*dest++ = *src++));

	    p = p-1;
	}

	if (*p == '/' || *p == '#' || *p == '?')
	    break;

	if (*p == ':')
        {
		*p = '\0';
                pmethod = post;
		post = p+1;

                if(ajCharPrefixCaseC(pmethod,"URL"))
                    pmethod = NULL;
                else
                    break;
	}
    }

    p = post;

    if(*p == '/')
    {
	if(p[1] == '/')
        {
	    phost = p+2;		/* There is a host   */
	    *p = '\0';
	    p = strchr(phost,'/');	/* Find end of host */

	    if(p)
            {
	        *p=0;			/* and terminate it */
	        pabs = p+1;		/* Path found */
	    }
	}
        else
	    pabs = p+1;		/* Path found but not host */
    }
    else
        prel = (*post) ? post : NULL;


    if(pmethod)
        ajStrAssignC(&comp->Method,pmethod);

    if(phost)
        ajStrAssignC(&comp->Host,phost);

    if(pabs)
        ajStrAssignC(&comp->Absolute,pabs);

    if(prel)
        ajStrAssignC(&comp->Relative,prel);

    AJFREE(ucopy);

    return;
}




/* @func ajStrUrlSplitPort ***************************************************
**
** Separate any port from a host specification (IPV4/6)
**
** @param [u] urli [AjPUrl] URL components object
** @return [void]
******************************************************************************/

void ajStrUrlSplitPort(AjPUrl urli)
{
    const char *p   = NULL;
    const char *end = NULL;
    const char *start = NULL;
    
    ajint len;

    if(!(len = ajStrGetLen(urli->Host)))
        return;

    end = (start = ajStrGetPtr(urli->Host)) + len - 1;

    p = end;

    if(!isdigit((int) *p))
        return;

    while(isdigit((int) *p) && p != start)
        --p;

    if(p == start)
        return;

    if(*p != ':')
        return;
    
    ajStrAssignC(&urli->Port,p+1);

    ajStrAssignSubC(&urli->Host,start,0,p-start-1);
    
    return;
}




/* @func ajStrUrlSplitUsername ************************************************
**
** Separate any username[:password] from a host specification (IPV4/6)
**
** @param [u] urli [AjPUrl] URL components object
** @return [void]
******************************************************************************/

void ajStrUrlSplitUsername(AjPUrl urli)
{
    const char *p   = NULL;
    const char *end = NULL;
    AjPStr userpass = NULL;
    AjPStr host = NULL;
    
    ajint len;

    if(!ajStrGetLen(urli->Host))
        return;

    if(!(end = strchr(ajStrGetPtr(urli->Host), (int)'@')))
        return;

    p = ajStrGetPtr(urli->Host);
    len = end - p;

    if(!len)
        return;
    
    userpass = ajStrNew();
    ajStrAssignSubC(&userpass, p, 0, end - p - 1);
    
    host = ajStrNew();
    ajStrAssignC(&host,end + 1);
    ajStrAssignS(&urli->Host,host);

    

    if(!(end = strchr(ajStrGetPtr(userpass), (int)':')))
    {
        ajStrAssignS(&urli->Username,userpass);
        ajStrDel(&userpass);
        ajStrDel(&host);

        return;
    }

    p = ajStrGetPtr(userpass);
    len = end - p;

    if(!len)
        ajWarn("ajStrUrlSplitUsername: Missing username in URL [%S@%S]",
               userpass,host);
    else
        ajStrAssignSubC(&urli->Username,p,0,len - 1);

    ajStrAssignC(&urli->Password, end + 1);

    ajStrDel(&userpass);
    ajStrDel(&host);

    return;
}




/* @func ajMartHttpUrl ***************************************************
**
** Returns the components of a URL (IPV4/6)
** An equivalent for seqHttpUrl().
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [w] iport [ajint*] Port
** @param [w] host [AjPStr*] Host name
** @param [w] urlget [AjPStr*] URL for the HTTP header GET
** @return [AjBool] ajTrue if the URL was parsed
** @@
******************************************************************************/

AjBool ajMartHttpUrl(const AjPSeqQuery qry, ajint* iport, AjPStr* host,
                     AjPStr* urlget)
{
    AjPStr url = NULL;
    AjPUrl uo = NULL;
    
    url = ajStrNew();

    if(!ajNamDbGetUrl(qry->DbName, &url))
    {
	ajErr("no URL defined for database %S", qry->DbName);

	return ajFalse;
    }

    uo = ajStrUrlNew();
    
    ajStrUrlParseC(&uo, ajStrGetPtr(url));
    ajStrUrlSplitPort(uo);
    
    ajStrAssignS(host,uo->Host);
    ajFmtPrintS(urlget,"/%S",uo->Absolute);

    if(ajStrGetLen(uo->Port))
        ajStrToInt(uo->Port,iport);

    ajStrDel(&url);
    ajStrUrlDel(&uo);

    return ajTrue;
}




/* @funcstatic martGetVirtualSchema ******************************************
**
** Return a virtual schema given a dataset name
**
** @param [r] dataset [const AjPStr] dataset name
** @return [const char*] virtualSchema value
******************************************************************************/

static const char* martGetVirtualSchema(const AjPStr dataset)
{
    static const char *defschema="default";
    const char *ret = NULL;
    ajuint i;

    ret = defschema;
    
    i = 0;
    while(dataset_schemas[i])
    {
        if(ajStrMatchC(dataset,dataset_schemas[i]))
        {
            ret = dataset_schemas[i+1];
            break;
        }

        i += 2;
    }

    return ret;
}




/* @func ajMartqueryNew ***************************************************
**
** Initialise mart query object
**
** @return [AjPMartquery] Mart query object
******************************************************************************/

AjPMartquery ajMartqueryNew(void)
{
    AjPMartquery ret = NULL;

    AJNEW0(ret);
    
    ret->Reghost  = ajStrNew();
    ret->Regpath  = ajStrNew();
    ret->Marthost = ajStrNew();
    ret->Martpath = ajStrNew();
    ret->Query    = ajStrNew();
    ret->Loc      = ajMartLocNew();

    ret->Dataset = ajMartDatasetNew();
    ret->Atts    = ajMartAttributeNew();
    ret->Filters = ajMartFilterNew();

    ret->Config = NULL;
    
    ret->Regport  =  80;
    ret->Martport  = 80;

    return ret;
}




/* @func ajMartqueryDel ***************************************************
**
** Delete Mart query object
**
** @param [u] thys [AjPMartquery*] Mary query object
** @return [void]
******************************************************************************/

void ajMartqueryDel(AjPMartquery *thys)
{
    AjPMartquery pthis = NULL;

    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    ajMartLocDel(&pthis->Loc);
    ajMartDatasetDel(&pthis->Dataset);
    ajMartAttributeDel(&pthis->Atts);
    ajMartFilterDel(&pthis->Filters);

    if(pthis->Config)
        ajDomDocumentDestroyNode(pthis->Config,pthis->Config);
    
    ajStrDel(&pthis->Reghost);
    ajStrDel(&pthis->Regpath);
    ajStrDel(&pthis->Marthost);
    ajStrDel(&pthis->Martpath);
    ajStrDel(&pthis->Query);

    AJFREE(pthis);
    
    *thys = NULL;
    
    return;
}




/* @func ajMartquerySeqinFree *************************************************
**
** Delete Mart query object given a seqin object
**
** @param [u] thys [AjPSeqin] Seqin query object
** @return [void]
******************************************************************************/

void ajMartquerySeqinFree(AjPSeqin thys)
{
    AjPSeqQuery qry = NULL;
    AjPMartquery mq = NULL;

    if(!thys)
        return;

    qry = thys->Query;

    if(!qry)
        return;
    
    mq = (AjPMartquery) qry->QryData;
    
    if(!mq)
        return;

    ajMartqueryDel(&mq);

    qry->QryData = NULL;

    return;
}





/* @func ajMartLocNew ********************************************************
**
** Create object for holding Mart location data read from XML
**
** @return [AjPMartLoc] MartLoc object
******************************************************************************/

AjPMartLoc ajMartLocNew(void)
{
    AjPMartLoc ret = NULL;

    AJNEW0(ret);

    ret->Reg_read  = ajListNew();
    ret->Data_read = ajListNew();
    ret->Url_read  = ajListNew();

    ret->Nregistries = 0;
    ret->Ndatabases  = 0;
    ret->Nurls       = 0;

    return ret;
}




/* @func ajMartLocDel *************************************************
**
** Delete a Mart Location object
**
** @param [u] thys [AjPMartLoc*] MartLoc object
** @return [void]
******************************************************************************/

void ajMartLocDel(AjPMartLoc *thys)
{
    AjPMartLoc pthis;
    ajuint i;
    
    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;
    
    for(i=0; i < pthis->Nregistries; ++i)
        ajTablestrFree(&pthis->Registries[i]);

    if(pthis->Nregistries)
        AJFREE(pthis->Registries);
    
    for(i=0; i < pthis->Ndatabases; ++i)
        ajTablestrFree(&pthis->Databases[i]);

    if(pthis->Ndatabases)
        AJFREE(pthis->Databases);
    
    for(i=0; i < pthis->Nurls; ++i)
        ajTablestrFree(&pthis->Urls[i]);

    if(pthis->Nurls)
        AJFREE(pthis->Urls);
    
    ajListFree(&pthis->Reg_read);
    ajListFree(&pthis->Data_read);    
    ajListFree(&pthis->Url_read);    
    
    AJFREE(pthis);
    *thys = NULL;

    return;
}




/* @func ajMartDatasetNew ****************************************************
**
** Create object for holding Mart Dataset read from XML (or tab-sep list)
**
** @return [AjPMartDataset] MartDataset object
******************************************************************************/

AjPMartDataset ajMartDatasetNew(void)
{
    AjPMartDataset ret = NULL;

    AJNEW0(ret);

    ret->Set_read  = ajListNew();

    ret->Nsets = 0;
    ret->Depth = 0;

    return ret;
}




/* @func ajMartDatasetDel *************************************************
**
** Delete a Mart Dataset object
**
** @param [u] thys [AjPMartDataset*] MartDataset object
** @return [void]
******************************************************************************/

void ajMartDatasetDel(AjPMartDataset *thys)
{
    AjPMartDataset pthis;
    ajuint i;
    
    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;
    
    for(i=0; i < pthis->Nsets; ++i)
        ajTablestrFree(&pthis->Sets[i]);

    if(pthis->Nsets)
        AJFREE(pthis->Sets);
    
    ajListFree(&pthis->Set_read);
    
    AJFREE(pthis);
    *thys = NULL;

    return;
}




/* @func ajMartAttributeNew ****************************************************
**
** Create object for holding Mart Attributes read from XML (or tab-sep list)
**
** @return [AjPMartAttribute] MartAttribute object
******************************************************************************/

AjPMartAttribute ajMartAttributeNew(void)
{
    AjPMartAttribute ret = NULL;

    AJNEW0(ret);

    ret->Att_read  = ajListNew();

    ret->Natts = 0;
    ret->Depth = 0;

    return ret;
}




/* @func ajMartAttributeDel *************************************************
**
** Delete a Mart Attribute object
**
** @param [u] thys [AjPMartAttribute*] MartAttribute object
** @return [void]
******************************************************************************/

void ajMartAttributeDel(AjPMartAttribute *thys)
{
    AjPMartAttribute pthis;
    ajuint i;
    
    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    if(pthis->Attributes)
        for(i=0; i < pthis->Natts; ++i)
            ajTablestrFree(&pthis->Attributes[i]);

    if(pthis->Natts)
        if(pthis->Attributes)
            AJFREE(pthis->Attributes);

    ajListFree(&pthis->Att_read);

    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajMartFilterNew *****************************************************
**
** Create object for holding Mart Filters read from XML (or tab-sep list)
**
** @return [AjPMartFilter] MartFilter object
******************************************************************************/

AjPMartFilter ajMartFilterNew(void)
{
    AjPMartFilter ret = NULL;

    AJNEW0(ret);

    ret->Filter_read  = ajListNew();

    ret->Nfilters = 0;
    ret->Depth = 0;

    return ret;
}




/* @func ajMartFilterDel *************************************************
**
** Delete a Mart Filter object
**
** @param [u] thys [AjPMartFilter*] MartFilter object
** @return [void]
******************************************************************************/

void ajMartFilterDel(AjPMartFilter *thys)
{
    AjPMartFilter pthis;
    ajuint i;
    
    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;
    
    if(pthis->Filters)
        for(i=0; i < pthis->Nfilters; ++i)
            ajTablestrFree(&pthis->Filters[i]);

    if(pthis->Nfilters)
        if(pthis->Filters)
            AJFREE(pthis->Filters);
    
    ajListFree(&pthis->Filter_read);
    
    AJFREE(pthis);
    *thys = NULL;

    return;
}




/* @func ajMartDsinfoNew *****************************************************
**
** Create object for query dataset info
**
** @return [AjPMartdsinfo] Martdsinfo object
******************************************************************************/

AjPMartdsinfo ajMartDsinfoNew(void)
{
    AjPMartdsinfo ret = NULL;

    AJNEW0(ret);

    ret->Attributes  = ajListNew();
    ret->Filters     = ajListNew();

    ret->Name      = ajStrNew();
    ret->Interface = ajStrNew();

    return ret;
}




/* @func ajMartDsinfoDel *************************************************
**
** Delete a Mart Dsinfo object
**
** @param [u] thys [AjPMartdsinfo*] Mart Dsinfo object
** @return [void]
******************************************************************************/

void ajMartDsinfoDel(AjPMartdsinfo *thys)
{
    AjPMartdsinfo pthis;
    AjPStr t = NULL;
    
    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    while(ajListPop(pthis->Attributes,(void **)&t))
        ajStrDel(&t);
    
    while(ajListPop(pthis->Filters,(void **)&t))
        ajStrDel(&t);

    ajListFree(&pthis->Attributes);
    ajListFree(&pthis->Filters);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Interface);

    AJFREE(pthis);
    *thys = NULL;

    return;
}




/* @func ajMartQinfoNew *****************************************************
**
** Create object for query info
**
** @param [r] n [ajuint] number of datasets
** @return [AjPMartqinfo] Martqinfo object
******************************************************************************/

AjPMartqinfo ajMartQinfoNew(ajuint n)
{
    AjPMartqinfo ret = NULL;
    ajuint i;
    
    AJNEW0(ret);

    ret->Hvirtualschema = ajStrNew();
    ret->Hversion       = ajStrNew();
    ret->Hformatter     = ajStrNew();

    ret->Hcount         = ajFalse;
    ret->Hheader = ajFalse;
    ret->Hstamp  = ajTrue;
    ret->Hunique = ajFalse;
    ret->Verify  = ajFalse;

    ret->Dnsets = n;

    AJCNEW(ret->Dsets, n);
    
    for(i=0; i < n; ++i)
        ret->Dsets[i] = ajMartDsinfoNew();
    
    return ret;
}




/* @func ajMartQinfoDel *************************************************
**
** Delete a Mart qinfo object
**
** @param [u] thys [AjPMartqinfo*] Mart Qinfo object
** @return [void]
******************************************************************************/

void ajMartQinfoDel(AjPMartqinfo *thys)
{
    AjPMartqinfo pthis;
    ajuint i;
    
    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    if(pthis->Dsets)
    {
        for(i = 0; i < pthis->Dnsets; ++i)
            ajMartDsinfoDel(&pthis->Dsets[i]);

        AJFREE(pthis->Dsets);
    }
    
    ajStrDel(&pthis->Hvirtualschema);
    ajStrDel(&pthis->Hversion);
    ajStrDel(&pthis->Hformatter);

    AJFREE(pthis);
    *thys = NULL;

    return;
}




/* @funcstatic martRegistryElementstart **************************************
**
** Handler for reading Mart registry XML elements
**
** @param [u] userData [void*] XML structure being loaded
** @param [r] name [const XML_Char*] Close element token name
** @param [r] atts [const XML_Char**] Close element token name
** @return [void]
******************************************************************************/

static void martRegistryElementstart(void *userData, const XML_Char *name,
                                     const XML_Char **atts)
{
    AjPMartLoc locptr   = NULL;
    
    AjPTable t = NULL;

    AjPStr key   = NULL;
    AjPStr value = NULL;

    ajuint i;
    
    locptr = (AjPMartLoc) userData;
    
    if(locptr->Depth > 1)
        ajWarn("Nested element depth (%d) potentially excessive in Mart "
               "registry file", locptr->Depth + 1);


    if(ajCharMatchCaseC("MartRegistry",name))
    {
        ++locptr->Depth;

        return;
    }

    
    if(!(ajCharMatchC(name,"RegistryDBPointer")    ||
         ajCharMatchC(name,"RegistryURLPointer")   ||
         ajCharMatchC(name,"MartDBLocation")       ||
         ajCharMatchC(name,"MartURLLocation")))
        ajWarn("Registry config: expected RegistryDBPointer,"
               "RegistryURLPointer,MartDBLocation\n"
               "or MartURLLocation. Got %s instead",name);

    t = ajTablestrNewLen(REGTABGUESS);

    for(i = 0; atts[i]; i += 2)
    {
        key   = ajStrNewC(atts[i]);
        value = ajStrNewC(atts[i+1]);
        ajTablePut(t, (void *)key, (void *)value);
    }


    if(ajCharMatchCaseC(name,"RegistryDBPointer"))
    {
        ajListPushAppend(locptr->Reg_read,(void *)t);
        ++locptr->Nregistries;
    }
    else if(ajCharMatchCaseC(name,"RegistryURLPointer"))
    {
        ajListPushAppend(locptr->Reg_read,(void *)t);
        ++locptr->Nregistries;
    }
    else if(ajCharMatchCaseC(name,"MartDBLocation"))
    {
        ajListPushAppend(locptr->Data_read,(void *)t);
        ++locptr->Ndatabases;
    }
    else if(ajCharMatchCaseC(name,"MartURLLocation"))
    {
        ajListPushAppend(locptr->Url_read,(void *)t);
        ++locptr->Nurls;
    }
    else
    {
        ajWarn("Could not assign config entry to a registry, DB or URL");
        ajTablestrFree(&t);
    }

    ++locptr->Depth;
    
    return;
}




/* @funcstatic martRegistryElementend **************************************
**
** Handler for reading Mart registry XML elements
**
** @param [u] userData [void*] XML structure being loaded
** @param [r] name [const XML_Char*] Close element token name
** @return [void]
******************************************************************************/

static void martRegistryElementend(void *userData, const XML_Char *name)
{
    AjPMartLoc locptr = NULL;

    (void) name;                /* make it used */

    locptr = (AjPMartLoc) userData;

    --locptr->Depth;

    return;
}




/* @funcstatic martConvertLocToArrays ****************************************
**
** Convert lists used for reading the XML into more usable arrays
** within the same object.
**
** @param [u] loc [AjPMartLoc] registry table
** @return [void] 
******************************************************************************/

static void martConvertLocToArrays(AjPMartLoc loc)
{
    ajuint n;

    if(!loc)
        return;
    
    n = ajListToarray(loc->Reg_read, (void ***) &loc->Registries);
    if(n != loc->Nregistries)
    {
        ajWarn("martConvertLocToArrays: mismatching Registries count");
        return;
    }

    n = ajListToarray(loc->Data_read, (void ***) &loc->Databases);
    if(n != loc->Ndatabases)
    {
        ajWarn("martConvertLocToArrays: mismatching Databases count");
        return;
    }

    n = ajListToarray(loc->Url_read, (void ***) &loc->Urls);
    if(n != loc->Nurls)
    {
        ajWarn("martConvertLocToArrays: mismatching URLs count");
        return;
    }

    
    return;
}




/* @func ajMartregistryParse **************************************
**
** Read a mart registry file
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartregistryParse(AjPSeqin seqin)
{
    XML_Parser parser;
    AjPStr line = NULL;
    AjPFilebuff buff = NULL;
    AjPMartquery mq = NULL;
    AjPMartLoc loc;
    
    int done;
    size_t len;

    buff = seqin->Filebuff;
    
    if(!buff)
        return ajFalse;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    loc = mq->Loc;
    
    if(!loc)
        return ajFalse;

    line = ajStrNew();
    
    parser = XML_ParserCreate(NULL);

    XML_SetElementHandler(parser, martRegistryElementstart,
                          martRegistryElementend);

    XML_SetUserData(parser, loc);

    done = 0;

    do
    {
        ajBuffreadLine(buff,&line);
        done = ajFilebuffIsEmpty(buff);
        len = ajStrGetLen(line);
        
        if(!XML_Parse(parser, line->Ptr, len, done))
        {
            ajWarn("ajMartregistryParse: %s at line %d\n",
                   XML_ErrorString(XML_GetErrorCode(parser)),
                   XML_GetCurrentLineNumber(parser));

            ajStrDel(&line);
            
            return ajFalse;
        }

    } while (!done);

    XML_ParserFree(parser);

    martConvertLocToArrays(loc);
    
    ajStrDel(&line);
    
    return ajTrue;
}




/* @func ajMartGetRegistry ***************************************************
**
** Return contents of a mart registry
**
** @param [u] seqin [AjPSeqin] "Sequence" input object 
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetRegistry(AjPSeqin seqin)
{
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPSeqQuery qry  = NULL;
    
    AjPStr get = NULL;
    
    AjPMartquery mq = NULL;
    
    ajint proxyport = 0;

    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    

    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    qry = seqin->Query;

    if(!qry)
        return ajFalse;
    
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;
    
    ajSeqHttpVersion(qry, &httpver);

    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%d%S?type=registry HTTP/%S\r\n",
                    mq->Reghost, mq->Regport, mq->Regpath, httpver);
    else
        ajFmtPrintS(&get, "GET %S?type=registry HTTP/%S\r\n", mq->Regpath,
                    httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Reghost, mq->Regport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Reghost, mq->Regport, get);

    if(!fp)
    {
        ajWarn("ajMartGetregistry: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }

 
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("socket buffer attach failed for host '%S'",
	      mq->Reghost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);
    
    ajFilebuffHtmlNoheader(seqin->Filebuff);

    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @funcstatic martGetMarttable ****************************************
**
** returns a mart table given a mart name
**
** @param [r] seqin [const AjPSeqin] seqin object
** @param [r] mart [const AjPStr] mart name
** @return [AjPTable] mart table or NULL
******************************************************************************/

static AjPTable martGetMarttable(const AjPSeqin seqin, const AjPStr mart)
{
    AjPTable t       = NULL;
    AjPMartquery pmq = NULL;
    AjPMartLoc loc   = NULL;
    ajuint nurls = 0;
    ajuint i = 0;
    AjPStr key   = NULL;
    AjPStr value = NULL;
    
    if(!seqin)
        return NULL;

    pmq = ajMartGetMartqueryPtr(seqin);
    
    if(!pmq)
        return NULL;

    loc = pmq->Loc;
    nurls = loc->Nurls;

    key = ajStrNewC("name");
    
    for(i=0; i < nurls; ++i)
    {
        t = loc->Urls[i];
        value = ajTableFetch(t,(void *)key);

        if(ajStrMatchCaseS(value,mart))
            break;
    }
    
    ajStrDel(&key);

    if(i == nurls)
        return NULL;

    return t;
}




/* @func ajMartGetDatasets ***************************************************
**
** Return datasets given a mart registry and a mart name within it
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] mart [const AjPStr] "Sequence" mart name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetDatasets(AjPSeqin seqin, const AjPStr mart)
{
    AjPTable marttab = NULL;

    AjPStr spmart = NULL;
    AjPStr khost  = NULL;
    AjPStr kpath  = NULL;
    AjPStr kport  = NULL;
    AjPStr host   = NULL;
    AjPStr path   = NULL;
    AjPStr port   = NULL;
    
    ajuint iport    = 0;
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;
    
    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    
    marttab = martGetMarttable(seqin, mart);
    
    if(!marttab)
    {
        ajWarn("martGetMarttable: No mart available called %S",mart);
        return ajFalse;
    }

    khost = ajStrNewC("host");
    kpath = ajStrNewC("path");
    kport = ajStrNewC("port");
    
    host = ajTableFetch(marttab,(void *)khost);
    path = ajTableFetch(marttab,(void *)kpath);
    port = ajTableFetch(marttab,(void *)kport);

    ajStrDel(&khost);
    ajStrDel(&kpath);
    ajStrDel(&kport);

    if(!host || !path || !port)
    {
        ajWarn("Invalid Mart location Host=%S Path=%S Port=%S",
               host,path,port);

        return ajFalse;
    }
    
    if(!ajStrToUint(port, &iport))
    {
        ajWarn("Invalid Port (%S)",port);

        return ajFalse;
    }
        

    /*
    ** Save the mart host locations at the top of the mart query object
    ** for future use by attribute/filter/query calls on the
    ** selected mart;
    ** This saves unnecessary searches through the tables each time
    ** such a mart is used.
    */

    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    ajStrAssignS(&mq->Marthost, host);
    ajStrAssignS(&mq->Martpath, path);
    mq->Martport = iport;

    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();
    spmart    = ajStrNew();
    
    
    ajSeqHttpVersion(qry, &httpver);

    ajStrAssignS(&spmart,mart);
    martSpacesToHex(&spmart);

    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%S%S?type=datasets&mart=%S "
                    "HTTP/%S\r\n",
                    host, port, path, spmart, httpver);
    else
    ajFmtPrintS(&get, "GET %S?type=datasets&mart=%S HTTP/%S\r\n", path, spmart,
                httpver);

    ajStrDel(&spmart);
    
    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               host, iport, get);
    else
        fp = ajSeqHttpGet(qry, host, iport, get);

    if(!fp)
    {
        ajWarn("ajMartGetdatasets: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }


    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a registry query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartGetdatasets: socket buffer attach failed for host '%S'",
	      host);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);

    ajFilebuffHtmlNoheader(seqin->Filebuff);

    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @funcstatic martTabToToken ****************************************
**
** Parse tab-separated list. Can't use normal AJAX tokens as
** there can be empty fields of the form \t\t
**
** @param [w] token [AjPStr*] token
** @param [r] s [const AjPStr] original string
** @param [r] loc [ajint] current location within s
** @return [ajint] updated value for loc or -1 if EOS
******************************************************************************/

static ajint martTabToToken(AjPStr *token, const AjPStr s, ajint loc)
{
    char *str = NULL;
    ajint p = 0;

    
    if(!token)
        return -1;

    if(!*token)
        return -1;

    ajStrAssignC(token,"");
    
    if(loc < 0)
        return -1;
    
    str = s->Ptr;

    p = loc;

    if(!str[p])
        return -1;

    while(str[p] != '\t' && str[p] != '\n')
    {
        if(!str[p])
            break;

        ++p;
    }

    if(p != loc)
        ajStrAssignSubC(token,str,loc,p-1);

    if(str[p])
        ++p;

    return p;
}




/* @funcstatic martBuffIsXML ****************************************
**
** Tests whether a file buffer is XML
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [AjBool] True if XML
******************************************************************************/

static AjBool martBuffIsXML(AjPFilebuff buff)
{
    ajuint i;
    AjBool isxml = ajFalse;
    AjPStr line = NULL;

    line = ajStrNew();
    
    for(i=0; i < MART_READ_AHEAD; ++i)
    {
        if(!ajBuffreadLine(buff,&line))
            break;

        if(strstr(line->Ptr, "/>"))
        {
            isxml = ajTrue;
            break;
        }
    }

    ajFilebuffReset(buff);
    ajStrDel(&line);
    
    return isxml;
}




/* @funcstatic martTablePush ****************************************
**
** Push a key/value pair to a table
**
** @param [u] table [AjPTable] Table
** @param [r] name [const char *] Key
** @param [r] token [const AjPStr] Value
** @return [void]
******************************************************************************/

static void martTablePush(AjPTable table, const char *name,
                          const AjPStr token)
{
    AjPStr key   = NULL;
    AjPStr value = NULL;
    
    key = ajStrNewC(name);
    value = ajStrNewS(token);
    ajTablePut(table, (void *)key, (void *)value);

    return;
}




/* @funcstatic martSpacesToHex ****************************************
**
** Replace any spaces in a string with %20
**
** @param [u] s [AjPStr*] String
** @return [void]
******************************************************************************/

static void martSpacesToHex(AjPStr *s)
{
    AjPStr stmp = NULL;
    const char *ptr = NULL;
    char c;
    
    if(!s)
        return;

    if(!*s)
        return;

    ptr = ajStrGetPtr(*s);
    
    stmp = ajStrNew();

    while((c = *ptr))
    {
        if(c == ' ')
            ajStrAppendC(&stmp,"%20");
        else
            ajStrAppendK(&stmp,c);

        ++ptr;
    }

    ajStrAssignS(s,stmp);

    ajStrDel(&stmp);
    
    return;
}




/* @funcstatic martParseTabbedDataset ****************************************
**
** Parse dataset information in the original tab-delimited format
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

static AjBool martParseTabbedDataset(AjPSeqin seqin)
{
    AjPFilebuff buff = NULL;
    AjPStr line  = NULL;
    AjPStr token = NULL;
    AjPTable table = NULL;
    AjPMartDataset ds = NULL;
    AjPMartquery mq   = NULL;
    AjBool error = ajFalse;
    
    ajuint n  = 0;
    ajint pos = 0;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    buff  = seqin->Filebuff;
    line  = ajStrNew();
    token = ajStrNew();

    ds = mq->Dataset;
    
    while(ajBuffreadLine(buff,&line))
    {
        if(ajStrGetLen(line) < 10)
            continue;

        table = ajTablestrNewLen(REGTABGUESS);
        
        pos = 0;

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (1)1\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"type",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (2)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"name",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (3)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"displayName",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (4)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"visible",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (5)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"version",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (6)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"unk1",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedDataset: missing tab field (7)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"unk2",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
            ajStrAssignC(&token,"");
        
        martTablePush(table,"interface",token);

        pos = martTabToToken(&token,line,pos);

        if(pos < 0)
            ajStrAssignC(&token,"");
        
        martTablePush(table,"date",token);
        
        ajListPushAppend(ds->Set_read,(void *)table);
        ++ds->Nsets;
    }
    
    ajStrDel(&line);
    ajStrDel(&token);

    if(error)
        return ajFalse;
    
    n = ajListToarray(ds->Set_read, (void ***) &ds->Sets);
    if(n != ds->Nsets)
    {
        ajWarn("martParseTabbedDataset: mismatching Set count");
        return ajFalse;
    }


    
    return ajTrue;
}




/* @func ajMartdatasetParse **************************************
**
** Read a mart dataset buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartdatasetParse(AjPSeqin seqin)
{
    AjBool ret = ajTrue;
    
    if(!martBuffIsXML(seqin->Filebuff))
        ret = martParseTabbedDataset(seqin);
    else
        ajFatal("Looks like the new Biomart XML format for datasets "
                "has just been implemented. New function needed");

    return ret;
}




/* @func ajMartGetAttributes ***************************************************
**
** Return attributes given a mart dataset and a mart host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetAttributes(AjPSeqin seqin, const AjPStr dataset)
{
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;

    const char *vschema = NULL;
    
    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetAttributes: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    

    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    
    ajSeqHttpVersion(qry, &httpver);


    vschema = martGetVirtualSchema(dataset);
    
    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%u%S?type=attributes&dataset=%S"
                    "&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Marthost, mq->Martport, mq->Martpath, dataset,
                    vschema, httpver);
    else
        ajFmtPrintS(&get, "GET %S?type=attributes&dataset=%S&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Martpath, dataset, vschema, httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Marthost, mq->Martport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Marthost, mq->Martport, get);

    if(!fp)
    {
        ajWarn("ajMartGetAttributes: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }

    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a dataset query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartGetAttributes: socket buffer attach failed for host '%S'",
	      mq->Marthost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }


    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);
 
    ajFilebuffHtmlNoheader(seqin->Filebuff);

    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @func ajMartGetAttributesRetry *********************************************
**
** Retry an attribute fetch using the main biomart registry site
** This routine should only be called after an ajMartGetAttribute
** call followed by a ajMartattributesParse has returned an error.
** That typically means that an external server has not provided all
** the attributes fields, possibly due to outdated biomart software.
** As the main biomart registry site contains all the metadata for
** external marts (but not the marts themselves) it can be
** queried using the same dataset. The true mart server should always
** be tried first to minimise load on the main biomart site
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetAttributesRetry(AjPSeqin seqin, const AjPStr dataset)
{
    AjPMartquery mq = NULL;
    AjPStr orighost = NULL;
    AjPStr origpath = NULL;
    AjPStr schema   = NULL;
    
    ajuint origport = 0;
    AjBool ret = ajFalse;
    
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetAttributesRetry: No previous Mart queried.\n"
               "Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }

    ajMartAttributeDel(&mq->Atts);

    mq->Atts = ajMartAttributeNew();

    orighost = ajStrNew();
    origpath = ajStrNew();
 
    ajStrAssignS(&orighost, mq->Marthost);
    ajStrAssignS(&origpath, mq->Martpath);
    origport = mq->Martport;

    ajStrAssignC(&mq->Marthost, DEFAULT_BIOMART_MART_HOST);
    ajStrAssignC(&mq->Martpath, DEFAULT_BIOMART_MART_PATH);
    mq->Martport = DEFAULT_BIOMART_MART_PORT;

    schema = ajStrNewC("default");
    ret = ajMartGetAttributesSchema(seqin,dataset,schema);

    ajStrAssignS(&mq->Marthost, orighost);
    ajStrAssignS(&mq->Martpath, origpath);
    mq->Martport = origport;

    ajStrDel(&orighost);
    ajStrDel(&origpath);
    ajStrDel(&schema);
    
    return ret;
}




/* @func ajMartGetAttributesSchema ********************************************
**
** Return attributes given a mart dataset, a mart host/path/port and a
** schemaname.
**
** This function will be deleted once virtual schemas
** are phased out. Use with caution.
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @param [r] schema [const AjPStr] Schema name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetAttributesSchema(AjPSeqin seqin, const AjPStr dataset,
                                 const AjPStr schema)
{
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;

    const char *vschema = NULL;
    
    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetAttributesSchema: Invalid Mart location "
               "Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    

    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    
    ajSeqHttpVersion(qry, &httpver);


    vschema = ajStrGetPtr(schema);
    
    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%u%S?type=attributes&dataset=%S"
                    "&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Marthost, mq->Martport, mq->Martpath, dataset,
                    vschema, httpver);
    else
        ajFmtPrintS(&get, "GET %S?type=attributes&dataset=%S&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Martpath, dataset, vschema, httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Marthost, mq->Martport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Marthost, mq->Martport, get);

    if(!fp)
    {
        ajWarn("ajMartGetAttributesSchema: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }

    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a dataset query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartGetAttributesSchema: socket buffer attach failed for "
              "host '%S'",
	      mq->Marthost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }


    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);
 
    ajFilebuffHtmlNoheader(seqin->Filebuff);

    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @funcstatic martAttcmp ****************************************************
**
** Finds the sort order of two mart tabbed strings  cast as void.  
**
** For use with {ajListSort}, {ajListUnique} and other functions which 
** require a general function with void arguments.
**
** @param [r] str [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

static int martAttcmp(const void* str, const void* str2)
{
    AjPStr a = NULL;
    AjPStr b = NULL;
    char *aptr = NULL;
    char *bptr = NULL;
    char *p = NULL;
    char *q = NULL;
    ajuint alen;
    ajuint blen;
    
    a = (*(AjPStr const *)str);
    b = (*(AjPStr const *)str2);
    
    aptr = a->Ptr;
    bptr = b->Ptr;

    p = strchr(aptr,(int)'\t');
    q = strchr(bptr,(int)'\t');
    
    alen = p - aptr;
    blen = q - bptr;

    if(alen > blen)
        return 1;

    if(blen > alen)
        return -1;

    return strncmp(aptr,bptr,alen);

}




/* @funcstatic martStrdel *********************************************
**
** Deletes a string when called by ajListSortUnique
**
** @param [r] str [void**] string to delete
** @param [r] cl [void*] not used
** @return [void]
** @@
******************************************************************************/

static void martStrdel(void** str, void* cl) 
{
    (void) cl;				/* make it used */

    ajStrDel((AjPStr*)str);

    return;
}




/* @funcstatic martParseTabbedAttributes *************************************
**
** Parse attribute information in the original tab-delimited format
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

static AjBool martParseTabbedAttributes(AjPSeqin seqin)
{
    AjPFilebuff buff = NULL;
    AjPStr line  = NULL;
    AjPStr token = NULL;
    AjPStr tline = NULL;
    
    AjPTable table = NULL;
    AjPMartAttribute att = NULL;
    AjPMartquery mq   = NULL;
    AjBool error = ajFalse;
    
    ajuint n  = 0;
    ajint pos = 0;
    
    AjPList ulist = NULL;
    
    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;
    
    buff  = seqin->Filebuff;
    line  = ajStrNew();
    token = ajStrNew();

    ulist = ajListNew();
    
    
    att = mq->Atts;

    while(ajBuffreadLine(buff,&line))
    {
        if(ajStrGetLen(line) < 10)
            continue;

        tline = ajStrNewS(line);

        ajListPush(ulist,(void *) tline);
    }

    ajListSortUnique(ulist, martAttcmp, martStrdel);

    while(ajListPop(ulist, (void **)&tline))
    {
        table = ajTablestrNewLen(REGTABGUESS);

        pos = 0;
        
        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedAttributes: missing tab field (1)\n%S",
                   tline);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"name",token);

        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedAttributes: missing tab field (2)\n%S",
                   tline);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"displayName",token);

        /*
        ** Some mart servers only provide the first two fields so 
        ** need to return some indication of an incomplete read
        ** in case an attempt has to be made to get the metadata
        ** from the main biomart (assuming the initial query has been
        ** made to a satellite server
        */

        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            error = ajTrue;
            ajDebug("martParseTabbedAttributes: missing tab field (3)\n%S",
                    tline);
            ajStrAssignC(&token,"");
        }
        

        martTablePush(table,"description",token);

        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            error = ajTrue;
            ajDebug("martParseTabbedAttributes: missing tab field (4)\n%S",
                    tline);
            ajStrAssignC(&token,"");
        }
        
        martTablePush(table,"page",token);


        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            error = ajTrue;
            ajDebug("martParseTabbedAttributes: missing tab field (5)\n%S",
                    tline);
            ajStrAssignC(&token,"");
        }
        
        martTablePush(table,"format",token);

        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            error = ajTrue;
            ajDebug("martParseTabbedAttributes: missing tab field (6)\n%S",
                    tline);
            ajStrAssignC(&token,"");
        }

        martTablePush(table,"tableName",token);

        pos = martTabToToken(&token,tline,pos);
        if(pos < 0)
        {
            error = ajTrue;
            ajDebug("martParseTabbedAttributes: missing tab field (7)\n%S",
                    tline);
            ajStrAssignC(&token,"");
        }

        martTablePush(table,"columnName",token);

        ajListPushAppend(att->Att_read,(void *)table);

        ++att->Natts;

        ajStrDel(&tline);
    }

    ajStrDel(&line);
    ajStrDel(&token);

    if(error)
        return ajFalse;
    
    n = ajListToarray(att->Att_read, (void ***) &att->Attributes);

    if(n != att->Natts)
    {
        ajWarn("martParseTabbedAttributes: mismatching Attribute count");
        return ajFalse;
    }

    ajListFree(&ulist);

    return ajTrue;
}




/* @func ajMartattributesParse **************************************
**
** Read a mart attributes buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartattributesParse(AjPSeqin seqin)
{
    AjBool ret = ajTrue;
    
    if(!martBuffIsXML(seqin->Filebuff))
    {
        ret = martParseTabbedAttributes(seqin);
    }
    else
        ajFatal("Looks like the new Biomart XML format for attributes "
                "has just been implemented. New function needed");
    
    return ret;
}




/* @func ajMartGetFilters ***************************************************
**
** Return filters given a mart dataset and a mart host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetFilters(AjPSeqin seqin, const AjPStr dataset)
{
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;

    const char *vschema = NULL;
    
    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);
    
    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetFilters: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    

    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    
    ajSeqHttpVersion(qry, &httpver);

    vschema = martGetVirtualSchema(dataset);
    
    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%u%S?type=filters&dataset=%S"
                    "&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Marthost, mq->Martport, mq->Martpath, dataset,
                    vschema, httpver);
    else
        ajFmtPrintS(&get, "GET %S?type=filters&dataset=%S&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Martpath, dataset, vschema, httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Marthost, mq->Martport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Marthost, mq->Martport, get);

    if(!fp)
    {
        ajWarn("ajMartGetFilters: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }


    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a dataset query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartGetFilters: socket buffer attach failed for host '%S'",
	      mq->Marthost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);
 
    ajFilebuffHtmlNoheader(seqin->Filebuff);


    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @func ajMartGetFiltersSchema ***********************************************
**
** Return filters given a mart dataset, a mart host/path/port and a schema
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @param [r] schema [const AjPStr] "Sequence" mart schema name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetFiltersSchema(AjPSeqin seqin, const AjPStr dataset,
                              const AjPStr schema)
{
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;

    const char *vschema = NULL;
    
    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);
    
    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetFiltersSchema: Invalid Mart location "
               "Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    

    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    
    ajSeqHttpVersion(qry, &httpver);

    vschema = ajStrGetPtr(schema);
    
    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%u%S?type=filters&dataset=%S"
                    "&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Marthost, mq->Martport, mq->Martpath, dataset,
                    vschema, httpver);
    else
        ajFmtPrintS(&get, "GET %S?type=filters&dataset=%S&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Martpath, dataset, vschema, httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Marthost, mq->Martport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Marthost, mq->Martport, get);

    if(!fp)
    {
        ajWarn("ajMartGetFiltersSchema: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }


    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a dataset query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartGetFiltersSchema: socket buffer attach failed for "
              "host '%S'", mq->Marthost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);
 
    ajFilebuffHtmlNoheader(seqin->Filebuff);


    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @func ajMartGetFiltersRetry ***********************************************
**
** Retry a filters fetch using the main biomart registry site
** This routine should only be called after an ajMartGetFilters
** call followed by a ajMartfiltersParse has returned an error.
** That typically means that an external server has not provided all
** the filters fields, possibly due to outdated biomart software.
** As the main biomart registry site contains all the metadata for
** external marts (but not the marts themselves) it can be
** queried using the same dataset. The true mart server should always
** be tried first to minimise load on the main biomart site
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetFiltersRetry(AjPSeqin seqin, const AjPStr dataset)
{
    AjPMartquery mq = NULL;
    AjPStr orighost = NULL;
    AjPStr origpath = NULL;
    AjPStr schema   = NULL;
    
    ajuint origport = 0;
    AjBool ret = ajFalse;
    
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetFiltersRetry: No previous Mart queried.\n"
               "Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }

    ajMartFilterDel(&mq->Filters);

    mq->Filters = ajMartFilterNew();

    orighost = ajStrNew();
    origpath = ajStrNew();
 
    ajStrAssignS(&orighost, mq->Marthost);
    ajStrAssignS(&origpath, mq->Martpath);
    origport = mq->Martport;

    ajStrAssignC(&mq->Marthost, DEFAULT_BIOMART_MART_HOST);
    ajStrAssignC(&mq->Martpath, DEFAULT_BIOMART_MART_PATH);
    mq->Martport = DEFAULT_BIOMART_MART_PORT;

    schema = ajStrNewC("default");
    ret = ajMartGetFiltersSchema(seqin,dataset,schema);

    ajStrAssignS(&mq->Marthost, orighost);
    ajStrAssignS(&mq->Martpath, origpath);
    mq->Martport = origport;

    ajStrDel(&orighost);
    ajStrDel(&origpath);
    ajStrDel(&schema);
    
    return ret;
}




/* @funcstatic martParseTabbedFilters ****************************************
**
** Parse filter information in the original tab-delimited format
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

static AjBool martParseTabbedFilters(AjPSeqin seqin)
{
    AjPFilebuff buff = NULL;
    AjPStr line  = NULL;
    AjPStr token = NULL;
    
    AjPTable table = NULL;
    AjPMartFilter filt = NULL;
    AjPMartquery mq   = NULL;
    AjBool error = ajFalse;
    
    ajint pos = 0;
    
    ajuint n = 0;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;
    
    buff  = seqin->Filebuff;
    line  = ajStrNew();
    token = ajStrNew();
    
    filt = mq->Filters;

    while(ajBuffreadLine(buff,&line))
    {
        if(ajStrGetLen(line) < 10)
            continue;

        table = ajTablestrNewLen(REGTABGUESS);

        pos = 0;
        
        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedFilters: missing tab field (1)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"name",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajWarn("martParseTabbedFilters: missing tab field (2)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"displayName",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (3)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"allowedValues",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (4)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"unk",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (5)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"page",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (6)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"type",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (7)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"value",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (8)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"tableName",token);

        pos = martTabToToken(&token,line,pos);
        if(pos < 0)
        {
            ajDebug("martParseTabbedFilters: missing tab field (9)\n%S",line);
            error = ajTrue;
            break;
        }
        
        martTablePush(table,"columnName",token);

        ajListPushAppend(filt->Filter_read,(void *)table);
        
        ++filt->Nfilters;
    }
    
    ajStrDel(&line);
    ajStrDel(&token);

    if(error)
        return ajFalse;
    
    n = ajListToarray(filt->Filter_read, (void ***) &filt->Filters);
    if(n != filt->Nfilters)
    {
        ajWarn("martParseTabbedFilters: mismatching Filter count");
        return ajFalse;
    }

    
    return ajTrue;
}




/* @func ajMartfiltersParse **************************************
**
** Parse a mart filters buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartfiltersParse(AjPSeqin seqin)
{
    AjBool ret = ajTrue;
    
    if(!martBuffIsXML(seqin->Filebuff))
        ret = martParseTabbedFilters(seqin);
    else
        ajFatal("Looks like the new Biomart XML format for filters "
                "has just been implemented. New function needed");
    
    return ret;
}




/* @func ajMartFilterMatch *****************************************************
**
** Associate filters with a given attribute table
**
** @param [u] atab [AjPTable] Attribute table
** @param [r] filt [const AjPMartFilter] filters 
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartFilterMatch(AjPTable atab, const AjPMartFilter filt)
{
    AjPStr tablekey  = NULL;
    AjPStr columnkey = NULL;
    AjPStr namekey   = NULL;
    AjPStr nameval   = NULL;
    
    AjPStr filterkey = NULL;
    AjPStr filterval = NULL;
    
    AjPStr atableval  = NULL;
    AjPStr acolumnval = NULL;
    AjPStr ftableval  = NULL;
    AjPStr fcolumnval = NULL;

    AjPTable ftab = NULL;
    AjBool firstval = ajTrue;
    
    
    ajuint i;
    ajuint nf = 0;
    
    
    columnkey = ajStrNewC("columnName");
    tablekey  = ajStrNewC("tableName");
    namekey   = ajStrNewC("name");
    filterkey = ajStrNewC("filters");

    filterval = ajStrNew();
    
    firstval = ajTrue;

    atableval  = ajTableFetch(atab,(void *)tablekey);
    acolumnval = ajTableFetch(atab,(void *)columnkey);

    nf = filt->Nfilters;
    
    for(i=0; i < nf; ++i)
    {
        ftab = filt->Filters[i];
        
        ftableval  = ajTableFetch(ftab,(void *)tablekey);
        fcolumnval = ajTableFetch(ftab,(void *)columnkey);

        if(!ajStrMatchS(atableval,ftableval) ||
           !ajStrMatchS(acolumnval,fcolumnval))
            continue;

        nameval  = ajTableFetch(ftab,(void *)namekey);

        if(firstval)
        {
            firstval = ajFalse;
            ajFmtPrintS(&filterval,"%S",nameval);
        }
        else
            ajFmtPrintAppS(&filterval,",%S",nameval);

    }

    ajTablePut(atab, (void *)filterkey, (void *)filterval);

    ajStrDel(&columnkey);
    ajStrDel(&tablekey);
    ajStrDel(&namekey);

    return ajTrue;
}




/* @func ajMartAssociate *****************************************************
**
** Associate filters with attributes thgat can use them
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartAssociate(AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    AjPMartAttribute att = NULL;
    AjPMartFilter filt   = NULL;

    AjPTable t = NULL;
    
    ajuint na = 0;

    ajuint i = 0;
    
    mq   = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;
    
    att  = mq->Atts;
    filt = mq->Filters;
    
    na = att->Natts;

    for(i=0; i < na; ++i)
    {
        t = att->Attributes[i];
        ajMartFilterMatch(t, filt);
    }
    
    return ajTrue;
}




/* @func ajMartMakeQueryXml ***************************************************
**
** Produce XML from an AjPMartqinfo object
**
** @param [r] qinfo [const AjPMartqinfo] Mart qinfo object
** @param [u] seqin [AjPSeqin] Seqin object
** @return [void]
******************************************************************************/

void ajMartMakeQueryXml(const AjPMartqinfo qinfo, AjPSeqin seqin)
{
    AjPMartquery mq = NULL;
    
    AjPStr tstr = NULL;
    ajuint i;
    ajuint j;
    ajuint len = 0;
    AjPList lst = NULL;
    
    mq   = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return;
    
    /* Create the XML header */
    
    ajFmtPrintS(&mq->Query,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<!DOCTYPE Query>"
                "<Query ");
    
    if(ajStrGetLen(qinfo->Hvirtualschema))
        ajFmtPrintAppS(&mq->Query,"virtualSchemaName = \"%S\" ",
                       qinfo->Hvirtualschema);

    if(ajStrGetLen(qinfo->Hformatter))
        ajFmtPrintAppS(&mq->Query,"formatter = \"%S\" ",
                       qinfo->Hformatter);

    if(ajStrGetLen(qinfo->Hversion))
        ajFmtPrintAppS(&mq->Query,"datasetConfigVersion = \"%S\" ",
                       qinfo->Hversion);

    if(qinfo->Hcount)
        ajFmtPrintAppS(&mq->Query,"count = \"1\" ");

    if(qinfo->Hheader)
        ajFmtPrintAppS(&mq->Query,"header = \"1\" ");
        
    if(qinfo->Hunique)
        ajFmtPrintAppS(&mq->Query,"uniqueRows = \"1\" ");
        
    if(qinfo->Hstamp)
        ajFmtPrintAppS(&mq->Query,"completionStamp = \"1\" ");

    ajFmtPrintAppS(&mq->Query,">");
    
    /* Loop over datasets */

    for(i=0; i < qinfo->Dnsets; ++i)
    {
        ajFmtPrintAppS(&mq->Query,"<Dataset ");
        ajFmtPrintAppS(&mq->Query,"name = \"%S\" ",qinfo->Dsets[i]->Name);
        ajFmtPrintAppS(&mq->Query,"interface = \"%S\" ",
                       qinfo->Dsets[i]->Interface);
        ajFmtPrintAppS(&mq->Query,">");

        lst = qinfo->Dsets[i]->Filters;
        len = ajListGetLength(lst);
        for(j = 0; j < len; ++j)
        {
            ajListPop(lst,(void **)&tstr);
            ajFmtPrintAppS(&mq->Query,"<Filter name = %S/>",tstr);
            ajListPushAppend(lst,(void *) tstr);
        }
    
        lst = qinfo->Dsets[i]->Attributes;
        len = ajListGetLength(lst);
        for(j = 0; j < len; ++j)
        {
            ajListPop(lst,(void **)&tstr);
            ajFmtPrintAppS(&mq->Query,"<Attribute name = \"%S\" />",tstr);
            ajListPushAppend(lst,(void *) tstr);
        }

        ajFmtPrintAppS(&mq->Query,"</Dataset>");
    }

    ajFmtPrintAppS(&mq->Query,"</Query>");

    return;
}




/* @funcstatic martHttpEncode ************************************************
**
** Replaces unsafe characters with hex codes for http GET
**
** @param [u] str [AjPStr *] string to encode
** @return [AjBool] true if length changed
******************************************************************************/

static AjBool martHttpEncode(AjPStr *str)
{
    const char *p = NULL;
    AjPStr tstr = NULL;
    AjBool ret;
    
    char c;
    static const char *excl="$&+,/:;=?@_-.";
    
    if(!str)
        return ajFalse;
    
    p = ajStrGetPtr(*str);

    tstr = ajStrNew();
    
    
    while((c = *p))
    {
        if(strchr(excl,(int)c))
            ajStrAppendK(&tstr,c);
        else if((c>='A' && c <='Z') || (c>='a' && c<='z') || (c>='0' && c<='9'))
            ajStrAppendK(&tstr,c);
        else
            ajFmtPrintAppS(&tstr,"%%%-02x",(int)c);

        ++p;
    }

    ret = (ajStrGetLen(*str) == ajStrGetLen(tstr)) ? ajFalse : ajTrue;

    ajStrAssignS(str,tstr);

    ajStrDel(&tstr);
    
    return ret;
}




/* @func ajMartSendQuery ***************************************************
**
** Send a query, given biomart query xml, to a mart server host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartSendQuery(AjPSeqin seqin)
{
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;

    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);
    
    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartSendQuery: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }

    if(!ajStrGetLen(mq->Query))
    {
        ajWarn("ajMartSendQuery: No biomart query specified");

        return ajFalse;
    }
    
    martHttpEncode(&mq->Query);
    
    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    
    ajSeqHttpVersion(qry, &httpver);


    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%u%S?query=%S "
                    "HTTP/%S\r\n",
                    mq->Marthost, mq->Martport, mq->Martpath, mq->Query,
                    httpver);
    else
        ajFmtPrintS(&get, "GET %S?query=%S HTTP/%S\r\n",
                    mq->Martpath, mq->Query, httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Marthost, mq->Martport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Marthost, mq->Martport, get);

    if(!fp)
    {
        ajWarn("ajMartSendQuery: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }


    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a dataset query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartSendQuery: socket buffer attach failed for host '%S'",
	      mq->Marthost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);
 
    ajFilebuffHtmlNoheader(seqin->Filebuff);


    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @func ajMartGetReghost ****************************************************
**
** Return the Registry host name
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [const AjPStr] Registry host name or NULL
******************************************************************************/

const AjPStr ajMartGetReghost(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    return mq->Reghost;
}




/* @func ajMartGetRegpath ****************************************************
**
** Return the Registry path name
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [const AjPStr] Registry host name or NULL
******************************************************************************/

const AjPStr ajMartGetRegpath(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    return mq->Regpath;
}




/* @func ajMartGetRegport ****************************************************
**
** Return the Registry port
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [ajuint] Registry port or 0
******************************************************************************/

ajuint ajMartGetRegport(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return 0;

    return mq->Regport;
}




/* @func ajMartGetMarthost ****************************************************
**
** Return the Mart host name
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [const AjPStr] Mart host name or NULL
******************************************************************************/

const AjPStr ajMartGetMarthost(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    return mq->Marthost;
}




/* @func ajMartGetMartpath ****************************************************
**
** Return the Mart path name
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [const AjPStr] Mart host name or NULL
******************************************************************************/

const AjPStr ajMartGetMartpath(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    return mq->Martpath;
}




/* @func ajMartGetMartport ****************************************************
**
** Return the Mart port
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [ajuint] Mart port or 0
******************************************************************************/

ajuint ajMartGetMartport(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return 0;

    return mq->Martport;
}




/* @func ajMartSetReghostC ****************************************************
**
** Set the Registry host name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] reghost [const char *] Registry name
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetReghostC(AjPSeqin seqin, const char *reghost)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    ajStrAssignC(&mq->Reghost,reghost);
    
    return ajTrue;
}




/* @func ajMartSetRegpathC ****************************************************
**
** Return the Registry path name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] regpath [const char *] Registry path
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetRegpathC(AjPSeqin seqin, const char *regpath)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    ajStrAssignC(&mq->Regpath,regpath);
    
    return ajTrue;
}




/* @func ajMartSetReghostS ***************************************************
**
** Set the Registry host name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] reghost [const AjPStr] Registry name
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetReghostS(AjPSeqin seqin, const AjPStr reghost)
{
    if(ajMartSetReghostC(seqin,reghost->Ptr))
        return ajTrue;

    return ajFalse;
}




/* @func ajMartSetRegpathS ****************************************************
**
** Return the Registry path name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] regpath [const AjPStr] Registry path
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetRegpathS(AjPSeqin seqin, const AjPStr regpath)
{
    if(ajMartSetRegpathC(seqin,regpath->Ptr))
        return ajTrue;

    return ajFalse;
}




/* @func ajMartSetRegport ****************************************************
**
** Return the Registry port
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] regport [ajuint] Registry port
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetRegport(AjPSeqin seqin, ajuint regport)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    mq->Regport = regport;

    return ajTrue;
}




/* @func ajMartSetMarthostC ****************************************************
**
** Set the Mart host name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] marthost [const char *] Mart name
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetMarthostC(AjPSeqin seqin, const char *marthost)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    ajStrAssignC(&mq->Marthost,marthost);
    
    return ajTrue;
}




/* @func ajMartSetMartpathC ***************************************************
**
** Set the Mart path name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] martpath [const char *] Mart path
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetMartpathC(AjPSeqin seqin, const char *martpath)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    ajStrAssignC(&mq->Martpath,martpath);
    
    return ajTrue;
}




/* @func ajMartSetMarthostS ***************************************************
**
** Set the Mart host name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] marthost [const AjPStr] Mart name
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetMarthostS(AjPSeqin seqin, const AjPStr marthost)
{
    if(ajMartSetMarthostC(seqin,marthost->Ptr))
        return ajTrue;

    return ajFalse;
}




/* @func ajMartSetMartpathS ***************************************************
**
** Set the Mart path name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] martpath [const AjPStr] Mart path
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetMartpathS(AjPSeqin seqin, const AjPStr martpath)
{
    if(ajMartSetMartpathC(seqin,martpath->Ptr))
        return ajTrue;

    return ajFalse;
}




/* @func ajMartSetMartport ****************************************************
**
** Set the Mart port
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] martport [ajuint] Mart port
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartSetMartport(AjPSeqin seqin, ajuint martport)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    mq->Martport = martport;

    return ajTrue;
}




/* @func ajMartFixRegistry ****************************************************
**
** Try to fill in missing registry fields
**
** @param [u] seqin [AjPSeqin] Seqin object
** @return [void]
******************************************************************************/

void ajMartFixRegistry(AjPSeqin seqin)
{
    if(!ajStrGetLen(ajMartGetReghost(seqin)))
        ajMartSetReghostC(seqin,DEFAULT_BIOMART_REG_HOST);

    if(!ajStrGetLen(ajMartGetRegpath(seqin)))
        ajMartSetRegpathC(seqin,DEFAULT_BIOMART_REG_PATH);

    if(!ajMartGetRegport(seqin))
        ajMartSetRegport(seqin,DEFAULT_BIOMART_REG_PORT);

    return;
}




/* @func ajMartFixMart ****************************************************
**
** Try to fill in missing mart fields
**
** @param [u] seqin [AjPSeqin] Seqin object
** @return [void] True if valid
******************************************************************************/

void ajMartFixMart(AjPSeqin seqin)
{
    if(!ajStrGetLen(ajMartGetMarthost(seqin)))
        ajMartSetMarthostC(seqin,DEFAULT_BIOMART_MART_HOST);

    if(!ajStrGetLen(ajMartGetMartpath(seqin)))
        ajMartSetMartpathC(seqin,DEFAULT_BIOMART_MART_PATH);

    if(!ajMartGetMartport(seqin))
        ajMartSetMartport(seqin,DEFAULT_BIOMART_MART_PORT);

    return;
}




/* @funcstatic martMatchAttribute *********************************************
**
** Test whether a given name is found in the attributes tables
**
** @param [r] name [const AjPStr] name
** @param [r] atts [const AjPMartAttribute] Attribute tables
** @return [AjBool] True if match found
******************************************************************************/

static AjBool martMatchAttribute(const AjPStr name, const AjPMartAttribute atts)
{
    ajuint i;
    ajuint n;
    AjBool ret = ajFalse;
    AjPStr key = NULL;
    AjPStr val = NULL;
    
    n = atts->Natts;

    key = ajStrNewC("name");
    
    for(i = 0; i < n; ++i)
    {
        val = ajTableFetch(atts->Attributes[i],(void *)key);

        if(ajStrMatchS(val, name))
        {
            ret = ajTrue;
            break;
        }
    }

    ajStrDel(&key);

    return ret;
}




/* @funcstatic martMatchFilter *********************************************
**
** Test whether a given name is found in the filters tables
**
** @param [r] name [const AjPStr] name
** @param [r] filts [const AjPMartFilter] Filter tables
** @return [AjBool] True if match found
******************************************************************************/

static AjBool martMatchFilter(const AjPStr name, const AjPMartFilter filts)
{
    ajuint i;
    ajuint n;
    AjBool ret = ajFalse;
    AjPStr key = NULL;
    AjPStr val = NULL;
    
    n = filts->Nfilters;

    key = ajStrNewC("name");
    
    for(i = 0; i < n; ++i)
    {
        val = ajTableFetch(filts->Filters[i],(void *)key);

        if(ajStrMatchS(val, name))
        {
            ret = ajTrue;
            break;
        }
    }

    ajStrDel(&key);


    return ret;
}




/* @func ajMartAttachMartquery ***********************************************
**
** Add a mart query object to a seqin object
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [u] mq [AjPMartquery] Martquery object
** @return [void]
******************************************************************************/

void ajMartAttachMartquery(AjPSeqin seqin, AjPMartquery mq)
{
    AjPSeqQuery qry = NULL;

    if(!seqin)
        return;

    qry = seqin->Query;

    if(!qry)
        return;

    qry->QryData = (void *) mq;

    return;
}




/* @func ajMartGetMartqueryPtr ***********************************************
**
** Return a seqin Query pointer
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [AjPMartquery] Martquery
******************************************************************************/

AjPMartquery ajMartGetMartqueryPtr(const AjPSeqin seqin)
{
    AjPSeqQuery qry = NULL;
    AjPMartquery mq = NULL;

    if(!seqin)
        return NULL;

    qry = seqin->Query;

    if(!qry)
        return NULL;

    mq = (AjPMartquery) qry->QryData;

    return mq;
}




/* @func ajStrtokQuotR *******************************************************
**
** Reentrant strtok that disallows tokenisation at delimiter
** characters if they are within quotation chracters
** e.g. tokenisation of  -token1="a,b,c",-token2
** will yield   -token1="a,b,c"   and   -token2
** if the delimstr contains ',' and the quotstr contains '"'
**
** The source string is unchanged
**
** @param [r] srcstr [const char*] Source string
** @param [r] delimstr [const char*] Delimiter string
** @param [r] quotstr [const char*] Allowable quotes(s)
** @param [r] ptrptr [const char**] Saved position in srcstr
** @param [w] buf [AjPStr*] Results buffer (the token)
** @return [char*] Pointer to start of token (in buf) or NULL
******************************************************************************/

char* ajStrtokQuotR(const char *srcstr, const char *delimstr,
                    const char *quotstr, const char **ptrptr,
                    AjPStr *buf)
{
    const char *p = NULL;
    const char *q = NULL;
    AjBool quoted = ajFalse;
    ajuint len    = 0;
    char c = '\0';

    if(!*buf)
        *buf = ajStrNew();

    if(srcstr != NULL)
        p = srcstr;
    else
        p = *ptrptr;

    ajStrAssignC(buf,"");
    
    if(!*p)
        return NULL;

    len = strspn(p,delimstr);
    p += len;
    if(!*p)
        return NULL;

    q = p;
    quoted = ajFalse;

    while(*q)
    {
        if(strchr(quotstr,(int)*q))
        {
            c = *q;
            ++q;
            quoted = ajTrue;

            while(*q && *q != c)
                ++q;

            if(*q == c)
            {
                ++q;
                quoted = ajFalse;
            }

            continue;
        }

        if(strchr(delimstr,(int) *q))
            break;

        ++q;
    }

    if(!*q && quoted)
    {
        *ptrptr = q;
        return NULL;
    }

    ajStrAssignSubC(buf,p,0,q-p-1);
    *ptrptr = q;

    return ajStrGetuniquePtr(buf);
}




/* @func ajMartParseParameters ***********************************************
**
** Parses attribute and filter strings and load a given Qinfo object
** with them
**
** @param [u] qinfo [AjPMartqinfo] Query object
** @param [r] atts [const AjPStr] Attributes
** @param [r] filts [const AjPStr] Filters
** @param [r] idx [ajuint] dataset index
** @return [AjBool] False on parsing or loading error
******************************************************************************/

AjBool ajMartParseParameters(AjPMartqinfo qinfo, const AjPStr atts,
                             const AjPStr filts, ajuint idx)
{
    AjPStr buf = NULL;
    AjPStr pushstr = NULL;
    AjPStr namstr = NULL;
    AjPStr valstr = NULL;
    
    const char *ptrsave = NULL;
    const char *p = NULL;
    const char *q = NULL;
    
    static const char *delimstr=",\t\n";
    static const char *quotstr="\"'";
    const char *tokstr = NULL;
    ajuint natts = 0;
    
    buf    = ajStrNew();
    namstr = ajStrNew();
    valstr = ajStrNew();
    
    tokstr = ajStrGetPtr(atts);
    
    while(ajStrtokQuotR(tokstr,delimstr,quotstr,&ptrsave,&buf))
    {
        tokstr = NULL;
        pushstr = ajStrNewS(buf);
        ajStrTrimWhite(&pushstr);
        
        ajListPushAppend(qinfo->Dsets[idx]->Attributes,(void *)pushstr);
        ++natts;
    }

    if(!natts)
    {
        ajStrDel(&buf);
        ajStrDel(&namstr);
        ajStrDel(&valstr);
        
        ajWarn("ajMartParseParameters: No parseable attributes given [%S]",
               atts);

        return ajFalse;
    }


    tokstr = ajStrGetPtr(filts);
    
    while(ajStrtokQuotR(tokstr,delimstr,quotstr,&ptrsave,&buf))
    {
        tokstr = NULL;
        ajStrTrimWhite(&buf);

        p = ajStrGetPtr(buf);
        
        if(ajStrPrefixC(buf,"!"))  /* Negated boolean */
        {
            if((q=strchr(p,(int)'=')))
            {                      /* Bad negated boolean */
                if(q == p+1)
                {
                    ajWarn("ajMartParseParameters: Illegal filter [%S]",buf);
                    ajStrDel(&buf);
                    ajStrDel(&namstr);
                    ajStrDel(&valstr);
                    
                    return ajFalse;
                }
                    
                ajWarn("ajMartParseParameters: Illegal assignment for "
                       "negated boolean filter [%S]\n"
                       "Stripping assignment and adding boolean",buf);
                ++p;
                ajStrAssignSubC(&namstr,p,0,q-p-1);

                pushstr = ajStrNew();
                ajFmtPrintS(&pushstr,"\"%S\" excluded = \"1\"",namstr);
                ajListPushAppend(qinfo->Dsets[idx]->Filters,(void *)pushstr);
            }
            else
            {                      /* Good negated boolean */
                if(ajStrGetLen(buf) < 2)
                {
                    ajWarn("ajMartParseParameters: Illegal filter [%S]",buf);
                    ajStrDel(&buf);
                    ajStrDel(&namstr);
                    ajStrDel(&valstr);
                    
                    return ajFalse;
                }

                ++p;
                ajStrAssignC(&namstr,p);

                pushstr = ajStrNew();
                ajFmtPrintS(&pushstr,"\"%S\" excluded = \"1\"",namstr);
                ajListPushAppend(qinfo->Dsets[idx]->Filters,(void *)pushstr);
            }

            continue;
        }


        /* Test for assignment type of filter '=' */
        
        if((q=strchr(p,(int)'=')))
        {
            if(q == p)
            {
                ajWarn("ajMartParseParameters: Illegal filter [%S]",buf);
                ajStrDel(&buf);
                ajStrDel(&namstr);
                ajStrDel(&valstr);
                
                return ajFalse;
            }

            ajStrAssignSubC(&namstr,p,0,q-p-1);
            p = q;
            while(*p == '=')
                ++p;

            ajStrAssignC(&valstr,p);
            ajStrTrimC(&valstr,"\"'");

            pushstr = ajStrNew();
            ajFmtPrintS(&pushstr,"\"%S\" value = \"%S\"",namstr,valstr);
            ajListPushAppend(qinfo->Dsets[idx]->Filters,(void *)pushstr);

            continue;
        }
        
        /* Must be a non-negated boolean */
        
        pushstr = ajStrNew();
        ajFmtPrintS(&pushstr,"\"%s\" excluded = \"0\"",p);
        ajListPushAppend(qinfo->Dsets[idx]->Filters,(void *)pushstr);
    }

    ajStrDel(&buf);
    ajStrDel(&namstr);
    ajStrDel(&valstr);
    
    return ajTrue;
}




/* @func ajMartSetQuerySchemaC ************************************************
**
** Set the Qinfo Virtual Schema field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] schema [const char*] Virtual schema
** @return [void]
******************************************************************************/

void ajMartSetQuerySchemaC(AjPMartqinfo qinfo, const char *schema)
{
    if(!qinfo)
        return;

    ajStrAssignC(&qinfo->Hvirtualschema, schema);

    return;
}




/* @func ajMartSetQueryVersionC ************************************************
**
** Set the Qinfo Software Version field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] version [const char*] Version
** @return [void]
******************************************************************************/

void ajMartSetQueryVersionC(AjPMartqinfo qinfo, const char *version)
{
    if(!qinfo)
        return;

    ajStrAssignC(&qinfo->Hversion, version);

    return;
}




/* @func ajMartSetQueryFormatC ***********************************************
**
** Set the Qinfo Format field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] format [const char*] Format
** @return [void]
******************************************************************************/

void ajMartSetQueryFormatC(AjPMartqinfo qinfo, const char *format)
{
    if(!qinfo)
        return;

    ajStrAssignC(&qinfo->Hformatter, format);

    return;
}




/* @func ajMartSetQueryCount ***********************************************
**
** Set the Qinfo Count (unique ID count only) field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] count [AjBool] count
** @return [void]
******************************************************************************/

void ajMartSetQueryCount(AjPMartqinfo qinfo, AjBool count)
{
    if(!qinfo)
        return;

    qinfo->Hcount = count;

    return;
}




/* @func ajMartSetQueryHeader ***********************************************
**
** Set the Qinfo Header (column title) field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] header [AjBool] Header
** @return [void]
******************************************************************************/

void ajMartSetQueryHeader(AjPMartqinfo qinfo, AjBool header)
{
    if(!qinfo)
        return;

    qinfo->Hheader = header;

    return;
}




/* @func ajMartSetQueryUnique ***********************************************
**
** Set the Qinfo Unique rows field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] unique [AjBool] Unique
** @return [void]
******************************************************************************/

void ajMartSetQueryUnique(AjPMartqinfo qinfo, AjBool unique)
{
    if(!qinfo)
        return;

    qinfo->Hunique = unique;

    return;
}




/* @func ajMartSetQueryStamp ***********************************************
**
** Set the Qinfo Completion Stamp ([success]) field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] stamp [AjBool] Stamp
** @return [void]
******************************************************************************/

void ajMartSetQueryStamp(AjPMartqinfo qinfo, AjBool stamp)
{
    if(!qinfo)
        return;

    qinfo->Hstamp = stamp;

    return;
}




/* @func ajMartSetQueryVerify ***********************************************
**
** Set the Qinfo verification field to test that given atts & filts
** exist in the dataset
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] verify [AjBool] verify flag
** @return [void]
******************************************************************************/

void ajMartSetQueryVerify(AjPMartqinfo qinfo, AjBool verify)
{
    if(!qinfo)
        return;

    qinfo->Verify = verify;

    return;
}




/* @func ajMartSetQueryDatasetName ********************************************
**
** Set a Qinfo dataset name field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] name [const AjPStr] Name
** @param [r] idx [ajuint] Dataset number (0->n-1)
** @return [void]
******************************************************************************/

void ajMartSetQueryDatasetName(AjPMartqinfo qinfo, const AjPStr name,
                               ajuint idx)
{
    if(!qinfo)
        return;

    if(!qinfo->Dsets)
        return;

    if(idx > qinfo->Dnsets - 1)
        return;
    
    ajStrAssignS(&qinfo->Dsets[idx]->Name, name);
    
    return;
}




/* @func ajMartSetQueryDatasetInterfaceC **************************************
**
** Set a Qinfo dataset interface field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] iface [const char*] Interface
** @param [r] idx [ajuint] Dataset number (0->n-1)
** @return [void]
******************************************************************************/

void ajMartSetQueryDatasetInterfaceC(AjPMartqinfo qinfo, const char *iface,
                                     ajuint idx)
{
    if(!qinfo)
        return;

    if(!qinfo->Dsets)
        return;

    if(idx > qinfo->Dnsets - 1)
        return;
    
    ajStrAssignC(&qinfo->Dsets[idx]->Interface, iface);
    
    return;
}




/* @func ajMartCheckQinfo ****************************************************
**
** Try to fill in missing fields and check user attributes/filters
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [u] qinfo [AjPMartqinfo] Mart qinfo object
** @return [AjBool] True if valid
******************************************************************************/

AjBool ajMartCheckQinfo(AjPSeqin seqin, AjPMartqinfo qinfo)
{
    AjPMartquery mq = NULL;
    AjPStr listval = NULL;
    AjPStr fname   = NULL;
    
    AjPMartAttribute att = NULL;
    AjPMartFilter filt    = NULL;
    AjPMartdsinfo dsinfo = NULL;
    

    ajuint i;
    ajuint j;
    ajuint n;
    

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!qinfo)
        return ajFalse;
    
    ajMartFixMart(seqin);
    

    if(!qinfo->Verify)
        return ajTrue;

    ajMartFixRegistry(seqin);

    for(i=0; i < qinfo->Dnsets; ++i)
    {
        /*
        ** Delete Existing atts. They may be from a previous query.
        ** Also, they could interfere with multi-dataset queries.
        */

        if(mq->Atts)
        {
            ajMartAttributeDel(&mq->Atts);
            mq->Atts = ajMartAttributeNew();
        }

        dsinfo = qinfo->Dsets[i];
        
        if(!ajMartGetAttributes(seqin,dsinfo->Name))
        {
            ajWarn("ajMartCheckQinfo: Cannot retrieve attributes for dataset "
                   "%S",dsinfo->Name);

            return ajFalse;
        }

        ajMartattributesParse(seqin);

        att = mq->Atts;

        n = ajListGetLength(dsinfo->Attributes);

        for(j=0; j < n; ++j)
        {
            ajListPop(dsinfo->Attributes,(void **)&listval);
            ajListPushAppend(dsinfo->Attributes,(void *)listval);

            if(!martMatchAttribute(listval, att))
            {
                ajWarn("ajMartCheckQinfo: Attribute %S not in dataset %S",
                       listval,dsinfo->Name);

                return ajFalse;
            }
        }

    }
    

    fname = ajStrNew();

    
    for(i=0; i < qinfo->Dnsets; ++i)
    {
        /*
        ** Delete Existing filts. They may be from a previous query.
        ** Also, they could interfere with multi-dataset queries.
        */

        if(mq->Filters)
        {
            ajMartFilterDel(&mq->Filters);
            mq->Filters = ajMartFilterNew();
        }
        
        dsinfo = qinfo->Dsets[i];

        if(!ajMartGetFilters(seqin,dsinfo->Name))
        {
            ajWarn("ajMartCheckQinfo: Cannot retrieve filters for dataset "
                   "%S",dsinfo->Name);


            ajStrDel(&fname);
            
            return ajFalse;
        }

        ajMartfiltersParse(seqin);        

        filt = mq->Filters;
        
        n = ajListGetLength(dsinfo->Filters);

        for(j=0; j < n; ++j)
        {
            ajListPop(dsinfo->Filters,(void **)&listval);
            ajListPushAppend(dsinfo->Filters,(void *)listval);
            ajFmtScanS(listval,"%S",&fname);
            ajStrTrimC(&fname,"\"\n ");
            
            if(!martMatchFilter(fname, filt))
            {
                ajWarn("ajMartCheckQinfo: Filter %S not in dataset %S",
                       fname,dsinfo->Name);

                ajStrDel(&fname);

                return ajFalse;
            }
        }
        

        
    }
    
    ajStrDel(&fname);

    return ajTrue;
}




/* @func ajMartGetConfiguration **********************************************
**
** Return config info given a mart dataset and a mart host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
******************************************************************************/

AjBool ajMartGetConfiguration(AjPSeqin seqin, const AjPStr dataset)
{
    ajint proxyport = 0;
    
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPSeqQuery qry = NULL;

    const char *vschema = NULL;
    
    FILE *fp = NULL;
    struct AJTIMEOUT timo;
    
    qry = seqin->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetConfiguration: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    

    /*
    ** Do the GET request
    */
    
    httpver   = ajStrNew();
    proxyname = ajStrNew();
    get       = ajStrNew();

    
    ajSeqHttpVersion(qry, &httpver);


    vschema = martGetVirtualSchema(dataset);
    
    if(ajSeqHttpProxy(qry, &proxyport, &proxyname))
        ajFmtPrintS(&get, "GET http://%S:%u%S?type=configuration&dataset=%S"
                    "&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Marthost, mq->Martport, mq->Martpath, dataset,
                    vschema, httpver);
    else
        ajFmtPrintS(&get, "GET %S?type=configuration&dataset=%S"
                    "&virtualSchema=%s"
                    " HTTP/%S\r\n",
                    mq->Martpath, dataset, vschema, httpver);


    if(ajStrGetLen(proxyname))
        fp = ajSeqHttpGetProxy(qry, proxyname, proxyport,
                               mq->Marthost, mq->Martport, get);
    else
        fp = ajSeqHttpGet(qry, mq->Marthost, mq->Martport, get);

    if(!fp)
    {
        ajWarn("ajMartGetConfiguration: Cannot open fp\n");
        
        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);
        
	return ajFalse;
    }


    /*
    ** The Filebuff needs deleting. It likely is non-NULL
    ** from previous use by (e.g.) a dataset query.
    */
    
    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!seqin->Filebuff)
    {
	ajErr("ajMartGetConfiguration: socket buffer attach failed for "
              "host '%S'",
	      mq->Marthost);

        ajStrDel(&get);
        ajStrDel(&httpver);
        ajStrDel(&proxyname);

        return ajFalse;
    }

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(seqin->Filebuff);

    ajSysTimeoutUnset(&timo);

    ajFilebuffHtmlNoheader(seqin->Filebuff);


    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    
    return ajTrue;
}




/* @func ajMartconfigurationParse **************************************
**
** Parse a mart configuration buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartconfigurationParse(AjPSeqin seqin)
{
    AjPMartquery pmq = NULL;
    AjBool ret = ajFalse;
    
    if(!seqin)
        return ajFalse;

    pmq = ajMartGetMartqueryPtr(seqin);
    
    if(!pmq)
        return ajFalse;

    pmq->Config = ajDomImplementationCreateDocument(NULL,NULL,NULL);
    if(!ajDomReadFilebuff(pmq->Config, seqin->Filebuff))
        ret = ajTrue;
    
    return ret;
}




/* @funcstatic martAttributePageCompar ***************************************
**
** Sort function based on attribute page names
**
** @param [r] a [const void*] First table
** @param [r] b [const void*] Second table
** @return [int] comparison of page strings
******************************************************************************/

static int martAttributePageCompar(const void *a, const void *b)
{
    const AjPTable t1;
    const AjPTable t2;

    AjPStr key = NULL;

    AjPStr v1 = NULL;
    AjPStr v2 = NULL;
    
    t1 = *((AjPTable const *) a);
    t2 = *((AjPTable const *) b);
    
    key = ajStrNewC("page");

    v1 = ajTableFetch(t1,(void *)key);
    v2 = ajTableFetch(t2,(void *)key);

    ajStrDel(&key);

    return strcmp(v1->Ptr,v2->Ptr);
}





/* @func ajMartattributesPageSort *****************************************
**
** Sort attributes based on page name
**
** @param [u] seqin [AjPSeqin] Seqin object holding attribute info
** @return [AjBool] True on success
******************************************************************************/

AjBool ajMartattributesPageSort(AjPSeqin seqin)
{
    AjPMartAttribute att = NULL;
    AjPMartquery pmq = NULL;

    pmq = ajMartGetMartqueryPtr(seqin);
    
    if(!pmq)
        return ajFalse;

    att = pmq->Atts;

    qsort((void *)att->Attributes,att->Natts,sizeof(AjPTable),
          martAttributePageCompar);
    
    return ajTrue;
}




/* @func ajMartNameIsNucC ****************************************************
**
** Test whether name matches any of the set of known nucleic acid
** biomart terms
**
** @param [r] name [const char *] Name
** @return [AjBool] True if nucleic acid name match
******************************************************************************/

AjBool ajMartNameIsNucC(const char *name)
{
    ajuint i;
    const char *p = NULL;
    
    i = 0;

    while((p = martNucTerms[i]))
    {
        if(ajCharMatchCaseC(p, name))
            return ajTrue;

        ++i;
    }

    return ajFalse;
}




/* @func ajMartNameIsProtC ***************************************************
**
** Test whether name matches any of the set of known protein
** biomart terms
**
** @param [r] name [const char *] Name
** @return [AjBool] True if nucleic acid name match
******************************************************************************/

AjBool ajMartNameIsProtC(const char *name)
{
    ajuint i;
    const char *p = NULL;
    
    i = 0;

    while((p = martProtTerms[i]))
    {
        if(ajCharMatchCaseC(p, name))
            return ajTrue;

        ++i;
    }

    return ajFalse;
}




/* @func ajMartTableNameIsNuc ************************************************
**
** Test whether table 'name' value  matches any of the set of known nucleic
** acid biomart terms
**
** @param [r] t [const AjPTable] Table
** @return [AjBool] True if nucleic acid name match
******************************************************************************/

AjBool ajMartTableNameIsNuc(const AjPTable t)
{
    AjPStr key    = NULL;
    AjPStr value  = NULL;
    const char *p = NULL;
    
    key   = ajStrNewC("name");
    value = ajTableFetch(t,(void *)key);

    ajStrDel(&key);

    if(value)
    {
        p = ajStrGetPtr(value);

        if(ajMartNameIsNucC(p))
            return ajTrue;
    }
    
    return ajFalse;
}




/* @func ajMartTableNameIsProt ***********************************************
**
** Test whether table 'name' value  matches any of the set of known protein
** biomart terms
**
** @param [r] t [const AjPTable] Table
** @return [AjBool] True if protein name match
******************************************************************************/

AjBool ajMartTableNameIsProt(const AjPTable t)
{
    AjPStr key    = NULL;
    AjPStr value  = NULL;
    const char *p = NULL;
    
    key   = ajStrNewC("name");
    value = ajTableFetch(t,(void *)key);

    ajStrDel(&key);
    
    if(value)
    {
        p = ajStrGetPtr(value);

        if(ajMartNameIsProtC(p))
            return ajTrue;
    }
    
    return ajFalse;
}




/* @func ajMartCheckHeader ****************************************************
**
** Return an array of attribute names in the order in which
** they are returned by the biomart server.
** Requires that the header flag was set and that an attribute
** query has previously loaded to MartAttribute field. 
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [u] qinfo [AjPMartqinfo] Mart qinfo object
** @return [AjPStr*] Array of attribute names terminated by a NULL entry.
**                   or NULL if mapping cannot be done
******************************************************************************/

AjPStr* ajMartCheckHeader(AjPSeqin seqin, AjPMartqinfo qinfo)
{
    AjPMartquery mq = NULL;
    
    AjPMartAttribute att = NULL;
    AjPFilebuff buff = NULL;
    AjPStr line = NULL;
    AjPStr *ret = NULL;

    AjPStr keyname  = NULL;
    AjPStr keydname = NULL;
    AjPStr value    = NULL;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    
    ajuint nfields = 0;
    
    ajuint i;
    ajuint j;
    
    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    if(!qinfo)
        return NULL;

    if(!qinfo->Hheader)
        return NULL;

    att = mq->Atts;
    if(!att)
        return NULL;
    
    buff = seqin->Filebuff;
    
    if(!buff)
        return NULL;

    line = ajStrNew();

    if(!ajBuffreadLine(buff, &line))
    {
        ajStrDel(&line);
        return NULL;
    }

    nfields = ajStrCalcCountK(line,'\t');
    ++nfields;

    AJCNEW0(ret,nfields + 1);

    ret[nfields] = NULL;

    keyname  = ajStrNewC("name");
    keydname = ajStrNewC("displayName");

    ajStrTokenAssignC(&handle, line, "\t\n");
    token = ajStrNew();
    
    for(i = 0; i < nfields; ++i)
    {
        ret[i] = ajStrNew();

        ajStrTokenNextParseNoskip(&handle,&token);
        
        for(j = 0; j < att->Natts; ++j)
        {
            value = ajTableFetch(att->Attributes[j],(void *)keydname);

            if(ajStrMatchS(value,token))
            {
                value = ajTableFetch(att->Attributes[j],(void *)keyname);
                ajStrAssignS(&ret[i],value);
                break;
            }
        }

        if(j == att->Natts)
            ajErr("ajMartCheckHeader: Cannot match column %S",token);
    }


    ajStrDel(&keyname);
    ajStrDel(&keydname);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrTokenDel(&handle);
    
    return ret;
}

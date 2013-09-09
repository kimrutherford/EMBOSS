/* @source ajmart *************************************************************
**
** AJAX Mart (database) functions
**
** These functions control all aspects of AJAX sequence database access
**
** @author Copyright (C) 2009 Alan Bleasby
** @version $Revision: 1.35 $
** @modified Nov 23 ajb First version
** @modified $Date: 2012/12/07 10:21:39 $ by $Author: rice $
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

#include "expat.h"
#include "ajmart.h"
#include "ajlib.h"
#include "ajhttp.h"
#include "ajfileio.h"



#define REGTABGUESS 100
#define MART_READ_AHEAD 5

#define DEFAULT_BIOMART_REG_HOST "www.biomart.org"
#define DEFAULT_BIOMART_REG_PATH "/biomart/martservice"
#define DEFAULT_BIOMART_REG_PORT 80

#define DEFAULT_BIOMART_MART_HOST "www.biomart.org"
#define DEFAULT_BIOMART_MART_PATH "/biomart/martservice"
#define DEFAULT_BIOMART_MART_PORT 80

#define MART_REGENTRY 1
#define MART_DSENTRY  2
#define MART_ATTENTRY 4
#define MART_FILENTRY 8
#define MART_CONENTRY 16




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
static AjBool martWriteCacheEntry(AjPMartquery mq, AjPFilebuff buff,
                                  ajint type);
static AjBool martReadCacheEntry(AjPMartquery mq, AjPFilebuff *buff,
                                 ajint type);
static AjBool martVerifyOrCreateCacheDir(AjPStr dir);
static void martEncodeHname(AjPStr *hname, const AjPStr host,
                            const AjPStr path, ajuint port);




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




/* @funcstatic martGetVirtualSchema *******************************************
**
** Return a virtual schema given a dataset name
**
** @param [r] dataset [const AjPStr] dataset name
** @return [const char*] virtualSchema value
**
** @release 6.3.0
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




/* @func ajMartqueryNew *******************************************************
**
** Initialise mart query object
**
** @return [AjPMartquery] Mart query object
**
** @release 6.3.0
******************************************************************************/

AjPMartquery ajMartqueryNew(void)
{
    AjPMartquery ret = NULL;

    AJNEW0(ret);
    
    ret->Reghost  = ajStrNew();
    ret->Regpath  = ajStrNew();
    ret->Marthost = ajStrNew();
    ret->Martpath = ajStrNew();
    ret->Cachedir = ajStrNew();
    ret->Query    = ajStrNew();
    ret->Mart     = ajStrNew();
    ret->Dsname   = ajStrNew();
    ret->Loc      = ajMartLocNew();

    ret->Dataset = ajMartDatasetNew();
    ret->Atts    = ajMartAttributeNew();
    ret->Filters = ajMartFilterNew();

    ret->Config = NULL;
    
    ret->Regport  =  80;
    ret->Martport  = 80;

    ret->Cacheflag = 0;
    
    return ret;
}




/* @func ajMartqueryDel *******************************************************
**
** Delete Mart query object
**
** @param [u] thys [AjPMartquery*] Mary query object
** @return [void]
**
** @release 6.3.0
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
        ajDomDocumentDestroyNode(pthis->Config,&pthis->Config);
    
    ajStrDel(&pthis->Reghost);
    ajStrDel(&pthis->Regpath);
    ajStrDel(&pthis->Marthost);
    ajStrDel(&pthis->Martpath);
    ajStrDel(&pthis->Cachedir);
    ajStrDel(&pthis->Mart);
    ajStrDel(&pthis->Dsname);
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
**
** @release 6.3.0
******************************************************************************/

void ajMartquerySeqinFree(AjPSeqin thys)
{
    AjPQuery qry = NULL;
    AjPMartquery mq = NULL;

    if(!thys)
        return;

    qry = thys->Input->Query;

    if(!qry)
        return;
    
    mq = (AjPMartquery) qry->QryData;
    
    if(!mq)
        return;

    ajMartqueryDel(&mq);

    qry->QryData = NULL;

    return;
}





/* @func ajMartLocNew *********************************************************
**
** Create object for holding Mart location data read from XML
**
** @return [AjPMartLoc] MartLoc object
**
** @release 6.3.0
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




/* @func ajMartLocDel *********************************************************
**
** Delete a Mart Location object
**
** @param [u] thys [AjPMartLoc*] MartLoc object
** @return [void]
**
** @release 6.3.0
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




/* @func ajMartDatasetNew *****************************************************
**
** Create object for holding Mart Dataset read from XML (or tab-sep list)
**
** @return [AjPMartDataset] MartDataset object
**
** @release 6.3.0
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




/* @func ajMartDatasetDel *****************************************************
**
** Delete a Mart Dataset object
**
** @param [u] thys [AjPMartDataset*] MartDataset object
** @return [void]
**
** @release 6.3.0
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




/* @func ajMartAttributeNew ***************************************************
**
** Create object for holding Mart Attributes read from XML (or tab-sep list)
**
** @return [AjPMartAttribute] MartAttribute object
**
** @release 6.3.0
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




/* @func ajMartAttributeDel ***************************************************
**
** Delete a Mart Attribute object
**
** @param [u] thys [AjPMartAttribute*] MartAttribute object
** @return [void]
**
** @release 6.3.0
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




/* @func ajMartFilterNew ******************************************************
**
** Create object for holding Mart Filters read from XML (or tab-sep list)
**
** @return [AjPMartFilter] MartFilter object
**
** @release 6.3.0
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




/* @func ajMartFilterDel ******************************************************
**
** Delete a Mart Filter object
**
** @param [u] thys [AjPMartFilter*] MartFilter object
** @return [void]
**
** @release 6.3.0
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




/* @func ajMartDsinfoNew ******************************************************
**
** Create object for query dataset info
**
** @return [AjPMartdsinfo] Martdsinfo object
**
** @release 6.3.0
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




/* @func ajMartDsinfoDel ******************************************************
**
** Delete a Mart Dsinfo object
**
** @param [u] thys [AjPMartdsinfo*] Mart Dsinfo object
** @return [void]
**
** @release 6.3.0
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




/* @func ajMartQinfoNew *******************************************************
**
** Create object for query info
**
** @param [r] n [ajuint] number of datasets
** @return [AjPMartqinfo] Martqinfo object
**
** @release 6.3.0
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




/* @func ajMartQinfoDel *******************************************************
**
** Delete a Mart qinfo object
**
** @param [u] thys [AjPMartqinfo*] Mart Qinfo object
** @return [void]
**
** @release 6.3.0
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




/* @funcstatic martRegistryElementstart ***************************************
**
** Handler for reading Mart registry XML elements
**
** @param [u] userData [void*] XML structure being loaded
** @param [r] name [const XML_Char*] Close element token name
** @param [r] atts [const XML_Char**] Close element token name
** @return [void]
**
** @release 6.3.0
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

    t = ajTablestrNew(REGTABGUESS);

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




/* @funcstatic martRegistryElementend *****************************************
**
** Handler for reading Mart registry XML elements
**
** @param [u] userData [void*] XML structure being loaded
** @param [r] name [const XML_Char*] Close element token name
** @return [void]
**
** @release 6.3.0
******************************************************************************/

static void martRegistryElementend(void *userData, const XML_Char *name)
{
    AjPMartLoc locptr = NULL;

    (void) name;                /* make it used */

    locptr = (AjPMartLoc) userData;

    --locptr->Depth;

    return;
}




/* @funcstatic martConvertLocToArrays *****************************************
**
** Convert lists used for reading the XML into more usable arrays
** within the same object.
**
** @param [u] loc [AjPMartLoc] registry table
** @return [void] 
**
** @release 6.3.0
******************************************************************************/

static void martConvertLocToArrays(AjPMartLoc loc)
{
    ajulong n;

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




/* @func ajMartregistryParse **************************************************
**
** Read a mart registry file
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
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

    buff = seqin->Input->Filebuff;
    
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

        ajDebug("XML_Parse: '%S'", line);

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




/* @func ajMartGetRegistry ****************************************************
**
** Return contents of a mart registry
**
** @param [u] seqin [AjPSeqin] "Sequence" input object 
** @return [AjBool] true on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetRegistry(AjPSeqin seqin)
{
    AjPQuery qry  = NULL;
    
    AjPStr get = NULL;
    
    AjPMartquery mq = NULL;
    
    AjBool force_write = ajFalse;
    ajuint httpcode = 0;

    qry = seqin->Input->Query;

    if(!qry)
        return ajFalse;
    
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    /* First check whether registry is in the cache */

    if(ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_RD))
    {
        /* Try a cache read */
        if(!martReadCacheEntry(mq, &seqin->Input->Filebuff, MART_REGENTRY))
        {
            if(mq->Cacheflag & MART_CACHE_WTIFEMPTY)
                force_write = ajTrue;
            else
                force_write = ajFalse;
        }
        else
            return ajTrue;
    }

    get       = ajStrNew();
    
    
    ajFmtPrintS(&get, "%S?type=registry\r\n", mq->Regpath);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                            qry->DbProxy,
                                            mq->Reghost, mq->Regport, get);

    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    if(httpcode)
    {
        ajWarn("ajMartGetregistry HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    if((ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_WT)) ||
       force_write)
    {
        /* Write to cache */
        martWriteCacheEntry(mq, seqin->Input->Filebuff, MART_REGENTRY);
    }
    
    ajStrDel(&get);
    
    return ajTrue;
}




/* @funcstatic martGetMarttable ***********************************************
**
** returns a mart table given a mart name
**
** @param [r] seqin [const AjPSeqin] seqin object
** @param [r] mart [const AjPStr] mart name
** @return [AjPTable] mart table or NULL
**
** @release 6.3.0
******************************************************************************/

static AjPTable martGetMarttable(const AjPSeqin seqin, const AjPStr mart)
{
    AjPTable t       = NULL;
    AjPMartquery pmq = NULL;
    AjPMartLoc loc   = NULL;
    ajuint nurls = 0;
    ajuint i = 0;
    AjPStr key   = NULL;
    const AjPStr value = NULL;
    
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
        value = ajTableFetchS(t, key);

        if(ajStrMatchCaseS(value,mart))
            break;
    }
    
    ajStrDel(&key);

    if(i == nurls)
        return NULL;

    return t;
}




/* @func ajMartGetDatasets ****************************************************
**
** Return datasets given a mart registry and a mart name within it
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] mart [const AjPStr] "Sequence" mart name
** @return [AjBool] true on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetDatasets(AjPSeqin seqin, const AjPStr mart)
{
    AjPTable marttab = NULL;

    AjPStr spmart = NULL;
    const AjPStr host   = NULL;
    const AjPStr path   = NULL;
    const AjPStr port   = NULL;
    
    ajuint iport    = 0;
    
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;
    
    AjBool force_write = ajFalse;
    ajuint httpcode = 0;

    marttab = martGetMarttable(seqin, mart);
    
    if(!marttab)
    {
        ajWarn("martGetMarttable: No mart available called %S",mart);
        return ajFalse;
    }

    host = ajTableFetchC(marttab, "host");
    path = ajTableFetchC(marttab, "path");
    port = ajTableFetchC(marttab, "port");

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

    qry = seqin->Input->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    ajStrAssignS(&mq->Marthost, host);
    ajStrAssignS(&mq->Martpath, path);
    mq->Martport = iport;


    ajStrAssignS(&mq->Mart, mart);
    
    /* First check whether dataset is in the cache */

    if(ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_RD))
    {
        /* Try a cache read */
        if(!martReadCacheEntry(mq, &seqin->Input->Filebuff, MART_DSENTRY))
        {
            if(mq->Cacheflag & MART_CACHE_WTIFEMPTY)
                force_write = ajTrue;
            else
                force_write = ajFalse;
        }
        else
            return ajTrue;
    }
    
    /*
    ** Do the GET request
    */
    
    get       = ajStrNew();
    spmart    = ajStrNew();
    
    

    ajStrAssignS(&spmart,mart);
    martSpacesToHex(&spmart);

    ajFmtPrintS(&get, "%S?type=datasets&mart=%S", path, spmart);
    
    ajFilebuffDel(&seqin->Input->Filebuff);
    
    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        host, iport, get);

    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    if(httpcode)
    {
        ajWarn("ajMartGetdatasets: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    if((ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_WT)) ||
       force_write)
    {
        /* Write to cache */
        martWriteCacheEntry(mq, seqin->Input->Filebuff, MART_DSENTRY);
    }

    ajStrDel(&get);
    ajStrDel(&spmart);
    
    return ajTrue;
}




/* @funcstatic martTabToToken *************************************************
**
** Parse tab-separated list. Can't use normal AJAX tokens as
** there can be empty fields of the form \t\t
**
** @param [w] token [AjPStr*] token
** @param [r] s [const AjPStr] original string
** @param [r] loc [ajint] current location within s
** @return [ajint] updated value for loc or -1 if EOS
**
** @release 6.3.0
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




/* @funcstatic martBuffIsXML **************************************************
**
** Tests whether a file buffer is XML
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [AjBool] True if XML
**
** @release 6.3.0
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




/* @funcstatic martTablePush **************************************************
**
** Push a key/value pair to a table
**
** @param [u] table [AjPTable] Table
** @param [r] name [const char *] Key
** @param [r] token [const AjPStr] Value
** @return [void]
**
** @release 6.3.0
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




/* @funcstatic martSpacesToHex ************************************************
**
** Replace any spaces in a string with %20
**
** @param [u] s [AjPStr*] String
** @return [void]
**
** @release 6.3.0
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




/* @funcstatic martParseTabbedDataset *****************************************
**
** Parse dataset information in the original tab-delimited format
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
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
    
    ajulong n  = 0;
    ajint pos = 0;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    buff  = seqin->Input->Filebuff;
    line  = ajStrNew();
    token = ajStrNew();

    ds = mq->Dataset;
    
    while(ajBuffreadLine(buff,&line))
    {
        if(ajStrGetLen(line) < 10)
            continue;

        table = ajTablestrNew(REGTABGUESS);
        
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




/* @func ajMartdatasetParse ***************************************************
**
** Read a mart dataset buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartdatasetParse(AjPSeqin seqin)
{
    AjBool ret = ajTrue;
    
    if(!martBuffIsXML(seqin->Input->Filebuff))
        ret = martParseTabbedDataset(seqin);
    else
        ajFatal("Looks like the new Biomart XML format for datasets "
                "has just been implemented. New function needed");

    return ret;
}




/* @func ajMartGetAttributes **************************************************
**
** Return attributes given a mart dataset and a mart host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetAttributes(AjPSeqin seqin, const AjPStr dataset)
{
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;

    const char *vschema = NULL;
    
    AjBool force_write = ajFalse;
    ajuint httpcode = 0;

    qry = seqin->Input->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetAttributes: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    

    ajStrAssignS(&mq->Dsname, dataset);
    
    /* First check whether registry is in the cache */

    if(ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_RD))
    {
        /* Try a cache read */
        if(!martReadCacheEntry(mq, &seqin->Input->Filebuff, MART_ATTENTRY))
        {
            if(mq->Cacheflag & MART_CACHE_WTIFEMPTY)
                force_write = ajTrue;
            else
                force_write = ajFalse;
        }
        else
            return ajTrue;
    }


    vschema = martGetVirtualSchema(dataset);
    
    ajFmtPrintS(&get, "%S?type=attributes&dataset=%S&virtualSchema=%s",
                        mq->Martpath, dataset, vschema);

    ajFilebuffDel(&seqin->Input->Filebuff);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        mq->Marthost, mq->Martport, get);

    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    if(httpcode)
    {
        ajWarn("ajMartGetAttributes: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    if((ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_WT)) ||
       force_write)
    {
        /* Write to cache */
        martWriteCacheEntry(mq, seqin->Input->Filebuff, MART_ATTENTRY);
    }

    ajStrDel(&get);
    
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
**
** @release 6.3.0
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
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetAttributesSchema(AjPSeqin seqin, const AjPStr dataset,
                                 const AjPStr schema)
{
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;

    const char *vschema = NULL;
    
    ajuint httpcode = 0;

    qry = seqin->Input->Query;
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
    

    get       = ajStrNew();
    

    vschema = ajStrGetPtr(schema);
    
    ajFmtPrintS(&get, "%S?type=attributes&dataset=%S&virtualSchema=%s",
                        mq->Martpath, dataset, vschema);

    ajFilebuffDel(&seqin->Input->Filebuff);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
					qry->DbProxy,
					mq->Marthost, mq->Martport, get);

    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);

    if(httpcode)
    {
        ajWarn("ajMartGetAttributesSchema: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    ajStrDel(&get);
    
    return ajTrue;
}




/* @funcstatic martAttcmp *****************************************************
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
**
** @release 6.3.0
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




/* @funcstatic martStrdel *****************************************************
**
** Deletes a string when called by ajListSortUnique
**
** @param [r] str [void**] string to delete
** @param [r] cl [void*] not used
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void martStrdel(void** str, void* cl) 
{
    (void) cl;				/* make it used */

    ajStrDel((AjPStr*)str);

    return;
}




/* @funcstatic martParseTabbedAttributes **************************************
**
** Parse attribute information in the original tab-delimited format
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
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
    
    ajulong n  = 0;
    ajint pos = 0;
    
    AjPList ulist = NULL;
    
    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;
    
    buff  = seqin->Input->Filebuff;
    line  = ajStrNew();
    token = ajStrNew();

    ulist = ajListNew();
    
    
    att = mq->Atts;

    while(ajBuffreadLine(buff,&line))
    {
        ajDebug("martParseTabbedAttributes '%S'", line);
        if(ajStrGetLen(line) < 10)
            continue;

        tline = ajStrNewS(line);

        ajListPush(ulist,(void *) tline);
    }

    ajListSortUnique(ulist, &martAttcmp, &martStrdel);

    while(ajListPop(ulist, (void **)&tline))
    {
        table = ajTablestrNew(REGTABGUESS);

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




/* @func ajMartattributesParse ************************************************
**
** Read a mart attributes buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartattributesParse(AjPSeqin seqin)
{
    AjBool ret = ajTrue;
    
    if(!martBuffIsXML(seqin->Input->Filebuff))
    {
        ret = martParseTabbedAttributes(seqin);
    }
    else
        ajFatal("Looks like the new Biomart XML format for attributes "
                "has just been implemented. New function needed");
    
    return ret;
}




/* @func ajMartGetFilters *****************************************************
**
** Return filters given a mart dataset and a mart host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetFilters(AjPSeqin seqin, const AjPStr dataset)
{
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;

    const char *vschema = NULL;
    
    AjBool force_write = ajFalse;
    ajuint httpcode = 0;

    qry = seqin->Input->Query;
    mq  = ajMartGetMartqueryPtr(seqin);
    
    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetFilters: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    
    ajStrAssignS(&mq->Dsname, dataset);
    
    /* First check whether registry is in the cache */

    if(ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_RD))
    {
        /* Try a cache read */
        if(!martReadCacheEntry(mq, &seqin->Input->Filebuff, MART_FILENTRY))
        {
            if(mq->Cacheflag & MART_CACHE_WTIFEMPTY)
                force_write = ajTrue;
            else
                force_write = ajFalse;
        }
        else
            return ajTrue;
    }

    /*
    ** Do the GET request
    */
    
    get       = ajStrNew();

    vschema = martGetVirtualSchema(dataset);

    ajFmtPrintS(&get, "%S?type=filters&dataset=%S&virtualSchema=%s",
                    mq->Martpath, dataset, vschema);

    ajFilebuffDel(&seqin->Input->Filebuff);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        mq->Marthost, mq->Martport, get);
    
    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);

    if(httpcode)
    {
        ajWarn("ajMartGetFilters: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    if((ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_WT)) ||
       force_write)
    {
        /* Write to cache */
        martWriteCacheEntry(mq, seqin->Input->Filebuff, MART_FILENTRY);
    }

    ajStrDel(&get);
    
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
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetFiltersSchema(AjPSeqin seqin, const AjPStr dataset,
                              const AjPStr schema)
{
    AjPStr get       = NULL;
    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;

    const char *vschema = NULL;
    
    ajuint httpcode = 0;

    qry = seqin->Input->Query;
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
    
    get       = ajStrNew();

    vschema = ajStrGetPtr(schema);
    
    ajFmtPrintS(&get, "GET %S?type=filters&dataset=%S&virtualSchema=%s",
	        mq->Martpath, dataset, vschema);


    ajFilebuffDel(&seqin->Input->Filebuff);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        mq->Marthost, mq->Martport, get);
    
    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    if(httpcode)
    {
        ajWarn("ajMartGetFiltersSchema: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    ajStrDel(&get);
    
    return ajTrue;
}




/* @func ajMartGetFiltersRetry ************************************************
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
**
** @release 6.3.0
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




/* @funcstatic martParseTabbedFilters *****************************************
**
** Parse filter information in the original tab-delimited format
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
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
    
    buff  = seqin->Input->Filebuff;
    line  = ajStrNew();
    token = ajStrNew();
    
    filt = mq->Filters;

    while(ajBuffreadLine(buff,&line))
    {
        ajDebug("martParseTabbedFilters '%S'", line);
        if(ajStrGetLen(line) < 10)
            continue;

        table = ajTablestrNew(REGTABGUESS);

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
    
    n = (ajint) ajListToarray(filt->Filter_read, (void ***) &filt->Filters);
    if(n != filt->Nfilters)
    {
        ajWarn("martParseTabbedFilters: mismatching Filter count");
        return ajFalse;
    }

    
    return ajTrue;
}




/* @func ajMartfiltersParse ***************************************************
**
** Parse a mart filters buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartfiltersParse(AjPSeqin seqin)
{
    AjBool ret = ajTrue;
    
    if(!martBuffIsXML(seqin->Input->Filebuff))
        ret = martParseTabbedFilters(seqin);
    else
        ajFatal("Looks like the new Biomart XML format for filters "
                "has just been implemented. New function needed");
    
    return ret;
}




/* @func ajMartFilterMatch ****************************************************
**
** Associate filters with a given attribute table
**
** @param [u] atab [AjPTable] Attribute table
** @param [r] filt [const AjPMartFilter] filters 
** @return [AjBool] True on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartFilterMatch(AjPTable atab, const AjPMartFilter filt)
{
    AjPStr filterkey = NULL;
    AjPStr filterval = NULL;
    
    const AjPStr atableval  = NULL;
    const AjPStr acolumnval = NULL;
    const AjPStr ftableval  = NULL;
    const AjPStr fcolumnval = NULL;
    const AjPStr nameval = NULL;

    AjPTable ftab = NULL;
    AjBool firstval = ajTrue;
    
    
    ajuint i;
    ajuint nf = 0;
    
    
    filterkey = ajStrNewC("filters");

    filterval = ajStrNew();
    
    firstval = ajTrue;

    atableval  = ajTableFetchC(atab, "tableName");
    acolumnval = ajTableFetchC(atab, "columnName");

    nf = filt->Nfilters;
    
    for(i=0; i < nf; ++i)
    {
        ftab = filt->Filters[i];
        
        ftableval  = ajTableFetchC(ftab, "tableName");
        fcolumnval = ajTableFetchC(ftab, "columnName");

        if(!ajStrMatchS(atableval,ftableval) ||
           !ajStrMatchS(acolumnval,fcolumnval))
            continue;

        nameval  = ajTableFetchC(ftab, "name");

        if(firstval)
        {
            firstval = ajFalse;
            ajFmtPrintS(&filterval,"%S",nameval);
        }
        else
            ajFmtPrintAppS(&filterval,",%S",nameval);

    }

    ajTablePut(atab, (void *)filterkey, (void *)filterval);

    return ajTrue;
}




/* @func ajMartAssociate ******************************************************
**
** Associate filters with attributes thgat can use them
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
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
**
** @release 6.3.0
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
        len = (ajuint) ajListGetLength(lst);
        for(j = 0; j < len; ++j)
        {
            ajListPop(lst,(void **)&tstr);
            ajFmtPrintAppS(&mq->Query,"<Filter name = %S/>",tstr);
            ajListPushAppend(lst,(void *) tstr);
        }
    
        lst = qinfo->Dsets[i]->Attributes;
        len = (ajuint) ajListGetLength(lst);
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




/* @funcstatic martHttpEncode *************************************************
**
** Replaces unsafe characters with hex codes for http GET
**
** @param [u] str [AjPStr *] string to encode
** @return [AjBool] true if length changed
**
** @release 6.3.0
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

    ajDebug("martHttpEncode '%S'\n", *str);

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
    
    ajDebug("    result %3B '%S'\n", ret, *str);

    return ret;
}




/* @func ajMartSendQuery ******************************************************
**
** Send a query, given biomart query xml, to a mart server host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @return [AjBool] true on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartSendQuery(AjPSeqin seqin)
{
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;

    ajuint httpcode = 0;

    qry = seqin->Input->Query;
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
    
    get       = ajStrNew();

    ajFmtPrintS(&get, "%S?query=%S", mq->Martpath, mq->Query);
    
    ajFilebuffDel(&seqin->Input->Filebuff);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        mq->Marthost, mq->Martport, get);
    
    if(!seqin->Input->Filebuff)
	return ajFalse;
 
    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    if(httpcode)
    {
        ajWarn("ajMartSendQuery: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    ajStrDel(&get);
    
    return ajTrue;
}




/* @func ajMartGetReghost *****************************************************
**
** Return the Registry host name
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [const AjPStr] Registry host name or NULL
**
** @release 6.3.0
******************************************************************************/

const AjPStr ajMartGetReghost(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    return mq->Reghost;
}




/* @func ajMartGetRegpath *****************************************************
**
** Return the Registry path name
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [const AjPStr] Registry host name or NULL
**
** @release 6.3.0
******************************************************************************/

const AjPStr ajMartGetRegpath(const AjPSeqin seqin)
{
    AjPMartquery mq = NULL;

    mq = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return NULL;

    return mq->Regpath;
}




/* @func ajMartGetRegport *****************************************************
**
** Return the Registry port
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [ajuint] Registry port or 0
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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




/* @func ajMartSetReghostS ****************************************************
**
** Set the Registry host name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] reghost [const AjPStr] Registry name
** @return [AjBool] True on success
**
** @release 6.3.0
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
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartSetRegpathS(AjPSeqin seqin, const AjPStr regpath)
{
    if(ajMartSetRegpathC(seqin,regpath->Ptr))
        return ajTrue;

    return ajFalse;
}




/* @func ajMartSetRegport *****************************************************
**
** Return the Registry port
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] regport [ajuint] Registry port
** @return [AjBool] True on success
**
** @release 6.3.0
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




/* @func ajMartSetMarthostC ***************************************************
**
** Set the Mart host name
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [r] marthost [const char *] Mart name
** @return [AjBool] True on success
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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




/* @func ajMartFixMart ********************************************************
**
** Try to fill in missing mart fields
**
** @param [u] seqin [AjPSeqin] Seqin object
** @return [void] True if valid
**
** @release 6.3.0
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
**
** @release 6.3.0
******************************************************************************/

static AjBool martMatchAttribute(const AjPStr name, const AjPMartAttribute atts)
{
    ajuint i;
    ajuint n;
    AjBool ret = ajFalse;
    const AjPStr val = NULL;
    
    n = atts->Natts;

    for(i = 0; i < n; ++i)
    {
        val = ajTableFetchC(atts->Attributes[i], "name");

        if(ajStrMatchS(val, name))
        {
            ret = ajTrue;
            break;
        }
    }

    return ret;
}




/* @funcstatic martMatchFilter ************************************************
**
** Test whether a given name is found in the filters tables
**
** @param [r] name [const AjPStr] name
** @param [r] filts [const AjPMartFilter] Filter tables
** @return [AjBool] True if match found
**
** @release 6.3.0
******************************************************************************/

static AjBool martMatchFilter(const AjPStr name, const AjPMartFilter filts)
{
    ajuint i;
    ajuint n;
    AjBool ret = ajFalse;
    const AjPStr val = NULL;
    
    n = filts->Nfilters;

    for(i = 0; i < n; ++i)
    {
        val = ajTableFetchC(filts->Filters[i], "name");

        if(ajStrMatchS(val, name))
        {
            ret = ajTrue;
            break;
        }
    }

    return ret;
}




/* @func ajMartAttachMartquery ************************************************
**
** Add a mart query object to a seqin object
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [u] mq [AjPMartquery] Martquery object
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartAttachMartquery(AjPSeqin seqin, AjPMartquery mq)
{
    AjPQuery qry = NULL;

    if(!seqin)
        return;

    qry = seqin->Input->Query;

    if(!qry)
        return;

    qry->QryData = (void *) mq;

    return;
}




/* @func ajMartGetMartqueryPtr ************************************************
**
** Return a seqin Query pointer
**
** @param [r] seqin [const AjPSeqin] Seqin object
** @return [AjPMartquery] Martquery
**
** @release 6.3.0
******************************************************************************/

AjPMartquery ajMartGetMartqueryPtr(const AjPSeqin seqin)
{
    AjPQuery qry = NULL;
    AjPMartquery mq = NULL;

    if(!seqin)
        return NULL;

    qry = seqin->Input->Query;

    if(!qry)
        return NULL;

    mq = (AjPMartquery) qry->QryData;

    return mq;
}




/* @func ajStrtokQuotR ********************************************************
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
**
** @release 6.3.0
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




/* @func ajMartParseParameters ************************************************
**
** Parses attribute and filter strings and load a given Qinfo object
** with them
**
** @param [u] qinfo [AjPMartqinfo] Query object
** @param [r] atts [const AjPStr] Attributes
** @param [r] filts [const AjPStr] Filters
** @param [r] idx [ajuint] dataset index
** @return [AjBool] False on parsing or loading error
**
** @release 6.3.0
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
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQuerySchemaC(AjPMartqinfo qinfo, const char *schema)
{
    if(!qinfo)
        return;

    ajStrAssignC(&qinfo->Hvirtualschema, schema);

    return;
}




/* @func ajMartSetQueryVersionC ***********************************************
**
** Set the Qinfo Software Version field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] version [const char*] Version
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQueryVersionC(AjPMartqinfo qinfo, const char *version)
{
    if(!qinfo)
        return;

    ajStrAssignC(&qinfo->Hversion, version);

    return;
}




/* @func ajMartSetQueryFormatC ************************************************
**
** Set the Qinfo Format field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] format [const char*] Format
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQueryFormatC(AjPMartqinfo qinfo, const char *format)
{
    if(!qinfo)
        return;

    ajStrAssignC(&qinfo->Hformatter, format);

    return;
}




/* @func ajMartSetQueryCount **************************************************
**
** Set the Qinfo Count (unique ID count only) field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] count [AjBool] count
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQueryCount(AjPMartqinfo qinfo, AjBool count)
{
    if(!qinfo)
        return;

    qinfo->Hcount = count;

    return;
}




/* @func ajMartSetQueryHeader *************************************************
**
** Set the Qinfo Header (column title) field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] header [AjBool] Header
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQueryHeader(AjPMartqinfo qinfo, AjBool header)
{
    if(!qinfo)
        return;

    qinfo->Hheader = header;

    return;
}




/* @func ajMartSetQueryUnique *************************************************
**
** Set the Qinfo Unique rows field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] unique [AjBool] Unique
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQueryUnique(AjPMartqinfo qinfo, AjBool unique)
{
    if(!qinfo)
        return;

    qinfo->Hunique = unique;

    return;
}




/* @func ajMartSetQueryStamp **************************************************
**
** Set the Qinfo Completion Stamp ([success]) field
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] stamp [AjBool] Stamp
** @return [void]
**
** @release 6.3.0
******************************************************************************/

void ajMartSetQueryStamp(AjPMartqinfo qinfo, AjBool stamp)
{
    if(!qinfo)
        return;

    qinfo->Hstamp = stamp;

    return;
}




/* @func ajMartSetQueryVerify *************************************************
**
** Set the Qinfo verification field to test that given atts & filts
** exist in the dataset
**
** @param [u] qinfo [AjPMartqinfo] Seqin object
** @param [r] verify [AjBool] verify flag
** @return [void]
**
** @release 6.3.0
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
**
** @release 6.3.0
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
**
** @release 6.3.0
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




/* @func ajMartCheckQinfo *****************************************************
**
** Try to fill in missing fields and check user attributes/filters
**
** @param [u] seqin [AjPSeqin] Seqin object
** @param [u] qinfo [AjPMartqinfo] Mart qinfo object
** @return [AjBool] True if valid
**
** @release 6.3.0
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

        n = (ajuint) ajListGetLength(dsinfo->Attributes);

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
        
        n = (ajuint) ajListGetLength(dsinfo->Filters);

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




/* @func ajMartGetConfiguration ***********************************************
**
** Return config info given a mart dataset and a mart host/path/port
**
** @param [u] seqin [AjPSeqin] "Sequence" input object
** @param [r] dataset [const AjPStr] "Sequence" mart dataset name
** @return [AjBool] true on success
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartGetConfiguration(AjPSeqin seqin, const AjPStr dataset)
{
    AjPStr get       = NULL;

    AjPMartquery mq = NULL;
    AjPQuery qry = NULL;

    const char *vschema = NULL;
    
    AjBool force_write = ajFalse;
    ajuint httpcode = 0;
   
    qry = seqin->Input->Query;
    mq  = ajMartGetMartqueryPtr(seqin);

    if(!mq)
        return ajFalse;

    if(!mq->Marthost || !mq->Martpath)
    {
        ajWarn("ajMartGetConfiguration: Invalid Mart location Host=%S Path=%S",
               mq->Marthost,mq->Martpath);

        return ajFalse;
    }
    
    ajStrAssignS(&mq->Dsname, dataset);
    
    /* First check whether registry is in the cache */

    if(ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_RD))
    {
        /* Try a cache read */
        if(!martReadCacheEntry(mq, &seqin->Input->Filebuff, MART_CONENTRY))
        {
            if(mq->Cacheflag & MART_CACHE_WTIFEMPTY)
                force_write = ajTrue;
            else
                force_write = ajFalse;
        }
        else
            return ajTrue;
    }

    /*
    ** Do the GET request
    */
    
    get       = ajStrNew();


    vschema = martGetVirtualSchema(dataset);
    
    ajFmtPrintS(&get, "%S?type=configuration&dataset=%S"
		"&virtualSchema=%s", mq->Martpath, dataset, vschema);

    ajFilebuffDel(&seqin->Input->Filebuff);

    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        mq->Marthost, mq->Martport, get);
    
    if(!seqin->Input->Filebuff)
	return ajFalse;

    httpcode = ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    if(httpcode)
    {
        ajWarn("ajMartGetConfiguration: HTTP code %u from '%S'",
               httpcode, get);
        return ajFalse;
    }

    if((ajStrGetLen(mq->Cachedir) && (mq->Cacheflag & MART_CACHE_WT)) ||
       force_write)
    {
        /* Write to cache */
        martWriteCacheEntry(mq, seqin->Input->Filebuff, MART_CONENTRY);
    }

    ajStrDel(&get);
    
    return ajTrue;
}




/* @func ajMartconfigurationParse *********************************************
**
** Parse a mart configuration buffer
**
** @param [u] seqin [AjPSeqin] Seqin object holding mart info
** @return [AjBool] True on success
**
** @release 6.3.0
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
    if(!ajDomReadFilebuff(pmq->Config, seqin->Input->Filebuff))
        ret = ajTrue;
    
    return ret;
}




/* @funcstatic martAttributePageCompar ****************************************
**
** Sort function based on attribute page names
**
** @param [r] a [const void*] First table
** @param [r] b [const void*] Second table
** @return [int] comparison of page strings
**
** @release 6.3.0
******************************************************************************/

static int martAttributePageCompar(const void *a, const void *b)
{
    const AjPTable t1;
    const AjPTable t2;

    const AjPStr v1 = NULL;
    const AjPStr v2 = NULL;
    
    t1 = *((AjPTable const *) a);
    t2 = *((AjPTable const *) b);
    
    v1 = ajTableFetchC(t1, "page");
    v2 = ajTableFetchC(t2, "page");

    return strcmp(v1->Ptr,v2->Ptr);
}





/* @func ajMartattributesPageSort *********************************************
**
** Sort attributes based on page name
**
** @param [u] seqin [AjPSeqin] Seqin object holding attribute info
** @return [AjBool] True on success
**
** @release 6.3.0
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




/* @func ajMartNameIsNucC *****************************************************
**
** Test whether name matches any of the set of known nucleic acid
** biomart terms
**
** @param [r] name [const char *] Name
** @return [AjBool] True if nucleic acid name match
**
** @release 6.3.0
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




/* @func ajMartNameIsProtC ****************************************************
**
** Test whether name matches any of the set of known protein
** biomart terms
**
** @param [r] name [const char *] Name
** @return [AjBool] True if nucleic acid name match
**
** @release 6.3.0
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




/* @func ajMartTableNameIsNuc *************************************************
**
** Test whether table 'name' value  matches any of the set of known nucleic
** acid biomart terms
**
** @param [r] t [const AjPTable] Table
** @return [AjBool] True if nucleic acid name match
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartTableNameIsNuc(const AjPTable t)
{
    const AjPStr value  = NULL;
    const char *p = NULL;
    
    value = ajTableFetchC(t, "name");

    if(value)
    {
        p = ajStrGetPtr(value);

        if(ajMartNameIsNucC(p))
            return ajTrue;
    }
    
    return ajFalse;
}




/* @func ajMartTableNameIsProt ************************************************
**
** Test whether table 'name' value  matches any of the set of known protein
** biomart terms
**
** @param [r] t [const AjPTable] Table
** @return [AjBool] True if protein name match
**
** @release 6.3.0
******************************************************************************/

AjBool ajMartTableNameIsProt(const AjPTable t)
{
    const AjPStr value  = NULL;
    const char *p = NULL;
    
    value = ajTableFetchC(t, "name");

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
**
** @release 6.3.0
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
    const AjPStr value    = NULL;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    
    ajulong nfields = 0;
    
    ajulong i;
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
    
    buff = seqin->Input->Filebuff;
    
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

        ajStrTokenNextParseNoskip(handle,&token);
        
        for(j = 0; j < att->Natts; ++j)
        {
            value = ajTableFetchS(att->Attributes[j], keydname);

            if(ajStrMatchS(value,token))
            {
                value = ajTableFetchS(att->Attributes[j], keyname);
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




/* @funcstatic martVerifyOrCreateCacheDir *************************************
**
** Check whether cache directory exists, try to create it if not.
**
** @param [u]  dir [AjPStr] directory
** @return [AjBool] true on success
**
** @release 6.4.0
******************************************************************************/

static AjBool martVerifyOrCreateCacheDir(AjPStr dir)
{
    if(!ajFilenameExistsDir(dir))
        if(!ajSysCommandMakedirS(dir))
            return ajFalse;

    return ajTrue;
}




/* @funcstatic martEncodeHname ************************************************
**
** Encode host/path/port into a suitable name for a directory
**
** @param [w] str [AjPStr*] Encoded directory name
** @param [r] host [const AjPStr] Host
** @param [r] path [const AjPStr] Path
** @param [r] port [ajuint] Port
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void martEncodeHname(AjPStr *str, const AjPStr host, const AjPStr path,
                            ajuint port)
{
    AjPStr ho = NULL;
    AjPStr pa = NULL;

    ho = ajStrNew();
    pa = ajStrNew();


    ajStrAssignS(&ho, host);
    ajStrAssignS(&pa, path);

    ajStrExchangeKK(&ho,'.','%');
    ajStrExchangeKK(&pa,'/','^');

    ajFmtPrintS(str,"%S+%S+%u",ho,pa,port);
    
    ajStrDel(&ho);
    ajStrDel(&pa);

    return;
}




/* @func ajMartDecodeHname ****************************************************
**
** Decode directory name into host/path/port
**
** @param [r] dir [const AjPStr] Encoded directory name
** @param [w] host [AjPStr*] Host
** @param [w] path [AjPStr*] Path
** @param [w] port [ajuint*] Port
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajMartDecodeHname(const AjPStr dir, AjPStr *host, AjPStr *path,
                       ajuint *port)
{
    AjPStr str = NULL;
    AjPStrTok handle = NULL;

    str = ajStrNew();

    handle = ajStrTokenNewC(dir, "+");    

    if(!ajStrTokenNextParse(handle, &str))
    {
        ajErr("ajMartDecodeHname: Bad directory name [%S]",dir);
        return;
    }

    ajStrExchangeKK(&str,'%','.');
    ajStrAssignS(host,str);

    if(!ajStrTokenNextParse(handle, &str))
    {
        ajErr("ajMartDecodeHname: Bad directory name [%S]",dir);
        return;
    }

    ajStrExchangeKK(&str,'^','/');
    ajStrAssignS(path,str);

    if(!ajStrTokenNextParse(handle, &str))
    {
        ajErr("ajMartDecodeHname: Bad directory name [%S]",dir);
        return;
    }

    ajStrToUint(str,port);
    
    ajStrDel(&str);
    ajStrTokenDel(&handle);

    return;
}




/* @funcstatic martWriteCacheEntry ********************************************
**
** Write buffer to cache file
**
** @param [u] mq [AjPMartquery] Query object containing host & cache info
** @param [u] buff [AjPFilebuff] File buffer
** @param [r] type [ajint] Type of information held in the file buffer
** @return [AjBool] true on success
**
** @release 6.4.0
******************************************************************************/

static AjBool martWriteCacheEntry(AjPMartquery mq, AjPFilebuff buff, ajint type)
{
    AjPStr dir   = NULL;
    AjPStr hname = NULL;
    AjPStr fname = NULL;
    AjPStr line  = NULL;
    
    AjPFile outf = NULL;
    
    if(!martVerifyOrCreateCacheDir(mq->Cachedir))
    {
        ajErr("Unable to create requested cache directory [%S]",
              mq->Cachedir);
        return ajFalse;
    }


    hname = ajStrNew();
    dir   = ajStrNew();
    
    switch(type)
    {
        case MART_REGENTRY:
            martEncodeHname(&hname,mq->Reghost,mq->Regpath,mq->Regport);
            
            ajFmtPrintS(&dir,"%S%c%S",mq->Cachedir,SLASH_CHAR,
                        hname);

            ajStrDel(&hname);
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&dir);

                return ajFalse;
            }

            fname = ajStrNew();

            ajFmtPrintS(&fname,"%S%cRegistry",dir,SLASH_CHAR);
            ajStrDel(&dir);

            if(!(outf = ajFileNewOutNameS(fname)))
            {
                ajErr("Unable to open registry cache file for write [%S]",
                      fname);

                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            while(ajBuffreadLine(buff,&line))
                ajFmtPrintF(outf,"%S",line);

            ajFileClose(&outf);
            ajStrDel(&line);

            ajFilebuffReset(buff);
            
            break;

        case MART_DSENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);
            
            ajFmtPrintS(&dir,"%S%c%S",mq->Cachedir,SLASH_CHAR,
                        hname);

            /* Create server directory if necessary */
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            /* Create mart directory if necessary */
            ajFmtPrintS(&dir,"%S%c%S%ceMART_%S",mq->Cachedir,SLASH_CHAR,
                        hname,SLASH_CHAR,mq->Mart);

            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            ajStrDel(&hname);
            fname = ajStrNew();

            ajFmtPrintS(&fname,"%S%cDatasets",dir,SLASH_CHAR);
            ajStrDel(&dir);

            if(!(outf = ajFileNewOutNameS(fname)))
            {
                ajErr("Unable to open registry cache file for write [%S]",
                      fname);

                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            while(ajBuffreadLine(buff,&line))
                ajFmtPrintF(outf,"%S",line);

            ajFileClose(&outf);
            ajStrDel(&line);

            ajFilebuffReset(buff);
            
            break;

        case MART_ATTENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);

            ajFmtPrintS(&dir,"%S%cDatasets",mq->Cachedir,SLASH_CHAR);

            /* Create Datasets directory if necessary */
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }
            
            /* Create host directory if necessary */

            ajFmtPrintS(&dir,"%S%cDatasets%c%S",mq->Cachedir,SLASH_CHAR,
                        SLASH_CHAR,hname);
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            /* Create dataset host directory if necessary */
            ajFmtPrintS(&dir,"%S%cDatasets%c%S%c%S",mq->Cachedir,
                        SLASH_CHAR, SLASH_CHAR,
                        hname, SLASH_CHAR, mq->Dsname);

            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            ajStrDel(&hname);
            fname = ajStrNew();

            ajFmtPrintS(&fname,"%S%cAttributes",dir,SLASH_CHAR);
            ajStrDel(&dir);

            if(!(outf = ajFileNewOutNameS(fname)))
            {
                ajErr("Unable to open registry cache file for write [%S]",
                      fname);

                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            while(ajBuffreadLine(buff,&line))
                ajFmtPrintF(outf,"%S",line);

            ajFileClose(&outf);
            ajStrDel(&line);

            ajFilebuffReset(buff);
            
            break;

         case MART_FILENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);

            ajFmtPrintS(&dir,"%S%cDatasets",mq->Cachedir,SLASH_CHAR);

            /* Create Datasets directory if necessary */
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }
            
            /* Create host directory if necessary */

            ajFmtPrintS(&dir,"%S%cDatasets%c%S",mq->Cachedir,SLASH_CHAR,
                        SLASH_CHAR,hname);
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            /* Create dataset host directory if necessary */
            ajFmtPrintS(&dir,"%S%cDatasets%c%S%c%S",mq->Cachedir,
                        SLASH_CHAR, SLASH_CHAR,
                        hname, SLASH_CHAR, mq->Dsname);

            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            ajStrDel(&hname);
            fname = ajStrNew();

            ajFmtPrintS(&fname,"%S%cFilters",dir,SLASH_CHAR);
            ajStrDel(&dir);

            if(!(outf = ajFileNewOutNameS(fname)))
            {
                ajErr("Unable to open registry cache file for write [%S]",
                      fname);

                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            while(ajBuffreadLine(buff,&line))
                ajFmtPrintF(outf,"%S",line);

            ajFileClose(&outf);
            ajStrDel(&line);

            ajFilebuffReset(buff);
            
            break;

         case MART_CONENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);

            ajFmtPrintS(&dir,"%S%cDatasets",mq->Cachedir,SLASH_CHAR);

            /* Create Datasets directory if necessary */
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }
            
            /* Create host directory if necessary */

            ajFmtPrintS(&dir,"%S%cDatasets%c%S",mq->Cachedir,SLASH_CHAR,
                        SLASH_CHAR,hname);
            
            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            /* Create dataset host directory if necessary */
            ajFmtPrintS(&dir,"%S%cDatasets%c%S%c%S",mq->Cachedir,
                        SLASH_CHAR, SLASH_CHAR,
                        hname, SLASH_CHAR, mq->Dsname);

            if(!martVerifyOrCreateCacheDir(dir))
            {
                ajErr("Unable to create requested cache directory [%S]",
                      dir);

                ajStrDel(&hname);
                ajStrDel(&dir);

                return ajFalse;
            }

            ajStrDel(&hname);
            fname = ajStrNew();

            ajFmtPrintS(&fname,"%S%cConfiguration",dir,SLASH_CHAR);
            ajStrDel(&dir);

            if(!(outf = ajFileNewOutNameS(fname)))
            {
                ajErr("Unable to open registry cache file for write [%S]",
                      fname);

                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            while(ajBuffreadLine(buff,&line))
                ajFmtPrintF(outf,"%S",line);

            ajFileClose(&outf);
            ajStrDel(&line);

            ajFilebuffReset(buff);
            
            break;

        default:
            ajStrDel(&hname);
            ajStrDel(&dir);
            break;
    }
    

    return ajTrue;
}




/* @funcstatic martReadCacheEntry *********************************************
**
** Read cache file into file buffer
**
** @param [u] mq [AjPMartquery] Query object containing host & cache info
** @param [u] buff [AjPFilebuff*] File buffer
** @param [r] type [ajint] Type of information held in the file buffer
** @return [AjBool] true on success
**
** @release 6.4.0
******************************************************************************/

static AjBool martReadCacheEntry(AjPMartquery mq, AjPFilebuff *buff,
                                 ajint type)
{
    AjPStr hname = NULL;
    AjPStr fname = NULL;
    AjPStr line  = NULL;
    
    AjPFile inf = NULL;
    
    hname = ajStrNew();
    fname = ajStrNew();

    switch(type)
    {
        case MART_REGENTRY:
            martEncodeHname(&hname,mq->Reghost,mq->Regpath,mq->Regport);
            
            ajFmtPrintS(&fname,"%S%c%S%cRegistry",mq->Cachedir,SLASH_CHAR,
                        hname,SLASH_CHAR);

            ajStrDel(&hname);
            
            if(!(inf = ajFileNewInNameS(fname)))
            {
                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            ajFilebuffDel(buff);
            *buff = ajFilebuffNewNofile();
            
            while(ajReadline(inf,&line))
                ajFilebuffLoadS(*buff,line);
            
            ajFileClose(&inf);
            ajStrDel(&line);

            ajFilebuffReset(*buff);

            break;

        case MART_DSENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);
            
            ajFmtPrintS(&fname,"%S%c%S%ceMART_%S%cDatasets",mq->Cachedir,
                        SLASH_CHAR,hname,SLASH_CHAR,mq->Mart,SLASH_CHAR);

            ajStrDel(&hname);
            
            if(!(inf = ajFileNewInNameS(fname)))
            {
                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            ajFilebuffDel(buff);
            *buff = ajFilebuffNewNofile();
            
            while(ajReadline(inf,&line))
                ajFilebuffLoadS(*buff,line);
            
            ajFileClose(&inf);
            ajStrDel(&line);

            ajFilebuffReset(*buff);

            break;

        case MART_ATTENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);
            
            ajFmtPrintS(&fname,"%S%cDatasets%c%S%c%S%cAttributes",mq->Cachedir,
                        SLASH_CHAR, SLASH_CHAR, hname, SLASH_CHAR, mq->Dsname,
                        SLASH_CHAR);

            ajStrDel(&hname);
            
            if(!(inf = ajFileNewInNameS(fname)))
            {
                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            ajFilebuffDel(buff);
            *buff = ajFilebuffNewNofile();
            
            while(ajReadline(inf,&line))
                ajFilebuffLoadS(*buff,line);
            
            ajFileClose(&inf);
            ajStrDel(&line);

            ajFilebuffReset(*buff);

            break;

        case MART_FILENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);
            
            ajFmtPrintS(&fname,"%S%cDatasets%c%S%c%S%cFilters",mq->Cachedir,
                        SLASH_CHAR, SLASH_CHAR, hname, SLASH_CHAR, mq->Dsname,
                        SLASH_CHAR);

            ajStrDel(&hname);
            
            if(!(inf = ajFileNewInNameS(fname)))
            {
                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            ajFilebuffDel(buff);
            *buff = ajFilebuffNewNofile();
            
            while(ajReadline(inf,&line))
                ajFilebuffLoadS(*buff,line);
            
            ajFileClose(&inf);
            ajStrDel(&line);

            ajFilebuffReset(*buff);

            break;

        case MART_CONENTRY:
            martEncodeHname(&hname,mq->Marthost,mq->Martpath,mq->Martport);
            
            ajFmtPrintS(&fname,"%S%cDatasets%c%S%c%S%cConfiguration",
                        mq->Cachedir, SLASH_CHAR, SLASH_CHAR, hname,
                        SLASH_CHAR, mq->Dsname, SLASH_CHAR);

            ajStrDel(&hname);
            
            if(!(inf = ajFileNewInNameS(fname)))
            {
                ajStrDel(&fname);

                return ajFalse;
            }

            ajStrDel(&fname);

            line = ajStrNew();

            ajFilebuffDel(buff);
            *buff = ajFilebuffNewNofile();
            
            while(ajReadline(inf,&line))
                ajFilebuffLoadS(*buff,line);
            
            ajFileClose(&inf);
            ajStrDel(&line);

            ajFilebuffReset(*buff);

            break;

        default:
            ajStrDel(&hname);
            ajStrDel(&fname);
            break;
    }
    

    return ajTrue;
}

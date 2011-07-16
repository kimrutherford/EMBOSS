/******************************************************************************
** @source support for DAS sequence data sources
**
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
#include "ajdas.h"
#include "ajdas.h"
#include "ajmart.h"
#include "ajseqdb.h"




static void dasRegistryElementstart(void *data, const XML_Char *name,
                                    const XML_Char **atts);

static void dasEntrypointsElementstart(void *data, const XML_Char *name,
                                       const XML_Char **atts);

static AjBool dasQueryParse(AjPFilebuff buff, void* results,
			    XML_StartElementHandler starth,
			    XML_CharacterDataHandler elementh);

static AjBool dasWriteDBdefinition(AjPFile cachef, const AjPDasSource source,
				   AjBool titleAndURI);




/* @func ajDasServerNew *******************************************************
**
** Returns a new DasServer object
**
** @return [AjPDasServer] DAS server object
******************************************************************************/

AjPDasServer ajDasServerNew(void)
{
    AjPDasServer ret = NULL;

    AJNEW0(ret);

    ret->host  = ajStrNew();
    ret->path  = ajStrNew();
    ret->port  = 80;

    ret->sources = ajListNew();

    return ret;
}




/* @func ajDasServerDel *******************************************************
**
** Delete DASserver object
**
** @param [u] thys [AjPDasServer*] DASserver object
** @return [void]
******************************************************************************/

void ajDasServerDel(AjPDasServer *thys)
{
    AjPDasServer pthis = NULL;
    AjPDasSource source;
    AjPDasCoordinate coord;
    AjIList iter;
    AjIList iterc;

    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    ajStrDel(&pthis->host);
    ajStrDel(&pthis->path);

    iter = ajListIterNew(pthis->sources);

    while(!ajListIterDone(iter))
    {
	source = ajListIterGet(iter);
        ajStrDel(&source->uri);
        ajStrDel(&source->title);
        ajStrDel(&source->description);
        ajStrDel(&source->sequence_query_uri);
        ajStrDel(&source->entry_points_uri);
        ajStrDel(&source->features_query_uri);

        iterc = ajListIterNew(source->coordinates);

        while(!ajListIterDone(iterc))
        {
            coord = ajListIterGet(iterc);
            ajStrDel(&coord->authority);
            ajStrDel(&coord->source);
            ajStrDel(&coord->taxid);
            ajStrDel(&coord->test_range);
            ajStrDel(&coord->uri);
            ajStrDel(&coord->version);
            AJFREE(coord);
        }

        ajListFree(&source->coordinates);
        AJFREE(source);
        ajListIterDel(&iterc);
    }

    ajListIterDel(&iter);
    ajListFree(&pthis->sources);

    AJFREE(pthis);
    
    *thys = NULL;
    
    return;
}




/* @func ajDasSegmentDel *****************************************************
**
** Delete DAS segment objects returned by entry_points queries
**
** @param [u] Psegment [AjPDasSegment*] DAS segment object
** @return [void]
******************************************************************************/

void ajDasSegmentDel(AjPDasSegment* Psegment)
{
    if(Psegment==NULL || *Psegment==NULL)
        return;

    ajStrDel(&(*Psegment)->id);
    ajStrDel(&(*Psegment)->type);
    ajStrDel(&(*Psegment)->orientation);

    AJFREE(*Psegment);

    return;
}




/* @funcstatic dasRegistryElementstart ****************************************
**
** Handler for reading DAS registry XML elements
**
** @param [u] data [void*] structure for storing results
** @param [r] name [const XML_Char*] XML element name
** @param [r] atts [const XML_Char**] array of element attributes,
**                                    name/value pairs
** @return [void]
******************************************************************************/

static void dasRegistryElementstart(void *data, const XML_Char *name,
                                    const XML_Char **atts)
{
    AjPList sources       = NULL;
    AjPDasSource source   = NULL;
    const char* key       = NULL;
    const char* value     = NULL;
    const char* type      = NULL;
    const char* query_uri = NULL;

    ajuint i;

    sources = (AjPList) data;

    ajDebug("dasRegistryElementstart: %s\n",name);

    if(ajCharMatchCaseC("SOURCES",name))
	return;

    if(ajCharMatchCaseC(name,"SOURCE"))
    {
	AJNEW0(source);
	source->coordinates = ajListNew();

	for(i = 0; atts[i]; i += 2)
	{
	    key   = atts[i];
	    value = atts[i+1];

	    if(ajCharMatchC(key,"uri"))
		ajStrAssignC(&source->uri,value);
	    else if(ajCharMatchC(key,"title"))
	    {
		ajStrAssignC(&source->title,value);
		ajStrTrimWhite(&source->title);
	    }
	    else if(ajCharMatchC(key,"description"))
		ajStrAssignC(&source->description,value);
	}

	ajListPushAppend(sources,source);
    }
    else if(ajCharMatchCaseC(name,"CAPABILITY"))
    {
	ajListPeekLast(sources,(void**)&source);

	for(i = 0; atts[i]; i += 2)
	{
	    key   = atts[i];
	    value = atts[i+1];

	    if(ajCharMatchC(key,"type"))
		type = value;
	    else
		query_uri = value;
	}

	if(ajCharMatchCaseC(type,"das1:features"))
	{
	    source->features = ajTrue;
	    ajStrAssignC(&source->features_query_uri,query_uri);
	}
	else if(ajCharMatchCaseC(type,"das1:sequence"))
	{
	    source->sequence = ajTrue;
	    ajStrAssignC(&source->sequence_query_uri,query_uri);
	}
	else if(ajCharMatchCaseC(type,"das1:entry_points"))
	{
	    source->entry_points = ajTrue;
	    ajStrAssignC(&source->entry_points_uri,query_uri);
	}
    }
    else if(ajCharMatchCaseC(name,"COORDINATES"))
    {
	AjPDasCoordinate coord;
	AJNEW0(coord);
	ajListPeekLast(sources,(void**)&source);

	for(i = 0; atts[i]; i += 2)
	{
	    key   = atts[i];
	    value = atts[i+1];

	    if(ajCharMatchCaseC(key,"uri"))
		ajStrAssignC(&coord->uri, value);
	    else if(ajCharMatchCaseC(key,"taxid"))
		ajStrAssignC(&coord->taxid, value);
	    else if(ajCharMatchCaseC(key,"source"))
		ajStrAssignC(&coord->source,value);
	    else if(ajCharMatchCaseC(key,"test_range"))
		ajStrAssignC(&coord->test_range,value);
	}

	ajListPushAppend(source->coordinates,coord);
    }
    else if(!(ajCharMatchC(name,"MAINTAINER")   ||
	    ajCharMatchC(name,"VERSION")       ||
	    ajCharMatchC(name,"PROP")
    ))
	ajDebug("DAS 'sources' request returned an unknown tag: %s",name);

    return;
}




/* @funcstatic dasEntrypointsElementstart *************************************
**
** Handler for reading DAS entry-point XML elements
**
** @param [u] data [void*] structure for storing results
** @param [r] name [const XML_Char*] XML element name
** @param [r] atts [const XML_Char**] array of name/value pairs
** @return [void]
******************************************************************************/

static void dasEntrypointsElementstart(void *data, const XML_Char *name,
                                     const XML_Char **atts)
{
    const char* key   = NULL;
    AjPStr value = NULL;

    AjPList segments = data;
    AjPDasSegment segment;

    ajuint i;


    ajDebug("dasEntrypointsElementstart: %s\n",name);

    if(ajCharMatchCaseC("DASEP",name) || ajCharMatchCaseC("ENTRY_POINTS",name))
    {
        return;
    }


    if(!(ajCharMatchC(name,"SEGMENT")))
        ajDebug("DAS 'entrypoints' request returned an unknown tag: %s",name);

    {

	AJNEW0(segment);

	for(i = 0; atts[i]; i += 2)
	{
	    key   = atts[i];
	    value = ajStrNewC(atts[i+1]);

	    if(ajCharMatchC(key,"id"))
		ajStrAssignS(&segment->id,value);
	    else if(ajCharMatchC(key,"start"))
	    {
		ajStrToUint(value, &segment->start);
	    }
	    else if(ajCharMatchC(key,"stop"))
	    {
		ajStrToUint(value, &segment->stop);
	    }
	    else if(ajCharMatchC(key,"subparts"))
	    {
		ajStrToBool(value, &segment->subparts);
	    }
	    else if(ajCharMatchC(key,"orientation"))
	    {
		ajStrAssignS(&segment->orientation,value);
	    }
	    else if(ajCharMatchC(key,"type"))
	    {
		ajStrAssignS(&segment->type,value);
	    }

	    ajStrDel(&value);
	}

	ajListPushAppend(segments,segment);
    }


    return;
}




/* @func ajDasParseRegistry ***************************************************
**
** Parses XML list of DAS sources, either coming from dasregistry.org
** or from individual DAS servers supporting DAS 'sources' command
**
** @param [u] buff [AjPFilebuff] XML input stream
** @param [u] sources [AjPList] pointer to the data structure for storing
**                              parser results of DAS/XML 'source' objects
** @return [AjBool] True on success
******************************************************************************/

AjBool ajDasParseRegistry(AjPFilebuff buff, AjPList sources)
{

    return dasQueryParse(buff, sources, dasRegistryElementstart, NULL);
}




/* @func ajDasParseEntrypoints ************************************************
**
** Parses XML list of DAS entry-points, either coming from dasregistry.org
** or from individual DAS servers supporting DAS 'entry_points' command
**
** @param [u] buff [AjPFilebuff] XML input stream
** @param [u] segments [AjPList] pointer to the data structure for storing
**                               parser results of DAS/XML 'segment' objects
** @return [AjBool] True on success
******************************************************************************/

AjBool ajDasParseEntrypoints(AjPFilebuff buff, AjPList segments)
{

    return dasQueryParse(buff, segments, dasEntrypointsElementstart, NULL);
}




/* @func ajDasServerSethostS **************************************************
**
** Set the DAS server host name
**
** @param [u] server [AjPDasServer]  DASserver object
** @param [r] host [const AjPStr] DAS server host name
** @return [AjBool] True on success
******************************************************************************/

AjBool ajDasServerSethostS(AjPDasServer server, const AjPStr host)
{
    ajStrAssignS(&server->host, host);

    return ajTrue;
}




/* @func ajDasServerSetpathS **************************************************
**
** Set the DAS server URL path
**
** @param [u] server [AjPDasServer]  DASserver object
** @param [r] path [const AjPStr] DAS server URL path
** @return [AjBool] True on success
******************************************************************************/

AjBool ajDasServerSetpathS(AjPDasServer server, const AjPStr path)
{
    ajStrAssignS(&server->path, path);

    return ajTrue;
}




/* @func ajDasServerSetport ***************************************************
**
** Set the DAS server port
**
** @param [u] server [AjPDasServer]  DASserver object
** @param [r] port [ajuint] DAS server port
** @return [AjBool] True on success
******************************************************************************/

AjBool ajDasServerSetport(AjPDasServer server, ajuint port)
{
    server->port = port;

    return ajTrue;
}




/* @func ajDasServerGetSources ************************************************
**
** Makes an http request for getting either list of all DAS sources
** or a particular DAS source on a given DAS server. Http response is parsed
** and results are stored in server object's 'sources' attribute
**
** @param [u] server [AjPDasServer] DAS server object
** @param [r] cmd [const AjPStr] DAS 'sources' command, queries either
**                         all DAS sources or a particular DAS source
** @return [void]
******************************************************************************/

void ajDasServerGetSources(AjPDasServer server, const AjPStr cmd)
{
    AjPStr cmdpath   = ajStrNew();
    AjPStr httpver   = ajStrNew();
    AjPStr dbname    = ajStrNew();
    AjPStr dbproxy   = ajStrNew();
    AjPTextin textin = ajTextinNewDatatype(AJDATATYPE_TEXT);

    if(ajStrGetCharLast(server->path)!='/')
	ajFmtPrintS(&cmdpath, "%S/%S", server->path, cmd);
    else
	ajFmtPrintS(&cmdpath, "%S%S", server->path, cmd);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpRead(httpver, dbname, dbproxy,
                                  server->host, server->port, cmdpath);

    if(textin->Filebuff)
    {
	ajFilebuffHtmlNoheader(textin->Filebuff);
	ajDasParseRegistry(textin->Filebuff, server->sources);
    }

    ajTextinDel(&textin);
    ajStrDel(&cmdpath);
    ajStrDel(&httpver);
    ajStrDel(&dbname);
    ajStrDel(&dbproxy);
}




/* @func ajDasGetSequenceQueryURI *********************************************
**
** Given a DAS server URL, returns sequence query URI for the DAS source
** specified in user query.
**
** It first makes a DAS source command/query for the specific DAS source.
** If above fails then makes another query for all DAS sources
** and checks them against the current DAS source.
**
** @param [r] qry [const AjPQuery] sequence query object
** @param [r] sourceURIorTitle [const AjPStr] DAS source URI or title
**                                      specified in current query
** @return [AjPStr] sequence-query URI, null if not found
******************************************************************************/

AjPStr ajDasGetSequenceQueryURI(const AjPQuery qry,
				  const AjPStr sourceURIorTitle)
{
    AjPDasServer server = NULL;
    AjPDasSource source = NULL;
    AjPStr host  = NULL;
    AjPStr path  = NULL;
    AjPStr cmd   = NULL;
    AjIList iter = NULL;

    AjPStr ret = NULL;
    ajint port = 80;
    AjBool validresponse = ajTrue;

    ajDebug("ajDasGetSequenceQueryURI: searching query URI for '%S'",
	    sourceURIorTitle);

    if(!ajHttpQueryUrl(qry, &port, &host, &path))
    {
	ajStrDel(&host);
	ajStrDel(&path);
	return ajFalse;
    }

    server = ajDasServerNew();

    ajDasServerSethostS(server,host);
    ajDasServerSetport(server,port);
    ajDasServerSetpathS(server,path);

    /* first try direct source command */

    ajFmtPrintS(&cmd, "sources/%S", sourceURIorTitle);
    ajDasServerGetSources(server, cmd);

    if (ajListGetLength(server->sources)==1)
    {
	ajListPeekFirst(server->sources, (void**)&source);

	if(source == NULL || ajListGetLength(source->coordinates)==0)
	{
	    ajDebug("DAS data source query '%S' didn't return a valid result",
	           cmd);
	    validresponse = ajFalse;
	}
	else if(source->sequence == ajFalse)
	{
	    ajErr("DAS data source (%S,%S) doesn't support sequence queries",
	           source->uri, source->title);
	    ajStrDel(&host);
	    ajStrDel(&path);
	    ajDasServerDel(&server);
	    ajStrDel(&cmd);

	    return NULL;
	}

	if(validresponse && ajStrMatchS(sourceURIorTitle, source->uri))
	{
	    if(source->sequence_query_uri!=NULL)
	    {
		ret = ajStrNewRef(source->sequence_query_uri);
		ajDebug("dasGetSequenceQueryURI: DAS source found "
			"by direct query '%S'",	ret);
	    }
	    else
	    {
		//TODO: here we can try building the query_uri unless host
		// is dasregistry.....
		ajFmtPrintS(&ret, "%S",host);
		ajWarn("server source description didn't have a query_uri...");
	    }
	}
    }

    if(ret==NULL)
    {
	ajDebug("dasGetSequenceQueryURI: trying a full 'sources' query");
	ajStrAssignC(&cmd, "sources");
	ajDasServerGetSources(server, cmd);

	iter = ajListIterNew(server->sources);

	while(!ajListIterDone(iter))
	{
	    source = ajListIterGet(iter);

	    if((ajStrMatchS(sourceURIorTitle, source->title) ||
		    ajStrMatchS(sourceURIorTitle, source->uri)))
	    {
		if(source->sequence == ajFalse)
		{
		    ajErr("DAS data source (%S,%S) doesn't support "
			    "sequence queries", source->uri, source->title);
		    ajStrDel(&host);
		    ajStrDel(&path);
		    ajDasServerDel(&server);
		    ajStrDel(&cmd);
		    ajListIterDel(&iter);

		    return NULL;
		}

		ret = ajStrNewRef(source->sequence_query_uri);
		ajDebug("ajDasGetSequenceQueryURI: URI found '%S'", ret);
		break;
	    }
	}

	ajListIterDel(&iter);
    }

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&cmd);

    ajDasServerDel(&server);

    return ret;
}




/* @funcstatic dasQueryParse **************************************************
**
** Parses DAS query responses
**
** @param [u] buff [AjPFilebuff] buffer object holding DAS query response
** @param [u] results [void*] object for storing parser output
** @param [u] starth [XML_StartElementHandler] handler for processing
**                                             XML element attributes
** @param [u] elementh [XML_CharacterDataHandler] handler for processing
**                                                XML element data
** @return [AjBool] True on success
******************************************************************************/

static AjBool dasQueryParse(AjPFilebuff buff, void* results,
			    XML_StartElementHandler starth,
			    XML_CharacterDataHandler elementh)
{
    XML_Parser parser = NULL;
    AjPStr line       = NULL;

    int done;
    size_t len;

    if(!buff)
	return ajFalse;

    line = ajStrNew();
    parser = XML_ParserCreate(NULL);

    XML_SetElementHandler(parser, starth, NULL);

    if(elementh!=NULL)
	XML_SetCharacterDataHandler(parser, elementh);

    XML_SetUserData(parser, results);

    done = 0;

    do
    {
	ajBuffreadLine(buff,&line);
	done = ajFilebuffIsEmpty(buff);
	len = ajStrGetLen(line);

	ajDebug("ajDasQueryParse: line: %S", line);

	if(!XML_Parse(parser, line->Ptr, len, done))
	{
	    ajDebug("ajDasQueryParse: %s at line %d\n",
	            XML_ErrorString(XML_GetErrorCode(parser)),
	            XML_GetCurrentLineNumber(parser));

	    ajErr("Failed to parse received DAS response: %s",
		    XML_ErrorString(XML_GetErrorCode(parser)));

	    XML_ParserFree(parser);
	    ajStrDel(&line);

	    return ajFalse;
	}

    } while (!done);

    XML_ParserFree(parser);
    ajStrDel(&line);

    return ajTrue;
}




/* @func ajDasSourceGetDBname ************************************************
**
** Returns an EMBOSS DB name for a given DAS source
**
** @param [r] source [const AjPDasSource] DAS source object
** @param [r] titleAndURI [AjBool] Include URI in name
** @return [AjPStr] DB name
** @@
******************************************************************************/

AjPStr ajDasSourceGetDBname(const AjPDasSource source, AjBool titleAndURI)
{
    AjPStr dbname = NULL;

    if (titleAndURI)
    {
	dbname = ajStrNewS(source->uri);

	ajStrAppendK(&dbname, '_');

	ajStrAppendS(&dbname, source->title);
    }
    else
	dbname = ajStrNewS(source->title);

    ajStrExchangeSetCC(&dbname, " \":+-()./", "_________");

    ajStrTrimC(&dbname, "_");

    ajStrTrimStartC(&dbname, "0123456789");

    return dbname;
}




/* @func ajDasPrintCachefile *************************************************
**
** Prints DB definition for the specified DAS server object
**
** @param [r] server [const AjPDasServer] DAS server object
** @param [w] cachef [AjPFile] server cachefile with DB definitions
** @return [void]
** @@
******************************************************************************/

void ajDasPrintCachefile(const AjPDasServer server, AjPFile cachef)
{
    AjPTime today = NULL;
    AjIList iter  = NULL;
    AjPStr fname  = NULL;

    AjPTable titlecount = NULL;
    AjPDasSource source = NULL;

    AjPStr title = NULL;

    ajuint* count = NULL;

    titlecount = ajTablestrNewCaseConst(ajListGetLength(server->sources)+20);

    fname  = ajStrNewS(ajFileGetPrintnameS(cachef));

    ajFilenameTrimPath(&fname);

    today =  ajTimeNewTodayFmt("cachefile");

    ajFmtPrintF(cachef,"# %S %D\n\n", fname, today);

    ajStrDel(&fname);
    ajTimeDel(&today);

    iter = ajListIterNew(server->sources);

    while(!ajListIterDone(iter))
    {
	source = ajListIterGet(iter);

	title = source->title;

	count = ajTableFetchmodS(titlecount,title);

	if (count != NULL)
        {
	    (*count)++;
        }
	else
	{
	    AJNEW(count);
	    *count = 1;
            ajTablePut(titlecount, title, (void*)count);
	}

    }

    ajListIterRewind(iter);


    while(!ajListIterDone(iter))
    {
	source = ajListIterGet(iter);

	title = source->title;

	count = ajTableFetchmodS(titlecount,title);

	dasWriteDBdefinition(cachef, source, *count>1);
    }

    ajListIterDel(&iter);
    ajTableDelValdel(&titlecount, ajMemFree);

    return;
}




/* @funcstatic dasWriteDBdefinition *******************************************
**
** Writes a DAS source DB definition to the specified cache-file
**
** @param [u] cachef [AjPFile] cache file
** @param [r] source [const AjPDasSource] DAS source object
** @param [r] titleAndURI [AjBool] Include URI in name
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool dasWriteDBdefinition(AjPFile cachef, const AjPDasSource source,
				   AjBool titleAndURI)
{
    AjPStr entry = NULL;
    ajint ibegin = 0;
    ajint iend   = 0;
    ajlong i     = 0;

    AjPDasCoordinate coord = NULL;
    AjIList coordsi = NULL;

    AjPStr dbname  = NULL;
    AjPStr comment = NULL;
    AjPStr type    = NULL;
    AjPStr format  = NULL;
    AjPStr url     = NULL;
    AjPStr example = NULL;
    AjPStr taxon   = NULL;
    AjPStr fields  = NULL;

    if(!source->sequence && !source->features)
	return ajFalse;

    dbname = ajDasSourceGetDBname(source, titleAndURI);
    comment = ajStrNewS(source->description);

    ajStrExchangeKK(&comment, '"', '\'');
    ajStrRemoveWhiteExcess(&comment);

    coordsi = ajListIterNew(source->coordinates);

    type = ajStrNew();
    format = ajStrNew();
    url = ajStrNew();
    taxon = ajStrNew();

    while(!ajListIterDone(coordsi))
    {
	coord = ajListIterGet(coordsi);

	example = ajDasTestrangeParse(coord->test_range, &entry,
	                              &ibegin, &iend);

	if (source->sequence)
	{
	    if(ajStrFindCaseC(coord->source, "nucleotide")!=-1 ||
		    ajStrFindCaseC(coord->source, "chromosome")!=-1 ||
		    ajStrFindCaseC(coord->source, "contig")!=-1 ||
		    ajStrFindCaseC(coord->source, "scaffold")!=-1)
		ajStrAssignC(&type, "Nucleotide");
	    else if(ajStrFindCaseC(coord->source, "Protein")!=-1)
		ajStrAssignC(&type,"Protein");
	    else
		ajStrAssignC(&type,"Sequence");

	    ajStrAssignC(&format, "das");

	    ajStrAssignS(&url, source->sequence_query_uri);

	    i = ajStrFindlastC(url,"/sequence");

	    if(i != -1)
	      ajStrCutEnd(&url,ajStrGetLen(url) - (size_t) i);

	}

	if (source->features)
	{
	    if(source->sequence)
	    {
		ajStrAppendC(&type,", ");
		ajStrAppendC(&format,", ");
	    }

	    if(ajStrFindCaseC(coord->source, "nucleotide")!=-1 ||
		    ajStrFindCaseC(coord->source, "chromosome")!=-1 ||
		    ajStrFindCaseC(coord->source, "contig")!=-1 ||
		    ajStrFindCaseC(coord->source, "scaffold")!=-1)
		ajStrAppendC(&type, "Nucfeatures");
	    else if(ajStrFindCaseC(coord->source, "Protein")!=-1)
		ajStrAppendC(&type, "Protfeatures");
	    else
		ajStrAppendC(&type,"Features");

	    ajStrAppendC(&format, "dasgff");

	    if(ajStrGetLen(url) == 0)
	    {
		ajStrAssignS(&url, source->features_query_uri);

		i = ajStrFindlastC(url,"/features");

		if(i != -1)
		  ajStrCutEnd(&url,ajStrGetLen(url)- (size_t) i);

	    }

	    fields = ajStrNewC("segment,type,category,categorize,feature_id");
	}
	else
	    fields = ajStrNewC("segment");

	if (coord->taxid)
	    ajFmtPrintS(&taxon, "  taxon: \"%S\"\n", coord->taxid);

	ajFmtPrintF(cachef,
		"DB %S [\n"
		"  method: das\n"
		"  type: \"%S\"\n"
		"%S"
		"  format: \"%S\"\n"
		"  url: %S\n"
		"  example: \"%S\"\n"
		"  comment: \"%S\"\n"
		"  hasaccession: \"N\"\n"
		"  identifier: \"segment\"\n"
		"  fields: \"%S\"\n"
		"]\n\n",
		dbname, type,
		taxon,
		format, url,
		example,
		comment,
		fields);

	break; /* TODO: multiple coordinates */
    }

    ajStrDel(&dbname);
    ajStrDel(&comment);
    ajStrDel(&type);
    ajStrDel(&format);
    ajStrDel(&url);
    ajStrDel(&url);
    ajStrDel(&example);
    ajStrDel(&taxon);
    ajStrDel(&fields);

    ajStrDel(&entry);

    ajListIterDel(&coordsi);

    return ajTrue;
}




/* @func ajDasTestrangeParse *************************************************
**
** Parses a DAS test_range attribute and returns sequence identifier, and
** 'begin' and 'end' positions if any.
**
** @param [r] testrange [const AjPStr] DAS test_range string
** @param [w] id [AjPStr*] sequence identifier
** @param [w] ibegin [ajint*] sequence begin position
** @param [w] iend [ajint*] sequence end position
** @return [AjPStr] returns the example query in EMBOSS USA syntax
** @@
******************************************************************************/

AjPStr ajDasTestrangeParse(const AjPStr testrange, AjPStr* id,
                           ajint* ibegin, ajint* iend)
{
    AjPStr tmp=NULL;
    AjPStrTok token;
    AjPStr ret = NULL;

    *ibegin = 0;
    *iend = 0;

    /* Parse the test_range string (identifier:begin,end) */

    token = ajStrTokenNewC(testrange, ":,");

    ret = ajStrNew();

    if(!(ajStrTokenNextParseNoskip(&token, id)))
    {
	ajStrTokenDel(&token);
	return ret;
    }

    if(ajStrTokenNextParseNoskip(&token, &tmp))
    {
	ajStrToInt(tmp, ibegin);

	if(ajStrTokenNextParseNoskip(&token, &tmp))
	{
	    ajStrToInt(tmp, iend);
	    ajFmtPrintS(&ret,"%S[%d:%d]", *id, *ibegin, *iend);
	}
    }

    if(*iend == 0)
	ajStrAssignRef(&ret, *id);

    ajStrTokenDel(&token);
    ajStrDel(&tmp);

    return ret;
}

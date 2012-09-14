/* @source dastest application
 **
 ** Return list of DAS sources from DAS registry or from specified DAS servers
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
#include "ajdas.h"
#include "ajfeatread.h"




static AjPTable     dastestGetTitleCount(const AjPDasServer server);

static AjPFeattable dastestFeatureQuery(const AjPStr usa,
                                        ajint ibegin, ajint iend);

static void         dastestSaveMappedFeatures(const AjPFeattable fttable,
                                              const AjPStr entry,
                                              AjPFile outf,
                                              ajint maxfeatures);

static void         dastestSaveRawFeatures(AjPFilebuff buff,
                                           const AjPStr ffname);




/* @prog dastest **************************************************************
 **
 ** Temporary application to explore list of sources on DAS servers.
 ** Can be used for testing DAS access method as well using -runtestqueries
 ** option.
 **
 *****************************************************************************/

int main(int argc, char **argv)
{
    AjPDasServer server   = NULL;
    AjPDasSource source   = NULL;
    AjPDasSegment segment = NULL;

    AjPStr host  = NULL;
    AjPStr path  = NULL;
    AjPFile outf = NULL;

    ajint port = 80;

    AjBool sequencesourcesonly;
    AjBool entrypoints;
    AjBool showtestqueries;
    AjBool runtestqueries = ajTrue;
    AjBool quickexit = ajFalse;

    ajint itest=0;
    ajint j=0;

    ajint maxtests=0;
    ajint maxfeatures=0;
    ajint maxsegments=0;

    AjIList iter     = NULL;
    AjIList coordsi  = NULL;
    AjIList itereps  = NULL;
    AjPFilebuff buff = NULL;
    AjPUrlref uo        = NULL;
    AjPList segments = NULL;
    AjPStr ffname    = NULL;
    AjPStr url       = NULL;

    AjPStr dbhttpver = ajStrNew();
    AjPStr dbname    = ajStrNew();
    AjPStr dbproxy   = ajStrNew();

    AjPStr servername   = NULL;
    AjPTable titlecount = NULL;
    const ajuint* count;
    int k=0;

    embInit("dastest", argc, argv);

    host = ajAcdGetString("host");
    path = ajAcdGetString("path");
    port = ajAcdGetInt("port");
    sequencesourcesonly = ajAcdGetBoolean("sequencesourcesonly");
    entrypoints = ajAcdGetBoolean("entrypoints");
    showtestqueries = ajAcdGetBoolean("showtestqueries");
    runtestqueries  = ajAcdGetBoolean("runtestqueries");

    servername = ajAcdGetString("servername");

    outf   = ajAcdGetOutfile("outfile");

    maxtests     = ajAcdGetInt("maxtests");
    maxfeatures = ajAcdGetInt("maxfeatures");
    maxsegments = ajAcdGetInt("maxsegments");

    server = ajDasServerNew();

    if(runtestqueries)
    {
	url = ajStrNew();

	if(!ajNamServer(servername))
	{
	    ajWarn("following das server is required to be defined "
		    "for test queries...");
	    ajWarn("\nSERVER %S [\n"
		    "   type: \"sequence\"\n"
		    "   method: \"das\"\n"
		    "   url: \"http://%S%S\"\n"
		    "]\n",servername, host,path);
	    ajWarn("ignoring -runtestqueries option...");
	    runtestqueries = ajFalse;
	}
	else
	{
	    ajNamSvrGetUrl(servername, &url);
	    ajHttpUrlDeconstruct(url, &port, &host, &path);

	    ajStrDel(&url);
	}
    }

    ajDasServerSethostS(server,host);
    ajDasServerSetport(server,port);


    if(ajStrGetCharLast(path)!='/')
	ajStrAppendK(&path,'/');

    ajStrAppendC(&path,"sources");

    ajDasServerSetpathS(server,path);

    ajFmtPrintF(outf,"host = %S\npath = %S\nport = %d\n",
	    server->host, server->path, server->port);


    /*
     * TODO: stop using http-read but instead use
     *       ajNamSvrListListDatabases(svrname, dbnames);
     */

    buff = ajHttpRead(dbhttpver, dbname, dbproxy, host, port, path);

    if(!buff)
	ajExitAbort();

    ajFilebuffHtmlNoheader(buff);

    ajDasParseRegistry(buff, server->sources);
    ajFmtPrintF(outf,"DAS sources and descriptions\n\n");

    titlecount = dastestGetTitleCount(server);

    iter = ajListIterNew(server->sources);

    while(!ajListIterDone(iter) && !quickexit)
    {
	source = ajListIterGet(iter);


	if ((sequencesourcesonly && !source->sequence)
		|| ajStrMatchC(source->title,"cath") || k++ <50)
	    continue;

	ajFmtPrintF(outf,"%-30S %-50S\n%S\n",source->uri,source->title,
		source->description);

	if(entrypoints && source->entry_points)
	{
	    uo = ajHttpUrlrefNew();

	    ajHttpUrlrefParseC(&uo, ajStrGetPtr(source->entry_points_uri));

	    if(ajStrGetLen(uo->Port))
		ajStrToInt(uo->Port, &port);
	    else
		port = 80;

	    ajFilebuffDel(&buff);
	    buff = ajHttpRead(dbhttpver, dbname, dbproxy,
	                      uo->Host, port, uo->Absolute);
	    ajHttpUrlrefDel(&uo);

	    if(!buff)
		continue;

	    ajFilebuffHtmlNoheader(buff);

	    segments = ajListNew();
	    ajDasParseEntrypoints(buff, segments);

	    itereps = ajListIterNew(segments);

	    ajFmtPrintF(outf, "Number of entry points %Lu\n",
		    ajListGetLength(segments));

	    j=0;

	    while(!ajListIterDone(itereps))
	    {
		segment = ajListIterGet(itereps);

		if (j++ < maxsegments)
		    ajFmtPrintF(outf,
		            "segment id:%S orientation:%S start:%d stop:%d\n",
		            segment->id,
		            segment->orientation,
		            segment->start, segment->stop);

		ajDasSegmentDel(&segment);
	    }

	    ajListIterDel(&itereps);
	    ajListFree(&segments);
	}

	if(showtestqueries || runtestqueries)
	{
	    AjPDasCoordinate coord;

	    coordsi = ajListIterNew(source->coordinates);

	    while(!ajListIterDone(coordsi) && !quickexit)
	    {
		coord = ajListIterGet(coordsi);
		ajDebug("coordinate uri:%S taxid:%S source:%S test_range:%S\n",
			coord->uri,
			coord->taxid,
			coord->source,
			coord->test_range);

		if(showtestqueries)
		{
		    if(source->sequence)
			ajFmtPrintF(outf,
			       "example/test entry = '%S?segment=%S'\n",
			       source->sequence_query_uri,coord->test_range);

		    if(source->features)
			ajFmtPrintF(outf,
			       "example/test entry = '%S?segment=%S'\n",
			       source->features_query_uri,coord->test_range);

		}

		if(runtestqueries)
		{
		    AjPStr idqry = ajStrNew();
		    AjPStr entry = NULL;
		    AjPSeq seq   = NULL;
		    ajint ibegin = 0;
		    ajint iend   = 0;
		    AjPStr example = NULL;

		    example = ajDasTestrangeParse(coord->test_range,
		                                  &entry, &ibegin, &iend);

		    if(ajStrGetLen(entry))
		    {
			count = ajTableFetchS(titlecount, source->title);
			dbname = ajDasSourceGetDBname(source, *count>1);

			if (source->features)
			{
			    AjPStr qpath=NULL;

			    uo = ajHttpUrlrefNew();

			    ajFmtPrintS(&idqry,"dasgff::%S:%S:%S",
				    servername,
				    dbname,
				    entry);
			    ajFmtPrintF(outf,
				    "feature query: %S  start:%d end:%d\n",
				    idqry,
				    ibegin, iend);


			    ajHttpUrlrefParseC(&uo,
				    ajStrGetPtr(source->features_query_uri));
			    ajHttpUrlrefSplitPort(uo);

			    ajFmtPrintS(&qpath,"%S?segment=%S",
				    uo->Absolute,entry);

			    if(iend>0)
				ajFmtPrintAppS(&qpath,":%d,%d",ibegin, iend);

			    if(ajStrGetLen(uo->Port))
				ajStrToInt(uo->Port, &port);
			    else
				port = 80;

			    ajDebug("calling ajHttpRead to get the raw"
				    " output; host:%S port:%d path:%S\n",
				    uo->Host, port, qpath);

			    ajFilebuffDel(&buff);
			    buff = ajHttpRead(dbhttpver, dbname, dbproxy,
			                      uo->Host, port, qpath);

			    if(buff)
			    {
				AjPFeattable ft;

				ajFmtPrintS(&ffname, "%S.%S", source->uri,
					    entry);

				ajFilebuffHtmlNoheader(buff);

				dastestSaveRawFeatures(buff, ffname);

				ajDebug("now using EMBOSS feature queries\n");

				ft = dastestFeatureQuery(idqry,
                                                         ibegin, iend);

				dastestSaveMappedFeatures(ft, ffname,
                                                          outf, maxfeatures);

				ajStrDel(&ffname);
				ajFeattableDel(&ft);
			    }

			    ajHttpUrlrefDel(&uo);
			    ajStrDel(&qpath);

			    if(++itest>=maxtests)
				quickexit = ajTrue;
			}
			else if(source->sequence)
			{
			    seq = ajSeqNewRes(iend-ibegin+1);

			    ajFmtPrintS(&idqry,"%S:%S:%S",
				    servername, dbname, entry);
			    ajFmtPrintF(outf,
				    "sequence query: %S  start:%d end:%d\n",
				    idqry,
				    ibegin, iend);
			    ajSeqGetFromUsaRange(idqry, ajFalse, ibegin, iend,
				    seq);
			    ajFmtPrintF(outf,
				    "length of sequence returned: %d\n",
				    ajSeqGetLen(seq));

			    if(ajSeqGetLen(seq)>0)
				ajFmtPrintF(outf,
				        "sequence returned (first 100 bases):"
					" %-100.100s\n",
					ajSeqGetSeqC(seq));

			    ajSeqDel(&seq);

			}

			ajStrDel(&dbname);
		    }

		    ajStrDel(&entry);
		    ajStrDel(&idqry);
		    ajStrDel(&example);
		}
	    }
	    ajListIterDel(&coordsi);
	}

    }

    ajListIterDel(&iter);

    ajDasServerDel(&server);
    ajFilebuffDel(&buff);

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&servername);

    ajStrDel(&dbhttpver);
    ajStrDel(&dbname);
    ajStrDel(&dbproxy);

    ajFileClose(&outf);

    ajTableDelValdel(&titlecount, &ajMemFree);

    embExit();

    return 0;
}




/* @funcstatic dastestFeatureQuery ********************************************
**
** experimental function to make feature queries
**
** @param [r] ufo [const AjPStr] USA
** @param [r] ibegin [ajint] sequence start position
** @param [r] iend [ajint] sequence end position
** @return [AjPFeattable] feature table
** @@
******************************************************************************/

static AjPFeattable dastestFeatureQuery(const AjPStr ufo,
                                        ajint ibegin, ajint iend)
{
    AjPFeattabin ftquery;
    AjPFeattable fttable;
    AjPStr fmt;

    fttable =  ajFeattableNew(NULL);

    fmt = ajStrNewC("dasgff");

    ftquery = ajFeattabinNewSS(fmt,ufo,"any");


    ftquery->Start = ibegin;
    ftquery->End = iend;

    ajStrAssignS(&ftquery->Input->Qry, ufo);

    ftquery->Input->Query->DataType = AJDATATYPE_FEATURES;

    ajFilebuffDel(&ftquery->Input->Filebuff);

    ajFeattabinRead(ftquery, fttable);

    ajStrDel(&fmt);

    ajFeattabinClear(ftquery);
    ajFeattabinDel(&ftquery);

    return fttable;
}




/* @funcstatic dastestSaveRawFeatures *****************************************
**
** Reads feature information from buffer
** and saves in a new file in current directory
**
** @param [u] buff [AjPFilebuff] Input buffer
** @param [r] ffname [const AjPStr] file name
** @return [void]
** @@
******************************************************************************/

static void dastestSaveRawFeatures(AjPFilebuff buff, const AjPStr ffname)
{
    AjPFile ffile = NULL;
    AjPStr line   = ajStrNew();
    AjPStr ffnamee = ajStrNew();

    ajFmtPrintS(&ffnamee, "%S.raw.dasgff", ffname);
    ffile = ajFileNewOutNameS(ffnamee);

    while(!ajFilebuffIsEmpty(buff))
    {
	ajBuffreadLine(buff,&line);

	ajDebug("ajDasQueryParse: line: %S", line);
	ajWriteline(ffile, line);
	ajStrDel(&line);
    }

    ajFileClose(&ffile);
    ajStrDel(&ffnamee);

    return;
}




/* @funcstatic dastestSaveMappedFeatures **************************************
**
** Prints summary feature information
** and saves features using dasgff format
**
** @param [r] fttable [const AjPFeattable] feature table
** @param [r] ffname [const AjPStr] file name
** @param [u] outf [AjPFile] Output file object
** @param [r] maxfeatures [ajint] Maximum number of features to save
** @return [void]
** @@
******************************************************************************/

static void dastestSaveMappedFeatures(const AjPFeattable fttable,
                                      const AjPStr ffname,
                                      AjPFile outf, ajint maxfeatures)
{
    AjPFeature feature  = NULL;
    AjIList iterfts     = NULL;
    AjPFeattabOut ftout = NULL;
    AjPFile ffile = NULL;
    AjPStr ffnamee = ajStrNew();

    ajint i=0;

    if (fttable == NULL)
    {
	ajWarn("null feature table, %S", ffname);
	return;
    }

    ajFmtPrintS(&ffnamee, "%S.mapped.dasgff", ffname);
    ffile = ajFileNewOutNameS(ffnamee);

    iterfts = ajListIterNew(fttable->Features);
    ftout= ajFeattabOutNewCSF("gff3",NULL,"",ffile);

    ajFmtPrintF(outf, "Number of features %Lu\n",
	    ajListGetLength(fttable->Features));

    while(!ajListIterDone(iterfts) && i++ < maxfeatures)
    {
	feature = ajListIterGet(iterfts);
	ajFmtPrintF(outf,
		"feature id:%S orientation:%c start:%d stop:%d\n",
		feature->Label,
		feature->Strand,
		feature->Start, feature->End);
    }

    ajListIterDel(&iterfts);

    ajFeattableWriteDasgff(ftout, fttable);

    ajFeattabOutDel(&ftout);
    ajFileClose(&ffile);
    ajStrDel(&ffnamee);

    return;
}




/* @funcstatic dastestGetTitleCount *******************************************
**
** Returns record of in how many different sources titles are used.
**
** Note: if a title is used more than once 'source' 'uri' attributes
** are also used to get DB names to make sure they are unique.
**
** @param [r] server [const AjPDasServer] DAS server obj
** @return [AjPTable] table of usage count for each title
** @@
******************************************************************************/

static AjPTable dastestGetTitleCount(const AjPDasServer server)
{
    AjIList iter  = NULL;

    AjPTable titlecount = NULL;
    AjPDasSource source = NULL;

    AjPStr title = NULL;

    ajuint* count = NULL;

    titlecount = ajTablestrNewCaseConst(ajListGetLength(server->sources)+20);


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

    ajListIterDel(&iter);

    return titlecount;
}

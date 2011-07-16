/* @source dbxresource application
**
** Index data resources from the data resource catalogue
**
** @author Copyright (C) Peter Rice (pmr@ebi.ac.uk)
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



static AjPRegexp dbxresource_wrdexp = NULL;

static AjBool dbxresource_NextEntry(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxresource_ParseDrcat(EmbPBtreeEntry entry, const AjPStr line);

EmbPBtreeField accfield = NULL;
EmbPBtreeField namfield = NULL;
EmbPBtreeField desfield = NULL;
EmbPBtreeField urlfield = NULL;
EmbPBtreeField catfield = NULL;
EmbPBtreeField taxidfield = NULL;
EmbPBtreeField edatfield = NULL;
EmbPBtreeField eidfield = NULL;
EmbPBtreeField efmtfield = NULL;
EmbPBtreeField etpcfield = NULL;
EmbPBtreeField xreffield = NULL;
EmbPBtreeField restfield = NULL;
EmbPBtreeField soapfield = NULL;

EmbPBtreeField qoutfield = NULL;
EmbPBtreeField qfmtfield = NULL;
EmbPBtreeField qinfield = NULL;
EmbPBtreeField qurlfield = NULL;




/* @prog dbxresource **********************************************************
**
** Index a flat file data resource catalogue database
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPBtreeEntry entry = NULL;
    
    AjPStr dbname   = NULL;
    AjPStr dbrs     = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;
    AjBool compressed;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr dbtype = NULL;
    AjPFile outf = NULL;

    AjPStr *fieldarray = NULL;
    
    ajint nfields;
    ajint nfiles;

    AjPStr tmpstr = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile inf = NULL;

    AjPBtId  idobj  = NULL;
    AjPBtPri priobj = NULL;
    AjPBtHybrid hyb = NULL;
    
    ajulong nentries = 0L;
    ajulong ientries = 0L;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;

    embInit("dbxresource", argc, argv);

    fieldarray = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    outf       = ajAcdGetOutfile("outfile");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    dbrs       = ajAcdGetString("dbresource");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    compressed = ajAcdGetBoolean("compressed");

    entry = embBtreeEntryNew();
    if(compressed)
        embBtreeEntrySetCompressed(entry);

    tmpstr = ajStrNew();
    
    idobj   = ajBtreeIdNew();
    priobj  = ajBtreePriNew();
    hyb     = ajBtreeHybNew();
    
    dbtype = ajStrNewC("drcat");

    nfields = embBtreeSetFields(entry,fieldarray);
    embBtreeSetDbInfo(entry,dbname,dbrs,datestr,release,dbtype,directory,
		      indexdir);

    for(i=0; i< nfields; i++)
    {
        if(ajStrMatchC(fieldarray[i], "acc"))
        {
            accfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(accfield);
        }
        else if(ajStrMatchC(fieldarray[i], "nam"))
        {
            namfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(namfield);
        }
        else if(ajStrMatchC(fieldarray[i], "des"))
        {
            desfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(desfield);
        }
        else if(ajStrMatchC(fieldarray[i], "url"))
        {
            urlfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(urlfield);
        }
        else if(ajStrMatchC(fieldarray[i], "cat"))
        {
            catfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(catfield);
        }
        else if(ajStrMatchC(fieldarray[i], "taxid"))
        {
            taxidfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(taxidfield);
        }
        else if(ajStrMatchC(fieldarray[i], "edat"))
        {
            edatfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(edatfield);
        }
        else if(ajStrMatchC(fieldarray[i], "efmt"))
        {
            efmtfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(efmtfield);
        }
        else if(ajStrMatchC(fieldarray[i], "eid"))
        {
            eidfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(eidfield);
        }
        else if(ajStrMatchC(fieldarray[i], "etpc"))
        {
            etpcfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(etpcfield);
        }
        else if(ajStrMatchC(fieldarray[i], "xref"))
        {
            xreffield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(xreffield);
        }
        else if(ajStrMatchC(fieldarray[i], "qout"))
        {
            qoutfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(qoutfield);
        }
        else if(ajStrMatchC(fieldarray[i], "qfmt"))
        {
            qfmtfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(qfmtfield);
        }
        else if(ajStrMatchC(fieldarray[i], "qin"))
        {
            qinfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(qinfield);
        }
        else if(ajStrMatchC(fieldarray[i], "qurl"))
        {
            qurlfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(qurlfield);
        }
         else if(ajStrMatchC(fieldarray[i], "rest"))
        {
            restfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(restfield);
        }
         else if(ajStrMatchC(fieldarray[i], "soap"))
        {
            soapfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(soapfield);
        }
        else if(!ajStrMatchC(fieldarray[i], "id"))
            ajErr("Unknown field '%S' specified for indexing", fieldarray[i]);
    }

    embBtreeGetRsInfo(entry);

    nfiles = embBtreeGetFiles(entry,directory,filename,exclude);
    if(!nfiles)
        ajDie("No input files in '%S' matched filename '%S'",
              directory, filename);

    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);

    starttime = ajTimeNewToday();

    ajFmtPrintF(outf, "Processing directory: %S\n", directory);

    for(i=0;i<nfiles;++i)
    {
        begintime = ajTimeNewToday();

	ajListstrPop(entry->files,&thysfile);
	ajListstrPushAppend(entry->files,thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(inf=ajFileNewInNameS(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	
	ajFilenameTrimPath(&tmpstr);
	ajFmtPrintF(outf,"Processing file: %S",tmpstr);

	ientries = 0L;

	while(dbxresource_NextEntry(entry, inf))
	{
	    ++ientries;
	    if(entry->do_id)
                embBtreeIndexEntry(entry, i);

	    if(accfield)
                embBtreeIndexField(accfield, entry, i);

	    if(namfield)
                embBtreeIndexField(namfield, entry, i);

	    if(desfield)
                embBtreeIndexField(desfield, entry, i);

	    if(urlfield)
                embBtreeIndexField(urlfield, entry, i);

	    if(catfield)
                embBtreeIndexField(catfield, entry, i);

	    if(taxidfield)
                embBtreeIndexField(taxidfield, entry, i);

	    if(edatfield)
                embBtreeIndexField(edatfield, entry, i);

	    if(efmtfield)
                embBtreeIndexField(efmtfield, entry, i);

	    if(eidfield)
                embBtreeIndexField(eidfield, entry, i);

	    if(etpcfield)
                embBtreeIndexField(etpcfield, entry, i);

	    if(xreffield)
                embBtreeIndexField(xreffield, entry, i);

	    if(qoutfield)
                embBtreeIndexField(qoutfield, entry, i);

	    if(qfmtfield)
                embBtreeIndexField(qfmtfield, entry, i);

	    if(qinfield)
                embBtreeIndexField(qinfield, entry, i);

	    if(qurlfield)
                embBtreeIndexField(qurlfield, entry, i);

	    if(restfield)
                embBtreeIndexField(restfield, entry, i);

	    if(soapfield)
                embBtreeIndexField(soapfield, entry, i);
	}
	
	ajFileClose(&inf);
	nentries += ientries;
	nowtime = ajTimeNewToday();
	ajFmtPrintF(outf, " entries: %Lu (%Lu) time: %.1fs (%.1fs)\n",
		    nentries, ientries,
		    ajTimeDiff(starttime, nowtime),
		    ajTimeDiff(begintime, nowtime));
	ajTimeDel(&begintime);
	ajTimeDel(&nowtime);
    }
    

    embBtreeDumpParameters(entry);
    embBtreeCloseCaches(entry);
    
    nowtime = ajTimeNewToday();
    ajFmtPrintF(outf, "Total time: %.1fs\n", ajTimeDiff(starttime, nowtime));
    ajTimeDel(&nowtime);
    ajTimeDel(&starttime);

    embBtreeReportEntry(outf, entry);

    if(accfield)
        embBtreeReportField(outf, accfield);
    if(namfield)
        embBtreeReportField(outf, namfield);
    if(desfield)
        embBtreeReportField(outf, desfield);
    if(urlfield)
        embBtreeReportField(outf, urlfield);
    if(catfield)
        embBtreeReportField(outf, catfield);
    if(taxidfield)
        embBtreeReportField(outf, taxidfield);
    if(edatfield)
        embBtreeReportField(outf, edatfield);
    if(etpcfield)
        embBtreeReportField(outf, etpcfield);
    if(eidfield)
        embBtreeReportField(outf, eidfield);
    if(efmtfield)
        embBtreeReportField(outf, efmtfield);
    if(qoutfield)
        embBtreeReportField(outf, qoutfield);
    if(qfmtfield)
        embBtreeReportField(outf, qfmtfield);
    if(qinfield)
        embBtreeReportField(outf, qinfield);
    if(qurlfield)
        embBtreeReportField(outf, qurlfield);
    if(xreffield)
        embBtreeReportField(outf, xreffield);
    if(restfield)
        embBtreeReportField(outf, restfield);
    if(soapfield)
        embBtreeReportField(outf, soapfield);

    ajFileClose(&outf);
    embBtreeEntryDel(&entry);
    ajStrDel(&tmpstr);
    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&dbname);
    ajStrDel(&dbrs);
    ajStrDel(&release);
    ajStrDel(&datestr);
    ajStrDel(&directory);
    ajStrDel(&indexdir);
    ajStrDel(&dbtype);
    

    nfields = 0;
    while(fieldarray[nfields])
	ajStrDel(&fieldarray[nfields++]);
    AJFREE(fieldarray);


    ajBtreeIdDel(&idobj);
    ajBtreePriDel(&priobj);
    ajBtreeHybDel(&hyb);

    ajRegFree(&dbxresource_wrdexp);

    embExit();

    return 0;
}




/* @funcstatic dbxresource_NextEntry ******************************************
**
** Parse the next entry from a data resource catalogue file
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxresource_NextEntry(EmbPBtreeEntry entry, AjPFile inf)
{
    static AjBool init = AJFALSE;
    static AjPStr line = NULL;
    static ajlong fpos = 0L;

    if(!init)
    {
        line = ajStrNew();
        init = ajTrue;
    }

    while(!ajStrPrefixC(line, "ID      "))
    {
        fpos = ajFileResetPos(inf);
	if(!ajReadlineTrim(inf,&line))
	{
	  ajStrDel(&line);
	  return ajFalse;
	}
    }

    dbxresource_ParseDrcat(entry, line);
    if(!ajReadlineTrim(inf,&line))
        return ajFalse;

    entry->fpos = fpos;

    while(!ajStrPrefixC(line, "ID      "))
    {
        fpos = ajFileResetPos(inf);
        dbxresource_ParseDrcat(entry, line);
	if(!ajReadlineTrim(inf,&line))
            break;
    }
    
    return ajTrue;
}




/* @funcstatic dbxresource_ParseDrcat *****************************************
**
** Parse the ID, name, synonyms and other details from a line read from
** a data resource catalogue file
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [r] line [const AjPStr] Input line
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxresource_ParseDrcat(EmbPBtreeEntry entry, const AjPStr line)
{
    AjPStr tmpfd = NULL;
    AjPStr name = NULL;
    AjPStr rest = NULL;
    AjPStr token = NULL;

    static AjPStr saveline = NULL;
    AjPStrTok handle = NULL;
    AjPStrTok subhandle = NULL;

    if(!dbxresource_wrdexp)
	dbxresource_wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    ajStrExtractFirst(line, &rest, &name);

    if(ajStrMatchC(name, "ID"))
    {
        if(entry->do_id && !ajStrMatchC(rest, "None"))
        {
            ajStrRemoveWhiteExcess(&rest);
            ajStrExchangeKK(&rest, '/', '_');
            ajStrExchangeKK(&rest, '-', '_');
            ajStrExchangeKK(&rest, '(', '_');
            ajStrExchangeKK(&rest, ')', '_');
            ajStrAssignS(&entry->id, rest);
        }
    }
    else if(ajStrMatchC(name, "IDalt"))
    {
        if(accfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, "|");
            while(ajStrTokenNextParse(&handle, &token))
            {
                ajStrRemoveWhiteExcess(&token);
                ajStrExchangeKK(&token, '/', '_');
                ajStrExchangeKK(&token, '-', '_');
                ajStrExchangeKK(&token, '(', '_');
                ajStrExchangeKK(&token, ')', '_');
                ajListstrPush(accfield->data, ajStrNewS(token));
            }
        }
    }
    else if(ajStrMatchC(name, "Name"))
    {
        if(namfield && !ajStrMatchC(rest, "None"))
            embBtreeParseField(rest, dbxresource_wrdexp, namfield);
    }
    else if(desfield && ajStrMatchC(name, "Desc"))
    {
        if(desfield && !ajStrMatchC(rest, "None"))
            embBtreeParseField(rest, dbxresource_wrdexp, desfield);
    }
    else if(ajStrMatchC(name, "URL"))
    {
        if(urlfield && !ajStrMatchC(rest, "None"))
            embBtreeParseField(rest, dbxresource_wrdexp, urlfield);
    }
    else if(ajStrSuffixC(name, "Cat"))
    {
        if(catfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, ";");
            while(ajStrTokenNextParse(&handle, &token))
            {
                ajStrRemoveWhiteExcess(&token);
                ajListstrPush(catfield->data,ajStrNewS(token));
            }
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrSuffixC(name, "Taxon"))
    {
        ajDebug("%S\n",  line);
        if(taxidfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, "|");
            ajStrTokenNextParse(&handle, &token);
            if(taxidfield)
            {
                ajStrRemoveWhiteExcess(&token);
                ajListstrPush(taxidfield->data,ajStrNewS(token));
            }
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrMatchC(name, "EDAMdat"))
    {
        if(edatfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, " \t|");
            ajStrTokenNextParse(&handle, &token);
            ajListstrPush(edatfield->data,ajStrNewS(token));
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrMatchC(name, "EDAMfmt"))
    {
        if(efmtfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, " \t|");
            ajStrTokenNextParse(&handle, &token);
            ajListstrPush(efmtfield->data,ajStrNewS(token));
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrMatchC(name, "EDAMid"))
    {
        if(eidfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, " \t|");
            ajStrTokenNextParse(&handle, &token);
            ajListstrPush(eidfield->data,ajStrNewS(token));
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrMatchC(name, "EDAMtpc"))
    {
        if(etpcfield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, " \t|");
            ajStrTokenNextParse(&handle, &token);
            ajListstrPush(etpcfield->data,ajStrNewS(token));
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrMatchC(name, "Xref"))
    {
        if(xreffield && !ajStrMatchC(rest, "None"))
        {
            handle = ajStrTokenNewC(rest, " \t|");
            ajStrTokenNextParse(&handle, &token);
            ajListstrPush(xreffield->data,ajStrNewS(token));
            ajStrTokenDel(&handle);
        }
    }
    else if(ajStrMatchC(name, "Query") && !ajStrMatchC(rest, "None"))
    {
        /* index qout qfmt qin qurl */
        if(ajStrParseCountC(rest, "|") >= 4)
        {
            handle = ajStrTokenNewC(rest, "|");
            ajStrTokenNextParse(&handle, &token);
            ajStrRemoveWhiteExcess(&token);

            if(qoutfield && !ajStrMatchC(token, "Unknown"))
                ajListstrPush(qoutfield->data,ajStrNewS(token));

            ajStrTokenNextParse(&handle, &token);
            ajStrRemoveWhiteExcess(&token);

            if(qfmtfield &&
               !ajStrMatchC(token, "Unknown") &&
               !ajStrMatchC(token, "???"))
                ajListstrPush(qfmtfield->data,ajStrNewS(token));

            ajStrTokenNextParse(&handle, &token);
            ajStrRemoveWhiteExcess(&token);

            if(qinfield &&
               !ajStrMatchC(token, "Unknown") &&
               !ajStrMatchC(token, "???"))
            {
                subhandle = ajStrTokenNewC(token, ";");
                while(ajStrTokenNextParse(&subhandle, &tmpfd))
                {
                    ajStrRemoveWhiteExcess(&tmpfd);
                    ajListstrPush(qinfield->data,ajStrNewS(tmpfd));
                }
                ajStrTokenDel(&subhandle);
            }

            ajStrTokenRestParse(&handle, &token);

            if(qurlfield &&
               !ajStrMatchC(token, "Unknown") &&
               !ajStrMatchC(token, "None"))
                embBtreeParseField(rest, dbxresource_wrdexp, qurlfield);

            ajStrTokenDel(&handle);
        }
        else
        {
            ajErr("Bad query line '%S'", line);
        }
    }
    else if(ajStrPrefixC(name, "URLrest"))
    {
        if(restfield && !ajStrMatchC(rest, "None"))
            embBtreeParseField(rest, dbxresource_wrdexp, restfield);
    }
    else if(ajStrPrefixC(name, "URLsoap"))
    {
        if(soapfield && !ajStrMatchC(rest, "None"))
            embBtreeParseField(rest, dbxresource_wrdexp, soapfield);
    }

    ajStrDel(&saveline);
    ajStrDel(&rest);
    ajStrDel(&tmpfd);
    ajStrDel(&token);
    ajStrDel(&name);

    return ajTrue;
}


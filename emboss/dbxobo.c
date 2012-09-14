/* @source dbxobo application
**
** Index OBO format ontologies
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



static AjPRegexp dbxobo_wrdexp = NULL;
static AjPRegexp dbxobo_synonymexp = NULL;


static AjBool dbxobo_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
                               const AjPStr dbprefix);
static AjBool dbxobo_ParseObo(EmbPBtreeEntry entry, const AjPStr line,
                              const AjPStr dbprefix);

EmbPBtreeField accfield = NULL;
EmbPBtreeField namfield = NULL;
EmbPBtreeField isafield = NULL;
EmbPBtreeField nsfield = NULL;
EmbPBtreeField desfield = NULL;




/* @prog dbxobo **************************************************************
**
** Index a flat file database
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPBtreeEntry entry = NULL;
    
    AjPStr dbname   = NULL;
    AjPStr dbprefix   = NULL;
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

    ajulong nentries = 0L;
    ajulong ientries = 0L;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;

    embInit("dbxobo", argc, argv);

    fieldarray = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    outf       = ajAcdGetOutfile("outfile");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    dbprefix   = ajAcdGetString("standardname");
    dbrs       = ajAcdGetString("dbresource");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    compressed = ajAcdGetBoolean("compressed");

    ajStrFmtUpper(&dbprefix);

    entry = embBtreeEntryNew(0);
    if(compressed)
        embBtreeEntrySetCompressed(entry);
    tmpstr = ajStrNew();
    
    dbtype = ajStrNewC("obo");

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
        else if(ajStrMatchC(fieldarray[i], "isa"))
        {
            isafield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(isafield);
        }
        else if(ajStrMatchC(fieldarray[i], "ns"))
        {
            nsfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(nsfield);
        }
        else if(ajStrMatchC(fieldarray[i], "des"))
        {
            desfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(desfield);
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

	ajListPop(entry->files,(void **)&thysfile);
	ajListstrPushAppend(entry->files,thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(inf=ajFileNewInNameS(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	
	ajFilenameTrimPath(&tmpstr);
	ajFmtPrintF(outf,"Processing file: %S\n",tmpstr);

	ientries = 0L;

	while(dbxobo_NextEntry(entry, inf, dbprefix))
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

	    if(isafield)
                embBtreeIndexField(isafield, entry, i);

	    if(nsfield)
                embBtreeIndexField(nsfield, entry, i);
	}
	
	ajFileClose(&inf);
	nentries += ientries;
	nowtime = ajTimeNewToday();
	ajFmtPrintF(outf, "entries: %Lu (%Lu) time: %.1fs (%.1fs)\n",
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
    
    if(isafield)
        embBtreeReportField(outf, isafield);
    
    if(nsfield)
        embBtreeReportField(outf, nsfield);
    
    if(desfield)
        embBtreeReportField(outf, desfield);
    
    ajFileClose(&outf);
    embBtreeEntryDel(&entry);
    ajStrDel(&tmpstr);
    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&dbname);
    ajStrDel(&dbprefix);
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

    ajRegFree(&dbxobo_wrdexp);
    ajRegFree(&dbxobo_synonymexp);

    embExit();

    return 0;
}




/* @funcstatic dbxobo_NextEntry ***********************************************
**
** Parse the next entry from an OBO file
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
** @param [r] dbprefix [const AjPStr] Database prefix
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxobo_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
                               const AjPStr dbprefix)
{
    static AjBool init = AJFALSE;
    static AjPStr line = NULL;
    static ajlong fpos = 0L;

    if(!init)
    {
        line = ajStrNew();
        init = ajTrue;
    }

    while(!ajStrPrefixC(line, "[Term]"))
    {
        fpos = ajFileResetPos(inf);
	if(!ajReadlineTrim(inf,&line))
	{
	  ajStrDel(&line);
	  return ajFalse;
	}
    }

    if(!ajReadlineTrim(inf,&line))
        return ajFalse;

    entry->fpos = fpos;

    while(!ajStrPrefixC(line, "["))
    {
        fpos = ajFileResetPos(inf);
        dbxobo_ParseObo(entry, line, dbprefix);
	if(!ajReadlineTrim(inf,&line))
            break;
    }
    
    return ajTrue;
}




/* @funcstatic dbxobo_ParseObo ************************************************
**
** Parse the ID, name, synonyms and other details from a line read from
** an OBO format ontology
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [r] line [const AjPStr] Input line
** @param [r] dbprefix [const AjPStr] Database prefix
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxobo_ParseObo(EmbPBtreeEntry entry, const AjPStr line,
                              const AjPStr dbprefix)
{
    AjPStr tmpfd = NULL;
    AjPStr name = NULL;
    AjPStr rest = NULL;
    AjPStr token = NULL;
    ajlong ipos;

    static AjPStr saveline = NULL;

    if(!dbxobo_synonymexp)
        dbxobo_synonymexp = ajRegCompC("[\"]([^\"]+)[\"] +([A-Z]+)");

    if(!dbxobo_wrdexp)
	dbxobo_wrdexp = ajRegCompC("([A-Za-z0-9]+)");

    if(ajStrGetLen(saveline))
    {
        ajStrCutEnd(&saveline, 1);
        ajStrAppendS(&saveline, line);
    }
    else
        saveline = ajStrNewS(line);

    if((ajStrGetCharLast(saveline) == '\\') &&
       (ajStrGetCharPos(saveline, -2) != '\\'))
        return ajTrue;          /* to be continued */

    ajObolineCutComment(&saveline, &rest);
    if(!ajStrGetLen(saveline))
    {
        ajStrDel(&rest);
        ajStrDel(&saveline);
        return ajTrue;
    }

    ajObolineEscape(&saveline);
    ajObolineCutModifier(&saveline, &rest);

    /* parse name: value ! comment */
    ajStrExtractWord(saveline, &rest, &name);
    if(!ajStrGetLen(rest))
    {
        ajWarn( "missing value for tag '%S' in entry '%S'", name, entry->id);
        ajStrDel(&name);
        ajStrDel(&rest);
        ajStrDel(&saveline);
        return ajTrue;
    }

    if(ajStrMatchC(name, "id:"))
    {
        if(ajStrPrefixS(rest, dbprefix))
        {
            ipos = ajStrFindAnyK(rest, ':');
            if(ipos > 0)
                ajStrCutStart(&rest, (size_t)(ipos+1));
        }
        ajStrAssignS(&entry->id, rest);
    }
    else if(ajStrMatchC(name, "alt_id:"))
    {
        if(accfield)
        {
            if(ajStrPrefixS(rest, dbprefix))
            {
                ipos = ajStrFindAnyK(rest, ':');
                if(ipos > 0)
                    ajStrCutStart(&rest, (size_t)(ipos+1));
            }
            ajListstrPush(accfield->data,ajStrNewS(rest));
        }
    }
    else if(ajStrMatchC(name, "name:"))
    {
        if(accfield)
            ajListstrPush(accfield->data,ajStrNewS(rest));

        if(namfield)
            embBtreeParseField(rest, dbxobo_wrdexp, namfield);
    }
    else if(ajStrMatchC(name, "namespace:"))
    {
        if(nsfield)
            ajListstrPush(nsfield->data,ajStrNewS(rest));
    }
    else if(ajStrMatchC(name, "def:"))
    {
        if(desfield)
            embBtreeParseField(rest, dbxobo_wrdexp, desfield);
    }
    else if(ajStrSuffixC(name, "synonym:"))
    {
        if(accfield)
        {
            if(ajRegExec(dbxobo_synonymexp, rest))
            {
                ajRegSubI(dbxobo_synonymexp, 1, &token);
                ajRegSubI(dbxobo_synonymexp, 2, &tmpfd);
                if(ajStrMatchC(token, "EXACT"))
                    ajListstrPush(accfield->data,ajStrNewS(token));
            }
        }
        if(namfield)
            embBtreeParseField(rest, dbxobo_wrdexp, namfield);
    }
    else if(ajStrMatchC(name, "is_a:"))
    {
        if(isafield)
        {
            if(ajStrPrefixS(rest, dbprefix))
            {
                ipos = ajStrFindAnyK(rest, ':');
                if(ipos > 0)
                    ajStrCutStart(&rest, (size_t)(ipos+1));
            }
            ajListstrPush(isafield->data,ajStrNewS(rest));
        }
    }

    ajStrDel(&saveline);
    ajStrDel(&rest);
    ajStrDel(&tmpfd);
    ajStrDel(&token);
    ajStrDel(&name);

    return ajTrue;
}

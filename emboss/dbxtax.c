/* @source dbxtax application
**
** Index NCBI's taxonomy database
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

static AjPRegexp dbxtax_wrdexp = NULL;

static AjPStr nameline = NULL;

static AjPTable mergeTable = NULL;

static ajlong namePos = 0L;

static AjBool dbxtax_NextEntry(EmbPBtreeEntry entry, AjPFile infnode,
                               AjPFile infname);
static AjBool dbxtax_ParseName(EmbPBtreeEntry entry, const AjPStr line);
static AjBool dbxtax_ParseNode(EmbPBtreeEntry entry, const AjPStr line);
static void   dbxtax_ParseMerged(AjPFile mergefile);

EmbPBtreeField accfield = NULL;
EmbPBtreeField taxfield = NULL;
EmbPBtreeField rnkfield = NULL;
EmbPBtreeField upfield = NULL;
EmbPBtreeField gcfield = NULL;
EmbPBtreeField mgcfield = NULL;




/* @prog dbxtax ************************************************************
**
** Index the NCBI taxonomy database
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPBtreeEntry entry = NULL;
    
    AjPStr dbname   = NULL;
    AjPStr dbrs     = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename = NULL;
    AjPStr exclude = NULL;
    AjPStr dbtype = NULL;
    AjPFile outf = NULL;

    AjBool compressed = ajTrue;

    AjPFile mergefile = NULL;
    AjPStr *fieldarray = NULL;
    
    ajint nfields;
    ajint nfiles;

    AjPStr tmpstr = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile infnode = NULL;
    AjPFile infname = NULL;

    ajulong nentries = 0L;
    ajulong ientries = 0L;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;

    embInit("dbxtax", argc, argv);

    fieldarray = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    outf       = ajAcdGetOutfile("outfile");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    dbname     = ajAcdGetString("dbname");
    dbrs       = ajAcdGetString("dbresource");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    compressed = ajAcdGetBoolean("compressed");

    entry = embBtreeEntryNew(1);
    tmpstr = ajStrNew();
    
    dbtype = ajStrNewC("taxonomy");

    nfields = embBtreeSetFields(entry,fieldarray);
    embBtreeSetDbInfo(entry,dbname,dbrs,datestr,release,dbtype,directory,
		      indexdir);
    entry->compressed = compressed;

    for(i=0; i< nfields; i++)
    {
        if(ajStrMatchC(fieldarray[i], "acc"))
        {
            accfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(accfield);
        }
        else if(ajStrMatchC(fieldarray[i], "up"))
        {
            upfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(upfield);
            embBtreeFieldSetIdtype(upfield);
        }
        else if(ajStrMatchC(fieldarray[i], "tax"))
        {
            taxfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(taxfield);
            embBtreeFieldSetIdtype(taxfield);
        }
        else if(ajStrMatchC(fieldarray[i], "rnk"))
        {
            rnkfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(rnkfield);
        }
        else if(ajStrMatchC(fieldarray[i], "gc"))
        {
            gcfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(gcfield);
        }
        else if(ajStrMatchC(fieldarray[i], "mgc"))
        {
            mgcfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(mgcfield);
        }
        else if(!ajStrMatchC(fieldarray[i], "id"))
            ajErr("Unknown field '%S' specified for indexing", fieldarray[i]);
    }

    embBtreeGetRsInfo(entry);

    ajStrAssignC(&exclude, "");
    ajStrAssignC(&filename, "nodes.dmp");
    nfiles = embBtreeGetFiles(entry,directory,filename,exclude);
    if(!nfiles)
        ajDie("No input files in '%S' matched filename '%S'",
              directory, filename);

    ajStrAssignC(&filename, "names.dmp");
    ajListPushAppend(entry->reffiles[0],(void *)filename);
    filename = NULL;

    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);

    starttime = ajTimeNewToday();

    ajFmtPrintF(outf, "Processing directory: %S\n", directory);

    ajFmtPrintS(&tmpstr,"%S%s",entry->directory,"merged.dmp");
    mergefile = ajFileNewInNameS(tmpstr);

    dbxtax_ParseMerged(mergefile);
    ajFileClose(&mergefile);

    for(i=0;i<nfiles;++i)
    {
        begintime = ajTimeNewToday();

	ajListPop(entry->reffiles[0],(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(infname=ajFileNewInNameS(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	
	ajListPop(entry->files,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(infnode=ajFileNewInNameS(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);

	ajFilenameTrimPath(&tmpstr);
	ajFmtPrintF(outf,"Processing file: %S\n",tmpstr);

	ientries = 0L;

	while(dbxtax_NextEntry(entry, infnode, infname))
	{
	    ++ientries;

	    if(entry->do_id)
                embBtreeIndexEntry(entry, i);

            if(accfield)
                embBtreeIndexField(accfield, entry, i);

	    if(taxfield)
                embBtreeIndexField(taxfield, entry, i);

	    if(rnkfield)
                embBtreeIndexField(rnkfield, entry, i);

	    if(upfield)
                embBtreeIndexField(upfield, entry, i);

	    if(gcfield)
                embBtreeIndexField(gcfield, entry, i);

	    if(mgcfield)
                embBtreeIndexField(mgcfield, entry, i);
	}
	
	ajFileClose(&infnode);
	ajFileClose(&infname);
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
    if(taxfield)
        embBtreeReportField(outf, taxfield);
    if(rnkfield)
        embBtreeReportField(outf, rnkfield);
    if(upfield)
        embBtreeReportField(outf, upfield);
    if(gcfield)
        embBtreeReportField(outf, gcfield);
    if(mgcfield)
        embBtreeReportField(outf, mgcfield);

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

    ajRegFree(&dbxtax_wrdexp);
    ajStrDel(&nameline);

    ajTablestrFree(&mergeTable);

    embExit();

    return 0;
}




/* @funcstatic dbxtax_NextEntry ********************************************
**
** Parse the next entry from nodes and names files
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] infnode [AjPFile] nodes file object ptr
** @param [u] infname [AjPFile] names file object ptr
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxtax_NextEntry(EmbPBtreeEntry entry, AjPFile infnode,
                               AjPFile infname)
{
    AjPStr line = NULL;
    AjBool ok = ajTrue;
    
    ajStrAssignC(&line,"");

    entry->fpos = ajFileResetPos(infnode);

    if(!ajReadlineTrim(infnode,&line))
    {
        ajStrDel(&line);
        return ajFalse;
    }

    dbxtax_ParseNode(entry, line);

    entry->reffpos[0] = namePos;

    if(!nameline)
        ok = ajReadlineTrim(infname,&nameline);

    while(ok && dbxtax_ParseName(entry, nameline))
    {
        namePos = ajFileResetPos(infname);
        ok = ajReadlineTrim(infname,&nameline);
    }

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic dbxtax_ParseNode ************************************************
**
** Parse the node record
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [r] line [const AjPStr] nodes.dmp record
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxtax_ParseNode(EmbPBtreeEntry entry,
                               const AjPStr line)
{
    AjPStr tmpstr  = NULL;
    AjPStr tmpfd  = NULL;

    AjPStrTok handle = NULL;
    const AjPStr oldids = NULL;

    if(!dbxtax_wrdexp)
	dbxtax_wrdexp = ajRegCompC("([A-Za-z0-9]+)");

    handle = ajStrTokenNewC(line, "|");

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* taxid */
        return ajFalse;
    ajStrTrimWhite(&tmpstr);

    ajStrAssignS(&entry->id, tmpstr);
    oldids = ajTableFetchS(mergeTable, tmpstr);
    if(oldids && accfield)
        embBtreeParseField(oldids, dbxtax_wrdexp, accfield);
    
    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* parent taxid */
        return ajFalse;
    ajStrTrimWhite(&tmpstr);
    if(upfield && ajStrGetLen(tmpstr))
	ajListPush(upfield->data,ajStrNewS(tmpstr));

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* rank */
        return ajFalse;

    ajStrTrimWhite(&tmpstr);

    ajStrTrimWhite(&tmpstr);
    if(rnkfield && ajStrGetLen(tmpstr))
	ajListPush(rnkfield->data,ajStrNewS(tmpstr));

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* embl code */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* division */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* division flag */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* gencode */
        return ajFalse;
    ajStrTrimWhite(&tmpstr);
    if(gcfield && ajStrGetLen(tmpstr))
	ajListPush(gcfield->data,ajStrNewS(tmpstr));

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* gencode flag */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* mitocode */
        return ajFalse;
    ajStrTrimWhite(&tmpstr);
    if(mgcfield && ajStrGetLen(tmpstr) && !ajStrMatchC(tmpstr, "0"))
	ajListPush(mgcfield->data,ajStrNewS(tmpstr));

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* mitocode flag */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* genbank hidden flag */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* nosequence flag */
        return ajFalse;

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* comments */
        return ajFalse;

    ajStrDel(&tmpstr);
    ajStrDel(&tmpfd);
    ajStrTokenDel(&handle);

    return ajTrue;
}




/* @funcstatic dbxtax_ParseName ************************************************
**
** Parse the next name record until the name does not match the entry
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [r] line [const AjPStr] names.dmp record
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxtax_ParseName(EmbPBtreeEntry entry, const AjPStr line)
{
    AjPStr tmpstr = NULL;
    AjPStr tmpname = NULL;

    AjPStrTok handle = NULL;
    ajlong pos;

    if(!dbxtax_wrdexp)
	dbxtax_wrdexp = ajRegCompC("([A-Za-z0-9]+)");

    handle = ajStrTokenNewC(line, "|");

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* taxid */
        return ajFalse;
    ajStrTrimWhite(&tmpstr);
    if(!ajStrMatchS(entry->id, tmpstr))
    {
        ajStrTokenDel(&handle);
        ajStrDel(&tmpstr);
        return ajFalse;
    }

    if(!ajStrTokenNextParse(&handle, &tmpname)) /* name */
    {
        ajStrTokenDel(&handle);
        ajStrDel(&tmpstr);
        ajStrDel(&tmpname);
        return ajFalse;
    }

    ajStrTrimWhite(&tmpname);

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* uniquename */
    {
        ajStrTokenDel(&handle);
        ajStrDel(&tmpstr);
        ajStrDel(&tmpname);
        return ajFalse;
    }

    ajStrTrimWhite(&tmpstr);
/*    if(ajStrGetLen(tmpstr))
      ajStrAssignS(&tmpname, tmpstr);*/

    if(!ajStrTokenNextParse(&handle, &tmpstr)) /* nameclass */
    {
        ajStrTokenDel(&handle);
        ajStrDel(&tmpstr);
        ajStrDel(&tmpname);
        return ajFalse;
    }

    ajStrTrimWhite(&tmpstr);

    if(taxfield &&
       ajStrGetLen(tmpname))
    {
        pos = ajStrFindAnyK(tmpname, '<');
        if(pos > 0)
            ajStrKeepRange(&tmpname, 0, pos-1);

        ajStrRemoveWhiteExcess(&tmpname);
        if(ajStrMatchC(tmpstr, "scientific name"))
            ajListPush(taxfield->data,ajStrNewS(tmpname));
        if(ajStrSuffixC(tmpstr, "common name"))
            ajListPush(taxfield->data,ajStrNewS(tmpname));
        if(ajStrMatchC(tmpstr, "synonym"))
        {
            pos = ajStrFindAnyK(tmpname, '(');
            if(pos > 0)
                ajStrKeepRange(&tmpname, 0, pos-1);
            ajListPush(taxfield->data,ajStrNewS(tmpname));
        }
    }

    ajStrDel(&tmpname);
    ajStrDel(&tmpstr);
    ajStrTokenDel(&handle);

    return ajTrue;
}




/* @funcstatic dbxtax_ParseMerged *********************************************
**
** Parse the merged taxid file
**
** @param [u] mergefile [AjPFile] Input file (merged.dmp)
**
** @return [void]
** @@
******************************************************************************/

static void dbxtax_ParseMerged(AjPFile mergefile)
{
    AjPStr line = NULL;
    AjPStr oldid = NULL;
    AjPStr newid = NULL;
    AjPStr record = NULL;
    AjPStrTok handle = NULL;

    mergeTable = ajTablestrNew(20000);

    while(ajReadlineTrim(mergefile, &line))
    {
        ajStrTokenAssignC(&handle, line, "\t|");
        ajStrTokenNextParse(&handle, &oldid);
        ajStrTokenNextParse(&handle, &newid);
        record = ajTableFetchmodS(mergeTable, newid);
        if(!record)
        {
            ajTablePut(mergeTable, ajStrNewS(newid), ajStrNewS(oldid));
        }
        else 
        {
            ajStrAppendK(&record, ' ');
            ajStrAppendS(&record, oldid);
        }
    }

    ajStrTokenDel(&handle);
    ajStrDel(&newid);
    ajStrDel(&oldid);
    ajStrDel(&line);

    return;
}





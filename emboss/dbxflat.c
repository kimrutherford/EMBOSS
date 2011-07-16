/* @source dbxflat application
**
** Index fasta format sequence files
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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


static AjPRegexp dbxflat_wrdexp = NULL;


static ajuint maxidlen = 0;
static ajuint idtrunc  = 0;

static AjBool dbxflat_ParseFastq(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseEmbl(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseGenbank(EmbPBtreeEntry entry, AjPFile inf);

static AjBool dbxflat_NextEntry(EmbPBtreeEntry entry, AjPFile inf);

int global = 0;

EmbPBtreeField accfield = NULL;
EmbPBtreeField svfield = NULL;
EmbPBtreeField orgfield = NULL;
EmbPBtreeField desfield = NULL;
EmbPBtreeField keyfield = NULL;

ajuint idtot = 0;
ajuint acctot = 0;
ajuint svtot = 0;
ajuint orgtot = 0;
ajuint destot = 0;
ajuint keytot = 0;


/* @datastatic DbxflatPParser *************************************************
**
** Parser definition structure
**
** @alias DbxflatSParser
** @alias DbxflatOParser
**
** @attr Name [const char*] Parser name
** @attr Parser [(AjBool*)] Parser function
** @@
******************************************************************************/

typedef struct DbxflatSParser
{
    const char* Name;
    AjBool (*Parser) (EmbPBtreeEntry entry, AjPFile inf);
} DbxflatOParser;
#define DbxflatPParser DbxflatOParser*




static DbxflatOParser parser[] =
{
    {"EMBL",   dbxflat_ParseEmbl},
    {"SWISS",  dbxflat_ParseEmbl},
    {"GB",     dbxflat_ParseGenbank},
    {"REFSEQ", dbxflat_ParseGenbank},
    {"FASTQ",  dbxflat_ParseFastq},
    {NULL,     NULL}
};





/* @prog dbxflat **************************************************************
**
** Index a flat file database
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPBtreeEntry entry = NULL;
    
    AjPStr dbname   = NULL;
    AjPStr dbrs     = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;
    AjBool statistics;
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

    AjPStr word = NULL;
    
    AjPBtId  idobj  = NULL;
    AjPBtPri priobj = NULL;
    AjPBtHybrid hyb = NULL;
    
    ajulong nentries = 0L;
    ajulong ientries = 0L;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;
    ajlong startclock = 0;
    ajlong beginclock = 0;
    ajlong nowclock = 0;
    
    ajulong idcache=0L, idread = 0L, idwrite = 0L, idsize= 0L;
    ajulong accache=0L, acread = 0L, acwrite = 0L, acsize= 0L;
    ajulong svcache=0L, svread = 0L, svwrite = 0L, svsize= 0L;
    ajulong kwcache=0L, kwread = 0L, kwwrite = 0L, kwsize= 0L;
    ajulong decache=0L, deread = 0L, dewrite = 0L, desize= 0L;
    ajulong txcache=0L, txread = 0L, txwrite = 0L, txsize= 0L;

    double tdiff = 0.0;
    ajint days = 0;
    ajint hours = 0;
    ajint mins = 0;
    
    embInit("dbxflat", argc, argv);

    dbtype     = ajAcdGetListSingle("idformat");
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
    statistics = ajAcdGetBoolean("statistics");
    compressed = ajAcdGetBoolean("compressed");

    entry = embBtreeEntryNew();
    if(compressed)
        embBtreeEntrySetCompressed(entry);

    tmpstr = ajStrNew();
    
    idobj   = ajBtreeIdNew();
    priobj  = ajBtreePriNew();
    hyb     = ajBtreeHybNew();
    

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
        else if(ajStrMatchC(fieldarray[i], "sv"))
        {
            svfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(svfield);
        }
        else if(ajStrMatchC(fieldarray[i], "des"))
        {
            desfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(desfield);
        }
        else if(ajStrMatchC(fieldarray[i], "key"))
        {
            keyfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(keyfield);
        }
        else if(ajStrMatchC(fieldarray[i], "org"))
        {
            orgfield = embBtreeGetFieldS(entry, fieldarray[i]);
            if(compressed)
                embBtreeFieldSetCompressed(orgfield);
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
        beginclock = ajClockNow();

	ajListPop(entry->files,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(inf=ajFileNewInNameS(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	ajFilenameTrimPath(&tmpstr);
	ajFmtPrintF(outf,"Processing file: %S",tmpstr);

	ientries = 0L;

	while(dbxflat_NextEntry(entry,inf))
	{
	    ++ientries;

	    if(entry->do_id)
	    {
                if(ajStrGetLen(entry->id) > entry->idlen)
                {
                    if(ajStrGetLen(entry->id) > maxidlen)
                    {
                        ajWarn("id '%S' too long, truncating to idlen %d",
                               entry->id, entry->idlen);
                        maxidlen = ajStrGetLen(entry->id);
                    }
                    idtrunc++;
                    ajStrKeepRange(&entry->id,0,entry->idlen-1);
                }
    
		ajStrFmtLower(&entry->id);
		ajStrAssignS(&hyb->key1,entry->id);
		hyb->dbno = i;
		hyb->offset = entry->fpos;
		hyb->dups = 0;
		ajBtreeHybInsertId(entry->idcache,hyb);
                ++idtot;
	    }

	    if(accfield)
	    {
                while(ajListPop(accfield->data,(void **)&word))
                {
		    ajStrFmtLower(&word);
                    ajStrAssignS(&hyb->key1,word);
                    hyb->dbno = i;
		    hyb->offset = entry->fpos;
		    hyb->dups = 0;
		    ajBtreeHybInsertId(accfield->cache,hyb);
                    ++acctot;
		    ajStrDel(&word);
                }
	    }

	    if(svfield)
	    {
                while(ajListPop(svfield->data,(void **)&word))
                {
		    ajStrFmtLower(&word);
                    ajStrAssignS(&hyb->key1,word);
                    hyb->dbno = i;
		    hyb->offset = entry->fpos;
		    hyb->dups = 0;
		    ajBtreeHybInsertId(svfield->cache,hyb);
                    ++svtot;
		    ajStrDel(&word);
                }
	    }

	    if(keyfield)
	    {
                while(ajListPop(keyfield->data,(void **)&word))
                {
		    ajStrFmtLower(&word);
		    ajStrAssignS(&priobj->id,entry->id);
                    ajStrAssignS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(keyfield->cache, priobj);
                    ++keytot;
		    ajStrDel(&word);
                }
	    }

	    if(desfield)
	    {
                while(ajListPop(desfield->data,(void **)&word))
                {
		    ajStrFmtLower(&word);
		    ajStrAssignS(&priobj->id,entry->id);
                    ajStrAssignS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    if(ajBtreeInsertKeyword(desfield->cache, priobj))
                        ++destot;
		    ajStrDel(&word);
                }
	    }

	    if(orgfield)
	    {
                while(ajListPop(orgfield->data,(void **)&word))
                {
		    ajStrFmtLower(&word);
		    ajStrAssignS(&priobj->id,entry->id);
                    ajStrAssignS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(orgfield->cache, priobj);
                    ++orgtot;
		    ajStrDel(&word);
                }
	    }
	}
	
	ajFileClose(&inf);
	nentries += ientries;
	nowtime = ajTimeNewToday();
        nowclock = ajClockNow();
	ajFmtPrintF(outf, " entries: %Lu (%Lu) time: %.1f/%.1fs (%.1f/%.1fs)\n",
		    nentries, ientries,
		    ajClockDiff(startclock,nowclock),
                    ajTimeDiff(starttime, nowtime),
		    ajClockDiff(beginclock,nowclock),
                    ajTimeDiff(begintime, nowtime));
        if(statistics)
        {
            if(entry->do_id)
                ajBtreeCacheStatsOut(outf, entry->idcache,
                                     &idcache, &idread, &idwrite, &idsize);
            if(accfield)
                ajBtreeCacheStatsOut(outf, accfield->cache,
                                     &accache, &acread, &acwrite, &acsize);
            if(svfield)
                ajBtreeCacheStatsOut(outf, svfield->cache,
                                     &svcache, &svread, &svwrite, &svsize);
            if(keyfield)
                ajBtreeCacheStatsOut(outf, keyfield->cache,
                                     &kwcache, &kwread, &kwwrite, &kwsize);
            if(desfield)
                ajBtreeCacheStatsOut(outf, desfield->cache,
                                     &decache, &deread, &dewrite, &desize);
            if(orgfield)
                ajBtreeCacheStatsOut(outf, orgfield->cache,
                                     &txcache, &txread, &txwrite, &txsize);
        }

	ajTimeDel(&begintime);
	ajTimeDel(&nowtime);
    }
    


    embBtreeDumpParameters(entry);
    embBtreeCloseCaches(entry);
    
    nowtime = ajTimeNewToday();
    tdiff = ajTimeDiff(starttime, nowtime);
    days = (ajint) (tdiff/(24.0*3600.0));
    tdiff -= (24.0*3600.0)*(double)days;
    hours = (ajint) (tdiff/3600.0);
    tdiff -= 3600.0*(double)hours;
    mins = (ajint) (tdiff/60.0);
    tdiff -= 60.0 * (double) mins;
    if(days)
        ajFmtPrintF(outf, "Total time: %d %02d:%02d:%04.1f\n",
                    days, hours, mins, tdiff);
    else if (hours)
        ajFmtPrintF(outf, "Total time: %d:%02d:%04.1f\n",
                    hours, mins, tdiff);
    else 
        ajFmtPrintF(outf, "Total time: %d:%04.1f\n",
                    mins, tdiff);
    ajTimeDel(&nowtime);
    ajTimeDel(&starttime);

    if(maxidlen)
    {
        ajFmtPrintF(outf,
                    "Resource idlen truncated %u IDs. "
                    "Maximum ID length was %u.",
                    idtrunc, maxidlen);
        ajWarn("Resource idlen truncated %u IDs. Maximum ID length was %u.",
               idtrunc, maxidlen);
    }

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

    ajRegFree(&dbxflat_wrdexp);
    embExit();

    return 0;
}




/* @funcstatic dbxflat_ParseEmbl **********************************************
**
** Parse the ID, accession from an EMBL entry.
**
** Reads to the end of the entry and then returns.
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseEmbl(EmbPBtreeEntry entry, AjPFile inf)
{
    AjPStr line = NULL;
    ajlong pos  = 0L;
    
    line = ajStrNewC("");
    
    while(!ajStrPrefixC(line,"//"))
    {
	pos = ajFileResetPos(inf);
	
	if(!ajReadlineTrim(inf,&line))
	{
	    ajStrDel(&line);
	    return ajFalse;
	}
	if(ajStrPrefixC(line,"ID"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(line,"%*S%S",&entry->id);
	    ajStrTrimEndC(&entry->id, ";");
/*
	    ++global;
	    printf("%d. %s\n",global,ajStrGetPtr(entry->id));
*/
	    if(svfield)
		embBtreeEmblSV(line,svfield->data);
	}


	if(svfield)
	    if(ajStrPrefixC(line,"SV") ||
	       ajStrPrefixC(line,"IV"))	/* emblcds database format */
		embBtreeEmblAC(line,svfield->data);

	if(accfield)
	    if(ajStrPrefixC(line,"AC") ||
	       ajStrPrefixC(line,"PA"))	/* emblcds database format */
		embBtreeEmblAC(line,accfield->data);
	
	if(keyfield)
	    if(ajStrPrefixC(line,"KW"))
		embBtreeEmblKW(line,keyfield->data,keyfield->len);

	if(desfield)
	    if(ajStrPrefixC(line,"DE"))
		embBtreeEmblDE(line,desfield->data,desfield->len);

	if(orgfield)
	    if(ajStrPrefixC(line,"OC") || ajStrPrefixC(line,"OS"))
		embBtreeEmblTX(line,orgfield->data,orgfield->len);
    }
    

    ajStrDel(&line);
    
    return ajTrue;
}




/* @funcstatic dbxflat_ParseGenbank *******************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseGenbank(EmbPBtreeEntry entry, AjPFile inf)
{
    AjPStr line = NULL;
    ajlong pos  = 0L;
    AjBool ret = ajTrue;
    
    AjPStr sumline = NULL;
    
    line = ajStrNewC("");
    sumline = ajStrNew();

    
    while(!ajStrPrefixC(line,"//") && ret)
    {
	if(ajStrPrefixC(line,"LOCUS"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(line,"%*S%S",&entry->id);
	}
	
	if(svfield)
	    if(ajStrPrefixC(line,"VERSION"))
		embBtreeGenBankAC(line,svfield->data);
	
	if(accfield)
	    if(ajStrPrefixC(line,"ACCESSION"))
		embBtreeGenBankAC(line,accfield->data);
	
	if(keyfield)
	    if(ajStrPrefixC(line,"KEYWORDS"))
	    {
		ajStrAssignS(&sumline,line);
		ret = ajReadlineTrim(inf,&line);
                while(ret && *MAJSTRGETPTR(line)==' ')
		{
		    ajStrAppendS(&sumline,line);
		    ret = ajReadlineTrim(inf,&line);
		}
		ajStrRemoveWhiteExcess(&sumline);
		embBtreeGenBankKW(sumline,keyfield->data,keyfield->len);
		continue;
	    }

	if(desfield)
	    if(ajStrPrefixC(line,"DEFINITION"))
	    {
		ajStrAssignS(&sumline,line);
		ret = ajReadlineTrim(inf,&line);
		while(ret && *MAJSTRGETPTR(line)==' ')
		{
		    ajStrAppendS(&sumline,line);
		    ret = ajReadlineTrim(inf,&line);
		}
		ajStrRemoveWhiteExcess(&sumline);
		embBtreeGenBankDE(sumline,desfield->data,desfield->len);
		continue;
	    }
	

	if(orgfield)
	    if(ajStrPrefixC(line,"SOURCE"))
	    {
		ret = ajReadlineTrim(inf,&line);
		ajStrAppendC(&line,";");
		while(ret && *MAJSTRGETPTR(line)==' ')
		{
		    ajStrAppendS(&sumline,line);
		    ret = ajReadlineTrim(inf,&line);
		}
		ajStrRemoveWhiteExcess(&sumline);
		embBtreeGenBankTX(sumline,orgfield->data,orgfield->len);
		continue;
	    }
	

	pos = ajFileResetPos(inf);

	if(!ajReadlineTrim(inf,&line))
	    ret = ajFalse;
    }

    ajStrDel(&line);
    ajStrDel(&sumline);
    
    return ret;
}




/* @funcstatic dbxflat_ParseFastq *********************************************
**
** Parse the ID, accession from a FASTQ format sequence entry.
**
** Reads to the end of the entry and then returns.
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseFastq(EmbPBtreeEntry entry, AjPFile inf)
{
    AjPStr line = NULL;
    ajlong pos  = 0L;
    ajuint seqlen = 0;
    ajuint qlen = 0;
    AjPStr tmpfd  = NULL;
    AjPStr str = NULL;
    AjPStr de = NULL;
    AjBool ok;

    if(!dbxflat_wrdexp)
	dbxflat_wrdexp = ajRegCompC("([A-Za-z0-9.:=]+)");

    line = ajStrNewC("");
    
    pos = ajFileResetPos(inf);

    if(!ajReadlineTrim(inf,&line))
    {
        ajStrDel(&line);
        return ajFalse;
    }

    /* first line of entry */

    if(!ajStrPrefixC(line,"@"))
        return ajFalse;

    entry->fpos = pos;
    ajStrCutStart(&line, 1);
    ajStrExtractFirst(line, &de, &entry->id);

    if(desfield && ajStrGetLen(de))
    {
	while(ajRegExec(dbxflat_wrdexp,de))
	{
	    ajRegSubI(dbxflat_wrdexp, 1, &tmpfd);
	    str = ajStrNew();
	    ajStrAssignS(&str,tmpfd);
	    ajListPush(desfield->data,(void *)str);
	    ajRegPost(dbxflat_wrdexp, &de);
	}
    }

/* now read sequence */
    ok = ajReadlineTrim(inf,&line);
    while(ok && !ajStrPrefixC(line, "+"))
    {
        ajStrRemoveWhite(&line);
        seqlen += MAJSTRGETLEN(line);
        ok = ajReadlineTrim(inf,&line);
    }

    if(!ok)
        return ajFalse;

    ok = ajReadlineTrim(inf,&line);
    while(ok)
    {
        qlen += MAJSTRGETLEN(line);
        if(qlen < seqlen)
            ok = ajReadlineTrim(inf,&line);
        else
            ok = ajFalse;
    }

    ajStrDel(&de);
    ajStrDel(&tmpfd);
    ajStrDel(&line);
    
    return ajTrue;
}





/* @funcstatic dbxflat_NextEntry ********************************************
**
** Parse the next entry from a flatfile
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxflat_NextEntry(EmbPBtreeEntry entry, AjPFile inf)
{
    static AjBool init = AJFALSE;
    static ajint  nparser = -1;
    ajint i;
    
    if(!init)
    {
	entry->fpos = 0L;
	for(i=0; parser[i].Name && nparser == -1; ++i)
	    if(ajStrMatchC(entry->dbtype, parser[i].Name))
		nparser = i;
	if(nparser == -1)
	    ajFatal("Database format (%S) unknown",entry->dbtype);
	init = ajTrue;
    }
    

    if(!parser[nparser].Parser(entry,inf))
	return ajFalse;
    


    return ajTrue;
}

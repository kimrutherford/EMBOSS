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

static AjPStr dbxflatRdline = NULL;
static AjPStr dbxflatSumline = NULL;

static AjPStr swissAccstr = NULL;

static AjBool dbxflat_ParseFastq(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseEmbl(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseGenbank(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseSwiss(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseIguspto(EmbPBtreeEntry entry, AjPFile inf);

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
** @attr Parser [AjBool function] Parser function
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
    {"SWISS",  dbxflat_ParseSwiss},
    {"GB",     dbxflat_ParseGenbank},
    {"REFSEQ", dbxflat_ParseGenbank},
    {"FASTQ",  dbxflat_ParseFastq},
    {"USPTO",  dbxflat_ParseIguspto},
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

    ajulong nentries = 0UL;
    ajulong ientries = 0UL;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;
    ajlong startclock = 0UL;
    ajlong beginclock = 0UL;
    ajlong nowclock = 0UL;
    
    ajulong idpricache=0L, idpriread = 0L, idpriwrite = 0L, idprisize= 0L;
    ajulong idseccache=0L, idsecread = 0L, idsecwrite = 0L, idsecsize= 0L;
    ajulong acpricache=0L, acpriread = 0L, acpriwrite = 0L, acprisize= 0L;
    ajulong acseccache=0L, acsecread = 0L, acsecwrite = 0L, acsecsize= 0L;
    ajulong svpricache=0L, svpriread = 0L, svpriwrite = 0L, svprisize= 0L;
    ajulong svseccache=0L, svsecread = 0L, svsecwrite = 0L, svsecsize= 0L;
    ajulong kwpricache=0L, kwpriread = 0L, kwpriwrite = 0L, kwprisize= 0L;
    ajulong kwseccache=0L, kwsecread = 0L, kwsecwrite = 0L, kwsecsize= 0L;
    ajulong depricache=0L, depriread = 0L, depriwrite = 0L, deprisize= 0L;
    ajulong deseccache=0L, desecread = 0L, desecwrite = 0L, desecsize= 0L;
    ajulong txpricache=0L, txpriread = 0L, txpriwrite = 0L, txprisize= 0L;
    ajulong txseccache=0L, txsecread = 0L, txsecwrite = 0L, txsecsize= 0L;

    ajulong splitrootid =0l, splitrootnum=0L;
    ajulong splitrootkey=0L, splitrootsec=0L;
    ajulong splitleafid =0L, splitleafnum=0L;
    ajulong splitleafkey=0L, splitleafsec=0L;
    ajulong reorderid   =0L, reordernum  =0L;
    ajulong reorderkey  =0L, reordersec  =0L;

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

    entry = embBtreeEntryNew(0);
    if(compressed)
        embBtreeEntrySetCompressed(entry);

    tmpstr = ajStrNew();
    
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
	ajFmtPrintF(outf,"Processing file: %S\n",tmpstr);

	ientries = 0L;

	while(dbxflat_NextEntry(entry,inf))
	{
	    ++ientries;

	    if(entry->do_id)
	    {
		embBtreeIndexEntry(entry, i);
                ++idtot;
	    }

	    if(accfield)
	    {
                acctot += embBtreeIndexPrimary(accfield, entry, i);
	    }

	    if(svfield)
	    {
                svtot += embBtreeIndexPrimary(svfield, entry, i);
	    }

	    if(keyfield)
	    {
                keytot += embBtreeIndexSecondary(keyfield, entry);
	    }

	    if(desfield)
	    {
                destot += embBtreeIndexSecondary(desfield, entry);
	    }

	    if(orgfield)
	    {
                orgtot += embBtreeIndexSecondary(orgfield, entry);
            }
	}
	
	ajFileClose(&inf);
	nentries += ientries;
	nowtime = ajTimeNewToday();
        nowclock = ajClockNow();
	ajFmtPrintF(outf, "entries: %Lu (%Lu) time: %.1f/%.1fs (%.1f/%.1fs)\n",
		    nentries, ientries,
		    ajClockDiff(startclock,nowclock),
                    ajTimeDiff(starttime, nowtime),
		    ajClockDiff(beginclock,nowclock),
                    ajTimeDiff(begintime, nowtime));

        if(statistics)
        {
            ajBtreeStatsOut(outf,
                            &splitrootid, &splitrootnum,
                            &splitrootkey, &splitrootsec,
                            &splitleafid, &splitleafnum,
                            &splitleafkey, &splitleafsec,
                            &reorderid, &reordernum,
                            &reorderkey, &reordersec);

            if(entry->do_id)
                ajBtreeCacheStatsOut(outf, entry->idcache,
                                     &idpricache, &idseccache,
                                     &idpriread, &idsecread,
                                     &idpriwrite, &idsecwrite,
                                     &idprisize, &idsecsize);
            if(accfield)
                ajBtreeCacheStatsOut(outf, accfield->cache,
                                     &acpricache, &acseccache,
                                     &acpriread,  &acsecread,
                                     &acpriwrite, &acsecwrite,
                                     &acprisize, &acsecsize);
            if(svfield)
                ajBtreeCacheStatsOut(outf, svfield->cache,
                                     &svpricache, &svseccache,
                                     &svpriread, &svsecread,
                                     &svpriwrite, &svsecwrite,
                                     &svprisize, &svsecsize);
            if(keyfield)
                ajBtreeCacheStatsOut(outf, keyfield->cache,
                                     &kwpricache, &kwseccache,
                                     &kwpriread, &kwsecread,
                                     &kwpriwrite, &kwsecwrite,
                                     &kwprisize, &kwsecsize);
            if(desfield)
                ajBtreeCacheStatsOut(outf, desfield->cache,
                                     &depricache, &deseccache,
                                     &depriread, &desecread,
                                     &depriwrite, &desecwrite,
                                     &deprisize, &desecsize);
            if(orgfield)
                ajBtreeCacheStatsOut(outf, orgfield->cache,
                                     &txpricache, &txseccache,
                                     &txpriread, &txsecread,
                                     &txpriwrite, &txsecwrite,
                                     &txprisize, &txsecsize);
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

    embBtreeReportEntry(outf, entry);

    if(accfield)
        embBtreeReportField(outf, accfield);
    if(svfield)
        embBtreeReportField(outf, svfield);
    if(orgfield)
        embBtreeReportField(outf, orgfield);
    if(desfield)
        embBtreeReportField(outf, desfield);
    if(keyfield)
        embBtreeReportField(outf, keyfield);

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

    ajRegFree(&dbxflat_wrdexp);
    ajStrDel(&dbxflatRdline);
    ajStrDel(&dbxflatSumline);
    ajStrDel(&swissAccstr);

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
    ajlong pos  = 0L;
    
    ajStrAssignC(&dbxflatRdline, "");
    
    while(!ajStrPrefixC(dbxflatRdline, "//"))
    {
	pos = ajFileResetPos(inf);
	
	if(!ajReadlineTrim(inf,&dbxflatRdline))
	{
	    ajStrDel(&dbxflatRdline);
	    return ajFalse;
	}

	if(ajStrPrefixC(dbxflatRdline,"ID"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(dbxflatRdline,"%*S%S",&entry->id);
	    ajStrTrimEndC(&entry->id, ";");
	    if(svfield)
		embBtreeParseEmblSv(dbxflatRdline,svfield);
	}


	if(svfield)
	    if(ajStrPrefixC(dbxflatRdline,"SV") ||
	       ajStrPrefixC(dbxflatRdline,"IV"))  /* emblcds database format */
		embBtreeParseEmblAc(dbxflatRdline,svfield);

	if(accfield)
	    if(ajStrPrefixC(dbxflatRdline,"AC") ||
	       ajStrPrefixC(dbxflatRdline,"PA"))  /* emblcds database format */
		embBtreeParseEmblAc(dbxflatRdline,accfield);
	
	if(keyfield)
	    if(ajStrPrefixC(dbxflatRdline,"KW"))
		embBtreeParseEmblKw(dbxflatRdline,keyfield);

	if(desfield)
	    if(ajStrPrefixC(dbxflatRdline,"DE"))
		embBtreeParseEmblDe(dbxflatRdline,desfield);

	if(orgfield)
	    if(ajStrPrefixC(dbxflatRdline,"OC") ||
               ajStrPrefixC(dbxflatRdline,"OS"))
		embBtreeParseEmblTx(dbxflatRdline,orgfield);
    }
    
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
    ajlong pos  = 0L;
    AjBool ret = ajTrue;
    
    ajStrAssignC(&dbxflatRdline, "");
    ajStrAssignC(&dbxflatSumline, "");
    
    while(!ajStrPrefixC(dbxflatRdline,"//") && ret)
    {
	if(ajStrPrefixC(dbxflatRdline,"LOCUS"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(dbxflatRdline,"%*S%S",&entry->id);
	}
	
	if(svfield)
	    if(ajStrPrefixC(dbxflatRdline,"VERSION"))
		embBtreeParseGenbankAc(dbxflatRdline,svfield);
	
	if(accfield)
	    if(ajStrPrefixC(dbxflatRdline,"ACCESSION"))
		embBtreeParseGenbankAc(dbxflatRdline,accfield);
	
	if(keyfield)
	    if(ajStrPrefixC(dbxflatRdline,"KEYWORDS"))
	    {
		ajStrAssignS(&dbxflatSumline,dbxflatRdline);
		ret = ajReadlineTrim(inf,&dbxflatRdline);
                while(ret && *MAJSTRGETPTR(dbxflatRdline)==' ')
		{
		    ajStrAppendS(&dbxflatSumline,dbxflatRdline);
		    ret = ajReadlineTrim(inf,&dbxflatRdline);
		}
		ajStrRemoveWhiteExcess(&dbxflatSumline);
		embBtreeParseGenbankKw(dbxflatSumline,keyfield);
		continue;
	    }

	if(desfield)
	    if(ajStrPrefixC(dbxflatRdline,"DEFINITION"))
	    {
		ajStrAssignS(&dbxflatSumline,dbxflatRdline);
		ret = ajReadlineTrim(inf,&dbxflatRdline);
		while(ret && *MAJSTRGETPTR(dbxflatRdline)==' ')
		{
		    ajStrAppendS(&dbxflatSumline,dbxflatRdline);
		    ret = ajReadlineTrim(inf,&dbxflatRdline);
		}
		ajStrRemoveWhiteExcess(&dbxflatSumline);
		embBtreeParseGenbankDe(dbxflatSumline,desfield);
		continue;
	    }
	

	if(orgfield)
	    if(ajStrPrefixC(dbxflatRdline,"SOURCE"))
	    {
		ajStrAssignC(&dbxflatSumline,"");
		ret = ajReadlineTrim(inf,&dbxflatRdline);
                ajStrAppendC(&dbxflatRdline, ";");
		while(ret && *MAJSTRGETPTR(dbxflatRdline)==' ')
		{
		    ajStrAppendS(&dbxflatSumline,dbxflatRdline);
		    ret = ajReadlineTrim(inf,&dbxflatRdline);
		}
		ajStrRemoveWhiteExcess(&dbxflatSumline);
		embBtreeParseGenbankTx(dbxflatSumline,orgfield);
		continue;
	    }
	

	pos = ajFileResetPos(inf);

	if(!ajReadlineTrim(inf,&dbxflatRdline))
	    ret = ajFalse;
    }

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
    ajlong pos  = 0L;
    ajuint seqlen = 0;
    ajuint qlen = 0;
    AjPStr tmpfd  = NULL;
    AjPStr str = NULL;
    AjPStr de = NULL;
    AjBool ok;

    if(!dbxflat_wrdexp)
	dbxflat_wrdexp = ajRegCompC("([A-Za-z0-9.:=]+)");

    ajStrAssignC(&dbxflatRdline, "");
    
    pos = ajFileResetPos(inf);

    if(!ajReadlineTrim(inf,&dbxflatRdline))
    {
        ajStrDel(&dbxflatRdline);
        return ajFalse;
    }

    /* first line of entry */

    if(!ajStrPrefixC(dbxflatRdline,"@"))
        return ajFalse;

    entry->fpos = pos;
    ajStrCutStart(&dbxflatRdline, 1);
    ajStrExtractFirst(dbxflatRdline, &de, &entry->id);

    if(desfield && ajStrGetLen(de))
    {
	while(ajRegExec(dbxflat_wrdexp,de))
	{
	    ajRegSubI(dbxflat_wrdexp, 1, &tmpfd);
	    str = ajStrNew();
	    ajStrAssignS(&str,tmpfd);
	    ajListstrPushAppend(desfield->data, str);
	    ajRegPost(dbxflat_wrdexp, &de);
	}
    }

/* now read sequence */
    ok = ajReadlineTrim(inf,&dbxflatRdline);
    while(ok && !ajStrPrefixC(dbxflatRdline, "+"))
    {
        ajStrRemoveWhite(&dbxflatRdline);
        seqlen += MAJSTRGETLEN(dbxflatRdline);
        ok = ajReadlineTrim(inf,&dbxflatRdline);
    }

    if(!ok)
        return ajFalse;

    ok = ajReadlineTrim(inf,&dbxflatRdline);
    while(ok)
    {
        qlen += MAJSTRGETLEN(dbxflatRdline);
        if(qlen < seqlen)
            ok = ajReadlineTrim(inf,&dbxflatRdline);
        else
            ok = ajFalse;
    }

    ajStrDel(&de);
    ajStrDel(&tmpfd);
    
    return ajTrue;
}





/* @funcstatic dbxflat_ParseIguspto *******************************************
**
** Parse the ID, accession from a USPTO format sequence entry.
**
** Reads to the end of the entry and then returns.
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseIguspto(EmbPBtreeEntry entry, AjPFile inf)
{
    ajlong pos  = 0L;
    ajuint seqlen = 0;
    AjPStr tmpfd  = NULL;
    AjPStr str = NULL;
    AjPStr de = NULL;
    AjBool ok = ajTrue;

    if(!dbxflat_wrdexp)
	dbxflat_wrdexp = ajRegCompC("([A-Za-z0-9.:=]+)");

    pos = ajFileResetPos(inf);

    if(!MAJSTRGETLEN(dbxflatRdline))
        ok = ajReadlineTrim(inf,&dbxflatRdline);

    if(!ok)
    {
        ajStrDel(&dbxflatRdline);
        return ajFalse;
    }

    /* first line of entry */

    entry->fpos = pos;

    if(!ajStrPrefixC(dbxflatRdline,";"))
        return ajFalse;

    while(ok && ajStrPrefixC(dbxflatRdline, ";"))
    {
        ajStrAssignSubS(&de, dbxflatRdline, 2, -1);

        if(desfield && ajStrGetLen(de))
        {
            while(ajRegExec(dbxflat_wrdexp,de))
            {
                ajRegSubI(dbxflat_wrdexp, 1, &tmpfd);
                str = ajStrNew();
                ajStrAssignS(&str,tmpfd);
                ajListstrPushAppend(desfield->data, str);
                ajRegPost(dbxflat_wrdexp, &de);
            }
        }

        ok = ajReadlineTrim(inf,&dbxflatRdline);
    }

    if(!ok)
        return ajFalse;

    ajStrAssignS(&entry->id, dbxflatRdline);
    ajStrRemoveWhite(&entry->id);

/* now read sequence */
    ok = ajReadlineTrim(inf,&dbxflatRdline);
    while(ok && !ajStrPrefixC(dbxflatRdline, ";"))
    {
        ajStrRemoveWhite(&dbxflatRdline);
        seqlen += MAJSTRGETLEN(dbxflatRdline);
        ok = ajReadlineTrim(inf,&dbxflatRdline);
    }

    ajStrDel(&de);
    ajStrDel(&tmpfd);
    
    return ajTrue;
}





/* @funcstatic dbxflat_ParseSwiss *********************************************
**
** Parse the ID, accession from a SwissProt or UniProtKB entry.
**
** Reads to the end of the entry and then returns.
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseSwiss(EmbPBtreeEntry entry, AjPFile inf)
{
    ajlong pos  = 0L;
    const char* swissprefix[] = {
        "RecName: ", "AltName: ", "SubName: ",
        "Includes:", "Contains:", "Flags: ",
        "Full=", "Short=", "EC=",
        "Allergen=", "Biotech=", "CD_antigen=", "INN=",
        NULL
    };
    ajuint swisslen[] = {
        9, 9, 9,
        9, 9, 7,
        5, 6, 3,
        9, 8, 11, 4,
        0
    };

    ajuint i;

    if(!dbxflat_wrdexp)
	dbxflat_wrdexp = ajRegCompC("([A-Za-z0-9_-]+)");

    ajStrAssignC(&dbxflatRdline, "");
    
    while(!ajStrPrefixC(dbxflatRdline,"//"))
    {
	pos = ajFileResetPos(inf);
	
	if(!ajReadlineTrim(inf,&dbxflatRdline))
	{
            if(svfield)
                ajStrDel(&swissAccstr);

	    return ajFalse;
	}

	if(ajStrPrefixC(dbxflatRdline,"ID"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(dbxflatRdline,"%*S%S",&entry->id);
	    ajStrTrimEndC(&entry->id, ";");
	}


	if(svfield)
        {
	    if(ajStrPrefixC(dbxflatRdline,"SV") ||
	       ajStrPrefixC(dbxflatRdline,"IV"))  /* emblcds database format */
		embBtreeParseEmblAc(dbxflatRdline,svfield);

            if(!MAJSTRGETLEN(swissAccstr) && ajStrPrefixC(dbxflatRdline,"AC"))
                embBtreeFindEmblAc(dbxflatRdline, svfield, &swissAccstr);

            if(MAJSTRGETLEN(swissAccstr) &&
               ajStrMatchWildC(dbxflatRdline,
                               "DT   \?\?-\?\?\?-\?\?\?\?, sequence version *"))
            {
                ajStrAppendK(&swissAccstr, '.');
                ajStrAppendSubS(&swissAccstr, dbxflatRdline, 35, -3);
                ajStrTrimEndC(&swissAccstr, ".\n\r"); /* in case of \n\r */
                ajListstrPushAppend(svfield->data, swissAccstr);
                swissAccstr = NULL;
            }
        }

	if(accfield)
	    if(ajStrPrefixC(dbxflatRdline,"AC") ||
	       ajStrPrefixC(dbxflatRdline,"PA"))  /* emblcds database format */
		embBtreeParseEmblAc(dbxflatRdline,accfield);
	
	if(keyfield)
	    if(ajStrPrefixC(dbxflatRdline,"KW"))
		embBtreeParseEmblKw(dbxflatRdline,keyfield);

	if(desfield)
	    if(ajStrPrefixC(dbxflatRdline,"DE"))
            {
                ajStrCutStart(&dbxflatRdline, 5);
                ajStrTrimWhiteStart(&dbxflatRdline);

                /*
                ** trim prefixes
                ** can be multiple
                ** e.g. SubName: Full=
                */

                for(i=0; swissprefix[i]; i++)
                {
                    if(ajStrPrefixC(dbxflatRdline, swissprefix[i]))
                        ajStrCutStart(&dbxflatRdline, swisslen[i]);
                }

		embBtreeParseField(dbxflatRdline,dbxflat_wrdexp, desfield);
            }
        
	if(orgfield)
	    if(ajStrPrefixC(dbxflatRdline,"OC") ||
               ajStrPrefixC(dbxflatRdline,"OS"))
		embBtreeParseEmblTx(dbxflatRdline,orgfield);
    }

    if(svfield)
        ajStrDel(&swissAccstr);

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
    

    if(!(*parser[nparser].Parser)(entry,inf))
	return ajFalse;
    


    return ajTrue;
}

/* @source dbxgcg application
**
** Index GCG and PIR/NBRF format databases
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




#define GCGTYPE_OTHER 0
#define GCGTYPE_ID 1
#define GCGTYPE_ACC 2
#define GCGTYPE_DES 3
#define GCGTYPE_KEY 4
#define GCGTYPE_TAX 5
#define GCGTYPE_VER 6

static AjPStr dbxgcgRefline = NULL;
static AjPStr dbxgcgSeqline = NULL;
static AjPStr dbxgcgTmpfd   = NULL;
static AjPStr dbxgcgTmpstr  = NULL;
static AjPStr dbxgcgTmpline = NULL;
static AjPStr dbxgcgGcgtype = NULL;
static AjPStr dbxgcgGcgdate = NULL;
static AjPStr dbxgcgTypstr  = NULL;
static AjPStr dbxgcgReflibstr = NULL;

static AjPRegexp dbxgcg_embl_typexp = NULL;
static AjPRegexp dbxgcg_embl_idexp  = NULL;
static AjPRegexp dbxgcg_embl_verexp = NULL;
static AjPRegexp dbxgcg_embl_wrdexp = NULL;
static AjPRegexp dbxgcg_embl_phrexp = NULL;
static AjPRegexp dbxgcg_embl_taxexp = NULL;

static AjPRegexp dbxgcg_gcg_rexp = NULL;
static AjPRegexp dbxgcg_gcg_sexp = NULL;
static AjPRegexp dbxgcg_splitexp = NULL;

static AjPRegexp dbxgcg_pir_idexp  = NULL;
static AjPRegexp dbxgcg_pir_acexp  = NULL;
static AjPRegexp dbxgcg_pir_ac2exp = NULL;
static AjPRegexp dbxgcg_pir_keyexp = NULL;
static AjPRegexp dbxgcg_pir_taxexp = NULL;
static AjPRegexp dbxgcg_pir_tax2exp = NULL;
static AjPRegexp dbxgcg_pir_wrdexp = NULL;
static AjPRegexp dbxgcg_pir_phrexp = NULL;
static AjPRegexp dbxgcg_pir_pirexp = NULL;

static AjPRegexp dbxgcg_genbank_typexp = NULL;
static AjPRegexp dbxgcg_genbank_morexp = NULL;
static AjPRegexp dbxgcg_genbank_wrdexp = NULL;
static AjPRegexp dbxgcg_genbank_phrexp = NULL;
static AjPRegexp dbxgcg_genbank_taxexp = NULL;
static AjPRegexp dbxgcg_genbank_verexp = NULL;

static AjBool dbxgcg_ParseEmbl(AjPFile infr, AjPStr *reflibstr);
static AjBool dbxgcg_ParseGenbank( AjPFile infr, AjPStr *reflibstr);
static AjBool dbxgcg_ParsePir(AjPFile infr, AjPStr *reflibstr);

static AjBool dbxgcg_NextEntry(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);

static ajlong dbxgcg_gcggetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);
static ajlong dbxgcg_pirgetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);

static ajlong dbxgcg_gcgappent(AjPFile infr, AjPFile infs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr);



EmbPBtreeField accfield = NULL;
EmbPBtreeField svfield = NULL;
EmbPBtreeField orgfield = NULL;
EmbPBtreeField desfield = NULL;
EmbPBtreeField keyfield = NULL;




/* @datastatic DbxgcgPParser *************************************************
**
** Parser definition structure
**
** @alias DbxgcgSParser
** @alias DbxgcgOParser
**
** @attr Name [const char*] Parser name
** @attr GcgType [AjBool] Gcg type parser if true, PIR type if false
** @attr Padding [char[4]] Padding to alignment boundary
** @attr Parser [AjBool function] Parser function
** @@
******************************************************************************/

typedef struct DbxgcgSParser
{
    const char* Name;
    AjBool GcgType;
    char   Padding[4];
    AjBool (*Parser) (AjPFile infr, AjPStr *reflibstr);
} DbxgcgOParser;
#define DbxgcgPParser DbxgcgOParser*




static DbxgcgOParser parser[] =
{
    {"EMBL", AJTRUE, "", dbxgcg_ParseEmbl},
    {"SWISS", AJTRUE, "", dbxgcg_ParseEmbl},
    {"GENBANK", AJTRUE, "", dbxgcg_ParseGenbank},
    {"PIR", AJFALSE, "", dbxgcg_ParsePir},
    {NULL, 0, "", NULL}
};





/* @prog dbxgcg **************************************************************
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

    AjPStr refname = NULL;
    AjPStr seqname = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile infs = NULL;
    AjPFile infr = NULL;

    ajulong nentries = 0L;
    ajulong ientries = 0L;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;

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

    embInit("dbxgcg", argc, argv);

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

    entry = embBtreeEntryNew(1);
    if(compressed)
        embBtreeEntrySetCompressed(entry);
    
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


    for(i=0; i<nfiles; ++i)
    {
	ajListPop(entry->files,(void **) &seqname);
	refname = ajStrNew();
	ajStrAssignS(&refname,seqname);
	ajFilenameReplaceExtC(&seqname,"seq");
	ajFilenameReplaceExtC(&refname,"ref");
	ajListstrPushAppend(entry->files, seqname);
	ajListstrPushAppend(entry->reffiles[0], refname);
    }
    

    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);

    starttime = ajTimeNewToday();

    ajFmtPrintF(outf, "Processing directory: %S\n", directory);

    for(i=0;i<nfiles;++i)
    {
        begintime = ajTimeNewToday();

	ajListPop(entry->reffiles[0],(void **)&thysfile);
	ajListstrPushAppend(entry->files, thysfile);
	ajFmtPrintS(&dbxgcgTmpstr,"%S%S",entry->directory,thysfile);
	if(!(infr=ajFileNewInNameS(dbxgcgTmpstr)))
	    ajFatal("Cannot open input file %S\n",dbxgcgTmpstr);
	
	ajListPop(entry->files,(void **)&thysfile);
	ajListstrPushAppend(entry->files, thysfile);
	ajFmtPrintS(&dbxgcgTmpstr,"%S%S",entry->directory,thysfile);
	if(!(infs=ajFileNewInNameS(dbxgcgTmpstr)))
	    ajFatal("Cannot open input file %S\n",dbxgcgTmpstr);

	ajFilenameTrimPath(&dbxgcgTmpstr);
	ajFmtPrintF(outf,"Processing file: %S\n",dbxgcgTmpstr);

	ientries = 0L;

	while(dbxgcg_NextEntry(entry,infs,infr,dbtype))
	{
	    ++ientries;

	    if(entry->do_id)
                embBtreeIndexEntry(entry, i);

	    if(accfield)
                embBtreeIndexPrimary(accfield, entry, i);

	    if(svfield)
                embBtreeIndexPrimary(svfield, entry, i);

	    if(keyfield)
                embBtreeIndexSecondary(keyfield, entry);

	    if(desfield)
                embBtreeIndexSecondary(desfield, entry);

	    if(orgfield)
                embBtreeIndexSecondary(orgfield, entry);
	}
	
	ajFileClose(&infs);
	ajFileClose(&infr);
	nentries += ientries;
	nowtime = ajTimeNewToday();
	ajFmtPrintF(outf, "entries: %Lu (%Lu) time: %.1fs (%.1fs)\n",
		    nentries, ientries,
		    ajTimeDiff(starttime, nowtime),
		    ajTimeDiff(begintime, nowtime));

        if(statistics)
        {
            if(entry->do_id)
                ajBtreeCacheStatsOut(outf, entry->idcache,
                                     &idpricache, &idseccache,
                                     &idpriread, &idsecread,
                                     &idpriwrite, &idsecwrite,
                                     &idprisize, &idsecsize);
            if(accfield)
                ajBtreeCacheStatsOut(outf, accfield->cache,
                                     &acpricache, &acseccache,
                                     &acpriread, &acsecread,
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
    

    nowtime = ajTimeNewToday();
    ajFmtPrintF(outf, "Total time: %.1fs\n", ajTimeDiff(starttime, nowtime));
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

    embBtreeDumpParameters(entry);
    embBtreeCloseCaches(entry);
    
    ajFileClose(&outf);
    embBtreeEntryDel(&entry);

    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&dbname);
    ajStrDel(&dbrs);
    ajStrDel(&release);
    ajStrDel(&datestr);
    ajStrDel(&directory);
    ajStrDel(&indexdir);
    ajStrDel(&dbtype);
    
    ajStrDel(&dbxgcgRefline);
    ajStrDel(&dbxgcgSeqline);
    ajStrDel(&dbxgcgTmpfd);
    ajStrDel(&dbxgcgTmpstr);
    ajStrDel(&dbxgcgTmpline);
    ajStrDel(&dbxgcgGcgtype);
    ajStrDel(&dbxgcgGcgdate);
    ajStrDel(&dbxgcgTypstr);
    ajStrDel(&dbxgcgReflibstr);

    nfields = 0;
    while(fieldarray[nfields])
	ajStrDel(&fieldarray[nfields++]);
    AJFREE(fieldarray);

    ajRegFree(&dbxgcg_embl_typexp);
    ajRegFree(&dbxgcg_embl_idexp);
    ajRegFree(&dbxgcg_embl_verexp);
    ajRegFree(&dbxgcg_embl_wrdexp);
    ajRegFree(&dbxgcg_embl_phrexp);
    ajRegFree(&dbxgcg_embl_taxexp);

    ajRegFree(&dbxgcg_gcg_rexp);
    ajRegFree(&dbxgcg_gcg_sexp);

    ajRegFree(&dbxgcg_splitexp);

    ajRegFree(&dbxgcg_pir_idexp);
    ajRegFree(&dbxgcg_pir_acexp);
    ajRegFree(&dbxgcg_pir_ac2exp);
    ajRegFree(&dbxgcg_pir_keyexp);
    ajRegFree(&dbxgcg_pir_taxexp);
    ajRegFree(&dbxgcg_pir_tax2exp);
    ajRegFree(&dbxgcg_pir_wrdexp);
    ajRegFree(&dbxgcg_pir_phrexp);
    ajRegFree(&dbxgcg_pir_pirexp);

    ajRegFree(&dbxgcg_genbank_typexp);
    ajRegFree(&dbxgcg_genbank_morexp);
    ajRegFree(&dbxgcg_genbank_wrdexp);
    ajRegFree(&dbxgcg_genbank_phrexp);
    ajRegFree(&dbxgcg_genbank_taxexp);
    ajRegFree(&dbxgcg_genbank_verexp);

    embExit();

    return 0;
}




/* @funcstatic dbxgcg_NextEntry ***********************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [AjBool] ajTrue if successful read
** @@
******************************************************************************/

static AjBool dbxgcg_NextEntry(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    char *p;

    if(!dbxgcg_splitexp)
	dbxgcg_splitexp = ajRegCompC("_0+$");

    entry->reffpos[0] = ajFileResetPos(infr);
    entry->fpos    = ajFileResetPos(infs);

    if(!dbxgcg_gcggetent(entry, infs, infr, dbtype) &&
       !dbxgcg_pirgetent(entry, infs, infr, dbtype))
	return ajFalse;

    ajDebug("id '%S' seqfpos:%d reffpos:%d\n",
	    entry->id, entry->fpos, entry->reffpos);

    ajStrAssignC(&dbxgcgTmpstr,ajStrGetPtr(entry->id));

    if(ajRegExec(dbxgcg_splitexp, entry->id))
    {
	p  = strrchr(ajStrGetPtr(dbxgcgTmpstr),'_');
	*p = '\0';
	ajStrAssignC(&entry->id,ajStrGetPtr(dbxgcgTmpstr));
    }

    return ajTrue;
}




/* @funcstatic dbxgcg_gcggetent ***********************************************
**
** get a single entry from the GCG database files
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [ajlong] Sequence length
** @@
******************************************************************************/

static ajlong dbxgcg_gcggetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    static ajint called   = 0;
    static ajint iparser  = -1;
    ajlong gcglen = 0;
    ajlong rblock;
    ajint i;

    ajStrAssignC(&dbxgcgSeqline, "");
    ajStrAssignC(&dbxgcgRefline, "");

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(dbtype, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("dbtype '%S' unknown", dbtype);

	ajDebug("dbtype '%S' Parser %d\n", dbtype, iparser);
	called = 1;
    }

    if(!parser[iparser].GcgType)
    {
      return 0;
    }

    if(!dbxgcg_gcg_rexp)
	dbxgcg_gcg_rexp = ajRegCompC("^>>>>([^ \t\n]+)");

    if(!dbxgcg_gcg_sexp)
	dbxgcg_gcg_sexp = ajRegCompC("^>>>>([^ \t]+)[ \t]+"
				     "(Dummy Header|[^ \t]+)[ \t]+([^ \t]+)"
				     "[ \t]+([^ \t]+)[ \t]+([0-9]+)");

    /* check for seqid first line */
    while(ajStrGetCharFirst(dbxgcgSeqline)!='>')
    {
	if(!ajReadline(infs, &dbxgcgSeqline))
	  {
              return 0;			/* end of file */
	  }
	ajDebug("... read until next seq %Ld '%S'\n",
		ajFileResetPos(infs), dbxgcgSeqline);
    }

    ajDebug("dbxgcg_gcggetent .seq (%S) %Ld '%S'\n",
	    dbtype, ajFileResetPos(infs), dbxgcgSeqline);

    /* get the encoding/sequence length info */
    if(!ajRegExec(dbxgcg_gcg_sexp, dbxgcgSeqline))
    {
        ajDebug("dbxgcg_gcggetent sequence expression FAILED\n");
	return 0;
    }

    ajRegSubI(dbxgcg_gcg_sexp, 1, &entry->id);		/* Entry ID returned */

    ajRegSubI(dbxgcg_gcg_sexp, 2, &dbxgcgGcgdate);
    ajRegSubI(dbxgcg_gcg_sexp, 3, &dbxgcgGcgtype);
    ajRegSubI(dbxgcg_gcg_sexp, 5, &dbxgcgTmpstr);
    ajStrToLong(dbxgcgTmpstr, &gcglen);

    ajDebug("new entry '%S' date:'%S' type:'%S' len:'%S'=%Ld\n",
	    entry->id, dbxgcgGcgdate, dbxgcgGcgtype, dbxgcgTmpstr, gcglen);

    ajDebug("dbxgcg_gcggetent .ref (%S) %Ld '%S'\n",
	    dbtype, ajFileResetPos(infr), dbxgcgRefline);

    /* check for refid first line */
    while(ajStrGetCharFirst(dbxgcgRefline)!='>')
    {
	if(!ajReadline(infr, &dbxgcgRefline))
	{
	    ajErr("ref ended before seq");
	    break;			/* end of file */
	}
	ajDebug("... read until next ref %Ld '%S'\n", ajFileResetPos(infr), dbxgcgRefline);
    }

    /* get the encoding/sequence length info */

    ajRegExec(dbxgcg_gcg_rexp, dbxgcgRefline);
    ajRegSubI(dbxgcg_gcg_rexp, 1, &dbxgcgReflibstr);

    (*parser[iparser].Parser)(infr,
                              &dbxgcgReflibstr); /* writes alistfile data */

    /* get the description line */
    ajReadline(infs, &dbxgcgSeqline);

    /* seek to the end of the sequence; +1 to jump over newline */
    if(ajStrGetCharFirst(dbxgcgGcgtype)=='2')
    {
	rblock = (gcglen+3)/4;
	ajFileSeek(infs,rblock+1,SEEK_CUR);
    }
    else
	ajFileSeek(infs,gcglen+1,SEEK_CUR);

    /*
    **  for big entries, need to append until we have all the parts.
    **  They are named with _0 on the first part, _1 on the second and so on.
    **  or _00 on the first part, _01 on the second and so on.
    **  We can look for the "id_" prefix.
    */

    if(!ajStrSuffixC(entry->id, "_0") &&
       !ajStrSuffixC(entry->id,"_00") &&
       !ajStrSuffixC(entry->id,"_000") &&
       !ajStrSuffixC(entry->id,"_0000"))
	return gcglen;

    gcglen += dbxgcg_gcgappent(infr, infs, dbxgcg_gcg_rexp, dbxgcg_gcg_sexp,
			       &entry->id);

    return gcglen;
}




/* @funcstatic dbxgcg_pirgetent ***********************************************
**
** Get a single entry from the PIR database files
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [ajlong] Sequence length
** @@
******************************************************************************/

static ajlong dbxgcg_pirgetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    ajint i;
    static ajint called  = 0;
    static ajint iparser = -1;
    ajlong gcglen;
    ajlong spos = 0;

    ajStrAssignC(&dbxgcgSeqline, "");
    ajStrAssignC(&dbxgcgRefline, "");

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(dbtype, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("dbtype '%S' unknown", dbtype);
	ajDebug("dbtype '%S' Parser %d\n", dbtype, iparser);
	called = 1;
    }

    if(parser[iparser].GcgType)
	return 0;

    if(!dbxgcg_pir_pirexp)
	dbxgcg_pir_pirexp = ajRegCompC("^>..;([^ \t\n]+)");

    /* skip to seqid first line */
    while(ajStrGetCharFirst(dbxgcgSeqline)!='>')
	if(!ajReadline(infs, &dbxgcgSeqline))
        {
	    return 0;			/* end of file */
        }

    ajDebug("dbxgcg_pirgetent .seq (%S) %Ld '%S' \n",
	    dbtype, ajFileResetPos(infs), dbxgcgSeqline);

    ajRegExec(dbxgcg_pir_pirexp, dbxgcgSeqline);

    /* skip to refid first line */
    while(ajStrGetCharFirst(dbxgcgRefline)!='>')
	if(!ajReadline(infr, &dbxgcgRefline))
	{
	    ajErr("ref ended before seq"); /* end of file */
	    break;
	}

    /* get the encoding/sequence length info */

    ajRegExec(dbxgcg_pir_pirexp, dbxgcgRefline);
    ajRegSubI(dbxgcg_pir_pirexp, 1, &dbxgcgReflibstr);
    ajRegSubI(dbxgcg_pir_pirexp, 1, &entry->id);

    ajDebug("dbigcg_pirgetent seqid '%S' spos: %Ld\n",
	    entry->id, ajFileResetPos(infs));
    ajDebug("dbxgcg_pirgetent refid '%S' spos: %Ld\n",
	    entry->id, ajFileResetPos(infr));

    (*parser[iparser].Parser)(infr,
                              &dbxgcgReflibstr);/* writes alistfile data */

    /* get the description line */
    ajReadline(infs, &dbxgcgSeqline);
    gcglen = 0;

    /* seek to the end of the sequence; +1 to jump over newline */
    while(ajStrGetCharFirst(dbxgcgSeqline)!='>')
    {
	spos = ajFileResetPos(infs);
	if(!ajReadline(infs, &dbxgcgSeqline))
	{
	    spos = 0;
	    break;
	}
	gcglen += ajStrGetLen(dbxgcgSeqline);
    }

    if(spos)
	ajFileSeek(infs, spos, 0);

    ajDebug("dbxgcg_pirgetent end spos %Ld line '%S'\n", spos, dbxgcgSeqline);

    return gcglen;
}




/* @funcstatic dbxgcg_gcgappent ***********************************************
**
** Go to end of a split GCG entry
**
** @param [u] infr [AjPFile] Reference file
** @param [u] infs [AjPFile] Sequence file
** @param [u] rexp [AjPRegexp] Regular expression to find ID in ref file
** @param [u] sexp [AjPRegexp] Regular expression to find ID in seq file
** @param [w] libstr [AjPStr*] ID
** @return [ajlong] Sequence length for this section
** @@
******************************************************************************/

static ajlong dbxgcg_gcgappent(AjPFile infr, AjPFile infs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr)
{
    AjPStr reflibstr = NULL;
    AjPStr seqlibstr = NULL;
    AjPStr testlibstr = NULL;
    ajint ilen;

    AjBool isend;
    const char *p;
    char *q;
    ajlong rpos;
    ajlong spos;

    /*
    ** keep reading until the end of entry is reached
    ** and return the extra number of bases
    */

    if(!testlibstr)
	testlibstr = ajStrNew();

    ajStrAssignS(&dbxgcgTmpstr,*libstr);

    ajDebug("dbi_gcgappent '%S'\n", dbxgcgTmpstr);

    p = ajStrGetPtr(dbxgcgTmpstr);
    q = strrchr(p,'_');
    *q = '\0';


    ajFmtPrintS(&testlibstr, "%s_",p);
    ilen = ajStrGetLen(testlibstr);

    isend = ajFalse;

    while(!isend)
    {
        spos = ajFileResetPos(infs);
	ajReadline(infs,&dbxgcgSeqline);
	while(strncmp(ajStrGetPtr(dbxgcgSeqline),">>>>",4))
	{
	    spos = ajFileResetPos(infs);
	    if(!ajReadline(infs, &dbxgcgSeqline))
	    {
	      ajStrDel(&reflibstr);
	      ajStrDel(&seqlibstr);
	      ajStrDel(&testlibstr);
	      ajDebug("end of file on seq\n");
	      return 1L;
	    }
	}

	ajRegExec(sexp, dbxgcgSeqline);
	ajRegSubI(sexp, 1, &seqlibstr);

	rpos = ajFileResetPos(infr);
	ajReadline(infr, &dbxgcgRefline);

	while(ajStrGetCharFirst(dbxgcgRefline)!='>')
	{
	  rpos = ajFileResetPos(infr);
	  if(!ajReadline(infr, &dbxgcgRefline))
	  {
	    ajDebug("end of file on seq\n");
	    ajDebug("ref ended before seq\n");
	    ajErr("ref ended before seq\n");
	    break;
	  }
	}

	ajRegExec(rexp, dbxgcgRefline);
	ajRegSubI(rexp, 1, &reflibstr);

	if(ajStrCmpLenS(reflibstr, testlibstr, ilen) ||
	   ajStrCmpLenS(seqlibstr, testlibstr, ilen))
	    isend = ajTrue;

	ajDebug("gcgappent %B test: '%S' seq: '%S' ref: '%S'\n",
		isend, testlibstr, seqlibstr, reflibstr);
    }

    ajDebug("gcgappent done at seq: '%S' ref: '%S'\n", seqlibstr, reflibstr);

    ajStrAssignC(libstr,p);

    ajFileSeek(infr, rpos, 0);
    ajFileSeek(infs, spos, 0);

    ajStrDel(&reflibstr);
    ajStrDel(&seqlibstr);
    ajStrDel(&testlibstr);

    return 1L;
}




/* @funcstatic dbxgcg_ParseEmbl ***********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxgcg_ParseEmbl(AjPFile infr,
			       AjPStr *id)
{
    ajint lineType;
    ajlong rpos;

    if(!dbxgcg_embl_typexp)
	dbxgcg_embl_typexp = ajRegCompC("^([A-Z][A-Z]) +");

    if(!dbxgcg_embl_wrdexp)
	dbxgcg_embl_wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbxgcg_embl_verexp)
	dbxgcg_embl_verexp = ajRegCompC("([A-Za-z0-9]+[.][0-9]+)");

    if(!dbxgcg_embl_phrexp)
	dbxgcg_embl_phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbxgcg_embl_taxexp)
	dbxgcg_embl_taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbxgcg_embl_idexp)
	dbxgcg_embl_idexp = ajRegCompC("^ID   ([^ \t;]+)");

    rpos = ajFileResetPos(infr);
    while(ajReadline(infr, &dbxgcgRefline))
    {
	if(ajStrGetCharFirst(dbxgcgRefline) == '>')
	    break;
	
        rpos = ajFileResetPos(infr);

	if(ajRegExec(dbxgcg_embl_typexp, dbxgcgRefline))
	{
	    ajRegSubI(dbxgcg_embl_typexp, 1, &dbxgcgTypstr);
	    if(ajStrMatchC(dbxgcgTypstr, "ID"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(dbxgcgTypstr, "SV"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(dbxgcgTypstr, "AC"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(dbxgcgTypstr, "DE"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(dbxgcgTypstr, "KW"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(dbxgcgTypstr, "OS"))
		lineType = GCGTYPE_TAX;
	    else if(ajStrMatchC(dbxgcgTypstr, "OC"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(dbxgcg_embl_typexp, &dbxgcgTmpline);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(dbxgcg_embl_idexp, dbxgcgRefline);
	    ajRegSubI(dbxgcg_embl_idexp, 1, id);
	    ajDebug("++id '%S'\n", *id);
	    continue;
	}

	if(lineType == GCGTYPE_ACC && accfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_embl_wrdexp, accfield);
	    continue;
	}
	else if(lineType == GCGTYPE_DES && desfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_embl_wrdexp, desfield);
	    continue;
	}
	else if(lineType == GCGTYPE_VER && svfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_embl_verexp, svfield);
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && keyfield)
	{
            embBtreeParseFieldTrim(dbxgcgTmpline, dbxgcg_embl_phrexp, keyfield);
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && orgfield)
	{
            embBtreeParseFieldTrim(dbxgcgTmpline, dbxgcg_embl_taxexp, orgfield);
	    continue;
	}
    }

    if(rpos)
        ajFileSeek(infr, rpos, 0);

    return ajFalse;
}




/* @funcstatic dbxgcg_ParseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxgcg_ParseGenbank(AjPFile infr,
			       AjPStr *id)
{
    ajlong rpos = 0;
    ajint lineType=GCGTYPE_OTHER;

    if(!dbxgcg_genbank_typexp)
	dbxgcg_genbank_typexp = ajRegCompC("^(  )?([A-Z]+)");

    if(!dbxgcg_genbank_morexp)
	dbxgcg_genbank_morexp = ajRegCompC("^            ");

    if(!dbxgcg_genbank_wrdexp)
	dbxgcg_genbank_wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbxgcg_genbank_phrexp)
	dbxgcg_genbank_phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbxgcg_genbank_taxexp)
	dbxgcg_genbank_taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbxgcg_genbank_verexp)
	dbxgcg_genbank_verexp = ajRegCompC("([A-Za-z0-9]+)( +GI:([0-9]+))?");

    while(ajReadline(infr, &dbxgcgRefline))
    {
	if(ajStrGetCharFirst(dbxgcgRefline) == '>')
	    break;
	
        rpos = ajFileResetPos(infr);
	ajStrAssignS(&dbxgcgTmpstr,dbxgcgRefline);

	if(ajRegExec(dbxgcg_genbank_typexp, dbxgcgTmpstr))
	{
	    ajRegSubI(dbxgcg_genbank_typexp, 2, &dbxgcgTypstr);
	    if(ajStrMatchC(dbxgcgTypstr, "LOCUS"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(dbxgcgTypstr, "VERSION"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(dbxgcgTypstr, "ACCESSION"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(dbxgcgTypstr, "DEFINITION"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(dbxgcgTypstr, "KEYWORDS"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(dbxgcgTypstr, "ORGANISM"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(dbxgcg_genbank_typexp, &dbxgcgTmpline);
	    ajDebug("++type line %d\n", lineType);
	}
	else if(lineType != GCGTYPE_OTHER &&
                ajRegExec(dbxgcg_genbank_morexp, dbxgcgRefline))
	{
	    ajRegPost(dbxgcg_genbank_morexp, &dbxgcgTmpline);
	    ajDebug("++more line %d\n", lineType);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(dbxgcg_genbank_wrdexp, dbxgcgTmpline);
	    ajRegSubI(dbxgcg_genbank_wrdexp, 1, id);
	}
	else if(lineType == GCGTYPE_ACC && accfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_genbank_wrdexp, accfield);
	    continue;
	}
	else if(lineType == GCGTYPE_DES && desfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_genbank_wrdexp, desfield);
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && keyfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_genbank_phrexp, keyfield);
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && orgfield)
	{
            embBtreeParseField(dbxgcgTmpline, dbxgcg_genbank_taxexp, orgfield);
	    continue;
	}
	else if(lineType == GCGTYPE_VER && svfield)
	{
            embBtreeParseFieldThird(dbxgcgTmpline, dbxgcg_genbank_verexp,
                                    svfield);
	    continue;
	}

    }

    if(rpos)
	ajFileSeek(infr, rpos, 0);

    return ajFalse;
}




/* @funcstatic dbxgcg_ParsePir ************************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbxgcg_ParsePir(AjPFile infr,
			       AjPStr *id)
{
    ajlong rpos;

    if(!dbxgcg_pir_wrdexp)
	dbxgcg_pir_wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbxgcg_pir_idexp)
	dbxgcg_pir_idexp = ajRegCompC("^>..;([^;.\n\r]+)");

    if(!dbxgcg_pir_phrexp)				/* allow . for "sp." */
	dbxgcg_pir_phrexp = ajRegCompC(" *([^,;\n\r]+)");

    if(!dbxgcg_pir_tax2exp)				/* allow . for "sp." */
	dbxgcg_pir_tax2exp = ajRegCompC(" *([^,;\n\r()]+)");

    if(!dbxgcg_pir_acexp)
	dbxgcg_pir_acexp = ajRegCompC("^C;Accession:");

    if(!dbxgcg_pir_ac2exp)
	dbxgcg_pir_ac2exp = ajRegCompC("([A-Za-z0-9]+)");

    if(!dbxgcg_pir_taxexp)
	dbxgcg_pir_taxexp = ajRegCompC("^C;Species:");

    if(!dbxgcg_pir_keyexp)
	dbxgcg_pir_keyexp = ajRegCompC("^C;Keywords:");

    rpos = ajFileResetPos(infr);

    ajDebug("++id '%S'\n", *id);


    ajReadline(infr, &dbxgcgRefline);
    ajDebug("line-2 '%S'\n", dbxgcgRefline);

    if(desfield)
    {
        embBtreeParseField(dbxgcgRefline, dbxgcg_pir_wrdexp, desfield);
    }

    while(ajStrGetCharFirst(dbxgcgRefline)!='>')
    {
        rpos = ajFileResetPos(infr);
	ajStrAssignS(&dbxgcgTmpstr,dbxgcgRefline);

        if(accfield)
        {
            if(ajRegExec(dbxgcg_pir_acexp, dbxgcgRefline))
            {
                ajRegPost(dbxgcg_pir_acexp, &dbxgcgTmpline);
                embBtreeParseField(dbxgcgTmpline, dbxgcg_pir_ac2exp, accfield);
            }
        }

	if(keyfield)
	{
	    if(ajRegExec(dbxgcg_pir_keyexp, dbxgcgRefline))
	    {
		ajRegPost(dbxgcg_pir_keyexp, &dbxgcgTmpline);
                embBtreeParseFieldTrim(dbxgcgTmpline, dbxgcg_pir_phrexp,
                                       keyfield);
	    }
	}

	if(orgfield)
	{
	    if(ajRegExec(dbxgcg_pir_taxexp, dbxgcgRefline))
	    {
		ajRegPost(dbxgcg_pir_taxexp, &dbxgcgTmpline);
                embBtreeParseFieldTrim(dbxgcgTmpline, dbxgcg_pir_tax2exp,
                                       orgfield);
	    }
	}

	if(!ajReadline(infr, &dbxgcgRefline))
	{
	    rpos = 0;
	    break;
	}
    }

    if(rpos)
	ajFileSeek(infr, rpos, 0);

    return ajFalse;
}

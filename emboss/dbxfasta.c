/* @source dbxfasta application
**
** Index flatfile databases
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


#define FASTATYPE_SIMPLE    1
#define FASTATYPE_IDACC     2
#define FASTATYPE_IDSV      3
#define FASTATYPE_GCGID     4
#define FASTATYPE_GCGIDACC  5
#define FASTATYPE_NCBI      6
#define FASTATYPE_DBID      7
#define FASTATYPE_ACCID     8
#define FASTATYPE_GCGACCID  9

static AjPRegexp dbxfasta_wrdexp = NULL;

static AjPStr dbxfastaac  = NULL;
static AjPStr dbxfastasv  = NULL;
static AjPStr dbxfastagi  = NULL;
static AjPStr dbxfastadb  = NULL;
static AjPStr dbxfastade  = NULL;

static AjBool dbxfasta_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
				 AjPRegexp typeexp, ajint idtype);
static AjBool dbxfasta_ParseFasta(EmbPBtreeEntry entry, AjPRegexp typeexp,
				  ajint idtype, const AjPStr line);
static AjPRegexp dbxfasta_getExpr(const AjPStr idformat, ajint *type);

EmbPBtreeField accfield = NULL;
EmbPBtreeField svfield = NULL;
EmbPBtreeField desfield = NULL;

ajuint idtot = 0;
ajuint acctot = 0;
ajuint svtot = 0;
ajuint destot = 0;




/* @prog dbxfasta ************************************************************
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

    ajulong nentries = 0L;
    ajulong ientries = 0L;
    AjPTime starttime = NULL;
    AjPTime begintime = NULL;
    AjPTime nowtime = NULL;

    AjPRegexp typeexp = NULL;
    ajint idtype = 0;

    ajulong idpricache=0L, idpriread = 0L, idpriwrite = 0L, idprisize= 0L;
    ajulong idseccache=0L, idsecread = 0L, idsecwrite = 0L, idsecsize= 0L;
    ajulong acpricache=0L, acpriread = 0L, acpriwrite = 0L, acprisize= 0L;
    ajulong acseccache=0L, acsecread = 0L, acsecwrite = 0L, acsecsize= 0L;
    ajulong svpricache=0L, svpriread = 0L, svpriwrite = 0L, svprisize= 0L;
    ajulong svseccache=0L, svsecread = 0L, svsecwrite = 0L, svsecsize= 0L;
    ajulong depricache=0L, depriread = 0L, depriwrite = 0L, deprisize= 0L;
    ajulong deseccache=0L, desecread = 0L, desecwrite = 0L, desecsize= 0L;

    embInit("dbxfasta", argc, argv);

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

    typeexp = dbxfasta_getExpr(dbtype,&idtype);

    starttime = ajTimeNewToday();

    ajFmtPrintF(outf, "Processing directory: %S\n", directory);

    for(i=0;i<nfiles;++i)
    {
        begintime = ajTimeNewToday();

	ajListPop(entry->files,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(inf=ajFileNewInNameS(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	
	ajFilenameTrimPath(&tmpstr);
	ajFmtPrintF(outf,"Processing file: %S\n",tmpstr);

	ientries = 0L;

	while(dbxfasta_NextEntry(entry,inf,typeexp,idtype))
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

	    if(desfield)
	    {
                destot += embBtreeIndexSecondary(desfield, entry);
	    }
	}
	
	ajFileClose(&inf);
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
            if(desfield)
                ajBtreeCacheStatsOut(outf, desfield->cache,
                                     &depricache, &deseccache,
                                     &depriread, &desecread,
                                     &depriwrite, &desecwrite,
                                     &deprisize, &desecsize);
        }

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
    if(svfield)
        embBtreeReportField(outf, svfield);
    if(desfield)
        embBtreeReportField(outf, desfield);

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
    
    ajStrDel(&dbxfastaac);
    ajStrDel(&dbxfastasv);
    ajStrDel(&dbxfastagi);
    ajStrDel(&dbxfastade);
    ajStrDel(&dbxfastadb);

    nfields = 0;
    while(fieldarray[nfields])
	ajStrDel(&fieldarray[nfields++]);
    AJFREE(fieldarray);

    ajRegFree(&dbxfasta_wrdexp);
    ajRegFree(&typeexp);

    embExit();

    return 0;
}




/* @funcstatic dbxfasta_NextEntry ********************************************
**
** Parse the next entry from a fasta file
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
** @param [u] typeexp [AjPRegexp] regexp corresponding to idtype
** @param [r] idtype [ajint] the kind of parsing required
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxfasta_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
				 AjPRegexp typeexp, ajint idtype)
{
    static AjBool init = AJFALSE;
    AjPStr line = NULL;
    
    if(!init)
    {
        line = ajStrNew();
        init = ajTrue;
    }

    ajStrAssignC(&line,"");

    while(*MAJSTRGETPTR(line) != '>')
    {
	entry->fpos = ajFileResetPos(inf);
	if(!ajReadlineTrim(inf,&line))
	{
	  ajStrDel(&line);
	  return ajFalse;
	}
    }


    dbxfasta_ParseFasta(entry, typeexp, idtype, line);

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic dbxfasta_getExpr ***********************************************
**
** Compile regular expression
**
** @param [r] idformat [const AjPStr] type of ID line
** @param [w] type [ajint *] numeric type
** @return [AjPRegexp] ajTrue on success.
** @@
******************************************************************************/

static AjPRegexp dbxfasta_getExpr(const AjPStr idformat, ajint *type)
{
    AjPRegexp exp = NULL;

    if(ajStrMatchC(idformat,"simple"))
    {
	*type = FASTATYPE_SIMPLE;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"idacc"))
    {
	*type = FASTATYPE_IDACC;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)+[ \t]+\\(?([A-Za-z0-9_-]+)\\)?");
    }
    else if(ajStrMatchC(idformat,"idsv"))
    {
	*type = FASTATYPE_IDSV;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)+[ \t]+\\(?(([A-Za-z0-9_-]+).[0-9]+)\\)?");
    }
    else if(ajStrMatchC(idformat,"accid"))
    {
	*type = FASTATYPE_ACCID;
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgid"))
    {
	*type = FASTATYPE_GCGID;
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgidacc"))
    {
	*type = FASTATYPE_GCGIDACC;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgaccid"))
    {
	*type = FASTATYPE_GCGACCID;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"ncbi"))
    {
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)"); /* dummy regexp */
	*type = FASTATYPE_NCBI;
    }
    else if(ajStrMatchC(idformat,"dbid"))
    {
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+[ \t]+([A-Za-z0-9_-]+)");
	*type = FASTATYPE_DBID;
    }
    else
        return NULL;

    return exp;
}




/* @funcstatic dbxfasta_ParseFasta ********************************************
**
** Parse the ID, accession from a FASTA format sequence
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] typeexp [AjPRegexp] regular expression
** @param [r] idtype [ajint] type of id line
** @param [r] line [const AjPStr] fasta '>' line
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxfasta_ParseFasta(EmbPBtreeEntry entry, AjPRegexp typeexp,
				  ajint idtype, const AjPStr line)
{

    if(!dbxfasta_wrdexp)
	dbxfasta_wrdexp = ajRegCompC("([A-Za-z0-9]+)");


    if(!ajRegExec(typeexp,line))
    {
	ajDebug("Invalid ID line [%S]",line);
	return ajFalse;
    }

    /*
    ** each case needs to set id, ac, sv, de
    ** using empty values if they are not found
    */
    
    ajStrAssignC(&dbxfastasv, "");
    ajStrAssignC(&dbxfastagi, "");
    ajStrAssignC(&dbxfastadb, "");
    ajStrAssignC(&dbxfastade, "");
    ajStrAssignC(&dbxfastaac, "");
    ajStrAssignC(&entry->id, "");

    switch(idtype)
    {
    case FASTATYPE_SIMPLE:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssignS(&dbxfastaac,entry->id);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_DBID:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssignS(&dbxfastaac,entry->id);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_GCGID:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssignS(&dbxfastaac,entry->id);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_NCBI:
	if(!ajSeqParseNcbi(line,&entry->id,&dbxfastaac,&dbxfastasv,
                           &dbxfastagi,&dbxfastadb,&dbxfastade))
	    return ajFalse;
	break;
    case FASTATYPE_GCGIDACC:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&dbxfastaac);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_GCGACCID:
	ajRegSubI(typeexp,1,&dbxfastaac);
	ajRegSubI(typeexp,2,&entry->id);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_IDACC:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&dbxfastaac);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_IDSV:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&dbxfastasv);
	ajRegSubI(typeexp,3,&dbxfastaac);
	ajRegPost(typeexp, &dbxfastade);
	break;
    case FASTATYPE_ACCID:
	ajRegSubI(typeexp,1,&dbxfastaac);
	ajRegSubI(typeexp,2,&entry->id);
	ajRegPost(typeexp, &dbxfastade);
	break;
    default:
	return ajFalse;
    }

    if(accfield && ajStrGetLen(dbxfastaac))
    {
        embBtreeParseFastaAc(dbxfastaac, accfield);
    }

    if(ajStrGetLen(dbxfastagi))
	ajStrAssignS(&dbxfastasv,dbxfastagi);

    if(svfield && ajStrGetLen(dbxfastasv))
    {
        embBtreeParseFastaSv(dbxfastasv, svfield);
    }
    
    if(desfield && ajStrGetLen(dbxfastade))
    {
        embBtreeParseFastaDe(dbxfastade, desfield);
    }

    return ajTrue;
}

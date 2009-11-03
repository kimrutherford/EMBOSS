/* @source dbifasta application
**
** Index fasta sequence file databases
**
** @author Copyright (C) Peter Rice, Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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

/******************************************************************************
**
** EMBOSS/Staden/EMBLCD indexing
**
** This version reads a flat file database, in fasta format,
** and writes entryname and field (e.g. accession) index files.
**
** It needs to know the format in order to
** parse the entryname and accession number.
**
** To save memory, it is also helpful to know the maximum number of
** entries in the database and the maximum entryname length so that
** space can be preallocated for storage.
**
** Entry names and accession numbers are held in list structures,
** then converted to arrays and sorted.
**
** Multiple input files are allowed.
**
** EMBLCD and Staden index files use different names but have essentially
** the same contents.
**
******************************************************************************/

#include "emboss.h"



#define FASTATYPE_SIMPLE    1
#define FASTATYPE_IDACC     2
#define FASTATYPE_GCGID     3
#define FASTATYPE_GCGIDACC 4
#define FASTATYPE_NCBI      5
#define FASTATYPE_DBID      6
#define FASTATYPE_ACCID     7
#define FASTATYPE_GCGACCID  8




static AjBool dbifasta_ParseFasta(AjPFile libr, ajint *dpos,
				  ajint* maxFieldLen, ajuint* countfield,
				  AjPRegexp idexp,
				  ajuint type, AjPFile* alistfile,
				  AjBool systemsort, AjPStr const * fields);

static EmbPEntry dbifasta_NextFlatEntry(AjPFile libr, ajint ifile,
					AjPRegexp idexp,
					ajuint type, AjBool systemsort,
					AjPStr const * fields,
					ajint* maxFieldLen,
					ajuint* maxidlen, ajuint* countfield,
					AjPFile elistfile,
					AjPFile* alistfile);

static AjPRegexp dbifasta_getExpr(const AjPStr idformat, ajuint *type);


static AjPStr rline  = NULL;
static EmbPEntry dbifastaEntry = NULL;
static AjPRegexp dbifastaWrdexp = NULL;

static AjPStr dbifastaTmpAc  = NULL;
static AjPStr dbifastaTmpSv  = NULL;
static AjPStr dbifastaTmpGi  = NULL;
static AjPStr dbifastaTmpDb  = NULL;
static AjPStr dbifastaTmpDes = NULL;
static AjPStr dbifastaTmpFd  = NULL;
static AjPStr id = NULL;
static AjPList* fdl = NULL;

/* @prog dbifasta *************************************************************
**
** Index a fasta database
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList idlist;
    AjPList* fieldList = NULL;

    AjBool systemsort;
    AjBool cleanup;

    ajuint maxindex;
    ajuint maxidlen = 0;
    ajuint maxlen;

    AjPFile elistfile  = NULL;
    AjPFile* alistfile = NULL;

    AjPStr dbname   = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;
    AjPStr sortopt  = NULL;
    void **entryIds = NULL;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr curfilename = NULL;

    AjPFile libr=NULL;
    AjPStr idformat = NULL;

    EmbPEntry entry;

    ajuint idtype  = 0;

    ajuint idCount = 0;
    ajuint idDone;
    AjPList listInputFiles = NULL;
    void ** inputFiles = NULL;
    ajuint nfiles;
    ajuint ifile;

    ajuint filesize;
    short recsize;
    ajuint maxfilelen = 20;
    char date[4] =
    {
	0,0,0,0
    };

    AjPStr tmpfname = NULL;
    AjPStr* fields  = NULL;

    AjPFile entFile  = NULL;

    AjPStr* divfiles   = NULL;
    AjPRegexp regIdExp      = NULL;
    ajint* maxFieldLen = NULL;

    ajuint ifield  = 0;
    ajuint nfields = 0;

    AjPFile logfile = NULL;
    ajuint* countField = NULL;
    ajuint* fieldTot = NULL;
    ajuint idCountFile = 0;
    ajuint i;

    embInit("dbifasta", argc, argv);

    idformat   = ajAcdGetListSingle("idformat");
    fields     = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    systemsort = ajAcdGetBool("systemsort");
    cleanup    = ajAcdGetBool("cleanup");
    sortopt    = ajAcdGetString("sortoptions");
    maxindex   = ajAcdGetInt("maxindex");
    logfile    = ajAcdGetOutfile("outfile");

    while(fields[nfields])		/* array ends with a NULL */
	nfields++;

    if(nfields)
    {
	AJCNEW(maxFieldLen, nfields);
	AJCNEW0(countField, nfields);
	AJCNEW0(fieldTot, nfields);
	for(ifield=0; ifield < nfields; ifield++)
	    maxFieldLen[ifield] = (ajint)maxindex * -1;

	if(systemsort)
	    AJCNEW(alistfile, nfields);
	else
	{
	    AJCNEW(fieldList, nfields);
	    for(ifield=0; ifield < nfields; ifield++)
		fieldList[ifield] = ajListNew();
	}
    }

    if(ajStrMatchC(datestr, "00/00/00"))
	ajFmtPrintS(&datestr, "%D", ajTimeRefTodayFmt("dbindex"));

    ajStrRemoveWhite(&dbname);		/* used for temp filenames */
    embDbiDateSet(datestr, date);
    idlist = ajListNew();

    regIdExp = dbifasta_getExpr(idformat, &idtype);

    ajDebug("reading '%S/%S'\n", directory, filename);
    ajDebug("writing '%S/'\n", indexdir);

    listInputFiles = embDbiFileListExc(directory, filename, exclude);
    ajListSort(listInputFiles, ajStrVcmp);
    nfiles = ajListToarray(listInputFiles, &inputFiles);
    if(!nfiles)
	ajFatal("No files selected");

    embDbiLogHeader(logfile, dbname, release, datestr,
		     indexdir, maxindex);

    embDbiLogFields(logfile, fields, nfields);
    embDbiLogSource(logfile, directory, filename, exclude,
		    (AjPStr*) inputFiles, nfiles);
    embDbiLogCmdline(logfile);

    AJCNEW0(divfiles, nfiles);

    /*
    ** process each input file, one at a time
    */

    for(ifile=0; ifile < nfiles; ifile++)
    {
	ajStrAssignS(&curfilename,(AjPStr) inputFiles[ifile]);
	embDbiFlatOpenlib(curfilename, &libr);
	ajFileNameTrim(&curfilename);
	if(ajStrGetLen(curfilename) >= maxfilelen)
	    maxfilelen = ajStrGetLen(curfilename) + 1;

	ajDebug("processing filename '%S' ...\n", curfilename);
	ajDebug("processing file '%F' ...\n", libr);
	ajStrAssignS(&divfiles[ifile], curfilename);

	if(systemsort)	 /* elistfile for entries, alist for fields */
	    elistfile = embDbiSortOpen(alistfile, ifile,
				       dbname, fields, nfields);

	idCountFile = 0;
	for(i=0;i<nfields;i++)
	    countField[i] = 0;
	while((entry=dbifasta_NextFlatEntry(libr, ifile,
					    regIdExp, idtype,
					    systemsort, fields, 
					    maxFieldLen, &maxidlen,
					    countField, elistfile,
					    alistfile)))
	{
	    idCountFile++;
	    if(!systemsort)	    /* save the entry data in lists */
		embDbiMemEntry(idlist, fieldList, nfields,
			       entry, ifile);
		entry = NULL;
	}
	idCount += idCountFile;
	if(systemsort)
	{
	    embDbiSortClose(&elistfile, alistfile, nfields);
	    AJFREE(entry);
	}
	else
	{
	    embDbiEntryDel(&dbifastaEntry);
	}
	embDbiLogFile(logfile, curfilename, idCountFile, fields,
		      countField, nfields);
    }

    /*  write the division.lkp file */
    embDbiWriteDivision(indexdir, dbname, release, date,
			maxfilelen, nfiles, divfiles, NULL);

    /* Write the entryname.idx index */
    ajStrAssignC(&tmpfname, "entrynam.idx");
    entFile = ajFileNewOutD(indexdir, tmpfname);

    recsize = maxidlen+10;
    filesize = 300 + (idCount*(ajint)recsize);
    embDbiHeader(entFile, filesize, idCount, recsize, dbname, release, date);

    if(systemsort)
        idDone = embDbiSortWriteEntry(entFile, maxidlen,
				      dbname, nfiles, cleanup, sortopt);
    else			  /* save entries in entryIds array */
    {
        idDone = embDbiMemWriteEntry(entFile, maxidlen,
				     idlist, &entryIds);
	if(idDone != idCount)
	    ajFatal("Duplicates not allowed for in-memory processing");
    }

    embDbiHeaderSize(entFile, 300+(idDone*(ajint)recsize), idDone);
    ajFileClose(&entFile);

    /* Write the fields index files */
    for(ifield=0; ifield < nfields; ifield++)
    {
        if(maxindex)
	    maxlen = maxindex;
	else
	{
	    if(maxFieldLen[ifield] >= 0)
		maxlen = maxFieldLen[ifield];
	    else
		maxlen = - maxFieldLen[ifield];
	}

        if(systemsort)
	    fieldTot[ifield] = embDbiSortWriteFields(dbname, release,
						     date, indexdir,
						     fields[ifield], maxlen,
						     nfiles, idCount,
						     cleanup, sortopt);
	else
	    fieldTot[ifield] = embDbiMemWriteFields(dbname, release,
						    date, indexdir,
						    fields[ifield], maxlen,
						    fieldList[ifield],
						    entryIds);
    }

    embDbiLogFinal(logfile,maxindex, maxFieldLen, fields, fieldTot,
		   nfields, nfiles, idDone, idCount);

    if(systemsort)
	embDbiRmEntryFile(dbname, cleanup);

    ajStrDel(&idformat);
    ajStrDelarray(&fields);
    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&dbname);
    ajStrDel(&release);
    ajStrDel(&datestr);
    ajStrDel(&sortopt);
    ajStrDel(&directory);
    ajStrDel(&indexdir);
    ajStrDel(&tmpfname);
    ajFileClose(&libr);
    ajFileClose(&logfile);

    for(i=0;i<nfields;i++)
    {
	if(systemsort)
	{
	    ajFileClose(&alistfile[i]);
	}
	else
	{
	    ajListMap(fieldList[i], embDbiFieldDelMap, NULL);
	    ajListFree(&fieldList[i]);
	}
    }

    AJFREE(alistfile);
    AJFREE(fieldList);
    AJFREE(maxFieldLen);
    AJFREE(countField);
    AJFREE(fieldTot);

    for(i=0;i<nfiles;i++)
    {
	ajStrDel(&divfiles[i]);
    }

    AJFREE(divfiles);
    AJFREE(inputFiles);

    embDbiEntryDel(&dbifastaEntry);

    ajStrDel(&rline);
    ajStrDel(&id);

    if(fdl)
    {
	for(i=0; i < nfields; i++)
	    ajListFree(&fdl[i]);
	AJFREE(fdl);
    }

    ajListMap(idlist, embDbiEntryDelMap, NULL);
    ajListFree(&idlist);
    ajListstrFreeData(&listInputFiles);
    AJFREE(entryIds);
    ajRegFree(&dbifastaWrdexp);
    ajRegFree(&regIdExp);

    ajStrDel(&dbifastaTmpAc);
    ajStrDel(&dbifastaTmpSv);
    ajStrDel(&dbifastaTmpGi);
    ajStrDel(&dbifastaTmpDb);
    ajStrDel(&dbifastaTmpDes);
    ajStrDel(&dbifastaTmpFd);
    ajStrDel(&curfilename);

    embExit();

    return 0;
}




/* @funcstatic dbifasta_NextFlatEntry *****************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] libr [AjPFile] Database file
** @param [r] ifile [ajint] File number.
** @param [u] idexp [AjPRegexp] Regular expression for id parsing
** @param [r] type [ajuint] type of fasta id.
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] maxidlen [ajuint*] Maximum entry ID length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [u] elistfile [AjPFile] entry file
** @param [u] alistfile [AjPFile*] field data files array
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbifasta_NextFlatEntry(AjPFile libr, ajint ifile,
					AjPRegexp idexp,
					ajuint type, AjBool systemsort,
					AjPStr const * fields,
					ajint* maxFieldLen,
					ajuint* maxidlen, ajuint* countfield,
					AjPFile elistfile, AjPFile* alistfile)
{
    ajint ir;
    ajint is = 0;
    char* token;
    ajint i;
    static ajint nfields;
    ajint ifield;

    if(!fdl)
    {
	nfields = 0;
	while(fields[nfields])
	    nfields++;
	if(nfields)
	    AJCNEW(fdl, nfields);
	for(i=0; i < nfields; i++)
	{
	    fdl[i] = ajListNew();
	}
    }

    if(!dbifastaEntry || !systemsort)
	dbifastaEntry = embDbiEntryNew(nfields);

    if(!dbifasta_ParseFasta(libr, &ir, maxFieldLen,
			    countfield, idexp, type,
			    alistfile, systemsort, fields))
	return NULL;

    /* id to dbifastaEntry->entry */
    if(ajStrGetLen(id) > *maxidlen)
	*maxidlen = ajStrGetLen(id);

    if(systemsort)
	ajFmtPrintF(elistfile, "%S %d %d %d\n", id, ir, is, ifile+1);
    else
    {
	dbifastaEntry->entry   = ajCharNewS(id);
	dbifastaEntry->rpos    = ir;
	dbifastaEntry->spos    = is;
	dbifastaEntry->filenum = ifile+1;

	/* field tokens as list, then move to ret->field */
	for(ifield=0; ifield < nfields; ifield++)
	{
	    dbifastaEntry->nfield[ifield] = ajListGetLength(fdl[ifield]);

	    if(dbifastaEntry->nfield[ifield])
	    {
		AJCNEW(dbifastaEntry->field[ifield],
		       dbifastaEntry->nfield[ifield]);

		i = 0;
		while(ajListPop(fdl[ifield], (void**) &token))
		    dbifastaEntry->field[ifield][i++] = token;
	    }
	    else
		dbifastaEntry->field[ifield] = NULL;
	}
    }

    return dbifastaEntry;
}




/* @funcstatic dbifasta_getExpr ***********************************************
**
** Compile regular expression
**
** @param [r] idformat [const AjPStr] type of ID line
** @param [w] type [ajuint *] numeric type
** @return [AjPRegexp] ajTrue on success.
** @@
******************************************************************************/

static AjPRegexp dbifasta_getExpr(const AjPStr idformat, ajuint *type)
{
    AjPRegexp retexp = NULL;

    if(ajStrMatchC(idformat,"simple"))
    {
	*type = FASTATYPE_SIMPLE;
	retexp   = ajRegCompC("^>([.A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"idacc"))
    {
	*type = FASTATYPE_IDACC;
	retexp   = ajRegCompC("^>([.A-Za-z0-9_-]+)+[ \t]+\\(?([A-Za-z0-9_-]+)\\)?");
    }
    else if(ajStrMatchC(idformat,"accid"))
    {
	*type = FASTATYPE_ACCID;
	retexp   = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgid"))
    {
	*type = FASTATYPE_GCGID;
	retexp   = ajRegCompC("^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgidacc"))
    {
	*type = FASTATYPE_GCGIDACC;
	retexp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgaccid"))
    {
	*type = FASTATYPE_GCGACCID;
	retexp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"ncbi"))
    {
	*type = FASTATYPE_NCBI;
	retexp   = ajRegCompC("^>([A-Za-z0-9_-]+)"); /* dummy regexp */
    }
    else if(ajStrMatchC(idformat,"dbid"))
    {
	*type = FASTATYPE_DBID;
	retexp   = ajRegCompC("^>[A-Za-z0-9_-]+[ \t]+([A-Za-z0-9_-]+)");
    }
    else
	return NULL;

    return retexp;
}




/* @funcstatic dbifasta_ParseFasta ********************************************
**
** Parse the ID, accession from a FASTA format sequence
**
** @param [u] libr [AjPFile] Input database file
** @param [w] dpos [ajint*] Byte offset
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [u] idexp [AjPRegexp] regular expression
** @param [r] type [ajuint] type of id line
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbifasta_ParseFasta(AjPFile libr, ajint* dpos,
				  ajint* maxFieldLen, ajuint* countfield,
				  AjPRegexp idexp,
				  ajuint type, AjPFile* alistfile,
				  AjBool systemsort, AjPStr const * fields)
{
    char* fd;
    ajint ipos;
    static AjPStr tstr = NULL;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else
		ajWarn("EMBL parsing unknown field '%S' ignored",
		       fields[numFields]);

	    numFields++;
	}
	reset = ajFalse;
    }

    if(!dbifastaWrdexp)
	dbifastaWrdexp = ajRegCompC("([A-Za-z0-9]+)");

    if(!tstr)
	tstr = ajStrNew();

    *dpos = ajFileTell(libr);
    ajFileGets(libr, &rline);
    if(!ajRegExec(idexp,rline))
    {
	ajStrDelStatic(&dbifastaTmpAc);
	ajDebug("Invalid ID line [%S]",rline);
	return ajFalse;
    }

    /*
    ** each case needs to set id, tmpac, tmpsv, tmpdes
    ** using empty values if they are not found
    */

    ajStrAssignC(&dbifastaTmpSv, "");
    ajStrAssignC(&dbifastaTmpGi, "");
    ajStrAssignC(&dbifastaTmpDb, "");
    ajStrAssignC(&dbifastaTmpDes, "");
    ajStrAssignC(&dbifastaTmpAc, "");
    ajStrAssignC(&id, "");

    switch(type)
    {
    case FASTATYPE_SIMPLE:
	ajRegSubI(idexp,1,&id);
	ajStrAssignS(&dbifastaTmpAc,id);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    case FASTATYPE_DBID:
	ajRegSubI(idexp,1,&id);
	ajStrAssignS(&dbifastaTmpAc,id);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    case FASTATYPE_GCGID:
	ajRegSubI(idexp,1,&id);
	ajStrAssignS(&dbifastaTmpAc,id);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    case FASTATYPE_NCBI:
	if(!ajSeqParseNcbi(rline,&id,&dbifastaTmpAc,&dbifastaTmpSv,
			   &dbifastaTmpGi,&dbifastaTmpDb,&dbifastaTmpDes))
	{
	    ajStrDelStatic(&dbifastaTmpAc);
	    return ajFalse;
	}
	break;
    case FASTATYPE_GCGIDACC:
	ajRegSubI(idexp,1,&id);
	ajRegSubI(idexp,2,&dbifastaTmpAc);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    case FASTATYPE_GCGACCID:
	ajRegSubI(idexp,1,&dbifastaTmpAc);
	ajRegSubI(idexp,2,&id);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    case FASTATYPE_IDACC:
	ajRegSubI(idexp,1,&id);
	ajRegSubI(idexp,2,&dbifastaTmpAc);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    case FASTATYPE_ACCID:
	ajRegSubI(idexp,1,&dbifastaTmpAc);
	ajRegSubI(idexp,2,&id);
	ajRegPost(idexp, &dbifastaTmpDes);
	break;
    default:
	ajStrDelStatic(&dbifastaTmpAc);
	return ajFalse;
    }

    ajStrFmtUpper(&id);
    ajStrFmtUpper(&dbifastaTmpAc);

    if(accfield >= 0)
	embDbiMaxlen(&dbifastaTmpAc, &maxFieldLen[accfield]);
    if(svnfield >= 0)
    {
	embDbiMaxlen(&dbifastaTmpSv, &maxFieldLen[svnfield]);
	embDbiMaxlen(&dbifastaTmpGi, &maxFieldLen[svnfield]);
    }

    if(systemsort)
    {
	if(accfield >= 0 && ajStrGetLen(dbifastaTmpAc))
	{
	    countfield[accfield]++;
	    ajFmtPrintF(alistfile[accfield],"%S %S\n",id,dbifastaTmpAc);
	}
	if(svnfield >= 0 && ajStrGetLen(dbifastaTmpSv))
	{
	    countfield[svnfield]++;
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", id, dbifastaTmpSv);
	}
	if(svnfield >= 0 && ajStrGetLen(dbifastaTmpGi))
	{
	    countfield[svnfield]++;
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", id, dbifastaTmpGi);
	}
	if(desfield >= 0 && ajStrGetLen(dbifastaTmpDes))
	    while(ajRegExec(dbifastaWrdexp, dbifastaTmpDes))
	    {
		ajRegSubI(dbifastaWrdexp, 1, &dbifastaTmpFd);
		embDbiMaxlen(&dbifastaTmpFd, &maxFieldLen[desfield]);
		ajStrFmtUpper(&dbifastaTmpFd);
		ajDebug("++des '%S' tmpdes '%S\n",
			dbifastaTmpFd, dbifastaTmpDes);
		countfield[desfield]++;
		ajFmtPrintF(alistfile[desfield], "%S %S\n",
			    id, dbifastaTmpFd);
		ajRegPost(dbifastaWrdexp, &dbifastaTmpDes);
	    }
    }
    else
    {
	if(accfield >= 0 && ajStrGetLen(dbifastaTmpAc))
	{
	    fd = ajCharNewS(dbifastaTmpAc);
	    ajListPushAppend(fdl[accfield],fd);
	    countfield[accfield]++;
	}

	if(svnfield >= 0 && ajStrGetLen(dbifastaTmpSv))
	{
	    fd = ajCharNewS(dbifastaTmpSv);
	    ajListPushAppend(fdl[svnfield], fd);
	    countfield[svnfield]++;
	}

	if(svnfield >= 0 && ajStrGetLen(dbifastaTmpGi))
	{
	    fd = ajCharNewS(dbifastaTmpGi);
	    ajListPushAppend(fdl[svnfield], fd);
	    countfield[svnfield]++;
	}

	if(desfield >= 0 && ajStrGetLen(dbifastaTmpDes))
	    while(ajRegExec(dbifastaWrdexp, dbifastaTmpDes))
	    {
		ajRegSubI(dbifastaWrdexp, 1, &dbifastaTmpFd);
		embDbiMaxlen(&dbifastaTmpFd, &maxFieldLen[desfield]);
		ajStrFmtUpper(&dbifastaTmpFd);
		ajDebug("++des '%S' tmpdes: '%S'\n",
			dbifastaTmpFd, dbifastaTmpDes);
		fd = ajCharNewS(dbifastaTmpFd);
		ajListPushAppend(fdl[desfield], fd);
		countfield[desfield]++;
		ajRegPost(dbifastaWrdexp, &dbifastaTmpDes);
	    }
    }

    ipos = ajFileTell(libr);

    while(ajFileGets(libr, &rline))
    {
	if(ajStrGetCharFirst(rline) == '>')
	{
	    ajFileSeek(libr, ipos, 0);
	    return ajTrue;
	}
	ipos = ajFileTell(libr);
    }

    ajFileSeek(libr, ipos, 0);		/* end of file reached */

    return ajTrue;
}

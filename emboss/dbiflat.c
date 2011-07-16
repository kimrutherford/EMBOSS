/* @source dbiflat application
**
** Index flatfile databases
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
** This version reads a flat file database,
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

#define FLATTYPE_OTHER 0
#define FLATTYPE_ID 1
#define FLATTYPE_ACC 2
#define FLATTYPE_DES 3
#define FLATTYPE_KEY 4
#define FLATTYPE_TAX 5
#define FLATTYPE_VER 6

static AjPStr rline = NULL;
static AjPStr id = NULL;
static AjPStr tmpstr = NULL;
static AjPStr tmpline = NULL;
static AjPStr tmpfd   = NULL;
static AjPStr typStr  = NULL;

static AjPRegexp regEmblType = NULL;
static AjPRegexp regEmblId  = NULL;
static AjPRegexp regEmblAcc = NULL;
static AjPRegexp regEmblWrd = NULL;
static AjPRegexp regEmblVer = NULL;
static AjPRegexp regEmblPhr = NULL;
static AjPRegexp regEmblTax = NULL;
static AjPRegexp regEmblEnd = NULL;

static AjPRegexp regGbType = NULL;
static AjPRegexp regGbMore = NULL;
static AjPRegexp regGbWrd = NULL;
static AjPRegexp regGbPhr = NULL;
static AjPRegexp regGbTax = NULL;
static AjPRegexp regGbVer = NULL;
static AjPRegexp regGbEnd = NULL;

static AjPRegexp regRefseqTyp = NULL;
static AjPRegexp regRefseqMore = NULL;
static AjPRegexp regRefseqId = NULL;
static AjPRegexp regRefseqWrd = NULL;
static AjPRegexp regRefseqPhr = NULL;
static AjPRegexp regRefseqTax = NULL;
static AjPRegexp regRefseqVer = NULL;
static AjPRegexp regRefseqEnd = NULL;

static EmbPEntry dbiflatEntry = NULL;

static AjPList* fdl  = NULL;

static AjBool dbiflat_ParseEmbl(AjPFile libr, AjPFile* alistfile,
				AjBool systemsort, AjPStr* fields,
				ajint* maxFieldLen, ajuint* countfield,
				ajint *dpos, AjPStr* myid, AjPList* acl);
static AjBool dbiflat_ParseGenbank(AjPFile libr, AjPFile* alistfile,
				   AjBool systemsort, AjPStr* fields,
				   ajint* maxFieldLen, ajuint* countfield,
				   ajint *dpos, AjPStr* myid, AjPList* acl);
static AjBool dbiflat_ParseRefseq(AjPFile libr, AjPFile* alistfile,
				  AjBool systemsort, AjPStr* fields,
				  ajint* maxFieldLen, ajuint* countfield,
				  ajint *dpos, AjPStr* myid, AjPList* acl);




/* @datastatic DbiflatPParser *************************************************
**
** Parser definition structure
**
** @alias DbiflatSParser
** @alias DbiflatOParser
**
** @attr Name [const char*] Parser name
** @attr Parser [(AjBool*)] Parser function
** @@
******************************************************************************/

typedef struct DbiflatSParser
{
    const char* Name;
    AjBool (*Parser) (AjPFile libr, AjPFile* alistfile,
		      AjBool systemsort, AjPStr* fields,
		      ajint* maxFieldLen, ajuint* countfield,
		      ajint *dpos, AjPStr* myid, AjPList* acl);
} DbiflatOParser;
#define DbiflatPParser DbiflatOParser*




static DbiflatOParser parser[] =
{
    {"EMBL", dbiflat_ParseEmbl},
    {"SWISS", dbiflat_ParseEmbl},
    {"GB", dbiflat_ParseGenbank},
    {"REFSEQ", dbiflat_ParseRefseq},
    {NULL, NULL}
};



static EmbPEntry dbiflat_NextFlatEntry(AjPFile libr, ajuint ifile,
				       const AjPStr idformat,
				       AjBool systemsort,
				       AjPStr* fields, ajint* maxFieldLen,
				       ajuint* maxidlen, ajuint* countfield,
				       AjPFile elistfile, AjPFile* alistfile);





/* @prog dbiflat **************************************************************
**
** Index a flat file database
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
    AjPFile libr = NULL;
    AjPStr idformat = NULL;

    EmbPEntry entry;

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

    AjPFile entFile = NULL;

    AjPStr* divfiles   = NULL;
    ajint* maxFieldLen = NULL;

    ajuint ifield  = 0;
    ajuint nfields = 0;

    AjPFile logfile = NULL;
    ajuint* countField = NULL;
    ajuint* fieldTot = NULL;
    ajuint idCountFile = 0;
    ajuint i;

    embInit("dbiflat", argc, argv);

    idformat   = ajAcdGetListSingle("idformat");
    fields     = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    systemsort = ajAcdGetBoolean("systemsort");
    cleanup    = ajAcdGetBoolean("cleanup");
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
	    maxFieldLen[ifield] = (ajint)maxindex * -1; /* -maxindex illegal */

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

    ajDebug("reading '%S/%S'\n", directory, filename);
    ajDebug("writing '%S/'\n", indexdir);

    listInputFiles = embDbiFileListExc(directory, filename, exclude);
    ajListSort(listInputFiles, ajStrVcmp);
    nfiles = ajListToarray(listInputFiles, &inputFiles);
    if(!nfiles)
        ajDie("No input files in '%S' matched filename '%S'",
              directory, filename);

    embDbiLogHeader(logfile, dbname, release, datestr,
		     indexdir, maxindex);

    embDbiLogFields(logfile, fields, nfields);
    embDbiLogSource(logfile, directory, filename, exclude,
		    (AjPStr*) inputFiles, nfiles);
    embDbiLogCmdline(logfile);

    AJCNEW0(divfiles, nfiles);

    /* process each input file, one at a time */
    for(ifile=0; ifile < nfiles; ifile++)
    {
	ajStrAssignS(&curfilename, (AjPStr) inputFiles[ifile]);
	embDbiFlatOpenlib(curfilename, &libr);
	ajFilenameTrimPath(&curfilename);
	if(ajStrGetLen(curfilename) >= maxfilelen)
	    maxfilelen = ajStrGetLen(curfilename) + 1;

	ajDebug("processing file '%F' ...\n", libr);
	ajStrAssignS(&divfiles[ifile], curfilename);

	if(systemsort)	 /* elistfile for entries, alist for fields */
	    elistfile = embDbiSortOpen(alistfile, ifile,
				       dbname, fields, nfields);

	idCountFile = 0;
	for(i=0;i<nfields;i++)
	    countField[i] = 0;
	while((entry=dbiflat_NextFlatEntry(libr, ifile, idformat,
					   systemsort, fields, maxFieldLen,
					   &maxidlen, countField,
					   elistfile, alistfile)))
	{
	    idCountFile++;

	    if(!systemsort)	    /* save the entry data in lists */
	    {
		embDbiMemEntry(idlist, fieldList, nfields, entry, ifile);
		entry = NULL;
	    }
	}
	idCount += idCountFile;
	if(systemsort)
	{
	    embDbiSortClose(&elistfile, alistfile, nfields);
	    AJFREE(entry);
	}
	else
	{
	    embDbiEntryDel(&dbiflatEntry);
	}
	embDbiLogFile(logfile, curfilename, idCountFile, fields,
		      countField, nfields);
    }


    embDbiWriteDivision(indexdir, dbname, release, date,
			maxfilelen, nfiles, divfiles, NULL);

    /* Write the entryname.idx index */
    ajStrAssignC(&tmpfname, "entrynam.idx");
    entFile = ajFileNewOutNamePathS(tmpfname, indexdir);

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
    {
	embDbiRmEntryFile(dbname, cleanup);
    }

    ajStrDel(&dbname);
    ajStrDel(&release);
    ajStrDel(&datestr);
    ajStrDel(&sortopt);
    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&directory);
    ajStrDel(&indexdir);
    ajFileClose(&libr);
    ajFileClose(&logfile);
    ajStrDel(&idformat);
    ajStrDelarray(&fields);

    ajStrDel(&tmpfname);
    ajFileClose(&elistfile);


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

    ajRegFree(&regEmblType);
    ajRegFree(&regEmblId);
    ajRegFree(&regEmblAcc);
    ajRegFree(&regEmblWrd);
    ajRegFree(&regEmblVer);
    ajRegFree(&regEmblPhr);
    ajRegFree(&regEmblTax);
    ajRegFree(&regEmblEnd);

    ajRegFree(&regGbType);
    ajRegFree(&regGbMore);
    ajRegFree(&regGbWrd);
    ajRegFree(&regGbPhr);
    ajRegFree(&regGbTax);
    ajRegFree(&regGbVer);
    ajRegFree(&regGbEnd);

    ajRegFree(&regRefseqTyp);
    ajRegFree(&regRefseqMore);
    ajRegFree(&regRefseqId);
    ajRegFree(&regRefseqWrd);
    ajRegFree(&regRefseqTax);
    ajRegFree(&regRefseqVer);
    ajRegFree(&regRefseqEnd);

    embDbiEntryDel(&dbiflatEntry);

    ajStrDel(&rline);
    ajStrDel(&tmpfd);
    ajStrDel(&tmpline);
    ajStrDel(&tmpstr);
    ajStrDel(&typStr);
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
    ajStrDel(&curfilename);

    embExit();

    return 0;
}




/* @funcstatic dbiflat_NextFlatEntry ******************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] libr [AjPFile] Database file
** @param [r] ifile [ajuint] File number.
** @param [r] idformat [const AjPStr] Format to be used
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [u] fields [AjPStr*] Fields to be indexed
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] maxidlen [ajuint*] Maximum entry ID length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [u] elistfile [AjPFile] entry file
** @param [u] alistfile [AjPFile*] field data files array
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbiflat_NextFlatEntry(AjPFile libr, ajuint ifile,
				       const AjPStr idformat,
				       AjBool systemsort,
				       AjPStr* fields, ajint* maxFieldLen,
				       ajuint* maxidlen, ajuint* countfield,
				       AjPFile elistfile, AjPFile* alistfile)
{
    ajint ir;
    ajint is = 0;
    char* token;
    ajint i;
    static ajint called  = 0;
    static ajint iparser = -1;
    static ajint nfields;
    ajint ifield;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(idformat, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("idformat '%S' unknown", idformat);
    }

    if(!fdl)
    {
	nfields = 0;
	while(fields[nfields])
	    nfields++;
	if(nfields)
	    AJCNEW(fdl, nfields);
	for(i=0; i < nfields; i++)
	    fdl[i] = ajListNew();
    }

    if(!dbiflatEntry || !systemsort)
	dbiflatEntry = embDbiEntryNew(nfields);

    if(!parser[iparser].Parser(libr, alistfile, systemsort, fields,
			       maxFieldLen, countfield, &ir, &id, fdl))
	return NULL;

    /* id to ret->entry */
    if(ajStrGetLen(id) > *maxidlen)
	*maxidlen = ajStrGetLen(id);

    if(systemsort)
	ajFmtPrintF(elistfile, "%S %d %d %d\n", id, ir, is, ifile+1);
    else
    {
	dbiflatEntry->entry   = ajCharNewS(id);
	dbiflatEntry->rpos    = ir;
	dbiflatEntry->spos    = is;
	dbiflatEntry->filenum = ifile+1;

	/* field tokens as list, then move to dbiflatEntry->field */
	for(ifield=0; ifield < nfields; ifield++)
	{
	    dbiflatEntry->nfield[ifield] = ajListGetLength(fdl[ifield]);

	    if(dbiflatEntry->nfield[ifield])
	    {
	        AJCNEW(dbiflatEntry->field[ifield],
		       dbiflatEntry->nfield[ifield]);

		i = 0;
		while(ajListPop(fdl[ifield],(void**) &token))
		    dbiflatEntry->field[ifield][i++] = token;
	    }
	    else
	        dbiflatEntry->field[ifield] = NULL;
	}
    }

    return dbiflatEntry;
}




/* @funcstatic dbiflat_ParseEmbl **********************************************
**
** Parse the ID, accession from an EMBL entry.
**
** Reads to the end of the entry and then returns.
**
** @param [u] libr [AjPFile] Input database file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [w] fields [AjPStr*] Fields required
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] dpos [ajint*] Byte offset
** @param [w] myid [AjPStr*] ID
** @param [w] myfdl [AjPList*] Lists of field values
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbiflat_ParseEmbl(AjPFile libr, AjPFile* alistfile,
				AjBool systemsort, AjPStr* fields,
				ajint* maxFieldLen, ajuint* countfield,
				ajint* dpos, AjPStr* myid,
				AjPList* myfdl)
{
    AjPStr tmpacnum = NULL;
    char* fd;
    ajint lineType;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint keyfield = -1;
    static ajint taxfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;
    AjBool svndone = ajFalse;
    AjBool done = ajFalse;
    ajint i = 0;
    ajint lo;
    ajint hi;
    ajint fieldwidth;
    AjPStr tmpac = NULL;
    AjPStr format = NULL;
    AjPStr prefix = NULL;
    const char* p;
    const char* q;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = keyfield = taxfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    countfield[numFields]=0;
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "key"))
		keyfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "org"))
		taxfield=numFields;
	    else
		ajWarn("EMBL parsing unknown field '%S' ignored",
		       fields[numFields]);
	    numFields++;
	}

	reset = ajFalse;
    }

    if(!regEmblType)
	regEmblType = ajRegCompC("^([A-Z][A-Z]) +");

    if(!regEmblAcc)
	regEmblAcc = ajRegCompC("([A-Za-z0-9-]+)");

    if(!regEmblWrd)
	regEmblWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!regEmblVer)
	regEmblVer = ajRegCompC("([A-Za-z0-9_.]+)");

    if(!regEmblPhr)
	regEmblPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!regEmblTax)
	regEmblTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!regEmblId)
	regEmblId = ajRegCompC("^ID   ([^\\s;]+)(;\\s+SV\\s+(\\d+))?");

    if(!regEmblEnd)
	regEmblEnd = ajRegCompC("^//");

    *dpos = (ajint) ajFileResetPos(libr); /* Lossy cast */

    while(ajReadline(libr, &rline))
    {
	if(ajRegExec(regEmblEnd, rline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(regEmblType, rline))
	{
	    ajRegSubI(regEmblType, 1, &typStr);
	    if(ajStrMatchC(typStr, "ID"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(typStr, "SV") ||
		    ajStrMatchC(typStr, "IV")) /* emblcds database */
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(typStr, "AC") ||
		    ajStrMatchC(typStr, "PA")) /* emblcds database */
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DE"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(typStr, "KW"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(typStr, "OS"))
		lineType = FLATTYPE_TAX;
	    else if(ajStrMatchC(typStr, "OC"))
		lineType = FLATTYPE_TAX;
	    else
		lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(regEmblType, &tmpline);
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)
	{
	    ajRegExec(regEmblId, rline);
	    ajRegSubI(regEmblId, 1, myid);
	    ajStrFmtUpper(myid);
	    ajDebug("++id '%S'\n", *myid);
	    ajRegSubI(regEmblId, 3, &tmpfd);
	    if(svnfield >= 0 && ajStrGetLen(tmpfd))
	    {
		ajStrFmtUpper(&tmpfd);
		ajStrInsertK(&tmpfd, 0, '.');
		ajStrInsertS(&tmpfd, 0, *myid);
		/*ajDebug("++sv '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;
	    }
	    continue;
	}

	if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(regEmblAcc, tmpline))
	    {
		ajRegSubI(regEmblAcc, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++acc '%S'\n", tmpfd);*/

		if(!tmpacnum)
		    ajStrAssignS(&tmpacnum, tmpfd);

		if((p=strchr(MAJSTRGETPTR(tmpfd),(int)'-')))
		{
		    q = p;
		    while(isdigit((int)*(--q)));
		    ++q;
		    ajStrAssignSubC(&tmpstr,q,0,(ajint)(p-q-1));
		    ajStrToInt(tmpstr,&lo);
		    fieldwidth = (ajint) (p-q);
		    ajFmtPrintS(&format,"%%S%%0%dd",fieldwidth);

		    ++p;
		    q = p;
		    while(!isdigit((int)*q))
			++q;
		    sscanf(q,"%d",&hi);
		    ajStrAssignSubC(&prefix,p,0,(ajint)(q-p-1));
	    
		    if(systemsort)
		    {
			for(i=lo;i<=hi;++i)
			{
			    ajFmtPrintS(&tmpac,MAJSTRGETPTR(format),prefix,i);
			    embDbiMaxlen(&tmpac, &maxFieldLen[accfield]);
			    countfield[accfield]++;
			    ajFmtPrintF(alistfile[accfield],
					"%S %S\n", *myid, tmpac);
			}
			ajStrDel(&tmpac);
		    }
		    else
		    {
			for(i=lo;i<=hi;++i)
			{
			    ajFmtPrintS(&tmpac,MAJSTRGETPTR(format),prefix,i);
			    embDbiMaxlen(&tmpac, &maxFieldLen[accfield]);
			    countfield[accfield]++;
			    fd = ajCharNewS(tmpac);
			    ajListPushAppend(myfdl[accfield], fd);
			}
			ajStrDel(&tmpac);
		    }
		    ajStrDel(&format);
		    ajStrDel(&prefix);
		}
		else {
		    embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);

		    countfield[accfield]++;
		    if(systemsort)
			ajFmtPrintF(alistfile[accfield],
				    "%S %S\n", *myid, tmpfd);
		    else
		    {
			fd = ajCharNewS(tmpfd);
			ajListPushAppend(myfdl[accfield], fd);
		    }
		}
		ajRegPost(regEmblAcc, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(regEmblWrd, tmpline))
	    {
		ajRegSubI(regEmblWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++des '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(regEmblWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    while(ajRegExec(regEmblVer, tmpline))
	    {
		ajRegSubI(regEmblVer, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++sv '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		ajRegPost(regEmblVer, &tmpstr);	
                ajStrAssignS(&tmpline, tmpstr);
            }
	    svndone = ajTrue;
	    continue;
	}
	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(regEmblPhr, tmpline))
	    {
		ajRegSubI(regEmblPhr, 1, &tmpfd);
		ajRegPost(regEmblPhr, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++key '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(regEmblTax, tmpline))
	    {
		ajRegSubI(regEmblTax, 1, &tmpfd);
		ajRegPost(regEmblTax, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrFmtUpper(&tmpfd);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		/*ajDebug("++tax '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[taxfield], fd);
		}
	    }
	    continue;
	}
    }

    if(!done)
	return ajFalse;

    if(svnfield >= 0 && !svndone && tmpacnum)
    {
	ajFmtPrintS(&tmpfd, "%S.0", tmpacnum);
	embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
	else
	{
	    fd = ajCharNewS(tmpfd);
	    ajListPushAppend(myfdl[svnfield], fd);
	}
    }

    ajStrDel(&tmpacnum);

    return ajTrue;
}




/* @funcstatic dbiflat_ParseGenbank *******************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [u] libr [AjPFile] Input database file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [w] fields [AjPStr*] Fields required
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] dpos [ajint*] Byte offset
** @param [w] myid [AjPStr*] ID
** @param [w] myfdl [AjPList*] Lists of field values
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbiflat_ParseGenbank(AjPFile libr, AjPFile* alistfile,
				   AjBool systemsort, AjPStr* fields,
				   ajint* maxFieldLen, ajuint* countfield,
				   ajint* dpos, AjPStr* myid,
				   AjPList* myfdl)
{
    ajint lineType  = FLATTYPE_OTHER;
    AjPStr tmpacnum = NULL;
    char* fd;
    ajlong ipos = 0;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint keyfield = -1;
    static ajint taxfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;
    AjBool done = ajFalse;
    AjBool svndone = ajFalse;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = keyfield = taxfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    countfield[numFields]=0;
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "key"))
		keyfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "org"))
		taxfield=numFields;
	    else
		ajWarn("GenBank parsing unknown field '%S' ignored",
		       fields[numFields]);

	    numFields++;
	}
	reset = ajFalse;
    }

    if(!regGbType)
	regGbType = ajRegCompC("^(  )?([A-Z]+)");

    if(!regGbMore)
	regGbMore = ajRegCompC("^            ");

    if(!regGbWrd)
	regGbWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!regGbPhr)
	regGbPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!regGbTax)
	regGbTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!regGbVer)
	regGbVer = ajRegCompC("([A-Za-z0-9.]+)( +GI:([0-9]+))?");
    if(!regGbEnd)
	regGbEnd = ajRegCompC("^//");

    ipos = ajFileResetPos(libr);

    while(ajReadline(libr, &rline))
    {
	if(ajRegExec(regGbEnd, rline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(regGbType, rline))
	{
	    ajRegSubI(regGbType, 2, &typStr);
	    if(ajStrMatchC(typStr, "LOCUS"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(typStr, "VERSION"))
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(typStr, "ACCESSION"))
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DEFINITION"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(typStr, "KEYWORDS"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(typStr, "ORGANISM"))
		lineType = FLATTYPE_TAX;
	    else lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(regGbType, &tmpline);
	    /*ajDebug("++type line %d\n", lineType);*/
	}
	else if(lineType != FLATTYPE_OTHER && ajRegExec(regGbMore, rline))
	{
	    ajRegPost(regGbMore, &tmpline);
	    /*ajDebug("++more line %d\n", lineType);*/
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)
	{
	    ajRegExec(regGbWrd, tmpline);
	    ajRegSubI(regGbWrd, 1, myid);
	    *dpos = (ajint) ipos; /* Lossy cast */
	}

	else if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(regGbWrd, tmpline))
	    {
		ajRegSubI(regGbWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++acc '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[accfield], fd);
		}
		ajRegPost(regGbWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(regGbWrd, tmpline))
	    {
	        ajRegSubI(regGbWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++des '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield],
				"%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(regGbWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(regGbPhr, tmpline))
	    {
	        ajRegSubI(regGbPhr, 1, &tmpfd);
		ajRegPost(regGbPhr, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++key '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield],
				"%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(regGbTax, tmpline))
	    {
	        ajRegSubI(regGbTax, 1, &tmpfd);
		ajRegPost(regGbTax, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++tax '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield],
				"%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[taxfield], fd);
		}
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    if(ajRegExec(regGbVer, tmpline))
	    {
		ajRegSubI(regGbVer, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++ver '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;

		ajRegSubI(regGbVer, 3, &tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++ver gi: '%S'\n", tmpfd);*/

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
	    }
	    continue;
	}

	ipos = ajFileResetPos(libr);
    }

    if(!done)
	return ajFalse;

    if(svnfield >= 0 && !svndone && tmpacnum)
    {
	ajFmtPrintS(&tmpfd, "%S.0", tmpacnum);
	embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
	else
	{
	    fd = ajCharNewS(tmpfd);
	    ajListPushAppend(myfdl[svnfield], fd);
	}
    }

    ajStrDel(&tmpacnum);

    return ajTrue;
}




/* @funcstatic dbiflat_ParseRefseq ********************************************
**
** Parse the ID, accession from an NCBI REFSEQ entry
**
** @param [u] libr [AjPFile] Input database file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [w] fields [AjPStr*] Fields required
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] dpos [ajint*] Byte offset
** @param [w] myid [AjPStr*] ID
** @param [w] myfdl [AjPList*] Lists of field values
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbiflat_ParseRefseq(AjPFile libr, AjPFile* alistfile,
				  AjBool systemsort, AjPStr* fields,
				  ajint* maxFieldLen, ajuint* countfield,
				  ajint* dpos, AjPStr* myid,
				  AjPList* myfdl)
{
    ajint lineType = FLATTYPE_OTHER;
    AjPStr tmpacnum = NULL;
    char* fd;
    ajlong ipos = 0;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint keyfield = -1;
    static ajint taxfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;
    AjBool done = ajFalse;
    AjBool svndone = ajFalse;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = keyfield = taxfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    countfield[numFields]=0;
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "key"))
		keyfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "org"))
		taxfield=numFields;
	    else
		ajWarn("GenBank parsing unknown field '%S' ignored",
		       fields[numFields]);

	    numFields++;
	}
	reset = ajFalse;
    }

    /*
    ** These are almost the same as GenBank, but with some exceptions noted
    */

    if(!regRefseqTyp)
	regRefseqTyp = ajRegCompC("^(  )?([A-Z]+)");

    if(!regRefseqMore)
	regRefseqMore = ajRegCompC("^            ");

    if(!regRefseqWrd)
	regRefseqWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!regRefseqId)				/* funny characters in IDs */
	regRefseqId = ajRegCompC("([^ \t\r\n]+)");

    if(!regRefseqPhr)
	regRefseqPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!regRefseqTax)
	regRefseqTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!regRefseqVer)			  /* allow '_' in accession/version */
	regRefseqVer = ajRegCompC("([A-Za-z0-9_.]+)( +GI:([0-9]+))?");

    if(!regRefseqEnd)
	regRefseqEnd = ajRegCompC("^//");

    ipos = ajFileResetPos(libr);

    while(ajReadline(libr, &rline))
    {
	if(ajRegExec(regRefseqEnd, rline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(regRefseqTyp, rline))
	{
	    ajRegSubI(regRefseqTyp, 2, &typStr);
	    if(ajStrMatchC(typStr, "LOCUS"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(typStr, "VERSION"))
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(typStr, "ACCESSION"))
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DEFINITION"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(typStr, "KEYWORDS"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(typStr, "ORGANISM"))
		lineType = FLATTYPE_TAX;
	    else
		lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(regRefseqTyp, &tmpline);
	    /*ajDebug("++type line %d\n", lineType);*/
	}
	else if(lineType != FLATTYPE_OTHER && ajRegExec(regRefseqMore, rline))
	{
	    ajRegPost(regRefseqMore, &tmpline);
	    /*ajDebug("++more line %d\n", lineType);*/
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)    /* use REFSEQ-specific idexp */
	{
	    ajRegExec(regRefseqId, tmpline);
	    ajRegSubI(regRefseqId, 1, myid);
	    ajStrFmtUpper(myid);
	    *dpos = (ajint) ipos; /* Lossy cast */
	}

	else if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(regRefseqWrd, tmpline)) /* should be OK */
	    {
		ajRegSubI(regRefseqWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++acc '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[accfield], fd);
		}
		ajRegPost(regRefseqWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(regRefseqWrd, tmpline))
	    {
	        ajRegSubI(regRefseqWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++des '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield],
				"%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(regRefseqWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(regRefseqPhr, tmpline))
	    {
	        ajRegSubI(regRefseqPhr, 1, &tmpfd);
		ajRegPost(regRefseqPhr, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++key '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield],
				"%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(regRefseqTax, tmpline))
	    {
	        ajRegSubI(regRefseqTax, 1, &tmpfd);
		ajRegPost(regRefseqTax, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++tax '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield],
				"%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[taxfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{			       /* special verexp for REFSEQ */
	    if(ajRegExec(regRefseqVer, tmpline))
	    {
		ajRegSubI(regRefseqVer, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++ver '%S'\n", tmpfd);*/
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;

		ajRegSubI(regRefseqVer, 3, &tmpfd);
		if(!ajStrGetLen(tmpfd)) continue;
		ajStrFmtUpper(&tmpfd);
		/*ajDebug("++ver gi: '%S'\n", tmpfd);*/

		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
	    }
	    continue;
	}

	ipos = ajFileResetPos(libr);
    }

    if(!done)
	return ajFalse;

    if(svnfield >= 0 && !svndone && tmpacnum)
    {
	ajFmtPrintS(&tmpfd, "%S.0", tmpacnum);
	embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, tmpfd);
	else
	{
	    fd = ajCharNewS(tmpfd);
	    ajListPushAppend(myfdl[svnfield], fd);
	}
    }

    ajStrDel(&tmpacnum);

    return ajTrue;
}

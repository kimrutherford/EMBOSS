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

/* Definiiton of global variables */

static AjPStr dbiflatGRline   = NULL;
static AjPStr dbiflatGTmpId   = NULL;
static AjPStr dbiflatGTmpStr  = NULL;
static AjPStr dbiflatGTmpLine = NULL;
static AjPStr dbiflatGTmpFd   = NULL;
static AjPStr dbiflatGTypStr  = NULL;

static AjPRegexp dbiflatGRegEmblType = NULL;
static AjPRegexp dbiflatGRegEmblId   = NULL;
static AjPRegexp dbiflatGRegEmblAcc  = NULL;
static AjPRegexp dbiflatGRegEmblWrd  = NULL;
static AjPRegexp dbiflatGRegEmblVer  = NULL;
static AjPRegexp dbiflatGRegEmblPhr  = NULL;
static AjPRegexp dbiflatGRegEmblTax  = NULL;
static AjPRegexp dbiflatGRegEmblEnd  = NULL;

static AjPRegexp dbiflatGRegGbType = NULL;
static AjPRegexp dbiflatGRegGbMore = NULL;
static AjPRegexp dbiflatGRegGbWrd = NULL;
static AjPRegexp dbiflatGRegGbPhr = NULL;
static AjPRegexp dbiflatGRegGbTax = NULL;
static AjPRegexp dbiflatGRegGbVer = NULL;
static AjPRegexp dbiflatGRegGbEnd = NULL;

static AjPRegexp dbiflatGRegRefseqTyp = NULL;
static AjPRegexp dbiflatGRegRefseqMore = NULL;
static AjPRegexp dbiflatGRegRefseqId = NULL;
static AjPRegexp dbiflatGRegRefseqWrd = NULL;
static AjPRegexp dbiflatGRegRefseqPhr = NULL;
static AjPRegexp dbiflatGRegRefseqTax = NULL;
static AjPRegexp dbiflatGRegRefseqVer = NULL;
static AjPRegexp dbiflatGRegRefseqEnd = NULL;

static EmbPEntry dbiflatGEntry = NULL;

static AjPList* dbiflatGFdl = NULL;

static AjBool dbiflat_ParseSwiss(AjPFile libr, AjPFile* alistfile,
                                 AjBool systemsort, AjPStr* fields,
                                 ajint* maxFieldLen, ajuint* countfield,
                                 ajint *dpos, AjPStr* myid, AjPList* acl);
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
** @attr Parser [AjBool function] Parser function
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
    {"SWISS", dbiflat_ParseSwiss},
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
    ajListSort(listInputFiles, &ajStrVcmp);
    nfiles = (ajuint) ajListToarray(listInputFiles, &inputFiles);
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
	    embDbiEntryDel(&dbiflatGEntry);
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
	    ajListMap(fieldList[i], &embDbiFieldDelMap, NULL);
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

    ajRegFree(&dbiflatGRegEmblType);
    ajRegFree(&dbiflatGRegEmblId);
    ajRegFree(&dbiflatGRegEmblAcc);
    ajRegFree(&dbiflatGRegEmblWrd);
    ajRegFree(&dbiflatGRegEmblVer);
    ajRegFree(&dbiflatGRegEmblPhr);
    ajRegFree(&dbiflatGRegEmblTax);
    ajRegFree(&dbiflatGRegEmblEnd);

    ajRegFree(&dbiflatGRegGbType);
    ajRegFree(&dbiflatGRegGbMore);
    ajRegFree(&dbiflatGRegGbWrd);
    ajRegFree(&dbiflatGRegGbPhr);
    ajRegFree(&dbiflatGRegGbTax);
    ajRegFree(&dbiflatGRegGbVer);
    ajRegFree(&dbiflatGRegGbEnd);

    ajRegFree(&dbiflatGRegRefseqTyp);
    ajRegFree(&dbiflatGRegRefseqMore);
    ajRegFree(&dbiflatGRegRefseqId);
    ajRegFree(&dbiflatGRegRefseqWrd);
    ajRegFree(&dbiflatGRegRefseqTax);
    ajRegFree(&dbiflatGRegRefseqVer);
    ajRegFree(&dbiflatGRegRefseqEnd);

    embDbiEntryDel(&dbiflatGEntry);

    ajStrDel(&dbiflatGRline);
    ajStrDel(&dbiflatGTmpFd);
    ajStrDel(&dbiflatGTmpLine);
    ajStrDel(&dbiflatGTmpStr);
    ajStrDel(&dbiflatGTypStr);
    ajStrDel(&dbiflatGTmpId);

    if(dbiflatGFdl)
    {
	for(i=0; i < nfields; i++)
	    ajListFree(&dbiflatGFdl[i]);
	AJFREE(dbiflatGFdl);
    }

    ajListMap(idlist, &embDbiEntryDelMap, NULL);
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

    if(!dbiflatGFdl)
    {
	nfields = 0;
	while(fields[nfields])
	    nfields++;
	if(nfields)
	    AJCNEW(dbiflatGFdl, nfields);
	for(i=0; i < nfields; i++)
	    dbiflatGFdl[i] = ajListNew();
    }

    if(!dbiflatGEntry || !systemsort)
	dbiflatGEntry = embDbiEntryNew(nfields);

    if(!(*parser[iparser].Parser)(libr, alistfile, systemsort, fields,
                                  maxFieldLen, countfield, &ir,
                                  &dbiflatGTmpId, dbiflatGFdl))
	return NULL;

    /* dbiflatGTmpId to ret->entry */
    if(ajStrGetLen(dbiflatGTmpId) > *maxidlen)
	*maxidlen = ajStrGetLen(dbiflatGTmpId);

    if(systemsort)
	ajFmtPrintF(elistfile, "%S %d %d %d\n",
                    dbiflatGTmpId, ir, is, ifile+1);
    else
    {
	dbiflatGEntry->entry   = ajCharNewS(dbiflatGTmpId);
	dbiflatGEntry->rpos    = ir;
	dbiflatGEntry->spos    = is;
	dbiflatGEntry->filenum = ifile+1;

	/* field tokens as list, then move to dbiflatGEntry->field */
	for(ifield=0; ifield < nfields; ifield++)
	{
	    dbiflatGEntry->nfield[ifield] =
            (ajuint) ajListGetLength(dbiflatGFdl[ifield]);

	    if(dbiflatGEntry->nfield[ifield])
	    {
	        AJCNEW(dbiflatGEntry->field[ifield],
		       dbiflatGEntry->nfield[ifield]);

		i = 0;
		while(ajListPop(dbiflatGFdl[ifield],(void**) &token))
		    dbiflatGEntry->field[ifield][i++] = token;
	    }
	    else
	        dbiflatGEntry->field[ifield] = NULL;
	}
    }

    return dbiflatGEntry;
}




/* @funcstatic dbiflat_ParseSwiss *********************************************
**
** Parse the ID, accession from a SwissProt or UniProtKB entry.
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

static AjBool dbiflat_ParseSwiss(AjPFile libr, AjPFile* alistfile,
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
    ajint i;
    ajint lo;
    ajint hi;
    ajint fieldwidth;
    AjPStr tmpac = NULL;
    AjPStr format = NULL;
    AjPStr prefix = NULL;
    const char* p;
    const char* q;
    const char* swissprefix[] = {
        "RecName: ", "AltName: ", "SubName: ",
        "Includes:", "Contains:", "Flags: ",
        "Full=", "Short=", "EC=",
        "Allergen=", "Biotech=", "CD_antigen=", "INN=",
        NULL
    };
    ajuint j;

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

    if(!dbiflatGRegEmblType)
	dbiflatGRegEmblType = ajRegCompC("^([A-Z][A-Z]) +");

    if(!dbiflatGRegEmblAcc)
	dbiflatGRegEmblAcc = ajRegCompC("([A-Za-z0-9-]+)");

    if(!dbiflatGRegEmblWrd)
	dbiflatGRegEmblWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbiflatGRegEmblVer)
	dbiflatGRegEmblVer = ajRegCompC("([A-Za-z0-9_.]+)");

    if(!dbiflatGRegEmblPhr)
	dbiflatGRegEmblPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbiflatGRegEmblTax)
	dbiflatGRegEmblTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbiflatGRegEmblId)
	dbiflatGRegEmblId = ajRegCompC("^ID   ([^\\s;]+)(;\\s+SV\\s+(\\d+))?");

    if(!dbiflatGRegEmblEnd)
	dbiflatGRegEmblEnd = ajRegCompC("^//");

    *dpos = (ajint) ajFileResetPos(libr); /* Lossy cast */

    while(ajReadline(libr, &dbiflatGRline))
    {
	if(ajRegExec(dbiflatGRegEmblEnd, dbiflatGRline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(dbiflatGRegEmblType, dbiflatGRline))
	{
	    ajRegSubI(dbiflatGRegEmblType, 1, &dbiflatGTypStr);
	    if(ajStrMatchC(dbiflatGTypStr, "ID"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(dbiflatGTypStr, "SV") ||
		    ajStrMatchC(dbiflatGTypStr, "IV")) /* emblcds database */
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(dbiflatGTypStr, "AC") ||
		    ajStrMatchC(dbiflatGTypStr, "PA")) /* emblcds database */
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(dbiflatGTypStr, "DE"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(dbiflatGTypStr, "KW"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(dbiflatGTypStr, "OS"))
		lineType = FLATTYPE_TAX;
	    else if(ajStrMatchC(dbiflatGTypStr, "OC"))
		lineType = FLATTYPE_TAX;
	    else
		lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(dbiflatGRegEmblType, &dbiflatGTmpLine);
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)
	{
	    ajRegExec(dbiflatGRegEmblId, dbiflatGRline);
	    ajRegSubI(dbiflatGRegEmblId, 1, myid);
	    ajStrFmtUpper(myid);
	    ajDebug("++id '%S'\n", *myid);
	    ajRegSubI(dbiflatGRegEmblId, 3, &dbiflatGTmpFd);
	    if(svnfield >= 0 && ajStrGetLen(dbiflatGTmpFd))
	    {
		ajStrFmtUpper(&dbiflatGTmpFd);
		ajStrInsertK(&dbiflatGTmpFd, 0, '.');
		ajStrInsertS(&dbiflatGTmpFd, 0, *myid);
		/*ajDebug("++sv '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;
	    }
	    continue;
	}

	if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblAcc, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblAcc, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++acc '%S'\n", dbiflatGTmpFd);*/

		if(!tmpacnum)
		    ajStrAssignS(&tmpacnum, dbiflatGTmpFd);

		if((p=strchr(MAJSTRGETPTR(dbiflatGTmpFd),(int)'-')))
		{
		    q = p;
		    while(isdigit((int)*(--q)));
		    ++q;
		    ajStrAssignSubC(&dbiflatGTmpStr,q,0,(ajint)(p-q-1));
		    ajStrToInt(dbiflatGTmpStr,&lo);
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
		    embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[accfield]);

		    countfield[accfield]++;
		    if(systemsort)
			ajFmtPrintF(alistfile[accfield],
				    "%S %S\n", *myid, dbiflatGTmpFd);
		    else
		    {
			fd = ajCharNewS(dbiflatGTmpFd);
			ajListPushAppend(myfdl[accfield], fd);
		    }
		}
		ajRegPost(dbiflatGRegEmblAcc, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
            ajStrTrimWhiteStart(&dbiflatGTmpLine);
            for(j=0; swissprefix[j]; j++)
            {
                if(ajStrPrefixC(dbiflatGTmpLine, swissprefix[j]))
                    ajStrCutStart(&dbiflatGTmpLine, strlen(swissprefix[j]));
            }
	    while(ajRegExec(dbiflatGRegEmblWrd, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblWrd, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++des '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(dbiflatGRegEmblWrd, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblVer, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblVer, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++sv '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		ajRegPost(dbiflatGRegEmblVer, &dbiflatGTmpStr);	
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
            }
	    svndone = ajTrue;
	    continue;
	}
	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblPhr, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblPhr, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegEmblPhr, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++key '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblTax, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblTax, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegEmblTax, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrFmtUpper(&dbiflatGTmpFd);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		/*ajDebug("++tax '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
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
	ajFmtPrintS(&dbiflatGTmpFd, "%S.0", tmpacnum);
	embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, dbiflatGTmpFd);
	else
	{
	    fd = ajCharNewS(dbiflatGTmpFd);
	    ajListPushAppend(myfdl[svnfield], fd);
	}
    }

    ajStrDel(&tmpacnum);

    return ajTrue;
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

    if(!dbiflatGRegEmblType)
	dbiflatGRegEmblType = ajRegCompC("^([A-Z][A-Z]) +");

    if(!dbiflatGRegEmblAcc)
	dbiflatGRegEmblAcc = ajRegCompC("([A-Za-z0-9-]+)");

    if(!dbiflatGRegEmblWrd)
	dbiflatGRegEmblWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbiflatGRegEmblVer)
	dbiflatGRegEmblVer = ajRegCompC("([A-Za-z0-9_.]+)");

    if(!dbiflatGRegEmblPhr)
	dbiflatGRegEmblPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbiflatGRegEmblTax)
	dbiflatGRegEmblTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbiflatGRegEmblId)
	dbiflatGRegEmblId = ajRegCompC("^ID   ([^\\s;]+)(;\\s+SV\\s+(\\d+))?");

    if(!dbiflatGRegEmblEnd)
	dbiflatGRegEmblEnd = ajRegCompC("^//");

    *dpos = (ajint) ajFileResetPos(libr); /* Lossy cast */

    while(ajReadline(libr, &dbiflatGRline))
    {
	if(ajRegExec(dbiflatGRegEmblEnd, dbiflatGRline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(dbiflatGRegEmblType, dbiflatGRline))
	{
	    ajRegSubI(dbiflatGRegEmblType, 1, &dbiflatGTypStr);
	    if(ajStrMatchC(dbiflatGTypStr, "ID"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(dbiflatGTypStr, "SV") ||
		    ajStrMatchC(dbiflatGTypStr, "IV")) /* emblcds database */
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(dbiflatGTypStr, "AC") ||
		    ajStrMatchC(dbiflatGTypStr, "PA")) /* emblcds database */
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(dbiflatGTypStr, "DE"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(dbiflatGTypStr, "KW"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(dbiflatGTypStr, "OS"))
		lineType = FLATTYPE_TAX;
	    else if(ajStrMatchC(dbiflatGTypStr, "OC"))
		lineType = FLATTYPE_TAX;
	    else
		lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(dbiflatGRegEmblType, &dbiflatGTmpLine);
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)
	{
	    ajRegExec(dbiflatGRegEmblId, dbiflatGRline);
	    ajRegSubI(dbiflatGRegEmblId, 1, myid);
	    ajStrFmtUpper(myid);
	    ajDebug("++id '%S'\n", *myid);
	    ajRegSubI(dbiflatGRegEmblId, 3, &dbiflatGTmpFd);
	    if(svnfield >= 0 && ajStrGetLen(dbiflatGTmpFd))
	    {
		ajStrFmtUpper(&dbiflatGTmpFd);
		ajStrInsertK(&dbiflatGTmpFd, 0, '.');
		ajStrInsertS(&dbiflatGTmpFd, 0, *myid);
		/*ajDebug("++sv '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;
	    }
	    continue;
	}

	if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblAcc, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblAcc, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++acc '%S'\n", dbiflatGTmpFd);*/

		if(!tmpacnum)
		    ajStrAssignS(&tmpacnum, dbiflatGTmpFd);

		if((p=strchr(MAJSTRGETPTR(dbiflatGTmpFd),(int)'-')))
		{
		    q = p;
		    while(isdigit((int)*(--q)));
		    ++q;
		    ajStrAssignSubC(&dbiflatGTmpStr,q,0,(ajint)(p-q-1));
		    ajStrToInt(dbiflatGTmpStr,&lo);
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
		    embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[accfield]);

		    countfield[accfield]++;
		    if(systemsort)
			ajFmtPrintF(alistfile[accfield],
				    "%S %S\n", *myid, dbiflatGTmpFd);
		    else
		    {
			fd = ajCharNewS(dbiflatGTmpFd);
			ajListPushAppend(myfdl[accfield], fd);
		    }
		}
		ajRegPost(dbiflatGRegEmblAcc, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblWrd, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblWrd, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++des '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(dbiflatGRegEmblWrd, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblVer, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblVer, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++sv '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		ajRegPost(dbiflatGRegEmblVer, &dbiflatGTmpStr);	
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
            }
	    svndone = ajTrue;
	    continue;
	}
	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblPhr, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblPhr, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegEmblPhr, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++key '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegEmblTax, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegEmblTax, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegEmblTax, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrFmtUpper(&dbiflatGTmpFd);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		/*ajDebug("++tax '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
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
	ajFmtPrintS(&dbiflatGTmpFd, "%S.0", tmpacnum);
	embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, dbiflatGTmpFd);
	else
	{
	    fd = ajCharNewS(dbiflatGTmpFd);
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

    if(!dbiflatGRegGbType)
	dbiflatGRegGbType = ajRegCompC("^(  )?([A-Z]+)");

    if(!dbiflatGRegGbMore)
	dbiflatGRegGbMore = ajRegCompC("^            ");

    if(!dbiflatGRegGbWrd)
	dbiflatGRegGbWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbiflatGRegGbPhr)
	dbiflatGRegGbPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbiflatGRegGbTax)
	dbiflatGRegGbTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbiflatGRegGbVer)
	dbiflatGRegGbVer = ajRegCompC("([A-Za-z0-9.]+)( +GI:([0-9]+))?");
    
    if(!dbiflatGRegGbEnd)
	dbiflatGRegGbEnd = ajRegCompC("^//");

    ipos = ajFileResetPos(libr);

    while(ajReadline(libr, &dbiflatGRline))
    {
	if(ajRegExec(dbiflatGRegGbEnd, dbiflatGRline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(dbiflatGRegGbType, dbiflatGRline))
	{
	    ajRegSubI(dbiflatGRegGbType, 2, &dbiflatGTypStr);
	    if(ajStrMatchC(dbiflatGTypStr, "LOCUS"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(dbiflatGTypStr, "VERSION"))
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(dbiflatGTypStr, "ACCESSION"))
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(dbiflatGTypStr, "DEFINITION"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(dbiflatGTypStr, "KEYWORDS"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(dbiflatGTypStr, "ORGANISM"))
		lineType = FLATTYPE_TAX;
	    else lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(dbiflatGRegGbType, &dbiflatGTmpLine);
	    /*ajDebug("++type line %d\n", lineType);*/
	}
	else if(lineType != FLATTYPE_OTHER &&
                ajRegExec(dbiflatGRegGbMore, dbiflatGRline))
	{
	    ajRegPost(dbiflatGRegGbMore, &dbiflatGTmpLine);
	    /*ajDebug("++more line %d\n", lineType);*/
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)
	{
	    ajRegExec(dbiflatGRegGbWrd, dbiflatGTmpLine);
	    ajRegSubI(dbiflatGRegGbWrd, 1, myid);
	    *dpos = (ajint) ipos; /* Lossy cast */
	}

	else if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegGbWrd, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegGbWrd, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++acc '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[accfield], fd);
		}
		ajRegPost(dbiflatGRegGbWrd, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegGbWrd, dbiflatGTmpLine))
	    {
	        ajRegSubI(dbiflatGRegGbWrd, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++des '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield],
				"%S %S\n", *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(dbiflatGRegGbWrd, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegGbPhr, dbiflatGTmpLine))
	    {
	        ajRegSubI(dbiflatGRegGbPhr, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegGbPhr, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++key '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield],
				"%S %S\n", *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegGbTax, dbiflatGTmpLine))
	    {
	        ajRegSubI(dbiflatGRegGbTax, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegGbTax, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++tax '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield],
				"%S %S\n", *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[taxfield], fd);
		}
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    if(ajRegExec(dbiflatGRegGbVer, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegGbVer, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++ver '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;

		ajRegSubI(dbiflatGRegGbVer, 3, &dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++ver gi: '%S'\n", dbiflatGTmpFd);*/

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
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
	ajFmtPrintS(&dbiflatGTmpFd, "%S.0", tmpacnum);
	embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, dbiflatGTmpFd);
	else
	{
	    fd = ajCharNewS(dbiflatGTmpFd);
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

    if(!dbiflatGRegRefseqTyp)
	dbiflatGRegRefseqTyp = ajRegCompC("^(  )?([A-Z]+)");

    if(!dbiflatGRegRefseqMore)
	dbiflatGRegRefseqMore = ajRegCompC("^            ");

    if(!dbiflatGRegRefseqWrd)
	dbiflatGRegRefseqWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbiflatGRegRefseqId) /* funny characters in IDs */
	dbiflatGRegRefseqId = ajRegCompC("([^ \t\r\n]+)");

    if(!dbiflatGRegRefseqPhr)
	dbiflatGRegRefseqPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbiflatGRegRefseqTax)
	dbiflatGRegRefseqTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbiflatGRegRefseqVer) /* allow '_' in accession/version */
	dbiflatGRegRefseqVer = ajRegCompC("([A-Za-z0-9_.]+)( +GI:([0-9]+))?");

    if(!dbiflatGRegRefseqEnd)
	dbiflatGRegRefseqEnd = ajRegCompC("^//");

    ipos = ajFileResetPos(libr);

    while(ajReadline(libr, &dbiflatGRline))
    {
	if(ajRegExec(dbiflatGRegRefseqEnd, dbiflatGRline))
	{
	    done = ajTrue;
	    break;
	}

	if(ajRegExec(dbiflatGRegRefseqTyp, dbiflatGRline))
	{
	    ajRegSubI(dbiflatGRegRefseqTyp, 2, &dbiflatGTypStr);
	    if(ajStrMatchC(dbiflatGTypStr, "LOCUS"))
		lineType = FLATTYPE_ID;
	    else if(ajStrMatchC(dbiflatGTypStr, "VERSION"))
		lineType = FLATTYPE_VER;
	    else if(ajStrMatchC(dbiflatGTypStr, "ACCESSION"))
		lineType = FLATTYPE_ACC;
	    else if(ajStrMatchC(dbiflatGTypStr, "DEFINITION"))
		lineType = FLATTYPE_DES;
	    else if(ajStrMatchC(dbiflatGTypStr, "KEYWORDS"))
		lineType = FLATTYPE_KEY;
	    else if(ajStrMatchC(dbiflatGTypStr, "ORGANISM"))
		lineType = FLATTYPE_TAX;
	    else
		lineType=FLATTYPE_OTHER;

	    if(lineType != FLATTYPE_OTHER)
		ajRegPost(dbiflatGRegRefseqTyp, &dbiflatGTmpLine);
	    /*ajDebug("++type line %d\n", lineType);*/
	}
	else if(lineType != FLATTYPE_OTHER &&
                ajRegExec(dbiflatGRegRefseqMore, dbiflatGRline))
	{
	    ajRegPost(dbiflatGRegRefseqMore, &dbiflatGTmpLine);
	    /*ajDebug("++more line %d\n", lineType);*/
	}
	else
	    lineType = FLATTYPE_OTHER;

	if(lineType == FLATTYPE_ID)    /* use REFSEQ-specific idexp */
	{
	    ajRegExec(dbiflatGRegRefseqId, dbiflatGTmpLine);
	    ajRegSubI(dbiflatGRegRefseqId, 1, myid);
	    ajStrFmtUpper(myid);
	    *dpos = (ajint) ipos; /* Lossy cast */
	}

	else if(lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegRefseqWrd, dbiflatGTmpLine))
            /* should be OK */
	    {
		ajRegSubI(dbiflatGRegRefseqWrd, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++acc '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[accfield], fd);
		}
		ajRegPost(dbiflatGRegRefseqWrd, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegRefseqWrd, dbiflatGTmpLine))
	    {
	        ajRegSubI(dbiflatGRegRefseqWrd, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++des '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield],
				"%S %S\n", *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[desfield], fd);
		}
		ajRegPost(dbiflatGRegRefseqWrd, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
	    }
	    continue;
	}

	else if(lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegRefseqPhr, dbiflatGTmpLine))
	    {
	        ajRegSubI(dbiflatGRegRefseqPhr, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegRefseqPhr, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++key '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield],
				"%S %S\n", *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(dbiflatGRegRefseqTax, dbiflatGTmpLine))
	    {
	        ajRegSubI(dbiflatGRegRefseqTax, 1, &dbiflatGTmpFd);
		ajRegPost(dbiflatGRegRefseqTax, &dbiflatGTmpStr);
                ajStrAssignS(&dbiflatGTmpLine, dbiflatGTmpStr);
		ajStrTrimWhiteEnd(&dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd))
		    continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++tax '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield],
				"%S %S\n", *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[taxfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == FLATTYPE_VER && svnfield >= 0)
	{			       /* special verexp for REFSEQ */
	    if(ajRegExec(dbiflatGRegRefseqVer, dbiflatGTmpLine))
	    {
		ajRegSubI(dbiflatGRegRefseqVer, 1, &dbiflatGTmpFd);
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++ver '%S'\n", dbiflatGTmpFd);*/
		embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
		    ajListPushAppend(myfdl[svnfield], fd);
		}
		svndone = ajTrue;

		ajRegSubI(dbiflatGRegRefseqVer, 3, &dbiflatGTmpFd);
		if(!ajStrGetLen(dbiflatGTmpFd)) continue;
		ajStrFmtUpper(&dbiflatGTmpFd);
		/*ajDebug("++ver gi: '%S'\n", dbiflatGTmpFd);*/

		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n",
                                *myid, dbiflatGTmpFd);
		else
		{
		    fd = ajCharNewS(dbiflatGTmpFd);
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
	ajFmtPrintS(&dbiflatGTmpFd, "%S.0", tmpacnum);
	embDbiMaxlen(&dbiflatGTmpFd, &maxFieldLen[svnfield]);

	countfield[svnfield]++;
	if(systemsort)
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *myid, dbiflatGTmpFd);
	else
	{
	    fd = ajCharNewS(dbiflatGTmpFd);
	    ajListPushAppend(myfdl[svnfield], fd);
	}
    }

    ajStrDel(&tmpacnum);

    return ajTrue;
}

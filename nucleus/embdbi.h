#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embdbi_h
#define embdbi_h




/* @data EmbPField ************************************************************
**
** NUCLEUS internal structure for database indexing applications
** to store field tokens with links to the entry index number.
**
** @attr field [char*] field token
** @attr entry [char*] entry name
** @attr nid [ajuint] entry number
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSField {
  char* field;
  char* entry;
  ajuint nid;
  char Padding[4];
} EmbOField;
#define EmbPField EmbOField*




/* @data EmbPEntry ************************************************************
**
** NUCLEUS internal structure for database indexing applications
** to store an entry id with a list of field tokens and file
** positions for writing to the index files.
**
** @attr entry [char*] entry name
** @attr filenum [ajuint] record in division file
** @attr rpos [ajint] entry offset in data file
** @attr spos [ajint] entry offset in sequence file
** @attr nfields [ajuint] number of fields
** @attr nfield [ajuint*] number of tokens for each field
** @attr field [char***] array of tokens for each field
** @@
******************************************************************************/

typedef struct EmbSEntry {
  char* entry;
  ajuint filenum;
  ajint rpos;
  ajint spos;
  ajuint nfields;
  ajuint* nfield;
  char*** field;
} EmbOEntry;
#define EmbPEntry EmbOEntry*




/*
** Prototype definitions
*/

ajint     embDbiCmpId (const void* a, const void* b);
ajint     embDbiCmpFieldId (const void* a, const void* b);
ajint     embDbiCmpFieldField (const void* a, const void* b);
void      embDbiDateSet (const AjPStr datestr, char date[4]);
void      embDbiEntryDel(EmbPEntry* Pentry);
void      embDbiEntryDelMap(void** pthys, void* cl);
EmbPEntry embDbiEntryNew (ajuint nfields);
void      embDbiExit(void);
void      embDbiFieldDel(EmbPField* pthys);
void      embDbiFieldDelMap(void** pthys, void* cl);
EmbPField embDbiFieldNew (void);
AjPFile   embDbiFileIn (const AjPStr dbname, const char* extension);
AjPFile   embDbiFileIndex (const AjPStr indexdir, const AjPStr field,
			   const char* extension);
AjPList   embDbiFileList (const AjPStr dir, const AjPStr wildfile,
			  AjBool trim);
AjPList   embDbiFileListExc (const AjPStr dir, const AjPStr wildfile,
			     const AjPStr exclude);
AjPFile   embDbiFileOut (const AjPStr dbname, const char* extension);
AjPFile   embDbiFileSingle (const AjPStr dbname, const char* extension,
			    ajuint num);
AjBool    embDbiFlatOpenlib(const AjPStr lname, AjPFile* libr);
void      embDbiHeader (AjPFile file, ajuint filesize,
			ajuint recordcnt, short recordlen,
			const AjPStr dbname, const AjPStr release,
			const char date[4]);
void      embDbiHeaderSize (AjPFile file, ajuint filesize, ajuint recordcnt);
void      embDbiLogCmdline(AjPFile logfile);
void      embDbiLogFields(AjPFile logfile, AjPStr const * fields,
			  ajuint nfields);
void      embDbiLogFile(AjPFile logfile, const AjPStr curfilename,
			ajuint idCountFile, AjPStr const * fields,
			const ajuint* countField,
			ajuint nfields);
void      embDbiLogFinal(AjPFile logfile, ajuint maxlen,
			 const ajint* maxFieldLen,
			 AjPStr const * fields, const ajuint* fieldTot,
			 ajuint nfields, ajuint nfiles,
			 ajuint idDone, ajuint idCount);
void      embDbiLogHeader(AjPFile logfile, const AjPStr dbname,
			  const AjPStr release, const AjPStr datestr,
			  const AjPStr indexdir,
			  ajuint maxindex);
void      embDbiLogSource(AjPFile logfile, const AjPStr directory,
			  const AjPStr filename, const AjPStr exclude,
			  AjPStr const * inputFiles, ajuint nfiles);
void      embDbiMaxlen (AjPStr* token, ajint* maxlen);
void      embDbiMemEntry (AjPList idlist,
			  AjPList* fieldList, ajuint nfields,
			  EmbPEntry entry, ajuint ifile);
ajuint     embDbiMemWriteEntry (AjPFile entFile, ajuint maxidlen,
			       const AjPList idlist, void ***ids);
ajuint     embDbiMemWriteFields (const AjPStr dbname, const AjPStr release,
				const char date[4], const AjPStr indexdir,
				const AjPStr field, ajuint maxFieldLen,
				const AjPList fieldList,
				void** ids);
void      embDbiRmEntryFile (const AjPStr dbname,  AjBool cleanup);
void      embDbiRmFile (const AjPStr dbname, const char* ext, ajuint nfiles,
			AjBool cleanup);
void      embDbiRmFileI (const AjPStr dbname, const char* ext, ajuint ifile,
			 AjBool cleanup);
void      embDbiSortClose (AjPFile* elistfile, AjPFile* alistfile,
			   ajuint nfields);
void      embDbiSortFile (const AjPStr dbname,
			  const char* ext1, const char* ext2,
			  ajuint nfiles, AjBool cleanup, const AjPStr sortopt);
AjPFile   embDbiSortOpen (AjPFile* alistfile, ajuint ifile,
			  const AjPStr dbname, AjPStr const * fields,
			  ajuint nfields);
ajuint     embDbiSortWriteEntry (AjPFile entFile, ajuint maxidlen,
				const AjPStr dbname, ajuint nfiles,
				AjBool cleanup, const AjPStr sortopt);
ajuint     embDbiSortWriteFields (const AjPStr dbname, const AjPStr release,
				 const char date[4], const AjPStr indexdir,
				 const AjPStr field,  ajuint maxFieldLen,
				 ajuint nfiles, ajuint nentries,
				 AjBool cleanup, const AjPStr sortopt);
void      embDbiWriteDivision (const AjPStr indexdir,
			       const AjPStr dbname, const AjPStr release,
			       const char date[4],
			       ajuint maxfilelen, ajuint nfiles,
			       AjPStr const * divfiles,
			       AjPStr const * seqfiles);
void      embDbiWriteDivisionRecord (AjPFile file,
				     ajuint maxnamlen, short recnum,
				     const AjPStr datfile,
				     const AjPStr seqfile);
void      embDbiWriteEntryRecord (AjPFile file, ajuint maxidlen,
				  const AjPStr id,
				  ajint rpos, ajint spos, short filenum);
void      embDbiWriteHit (AjPFile file, ajuint idnum);
void      embDbiWriteTrg (AjPFile file, ajuint maxfieldlen,
			  ajuint idnum, ajuint idcnt, const AjPStr hitstr);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

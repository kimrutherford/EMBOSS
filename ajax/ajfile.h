#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfile_h
#define ajfile_h

#include "ajlist.h"
#include <sys/stat.h>

/* @data AjPFile **************************************************************
**
** Ajax file object. Holds information for an open (unbuffered)
** input or output file.
**
** On output, conversion code "%F" writes the filename.
**
** @alias AjSFile
** @alias AjOFile
**
** @new ajFileNew Default constructor for an input file
** @new ajFileNewF Constructor using an existing open file,
**                 for example stdin or stdout
** @new ajFileNewIn Constructor using a filename for an input file
** @new ajFileNewApp Default constructor using a filename for an output file
**                   to be opened for appending records to the end of the file
** @new ajFileNewOut Default constructor using a filename for an output file
** @new ajFileNewInPipe Constructor using output from a forked command
** @new ajFileNewInList Constructor using an AjPList of filenames
** @delete ajFileClose Default destructor
** @modify ajFileGets Reads a record from a file
** @modify ajFileGetsTrim Reads a record from a file and removes
**                     newline characters
** @cast ajFileName Returns the filename as char*
** @cast ajFileNameS Returns the filename as an AjPStr
** @cast ajFileFp Returns the equivalent C file pointer
** @cast ajFileGetApp Returns the App element.
** @other AjPFileBuff Buffered input file.
**
** @attr fp [FILE*] C file pointer
** @attr Name [AjPStr] File name as used when opening
** @attr List [AjPList] List of file names (first is open)
** @attr End [AjBool] True if EOF has been reached
** @attr App [AjBool] True if file was opened for append. 
** @attr Buff [AjPStr] Buffer for latest line read
** @attr Pid [pid_t] Process PID if any
** @attr Handle [ajint] AJAX file number 0 if unused
** @@
******************************************************************************/

typedef struct AjSFile {
  FILE *fp;
  AjPStr Name;
  AjPList List;
  AjBool End;
  AjBool App;
  AjPStr Buff;
#ifndef WIN32
  pid_t Pid;
#endif
  ajint Handle;
} AjOFile;

#define AjPFile AjOFile*

/* @data AjPFileBuffList ******************************************************
**
** Ajax file buffer, holding a simple linked list of buffered lines.
** This does not use the AjPList objects.
**
** This is a substructure of the AjPFileBuff object.
**
** @attr Line [AjPStr] String : this line
** @attr Next [struct AjSFileBuffList*] Next line in the list, NULL for last
** @attr Fpos [ajlong] File offset for start of this line
** @@
******************************************************************************/

typedef struct AjSFileBuffList {
  AjPStr Line;
  struct AjSFileBuffList* Next;
  ajlong Fpos;
} AjOFileBuffList;

#define AjPFileBuffList AjOFileBuffList*

/* @data AjPFileBuff **********************************************************
**
** Ajax buffered file object. Holds information for a buffered input file.
**
** @alias AjSFileBuff
** @alias AjOFileBuff
**
** @new ajFileBuffNew Default constructor for an input file
** @new ajFileBuffNewF Constructor using an existing open file,
**                 for example stdin or stdout.
** @new ajFileBuffNewIn Constructor using a filename
** @new ajFileBuffNewS Constructor using a line of buffered data
** @new ajFileBuffNewDW Constructor using a directory and wildcard filename
** @new ajFileBuffNewInList Constructor using a list of filenames
** @delete ajFileBuffDel Default destructor
** @modify ajFileBuffNobuff Turns off input buffering.
** @modify ajFileBuffReset Resets so the next read uses the first buffered line
** @modify ajFileBuffClear Deletes unwanted old lines from the buffer
**                      but can keep the most recent line(s) for reuse.
** @modify ajFileBuffStripHtml Processes data in the file buffer,
**                          removing HTML tokens
** @input ajFileBuffLoad Reads all input lines from a file into the buffer.
** @modify ajFileBuffGet Reads a line from a buffered file.
** @modify ajFileBuffGetStore Reads a line from a buffered file with append.
** @modify ajFileBuffGetStoreL Reads a line from a buffered file with append.
** @use ajFileBuffEmpty Tests whether a file buffer is empty.
** @use ajFileBuffTrace Writes debug messages to indicate the contents
**                      of a buffered file.
** @cast ajFileBuffFile Returns the equivalent AjPFile without the
**                      buffer access.
** @use ajFileBuffTraceFull Writes debug messages to show the full contents
**                      of a buffered file.
** @cast ajFileBuffIsBuffered Tests for input buffering.
** @other AjPFile Simple input file.
**
** @attr File [AjPFile] The input file - data to be buffered
** @attr Lines [AjPFileBuffList] All lines ... where the data really is
** @attr Freelines [AjPFileBuffList] Free list of lines for reuse
** @attr Curr [AjPFileBuffList] Current line in Lines list
** @attr Prev [AjPFileBuffList] Previous line (points to Curr for delete)
** @attr Last [AjPFileBuffList] Last line for quick appending
** @attr Freelast [AjPFileBuffList] Last free line for quick append
** @attr Nobuff [AjBool] if true, do not buffer the file
** @attr Pos [ajint] Position in list
** @attr Size [ajint] Size of list
** @attr FreeSize [ajint] Size of free list
** @attr Fpos [ajlong] File position in File
** @@
******************************************************************************/

typedef struct AjSFileBuff {
  AjPFile File;
  AjPFileBuffList Lines;
  AjPFileBuffList Freelines;
  AjPFileBuffList Curr;
  AjPFileBuffList Prev;
  AjPFileBuffList Last;
  AjPFileBuffList Freelast;
  AjBool Nobuff;
  ajint Pos;
  ajint Size;
  ajint FreeSize;
  ajlong Fpos;
} AjOFileBuff;

#define AjPFileBuff AjOFileBuff*

/* @data AjPDir **************************************************************
**
** Ajax directory object. Holds information for an open
** input or output directory.
**
** @alias AjSDir
** @alias AjODir
**
** @new ajDirNew Default constructor for a directory
** @new ajDirNewS Directory constructor with extension
** @new ajDirNewSS Directory constructor with prefix and extension
** @new ajDiroutNew Default constructor for an output directory
** @new ajDiroutNewS Output directory constructor with extension
** @new ajDiroutNewSS Output directory constructor with prefix
**                        and extension
** @delete ajDirDel Default destructor
** @delete ajDiroutDel Default destructor for output directory
**
** @attr Name [AjPStr] Path
** @attr Prefix [AjPStr] Default filename prefix
** @attr Extension [AjPStr] Default file extension
** @attr Output [AjBool] True if to be used for output
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSDir {
  AjPStr Name;
  AjPStr Prefix;
  AjPStr Extension;
  AjBool Output;
  char Padding[4];
} AjODir;

#define AjPDir AjODir*

/* @data AjPOutfile ***********************************************************
**
** Ajax file object. Holds information for an open (unbuffered)
** input or output file.
**
** On output, conversion code "%F" writes the filename.
**
** @alias AjSOutfile
** @alias AjOOutfile
**
** @new ajOutfileNew Default constructor for an output file
** @delete ajOutfileClose Close file and descruct
** @delete ajOutfileDel Default destructor
**
** @attr File [AjPFile] File object
** @attr Type [AjPStr] Named data file type
** @attr Formatstr [AjPStr] Format specific for this data type
** @attr Itype [ajint] Index number for Type
** @attr Format [ajint] Index for Formatstr for this data type
** @@
******************************************************************************/

typedef struct AjSOutfile {
  AjPFile File;
  AjPStr Type;
  AjPStr Formatstr;
  ajint Itype;
  ajint Format;
} AjOOutfile;

#define AjPOutfile AjOOutfile*

enum AjEOutfileType
{
    OUTFILE_UNKNOWN,			/* Unknown - none of these */
    OUTFILE_CODON,			/* Codon usage */
    OUTFILE_CPDB,			/* Clean PDB */
    OUTFILE_DISCRETE,			/* Discrete data */
    OUTFILE_DISTANCE,			/* Distance matrix data */
    OUTFILE_FREQ,			/* Frequency data */
    OUTFILE_MATRIX,			/* Integer matrix data */
    OUTFILE_MATRIXF,			/* Floating point matrix data */
    OUTFILE_PROPERTIES,			/* Phylogenetic properties */
    OUTFILE_SCOP,			/* SCOP data */
    OUTFILE_TREE			/* Phylogenetic tree data */
};


/*
** Prototype definitions
*/

void        ajDirDel (AjPDir* pthis);
void        ajDiroutDel (AjPDir* pthis);
AjPStr      ajDirExt(const AjPDir thys);
AjPStr      ajDirName(const AjPDir thys);
AjPDir      ajDirNew (const AjPStr name);
AjPDir      ajDirNewS (const AjPStr name, const AjPStr ext);
AjPDir      ajDirNewSS (const AjPStr name, const AjPStr prefix,
			const AjPStr ext);
AjPDir      ajDiroutNew (const AjPStr name);
AjPDir      ajDiroutNewS (const AjPStr name, const AjPStr ext);
AjPDir      ajDiroutNewSS (const AjPStr name, const AjPStr prefix,
			   const AjPStr ext);

AjBool      ajFileBuffBuff (AjPFileBuff thys);
void        ajFileBuffClear (AjPFileBuff thys, ajint lines);
void        ajFileBuffClearStore (AjPFileBuff thys, ajint lines,
				  const AjPStr rdline, AjBool store,
				  AjPStr *astr);
void        ajFileBuffDel (AjPFileBuff* pthis);
AjBool      ajFileBuffEmpty (const AjPFileBuff thys);
AjBool      ajFileBuffEnd (const AjPFileBuff thys);
AjBool      ajFileBuffEof (const AjPFileBuff thys);
AjPFile     ajFileBuffFile (const AjPFileBuff thys);
void        ajFileBuffFix (AjPFileBuff thys);
FILE*       ajFileBuffFp (const AjPFileBuff thys);
void        ajFileBuffFreeClear (AjPFileBuff thys);
AjBool      ajFileBuffGet  (AjPFileBuff thys, AjPStr *pdest);
AjBool      ajFileBuffGetL (AjPFileBuff thys, AjPStr *pdest,
			    ajlong* fpos);
AjBool      ajFileBuffGetStore (AjPFileBuff thys, AjPStr* pdest,
				AjBool store, AjPStr *astr);
AjBool      ajFileBuffGetStoreL (AjPFileBuff thys, AjPStr* pdest,
				 ajlong* fpos, AjBool store, AjPStr *astr);
AjBool      ajFileBuffGetTrim(AjPFileBuff thys, AjPStr* pdest);
AjBool      ajFileBuffIsBuffered (const AjPFileBuff thys);
void        ajFileBuffLoad (AjPFileBuff thys);
void        ajFileBuffLoadC (AjPFileBuff thys, const char* str);
void        ajFileBuffLoadS (AjPFileBuff thys, const AjPStr str);
AjPFileBuff ajFileBuffNew (void);
AjPFileBuff ajFileBuffNewDC (const AjPStr dir, const char* filename);
AjPFileBuff ajFileBuffNewDF (const AjPStr dir, const AjPStr filename);
AjPFileBuff ajFileBuffNewDW (const AjPStr dir, const AjPStr wildfile);
AjPFileBuff ajFileBuffNewDWE (const AjPStr dir, const AjPStr wildfile,
			      const AjPStr exclude);
AjPFileBuff ajFileBuffNewF (FILE *fp);
AjPFileBuff ajFileBuffNewFile (AjPFile file);
AjPFileBuff ajFileBuffNewIn (const AjPStr name);
AjPFileBuff ajFileBuffNewInList (AjPList list);
AjPFileBuff ajFileBuffNewS (const AjPStr data);
AjBool      ajFileBuffNobuff (AjPFileBuff thys);
void        ajFileBuffPrint (const AjPFileBuff thys, const char* title);
void        ajFileBuffReset (AjPFileBuff thys);
void        ajFileBuffResetPos (AjPFileBuff thys);
void        ajFileBuffResetStore (AjPFileBuff thys,
				  AjBool store, AjPStr *astr);
AjBool      ajFileBuffSetFile (AjPFileBuff* pthys, AjPFile file,
			       AjBool samefile);
ajint       ajFileBuffSize (void);
void        ajFileBuffStripHtml (AjPFileBuff thys);
AjBool      ajFileBuffStripHtmlPre (AjPFileBuff thys);
void        ajFileBuffTrace (const AjPFileBuff thys);
void        ajFileBuffTraceFull (const AjPFileBuff thys, size_t nlines,
				 size_t nfree);
void        ajFileClose (AjPFile *pthis);
void        ajFileDataDirNew  (const AjPStr tfile, const AjPStr dir,
			       AjPFile *fnew);
void        ajFileDataDirNewC (const char *s, const char* d,
			       AjPFile *f);
void        ajFileDataNew  (const AjPStr filename, AjPFile *newfileptr);
void        ajFileDataNewC (const char *s, AjPFile *f);
void        ajFileDataNewWrite (const AjPStr tfile, AjPFile *fnew);
AjBool      ajFileDir (AjPStr* dir);
void        ajFileDirFix (AjPStr* dir);
AjBool      ajFileDirUp (AjPStr* dir);
AjBool      ajFileDirPath (AjPStr* dir);
AjBool      ajFileDirTrim (AjPStr* name);
AjBool      ajFileExtnTrim (AjPStr* name);
AjBool      ajFileDirExtnTrim (AjPStr* name);
AjBool      ajFileEof (const AjPFile thys);
void        ajFileExit (void);
AjPList     ajFileFileList(const AjPStr files);
FILE*       ajFileFp (const AjPFile thys);
AjBool      ajFileGetwd (AjPStr* dir);
AjBool      ajFileGetApp (const AjPFile thys);
AjPStr      ajFileGetName (const AjPFile thys);
AjBool      ajFileGets (AjPFile thys, AjPStr *pdest);
AjBool      ajFileGetsL (AjPFile thys, AjPStr *pdest, ajlong* fpos);
AjBool      ajFileGetsTrim  (AjPFile thys, AjPStr *pdest);
AjBool      ajFileGetsTrimL (AjPFile thys, AjPStr *pdest, ajlong* fpos);
AjBool      ajFileHasDir (const AjPStr name);
ajlong      ajFileLength (const AjPStr fname);
const char* ajFileName (const AjPFile thys);
const AjPStr ajFileNameS (const AjPFile thys);
AjBool      ajFileNameDir (AjPStr* filename, const AjPDir dir,
			   const AjPStr name);
AjBool      ajFileNameDirSet  (AjPStr* filename, const AjPStr dir);
AjBool      ajFileNameDirSetC (AjPStr* filename, const char* dir);
AjBool      ajFileNameExt  (AjPStr* filename, const AjPStr extension);
AjBool      ajFileNameExtC (AjPStr* filename, const char* extension);
AjBool      ajFileNameShorten(AjPStr *fname);
AjBool      ajFileNameTrim(AjPStr *fname);
AjPFile     ajFileNew (void);
AjPFile     ajFileNewApp (const AjPStr name);
AjPFile     ajFileNewDirF(const AjPDir dir, const AjPStr filename);
AjPFile     ajFileNewDC (const AjPStr dir, const char* filename);
AjPFile     ajFileNewDF (const AjPStr dir, const AjPStr filename);
AjPFile     ajFileNewDW (const AjPStr dir, const AjPStr wildfile);
AjPFile     ajFileNewDWE (const AjPStr dir, const AjPStr wildfile,
			  const AjPStr exclude);
AjPFile     ajFileNewF (FILE* file);
AjPFile     ajFileNewIn (const AjPStr name);
AjPFile     ajFileNewInC (const char *name);
AjPFile     ajFileNewInPipe (const AjPStr name);
AjPFile     ajFileNewInList (AjPList list);
AjPFile     ajFileNewOut (const AjPStr name);
AjPFile     ajFileNewOutC (const char *name);
AjPFile     ajFileNewOutD (const AjPStr dir, const AjPStr name);
AjPFile     ajFileNewOutDir(const AjPDir dir, const AjPStr name);
void        ajFileOutHeader (AjPFile thys);
void        ajFileOutClose (AjPFile *pthis);
AjBool      ajFileNext (AjPFile thys);
size_t      ajFileRead (void* ptr, size_t element_size, size_t count,
			AjPFile thys);
AjBool      ajFileReadAppend (AjPFile thys, AjPStr* pbuff);
AjBool      ajFileReadLine (AjPFile thys, AjPStr *pdest);
ajuint      ajFileReadUint (AjPFile thys, AjBool Bigendian);
FILE*       ajFileReopen (AjPFile thys, const AjPStr name);
ajint 	    ajFileScan (const AjPStr path, const AjPStr filename,
			AjPList *result,
			AjBool show, AjBool dolist, AjPList *list,
			AjPList rlist, AjBool recurs, AjPFile outf);
ajint       ajFileSeek (AjPFile thys, ajlong offset, ajint wherefrom);
AjBool      ajFileSetDir (AjPStr *pname, const AjPStr dir);
AjBool      ajFileStat (const AjPStr filename, ajint mode);
AjBool      ajFileNameValid (const AjPStr filename);
AjBool      ajFileStderr (const AjPFile file);
AjBool      ajFileStdin (const AjPFile file);
AjBool      ajFileStdout (const AjPFile file);
ajlong      ajFileTell (const AjPFile thys);
const char* ajFileTempName (const char *dir);
AjBool      ajFileTestSkip (const AjPStr fullname,
			    const AjPStr exc, const AjPStr inc,
			    AjBool keep, AjBool ignoredirectory);
void        ajFileTrace (const AjPFile thys);
void        ajFileUnbuffer (AjPFile thys);
size_t      ajFileWrite (AjPFile thys, const void* ptr,
			 size_t element_size, size_t count);

ajint       ajFileWriteByte (AjPFile thys, char ch);
ajint       ajFileWriteChar (AjPFile thys, const char* str, ajint len);
ajint       ajFileWriteInt2 (AjPFile thys, short i);
ajint       ajFileWriteInt4 (AjPFile thys, ajint i);
ajint       ajFileWriteInt8 (AjPFile thys, ajlong i);
ajint       ajFileWriteStr  (AjPFile thys, const AjPStr str, ajuint len);

void        ajOutfileClose(AjPOutfile* pthis);
void        ajOutfileDel(AjPOutfile* pthis);
AjPFile     ajOutfileFile (const AjPOutfile thys);
AjPStr      ajOutfileFormat (const AjPOutfile thys);
FILE*       ajOutfileFp (const AjPOutfile thys);
AjPOutfile  ajOutfileNew(const AjPStr name);

/*
** End of prototype definitions
*/



/* ============= definitions =========================*/
#ifndef WIN32
#define AJ_FILE_R S_IRUSR
#define AJ_FILE_W S_IWUSR
#define AJ_FILE_X S_IXUSR
#else
#define AJ_FILE_R S_IREAD
#define AJ_FILE_W S_IWRITE
#define AJ_FILE_X S_IEXEC
#endif

#endif

#ifdef __cplusplus
}
#endif

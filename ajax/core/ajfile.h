#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfile_h
#define ajfile_h

#ifdef WIN32
#include "win32.h"
#endif

#include "ajlist.h"
#include <sys/stat.h>

#ifdef WIN32
#define pid_t void*
#else
#define HANDLE void*
#endif

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
** @attr fp [FILE*] C file pointer
** @attr Name [AjPStr] File name as used when opening
** @attr Printname [AjPStr] File name in a clean form for reporting
** @attr List [AjPList] List of file names (first is open)
** @attr End [AjBool] True if EOF has been reached
** @attr App [AjBool] True if file was opened for append. 
** @attr Buff [AjPStr] Buffer for latest line read
** @attr Workbuffer [char*] Block as a buffer for fgets etc
** @attr Readblock [char*] Block as a buffer for fread
** @attr Filepos [ajlong] File offset for start of latest read
** @attr Blocksize [ajuint] Read block maximum size
** @attr Blockpos [ajuint] Read block position
** @attr Blocklen [ajuint] Read block length used
** @attr Buffsize [ajuint] Buffer size (zero for default size)
** @attr Handle [ajint] AJAX file number 0 if unused
** @attr Pid [pid_t] Process PID if any (non-WIN32 only)
** @attr Process [HANDLE] Process handle (WIN32 only)
** @attr Thread [HANDLE] Thread handle (WIN32 only)
** @@
******************************************************************************/

typedef struct AjSFile
{
    FILE *fp;
    AjPStr Name;
    AjPStr Printname;
    AjPList List;
    AjBool End;
    AjBool App;
    AjPStr Buff;
    char *Workbuffer;
    char *Readblock;
    ajlong Filepos;
    ajuint Blocksize;
    ajuint Blockpos;
    ajuint Blocklen;
    ajuint Buffsize;
    ajint Handle;
    pid_t Pid;
    HANDLE Process;
    HANDLE Thread;
} AjOFile;

#define AjPFile AjOFile*




/* @data AjPFilebufflist ******************************************************
**
** Ajax file buffer, holding a simple linked list of buffered lines.
** This does not use the AjPList objects.
**
** This is a substructure of the AjPFilebuff object.
**
** @alias AjSFilebufflist
** @alias AjSOilebufflist
**
** @attr Line [AjPStr] String : this line
** @attr Next [struct AjSFilebufflist*] Next line in the list, NULL for last
** @attr Fpos [ajlong] File offset for start of this line
** @@
******************************************************************************/

typedef struct AjSFilebufflist {
  AjPStr Line;
  struct AjSFilebufflist* Next;
  ajlong Fpos;
} AjOFilebufflist;

#define AjPFilebufflist AjOFilebufflist*




/* @data AjPFilebuff **********************************************************
**
** Ajax buffered file object. Holds information for a buffered input file.
**
** @alias AjSFilebuff
** @alias AjOFilebuff
**
** @attr File [AjPFile] The input file - data to be buffered
** @attr Lines [AjPFilebufflist] All lines ... where the data really is
** @attr Freelines [AjPFilebufflist] Free list of lines for reuse
** @attr Curr [AjPFilebufflist] Current line in Lines list
** @attr Prev [AjPFilebufflist] Previous line (points to Curr for delete)
** @attr Last [AjPFilebufflist] Last line for quick appending
** @attr Freelast [AjPFilebufflist] Last free line for quick append
** @attr Nobuff [AjBool] if true, do not buffer the file
** @attr Pos [ajint] Position in list
** @attr Size [ajint] Size of list
** @attr FreeSize [ajint] Size of free list
** @attr Fpos [ajlong] File position in File
** @@
******************************************************************************/

typedef struct AjSFilebuff {
  AjPFile File;
  AjPFilebufflist Lines;
  AjPFilebufflist Freelines;
  AjPFilebufflist Curr;
  AjPFilebufflist Prev;
  AjPFilebufflist Last;
  AjPFilebufflist Freelast;
  AjBool Nobuff;
  ajint Pos;
  ajint Size;
  ajint FreeSize;
  ajlong Fpos;
} AjOFilebuff;

#define AjPFilebuff AjOFilebuff*




/* @data AjPDir **************************************************************
**
** Ajax directory object. Holds information for an open
** input directory.
**
** @alias AjSDir
** @alias AjODir
**
** @attr Name [AjPStr] Path
** @attr Printname [AjPStr] Directory path in a clean form for reporting
** @attr Prefix [AjPStr] Default filename prefix
** @attr Extension [AjPStr] Default file extension
** @@
******************************************************************************/

typedef struct AjSDir {
  AjPStr Name;
  AjPStr Printname;
  AjPStr Prefix;
  AjPStr Extension;
} AjODir;

#define AjPDir AjODir*




/* @data AjPDirout ***********************************************************
**
** Ajax output directory object. Holds information for an open
** output directory.
**
** @alias AjSDirout
** @alias AjODirout
**
** @attr Name [AjPStr] Path
** @attr Printname [AjPStr] Directory path in a clean form for reporting
** @attr Extension [AjPStr] Default file extension
** @@
******************************************************************************/

typedef struct AjSDirout {
  AjPStr Name;
  AjPStr Printname;
  AjPStr Extension;
} AjODirout;

#define AjPDirout AjODirout*




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
** @attr File [AjPFile] File object
** @attr Type [AjPStr] Named data file type
** @attr Formatstr [AjPStr] Format specific for this data type
** @attr Itype [ajuint] Index number for Type
** @attr Format [ajint] Index for Formatstr for this data type
** @attr Records [ajuint] Number of records written
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSOutfile {
    AjPFile File;
    AjPStr Type;
    AjPStr Formatstr;
    ajuint Itype;
    ajint Format;
    ajuint Records;
    char Padding[4];
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
    OUTFILE_OBO,			/* OBO ontology data */
    OUTFILE_PROPERTIES,			/* Phylogenetic properties */
    OUTFILE_RESOURCE,                   /* Data resource catalogue data */
    OUTFILE_SCOP,			/* SCOP data */
    OUTFILE_TAXON,			/* NCBI taxonomy data */
    OUTFILE_TEXT,			/* Text data */
    OUTFILE_TREE,			/* Phylogenetic tree data */
    OUTFILE_URL,			/* URL data */
    OUTFILE_VARIATION			/* Variation data */
};


/*
** Prototype definitions
*/

void           ajDirDel(AjPDir* pthis);
void           ajDiroutDel(AjPDirout* pthis);
const AjPStr   ajDirGetExt(const AjPDir thys);
const AjPStr   ajDirGetPath(const AjPDir thys);
const AjPStr   ajDirGetPrintpath(const AjPDir thys);
const AjPStr   ajDirGetPrefix(const AjPDir thys);
const AjPStr   ajDiroutGetExt(const AjPDirout thys);
const AjPStr   ajDiroutGetPath(const AjPDirout thys);
const AjPStr   ajDiroutGetPrintpath(const AjPDirout thys);
AjPDir         ajDirNewPath(const AjPStr path);
AjPDir         ajDirNewPathExt(const AjPStr path, const AjPStr ext);
AjPDir         ajDirNewPathPreExt(const AjPStr path, const AjPStr prefix,
                                const AjPStr ext);
AjBool         ajDiroutExists(AjPDirout thys);
AjPDirout      ajDiroutNewPath(const AjPStr name);
AjPDirout      ajDiroutNewPathExt(const AjPStr name, const AjPStr ext);
AjBool         ajDiroutOpen(AjPDirout thys);
ajint          ajFilelistAddDirectory(AjPList list,
                                      const AjPDir dir);
ajint          ajFilelistAddPathWildRecursiveIgnore(AjPList list,
						    const AjPStr path,
						    const AjPStr wildname,
						    AjPList ignorelist);
void           ajDirnamePrintRecursiveIgnore(const AjPStr path,
					     AjPList ignorelist,
					     AjPFile outfile);
ajint          ajFilelistAddPath(AjPList list,
				 const AjPStr path);
ajint          ajFilelistAddPathDir(AjPList list,
                                    const AjPStr path);
ajint          ajFilelistAddPathWild(AjPList list,
				     const AjPStr path,
				     const AjPStr filename);
ajint          ajFilelistAddPathWildDir(AjPList list,
				     const AjPStr path,
				     const AjPStr filename);

void           ajFilebuffDel(AjPFilebuff* pthis);
AjPFilebuff    ajFilebuffNewNofile(void);
AjPFilebuff    ajFilebuffNewNameS(const AjPStr name);
AjPFilebuff    ajFilebuffNewNamePathC(const char* filename, const AjPStr dir);
AjPFilebuff    ajFilebuffNewNamePathS(const AjPStr filename, const AjPStr dir);
AjPFilebuff    ajFilebuffNewPathWild(const AjPStr path,
                                     const AjPStr wildname);
AjPFilebuff    ajFilebuffNewPathWildExclude(const AjPStr path,
                                            const AjPStr wildname,
                                            const AjPStr exclude);
AjPFilebuff    ajFilebuffNewFromCfile(FILE *fp);
AjPFilebuff    ajFilebuffNewFromFile(AjPFile file);
AjPFilebuff    ajFilebuffNewLine(const AjPStr line);
AjPFilebuff    ajFilebuffNewListinList(AjPList list);
AjBool         ajFilebuffSetBuffered(AjPFilebuff thys);
void           ajFilebuffClear(AjPFilebuff thys, ajint lines);
void           ajFilebuffClearStore(AjPFilebuff thys, ajint lines,
                                    const AjPStr rdline, AjBool store,
                                    AjPStr *astr);
AjBool         ajFilebuffIsEmpty(const AjPFilebuff thys);
AjBool         ajFilebuffIsEnded(const AjPFilebuff thys);
AjBool         ajFilebuffIsEof(const AjPFilebuff thys);
AjPFile        ajFilebuffGetFile(const AjPFilebuff thys);
void           ajFilebuffFix(AjPFilebuff thys);
FILE*          ajFilebuffGetFileptr(const AjPFilebuff thys);
const AjPStr   ajFilebuffGetFirst(const AjPFilebuff thys);
AjBool         ajFilebuffIsBuffered(const AjPFilebuff thys);
void           ajFilebuffLoadAll(AjPFilebuff buff);
AjBool         ajFilebuffLoadReadurl(AjPFilebuff buff, const AjPStr url);
void           ajFilebuffLoadC(AjPFilebuff thys, const char* str);
void           ajFilebuffLoadS(AjPFilebuff thys, const AjPStr str);
AjBool         ajFilebuffSetUnbuffered(AjPFilebuff thys);
void           ajFilebuffTraceTitle(const AjPFilebuff thys, const char* title);
void           ajFilebuffReset(AjPFilebuff thys);
void           ajFilebuffResetPos(AjPFilebuff thys);
void           ajFilebuffResetStore(AjPFilebuff thys,
                                    AjBool store, AjPStr *astr);
AjBool         ajFilebuffReopenFile(AjPFilebuff* Pbuff, AjPFile file);
ajuint         ajFilebuffHtmlNoheader(AjPFilebuff buff);
void           ajFilebuffHtmlStrip(AjPFilebuff thys);
AjBool         ajFilebuffHtmlPre(AjPFilebuff thys);
void           ajFilebuffTrace(const AjPFilebuff thys);
void           ajFilebuffTraceFull(const AjPFilebuff thys, size_t nlines,
                                   size_t nfree);
void           ajFileClose(AjPFile *pthis);
AjBool         ajDirnameFixExists(AjPStr* dir);
void           ajDirnameFix(AjPStr* dir);
AjBool         ajDirnameUp(AjPStr* dir);
AjBool         ajDirnameFillPath(AjPStr* dir);
AjBool         ajFileExtnTrim(AjPStr* name);
AjBool         ajFileDirExtnTrim(AjPStr* name);
AjBool         ajFileIsEof(const AjPFile thys);
void           ajFileExit(void);
ajint          ajFilelistAddListname(AjPList list, const AjPStr files);
FILE*          ajFileGetFileptr(const AjPFile thys);
const AjPStr   ajFileValueCwd(void);
AjBool         ajFileIsAppend(const AjPFile thys);
const AjPStr   ajFileGetName(const AjPFile thys);
AjBool         ajFilenameHasPath(const AjPStr name);
ajlong         ajFilenameGetSize(const AjPStr fname);
AjBool         ajFilenameTrimAll(AjPStr *fname);
AjBool         ajFilenameTrimExt(AjPStr* Pfilename);
AjBool         ajFilenameTrimPath(AjPStr* Pfilename);
AjBool         ajFilenameTrimPathExt(AjPStr* Pfilename);
const char*    ajFileGetNameC(const AjPFile thys);
const AjPStr   ajFileGetNameS(const AjPFile thys);
const char*    ajFileGetPrintnameC(const AjPFile thys);
const AjPStr   ajFileGetPrintnameS(const AjPFile thys);
AjBool         ajFilenameReplacePathS(AjPStr* filename, const AjPStr dir);
AjBool         ajFilenameReplacePathC(AjPStr* filename, const char* dir);
AjBool         ajFilenameReplaceExtC(AjPStr* filename, const char* extension);
AjBool         ajFilenameReplaceExtS(AjPStr* filename,
                                     const AjPStr extension);
AjBool         ajFilenameSetExtC(AjPStr* filename, const char* extension);
AjBool         ajFilenameSetExtS(AjPStr* filename,
                                 const AjPStr extension);
AjPFile        ajFileNewOutappendNameS(const AjPStr name);
AjPFile        ajFileNewListinList(AjPList list);
AjPFile        ajFileNewListinDirPre(const AjPDir dir, const AjPStr prefix);
AjPFile        ajFileNewListinNameDirS(const AjPStr name, const AjPDir dir);
AjPFile        ajFileNewListinPathWild(const AjPStr path,
                                       const AjPStr wildname);
AjPFile        ajFileNewListinPathWildExclude(const AjPStr path,
                                              const AjPStr wildname,
                                              const AjPStr exclude);

AjPFile        ajFileNewFromCfile(FILE* file);
AjPFile        ajFileNewInNameC(const char *name);
AjPFile        ajFileNewInNameS(const AjPStr name);
AjPFile        ajFileNewInBlockS(const AjPStr name, ajuint blocksize);
AjPFile        ajFileNewInPipe(const AjPStr name);
AjPFile        ajFileNewInNamePathC(const char* name, const AjPStr path);
AjPFile        ajFileNewInNamePathS(const AjPStr name, const AjPStr path);
AjPFile        ajFileNewOutNameC(const char *name);
AjPFile        ajFileNewOutNameS(const AjPStr name);
AjPFile        ajFileNewOutNameDirS(const AjPStr name, const AjPDirout dir);
AjPFile        ajFileNewOutNamePathS(const AjPStr name, const AjPStr path);
void           ajFileOutHeader(AjPFile thys);
AjBool         ajFileReopenName(AjPFile thys, const AjPStr name);
AjBool         ajFileReopenNext(AjPFile thys);
ajint 	       ajFileScan(const AjPStr path, const AjPStr filename,
                          AjPList *result,
                          AjBool show, AjBool dolist, AjPList *list,
                          AjPList rlist, AjBool recurs, AjPFile outf);
ajint          ajFileSeek(AjPFile thys, ajlong offset, ajint wherefrom);
AjBool         ajFilenameExists(const AjPStr filename);
AjBool         ajFilenameExistsDir(const AjPStr filename);
AjBool         ajFilenameExistsExec(const AjPStr filename);
AjBool         ajFilenameExistsRead(const AjPStr filename);
AjBool         ajFilenameExistsWrite(const AjPStr filename);
AjBool         ajFileValueRedirectStderr(void);
AjBool         ajFileValueRedirectStdin(void);
AjBool         ajFileValueRedirectStdout(void);
AjBool         ajFileIsFile(const AjPFile file);
AjBool         ajFileIsStderr(const AjPFile file);
AjBool         ajFileIsStdin(const AjPFile file);
AjBool         ajFileIsStdout(const AjPFile file);
AjBool         ajFileFix(AjPFile file);
AjBool         ajFileResetEof(AjPFile thys);
ajlong         ajFileResetPos(AjPFile thys);
AjBool         ajFilenameSetTempname(AjPStr* Pfilename);
AjBool         ajFilenameSetTempnamePathC(AjPStr* Pfilename,const char* txt);
AjBool         ajFilenameSetTempnamePathS(AjPStr* Pfilename, const AjPStr str);
AjBool         ajFilenameTestExclude(const AjPStr filename,
				     const AjPStr exclude,
				     const AjPStr include);
AjBool         ajFilenameTestInclude(const AjPStr filename,
				     const AjPStr exclude,
				     const AjPStr include);
AjBool         ajFilenameTestExcludePath(const AjPStr filename,
					 const AjPStr exclude,
					 const AjPStr include);
AjBool         ajFilenameTestIncludePath(const AjPStr filename,
					 const AjPStr exclude,
					 const AjPStr include);
void           ajFileTrace(const AjPFile thys);
void           ajFileSetUnbuffer(AjPFile thys);

ajuint         ajFileValueBuffsize(void);


AjPOutfile     ajOutfileNewNameS(const AjPStr name);
void           ajOutfileClose(AjPOutfile* pthis);
AjPFile        ajOutfileGetFile(const AjPOutfile thys);
FILE*          ajOutfileGetFileptr(const AjPOutfile thys);
const AjPStr   ajOutfileGetFormat(const AjPOutfile thys);
ajuint         ajOutfileGetFormatindex (const AjPOutfile file);

/*
** End of prototype definitions
*/

__deprecated AjBool         ajFileDirTrim(AjPStr* name);
__deprecated ajint          ajFileBuffSize(void);
__deprecated void           ajFileBuffClearStore(AjPFilebuff buff, ajint lines,
                                                 const AjPStr rdline,
                                                 AjBool store, AjPStr *astr);
__deprecated AjBool         ajFileBuffBuff (AjPFilebuff thys);
__deprecated AjBool         ajFileBuffEmpty (const AjPFilebuff thys);
__deprecated AjBool         ajFileBuffEnd (const AjPFilebuff thys);
__deprecated AjBool         ajFileBuffEof (const AjPFilebuff thys);
__deprecated AjPFile        ajFileBuffFile (const AjPFilebuff thys);
__deprecated void           ajFileBuffFix (AjPFilebuff thys);
__deprecated AjBool         ajFileBuffIsBuffered (const AjPFilebuff thys);
__deprecated FILE*          ajFileBuffFp (const AjPFilebuff thys);
__deprecated void           ajFileBuffTrace (const AjPFilebuff thys);
__deprecated void           ajFileBuffLoadC(AjPFilebuff thys, const char* str);
__deprecated void           ajFileBuffLoadS(AjPFilebuff thys, const AjPStr str);
__deprecated AjBool         ajFileBuffNobuff (AjPFilebuff thys);
__deprecated void           ajFileBuffPrint (const AjPFilebuff thys,
                                             const char* title);
__deprecated void           ajFileBuffReset (AjPFilebuff thys);
__deprecated void           ajFileBuffResetPos (AjPFilebuff thys);
__deprecated void           ajFileBuffResetStore (AjPFilebuff thys,
                                                  AjBool store, AjPStr *astr);
__deprecated void           ajFileBuffTraceFull (const AjPFilebuff thys,
                                                 size_t nlines,
                                  size_t nfree);
__deprecated AjBool         ajFileGetwd (AjPStr* dir);
__deprecated AjBool         ajFileHasDir (const AjPStr name);
__deprecated ajlong         ajFileLength (const AjPStr fname);
__deprecated AjBool         ajFileNameShorten(AjPStr *fname);
__deprecated AjBool         ajFileNameTrim(AjPStr *fname);
__deprecated const char*    ajFileTempName (const char *dir);
__deprecated AjBool         ajFileTestSkip (const AjPStr fullname,
                             const AjPStr exc, const AjPStr inc,
                             AjBool keep, AjBool ignoredirectory);
__deprecated AjBool         ajFileDir (AjPStr* dir);
__deprecated void           ajFileDirFix (AjPStr* dir);
__deprecated AjBool         ajFileDirPath (AjPStr* dir);
__deprecated AjBool         ajFileDirUp (AjPStr* dir);
__deprecated ajint          ajDirScan(const AjPStr path,
				      const AjPStr filename,
				      AjPList *result);
    
__deprecated AjBool         ajFileSetDir (AjPStr *pname, const AjPStr dir);
__deprecated FILE*          ajOutfileFp (const AjPOutfile thys);
__deprecated AjBool         ajFileNameValid (const AjPStr filename);
__deprecated AjBool         ajFileStat (const AjPStr filename, ajint mode);
__deprecated AjPFile        ajOutfileFile (const AjPOutfile thys);
__deprecated AjPStr         ajOutfileFormat (const AjPOutfile thys);
__deprecated void           ajOutfileDel(AjPOutfile* pthis);
__deprecated AjPOutfile     ajOutfileNew(const AjPStr name);
__deprecated void           ajFileBuffStripHtml (AjPFilebuff thys);
__deprecated AjBool         ajFileBuffStripHtmlPre (AjPFilebuff thys);
__deprecated const AjPStr   ajFileNameS (const AjPFile thys);
__deprecated const char*    ajFileName (const AjPFile thys);
__deprecated AjPFilebuff ajFileBuffNewS(const AjPStr data);
__deprecated AjPFilebuff ajFileBuffNewList(AjPList list);
__deprecated AjBool         ajFileEof (const AjPFile thys);
__deprecated FILE*          ajFileFp (const AjPFile thys);
__deprecated AjBool         ajFileStderr (const AjPFile file);
__deprecated AjBool         ajFileStdin (const AjPFile file);
__deprecated AjBool         ajFileStdout (const AjPFile file);
__deprecated void           ajFileBuffDel (AjPFilebuff* pthis);
__deprecated AjPFilebuff    ajFileBuffNew (void);
__deprecated AjPFilebuff    ajFileBuffNewF (FILE *fp);
__deprecated void           ajFileBuffClear (AjPFilebuff thys, ajint lines);
__deprecated AjBool ajFileBuffSetFile (AjPFilebuff* pthys, AjPFile file,
                                       AjBool samefile);
__deprecated AjBool         ajFileGetApp (const AjPFile thys);
__deprecated ajlong         ajFileTell (AjPFile thys);
__deprecated AjBool         ajFileNext (AjPFile thys);
__deprecated FILE*          ajFileReopen (AjPFile thys, const AjPStr name);
__deprecated const AjPStr      ajDirExt(const AjPDir thys);
__deprecated const AjPStr      ajDirName(const AjPDir thys);
__deprecated AjPDir      ajDirNew (const AjPStr name);
__deprecated AjPDir      ajDirNewS (const AjPStr name, const AjPStr ext);
__deprecated AjPDir      ajDirNewSS (const AjPStr name, const AjPStr prefix,
			const AjPStr ext);
__deprecated AjPFile     ajFileNew(void);
__deprecated AjPFilebuff ajFileBuffNewIn(const AjPStr name);
__deprecated AjPFile     ajFileNewIn(const AjPStr name);
__deprecated AjPFile     ajFileNewInList(AjPList list);
__deprecated AjPFile     ajFileNewDW(const AjPStr dir, const AjPStr wildfile);
__deprecated AjPFile     ajFileNewDWE(const AjPStr dir, const AjPStr wildfile,
                                      const AjPStr exclude);


__deprecated AjPFilebuff ajFileBuffNewFile(AjPFile file);
__deprecated AjPFilebuff ajFileBuffNewDW(const AjPStr dir,
                                         const AjPStr wildfile);
__deprecated AjPFilebuff ajFileBuffNewDWE(const AjPStr dir,
                                          const AjPStr wildfile,
                                          const AjPStr exclude);

__deprecated AjPFile     ajFileNewApp(const AjPStr name);

__deprecated AjPFile ajFileNewOut(const AjPStr name);

__deprecated AjPFile     ajFileNewOutD (const AjPStr dir, const AjPStr name);
__deprecated AjPFile     ajFileNewOutDir(const AjPDirout dir,
                                         const AjPStr name);
__deprecated AjPFile     ajFileNewF (FILE* file);
__deprecated AjPDir ajDirOutNew(const AjPStr name);
__deprecated AjPDir ajDirOutNewS(const AjPStr name, const AjPStr ext);
__deprecated AjPDir ajDirOutNewSS(const AjPStr name,
                                  const AjPStr prefix, const AjPStr ext);

__deprecated AjPFilebuff  ajFileBuffNewDC (const AjPStr dir,
                                           const char* filename);
__deprecated AjPFilebuff  ajFileBuffNewDF (const AjPStr dir,
                                           const AjPStr filename);
__deprecated AjPFile      ajFileNewDC (const AjPStr dir,
                                       const char* filename);
__deprecated AjPFile      ajFileNewDF (const AjPStr dir,
                                       const AjPStr filename);

__deprecated AjBool       ajFileNameDirSet  (AjPStr* filename,
                                             const AjPStr dir);
__deprecated AjBool       ajFileNameDirSetC (AjPStr* filename,
                                             const char* dir);
__deprecated AjBool       ajFileNameExt  (AjPStr* filename,
                                          const AjPStr extension);
__deprecated AjBool       ajFileNameExtC (AjPStr* filename,
                                          const char* extension);

__deprecated AjPFile      ajFileNewDirF(const AjPDir dir,
                                        const AjPStr filename);

__deprecated void         ajFileUnbuffer (AjPFile thys);
__deprecated AjPFile        ajFileNewInC(const char *name);

#define    MAJFILETELL(file) (file->fp ? ftell(file->fp) : 0L)

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

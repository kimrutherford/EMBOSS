#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtextdata_h
#define ajtextdata_h


#define NULLFPOS -1

enum AjETextqryType {TEXTQRY_UNKNOWN, TEXTQRY_ENTRY,
                     TEXTQRY_QUERY, TEXTQRY_ALL};

typedef enum {
    TEXTQRY_INIT, TEXTQRY_OR,
    TEXTQRY_AND, TEXTQRY_NOT, TEXTQRY_EOR
}
    AjETextqryLink;

typedef struct AjSTextAccess AjSTextAccess;



/* @data AjPText **************************************************************
**
** Ajax Text data object.
**
** Holds the input specification and information needed to read
** the entry text and possible further entries
**
** @alias AjSText
** @alias AjOText
**
** @attr Id        [AjPStr]  Id of text item
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Lines     [AjPList] Lines of input data
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSText {
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPList Lines;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjOText;

#define AjPText AjOText*




/* @data AjPTextin *************************************************************
**
** Ajax Text Input object.
**
** Holds the input specification and information needed to read
** the entry text and possible further entries
**
** @alias AjSTextin
** @alias AjOTextin
**
** @attr Db [AjPStr] Database name (from commandline, replace on reading)
** @attr Qry [AjPStr] Querystring for the entry
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr List [AjPList] List of USAs to be read
** @attr Filebuff [AjPFilebuff] Input buffered file
** @attr Fpos [ajlong] File position (fseek) for building USA
** @attr Query [AjPQuery] Query data
** @attr TextData [void*] Format data for reuse, e.g. multiple entry input
** @attr Search [AjBool] Search for more entries (always true?)
** @attr Single [AjBool] Read single entries
** @attr Multi [AjBool] Multiple entry input
** @attr CaseId [AjBool] Id case sensitive (default false)
** @attr Text [AjBool] true: save full text of entry
** @attr ChunkEntries [AjBool] true: access method returns entries in chunks
**                             and should be called again when input is empty
** @attr Count [ajuint] count of entries so far. Used when ACD reads first
**                     entry and we need to reuse it in a Next loop
** @attr Filecount [ajuint] Number of files read
** @attr Entrycount [ajuint] Number of entries in file
** @attr Records [ajuint] Records processed
** @attr Format [AjEnum] Text input format enum
** @attr TextFormat [AjEnum] Text input format enum
** @@
******************************************************************************/

typedef struct AjSTextin {
  AjPStr Db;
  AjPStr Qry;
  AjPStr Formatstr;
  AjPStr Filename;
  AjPList List;
  AjPFilebuff Filebuff;
  ajlong Fpos;
  AjPQuery Query;
  void *TextData;
  AjBool Search;
  AjBool Single;
  AjBool Multi;
  AjBool CaseId;
  AjBool Text;
  AjBool ChunkEntries;
  ajuint Count;
  ajuint Filecount;
  ajuint Entrycount;
  ajuint Records;
  AjEnum Format;
  AjEnum TextFormat;
} AjOTextin;

#define AjPTextin AjOTextin*




/* @data AjPTextall ***********************************************************
**
** Ajax text data all (stream) object.
**
** Inherits an AjPText but allows more text blocks to be read from the
** same input by also inheriting the AjPTextin input object.
**
** @alias AjSTextall
** @alias AjOTextall
**
** @attr Text [AjPText] Current text block
** @attr Textin [AjPTextin] Text input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Text object has been returned to a new owner
**                         and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTextall
{
    AjPText Text;
    AjPTextin Textin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOTextall;

#define AjPTextall AjOTextall*



/* @data AjPTextAccess *********************************************************
**
** Ajax Text Access database reading object.
**
** Holds information needed to read a text entry from a database.
** Access methods are defined for each known database type.
**
** Text entries are read from the database using the defined
** database access function, which is usually a static function
** within ajtextdb.c
**
** This should be a static data object but is needed for the definition
** of AjPTextin.
**
** @alias AjSTextAccess
** @alias AjOTextAccess
**
** @attr Name [const char*] Access method name used in emboss.default
** @attr Access [(AjBool*)] Access function
** @attr AccessFree [(AjBool*)] Access cleanup function
** @attr Qlink [const char*] Supported query link operators
** @attr Desc [const char*] Description
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Chunked [AjBool] Supports retrieval of entries in chunks
** @@
******************************************************************************/

typedef struct AjSTextAccess
{
    const char *Name;
    AjBool (*Access) (AjPTextin textin);
    AjBool (*AccessFree) (void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
} AjOTextAccess;

#define AjPTextAccess AjOTextAccess*




#endif

#ifdef __cplusplus
}
#endif

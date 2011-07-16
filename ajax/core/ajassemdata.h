#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajassemdata_h
#define ajassemdata_h


#define NULLFPOS -1

typedef struct AjSAssemAccess AjSAssemAccess;



/* @data AjPAssem *************************************************************
**
** Ajax Assembly object.
**
** Holds the assembly itself, plus associated information.
**
** @alias AjSAssem
** @alias AjOAssem
**
** @attr Id        [AjPStr]  Id of term
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Textptr   [AjPStr]  Full text
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSAssem {
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  Textptr;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjOAssem;

#define AjPAssem AjOAssem*




/* @data AjPAssemin ************************************************************
**
** Ajax Assembly Input object.
**
** Holds the input specification and information needed to read
** the assembly and possible further entries
**
** @alias AjSAssemin
** @alias AjOAssemin
**
** @attr Input [AjPTextin] General text input object
** @attr AssemData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @@
******************************************************************************/

typedef struct AjSAssemin {
    AjPTextin Input;
    void *AssemData;
} AjOAssemin;

#define AjPAssemin AjOAssemin*




/* @data AjPAssemAccess ********************************************************
**
** Ajax assembly access database reading object.
**
** Holds information needed to read an assembly entry from a database.
** Access methods are defined for each known database type.
**
** Assembly entries are read from the database using the defined
** database access function, which is usually a static function
** within ajassemdb.c
**
** This should be a static data object but is needed for the definition
** of AjPAssemin.
**
** @alias AjSAssemAccess
** @alias AjOAssemAccess
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

typedef struct AjSAssemAccess
{
    const char *Name;
    AjBool (*Access) (AjPAssemin assemin);
    AjBool (*AccessFree) (void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
} AjOAssemAccess;

#define AjPAssemAccess AjOAssemAccess*




#endif

#ifdef __cplusplus
}
#endif

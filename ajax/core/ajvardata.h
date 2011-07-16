#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajvardata_h
#define ajvardata_h


#define NULLFPOS -1

typedef struct AjSVarAccess AjSVarAccess;



/* @data AjPVar *************************************************************
**
** Ajax var object.
**
** Holds the var itself, plus associated information.
**
** @alias AjSVar
** @alias AjOVar
**
** @attr Id        [AjPStr]  Id of term
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Lines     [AjPStr*]  Full text
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSVar {
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr *Lines;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjOVar;

#define AjPVar AjOVar*




/* @data AjPVarin ************************************************************
**
** Ajax var input object.
**
** Holds the input specification and information needed to read
** the var and possible further entries
**
** @alias AjSVarin
** @alias AjOVarin
**
** @attr Input [AjPTextin] General text input object
** @attr VarData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @@
******************************************************************************/

typedef struct AjSVarin {
    AjPTextin Input;
    void *VarData;
} AjOVarin;

#define AjPVarin AjOVarin*




/* @data AjPVarall ************************************************************
**
** Ajax variation all (stream) object.
**
** Inherits an AjPVar but allows more variations to be read from the
** same input by also inheriting the AjPVarin input object.
**
** @alias AjSVarall
** @alias AjOVarall
**
** @attr Var [AjPVar] Current variation
** @attr Varin [AjPVarin] Variation input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Variation object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSVarall
{
    AjPVar Var;
    AjPVarin Varin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOVarall;

#define AjPVarall AjOVarall*



/* @data AjPVarAccess ********************************************************
**
** Ajax variation access database reading object.
**
** Holds information needed to read a var entry from a database.
** Access methods are defined for each known database type.
**
** Var entries are read from the database using the defined
** database access function, which is usually a static function
** within ajvardb.c
**
** This should be a static data object but is needed for the definition
** of AjPVarin.
**
** @alias AjSVarAccess
** @alias AjOVarAccess
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

typedef struct AjSVarAccess
{
    const char *Name;
    AjBool (*Access) (AjPVarin varin);
    AjBool (*AccessFree) (void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
} AjOVarAccess;

#define AjPVarAccess AjOVarAccess*




#endif

#ifdef __cplusplus
}
#endif

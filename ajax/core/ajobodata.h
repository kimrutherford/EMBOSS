/* @include ajobodata *********************************************************
**
** AJAX OBO data structures
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.8 $
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added query and reading functions
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJOBODATA_H
#define AJOBODATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPOboin *************************************************************
**
** Ajax Obo Input object.
**
** Holds the obo term specification and information needed to read
** the obo term and possible further terms.
**
** @alias AjSOboin
** @alias AjOOboin
**
** @attr Input [AjPTextin] General text input object
** @attr OboData [void*] Format data for reuse, e.g. multiple term input
**                       (unused in current code)
** @@
******************************************************************************/

typedef struct AjSOboin
{
    AjPTextin Input;
    void *OboData;
} AjOOboin;

#define AjPOboin AjOOboin*




/* @data AjPOboAccess *********************************************************
**
** Ajax Obo Access database reading object.
**
** Holds information needed to read an obo term from a database.
** Access methods are defined for each known database type.
**
** Obo terms are read from the database using the defined
** database access function, which is usually a static function
** within ajobodb.c
**
** This should be a static data object but is needed for the definition
** of AjPOboin.
**
** @alias AjSOboAccess
** @alias AjOOboAccess
**
** @new ajOboMethod returns a copy of a known access method definition.
** @other AjPOboin Obo input
**
** @attr Name [const char*] Access method name used in emboss.default
** @attr Access [AjBool function] Access function
** @attr AccessFree [AjBool function] Access cleanup function
** @attr Qlink [const char*] Supported query link operators
** @attr Desc [const char*] Description
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Chunked [AjBool] Supports retrieval of entries in chunks
** @attr Padding [AjBool] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSOboAccess
{
    const char *Name;
    AjBool (*Access)(AjPOboin oboin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOOboAccess;

#define AjPOboAccess AjOOboAccess*




/* @data AjPOboData ***********************************************************
**
** OBO parsed data
**
** @attr Termtable [AjPTable] Table of terms by id
** @attr Termnametable [AjPTable] Table of terms by name
** @attr Typedeftable [AjPTable] Table of typedefs
** @attr Instancetable [AjPTable] Table of instances
** @attr Annotable [AjPTable] Table of annotation stanzas
** @attr Formulatable [AjPTable] Table of formula stanzas
** @attr Misctable [AjPTable] Table of other stanzas
** @@
******************************************************************************/

typedef struct AjSOboData
{
    AjPTable Termtable;
    AjPTable Termnametable;
    AjPTable Typedeftable;
    AjPTable Instancetable;
    AjPTable Annotable;
    AjPTable Formulatable;
    AjPTable Misctable;
} AjOOboData;

#define AjPOboData AjOOboData*




/* @data AjPObotag ************************************************************
**
** OBO tag name, value and comment.
**
** @alias AjSObotag
** @alias AjOObotag
**
** @attr Name [AjPStr] Tag name
** @attr Value [AjPStr] Tag value
** @attr Modifier [AjPStr] Tag modifier
** @attr Comment [AjPStr] Tag comment
** @attr Linenumber [ajuint] OBO file line number
** @attr Padding [ajuint] Padding to align structure
** @@
******************************************************************************/

typedef struct AjSObotag
{
    AjPStr Name;
    AjPStr Value;
    AjPStr Modifier;
    AjPStr Comment;
    ajuint Linenumber;
    ajuint Padding;
} AjOObotag;

#define AjPObotag AjOObotag*




/* @data AjPOboxref ***********************************************************
**
** OBO dbxref name and description
**
** @alias AjSOboxref
** @alias AjOOboxref
**
** @attr Name [AjPStr] Dbxref name
** @attr Desc [AjPStr] Description (optional)
** @@
******************************************************************************/

typedef struct AjSOboxref
{
    AjPStr Name;
    AjPStr Desc;
} AjOOboxref;

#define AjPOboxref AjOOboxref*




/* @data AjPOboalias **********************************************************
**
** Alias name for an OBO identifier
**
** @alias AjSOboalias
** @alias AjOOboalias
**
** @attr Alias     [AjPStr]  Alias name of term
** @attr Id        [AjPStr]  Id of term
******************************************************************************/

typedef struct AjSOboalias
{
    AjPStr  Alias;
    AjPStr  Id;
} AjOOboalias;

#define AjPOboalias AjOOboalias*




/* @data AjPObo ***************************************************************
**
** Ajax obo term object.
**
** Holds the term itself, plus associated information.
**
** @alias AjOObo
** @alias AjSObo
**
** @attr Id        [AjPStr]  Id of term
** @attr Trueid    [AjPStr]  True id of an alt_id
** @attr Fullid    [AjPStr]  Id of term with prefix
** @attr Name      [AjPStr]  Name of term
** @attr Namespace [AjPStr]  Namespace of term
** @attr Def       [AjPStr]  Definition of term
** @attr Dbxrefs   [AjPList] List of AjPOboxref names descriptions and modifiers
** @attr Comment   [AjPStr]  Comment tag in term stanza
** @attr Replaced  [AjPStr]  Replacement term for an obsolete term
** @attr Taglist   [AjPList]  List of other AjPObotag tag name-value pairs
** @attr Obsolete  [AjBool]  True if term is obsolete
** @attr Builtin   [AjBool]  True if term is an OBO Built-in term
** @attr Db [AjPStr] Database name from input
** @attr Setdb [AjPStr] Database name from command line
** @attr Full [AjPStr] Full name
** @attr Qry [AjPStr] Query for re-reading
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr TextPtr [AjPStr] Full text
** @attr Fpos [ajlong] File position (fseek) for Query
** @attr Format [AjEnum] Input format enum
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSObo
{
    AjPStr  Id;
    AjPStr  Trueid;
    AjPStr  Fullid;
    AjPStr  Name;
    AjPStr  Namespace;
    AjPStr  Def;
    AjPList Dbxrefs;
    AjPStr  Comment;
    AjPStr  Replaced;
    AjPList Taglist;
    AjBool  Obsolete;
    AjBool  Builtin;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  TextPtr;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Padding;
} AjOObo;

#define AjPObo AjOObo*




/* @data AjPOboall ************************************************************
**
** Ajax obo term all (stream) object.
**
** Inherits an AjPObo but allows more obo terms to be read from the
** same input by also inheriting the AjPOboin input object.
**
** @alias AjSOboall
** @alias AjOOboall
**
** @attr Obo [AjPObo] Current term
** @attr Oboin [AjPOboin] Obo input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Obo object has been returned to a new owner
**                         and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSOboall
{
    AjPObo Obo;
    AjPOboin Oboin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOOboall;

#define AjPOboall AjOOboall*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJOBODATA_H */

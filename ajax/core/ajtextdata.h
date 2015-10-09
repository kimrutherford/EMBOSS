/* @include ajtextdata ********************************************************
**
** AJAX TEXT data structures
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.15 $
** @modified Oct 5 pmr First version
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

#ifndef AJTEXTDATA_H
#define AJTEXTDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajfile.h"
#include "ajquerydata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @enum AjETextqryType *******************************************************
**
** Enumerated text query type
**
** @value TEXTQRY_UNKNOWN Unknown
** @value TEXTQRY_ENTRY   Single entry
** @value TEXTQRY_QUERY   Query
** @value TEXTQRY_ALL     All entries
******************************************************************************/

typedef enum AjOTextqryType
{
    TEXTQRY_UNKNOWN,
    TEXTQRY_ENTRY,
    TEXTQRY_QUERY,
    TEXTQRY_ALL
} AjETextqryType;




/* @enum AjETextqryLink *******************************************************
**
** Text query link operators
**
** @value TEXTQRY_INIT Undefined
** @value TEXTQRY_OR   Current OR next
** @value TEXTQRY_AND  Current AND next
** @value TEXTQRY_NOT  Current but NOT next
** @value TEXTQRY_EOR  Current EOR next
** @value TEXTQRY_MAX  Beyond last defined value
******************************************************************************/

typedef enum AjOTextqryLink
{
    TEXTQRY_INIT,
    TEXTQRY_OR,
    TEXTQRY_AND,
    TEXTQRY_NOT,
    TEXTQRY_EOR,
    TEXTQRY_MAX
} AjETextqryLink;




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

typedef struct AjSText
{
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




/* @data AjPTextin ************************************************************
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
** @attr QryFields [AjPStr] Query fields or ID list
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
** @attr Dataread [AjBool] true: input through a loader has started to read
** @attr Datadone [AjBool] true: input through a loader has read everything
** @attr Count [ajuint] count of entries so far. Used when ACD reads first
**                     entry and we need to reuse it in a Next loop
** @attr Filecount [ajuint] Number of files read
** @attr Entrycount [ajuint] Number of entries in file
** @attr Datacount [ajuint] Number of data values read in current entry
** @attr Records [ajuint] Records processed
** @attr Format [AjEnum] Text input format enum
** @attr TextFormat [AjEnum] Text input format enum
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTextin
{
    AjPStr Db;
    AjPStr Qry;
    AjPStr Formatstr;
    AjPStr QryFields;
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
    AjBool Dataread;
    AjBool Datadone;
    ajuint Count;
    ajuint Filecount;
    ajuint Entrycount;
    ajuint Datacount;
    ajuint Records;
    AjEnum Format;
    AjEnum TextFormat;
    ajuint Padding;
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




/* @data AjPTextAccess ********************************************************
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
** @attr Access [AjBool function] Access function
** @attr AccessFree [AjBool function] Access cleanup function
** @attr Qlink [const char*] Supported query link operators
** @attr Desc [const char*] Description
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Chunked [AjBool] Supports retrieval of entries in chunks
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTextAccess
{
    const char *Name;
    AjBool (*Access)(AjPTextin textin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    ajuint Padding;
} AjOTextAccess;

#define AjPTextAccess AjOTextAccess*




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

#endif /* !AJTEXTDATA_H */

/* @include ajurldata *********************************************************
**
** AJAX url data structures
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.7 $
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

#ifndef AJURLDATA_H
#define AJURLDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"
#include "ajresourcedata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPUrl ***************************************************************
**
** Ajax url object.
**
** Holds the url itself, plus associated information.
**
** @alias AjSUrl
** @alias AjOUrl
**
** @attr Id        [AjPStr]  Id of term
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Lines     [AjPStr*]  Full text
** @attr Resqry    [AjPResquery]  Resource query
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSUrl
{
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr *Lines;
    AjPResquery Resqry;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjOUrl;

#define AjPUrl AjOUrl*




/* @data AjPUrlin *************************************************************
**
** Ajax url input object.
**
** Holds the input specification and information needed to read
** the url and possible further entries
**
** @alias AjSUrlin
** @alias AjOUrlin
**
** @attr Input [AjPTextin] General text input object
** @attr Resource [AjPResource] Resource object
** @attr UrlList [AjPList] Result URLs list
** @attr QryList [AjPList] Resource query objects for result URLs list
** @attr UrlData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @attr Identifiers [AjPStr] Identifiers of source entry
** @attr Accession [AjPStr] Accession number of source entry
** @attr IdTypes [AjPStr] Identifier types in DRCAT
** @attr IsSwiss [AjBool] True if URL is from a SwissProt reference
** @attr IsEmbl [AjBool] True if URL is from an Embl/GenBank/DDBJ reference
** @@
******************************************************************************/

typedef struct AjSUrlin
{
    AjPTextin Input;
    AjPResource Resource;
    AjPList UrlList;
    AjPList QryList;
    void *UrlData;
    AjPStr Identifiers;
    AjPStr Accession;
    AjPStr IdTypes;
    AjBool IsSwiss;
    AjBool IsEmbl;
} AjOUrlin;

#define AjPUrlin AjOUrlin*




/* @data AjPUrlall ************************************************************
**
** Ajax URL all (stream) object.
**
** Inherits an AjPUrl but allows more URLs to be read from the
** same input by also inheriting the AjPUrlin input object.
**
** @alias AjSUrlall
** @alias AjOUrlall
**
** @attr Url [AjPUrl] Current URL
** @attr Urlin [AjPUrlin] URL input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: URL object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSUrlall
{
    AjPUrl Url;
    AjPUrlin Urlin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOUrlall;

#define AjPUrlall AjOUrlall*




/* @data AjPUrlAccess *********************************************************
**
** Ajax url access database reading object.
**
** Holds information needed to read an url entry from a database.
** Access methods are defined for each known database type.
**
** Url entries are read from the database using the defined
** database access function, which is usually a static function
** within ajurldb.c
**
** This should be a static data object but is needed for the definition
** of AjPUrlin.
**
** @alias AjSUrlAccess
** @alias AjOUrlAccess
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

typedef struct AjSUrlAccess
{
    const char *Name;
    AjBool (*Access)(AjPUrlin urlin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOUrlAccess;

#define AjPUrlAccess AjOUrlAccess*




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

#endif /* !AJURLDATA_H */

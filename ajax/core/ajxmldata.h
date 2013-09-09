/* @include ajxmldata ********************************************************
**
** __AJAX or NUCLEUS__  __LibraryArea__ functions
**
** __ShortDescription__
**
** @author Copyright (C) __Year__  __AuthorName__
** @version $Revision: 1.2 $
** @modified __EditDate__  __EditorName__  __Description of Edit__
** @modified $Date: 2012/09/03 18:18:24 $ by $Author: rice $
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

#ifndef AjPXMLDATA_H
#define AjPXMLDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"
#include "ajdom.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

#define NULLFPOS -1




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPXml **************************************************************
**
** Ajax xml object.
**
** Holds the xml itself, plus associated information.
**
** @alias AjSXml
** @alias AjOXml
**
** @attr Id        [AjPStr]  Id of term
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Doc       [AjPDomDocument]  Parsed document
** @attr TextPtr   [AjPStr] Full text
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSXml
{
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPDomDocument Doc;
    AjPStr  TextPtr;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjOXml;

#define AjPXml AjOXml*




/* @data AjPXmlin ************************************************************
**
** Ajax xml input object.
**
** Holds the input specification and information needed to read
** the xml and possible further entries
**
** @alias AjSXmlin
** @alias AjOXmlin
**
** @attr Input [AjPTextin] General text input object
** @attr XmlData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @@
******************************************************************************/

typedef struct AjSXmlin
{
    AjPTextin Input;
    void *XmlData;
} AjOXmlin;

#define AjPXmlin AjOXmlin*




/* @data AjPXmlall ***********************************************************
**
** Ajax xml all (stream) object.
**
** Inherits an AjPXml but allows more xmls to be read from the
** same input by also inheriting the AjPXmlin input object.
**
** @alias AjSXmlall
** @alias AjOXmlall
**
** @attr Xml [AjPXml] Current xml
** @attr Xmlin [AjPXmlin] Xml input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Xml object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSXmlall
{
    AjPXml Xml;
    AjPXmlin Xmlin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOXmlall;

#define AjPXmlall AjOXmlall*




/* @data AjPXmlAccess ********************************************************
**
** Ajax xml access database reading object.
**
** Holds information needed to read a xml entry from a database.
** Access methods are defined for each known database type.
**
** Xml entries are read from the database using the defined
** database access function, which is usually a static function
** within ajxmldb.c
**
** This should be a static data object but is needed for the definition
** of AjPXmlin.
**
** @alias AjSXmlAccess
** @alias AjOXmlAccess
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
** @@
******************************************************************************/

typedef struct AjSXmlAccess
{
    const char *Name;
    AjBool (*Access)(AjPXmlin xmlin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
} AjOXmlAccess;

#define AjPXmlAccess AjOXmlAccess*




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

#endif /* !AjPXMLDATA_H */

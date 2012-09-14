/* @include ajresourcedata ****************************************************
**
** AJAX data resource data structures
**
** These functions control all aspects of AJAX data resource
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version  $Revision: 1.15 $
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

#ifndef AJRESOURCEDATA_H
#define AJRESOURCEDATA_H

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




/* @data AjPResource **********************************************************
**
** Ajax resource object.
**
** Holds the resource itself, plus associated information.
**
** @alias AjSResource
** @alias AjOResource
**
** @attr Id        [AjPStr]  Standard identifier of rerousce
** @attr Idalt     [AjPList] Alternative identifiers for this resource
** @attr Acc       [AjPStr]  Accession number for this resource
** @attr Name      [AjPStr]  Name, usually the ID in full
** @attr Desc      [AjPStr]  Description
** @attr Url       [AjPStr]  Resource web server
** @attr Urllink   [AjPStr]  URL on linking instructions.
** @attr Urlrest   [AjPStr]  URL of documentation on REST interface
** @attr Urlsoap   [AjPStr]  URL of documentation on SOAP interface
** @attr Cat       [AjPList] Database category (from drcat.dat)
** @attr Taxon     [AjPList] NCBI Taxonomy
** @attr Edamdat   [AjPList] EDAM ontology references (retrievable data)
** @attr Edamfmt   [AjPList] EDAM ontology references (retrieved data format)
** @attr Edamid    [AjPList] EDAM ontology references (query fields)
** @attr Edamtpc   [AjPList] EDAM ontology references (data resource topic)
** @attr Xref      [AjPList] Database cross-reference link details
** @attr Query     [AjPList] Formatted query definitions
** @attr Example   [AjPList] Formatted query examples
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr TextPtr   [AjPStr]  Full text
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSResource
{
    AjPStr  Id;
    AjPList Idalt;
    AjPStr  Acc;
    AjPStr  Name;
    AjPStr  Desc;
    AjPStr  Url;
    AjPStr  Urllink;
    AjPStr  Urlrest;
    AjPStr  Urlsoap;
    AjPList Cat;
    AjPList Taxon;
    AjPList Edamdat;
    AjPList Edamfmt;
    AjPList Edamid;
    AjPList Edamtpc;
    AjPList Xref;
    AjPList Query;
    AjPList Example;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  TextPtr;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjOResource;

#define AjPResource AjOResource*




/* @data AjPResquery **********************************************************
**
** Ajax data resource query object
**
** Describes a standard public data resource query definition
**
** AjPResquery is implemented as a pointer to a C data structure.
**
** @alias AjSResquery
** @alias AjOResquery
**
** @attr Datatype [AjPStr] Resource query datatype
** @attr Format   [AjPStr] Resource query format
** @attr FormatTerm [AjPStr] Resource query format EDAM term
** @attr Term     [AjPStr] Resource query term
** @attr Url      [AjPStr] Resource query URL
** @attr Nterms   [ajuint] Number of terms
** @attr Padding  [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSResquery
{
    AjPStr Datatype;
    AjPStr Format;
    AjPStr FormatTerm;
    AjPStr Term;
    AjPStr Url;
    ajuint Nterms;
    ajuint Padding;
} AjOResquery;

#define AjPResquery AjOResquery*




/* @data AjPReslink ***********************************************************
**
** Ajax data resource link object
**
** Describes a standard public data resource link definition
**
** AjPReslink is implemented as a pointer to a C data structure.
**
** @alias AjSReslink
** @alias AjOReslink
**
** @attr Source   [AjPStr] Resource link source
** @attr Term     [AjPStr] Resource query terms
** @attr Nterms   [ajuint] Resource query number of terms
** @attr Padding  [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSReslink
{
    AjPStr Source;
    AjPStr Term;
    ajuint Nterms;
    ajuint Padding;
} AjOReslink;

#define AjPReslink AjOReslink*




/* @data AjPResterm ***********************************************************
**
** Ajax data resource term object
**
** Describes a standard public data resource term definition
**
** AjPResterm is implemented as a pointer to a C data structure.
**
** @alias AjSResterm
** @alias AjOResterm
**
** @attr Id       [AjPStr] Term ID in EDAM
** @attr Name     [AjPStr] Term name in EDAM
** @@
******************************************************************************/

typedef struct AjSResterm
{
    AjPStr Id;
    AjPStr Name;
} AjOResterm;

#define AjPResterm AjOResterm*




/* @data AjPResourcein ********************************************************
**
** Ajax resource input object.
**
** Holds the input specification and information needed to read
** the resource and possible further entries
**
** @alias AjSResourcein
** @alias AjOResourcein
**
** @attr Input [AjPTextin] General text input object
** @attr ResourceData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @@
******************************************************************************/

typedef struct AjSResourcein
{
    AjPTextin Input;
    void *ResourceData;
} AjOResourcein;

#define AjPResourcein AjOResourcein*




/* @data AjPResourceall *******************************************************
**
** Ajax data resource all (stream) object.
**
** Inherits an AjPResource but allows more data resources to be read from the
** same input by also inheriting the AjPResourcein input object.
**
** @alias AjSResourceall
** @alias AjOResourceall
**
** @attr Resource [AjPResource] Current term
** @attr Resourcein [AjPResourcein] Data resource input for reading next
** @attr Totresources [ajulong] Count of resources so far
** @attr Count [ajuint] Count of resources so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: data resource object has been returned
**                         to a new owner
**                         and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSResourceall
{
    AjPResource Resource;
    AjPResourcein Resourcein;
    ajulong Totresources;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOResourceall;

#define AjPResourceall AjOResourceall*




/* @data AjPResourceAccess ****************************************************
**
** Ajax data access database reading object.
**
** Holds information needed to read a resource entry from a database.
** Access methods are defined for each known database type.
**
** Resource entries are read from the database using the defined
** database access function, which is usually a static function
** within ajresourcedb.c
**
** This should be a static data object but is needed for the definition
** of AjPResourcein.
**
** @alias AjSResourceAccess
** @alias AjOResourceAccess
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

typedef struct AjSResourceAccess
{
    const char *Name;
    AjBool (*Access)(AjPResourcein resourcein);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOResourceAccess;

#define AjPResourceAccess AjOResourceAccess*




AJ_END_DECLS

#endif /* !AJRESOURCEDATA_H */

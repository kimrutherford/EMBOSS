/* @include ajquerydata *******************************************************
**
** AJAX Query data structures
**
** @author Copyright (C) 2011 Peter Rice
** @version $Revision: 1.22 $
** @modified Jul 15 pmr First version with code from all datatypes merged
** @modified $Date: 2012/09/03 14:26:04 $ by $Author: rice $
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

#ifndef AJQUERYDATA_H
#define AJQUERYDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajlist.h"
#include "ajtable.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define NULLFPOS -1




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @enum AjEQryType ***********************************************************
**
** Query type or scope (one, some or all entries)
**
** @value AJQUERY_UNKNOWN Unknown
** @value AJQUERY_ENTRY   Single entry
** @value AJQUERY_QUERY   Query for one or more entries
** @value AJQUERY_ALL     All entries
******************************************************************************/

typedef enum AjOQryType
{
    AJQUERY_UNKNOWN, AJQUERY_ENTRY,
    AJQUERY_QUERY, AJQUERY_ALL
} AjEQryType;




/* @enum AjEQryLink ***********************************************************
**
** Query link operators
**
** @value AJQLINK_INIT Initial query
** @value AJQLINK_OR   Current OR next
** @value AJQLINK_AND  Current AND next
** @value AJQLINK_EOR  Current EOR next
** @value AJQLINK_NOT  Current but NOT next
** @value AJQLINK_ELSE Current if it exists, else next
** @value AJQLINK_MAX  Above last defined value
******************************************************************************/

typedef enum AjOQryLink
{
    AJQLINK_INIT,
    AJQLINK_OR,
    AJQLINK_AND,
    AJQLINK_EOR,
    AJQLINK_NOT,
    AJQLINK_ELSE,
    AJQLINK_MAX
} AjEQryLink;




/* @enum AjEDataType **********************************************************
**
** AJAX data type
**
** @value AJDATATYPE_UNKNOWN   Undefined type
** @value AJDATATYPE_SEQUENCE  Sequence data
** @value AJDATATYPE_FEATURES  Feature data
** @value AJDATATYPE_ASSEMBLY  Assembly data
** @value AJDATATYPE_OBO       OBO ontology term data
** @value AJDATATYPE_REFSEQ    Reference sequence data
** @value AJDATATYPE_RESOURCE  Data resource data
** @value AJDATATYPE_TAXON     Taxonomy data
** @value AJDATATYPE_TEXT      Text data
** @value AJDATATYPE_URL       URL data
** @value AJDATATYPE_VARIATION Variation data
** @value AJDATATYPE_XML       XML data
** @value AJDATATYPE_MAX       Above last defined value
******************************************************************************/

typedef enum AjODataType
{
    AJDATATYPE_UNKNOWN,
    AJDATATYPE_SEQUENCE,
    AJDATATYPE_FEATURES,
    AJDATATYPE_ASSEMBLY,
    AJDATATYPE_OBO,
    AJDATATYPE_REFSEQ,
    AJDATATYPE_RESOURCE,
    AJDATATYPE_TAXON,
    AJDATATYPE_TEXT,
    AJDATATYPE_URL,
    AJDATATYPE_VARIATION,
    AJDATATYPE_XML,
    AJDATATYPE_MAX
} AjEDataType;




/* @data AjPQueryField ********************************************************
**
** Ajax Query Field object.
**
** Holds data needed for a specific field query
** This must refer to an field name (id, acc or any other queriable field)
** and the query string including wildcards specified by the user
**
** @alias AjSQueryField
** @alias AjOQueryField
**
** @attr Field     [AjPStr] Field name
** @attr Wildquery [AjPStr] Query Wildcard
** @attr Link [AjEQryLink] Link to previous queries
** @attr Padding [ajuint] Padding to alignment boundary
******************************************************************************/

typedef struct AjSQueryField
{
    AjPStr Field;
    AjPStr Wildquery;
    AjEQryLink Link;
    ajuint Padding;
} AjOQueryField;

#define AjPQueryField AjOQueryField*




/* @data AjPQuery *************************************************************
**
** Ajax Query object.
**
** Holds data needed to interpret an entry specification
** This can refer to an entry name (or "id"), and accession number or
** other queriable items.
**
** AjPQuery is created with the entry specification part of a USA
** (Uniform Sequence Address) or euivalent for other data types.
** The syntax is currently related to that
** used by SRS release 5.1.
**
** @alias AjSQuery
** @alias AjOQuery
**
** @attr SvrName [AjPStr] Server name used by EMBOSS
** @attr DbName [AjPStr] Database name used by EMBOSS
** @attr DbAlias [AjPStr] Database name used by access method
** @attr DbType [AjPStr] Database type
** @attr QueryFields [AjPList] Query field list
** @attr ResultsList [AjPList] Query results list
** @attr ResultsTable [AjPTable] Query results table
** @attr Method [AjPStr] Name of access method
** @attr Qlinks [AjPStr] Supported query link operators
** @attr Formatstr [AjPStr] Name of input format for parser
** @attr IndexDir [AjPStr] Index directory
** @attr Directory [AjPStr] Data directory
** @attr Filename [AjPStr] Individual filename
** @attr Exclude [AjPStr] File wildcards to exclude (spaced)
** @attr Namespace [AjPStr] Ontology namespace query to include
** @attr Organisms [AjPStr] Organism/taxonomy query to include
** @attr DbFields [AjPStr] Query fields (plus id and acc)
** @attr DbFilter [AjPStr] Database filter (query) fields
** @attr DbReturn [AjPStr] Database attribute (returned data) fields
** @attr DbIdentifier [AjPStr] Database primary identifier field
** @attr DbAccession [AjPStr] Database secondary identifier field
** @attr DbUrl [AjPStr] Query URL
** @attr DbProxy [AjPStr] Proxy host
** @attr DbHttpVer [AjPStr] HTTP version
** @attr ServerVer [AjPStr] Server version
** @attr SingleField [AjPStr] Query single field from end of query string
** @attr QryString [AjPStr] Query term
** @attr QryFields [AjPStr] Query fields or ID list
** @attr Application [AjPStr] External application command
** @attr Fpos [ajlong] File position from fseek
** @attr TextAccess [void*] Text access function : see ajtextdb.h
** @attr Access [void*] Datatype-specific access function : see ajseqdb.h
**                      ajobodb.h and others
** @attr QryData [void*] Private data for access function
** @attr QueryType [AjEQryType] Enumerated query type
** @attr DataType [AjEDataType] Enumerated datatype general scope
** @attr QryDone [AjBool] Has the query been done yet
** @attr SetServer [AjBool] True if server data has been set
** @attr SetDatabase [AjBool] True if database data has been set
** @attr SetQuery [AjBool] True if query data has been set
** @attr InDrcat [AjBool] True if database is defined in DRCAT
** @attr Wild [AjBool] True if query contains '*' or '?'
** @attr CaseId [AjBool] True if ID match is case-sensitive
** @attr HasAcc [AjBool] True if entries have acc field
** @attr CountEntries [ajuint] Number of entries processed
** @attr TotalEntries [ajuint] Number of entries found
** @@
******************************************************************************/

typedef struct AjSQuery
{
    AjPStr SvrName;
    AjPStr DbName;
    AjPStr DbAlias;
    AjPStr DbType;
    AjPList QueryFields;
    AjPList ResultsList;
    AjPTable ResultsTable;
    AjPStr Method;
    AjPStr Qlinks;
    AjPStr Formatstr;
    AjPStr IndexDir;
    AjPStr Directory;
    AjPStr Filename;
    AjPStr Exclude;
    AjPStr Namespace;
    AjPStr Organisms;
    AjPStr DbFields;
    AjPStr DbFilter;
    AjPStr DbReturn;
    AjPStr DbIdentifier;
    AjPStr DbAccession;
    AjPStr DbUrl;
    AjPStr DbProxy;
    AjPStr DbHttpVer;
    AjPStr ServerVer;
    AjPStr SingleField;
    AjPStr QryString;
    AjPStr QryFields;
    AjPStr Application;
    ajlong Fpos;
    void* TextAccess;
    void* Access;
    void* QryData;
    AjEQryType QueryType;
    AjEDataType DataType;
    AjBool QryDone;
    AjBool SetServer;
    AjBool SetDatabase;
    AjBool SetQuery;
    AjBool InDrcat;
    AjBool Wild;
    AjBool CaseId;
    AjBool HasAcc;
    ajuint CountEntries;
    ajuint TotalEntries;
} AjOQuery;

#define AjPQuery AjOQuery*




/* @data AjPQueryList *********************************************************
**
** Query processing list of queries from a list file.
**
** Includes data from the original query (@listfile)
**
** @alias AjSQueryList
** @alias AjOQueryList
**
** @attr Qry [AjPStr] Current query
** @attr Formatstr [AjPStr]  Format name from original query
** @attr QryFields [AjPStr]  Query fields or ID list
** @attr Fpos      [ajlong]  File position
** @attr Format    [ajuint]  Format enumeration
** @attr Padding   [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSQueryList
{
    AjPStr Qry;
    AjPStr Formatstr;
    AjPStr  QryFields;
    ajlong  Fpos;
    ajuint Format;
    char Padding[4];
} AjOQueryList;

#define AjPQueryList AjOQueryList*




/* @enum AjEQueryFmtcode ******************************************************
**
** Format code.
**
** @value FMT_OK OK
** @value FMT_NOMATCH No match
** @value FMT_BADTYPE Bad datatype
** @value FMT_FAIL Failed
** @value FMT_EOF End of file reached
** @value FMT_EMPTY Data object empty
******************************************************************************/

typedef enum AjOQueryFmtcode
{
    FMT_OK,
    FMT_NOMATCH,
    FMT_BADTYPE,
    FMT_FAIL,
    FMT_EOF,
    FMT_EMPTY
} AjEQueryFmtcode;




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

#endif  /* !AJQUERYDATA_H */

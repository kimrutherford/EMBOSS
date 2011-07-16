#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajquerydata_h
#define ajquerydata_h

#define NULLFPOS -1

typedef enum
{
    AJQUERY_UNKNOWN, AJQUERY_ENTRY,
    AJQUERY_QUERY, AJQUERY_ALL
} AjEQryType;

typedef enum
{
    AJQLINK_INIT, AJQLINK_OR,
    AJQLINK_AND, AJQLINK_EOR,
    AJQLINK_NOT, AJQLINK_ELSE, AJQLINK_MAX
} AjEQryLink;


typedef enum
{
    AJDATATYPE_UNKNOWN,
    AJDATATYPE_SEQUENCE, AJDATATYPE_FEATURES, AJDATATYPE_ASSEMBLY,
    AJDATATYPE_OBO, AJDATATYPE_RESOURCE, AJDATATYPE_TAXON,
    AJDATATYPE_TEXT, AJDATATYPE_URL, AJDATATYPE_VARIATION, AJDATATYPE_MAX
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
******************************************************************************/

typedef struct AjSQueryField 
{
    AjPStr Field;
    AjPStr Wildquery;
    AjEQryLink Link;
} AjOQueryField;
#define AjPQueryField AjOQueryField*




/* @data AjPQuery **************************************************************
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
** @attr DbFields [AjPStr] Query fields (plus id and acc)
** @attr DbFilter [AjPStr] Database filter (query) fields
** @attr DbReturn [AjPStr] Database attribute (returned data) fields
** @attr DbIdentifier [AjPStr] Database primary identifier field
** @attr DbAccession [AjPStr] Database secondary identifier field
** @attr DbUrl [AjPStr] Query URL
** @attr DbProxy [AjPStr] Proxy host
** @attr DbHttpVer [AjPStr] HTTP version
** @attr ServerVer [AjPStr] Server version
** @attr Field [AjPStr] Query field
** @attr QryString [AjPStr] Query term
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
** @attr Wild [AjBool] True if query contains '*' or '?'
** @attr CaseId [AjBool] True if ID match is case-sensitive
** @attr HasAcc [AjBool] True if entries have acc field
** @attr CountEntries [ajuint] Number of entries processed
** @attr TotalEntries [ajuint] Number of entries found
** @attr Padding [char[4]] Padding to alignment boundary
**
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
    AjPStr DbFields;
    AjPStr DbFilter;
    AjPStr DbReturn;
    AjPStr DbIdentifier;
    AjPStr DbAccession;
    AjPStr DbUrl;
    AjPStr DbProxy;
    AjPStr DbHttpVer;
    AjPStr ServerVer;
    AjPStr Field;
    AjPStr QryString;
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
    AjBool Wild;
    AjBool CaseId;
    AjBool HasAcc;
    ajuint CountEntries;
    ajuint TotalEntries;
    char Padding[4];
} AjOQuery;

#define AjPQuery AjOQuery*



/* @data AjPQueryList **********************************************************
**
** Query processing list of queries from a list file.
**
** Includes data from the original query (@listfile)
**
** @alias AjSQueryList
** @alias AjOQueryList
**
** @attr Qry [AjPStr] Current query
** @attr Formatstr [AjPStr] Format name from original query
** @attr Format [ajuint] Format enumeration
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSQueryList
{
    AjPStr Qry;
    AjPStr Formatstr;
    ajuint Format;
    char Padding[4];
} AjOQueryList;

#define AjPQueryList AjOQueryList*




enum fmtcode {FMT_OK, FMT_NOMATCH, FMT_BADTYPE, FMT_FAIL, FMT_EOF, FMT_EMPTY};



#endif

#ifdef __cplusplus
}
#endif

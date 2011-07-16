#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtaxdata_h
#define ajtaxdata_h


#define NULLFPOS -1

typedef struct AjSTaxAccess AjSTaxAccess;



/* @data AjPTax *************************************************************
**
** Ajax tax object.
**
** Holds the tax itself, plus associated information.
**
** @alias AjSTax
** @alias AjOTax
**
** @attr Id        [AjPStr]  Taxonomy id
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Rank      [AjPStr]  Rank in taxomony hierarchy
** @attr Emblcode  [AjPStr]  EMBL locus name prefix (not unique)
** @attr Comment   [AjPStr]  Comment
** @attr Name      [AjPStr]  Standard name
** @attr Namelist  [AjPList] Names list
** @attr Citations [AjPList] Citations list
** @attr Lines     [AjPStr*] Lines of input data
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @attr Taxid     [ajuint]  Taxon node id
** @attr Parent    [ajuint]  Parent taxid
** @attr Flags     [ajuint]  Flags:
**                               if node inherits division from parent
**                               if node inherits genetic code from parent
**                               if node inherits mito gencode from parent
**                               if name is suppressed in GB entry lineage
**                               if this subtree has no sequence data yet
** @attr Divid     [unsigned char]  Division id
** @attr Gencode   [unsigned char]  Genetic code id
** @attr Mitocode  [unsigned char]  Mitochondrial genetic code id
** @attr Padchar   [unsigned char]  Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTax {
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  Rank;
    AjPStr  Emblcode;
    AjPStr  Comment;
    AjPStr  Name;
    AjPList Namelist;
    AjPList Citations;
    AjPStr *Lines;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
    ajuint Taxid;
    ajuint Parent;
    ajuint Flags;
    unsigned char Divid;
    unsigned char Gencode;
    unsigned char Mitocode;
    unsigned char Padchar;
} AjOTax;

#define AjPTax AjOTax*




/* @data AjPTaxin ************************************************************
**
** Ajax tax input object.
**
** Holds the input specification and information needed to read
** the tax and possible further entries
**
** @alias AjSTaxin
** @alias AjOTaxin
**
** @attr Input [AjPTextin] General text input object
** @attr TaxData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @@
******************************************************************************/

typedef struct AjSTaxin {
    AjPTextin Input;
    void *TaxData;
} AjOTaxin;

#define AjPTaxin AjOTaxin*


/* @data AjPTaxall ************************************************************
**
** Ajax taxon all (stream) object.
**
** Inherits an AjPTax but allows more taxons to be read from the
** same input by also inheriting the AjPTaxin input object.
**
** @alias AjSTaxall
** @alias AjOTaxall
**
** @attr Tax [AjPTax] Current taxon
** @attr Taxin [AjPTaxin] Taxon input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Taxon object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTaxall
{
    AjPTax Tax;
    AjPTaxin Taxin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOTaxall;

#define AjPTaxall AjOTaxall*



/* @data AjPTaxname *********************************************************
**
** NCBI taxonomy node name, listed within AjPTax
**
** @alias AjSTaxname
** @alias AjOTaxname
**
** @attr Name          [AjPStr] Name of taxon node
** @attr UniqueName    [AjPStr] Unique name of taxon node
** @attr NameClass     [AjPStr] Synonym, common name, etc.
**
******************************************************************************/

typedef struct AjSTaxname
{
    AjPStr Name;
    AjPStr UniqueName;
    AjPStr NameClass;
} AjOTaxname;
#define AjPTaxname AjOTaxname*




/* @data AjPTaxdiv *********************************************************
**
** NCBI taxonomy division
**
** @alias AjSTaxdiv
** @alias AjOTaxdiv
**
** @attr GbCode       [AjPStr] GenBank division code (three characters)
** @attr GbName       [AjPStr] GenBank division name
** @attr Comments     [AjPStr] Comments
** @attr Divid        [ajuint] Taxonomy division id
** @attr Padding      [ajuint] Padding to alignment boundary
**
******************************************************************************/

typedef struct AjSTaxdiv
{
    AjPStr GbCode;
    AjPStr GbName;
    AjPStr Comments;
    ajuint Divid;
    ajuint Padding;
} AjOTaxdiv;
#define AjPTaxdiv AjOTaxdiv*




/* @data AjPTaxcode *********************************************************
**
** NCBI taxonomy genetic code
**
** @alias AjSTaxcode
** @alias AjOTaxcode
**
** @attr Abbrev     [AjPStr] Genetic code name abbreviation
** @attr Name       [AjPStr] Genetic code full name
** @attr Trans      [AjPStr] Amino acids for each codon
** @attr Starts     [AjPStr] Start codons
** @attr Gencode    [ajuint]  NCBI genetic code id
** @attr Padding    [ajuint]  Padding to alignment boundary
**
******************************************************************************/

typedef struct AjSTaxcode
{

    AjPStr Abbrev;
    AjPStr Name;
    AjPStr Trans;
    AjPStr Starts;
    ajuint Gencode;
    ajuint Padding;
} AjOTaxcode;
#define AjPTaxcode AjOTaxcode*




/* @data AjPTaxdel *********************************************************
**
** NCBI taxonomy deleted node
**
** @alias AjSTaxdel
** @alias AjOTaxdel
**
** @attr Taxid            [ajuint]  Taxon node id
**
******************************************************************************/

typedef struct AjSTaxdel
{
    ajuint Taxid;
} AjOTaxdel;
#define AjPTaxdel AjOTaxdel*




/* @data AjPTaxmerge *********************************************************
**
** NCBI taxonomy mergednode
**
** @alias AjSTaxmerge
** @alias AjOTaxmerge
**
** @attr Taxid            [ajuint]  Oldaxon node id
** @attr Mergeid          [ajuint]  Merged taxon node id
**
******************************************************************************/

typedef struct AjSTaxmerge
{
    ajuint Taxid;
    ajuint Mergeid;
} AjOTaxmerge;
#define AjPTaxmerge AjOTaxmerge*




/* @data AjPTaxcit *********************************************************
**
** NCBI taxonomy citation, listed within AjPTax
**
** @alias AjSTaxcit
** @alias AjOTaxcit
**
** @attr Citid      [ajuint] Taxonomy citation id
** @attr Pubmed     [ajuint] PubMed id (0 if not found)
** @attr Medline    [ajuint] Medline id (0 if not found)
** @attr Refcount   [ajuint] Reference count
** @attr Key      [AjPStr] Citation key
** @attr Url [AjPStr] Url associated with citation
** @attr Text [AjPStr] Text, usually article name and authors
**
******************************************************************************/

typedef struct AjSTaxcit
{
    ajuint Citid;
    ajuint Pubmed;
    ajuint Medline;
    ajuint Refcount;
    AjPStr Key;
    AjPStr Url;
    AjPStr Text;
} AjOTaxcit;
#define AjPTaxcit AjOTaxcit*




/* @data AjPTaxAccess ********************************************************
**
** Ajax taxonomy access database reading object.
**
** Holds information needed to read a tax entry from a database.
** Access methods are defined for each known database type.
**
** Tax entries are read from the database using the defined
** database access function, which is usually a static function
** within ajtaxdb.c
**
** This should be a static data object but is needed for the definition
** of AjPTaxin.
**
** @alias AjSTaxAccess
** @alias AjOTaxAccess
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

typedef struct AjSTaxAccess
{
    const char *Name;
    AjBool (*Access) (AjPTaxin taxin);
    AjBool (*AccessFree) (void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
} AjOTaxAccess;

#define AjPTaxAccess AjOTaxAccess*




#endif

#ifdef __cplusplus
}
#endif

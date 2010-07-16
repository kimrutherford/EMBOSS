#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtax_h
#define ajtax_h

#include "ajax.h"




/* @data AjPTax ***************************************************************
**
** NCBI taxonomy node
**
** @alias AjSTax
** @alias AjOTax
**
** @attr Taxid            [ajuint]  Taxon node id
** @attr Parent           [ajuint]  Parent taxid
** @attr Divid            [unsigned char]  Division id
** @attr Gencode          [unsigned char]  Genetic code id
** @attr Mitocode         [unsigned char]  Mitochondrial genetic code id
** @attr Padchar          [unsigned char]  Padding to alignment boundary
** @attr Flags            [ajuint]  Flags:
**                            if node inherits division from parent
**                            if node inherits genetic code from parent
**                            if node inherits mito gencode from parent
**                            if name is suppressed in GB entry lineage
**                            if this subtree has no sequence data yet
** @attr Rank             [AjPStr]  Rank in taxomony hierarchy
** @attr Emblcode         [AjPStr]  EMBL locus name prefix (not unique)
** @attr Comment          [AjPStr]  Comment
** @attr Namelist         [AjPList] Names list
** @attr Citations        [AjPList] Citation id list
**
******************************************************************************/

typedef struct AjSTax
{
    ajuint Taxid;
    ajuint Parent;
    unsigned char Divid;
    unsigned char Gencode;
    unsigned char Mitocode;
    unsigned char Padchar;
    ajuint Flags;
    AjPStr Rank;
    AjPStr Emblcode;
    AjPStr Comment;
    AjPList Namelist;
    AjPList Citations;
} AjOTax;
#define AjPTax AjOTax*




/* @data AjPTaxName *********************************************************
**
** NCBI taxonomy node name, listed within AjPTax
**
** @alias AjSTaxName
** @alias AjOTaxName
**
** @attr Name          [AjPStr] Name of taxon node
** @attr UniqueName    [AjPStr] Unique name of taxon node
** @attr NameClass     [AjPStr] Synonym, common name, etc.
**
******************************************************************************/

typedef struct AjSTaxName
{
    AjPStr Name;
    AjPStr UniqueName;
    AjPStr NameClass;
} AjOTaxName;
#define AjPTaxName AjOTaxName*




/* @data AjPTaxDiv *********************************************************
**
** NCBI taxonomy division
**
** @alias AjSTaxDiv
** @alias AjOTaxDiv
**
** @attr GbCode       [AjPStr] GenBank division code (three characters)
** @attr GbName       [AjPStr] GenBank division name
** @attr Comments     [AjPStr] Comments
** @attr Divid        [ajuint] Taxonomy division id
** @attr Padding      [ajuint] Padding to alignment boundary
**
******************************************************************************/

typedef struct AjSTaxDiv
{
    AjPStr GbCode;
    AjPStr GbName;
    AjPStr Comments;
    ajuint Divid;
    ajuint Padding;
} AjOTaxDiv;
#define AjPTaxDiv AjOTaxDiv*




/* @data AjPTaxCode *********************************************************
**
** NCBI taxonomy genetic code
**
** @alias AjSTaxCode
** @alias AjOTaxCode
**
** @attr Abbrev     [AjPStr] Genetic code name abbreviation
** @attr Name       [AjPStr] Genetic code full name
** @attr Trans      [AjPStr] Amino acids for each codon
** @attr Starts     [AjPStr] Start codons
** @attr Gencode    [ajuint]  NCBI genetic code id
** @attr Padding    [ajuint]  Padding to alignment boundary
**
******************************************************************************/

typedef struct AjSTaxCode
{

    AjPStr Abbrev;
    AjPStr Name;
    AjPStr Trans;
    AjPStr Starts;
    ajuint Gencode;
    ajuint Padding;
} AjOTaxCode;
#define AjPTaxCode AjOTaxCode*




/* @data AjPTaxDel *********************************************************
**
** NCBI taxonomy deleted node
**
** @alias AjSTaxDel
** @alias AjOTaxDel
**
** @attr Taxid            [ajuint]  Taxon node id
**
******************************************************************************/

typedef struct AjSTaxDel
{
    ajuint Taxid;
} AjOTaxDel;
#define AjPTaxDel AjOTaxDel*




/* @data AjPTaxMerge *********************************************************
**
** NCBI taxonomy mergednode
**
** @alias AjSTaxMerge
** @alias AjOTaxMerge
**
** @attr Taxid            [ajuint]  Oldaxon node id
** @attr Mergeid          [ajuint]  Merged taxon node id
**
******************************************************************************/

typedef struct AjSTaxMerge
{
    ajuint Taxid;
    ajuint Mergeid;
} AjOTaxMerge;
#define AjPTaxMerge AjOTaxMerge*




/* @data AjPTaxCit *********************************************************
**
** NCBI taxonomy citation, listed within AjPTax
**
** @alias AjSTaxCit
** @alias AjOTaxCit
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

typedef struct AjSTaxCit
{
    ajuint Citid;
    ajuint Pubmed;
    ajuint Medline;
    ajuint Refcount;
    AjPStr Key;
    AjPStr Url;
    AjPStr Text;
} AjOTaxCit;
#define AjPTaxCit AjOTaxCit*




/*
** Prototype definitions
*/

AjPTax       ajTaxNew(void);
AjPTaxCit    ajTaxCitNew(void);
AjPTaxCode   ajTaxCodeNew(void);
AjPTaxDel    ajTaxDelNew(void);
AjPTaxDiv    ajTaxDivNew(void);
AjPTaxMerge  ajTaxMergeNew(void);
AjPTaxName   ajTaxNameNew(void);
void         ajTaxDel(AjPTax *Ptax);
AjBool       ajTaxParse(AjPFile taxfile);
AjBool       ajTaxLoad(const AjPDir taxdir);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

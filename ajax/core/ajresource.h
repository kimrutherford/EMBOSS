#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajresource_h
#define ajresource_h




/* @data AjPResource **********************************************************
**
** Ajax data resource object
**
** Describes a standard public data resource
**
** AjPResource is implemented as a pointer to a C data structure.
**
** @alias AjSResource
** @alias AjOResource
**
** @attr Id [AjPStr] Standard identifier of resource
** @attr Otherids [AjPList] Alternative identifiers for this resource
** @attr Name [AjPStr] Name, usually the ID in full
** @attr Desc [AjPStr] Description
** @attr Server [AjPStr] Resource web server
** @attr Category [AjPStr] Database category
** @attr Edamrefs [AjPList] EDAM ontology references for the data resource
** @attr Links [AjPList] Database cross-reference link details
** @attr Linknote [AjPStr] Notes on the database cross-reference links
** @attr Queries [AjPList] Formatted query definitions
** @attr Examples [AjPList] Formatted query examples
** @attr Contact [AjPStr] Resource contact name
** @attr Email [AjPStr] Resource contact email address
** @attr Notes [AjPStr] Notes on resource entry
** @attr Status [AjPStr] Status of resource (controlled vocabulary)
** @@
******************************************************************************/

typedef struct AjSResource
{
    AjPStr Id;
    AjPList Otherids;
    AjPStr Name;
    AjPStr Desc;
    AjPStr Server;
    AjPStr Category;
    AjPList Edamrefs;
    AjPList Links;
    AjPStr Linknote;
    AjPList Queries;
    AjPList Examples;
    AjPStr Contact;
    AjPStr Email;
    AjPStr Notes;
    AjPStr Status;
} AjOResource;

#define AjPResource AjOResource*



/*
** Prototype definitions
*/

void         ajResourceParse(AjPFile dbfile, const char* validations);

/*
** End of prototype definitions
*/


#endif

#ifdef __cplusplus
}
#endif

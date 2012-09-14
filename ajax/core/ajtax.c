/* @source ajtax **************************************************************
**
** AJAX taxonomy functions
**
** These functions control all aspects of AJAX taxonomy
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.15 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/02 17:36:59 $ by $Author: rice $
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

#include "ajlib.h"

#include "ajtax.h"
#include "ajlist.h"
#include "ajtaxread.h"
#include "ajtaxwrite.h"

static AjPStr taxTempQry = NULL;

static void taxMakeQry(const AjPTax thys, AjPStr* qry);




/* @filesection ajtax *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPTax] Taxonomy data ***************************************
**
** Function is for manipulating taxonomy data objects
**
** @nam2rule Tax Taxonomy data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTax]
**
** @nam3rule New Constructor
** @nam4rule NewTax Copy constructor
**
** @argrule NewTax tax [const AjPTax] Source taxon object
**
** @valrule * [AjPTax] Taxonomy data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxNew *************************************************************
**
** Tax data constructor
**
** @return [AjPTax] New object
**
** @release 6.3.0
** @@
******************************************************************************/

AjPTax ajTaxNew(void)
{
    AjPTax ret;

    AJNEW0(ret);

    ret->Namelist = ajListNew();
    ret->Citations = ajListNew();

    return ret;
}




/* @func ajTaxNewTax **********************************************************
**
** Taxon copy constructor
**
** @param [r] tax [const AjPTax] Taxon
** @return [AjPTax] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTax ajTaxNewTax(const AjPTax tax)
{
    AjPTax ret;
    AjIList iter = NULL;
    AjPTaxname name = NULL;

    AJNEW0(ret);

    if(tax->Id)
        ret->Id = ajStrNewS(tax->Id);
    if(tax->Db)
        ret->Db = ajStrNewS(tax->Db);
    if(tax->Setdb)
        ret->Setdb = ajStrNewS(tax->Setdb);
    if(tax->Full)
        ret->Full = ajStrNewS(tax->Full);
    if(tax->Qry)
        ret->Qry = ajStrNewS(tax->Qry);
    if(tax->Formatstr)
        ret->Formatstr = ajStrNewS(tax->Formatstr);
    if(tax->Filename)
        ret->Filename = ajStrNewS(tax->Filename);
    if(tax->Rank)
        ret->Rank = ajStrNewS(tax->Rank);
    if(tax->Emblcode)
        ret->Emblcode = ajStrNewS(tax->Emblcode);
    if(tax->Comment)
        ret->Comment = ajStrNewS(tax->Comment);
    if(tax->Name)
        ret->Name = ajStrNewS(tax->Name);

    if(tax->Namelist)
    {
        ret->Namelist = ajListNew();
        iter = ajListIterNewread(tax->Namelist);
        while(!ajListIterDone(iter))
        {
            name = ajListIterGet(iter);
            ajListPushAppend(ret->Namelist, ajTaxnameNewName(name));
        }
        ajListIterDel(&iter);
    }

    ret->Fpos = tax->Fpos;
    ret->Format = tax->Format;
    ret->Count = tax->Count;
    ret->Taxid = tax->Taxid;
    ret->Parent = tax->Parent;
    ret->Flags = tax->Flags;
    ret->Divid = tax->Divid;
    ret->Gencode = tax->Gencode;
    ret->Mitocode = tax->Mitocode;

    return ret;
}




/* @section Taxonomy data destructors *****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the taxonomy data object.
**
** @fdata [AjPTax]
**
** @nam3rule Del Destructor
**
** @argrule Del Ptax [AjPTax*] Taxonomy data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTaxDel *************************************************************
**
** Taxonomy data destructor
**
** @param [d] Ptax       [AjPTax*] Taxonomy data object to delete
** @return [void] 
**
** @release 6.3.0
** @@
******************************************************************************/

void ajTaxDel(AjPTax *Ptax)
{
    AjPTax tax;
    AjPTaxname name = NULL;
    AjPTaxcit   cit = NULL;

    if(!Ptax) return;
    if(!(*Ptax)) return;

    tax = *Ptax;

    ajStrDel(&tax->Id);
    ajStrDel(&tax->Db);
    ajStrDel(&tax->Setdb);
    ajStrDel(&tax->Full);
    ajStrDel(&tax->Qry);
    ajStrDel(&tax->Formatstr);
    ajStrDel(&tax->Filename);
    ajStrDel(&tax->Rank);
    ajStrDel(&tax->Emblcode);
    ajStrDel(&tax->Comment);
    ajStrDel(&tax->Name);

    ajStrDelarray(&tax->Lines);

    if(tax->Namelist)
    {
        while(ajListPop(tax->Namelist, (void*) &name))
            ajTaxnameDel(&name);
        ajListFree(&tax->Namelist);
    }

    if(tax->Citations)
    {
        while(ajListPop(tax->Citations, (void*) &cit))
            ajTaxcitDel(&cit);
        ajListFree(&tax->Citations);
    }


    AJFREE(*Ptax);
    *Ptax = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a taxonomy data object
**
** @fdata [AjPTax]
**
** @nam3rule Get Return a value
** @nam3rule Is Test a condition
** @nam4rule Db Source database name
** @nam4rule Id Identifier string
** @nam4rule Name Name
** @nam4rule Parent Return the parent taxid
** @nam4rule Qry Return a query field
** @nam4rule Rank Taxonomic rank
** @nam4rule Tree List of taxons below in hierarchy
** @nam4rule Hidden Taxon has the GenBank hidden flag set
** @nam4rule Species Taxon is a species
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * tax [const AjPTax] Taxonomy data object.
** @argrule Tree taxlist [AjPList] Child taxons
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetId [const AjPStr] Identifier string
** @valrule *GetName [const AjPStr] Name
** @valrule *GetParent [ajuint] Parent taxid
** @valrule *GetRank [const AjPStr] Taxonomic rank
** @valrule *GetTree [ajuint] Number of new terms in list
** @valrule *IsHidden [AjBool] Hidden flag
** @valrule *IsSpecies [AjBool] Species taxon
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTaxGetDb ***********************************************************
**
** Return the database name
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [const AjPStr] Database name
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajTaxGetDb(const AjPTax tax)
{
    return tax->Db;
}




/* @func ajTaxGetId ***********************************************************
**
** Return the identifier
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [const AjPStr] Returned id
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajTaxGetId(const AjPTax tax)
{
    return tax->Id;
}




/* @func ajTaxGetName *********************************************************
**
** Return the name
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [const AjPStr] Returned name
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajTaxGetName(const AjPTax tax)
{
    return tax->Name;
}




/* @func ajTaxGetParent *******************************************************
**
** Return the parent id of a taxon
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [ajuint] Parent id
**
**
** @release 6.4.0
******************************************************************************/

ajuint ajTaxGetParent(const AjPTax tax)
{
    return tax->Parent;
}




/* @func ajTaxGetQryC *********************************************************
**
** Returns the query string of a taxonomy data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] tax [const AjPTax] Taxonomy data object.
** @return [const char*] Query as a character string.
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTaxGetQryC(const AjPTax tax)
{
    return MAJSTRGETPTR(ajTaxGetQryS(tax));
}




/* @func ajTaxGetQryS *********************************************************
**
** Returns the query string of a taxonomy data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] tax [const AjPTax] Taxonomy data object.
** @return [const AjPStr] Query as a string.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajTaxGetQryS(const AjPTax tax)
{
    ajDebug("ajTaxGetQryS '%S'\n", tax->Qry);

    if(ajStrGetLen(tax->Qry))
	return tax->Qry;

    taxMakeQry(tax, &taxTempQry);

    return taxTempQry;
}




/* @func ajTaxGetRank *********************************************************
**
** Return the taxonomic rank
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [const AjPStr] Returned rank
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajTaxGetRank(const AjPTax tax)
{
    return tax->Rank;
}




/* @func ajTaxGetTree *********************************************************
**
** Return a list with all this taxon's descendants appended
**
** @param [r] tax [const AjPTax] Taxon
** @param [u] taxlist [AjPList] List of taxons
**
** @return [ajuint] Number of taxons returned
**
**
** @release 6.4.0
******************************************************************************/

ajuint ajTaxGetTree(const AjPTax tax, AjPList taxlist)
{
    AjPTax taxnext = NULL;
    AjPTaxin taxin = NULL;
    AjPStr taxqry = NULL;
    static ajuint depth = 0;

    depth++;
    taxin = ajTaxinNew();
    ajFmtPrintS(&taxqry, "%S-up:%S", tax->Db, tax->Id);
    ajTaxinQryS(taxin, taxqry);

    ajDebug("ajTaxGetTree %u '%S'\n", depth, taxqry);
    taxnext = ajTaxNew();
    while(ajTaxinRead(taxin, taxnext))
    {
        ajDebug("ajTaxGetTree push '%S'\n", taxnext->Id);
        ajListPushAppend(taxlist, taxnext);
        ajTaxGetTree(taxnext, taxlist);
        taxnext = ajTaxNew();
    }

    ajTaxDel(&taxnext);
    ajTaxinDel(&taxin);
    ajStrDel(&taxqry);

    depth--;

    return (ajuint) ajListGetLength(taxlist);
}




/* @funcstatic taxMakeQry *****************************************************
**
** Sets the query for a taxonomy data object.
**
** @param [r] thys [const AjPTax] Taxonomy data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void taxMakeQry(const AjPTax thys, AjPStr* qry)
{
    ajDebug("taxMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajTaxTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(qry, "%S-id:%S", thys->Db, thys->Id);
    else
    {
	ajFmtPrintS(qry, "%S::%S:%S", thys->Formatstr,
                    thys->Filename,thys->Id);
    }

    ajDebug("      result: <%S>\n",
	    *qry);

    return;
}




/* @func ajTaxIsHidden ********************************************************
**
** Tests whether a taxon had tghe GenBank "hidden" flag set.
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [AjBool] True if node is hidden in GenBank taxonomy record
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTaxIsHidden(const AjPTax tax)
{
    if(tax->Flags & 8)
        return ajTrue;

    return ajFalse;
}




/* @func ajTaxIsSpecies *******************************************************
**
** Tests whether a taxon is at the rank of 'species'
**
** @param [r] tax [const AjPTax] Taxon
**
** @return [AjBool] True if taxon is a species
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTaxIsSpecies(const AjPTax tax)
{
    if(ajStrMatchC(tax->Rank, "species"))
        return ajTrue;

    return ajFalse;
}




/* @section taxonomy data modifiers *******************************************
**
** Taxonomy data modifiers
**
** @fdata [AjPTax]
**
** @nam3rule Clear clear internal values
**
** @argrule * tax [AjPTax] Taxonomy data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTaxClear ***********************************************************
**
** Resets all data for a taxonomy data object so that it can be reused.
**
** @param [u] tax [AjPTax] Taxonomy data object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxClear(AjPTax tax)
{
    AjPTaxname name = NULL;

    if(MAJSTRGETLEN(tax->Id))
       ajStrSetClear(&tax->Id);

    if(MAJSTRGETLEN(tax->Db))
       ajStrSetClear(&tax->Db);

    if(MAJSTRGETLEN(tax->Setdb))
       ajStrSetClear(&tax->Setdb);

    if(MAJSTRGETLEN(tax->Full))
       ajStrSetClear(&tax->Full);

    if(MAJSTRGETLEN(tax->Qry))
       ajStrSetClear(&tax->Qry);

    if(MAJSTRGETLEN(tax->Formatstr))
       ajStrSetClear(&tax->Formatstr);

    if(MAJSTRGETLEN(tax->Filename))
       ajStrSetClear(&tax->Filename);

    if(MAJSTRGETLEN(tax->Name))
       ajStrSetClear(&tax->Name);

    ajStrDelarray(&tax->Lines);

    if(tax->Namelist)
    {
        while(ajListPop(tax->Namelist, (void*) &name))
            ajTaxnameDel(&name);
    }

    tax->Count = 0;
    tax->Fpos = 0L;
    tax->Format = 0;
    tax->Flags = 0;
    tax->Taxid = 0;
    tax->Parent = 0;

    return;
}




/* @datasection [AjPTaxcit] Taxonomy citation data ****************************
**
** Function is for manipulating taxonomy citation data objects
**
** @nam2rule Taxcit Taxonomy data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTaxcit]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTaxcit] Taxonomy citation data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxcitNew **********************************************************
**
** Taxonomy citation constructor
**
** @return [AjPTaxcit] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxcit ajTaxcitNew(void)
{
    AjPTaxcit ret;

    AJNEW0(ret);

    return ret;
}




/* @section destructors ***********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the taxon citation object.
**
** @fdata [AjPTaxcit]
**
** @nam3rule Del destructor
**
** @argrule Del Pcit [AjPTaxcit*]Taxon citation
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTaxcitDel **********************************************************
**
** Taxonomy citation destructor
**
** @param [d] Pcit [AjPTaxcit*] Taxon citation object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxcitDel(AjPTaxcit *Pcit)
{
    AjPTaxcit cit;

    if(!Pcit)
        return;
    if(!*Pcit)
        return;

    cit = *Pcit;
    ajStrDel(&cit->Key);
    ajStrDel(&cit->Url);
    ajStrDel(&cit->Text);

    AJFREE(*Pcit);

    return;
}




/* @datasection [AjPTaxcode] Taxonomy genetic code data ***********************
**
** Function is for manipulating taxonomy genetic code data objects
**
** @nam2rule Taxcode Taxonomy genetic code data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTaxcode]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTaxcode] Taxonomy genetic code data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxcodeNew *********************************************************
**
** Taxonomy genetic code constructor
**
** @return [AjPTaxcode] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxcode ajTaxcodeNew(void)
{
    AjPTaxcode ret;

    AJNEW0(ret);

    return ret;
}




/* @datasection [AjPTaxdel] Taxonomy deleted id data **************************
**
** Function is for manipulating taxonomy deleted id data objects
**
** @nam2rule Taxdel Taxonomy deleted id data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTaxdel]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTaxdel] Taxonomy deleted id data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxdelNew **********************************************************
**
** Taxonomy deleted id constructor
**
** @return [AjPTaxdel] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxdel ajTaxdelNew(void)
{
    AjPTaxdel ret;

    AJNEW0(ret);

    return ret;
}




/* @datasection [AjPTaxdiv] Taxonomy division data ****************************
**
** Function is for manipulating taxonomy division data objects
**
** @nam2rule Taxdiv Taxonomy division data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTaxdiv]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTaxdiv] Taxonomy division data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxdivNew **********************************************************
**
** Taxonomy division constructor
**
** @return [AjPTaxdiv] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxdiv ajTaxdivNew(void)
{
    AjPTaxdiv ret;

    AJNEW0(ret);

    return ret;
}




/* @datasection [AjPTaxmerge] Taxonomy merged id data *************************
**
** Function is for manipulating taxonomy merged id data objects
**
** @nam2rule Taxmerge Taxonomy merged id data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTaxmerge]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTaxmerge] Taxonomy merged id data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxmergeNew ********************************************************
**
** Taxonomy merged id  constructor
**
** @return [AjPTaxmerge] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxmerge ajTaxmergeNew(void)
{
    AjPTaxmerge ret;

    AJNEW0(ret);

    return ret;
}




/* @datasection [AjPTaxname] Taxonomy name data *******************************
**
** Function is for manipulating taxonomy name data objects
**
** @nam2rule Taxname Taxonomy name data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTaxname]
**
** @nam3rule New Constructor
** @nam4rule NewName  Copy constructor
**
** @argrule *NewName name [const AjPTaxname] Taxonomy name data object
**
** @valrule * [AjPTaxname] Taxonomy name data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxnameNew *********************************************************
**
** Taxonomy names constructor
**
** @return [AjPTaxname] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxname ajTaxnameNew(void)
{
    AjPTaxname ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxnameNewName *****************************************************
**
** Taxonomy names copy constructor
**
** @param [r] name [const AjPTaxname] Name
** @return [AjPTaxname] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxname ajTaxnameNewName(const AjPTaxname name)
{
    AjPTaxname ret;

    AJNEW0(ret);

    if(name->Name)
        ret->Name = ajStrNewS(name->Name);
    if(name->UniqueName)
        ret->UniqueName = ajStrNewS(name->UniqueName);
    if(name->NameClass)
        ret->NameClass = ajStrNewS(name->NameClass);

    return ret;
}




/* @section destructors ***********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the taxon name object.
**
** @fdata [AjPTaxname]
**
** @nam3rule Del destructor
**
** @argrule Del Pname [AjPTaxname*]Taxon name
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTaxnameDel *********************************************************
**
** Taxonomy names destructor
**
** @param [d] Pname [AjPTaxname*] Taxon name object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxnameDel(AjPTaxname *Pname)
{
    AjPTaxname name;

    if(!Pname)
        return;
    if(!*Pname)
        return;

    name = *Pname;
    ajStrDel(&name->Name);
    ajStrDel(&name->UniqueName);
    ajStrDel(&name->NameClass);

    AJFREE(*Pname);

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Tax Taxonomy internals
**
******************************************************************************/




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajTaxExit ************************************************************
**
** Cleans up taxonomy processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxExit(void)
{
    ajTaxinExit();
    ajTaxoutExit();

    return;
}




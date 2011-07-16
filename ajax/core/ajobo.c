/******************************************************************************
** @source AJAX OBO handling functions
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added query and reading functions
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"


static AjPStr oboTempQry = NULL;


static void oboMakeQry(const AjPObo thys, AjPStr* qry);




/* @filesection ajobo ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPObo] Obo Term ********************************************
**
** Function is for manipulating obo term objects
**
** @nam2rule Obo Obo term objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPObo]
**
** @nam3rule New Constructor
** @nam4rule NewObo Copy constructor
** @nam3rule Parse Read from a file
** @nam4rule Oboterm Read term
**
** @argrule NewObo obo [const AjPObo] Source obo term object
** @argrule Parse obobuff [AjPFilebuff] Input buffered file
**
** @valrule * [AjPObo] Obo term object
**
** @fcategory new
**
******************************************************************************/




/* @func ajOboNew *************************************************************
**
** Term constructor
**
** @return [AjPObo] New object
** @@
******************************************************************************/

AjPObo ajOboNew(void)
{
    AjPObo ret;

    AJNEW0(ret);

    ret->Taglist = ajListNew();

    return ret;
}




/* @func ajOboNewObo **********************************************************
**
** Term copy constructor
**
** @param [r] obo [const AjPObo] Obo term
** @return [AjPObo] New object
** @@
******************************************************************************/

AjPObo ajOboNewObo(const AjPObo obo)
{
    AjPObo ret;
    AjIList iter;
    AjPObotag tag = NULL;
    AjPOboxref xref = NULL;

    AJNEW0(ret);

    ret->Obsolete = obo->Obsolete;
    ret->Builtin = obo->Builtin;
    ret->Fpos = obo->Fpos;
    ret->Format = obo->Format;
    
    if(obo->Id)
        ret->Id = ajStrNewS(obo->Id);
    if(obo->Trueid)
        ret->Trueid = ajStrNewS(obo->Trueid);
    if(obo->Fullid)
        ret->Fullid = ajStrNewS(obo->Fullid);
    if(obo->Name)
        ret->Name = ajStrNewS(obo->Name);
    if(obo->Namespace)
        ret->Namespace = ajStrNewS(obo->Namespace);
    if(obo->Def)
        ret->Def = ajStrNewS(obo->Def);
    if(obo->Comment)
        ret->Comment = ajStrNewS(obo->Comment);
    if(obo->Replaced)
        ret->Replaced = ajStrNewS(obo->Replaced);
    if(obo->Db)
        ret->Db = ajStrNewS(obo->Db);
    if(obo->Setdb)
        ret->Setdb = ajStrNewS(obo->Setdb);
    if(obo->Full)
        ret->Full = ajStrNewS(obo->Full);
    if(obo->Qry)
        ret->Qry = ajStrNewS(obo->Qry);
    if(obo->Formatstr)
        ret->Formatstr = ajStrNewS(obo->Formatstr);
    if(obo->Filename)
        ret->Filename = ajStrNewS(obo->Filename);
    if(obo->TextPtr)
        ret->TextPtr = ajStrNewS(obo->TextPtr);

    if(obo->Taglist)
    {
        ret->Taglist = ajListNew();
        iter = ajListIterNewread(obo->Taglist);
        while(!ajListIterDone(iter))
        {
            tag = ajListIterGet(iter);
            ajListPushAppend(ret->Taglist, ajObotagNewTag(tag));
        }
        ajListIterDel(&iter);
    }

    if(obo->Dbxrefs)
    {
        ret->Dbxrefs = ajListNew();
        iter = ajListIterNewread(obo->Dbxrefs);
        while(!ajListIterDone(iter))
        {
            xref = ajListIterGet(iter);
            ajListPushAppend(ret->Dbxrefs, ajOboxrefNewXref(xref));
        }
        ajListIterDel(&iter);
    }

    return ret;
}




/* @section OBO term destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the obo term object.
**
** @fdata [AjPObo]
**
** @nam3rule Del Destructor
**
** @argrule Del Pobo [AjPObo*] Obo term
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajOboDel *************************************************************
**
** Term destructor
**
** @param [d] Pobo       [AjPObo*]  Obo term object to delete
** @return [void] 
** @@
******************************************************************************/

void ajOboDel(AjPObo *Pobo)
{
    AjPObo     term = NULL;
    AjPObotag  tag  = NULL;
    AjPOboxref xref = NULL;

    if(!Pobo) return;
    if(!(*Pobo)) return;

    term = *Pobo;

    ajStrDel(&term->Id);
    ajStrDel(&term->Trueid);
    ajStrDel(&term->Fullid);
    ajStrDel(&term->Name);
    ajStrDel(&term->Namespace);
    ajStrDel(&term->Def);
    ajStrDel(&term->Comment);
    ajStrDel(&term->Replaced);

    while(ajListPop(term->Dbxrefs, (void**) &xref))
        ajOboxrefDel(&xref);

    while(ajListPop(term->Taglist, (void**) &tag))
        ajObotagDel(&tag);

    ajListFree(&term->Taglist);

    ajStrDel(&term->Db);
    ajStrDel(&term->Setdb);
    ajStrDel(&term->Full);
    ajStrDel(&term->Qry);
    ajStrDel(&term->Formatstr);
    ajStrDel(&term->Filename);
    ajStrDel(&term->TextPtr);

    AJFREE(*Pobo);
    *Pobo = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from an obo term object
**
** @fdata [AjPObo]
**
** @nam3rule Get Return a value
** @nam3rule Is Test a condition
** @nam4rule Replaced Replacement id for an obsolete term
** @nam4rule Db Source database name
** @nam4rule Def Definition string
** @nam4rule Entry Return entry text
** @nam4rule Id Identifier string
** @nam4rule Namespace Namespace
** @nam4rule Parents Return a list of parent ids
** @nam4rule Qry Return a query field
** @nam4rule Obsolete Term is flagged as obsolete
** @nam4rule Tree List of terms below in hierarchy
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * obo [const AjPObo] Obo term object.
** @argrule Def Pdefstr [AjPStr*] Returned definition string
** @argrule Def nrefs [ajuint*] Number of dbxrefs after the definition
** @argrule Parents uplist [AjPList] List of parent ids
** @argrule Tree obolist [AjPList] Child obo terms
**
** @valrule *GetParents [ajuint] Number of new ids appended to list
** @valrule *GetTree [ajuint] Number of new terms in list
** @valrule *GetDef     [AjBool] True on success
** @valrule *IsObsolete [AjBool] True if obsolete
** @valrule *GetReplaced [const AjPStr] Replacement identifier for an
**                                      obsolete term
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetEntry [const AjPStr] Entry full text
** @valrule *GetId [const AjPStr] Identifier string
** @valrule *GetNamespace [const AjPStr] Namespace name
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboGetDb ***********************************************************
**
** Return the database name
**
** @param [r] obo [const AjPObo] OBO term
**
** @return [const AjPStr] Database name
**
******************************************************************************/

const AjPStr ajOboGetDb(const AjPObo obo)
{
    return obo->Db;
}




/* @func ajOboGetDef **********************************************************
**
** Return the definition string and the number of dbxrefs
**
** @param [r] obo [const AjPObo] OBO term
** @param [w] Pdefstr [AjPStr*] Returned definition string
** @param [w] nrefs [ajuint*] Number of dbxrefs after the definition
**
** @return [AjBool] True on success.
**
******************************************************************************/

AjBool ajOboGetDef(const AjPObo obo, AjPStr *Pdefstr, ajuint *nrefs)
{
    const char* cp;
    ajuint i = 0;
    
    if(!obo) return ajFalse;

    cp = ajStrGetPtr(obo->Def);

    if(*cp == '"')
    {
        cp++;
        i = 1;
        while(*cp && *cp != '"')
        {
            i++;
            cp++;
        }
        cp++;
        ajStrAssignSubS(Pdefstr, obo->Def, 1, i);
    }
    else
    {
        i = 0;
        while(*cp && *cp != '[')
        {
            i++;
            cp++;
        }
        ajStrAssignSubS(Pdefstr, obo->Def, 0, i);
    }
    
    while(*cp && *cp != '[')
        cp++;

    *nrefs = 0;

    while (*cp) 
    {
        
    }
    

    return ajTrue;
    
}




/* @func ajOboGetEntry ********************************************************
**
** Return the full text
**
** @param [r] obo [const AjPObo] OBO term
**
** @return [const AjPStr] Returned full text
**
******************************************************************************/

const AjPStr ajOboGetEntry(const AjPObo obo)
{
    if(obo->TextPtr)
        return obo->TextPtr;

    return ajStrConstEmpty();
}




/* @func ajOboGetId ***********************************************************
**
** Return the identifier
**
** @param [r] obo [const AjPObo] OBO term
**
** @return [const AjPStr] Returned id
**
******************************************************************************/

const AjPStr ajOboGetId(const AjPObo obo)
{
    return obo->Id;
}




/* @func ajOboGetNamespace ****************************************************
**
** Return the namespace name
**
** @param [r] obo [const AjPObo] OBO term
**
** @return [const AjPStr] Returned namespace
**
******************************************************************************/

const AjPStr ajOboGetNamespace(const AjPObo obo)
{
    return obo->Namespace;
}




/* @func ajOboGetParents ******************************************************
**
** Return a list of all parent terms derived from is_a tags in the current
** obo term
**
** @param [r] obo [const AjPObo] OBO term
** @param [u] uplist [AjPList] List of parent terms
**
** @return [ajuint] Number of new terms added to list
**
******************************************************************************/

ajuint ajOboGetParents(const AjPObo obo, AjPList uplist)
{
    AjIList iter;
    AjPObotag tag = NULL;
    AjPStr up = NULL;
    ajlong colpos = 0;

    ajuint oldlen;

    oldlen = ajListGetLength(uplist);

    if(!obo)
        return 0;

    iter = ajListIterNewread(obo->Taglist);

    while(!ajListIterDone(iter))
    {
        tag = (AjPObotag) ajListIterGet(iter);

        if(!ajStrMatchC(tag->Name, "is_a"))
            continue;

        colpos = 1 + ajStrFindAnyK(tag->Value, ':');
        ajStrAssignSubS(&up, tag->Value, colpos, -1);

        ajListPushAppend(uplist, up);
        up = NULL;
    }

    if(obo->Obsolete && ajStrGetLen(obo->Replaced))
    {
        colpos = 1 + ajStrFindAnyK(obo->Replaced, ':');
        ajStrAssignSubS(&up, obo->Replaced, colpos, -1);
        ajDebug("ajOboGetParents obo id '%S' obsolete, consider '%S'\n",
                obo->Id, up);
        ajListPushAppend(uplist, up);
        up = NULL;
    }
        
    ajListIterDel(&iter);

    return ajListGetLength(uplist) - oldlen;
}




/* @func ajOboGetQryC *********************************************************
**
** Returns the query string of an obo term.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] obo [const AjPObo] Obo term object.
** @return [const char*] Query as a character string.
** @@
******************************************************************************/

const char* ajOboGetQryC(const AjPObo obo)
{
    return MAJSTRGETPTR(ajOboGetQryS(obo));
}




/* @func ajOboGetQryS *********************************************************
**
** Returns the query string of an obo term.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] obo [const AjPObo] Obo term object.
** @return [const AjPStr] Query as a string.
** @@
******************************************************************************/

const AjPStr ajOboGetQryS(const AjPObo obo)
{
    ajDebug("ajOboGetQryS '%S'\n", obo->Qry);

    if(ajStrGetLen(obo->Qry))
	return obo->Qry;

    oboMakeQry(obo, &oboTempQry);

    return oboTempQry;
}




/* @func ajOboGetReplaced *****************************************************
**
** Return the recommended replacement for an obsolete term
**
** @param [r] obo [const AjPObo] OBO term
**
** @return [const AjPStr] Returned id
**
******************************************************************************/

const AjPStr ajOboGetReplaced(const AjPObo obo)
{
    return obo->Replaced;
}




/* @func ajOboGetTree *********************************************************
**
** Return a list with all this term's descendants appended
**
** @param [r] obo [const AjPObo] OBO term
** @param [u] obolist [AjPList] List of obo terms
**
** @return [ajuint] Number of terms returned
**
******************************************************************************/

ajuint ajOboGetTree(const AjPObo obo, AjPList obolist)
{
    AjPObo obonext = NULL;
    AjPOboin oboin = NULL;
    AjPStr oboqry = NULL;
    static ajuint depth = 0;

    depth++;
    oboin = ajOboinNew();
    ajFmtPrintS(&oboqry, "%S-isa:%S", obo->Db, obo->Id);
    ajOboinQryS(oboin, oboqry);

    if(ajStrGetLen(obo->TextPtr))
        oboin->Input->Text = ajTrue;

    ajDebug("ajOboGetTree %u '%S'\n", depth, oboqry);
    obonext = ajOboNew();
    while(ajOboinRead(oboin, obonext))
    {
        ajDebug("ajOboGetTree push '%S'\n", obonext->Id);
        ajListPushAppend(obolist, obonext);
        ajOboGetTree(obonext, obolist);
        obonext = ajOboNew();
    }

    ajOboDel(&obonext);
    ajOboinDel(&oboin);
    ajStrDel(&oboqry);

    depth--;

    return ajListGetLength(obolist);
}





/* @func ajOboIsObsolete ******************************************************
**
** Test whether an obo term is obsolete
**
** @param [r] obo [const AjPObo] OBO term
**
** @return [AjBool] True if term is obsolete
**
******************************************************************************/

AjBool ajOboIsObsolete(const AjPObo obo)
{
    return obo->Obsolete;
}




/* @funcstatic oboMakeQry ****************************************************
**
** Sets the query for an obo term
**
** @param [r] thys [const AjPObo] Obo term object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
** @@
******************************************************************************/

static void oboMakeQry(const AjPObo thys, AjPStr* qry)
{
    ajDebug("oboMakeQry (Name <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Name, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajOboTrace(thys); */

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




/* @section obo term modifiers ************************************************
**
** Obo term modifiers
**
** @fdata [AjPObo]
**
** @nam3rule Clear clear internal values
**
** @argrule * obo [AjPObo] obo term object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajOboClear ***********************************************************
**
** Resets all data for an obo term object so that it can be reused.
**
** @param [u] obo [AjPObo] Obo term
** @return [void]
** @@
******************************************************************************/

void ajOboClear(AjPObo obo)
{
    AjPObotag  tag  = NULL;
    AjPOboxref xref = NULL;

    if(MAJSTRGETLEN(obo->Id))
       ajStrSetClear(&obo->Id);
    if(MAJSTRGETLEN(obo->Trueid))
       ajStrSetClear(&obo->Trueid);
    if(MAJSTRGETLEN(obo->Fullid))
        ajStrSetClear(&obo->Fullid);
    if(MAJSTRGETLEN(obo->Name))
       ajStrSetClear(&obo->Name);
    if(MAJSTRGETLEN(obo->Namespace))
       ajStrSetClear(&obo->Namespace);
    if(MAJSTRGETLEN(obo->Def))
       ajStrSetClear(&obo->Def);
    if(MAJSTRGETLEN(obo->Comment))
       ajStrSetClear(&obo->Comment);
    if(MAJSTRGETLEN(obo->Replaced))
       ajStrSetClear(&obo->Replaced);

    if(MAJSTRGETLEN(obo->Db))
       ajStrSetClear(&obo->Db);
    if(MAJSTRGETLEN(obo->Setdb))
       ajStrSetClear(&obo->Setdb);
    if(MAJSTRGETLEN(obo->Full))
       ajStrSetClear(&obo->Full);
    if(MAJSTRGETLEN(obo->Qry))
       ajStrSetClear(&obo->Qry);
    if(MAJSTRGETLEN(obo->Formatstr))
       ajStrSetClear(&obo->Formatstr);
    if(MAJSTRGETLEN(obo->Filename))
       ajStrSetClear(&obo->Filename);
    if(MAJSTRGETLEN(obo->TextPtr))
       ajStrSetClear(&obo->TextPtr);

    if(obo->Taglist)   
        while(ajListPop(obo->Taglist,(void**)&tag))
            ajObotagDel(&tag);

    if(obo->Dbxrefs)   
        while(ajListPop(obo->Dbxrefs,(void**)&xref))
            ajOboxrefDel(&xref);

    obo->Obsolete = ajFalse;
    obo->Builtin = ajFalse;
    obo->Fpos = 0L;
    obo->Format = 0;

    return;
}




/* @datasection [AjPObotag] Obo tags *****************************************
**
** Function is for manipulating obo tag objects
**
** @nam2rule Obotag Obo tags
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPObotag]
**
** @nam3rule New Constructor
** @nam4rule NewData Constructor using initial data values
** @nam4rule NewTag Copy constructor
**
** @argrule NewData name [const AjPStr] Name
** @argrule NewData value [const AjPStr] Value
** @argrule NewData  modifier [const AjPStr] Modifier
** @argrule NewData comment [const AjPStr] Comment
** @argrule NewData linenum [ajuint] OBO file line number
** @argrule NewTag tag [const AjPObotag] Name
**
** @valrule * [AjPObotag] Obo tag object
**
** @fcategory new
**
******************************************************************************/




/* @func ajObotagNewData ******************************************************
**
** Tag constructor
**
** @param [r] name [const AjPStr] Name
** @param [r] value [const AjPStr] Value
** @param [r] modifier [const AjPStr] Modifier
** @param [r] comment [const AjPStr] Comment
** @param [r] linenum [ajuint] OBO file line number
** @return [AjPObotag] New object
** @@
******************************************************************************/

AjPObotag ajObotagNewData(const AjPStr name, const AjPStr value,
                          const AjPStr modifier, const AjPStr comment,
                          ajuint linenum)
{
    AjPObotag ret;

    AJNEW0(ret);
    ret->Name  = ajStrNewS(name);
    ret->Value    = ajStrNewS(value);
    ret->Modifier  = ajStrNewS(modifier);
    ret->Comment  = ajStrNewS(comment);
    ret->Linenumber = linenum;

    return ret;
}





/* @func ajObotagNewTag *******************************************************
**
** Tag copy constructor
**
** @param [r] tag [const AjPObotag] Obo tag
** @return [AjPObotag] New object
** @@
******************************************************************************/

AjPObotag ajObotagNewTag(const AjPObotag tag)
{
    AjPObotag ret;

    AJNEW0(ret);

    if(tag->Name)
        ret->Name  = ajStrNewS(tag->Name);
    if(tag->Value)
        ret->Value    = ajStrNewS(tag->Value);
    if(tag->Modifier)
        ret->Modifier  = ajStrNewS(tag->Modifier);
    if(tag->Comment)
        ret->Comment  = ajStrNewS(tag->Comment);

    ret->Linenumber = tag->Linenumber;

    return ret;
}





/* @section obo tag destructors ***********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the obo tag object.
**
** @fdata [AjPObotag]
**
** @nam3rule Del destructor
**
** @argrule Del Ptag [AjPObotag*] Obo tag
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajObotagDel *********************************************************
**
** Tag destructor
**
** @param [d] Ptag       [AjPObotag*]  Tag object to delete
** @return [void] 
** @@
******************************************************************************/

void ajObotagDel(AjPObotag *Ptag)
{
    if(!Ptag) return;
    if(!(*Ptag)) return;

    ajStrDel(&(*Ptag)->Name);
    ajStrDel(&(*Ptag)->Value);
    ajStrDel(&(*Ptag)->Modifier);
    ajStrDel(&(*Ptag)->Comment);

    AJFREE(*Ptag);
    *Ptag = NULL;

    return;
}





/* @datasection [AjPOboxref] Obo dbxrefs ***************************************
**
** Function is for manipulating obo dbxref objects
**
** @nam2rule Oboxref Obo dbxrefs
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPOboxref]
**
** @nam3rule New Constructor
** @nam4rule NewData Constructor with initial values
** @nam4rule NewXref Copy constructor
**
** @argrule NewData name [const AjPStr] Name
** @argrule NewData desc [const AjPStr] Description
** @argrule NewXref xref [const AjPOboxref] Source dbxref object
**
** @valrule * [AjPOboxref] Obo dbxref object
**
** @fcategory new
**
******************************************************************************/




/* @func ajOboxrefNewData *****************************************************
**
** Dbxref constructor
**
** @param [r] name [const AjPStr] Name
** @param [r] desc [const AjPStr] Description
** @return [AjPOboxref] New object
** @@
******************************************************************************/

AjPOboxref ajOboxrefNewData(const AjPStr name, const AjPStr desc)
{
    AjPOboxref ret;

    AJNEW0(ret);
    ret->Name = ajStrNewS(name);
    ret->Desc = ajStrNewS(desc);

    return ret;
}





/* @func ajOboxrefNewXref *****************************************************
**
** Dbxref copy constructor
**
** @param [r] xref [const AjPOboxref] Dbxref
** @return [AjPOboxref] New object
** @@
******************************************************************************/

AjPOboxref ajOboxrefNewXref(const AjPOboxref xref)
{
    AjPOboxref ret;

    AJNEW0(ret);

    ret->Name = ajStrNewS(xref->Name);
    ret->Desc = ajStrNewS(xref->Desc);

    return ret;
}





/* @section obo dbxref destructors ********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the obo dbxref object.
**
** @fdata [AjPOboxref]
**
** @nam3rule Del destructor
**
** @argrule Del Pxref [AjPOboxref*] Obo dbxref
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajOboxrefDel ********************************************************
**
** Dbxref destructor
**
** @param [d] Pxref      [AjPOboxref*]  Dbxref object to delete
** @return [void] 
** @@
******************************************************************************/

void ajOboxrefDel(AjPOboxref *Pxref)
{
    if(!Pxref) return;
    if(!(*Pxref)) return;

    ajStrDel(&(*Pxref)->Name);
    ajStrDel(&(*Pxref)->Desc);

    AJFREE(*Pxref);
    *Pxref = NULL;

    return;
}





/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Obo Obo internals
**
******************************************************************************/




/* @section Miscellaneous *****************************************************
**
** Functions to initialise and clean up internals
**
** @fdata [none]
**
** @nam3rule Exit Clean up and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajOboExit ************************************************************
**
** Cleans up obo term internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajOboExit(void)
{
    /* Query processing regular expressions */

    ajOboinExit();
    ajObooutExit();

    return;
}





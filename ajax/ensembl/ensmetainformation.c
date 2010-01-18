/******************************************************************************
** @source Ensembl Meta-Information functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.3 $
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

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensmetainformation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool metaInformationadaptorFetchAllBySQL(
    EnsPMetainformationadaptor adaptor,
    const AjPStr statement,
    AjPList mis);

static AjBool metaInformationadaptorCacheInit(
    EnsPMetainformationadaptor adaptor);

static void metaInformationadaptorClearIdentifierCache(void **key,
                                                       void **value,
                                                       void *cl);

static void metaInformationadaptorClearKeyCache(void **key, void **value,
                                                void *cl);




/* @filesection ensmetainformation ********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMetainformation] Meta-Information ************************
**
** Functions for manipulating Ensembl Meta-Information objects
**
** @nam2rule Metainformation
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Meta-Information by pointer.
** It is the responsibility of the user to first destroy any previous
** Meta-Information. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMetainformation]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMetainformation] Ensembl Meta-Information
** @argrule Ref object [EnsPMetainformation] Ensembl Meta-Information
**
** @valrule * [EnsPMetainformation] Ensembl Meta-Information
**
** @fcategory new
******************************************************************************/




/* @func ensMetainformationNew ************************************************
**
** Default constructor for an Ensembl Meta-Information object.
**
** @cc Bio::EnsEMBL::Storable
** @param [r] adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                                 Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::MetaContainer
** @param [r] species [ajuint] Species identififer
** @param [u] key [AjPStr] Key
** @param [u] value [AjPStr] Value
**
** @return [EnsPMetainformation] Ensembl Meta-Information or NULL
** @@
******************************************************************************/

EnsPMetainformation ensMetainformationNew(EnsPMetainformationadaptor adaptor,
                                          ajuint identifier,
                                          ajuint species,
                                          AjPStr key,
                                          AjPStr value)
{
    EnsPMetainformation mi = NULL;
    
    AJNEW0(mi);
    
    mi->Use = 1;
    
    mi->Identifier = identifier;
    
    mi->Adaptor = adaptor;
    
    if(key)
	mi->Key = ajStrNewRef(key);
    
    if(value)
	mi->Value = ajStrNewRef(value);
    
    mi->Species = (species) ? species : 1;
    
    return mi;
}




/* @func ensMetainformationNewObj *********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [EnsPMetainformation] Ensembl Meta-Information
**
** @return [EnsPMetainformation] Ensembl Meta-Information or NULL
** @@
******************************************************************************/

EnsPMetainformation ensMetainformationNewObj(EnsPMetainformation object)
{
    EnsPMetainformation mi = NULL;
    
    if(!object)
	return NULL;
    
    AJNEW0(mi);
    
    mi->Use = 1;
    
    mi->Identifier = object->Identifier;
    
    mi->Adaptor = object->Adaptor;
    
    if(object->Key)
        mi->Key = ajStrNewRef(object->Key);
    
    if(object->Value)
        mi->Value = ajStrNewRef(object->Value);
    
    mi->Species = (object->Species) ? object->Species : 1;
    
    return mi;
}




/* @func ensMetainformationNewRef *********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
**
** @return [EnsPMetainformation] Ensembl Meta-Information or NULL
** @@
******************************************************************************/

EnsPMetainformation ensMetainformationNewRef(EnsPMetainformation mi)
{
    if(!mi)
	return NULL;
    
    mi->Use++;
    
    return mi;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Meta-Information.
**
** @fdata [EnsPMetainformation]
** @fnote None
**
** @nam3rule Del Destroy (free) a Meta-Information object
**
** @argrule * Pmi [EnsPMetainformation*] Meta-Information object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMetainformationDel ************************************************
**
** Default destructor for an Ensembl Meta-Information object.
**
** @param [d] Pmi [EnsPMetainformation*] Ensembl Meta-Information address
**
** @return [void]
** @@
******************************************************************************/

void ensMetainformationDel(EnsPMetainformation *Pmi)
{
    EnsPMetainformation pthis = NULL;
    
    if(!Pmi)
        return;
    
    if(!*Pmi)
        return;

    pthis = *Pmi;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pmi = NULL;
	
	return;
    }
    
    ajStrDel(&pthis->Key);
    
    ajStrDel(&pthis->Value);
    
    AJFREE(pthis);

    *Pmi = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Meta-Information object.
**
** @fdata [EnsPMetainformation]
** @fnote None
**
** @nam3rule Get Return Meta-Information attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Meta-Information Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetKey Return the key
** @nam4rule GetValue Return the value
** @nam4rule GetSpecies Return the species identifier
**
** @argrule * mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @valrule Adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                               Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Key [AjPStr] Key
** @valrule Value [AjPStr] Value
** @valrule Species [ajuint] Species identifier
**
** @fcategory use
******************************************************************************/




/* @func ensMetainformationGetAdaptor *****************************************
**
** Get the Ensembl Meta-Information Adaptor element of an
** Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPMetainformationadaptor ensMetainformationGetAdaptor(
    const EnsPMetainformation mi)
{
    if(!mi)
        return NULL;
    
    return mi->Adaptor;
}




/* @func ensMetainformationGetIdentifier **************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensMetainformationGetIdentifier(const EnsPMetainformation mi)
{
    if(!mi)
	return 0;
    
    return mi->Identifier;
}




/* @func ensMetainformationGetKey *********************************************
**
** Get the key element of an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [AjPStr] Key
** @@
******************************************************************************/

AjPStr ensMetainformationGetKey(const EnsPMetainformation mi)
{
    if(!mi)
        return NULL;
    
    return mi->Key;
}




/* @func ensMetainformationGetValue *******************************************
**
** Get the value element of an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [AjPStr] Value
** @@
******************************************************************************/

AjPStr ensMetainformationGetValue(const EnsPMetainformation mi)
{
    if(!mi)
	return NULL;
    
    return mi->Value;
}




/* @func ensMetainformationGetSpecies *****************************************
**
** Get the species identifier element of an Ensembl Meta-Information object.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [ajuint] Species identifier
** @@
******************************************************************************/

ajuint ensMetainformationGetSpecies(const EnsPMetainformation mi)
{
    if(!mi)
	return 0;
    
    return mi->Species;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Meta-Information object.
**
** @fdata [EnsPMetainformation]
** @fnote None
**
** @nam3rule Set Set one element of a Meta-Information
** @nam4rule SetAdaptor Set the Ensembl Meta-Information Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetKey Set the key
** @nam4rule SetValue Set the value
** @nam4rule SetSpecies Set the species identifier
**
** @argrule * mi [EnsPMetainformation] Ensembl Meta-Information object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMetainformationSetAdaptor *****************************************
**
** Set the Ensembl Meta-Information Adaptor element of an
** Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [r] adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                                 Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetAdaptor(EnsPMetainformation mi,
                                    EnsPMetainformationadaptor adaptor)
{
    if(!mi)
        return ajFalse;
    
    mi->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensMetainformationSetIdentifier **************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetIdentifier(EnsPMetainformation mi,
                                       ajuint identifier)
{
    if(!mi)
        return ajFalse;
    
    mi->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensMetainformationSetKey *********************************************
**
** Set the key element of an Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [u] key [AjPStr] Key
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetKey(EnsPMetainformation mi, AjPStr key)
{
    if(!mi)
        return ajFalse;
    
    ajStrDel(&mi->Key);
    
    if(key)
	mi->Key = ajStrNewRef(key);
    
    return ajTrue;
}




/* @func ensMetainformationSetValue *******************************************
**
** Set the value element of an Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [u] value [AjPStr] Value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetValue(EnsPMetainformation mi, AjPStr value)
{
    if(!mi)
        return ajFalse;
    
    ajStrDel(&mi->Value);
    
    if(value)
	mi->Value = ajStrNewRef(value);
    
    return ajTrue;
}




/* @func ensMetainformationSetSpecies *****************************************
**
** Set the species identifier element of an Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [r] species [ajuint] Species identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetSpecies(EnsPMetainformation mi, ajuint species)
{
    if(!mi)
        return ajFalse;
    
    mi->Species = species;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Meta-Information.
**
** @fdata [EnsPMetainformation]
** @nam3rule Trace Report Ensembl Meta-Information elements to debug file
**
** @argrule Trace mi [const EnsPMetainformation] Ensembl Meta-Information
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMetainformationTrace **********************************************
**
** Trace an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationTrace(const EnsPMetainformation mi, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!mi)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensMetainformationTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Key '%S'\n"
	    "%S  Value '%S'\n"
	    "%S  Species %u\n",
	    indent, mi,
	    indent, mi->Use,
	    indent, mi->Identifier,
	    indent, mi->Adaptor,
	    indent, mi->Key,
	    indent, mi->Value,
	    indent, mi->Species);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensMetainformationGetMemSize *****************************************
**
** Get the memory size in bytes of an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensMetainformationGetMemSize(const EnsPMetainformation mi)
{
    ajuint size = 0;
    
    if(!mi)
	return 0;
    
    size += (ajuint) sizeof (EnsOMetainformation);
    
    if(mi->Key)
    {
	size += sizeof (AjOStr);
	
	size += ajStrGetRes(mi->Key);
    }
    
    if(mi->Value)
    {
	size += sizeof (AjOStr);
	
	size += ajStrGetRes(mi->Value);
    }
    
    return size;
}




/* @datasection [EnsPMetainformationadaptor] Meta-Information Adaptor *********
**
** Functions for manipulating Ensembl Meta-Information Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseMetaContainer CVS Revision: 1.4
** @cc Bio::EnsEMBL::DBSQL::MetaContainer CVS Revision: 1.28
**
** @nam2rule Metainformationadaptor
**
******************************************************************************/




/* @funcstatic metaInformationadaptorFetchAllBySQL ****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Meta-Information objects.
**
** @param [r] adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                                 Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] mis [AjPList] AJAX List of Ensembl Meta-Informations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool metaInformationadaptorFetchAllBySQL(
    EnsPMetainformationadaptor adaptor,
    const AjPStr statement,
    AjPList mis)
{
    ajuint identifier = 0;
    ajuint species    = 0;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr key   = NULL;
    AjPStr value = NULL;
    
    EnsPMetainformation mi = NULL;
    
    /*
     ajDebug("metaInformationadaptorFetchAllByQuery\n"
	     "  adaptor %p\n"
	     "  statement %p\n"
	     "  mis %p\n",
	     adaptor,
	     statement,
	     mis);
     */
    
    if(!adaptor)
        return ajFalse;
    
    if(!statement)
        return ajFalse;
    
    if(!mis)
        return ajFalse;
    
    sqls = ensDatabaseadaptorSqlstatementNew(adaptor->Adaptor, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier = 0;
	species    = 0;
	
	key   = ajStrNew();
	value = ajStrNew();
	
        sqlr = ajSqlrowiterGet(sqli);
	
        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &species);
        ajSqlcolumnToStr(sqlr, &key);
        ajSqlcolumnToStr(sqlr, &value);
	
        mi = ensMetainformationNew(adaptor, identifier, species, key, value);
	
        ajListPushAppend(mis, (void *) mi);
	
	ajStrDel(&key);
	ajStrDel(&value);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    return ajTrue;
}




/* @funcstatic metaInformationadaptorCacheInit ********************************
**
** Initialise the internal Meta-Information cache of an
** Ensembl Meta-Information Adaptor.
**
** @param [u] adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                                 Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool metaInformationadaptorCacheInit(
    EnsPMetainformationadaptor adaptor)
{
    ajuint *Pidentifier = NULL;
    
    AjPList mis      = NULL;
    AjPList list     = NULL;
    AjPStr statement = NULL;
    
    EnsPMetainformation mi   = NULL;
    EnsPMetainformation temp = NULL;
    
    /*
     ajDebug("metaInformationadaptorCacheInit\n"
	     "  adaptor %p\n",
	     adaptor);
     */
    
    if(!adaptor)
	return ajFalse;
    
    statement = ajStrNewC("SELECT "
			  "meta.meta_id, "
			  "meta.species_id, "
			  "meta.meta_key, "
			  "meta.meta_value "
			  "FROM "
			  "meta ");
    
    mis = ajListNew();
    
    metaInformationadaptorFetchAllBySQL(adaptor, statement, mis);
    
    while(ajListPop(mis, (void **) &mi))
    {
	/* Insert into the identifier cache. */
	
	AJNEW0(Pidentifier);
	
	*Pidentifier = mi->Identifier;
	
	temp = (EnsPMetainformation) ajTablePut(adaptor->CacheByIdentifier,
						(void *) Pidentifier,
						(void *) mi);
	
	if(temp)
	{
	    ajWarn("metaInformationCacheInit got more than one "
		   "Ensembl Meta-Information with identifier %u.\n",
		   temp->Identifier);
	    
	    ensMetainformationDel(&temp);
	}
	
	/* Insert into the key cache. */
	
	list = (AjPList)
	    ajTableFetch(adaptor->CacheByKey, (const void *) mi->Key);
	
	if(!list)
	{
	    list = ajListNew();
	    
	    ajTablePut(adaptor->CacheByKey,
		       (void *) ajStrNewS(mi->Key),
		       (void *) list);
	}
	
	ajListPushAppend(list, (void *) ensMetainformationNewRef(mi));
    }
    
    ajListFree(&mis);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Meta-Information Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Meta-Information Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMetainformationadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                                  Adaptor
** @argrule Ref object [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                                  Adaptor
**
** @valrule * [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensMetainformationadaptorNew *****************************************
**
** Default constructor for an Ensembl Meta-Information Adaptor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPMetainformationadaptor ensMetainformationadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPMetainformationadaptor mia = NULL;
    
    /*
     ajDebug("ensMetainformationadaptorNew\n"
	     "  dba %p\n",
	     dba);
     */
    
    if(!dba)
	return NULL;
    
    AJNEW0(mia);
    
    mia->Adaptor = dba;
    
    mia->CacheByIdentifier = MENSTABLEUINTNEW(0);
    
    mia->CacheByKey = ajTablestrNewLen(0);
    
    metaInformationadaptorCacheInit(mia);
    
    return mia;
}




/* @funcstatic metaInformationadaptorClearIdentifierCache *********************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Meta-Information
** Adaptor-internal Ensembl Meta-Information cache. This function deletes the
** unsigned integer identifier key and the Ensembl Meta-Information value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Meta-Information value data address
** @param [r] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metaInformationadaptorClearIdentifierCache(void **key,
                                                       void **value,
                                                       void *cl)
{
    if(!key)
	return;
    
    if(!*key)
	return;
    
    if(!value)
	return;
    
    if(!*value)
	return;
    
    (void) cl;
    
    AJFREE(*key);
    
    ensMetainformationDel((EnsPMetainformation *) value);
    
    return;
}




/* @funcstatic metaInformationadaptorClearKeyCache ****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Meta-Information
** Adaptor-internal Ensembl Meta-Information cache. This function deletes the
** AJAX String Meta-Information key data and the AJAX List value data, as well
** as the Ensembl Meta-Information data from the AJAX List.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX List value data address
** @param [r] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metaInformationadaptorClearKeyCache(void **key, void **value,
                                                void *cl)
{
    EnsPMetainformation mi = NULL;
    
    if(!key)
	return;
    
    if(!*key)
	return;
    
    if(!value)
	return;
    
    if(!*value)
	return;
    
    (void) cl;
    
    ajStrDel((AjPStr *) key);
    
    while(ajListPop(*((AjPList *) value), (void **) &mi))
	ensMetainformationDel(&mi);
    
    ajListFree((AjPList *) value);
    
    return;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Meta-Information Adaptor.
**
** @fdata [EnsPMetainformationadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) a Meta-Information Adaptor object
**
** @argrule * Padaptor [EnsPMetainformationadaptor*] Meta-Information Adaptor
**                                                   object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMetainformationadaptorDel *****************************************
**
** Default destructor for an Ensembl Meta-Information Adaptor.
** This function also clears the internal caches.
**
** @param [d] Padaptor [EnsPMetainformationadaptor*] Ensembl Meta-Information
**                                                   Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensMetainformationadaptorDel(EnsPMetainformationadaptor *Padaptor)
{
    EnsPMetainformationadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    /* Clear and delete the identifier cache. */
    
    ajTableMapDel(pthis->CacheByIdentifier,
		  metaInformationadaptorClearIdentifierCache,
		  NULL);
    
    ajTableFree(&((*Padaptor)->CacheByIdentifier));
    
    /* Clear and delete the key cache. */
    
    ajTableMapDel(pthis->CacheByKey,
		  metaInformationadaptorClearKeyCache,
		  NULL);
    
    ajTableFree(&pthis->CacheByKey);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @func ensMetainformationadaptorFetchAllByKey *******************************
**
** Fetch Ensembl Meta-Information objects by a key.
**
** @param [r] adaptor [const EnsPMetainformationadaptor] Ensembl
**                                                       Meta-Information
**                                                       Adaptor
** @param [r] key [const AjPStr] Key
** @param [u] mis [AjPList] AJAX List of Ensembl Meta-Informations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorFetchAllByKey(
    const EnsPMetainformationadaptor adaptor,
    const AjPStr key,
    AjPList mis)
{
    AjIList iter = NULL;
    AjPList list = NULL;
    
    EnsPMetainformation mi = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!key)
        return ajFalse;
    
    if(!mis)
        return ajFalse;
    
    list = (AjPList) ajTableFetch(adaptor->CacheByKey, (const void *) key);
    
    iter = ajListIterNew(list);
    
    while(!ajListIterDone(iter))
    {
	mi = (EnsPMetainformation) ajListIterGet(iter);
	
	ajListPushAppend(mis, (void **) ensMetainformationNewRef(mi));
    }
    
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @func ensMetainformationadaptorGetValueByKey *******************************
**
** Get a single Meta-Information value by a key.
** The function warns in case there is more than one Meta-Information value to
** a particular key and will return the first value returned by the database
** engine.
**
** @param [r] adaptor [const EnsPMetainformationadaptor] Ensembl
**                                                       Meta-Information
**                                                       Adaptor
** @param [r] key [const AjPStr] Key
** @param [wP] Pvalue [AjPStr*] Value String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorGetValueByKey(
    const EnsPMetainformationadaptor adaptor,
    const AjPStr key,
    AjPStr* Pvalue)
{
    ajint errors = 0;
    ajint length = 0;
    
    AjPList list = NULL;
    
    EnsPMetainformation mi = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!key)
        return ajFalse;
    
    if(!Pvalue)
        return ajFalse;
    
    if(!*Pvalue)
        return ajFalse;
    
    list = (AjPList) ajTableFetch(adaptor->CacheByKey, (const void *) key);
    
    length = ajListGetLength(list);
    
    if(!length)
    {
        errors++;
	
        ajDebug("ensMetainformationadaptorGetValueByKey No 'meta.meta_value' "
	        "matched for 'meta.meta_key' '%S'.\n", key);
    }
    else
    {
	if(length > 1)
	{
	    errors++;
	    
	    ajDebug("ensMetainformationadaptorGetValueByKey matched %d "
		    "'meta.meta_value' by 'meta.meta_key' '%S', "
		    "but only the first value was returned.\n",
		    length, key);
	}
	
	ajListPeekFirst(list, (void **) &mi);
	
	ajStrAssignS(Pvalue, ensMetainformationGetValue(mi));
    }
    
    if(errors)
        return ajFalse;
    
    return ajTrue;
}




/* @func ensMetainformationadaptorGetGenebuildVersion *************************
**
** Get the Ensembl Genebuild Version.
**
** @param [r] adaptor [const EnsPMetainformationadaptor] Ensembl
**                                                       Meta-Information
*                                                        Adaptor
** @param [wP] Pvalue [AjPStr*] Value String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorGetGenebuildVersion(
    const EnsPMetainformationadaptor adaptor,
    AjPStr *Pvalue)
{
    ajint errors = 0;
    
    AjPStr key = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!Pvalue)
        return ajFalse;
    
    if(!*Pvalue)
        return ajFalse;
    
    key = ajStrNewC("genebuild.version");
    
    if(!ensMetainformationadaptorGetValueByKey(adaptor, key, Pvalue))
        errors++;
    
    ajStrDel(&key);
    
    if(errors)
        return ajFalse;
    
    return ajTrue;
}




/* @func ensMetainformationadaptorGetSchemaVersion ****************************
**
** Get the Ensembl Database Schema Version.
**
** @param [r] adaptor [const EnsPMetainformationadaptor] Ensembl
**                                                       Meta-Information
**                                                       Adaptor
** @param [wP] Pvalue [AjPStr*] Value String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorGetSchemaVersion(
    const EnsPMetainformationadaptor adaptor,
    AjPStr *Pvalue)
{
    ajint errors = 0;
    
    AjPStr key = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!Pvalue)
        return ajFalse;
    
    if(!*Pvalue)
        return ajFalse;
    
    key = ajStrNewC("schema_version");
    
    if(!ensMetainformationadaptorGetValueByKey(adaptor, key, Pvalue))
        errors++;
    
    ajStrDel(&key);
    
    if(errors)
        return ajFalse;
    
    return ajTrue;
}




/* @func ensMetainformationadaptorGetTaxonomyIdentifier ***********************
**
** Fetch the NCBI Taxonomy database identifier.
**
** @param [r] adaptor [const EnsPMetainformationadaptor] Ensembl
**                                                       Meta-Information
**                                                       Adaptor
** @param [wP] Pvalue [AjPStr*] Value String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorGetTaxonomyIdentifier(
    const EnsPMetainformationadaptor adaptor,
    AjPStr *Pvalue)
{
    ajint errors = 0;
    
    AjPStr key = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!Pvalue)
        return ajFalse;
    
    if(!*Pvalue)
	return ajFalse;
    
    key = ajStrNewC("species.taxonomy_id");
    
    if(!ensMetainformationadaptorGetValueByKey(adaptor, key, Pvalue))
        errors++;
    
    ajStrDel(&key);
    
    if(errors)
	return ajFalse;
    
    return ajTrue;
}




/* @func ensMetainformationadaptorKeyValueExists ******************************
**
** Test whether an Ensembl Meta-Information objects with a particular
** key and value exists.
**
** @param [r] adaptor [const EnsPMetainformationadaptor] Ensembl
**                                                       Meta-Information
**                                                       Adaptor
** @param [r] key [const AjPStr] Key
** @param [r] value [const AjPStr] Value
**
** @return [AjBool] ajTrue if the Meta Information exists, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorKeyValueExists(
    const EnsPMetainformationadaptor adaptor,
    const AjPStr key,
    const AjPStr value)
{
    AjBool bool = AJFALSE;
    
    AjIList iter = NULL;
    AjPList list = NULL;
    
    EnsPMetainformation mi = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!key)
        return ajFalse;
    
    if(!value)
        return ajFalse;
    
    list = (AjPList) ajTableFetch(adaptor->CacheByKey, (const void *) key);
    
    iter = ajListIterNew(list);
    
    while(!ajListIterDone(iter))
    {
	mi = (EnsPMetainformation) ajListIterGet(iter);
	
	if(ajStrMatchS(mi->Value, value))
	{
	    bool = ajTrue;
	    
	    break;
	}
    }
    
    ajListIterDel(&iter);
    
    return bool;
}

/******************************************************************************
** @source Ensembl Miscellaneous functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.6 $
** @@
**
** Bio::EnsEMBL::MiscSet CVS Revision: 1.5
** Bio::EnsEMBL::DBSQL::MiscSetAdaptor CVS Revision: 1.8
** Bio::EnsEMBL::MiscFeature CVS Revision: 1.12
** Bio::EnsEMBL::DBSQL::MiscFeatureadaptor CVS Revision: 1.18
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

#include "ensmiscellaneous.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPAssemblymapperadaptor
ensRegistryGetAssemblymapperadaptor(EnsPDatabaseadaptor dba);

extern EnsPCoordsystemadaptor
ensRegistryGetCoordsystemadaptor(EnsPDatabaseadaptor dba);

extern EnsPMiscellaneousfeatureadaptor
ensRegistryGetMiscellaneousfeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPMiscellaneoussetadaptor
ensRegistryGetMiscellaneoussetadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static AjBool miscellaneousSetAdaptorFetchAllBySQL(
    EnsPMiscellaneoussetadaptor msa,
    const AjPStr statement,
    AjPList mslist);

static AjBool miscellaneousSetAdaptorCacheInsert(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset *Pms);

#if AJFALSE
static AjBool miscellaneousSetAdaptorCacheRemove(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset ms);
#endif

static AjBool miscellaneousSetAdaptorCacheInit(EnsPMiscellaneoussetadaptor msa);

static void miscellaneousSetAdaptorCacheClearIdentifier(void **key,
                                                        void **value,
                                                        void *cl);

static void miscellaneousSetAdaptorCacheClearCode(void **key,
                                                  void **value,
                                                  void *cl);

static AjBool miscellaneousSetAdaptorCacheExit(
    EnsPMiscellaneoussetadaptor adaptor);

static void miscellaneousSetAdaptorFetchAll(const void *key, void **value,
                                            void *cl);

static AjBool miscellaneousFeatureadaptorHasAttribute(AjPTable attributes,
                                                      const AjPStr code,
                                                      const AjPStr value);

static AjBool miscellaneousFeatureadaptorClearAttributes(AjPTable attributes);

static AjBool miscellaneousFeatureadaptorHasMiscellaneousset(AjPTable sets,
                                                             ajuint msid);

static AjBool miscellaneousFeatureadaptorClearMiscellaneoussets(AjPTable sets);

static AjBool miscellaneousFeatureadaptorFetchAllBySQL(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mflist);

static void *miscellaneousFeatureadaptorCacheReference(void *value);

static void miscellaneousFeatureadaptorCacheDelete(void **value);

static ajuint miscellaneousFeatureadaptorCacheSize(const void *value);

static EnsPFeature miscellaneousFeatureadaptorGetFeature(const void *value);




/* @filesection ensmiscellaneous **********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/



/* @datasection [EnsPMiscellaneousset] Miscellaneous Set **********************
**
** Functions for manipulating Ensembl Miscellaneous Set objects
**
** @cc Bio::EnsEMBL::MiscSet CVS Revision: 1.6
**
** @nam2rule Miscellaneousset
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Set by pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Set. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneousset]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @argrule Ref object [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @valrule * [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneoussetNew ***********************************************
**
** Default constructor for an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                  Set Adaptor.
** @param [r] identifier [ajuint] SQL database-internal identifier.
** @cc Bio::EnsEMBL::MiscSet::new
** @param [u] code [AjPStr] Code.
** @param [u] name [AjPStr] Name.
** @param [u] description [AjPStr] Description.
** @param [r] maxlen [ajuint] Maximum length.
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL.
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNew(
    EnsPMiscellaneoussetadaptor adaptor,
    ajuint identifier,
    AjPStr code,
    AjPStr name,
    AjPStr description,
    ajuint maxlen)
{
    EnsPMiscellaneousset ms = NULL;
    
    AJNEW0(ms);
    
    ms->Use = 1;
    
    ms->Identifier = identifier;
    
    ms->Adaptor = adaptor;
    
    if(code)
        ms->Code = ajStrNewRef(code);
    
    if(name)
        ms->Name = ajStrNewRef(name);
    
    if(description)
        ms->Description = ajStrNewRef(description);
    
    ms->MaximumLength = maxlen;
    
    return ms;
}




/* @func ensMiscellaneoussetNewObj ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL.
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewObj(
    const EnsPMiscellaneousset object)
{
    EnsPMiscellaneousset ms = NULL;
    
    if(!object)
	return NULL;
    
    AJNEW0(ms);
    
    ms->Use = 1;
    
    ms->Identifier = object->Identifier;
    
    ms->Adaptor = object->Adaptor;
    
    if(object->Code)
        ms->Code = ajStrNewRef(object->Code);
    
    if(object->Name)
        ms->Name = ajStrNewRef(object->Name);
    
    if(object->Description)
        ms->Description = ajStrNewRef(object->Description);
    
    ms->MaximumLength = object->MaximumLength;
    
    return ms;
}




/* @func ensMiscellaneoussetNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewRef(EnsPMiscellaneousset ms)
{
    if(!ms)
	return NULL;
    
    ms->Use++;
    
    return ms;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Miscellaneous Set.
**
** @fdata [EnsPMiscellaneousset]
** @fnote None
**
** @nam3rule Del Destroy (free) an Miscellaneous Set object
**
** @argrule * Pms [EnsPMiscellaneousset*] Miscellaneous Set object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneoussetDel ***********************************************
**
** Default destructor for an Ensembl Miscellaneous Set.
**
** @param [d] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address.
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneoussetDel(EnsPMiscellaneousset *Pms)
{
    EnsPMiscellaneousset pthis = NULL;
    
    /*
     ajDebug("ensMiscellaneoussetDel\n"
	     "  *Pms %p\n",
	     *Pms);
     
     ensMiscellaneoussetTrace(*Pms, 1);
     */
    
    if(!Pms)
        return;
    
    if(!*Pms)
        return;
    
    (*Pms)->Use--;

    pthis = *Pms;
    
    if(pthis->Use)
    {
	*Pms = NULL;
	
	return;
    }
    
    ajStrDel(&pthis->Code);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);
    
    AJFREE(pthis);

    *Pms = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
** @fnote None
**
** @nam3rule Get Return Ensembl Miscellaneous Set attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Miscellaneous Set Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetCode Return the code
** @nam4rule GetName Return the name
** @nam4rule GetDescription Return the description
** @nam4rule GetMaximumLength Return the maximum length
**
** @argrule * ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @valrule Adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                Set Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Code [AjPStr] Code
** @valrule Name [AjPStr] Name
** @valrule Description [AjPStr] Description
** @valrule MaximumLength [ajuint] Maximum length
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneoussetGetAdaptor ****************************************
**
** Get the Ensembl Miscellaneous Set Adaptor element of an
** Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor.
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensMiscellaneoussetGetAdaptor(
    const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;
    
    return ms->Adaptor;
}




/* @func ensMiscellaneoussetGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [ajuint] Internal database identifier.
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetIdentifier(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return 0;
    
    return ms->Identifier;
}




/* @func ensMiscellaneoussetGetCode *******************************************
**
** Get the code element of an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [AjPStr] Code.
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetCode(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;
    
    return ms->Code;
}




/* @func ensMiscellaneoussetGetName *******************************************
**
** Get the name element of an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [AjPStr] Name.
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetName(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;
    
    return ms->Name;
}




/* @func ensMiscellaneoussetGetDescription ************************************
**
** Get the description element of an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [AjPStr] Description.
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetDescription(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;
    
    return ms->Description;
}




/* @func ensMiscellaneoussetGetMaximumLength **********************************
**
** Get the maximum length element of an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [ajuint] Maximum length.
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetMaximumLength(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return 0;
    
    return ms->MaximumLength;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Miscellaneous Set
** @nam4rule SetAdaptor Set the Ensembl Miscellaneous Set Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetCode Set the code
** @nam4rule SetName Set the name
** @nam4rule SetDescription Set the description
** @nam4rule SetMaximumLength Set the maximum length
**
** @argrule * ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMiscellaneoussetSetAdaptor ****************************************
**
** Set the Ensembl Miscellaneous Set Adaptor element of an
** Ensembl Miscellaneous Set.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [r] adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetAdaptor(EnsPMiscellaneousset ms,
                                     EnsPMiscellaneoussetadaptor adaptor)
{
    if(!ms)
        return ajFalse;
    
    ms->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensMiscellaneoussetSetIdentifier *************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Miscellaneous Set.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [r] identifier [ajuint] SQL database-internal identifier.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetIdentifier(EnsPMiscellaneousset ms,
                                        ajuint identifier)
{
    if(!ms)
        return ajFalse;
    
    ms->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensMiscellaneoussetSetCode *******************************************
**
** Set the code element of an Ensembl Miscellaneous Set.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [u] code [AjPStr] Code.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetCode(EnsPMiscellaneousset ms, AjPStr code)
{
    if(!ms)
        return ajFalse;
    
    ajStrDel(&ms->Code);
    
    ms->Code = ajStrNewRef(code);
    
    return ajTrue;
}




/* @func ensMiscellaneoussetSetName *******************************************
**
** Set the name element of an Ensembl Miscellaneous Set.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [u] name [AjPStr] Name.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetName(EnsPMiscellaneousset ms, AjPStr name)
{
    if(!ms)
        return ajFalse;
    
    ajStrDel(&ms->Name);
    
    ms->Name = ajStrNewRef(name);
    
    return ajTrue;
}




/* @func ensMiscellaneoussetSetDescription ************************************
**
** Set the description element of an Ensembl Miscellaneous Set.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [u] description [AjPStr] Description.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetDescription(EnsPMiscellaneousset ms,
                                         AjPStr description)
{
    if(!ms)
        return ajFalse;
    
    ajStrDel(&ms->Description);
    
    ms->Description = ajStrNewRef(description);
    
    return ajTrue;
}




/* @func ensMiscellaneoussetSetMaximumLength **********************************
**
** Set the maximum length element of an Ensembl Miscellaneous Set.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [r] maxlen [ajuint] Maximum length.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetMaximumLength(EnsPMiscellaneousset ms,
                                           ajuint maxlen)
{
    if(!ms)
        return ajFalse;
    
    ms->MaximumLength = maxlen;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
** @nam3rule Trace Report Ensembl Miscellaneous Set elements to debug file
**
** @argrule Trace ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneoussetTrace *********************************************
**
** Trace an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
** @param [r] level [ajuint] Indentation level.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetTrace(const EnsPMiscellaneousset ms, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!ms)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensMiscellaneoussetTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Code '%S'\n"
	    "%S  Name '%S'\n"
	    "%S  Description '%S'\n"
	    "%S  MaximumLength %u\n",
	    indent, ms,
	    indent, ms->Use,
	    indent, ms->Identifier,
	    indent, ms->Adaptor,
	    indent, ms->Code,
	    indent, ms->Name,
	    indent, ms->Description,
	    indent, ms->MaximumLength);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensMiscellaneoussetGetMemSize ****************************************
**
** Get the memory size in bytes of an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [ajuint] Memory size.
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetMemSize(const EnsPMiscellaneousset ms)
{
    ajuint size = 0;
    
    if(!ms)
	return 0;
    
    size += (ajuint) sizeof (EnsOMiscellaneousset);
    
    if(ms->Code)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(ms->Code);
    }
    
    if(ms->Name)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(ms->Name);
    }
    
    if(ms->Description)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(ms->Description);
    }
    
    return size;
}




/* @datasection [EnsPMiscellaneoussetadaptor] Miscellaneous Set Adaptor *******
**
** Functions for manipulating Ensembl Miscellaneous Set Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor CVS Revision: 1.12
**
** @nam2rule Miscellaneoussetadaptor
**
******************************************************************************/




/* @funcstatic miscellaneousSetAdaptorFetchAllBySQL ***************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Miscellaneous Set objects.
**
** @param [r] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor.
** @param [r] statement [const AjPStr] SQL statement.
** @param [u] mslist [AjPList] AJAX List of Ensembl Miscellaneous Sets.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousSetAdaptorFetchAllBySQL(
    EnsPMiscellaneoussetadaptor msa,
    const AjPStr statement,
    AjPList mslist)
{
    ajuint identifier = 0;
    ajuint maxlen     = 0;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr code        = NULL;
    AjPStr name        = NULL;
    AjPStr description = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    if(!msa)
        return ajFalse;
    
    if(!statement)
        return ajFalse;
    
    if(!mslist)
        return ajFalse;
    
    sqls = ensDatabaseadaptorSqlstatementNew(msa->Adaptor, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier = 0;
	code = ajStrNew();
	name = ajStrNew();
	description = ajStrNew();
	maxlen = 0;
	
        sqlr = ajSqlrowiterGet(sqli);
	
        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &code);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToUint(sqlr, &maxlen);
	
        ms = ensMiscellaneoussetNew(msa,
				    identifier,
				    code,
				    name,
				    description,
				    maxlen);
	
        ajListPushAppend(mslist, (void *) ms);
	
	ajStrDel(&code);
	ajStrDel(&name);
	ajStrDel(&description);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    return ajTrue;
}




/* @funcstatic miscellaneousSetAdaptorCacheInsert *****************************
**
** Insert an Ensembl Miscellaneous Set into the
** Miscellaneous Set Adaptor-internal cache.
** If a Miscellaneous Set with the same code element is already present in the
** adaptor cache, the Miscellaneous Set is deleted and a pointer to the cached
** Miscellaneous Set is returned.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor.
** @param [u] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousSetAdaptorCacheInsert(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset *Pms)
{
    ajuint *Pidentifier = NULL;
    
    EnsPMiscellaneousset ms1 = NULL;
    EnsPMiscellaneousset ms2 = NULL;
    
    if(!msa)
        return ajFalse;
    
    if(!msa->CacheByIdentifier)
        return ajFalse;
    
    if(!msa->CacheByCode)
        return ajFalse;
    
    if(!Pms)
        return ajFalse;
    
    if(!*Pms)
        return ajFalse;
    
    /* Search the identifer cache. */
    
    ms1 = (EnsPMiscellaneousset)
	ajTableFetch(msa->CacheByIdentifier,
		     (const void *) &((*Pms)->Identifier));
    
    /* Search the code cache. */
    
    ms2 = (EnsPMiscellaneousset)
	ajTableFetch(msa->CacheByCode,
		     (const void *) (*Pms)->Code);
    
    if((!ms1) && (! ms2))
    {
	/* Insert into the identifier cache. */
	
	AJNEW0(Pidentifier);
	
	*Pidentifier = (*Pms)->Identifier;
	
	ajTablePut(msa->CacheByIdentifier,
		   (void *) Pidentifier,
		   (void *) ensMiscellaneoussetNewRef(*Pms));
	
	/* Insert into the code cache. */
	
        ajTablePut(msa->CacheByCode,
		   (void *) ajStrNewS((*Pms)->Code),
		   (void *) ensMiscellaneoussetNewRef(*Pms));
    }
    
    if(ms1 && ms2 && (ms1 == ms2))
    {
        ajDebug("miscellaneousSetCacheInsert replaced Miscellaneous Set %p "
		"with one already cached %p.\n", *Pms, ms1);
	
	ensMiscellaneoussetDel(Pms);
	
	Pms = &ms1;
    }
    
    if(ms1 && ms2 && (ms1 != ms2))
	ajDebug("miscellaneousSetCacheInsert detected Miscellaneous Sets in "
		"the identifier and code cache with identical codes "
		"('%S' and '%S') but different addresses (%p and %p).\n",
		ms1->Code, ms2->Code, ms1, ms2);
    
    if(ms1 && (!ms2))
        ajDebug("miscellaneousSetCacheInsert detected an "
		"Ensembl Miscellaneous Set "
		"in the identifier, but not in the code cache.\n");
    
    if((!ms1) && ms2)
        ajDebug("miscellaneousSetCacheInsert detected an "
		"Ensembl Miscellaneous Set "
		"in the code, but not in the identifier cache.\n");
    
    return ajTrue;
}




/* @funcstatic miscellaneousSetAdaptorCacheRemove *****************************
**
** Remove an Ensembl Miscellaneous Set from the
** Miscellaneous Set Adaptor-internal cache.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor.
** @param [r] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

#if AJFALSE

static AjBool miscellaneousSetAdaptorCacheRemove(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset ms)
{
    ajuint *Pidentifier = NULL;
    
    AjPStr key = NULL;
    
    EnsPMiscellaneousset ms1 = NULL;
    EnsPMiscellaneousset ms2 = NULL;
    
    if(!msa)
	return ajFalse;
    
    if(!ms)
	return ajFalse;
    
    /* Remove the table nodes. */
    
    ms1 = (EnsPMiscellaneousset)
	ajTableRemoveKey(msa->CacheByIdentifier,
			 (const void *) &(ms->Identifier),
			 (void **) &Pidentifier);
    
    ms2 = (EnsPMiscellaneousset)
	ajTableRemoveKey(msa->CacheByCode,
			 (const void *) ms->Code,
			 (void **) &key);
    
    if(ms1 && (!ms2))
	ajWarn("miscellaneousSetAdaptorCacheRemove could remove "
	       "Miscellaneous Set '%S' (%u) "
	       "only from the identifier cache.\n",
	       ms->Code, ms->Identifier);
    
    if((!ms1) && ms2)
	ajWarn("miscellaneousSetAdaptorCacheRemove could remove "
	       "Miscellaneous Set '%S' (%u) "
	       "only from the code cache.\n",
	       ms->Code, ms->Identifier);
    
    /* Delete the keys. */
    
    AJFREE(Pidentifier);
    
    ajStrDel(&key);
    
    /* Delete (or at least de-reference) the Ensembl Analyses. */
    
    ensMiscellaneoussetDel(&ms1);
    ensMiscellaneoussetDel(&ms2);
    
    return ajTrue;
}

#endif




/* @funcstatic miscellaneousSetAdaptorCacheInit *******************************
**
** Initialise the internal Miscellaneous Set cache of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousSetAdaptorCacheInit(EnsPMiscellaneoussetadaptor msa)
{
    AjPList mslist   = NULL;
    AjPStr statement = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    if(!msa)
        return ajFalse;
    
    if(msa->CacheByIdentifier)
        return ajFalse;
    else
        msa->CacheByIdentifier =
	    ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);
    
    if(msa->CacheByCode)
        return ajFalse;
    else
        msa->CacheByCode = ajTablestrNewLen(0);
    
    statement = ajStrNewC("SELECT "
			  "misc_set.misc_set_id, "
			  "misc_set.code, "
			  "misc_set.name, "
			  "misc_set.description, "
			  "misc_set.max_length "
			  "FROM "
			  "misc_set");
    
    mslist = ajListNew();
    
    miscellaneousSetAdaptorFetchAllBySQL(msa, statement, mslist);
    
    while(ajListPop(mslist, (void **) &ms))
    {
        miscellaneousSetAdaptorCacheInsert(msa, &ms);
	
	/*
	 ** Both caches hold internal references to the
	 ** Miscellaneous Set objects.
	 */
	
	ensMiscellaneoussetDel(&ms);
    }
    
    ajListFree(&mslist);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Set Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Set Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneoussetadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                   Set Adaptor
** @argrule Ref object [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                   Set Adaptor
**
** @valrule * [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneoussetadaptorNew ****************************************
**
** Default constructor for an Ensembl Miscellaneous Set Adaptor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
**
** @return [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneousset Adaptor
**                                       or NULL.
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensMiscellaneoussetadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMiscellaneoussetadaptor msa = NULL;
    
    if(!dba)
        return NULL;
    
    AJNEW0(msa);
    
    msa->Adaptor = dba;
    
    miscellaneousSetAdaptorCacheInit(msa);
    
    return msa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Miscellaneous Set Adaptor.
**
** @fdata [EnsPMiscellaneoussetadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Miscellaneous Set Adaptor object
**
** @argrule * Padaptor [EnsPMiscellaneoussetadaptor*] Ensembl Miscellaneous Set
**                                                    Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic miscellaneousSetAdaptorCacheClearIdentifier ********************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Miscellaneous Set
** Adaptor-internal Miscellaneous Set cache. This function deletes the
** unsigned integer identifier key and the Ensembl Miscellaneous Set
** value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Miscellaneous Set value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousSetAdaptorCacheClearIdentifier(void **key,
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
    
    ensMiscellaneoussetDel((EnsPMiscellaneousset *) value);
    
    return;
}




/* @funcstatic miscellaneousSetAdaptorCacheClearCode **************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Miscellaneous Set
** Adaptor-internal Miscellaneous Set cache. This function deletes the code
** AJAX String key data and the Ensembl Miscellaneous Set value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl Miscellaneous Set value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousSetAdaptorCacheClearCode(void **key,
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
    
    ajStrDel((AjPStr *) key);
    
    ensMiscellaneoussetDel((EnsPMiscellaneousset *) value);
    
    return;
}




/* @funcstatic miscellaneousSetAdaptorCacheExit *******************************
**
** Clears the internal Miscellaneous Set cache of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [u] adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                  Set Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousSetAdaptorCacheExit(
    EnsPMiscellaneoussetadaptor adaptor)
{
    if(!adaptor)
        return ajFalse;
    
    /* Clear and delete the identifier cache. */
    
    ajTableMapDel(adaptor->CacheByIdentifier,
		  miscellaneousSetAdaptorCacheClearIdentifier,
		  NULL);
    
    ajTableFree(&(adaptor->CacheByIdentifier));
    
    /* Clear and delete the code cache. */
    
    ajTableMapDel(adaptor->CacheByCode,
		  miscellaneousSetAdaptorCacheClearCode,
		  NULL);
    
    ajTableFree(&(adaptor->CacheByCode));
    
    return ajTrue;
}




/* @func ensMiscellaneoussetadaptorDel ****************************************
**
** Default destructor for an Ensembl Miscellaneous Set Adaptor.
** This function also clears the internal caches.
**
** @param [d] Padaptor [EnsPMiscellaneoussetadaptor*] Ensembl Miscellaneous
**                                                    Set Adaptor address.
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneoussetadaptorDel(EnsPMiscellaneoussetadaptor *Padaptor)
{
    EnsPMiscellaneoussetadaptor pthis = NULL;
    
    if(!Padaptor)
        return;
    
    if(!*Padaptor)
        return;

    pthis = *Padaptor;
    
    miscellaneousSetAdaptorCacheExit(pthis);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneoussetadaptor]
** @fnote None
**
** @nam3rule Get Return Ensembl Miscellaneous Set Adaptor attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Database Adaptor
**
** @argrule * adaptor [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                        Set Adaptor
**
** @valrule Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneoussetadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [r] adaptor [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                        Set Adaptor.
**
** @return [const EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL.
** @@
******************************************************************************/

const EnsPDatabaseadaptor ensMiscellaneoussetadaptorGetDatabaseadaptor(
    const EnsPMiscellaneoussetadaptor adaptor)
{
    if(!adaptor)
        return NULL;
    
    return adaptor->Adaptor;
}




/* @section object fetching ***************************************************
**
** Functions for retrieving Ensembl Miscellaneous Set objects from an
** Ensembl Core database.
**
** @fdata [EnsPMiscellaneoussetadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Miscellaneous Set object(s)
** @nam4rule FetchAll Retrieve all Ensembl Miscellaneous Set objects.
** @nam5rule FetchAllBy Retrieve all Ensembl Miscellaneous Set objects
**                      matching a criterion.
** @nam4rule FetchBy Retrieve one Ensembl Miscellaneous Set object
**                   matching a criterion.
**
** @argrule * adaptor [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                        Set Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Miscellaneous Set objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise.
**
** @fcategory use
******************************************************************************/




/* @funcstatic miscellaneousSetAdaptorFetchAll ********************************
**
** An ajTableMap 'apply' function to return all Miscellaneous Set objects
** from the Ensembl Miscellaneous Set Adaptor-internal cache.
**
** @param [u] key [const void *] Pointer to unsigned integer identifier
**                               key data.
** @param [u] value [void**] Pointer to Ensembl Miscellaneous Set value data.
** @param [u] cl [void*] AJAX List of Ensembl Miscellaneous Set objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousSetAdaptorFetchAll(const void *key, void **value,
                                            void *cl)
{
    EnsPMiscellaneousset *Pms = NULL;
    
    if(!key)
	return;
    
    if(!value)
	return;
    
    if(!*value)
	return;
    
    if(!cl)
	return;
    
    Pms = (EnsPMiscellaneousset *) value;
    
    ajListPushAppend((AjPList) cl, (void *) ensMiscellaneoussetNewRef(*Pms));
    
    return;
}




/* @func ensMiscellaneoussetadaptorFetchAll ***********************************
**
** Fetch all Ensembl Miscellaneous Sets.
**
** @param [r] adaptor [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                        Set Adaptor.
** @param [u] mslist [AjPList] AJAX List of Ensembl Miscellaneous Sets.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchAll(
    const EnsPMiscellaneoussetadaptor adaptor,
    AjPList mslist)
{
    if(!adaptor)
        return ajFalse;
    
    if(!mslist)
        return ajFalse;
    
    ajTableMap(adaptor->CacheByIdentifier,
	       miscellaneousSetAdaptorFetchAll,
	       (void *) mslist);
    
    return ajTrue;
}




/* @func ensMiscellaneoussetadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl Miscellaneous Set by its SQL database-internal identifier.
**
** @param [r] adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                  Set Adaptor.
** @param [r] identifier [ajuint] SQL database-internal identifier.
** @param [wP] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchByIdentifier(
    EnsPMiscellaneoussetadaptor adaptor,
    ajuint identifier,
    EnsPMiscellaneousset *Pms)
{
    AjPList mslist = NULL;
    
    AjPStr statement = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if(!identifier)
        return ajFalse;
    
    if(!Pms)
	return ajFalse;
    
    /*
    ** Initally, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */
    
    *Pms = (EnsPMiscellaneousset)
	ajTableFetch(adaptor->CacheByIdentifier, (const void *) &identifier);
    
    if(*Pms)
    {
	ensMiscellaneoussetNewRef(*Pms);
	
	return ajTrue;
    }
    
    /* For a cache miss query the database. */
    
    statement = ajFmtStr("SELECT "
			 "misc_set.misc_set_id, "
			 "misc_set.code, "
			 "misc_set.name, "
			 "misc_set.description, "
			 "misc_set.max_length "
			 "FROM "
			 "misc_set"
			 "WHERE "
			 "misc_set.misc_set_id = %u",
			 identifier);
    
    mslist = ajListNew();
    
    miscellaneousSetAdaptorFetchAllBySQL(adaptor, statement, mslist);
    
    if(ajListGetLength(mslist) > 1)
	ajWarn("ensMiscellaneoussetadaptorFetchByIdentifier got more "
	       "than one Ensembl Miscellaneous Set for (PRIMARY KEY) "
	       "identifier %u.\n", identifier);
    
    ajListPop(mslist, (void **) Pms);
    
    miscellaneousSetAdaptorCacheInsert(adaptor, Pms);
    
    while(ajListPop(mslist, (void **) &ms))
    {
	miscellaneousSetAdaptorCacheInsert(adaptor, &ms);
	
	ensMiscellaneoussetDel(&ms);
    }
    
    ajListFree(&mslist);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @func ensMiscellaneoussetadaptorFetchByCode ********************************
**
** Fetch an Ensembl Miscellaneous Set by its code.
**
** @param [r] adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                  Set Adaptor.
** @param [r] code [const AjPStr] Ensembl Miscellaneous Set code.
** @param [wP] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchByCode(
    EnsPMiscellaneoussetadaptor adaptor,
    const AjPStr code,
    EnsPMiscellaneousset *Pms)
{
    char *txtcode = NULL;
    
    AjPList mslist = NULL;
    
    AjPStr statement = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    if(!adaptor)
        return ajFalse;
    
    if((!code) && (! ajStrGetLen(code)))
        return ajFalse;
    
    if(!Pms)
	return ajFalse;
    
    /*
    ** Initally, search the code cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */
    
    *Pms = (EnsPMiscellaneousset)
	ajTableFetch(adaptor->CacheByCode, (const void *) code);
    
    if(*Pms)
    {
	ensMiscellaneoussetNewRef(*Pms);
	
	return ajTrue;
    }
    
    /* In case of a cache miss, query the database. */
    
    ensDatabaseadaptorEscapeC(adaptor->Adaptor, &txtcode, code);
    
    statement = ajFmtStr("SELECT "
			 "misc_set.misc_set_id, "
			 "misc_set.code, "
			 "misc_set.name, "
			 "misc_set.description, "
			 "misc_set.max_length "
			 "FROM "
			 "misc_set"
			 "WHERE "
			 "misc_set.code = '%s'",
			 txtcode);
    
    ajCharDel(&txtcode);
    
    mslist = ajListNew();
    
    miscellaneousSetAdaptorFetchAllBySQL(adaptor, statement, mslist);
    
    if(ajListGetLength(mslist) > 1)
	ajWarn("ensMiscellaneoussetadaptorFetchByCode got more than one "
	       "Ensembl Miscellaneous Sets for (UNIQUE) code '%S'.\n",
	       code);
    
    ajListPop(mslist, (void **) Pms);
    
    miscellaneousSetAdaptorCacheInsert(adaptor, Pms);
    
    while(ajListPop(mslist, (void **) &ms))
    {
	miscellaneousSetAdaptorCacheInsert(adaptor, &ms);
	
	ensMiscellaneoussetDel(&ms);
    }
    
    ajListFree(&mslist);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @datasection [EnsPMiscellaneousfeature] Miscellaneous Feature **************
**
** Functions for manipulating Ensembl Miscellaneous Feature objects
**
** @cc Bio::EnsEMBL::MiscFeature CVS Revision: 1.13
**
** @nam2rule Miscellaneousfeature
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Feature. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneousfeature]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @argrule Ref object [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
**
** @valrule * [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneousfeatureNew *******************************************
**
** Default constructor for an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                      Feature Adaptor.
** @param [r] identifier [ajuint] SQL database-internal identifier.
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature.
** @cc Bio::EnsEMBL::MiscFeature::new
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL.
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNew(
    EnsPMiscellaneousfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature)
{
    EnsPMiscellaneousfeature mf = NULL;
    
    if(!feature)
	return NULL;
    
    AJNEW0(mf);
    
    mf->Use = 1;
    
    mf->Identifier = identifier;
    
    mf->Adaptor = adaptor;
    
    mf->Feature = ensFeatureNewRef(feature);
    
    mf->Attributes = ajListNew();
    
    mf->Miscellaneoussets = ajListNew();
    
    return mf;
}




/* @func ensMiscellaneousfeatureNewObj ****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                    Feature.
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL.
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewObj(
    const EnsPMiscellaneousfeature object)
{
    AjIList iter = NULL;
    
    EnsPAttribute attribute = NULL;
    
    EnsPMiscellaneousfeature mf = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    AJNEW0(mf);
    
    mf->Use = 1;
    
    mf->Identifier = object->Identifier;
    
    mf->Adaptor = object->Adaptor;
    
    mf->Feature = ensFeatureNewRef(object->Feature);
    
    /* Copy the AJAX List of Ensembl Attributes. */
    
    mf->Attributes = ajListNew();
    
    iter = ajListIterNew(object->Attributes);
    
    while(!ajListIterDone(iter))
    {
	attribute = (EnsPAttribute) ajListIterGet(iter);
	
	ajListPushAppend(mf->Attributes,
			 (void *) ensAttributeNewRef(attribute));
    }
    
    ajListIterDel(&iter);
    
    /* Copy the AJAX List of Ensembl Miscellaneous Sets. */
    
    mf->Miscellaneoussets = ajListNew();
    
    iter = ajListIterNew(object->Miscellaneoussets);
    
    while(!ajListIterDone(iter))
    {
	ms = (EnsPMiscellaneousset) ajListIterGet(iter);
	
	ajListPushAppend(mf->Miscellaneoussets,
			 (void *) ensMiscellaneoussetNewRef(ms));
    }
    
    ajListIterDel(&iter);
    
    return mf;
}




/* @func ensMiscellaneousfeatureNewRef ****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL.
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewRef(
    EnsPMiscellaneousfeature mf)
{
    if(!mf)
	return NULL;
    
    mf->Use++;
    
    return mf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Miscellaneous Features.
**
** @fdata [EnsPMiscellaneousfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Miscellaneous Feature object
**
** @argrule * Pmf [EnsPMiscellaneousfeature*] Miscellaneous Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneousfeatureDel *******************************************
**
** Default destructor for an Ensembl Miscellaneous Feature.
**
** @param [d] Pmf [EnsPMiscellaneousfeature*] Ensembl Miscellaneous
**                                            Feature address.
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneousfeatureDel(EnsPMiscellaneousfeature *Pmf)
{
    EnsPAttribute attribute = NULL;
    
    EnsPMiscellaneousset ms = NULL;

    EnsPMiscellaneousfeature pthis = NULL;
    
    
    if(!Pmf)
        return;
    
    if(!*Pmf)
        return;
    
    (*Pmf)->Use--;

    pthis = *Pmf;
    
    if(pthis->Use)
    {
	*Pmf = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    /* Clear and free the AJAX List of Ensembl Attributes. */
    
    while(ajListPop(pthis->Attributes, (void **) &attribute))
	ensAttributeDel(&attribute);
    
    ajListFree(&pthis->Attributes);
    
    /* Clear and free the AJAX List of Ensembl Miscellaneous Sets. */
    
    while(ajListPop(pthis->Miscellaneoussets, (void **) &ms))
	ensMiscellaneoussetDel(&ms);
    
    ajListFree(&pthis->Miscellaneoussets);
    
    AJFREE(pthis);

    *Pmf = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
** @fnote None
**
** @nam3rule Get Return Miscellaneous Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Miscellaneous Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetAttributes Return all Ensembl Attributes
** @nam4rule GetMiscellaneoussets Return all Ensembl Miscellaneous Sets
**
** @argrule * mf [const EnsPMiscellaneousfeature] Miscellaneous Feature
**
** @valrule Adaptor [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                    Feature Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule Attributes [const AjPList] AJAX List of Ensembl Attributes
** @valrule Miscellaneoussets [const AjPList] AJAX List of Ensembl
**                                            Miscellaneous Sets
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureGetAdaptor ************************************
**
** Get the Ensembl Miscellaneous Feature Adaptor element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
**
** @return [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                           Feature Adaptor.
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureGetAdaptor(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;
    
    return mf->Adaptor;
}




/* @func ensMiscellaneousfeatureGetIdentifier *********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
**
** @return [ajuint] Internal database identifier.
** @@
******************************************************************************/

ajuint ensMiscellaneousfeatureGetIdentifier(const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return 0;
    
    return mf->Identifier;
}




/* @func ensMiscellaneousfeatureGetFeature ************************************
**
** Get the Ensembl Feature element of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
**
** @return [EnsPFeature] Ensembl Feature.
** @@
******************************************************************************/

EnsPFeature ensMiscellaneousfeatureGetFeature(const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;
    
    return mf->Feature;
}




/* @func ensMiscellaneousfeatureGetAttributes *********************************
**
** Get all Ensembl Attributes of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
**
** @return [const AjPList] AJAX List of Ensembl Attributes.
** @@
******************************************************************************/

const AjPList ensMiscellaneousfeatureGetAttributes(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;
    
    return mf->Attributes;
}




/* @func ensMiscellaneousfeatureGetMiscellaneoussets **************************
**
** Get all Ensembl Miscellaneous Sets of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
**
** @return [const AjPList] AJAX List of Ensembl Miscellaneous Sets.
** @@
******************************************************************************/

const AjPList ensMiscellaneousfeatureGetMiscellaneoussets(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;
    
    return mf->Miscellaneoussets;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
** @fnote None
**
** @nam3rule Set Set one element of a Miscellaneous Feature
** @nam4rule SetAdaptor Set the Ensembl Miscellaneous Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule AddAttribute Add an Ensembl Attribute
** @nam4rule AddMiscellaneousset Add an Ensembl Miscellaneous Set
**
** @argrule * mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                          Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMiscellaneousfeatureSetAdaptor ************************************
**
** Set the Ensembl Miscellaneous Feature Adaptor element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [r] adaptor [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                      Feature Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetAdaptor(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousfeatureadaptor adaptor)
{
    if(!mf)
        return ajFalse;
    
    mf->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureSetIdentifier *********************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [r] identifier [ajuint] SQL database-internal identifier.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetIdentifier(EnsPMiscellaneousfeature mf,
                                            ajuint identifier)
{
    if(!mf)
        return ajFalse;
    
    mf->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureSetFeature ************************************
**
** Set the Ensembl Feature element of an Ensembl Miscellaneous Feature.
**
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [u] feature [EnsPFeature] Ensembl Feature.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetFeature(EnsPMiscellaneousfeature mf,
                                         EnsPFeature feature)
{
    if(!mf)
        return ajFalse;
    
    ensFeatureDel(&(mf->Feature));
    
    mf->Feature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureAddAttribute **********************************
**
** Add an Ensembl Attribute to an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Miscellaneousfeature::add_Attribute
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [u] attribute [EnsPAttribute] Ensembl Attribute.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureAddAttribute(EnsPMiscellaneousfeature mf,
                                           EnsPAttribute attribute)
{
    if(!mf)
        return ajFalse;
    
    if(!attribute)
	return ajFalse;
    
    ajListPushAppend(mf->Attributes, (void *) ensAttributeNewRef(attribute));
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureAddMiscellaneousset ***************************
**
** Add an Ensembl Miscellaneous Set to an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Miscellaneousfeature::add_MiscSet
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureAddMiscellaneousset(EnsPMiscellaneousfeature mf,
                                                  EnsPMiscellaneousset ms)
{
    if(!mf)
        return ajFalse;
    
    if(!ms)
	return ajFalse;
    
    ajListPushAppend(mf->Miscellaneoussets,
		     (void *) ensMiscellaneoussetNewRef(ms));
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
** @nam3rule Trace Report Ensembl Miscellaneous Feature elements to debug file
**
** @argrule Trace mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                    Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneousfeatureTrace *****************************************
**
** Trace an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
** @param [r] level [ajuint] Indentation level.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureTrace(const EnsPMiscellaneousfeature mf,
                                    ajuint level)
{
    AjPStr indent = NULL;
    
    if(!mf)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensMiscellaneousfeatureTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Feature %p\n"
	    "%S  Attributes %p\n"
	    "%S  Miscellaneoussets %p\n",
	    indent, mf,
	    indent, mf->Use,
	    indent, mf->Identifier,
	    indent, mf->Adaptor,
	    indent, mf->Feature,
	    indent, mf->Attributes,
	    indent, mf->Miscellaneoussets);
    
    ensFeatureTrace(mf->Feature, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureGetMemSize ************************************
**
** Get the memory size in bytes of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature.
**
** @return [ajuint] Memory size.
** @@
******************************************************************************/

ajuint ensMiscellaneousfeatureGetMemSize(const EnsPMiscellaneousfeature mf)
{
    ajuint size = 0;
    
    AjIList iter = NULL;
    
    EnsPAttribute attribute = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    if(!mf)
	return 0;
    
    size += (ajuint) sizeof (EnsOMiscellaneousfeature);
    
    size += ensFeatureGetMemSize(mf->Feature);
    
    if(mf->Attributes)
    {
	iter = ajListIterNewread(mf->Attributes);
	
	while(!ajListIterDone(iter))
	{
	    attribute = (EnsPAttribute) ajListIterGet(iter);
	    
	    size += ensAttributeGetMemSize(attribute);
	}
	
	ajListIterDel(&iter);
    }
    
    if(mf->Miscellaneoussets)
    {
	iter = ajListIterNewread(mf->Miscellaneoussets);
	
	while(!ajListIterDone(iter))
	{
	    ms = (EnsPMiscellaneousset) ajListIterGet(iter);
	    
	    size += ensMiscellaneoussetGetMemSize(ms);
	}
	
	ajListIterDel(&iter);
    }
    
    return size;
}




/* @func ensMiscellaneousfeatureFetchAllAttributes ****************************
**
** Fetch all Ensembl Attributes of an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_Attributes
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [r] code [const AjPStr] Ensembl Attribute code.
** @param [r] attributes [AjPList] AJAX List of Ensembl Attributes.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
** The caller is responsible for deleting the Ensembl Attributes from the
** AJAX List before deleting the List.
******************************************************************************/

AjBool ensMiscellaneousfeatureFetchAllAttributes(EnsPMiscellaneousfeature mf,
                                                 const AjPStr code,
                                                 AjPList attributes)
{
    AjIList iter = NULL;
    
    EnsPAttribute attribute = NULL;
    
    if(!mf)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    iter = ajListIterNewread(mf->Attributes);
    
    while(!ajListIterDone(iter))
    {
	attribute = (EnsPAttribute) ajListIterGet(iter);
	
	if(code && ajStrGetLen(code))
	{
	    if(ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
		ajListPushAppend(attributes,
				 (void *) ensAttributeNewRef(attribute));
	}
	else
	    ajListPushAppend(attributes,
			     (void *) ensAttributeNewRef(attribute));
    }
    
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureFetchAllMiscellaneoussets *********************
**
** Fetch all Ensembl Miscellaneous Sets of an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_MiscSets
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature.
** @param [r] code [const AjPStr] Ensembl Miscellaneous Set code.
** @param [r] mslist [AjPList] AJAX List of Ensembl Miscellanaeous Sets.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
** The caller is responsible for deleting the Ensembl Miscellaneous Sets from
** the AJAX List before deleting the List.
******************************************************************************/

AjBool ensMiscellaneousfeatureFetchAllMiscellaneoussets(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList mslist)
{
    AjIList iter = NULL;
    
    EnsPMiscellaneousset ms = NULL;
    
    if(!mf)
	return ajFalse;
    
    if(!mslist)
	return ajFalse;
    
    iter = ajListIterNewread(mf->Miscellaneoussets);
    
    while(!ajListIterDone(iter))
    {
	ms = (EnsPMiscellaneousset) ajListIterGet(iter);
	
	if(code && ajStrGetLen(code))
	{
	    if(ajStrMatchCaseS(code, ensMiscellaneoussetGetCode(ms)))
		ajListPushAppend(mslist,
				 (void *) ensMiscellaneoussetNewRef(ms));
	}
	else
	    ajListPushAppend(mslist, (void *) ensMiscellaneoussetNewRef(ms));
    }
    
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @datasection [EnsPMiscellaneousfeatureadaptor] Miscellaneous Feature Adaptor
**
** Functions for manipulating Ensembl Miscellaneous Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureadaptor CVS Revision: 1.22
**
** @nam2rule Miscellaneousfeatureadaptor
**
******************************************************************************/

static const char *miscellaneousFeatureadaptorTables[] =
{
    "misc_feature",
    "misc_feature_misc_set",
    "misc_attrib",
    "attrib_type",
    NULL
};




static const char *miscellaneousFeatureadaptorColumns[] =
{
    "misc_feature.misc_feature_id",
    "misc_feature.seq_region_id",
    "misc_feature.seq_region_start",
    "misc_feature.seq_region_end",
    "misc_feature.seq_region_strand",
    "misc_attrib.value",
    "attrib_type.code",
    "attrib_type.name",
    "attrib_type.description",
    "misc_feature_misc_set.misc_set_id",
    NULL
};




static EnsOBaseadaptorLeftJoin miscellaneousFeatureadaptorLeftJoin[] =
{
    {
	"misc_feature_misc_set",
	"misc_feature.misc_feature_id = misc_feature_misc_set.misc_feature_id"
    },
    {
	"misc_attrib",
	"misc_feature.misc_feature_id = misc_attrib.misc_feature_id"
    },
    {
	"attrib_type",
	"misc_attrib.attrib_type_id = attrib_type.attrib_type_id"
    },
    {NULL, NULL}
};




static const char *miscellaneousFeatureadaptorDefaultCondition = NULL;

static const char *miscellaneousFeatureadaptorFinalCondition =
" ORDER BY misc_feature.misc_feature_id";




/* @funcstatic miscellaneousFeatureadaptorHasAttribute ************************
**
** Check whether an Ensembl Attribute has already been indexed for a particular
** Ensembl Miscellaneous Feature.
**
** @param [u] attributes [AjPTable] Attribute Table.
** @param [r] code [const AjPStr] Ensembl Attribute code.
** @param [r] value [const AjPStr] Ensembl Attribute value.
**
** @return [AjBool] ajTrue: A particular code:value String has already
**                          been indexed.
**                  ajFalse: A particular code:value String has not been
**                           indexed before, but has been added now.
** @@
** This function keeps an AJAX Table of Attribute code:value String key
** data and AjBool value data.
******************************************************************************/

static AjBool miscellaneousFeatureadaptorHasAttribute(AjPTable attributes,
                                                      const AjPStr code,
                                                      const AjPStr value)
{
    AjBool *Pbool = NULL;
    
    AjPStr key = NULL;
    
    key = ajFmtStr("%S:$S", code, value);
    
    if(ajTableFetch(attributes, (const void *) key))
    {
	ajStrDel(&key);
	
	return ajTrue;
    }
    else
    {
	AJNEW0(Pbool);
	
	*Pbool = ajTrue;
	
	ajTablePut(attributes, (void *) key, (void *) Pbool);
	
	return ajFalse;
    }
}




/* @funcstatic miscellaneousFeatureadaptorClearAttributes *********************
**
** Clear an Ensembl Miscellaneous Feature Adaptor-internal
** Ensembl Attribute cache.
**
** This function clears Attribute code:value AJAX String key and
** AJAX Boolean value data from the AJAX Table.
**
** @param [u] attributes [AjPTable] Attribute cache Table.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousFeatureadaptorClearAttributes(AjPTable attributes)
{
    void **keyarray = NULL;
    void **valarray = NULL;
    
    register ajuint i = 0;
    
    if(!attributes)
	return ajFalse;
    
    ajTableToarrayKeysValues(attributes, &keyarray, &valarray);
    
    for(i = 0; keyarray[i]; i++)
    {
	/* Delete the AJAX String key data. */
	
	ajStrDel((AjPStr *) &keyarray[i]);
	
	/* Delete the AjBool value data. */
	
	AJFREE(valarray[i]);
    }
    
    AJFREE(keyarray);
    AJFREE(valarray);
    
    return ajTrue;
}




/* @funcstatic miscellaneousFeatureadaptorHasMiscellaneousset *****************
**
** Check whether an Ensembl Miscellaneous Set has already been indexed for a
** particular Ensembl Miscellaneous Feature.
**
** @param [u] sets [AjPTable] Miscellaneous Sets Table.
** @param [r] msid [ajuint] Ensembl Miscellaneous Set identifier.
**
** @return [AjBool] ajTrue: A particular Miscellaneous Set identifier
**                          has already been indexed.
**                  ajFalse: A particular Miscellaneous Set identifier has not
**                           been indexed before, but has been added now.
** @@
** This function keeps an AJAX Table of Miscellaneous Set identifier unsigned
** integer key data and AjBool value data.
******************************************************************************/

static AjBool miscellaneousFeatureadaptorHasMiscellaneousset(AjPTable sets,
                                                             ajuint msid)
{
    AjBool *Pbool = NULL;
    
    ajuint *Pidentifier = NULL;
    
    if(ajTableFetch(sets, (const void *) &msid))
	return ajTrue;
    else
    {
	AJNEW0(Pidentifier);
	
	*Pidentifier = msid;
	
	AJNEW0(Pbool);
	
	*Pbool = ajTrue;
	
	ajTablePut(sets, (void *) Pidentifier, (void *) Pbool);
	
	return ajFalse;
    }
}




/* @funcstatic miscellaneousFeatureadaptorClearMiscellaneoussets **************
**
** Clear an Ensembl Miscellaneous Feature Adaptor-internal
** Ensembl Miscellaneous Set cache.
**
** This function clears Miscellaneous Set identifier unsigned integer key and
** AJAX Boolean value data from the AJAX Table.
**
** @param [u] sets [AjPTable] Miscellaneous Sets cache Table.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousFeatureadaptorClearMiscellaneoussets(AjPTable sets)
{
    void **keyarray = NULL;
    void **valarray = NULL;
    
    register ajuint i = 0;
    
    if(!sets)
	return ajFalse;
    
    ajTableToarrayKeysValues(sets, &keyarray, &valarray);
    
    for(i = 0; keyarray[i]; i++)
    {
	/* Delete the unsigned integer key data. */
	
	AJFREE(keyarray[i]);
	
	/* Delete the AjBool value data. */
	
	AJFREE(valarray[i]);
    }
    
    AJFREE(keyarray);
    AJFREE(valarray);
    
    return ajTrue;
}




/* @funcstatic miscellaneousFeatureadaptorFetchAllBySQL ***********************
**
** Fetch all Ensembl Miscellaneous Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::Miscellaneousfeatureadaptor::_objs_from_sth
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] statement [const AjPStr] SQL statement.
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper.
** @param [r] slice [EnsPSlice] Ensembl Slice.
** @param [u] mflist [AjPList] AJAX List of Ensembl Miscellaneous Feature
**                             objects.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool miscellaneousFeatureadaptorFetchAllBySQL(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mflist)
{
    ajuint identifier = 0;
    ajuint msid       = 0;
    
    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;
    
    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    
    ajuint current = 0;
    ajuint throw = 0;
    
    AjPList mrs = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr avalue = NULL;
    AjPStr atcode = NULL;
    AjPStr atname = NULL;
    AjPStr atdescription = NULL;
    
    AjPTable attributes = NULL;
    AjPTable sets       = NULL;
    
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPAttribute attribute = NULL;
    
    EnsPCoordsystemadaptor csa = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPMiscellaneousfeature mf         = NULL;
    EnsPMiscellaneousfeatureadaptor mfa = NULL;
    
    EnsPMiscellaneousset ms         = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
    ajDebug("miscellaneousFeatureadaptorFetchAllBySQL\n"
	    "  dba %p\n"
	    "  statement %p\n"
	    "  am %p\n"
	    "  slice %p\n"
	    "  mflist %p\n",
	    dba,
	    statement,
	    am,
	    slice,
	    mflist);
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!mflist)
	return ajFalse;
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    mfa = ensRegistryGetMiscellaneousfeatureadaptor(dba);
    
    msa = ensRegistryGetMiscellaneoussetadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    if(slice)
	ama = ensRegistryGetAssemblymapperadaptor(dba);
    
    attributes = MENSTABLEUINTNEW(0);
    
    sets = ajTablestrNewLen(0);
    
    mrs = ajListNew();
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier    = 0;
	srid          = 0;
	srstart       = 0;
	srend         = 0;
	srstrand      = 0;
	avalue        = ajStrNew();
	atcode        = ajStrNew();
	atname        = ajStrNew();
	atdescription = ajStrNew();
	msid          = 0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToStr(sqlr, &avalue);
	ajSqlcolumnToStr(sqlr, &atcode);
        ajSqlcolumnToStr(sqlr, &atname);
	ajSqlcolumnToStr(sqlr, &atdescription);
	ajSqlcolumnToUint(sqlr, &msid);
	
	if(identifier == throw)
	{
	    /*
	    ** This Miscellanous Feature is already known to map into a gap,
	    ** therefore throw it away immediately.
	    */
	    
	    ajStrDel(&avalue);
	    ajStrDel(&atcode);
	    ajStrDel(&atname);
	    ajStrDel(&atdescription);
	    
	    continue;
	}
	
	if(identifier == current)
	{
	    /* Still working on the same Miscellaneous Feature. */
	    
	    if(msid)
		ensMiscellaneoussetadaptorFetchByIdentifier(msa, msid, &ms);
	    
	    if(ajStrGetLen(atcode) &&
               ajStrGetLen(avalue) &&
               !miscellaneousFeatureadaptorHasAttribute(attributes,
                                                        atcode,
                                                        avalue))
		attribute =
		    ensAttributeNew(atcode, atname, atdescription, avalue);
	}
	else
	{
	    /*
	    ** This is a new Miscellaneous Feature, therefore clear all
	    ** internal caches.
	    */
	    
	    miscellaneousFeatureadaptorClearAttributes(attributes);
	    
	    miscellaneousFeatureadaptorClearMiscellaneoussets(sets);
	    
	    current = identifier;
	    
	    throw = 0;
	    
	    /* Need to get the internal Ensembl Sequence Region identifier. */
	    
	    srid =
		ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);
	    
	    /*
	    ** Since the Ensembl SQL schema defines Sequence Region start and
	    ** end coordinates as unsigned integers for all Features, the
	    ** range needs checking.
	    */
	    
	    if(srstart <= INT_MAX)
		slstart = (ajint) srstart;
	    else
		ajFatal("miscellaneousFeatureadaptorFetchAllBySQL got a "
			"Sequence Region start coordinate (%u) outside the "
			"maximum integer limit (%d).",
			srstart, INT_MAX);
	    
	    if(srend <= INT_MAX)
		slend = (ajint) srend;
	    else
		ajFatal("miscellaneousFeatureadaptorFetchAllBySQL got a "
			"Sequence Region end coordinate (%u) outside the "
			"maximum integer limit (%d).",
			srend, INT_MAX);
	    
	    slstrand = srstrand;
	    
	    /* Fetch a Slice spanning the entire Sequence Region. */
	    
	    ensSliceadaptorFetchBySeqregionIdentifier(sa,
						      srid,
						      0,
						      0,
						      0,
						      &srslice);
	    
	    /*
	    ** Obtain an Ensembl Assembly Mapper if none was defined, but a
	    ** destination Slice was.
	    */
	    
	    if(am)
		am = ensAssemblymapperNewRef(am);
	    
	    if((!am) &&
		slice &&
		(!ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
				       ensSliceGetCoordsystem(srslice))))
		am = ensAssemblymapperadaptorFetchByCoordsystems(
                    ama,
                    ensSliceGetCoordsystem(slice),
                    ensSliceGetCoordsystem(srslice));
	    
	    /*
	    ** Remap the Feature coordinates to another Ensembl Coordinate
	    ** System if an Ensembl Assembly Mapper was provided.
	    */
	    
	    if(am)
	    {
		ensAssemblymapperFastMap(am,
					 ensSliceGetSeqregion(srslice),
					 slstart,
					 slend,
					 slstrand,
					 mrs);
		
		/*
		** The ensAssemblymapperFastMap function returns at best one
		** Ensembl Mapper Result.
		*/
		
		ajListPop(mrs, (void **) &mr);
		
		/*
		** Skip Features that map to gaps or
		** Coordinate System boundaries.
		*/
		
		if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
		{
		    /* Load the next Feature but destroy first! */
		    
		    ajStrDel(&avalue);
		    ajStrDel(&atcode);
		    ajStrDel(&atname);
		    ajStrDel(&atdescription);
		    
		    ensSliceDel(&srslice);
		    
		    ensAssemblymapperDel(&am);
		    
		    ensMapperresultDel(&mr);
		    
		    /* Mark the Miscellaneous Feature as one to throw away. */
		    
		    throw = identifier;
		    
		    continue;
		}
		
		srid = ensMapperresultGetObjectIdentifier(mr);
		
		slstart = ensMapperresultGetStart(mr);
		
		slend = ensMapperresultGetEnd(mr);
		
		slstrand = ensMapperresultGetStrand(mr);
		
		/*
		** Delete the Sequence Region Slice and fetch a Slice
		** in the Coordinate System we just mapped to.
		*/
		
		ensSliceDel(&srslice);
		
		ensSliceadaptorFetchBySeqregionIdentifier(sa,
							  srid,
							  0,
							  0,
							  0,
							  &srslice);
		
		ensMapperresultDel(&mr);
	    }
	    
	    /*
	    ** Convert Sequence Region Slice coordinates to destination Slice
	    ** coordinates, if a destination Slice has been provided.
	    */
	    
	    if(slice)
	    {
		/* Check that the length of the Slice is within range. */
		
		if(ensSliceGetLength(slice) <= INT_MAX)
		    sllength = (ajint) ensSliceGetLength(slice);
		else
		    ajFatal("miscellaneousFeatureadaptorFetchAllBySQL "
			    "got a Slice, "
			    "whose length (%u) exceeds the "
			    "maximum integer limit (%d).",
			    ensSliceGetLength(slice), INT_MAX);
		
		/*
		** Nothing needs to be done if the destination Slice starts
		** at 1 and is on the forward strand.
		*/
		
		if((ensSliceGetStart(slice) != 1) ||
		    (ensSliceGetStrand(slice) < 0))
		{
		    if(ensSliceGetStrand(slice) >= 0)
		    {
			slstart = slstart - ensSliceGetStart(slice) + 1;
			
			slend = slend - ensSliceGetStart(slice) + 1;
		    }
		    else
		    {
			slend = ensSliceGetEnd(slice) - slstart + 1;
			
			slstart = ensSliceGetEnd(slice) - slend + 1;
			
			slstrand *= -1;
		    }
		}
		
		/*
		** Throw away Features off the end of the requested Slice or
		** on any other than the requested Slice.
		*/
		
		if((slend < 1) ||
		    (slstart > sllength) ||
		    (srid != ensSliceGetSeqregionIdentifier(slice)))
		{
		    /* Load the next Feature but destroy first! */
		    
		    ajStrDel(&avalue);
		    ajStrDel(&atcode);
		    ajStrDel(&atname);
		    ajStrDel(&atdescription);
		    
		    ensSliceDel(&srslice);
		    
		    ensAssemblymapperDel(&am);
		    
		    /* Mark the Miscellaneous Feature as one to throw away. */
		    
		    throw = identifier;
		    
		    continue;
		}
		
		/*
		** Delete the Sequence Region Slice and set the
		** requested Slice.
		*/
		
		ensSliceDel(&srslice);
		
		srslice = ensSliceNewRef(slice);
	    }
	    
	    /* Finally, create a new Ensembl Miscellaneous Feature. */
	    
	    feature = ensFeatureNewS((EnsPAnalysis) NULL,
				     srslice,
				     slstart,
				     slend,
				     slstrand);
	    
	    mf = ensMiscellaneousfeatureNew(mfa, identifier, feature);
	    
	    ajListPushAppend(mflist, (void *) mf);
	    
	    /* Add an Ensembl Attribute if one was defined. */
	    
	    if(ajStrGetLen(atcode) && ajStrGetLen(avalue))
	    {
		attribute =
		ensAttributeNew(atcode, atname, atdescription, avalue);
		
		if(!miscellaneousFeatureadaptorHasAttribute(attributes,
							      atcode,
							      avalue))
		    ensMiscellaneousfeatureAddAttribute(mf, attribute);
		
		ensAttributeDel(&attribute);
	    }
	    
	    /* Add an Ensembl Miscellaneous Set if one was defined. */
	    
	    if(msid)
	    {
		if(!miscellaneousFeatureadaptorHasMiscellaneousset(sets, msid))
		{
		    ensMiscellaneoussetadaptorFetchByIdentifier(msa, msid, &ms);
		    
		    ensMiscellaneousfeatureAddMiscellaneousset(mf, ms);
		    
		    ensMiscellaneoussetDel(&ms);
		}
	    }
	    
	    ensFeatureDel(&feature);
	    
	    ensAssemblymapperDel(&am);
	    
	    ensSliceDel(&srslice);
	    
	    ajStrDel(&avalue);
	    ajStrDel(&atcode);
	    ajStrDel(&atname);
	    ajStrDel(&atdescription);
	}
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    miscellaneousFeatureadaptorClearAttributes(attributes);
    
    ajTableFree(&attributes);
    
    miscellaneousFeatureadaptorClearMiscellaneoussets(sets);
    
    ajTableFree(&sets);
    
    return ajTrue;
}




/* @funcstatic miscellaneousFeatureadaptorCacheReference **********************
**
** Wrapper function to reference an Ensembl Miscellaneous Feature
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Miscellaneous Feature.
**
** @return [void *] Ensembl Miscellaneous Feature or NULL;
** @@
******************************************************************************/

static void *miscellaneousFeatureadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *)
        ensMiscellaneousfeatureNewRef((EnsPMiscellaneousfeature) value);
}




/* @funcstatic miscellaneousFeatureadaptorCacheDelete *************************
**
** Wrapper function to delete an Ensembl Miscellaneous Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Miscellaneous Feature address.
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousFeatureadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensMiscellaneousfeatureDel((EnsPMiscellaneousfeature *) value);
    
    return;
}




/* @funcstatic miscellaneousFeatureadaptorCacheSize ***************************
**
** Wrapper function to determine the memory size of an
** Ensembl Miscellaneous Feature from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Miscellaneous Feature.
**
** @return [ajuint] Memory size.
** @@
******************************************************************************/

static ajuint miscellaneousFeatureadaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensMiscellaneousfeatureGetMemSize(
        (const EnsPMiscellaneousfeature) value);
}




/* @funcstatic miscellaneousFeatureadaptorGetFeature **************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Miscellaneous Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Miscellaneous Feature.
**
** @return [EnsPFeature] Ensembl Feature.
** @@
******************************************************************************/

static EnsPFeature miscellaneousFeatureadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensMiscellaneousfeatureGetFeature(
        (const EnsPMiscellaneousfeature) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Feature Adaptor by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                       Feature Adaptor
** @argrule Ref object [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                       Feature Adaptor
**
** @valrule * [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                              Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorNew ************************************
**
** Default Ensembl Miscellaneous Feature Adaptor constructor.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureadaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
**
** @return [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous Feature
**                                           Adaptor or NULL.
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMiscellaneousfeatureadaptor mfa = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(mfa);
    
    mfa->Adaptor =
	ensFeatureadaptorNew(dba,
			     miscellaneousFeatureadaptorTables,
			     miscellaneousFeatureadaptorColumns,
			     miscellaneousFeatureadaptorLeftJoin,
			     miscellaneousFeatureadaptorDefaultCondition,
			     miscellaneousFeatureadaptorFinalCondition,
			     miscellaneousFeatureadaptorFetchAllBySQL,
			     (void* (*)(const void* key)) NULL, /* Fread */
			     miscellaneousFeatureadaptorCacheReference,
			     (AjBool (*)(const void* value)) NULL, /* Fwrite */
			     miscellaneousFeatureadaptorCacheDelete,
			     miscellaneousFeatureadaptorCacheSize,
			     miscellaneousFeatureadaptorGetFeature,
			     "Miscellaneous Feature");
    
    return mfa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Miscellaneous Set Adaptor.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Miscellaneous Feature Adaptor object
**
** @argrule * Padaptor [EnsPMiscellaneousfeatureadaptor*] Ensembl Miscellaneous
**                                                        Feature Adaptor
**                                                        address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorDel ************************************
**
** Default destructor for an Ensembl Miscellaneous Feature Adaptor.
**
** @param [d] Padaptor [EnsPMiscellaneousfeatureadaptor*] Ensembl Miscellaneous
**                                                        Feature Adaptor
**                                                        address.
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneousfeatureadaptorDel(
    EnsPMiscellaneousfeatureadaptor *Padaptor)
{
    EnsPMiscellaneousfeatureadaptor pthis = NULL;
    
    if(!Padaptor)
	return;
    
    if(!*Padaptor)
	return;

    pthis = *Padaptor;
    
    ensFeatureadaptorDel(&pthis->Adaptor);
    
    AJFREE(pthis);

    *Padaptor = NULL;
    
    return;
}




/* @section object fetching ***************************************************
**
** Functions for retrieving Ensembl Miscellaneous Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Miscellaneous Feature object(s)
** @nam4rule FetchAll Retrieve all Ensembl Miscellaneous Feature objects.
** @nam5rule FetchAllBy Retrieve all Ensembl Miscellaneous Feature objects
**                      matching a criterion.
** @nam4rule FetchBy Retrieve one Ensembl Miscellaneous Feature object
**                   matching a criterion.
**
** @argrule * adaptor [const EnsPMiscellaneousfeatureadaptor] Ensembl
**                                                            Miscellaneous
**                                                            Feature Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Miscellaneous Feature
**                             objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise.
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorFetchAllBySliceAndSetCode **************
**
** Fetch all Ensembl Miscellaneous Features by an Ensembl Slice and
** Ensembl Miscellaneous Set codes.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureadaptor::
**     fetch_all_by_Slice_and_set_code
** @param [r] adaptor [const EnsPMiscellaneousfeatureadaptor] Ensembl
**                                                            Miscellaneous
**                                                            Feature Adaptor.
** @param [u] slice [EnsPSlice] Ensembl Slice.
** @param [r] codes [const AjPList] Ensembl Miscellaneous Set codes.
** @param [u] mflist [AjPList] AJAX List of Ensembl Miscellaneous Features.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorFetchAllBySliceAndSetCode(
    const EnsPMiscellaneousfeatureadaptor adaptor,
    EnsPSlice slice,
    const AjPList codes,
    AjPList mflist)
{
    ajuint maxlen = 0;
    
    AjIList iter   = NULL;
    AjPList mslist = NULL;
    
    AjPStr code       = NULL;
    AjPStr constraint = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPMiscellaneousset ms         = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!codes)
	return ajFalse;
    
    if(!mflist)
	return ajFalse;
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
    
    msa = ensRegistryGetMiscellaneoussetadaptor(dba);
    
    mslist = ajListNew();
    
    iter = ajListIterNewread(codes);
    
    while(!ajListIterDone(iter))
    {
	code = (AjPStr) ajListIterGet(iter);
	
	ensMiscellaneoussetadaptorFetchByCode(msa, code, &ms);
	
	if(ms)
	{
	    maxlen = (ensMiscellaneoussetGetMaximumLength(ms) > maxlen) ?
	    ensMiscellaneoussetGetMaximumLength(ms) : maxlen;
	    
	    ajListPushAppend(mslist, (void *) ms);
	}
	else
	    ajWarn("ensMiscellaneousfeatureadaptorFetchAllBySliceAndSetCode "
		   "no Miscellaneous Set with code '%S'.\n", code);
    }
    
    ajListIterDel(&iter);
    
    if(ajListGetLength(mslist) > 0)
    {
	constraint = ajStrNewC("misc_feature_misc_set.misc_set_id IN (");
	
	while(ajListPop(mslist, (void **) &ms))
	{
	    ajFmtPrintAppS(&constraint, "%u, ",
			   ensMiscellaneoussetGetIdentifier(ms));
	    
	    ensMiscellaneoussetDel(&ms);
	}
	
	/* Remove last comma and space from the constraint. */
	
	ajStrCutEnd(&constraint, 2);
	
	ajStrAppendC(&constraint, ")");
	
	ensFeatureadaptorSetMaximumFeatureLength(adaptor->Adaptor, maxlen);
	
	ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						   slice,
						   constraint,
						   (const AjPStr) NULL,
						   mflist);
	
	ensFeatureadaptorSetMaximumFeatureLength(adaptor->Adaptor, 0);
	
	ajStrDel(&constraint);
    }
    
    ajListFree(&mslist);
    
    return ajTrue;
}




/* @func ensMiscellaneousfeatureadaptorFetchAllByAttributeCodeValue ***********
**
** Fetch all Ensembl Miscellaneous Features by an Ensembl Slice and
** Ensembl Miscellaneous Set codes.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureadaptor::
**     fetch_all_by_attribute_type_value
** @param [r] adaptor [const EnsPMiscellaneousfeatureadaptor] Ensembl
**                                                            Miscellaneous
**                                                            Feature Adaptor.
** @param [r] code [const AjPStr] Attribute type codes.
** @param [r] value [const AjPStr] Miscellaneous attribute value.
** @param [u] mflist [AjPList] AJAX List of Ensembl Miscellaneous Features.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorFetchAllByAttributeCodeValue(
    const EnsPMiscellaneousfeatureadaptor adaptor,
    const AjPStr code,
    const AjPStr value,
    AjPList mflist)
{
    ajuint counter = 0;
    ajuint mfid    = 0;
    
    char *txtcode  = NULL;
    char *txtvalue = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr constraint  = NULL;
    AjPStr identifiers = NULL;
    AjPStr statement   = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!code)
	ajFatal("ensMiscellaneousfeatureadaptorFetchAllByAttributeCodeValue "
		"requires an Ensembl Attribute code.\n");
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
    
    /*
    ** Need to do two queries so that all of the ids come back with the
    ** features.  The problem with adding attrib constraints to filter the
    ** misc_features which come back is that not all of the attributes will
    ** come back
    */
    
    ensDatabaseadaptorEscapeC(dba, &txtcode, code);
    
    statement =
	ajFmtStr("SELECT DISTINCT "
		 "misc_attrib.misc_feature_id "
		 "FROM "
		 "misc_attrib, "
		 "attrib_type, "
		 "misc_feature, "
		 "seq_region, "
		 "coord_system "
		 "WHERE "
		 "misc_attrib.attrib_type_id = attrib_type.attrib_type_id "
		 "AND "
		 "attrib_type.code = '%s' "
		 "AND "
		 "misc_attrib.misc_feature_id = misc_feature.misc_feature_id "
		 "AND "
		 "misc_feature.seq_region_id = seq_region.seq_region_id "
		 "AND "
		 "seq_region.coord_system_id = coord_system.coord_system_id "
		 "AND "
		 "coord_system.species_id = %u",
		 txtcode,
		 ensDatabaseadaptorGetIdentifier(dba));
    
    ajCharDel(&txtcode);
    
    if(value)
    {
	ensDatabaseadaptorEscapeC(dba, &txtvalue, value);
	
	ajFmtPrintAppS(&statement, " AND misc_attrib.value = '%s'", txtvalue);
	
	ajCharDel(&txtvalue);
    }
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    identifiers = ajStrNew();
    
    while(!ajSqlrowiterDone(sqli))
    {
        mfid = 0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
        ajSqlcolumnToUint(sqlr, &mfid);
	
	ajFmtPrintAppS(&identifiers, "%u, ", mfid);
	
	/*
	** Construct constraints from the list of identifiers.  Split
	** identifiers into groups of 1000 to ensure that the statement is not
	** too big.
	*/
    	
	counter++;
	
	if(counter > 1000)
	{
	    constraint = ajFmtStr("misc_feature.misc_feature_id in (%S)",
				  identifiers);
	    
	    ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       mflist);
	    
	    ajStrDel(&constraint);
	    
	    ajStrSetClear(&identifiers);
	    
	    counter = 0;
	}
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajStrDel(&statement);
    
    /* Run the last statement. */
    
    constraint = ajFmtStr("misc_feature.misc_feature_id in (%S)",
			  identifiers);
    
    ensBaseadaptorGenericFetch(ba,
			       constraint,
			       (EnsPAssemblymapper) NULL,
			       (EnsPSlice) NULL,
			       mflist);
    
    
    
    ajStrDel(&constraint);
    ajStrDel(&identifiers);
    
    return ajTrue;
}

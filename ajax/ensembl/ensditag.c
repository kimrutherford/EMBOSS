/******************************************************************************
** @source Ensembl diTag functions
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

#include "ensditag.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

static const char *diTagFeatureSide[] =
{
    NULL,
    "L",
    "R",
    "F",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPAnalysisadaptor
ensRegistryGetAnalysisadaptor(EnsPDatabaseadaptor dba);

extern EnsPAssemblymapperadaptor
ensRegistryGetAssemblymapperadaptor(EnsPDatabaseadaptor dba);

extern EnsPDitagfeatureadaptor
ensRegistryGetDitagfeatureadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static AjBool diTagadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                        const AjPStr statement,
                                        AjPList list);

static AjBool diTagFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                               const AjPStr statement,
                                               EnsPAssemblymapper am,
                                               EnsPSlice slice,
                                               AjPList dtfs);

static void *diTagFeatureadaptorCacheReference(void *value);

static void diTagFeatureadaptorCacheDelete(void **value);

static ajuint diTagFeatureadaptorCacheSize(const void *value);

static EnsPFeature diTagFeatureadaptorGetFeature(const void *value);




/* @filesection ensditag ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPDitag] Ditag *********************************************
**
** Functions for manipulating Ensembl Ditag objects
**
** @cc Bio::EnsEMBL::Map::Ditag CVS Revision: 1.4
**
** @nam2rule Ditag
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditags. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitag]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPDitag] Ensembl Ditag
** @argrule Ref object [EnsPDitag] Ensembl Ditag
**
** @valrule * [EnsPDitag] Ensembl Ditag
**
** @fcategory new
******************************************************************************/




/* @func ensDitagNew **********************************************************
**
** Default constructor for an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Map::Ditag::new
** @param [u] name [AjPStr] Name
** @param [u] type [AjPStr] Type
** @param [u] sequence [AjPStr] Sequence
** @param [r] count [ajuint] Count
**
** @return [EnsPDitag] Ensembl Ditag or NULL
** @@
******************************************************************************/

EnsPDitag ensDitagNew(EnsPDatabaseadaptor adaptor,
                      ajuint identifier,
                      AjPStr name,
                      AjPStr type,
                      AjPStr sequence,
                      ajuint count)
{
    EnsPDitag dt = NULL;
    
    AJNEW0(dt);
    
    dt->Use        = 1;
    dt->Identifier = identifier;
    dt->Adaptor    = adaptor;
    
    if(name)
        dt->Name = ajStrNewRef(name);
    
    if(type)
        dt->Type = ajStrNewRef(type);
    
    if(sequence)
        dt->Sequence = ajStrNewRef(sequence);
    
    dt->Count = count;
    
    return dt;
}




/* @func ensDitagNewObj *******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPDitag] Ensembl Ditag
**
** @return [EnsPDitag] Ensembl Ditag or NULL
** @@
******************************************************************************/

EnsPDitag ensDitagNewObj(const EnsPDitag object)
{
    EnsPDitag dt = NULL;
    
    if(!object)
	return NULL;
    
    AJNEW0(dt);
    
    dt->Use        = 1;
    dt->Identifier = object->Identifier;
    dt->Adaptor    = object->Adaptor;
    
    if(object->Name)
        dt->Name = ajStrNewRef(object->Name);
    
    if(object->Type)
        dt->Type = ajStrNewRef(object->Type);
    
    if(object->Sequence)
        dt->Sequence = ajStrNewRef(object->Sequence);
    
    dt->Count = object->Count;
    
    return dt;
}




/* @func ensDitagNewRef *******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dt [EnsPDitag] Ensembl Ditag
**
** @return [EnsPDitag] Ensembl Ditag or NULL
** @@
******************************************************************************/

EnsPDitag ensDitagNewRef(EnsPDitag dt)
{
    if(!dt)
	return NULL;
    
    dt->Use++;
    
    return dt;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Ditag.
**
** @fdata [EnsPDitag]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Ditag object
**
** @argrule * Pdt [EnsPDitag*] Ensembl Ditag object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDitagDel **********************************************************
**
** Default destructor for an Ensembl Ditag.
**
** @param [d] Pdt [EnsPDitag*] Ensembl Ditag address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagDel(EnsPDitag *Pdt)
{
    EnsPDitag pthis = NULL;
    /*
     ajDebug("ensDitagDel\n"
	     "  *Pdt %p\n",
	     *Pdt);
     
     ensDitagTrace(*Pdt, 1);
     */
    
    if(!Pdt)
        return;
    
    if(!*Pdt)
        return;

    pthis = *Pdt;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pdt = NULL;
	
	return;
    }
    
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Sequence);
    
    AJFREE(*Pdt);
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
** @fnote None
**
** @nam3rule Get Return Ditag attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Database Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetType Return the type
** @nam4rule GetSequence Return the sequence
** @nam4rule GetCount Return the count
**
** @argrule * dt [const EnsPDitag] Ditag
**
** @valrule Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Type [AjPStr] Type
** @valrule Sequence [AjPStr] Sequence
** @valrule Count [ajuint] Count
**
** @fcategory use
******************************************************************************/




/* @func ensDitagGetAdaptor ***************************************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDitagGetAdaptor(const EnsPDitag dt)
{
    if(!dt)
        return NULL;
    
    return dt->Adaptor;
}




/* @func ensDitagGetIdentifier ************************************************
**
** Get the SQL database-internal identifier element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensDitagGetIdentifier(const EnsPDitag dt)
{
    if(!dt)
        return 0;
    
    return dt->Identifier;
}




/* @func ensDitagGetName ******************************************************
**
** Get the name element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::name
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensDitagGetName(const EnsPDitag dt)
{
    if(!dt)
        return NULL;
    
    return dt->Name;
}




/* @func ensDitagGetType ******************************************************
**
** Get the type element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::type
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Type
** @@
******************************************************************************/

AjPStr ensDitagGetType(const EnsPDitag dt)
{
    if(!dt)
        return NULL;
    
    return dt->Type;
}




/* @func ensDitagGetSequence **************************************************
**
** Get the sequence element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::sequence
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Sequence
** @@
******************************************************************************/

AjPStr ensDitagGetSequence(const EnsPDitag dt)
{
    if(!dt)
        return NULL;
    
    return dt->Sequence;
}




/* @func ensDitagGetCount *****************************************************
**
** Get the count element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::tag_count
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] Count
** @@
******************************************************************************/

ajuint ensDitagGetCount(const EnsPDitag dt)
{
    if(!dt)
        return 0;
    
    return dt->Count;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Ditag
** @nam4rule SetAdaptor Set the Ensembl Database Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetType Set the type
** @nam4rule SetSequence Set the sequence
** @nam4rule SetCount Set the count
**
** @argrule * dt [EnsPDitag] Ensembl Ditag object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDitagSetAdaptor ***************************************************
**
** Set the Ensembl Database Adaptor element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetAdaptor(EnsPDitag dt, EnsPDatabaseadaptor adaptor)
{
    if(!dt)
        return ajFalse;
    
    dt->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensDitagSetIdentifier ************************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetIdentifier(EnsPDitag dt, ajuint identifier)
{
    if(!dt)
        return ajFalse;
    
    dt->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensDitagSetName ******************************************************
**
** Set the name element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::name
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetName(EnsPDitag dt, AjPStr name)
{
    if(!dt)
        return ajFalse;
    
    ajStrDel(&dt->Name);
    
    dt->Name = ajStrNewRef(name);
    
    return ajTrue;
}




/* @func ensDitagSetType ******************************************************
**
** Set the type element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::type
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] type [AjPStr] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetType(EnsPDitag dt, AjPStr type)
{
    if(!dt)
        return ajFalse;
    
    ajStrDel(&dt->Type);
    
    dt->Type = ajStrNewRef(type);
    
    return ajTrue;
}




/* @func ensDitagSetSequence **************************************************
**
** Set the sequence element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::sequence
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] sequence [AjPStr] Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetSequence(EnsPDitag dt, AjPStr sequence)
{
    if(!dt)
        return ajFalse;
    
    ajStrDel(&dt->Sequence);
    
    dt->Sequence = ajStrNewRef(sequence);
    
    return ajTrue;
}




/* @func ensDitagSetCount *****************************************************
**
** Set the count element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::tag_count
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [r] count [ajuint] Count
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetCount(EnsPDitag dt, ajuint count)
{
    if(!dt)
        return ajFalse;
    
    dt->Count = count;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
** @nam3rule Trace Report Ensembl Ditag elements to debug file.
**
** @argrule Trace dt [const EnsPDitag] Ensembl Ditag
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDitagTrace ********************************************************
**
** Trace an Ensembl Ditag.
**
** @param [r] dt [const EnsPDitag] Ensembl Ditag
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagTrace(const EnsPDitag dt, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!dt)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensDitagTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Name '%S'\n"
	    "%S  Type '%S'\n"
	    "%S  Sequence '%S'\n"
	    "%S  Count %u\n",
	    indent, dt,
	    indent, dt->Use,
	    indent, dt->Identifier,
	    indent, dt->Adaptor,
	    indent, dt->Name,
	    indent, dt->Type,
	    indent, dt->Sequence,
	    indent, dt->Count);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensDitagGetMemSize ***************************************************
**
** Get the memory size in bytes of an Ensembl Ditag.
**
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensDitagGetMemSize(const EnsPDitag dt)
{
    ajuint size = 0;
    
    if(!dt)
	return 0;
    
    size += (ajuint) sizeof (EnsODitag);
    
    if(dt->Name)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(dt->Name);
    }
    
    if(dt->Type)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(dt->Type);
    }
    
    if(dt->Sequence)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(dt->Sequence);
    }
    
    return size;
}




/* @datasection [EnsPDatabaseadaptor] Ensembl Database Adaptor ****************
**
** Functions for manipulating Ensembl Ditag Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor CVS Revision: 1.6
**
** @nam2rule Ditagadaptor
**
******************************************************************************/



/* @funcstatic diTagadaptorFetchAllBySQL **************************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Ditag objects.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::_fetch
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] list [AjPList] AJAX List of Ensembl Ditags
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool diTagadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                        const AjPStr statement,
                                        AjPList list)
{
    ajuint identifier = 0;
    ajuint count      = 0;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr name     = NULL;
    AjPStr type     = NULL;
    AjPStr sequence = NULL;
    
    EnsPDitag dt = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!(statement && ajStrGetLen(statement)))
	return ajFalse;
    
    if(!list)
	return ajFalse;
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier = 0;
	name       = ajStrNew();
	type       = ajStrNew();
	sequence   = ajStrNew();
	count      = 0;
	
        sqlr = ajSqlrowiterGet(sqli);
	
        ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToStr(sqlr, &name);
	ajSqlcolumnToStr(sqlr, &type);
	ajSqlcolumnToStr(sqlr, &sequence);
        ajSqlcolumnToUint(sqlr, &count);
	
	dt = ensDitagNew(dba, identifier, name, type, sequence, count);
	
	ajListPushAppend(list, (void *) dt);
	
	ajStrDel(&name);
	ajStrDel(&type);
	ajStrDel(&sequence);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Ditag objects from an
** Ensembl Core database.
**
** @fdata [EnsPDatabaseadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Ditag object(s)
** @nam4rule FetchAll Retrieve all Ensembl Ditag objects
** @nam5rule FetchAllBy Retrieve all Ensembl Ditag objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Ditag object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Ditag objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDitagadaptorFetchByIdentifier *************************************
**
** Fetch an Ensembl Ditag via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::fetch_by_dbID
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdt [EnsPDitag*] Ensembl Ditag address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchByIdentifier(EnsPDatabaseadaptor dba,
                                        ajuint identifier,
                                        EnsPDitag *Pdt)
{
    AjBool value = AJFALSE;
    
    AjPList dts = NULL;
    
    AjPStr statement = NULL;
    
    EnsPDitag dt = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Pdt)
	return ajFalse;
    
    *Pdt = NULL;
    
    statement = ajFmtStr("SELECT "
			 "ditag.ditag_id, "
			 "ditag.name, "
			 "ditag.type, "
			 "ditag.tag_count, "
			 "ditag.sequence "
			 "FROM "
			 "ditag "
			 "WHERE "
			 "ditag.ditag_id = %u",
			 identifier);
    
    dts = ajListNew();
    
    value = diTagadaptorFetchAllBySQL(dba, statement, dts);
    
    if(ajListGetLength(dts) == 0)
	ajDebug("ensDitagadaptorFetchByIdentifier got no Ensembl Ditag "
		"for identifier %u.\n", identifier);
    
    if(ajListGetLength(dts) > 1)
	ajWarn("ensDitagadaptorFetchByIdentifier got more than one "
	       "Ensembl Ditag for identifier %u.\n", identifier);
    
    ajListPop(dts, (void **) Pdt);
    
    while(ajListPop(dts, (void **) &dt))
	ensDitagDel(&dt);
    
    ajListFree(&dts);
    
    ajStrDel(&statement);
    
    return value;
}




/* @func ensDitagadaptorFetchAll **********************************************
**
** Fetch all Ensembl Ditag objects by name or type.
**
** The caller is responsible for deleting the Ensembl Ditags before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::fetch_all
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::fetch_all_by_name
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::fetch_all_by_type
** @param [u] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [rN] name [const AjPStr] Name
** @param [rN] type [const AjPStr] Type
** @param [u] dts [AjPList] AJAX List of Ensembl Ditags
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAll(EnsPDatabaseadaptor dba,
                               const AjPStr name,
                               const AjPStr type,
                               AjPList dts)
{
    char *txtname = NULL;
    char *txttype = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr statement = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!dts)
	return ajFalse;
    
    if(name && ajStrGetLen(name))
	ensDatabaseadaptorEscapeC(dba, &txtname, name);
    
    if(type && ajStrGetLen(type))
	ensDatabaseadaptorEscapeC(dba, &txttype, type);
    
    statement = ajStrNewC("SELECT "
			  "ditag.ditag_id, "
			  "ditag.name, "
			  "ditag.type, "
			  "ditag.tag_count, "
			  "ditag.sequence "
			  "FROM "
			  "ditag");
    
    if(txtname || txttype)
	ajStrAppendC(&statement, " WHERE");
    
    if(txtname)
	ajFmtPrintAppS(&statement, " ditag.name = '%s'", txtname);
    
    if(txttype)
    {
	if(txtname)
	    ajStrAppendC(&statement, " AND");
	
	ajFmtPrintAppS(&statement, " ditag.type = '%s'", txttype);
    }
    
    ajCharDel(&txtname);
    
    ajCharDel(&txttype);
    
    value = diTagadaptorFetchAllBySQL(dba, statement, dts);
    
    ajStrDel(&statement);
    
    return value;
}




/* @func ensDitagadaptorFetchAllByName ****************************************
**
** Fetch all Ensembl Ditag objects by name.
**
** The caller is responsible for deleting the Ensembl Ditags before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::fetch_all_by_name
** @param [u] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] name [const AjPStr] Name
** @param [u] dts [AjPList] AJAX List of Ensembl Ditags
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllByName(EnsPDatabaseadaptor dba,
                                     const AjPStr name,
                                     AjPList dts)
{
    char *txtname = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr statement = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!name)
	return ajFalse;
    
    if(!dts)
	return ajFalse;
    
    ensDatabaseadaptorEscapeC(dba, &txtname, name);
    
    statement = ajFmtStr("SELECT "
			 "ditag.ditag_id, "
			 "ditag.name, "
			 "ditag.type, "
			 "ditag.tag_count, "
			 "ditag.sequence "
			 "FROM "
			 "ditag "
			 "WHERE "
			 "ditag.name = '%s'",
			 txtname);
    
    ajCharDel(&txtname);
    
    value = diTagadaptorFetchAllBySQL(dba, statement, dts);
    
    ajStrDel(&statement);
    
    return value;
}




/* @func ensDitagadaptorFetchAllByType ****************************************
**
** Fetch all Ensembl Ditag objects by type.
**
** The caller is responsible for deleting the Ensembl Ditags before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagadaptor::fetch_all_by_type
** @param [u] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] type [const AjPStr] Type
** @param [u] dts [AjPList] AJAX List of Ensembl Ditags
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllByType(EnsPDatabaseadaptor dba,
                                     const AjPStr type,
                                     AjPList dts)
{
    char *txttype = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr statement = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!type)
	return ajFalse;
    
    if(!dts)
	return ajFalse;
    
    ensDatabaseadaptorEscapeC(dba, &txttype, type);
    
    statement = ajFmtStr("SELECT "
			 "ditag.ditag_id, "
			 "ditag.name, "
			 "ditag.type, "
			 "ditag.tag_count, "
			 "ditag.sequence "
			 "FROM "
			 "ditag "
			 "WHERE "
			 "ditag.type = '%s'",
			 txttype);
    
    ajCharDel(&txttype);
    
    value = diTagadaptorFetchAllBySQL(dba, statement, dts);
    
    ajStrDel(&statement);
    
    return value;
}




/* @datasection [EnsPDitagfeature] Ditag Feature ******************************
**
** Functions for manipulating Ensembl Ditag Feature objects
**
** @cc Bio::EnsEMBL::Map::DitagFeature CVS Revision: 1.11
**
** @nam2rule Ditagfeature
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditag Features. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitagfeature]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPDitagfeature] Ensembl Ditag Feature
** @argrule Ref object [EnsPDitagfeature] Ensembl Ditag Feature
**
** @valrule * [EnsPDitagfeature] Ensembl Ditag Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDitagfeatureNew ***************************************************
**
** Default constructor for an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable
** @param [r] adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Map::DitagFeature::new
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] cigar [AjPStr] CIGAR line
** @param [r] side [AjEnum] Side
** @param [r] tstart [ajint] Target start
** @param [r] tend [ajint] Target end
** @param [r] tstrand [ajint] Target strand
** @param [r] pairid [ajuint] Pair identifier
**
** @return [EnsPDitag] Ensembl Ditag or NULL
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNew(EnsPDitagfeatureadaptor adaptor,
                                    ajuint identifier,
                                    EnsPFeature feature,
                                    EnsPDitag dt,
                                    AjPStr cigar,
                                    AjEnum side,
                                    ajint tstart,
                                    ajint tend,
                                    ajint tstrand,
                                    ajuint pairid)
{
    EnsPDitagfeature dtf = NULL;
    
    if(!feature)
	return NULL;
    
    if((tstrand < -1) || (tstrand > 1))
    {
	ajDebug("ensDitagfeatureNew got target strand not -1, 0 or +1.\n");
	
	return NULL;
    }
    
    AJNEW0(dtf);
    
    dtf->Use        = 1;
    dtf->Identifier = identifier;
    dtf->Adaptor    = adaptor;
    dtf->Feature    = ensFeatureNewRef(feature);
    dtf->Ditag      = ensDitagNewRef(dt);
    
    if(cigar)
        dtf->Cigar = ajStrNewRef(cigar);
    
    dtf->Side           = side;
    dtf->TargetStart    = tstart;
    dtf->TargetEnd      = tend;
    dtf->TargetStrand   = tstrand;
    dtf->PairIdentifier = pairid;
    
    return dtf;
}




/* @func ensDitagfeatureNewObj ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitagfeature] Ensembl Ditag Feature or NULL
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNewObj(const EnsPDitagfeature object)
{
    EnsPDitagfeature dtf = NULL;
    
    if(!object)
	return NULL;
    
    AJNEW0(dtf);
    
    dtf->Use        = 1;
    dtf->Identifier = object->Identifier;
    dtf->Adaptor    = object->Adaptor;
    dtf->Feature    = ensFeatureNewRef(object->Feature);
    dtf->Ditag      = ensDitagNewRef(object->Ditag);
    
    if (object->Cigar)
        dtf->Cigar = ajStrNewRef(object->Cigar);
    
    dtf->Side           = object->Side;
    dtf->TargetStart    = object->TargetStart;
    dtf->TargetEnd      = object->TargetEnd;
    dtf->TargetStrand   = object->TargetStrand;
    dtf->PairIdentifier = object->PairIdentifier;
    
    return dtf;
}




/* @func ensDitagfeatureNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitagfeature] Ensembl Ditag Feature or NULL
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNewRef(EnsPDitagfeature dtf)
{
    if(!dtf)
	return NULL;
    
    dtf->Use++;
    
    return dtf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Ditag Feature.
**
** @fdata [EnsPDitagfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Feature object
**
** @argrule * Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDitagfeatureDel ***************************************************
**
** Default destructor for an Ensembl Ditag Feature.
**
** @param [d] Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagfeatureDel(EnsPDitagfeature *Pdtf)
{
    EnsPDitagfeature pthis = NULL;
    
    /*
     ajDebug("ensDitagfeatureDel\n"
	     "  *Pdtf %p\n",
	     *Pdtf);
     
     ensDitagfeatureTrace(*Pdtf, 1);
     */
    
    if(!Pdtf)
        return;
    
    if(!*Pdtf)
        return;

    pthis = *Pdtf;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pdtf = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    ensDitagDel(&pthis->Ditag);
    
    ajStrDel(&pthis->Cigar);
    
    AJFREE(pthis);

    *Pdtf = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
** @fnote None
**
** @nam3rule Get Return Ditag Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Ditag Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetDitag Return the Ensembl Ditag
** @nam4rule GetCigar Return the CIGAR line
** @nam4rule GetSide Return the side
** @nam4rule GetTartgetStart Return the target start
** @nam4rule GetTargetEnd Return the target end
** @nam4rule GetTargetStrand Return the target strand
** @nam4rule GetPairIdentifier Return the pair identifier
**
** @argrule * dtf [const EnsPDitagfeature] Ditag Feature
**
** @valrule Adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule Ditag [EnsPDitag] Ensembl Ditag
** @valrule Cigar [AjPStr] CIGAR line
** @valrule Side [AjEnum] Side
** @valrule TargetStart [ajint] Target start
** @valrule TargetEnd [ajint] Target end
** @valrule TargetStrand [ajint] Target strand
** @valrule PairIdentifier [ajuint] Pair identifier
**
** @fcategory use
******************************************************************************/




/* @func ensDitagfeatureGetAdaptor ********************************************
**
** Get the Ensembl Ditag Feature Adaptor element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensDitagfeatureGetAdaptor(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;
    
    return dtf->Adaptor;
}




/* @func ensDitagfeatureGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensDitagfeatureGetIdentifier(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;
    
    return dtf->Identifier;
}




/* @func ensDitagfeatureGetFeature ********************************************
**
** Get the Ensembl Feature element of an Ensembl Ditag Feature.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensDitagfeatureGetFeature(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;
    
    return dtf->Feature;
}




/* @func ensDitagfeatureGetDitag **********************************************
**
** Get the Ensembl Ditag element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitag] Ensembl Ditag
** @@
******************************************************************************/

EnsPDitag ensDitagfeatureGetDitag(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;
    
    return dtf->Ditag;
}




/* @func ensDitagfeatureGetCigar **********************************************
**
** Get the CIGAR line element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::cigar_line
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [AjPStr] CIGAR line
** @@
******************************************************************************/

AjPStr ensDitagfeatureGetCigar(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;
    
    return dtf->Cigar;
}




/* @func ensDitagfeatureGetSide ***********************************************
**
** Get the side element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_side
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [AjEnum] Side
** @@
******************************************************************************/

AjEnum ensDitagfeatureGetSide(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return ensEDitagfeatureSideNULL;
    
    return dtf->Side;
}




/* @func ensDitagfeatureGetTargetStart ****************************************
**
** Get the target start element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_start
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target start
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetStart(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;
    
    return dtf->TargetStart;
}




/* @func ensDitagfeatureGetTargetEnd ******************************************
**
** Get the target end element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_end
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target end
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetEnd(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;
    
    return dtf->TargetEnd;
}




/* @func ensDitagfeatureGetTargetStrand ***************************************
**
** Get the target strand element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_strand
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target strand
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetStrand(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;
    
    return dtf->TargetStrand;
}




/* @func ensDitagfeatureGetPairIdentifier *************************************
**
** Get the pair identifier element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_pair_id
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] Pair identifier
** @@
******************************************************************************/

ajuint ensDitagfeatureGetPairIdentifier(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;
    
    return dtf->PairIdentifier;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Ditag Feature
** @nam4rule SetAdaptor Set the Ensembl Ditag Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetDitag Set the Ensembl Ditag
** @nam4rule SetCigar Set the CIGAR line
** @nam4rule SetSide Set the side
** @nam4rule SetTargetStart Set the target start
** @nam4rule SetTargetEnd Set the target end
** @nam4rule SetTargetStrand Set the target strand
**
** @argrule * dtf [EnsPDitagfeature] Ensembl Ditag Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDitagfeatureSetAdaptor ********************************************
**
** Set the Ensembl Ditag Feature Adaptor element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetAdaptor(EnsPDitagfeature dtf,
                                 EnsPDitagfeatureadaptor adaptor)
{
    if(!dtf)
        return ajFalse;
    
    dtf->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensDitagfeatureSetIdentifier *****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetIdentifier(EnsPDitagfeature dtf, ajuint identifier)
{
    if(!dtf)
        return ajFalse;
    
    dtf->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensDitagfeatureSetFeature ********************************************
**
** Set the Ensembl Feature element of an Ensembl Ditag Feature.
**
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetFeature(EnsPDitagfeature dtf, EnsPFeature feature)
{
    if(!dtf)
        return ajFalse;
    
    ensFeatureDel(&(dtf->Feature));
    
    dtf->Feature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensDitagfeatureSetDitag **********************************************
**
** Set the Ensembl Ditag element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] dt [EnsPDitag] Ensembl Ditag
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetDitag(EnsPDitagfeature dtf, EnsPDitag dt)
{
    if(!dtf)
        return ajFalse;
    
    ensDitagDel(&(dtf->Ditag));
    
    dtf->Ditag = ensDitagNewRef(dt);
    
    return ajTrue;
}




/* @func ensDitagfeatureSetCigar **********************************************
**
** Set the CIGAR line element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::cigar_line
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] cigar [AjPStr] CIGAR line
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetCigar(EnsPDitagfeature dtf, AjPStr cigar)
{
    if(!dtf)
        return ajFalse;
    
    ajStrDel(&dtf->Cigar);
    
    dtf->Cigar = ajStrNewRef(cigar);
    
    return ajTrue;
}




/* @func ensDitagfeatureSetSide ***********************************************
**
** Set the side element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_side
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] side [AjEnum] Side
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetSide(EnsPDitagfeature dtf, AjEnum side)
{
    if(!dtf)
        return ajFalse;
    
    dtf->Side = side;
    
    return ajTrue;
}




/* @func ensDitagfeatureSetTargetStart ****************************************
**
** Set the target start element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_start
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] tstart [ajint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetTargetStart(EnsPDitagfeature dtf, ajint tstart)
{
    if(!dtf)
        return ajFalse;
    
    dtf->TargetStart = tstart;
    
    return ajTrue;
}




/* @func ensDitagfeatureSetTargetEnd ******************************************
**
** Set the target end element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_end
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] tend [ajint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetTargetEnd(EnsPDitagfeature dtf, ajint tend)
{
    if(!dtf)
        return ajFalse;
    
    dtf->TargetEnd = tend;
    
    return ajTrue;
}




/* @func ensDitagfeatureSetTargetStrand ***************************************
**
** Set the target strand element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_strand
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] tstrand [ajint] Target strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetTargetStrand(EnsPDitagfeature dtf, ajint tstrand)
{
    if(!dtf)
        return ajFalse;
    
    dtf->TargetStrand = tstrand;
    
    return ajTrue;
}




/* @func ensDitagfeatureSetPairIdentifier *************************************
**
** Set the pair identifier element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_pair_id
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] pairid [ajuint] Pair identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetPairIdentifier(EnsPDitagfeature dtf, ajuint pairid)
{
    if(!dtf)
        return ajFalse;
    
    dtf->PairIdentifier = pairid;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
** @nam3rule Trace Report Ensembl Ditag Feature elements to debug file.
**
** @argrule Trace dtf [const EnsPDitagfeature] Ensembl Ditag Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDitagfeatureTrace *************************************************
**
** Trace an Ensembl Ditag Feature.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureTrace(const EnsPDitagfeature dtf, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!dtf)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensDitagfeatureTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Feature %p\n"
	    "%S  Ditag %p\n"
	    "%S  Cigar '%S'\n"
	    "%S  Side '%s'\n"
	    "%S  TargetStart %d\n"
	    "%S  TargetEnd %d\n"
	    "%S  TargetStrand %d\n"
	    "%S  PairIdentifier %u\n",
	    indent, dtf,
	    indent, dtf->Use,
	    indent, dtf->Identifier,
	    indent, dtf->Adaptor,
	    indent, dtf->Feature,
	    indent, dtf->Ditag,
	    indent, dtf->Cigar,
	    indent, ensDitagfeatureSideToChar(dtf->Side),
	    indent, dtf->TargetStart,
	    indent, dtf->TargetEnd,
	    indent, dtf->TargetStrand,
	    indent, dtf->PairIdentifier);
    
    ensFeatureTrace(dtf->Feature, level + 1);
    
    ensDitagTrace(dtf->Ditag, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensDitagfeatureGetMemSize ********************************************
**
** Get the memory size in bytes of an Ensembl Ditag Feature.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensDitagfeatureGetMemSize(const EnsPDitagfeature dtf)
{
    ajuint size = 0;
    
    if(!dtf)
	return 0;
    
    size += (ajuint) sizeof (EnsODitagfeature);
    
    size += ensFeatureGetMemSize(dtf->Feature);
    
    size += ensDitagGetMemSize(dtf->Ditag);
    
    if(dtf->Cigar)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(dtf->Cigar);
    }
    
    return size;
}




/* @func ensDitagfeatureSideFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Ditag Feature side element.
**
** @param [r] side [const AjPStr] Ensembl Ditag Feature side string
**
** @return [AjEnum] Ensembl Ditag Feature side element or
**                  ensEDitagfeatureSideNULL
** @@
******************************************************************************/

AjEnum ensDitagfeatureSideFromStr(const AjPStr side)
{
    register ajint i = 0;
    
    AjEnum eside = ensEDitagfeatureSideNULL;
    
    for(i = 1; diTagFeatureSide[i]; i++)
	if (ajStrMatchC(side, diTagFeatureSide[i]))
	    eside = i;
    
    if(!eside)
	ajDebug("ensDitagfeatureSideFromStr encountered "
		"unexpected string '%S'.\n", side);
    
    return eside;
}




/* @func ensDitagfeatureSideToChar ********************************************
**
** Convert an Ensembl Ditag Feature side element into a C-type (char*) string.
**
** @param [r] side [const AjEnum] Ensembl Ditag Feature side enumerator
**
** @return [const char*] Ensembl Ditag Feature side C-type (char*) string
** @@
******************************************************************************/

const char* ensDitagfeatureSideToChar(const AjEnum side)
{
    register ajint i = 0;
    
    if(!side)
	return NULL;
    
    for(i = 1; diTagFeatureSide[i] && (i < side); i++);
    
    if(!diTagFeatureSide[i])
	ajDebug("ensDitagfeatureSideToChar encountered an "
		"out of boundary error on gender %d.\n", side);
    
    return diTagFeatureSide[i];
}




/* @datasection [EnsPDitagfeatureadaptor] Ditag Feature Adaptor ***************
**
** Functions for manipulating Ensembl Ditag Feature Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor CVS Revision: 1.16
**
** @nam2rule Ditagfeatureadaptor
**
******************************************************************************/

/*
** NOTE: For now, the ditag_feature and ditag tables are permanently joined to
** allow for selection of Ditag Features on the basis of a Ditag type.
*/

static const char *diTagFeatureadaptorTables[] =
{
    "ditag_feature",
    "ditag",
    (const char *) NULL
};

static const char *diTagFeatureadaptorColumns[] =
{
    "ditag_feature.ditag_feature_id",
    "ditag_feature.seq_region_id",
    "ditag_feature.seq_region_start",
    "ditag_feature.seq_region_end",
    "ditag_feature.seq_region_strand",
    "ditag_feature.analysis_id",
    "ditag_feature.ditag_id",
    "ditag_feature.hit_start",
    "ditag_feature.hit_end",
    "ditag_feature.hit_strand",
    "ditag_feature.cigar_line",
    "ditag_feature.ditag_side",
    "ditag_feature.ditag_pair_id",
    (const char *) NULL
};

static EnsOBaseadaptorLeftJoin diTagFeatureadaptorLeftJoin[] =
{
    {(const char *) NULL, (const char *) NULL}
};

static const char *diTagFeatureadaptorDefaultCondition =
"ditag_feature.ditag_id = ditag.ditag_id";

static const char *diTagFeatureadaptorFinalCondition = NULL;

/*
** TODO: Remove this?
 static const char *diTagFeatureDitagadaptorTables[] =
 {
     "ditag_feature",
     "ditag",
     (const char *) NULL
 };
 
 static const char *diTagFeatureDitagadaptorColumns[] =
 {
     "ditag_feature.ditag_feature_id",
     "ditag_feature.seq_region_id",
     "ditag_feature.seq_region_start",
     "ditag_feature.seq_region_end",
     "ditag_feature.seq_region_strand",
     "ditag_feature.analysis_id",
     "ditag_feature.ditag_id",
     "ditag_feature.hit_start",
     "ditag_feature.hit_end",
     "ditag_feature.hit_strand",
     "ditag_feature.cigar_line",
     "ditag_feature.ditag_side",
     "ditag_feature.ditag_pair_id",
     "ditag.ditag_id",
     "ditag.name",
     "ditag.type",
     "ditag.tag_count",
     "ditag.sequence",
     (const char *) NULL
 };
 
 static const char *diTagFeatureDitagadaptorDefaultCondition =
 "ditag_feature.ditag_id = ditag.ditag_id";
 
 static const char *diTagFeatureDitagadaptorFinalCondition =
 "ORDER BY ditag_feature.ditag_id, ditag_feature.ditag_pair_id";
 */




/* @funcstatic diTagFeatureadaptorFetchAllBySQL *******************************
**
** Fetch all Ensembl Ditag Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::_fetch
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool diTagFeatureadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                               const AjPStr statement,
                                               EnsPAssemblymapper am,
                                               EnsPSlice slice,
                                               AjPList dtfs)
{
    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    ajint srstrand = 0;
    ajint dtstrand = 0;
    
    ajuint identifier = 0;
    ajuint srid       = 0;
    ajuint srstart    = 0;
    ajuint srend      = 0;
    ajuint analysisid = 0;
    ajuint dtid       = 0;
    ajuint dtstart    = 0;
    ajuint dtend      = 0;
    ajuint dtpairid   = 0;
    
    AjEnum eside = ensEDitagfeatureSideNULL;
    
    AjPList mrs = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr cigar = NULL;
    AjPStr side  = NULL;
    
    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPDitag dt = NULL;
    
    EnsPDitagfeature dtf         = NULL;
    EnsPDitagfeatureadaptor dtfa = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    (void) am;
    
    (void) slice;
    
    if(!dtfs)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    dtfa = ensRegistryGetDitagfeatureadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    if(slice)
	ama = ensRegistryGetAssemblymapperadaptor(dba);
    
    mrs = ajListNew();
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier = 0;
	srid = 0;
	srstart = 0;
	srend = 0;
	srstrand = 0;
	analysisid = 0;
	dtid = 0;
	dtstart = 0;
	dtend = 0;
	dtstrand = 0;
	cigar = ajStrNew();
	side = ajStrNew();
	dtpairid = 0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToUint(sqlr, &analysisid);
	ajSqlcolumnToUint(sqlr, &dtid);
	ajSqlcolumnToUint(sqlr, &dtstart);
	ajSqlcolumnToUint(sqlr, &dtend);
	ajSqlcolumnToInt(sqlr, &dtstrand);
	ajSqlcolumnToStr(sqlr, &cigar);
	ajSqlcolumnToStr(sqlr, &side);
	ajSqlcolumnToUint(sqlr, &dtpairid);
	
	/*
	** Since the Ensembl SQL schema defines Sequence Region start and end
	** coordinates as unsigned integers for all Features, the range needs
	** checking.
	*/
	
	if(srstart <= INT_MAX)
	    slstart = (ajint) srstart;
	else
	    ajFatal("diTagFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("diTagFeatureadaptorFetchAllBySQL got a "
		    "Sequence Region end coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srend, INT_MAX);
	
	slstrand = srstrand;
	
	/* Fetch a Slice spanning the entire Sequence Region. */
	
	ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);
	
	/*
	** Increase the reference counter of the Ensembl Assembly Mapper if
	** one has been specified, otherwise fetch it from the database if a
	** destination Slice has been specified.
	*/
	
	/*
	** FIXME: The Perl API does not retrieve an Assembly Mapper if it has
	** not been passed in!
	*/
	
	if(am)
	    am = ensAssemblymapperNewRef(am);
	
	if((! am) &&
           slice &&
           (! ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
                                  ensSliceGetCoordsystem(srslice))))
	    am = ensAssemblymapperadaptorFetchByCoordsystems(
                ama,
                ensSliceGetCoordsystem(slice),
                ensSliceGetCoordsystem(srslice));
	
	/*
	** Remap the Feature coordinates to another Ensembl Coordinate System
	** if an Ensembl Assembly Mapper is defined at this point.
	*/
	
	if(am)
	{
	    ensAssemblymapperFastMap(
                am,
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
		
		ajStrDel(&cigar);
		ajStrDel(&side);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		ensMapperresultDel(&mr);
		
		continue;
	    }
	    
	    srid = ensMapperresultGetObjectIdentifier(mr);
	    
	    slstart = ensMapperresultGetStart(mr);
	    
	    slend = ensMapperresultGetEnd(mr);
	    
	    slstrand = ensMapperresultGetStrand(mr);
	    
	    /*
	    ** Delete the Sequence Region Slice and fetch a Slice in the
	    ** Coordinate System we just mapped to.
	    */
	    
	    ensSliceDel(&srslice);
	    
	    ensSliceadaptorFetchBySeqregionIdentifier(
                sa,
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
		ajFatal("diTagFeatureadaptorFetchAllBySQL got a Slice, "
			"which length (%u) exceeds the "
			"maximum integer limit (%d).",
			ensSliceGetLength(slice), INT_MAX);
	    
	    /*
	    ** Nothing needs to be done if the destination Slice starts at 1
	    ** and is on the forward strand.
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
	    ** Throw away Features off the end of the requested Slice or on
	    ** any other than the requested Slice.
	    */
	    
	    if((slend < 1) ||
		(slstart > sllength) ||
		(srid != ensSliceGetSeqregionIdentifier(slice)))
	    {
		/* Load the next Feature but destroy first! */
		
		ajStrDel(&cigar);
		ajStrDel(&side);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		continue;
	    }
	    
	    /* Delete the Sequence Region Slice and set the requested Slice. */
	    
	    ensSliceDel(&srslice);
	    
	    srslice = ensSliceNewRef(slice);
	}
	
	ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);
	
	feature = ensFeatureNewS(analysis, srslice, slstart, slend, slstrand);
	
	ensDitagadaptorFetchByIdentifier(dba, dtid, &dt);
	
	eside = ensDitagfeatureSideFromStr(side);
	
	dtf = ensDitagfeatureNew(dtfa,
				 identifier,
				 feature,
				 dt,
				 cigar,
				 eside,
				 dtstart,
				 dtend,
				 dtstrand,
				 dtpairid);
	
	ajListPushAppend(dtfs, (void *) dtf);
	
	ensDitagDel(&dt);
	
	ensFeatureDel(&feature);
	
	ensAnalysisDel(&analysis);
	
	ajStrDel(&cigar);
	ajStrDel(&side);
	
	ensSliceDel(&srslice);
	
	ensAssemblymapperDel(&am);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @funcstatic diTagFeatureadaptorCacheReference ******************************
**
** Wrapper function to reference an Ensembl Ditag Feature
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Ditag Feature
**
** @return [void *] Ensembl Ditag Feature or NULL
** @@
******************************************************************************/

static void *diTagFeatureadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensDitagfeatureNewRef((EnsPDitagfeature) value);
}




/* @funcstatic diTagFeatureadaptorCacheDelete *********************************
**
** Wrapper function to delete an Ensembl Ditag Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Ditag Feature address
**
** @return [void]
** @@
******************************************************************************/

static void diTagFeatureadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensDitagfeatureDel((EnsPDitagfeature *) value);
    
    return;
}




/* @funcstatic DitagfeatureadaptorCacheSize ***********************************
**
** Wrapper function to determine the memory size of an Ensembl Ditag Feature
** via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Ditag Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

static ajuint diTagFeatureadaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensDitagfeatureGetMemSize((const EnsPDitagfeature) value);
}




/* @funcstatic diTagFeatureadaptorGetFeature **********************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Ditag Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Ditag Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature diTagFeatureadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensDitagfeatureGetFeature((const EnsPDitagfeature) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditag Feature Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitagfeatureadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @argrule Ref object [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @valrule * [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensDitagfeatureadaptorNew ********************************************
**
** Default Ensembl Ditag Feature Adaptor constructor.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensDitagfeatureadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPDitagfeatureadaptor adaptor = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(adaptor);
    
    adaptor->Adaptor =
	ensFeatureadaptorNew(dba,
			     diTagFeatureadaptorTables,
			     diTagFeatureadaptorColumns,
			     diTagFeatureadaptorLeftJoin,
			     diTagFeatureadaptorDefaultCondition,
			     diTagFeatureadaptorFinalCondition,
			     diTagFeatureadaptorFetchAllBySQL,
			     (void* (*)(const void* key)) NULL, /* Fread */
			     diTagFeatureadaptorCacheReference,
			     (AjBool (*)(const void* value)) NULL, /* Fwrite */
			     diTagFeatureadaptorCacheDelete,
			     diTagFeatureadaptorCacheSize,
			     diTagFeatureadaptorGetFeature,
			     "Ditag Feature");
    
    return adaptor;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Ditag Feature Adaptor.
**
** @fdata [EnsPDitagfeatureadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Feature Adaptor object.
**
** @argrule * Padaptor [EnsPDitagfeatureadaptor*] Ensembl Ditag Feature
**                                                Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDitagfeatureadaptorDel ********************************************
**
** Default destructor for an Ensembl Ditag Feature Adaptor.
**
** @param [d] Padaptor [EnsPDitagfeatureadaptor*] Ensembl Ditag Feature
**                                                Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagfeatureadaptorDel(EnsPDitagfeatureadaptor *Padaptor)
{
    EnsPDitagfeatureadaptor pthis = NULL;
    
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




/* @func ensDitagfeatureadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Ditag Feature via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::fetch_by_dbID
** @param [r] adaptor [const EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllByIdentifier(const EnsPDitagfeatureadaptor adaptor,
                                           ajuint identifier,
                                           EnsPDitagfeature *Pdtf)
{
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Pdtf)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    *Pdtf = (EnsPDitagfeature) ensBaseadaptorFetchByIdentifier(ba, identifier);
    
    return ajTrue;
}




/* @func ensDitagfeatureadaptorFetchAllByDitag ********************************
**
** Fetch all Ensembl Ditag Features by an Ensembl Ditag.
**
** The caller is responsible for deleting the Ensembl Ditag Features before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::fetch_all_by_ditagID
** @param [r] adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag
**                                                    Feature Adaptor
** @param [r] ditag [const EnsPDitag] Ensembl Ditag
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllByDitag(const EnsPDitagfeatureadaptor adaptor,
                                             const EnsPDitag dt,
                                             AjPList dtfs)
{
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!dt)
	return ajFalse;
    
    if(!dtfs)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    constraint = ajFmtStr("ditag_feature.ditag_id = %u", dt->Identifier);
    
    value = ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       dtfs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDitagfeatureadaptorFetchAllByType *********************************
**
** Fetch all Ensembl Ditag Features by an Ensembl Ditag type.
**
** The caller is responsible for deleting the Ensembl Ditag Features before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::fetch_all
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::fetch_all_by_type
** @param [r] adaptor [const EnsPDitagfeatureadaptor] Ensembl Ditag
**                                                    Feature Adaptor
** @param [rN] type [const AjPStr] Ensembl Ditag type
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllByType(const EnsPDitagfeatureadaptor adaptor,
                                            const AjPStr type,
                                            AjPList dtfs)
{
    char *txttype = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!(type && ajStrGetLen(type)))
	return ajFalse;
    
    if(!dtfs)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor->Adaptor);
    
    ensBaseadaptorEscapeC(ba, &txttype, type);
    
    /*
    ** NOTE: For this query the ditag_feature and the ditag tables need to be
    ** permanently joined.
    */
    
    constraint = ajFmtStr("ditag.type = '%s'", txttype);
    
    ajCharDel(&txttype);
    
    value = ensBaseadaptorGenericFetch(ba,
				       constraint,
				       (EnsPAssemblymapper) NULL,
				       (EnsPSlice) NULL,
				       dtfs);
    
    ajStrDel(&constraint);
    
    return value;
}




/* @func ensDitagfeatureadaptorFetchAllBySlice ********************************
**
** Fetch all Ensembl Ditag Features by an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Ditag Features before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor::fetch_all_by_Slice
** @param [r] adaptor [const EnsPDitagfeatureadaptor] Ensembl Ditag
**                                                    Feature Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [rN] type [const AjPStr] Ensembl Ditag type
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllBySlice(const EnsPDitagfeatureadaptor adaptor,
                                             EnsPSlice slice,
                                             const AjPStr type,
                                             const AjPStr anname,
                                             AjPList dtfs)
{
    char *txttype = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!dtfs)
	return ajFalse;
    
    if(type && ajStrGetLen(type))
    {
	ensFeatureadaptorEscapeC(adaptor->Adaptor, &txttype, type);
	
	constraint = ajFmtStr("ditag.type = '%s'", txttype);
	
	ajCharDel(&txttype);
    }
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       dtfs);
    
    ajStrDel(&constraint);
    
    return value;
}




#if AJFALSE

AjBool ensDitagFeatureadaptorFetchPairsBySlice(const EnsPDitagfeatureadaptor adaptor,
                                               EnsPSlice slice,
                                               const AjPStr type,
                                               const AjPStr anname,
                                               AjPList dtfs)
{
    char *txttype = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!dtfs)
	return ajFalse;
    
    if(type && ajStrGetLen(type))
    {
	ensFeatureadaptorEscapeC(adaptor->Adaptor, &txttype, type);
	
	constraint = ajFmtStr("ditag.type = '%s'", txttype);
	
	ajCharDel(&txttype);
    }
    
    /*
    ** FIXME: This would be problematic in multi-threaded environments.
    **  ensFeatureadaptorSetFinalCondition(adaptor->Adaptor,
    **  <#const char * final#>)
    */
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       anname,
						       dtfs);
    
    ajStrDel(&constraint);
    
    /*
    ** FIXME: Finish this. Since it is not simply possible to append a
    ** GROUP BY df.ditag_id, df.ditag_pair_id condition, the results would
    ** need to be calculated by means of Lists and Tables here.
    ** It would be possible to append the condition via
    ** ensFeatureadaptorSetFinalCondition(adaptor->Adaptor,
    ** <#const char * final#>),
    ** but this would be problematic in a multi-threaded environment.
    ** Could a separate EnsPDitagFeaturepairadaptor be the solution?
    ** A similr prolem exists for the EnsPExonadaptor and the
    ** ExonTranscript Adaptor ...
    */
    
    return value;
}

#endif

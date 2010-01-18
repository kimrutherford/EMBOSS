/******************************************************************************
** @source Ensembl Karyotype Band functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.4 $
** @@
**
** Bio::EnsEMBL::Karyotypeband CVS Revision: 1.6
** Bio::EnsEMBL::DBSQL::Karyotypebandadaptor CVS Revision: 1.27
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

#include "enskaryotype.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPCoordsystemadaptor
ensRegistryGetCoordsystemadaptor(EnsPDatabaseadaptor dba);

extern EnsPKaryotypebandadaptor
ensRegistryGetKaryotypebandadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static AjBool karyotypeBandadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                const AjPStr statement,
                                                EnsPAssemblymapper am,
                                                EnsPSlice slice,
                                                AjPList kblist);

static void *karyotypeBandadaptorCacheReference(void *value);

static void karyotypeBandadaptorCacheDelete(void **value);

static ajuint karyotypeBandadaptorCacheSize(const void *value);

static EnsPFeature karyotypeBandadaptorGetFeature(const void *value);




/* @filesection enskaryotype **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPKaryotypeband] Karyotype Band ****************************
**
** Functions for manipulating Ensembl Karyotype Band objects
**
** @cc Bio::EnsEMBL::Karyotypeband CVS Revision: 1.7
**
** @nam2rule Karyotypeband
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Karyotype Band by pointer.
** It is the responsibility of the user to first destroy any previous
** Karyotype Band. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPKaryotypeband]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPKaryotypeband] Ensembl Karyotype Band
** @argrule Ref object [EnsPKaryotypeband] Ensembl Karyotype Band
**
** @valrule * [EnsPKaryotypeband] Ensembl Karyotype Band
**
** @fcategory new
******************************************************************************/




/* @func ensKaryotypebandNew **************************************************
**
** Default Ensembl Karyotype Band constructor.
**
** @cc Bio::EnsEMBL::Karyotypeband::new
** @param [r] identifier [ajuint]
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature.
** @param [u] name [AjPStr] Name.
** @param [u] stain [AjPStr] Stain.
**
** @return [EnsPKaryotypeband] Ensembl Karyotype Band or NULL.
** @@
******************************************************************************/

EnsPKaryotypeband ensKaryotypebandNew(ajuint identifier,
                                      EnsPFeature feature,
                                      AjPStr name,
                                      AjPStr stain)
{
    EnsPKaryotypeband kb = NULL;
    
    if(!feature)
	return NULL;
    
    AJNEW0(kb);
    
    kb->Use = 1;
    
    kb->Identifier = identifier;
    
    kb->Feature = ensFeatureNewRef(feature);
    
    if(name)
	kb->Name = ajStrNewRef(name);
    
    if(stain)
	kb->Stain = ajStrNewRef(stain);
    
    return kb;
}




/* @func ensKaryotypebandNewObj ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [EnsPKaryotypeband] Ensembl Karyotype Band or NULL.
** @@
******************************************************************************/

EnsPKaryotypeband ensKaryotypebandNewObj(const EnsPKaryotypeband object)
{
    EnsPKaryotypeband kb = NULL;
    
    if(!object)
	return NULL;
    
    AJNEW0(kb);
    
    kb->Use = 1;
    
    kb->Identifier = object->Identifier;
    
    kb->Adaptor = object->Adaptor;
    
    kb->Feature = ensFeatureNewRef(object->Feature);
    
    if(object->Name)
        kb->Name = ajStrNewRef(object->Name);
    
    if(object->Stain)
        kb->Stain = ajStrNewRef(object->Stain);
    
    return kb;
}




/* @func ensKaryotypebandNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [EnsPKaryotypeband] Ensembl Karyotype Band.
** @@
******************************************************************************/

EnsPKaryotypeband ensKaryotypebandNewRef(EnsPKaryotypeband kb)
{
    if(!kb)
	return NULL;
    
    kb->Use++;
    
    return kb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Karyotype Band.
**
** @fdata [EnsPKaryotypeband]
** @fnote None
**
** @nam3rule Del Destroy (free) an Karyotype Band object
**
** @argrule * Pkb [EnsPKaryotypeband*] Karyotype Band object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensKaryotypebandDel **************************************************
**
** Default destructor for an Ensembl Karyotype Band.
**
** @param [d] Pkb [EnsPKaryotypeband*] Ensembl Karyotypeband address.
**
** @return [void]
** @@
******************************************************************************/

void ensKaryotypebandDel(EnsPKaryotypeband *Pkb)
{
    EnsPKaryotypeband pthis = NULL;
    
    if(!Pkb)
        return;
    
    if(!*Pkb)
        return;

    pthis = *Pkb;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pkb = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    ajStrDel(&pthis->Name);
    
    ajStrDel(&pthis->Stain);
    
    AJFREE(pthis);

    *Pkb = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
** @fnote None
**
** @nam3rule Get Return Karyotype Band attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Karyotype Band Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetName Return the name
** @nam4rule GetStain Return the stain
**
** @argrule * kb [const EnsPKaryotypeband] Karyotype Band
**
** @valrule Adaptor [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Stain [AjPStr] Stain
**
** @fcategory use
******************************************************************************/




/* @func ensKaryotypebandGetAdaptor *******************************************
**
** Get the Ensembl Karyotype Band Adaptor element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor.
** @@
******************************************************************************/

EnsPKaryotypebandadaptor ensKaryotypebandGetAdaptor(const EnsPKaryotypeband kb)
{
    if(!kb)
	return NULL;
    
    return kb->Adaptor;
}




/* @func ensExonGetIdentifier *************************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [EnsPKaryotypebandadaptor] Internal database identifier.
** @@
******************************************************************************/

ajuint ensKaryotypebandGetIdentifier(const EnsPKaryotypeband kb)
{
    if(!kb)
	return 0;
    
    return kb->Identifier;
}




/* @func ensKaryotypebandGetFeature *******************************************
**
** Get the Ensembl Feature element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [EnsPFeature] Ensembl Feature.
** @@
******************************************************************************/

EnsPFeature ensKaryotypebandGetFeature(const EnsPKaryotypeband kb)
{
    if(!kb)
	return NULL;
    
    return kb->Feature;
}




/* @func ensKaryotypebandGetName **********************************************
**
** Get the name element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [AjPStr] Name.
** @@
******************************************************************************/

AjPStr ensKaryotypebandGetName(const EnsPKaryotypeband kb)
{
    if(!kb)
	return NULL;
    
    return kb->Name;
}




/* @func ensKaryotypebandGetStain *********************************************
**
** Get the stain element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [AjPStr] Stain.
** @@
******************************************************************************/

AjPStr ensKaryotypebandGetStain(const EnsPKaryotypeband kb)
{
    if(!kb)
	return NULL;
    
    return kb->Stain;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
** @fnote None
**
** @nam3rule Set Set one element of an Karyotype Band
** @nam4rule SetAdaptor Set the Ensembl Karyotype Band Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetName Set the name
** @nam4rule SetStain Set the stain
**
** @argrule * kb [EnsPKaryotypeband] Ensembl Karyotype Band object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensKaryotypebandSetAdaptor *******************************************
**
** Set the Ensembl Karyotype Band Adaptor element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band.
** @param [r] kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandSetAdaptor(EnsPKaryotypeband kb,
                                  EnsPKaryotypebandadaptor kba)
{
    if(!kb)
        return ajFalse;
    
    kb->Adaptor = kba;
    
    return ajTrue;
}




/* @func ensKaryotypebandSetIdentifier ****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band.
** @param [r] identifier [ajuint] Database identifier.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandSetIdentifier(EnsPKaryotypeband kb, ajuint identifier)
{
    if(!kb)
        return ajFalse;
    
    kb->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensKaryotypebandSetFeature *******************************************
**
** Set the Ensembl Feature element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band.
** @param [u] feature [EnsPFeature] Ensembl Feature.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandSetFeature(EnsPKaryotypeband kb, EnsPFeature feature)
{
    if(!kb)
        return ajFalse;
    
    ensFeatureDel(&kb->Feature);
    
    kb->Feature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensKaryotypebandSetName **********************************************
**
** Set the name element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band.
** @param [u] name [AjPStr] Name.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandSetName(EnsPKaryotypeband kb, AjPStr name)
{
    if(!kb)
        return ajFalse;
    
    ajStrDel(&kb->Name);
    
    kb->Name = ajStrNewRef(name);
    
    return ajTrue;
}




/* @func ensKaryotypebandSetStain *********************************************
**
** Set the stain element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band.
** @param [u] stain [AjPStr] Logic name.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandSetStain(EnsPKaryotypeband kb, AjPStr stain)
{
    if(!kb)
        return ajFalse;
    
    ajStrDel(&kb->Stain);
    
    kb->Stain = ajStrNewRef(stain);
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
** @nam3rule Trace Report Ensembl Karyotype Band elements to debug file
**
** @argrule Trace kb [const EnsPKaryotypeband] Ensembl Karyotype Band
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensKaryotypebandTrace ************************************************
**
** Trace an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
** @param [r] level [ajuint] Indentation level.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandTrace(const EnsPKaryotypeband kb, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!kb)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensKaryotypebandTrace %p\n"
	    "%S  Adaptor %p\n"
	    "%S  Identifier %u\n"
	    "%S  Feature %p\n"
	    "%S  Name '%S'\n"
	    "%S  Stain '%S'\n",
	    indent, kb,
	    indent, kb->Adaptor,
	    indent, kb->Identifier,
	    indent, kb->Feature,
	    indent, kb->Name,
	    indent, kb->Stain);
    
    ensFeatureTrace(kb->Feature, 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensKaryotypebandGetMemSize *******************************************
**
** Get the memory size in bytes of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band.
**
** @return [ajuint] Memory size.
** @@
******************************************************************************/

ajuint ensKaryotypebandGetMemSize(const EnsPKaryotypeband kb)
{
    ajuint size = 0;
    
    if(!kb)
	return 0;
    
    size += sizeof (EnsOKaryotypeband);
    
    size += ensFeatureGetMemSize(kb->Feature);
    
    if(kb->Name)
    {
	size += sizeof (AjOStr);
	
	size += ajStrGetRes(kb->Name);
    }
    
    if(kb->Stain)
    {
	size += sizeof (AjOStr);
	
	size += ajStrGetRes(kb->Stain);
    }
    
    return size;
}




/* @datasection [EnsPKaryotypebandadaptor] Karyotype Band Adaptor *************
**
** Functions for manipulating Ensembl Karyotype Band Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Karyotypebandadaptor CVS Revision: 1.30
**
** @nam2rule Karyotypebandadaptor
**
******************************************************************************/

static const char *karyotypeBandadaptorTables[] =
{
    "karyotype",
    NULL
};




static const char *karyotypeBandadaptorColumns[] =
{
    "karyotype.karyotype_id",
    "karyotype.seq_region_id",
    "karyotype.seq_region_start",
    "karyotype.seq_region_end",
    "karyotype.band",
    "karyotype.stain",
    NULL
};




static EnsOBaseadaptorLeftJoin karyotypeBandadaptorLeftJoin[] =
{
    {NULL, NULL}
};




static const char *karyotypeBandadaptorDefaultCondition = NULL;

static const char *karyotypeBandadaptorFinalCondition = NULL;




/* @funcstatic karyotypeBandadaptorFetchAllBySQL ******************************
**
** Fetch all Ensembl Karyotype Band objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] statement [const AjPStr] SQL statement.
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper.
** @param [r] slice [EnsPSlice] Ensembl Slice.
** @param [u] kblist [AjPList] AJAX List of Ensembl Karyotype Band objects.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool karyotypeBandadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                const AjPStr statement,
                                                EnsPAssemblymapper am,
                                                EnsPSlice slice,
                                                AjPList kblist)
{
    ajuint identifier = 0;
    
    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    
    AjPStr name  = NULL;
    AjPStr stain = NULL;
    
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    EnsPCoordsystemadaptor csa = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPKaryotypeband kb         = NULL;
    EnsPKaryotypebandadaptor kba = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    (void) am;
    
    (void) slice;
    
    if(!kblist)
	return ajFalse;
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    kba = ensRegistryGetKaryotypebandadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	identifier = 0;
	srid       = 0;
	srstart    = 0;
	srend      = 0;
	name       = ajStrNew();
	stain      = ajStrNew();
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &stain);
	
	/* Need to get the internal Ensembl Sequence Region identifier. */
	
	srid = ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);
	
	/*
	** FIXME: karyotype.seq_region_start and karyotype.seq_region_end are
	** defined as signed in the tables.sql file.
	** All other features use unsigned start and end coordinates.
	*/
	
	ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);
	
	feature = ensFeatureNewS((EnsPAnalysis) NULL,
				 srslice,
				 srstart,
				 srend,
				 1);
	
	kb = ensKaryotypebandNew(identifier, feature, name, stain);
	
	ajListPushAppend(kblist, (void *) kb);
	
	ensFeatureDel(&feature);
	
	ensSliceDel(&srslice);
	
	ajStrDel(&name);
	ajStrDel(&stain);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    return ajTrue;
}




/* @funcstatic karyotypeBandadaptorCacheReference *****************************
**
** Wrapper function to reference an Ensembl Karyotype Band
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Karyotype Band.
**
** @return [void *] Ensembl Karyotype Band or NULL.
** @@
******************************************************************************/

static void *karyotypeBandadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensKaryotypebandNewRef((EnsPKaryotypeband) value);
}




/* @funcstatic karyotypeBandadaptorCacheDelete ********************************
**
** Wrapper function to delete (de-reference) an Ensembl Karyotype Band
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Karyotype Band.
**
** @return [void]
** @@
******************************************************************************/

static void karyotypeBandadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensKaryotypebandDel((EnsPKaryotypeband *) value);
    
    return;
}




/* @funcstatic karyotypeBandadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Karyotype Band
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Karyotype Band.
**
** @return [ajuint] Memory size.
** @@
******************************************************************************/

static ajuint karyotypeBandadaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensKaryotypebandGetMemSize((const EnsPKaryotypeband) value);
}




/* @funcstatic karyotypeBandadaptorGetFeature *********************************
**
** Wrapper function to get the Ensembl Feature element from an
** Ensembl Karyotype Band.
**
** @param [r] value [const void*] Ensembl Karyotype Band.
**
** @return [EnsPFeature] Ensembl Feature.
** @@
******************************************************************************/

static EnsPFeature karyotypeBandadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensKaryotypebandGetFeature((const EnsPKaryotypeband) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Karyotype Band Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Karyotype Band Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPKaryotypebandadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPKaryotypebandadaptor] Ensembl Karyotype
**                                                Band Adaptor
** @argrule Ref object [EnsPKaryotypebandadaptor] Ensembl Karyotype
**                                                Band Adaptor
**
** @valrule * [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensKaryotypebandadaptorNew *******************************************
**
** Default Ensembl Karyotype Band Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
**
** @return [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor or NULL.
** @@
******************************************************************************/

EnsPKaryotypebandadaptor ensKaryotypebandadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPKaryotypebandadaptor kba = NULL;
    
    if(!dba)
	return NULL;
    
    AJNEW0(kba);
    
    kba->Adaptor =
	ensFeatureadaptorNew(dba,
			     karyotypeBandadaptorTables,
			     karyotypeBandadaptorColumns,
			     karyotypeBandadaptorLeftJoin,
			     karyotypeBandadaptorDefaultCondition,
			     karyotypeBandadaptorFinalCondition,
			     karyotypeBandadaptorFetchAllBySQL,
			     (void* (*)(const void* key)) NULL, /* Fread */
			     karyotypeBandadaptorCacheReference,
			     (AjBool (*)(const void* value)) NULL, /* Fwrite */
			     karyotypeBandadaptorCacheDelete,
			     karyotypeBandadaptorCacheSize,
			     karyotypeBandadaptorGetFeature,
			     "Karyotype Band");
    
    return kba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Karyotype Band Adaptor.
**
** @fdata [EnsPKaryotypebandadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Karyotype Band Adaptor object
**
** @argrule * Padaptor [EnsPKaryotypebandadaptor*] Ensembl Karyotype Band
**                                                 Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensKaryotypebandadaptorDel *******************************************
**
** Default destructor for an Ensembl Karyotype Band Adaptor.
**
** @param [d] Padaptor [EnsPKaryotypebandadaptor*] Ensembl Karyotype Band
**                                                 Adaptor address.
**
** @return [void]
** @@
******************************************************************************/

void ensKaryotypebandadaptorDel(EnsPKaryotypebandadaptor *Padaptor)
{
    EnsPKaryotypebandadaptor pthis = NULL;
    
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




/* @func ensKaryotypebandadaptorFetchAllByChromosomeName **********************
**
** Fetch all Ensembl Karyotype Bands via a chromosome name.
**
** @cc Bio::EnsEMBL::DBSQL::Karyotypebandadaptor::fetch_all_by_chr_name
** @param [u] adaptor [EnsPKaryotypebandadaptor] Ensembl Karyotype Band
**                                               Adaptor.
** @param [r] name [const AjPStr] Chromosome name.
** @param [u] kblist [AjPList] AJAX List of Ensembl Karyotype Bands.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandadaptorFetchAllByChromosomeName(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    AjPList kblist)
{
    AjBool value = AJFALSE;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!name)
	return ajFalse;
    
    if(!kblist)
	return ajFalse;
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    ensSliceadaptorFetchByRegion(sa,
				 (const AjPStr) NULL,
				 (const AjPStr) NULL,
				 name,
				 0,
				 0,
				 0,
				 &slice);
    
    value = ensFeatureadaptorFetchAllBySlice(adaptor->Adaptor,
					     slice,
					     (const AjPStr) NULL,
					     kblist);
    
    ensSliceDel(&slice);
    
    return value;
}




/* @func ensKaryotypebandadaptorFetchAllByChromosomeBand **********************
**
** Fetch all Ensembl Karyotype Bands via a chromosome and band name.
**
** This function uses fuzzy matching of the band name.
** For example the bands 'q23.1' and 'q23.4' could be matched by 'q23'.
**
** @cc Bio::EnsEMBL::DBSQL::Karyotypebandadaptor::fetch_all_by_chr_band
** @param [u] adaptor [EnsPKaryotypebandadaptor] Ensembl Karyotype Band
**                                               Adaptor.
** @param [r] name [const AjPStr] Chromosome name.
** @param [r] band [const AjPStr] Karyotype Band name.
** @param [u] kblist [AjPList] AJAX List of Ensembl Karyotype Bands.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensKaryotypebandadaptorFetchAllByChromosomeBand(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    const AjPStr band,
    AjPList kblist)
{
    char *txtband = NULL;
    
    AjBool value = AJFALSE;
    
    AjPStr constraint = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!name)
	return ajFalse;
    
    if(!band)
	return ajFalse;
    
    if(!kblist)
	return ajFalse;
    
    dba = ensFeatureadaptorGetDatabaseadaptor(adaptor->Adaptor);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    ensSliceadaptorFetchByRegion(sa,
				 (const AjPStr) NULL,
				 (const AjPStr) NULL,
				 name,
				 0,
				 0,
				 0,
				 &slice);
    
    ensDatabaseadaptorEscapeC(dba, &txtband, band);
    
    constraint = ajFmtStr("karyotype.band LIKE '%s%%'", txtband);
    
    ajCharDel(&txtband);
    
    value = ensFeatureadaptorFetchAllBySliceConstraint(adaptor->Adaptor,
						       slice,
						       constraint,
						       (const AjPStr) NULL,
						       kblist);
    
    ajStrDel(&constraint);
    
    ensSliceDel(&slice);
    
    return value;
}

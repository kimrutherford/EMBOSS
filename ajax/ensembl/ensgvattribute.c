/* @source Ensembl Genetic Variation Attribute functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/08 11:40:49 $ by $Author: mks $
** @version $Revision: 1.2 $
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

#include "ensattribute.h"
#include "ensgvattribute.h"
#include "enstable.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic gvattributeadaptorTables **************************************
**
** Array of Ensembl Genetic Variation Attribute Adaptor SQL table names
**
******************************************************************************/

static const char* const gvattributeadaptorTables[] =
{
    "attrib",
    (const char*) NULL
};




/* @conststatic gvattributeadaptorColumns *************************************
**
** Array of Ensembl Genetic Variation Attribute Adaptor SQL column names
**
******************************************************************************/

static const char* const gvattributeadaptorColumns[] =
{
    "attrib.attrib_id",
    "attrib.attrib_type_id",
    "attrib.value",
    (const char*) NULL
};




/* @conststatic gvattributetypeCode *******************************************
**
** Ensembl Attribute Type code elements are enumerated for attribute sets in
** the Ensembl Genetic Variation module.
**
******************************************************************************/

static const char* const gvattributetypeCode[] =
{
    "SO_accession",
    "SO_term",
    "display_term",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool gvattributeadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvas);

static AjBool gvattributeadaptorSetsClear(EnsPGvattributeadaptor gvaa);

static AjBool gvattributeadaptorSetsInit(EnsPGvattributeadaptor gvaa);

static AjBool gvattributeadaptorCacheInit(EnsPGvattributeadaptor gvaa);

static AjBool gvattributeadaptorCacheInsert(EnsPGvattributeadaptor gvaa,
                                            EnsPGvattribute* Pgva);

static void gvattributeadaptorCacheClearIdentifier(void** key,
                                                   void** value,
                                                   void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensgvattribute ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvattribute] Ensembl Genetic Variation Attribute *********
**
** @nam2rule Gvattribute Functions for manipulating
** Ensembl Genetic Variation Attribute objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor
** @cc CVS Revision: 1.6
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Attribute by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Attribute.
** The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvattribute]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gva [const EnsPGvattribute]
** Ensembl Genetic Variation Attribute
** @argrule Ini gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini at [EnsPAttributetype] Ensembl Attribute Type
** @argrule Ini value [AjPStr] Value
** @argrule Ref gva [EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @valrule * [EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvattributeNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

EnsPGvattribute ensGvattributeNewCpy(const EnsPGvattribute gva)
{
    EnsPGvattribute pthis = NULL;

    AJNEW0(pthis);

    pthis->Use           = 1;
    pthis->Identifier    = gva->Identifier;
    pthis->Adaptor       = gva->Adaptor;
    pthis->Attributetype = ensAttributetypeNewRef(gva->Attributetype);

    if(gva->Value)
        pthis->Value = ajStrNewRef(gva->Value);

    return pthis;
}




/* @func ensGvattributeNewIni *************************************************
**
** Ensembl Genetic Variation Attribute constructor with initial values.
**
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] value [AjPStr] Value
**
** @return [EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

EnsPGvattribute ensGvattributeNewIni(EnsPGvattributeadaptor gvaa,
                                     ajuint identifier,
                                     EnsPAttributetype at,
                                     AjPStr value)
{
    EnsPGvattribute gva = NULL;

    AJNEW0(gva);

    gva->Use           = 1;
    gva->Identifier    = identifier;
    gva->Adaptor       = gvaa;
    gva->Attributetype = ensAttributetypeNewRef(at);

    if(value)
        gva->Value = ajStrNewRef(value);

    return gva;
}




/* @func ensGvattributeNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gva [EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

EnsPGvattribute ensGvattributeNewRef(EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    gva->Use++;

    return gva;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Attribute object.
**
** @fdata [EnsPGvattribute]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Attribute object
**
** @argrule * Pgva [EnsPGvattribute*]
** Ensembl Genetic Variation Attribute object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvattributeDel ****************************************************
**
** Default destructor for an Ensembl Genetic Variation Attribute.
**
** @param [d] Pgva [EnsPGvattribute*]
** Ensembl Genetic Variation Attribute object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvattributeDel(EnsPGvattribute* Pgva)
{
    EnsPGvattribute pthis = NULL;

    if(!Pgva)
        return;

    if(!*Pgva)
        return;

    pthis = *Pgva;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgva = NULL;

        return;
    }

    ensAttributetypeDel(&pthis->Attributetype);

    ajStrDel(&pthis->Value);

    AJFREE(pthis);

    *Pgva = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Attribute object.
**
** @fdata [EnsPGvattribute]
**
** @nam3rule Get Return Ensembl Genetic Variation Attribute attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Attribute Adaptor
** @nam4rule Attributetype Return the Ensembl Attribute Type
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Value Return the value
**
** @argrule * gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @valrule Adaptor [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor or NULL
** @valrule Attributetype [EnsPAttributetype]
** Ensembl Attribute Type or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Value [AjPStr] Value or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvattributeGetAdaptor *********************************************
**
** Get the Ensembl Genetic Variation Attribute Adaptor element of an
** Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor or NULL
** @@
******************************************************************************/

EnsPGvattributeadaptor ensGvattributeGetAdaptor(const EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    return gva->Adaptor;
}




/* @func ensGvattributeGetAttributetype ***************************************
**
** Get the Ensembl Attribute Type element of an
** Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Attribute::code
** @cc Bio::EnsEMBL::Attribute::description
** @cc Bio::EnsEMBL::Attribute::name
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [EnsPAttributetype] Ensembl Attribute Type or NULL
** @@
******************************************************************************/

EnsPAttributetype ensGvattributeGetAttributetype(const EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    return gva->Attributetype;
}




/* @func ensGvattributeGetIdentifier ******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvattributeGetIdentifier(const EnsPGvattribute gva)
{
    if(!gva)
        return 0;

    return gva->Identifier;
}




/* @func ensGvattributeGetValue ***********************************************
**
** Get the value element of an Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Attribute::value
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [AjPStr] Value or NULL
** @@
******************************************************************************/

AjPStr ensGvattributeGetValue(const EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    return gva->Value;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Attribute object.
**
** @fdata [EnsPGvattribute]
**
** @nam3rule Trace Report Ensembl Genetic Variation Attribute elements
**                 to debug file
**
** @argrule Trace gva [const EnsPGvattribute]
** Ensembl Genetic Variation Attribute
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvattributeTrace **************************************************
**
** Trace an Ensembl Genetic Variation Attribute.
**
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvattributeTrace(const EnsPGvattribute gva, ajuint level)
{
    AjPStr indent = NULL;

    if(!gva)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvattributeTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Identifier %u\n"
            "%S  Attributetype %p\n"
            "%S  Value '%S'\n"
            "%S  Use %u\n",
            indent, gva,
            indent, gva->Adaptor,
            indent, gva->Identifier,
            indent, gva->Attributetype,
            indent, gva->Value,
            indent, gva->Use);

    ensAttributetypeTrace(gva->Attributetype, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Attribute object.
**
** @fdata [EnsPGvattribute]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Attribute values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvattributeCalculateMemsize ***************************************
**
** Calclate the memory size in bytes of an
** Ensembl Genetic Variation Attribute.
**
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensGvattributeCalculateMemsize(const EnsPGvattribute gva)
{
    size_t size = 0;

    if(!gva)
        return 0;

    size += sizeof (EnsOGvattribute);

    size += ensAttributetypeCalculateMemsize(gva->Attributetype);

    if(gva->Value)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gva->Value);
    }

    return size;
}




/* @section convenience functions *********************************************
**
** Ensembl Genetic Variation Attribute convenience functions
**
** @fdata [EnsPGvattribute]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Code Get the code element of the Ensembl Attribute Type
** linked to an Ensembl Genetic Variation Attribute
** @nam4rule Description Get the description element of an
** Ensembl Attribute Type linked to an Ensembl Genetic Variation Attribute
** @nam4rule Name Get the name element of an Ensembl Attribute Type
** linked to an Ensembl Genetic Variation Attribute
**
** @argrule * gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @valrule Code [AjPStr] Code or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvattributeGetCode ************************************************
**
** Get the code element of an Ensembl Attribute Type
** linked to an Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Attribute::code
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [AjPStr] Code or NULL
** @@
******************************************************************************/

AjPStr ensGvattributeGetCode(const EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    return ensAttributetypeGetCode(gva->Attributetype);
}




/* @func ensGvattributeGetDescription *****************************************
**
** Get the description element of an Ensembl Attribute Type
** linked to an Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Attribute::description
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensGvattributeGetDescription(const EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    return ensAttributetypeGetDescription(gva->Attributetype);
}




/* @func ensGvattributeGetName ************************************************
**
** Get the name element of an Ensembl Attribute Type
** linked to an Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Attribute::name
** @param [r] gva [const EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensGvattributeGetName(const EnsPGvattribute gva)
{
    if(!gva)
        return NULL;

    return ensAttributetypeGetName(gva->Attributetype);
}




/* @datasection [EnsPGvattributeadaptor] Ensembl Genetic Variation
** Attribute Adaptor
**
** @nam2rule Gvattributeadaptor Functions for manipulating
** Ensembl Genetic Variation Attribute Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor
** @cc CVS Revision: 1.6
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic gvattributeadaptorFetchAllbyStatement **************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Genetic Variation Attribute objects.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Attribute objects before
** deleting the AJAX List.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvas [AjPList]
** AJAX List of Ensembl Genetic Variation Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvattributeadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvas)
{
    ajuint atid       = 0;
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr value       = NULL;

    EnsPGvattribute        gva  = NULL;
    EnsPGvattributeadaptor gvaa = NULL;

    EnsPAttributetype        at  = NULL;
    EnsPAttributetypeadaptor ata = NULL;

    if(ajDebugTest("gvattributeadaptorFetchAllbyStatement"))
        ajDebug("gvattributeadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvas %p\n",
                dba,
                statement,
                am,
                slice,
                gvas);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvas)
        return ajFalse;

    ata = ensRegistryGetAttributetypeadaptor(dba);

    gvaa = ensRegistryGetGvattributeadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        atid       = 0;
        value      = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &atid);
        ajSqlcolumnToStr(sqlr, &value);

        ensAttributetypeadaptorFetchByIdentifier(ata, atid, &at);

        gva = ensGvattributeNewIni(gvaa, identifier, at, value);

        ajListPushAppend(gvas, (void*) gva);

        ensAttributetypeDel(&at);

        ajStrDel(&value);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic gvattributeadaptorSetsClear ************************************
**
** Clear the Ensembl Attribute cache by sets of an
** Ensembl Genetic Variation Attribute Adaptor.
**
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvattributeadaptorSetsClear(EnsPGvattributeadaptor gvaa)
{
    register ajuint i  = 0;
    register ajuint j  = 0;
    register ajuint l1 = 0;
    register ajuint l2 = 0;

    AjPVoid array = NULL;

    EnsPGvattribute gva = NULL;

    if(!gvaa)
        return ajFalse;

    if(!gvaa->CacheBySet)
        return ajTrue;

    l1 = ajVoidLen(gvaa->CacheBySet);

    for(i = 0; i < l1; i++)
    {
        array = (AjPVoid) ajVoidGet(gvaa->CacheBySet, i);

        if(!array)
            continue;

        l2 = ajVoidLen(array);

        for(j = 0; j < l2; j++)
        {
            gva = (EnsPGvattribute) ajVoidGet(array, j);

            ensGvattributeDel(&gva);
        }

        ajVoidDel(&array);
    }

    ajVoidDel(&gvaa->CacheBySet);

    return ajTrue;
}




/* @funcstatic gvattributeadaptorSetsInit *************************************
**
** Initialise the Ensembl Attribute cache by sets of an
** Ensembl Genetic Variation Attribute Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::_fetch_sets_by_type
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvattributeadaptorSetsInit(EnsPGvattributeadaptor gvaa)
{
    ajuint asid = 0;
    ajuint atid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPVoid array = NULL;

    EnsEGvattributetypeCode gvatc = ensEGvattributetypeCodeNULL;

    EnsPGvattribute gva = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvaa)
        return ajFalse;

    if(gvaa->CacheBySet)
        return ajFalse;
    else
        gvaa->CacheBySet = ajVoidNew();

    dba = ensBaseadaptorGetDatabaseadaptor(gvaa->Adaptor);

    statement = ajStrNewC(
        "SELECT "
        "attrib_set.attrib_set_id, "
        "attrib_set.attrib_id "
        "FROM "
        "attrib_set ");

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        asid = 0;
        atid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &asid);
        ajSqlcolumnToUint(sqlr, &atid);

        ensGvattributeadaptorFetchByIdentifier(gvaa, atid, &gva);

        if(!gva)
            continue;

        gvatc = ensGvattributetypeCodeFromStr(ensGvattributeGetCode(gva));

        if(!gvatc)
            continue;

        /*
        ** First dimension is the
        ** Ensembl genetic Variation Attribute Set identifier.
        */

        array = ajVoidGet(gvaa->CacheBySet, asid);

        if(!array)
            array = ajVoidNew();

        ajVoidPut(&gvaa->CacheBySet, asid, (void*) array);

        /*
        ** Second dimension is the
        ** Ensembl Genetic Variation Attribute Type Code enumeration.
        */

        ajVoidPut(&array, gvatc, (void*) ensGvattributeNewRef(gva));

        ensGvattributeDel(&gva);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Attribute Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Attribute Adaptor.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPGvattributeadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvattributeadaptorNew *********************************************
**
** Default constructor for an Ensembl Genetic Variation Attribute Adaptor.
**
** Ensembl Object Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Object Adaptor connected to a particular database is
** sufficient to instantiate any number of Ensembl Objects from the database.
** Each Ensembl Object will have a weak reference to the Object Adaptor that
** instantiated it. Therefore, Ensembl Object Adaptors should not be
** instantiated directly, but rather obtained from the Ensembl Registry,
** which will in turn call this function if neccessary.
**
** @see ensRegistryGetDatabaseadaptor
** @see ensRegistryGetGvattributeadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor or NULL
** @@
******************************************************************************/

EnsPGvattributeadaptor ensGvattributeadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPGvattributeadaptor gvaa = NULL;

    if(!dba)
        return NULL;

    if(ajDebugTest("ensGvattributeadaptorNew"))
        ajDebug("ensGvattributeadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(gvaa);

    gvaa->Adaptor = ensBaseadaptorNew(
        dba,
        gvattributeadaptorTables,
        gvattributeadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        gvattributeadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** gvattributeadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint,
    ** which calls gvattributeadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetGvattributeadaptor. At that point, however,
    ** the Ensembl Genetic Variation Attribute Adaptor has not been stored in
    ** the Registry. Therefore, each ensGvattributeadaptorFetch function has
    ** to test the presence of the adaptor-internal cache and eventually
    ** initialise before accessing it.
    **
    **  gvattributeadaptorCacheInit(gvaa);
    */

    return gvaa;
}




/* @section cache *************************************************************
**
** Functions for maintaining the
** Ensembl Genetic Variation Attribute Adaptor-internal cache of
** Ensembl Attribute objects.
**
** @fdata [EnsPGvattributeadaptor]
**
** @nam3rule Cache
** Process an Ensembl Genetic Variation Attribute Adaptor-internal cache
** @nam4rule Clear
** Clear the Ensembl Genetic Variation Attribute Adaptor-internal cache
**
** @argrule * gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @valrule * [AjBool] True on success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic gvattributeadaptorCacheInit ************************************
**
** Initialise the internal Ensembl Attribute cache of an
** Ensembl Genetic Variation Attribute Adaptor.
**
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvattributeadaptorCacheInit(EnsPGvattributeadaptor gvaa)
{
    AjPList gvas = NULL;

    EnsPGvattribute gva = NULL;

    if(ajDebugTest("gvattributeadaptorCacheInit"))
        ajDebug("gvattributeadaptorCacheInit\n"
                "  gvaa %p\n",
                gvaa);

    if(!gvaa)
        return ajFalse;

    if(gvaa->CacheByIdentifier)
        return ajFalse;
    else
        gvaa->CacheByIdentifier = ensTableuintNewLen(0);

    gvas = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvaa->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvas);

    while(ajListPop(gvas, (void**) &gva))
    {
        gvattributeadaptorCacheInsert(gvaa, &gva);

        /*
        ** Both caches hold internal references to the
        ** Ensembl Analysis objects.
        */

        ensGvattributeDel(&gva);
    }

    ajListFree(&gvas);

    return ajTrue;
}




/* @funcstatic gvattributeadaptorCacheInsert **********************************
**
** Insert an Ensembl Attribute into the Ensembl Genetic Variation Attribute
** Adaptor-internal cache.
** If an Ensembl Attribute with the same identifier element is already present
** in the Ensembl Genetic Variation Attribute Adaptor-internal cache, the
** Ensembl Attribute is deleted and a pointer to the cached
** Ensembl Attribute is returned.
**
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [u] Pgva [EnsPGvattribute*]
** Ensembl Genetic Variation Attribute address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvattributeadaptorCacheInsert(EnsPGvattributeadaptor gvaa,
                                            EnsPGvattribute* Pgva)
{
    ajuint* Pidentifier = NULL;

    EnsPGvattribute gva = NULL;

    if(!gvaa)
        return ajFalse;

    if(!gvaa->CacheByIdentifier)
        return ajFalse;

    if(!Pgva)
        return ajFalse;

    if(!*Pgva)
        return ajFalse;

    /* Search the identifer cache. */

    gva = (EnsPGvattribute) ajTableFetchmodV(
        gvaa->CacheByIdentifier,
        (const void*) &((*Pgva)->Identifier));

    if(!gva)
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pgva)->Identifier;

        ajTablePut(gvaa->CacheByIdentifier,
                   (void*) Pidentifier,
                   (void*) ensGvattributeNewRef(*Pgva));
    }

    ajDebug("gvattributeadaptoradaptorCacheInsert replaced "
            "Ensembl Genetic Variation Attribute %p with one "
            "already cached %p.\n",
            *Pgva, gva);

    ensGvattributeDel(Pgva);

    ensGvattributeNewRef(gva);

    Pgva = &gva;

    return ajTrue;
}




#if AJFALSE
/* @funcstatic gvattributeadaptorCacheRemove **********************************
**
** Remove an Ensembl Attribute from the
** Ensembl Genetic Variation Attribute Adaptor-internal cache.
**
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [u] gva [EnsPGvattribute] Ensembl Genetic Variation Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvattributeadaptorCacheRemove(EnsPGvattributeadaptor gvaa,
                                            EnsPGvattribute gva)
{
    ajuint* Pidentifier = NULL;

    if(!gvaa)
        return ajFalse;

    if(!gva)
        return ajFalse;

    /* Remove the table nodes. */

    gva = (EnsPGvattribute)
        ajTableRemoveKey(gvaa->CacheByIdentifier,
                         (const void*) &gva->Identifier,
                         (void**) &Pidentifier);

    AJFREE(Pidentifier);

    ensAttributeDel(&gva);

    return ajTrue;
}

#endif




/* @funcstatic gvattributeadaptorCacheClearIdentifier *************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Genetic Variation
** Attribute Adaptor-internal Ensembl Attribute cache.
** This function deletes the AJAX unsigned integer (identifier) key and
** the Ensembl Attribute value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Attribute value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvattributeadaptorCacheClearIdentifier(void** key,
                                                   void** value,
                                                   void* cl)
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

    ensGvattributeDel((EnsPGvattribute*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensGvattributeadaptorCacheClear **************************************
**
** Clear the Ensembl Genetic Variation Attribute Adaptor-internal cache of
** Ensembl Attribute objects.
**
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvattributeadaptorCacheClear(EnsPGvattributeadaptor gvaa)
{
    if(!gvaa)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(gvaa->CacheByIdentifier,
                  gvattributeadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&gvaa->CacheByIdentifier);

    /* Clear the attribute sets cache. */

    gvattributeadaptorSetsClear(gvaa);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Attribute Adaptor object.
**
** @fdata [EnsPGvattributeadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Attribute Adaptor object.
**
** @argrule * Pgvaa [EnsPGvattributeadaptor*]
** Ensembl Genetic Variation Attribute Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvattributeadaptorDel *********************************************
**
** Default destructor for an Ensembl Genetic Variation Attribute Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvaa [EnsPGvattributeadaptor*]
** Ensembl Genetic Variation Attribute Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvattributeadaptorDel(EnsPGvattributeadaptor* Pgvaa)
{
    EnsPGvattributeadaptor pthis = NULL;

    if(!Pgvaa)
        return;

    if(!*Pgvaa)
        return;

    pthis = *Pgvaa;

    ensGvattributeadaptorCacheClear(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pgvaa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Attribute Adaptor object.
**
** @fdata [EnsPGvattributeadaptor]
**
** @nam3rule Get
** Return Ensembl Genetic Variation Attribute Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * gvaa [const EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvattributeadaptorGetBaseadaptor **********************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Attribute Adaptor.
**
** @param [r] gvaa [const EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensGvattributeadaptorGetBaseadaptor(
    const EnsPGvattributeadaptor gvaa)
{
    if(!gvaa)
        return NULL;

    return gvaa->Adaptor;
}




/* @func ensGvattributeadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Genetic Variation Attribute Adaptor.
**
** @param [r] gvaa [const EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvattributeadaptorGetDatabaseadaptor(
    const EnsPGvattributeadaptor gvaa)
{
    if(!gvaa)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(gvaa->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Attribute objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGvattributeadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Attribute object(s)
** @nam4rule All Fetch all Ensembl Genetic Variation Attribute objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Attribute objects
** matching a criterion
** @nam5rule Code Fetch all Ensembl Genetic Variation Attribute objects by an
** Ensembl Attribute Type code and optionally,
** Ensembl Genetic Variation Attribute value
** @nam4rule By Fetch one Ensembl Genetic Variation Attribute object
** matching a criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
**
** @argrule * gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @argrule All gvas [AjPList]
** AJAX List of Ensembl Genetic Variation Attribute objects
** @argrule Code code [const AjPStr] Ensembl Attribute Type code
** @argrule Code value [const AjPStr] Ensembl Genetic Variation Attribute value
** @argrule Allby gvas [AjPList]
** AJAX List of Ensembl Genetic Variation Attribute objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pgva [EnsPGvattribute*]
** Ensembl Genetic Variation Attribute address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvattributeadaptorFetchAllbyCode **********************************
**
** Fetch all Ensembl Genetic Variation Attribute objects by their
** Ensembl Attribute Type code and
** optionally, Ensembl Genetic Variation Attribute value.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::
** attrib_id_for_type_value
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] code [const AjPStr] Ensembl Attribute Type code
** @param [rN] value [const AjPStr] Ensembl Genetic Variation Attribute value
** @param [u] gvas [AjPList]
** AJAX List of Ensembl Genetic Variation Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvattributeadaptorFetchAllbyCode(EnsPGvattributeadaptor gvaa,
                                           const AjPStr code,
                                           const AjPStr value,
                                           AjPList gvas)
{
    void** valarray = NULL;

    register ajuint i = 0;

    EnsPGvattribute gva = NULL;

    if(!gvaa)
        return ajFalse;

    if((code == NULL) || (ajStrGetLen(code) == 0))
        return ajFalse;

    if(!gvas)
        return ajFalse;

    if(!gvaa->CacheByIdentifier)
        gvattributeadaptorCacheInit(gvaa);

    ajTableToarrayValues(gvaa->CacheByIdentifier, &valarray);

    for(i = 0; valarray[i]; i++)
    {
        gva = (EnsPGvattribute) valarray[i];

        if(!ajStrMatchS(ensGvattributeGetCode(gva), code))
            continue;

        if((value != NULL) && (ajStrGetLen(value) > 0)
           &&
           (!ajStrMatchS(ensGvattributeGetValue(gva), value)))
            continue;

        ajListPushAppend(gvas, (void*) ensGvattributeNewRef(gva));
    }

    AJFREE(valarray);

    return ajTrue;
}




/* @func ensGvattributeadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Genetic Variation Attribute by its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Attribute.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::attrib_value_for_id
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgva [EnsPGvattribute*]
** Ensembl Genetic Variation Attribute address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvattributeadaptorFetchByIdentifier(EnsPGvattributeadaptor gvaa,
                                              ajuint identifier,
                                              EnsPGvattribute* Pgva)
{
    AjPList gvas = NULL;

    AjPStr constraint = NULL;

    EnsPGvattribute gva = NULL;

    if(!gvaa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgva)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!gvaa->CacheByIdentifier)
        gvattributeadaptorCacheInit(gvaa);

    *Pgva = (EnsPGvattribute) ajTableFetchmodV(gvaa->CacheByIdentifier,
                                               (const void*) &identifier);

    if(*Pgva)
    {
        ensGvattributeNewRef(*Pgva);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    constraint = ajFmtStr("attrib.attrib_id = %u", identifier);

    gvas = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvaa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvas);

    if(ajListGetLength(gvas) > 1)
        ajWarn("ensGvattributeadaptorFetchByIdentifier got more than one "
               "Ensembl Attribute for (PRIMARY KEY) identifier %u.\n",
               identifier);

    ajListPop(gvas, (void**) Pgva);

    gvattributeadaptorCacheInsert(gvaa, Pgva);

    while(ajListPop(gvas, (void**) &gva))
    {
        gvattributeadaptorCacheInsert(gvaa, &gva);

        ensGvattributeDel(&gva);
    }

    ajListFree(&gvas);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @section object cast  ******************************************************
**
** Functions for casting Ensembl Genetic Variation Attribute objects.
**
** @fdata [EnsPGvattributeadaptor]
**
** @nam3rule Get Get Ensembl Genetic Variation Attribute object(s)
** @nam4rule Displayterm Get a display term
** @nam4rule Soaccession Get a Sequence Ontology accession number
** @nam4rule Soterm Get a Sequence Ontology term
** @nam5rule Fromsoaccession Get an Ensembl Genetic Variation Attribute object
** matching a Sequence Ontology accession number
** @nam5rule Fromsoterm Get an Ensembl Genetic Variation Attribute object
** matching a Sequence Ontology term
**
** @argrule * gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @argrule Fromsoaccession soaccession [const AjPStr]
** Sequence Ontology accession number
** @argrule Fromsoterm soterm [const AjPStr]
** Sequence Ontology term
**
** @valrule * [const EnsPGvattribute]
** Ensembl Genetic Variation Attribute or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvattributeadaptorGetDisplaytermFromsoaccession *******************
**
** Get an Ensembl Genetic Variation Attribute representing a display term by a
** Sequence Ontology accession number.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::
** display_term_for_SO_accession
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] soaccession [const AjPStr] Sequence Ontology accession number
**
** @return [const EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

const EnsPGvattribute ensGvattributeadaptorGetDisplaytermFromsoaccession(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soaccession)
{
    register ajuint i = 0;

    AjPVoid array = NULL;

    EnsPGvattribute qgva = NULL;
    EnsPGvattribute tgva = NULL;

    if(!gvaa)
        return NULL;

    if(!gvaa->CacheBySet)
        gvattributeadaptorSetsInit(gvaa);

    for(i = 0; (array = ajVoidGet(gvaa->CacheBySet, i)); i++)
    {
        qgva = (EnsPGvattribute) ajVoidGet(
            array,
            (ajuint) ensEGvattributetypeCodeSoaccession);

        if(ajStrMatchS(ensGvattributeGetValue(qgva), soaccession))
        {
            tgva = (EnsPGvattribute) ajVoidGet(
                array,
                (ajuint) ensEGvattributetypeCodeDisplayterm);

            break;
        }
    }

    return tgva;
}




/* @func ensGvattributeadaptorGetDisplaytermFromsoterm ************************
**
** Get an Ensembl Genetic Variation Attribute representing a display term
** by a Sequence Ontology term.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::
** display_term_for_SO_term
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] soterm [const AjPStr] Sequence Ontology term
**
** @return [const EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

const EnsPGvattribute ensGvattributeadaptorGetDisplaytermFromsoterm(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soterm)
{
    register ajuint i = 0;

    AjPVoid array = NULL;

    EnsPGvattribute qgva = NULL;
    EnsPGvattribute tgva = NULL;

    if(!gvaa)
        return NULL;

    if(!gvaa->CacheBySet)
        gvattributeadaptorSetsInit(gvaa);

    for(i = 0; (array = ajVoidGet(gvaa->CacheBySet, i)); i++)
    {
        qgva = (EnsPGvattribute) ajVoidGet(
            array,
            (ajuint) ensEGvattributetypeCodeSoterm);

        if(ajStrMatchS(ensGvattributeGetValue(qgva), soterm))
        {
            tgva = (EnsPGvattribute) ajVoidGet(
                array,
                (ajuint) ensEGvattributetypeCodeDisplayterm);

            break;
        }
    }

    return tgva;
}




/* @func ensGvattributeadaptorGetSoaccessionFromsoterm ************************
**
** Get an Ensembl Genetic Variation Attribute representing a
** Sequence Ontology accession number
** by a Sequence Ontology term.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::
** SO_accession_for_SO_term
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] soterm [const AjPStr] Sequence Ontology term
**
** @return [const EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

const EnsPGvattribute ensGvattributeadaptorGetSoaccessionFromsoterm(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soterm)
{
    register ajuint i = 0;

    AjPVoid array = NULL;

    EnsPGvattribute qgva = NULL;
    EnsPGvattribute tgva = NULL;

    if(!gvaa)
        return NULL;

    if(!gvaa->CacheBySet)
        gvattributeadaptorSetsInit(gvaa);

    for(i = 0; (array = ajVoidGet(gvaa->CacheBySet, i)); i++)
    {
        qgva = (EnsPGvattribute) ajVoidGet(
            array,
            (ajuint) ensEGvattributetypeCodeSoterm);

        if(ajStrMatchS(ensGvattributeGetValue(qgva), soterm))
        {
            tgva = (EnsPGvattribute) ajVoidGet(
                array,
                (ajuint) ensEGvattributetypeCodeSoaccession);

            break;
        }
    }

    return tgva;
}




/* @func ensGvattributeadaptorGetSotermFromsoaccession ************************
**
** Get an Ensembl Genetic Variation Attribute representing a
** Sequence Ontology term by a Sequence Ontology accession number.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor::
** SO_term_for_SO_accession
** @param [u] gvaa [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @param [r] soaccession [const AjPStr] Sequence Ontology accession number
**
** @return [const EnsPGvattribute] Ensembl Genetic Variation Attribute or NULL
** @@
******************************************************************************/

const EnsPGvattribute ensGvattributeadaptorGetSotermFromsoaccession(
    EnsPGvattributeadaptor gvaa,
    const AjPStr soaccession)
{
    register ajuint i = 0;

    AjPVoid array = NULL;

    EnsPGvattribute qgva = NULL;
    EnsPGvattribute tgva = NULL;

    if(!gvaa)
        return NULL;

    if(!gvaa->CacheBySet)
        gvattributeadaptorSetsInit(gvaa);

    for(i = 0; (array = ajVoidGet(gvaa->CacheBySet, i)); i++)
    {
        qgva = (EnsPGvattribute) ajVoidGet(
            array,
            (ajuint) ensEGvattributetypeCodeSoaccession);

        if(ajStrMatchS(ensGvattributeGetValue(qgva), soaccession))
        {
            tgva = (EnsPGvattribute) ajVoidGet(
                array,
                (ajuint) ensEGvattributetypeCodeSoterm);

            break;
        }
    }

    return tgva;
}




/* @datasection [EnsEGvattributetypeCode] Ensembl Genetic Variation Attribute
** Type Code
**
** @nam2rule Gvattributetype Functions for manipulating
** Ensembl Genetic Variation Attribute Type objects
** @nam3rule GvattributetypeCode Functions for manipulating
** Ensembl Genetic Variation Attribute Type Code enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an
** Ensembl Genetic Variation Attribute Type Code enumeration.
**
** @fdata [EnsEGvattributetypeCode]
**
** @nam4rule From Ensembl Genetic Variation Attribute Type Code query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  code  [const AjPStr] Code string
**
** @valrule * [EnsEGvattributetypeCode]
** Ensembl Genetic Variation Attribute Type Code enumeration or
** ensEGvattributetypeCodeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvattributetypeCodeFromStr ****************************************
**
** Return an Ensembl Genetic Variation Attribute Type Code enumeration
** from an AJAX String.
**
** @param [r] code [const AjPStr] Code string
**
** @return [EnsEGvattributetypeCode]
** Ensembl Genetic Variation Attribute Type Code or ensEGvattributetypeCodeNULL
** @@
******************************************************************************/

EnsEGvattributetypeCode ensGvattributetypeCodeFromStr(const AjPStr code)
{
    register EnsEGvattributetypeCode i = ensEGvattributetypeCodeNULL;

    EnsEGvattributetypeCode gvatc = ensEGvattributetypeCodeNULL;

    for(i = ensEGvattributetypeCodeNULL
            ;
        gvattributetypeCode[i];
        i++)
        if(ajStrMatchC(code, gvattributetypeCode[i]))
            gvatc = i;

    if(!gvatc)
        ajDebug("ensTranscriptStatusFromStr encountered "
                "unexpected string '%S'.\n", code);

    return gvatc;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Attribute Type Code enumeration.
**
** @fdata [EnsEGvattributetypeCode]
**
** @nam4rule To
** Return Ensembl Genetic Variation Attribute Type Code enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvatc [EnsEGvattributetypeCode]
** Ensembl Genetic Variation Attribute Type Code enumeration
**
** @valrule Char [const char*] Code
**
** @fcategory cast
******************************************************************************/




/* @func ensGvattributetypeCodeToChar *****************************************
**
** Cast an Ensembl Genetic Variation Attribute Type Code enumeration into a
** C-type (char*) string.
**
** @param [u] gvatc [EnsEGvattributetypeCode]
** Ensembl Genetic Variation Attribute Type Code enumeration
**
** @return [const char*] Ensembl Genetic Variation Attribute Type code
** C-type (char*) string
** @@
******************************************************************************/

const char* ensGvattributetypeCodeToChar(EnsEGvattributetypeCode gvatc)
{
    register EnsEGvattributetypeCode i = ensEGvattributetypeCodeNULL;

    for(i = ensEGvattributetypeCodeNULL;
        gvattributetypeCode[i] && (i < gvatc);
        i++);

    if(!gvattributetypeCode[i])
        ajDebug("ensGvattributetypeCodeToChar encountered an "
                "out of boundary error on Ensembl Genetic Variation "
                "Attribute Type Code enumeration %d.\n",
                gvatc);

    return gvattributetypeCode[i];
}

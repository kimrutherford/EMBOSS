/* @source ensattribute *******************************************************
**
** Ensembl Attribute functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.40 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensattribute.h"
#include "ensgene.h"
#include "enstable.h"
#include "enstranscript.h"
#include "enstranslation.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @conststatic attributetypeadaptorKTables ***********************************
**
** Array of Ensembl Attribute Type Adaptor SQL table names
**
******************************************************************************/

static const char *const attributetypeadaptorKTables[] =
{
    "attrib_type",
    (const char *) NULL
};




/* @conststatic attributetypeadaptorKColumns **********************************
**
** Array of Ensembl Attribute Type Adaptor SQL column names
**
******************************************************************************/

static const char *const attributetypeadaptorKColumns[] =
{
    "attrib_type.attrib_type_id",
    "attrib_type.code",
    "attrib_type.description",
    "attrib_type.name",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool attributeadaptorFetchAllbyStatement(
    EnsPAttributeadaptor aa,
    const AjPStr statement,
    AjPList attributes);

static AjBool attributetypeadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ats);

static AjBool attributetypeadaptorCacheInit(EnsPAttributetypeadaptor ata);

static AjBool attributetypeadaptorCacheInsert(EnsPAttributetypeadaptor ata,
                                              EnsPAttributetype *Pat);

static void attributetypeadaptorFetchAll(const void *key,
                                         void **Pvalue,
                                         void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensattribute **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPAttribute] Ensembl Attribute *****************************
**
** @nam2rule Attribute Functions for manipulating Ensembl Attribute objects
**
** @cc Bio::EnsEMBL::Attribute
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Attribute by pointer.
** It is the responsibility of the user to first destroy any previous
** Attribute. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAttribute]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy attribute [const EnsPAttribute] Ensembl Attribute
** @argrule Ini at [EnsPAttributetype] Ensembl Attribute Type
** @argrule Ini value [AjPStr] Value
** @argrule Ref attribute [EnsPAttribute] Ensembl Attribute
**
** @valrule * [EnsPAttribute] Ensembl Attribute or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAttributeNewCpy ***************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttribute ensAttributeNewCpy(const EnsPAttribute attribute)
{
    EnsPAttribute pthis = NULL;

    AJNEW0(pthis);

    pthis->Attributetype = ensAttributetypeNewRef(attribute->Attributetype);

    if (attribute->Value)
        pthis->Value = ajStrNewRef(attribute->Value);

    pthis->Use = 1U;

    return pthis;
}




/* @func ensAttributeNewIni ***************************************************
**
** Ensembl Attribute constructor with initial values.
**
** @cc Bio::EnsEMBL::Attribute::new
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] value [AjPStr] Value
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttribute ensAttributeNewIni(EnsPAttributetype at,
                                 AjPStr value)
{
    EnsPAttribute attribute = NULL;

    AJNEW0(attribute);

    attribute->Attributetype = ensAttributetypeNewRef(at);

    if (value)
        attribute->Value = ajStrNewRef(value);

    attribute->Use = 1U;

    return attribute;
}




/* @func ensAttributeNewRef ***************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAttribute ensAttributeNewRef(EnsPAttribute attribute)
{
    if (!attribute)
        return NULL;

    attribute->Use++;

    return attribute;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Attribute object.
**
** @fdata [EnsPAttribute]
**
** @nam3rule Del Destroy (free) an Ensembl Attribute
**
** @argrule * Pattribute [EnsPAttribute*] Ensembl Attribute address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAttributeDel ******************************************************
**
** Default destructor for an Ensembl Attribute.
**
** @param [d] Pattribute [EnsPAttribute*] Ensembl Attribute address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAttributeDel(EnsPAttribute *Pattribute)
{
    EnsPAttribute pthis = NULL;

    if (!Pattribute)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAttributeDel"))
    {
        ajDebug("ensAttributeDel\n"
                "  *Pattribute %p\n",
                *Pattribute);

        ensAttributeTrace(*Pattribute, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pattribute)
        return;

    pthis = *Pattribute;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pattribute = NULL;

        return;
    }

    ensAttributetypeDel(&pthis->Attributetype);

    ajStrDel(&pthis->Value);

    AJFREE(pthis);

    *Pattribute = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Attribute object.
**
** @fdata [EnsPAttribute]
**
** @nam3rule Get Return Ensembl Attribute attribute(s)
** @nam4rule Attributetype Return the Ensembl Attribute Type
** @nam4rule Value Return the value
**
** @argrule * attribute [const EnsPAttribute] Ensembl Attribute
**
** @valrule Attributetype [EnsPAttributetype] Ensembl Attribute Type or NULL
** @valrule Value [AjPStr] Value or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAttributeGetAttributetype *****************************************
**
** Get the Ensembl Attribute Type member of an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::code
** @cc Bio::EnsEMBL::Attribute::description
** @cc Bio::EnsEMBL::Attribute::name
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [EnsPAttributetype] Ensembl Attribute Type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttributetype ensAttributeGetAttributetype(const EnsPAttribute attribute)
{
    return (attribute) ? attribute->Attributetype : NULL;
}




/* @func ensAttributeGetValue *************************************************
**
** Get the value member of an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::value
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Value or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAttributeGetValue(const EnsPAttribute attribute)
{
    return (attribute) ? attribute->Value : NULL;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Attribute object.
**
** @fdata [EnsPAttribute]
**
** @nam3rule Trace Report Ensembl Attribute members to debug file
**
** @argrule Trace attribute [const EnsPAttribute] Ensembl Attribute
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAttributeTrace ****************************************************
**
** Trace an Ensembl Attribute.
**
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAttributeTrace(const EnsPAttribute attribute, ajuint level)
{
    AjPStr indent = NULL;

    if (!attribute)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensAttributeTrace %p\n"
            "%S  Attributetype %p\n"
            "%S  Value '%S'\n"
            "%S  Use %u\n",
            indent, attribute,
            indent, attribute->Attributetype,
            indent, attribute->Value,
            indent, attribute->Use);

    ensAttributetypeTrace(attribute->Attributetype, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Attribute object.
**
** @fdata [EnsPAttribute]
**
** @nam3rule Calculate Calculate Ensembl Attribute values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * attribute [const EnsPAttribute] Ensembl Attribute
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensAttributeCalculateMemsize *****************************************
**
** Calclate the memory size in bytes of an Ensembl Attribute.
**
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensAttributeCalculateMemsize(const EnsPAttribute attribute)
{
    size_t size = 0;

    if (!attribute)
        return 0;

    size += sizeof (EnsOAttribute);

    size += ensAttributetypeCalculateMemsize(attribute->Attributetype);

    if (attribute->Value)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(attribute->Value);
    }

    return size;
}




/* @section convenience functions *********************************************
**
** Ensembl Attribute convenience functions
**
** @fdata [EnsPAttribute]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Code Get the code member of the Ensembl Attribute Type
** linked to an Ensembl Attribute
** @nam4rule Description Get the description member of an
** Ensembl Attribute Type linked to an Ensembl Attribute
** @nam4rule Name Get the name member of an Ensembl Attribute Type
** linked to an Ensembl Attribute
**
** @argrule * attribute [const EnsPAttribute] Ensembl Attribute
**
** @valrule Code [AjPStr] Code or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAttributeGetCode **************************************************
**
** Get the code member of an Ensembl Attribute Type
** linked to an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::code
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Code or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAttributeGetCode(const EnsPAttribute attribute)
{
    return (attribute) ?
        ensAttributetypeGetCode(attribute->Attributetype) : NULL;
}




/* @func ensAttributeGetDescription *******************************************
**
** Get the description member of an Ensembl Attribute Type
** linked to an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::description
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAttributeGetDescription(const EnsPAttribute attribute)
{
    return (attribute) ?
        ensAttributetypeGetDescription(attribute->Attributetype) : NULL;
}




/* @func ensAttributeGetName **************************************************
**
** Get the name member of an Ensembl Attribute Type
** linked to an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::name
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAttributeGetName(const EnsPAttribute attribute)
{
    return (attribute) ?
        ensAttributetypeGetName(attribute->Attributetype) : NULL;
}




/* @datasection [EnsPAttributeadaptor] Ensembl Attribute Adaptor **************
**
** @nam2rule Attributeadaptor Functions for manipulating
** Ensembl Attribute Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor
** @cc CVS Revision: 1.29
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic attributeadaptorFetchAllbyStatement ****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Attribute objects.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @param [u] aa [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool attributeadaptorFetchAllbyStatement(
    EnsPAttributeadaptor aa,
    const AjPStr statement,
    AjPList attributes)
{
    ajuint atid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr value       = NULL;

    EnsPAttribute attribute = NULL;

    EnsPAttributetype        at  = NULL;
    EnsPAttributetypeadaptor ata = NULL;

    if (!aa)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    ata = ensRegistryGetAttributetypeadaptor(aa);

    sqls = ensDatabaseadaptorSqlstatementNew(aa, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        atid  = 0;
        value = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &atid);
        ajSqlcolumnToStr(sqlr, &value);

        ensAttributetypeadaptorFetchByIdentifier(ata, atid, &at);

        attribute = ensAttributeNewIni(at, value);

        ajListPushAppend(attributes, (void *) attribute);

        ensAttributetypeDel(&at);

        ajStrDel(&value);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(aa, &sqls);

    return ajTrue;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Attribute Adaptor object.
**
** @fdata [EnsPAttributeadaptor]
**
** @nam3rule Get Return Ensembl Attribute Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensAttributeadaptorGetDatabaseadaptor ********************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Attribute Adaptor.
** The Ensembl Attribute Adaptor is just an alias for the
** Ensembl Database Adaptor.
**
** @param [u] ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAttributeadaptorGetDatabaseadaptor(
    EnsPAttributeadaptor ata)
{
    if (!ata)
        return NULL;

    return ata;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Attribute objects from an
** Ensembl SQL database.
**
** @fdata [EnsPAttributeadaptor]
**
** @nam3rule Fetch Fetch Ensembl Attribute object(s)
** @nam4rule All   Fetch all Ensembl Attribute objects
** @nam4rule Allby Fetch all Ensembl Attribute objects matching criteria
** @nam5rule Gene  Fetch all by an Ensembl Gene
** @nam5rule Seqregion   Fetch all by an Ensembl Sequence Region
** @nam5rule Slice       Fetch all by an Ensembl Slice
** @nam5rule Transcript  Fetch all by an Ensembl Transcript
** @nam5rule Translation Fetch all by an Ensembl Translation
** @nam4rule By    Fetch one Ensembl Attribute object matching criteria
**
** @argrule * ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @argrule AllbyGene gene [const EnsPGene] Ensembl Gene
** @argrule AllbyGene code [const AjPStr] Ensembl Attribute code
** @argrule AllbyGene attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllbySeqregion sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule AllbySeqregion code [const AjPStr] Ensembl Attribute code
** @argrule AllbySeqregion attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllbySlice slice [const EnsPSlice] Ensembl Slice
** @argrule AllbySlice code [const AjPStr] Ensembl Attribute code
** @argrule AllbySlice attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllbyTranscript transcript [const EnsPTranscript]
** Ensembl Transcript
** @argrule AllbyTranscript code [const AjPStr] Ensembl Attribute code
** @argrule AllbyTranscript attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllbyTranslation translation [const EnsPTranslation]
** Ensembl Translation
** @argrule AllbyTranslation code [const AjPStr] Ensembl Attribute code
** @argrule AllbyTranslation attributes [AjPList]
** AJAX List of Ensembl Attribute objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensAttributeadaptorFetchAllbyGene ************************************
**
** Fetch all Ensembl Attribute objects via an Ensembl Gene.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::fetch_all_by_
** @param [u] ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllbyGene(
    EnsPAttributeadaptor ata,
    const EnsPGene gene,
    const AjPStr code,
    AjPList attributes)
{
    char *txtcode = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ata)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "gene_attrib.attrib_type_id, "
        "gene_attrib.value "
        "FROM "
        "attrib_type, "
        "gene_attrib "
        "WHERE "
        "attrib_type.attrib_type_id = "
        "gene_attrib.attrib_type_id "
        "AND "
        "gene_attrib.gene_id = %u",
        ensGeneGetIdentifier(gene));

    if (code && ajStrGetLen(code))
    {
        dba = ensAttributeadaptorGetDatabaseadaptor(ata);

        ensDatabaseadaptorEscapeC(dba, &txtcode, code);

        ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);

        ajCharDel(&txtcode);
    }

    attributeadaptorFetchAllbyStatement(ata, statement, attributes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAttributeadaptorFetchAllbySeqregion *******************************
**
** Fetch all Ensembl Attribute objects via an Ensembl Sequence Region.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::fetch_all_by_
** @param [u] ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllbySeqregion(
    EnsPAttributeadaptor ata,
    const EnsPSeqregion sr,
    const AjPStr code,
    AjPList attributes)
{
    char *txtcode = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ata)
        return ajFalse;

    if (!sr)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "seq_region_attrib.attrib_type_id, "
        "seq_region_attrib.value "
        "FROM "
        "attrib_type, "
        "seq_region_attrib "
        "WHERE "
        "attrib_type.attrib_type_id = "
        "seq_region_attrib.attrib_type_id "
        "AND "
        "seq_region_attrib.seq_region_id = %u",
        ensSeqregionGetIdentifier(sr));

    if (code && ajStrGetLen(code))
    {
        dba = ensAttributeadaptorGetDatabaseadaptor(ata);

        ensDatabaseadaptorEscapeC(dba, &txtcode, code);

        ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);

        ajCharDel(&txtcode);
    }

    attributeadaptorFetchAllbyStatement(ata, statement, attributes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAttributeadaptorFetchAllbySlice ***********************************
**
** Fetch all Ensembl Attribute objects via an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::fetch_all_by_
** @param [u] ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllbySlice(
    EnsPAttributeadaptor ata,
    const EnsPSlice slice,
    const AjPStr code,
    AjPList attributes)
{
    EnsPSeqregion sr = NULL;

    if (!ata)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    sr = ensSliceGetSeqregion(slice);

    if (!sr)
    {
        ajDebug("ensAttributeadaptorFetchAllbySlice cannot get "
                "Ensembl Attribute objects for an Ensembl Slice without an "
                "Ensembl Sequence Region.\n");

        return ajFalse;
    }

    return ensAttributeadaptorFetchAllbySeqregion(ata,
                                                  sr,
                                                  code,
                                                  attributes);
}




/* @func ensAttributeadaptorFetchAllbyTranscript ******************************
**
** Fetch all Ensembl Attribute objects via an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::fetch_all_by_
** @param [u] ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllbyTranscript(
    EnsPAttributeadaptor ata,
    const EnsPTranscript transcript,
    const AjPStr code,
    AjPList attributes)
{
    char *txtcode = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ata)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "transcript_attrib.attrib_type_id, "
        "transcript_attrib.value "
        "FROM "
        "attrib_type, "
        "transcript_attrib "
        "WHERE "
        "attrib_type.attrib_type_id = "
        "transcript_attrib.attrib_type_id "
        "AND "
        "transcript_attrib.transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

    if (code && ajStrGetLen(code))
    {
        dba = ensAttributeadaptorGetDatabaseadaptor(ata);

        ensDatabaseadaptorEscapeC(dba, &txtcode, code);

        ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);

        ajCharDel(&txtcode);
    }

    attributeadaptorFetchAllbyStatement(ata, statement, attributes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAttributeadaptorFetchAllbyTranslation *****************************
**
** Fetch all Ensembl Attribute objects via an Ensembl Translation.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor::fetch_all_by_
** @param [u] ata [EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllbyTranslation(
    EnsPAttributeadaptor ata,
    const EnsPTranslation translation,
    const AjPStr code,
    AjPList attributes)
{
    char *txtcode = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ata)
        return ajFalse;

    if (!translation)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "translation_attrib.attrib_type_id, "
        "translation_attrib.value "
        "FROM "
        "attrib_type, "
        "translation_attrib "
        "WHERE "
        "attrib_type.attrib_type_id = "
        "translation_attrib.attrib_type_id "
        "AND "
        "translation_attrib.translation_id = %u",
        ensTranslationGetIdentifier(translation));

    if (code && ajStrGetLen(code))
    {
        dba = ensAttributeadaptorGetDatabaseadaptor(ata);

        ensDatabaseadaptorEscapeC(dba, &txtcode, code);

        ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);

        ajCharDel(&txtcode);
    }

    attributeadaptorFetchAllbyStatement(ata, statement, attributes);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPAttributetype] Ensembl Attribute Type ********************
**
** @nam2rule Attributetype Functions for manipulating
** Ensembl Attribute Type objects
**
** @cc Bio::EnsEMBL::Attribute
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Attribute Type by pointer.
** It is the responsibility of the user to first destroy any previous
** Attribute Type. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAttributetype]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy at [const EnsPAttributetype] Ensembl Attribute Type
** @argrule Ini ata [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini code [AjPStr] Code
** @argrule Ini name [AjPStr] Name
** @argrule Ini description [AjPStr] Description
** @argrule Ref at [EnsPAttributetype] Ensembl Attribute Type
**
** @valrule * [EnsPAttributetype] Ensembl Attribute Type or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAttributetypeNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [EnsPAttributetype] Ensembl Attribute Type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttributetype ensAttributetypeNewCpy(const EnsPAttributetype at)
{
    EnsPAttributetype pthis = NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = at->Identifier;
    pthis->Adaptor    = at->Adaptor;

    if (at->Code)
        pthis->Code = ajStrNewRef(at->Code);

    if (at->Name)
        pthis->Name = ajStrNewRef(at->Name);

    if (at->Description)
        pthis->Description = ajStrNewRef(at->Description);

    return pthis;
}




/* @func ensAttributetypeNewIni ***********************************************
**
** Ensembl Attribute Type constructor with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::AttributeType::new
** @param [u] code [AjPStr] Code
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
**
** @return [EnsPAttributetype] Ensembl Attribute Type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttributetype ensAttributetypeNewIni(EnsPAttributetypeadaptor ata,
                                         ajuint identifier,
                                         AjPStr code,
                                         AjPStr name,
                                         AjPStr description)
{
    EnsPAttributetype at = NULL;

    AJNEW0(at);

    at->Use        = 1U;
    at->Adaptor    = ata;
    at->Identifier = identifier;

    if (code)
        at->Code = ajStrNewRef(code);

    if (name)
        at->Name = ajStrNewRef(name);

    if (description)
        at->Description = ajStrNewRef(description);

    return at;
}




/* @func ensAttributetypeNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
**
** @return [EnsPAttributetype] Ensembl Attribute Type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttributetype ensAttributetypeNewRef(EnsPAttributetype at)
{
    if (!at)
        return NULL;

    at->Use++;

    return at;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Attribute Type object.
**
** @fdata [EnsPAttributetype]
**
** @nam3rule Del Destroy (free) an Ensembl Attribute Type
**
** @argrule * Pat [EnsPAttributetype*] Ensembl Attribute Type address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAttributetypeDel **************************************************
**
** Default destructor for an Ensembl Attribute Type.
**
** @param [d] Pat [EnsPAttributetype*] Ensembl Attribute Type address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensAttributetypeDel(EnsPAttributetype *Pat)
{
    EnsPAttributetype pthis = NULL;

    if (!Pat)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAttributetypeDel"))
    {
        ajDebug("ensAttributetypeDel\n"
                "  *Pat %p\n",
                *Pat);

        ensAttributetypeTrace(*Pat, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pat)
        return;

    pthis = *Pat;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pat = NULL;

        return;
    }

    ajStrDel(&pthis->Code);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pat = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Attribute Type object.
**
** @fdata [EnsPAttributetype]
**
** @nam3rule Get Return Ensembl Attribute Type attribute(s)
** @nam4rule Adaptor Return the Ensembl Attribute Type Adaptor
** @nam4rule Code Return the code
** @nam4rule Description Return the description
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
**
** @argrule * at [const EnsPAttributetype] Ensembl Attribute Type
**
** @valrule Adaptor [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor or NULL
** @valrule Code [AjPStr] Code or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAttributetypeGetAdaptor *******************************************
**
** Get the Ensembl Attribute Type Adaptor member of an
** Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttributetypeadaptor ensAttributetypeGetAdaptor(
    const EnsPAttributetype at)
{
    return (at) ? at->Adaptor : NULL;
}




/* @func ensAttributetypeGetCode **********************************************
**
** Get the code member of an Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Attribute::code
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [AjPStr] Code or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensAttributetypeGetCode(
    const EnsPAttributetype at)
{
    return (at) ? at->Code : NULL;
}




/* @func ensAttributetypeGetDescription ***************************************
**
** Get the description member of an Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Attribute::description
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [AjPStr] Description or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensAttributetypeGetDescription(
    const EnsPAttributetype at)
{
    return (at) ? at->Description : NULL;
}




/* @func ensAttributetypeGetIdentifier ****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAttributetypeGetIdentifier(
    const EnsPAttributetype at)
{
    return (at) ? at->Identifier : 0U;
}




/* @func ensAttributetypeGetName **********************************************
**
** Get the name member of an Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Attribute::name
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [AjPStr] Name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensAttributetypeGetName(
    const EnsPAttributetype at)
{
    return (at) ? at->Name : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Attribute Type object.
**
** @fdata [EnsPAttributetype]
**
** @nam3rule Set Set one member of an Ensembl Attribute Type
** @nam4rule Adaptor Set the Ensembl Attribute Type Adaptor
** @nam4rule Code Set the code
** @nam4rule Description Set the description
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
**
** @argrule * at [EnsPAttributetype] Ensembl Attribute Type object
** @argrule Adaptor ata [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor
** @argrule Code code [AjPStr] Code
** @argrule Description description [AjPStr] Description
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensAttributetypeSetAdaptor *******************************************
**
** Set the Ensembl Attribute Type Adaptor member of an
** Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeSetAdaptor(EnsPAttributetype at,
                                  EnsPAttributetypeadaptor ata)
{
    if (!at)
        return ajFalse;

    at->Adaptor = ata;

    return ajTrue;
}




/* @func ensAttributetypeSetCode **********************************************
**
** Set the code member of an Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Attribute::code
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] code [AjPStr] Code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeSetCode(EnsPAttributetype at,
                               AjPStr code)
{
    if (!at)
        return ajFalse;

    ajStrDel(&at->Code);

    at->Code = ajStrNewRef(code);

    return ajTrue;
}




/* @func ensAttributetypeSetDescription ***************************************
**
** Set the description member of an Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Attribute::description
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeSetDescription(EnsPAttributetype at,
                                      AjPStr description)
{
    if (!at)
        return ajFalse;

    ajStrDel(&at->Description);

    at->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensAttributetypeSetIdentifier ****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeSetIdentifier(EnsPAttributetype at,
                                     ajuint identifier)
{
    if (!at)
        return ajFalse;

    at->Identifier = identifier;

    return ajTrue;
}




/* @func ensAttributetypeSetName **********************************************
**
** Set the name member of an Ensembl Attribute Type.
**
** @cc Bio::EnsEMBL::Attribute::name
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeSetName(EnsPAttributetype at,
                               AjPStr name)
{
    if (!at)
        return ajFalse;

    ajStrDel(&at->Name);

    at->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Attribute Type object.
**
** @fdata [EnsPAttributetype]
**
** @nam3rule Trace Report Ensembl Attribute Type members to debug file
**
** @argrule Trace at [const EnsPAttributetype] Ensembl Attribute Type
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAttributetypeTrace ************************************************
**
** Trace an Ensembl Attribute Type.
**
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeTrace(const EnsPAttributetype at, ajuint level)
{
    AjPStr indent = NULL;

    if (!at)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensAttributetypeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Code '%S'\n"
            "%S  Name '%S'\n"
            "%S  Description '%S'\n"
            "%S  Value '%S'\n",
            indent, at,
            indent, at->Use,
            indent, at->Identifier,
            indent, at->Adaptor,
            indent, at->Code,
            indent, at->Name,
            indent, at->Description);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Attribute Type object.
**
** @fdata [EnsPAttributetype]
**
** @nam3rule Calculate Calculate Ensembl Attribute Type values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * at [const EnsPAttributetype] Ensembl Attribute Type
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensAttributetypeCalculateMemsize *************************************
**
** Calclate the memory size in bytes of an Ensembl Attribute Type.
**
** @param [r] at [const EnsPAttributetype] Ensembl Attribute Type
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensAttributetypeCalculateMemsize(const EnsPAttributetype at)
{
    size_t size = 0;

    if (!at)
        return 0;

    size += sizeof (EnsOAttributetype);

    if (at->Code)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(at->Code);
    }

    if (at->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(at->Name);
    }

    if (at->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(at->Description);
    }

    return size;
}




/* @datasection [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor *****
**
** @nam2rule Attributetypeadaptor Functions for manipulating
** Ensembl Attribute Type Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AttributeAdaptor
** @cc CVS Revision: 1.29
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic attributetypeadaptorFetchAllbyStatement ************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Attribute Type objects.
**
** The caller is responsible for deleting the Ensembl Attribute Type objects
** before deleting the AJAX List.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] ats [AjPList] AJAX List of Ensembl Attribute Type objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool attributetypeadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ats)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr code        = NULL;
    AjPStr name        = NULL;
    AjPStr description = NULL;

    EnsPAttributetype        at  = NULL;
    EnsPAttributetypeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (ajDebugTest("attributetypeadaptorFetchAllbyStatement"))
        ajDebug("attributetypeadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  ats %p\n",
                ba,
                statement,
                am,
                slice,
                ats);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!ats)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ata = ensRegistryGetAttributetypeadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        code        = ajStrNew();
        name        = ajStrNew();
        description = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &code);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &description);

        at = ensAttributetypeNewIni(ata, identifier, code, name, description);

        ajListPushAppend(ats, (void *) at);

        ajStrDel(&code);
        ajStrDel(&name);
        ajStrDel(&description);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Attribute Type Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Attribute Type Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAttributetypeadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPAttributetypeadaptor]
** Ensembl Attribute Type Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAttributetypeadaptorNew *******************************************
**
** Default constructor for an Ensembl Attribute Type Adaptor.
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
** @see ensRegistryGetAttributetypeadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttributetypeadaptor ensAttributetypeadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAttributetypeadaptor ata = NULL;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensAttributetypeadaptorNew"))
        ajDebug("ensAttributetypeadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(ata);

    ata->Adaptor = ensBaseadaptorNew(
        dba,
        attributetypeadaptorKTables,
        attributetypeadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &attributetypeadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** attributetypeadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint,
    ** which calls attributetypeadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetAttributetypeadaptor. At that point, however, the
    ** Ensembl Attribute Type Adaptor has not been stored in the Registry.
    ** Therefore, each ensAttributetypeadaptorFetch function has to test the
    ** presence of the adaptor-internal cache and eventually initialise before
    ** accessing it.
    **
    **  attributetypeadaptorCacheInit(ata);
    */

    return ata;
}




/* @section cache *************************************************************
**
** Functions for maintaining the Ensembl Attribute Type Adaptor-internal
** cache of Ensembl Attribute Type objects.
**
** @fdata [EnsPAttributetypeadaptor]
**
** @nam3rule Cache Process an Ensembl Attribute Type Adaptor-internal cache
** @nam4rule Clear Clear the Ensembl Attribute Type Adaptor-internal cache
**
** @argrule * ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @valrule * [AjBool] True on success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic attributetypeadaptorCacheInit **********************************
**
** Initialise the internal Attribute Type cache of an
** Ensembl Attribute Type Adaptor.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool attributetypeadaptorCacheInit(EnsPAttributetypeadaptor ata)
{
    AjPList ats = NULL;

    EnsPAttributetype at = NULL;

    if (ajDebugTest("attributetypeadaptorCacheInit"))
        ajDebug("attributetypeadaptorCacheInit\n"
                "  ata %p\n",
                ata);

    if (!ata)
        return ajFalse;

    if (ata->CacheByIdentifier)
        return ajFalse;
    else
    {
        ata->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            ata->CacheByIdentifier,
            (void (*)(void **)) &ensAttributetypeDel);
    }

    if (ata->CacheByCode)
        return ajFalse;
    else
    {
        ata->CacheByCode = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            ata->CacheByCode,
            (void (*)(void **)) &ensAttributetypeDel);
    }

    ats = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ata->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       ats);

    while (ajListPop(ats, (void **) &at))
    {
        attributetypeadaptorCacheInsert(ata, &at);

        /*
        ** Both caches hold internal references to the
        ** Ensembl attribute Type objects.
        */

        ensAttributetypeDel(&at);
    }

    ajListFree(&ats);

    return ajTrue;
}




/* @funcstatic attributetypeadaptorCacheInsert ********************************
**
** Insert an Ensembl Attribute Type into the
** Ensembl Attribute Type Adaptor-internal cache.
** If an Ensembl Attribute Type with the same code member is already present
** in the adaptor cache, the Ensembl Attribute Type is deleted and a pointer
** to the cached Ensembl Attribute Type is returned.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @param [u] Pat [EnsPAttributetype*] Ensembl Attribute Type address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool attributetypeadaptorCacheInsert(EnsPAttributetypeadaptor ata,
                                              EnsPAttributetype *Pat)
{
    ajuint *Pidentifier = NULL;

    EnsPAttributetype at1 = NULL;
    EnsPAttributetype at2 = NULL;

    if (!ata)
        return ajFalse;

    if (!ata->CacheByIdentifier)
        return ajFalse;

    if (!ata->CacheByCode)
        return ajFalse;

    if (!Pat)
        return ajFalse;

    if (!*Pat)
        return ajFalse;

    /* Search the identifer cache. */

    at1 = (EnsPAttributetype) ajTableFetchmodV(
        ata->CacheByIdentifier,
        (const void *) &((*Pat)->Identifier));

    /* Search the code cache. */

    at2 = (EnsPAttributetype) ajTableFetchmodS(
        ata->CacheByCode,
        (*Pat)->Code);

    if ((!at1) && (!at2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pat)->Identifier;

        ajTablePut(ata->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensAttributetypeNewRef(*Pat));

        /* Insert into the code cache. */

        ajTablePut(ata->CacheByCode,
                   (void *) ajStrNewS((*Pat)->Code),
                   (void *) ensAttributetypeNewRef(*Pat));
    }

    if (at1 && at2 && (at1 == at2))
    {
        ajDebug("attributetypeadaptorCacheInsert replaced "
                "Ensembl Attribute Type %p with "
                "one already cached %p.\n",
                *Pat, at1);

        ensAttributetypeDel(Pat);

        ensAttributetypeNewRef(at1);

        Pat = &at1;
    }

    if (at1 && at2 && (at1 != at2))
        ajDebug("attributetypeadaptorCacheInsert detected "
                "Ensembl Attribute Type objects in the "
                "identifier and code cache with identical codes "
                "('%S' and '%S') but different addresses (%p and %p).\n",
                at1->Code, at2->Code, at1, at2);

    if (at1 && (!at2))
        ajDebug("attributetypeadaptorCacheInsert detected an "
                "Ensembl Attribute Type object in the identifier, "
                "but not in the code cache.\n");

    if ((!at1) && at2)
        ajDebug("attributetypeadaptorCacheInsert detected an "
                "Ensembl Attribute Type object in the code, "
                "but not in the identifier cache.\n");

    return ajTrue;
}




#if AJFALSE
/* @funcstatic attributetypeadaptorCacheRemove ********************************
**
** Remove an Ensembl Attribute Type from the
** Ensembl Attribute Type Adaptor-internal cache.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool attributetypeadaptorCacheRemove(EnsPAttributetypeadaptor ata,
                                              EnsPAttributetype at)
{
    EnsPAttributetype at1 = NULL;
    EnsPAttributetype at2 = NULL;

    if (!ata)
        return ajFalse;

    if (!at)
        return ajFalse;

    at1 = (EnsPAttributetype) ajTableRemove(
        ata->CacheByIdentifier,
        (const void *) &at->Identifier);

    at2 = (EnsPAttributetype) ajTableRemove(
        ata->CacheByCode,
        (const void *) at->Code);

    if (at1 && (!at2))
        ajWarn("attributetypeadaptorCacheRemove could remove "
               "Ensembl Attribute Type with "
               "identifier %u and code '%S' only from the identifier cache.\n",
               at->Identifier,
               at->Code);

    if ((!at1) && at2)
        ajWarn("attributetypeadaptorCacheRemove could remove "
               "Ensembl Attribute Type with "
               "identifier %u and code '%S' only from the code cache.\n",
               at->Identifier,
               at->Code);

    ensAttributetypeDel(&at1);
    ensAttributetypeDel(&at2);

    return ajTrue;
}

#endif /* AJFALSE */




/* @func ensAttributetypeadaptorCacheClear ************************************
**
** Clear the Ensembl Attribute Type Adaptor-internal cache of
** Ensembl Attribute Type objects.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeadaptorCacheClear(EnsPAttributetypeadaptor ata)
{
    if (!ata)
        return ajFalse;

    ajTableDel(&ata->CacheByIdentifier);
    ajTableDel(&ata->CacheByCode);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Attribute Type Adaptor object.
**
** @fdata [EnsPAttributetypeadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Attribute Type Adaptor
**
** @argrule * Pata [EnsPAttributetypeadaptor*]
** Ensembl Attribute Type Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAttributetypeadaptorDel *******************************************
**
** Default destructor for an Ensembl Attribute Type Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pata [EnsPAttributetypeadaptor*]
** Ensembl Attribute Type Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensAttributetypeadaptorDel(EnsPAttributetypeadaptor *Pata)
{
    EnsPAttributetypeadaptor pthis = NULL;

    if (!Pata)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAttributetypeadaptorDel"))
        ajDebug("ensAttributetypeadaptorDel\n"
                "  *Pata %p\n",
                *Pata);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pata)
        return;

    pthis = *Pata;

    ajTableDel(&pthis->CacheByIdentifier);
    ajTableDel(&pthis->CacheByCode);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pata = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Attribute Type Adaptor object.
**
** @fdata [EnsPAttributetypeadaptor]
**
** @nam3rule Get Return Ensembl Attribute Type Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAttributetypeadaptorGetBaseadaptor ********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Attribute Type Adaptor.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensAttributetypeadaptorGetBaseadaptor(
    EnsPAttributetypeadaptor ata)
{
    return (ata) ? ata->Adaptor : NULL;
}




/* @func ensAttributetypeadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Attribute Type Adaptor.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAttributetypeadaptorGetDatabaseadaptor(
    EnsPAttributetypeadaptor ata)
{
    return (ata) ? ensBaseadaptorGetDatabaseadaptor(ata->Adaptor) : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Attribute Type objects from an
** Ensembl SQL database.
**
** @fdata [EnsPAttributetypeadaptor]
**
** @nam3rule Fetch Fetch Ensembl Attribute Type object(s)
** @nam4rule All Fetch all Ensembl Attribute Type objects
** @nam4rule Allby
** Fetch all Ensembl Attribute Type objects matching a criterion
** @nam4rule By Fetch one Ensembl Attribute Type object matching a criterion
** @nam5rule Code Fetch by a code
** @nam5rule Identifier Fetch by an SQL database internal identifier
**
** @argrule * ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @argrule All ats [AjPList] AJAX List of Ensembl Attribute Type objects
** @argrule ByCode code [const AjPStr] Ensembl Attribute Type code
** @argrule ByCode Pat [EnsPAttributetype*] Ensembl Attribute Type address
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pat [EnsPAttributetype*]
** Ensembl Attribute Type address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic attributetypeadaptorFetchAll ***********************************
**
** An ajTableMap "apply" function to return all Ensembl Attribute Type objects
** from the Ensembl Attribute Type Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] Pvalue [void**] Ensembl Attribute Type value data address
** @param [u] cl [void*]
** AJAX List of Ensembl Attribute Type objects, passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void attributetypeadaptorFetchAll(const void *key,
                                         void **Pvalue,
                                         void *cl)
{
    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void *) ensAttributetypeNewRef(
                         *((EnsPAttributetype *) Pvalue)));

    return;
}




/* @func ensAttributetypeadaptorFetchAll **************************************
**
** Fetch all Ensembl Attribute Type objects.
**
** The caller is responsible for deleting the Ensembl Attribute Type objects
** before deleting the AJAX List object.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @param [u] ats [AjPList] AJAX List of Ensembl Attribute Type objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeadaptorFetchAll(EnsPAttributetypeadaptor ata,
                                       AjPList ats)
{
    if (!ata)
        return ajFalse;

    if (!ats)
        return ajFalse;

    if (!ata->CacheByIdentifier)
        attributetypeadaptorCacheInit(ata);

    ajTableMap(ata->CacheByIdentifier,
               &attributetypeadaptorFetchAll,
               (void *) ats);

    return ajTrue;
}




/* @func ensAttributetypeadaptorFetchByCode ***********************************
**
** Fetch an Ensembl Attribute Type by its code.
**
** The caller is responsible for deleting the Ensembl Attribute Type.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @param [r] code [const AjPStr] Ensembl Attribute Type code
** @param [wP] Pat [EnsPAttributetype*] Ensembl Attribute Type address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeadaptorFetchByCode(EnsPAttributetypeadaptor ata,
                                          const AjPStr code,
                                          EnsPAttributetype *Pat)
{
    char *txtcode = NULL;

    AjPList ats = NULL;

    AjPStr constraint = NULL;

    EnsPAttributetype at = NULL;

    if (!ata)
        return ajFalse;

    if (!(code && ajStrGetLen(code)))
        return ajFalse;

    if (!Pat)
        return ajFalse;

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!ata->CacheByCode)
        attributetypeadaptorCacheInit(ata);

    *Pat = (EnsPAttributetype) ajTableFetchmodV(ata->CacheByCode,
                                                (const void *) code);

    if (*Pat)
    {
        ensAttributetypeNewRef(*Pat);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ensBaseadaptorEscapeC(ata->Adaptor, &txtcode, code);

    constraint = ajFmtStr("attrib_type.code = '%s'", txtcode);

    ajCharDel(&txtcode);

    ats = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ata->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       ats);

    if (ajListGetLength(ats) > 1)
        ajWarn("ensAttributetypeadaptorFetchByCode got more than one "
               "Ensembl Attribute Type for (UNIQUE) code '%S'.\n",
               code);

    ajListPop(ats, (void **) Pat);

    attributetypeadaptorCacheInsert(ata, Pat);

    while (ajListPop(ats, (void **) &at))
    {
        attributetypeadaptorCacheInsert(ata, &at);

        ensAttributetypeDel(&at);
    }

    ajListFree(&ats);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensAttributetypeadaptorFetchByIdentifier *****************************
**
** Fetch an Ensembl Attribute Type by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Attribute Type.
**
** @param [u] ata [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pat [EnsPAttributetype*] Ensembl Attribute Type address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAttributetypeadaptorFetchByIdentifier(EnsPAttributetypeadaptor ata,
                                                ajuint identifier,
                                                EnsPAttributetype *Pat)
{
    AjPList ats = NULL;

    AjPStr constraint = NULL;

    EnsPAttributetype at = NULL;

    if (!ata)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pat)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!ata->CacheByIdentifier)
        attributetypeadaptorCacheInit(ata);

    *Pat = (EnsPAttributetype) ajTableFetchmodV(ata->CacheByIdentifier,
                                                (const void *) &identifier);

    if (*Pat)
    {
        ensAttributetypeNewRef(*Pat);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    constraint = ajFmtStr("attrib_type.attrib_type_id = %u", identifier);

    ats = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ata->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       ats);

    if (ajListGetLength(ats) > 1)
        ajWarn("ensAttributetypeadaptorFetchByIdentifier got more than one "
               "Ensembl Attribute Type for (PRIMARY KEY) identifier %u.\n",
               identifier);

    ajListPop(ats, (void **) Pat);

    attributetypeadaptorCacheInsert(ata, Pat);

    while (ajListPop(ats, (void **) &at))
    {
        attributetypeadaptorCacheInsert(ata, &at);

        ensAttributetypeDel(&at);
    }

    ajListFree(&ats);

    ajStrDel(&constraint);

    return ajTrue;
}

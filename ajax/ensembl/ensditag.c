/* @source ensditag ***********************************************************
**
** Ensembl Ditag functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.60 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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

#include "ensditag.h"
#include "enstable.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic ditagadaptorKTables *******************************************
**
** Array of Ensembl Ditag Adaptor SQL table names
**
******************************************************************************/

static const char *const ditagadaptorKTables[] =
{
    "ditag",
    (const char *) NULL
};




/* @conststatic ditagadaptorKColumns ******************************************
**
** Array of Ensembl Ditag Adaptor SQL column names
**
******************************************************************************/

static const char *const ditagadaptorKColumns[] =
{
    "ditag.ditag_id",
    "ditag.name",
    "ditag.type",
    "ditag.tag_count",
    "ditag.sequence",
    (const char *) NULL
};




/* @conststatic ditagfeatureKSide *********************************************
**
** The Ditag Feature side is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to
** EnsEDitagfeatureSide.
**
** L: left or start feature
** R: right or end feature
** F: tags with only one feature (e.g. CAGE tags, which only have a 5' tag)
**
******************************************************************************/

static const char *const ditagfeatureKSide[] =
{
    "",
    "L",
    "R",
    "F",
    (const char *) NULL
};




/* @conststatic ditagfeatureadaptorKTables ************************************
**
** Array of Ensembl Ditag Feature Adaptor SQL table names
**
** NOTE: For now, the ditag_feature and ditag tables are permanently joined to
** allow for selection of Ditag Feature objects on the basis of a Ditag type.
**
******************************************************************************/

static const char *const ditagfeatureadaptorKTables[] =
{
    "ditag_feature",
    "ditag",
    (const char *) NULL
};




/* @conststatic ditagfeatureadaptorKColumns ***********************************
**
** Array of Ensembl Ditag Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const ditagfeatureadaptorKColumns[] =
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




/* @conststatic ditagfeatureadaptorKDefaultcondition **************************
**
** Ensembl Ditag Feature Adaptor default SQL condition
**
******************************************************************************/

static const char *ditagfeatureadaptorKDefaultcondition =
    "ditag_feature.ditag_id = ditag.ditag_id";




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static void ditagfeatureadaptorListDitagfeatureMove(void **Pkey,
                                                    void **Pvalue,
                                                    void *cl);

static void ditagfeatureadaptorListDitagfeatureValdel(void **Pvalue);

static AjBool ditagadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dts);

static int listDitagfeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listDitagfeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listDitagfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listDitagfeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listDitagfeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool ditagfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dtfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensditag ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPDitag] Ensembl Ditag *************************************
**
** @nam2rule Ditag Functions for manipulating Ensembl Ditag objects
**
** @cc Bio::EnsEMBL::Map::Ditag
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditag. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitag]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy dt [const EnsPDitag] Ensembl Ditag
** @argrule Ini dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ini type [AjPStr] Type
** @argrule Ini sequence [AjPStr] Sequence
** @argrule Ini count [ajuint] Count
** @argrule Ref dt [EnsPDitag] Ensembl Ditag
**
** @valrule * [EnsPDitag] Ensembl Ditag or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDitagNewCpy *******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [EnsPDitag] Ensembl Ditag or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDitag ensDitagNewCpy(const EnsPDitag dt)
{
    EnsPDitag pthis = NULL;

    if (!dt)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = dt->Identifier;
    pthis->Adaptor    = dt->Adaptor;

    if (dt->Name)
        pthis->Name = ajStrNewRef(dt->Name);

    if (dt->Type)
        pthis->Type = ajStrNewRef(dt->Type);

    if (dt->Sequence)
        pthis->Sequence = ajStrNewRef(dt->Sequence);

    pthis->Count = dt->Count;

    return pthis;
}




/* @func ensDitagNewIni *******************************************************
**
** Constructor for an Ensembl Ditag with initial values.
**
** @cc Bio::EnsEMBL::Storable
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Map::Ditag::new
** @param [u] name [AjPStr] Name
** @param [u] type [AjPStr] Type
** @param [u] sequence [AjPStr] Sequence
** @param [r] count [ajuint] Count
**
** @return [EnsPDitag] Ensembl Ditag or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDitag ensDitagNewIni(EnsPDitagadaptor dta,
                         ajuint identifier,
                         AjPStr name,
                         AjPStr type,
                         AjPStr sequence,
                         ajuint count)
{
    EnsPDitag dt = NULL;

    AJNEW0(dt);

    dt->Use        = 1U;
    dt->Identifier = identifier;
    dt->Adaptor    = dta;

    if (name)
        dt->Name = ajStrNewRef(name);

    if (type)
        dt->Type = ajStrNewRef(type);

    if (sequence)
        dt->Sequence = ajStrNewRef(sequence);

    dt->Count = count;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDitag ensDitagNewRef(EnsPDitag dt)
{
    if (!dt)
        return NULL;

    dt->Use++;

    return dt;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
** @nam3rule Del Destroy (free) an Ensembl Ditag
**
** @argrule * Pdt [EnsPDitag*] Ensembl Ditag address
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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensDitagDel(EnsPDitag *Pdt)
{
    EnsPDitag pthis = NULL;

    if (!Pdt)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensDitagDel"))
    {
        ajDebug("ensDitagDel\n"
                "  *Pdt %p\n",
                *Pdt);

        ensDitagTrace(*Pdt, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pdt)
        return;

    pthis = *Pdt;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pdt = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Sequence);

    AJFREE(pthis);

    *Pdt = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
** @nam3rule Get Return Ditag attribute(s)
** @nam4rule Adaptor Return the Ensembl Ditag Adaptor
** @nam4rule Count Return the count
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Sequence Return the sequence
** @nam4rule Type Return the type
**
** @argrule * dt [const EnsPDitag] Ditag
**
** @valrule Adaptor [EnsPDitagadaptor] Ensembl Ditag Adaptor or NULL
** @valrule Count [ajuint] Count or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Sequence [AjPStr] Sequence or NULL
** @valrule Type [AjPStr] Type or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDitagGetAdaptor ***************************************************
**
** Get the Ensembl Ditag Adaptor member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [EnsPDitagadaptor] Ensembl Ditag Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDitagadaptor ensDitagGetAdaptor(const EnsPDitag dt)
{
    return (dt) ? dt->Adaptor : NULL;
}




/* @func ensDitagGetCount *****************************************************
**
** Get the count member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::tag_count
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] Count or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensDitagGetCount(const EnsPDitag dt)
{
    return (dt) ? dt->Count : 0U;
}




/* @func ensDitagGetIdentifier ************************************************
**
** Get the SQL database-internal identifier member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensDitagGetIdentifier(const EnsPDitag dt)
{
    return (dt) ? dt->Identifier : 0U;
}




/* @func ensDitagGetName ******************************************************
**
** Get the name member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::name
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDitagGetName(const EnsPDitag dt)
{
    return (dt) ? dt->Name : NULL;
}




/* @func ensDitagGetSequence **************************************************
**
** Get the sequence member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::sequence
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Sequence or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDitagGetSequence(const EnsPDitag dt)
{
    return (dt) ? dt->Sequence : NULL;
}




/* @func ensDitagGetType ******************************************************
**
** Get the type member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::type
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Type or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDitagGetType(const EnsPDitag dt)
{
    return (dt) ? dt->Type : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
** @nam3rule Set Set one member of an Ensembl Ditag
** @nam4rule SetAdaptor Set the Ensembl Ditag Adaptor
** @nam4rule SetCount Set the count
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetSequence Set the sequence
** @nam4rule SetType Set the type
**
** @argrule * dt [EnsPDitag] Ensembl Ditag object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Adaptor dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @argrule Count count [ajuint] Count
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Sequence sequence [AjPStr] Sequence
** @argrule Type type [AjPStr] Type
**
** @fcategory modify
******************************************************************************/




/* @func ensDitagSetAdaptor ***************************************************
**
** Set the Ensembl Ditag Adaptor member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagSetAdaptor(EnsPDitag dt, EnsPDitagadaptor dta)
{
    if (!dt)
        return ajFalse;

    dt->Adaptor = dta;

    return ajTrue;
}




/* @func ensDitagSetCount *****************************************************
**
** Set the count member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::tag_count
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [r] count [ajuint] Count
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagSetCount(EnsPDitag dt, ajuint count)
{
    if (!dt)
        return ajFalse;

    dt->Count = count;

    return ajTrue;
}




/* @func ensDitagSetIdentifier ************************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagSetIdentifier(EnsPDitag dt, ajuint identifier)
{
    if (!dt)
        return ajFalse;

    dt->Identifier = identifier;

    return ajTrue;
}




/* @func ensDitagSetName ******************************************************
**
** Set the name member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::name
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagSetName(EnsPDitag dt, AjPStr name)
{
    if (!dt)
        return ajFalse;

    ajStrDel(&dt->Name);

    dt->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensDitagSetSequence **************************************************
**
** Set the sequence member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::sequence
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] sequence [AjPStr] Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagSetSequence(EnsPDitag dt, AjPStr sequence)
{
    if (!dt)
        return ajFalse;

    ajStrDel(&dt->Sequence);

    dt->Sequence = ajStrNewRef(sequence);

    return ajTrue;
}




/* @func ensDitagSetType ******************************************************
**
** Set the type member of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::type
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] type [AjPStr] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagSetType(EnsPDitag dt, AjPStr type)
{
    if (!dt)
        return ajFalse;

    ajStrDel(&dt->Type);

    dt->Type = ajStrNewRef(type);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
** @nam3rule Trace Report Ensembl Ditag members to debug file.
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagTrace(const EnsPDitag dt, ajuint level)
{
    AjPStr indent = NULL;

    if (!dt)
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




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
** @nam3rule Calculate Calculate Ensembl Ditag values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize dt [const EnsPDitag] Ensembl Ditag
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensDitagCalculateMemsize *********************************************
**
** Calculate the memory size in bytes of an Ensembl Ditag.
**
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensDitagCalculateMemsize(const EnsPDitag dt)
{
    size_t size = 0;

    if (!dt)
        return 0;

    size += sizeof (EnsODitag);

    if (dt->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dt->Name);
    }

    if (dt->Type)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dt->Type);
    }

    if (dt->Sequence)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dt->Sequence);
    }

    return size;
}




/* @datasection [EnsPDitagadaptor] Ensembl Ditag Adaptor **********************
**
** @nam2rule Ditagadaptor Functions for manipulating
** Ensembl Ditag Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor
** @cc CVS Revision: 1.11
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic ditagadaptorFetchAllbyStatement ********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Ditag objects.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::_fetch
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] dts [AjPList] AJAX List of Ensembl Ditag objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool ditagadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dts)
{
    ajuint identifier = 0U;
    ajuint count      = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name     = NULL;
    AjPStr type     = NULL;
    AjPStr sequence = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDitag dt         = NULL;
    EnsPDitagadaptor dta = NULL;

    if (ajDebugTest("ditagadaptorFetchAllbyStatement"))
        ajDebug("ditagadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  dts %p\n",
                ba,
                statement,
                am,
                slice,
                dts);

    if (!ba)
        return ajFalse;

    if (!(statement && ajStrGetLen(statement)))
        return ajFalse;

    if (!dts)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    dta = ensRegistryGetDitagadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
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

        dt = ensDitagNewIni(dta, identifier, name, type, sequence, count);

        ajListPushAppend(dts, (void *) dt);

        ajStrDel(&name);
        ajStrDel(&type);
        ajStrDel(&sequence);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditag Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDitagadaptor] Ensembl Ditag Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDitagadaptorNew ***************************************************
**
** Default constructor for an Ensembl Ditag Adaptor.
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
** @see ensRegistryGetDitagadaptor
**
** @cc Bio::EnsEMBL::DBSQL::DitagAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDitagadaptor] Ensembl Ditag Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDitagadaptor ensDitagadaptorNew(EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    if (ajDebugTest("ensDitagadaptorNew"))
        ajDebug("ensDitagadaptorNew\n"
                "  dba %p\n",
                dba);

    return ensBaseadaptorNew(
        dba,
        ditagadaptorKTables,
        ditagadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &ditagadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ditag Adaptor object.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Adaptor
**
** @argrule * Pdta [EnsPDitagadaptor*] Ensembl Ditag Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDitagadaptorDel ***************************************************
**
** Default destructor for an Ensembl Ditag Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pdta [EnsPDitagadaptor*] Ensembl Ditag Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensDitagadaptorDel(EnsPDitagadaptor *Pdta)
{
    ensBaseadaptorDel(Pdta);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ditag Adaptor object.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule Get Return Ensembl Ditag Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDitagadaptorGetDatabaseadaptor ************************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Ditag Adaptor.
** The Ensembl Ditag Adaptor is just an alias for the
** Ensembl Database Adaptor.
**
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDitagadaptorGetDatabaseadaptor(EnsPDitagadaptor dta)
{
    return (dta) ? ensBaseadaptorGetDatabaseadaptor(dta) : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Ditag objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule Fetch Fetch Ensembl Ditag object(s)
** @nam4rule All Fetch all Ensembl Ditag objects
** @nam4rule Allby Fetch all Ensembl Ditag objects matching a criterion
** @nam5rule Identifiers Fetch all by an AJAX Table
** @nam5rule Name Fetch by a name
** @nam5rule Type Fetch by a type
** @nam4rule By Fetch one Ensembl Ditag object matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @argrule All name [const AjPStr] Name
** @argrule All type [const AjPStr] Type
** @argrule All dts [AjPList] AJAX List of Ensembl Ditag objects
** @argrule AllbyIdentifiers dts [AjPTable] AJAX Table
** @argrule AllbyName name [const AjPStr] Name
** @argrule AllbyName dts [AjPList] AJAX List of Ensembl Ditag objects
** @argrule AllbyType type [const AjPStr] Type
** @argrule AllbyType dts [AjPList] AJAX List of Ensembl Ditag objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pdt [EnsPDitag*] Ensembl Ditag address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDitagadaptorFetchAll **********************************************
**
** Fetch all Ensembl Ditag objects by name or type.
**
** The caller is responsible for deleting the Ensembl Ditag objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::fetch_all
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::fetch_all_by_name
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::fetch_all_by_type
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @param [rN] name [const AjPStr] Name
** @param [rN] type [const AjPStr] Type
** @param [u] dts [AjPList] AJAX List of Ensembl Ditag objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAll(EnsPDitagadaptor dta,
                               const AjPStr name,
                               const AjPStr type,
                               AjPList dts)
{
    char *txtname = NULL;
    char *txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dta)
        return ajFalse;

    if (!dts)
        return ajFalse;

    if (name && ajStrGetLen(name))
        ensBaseadaptorEscapeC(dta, &txtname, name);

    if (type && ajStrGetLen(type))
        ensBaseadaptorEscapeC(dta, &txttype, type);

    if (txtname || txttype)
        constraint = ajStrNew();

    if (txtname)
        ajFmtPrintAppS(&constraint, "ditag.name = '%s'", txtname);

    if (txttype)
    {
        if (txtname)
            ajStrAppendC(&constraint, " AND");

        ajFmtPrintAppS(&constraint, " ditag.type = '%s'", txttype);
    }

    ajCharDel(&txtname);

    ajCharDel(&txttype);

    result = ensBaseadaptorFetchAllbyConstraint(dta,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                dts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDitagadaptorFetchAllbyIdentifiers *********************************
**
** Fetch Ensembl Ditag objects by an AJAX Table of AJAX unsigned integer
** key data and assign them as value data.
**
** The caller is responsible for deleting the Ensembl Ditag value data
** before deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @param [u] dts [AjPTable]
** AJAX Table of AJAX unsigned integer identifier key data and
** Ensembl Ditag value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllbyIdentifiers(EnsPDitagadaptor dta,
                                            AjPTable dts)
{
    return ensBaseadaptorFetchAllbyIdentifiers(
        dta,
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensDitagGetIdentifier,
        dts);
}




/* @func ensDitagadaptorFetchAllbyName ****************************************
**
** Fetch all Ensembl Ditag objects by name.
**
** The caller is responsible for deleting the Ensembl Ditag objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::fetch_all_by_name
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @param [r] name [const AjPStr] Name
** @param [u] dts [AjPList] AJAX List of Ensembl Ditag objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllbyName(EnsPDitagadaptor dta,
                                     const AjPStr name,
                                     AjPList dts)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dta)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!dts)
        return ajFalse;

    ensBaseadaptorEscapeC(dta, &txtname, name);

    constraint = ajFmtStr("ditag.name = '%s'", txtname);

    ajCharDel(&txtname);

    result = ensBaseadaptorFetchAllbyConstraint(dta,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                dts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDitagadaptorFetchAllbyType ****************************************
**
** Fetch all Ensembl Ditag objects by type.
**
** The caller is responsible for deleting the Ensembl Ditag objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::fetch_all_by_type
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @param [r] type [const AjPStr] Type
** @param [u] dts [AjPList] AJAX List of Ensembl Ditag objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllbyType(EnsPDitagadaptor dta,
                                     const AjPStr type,
                                     AjPList dts)
{
    char *txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dta)
        return ajFalse;

    if (!type)
        return ajFalse;

    if (!dts)
        return ajFalse;

    ensBaseadaptorEscapeC(dta, &txttype, type);

    constraint = ajFmtStr("ditag.type = '%s'", txttype);

    ajCharDel(&txttype);

    result = ensBaseadaptorFetchAllbyConstraint(dta,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                dts);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDitagadaptorFetchByIdentifier *************************************
**
** Fetch an Ensembl Ditag via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::fetch_by_dbID
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdt [EnsPDitag*] Ensembl Ditag address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchByIdentifier(EnsPDitagadaptor dta,
                                        ajuint identifier,
                                        EnsPDitag *Pdt)
{
    if (!dta)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pdt)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(dta, identifier, (void **) Pdt);
}




/* @datasection [EnsPDitagfeature] Ensembl Ditag Feature **********************
**
** @nam2rule Ditagfeature Functions for manipulating
** Ensembl Ditag Feature objects
**
** @cc Bio::EnsEMBL::Map::DitagFeature
** @cc CVS Revision: 1.17
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditag Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy dtf [const EnsPDitagfeature] Ensembl Ditag Feature
** @argrule Ini dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini dt [EnsPDitag] Ensembl Ditag
** @argrule Ini cigar [AjPStr] CIGAR line
** @argrule Ini side [EnsEDitagfeatureSide] Side
** @argrule Ini tstart [ajint] Target start
** @argrule Ini tend [ajint] Target end
** @argrule Ini tstrand [ajint] Target strand
** @argrule Ini pairid [ajuint] Pair identifier
** @argrule Ref dtf [EnsPDitagfeature] Ensembl Ditag Feature
**
** @valrule * [EnsPDitagfeature] Ensembl Ditag Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDitagfeatureNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitagfeature] Ensembl Ditag Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNewCpy(const EnsPDitagfeature dtf)
{
    EnsPDitagfeature pthis = NULL;

    if (!dtf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = dtf->Identifier;
    pthis->Adaptor    = dtf->Adaptor;
    pthis->Feature    = ensFeatureNewRef(dtf->Feature);
    pthis->Ditag      = ensDitagNewRef(dtf->Ditag);

    if (dtf->Cigar)
        pthis->Cigar = ajStrNewRef(dtf->Cigar);

    pthis->Side           = dtf->Side;
    pthis->TargetStart    = dtf->TargetStart;
    pthis->TargetEnd      = dtf->TargetEnd;
    pthis->TargetStrand   = dtf->TargetStrand;
    pthis->Pairidentifier = dtf->Pairidentifier;

    return pthis;
}




/* @func ensDitagfeatureNewIni ************************************************
**
** Constructor for an Ensembl Ditag Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Map::DitagFeature::new
** @param [uN] dt [EnsPDitag] Ensembl Ditag
** @param [uN] cigar [AjPStr] CIGAR line
** @param [u] side [EnsEDitagfeatureSide] Side
** @param [r] tstart [ajint] Target start
** @param [r] tend [ajint] Target end
** @param [r] tstrand [ajint] Target strand
** @param [r] pairid [ajuint] Pair identifier
**
** @return [EnsPDitagfeature] Ensembl Ditag Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNewIni(EnsPDitagfeatureadaptor dtfa,
                                       ajuint identifier,
                                       EnsPFeature feature,
                                       EnsPDitag dt,
                                       AjPStr cigar,
                                       EnsEDitagfeatureSide side,
                                       ajint tstart,
                                       ajint tend,
                                       ajint tstrand,
                                       ajuint pairid)
{
    EnsPDitagfeature dtf = NULL;

    if (!feature)
        return NULL;

    if ((tstrand < -1) || (tstrand > 1))
    {
        ajDebug("ensDitagfeatureNewIni got target strand not -1, 0 or +1.\n");

        return NULL;
    }

    AJNEW0(dtf);

    dtf->Use        = 1U;
    dtf->Identifier = identifier;
    dtf->Adaptor    = dtfa;
    dtf->Feature    = ensFeatureNewRef(feature);
    dtf->Ditag      = ensDitagNewRef(dt);

    if (cigar)
        dtf->Cigar = ajStrNewRef(cigar);

    dtf->Side           = side;
    dtf->TargetStart    = tstart;
    dtf->TargetEnd      = tend;
    dtf->TargetStrand   = tstrand;
    dtf->Pairidentifier = pairid;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNewRef(EnsPDitagfeature dtf)
{
    if (!dtf)
        return NULL;

    dtf->Use++;

    return dtf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Feature
**
** @argrule * Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature address
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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensDitagfeatureDel(EnsPDitagfeature *Pdtf)
{
    EnsPDitagfeature pthis = NULL;

    if (!Pdtf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensDitagfeatureDel"))
    {
        ajDebug("ensDitagfeatureDel\n"
                "  *Pdtf %p\n",
                *Pdtf);

        ensDitagfeatureTrace(*Pdtf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pdtf)
        return;

    pthis = *Pdtf;

    pthis->Use--;

    if (pthis->Use)
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




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule Get Return Ditag Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Ditag Feature Adaptor
** @nam4rule Cigar Return the CIGAR line
** @nam4rule Ditag Return the Ensembl Ditag
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Pairidentifier Return the pair identifier
** @nam4rule Side Return the side
** @nam4rule Target Return target members
** @nam5rule End Return the target end
** @nam5rule Start Return the target start
** @nam5rule Strand Return the target strand
**
** @argrule * dtf [const EnsPDitagfeature] Ditag Feature
**
** @valrule Adaptor [EnsPDitagfeatureadaptor]
** Ensembl Ditag Feature Adaptor or NULL
** @valrule Cigar [AjPStr] CIGAR line or NULL
** @valrule Ditag [EnsPDitag] Ensembl Ditag or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Pairidentifier [ajuint] Pair identifier or 0U
** @valrule Side [EnsEDitagfeatureSide] Side or ensEDitagfeatureSideNULL
** @valrule TargetEnd [ajint] Target end or 0
** @valrule TargetStart [ajint] Target start or 0
** @valrule TargetStrand [ajint] Target strand or 0
**
** @fcategory use
******************************************************************************/




/* @func ensDitagfeatureGetAdaptor ********************************************
**
** Get the Ensembl Ditag Feature Adaptor member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensDitagfeatureGetAdaptor(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Adaptor : NULL;
}




/* @func ensDitagfeatureGetCigar **********************************************
**
** Get the CIGAR line member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::cigar_line
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [AjPStr] CIGAR line or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDitagfeatureGetCigar(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Cigar : NULL;
}




/* @func ensDitagfeatureGetDitag **********************************************
**
** Get the Ensembl Ditag member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitag] Ensembl Ditag or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDitag ensDitagfeatureGetDitag(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Ditag : NULL;
}




/* @func ensDitagfeatureGetFeature ********************************************
**
** Get the Ensembl Feature member of an Ensembl Ditag Feature.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensDitagfeatureGetFeature(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Feature : NULL;
}




/* @func ensDitagfeatureGetIdentifier *****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensDitagfeatureGetIdentifier(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Identifier : 0U;
}




/* @func ensDitagfeatureGetPairidentifier *************************************
**
** Get the pair identifier member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_pair_id
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] Pair identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensDitagfeatureGetPairidentifier(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Pairidentifier : 0U;
}




/* @func ensDitagfeatureGetSide ***********************************************
**
** Get the side member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_side
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsEDitagfeatureSide] Side or ensEDitagfeatureSideNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEDitagfeatureSide ensDitagfeatureGetSide(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->Side : ensEDitagfeatureSideNULL;
}




/* @func ensDitagfeatureGetTargetEnd ******************************************
**
** Get the target end member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_end
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target end or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetEnd(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->TargetEnd : 0;
}




/* @func ensDitagfeatureGetTargetStart ****************************************
**
** Get the target start member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_start
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target start or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetStart(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->TargetStart : 0;
}




/* @func ensDitagfeatureGetTargetStrand ***************************************
**
** Get the target strand member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_strand
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target strand or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetStrand(const EnsPDitagfeature dtf)
{
    return (dtf) ? dtf->TargetStrand : 0;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule Set Set one member of an Ensembl Ditag Feature
** @nam4rule Adaptor Set the Ensembl Ditag Feature Adaptor
** @nam4rule Cigar Set the CIGAR line
** @nam4rule Ditag Set the Ensembl Ditag
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Pairidentifier Set the pair identifier
** @nam4rule Side Set the side
** @nam4rule Target Set target members
** @nam5rule End Set the target end
** @nam5rule Start Set the target start
** @nam5rule Strand Set the target strand
**
** @argrule * dtf [EnsPDitagfeature] Ensembl Ditag Feature object
** @argrule Adaptor dtfa [EnsPDitagfeatureadaptor]
** Ensembl Ditag Feature Adaptor
** @argrule Cigar cigar [AjPStr] CIGAR line
** @argrule Ditag dt [EnsPDitag] Ensembl Ditag
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Pairidentifier pairid [ajuint] Pair identifier
** @argrule Side side [EnsEDitagfeatureSide] Side
** @argrule TargetEnd tend [ajint] Target end
** @argrule TargetStart tstart [ajint] Target start
** @argrule TargetStrand tstrand [ajint] Target strand
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDitagfeatureSetAdaptor ********************************************
**
** Set the Ensembl Ditag Feature Adaptor member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetAdaptor(EnsPDitagfeature dtf,
                                 EnsPDitagfeatureadaptor dtfa)
{
    if (!dtf)
        return ajFalse;

    dtf->Adaptor = dtfa;

    return ajTrue;
}




/* @func ensDitagfeatureSetCigar **********************************************
**
** Set the CIGAR line member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::cigar_line
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] cigar [AjPStr] CIGAR line
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetCigar(EnsPDitagfeature dtf, AjPStr cigar)
{
    if (!dtf)
        return ajFalse;

    ajStrDel(&dtf->Cigar);

    dtf->Cigar = ajStrNewRef(cigar);

    return ajTrue;
}




/* @func ensDitagfeatureSetDitag **********************************************
**
** Set the Ensembl Ditag member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] dt [EnsPDitag] Ensembl Ditag
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetDitag(EnsPDitagfeature dtf, EnsPDitag dt)
{
    if (!dtf)
        return ajFalse;

    ensDitagDel(&dtf->Ditag);

    dtf->Ditag = ensDitagNewRef(dt);

    return ajTrue;
}




/* @func ensDitagfeatureSetFeature ********************************************
**
** Set the Ensembl Feature member of an Ensembl Ditag Feature.
**
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetFeature(EnsPDitagfeature dtf, EnsPFeature feature)
{
    if (!dtf)
        return ajFalse;

    ensFeatureDel(&dtf->Feature);

    dtf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensDitagfeatureSetIdentifier *****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetIdentifier(EnsPDitagfeature dtf, ajuint identifier)
{
    if (!dtf)
        return ajFalse;

    dtf->Identifier = identifier;

    return ajTrue;
}




/* @func ensDitagfeatureSetPairidentifier *************************************
**
** Set the pair identifier member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_pair_id
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] pairid [ajuint] Pair identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetPairidentifier(EnsPDitagfeature dtf, ajuint pairid)
{
    if (!dtf)
        return ajFalse;

    dtf->Pairidentifier = pairid;

    return ajTrue;
}




/* @func ensDitagfeatureSetSide ***********************************************
**
** Set the side member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_side
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] side [EnsEDitagfeatureSide] Side
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetSide(EnsPDitagfeature dtf, EnsEDitagfeatureSide side)
{
    if (!dtf)
        return ajFalse;

    dtf->Side = side;

    return ajTrue;
}




/* @func ensDitagfeatureSetTargetEnd ******************************************
**
** Set the target end member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_end
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] tend [ajint] Target end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetTargetEnd(EnsPDitagfeature dtf, ajint tend)
{
    if (!dtf)
        return ajFalse;

    dtf->TargetEnd = tend;

    return ajTrue;
}




/* @func ensDitagfeatureSetTargetStart ****************************************
**
** Set the target start member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_start
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] tstart [ajint] Target start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetTargetStart(EnsPDitagfeature dtf, ajint tstart)
{
    if (!dtf)
        return ajFalse;

    dtf->TargetStart = tstart;

    return ajTrue;
}




/* @func ensDitagfeatureSetTargetStrand ***************************************
**
** Set the target strand member of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_strand
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [r] tstrand [ajint] Target strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureSetTargetStrand(EnsPDitagfeature dtf, ajint tstrand)
{
    if (!dtf)
        return ajFalse;

    dtf->TargetStrand = tstrand;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule Trace Report Ensembl Ditag Feature members to debug file.
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureTrace(const EnsPDitagfeature dtf, ajuint level)
{
    AjPStr indent = NULL;

    if (!dtf)
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
            "%S  Pairidentifier %u\n",
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
            indent, dtf->Pairidentifier);

    ensFeatureTrace(dtf->Feature, level + 1);

    ensDitagTrace(dtf->Ditag, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule Calculate Calculate Ensembl Ditag Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensDitagfeatureCalculateMemsize **************************************
**
** Get the memory size in bytes of an Ensembl Ditag Feature.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensDitagfeatureCalculateMemsize(const EnsPDitagfeature dtf)
{
    size_t size = 0;

    if (!dtf)
        return 0;

    size += sizeof (EnsODitagfeature);

    size += ensFeatureCalculateMemsize(dtf->Feature);

    size += ensDitagCalculateMemsize(dtf->Ditag);

    if (dtf->Cigar)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dtf->Cigar);
    }

    return size;
}




/* @datasection [EnsEDitagfeatureSide] Ensembl Ditag Feature Side *************
**
** @nam2rule Ditagfeature Functions for manipulating
** Ensembl Ditag Feature objects
** @nam3rule DitagfeatureSide Functions for manipulating
** Ensembl Ditag Feature Side enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Ditag Feature Side enumeration.
**
** @fdata [EnsEDitagfeatureSide]
**
** @nam4rule From Ensembl Ditag Feature Side query
** @nam5rule Str AJAX String object query
**
** @argrule Str side [const AjPStr] Side string
**
** @valrule * [EnsEDitagfeatureSide]
** Ensembl Ditag Feature Side or ensEDitagfeatureSideNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensDitagfeatureSideFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Ditag Feature Side enumeration.
**
** @param [r] side [const AjPStr] Ensembl Ditag Feature Side string
**
** @return [EnsEDitagfeatureSide]
** Ensembl Ditag Feature Side or ensEDitagfeatureSideNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEDitagfeatureSide ensDitagfeatureSideFromStr(const AjPStr side)
{
    register EnsEDitagfeatureSide i = ensEDitagfeatureSideNULL;

    EnsEDitagfeatureSide eside = ensEDitagfeatureSideNULL;

    for (i = ensEDitagfeatureSideNULL;
         ditagfeatureKSide[i];
         i++)
        if (ajStrMatchC(side, ditagfeatureKSide[i]))
            eside = i;

    if (!eside)
        ajDebug("ensDitagfeatureSideFromStr encountered "
                "unexpected string '%S'.\n", side);

    return eside;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Ditag Feature Side enumeration.
**
** @fdata [EnsEDitagfeatureSide]
**
** @nam4rule To   Return Ensembl Ditag Feature Side enumeration
** @nam5rule Char Return C character string value
**
** @argrule To dtfs [EnsEDitagfeatureSide]
** Ensembl Ditag Feature Side enumeration
**
** @valrule Char [const char*]
** Ensembl Ditag Feature Side C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensDitagfeatureSideToChar ********************************************
**
** Convert an Ensembl Ditag Feature Side enumeration into
** a C-type (char *) string.
**
** @param [u] dtfs [EnsEDitagfeatureSide]
** Ensembl Ditag Feature Side enumeration
**
** @return [const char*] Ensembl Ditag Feature Side C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensDitagfeatureSideToChar(EnsEDitagfeatureSide dtfs)
{
    register EnsEDitagfeatureSide i = ensEDitagfeatureSideNULL;

    for (i = ensEDitagfeatureSideNULL;
         ditagfeatureKSide[i] && (i < dtfs);
         i++);

    if (!ditagfeatureKSide[i])
        ajDebug("ensDitagfeatureSideToChar encountered an "
                "out of boundary error on "
                "Ensembl Ditag Feature Side enumeration %d.\n",
                dtfs);

    return ditagfeatureKSide[i];
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listDitagfeatureCompareEndAscending ****************************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** Ensembl Feature end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Ditag Feature address 1
** @param [r] item2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listDitagfeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPDitagfeature dtf1 = *(EnsODitagfeature *const *) item1;
    EnsPDitagfeature dtf2 = *(EnsODitagfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listDitagfeatureCompareEndAscending"))
        ajDebug("listDitagfeatureCompareEndAscending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (dtf1 && (!dtf2))
        return -1;

    if ((!dtf1) && (!dtf2))
        return 0;

    if ((!dtf1) && dtf2)
        return +1;

    return ensFeatureCompareEndAscending(dtf1->Feature, dtf2->Feature);
}




/* @funcstatic listDitagfeatureCompareEndDescending ***************************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** Ensembl Feature end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Ditag Feature address 1
** @param [r] item2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listDitagfeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPDitagfeature dtf1 = *(EnsODitagfeature *const *) item1;
    EnsPDitagfeature dtf2 = *(EnsODitagfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listDitagfeatureCompareEndDescending"))
        ajDebug("listDitagfeatureCompareEndDescending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (dtf1 && (!dtf2))
        return -1;

    if ((!dtf1) && (!dtf2))
        return 0;

    if ((!dtf1) && dtf2)
        return +1;

    return ensFeatureCompareEndDescending(dtf1->Feature, dtf2->Feature);
}




/* @funcstatic listDitagfeatureCompareIdentifierAscending *********************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Ditag Feature address 1
** @param [r] item2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listDitagfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPDitagfeature dtf1 = *(EnsODitagfeature *const *) item1;
    EnsPDitagfeature dtf2 = *(EnsODitagfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listDitagfeatureCompareIdentifierAscending"))
        ajDebug("listDitagfeatureCompareIdentifierAscending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (dtf1 && (!dtf2))
        return -1;

    if ((!dtf1) && (!dtf2))
        return 0;

    if ((!dtf1) && dtf2)
        return +1;

    if (dtf1->Identifier < dtf2->Identifier)
        return -1;

    if (dtf1->Identifier > dtf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listDitagfeatureCompareStartAscending **************************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Ditag Feature address 1
** @param [r] item2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listDitagfeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPDitagfeature dtf1 = *(EnsODitagfeature *const *) item1;
    EnsPDitagfeature dtf2 = *(EnsODitagfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listDitagfeatureCompareStartAscending"))
        ajDebug("listDitagfeatureCompareStartAscending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (dtf1 && (!dtf2))
        return -1;

    if ((!dtf1) && (!dtf2))
        return 0;

    if ((!dtf1) && dtf2)
        return +1;

    return ensFeatureCompareStartAscending(dtf1->Feature, dtf2->Feature);
}




/* @funcstatic listDitagfeatureCompareStartDescending *************************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Ditag Feature address 1
** @param [r] item2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listDitagfeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPDitagfeature dtf1 = *(EnsODitagfeature *const *) item1;
    EnsPDitagfeature dtf2 = *(EnsODitagfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listDitagfeatureCompareStartDescending"))
        ajDebug("listDitagfeatureCompareStartDescending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (dtf1 && (!dtf2))
        return -1;

    if ((!dtf1) && (!dtf2))
        return 0;

    if ((!dtf1) && dtf2)
        return +1;

    return ensFeatureCompareStartDescending(dtf1->Feature, dtf2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Ditagfeature Functions for manipulating AJAX List objects of
** Ensembl Ditag Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * dtfs [AjPList]
** AJAX List of Ensembl Ditag Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListDitagfeatureSortEndAscending **********************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** Ensembl Feature end member in ascending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortEndAscending(AjPList dtfs)
{
    if (!dtfs)
        return ajFalse;

    ajListSortTwoThree(dtfs,
                       &listDitagfeatureCompareEndAscending,
                       &listDitagfeatureCompareStartAscending,
                       &listDitagfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListDitagfeatureSortEndDescending *********************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** Ensembl Feature end member in descending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortEndDescending(AjPList dtfs)
{
    if (!dtfs)
        return ajFalse;

    ajListSortTwoThree(dtfs,
                       &listDitagfeatureCompareEndDescending,
                       &listDitagfeatureCompareStartDescending,
                       &listDitagfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListDitagfeatureSortIdentifierAscending ***************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** identifier member in ascending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortIdentifierAscending(AjPList dtfs)
{
    if (!dtfs)
        return ajFalse;

    ajListSort(dtfs, &listDitagfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListDitagfeatureSortStartAscending ********************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** Ensembl Feature start member in ascending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortStartAscending(AjPList dtfs)
{
    if (!dtfs)
        return ajFalse;

    ajListSortTwoThree(dtfs,
                       &listDitagfeatureCompareStartAscending,
                       &listDitagfeatureCompareEndAscending,
                       &listDitagfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListDitagfeatureSortStartDescending *******************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** Ensembl Feature start member in descending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortStartDescending(AjPList dtfs)
{
    if (!dtfs)
        return ajFalse;

    ajListSortTwoThree(dtfs,
                       &listDitagfeatureCompareStartDescending,
                       &listDitagfeatureCompareEndDescending,
                       &listDitagfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor *******
**
** @nam2rule Ditagfeatureadaptor Functions for manipulating
** Ensembl Ditag Feature Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor
** @cc CVS Revision: 1.21
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic ditagfeatureadaptorListDitagfeatureMove ************************
**
** An ajTableMapDel "apply" function to link Ensembl Ditag Feature objects to
** Ensembl Ditag objects.
** This function also deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Ditag Feature objects after association.
**
** @param [d] Pkey [void**] AJAX unsigned integer key data address
** @param [d] Pvalue [void**] AJAX Lists of Ensembl Ditag Feature objects
** @param [u] cl [void*]
** AJAX Table of Ensembl Ditag objects, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void ditagfeatureadaptorListDitagfeatureMove(void **Pkey,
                                                    void **Pvalue,
                                                    void *cl)
{
    EnsPDitag dt = NULL;

    EnsPDitagfeature dtf = NULL;

    if (!Pkey)
        return;

    if (!*Pkey)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    dt = (EnsPDitag) ajTableFetchmodV(cl, *Pkey);

    /*
    ** The Ensembl Ditag Feature objects can be deleted after associating
    ** them with Ensembl Ditag Feature objects, because this AJAX Table holds
    ** independent references for these objects.
    */

    while (ajListPop(*((AjPList *) Pvalue), (void **) &dtf))
    {
        ensDitagfeatureSetDitag(dtf, dt);

        ensDitagfeatureDel(&dtf);
    }

    ajMemFree(Pkey);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic ditagfeatureadaptorListDitagfeatureValdel **********************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl Ditag Feature objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX Lists of Ensembl Ditag Feature objects
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void ditagfeatureadaptorListDitagfeatureValdel(void **Pvalue)
{
    EnsPDitagfeature dtf = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &dtf))
        ensDitagfeatureDel(&dtf);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic ditagfeatureadaptorFetchAllbyStatement *************************
**
** Fetch all Ensembl Ditag Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::_fetch
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool ditagfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dtfs)
{

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    ajuint identifier = 0U;
    ajuint analysisid = 0U;
    ajuint dtid       = 0U;
    ajuint dtstart    = 0U;
    ajuint dtend      = 0U;
    ajint  dtstrand   = 0;
    ajuint dtpairid   = 0U;

    ajuint *Pidentifier = NULL;

    EnsEDitagfeatureSide eside = ensEDitagfeatureSideNULL;

    AjPList list = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr cigar = NULL;
    AjPStr side  = NULL;

    AjPTable dts      = NULL;
    AjPTable dttodtfs = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDitagadaptor dta = NULL;

    EnsPDitagfeature dtf         = NULL;
    EnsPDitagfeatureadaptor dtfa = NULL;

    EnsPFeature feature = NULL;

    if (ajDebugTest("ditagfeatureadaptorFetchAllbyStatement"))
        ajDebug("ditagfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  dtfs %p\n",
                ba,
                statement,
                am,
                slice,
                dtfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!dtfs)
        return ajFalse;

    dts      = ajTableuintNew(0);
    dttodtfs = ajTableuintNew(0);

    ajTableSetDestroyvalue(
        dts,
        (void (*)(void **)) &ensDitagDel);

    ajTableSetDestroyvalue(
        dttodtfs,
        (void (*)(void **)) &ditagfeatureadaptorListDitagfeatureValdel);

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    dta  = ensRegistryGetDitagadaptor(dba);
    dtfa = ensRegistryGetDitagfeatureadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
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

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&cigar);
            ajStrDel(&side);

            continue;
        }

        eside = ensDitagfeatureSideFromStr(side);

        dtf = ensDitagfeatureNewIni(dtfa,
                                    identifier,
                                    feature,
                                    (EnsPDitag) NULL,
                                    cigar,
                                    eside,
                                    dtstart,
                                    dtend,
                                    dtstrand,
                                    dtpairid);

        ajListPushAppend(dtfs, (void *) dtf);

        /*
        ** Populate two AJAX Table objects to fetch Ensembl Ditag objects from
        ** the database and associate them with Ensembl Ditag Feature objects.
        **
        ** dts
        **   key data:   Ensembl Ditag identifiers
        **   value data: Ensembl Ditag objects fetched by
        **               ensDitagadaptorFetchAllbyIdentifiers
        **
        ** dttodtfs
        **   key data:   Ensembl Ditag identifiers
        **   value data: AJAX List objects of Ensembl Ditag Feature objects
        **               that need to be associated with Ensembl Ditag objects
        **               once they have been fetched from the database
        */

        if (!ajTableMatchV(dts, (const void *) &dtid))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = dtid;

            ajTablePut(dts, (void *) Pidentifier, (void *) NULL);
        }

        list = (AjPList) ajTableFetchmodV(dttodtfs, (const void *) &dtid);

        if (!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = dtid;

            list = ajListNew();

            ajTablePut(dttodtfs,
                       (void *) Pidentifier,
                       (void *) list);
        }

        ajListPushAppend(list, (void *) ensDitagfeatureNewRef(dtf));

        ensFeatureDel(&feature);

        ajStrDel(&cigar);
        ajStrDel(&side);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ensDitagadaptorFetchAllbyIdentifiers(dta, dts);

    /* Associate Ensembl Ditag objects with Ensembl Ditag Feature objects. */

    ajTableMapDel(dttodtfs,
                  &ditagfeatureadaptorListDitagfeatureMove,
                  (void *) dts);

    ajTableDel(&dttodtfs);
    ajTableDel(&dts);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ditag Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Ditag Feature Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDitagfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDitagfeatureadaptorNew ********************************************
**
** Default constructor for an Ensembl Ditag Feature Adaptor.
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
** @see ensRegistryGetDitagfeatureadaptor
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensDitagfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        ditagfeatureadaptorKTables,
        ditagfeatureadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        ditagfeatureadaptorKDefaultcondition,
        (const char *) NULL,
        &ditagfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensDitagfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensDitagfeatureDel,
        (size_t (*)(const void *)) &ensDitagfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensDitagfeatureGetFeature,
        "Ditag Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ditag Feature Adaptor object.
**
** @fdata [EnsPDitagfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Feature Adaptor
**
** @argrule * Pdtfa [EnsPDitagfeatureadaptor*]
** Ensembl Ditag Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDitagfeatureadaptorDel ********************************************
**
** Default destructor for an Ensembl Ditag Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pdtfa [EnsPDitagfeatureadaptor*]
** Ensembl Ditag Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensDitagfeatureadaptorDel(EnsPDitagfeatureadaptor *Pdtfa)
{
    ensFeatureadaptorDel(Pdtfa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Ditag Feature Adaptor object.
**
** @fdata [EnsPDitagfeatureadaptor]
**
** @nam3rule Get Return Ensembl Ditag Feature Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDitagfeatureadaptorGetDatabaseadaptor *****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Ditag Feature Adaptor.
**
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDitagfeatureadaptorGetDatabaseadaptor(
    EnsPDitagfeatureadaptor dtfa)
{
    return ensFeatureadaptorGetDatabaseadaptor(dtfa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Ditag Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDitagfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Ditag Feature object(s)
** @nam4rule All Fetch all Ensembl Ditag Feature objects
** @nam4rule Allby Fetch all Ensembl Ditag Feature objects
**                 matching a criterion
** @nam5rule Ditag Fetch all by an Ensembl Ditag
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Type  Fetch all by a type
** @nam4rule By Fetch one Ensembl Ditag Feature object matching a criterion
** @nam5rule ByIdentifier Fetch by an SQL database-internal identifier
**
** @argrule * dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @argrule All dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
** @argrule AllbyDitag dt [const EnsPDitag] Ensembl Ditag
** @argrule AllbyDitag dtfs [AjPList]
** AJAX List of Ensembl Ditag Feature objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice type [const AjPStr] Ensembl Ditag type
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice dtfs [AjPList]
** AJAX List of Ensembl Ditag Feature objects
** @argrule AllbyType type [const AjPStr] Ensembl Ditag type
** @argrule AllbyType dtfs [AjPList]
** AJAX List of Ensembl Ditag Feature objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Pdtf [EnsPDitagfeature*]
** Ensembl Ditag Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDitagfeatureadaptorFetchAllbyDitag ********************************
**
** Fetch all Ensembl Ditag Feature objects by an Ensembl Ditag.
**
** The caller is responsible for deleting the Ensembl Ditag Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::fetch_all_by_ditagID
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [r] dt [const EnsPDitag] Ensembl Ditag
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllbyDitag(
    EnsPDitagfeatureadaptor dtfa,
    const EnsPDitag dt,
    AjPList dtfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!dtfa)
        return ajFalse;

    if (!dt)
        return ajFalse;

    if (!dtfs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dtfa);

    constraint = ajFmtStr("ditag_feature.ditag_id = %u", dt->Identifier);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                dtfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDitagfeatureadaptorFetchAllbySlice ********************************
**
** Fetch all Ensembl Ditag Feature objects by an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Ditag Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::fetch_all_by_Slice
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] type [const AjPStr] Ensembl Ditag type
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllbySlice(
    EnsPDitagfeatureadaptor dtfa,
    EnsPSlice slice,
    const AjPStr type,
    const AjPStr anname,
    AjPList dtfs)
{
    char *txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dtfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!dtfs)
        return ajFalse;

    if (type && ajStrGetLen(type))
    {
        ensFeatureadaptorEscapeC(dtfa, &txttype, type);

        constraint = ajFmtStr("ditag.type = '%s'", txttype);

        ajCharDel(&txttype);
    }

    result = ensFeatureadaptorFetchAllbySlice(dtfa,
                                              slice,
                                              constraint,
                                              anname,
                                              dtfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDitagfeatureadaptorFetchAllbyType *********************************
**
** Fetch all Ensembl Ditag Feature objects by an Ensembl Ditag type.
**
** The caller is responsible for deleting the Ensembl Ditag Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::fetch_all
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::fetch_all_by_type
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [rN] type [const AjPStr] Ensembl Ditag type
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllbyType(
    EnsPDitagfeatureadaptor dtfa,
    const AjPStr type,
    AjPList dtfs)
{
    char *txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!dtfa)
        return ajFalse;

    if (!(type && ajStrGetLen(type)))
        return ajFalse;

    if (!dtfs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dtfa);

    ensBaseadaptorEscapeC(ba, &txttype, type);

    /*
    ** NOTE: For this query the ditag_feature and the ditag tables need
    ** to be permanently joined.
    */

    constraint = ajFmtStr("ditag.type = '%s'", txttype);

    ajCharDel(&txttype);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                dtfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDitagfeatureadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Ditag Feature via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::fetch_by_dbID
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchByIdentifier(
    EnsPDitagfeatureadaptor dtfa,
    ajuint identifier,
    EnsPDitagfeature *Pdtf)
{
    EnsPBaseadaptor ba = NULL;

    if (!dtfa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pdtf)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dtfa);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void **) Pdtf);
}




#if AJFALSE

AjBool ensDitagFeatureadaptorFetchPairsBySlice(
    EnsPDitagfeatureadaptor dtfa,
    EnsPSlice slice,
    const AjPStr type,
    const AjPStr anname,
    AjPList dtfs)
{
    char *txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dtfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!dtfs)
        return ajFalse;

    if (type && ajStrGetLen(type))
    {
        ensFeatureadaptorEscapeC(dtfa, &txttype, type);

        constraint = ajFmtStr("ditag.type = '%s'", txttype);

        ajCharDel(&txttype);
    }

    /*
    ** FIXME: This would be problematic in multi-threaded environments.
    ** ensFeatureadaptorSetFinalcondition(dtfa->Adaptor, const char *final)
    **
    ** TODO: Maybe this could be solved by having two EnsPFeatureadaptor
    ** objects wrapped up under this EnsPDitagfeatureadaptor.
    ** One adaptor could implement the simple join condition, while the
    ** other one could implement the extended join. Could the same be
    ** applied to the Exon Adaptor? A problem that could remain is that
    ** the two Feature Adaptors have separate Feature caches.
    */

    result = ensFeatureadaptorFetchAllbySlice(dtfa,
                                              slice,
                                              constraint,
                                              anname,
                                              dtfs);

    ajStrDel(&constraint);

    /*
    ** FIXME: Finish this. Since it is not simply possible to append a
    ** GROUP BY df.ditag_id, df.ditag_pair_id condition, the results
    ** would need to be calculated by means of AJAX List and AJAX Table
    ** objects here. It would be possible to append the condition via
    ** ensFeatureadaptorSetFinalcondition(dtfa->Adaptor, const char *final),
    ** but this would be problematic in a multi-threaded environment.
    ** Could a separate EnsPDitagFeaturepairadaptor be the solution?
    ** A similar prolem exists for the EnsPExonadaptor and the
    ** ExonTranscript Adaptor ...
    */

    return result;
}

#endif /* AJFALSE */




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Ditag Feature objects
** from an Ensembl SQL database.
**
** @fdata [EnsPDitagfeatureadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Ditag Feature-releated object(s)
** @nam4rule All Retrieve all Ensembl Ditag Feature-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
**
** @argrule * dtfa [EnsPDitagfeatureadaptor]
** Ensembl Ditag Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDitagfeatureadaptorRetrieveAllIdentifiers *************************
**
** Retrieve all SQL database-internal identifiers of
** Ensembl Ditag Feature objects.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::DitagFeatureAdaptor::list_dbIDs
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorRetrieveAllIdentifiers(
    EnsPDitagfeatureadaptor dtfa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!dtfa)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dtfa);

    table = ajStrNewC("ditag_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}

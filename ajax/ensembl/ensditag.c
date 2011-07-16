/* @source Ensembl Ditag functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.41 $
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

/* @conststatic ditagadaptorTables ********************************************
**
** Array of Ensembl Ditag Adaptor SQL table names
**
******************************************************************************/

static const char* const ditagadaptorTables[] =
{
    "ditag",
    (const char*) NULL
};




/* @conststatic ditagadaptorColumns *******************************************
**
** Array of Ensembl Ditag Adaptor SQL column names
**
******************************************************************************/

static const char* const ditagadaptorColumns[] =
{
    "ditag.ditag_id",
    "ditag.name",
    "ditag.type",
    "ditag.tag_count",
    "ditag.sequence",
    (const char*) NULL
};




/* @conststatic ditagfeatureSide **********************************************
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

static const char* const ditagfeatureSide[] =
{
    "",
    "L",
    "R",
    "F",
    (const char*) NULL
};




/* @conststatic ditagfeatureadaptorTables *************************************
**
** Array of Ensembl Ditag Feature Adaptor SQL table names
**
** NOTE: For now, the ditag_feature and ditag tables are permanently joined to
** allow for selection of Ditag Feature objects on the basis of a Ditag type.
**
******************************************************************************/

static const char* const ditagfeatureadaptorTables[] =
{
    "ditag_feature",
    "ditag",
    (const char*) NULL
};




/* @conststatic ditagfeatureadaptorColumns ************************************
**
** Array of Ensembl Ditag Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const ditagfeatureadaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic ditagfeatureadaptorDefaultcondition ***************************
**
** Ensembl Ditag Feature Adaptor default SQL condition
**
******************************************************************************/

static const char* ditagfeatureadaptorDefaultcondition =
    "ditag_feature.ditag_id = ditag.ditag_id";




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void tableDitagClear(void** key,
                            void** value,
                            void* cl);

static void ditagfeatureadaptorLinkDitag(void** key,
                                         void** value,
                                         void* cl);

static AjBool ditagadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dts);

static int listDitagfeatureCompareStartAscending(const void* P1,
                                                 const void* P2);

static int listDitagfeatureCompareStartDescending(const void* P1,
                                                  const void* P2);

static AjBool ditagfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dtfs);

static void* ditagfeatureadaptorCacheReference(void* value);

static void ditagfeatureadaptorCacheDelete(void** value);

static size_t ditagfeatureadaptorCacheSize(const void* value);

static EnsPFeature ditagfeatureadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-62
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
** @@
******************************************************************************/

EnsPDitag ensDitagNewCpy(const EnsPDitag dt)
{
    EnsPDitag pthis = NULL;

    if(!dt)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1;
    pthis->Identifier = dt->Identifier;
    pthis->Adaptor    = dt->Adaptor;

    if(dt->Name)
        pthis->Name = ajStrNewRef(dt->Name);

    if(dt->Type)
        pthis->Type = ajStrNewRef(dt->Type);

    if(dt->Sequence)
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

    dt->Use        = 1;
    dt->Identifier = identifier;
    dt->Adaptor    = dta;

    if(name)
        dt->Name = ajStrNewRef(name);

    if(type)
        dt->Type = ajStrNewRef(type);

    if(sequence)
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
** memory allocated for an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
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
** @param [d] Pdt [EnsPDitag*] Ensembl Ditag object address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagDel(EnsPDitag* Pdt)
{
    EnsPDitag pthis = NULL;

    if(!Pdt)
        return;

    if(!*Pdt)
        return;

    if(ajDebugTest("ensDitagDel"))
    {
        ajDebug("ensDitagDel\n"
                "  *Pdt %p\n",
                *Pdt);

        ensDitagTrace(*Pdt, 1);
    }

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

    AJFREE(pthis);

    *Pdt = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Ditag object.
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
** @valrule Count [ajuint] Count or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Sequence [AjPStr] Sequence or NULL
** @valrule Type [AjPStr] Type or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDitagGetAdaptor ***************************************************
**
** Get the Ensembl Ditag Adaptor element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [EnsPDitagadaptor] Ensembl Ditag Adaptor or NULL
** @@
******************************************************************************/

EnsPDitagadaptor ensDitagGetAdaptor(const EnsPDitag dt)
{
    if(!dt)
        return NULL;

    return dt->Adaptor;
}




/* @func ensDitagGetCount *****************************************************
**
** Get the count element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::tag_count
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] Count or 0
** @@
******************************************************************************/

ajuint ensDitagGetCount(const EnsPDitag dt)
{
    if(!dt)
        return 0;

    return dt->Count;
}




/* @func ensDitagGetIdentifier ************************************************
**
** Get the SQL database-internal identifier element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [ajuint] SQL database-internal identifier or 0
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
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensDitagGetName(const EnsPDitag dt)
{
    if(!dt)
        return NULL;

    return dt->Name;
}




/* @func ensDitagGetSequence **************************************************
**
** Get the sequence element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::sequence
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Sequence or NULL
** @@
******************************************************************************/

AjPStr ensDitagGetSequence(const EnsPDitag dt)
{
    if(!dt)
        return NULL;

    return dt->Sequence;
}




/* @func ensDitagGetType ******************************************************
**
** Get the type element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Map::Ditag::type
** @param [r] dt [const EnsPDitag] Ensembl Ditag
**
** @return [AjPStr] Type or NULL
** @@
******************************************************************************/

AjPStr ensDitagGetType(const EnsPDitag dt)
{
    if(!dt)
        return NULL;

    return dt->Type;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
** @nam3rule Set Set one element of an Ensembl Ditag
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
** Set the Ensembl Ditag Adaptor element of an Ensembl Ditag.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dt [EnsPDitag] Ensembl Ditag
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagSetAdaptor(EnsPDitag dt, EnsPDitagadaptor dta)
{
    if(!dt)
        return ajFalse;

    dt->Adaptor = dta;

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




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ditag object.
**
** @fdata [EnsPDitag]
**
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
** @@
******************************************************************************/

size_t ensDitagCalculateMemsize(const EnsPDitag dt)
{
    size_t size = 0;

    if(!dt)
        return 0;

    size += sizeof (EnsODitag);

    if(dt->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dt->Name);
    }

    if(dt->Type)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dt->Type);
    }

    if(dt->Sequence)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(dt->Sequence);
    }

    return size;
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Table Functions for manipulating AJAX Table objects
**
******************************************************************************/




/* @section table *************************************************************
**
** Functions for manipulating AJAX Table objects.
**
** @fdata [AjPTable]
**
** @nam3rule Ditag AJAX Table of AJAX unsigned integer key data and
**                Ensembl Ditag value data
** @nam4rule Clear Clear an AJAX Table
** @nam4rule Delete Delete an AJAX Table
**
** @argrule Clear table [AjPTable] AJAX Table
** @argrule Delete Ptable [AjPTable*] AJAX Table address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic tableDitagClear ************************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Ditag value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Ditag address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableDitagClear(void** key,
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

    ensDitagDel((EnsPDitag*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableDitagClear ***************************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Ditag value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableDitagClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableDitagClear, NULL);

    return ajTrue;
}




/* @func ensTableDitagDelete **************************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Ditag value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableDitagDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableDitagClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [EnsPDitagadaptor] Ensembl Ditag Adaptor **********************
**
** @nam2rule Ditagadaptor Functions for manipulating
** Ensembl Ditag Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic ditagadaptorFetchAllbyStatement ********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Ditag objects.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor::_fetch
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] dts [AjPList] AJAX List of Ensembl Ditag objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ditagadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dts)
{
    ajuint identifier = 0;
    ajuint count      = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name     = NULL;
    AjPStr type     = NULL;
    AjPStr sequence = NULL;

    EnsPDitag dt         = NULL;
    EnsPDitagadaptor dta = NULL;

    if(ajDebugTest("ditagadaptorFetchAllbyStatement"))
        ajDebug("ditagadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  dts %p\n",
                dba,
                statement,
                am,
                slice,
                dts);

    if(!dba)
        return ajFalse;

    if(!(statement && ajStrGetLen(statement)))
        return ajFalse;

    if(!dts)
        return ajFalse;

    dta = ensRegistryGetDitagadaptor(dba);

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

        dt = ensDitagNewIni(dta, identifier, name, type, sequence, count);

        ajListPushAppend(dts, (void*) dt);

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
** @valrule * [EnsPDitagadaptor] Ensembl Ditag Adaptor
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
** @@
******************************************************************************/

EnsPDitagadaptor ensDitagadaptorNew(EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    if(ajDebugTest("ensDitagadaptorNew"))
        ajDebug("ensDitagadaptorNew\n"
                "  dba %p\n",
                dba);

    return ensBaseadaptorNew(
        dba,
        ditagadaptorTables,
        ditagadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        ditagadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Ditag Adaptor object.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Adaptor object.
**
** @argrule * Pdta [EnsPDitagadaptor*] Ensembl Ditag Adaptor object address
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
** @param [d] Pdta [EnsPDitagadaptor*] Ensembl Ditag Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagadaptorDel(EnsPDitagadaptor* Pdta)
{
    EnsPDitagadaptor pthis = NULL;

    if(!Pdta)
        return;

    if(!*Pdta)
        return;

    pthis = *Pdta;

    ensBaseadaptorDel(&pthis);

    *Pdta = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Ditag Adaptor object.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule Get Return Ensembl Ditag Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensDitagadaptorGetDatabaseadaptor ************************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Ditag Adaptor.
** The Ensembl Ditag Adaptor is just an alias for the
** Ensembl Database Adaptor.
**
** @param [u] dta [EnsPDitagadaptor] Ensembl Ditag Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDitagadaptorGetDatabaseadaptor(EnsPDitagadaptor dta)
{
    if(!dta)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(dta);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Ditag objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDitagadaptor]
**
** @nam3rule Fetch Fetch Ensembl Ditag object(s)
** @nam4rule All   Fetch all Ensembl Ditag objects
** @nam4rule Allby Fetch all Ensembl Ditag objects matching a criterion
** @nam5rule Identifiers Fetch all by an AJAX Table
** @nam5rule Name Fetch by a name
** @nam5rule Type Fetch by a type
** @nam4rule By    Fetch one Ensembl Ditag object matching a criterion
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
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAll(EnsPDitagadaptor dta,
                               const AjPStr name,
                               const AjPStr type,
                               AjPList dts)
{
    char* txtname = NULL;
    char* txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!dta)
        return ajFalse;

    if(!dts)
        return ajFalse;

    if(name && ajStrGetLen(name))
        ensBaseadaptorEscapeC(dta, &txtname, name);

    if(type && ajStrGetLen(type))
        ensBaseadaptorEscapeC(dta, &txttype, type);

    if(txtname || txttype)
        constraint = ajStrNew();

    if(txtname)
        ajFmtPrintAppS(&constraint, "ditag.name = '%s'", txtname);

    if(txttype)
    {
        if(txtname)
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
** @param [u] dts [AjPTable] AJAX Table of AJAX unsigned integer identifier
**                           key data and Ensembl Ditag value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllbyIdentifiers(EnsPDitagadaptor dta,
                                            AjPTable dts)
{
    void** keyarray = NULL;

    const char* template = "ditag.ditag_id IN (%S)";

    register ajuint i = 0;

    ajuint identifier = 0;

    ajuint* Pidentifier = NULL;

    AjPList ldts = NULL;

    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    EnsPDitag dt = NULL;

    if(!dta)
        return ajFalse;

    if(!dts)
        return ajFalse;

    ldts = ajListNew();

    csv = ajStrNew();

/*
** Large queries are split into smaller ones on the basis of the maximum
** number of identifiers configured in the Ensembl Base Adaptor module.
** This ensures that MySQL is faster and the maximum query size is not
** exceeded.
*/

    ajTableToarrayKeys(dts, &keyarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint*) keyarray[i]));

        /* Run the statement if the maximum chunk size is exceed. */

        if(((i + 1) % ensBaseadaptorMaximumIdentifiers) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            constraint = ajFmtStr(template, csv);

            ensBaseadaptorFetchAllbyConstraint(dta,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               ldts);

            ajStrDel(&constraint);

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

/* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
    {
        constraint = ajFmtStr(template, csv);

        ensBaseadaptorFetchAllbyConstraint(dta,
                                           constraint,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           ldts);

        ajStrDel(&constraint);
    }

    ajStrDel(&csv);

/* Move Ensembl Ditag objects from the AJAX List to the AJAX Table. */

    while(ajListPop(ldts, (void**) &dt))
    {
        identifier = ensDitagGetIdentifier(dt);

        if(ajTableMatchV(dts, (const void*) &identifier))
            ajTablePut(dts, (void*) &identifier, (void*) dt);
        else
        {
            /*
            ** This should not happen, because the keys should have been in
            ** the AJAX Table in the first place.
            */

            AJNEW0(Pidentifier);

            *Pidentifier = ensDitagGetIdentifier(dt);

            ajTablePut(dts, (void*) Pidentifier, (void*) dt);
        }
    }

    ajListFree(&ldts);

    return ajTrue;
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
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllbyName(EnsPDitagadaptor dta,
                                     const AjPStr name,
                                     AjPList dts)
{
    char* txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!dta)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!dts)
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
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchAllbyType(EnsPDitagadaptor dta,
                                     const AjPStr type,
                                     AjPList dts)
{
    char* txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!dta)
        return ajFalse;

    if(!type)
        return ajFalse;

    if(!dts)
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
** @@
******************************************************************************/

AjBool ensDitagadaptorFetchByIdentifier(EnsPDitagadaptor dta,
                                        ajuint identifier,
                                        EnsPDitag* Pdt)
{
    if(!dta)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pdt)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(dta, identifier, (void**) Pdt);
}




/* @datasection [EnsPDitagfeature] Ensembl Ditag Feature **********************
**
** @nam2rule Ditagfeature Functions for manipulating
** Ensembl Ditag Feature objects
**
** @cc Bio::EnsEMBL::Map::DitagFeature
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-62
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
** @@
******************************************************************************/

EnsPDitagfeature ensDitagfeatureNewCpy(const EnsPDitagfeature dtf)
{
    EnsPDitagfeature pthis = NULL;

    if(!dtf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = dtf->Identifier;
    pthis->Adaptor    = dtf->Adaptor;
    pthis->Feature    = ensFeatureNewRef(dtf->Feature);
    pthis->Ditag      = ensDitagNewRef(dtf->Ditag);

    if(dtf->Cigar)
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

    if(!feature)
        return NULL;

    if((tstrand < -1) || (tstrand > 1))
    {
        ajDebug("ensDitagfeatureNewIni got target strand not -1, 0 or +1.\n");

        return NULL;
    }

    AJNEW0(dtf);

    dtf->Use        = 1;
    dtf->Identifier = identifier;
    dtf->Adaptor    = dtfa;
    dtf->Feature    = ensFeatureNewRef(feature);
    dtf->Ditag      = ensDitagNewRef(dt);

    if(cigar)
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
** memory allocated for an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
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
** @param [d] Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature object address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagfeatureDel(EnsPDitagfeature* Pdtf)
{
    EnsPDitagfeature pthis = NULL;

    if(!Pdtf)
        return;

    if(!*Pdtf)
        return;

    if(ajDebugTest("ensDitagfeatureDel"))
    {
        ajDebug("ensDitagfeatureDel\n"
                "  *Pdtf %p\n",
                *Pdtf);

        ensDitagfeatureTrace(*Pdtf, 1);
    }

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
**
** @nam3rule Get Return Ditag Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Ditag Feature Adaptor
** @nam4rule Cigar Return the CIGAR line
** @nam4rule Ditag Return the Ensembl Ditag
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Pairidentifier Return the pair identifier
** @nam4rule Side Return the side
** @nam4rule Target Return target elements
** @nam5rule End Return the target end
** @nam5rule Start Return the target start
** @nam5rule Strand Return the target strand
**
** @argrule * dtf [const EnsPDitagfeature] Ditag Feature
**
** @valrule Adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** or NULL
** @valrule Cigar [AjPStr] CIGAR line or NULL
** @valrule Ditag [EnsPDitag] Ensembl Ditag or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Pairidentifier [ajuint] Pair identifier or 0
** @valrule Side [EnsEDitagfeatureSide] Side or ensEDitagfeatureSideNULL
** @valrule TargetEnd [ajint] Target end or 0
** @valrule TargetStart [ajint] Target start or 0
** @valrule TargetStrand [ajint] Target strand or 0
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
** @return [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensDitagfeatureGetAdaptor(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;

    return dtf->Adaptor;
}




/* @func ensDitagfeatureGetCigar **********************************************
**
** Get the CIGAR line element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::cigar_line
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [AjPStr] CIGAR line or NULL
** @@
******************************************************************************/

AjPStr ensDitagfeatureGetCigar(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;

    return dtf->Cigar;
}




/* @func ensDitagfeatureGetDitag **********************************************
**
** Get the Ensembl Ditag element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPDitag] Ensembl Ditag or NULL
** @@
******************************************************************************/

EnsPDitag ensDitagfeatureGetDitag(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;

    return dtf->Ditag;
}




/* @func ensDitagfeatureGetFeature ********************************************
**
** Get the Ensembl Feature element of an Ensembl Ditag Feature.
**
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensDitagfeatureGetFeature(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return NULL;

    return dtf->Feature;
}




/* @func ensDitagfeatureGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensDitagfeatureGetIdentifier(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;

    return dtf->Identifier;
}




/* @func ensDitagfeatureGetPairidentifier *************************************
**
** Get the pair identifier element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_pair_id
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajuint] Pair identifier or 0
** @@
******************************************************************************/

ajuint ensDitagfeatureGetPairidentifier(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;

    return dtf->Pairidentifier;
}




/* @func ensDitagfeatureGetSide ***********************************************
**
** Get the side element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_side
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [EnsEDitagfeatureSide] Side or ensEDitagfeatureSideNULL
** @@
******************************************************************************/

EnsEDitagfeatureSide ensDitagfeatureGetSide(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return ensEDitagfeatureSideNULL;

    return dtf->Side;
}




/* @func ensDitagfeatureGetTargetEnd ******************************************
**
** Get the target end element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_end
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target end or 0
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetEnd(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;

    return dtf->TargetEnd;
}




/* @func ensDitagfeatureGetTargetStart ****************************************
**
** Get the target start element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_start
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target start or 0
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetStart(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;

    return dtf->TargetStart;
}




/* @func ensDitagfeatureGetTargetStrand ***************************************
**
** Get the target strand element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::hit_strand
** @param [r] dtf [const EnsPDitagfeature] Ensembl Ditag Feature
**
** @return [ajint] Target strand or 0
** @@
******************************************************************************/

ajint ensDitagfeatureGetTargetStrand(const EnsPDitagfeature dtf)
{
    if(!dtf)
        return 0;

    return dtf->TargetStrand;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
** @nam3rule Set Set one element of an Ensembl Ditag Feature
** @nam4rule Adaptor Set the Ensembl Ditag Feature Adaptor
** @nam4rule Cigar Set the CIGAR line
** @nam4rule Ditag Set the Ensembl Ditag
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Pairidentifier Set the pair identifier
** @nam4rule Side Set the side
** @nam4rule Target Set target elements
** @nam5rule End Set the target end
** @nam5rule Start Set the target start
** @nam5rule Strand Set the target strand
**
** @argrule * dtf [EnsPDitagfeature] Ensembl Ditag Feature object
** @argrule Adaptor dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag
**                                                 Feature Adaptor
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
** Set the Ensembl Ditag Feature Adaptor element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetAdaptor(EnsPDitagfeature dtf,
                                 EnsPDitagfeatureadaptor dtfa)
{
    if(!dtf)
        return ajFalse;

    dtf->Adaptor = dtfa;

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

    ensDitagDel(&dtf->Ditag);

    dtf->Ditag = ensDitagNewRef(dt);

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

    ensFeatureDel(&dtf->Feature);

    dtf->Feature = ensFeatureNewRef(feature);

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




/* @func ensDitagfeatureSetPairidentifier *************************************
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

AjBool ensDitagfeatureSetPairidentifier(EnsPDitagfeature dtf, ajuint pairid)
{
    if(!dtf)
        return ajFalse;

    dtf->Pairidentifier = pairid;

    return ajTrue;
}




/* @func ensDitagfeatureSetSide ***********************************************
**
** Set the side element of an Ensembl Ditag Feature.
**
** @cc Bio::EnsEMBL::Map::DitagFeature::ditag_side
** @param [u] dtf [EnsPDitagfeature] Ensembl Ditag Feature
** @param [u] side [EnsEDitagfeatureSide] Side
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDitagfeatureSetSide(EnsPDitagfeature dtf, EnsEDitagfeatureSide side)
{
    if(!dtf)
        return ajFalse;

    dtf->Side = side;

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




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ditag Feature object.
**
** @fdata [EnsPDitagfeature]
**
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
** @@
******************************************************************************/

size_t ensDitagfeatureCalculateMemsize(const EnsPDitagfeature dtf)
{
    size_t size = 0;

    if(!dtf)
        return 0;

    size += sizeof (EnsODitagfeature);

    size += ensFeatureCalculateMemsize(dtf->Feature);

    size += ensDitagCalculateMemsize(dtf->Ditag);

    if(dtf->Cigar)
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
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  side [const AjPStr] Side string
**
** @valrule * [EnsEDitagfeatureSide] Ensembl Ditag Feature Side or
**                                   ensEDitagfeatureSideNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensDitagfeatureSideFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Ditag Feature Side enumeration.
**
** @param [r] side [const AjPStr] Ensembl Ditag Feature Side string
**
** @return [EnsEDitagfeatureSide] Ensembl Ditag Feature Side or
**                                ensEDitagfeatureSideNULL
** @@
******************************************************************************/

EnsEDitagfeatureSide ensDitagfeatureSideFromStr(const AjPStr side)
{
    register EnsEDitagfeatureSide i = ensEDitagfeatureSideNULL;

    EnsEDitagfeatureSide eside = ensEDitagfeatureSideNULL;

    for(i = ensEDitagfeatureSideNULL;
        ditagfeatureSide[i];
        i++)
        if(ajStrMatchC(side, ditagfeatureSide[i]))
            eside = i;

    if(!eside)
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
** @argrule To dtfs [EnsEDitagfeatureSide] Ensembl Ditag Feature Side
**                                         enumeration
**
** @valrule Char [const char*] Ensembl Ditag Feature Side C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensDitagfeatureSideToChar ********************************************
**
** Convert an Ensembl Ditag Feature Side enumeration into
** a C-type (char*) string.
**
** @param [u] dtfs [EnsEDitagfeatureSide] Ensembl Ditag Feature Side
**                                        enumeration
**
** @return [const char*] Ensembl Ditag Feature Side C-type (char*) string
** @@
******************************************************************************/

const char* ensDitagfeatureSideToChar(EnsEDitagfeatureSide dtfs)
{
    register EnsEDitagfeatureSide i = ensEDitagfeatureSideNULL;

    for(i = ensEDitagfeatureSideNULL;
        ditagfeatureSide[i] && (i < dtfs);
        i++);

    if(!ditagfeatureSide[i])
        ajDebug("ensDitagfeatureSideToChar encountered an "
                "out of boundary error on "
                "Ensembl Ditag Feature Side enumeration %d.\n",
                dtfs);

    return ditagfeatureSide[i];
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Ditagfeature Functions for manipulating AJAX List objects of
** Ensembl Ditag Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending dtfs [AjPList] AJAX List of Ensembl Ditag Feature
**                                   objects
** @argrule Descending dtfs [AjPList] AJAX List of Ensembl Ditag Feature
**                                    objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listDitagfeatureCompareStartAscending **************************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Ditag Feature address 1
** @param [r] P2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listDitagfeatureCompareStartAscending(const void* P1,
                                                 const void* P2)
{
    const EnsPDitagfeature dtf1 = NULL;
    const EnsPDitagfeature dtf2 = NULL;

    dtf1 = *(EnsPDitagfeature const*) P1;
    dtf2 = *(EnsPDitagfeature const*) P2;

    if(ajDebugTest("listDitagfeatureCompareStartAscending"))
        ajDebug("listDitagfeatureCompareStartAscending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);

/* Sort empty values towards the end of the AJAX List. */

    if(dtf1 && (!dtf2))
        return -1;

    if((!dtf1) && (!dtf2))
        return 0;

    if((!dtf1) && dtf2)
        return +1;

    return ensFeatureCompareStartAscending(dtf1->Feature, dtf2->Feature);
}




/* @func ensListDitagfeatureSortStartAscending ********************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** Ensembl Feature start element in ascending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortStartAscending(AjPList dtfs)
{
    if(!dtfs)
        return ajFalse;

    ajListSort(dtfs, listDitagfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listDitagfeatureCompareStartDescending *************************
**
** AJAX List of Ensembl Ditag Feature objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Ditag Feature address 1
** @param [r] P2 [const void*] Ensembl Ditag Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listDitagfeatureCompareStartDescending(const void* P1,
                                                  const void* P2)
{
    const EnsPDitagfeature dtf1 = NULL;
    const EnsPDitagfeature dtf2 = NULL;

    dtf1 = *(EnsPDitagfeature const*) P1;
    dtf2 = *(EnsPDitagfeature const*) P2;

    if(ajDebugTest("listDitagfeatureCompareStartDescending"))
        ajDebug("listDitagfeatureCompareStartDescending\n"
                "  dtf1 %p\n"
                "  dtf2 %p\n",
                dtf1,
                dtf2);

/* Sort empty values towards the end of the AJAX List. */

    if(dtf1 && (!dtf2))
        return -1;

    if((!dtf1) && (!dtf2))
        return 0;

    if((!dtf1) && dtf2)
        return +1;

    return ensFeatureCompareStartDescending(dtf1->Feature, dtf2->Feature);
}




/* @func ensListDitagfeatureSortStartDescending *******************************
**
** Sort an AJAX List of Ensembl Ditag Feature objects by their
** Ensembl Feature start element in descending order.
**
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListDitagfeatureSortStartDescending(AjPList dtfs)
{
    if(!dtfs)
        return ajFalse;

    ajListSort(dtfs, listDitagfeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor *******
**
** @nam2rule Ditagfeatureadaptor Functions for manipulating
** Ensembl Ditag Feature Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor
** @cc CVS Revision: 1.20
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic ditagfeatureadaptorLinkDitag ***********************************
**
** An ajTableMapDel "apply" function to link Ensembl Ditag Feature objects to
** Ensembl Ditag objects.
** This function also deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Ditag Feature objects after association.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX Lists of Ensembl Ditag Feature objects
** @param [u] cl [void*] AJAX Table of Ensembl Ditag objects,
**                       passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void ditagfeatureadaptorLinkDitag(void** key,
                                         void** value,
                                         void* cl)
{
    EnsPDitag dt = NULL;

    EnsPDitagfeature dtf = NULL;

    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    dt = (EnsPDitag) ajTableFetchmodV(cl, *key);

/*
** The Ensembl Ditag Feature objects can be deleted after associating
** them with Ensembl Ditag Feature objects, because this AJAX Table holds
** independent references for these objects.
*/

    while(ajListPop(*((AjPList*) value), (void**) &dtf))
    {
        ensDitagfeatureSetDitag(dtf, dt);

        ensDitagfeatureDel(&dtf);
    }

    AJFREE(*key);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic ditagfeatureadaptorFetchAllbyStatement *************************
**
** Fetch all Ensembl Ditag Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::Map::DBSQL::DitagFeatureAdaptor::_fetch
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] dtfs [AjPList] AJAX List of Ensembl Ditag Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ditagfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
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

    ajuint* Pidentifier = NULL;

    EnsEDitagfeatureSide eside = ensEDitagfeatureSideNULL;

    AjPList list = NULL;
    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr cigar = NULL;
    AjPStr side  = NULL;

    AjPTable dts      = NULL;
    AjPTable dttodtfs = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPDitagadaptor dta = NULL;

    EnsPDitagfeature dtf         = NULL;
    EnsPDitagfeatureadaptor dtfa = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("ditagfeatureadaptorFetchAllbyStatement"))
        ajDebug("ditagfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  dtfs %p\n",
                dba,
                statement,
                am,
                slice,
                dtfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!dtfs)
        return ajFalse;

    dts      = ensTableuintNewLen(0);
    dttodtfs = ensTableuintNewLen(0);

    aa = ensRegistryGetAnalysisadaptor(dba);

    dta = ensRegistryGetDitagadaptor(dba);

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
        ** coordinates as unsigned integers for all Feature objects, the range
        ** needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("ditagfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("ditagfeatureadaptorFetchAllbyStatement got a "
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

        if(am)
            am = ensAssemblymapperNewRef(am);
        else if(slice && (!ensCoordsystemMatch(
                              ensSliceGetCoordsystemObject(slice),
                              ensSliceGetCoordsystemObject(srslice))))
            ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice, &am);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper is defined at this point.
        */

        if(am)
        {
            ensAssemblymapperFastmap(
                am,
                ensSliceGetSeqregion(srslice),
                slstart,
                slend,
                slstrand,
                mrs);

            /*
            ** The ensAssemblymapperFastmap function returns at best one
            ** Ensembl Mapper Result.
            */

            ajListPop(mrs, (void**) &mr);

            /*
            ** Skip Feature objects that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&cigar);
                ajStrDel(&side);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                ensMapperresultDel(&mr);

                continue;
            }

            srid     = ensMapperresultGetObjectidentifier(mr);
            slstart  = ensMapperresultGetCoordinateStart(mr);
            slend    = ensMapperresultGetCoordinateEnd(mr);
            slstrand = ensMapperresultGetCoordinateStrand(mr);

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

            if(ensSliceCalculateLength(slice) <= INT_MAX)
                sllength = (ajint) ensSliceCalculateLength(slice);
            else
                ajFatal("ditagfeatureadaptorFetchAllbyStatement got a "
                        "Slice, which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceCalculateLength(slice), INT_MAX);

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
            ** Throw away Feature objects off the end of the requested Slice
            ** or on any other than the requested Slice.
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

        feature = ensFeatureNewIniS(analysis,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

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

        ajListPushAppend(dtfs, (void*) dtf);

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

        if(!ajTableMatchV(dts, (const void*) &dtid))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = dtid;

            ajTablePut(dts, (void*) Pidentifier, (void*) NULL);
        }

        list = (AjPList) ajTableFetchmodV(dttodtfs, (const void*) &dtid);

        if(!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = dtid;

            list = ajListNew();

            ajTablePut(dttodtfs,
                       (void*) Pidentifier,
                       (void*) list);
        }

        ajListPushAppend(list, (void*) ensDitagfeatureNewRef(dtf));

        ensFeatureDel(&feature);

        ensAnalysisDel(&analysis);

        ajStrDel(&cigar);
        ajStrDel(&side);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    ensDitagadaptorFetchAllbyIdentifiers(dta, dts);

/* Associate Ensembl Ditag objects with Ensembl Ditag Feature objects. */

    ajTableMapDel(dttodtfs, ditagfeatureadaptorLinkDitag, (void*) dts);

    ajTableFree(&dttodtfs);

/* Delete the utility AJAX Table object. */

    ensTableDitagDelete(&dts);

    return ajTrue;
}




/* @funcstatic ditagfeatureadaptorCacheReference ******************************
**
** Wrapper function to reference an Ensembl Ditag Feature
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Ditag Feature
**
** @return [void*] Ensembl Ditag Feature or NULL
** @@
******************************************************************************/

static void* ditagfeatureadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensDitagfeatureNewRef((EnsPDitagfeature) value);
}




/* @funcstatic ditagfeatureadaptorCacheDelete *********************************
**
** Wrapper function to delete an Ensembl Ditag Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Ditag Feature address
**
** @return [void]
** @@
******************************************************************************/

static void ditagfeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensDitagfeatureDel((EnsPDitagfeature*) value);

    return;
}




/* @funcstatic ditagfeatureadaptorCacheSize ***********************************
**
** Wrapper function to determine the memory size of an Ensembl Ditag Feature
** via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Ditag Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t ditagfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensDitagfeatureCalculateMemsize((const EnsPDitagfeature) value);
}




/* @funcstatic ditagfeatureadaptorGetFeature **********************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Ditag Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Ditag Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature ditagfeatureadaptorGetFeature(const void* value)
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
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
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
** @@
******************************************************************************/

EnsPDitagfeatureadaptor ensDitagfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        ditagfeatureadaptorTables,
        ditagfeatureadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        ditagfeatureadaptorDefaultcondition,
        (const char*) NULL,
        ditagfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        ditagfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        ditagfeatureadaptorCacheDelete,
        ditagfeatureadaptorCacheSize,
        ditagfeatureadaptorGetFeature,
        "Ditag Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Ditag Feature Adaptor object.
**
** @fdata [EnsPDitagfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Ditag Feature Adaptor object
**
** @argrule * Pdtfa [EnsPDitagfeatureadaptor*] Ensembl Ditag Feature Adaptor
**                                             object address
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
** @param [d] Pdtfa [EnsPDitagfeatureadaptor*] Ensembl Ditag Feature
**                                             Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensDitagfeatureadaptorDel(EnsPDitagfeatureadaptor* Pdtfa)
{
    EnsPDitagfeatureadaptor pthis = NULL;

    if(!Pdtfa)
        return;

    if(!*Pdtfa)
        return;

    pthis = *Pdtfa;

    ensFeatureadaptorDel(&pthis);

    *Pdtfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
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
** Get the Ensembl Database Adaptor element of an
** Ensembl Ditag Feature Adaptor.
**
** @param [u] dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDitagfeatureadaptorGetDatabaseadaptor(
    EnsPDitagfeatureadaptor dtfa)
{
    if(!dtfa)
        return NULL;

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
** @argrule AllbyDitag dtfs [AjPList] AJAX List of Ensembl Ditag Feature
**                                    objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice type [const AjPStr] Ensembl Ditag type
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice dtfs [AjPList] AJAX List of Ensembl Ditag Feature
**                                    objects
** @argrule AllbyType type [const AjPStr] Ensembl Ditag type
** @argrule AllbyType dtfs [AjPList] AJAX List of Ensembl Ditag Feature
**                                   objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pdtf [EnsPDitagfeature*] Ensembl Ditag Feature
**                                                address
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

    if(!dtfa)
        return ajFalse;

    if(!dt)
        return ajFalse;

    if(!dtfs)
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
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllbySlice(
    EnsPDitagfeatureadaptor dtfa,
    EnsPSlice slice,
    const AjPStr type,
    const AjPStr anname,
    AjPList dtfs)
{
    char* txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!dtfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!dtfs)
        return ajFalse;

    if(type && ajStrGetLen(type))
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
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchAllbyType(
    EnsPDitagfeatureadaptor dtfa,
    const AjPStr type,
    AjPList dtfs)
{
    char* txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!dtfa)
        return ajFalse;

    if(!(type && ajStrGetLen(type)))
        return ajFalse;

    if(!dtfs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dtfa);

    ensBaseadaptorEscapeC(ba, &txttype, type);

/*
** NOTE: For this query the ditag_feature and the ditag tables need to be
** permanently joined.
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
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorFetchByIdentifier(
    EnsPDitagfeatureadaptor dtfa,
    ajuint identifier,
    EnsPDitagfeature* Pdtf)
{
    EnsPBaseadaptor ba = NULL;

    if(!dtfa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pdtf)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dtfa);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void**) Pdtf);
}




#if AJFALSE

AjBool ensDitagFeatureadaptorFetchPairsBySlice(
    EnsPDitagfeatureadaptor dtfa,
    EnsPSlice slice,
    const AjPStr type,
    const AjPStr anname,
    AjPList dtfs)
{
    char* txttype = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!dtfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!dtfs)
        return ajFalse;

    if(type && ajStrGetLen(type))
    {
        ensFeatureadaptorEscapeC(dtfa, &txttype, type);

        constraint = ajFmtStr("ditag.type = '%s'", txttype);

        ajCharDel(&txttype);
    }

/*
** FIXME: This would be problematic in multi-threaded environments.
** ensFeatureadaptorSetFinalcondition(dtfa->Adaptor, const char* final)
**
** TODO: Maybe this could be solved by having two EnsPFeatureadaptor
** objects wrapped up under this EnsPDitagfeatureadaptor.
** One adaptor could implement the simple join condition, while the other
** one could implement the extended join. Could the same be applied to
** the Exon Adaptor? A problem that could remain is that the two
** Feature Adaptors have separate Feature caches.
*/

    result = ensFeatureadaptorFetchAllbySlice(dtfa,
                                              slice,
                                              constraint,
                                              anname,
                                              dtfs);

    ajStrDel(&constraint);

/*
** FIXME: Finish this. Since it is not simply possible to append a
** GROUP BY df.ditag_id, df.ditag_pair_id condition, the results would
** need to be calculated by means of AJAX List and AJAX Table objects here.
** It would be possible to append the condition via
** ensFeatureadaptorSetFinalcondition(dtfa->Adaptor, const char* final),
** but this would be problematic in a multi-threaded environment.
** Could a separate EnsPDitagFeaturepairadaptor be the solution?
** A similar prolem exists for the EnsPExonadaptor and the
** ExonTranscript Adaptor ...
*/

    return result;
}

#endif




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
** @argrule * dtfa [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
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
** @@
******************************************************************************/

AjBool ensDitagfeatureadaptorRetrieveAllIdentifiers(
    EnsPDitagfeatureadaptor dtfa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!dtfa)
        return ajFalse;

    if(!identifiers)
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

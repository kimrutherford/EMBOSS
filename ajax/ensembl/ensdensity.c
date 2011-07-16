/* @source Ensembl Density functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.46 $
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

#include "ensdensity.h"
#include "enstable.h"
#include <math.h>




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* @datastatic DensityPTypeRatio **********************************************
**
** Ensembl Density Type Ratio.
**
** Holds Ensembl Density Types and corresponding ratios.
**
** @alias DensitySTypeRatio
** @alias DensityOTypeRatio
**
** @attr Densitytype [EnsPDensitytype] Ensembl Density Type
** @attr Ratio [float] Ratio
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct DensitySTypeRatio
{
    EnsPDensitytype Densitytype;
    float Ratio;
    ajuint Padding;
} DensityOTypeRatio;

#define DensityPTypeRatio DensityOTypeRatio*




/* @datastatic DensityPLengthValue ********************************************
**
** Ensembl Density Length Value.
**
** Holds density values and corresponding lengths.
**
** @alias DensitySLengthValue
** @alias DensityOLengthValue
**
** @attr Length [ajuint] Length
** @attr Value [float] Value
** @@
******************************************************************************/

typedef struct DensitySLengthValue
{
    ajuint Length;
    float Value;
} DensityOLengthValue;

#define DensityPLengthValue DensityOLengthValue*




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic densitytypeType ***********************************************
**
** The Density Type type is enumerated in both, the SQL table definition
** and the data structure. The following strings are used for
** conversion in database operations and correspond to
** EnsEDensitytypeType.
**
** sum:
** ratio:
**
******************************************************************************/

static const char* const densitytypeType[] =
{
    "",
    "sum",
    "ratio",
    (const char*) NULL
};




/* @conststatic densitytypeadaptorTables **************************************
**
** Ensembl Density Type Adaptor SQL table names
**
******************************************************************************/

static const char* const densitytypeadaptorTables[] =
{
    "density_type",
    (const char*) NULL
};




/* @conststatic densitytypeadaptorColumns *************************************
**
** Ensembl Density Type Adaptor SQL column names
**
******************************************************************************/

static const char* const densitytypeadaptorColumns[] =
{
    "density_type.density_type_id",
    "density_type.analysis_id",
    "density_type.value_type",
    "density_type.block_size",
    "density_type.region_features",
    (const char*) NULL
};




/* @conststatic densityfeatureadaptorTables ***********************************
**
** Ensembl Density Feature Adaptor SQL table names
**
******************************************************************************/

static const char* const densityfeatureadaptorTables[] =
{
    "density_feature",
    (const char*) NULL
};




/* @conststatic densityfeatureadaptorColumns **********************************
**
** Ensembl Density Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const densityfeatureadaptorColumns[] =
{
    "density_feature.density_feature_id",
    "density_feature.seq_region_id",
    "density_feature.seq_region_start",
    "density_feature.seq_region_end",
    "density_feature.density_type_id",
    "density_feature.density_value",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static int densitytyperatioCompareRatioAscending(const void* P1,
                                                 const void* P2);

static int listDensityfeatureCompareStartAscending(const void* P1,
                                                   const void* P2);

static int listDensityfeatureCompareStartDescending(const void* P1,
                                                    const void* P2);

static AjBool densitytypeadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dts);

static AjBool densitytypeadaptorCacheInsert(EnsPDensitytypeadaptor dta,
                                            EnsPDensitytype* Pdt);

static AjBool densitytypeadaptorCacheInit(EnsPDensitytypeadaptor dta);

static void densitytypeadaptorCacheClearIdentifier(void** key,
                                                   void** value,
                                                   void* cl);

static AjBool densitytypeadaptorCacheExit(EnsPDensitytypeadaptor dta);

static void densitytypeadaptorFetchAll(const void* key,
                                       void** value,
                                       void* cl);

static AjBool densityfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dfs);

static void* densityfeatureadaptorCacheReference(void* value);

static void densityfeatureadaptorCacheDelete(void** value);

static size_t densityfeatureadaptorCacheSize(const void* value);

static EnsPFeature densityfeatureadaptorGetFeature(const void* value);




/* @funcstatic densitytyperatioCompareRatioAscending **************************
**
** Comparison function to sort Density Type Ratios by their ratio
** in ascending order.
**
** @param [r] P1 [const void*] Density Type Ratio address 1
** @param [r] P2 [const void*] Density Type Ratio address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int densitytyperatioCompareRatioAscending(const void* P1,
                                                 const void* P2)
{
    int result = 0;

    const DensityPTypeRatio dtr1 = NULL;
    const DensityPTypeRatio dtr2 = NULL;

    dtr1 = *(DensityPTypeRatio const*) P1;
    dtr2 = *(DensityPTypeRatio const*) P2;

    if(ajDebugTest("densitytyperatioCompareRatioAscending"))
        ajDebug("densitytyperatioCompareRatioAscending\n"
                "  dtr1 %p\n"
                "  dtr2 %p\n",
                dtr1,
                dtr2);

    /* Sort empty values towards the end of the AJAX List. */

    if(dtr1 && (!dtr2))
        return -1;

    if((!dtr1) && (!dtr2))
        return 0;

    if((!dtr1) && dtr2)
        return +1;

    /* Evaluate Density Type ratios. */

    if(dtr1->Ratio < dtr2->Ratio)
        result = -1;

    if(dtr1->Ratio > dtr2->Ratio)
        result = +1;

    return result;
}




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensdensity ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPDensitytype] Ensembl Density Type ************************
**
** @nam2rule Densitytype Functions for manipulating
** Ensembl Density Type objects.
**
** @cc Bio::EnsEMBL::DensityType
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Density Type by pointer.
** It is the responsibility of the user to first destroy any previous
** Density Type. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDensitytype]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy dt [const EnsPDensitytype] Ensembl Density Type
** @argrule Ini dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini type [EnsEDensitytypeType] Type
** @argrule Ini size [ajuint] Block size
** @argrule Ini features [ajuint] Number of Ensembl Density Feature objects per
**                                Ensembl Sequence Region
** @argrule Ref dt [EnsPDensitytype] Ensembl Density Type
**
** @valrule * [EnsPDensitytype] Ensembl Density Type or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDensitytypeNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [EnsPDensitytype] Ensembl Density Type or NULL
** @@
******************************************************************************/

EnsPDensitytype ensDensitytypeNewCpy(const EnsPDensitytype dt)
{
    EnsPDensitytype pthis = NULL;

    if(!dt)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = dt->Identifier;
    pthis->Adaptor    = dt->Adaptor;
    pthis->Analysis   = ensAnalysisNewRef(dt->Analysis);
    pthis->Type       = dt->Type;
    pthis->Size       = dt->Size;
    pthis->Features   = dt->Features;

    return pthis;
}




/* @func ensDensitytypeNewIni *************************************************
**
** Constructor for an Ensembl Density Type with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::DensityType::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] type [EnsEDensitytypeType] Type
** @param [r] size [ajuint] Block size
** @param [r] features [ajuint] Number of Ensembl Density Feature objects per
**                              Ensembl Sequence Region
**
** @return [EnsPDensitytype] Ensembl Density Type or NULL
** @@
******************************************************************************/

EnsPDensitytype ensDensitytypeNewIni(EnsPDensitytypeadaptor dta,
                                     ajuint identifier,
                                     EnsPAnalysis analysis,
                                     EnsEDensitytypeType type,
                                     ajuint size,
                                     ajuint features)
{
    EnsPDensitytype dt = NULL;

    if(!analysis)
        return NULL;

    AJNEW0(dt);

    dt->Use = 1;

    dt->Identifier = identifier;
    dt->Adaptor    = dta;
    dt->Analysis   = ensAnalysisNewRef(analysis);
    dt->Type       = type;
    dt->Size       = size;
    dt->Features   = features;

    return dt;
}




/* @func ensDensitytypeNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
**
** @return [EnsPDensitytype] Ensembl Density Type or NULL
** @@
******************************************************************************/

EnsPDensitytype ensDensitytypeNewRef(EnsPDensitytype dt)
{
    if(!dt)
        return NULL;

    dt->Use++;

    return dt;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Density Type object.
**
** @fdata [EnsPDensitytype]
**
** @nam3rule Del Destroy (free) an Ensembl Density Type object
**
** @argrule * Pdt [EnsPDensitytype*] Ensembl Density Type object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDensitytypeDel ****************************************************
**
** Default destructor for an Ensembl Density Type.
**
** @param [d] Pdt [EnsPDensitytype*] Ensembl Density Type object address
**
** @return [void]
** @@
******************************************************************************/

void ensDensitytypeDel(EnsPDensitytype* Pdt)
{
    EnsPDensitytype pthis = NULL;

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

    ensAnalysisDel(&pthis->Analysis);

    AJFREE(pthis);

    *Pdt = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Density Type object.
**
** @fdata [EnsPDensitytype]
**
** @nam3rule Get Return Density Type attribute(s)
** @nam4rule Adaptor Return the Ensembl Density Type Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Features Return the number of Ensembl Density Feature objects
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Size Return the block size
** @nam4rule Type Return the type
**
** @argrule * dt [const EnsPDensitytype] Density Type
**
** @valrule Adaptor [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Features [ajuint] Number of Ensembl Density Feature objects or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Size [ajuint] Block size or 0
** @valrule Type [EnsEDensitytypeType] Type or ensEDensitytypeTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensDensitytypeGetAdaptor *********************************************
**
** Get the Ensembl Density Type Adaptor element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor or NULL
** @@
******************************************************************************/

EnsPDensitytypeadaptor ensDensitytypeGetAdaptor(const EnsPDensitytype dt)
{
    if(!dt)
        return NULL;

    return dt->Adaptor;
}




/* @func ensDensitytypeGetAnalysis ********************************************
**
** Get the Ensembl Analysis element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::analysis
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
** @@
******************************************************************************/

EnsPAnalysis ensDensitytypeGetAnalysis(const EnsPDensitytype dt)
{
    if(!dt)
        return NULL;

    return dt->Analysis;
}




/* @func ensDensitytypeGetFeatures ********************************************
**
** Get the number of Ensembl Density Feature objects element of an
** Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::region_features
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [ajuint] Number of Ensembl Density Feature objects or 0
** @@
******************************************************************************/

ajuint ensDensitytypeGetFeatures(const EnsPDensitytype dt)
{
    if(!dt)
        return 0;

    return dt->Features;
}




/* @func ensDensitytypeGetIdentifier ******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Density Type.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensDensitytypeGetIdentifier(const EnsPDensitytype dt)
{
    if(!dt)
        return 0;

    return dt->Identifier;
}




/* @func ensDensitytypeGetSize ************************************************
**
** Get the block size element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::block_size
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [ajuint] Block size or 0
** @@
******************************************************************************/

ajuint ensDensitytypeGetSize(const EnsPDensitytype dt)
{
    if(!dt)
        return 0;

    return dt->Size;
}




/* @func ensDensitytypeGetType ************************************************
**
** Get the type element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::value_type
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [EnsEDensitytypeType] Type or ensEDensitytypeTypeNULL
** @@
******************************************************************************/

EnsEDensitytypeType ensDensitytypeGetType(const EnsPDensitytype dt)
{
    if(!dt)
        return ensEDensitytypeTypeNULL;

    return dt->Type;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Density Type object.
**
** @fdata [EnsPDensitytype]
**
** @nam3rule Set Set one element of a Density Type
** @nam4rule Adaptor Set the Ensembl Density Type Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Features Set the number of Ensembl Density Feature objects
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Size Set the block size
** @nam4rule Type Set the type
**
** @argrule * dt [EnsPDensitytype] Ensembl Density Feature object
** @argrule Adaptor dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Features features [ajuint]
** Number of Ensembl Density Feature objects
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Size size [ajuint] Block size
** @argrule Type type [EnsEDensitytypeType] Type
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDensitytypeSetAdaptor *********************************************
**
** Set the Ensembl Density Type Adaptor element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeSetAdaptor(EnsPDensitytype dt,
                                EnsPDensitytypeadaptor dta)
{
    if(!dt)
        return ajFalse;

    dt->Adaptor = dta;

    return ajTrue;
}




/* @func ensDensitytypeSetAnalysis ********************************************
**
** Set the Ensembl Analysis element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::analysis
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeSetAnalysis(EnsPDensitytype dt,
                                 EnsPAnalysis analysis)
{
    if(!dt)
        return ajFalse;

    ensAnalysisDel(&dt->Analysis);

    dt->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensDensitytypeSetFeatures ********************************************
**
** Set the number of Ensembl Density Feature objects element of an
** Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::region_features
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [r] features [ajuint] Number of Ensembl Density Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeSetFeatures(EnsPDensitytype dt,
                                 ajuint features)
{
    if(!dt)
        return ajFalse;

    dt->Features = features;

    return ajTrue;
}




/* @func ensDensitytypeSetIdentifier ******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Density Type.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeSetIdentifier(EnsPDensitytype dt,
                                   ajuint identifier)
{
    if(!dt)
        return ajFalse;

    dt->Identifier = identifier;

    return ajTrue;
}




/* @func ensDensitytypeSetSize ************************************************
**
** Set the block size element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::block_size
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [r] size [ajuint] Block size
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeSetSize(EnsPDensitytype dt,
                             ajuint size)
{
    if(!dt)
        return ajFalse;

    dt->Size = size;

    return ajTrue;
}




/* @func ensDensitytypeSetType ************************************************
**
** Set the type element of an Ensembl Density Type.
**
** @cc Bio::EnsEMBL::DensityType::value_type
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [u] type [EnsEDensitytypeType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeSetType(EnsPDensitytype dt,
                             EnsEDensitytypeType type)
{
    if(!dt)
        return ajFalse;

    dt->Type = type;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Density Type object.
**
** @fdata [EnsPDensitytype]
**
** @nam3rule Trace Report Ensembl Density Type elements to debug file
**
** @argrule Trace dt [const EnsPDensitytype] Ensembl Density Type
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDensitytypeTrace **************************************************
**
** Trace an Ensembl Density Type.
**
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeTrace(const EnsPDensitytype dt, ajuint level)
{
    AjPStr indent = NULL;

    if(!dt)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensDensitytypeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Analysis %p\n"
            "%S  Type '%s'\n"
            "%S  Size %u\n"
            "%S  Features %u\n",
            indent, dt,
            indent, dt->Use,
            indent, dt->Identifier,
            indent, dt->Adaptor,
            indent, dt->Analysis,
            indent, ensDensitytypeTypeToChar(dt->Type),
            indent, dt->Size,
            indent, dt->Features);

    ensAnalysisTrace(dt->Analysis, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Density Type object.
**
** @fdata [EnsPDensitytype]
**
** @nam3rule Calculate Calculate Ensembl Density Type values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * dt [const EnsPDensitytype] Ensembl Density Type
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensDensitytypeCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an Ensembl Density Type.
**
** @param [r] dt [const EnsPDensitytype] Ensembl Density Type
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensDensitytypeCalculateMemsize(const EnsPDensitytype dt)
{
    size_t size = 0;

    if(!dt)
        return 0;

    size += sizeof (EnsODensitytype);

    size += ensAnalysisCalculateMemsize(dt->Analysis);

    return size;
}




/* @datasection [EnsEDensitytypeType] Ensembl Density Type Type ***************
**
** @nam2rule Densitytype Functions for manipulating
** Ensembl Density Type objects
** @nam3rule DensitytypeType Functions for manipulating
** Ensembl Density Type Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Density Type Type enumeration.
**
** @fdata [EnsEDensitytypeType]
**
** @nam4rule From Ensembl Density Type Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type [const AjPStr] Type string
**
** @valrule * [EnsEDensitytypeType] Ensembl Density Type Type or
**                                  ensEDensitytypeTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensDensitytypeTypeFromStr ********************************************
**
** Convert an AJAX String into an Ensembl Density Type Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEDensitytypeType] Ensembl Density Type Type or
**                                    ensEDensitytypeTypeNULL
** @@
******************************************************************************/

EnsEDensitytypeType ensDensitytypeTypeFromStr(const AjPStr type)
{
    register EnsEDensitytypeType i = ensEDensitytypeTypeNULL;

    EnsEDensitytypeType etype = ensEDensitytypeTypeNULL;

    for(i = ensEDensitytypeTypeNULL;
        densitytypeType[i];
        i++)
        if(ajStrMatchC(type, densitytypeType[i]))
            etype = i;

    if(!etype)
        ajDebug("ensDensitytypeTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Density Type Type enumeration.
**
** @fdata [EnsEDensitytypeType]
**
** @nam4rule To   Return Ensembl Density Type Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To dtt [EnsEDensitytypeType] Ensembl Density Type Type enumeration
**
** @valrule Char [const char*] Ensembl Density Type type C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensDensitytypeTypeToChar *********************************************
**
** Convert an Ensembl Density Type Type enumeration
** into a C-type (char*) string.
**
** @param [u] dtt [EnsEDensitytypeType] Ensembl Density Type Type enumeration
**
** @return [const char*] Ensembl Density Type type C-type (char*) string
** @@
******************************************************************************/

const char* ensDensitytypeTypeToChar(EnsEDensitytypeType dtt)
{
    register EnsEDensitytypeType i = ensEDensitytypeTypeNULL;

    for(i = ensEDensitytypeTypeNULL;
        densitytypeType[i] && (i < dtt);
        i++);

    if(!densitytypeType[i])
        ajDebug("ensDensitytypeTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Density Type Type enumeration %d.\n",
                dtt);

    return densitytypeType[i];
}




/* @datasection [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor *********
**
** @nam2rule Densitytypeadaptor Functions for manipulating
** Ensembl Density Type Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DensityTypeAdaptor
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic densitytypeadaptorFetchAllbyStatement **************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Density Type objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] dts [AjPList] AJAX List of Ensembl Density Types
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool densitytypeadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dts)
{
    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint size       = 0;
    ajuint features   = 0;

    EnsEDensitytypeType etype = ensEDensitytypeTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr type = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDensitytype dt         = NULL;
    EnsPDensitytypeadaptor dta = NULL;

    if(ajDebugTest("densitytypeadaptorFetchAllbyStatement"))
        ajDebug("densitytypeadaptorFetchAllbyStatement\n"
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

    if(!statement)
        return ajFalse;

    if(!dts)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    dta = ensRegistryGetDensitytypeadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        analysisid = 0;
        type = ajStrNew();
        size = 0;
        features = 0;
        etype = ensEDensitytypeTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &size);
        ajSqlcolumnToUint(sqlr, &features);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        etype = ensDensitytypeTypeFromStr(type);

        dt = ensDensitytypeNewIni(dta,
                                  identifier,
                                  analysis,
                                  etype,
                                  size,
                                  features);

        ajListPushAppend(dts, (void*) dt);

        ensAnalysisDel(&analysis);

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic densitytypeadaptorCacheInsert **********************************
**
** Insert an Ensembl Density Type into the Density Type Adaptor-internal cache.
** If a Density Type with the same identifier element is already present in the
** adaptor cache, the Density Type is deleted and a pointer to the cached
** Density Type is returned.
**
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @param [u] Pdt [EnsPDensitytype*] Ensembl Density Type address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool densitytypeadaptorCacheInsert(EnsPDensitytypeadaptor dta,
                                            EnsPDensitytype* Pdt)
{
    ajuint* Pidentifier = NULL;

    EnsPDensitytype dt = NULL;

    if(!dta)
        return ajFalse;

    if(!dta->CacheByIdentifier)
        return ajFalse;

    if(!Pdt)
        return ajFalse;

    if(!*Pdt)
        return ajFalse;

    /* Search the identifier cache. */

    dt = (EnsPDensitytype) ajTableFetchmodV(
        dta->CacheByIdentifier,
        (const void*) &((*Pdt)->Identifier));

    if(dt)
    {
        ajDebug("densitytypeadaptorCacheInsert replaced Density Type %p with "
                "one already cached %p.\n",
                *Pdt, dt);

        ensDensitytypeDel(Pdt);

        ensDensitytypeNewRef(dt);

        Pdt = &dt;
    }
    else
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pdt)->Identifier;

        ajTablePut(dta->CacheByIdentifier,
                   (void*) Pidentifier,
                   (void*) ensDensitytypeNewRef(*Pdt));
    }

    return ajTrue;
}




/* @funcstatic densitytypeadaptorCacheInit ************************************
**
** Initialise the internal Density Type cache of an
** Ensembl Density Type Adaptor.
**
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool densitytypeadaptorCacheInit(EnsPDensitytypeadaptor dta)
{
    AjPList dts = NULL;

    EnsPDensitytype dt = NULL;

    if(ajDebugTest("densitytypeadaptorCacheInit"))
        ajDebug("densitytypeadaptorCacheInit\n"
                "  dta %p\n",
                dta);

    if(!dta)
        return ajFalse;

    if(dta->CacheByIdentifier)
        return ajFalse;
    else
        dta->CacheByIdentifier = ensTableuintNewLen(0);

    dts = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(dta->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       dts);

    while(ajListPop(dts, (void**) &dt))
    {
        densitytypeadaptorCacheInsert(dta, &dt);

        ensDensitytypeDel(&dt);
    }

    ajListFree(&dts);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Density Type Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Density Type Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDensitytypeadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensDensitytypeadaptorNew *********************************************
**
** Default constructor for an Ensembl Density Type Adaptor.
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
** @see ensRegistryGetDensitytypeadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor or NULL
** @@
******************************************************************************/

EnsPDensitytypeadaptor ensDensitytypeadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPDensitytypeadaptor dta = NULL;

    if(!dba)
        return NULL;

    if(ajDebugTest("ensDensitytypeadaptorNew"))
        ajDebug("ensDensitytypeadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(dta);

    dta->Adaptor = ensBaseadaptorNew(
        dba,
        densitytypeadaptorTables,
        densitytypeadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        densitytypeadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** densitytypeadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint, which calls
    ** densitytypeadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetDensitytypeadaptor. At that point, however, the
    ** Density Type Adaptor has not been stored in the Registry. Therefore,
    ** each ensDensitytypeadaptorFetch function has to test the presence of
    ** the adaptor-internal cache and eventually initialise before accessing
    ** it.
    **
    ** densitytypeadaptorCacheInit(dta);
    */

    return dta;
}




/* @funcstatic densitytypeadaptorCacheClearIdentifier *************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Density Type
** Adaptor-internal Density Type cache. This function deletes the
** unsigned integer identifier key and the Ensembl Density Type value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Density Type value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void densitytypeadaptorCacheClearIdentifier(void** key,
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

    ensDensitytypeDel((EnsPDensitytype*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic densitytypeadaptorCacheExit ************************************
**
** Clears the internal Density Type cache of an Ensembl Density Type Adaptor.
**
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool densitytypeadaptorCacheExit(EnsPDensitytypeadaptor dta)
{
    if(!dta)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(dta->CacheByIdentifier,
                  densitytypeadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&dta->CacheByIdentifier);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Density Type Adaptor object.
**
** @fdata [EnsPDensitytypeadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Density Type Adaptor object.
**
** @argrule * Pdta [EnsPDensitytypeadaptor*] Ensembl Density Type Adaptor
**                                           object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDensitytypeadaptorDel *********************************************
**
** Default destructor for an Ensembl Density Type Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pdta [EnsPDensitytypeadaptor*] Ensembl Density Type Adaptor
**                                           object address
**
** @return [void]
** @@
******************************************************************************/

void ensDensitytypeadaptorDel(EnsPDensitytypeadaptor* Pdta)
{
    EnsPDensitytypeadaptor pthis = NULL;

    if(!Pdta)
        return;

    if(!*Pdta)
        return;

    if(ajDebugTest("ensDensitytypeadaptorDel"))
        ajDebug("ensDensitytypeadaptorDel\n"
                "  Pdta %p\n",
                Pdta);

    pthis = *Pdta;

    densitytypeadaptorCacheExit(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pdta = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Density Type Adaptor object.
**
** @fdata [EnsPDensitytypeadaptor]
**
** @nam3rule Get Return Ensembl Density Type Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Databaseadaptor
**
** @argrule * dta [const EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**                                                or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDensitytypeadaptorGetBaseadaptor **********************************
**
** Get the Ensembl Base Adaptor element of an Ensembl Density Type Adaptor.
**
** @param [r] dta [const EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensDensitytypeadaptorGetBaseadaptor(
    const EnsPDensitytypeadaptor dta)
{
    if(!dta)
        return NULL;

    return dta->Adaptor;
}




/* @func ensDensitytypeadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Density Type Adaptor.
**
** @param [r] dta [const EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDensitytypeadaptorGetDatabaseadaptor(
    const EnsPDensitytypeadaptor dta)
{
    if(!dta)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(dta->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Density Type objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDensitytypeadaptor]
**
** @nam3rule Fetch Fetch Ensembl Density Type object(s)
** @nam4rule All Fetch all Ensembl Density Type objects
** @nam4rule Allby Fetch all Ensembl Density Type objects matching a criterion
** @nam5rule AllbyAnalysisname Fetch all by an Ensembl Analysis name
** @nam4rule By Fetch one Ensembl Density Type object matching a criterion
** @nam5rule ByIdentifier Fetch by an SQL database-internal identifier
**
** @argrule * dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @argrule All dts [AjPList] AJAX List of Ensembl Density Type objects
** @argrule AllbyAnalysisname name [const AjPStr] Ensembl Analysis name
** @argrule AllbyAnalysisname dts [AjPList] AJAX List of Ensembl Density Type
**                                          objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pdt [EnsPDensitytype*] Ensembl Density Type address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic densitytypeadaptorFetchAll *************************************
**
** An ajTableMap "apply" function to return all Density Type objects from the
** Ensembl Density Type Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Density Type value data address
** @param [u] cl [void*] AJAX List of Ensembl Density Type objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void densitytypeadaptorFetchAll(const void* key,
                                       void** value,
                                       void* cl)
{
    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void*)
                     ensDensitytypeNewRef(*((EnsPDensitytype*) value)));

    return;
}




/* @func ensDensitytypeadaptorFetchAll ****************************************
**
** Fetch all Ensembl Density Type objects.
**
** The caller is responsible for deleting the Ensembl Density Type objects
** before deleting the AJAX List.
**
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @param [u] dts [AjPList] AJAX List of Ensembl Density Type objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeadaptorFetchAll(
    EnsPDensitytypeadaptor dta,
    AjPList dts)
{
    if(!dta)
        return ajFalse;

    if(!dts)
        return ajFalse;

    if(!dta->CacheByIdentifier)
        densitytypeadaptorCacheInit(dta);

    ajTableMap(dta->CacheByIdentifier,
               densitytypeadaptorFetchAll,
               (void*) dts);

    return ajTrue;
}




/* @func ensDensitytypeadaptorFetchAllbyAnalysisname **************************
**
** Fetch all Ensembl Density Types by an Ensembl Analysis name.
**
** The caller is responsible for deleting the Ensembl Density Type objects
** before deleting the AJAX List.
**
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @param [r] name [const AjPStr] Ensembl Analysis name
** @param [u] dts [AjPList] AJAX List of Ensembl Density Type objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeadaptorFetchAllbyAnalysisname(
    EnsPDensitytypeadaptor dta,
    const AjPStr name,
    AjPList dts)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    register ajuint i = 0;

    EnsPAnalysis analysis = NULL;

    if(!dta)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!dts)
        return ajFalse;

    if(!dta->CacheByIdentifier)
        densitytypeadaptorCacheInit(dta);

    ajTableToarrayKeysValues(dta->CacheByIdentifier, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        analysis = ensDensitytypeGetAnalysis((EnsPDensitytype) valarray[i]);

        if(ajStrMatchS(name, ensAnalysisGetName(analysis)))
        {
            ajListPushAppend(dts, (void*)
                             ensDensitytypeNewRef((EnsPDensitytype)
                                                  valarray[i]));
        }
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @func ensDensitytypeadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Density Type by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Density Type.
**
** @param [u] dta [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pdt [EnsPDensitytype*] Ensembl Density Type address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensitytypeadaptorFetchByIdentifier(
    EnsPDensitytypeadaptor dta,
    ajuint identifier,
    EnsPDensitytype* Pdt)
{
    AjPList dts = NULL;

    AjPStr constraint = NULL;

    EnsPDensitytype dt = NULL;

    if(!dta)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pdt)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!dta->CacheByIdentifier)
        densitytypeadaptorCacheInit(dta);

    *Pdt = (EnsPDensitytype) ajTableFetchmodV(dta->CacheByIdentifier,
                                              (const void*) &identifier);

    if(*Pdt)
    {
        ensDensitytypeNewRef(*Pdt);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    constraint = ajFmtStr("density_type.density_type_id = %u", identifier);

    dts = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(dta->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       dts);

    if(ajListGetLength(dts) > 1)
        ajWarn("ensDensitytypeadaptorFetchByIdentifier got more than one "
               "Ensembl Density Type for (PRIMARY KEY) identifier %u.\n",
               identifier);

    ajListPop(dts, (void**) Pdt);

    densitytypeadaptorCacheInsert(dta, Pdt);

    while(ajListPop(dts, (void**) &dt))
    {
        densitytypeadaptorCacheInsert(dta, &dt);

        ensDensitytypeDel(&dt);
    }

    ajListFree(&dts);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @datasection [EnsPDensityfeature] Ensembl Density Feature ******************
**
** @nam2rule Densityfeature Functions for manipulating
** Ensembl Density Feature objects
**
** @cc Bio::EnsEMBL::DensityFeature
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Density Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Density Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDensityfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy df [const EnsPDensityfeature] Ensembl Density Feature
** @argrule Ini dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini dt [EnsPDensitytype] Ensembl Density Type
** @argrule Ini value [float] Density value
** @argrule Ref df [EnsPDensityfeature] Ensembl Density Feature
**
** @valrule * [EnsPDensityfeature] Ensembl Density Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDensityfeatureNewCpy **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [EnsPDensityfeature] Ensembl Density Feature or NULL
** @@
******************************************************************************/

EnsPDensityfeature ensDensityfeatureNewCpy(const EnsPDensityfeature df)
{
    EnsPDensityfeature pthis = NULL;

    if(!df)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier  = df->Identifier;
    pthis->Adaptor     = df->Adaptor;
    pthis->Feature     = ensFeatureNewRef(df->Feature);
    pthis->Densitytype = ensDensitytypeNewRef(df->Densitytype);
    pthis->Value       = df->Value;

    return pthis;
}




/* @func ensDensityfeatureNewIni **********************************************
**
** Constructor for an Ensembl Density Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::DensityFeature::new
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
** @param [r] value [float] Density value
**
** @return [EnsPDensityfeature] Ensembl Density Feature or NULL
** @@
******************************************************************************/

EnsPDensityfeature ensDensityfeatureNewIni(EnsPDensityfeatureadaptor dfa,
                                           ajuint identifier,
                                           EnsPFeature feature,
                                           EnsPDensitytype dt,
                                           float value)
{
    EnsPDensityfeature df = NULL;

    if(!feature)
        return NULL;

    AJNEW0(df);

    df->Use = 1;

    df->Identifier  = identifier;
    df->Adaptor     = dfa;
    df->Feature     = ensFeatureNewRef(feature);
    df->Densitytype = ensDensitytypeNewRef(dt);
    df->Value       = value;

    return df;
}




/* @func ensDensityfeatureNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] df [EnsPDensityfeature] Ensembl Density Feature
**
** @return [EnsPDensityfeature] Ensembl Density Feature or NULL
** @@
******************************************************************************/

EnsPDensityfeature ensDensityfeatureNewRef(EnsPDensityfeature df)
{
    if(!df)
        return NULL;

    df->Use++;

    return df;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Density Feature object.
**
** @fdata [EnsPDensityfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Density Feature object
**
** @argrule * Pdf [EnsPDensityfeature*] Ensembl Density Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDensityfeatureDel *************************************************
**
** Default destructor for an Ensembl Density Feature.
**
** @param [d] Pdf [EnsPDensityfeature*] Ensembl Density Feature object address
**
** @return [void]
** @@
******************************************************************************/

void ensDensityfeatureDel(EnsPDensityfeature* Pdf)
{
    EnsPDensityfeature pthis = NULL;

    if(!Pdf)
        return;

    if(!*Pdf)
        return;

    pthis = *Pdf;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pdf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensDensitytypeDel(&pthis->Densitytype);

    AJFREE(pthis);

    *Pdf = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Density Feature object.
**
** @fdata [EnsPDensityfeature]
**
** @nam3rule Get Return Density Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Density Feature Adaptor
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Densitytype Return the Ensembl Density Type
** @nam4rule Value Return the value
**
** @argrule * df [const EnsPDensityfeature] Density Feature
**
** @valrule Adaptor [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
** or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Densitytype [EnsPDensitytype] Ensembl Density Type or
** ensEDensitytypeTypeNULL
** @valrule Value [float] Value or 0.0F
**
** @fcategory use
******************************************************************************/




/* @func ensDensityfeatureGetAdaptor ******************************************
**
** Get the Ensembl Density Feature Adaptor element of an
** Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDensityfeatureadaptor ensDensityfeatureGetAdaptor(
    const EnsPDensityfeature df)
{
    if(!df)
        return NULL;

    return df->Adaptor;
}




/* @func ensDensityfeatureGetDensitytype **************************************
**
** Get the Ensembl Density Type element of an Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::DensityFeature::density_type
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [EnsPDensitytype] Ensembl Density Type or NULL
** @@
******************************************************************************/

EnsPDensitytype ensDensityfeatureGetDensitytype(const EnsPDensityfeature df)
{
    if(!df)
        return NULL;

    return df->Densitytype;
}




/* @func ensDensityfeatureGetFeature ******************************************
**
** Get the Ensembl Feature element of an Ensembl Density Feature.
**
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensDensityfeatureGetFeature(const EnsPDensityfeature df)
{
    if(!df)
        return NULL;

    return df->Feature;
}




/* @func ensDensityfeatureGetIdentifier ***************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensDensityfeatureGetIdentifier(const EnsPDensityfeature df)
{
    if(!df)
        return 0;

    return df->Identifier;
}




/* @func ensDensityfeatureGetValue ********************************************
**
** Get the value element of an Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::DensityFeature::density_value
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [float] Value or 0.0F
** @@
******************************************************************************/

float ensDensityfeatureGetValue(const EnsPDensityfeature df)
{
    if(!df)
        return 0.0F;

    return df->Value;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Density Feature object.
**
** @fdata [EnsPDensityfeature]
**
** @nam3rule Set Set one element of a Density Feature
** @nam4rule Adaptor Set the Ensembl Density Feature Adaptor
** @nam4rule Densitytype Set the Ensembl Density Type
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Value Set the value
**
** @argrule * df [EnsPDensityfeature] Ensembl Density Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Adaptor dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature
**                                                  Adaptor
** @argrule Densitytype dt [EnsPDensitytype] Ensembl Density Type
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Value value [float] Value
**
** @fcategory modify
******************************************************************************/




/* @func ensDensityfeatureSetAdaptor ******************************************
**
** Set the Ensembl Density Feature Adaptor element of an
** Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] df [EnsPDensityfeature] Ensembl Density Feature
** @param [u] dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureSetAdaptor(EnsPDensityfeature df,
                                   EnsPDensityfeatureadaptor dfa)
{
    if(!df)
        return ajFalse;

    df->Adaptor = dfa;

    return ajTrue;
}




/* @func ensDensityfeatureSetDensitytype **************************************
**
** Set the Ensembl Density Type element of an Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::DensityFeature::density_type
** @param [u] df [EnsPDensityfeature] Ensembl Density Feature
** @param [u] dt [EnsPDensitytype] Ensembl Density Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureSetDensitytype(EnsPDensityfeature df,
                                       EnsPDensitytype dt)
{
    if(!df)
        return ajFalse;

    ensDensitytypeDel(&df->Densitytype);

    df->Densitytype = ensDensitytypeNewRef(dt);

    return ajTrue;
}




/* @func ensDensityfeatureSetFeature ******************************************
**
** Set the Ensembl Feature element of an Ensembl Density Feature.
**
** @param [u] df [EnsPDensityfeature] Ensembl Density Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureSetFeature(EnsPDensityfeature df,
                                   EnsPFeature feature)
{
    if(!df)
        return ajFalse;

    ensFeatureDel(&df->Feature);

    df->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensDensityfeatureSetIdentifier ***************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] df [EnsPDensityfeature] Ensembl Density Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureSetIdentifier(EnsPDensityfeature df,
                                      ajuint identifier)
{
    if(!df)
        return ajFalse;

    df->Identifier = identifier;

    return ajTrue;
}




/* @func ensDensityfeatureSetValue ********************************************
**
** Set the value element of an Ensembl Density Feature.
**
** @cc Bio::EnsEMBL::DensityFeature::density_value
** @param [u] df [EnsPDensityfeature] Ensembl Density Feature
** @param [r] value [float] Value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureSetValue(EnsPDensityfeature df,
                                 float value)
{
    if(!df)
        return ajFalse;

    df->Value = value;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Density Feature object.
**
** @fdata [EnsPDensityfeature]
**
** @nam3rule Trace Report Ensembl Density Feature elements to debug file
**
** @argrule Trace df [const EnsPDensityfeature] Ensembl Density Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDensityfeatureTrace ***********************************************
**
** Trace an Ensembl Density Feature.
**
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureTrace(const EnsPDensityfeature df, ajuint level)
{
    AjPStr indent = NULL;

    if(!df)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensDensityfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Densitytype %p\n"
            "%S  Value %f\n",
            indent, df,
            indent, df->Use,
            indent, df->Identifier,
            indent, df->Adaptor,
            indent, df->Feature,
            indent, df->Densitytype,
            indent, df->Value);

    ensFeatureTrace(df->Feature, level + 1);

    ensDensitytypeTrace(df->Densitytype, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Density Feature object.
**
** @fdata [EnsPDensityfeature]
**
** @nam3rule Calculate Calculate Ensembl Density Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * df [const EnsPDensityfeature] Ensembl Density Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensDensityfeatureCalculateMemsize ************************************
**
** Calculate the memory size in bytes of an Ensembl Density Feature.
**
** @param [r] df [const EnsPDensityfeature] Ensembl Density Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensDensityfeatureCalculateMemsize(const EnsPDensityfeature df)
{
    size_t size = 0;

    if(!df)
        return 0;

    size += sizeof (EnsODensityfeature);

    size += ensFeatureCalculateMemsize(df->Feature);

    size += ensDensitytypeCalculateMemsize(df->Densitytype);

    return size;
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
** @nam3rule Densityfeature Functions for manipulating AJAX List objects of
** Ensembl Density Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending dfs [AjPList] AJAX List of Ensembl Density Feature
**                                  objects
** @argrule Descending dfs [AjPList] AJAX List of Ensembl Density Feature
**                                   objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listDensityfeatureCompareStartAscending ************************
**
** AJAX List of Ensembl Density Feature objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Density Feature address 1
** @param [r] P2 [const void*] Ensembl Density Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listDensityfeatureCompareStartAscending(const void* P1,
                                                   const void* P2)
{
    const EnsPDensityfeature df1 = NULL;
    const EnsPDensityfeature df2 = NULL;

    df1 = *(EnsPDensityfeature const*) P1;
    df2 = *(EnsPDensityfeature const*) P2;

    if(ajDebugTest("listDensityfeatureCompareStartAscending"))
        ajDebug("listDensityfeatureCompareStartAscending\n"
                "  df1 %p\n"
                "  df2 %p\n",
                df1,
                df2);

    /* Sort empty values towards the end of the AJAX List. */

    if(df1 && (!df2))
        return -1;

    if((!df1) && (!df2))
        return 0;

    if((!df1) && df2)
        return +1;

    return ensFeatureCompareStartAscending(df1->Feature, df2->Feature);
}




/* @func ensListDensityfeatureSortStartAscending ******************************
**
** Sort an AJAX List of Ensembl Density Feature objects by their
** Ensembl Feature start element in ascending order.
**
** @param [u] dfs [AjPList] AJAX List of Ensembl Density Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListDensityfeatureSortStartAscending(AjPList dfs)
{
    if(!dfs)
        return ajFalse;

    ajListSort(dfs, listDensityfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listDensityfeatureCompareStartDescending ***********************
**
** AJAX List of Ensembl Density Feature objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Density Feature address 1
** @param [r] P2 [const void*] Ensembl Density Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listDensityfeatureCompareStartDescending(const void* P1,
                                                    const void* P2)
{
    const EnsPDensityfeature df1 = NULL;
    const EnsPDensityfeature df2 = NULL;

    df1 = *(EnsPDensityfeature const*) P1;
    df2 = *(EnsPDensityfeature const*) P2;

    if(ajDebugTest("listDensityfeatureCompareStartDescending"))
        ajDebug("listDensityfeatureCompareStartDescending\n"
                "  df1 %p\n"
                "  df2 %p\n",
                df1,
                df2);

    /* Sort empty values towards the end of the AJAX List. */

    if(df1 && (!df2))
        return -1;

    if((!df1) && (!df2))
        return 0;

    if((!df1) && df2)
        return +1;

    return ensFeatureCompareStartDescending(df1->Feature, df2->Feature);
}




/* @func ensListDensityfeatureSortStartDescending *****************************
**
** Sort an AJAX List of Ensembl Density Feature objects by their
** Ensembl Feature start element in descending order.
**
** @param [u] dfs [AjPList] AJAX List of Ensembl Density Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListDensityfeatureSortStartDescending(AjPList dfs)
{
    if(!dfs)
        return ajFalse;

    ajListSort(dfs, listDensityfeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor ***
**
** @nam2rule Densityfeatureadaptor Functions for manipulating
** Ensembl Density Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DensityFeatureAdaptor
** @cc CVS Revision: 1.26
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic densityfeatureadaptorFetchAllbyStatement ***********************
**
** Fetch all Ensembl Density Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::DensityFeatureAdaptor::_objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] dfs [AjPList] AJAX List of Ensembl Density Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool densityfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList dfs)
{
    float value = 0.0F;

    ajuint identifier = 0;
    ajuint dtid       = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPDensityfeature df         = NULL;
    EnsPDensityfeatureadaptor dfa = NULL;

    EnsPDensitytype dt         = NULL;
    EnsPDensitytypeadaptor dta = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("densityfeatureadaptorFetchAllbyStatement"))
        ajDebug("densityfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  dfs %p\n",
                dba,
                statement,
                am,
                slice,
                dfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!dfs)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(dba);

    dfa = ensRegistryGetDensityfeatureadaptor(dba);

    dta = ensRegistryGetDensitytypeadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        dtid       = 0;
        value      = 0.0F;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToUint(sqlr, &dtid);
        ajSqlcolumnToFloat(sqlr, &value);

        /* Need to get the internal Ensembl Sequence Region identifier. */

        srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa, srid);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Feature objects, the range
        ** needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("densityfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("densityfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region end coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srend, INT_MAX);

        slstrand = 1;

        /* Fetch a Slice spanning the entire Sequence Region. */

        ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);

        /*
        ** Obtain an Ensembl Assembly Mapper if none was defined, but a
        ** destination Slice was.
        */

        if(am)
            am = ensAssemblymapperNewRef(am);
        else if(slice && (!ensCoordsystemMatch(
                              ensSliceGetCoordsystemObject(slice),
                              ensSliceGetCoordsystemObject(srslice))))
            ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice, &am);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper was provided.
        */

        if(am)
        {
            ensAssemblymapperFastmap(am,
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

            if(ensSliceCalculateLength(slice) <= INT_MAX)
                sllength = (ajint) ensSliceCalculateLength(slice);
            else
                ajFatal("densityfeatureadaptorFetchAllbyStatement got a "
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
            ** Throw away Feature objects off the end of the requested Slice or
            ** on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Load the next Feature but destroy first! */

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        ensDensitytypeadaptorFetchByIdentifier(dta, dtid, &dt);

        /* Finally, create a new Ensembl Density Feature. */

        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    srslice,
                                    slstart,
                                    slend,
                                    1);

        df = ensDensityfeatureNewIni(dfa, identifier, feature, dt, value);

        ajListPushAppend(dfs, (void*) df);

        ensDensitytypeDel(&dt);

        ensFeatureDel(&feature);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic densityfeatureadaptorCacheReference ****************************
**
** Wrapper function to reference an Ensembl Density Feature
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Density Feature
**
** @return [void*] Ensembl Density Feature or NULL
** @@
******************************************************************************/

static void* densityfeatureadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensDensityfeatureNewRef((EnsPDensityfeature) value);
}




/* @funcstatic densityfeatureadaptorCacheDelete *******************************
**
** Wrapper function to delete an Ensembl Density Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Density Feature address
**
** @return [void]
** @@
******************************************************************************/

static void densityfeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensDensityfeatureDel((EnsPDensityfeature*) value);

    return;
}




/* @funcstatic densityfeatureadaptorCacheSize *********************************
**
** Wrapper function to determine the memory size of an Ensembl Density Feature
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Density Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t densityfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensDensityfeatureCalculateMemsize((const EnsPDensityfeature) value);
}




/* @funcstatic densityfeatureadaptorGetFeature ********************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Density Feature
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Density Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature densityfeatureadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensDensityfeatureGetFeature((const EnsPDensityfeature) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Density Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Density Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDensityfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensDensityfeatureadaptorNew ******************************************
**
** Default constructor for an Ensembl Density Feature Adaptor.
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
** @see ensRegistryGetDensityfeatureadaptor
**
** @cc Bio::EnsEMBL::DBSQL::DensityFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPDensityfeatureadaptor ensDensityfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        densityfeatureadaptorTables,
        densityfeatureadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        densityfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        densityfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        densityfeatureadaptorCacheDelete,
        densityfeatureadaptorCacheSize,
        densityfeatureadaptorGetFeature,
        "Density Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Density Feature Adaptor object.
**
** @fdata [EnsPDensityfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Density Feature Adaptor object
**
** @argrule * Pdfa [EnsPDensityfeatureadaptor*] Ensembl Density Feature Adaptor
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDensityfeatureadaptorDel ******************************************
**
** Default destructor for an Ensembl Density Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pdfa [EnsPDensityfeatureadaptor*] Ensembl Density Feature Adaptor
**                                              object address
**
** @return [void]
** @@
******************************************************************************/

void ensDensityfeatureadaptorDel(EnsPDensityfeatureadaptor* Pdfa)
{
    if(!Pdfa)
        return;

    if(!*Pdfa)
        return;

    ensFeatureadaptorDel(Pdfa);

    *Pdfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Density Feature Adaptor object.
**
** @fdata [EnsPDensityfeatureadaptor]
**
** @nam3rule Get Return Ensembl Density Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDensityfeatureadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Density Feature Adaptor.
**
** @param [u] dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDensityfeatureadaptorGetDatabaseadaptor(
    EnsPDensityfeatureadaptor dfa)
{
    if(!dfa)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(dfa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Density Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDensityfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Density Feature object(s)
** @nam4rule All Fetch all Ensembl Density Feature objects
** @nam4rule Allby Fetch all Ensembl Density Feature objects
**                 matching a criterion
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam4rule By Fetch one Ensembl Density Feature object matching a criterion
** @nam5rule ByIdentifier Fetch by an SQL database-internal identifier
**
** @argrule * dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
** @argrule All dfs [AjPList] AJAX List of Ensembl Density Feature objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice blocks [ajuint] The number of features
** @argrule AllbySlice interpolate [AjBool] A flag indicating interpolation
** @argrule AllbySlice maxratio [float] The maximum ratio
** @argrule AllbySlice dfs [AjPList] AJAX List of Ensembl Density Feature
**                                   objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pdf [EnsPDensityfeature*] Ensembl Density Feature
**                                                 address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDensityfeatureadaptorFetchAllbySlice ******************************
**
** Fetch all Ensembl Density Feature objects on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [u] dfa [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] blocks [ajuint] The number of Ensembl Density Feature objects
**     that are desired. The ratio between the size of
**     features and the size of the features in the database will be
**     used to determine, which database features will be used.
** @param [r] interpolate [AjBool] A flag indicating
**     whether the features in the database should be interpolated to
**     fit them to the requested number of features. If true the
**     features will be interpolated to provide $num_blocks features.
**     This will not guarantee that exactly $num_blocks features are
**     returned due to rounding etc. but it will be close.
** @param [r] maxratio [float] The maximum ratio between the size of the
**     requested features (as determined by blocks) and the actual
**     size of the features in the database.  If this value is exceeded
**     then an empty list will be returned.  This can be used to
**     prevent excessive interpolation of the database values.
** @param [u] dfs [AjPList] AJAX List of Ensembl Density Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDensityfeatureadaptorFetchAllbySlice(
    EnsPDensityfeatureadaptor dfa,
    EnsPSlice slice,
    const AjPStr anname,
    ajuint blocks,
    AjBool interpolate,
    float maxratio,
    AjPList dfs)
{
    double portion = 0.0;
    double ratio   = 0.0;
    ajint  slctmp  = 0;
    double ceitmp  = 0.0;
    float value    = 0.0F;

    register ajuint i = 0;

    /* Block start and end */
    ajint bstart   = 0;
    ajint bend     = 0;
    ajuint blength = 0;

    /* Feature start and end */
    ajint fstart = 0;
    ajint fend   = 0;

    ajint bsize = 0;

    AjBool debug = AJFALSE;

    AjPList dts       = NULL;
    AjPList dlvs      = NULL;
    AjPList dtrs      = NULL;
    AjPList dfeatures = NULL;

    AjPStr constraint = NULL;
    AjPStr names      = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDensityfeature df    = NULL;
    EnsPDensityfeature newdf = NULL;

    EnsPDensitytype dt         = NULL;
    EnsPDensitytype newdt      = NULL;
    EnsPDensitytypeadaptor dta = NULL;

    EnsPFeature feature    = NULL;
    EnsPFeature newfeature = NULL;

    DensityPLengthValue dlv = NULL;

    DensityPTypeRatio dtr = NULL;

    debug = ajDebugTest("ensDensityfeatureadaptorFetchAllbySlice");

    if(debug)
        ajDebug("ensDensityfeatureadaptorFetchAllbySlice\n"
                "  dfa %p\n"
                "  slice %p\n"
                "  anname '%S'\n"
                "  blocks %u\n"
                "  interpolate %B\n"
                "  maxratio %f\n"
                "  dfs %p\n",
                dfa,
                slice,
                anname,
                blocks,
                interpolate,
                maxratio,
                dfs);

    if(!dfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!dfs)
        return ajFalse;

    if(!blocks)
        blocks = 50U;

    /*
    ** Get all of the Density Types and choose the one with the
    ** block size closest to our desired block size.
    */

    dba = ensFeatureadaptorGetDatabaseadaptor(dfa);

    dta = ensRegistryGetDensitytypeadaptor(dba);

    dts = ajListNew();

    ensDensitytypeadaptorFetchAllbyAnalysisname(dta, anname, dts);

    if(!ajListGetLength(dts))
    {
        ensDensitytypeadaptorFetchAll(dta, dts);

        names = ajStrNew();

        while(ajListPop(dts, (void**) &dt))
        {
            analysis = ensDensitytypeGetAnalysis(dt);

            names = ajFmtPrintAppS(&names, "'%S' ",
                                   ensAnalysisGetName(analysis));

            ensDensitytypeDel(&dt);
        }

        ajWarn("ensDensityfeatureadaptorFetchAllbySlice got an invalid "
               "Ensembl Analysis name '%S' only the following are allowed %S.",
               anname,
               names);

        ajStrDel(&names);

        ajListFree(&dts);

        return ajFalse;
    }

    /*
    ** The window size is the length of the Ensembl Slice
    ** divided by the number of requested blocks.
    ** Need to go through some horrendous casting to
    ** satisfy really pedantic compilers
    */

    slctmp = (ajint) ensSliceCalculateLength(slice);
    ceitmp = ceil((double) slctmp / (double) blocks);

    bsize = (ajint) ceitmp;

    dtrs = ajListNew();

    while(ajListPop(dts, (void**) &dt))
    {
        if(ensDensitytypeGetSize(dt) > 0)
            ratio = (double) bsize /
                (double) (ajint) ensDensitytypeGetSize(dt);
        else
        {
            /*
            ** This is only valid if the requested Sequence Region is the one
            ** the Feature objects are stored on. Please use sensibly or find a
            ** better implementation.
            */

            ratio = (double) bsize /
                (ensSliceGetSeqregionLength(slice) /
                 (double) (ajint) ensDensitytypeGetFeatures(dt));
        }

        /*
        ** We prefer to use a block size that is smaller than the
        ** required one, which gives better results on interpolation.
        ** Give larger bits a disadvantage and make them comparable.
        */

        if(ratio < 1.0)
            ratio = 5.0 / ratio;

        AJNEW0(dtr);

        dtr->Densitytype = dt;

        dtr->Ratio = (float) ratio;

        ajListPushAppend(dtrs, (void*) dtr);
    }

    ajListFree(&dts);

    /*
    ** Sort the AJAX List of Ensembl Density Types by their ratio
    ** and get the best one.
    */

    ajListSort(dtrs, densitytyperatioCompareRatioAscending);

    ajListPeekFirst(dtrs, (void**) &dtr);

    if(debug)
    {
        if(dtr)
            ajDebug("ensDensityfeatureadaptorFetchAllbySlice got ratio %f "
                    "and maxratio %f.\n", dtr->Ratio, maxratio);
        else
            ajDebug("ensDensityfeatureadaptorFetchAllbySlice got no ratio.\n");
    }

    /*
    ** The ratio was not good enough, or the Ensembl Analysis name was not
    ** in the 'density_type' table, return an empty list.
    */

    if((!dtr) || (dtr->Ratio > maxratio))
    {
        while(ajListPop(dtrs, (void**) &dtr))
        {
            ensDensitytypeDel(&dtr->Densitytype);

            AJFREE(dtr);
        }

        ajListFree(&dtrs);

        return ajTrue;
    }

    constraint = ajFmtStr("density_feature.density_type_id = %u",
                          ensDensitytypeGetIdentifier(dtr->Densitytype));

    ensFeatureadaptorFetchAllbySlice(dfa,
                                     slice,
                                     constraint,
                                     (const AjPStr) NULL,
                                     dfs);

    ajStrDel(&constraint);

    if(!interpolate)
    {
        while(ajListPop(dtrs, (void**) &dtr))
        {
            ensDensitytypeDel(&dtr->Densitytype);

            AJFREE(dtr);
        }

        ajListFree(&dtrs);

        return ajTrue;
    }

    /*
    ** Interpolate the Density Feature objects into new Density Feature objects
    ** of a different size, but move them to a new AJAX List first. Sort
    ** Ensembl Density Feature objects by ascening start position.
    */

    dfeatures = ajListNew();

    while(ajListPop(dfs, (void**) &df))
        ajListPushAppend(dfeatures, (void*) df);

    ensListDensityfeatureSortStartAscending(dfeatures);

    /* Resize the Feature objects that were returned. */

    bstart = 1;

    bend = bstart + bsize - 1;

    /*
    ** Create a new Ensembl Density Type for the interpolated
    ** Ensembl Density Feature objects that are not stored in the database.
    */

    newdt = ensDensitytypeNewCpy(dtr->Densitytype);

    ensDensitytypeSetIdentifier(newdt, 0);

    ensDensitytypeSetSize(newdt, bsize);

    dlvs = ajListNew();

    while(bstart < (ajint) ensSliceCalculateLength(slice))
    {
        value = 0.0F;

        if(debug)
            ajDebug("ensDensityfeatureadaptorFetchAllbySlice "
                    "bstart %d bend %d value %f\n",
                    bstart, bend, value);

        /*
        ** Construct a new Ensembl Density Feature using all the old
        ** Ensembl Density Feature objects that overlapped.
        */

        while(ajListPeekNumber(dfeatures, i, (void**) &df) &&
              (feature = ensDensityfeatureGetFeature(df)) &&
              (ensFeatureGetStart(feature) < bend))
        {
            /*
            ** What portion of this Feature is used
            ** to construct the new block?
            */

            /* Constrain the Feature start to no less than the block start. */

            fstart = (ensFeatureGetStart(feature) < bstart)
                ? bstart : ensFeatureGetStart(feature);

            /* Constrain the Feature end to no more than the block end. */

            fend = (ensFeatureGetEnd(feature) > bend)
                ? bend : ensFeatureGetEnd(feature);

            /* Constrain the Feature end to no more than the Slice length. */

            fend = (fend > (ajint) ensSliceCalculateLength(slice))
                ? (ajint) ensSliceCalculateLength(slice) : fend;

            if(debug)
                ajDebug("ensDensityfeatureadaptorFetchAllbySlice "
                        "bstart %d bend %d fstart %d fend %d id %u value %f\n",
                        bstart, bend, fstart, fend,
                        ensDensityfeatureGetIdentifier(df),
                        ensDensityfeatureGetValue(df));

            switch(ensDensitytypeGetType(newdt))
            {
                case ensEDensitytypeTypeSum:

                    portion = (fend - fstart + 1) /
                        ensFeatureCalculateLength(feature);

                    /*
                    ** Take a percentage of density value, depending on
                    ** how much of the Ensembl Density Feature was overlapped.
                    */

                    value += (float) (portion *
                                      ensDensityfeatureGetValue(df));

                    break;

                case ensEDensitytypeTypeRatio:

                    /*
                    ** Maintain a running total of the length used from each
                    ** Ensembl Density Feature and its value.
                    */

                    AJNEW0(dlv);

                    dlv->Length = fend - fstart + 1;

                    dlv->Value = ensDensityfeatureGetValue(df);

                    ajListPushAppend(dlvs, (void*) dlv);

                    break;

                default:

                    ajWarn("ensDensityfeatureadaptorFetchAllbySlice got an "
                           "Ensembl Density Type with unknown type %d.",
                           ensDensitytypeGetType(newdt));
            }

            /*
            ** Do not look at the next Density Feature,
            ** if only a part of this one has been used.
            */

            if(fend < ensFeatureGetEnd(feature))
                break;

            i++;
        }

        if(ensDensitytypeGetType(newdt) == ensEDensitytypeTypeRatio)
        {
            /*
            ** Take a weighted average of all the density values
            ** of the Feature objects used to construct this one.
            */

            blength = bend - bstart + 1;

            while(ajListPop(dlvs, (void**) &dlv))
            {
                if(blength > 0)
                    value += dlv->Value * (float) dlv->Length /
                        (float) blength;

                if(debug)
                    ajDebug("ensDensityfeatureadaptorFetchAllbySlice "
                            "bstart %d bend %d blength %u "
                            "DLV Length %u DLV Value %f "
                            "value %f \n",
                            bstart, bend, blength,
                            dlv->Length, dlv->Value,
                            value);

                AJFREE(dlv);
            }
        }

        /*
        ** Interpolated Ensembl Density Feature objects are not stored in the
        ** database so they do not need an identifier or
        ** Ensembl Density Feature Adaptor.
        */

        newfeature = ensFeatureNewCpy(feature);

        ensFeatureMove(newfeature, bstart, bend, 0);

        newdf = ensDensityfeatureNewIni((EnsPDensityfeatureadaptor) NULL,
                                        0,
                                        newfeature,
                                        newdt,
                                        value);

        ajListPushAppend(dfs, (void*) newdf);

        ensFeatureDel(&newfeature);

        /* Clear the AJAX List of remaining density length values. */

        while(ajListPop(dlvs, (void**) &dlv))
            AJFREE(dlv);

        /* Advance one block. */

        bstart = bend + 1;

        bend  += bsize;
    }

    ajListFree(&dlvs);

    ensDensitytypeDel(&newdt);

    /* Clear the AJAX List of intermediary Ensembl Density Feature objects. */

    while(ajListPop(dfeatures, (void**) &df))
        ensDensityfeatureDel(&df);

    ajListFree(&dfeatures);

    /* Clear the AJAX List of density type ratios. */

    while(ajListPop(dtrs, (void**) &dtr))
    {
        ensDensitytypeDel(&dtr->Densitytype);

        AJFREE(dtr);
    }

    ajListFree(&dtrs);

    return ajTrue;
}

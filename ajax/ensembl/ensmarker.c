/* @source ensmarker **********************************************************
**
** Ensembl Marker functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.59 $
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

#include "ensmarker.h"
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

/* @conststatic markerKType ***************************************************
**
** The Ensembl Marker type member is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsEMarkerType.
**
******************************************************************************/

static const char *const markerKType[] =
{
    "",
    "est",
    "microsatellite",
    (const char *) NULL
};




/* @conststatic markeradaptorKTables ******************************************
**
** Array of Ensembl Marker Adaptor SQL table names
**
******************************************************************************/

static const char *const markeradaptorKTables[] =
{
    "marker",
    "marker_synonym",
    (const char *) NULL
};




/* @conststatic markeradaptorKColumns *****************************************
**
** Array of Ensembl Marker Adaptor SQL column names
**
******************************************************************************/

static const char *const markeradaptorKColumns[] =
{
    "marker.marker_id",
    "marker.display_marker_synonym_id",
    "marker.left_primer",
    "marker.right_primer",
    "marker.min_primer_dist",
    "marker.max_primer_dist",
    "marker.priority",
    "marker.type",
    "marker_synonym.marker_synonym_id",
    "marker_synonym.name",
    "marker_synonym.source",
    (const char *) NULL
};




/* @conststatic markeradaptorKLeftjoin ****************************************
**
** Array of Ensembl Marker Adaptor SQL left join conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin markeradaptorKLeftjoin[] =
{
    {
        "marker_synonym",
        "marker.marker_id = marker_synonym.marker_id"
    },
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic markerfeatureadaptorKTables ***********************************
**
** Array of Ensembl Marker Feature Adaptor SQL table names
**
******************************************************************************/

static const char *const markerfeatureadaptorKTables[] =
{
    "marker_feature",
    "marker",
    "marker_synonym",
    (const char *) NULL
};




/* @conststatic markerfeatureadaptorKColumns **********************************
**
** Array of Ensembl Marker Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const markerfeatureadaptorKColumns[] =
{
    "marker_feature.marker_feature_id",
    "marker_feature.seq_region_id",
    "marker_feature.seq_region_start",
    "marker_feature.seq_region_end",
    "marker_feature.analysis_id",
    "marker_feature.map_weight",
    "marker.marker_id",
    "marker.left_primer",
    "marker.right_primer",
    "marker.min_primer_dist",
    "marker.max_primer_dist",
    "marker.priority",
    "marker.type",
    "marker_synonym.marker_synonym_id",
    "marker_synonym.name",
    "marker_synonym.source",
    (const char *) NULL
};




/* @conststatic markerfeatureadaptorKLeftjoin *********************************
**
** Array of Ensembl Marker Feature Adaptor SQL left join conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin markerfeatureadaptorKLeftjoin[] =
{
    {
        "marker_synonym",
        "marker.display_marker_synonym_id = marker_synonym.marker_synonym_id"
    },
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic markerfeatureadaptorKDefaultcondition *************************
**
** Ensembl Marker Feature Adaptor SQL default condition
**
******************************************************************************/

static const char *markerfeatureadaptorKDefaultcondition =
    "marker_feature.marker_id = marker.marker_id";




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool markersynonymadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mss);

static AjBool markermaplocationadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mmls);

static AjBool markeradaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList markers);

static int listMarkerfeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listMarkerfeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listMarkerfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listMarkerfeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listMarkerfeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool markerfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensmarker *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMarkersynonym] Ensembl Marker Synonym ********************
**
** @nam2rule Markersynonym Functions for manipulating
** Ensembl Marker Synonym objects
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym
** @cc CVS Revision: 1.7
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Marker Synonym by pointer.
** It is the responsibility of the user to first destroy any previous
** Marker Synonym. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ms [const EnsPMarkersynonym] Ensembl Marker Synonym
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini source [AjPStr] Source
** @argrule Ini name [AjPStr] Name
** @argrule Ref ms [EnsPMarkersynonym] Ensembl Marker Synonym
**
** @valrule * [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMarkersynonymNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkersynonymNewCpy(const EnsPMarkersynonym ms)
{
    EnsPMarkersynonym pthis = NULL;

    if (!ms)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = ms->Identifier;

    if (ms->Source)
        pthis->Source = ajStrNewRef(ms->Source);

    if (ms->Name)
        pthis->Name = ajStrNewRef(ms->Name);

    return pthis;
}




/* @func ensMarkersynonymNewIni ***********************************************
**
** Constructor for an Ensembl Marker Synonym with initial values.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::new
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] source [AjPStr] Source
** @param [u] name [AjPStr] Name
**
** @return [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkersynonymNewIni(ajuint identifier,
                                         AjPStr source,
                                         AjPStr name)
{
    EnsPMarkersynonym ms = NULL;

    AJNEW0(ms);

    ms->Use        = 1U;
    ms->Identifier = identifier;

    if (source)
        ms->Source = ajStrNewRef(source);

    if (name)
        ms->Name = ajStrNewRef(name);

    return ms;
}




/* @func ensMarkersynonymNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkersynonymNewRef(EnsPMarkersynonym ms)
{
    if (!ms)
        return NULL;

    ms->Use++;

    return ms;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Synonym
**
** @argrule * Pms [EnsPMarkersynonym*] Ensembl Marker Synonym address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkersynonymDel **************************************************
**
** Default destructor for an Ensembl Marker Synonym.
**
** @param [d] Pms [EnsPMarkersynonym*] Ensembl Marker Synonym address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMarkersynonymDel(EnsPMarkersynonym *Pms)
{
    EnsPMarkersynonym pthis = NULL;

    if (!Pms)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMarkersynonymDel"))
    {
        ajDebug("ensMarkersynonymDel\n"
                "  *Pms %p\n",
                *Pms);

        ensMarkersynonymTrace(*Pms, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pms)
        return;

    pthis = *Pms;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pms = NULL;

        return;
    }

    ajStrDel(&pthis->Source);
    ajStrDel(&pthis->Name);

    AJFREE(pthis);

    *Pms = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Get Return Marker Synonym attribute(s)
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Source Return the source
**
** @argrule * ms [const EnsPMarkersynonym] Marker Synonym
**
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Source [AjPStr] Source or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkersynonymGetIdentifier ****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::dbID
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensMarkersynonymGetIdentifier(const EnsPMarkersynonym ms)
{
    return (ms) ? ms->Identifier : 0U;
}




/* @func ensMarkersynonymGetName **********************************************
**
** Get the name member of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::name
** @param  [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjPStr] Name or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensMarkersynonymGetName(const EnsPMarkersynonym ms)
{
    return (ms) ? ms->Name : NULL;
}




/* @func ensMarkersynonymGetSource ********************************************
**
** Get the source member of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::source
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjPStr] Source or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensMarkersynonymGetSource(const EnsPMarkersynonym ms)
{
    return (ms) ? ms->Source : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Set Set one member of an Ensembl Marker Synonym
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Source Set the source
**
** @argrule * ms [EnsPMarkersynonym] Ensembl Marker Synonym object
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Source source [AjPStr] Source
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMarkersynonymSetIdentifier ****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::dbID
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkersynonymSetIdentifier(EnsPMarkersynonym ms, ajuint identifier)
{
    if (!ms)
        return ajFalse;

    ms->Identifier = identifier;

    return ajTrue;
}




/* @func ensMarkersynonymSetName **********************************************
**
** Set the name member of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::name
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkersynonymSetName(EnsPMarkersynonym ms, AjPStr name)
{
    if (!ms)
        return ajFalse;

    ajStrDel(&ms->Name);

    ms->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensMarkersynonymSetSource ********************************************
**
** Set the source member of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::source
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [u] source [AjPStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkersynonymSetSource(EnsPMarkersynonym ms, AjPStr source)
{
    if (!ms)
        return ajFalse;

    ajStrDel(&ms->Source);

    ms->Source = ajStrNewRef(source);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Trace Report Ensembl Marker Synonym members to debug file.
**
** @argrule Trace ms [const EnsPMarkersynonym] Ensembl Marker Synonym
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkersynonymTrace ************************************************
**
** Trace an Ensembl Marker Synonym.
**
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkersynonymTrace(const EnsPMarkersynonym ms, ajuint level)
{
    AjPStr indent = NULL;

    if (!ms)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMarkersynonymTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Source '%S'\n"
            "%S  Name '%S'\n",
            indent, ms,
            indent, ms->Use,
            indent, ms->Identifier,
            indent, ms->Source,
            indent, ms->Name);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Calculate Calculate Ensembl Marker Synonym values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkersynonymCalculateMemsize *************************************
**
** Calculate the memory size in bytes of an Ensembl Marker Synonym.
**
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMarkersynonymCalculateMemsize(const EnsPMarkersynonym ms)
{
    size_t size = 0;

    if (!ms)
        return 0;

    size += sizeof (EnsOMarkersynonym);

    if (ms->Source)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Source);
    }

    if (ms->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Name);
    }

    return size;
}




/* @datasection [EnsPDatabaseadaptor] Ensembl Database Adaptor ****************
**
** @nam2rule Markersynonymadaptor Functions for manipulating
** Ensembl Marker Synonym Adaptor objects
**
******************************************************************************/




/* @funcstatic markersynonymadaptorFetchAllbyStatement ************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Marker Synonym objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] mss [AjPList] AJAX List of Ensembl Marker Synonym objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool markersynonymadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mss)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name   = NULL;
    AjPStr source = NULL;

    EnsPMarkersynonym ms = NULL;

    if (!dba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!mss)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name = ajStrNew();
        source = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &source);

        ms = ensMarkersynonymNewIni(identifier, source, name);

        ajListPushAppend(mss, (void *) ms);

        ajStrDel(&name);
        ajStrDel(&source);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Marker Synonym Adaptor objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Fetch Fetch Ensembl Marker Synonym object(s)
** @nam4rule All Fetch all Ensembl Marker Synonym objects
** @nam4rule Allby Fetch all Ensembl Marker Synonym objects matching a
**                 criterion
** @nam5rule Markeridentifier Fetch all by an Ensembl Marker identifier
** @nam4rule By Fetch one Ensembl Marker Synonym object matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Marker Synonym Adaptor
** @argrule AllbyMarkeridentifier markerid [ajuint] Ensembl Marker identifier
** @argrule AllbyMarkeridentifier mss [AjPList] AJAX List of
**                                              Ensembl Marker Synonym objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pms [EnsPMarkersynonym*] Ensembl Marker Synonym
**                                                address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMarkersynonymadaptorFetchAllbyMarkeridentifier ********************
**
** Fetch all Ensembl Marker Synonym objects by an Ensembl Marker identifier.
**
** The caller is responsible for deleting the Ensembl Marker Synonym objects
** before deleting the AJAX List.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] markerid [ajuint] Ensembl Marker identifier
** @param [u] mss [AjPList] AJAX List of Ensembl Marker Synonym objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkersynonymadaptorFetchAllbyMarkeridentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mss)
{
    AjBool result = AJFALSE;

    AjPStr statement = NULL;

    if (!dba)
        return ajFalse;

    if (!mss)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "marker_synonym.marker_synonym_id, "
        "marker_synonym.source, "
        "marker_synonym.name "
        "FROM "
        "marker_synonym "
        "WHERE "
        "marker_synonym.marker_id = %u",
        markerid);

    result = markersynonymadaptorFetchAllbyStatement(dba, statement, mss);

    ajStrDel(&statement);

    return result;
}




/* @func ensMarkersynonymadaptorFetchByIdentifier *****************************
**
** Fetch an Ensembl Marker Synonym via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Marker Synonym.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pms [EnsPMarkersynonym*] Ensembl Marker Synonym address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkersynonymadaptorFetchByIdentifier(
    EnsPDatabaseadaptor dba,
    ajuint identifier,
    EnsPMarkersynonym *Pms)
{
    AjPList mss = NULL;

    AjPStr statement = NULL;

    EnsPMarkersynonym ms = NULL;

    if (!dba)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pms)
        return ajFalse;

    *Pms = NULL;

    statement = ajFmtStr(
        "SELECT "
        "marker_synonym.marker_synonym_id, "
        "marker_synonym.source, "
        "marker_synonym.name "
        "FROM "
        "marker_synonym "
        "WHERE "
        "marker_synonym.marker_synonym_id = %u",
        identifier);

    mss = ajListNew();

    markersynonymadaptorFetchAllbyStatement(dba, statement, mss);

    if (ajListGetLength(mss) > 1)
        ajWarn("ensMarkersynonymadaptorFetchByIdentifier got more than one "
               "Ensembl Marker Synonym for identifier %u.\n",
               identifier);

    ajListPop(mss, (void **) Pms);

    while (ajListPop(mss, (void **) &ms))
        ensMarkersynonymDel(&ms);

    ajListFree(&mss);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPMarkermaplocation] Ensembl Marker Map Location ***********
**
** @nam2rule Markermaplocation Functions for manipulating
** Ensembl Marker Map Location objects
**
** @cc Bio::EnsEMBL::Map::MapLocation
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Marker Map Location by pointer.
** It is the responsibility of the user to first destroy any previous
** Marker Map Location. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
** @argrule Ini ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @argrule Ini mapname [AjPStr] Map name
** @argrule Ini chrname [AjPStr] Chromosome name
** @argrule Ini position [AjPStr] Position
** @argrule Ini lodscore [float] LOD score
** @argrule Ref mml [EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @valrule * [EnsPMarkermaplocation] Ensembl Marker Map Location or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMarkermaplocationNewCpy *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [EnsPMarkermaplocation] Ensembl Marker Map Location or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkermaplocation ensMarkermaplocationNewCpy(
    const EnsPMarkermaplocation mml)
{
    EnsPMarkermaplocation pthis = NULL;

    if (!mml)
        return NULL;

    AJNEW0(pthis);

    pthis->Markersynonym = ensMarkersynonymNewRef(mml->Markersynonym);

    if (mml->Mapname)
        pthis->Mapname = ajStrNewRef(mml->Mapname);

    if (mml->Chromosomename)
        pthis->Chromosomename = ajStrNewRef(mml->Chromosomename);

    if (mml->Position)
        pthis->Position = ajStrNewRef(mml->Position);

    pthis->Lodscore = mml->Lodscore;
    pthis->Use      = 1U;

    return pthis;
}




/* @func ensMarkermaplocationNewIni *******************************************
**
** Constructor for an Ensembl Marker Map Location with initial values.
**
** @cc Bio::EnsEMBL::Map::MapLocation::new
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [u] mapname [AjPStr] Map name
** @param [u] chrname [AjPStr] Chromosome name
** @param [u] position [AjPStr] Position
** @param [r] lodscore [float] LOD score
**
** @return [EnsPMarkermaplocation] Ensembl Marker Map Location or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkermaplocation ensMarkermaplocationNewIni(EnsPMarkersynonym ms,
                                                 AjPStr mapname,
                                                 AjPStr chrname,
                                                 AjPStr position,
                                                 float lodscore)
{
    EnsPMarkermaplocation mml = NULL;

    AJNEW0(mml);

    mml->Markersynonym = ensMarkersynonymNewRef(ms);

    if (mapname)
        mml->Mapname = ajStrNewRef(mapname);

    if (chrname)
        mml->Chromosomename = ajStrNewRef(chrname);

    if (position)
        mml->Position = ajStrNewRef(position);

    mml->Lodscore = lodscore;
    mml->Use      = 1U;

    return mml;
}




/* @func ensMarkermaplocationNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [EnsPMarkermaplocation] Ensembl Marker Map Location or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkermaplocation ensMarkermaplocationNewRef(EnsPMarkermaplocation mml)
{
    if (!mml)
        return NULL;

    mml->Use++;

    return mml;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Map Location
**
** @argrule * Pmml [EnsPMarkermaplocation*] Ensembl Marker Map Location address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkermaplocationDel **********************************************
**
** Default destructor for an Ensembl Marker Map Location.
**
** @param [d] Pmml [EnsPMarkermaplocation*] Ensembl Marker Map Location address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMarkermaplocationDel(EnsPMarkermaplocation *Pmml)
{
    EnsPMarkermaplocation pthis = NULL;

    if (!Pmml)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMarkermaplocationDel"))
    {
        ajDebug("ensMarkermaplocationDel\n"
                "  *Pmml %p\n",
                *Pmml);

        ensMarkermaplocationTrace(*Pmml, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmml)
        return;

    pthis = *Pmml;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmml = NULL;

        return;
    }

    ensMarkersynonymDel(&pthis->Markersynonym);

    ajStrDel(&pthis->Mapname);
    ajStrDel(&pthis->Chromosomename);
    ajStrDel(&pthis->Position);

    AJFREE(pthis);

    *Pmml = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Get Return Marker Map Location attribute(s)
** @nam4rule Chromosomename Return the chromosome name
** @nam4rule Lodscore Return the LOD score
** @nam4rule Mapname Return the map name
** @nam4rule Markersynonym Return the Ensembl Marker Synonym
** @nam4rule Position Return the position
**
** @argrule * mml [const EnsPMarkermaplocation] Marker Map Location
**
** @valrule Chromosomename [AjPStr] Chromosome name or NULL
** @valrule Lodscore [float] LOD score or 0.0F
** @valrule Mapname [AjPStr] Map name or NULL
** @valrule Markersynonym [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
** @valrule Position [AjPStr] Position or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkermaplocationGetChromosomename ********************************
**
** Get the chromosome name member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::chromosome_name
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjPStr] Chromosome name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensMarkermaplocationGetChromosomename(const EnsPMarkermaplocation mml)
{
    return (mml) ? mml->Chromosomename : NULL;
}




/* @func ensMarkermaplocationGetLodscore **************************************
**
** Get the LOD score member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::lod_score
** @param [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [float] LOD score or 0.0F
**
** @release 6.4.0
** @@
******************************************************************************/

float ensMarkermaplocationGetLodscore(const EnsPMarkermaplocation mml)
{
    return (mml) ? mml->Lodscore : 0.0F;
}




/* @func ensMarkermaplocationGetMapname ***************************************
**
** Get the map name member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::map_name
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjPStr] Map name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensMarkermaplocationGetMapname(const EnsPMarkermaplocation mml)
{
    return (mml) ? mml->Mapname : NULL;
}




/* @func ensMarkermaplocationGetMarkersynonym *********************************
**
** Get the Ensembl Marker Synonym member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::name
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkermaplocationGetMarkersynonym(
    const EnsPMarkermaplocation mml)
{
    return (mml) ? mml->Markersynonym : NULL;
}




/* @func ensMarkermaplocationGetPosition **************************************
**
** Get the position member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::position
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjPStr] Position or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensMarkermaplocationGetPosition(const EnsPMarkermaplocation mml)
{
    return (mml) ? mml->Position : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Set Set one member of an Ensembl Marker Map Location
** @nam4rule Chromosomename Set the chromosome name
** @nam4rule Lodscore Set the LOD score
** @nam4rule Mapname Set the map name
** @nam4rule Markersynonym Set the Ensembl Marker Synonym
** @nam4rule Position Set the position
**
** @argrule * mml [EnsPMarkermaplocation] Ensembl Marker Map Location object
** @argrule Chromosomename chrname [AjPStr] Chromosome name
** @argrule Lodscore lodscore [float] LOD score
** @argrule Mapname mapname [AjPStr] Map name
** @argrule Markersynonym ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @argrule Position position [AjPStr] Position
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMarkermaplocationSetChromosomename ********************************
**
** Set the chromosome name member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::chromosome_name
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] chrname [AjPStr] Chromosome name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetChromosomename(EnsPMarkermaplocation mml,
                                             AjPStr chrname)
{
    if (!mml)
        return ajFalse;

    ajStrDel(&mml->Chromosomename);

    mml->Chromosomename = ajStrNewRef(chrname);

    return ajTrue;
}




/* @func ensMarkermaplocationSetLodscore **************************************
**
** Set the LOD score member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::lod_score
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [r] lodscore [float] LOD score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetLodscore(EnsPMarkermaplocation mml,
                                       float lodscore)
{
    if (!mml)
        return ajFalse;

    mml->Lodscore = lodscore;

    return ajTrue;
}




/* @func ensMarkermaplocationSetMapname ***************************************
**
** Set the map name member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::map_name
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] mapname [AjPStr] Map name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetMapname(EnsPMarkermaplocation mml,
                                      AjPStr mapname)
{
    if (!mml)
        return ajFalse;

    ajStrDel(&mml->Mapname);

    mml->Mapname = ajStrNewRef(mapname);

    return ajTrue;
}




/* @func ensMarkermaplocationSetMarkersynonym *********************************
**
** Set the Ensembl Marker Synonym member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::name
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetMarkersynonym(EnsPMarkermaplocation mml,
                                            EnsPMarkersynonym ms)
{
    if (!mml)
        return ajFalse;

    ensMarkersynonymDel(&mml->Markersynonym);

    mml->Markersynonym = ensMarkersynonymNewRef(ms);

    return ajTrue;
}




/* @func ensMarkermaplocationSetPosition **************************************
**
** Set the position member of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::position
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] position [AjPStr] Position
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetPosition(EnsPMarkermaplocation mml,
                                       AjPStr position)
{
    if (!mml)
        return ajFalse;

    ajStrDel(&mml->Position);

    mml->Position = ajStrNewRef(position);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Trace Report Ensembl Marker Map Location members to debug file.
**
** @argrule Trace mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkermaplocationTrace ********************************************
**
** Trace an Ensembl Marker Map Location.
**
** @param [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationTrace(const EnsPMarkermaplocation mml,
                                 ajuint level)
{
    AjPStr indent = NULL;

    if (!mml)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMarkermaplocationTrace %p\n"
            "%S  Markersynonym %p\n"
            "%S  Mapname '%S'\n"
            "%S  Chromosomename '%S'\n"
            "%S  Position '%S'\n"
            "%S  Lodscore %f\n"
            "%S  Use %u\n",
            indent, mml,
            indent, mml->Use,
            indent, mml->Markersynonym,
            indent, mml->Mapname,
            indent, mml->Chromosomename,
            indent, mml->Lodscore);

    ensMarkersynonymTrace(mml->Markersynonym, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Calculate Calculate Ensembl Marker Map Location values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkermaplocationCalculateMemsize *********************************
**
** Calculate the memory size in bytes of an Ensembl Marker Map Location.
**
** @param [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMarkermaplocationCalculateMemsize(const EnsPMarkermaplocation mml)
{
    size_t size = 0;

    if (!mml)
        return 0;

    size += sizeof (EnsOMarkermaplocation);

    size += ensMarkersynonymCalculateMemsize(mml->Markersynonym);

    if (mml->Mapname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mml->Mapname);
    }

    if (mml->Chromosomename)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mml->Chromosomename);
    }

    if (mml->Position)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mml->Position);
    }

    return size;
}




/* @datasection [EnsPDatabaseadaptor] Ensembl Database Adaptor ****************
**
** @nam2rule Markermaplocationadaptor Functions for manipulating
** Ensembl Marker Map Location Adaptor objects
**
******************************************************************************/




/* @funcstatic markermaplocationadaptorFetchAllbyStatement ********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Marker Map Location objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] mmls [AjPList] AJAX List of Ensembl Marker Map Location objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool markermaplocationadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mmls)
{
    float lodscore = 0.0F;

    ajuint msid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr msname   = NULL;
    AjPStr mssource = NULL;
    AjPStr chrname  = NULL;
    AjPStr position = NULL;
    AjPStr mapname  = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPMarkermaplocation mml = NULL;

    if (!dba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!mmls)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        chrname  = ajStrNew();
        position = ajStrNew();
        lodscore = 0.0F;
        mapname  = ajStrNew();
        msid     = 0;
        mssource = ajStrNew();
        msname   = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &chrname);
        ajSqlcolumnToStr(sqlr, &position);
        ajSqlcolumnToFloat(sqlr, &lodscore);
        ajSqlcolumnToStr(sqlr, &mapname);
        ajSqlcolumnToUint(sqlr, &msid);
        ajSqlcolumnToStr(sqlr, &mssource);
        ajSqlcolumnToStr(sqlr, &msname);

        ms = ensMarkersynonymNewIni(msid, mssource, msname);

        mml = ensMarkermaplocationNewIni(ms,
                                         mapname,
                                         chrname,
                                         position,
                                         lodscore);

        ajListPushAppend(mmls, (void *) mml);

        ensMarkersynonymDel(&ms);

        ajStrDel(&chrname);
        ajStrDel(&position);
        ajStrDel(&mapname);
        ajStrDel(&msname);
        ajStrDel(&mssource);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Marker Map Location Adaptor objects from an
** Ensembl SQL database.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Fetch Fetch Ensembl Marker Synonym object(s)
** @nam4rule All Fetch all Ensembl Marker Map Location objects
** @nam4rule Allby Fetch all Ensembl Marker Map Location objects matching a
**                 criterion
** @nam5rule Markeridentifier Fetch all by an Ensembl Marker identifier
** @nam4rule By Fetch one Ensembl Marker Map Location object matching a
**              criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Marker Map Location Adaptor
** @argrule AllbyMarkeridentifier markerid [ajuint] Ensembl Marker identifier
** @argrule AllbyMarkeridentifier mmls [AjPList] AJAX List of
**                                         Ensembl Marker Map Location objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMarkermaplocationadaptorFetchAllbyMarkeridentifier ****************
**
** Fetch all Ensembl Marker Map Location objects by an
** Ensembl Marker identifier.
**
** The caller is responsible for deleting the Ensembl Marker Map Location
** objects before deleting the AJAX List.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] markerid [ajuint] Ensembl Marker identifier
** @param [u] mmls [AjPList] AJAX List of Ensembl Marker Map Location objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkermaplocationadaptorFetchAllbyMarkeridentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mmls)
{
    AjBool result = AJFALSE;

    AjPStr statement = NULL;

    if (!dba)
        return ajFalse;

    if (!markerid)
        return ajFalse;

    if (!mmls)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "marker_map_location.chromosome_name, "
        "marker_map_location.position, "
        "marker_map_location.lod_score, "
        "map.map_name, "
        "marker_synonym.marker_synonym_id, "
        "marker_synonym.source, "
        "marker_synonym.name, "
        "FROM "
        "marker_map_location, "
        "map, "
        "marker_synonym "
        "WHERE "
        "marker_map_location.marker_id = %u "
        "AND "
        "marker_map_location.map_id = map.map_id "
        "AND "
        "marker_map_location.marker_synonym_id = "
        "marker_synonym.marker_synonym_id",
        markerid);

    result = markermaplocationadaptorFetchAllbyStatement(dba, statement, mmls);

    ajStrDel(&statement);

    return result;
}




/* @datasection [EnsPMarker] Ensembl Marker ***********************************
**
** @nam2rule Marker Functions for manipulating Ensembl Marker objects
**
** @cc Bio::EnsEMBL::Map::Marker
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Marker by pointer.
** It is the responsibility of the user to first destroy any previous
** Markers. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMarker]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy marker [const EnsPMarker] Ensembl Marker
** @argrule Ini ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini display [EnsPMarkersynonym] Ensembl Marker Synonym
** @argrule Ini lprimer [AjPStr] Left primer
** @argrule Ini rprimer [AjPStr] Right primer
** @argrule Ini mss [AjPList] AJAX List of Ensembl Marker Synonym objects
** @argrule Ini mmls [AjPList] AJAX List of Ensembl Marker Map Location objects
** @argrule Ini mindistance [ajuint] Minimum primer distance
** @argrule Ini maxdistance [ajuint] Maximum primer distance
** @argrule Ini type [EnsEMarkerType] Type
** @argrule Ini priority [ajint] Priority
** @argrule Ref marker [EnsPMarker] Ensembl Marker
**
** @valrule * [EnsPMarker] Ensembl Marker or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMarkerNewCpy ******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsPMarker] Ensembl Marker or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarker ensMarkerNewCpy(const EnsPMarker marker)
{
    AjIList iter = NULL;

    EnsPMarker pthis = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    if (!marker)
        return NULL;

    AJNEW0(pthis);

    pthis->Use            = 1U;
    pthis->Identifier     = marker->Identifier;
    pthis->Adaptor        = marker->Adaptor;
    pthis->Displaysynonym = ensMarkersynonymNewRef(marker->Displaysynonym);

    if (marker->PrimerLeft)
        pthis->PrimerLeft = ajStrNewRef(marker->PrimerLeft);

    if (marker->PrimerRight)
        pthis->PrimerRight = ajStrNewRef(marker->PrimerRight);

    /* Copy the AJAX List of Ensembl Marker Synonym objects. */

    if (marker->Markersynonyms && ajListGetLength(marker->Markersynonyms))
    {
        pthis->Markersynonyms = ajListNew();

        iter = ajListIterNew(marker->Markersynonyms);

        while (!ajListIterDone(iter))
        {
            ms = (EnsPMarkersynonym) ajListIterGet(iter);

            if (ms)
                ajListPushAppend(pthis->Markersynonyms,
                                 (void *) ensMarkersynonymNewRef(ms));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Markersynonyms = NULL;

    /* Copy the AJAX List of Ensembl Marker Map Location objects. */

    if (marker->Markermaplocations &&
        ajListGetLength(marker->Markermaplocations))
    {
        pthis->Markermaplocations = ajListNew();

        iter = ajListIterNew(marker->Markermaplocations);

        while (!ajListIterDone(iter))
        {
            mml = (EnsPMarkermaplocation) ajListIterGet(iter);

            if (mml)
                ajListPushAppend(pthis->Markermaplocations,
                                 (void *) ensMarkermaplocationNewRef(mml));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Markermaplocations = NULL;

    pthis->DistanceMinimum = marker->DistanceMinimum;

    pthis->DistanceMaximum = marker->DistanceMaximum;

    pthis->Type = marker->Type;

    pthis->Priority = marker->Priority;

    return pthis;
}




/* @func ensMarkerNewIni ******************************************************
**
** Constructor for an Ensembl Marker with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Map::Marker::new
** @param [u] display [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [u] lprimer [AjPStr] Left primer
** @param [u] rprimer [AjPStr] Right primer
** @param [u] mss [AjPList] AJAX List of Ensembl Marker Synonym objects
** @param [u] mmls [AjPList] AJAX List of Ensembl Marker Map Location objects
** @param [r] mindistance [ajuint] Minimum primer distance
** @param [r] maxdistance [ajuint] Maximum primer distance
** @param [u] type [EnsEMarkerType] Type
** @param [r] priority [ajint] Priority
**
** @return [EnsPMarker] Ensembl Marker or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarker ensMarkerNewIni(EnsPMarkeradaptor ma,
                           ajuint identifier,
                           EnsPMarkersynonym display,
                           AjPStr lprimer,
                           AjPStr rprimer,
                           AjPList mss,
                           AjPList mmls,
                           ajuint mindistance,
                           ajuint maxdistance,
                           EnsEMarkerType type,
                           ajint priority)
{
    AjIList iter = NULL;

    EnsPMarker marker = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    AJNEW0(marker);

    marker->Use            = 1U;
    marker->Identifier     = identifier;
    marker->Adaptor        = ma;
    marker->Displaysynonym = ensMarkersynonymNewRef(display);

    if (lprimer)
        marker->PrimerLeft = ajStrNewRef(lprimer);

    if (rprimer)
        marker->PrimerRight = ajStrNewRef(rprimer);

    /* Copy the AJAX List of Ensembl Marker Synonym objects. */

    if (mss && ajListGetLength(mss))
    {
        marker->Markersynonyms = ajListNew();

        iter = ajListIterNew(mss);

        while (!ajListIterDone(iter))
        {
            ms = (EnsPMarkersynonym) ajListIterGet(iter);

            if (ms)
                ajListPushAppend(marker->Markersynonyms,
                                 (void *) ensMarkersynonymNewRef(ms));
        }

        ajListIterDel(&iter);
    }
    else
        marker->Markersynonyms = NULL;

    /* Copy the AJAX List of Ensembl Marker Map Location objects. */

    if (mmls && ajListGetLength(mmls))
    {
        marker->Markermaplocations = ajListNew();

        iter = ajListIterNew(mmls);

        while (!ajListIterDone(iter))
        {
            mml = (EnsPMarkermaplocation) ajListIterGet(iter);

            if (mml)
                ajListPushAppend(marker->Markermaplocations,
                                 (void *) ensMarkermaplocationNewRef(mml));
        }

        ajListIterDel(&iter);
    }
    else
        marker->Markermaplocations = NULL;

    marker->DistanceMinimum = mindistance;

    marker->DistanceMaximum = maxdistance;

    marker->Type = type;

    marker->Priority = priority;

    return marker;
}




/* @func ensMarkerNewRef ******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [EnsPMarker] Ensembl Marker or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarker ensMarkerNewRef(EnsPMarker marker)
{
    if (!marker)
        return NULL;

    marker->Use++;

    return marker;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Del Destroy (free) an Ensembl Marker
**
** @argrule * Pmarker [EnsPMarker*] Ensembl Marker address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkerDel *********************************************************
**
** Default destructor for an Ensembl Marker.
**
** @param [d] Pmarker [EnsPMarker*] Ensembl Marker address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMarkerDel(EnsPMarker *Pmarker)
{
    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPMarker pthis = NULL;

    if (!Pmarker)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMarkerDel"))
    {
        ajDebug("ensMarkerDel\n"
                "  *Pmarker %p\n",
                *Pmarker);

        ensMarkerTrace(*Pmarker, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmarker)
        return;

    pthis = *Pmarker;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmarker = NULL;

        return;
    }

    ajStrDel(&pthis->PrimerLeft);
    ajStrDel(&pthis->PrimerRight);

    ensMarkersynonymDel(&pthis->Displaysynonym);

    while (ajListPop(pthis->Markersynonyms, (void **) &ms))
        ensMarkersynonymDel(&ms);

    ajListFree(&pthis->Markersynonyms);

    while (ajListPop(pthis->Markermaplocations, (void **) &mml))
        ensMarkermaplocationDel(&mml);

    ajListFree(&pthis->Markermaplocations);

    AJFREE(pthis);

    *Pmarker = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Get Return Marker attribute(s)
** @nam4rule Adaptor Return the Ensembl Marker Adaptor
** @nam4rule Displaysynonym Return the display synonym
** @nam4rule Distance Return a distance
** @nam5rule Maximum Return the maximum distance
** @nam5rule Minimum Return the minimum distance
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Primer Return a primer
** @nam5rule Left Return the left primer
** @nam5rule Right Return the right primer
** @nam4rule Priority return the priority
** @nam4rule Type Return the type
**
** @argrule * marker [const EnsPMarker] Marker
**
** @valrule Adaptor [EnsPMarkeradaptor] Ensembl Marker Adaptor or NULL
** @valrule Displaysynonym [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
** @valrule DistanceMaximum [ajuint] Maximum distance or 0U
** @valrule DistanceMinimum [ajuint] Minimum distance or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule PrimerLeft [AjPStr] Left primer or NULL
** @valrule PrimerRight [AjPStr] Right primer or NULL
** @valrule Priority [ajint] Priority or 0
** @valrule Type [EnsEMarkerType] Type or ensEMarkerTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerGetAdaptor **************************************************
**
** Get the Ensembl Marker Adaptor member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsPMarkeradaptor] Ensembl Marker Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkeradaptor ensMarkerGetAdaptor(const EnsPMarker marker)
{
    return (marker) ? marker->Adaptor : NULL;
}




/* @func ensMarkerGetDisplaysynonym *******************************************
**
** Get the display synonym member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::display_MarkerSynonym
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsPMarkersynonym] Display Ensembl Marker Synonym or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkerGetDisplaysynonym(const EnsPMarker marker)
{
    return (marker) ? marker->Displaysynonym : NULL;
}




/* @func ensMarkerGetDistanceMaximum ******************************************
**
** Get the maximum distance member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::max_primer_dist
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajuint] Maximum distance or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMarkerGetDistanceMaximum(const EnsPMarker marker)
{
    return (marker) ? marker->DistanceMaximum : 0U;
}




/* @func ensMarkerGetDistanceMinimum ******************************************
**
** Get the minimum distance member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::min_primer_dist
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajuint] Minimum distance or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMarkerGetDistanceMinimum(const EnsPMarker marker)
{
    return (marker) ? marker->DistanceMinimum : 0U;
}




/* @func ensMarkerGetIdentifier ***********************************************
**
** Get the SQL database-internal identifier member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensMarkerGetIdentifier(const EnsPMarker marker)
{
    return (marker) ? marker->Identifier : 0U;
}




/* @func ensMarkerGetPrimerLeft ***********************************************
**
** Get the left primer member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::left_primer
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [AjPStr] Left primer or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensMarkerGetPrimerLeft(const EnsPMarker marker)
{
    return (marker) ? marker->PrimerLeft : NULL;
}




/* @func ensMarkerGetPrimerRight **********************************************
**
** Get the right primer member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::right_primer
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [AjPStr] Right primer or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensMarkerGetPrimerRight(const EnsPMarker marker)
{
    return (marker) ? marker->PrimerRight : NULL;
}




/* @func ensMarkerGetPriority *************************************************
**
** Get the priority member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::priority
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajint] Priority or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMarkerGetPriority(const EnsPMarker marker)
{
    return (marker) ? marker->Priority : 0;
}




/* @func ensMarkerGetType *****************************************************
**
** Get the type member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::type
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsEMarkerType] Type or ensEMarkerTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEMarkerType ensMarkerGetType(const EnsPMarker marker)
{
    return (marker) ? marker->Type : ensEMarkerTypeNULL;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Marker object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPMarker]
**
** @nam3rule Load Return Ensembl Marker attribute(s) loaded on demand
** @nam4rule Markermaplocations Return all Ensembl Marker Map Location objects
** @nam4rule Markersynonyms Return all Ensembl Marker Synonym objects
**
** @argrule * marker [EnsPMarker] Ensembl Marker
**
** @valrule Markermaplocations [const AjPList] AJAX List of Ensembl Marker
**                                             Map Location objects
** @valrule Markersynonyms [const AjPList] AJAX List of
**                                         Ensembl Marker Synonym objects
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerLoadMarkermaplocations **************************************
**
** Load all Ensembl Marker Map Location objects of an Ensembl Marker.
**
** This is not a simple accessor function, it will fetch
** Ensembl Marker Map Location objects from the Ensembl Core database
** in case the AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Map::Marker::get_all_MapLocations
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [const AjPList] AJAX List of Ensembl Marker Map Location objects
**
** @release 6.4.0
** @@
** NOTE: The Perl API uses a
** Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_attributes method to load
** Bio::EnsEMBL::Map::MarkerSynonym and Bio::EnsEMBL::Map::MapLocation objects.
** This is not ideal as the method requires a Marker Adaptor and a Marker.
** There is no check that the MArker Adaptor and Marker refer to the same
** Ensembl Core database.
******************************************************************************/

const AjPList ensMarkerLoadMarkermaplocations(EnsPMarker marker)
{
    ajuint identifier = 0U;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!marker)
        return NULL;

    if (marker->Markermaplocations)
        return marker->Markermaplocations;

    ba = ensMarkeradaptorGetBaseadaptor(marker->Adaptor);

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    if (!dba)
        return NULL;

    identifier = ensMarkerGetIdentifier(marker);

    marker->Markermaplocations = ajListNew();

    ensMarkermaplocationadaptorFetchAllbyMarkeridentifier(
        dba,
        identifier,
        marker->Markermaplocations);

    return marker->Markermaplocations;
}




/* @func ensMarkerLoadMarkersynonyms ******************************************
**
** Load all Ensembl Marker Synonym objects of an Ensembl Marker.
**
** This is not a simple accessor function, it will fetch
** Ensembl Marker Synonym objects from the Ensembl SQL database
** in case the AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Map::Marker::get_all_MarkerSynonyms
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [const AjPList] AJAX List of Ensembl Marker Synonym objects
**
** @release 6.4.0
** @@
** NOTE: The Perl API uses a
** Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_attributes method to load
** Bio::EnsEMBL::Map::MarkerSynonym and Bio::EnsEMBL::Map::MapLocation objects.
** This is not ideal as the method requires a Marker Adaptor and a Marker.
** There is no check that the MArker Adaptor and Marker refer to the same
** Ensembl Core database.
******************************************************************************/

const AjPList ensMarkerLoadMarkersynonyms(EnsPMarker marker)
{
    ajuint identifier = 0U;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!marker)
        return NULL;

    if (marker->Markersynonyms)
        return marker->Markersynonyms;

    ba = ensMarkeradaptorGetBaseadaptor(marker->Adaptor);

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    if (!dba)
        return NULL;

    identifier = ensMarkerGetIdentifier(marker);

    marker->Markersynonyms = ajListNew();

    ensMarkersynonymadaptorFetchAllbyMarkeridentifier(dba,
                                                      identifier,
                                                      marker->Markersynonyms);

    return marker->Markersynonyms;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Set Set one member of an Ensembl Marker
** @nam4rule Adaptor Set the Ensembl Marker Adaptor
** @nam4rule Displaysynonym Set the display synonym
** @nam4rule Distance Set a distance
** @nam5rule Minimum Set the minimum distance
** @nam5rule Maximum Set the maximum distance
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Primer Set a primer
** @nam5rule Left Set the left primer
** @nam5rule Right Set the right primer
** @nam4rule Priority Set the priority
** @nam4rule Type Set the type
**
** @argrule * marker [EnsPMarker] Ensembl Marker object
** @argrule Adaptor ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @argrule Displaysynonym display [EnsPMarkersynonym] Display synonym
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule PrimerLeft lprimer [AjPStr] Left primer
** @argrule PrimerRight rprimer [AjPStr] Right primer
** @argrule DistanceMinimum mindistance [ajuint] Minimum distance
** @argrule DistanceMaximum maxdistance [ajuint] Maximum distance
** @argrule Priority priority [ajint] Priority
** @argrule Type type [EnsEMarkerType] Type
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMarkerSetAdaptor **************************************************
**
** Set the Ensembl Marker Adaptor member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerSetAdaptor(EnsPMarker marker,
                           EnsPMarkeradaptor ma)
{
    if (!marker)
        return ajFalse;

    marker->Adaptor = ma;

    return ajTrue;
}




/* @func ensMarkerSetDisplaysynonym *******************************************
**
** Set the display synonym member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::display_MarkerSynonym
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] display [EnsPMarkersynonym] Display synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerSetDisplaysynonym(EnsPMarker marker,
                                  EnsPMarkersynonym display)
{
    if (!marker)
        return ajFalse;

    ensMarkersynonymDel(&marker->Displaysynonym);

    marker->Displaysynonym = ensMarkersynonymNewRef(display);

    return ajTrue;
}




/* @func ensMarkerSetDistanceMaximum ******************************************
**
** Set the maximum distance member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::max_primer_dist
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] maxdistance [ajuint] Maximum distance
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerSetDistanceMaximum(EnsPMarker marker,
                                   ajuint maxdistance)
{
    if (!marker)
        return ajFalse;

    marker->DistanceMaximum = maxdistance;

    return ajTrue;
}




/* @func ensMarkerSetDistanceMinimum ******************************************
**
** Set the minimum distance member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::min_primer_dist
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] mindistance [ajuint] Minimum distance
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerSetDistanceMinimum(EnsPMarker marker,
                                   ajuint mindistance)
{
    if (!marker)
        return ajFalse;

    marker->DistanceMinimum = mindistance;

    return ajTrue;
}




/* @func ensMarkerSetIdentifier ***********************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerSetIdentifier(EnsPMarker marker,
                              ajuint identifier)
{
    if (!marker)
        return ajFalse;

    marker->Identifier = identifier;

    return ajTrue;
}




/* @func ensMarkerSetPrimerLeft ***********************************************
**
** Set the left primer member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::left_primer
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] lprimer [AjPStr] Left primer
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerSetPrimerLeft(EnsPMarker marker,
                              AjPStr lprimer)
{
    if (!marker)
        return ajFalse;

    ajStrDel(&marker->PrimerLeft);

    if (lprimer)
        marker->PrimerLeft = ajStrNewRef(lprimer);

    return ajTrue;
}




/* @func ensMarkerSetPrimerRight **********************************************
**
** Set the right primer member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::right_primer
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] rprimer [AjPStr] Right primer
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerSetPrimerRight(EnsPMarker marker,
                               AjPStr rprimer)
{
    if (!marker)
        return ajFalse;

    ajStrDel(&marker->PrimerRight);

    if (rprimer)
        marker->PrimerRight = ajStrNewRef(rprimer);

    return ajTrue;
}




/* @func ensMarkerSetPriority *************************************************
**
** Set the priority member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::priority
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] priority [ajint] Priority
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerSetPriority(EnsPMarker marker,
                            ajint priority)
{
    if (!marker)
        return ajFalse;

    marker->Priority = priority;

    return ajTrue;
}




/* @func ensMarkerSetType *****************************************************
**
** Set the type member of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::type
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] type [EnsEMarkerType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerSetType(EnsPMarker marker,
                        EnsEMarkerType type)
{
    if (!marker)
        return ajFalse;

    marker->Type = type;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Trace Report Ensembl Marker members to debug file.
**
** @argrule Trace marker [const EnsPMarker] Ensembl Marker
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkerTrace *******************************************************
**
** Trace an Ensembl Marker.
**
** @param [r] marker [const EnsPMarker] Ensembl Marker
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerTrace(const EnsPMarker marker, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    if (!marker)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMarkerTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  PrimerLeft '%S'\n"
            "%S  PrimerRight '%S'\n"
            "%S  Type '%S'\n"
            "%S  Displaysynonym %p\n"
            "%S  Markersynonyms %p\n"
            "%S  Markermaplocations %p\n"
            "%S  DistanceMinimum %u\n"
            "%S  DistanceMaximum %u\n"
            "%S  Priority %d\n",
            indent, marker,
            indent, marker->Use,
            indent, marker->Identifier,
            indent, marker->Adaptor,
            indent, marker->PrimerLeft,
            indent, marker->PrimerRight,
            indent, marker->Type,
            indent, marker->Displaysynonym,
            indent, marker->Markersynonyms,
            indent, marker->Markermaplocations,
            indent, marker->DistanceMinimum,
            indent, marker->DistanceMaximum,
            indent, marker->Priority);

    ensMarkersynonymTrace(marker->Displaysynonym, level + 1);

    /* Trace the AJAX List of Ensembl Marker Synonym objects. */

    iter = ajListIterNew(marker->Markersynonyms);

    while (!ajListIterDone(iter))
    {
        ms = (EnsPMarkersynonym) ajListIterGet(iter);

        ensMarkersynonymTrace(ms, level + 1);
    }

    ajListIterDel(&iter);

    /* Trace the AJAX List of Ensembl Marker Map Location objects. */

    iter = ajListIterNew(marker->Markermaplocations);

    while (!ajListIterDone(iter))
    {
        mml = (EnsPMarkermaplocation) ajListIterGet(iter);

        ensMarkermaplocationTrace(mml, level + 1);
    }

    ajListIterDel(&iter);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Calculate Calculate Ensembl Marker values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * marker [const EnsPMarker] Ensembl Marker
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkerCalculateMemsize ********************************************
**
** Calculate the memory size in bytes of an Ensembl Marker.
**
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMarkerCalculateMemsize(const EnsPMarker marker)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    if (!marker)
        return 0;

    size += sizeof (EnsOMarker);

    size += ensMarkersynonymCalculateMemsize(marker->Displaysynonym);

    if (marker->PrimerLeft)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(marker->PrimerLeft);
    }

    if (marker->PrimerRight)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(marker->PrimerRight);
    }

    /* Summarise the AJAX List of Ensembl Marker Synonym objects. */

    iter = ajListIterNew(marker->Markersynonyms);

    while (!ajListIterDone(iter))
    {
        ms = (EnsPMarkersynonym) ajListIterGet(iter);

        size += ensMarkersynonymCalculateMemsize(ms);
    }

    ajListIterDel(&iter);

    /* Summarise the AJAX List of Ensembl Marker Map Location objects. */

    iter = ajListIterNew(marker->Markermaplocations);

    while (!ajListIterDone(iter))
    {
        mml = (EnsPMarkermaplocation) ajListIterGet(iter);

        size += ensMarkermaplocationCalculateMemsize(mml);
    }

    ajListIterDel(&iter);

    return size;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Add Add one object to an Ensembl Marker
** @nam4rule Markermaplocation Add an Ensembl Marker Map Location
** @nam4rule Markersynonym Add an Ensembl Marker Synonym
**
** @argrule * marker [EnsPMarker] Ensembl Marker object
** @argrule Markermaplocation mml [EnsPMarkermaplocation] Ensembl Marker
**                                                        Map Location
** @argrule Markersynonym ms [EnsPMarkersynonym] Ensembl Marker Synonym
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMarkerAddMarkermaplocation ****************************************
**
** Add an Ensembl Marker Map Location to an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::add_MapLocations
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerAddMarkermaplocation(EnsPMarker marker,
                                     EnsPMarkermaplocation mml)
{
    if (!marker)
        return ajFalse;

    if (!mml)
        return ajFalse;

    if (!marker->Markermaplocations)
        marker->Markermaplocations = ajListNew();

    ajListPushAppend(marker->Markermaplocations,
                     (void *) ensMarkermaplocationNewRef(mml));

    return ajTrue;
}




/* @func ensMarkerAddMarkersynonym ********************************************
**
** Add an Ensembl Marker Synonym to an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::add_MarkerSynonym
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerAddMarkersynonym(EnsPMarker marker,
                                 EnsPMarkersynonym ms)
{
    if (!marker)
        return ajFalse;

    if (!ms)
        return ajFalse;

    if (!marker->Markersynonyms)
        marker->Markersynonyms = ajListNew();

    ajListPushAppend(marker->Markersynonyms,
                     (void *) ensMarkersynonymNewRef(ms));

    return ajTrue;
}




/* @section member removal ****************************************************
**
** Functions for clearing members of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Clear Clear members of an Ensembl Marker
** @nam4rule Markermaplocations Clear all Ensembl Marker Map Location objects
** @nam4rule Markersynonyms Clear all Ensembl Marker Synonym objects
**
** @argrule * marker [EnsPMarker] Ensembl Marker object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMarkerClearMarkermaplocations *************************************
**
** Clear all Ensembl Marker Map Location objects of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::flush_MapLocations
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerClearMarkermaplocations(EnsPMarker marker)
{
    EnsPMarkermaplocation mml = NULL;

    if (!marker)
        return ajFalse;

    while (ajListPop(marker->Markermaplocations, (void **) &mml))
        ensMarkermaplocationDel(&mml);

    return ajTrue;
}




/* @func ensMarkerClearMarkersynonyms *****************************************
**
** Clear all Ensembl Marker Synonym objects of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::flush_MarkerSynonyms
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerClearMarkersynonyms(EnsPMarker marker)
{
    EnsPMarkersynonym ms = NULL;

    if (!marker)
        return ajFalse;

    while (ajListPop(marker->Markersynonyms, (void **) &ms))
        ensMarkersynonymDel(&ms);

    return ajTrue;
}




/* @datasection [EnsEMarkerType] Ensembl Marker Type **************************
**
** @nam2rule Marker Functions for manipulating
** Ensembl Marker objects
** @nam3rule MarkerType Functions for manipulating
** Ensembl Marker Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Marker Type enumeration.
**
** @fdata [EnsEMarkerType]
**
** @nam4rule From Ensembl Marker Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEMarkerType]
** Ensembl Marker Type enumeration or ensEMarkerTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkerTypeFromStr *************************************************
**
** Convert an AJAX String into an Ensembl Marker Type enumeration.
**
** @param [r] type [const AjPStr] Ensembl Marker Type string
**
** @return [EnsEMarkerType]
** Ensembl Marker Type enumeration or ensEMarkerTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEMarkerType ensMarkerTypeFromStr(const AjPStr type)
{
    register EnsEMarkerType i = ensEMarkerTypeNULL;

    EnsEMarkerType etype = ensEMarkerTypeNULL;

    for (i = ensEMarkerTypeNULL;
         markerKType[i];
         i++)
        if (ajStrMatchC(type, markerKType[i]))
            etype = i;

    if (!etype)
        ajDebug("ensMarkerTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Marker Type enumeration.
**
** @fdata [EnsEMarkerType]
**
** @nam4rule To   Return Ensembl Marker Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To type [EnsEMarkerType] Ensembl Marker Type enumeration
**
** @valrule Char [const char*] Ensembl Marker Type C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensMarkerTypeToChar **************************************************
**
** Convert an Ensembl Marker Type enumeration into a C-type (char *) string.
**
** @param [u] type [EnsEMarkerType] Ensembl Marker Type enumeration
**
** @return [const char*] Ensembl Marker Type C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensMarkerTypeToChar(EnsEMarkerType type)
{
    register EnsEMarkerType i = ensEMarkerTypeNULL;

    for (i = ensEMarkerTypeNULL;
         markerKType[i] && (i < type);
         i++);

    if (!markerKType[i])
        ajDebug("ensMarkerTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Marker Type enumeration %d.\n",
                type);

    return markerKType[i];
}




/* @datasection [EnsPMarkeradaptor] Ensembl Marker Adaptor ********************
**
** @nam2rule Markeradaptor Functions for manipulating
** Ensembl Marker Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic markeradaptorFetchAllbyStatement *******************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Marker objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] markers [AjPList] AJAX List of Ensembl Markers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: This implementation of the Ensembl Marker Adaptor immediately selects
** all Marker Synonym objects via a LEFT JOIN clause, sets the special display
** Marker Synomym and adds all others to to the AJAX List of Marker Synonym
** objects.
** The Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_all and
** Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_by_dbID methods select only
** the display Marker Synonym via
** marker.display_marker_synonym = marker_synonym.marker_synonym_id.
** All other Ensembl Marker Synonym objects are loaded via the
** Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_attributes method.
******************************************************************************/

static AjBool markeradaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList markers)
{
    ajint priority = 0;

    ajuint current     = 0U;
    ajuint identifier  = 0U;
    ajuint displaymsid = 0U;
    ajuint mindistance = 0U;
    ajuint maxdistance = 0U;
    ajuint msid        = 0U;

    EnsEMarkerType etype = ensEMarkerTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr lprimer  = NULL;
    AjPStr rprimer  = NULL;
    AjPStr type     = NULL;
    AjPStr msname   = NULL;
    AjPStr mssource = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMarker marker    = NULL;
    EnsPMarkeradaptor ma = NULL;

    EnsPMarkersynonym ms = NULL;

    if (ajDebugTest("markeradaptorFetchAllbyStatement"))
        ajDebug("markeradaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  markers %p\n",
                ba,
                statement,
                am,
                slice,
                markers);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!markers)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ma = ensRegistryGetMarkeradaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        displaymsid = 0;
        lprimer     = ajStrNew();
        rprimer     = ajStrNew();
        mindistance = 0;
        maxdistance = 0;
        priority    = 0;
        type        = ajStrNew();
        msid        = 0;
        msname      = ajStrNew();
        mssource    = ajStrNew();

        etype = ensEMarkerTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &displaymsid);
        ajSqlcolumnToStr(sqlr, &lprimer);
        ajSqlcolumnToStr(sqlr, &rprimer);
        ajSqlcolumnToUint(sqlr, &mindistance);
        ajSqlcolumnToUint(sqlr, &maxdistance);
        ajSqlcolumnToInt(sqlr, &priority);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &msid);
        ajSqlcolumnToStr(sqlr, &msname);
        ajSqlcolumnToStr(sqlr, &mssource);

        if (current != identifier)
        {
            etype = ensMarkerTypeFromStr(type);

            marker = ensMarkerNewIni(ma,
                                     identifier,
                                     (EnsPMarkersynonym) NULL,
                                     lprimer,
                                     rprimer,
                                     (AjPList) NULL,
                                     (AjPList) NULL,
                                     mindistance,
                                     maxdistance,
                                     etype,
                                     priority);

            ajListPushAppend(markers, (void *) marker);

            current = identifier;
        }

        if (msid)
        {
            ms = ensMarkersynonymNewIni(msid, mssource, msname);

            if (msid == displaymsid)
                ensMarkerSetDisplaysynonym(marker, ms);
            else
                ensMarkerAddMarkersynonym(marker, ms);

            ensMarkersynonymDel(&ms);
        }

        ajStrDel(&lprimer);
        ajStrDel(&rprimer);
        ajStrDel(&type);
        ajStrDel(&msname);
        ajStrDel(&mssource);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Marker Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Marker Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMarkeradaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMarkeradaptor] Ensembl Marker Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMarkeradaptorNew **************************************************
**
** Default constructor for an Ensembl Marker Adaptor.
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
** @see ensRegistryGetMarkeradaptor
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMarkeradaptor] Ensembl Marker Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkeradaptor ensMarkeradaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        markeradaptorKTables,
        markeradaptorKColumns,
        markeradaptorKLeftjoin,
        (const char *) NULL,
        (const char *) NULL,
        &markeradaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Marker Adaptor object.
**
** @fdata [EnsPMarkeradaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Adaptor
**
** @argrule * Pma [EnsPMarkeradaptor*] Ensembl Marker Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkeradaptorDel **************************************************
**
** Default destructor for an Ensembl Marker Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pma [EnsPMarkeradaptor*] Ensembl Marker Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMarkeradaptorDel(EnsPMarkeradaptor *Pma)
{
	ensBaseadaptorDel(Pma);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Marker Adaptor object.
**
** @fdata [EnsPMarkeradaptor]
**
** @nam3rule Get Return Marker Adaptor attribute(s)
** @nam4rule GetBaseadaptor Return the Ensembl Base Adaptor
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ma [EnsPMarkeradaptor] Marker Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensMarkeradaptorGetBaseadaptor ***************************************
**
** Get the Ensembl Base Adaptor member of an Ensembl Marker Adaptor.
**
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBaseadaptor ensMarkeradaptorGetBaseadaptor(EnsPMarkeradaptor ma)
{
    return ma;
}




/* @func ensMarkeradaptorGetDatabaseadaptor ***********************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Marker Adaptor.
**
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMarkeradaptorGetDatabaseadaptor(EnsPMarkeradaptor ma)
{
    return ensBaseadaptorGetDatabaseadaptor(ma);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Marker objects from an
** Ensembl Core database.
**
** @fdata [EnsPMarkeradaptor]
**
** @nam3rule Fetch Fetch Ensembl Marker object(s)
** @nam4rule All Fetch all Ensembl Marker objects
** @nam4rule Allby Fetch all Ensembl Marker objects matching a criterion
** @nam5rule Synonym Fetch all by a Marker Synonym name and source
** @nam4rule By Fetch one Ensembl Marker object matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @argrule All markers [AjPList] AJAX List of Ensembl Marker objects
** @argrule AllbySynonym name [const AjPStr] Marker Synonym name
** @argrule AllbySynonym source [const AjPStr] Marker Synonym source
** @argrule AllbySynonym markers [AjPList] AJAX List of Ensembl Marker objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pmarker [EnsPMarker*] Ensembl Marker address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMarkeradaptorFetchAll *********************************************
**
** Fetch all Ensembl Markers.
**
** The caller is responsible for deleting the Ensembl Markers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_all
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @param [u] markers [AjPList] AJAX List of Ensembl Markers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkeradaptorFetchAll(EnsPMarkeradaptor ma,
                                AjPList markers)
{
    AjBool result = AJFALSE;

    if (!ma)
        return ajFalse;

    if (!markers)
        return ajFalse;

    result = ensBaseadaptorFetchAllbyConstraint(ma,
                                                (AjPStr) NULL,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                markers);

    return result;
}




/* @func ensMarkeradaptorFetchAllbySynonym ************************************
**
** Fetch all Ensembl Markers via a Marker Synonym name and source.
**
** The caller is responsible for deleting the Ensembl Markers before deleting
** the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_all_by_synonym
** @param [u]  ma      [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @param [r]  name    [const AjPStr] Marker Synonym name
** @param [rN] source  [const AjPStr] Marker Synonym source
** @param [w]  markers [AjPList] AJAX List of Ensembl Marker objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkeradaptorFetchAllbySynonym(EnsPMarkeradaptor ma,
                                         const AjPStr name,
                                         const AjPStr source,
                                         AjPList markers)
{
    char *txtname   = NULL;
    char *txtsource = NULL;

    ajuint identifier = 0U;

    ajuint *Pidentifier = NULL;

    AjBool *Pbool = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable table = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMarker marker = NULL;

    if (!ma)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!markers)
        return ajFalse;

    table = ajTableuintNew(0);

    ajTableSetDestroyvalue(table, (void (*)(void **)) &ajMemFree);

    dba = ensBaseadaptorGetDatabaseadaptor(ma);

    ensDatabaseadaptorEscapeC(dba, &txtname, name);

    statement = ajFmtStr(
        "SELECT "
        "marker_synonym.marker_id "
        "FROM "
        "marker_synonym "
        "WHERE "
        "marker_synonym.name = '%s'",
        txtname);

    ajCharDel(&txtname);

    if (source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        ajFmtPrintAppS(&statement,
                       " AND marker_synonym.source = '%s'",
                       txtsource);

        ajCharDel(&txtsource);
    }

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        marker = NULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);

        if (ajTableMatchV(table, (const void *) &identifier))
            continue;

        AJNEW0(Pidentifier);

        *Pidentifier = identifier;

        AJNEW0(Pbool);

        *Pbool = ajTrue;

        ajTablePut(table, (void *) Pidentifier, (void *) Pbool);

        ensMarkeradaptorFetchByIdentifier(ma, identifier, &marker);

        if (marker)
            ajListPushAppend(markers, (void *) marker);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    ajTableDel(&table);

    return ajTrue;
}




/* @func ensMarkeradaptorFetchByIdentifier ************************************
**
** Fetch an Ensembl Marker via its SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor::fetch_by_dbID
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pmarker [EnsPMarker*] Ensembl Marker address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkeradaptorFetchByIdentifier(EnsPMarkeradaptor ma,
                                         ajuint identifier,
                                         EnsPMarker *Pmarker)
{
    AjBool result = AJFALSE;

    AjPList markers = NULL;

    AjPStr constraint = NULL;

    EnsPMarker marker = NULL;

    if (!ma)
        return ajFalse;

    if (!Pmarker)
        return ajFalse;

    *Pmarker = NULL;

    constraint = ajFmtStr("marker.marker_id = %u", identifier);

    markers = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(ma,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                markers);

    if (ajListGetLength(markers) == 0)
        ajDebug("ensMarkeradaptorFetchByIdentifier got no Ensembl Marker "
                "for identifier %u.\n", identifier);

    if (ajListGetLength(markers) > 1)
        ajWarn("ensMarkeradaptorFetchByIdentifier got more than one "
               "Ensembl Marker for identifier %u.\n", identifier);

    ajListPop(markers, (void **) Pmarker);

    while (ajListPop(markers, (void **) &marker))
        ensMarkerDel(&marker);

    ajListFree(&markers);

    ajStrDel(&constraint);

    return result;
}




/* @datasection [EnsPMarkerfeature] Ensembl Marker Feature ********************
**
** @nam2rule Markerfeature Functions for manipulating
** Ensembl Marker Feature objects
**
** @cc Bio::EnsEMBL::Map::MarkerFeature
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Marker Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Marker Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mf [const EnsPMarkerfeature] Ensembl Marker Feature
** @argrule Ini mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini marker [EnsPMarker] Ensembl Marker
** @argrule Ini mapweight [ajint] Map weight
** @argrule Ref mf [EnsPMarkerfeature] Ensembl Marker Feature
**
** @valrule * [EnsPMarkerfeature] Ensembl Marker Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMarkerfeatureNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPMarkerfeature] Ensembl Marker Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkerfeature ensMarkerfeatureNewCpy(const EnsPMarkerfeature mf)
{
    EnsPMarkerfeature pthis = NULL;

    if (!mf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = mf->Identifier;
    pthis->Adaptor    = mf->Adaptor;
    pthis->Feature    = ensFeatureNewRef(mf->Feature);
    pthis->Marker     = ensMarkerNewRef(mf->Marker);
    pthis->Mapweight  = mf->Mapweight;

    return pthis;
}




/* @func ensMarkerfeatureNewIni ***********************************************
**
** Constructor for an Ensembl Marker Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Map::MarkerFeature::new
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] mapweight [ajint] Map weight
**
** @return [EnsPMarkerfeature] Ensembl Marker Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMarkerfeature ensMarkerfeatureNewIni(EnsPMarkerfeatureadaptor mfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         EnsPMarker marker,
                                         ajint mapweight)
{
    EnsPMarkerfeature mf = NULL;

    AJNEW0(mf);

    mf->Use        = 1U;
    mf->Identifier = identifier;
    mf->Adaptor    = mfa;
    mf->Feature    = ensFeatureNewRef(feature);
    mf->Marker     = ensMarkerNewRef(marker);
    mf->Mapweight  = mapweight;

    return mf;
}




/* @func ensMarkerfeatureNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPMarkerfeature] Ensembl Marker Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkerfeature ensMarkerfeatureNewRef(EnsPMarkerfeature mf)
{
    if (!mf)
        return NULL;

    mf->Use++;

    return mf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Feature
**
** @argrule * Pmf [EnsPMarkerfeature*] Ensembl Marker Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkerfeatureDel **************************************************
**
** Default destructor for an Ensembl Marker Feature.
**
** @param [d] Pmf [EnsPMarkerfeature*] Ensembl Marker Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMarkerfeatureDel(EnsPMarkerfeature *Pmf)
{
    EnsPMarkerfeature pthis = NULL;

    if (!Pmf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMarkerfeatureDel"))
    {
        ajDebug("ensMarkerfeatureDel\n"
                "  *Pmf %p\n",
                *Pmf);

        ensMarkerfeatureTrace(*Pmf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmf)
        return;

    pthis = *Pmf;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensMarkerDel(&pthis->Marker);

    AJFREE(pthis);

    *Pmf = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Get Return Marker Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Marker Feature Adaptor
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Marker Return the Ensembl Marker
** @nam4rule Mapweight return the map weight
**
** @argrule * mf [const EnsPMarkerfeature] Marker Feature
**
** @valrule Adaptor [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Marker [EnsPMarker] Ensembl Marker or NULL
** @valrule Mapweight [ajint] Map weight or 0
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerfeatureGetAdaptor *******************************************
**
** Get the Ensembl Marker Feature Adaptor member of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkerfeatureadaptor ensMarkerfeatureGetAdaptor(
    const EnsPMarkerfeature mf)
{
    return (mf) ? mf->Adaptor : NULL;
}




/* @func ensMarkerfeatureGetFeature *******************************************
**
** Get the Ensembl Feature member of an Ensembl Marker Feature.
**
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensMarkerfeatureGetFeature(const EnsPMarkerfeature mf)
{
    return (mf) ? mf->Feature : NULL;
}




/* @func ensMarkerfeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensMarkerfeatureGetIdentifier(const EnsPMarkerfeature mf)
{
    return (mf) ? mf->Identifier : 0U;
}




/* @func ensMarkerfeatureGetMapweight *****************************************
**
** Get the map wight member of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::map_weight
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [ajint] Map weight or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensMarkerfeatureGetMapweight(const EnsPMarkerfeature mf)
{
    return (mf) ? mf->Mapweight : 0;
}




/* @func ensMarkerfeatureGetMarker ********************************************
**
** Get the Ensembl Marker member of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::marker
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPMarker] Ensembl Marker or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarker ensMarkerfeatureGetMarker(const EnsPMarkerfeature mf)
{
    return (mf) ? mf->Marker : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Set Set one member of an Ensembl Marker Feature
** @nam4rule Adaptor Set the Ensembl Marker Feature Adaptor
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Mapweight Set the map weight
** @nam4rule Marker Set the Ensembl Marker
**
** @argrule * mf [EnsPMarkerfeature] Ensembl Marker Feature object
** @argrule Adaptor mfa [EnsPMarkerfeatureadaptor] Ensembl Marker
**                                                 Feature Adaptor
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Mapweight mapweight [ajint] Map weight
** @argrule Marker marker [EnsPMarker] Ensembl Marker
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMarkerfeatureSetAdaptor *******************************************
**
** Set the Ensembl Marker Feature Adaptor member of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetAdaptor(EnsPMarkerfeature mf,
                                  EnsPMarkerfeatureadaptor mfa)
{
    if (!mf)
        return ajFalse;

    mf->Adaptor = mfa;

    return ajTrue;
}




/* @func ensMarkerfeatureSetFeature *******************************************
**
** Set the Ensembl Feature member of an Ensembl Marker Feature.
**
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetFeature(EnsPMarkerfeature mf, EnsPFeature feature)
{
    if (!mf)
        return ajFalse;

    ensFeatureDel(&mf->Feature);

    mf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensMarkerfeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetIdentifier(EnsPMarkerfeature mf, ajuint identifier)
{
    if (!mf)
        return ajFalse;

    mf->Identifier = identifier;

    return ajTrue;
}




/* @func ensMarkerfeatureSetMapweight *****************************************
**
** Set the map weight member of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::map_weight
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [r] mapweight [ajint] Map weight
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetMapweight(EnsPMarkerfeature mf, ajint mapweight)
{
    if (!mf)
        return ajFalse;

    mf->Mapweight = mapweight;

    return ajTrue;
}




/* @func ensMarkerfeatureSetMarker ********************************************
**
** Set the Ensembl Marker member of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::marker
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetMarker(EnsPMarkerfeature mf, EnsPMarker marker)
{
    if (!mf)
        return ajFalse;

    ensMarkerDel(&mf->Marker);

    mf->Marker = ensMarkerNewRef(marker);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Trace Report Ensembl Marker Feature members to debug file.
**
** @argrule Trace mf [const EnsPMarkerfeature] Ensembl Marker Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkerfeatureTrace ************************************************
**
** Trace an Ensembl Marker Feature.
**
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureTrace(const EnsPMarkerfeature mf, ajuint level)
{
    AjPStr indent = NULL;

    if (!mf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMarkerfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Marker %p\n"
            "%S  Mapweight %d\n",
            indent, mf,
            indent, mf->Use,
            indent, mf->Identifier,
            indent, mf->Adaptor,
            indent, mf->Feature,
            indent, mf->Marker,
            indent, mf->Mapweight);

    ensFeatureTrace(mf->Feature, level + 1);

    ensMarkerTrace(mf->Marker, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Calculate Calculate Ensembl Marker Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkerfeatureCalculateMemsize *************************************
**
** Calculate the memory size in bytes of an Ensembl Marker Feature.
**
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMarkerfeatureCalculateMemsize(const EnsPMarkerfeature mf)
{
    size_t size = 0;

    if (!mf)
        return 0;

    size += sizeof (EnsOMarkerfeature);

    size += ensFeatureCalculateMemsize(mf->Feature);

    size += ensMarkerCalculateMemsize(mf->Marker);

    return size;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listMarkerfeatureCompareEndAscending ***************************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** Ensembl Feature end member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Marker Feature address 1
** @param [r] item2 [const void*] Ensembl Marker Feature address 2
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

static int listMarkerfeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPMarkerfeature mf1 = *(EnsOMarkerfeature *const *) item1;
    EnsPMarkerfeature mf2 = *(EnsOMarkerfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMarkerfeatureCompareEndAscending"))
        ajDebug("listMarkerfeatureCompareEndAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareEndAscending(mf1->Feature, mf2->Feature);
}




/* @funcstatic listMarkerfeatureCompareEndDescending ************************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** Ensembl Feature end member in descending order.
**
** @param [r] item1 [const void*] Ensembl Marker Feature address 1
** @param [r] item2 [const void*] Ensembl Marker Feature address 2
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

static int listMarkerfeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPMarkerfeature mf1 = *(EnsOMarkerfeature *const *) item1;
    EnsPMarkerfeature mf2 = *(EnsOMarkerfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMarkerfeatureCompareEndDescending"))
        ajDebug("listMarkerfeatureCompareEndDescending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareEndDescending(mf1->Feature, mf2->Feature);
}




/* @funcstatic listMarkerfeatureCompareIdentifierAscending ********************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** identifier member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Marker Feature address 1
** @param [r] item2 [const void*] Ensembl Marker Feature address 2
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

static int listMarkerfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPMarkerfeature mf1 = *(EnsOMarkerfeature *const *) item1;
    EnsPMarkerfeature mf2 = *(EnsOMarkerfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMarkerfeatureCompareIdentifierAscending"))
        ajDebug("listMarkerfeatureCompareIdentifierAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    if (mf1->Identifier < mf2->Identifier)
        return -1;

    if (mf1->Identifier > mf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listMarkerfeatureCompareStartAscending *************************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** Ensembl Feature start member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Marker Feature address 1
** @param [r] item2 [const void*] Ensembl Marker Feature address 2
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

static int listMarkerfeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPMarkerfeature mf1 = *(EnsOMarkerfeature *const *) item1;
    EnsPMarkerfeature mf2 = *(EnsOMarkerfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMarkerfeatureCompareStartAscending"))
        ajDebug("listMarkerfeatureCompareStartAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartAscending(mf1->Feature, mf2->Feature);
}




/* @funcstatic listMarkerfeatureCompareStartDescending ************************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** Ensembl Feature start member in descending order.
**
** @param [r] item1 [const void*] Ensembl Marker Feature address 1
** @param [r] item2 [const void*] Ensembl Marker Feature address 2
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

static int listMarkerfeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPMarkerfeature mf1 = *(EnsOMarkerfeature *const *) item1;
    EnsPMarkerfeature mf2 = *(EnsOMarkerfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMarkerfeatureCompareStartDescending"))
        ajDebug("listMarkerfeatureCompareStartDescending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartDescending(mf1->Feature, mf2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Markerfeature Functions for manipulating AJAX List objects of
** Ensembl Marker Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending mfs [AjPList] AJAX List of
**                                  Ensembl Marker Feature objects
** @argrule Descending mfs [AjPList] AJAX List of
**                                   Ensembl Marker Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListMarkerfeatureSortEndAscending *********************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** Ensembl Feature end member in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortEndAscending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMarkerfeatureCompareEndAscending,
                       &listMarkerfeatureCompareStartAscending,
                       &listMarkerfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMarkerfeatureSortEndDescending ********************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** Ensembl Feature end member in descending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortEndDescending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMarkerfeatureCompareEndDescending,
                       &listMarkerfeatureCompareStartDescending,
                       &listMarkerfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMarkerfeatureSortIdentifierAscending **************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** identifier member in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortIdentifierAscending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSort(mfs, &listMarkerfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMarkerfeatureSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** Ensembl Feature start member in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortStartAscending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMarkerfeatureCompareStartAscending,
                       &listMarkerfeatureCompareEndAscending,
                       &listMarkerfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMarkerfeatureSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** Ensembl Feature start member in descending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortStartDescending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMarkerfeatureCompareStartDescending,
                       &listMarkerfeatureCompareEndDescending,
                       &listMarkerfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor *****
**
** @nam2rule Markerfeatureadaptor Functions for manipulating
** Ensembl Marker Feature Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor
** @cc CVS Revision: 1.17
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic markerfeatureadaptorFetchAllbyStatement ************************
**
** Fetch all Ensembl Marker Feature objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool markerfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs)
{
    ajint mapweight = 0;
    ajint priority  = 0;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 1;

    ajuint identifier  = 0U;
    ajuint analysisid  = 0U;
    ajuint markerid    = 0U;
    ajuint mindistance = 0U;
    ajuint maxdistance = 0U;
    ajuint msid        = 0U;

    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr lprimer  = NULL;
    AjPStr rprimer  = NULL;
    AjPStr type     = NULL;
    AjPStr msname   = NULL;
    AjPStr mssource = NULL;

    AjPTable markers = NULL;

    EnsEMarkerType etype = ensEMarkerTypeNULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPMarker marker    = NULL;
    EnsPMarkeradaptor ma = NULL;

    EnsPMarkerfeature mf         = NULL;
    EnsPMarkerfeatureadaptor mfa = NULL;

    EnsPMarkersynonym ms = NULL;

    if (ajDebugTest("markerfeatureadaptorFetchAllbyStatement"))
        ajDebug("markerfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  mfs %p\n",
                ba,
                statement,
                am,
                slice,
                mfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!mfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ma  = ensRegistryGetMarkeradaptor(dba);
    mfa = ensRegistryGetMarkerfeatureadaptor(dba);

    markers = ajTableuintNew(0);

    ajTableSetDestroyvalue(markers, (void (*)(void **)) &ensMarkerDel);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        srid        = 0;
        srstart     = 0;
        srend       = 0;
        analysisid  = 0;
        mapweight   = 0;
        markerid    = 0;
        lprimer     = ajStrNew();
        rprimer     = ajStrNew();
        mindistance = 0;
        maxdistance = 0;
        priority    = 0;
        type        = ajStrNew();
        msid        = 0;
        msname      = ajStrNew();
        mssource    = ajStrNew();
        etype       = ensEMarkerTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToInt(sqlr, &mapweight);
        ajSqlcolumnToUint(sqlr, &markerid);
        ajSqlcolumnToStr(sqlr, &lprimer);
        ajSqlcolumnToStr(sqlr, &rprimer);
        ajSqlcolumnToUint(sqlr, &mindistance);
        ajSqlcolumnToUint(sqlr, &maxdistance);
        ajSqlcolumnToInt(sqlr, &priority);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &msid);
        ajSqlcolumnToStr(sqlr, &msname);
        ajSqlcolumnToStr(sqlr, &mssource);

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
            ajStrDel(&lprimer);
            ajStrDel(&rprimer);
            ajStrDel(&type);
            ajStrDel(&msname);
            ajStrDel(&mssource);

            continue;
        }

        marker = (EnsPMarker) ajTableFetchmodV(markers,
                                               (const void *) &markerid);

        if (!marker)
        {
            if (msid)
                ms = ensMarkersynonymNewIni(msid, mssource, msname);

            AJNEW0(Pidentifier);

            *Pidentifier = markerid;

            etype = ensMarkerTypeFromStr(type);

            marker = ensMarkerNewIni(ma,
                                     markerid,
                                     ms,
                                     lprimer,
                                     rprimer,
                                     (AjPList) NULL,
                                     (AjPList) NULL,
                                     mindistance,
                                     maxdistance,
                                     etype,
                                     priority);

            ajTablePut(markers, (void *) Pidentifier, (void *) marker);

            ensMarkersynonymDel(&ms);
        }

        mf = ensMarkerfeatureNewIni(mfa,
                                    identifier,
                                    feature,
                                    marker,
                                    mapweight);

        ajListPushAppend(mfs, (void *) mf);

        ensFeatureDel(&feature);

        ajStrDel(&lprimer);
        ajStrDel(&rprimer);
        ajStrDel(&type);
        ajStrDel(&msname);
        ajStrDel(&mssource);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajTableDel(&markers);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Marker Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Marker Feature Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMarkerfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMarkerfeatureadaptorNew *******************************************
**
** Default constructor for an Ensembl Marker Feature Adaptor.
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
** @see ensRegistryGetMarkerfeatureadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMarkerfeatureadaptor ensMarkerfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        markerfeatureadaptorKTables,
        markerfeatureadaptorKColumns,
        markerfeatureadaptorKLeftjoin,
        markerfeatureadaptorKDefaultcondition,
        (const char *) NULL,
        &markerfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensMarkerfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensMarkerfeatureDel,
        (size_t (*)(const void *)) &ensMarkerfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensMarkerfeatureGetFeature,
        "Marker Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Marker Feature Adaptor object.
**
** @fdata [EnsPMarkerfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Feature Adaptor
**
** @argrule * Pmfa [EnsPMarkerfeatureadaptor*]
** Ensembl Marker Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkerfeatureadaptorDel *******************************************
**
** Default destructor for an Ensembl Marker Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pmfa [EnsPMarkerfeatureadaptor*]
** Ensembl Marker Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMarkerfeatureadaptorDel(EnsPMarkerfeatureadaptor *Pmfa)
{
    ensFeatureadaptorDel(Pmfa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Marker Feature Adaptor
** object.
**
** @fdata [EnsPMarkerfeatureadaptor]
**
** @nam3rule Get Return Marker Feature Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * mfa [EnsPMarkerfeatureadaptor] Marker Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerfeatureadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Marker Feature Adaptor.
**
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMarkerfeatureadaptorGetDatabaseadaptor(
    EnsPMarkerfeatureadaptor mfa)
{
    return ensFeatureadaptorGetDatabaseadaptor(mfa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Marker Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPMarkerfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Marker Feature object(s)
** @nam4rule All Fetch all Ensembl Marker Feature objects
** @nam4rule Allby Fetch all Ensembl Marker Feature objects
**                 matching a criterion
** @nam5rule Marker Fetch all by an Ensembl Marker
** @nam5rule Slice  Fetch all by an Ensembl Slice
** @nam5rule Slicemarkername Fetch all by an Ensembl Slice and
**                           Ensembl Marker name
** @nam4rule By Fetch one Ensembl Marker Feature object
**                    matching a criterion
**
** @argrule * mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @argrule All mfs [AjPList] AJAX List of Ensembl Marker Feature objects
** @argrule AllbyMarker marker [const EnsPMarker] Ensembl Marker
** @argrule AllbyMarker mfs [AjPList] AJAX List of Ensembl Marker Feature
**                                    objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice priority [ajint] Priority
** @argrule AllbySlice mapweight [ajuint] Map weight
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice mfs [AjPList] AJAX List of Ensembl Marker Feature
**                                   objects
** @argrule AllbySlicemarkername slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicemarkername name [const AjPStr] Ensembl Marker name
** @argrule AllbySlicemarkername mfs [AjPList] AJAX List of
**                                             Ensembl Marker Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerfeatureadaptorFetchAllbyMarker ******************************
**
** Fetch all Ensembl Marker Feature objects by an Ensembl Marker.
**
** The caller is responsible for deleting the Ensembl Markers Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor::fetch_all_by_Marker
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @param [r] marker [const EnsPMarker] Ensembl Marker
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureadaptorFetchAllbyMarker(
    EnsPMarkerfeatureadaptor mfa,
    const EnsPMarker marker,
    AjPList mfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!mfa)
        return ajFalse;

    if (!marker)
        return ajFalse;

    if (!mfs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(mfa);

    constraint = ajFmtStr("marker.marker_id = %u", marker->Identifier);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                mfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensMarkerfeatureadaptorFetchAllbySlice *******************************
**
** Fetch all Ensembl Marker Feature objects by an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Markers Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor::
**     fetch_all_by_Slice_and_priority
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] priority [ajint] Priority
** @param [r] mapweight [ajuint] Map weight
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureadaptorFetchAllbySlice(
    EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    ajint priority,
    ajuint mapweight,
    const AjPStr anname,
    AjPList mfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!mfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!mfs)
        return ajFalse;

    if (priority)
        constraint = ajFmtStr("marker.priority > %d", priority);

    if (mapweight)
    {
        if (constraint)
            ajFmtPrintAppS(&constraint,
                           " AND marker_feature.map_weight < %u",
                           mapweight);
        else
            constraint = ajFmtStr("marker_feature.map_weight < %u", mapweight);
    }

    result = ensFeatureadaptorFetchAllbySlice(mfa,
                                              slice,
                                              constraint,
                                              anname,
                                              mfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensMarkerfeatureadaptorFetchAllbySlicemarkername *********************
**
** Fetch all Ensembl Marker Feature objects by an Ensembl Slice and
** Ensembl Marker name.
**
** The caller is responsible for deleting the Ensembl Markers Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor::
**     fetch_all_by_Slice_and_MarkerName
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] name [const AjPStr] Ensembl Marker name
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMarkerfeatureadaptorFetchAllbySlicemarkername(
    EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPStr name,
    AjPList mfs)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!mfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!mfs)
        return ajFalse;

    ensFeatureadaptorEscapeC(mfa, &txtname, name);

    constraint = ajFmtStr("marker_synonym.name = '%s'", txtname);

    ajCharDel(&txtname);

    result = ensFeatureadaptorFetchAllbySlice(mfa,
                                              slice,
                                              constraint,
                                              (AjPStr) NULL,
                                              mfs);

    ajStrDel(&constraint);

    return result;
}

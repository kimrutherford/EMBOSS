/* @source Ensembl Marker functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.39 $
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

#include "ensmarker.h"
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

/* @conststatic markerType ****************************************************
**
** The Ensembl Marker type element is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsEMarkerType.
**
******************************************************************************/

static const char* const markerType[] =
{
    "",
    "est",
    "microsatellite",
    (const char*) NULL
};




/* @conststatic markeradaptorTables *******************************************
**
** Array of Ensembl Marker Adaptor SQL table names
**
******************************************************************************/

static const char* const markeradaptorTables[] =
{
    "marker",
    "marker_synonym",
    (const char*) NULL
};




/* @conststatic markeradaptorColumns ******************************************
**
** Array of Ensembl Marker Adaptor SQL column names
**
******************************************************************************/

static const char* const markeradaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic markeradaptorLeftjoin *****************************************
**
** Array of Ensembl Marker Adaptor SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin markeradaptorLeftjoin[] =
{
    {
        "marker_synonym",
        "marker.marker_id = marker_synonym.marker_id"
    },
    {(const char*) NULL, (const char*) NULL}
};




/* @conststatic markerfeatureadaptorTables ************************************
**
** Array of Ensembl Marker Feature Adaptor SQL table names
**
******************************************************************************/

static const char* const markerfeatureadaptorTables[] =
{
    "marker_feature",
    "marker",
    "marker_synonym",
    (const char*) NULL
};




/* @conststatic markerfeatureadaptorColumns ***********************************
**
** Array of Ensembl Marker Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const markerfeatureadaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic markerfeatureadaptorLeftjoin **********************************
**
** Array of Ensembl Marker Feature Adaptor SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin markerfeatureadaptorLeftjoin[] =
{
    {
        "marker_synonym",
        "marker.display_marker_synonym_id = marker_synonym.marker_synonym_id"
    },
    {(const char*) NULL, (const char*) NULL}
};




/* @conststatic markerfeatureadaptorDefaultcondition **************************
**
** Ensembl Marker Feature Adaptor SQL default condition
**
******************************************************************************/

static const char* markerfeatureadaptorDefaultcondition =
    "marker_feature.marker_id = marker.marker_id";




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool markersynonymadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mss);

static AjBool markermaplocationadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mmls);

static void tableMarkerClear(void** key,
                             void** value,
                             void* cl);

static AjBool markeradaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList markers);

static void markeradaptorClearIdentifierTable(void** key,
                                              void** value,
                                              void* cl);

static int listMarkerfeatureCompareStartAscending(const void* P1,
                                                  const void* P2);

static int listMarkerfeatureCompareStartDescending(const void* P1,
                                                   const void* P2);

static AjBool markerfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs);

static void* markerfeatureadaptorCacheReference(void* value);

static void markerfeatureadaptorCacheDelete(void** value);

static size_t markerfeatureadaptorCacheSize(const void* value);

static EnsPFeature markerfeatureadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.6
** @cc CVS Tag: branch-ensembl-62
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
** @valrule * [EnsPMarkersynonym] Ensembl Marker Synonym
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
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkersynonymNewCpy(const EnsPMarkersynonym ms)
{
    EnsPMarkersynonym pthis = NULL;

    if(!ms)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = ms->Identifier;

    if(ms->Source)
        pthis->Source = ajStrNewRef(ms->Source);

    if(ms->Name)
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
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkersynonymNewIni(ajuint identifier,
                                         AjPStr source,
                                         AjPStr name)
{
    EnsPMarkersynonym ms = NULL;

    AJNEW0(ms);

    ms->Use = 1;

    ms->Identifier = identifier;

    if(source)
        ms->Source = ajStrNewRef(source);

    if(name)
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
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkersynonymNewRef(EnsPMarkersynonym ms)
{
    if(!ms)
        return NULL;

    ms->Use++;

    return ms;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Synonym object
**
** @argrule * Pms [EnsPMarkersynonym*] Ensembl Marker Synonym object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkersynonymDel **************************************************
**
** Default destructor for an Ensembl Marker Synonym.
**
** @param [d] Pms [EnsPMarkersynonym*] Ensembl Marker Synonym object address
**
** @return [void]
** @@
******************************************************************************/

void ensMarkersynonymDel(EnsPMarkersynonym* Pms)
{
    EnsPMarkersynonym pthis = NULL;

    if(!Pms)
        return;

    if(!*Pms)
        return;

    if(ajDebugTest("ensMarkersynonymDel"))
    {
        ajDebug("ensMarkersynonymDel\n"
                "  *Pms %p\n",
                *Pms);

        ensMarkersynonymTrace(*Pms, 1);
    }

    pthis = *Pms;

    pthis->Use--;

    if(pthis->Use)
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




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Marker Synonym object.
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
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Source [AjPStr] Source or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkersynonymGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::dbID
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensMarkersynonymGetIdentifier(const EnsPMarkersynonym ms)
{
    if(!ms)
        return 0;

    return ms->Identifier;
}




/* @func ensMarkersynonymGetName **********************************************
**
** Get the name element of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::name
** @param  [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensMarkersynonymGetName(const EnsPMarkersynonym ms)
{
    if(!ms)
        return NULL;

    return ms->Name;
}




/* @func ensMarkersynonymGetSource ********************************************
**
** Get the source element of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::source
** @param [r] ms [const EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjPStr] Source or NULL
** @@
******************************************************************************/

AjPStr ensMarkersynonymGetSource(const EnsPMarkersynonym ms)
{
    if(!ms)
        return NULL;

    return ms->Source;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Marker Synonym object.
**
** @fdata [EnsPMarkersynonym]
**
** @nam3rule Set Set one element of an Ensembl Marker Synonym
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
** Set the SQL database-internal identifier element of an
** Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::dbID
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkersynonymSetIdentifier(EnsPMarkersynonym ms, ajuint identifier)
{
    if(!ms)
        return ajFalse;

    ms->Identifier = identifier;

    return ajTrue;
}




/* @func ensMarkersynonymSetName **********************************************
**
** Set the name element of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::name
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkersynonymSetName(EnsPMarkersynonym ms, AjPStr name)
{
    if(!ms)
        return ajFalse;

    ajStrDel(&ms->Name);

    ms->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensMarkersynonymSetSource ********************************************
**
** Set the source element of an Ensembl Marker Synonym.
**
** @cc Bio::EnsEMBL::Map::MarkerSynonym::source
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
** @param [u] source [AjPStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkersynonymSetSource(EnsPMarkersynonym ms, AjPStr source)
{
    if(!ms)
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
** @nam3rule Trace Report Ensembl Marker Synonym elements to debug file.
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
** @@
******************************************************************************/

AjBool ensMarkersynonymTrace(const EnsPMarkersynonym ms, ajuint level)
{
    AjPStr indent = NULL;

    if(!ms)
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
** @@
******************************************************************************/

size_t ensMarkersynonymCalculateMemsize(const EnsPMarkersynonym ms)
{
    size_t size = 0;

    if(!ms)
        return 0;

    size += sizeof (EnsOMarkersynonym);

    if(ms->Source)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Source);
    }

    if(ms->Name)
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
** @@
******************************************************************************/

static AjBool markersynonymadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mss)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name   = NULL;
    AjPStr source = NULL;

    EnsPMarkersynonym ms = NULL;

    if(!dba)
        return ajFalse;

    if(!(statement && ajStrGetLen(statement)))
        return ajFalse;

    if(!mss)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name = ajStrNew();
        source = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &source);

        ms = ensMarkersynonymNewIni(identifier, source, name);

        ajListPushAppend(mss, (void*) ms);

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
** @@
******************************************************************************/

AjBool ensMarkersynonymadaptorFetchAllbyMarkeridentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mss)
{
    AjBool result = AJFALSE;

    AjPStr statement = NULL;

    if(!dba)
        return ajFalse;

    if(!mss)
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
** @@
******************************************************************************/

AjBool ensMarkersynonymadaptorFetchByIdentifier(
    EnsPDatabaseadaptor dba,
    ajuint identifier,
    EnsPMarkersynonym* Pms)
{
    AjPList mss = NULL;

    AjPStr statement = NULL;

    EnsPMarkersynonym ms = NULL;

    if(!dba)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pms)
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

    if(ajListGetLength(mss) > 1)
        ajWarn("ensMarkersynonymadaptorFetchByIdentifier got more than one "
               "Ensembl Marker Synonym for identifier %u.\n",
               identifier);

    ajListPop(mss, (void**) Pms);

    while(ajListPop(mss, (void**) &ms))
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
** @cc CVS Revision: 1.7
** @cc CVS Tag: branch-ensembl-62
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
** @valrule * [EnsPMarkermaplocation] Ensembl Marker Map Location
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
** @@
******************************************************************************/

EnsPMarkermaplocation ensMarkermaplocationNewCpy(
    const EnsPMarkermaplocation mml)
{
    EnsPMarkermaplocation pthis = NULL;

    if(!mml)
        return NULL;

    AJNEW0(pthis);

    pthis->Markersynonym = ensMarkersynonymNewRef(mml->Markersynonym);

    if(mml->Mapname)
        pthis->Mapname = ajStrNewRef(mml->Mapname);

    if(mml->Chromosomename)
        pthis->Chromosomename = ajStrNewRef(mml->Chromosomename);

    if(mml->Position)
        pthis->Position = ajStrNewRef(mml->Position);

    pthis->Lodscore = mml->Lodscore;

    pthis->Use = 1;

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

    if(mapname)
        mml->Mapname = ajStrNewRef(mapname);

    if(chrname)
        mml->Chromosomename = ajStrNewRef(chrname);

    if(position)
        mml->Position = ajStrNewRef(position);

    mml->Lodscore = lodscore;

    mml->Use = 1;

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
** @@
******************************************************************************/

EnsPMarkermaplocation ensMarkermaplocationNewRef(EnsPMarkermaplocation mml)
{
    if(!mml)
        return NULL;

    mml->Use++;

    return mml;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Map Location object
**
** @argrule * Pmml [EnsPMarkermaplocation*] Ensembl Marker Map Location
**                                          object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkermaplocationDel **********************************************
**
** Default destructor for an Ensembl Marker Map Location.
**
** @param [d] Pmml [EnsPMarkermaplocation*] Ensembl Marker Map Location
**                                          object address
**
** @return [void]
** @@
******************************************************************************/

void ensMarkermaplocationDel(EnsPMarkermaplocation* Pmml)
{
    EnsPMarkermaplocation pthis = NULL;

    if(!Pmml)
        return;

    if(!*Pmml)
        return;

    if(ajDebugTest("ensMarkermaplocationDel"))
    {
        ajDebug("ensMarkermaplocationDel\n"
                "  *Pmml %p\n",
                *Pmml);

        ensMarkermaplocationTrace(*Pmml, 1);
    }

    pthis = *Pmml;

    pthis->Use--;

    if(pthis->Use)
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




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Marker Map Location object.
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
** Get the chromosome name element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::chromosome_name
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjPStr] Chromosome name or NULL
** @@
******************************************************************************/

AjPStr ensMarkermaplocationGetChromosomename(const EnsPMarkermaplocation mml)
{
    if(!mml)
        return NULL;

    return mml->Chromosomename;
}




/* @func ensMarkermaplocationGetLodscore **************************************
**
** Get the LOD score element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::lod_score
** @param [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [float] LOD score or 0.0F
** @@
******************************************************************************/

float ensMarkermaplocationGetLodscore(const EnsPMarkermaplocation mml)
{
    if(!mml)
        return 0.0F;

    return mml->Lodscore;
}




/* @func ensMarkermaplocationGetMapname ***************************************
**
** Get the map name element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::map_name
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjPStr] Map name or NULL
** @@
******************************************************************************/

AjPStr ensMarkermaplocationGetMapname(const EnsPMarkermaplocation mml)
{
    if(!mml)
        return NULL;

    return mml->Mapname;
}




/* @func ensMarkermaplocationGetMarkersynonym *********************************
**
** Get the Ensembl Marker Synonym element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::name
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [EnsPMarkersynonym] Ensembl Marker Synonym or NULL
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkermaplocationGetMarkersynonym(
    const EnsPMarkermaplocation mml)
{
    if(!mml)
        return NULL;

    return mml->Markersynonym;
}




/* @func ensMarkermaplocationGetPosition **************************************
**
** Get the position element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::position
** @param  [r] mml [const EnsPMarkermaplocation] Ensembl Marker Map Location
**
** @return [AjPStr] Position or NULL
** @@
******************************************************************************/

AjPStr ensMarkermaplocationGetPosition(const EnsPMarkermaplocation mml)
{
    if(!mml)
        return NULL;

    return mml->Position;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Marker Map Location object.
**
** @fdata [EnsPMarkermaplocation]
**
** @nam3rule Set Set one element of an Ensembl Marker Map Location
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
** Set the chromosome name element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::chromosome_name
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] chrname [AjPStr] Chromosome name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetChromosomename(EnsPMarkermaplocation mml,
                                             AjPStr chrname)
{
    if(!mml)
        return ajFalse;

    ajStrDel(&mml->Chromosomename);

    mml->Chromosomename = ajStrNewRef(chrname);

    return ajTrue;
}




/* @func ensMarkermaplocationSetLodscore **************************************
**
** Set the LOD score element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::lod_score
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [r] lodscore [float] LOD score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetLodscore(EnsPMarkermaplocation mml,
                                       float lodscore)
{
    if(!mml)
        return ajFalse;

    mml->Lodscore = lodscore;

    return ajTrue;
}




/* @func ensMarkermaplocationSetMapname ***************************************
**
** Set the map name element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::map_name
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] mapname [AjPStr] Map name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetMapname(EnsPMarkermaplocation mml,
                                      AjPStr mapname)
{
    if(!mml)
        return ajFalse;

    ajStrDel(&mml->Mapname);

    mml->Mapname = ajStrNewRef(mapname);

    return ajTrue;
}




/* @func ensMarkermaplocationSetMarkersynonym *********************************
**
** Set the Ensembl Marker Synonym element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::name
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] ms [EnsPMarkersynonym] Ensembl Marker Synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetMarkersynonym(EnsPMarkermaplocation mml,
                                            EnsPMarkersynonym ms)
{
    if(!mml)
        return ajFalse;

    ensMarkersynonymDel(&mml->Markersynonym);

    mml->Markersynonym = ensMarkersynonymNewRef(ms);

    return ajTrue;
}




/* @func ensMarkermaplocationSetPosition **************************************
**
** Set the position element of an Ensembl Marker Map Location.
**
** @cc Bio::EnsEMBL::Map::MapLocation::position
** @param [u] mml [EnsPMarkermaplocation] Ensembl Marker Map Location
** @param [u] position [AjPStr] Position
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkermaplocationSetPosition(EnsPMarkermaplocation mml,
                                       AjPStr position)
{
    if(!mml)
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
** @nam3rule Trace Report Ensembl Marker Map Location elements to debug file.
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
** @@
******************************************************************************/

AjBool ensMarkermaplocationTrace(const EnsPMarkermaplocation mml,
                                 ajuint level)
{
    AjPStr indent = NULL;

    if(!mml)
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
** @@
******************************************************************************/

size_t ensMarkermaplocationCalculateMemsize(const EnsPMarkermaplocation mml)
{
    size_t size = 0;

    if(!mml)
        return 0;

    size += sizeof (EnsOMarkermaplocation);

    size += ensMarkersynonymCalculateMemsize(mml->Markersynonym);

    if(mml->Mapname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mml->Mapname);
    }

    if(mml->Chromosomename)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mml->Chromosomename);
    }

    if(mml->Position)
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
** @@
******************************************************************************/

static AjBool markermaplocationadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    AjPList mmls)
{
    float lodscore = 0.0F;

    ajuint msid = 0;

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

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!mmls)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        ajListPushAppend(mmls, (void*) mml);

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
** @@
******************************************************************************/

AjBool ensMarkermaplocationadaptorFetchAllbyMarkeridentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mmls)
{
    AjBool result = AJFALSE;

    AjPStr statement = NULL;

    if(!dba)
        return ajFalse;

    if(!markerid)
        return ajFalse;

    if(!mmls)
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
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-62
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
** @valrule * [EnsPMarker] Ensembl Marker
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
** @@
******************************************************************************/

EnsPMarker ensMarkerNewCpy(const EnsPMarker marker)
{
    AjIList iter = NULL;

    EnsPMarker pthis = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    if(!marker)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = marker->Identifier;

    pthis->Adaptor = marker->Adaptor;

    pthis->Displaysynonym = ensMarkersynonymNewRef(marker->Displaysynonym);

    if(marker->PrimerLeft)
        pthis->PrimerLeft = ajStrNewRef(marker->PrimerLeft);

    if(marker->PrimerRight)
        pthis->PrimerRight = ajStrNewRef(marker->PrimerRight);

    /* Copy the AJAX List of Ensembl Marker Synonym objects. */

    if(marker->Markersynonyms && ajListGetLength(marker->Markersynonyms))
    {
        pthis->Markersynonyms = ajListNew();

        iter = ajListIterNew(marker->Markersynonyms);

        while(!ajListIterDone(iter))
        {
            ms = (EnsPMarkersynonym) ajListIterGet(iter);

            if(ms)
                ajListPushAppend(pthis->Markersynonyms,
                                 (void*) ensMarkersynonymNewRef(ms));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Markersynonyms = NULL;

    /* Copy the AJAX List of Ensembl Marker Map Location objects. */

    if(marker->Markermaplocations &&
       ajListGetLength(marker->Markermaplocations))
    {
        pthis->Markermaplocations = ajListNew();

        iter = ajListIterNew(marker->Markermaplocations);

        while(!ajListIterDone(iter))
        {
            mml = (EnsPMarkermaplocation) ajListIterGet(iter);

            if(mml)
                ajListPushAppend(pthis->Markermaplocations,
                                 (void*) ensMarkermaplocationNewRef(mml));
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

    marker->Use = 1;

    marker->Identifier = identifier;

    marker->Adaptor = ma;

    marker->Displaysynonym = ensMarkersynonymNewRef(display);

    if(lprimer)
        marker->PrimerLeft = ajStrNewRef(lprimer);

    if(rprimer)
        marker->PrimerRight = ajStrNewRef(rprimer);

    /* Copy the AJAX List of Ensembl Marker Synonym objects. */

    if(mss && ajListGetLength(mss))
    {
        marker->Markersynonyms = ajListNew();

        iter = ajListIterNew(mss);

        while(!ajListIterDone(iter))
        {
            ms = (EnsPMarkersynonym) ajListIterGet(iter);

            if(ms)
                ajListPushAppend(marker->Markersynonyms,
                                 (void*) ensMarkersynonymNewRef(ms));
        }

        ajListIterDel(&iter);
    }
    else
        marker->Markersynonyms = NULL;

    /* Copy the AJAX List of Ensembl Marker Map Location objects. */

    if(mmls && ajListGetLength(mmls))
    {
        marker->Markermaplocations = ajListNew();

        iter = ajListIterNew(mmls);

        while(!ajListIterDone(iter))
        {
            mml = (EnsPMarkermaplocation) ajListIterGet(iter);

            if(mml)
                ajListPushAppend(marker->Markermaplocations,
                                 (void*) ensMarkermaplocationNewRef(mml));
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
** @@
******************************************************************************/

EnsPMarker ensMarkerNewRef(EnsPMarker marker)
{
    if(!marker)
        return NULL;

    marker->Use++;

    return marker;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Del Destroy (free) an Ensembl Marker object
**
** @argrule * Pmarker [EnsPMarker*] Ensembl Marker object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkerDel *********************************************************
**
** Default destructor for an Ensembl Marker.
**
** @param [d] Pmarker [EnsPMarker*] Ensembl Marker object address
**
** @return [void]
** @@
******************************************************************************/

void ensMarkerDel(EnsPMarker* Pmarker)
{
    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPMarker pthis = NULL;

    if(!Pmarker)
        return;

    if(!*Pmarker)
        return;

    if(ajDebugTest("ensMarkerDel"))
    {
        ajDebug("ensMarkerDel\n"
                "  *Pmarker %p\n",
                *Pmarker);

        ensMarkerTrace(*Pmarker, 1);
    }

    pthis = *Pmarker;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmarker = NULL;

        return;
    }

    ajStrDel(&pthis->PrimerLeft);
    ajStrDel(&pthis->PrimerRight);

    ensMarkersynonymDel(&pthis->Displaysynonym);

    while(ajListPop(pthis->Markersynonyms, (void**) &ms))
        ensMarkersynonymDel(&ms);

    ajListFree(&pthis->Markersynonyms);

    while(ajListPop(pthis->Markermaplocations, (void**) &mml))
        ensMarkermaplocationDel(&mml);

    ajListFree(&pthis->Markermaplocations);

    AJFREE(pthis);

    *Pmarker = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Marker object.
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
** @valrule DistanceMaximum [ajuint] Maximum distance or 0
** @valrule DistanceMinimum [ajuint] Minimum distance or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule PrimerLeft [AjPStr] Left primer or NULL
** @valrule PrimerRight [AjPStr] Right primer or NULL
** @valrule Priority [ajint] Priority or 0
** @valrule Type [EnsEMarkerType] Type or ensEMarkerTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerGetAdaptor **************************************************
**
** Get the Ensembl Marker Adaptor element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsPMarkeradaptor] Ensembl Marker Adaptor or NULL
** @@
******************************************************************************/

EnsPMarkeradaptor ensMarkerGetAdaptor(const EnsPMarker marker)
{
    if(!marker)
        return NULL;

    return marker->Adaptor;
}




/* @func ensMarkerGetDisplaysynonym *******************************************
**
** Get the display synonym element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::display_MarkerSynonym
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsPMarkersynonym] Display Ensembl Marker Synonym or NULL
** @@
******************************************************************************/

EnsPMarkersynonym ensMarkerGetDisplaysynonym(const EnsPMarker marker)
{
    if(!marker)
        return NULL;

    return marker->Displaysynonym;
}




/* @func ensMarkerGetDistanceMaximum ******************************************
**
** Get the maximum distance element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::max_primer_dist
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajuint] Maximum distance or 0
** @@
******************************************************************************/

ajuint ensMarkerGetDistanceMaximum(const EnsPMarker marker)
{
    if(!marker)
        return 0;

    return marker->DistanceMaximum;
}




/* @func ensMarkerGetDistanceMinimum ******************************************
**
** Get the minimum distance element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::min_primer_dist
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajuint] Minimum distance or 0
** @@
******************************************************************************/

ajuint ensMarkerGetDistanceMinimum(const EnsPMarker marker)
{
    if(!marker)
        return 0;

    return marker->DistanceMinimum;
}




/* @func ensMarkerGetIdentifier ***********************************************
**
** Get the SQL database-internal identifier element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensMarkerGetIdentifier(const EnsPMarker marker)
{
    if(!marker)
        return 0;

    return marker->Identifier;
}




/* @func ensMarkerGetPrimerLeft ***********************************************
**
** Get the left primer element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::left_primer
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [AjPStr] Left primer or NULL
** @@
******************************************************************************/

AjPStr ensMarkerGetPrimerLeft(const EnsPMarker marker)
{
    if(!marker)
        return NULL;

    return marker->PrimerLeft;
}




/* @func ensMarkerGetPrimerRight **********************************************
**
** Get the right primer element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::right_primer
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [AjPStr] Right primer or NULL
** @@
******************************************************************************/

AjPStr ensMarkerGetPrimerRight(const EnsPMarker marker)
{
    if(!marker)
        return NULL;

    return marker->PrimerRight;
}




/* @func ensMarkerGetPriority *************************************************
**
** Get the priority element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::priority
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [ajint] Priority or 0
** @@
******************************************************************************/

ajint ensMarkerGetPriority(const EnsPMarker marker)
{
    if(!marker)
        return 0;

    return marker->Priority;
}




/* @func ensMarkerGetType *****************************************************
**
** Get the type element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::type
** @param [r] marker [const EnsPMarker] Ensembl Marker
**
** @return [EnsEMarkerType] Type or ensEMarkerTypeNULL
** @@
******************************************************************************/

EnsEMarkerType ensMarkerGetType(const EnsPMarker marker)
{
    if(!marker)
        return ensEMarkerTypeNULL;

    return marker->Type;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Marker object,
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
    ajuint identifier = 0;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!marker)
        return NULL;

    if(marker->Markermaplocations)
        return marker->Markermaplocations;

    ba = ensMarkeradaptorGetBaseadaptor(marker->Adaptor);

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    if(!dba)
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
    ajuint identifier = 0;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!marker)
        return NULL;

    if(marker->Markersynonyms)
        return marker->Markersynonyms;

    ba = ensMarkeradaptorGetBaseadaptor(marker->Adaptor);

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    if(!dba)
        return NULL;

    identifier = ensMarkerGetIdentifier(marker);

    marker->Markersynonyms = ajListNew();

    ensMarkersynonymadaptorFetchAllbyMarkeridentifier(dba,
                                                      identifier,
                                                      marker->Markersynonyms);

    return marker->Markersynonyms;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Set Set one element of an Ensembl Marker
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
** Set the Ensembl Marker Adaptor element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetAdaptor(EnsPMarker marker,
                           EnsPMarkeradaptor ma)
{
    if(!marker)
        return ajFalse;

    marker->Adaptor = ma;

    return ajTrue;
}




/* @func ensMarkerSetDisplaysynonym *******************************************
**
** Set the display synonym element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::display_MarkerSynonym
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] display [EnsPMarkersynonym] Display synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetDisplaysynonym(EnsPMarker marker,
                                  EnsPMarkersynonym display)
{
    if(!marker)
        return ajFalse;

    ensMarkersynonymDel(&marker->Displaysynonym);

    marker->Displaysynonym = ensMarkersynonymNewRef(display);

    return ajTrue;
}




/* @func ensMarkerSetDistanceMaximum ******************************************
**
** Set the maximum distance element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::max_primer_dist
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] maxdistance [ajuint] Maximum distance
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetDistanceMaximum(EnsPMarker marker,
                                   ajuint maxdistance)
{
    if(!marker)
        return ajFalse;

    marker->DistanceMaximum = maxdistance;

    return ajTrue;
}




/* @func ensMarkerSetDistanceMinimum ******************************************
**
** Set the minimum distance element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::min_primer_dist
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] mindistance [ajuint] Minimum distance
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetDistanceMinimum(EnsPMarker marker,
                                   ajuint mindistance)
{
    if(!marker)
        return ajFalse;

    marker->DistanceMinimum = mindistance;

    return ajTrue;
}




/* @func ensMarkerSetIdentifier ***********************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Marker.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetIdentifier(EnsPMarker marker,
                              ajuint identifier)
{
    if(!marker)
        return ajFalse;

    marker->Identifier = identifier;

    return ajTrue;
}




/* @func ensMarkerSetPrimerLeft ***********************************************
**
** Set the left primer element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::left_primer
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] lprimer [AjPStr] Left primer
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetPrimerLeft(EnsPMarker marker,
                              AjPStr lprimer)
{
    if(!marker)
        return ajFalse;

    ajStrDel(&marker->PrimerLeft);

    if(lprimer)
        marker->PrimerLeft = ajStrNewRef(lprimer);

    return ajTrue;
}




/* @func ensMarkerSetPrimerRight **********************************************
**
** Set the right primer element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::right_primer
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] rprimer [AjPStr] Right primer
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetPrimerRight(EnsPMarker marker,
                               AjPStr rprimer)
{
    if(!marker)
        return ajFalse;

    ajStrDel(&marker->PrimerRight);

    if(rprimer)
        marker->PrimerRight = ajStrNewRef(rprimer);

    return ajTrue;
}




/* @func ensMarkerSetPriority *************************************************
**
** Set the priority element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::priority
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [r] priority [ajint] Priority
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetPriority(EnsPMarker marker,
                            ajint priority)
{
    if(!marker)
        return ajFalse;

    marker->Priority = priority;

    return ajTrue;
}




/* @func ensMarkerSetType *****************************************************
**
** Set the type element of an Ensembl Marker.
**
** @cc Bio::EnsEMBL::Map::Marker::type
** @param [u] marker [EnsPMarker] Ensembl Marker
** @param [u] type [EnsEMarkerType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerSetType(EnsPMarker marker,
                        EnsEMarkerType type)
{
    if(!marker)
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
** @nam3rule Trace Report Ensembl Marker elements to debug file.
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
** @@
******************************************************************************/

AjBool ensMarkerTrace(const EnsPMarker marker, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    if(!marker)
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

    while(!ajListIterDone(iter))
    {
        ms = (EnsPMarkersynonym) ajListIterGet(iter);

        ensMarkersynonymTrace(ms, level + 1);
    }

    ajListIterDel(&iter);

    /* Trace the AJAX List of Ensembl Marker Map Location objects. */

    iter = ajListIterNew(marker->Markermaplocations);

    while(!ajListIterDone(iter))
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
** @@
******************************************************************************/

size_t ensMarkerCalculateMemsize(const EnsPMarker marker)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPMarkermaplocation mml = NULL;

    EnsPMarkersynonym ms = NULL;

    if(!marker)
        return 0;

    size += sizeof (EnsOMarker);

    size += ensMarkersynonymCalculateMemsize(marker->Displaysynonym);

    if(marker->PrimerLeft)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(marker->PrimerLeft);
    }

    if(marker->PrimerRight)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(marker->PrimerRight);
    }

    /* Summarise the AJAX List of Ensembl Marker Synonym objects. */

    iter = ajListIterNew(marker->Markersynonyms);

    while(!ajListIterDone(iter))
    {
        ms = (EnsPMarkersynonym) ajListIterGet(iter);

        size += ensMarkersynonymCalculateMemsize(ms);
    }

    ajListIterDel(&iter);

    /* Summarise the AJAX List of Ensembl Marker Map Location objects. */

    iter = ajListIterNew(marker->Markermaplocations);

    while(!ajListIterDone(iter))
    {
        mml = (EnsPMarkermaplocation) ajListIterGet(iter);

        size += ensMarkermaplocationCalculateMemsize(mml);
    }

    ajListIterDel(&iter);

    return size;
}




/* @section element addition **************************************************
**
** Functions for adding elements to an Ensembl Marker object.
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
** @@
******************************************************************************/

AjBool ensMarkerAddMarkermaplocation(EnsPMarker marker,
                                     EnsPMarkermaplocation mml)
{
    if(!marker)
        return ajFalse;

    if(!mml)
        return ajFalse;

    if(!marker->Markermaplocations)
        marker->Markermaplocations = ajListNew();

    ajListPushAppend(marker->Markermaplocations,
                     (void*) ensMarkermaplocationNewRef(mml));

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
** @@
******************************************************************************/

AjBool ensMarkerAddMarkersynonym(EnsPMarker marker,
                                 EnsPMarkersynonym ms)
{
    if(!marker)
        return ajFalse;

    if(!ms)
        return ajFalse;

    if(!marker->Markersynonyms)
        marker->Markersynonyms = ajListNew();

    ajListPushAppend(marker->Markersynonyms,
                     (void*) ensMarkersynonymNewRef(ms));

    return ajTrue;
}




/* @section element removal ***************************************************
**
** Functions for clearing elements of an Ensembl Marker object.
**
** @fdata [EnsPMarker]
**
** @nam3rule Clear Clear elements of an Ensembl Marker
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
** @@
******************************************************************************/

AjBool ensMarkerClearMarkermaplocations(EnsPMarker marker)
{
    EnsPMarkermaplocation mml = NULL;

    if(!marker)
        return ajFalse;

    while(ajListPop(marker->Markermaplocations, (void**) &mml))
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
** @@
******************************************************************************/

AjBool ensMarkerClearMarkersynonyms(EnsPMarker marker)
{
    EnsPMarkersynonym ms = NULL;

    if(!marker)
        return ajFalse;

    while(ajListPop(marker->Markersynonyms, (void**) &ms))
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
** @valrule * [EnsEMarkerType] Ensembl Marker Type enumeration or
**                             ensEMarkerTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensMarkerTypeFromStr *************************************************
**
** Convert an AJAX String into an Ensembl Marker Type enumeration.
**
** @param [r] type [const AjPStr] Ensembl Marker Type string
**
** @return [EnsEMarkerType] Ensembl Marker Type enumeration or
**                          ensEMarkerTypeNULL
** @@
******************************************************************************/

EnsEMarkerType ensMarkerTypeFromStr(const AjPStr type)
{
    register EnsEMarkerType i = ensEMarkerTypeNULL;

    EnsEMarkerType etype = ensEMarkerTypeNULL;

    for(i = ensEMarkerTypeNULL;
        markerType[i];
        i++)
        if(ajStrMatchC(type, markerType[i]))
            etype = i;

    if(!etype)
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
** @valrule Char [const char*] Ensembl Marker Type C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensMarkerTypeToChar **************************************************
**
** Convert an Ensembl Marker Type enumeration into a C-type (char*) string.
**
** @param [u] type [EnsEMarkerType] Ensembl Marker Type enumeration
**
** @return [const char*] Ensembl Marker Type C-type (char*) string
** @@
******************************************************************************/

const char* ensMarkerTypeToChar(EnsEMarkerType type)
{
    register EnsEMarkerType i = ensEMarkerTypeNULL;

    for(i = ensEMarkerTypeNULL;
        markerType[i] && (i < type);
        i++);

    if(!markerType[i])
        ajDebug("ensMarkerTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Marker Type enumeration %d.\n",
                type);

    return markerType[i];
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
** @nam3rule Marker AJAX Table of AJAX unsigned integer key data and
**                  Ensembl Marker value data
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




/* @funcstatic tableMarkerClear ***********************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Marker value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Marker address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableMarkerClear(void** key,
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

    ensMarkerDel((EnsPMarker*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableMarkerClear **************************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Marker value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableMarkerClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableMarkerClear, NULL);

    return ajTrue;
}




/* @func ensTableMarkerDelete *************************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Marker value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableMarkerDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableMarkerClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [EnsPMarkeradaptor] Ensembl Marker Adaptor ********************
**
** @nam2rule Markeradaptor Functions for manipulating
** Ensembl Marker Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor
** @cc CVS Revision: 1.14
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic markeradaptorFetchAllbyStatement *******************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Marker objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] markers [AjPList] AJAX List of Ensembl Markers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList markers)
{
    ajint priority = 0;

    ajuint current     = 0;
    ajuint identifier  = 0;
    ajuint displaymsid = 0;
    ajuint mindistance = 0;
    ajuint maxdistance = 0;
    ajuint msid = 0;

    EnsEMarkerType etype = ensEMarkerTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr lprimer  = NULL;
    AjPStr rprimer  = NULL;
    AjPStr type     = NULL;
    AjPStr msname   = NULL;
    AjPStr mssource = NULL;

    EnsPMarker marker    = NULL;
    EnsPMarkeradaptor ma = NULL;

    EnsPMarkersynonym ms = NULL;

    if(ajDebugTest("markeradaptorFetchAllbyStatement"))
        ajDebug("markeradaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  markers %p\n",
                dba,
                statement,
                am,
                slice,
                markers);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!markers)
        return ajFalse;

    ma = ensRegistryGetMarkeradaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        if(current != identifier)
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

            ajListPushAppend(markers, (void*) marker);

            current = identifier;
        }

        if(msid)
        {
            ms = ensMarkersynonymNewIni(msid, mssource, msname);

            if(msid == displaymsid)
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
** @valrule * [EnsPMarkeradaptor] Ensembl Marker Adaptor
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
** @@
******************************************************************************/

EnsPMarkeradaptor ensMarkeradaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        markeradaptorTables,
        markeradaptorColumns,
        markeradaptorLeftjoin,
        (const char*) NULL,
        (const char*) NULL,
        markeradaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Marker Adaptor object.
**
** @fdata [EnsPMarkeradaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Adaptor object.
**
** @argrule * Pma [EnsPMarkeradaptor*] Ensembl Marker Adaptor object address
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
** @param [d] Pma [EnsPMarkeradaptor*] Ensembl Marker Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensMarkeradaptorDel(EnsPMarkeradaptor* Pma)
{
    if(!Pma)
        return;

    if(!*Pma)
        return;

    ensBaseadaptorDel(Pma);

    *Pma = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Marker Adaptor object.
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
** Get the Ensembl Base Adaptor element of an Ensembl Marker Adaptor.
**
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensMarkeradaptorGetBaseadaptor(EnsPMarkeradaptor ma)
{
    if(!ma)
        return NULL;

    return ma;
}




/* @func ensMarkeradaptorGetDatabaseadaptor ***********************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Marker Adaptor.
**
** @param [u] ma [EnsPMarkeradaptor] Ensembl Marker Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMarkeradaptorGetDatabaseadaptor(EnsPMarkeradaptor ma)
{
    if(!ma)
        return NULL;

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
** @@
******************************************************************************/

AjBool ensMarkeradaptorFetchAll(EnsPMarkeradaptor ma,
                                AjPList markers)
{
    AjBool result = AJFALSE;

    if(!ma)
        return ajFalse;

    if(!markers)
        return ajFalse;

    result = ensBaseadaptorFetchAllbyConstraint(ma,
                                                (AjPStr) NULL,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                markers);

    return result;
}




/* @funcstatic markeradaptorClearIdentifierTable ******************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Marker Adaptor-
** internal table. This function deletes the AJAX unsigned integer key and
** the AJAX boolean value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX boolean value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void markeradaptorClearIdentifierTable(void** key,
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
    AJFREE(*value);

    *key   = NULL;
    *value = NULL;

    return;
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
** @@
******************************************************************************/

AjBool ensMarkeradaptorFetchAllbySynonym(EnsPMarkeradaptor ma,
                                         const AjPStr name,
                                         const AjPStr source,
                                         AjPList markers)
{
    char* txtname   = NULL;
    char* txtsource = NULL;

    ajuint identifier = 0;

    ajuint* Pidentifier = NULL;

    AjBool* Pbool = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable table = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMarker marker = NULL;

    if(!ma)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!markers)
        return ajFalse;

    table = ensTableuintNewLen(0);

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

    if(source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        ajFmtPrintAppS(&statement,
                       " AND marker_synonym.source = '%s'",
                       txtsource);

        ajCharDel(&txtsource);
    }

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        marker = NULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);

        if(ajTableMatchV(table, (const void*) &identifier))
            continue;

        AJNEW0(Pidentifier);

        *Pidentifier = identifier;

        AJNEW0(Pbool);

        *Pbool = ajTrue;

        ajTablePut(table, (void*) Pidentifier, (void*) Pbool);

        ensMarkeradaptorFetchByIdentifier(ma, identifier, &marker);

        if(marker)
            ajListPushAppend(markers, (void*) marker);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    ajTableMapDel(table, markeradaptorClearIdentifierTable, NULL);

    ajTableFree(&table);

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
** @@
******************************************************************************/

AjBool ensMarkeradaptorFetchByIdentifier(EnsPMarkeradaptor ma,
                                         ajuint identifier,
                                         EnsPMarker* Pmarker)
{
    AjBool result = AJFALSE;

    AjPList markers = NULL;

    AjPStr constraint = NULL;

    EnsPMarker marker = NULL;

    if(!ma)
        return ajFalse;

    if(!Pmarker)
        return ajFalse;

    *Pmarker = NULL;

    constraint = ajFmtStr("marker.marker_id = %u", identifier);

    markers = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(ma,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                markers);

    if(ajListGetLength(markers) == 0)
        ajDebug("ensMarkeradaptorFetchByIdentifier got no Ensembl Marker "
                "for identifier %u.\n", identifier);

    if(ajListGetLength(markers) > 1)
        ajWarn("ensMarkeradaptorFetchByIdentifier got more than one "
               "Ensembl Marker for identifier %u.\n", identifier);

    ajListPop(markers, (void**) Pmarker);

    while(ajListPop(markers, (void**) &marker))
        ensMarkerDel(&marker);

    ajListFree(&markers);

    ajStrDel(&constraint);

    return result;
}




/*
** Fetch Ensembl Marker Synonym objects and Ensembl Maker Map Location objects
** of an Ensembl Marker.
**
** NOTE: This function is not ideal as it requires a Marker Adaptor although
** a Marker Adaptor is most likely already set in the Marker object.
** The Perl API version of this function does not test whether the Marker
** object has actually been generated from the same source database.
** By loading Marker Synonym objects and Marker Map Location objects via
** ensMarkerLoadMarkersynonyms and ensMarkerLoadMarkermaplocations,
** respectively, it is ensured that Marker and Attribute objects come from
** the same database.
*/




/* @obsolete ensMarkeradaptorFetchAttributes **********************************
**
** @remove ensMarkerLoadMarkersynonyms or ensMarkerLoadMarkermaplocations
**
******************************************************************************/

__deprecated AjBool ensMarkeradaptorFetchAttributes(EnsPMarkeradaptor ma,
                                                    EnsPMarker marker)
{
    float lodscore = 0.0F;

    ajuint msid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr msname    = NULL;
    AjPStr mssource  = NULL;
    AjPStr chrname   = NULL;
    AjPStr position  = NULL;
    AjPStr mapname   = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPMarkermaplocation mml = NULL;

    if(!ma)
        return ajFalse;

    if(!marker)
        return ajFalse;

    if(!marker->Identifier)
    {
        ajWarn("ensMarkeradaptorFetchAttributes got an Ensembl Marker "
               "without an identifier.");

        return ajFalse;
    }

    dba = ensBaseadaptorGetDatabaseadaptor(ma);

    /*
    ** First, retrieve all Ensembl Marker Synonym objects for this
    ** Ensembl Marker.
    */

    ensMarkerClearMarkersynonyms(marker);

    statement = ajFmtStr(
        "SELECT "
        "marker_synonym.marker_synonym_id, "
        "marker_synonym.source, "
        "marker_synonym.name "
        "FROM "
        "marker_synonym "
        "WHERE "
        "marker_synonym.marker_id = %u",
        marker->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        msid     = 0;
        msname   = ajStrNew();
        mssource = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &msid);
        ajSqlcolumnToStr(sqlr, &msname);
        ajSqlcolumnToStr(sqlr, &mssource);

        ms = ensMarkersynonymNewIni(msid, mssource, msname);

        ensMarkerAddMarkersynonym(marker, ms);

        ensMarkersynonymDel(&ms);

        ajStrDel(&msname);
        ajStrDel(&mssource);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    /*
    ** Now, retrieve all Ensembl Marker Map Location objects for this
    ** Ensembl Marker.
    */

    ensMarkerClearMarkermaplocations(marker);

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
        "map.map_id = marker_map_location.map_id "
        "AND "
        "marker_synonym.marker_synonym_id = "
        "marker_map_location.marker_synonym_id",
        marker->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        ensMarkerAddMarkermaplocation(marker, mml);

        ensMarkermaplocationDel(&mml);

        ensMarkersynonymDel(&ms);

        ajStrDel(&chrname);
        ajStrDel(&position);
        ajStrDel(&mapname);
        ajStrDel(&msname);
        ajStrDel(&mssource);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPMarkerfeature] Ensembl Marker Feature ********************
**
** @nam2rule Markerfeature Functions for manipulating
** Ensembl Marker Feature objects
**
** @cc Bio::EnsEMBL::Map::MarkerFeature
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-62
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
** @valrule * [EnsPMarkerfeature] Ensembl Marker Feature
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
** @@
******************************************************************************/

EnsPMarkerfeature ensMarkerfeatureNewCpy(const EnsPMarkerfeature mf)
{
    EnsPMarkerfeature pthis = NULL;

    if(!mf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = mf->Identifier;

    pthis->Adaptor = mf->Adaptor;

    pthis->Feature = ensFeatureNewRef(mf->Feature);

    pthis->Marker = ensMarkerNewRef(mf->Marker);

    pthis->Mapweight = mf->Mapweight;

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

    mf->Use = 1;

    mf->Identifier = identifier;

    mf->Adaptor = mfa;

    mf->Feature = ensFeatureNewRef(feature);

    mf->Marker = ensMarkerNewRef(marker);

    mf->Mapweight = mapweight;

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
** @@
******************************************************************************/

EnsPMarkerfeature ensMarkerfeatureNewRef(EnsPMarkerfeature mf)
{
    if(!mf)
        return NULL;

    mf->Use++;

    return mf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Feature object
**
** @argrule * Pmf [EnsPMarkerfeature*] Ensembl Marker Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMarkerfeatureDel **************************************************
**
** Default destructor for an Ensembl Marker Feature.
**
** @param [d] Pmf [EnsPMarkerfeature*] Ensembl Marker Feature object address
**
** @return [void]
** @@
******************************************************************************/

void ensMarkerfeatureDel(EnsPMarkerfeature* Pmf)
{
    EnsPMarkerfeature pthis = NULL;

    if(!Pmf)
        return;

    if(!*Pmf)
        return;

    if(ajDebugTest("ensMarkerfeatureDel"))
    {
        ajDebug("ensMarkerfeatureDel\n"
                "  *Pmf %p\n",
                *Pmf);

        ensMarkerfeatureTrace(*Pmf, 1);
    }

    pthis = *Pmf;

    pthis->Use--;

    if(pthis->Use)
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




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Marker Feature object.
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
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Marker [EnsPMarker] Ensembl Marker or NULL
** @valrule Mapweight [ajint] Map weight or 0
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerfeatureGetAdaptor *******************************************
**
** Get the Ensembl Marker Feature Adaptor element of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPMarkerfeatureadaptor ensMarkerfeatureGetAdaptor(
    const EnsPMarkerfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Adaptor;
}




/* @func ensMarkerfeatureGetFeature *******************************************
**
** Get the Ensembl Feature element of an Ensembl Marker Feature.
**
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensMarkerfeatureGetFeature(const EnsPMarkerfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Feature;
}




/* @func ensMarkerfeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensMarkerfeatureGetIdentifier(const EnsPMarkerfeature mf)
{
    if(!mf)
        return 0;

    return mf->Identifier;
}




/* @func ensMarkerfeatureGetMapweight *****************************************
**
** Get the map wight element of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::map_weight
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [ajint] Map weight or 0
** @@
******************************************************************************/

ajint ensMarkerfeatureGetMapweight(const EnsPMarkerfeature mf)
{
    if(!mf)
        return 0;

    return mf->Mapweight;
}




/* @func ensMarkerfeatureGetMarker ********************************************
**
** Get the Ensembl Marker element of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::marker
** @param [r] mf [const EnsPMarkerfeature] Ensembl Marker Feature
**
** @return [EnsPMarker] Ensembl Marker or NULL
** @@
******************************************************************************/

EnsPMarker ensMarkerfeatureGetMarker(const EnsPMarkerfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Marker;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Marker Feature object.
**
** @fdata [EnsPMarkerfeature]
**
** @nam3rule Set Set one element of an Ensembl Marker Feature
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
** Set the Ensembl Marker Feature Adaptor element of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetAdaptor(EnsPMarkerfeature mf,
                                  EnsPMarkerfeatureadaptor mfa)
{
    if(!mf)
        return ajFalse;

    mf->Adaptor = mfa;

    return ajTrue;
}




/* @func ensMarkerfeatureSetFeature *******************************************
**
** Set the Ensembl Feature element of an Ensembl Marker Feature.
**
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetFeature(EnsPMarkerfeature mf, EnsPFeature feature)
{
    if(!mf)
        return ajFalse;

    ensFeatureDel(&mf->Feature);

    mf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensMarkerfeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetIdentifier(EnsPMarkerfeature mf, ajuint identifier)
{
    if(!mf)
        return ajFalse;

    mf->Identifier = identifier;

    return ajTrue;
}




/* @func ensMarkerfeatureSetMapweight *****************************************
**
** Set the map weight element of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::map_weight
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [r] mapweight [ajint] Map weight
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetMapweight(EnsPMarkerfeature mf, ajint mapweight)
{
    if(!mf)
        return ajFalse;

    mf->Mapweight = mapweight;

    return ajTrue;
}




/* @func ensMarkerfeatureSetMarker ********************************************
**
** Set the Ensembl Marker element of an Ensembl Marker Feature.
**
** @cc Bio::EnsEMBL::Map::MarkerFeature::marker
** @param [u] mf [EnsPMarkerfeature] Ensembl Marker Feature
** @param [u] marker [EnsPMarker] Ensembl Marker
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMarkerfeatureSetMarker(EnsPMarkerfeature mf, EnsPMarker marker)
{
    if(!mf)
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
** @nam3rule Trace Report Ensembl Marker Feature elements to debug file.
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
** @@
******************************************************************************/

AjBool ensMarkerfeatureTrace(const EnsPMarkerfeature mf, ajuint level)
{
    AjPStr indent = NULL;

    if(!mf)
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
** @@
******************************************************************************/

size_t ensMarkerfeatureCalculateMemsize(const EnsPMarkerfeature mf)
{
    size_t size = 0;

    if(!mf)
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




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Markerfeature Functions for manipulating AJAX List objects of
** Ensembl Marker Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
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




/* @funcstatic listMarkerfeatureCompareStartAscending *************************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** Ensembl Feature start element in ascending order.
**
** @param [r] P1 [const void*] Ensembl Marker Feature address 1
** @param [r] P2 [const void*] Ensembl Marker Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listMarkerfeatureCompareStartAscending(const void* P1,
                                                  const void* P2)
{
    const EnsPMarkerfeature mf1 = NULL;
    const EnsPMarkerfeature mf2 = NULL;

    mf1 = *(EnsPMarkerfeature const*) P1;
    mf2 = *(EnsPMarkerfeature const*) P2;

    if(ajDebugTest("listMarkerfeatureCompareStartAscending"))
        ajDebug("listMarkerfeatureCompareStartAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(mf1 && (!mf2))
        return -1;

    if((!mf1) && (!mf2))
        return 0;

    if((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartAscending(mf1->Feature, mf2->Feature);
}




/* @func ensListMarkerfeatureSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** Ensembl Feature start element in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortStartAscending(AjPList mfs)
{
    if(!mfs)
        return ajFalse;

    ajListSort(mfs, listMarkerfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listMarkerfeatureCompareStartDescending ************************
**
** AJAX List of Ensembl Marker Feature objects comparison function to sort by
** Ensembl Feature start element in descending order.
**
** @param [r] P1 [const void*] Ensembl Marker Feature address 1
** @param [r] P2 [const void*] Ensembl Marker Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listMarkerfeatureCompareStartDescending(const void* P1,
                                                   const void* P2)
{
    const EnsPMarkerfeature mf1 = NULL;
    const EnsPMarkerfeature mf2 = NULL;

    mf1 = *(EnsPMarkerfeature const*) P1;
    mf2 = *(EnsPMarkerfeature const*) P2;

    if(ajDebugTest("listMarkerfeatureCompareStartDescending"))
        ajDebug("listMarkerfeatureCompareStartDescending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(mf1 && (!mf2))
        return -1;

    if((!mf1) && (!mf2))
        return 0;

    if((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartDescending(mf1->Feature, mf2->Feature);
}




/* @func ensListMarkerfeatureSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Marker Feature objects by their
** Ensembl Feature start element in descending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListMarkerfeatureSortStartDescending(AjPList mfs)
{
    if(!mfs)
        return ajFalse;

    ajListSort(mfs, listMarkerfeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor *****
**
** @nam2rule Markerfeatureadaptor Functions for manipulating
** Ensembl Marker Feature Adaptor objects
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor
** @cc CVS Revision: 1.16
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic markerfeatureadaptorFetchAllbyStatement ************************
**
** Fetch all Ensembl Marker Feature objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] mfs [AjPList] AJAX List of Ensembl Marker Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool markerfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs)
{
    ajint mapweight = 0;
    ajint priority  = 0;
    ajint slstart   = 0;
    ajint slend     = 0;
    ajint slstrand  = 0;
    ajint sllength  = 0;

    ajuint identifier  = 0;
    ajuint srid        = 0;
    ajuint srstart     = 0;
    ajuint srend       = 0;
    ajuint analysisid  = 0;
    ajuint markerid    = 0;
    ajuint mindistance = 0;
    ajuint maxdistance = 0;
    ajuint msid        = 0;

    ajuint* Pidentifier = NULL;

    EnsEMarkerType etype = ensEMarkerTypeNULL;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr lprimer  = NULL;
    AjPStr rprimer  = NULL;
    AjPStr type     = NULL;
    AjPStr msname   = NULL;
    AjPStr mssource = NULL;

    AjPTable markers = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPMarker marker    = NULL;
    EnsPMarkeradaptor ma = NULL;

    EnsPMarkerfeature mf         = NULL;
    EnsPMarkerfeatureadaptor mfa = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("markerfeatureadaptorFetchAllbyStatement"))
        ajDebug("markerfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  mfs %p\n",
                dba,
                statement,
                am,
                slice,
                mfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!mfs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    ma = ensRegistryGetMarkeradaptor(dba);

    mfa = ensRegistryGetMarkerfeatureadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    markers = ensTableuintNewLen(0);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Feature objects, the range
        ** needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("markerfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("markerfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region end coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srend, INT_MAX);

        slstrand = 1;

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

                ajStrDel(&lprimer);
                ajStrDel(&rprimer);
                ajStrDel(&type);
                ajStrDel(&msname);
                ajStrDel(&mssource);

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
                ajFatal("markerfeatureadaptorFetchAllbyStatement got a "
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

                ajStrDel(&lprimer);
                ajStrDel(&rprimer);
                ajStrDel(&type);
                ajStrDel(&msname);
                ajStrDel(&mssource);

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

        marker = (EnsPMarker) ajTableFetchmodV(markers,
                                               (const void*) &markerid);

        if(!marker)
        {
            if(msid)
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

            ajTablePut(markers, (void*) Pidentifier, (void*) marker);

            ensMarkersynonymDel(&ms);
        }

        mf = ensMarkerfeatureNewIni(mfa,
                                    identifier,
                                    feature,
                                    marker,
                                    mapweight);

        ajListPushAppend(mfs, (void*) mf);

        ensFeatureDel(&feature);

        ensAnalysisDel(&analysis);

        ajStrDel(&lprimer);
        ajStrDel(&rprimer);
        ajStrDel(&type);
        ajStrDel(&msname);
        ajStrDel(&mssource);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    ensTableMarkerDelete(&markers);

    return ajTrue;
}




/* @funcstatic markerfeatureadaptorCacheReference *****************************
**
** Wrapper function to reference an Ensembl Marker Feature
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Marker Feature
**
** @return [void*] Ensembl Marker Feature or NULL
** @@
******************************************************************************/

static void* markerfeatureadaptorCacheReference(void*value)
{
    if(!value)
        return NULL;

    return (void*) ensMarkerfeatureNewRef((EnsPMarkerfeature) value);
}




/* @funcstatic markerfeatureadaptorCacheDelete ********************************
**
** Wrapper function to delete an Ensembl Marker Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Marker Feature address
**
** @return [void]
** @@
******************************************************************************/

static void markerfeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensMarkerfeatureDel((EnsPMarkerfeature*) value);

    return;
}




/* @funcstatic markerfeatureadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Marker Feature
** via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Marker Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t markerfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensMarkerfeatureCalculateMemsize((const EnsPMarkerfeature) value);
}




/* @funcstatic markerfeatureadaptorGetFeature *********************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Marker Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Marker Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature markerfeatureadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensMarkerfeatureGetFeature((const EnsPMarkerfeature) value);
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
** @valrule * [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
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
** @@
******************************************************************************/

EnsPMarkerfeatureadaptor ensMarkerfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        markerfeatureadaptorTables,
        markerfeatureadaptorColumns,
        markerfeatureadaptorLeftjoin,
        markerfeatureadaptorDefaultcondition,
        (const char*) NULL,
        markerfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        markerfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        markerfeatureadaptorCacheDelete,
        markerfeatureadaptorCacheSize,
        markerfeatureadaptorGetFeature,
        "Marker Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Marker Feature Adaptor object.
**
** @fdata [EnsPMarkerfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Marker Feature Adaptor object.
**
** @argrule * Pmfa [EnsPMarkerfeatureadaptor*] Ensembl Marker Feature Adaptor
**                                             object address
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
** @param [d] Pmfa [EnsPMarkerfeatureadaptor*] Ensembl Marker Feature Adaptor
**                                             object address
**
** @return [void]
** @@
******************************************************************************/

void ensMarkerfeatureadaptorDel(EnsPMarkerfeatureadaptor* Pmfa)
{
    if(!Pmfa)
        return;

    if(!*Pmfa)
        return;

    ensFeatureadaptorDel(Pmfa);

    *Pmfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Marker Feature Adaptor
** object.
**
** @fdata [EnsPMarkerfeatureadaptor]
**
** @nam3rule Get Return Marker Feature Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * mfa [EnsPMarkerfeatureadaptor] Marker Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMarkerfeatureadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Marker Feature Adaptor.
**
** @param [u] mfa [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMarkerfeatureadaptorGetDatabaseadaptor(
    EnsPMarkerfeatureadaptor mfa)
{
    if(!mfa)
        return NULL;

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

    if(!mfa)
        return ajFalse;

    if(!marker)
        return ajFalse;

    if(!mfs)
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

    if(!mfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!mfs)
        return ajFalse;

    if(priority)
        constraint = ajFmtStr("marker.priority > %d", priority);

    if(mapweight)
    {
        if(constraint)
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
** @@
******************************************************************************/

AjBool ensMarkerfeatureadaptorFetchAllbySlicemarkername(
    EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPStr name,
    AjPList mfs)
{
    char* txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!mfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!mfs)
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

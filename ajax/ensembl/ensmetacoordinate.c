/******************************************************************************
** @source Ensembl Meta-Coordinate functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.8 $
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

#include "ensmetacoordinate.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool metacoordinateadaptorCacheInit(EnsPMetacoordinateadaptor mca);

static void metacoordinateadaptorClearNameCache(void **key,
                                                void **value,
                                                void *cl);

static void metacoordinateadaptorClearLengthCacheL2(void **key,
                                                    void **value,
                                                    void *cl);

static void metacoordinateadaptorClearLengthCacheL1(void **key,
                                                    void **value,
                                                    void *cl);




/* @filesection ensmetacoordinate *********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMetacoordinateadaptor] Meta-Coordinate Adaptor ***********
**
** Functions for manipulating Ensembl Meta-Coordinate Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer CVS Revision: 1.12
**
** @nam2rule Metacoordinateadaptor
**
******************************************************************************/




/* @funcstatic metacoordinateadaptorCacheInit *********************************
**
** Initialise the internal Meta-Coordinate cache of an
** Ensembl Meta-Coordinate Adaptor.
**
** @param [u] mca [EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool metacoordinateadaptorCacheInit(EnsPMetacoordinateadaptor mca)
{
    ajuint csid   = 0;
    ajuint length = 0;

    ajuint *Pcsid   = NULL;
    ajuint *Plength = NULL;

    AjPList css  = NULL;
    AjPList list = NULL;

    AjPTable table = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr csv       = NULL;
    AjPStr name      = NULL;
    AjPStr statement = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    if(!mca)
        return ajFalse;

    /* Fetch all Ensembl Coordinate Systems for this species. */

    csa = ensRegistryGetCoordsystemadaptor(mca->Adaptor);

    css = ajListNew();

    ensCoordsystemadaptorFetchAll(csa, css);

    csv = ajStrNew();

    while(ajListPop(css, (void **) &cs))
    {
        ajFmtPrintAppS(&csv, "%u, ", ensCoordsystemGetIdentifier(cs));

        ensCoordsystemDel(&cs);
    }

    ajListFree(&css);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    statement = ajFmtStr(
        "SELECT "
        "meta_coord.table_name, "
        "meta_coord.coord_system_id, "
        "meta_coord.max_length "
        "FROM "
        "meta_coord "
        "WHERE "
        "meta_coord.coord_system_id IN (%S)",
        csv);

    ajStrDel(&csv);

    sqls = ensDatabaseadaptorSqlstatementNew(mca->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        name = ajStrNew();

        csid = 0;

        length = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToUint(sqlr, &csid);
        ajSqlcolumnToUint(sqlr, &length);

        ajStrFmtLower(&name);

        /* Add to the Feature Type Cache. */

        list = (AjPList) ajTableFetch(mca->CacheByName, (const void *) name);

        if(!list)
        {
            list = ajListNew();

            ajTablePut(mca->CacheByName,
                       (void *) ajStrNewS(name),
                       (void *) list);
        }

        AJNEW0(Pcsid);

        *Pcsid = csid;

        ajListPushAppend(list, (void *) Pcsid);

        /* Add to the Length Cache. */

        table = (AjPTable) ajTableFetch(mca->CacheByLength,
                                        (const void *) &csid);

        if(!table)
        {
            AJNEW0(Pcsid);

            *Pcsid = csid;

            table = ajTablestrNewCaseLen(0);

            ajTablePut(mca->CacheByLength, (void *) Pcsid, (void *) table);
        }

        AJNEW0(Plength);

        *Plength = length;

        ajTablePut(table, (void *) ajStrNewS(name), (void *) Plength);

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(mca->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Meta-Coordinate Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Meta-Coordinate Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMetacoordinateadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensMetacoordinateadaptorNew ******************************************
**
** Default constructor for an Ensembl Meta-Coordinate Adaptor.
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
** @see ensRegistryGetMetacoordinateadaptor
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate Adaptor or NULL
** @@
******************************************************************************/

EnsPMetacoordinateadaptor ensMetacoordinateadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMetacoordinateadaptor mca = NULL;

    if(!dba)
        return NULL;

    AJNEW0(mca);

    mca->Adaptor = dba;

    mca->CacheByName = ajTablestrNewCaseLen(0);

    mca->CacheByLength = MENSTABLEUINTNEW(0);

    metacoordinateadaptorCacheInit(mca);

    return mca;
}




/* @funcstatic metacoordinateadaptorClearNameCache ****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Meta-Coordinate
** Adaptor-internal cache. This function deletes the AJAX String Feature name
** key data and the AJAX List value data, as well as the pointers to ajuint
** Coordinate System identifiers list elements.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX List value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metacoordinateadaptorClearNameCache(void **key,
                                                void **value,
                                                void *cl)
{
    void *Pcsid = NULL;

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

    while(ajListPop(*((AjPList *) value), &Pcsid))
        AJFREE(Pcsid);

    ajListFree((AjPList *) value);

    return;
}




/* @funcstatic metacoordinateadaptorClearLengthCacheL2 ************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Meta-Coordinate
** Adaptor-internal maximum length cache. This function clears the
** second-level cache of Ensembl Feture name AJAX String key data and
** pointer to ajuint maximum length value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX unsigned integer value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metacoordinateadaptorClearLengthCacheL2(void **key,
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

    AJFREE(*value);

    return;
}




/* @funcstatic metacoordinateadaptorClearLengthCacheL1 ************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Meta-Coordinate
** Adaptor-internal maximum length cache. This function clears the
** first-level cache of ajuint Coordinate System identifier key data and the
** second-level AJAX Table value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX Table value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metacoordinateadaptorClearLengthCacheL1(void **key,
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

    ajTableMapDel(*((AjPTable *) value),
                  metacoordinateadaptorClearLengthCacheL2,
                  NULL);

    ajTableFree((AjPTable *) value);

    return;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Meta-Coordinate Adaptor.
**
** @fdata [EnsPMetacoordinateadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Meta-Coordinate Adaptor object
**
** @argrule * Pmca [EnsPMetacoordinateadaptor*] Ensembl Meta-Coordinate
**                                              Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMetacoordinateadaptorDel ******************************************
**
** Default destructor for an Ensembl Meta-Coordinate Adaptor.
** This function also clears the internal cordinate system identifer and
** maximum length caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pmca [EnsPMetacoordinateadaptor*] Ensembl Meta-Coordinate
**                                              Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensMetacoordinateadaptorDel(EnsPMetacoordinateadaptor *Pmca)
{
    EnsPMetacoordinateadaptor pthis = NULL;

    if(!Pmca)
        return;

    if(!*Pmca)
        return;

    pthis = *Pmca;

    ajTableMapDel(pthis->CacheByName,
                  metacoordinateadaptorClearNameCache,
                  NULL);

    ajTableFree(&pthis->CacheByName);

    ajTableMapDel(pthis->CacheByLength,
                  metacoordinateadaptorClearLengthCacheL1,
                  NULL);

    ajTableFree(&pthis->CacheByLength);

    AJFREE(pthis);

    *Pmca = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning Ensembl Coordinate Systems.
**
** @fdata [EnsPMetacoordinateadaptor]
** @fnote None
**
** @nam3rule Fetch Fetch Fetch data
** @nam4rule FetchAll Fetch all data
** @nam5rule FetchAllCoordsystems Fetch coordinate systems
** @nam3rule Get   Return an attribute
** @nam4rule GetMaximumlength   Return maximum length
**
** @argrule * mca [const EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate
**                                                Adaptor
** @argrule Fetch name [const AjPStr] Ensembl Feature (i.e. SQL table) name
** @argrule Fetch css [AjPList] AJAX List of Ensembl Coordinate Systems
** @argrule Get cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule Get name [const AjPStr] Ensembl Feature (i.e. SQL table) name
**
** @valrule * [AjBool] True on success
** @valrule *Maximumlength [ajuint] Maximum length
**
** @fcategory use
******************************************************************************/




/* @func ensMetacoordinateadaptorFetchAllCoordsystems *************************
**
** Fetch all Ensembl Coordinate Systems associated with a particular
** Ensembl Feature name.
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer::
**     fetch_all_Coordsystems_by_feature_type
** @param [r] mca [const EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate
**                                                 Adaptor
** @param [r] name [const AjPStr] Ensembl Feature (i.e. SQL table) name
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetacoordinateadaptorFetchAllCoordsystems(
    const EnsPMetacoordinateadaptor mca,
    const AjPStr name,
    AjPList css)
{
    ajuint *Pcsid = NULL;

    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    if(ajDebugTest("ensMetacoordinateadaptorFetchAllCoordsystems"))
        ajDebug("ensMetacoordinateadaptorFetchAllCoordsystems\n"
                "  mca %p\n"
                "  name '%S'\n"
                "  css %p\n",
                mca,
                name,
                css);

    if(!mca)
        return ajFalse;

    if(!name)
        return ajFalse;

    list = (AjPList) ajTableFetch(mca->CacheByName, (const void *) name);

    if(!list)
    {
        ajDebug("ensMetacoordinateadaptorFetchAllCoordsystems "
                "got no Coordinate System List for name '%S'\n", name);

        return ajTrue;
    }

    csa = ensRegistryGetCoordsystemadaptor(mca->Adaptor);

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        Pcsid = (ajuint *) ajListIterGet(iter);

        ensCoordsystemadaptorFetchByIdentifier(csa, *Pcsid, &cs);

        if(cs)
            ajListPush(css, (void *) cs);
        else
            ajFatal("ensMetacoordinateadaptorFetchAllCoordsystems "
                    "Ensembl Core 'meta_coord' table refers to non-existing "
                    "Ensembl Coordinate System with identifier %u\n", *Pcsid);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensMetacoordinateadaptorGetMaximumlength *****************************
**
** Get the maximum length of a particular Ensembl Feature type in an
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer::
**     fetch_max_length_by_Coordsystem_feature_type
** @param [r] mca [const EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate
**                                                  Adaptor
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [r] name [const AjPStr] Ensembl Feature (i.e. SQL table) name
**
** @return [ajuint] Maximum length
** @@
******************************************************************************/

ajuint ensMetacoordinateadaptorGetMaximumlength(
    const EnsPMetacoordinateadaptor mca,
    const EnsPCoordsystem cs,
    const AjPStr name)
{
    ajuint *Pcsid = NULL;
    ajuint *Plength = NULL;

    AjPTable table = NULL;

    AJNEW0(Pcsid);

    *Pcsid = ensCoordsystemGetIdentifier(cs);

    table = (AjPTable)
        ajTableFetch(mca->CacheByLength, (const void *) Pcsid);

    AJFREE(Pcsid);

    if(table)
        Plength = (ajuint *) ajTableFetch(table, (const void *) name);
    else
        return 0;

    return *Plength;
}

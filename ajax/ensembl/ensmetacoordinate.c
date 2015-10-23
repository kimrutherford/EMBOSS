/* @source ensmetacoordinate **************************************************
**
** Ensembl Meta-Coordinate functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.38 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
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

#include "ensmetacoordinate.h"
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




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool metacoordinateadaptorCacheInit(EnsPMetacoordinateadaptor mca);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensmetacoordinate *********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate Adaptor ***
**
** @nam2rule Metacoordinateadaptor Functions for manipulating
** Ensembl Meta-Coordinate Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer
** @cc CVS Revision: 1.16
** @cc CVS Tag: branch-ensembl-68
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
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool metacoordinateadaptorCacheInit(EnsPMetacoordinateadaptor mca)
{
    ajuint csid   = 0U;
    ajint  length = 0;

    ajuint *Pcsid   = NULL;
    ajint  *Plength = NULL;

    AjPList css  = NULL;
    AjPList list = NULL;

    AjPTable table = NULL;

    AjISqlrow       sqli = NULL;
    AjPSqlrow       sqlr = NULL;
    AjPSqlstatement sqls = NULL;

    AjPStr csv       = NULL;
    AjPStr name      = NULL;
    AjPStr statement = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!mca)
        return ajFalse;

    /* Fetch all Ensembl Coordinate System objects for this species. */

    dba = ensMetacoordinateadaptorGetDatabaseadaptor(mca);

    css = ajListNew();

    ensCoordsystemadaptorFetchAll(
        ensRegistryGetCoordsystemadaptor(dba),
        css);

    csv = ajStrNew();

    while (ajListPop(css, (void **) &cs))
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

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        name   = ajStrNew();
        csid   = 0;
        length = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToUint(sqlr, &csid);
        ajSqlcolumnToInt(sqlr, &length);

        ajStrFmtLower(&name);

        /* Add to the Feature Type Cache. */

        list = (AjPList) ajTableFetchmodS(mca->CacheByName, name);

        if (!list)
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

        table = (AjPTable) ajTableFetchmodV(mca->CacheByLength,
                                            (const void *) &csid);

        if (!table)
        {
            AJNEW0(Pcsid);

            *Pcsid = csid;

            table = ajTablestrNew(0U);

            ajTableSetDestroyvalue(table, &ajMemFree);

            ajTablePut(mca->CacheByLength, (void *) Pcsid, (void *) table);
        }

        AJNEW0(Plength);

        *Plength = length;

        ajTablePut(table, (void *) ajStrNewS(name), (void *) Plength);

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

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
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor or NULL
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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMetacoordinateadaptor ensMetacoordinateadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMetacoordinateadaptor mca = NULL;

    if (!dba)
        return NULL;

    AJNEW0(mca);

    mca->Adaptor = dba;

    mca->CacheByName = ajTablestrNew(0U);

    ajTableSetDestroyvalue(
        mca->CacheByName,
        (void (*)(void **)) &ajListFreeData);

    mca->CacheByLength = ajTableuintNew(0U);

    ajTableSetDestroyvalue(
        mca->CacheByLength,
        (void (*)(void **)) &ajTableDel);

    metacoordinateadaptorCacheInit(mca);

    return mca;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Meta-Coordinate Adaptor object.
**
** @fdata [EnsPMetacoordinateadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Meta-Coordinate Adaptor
**
** @argrule * Pmca [EnsPMetacoordinateadaptor*]
** Ensembl Meta-Coordinate Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMetacoordinateadaptorDel ******************************************
**
** Default destructor for an Ensembl Meta-Coordinate Adaptor.
**
** This function also clears the internal cordinate system identifer and
** maximum length caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pmca [EnsPMetacoordinateadaptor*]
** Ensembl Meta-Coordinate Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMetacoordinateadaptorDel(EnsPMetacoordinateadaptor *Pmca)
{
    EnsPMetacoordinateadaptor pthis = NULL;

    if (!Pmca)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMetacoordinateadaptorDel"))
        ajDebug("ensMetacoordinateadaptorDel\n"
                "  *Pmca %p\n",
                *Pmca);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pmca))
        return;

    ajTableDel(&pthis->CacheByName);
    ajTableDel(&pthis->CacheByLength);

    ajMemFree((void **) Pmca);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Meta-Coordinate Adaptor object.
**
** @fdata [EnsPMetacoordinateadaptor]
**
** @nam3rule Get Return Ensembl Meta-Coordinate Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * mca [EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMetacoordinateadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor of an Ensembl Meta-Coordinate Adaptor.
**
** @param [u] mca [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMetacoordinateadaptorGetDatabaseadaptor(
    EnsPMetacoordinateadaptor mca)
{
    return (mca) ? mca->Adaptor : NULL;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Meta-Coordinate objects
** from an Ensembl SQL database.
**
** @fdata [EnsPMetacoordinateadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Meta-Coordinate-releated object(s)
** @nam4rule All Retrieve all Ensembl Meta-Coordinate-releated objects
** @nam5rule Coordsystems Retrieve all Ensembl Coordinate System objects
** @nam4rule Maximumlength Retrieve the maximum length
**
** @argrule * mca [EnsPMetacoordinateadaptor] Ensembl Meta-Coordinate Adaptor
** @argrule AllCoordsystems name [const AjPStr]
** Ensembl Feature (i.e. SQL table) name
** @argrule AllCoordsystems css [AjPList]
** AJAX List of Ensembl Coordinate System objects
** @argrule Maximumlength cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule Maximumlength name [const AjPStr]
** Ensembl Feature (i.e. SQL table) name
** @argrule Maximumlength Pmaxlength [ajint*]
** AJAX signed integer (maximum length) address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMetacoordinateadaptorRetrieveAllCoordsystems **********************
**
** Retrieve all Ensembl Coordinate System objects associated with a particular
** Ensembl Feature name.
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer::
**     fetch_all_Coordsystems_by_feature_type
** @param [u] mca [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor
** @param [r] name [const AjPStr] Ensembl Feature (i.e. SQL table) name
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate System objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensMetacoordinateadaptorRetrieveAllCoordsystems(
    EnsPMetacoordinateadaptor mca,
    const AjPStr name,
    AjPList css)
{
    ajuint *Pcsid = NULL;

    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (ajDebugTest("ensMetacoordinateadaptorRetrieveAllCoordsystems"))
        ajDebug("ensMetacoordinateadaptorRetrieveAllCoordsystems\n"
                "  mca %p\n"
                "  name '%S'\n"
                "  css %p\n",
                mca,
                name,
                css);

    if (!mca)
        return ajFalse;

    if (!name)
        return ajFalse;

    list = (AjPList) ajTableFetchmodS(mca->CacheByName, name);

    if (!list)
    {
        ajDebug("ensMetacoordinateadaptorRetrieveAllCoordsystems "
                "got no AJAX List of "
                "Ensembl Coordinate System objects for name '%S'\n", name);

        return ajTrue;
    }

    dba = ensMetacoordinateadaptorGetDatabaseadaptor(mca);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    iter = ajListIterNew(list);

    while (!ajListIterDone(iter))
    {
        Pcsid = (ajuint *) ajListIterGet(iter);

        ensCoordsystemadaptorFetchByIdentifier(csa, *Pcsid, &cs);

        if (cs)
            ajListPush(css, (void *) cs);
        else
            ajFatal("ensMetacoordinateadaptorRetrieveAllCoordsystems "
                    "Ensembl Core 'meta_coord' table refers to non-existing "
                    "Ensembl Coordinate System with identifier %u\n", *Pcsid);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensMetacoordinateadaptorRetrieveMaximumlength ************************
**
** Retrieve the maximum length of a particular Ensembl Feature type in an
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::MetaCoordContainer::
**     fetch_max_length_by_Coordsystem_feature_type
** @param [u] mca [EnsPMetacoordinateadaptor]
** Ensembl Meta-Coordinate Adaptor
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [r] name [const AjPStr] Ensembl Feature (i.e. SQL table) name
** @param [wP] Pmaxlength [ajint*]
** AJAX signed integer (maximum length) address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensMetacoordinateadaptorRetrieveMaximumlength(
    EnsPMetacoordinateadaptor mca,
    const EnsPCoordsystem cs,
    const AjPStr name,
    ajint *Pmaxlength)
{
    ajuint csid = 0U;

    ajint *Plength = NULL;

    AjPTable table = NULL;

    if (!mca)
        return ajFalse;

    if (!cs)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!Pmaxlength)
        return ajFalse;

    *Pmaxlength = 0;

    csid = ensCoordsystemGetIdentifier(cs);

    table = (AjPTable) ajTableFetchmodV(mca->CacheByLength,
                                        (const void *) &csid);

    if (table)
    {
        Plength = (ajint *) ajTableFetchmodV(table, (const void *) name);

        *Pmaxlength = *Plength;

        return ajTrue;
    }

    return ajFalse;
}

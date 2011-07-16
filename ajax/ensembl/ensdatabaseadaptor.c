/* @source Ensembl Database Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.28 $
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

#include "ensdatabaseadaptor.h"
#include "ensmetainformation.h"
#include "enssoftware.h"




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

/* @conststatic databaseadaptorGroup ******************************************
**
** The Ensembl Database Adaptor group element is enumerated in the data
** structure. The following strings are used for conversion in database
** operations and correspond to EnsEDatabaseadaptorGroup.
**
******************************************************************************/

static const char* databaseadaptorGroup[] =
{
    "",
    "core",
    "vega",
    "otherfeatures",
    "cdna",
    "variation",
    "funcgen",
    "compara",
    "ontology",
    "qc",
    "pipeline",
    "hive",
    "coreexpressionest",
    "coreexpressiongnf",
    "ancestral",
    "website",
    "production",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensdatabaseadaptor ********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPDatabaseadaptor] Ensembl Database Adaptor ****************
**
** @nam2rule Databaseadaptor Functions for manipulating
** Ensembl Database Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor
** @cc CVS Revision: 1.219
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Database Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Database Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule New Constructor
** @nam4rule Databasename Constructor with a database name
** @nam4rule Ini Constructor with initial values
** @nam4rule Url Constructor with a Uniform Resource Locator (URL)
**
** @argrule Databasename dbc [EnsPDatabaseconnection]
** Ensembl Database Connection
** @argrule Databasename database [AjPStr] Database name
** @argrule Ini dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @argrule Ini database [AjPStr] Database name
** @argrule Ini species [AjPStr] Species
** @argrule Ini dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
** @argrule Ini multi [AjBool] Multiple species
** @argrule Ini identifier [ajuint] Species identifier
** @argrule Url url [const AjPStr] Uniform Resource Locator
**
** @valrule * [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseadaptorNewDatabasename ************************************
**
** Constructor for an Ensembl Database Adaptor with a database name.
**
** An Ensembl database name encodes all information to create an
** Ensembl Database Adaptor. This function parses the database name and sets
** the species and Ensembl Database Adaptor Group accordingly. For collection
** databases only a single Ensembl Database Adaptor representing the entire
** collection is returned, which has the multi-species member set to ajTrue and
** the species indentifier set to 0. An Ensembl Database Adaptor representing a
** collection has to be expanded into species-specific ones.
**
** Ensembl Database Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Database Adaptor connected to a particular database
** is sufficient to instantiate any number of Ensembl Object Adaptors from the
** database. Each Ensembl Object Adaptor will have a weak reference to the
** Ensembl Database Adaptor that instantiated it. Therefore, Ensembl Database
** Adaptors should not be instantiated directly, but rather obtained from the
** Ensembl Registry, which will in turn call this function if neccessary.
**
** @see ensRegistryNewDatabaseadaptor
** @see ensRegistryGetDatabaseadaptor
**
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] database [AjPStr] Database name
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseadaptorNewDatabasename(
    EnsPDatabaseconnection dbc,
    AjPStr database)
{
    AjBool debug = AJFALSE;

    AjPRegexp multire      = NULL;
    AjPRegexp speciesre    = NULL;
    AjPRegexp collectionre = NULL;

    AjPStr group     = NULL;
    AjPStr prefix    = NULL;
    AjPStr swversion = NULL;
    AjPStr multi     = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    EnsPDatabaseadaptor dba = NULL;

    debug = ajDebugTest("ensDatabaseadaptorNewDatabasename");

    if(debug)
    {
        ajDebug("ensDatabaseadaptorNewDatabasename\n"
                "  dbc %p\n"
                "  database '%S'\n",
                dbc,
                database);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dbc)
        return NULL;

    if((database == NULL) || (ajStrGetLen(database) == 0))
        return NULL;

    collectionre =
        ajRegCompC("^\\w+_collection_([a-z]+)(?:_\\d+)?_(\\d+)_\\w+");

    multire =
        ajRegCompC("^ensembl_([a-z]+)(_\\w+?)*?(?:_\\d+)?_(\\d+)$");

    speciesre =
        ajRegCompC("^([a-z]+_[a-z0-9]+)_([a-z]+)(?:_\\d+)?_(\\d+)_\\w+");

    multi = ajStrNewC("default");

    if(ajRegExec(collectionre, database))
    {
        /* Ensembl Collection databases have to be matched first. */

        group     = ajStrNew();
        swversion = ajStrNew();

        ajRegSubI(collectionre, 1, &group);
        ajRegSubI(collectionre, 2, &swversion);

        if(ajStrMatchCaseC(swversion, ensSoftwareVersion))
        {
            if(debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "collection database '%S'.\n",
                        database);

            dbag = ensDatabaseadaptorGroupFromStr(group);

            if(dbag)
                dba = ensDatabaseadaptorNewIni(dbc,
                                               database,
                                               prefix,
                                               dbag,
                                               ajTrue,
                                               0);
            else
                ajDebug("ensDatabaseadaptorNewDatabasename got unexpected "
                        "group string '%S' for database name '%S'.\n",
                        group, database);
        }
        else
        {
            if(debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "collection database '%S', "
                        "but not the software version '%s'.\n",
                        database, ensSoftwareVersion);
        }

        ajStrDel(&group);
        ajStrDel(&swversion);
    }
    else if(ajRegExec(multire, database))
    {
        /* Multi-species databases */

        group     = ajStrNew();
        prefix    = ajStrNew();
        swversion = ajStrNew();

        ajRegSubI(multire, 1, &group);

        if(ajRegLenI(multire, 3))
        {
            ajRegSubI(multire, 2, &prefix);
            ajRegSubI(multire, 3, &swversion);

            /* Remove leading underscores from the species prefix. */

            if(ajStrPrefixC(prefix, "_"))
                ajStrCutStart(&prefix, 1);
        }
        else
            ajRegSubI(multire, 2, &swversion);

        if(ajStrMatchCaseC(swversion, ensSoftwareVersion))
        {
            if(debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "multi-species database '%S'.\n",
                        database);

            dbag = ensDatabaseadaptorGroupFromStr(group);

            if(dbag)
                dba = ensDatabaseadaptorNewIni(dbc,
                                               database,
                                               (ajStrGetLen(prefix))
                                               ? prefix : multi,
                                               dbag,
                                               ajFalse,
                                               0);
            else
                ajDebug("ensDatabaseadaptorNewDatabasename got unexpected "
                        "group string '%S' for database name '%S'.\n",
                        group, database);
        }
        else
        {
            if(debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "multi-species database '%S', "
                        "but not the software version '%s'.\n",
                        database, ensSoftwareVersion);
        }

        ajStrDel(&group);
        ajStrDel(&prefix);
        ajStrDel(&swversion);
    }
    else if(ajRegExec(speciesre, database))
    {
        /* Species-specific databases */

        prefix    = ajStrNew();
        group     = ajStrNew();
        swversion = ajStrNew();

        ajRegSubI(speciesre, 1, &prefix);
        ajRegSubI(speciesre, 2, &group);
        ajRegSubI(speciesre, 3, &swversion);

        if(ajStrMatchCaseC(swversion, ensSoftwareVersion))
        {
            if(debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "species-specific database '%S'.\n",
                        database);

            dbag = ensDatabaseadaptorGroupFromStr(group);

            if(dbag)
                dba = ensDatabaseadaptorNewIni(dbc,
                                               database,
                                               prefix,
                                               dbag,
                                               ajFalse,
                                               0);
            else
                ajDebug("ensDatabaseadaptorNewDatabasename got unexpected "
                        "group string '%S' for database name '%S'.\n",
                        group, database);
        }
        else
        {
            if(debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "species-specific database '%S', "
                        "but not the software version '%s'.\n",
                        database, ensSoftwareVersion);
        }

        ajStrDel(&prefix);
        ajStrDel(&group);
        ajStrDel(&swversion);
    }
    else
        ajDebug("ensDatabaseadaptorNewDatabasename no match for "
                "database name '%S'.\n",
                database);

    ajRegFree(&collectionre);
    ajRegFree(&multire);
    ajRegFree(&speciesre);

    ajStrDel(&multi);

    return dba;
}




/* @func ensDatabaseadaptorNewIni *********************************************
**
** Constructor for an Ensembl Database Adaptor with initial values.
**
** Ensembl Database Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Database Adaptor connected to a particular database
** is sufficient to instantiate any number of Ensembl Object Adaptors from the
** database. Each Ensembl Object Adaptor will have a weak reference to the
** Ensembl Database Adaptor that instantiated it. Therefore, Ensembl Database
** Adaptors should not be instantiated directly, but rather obtained from the
** Ensembl Registry, which will in turn call this function if neccessary.
**
** @see ensRegistryNewDatabaseadaptor
** @see ensRegistryGetDatabaseadaptor
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::new
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] database [AjPStr] Database name (optional). If not provided, the
**                              database name in the Database Connection will
**                              be used.
** @param [u] species [AjPStr] Species
** @param [u] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
** NOTE: The Perl API automatically registers the Ensembl Database Adaptor in
** the Ensembl Registry via Bio::EnsEMBL::Utils::ConfigRegistry::gen_load.
** In this implementation both objects are kept separate. The Ensembl Registry
** object provides a function, which generates an Ensembl Database Adaptor
** based on the same parameter set and automatically registers it.
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseadaptorNewIni(
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr species,
    EnsEDatabaseadaptorGroup dbag,
    AjBool multi,
    ajuint identifier)
{
    EnsPDatabaseadaptor dba = NULL;

    if(ajDebugTest("ensDatabaseadaptorNewIni"))
    {
        ajDebug("ensDatabaseadaptorNewIni\n"
                "  dbc %p\n"
                "  database '%S'\n"
                "  species '%S'\n"
                "  dbag %d\n"
                "  multi %B\n"
                "  identifier %u\n",
                dbc,
                database,
                species,
                dbag,
                multi,
                identifier);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dbc)
        return NULL;

    if(!species)
        return NULL;

    if(!dbag)
        return NULL;

    AJNEW0(dba);

    dba->Databaseconnection = ensDatabaseconnectionNewCpy(dbc, database);

    if(species)
        dba->Species = ajStrNewRef(species);

    dba->Group = dbag;

    dba->Multispecies = multi;

    dba->Identifier = identifier;

    return dba;
}




/* @func ensDatabaseadaptorNewUrl *********************************************
**
** Constructor for an Ensembl Database Adaptor with a
** Uniform Resource Locator (URL).
**
** Ensembl Database Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Database Adaptor connected to a particular database
** is sufficient to instantiate any number of Ensembl Object Adaptors from the
** database. Each Ensembl Object Adaptor will have a weak reference to the
** Ensembl Database Adaptor that instantiated it. Therefore, Ensembl Database
** Adaptors should not be instantiated directly, but rather obtained from the
** Ensembl Registry, which will in turn call this function if neccessary.
**
** @see ensRegistryNewDatabaseadaptor
** @see ensRegistryGetDatabaseadaptor
**
** @param [r] url [const AjPStr] Uniform Resource Locator
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseadaptorNewUrl(const AjPStr url)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseconnection dbc = NULL;

    if((url == NULL) || (ajStrGetLen(url) == 0))
        return NULL;

    dbc = ensDatabaseconnectionNewUrl(url);

    if(!dbc)
        return NULL;

    dba = ensDatabaseadaptorNewDatabasename(
        dbc,
        ensDatabaseconnectionGetDatabasename(dbc));

    ensDatabaseconnectionDel(&dbc);

    return dba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Database Adaptor object
**
** @argrule * Pdba [EnsPDatabaseadaptor*] Ensembl Database Adaptor
**                                        object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseadaptorDel ************************************************
**
** Default destructor for an Ensembl Database Adaptor.
**
** Ensembl Database Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Object Adaptors that have
** been instantiated by it. Therefore, Ensembl Database Adaptors should never
** be destroyed directly. Upon exit, the Ensembl Registry will call this
** function if required.
**
** @see ensExit
** @see ensRegistryExit
**
** @param [d] Pdba [EnsPDatabaseadaptor*] Ensembl Database Adaptor
**                                        object address
**
** @return [void]
** @@
** NOTE: The Perl API removes the Ensembl Database Adaptor from the
** Ensembl Registry via Bio::EnsEMBL::Registry::remove_db.
** This implementation keeps both objects independent and Ensembl Database
** Adaptors are deleted from the Ensembl Registry.
******************************************************************************/

void ensDatabaseadaptorDel(EnsPDatabaseadaptor* Pdba)
{
    EnsPDatabaseadaptor pthis = NULL;

    if(!Pdba)
        return;

    if(!*Pdba)
        return;

    if(ajDebugTest("ensDatabaseadaptorDel"))
    {
        ajDebug("ensDatabaseadaptorDel\n"
                "  *Pdba %p\n",
                *Pdba);

        ensDatabaseadaptorTrace(*Pdba, 1);
    }

    pthis = *Pdba;

    ensDatabaseconnectionDel(&pthis->Databaseconnection);

    ajStrDel(&pthis->Species);

    ajListstrFreeData(&pthis->SpeciesNames);

    AJFREE(pthis);

    *Pdba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Get Return Ensembl Database Adaptor attribute(s)
** @nam4rule Databaseconnection Return the Ensembl Database Connection
** @nam4rule Group Return the Ensembl Database Adaptor Group enumeration
** @nam4rule Identifier Return the species identifier
** @nam4rule Multispecies Return the multi-species flag
** @nam4rule Species Return the species
**
** @argrule * dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule Databaseconnection [EnsPDatabaseconnection]
** Ensembl Database Connection or NULL
** @valrule Species [AjPStr] Species or NULL
** @valrule Group [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration or ensEDatabaseadaptorGroupNULL
** @valrule Multispecies [AjBool] Multiple species or ajFalse
** @valrule Identifier [ajuint] Species identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseadaptorGetDatabaseconnection ******************************
**
** Get the Ensembl Database Connection element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::dbc
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseadaptorGetDatabaseconnection(
    const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return dba->Databaseconnection;
}




/* @func ensDatabaseadaptorGetGroup *******************************************
**
** Get the group element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::group
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration or ensEDatabaseadaptorGroupNULL
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensDatabaseadaptorGetGroup(
    const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return ensEDatabaseadaptorGroupNULL;

    return dba->Group;
}




/* @func ensDatabaseadaptorGetIdentifier **************************************
**
** Get the species identifier element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species_id
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [ajuint] Species identifier, defaults to 1
** @@
******************************************************************************/

ajuint ensDatabaseadaptorGetIdentifier(const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return 0;

    if(!dba->Identifier)
        return 1;

    return dba->Identifier;
}




/* @func ensDatabaseadaptorGetMultispecies ************************************
**
** Get the multi-species element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::is_multispecies
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue if the database contains multiple species
** @@
******************************************************************************/

AjBool ensDatabaseadaptorGetMultispecies(const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return ajFalse;

    return dba->Multispecies;
}




/* @func ensDatabaseadaptorGetSpecies *****************************************
**
** Get the species element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjPStr] Species or NULL
** @@
******************************************************************************/

AjPStr ensDatabaseadaptorGetSpecies(const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return dba->Species;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Set Set one element of an Ensembl Database Adaptor
** @nam4rule Databaseconnection Set the Ensembl Database Connection
** @nam4rule Group Set the group
** @nam4rule Identifier Set the species identifier
** @nam4rule Multispecies Set the multiple-species flag
** @nam4rule Species Set the species
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor object
** @argrule Databaseconnection dbc [EnsPDatabaseconnection] Ensembl Database
** Connection
** @argrule Group dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor
** Group enumeration
** @argrule Identifier identifier [ajuint] Species identifier
** @argrule Multispecies multi [AjBool] Multi-species attribute
** @argrule Species species [AjPStr] Species
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensDatabaseadaptorSetDatabaseconnection ******************************
**
** Set the Ensembl Database Connection element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::dbc
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetDatabaseconnection(EnsPDatabaseadaptor dba,
                                               EnsPDatabaseconnection dbc)
{
    if(!dba)
        return ajFalse;

    ensDatabaseconnectionDel(&dba->Databaseconnection);

    dba->Databaseconnection = ensDatabaseconnectionNewRef(dbc);

    return ajTrue;
}




/* @func ensDatabaseadaptorSetGroup *******************************************
**
** Set the group element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::group
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetGroup(EnsPDatabaseadaptor dba,
                                  EnsEDatabaseadaptorGroup dbag)
{
    if(!dba)
        return ajFalse;

    dba->Group = dbag;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetIdentifier **************************************
**
** Set the species identifier element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species_id
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] identifier [ajuint] Species identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetIdentifier(EnsPDatabaseadaptor dba,
                                       ajuint identifier)
{
    if(!dba)
        return ajFalse;

    dba->Identifier = identifier;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetMultispecies ************************************
**
** Set the multi-species element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::is_multispecies
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] multi [AjBool] Multi-species attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetMultispecies(EnsPDatabaseadaptor dba,
                                         AjBool multi)
{
    if(!dba)
        return ajFalse;

    dba->Multispecies = multi;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetSpecies *****************************************
**
** Set the species element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] species [AjPStr] Species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetSpecies(EnsPDatabaseadaptor dba, AjPStr species)
{
    if(!dba)
        return ajFalse;

    ajStrDel(&dba->Species);

    if(species)
        dba->Species = ajStrNewRef(species);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Trace Report Ensembl Database Adaptor elements to debug file
**
** @argrule Trace dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseadaptorTrace **********************************************
**
** Trace an Ensembl Database Adaptor.
**
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorTrace(const EnsPDatabaseadaptor dba, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr name = NULL;

    if(!dba)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensDatabaseadaptorTrace %p\n"
            "%S  Databaseconnection %p\n"
            "%S  Species '%S'\n"
            "%S  SpeciesNames %p\n"
            "%S  Group '%s'\n"
            "%S  Multispecies %B\n"
            "%S  Identifier %u\n",
            indent, dba,
            indent, dba->Databaseconnection,
            indent, dba->Species,
            indent, dba->SpeciesNames,
            indent, ensDatabaseadaptorGroupToChar(dba->Group),
            indent, dba->Multispecies,
            indent, dba->Identifier);

    ensDatabaseconnectionTrace(dba->Databaseconnection, level + 1);

    /* Trace the AJAX List of AJAX String (species name) objects. */

    if(dba->SpeciesNames)
    {
        ajDebug("%S    AJAX List %p of AJAX String (species name) objects:\n",
                indent, dba->SpeciesNames);

        iter = ajListIterNewread(dba->SpeciesNames);

        while(!ajListIterDone(iter))
        {
            name = (AjPStr) ajListIterGet(iter);

            ajDebug("%S      %S\n", indent, name);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Database Adaptor objects.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Match Compare two Ensembl Database Adaptor objects
** @nam3rule Matchcomponents Match components of an Ensembl Database Adaptor
**
** @argrule Match dba1 [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Match dba2 [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Matchcomponents dba [const EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @argrule Matchcomponents dbc [const EnsPDatabaseconnection]
** Ensembl Database Connection
** @argrule Matchcomponents species [const AjPStr] Species
** @argrule Matchcomponents dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule Matchcomponents multi [AjBool] Multiple species
** @argrule Matchcomponents identifier [ajuint] Species identifier
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseadaptorMatch **********************************************
**
** Tests for matching two Ensembl Database Adaptors.
**
** @param [r] dba1 [const EnsPDatabaseadaptor] First Ensembl Database Adaptor
** @param [r] dba2 [const EnsPDatabaseadaptor] Second Ensembl Database Adaptor
**
** @return [AjBool] ajTrue if the Ensembl Database Adaptors are equal
** @@
** The comparison is based on initial pointer equality and if that fails a
** comparison of the group element, the species identifier and a
** case-insensitive string comparison of the species element.
******************************************************************************/

AjBool ensDatabaseadaptorMatch(const EnsPDatabaseadaptor dba1,
                               const EnsPDatabaseadaptor dba2)
{
    if(!dba1)
        return ajFalse;

    if(!dba2)
        return ajFalse;

    if(dba1 == dba2)
        return ajTrue;

    if(!ensDatabaseconnectionMatch(dba1->Databaseconnection,
                                   dba2->Databaseconnection))
        return ajFalse;

    if(dba1->Group != dba2->Group)
        return ajFalse;

    if(dba1->Multispecies != dba2->Multispecies)
        return ajFalse;

    if(dba1->Identifier != dba2->Identifier)
        return ajFalse;

    if(!ajStrMatchCaseS(dba1->Species, dba2->Species))
        return ajFalse;

    return ajTrue;
}




/* @func ensDatabaseadaptorMatchcomponents ************************************
**
** Tests whether an Ensembl Database Adaptor matches component elements.
**
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] species [const AjPStr] Species
** @param [u] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [AjBool] ajTrue if all elements match
** @@
******************************************************************************/

AjBool ensDatabaseadaptorMatchcomponents(const EnsPDatabaseadaptor dba,
                                         const EnsPDatabaseconnection dbc,
                                         const AjPStr species,
                                         EnsEDatabaseadaptorGroup dbag,
                                         AjBool multi,
                                         ajuint identifier)
{
    if(!dba)
        return ajFalse;

    if(!dbc)
        return ajFalse;

    if(!species)
        return ajFalse;

    if(!dbag)
        return ajFalse;

    if(dba->Group != dbag)
        return ajFalse;

    if(dba->Multispecies != multi)
        return ajFalse;

    if(dba->Identifier != identifier)
        return ajFalse;

    if(!ensDatabaseconnectionMatch(dba->Databaseconnection, dbc))
        return ajFalse;

    if(!ajStrMatchCaseS(dba->Species, species))
        return ajFalse;

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Database Adaptor convenience functions
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Disconnect Disconnect the underlying Ensembl Database Connection
** @nam3rule Escape Escape a string
** @nam4rule C Escape to an AJAX String
** @nam4rule S Escape to a C-type character string
** @nam3rule Get Get members(s)
** @nam4rule All Get all members
** @nam5rule Speciesnames Get all species names
** @nam3rule Sqlstatement AJAX SQL Statement
** @nam4rule Del Delete an AJAX SQL Statement
** @nam4rule New Run a new AJAX SQL statement
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule EscapeC Ptxt [char**] Address of the (new) SQL-escaped C string
** @argrule EscapeC str [const AjPStr] AJAX String to be escaped
** @argrule EscapeS Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @argrule EscapeS str [const AjPStr] AJAX String to be escaped
** @argrule SqlstatementDel Psqls [AjPSqlstatement*] AJAX SQL Statement address
** @argrule SqlstatementNew statement [const AjPStr] SQL statement
**
** @valrule Disconnect [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule EscapeC [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule EscapeS [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Speciesnames [const AjPList] AJAX List of
** AJAX String (species name) objects
** @valrule SqlstatementDel [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule SqlstatementNew [AjPSqlstatement] AJAX SQL Statement
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseadaptorDisconnect *****************************************
**
** Disconnect the Ensembl Database Connection this Ensembl Database Adaptor
** is based on.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorDisconnect(EnsPDatabaseadaptor dba)
{
    if(!dba)
        return ajFalse;

    return ensDatabaseconnectionDisconnect(dba->Databaseconnection);
}




/* @func ensDatabaseadaptorEscapeC ********************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return a C-type character string.
**
** The caller is responsible for deleting the escaped C-type character string.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorEscapeC(EnsPDatabaseadaptor dba,
                                 char** Ptxt,
                                 const AjPStr str)
{
    if(!dba)
        return ajFalse;

    if(!Ptxt)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(ajDebugTest("ensDatabaseadaptorEscapeC"))
    {
        ajDebug("ensDatabaseadaptorEscapeC\n"
                "  dba %p\n"
                "  Ptxt %p\n"
                "  str '%S'\n",
                dba,
                Ptxt,
                str);

        ensDatabaseadaptorTrace(dba, 1);
    }

    return ensDatabaseconnectionEscapeC(dba->Databaseconnection, Ptxt, str);
}




/* @func ensDatabaseadaptorEscapeS ********************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return an AJAX String.
**
** The caller is responsible for deleting the escaped AJAX String.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorEscapeS(EnsPDatabaseadaptor dba,
                                 AjPStr* Pstr,
                                 const AjPStr str)
{
    if(!dba)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(ajDebugTest("ensDatabaseadaptorEscapeS"))
    {
        ajDebug("ensDatabaseadaptorEscapeS\n"
                "  dba %p\n"
                "  Pstr %p\n"
                "  str '%S'\n",
                dba,
                Pstr,
                str);

        ensDatabaseadaptorTrace(dba, 1);
    }

    return ensDatabaseconnectionEscapeS(dba->Databaseconnection, Pstr, str);
}




/* @func ensDatabaseadaptorGetAllSpeciesnames *********************************
**
** Get the name of all species contained in the database this
** Ensembl Database Adaptor is connected to.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [const AjPList] AJAX List of AJAX String objects or NULL
** @@
******************************************************************************/

const AjPList ensDatabaseadaptorGetAllSpeciesnames(EnsPDatabaseadaptor dba)
{
    EnsPMetainformationadaptor mia = NULL;

    if(!dba)
        return NULL;

    if(dba->SpeciesNames)
        return dba->SpeciesNames;
    else
        dba->SpeciesNames = ajListstrNew();

    mia = ensRegistryGetMetainformationadaptor(dba);

    if(dba->Multispecies)
        ensMetainformationadaptorRetrieveAllSpeciesnames(mia,
                                                         dba->SpeciesNames);
    else
        ajListPushAppend(dba->SpeciesNames, (void*) ajStrNewS(dba->Species));

    return dba->SpeciesNames;
}




/* @func ensDatabaseadaptorSqlstatementDel ************************************
**
** Delete an AJAX SQL Statement associated with an
** Ensembl Database Adaptor.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [d] Psqls [AjPSqlstatement*] AJAX SQL Statement address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSqlstatementDel(EnsPDatabaseadaptor dba,
                                         AjPSqlstatement* Psqls)
{
    if(!dba)
        return ajFalse;

    if(!Psqls)
        return ajFalse;

    if(ajDebugTest("ensDatabaseadaptorSqlstatementDel"))
        ajDebug("ensDatabaseadaptorSqlstatementDel\n"
                "  dba %p\n"
                "  PSqls %p\n",
                dba,
                Psqls);

    return ensDatabaseconnectionSqlstatementDel(dba->Databaseconnection,
                                                Psqls);
}




/* @func ensDatabaseadaptorSqlstatementNew ************************************
**
** Constructor for an AJAX SQL statement via an Ensembl Database Adaptor.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
**
** @return [AjPSqlstatement] AJAX SQL Statement or NULL
** @@
******************************************************************************/

AjPSqlstatement ensDatabaseadaptorSqlstatementNew(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement)
{
    if(!dba)
        return NULL;

    if(!statement)
        return NULL;

    return ensDatabaseconnectionSqlstatementNew(dba->Databaseconnection,
                                                statement);
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Fetch Fetch Ensembl Database Adaptor-related objects
** @nam4rule All Fetch all objects
** @nam4rule Schemabuild Fetch the schema build
**
** @argrule Schemabuild dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Schemabuild Pbuild [AjPStr*] Schema build
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseadaptorFetchSchemabuild ***********************************
**
** Fetch the schema build of an Ensembl database, which is the software version
** and the data version separated by an underscore.
** So for database homo_sapiens_core_50_36l the schema build would be 50_36l.
** The caller is responsible for deleting the AJAX String.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] Pbuild [AjPStr*] Schema build
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorFetchSchemabuild(EnsPDatabaseadaptor dba,
                                          AjPStr* Pbuild)
{
    ajuint i = 0;
    ajuint tokens = 0;

    AjPStr database = NULL;
    AjPStr temp = NULL;
    AjPStrTok token = NULL;

    if(!dba)
        return ajFalse;

    if(!Pbuild)
        return ajFalse;

    if(*Pbuild)
        ajStrAssignClear(Pbuild);
    else
        *Pbuild = ajStrNew();

    database = ensDatabaseconnectionGetDatabasename(dba->Databaseconnection);

    tokens = ajStrParseCountC(database, "_");

    if(tokens < 2)
        return ajFalse;

    token = ajStrTokenNewC(database, "_");

    /* Parse all string tokens but the last two. */

    temp = ajStrNew();

    for(i = 0; i < (tokens - 2); i++)
        ajStrTokenNextParse(&token, &temp);

    ajStrDel(&temp);

    ajStrTokenRestParse(&token, Pbuild);

    ajStrTokenDel(&token);

    return ajTrue;
}




/* @datasection [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group *****
**
** @nam2rule Databaseadaptor Functions for manipulating
** Ensembl Database Adaptor objects
** @nam3rule DatabaseadaptorGroup Functions for manipulating
** Ensembl Database Adaptor Group enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Database Adaptor Group enumeration.
**
** @fdata [EnsEDatabaseadaptorGroup]
**
** @nam4rule From Ensembl Database Adaptor Group query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  group  [const AjPStr] Group string
**
** @valrule * [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration or ensEDatabaseadaptorGroupNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseadaptorGroupFromStr ***************************************
**
** Convert an AJAX String into an Ensembl Database Adaptor Group enumeration.
**
** @param [r] group [const AjPStr] Group string
**
** @return [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration or ensEDatabaseadaptorGroupNULL
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensDatabaseadaptorGroupFromStr(const AjPStr group)
{
    register EnsEDatabaseadaptorGroup i = ensEDatabaseadaptorGroupNULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    for(i = ensEDatabaseadaptorGroupNULL;
        databaseadaptorGroup[i];
        i++)
        if(ajStrMatchCaseC(group, databaseadaptorGroup[i]))
            dbag = i;

    if(!dbag)
        ajDebug("ensDatabaseadaptorGroupFromStr encountered "
                "unexpected string '%S'.\n", group);

    return dbag;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Database Adaptor Group enumeration.
**
** @fdata [EnsEDatabaseadaptorGroup]
**
** @nam4rule To   Return Ensembl Database Adaptor Group enumeration
** @nam5rule Char Return C character string value
**
** @argrule To dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
**
** @valrule Char [const char*] Ensembl Database Adaptor Group C-type
** (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensDatabaseadaptorGroupToChar ****************************************
**
** Convert an Ensembl Database Adaptor Group enumeration into a
** C-type (char*) string.
**
** @param [u] dbag [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor Group
** enumeration
**
** @return [const char*] Ensembl Database Adaptor Group C-type (char*) string
** @@
******************************************************************************/

const char* ensDatabaseadaptorGroupToChar(EnsEDatabaseadaptorGroup dbag)
{
    register EnsEDatabaseadaptorGroup i = ensEDatabaseadaptorGroupNULL;

    for(i = ensEDatabaseadaptorGroupNULL;
        databaseadaptorGroup[i] && (i < dbag);
        i++);

    if(!databaseadaptorGroup[i])
        ajDebug("ensDatabaseadaptorGroupToChar encountered an "
                "out of boundary error on "
                "Ensembl Databaseadaptor Group enumeration %d.\n",
                dbag);

    return databaseadaptorGroup[i];
}

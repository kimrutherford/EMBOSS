/* @source ensdatabaseadaptor *************************************************
**
** Ensembl Database Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.42 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/26 06:37:32 $ by $Author: mks $
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

#include "ensdatabaseadaptor.h"
#include "ensmetainformation.h"
#include "enssoftware.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const ensKDatabaseadaptorRegCollection ************************************
**
** Regular expression to match Ensembl collection databases.
**
******************************************************************************/

const char *ensKDatabaseadaptorRegCollection =
    "^"                     /* Start of string assertion */
    "(\\w+)"                /* Genus name (1) */
    "_"                     /* Separator */
    "collection"            /* Prefix */
    "_"                     /* Separator */
    "([a-z]+)"              /* Ensembl Database group (2) */
    "(?:_\\d+)?"            /* Ensembl Genomes version */
    "_"                     /* Separator */
    "(\\d+)"                /* Ensembl Software version (3) */
    "_"                     /* Separator */
    "[0-9a-z]+"             /* Assembly suffix */
    "$";                    /* End of string assertion */




/* @const ensKDatabaseadaptorRegMulti *****************************************
**
** Regular expression to match Ensembl multi-species databases.
**
******************************************************************************/

const char *ensKDatabaseadaptorRegMulti =
    "^"                     /* Start of string assertion */
    "ensembl(?:genomes)?"   /* Prefix */
    "_"                     /* Separator */
    "([a-z]+)"              /* Ensembl Database group (1) */
    "(_\\w+?)*?"            /* Ensembl Genomes section (2) */
    "(?:_\\d+)?"            /* Ensembl Genomes version */
    "_"                     /* Separator */
    "(\\d+)"                /* Ensembl Software version (3) */
    "$";                    /* End of string assertion */




/* @const ensKDatabaseadaptorRegSpecies ***************************************
**
** Regular expression to match Ensembl species-specific databases.
**
******************************************************************************/

const char *ensKDatabaseadaptorRegSpecies =
    "^"                     /* Start of string assertion */
    "([a-z]+_[0-9a-z]+)"    /* Species name (1) */
    "_"                     /* Separator */
    "([a-z]+)"              /* Database group (2) */
    "(?:_\\d+)?"            /* Ensembl Genomes version */
    "_"                     /* Separator */
    "(\\d+)"                /* Ensembl Software version (3) */
    "_"                     /* Separator */
    "[0-9a-z]+"             /* Assembly suffix */
    "$";                    /* End of string assertion */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic databaseadaptorKGroup *****************************************
**
** The Ensembl Database Adaptor group member is enumerated in the data
** structure. The following strings are used for conversion in database
** operations and correspond to EnsEDatabaseadaptorGroup.
**
******************************************************************************/

static const char *databaseadaptorKGroup[] =
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
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




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
** @cc CVS Revision: 1.225
** @cc CVS Tag: branch-ensembl-66
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
** @argrule Ini dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseadaptorNewDatabasename(
    EnsPDatabaseconnection dbc,
    AjPStr database)
{
    AjBool debug = AJFALSE;

    AjPRegexp rec = NULL;
    AjPRegexp rem = NULL;
    AjPRegexp res = NULL;

    AjPStr group     = NULL;
    AjPStr prefix    = NULL;
    AjPStr swversion = NULL;
    AjPStr multi     = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    EnsPDatabaseadaptor dba = NULL;

    debug = ajDebugTest("ensDatabaseadaptorNewDatabasename");

    if (debug)
    {
        ajDebug("ensDatabaseadaptorNewDatabasename\n"
                "  dbc %p\n"
                "  database '%S'\n",
                dbc,
                database);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if (!dbc)
        return NULL;

    if ((database == NULL) || (ajStrGetLen(database) == 0))
        return NULL;

    rec = ajRegCompC(ensKDatabaseadaptorRegCollection);
    rem = ajRegCompC(ensKDatabaseadaptorRegMulti);
    res = ajRegCompC(ensKDatabaseadaptorRegSpecies);

    multi = ajStrNewC("default");

    if (ajRegExec(rec, database))
    {
        /* Ensembl Collection databases have to be matched first. */

        group     = ajStrNew();
        prefix    = ajStrNew();
        swversion = ajStrNew();

        ajRegSubI(rec, 1, &prefix);
        ajRegSubI(rec, 2, &group);
        ajRegSubI(rec, 3, &swversion);

        if (ajStrMatchCaseC(swversion, ensKSoftwareVersion))
        {
            if (debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "collection database '%S'.\n",
                        database);

            dbag = ensDatabaseadaptorGroupFromStr(group);

            if (dbag)
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
            if (debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "collection database '%S', "
                        "but not the software version '%s'.\n",
                        database, ensKSoftwareVersion);
        }

        ajStrDel(&group);
        ajStrDel(&prefix);
        ajStrDel(&swversion);
    }
    else if (ajRegExec(rem, database))
    {
        /* Multi-species databases */

        group     = ajStrNew();
        prefix    = ajStrNew();
        swversion = ajStrNew();

        ajRegSubI(rem, 1, &group);

        if (ajRegLenI(rem, 3))
        {
            ajRegSubI(rem, 2, &prefix);
            ajRegSubI(rem, 3, &swversion);

            /* Remove leading underscores from the species prefix. */

            if (ajStrPrefixC(prefix, "_"))
                ajStrCutStart(&prefix, 1);
        }
        else
            ajRegSubI(rem, 2, &swversion);

        if (ajStrMatchCaseC(swversion, ensKSoftwareVersion))
        {
            if (debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "multi-species database '%S'.\n",
                        database);

            dbag = ensDatabaseadaptorGroupFromStr(group);

            if (dbag)
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
            if (debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "multi-species database '%S', "
                        "but not the software version '%s'.\n",
                        database, ensKSoftwareVersion);
        }

        ajStrDel(&group);
        ajStrDel(&prefix);
        ajStrDel(&swversion);
    }
    else if (ajRegExec(res, database))
    {
        /* Species-specific databases */

        prefix    = ajStrNew();
        group     = ajStrNew();
        swversion = ajStrNew();

        ajRegSubI(res, 1, &prefix);
        ajRegSubI(res, 2, &group);
        ajRegSubI(res, 3, &swversion);

        if (ajStrMatchCaseC(swversion, ensKSoftwareVersion))
        {
            if (debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "species-specific database '%S'.\n",
                        database);

            dbag = ensDatabaseadaptorGroupFromStr(group);

            if (dbag)
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
            if (debug)
                ajDebug("ensDatabaseadaptorNewDatabasename matched "
                        "species-specific database '%S', "
                        "but not the software version '%s'.\n",
                        database, ensKSoftwareVersion);
        }

        ajStrDel(&prefix);
        ajStrDel(&group);
        ajStrDel(&swversion);
    }
    else
        ajDebug("ensDatabaseadaptorNewDatabasename no match for "
                "database name '%S'.\n",
                database);

    ajRegFree(&rec);
    ajRegFree(&rem);
    ajRegFree(&res);

    ajStrDel(&multi);

    return dba;
}




/* @func ensDatabaseadaptorNewIni *********************************************
**
** Constructor for an Ensembl Database Adaptor with initial values.
**
** If a database name has not been provided, the database name of the
** Ensembl Database Connection will be used.
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
** @param [uN] database [AjPStr] Database name
** @param [u] species [AjPStr] Species
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
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

    if (ajDebugTest("ensDatabaseadaptorNewIni"))
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

    if (!dbc)
        return NULL;

    if (!species)
        return NULL;

    if (!dbag)
        return NULL;

    AJNEW0(dba);

    dba->Databaseconnection = ensDatabaseconnectionNewCpy(dbc, database);

    if (species)
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDatabaseadaptorNewUrl(const AjPStr url)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseconnection dbc = NULL;

    if ((url == NULL) || (ajStrGetLen(url) == 0))
        return NULL;

    dbc = ensDatabaseconnectionNewUrl(url);

    if (!dbc)
        return NULL;

    dba = ensDatabaseadaptorNewDatabasename(
        dbc,
        ensDatabaseconnectionGetDatabasename(dbc));

    ensDatabaseconnectionDel(&dbc);

    return dba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Database Adaptor
**
** @argrule * Pdba [EnsPDatabaseadaptor*]
** Ensembl Database Adaptor address
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
** @param [d] Pdba [EnsPDatabaseadaptor*] Ensembl Database Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
** NOTE: The Perl API removes the Ensembl Database Adaptor from the
** Ensembl Registry via Bio::EnsEMBL::Registry::remove_db.
** This implementation keeps both objects independent and Ensembl Database
** Adaptors are deleted from the Ensembl Registry.
******************************************************************************/

void ensDatabaseadaptorDel(EnsPDatabaseadaptor *Pdba)
{
    EnsPDatabaseadaptor pthis = NULL;

    if (!Pdba)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensDatabaseadaptorDel"))
    {
        ajDebug("ensDatabaseadaptorDel\n"
                "  *Pdba %p\n",
                *Pdba);

        ensDatabaseadaptorTrace(*Pdba, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pdba)
        return;

    pthis = *Pdba;

    ensDatabaseconnectionDel(&pthis->Databaseconnection);

    ajStrDel(&pthis->Species);

    ajListstrFreeData(&pthis->SpeciesNames);

    AJFREE(pthis);

    *Pdba = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Database Adaptor object.
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
** @valrule Group [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration or ensEDatabaseadaptorGroupNULL
** @valrule Multispecies [AjBool] Multiple species or ajFalse
** @valrule Identifier [ajuint] Species identifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensDatabaseadaptorGetDatabaseconnection ******************************
**
** Get the Ensembl Database Connection member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::dbc
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDatabaseconnection] Ensembl Database Connection or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseconnection ensDatabaseadaptorGetDatabaseconnection(
    const EnsPDatabaseadaptor dba)
{
    return (dba) ? dba->Databaseconnection : NULL;
}




/* @func ensDatabaseadaptorGetGroup *******************************************
**
** Get the group member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::group
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration or ensEDatabaseadaptorGroupNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensDatabaseadaptorGetGroup(
    const EnsPDatabaseadaptor dba)
{
    return (dba) ? dba->Group : ensEDatabaseadaptorGroupNULL;
}




/* @func ensDatabaseadaptorGetIdentifier **************************************
**
** Get the species identifier member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species_id
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [ajuint] Species identifier or 0U, defaults to 1U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensDatabaseadaptorGetIdentifier(const EnsPDatabaseadaptor dba)
{
    if (!dba)
        return 0U;

    if (!dba->Identifier)
        return 1U;

    return dba->Identifier;
}




/* @func ensDatabaseadaptorGetMultispecies ************************************
**
** Get the multi-species member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::is_multispecies
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue if the database contains multiple species
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorGetMultispecies(const EnsPDatabaseadaptor dba)
{
    return (dba) ? dba->Multispecies : ajFalse;
}




/* @func ensDatabaseadaptorGetSpecies *****************************************
**
** Get the species member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjPStr] Species or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensDatabaseadaptorGetSpecies(const EnsPDatabaseadaptor dba)
{
    return (dba) ? dba->Species : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Set Set one member of an Ensembl Database Adaptor
** @nam4rule Databaseconnection Set the Ensembl Database Connection
** @nam4rule Group Set the group
** @nam4rule Identifier Set the species identifier
** @nam4rule Multispecies Set the multiple-species flag
** @nam4rule Species Set the species
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor object
** @argrule Databaseconnection dbc [EnsPDatabaseconnection]
** Ensembl Database Connection
** @argrule Group dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
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
** Set the Ensembl Database Connection member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::dbc
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] dbc [EnsPDatabaseconnection] Ensembl Database Connection
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetDatabaseconnection(EnsPDatabaseadaptor dba,
                                               EnsPDatabaseconnection dbc)
{
    if (!dba)
        return ajFalse;

    ensDatabaseconnectionDel(&dba->Databaseconnection);

    dba->Databaseconnection = ensDatabaseconnectionNewRef(dbc);

    return ajTrue;
}




/* @func ensDatabaseadaptorSetGroup *******************************************
**
** Set the group member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::group
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetGroup(EnsPDatabaseadaptor dba,
                                  EnsEDatabaseadaptorGroup dbag)
{
    if (!dba)
        return ajFalse;

    dba->Group = dbag;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetIdentifier **************************************
**
** Set the species identifier member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species_id
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] identifier [ajuint] Species identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetIdentifier(EnsPDatabaseadaptor dba,
                                       ajuint identifier)
{
    if (!dba)
        return ajFalse;

    dba->Identifier = identifier;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetMultispecies ************************************
**
** Set the multi-species member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::is_multispecies
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] multi [AjBool] Multi-species attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetMultispecies(EnsPDatabaseadaptor dba,
                                         AjBool multi)
{
    if (!dba)
        return ajFalse;

    dba->Multispecies = multi;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetSpecies *****************************************
**
** Set the species member of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::species
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] species [AjPStr] Species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetSpecies(EnsPDatabaseadaptor dba, AjPStr species)
{
    if (!dba)
        return ajFalse;

    ajStrDel(&dba->Species);

    if (species)
        dba->Species = ajStrNewRef(species);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
**
** @nam3rule Trace Report Ensembl Database Adaptor members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorTrace(const EnsPDatabaseadaptor dba, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr name = NULL;

    if (!dba)
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

    if (dba->SpeciesNames)
    {
        ajDebug("%S    AJAX List %p of AJAX String (species name) objects:\n",
                indent, dba->SpeciesNames);

        iter = ajListIterNewread(dba->SpeciesNames);

        while (!ajListIterDone(iter))
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
**
** @release 6.2.0
** @@
** The comparison is based on initial pointer equality and if that fails a
** comparison of the group member, the species identifier and a
** case-insensitive string comparison of the species member.
******************************************************************************/

AjBool ensDatabaseadaptorMatch(const EnsPDatabaseadaptor dba1,
                               const EnsPDatabaseadaptor dba2)
{
    if (!dba1)
        return ajFalse;

    if (!dba2)
        return ajFalse;

    if (dba1 == dba2)
        return ajTrue;

    if (!ensDatabaseconnectionMatch(dba1->Databaseconnection,
                                    dba2->Databaseconnection))
        return ajFalse;

    if (dba1->Group != dba2->Group)
        return ajFalse;

    if (dba1->Multispecies != dba2->Multispecies)
        return ajFalse;

    if (dba1->Identifier != dba2->Identifier)
        return ajFalse;

    if (!ajStrMatchCaseS(dba1->Species, dba2->Species))
        return ajFalse;

    return ajTrue;
}




/* @func ensDatabaseadaptorMatchcomponents ************************************
**
** Tests whether an Ensembl Database Adaptor matches component members.
**
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] species [const AjPStr] Species
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [AjBool] ajTrue if all members match
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorMatchcomponents(const EnsPDatabaseadaptor dba,
                                         const EnsPDatabaseconnection dbc,
                                         const AjPStr species,
                                         EnsEDatabaseadaptorGroup dbag,
                                         AjBool multi,
                                         ajuint identifier)
{
    if (!dba)
        return ajFalse;

    if (!dbc)
        return ajFalse;

    if (!species)
        return ajFalse;

    if (!dbag)
        return ajFalse;

    if (dba->Group != dbag)
        return ajFalse;

    if (dba->Multispecies != multi)
        return ajFalse;

    if (dba->Identifier != identifier)
        return ajFalse;

    if (!ensDatabaseconnectionMatch(dba->Databaseconnection, dbc))
        return ajFalse;

    if (!ajStrMatchCaseS(dba->Species, species))
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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorDisconnect(EnsPDatabaseadaptor dba)
{
    if (!dba)
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorEscapeC(EnsPDatabaseadaptor dba,
                                 char **Ptxt,
                                 const AjPStr str)
{
    if (!dba)
        return ajFalse;

    if (!Ptxt)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensDatabaseadaptorEscapeC"))
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorEscapeS(EnsPDatabaseadaptor dba,
                                 AjPStr *Pstr,
                                 const AjPStr str)
{
    if (!dba)
        return ajFalse;

    if (!Pstr)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensDatabaseadaptorEscapeS"))
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
**
** @release 6.4.0
** FIXME: This should become ensDatabaseadaptorLoadAllSpeciesnames
** @@
******************************************************************************/

const AjPList ensDatabaseadaptorGetAllSpeciesnames(EnsPDatabaseadaptor dba)
{
    EnsPMetainformationadaptor mia = NULL;

    if (!dba)
        return NULL;

    if (dba->SpeciesNames)
        return dba->SpeciesNames;
    else
        dba->SpeciesNames = ajListstrNew();

    mia = ensRegistryGetMetainformationadaptor(dba);

    if (dba->Multispecies)
        ensMetainformationadaptorRetrieveAllSpeciesnames(mia,
                                                         dba->SpeciesNames);
    else
        ajListPushAppend(dba->SpeciesNames, (void *) ajStrNewS(dba->Species));

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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSqlstatementDel(EnsPDatabaseadaptor dba,
                                         AjPSqlstatement *Psqls)
{
    if (!dba)
        return ajFalse;

    if (!Psqls)
        return ajFalse;

    if (ajDebugTest("ensDatabaseadaptorSqlstatementDel"))
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjPSqlstatement ensDatabaseadaptorSqlstatementNew(EnsPDatabaseadaptor dba,
                                                  const AjPStr statement)
{
    if (!dba)
        return NULL;

    if (!statement)
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDatabaseadaptorFetchSchemabuild(EnsPDatabaseadaptor dba,
                                          AjPStr *Pbuild)
{
    register ajuint i = 0U;
    ajuint tokens = 0U;

    AjPStr database = NULL;
    AjPStr temp = NULL;
    AjPStrTok token = NULL;

    if (!dba)
        return ajFalse;

    if (!Pbuild)
        return ajFalse;

    if (*Pbuild)
        ajStrAssignClear(Pbuild);
    else
        *Pbuild = ajStrNew();

    database = ensDatabaseconnectionGetDatabasename(dba->Databaseconnection);

    tokens = ajStrParseCountC(database, "_");

    if (tokens < 2U)
        return ajFalse;

    token = ajStrTokenNewC(database, "_");

    /* Parse all string tokens but the last two. */

    temp = ajStrNew();

    for (i = 0U; i < (tokens - 2U); i++)
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
** @valrule * [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration or ensEDatabaseadaptorGroupNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensDatabaseadaptorGroupFromStr ***************************************
**
** Convert an AJAX String into an Ensembl Database Adaptor Group enumeration.
**
** @param [r] group [const AjPStr] Group string
**
** @return [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration or ensEDatabaseadaptorGroupNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensDatabaseadaptorGroupFromStr(const AjPStr group)
{
    register EnsEDatabaseadaptorGroup i = ensEDatabaseadaptorGroupNULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;

    for (i = ensEDatabaseadaptorGroupNULL;
         databaseadaptorKGroup[i];
         i++)
        if (ajStrMatchCaseC(group, databaseadaptorKGroup[i]))
            dbag = i;

    if (!dbag)
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
** @argrule To dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
**
** @valrule Char [const char*]
** Ensembl Database Adaptor Group C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensDatabaseadaptorGroupToChar ****************************************
**
** Convert an Ensembl Database Adaptor Group enumeration into a
** C-type (char *) string.
**
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
**
** @return [const char*] Ensembl Database Adaptor Group C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensDatabaseadaptorGroupToChar(EnsEDatabaseadaptorGroup dbag)
{
    register EnsEDatabaseadaptorGroup i = ensEDatabaseadaptorGroupNULL;

    for (i = ensEDatabaseadaptorGroupNULL;
         databaseadaptorKGroup[i] && (i < dbag);
         i++);

    if (!databaseadaptorKGroup[i])
        ajDebug("ensDatabaseadaptorGroupToChar encountered an "
                "out of boundary error on "
                "Ensembl Databaseadaptor Group enumeration %d.\n",
                dbag);

    return databaseadaptorKGroup[i];
}

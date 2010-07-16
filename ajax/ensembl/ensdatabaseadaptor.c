/******************************************************************************
** @source Ensembl Database Adaptor functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.12 $
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




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

static const char *databaseadaptorGroup[] =
{
    NULL,
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
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensdatabaseadaptor ********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPDatabaseadaptor] Database Adaptor ************************
**
** Functions for manipulating Ensembl Database Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor CVS Revision: 1.208
**
** @nam2rule Databaseadaptor
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
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @argrule New database [AjPStr] Database name
** @argrule New species [AjPStr] Species
** @argrule New group [EnsEDatabaseadaptorGroup] Group
** @argrule New multi [AjBool] Multiple species
** @argrule New identifier [ajuint] Species identifier
**
** @valrule * [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensDatabaseadaptorNew ************************************************
**
** Default Ensembl Database Adaptor constructor.
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
** @param [r] dbc [EnsPDatabaseconnection] Ensembl Database Connection
** @param [u] database [AjPStr] Database name (optional). If not provided, the
**                              database name in the Database Connection will
**                              be used.
** @param [u] species [AjPStr] Species
** @param [r] group [EnsEDatabaseadaptorGroup] Group
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

EnsPDatabaseadaptor ensDatabaseadaptorNew(
    EnsPDatabaseconnection dbc,
    AjPStr database,
    AjPStr species,
    EnsEDatabaseadaptorGroup group,
    AjBool multi,
    ajuint identifier)
{
    EnsPDatabaseadaptor dba = NULL;

    if(ajDebugTest("ensDatabaseadaptorNew"))
    {
        ajDebug("ensDatabaseadaptorNew\n"
                "  dbc %p\n"
                "  database '%S'\n"
                "  species '%S'\n"
                "  group %d\n"
                "  multi %B\n"
                "  identifier %u\n",
                dbc,
                database,
                species,
                group,
                multi,
                identifier);

        ensDatabaseconnectionTrace(dbc, 1);
    }

    if(!dbc)
        return NULL;

    if(!species)
        return NULL;

    if(!group)
        return NULL;

    AJNEW0(dba);

    dba->Databaseconnection = ensDatabaseconnectionNewC(dbc, database);

    if(species)
        dba->Species = ajStrNewRef(species);

    dba->Group = group;

    dba->MultiSpecies = multi;

    dba->Identifier = identifier;

    return dba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Database Adaptor.
**
** @fdata [EnsPDatabaseadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Database Adaptor object
**
** @argrule * Pdba [EnsPDatabaseadaptor*] Ensembl Database Adaptor object
**                                        address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDatabaseadaptorDel ************************************************
**
** Default Ensembl Database Adaptor destructor.
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
** @fnote None
**
** @nam3rule Get Return Database Adaptor attribute(s)
** @nam4rule GetDatabaseconnection Return the Ensembl Database Connection
** @nam4rule GetSpecies Return the species
** @nam4rule GetGroup Return the group
** @nam4rule GetMultiSpecies Return the multi-species flag
** @nam4rule GetIdentifier Return the species identifier
**
** @argrule * dba [const EnsPDatabaseadaptor] Database Adaptor
**
** @valrule Databaseconnection [EnsPDatabaseconnection] Ensembl Database
**                                                      Connection
** @valrule Species [AjPStr] Species
** @valrule Group [EnsEDatabaseadaptorGroup] Group
** @valrule MultiSpecies [AjBool] Multiple species
** @valrule Identifier [ajuint] Species identifier
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




/* @func ensDatabaseadaptorGetGroup *******************************************
**
** Get the group element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::group
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsEDatabaseadaptorGroup] Group or ensEDatabaseadaptorGroupNULL
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensDatabaseadaptorGetGroup(
    const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return ensEDatabaseadaptorGroupNULL;

    return dba->Group;
}




/* @func ensDatabaseadaptorGetMultiSpecies ************************************
**
** Get the multi-species element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::is_multispecies
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue if the database contains multiple species
** @@
******************************************************************************/

AjBool ensDatabaseadaptorGetMultiSpecies(const EnsPDatabaseadaptor dba)
{
    if(!dba)
        return ajFalse;

    return dba->MultiSpecies;
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




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Database Adaptor
** @nam4rule SetDatabaseconnection Set the Ensembl Database Connection
** @nam4rule SetSpecies Set the species
** @nam4rule SetGroup Set the group
** @nam4rule SetMultiSpecies Set the multiple-species flag
** @nam4rule SetIdentifier Set the species identifier
**
** @argrule * dba [EnsPDatabaseadaptor] Ensembl Database Adaptor object
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




/* @func ensDatabaseadaptorSetGroup *******************************************
**
** Set the group element of an Ensembl Database Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::DBAdaptor::group
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] group [EnsEDatabaseadaptorGroup] Group
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorSetGroup(EnsPDatabaseadaptor dba,
                                  EnsEDatabaseadaptorGroup group)
{
    if(!dba)
        return ajFalse;

    dba->Group = group;

    return ajTrue;
}




/* @func ensDatabaseadaptorSetMultiSpecies ************************************
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

AjBool ensDatabaseadaptorSetMultiSpecies(EnsPDatabaseadaptor dba,
                                         AjBool multi)
{
    if(!dba)
        return ajFalse;

    dba->MultiSpecies = multi;

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




/* @func ensDatabaseadaptorGroupFromStr ***************************************
**
** Convert an AJAX String into an Ensembl Database Adaptor group element.
**
** @param [r] group [const AjPStr] Group string
**
** @return [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor group element or
**                                    ensEDatabaseadaptorGroupNULL
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensDatabaseadaptorGroupFromStr(const AjPStr group)
{
    register EnsEDatabaseadaptorGroup i = ensEDatabaseadaptorGroupNULL;

    EnsEDatabaseadaptorGroup egroup = ensEDatabaseadaptorGroupNULL;

    for(i = ensEDatabaseadaptorGroupCore; databaseadaptorGroup[i]; i++)
        if(ajStrMatchCaseC(group, databaseadaptorGroup[i]))
            egroup = i;

    if(!egroup)
        ajDebug("ensDatabaseadaptorGroupFromStr encountered "
                "unexpected string '%S'.\n", group);

    return egroup;
}




/* @func ensDatabaseadaptorGroupToChar ****************************************
**
** Convert an Ensembl Database Adaptor group element into a
** C-type (char*) string.
**
** @param [r] group [EnsEDatabaseadaptorGroup] Database Adaptor group
**
** @return [const char*] Database Adaptor group C-type (char*) string
** @@
******************************************************************************/

const char* ensDatabaseadaptorGroupToChar(EnsEDatabaseadaptorGroup group)
{
    register EnsEDatabaseadaptorGroup i = ensEDatabaseadaptorGroupNULL;

    if(!group)
        return NULL;

    for(i = ensEDatabaseadaptorGroupCore;
        databaseadaptorGroup[i] && (i < group);
        i++);

    if(!databaseadaptorGroup[i])
        ajDebug("ensDatabaseadaptorGroupToChar encountered an "
                "out of boundary error on group %d.\n", group);

    return databaseadaptorGroup[i];
}




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

    if(dba1->MultiSpecies != dba2->MultiSpecies)
        return ajFalse;

    if(dba1->Identifier != dba2->Identifier)
        return ajFalse;

    if(!ajStrMatchCaseS(dba1->Species, dba2->Species))
        return ajFalse;

    return ajTrue;
}




/* @func ensDatabaseadaptorMatchComponents ************************************
**
** Tests whether an Ensembl Database Adaptor matches component elements.
**
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] dbc [const EnsPDatabaseconnection] Ensembl Database Connection
** @param [r] species [const AjPStr] Species
** @param [r] group [EnsEDatabaseadaptorGroup] Group
** @param [r] multi [AjBool] Multiple species
** @param [r] identifier [ajuint] Species identifier
**
** @return [AjBool] ajTrue if all elements match
** @@
******************************************************************************/

AjBool ensDatabaseadaptorMatchComponents(const EnsPDatabaseadaptor dba,
                                         const EnsPDatabaseconnection dbc,
                                         const AjPStr species,
                                         EnsEDatabaseadaptorGroup group,
                                         AjBool multi,
                                         ajuint identifier)
{
    if(!dba)
        return ajFalse;

    if(!dbc)
        return ajFalse;

    if(!species)
        return ajFalse;

    if(!group)
        return ajFalse;

    if(dba->Group != group)
        return ajFalse;

    if(dba->MultiSpecies != multi)
        return ajFalse;

    if(dba->Identifier != identifier)
        return ajFalse;

    if(!ensDatabaseconnectionMatch(dba->Databaseconnection, dbc))
        return ajFalse;

    if(!ajStrMatchCaseS(dba->Species, species))
        return ajFalse;

    return ajTrue;
}




/* @func ensDatabaseadaptorSqlstatementNew ************************************
**
** Run an SQL statement against an Ensembl Database Adaptor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
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
                                         AjPSqlstatement *Psqls)
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




/* @func ensDatabaseadaptorEscapeC *******************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the C-type character string.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorEscapeC(EnsPDatabaseadaptor dba,
                                 char **Ptxt,
                                 const AjPStr str)
{
    if(!dba)
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




/* @func ensDatabaseadaptorEscapeS *******************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the AJAX String.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorEscapeS(EnsPDatabaseadaptor dba,
                                 AjPStr *Pstr,
                                 const AjPStr str)
{
    if(!dba)
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




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Database Adaptor object.
**
** @fdata [EnsPDatabaseadaptor]
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
            "%S  MultiSpecies %B\n"
            "%S  Identifier %u\n",
            indent, dba,
            indent, dba->Databaseconnection,
            indent, dba->Species,
            indent, dba->SpeciesNames,
            indent, ensDatabaseadaptorGroupToChar(dba->Group),
            indent, dba->MultiSpecies,
            indent, dba->Identifier);

    ensDatabaseconnectionTrace(dba->Databaseconnection, level + 1);

    /* Trace the AJAX List of species name AJAX Strings. */

    if(dba->SpeciesNames)
    {
        ajDebug("%S    AJAX List %p of species name AJAX Strings\n",
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




/* @func ensDatabaseadaptorGetSchemaBuild *************************************
**
** Get the schema build of an Ensembl database, which is the software version
** and the data version separated by an underscore.
** So for database homo_sapiens_core_50_36l the schema build would be 50_36l.
**
** @param [r] dba [const EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] Pbuild [AjPStr*] Schema build
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDatabaseadaptorGetSchemaBuild(const EnsPDatabaseadaptor dba,
                                        AjPStr *Pbuild)
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

    ajStrAssignClear(Pbuild);

    database = ensDatabaseconnectionGetDatabaseName(dba->Databaseconnection);

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




/* @func ensDatabaseadaptorGetAllSpeciesNames *********************************
**
** Get the name of all species contained in the database this
** Ensembl Database Adaptor is connected to.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [const AjPList] AJAX List of AJAX Strings
** @@
******************************************************************************/

const AjPList ensDatabaseadaptorGetAllSpeciesNames(EnsPDatabaseadaptor dba)
{
    EnsPMetainformationadaptor mia = NULL;

    if(!dba)
        return NULL;

    if(dba->SpeciesNames)
        return dba->SpeciesNames;
    else
        dba->SpeciesNames = ajListstrNew();

    mia = ensRegistryGetMetainformationadaptor(dba);

    if(dba->MultiSpecies)
        ensMetainformationadaptorFetchAllSpeciesNames(mia, dba->SpeciesNames);
    else
        ajListPushAppend(dba->SpeciesNames, (void *) ajStrNewS(dba->Species));

    return dba->SpeciesNames;
}

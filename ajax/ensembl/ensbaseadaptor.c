/******************************************************************************
** @source Ensembl Base Adaptor functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.5 $
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

#include "ensbaseadaptor.h"




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

/* Default empty SQL SELECT LEFT_JOIN statement. */

static EnsOBaseadaptorLeftJoin baseAdaptorLeftJoin[] =
{
    {NULL, NULL}
};

/*
** Limit the number of identifiers in SQL queries to chunks of maximum size.
** Ensure that the MySQL max_allowed_packet is not exceeded, which defaults
** to 1 MB, by splitting large queries into smaller queries of at most 256 KB
** (32768 8-bit characters). Assuming a (generous) average identifier string
** length of 16, this means a maximum of 2048 identifiers in each statement.
*/

const ajuint baseAdaptorFetchAllByIdentifiersMax = 2048;



static AjBool baseAdaptorFetchAllByIdentifiers(const EnsPBaseadaptor ba,
                                               AjPStr identifiers,
                                               AjPList objects);




/* @filesection ensbaseadaptor ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPBaseadaptor] Base Adaptor ********************************
**
** Functions for manipulating Ensembl Base Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor CVS Revision: 1.48
**
** @nam2rule Baseadaptor Base adaptor object function
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Base Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Base Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPBaseadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPBaseadaptor] Ensembl Base Adaptor
** @argrule Ref object [EnsPBaseadaptor] Ensembl Base Adaptor
**
** @valrule * [EnsPBaseadaptor] Ensembl Base Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensBaseadaptorNew ****************************************************
**
** Default constructor for an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] Ptables [const char**] Array of char strings of table names
** @param [r] Pcolumns [const char**] Array of char strings of column names
** @param [r] left_join [EnsOBaseadaptorLeftJoin*] Address of an array of
**                                   Ensembl Base Adaptor Left Join Statements
** @param [r] condition [const char *] Default WHERE condition in a
**                                     SQL SELECT statement
** @param [r] final [const char *] Final condition in a
**                                 SQL SELECT statement
** @param [f] Fquery [AjBool function] Pointer to a function to handle the
**                              Ensembl Object-specific SQL statement
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensBaseadaptorNew(
    EnsPDatabaseadaptor dba,
    const char **Ptables,
    const char **Pcolumns,
    EnsOBaseadaptorLeftJoin *left_join,
    const char *condition,
    const char *final,
    AjBool Fquery(EnsPDatabaseadaptor dba,
                  const AjPStr statement,
                  EnsPAssemblymapper am,
                  EnsPSlice slice,
                  AjPList objects))
{
    EnsPBaseadaptor ba = NULL;

    if(!dba)
        return NULL;

    if(!Pcolumns)
        return NULL;

    if(!Ptables)
        return NULL;

    AJNEW0(ba);

    ba->Adaptor = dba;

    ba->Tables = Ptables;

    ba->Columns = Pcolumns;

    if(left_join)
        ba->LeftJoin = left_join;

    else
        ba->LeftJoin = baseAdaptorLeftJoin;

    ba->DefaultCondition = condition;

    ba->FinalCondition = final;

    ba->Query = Fquery;

    return ba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Base Adaptor.
**
** @fdata [EnsPBaseadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Base Adaptor object
**
** @argrule * Pba [EnsPBaseadaptor*] Base Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensBaseadaptorDel ****************************************************
**
** Default destructor for an Ensembl Base Adaptor.
**
** @param [d] Pba [EnsPBaseadaptor*] Ensembl Base Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensBaseadaptorDel(EnsPBaseadaptor *Pba)
{
    if(!Pba)
        return;

    if(!*Pba)
        return;

    AJFREE(*Pba);

    *Pba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
** @fnote None
**
** @nam3rule Get Return Base Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
** @nam4rule GetColumns Return the columns
** @nam4rule GetTables Return the tables
**
** @argrule * ba [const EnsPBaseadaptor] Base Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @valrule Columns [const char**] Columns
** @valrule Tables [const char**] Tables
**
** @fcategory use
******************************************************************************/




/* @func ensBaseadaptorGetDatabaseadaptor *************************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::db
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensBaseadaptorGetDatabaseadaptor(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Adaptor;
}




/* @func ensBaseadaptorGetTables **********************************************
**
** Get the tables element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_tables
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char**] Address of the table name character string array
**                         or NULL
** @@
******************************************************************************/

const char **ensBaseadaptorGetTables(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Tables;
}




/* @func ensBaseadaptorGetColumns *********************************************
**
** Get the columns element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_columns
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char**] Address of the column name character string array
**                         or NULL
** @@
******************************************************************************/

const char **ensBaseadaptorGetColumns(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Columns;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
** @fnote None
**
** @nam3rule Set Set one element of a Base Adaptor
** @nam4rule SetTables Set the tables
** @nam4rule SetColumns Set the columns
** @nam4rule SetDefaultCondition Set the default SQL condition
** @nam4rule SetFinalCondition Set the final SQL condition
**
** @argrule * ba [EnsPBaseadaptor] Base Adaptor
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensBaseadaptorSetTables **********************************************
**
** Set the tables element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_tables
** @param [r] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] Ptables [const char**] One dimensional array of table name
**                                    character strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetTables(EnsPBaseadaptor ba, const char ** Ptables)
{
    if(!ba)
        return ajFalse;

    if(!Ptables)
        return ajFalse;

    ba->Tables = Ptables;

    return ajTrue;
}




/* @func ensBaseadaptorSetColumns *********************************************
**
** Set the columns element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_columns
** @param [r] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] Pcolumns [const char**] One dimensional array of column name
**                                     character strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetColumns(EnsPBaseadaptor ba, const char ** Pcolumns)
{
    if(!ba)
        return ajFalse;

    if(!Pcolumns)
        return ajFalse;

    ba->Columns = Pcolumns;

    return ajTrue;
}




/* @func ensBaseadaptorSetDefaultCondition ************************************
**
** Set the default SQL condition element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_default_where_clause
** @param [r] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] condition [const char *] Default SQL condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetDefaultCondition(EnsPBaseadaptor ba,
                                         const char *condition)
{
    if(!ba)
        return ajFalse;

    ba->DefaultCondition = condition;

    return ajTrue;
}




/* @func ensBaseadaptorSetFinalCondition **************************************
**
** Set the final SQL condition element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_final_clause
** @param [r] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] final [const char *] Final SQL condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetFinalCondition(EnsPBaseadaptor ba, const char *final)
{
    if(!ba)
        return ajFalse;

    ba->FinalCondition = final;

    return ajTrue;
}




/* @func ensBaseadaptorGetPrimaryTable ****************************************
**
** Get the primary table name of an Ensembl Base Adaptor.
** The primary table name is the first one in the array.
**
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char *] Address of the primary table name character strings
**                        or NULL
** @@
******************************************************************************/

const char *ensBaseadaptorGetPrimaryTable(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Tables[0];
}




/* @func ensBaseadaptorGetMultiSpecies ****************************************
**
** Get the multiple-species element of the Ensembl Database Adaptor element
** of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::is_multispecies
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [AjBool] ajTrue if the database contains multiple species,
**                  ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorGetMultiSpecies(const EnsPBaseadaptor ba)
{
    if(!ba)
        return ajFalse;

    return ensDatabaseadaptorGetMultiSpecies(ba->Adaptor);
}




/* @func ensBaseadaptorGetSpeciesIdentifier ***********************************
**
** Get the species identifier element of the Ensembl Database Adaptor element
** of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::species_id
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [ajuint] Ensembl species identifier, defaults to 1
** @@
******************************************************************************/

ajuint ensBaseadaptorGetSpeciesIdentifier(const EnsPBaseadaptor ba)
{
    if(!ba)
        return 0;

    return ensDatabaseadaptorGetIdentifier(ba->Adaptor);
}




/* @func ensBaseadaptorEscapeC ***********************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the C-type character string.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorEscapeC(EnsPBaseadaptor ba, char **Ptxt, const AjPStr str)
{
    if(!ba)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(ajDebugTest("ensBaseadaptorEscapeC"))
        ajDebug("ensBaseadaptorEscapeC\n"
                "  ba %p\n"
                "  Ptxt %p\n"
                "  str '%S'\n",
                ba,
                Ptxt,
                str);

    return ensDatabaseadaptorEscapeC(ba->Adaptor, Ptxt, str);
}




/* @func ensBaseadaptorEscapeS ***********************************************
**
** Escape an AJAX String based on an AJAX SQL Connection.
** The caller is responsible for deleting the AJAX String.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorEscapeS(EnsPBaseadaptor ba, AjPStr *Pstr,
                             const AjPStr str)
{
    if(!ba)
        return ajFalse;

    if(!str)
        return ajFalse;

    if(ajDebugTest("ensBaseadaptorEscapeS"))
        ajDebug("ensBaseadaptorEscapeS\n"
                "  ba %p\n"
                "  Pstr %p\n"
                "  str '%S'\n",
                ba,
                Pstr,
                str);

    return ensDatabaseadaptorEscapeS(ba->Adaptor, Pstr, str);
}




/* @func ensBaseadaptorGenericFetch *******************************************
**
** Generic function to fetch Ensembl Objects via an Ensembl Base Adaptor.
**
** The caller is responsible for deleting the Ensembl Objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::generic_fetch
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] constraint [const AjPStr] Constraint in a SQL SELECT statement
** @param [r] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] objects [AjPList] AJAX List to Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorGenericFetch(const EnsPBaseadaptor ba,
                                  const AjPStr constraint,
                                  EnsPAssemblymapper am,
                                  EnsPSlice slice,
                                  AjPList objects)
{
    register ajint i = 0;
    register ajint j = 0;

    AjBool match = AJFALSE;

    AjPStr columns = NULL;
    AjPStr tables = NULL;
    AjPStr joins = NULL;
    AjPStr parentheses = NULL;
    AjPStr statement = NULL;

    if(ajDebugTest("ensBaseadaptorGenericFetch"))
        ajDebug("ensBaseadaptorGenericFetch\n"
                "  ba %p\n"
                "  constraint '%S'\n"
                "  am %p\n"
                "  slice %p\n"
                "  objects %p\n",
                ba,
                constraint,
                am,
                slice,
                objects);

    if(!ba)
        return ajFalse;

    if(!objects)
        return ajFalse;

    columns = ajStrNew();

    tables = ajStrNew();

    joins = ajStrNew();

    parentheses = ajStrNew();

    /* Build the column expression. */

    for(i = 0; ba->Columns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", ba->Columns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /*
    ** FIXME: The extra_default_where clause is missing, which joins
    ** features in multi-species databases to the seq_region and coord_system
    ** tables.
    ** The problem is to find out that the query is for a Feature sub class and
    ** not just for an Object fetched via the Base Adaptor like Anaylsis, which
    ** is not a Feature.
    **
    ** Maybe this functionality should just move into the Feature Adaptor?
    ** But then, how to propagate this into the Base Adaptor? The default where
    ** clause cannot be used because it is of type const char*. Since an
    ** Ensembl Base Adaptor is instantiated from an Ensembl Database Adaptor it
    ** would be clear at that point that this is a multi species database.
    ** The const char* default where clause could be used to allocate a
    ** Base Adaptor-internal AJAX String that contains the template of the
    ** default_where clause in the corresponding Object Adaptor and in case
    ** this is a multi-species database, could be extended by the
    ** sequence_region and coord_system table? Doable?
    **
    ** FIXME: Rename DefaultWhere into DefaultCondition throughout.
    **
    ** TODO: Report to core.
    **
    ** The Perl API also applies a hack for the
    ** Bio::EnsEMBL::DBSQL::UnmappedObjectAdaptor, which should not be a sub
    ** class of Bio::EnsEMBL::Feature in the first place.
    */

    /*
    ** Construct a left join statement if one was defined, and remove the
    ** left-joined tables from the table expression.
    */

    for(i = 0; ba->Tables[i]; i++)
    {
        match = ajFalse;

        if(ba->LeftJoin)
        {
            for(j = 0; ba->LeftJoin[j].Table; j++)
            {
                if(ajCharMatchC(ba->Tables[i], ba->LeftJoin[j].Table))
                {
                    ajStrAppendK(&parentheses, '(');

                    ajFmtPrintAppS(&joins,
                                   "LEFT JOIN %s ON %s) ",
                                   ba->LeftJoin[j].Table,
                                   ba->LeftJoin[j].Condition);

                    match = ajTrue;

                    break;
                }
            }
        }

        if(!match)
            ajFmtPrintAppS(&tables, "%s, ", ba->Tables[i]);
    }

    /* Remove last comma and space from the table expression. */

    ajStrCutEnd(&tables, 2);

    /* Build the SQL statement. */

    statement = ajStrNewC("SELECT");

    if(ba->StraightJoin)
        ajStrAppendC(&statement, " STRAIGHT_JOIN");

    ajFmtPrintAppS(&statement, " %S FROM %S(%S)",
                   columns, parentheses, tables);

    if(joins && ajStrGetLen(joins))
        ajFmtPrintAppS(&statement, " %S", joins);

    if(constraint && ajStrGetLen(constraint))
    {
        ajFmtPrintAppS(&statement, " WHERE %S", constraint);

        if(ba->DefaultCondition)
            ajFmtPrintAppS(&statement, " AND %s", ba->DefaultCondition);
    }

    else if(ba->DefaultCondition)
        ajFmtPrintAppS(&statement, " WHERE %s", ba->DefaultCondition);

    if(ba->FinalCondition)
        ajFmtPrintAppS(&statement, " %s", ba->FinalCondition);

    ba->Query(ba->Adaptor, statement, am, slice, objects);

    ajStrDel(&columns);

    ajStrDel(&tables);

    ajStrDel(&joins);

    ajStrDel(&parentheses);

    ajStrDel(&statement);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Objects from an Ensembl database.
**
** @fdata [EnsPBaseadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Object(s)
** @nam4rule FetchAll Retrieve all Ensembl Objects.
** @nam5rule FetchAllBy Retrieve all Ensembl Objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPBaseadaptor] Ensembl Base Adaptor
** @argrule FetchAll objects [AjPList] AJAX List of Ensembl Objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensBaseadaptorFetchByIdentifier **************************************
**
** Generic function to fetch an Ensembl Object by its SQL database-internal
** identifier via an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [void *] Ensembl Object or NULL
** @@
******************************************************************************/

void *ensBaseadaptorFetchByIdentifier(const EnsPBaseadaptor ba,
                                      ajuint identifier)
{
    void *Pobject = NULL;

    AjPList objects = NULL;

    AjPStr constraint = NULL;

    if(!ba)
        return NULL;

    if(!identifier)
        return NULL;

    if(!ba->Tables[0])
        return NULL;

    constraint = ajFmtStr("%s.%s_id = %u",
                          ba->Tables[0],
                          ba->Tables[0],
                          identifier);

    objects = ajListNew();

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               objects);

    if(ajListGetLength(objects) > 1)
        ajFatal("ensBaseadaptorFetchByIdentifier got more than one object "
                "for constraint '%S'.\n", constraint);

    ajListPop(objects, &Pobject);

    ajListFree(&objects);

    ajStrDel(&constraint);

    return Pobject;
}




/* @funcstatic baseAdaptorFetchAllByIdentifiers *******************************
**
** Helper function for the generic function to fetch Ensembl Objects by an
** AJAX List of SQL database-internal identifiers via an Ensembl Base Adaptor.
**
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] identifiers [AjPStr] Comma-separated list of SQL database-
**                                 internal identifiers used in an IN
**                                 comparison function in a SQL SELECT
**                                 statement
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool baseAdaptorFetchAllByIdentifiers(const EnsPBaseadaptor ba,
                                               AjPStr identifiers,
                                               AjPList objects)
{
    AjPStr constraint = NULL;

    if(!ba)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    if(!objects)
        return ajFalse;

    if(!ba->Tables[0])
        return ajFalse;

    /*
    ** FIXME: The Perl API has an ordered parameter to this function, which
    ** appends an 'order by seq_region_id, seq_region_start' clause.
    ** A seq_region column however may not be part of the table definition.
    ** This should probably move to Bio::EnEMBL::DBSQL::BaseFeatureAdaptor,
    ** which always has a seq_region associated.
    */

    constraint = ajFmtStr("%s.%s_id IN (%S)",
                          ba->Tables[0],
                          ba->Tables[0],
                          identifiers);

    ensBaseadaptorGenericFetch(ba,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               objects);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensBaseadaptorFetchAllByIdentifiers **********************************
**
** Generic function to fetch Ensembl Objects by an AJAX List of
** SQL database-internal identifiers via an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor.
** @param [r] idlist [AjPList] AJAX List of AJAX unsigned integers of
**                             SQL database-internal identifiers
**                             used in an IN comparison function in a
**                             SQL SELECT statement
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAllByIdentifiers(const EnsPBaseadaptor ba,
                                           AjPList idlist,
                                           AjPList objects)
{
    register ajuint i = 0;

    ajuint *Pidentifier = NULL;

    AjIList iter = NULL;

    AjPStr identifiers = NULL;

    if(!ba)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    if(!objects)
        return ajFalse;

    iter = ajListIterNew(idlist);

    identifiers = ajStrNew();

    while(!ajListIterDone(iter))
    {
        Pidentifier = (ajuint *) ajListIterGet(iter);

        ajFmtPrintAppS(&identifiers, "%u, ", *Pidentifier);

        i++;

        /* Run the statement if we exceed the maximum chunk size. */

        if(i >= baseAdaptorFetchAllByIdentifiersMax)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&identifiers, 2);

            baseAdaptorFetchAllByIdentifiers(ba, identifiers, objects);

            ajStrAssignClear(&identifiers);

            i = 0;
        }
    }

    ajListIterDel(&iter);

    /* Run the final statement. */

    /* Remove the last comma and space. */

    ajStrCutEnd(&identifiers, 2);

    baseAdaptorFetchAllByIdentifiers(ba, identifiers, objects);

    ajStrDel(&identifiers);

    return ajTrue;
}




/* @func ensBaseadaptorFetchAll ***********************************************
**
** Generic function to fetch all Ensembl Objects via an Ensembl Base Adaptor.
** Please note that it is probably not a good idea to use this function on
** *very* large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAll(const EnsPBaseadaptor ba, AjPList objects)
{
    if(!ba)
        return ajFalse;

    if(!objects)
        return ajFalse;

    return ensBaseadaptorGenericFetch(ba,
                                      (AjPStr) NULL,
                                      (EnsPAssemblymapper) NULL,
                                      (EnsPSlice) NULL,
                                      objects);
}




/* @func ensBaseadaptorFetchAllIdentifiers ************************************
**
** Generic function to fetch SQL database-internal identifiers of
** Ensembl Objects via an Ensembl Base Adaptor.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_list_dbIDs
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] table [AjPStr] SQL table name
** @param [r] prikey [AjPStr] Primary key of the SQL table
** @param [u] idlist [AjPList] AJAX List of AJAX unsigned integer
**                             SQL database-internal identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAllIdentifiers(const EnsPBaseadaptor ba,
                                         AjPStr table,
                                         AjPStr prikey,
                                         AjPList idlist)
{
    ajuint *Pid = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli = NULL;
    AjPSqlrow sqlr = NULL;

    AjPStr statement = NULL;

    if(!ba)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    if(prikey && ajStrGetLen(prikey))
        statement = ajFmtStr("SELECT %S.%S FROM %S", table, prikey, table);
    else
        statement = ajFmtStr("SELECT %S.%S_id FROM %S", table, table, table);

    sqls = ensDatabaseadaptorSqlstatementNew(ba->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pid);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pid);

        ajListPushAppend(idlist, (void *) Pid);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}

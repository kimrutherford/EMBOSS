/* @source Ensembl Base Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.22 $
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
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const ensBaseadaptorMaximumIdentifiers ************************************
**
** Limit the number of identifiers in SQL queries to chunks of maximum size.
** Ensure that the MySQL max_allowed_packet is not exceeded, which defaults
** to 1 MB, by splitting large queries into smaller queries of at most 256 KiB
** (32768 8-bit characters). Assuming a (generous) average identifier string
** length of 16, this means a maximum of 2048 identifiers in each statement.
**
******************************************************************************/

const ajuint ensBaseadaptorMaximumIdentifiers = 2048U;




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool baseadaptorFetchAllbyIdentifiers(EnsPBaseadaptor ba,
                                               const AjPStr csv,
                                               EnsPSlice slice,
                                               AjPList objects);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensbaseadaptor ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPBaseadaptor] Ensembl Base Adaptor ************************
**
** @nam2rule Baseadaptor Functions for manipulating
** Ensembl Base Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor
** @cc CVS Revision: 1.56
** @cc CVS Tag: branch-ensembl-62
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
**
** @nam3rule New Constructor with initial values
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule New Ptables [const char* const*] Table names
** @argrule New Pcolumns [const char* const*] Column names
** @argrule New leftjoin [const EnsPBaseadaptorLeftjoin] LEFT JOIN statements
** @argrule New condition [const char*] SQL SELECT default condition
** @argrule New final [const char*] SQL SELECT final condition
** @argrule New Fstatement [AjBool function] Statement function address
**
** @valrule * [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensBaseadaptorNew ****************************************************
**
** Default constructor for an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] Ptables [const char* const*] Table names
** @param [r] Pcolumns [const char* const*] Column names
** @param [r] leftjoin [const EnsPBaseadaptorLeftjoin] LEFT JOIN conditions
** @param [r] condition [const char*] SQL SELECT default condition
** @param [r] final [const char*] SQL SELECT final condition
** @param [f] Fstatement [AjBool function] Statement function address
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensBaseadaptorNew(
    EnsPDatabaseadaptor dba,
    const char* const* Ptables,
    const char* const* Pcolumns,
    const EnsPBaseadaptorLeftjoin leftjoin,
    const char* condition,
    const char* final,
    AjBool Fstatement(EnsPDatabaseadaptor dba,
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

    ba->Adaptor          = dba;
    ba->Tables           = Ptables;
    ba->Columns          = Pcolumns;
    ba->Leftjoin         = leftjoin;
    ba->Defaultcondition = condition;
    ba->Finalcondition   = final;
    ba->Fstatement       = Fstatement;

    return ba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Base Adaptor object
**
** @argrule * Pba [EnsPBaseadaptor*] Ensembl Base Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensBaseadaptorDel ****************************************************
**
** Default destructor for an Ensembl Base Adaptor.
**
** @param [d] Pba [EnsPBaseadaptor*] Ensembl Base Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensBaseadaptorDel(EnsPBaseadaptor* Pba)
{
    EnsPBaseadaptor pthis = NULL;

    if(!Pba)
        return;

    if(!*Pba)
        return;

    pthis = *Pba;

    AJFREE(pthis);

    *Pba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Get Return Base Adaptor attribute(s)
** @nam4rule Columns Return the columns
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Tables Return the tables
**
** @argrule * ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @valrule Columns [const char* const*] Columns or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
** @valrule Tables [const char* const*] Tables or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensBaseadaptorGetColumns *********************************************
**
** Get the columns element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_columns
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char* const*] Columns or NULL
** @@
******************************************************************************/

const char* const* ensBaseadaptorGetColumns(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Columns;
}




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
** @return [const char* const*] Tables or NULL
** @@
******************************************************************************/

const char* const* ensBaseadaptorGetTables(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Tables;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Set Set one element of a Base Adaptor
** @nam4rule Columns Set the columns
** @nam4rule Defaultcondition Set the SQL SELECT default condition
** @nam4rule Finalcondition Set the SQL SELECT final condition
** @nam4rule Tables Set the tables
**
** @argrule * ba [EnsPBaseadaptor] Base Adaptor
** @argrule Columns Pcolumns [const char* const*] Columns
** @argrule Defaultcondition condition [const char*]
** SQL SELECT default condition
** @argrule Finalcondition final [const char*]
** SQL SELECT final condition
** @argrule Tables Ptables [const char* const*] Tables
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensBaseadaptorSetColumns *********************************************
**
** Set the columns element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_columns
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] Pcolumns [const char* const*] Columns
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetColumns(EnsPBaseadaptor ba,
                                const char* const* Pcolumns)
{
    if(!ba)
        return ajFalse;

    if(!Pcolumns)
        return ajFalse;

    ba->Columns = Pcolumns;

    return ajTrue;
}




/* @func ensBaseadaptorSetDefaultcondition ************************************
**
** Set the SQL SELECT default condition element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_default_where_clause
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] condition [const char*] SQL SELECT default condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetDefaultcondition(EnsPBaseadaptor ba,
                                         const char* condition)
{
    if(!ba)
        return ajFalse;

    ba->Defaultcondition = condition;

    return ajTrue;
}




/* @func ensBaseadaptorSetFinalcondition **************************************
**
** Set the SQL SELECT final condition element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_final_clause
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] final [const char*] SQL SELECT final condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetFinalcondition(EnsPBaseadaptor ba, const char* final)
{
    if(!ba)
        return ajFalse;

    ba->Finalcondition = final;

    return ajTrue;
}




/* @func ensBaseadaptorSetTables **********************************************
**
** Set the tables element of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_tables
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] Ptables [const char* const*] Tables
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorSetTables(EnsPBaseadaptor ba,
                               const char* const* Ptables)
{
    if(!ba)
        return ajFalse;

    if(!Ptables)
        return ajFalse;

    ba->Tables = Ptables;

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Base Adaptor convenience functions
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Escape Escape a string
** @nam4rule C Escape to an AJAX String
** @nam4rule S Escape to a C-type character string
** @nam3rule Get Get members(s)
** @nam4rule All Get all members
** @nam4rule Multispecies Get the Ensembl Database Adaptor
** multiple-species element
** @nam4rule Primarytable Get the primarty SQL table name
** @nam4rule Speciesidentifier Get the Ensembl Database Adaptor
** species identifier element
**
** @argrule Escape ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @argrule EscapeC Ptxt [char**] Address of the (new) SQL-escaped C string
** @argrule EscapeC str [const AjPStr] AJAX String to be escaped
** @argrule EscapeS Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @argrule EscapeS str [const AjPStr] AJAX String to be escaped
** @argrule Get ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @valrule EscapeC [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule EscapeS [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Multispecies [AjBool] ajTrue for multiple species
** @valrule Primarytable [const char*] Primary table name address or NULL
** @valrule Speciesidentifier [ajuint] Ensembl Database Adaptor
** species identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensBaseadaptorEscapeC ************************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return a C-type character string.
**
** The caller is responsible for deleting the escaped C-type character string.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [wP] Ptxt [char**] Address of the (new) SQL-escaped C string
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorEscapeC(EnsPBaseadaptor ba, char** Ptxt, const AjPStr str)
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




/* @func ensBaseadaptorEscapeS ************************************************
**
** Escape special characters in an AJAX String for use in an SQL statement,
** taking into account the current character set of the AJAX SQL Connection
** and return an AJAX String.
**
** The caller is responsible for deleting the escaped AJAX String.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [wP] Pstr [AjPStr*] Address of the (new) SQL-escaped AJAX String
** @param [r] str [const AjPStr] AJAX String to be escaped
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorEscapeS(EnsPBaseadaptor ba, AjPStr* Pstr,
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




/* @func ensBaseadaptorGetMultispecies ****************************************
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

AjBool ensBaseadaptorGetMultispecies(const EnsPBaseadaptor ba)
{
    if(!ba)
        return ajFalse;

    return ensDatabaseadaptorGetMultispecies(ba->Adaptor);
}




/* @func ensBaseadaptorGetPrimarytable ****************************************
**
** Get the primary SQL table name of an Ensembl Base Adaptor.
** The primary table name is the first one in the array.
**
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char*] Primary table name address or NULL
** @@
******************************************************************************/

const char* ensBaseadaptorGetPrimarytable(const EnsPBaseadaptor ba)
{
    if(!ba)
        return NULL;

    return ba->Tables[0];
}




/* @func ensBaseadaptorGetSpeciesidentifier ***********************************
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

ajuint ensBaseadaptorGetSpeciesidentifier(const EnsPBaseadaptor ba)
{
    if(!ba)
        return 0;

    return ensDatabaseadaptorGetIdentifier(ba->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Objects from an Ensembl database.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Fetch       Fetch Ensembl Object(s)
** @nam4rule All         Fetch all Ensembl Objects
** @nam4rule Allby       Fetch all Ensembl Objects matching a criterion
** @nam5rule Constraint  Fetch all Ensembl Objects by an SQL SELECT constraint
** @nam5rule Identifiers Fetch all by SQL database-internal identifiers
** @nam4rule By          Fetch one Ensembl Object matching a criterion
** @nam5rule Identifier  Fetch by SQL database-internal identifier
**
** @argrule * ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @argrule All objects [AjPList] AJAX List of Ensembl Objects
** @argrule AllbyConstraint constraint [const AjPStr] SQL SELECT constraint
** @argrule AllbyConstraint am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @argrule AllbyConstraint slice [EnsPSlice] Ensembl Slice
** @argrule AllbyIdentifiers identifiers [const AjPList] AJAX List of
** AJAX unsigned integer (SQL database-internal identifier) objects
** @argrule AllbyIdentifiers slice [EnsPSlice] Ensembl Slice
** @argrule Allby objects [AjPList] AJAX List of Ensembl Objects
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pobject [void**] Ensembl Object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensBaseadaptorFetchAll ***********************************************
**
** Generic function to fetch all Ensembl Objects via an Ensembl Base Adaptor.
** Please note that it is probably not a good idea to use this function on
** *very* large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAll(EnsPBaseadaptor ba,
                              AjPList objects)
{
    if(!ba)
        return ajFalse;

    if(!objects)
        return ajFalse;

    return ensBaseadaptorFetchAllbyConstraint(ba,
                                              (AjPStr) NULL,
                                              (EnsPAssemblymapper) NULL,
                                              (EnsPSlice) NULL,
                                              objects);
}




/* @func ensBaseadaptorFetchAllbyConstraint ***********************************
**
** Generic function to fetch Ensembl Objects via an Ensembl Base Adaptor.
**
** The caller is responsible for deleting the Ensembl Objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::generic_fetch
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] constraint [const AjPStr] SQL SELECT constraint
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] objects [AjPList] AJAX List to Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAllbyConstraint(EnsPBaseadaptor ba,
                                          const AjPStr constraint,
                                          EnsPAssemblymapper am,
                                          EnsPSlice slice,
                                          AjPList objects)
{
    register ajint i = 0;
    register ajint j = 0;

    AjBool match = AJFALSE;

    AjPStr columns     = NULL;
    AjPStr tables      = NULL;
    AjPStr joins       = NULL;
    AjPStr parentheses = NULL;
    AjPStr statement   = NULL;

    if(ajDebugTest("ensBaseadaptorFetchAllbyConstraint"))
        ajDebug("ensBaseadaptorFetchAllbyConstraint\n"
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

    columns     = ajStrNew();
    tables      = ajStrNew();
    joins       = ajStrNew();
    parentheses = ajStrNew();

    /* Build the column expression. */

    for(i = 0; ba->Columns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", ba->Columns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /*
    ** Construct a left join statement if one was defined, and remove the
    ** left-joined tables from the table expression.
    */

    for(i = 0; ba->Tables[i]; i++)
    {
        match = ajFalse;

        if(ba->Leftjoin)
        {
            for(j = 0; ba->Leftjoin[j].Table; j++)
            {
                if(ajCharMatchC(ba->Tables[i], ba->Leftjoin[j].Table))
                {
                    ajStrAppendK(&parentheses, '(');

                    ajFmtPrintAppS(&joins,
                                   "LEFT JOIN %s ON %s) ",
                                   ba->Leftjoin[j].Table,
                                   ba->Leftjoin[j].Condition);

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

        if(ba->Defaultcondition)
            ajFmtPrintAppS(&statement, " AND %s", ba->Defaultcondition);
    }
    else if(ba->Defaultcondition)
        ajFmtPrintAppS(&statement, " WHERE %s", ba->Defaultcondition);

    if(ba->Finalcondition)
        ajFmtPrintAppS(&statement, " %s", ba->Finalcondition);

    ba->Fstatement(ba->Adaptor, statement, am, slice, objects);

    ajStrDel(&columns);
    ajStrDel(&tables);
    ajStrDel(&joins);
    ajStrDel(&parentheses);
    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic baseadaptorFetchAllbyIdentifiers *******************************
**
** Helper function for the generic function to fetch Ensembl Objects by an
** AJAX List of SQL database-internal identifiers via an Ensembl Base Adaptor.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] csv [const AjPStr] Comma-separated values of SQL database-
** internal identifiers used in an IN comparison function in a SQL SELECT
** statement
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: The Perl API has an ordered parameter to this function, which
** appends an 'order by seq_region_id, seq_region_start' clause.
** A seq_region column, however, may not be part of the table definition,
** especially if this object does not inherit from Bio::EnsEMBL::Feature.
** This should probably move into the Bio::EnEMBL::DBSQL::BaseFeatureAdaptor,
** which always has a seq_region_id associated.
******************************************************************************/

static AjBool baseadaptorFetchAllbyIdentifiers(EnsPBaseadaptor ba,
                                               const AjPStr csv,
                                               EnsPSlice slice,
                                               AjPList objects)
{
    AjPStr constraint = NULL;

    if(!ba)
        return ajFalse;

    if(!csv)
        return ajFalse;

    if(!objects)
        return ajFalse;

    if(!ba->Tables[0])
        return ajFalse;

    constraint = ajFmtStr("%s.%s_id IN (%S)",
                          ba->Tables[0],
                          ba->Tables[0],
                          csv);

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       slice,
                                       objects);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensBaseadaptorFetchAllbyIdentifiers **********************************
**
** Generic function to fetch Ensembl Objects by an AJAX List of
** SQL database-internal identifiers via an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor.
** @param [r] identifiers [const AjPList] AJAX List of AJAX unsigned integer
** (SQL database-internal identifier) objects used in an SQL SELECT IN
** comparison function
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAllbyIdentifiers(EnsPBaseadaptor ba,
                                           const AjPList identifiers,
                                           EnsPSlice slice,
                                           AjPList objects)
{
    register ajuint i = 0;

    ajuint* Pidentifier = NULL;

    AjIList iter = NULL;

    AjPStr csv = NULL;

    if(!ba)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    if(!objects)
        return ajFalse;

    iter = ajListIterNewread(identifiers);

    csv = ajStrNew();

    while(!ajListIterDone(iter))
    {
        Pidentifier = (ajuint*) ajListIterGet(iter);

        ajFmtPrintAppS(&csv, "%u, ", *Pidentifier);

        i++;

        /* Run the statement if we exceed the maximum chunk size. */

        if((i % ensBaseadaptorMaximumIdentifiers) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            baseadaptorFetchAllbyIdentifiers(ba, csv, slice, objects);

            ajStrAssignClear(&csv);
        }
    }

    ajListIterDel(&iter);

    /* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
        baseadaptorFetchAllbyIdentifiers(ba, csv, slice, objects);

    ajStrDel(&csv);

    return ajTrue;
}




/* @func ensBaseadaptorFetchByIdentifier **************************************
**
** Generic function to fetch an Ensembl Object by its SQL database-internal
** identifier via an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pobject [void**] Ensembl Object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchByIdentifier(EnsPBaseadaptor ba,
                                       ajuint identifier,
                                       void** Pobject)
{
    AjPList objects = NULL;

    AjPStr constraint = NULL;

    if(!ba)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pobject)
        return ajFalse;

    if(!ba->Tables[0])
        return ajFalse;

    constraint = ajFmtStr("%s.%s_id = %u",
                          ba->Tables[0],
                          ba->Tables[0],
                          identifier);

    objects = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       objects);

    if(ajListGetLength(objects) > 1)
        ajFatal("ensBaseadaptorFetchByIdentifier got more than one object "
                "for constraint '%S'.\n", constraint);

    /*
    ** NOTE: Since an object-specific deletion function is not available,
    ** any additional objects lead to a memory leak.
    */

    ajListPop(objects, Pobject);

    ajListFree(&objects);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Transcript objects
** from an Ensembl SQL database.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Object-releated object(s)
** @nam4rule All Retrieve all Ensembl Object-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
** @nam5rule Strings Retrieve all AJAX String objects
**
** @argrule * ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @argrule * table [const AjPStr] SQL table name
** @argrule * primary [const AjPStr] SQL table primary key
** @argrule Identifiers identifiers [AjPList] AJAX List of
** AJAX unsigned integer objects
** @argrule Strings strings [AjPList] AJAX List of AJAX String objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensBaseadaptorRetrieveAllIdentifiers *********************************
**
** Generic function to retrieve SQL database-internal identifiers of
** Ensembl Objects via an Ensembl Base Adaptor.
**
** The caller is responsible for deleting the AJAX unsigned integer objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_list_dbIDs
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] table [const AjPStr] SQL table name
** @param [rN] primary [const AjPStr] SQL table primary key
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integer
** (SQL database-internal identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorRetrieveAllIdentifiers(EnsPBaseadaptor ba,
                                            const AjPStr table,
                                            const AjPStr primary,
                                            AjPList identifiers)
{
    ajuint* Pid = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    if(!ba)
        return ajFalse;

    if(!table)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    if(primary && ajStrGetLen(primary))
        statement = ajFmtStr("SELECT %S.%S FROM %S", table, primary, table);
    else
        statement = ajFmtStr("SELECT %S.%S_id FROM %S", table, table, table);

    sqls = ensDatabaseadaptorSqlstatementNew(ba->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pid);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pid);

        ajListPushAppend(identifiers, (void*) Pid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ba->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensBaseadaptorRetrieveAllStrings *************************************
**
** Generic function to retrieve SQL database-internal AJAX String objects of
** Ensembl Objects via an Ensembl Base Adaptor.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_list_dbIDs
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] table [const AjPStr] SQL table name
** @param [rN] primary [const AjPStr] Primary key of the SQL table
** @param [u] strings [AjPList] AJAX List of AJAX String
**                              SQL database-internal strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorRetrieveAllStrings(EnsPBaseadaptor ba,
                                        const AjPStr table,
                                        const AjPStr primary,
                                        AjPList strings)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr string    = NULL;

    if(!ba)
        return ajFalse;

    if(!table)
        return ajFalse;

    if(!strings)
        return ajFalse;

    if(primary && ajStrGetLen(primary))
        statement = ajFmtStr("SELECT %S.%S FROM %S", table, primary, table);
    else
        statement = ajFmtStr("SELECT %S.%S_id FROM %S", table, table, table);

    sqls = ensDatabaseadaptorSqlstatementNew(ba->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        string = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &string);

        ajListPushAppend(strings, (void*) string);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ba->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

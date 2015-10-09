/* @source ensbaseadaptor *****************************************************
**
** Ensembl Base Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.38 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/26 06:36:44 $ by $Author: mks $
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

#include "ensanalysis.h"
#include "ensbaseadaptor.h"
#include "ensfeature.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const ensKBaseadaptorMaximumIdentifiers ***********************************
**
** Limit the number of identifiers in SQL queries to chunks of maximum size.
** Ensure that the MySQL max_allowed_packet is not exceeded, which defaults
** to 1 MB, by splitting large queries into smaller queries of at most 256 KiB
** (32768 8-bit characters). Assuming a (generous) average identifier string
** length of 16, this means a maximum of 2048 identifiers in each statement.
**
******************************************************************************/

const ajuint ensKBaseadaptorMaximumIdentifiers = 2048U;




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

static AjBool baseadaptorRetrieveAllStatement(
    EnsPBaseadaptor ba,
    const AjPStr table,
    const AjPStr primary,
    AjPStr *Pstatement);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




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
** @cc CVS Revision: 1.65
** @cc CVS Tag: branch-ensembl-66
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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBaseadaptor ensBaseadaptorNew(
    EnsPDatabaseadaptor dba,
    const char *const *Ptables,
    const char *const *Pcolumns,
    const EnsPBaseadaptorLeftjoin leftjoin,
    const char *condition,
    const char *final,
    AjBool (*Fstatement) (EnsPBaseadaptor ba,
                          const AjPStr statement,
                          EnsPAssemblymapper am,
                          EnsPSlice slice,
                          AjPList objects))
{
    EnsPBaseadaptor ba = NULL;

    if (!dba)
        return NULL;

    if (!Ptables)
        return NULL;

    if (!Pcolumns)
        return NULL;

    if (!Fstatement)
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
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Base Adaptor
**
** @argrule * Pba [EnsPBaseadaptor*] Ensembl Base Adaptor address
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
**
** @release 6.2.0
** @@
******************************************************************************/

void ensBaseadaptorDel(EnsPBaseadaptor *Pba)
{
    EnsPBaseadaptor pthis = NULL;

    if (!Pba)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensBaseadaptorDel"))
        ajDebug("ensBaseadaptorDel\n"
                "  *Pba %p\n",
                *Pba);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pba)
        return;

    pthis = *Pba;

    AJFREE(pthis);

    *Pba = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Base Adaptor object.
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
** Get the columns member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_columns
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char* const*] Columns or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* const* ensBaseadaptorGetColumns(const EnsPBaseadaptor ba)
{
    return (ba) ? ba->Columns : NULL;
}




/* @func ensBaseadaptorGetDatabaseadaptor *************************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::db
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensBaseadaptorGetDatabaseadaptor(const EnsPBaseadaptor ba)
{
    return (ba) ? ba->Adaptor : NULL;
}




/* @func ensBaseadaptorGetTables **********************************************
**
** Get the tables member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_tables
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char* const*] Tables or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* const* ensBaseadaptorGetTables(const EnsPBaseadaptor ba)
{
    return (ba) ? ba->Tables : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Base Adaptor object.
**
** @fdata [EnsPBaseadaptor]
**
** @nam3rule Set Set one member of a Base Adaptor
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
** Set the columns member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_columns
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] Pcolumns [const char* const*] Columns
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBaseadaptorSetColumns(EnsPBaseadaptor ba,
                                const char* const* Pcolumns)
{
    if (!ba)
        return ajFalse;

    if (!Pcolumns)
        return ajFalse;

    ba->Columns = Pcolumns;

    return ajTrue;
}




/* @func ensBaseadaptorSetDefaultcondition ************************************
**
** Set the SQL SELECT default condition member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_default_where_clause
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] condition [const char*] SQL SELECT default condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensBaseadaptorSetDefaultcondition(EnsPBaseadaptor ba,
                                         const char *condition)
{
    if (!ba)
        return ajFalse;

    ba->Defaultcondition = condition;

    return ajTrue;
}




/* @func ensBaseadaptorSetFinalcondition **************************************
**
** Set the SQL SELECT final condition member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_final_clause
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] final [const char*] SQL SELECT final condition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensBaseadaptorSetFinalcondition(EnsPBaseadaptor ba,
                                       const char *final)
{
    if (!ba)
        return ajFalse;

    ba->Finalcondition = final;

    return ajTrue;
}




/* @func ensBaseadaptorSetTables **********************************************
**
** Set the tables member of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_tables
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] Ptables [const char* const*] Tables
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBaseadaptorSetTables(EnsPBaseadaptor ba,
                               const char* const* Ptables)
{
    if (!ba)
        return ajFalse;

    if (!Ptables)
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
** multiple-species flag
** @nam4rule Primarytable Get the primarty SQL table name
** @nam4rule Speciesidentifier Get the Ensembl Database Adaptor
** species identifier
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBaseadaptorEscapeC(EnsPBaseadaptor ba,
                             char **Ptxt,
                             const AjPStr str)
{
    if (!ba)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensBaseadaptorEscapeC"))
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBaseadaptorEscapeS(EnsPBaseadaptor ba,
                             AjPStr *Pstr,
                             const AjPStr str)
{
    if (!ba)
        return ajFalse;

    if (!str)
        return ajFalse;

    if (ajDebugTest("ensBaseadaptorEscapeS"))
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
** Get the multiple-species member of the Ensembl Database Adaptor member
** of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::is_multispecies
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [AjBool] ajTrue if the database contains multiple species,
**                  ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensBaseadaptorGetMultispecies(const EnsPBaseadaptor ba)
{
    return (ba) ? ensDatabaseadaptorGetMultispecies(ba->Adaptor) : ajFalse;
}




/* @func ensBaseadaptorGetPrimarytable ****************************************
**
** Get the primary SQL table name of an Ensembl Base Adaptor.
** The primary table name is the first one in the array.
**
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [const char*] Primary table name address or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensBaseadaptorGetPrimarytable(const EnsPBaseadaptor ba)
{
    return (ba) ? ba->Tables[0] : NULL;
}




/* @func ensBaseadaptorGetSpeciesidentifier ***********************************
**
** Get the species identifier member of the Ensembl Database Adaptor member
** of an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::species_id
** @param [r] ba [const EnsPBaseadaptor] Ensembl Base Adaptor
**
** @return [ajuint] Ensembl species identifier or 0U, defaults to 1U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensBaseadaptorGetSpeciesidentifier(const EnsPBaseadaptor ba)
{
    return (ba) ? ensDatabaseadaptorGetIdentifier(ba->Adaptor) : 0U;
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
** @argrule AllbyConstraint objects [AjPList] AJAX List of Ensembl Objects
** @argrule AllbyIdentifiers slice [EnsPSlice] Ensembl Slice
** @argrule AllbyIdentifiers FobjectGetIdentifier [ajuint function]
** Get Ensembl Object Identifier function address
** @argrule AllbyIdentifiers objects [AjPTable]
** AJAX Table of AJAX unsigned integer key data (SQL database-internal
** identifier used in an SQL SELECT IN comparison function) and Ensembl Object
** value data
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
** very large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAll(
    EnsPBaseadaptor ba,
    AjPList objects)
{
    if (!ba)
        return ajFalse;

    if (!objects)
        return ajFalse;

    return ensBaseadaptorFetchAllbyConstraint(
        ba,
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
** @param [rN] constraint [const AjPStr] SQL SELECT constraint
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] objects [AjPList] AJAX List to Ensembl Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAllbyConstraint(
    EnsPBaseadaptor ba,
    const AjPStr constraint,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList objects)
{
    register ajuint i = 0U;
    register ajuint j = 0U;

    AjBool debug  = AJFALSE;
    AjBool match  = AJFALSE;
    AjBool result = AJFALSE;

    AjPStr columns     = NULL;
    AjPStr tables      = NULL;
    AjPStr joins       = NULL;
    AjPStr parentheses = NULL;
    AjPStr statement   = NULL;

    debug = ajDebugTest("ensBaseadaptorFetchAllbyConstraint");

    if (debug)
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

    if (!ba)
        return ajFalse;

    if (!objects)
        return ajFalse;

    columns     = ajStrNew();
    tables      = ajStrNew();
    joins       = ajStrNew();
    parentheses = ajStrNew();

    /* Build the column expression. */

    for (i = 0U; ba->Columns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", ba->Columns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /*
    ** Construct a left join statement if one was defined, and remove the
    ** left-joined tables from the table expression.
    */

    for (i = 0U; ba->Tables[i]; i++)
    {
        if (debug)
            ajDebug("ensBaseadaptorFetchAllbyConstraint "
                    "element %u "
                    "table '%s'\n",
                    i, ba->Tables[i]);

        match = ajFalse;

        if (ba->Leftjoin)
        {
            for (j = 0U; ba->Leftjoin[j].Table; j++)
            {
                if (ajCharMatchC(ba->Tables[i], ba->Leftjoin[j].Table))
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

        if (!match)
            ajFmtPrintAppS(&tables, "%s, ", ba->Tables[i]);
    }

    /* Remove last comma and space from the table expression. */

    ajStrCutEnd(&tables, 2);

    /* Build the SQL statement. */

    statement = ajStrNewC("SELECT");

    if (ba->StraightJoin)
        ajStrAppendC(&statement, " STRAIGHT_JOIN");

    ajFmtPrintAppS(&statement, " %S FROM %S(%S)",
                   columns, parentheses, tables);

    if (joins && ajStrGetLen(joins))
        ajFmtPrintAppS(&statement, " %S", joins);

    if (constraint && ajStrGetLen(constraint))
    {
        ajFmtPrintAppS(&statement, " WHERE %S", constraint);

        if (ba->Defaultcondition)
            ajFmtPrintAppS(&statement, " AND %s", ba->Defaultcondition);
    }
    else if (ba->Defaultcondition)
        ajFmtPrintAppS(&statement, " WHERE %s", ba->Defaultcondition);

    if (ba->Finalcondition)
        ajFmtPrintAppS(&statement, " %s", ba->Finalcondition);

    result = (*ba->Fstatement) (ba, statement, am, slice, objects);

    ajStrDel(&columns);
    ajStrDel(&tables);
    ajStrDel(&joins);
    ajStrDel(&parentheses);
    ajStrDel(&statement);

    return result;
}




/* @func ensBaseadaptorFetchAllbyIdentifiers **********************************
**
** Generic function to fetch Ensembl Objects by an AJAX Table of
** AJAX unsigned integer key data (SQL database-internal identifiers)
** via an Ensembl Base Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor.
**
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [f] FobjectGetIdentifier [ajuint function]
** Get Ensembl Object Identifier function address
** @param [u] objects [AjPTable]
** AJAX Table of AJAX unsigned integer key data (SQL database-internal
** identifier used in an SQL SELECT IN comparison function) and Ensembl Object
** value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchAllbyIdentifiers(
    EnsPBaseadaptor ba,
    EnsPSlice slice,
    ajuint (*FobjectGetIdentifier) (const void *object),
    AjPTable objects)
{
    void **keyarray = NULL;

    void *object = NULL;

    register ajuint i = 0U;

    ajuint *Pidentifier = NULL;

    AjBool debug = AJFALSE;

    AjPList list = NULL;

    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    debug = ajDebugTest("ensBaseadaptorFetchAllbyIdentifiers");

    if (debug)
    {
        ajDebug("ensBaseadaptorFetchAllbyIdentifiers"
                "  ba %p\n"
                "  slice %p\n"
                "  FobjectGetIdentifier %p\n"
                "  objects %p\n",
                ba,
                slice,
                FobjectGetIdentifier,
                objects);

        ajTableTrace(objects);
    }

    if (!ba)
        return ajFalse;

    if (!FobjectGetIdentifier)
        return ajFalse;

    if (!objects)
        return ajFalse;

    list = ajListNew();

    csv = ajStrNew();

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifiers configured in the Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    ajTableToarrayKeys(objects, &keyarray);

    for (i = 0U; keyarray[i]; i++)
    {
        if (debug)
            ajDebug("ensBaseadaptorFetchAllbyIdentifiers identifier %u\n",
                    *((ajuint *) keyarray[i]));

        ajFmtPrintAppS(&csv, "%u, ", *((ajuint *) keyarray[i]));

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more array elements to process.
        */

        if ((((i + 1U) % ensKBaseadaptorMaximumIdentifiers) == 0) ||
            (keyarray[i + 1U] == NULL))
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                constraint = ajFmtStr("%s.%s_id IN (%S)",
                                      ba->Tables[0U],
                                      ba->Tables[0U],
                                      csv);

                ensBaseadaptorFetchAllbyConstraint(ba,
                                                   constraint,
                                                   (EnsPAssemblymapper) NULL,
                                                   slice,
                                                   list);

                ajStrDel(&constraint);
            }

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    ajStrDel(&csv);

    /* Move Ensembl Objects from the AJAX List to the AJAX Table. */

    while (ajListPop(list, (void **) &object))
    {
        AJNEW0(Pidentifier);

        *Pidentifier = (*FobjectGetIdentifier) (object);

        ajTablePut(objects, (void *) Pidentifier, object);
    }

    ajListFree(&list);

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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBaseadaptorFetchByIdentifier(
    EnsPBaseadaptor ba,
    ajuint identifier,
    void **Pobject)
{
    AjPList objects = NULL;

    AjPStr constraint = NULL;

    if (!ba)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pobject)
        return ajFalse;

    if (!ba->Tables[0])
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

    if (ajListGetLength(objects) > 1)
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
** @nam4rule Feature Retrieve an Ensembl Feature
**
** @argrule * ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @argrule All table [const AjPStr] SQL table name
** @argrule All primary [const AjPStr] SQL table primary key
** @argrule Identifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer objects
** @argrule Strings strings [AjPList]
** AJAX List of AJAX String objects
** @argrule Feature analysisid [ajuint] Ensembl Analysis identifier
** @argrule Feature srid [ajuint] Ensembl Sequence Region identifier
** @argrule Feature srstart [ajuint] Ensembl Sequence Region start
** @argrule Feature srend [ajuint] Ensembl Sequence Region end
** @argrule Feature srstrand [ajint] Ensembl Sequence Region strand
** @argrule Feature am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @argrule Feature slice [EnsPSlice] Ensembl Slice
** @argrule Feature Pfeature [EnsPFeature*] Ensembl Feature
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic baseadaptorRetrieveAllStatement ********************************
**
** Helper function building an SQL statement for the generic retrieval of
** AJAX unsigned integer or AJAX String Ensembl Object identifiers via an
** Ensembl Base Adaptor.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::_list_dbIDs
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] table [const AjPStr] SQL table name
** @param [rN] primary [const AjPStr] SQL table primary key
** @param [u] Pstatement [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool baseadaptorRetrieveAllStatement(
    EnsPBaseadaptor ba,
    const AjPStr table,
    const AjPStr primary,
    AjPStr *Pstatement)
{
    if (!ba)
        return ajFalse;

    if (!table)
        return ajFalse;

    if (!Pstatement)
        return ajFalse;

    if (primary && ajStrGetLen(primary))
        ajFmtPrintAppS(Pstatement,
                       "SELECT %S.%S FROM %S",
                       table,
                       primary,
                       table);
    else
        ajFmtPrintAppS(Pstatement,
                       "SELECT %S.%S_id FROM %S",
                       table,
                       table,
                       table);

    return ajTrue;
}




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
**
** @release 6.4.0
** @@
** NOTE: The Perl API has an ordered parameter to this function, which
** appends an 'order by seq_region_id, seq_region_start' clause.
** A seq_region column, however, may not be part of the table definition,
** especially if this object does not inherit from Bio::EnsEMBL::Feature.
** This should probably move into the Bio::EnEMBL::DBSQL::BaseFeatureAdaptor,
** which always has a seq_region_id associated.
******************************************************************************/

AjBool ensBaseadaptorRetrieveAllIdentifiers(
    EnsPBaseadaptor ba,
    const AjPStr table,
    const AjPStr primary,
    AjPList identifiers)
{
    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    if (!ba)
        return ajFalse;

    if (!table)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    statement = ajStrNew();

    baseadaptorRetrieveAllStatement(ba, table, primary, &statement);

    sqls = ensDatabaseadaptorSqlstatementNew(ba->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(identifiers, (void *) Pidentifier);
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
**
** @release 6.4.0
** @@
** NOTE: The Perl API has an ordered parameter to this function, which
** appends an 'order by seq_region_id, seq_region_start' clause.
** A seq_region column, however, may not be part of the table definition,
** especially if this object does not inherit from Bio::EnsEMBL::Feature.
** This should probably move into the Bio::EnEMBL::DBSQL::BaseFeatureAdaptor,
** which always has a seq_region_id associated.
******************************************************************************/

AjBool ensBaseadaptorRetrieveAllStrings(
    EnsPBaseadaptor ba,
    const AjPStr table,
    const AjPStr primary,
    AjPList strings)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr string    = NULL;

    if (!ba)
        return ajFalse;

    if (!table)
        return ajFalse;

    if (!strings)
        return ajFalse;

    statement = ajStrNew();

    baseadaptorRetrieveAllStatement(ba, table, primary, &statement);

    sqls = ensDatabaseadaptorSqlstatementNew(ba->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        string = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &string);

        ajListPushAppend(strings, (void *) string);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ba->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensBaseadaptorRetrieveFeature ****************************************
**
** Maps Ensembl Sequence Region coordinates for Ensembl Feature objects into
** Ensembl Slice coordinates and returns an Ensembl Feature object.
**
** The caller is responsible for deleting the Ensembl Feature.
** This function aims to simplify all private
** objectadaptorFetchAllByStatement functions.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] analysisid [ajuint] Ensembl Analysis identifier
** @param [r] srid [ajuint] Ensembl Sequence Region identifier
** @param [r] srstart [ajuint] Ensembl Sequence Region start
** @param [r] srend [ajuint] Ensembl Sequence Region end
** @param [r] srstrand [ajint] Ensembl Sequence Region strand
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] Pfeature [EnsPFeature*] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBaseadaptorRetrieveFeature(
    EnsPBaseadaptor ba,
    ajuint analysisid,
    ajuint srid,
    ajuint srstart,
    ajuint srend,
    ajint srstrand,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    EnsPFeature *Pfeature)
{
    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    ajint tmpstart = 0;

    AjBool circular = AJFALSE;

    AjPList mrs = NULL;

    EnsPAnalysis analysis   = NULL;
    EnsPAnalysisadaptor ana = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice    = NULL;
    EnsPSliceadaptor sla = NULL;

    /* Presult has to be the first! */

    if (Pfeature)
        *Pfeature = NULL;
    else
        return ajFalse;

    if (!ba)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ana = ensRegistryGetAnalysisadaptor(dba);
    ama = ensRegistryGetAssemblymapperadaptor(dba);
    csa = ensRegistryGetCoordsystemadaptor(dba);
    sla = (slice) ? ensSliceGetAdaptor(slice) : ensRegistryGetSliceadaptor(dba);

    /* Need to get the internal Ensembl Sequence Region identifier. */

    srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa, srid);

    /*
    ** Since the Ensembl SQL schema defines Sequence Region start and end
    ** coordinates as unsigned integers for all Ensembl Feature objects,
    ** the range needs checking.
    */

    if (srstart <= INT_MAX)
        slstart = (ajint) srstart;
    else
        ajFatal("ensBaseadaptorMapFeature got an Ensembl "
                "Sequence Region start coordinate (%u) outside the "
                "maximum integer limit (%d).",
                srstart, INT_MAX);

    if (srend <= INT_MAX)
        slend = (ajint) srend;
    else
        ajFatal("ensBaseadaptorMapFeature got an Ensembl "
                "Sequence Region end coordinate (%u) outside the "
                "maximum integer limit (%d).",
                srend, INT_MAX);

    slstrand = srstrand;

    /* Fetch a Slice spanning the entire Sequence Region. */

    ensSliceadaptorFetchBySeqregionIdentifier(sla, srid, 0, 0, 0, &srslice);

    /*
    ** Increase the reference counter of the Ensembl Assembly Mapper if
    ** one has been specified, otherwise fetch it from the database if a
    ** destination Slice has been specified.
    */

    if (am)
        am = ensAssemblymapperNewRef(am);
    else if (slice && (!ensCoordsystemMatch(
                           ensSliceGetCoordsystemObject(slice),
                           ensSliceGetCoordsystemObject(srslice))))
        ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice, &am);

    /*
    ** Remap the Feature coordinates to another Ensembl Coordinate System
    ** if an Ensembl Assembly Mapper is defined at this point.
    */

    if (am)
    {
        mrs = ajListNew();

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

        ajListPeekFirst(mrs, (void **) &mr);

        /*
        ** Skip Features that map to gaps or
        ** Coordinate System boundaries.
        */

        if (ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
        {
            /* This Ensembl Feature could not be mapped successfully. */

            while (ajListPop(mrs, (void **) &mr))
                ensMapperresultDel(&mr);

            ajListFree(&mrs);

            ensAssemblymapperDel(&am);

            ensSliceDel(&srslice);

            return ajTrue;
        }

        srid     = ensMapperresultGetObjectidentifier(mr);
        slstart  = ensMapperresultGetCoordinateStart(mr);
        slend    = ensMapperresultGetCoordinateEnd(mr);
        slstrand = ensMapperresultGetCoordinateStrand(mr);

        /*
        ** Replace the original Sequence Region Slice by a Slice in the
        ** Coordinate System just mapped to.
        */

        ensSliceDel(&srslice);

        ensSliceadaptorFetchBySeqregionIdentifier(sla,
                                                  srid,
                                                  0,
                                                  0,
                                                  0,
                                                  &srslice);

        while (ajListPop(mrs, (void **) &mr))
            ensMapperresultDel(&mr);

        ajListFree(&mrs);
    }

    ensAssemblymapperDel(&am);

    /*
    ** Convert Sequence Region Slice coordinates to destination Slice
    ** coordinates, if a destination Slice has been provided.
    */

    if (slice)
    {
        /* Check that the length of the Slice is within range. */

        if (ensSliceCalculateLength(slice) <= INT_MAX)
            sllength = (ajint) ensSliceCalculateLength(slice);
        else
            ajFatal("ensBaseadaptorMapFeature got an Ensembl Slice, "
                    "which length (%u) exceeds the "
                    "maximum integer limit (%d).",
                    ensSliceCalculateLength(slice), INT_MAX);

        if (!ensSliceIsCircular(slice, &circular))
            return ajFalse;

        if (ensSliceGetStrand(slice) >= 0)
        {
            /* On the positive strand ... */

            slstart = slstart - ensSliceGetStart(slice) + 1;
            slend   = slend   - ensSliceGetStart(slice) + 1;

            if (
                (
                    (slend > ensSliceGetStart(slice))
                    ||
                    (slend < 0)
                    ||
                    (
                        (ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                        &&
                        (slend < 0)
                     )
                 )
                &&
                (circular == ajTrue)
                )
            {
                if (slstart > slend)
                {
                    /* A Feature overlapping the chromsome origin. */

                    /* Region in the beginning of the chromosome. */
                    if (slend > ensSliceGetStart(slice))
                        slstart -= sllength;

                    if (slend < 0)
                        slend += sllength;
                }
                else
                {
                    if ((ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                        && (slend < 0))
                    {
                        /*
                        ** A region overlapping the chromosome origin
                        ** and a Feature, which is at the beginning of
                        ** the chromosome.
                        */

                        slstart += sllength;
                        slend   += sllength;
                    }
                }
            }
        }
        else
        {
            /* On the negative strand ... */

            if ((slstart > slend) && (circular == ajTrue))
            {
                /* Handle circular chromosomes. */

                if (ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                {
                    tmpstart = slstart;
                    slstart = ensSliceGetEnd(slice) - slend + 1;
                    slend   = ensSliceGetEnd(slice) + sllength - tmpstart + 1;
                }
                else
                {
                    if (slend > ensSliceGetStart(slice))
                    {
                        /*
                        ** Looking at the region in the beginning of the
                        ** chromosome.
                        */

                        slstart = ensSliceGetEnd(slice) - slend + 1;
                        slend   = slend - sllength - slstart + 1;
                    }
                    else
                    {
                        tmpstart = slstart;
                        slstart  = ensSliceGetEnd(slice) - slend - sllength + 1;
                        slend    = slend - tmpstart + 1;
                    }
                }
            }
            else
            {
                /* Non-circular Ensembl Slice objects... */

                slend   = ensSliceGetEnd(slice) - slstart + 1;
                slstart = ensSliceGetEnd(slice) - slend   + 1;
            }

            slstrand *= -1;
        }

        /*
        ** Throw away Features off the end of the requested Slice or on
        ** any other than the requested Slice.
        */

        if ((slend < 1) ||
            (slstart > sllength) ||
            (srid != ensSliceGetSeqregionIdentifier(slice)))
        {
            /* This Ensembl Feature could not be mapped successfully. */

            ensSliceDel(&srslice);

            return ajTrue;
        }

        /* Delete the Sequence Region Slice and set the requested Slice. */

        ensSliceDel(&srslice);

        srslice = ensSliceNewRef(slice);
    }

    if (analysisid)
        ensAnalysisadaptorFetchByIdentifier(ana, analysisid, &analysis);

    *Pfeature = ensFeatureNewIniS(analysis, srslice, slstart, slend, slstrand);

    ensAnalysisDel(&analysis);

    ensSliceDel(&srslice);

    return ajTrue;
}

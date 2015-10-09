/* @source ensgvdatabaseadaptor ***********************************************
**
** Ensembl Genetic Variation Database Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.12 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#include "ensgvdatabaseadaptor.h"




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




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvdatabaseadaptor ******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvdatabaseadaptor] Ensembl Genetic Variation
** Database Adaptor
**
** @nam2rule Gvdatabaseadaptor Functions for manipulating
** Ensembl Genetic Variation Database Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor
** @cc CVS Revision: 1.39
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Database Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Genentic Variation Database Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvdatabaseadaptor]
**
** @nam3rule New Constructor
** @nam4rule Ini Constructor with initial values
**
** @argrule Ini dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvdatabaseadaptorNewIni *******************************************
**
** Constructor for an Ensembl Genetic Variation Database Adaptor with
** initial values.
**
** Ensembl Database Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Database Adaptor connected to a particular database
** is sufficient to instantiate any number of Ensembl Object Adaptors from the
** database. Each Ensembl Object Adaptor will have a weak reference to the
** Ensembl Database Adaptor that instantiated it. Therefore, Ensembl Database
** Adaptors should not be instantiated directly, but rather obtained from the
** Ensembl Registry, which will in turn call this function if neccessary.
**
** @see ensRegistryGetGvdatabaseadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvdatabaseadaptorNewIni(EnsPDatabaseadaptor dba)
{
    EnsPGvdatabaseadaptor gvdba = NULL;

    if (!dba)
        return NULL;

    if (ensDatabaseadaptorGetGroup(dba) !=
        ensEDatabaseadaptorGroupGeneticVariation)
        return NULL;

    AJNEW0(gvdba);

    gvdba->Adaptor = dba;
    gvdba->Failedvariations = ajFalse;

    return gvdba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Database Adaptor object.
**
** @fdata [EnsPGvdatabaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Database Adaptor
**
** @argrule * Pgvdba [EnsPGvdatabaseadaptor*]
** Ensembl Genetic Variation Database Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvdatabaseadaptorDel **********************************************
**
** Default destructor for an Ensembl Genetic Variation Database Adaptor.
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
** @param [d] Pgvdba [EnsPGvdatabaseadaptor*]
** Ensembl Genetic Variation Database Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvdatabaseadaptorDel(EnsPGvdatabaseadaptor *Pgvdba)
{
    EnsPGvdatabaseadaptor pthis = NULL;

    if (!Pgvdba)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvdatabaseadaptorDel"))
    {
        ajDebug("ensGvdatabaseadaptorDel\n"
                "  *Pgvdba %p\n",
                *Pgvdba);

        ensGvdatabaseadaptorTrace(*Pgvdba, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvdba)
        return;

    pthis = *Pgvdba;

    AJFREE(pthis);

    *Pgvdba = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Database Adaptor object.
**
** @fdata [EnsPGvdatabaseadaptor]
**
** @nam3rule Get Return Ensembl Database Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Failedvariations Return the failed variations attribute
**
** @argrule * gvdba [const EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Failedvariations [AjBool] Failed variations attribute or ajFalse
**
** @fcategory use
******************************************************************************/




/* @func ensGvdatabaseadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Database Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::db
** @param [r] gvdba [const EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvdatabaseadaptorGetDatabaseadaptor(
    const EnsPGvdatabaseadaptor gvdba)
{
    return (gvdba) ? gvdba->Adaptor : NULL;
}




/* @func ensGvdatabaseadaptorGetFailedvariations ******************************
**
** Get the failed variations member of an
** Ensembl Genetic Variation Database Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::include_failed_variations
** @param [r] gvdba [const EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
**
** @return [AjBool] Failed variation attribute or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorGetFailedvariations(
    const EnsPGvdatabaseadaptor gvdba)
{
    return (gvdba) ? gvdba->Failedvariations : ajFalse;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Database Adaptor object.
**
** @fdata [EnsPGvdatabaseadaptor]
**
** @nam3rule Set Set one member of an
** Ensembl Genetic Variation Database Adaptor
** @nam4rule Databaseadaptor Set the Ensembl Database Adaptor
** @nam4rule Failedvariations Set the failed variations attribute
**
** @argrule * gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor object
** @argrule Databaseadaptor dba [EnsPDatabaseadaptor]
** Ensembl Database Adaptor
** @argrule Failedvariations fv [AjBool] Failed variations attribute
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvdatabaseadaptorSetDatabaseadaptor *******************************
**
** Set the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Database Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::db
** @param [u] gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorSetDatabaseadaptor(EnsPGvdatabaseadaptor gvdba,
                                              EnsPDatabaseadaptor dba)
{
    if (!gvdba)
        return ajFalse;

    gvdba->Adaptor = dba;

    return ajTrue;
}




/* @func ensGvdatabaseadaptorSetFailedvariations ******************************
**
** Set the failed variation member of an
** Ensembl Genetic Variation Database Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::include_failed_variations
** @param [u] gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [r] fv [AjBool] Failed variations attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorSetFailedvariations(EnsPGvdatabaseadaptor gvdba,
                                               AjBool fv)
{
    if (!gvdba)
        return ajFalse;

    gvdba->Failedvariations = fv;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Database Adaptor object.
**
** @fdata [EnsPGvdatabaseadaptor]
**
** @nam3rule Trace Report Ensembl Genetic Variation Database Adaptor
** members to debug file
**
** @argrule Trace gvdba [const EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvdatabaseadaptorTrace ********************************************
**
** Trace an Ensembl Genetic Variation Database Adaptor.
**
** @param [r] gvdba [const EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorTrace(const EnsPGvdatabaseadaptor gvdba,
                                 ajuint level)
{
    AjPStr indent = NULL;

    if (!gvdba)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvdatabaseadaptorTrace %p\n"
            "%S  Databaseadaptor %p\n"
            "%S  Failedvariations '%B'\n",
            indent, gvdba,
            indent, gvdba->Adaptor,
            indent, gvdba->Failedvariations);

    ensDatabaseadaptorTrace(gvdba->Adaptor, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to
** Ensembl Genetic Variation Database Adaptor objects.
**
** @fdata [EnsPGvdatabaseadaptor]
**
** @nam3rule Failedallelesconstraint Retrieve an SQL constraint for
** failed Ensembl Genetic Variation Allele object(s)
** @nam3rule Failedstructuralsconstraint Retrieve an SQL constraint for
** failed Ensembl Genetic Variation Structural Variation object(s)
** @nam3rule Failedvariationsconstraint Retrieve an SQL constraint for
** failed Ensembl Genetic Variation Variation object(s)
**
** @argrule * gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @argrule Failedallelesconstraint tablename [const AjPStr] SQL table name
** @argrule Failedallelesconstraint Pconstraint [AjPStr*] SQL constraint
** @argrule Failedstructuralsconstraint tablename [const AjPStr] SQL table name
** @argrule Failedstructuralsconstraint Pconstraint [AjPStr*] SQL constraint
** @argrule Failedvariationsconstraint tablename [const AjPStr] SQL table name
** @argrule Failedvariationsconstraint Pconstraint [AjPStr*] SQL constraint
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvdatabaseadaptorFailedallelesconstraint **************************
**
** API-internal method for getting the constraint to filter out failed
** Ensembl Genetic Variation Allele objects.
** Assumes that the "failed_variation" SQL table has been
** (left) joined to the SQL statement and that the SQL table name is either
** supplied or equals "failed_variation".
**
** The caller is responsible for deleting the AJAX String
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::
** _exclude_failed_alleles_constraint
** @param [u] gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [rN] tablename [const AjPStr] SQL table name
** @param [u] Pconstraint [AjPStr*] SQL constraint
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorFailedallelesconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr *Pconstraint)
{
    char *txttablename = NULL;

    if (!gvdba)
        return ajFalse;

    if (!Pconstraint)
        return ajFalse;

    if (!*Pconstraint)
        *Pconstraint = ajStrNew();

    if (gvdba->Failedvariations)
    {
        if ((tablename != NULL) && (ajStrGetLen(tablename)))
        {
            ensDatabaseadaptorEscapeC(gvdba->Adaptor,
                                      &txttablename,
                                      tablename);

            ajStrAssignC(Pconstraint, txttablename);

            ajCharDel(&txttablename);
        }
        else
            ajStrAssignC(Pconstraint, "failed_variation");

        ajStrAppendC(Pconstraint, ".allele_id IS NULL");
    }
    else
        ajStrAssignC(Pconstraint, "1");

    return ajTrue;
}




/* @func ensGvdatabaseadaptorFailedstructuralsconstraint **********************
**
** API-internal method for getting the constraint to filter out failed
** Ensembl Genetic Variation Structural Variation objects.
** Assumes that the "failed_structural_variation" SQL table has been
** (left) joined to the SQL statement and that the SQL table name is either
** supplied or equals "failed_structural_variation".
**
** The caller is responsible for deleting the AJAX String
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::
** _exclude_failed_structural_variations_constraint
** @param [u] gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [rN] tablename [const AjPStr] SQL table name
** @param [u] Pconstraint [AjPStr*] SQL constraint
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorFailedstructuralsconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr *Pconstraint)
{
    char *txttablename = NULL;

    if (!gvdba)
        return ajFalse;

    if (!Pconstraint)
        return ajFalse;

    if (!*Pconstraint)
        *Pconstraint = ajStrNew();

    if (gvdba->Failedvariations)
    {
        if ((tablename != NULL) && (ajStrGetLen(tablename)))
        {
            ensDatabaseadaptorEscapeC(gvdba->Adaptor,
                                      &txttablename,
                                      tablename);

            ajStrAssignC(Pconstraint, txttablename);

            ajCharDel(&txttablename);
        }
        else
            ajStrAssignC(Pconstraint, "failed_structural_variation");

        ajStrAppendC(Pconstraint, ".structural_variation_id IS NULL");
    }
    else
        ajStrAssignC(Pconstraint, "1");

    return ajTrue;
}




/* @func ensGvdatabaseadaptorFailedvariationsconstraint ***********************
**
** API-internal method for getting the constraint to filter out failed
** Ensembl Genetic Variation Variation objects.
** Assumes that the "failed_variation" SQL table has been
** (left) joined to the SQL statement and that the SQL table name is either
** supplied or equals "failed_variation".
**
** The caller is responsible for deleting the AJAX String
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor::
** _exclude_failed_variations_constraint
** @param [u] gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [rN] tablename [const AjPStr] SQL table name
** @param [u] Pconstraint [AjPStr*] SQL constraint
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvdatabaseadaptorFailedvariationsconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr *Pconstraint)
{
    char *txttablename = NULL;

    if (!gvdba)
        return ajFalse;

    if (!Pconstraint)
        return ajFalse;

    if (!*Pconstraint)
        *Pconstraint = ajStrNew();

    if (gvdba->Failedvariations)
    {
        if ((tablename != NULL) && (ajStrGetLen(tablename)))
        {
            ensDatabaseadaptorEscapeC(gvdba->Adaptor,
                                      &txttablename,
                                      tablename);

            ajStrAssignC(Pconstraint, txttablename);

            ajCharDel(&txttablename);
        }
        else
            ajStrAssignC(Pconstraint, "failed_variation");

        ajStrAppendC(Pconstraint, ".variation_id IS NULL");
    }
    else
        ajStrAssignC(Pconstraint, "1");

    return ajTrue;
}

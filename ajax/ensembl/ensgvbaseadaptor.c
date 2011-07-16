/* @source Ensembl Genetic Variation Base Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:55:02 $ by $Author: mks $
** @version $Revision: 1.3 $
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

#include "ensgvbaseadaptor.h"
#include "ensgvdatabaseadaptor.h"




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




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensgvdatabaseadaptor ******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor ****
**
** @nam2rule Gvbaseadaptor Functions for manipulating
** Ensembl Genetic Variation Base Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor
** @cc CVS Revision: 1.3
** @cc CVS Tag: branch-ensembl-61
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Base Adaptor
** by pointer. It is the responsibility of the user to first destroy any
** previous Ensembl Genetic Variation Base Adaptor.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPGvbaseadaptor]
**
** @nam3rule New Constructor with initial values
**
** @argrule New gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @argrule New Ptables [const char* const*] Table names
** @argrule New Pcolumns [const char* const*] Column names
** @argrule New leftjoin [const EnsPBaseadaptorLeftjoin] LEFT JOIN statements
** @argrule New condition [const char*] SQL SELECT default condition
** @argrule New final [const char*] SQL SELECT final condition
** @argrule New Fstatement [AjBool function] Statement function address
**
** @valrule * [EnsPGvbaseadaptor] Ensembl Base Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvbaseadaptorNew **************************************************
**
** Default constructor for an Ensembl Genetic Variation Base Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor::new
** @param [u] gvdba [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @param [r] Ptables [const char* const*] Table names
** @param [r] Pcolumns [const char* const*] Column names
** @param [r] leftjoin [const EnsPBaseadaptorLeftjoin] LEFT JOIN conditions
** @param [r] condition [const char*] SQL SELECT default condition
** @param [r] final [const char*] SQL SELECT final condition
** @param [f] Fstatement [AjBool function] Statement function address
**
** @return [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor or NULL
** @@
******************************************************************************/

EnsPGvbaseadaptor ensGvbaseadaptorNew(
    EnsPGvdatabaseadaptor gvdba,
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

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvbaseadaptor gvba = NULL;

    if(!gvdba)
        return NULL;

    if(!Pcolumns)
        return NULL;

    if(!Ptables)
        return NULL;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvdba);

    if(!dba)
        return NULL;

    ba = ensBaseadaptorNew(dba,
                           Ptables,
                           Pcolumns,
                           leftjoin,
                           condition,
                           final,
                           Fstatement);

    if(!ba)
        return NULL;

    AJNEW0(gvba);

    gvba->Adaptor     = gvdba;
    gvba->Baseadaptor = ba;

    return gvba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Base Adaptor object.
**
** @fdata [EnsPGvbaseadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Base Adaptor object
**
** @argrule * Pgvba [EnsPGvbaseadaptor*] Ensembl Genetic Variation Base Adaptor
** object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvbaseadaptorDel **************************************************
**
** Default destructor for an Ensembl Genetic Variation Base Adaptor.
**
** @param [d] Pgvba [EnsPGvbaseadaptor*] Ensembl Genetic Variation Base Adaptor
** object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvbaseadaptorDel(EnsPGvbaseadaptor* Pgvba)
{
    EnsPGvbaseadaptor pthis = NULL;

    if(!Pgvba)
        return;

    if(!*Pgvba)
        return;

    pthis = *Pgvba;

    ensBaseadaptorDel(&pthis->Baseadaptor);

    AJFREE(pthis);

    *Pgvba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Base Adaptor object.
**
** @fdata [EnsPGvbaseadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Base Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Failedvariations Return the failed variations attribute
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
**
** @argrule * gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Failedvariations [AjBool] Failed variations attribute or ajFalse
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensGvbaseadaptorGetBaseadaptor ***************************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensGvbaseadaptorGetBaseadaptor(
    const EnsPGvbaseadaptor gvba)
{
    if(!gvba)
        return NULL;

    return gvba->Baseadaptor;
}




/* @func ensGvbaseadaptorGetDatabaseadaptor ***********************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvbaseadaptorGetDatabaseadaptor(
    const EnsPGvbaseadaptor gvba)
{
    if(!gvba)
        return NULL;

    return ensGvdatabaseadaptorGetDatabaseadaptor(gvba->Adaptor);
}




/* @func ensGvbaseadaptorGetFailedvariations **********************************
**
** Get the failed variations element of the
** Ensembl Genetic Variation Database Adaptor element of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [AjBool] Failed variations attribute or ajFalse
** @@
******************************************************************************/

AjBool ensGvbaseadaptorGetFailedvariations(
    const EnsPGvbaseadaptor gvba)
{
    if(!gvba)
        return ajFalse;

    return ensGvdatabaseadaptorGetFailedvariations(gvba->Adaptor);
}




/* @func ensGvbaseadaptorGetGvdatabaseadaptor *********************************
**
** Get the Ensembl Genetic Variation Database Adaptor element of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [EnsPGvdatabaseadaptor] Ensembl Genetic Variation Database Adaptor
** or NULL
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvbaseadaptorGetGvdatabaseadaptor(
    const EnsPGvbaseadaptor gvba)
{
    if(!gvba)
        return NULL;

    return gvba->Adaptor;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Objects from an
** Ensembl database.
**
** @fdata [EnsPGvbaseadaptor]
**
** @nam3rule Fetch       Fetch Ensembl Genetic Variation Object(s)
** @nam4rule All         Fetch all Ensembl Genetic Variation Objects
** @nam4rule Allby       Fetch all Ensembl Genetic Variation Objects
** matching a criterion
** @nam4rule Allsomatic  Fetch all somatic Ensembl Genetic Variation Objects
** @nam4rule By          Fetch one Ensembl Genetic Variation Object
** matching a criterion
**
** @argrule * gvba [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor
** @argrule All objects [AjPList]
** AJAX List of Ensembl Genetic Variation Objects
** @argrule Allby objects [AjPList]
** AJAX List of Ensembl Genetic Variation Objects
** @argrule Allsomatic objects [AjPList]
** AJAX List of Ensembl Genetic Variation Objects
** @argrule By Pobject [void**] Ensembl Genetic Variation Object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvbaseadaptorFetchAll *********************************************
**
** Generic function to fetch all Ensembl Genetic Variation Objects via
** an Ensembl Genetic Variation Base Adaptor.
**
** Please note that it is probably not a good idea to use this function on
** *very* large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor::fetch_all
** @param [u] gvba [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Genetic Variation Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvbaseadaptorFetchAll(EnsPGvbaseadaptor gvba,
                                AjPList objects)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!gvba)
        return ajFalse;

    if(!objects)
        return ajFalse;

    constraint = ajStrNewC("sample.somatic = 0");

    result = ensBaseadaptorFetchAllbyConstraint(gvba->Baseadaptor,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                objects);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvbaseadaptorFetchAllsomatic **************************************
**
** Generic function to fetch all somatic Ensembl Genetic Variation Objects via
** an Ensembl Genetic Variation Base Adaptor.
**
** Please note that it is probably not a good idea to use this function on
** *very* large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor::fetch_all_somatic
** @param [u] gvba [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor
** @param [u] objects [AjPList] AJAX List of Ensembl Genetic Variation Objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvbaseadaptorFetchAllsomatic(EnsPGvbaseadaptor gvba,
                                       AjPList objects)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!gvba)
        return ajFalse;

    if(!objects)
        return ajFalse;

    constraint = ajStrNewC("sample.somatic = 1");

    result = ensBaseadaptorFetchAllbyConstraint(gvba->Baseadaptor,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                objects);

    ajStrDel(&constraint);

    return result;
}

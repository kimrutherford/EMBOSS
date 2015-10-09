/* @source ensgvbaseadaptor ***************************************************
**
** Ensembl Genetic Variation Base Adaptor functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.16 $
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

#include "ensgvbaseadaptor.h"
#include "ensgvdatabaseadaptor.h"
#include "ensmetainformation.h"




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




/* @datasection [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor ****
**
** @nam2rule Gvbaseadaptor Functions for manipulating
** Ensembl Genetic Variation Base Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvbaseadaptor ensGvbaseadaptorNew(
    EnsPGvdatabaseadaptor gvdba,
    const char* const* Ptables,
    const char* const* Pcolumns,
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

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvbaseadaptor gvba = NULL;

    if (!gvdba)
        return NULL;

    if (!Ptables)
        return NULL;

    if (!Pcolumns)
        return NULL;

    if (!Fstatement)
        return NULL;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvdba);

    if (!dba)
        return NULL;

    ba = ensBaseadaptorNew(dba,
                           Ptables,
                           Pcolumns,
                           leftjoin,
                           condition,
                           final,
                           Fstatement);

    if (!ba)
        return NULL;

    AJNEW0(gvba);

    gvba->Adaptor     = gvdba;
    gvba->Baseadaptor = ba;

    return gvba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Base Adaptor object.
**
** @fdata [EnsPGvbaseadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Base Adaptor
**
** @argrule * Pgvba [EnsPGvbaseadaptor*]
** Ensembl Genetic Variation Base Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvbaseadaptorDel **************************************************
**
** Default destructor for an Ensembl Genetic Variation Base Adaptor.
**
** @param [d] Pgvba [EnsPGvbaseadaptor*]
** Ensembl Genetic Variation Base Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvbaseadaptorDel(EnsPGvbaseadaptor *Pgvba)
{
    EnsPGvbaseadaptor pthis = NULL;

    if (!Pgvba)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvbaseadaptorDel"))
        ajDebug("ensGvbaseadaptorDel\n"
                "  *Pgvba %p\n",
                *Pgvba);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvba)
        return;

    pthis = *Pgvba;

    ensBaseadaptorDel(&pthis->Baseadaptor);

    AJFREE(pthis);

    *Pgvba = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
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
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvbaseadaptorGetBaseadaptor(
    const EnsPGvbaseadaptor gvba)
{
    return (gvba) ? gvba->Baseadaptor : NULL;
}




/* @func ensGvbaseadaptorGetDatabaseadaptor ***********************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvbaseadaptorGetDatabaseadaptor(
    const EnsPGvbaseadaptor gvba)
{
    return (gvba) ?
        ensGvdatabaseadaptorGetDatabaseadaptor(gvba->Adaptor) : NULL;
}




/* @func ensGvbaseadaptorGetFailedvariations **********************************
**
** Get the failed variations member of the
** Ensembl Genetic Variation Database Adaptor member of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [AjBool] Failed variations attribute or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvbaseadaptorGetFailedvariations(
    const EnsPGvbaseadaptor gvba)
{
    return (gvba) ?
        ensGvdatabaseadaptorGetFailedvariations(gvba->Adaptor) : ajFalse;
}




/* @func ensGvbaseadaptorGetGvdatabaseadaptor *********************************
**
** Get the Ensembl Genetic Variation Database Adaptor member of an
** Ensembl Genetic Variation Base Adaptor.
**
** @param [r] gvba [const EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [EnsPGvdatabaseadaptor] Ensembl Genetic Variation Database Adaptor
** or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvbaseadaptorGetGvdatabaseadaptor(
    const EnsPGvbaseadaptor gvba)
{
    return (gvba) ? gvba->Adaptor : NULL;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Base Adaptor object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPGvbaseadaptor]
**
** @nam3rule Load Return Ensembl Genetic Variation Base Adaptor attribute(s)
** loaded on demand
** @nam4rule Ploidy Return the ploidy
**
** @argrule * gvba [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor
**
** @valrule Ploidy [ajuint] Ploidy or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvbaseadaptorLoadPloidy *******************************************
**
** Load the ploidy member via an Ensembl Genetic Variation Base Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor::ploidy
** @param [u] gvba [EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor
**
** @return [ajuint] Ploidy or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvbaseadaptorLoadPloidy(
    EnsPGvbaseadaptor gvba)
{
    AjBool result = AJFALSE;

    AjPStr key   = NULL;
    AjPStr value = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMetainformationadaptor mia = NULL;

    if (!gvba)
        return 0U;

    if (gvba->Ploidy)
        return gvba->Ploidy;

    dba = ensGvbaseadaptorGetDatabaseadaptor(gvba);

    mia = ensRegistryGetMetainformationadaptor(dba);

    key   = ajStrNewC("ploidy");
    value = ajStrNew();

    ensMetainformationadaptorRetrieveValue(mia, key, &value);

    result = ajStrToUint(value, &gvba->Ploidy);

    if (!result)
        ajWarn("ensGvbaseadaptorFetchPloidy could not parse value '%S' for "
               "for meta key \"ploidy\" as AJAX unsigned integer.\n", value);

    ajStrDel(&key);
    ajStrDel(&value);

    if (!result)
        return 0U;

    return gvba->Ploidy;

}

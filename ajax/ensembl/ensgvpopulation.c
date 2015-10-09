/* @source ensgvpopulation ****************************************************
**
** Ensembl Genetic Variation Population functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.53 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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
#include "ensgvgenotype.h"
#include "ensgvindividual.h"
#include "ensgvpopulation.h"
#include "ensgvsample.h"
#include "ensgvvariation.h"
#include "ensmetainformation.h"
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

/* @conststatic gvpopulationadaptorKTables ************************************
**
** Array of Ensembl Genetic Variation Population Adaptor SQL table names
**
******************************************************************************/

static const char *gvpopulationadaptorKTables[] =
{
    "sample",
    "population",
    (const char *) NULL
};




/* @conststatic gvpopulationadaptorKColumns ***********************************
**
** Array of Ensembl Genetic Variation Population Adaptor SQL column names
**
******************************************************************************/

static const char *gvpopulationadaptorKColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "sample.display",
    (const char *) NULL
};




/* @conststatic gvpopulationadaptorKDefaultcondition **************************
**
** Ensembl Genetic Variation Population Adaptor SQL default condition
**
******************************************************************************/

static const char *gvpopulationadaptorKDefaultcondition =
    "sample.sample_id = population.sample_id";




/* @conststatic gvpopulationgenotypeadaptorKTables ****************************
**
** Array of Ensembl Genetic Variation Population Genotype Adaptor
** SQL table names
**
******************************************************************************/

static const char *const gvpopulationgenotypeadaptorKTables[] =
{
    "population_genotype",
    "failed_variation",
    (const char *) NULL
};




/* @conststatic gvpopulationgenotypeadaptorKColumns ***************************
**
** Array of Ensembl Genetic Variation Population Genotype Adaptor
** SQL column names
**
******************************************************************************/

static const char *const gvpopulationgenotypeadaptorKColumns[] =
{
    "population_genotype.population_genotype_id",
    "population_genotype.variation_id",
    "population_genotype.subsnp_id",
    "population_genotype.sample_id",
    "population_genotype.genotype_code_id",
    "population_genotype.count",
    "population_genotype.frequency",
    (const char *) NULL
};




/* @conststatic gvpopulationgenotypeadaptorKLeftjoin **************************
**
** Array of Ensembl Genetic Variation Population Genotype Adaptor
** SQL left join conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin gvpopulationgenotypeadaptorKLeftjoin[] =
{
    {
        "failed_variation",
        "population_genotype.variation_id = failed_variation.variation_id"
    },
    {(const char *) NULL, (const char *) NULL}
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool gvpopulationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvps);

static void gvpopulationgenotypeadaptorLinkGvgenotypecode(
    void **Pkey,
    void **Pvalue,
    void *cl);

static void gvpopulationgenotypeadaptorLinkGvpopulation(
    void **Pkey,
    void **Pvalue,
    void *cl);

static void gvpopulationgenotypeadaptorLinkGvvariation(
    void **Pkey,
    void **Pvalue,
    void *cl);

static AjBool gvpopulationgenotypeadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvpgs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvpopulation ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvpopulation] Ensembl Genetic Variation Population *******
**
** @nam2rule Gvpopulation Functions for manipulating
** Ensembl Genetic Variation Population objects
**
** @cc Bio::EnsEMBL::Variation::Population
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Population by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Population. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule Ini gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @argrule Ini identifier [ajuint]
** SQL database-internal identifier
** @argrule Ini gvs [EnsPGvsample]
** Ensembl Genetic Variation Sample
** @argrule Ini subpopulations [AjPList]
** AJAX List of Ensembl Genetic Variation (Sub-) Population objects
** @argrule Ref gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
**
** @valrule * [EnsPGvpopulation]
** Ensembl Genetic Variation Population or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvpopulationNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNewCpy(const EnsPGvpopulation gvp)
{
    AjIList iter = NULL;

    EnsPGvpopulation pthis  = NULL;
    EnsPGvpopulation subgvp = NULL;

    if (!gvp)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvp->Identifier;
    pthis->Adaptor    = gvp->Adaptor;
    pthis->Gvsample   = ensGvsampleNewRef(gvp->Gvsample);

    /*
    ** Copy the AJAX List of Ensembl Genetic Variation (Sub-) Population
    ** objects.
    */

    if (ajListGetLength(gvp->Subgvpopulations))
    {
        pthis->Subgvpopulations = ajListNew();

        iter = ajListIterNew(gvp->Subgvpopulations);

        while (!ajListIterDone(iter))
        {
            subgvp = (EnsPGvpopulation) ajListIterGet(iter);

            ajListPushAppend(pthis->Subgvpopulations,
                             (void *) ensGvpopulationNewRef(subgvp));
        }

        ajListIterDel(&iter);
    }

    return pthis;
}




/* @func ensGvpopulationNewIni ************************************************
**
** Constructor for an Ensembl Genetic Variation Population with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @cc Bio::EnsEMBL::Variation::Population::new
** @param [u] subpopulations [AjPList]
** AJAX List of Ensembl Genetic Variation (Sub-) Population objects
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNewIni(EnsPGvpopulationadaptor gvpa,
                                       ajuint identifier,
                                       EnsPGvsample gvs,
                                       AjPList subpopulations)
{
    AjIList iter = NULL;

    EnsPGvpopulation gvp    = NULL;
    EnsPGvpopulation subgvp = NULL;

    if (!gvs)
        return NULL;

    AJNEW0(gvp);

    gvp->Use        = 1U;
    gvp->Identifier = identifier;
    gvp->Adaptor    = gvpa;
    gvp->Gvsample   = ensGvsampleNewRef(gvs);

    if (ajListGetLength(subpopulations))
    {
        gvp->Subgvpopulations = ajListNew();

        iter = ajListIterNew(subpopulations);

        while (!ajListIterDone(iter))
        {
            subgvp = (EnsPGvpopulation) ajListIterGet(iter);

            ajListPushAppend(gvp->Subgvpopulations,
                             (void *) ensGvpopulationNewRef(subgvp));
        }

        ajListIterDel(&iter);
    }

    return gvp;
}




/* @func ensGvpopulationNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNewRef(EnsPGvpopulation gvp)
{
    if (!gvp)
        return NULL;

    gvp->Use++;

    return gvp;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Population
**
** @argrule * Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvpopulationDel ***************************************************
**
** Default destructor for an Ensembl Genetic Variation Population.
**
** @param [d] Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvpopulationDel(EnsPGvpopulation *Pgvp)
{
    EnsPGvpopulation subgvp = NULL;
    EnsPGvpopulation pthis = NULL;

    if (!Pgvp)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvpopulationDel"))
    {
        ajDebug("ensGvpopulationDel\n"
                "  *Pgvp %p\n",
                *Pgvp);

        ensGvpopulationTrace(*Pgvp, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvp)
        return;

    pthis = *Pgvp;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pgvp = NULL;

        return;
    }

    ensGvsampleDel(&pthis->Gvsample);

    while (ajListPop(pthis->Subgvpopulations, (void **) &subgvp))
        ensGvpopulationDel(&subgvp);

    ajListFree(&pthis->Subgvpopulations);

    AJFREE(pthis);

    *Pgvp = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Get Return Genetic Variation Population attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Population Adaptor
** @nam4rule Gvsample Return the Ensembl Genetic Variation Sample
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Subgvpopulations Return the AJAX List of Ensembl Genetic
** Variation (Sub-) Population objects
**
** @argrule * gvp [const EnsPGvpopulation] Genetic Variation Population
**
** @valrule Adaptor [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor or NULL
** @valrule Gvsample [EnsPGvsample]
** Ensembl Genetic Variation Sample or NULL
** @valrule Identifier [ajuint]
** SQL database-internal identifier or 0U
** @valrule Subgvpopulations [const AjPList]
** AJAX List of Ensembl Genetic Variation (Sub-) Population objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationGetAdaptor ********************************************
**
** Get the Ensembl Genetic Variation Population Adaptor member of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensGvpopulationGetAdaptor(const EnsPGvpopulation gvp)
{
    return (gvp) ? gvp->Adaptor : NULL;
}




/* @func ensGvpopulationGetGvsample *******************************************
**
** Get the Ensembl Genetic Variation Sample member of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvsample ensGvpopulationGetGvsample(const EnsPGvpopulation gvp)
{
    return (gvp) ? gvp->Gvsample : NULL;
}




/* @func ensGvpopulationGetIdentifier *****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvpopulationGetIdentifier(const EnsPGvpopulation gvp)
{
    return (gvp) ? gvp->Identifier : 0U;
}




/* @func ensGvpopulationGetSubgvpopulations ***********************************
**
** Get the AJAX List of Ensembl Genetic Variation (Sub-) Population objects
** of an Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [const AjPList]
** AJAX List of Ensembl Genetic Variation (Sub-) Population objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGvpopulationGetSubgvpopulations(const EnsPGvpopulation gvp)
{
    return (gvp) ? gvp->Subgvpopulations : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Set Set one member of a Genetic Variation Population
** @nam4rule Adaptor Set the Ensembl Genetic Variation Population Adaptor
** @nam4rule Gvsample Set the Ensembl Genetic Variation Sample
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule Adaptor gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @argrule Gvsample gvs [EnsPGvsample]
** Ensembl Genetic Variation Sample
** @argrule Identifier identifier [ajuint]
** SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvpopulationSetAdaptor ********************************************
**
** Set the Ensembl Genetic Variation Population Adaptor member of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvpopulationSetAdaptor(EnsPGvpopulation gvp,
                                 EnsPGvpopulationadaptor gvpa)
{
    if (!gvp)
        return ajFalse;

    gvp->Adaptor = gvpa;

    return ajTrue;
}




/* @func ensGvpopulationSetGvsample *******************************************
**
** Set the Ensembl Genetic Variation Sample member of an
** Ensembl Genetic Variation Population.
**
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvpopulationSetGvsample(EnsPGvpopulation gvp, EnsPGvsample gvs)
{
    if (!gvp)
        return ajFalse;

    ensGvsampleDel(&gvp->Gvsample);

    gvp->Gvsample = ensGvsampleNewRef(gvs);

    return ajTrue;
}




/* @func ensGvpopulationSetIdentifier *****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvpopulationSetIdentifier(EnsPGvpopulation gvp, ajuint identifier)
{
    if (!gvp)
        return ajFalse;

    gvp->Identifier = identifier;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Genetic Variation Population
** object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Add Add one object to an Ensembl Genetic Variation Population
** @nam4rule Subgvpopulation Add an Ensembl Genetic Variation (Sub-) Population
**
** @argrule * gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @argrule Subgvpopulation subgvp [EnsPGvpopulation]
** Ensembl Genetic Variation (Sub-) Population
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvpopulationAddSubgvpopulation ************************************
**
** Add an Ensembl Genetic Variation (Sub-) Population to an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Variation::Population::add_sub_Population
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] subgvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationAddSubgvpopulation(EnsPGvpopulation gvp,
                                         EnsPGvpopulation subgvp)
{
    if (!gvp)
        return ajFalse;

    if (!subgvp)
        return ajFalse;

    if (!gvp->Subgvpopulations)
        gvp->Subgvpopulations = ajListNew();

    ajListPushAppend(gvp->Subgvpopulations,
                     (void *) ensGvpopulationNewRef(subgvp));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Trace Report Ensembl Genetic Variation Population members to
**                 debug file
**
** @argrule Trace gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvpopulationTrace *************************************************
**
** Trace an Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvpopulationTrace(const EnsPGvpopulation gvp, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPGvpopulation subgvp = NULL;

    if (!gvp)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvpopulationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Sample %p\n"
            "%S  Subgvpopulations %p\n",
            indent, gvp,
            indent, gvp->Use,
            indent, gvp->Identifier,
            indent, gvp->Adaptor,
            indent, gvp->Gvsample,
            indent, gvp->Subgvpopulations);

    ensGvsampleTrace(gvp->Gvsample, level + 1);

    /*
    ** Trace the AJAX List of Ensembl Genetic Variation (Sub-) Population
    ** objects.
    */

    if (gvp->Subgvpopulations)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Genetic Variation (Sub-) Population objects\n",
                indent, gvp->Subgvpopulations);

        iter = ajListIterNewread(gvp->Subgvpopulations);

        while (!ajListIterDone(iter))
        {
            subgvp = (EnsPGvpopulation) ajListIterGet(iter);

            ensGvpopulationTrace(subgvp, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Population values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvpopulationCalculateMemsize **************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvpopulationCalculateMemsize(const EnsPGvpopulation gvp)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPGvpopulation subgvp = NULL;

    if (!gvp)
        return 0;

    size += sizeof (EnsOGvpopulation);

    size += ensGvsampleCalculateMemsize(gvp->Gvsample);

    /*
    ** Summarise the AJAX List of Ensembl Genetic Variation
    ** (Sub-) Population objects.
    */

    iter = ajListIterNewread(gvp->Subgvpopulations);

    while (!ajListIterDone(iter))
    {
        subgvp = (EnsPGvpopulation) ajListIterGet(iter);

        size += ensGvpopulationCalculateMemsize(subgvp);
    }

    ajListIterDel(&iter);

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an
** Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Population values
** @nam4rule All Fetch all objects
** @nam5rule Gvindividuals Fetch all
** Ensembl Genetic Variation Individual objects
** @nam5rule Synonyms Fetch all synonyms
**
** @argrule AllGvindividuals gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllGvindividuals gvis [AjPList]
** AJAX List of Ensembl Genetic Variation Individual objects
** @argrule AllSynonyms gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllSynonyms source [const AjPStr]
** Source
** @argrule AllSynonyms synonyms [AjPList]
** AJAX List of (synonym) AJAX String objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvpopulationFetchAllGvindividuals *********************************
**
** Fetch all Ensembl Genetic Variation Individual objectss of an
** Ensembl Genetic Variation Population.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Individual objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::Population::get_all_Individuals
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvis [AjPList]
** AJAX List of Ensembl Genetic Variation Individual objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationFetchAllGvindividuals(EnsPGvpopulation gvp,
                                            AjPList gvis)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvindividualadaptor gvia = NULL;

    if (!gvp)
        return ajFalse;

    if (!gvis)
        return ajFalse;

    if (!gvp->Adaptor)
        return ajTrue;

    dba = ensGvpopulationadaptorGetDatabaseadaptor(gvp->Adaptor);

    gvia = ensRegistryGetGvindividualadaptor(dba);

    return ensGvindividualadaptorFetchAllbyGvpopulation(gvia,
                                                        gvp,
                                                        gvis);
}




/* @func ensGvpopulationFetchAllSynonyms **************************************
**
** Fetch all (synonym) AJAX String objects of an
** Ensembl Genetic Variation Population.
**
** The caller is responsible for deleting the AJAX String objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::Population::get_all_synonyms
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] source [const AjPStr] Source
** @param [u] synonyms [AjPList] AJAX List of (synonym) AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationFetchAllSynonyms(EnsPGvpopulation gvp,
                                       const AjPStr source,
                                       AjPList synonyms)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsampleadaptor gvsa = NULL;

    if (!gvp)
        return ajFalse;

    if (!synonyms)
        return ajFalse;

    if (!gvp->Adaptor)
        return ajTrue;

    dba = ensGvpopulationadaptorGetDatabaseadaptor(gvp->Adaptor);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    return ensGvsampleadaptorRetrieveAllSynonymsByIdentifier(gvsa,
                                                             gvp->Identifier,
                                                             source,
                                                             synonyms);
}




/* @datasection [EnsPGvpopulationadaptor] Ensembl Genetic Variation Population
** Adaptor
**
** @nam2rule Gvpopulationadaptor Functions for manipulating
** Ensembl Genetic Variation Population Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor
** @cc CVS Revision: 1.33
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic gvpopulationadaptorFetchAllbyStatement *************************
**
** Fetch all Ensembl Genetic Variation Population objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvpopulationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvps)
{
    ajuint identifier = 0U;
    ajuint ssize      = 0U;

    EnsEGvsampleDisplay esdisplay = ensEGvsampleDisplayNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr sname        = NULL;
    AjPStr sdescription = NULL;
    AjPStr sdisplay     = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvpopulation gvp         = NULL;
    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if (ajDebugTest("gvpopulationadaptorFetchAllbyStatement"))
        ajDebug("gvpopulationadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvps %p\n",
                ba,
                statement,
                am,
                slice,
                gvps);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvpa = ensRegistryGetGvpopulationadaptor(dba);

    gvsa = ensGvpopulationadaptorGetBaseadaptor(gvpa);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier   = 0;
        sname        = ajStrNew();
        ssize        = 0;
        sdescription = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &sname);
        ajSqlcolumnToUint(sqlr, &ssize);
        ajSqlcolumnToStr(sqlr, &sdescription);
        ajSqlcolumnToStr(sqlr, &sdisplay);

        gvs = ensGvsampleNewIni(gvsa,
                                identifier,
                                sname,
                                sdescription,
                                esdisplay,
                                ssize);

        gvp = ensGvpopulationNewIni(gvpa,
                                    identifier,
                                    gvs,
                                    (AjPList) NULL);

        ajListPushAppend(gvps, (void *) gvp);

        ensGvsampleDel(&gvs);

        ajStrDel(&sname);
        ajStrDel(&sdescription);
        ajStrDel(&sdisplay);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Population Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Population Adaptor. The target pointer does not
** need to be initialised to NULL, but it is good programming practice to do
** so anyway.
**
** @fdata [EnsPGvpopulationadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvpopulationadaptorNew ********************************************
**
** Default constructor for an Ensembl Genetic Variation Population Adaptor.
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
** @see ensRegistryGetGvpopulationadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensGvpopulationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        gvpopulationadaptorKTables,
        gvpopulationadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        gvpopulationadaptorKDefaultcondition,
        (const char *) NULL,
        &gvpopulationadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Population Adaptor object.
**
** @fdata [EnsPGvpopulationadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Population Adaptor
**
** @argrule * Pgvpa [EnsPGvpopulationadaptor*]
** Ensembl Genetic Variation Population Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvpopulationadaptorDel ********************************************
**
** Default destructor for an Ensembl Genetic Variation Population Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvpa [EnsPGvpopulationadaptor*]
** Ensembl Genetic Variation Population Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvpopulationadaptorDel(EnsPGvpopulationadaptor *Pgvpa)
{
    ensBaseadaptorDel(Pgvpa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Population Adaptor object.
**
** @fdata [EnsPGvpopulationadaptor]
**
** @nam3rule Get Return Genetic Variation Population Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Databaseadaptor
**
** @argrule * gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Databaseadaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationadaptorGetBaseadaptor *********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Population Adaptor.
**
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvpopulationadaptorGetBaseadaptor(
    EnsPGvpopulationadaptor gvpa)
{
    return gvpa;
}




/* @func ensGvpopulationadaptorGetDatabaseadaptor *****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Population Adaptor.
**
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvpopulationadaptorGetDatabaseadaptor(
    EnsPGvpopulationadaptor gvpa)
{
    return ensBaseadaptorGetDatabaseadaptor(gvpa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Population objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGvpopulationadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Population object(s)
** @nam4rule All Fetch all Ensembl Genetic Variation Population objects
** @nam5rule Thousandgenomes Fetch all 1000 Genomes populations
** @nam5rule Hapmap Fetch all HapMap populations
** @nam5rule Ld Fetch all for Linkage Disequilibrium
** @nam5rule Tagged Fetch all tagged
** @nam5rule Tags Fetch all tags
** @nam4rule Allby Fetch all Ensembl Genetic Variation Population objects
**                 matching a criterion
** @nam5rule Gvindividual Fetch all by an Ensembl Genetic Variation Individual
** @nam5rule Gvindividuals
** Fetch all by an AJAX List of Ensembl Genetic Variation Individual objects
** @nam5rule Identifiers Fetch all by an AJAX Table
** @nam5rule Namesearch Fetch all by a name search
** @nam5rule Synonym Fetch all by an Ensembl Genetic Variation Sample synonym
** @nam5rule Gvpopulation Fetch all by an Ensembl Genetic Variation Population
** @nam6rule Sub Fetch all by sub-population
** @nam6rule Super Fetch all by super-population
** @nam4rule By Fetch one Ensembl Genetic Variation Population object
**              matching a criterion
** @nam5rule Identifier Fetch ba a SQL database-internal identifier
** @nam5rule Name Fetch by a name
** @nam4rule Defaultld Fetch default for Linkage Disequilibrium
**
** @argrule * gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @argrule AllTagged gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Feature
** @argrule AllTags gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Feature
** @argrule All gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule AllbyGvindividual gvi [const EnsPGvindividual]
** Ensembl Genetic Variation Individual
** @argrule AllbyGvindividual gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule AllbyGvindividuals gvis [AjPList]
** AJAX List of Ensembl Genetic Variation Individual objects
** @argrule AllbyGvindividuals gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule AllbyGvpopulationSub gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllbyGvpopulationSub gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population
** @argrule AllbyGvpopulationSuper gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllbyGvpopulationSuper gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population
** @argrule AllbyIdentifiers gvps [AjPTable]
** AJAX Table of Ensembl Genetic Variation Population objects
** @argrule AllbyNamesearch name [const AjPStr]
** Ensembl Genetic Variation Population name
** @argrule AllbyNamesearch gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule AllbySynonym synonym [const AjPStr]
** Ensembl Genetic Variation Sample synonym
** @argrule AllbySynonym source [const AjPStr]
** Source
** @argrule AllbySynonym gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population object address
** @argrule ByName name [const AjPStr]
** Ensembl Genetic Variation Population name
** @argrule ByName Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population object address
** @argrule ensGvpopulationadaptorFetchDefaultld Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationadaptorFetchAllHapmap *********************************
**
** Fetch all Ensembl Genetic Variation Population objects, which are linked to
** HapMap data.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** fetch_all_HapMap_populations
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllHapmap(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps)
{
    AjPStr constraint = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    constraint = ajStrNewC("sample.name LIKE 'cshl-hapmap%'");

    ensBaseadaptorFetchAllbyConstraint(gvpa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvps);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllLd *************************************
**
** Fetch all Ensembl Genetic Variation Population objects, which can be used
** in the LD display of the pairwise LD data.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_ FIXME!
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllLd(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps)
{
    AjPStr constraint = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    constraint = ajStrNewC("sample.display = 'LD'");

    ensBaseadaptorFetchAllbyConstraint(gvpa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvps);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllTagged *********************************
**
** Fetch all tagged Ensembl Genetic Variation Population objects,
** which are linked to an Ensembl Genetic Variation Feature.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_tagged_Population
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Feature
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllTagged(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvvariationfeature gvvf,
    AjPList gvps)
{
    AjPStr statement = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvvf)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    if (!ensGvvariationfeatureGetIdentifier(gvvf))
    {
        ajDebug("ensGvpopulationadaptorFetchTagged cannot fetch a tagged "
                "Ensembl Genetic Variation Population for an "
                "Ensembl Genetic Variation Variation Feature with out an "
                "identifier.\n");

        return ajFalse;
    }

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description "
        "FROM "
        "sample, "
        "population, "
        "tagged_variation_feature "
        "WHERE "
        "sample.sample_id = population.sample_id "
        "AND "
        "population.sample_id = tagged_variation_feature.sample_id "
        "AND "
        "tagged_variation_feature.tagged_variation_feature_id = %u",
        ensGvvariationfeatureGetIdentifier(gvvf));

    gvpopulationadaptorFetchAllbyStatement(gvpa,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllTags ***********************************
**
** Fetch all Ensembl Genetic Variation Population objects in which the
** Ensembl Genetic Variation Feature is a tag.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_tag_Population
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Feature
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllTags(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvvariationfeature gvvf,
    AjPList gvps)
{
    AjPStr statement = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvvf)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    if (!ensGvvariationfeatureGetIdentifier(gvvf))
    {
        ajDebug("ensGvpopulationadaptorFetchTagged cannot fetch a tagged "
                "Ensembl Genetic Variation Population for an "
                "Ensembl Genetic Variation Variation Feature with out an "
                "identifier.\n");

        return ajFalse;
    }

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description "
        "FROM "
        "sample, "
        "population, "
        "tagged_variation_feature "
        "WHERE "
        "sample.sample_id = population.sample_id "
        "AND "
        "population.sample_id = tagged_variation_feature.sample_id "
        "AND "
        "tagged_variation_feature.variation_feature_id = %u",
        ensGvvariationfeatureGetIdentifier(gvvf));

    gvpopulationadaptorFetchAllbyStatement(gvpa,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllThousandgenomes ************************
**
** Fetch all Ensembl Genetic Variation Population objects, which are linked to
** 1000 Genomes data.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** etch_all_1KG_populations
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllThousandgenomes(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps)
{
    AjPStr constraint = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    constraint = ajStrNewC("sample.name LIKE '1000GENOMES%'");

    ensBaseadaptorFetchAllbyConstraint(gvpa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvps);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbyGvindividual *************************
**
** Fetch all Ensembl Genetic Variation Population objects by an
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_Individual
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] gvi [const EnsPGvindividual]
** Ensembl Genetic Variation Individual
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvindividual(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvindividual gvi,
    AjPList gvps)
{
    AjPStr statement = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvi)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    if (!gvi->Identifier)
    {
        ajDebug("ensGvpopulationadaptorFetchAllbyGvindividual "
                "cannot fetch Population for Individual "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'individual_population' table is
    ** required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display "
        "FROM "
        "sample, "
        "population, "
        "individual_population "
        "WHERE "
        "sample.sample_id = population.sample_id "
        "AND "
        "sample.sample_id = "
        "individual_population.population_sample_id "
        "AND "
        "individual_population.individual_sample_id = %u",
        gvi->Identifier);

    gvpopulationadaptorFetchAllbyStatement(gvpa,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbyGvindividuals ************************
**
** Fetch all Ensembl Genetic Variation Population objects by
** Ensembl Genetic Variation Individual objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_Individual_list
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [u] gvis [AjPList]
** AJAX List of Ensembl Genetic Variation Individual objects
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvindividuals(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvis,
    AjPList gvps)
{
    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'individual_population' table is
    ** required.
    */

    const char *template =
        "SELECT "
        "population.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description "
        "FROM "
        "population p, "
        "individual_population ip, "
        "sample s "
        "WHERE "
        "sample.sample_id = individual_population.population_sample_id "
        "AND "
        "sample.sample_id = population.sample_id "
        "AND "
        "individual_population.individual_sample_id IN (%s)";

    register ajuint i = 0U;

    AjIList iter = NULL;

    AjPStr csv       = NULL;
    AjPStr statement = NULL;

    EnsPGvindividual gvi = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvis)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    csv = ajStrNew();

    iter = ajListIterNew(gvis);

    while (!ajListIterDone(iter)) {

        gvi = (EnsPGvindividual) ajListIterGet(iter);

        ajFmtPrintAppS(&csv, "%u, ", ensGvindividualGetIdentifier(gvi));

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more AJAX List elements to process.
        */

        if ((((i + 1U) % ensKBaseadaptorMaximumIdentifiers) == 0) ||
            ajListIterDone(iter))
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                statement = ajFmtStr(template, csv);

                gvpopulationadaptorFetchAllbyStatement(gvpa,
                                                       statement,
                                                       (EnsPAssemblymapper) NULL,
                                                       (EnsPSlice) NULL,
                                                       gvps);

                ajStrDel(&statement);
            }

            ajStrAssignClear(&csv);
        }

        i++;
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbyGvpopulationSub **********************
**
** Fetch all Ensembl Genetic Variation Population objects by a
** sub-population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_sub_Population
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvpopulationSub(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps)
{
    AjPStr statement = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    if (!gvp->Identifier)
    {
        ajDebug("ensGvpopulationadaptorFetchAllbyGvpopulationSub "
                "cannot fetch Population for sub Population "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'population_structure' table is
    ** required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display "
        "FROM "
        "sample, "
        "population, "
        "population_structure "
        "WHERE "
        "sample.sample_id = population.sample_id "
        "AND "
        "population.sample_id = "
        "population_structure.super_population_sample_id "
        "AND "
        "population_structure.sub_population_sample_id = %u",
        gvp->Identifier);

    gvpopulationadaptorFetchAllbyStatement(gvpa,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbyGvpopulationSuper ********************
**
** Fetch all Ensembl Genetic Variation Population objects by a
** super-population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_super_Population
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvpopulationSuper(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps)
{
    AjPStr statement = NULL;

    if (!gvpa)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    if (!gvp->Identifier)
    {
        ajDebug("ensGvpopulationadaptorFetchAllbyGvpopulationSuper "
                "cannot fetch Population for super Population "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'population_structure' table is
    ** required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display "
        "FROM "
        "sample, "
        "population, "
        "population_structure "
        "WHERE "
        "sample.sample_id = population.sample_id "
        "AND "
        "population.sample_id = "
        "population_structure.sub_population_sample_id "
        "AND "
        "population_structure.super_population_sample_id = %u",
        gvp->Identifier);

    gvpopulationadaptorFetchAllbyStatement(gvpa,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbyIdentifiers **************************
**
** Fetch all Ensembl Genetic Variation Population objects by an AJAX Table of
** AJAX unsigned integer key data and assign them as value data.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Population value data before deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all_by_dbID_list
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [u] gvps [AjPTable] AJAX Table of AJAX unsigned integer identifier
**                            key data and Ensembl Genetic Variation
**                            Population value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyIdentifiers(
    EnsPGvpopulationadaptor gvpa,
    AjPTable gvps)
{
    return ensBaseadaptorFetchAllbyIdentifiers(
        gvpa,
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensGvpopulationGetIdentifier,
        gvps);
}




/* @func ensGvpopulationadaptorFetchAllbyNamesearch ***************************
**
** Fetch all Ensembl Genetic Variation Population objects by a name search.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** fetch_all_by_name_search
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Population name
** @param [u] gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyNamesearch(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    AjPList gvps)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvpa)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    ensBaseadaptorEscapeC(gvpa, &txtname, name);

    constraint = ajFmtStr("sample.name LIKE CONCAT('%%','%s','%%')", txtname);

    ajCharDel(&txtname);

    result = ensBaseadaptorFetchAllbyConstraint(
        gvpa,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvps);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvpopulationadaptorFetchAllbySynonym ******************************
**
** Fetch a Ensembl Genetic Variation Population by an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_population_by_synonym
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [rN] source [const AjPStr] Source
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbySynonym(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList gvps)
{
    char *txtsource  = NULL;
    char *txtsynonym = NULL;

    AjPStr statement = NULL;

    if (!gvpa)
        return ajFalse;

    if (!synonym)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'sample_synonym' table is
    ** required.
    */

    ensBaseadaptorEscapeC(gvpa, &txtsynonym, synonym);

    if (source && ajStrGetLen(source))
    {
        ensBaseadaptorEscapeC(gvpa, &txtsource, source);

        statement = ajFmtStr(
            "SELECT "
            "sample.sample_id, "
            "sample.name, "
            "sample.size, "
            "sample.description, "
            "sample.display "
            "FROM "
            "sample, "
            "population, "
            "sample_synonym, "
            "source "
            "WHERE "
            "sample.sample_id = population.sample_id "
            "AND "
            "sample.sample_id = sample_synonym.sample_id "
            "AND "
            "sample_synonym.synonym = '%s' "
            "AND "
            "sample_synonym.source_id = source.source_id "
            "AND "
            "source.name = '%s'",
            txtsynonym,
            txtsource);

        ajCharDel(&txtsource);
    }
    else
        statement = ajFmtStr(
            "SELECT "
            "sample.sample_id, "
            "sample.name, "
            "sample.size, "
            "sample.description, "
            "sample.display "
            "FROM "
            "sample, "
            "population, "
            "sample_synonym "
            "WHERE "
            "sample.sample_id = population.sample_id "
            "AND "
            "sample.sample_id = sample_synonym.sample_id "
            "AND "
            "sample_synonym.synonym = '%s'",
            txtsynonym);

    ajCharDel(&txtsynonym);

    gvpopulationadaptorFetchAllbyStatement(gvpa,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

    return ajTrue;
}

#if AJFALSE
AjBool ensGvpopulationadaptorFetchAllbySynonymOld(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList gvps)
{
    ajuint *Pidentifier = 0U;

    AjPList idlist = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvpopulation gvp = NULL;

    EnsPGvsampleadaptor gvsa = NULL;

    if (!gvpa)
        return ajFalse;

    if (!synonym)
        return ajFalse;

    if (!gvps)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    idlist = ajListNew();

    ensGvsampleadaptorRetrieveAllIdentifiersBySynonym(gvsa,
                                                      synonym,
                                                      source,
                                                      idlist);

    while (ajListPop(idlist, (void **) &Pidentifier))
    {
        ensGvpopulationadaptorFetchByIdentifier(gvpa,
                                                *Pidentifier,
                                                &gvp);

        ajListPushAppend(gvps, (void *) gvp);

        AJFREE(Pidentifier);
    }

    ajListFree(&idlist);

    return ajTrue;
}
#endif /* AJFALSE */




/* @func ensGvpopulationadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Genetic Variation Population via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::fetch_by_dbID
** @param [u] gvpa [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @param [r] identifier [ajuint] SQL database-internal Population identifier
** @param [wP] Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchByIdentifier(
    EnsPGvpopulationadaptor gvpa,
    ajuint identifier,
    EnsPGvpopulation *Pgvp)
{
    if (!gvpa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pgvp)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(gvpa, identifier, (void **) Pgvp);
}




/* @func ensGvpopulationadaptorFetchByName ************************************
**
** Fetch an Ensembl Genetic Variation Population by name.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::fetch_by_name
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Population name
** @param [w] Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                     address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchByName(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    EnsPGvpopulation *Pgvp)
{
    char *txtname = NULL;

    AjPList gvps = NULL;

    AjPStr constraint = NULL;

    EnsPGvpopulation gvp = NULL;

    if (!gvpa)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!Pgvp)
        return ajFalse;

    ensBaseadaptorEscapeC(gvpa, &txtname, name);

    constraint = ajFmtStr("sample.name = '%s'", txtname);

    ajCharDel(&txtname);

    gvps = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvpa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvps);

    if (ajListGetLength(gvps) == 0)
        ajDebug("ensGvpopulationadaptorFetchByName could not get an "
                "Ensembl Genetic Variation Population "
                "for name '%S'.\n",
                name);

    if (ajListGetLength(gvps) > 1)
        ajDebug("ensGvpopulationadaptorFetchByName got more than one "
                "Ensembl Genetic Variation Population "
                "for name '%S'.\n",
                name);

    ajListPop(gvps, (void **) Pgvp);

    while (ajListPop(gvps, (void **) &gvp))
        ensGvpopulationDel(&gvp);

    ajListFree(&gvps);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchDefaultld *********************************
**
** Fetch the Ensembl Genetic Variation Population, which is used as a default
** in the LD display of the pairwise LD data.
**
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [wP] Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                      address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchDefaultld(
    EnsPGvpopulationadaptor gvpa,
    EnsPGvpopulation *Pgvp)
{
    ajuint identifier = 0U;

    AjPList mis = NULL;

    AjPStr key = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMetainformation mi         = NULL;
    EnsPMetainformationadaptor mia = NULL;

    if (!gvpa)
        return ajFalse;

    if (!Pgvp)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    mia = ensRegistryGetMetainformationadaptor(dba);

    key = ajStrNewC("pairwise_ld.default_population");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(mia, key, mis);

    ajListPop(mis, (void **) &mi);

    if (mi)
    {
        ajStrToUint(ensMetainformationGetValue(mi), &identifier);

        ensGvpopulationadaptorFetchByIdentifier(gvpa, identifier, Pgvp);

        ensMetainformationDel(&mi);
    }

    while (ajListPop(mis, (void **) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    ajStrDel(&key);

    return ajTrue;
}




/* @datasection [EnsPGvpopulationgenotype] Ensembl Genetic Variation Population
** Genotype
**
** @nam2rule Gvpopulationgenotype Functions for manipulating
** Ensembl Genetic Variation Population Genotype objects
**
** @cc Bio::EnsEMBL::Variation::PopulationGenotype
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Population Genotype
** by pointer. It is the responsibility of the user to first destroy any
** previous Genetic Variation Population Genotype. The target pointer does not
** need to be initialised to NULL, but it is good programming practice to do
**  so anyway.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @argrule Ini gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic
** Variation Population Genotype Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @argrule Ini gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @argrule Ini counter [ajuint] Counter
** @argrule Ini frequency [float] Frequency
** @argrule Ref gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @valrule * [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvpopulationgenotypeNewCpy ****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewCpy(
    const EnsPGvpopulationgenotype gvpg)
{
    EnsPGvpopulationgenotype pthis = NULL;

    if (!gvpg)
        return NULL;

    AJNEW0(pthis);

    pthis->Use          = 1U;
    pthis->Identifier   = gvpg->Identifier;
    pthis->Adaptor      = gvpg->Adaptor;
    pthis->Gvpopulation = ensGvpopulationNewRef(gvpg->Gvpopulation);
    pthis->Gvgenotype   = ensGvgenotypeNewRef(gvpg->Gvgenotype);

    pthis->Counter   = gvpg->Counter;
    pthis->Frequency = gvpg->Frequency;

    return pthis;
}




/* @func ensGvpopulationgenotypeNewIni ****************************************
**
** Constructor for an Ensembl Genetic Variation Population Genotype with
** initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvpga [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::PopulationGenotype::new
** @param [uN] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [uN] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [r] counter [ajuint] Counter
** @param [r] frequency [float] Frequency
**
** @return [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewIni(
    EnsPGvpopulationgenotypeadaptor gvpga,
    ajuint identifier,
    EnsPGvpopulation gvp,
    EnsPGvgenotype gvg,
    ajuint counter,
    float frequency)
{
    EnsPGvpopulationgenotype gvpg = NULL;

    AJNEW0(gvpg);

    gvpg->Use          = 1U;
    gvpg->Adaptor      = gvpga;
    gvpg->Identifier   = identifier;
    gvpg->Gvpopulation = ensGvpopulationNewRef(gvp);
    gvpg->Gvgenotype   = ensGvgenotypeNewRef(gvg);
    gvpg->Counter      = counter;
    gvpg->Frequency    = frequency;

    return gvpg;
}




/* @func ensGvpopulationgenotypeNewRef ****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewRef(
    EnsPGvpopulationgenotype gvpg)
{
    if (!gvpg)
        return NULL;

    gvpg->Use++;

    return gvpg;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Population Genotype
** object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Population Genotype
**
** @argrule * Pgvpg [EnsPGvpopulationgenotype*]
** Ensembl Genetic Variation Population Genotype address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvpopulationgenotypeDel *******************************************
**
** Default destructor for an Ensembl Genetic Variation Population Genotype.
**
** @param [d] Pgvpg [EnsPGvpopulationgenotype*]
** Ensembl Genetic Variation Population Genotype address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvpopulationgenotypeDel(EnsPGvpopulationgenotype *Pgvpg)
{
    EnsPGvpopulationgenotype pthis = NULL;

    if (!Pgvpg)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvpopulationgenotypeDel"))
    {
        ajDebug("ensGvpopulationgenotypeDel\n"
                "  *Pgvpg %p\n",
                *Pgvpg);

        ensGvpopulationgenotypeTrace(*Pgvpg, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvpg)
        return;

    pthis = *Pgvpg;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pgvpg = NULL;

        return;
    }

    ensGvpopulationDel(&pthis->Gvpopulation);

    ensGvgenotypeDel(&pthis->Gvgenotype);

    AJFREE(pthis);

    *Pgvpg = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Population Genotype object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Get Return Genetic Variation Population Genotype attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation
**                   Population Genotype Adaptor
** @nam4rule Counter Return the counter
** @nam4rule Frequency Return the frequency
** @nam4rule Gvgenotype Return the Ensembl Genetic Variation Genotype
** @nam4rule Gvpopulation Return the Ensembl Genetic Variation Population
** @nam4rule Identifier Return the SQL database-internal identifier
**
** @argrule * gvpg [const EnsPGvpopulationgenotype]
** Genetic Variation Population Genotype
**
** @valrule Adaptor [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor or NULL
** @valrule Counter [ajuint] Counter or 0U
** @valrule Frequency [float] Frequency or 0.0F
** @valrule Gvgenotype [EnsPGvgenotype]
** Ensembl Genetic Variation Genotype or NULL
** @valrule Gvpopulation [EnsPGvpopulation]
** Ensembl Genetic Variation Population or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationgenotypeGetAdaptor ************************************
**
** Get the Ensembl Genetic Variation Population Genotype Adaptor member of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeGetAdaptor(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? gvpg->Adaptor : NULL;
}




/* @func ensGvpopulationgenotypeGetCounter ************************************
**
** Get the counter member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [ajuint] Counter or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvpopulationgenotypeGetCounter(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? gvpg->Counter : 0U;
}




/* @func ensGvpopulationgenotypeGetFrequency **********************************
**
** Get the frequency member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [float] Frequency or 0.0F
**
** @release 6.4.0
** @@
******************************************************************************/

float ensGvpopulationgenotypeGetFrequency(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? gvpg->Frequency : 0.0F;
}




/* @func ensGvpopulationgenotypeGetGvgenotype *********************************
**
** Get the Ensembl Genetic Variation Genotype member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvgenotype ensGvpopulationgenotypeGetGvgenotype(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? gvpg->Gvgenotype : NULL;
}




/* @func ensGvpopulationgenotypeGetGvpopulation *******************************
**
** Get the Ensembl Genetic Variation Population member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationgenotypeGetGvpopulation(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? gvpg->Gvpopulation : NULL;
}




/* @func ensGvpopulationgenotypeGetIdentifier *********************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvpopulationgenotypeGetIdentifier(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? gvpg->Identifier : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Population Genotype object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Set Set one member of a Genetic Variation Population Genotype
** @nam4rule Adaptor
** Set the Ensembl Genetic Variation Population Genotype Adaptor
** @nam4rule Counter Set the counter
** @nam4rule Frequency Set the frequency
** @nam4rule Gvgenotype Set the Ensembl Genetic Variation Genotype
** @nam4rule Gvpopulation Set the Ensembl Genetic Variation Population
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype object
** @argrule Adaptor gvpga [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor
** @argrule Counter counter [ajuint] Counter
** @argrule Frequency frequency [float] Frequency
** @argrule Gvgenotype gvg [EnsPGvgenotype]
** Ensembl Genetic Variation Genotype
** @argrule Gvpopulation gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule Identifier identifier [ajuint]
** SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvpopulationgenotypeSetAdaptor ************************************
**
** Set the Ensembl Genetic Variation Population Genotype Adaptor member of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] gvpga [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetAdaptor(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulationgenotypeadaptor gvpga)
{
    if (!gvpg)
        return ajFalse;

    gvpg->Adaptor = gvpga;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetCounter ************************************
**
** Set the counter member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] counter [ajuint] Counter
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetCounter(
    EnsPGvpopulationgenotype gvpg,
    ajuint counter)
{
    if (!gvpg)
        return ajFalse;

    gvpg->Counter = counter;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetFrequency **********************************
**
** Set the frequency member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] frequency [float] Frequency
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetFrequency(
    EnsPGvpopulationgenotype gvpg,
    float frequency)
{
    if (!gvpg)
        return ajFalse;

    gvpg->Frequency = frequency;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetGvgenotype *********************************
**
** Set the Ensembl Genetic Variation Genotype member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] gvg [EnsPGvgenotype]
** Ensembl Genetic Variation Genotype
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetGvgenotype(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvgenotype gvg)
{
    if (!gvpg)
        return ajFalse;

    ensGvgenotypeDel(&gvpg->Gvgenotype);

    gvpg->Gvgenotype = ensGvgenotypeNewRef(gvg);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetGvpopulation *******************************
**
** Set the Ensembl Genetic Variation Population member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetGvpopulation(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulation gvp)
{
    if (!gvpg)
        return ajFalse;

    ensGvpopulationDel(&gvpg->Gvpopulation);

    gvpg->Gvpopulation = ensGvpopulationNewRef(gvp);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetIdentifier *********************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetIdentifier(
    EnsPGvpopulationgenotype gvpg,
    ajuint identifier)
{
    if (!gvpg)
        return ajFalse;

    gvpg->Identifier = identifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Population Genotype object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Trace Report Ensembl Genetic Variation Population Genotype
**                 members to debug file
**
** @argrule Trace gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvpopulationgenotypeTrace *****************************************
**
** Trace an Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeTrace(const EnsPGvpopulationgenotype gvpg,
                                    ajuint level)
{
    AjPStr indent = NULL;

    if (!gvpg)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvpopulationgenotypeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Gvgenotype %p\n"
            "%S  Gvpopulation %p\n"
            "%S  Allele1 '%S'\n"
            "%S  Allele2 '%S'\n"
            "%S  Counter %u\n"
            "%S  Frequency %f\n",
            indent, gvpg,
            indent, gvpg->Use,
            indent, gvpg->Identifier,
            indent, gvpg->Adaptor,
            indent, gvpg->Gvgenotype,
            indent, gvpg->Gvpopulation,
            indent, gvpg->Counter,
            indent, gvpg->Frequency);

    ensGvgenotypeTrace(gvpg->Gvgenotype, level + 1);

    ensGvpopulationTrace(gvpg->Gvpopulation, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Genetic Variation Populatin Genotype convenience functions
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Subhandle Get the subhandle
** @nam4rule Subidentifier Get the subidentifier
**
** @argrule * gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @valrule Subhandle [AjPStr] Subhandle or NULL
** @valrule Subidentifier [ajuint] Subidentifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationgenotypeGetSubhandle **********************************
**
** Get the subhandle member of the Ensembl Genetic Variation Genotype
** member of the Ensembl Genetic Variation Genotype member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [AjPStr] Subhandle or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensGvpopulationgenotypeGetSubhandle(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? ensGvgenotypeGetSubhandle(gvpg->Gvgenotype) : NULL;
}




/* @func ensGvpopulationgenotypeGetSubidentifier ******************************
**
** Get the subidentifier member of the Ensembl Genetic Variation Genotype
** member of the Ensembl Genetic Variation Genotype member of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [ajuint] Subidentifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensGvpopulationgenotypeGetSubidentifier(
    const EnsPGvpopulationgenotype gvpg)
{
    return (gvpg) ? ensGvgenotypeGetSubidentifier(gvpg->Gvgenotype) : 0U;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Population Genotype object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Population Genotype
** values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvpopulationgenotypeCalculateMemsize ******************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvpopulationgenotypeCalculateMemsize(
    const EnsPGvpopulationgenotype gvpg)
{
    size_t size = 0;

    if (!gvpg)
        return 0;

    size += sizeof (EnsOGvpopulationgenotype);

    size += ensGvgenotypeCalculateMemsize(gvpg->Gvgenotype);

    size += ensGvpopulationCalculateMemsize(gvpg->Gvpopulation);

    return size;
}




/* @datasection [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor
**
** @nam2rule Gvpopulationgenotypeadaptor Functions for manipulating
** Ensembl Genetic Variation Population Genotype Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationGenotypeAdaptor
** @cc CVS Revision: 1.23.2.1
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic gvpopulationgenotypeadaptorLinkGvgenotypecode ******************
**
** An ajTableMapDel "apply" function to link Ensembl Genetic Variation
** Genotype Code objects to Ensembl Genetic Variation Genotype objects
** linked to Ensembl Genetic Variation Population Genotype objects.
** This function deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Genetic Variation Population Genotype objects.
**
** @param [d] Pkey [void**] AJAX unsigned integer key data address
** @param [d] Pvalue [void**] AJAX Lists of Ensembl Genetic Variation
**                           Population Genotype objects
** @param [u] cl [void*] AJAX Table of Ensembl Genetic Variation Population
**                       objects, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void gvpopulationgenotypeadaptorLinkGvgenotypecode(
    void **Pkey,
    void **Pvalue,
    void *cl)
{
    EnsPGvgenotype gvg = NULL;

    EnsPGvgenotypecode gvgc = NULL;

    EnsPGvpopulationgenotype gvpg = NULL;

    if (!Pkey)
        return;

    if (!*Pkey)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    gvgc = (EnsPGvgenotypecode) ajTableFetchmodV((AjPTable) cl, *Pkey);

    /*
    ** The Ensembl Genetic Variation Population Genotype objects can be
    ** deleted after associating them with Ensembl Genetic Variation
    ** Population objects, because this AJAX Table holds independent
    ** references for these objects.
    */

    while (ajListPop(*((AjPList *) Pvalue), (void **) &gvpg))
    {
        gvg = ensGvpopulationgenotypeGetGvgenotype(gvpg);

        ensGvgenotypeAddGvgenotypecode(gvg, gvgc);

        ensGvpopulationgenotypeDel(&gvpg);
    }

    ajMemFree(Pkey);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic gvpopulationgenotypeadaptorLinkGvpopulation ********************
**
** An ajTableMapDel "apply" function to link Ensembl Genetic Variation
** Population objects to Ensembl Genetic Variation Population Genotype objects.
** This function deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Genetic Variation Population Genotype objects.
**
** @param [d] Pkey [void**] AJAX unsigned integer key data address
** @param [d] Pvalue [void**] AJAX Lists of Ensembl Genetic Variation
**                           Population Genotype objects
** @param [u] cl [void*] AJAX Table of Ensembl Genetic Variation Population
**                       objects, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void gvpopulationgenotypeadaptorLinkGvpopulation(
    void **Pkey,
    void **Pvalue,
    void *cl)
{
    EnsPGvpopulation gvp = NULL;

    EnsPGvpopulationgenotype gvpg = NULL;

    if (!Pkey)
        return;

    if (!*Pkey)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    gvp = (EnsPGvpopulation) ajTableFetchmodV((AjPTable) cl, *Pkey);

    /*
    ** The Ensembl Genetic Variation Population Genotype objects can be
    ** deleted after associating them with Ensembl Genetic Variation
    ** Population objects, because this AJAX Table holds independent
    ** references for these objects.
    */

    while (ajListPop(*((AjPList *) Pvalue), (void **) &gvpg))
    {
        ensGvpopulationgenotypeSetGvpopulation(gvpg, gvp);

        ensGvpopulationgenotypeDel(&gvpg);
    }

    ajMemFree(Pkey);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic gvpopulationgenotypeadaptorLinkGvvariation *********************
**
** An ajTableMapDel "apply" function to link Ensembl Genetic Variation
** Variation objects to Ensembl Genetic Variation Population Genotype objects.
** This function deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Genetic Variation Population Genotype objects.
**
** @param [d] Pkey [void**] AJAX unsigned integer key data address
** @param [d] Pvalue [void**] AJAX Lists of Ensembl Genetic Variation
**                           Population Genotype objects
** @param [u] cl [void*] AJAX Table of Ensembl Genetic Variation Variation
**                       objects, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.4.0
** @@
** @@
******************************************************************************/

static void gvpopulationgenotypeadaptorLinkGvvariation(
    void **Pkey,
    void **Pvalue,
    void *cl)
{
    EnsPGvpopulationgenotype gvpg = NULL;

    EnsPGvgenotype gvg = NULL;

    EnsPGvvariation gvv = NULL;

    if (!Pkey)
        return;

    if (!*Pkey)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    gvv = (EnsPGvvariation) ajTableFetchmodV((AjPTable) cl, *Pkey);

    /*
    ** The Ensembl Genetic Variation Population Genotype objects can be
    ** deleted after associating them with Ensembl Genetic Variation
    ** Variation objects, because this AJAX Table holds independent
    ** references for these objects.
    */

    while (ajListPop(*((AjPList *) Pvalue), (void **) &gvpg))
    {
        gvg = ensGvpopulationgenotypeGetGvgenotype(gvpg);

        ensGvgenotypeSetGvvariation(gvg, gvv);

        ensGvpopulationgenotypeDel(&gvpg);
    }

    ajMemFree(Pkey);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @funcstatic gvpopulationgenotypeadaptorFetchAllbyStatement *****************
**
** Fetch all Ensembl Genetic Variation Population Genotype objects via an
** SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationGenotypeAdaptor::
**     _objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvpgs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Population Genotype objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvpopulationgenotypeadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvpgs)
{
    float frequency = 0.0F;

    ajuint *Pidentifier = NULL;

    ajuint identifier    = 0U;
    ajuint gvvidentifier = 0U;
    ajuint gvgidentifier = 0U;
    ajuint gvsidentifier = 0U;
    ajuint subidentifier = 0U;
    ajuint counter       = 0U;

    AjPList list = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPTable gvgcs        = NULL;
    AjPTable gvgcstogvpgs = NULL;
    AjPTable gvps         = NULL;
    AjPTable gvpstogvpgs  = NULL;
    AjPTable gvvs         = NULL;
    AjPTable gvvstogvpgs  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvgenotype gvg = NULL;

    EnsPGvgenotypecodeadaptor gvgca = NULL;

    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvpopulationgenotype        gvpg  = NULL;
    EnsPGvpopulationgenotypeadaptor gvpga = NULL;

    EnsPGvvariationadaptor gvva = NULL;

    if (ajDebugTest("gvpopulationgenotypeadaptorFetchAllbyStatement"))
        ajDebug("gvpopulationgenotypeadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvpgs %p\n",
                ba,
                statement,
                am,
                slice,
                gvpgs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvpgs)
        return ajFalse;

    gvgcs        = ajTableuintNew(0U);
    gvgcstogvpgs = ajTableuintNew(0U);
    gvps         = ajTableuintNew(0U);
    gvpstogvpgs  = ajTableuintNew(0U);
    gvvs         = ajTableuintNew(0U);
    gvvstogvpgs  = ajTableuintNew(0U);

    ajTableSetDestroyvalue(gvgcs, (void (*)(void **)) &ensGvgenotypecodeDel);
    ajTableSetDestroyvalue(gvps,  (void (*)(void **)) &ensGvpopulationDel);
    ajTableSetDestroyvalue(gvvs,  (void (*)(void **)) &ensGvvariationDel);

    /*
    ** The AJAX Table objects gvpstogvpgs and gvvstogvpgs are cleared by
    ** linking Ensembl Genetic Variation Population and
    ** Ensembl Genetic Variation Variation objects to
    ** Ensembl Genetic Variation Poulation Genotype objects.
    */

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvpga = ensRegistryGetGvpopulationgenotypeadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier    = 0U;
        gvvidentifier = 0U;
        subidentifier = 0U;
        gvsidentifier = 0U;
        gvgidentifier = 0U;
        counter       = 0U;
        frequency     = 0.0F;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &gvvidentifier);
        ajSqlcolumnToUint(sqlr, &subidentifier);
        ajSqlcolumnToUint(sqlr, &gvsidentifier);
        ajSqlcolumnToUint(sqlr, &gvgidentifier);
        ajSqlcolumnToUint(sqlr, &counter);
        ajSqlcolumnToFloat(sqlr, &frequency);

        if ((ensGvpopulationgenotypeGetIdentifier(gvpg) != identifier) ||
            (ensGvpopulationgenotypeGetSubidentifier(gvpg) != subidentifier))
        {
            /* New object. */

            gvg = ensGvgenotypeNewIni(dba,
                                      identifier,
                                      (EnsPGvvariation) NULL,
                                      (AjPList) NULL, /* alleles */
                                      (AjPStr) NULL, /* subhandle */
                                      subidentifier);

            gvpg = ensGvpopulationgenotypeNewIni(gvpga,
                                                 identifier,
                                                 (EnsPGvpopulation) NULL,
                                                 gvg,
                                                 counter,
                                                 frequency);

            ensGvgenotypeDel(&gvg);

            ajListPushAppend(gvpgs, (void *) gvpg);

            /*
            ** Populate two AJAX Table objects to fetch
            ** Ensembl Genetic Variation Population objects
            ** from the database before associating them with
            ** Ensembl Genetic Variation Population Genotype objects.
            **
            ** gvps
            **   key data:   Ensembl Genetic Variation Population (or Sample)
            **               identifiers
            **   value data: Ensembl Genetic Variation Population objects
            **               fetched by
            **               ensGvpopulationadaptorFetchAllbyIdentifiers
            **
            ** gvpstogvpgs
            **   key data:   Ensembl Genetic Variation Population (or Sample)
            **               identifiers
            **   value data: AJAX List objects of Ensembl Genetic Variation
            **               Population Genotype objects that need associating
            **               with Ensembl Genetic Variation Population objects
            **               once they have been fetched from the database
            */

            if (!ajTableMatchV(gvps, (const void *) &gvsidentifier))
            {
                AJNEW0(Pidentifier);

                *Pidentifier = gvsidentifier;

                ajTablePut(gvps, (void *) Pidentifier, (void *) NULL);
            }

            list = (AjPList) ajTableFetchmodV(gvpstogvpgs,
                                              (const void *) &gvsidentifier);

            if (!list)
            {
                AJNEW0(Pidentifier);

                *Pidentifier = gvsidentifier;

                list = ajListNew();

                ajTablePut(gvpstogvpgs,
                           (void *) Pidentifier,
                           (void *) list);
            }

            ajListPushAppend(list,
                             (void *) ensGvpopulationgenotypeNewRef(gvpg));

            /*
            ** Populate two AJAX Table objects to fetch
            ** Ensembl Genetic Variation Genotype Code objects
            ** from the database before associating them with
            ** Ensembl Genetic Variation Genotype objects linked to
            ** Ensembl Genetic Variation Population Genotype objects.
            **
            ** gvgcs
            **   key data:   Ensembl Genetic Variation Genotype Code
            **               identifiers
            **   value data: Ensembl Genetic Variation Genotype Code objects
            **               fetched by
            **               ensGvgenotypecodeadaptorFetchAllbyIdentifiers FIXME!!!
            **
            ** gvgcstogvpgs
            **   key data:   Ensembl Genetic Variation Genotype Code
            **               identifiers
            **   value data: AJAX List objects of Ensembl Genetic Variation
            **               Population Genotype objects that need associating
            **               with Ensembl Genetic Variation Genotype Code
            **               objects once they have been fetched from the
            **               database
            */

            if (!ajTableMatchV(gvgcs, (const void *) &gvgidentifier))
            {
                AJNEW0(Pidentifier);

                *Pidentifier = gvgidentifier;

                ajTablePut(gvgcs, (void *) Pidentifier, (void *) NULL);
            }

            if (!ajTableMatchV(gvgcstogvpgs, (const void *) &gvgidentifier))
            {
                AJNEW0(Pidentifier);

                *Pidentifier = gvgidentifier;

                list = ajListNew();

                ajTablePut(gvgcstogvpgs, (void *) Pidentifier, (void *) list);
            }

            ajListPushAppend(list,
                             (void *) ensGvpopulationgenotypeNewRef(gvpg));

            /*
            ** Populate two AJAX Table objects to fetch
            ** Ensembl Genetic Variation Variation objects
            ** from the database before associating them with
            ** Ensembl Genetic Variation Genotype objects linked to
            ** Ensembl Genetic Variation Population Genotype objects.
            **
            ** gvvs
            **   key data:   Ensembl Genetic Variation Variation identifiers
            **   value data: Ensembl Genetic Variation Variation objects
            **               fetched by
            **               ensGvvariationadaptorFetchAllbyIdentifiers
            **
            ** gvvstogvpgs
            **   key data:   Ensembl Genetic Variation Variation identifiers
            **   value data: AJAX List objects of Ensembl Genetic Variation
            **               Population Genotype objects that need associating
            **               with Ensembl Genetic Variation Variation objects
            **               once they have been fetched from the database
            */

            if (!ajTableMatchV(gvvs, (const void *) &gvvidentifier))
            {
                AJNEW0(Pidentifier);

                *Pidentifier = gvvidentifier;

                ajTablePut(gvvs, (void *) Pidentifier, (void *) NULL);
            }

            if (!ajTableMatchV(gvvstogvpgs, (const void *) &gvvidentifier))
            {
                AJNEW0(Pidentifier);

                *Pidentifier = gvvidentifier;

                list = ajListNew();

                ajTablePut(gvvstogvpgs, (void *) Pidentifier, (void *) list);
            }

            ajListPushAppend(list,
                             (void *) ensGvpopulationgenotypeNewRef(gvpg));
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    /* Fetch the Ensembl Genetic Variation Genotype Code objects. */

    gvgca = ensRegistryGetGvgenotypecodeadaptor(dba);

    ensGvgenotypecodeadaptorFetchAllbyIdentifiers(gvgca, gvgcs);

    /* Fetch the Ensembl Genetic Variation Population objects. */

    gvpa = ensRegistryGetGvpopulationadaptor(dba);

    ensGvpopulationadaptorFetchAllbyIdentifiers(gvpa, gvps);

    /* Fetch the Ensembl Genetic Variation Variation objects. */

    gvva = ensRegistryGetGvvariationadaptor(dba);

    ensGvvariationadaptorFetchAllbyIdentifiers(gvva, gvvs);

    /*
    ** Associate
    ** Ensembl Genetic Variation Genotype Code objects with
    ** Ensembl Genetic Variation Genotype objects linked to
    ** Ensembl Genetic Variation Population Genotype objects.
    */

    ajTableMapDel(gvgcstogvpgs,
                  &gvpopulationgenotypeadaptorLinkGvgenotypecode,
                  (void *) gvgcs);

    ajTableFree(&gvgcstogvpgs);

    /*
    ** Associate
    ** Ensembl Genetic Variation Population objects with
    ** Ensembl Genetic Variation Population Genotype objects.
    */

    ajTableMapDel(gvpstogvpgs,
                  &gvpopulationgenotypeadaptorLinkGvpopulation,
                  (void *) gvps);

    ajTableFree(&gvpstogvpgs);

    /*
    ** Associate
    ** Ensembl Genetic Variation Variation objects with
    ** Ensembl Genetic Variation Population Genotype objects.
    */

    ajTableMapDel(gvvstogvpgs,
                  &gvpopulationgenotypeadaptorLinkGvvariation,
                  (void *) gvvs);

    ajTableFree(&gvvstogvpgs);

    ajTableDel(&gvgcs);
    ajTableDel(&gvps);
    ajTableDel(&gvvs);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Population Genotype
** Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Population Genotype Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvpopulationgenotypeadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvpopulationgenotypeadaptorNew ************************************
**
** Default constructor for an
** Ensembl Genetic Variation Population Genotype Adaptor.
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
** @see ensRegistryGetGvpopulationgenotypeadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationGenotypeAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPBaseadaptor ba = NULL;

    EnsPGvpopulationgenotypeadaptor gvpga;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensGvpopulationgenotypeadaptorNew"))
        ajDebug("ensGvpopulationgenotypeadaptorNew\n"
                "  dba %p\n",
                dba);

    ba = ensBaseadaptorNew(
        dba,
        gvpopulationgenotypeadaptorKTables,
        gvpopulationgenotypeadaptorKColumns,
        gvpopulationgenotypeadaptorKLeftjoin,
        (const char *) NULL,
        (const char *) NULL,
        &gvpopulationgenotypeadaptorFetchAllbyStatement);

    if (!ba)
        return NULL;

    AJNEW0(gvpga);

    gvpga->Adaptor     = ensRegistryGetGvdatabaseadaptor(dba);
    gvpga->Baseadaptor = ba;

    return gvpga;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Population Genotype Adaptor
** object.
**
** @fdata [EnsPGvpopulationgenotypeadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Population Genotype Adaptor
**
** @argrule * Pgvpga [EnsPGvpopulationgenotypeadaptor*]
** Ensembl Genetic Variation Population Genotype Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvpopulationgenotypeadaptorDel ************************************
**
** Default destructor for an
** Ensembl Genetic Variation Population Genotype Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvpga [EnsPGvpopulationgenotypeadaptor*]
** Ensembl Genetic Variation Population Genotype Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvpopulationgenotypeadaptorDel(
    EnsPGvpopulationgenotypeadaptor *Pgvpga)
{
    EnsPGvpopulationgenotypeadaptor pthis = NULL;

    if (!Pgvpga)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvpopulationgenotypeadaptorDel"))
        ajDebug("ensGvpopulationgenotypeadaptorDel\n"
                "  *Pgvpga %p\n",
                *Pgvpga);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvpga)
        return;

    pthis = *Pgvpga;

    ensBaseadaptorDel(&pthis->Baseadaptor);

    AJFREE(pthis);

    *Pgvpga = NULL;

    return;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Population Genotype objects
** from an Ensembl SQL database.
**
** @fdata [EnsPGvpopulationgenotypeadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Population Genotype
** object(s)
** @nam4rule All   Fetch all Ensembl Genetic Variation Population Genotype
** objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Population Genotype
**                 objects matching a criterion
** @nam5rule Gvpopulation Fetch all by an Ensembl Genetic Variation Population
** @nam5rule Gvvariation Fetch all by an Ensembl Genetic Variation Variation
** @nam5rule Identifier Fetch by an SQL database-internal identifier
** @nam5rule Name       Fetch by a name
**
** @argrule * gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor
** @argrule All gvpgs [AjPList] AJAX List of Ensembl Genetic Variation
**                              Population Genotype objects
** @argrule Gvpopulation gvp [const EnsPGvpopulation] Ensembl Genetic Variation
** Population
** @argrule Gvvariation gvv [const EnsPGvvariation] Ensembl Genetic Variation
** Variation
** @argrule Allby gvpgs [AjPList] AJAX List of Ensembl Genetic Variation
**                                Population Genotype objects
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pgpgs [EnsPGvpopulationgenotype*] Ensembl Genetic Variation
** Population Genotype address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationgenotypeadaptorFetchAllbyGvpopulation *****************
**
** Fetch all Ensembl Genetic Variation Population Genotype objects by an
** Ensembl Genetic Variation Population.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Population Genotype objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationGenotypeAdaptor::
**     fetch_all_by_Population
** @param [u] gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
**                                              Population Genotype Adaptor
** @param [r] gvp   [const EnsPGvpopulation] Ensembl Genetic Variation
** Population
** @param [u] gvpgs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Population Genotype objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeadaptorFetchAllbyGvpopulation(
    EnsPGvpopulationgenotypeadaptor gvpga,
    const EnsPGvpopulation gvp,
    AjPList gvpgs)
{
    AjPStr constraint    = NULL;
    AjPStr fvsconstraint = NULL;

    if (!gvpga)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!gvpgs)
        return ajFalse;

    if (ensGvpopulationGetIdentifier(gvp) == 0)
    {
        ajDebug("ensGvpopulationgenotypeadaptorFetchAllbyGvpopulation "
                "got Ensembl Genetic Variation Population without "
                "identifier.n");

        return ajFalse;
    }

    ensGvdatabaseadaptorFailedvariationsconstraint(gvpga->Adaptor,
                                                   (const AjPStr) NULL,
                                                   &fvsconstraint);

    constraint = ajFmtStr("population_genotype.sample_id = %u AND %S",
                          ensGvpopulationGetIdentifier(gvp),
                          fvsconstraint);

    ensBaseadaptorFetchAllbyConstraint(gvpga->Baseadaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvpgs);

    ajStrDel(&constraint);
    ajStrDel(&fvsconstraint);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeadaptorFetchAllbyGvvariation ******************
**
** Fetch all Ensembl Genetic Variation Population Genotype objects by an
** Ensembl Genetic Variation Variation.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Population Genotype objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationGenotypeAdaptor::
**     fetch_all_by_Variation
** @param [u] gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
**                                              Population Genotype Adaptor
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvpgs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Population Genotype objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeadaptorFetchAllbyGvvariation(
    EnsPGvpopulationgenotypeadaptor gvpga,
    const EnsPGvvariation gvv,
    AjPList gvpgs)
{
    AjPStr constraint = NULL;

    if (!gvpga)
        return ajFalse;

    if (!gvv)
        return ajFalse;

    if (!gvpgs)
        return ajFalse;

    if (ensGvvariationGetIdentifier(gvv) == 0)
    {
        ajDebug("ensGvpopulationgenotypeadaptorFetchAllbyGvvariation "
                "got Ensembl Genetic Variation Variation without "
                "identifier.n");

        return ajFalse;
    }

    constraint = ajFmtStr("population_genotype.variation_id = %u",
                          ensGvvariationGetIdentifier(gvv));

    ensBaseadaptorFetchAllbyConstraint(gvpga->Baseadaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvpgs);

    ajStrDel(&constraint);

    return ajTrue;
}

/* @source Ensembl Genetic Variation Population functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.34 $
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

#include "ensgvdatabaseadaptor.h"
#include "ensgvindividual.h"
#include "ensgvpopulation.h"
#include "ensgvsample.h"
#include "ensgvvariation.h"
#include "ensmetainformation.h"
#include "enstable.h"




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

/* @conststatic gvpopulationadaptorTables *************************************
**
** Array of Ensembl Genetic Variation Population Adaptor SQL table names
**
******************************************************************************/

static const char* gvpopulationadaptorTables[] =
{
    "sample",
    "population",
    (const char*) NULL
};




/* @conststatic gvpopulationadaptorColumns ************************************
**
** Array of Ensembl Genetic Variation Population Adaptor SQL column names
**
******************************************************************************/

static const char* gvpopulationadaptorColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "sample.display",
    (const char*) NULL
};




/* @conststatic gvpopulationadaptorDefaultcondition ***************************
**
** Ensembl Genetic Variation Population Adaptor SQL default condition
**
******************************************************************************/

static const char* gvpopulationadaptorDefaultcondition =
    "sample.sample_id = population.sample_id";




/* @conststatic gvpopulationgenotypeadaptorTables *****************************
**
** Array of Ensembl Genetic Variation Population Genotype Adaptor
** SQL table names
**
******************************************************************************/

static const char* const gvpopulationgenotypeadaptorTables[] =
{
    "population_genotype",
    "failed_variation",
    (const char*) NULL
};




/* @conststatic gvpopulationgenotypeadaptorColumns ****************************
**
** Array of Ensembl Genetic Variation Population Genotype Adaptor
** SQL column names
**
******************************************************************************/

static const char* const gvpopulationgenotypeadaptorColumns[] =
{
    "population_genotype.population_genotype_id",
    "population_genotype.sample_id",
    "population_genotype.variation_id",
    "population_genotype.allele_1",
    "population_genotype.allele_2",
    "population_genotype.counter",
    "population_genotype.subsnp_id",
    "population_genotype.frequency",
    (const char*) NULL
};




/* @conststatic gvpopulationgenotypeadaptorLeftjoin ***************************
**
** Array of Ensembl Genetic Variation Population Genotype Adaptor
** SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin gvpopulationgenotypeadaptorLeftjoin[] =
{
    {
        "failed_variation",
        "population_genotype.variation_id = failed_variation.variation_id"
    },
    {(const char*) NULL, (const char*) NULL}
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void tableGvpopulationClear(void** key,
                                   void** value,
                                   void* cl);

static AjBool gvpopulationadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvps);

static void gvpopulationgenotypeadaptorLinkGvpopulation(void** key,
                                                        void** value,
                                                        void* cl);

static void gvpopulationgenotypeadaptorLinkGvvariation(void** key,
                                                       void** value,
                                                       void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-62
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
** @argrule Cpy gvp [const EnsPGvpopulation] Ensembl Genetic Variation
**                                           Population
** @argrule Ini gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
** Population Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Ini subpopulations [AjPList] AJAX List of Ensembl Genetic
** Variation (Sub-) Population objects
** @argrule Ref gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @valrule * [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
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
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNewCpy(const EnsPGvpopulation gvp)
{
    AjIList iter = NULL;

    EnsPGvpopulation pthis  = NULL;
    EnsPGvpopulation subgvp = NULL;

    if(!gvp)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gvp->Identifier;

    pthis->Adaptor = gvp->Adaptor;

    pthis->Gvsample = ensGvsampleNewRef(gvp->Gvsample);

    /*
    ** Copy the AJAX List of Ensembl Genetic Variation (Sub-) Population
    ** objects.
    */

    if(ajListGetLength(gvp->Subgvpopulations))
    {
        pthis->Subgvpopulations = ajListNew();

        iter = ajListIterNew(gvp->Subgvpopulations);

        while(!ajListIterDone(iter))
        {
            subgvp = (EnsPGvpopulation) ajListIterGet(iter);

            ajListPushAppend(pthis->Subgvpopulations,
                             (void*) ensGvpopulationNewRef(subgvp));
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
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @cc Bio::EnsEMBL::Variation::Population::new
** @param [u] subpopulations [AjPList] AJAX List of Ensembl Genetic
**                                     Variation (Sub-) Population objects
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
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

    if(!gvs)
        return NULL;

    AJNEW0(gvp);

    gvp->Use = 1;

    gvp->Identifier = identifier;

    gvp->Adaptor = gvpa;

    gvp->Gvsample = ensGvsampleNewRef(gvs);

    if(ajListGetLength(subpopulations))
    {
        gvp->Subgvpopulations = ajListNew();

        iter = ajListIterNew(subpopulations);

        while(!ajListIterDone(iter))
        {
            subgvp = (EnsPGvpopulation) ajListIterGet(iter);

            ajListPushAppend(gvp->Subgvpopulations,
                             (void*) ensGvpopulationNewRef(subgvp));
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
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNewRef(EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    gvp->Use++;

    return gvp;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Population object
**
** @argrule * Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                     object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvpopulationDel ***************************************************
**
** Default destructor for an Ensembl Genetic Variation Population.
**
** @param [d] Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                     object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvpopulationDel(EnsPGvpopulation* Pgvp)
{
    EnsPGvpopulation subgvp = NULL;
    EnsPGvpopulation pthis = NULL;

    if(!Pgvp)
        return;

    if(!*Pgvp)
        return;

    if(ajDebugTest("ensGvpopulationDel"))
    {
        ajDebug("ensGvpopulationDel\n"
                "  *Pgvp %p\n",
                *Pgvp);

        ensGvpopulationTrace(*Pgvp, 1);
    }

    pthis = *Pgvp;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvp = NULL;

        return;
    }

    ensGvsampleDel(&pthis->Gvsample);

    while(ajListPop(pthis->Subgvpopulations, (void**) &subgvp))
        ensGvpopulationDel(&subgvp);

    ajListFree(&pthis->Subgvpopulations);

    AJFREE(pthis);

    *Pgvp = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
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
** @valrule Adaptor [EnsPGvpopulationadaptor] Ensembl Genetic Variation
** Population Adaptor or NULL
** @valrule Gvsample [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Subgvpopulations [const AjPList] AJAX List of Ensembl Genetic
** Variation (Sub-) Population objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationGetAdaptor ********************************************
**
** Get the Ensembl Genetic Variation Population Adaptor element of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                   Population Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensGvpopulationGetAdaptor(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    return gvp->Adaptor;
}




/* @func ensGvpopulationGetGvsample *******************************************
**
** Get the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
** @@
******************************************************************************/

EnsPGvsample ensGvpopulationGetGvsample(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    return gvp->Gvsample;
}




/* @func ensGvpopulationGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvpopulationGetIdentifier(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return 0;

    return gvp->Identifier;
}




/* @func ensGvpopulationGetSubgvpopulations ***********************************
**
** Get the AJAX List of Ensembl Genetic Variation (Sub-) Population objects
** of an Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [const AjPList] AJAX List of
** Ensembl Genetic Variation (Sub-) Population objects or NULL
** @@
******************************************************************************/

const AjPList ensGvpopulationGetSubgvpopulations(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    return gvp->Subgvpopulations;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Set Set one element of a Genetic Variation Population
** @nam4rule Adaptor Set the Ensembl Genetic Variation Population Adaptor
** @nam4rule Gvsample Set the Ensembl Genetic Variation Sample
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**                                   object
** @argrule Adaptor gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
** Population Adaptor
** @argrule Gvsample gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvpopulationSetAdaptor ********************************************
**
** Set the Ensembl Genetic Variation Population Adaptor element of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationSetAdaptor(EnsPGvpopulation gvp,
                                 EnsPGvpopulationadaptor gvpa)
{
    if(!gvp)
        return ajFalse;

    gvp->Adaptor = gvpa;

    return ajTrue;
}




/* @func ensGvpopulationSetGvsample *******************************************
**
** Set the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Population.
**
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationSetGvsample(EnsPGvpopulation gvp, EnsPGvsample gvs)
{
    if(!gvp)
        return ajFalse;

    ensGvsampleDel(&gvp->Gvsample);

    gvp->Gvsample = ensGvsampleNewRef(gvs);

    return ajTrue;
}




/* @func ensGvpopulationSetIdentifier *****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationSetIdentifier(EnsPGvpopulation gvp, ajuint identifier)
{
    if(!gvp)
        return ajFalse;

    gvp->Identifier = identifier;

    return ajTrue;
}




/* @section element addition **************************************************
**
** Functions for adding elements to an Ensembl Genetic Variation Population
** object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Add Add one object to an Ensembl Genetic Variation Population
** @nam4rule Subgvpopulation Add an Ensembl Genetic Variation (Sub-) Population
**
** @argrule * gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @argrule Subgvpopulation subgvp [EnsPGvpopulation] Ensembl Genetic
** Variation (Sub-) Population
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
** @@
******************************************************************************/

AjBool ensGvpopulationAddSubgvpopulation(EnsPGvpopulation gvp,
                                         EnsPGvpopulation subgvp)
{
    if(!gvp)
        return ajFalse;

    if(!subgvp)
        return ajFalse;

    if(!gvp->Subgvpopulations)
        gvp->Subgvpopulations = ajListNew();

    ajListPushAppend(gvp->Subgvpopulations,
                     (void*) ensGvpopulationNewRef(subgvp));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
**
** @nam3rule Trace Report Ensembl Genetic Variation Population elements to
**                 debug file
**
** @argrule Trace gvp [const EnsPGvpopulation] Ensembl Genetic Variation
**                                             Population
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
** @@
******************************************************************************/

AjBool ensGvpopulationTrace(const EnsPGvpopulation gvp, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPGvpopulation subgvp = NULL;

    if(!gvp)
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

    if(gvp->Subgvpopulations)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Genetic Variation (Sub-) Population objects\n",
                indent, gvp->Subgvpopulations);

        iter = ajListIterNewread(gvp->Subgvpopulations);

        while(!ajListIterDone(iter))
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
** @@
******************************************************************************/

size_t ensGvpopulationCalculateMemsize(const EnsPGvpopulation gvp)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPGvpopulation subgvp = NULL;

    if(!gvp)
        return 0;

    size += sizeof (EnsOGvpopulation);

    size += ensGvsampleCalculateMemsize(gvp->Gvsample);

    /*
    ** Summarise the AJAX List of Ensembl Genetic Variation
    ** (Sub-) Population objects.
    */

    iter = ajListIterNewread(gvp->Subgvpopulations);

    while(!ajListIterDone(iter))
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
** @argrule AllGvindividuals gvis [AjPList] AJAX List of
** Ensembl Genetic Variation Individual objects
** @argrule AllSynonyms gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllSynonyms source [const AjPStr] Source
** @argrule AllSynonyms synonyms [AjPList] AJAX List of
** (synonym) AJAX String objects
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
** @param [u] gvis [AjPList] AJAX List of
** Ensembl Geentic Variation Individual objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationFetchAllGvindividuals(EnsPGvpopulation gvp,
                                            AjPList gvis)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvindividualadaptor gvia = NULL;

    if(!gvp)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    if(!gvp->Adaptor)
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
** @@
******************************************************************************/

AjBool ensGvpopulationFetchAllSynonyms(EnsPGvpopulation gvp,
                                       const AjPStr source,
                                       AjPList synonyms)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvp)
        return ajFalse;

    if(!synonyms)
        return ajFalse;

    if(!gvp->Adaptor)
        return ajTrue;

    dba = ensGvpopulationadaptorGetDatabaseadaptor(gvp->Adaptor);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    return ensGvsampleadaptorRetrieveAllSynonymsByIdentifier(gvsa,
                                                             gvp->Identifier,
                                                             source,
                                                             synonyms);
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Table Functions for manipulating AJAX Table objects
**
******************************************************************************/




/* @section table *************************************************************
**
** Functions for manipulating AJAX Table objects.
**
** @fdata [AjPTable]
**
** @nam3rule Gvpopulation AJAX Table of AJAX unsigned integer key data and
**                        Ensembl Genetic Variation Population value data
** @nam4rule Clear Clear an AJAX Table
** @nam4rule Delete Delete an AJAX Table
**
** @argrule Clear table [AjPTable] AJAX Table
** @argrule Delete Ptable [AjPTable*] AJAX Table address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic tableGvpopulationClear *****************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Population value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Genetic Variation Population address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableGvpopulationClear(void** key,
                                   void** value,
                                   void* cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);

    ensGvpopulationDel((EnsPGvpopulation*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableGvpopulationClear ********************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Population value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableGvpopulationClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableGvpopulationClear, NULL);

    return ajTrue;
}




/* @func ensTableGvpopulationDelete *******************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Population value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableGvpopulationDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableGvpopulationClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [EnsPGvpopulationadaptor] Ensembl Genetic Variation Population
** Adaptor
**
** @nam2rule Gvpopulationadaptor Functions for manipulating
** Ensembl Genetic Variation Population Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor
** @cc CVS Revision: 1.28
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic gvpopulationadaptorFetchAllbyStatement *************************
**
** Fetch all Ensembl Genetic Variation Population objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvpopulationadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvps)
{
    ajuint identifier = 0;
    ajuint ssize      = 0;

    EnsEGvsampleDisplay esdisplay = ensEGvsampleDisplayNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr sname        = NULL;
    AjPStr sdescription = NULL;
    AjPStr sdisplay     = NULL;

    EnsPGvpopulation gvp         = NULL;
    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(ajDebugTest("gvpopulationadaptorFetchAllbyStatement"))
        ajDebug("gvpopulationadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvps %p\n",
                dba,
                statement,
                am,
                slice,
                gvps);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    gvpa = ensRegistryGetGvpopulationadaptor(dba);

    gvsa = ensGvpopulationadaptorGetBaseadaptor(gvpa);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        ajListPushAppend(gvps, (void*) gvp);

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
** @valrule * [EnsPGvpopulationadaptor] Ensembl Genetic Variation Population
**                                      Adaptor or NULL
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
** @return [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                   Population Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensGvpopulationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        gvpopulationadaptorTables,
        gvpopulationadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        gvpopulationadaptorDefaultcondition,
        (const char*) NULL,
        gvpopulationadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Population Adaptor object.
**
** @fdata [EnsPGvpopulationadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Population Adaptor object
**
** @argrule * Pgvpa [EnsPGvpopulationadaptor*]
** Ensembl Genetic Variation Population Adaptor object address
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
** Ensembl Genetic Variation Population Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvpopulationadaptorDel(EnsPGvpopulationadaptor* Pgvpa)
{
    if(!Pgvpa)
        return;

    ensBaseadaptorDel(Pgvpa);

    *Pgvpa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Population Adaptor object.
**
** @fdata [EnsPGvpopulationadaptor]
**
** @nam3rule Get Return Genetic Variation Population Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return teh Ensembl Databaseadaptor
**
** @argrule * gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Databaseadaptor or
**                                                NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationadaptorGetBaseadaptor *********************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Population Adaptor.
**
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensGvpopulationadaptorGetBaseadaptor(
    EnsPGvpopulationadaptor gvpa)
{
    if(!gvpa)
        return NULL;

    return gvpa;
}




/* @func ensGvpopulationadaptorGetDatabaseadaptor *****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Genetic Variation Population Adaptor.
**
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvpopulationadaptorGetDatabaseadaptor(
    EnsPGvpopulationadaptor gvpa)
{
    if(!gvpa)
        return NULL;

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
** @nam4rule Allby Fetch all Ensembl Genetic Variation Population objects
**                 matching a criterion
** @nam5rule Gvindividual Fetch all by an Ensembl Genetic Variation Individual
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
** @argrule All gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule AllbyGvindividual gvi [const EnsPGvindividual]
** Ensembl Genetic Variation Individual
** @argrule AllbyGvindividual gvps [AjPList]
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
** @argrule AllbySynonym source [const AjPStr] Source
** @argrule AllbySynonym gvps [AjPList]
** AJAX List of Ensembl Genetic Variation Population objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pgvp [EnsPGvpopulation*]
** Ensembl Genetic Variation Population object address
** @argrule ByName name [const AjPStr]
** Ensembl Genetic Variation Population name
** @argrule ByName Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation
** Population object address
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
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [u] gvps [AjPList] List of Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllHapmap(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps)
{
    AjPStr constraint = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvps)
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
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [u] gvps [AjPList] List of Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllLd(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps)
{
    AjPStr constraint = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvps)
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




/* @func ensGvpopulationadaptorFetchAllThousandgenomes ************************
**
** Fetch all Ensembl Genetic Variation Population objects, which are linked to
** 1000 Genomes data.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** etch_all_1KG_populations
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [u] gvps [AjPList] List of Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllThousandgenomes(
    EnsPGvpopulationadaptor gvpa,
    AjPList gvps)
{
    AjPStr constraint = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvps)
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
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvindividual(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvindividual gvi,
    AjPList gvps)
{
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvi)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    if(!gvi->Identifier)
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

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvpopulationadaptorFetchAllbyStatement(dba,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvps);

    ajStrDel(&statement);

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
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvpopulationSub(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps)
{
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    if(!gvp->Identifier)
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

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvpopulationadaptorFetchAllbyStatement(dba,
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
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyGvpopulationSuper(
    EnsPGvpopulationadaptor gvpa,
    const EnsPGvpopulation gvp,
    AjPList gvps)
{
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    if(!gvp->Identifier)
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

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvpopulationadaptorFetchAllbyStatement(dba,
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
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [u] gvps [AjPTable] AJAX Table of AJAX unsigned integer identifier
**                            key data and Ensembl Genetic Variation
**                            Population value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyIdentifiers(
    EnsPGvpopulationadaptor gvpa,
    AjPTable gvps)
{
    void** keyarray = NULL;

    const char* template = "sample.sample_id IN (%S)";

    register ajuint i = 0;

    ajuint identifier = 0;

    ajuint* Pidentifier = NULL;

    AjPList lgvps = NULL;

    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    EnsPGvpopulation gvp = NULL;

    if(!gvpa)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    lgvps = ajListNew();

    csv = ajStrNew();

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifiers configured in the Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    ajTableToarrayKeys(gvps, &keyarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint*) keyarray[i]));

        /* Run the statement if the maximum chunk size is exceed. */

        if(((i + 1) % ensBaseadaptorMaximumIdentifiers) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            constraint = ajFmtStr(template, csv);

            ensBaseadaptorFetchAllbyConstraint(gvpa,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               lgvps);

            ajStrDel(&constraint);

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
    {
        constraint = ajFmtStr(template, csv);

        ensBaseadaptorFetchAllbyConstraint(gvpa,
                                           constraint,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           lgvps);

        ajStrDel(&constraint);
    }

    ajStrDel(&csv);

    /*
    ** Move Ensembl Genetic Variation Population objects from the AJAX List
    ** to the AJAX Table.
    */

    while(ajListPop(lgvps, (void**) &gvp))
    {
        identifier = ensGvpopulationGetIdentifier(gvp);

        if(ajTableMatchV(gvps, (const void*) &identifier))
            ajTablePut(gvps, (void*) &identifier, (void*) gvp);
        else
        {
            /*
            ** This should not happen, because the keys should have been in
            ** the AJAX Table in the first place.
            */

            AJNEW0(Pidentifier);

            *Pidentifier = ensGvpopulationGetIdentifier(gvp);

            ajTablePut(gvps, (void*) Pidentifier, (void*) gvp);
        }
    }

    ajListFree(&lgvps);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbyNamesearch ***************************
**
** Fetch all Ensembl Genetic Variation Population objects by a name search.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** fetch_all_by_name_search
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Population name
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbyNamesearch(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    AjPList gvps)
{
    char* txtname = NULL;

    AjPStr constraint = NULL;

    if(!gvpa)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    ensBaseadaptorEscapeC(gvpa, &txtname, name);

    constraint = ajFmtStr("sample.name LIKE CONCAT('%%','%s','%%')", txtname);

    ajCharDel(&txtname);

    ensBaseadaptorFetchAllbyConstraint(gvpa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvps);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllbySynonym ******************************
**
** Fetch a Ensembl Genetic Variation Population by an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_population_by_synonym
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [rN] source [const AjPStr] Source
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllbySynonym(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList gvps)
{
    char* txtsource  = NULL;
    char* txtsynonym = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvpa)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'sample_synonym' table is
    ** required.
    */

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    ensDatabaseadaptorEscapeC(dba, &txtsynonym, synonym);

    if(source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

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

    gvpopulationadaptorFetchAllbyStatement(dba,
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
    ajuint* Pidentifier = 0;

    AjPList idlist = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvpopulation gvp = NULL;

    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvpa)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!gvps)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    idlist = ajListNew();

    ensGvsampleadaptorRetrieveAllIdentifiersBySynonym(gvsa,
                                                      synonym,
                                                      source,
                                                      idlist);

    while(ajListPop(idlist, (void**) &Pidentifier))
    {
        ensGvpopulationadaptorFetchByIdentifier(gvpa,
                                                *Pidentifier,
                                                &gvp);

        ajListPushAppend(gvps, (void*) gvp);

        AJFREE(Pidentifier);
    }

    ajListFree(&idlist);

    return ajTrue;
}
#endif




/* @func ensGvpopulationadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Genetic Variation Population via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::fetch_by_dbID
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] identifier [ajuint] SQL database-internal Population identifier
** @param [wP] Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                      address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchByIdentifier(
    EnsPGvpopulationadaptor gvpa,
    ajuint identifier,
    EnsPGvpopulation* Pgvp)
{
    if(!gvpa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvp)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(gvpa, identifier, (void**) Pgvp);
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
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchByName(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    EnsPGvpopulation* Pgvp)
{
    char* txtname = NULL;

    AjPList gvps = NULL;

    AjPStr constraint = NULL;

    EnsPGvpopulation gvp = NULL;

    if(!gvpa)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Pgvp)
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

    if(ajListGetLength(gvps) == 0)
        ajDebug("ensGvpopulationadaptorFetchByName could not get an "
                "Ensembl Genetic Variation Population "
                "for name '%S'.\n",
                name);

    if(ajListGetLength(gvps) > 1)
        ajDebug("ensGvpopulationadaptorFetchByName got more than one "
                "Ensembl Genetic Variation Population "
                "for name '%S'.\n",
                name);

    ajListPop(gvps, (void**) Pgvp);

    while(ajListPop(gvps, (void**) &gvp))
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
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchDefaultld(
    EnsPGvpopulationadaptor gvpa,
    EnsPGvpopulation* Pgvp)
{
    ajuint identifier = 0;

    AjPList mis = NULL;

    AjPStr key = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMetainformation mi         = NULL;
    EnsPMetainformationadaptor mia = NULL;

    if(!gvpa)
        return ajFalse;

    if(!Pgvp)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    mia = ensRegistryGetMetainformationadaptor(dba);

    key = ajStrNewC("pairwise_ld.default_population");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(mia, key, mis);

    ajListPop(mis, (void**) &mi);

    if(mi)
    {
        ajStrToUint(ensMetainformationGetValue(mi), &identifier);

        ensGvpopulationadaptorFetchByIdentifier(gvpa, identifier, Pgvp);

        ensMetainformationDel(&mi);
    }

    while(ajListPop(mis, (void**) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    ajStrDel(&key);

    return ajTrue;
}




/*
** TODO: Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** fetch_tagged_Population is missing, but this needs a VariationFeature first.
*/




/* @datasection [EnsPGvpopulationgenotype] Ensembl Genetic Variation Population
** Genotype
**
** @nam2rule Gvpopulationgenotype Functions for manipulating
** Ensembl Genetic Variation Population Genotype objects
**
** @cc Bio::EnsEMBL::Variation::PopulationGenotype
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-62
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
** @argrule Cpy gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                   Population Genotype
** @argrule Ini gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic
** Variation Population Genotype Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @argrule Ini gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @argrule Ini allele1 [AjPStr] Allele 1
** @argrule Ini allele2 [AjPStr] Allele 2
** @argrule Ini counter [ajuint] Counter
** @argrule Ini frequency [float] Frequency
** @argrule Ref gvpg [EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                              Population Genotype
**
** @valrule * [EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                       Population Genotype or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvpopulationgenotypeNewCpy ****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic
** Variation Population Genotype
**
** @return [EnsPGvpopulationgenotype] Ensembl Genetic Variation Population
** Genotype or NULL
** @@
******************************************************************************/

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewCpy(
    const EnsPGvpopulationgenotype gvpg)
{
    EnsPGvpopulationgenotype pthis = NULL;

    if(!gvpg)
        return NULL;

    AJNEW0(pthis);

    pthis->Use          = 1;
    pthis->Identifier   = gvpg->Identifier;
    pthis->Adaptor      = gvpg->Adaptor;
    pthis->Gvpopulation = ensGvpopulationNewRef(gvpg->Gvpopulation);
    pthis->Gvvariation  = ensGvvariationNewRef(gvpg->Gvvariation);

    if(gvpg->Allele1)
        pthis->Allele1 = ajStrNewRef(gvpg->Allele1);

    if(gvpg->Allele2)
        pthis->Allele2 = ajStrNewRef(gvpg->Allele2);

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
** @param [u] gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic
** Variation Population Genotype Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::PopulationGenotype::new
** @param [uN] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [uN] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [uN] allele1 [AjPStr] Allele 1
** @param [uN] allele2 [AjPStr] Allele 2
** @param [r] counter [ajuint] Counter
** @param [r] frequency [float] Frequency
**
** @return [EnsPGvpopulationgenotype] Ensembl Genetic Variation Population
** Genotype or NULL
** @@
******************************************************************************/

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewIni(
    EnsPGvpopulationgenotypeadaptor gvpga,
    ajuint identifier,
    EnsPGvpopulation gvp,
    EnsPGvvariation gvv,
    AjPStr allele1,
    AjPStr allele2,
    ajuint counter,
    float frequency)
{
    EnsPGvpopulationgenotype gvpg = NULL;

    AJNEW0(gvpg);

    gvpg->Use          = 1;
    gvpg->Adaptor      = gvpga;
    gvpg->Identifier   = identifier;
    gvpg->Gvpopulation = ensGvpopulationNewRef(gvp);
    gvpg->Gvvariation  = ensGvvariationNewRef(gvv);

    if(allele1)
        gvpg->Allele1 = ajStrNewRef(allele1);

    if(allele2)
        gvpg->Allele2 = ajStrNewRef(allele2);

    gvpg->Counter   = counter;
    gvpg->Frequency = frequency;

    return gvpg;
}




/* @func ensGvpopulationgenotypeNewRef ****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvpg [EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                            Population Genotype
**
** @return [EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                    Population Genotype
** @@
******************************************************************************/

EnsPGvpopulationgenotype ensGvpopulationgenotypeNewRef(
    EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return NULL;

    gvpg->Use++;

    return gvpg;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Population Genotype
** object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Population Genotype object
**
** @argrule * Pgvpg [EnsPGvpopulationgenotype*]
** Ensembl Genetic Variation Population Genotype object address
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
** Ensembl Genetic Variation Population Genotype object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvpopulationgenotypeDel(EnsPGvpopulationgenotype* Pgvpg)
{
    EnsPGvpopulationgenotype pthis = NULL;

    if(!Pgvpg)
        return;

    if(!*Pgvpg)
        return;

    if(ajDebugTest("ensGvpopulationgenotypeDel"))
    {
        ajDebug("ensGvpopulationgenotypeDel\n"
                "  *Pgvpg %p\n",
                *Pgvpg);

        ensGvpopulationgenotypeTrace(*Pgvpg, 1);
    }

    pthis = *Pgvpg;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvpg = NULL;

        return;
    }

    ensGvpopulationDel(&pthis->Gvpopulation);

    ensGvvariationDel(&pthis->Gvvariation);

    ajStrDel(&pthis->Allele1);
    ajStrDel(&pthis->Allele2);

    AJFREE(pthis);

    *Pgvpg = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Population Genotype object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Get Return Genetic Variation Population Genotype attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation
**                   Population Genotype Adaptor
** @nam4rule Allele1 Return the allele 1
** @nam4rule Allele2 Return the allele 2
** @nam4rule Counter Return the counter
** @nam4rule Frequency Return the frequency
** @nam4rule Gvpopulation Return the Ensembl Genetic Variation Population
** @nam4rule Gvvariation Return the Ensembl Genetic Variation Variation
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Subidentifier Return the sub-identifier
**
** @argrule * gvpg [const EnsPGvpopulationgenotype] Genetic Variation
**                                                  Population Genotype
**
** @valrule Adaptor [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor or NULL
** @valrule Allele1 [AjPStr] Allele 1 or NULL
** @valrule Allele2 [AjPStr] Allele 2 or NULL
** @valrule Counter [ajuint] Counter or 0
** @valrule Frequency [float] Frequency or 0.0F
** @valrule Gvpopulation [EnsPGvpopulation] Ensembl Genetic Variation
** Population or NULL
** @valrule Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Subidentifier [ajuint] Sub-Identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationgenotypeGetAdaptor ************************************
**
** Get the Ensembl Genetic Variation Population Genotype Adaptor element of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeGetAdaptor(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return NULL;

    return gvpg->Adaptor;
}




/* @func ensGvpopulationgenotypeGetAllele1 ************************************
**
** Get the allele1 element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [AjPStr] Allele1 or NULL
** @@
******************************************************************************/

AjPStr ensGvpopulationgenotypeGetAllele1(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return NULL;

    return gvpg->Allele1;
}




/* @func ensGvpopulationgenotypeGetAllele2 ************************************
**
** Get the allele2 element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [AjPStr] Allele2 or NULL
** @@
******************************************************************************/

AjPStr ensGvpopulationgenotypeGetAllele2(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return NULL;

    return gvpg->Allele2;
}




/* @func ensGvpopulationgenotypeGetCounter ************************************
**
** Get the counter element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [ajuint] Counter or 0
** @@
******************************************************************************/

ajuint ensGvpopulationgenotypeGetCounter(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return 0;

    return gvpg->Counter;
}




/* @func ensGvpopulationgenotypeGetFrequency **********************************
**
** Get the frequency element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [float] Frequency or 0.0F
** @@
******************************************************************************/

float ensGvpopulationgenotypeGetFrequency(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return 0.0F;

    return gvpg->Frequency;
}




/* @func ensGvpopulationgenotypeGetGvpopulation *******************************
**
** Get the Ensembl Genetic Variation Population element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationgenotypeGetGvpopulation(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return NULL;

    return gvpg->Gvpopulation;
}




/* @func ensGvpopulationgenotypeGetGvvariation ********************************
**
** Get the Ensembl Genetic Variation Variation element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
** @@
******************************************************************************/

EnsPGvvariation ensGvpopulationgenotypeGetGvvariation(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return NULL;

    return gvpg->Gvvariation;
}




/* @func ensGvpopulationgenotypeGetIdentifier *********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvpopulationgenotypeGetIdentifier(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return 0;

    return gvpg->Identifier;
}




/* @func ensGvpopulationgenotypeGetSubidentifier ******************************
**
** Get the sub-identifier element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [r] gvpg [const EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                                  Population Genotype
**
** @return [ajuint] Sub-identifier or 0
** @@
******************************************************************************/

ajuint ensGvpopulationgenotypeGetSubidentifier(
    const EnsPGvpopulationgenotype gvpg)
{
    if(!gvpg)
        return 0;

    return gvpg->Subidentifier;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Population Genotype object.
**
** @fdata [EnsPGvpopulationgenotype]
**
** @nam3rule Set Set one element of a Genetic Variation Population Genotype
** @nam4rule Adaptor Set the Ensembl Genetic Variation
**                   Population Genotype Adaptor
** @nam4rule Allele1 Set the allele 1
** @nam4rule Allele2 Set the allele 2
** @nam4rule Counter Set the counter
** @nam4rule Frequency Set the frequency
** @nam4rule Gvpopulation Set the Ensembl Genetic Variation Population
** @nam4rule Gvvariation Set the Ensembl Genetic Variation Variation
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Subidentifier Set the sub-identifier
**
** @argrule * gvpg [EnsPGvpopulationgenotype] Ensembl Genetic Variation
**                                            Population Genotype object
** @argrule Adaptor gvpga [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic
** Variation Population Genotype Adaptor
** @argrule Allele1 allele1 [AjPStr] Allele 1
** @argrule Allele2 allele2 [AjPStr] Allele 2
** @argrule Counter counter [ajuint] Counter
** @argrule Frequency frequency [float] Frequency
** @argrule Gvpopulation gvp [EnsPGvpopulation] Ensembl Genetic Variation
** Population
** @argrule Gvvariation gvv [EnsPGvvariation] Ensembl Genetic Variation
** Variation
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Subidentifier subidentifier [ajuint] Sub-identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvpopulationgenotypeSetAdaptor ************************************
**
** Set the Ensembl Genetic Variation Population Genotype Adaptor element of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] gvpga [EnsPGvpopulationgenotypeadaptor]
** Ensembl Genetic Variation Population Genotype Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetAdaptor(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulationgenotypeadaptor gvpga)
{
    if(!gvpg)
        return ajFalse;

    gvpg->Adaptor = gvpga;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetAllele1 ************************************
**
** Set the allele 1 element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] allele1 [AjPStr] Allele 1
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetAllele1(
    EnsPGvpopulationgenotype gvpg,
    AjPStr allele1)
{
    if(!gvpg)
        return ajFalse;

    ajStrDel(&gvpg->Allele1);

    gvpg->Allele1 = ajStrNewRef(allele1);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetAllele2 ************************************
**
** Set the allele 2 element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] allele2 [AjPStr] Allele 2
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetAllele2(
    EnsPGvpopulationgenotype gvpg,
    AjPStr allele2)
{
    if(!gvpg)
        return ajFalse;

    ajStrDel(&gvpg->Allele2);

    gvpg->Allele2 = ajStrNewRef(allele2);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetCounter ************************************
**
** Set the counter element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] counter [ajuint] Counter
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetCounter(
    EnsPGvpopulationgenotype gvpg,
    ajuint counter)
{
    if(!gvpg)
        return ajFalse;

    gvpg->Counter = counter;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetFrequency **********************************
**
** Set the frequency element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] frequency [float] Frequency
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetFrequency(
    EnsPGvpopulationgenotype gvpg,
    float frequency)
{
    if(!gvpg)
        return ajFalse;

    gvpg->Frequency = frequency;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetGvpopulation *******************************
**
** Set the Ensembl Genetic Variation Population element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetGvpopulation(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvpopulation gvp)
{
    if(!gvpg)
        return ajFalse;

    ensGvpopulationDel(&gvpg->Gvpopulation);

    gvpg->Gvpopulation = ensGvpopulationNewRef(gvp);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetGvvariation ********************************
**
** Set the Ensembl Genetic Variation Variation element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetGvvariation(
    EnsPGvpopulationgenotype gvpg,
    EnsPGvvariation gvv)
{
    if(!gvpg)
        return ajFalse;

    ensGvvariationDel(&gvpg->Gvvariation);

    gvpg->Gvvariation = ensGvvariationNewRef(gvv);

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetIdentifier *********************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Population Genotype.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetIdentifier(
    EnsPGvpopulationgenotype gvpg,
    ajuint identifier)
{
    if(!gvpg)
        return ajFalse;

    gvpg->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvpopulationgenotypeSetSubidentifier ******************************
**
** Set the sub-identifier element of an
** Ensembl Genetic Variation Population Genotype.
**
** @param [u] gvpg [EnsPGvpopulationgenotype]
** Ensembl Genetic Variation Population Genotype
** @param [r] subidentifier [ajuint] Sub-identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeSetSubidentifier(
    EnsPGvpopulationgenotype gvpg,
    ajuint subidentifier)
{
    if(!gvpg)
        return ajFalse;

    gvpg->Subidentifier = subidentifier;

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
**                 elements to debug file
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
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeTrace(const EnsPGvpopulationgenotype gvpg,
                                    ajuint level)
{
    AjPStr indent = NULL;

    if(!gvpg)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvpopulationgenotypeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Gvpopulation %p\n"
            "%S  Gvvariation %p\n"
            "%S  Allele1 '%S'\n"
            "%S  Allele2 '%S'\n"
            "%S  Counter %u\n"
            "%S  Subidentifier %u\n"
            "%S  Frequency %f\n",
            indent, gvpg,
            indent, gvpg->Use,
            indent, gvpg->Identifier,
            indent, gvpg->Adaptor,
            indent, gvpg->Gvpopulation,
            indent, gvpg->Gvvariation,
            indent, gvpg->Allele1,
            indent, gvpg->Allele2,
            indent, gvpg->Counter,
            indent, gvpg->Subidentifier,
            indent, gvpg->Frequency);

    ensGvpopulationTrace(gvpg->Gvpopulation, level + 1);

    ensGvvariationTrace(gvpg->Gvvariation, level + 1);

    ajStrDel(&indent);

    return ajTrue;
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
** @@
******************************************************************************/

size_t ensGvpopulationgenotypeCalculateMemsize(
    const EnsPGvpopulationgenotype gvpg)
{
    size_t size = 0;

    if(!gvpg)
        return 0;

    size += sizeof (EnsOGvpopulationgenotype);

    size += ensGvpopulationCalculateMemsize(gvpg->Gvpopulation);

    size += ensGvvariationCalculateMemsize(gvpg->Gvvariation);

    if(gvpg->Allele1)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvpg->Allele1);
    }

    if(gvpg->Allele2)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvpg->Allele2);
    }

    return size;
}




/* @datasection [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor
**
** @nam2rule Gvpopulationgenotypeadaptor Functions for manipulating
** Ensembl Genetic Variation Population Genotype Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationGenotypeAdaptor
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic gvpopulationgenotypeadaptorLinkGvpopulation ********************
**
** An ajTableMapDel "apply" function to link Ensembl Genetic Variation
** Population objects to Ensembl Genetic Variation Population Genotype objects.
** This function deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Genetic Variation Population Genotype objects.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX Lists of Ensembl Genetic Variation
**                           Population Genotype objects
** @param [u] cl [void*] AJAX Table of Ensembl Genetic Variation Population
**                       objects, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvpopulationgenotypeadaptorLinkGvpopulation(void** key,
                                                        void** value,
                                                        void* cl)
{
    EnsPGvpopulation gvp = NULL;

    EnsPGvpopulationgenotype gvpg = NULL;

    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    gvp = (EnsPGvpopulation) ajTableFetchmodV(cl, *key);

    /*
    ** The Ensembl Genetic Variation Population Genotype objects can be
    ** deleted after associating them with Ensembl Genetic Variation
    ** Population objects, because this AJAX Table holds independent
    ** references for these objects.
    */

    while(ajListPop(*((AjPList*) value), (void**) &gvpg))
    {
        ensGvpopulationgenotypeSetGvpopulation(gvpg, gvp);

        ensGvpopulationgenotypeDel(&gvpg);
    }

    AJFREE(*key);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic gvpopulationgenotypeadaptorLinkGvvariation *********************
**
** An ajTableMapDel "apply" function to link Ensembl Genetic Variation
** Variation objects to Ensembl Genetic Variation Population Genotype objects.
** This function deletes the AJAX unsigned integer identifier key and the
** AJAX List objects of Ensembl Genetic Variation Population Genotype objects.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX Lists of Ensembl Genetic Variation
**                           Population Genotype objects
** @param [u] cl [void*] AJAX Table of Ensembl Genetic Variation Variation
**                       objects, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
** @@
******************************************************************************/

static void gvpopulationgenotypeadaptorLinkGvvariation(void** key,
                                                       void** value,
                                                       void* cl)
{
    EnsPGvvariation gvv = NULL;

    EnsPGvpopulationgenotype gvpg = NULL;

    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    gvv = (EnsPGvvariation) ajTableFetchmodV(cl, *key);

    /*
    ** The Ensembl Genetic Variation Population Genotype objects can be
    ** deleted after associating them with Ensembl Genetic Variation
    ** Variation objects, because this AJAX Table holds independent
    ** references for these objects.
    */

    while(ajListPop(*((AjPList*) value), (void**) &gvpg))
    {
        ensGvpopulationgenotypeSetGvvariation(gvpg, gvv);

        ensGvpopulationgenotypeDel(&gvpg);
    }

    AJFREE(*key);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic gvpopulationgenotypeadaptorFetchAllbyStatement *****************
**
** Fetch all Ensembl Genetic Variation Population Genotype objects via an
** SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationGenotypeAdaptor::
**     _objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvpgs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Population Genotype objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvpopulationgenotypeadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvpgs)
{
    float frequency = 0.0F;

    ajuint* Pidentifier = NULL;

    ajuint lastgvvfid    = 0;
    ajuint identifier    = 0;
    ajuint gvvidentifier = 0;
    ajuint gvsidentifier = 0;
    ajuint subidentifier = 0;
    ajuint counter       = 0;

    AjPList list = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr allele1 = NULL;
    AjPStr allele2 = NULL;

    AjPTable gvps        = NULL;
    AjPTable gvpstogvpgs = NULL;
    AjPTable gvvs        = NULL;
    AjPTable gvvstogvpgs = NULL;

    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvpopulationgenotype        gvpg  = NULL;
    EnsPGvpopulationgenotypeadaptor gvpga = NULL;

    EnsPGvvariationadaptor gvva = NULL;

    if(ajDebugTest("gvpopulationgenotypeadaptorFetchAllbyStatement"))
        ajDebug("gvpopulationgenotypeadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvpgs %p\n",
                dba,
                statement,
                am,
                slice,
                gvpgs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvpgs)
        return ajFalse;

    gvps        = ensTableuintNewLen(0);
    gvpstogvpgs = ensTableuintNewLen(0);
    gvvs        = ensTableuintNewLen(0);
    gvvstogvpgs = ensTableuintNewLen(0);

    gvpga = ensRegistryGetGvpopulationgenotypeadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier    = 0;
        gvsidentifier = 0;
        gvvidentifier = 0;
        allele1       = ajStrNew();
        allele2       = ajStrNew();
        counter       = 0;
        subidentifier = 0;
        frequency     = 0.0F;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &gvsidentifier);
        ajSqlcolumnToUint(sqlr, &gvvidentifier);
        ajSqlcolumnToStr(sqlr, &allele1);
        ajSqlcolumnToStr(sqlr, &allele2);
        ajSqlcolumnToUint(sqlr, &counter);
        ajSqlcolumnToUint(sqlr, &subidentifier);
        ajSqlcolumnToFloat(sqlr, &frequency);

        /* Skip multiple rows because of the left join to failed_variation. */

        if(lastgvvfid == identifier)
        {
            ajStrDel(&allele1);
            ajStrDel(&allele2);

            continue;
        }
        else
            lastgvvfid = identifier;

        gvpg = ensGvpopulationgenotypeNewIni(gvpga,
                                             identifier,
                                             (EnsPGvpopulation) NULL,
                                             (EnsPGvvariation) NULL,
                                             allele1,
                                             allele2,
                                             counter,
                                             frequency);

        ajListPushAppend(gvpgs, (void*) gvpg);

        /*
        ** Populate two AJAX Table objects to fetch Ensembl Genetic Variation
        ** Population objects from the database and associate them with
        ** Ensembl Genetic Variation Population Genotype objects.
        **
        ** gvps
        **   key data:   Ensembl Genetic Variation Population (or Sample)
        **               identifiers
        **   value data: Ensembl Genetic Variation Population objects
        **               fetched by ensGvpopulationadaptorFetchAllbyIdentifiers
        **
        ** gvpstogvpgs
        **   key data:   Ensembl Genetic Variation Population (or Sample)
        **               identifiers
        **   value data: AJAX List objects of Ensembl Genetic Variation
        **               Population Genotype objects that need to be associated
        **               with Ensembl Genetic Variation Population objects once
        **               they have been fetched from the database
        */

        if(!ajTableMatchV(gvps, (const void*) &gvsidentifier))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = gvsidentifier;

            ajTablePut(gvps, (void*) Pidentifier, (void*) NULL);
        }

        list = (AjPList) ajTableFetchmodV(gvpstogvpgs,
                                          (const void*) &gvsidentifier);

        if(!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = gvsidentifier;

            list = ajListNew();

            ajTablePut(gvpstogvpgs,
                       (void*) Pidentifier,
                       (void*) list);
        }

        ajListPushAppend(list, (void*) ensGvpopulationgenotypeNewRef(gvpg));

        /*
        ** Populate two AJAX Table objects to fetch Ensembl Genetic Variation
        ** Variation objects from the database and associate them with
        ** Ensembl Genetic Variation Population Genotype objects.
        **
        ** gvvs
        **   key data:   Ensembl Genetic Variation Variation identifiers
        **   value data: Ensembl Genetic Variation Variation objects
        **               fetched by ensGvvariationadaptorFetchAllbyIdentifiers
        **
        ** gvvstogvpgs
        **   key data:   Ensembl Genetic Variation Variation identifiers
        **   value data: AJAX List objects of Ensembl Genetic Variation
        **               Population Genotype objects that need to be associated
        **               with Ensembl Genetic Variation Variation objects once
        **               they have been fetched from the database
        */

        if(!ajTableMatchV(gvvs, (const void*) &gvvidentifier))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = gvvidentifier;

            ajTablePut(gvvs, (void*) Pidentifier, (void*) NULL);
        }

        if(!ajTableFetchmodV(gvvstogvpgs, (const void*) &gvvidentifier))
        {
            AJNEW0(Pidentifier);

            *Pidentifier = gvvidentifier;

            list = ajListNew();

            ajTablePut(gvvstogvpgs, (void*) Pidentifier, (void*) list);
        }

        ajListPushAppend(list, (void*) ensGvpopulationgenotypeNewRef(gvpg));

        ajStrDel(&allele1);
        ajStrDel(&allele2);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    /* Fetch the Ensembl Genetic Variation Population objects. */

    gvpa = ensRegistryGetGvpopulationadaptor(dba);

    ensGvpopulationadaptorFetchAllbyIdentifiers(gvpa, gvps);

    /* Fetch the Ensembl Genetic Variation Variation objects. */

    gvva = ensRegistryGetGvvariationadaptor(dba);

    ensGvvariationadaptorFetchAllbyIdentifiers(gvva, gvvs);

    /*
    ** Associate
    ** Ensembl Genetic Variation Population objects with
    ** Ensembl Genetic Variation Population Genotype objects.
    */

    ajTableMapDel(gvpstogvpgs,
                  gvpopulationgenotypeadaptorLinkGvpopulation,
                  (void*) gvps);

    ajTableFree(&gvpstogvpgs);

    /*
    ** Associate
    ** Ensembl Genetic Variation Variation objects with
    ** Ensembl Genetic Variation Population Genotype objects.
    */

    ajTableMapDel(gvvstogvpgs,
                  gvpopulationgenotypeadaptorLinkGvvariation,
                  (void*) gvvs);

    ajTableFree(&gvvstogvpgs);

    /* Delete the utility AJAX Table objects. */

    ensTableGvpopulationDelete(&gvps);
    ensTableGvvariationDelete(&gvvs);

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
** @valrule * [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor or NULL
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
** @return [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic Variation
** Population Genotype Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationgenotypeadaptor ensGvpopulationgenotypeadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPBaseadaptor ba = NULL;

    EnsPGvpopulationgenotypeadaptor gvpga;

    if(!dba)
        return NULL;

    if(ajDebugTest("ensGvpopulationgenotypeadaptorNew"))
        ajDebug("ensGvpopulationgenotypeadaptorNew\n"
                "  dba %p\n",
                dba);

    ba = ensBaseadaptorNew(
        dba,
        gvpopulationgenotypeadaptorTables,
        gvpopulationgenotypeadaptorColumns,
        gvpopulationgenotypeadaptorLeftjoin,
        (const char*) NULL,
        (const char*) NULL,
        gvpopulationgenotypeadaptorFetchAllbyStatement);

    if(!ba)
        return NULL;

    AJNEW0(gvpga);

    gvpga->Adaptor     = ensRegistryGetGvdatabaseadaptor(dba);
    gvpga->Baseadaptor = ba;

    return gvpga;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Population Genotype
** Adaptor object.
**
** @fdata [EnsPGvpopulationgenotypeadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Population Genotype Adaptor object
**
** @argrule * Pgvpga [EnsPGvpopulationgenotypeadaptor*]
** Ensembl Genetic Variation Population Genotype Adaptor object address
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
** Ensembl Genetic Variation Population Genotype Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvpopulationgenotypeadaptorDel(
    EnsPGvpopulationgenotypeadaptor* Pgvpga)
{
    EnsPGvpopulationgenotypeadaptor pthis = NULL;

    if(!Pgvpga)
        return;

    if(!*Pgvpga)
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
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeadaptorFetchAllbyGvpopulation(
    EnsPGvpopulationgenotypeadaptor gvpga,
    const EnsPGvpopulation gvp,
    AjPList gvpgs)
{
    AjPStr constraint    = NULL;
    AjPStr fvsconstraint = NULL;

    if(!gvpga)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvpgs)
        return ajFalse;

    if(ensGvpopulationGetIdentifier(gvp) == 0)
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
** @@
******************************************************************************/

AjBool ensGvpopulationgenotypeadaptorFetchAllbyGvvariation(
    EnsPGvpopulationgenotypeadaptor gvpga,
    const EnsPGvvariation gvv,
    AjPList gvpgs)
{
    AjPStr constraint = NULL;

    if(!gvpga)
        return ajFalse;

    if(!gvv)
        return ajFalse;

    if(!gvpgs)
        return ajFalse;

    if(ensGvvariationGetIdentifier(gvv) == 0)
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

/******************************************************************************
** @source Ensembl Genetic Variation Population functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.1 $
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

#include "ensgvpopulation.h"
#include "ensmetainformation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPMetainformationadaptor
ensRegistryGetMetainformationadaptor(EnsPDatabaseadaptor dba);

extern EnsPGvpopulationadaptor
ensRegistryGetGvpopulationadaptor(EnsPDatabaseadaptor dba);

extern EnsPGvsampleadaptor
ensRegistryGetGvsampleadaptor(EnsPDatabaseadaptor dba);

static AjBool gvPopulationadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                               const AjPStr statement,
                                               EnsPAssemblymapper am,
                                               EnsPSlice slice,
                                               AjPList gvps);




/* @filesection ensgvpopulation ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvpopulation] Genetic Variation Population ***************
**
** Functions for manipulating Ensembl Genetic Variation Population objects
**
** @cc Bio::EnsEMBL::Variation::Population CVS Revision: 1.10
**
** @nam2rule Gvpopulation
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGvpopulation] Ensembl Genetic Variation Population
** @argrule Ref object [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @valrule * [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @fcategory new
******************************************************************************/




/* @func ensGvpopulationNew ***************************************************
**
** Default constructor for an Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] gvs [EnsPGvvsample] Ensembl Genetic Variation Sample
** @cc Bio::EnsEMBL::Variation::Population::new
** @param [r] subpopulations [AjPList] AJAX List of Ensembl Genetic
**                                     Variation (Sub-) Populations
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNew(EnsPGvpopulationadaptor gvpa,
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

    gvp->SubPopulations = ajListNew();

    iter = ajListIterNew(subpopulations);

    while(!ajListIterDone(iter))
    {
        subgvp = (EnsPGvpopulation) ajListIterGet(iter);

        ajListPushAppend(gvp->SubPopulations,
                         (void *) ensGvpopulationNewRef(subgvp));
    }

    ajListIterDel(&iter);

    return gvp;
}




/* @func ensGvpopulationNewObj ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvpopulation] Ensembl Genetic Variation
**                                            Population
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
** @@
******************************************************************************/

EnsPGvpopulation ensGvpopulationNewObj(const EnsPGvpopulation object)
{
    AjIList iter = NULL;

    EnsPGvpopulation gvp    = NULL;
    EnsPGvpopulation subgvp = NULL;

    if(!object)
        return NULL;

    AJNEW0(gvp);

    gvp->Use = 1;

    gvp->Identifier = object->Identifier;

    gvp->Adaptor = object->Adaptor;

    gvp->Gvsample = ensGvsampleNewRef(object->Gvsample);

    /* Copy the AJAX List of Ensembl Genetic Variation (Sub-) Populations. */

    gvp->SubPopulations = ajListNew();

    iter = ajListIterNew(object->SubPopulations);

    while(!ajListIterDone(iter))
    {
        subgvp = (EnsPGvpopulation) ajListIterGet(iter);

        ajListPushAppend(gvp->SubPopulations,
                         (void *) ensGvpopulationNewRef(subgvp));
    }

    ajListIterDel(&iter);

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
** memory allocated for the Ensembl Genetic Variation Population.
**
** @fdata [EnsPGvpopulation]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Population object
**
** @argrule * Pgvp [EnsPGvpopulation*] Genetic Variation Population
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
**                                     address
**
** @return [void]
** @@
******************************************************************************/

void ensGvpopulationDel(EnsPGvpopulation *Pgvp)
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

    while(ajListPop(pthis->SubPopulations, (void **) &subgvp))
        ensGvpopulationDel(&subgvp);

    ajListFree(&pthis->SubPopulations);

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
** @fnote None
**
** @nam3rule Get Return Genetic Variation Population attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Population Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetGvsample Return the Ensembl Genetic Variation Sample
** @nam4rule GetSubPopulation Return the AJAX List of Ensembl Genetic
**                            Variation (sub-) Populations
**
** @argrule * population [const EnsPGvpopulation] Genetic Variation Population
**
** @valrule Adaptor [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                            Population Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Gvsample [EnsPGvsample] Ensembl Genetic Variation Sample
** @valrule SubPopulations [AjPList] AJAX List of Ensembl Genetic Variation
**                                  (Sub-) Populations
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationGetAdaptor ********************************************
**
** Get the Ensembl Genetic Variation Population Adaptor element of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                   Population Adaptor
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensGvpopulationGetAdaptor(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    return gvp->Adaptor;
}




/* @func ensGvpopulationGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvpopulationGetIdentifier(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return 0;

    return gvp->Identifier;
}




/* @func ensGvpopulationGetGvsample *******************************************
**
** Get the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample
** @@
******************************************************************************/

EnsPGvsample ensGvpopulationGetGvsample(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    return gvp->Gvsample;
}




/* @func ensGvpopulationGetSubPopulations *************************************
**
** Get the AJAX List of Ensembl Genetic Variation (Sub-) Populations of an
** Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [const AjPList] AJAX List of Ensembl Genetic Variation Populations
** @@
******************************************************************************/

const AjPList ensGvpopulationGetSubPopulations(const EnsPGvpopulation gvp)
{
    if(!gvp)
        return NULL;

    return gvp->SubPopulations;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Population
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation Population Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetSample Set the Ensembl Genetic Variation Sample
**
** @argrule * gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**                                   object
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
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
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




/* @func ensGvpopulationSetIdentifier *****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Population.
**
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




/* @func ensGvpopulationAddSubPopulation **************************************
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

AjBool ensGvpopulationAddSubPopulation(EnsPGvpopulation gvp,
                                       EnsPGvpopulation subgvp)
{
    if(!gvp)
        return ajFalse;

    if(!subgvp)
        return ajFalse;

    if(!gvp->SubPopulations)
        gvp->SubPopulations = ajListNew();

    ajListPushAppend(gvp->SubPopulations,
                     (void *) ensGvpopulationNewRef(subgvp));

    return ajTrue;
}




/* @func ensGvpopulationGetMemSize ********************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Population.
**
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensGvpopulationGetMemSize(const EnsPGvpopulation gvp)
{
    ajuint size = 0;

    AjIList iter = NULL;

    EnsPGvpopulation subgvp = NULL;

    if(!gvp)
        return 0;

    size += (ajuint) sizeof (EnsOGvpopulation);

    size += ensGvsampleGetMemSize(gvp->Gvsample);

    /*
    ** Summarise the AJAX List of Ensembl Genetic Variation
    ** (Sub-) Populations.
    */

    iter = ajListIterNewread(gvp->SubPopulations);

    while(!ajListIterDone(iter))
    {
        subgvp = (EnsPGvpopulation) ajListIterGet(iter);

        size += ensGvpopulationGetMemSize(subgvp);
    }

    ajListIterDel(&iter);

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Population object.
**
** @fdata [EnsPGvpopulation]
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
            "%S  SubPopulations %p\n",
            indent, gvp,
            indent, gvp->Use,
            indent, gvp->Identifier,
            indent, gvp->Adaptor,
            indent, gvp->Gvsample,
            indent, gvp->SubPopulations);

    ensGvsampleTrace(gvp->Gvsample, level + 1);

    /* Trace the AJAX List of Ensembl Genetic Variation (Sub-) Populations. */

    if(gvp->SubPopulations)
    {
        ajDebug("%S    AJAX List %p of "
                "Ensembl Genetic Variation (Sub-) Populations\n",
                indent, gvp->SubPopulations);

        iter = ajListIterNewread(gvp->SubPopulations);

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




/* @datasection [EnsPGvpopulationadaptor] Genetic Variation Population Adaptor
**
** Functions for manipulating Ensembl Genetic Variation Population Adaptor
** objects
**
** @nam2rule Gvpopulationadaptor
**
******************************************************************************/

static const char *gvPopulationadaptorTables[] =
{
    "sample",
    "population",
    NULL
};

static const char *gvPopulationadaptorColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "sample.display",
    NULL
};

static EnsOBaseadaptorLeftJoin gvPopulationadaptorLeftJoin[] =
{
    {NULL, NULL}
};

static const char *gvPopulationadaptorDefaultCondition =
"sample.sample_id = population.sample_id";

static const char *gvPopulationadaptorFinalCondition = NULL;




/* @funcstatic gvPopulationadaptorFetchAllBySQL *******************************
**
** Fetch all Ensembl Genetic Variation Population objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Population
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvPopulationadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                               const AjPStr statement,
                                               EnsPAssemblymapper am,
                                               EnsPSlice slice,
                                               AjPList gvps)
{
    ajuint identifier = 0;
    ajuint ssize      = 0;

    AjEnum esdisplay = ensEGvsampleDisplayNULL;

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

    if(ajDebugTest("gvPopulationadaptorFetchAllBySQL"))
        ajDebug("gvPopulationadaptorFetchAllBySQL\n"
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

    gvsa = ensGvpopulationadaptorGetAdaptor(gvpa);

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

        gvs = ensGvsampleNew(gvsa,
                             identifier,
                             sname,
                             sdescription,
                             esdisplay,
                             ssize);

        gvp = ensGvpopulationNew(gvpa,
                                 identifier,
                                 gvs,
                                 (AjPList) NULL);

        ensGvsampleDel(&gvs);

        ajStrDel(&sname);
        ajStrDel(&sdescription);
        ajStrDel(&sdisplay);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    return ajTrue;
}




/* @func ensGvpopulationadaptorNew ********************************************
**
** Default constructor for an Ensembl Genetic Variation Population Adaptor.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                   Population Adaptor or NULL
** @@
******************************************************************************/

EnsPGvpopulationadaptor ensGvpopulationadaptorNew(EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(dba,
                             gvPopulationadaptorTables,
                             gvPopulationadaptorColumns,
                             gvPopulationadaptorLeftJoin,
                             gvPopulationadaptorDefaultCondition,
                             gvPopulationadaptorFinalCondition,
                             gvPopulationadaptorFetchAllBySQL);
}




/* @func ensGvpopulationadaptorDel ********************************************
**
** Default destructor for an Ensembl Gentic Variation Population Adaptor.
**
** @param [d] Pgvpa [EnsPGvpopulationadaptor*] Ensembl Genetic Variation
**                                             Population Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensGvpopulationadaptorDel(EnsPGvpopulationadaptor *Pgvpa)
{
    if(!Pgvpa)
        return;

    ensBaseadaptorDel(Pgvpa);

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Population Adaptor object.
**
** @fdata [EnsPGvpopulationadaptor]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Population Adaptor attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Sample Adaptor
**
** @argrule * gvpa [const EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
**
** @valrule Adaptor [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                        Sample Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensGvpopulationadaptorGetAdaptor *************************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Population Adaptor.
**
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensGvpopulationadaptorGetAdaptor(
    EnsPGvpopulationadaptor gvpa)
{
    if(!gvpa)
        return NULL;

    return gvpa;
}




/* @func ensGvpopulationadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Genetic Variation Population via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::fetch_by_dbID
** @param [r] gvpa [const EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
** @param [r] identifier [ajuint] SQL database-internal Population identifier
** @param [wP] Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                      address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchByIdentifier(
    const EnsPGvpopulationadaptor gvpa,
    ajuint identifier,
    EnsPGvpopulation *Pgvp)
{
    if(!gvpa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvp)
        return ajFalse;

    *Pgvp = (EnsPGvpopulation)
        ensBaseadaptorFetchByIdentifier(gvpa, identifier);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchByName ************************************
**
** Fetch an Ensembl Genetic Variation Population by name.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::fetch_by_name
** @param [u] gvpa [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                           Population Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Population name
** @param [w] Pgvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**                                    address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchByName(
    EnsPGvpopulationadaptor gvpa,
    const AjPStr name,
    EnsPGvpopulation* Pgvp)
{
    char *txtname = NULL;

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

    ensBaseadaptorGenericFetch(gvpa,
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

    ajListPop(gvps, (void **) Pgvp);

    while(ajListPop(gvps, (void **) &gvp))
        ensGvpopulationDel(&gvp);

    ajListFree(&gvps);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllBySynonym ******************************
**
** Fetch all Ensembl Genetic Variation Individuals by an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_population_by_synonym
** @param [r] gvpa [const EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Populations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllBySynonym(
    const EnsPGvpopulationadaptor gvpa,
    const AjPStr synonym,
    AjPList gvps)
{
    ajuint *Pidentifier = 0;

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

    ensGvsampleadaptorFetchAllIdentifiersBySynonym(gvsa,
                                                   synonym,
                                                   (const AjPStr) NULL,
                                                   idlist);

    while(ajListPop(idlist, (void **) &Pidentifier))
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




/* @func ensGvpopulationadaptorFetchAllBySuperPopulation **********************
**
** Fetch all Ensembl Genetic Variation Populations by a super Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_super_Population
** @param [r] gvpa [const EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Populations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllBySuperPopulation(
    const EnsPGvpopulationadaptor gvpa,
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
        ajDebug("ensGvpopulationadaptorFetchAllBySuperPopulation "
                "cannot fetch Population for super Population "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** This function does not use the Ensembl Base Adaptor functionality
    ** because an additional 'population_structure' table is required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display "
        "FROM "
        "population, "
        "population_structure, "
        "sample "
        "WHERE "
        "population.sample_id = "
        "population_structure.sub_population_sample_id "
        "AND "
        "population_structure.super_population_sample_id = %u "
        "AND "
        "population.sample_id = sample.sample_id",
        gvp->Identifier);

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvPopulationadaptorFetchAllBySQL(dba,
                                     statement,
                                     (EnsPAssemblymapper) NULL,
                                     (EnsPSlice) NULL,
                                     gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllBySubPopulation ************************
**
** Fetch all Ensembl Genetic Variation Populations by a sub Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_sub_Population
** @param [r] gvpa [const EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Populations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllBySubPopulation(
    const EnsPGvpopulationadaptor gvpa,
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
        ajDebug("ensGvpopulationadaptorFetchAllBySubPopulation "
                "cannot fetch Population for sub Population "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** This function does not use the Ensembl Base Adaptor functionality
    ** because an additional 'population_structure' table is required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display "
        "FROM "
        "population, "
        "population_structure, "
        "sample "
        "WHERE "
        "population.sample_id = "
        "population_structure.super_population_sample_id "
        "AND "
        "population_structure.sub_population_sample_id = %u "
        "AND "
        "population.sample_id = sample.sample_id",
        gvp->Identifier);

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvPopulationadaptorFetchAllBySQL(dba,
                                     statement,
                                     (EnsPAssemblymapper) NULL,
                                     (EnsPSlice) NULL,
                                     gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchAllByIndvividual **************************
**
** Fetch all Ensembl Genetic Variation Populations by an
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
**     fetch_all_by_Individual
** @param [r] gvpa [const EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] gvps [AjPList] AJAX List of Ensembl Genetic Variation Populations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchAllByIndvividual(
    const EnsPGvpopulationadaptor gvpa,
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
        ajDebug("ensGvpopulationadaptorFetchAllByIndvividual "
                "cannot fetch Population for Individual "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** This function does not use the Ensembl Base Adaptor functionality
    ** because an additional 'population_structure' table is required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display "
        "FROM "
        "population, "
        "individual_population, "
        "sample "
        "WHERE "
        "sample.sample_id = "
        "individual_population.population_sample_id "
        "AND "
        "sample.sample_id = population.sample_id "
        "AND "
        "individual_population.individual_sample_id = %u",
        gvi->Identifier);

    dba = ensBaseadaptorGetDatabaseadaptor(gvpa);

    gvPopulationadaptorFetchAllBySQL(dba,
                                     statement,
                                     (EnsPAssemblymapper) NULL,
                                     (EnsPSlice) NULL,
                                     gvps);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvpopulationadaptorFetchDefaultLDPopulation ***********************
**
** Fetch the Ensembl Genetic Variation Population, which is used as a default
** in the LD display of the pairwise LD data.
**
** @param [r] gvpa [const EnsPGvpopulationAdaptor] Ensembl Genetic Variation
**                                                 Population Adaptor
** @param [wP] Pgvp [EnsPGvpopulation*] Ensembl Genetic Variation Population
**                                      address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvpopulationadaptorFetchDefaultLDPopulation(
    const EnsPGvpopulationadaptor gvpa,
    EnsPGvpopulation *Pgvp)
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

    ensMetainformationadaptorFetchAllByKey(mia, key, mis);

    ajListPop(mis, (void **) &mi);

    if(mi)
    {
        ajStrToUint(ensMetainformationGetValue(mi), &identifier);

        ensGvpopulationadaptorFetchByIdentifier(gvpa, identifier, Pgvp);

        ensMetainformationDel(&mi);
    }

    while(ajListPop(mis, (void **) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    ajStrDel(&key);

    return ajTrue;
}




/*
** TODO: Bio::EnsEMBL::Variation::DBSQL::PopulationAdaptor::
** fetch_tagged_Population is missing, but this needs a VariationFeature first.
*/

/* @source ensgvallele ********************************************************
**
** Ensembl Genetic Variation Allele functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.23 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:10 $ by $Author: mks $
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

#include "ensgvallele.h"
#include "ensgvbaseadaptor.h"
#include "ensgvdatabaseadaptor.h"
#include "ensgvpopulation.h"
#include "ensgvvariation.h"




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

/* @conststatic gvalleleadaptorKTablenames ************************************
**
** Array of Ensembl Genetic Variation Allele Adaptor SQL table names
**
******************************************************************************/

static const char *gvalleleadaptorKTablenames[] =
{
    "allele",
    "allele_code",
    (const char *) NULL
};




/* @conststatic gvalleleadaptorKColumnnames ***********************************
**
** Array of Ensembl Genetic Variation Allele Adaptor SQL column names
**
******************************************************************************/

static const char *gvalleleadaptorKColumnnames[] =
{
    "allele.allele_id",
    "allele.variation_id",
    "allele.subsnp_id",
    "allele_code.allele",
    "allele.frequency",
    "allele.sample_id",
    "allele.count",
    (const char *) NULL
};




/* @conststatic gvalleleadaptorKLeftjoins *************************************
**
** Array of Ensembl Genetic Variation Allele Adaptor SQL LEFT JOIN conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin gvalleleadaptorKLeftjoins[] =
{
    {"failed_allele", "allele.allele_id = failed_allele.allele_id"},
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic gvalleleadaptorKDefaultcondition ******************************
**
** Ensembl Genetic Variation Allele Adaptor SQL SELECT default condition
**
******************************************************************************/

static const char *gvalleleadaptorKDefaultcondition =
    "allele.allele_code_id = allele_code.allele_code_id";




/* @conststatic gvalleleadaptorfailedvariationKTablenames *********************
**
** Array of Ensembl Genetic Variation Allele Adaptor SQL table names
**
******************************************************************************/

static const char *gvalleleadaptorfailedvariationKTablenames[] =
{
    "allele",
    "allele_code",
    "failed_allele",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool gvalleleadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvas);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvallele ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvallele] Ensembl Genetic Variation Allele ***************
**
** @nam2rule Gvallele Functions for manipulating
** Ensembl Genetic Variation Allele objects
**
** @cc Bio::EnsEMBL::Variation::Allele
** @cc CVS Revision: 1.20
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::Variation::Failable
** @cc CVS Revision: 1.2
** @cc CVS Tag: branch-ensembl-68
**
** TODO: Lazy loading of Ensembl Genetic Variation Population objects and
** Ensembl Genetic Variation Variation objects is not implemented yet.
** In the Perl implementation Variation objects have references to Allele
** objects and vice versa so to avoid circular references, the Allele object
** weakens its link to the Variation object.
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Allele by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Allele. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvallele]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gva [const EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Ini gvaa [EnsPGvalleleadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @argrule Ini allele [AjPStr] Allele
** @argrule Ini frequency [float] Frequency
** @argrule Ini subidentifier [ajuint] Subidentifier
** @argrule Ini counter [ajuint] Counter
** @argrule Ini gvvid [ajuint] Ensembl Genetic Variation Variation identifier
** @argrule Ref gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @valrule * [EnsPGvallele] Ensembl Genetic Variation Allele or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvalleleNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvallele] Ensembl Genetic Variation Allele or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvallele ensGvalleleNewCpy(const EnsPGvallele gva)
{
    AjIList iter = NULL;

    AjPStr description = NULL;

    EnsPGvallele pthis = NULL;

    if (!gva)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = gva->Identifier;

    pthis->Adaptor = gva->Adaptor;

    pthis->Gvpopulation = ensGvpopulationNewRef(gva->Gvpopulation);

    if (gva->Allele)
        pthis->Allele = ajStrNewRef(gva->Allele);

    if (gva->Subhandle)
        pthis->Subhandle = ajStrNewRef(gva->Subhandle);

    /* NOTE: Copy the AJAX List of AJAX String (failed description) objects. */

    if (gva->Faileddescriptions)
    {
        pthis->Faileddescriptions = ajListstrNew();

        iter = ajListIterNew(gva->Faileddescriptions);

        while (!ajListIterDone(iter))
        {
            description = ajListstrIterGet(iter);

            if (description)
                ajListstrPushAppend(pthis->Faileddescriptions,
                                    ajStrNewS(description));
        }

        ajListIterDel(&iter);
    }

    pthis->Counter = gva->Counter;

    pthis->Gvvariationidentifier = gva->Gvvariationidentifier;

    pthis->Subidentifier = gva->Subidentifier;

    pthis->Frequency = gva->Frequency;

    return pthis;
}




/* @func ensGvalleleNewIni ****************************************************
**
** Constructor for an Ensembl Genetic Variation Allele with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvaa [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                       Allele Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Allele::new
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] allele [AjPStr] Allele
** @param [r] frequency [float] Frequency
** @param [r] subidentifier [ajuint] Subidentifier
** @param [r] counter [ajuint] Counter
** @param [r] gvvid [ajuint] Ensembl Genetic Variation Variation identifier
**
** @return [EnsPGvallele] Ensembl Genetic Variation Allele or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvallele ensGvalleleNewIni(EnsPGvalleleadaptor gvaa,
                               ajuint identifier,
                               EnsPGvpopulation gvp,
                               AjPStr allele,
                               float frequency,
                               ajuint subidentifier,
                               ajuint counter,
                               ajuint gvvid)
{
    EnsPGvallele gva = NULL;

    if (!gvp)
        return NULL;

    if (!allele)
        return NULL;

    AJNEW0(gva);

    gva->Use = 1U;

    gva->Identifier = identifier;

    gva->Adaptor = gvaa;

    gva->Gvpopulation = ensGvpopulationNewRef(gvp);

    if (allele)
        gva->Allele = ajStrNewS(allele);

    gva->Subidentifier = subidentifier;

    gva->Counter = counter;

    gva->Gvvariationidentifier = gvvid;

    gva->Frequency = frequency;

    return gva;
}




/* @func ensGvalleleNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvallele] Ensembl Genetic Variation Allele or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvallele ensGvalleleNewRef(EnsPGvallele gva)
{
    if (!gva)
        return NULL;

    gva->Use++;

    return gva;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Allele
**
** @argrule * Pgva [EnsPGvallele*] Ensembl Genetic Variation Allele address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvalleleDel *******************************************************
**
** Default destructor for an Ensembl Genetic Variation Allele.
**
** @param [d] Pgva [EnsPGvallele*] Ensembl Genetic Variation Allele address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvalleleDel(EnsPGvallele *Pgva)
{
    EnsPGvallele pthis = NULL;

    if (!Pgva)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvalleleDel"))
    {
        ajDebug("ensGvalleleDel\n"
                "  *Pgva %p\n",
                *Pgva);

        ensGvalleleTrace(*Pgva, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgva) || --pthis->Use)
    {
        *Pgva = NULL;

        return;
    }

    ensGvpopulationDel(&pthis->Gvpopulation);

    ajStrDel(&pthis->Allele);

    ajStrDel(&pthis->Subhandle);

    ajListstrFreeData(&pthis->Faileddescriptions);

    ajMemFree((void **) Pgva);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Get Return Genetic Variation Allele attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Allele Adaptor
** @nam4rule Allele Return the allele
** @nam4rule Counter Return the counter
** @nam4rule Frequency Return the frequency
** @nam4rule Gvpopulation Return the Ensembl Genetic Variation Population
** @nam4rule Gvvariationidentifier
** Return the Ensembl Genetic Variation Variation identifier
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Subidentifier Return the subidentifier
**
** @argrule * gva [const EnsPGvallele] Genetic Variation Allele
**
** @valrule Adaptor [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                        Allele Adaptor or NULL
** @valrule Allele [AjPStr] Allele or NULL
** @valrule Counter [ajuint] Counter or 0U
** @valrule Frequency [float] Frequency or 0.0F
** @valrule Gvpopulation [EnsPGvpopulation]
** Ensembl Genetic Variation Population or NULL
** @valrule Gvvariationidentifier [ajuint]
** Ensembl Genetic Variation Variation identifier or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Subidentifier [ajuint] Subidentifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleGetAdaptor ************************************************
**
** Get the Ensembl Genetic Variation Allele Adaptor member of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvalleleadaptor] Ensembl Genetic Variation Allele Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvalleleadaptor ensGvalleleGetAdaptor(const EnsPGvallele gva)
{
    return (gva) ? gva->Adaptor : NULL;
}




/* @func ensGvalleleGetAllele *************************************************
**
** Get the allele member of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [AjPStr] Allele or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGvalleleGetAllele(const EnsPGvallele gva)
{
    return (gva) ? gva->Allele : NULL;
}




/* @func ensGvalleleGetCounter ************************************************
**
** Get the counter member of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] Counter or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvalleleGetCounter(const EnsPGvallele gva)
{
    return (gva) ? gva->Counter : 0U;
}




/* @func ensGvalleleGetFrequency **********************************************
**
** Get the frequency member of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [float] Frequency or 0.0F
**
** @release 6.2.0
** @@
******************************************************************************/

float ensGvalleleGetFrequency(const EnsPGvallele gva)
{
    return (gva) ? gva->Frequency : 0.0F;
}




/* @func ensGvalleleGetGvpopulation *******************************************
**
** Get the Ensembl Genetic Variation Population member of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvpopulation ensGvalleleGetGvpopulation(const EnsPGvallele gva)
{
    return (gva) ? gva->Gvpopulation : NULL;
}




/* @func ensGvalleleGetGvvariationidentifier **********************************
**
** Get the Ensembl Genetic Variation Variation identifier member of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] Ensembl Genetic Variation Variation identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvalleleGetGvvariationidentifier(const EnsPGvallele gva)
{
    return (gva) ? gva->Gvvariationidentifier : 0U;
}




/* @func ensGvalleleGetIdentifier *********************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvalleleGetIdentifier(const EnsPGvallele gva)
{
    return (gva) ? gva->Identifier : 0U;
}




/* @func ensGvalleleGetSubidentifier ******************************************
**
** Get the subidentifier member of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] Subidentifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvalleleGetSubidentifier(const EnsPGvallele gva)
{
    return (gva) ? gva->Subidentifier : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Genetic Variation Allele
** object, which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Load Return Ensembl Genetic Variation Allele attribute(s)
** loaded on demand
** @nam4rule All Return all Ensembl Genetic Variation Allele attribute(s)
** loaded on demand
** @nam5rule Faileddescriptions Load all failed descriptions
** @nam4rule Subhandle Return the subhandle
**
** @argrule * gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @valrule Faileddescriptions [const AjPList]
** AJAX List of AJAX String (failed description) objects or NULL
** @valrule Subhandle [AjPStr] Subhandle or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleLoadAllFaileddescriptions *********************************
**
** Load all failed descriptions of an Ensembl Genetic Variation Allele.
**
** This is not a simple accessor function, it will fetch the Ensembl Genetic
** Variation Allele failed descriptions from the Ensembl Genetic Variation
** database in case the AJAX List is not defined.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [const AjPList]
** AJAX List of AJAX String (failed description) objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGvalleleLoadAllFaileddescriptions(EnsPGvallele gva)
{
    if (!gva)
        return NULL;

    if (gva->Faileddescriptions)
        return gva->Faileddescriptions;

    if (!gva->Adaptor)
    {
        ajDebug("ensGvalleleLoadAllFaileddescriptions cannot retrieve "
                "AJAX String (failed description) objects for an "
                "Ensembl Genetic Variation Allele whithout an "
                "Ensembl Genetic Variation Allele Adaptor.\n");

        return NULL;
    }

    gva->Faileddescriptions = ajListstrNew();

    ensGvalleleadaptorRetrieveAllFaileddescriptions(
        gva->Adaptor,
        gva,
        gva->Faileddescriptions);

    return gva->Faileddescriptions;
}




/* @func ensGvalleleLoadSubhandle *********************************************
**
** Get the subhandle member of an Ensembl Genetic Variation Allele.
**
** This is not a simple accessor function, it will fetch the Ensembl Genetic
** Variation Allele subhandle from the Ensembl Genetic Variation database
** in case the AJAX String is not defined.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [AjPStr] Subhandle or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvalleleLoadSubhandle(EnsPGvallele gva)
{
    if (!gva)
        return NULL;

    if (gva->Subhandle)
        return gva->Subhandle;

    if (!gva->Adaptor)
    {
        ajDebug("ensGvalleleLoadSubhandle cannot fetch a subhandle "
                "from the database for an Ensembl Genetic Variation Allele "
                "whithout an Ensembl Genetic Variation Allele Adaptor.\n");

        return gva->Subhandle;
    }

    gva->Subhandle = ajStrNew();

    ensGvalleleadaptorRetrieveSubhandle(
        gva->Adaptor,
        gva,
        &gva->Subhandle);

    return gva->Subhandle;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Set Set one member of a Genetic Variation Allele
** @nam4rule Adaptor Set the Ensembl Genetic Variation Allele Adaptor
** @nam4rule Allele Set the allele
** @nam4rule Counter Set the counter
** @nam4rule Frequency Set the frequency
** @nam4rule Gvpopulation Set the Ensembl Genetic Variation Population
** @nam4rule Gvvariationidentifier
** Set the Ensembl Genetic Variation Variation identifier
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Subhandle Set the subhandle
** @nam4rule Subidentifier Set the subidentifier
**
** @argrule * gva [EnsPGvallele] Ensembl Genetic Variation Allele object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
** @argrule Adaptor gvaa [EnsPGvalleleadaptor] Ensembl Genetic Variation
** @argrule Allele allele [AjPStr] Allele
** @argrule Counter counter [ajuint] Counter
** @argrule Frequency frequency [float] Frequency
** @argrule Gvpopulation gvp [EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule Gvvariationidentifier gvvid [ajuint]
** Ensembl Genetic Variation Variation identifier
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Subhandle subhandle [AjPStr] Subhandle
** @argrule Subidentifier subidentifier [ajuint] Subidentifier
**
** @fcategory modify
******************************************************************************/




/* @func ensGvalleleSetAdaptor ************************************************
**
** Set the Ensembl Genetic Variation Allele Adaptor member of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] gvaa [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                       Allele Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvalleleSetAdaptor(EnsPGvallele gva, EnsPGvalleleadaptor gvaa)
{
    if (!gva)
        return ajFalse;

    gva->Adaptor = gvaa;

    return ajTrue;
}




/* @func ensGvalleleSetAllele *************************************************
**
** Set the allele member of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] allele [AjPStr] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvalleleSetAllele(EnsPGvallele gva, AjPStr allele)
{
    if (!gva)
        return ajFalse;

    ajStrDel(&gva->Allele);

    if (allele)
        gva->Allele = ajStrNewRef(allele);

    return ajTrue;
}




/* @func ensGvalleleSetCounter ************************************************
**
** Set the counter member of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] counter [ajuint] Counter
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleSetCounter(EnsPGvallele gva, ajuint counter)
{
    if (!gva)
        return ajFalse;

    gva->Counter = counter;

    return ajTrue;
}




/* @func ensGvalleleSetFrequency **********************************************
**
** Set the frequency member of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] frequency [float] Frequency
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvalleleSetFrequency(EnsPGvallele gva, float frequency)
{
    if (!gva)
        return ajFalse;

    gva->Frequency = frequency;

    return ajTrue;
}




/* @func ensGvalleleSetGvpopulation *******************************************
**
** Set the Ensembl Genetic Variation Population member of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleSetGvpopulation(EnsPGvallele gva, EnsPGvpopulation gvp)
{
    if (!gva)
        return ajFalse;

    ensGvpopulationDel(&gva->Gvpopulation);

    gva->Gvpopulation = ensGvpopulationNewRef(gvp);

    return ajTrue;
}




/* @func ensGvalleleSetGvvariationidentifier **********************************
**
** Set the Ensembl Genetic Variation Variation identifier member of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] gvvid [ajuint] Ensembl Genetic Variation Variation identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleSetGvvariationidentifier(EnsPGvallele gva, ajuint gvvid)
{
    if (!gva)
        return ajFalse;

    gva->Gvvariationidentifier = gvvid;

    return ajTrue;
}




/* @func ensGvalleleSetIdentifier *********************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvalleleSetIdentifier(EnsPGvallele gva, ajuint identifier)
{
    if (!gva)
        return ajFalse;

    gva->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvalleleSetSubhandle **********************************************
**
** Set the subhandle member of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] subhandle [AjPStr] Subhandle
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleSetSubhandle(EnsPGvallele gva, AjPStr subhandle)
{
    if (!gva)
        return ajFalse;

    ajStrDel(&gva->Subhandle);

    if (subhandle)
        gva->Subhandle = ajStrNewRef(subhandle);

    return ajTrue;
}




/* @func ensGvalleleSetSubidentifier ******************************************
**
** Set the subidentifier member of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] subidentifier [ajuint] Subidentifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleSetSubidentifier(EnsPGvallele gva, ajuint subidentifier)
{
    if (!gva)
        return ajFalse;

    gva->Subidentifier = subidentifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Trace Report Ensembl Genetic Variation Allele members to
**                 debug file
**
** @argrule Trace gva [const EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvalleleTrace *****************************************************
**
** Trace an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvalleleTrace(const EnsPGvallele gva, ajuint level)
{
    AjIList iter = NULL;

    AjPStr description = NULL;
    AjPStr indent      = NULL;

    if (!gva)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvalleleTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Population %p\n"
            "%S  Allele '%S'\n"
            "%S  Subhandle '%S'\n"
            "%S  Faileddescriptions %p\n"
            "%S  Counter %u\n"
            "%S  Gvvariationidentifier %u\n"
            "%S  Subidentifier %u\n"
            "%S  Frequency %f\n",
            indent, gva,
            indent, gva->Use,
            indent, gva->Identifier,
            indent, gva->Adaptor,
            indent, gva->Gvpopulation,
            indent, gva->Allele,
            indent, gva->Subhandle,
            indent, gva->Faileddescriptions,
            indent, gva->Counter,
            indent, gva->Gvvariationidentifier,
            indent, gva->Subidentifier,
            indent, gva->Frequency);

    ensGvpopulationTrace(gva->Gvpopulation, level + 1);

    /* Trace the AJAX List of AJAX String (failed description) objects. */

    if (gva->Faileddescriptions)
    {
        ajDebug("%S    AJAX List %p of AJAX String (failed description) "
                "objects:\n",
                indent, gva->Faileddescriptions);

        iter = ajListIterNewread(gva->Faileddescriptions);

        while (!ajListIterDone(iter))
        {
            description = ajListstrIterGet(iter);

            ajDebug("%S      %S\n", description);
        }
    }

    ajListIterDel(&iter);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Allele information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvalleleCalculateMemsize ******************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvalleleCalculateMemsize(const EnsPGvallele gva)
{
    size_t size = 0;

    if (!gva)
        return 0;

    size += sizeof (EnsOGvallele);

    size += ensGvpopulationCalculateMemsize(gva->Gvpopulation);

    if (gva->Allele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gva->Allele);
    }

    if (gva->Subhandle)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gva->Subhandle);
    }

    return size;
}




/* @section query *************************************************************
**
** Functions for querying the properties of an
** Ensembl Genetic Variation Allele.
**
** @fdata [EnsPGvallele]
**
** @nam3rule Is Check whether an Ensembl Genetic Variation Allele represents a
** certain property
** @nam4rule Failed Check for failed
**
** @argrule Failed gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Failed Presult [AjBool*] Boolean result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleIsFailed **************************************************
**
** Check if an Ensembl Genetic Variation Allele has been failed.
** This function calls ensGvalleleLoadAllFaileddescriptions to find out if
** failed descriptiosn are available.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] Presult [AjBool*] ajTrue, if an Ensembl Genetic Variation Allele
**                              has been failed
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleIsFailed(EnsPGvallele gva, AjBool *Presult)
{
    const AjPList descriptions = NULL;

    if (!gva)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    descriptions = ensGvalleleLoadAllFaileddescriptions(gva);

    if (ajListGetLength(descriptions) > 0)
        *Presult = ajTrue;
    else
        *Presult = ajFalse;

    return ajTrue;
}




/* @datasection [EnsPGvalleleadaptor] Ensembl Genetic Variation Allele Adaptor
**
** @nam2rule Gvalleleadaptor Functions for manipulating
** Ensembl Genetic Variation Allele Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic gvalleleadaptorFetchAllbyStatement *****************************
**
** Fetch all Ensembl Genetic Variation Allele objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvas [AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvalleleadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvas)
{
    float frequency = 0.0F;

    ajuint counter    = 0U;
    ajuint identifier = 0U;
    ajuint gvpid      = 0U;
    ajuint gvvid      = 0U;
    ajuint lastid     = 0U;
    ajuint subid      = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr allele       = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvallele        gva  = NULL;
    EnsPGvalleleadaptor gvaa = NULL;

    EnsPGvpopulation        gvp  = NULL;
    EnsPGvpopulationadaptor gvpa = NULL;

    if (ajDebugTest("gvalleleadaptorFetchAllbyStatement"))
        ajDebug("gvalleleadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvas %p\n",
                ba,
                statement,
                am,
                slice,
                gvas);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvas)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvaa = ensRegistryGetGvalleleadaptor(dba);
    gvpa = ensRegistryGetGvpopulationadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0U;
        gvvid      = 0U;
        subid      = 0U;
        allele     = ajStrNew();
        frequency  = 0.0F;
        gvpid      = 0U;
        counter    = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &gvvid);
        ajSqlcolumnToUint(sqlr, &subid);
        ajSqlcolumnToStr(sqlr, &allele);
        ajSqlcolumnToFloat(sqlr, &frequency);
        ajSqlcolumnToUint(sqlr, &gvpid);
        ajSqlcolumnToUint(sqlr, &counter);

        if (identifier != lastid)
        {
            ensGvpopulationadaptorFetchByIdentifier(gvpa, gvpid, &gvp);

            gva = ensGvalleleNewIni(gvaa,
                                    identifier,
                                    gvp,
                                    allele,
                                    frequency,
                                    subid,
                                    counter,
                                    gvvid);

            ajListPushAppend(gvas, (void *) gva);

            ensGvpopulationDel(&gvp);
        }

        ajStrDel(&allele);

        lastid = identifier;
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Allele Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Allele Adaptor. The target pointer does not
** need to be initialised to NULL, but it is good programming practice to do
** so anyway.
**
** @fdata [EnsPGvalleleadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvalleleadaptorNew ************************************************
**
** Default constructor for an Ensembl Genetic Variation Allele Adaptor.
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
** @see ensRegistryGetGvalleleadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvalleleadaptor ensGvalleleadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPGvalleleadaptor gvaa = NULL;

    EnsPGvdatabaseadaptor gvdba = NULL;

    if (!dba)
        return NULL;

    gvdba = ensRegistryGetGvdatabaseadaptor(dba);

    if (!gvdba)
        return NULL;

    AJNEW0(gvaa);

    gvaa->Excludeadaptor = ensGvbaseadaptorNew(
        gvdba,
        gvalleleadaptorKTablenames,
        gvalleleadaptorKColumnnames,
        gvalleleadaptorKLeftjoins,
        gvalleleadaptorKDefaultcondition,
        (const char *) NULL,
        &gvalleleadaptorFetchAllbyStatement);

    /*
    ** If failed Variation objects are included, add the "failed_variation"
    ** SQL table name, but skip the SQL LEFT JOIN.
    */

    gvaa->Includeadaptor = ensGvbaseadaptorNew(
        gvdba,
        gvalleleadaptorfailedvariationKTablenames,
        gvalleleadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        gvalleleadaptorKDefaultcondition,
        (const char *) NULL,
        &gvalleleadaptorFetchAllbyStatement);

    return gvaa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Allele Adaptor object.
**
** @fdata [EnsPGvalleleadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Allele Adaptor
**
** @argrule * Pgvaa [EnsPGvalleleadaptor*]
** Ensembl Genetic Variation Allele Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvalleleadaptorDel ************************************************
**
** Default destructor for an Ensembl Genetic Variation Allele Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvaa [EnsPGvalleleadaptor*]
** Ensembl Genetic Variation Allele Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvalleleadaptorDel(EnsPGvalleleadaptor *Pgvaa)
{
    EnsPGvalleleadaptor pthis = NULL;

    if (!Pgvaa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvalleleadaptorDel"))
        ajDebug("ensGvalleleadaptorDel\n"
                "  *Pgvaa %p\n",
                *Pgvaa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvaa))
        return;

    ensGvbaseadaptorDel(&pthis->Excludeadaptor);
    ensGvbaseadaptorDel(&pthis->Includeadaptor);

    ajMemFree((void **) Pgvaa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Allele Adaptor object.
**
** @fdata [EnsPGvalleleadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Allele Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
**
** @argrule * gvaa [const EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Allele Adaptor.
**
** @param [r] gvaa [const EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvalleleadaptorGetDatabaseadaptor(
    const EnsPGvalleleadaptor gvaa)
{
    if (!gvaa)
        return NULL;

    /*
    ** It does not matter from which Ensembl Genetic Variation Base Adaptor
    ** the Ensembl Genetic Variation Database Adaptor is taken, they both
    ** link to the same object.
    */

    return ensGvdatabaseadaptorGetDatabaseadaptor(
        ensGvbaseadaptorGetGvdatabaseadaptor(gvaa->Excludeadaptor));
}




/* @func ensGvalleleadaptorGetGvdatabaseadaptor *******************************
**
** Get the Ensembl Genetic Variation Database Adaptor member of an
** Ensembl Genetic Variation Allele Adaptor.
**
** @param [r] gvaa [const EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvalleleadaptorGetGvdatabaseadaptor(
    const EnsPGvalleleadaptor gvaa)
{
    /*
    ** It does not matter from which Ensembl Genetic Variation Base Adaptor
    ** the Ensembl Genetic Variation Database Adaptor is taken, they both
    ** link to the same object.
    */

    return (gvaa) ?
        ensGvbaseadaptorGetGvdatabaseadaptor(gvaa->Excludeadaptor) : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Allele objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGvalleleadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Allele object(s)
** @nam4rule All Fetch all Ensembl Genetic Variation Allele objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Allele objects
**                 matching a criterion
** @nam5rule Gvvariation Fetch all by an Ensembl Genetic Variation Variation
** @nam5rule Subidentifier
** @nam4rule By Fetch one Ensembl Genetic Variation Allele object
**              matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * gvaa [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @argrule Gvvariation gvv [const EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule Gvvariation gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule Subidentifier subidentifier [ajuint] Subidentifier
** @argrule All gvas [AjPList] AJAX List of
** Ensembl Genetic Variation Allele objects
** @argrule Allby gvas [AjPList] AJAX List of
** Ensembl Genetic Variation Allele objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pgva [EnsPGvallele*]
** Ensembl Genetic Variation Allele object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleadaptorFetchAllbyGvvariation ******************************
**
** Fetch all Ensembl Genetic Variation Allele objects by an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor::fetch_all_by_Variation
** @param [u] gvaa [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @param [r] gvv [const EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @param [rN] gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @param [u] gvas [AjPList] AJAX List of
** Ensembl Genetic Variation Allele objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleadaptorFetchAllbyGvvariation(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvvariation gvv,
    const EnsPGvpopulation gvp,
    AjPList gvas)
{
    AjBool result = AJFALSE;

    AjPStr constraint    = NULL;
    AjPStr fvsconstraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvdatabaseadaptor gvdba = NULL;

    if (!gvaa)
        return ajFalse;

    if (!gvv)
        return ajFalse;

    if (!gvas)
        return ajFalse;

    /*
    ** It does not matter from which Ensembl Genetic Variation Base Adaptor
    ** the Ensembl Genetic Variation Database Adaptor is taken, they both
    ** link to the same object. It does, however, matter, which
    ** Ensembl Genetic Variation Base Adaptor gets used.
    */

    gvdba = ensGvbaseadaptorGetGvdatabaseadaptor(gvaa->Excludeadaptor);

    if (ensGvdatabaseadaptorGetFailedvariations(gvdba))
        ba = ensGvbaseadaptorGetBaseadaptor(gvaa->Includeadaptor);
    else
        ba = ensGvbaseadaptorGetBaseadaptor(gvaa->Excludeadaptor);

    constraint = ajFmtStr("allele.variation_id = %u",
                          ensGvvariationGetIdentifier(gvv));

    if (gvp)
        ajFmtPrintAppS(&constraint, " AND allele.sample_id = %u",
                       ensGvpopulationGetIdentifier(gvp));

    ensGvdatabaseadaptorFailedvariationsconstraint(
        gvdba,
        (const AjPStr) NULL,
        &fvsconstraint);

    ajStrAppendC(&constraint, " AND ");
    ajStrAppendS(&constraint, fvsconstraint);

    ajStrDel(&fvsconstraint);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvas);

    /*
    ** NOTE: The Perl API iterates over the Ensembl Genetic Variation Allele
    ** objects and adds the Ensembl Genetic Variation Variation object to each
    ** one of them. This will also add the Ensembl Genetic Variation Allele
    ** object to the Ensembl Genetic Variation Variation object and weaken
    ** the Variation object's link back to the Allele object.
    ** If an Ensembl Genetic Variation Population object was specified, the
    ** Perl API adds this object to the Allele as well.
    */

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvalleleadaptorFetchAllbySubidentifier ****************************
**
** Fetch all Ensembl Genetic Variation Allele objects by a Subidentifier.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor::fetch_all_by_subsnp_id
** @param [u] gvaa [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @param [r] subidentifier [ajuint] Subidentifier
** @param [u] gvas [AjPList] AJAX List of
** Ensembl Genetic Variation Allele objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleadaptorFetchAllbySubidentifier(
    EnsPGvalleleadaptor gvaa,
    ajuint subidentifier,
    AjPList gvas)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvdatabaseadaptor gvdba = NULL;

    if (!gvaa)
        return ajFalse;

    if (!subidentifier)
        return ajFalse;

    if (!gvas)
        return ajFalse;

    gvdba = ensGvbaseadaptorGetGvdatabaseadaptor(gvaa->Excludeadaptor);

    if (ensGvdatabaseadaptorGetFailedvariations(gvdba))
        ba = ensGvbaseadaptorGetBaseadaptor(gvaa->Includeadaptor);
    else
        ba = ensGvbaseadaptorGetBaseadaptor(gvaa->Excludeadaptor);

    constraint = ajFmtStr("allele.subsnp_id = %u", subidentifier);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvas);

    ajStrDel(&constraint);

    return result;
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to
** Ensembl Genetic Variation Allele
** objects from an Ensembl SQL database.
**
** @fdata [EnsPGvalleleadaptor]
**
** @nam3rule Retrieve
** Retrieve Ensembl Genetic Variation Allele-releated object(s)
** @nam4rule All
** Retrieve all Ensembl Genetic Variation Allele-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
** @nam5rule Faileddescriptions Retrieve all failed descriptions
** @nam4rule Subhandle Retrieve the Subhandle
**
** @argrule * gvaa [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer objects
** @argrule AllFaileddescriptions gva [const EnsPGvallele]
** Ensembl Genetic Variation Allele
** @argrule AllFaileddescriptions descriptions [AjPList]
** AJAX List of AJAX String (failed description) objects
** @argrule Subhandle gva [const EnsPGvallele]
** Ensembl Genetic Variation Allele
** @argrule Subhandle Psubhandle [AjPStr*] Subhandle
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleadaptorRetrieveAllFaileddescriptions **********************
**
** Retrieve all failed descriptions of an
** Ensembl Genetic Variation Allele object.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor::
** get_all_failed_descriptions
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor::
** _internal_get_failed_descriptions
** @param [u] gvaa [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @param [r] gva [const EnsPGvallele]
** Ensembl Genetic Variation Allele
** @param [u] descriptions [AjPList] AJAX List of
** AJAX String (description) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleadaptorRetrieveAllFaileddescriptions(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvallele gva,
    AjPList descriptions)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr description = NULL;
    AjPStr statement   = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvaa)
        return ajFalse;

    if (!gva)
        return ajFalse;

    if (!descriptions)
        return ajFalse;

    dba = ensGvalleleadaptorGetDatabaseadaptor(gvaa);

    statement = ajFmtStr(
        "SELECT "
        "DISTINCT "
        "failed_description.description "
        "FROM "
        "failed_allele "
        "JOIN "
        "failed_description "
        "ON "
        "("
        "failed_description.failed_description_id = "
        "failed_allele.failed_description_id"
        ") "
        "WHERE "
        "failed_allele.allele_id = %u",
        gva->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        description = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &description);

        ajListstrPushAppend(descriptions, description);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvalleleadaptorRetrieveSubhandle **********************************
**
** Retrieve the Subhandle of an
** Ensembl Genetic Variation Allele object.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor::get_subsnp_handle
** @param [u] gvaa [EnsPGvalleleadaptor]
** Ensembl Genetic Variation Allele Adaptor
** @param [r] gva [const EnsPGvallele]
** Ensembl Genetic Variation Allele
** @param [u] Psubhandle [AjPStr*] AJAX String (Subhandle) address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvalleleadaptorRetrieveSubhandle(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvallele gva,
    AjPStr *Psubhandle)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr handle    = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvaa)
        return ajFalse;

    if (!gva)
        return ajFalse;

    dba = ensGvalleleadaptorGetDatabaseadaptor(gvaa);

    statement = ajFmtStr(
        "SELECT "
        "subsnp_handle.handle "
        "FROM "
        "subsnp_handle "
        "WHERE "
        "subsnp_handle.subsnp_id = %u "
        /* NOTE: LIMIT 1 is MySQL-specific. */
        "LIMIT 1",
        gva->Subidentifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        handle = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &handle);

        ajStrAssignS(Psubhandle, handle);

        ajStrDel(&handle);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

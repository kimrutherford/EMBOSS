/* @source ensgvvariation *****************************************************
**
** Ensembl Genetic Variation Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.36 $
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

#include "ensgene.h"
#include "ensgvallele.h"
#include "ensgvattribute.h"
#include "ensgvbaseadaptor.h"
#include "ensgvdatabaseadaptor.h"
#include "ensgvpopulation.h"
#include "ensgvsource.h"
#include "ensgvsynonym.h"
#include "ensgvvariation.h"
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

/* @conststatic gvvariationKClass *********************************************
**
** The Ensembl Genetic Variation Variation class member is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvvariationClass and the
** "variation.class_so_id" field.
**
** #attr [static const char *const*] gvvariationKClass
** ##
******************************************************************************/

static const char *const gvvariationKClass[] =
{
    "",
    "SO:0001483", /* SNV */
    "SO:1000002", /* substitution */
    "SO:0000667", /* insertion */
    "SO:0000159", /* deletion */
    "SO:0000705", /* tandem_repeat */
    "SO:1000032", /* indel */
    "SO:0001059", /* sequence_alteration */
    "SO:0001019", /* copy_number_variation */
    (const char *) NULL
};




/* @conststatic gvvariationKValidation ****************************************
**
** The Ensembl Genetic Variation Variation validation state member is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvvariationValidation and the
** 'variation.validation_status' field.
**
** #attr [static const char *const*] gvvariationKValidation
** ##
******************************************************************************/

static const char *const gvvariationKValidation[] =
{
    "",
    "cluster",
    "freq",
    "submitter",
    "doublehit",
    "hapmap",
    "1000Genome",
    "failed",
    "precious",
    (const char *) NULL
};




/* @conststatic gvvariationadaptorKTablenames *********************************
**
** Array of Ensembl Genetic Variation Variation Adaptor
** SQL table names
**
******************************************************************************/

static const char *const gvvariationadaptorKTablenames[] =
{
    "variation",
    "source source1",
    "variation_synonym",
    "source source2",
    (const char *) NULL
};




/* @conststatic gvvariationadaptorKColumnnames ********************************
**
** Array of Ensembl Genetic Variation Variation Adaptor
** SQL column names
**
******************************************************************************/

static const char *const gvvariationadaptorKColumnnames[] =
{
    "variation.variation_id",
    "variation.source_id",
    "variation.v.name",
    "variation.validation_status",
    "variation.ancestral_allele",
    "variation.class_attrib_id",
    "variation.somatic",
    "variation.flipped",
    "variation.minor_allele",
    "variation.minor_count",
    "variation.minor_freq",
    "variation.clinical_significance_attrib_id",
    "variation_synonym.variation_synonym_id",
    "variation_synonym.subsnp_id",
    "variation_synonym.source_id",
    "variation_synonym.name",
    "variation_synonym.moltype",
    (const char *) NULL
};




/* @conststatic gvvariationadaptorKLeftjoins **********************************
**
** Array of Ensembl Genetic Variation Adaptor SQL LEFT JOIN conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin gvvariationadaptorKLeftjoins[] =
{
    {
        "variation_synonym",
        "variation.variation_id = variation_synonym.variation_id"
    },
    {
        "source source2",
        "variation_synonym.source_id = source2.source_id",
    },
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic gvvariationadaptorKDefaultcondition ***************************
**
** Ensembl Genetic Variation Variation Adaptor SQL SELECT default condition
**
******************************************************************************/

static const char *gvvariationadaptorKDefaultcondition =
    "variation.source_id = source1.source_id";




/* @conststatic gvvariationfeatureadaptorKTablenames **************************
**
** Array of Ensembl Genetic Variation Variation Feature Adaptor
** SQL table names
**
******************************************************************************/

static const char *const gvvariationfeatureadaptorKTablenames[] =
{
    "variation_feature",
    "source",
    "failed_variation",
    (const char *) NULL
};




/* @conststatic gvvariationfeatureadaptorKColumnnames *************************
**
** Array of Ensembl Genetic Variation Variation Feature Adaptor
** SQL column names
**
******************************************************************************/

static const char *const gvvariationfeatureadaptorKColumnnames[] =
{
    "variation_feature.variation_feature_id",
    "variation_feature.seq_region_id",
    "variation_feature.seq_region_start",
    "variation_feature.seq_region_end",
    "variation_feature.seq_region_strand",
    "variation_feature.variation_id",
    "variation_feature.allele_string",
    "variation_feature.variation_name",
    "variation_feature.map_weight",
    "variation_feature.source_id",
    "variation_feature.validation_status",
    "variation_feature.consequence_type",
    "variation_feature.class_so_id",
    (const char *) NULL
};




/* @conststatic gvvariationfeatureadaptorKLeftjoins ***************************
**
** Array of Ensembl Genetic Variation Feature Adaptor SQL LEFT JOIN conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin gvvariationfeatureadaptorKLeftjoins[] =
{
    {
        "failed_variation",
        "variation_feature.variation_id = failed_variation.variation_id"
    },
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic gvvariationfeatureadaptorKDefaultcondition ********************
**
** Ensembl Genetic Variation Variation Feature Adaptor
** SQL SELECT default condition
**
******************************************************************************/

static const char *gvvariationfeatureadaptorKDefaultcondition =
    "variation_feature.source_id = source.source_id";




/* @conststatic gvvariationsetadaptorKTablenames ******************************
**
** Array of Ensembl Genetic Variation Variation Set Adaptor
** SQL table names
**
******************************************************************************/

static const char *const gvvariationsetadaptorKTablenames[] =
{
    "variation_set",
    (const char *) NULL
};




/* @conststatic gvvariationsetadaptorKColumnnames *****************************
**
** Array of Ensembl Genetic Variation Variation Set Adaptor
** SQL column names
**
******************************************************************************/

static const char *const gvvariationsetadaptorKColumnnames[] =
{
    "variation_set.variation_set_id",
    "variation_set.name",
    "variation_set.description"
    "variation_set.short_name_attrib_id",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */

/* #varstatic array elements **************************************************
**
** #value [ajuint] gvvariationadaptorGColumnnamesElements
** Number of gvvariationsetadaptorKColumnnames elements
** #value [ajuint] gvvariationadaptorGTablenamesElements
** Number of gvvariationsetadaptorKTable elements
** #value [ajuint] gvvariationadaptorGLeftjoinsElements
** Number of gvvariationsetadaptorKLeftjoins elements
**
******************************************************************************/

static ajuint gvvariationadaptorGColumnnamesElements = 0U;
static ajuint gvvariationadaptorGTablenamesElements  = 0U;
static ajuint gvvariationadaptorGLeftjoinsElements   = 0U;




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listGvvariationCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static void listGvvariationDelete(void **Pitem, void *cl);

static AjBool gvvariationadaptorFetchAllbyConstraint(
    EnsPGvvariationadaptor gvva,
    const AjPStr constraint,
    AjBool joinallele,
    AjBool joinflank,
    AjBool joinsource,
    AjPList gvvs);

static AjBool gvvariationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvs);

static AjBool gvvariationadaptorFetchAllbyNames(
    EnsPGvvariationadaptor gvva,
    AjBool synonym,
    AjBool subsnp,
    AjPTable gvvnamestogvv,
    AjPTable *Pgvvidentifiers);

static AjBool gvvariationadaptorRetrieveAllFaileddescriptions(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    const AjPStr constraint,
    AjPTable fvs);

static AjBool gvvariationadaptorRetrieveFlankFromCore(
    EnsPGvvariationadaptor gvva,
    ajuint srid,
    ajint srstart,
    ajint srend,
    ajint srstrand,
    AjPStr *Psequence);

static int listGvvariationfeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listGvvariationfeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listGvvariationfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listGvvariationfeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listGvvariationfeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool gvvariationfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvfs);

static AjBool gvvariationsetadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvss);

static int listGvvariationsetCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static void listGvvariationsetDelete(void **Pitem, void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvvariation ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvvariation] Ensembl Genetic Variation Variation *********
**
** @nam2rule Gvvariation Functions for manipulating
** Ensembl Genetic Variation Variation objects
**
** @cc Bio::EnsEMBL::Variation::Variation
** @cc CVS Revision: 1.68
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation by
** pointer. It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @argrule Ini gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvaclass [EnsPGvattribute] Class
** @argrule Ini gvaclinical [EnsPGvattribute] Clinical Significance
** @argrule Ini gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Ini name [AjPStr] Name
** @argrule Ini ancestralallele [AjPStr] Ancestral allele
** @argrule Ini gvalleles [AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects
** @argrule Ini gvsynonyms [AjPList]
** AJAX List of Ensembl Genetic Variation Synonym objects
** @argrule Ini validationstates [AjPStr]
** Comma-separated list of validation states
** @argrule Ini moltype [AjPStr] Molecule type
** @argrule Ini flankfive [AjPStr] Five-prime flanking sequence
** @argrule Ini flankthree [AjPStr] Three-prime flanking sequence
** @argrule Ini flankexists [AjBool] Flanking sequence exists
** @argrule Ini flipped [AjBool] Flipped from reverse to forward strand
** @argrule Ini somatic [AjBool] Somatic or germline flag
** @argrule Ini maallele [AjPStr] Minor allele allele
** @argrule Ini macount [ajuint] Minor allele count
** @argrule Ini mafrequency [float] Minor allele frequency
** @argrule Ref gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @valrule * [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewCpy(const EnsPGvvariation gvv)
{
    AjIList iter = NULL;

    AjPStr description = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    EnsPGvvariation pthis = NULL;

    if (!gvv)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvv->Identifier;
    pthis->Adaptor    = gvv->Adaptor;
    pthis->Class      = ensGvattributeNewRef(gvv->Class);
    pthis->Clinical   = ensGvattributeNewRef(gvv->Clinical);
    pthis->Gvsource   = ensGvsourceNewRef(gvv->Gvsource);

    if (gvv->Name)
        pthis->Name = ajStrNewRef(gvv->Name);

    if (gvv->Ancestralallele)
        pthis->Ancestralallele = ajStrNewRef(gvv->Ancestralallele);

    if (gvv->MinoralleleAllele)
        pthis->MinoralleleAllele = ajStrNewRef(gvv->MinoralleleAllele);

    /*
    ** NOTE: Copy the AJAX List of
    ** Ensembl Genetic Variation Allele objects.
    */

    if (gvv->Gvalleles)
    {
        pthis->Gvalleles = ajListNew();

        iter = ajListIterNew(gvv->Gvalleles);

        while (!ajListIterDone(iter))
        {
            gva = (EnsPGvallele) ajListIterGet(iter);

            ajListPushAppend(pthis->Gvalleles,
                             (void *) ensGvalleleNewRef(gva));
        }

        ajListIterDel(&iter);
    }

    /*
    ** NOTE: Copy the AJAX List of
    ** Ensembl Genetic Variation Synonym objects.
    */

    if (gvv->Gvsynonyms)
    {
        pthis->Gvsynonyms = ajListNew();

        iter = ajListIterNew(gvv->Gvsynonyms);

        while (!ajListIterDone(iter))
        {
            gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

            ajListPushAppend(pthis->Gvsynonyms,
                             (void *) ensGvsynonymNewRef(gvsynonym));
        }

        ajListIterDel(&iter);
    }

    /* NOTE: Copy the AJAX List of AJAX String (failed description) objects. */

    if (gvv->Faileddescriptions)
    {
        pthis->Faileddescriptions = ajListstrNew();

        iter = ajListIterNew(gvv->Faileddescriptions);

        while (!ajListIterDone(iter))
        {
            description = ajListstrIterGet(iter);

            if (description)
                ajListstrPushAppend(pthis->Faileddescriptions,
                                    ajStrNewS(description));
        }

        ajListIterDel(&iter);
    }

    if (gvv->Moleculetype)
        pthis->Moleculetype = ajStrNewRef(gvv->Moleculetype);

    if (gvv->FlankFive)
        pthis->FlankFive = ajStrNewRef(gvv->FlankFive);

    if (gvv->FlankThree)
        pthis->FlankThree = ajStrNewRef(gvv->FlankThree);

    pthis->FlankExists = gvv->FlankExists;
    pthis->Somatic     = gvv->Somatic;
    pthis->Flipped     = gvv->Flipped;
    pthis->Validations = gvv->Validations;

    pthis->MinoralleleCount     = gvv->MinoralleleCount;
    pthis->MinoralleleFrequency = gvv->MinoralleleFrequency;

    return pthis;
}




/* @func ensGvvariationNewIni *************************************************
**
** Constructor for an Ensembl Genetic Variation Variation with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Variation::new
** @param [u] gvaclass [EnsPGvattribute] Class
** @param [u] gvaclinical [EnsPGvattribute] Clinical significance
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] name [AjPStr] Name
** @param [uN] ancestralallele [AjPStr] Ancestral allele
** @param [uN] gvalleles [AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects
** @param [uN] gvsynonyms [AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects
** @param [uN] validationstates [AjPStr]
** Comma-separated list of validation states
** @param [uN] moltype [AjPStr] Molecule type
** @param [uN] flankfive [AjPStr] Five-prime flanking sequence
** @param [uN] flankthree [AjPStr] Three-prime flanking sequence
** @param [r] flankexists [AjBool] Flanking sequence exists
** @param [r] flipped [AjBool] Flipped from reverse to forward strand
** @param [r] somatic [AjBool] Somatic or germline flag
** @param [u] maallele [AjPStr] Minor allele allele
** @param [r] macount [ajuint] Minor allele count
** @param [r] mafrequency [float] Minor allele frequency
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewIni(EnsPGvvariationadaptor gvva,
                                     ajuint identifier,
                                     EnsPGvattribute gvaclass,
                                     EnsPGvattribute gvaclinical,
                                     EnsPGvsource gvsource,
                                     AjPStr name,
                                     AjPStr ancestralallele,
                                     AjPList gvalleles,
                                     AjPList gvsynonyms,
                                     AjPStr validationstates,
                                     AjPStr moltype,
                                     AjPStr flankfive,
                                     AjPStr flankthree,
                                     AjBool flankexists,
                                     AjBool flipped,
                                     AjBool somatic,
                                     AjPStr maallele,
                                     ajuint macount,
                                     float mafrequency)
{
    AjIList iter = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    EnsPGvvariation gvv = NULL;

    if (!gvsource)
        return NULL;

    if (!name)
        return NULL;

    AJNEW0(gvv);

    gvv->Use        = 1U;
    gvv->Identifier = identifier;
    gvv->Adaptor    = gvva;
    gvv->Class      = ensGvattributeNewRef(gvaclass);
    gvv->Clinical   = ensGvattributeNewRef(gvaclinical);
    gvv->Gvsource   = ensGvsourceNewRef(gvsource);

    if (name)
        gvv->Name = ajStrNewRef(name);

    if (ancestralallele)
        gvv->Ancestralallele = ajStrNewRef(ancestralallele);

    if (maallele)
        gvv->MinoralleleAllele = ajStrNewRef(maallele);

    /* Copy the AJAX List of Ensembl Genetic Variation Allele objects. */

    if (gvalleles)
    {
        gvv->Gvalleles = ajListNew();

        iter = ajListIterNew(gvalleles);

        while (!ajListIterDone(iter))
        {
            gva = (EnsPGvallele) ajListIterGet(iter);

            ajListPushAppend(gvv->Gvalleles,
                             (void *) ensGvalleleNewRef(gva));
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of Ensembl Genetic Variation Synonym objects. */

    if (gvsynonyms)
    {
        gvv->Gvsynonyms = ajListNew();

        iter = ajListIterNew(gvsynonyms);

        while (!ajListIterDone(iter))
        {
            gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

            ajListPushAppend(gvv->Gvsynonyms,
                             (void *) ensGvsynonymNewRef(gvsynonym));
        }

        ajListIterDel(&iter);
    }

    if (moltype)
        gvv->Moleculetype = ajStrNewRef(moltype);

    if (flankfive)
        gvv->FlankFive = ajStrNewRef(flankfive);

    if (flankthree)
        gvv->FlankThree = ajStrNewRef(flankthree);

    gvv->FlankExists = flankexists;
    gvv->Somatic     = somatic;
    gvv->Flipped     = flipped;
    gvv->Validations = ensGvvariationValidationsFromSet(validationstates);

    gvv->MinoralleleCount     = macount;
    gvv->MinoralleleFrequency = mafrequency;

    return gvv;
}




/* @func ensGvvariationNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewRef(EnsPGvvariation gvv)
{
    if (!gvv)
        return NULL;

    gvv->Use++;

    return gvv;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Variation
**
** @argrule * Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationDel ****************************************************
**
** Default destructor for an Ensembl Genetic Variation Variation.
**
** @param [d] Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvvariationDel(EnsPGvvariation *Pgvv)
{
    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    EnsPGvvariation pthis = NULL;

    if (!Pgvv)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvvariationDel"))
    {
        ajDebug("ensGvvariationDel\n"
                "  *Pgvv %p\n",
                *Pgvv);

        ensGvvariationTrace(*Pgvv, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvv) || --pthis->Use)
    {
        *Pgvv = NULL;

        return;
    }

    ensGvattributeDel(&pthis->Class);
    ensGvattributeDel(&pthis->Clinical);

    ensGvsourceDel(&pthis->Gvsource);

    ajStrDel(&pthis->Name);

    ajStrDel(&pthis->Ancestralallele);
    ajStrDel(&pthis->MinoralleleAllele);

    /* Clear the AJAX List of Ensembl Genetic Variation Allele objects. */

    while (ajListPop(pthis->Gvalleles, (void **) &gva))
        ensGvalleleDel(&gva);

    ajListFree(&pthis->Gvalleles);

    /* Clear the AJAX List of Ensembl Genetic Variation Synonym objects. */

    while (ajListPop(pthis->Gvsynonyms, (void **) &gvsynonym))
        ensGvsynonymDel(&gvsynonym);

    ajListFree(&pthis->Gvsynonyms);

    /* Clear the AJAX List of AJAX String (failed description) objects. */

    ajListstrFreeData(&pthis->Faileddescriptions);

    ajStrDel(&pthis->Moleculetype);
    ajStrDel(&pthis->FlankFive);
    ajStrDel(&pthis->FlankThree);

    ajMemFree((void **) Pgvv);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Get Return Genetic Variation Variation attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Variation Adaptor
** @nam4rule Ancestralallele Return the ancestral allele
** @nam4rule Class Return the Class Ensembl Genetic Variation Attribute
** @nam4rule Clinical
** Return the Clinical Significance Ensembl Genetic Variation Attribute
** @nam4rule Flank Return flanking sequence members
** @nam4rule Flipped Return the flipped member
** @nam5rule Exists Return the flanking sequence exists
** @nam5rule Five Return the five-prime flanking sequence
** @nam5rule Three Return the three-prime flanking sequence
** @nam4rule Gvalleles Return Ensembl Genetic Variation Allele objects
** @nam4rule Gvsource Return the Ensembl Genetic Variation Source
** @nam4rule Gvsynonyms Return Ensembl Genetic Variation Synonms
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Minorallele Return minor allele members
** @nam5rule Allele Return the minor allele allele
** @nam5rule Count Return the minor allele count
** @nam5rule Frequency Return the minor allele frequency
** @nam4rule Moleculetype Return the molecule type
** @nam4rule Name Return the name
** @nam4rule Somatic Return the somatic or germline flag
** @nam4rule Validations Return the validation state bit field
**
** @argrule * gvv [const EnsPGvvariation] Genetic Variation Variation
**
** @valrule Adaptor [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                           Variation Adaptor or NULL
** @valrule Ancestralallele [AjPStr] Ancestral allele or NULL
** @valrule Class [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute or NULL
** @valrule Clinical [EnsPGvattribute]
** Clinical significance Ensembl Genetic Variation Attribute
** @valrule FlankFive [AjPStr] Five-prime flanking sequence or NULL
** @valrule FlankThree [AjPStr] Three-prime flanking sequence or NULL
** @valrule FlankExists [AjBool] Flanking sequence exists or ajFalse
** @valrule Flipped [AjBool] Flipped member or ajFalse
** @valrule Gvalleles [const AjPList]
** Ensembl Genetic Variation Allele objects or NULL
** @valrule Gvsource [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @valrule Gvsynonyms [const AjPList]
** Ensembl Genetic Variation Synonym objects or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Moleculetype [AjPStr] Molecule type or NULL
** @valrule MinoralleleAllele [AjPStr] Minor allele allele or NULL
** @valrule MinoralleleCount [ajuint] Minor allele count or 0U
** @valrule MinoralleleFrequency [float] Minor allele frequency or 0.0F
** @valrule Name [AjPStr] Name or NULL
** @valrule Somatic [AjBool] Somatic or germline flag or ajFalse
** @valrule Validations [ajuint] Validations bit field or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationGetAdaptor *********************************************
**
** Get the Ensembl Genetic Variation Variation Adaptor member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvvariationadaptor ensGvvariationGetAdaptor(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Adaptor : NULL;
}




/* @func ensGvvariationGetAncestralallele *************************************
**
** Get the ancestral allele member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::ancestral_allele
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Ancestral allele or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationGetAncestralallele(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Ancestralallele : NULL;
}




/* @func ensGvvariationGetClass ***********************************************
**
** Get the class Ensembl Genetic Variation Attribute member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::var_class
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvattribute] Class Ensembl Genetic Variation Attribute or NULL
**
** @release 6.4.0
** @@
** FIXME: The Perl Bio::EnsEMBL::Variation::Variation::var_class method is not
** a simple accessor method, it rather assembles the class from
** Ensembl Genetic Variation Allele objects, but depends on
** Bio::EnsEMBL::Variation::Utils::Sequence::SO_variation_class.
******************************************************************************/

EnsPGvattribute ensGvvariationGetClass(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Class : NULL;
}




/* @func ensGvvariationGetClinical ********************************************
**
** Get the clinical significance Ensembl Genetic Variation Attribute member
** of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::clinical_significance
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvattribute]
** Clinical significance Ensembl Genetic Variation Attribute or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvattribute ensGvvariationGetClinical(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Clinical : NULL;
}




/* @func ensGvvariationGetFlankExists *****************************************
**
** Get the flanking sequence exists member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::flank_flag
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] Flanking sequence exists or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationGetFlankExists(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->FlankExists : ajFalse;
}




/* @func ensGvvariationGetFlipped *********************************************
**
** Get the flipped member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::flipped
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] Flipped member or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationGetFlipped(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Flipped : ajFalse;
}




/* @func ensGvvariationGetGvalleles *******************************************
**
** Get all Ensembl Genetic Variation Allele objects of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_Alleles
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [const AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPList ensGvvariationGetGvalleles(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Gvalleles : NULL;
}




/* @func ensGvvariationGetGvsource ********************************************
**
** Get the Ensembl Genetic Variation Source member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::source
** @cc Bio::EnsEMBL::Variation::Variation::source_description
** @cc Bio::EnsEMBL::Variation::Variation::source_url
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPGvsource ensGvvariationGetGvsource(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Gvsource : NULL;
}




/* @func ensGvvariationGetGvsynonyms ******************************************
**
** Get the AJAX List of Ensembl Genetic Variation Synonym objects member
** of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_synonyms
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @see ensGvvariationFetchAllGvsynonyms
** @see ensGvvariationFetchAllGvsources
**
** @return [const AjPList]
** AJAX List of Ensembl Genetic Variation Synonym objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGvvariationGetGvsynonyms(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Gvsynonyms : NULL;
}




/* @func ensGvvariationGetIdentifier ******************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvvariationGetIdentifier(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Identifier : 0U;
}




/* @func ensGvvariationGetMinoralleleAllele ***********************************
**
** Get the minor allele allele member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::minor_allele
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Minor allele allele or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensGvvariationGetMinoralleleAllele(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->MinoralleleAllele : NULL;
}




/* @func ensGvvariationGetMinoralleleCount ************************************
**
** Get the minor allele count member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::minor_allele_count
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] Minor allele count or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensGvvariationGetMinoralleleCount(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->MinoralleleCount : 0U;
}




/* @func ensGvvariationGetMinoralleleFrequency ********************************
**
** Get the minor allele frequency member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::minor_allele_frequency
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [float] Minor allele frequency or 0.0F
**
** @release 6.5.0
** @@
******************************************************************************/

float ensGvvariationGetMinoralleleFrequency(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->MinoralleleFrequency : 0.0F;
}




/* @func ensGvvariationGetMoleculetype ****************************************
**
** Get the molecule type member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::moltype
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Molecule type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationGetMoleculetype(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Moleculetype : NULL;
}




/* @func ensGvvariationGetName ************************************************
**
** Get the name member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::name
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGvvariationGetName(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Name : NULL;
}




/* @func ensGvvariationGetSomatic *********************************************
**
** Get the somatic or germline member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] Somatic or germline flag or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationGetSomatic(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Somatic : ajFalse;
}




/* @func ensGvvariationGetValidations *****************************************
**
** Get the validations bit field of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_validation_states
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] Validations bit field or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvvariationGetValidations(const EnsPGvvariation gvv)
{
    return (gvv) ? gvv->Validations : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Genetic Variation Variation
** object, which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Load Return Ensembl Genetic Variation Variation attribute(s)
** loaded on demand
** @nam4rule All Returns all Ensembl Genetic Variation Variation members
** loaded on demand
** @nam5rule Faileddescriptions Returns an AJAX List of AJAX String
** (failed description) objects
** @nam5rule Gvalleles
** Return an AJAX List of Ensembl Genetic Variation Allele obects
** @nam4rule Flank Return a flanking sequence
** @nam5rule Five Return the five-prime flanking sequence
** @nam5rule Three Return the three-prime flanking sequence
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @valrule Faileddescriptions [const AjPList]
** AJAX List of AJAX String (failed description) objects
** @valrule Gvalleles [const AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects
** @valrule Flank [AjPStr] Flanking sequence or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationLoadAllFaileddescriptions ******************************
**
** Load all failed descriptions of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_failed_descriptions
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [const AjPList]
** AJAX List of AJAX String (failed description) objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGvvariationLoadAllFaileddescriptions(EnsPGvvariation gvv)
{
    if (!gvv)
        return ajFalse;

    if (gvv->Faileddescriptions)
        return gvv->Faileddescriptions;

    if (!gvv->Adaptor)
    {
        ajDebug("ensGvvariationLoadAllFaileddescriptions cannot retrieve "
                "AJAX String (failed description) objects for an "
                "Ensembl Genetic Variation Variation without an "
                "Ensembl Genetic Variation Variation Adaptor.\n");

        return NULL;
    }

    gvv->Faileddescriptions = ajListstrNew();

    ensGvvariationadaptorRetrieveAllFaileddescriptions(
        gvv->Adaptor,
        gvv->Identifier,
        gvv->Faileddescriptions);

    return gvv->Faileddescriptions;
}




/* @func ensGvvariationLoadAllGvalleles ***************************************
**
** Load all Ensembl Genetic Variation Allele objects of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_Alleles
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [const AjPList]
** AJAX List of Ensembl Genetic Variation Allele objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGvvariationLoadAllGvalleles(EnsPGvvariation gvv)
{
    if (!gvv)
        return NULL;

    if (gvv->Gvalleles)
        return gvv->Gvalleles;

    if (!gvv->Adaptor)
    {
        ajDebug("ensGvvariationLoadAllGvalleles cannot retrieve "
                "Ensembl Genetic Variation Allele objects for an "
                "Ensembl Genetic Variation Variation without an "
                "Ensembl Genetic Variation Variation Adaptor.\n");

        return NULL;
    }

    gvv->Gvalleles = ajListNew();

    ensGvalleleadaptorFetchAllbyGvvariation(
        ensRegistryGetGvalleleadaptor(
            ensGvvariationadaptorGetDatabaseadaptor(gvv->Adaptor)),
        gvv,
        (EnsPGvpopulation) NULL,
        gvv->Gvalleles);

    return gvv->Gvalleles;
}




/* @func ensGvvariationLoadFlankFive ******************************************
**
** Get the five-prime flank member of an Ensembl Genetic Variation Variation.
**
** This is not a simple accessor function, it will fetch the five-prime
** flanking sequence from the Ensembl Genetic Variation database in case it is
** not defined.
**
** @cc Bio::EnsEMBL::Variation::Variation::five_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Five-prime flank or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationLoadFlankFive(EnsPGvvariation gvv)
{
    if (!gvv)
        return NULL;

    if (gvv->FlankFive)
        return gvv->FlankFive;

    if (gvv->Adaptor)
        ensGvvariationadaptorRetrieveFlank(
            gvv->Adaptor,
            gvv->Identifier,
            &gvv->FlankFive,
            &gvv->FlankThree);

    return gvv->FlankFive;
}




/* @func ensGvvariationLoadFlankThree *****************************************
**
** Get the three-prime flank member of an Ensembl Genetic Variation Variation.
**
** This is not a simple accessor function, it will fetch the three-prime
** flanking sequence from the Ensembl Genetic Variation database in case it is
** not defined.
**
** @cc Bio::EnsEMBL::Variation::Variation::three_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Three-prime flank or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationLoadFlankThree(EnsPGvvariation gvv)
{
    if (!gvv)
        return NULL;

    if (gvv->FlankThree)
        return gvv->FlankThree;

    if (gvv->Adaptor)
        ensGvvariationadaptorRetrieveFlank(
            gvv->Adaptor,
            gvv->Identifier,
            &gvv->FlankFive,
            &gvv->FlankThree);

    return gvv->FlankThree;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Set Set one member of a Genetic Variation Variation
** @nam4rule Adaptor Set the Ensembl Genetic Variation Variation Adaptor
** @nam4rule Ancestralallele Set the ancestral allele
** @nam4rule Class Set the
** Class Ensembl Genetic Variation Attribute
** @nam4rule Clinical Set the
** Clinical significance Ensembl Genetic Variation Attribute
** @nam4rule Flank Set flanking sequence members
** @nam5rule Exists Set the flanking sequence exists
** @nam5rule Five Set the five-prime flank
** @nam5rule Three Set the three-prime flank
** @nam4rule Flipped Set the flipped member
** @nam4rule Gvsource Set the Ensembl Genetic Variation Source
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Minorallele Set minor allele members
** @nam5rule Allele Set the minor allele allele
** @nam5rule Count Set the minor allele count
** @nam5rule Frequency Set the minor allele frequency
** @nam4rule Moleculetype Set the molecule type
** @nam4rule Name Set the name
** @nam4rule Somatic Set the somatic or germline flag
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation object
** @argrule Adaptor gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
** @argrule Ancestralallele ancestralallele [AjPStr] Ancestral allele
** @argrule Class gvaclass [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
** @argrule Clinical gvaclinical [EnsPGvattribute]
** Clinical significance Ensembl Genetic Variation Attribute
** @argrule FlankExists flankexists [AjBool] Flank
** @argrule FlankFive flankfive [AjPStr] Five-prime flank
** @argrule FlankThree flankthree [AjPStr] Three-prime flank
** @argrule Flipped flipped [AjBool] Flipped member
** @argrule Gvsource gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule MinoralleleAllele maallele [AjPStr] Minor allele allele
** @argrule MinoralleleCount macount [ajuint] Minot allele count
** @argrule MinoralleleFrequency mafrequency [float] Minor allele frequency
** @argrule Moleculetype moltype [AjPStr] Molecule type
** @argrule Name name [AjPStr] Name
** @argrule Somatic somatic [AjBool] Somatic or germline flag
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationSetAdaptor *********************************************
**
** Set the Ensembl Genetic Variation Variation Adaptor member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationSetAdaptor(EnsPGvvariation gvv,
                                EnsPGvvariationadaptor gvva)
{
    if (!gvv)
        return ajFalse;

    gvv->Adaptor = gvva;

    return ajTrue;
}




/* @func ensGvvariationSetAncestralallele *************************************
**
** Set the ancestral allele member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::ancestral_allele
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] ancestralallele [AjPStr] Ancestral allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetAncestralallele(EnsPGvvariation gvv,
                                        AjPStr ancestralallele)
{
    if (!gvv)
        return ajFalse;

    ajStrDel(&gvv->Ancestralallele);

    if (ancestralallele)
        gvv->Ancestralallele = ajStrNewRef(ancestralallele);

    return ajTrue;
}




/* @func ensGvvariationSetClass ***********************************************
**
** Set the class Ensembl Genetic Variation Attribute member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::var_class
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvaclass [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetClass(EnsPGvvariation gvv,
                              EnsPGvattribute gvaclass)
{
    if (!gvv)
        return ajFalse;

    ensGvattributeDel(&gvv->Class);

    gvv->Class = ensGvattributeNewRef(gvaclass);

    return ajTrue;
}




/* @func ensGvvariationSetClinical ********************************************
**
** Set the clinical significance Ensembl Genetic Variation Attribute member
** of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::clinical_significance
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvaclinical [EnsPGvattribute]
** Clinical significance Ensembl Genetic Variation Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetClinical(EnsPGvvariation gvv,
                                 EnsPGvattribute gvaclinical)
{
    if (!gvv)
        return ajFalse;

    ensGvattributeDel(&gvv->Clinical);

    gvv->Clinical = ensGvattributeNewRef(gvaclinical);

    return ajTrue;
}




/* @func ensGvvariationSetFlankExists *****************************************
**
** Set the flanking sequence exists member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::flank_flag
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] flankexists [AjBool] Flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetFlankExists(EnsPGvvariation gvv,
                                    AjBool flankexists)
{
    if (!gvv)
        return ajFalse;

    gvv->FlankExists = flankexists;

    return ajTrue;
}




/* @func ensGvvariationSetFlankFive *******************************************
**
** Set the five-prime flank member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::five_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] flankfive [AjPStr] Five-prime flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetFlankFive(EnsPGvvariation gvv,
                                  AjPStr flankfive)
{
    if (!gvv)
        return ajFalse;

    ajStrDel(&gvv->FlankFive);

    if (flankfive)
        gvv->FlankFive = ajStrNewRef(flankfive);

    return ajTrue;
}




/* @func ensGvvariationSetFlankThree ******************************************
**
** Set the three-prime flank member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::three_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] flankthree [AjPStr] Three-prime flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetFlankThree(EnsPGvvariation gvv,
                                   AjPStr flankthree)
{
    if (!gvv)
        return ajFalse;

    ajStrDel(&gvv->FlankThree);

    if (flankthree)
        gvv->FlankThree = ajStrNewRef(flankthree);

    return ajTrue;
}




/* @func ensGvvariationSetFlipped *********************************************
**
** Set the flipped member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::flipped
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] flipped [AjBool] Flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetFlipped(EnsPGvvariation gvv,
                                AjBool flipped)
{
    if (!gvv)
        return ajFalse;

    gvv->Flipped = flipped;

    return ajTrue;
}




/* @func ensGvvariationSetGvsource ********************************************
**
** Set the Ensembl Genetic Variation Source member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::source
** @cc Bio::EnsEMBL::Variation::Variation::source_description
** @cc Bio::EnsEMBL::Variation::Variation::source_url
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvvariationSetGvsource(EnsPGvvariation gvv,
                                 EnsPGvsource gvs)
{
    if (!gvv)
        return ajFalse;

    ensGvsourceDel(&gvv->Gvsource);

    gvv->Gvsource = ensGvsourceNewRef(gvs);

    return ajTrue;
}




/* @func ensGvvariationSetIdentifier ******************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationSetIdentifier(EnsPGvvariation gvv,
                                   ajuint identifier)
{
    if (!gvv)
        return ajFalse;

    gvv->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationSetMinoralleleAllele ***********************************
**
** Set the minor allele allele member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::minor_allele
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] maallele [AjPStr] Minor allele allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetMinoralleleAllele(EnsPGvvariation gvv,
                                          AjPStr maallele)
{
    if (!gvv)
        return ajFalse;

    ajStrDel(&gvv->MinoralleleAllele);

    if (maallele)
        gvv->MinoralleleAllele = ajStrNewRef(maallele);

    return ajTrue;
}




/* @func ensGvvariationSetMinoralleleCount ************************************
**
** Set the minor allele count member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::minor_allele_count
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] macount [ajuint] Minor allele count
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetMinoralleleCount(EnsPGvvariation gvv,
                                         ajuint macount)
{
    if (!gvv)
        return ajFalse;

    gvv->MinoralleleCount = macount;

    return ajTrue;
}




/* @func ensGvvariationSetMinoralleleFrequency ********************************
**
** Set the minor allele frequency member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::minor_allele_frequency
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] mafrequency [float] Minor allele frequency
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetMinoralleleFrequency(EnsPGvvariation gvv,
                                             float mafrequency)
{
    if (!gvv)
        return ajFalse;

    gvv->MinoralleleFrequency = mafrequency;

    return ajTrue;
}




/* @func ensGvvariationSetMoleculetype ****************************************
**
** Set the molecule type member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::moltype
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] moltype [AjPStr] Molecule type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetMoleculetype(EnsPGvvariation gvv,
                                     AjPStr moltype)
{
    if (!gvv)
        return ajFalse;

    ajStrDel(&gvv->Moleculetype);

    if (moltype)
        gvv->Moleculetype = ajStrNewRef(moltype);

    return ajTrue;
}




/* @func ensGvvariationSetName ************************************************
**
** Set the name member of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::name
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationSetName(EnsPGvvariation gvv,
                             AjPStr name)
{
    if (!gvv)
        return ajFalse;

    ajStrDel(&gvv->Name);

    if (name)
        gvv->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationSetSomatic *********************************************
**
** Set the somatic or germline member of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] somatic [AjBool] Somatic or germline flag
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationSetSomatic(EnsPGvvariation gvv,
                                AjBool somatic)
{
    if (!gvv)
        return ajFalse;

    gvv->Somatic = somatic;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Trace Report Ensembl Genetic Variation Variation members
**                 to debug file
**
** @argrule Trace gvv [const EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationTrace **************************************************
**
** Trace an Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationTrace(const EnsPGvvariation gvv, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr states = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvs = NULL;

    if (!gvv)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvvariationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Class %p\n"
            "%S  Clinical %p\n"
            "%S  Gvsource %p\n"
            "%S  Name '%S'\n"
            "%S  Ancestralallele '%S'\n"
            "%S  MinoralleleAllele '%S'\n"
            "%S  Gvalleles %p\n"
            "%S  Gvsynonyms %p\n"
            "%S  Moleculetype '%S'\n"
            "%S  FlankFive '%S'\n"
            "%S  FlankThree '%S'\n"
            "%S  FlankExists '%B'\n"
            "%S  Flipped '%B'\n"
            "%S  Somatic '%B'\n"
            "%S  Validations %u\n"
            "%S  MinoralleleCount %u\n"
            "%S  MinoralleleFrequency %f\n",
            indent, gvv,
            indent, gvv->Use,
            indent, gvv->Identifier,
            indent, gvv->Adaptor,
            indent, gvv->Class,
            indent, gvv->Clinical,
            indent, gvv->Gvsource,
            indent, gvv->Name,
            indent, gvv->Ancestralallele,
            indent, gvv->MinoralleleAllele,
            indent, gvv->Gvalleles,
            indent, gvv->Gvsynonyms,
            indent, gvv->Moleculetype,
            indent, gvv->FlankFive,
            indent, gvv->FlankThree,
            indent, gvv->FlankExists,
            indent, gvv->Flipped,
            indent, gvv->Somatic,
            indent, gvv->Validations,
            indent, gvv->MinoralleleCount,
            indent, gvv->MinoralleleFrequency);

    ensGvattributeTrace(gvv->Class, level + 1);
    ensGvattributeTrace(gvv->Clinical, level + 1);

    ensGvsourceTrace(gvv->Gvsource, level + 1);

    states = ajStrNew();

    ensGvvariationValidationsToSet(gvv->Validations, &states);

    ajDebug("%S  Validation states SQL set: '%S'\n", indent, states);

    ajStrDel(&states);

    /* Trace the AJAX List of Ensembl Genetic Variation Allele objects. */

    ajDebug("%S  Ensembl Genetic Variation Allele objects:\n", indent);

    iter = ajListIterNew(gvv->Gvalleles);

    while (!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        ensGvalleleTrace(gva, level + 1);
    }

    ajListIterDel(&iter);

    /* Trace the AJAX List of Ensembl Genetic Variation Synonym objects. */

    ajDebug("%S  Ensembl Genetic Variation Synonym objects:\n", indent);

    iter = ajListIterNew(gvv->Gvsynonyms);

    while (!ajListIterDone(iter))
    {
        gvs = (EnsPGvsynonym) ajListIterGet(iter);

        ensGvsynonymTrace(gvs, level + 1);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Calculate
** Calculate Ensembl Genetic Variation Variation information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvvariationCalculateMemsize(const EnsPGvvariation gvv)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    if (!gvv)
        return 0;

    size += sizeof (EnsOGvvariation);

    size += ensGvattributeCalculateMemsize(gvv->Class);
    size += ensGvattributeCalculateMemsize(gvv->Clinical);

    size += ensGvsourceCalculateMemsize(gvv->Gvsource);

    if (gvv->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->Name);
    }

    if (gvv->Ancestralallele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->Ancestralallele);
    }

    if (gvv->MinoralleleAllele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->MinoralleleAllele);
    }

    /* Summarise the AJAX List of Ensembl Genetic Variation Allele objects. */

    iter = ajListIterNewread(gvv->Gvalleles);

    while (!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        size += ensGvalleleCalculateMemsize(gva);
    }

    ajListIterDel(&iter);

    /* Summarise the AJAX List of Ensembl Genetic Variation Synonym objects. */

    iter = ajListIterNewread(gvv->Gvsynonyms);

    while (!ajListIterDone(iter))
    {
        gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

        size += ensGvsynonymCalculateMemsize(gvsynonym);
    }

    ajListIterDel(&iter);

    if (gvv->Moleculetype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->Moleculetype);
    }

    if (gvv->FlankFive)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->FlankFive);
    }

    if (gvv->FlankThree)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->FlankThree);
    }

    return size;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Add Add one object to an Ensembl Genetic Variation Variation
** @nam4rule Faileddescription Add a failed description
** @nam4rule Gvallele Add an Ensembl Genetic Variation Allele
** @nam4rule Gvsynonym Add an Ensembl Genetic Variation Synonym
** @nam4rule Validation
** Add an Ensembl Genetic Variation Variation Validation enumeration
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation object
** @argrule Faileddescription description [AjPStr] Failed description
** @argrule Gvallele gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Gvsynonym gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @argrule Validation gvvv [EnsEGvvariationValidation]
** Ensembl Genetic Variation Validation enumeration
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationAddFaileddescription ***********************************
**
** Add a failed description to an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::failed_description
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] description [AjPStr] Failed description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationAddFaileddescription(EnsPGvvariation gvv,
                                          AjPStr description)
{
    if (!gvv)
        return ajFalse;

    if (!description)
        return ajFalse;

    if (!gvv->Faileddescriptions)
        gvv->Faileddescriptions = ajListstrNew();

    ajListstrPushAppend(gvv->Faileddescriptions, ajStrNewS(description));

    return ajTrue;
}




/* @func ensGvvariationAddGvallele ********************************************
**
** Add an Ensembl Genetic Variation Allele to an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::add_Allele
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationAddGvallele(EnsPGvvariation gvv,
                                 EnsPGvallele gva)
{
    if (!gvv)
        return ajFalse;

    if (!gva)
        return ajFalse;

    if (!gvv->Gvalleles)
        gvv->Gvalleles = ajListNew();

    ajListPushAppend(gvv->Gvalleles, (void *) ensGvalleleNewRef(gva));

    return ajTrue;
}




/* @func ensGvvariationAddGvsynonym *******************************************
**
** Add an Ensembl Genetic Variation Synonym to an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::add_synonym
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationAddGvsynonym(EnsPGvvariation gvv,
                                  EnsPGvsynonym gvs)
{
    if (!gvv)
        return ajFalse;

    if (!gvs)
        return ajFalse;

    if (!gvv->Gvsynonyms)
        gvv->Gvsynonyms = ajListNew();

    ajListPushAppend(gvv->Gvsynonyms, (void *) ensGvsynonymNewRef(gvs));

    return ajTrue;
}




/* @func ensGvvariationAddValidation ******************************************
**
** Add a validation state to an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::add_validation_state
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvvv [EnsEGvvariationValidation]
** Ensembl Genetic Variation Variation Validation enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationAddValidation(EnsPGvvariation gvv,
                                   EnsEGvvariationValidation gvvv)
{
    if (!gvv)
        return ajFalse;

    gvv->Validations |= (1 << gvvv);

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching information from an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation information
** @nam4rule All Fetch all objects
** @nam5rule Genes Fetch all Ensembl Gene objects
** @nam5rule Gvsources Fetch all Ensembl Genetic Variation Source objects
** @nam5rule Gvsynonyms Fetch all Ensembl Genetic Variation Synonym objects
** @nam5rule Gvvariationfeatures
** Fetch all Ensembl Genetic Variation Variation Feature objects
**
** @argrule AllGenes gvv [EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule AllGenes flank [ajint]
** Overlap between Gene and Variation Feature objects, defaults to 5000 base
** pairs
** @argrule AllGenes genes [AjPList] AJAX List of Ensembl Gene objects
** @argrule AllGvsources gvv [const EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule AllGvsources gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Source objects
** @argrule AllGvsynonyms gvv [const EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule AllGvsynonyms gvsourcename [const AjPStr]
** Ensembl Genetic Variation Source name
** @argrule AllGvsynonyms gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Synonym objects
** @argrule AllGvvariationfeatures gvv [EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule AllGvvariationfeatures gvvfs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationFetchAllGenes ******************************************
**
** Fetch all Ensembl Gene objects where an Ensembl Genetic Variation Variation
** has a consequence.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_Genes
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] flank [ajint]
** Overlap between Gene and Variation Feature objects, defaults to 5000 base
** pairs
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGenes(EnsPGvvariation gvv,
                                   ajint flank,
                                   AjPList genes)
{
    ajuint *Pidentifier = NULL;

    ajuint identifier = 0U;

    AjPList genelist = NULL;
    AjPList gvvfs    = NULL;

    AjPTable genetable = NULL;

    EnsPDatabaseadaptor csdba = NULL;
    EnsPDatabaseadaptor gvdba = NULL;

    EnsPFeature gfeature = NULL;
    EnsPFeature vfeature = NULL;

    EnsPGene        gene  = NULL;
    EnsPGeneadaptor genea = NULL;

    EnsPGvvariationfeature        gvvf  = NULL;
    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    EnsPSlice    slice   = NULL;
    EnsPSlice newslice   = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!gvv)
        return ajFalse;

    if (!flank)
        flank = 5000;

    if (!genes)
        return ajFalse;

    if (!gvv->Adaptor)
        return ajTrue;

    genetable = ajTableuintNew(0U);

    ajTableSetDestroyvalue(genetable, (void (*)(void **)) &ensGeneDel);

    gvdba = ensGvvariationadaptorGetDatabaseadaptor(gvv->Adaptor);

    csdba = ensRegistryGetDatabaseadaptor(
        ensEDatabaseadaptorGroupCore,
        ensDatabaseadaptorGetSpecies(gvdba));

    sla = ensRegistryGetSliceadaptor(csdba);

    genea = ensRegistryGetGeneadaptor(csdba);

    genelist = ajListNew();

    gvvfa = ensRegistryGetGvvariationfeatureadaptor(gvdba);

    gvvfs = ajListNew();

    ensGvvariationfeatureadaptorFetchAllbyGvvariation(gvvfa, gvv, gvvfs);

    /*
    ** For each Ensembl Genetic Variation Variation Feature, get the Slice
    ** is on, use the USTREAM and DOWNSTREAM limits to get all the Gene
    ** objects, and see if the Ensembl Genetic Variation Variation Feature is
    ** within the gene.
    */

    while (ajListPop(gvvfs, (void **) &gvvf))
    {
        /* Fetch an expanded Feature Slice. */

        vfeature = ensGvvariationfeatureGetFeature(gvvf);

        ensSliceadaptorFetchByFeature(sla, vfeature, flank, &slice);

        /* Fetch all Ensembl Gene objects on the new Ensembl Slice. */

        ensGeneadaptorFetchAllbySlice(genea,
                                      newslice,
                                      (AjPStr) NULL,
                                      (AjPStr) NULL,
                                      (AjPStr) NULL,
                                      ajFalse,
                                      genelist);

        while (ajListPop(genelist, (void **) &gene))
        {
            if ((ensFeatureGetSeqregionStart(vfeature)
                 >= (ensFeatureGetSeqregionStart(gfeature) - flank))
                &&
                (ensFeatureGetSeqregionStart(vfeature)
                 <= (ensFeatureGetSeqregionEnd(gfeature) + flank))
                &&
                (ensFeatureGetSeqregionEnd(vfeature)
                 <= (ensFeatureGetSeqregionEnd(gfeature) + flank)))
            {
                /*
                ** The Ensembl Genetic Variation Variation Feature is
                ** affecting the Gene, add to the AJAX Table if not present
                ** already
                */

                identifier = ensGeneGetIdentifier(gene);

                if (!ajTableMatchV(genetable, (const void *) &identifier))
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = ensGeneGetIdentifier(gene);

                    ajTablePut(genetable,
                               (void *) Pidentifier,
                               (void *) ensGeneNewRef(gene));
                }

                ensGeneDel(&gene);
            }
        }

        ensSliceDel(&slice);

        ensGvvariationfeatureDel(&gvvf);
    }

    ajListFree(&genelist);
    ajListFree(&gvvfs);

    /* Move all Ensembl Gene objects from the AJAX Table onto the AJAX List. */

    ensTableuintToList(genetable, genes);

    ajTableFree(&genetable);

    return ajTrue;
}




/* @func ensGvvariationFetchAllGvsources **************************************
**
** Fetch all Ensembl Genetic Variation Source objects of
** Ensembl Genetic Variation Synonym objects of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_synonym_sources
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Source objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGvsources(const EnsPGvvariation gvv,
                                       AjPList gvss)
{
    AjBool found = AJFALSE;

    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    EnsPGvsource gvs1 = NULL;
    EnsPGvsource gvs2 = NULL;

    if (!gvv)
        return ajFalse;

    if (!gvss)
        return ajFalse;

    iter1 = ajListIterNew(gvv->Gvsynonyms);
    iter2 = ajListIterNew(gvss);

    while (!ajListIterDone(iter1))
    {
        gvs1 = (EnsPGvsource) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        found = ajFalse;

        while (!ajListIterDone(iter2))
        {
            gvs2 = (EnsPGvsource) ajListIterGet(iter2);

            if (ensGvsourceMatch(gvs1, gvs2))
            {
                found = ajTrue;

                break;
            }
        }

        if (!found)
            ajListPushAppend(gvss, (void *) ensGvsourceNewRef(gvs1));
    }

    ajListIterDel(&iter1);
    ajListIterDel(&iter2);

    return ajTrue;
}




/* @func ensGvvariationFetchAllGvsynonyms *************************************
**
** Fetch all Ensembl Genetic Variation Synonym objects of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_synonyms
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] gvsourcename [const AjPStr] Ensembl Genetic Variation Source name
** @param [u] gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Synonym objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGvsynonyms(const EnsPGvvariation gvv,
                                        const AjPStr gvsourcename,
                                        AjPList gvss)
{
    AjIList iter = NULL;

    EnsPGvsource gvsource = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    if (!gvv)
        return ajFalse;

    if (!gvss)
        return ajFalse;

    iter = ajListIterNew(gvv->Gvsynonyms);

    while (!ajListIterDone(iter))
    {
        gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

        if (gvsourcename && ajStrGetLen(gvsourcename))
        {
            gvsource = ensGvsynonymGetGvsource(gvsynonym);

            if (ajStrMatchS(ensGvsourceGetName(gvsource), gvsourcename))
                ajListPushAppend(gvss, (void *) ensGvsynonymNewRef(gvsynonym));
        }
        else
            ajListPushAppend(gvss, (void *) ensGvsynonymNewRef(gvsynonym));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensGvvariationFetchAllGvvariationfeatures ****************************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_VariationFeatures
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvvfs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGvvariationfeatures(EnsPGvvariation gvv,
                                                 AjPList gvvfs)
{
    if (!gvv)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    if (!gvv->Adaptor)
    {
        ajDebug("ensGvvariationFetchAllGvvariationfeatures got an "
                "Ensembl Genetic Variation Variation without an "
                "Ensembl Genetic Variation Variation Adaptor attached.\n");

        return ajTrue;
    }

    return ensGvvariationfeatureadaptorFetchAllbyGvvariation(
        ensRegistryGetGvvariationfeatureadaptor(
            ensGvvariationadaptorGetDatabaseadaptor(gvv->Adaptor)),
        gvv,
        gvvfs);
}




/* @section query *************************************************************
**
** Functions for querying the properties of an
** Ensembl Genetic Variation Variation.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Has
** Check whether an Ensembl Genetic Variation Variation has a certain property
** @nam4rule Failedgvalleles
** Check for failed Ensembl Genetic Variation Allele objects
** @nam3rule Is
** Check whether an Ensembl Genetic Variation Variation represents a certain
** property
** @nam4rule Failed
** Check whether an Ensembl Genetic Variation Variation has been failed
**
** @argrule * gvv [EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule * Presult [AjBool*] Boolean result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationHasFailedgvalleles *************************************
**
** Check if an Ensembl Genetic Variation Variation has failed
** Ensembl Genetic Variation Allele objects.
**
** @cc Bio::EnsEMBL::Variation::Variation::has_failed_alleles
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] Presult [AjBool*]
** ajTrue, if at least one Ensembl Genetic Variation Allele has been failed
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationHasFailedgvalleles(EnsPGvvariation gvv, AjBool *Presult)
{
    AjBool result = AJFALSE;

    AjIList iter = NULL;

    EnsPGvallele gva = NULL;

    if (!gvv)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    *Presult = ajFalse;

    if (!gvv->Gvalleles)
        return ajTrue;

    iter = ajListIterNewread(gvv->Gvalleles);

    while (!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        ensGvalleleIsFailed(gva, &result);

        if (result == ajTrue)
        {
            *Presult = ajTrue;
            break;
        }
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensGvvariationIsFailed ***********************************************
**
** Check if an Ensembl Genetic Variation Variation has been failed.
** This function calls ensGvvariationLoadAllFaileddescriptions to find out if
** failed descriptiosn are available.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_failed
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] Presult [AjBool*]
** ajTrue, if an Ensembl Genetic Variation Variation has been failed
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationIsFailed(EnsPGvvariation gvv, AjBool *Presult)
{
    const AjPList descriptions = NULL;

    if (!gvv)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    descriptions = ensGvvariationLoadAllFaileddescriptions(gvv);

    if (ajListGetLength(descriptions) > 0)
        *Presult = ajTrue;
    else
        *Presult = ajFalse;

    return ajTrue;
}




/* @datasection [EnsEGvvariationClass] Ensembl Genetic Variation
** Variation Class enumeration
**
** @nam2rule Gvvariation Functions for manipulating
** Ensembl Genetic Variation Variation objects
** @nam3rule GvvariationClass Functions for manipulating
** Ensembl Genetic Variation Variation Class enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Variation Class
** enumeration.
**
** @fdata [EnsEGvvariationClass]
**
** @nam4rule From Ensembl Genetic Variation Variation Class query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  class  [const AjPStr] Class string
**
** @valrule * [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration or
** ensEGvvariationClassNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationClassFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Variation
** Class enumeration.
**
** @param [r] class [const AjPStr] Class string
**
** @return [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration or
** ensEGvvariationClassNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvvariationClass ensGvvariationClassFromStr(
    const AjPStr class)
{
    register EnsEGvvariationClass i =
        ensEGvvariationClassNULL;

    EnsEGvvariationClass gvvc =
        ensEGvvariationClassNULL;

    for (i = ensEGvvariationClassNULL;
         gvvariationKClass[i];
         i++)
        if (ajStrMatchC(class, gvvariationKClass[i]))
            gvvc = i;

    if (!gvvc)
        ajDebug("ensGvvariationClassFromStr encountered "
                "unexpected string '%S'.\n", class);

    return gvvc;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Variation Class enumeration.
**
** @fdata [EnsEGvvariationClass]
**
** @nam4rule To   Return Ensembl Genetic Variation Variation Class
**                       enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
**
** @valrule Char [const char*]
** Ensembl Genetic Variation Variation Class C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvvariationClassToChar ********************************************
**
** Convert an Ensembl Genetic Variation Variation Class enumeration
** into a C-type (char *) string.
**
** @param [u] gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
**
** @return [const char*]
** Ensembl Genetic Variation Variation Class C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensGvvariationClassToChar(
    EnsEGvvariationClass gvvc)
{
    register EnsEGvvariationClass i =
        ensEGvvariationClassNULL;

    for (i = ensEGvvariationClassNULL;
         gvvariationKClass[i] && (i < gvvc);
         i++);

    if (!gvvariationKClass[i])
        ajDebug("ensGvvariationClassToChar "
                "encountered an out of boundary error on "
                "Ensembl Genetic Variation Variation Class "
                "enumeration %d.\n",
                gvvc);

    return gvvariationKClass[i];
}




/* @datasection [EnsEGvvariationValidation] Ensembl Genetic Variation
** Variation Validation enumeration
**
** @nam2rule Gvvariation Functions for manipulating
** Ensembl Genetic Variation Variation objects
** @nam3rule GvvariationValidation Functions for manipulating
** Ensembl Genetic Variation Variation Validation enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Variation Validation
** enumeration.
**
** @fdata [EnsEGvvariationValidation]
**
** @nam4rule From Ensembl Genetic Variation Variation Validation query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  validation  [const AjPStr] Validation string
**
** @valrule * [EnsEGvvariationValidation]
** Ensembl Genetic Variation Variation Validation enumeration or
** ensEGvvariationValidationNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationValidationFromStr **************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Variation
** Validation enumeration.
**
** @param [r] validation [const AjPStr] Validation string
**
** @return [EnsEGvvariationValidation]
** Ensembl Genetic Variation Variation Validation enumeration or
** ensEGvvariationValidationNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvvariationValidation ensGvvariationValidationFromStr(
    const AjPStr validation)
{
    register EnsEGvvariationValidation i =
        ensEGvvariationValidationNULL;

    EnsEGvvariationValidation gvvv =
        ensEGvvariationValidationNULL;

    for (i = ensEGvvariationValidationNULL;
         gvvariationKValidation[i];
         i++)
        if (ajStrMatchC(validation, gvvariationKValidation[i]))
            gvvv = i;

    if (!gvvv)
        ajDebug("ensGvvariationValidationFromStr encountered "
                "unexpected string '%S'.\n", validation);

    return gvvv;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Variation Validation enumeration.
**
** @fdata [EnsEGvvariationValidation]
**
** @nam4rule To   Return Ensembl Genetic Variation Variation Validation
**                       enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvvv [EnsEGvvariationValidation]
** Ensembl Genetic Variation Variation Validation enumeration
**
** @valrule Char [const char*]
** Ensembl Genetic Variation Variation Validation C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvvariationValidationToChar ***************************************
**
** Convert an Ensembl Genetic Variation Variation Validation enumeration
** into a C-type (char *) string.
**
** @param [u] gvvv [EnsEGvvariationValidation]
** Ensembl Genetic Variation Variation Validation enumeration
**
** @return [const char*]
** Ensembl Genetic Variation Variation Validation C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensGvvariationValidationToChar(
    EnsEGvvariationValidation gvvv)
{
    register EnsEGvvariationValidation i =
        ensEGvvariationValidationNULL;

    for (i = ensEGvvariationValidationNULL;
         gvvariationKValidation[i] && (i < gvvv);
         i++);

    if (!gvvariationKValidation[i])
        ajDebug("ensGvvariationValidationToChar "
                "encountered an out of boundary error on "
                "Ensembl Genetic Variation Variation Validation "
                "enumeration %d.\n",
                gvvv);

    return gvvariationKValidation[i];
}




/* @datasection [none] C-types ************************************************
**
** @nam2rule Gvvariation Functions for manipulating
** Ensembl Genetic Variation Variation objects
** @nam3rule GvvariationValidations Functions for manipulating
** Ensembl Genetic Variation Variation Validation (AJAX unsigned integer)
** bit fields
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Variation Validation
** bit field.
**
** @fdata [none]
**
** @nam4rule From Ensembl Genetic Variation Variation Validation
** bit field query
** @nam5rule Set SQL set (comma-separated strings) query
**
** @argrule  Set gvvvset [const AjPStr] SQL set (comma-separated strings)
**
** @valrule * [ajuint]
** Ensembl Genetic Variation Variation Validation bit field or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationValidationsFromSet *************************************
**
** Convert an AJAX String representing a comma-separared SQL set of
** validation states into an Ensembl Genetic Variation Variation Validation
** (AJAX unsigned integer) bit field.
**
** @cc Bio::EnsEMBL::Variation::Utils::Sequence::array_to_bitval
** @param [r] gvvvset [const AjPStr] SQL set
**
** @return [ajuint]
** Ensembl Genetic Variation Variation Validation bit field or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvvariationValidationsFromSet(const AjPStr gvvvset)
{
    ajuint gvvvbf = 0U;

    AjPStr gvvvstring = NULL;

    AjPStrTok token = NULL;

    if (!gvvvset)
        return 0;

    /* Split the comma-separated list of validation states. */

    token = ajStrTokenNewC(gvvvset, ",");

    gvvvstring = ajStrNew();

    while (ajStrTokenNextParse(token, &gvvvstring))
        gvvvbf |= (1 << ensGvvariationValidationFromStr(gvvvstring));

    ajStrDel(&gvvvstring);

    ajStrTokenDel(&token);

    return gvvvbf;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Variation Validation bit field.
**
** @fdata [none]
**
** @nam4rule To Cast an Ensembl Genetic Variation Variation Validation
** bit field
** @nam5rule Set Cast into an SQL set (comma-separated strings)
**
** @argrule To gvvvbf [ajuint]
** Ensembl Genetic Variation Variation Validation bit field
** @argrule Set Pgvvvset [AjPStr*] SQL set
**
** @valrule * [AjBool] True on success, false on failure
**
** @fcategory cast
******************************************************************************/




/* @func ensGvvariationValidationsToSet ***************************************
**
** Cast an Ensembl Genetic Variation Variation Validation bit field
** (AJAX unsigned integer) into an SQL set (comma-separared strings).
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Variation::Utils::Sequence::bitval_to_array
** @param [r] gvvvbf [ajuint]
** Ensembl Genetic Variation Variation Validation bit field
** @param [w] Pgvvvset [AjPStr*] SQL set
**
** @return [AjBool] True on success, false on failure
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationValidationsToSet(ajuint gvvvbf, AjPStr *Pgvvvset)
{
    register ajuint i = 0U;

    if (*Pgvvvset)
        ajStrAssignClear(Pgvvvset);
    else
        *Pgvvvset = ajStrNew();

    for (i = 1U; gvvariationKValidation[i]; i++)
        if (gvvvbf & (1U << i))
        {
            ajStrAppendC(Pgvvvset, gvvariationKValidation[i]);
            ajStrAppendC(Pgvvvset, ",");
        }

    /* Remove the last comma if one exists. */

    if (ajStrGetLen(*Pgvvvset) > 0U)
        ajStrCutEnd(Pgvvvset, 1);

    return ajTrue;
}




/* @funcstatic listGvvariationCompareIdentifierAscending **********************
**
** AJAX List comparison function to sort Ensembl Genetic Variation Variation
** objects by identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Genetic Variation Variation 1
** @param [r] item2 [const void*] Ensembl Genetic Variation Variation 2
** @see ajListSort
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPGvvariation gvv1 = *(EnsOGvvariation *const *) item1;
    EnsPGvvariation gvv2 = *(EnsOGvvariation *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvvariationCompareIdentifierAscending"))
        ajDebug("listGvvariationCompareIdentifierAscending\n"
                "  gvv1 %p\n"
                "  gvv2 %p\n",
                gvv1,
                gvv2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    if (gvv1->Identifier < gvv2->Identifier)
        result = -1;

    if (gvv1->Identifier > gvv2->Identifier)
        result = +1;

    return result;
}




/* @funcstatic listGvvariationDelete ******************************************
**
** ajListSortUnique "itemdel" function to delete Ensembl Genetic Variation
** Variation objects that are redundant.
**
** @param [r] Pitem [void**] Ensembl Genetic Variation Variation address
** @param [r] cl [void*] Standard, passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void listGvvariationDelete(void **Pitem, void *cl)
{
    if (!Pitem)
        return;

    (void) cl;

    ensGvvariationDel((EnsPGvvariation *) Pitem);

    return;
}




/*
** TODO: The following methods are not implemented:
**   failed_description
**   get_VariationFeature_by_dbID
**   get_all_IndividualGenotypes
**   get_all_PopulationGenotypes
**   add_PopulationGenotype
**   ambig_code
**   derived_allele_frequency
**   derived_allele
*/




/* @datasection [EnsPGvvariationadaptor] Ensembl Genetic Variation Variation
** Adaptor
**
** @nam2rule Gvvariationadaptor Functions for manipulating
** Ensembl Genetic Variation Variation Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor
** @cc CVS Revision: 1.90
** @cc CVS Tag: branch-ensembl-68
**
** NOTE: Since the "source" SQL table contains only a small number of records
** linked to a potentially large number of records in other SQL tables, the
** contents have been modelled as independent objects. The EnsPGvsourceadaptor
** caches all EnsPGvsource entries, which should help reduce memory
** requirements. Consequently, neither the "variation" nor "variation_synonym"
** SQL tables need joining to the "source" table.
******************************************************************************/




/* @funcstatic gvvariationadaptorFetchAllbyConstraint *************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an SQL constraint.
**
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] constraint [const AjPStr] SQL constraint
** @param [r] joinallele [AjBool] Join the "allele" SQL table
** @param [r] joinflank  [AjBool] Join the "flanking_sequence" SQL table
** @param [r] joinsource [AjBool] Join the "source" SQL table
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool gvvariationadaptorFetchAllbyConstraint(
    EnsPGvvariationadaptor gvva,
    const AjPStr constraint,
    AjBool joinallele,
    AjBool joinflank,
    AjBool joinsource,
    AjPList gvvs)
{
    register ajuint i = 0U;
    register ajuint j = 0U;
    register ajuint k = 0U;

    AjPStr defaultcondition = NULL;
    AjPStr fvsconstraint    = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    /*
    ** The SQL column name, SQL table name and Leftjoin arrays have been
    ** allocated to their maximum size.
    ** Set the iterators to their last common array element.
    */

    for (i = 0U; gvvariationadaptorKColumnnames[i]; i++);

    for (j = 0U; gvvariationadaptorKTablenames[j]; j++);

    for (k = 0U; gvvariationadaptorKLeftjoins[k].Tablename; k++);

    ajCharDel(&gvva->Defaultcondition);

    defaultcondition = ajStrNewC(gvvariationadaptorKDefaultcondition);

    if (joinallele == ajTrue)
    {
        gvva->Tablenames[j] = ajCharNewC("allele");
        j++;

        if (ajStrGetLen(defaultcondition))
            ajStrAppendC(&defaultcondition,
                         " AND "
                         "allele.variation_id = "
                         "variation.variation_id");
        else
            ajStrAppendC(&defaultcondition,
                         "allele.variation_id = "
                         "variation.variation_id");
    }

    if (!ensGvbaseadaptorGetFailedvariations(
            ensGvvariationadaptorGetGvbaseadaptor(gvva)))
    {
        gvva->Tablenames[j] = ajCharNewC("failed_variation");
        j++;

        gvva->Leftjoins[k].Tablename =
            ajCharNewC("failed_variation");
        gvva->Leftjoins[k].Condition =
            ajCharNewC("variation.variation_id = "
                       "failed_variation.variation_id");
        k++;
    }

    if (joinflank == ajTrue)
    {
        gvva->Columnnames[i] = ajCharNewC(
            "("
            "flanking_sequence.up_seq IS NOT NULL "
            "OR "
            "flanking_sequence.down_seq IS NOT NULL"
            ")");
        i++;

        gvva->Tablenames[j] = ajCharNewC("flanking_sequence");
        j++;

        gvva->Leftjoins[k].Tablename =
            ajCharNewC("flanking_sequence");
        gvva->Leftjoins[k].Condition =
            ajCharNewC("variation.variation_id = "
                       "flanking_sequence.variation_id");
        k++;
    }
    else
    {
        gvva->Columnnames[i] = ajCharNewC("0");
        i++;
    }

    if (joinsource)
    {
        gvva->Tablenames[i] = ajCharNewC("source");
        i++;

        if (ajStrGetLen(defaultcondition))
            ajStrAppendC(&defaultcondition,
                         " AND variation.source_id = source.source_id");
        else
            ajStrAppendC(&defaultcondition,
                         "variation.source_id = source.source_id");
    }

    gvva->Columnnames[i]         = NULL;
    gvva->Tablenames[j]          = NULL;
    gvva->Leftjoins[k].Tablename = NULL;
    gvva->Leftjoins[k].Condition = NULL;

    /*
    ** FIXME: Not sure, but this may have to add the failed_variation
    ** condition here.
    */

    ensGvdatabaseadaptorFailedvariationsconstraint(
        ensGvvariationadaptorGetGvdatabaseadaptor(gvva),
        (const AjPStr) NULL,
        &fvsconstraint);

    ajStrAppendC(&defaultcondition, " AND ");
    ajStrAppendS(&defaultcondition, fvsconstraint);

    ajStrDel(&fvsconstraint);

    gvva->Defaultcondition = ajCharNewC(ajStrGetPtr(defaultcondition));

    ajStrDel(&defaultcondition);

    return ensBaseadaptorFetchAllbyConstraint(
        ensGvvariationadaptorGetBaseadaptor(gvva),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvs);
}




/* @funcstatic gvvariationadaptorFetchAllbyStatement **************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvvariationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvs)
{
    float gvafrequency   = 0.0F;
    float gvvmafrequency = 0.0F;

    ajuint gvvidentifier     = 0U;
    ajuint gvvidentifierold  = 0U;
    ajuint gvasubidentifier  = 0U;
    ajuint gvvgvaclassid     = 0U;
    ajuint gvvgvaclinicalid  = 0U;
    ajuint gvvsourceid       = 0U;
    ajuint gvaidentifier     = 0U;
    ajuint gvaidentifierold  = 0U;
    ajuint gvacount          = 0U;
    ajuint gvagvsampleid     = 0U;
    ajuint gvvsidentifier    = 0U;
    ajuint gvvssubidentifier = 0U;
    ajuint gvvssourceid      = 0U;
    ajuint gvvmacount        = 0U;

    ajuint *Pidentifier = NULL;

    AjBool gvvflank   = AJFALSE;
    AjBool gvvflipped = AJFALSE;
    AjBool gvvsomatic = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr gvvname          = NULL;
    AjPStr gvvvalidation    = NULL;
    AjPStr gvvancestral     = NULL;
    AjPStr gvaallele        = NULL;
    AjPStr gvvsmoleculetype = NULL;
    AjPStr gvvsname         = NULL;
    AjPStr gvvmaallele      = NULL;

    AjPTable gvpopulations = NULL;
    AjPTable gvsynonyms    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvallele        gvallele  = NULL;
    EnsPGvalleleadaptor gvallelea = NULL;

    EnsPGvattribute        gvaclass     = NULL;
    EnsPGvattribute        gvaclinical  = NULL;
    EnsPGvattributeadaptor gvattributea = NULL;

    EnsPGvpopulation        gvp  = NULL;
    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvvariation        gvv  = NULL;
    EnsPGvvariationadaptor gvva = NULL;

    EnsPGvsource        gvsource  = NULL;
    EnsPGvsourceadaptor gvsourcea = NULL;

    EnsPGvsynonym        gvsynonym  = NULL;
    EnsPGvsynonymadaptor gvsynonyma = NULL;

    if (ajDebugTest("gvvariationadaptorFetchAllbyStatement"))
        ajDebug("gvvariationadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvvs %p\n",
                ba,
                statement,
                am,
                slice,
                gvvs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    /*
    ** Initialise AJAX Table objects with AJAX unsigned integer key and
    ** Ensembl Genetic Variation Population and Ensembl Genetic Variation
    ** Synonym value data.
    */
    gvpopulations = ajTableuintNew(0U);
    gvsynonyms    = ajTableuintNew(0U);

    ajTableSetDestroyvalue(gvpopulations,
                           (void (*)(void **)) &ensGvpopulationDel);

    ajTableSetDestroyvalue(gvsynonyms,
                           (void (*)(void **)) &ensGvsynonymDel);

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvallelea    = ensRegistryGetGvalleleadaptor(dba);
    gvattributea = ensRegistryGetGvattributeadaptor(dba);
    gvpa         = ensRegistryGetGvpopulationadaptor(dba);
    gvsourcea    = ensRegistryGetGvsourceadaptor(dba);
    gvsynonyma   = ensRegistryGetGvsynonymadaptor(dba);
    gvva         = ensRegistryGetGvvariationadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        /* "variation" SQL table */
        gvvidentifier     = 0U;
        gvvsourceid       = 0U;
        gvvname           = ajStrNew();
        gvvvalidation     = ajStrNew();
        gvvancestral      = ajStrNew();
        gvvgvaclassid     = 0U;
        gvvsomatic        = ajFalse;
        gvvflipped        = ajFalse;
        gvvmaallele       = ajStrNew();
        gvvmacount        = 0U;
        gvvmafrequency    = 0.0F;
        gvvgvaclinicalid  = 0U;
        /* "allele" SQL table */
        gvaidentifier     = 0U;
        gvasubidentifier  = 0U;
        gvaallele         = ajStrNew();
        gvafrequency      = 0.0F;
        gvacount          = 0U;
        gvagvsampleid     = 0U;
        /* "variation_synonym" SQL table */
        gvvsidentifier    = 0U;
        gvvssubidentifier = 0U;
        gvvsourceid       = 0U;
        gvvsname          = ajStrNew();
        gvvsmoleculetype  = ajStrNew();
        /* */
        gvvflank          = ajFalse;

        sqlr = ajSqlrowiterGet(sqli);

        /* "variation" SQL table */
        ajSqlcolumnToUint(sqlr, &gvvidentifier);
        ajSqlcolumnToUint(sqlr, &gvvsourceid);
        ajSqlcolumnToStr(sqlr, &gvvname);
        ajSqlcolumnToStr(sqlr, &gvvvalidation);
        ajSqlcolumnToStr(sqlr, &gvvancestral);
        ajSqlcolumnToUint(sqlr, &gvvgvaclassid);
        ajSqlcolumnToBool(sqlr, &gvvsomatic);
        ajSqlcolumnToBool(sqlr, &gvvflipped);
        ajSqlcolumnToStr(sqlr, &gvvmaallele);
        ajSqlcolumnToUint(sqlr, &gvvmacount);
        ajSqlcolumnToFloat(sqlr, &gvvmafrequency);
        ajSqlcolumnToUint(sqlr, &gvvgvaclinicalid);
        /* "allele" SQL table */
        ajSqlcolumnToUint(sqlr, &gvaidentifier);
        ajSqlcolumnToUint(sqlr, &gvasubidentifier);
        ajSqlcolumnToStr(sqlr, &gvaallele);
        ajSqlcolumnToFloat(sqlr, &gvafrequency);
        ajSqlcolumnToUint(sqlr, &gvacount);
        ajSqlcolumnToUint(sqlr, &gvagvsampleid);
        /* "variation_synonym" SQL table */
        ajSqlcolumnToUint(sqlr, &gvvsidentifier);
        ajSqlcolumnToUint(sqlr, &gvvssubidentifier);
        ajSqlcolumnToUint(sqlr, &gvvssourceid);
        ajSqlcolumnToStr(sqlr, &gvvsname);
        ajSqlcolumnToStr(sqlr, &gvvsmoleculetype);
        /*
        ** (flanking_sequence.up_seq IS NOT NULL
        ** OR
        ** flanking_sequence.down_seq IS NOT NULL)
        */
        ajSqlcolumnToBool(sqlr, &gvvflank);

        if (gvvidentifier != gvvidentifierold)
        {
            ensGvattributeadaptorFetchByIdentifier(gvattributea,
                                                   gvvgvaclassid,
                                                   &gvaclass);

            ensGvattributeadaptorFetchByIdentifier(gvattributea,
                                                   gvvgvaclinicalid,
                                                   &gvaclinical);

            /*
            ** NOTE: Because information from the "source" SQL table is
            ** modelled as independent EnsPGvsource object, neither
            ** "variation" nor "variation_synonym" need joining.
            */

            ensGvsourceadaptorFetchByIdentifier(gvsourcea,
                                                gvvsourceid,
                                                &gvsource);

            gvv = ensGvvariationNewIni(gvva,
                                       gvvidentifier,
                                       gvaclass,
                                       gvaclinical,
                                       gvsource,
                                       gvvname,
                                       gvvancestral,
                                       (AjPList) NULL, /* synonyms */
                                       (AjPList) NULL, /* alleles */
                                       gvvvalidation,
                                       gvvsmoleculetype,
                                       (AjPStr) NULL, /* flankfive */
                                       (AjPStr) NULL, /* flankthree */
                                       gvvflank,
                                       gvvsomatic,
                                       gvvflipped,
                                       gvvmaallele,
                                       gvvmacount,
                                       gvvmafrequency);

            ajListPushAppend(gvvs, (void *) gvv);

            ensGvattributeDel(&gvaclass);

            ensGvsourceDel(&gvsource);

            gvvidentifierold = gvvidentifier;
        }

        if (gvaidentifier != gvaidentifierold)
        {
            /* Fetch the Ensembl Genetic Variation Population. */

            if (gvagvsampleid)
            {
                gvp = (EnsPGvpopulation) ajTableFetchmodV(
                    gvpopulations,
                    (const void *) &gvagvsampleid);

                if (!gvp)
                {
                    ensGvpopulationadaptorFetchByIdentifier(gvpa,
                                                            gvagvsampleid,
                                                            &gvp);

                    if (gvp)
                    {
                        AJNEW0(Pidentifier);

                        *Pidentifier = ensGvpopulationGetIdentifier(gvp);

                        ajTablePut(gvpopulations,
                                   (void *) Pidentifier,
                                   (void *) gvp);
                    }
                }
            }
            else
                gvp = NULL;

            if (gvaidentifier)
            {
                gvallele = ensGvalleleNewIni(gvallelea,
                                             gvaidentifier,
                                             gvp,
                                             gvaallele,
                                             gvafrequency,
                                             gvasubidentifier,
                                             gvacount,
                                             gvvidentifier);

                ensGvvariationAddGvallele(gvv, gvallele);

                ensGvalleleDel(&gvallele);

                gvaidentifierold = gvaidentifier;
            }
            else
                gvallele = NULL;
        }

        if (gvvsidentifier)
        {
            gvsynonym = (EnsPGvsynonym) ajTableFetchmodV(
                gvsynonyms,
                (const void *) &gvvsidentifier);

            if (!gvsynonym)
            {
                ensGvsourceadaptorFetchByIdentifier(gvsourcea,
                                                    gvvssourceid,
                                                    &gvsource);

                gvsynonym = ensGvsynonymNewIni(gvsynonyma,
                                               gvvsidentifier,
                                               gvsource,
                                               gvvsname,
                                               gvvsmoleculetype,
                                               gvvidentifier,
                                               gvvssubidentifier);

                ensGvsourceDel(&gvsource);

                if (gvsynonym)
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = gvvsidentifier;

                    ajTablePut(gvsynonyms,
                               (void *) Pidentifier,
                               (void *) gvsynonym);
                }
            }

            ensGvvariationAddGvsynonym(gvv, gvsynonym);
        }

        ajStrDel(&gvvname);
        ajStrDel(&gvvvalidation);
        ajStrDel(&gvvancestral);
        ajStrDel(&gvaallele);
        ajStrDel(&gvvsmoleculetype);
        ajStrDel(&gvvsname);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajTableDel(&gvpopulations);
    ajTableDel(&gvsynonyms);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariationadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationadaptorNew *********************************************
**
** Default constructor for an
** Ensembl Genetic Variation Variation Adaptor.
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
** @see ensRegistryGetGvvariationadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvvariationadaptor ensGvvariationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    register ajuint i = 0U;

    EnsPGvbaseadaptor gvba = NULL;

    EnsPGvvariationadaptor gvva = NULL;

    if (!dba)
        return NULL;

    gvba = ensGvbaseadaptorNew(
        ensRegistryGetGvdatabaseadaptor(dba),
        gvvariationadaptorKTablenames,
        gvvariationadaptorKColumnnames,
        gvvariationadaptorKLeftjoins,
        gvvariationadaptorKDefaultcondition,
        (const char *) NULL,
        gvvariationadaptorFetchAllbyStatement);

    if (!gvba)
        return NULL;

    AJNEW0(gvva);

    gvva->Adaptor = gvba;

    /*
    ** Allocate SQL column name, SQL table name and Leftjoin arrays to their
    ** maximum size and populate them with unghangeable values.
    */

    /*
    ** The SQL column name array needs additional space for the
    ** flanking sequence flag SQL column.
    */

    for (i = 0U; gvvariationadaptorKColumnnames[i]; i++);

    gvvariationadaptorGColumnnamesElements = i + 1U + 1U;

    gvva->Columnnames = AJCALLOC0(gvvariationadaptorGColumnnamesElements,
                                  sizeof (char *));

    for (i = 0U; gvvariationadaptorKColumnnames[i]; i++)
        gvva->Columnnames[i] = ajCharNewC(gvvariationadaptorKColumnnames[i]);

    /*
    ** The SQL table name array needs additional space for
    ** "allele", "failed_variation", "flanking_sequence",
    ** and "source" SQL table names.
    */

    for (i = 0U; gvvariationadaptorKTablenames[i]; i++);

    gvvariationadaptorGTablenamesElements = i + 1U + 4U;

    gvva->Tablenames = AJCALLOC0(gvvariationadaptorGTablenamesElements,
                                 sizeof (char *));

    for (i = 0U; gvvariationadaptorKTablenames[i]; i++)
        gvva->Tablenames[i] = ajCharNewC(gvvariationadaptorKTablenames[i]);

    /*
    ** The Leftjoins array needs additional space for
    ** "failed_variation" and "flanking_sequence"
    ** SQL LEFT JOIN conditions.
    */

    for (i = 0U; gvvariationadaptorKLeftjoins[i].Tablename; i++);

    gvvariationadaptorGLeftjoinsElements = i + 1U + 2U;

    gvva->Leftjoins = AJCALLOC0(gvvariationadaptorGLeftjoinsElements,
                                sizeof (EnsOBaseadaptorLeftjoin));

    for (i = 0U; gvvariationadaptorKLeftjoins[i].Tablename; i++)
    {
        gvva->Leftjoins[i].Tablename =
            ajCharNewC(gvvariationadaptorKLeftjoins[i].Tablename);

        gvva->Leftjoins[i].Condition =
            ajCharNewC(gvvariationadaptorKLeftjoins[i].Condition);
    }

    return gvva;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Variation Adaptor object.
**
** @fdata [EnsPGvvariationadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Adaptor
**
** @argrule * Pgvva [EnsPGvvariationadaptor*]
** Ensembl Genetic Variation Variation Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationadaptorDel *********************************************
**
** Default destructor for an
** Ensembl Genetic Variation Variation Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvva [EnsPGvvariationadaptor*]
** Ensembl Genetic Variation Variation Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvvariationadaptorDel(EnsPGvvariationadaptor *Pgvva)
{
    register ajuint i = 0U;

    EnsPGvvariationadaptor pthis = NULL;

    if (!Pgvva)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvvariationadaptorDel"))
        ajDebug("ensGvvariationadaptorDel\n"
                "  *Pgvva %p\n",
                *Pgvva);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvva))
        return;

    ensGvbaseadaptorDel(&pthis->Adaptor);

    for (i = 0U; i < gvvariationadaptorGTablenamesElements; i++)
        ajCharDel(&pthis->Tablenames[i]);

    AJFREE(pthis->Tablenames);

    for (i = 0U; i < gvvariationadaptorGColumnnamesElements; i++)
        ajCharDel(&pthis->Columnnames[i]);

    AJFREE(pthis->Columnnames);

    for (i = 0U; i < gvvariationadaptorGLeftjoinsElements; i++)
    {
        ajCharDel(&(pthis->Leftjoins[i].Tablename));
        ajCharDel(&(pthis->Leftjoins[i].Condition));
    }

    AJFREE(pthis->Leftjoins);

    ajCharDel(&pthis->Defaultcondition);

    ajMemFree((void **) Pgvva);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Variation Adaptor object.
**
** @fdata [EnsPGvvariationadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Variation Adaptor
** attribute(s)
** @nam4rule Baseadaptor
** Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor
** Return the Ensembl Database Adaptor
** @nam4rule Gvbaseadaptor
** Return the Ensembl Genetic Variation Base Adaptor
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
**
** @argrule * gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Gvbaseadaptor [EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor or NULL
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationadaptorGetBaseadaptor **********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Variation Adaptor.
**
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvvariationadaptorGetBaseadaptor(
    EnsPGvvariationadaptor gvva)
{
    return ensGvbaseadaptorGetBaseadaptor(
        ensGvvariationadaptorGetGvbaseadaptor(gvva));
}




/* @func ensGvvariationadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Variation Adaptor.
**
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvvariationadaptorGetDatabaseadaptor(
    EnsPGvvariationadaptor gvva)
{
    return ensGvbaseadaptorGetDatabaseadaptor(
        ensGvvariationadaptorGetGvbaseadaptor(gvva));
}




/* @func ensGvvariationadaptorGetGvbaseadaptor ********************************
**
** Get the Ensembl Genetic Variation Base Adaptor member of an
** Ensembl Genetic Variation Variation Adaptor.
**
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @return [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvbaseadaptor ensGvvariationadaptorGetGvbaseadaptor(
    EnsPGvvariationadaptor gvva)
{
    return (gvva) ? gvva->Adaptor : NULL;
}




/* @func ensGvvariationadaptorGetGvdatabaseadaptor ****************************
**
** Get the Ensembl Genetic Variation Database Adaptor member of an
** Ensembl Genetic Variation Variation Adaptor.
**
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvvariationadaptorGetGvdatabaseadaptor(
    EnsPGvvariationadaptor gvva)
{
    return ensGvbaseadaptorGetGvdatabaseadaptor(
        ensGvvariationadaptorGetGvbaseadaptor(gvva));
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Variation objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGvvariationadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation object(s)
** @nam4rule All   Fetch all Ensembl Genetic Variation Variation objects
** @nam4rule Allby
** Fetch all Ensembl Genetic Variation Variation objects matching a criterion
** @nam5rule Gvpopulation Fetch by an Ensembl Genetic Variation Population
** @nam5rule Gvpopulationfrequency Fetch by an Ensembl Genetic Variation
** Population and frequency
** @nam5rule Gvsource Fetch all by an Ensembl Genetic Variation Source
** @nam6rule GvsourceType
** Fetch all by an Ensembl Genetic Variation Source Type enumeration
** @nam5rule Gvvariationset Fetch by an Ensembl Genetic Variation Variation Set
** @nam5rule Identifiers    Fetch by an AJAX Table of AJAX unsigned integer
** (identifier) key data
** @nam5rule Names          Fetch by names
** @nam5rule Source         Fetch by a source
** @nam4rule Allsomatic
** Fetch all somatic Ensembl Genetic Variation Variation objects
** @nam4rule By             Fetch one Ensembl Genetic Variation Variation
**                          object matching a criterion
** @nam5rule Identifier     Fetch by an SQL database-internal identifier
** @nam5rule Name           Fetch by a name
** @nam5rule Subidentifier  Fetch by a Sub-SNP identifier
** @nam5rule Synonym        Fetch by a synonym
**
** @argrule * gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @argrule All gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule Allsomatic gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule AllbyGvpopulation gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllbyGvpopulation gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule AllbyGvpopulationfrequency gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @argrule AllbyGvpopulationfrequency frequency [float]
** Lower minor allele frequency (MAF) threshold
** @argrule AllbyGvpopulationfrequency gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule AllbyGvsourceType gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @argrule AllbyGvsourceType primary [AjBool] Primary
** @argrule AllbyGvsourceType gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule AllbyGvvariationset gvvset [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @argrule AllbyGvvariationset gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule AllbyIdentifiers gvvs [AjPTable] AJAX Table of
** AJAX unsigned integer identifier key data and
** Ensembl Genetic Variation Variation value data
** @argrule AllbyNames gvvs [AjPTable] AJAX Table of
** AJAX String (name) key data and
** AJAX Genetic Variation Variation value data
** @argrule AllbySource source [const AjPStr] Source
** @argrule AllbySource primary [AjBool] Primary
** @argrule AllbySource gvvs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByName name [const AjPStr]
** Ensembl Genetic Variation Variation name
** @argrule ByName source [const AjPStr] Source
** @argrule BySubidentifier name [const AjPStr] Name
** @argrule BySynonym synonym [const AjPStr] Synonym
** @argrule BySynonym source [const AjPStr] Source
** @argrule By Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationadaptorFetchAll ****************************************
**
** Fetch all Ensembl Genetic Variation Variation objects via
** an Ensembl Genetic Variation Variation Adaptor.
**
** Please note that it is probably not a good idea to use this function on
** very large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_all
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAll(EnsPGvvariationadaptor gvva,
                                     AjPList gvvs)
{
    AjBool joinallele = AJFALSE;
    AjBool joinflank  = AJFALSE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    constraint = ajStrNewC("variation.somatic = 0");

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchAllbyGvpopulation **************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an
** Ensembl Genetic Variation Population.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
**     fetch_all_by_Population
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] gvp [const EnsPGvpopulation]
** Ensembl Genetic Variation Population
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyGvpopulation(
    EnsPGvvariationadaptor gvva,
    const EnsPGvpopulation gvp,
    AjPList gvvs)
{
    /* This function needs to join the to the allele SQL table. */

    AjBool joinallele = AJTRUE;
    AjBool joinflank  = AJFALSE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    constraint = ajFmtStr(
        "allele.sample_id = %u",
        ensGvpopulationGetIdentifier(gvp));

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchAllbyGvpopulationfrequency *****************
**
** Fetch all Ensembl Genetic Variation Variation objects via an
** Ensembl Genetic Variation Population and frequency information.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
**     fetch_all_by_Population
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] frequency [float] Lower minor allele frequency (MAF) threshold
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyGvpopulationfrequency(
    EnsPGvvariationadaptor gvva,
    const EnsPGvpopulation gvp,
    float frequency,
    AjPList gvvs)
{
    /* This function needs to join the to the allele SQL table. */

    AjBool joinallele = AJTRUE;
    AjBool joinflank  = AJFALSE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    /* Adjust the frequency if given a percentage. */

    if (frequency > 1.0F)
        frequency /= 100.0F;

    constraint = ajFmtStr(
        "allele.sample_id = %u "
        "AND "
        "("
        "IF("
        "allele.frequency > 0.5, "
        "1 - allele.frequency, "
        "allele.frequency"
        ") > %f"
        ")",
        ensGvpopulationGetIdentifier(gvp),
        frequency);

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchAllbyGvsourceType **************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an
** Ensembl Genetic Variation Source Type enumeration.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
** fetch_all_by_source_type
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [u] gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @param [r] primary [AjBool] Primary
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyGvsourceType(
    EnsPGvvariationadaptor gvva,
    EnsEGvsourceType gvst,
    AjBool primary,
    AjPList gvvs)
{
    AjPList gvss = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsource        gvs  = NULL;
    EnsPGvsourceadaptor gvsa = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvst)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    dba = ensGvbaseadaptorGetDatabaseadaptor(gvva->Adaptor);

    gvsa = ensRegistryGetGvsourceadaptor(dba);

    gvss = ajListNew();

    ensGvsourceadaptorFetchAllbyGvsourceType(gvsa, gvst, gvss);

    while (ajListPop(gvss, (void *) &gvs))
    {
        ensGvvariationadaptorFetchAllbySource(
            gvva,
            ensGvsourceGetName(gvs),
            primary,
            gvvs);

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

    /*
    ** Remove duplicate Ensembl Genetic Variation Variation objects
    ** from the AJAX List.
    */

    ajListSortUnique(gvvs,
                     &listGvvariationCompareIdentifierAscending,
                     &listGvvariationDelete);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchAllbyGvvariationset ************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an
** Ensembl Genetic Variation Variation Set.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
**     fetch_all_by_VariationSet
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [u] gvvset [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: Since the Bio::EnsEMBL::Utils::Iterator class is not implemented yet,
** this function has not been changed since Ensembl version 60.
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyGvvariationset(
    EnsPGvvariationadaptor gvva,
    EnsPGvvariationset gvvset,
    AjPList gvvs)
{
    AjPList variationsets = NULL;

    AjPStr statement = NULL;

    EnsPGvvariationset variationset = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    if (!gvvset->Adaptor)
        return ajFalse;

    if (!gvvset->Identifier)
        return ajFalse;

    variationsets = ajListNew();

    /*
    ** First, get all immediate subsets of the specified Ensembl Genetic
    ** Variation Variation Set and get their Ensembl Genetic Variation
    ** Variation objects.
    */

    ensGvvariationsetFetchAllSub(gvvset, ajFalse, variationsets);

    while (ajListPop(variationsets, (void **) &variationset))
    {
        /*
        ** FIXME: This recursive approach may not work because fetching
        ** sub-sets of sub-sets can be problematic.
        */
        ensGvvariationadaptorFetchAllbyGvvariationset(gvva,
                                                      variationset,
                                                      gvvs);

        ensGvvariationsetDel(&variationset);
    }

    ajListFree(&variationsets);

    /*
    ** Then get all Ensembl Genetic Variation Variation objects belonging to
    ** this Ensembl Genetic Variation Variation Set.
    */

    /* FIXME: The SQL statement needs to be changed to an SQL constraint. */
    statement = ajFmtStr(
        "SELECT "
        "variation.variation_id, "
        "variation.source_id, "
        "variation.name, "
        "variation.validation_status, "
        "variation.ancestral_allele, "
        "variation.class_attrib_id, "
        "variation.somatic, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.count, "
        "allele.sample_id, "
        "variation_synonym.variation_synonym_id, "
        "variation_synonym.subsnp_id, "
        "variation_synonym.source_id, "
        "variation_synonym.name, "
        "variation_synonym.moltype, "
        "0 " /* "flanking_sequence" flag */
        "FROM "
        "(variation, allele, variation_set_variation) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = allele.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "WHERE "
        "variation.variation_id = variation_set_variation.variation_id ",
        "AND "
        "variation_set_variation.variation_set_id = %u",
        ensGvvariationsetGetIdentifier(gvvset));

    gvvariationadaptorFetchAllbyStatement(
        ensGvvariationadaptorGetBaseadaptor(gvva),
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvs);

    ajStrDel(&statement);

    /*
    ** Remove duplicate Ensembl Genetic Variation Variation objects
    ** from the AJAX List.
    */

    ajListSortUnique(gvvs,
                     &listGvvariationCompareIdentifierAscending,
                     &listGvvariationDelete);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchAllbyIdentifiers ***************************
**
** Fetch all Ensembl Genetic Variation Variation objects by an AJAX Table of
** AJAX unsigned integer key data and assign them as value data.
**
** The caller is responsible for deleting the
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Variation value data before deleting the
** AJAX Table.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_all_by_dbID_list
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [u] gvvs [AjPTable]
** AJAX Table of AJAX unsigned integer (identifier)
** key data and Ensembl Genetic Variation
** Variation value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyIdentifiers(
    EnsPGvvariationadaptor gvva,
    AjPTable gvvs)
{
    void **keyarray = NULL;

    /* This function needs to join the to the flanking_sequence SQL table. */

    AjBool joinallele = AJFALSE;
    AjBool joinflank  = AJFALSE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    const char *template = "variation.variation_id IN (%S)";

    register ajuint i = 0U;

    AjPList lgvvs = NULL;

    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    lgvvs = ajListNew();

    csv = ajStrNew();

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifier instances configured in the
    ** Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    ajTableToarrayKeys(gvvs, &keyarray);

    for (i = 0U; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint *) keyarray[i]));

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more array elements to process.
        */

        if ((((i + 1U) % ensKBaseadaptorMaximumIdentifiers) == 0)
            || (keyarray[i + 1U] == NULL))
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                constraint = ajFmtStr(template, csv);

                result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                                constraint,
                                                                joinallele,
                                                                joinflank,
                                                                joinsource,
                                                                lgvvs);

                ajStrDel(&constraint);
            }

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    ajStrDel(&csv);

    /*
    ** Move Ensembl Genetic Variation Variation objects from the AJAX List
    ** to the AJAX Table.
    */

    ensTableuintFromList(
        gvvs,
        (ajuint (*)(const void *)) &ensGvvariationGetIdentifier,
        lgvvs);

    ajListFree(&lgvvs);

    return result;
}




/*
** Internal method for getting the internal dbIDs for a list of names.
** Will also query the variation_synonym and allele (for subsnp_ids) tables.
*/

static AjBool gvvariationadaptorFetchAllbyNames(
    EnsPGvvariationadaptor gvva,
    AjBool synonym,
    AjBool subsnp,
    AjPTable gvvnamestogvv,
    AjPTable *Pgvvidentifiers)
{
    void **keyarray = NULL;

    /* Statement to get the dbIDs from variation or variation_synonym. */
    const char *template = "SELECT %s FROM %s v WHERE %s IN (%S)";

    register ajuint i = 0U;

    ajuint *Pgvvidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr csv       = NULL;
    AjPStr gvvname   = NULL;
    AjPStr statement = NULL;

    AjPTable table = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvnamestogvv)
        return ajFalse;

    if (!Pgvvidentifiers)
        return ajFalse;

    if (!*Pgvvidentifiers)
    {
        /*
        ** If not initialised already create an AJAX Table of
        ** AJAX String key and AJAX unsigned integer value data.
        */

        *Pgvvidentifiers = ajTablestrNew(ajTableGetSize(gvvnamestogvv));

        ajTableSetDestroyvalue(*Pgvvidentifiers, &ajMemFree);
    }

    dba = ensGvvariationadaptorGetDatabaseadaptor(gvva);

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifier instances configured in the
    ** Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    csv = ajStrNew();

    ajTableToarrayKeys(gvvnamestogvv, &keyarray);

    for (i = 0U; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "'%S', ", (AjPStr) keyarray[i]);

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more array elements to process.
        */

        if ((((i + 1U) % ensKBaseadaptorMaximumIdentifiers) == 0)
            || (keyarray[i + 1U] == NULL))
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                statement = ajFmtStr(
                    template,
                    /* SQL column name array */
                    (synonym && subsnp) ?
                    "CONCAT('ss',variation.subsnp_id) AS name, v.variation_id" :
                    "v.name, v.variation_id",
                    /* SQL table name array */
                    synonym ?
                    (subsnp ? "allele" : "variation_synonym") :
                    "variation",
                    /* SQL SELECT default condition */
                    (synonym && subsnp ? "v.subsnp_id" : "v.name"),
                    /* Name list */
                    csv);

                sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

                sqli = ajSqlrowiterNew(sqls);

                while (!ajSqlrowiterDone(sqli))
                {
                    gvvname = ajStrNew();
                    AJNEW0(Pgvvidentifier);
                    *Pgvvidentifier = 0U;

                    sqlr = ajSqlrowiterGet(sqli);

                    ajSqlcolumnToStr(sqlr, &gvvname);
                    ajSqlcolumnToUint(sqlr, Pgvvidentifier);

                    ajTablePut(*Pgvvidentifiers,
                               (void *) gvvname,
                               Pgvvidentifier);
                }

                ajSqlrowiterDel(&sqli);

                ensDatabaseadaptorSqlstatementDel(dba, &sqls);

                ajStrDel(&statement);
            }

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    ajStrDel(&csv);

    /*
    ** For queries against the "variation" SQL table and unmapped names,
    ** also query the "variation_synonym" and "allele" SQL tables.
    */

    if (!synonym || !subsnp)
    {
        /*
        ** Find the unmapped names and put them into a new AJAX Table of
        ** AJAX String key and
        ** Ensembl Genetic Variation Variation value data.
        */

        table = ajTablestrNew(ajTableGetSize(gvvnamestogvv));

        ajTableSetDestroyvalue(table, (void (*)(void **)) &ensGvvariationDel);

        ajTableToarrayKeys(gvvnamestogvv, &keyarray);

        for (i = 0U; keyarray[i]; i++)
        {
            if (!ajTableMatchS(table, (const AjPStr) keyarray[i]))
                ajTablePut(table,
                           (void *) ajStrNewS((const AjPStr) keyarray[i]),
                           (void *) NULL);
        }

        AJFREE(keyarray);

        /*
        ** TODO: If we are going to query for subsnp_ids, get the names that
        ** look like ssIds and strip the ss prefix.
        */
        /*
          if ($synonym) {
          (at)unmapped = grep {$_ =~ s/^ss(\d+)$/$1/} (at)unmapped;
          }
        */

        /* FIXME: Why is synonym passed in in third position?
        ** my $names = $self->_name_to_dbID(\(at)unmapped,1,$synonym);
        */
        gvvariationadaptorFetchAllbyNames(gvva,
                                          ajTrue,
                                          synonym,
                                          table,
                                          Pgvvidentifiers);

        ajTableDel(&table);
    }

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchAllbyNames *********************************
**
** Fetch all Ensembl Genetic Variation Variation objects by an
** AJAX List of AJAX Sring (name) objects.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_all_by_name_list
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [u] gvvs [AjPTable] AJAX Table of
** AJAX String (name) key data and
** Ensembl Genetic Variation Variation value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyNames(
    EnsPGvvariationadaptor gvva,
    AjPTable gvvs)
{
    void **keyarray = NULL;

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    */

    const char *template1 =
        "SELECT "
        "variation.variation_id, "
        "variation.source_id, "
        "variation.name, "
        "variation.validation_status, "
        "variation.ancestral_allele, "
        "variation.class_attrib_id, "
        "variation.somatic, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.count, "
        "allele.sample_id, "
        "variation_synonym.variation_synonym_id, "
        "variation_synonym.subsnp_id, "
        "variation_synonym.source_id, "
        "variation_synonym.name, "
        "variation_synonym.moltype, "
        "0 " /* "flanking_sequence" flag */
        "FROM (variation) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = allele.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = failed_variation.variation_id "
        "WHERE "
        "variation.name IN (%S)";

    /*
    ** NOTE: Values are extracted from variation_synonym1,
    ** while variation_synonym2 is included in the condition.
    */

    const char *template2 =
        "SELECT "
        "variation.variation_id, "
        "variation.source_id, "
        "variation.name, "
        "variation.validation_status, "
        "variation.ancestral_allele, "
        "variation.class_attrib_id, "
        "variation.somatic, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.count, "
        "allele.sample_id, "
        "variation_synonym1.variation_synonym_id, "
        "variation_synonym1.subsnp_id, "
        "variation_synonym1.source_id, "
        "variation_synonym1.name, "
        "variation_synonym1.moltype, "
        "0 " /* "flanking_sequence" flag */
        "FROM "
        "variation, "
        "allele, "
        "variation_synonym variation_synonym1, "
        "variation_synonym variation_synonym2, "
        /* FIXME: Why is "flanking_sequence" SQL table joined in? It doesn't seem to be used. */
        "flanking_sequence "
        "WHERE "
        "variation.variation_id = allele.variation_id "
        "AND "
        "variation.variation_id = variation_synonym1.variation_id "
        "AND "
        "variation.variation_id = variation_synonym2.variation_id "
        "AND "
        "variation.variation_id = flanking_sequence.variation_id "
        "AND "
        "variation_synonym2.name IN (%S) "
        "ORDER BY "
        "allele.allele_id";

    register ajuint i = 0U;
    register ajuint j = 0U;

    AjPList lgvvs = NULL;

    AjPStr statement = NULL;
    AjPStr csv       = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvvariation gvv = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    /* FIXME: This is just here to use the function temporarily. */
    gvvariationadaptorFetchAllbyNames(gvva,
                                      ajFalse,
                                      ajFalse,
                                      (AjPTable) NULL,
                                      (AjPTable *) NULL);

    ba = ensGvvariationadaptorGetBaseadaptor(gvva);

    lgvvs = ajListNew();

    csv = ajStrNew();

    /*
    ** MySQL is faster and we ensure that we do not exceed the maximum
    ** query size by splitting large queries into smaller queries of
    ** up to 200 names.
    */

    ajTableToarrayKeys(gvvs, &keyarray);

    for (i = 0U; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "'%S', ", (AjPStr) keyarray[i]);

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more array elements to process.
        ** Assume that average name strings are 4 times longer than
        ** integer strings.
        */

        if ((((i + 1U) % (ensKBaseadaptorMaximumIdentifiers / 4U)) == 0)
            || (keyarray[i + 1U] == NULL))
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                statement = ajFmtStr(template1, csv);

                gvvariationadaptorFetchAllbyStatement(
                    ba,
                    statement,
                    (EnsPAssemblymapper) NULL,
                    (EnsPSlice) NULL,
                    lgvvs);

                ajStrDel(&statement);
            }

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /*
    ** Move Ensembl Genetic Variation Variation objects from the AJAX List
    ** to the AJAX Table.
    */

    ensTablestrFromList(
        gvvs,
        (AjPStr (*)(const void *)) &ensGvvariationGetName,
        lgvvs);

    /* Check, which names yielded no result above and search for synonyms. */

    ajTableToarrayKeys(gvvs, &keyarray);

    for (i = 0U; keyarray[i]; i++)
    {
        gvv = (EnsPGvvariation) ajTableFetchmodV(gvvs,
                                                 (const void *) keyarray[i]);

        if (gvv == NULL)
        {
            ajFmtPrintAppS(&csv, "'%S', ", (AjPStr) keyarray[i]);

            j++;
        }

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more array elements to process.
        ** Assume that average name strings are 4 times longer than
        ** integer strings.
        */

        if ((((j + 1U) % (ensKBaseadaptorMaximumIdentifiers / 4U)) == 0)
            || (keyarray[i + 1U] == NULL))
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                statement = ajFmtStr(template2, csv);

                gvvariationadaptorFetchAllbyStatement(
                    ba,
                    statement,
                    (EnsPAssemblymapper) NULL,
                    (EnsPSlice) NULL,
                    lgvvs);

                ajStrDel(&statement);
            }

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /*
    ** Move Ensembl Genetic Variation Variation objects from the AJAX List
    ** to the AJAX Table based on a synonym.
    */

    ensTablestrFromList(
        gvvs,
        (AjPStr (*)(const void *)) &ensGvvariationGetName,
        lgvvs);

    ajListFree(&lgvvs);

    ajStrDel(&csv);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchAllbySource ********************************
**
** Fetch all Ensembl Genetic Variation Variation objects via a source.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_all_by_source
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] source [const AjPStr] Source
** @param [r] primary [AjBool] Primary
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbySource(
    EnsPGvvariationadaptor gvva,
    const AjPStr source,
    AjBool primary,
    AjPList gvvs)
{
    char *txtsource = NULL;

    AjBool joinallele = AJFALSE;
    AjBool joinflank  = AJFALSE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!gvva)
        return ajFalse;

    if (!source)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    ba = ensGvvariationadaptorGetBaseadaptor(gvva);

    ensBaseadaptorEscapeC(ba, &txtsource, source);

    constraint = ajFmtStr("source1.name = '%s'", txtsource);

    if (primary == ajFalse)
        ajFmtPrintAppS(&constraint, " OR source2.name = '%s'", txtsource);

    ajCharDel(&txtsource);

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchAllsomatic *********************************
**
** Fetch all somatic Ensembl Genetic Variation Objects via
** an Ensembl Genetic Variation Variation Adaptor.
**
** Please note that it is probably not a good idea to use this function on
** very large tables quite common in the Ensembl genome annotation system.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_all_somatic
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [u] gvvs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllsomatic(EnsPGvvariationadaptor gvva,
                                            AjPList gvvs)
{
    AjBool joinallele = AJFALSE;
    AjBool joinflank  = AJFALSE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvva)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    constraint = ajStrNewC("variation.somatic = 1");

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Genetic Variation Variation via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_dbID
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchByIdentifier(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    EnsPGvvariation *Pgvv)
{
    /* This function needs to join the to the flanking_sequence SQL table. */

    AjBool joinallele = AJFALSE;
    AjBool joinflank  = AJTRUE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPList gvvs = NULL;

    AjPStr constraint = NULL;

    EnsPGvvariation gvv = NULL;

    if (!gvva)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pgvv)
        return ajFalse;

    *Pgvv = NULL;

    constraint = ajFmtStr("variation.variation_id = %u", identifier);

    gvvs = ajListNew();

    /* Join the flanking_sequence SQL table. */

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    if (ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier got more than one "
               "Ensembl Genetic Variation Variation for identifier %u.",
               identifier);

    ajListPop(gvvs, (void **) Pgvv);

    while (ajListPop(gvvs, (void **) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchByName *************************************
**
** Fetch an Ensembl Genetic Variation Variation via its name.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_name
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] source [const AjPStr] Source
** @param [wP] Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchByName(
    EnsPGvvariationadaptor gvva,
    const AjPStr name,
    const AjPStr source,
    EnsPGvvariation *Pgvv)
{
    char *txtname   = NULL;
    char *txtsource = NULL;

    /* This function needs to join the to the flanking_sequence SQL table. */

    AjBool joinallele = AJFALSE;
    AjBool joinflank  = AJTRUE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPList gvvs = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvvariation gvv = NULL;

    if (!gvva)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!Pgvv)
        return ajFalse;

    *Pgvv = NULL;

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    ** NOTE: The "flanking_sequence" SQL table is joined here and in
    ** ensGvvariationadaptorFetchByIdentifier so that the flank_flag can be
    ** set where the flanking sequence differs from the reference. It is just
    ** set to 0 when fetching Ensembl Geentic Variation Variation objects by
    ** other methods since otherwise the join takes too long.
    */

    ba = ensGvvariationadaptorGetBaseadaptor(gvva);

    ensBaseadaptorEscapeC(ba, &txtname, name);

    constraint = ajFmtStr("source.name = '%s'", txtname);

    ajCharDel(&txtname);

    if (source && ajStrGetLen(source))
    {
        joinsource = ajTrue;

        ensBaseadaptorEscapeC(ba, &txtsource, source);

        ajFmtPrintAppS(&constraint, " AND source.name = '%s'", txtsource);

        ajCharDel(&txtsource);
    }
    else
        joinsource = ajFalse;

    gvvs = ajListNew();

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    ajStrDel(&constraint);

    if (ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchByName got more than one "
               "Ensembl Genetic Variation Variation for "
               "name '%S' and source '%S'.", name, source);

    ajListPop(gvvs, (void **) Pgvv);

    while (ajListPop(gvvs, (void **) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&gvvs);

    /*
    ** Check the variation_synonym SQL table if the name was not found in the
    ** variation SQL table.
    */

    if (*Pgvv == NULL)
        result = ensGvvariationadaptorFetchBySynonym(gvva,
                                                     name,
                                                     source,
                                                     Pgvv);

    /*
    ** If the query yielded no result and the name looks like a
    ** Sub-SNP identifier, query again.
    */

    if ((*Pgvv == NULL) && (ajStrPrefixC(name, "ss") == ajTrue))
        result = ensGvvariationadaptorFetchBySubidentifier(gvva, name, Pgvv);

    return result;
}




/* @func ensGvvariationadaptorFetchBySubidentifier ****************************
**
** Fetch an Ensembl Genetic Variation Variation via its sub-identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_subsnp_id
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] name [const AjPStr] Name
** @param [wP] Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchBySubidentifier(
    EnsPGvvariationadaptor gvva,
    const AjPStr name,
    EnsPGvvariation *Pgvv)
{
    char *txtname = NULL;

    /*
    ** This function needs to join the to the flanking_sequence and
    ** allele SQL tables.
    */

    AjBool joinallele = AJTRUE;
    AjBool joinflank  = AJTRUE;
    AjBool joinsource = AJFALSE;
    AjBool result     = AJFALSE;

    AjPList gvvs = NULL;

    AjPStr constraint    = NULL;
    AjPStr subidentifier = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariation gvv = NULL;

    if (!gvva)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!Pgvv)
        return ajFalse;

    *Pgvv = NULL;

    dba = ensGvvariationadaptorGetDatabaseadaptor(gvva);

    if (ajStrPrefixC(name, "ss"))
    {
        subidentifier = ajStrNew();
        ajStrAssignSubS(&subidentifier, name, 0, 2);
        ensDatabaseadaptorEscapeC(dba, &txtname, subidentifier);
        ajStrDel(&subidentifier);
    }
    else
        ensDatabaseadaptorEscapeC(dba, &txtname, name);

    constraint = ajFmtStr("allele.subsnp_id = '%s'", txtname);

    ajCharDel(&txtname);

    gvvs = ajListNew();

    result = gvvariationadaptorFetchAllbyConstraint(gvva,
                                                    constraint,
                                                    joinallele,
                                                    joinflank,
                                                    joinsource,
                                                    gvvs);

    if (ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchBySubidentifier got more than one "
               "Ensembl Genetic Variation Variation for "
               "sub-identifier '%S'.", name);

    ajListPop(gvvs, (void **) Pgvv);

    while (ajListPop(gvvs, (void **) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&gvvs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationadaptorFetchBySynonym **********************************
**
** Fetch an Ensembl Genetic Variation Variation via a synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_synonym
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] synonym [const AjPStr] Synonym
** @param [rN] source [const AjPStr] Source
** @param [wP] Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchBySynonym(
    EnsPGvvariationadaptor gvva,
    const AjPStr synonym,
    const AjPStr source,
    EnsPGvvariation *Pgvv)
{
    char *txtname = NULL;

    ajuint gvvidentifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr constraint = NULL;
    AjPStr statement  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvva)
        return ajFalse;

    if (!synonym)
        return ajFalse;

    if (!Pgvv)
        return ajFalse;

    *Pgvv = NULL;

    dba = ensGvvariationadaptorGetDatabaseadaptor(gvva);

    ensDatabaseadaptorEscapeC(dba, &txtname, synonym);

    constraint = ajFmtStr("variation_synonym.name = '%s'", txtname);

    ajCharDel(&txtname);

    if (source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtname, source);

        ajFmtPrintAppS(&constraint, " AND source.name = '%s'", txtname);

        ajCharDel(&txtname);
    }

    statement = ajFmtStr(
        "SELECT "
        "variation_synonym.variation_id "
        "FROM "
        "variation_synonym "
        "JOIN "
        "source "
        "ON "
        "("
        "source.source_id = variation_synonym.source_id"
        ")"
        "WHERE "
        "%S"
        "LIMIT 1 ",
        constraint);

    /*
    ** This statement will only return one row which is consistent with the
    ** behaviour of ensGvvariationadaptorFetchByName.
    ** However, the synonym name is only guaranteed to be unique in
    ** combination with the source
    */

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gvvidentifier = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gvvidentifier);

        ensGvvariationadaptorFetchByIdentifier(gvva, gvvidentifier, Pgvv);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&constraint);
    ajStrDel(&statement);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Genetic Variation Variation-releated
** objects from an Ensembl SQL database.
**
** @fdata [EnsPGvvariationadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Genetic Variation Variation-related
** object(s)
** @nam4rule All Retrieve all related objects
** @nam5rule Faileddescriptions Retrieve all failed descriptions for
** Ensembl Genetic Variation Variation objects
** @nam4rule Flank Retrieve flanking sequences
**
** @argrule * gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @argrule Faileddescriptions identifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @argrule Faileddescriptions fvs [AjPList]
** AJAX List of AJAX String (description) objects
** @argrule Flank identifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @argrule Flank Pfiveseq [AjPStr*] Five-prime flanking sequence
** @argrule Flank Pthreeseq [AjPStr*] Three-prime flanking sequence
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic gvvariationadaptorRetrieveAllFaileddescriptions ****************
**
** Retrieve all failed descriptions of an
** Ensembl Genetic Variation Variation objects.
**
** The caller is responsible for deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
** _internal_get_failed_descriptions
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] identifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @param [r] constraint [const AjPStr] SQL constraint
** @param [u] fvs [AjPTable] AJAX Table of
** AJAX unsigned integer (Ensembl Genetic Variation Variation identifier)
** key data and
** AJAX Table value data.
** Second-level AJAX Table of
** AJAX unsigned integer (sub-SNP identifier) key data and
** AJAX String (description) value data.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvvariationadaptorRetrieveAllFaileddescriptions(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    const AjPStr constraint,
    AjPTable fvs)
{
    ajuint gvvidentifier = 0U;
    ajuint subsnpid      = 0U;

    ajuint *Pgvvidentifier = NULL;
    ajuint *Psubsnpid      = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr description = NULL;
    AjPStr statement   = NULL;

    AjPTable table = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvva)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!fvs)
        return ajFalse;

    dba = ensGvvariationadaptorGetDatabaseadaptor(gvva);

    /*
    ** NOTE: The Perl API uses the following construct ...
    ** IFNULL(failed_variation.subsnp_id, 'rs') AS subsnp_id
    ** ... which is not compatible with the unsigned integer type.
    */

    statement = ajFmtStr(
        "SELECT "
        "failed_variation.variation_id, "
        "failed_variation.subsnp_id, "
        "failed_description.description "
        "FROM "
        "failed_variation "
        "JOIN "
        "failed_description "
        "ON "
        "(failed_variation.failed_description_id = "
        "failed_description.failed_description_id) "
        "WHERE "
        "failed_variation.variation_id = %u",
        identifier);

    if ((constraint != NULL) && (ajStrGetLen(constraint) > 0))
    {
        ajStrAppendC(&statement, " AND ");
        ajStrAppendS(&statement, constraint);
    }

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gvvidentifier = 0;
        subsnpid      = 0;
        description   = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gvvidentifier);
        ajSqlcolumnToUint(sqlr, &subsnpid);
        ajSqlcolumnToStr(sqlr, &description);

        table = (AjPTable) ajTableFetchmodV(fvs, (const void *) &gvvidentifier);

        if (!table)
        {
            AJNEW0(Pgvvidentifier);

            *Pgvvidentifier = gvvidentifier;

            table = ajTableuintNew(0U);

            ajTableSetDestroyvalue(table, (void (*)(void **)) &ajStrDel);

            ajTablePut(fvs, (void *) Pgvvidentifier, (void *) table);
        }

        if (!ajTableMatchV(table, (const void *) &subsnpid))
        {
            AJNEW0(Psubsnpid);

            *Psubsnpid = subsnpid;

            ajTablePut(table, (void *) Psubsnpid, (void *) description);
        }
        else
            ajStrDel(&description);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvvariationadaptorRetrieveAllFaileddescriptions *******************
**
** Retrieve all failed descriptions for an Ensembl Genetic Variation Variation.
**
** The caller is responsible for deleting the AJAX String objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
** get_all_failed_descriptions
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] identifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @param [u] fvs [AjPList] AJAX List of AJAX String (description) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorRetrieveAllFaileddescriptions(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    AjPList fvs)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    AjPTable fvstable1 = NULL;
    AjPTable fvstable2 = NULL;
    AjPTable strtable  = NULL;

    if (!gvva)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!fvs)
        return ajFalse;

    fvstable1 = ajTableuintNew(0U);

    ajTableSetDestroyvalue(fvstable1, (void (*)(void **)) &ajTableDel);

    gvvariationadaptorRetrieveAllFaileddescriptions(
        gvva,
        identifier,
        (AjPStr) NULL,
        fvstable1);

    fvstable2 = (AjPTable) ajTableFetchmodV(fvstable1,
                                            (const void *) &identifier);

    if (!fvstable2)
        return ajFalse;

    strtable = ajTablestrNew(0U);

    ajTableSetDestroyvalue(strtable, &ajMemFree);

    ajTableToarrayValues(fvstable2, &valarray);

    for (i = 0U; valarray[i]; i++)
        if (!ajTableMatchV(strtable, (const void *) valarray[i]))
            ajTablePut(strtable,
                       (void *) ajStrNewS((AjPStr) valarray[i]),
                       NULL);

    AJFREE(valarray);

    ajTableToarrayKeys(strtable, &keyarray);

    for (i = 0U; keyarray[i]; i++)
        ajListPushAppend(fvs, (void *) keyarray[i]);

    ajTableDel(&fvstable1);
    ajTableDel(&strtable);

    return ajTrue;
}




/*
** TODO: The following methods are not implemented:
** is_failed
** has_failed_subsnps
** get_failed_description
*/




/* @funcstatic gvvariationadaptorRetrieveFlankFromCore ************************
**
** Retrieve the flanking sequence from the corresponding Ensembl Core Database.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::_get_flank_from_core
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] srid [ajuint] Ensembl Sequence Region identifier
** @param [r] srstart [ajint] Ensembl Sequence Region start
** @param [r] srend [ajint] Ensembl Sequence Region end
** @param [r] srstrand [ajint] Ensembl Sequence Region strand
** @param [wP] Psequence [AjPStr*] Flanking sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvvariationadaptorRetrieveFlankFromCore(
    EnsPGvvariationadaptor gvva,
    ajuint srid,
    ajint srstart,
    ajint srend,
    ajint srstrand,
    AjPStr *Psequence)
{
    EnsPDatabaseadaptor dba = NULL;
    EnsPDatabaseadaptor rsa = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!gvva)
        return ajFalse;

    if (!srid)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    dba = ensGvvariationadaptorGetDatabaseadaptor(gvva);

    rsa = ensRegistryGetReferenceadaptor(dba);

    sla = ensRegistryGetSliceadaptor(rsa);

    if (sla)
    {
        ensSliceadaptorFetchBySeqregionIdentifier(sla, srid, 0, 0, 0, &slice);

        if (!slice)
            ajWarn("gvvariationadaptorRetrieveFlankFromCore could not get an "
                   "Ensembl Slice for Ensembl Sequence region identifier %u.",
                   srid);

        ensSliceFetchSequenceSubStr(slice,
                                    srstart,
                                    srend,
                                    srstrand,
                                    Psequence);
    }

    return ajTrue;
}




/* @func ensGvvariationadaptorRetrieveFlank ***********************************
**
** Retrieve the flanking sequence of an Ensembl Genetic Variation Variation.
**
** If the flanking sequence is not in the flankinq_sequence table of the
** Ensembl Genetic Variation database the sequence is retrieved from the
** corresponding Ensembl Core database.
**
** The caller is responsible for deleting the AJAX String objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_flanking_sequence
** @param [u] gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @param [r] identifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @param [wP] Pfiveseq [AjPStr*] Five-prime flanking sequence
** @param [wP] Pthreeseq [AjPStr*] Three-prime flanking sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationadaptorRetrieveFlank(EnsPGvvariationadaptor gvva,
                                          ajuint identifier,
                                          AjPStr *Pfiveseq,
                                          AjPStr *Pthreeseq)
{
    ajint srstrand = 0;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajuint usrstart = 0U;
    ajuint usrend   = 0U;
    ajuint dsrstart = 0U;
    ajuint dsrend   = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr useq      = NULL;
    AjPStr dseq      = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvva)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pfiveseq)
        return ajFalse;

    if (!Pthreeseq)
        return ajFalse;

    dba = ensGvvariationadaptorGetDatabaseadaptor(gvva);

    statement = ajFmtStr(
        "SELECT "
        "flanking_sequence.seq_region_id, "
        "flanking_sequence.seq_region_strand, "
        "flanking_sequence.up_seq, "
        "flanking_sequence.down_seq, "
        "flanking_sequence.up_seq_region_start, "
        "flanking_sequence.up_seq_region_end, "
        "flanking_sequence.down_seq_region_start, "
        "flanking_sequence.down_seq_region_end "
        "FROM "
        "flanking_sequence "
        "WHERE "
        "flanking_sequence.variation_id = %u",
        identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        srid     = 0U;
        srstrand = 0;
        useq     = ajStrNew();
        dseq     = ajStrNew();
        usrstart = 0U;
        usrend   = 0U;
        dsrstart = 0U;
        dsrend   = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToStr(sqlr, &useq);
        ajSqlcolumnToStr(sqlr, &dseq);
        ajSqlcolumnToUint(sqlr, &usrstart);
        ajSqlcolumnToUint(sqlr, &usrend);
        ajSqlcolumnToUint(sqlr, &dsrstart);
        ajSqlcolumnToUint(sqlr, &dsrend);

        if (!ajStrGetLen(useq))
        {
            if (srid)
            {
                if (usrend < usrstart)
                {
                    srstart = usrend;
                    srend   = usrstart;
                }
                else
                {
                    srstart = usrstart;
                    srend   = usrend;
                }

                gvvariationadaptorRetrieveFlankFromCore(gvva,
                                                        srid,
                                                        srstart,
                                                        srend,
                                                        srstrand,
                                                        &useq);
            }
            else
                ajWarn("ensGvvariationadaptorRetrieveFlank "
                       "could not get "
                       "Ensembl Sequence Region identifier for "
                       "Ensembl Genetic Variation Variation %u.",
                       identifier);
        }

        ajStrAssignS(Pfiveseq, useq);

        if (!ajStrGetLen(dseq))
        {
            if (srid)
            {
                if (dsrend < dsrstart)
                {
                    srstart = dsrend;
                    srend   = dsrstart;
                }
                else
                {
                    srstart = dsrstart;
                    srend   = dsrend;
                }

                gvvariationadaptorRetrieveFlankFromCore(gvva,
                                                        srid,
                                                        srstart,
                                                        srend,
                                                        srstrand,
                                                        &dseq);
            }
            else
                ajWarn("ensGvvariationadaptorRetrieveFlank "
                       "could not get "
                       "Ensembl Sequence Region identifier for "
                       "Ensembl Genetic Variation Variation %u.",
                       identifier);
        }

        ajStrAssignS(Pthreeseq, dseq);

        ajStrDel(&useq);
        ajStrDel(&dseq);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPGvvariationfeature] Ensembl Genetic Variation Variation
** Feature
**
** @nam2rule Gvvariationfeature Functions for manipulating
** Ensembl Genetic Variation Variation Feature objects
**
** @cc Bio::EnsEMBL::Variation::VariationFeature
** @cc CVS Revision: 1.90
** @cc CVS Tag: branch-ensembl-61
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation Feature by
** pointer. It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation Feature. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Identifier Constructor with an Ensembl Genetic Variation Variation
** identifier
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
** @argrule Identifier gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Identifier feature [EnsPFeature] Ensembl Feature
** @argrule Identifier gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Identifier gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
** @argrule Identifier gvvidentifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @argrule Identifier allele [AjPStr] Allele
** @argrule Identifier name [AjPStr] Name
** @argrule Identifier validation [AjPStr] Validation code
** @argrule Identifier consequence [AjPStr] Consequence type
** @argrule Identifier mapweight [ajuint] Map weight
** @argrule Ini gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Ini gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
** @argrule Ini gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @argrule Ini allele [AjPStr] Allele
** @argrule Ini name [AjPStr] Name
** @argrule Ini validation [AjPStr] Validation code
** @argrule Ini consequence [AjPStr] Consequence type
** @argrule Ini mapweight [ajuint] Map weight
** @argrule Ref gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
**
** @valrule * [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationfeatureNewCpy ******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
**
** @return [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewCpy(
    const EnsPGvvariationfeature gvvf)
{
    EnsPGvvariationfeature pthis = NULL;

    if (!gvvf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use         = 1U;
    pthis->Identifier  = gvvf->Identifier;
    pthis->Adaptor     = gvvf->Adaptor;
    pthis->Feature     = ensFeatureNewRef(gvvf->Feature);
    pthis->Gvsource    = ensGvsourceNewRef(gvvf->Gvsource);
    pthis->Gvvariation = ensGvvariationNewRef(gvvf->Gvvariation);

    if (gvvf->Allele)
        pthis->Allele = ajStrNewRef(gvvf->Allele);

    if (gvvf->Name)
        pthis->Name = ajStrNewRef(gvvf->Name);

    if (gvvf->Validationcode)
        pthis->Validationcode = ajStrNewRef(gvvf->Validationcode);

    if (gvvf->Consequencetype)
        pthis->Consequencetype = ajStrNewRef(gvvf->Consequencetype);

    pthis->Mapweight             = gvvf->Mapweight;
    pthis->Gvvariationidentifier = gvvf->Gvvariationidentifier;
    pthis->GvvariationClass      = gvvf->GvvariationClass;

    return pthis;
}




/* @func ensGvvariationfeatureNewIdentifier ***********************************
**
** Constructor for an Ensembl Genetic Variation Variation Feature.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::VariationFeature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** Ensembl Genetic Variation Variation identifier
** @param [u] gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
** @param [r] gvvidentifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @param [u] allele [AjPStr] Allele
** @param [u] name [AjPStr] Name
** @param [u] validation [AjPStr] Validation code
** @param [u] consequence [AjPStr] Consequence type
** @param [r] mapweight [ajuint] Map weight
**
** @return [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewIdentifier(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPGvsource gvsource,
    EnsEGvvariationClass gvvc,
    ajuint gvvidentifier,
    AjPStr allele,
    AjPStr name,
    AjPStr validation,
    AjPStr consequence,
    ajuint mapweight)
{
    EnsPGvvariationfeature gvvf = NULL;

    if (!feature)
        return NULL;

    if (!gvvidentifier)
        return NULL;

    AJNEW0(gvvf);

    gvvf->Use                   = 1U;
    gvvf->Identifier            = identifier;
    gvvf->Adaptor               = gvvfa;
    gvvf->Feature               = ensFeatureNewRef(feature);
    gvvf->Gvsource              = ensGvsourceNewRef(gvsource);
    gvvf->Gvvariationidentifier = gvvidentifier;

    if (allele)
        gvvf->Allele = ajStrNewRef(allele);

    if (name)
        gvvf->Name = ajStrNewRef(name);

    if (validation)
        gvvf->Validationcode = ajStrNewRef(validation);

    if (consequence)
        gvvf->Consequencetype = ajStrNewRef(consequence);

    gvvf->Mapweight = mapweight;

    gvvf->GvvariationClass = gvvc;

    return gvvf;
}




/* @func ensGvvariationfeatureNewIni ******************************************
**
** Constructor for an Ensembl Genetic Variation Variation Feature with initial
** values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::VariationFeature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] allele [AjPStr] Allele
** @param [u] name [AjPStr] Name
** @param [u] validation [AjPStr] Validation code
** @param [u] consequence [AjPStr] Consequence type
** @param [r] mapweight [ajuint] Map weight
**
** @return [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewIni(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPGvsource gvsource,
    EnsEGvvariationClass gvvc,
    EnsPGvvariation gvv,
    AjPStr allele,
    AjPStr name,
    AjPStr validation,
    AjPStr consequence,
    ajuint mapweight)
{
    EnsPGvvariationfeature gvvf = NULL;

    if (!feature)
        return NULL;

    if (!gvv)
        return NULL;

    AJNEW0(gvvf);

    gvvf->Use         = 1;
    gvvf->Identifier  = identifier;
    gvvf->Adaptor     = gvvfa;
    gvvf->Feature     = ensFeatureNewRef(feature);
    gvvf->Gvsource    = ensGvsourceNewRef(gvsource);
    gvvf->Gvvariation = ensGvvariationNewRef(gvv);

    if (allele)
        gvvf->Allele = ajStrNewRef(allele);

    if (name)
        gvvf->Name = ajStrNewRef(name);

    if (validation)
        gvvf->Validationcode = ajStrNewRef(validation);

    if (consequence)
        gvvf->Consequencetype = ajStrNewRef(consequence);

    gvvf->Mapweight        = mapweight;
    gvvf->GvvariationClass = gvvc;

    return gvvf;
}




/* @func ensGvvariationfeatureNewRef ******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
**
** @return [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewRef(
    EnsPGvvariationfeature gvvf)
{
    if (!gvvf)
        return NULL;

    gvvf->Use++;

    return gvvf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Feature
**
** @argrule * Pgvvf [EnsPGvvariationfeature*]
** Ensembl Genetic Variation Variation Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationfeatureDel *********************************************
**
** Default destructor for an Ensembl Genetic Variation Variation Feature.
**
** @param [d] Pgvvf [EnsPGvvariationfeature*]
** Ensembl Genetic Variation Variation Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvvariationfeatureDel(EnsPGvvariationfeature *Pgvvf)
{
    EnsPGvvariationfeature pthis = NULL;

    if (!Pgvvf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvvariationfeatureDel"))
    {
        ajDebug("ensGvvariationfeatureDel\n"
                "  *Pgvvf %p\n",
                *Pgvvf);

        ensGvvariationfeatureTrace(*Pgvvf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvvf) || --pthis->Use)
    {
        *Pgvvf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensGvsourceDel(&pthis->Gvsource);

    ensGvvariationDel(&pthis->Gvvariation);

    ajStrDel(&pthis->Allele);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Validationcode);
    ajStrDel(&pthis->Consequencetype);

    ajMemFree((void **) Pgvvf);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Get Return Genetic Variation Variation Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation
**                   Variation Feature Adaptor
** @nam4rule Allele Return the allele
** @nam4rule Consequencetype Return the consequence type
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Gvsource Return the Ensemb Genetic Variation Source
** @nam4rule Gvvariation Return the Ensembl Genetic Variation Variation
** @nam4rule Gvvariationclass
** Return the Ensembl Geentic Variation Variation Class enumeration
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Mapweight Return the map weight
** @nam4rule Name Return the name
** @nam4rule Validationcode Return the validation code
**
** @argrule * gvvf [const EnsPGvvariationfeature] Genetic Variation
**                                                Variation Feature
**
** @valrule Adaptor [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
** Variation Feature Adaptor or NULL
** @valrule Allele [AjPStr] Allele or NULL
** @valrule Consequencetype [AjPStr] Consequence type or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Gvsource [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @valrule Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** or NULL
** @valrule Gvvariationclass [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class or ensEGvvariationClassNULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Mapweight [ajuint] Map weight or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Validationcode [AjPStr] Validation code or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureGetAdaptor **************************************
**
** Get the Ensembl Genetic Variation Variation Feature Adaptor member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                         Variation Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvvariationfeatureadaptor ensGvvariationfeatureGetAdaptor(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Adaptor : NULL;
}




/* @func ensGvvariationfeatureGetAllele ***************************************
**
** Get the allele member of an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Allele or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetAllele(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Allele : NULL;
}




/* @func ensGvvariationfeatureGetConsequencetype ******************************
**
** Get the consequence type member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Consequence type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetConsequencetype(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Consequencetype : NULL;
}




/* @func ensGvvariationfeatureGetFeature **************************************
**
** Get the Ensembl Feature member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensGvvariationfeatureGetFeature(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Feature : NULL;
}




/* @func ensGvvariationfeatureGetGvsource *************************************
**
** Get the Ensembl Genetic Variation Source member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsource ensGvvariationfeatureGetGvsource(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Gvsource : NULL;
}




/* @func ensGvvariationfeatureGetGvvariationclass *****************************
**
** Get the Ensembl Genetic Variation Variation Class enumeration member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
**
** @return [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration or
** ensEGvvariationClassNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvvariationClass ensGvvariationfeatureGetGvvariationclass(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->GvvariationClass : ensEGvvariationClassNULL;
}




/* @func ensGvvariationfeatureGetIdentifier ***********************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetIdentifier(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Identifier : 0U;
}




/* @func ensGvvariationfeatureGetMapweight ************************************
**
** Get the map weight member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] Map weight or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetMapweight(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Mapweight : 0U;
}




/* @func ensGvvariationfeatureGetName *****************************************
**
** Get the name member of an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetName(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Name : NULL;
}




/* @func ensGvvariationfeatureGetValidationcode *******************************
**
** Get the validation code member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Validation code or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetValidationcode(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? gvvf->Validationcode : NULL;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Genetic Variation Variation
** Feature object, which may need loading from an Ensembl SQL database on
** demand.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Load Return Ensembl Genetic Variation Variation Feature
** attribute(s) loaded on demand
** @nam4rule Gvvariation Return The Ensembl Genetic Variation Variation
**
** @argrule * gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
** Variation Feature
**
** @valrule Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureLoadGvvariation *********************************
**
** Get the Ensembl Genetic Variation Variation member of an
** Ensembl Genetic Variation Variation Feature.
**
** This is not a simple accessor function, it will fetch the Ensembl Genetic
** Variation Variation from the Ensembl Genetic Variation database in case it
** is not defined.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationfeatureLoadGvvariation(
    EnsPGvvariationfeature gvvf)
{
    if (!gvvf)
        return NULL;

    if (gvvf->Gvvariation)
        return gvvf->Gvvariation;

    if (!gvvf->Adaptor)
        return NULL;

    ensGvvariationadaptorFetchByIdentifier(
        ensRegistryGetGvvariationadaptor(
            ensGvvariationfeatureadaptorGetDatabaseadaptor(gvvf->Adaptor)),
        gvvf->Gvvariationidentifier,
        &gvvf->Gvvariation);

    return gvvf->Gvvariation;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Set Set one member of a Genetic Variation Variation Feature
** @nam4rule Adaptor
** Set the Ensembl Genetic Variation Variation Feature Adaptor
** @nam4rule Allele Set the allele
** @nam4rule Consequencetype Set the consequence type
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Gvsource Set the Ensembl Genetic Variation Source
** @nam4rule Gvvariation Set the Ensembl Genetic Variation Variation
** @nam4rule Gvvariationclass
** Set the Ensembl Genetic Variation Variation Class enumeration
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Mapweight Set the map weight
** @nam4rule Name Set the name
** @nam4rule Validationcode Set the validation code
**
** @argrule * gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature object
** @argrule Adaptor gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @argrule Allele allele [AjPStr] Allele
** @argrule Consequencetype consequencetype [AjPStr] Consequence type
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Gvsource gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Gvvariation gvv [EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule Gvvariationclass gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Mapweight mapweight [ajuint] Map weight
** @argrule Name name [AjPStr] Name
** @argrule Validationcode validationcode [AjPStr] Validation code
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationfeatureSetAdaptor **************************************
**
** Set the Ensembl Genetic Variation Variation Feature Adaptor member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetAdaptor(EnsPGvvariationfeature gvvf,
                                       EnsPGvvariationfeatureadaptor gvvfa)
{
    if (!gvvf)
        return ajFalse;

    gvvf->Adaptor = gvvfa;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetAllele ***************************************
**
** Set the allele member of an Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] allele [AjPStr] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetAllele(EnsPGvvariationfeature gvvf,
                                      AjPStr allele)
{
    if (!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Allele);

    if (allele)
        gvvf->Allele = ajStrNewRef(allele);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetConsequencetype ******************************
**
** Set the consequence type member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] consequencetype [AjPStr] Consequence type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetConsequencetype(EnsPGvvariationfeature gvvf,
                                               AjPStr consequencetype)
{
    if (!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Consequencetype);

    if (consequencetype)
        gvvf->Consequencetype = ajStrNewRef(consequencetype);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetFeature **************************************
**
** Set the Ensembl Feature member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetFeature(EnsPGvvariationfeature gvvf,
                                       EnsPFeature feature)
{
    if (!gvvf)
        return ajFalse;

    ensFeatureDel(&gvvf->Feature);

    gvvf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvsource *************************************
**
** Set the Ensembl Genetic Variation Source member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvsource(EnsPGvvariationfeature gvvf,
                                        EnsPGvsource gvsource)
{
    if (!gvvf)
        return ajFalse;

    ensGvsourceDel(&gvvf->Gvsource);

    gvvf->Gvsource = ensGvsourceNewRef(gvsource);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvvariation **********************************
**
** Set the Ensembl Genetic Variation Variation member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvvariation(EnsPGvvariationfeature gvvf,
                                           EnsPGvvariation gvv)
{
    if (!gvvf)
        return ajFalse;

    ensGvvariationDel(&gvvf->Gvvariation);

    gvvf->Gvvariation = ensGvvariationNewRef(gvv);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvvariationclass *****************************
**
** Set the Ensembl Genetic Variation Variation Class enumeration member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
** @param [u] gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvvariationclass(EnsPGvvariationfeature gvvf,
                                                EnsEGvvariationClass gvvc)
{
    if (!gvvf)
        return ajFalse;

    gvvf->GvvariationClass = gvvc;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetIdentifier ***********************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetIdentifier(EnsPGvvariationfeature gvvf,
                                          ajuint identifier)
{
    if (!gvvf)
        return ajFalse;

    gvvf->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetMapweight ************************************
**
** Set the map weight member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] mapweight [ajuint] Map weight
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetMapweight(EnsPGvvariationfeature gvvf,
                                         ajuint mapweight)
{
    if (!gvvf)
        return ajFalse;

    gvvf->Mapweight = mapweight;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetName *****************************************
**
** Set the name member of an Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetName(EnsPGvvariationfeature gvvf,
                                    AjPStr name)
{
    if (!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Name);

    if (name)
        gvvf->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetValidationcode *******************************
**
** Set the validation code member of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] validationcode [AjPStr] Validation code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetValidationcode(EnsPGvvariationfeature gvvf,
                                              AjPStr validationcode)
{
    if (!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Validationcode);

    if (validationcode)
        gvvf->Validationcode = ajStrNewRef(validationcode);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Trace Report Ensembl Genetic Variation Variation Feature members
**                 to debug file
**
** @argrule Trace gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                    Variation Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationfeatureTrace *******************************************
**
** Trace an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureTrace(const EnsPGvvariationfeature gvvf,
                                  ajuint level)
{
    AjPStr indent = NULL;

    if (!gvvf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvvariationfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Gvsource %p\n"
            "%S  Gvvariation %p\n"
            "%S  Allele '%S'\n"
            "%S  Name '%S'\n"
            "%S  Validationcode '%S'\n"
            "%S  Consequencetype '%S'\n"
            "%S  Mapweight %u\n"
            "%S  GvvariationClass '%s'\n",
            indent, gvvf,
            indent, gvvf->Use,
            indent, gvvf->Identifier,
            indent, gvvf->Adaptor,
            indent, gvvf->Feature,
            indent, gvvf->Gvsource,
            indent, gvvf->Gvvariation,
            indent, gvvf->Allele,
            indent, gvvf->Name,
            indent, gvvf->Validationcode,
            indent, gvvf->Consequencetype,
            indent, gvvf->Mapweight,
            indent, ensGvvariationClassToChar(gvvf->GvvariationClass));

    ajStrDel(&indent);

    ensFeatureTrace(gvvf->Feature, level + 1);

    ensGvsourceTrace(gvvf->Gvsource, level + 1);

    ensGvvariationTrace(gvvf->Gvvariation, level + 1);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Calculate
**Calculate Ensembl Genetic Variation Variation Feature information
** @nam4rule Length Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
** Variation Feature
**
** @valrule Length [ajuint] Length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationfeatureCalculateLength *********************************
**
** Calculate the length of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] Length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvvariationfeatureCalculateLength(
    const EnsPGvvariationfeature gvvf)
{
    if (!gvvf)
        return 0U;

    return ensFeatureGetEnd(gvvf->Feature)
        - ensFeatureGetStart(gvvf->Feature)
        + 1U;
}




/* @func ensGvvariationfeatureCalculateMemsize ********************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvvariationfeatureCalculateMemsize(
    const EnsPGvvariationfeature gvvf)
{
    size_t size = 0;

    if (!gvvf)
        return 0;

    size += sizeof (EnsOGvvariationfeature);

    size += ensFeatureCalculateMemsize(gvvf->Feature);

    size += ensGvsourceCalculateMemsize(gvvf->Gvsource);

    size += ensGvvariationCalculateMemsize(gvvf->Gvvariation);

    if (gvvf->Allele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Allele);
    }

    if (gvvf->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Name);
    }

    if (gvvf->Validationcode)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Validationcode);
    }

    if (gvvf->Consequencetype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Consequencetype);
    }

    return size;
}




/* @section convenience functions *********************************************
**
** Ensembl Genetic Variation Variation Feature convenience functions
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Somatic Return the somatic or germline attribute of the
** Ensembl Genetic Variation Source
** @nam4rule Sourcename Return the Ensembl Genetic Variation Source name
**
** @argrule * gvvf [const EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
**
** @valrule Somatic [AjBool] Somatic or germline attribute or ajFalse
** @valrule Sourcename [AjPStr] Ensembl Genetic Variation Source name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureGetSomatic **************************************
**
** Get the somatic member of an
** Ensembl Genetic Variation Variation Feature.
**
** @cc Bio::EnsEMBL::Variation::VariationFeature::somatic
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjBool] Somatic attribute or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureGetSomatic(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? ensGvsourceGetSomatic(gvvf->Gvsource) : ajFalse;
}




/* @func ensGvvariationfeatureGetSourcename ***********************************
**
** Get the name member of an Ensembl Genetic Variation Source associated
** with an Ensembl Genetic Variation Variation Feature.
**
** @cc Bio::EnsEMBL::Variation::VariationFeature::source
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Ensembl Genetic Variation Source name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetSourcename(
    const EnsPGvvariationfeature gvvf)
{
    return (gvvf) ? ensGvsourceGetName(gvvf->Gvsource) : NULL;
}




/* @section query *************************************************************
**
** Functions for querying the properties of an Ensembl Slice.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Is Check whether an Ensembl Genetic Variation Variation Feature
** represents a certain property
** @nam4rule Reference Check for annotation on a reference Ensembl Slice
**
** @argrule * gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
** Variation Feature
** @argrule * Presult [AjBool*] Boolean result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureIsReference *************************************
**
** Check whether an Ensembl Genetic Variation Variation Feature is annotated
** on an Ensembl Slice based on an Ensembl Sequence Region, which
** has an Ensembl Attribute of code "non_ref" set.
**
** @cc Bio::EnsEMBL::Variation::VariationFeature::is_reference
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
** Variation Feature
** @param [u] Presult [AjBool*] ajTrue if the Ensembl Sequence Region has the
**                              Ensembl Attribute of code "non_ref" not set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureIsReference(EnsPGvvariationfeature gvvf,
                                        AjBool *Presult)
{
    AjBool nonreference = AJFALSE;
    AjBool result = AJFALSE;

    EnsPSlice slice = NULL;

    if (!gvvf)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    slice = ensFeatureGetSlice(gvvf->Feature);

    result = ensSliceIsNonreference(slice, &nonreference);

    if (nonreference == ajTrue)
        *Presult = ajFalse;
    else
        *Presult = ajTrue;

    return result;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listGvvariationfeatureCompareEndAscending **********************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by Ensembl Feature end member in ascending order.
**
** @param [r] item1 [const void*]
** Ensembl Genetic VariationVariation Feature address 1
** @param [r] item2 [const void*]
** Ensembl Genetic Variation Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationfeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPGvvariationfeature gvvf1 = *(EnsOGvvariationfeature *const *) item1;
    EnsPGvvariationfeature gvvf2 = *(EnsOGvvariationfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvvariationfeatureCompareEndAscending"))
        ajDebug("listGvvariationfeatureCompareEndAscending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gvvf1 && (!gvvf2))
        return -1;

    if ((!gvvf1) && (!gvvf2))
        return 0;

    if ((!gvvf1) && gvvf2)
        return +1;

    return ensFeatureCompareEndAscending(gvvf1->Feature, gvvf2->Feature);
}




/* @funcstatic listGvvariationfeatureCompareEndDescending *********************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by Ensembl Feature end member in descending order.
**
** @param [r] item1 [const void*]
** Ensembl Genetic Variation Variation Feature address 1
** @param [r] item2 [const void*]
** Ensembl Genetic Variation Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationfeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPGvvariationfeature gvvf1 = *(EnsOGvvariationfeature *const *) item1;
    EnsPGvvariationfeature gvvf2 = *(EnsOGvvariationfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvvariationfeatureCompareEndDescending"))
        ajDebug("listGvvariationfeatureCompareEndDescending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gvvf1 && (!gvvf2))
        return -1;

    if ((!gvvf1) && (!gvvf2))
        return 0;

    if ((!gvvf1) && gvvf2)
        return +1;

    return ensFeatureCompareEndDescending(gvvf1->Feature, gvvf2->Feature);
}




/* @funcstatic listGvvariationfeatureCompareIdentifierAscending ***************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by identifier member in ascending order.
**
** @param [r] item1 [const void*]
** Ensembl Genetic VariationVariation Feature address 1
** @param [r] item2 [const void*]
** Ensembl Genetic Variation Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPGvvariationfeature gvvf1 = *(EnsOGvvariationfeature *const *) item1;
    EnsPGvvariationfeature gvvf2 = *(EnsOGvvariationfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvvariationfeatureCompareIdentifierAscending"))
        ajDebug("listGvvariationfeatureCompareIdentifierAscending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gvvf1 && (!gvvf2))
        return -1;

    if ((!gvvf1) && (!gvvf2))
        return 0;

    if ((!gvvf1) && gvvf2)
        return +1;

    if (gvvf1->Identifier < gvvf2->Identifier)
        return -1;

    if (gvvf1->Identifier > gvvf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listGvvariationfeatureCompareStartAscending ********************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by Ensembl Feature start member in ascending order.
**
** @param [r] item1 [const void*]
** Ensembl Genetic VariationVariation Feature address 1
** @param [r] item2 [const void*]
** Ensembl Genetic Variation Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationfeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPGvvariationfeature gvvf1 = *(EnsOGvvariationfeature *const *) item1;
    EnsPGvvariationfeature gvvf2 = *(EnsOGvvariationfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvvariationfeatureCompareStartAscending"))
        ajDebug("listGvvariationfeatureCompareStartAscending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gvvf1 && (!gvvf2))
        return -1;

    if ((!gvvf1) && (!gvvf2))
        return 0;

    if ((!gvvf1) && gvvf2)
        return +1;

    return ensFeatureCompareStartAscending(gvvf1->Feature, gvvf2->Feature);
}




/* @funcstatic listGvvariationfeatureCompareStartDescending *******************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by Ensembl Feature start member in descending order.
**
** @param [r] item1 [const void*]
** Ensembl Genetic Variation Variation Feature address 1
** @param [r] item2 [const void*]
** Ensembl Genetic Variation Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationfeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPGvvariationfeature gvvf1 = *(EnsOGvvariationfeature *const *) item1;
    EnsPGvvariationfeature gvvf2 = *(EnsOGvvariationfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvvariationfeatureCompareStartDescending"))
        ajDebug("listGvvariationfeatureCompareStartDescending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gvvf1 && (!gvvf2))
        return -1;

    if ((!gvvf1) && (!gvvf2))
        return 0;

    if ((!gvvf1) && gvvf2)
        return +1;

    return ensFeatureCompareStartDescending(gvvf1->Feature, gvvf2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Gvvariationfeature Functions for manipulating AJAX List objects
** of Ensembl Genetic Variation Variation Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * gvvfs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListGvvariationfeatureSortEndAscending ****************************
**
** Sort Ensembl Genetic Variation Variation Feature by their
** Ensembl Feature end coordinate in ascending order.
**
** @param [u] gvvfs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortEndAscending(AjPList gvvfs)
{
    if (!gvvfs)
        return ajFalse;

    ajListSortTwoThree(gvvfs,
                       &listGvvariationfeatureCompareEndAscending,
                       &listGvvariationfeatureCompareStartAscending,
                       &listGvvariationfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGvvariationfeatureSortEndDescending ***************************
**
** Sort Ensembl Genetic Variation Variation Feature by their
** Ensembl Feature end coordinate in descending order.
**
** @param [u] gvvfs [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortEndDescending(AjPList gvvfs)
{
    if (!gvvfs)
        return ajFalse;

    ajListSortTwoThree(gvvfs,
                       &listGvvariationfeatureCompareEndDescending,
                       &listGvvariationfeatureCompareStartDescending,
                       &listGvvariationfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGvvariationfeatureSortIdentifierAscending *********************
**
** Sort Ensembl Genetic Variation Variation Feature by their
** identifier member in ascending order.
**
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortIdentifierAscending(AjPList gvvfs)
{
    if (!gvvfs)
        return ajFalse;

    ajListSort(gvvfs, &listGvvariationfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGvvariationfeatureSortStartAscending **************************
**
** Sort Ensembl Genetic Variation Variation Feature by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortStartAscending(AjPList gvvfs)
{
    if (!gvvfs)
        return ajFalse;

    ajListSortTwoThree(gvvfs,
                       &listGvvariationfeatureCompareStartAscending,
                       &listGvvariationfeatureCompareEndAscending,
                       &listGvvariationfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGvvariationfeatureSortStartDescending *************************
**
** Sort Ensembl Genetic Variation Variation Feature by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortStartDescending(AjPList gvvfs)
{
    if (!gvvfs)
        return ajFalse;

    ajListSortTwoThree(gvvfs,
                       &listGvvariationfeatureCompareStartDescending,
                       &listGvvariationfeatureCompareEndDescending,
                       &listGvvariationfeatureCompareIdentifierAscending);

    return ajTrue;
}




/*
** TODO: The following methods are not implemented:
**   get_all_TranscriptVariations
**   add_TranscriptVariation
**   display_consequence
**   add_consequence_type
*/




/* @datasection [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
** Variation Feature Adaptor
**
** @nam2rule Gvvariationfeatureadaptor Functions for manipulating
** Ensembl Genetic Variation Variation Feature Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor
** @cc CVS Revision: 1.46
** @cc CVS Tag: branch-ensembl-61
**
** NOTE: Since the "source" SQL table contains only a small number of records
** linked to a potentially large number of records in other SQL tables, the
** contents have been modelled as independent objects. The EnsPGvsourceadaptor
** caches all EnsPGvsource entries, which should help reduce memory
** requirements. Consequently, neither the "variation" nor "variation_synonym"
** SQL tables need joining to the "source" table.
**
******************************************************************************/




/* @funcstatic gvvariationfeatureadaptorFetchAllbyStatement *******************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects
** via an SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvvariationfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvfs)
{
    ajint mapweight = 0;

    ajuint lastgvvfid    = 0U;
    ajuint identifier    = 0U;
    ajuint gvvidentifier = 0U;
    ajuint sourceid      = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr allele      = NULL;
    AjPStr name        = NULL;
    AjPStr validation  = NULL;
    AjPStr consequence = NULL;
    AjPStr class       = NULL;

    EnsEGvvariationClass gvvc = ensEGvvariationClassNULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsource        gvsource  = NULL;
    EnsPGvsourceadaptor gvsourcea = NULL;

    EnsPGvvariationfeature        gvvf  = NULL;
    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    EnsPFeature feature = NULL;

    if (ajDebugTest("gvvariationfeatureadaptorFetchAllbyStatement"))
        ajDebug("gvvariationfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvvfs %p\n",
                ba,
                statement,
                am,
                slice,
                gvvfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvvfa     = ensRegistryGetGvvariationfeatureadaptor(dba);
    gvsourcea = ensRegistryGetGvsourceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier    = 0;
        srid          = 0;
        srstart       = 0;
        srend         = 0;
        srstrand      = 0;
        gvvidentifier = 0;
        allele        = ajStrNew();
        name          = ajStrNew();
        mapweight     = 0;
        sourceid      = 0;
        validation    = ajStrNew();
        consequence   = ajStrNew();
        class         = ajStrNew();
        gvvc          = ensEGvvariationClassNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &gvvidentifier);
        ajSqlcolumnToStr(sqlr, &allele);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToInt(sqlr, &mapweight);
        ajSqlcolumnToUint(sqlr, &sourceid);
        ajSqlcolumnToStr(sqlr, &validation);
        ajSqlcolumnToStr(sqlr, &consequence);
        ajSqlcolumnToStr(sqlr, &class);

        /*
        ** Skip multiple rows, because of the LEFT JOIN condition to the
        ** "failed_variation" SQL table.
        */

        if (lastgvvfid == identifier)
        {
            ajStrDel(&allele);
            ajStrDel(&name);
            ajStrDel(&validation);
            ajStrDel(&consequence);
            ajStrDel(&class);

            continue;
        }
        else
            lastgvvfid = identifier;

        ensBaseadaptorRetrieveFeature(ba,
                                      0U,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&allele);
            ajStrDel(&name);
            ajStrDel(&validation);
            ajStrDel(&consequence);
            ajStrDel(&class);

            continue;
        }

        /*
        ** Finally, create a new
        ** Ensembl Genetic Variation Variation Feature.
        */

        ensGvsourceadaptorFetchByIdentifier(gvsourcea, sourceid, &gvsource);

        /* Set the Ensembl Genetic Variation Variation Class. */

        gvvc = ensGvvariationClassFromStr(class);

        if (!gvvc)
            ajFatal("gvvariationfeatureadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'variation_feature.class_so_id' field.\n", class);

        gvvf = ensGvvariationfeatureNewIdentifier(gvvfa,
                                                  identifier,
                                                  feature,
                                                  gvsource,
                                                  gvvc,
                                                  gvvidentifier,
                                                  allele,
                                                  name,
                                                  validation,
                                                  consequence,
                                                  mapweight);

        ajListPushAppend(gvvfs, (void *) gvvf);

        ensGvsourceDel(&gvsource);

        ensFeatureDel(&feature);

        ajStrDel(&allele);
        ajStrDel(&name);
        ajStrDel(&validation);
        ajStrDel(&consequence);
        ajStrDel(&class);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation Feature
** Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation Feature Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationfeatureadaptorNew **************************************
**
** Default constructor for an
** Ensembl Genetic Variation Variation Feature Adaptor.
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
** @see ensRegistryGetGvvariationfeatureadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationfeatureadaptor ensGvvariationfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPFeatureadaptor fa = NULL;

    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    if (!dba)
        return NULL;

    fa = ensFeatureadaptorNew(
        dba,
        gvvariationfeatureadaptorKTablenames,
        gvvariationfeatureadaptorKColumnnames,
        gvvariationfeatureadaptorKLeftjoins,
        gvvariationfeatureadaptorKDefaultcondition,
        (const char *) NULL,
        &gvvariationfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensGvvariationfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensGvvariationfeatureDel,
        (size_t (*)(const void *)) &ensGvvariationfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensGvvariationfeatureGetFeature,
        "Ensembl Genetic Variation Variation Feature");

    if (!fa)
        return NULL;

    AJNEW0(gvvfa);

    gvvfa->Adaptor        = ensRegistryGetGvdatabaseadaptor(dba);
    gvvfa->Featureadaptor = fa;

    return gvvfa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Variation Feature Adaptor object.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @argrule * Pgvvfa [EnsPGvvariationfeatureadaptor*]
** Ensembl Genetic Variation Variation Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationfeatureadaptorDel **************************************
**
** Default destructor for an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvvfa [EnsPGvvariationfeatureadaptor*]
** Ensembl Genetic Variation Variation Feature Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvvariationfeatureadaptorDel(EnsPGvvariationfeatureadaptor *Pgvvfa)
{
    EnsPGvvariationfeatureadaptor pthis = NULL;

    if (!Pgvvfa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvvariationfeatureadaptorDel"))
        ajDebug("ensGvvariationfeatureadaptorDel\n"
                "  *Pgvvfa %p\n",
                *Pgvvfa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvvfa))
        return;

    ensFeatureadaptorDel(&pthis->Featureadaptor);

    ajMemFree((void **) Pgvvfa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Variation Feature Adaptor object.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Variation Feature Adaptor
** attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
**
** @argrule * gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureadaptorGetBaseadaptor ***************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvvariationfeatureadaptorGetBaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensGvvariationfeatureadaptorGetFeatureadaptor(gvvfa));
}




/* @func ensGvvariationfeatureadaptorGetDatabaseadaptor ***********************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvvariationfeatureadaptorGetDatabaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    return ensGvdatabaseadaptorGetDatabaseadaptor(
        ensGvvariationfeatureadaptorGetGvdatabaseadaptor(gvvfa));
}




/* @func ensGvvariationfeatureadaptorGetFeatureadaptor ************************
**
** Get the Ensembl Feature Adaptor member of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensGvvariationfeatureadaptorGetFeatureadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    return (gvvfa) ? gvvfa->Featureadaptor : NULL;
}




/* @func ensGvvariationfeatureadaptorGetGvdatabaseadaptor *********************
**
** Get the Ensembl Genetic Variation Database Adaptor member of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvvariationfeatureadaptorGetGvdatabaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    return (gvvfa) ? gvvfa->Adaptor : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Variation Feature objects
** from an Ensembl SQL database.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation Feature object(s)
** @nam4rule All   Fetch all Ensembl Genetic Variation Variation Feature
** objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Variation Feature
** objects matching a criterion
** @nam5rule Frequency    Fetch all by an Ensembl Genetic Variation Population
** and minor allele frequency
** @nam5rule Gvpopulation Fetch all by an Ensembl Genetic Variation Population
** @nam5rule Gvvariation  Fetch all by an Ensembl Genetic Variation Variation
** @nam5rule Slice        Fetch all by an Ensembl Slice
** @nam6rule Annotated    Fetch all associated with annotations
** @nam6rule Constraint   Fetch all by an SQL constraint
** @nam6rule Genotyped    Fetch all genotyped
** @nam4rule By    Fetch one Ensembl Genetic Variation Variation Feature object
**                 matching a criterion
** @nam5rule Identifier   Fetch by an SQL database-internal identifier
**
** @argrule * gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
** @argrule AllbyFrequency gvp [const EnsPGvpopulation] Ensembl Genetic
** Variation Population
** @argrule AllbyFrequency frequency [float] Minor allele frequency
** @argrule AllbyFrequency slice [EnsPSlice] Ensembl Slice
** @argrule AllbyFrequency gvvfs [AjPList] AJAX List of Ensembl Genetic
** Variation Variation Feature objects
** @argrule AllbyGvpopulation gvp [const EnsPGvpopulation] Ensembl Genetic
** Variation Population
** @argrule AllbyGvpopulation slice [EnsPSlice] Ensembl Slice
** @argrule AllbyGvpopulation gvvfs [AjPList] AJAX List of Ensembl Genetic
** Variation Variation Feature objects
** @argrule AllbyGvvariation gvv [const EnsPGvvariation] Ensembl Genetic
** Variation Variation
** @argrule AllbyGvvariation gvvfs [AjPList] AJAX List of Ensembl Genetic
** Variation Variation Feature objects
** @argrule AllbySliceAnnotated vsource [const AjPStr] variation_feature source
** @argrule AllbySliceAnnotated psource [const AjPStr] annotation_source
** @argrule AllbySliceAnnotated annotation [const AjPStr] annotation_name
** @argrule AllbySliceAnnotated somatic [AjBool] Somatic (ajTrue) or
** germline (ajFalse)
** @argrule AllbySliceAnnotated slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceAnnotated gvvfs [AjPList] AJAX List of Ensembl Genetic
** Variation Variation Feature objects
** @argrule AllbySliceConstraint slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceConstraint constraint [const AjPStr] SQL constraint
** @argrule AllbySliceConstraint somatic [AjBool] Somatic (ajTrue) or
** germline (ajFalse)
** @argrule AllbySliceConstraint gvvfs [AjPList] AJAX List of Ensembl Genetic
** Variation Variation Feature objects
** @argrule AllbySliceGenotyped slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceGenotyped gvvfs [AjPList] AJAX List of Ensembl Genetic
** Variation Variation Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pgvvf [EnsPGvvariationfeature*]
** Ensembl Genetic Variation Variation Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureadaptorFetchAllbyFrequency **********************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects associated
** with an Ensembl Genetic Variation Population and minor allele frequency on
** an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation Feature objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_by_Slice_Population
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic
** Variation Variation Feature Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] frequency [float] Minor allele frequency
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbyFrequency(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvpopulation gvp,
    float frequency,
    EnsPSlice slice,
    AjPList gvvfs)
{
    const char *const *txtcolumns = NULL;

    register ajuint i = 0U;

    AjBool result = AJFALSE;

    AjPStr columns       = NULL;
    AjPStr fvsconstraint = NULL;
    AjPStr statement     = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!gvvfa)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    /* Adjust frequency if given a percentage. */

    if (frequency > 1.0F)
        frequency /= 100.0F;

    /* Build the column expression. */

    ba = ensGvvariationfeatureadaptorGetBaseadaptor(gvvfa);

    txtcolumns = ensBaseadaptorGetColumnnames(ba);

    columns = ajStrNew();

    for (i = 0U; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /* Build the final SQL statement. */

    ensGvdatabaseadaptorFailedvariationsconstraint(
        ensGvvariationfeatureadaptorGetGvdatabaseadaptor(gvvfa),
        (const AjPStr) NULL,
        &fvsconstraint);

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "("
        "variation_feature vf, "
        "source, "
        "allele a"
        ") "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "(variation_feature.variation_id = failed_variation.variation_id) "
        "WHERE "
        "variation_feature.source_id = source.source_id "
        "AND "
        "variation_feature.variation_id = allele.variation_id "
        "AND "
        "allele.sample_id = %u "
        "AND "
        "("
        "IF(allele.frequency > 0.5, 1 - allele.frequency, allele.frequency) > "
        "%f"
        ") "
        "AND "
        "variation_feature.seq_region_id = %u "
        "AND "
        "variation_feature.seq_region_end > %d "
        "AND "
        "variation_feature.seq_region_start < %d "
        "AND "
        "%S "
        "GROUP BY "
        "allele.variation_id",
        columns,
        ensGvpopulationGetIdentifier(gvp),
        frequency,
        ensSliceGetSeqregionIdentifier(slice),
        ensSliceGetStart(slice),
        ensSliceGetEnd(slice),
        fvsconstraint);

    ajStrDel(&fvsconstraint);

    result = gvvariationfeatureadaptorFetchAllbyStatement(
        ba,
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvfs);

    ajStrDel(&columns);
    ajStrDel(&statement);

    return result;
}




/* @func ensGvvariationfeatureadaptorFetchAllbyGvpopulation *******************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects associated
** with an Ensembl Genetic Variation Population on an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation Feature objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_by_Slice_Population
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic
**                                         Variation Variation Feature Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbyGvpopulation(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvpopulation gvp,
    EnsPSlice slice,
    AjPList gvvfs)
{
    const char *const *txtcolumns = NULL;

    register ajuint i = 0U;

    AjBool result = AJFALSE;

    AjPStr columns   = NULL;
    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!gvvfa)
        return ajFalse;

    if (!gvp)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    /* Build the column expression. */

    ba = ensGvvariationfeatureadaptorGetBaseadaptor(gvvfa);

    txtcolumns = ensBaseadaptorGetColumnnames(ba);

    columns = ajStrNew();

    for (i = 0U; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /* Build the final SQL statement. */

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "variation_feature vf, "
        "source, "
        "allele "
        "WHERE "
        "variation_feature.source_id = source.source_id "
        "AND "
        "variation_feature.variation_id = allele.variation_id "
        "AND "
        "allele.sample_id = %u "
        "AND "
        "variation_feature.seq_region_id = %u "
        "AND "
        "variation_feature.seq_region_end > %d "
        "AND "
        "variation_feature.seq_region_start < %d "
        "GROUP BY "
        "allele.variation_id",
        columns,
        ensGvpopulationGetIdentifier(gvp),
        ensSliceGetSeqregionIdentifier(slice),
        ensSliceGetStart(slice),
        ensSliceGetEnd(slice));

    result = gvvariationfeatureadaptorFetchAllbyStatement(
        ba,
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvfs);

    ajStrDel(&columns);
    ajStrDel(&statement);

    return result;
}




/* @func ensGvvariationfeatureadaptorFetchAllbyGvvariation ********************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects by an
** Ensembl Genetic Variation Variation.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation Feature objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_by_Variation
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic
**                                         Variation Variation Feature Adaptor
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbyGvvariation(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvvariation gvv,
    AjPList gvvfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvvfa)
        return ajFalse;

    if (!gvv)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    constraint = ajFmtStr("variation_feature.variation_id = %u",
                          ensGvvariationGetIdentifier(gvv));

    result = ensBaseadaptorFetchAllbyConstraint(
        ensGvvariationfeatureadaptorGetBaseadaptor(gvvfa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationfeatureadaptorFetchAllbySliceAnnotated *****************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects associated
** with annotations on an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation Feature objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     _internal_fetch_all_with_annotation_by_Slice
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_with_annotation_by_Slice
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_somatic_with_annotation_by_Slice
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic
**                                         Variation Variation Feature Adaptor
** @param [r] vsource [const AjPStr] variation_feature source
** @param [r] psource [const AjPStr] annotation_source
** @param [r] annotation [const AjPStr] annotation_name
** @param [r] somatic [AjBool] Somatic (ajTrue) or germline (ajFalse)
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbySliceAnnotated(
    EnsPGvvariationfeatureadaptor gvvfa,
    const AjPStr vsource,
    const AjPStr psource,
    const AjPStr annotation,
    AjBool somatic,
    EnsPSlice slice,
    AjPList gvvfs)
{
    const char *const *txtcolumns = NULL;

    char *txtstring = NULL;

    register ajuint i = 0U;

    AjBool result = AJFALSE;

    AjPStr columns       = NULL;
    AjPStr constraint    = NULL;
    AjPStr fvsconstraint = NULL;
    AjPStr statement     = NULL;

    EnsPBaseadaptor ba = NULL;

    ensGvdatabaseadaptorFailedvariationsconstraint(
        ensGvvariationfeatureadaptorGetGvdatabaseadaptor(gvvfa),
        (const AjPStr) NULL,
        &fvsconstraint);

    constraint = ajFmtStr(
        "vsource.somatic = %d AND %S",
        somatic,
        fvsconstraint);

    ajStrDel(&fvsconstraint);

    if (vsource && ajStrGetLen(vsource))
    {
        ensFeatureadaptorEscapeC(gvvfa->Featureadaptor, &txtstring, vsource);

        ajFmtPrintAppS(&constraint, " AND vsource.name = '%s'", txtstring);

        ajCharDel(&txtstring);
    }

    if (psource && ajStrGetLen(psource))
    {
        ensFeatureadaptorEscapeC(gvvfa->Featureadaptor,
                                 &txtstring,
                                 psource);

        ajFmtPrintAppS(&constraint, " AND psource.name = '%s'", txtstring);

        ajCharDel(&txtstring);
    }

    if (annotation && ajStrGetLen(annotation))
    {
        ensFeatureadaptorEscapeC(gvvfa->Featureadaptor,
                                 &txtstring,
                                 annotation);

        if (ajStrIsNum(annotation))
            ajFmtPrintAppS(&constraint,
                           " AND phenotype.phenotype_id = %s",
                           txtstring);
        else
            ajFmtPrintAppS(&constraint,
                           " AND "
                           "("
                           "phenotype.name = '%s' "
                           "OR "
                           "phenotype.description LIKE '%%%s%%'"
                           ")",
                           txtstring,
                           txtstring);

        ajCharDel(&txtstring);
    }

    /* Build the column expression. */

    ba = ensGvvariationfeatureadaptorGetBaseadaptor(gvvfa);

    txtcolumns = ensBaseadaptorGetColumnnames(ba);

    columns = ajStrNew();

    for (i = 0U; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /* Build the final SQL statement. */

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "("
        "variation_feature, "
        "variation_annotation, "
        "phenotype, "
        "source vsource, "
        "source psource"
        ")"
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "(variation_feature.variation_id = failed_variation.variation_id) "
        "WHERE "
        "variation_annotation.source_id = psource.source_id "
        "AND "
        "variation_feature.source_id = vsource.source_id "
        "AND "
        "variation_feature.variation_id = variation_annotation.variation_id "
        "AND "
        "variation_annotation.phenotype_id = phenotype.phenotype_id "
        "%S "
        "AND "
        "variation_feature.seq_region_id = %u "
        "AND "
        "variation_feature.seq_region_end > %d "
        "AND "
        "variation_feature.seq_region_start < %d "
        "GROUP BY "
        "variation_feature.variation_feature_id",
        columns,
        constraint,
        ensSliceGetSeqregionIdentifier(slice),
        ensSliceGetStart(slice),
        ensSliceGetEnd(slice));

    result = gvvariationfeatureadaptorFetchAllbyStatement(
        ba,
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvfs);

    ajStrDel(&columns);
    ajStrDel(&constraint);
    ajStrDel(&statement);

    return result;
}




/* @func ensGvvariationfeatureadaptorFetchAllbySliceConstraint ****************
**
** Fetch all Ensembl Genetic Variation Variation Feature objects matching a
** SQL constraint on an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation Feature objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_by_Slice
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_by_Slice_constraint
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_somatic_by_Slice
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_somatic_by_Slice_constraint
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] constraint [const AjPStr] SQL constraint
** @param [r] somatic [AjBool] Somatic (ajTrue) or germline (ajFalse)
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbySliceConstraint(
    EnsPGvvariationfeatureadaptor gvvfa,
    EnsPSlice slice,
    const AjPStr constraint,
    AjBool somatic,
    AjPList gvvfs)
{
    AjBool result = AJFALSE;

    AjPStr fvsconstraint = NULL;
    AjPStr newconstraint = NULL;

    if (!gvvfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    ensGvdatabaseadaptorFailedvariationsconstraint(
        ensGvvariationfeatureadaptorGetGvdatabaseadaptor(gvvfa),
        (const AjPStr) NULL,
        &fvsconstraint);

    /* By default, filter out somatic mutations. */

    if (constraint && ajStrGetLen(constraint))
        newconstraint = ajFmtStr("%S AND source.somatic = %d AND %S",
                                 constraint,
                                 somatic,
                                 fvsconstraint);
    else
        newconstraint = ajFmtStr("source.somatic = %d AND %S",
                                 somatic,
                                 fvsconstraint);

    result = ensFeatureadaptorFetchAllbySlice(
        ensGvvariationfeatureadaptorGetFeatureadaptor(gvvfa),
        slice,
        newconstraint,
        (AjPStr) NULL,
        gvvfs);

    ajStrDel(&fvsconstraint);
    ajStrDel(&newconstraint);

    return result;
}




/* @func ensGvvariationfeatureadaptorFetchAllbySliceGenotyped *****************
**
** Fetch all genotyped Ensembl Genetic Variation Variation Feature objects on
** an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Variation Feature objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::
**     fetch_all_genotyped_by_Slice
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic
**                                         Variation Variation Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbySliceGenotyped(
    EnsPGvvariationfeatureadaptor gvvfa,
    EnsPSlice slice,
    AjPList gvvfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvvfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!gvvfs)
        return ajFalse;

    constraint = ajStrNewC("variation_feature.flags & 1");

    result = ensGvvariationfeatureadaptorFetchAllbySliceConstraint(
        gvvfa,
        slice,
        constraint,
        ajTrue,
        gvvfs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationfeatureadaptorFetchByIdentifier ************************
**
** Fetch an Ensembl Genetic Variation Variation Feature via its
** SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Exon.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvvf [EnsPGvvariationfeature*]
** Ensembl Genetic Variation Variation Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchByIdentifier(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPGvvariationfeature *Pgvvf)
{
    return ensBaseadaptorFetchByIdentifier(
        ensGvvariationfeatureadaptorGetBaseadaptor(gvvfa),
        identifier,
        (void **) Pgvvf);
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Genetic Variation Variation Feature-
** releated objects from an Ensembl SQL database.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Genetic Variation Variation Feature-
** related object(s)
** @nam4rule All Retrieve all releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
**
** @argrule * gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer
** (Ensembl Genetic Variation Variation Feature identifier) objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureadaptorRetrieveAllIdentifiers *******************
**
** Retrieve all SQL database-internal identifier objects of
** Ensembl Genetic Variation Variation Feature objects.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::list_dbIDs
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
** @param [u] identifiers [AjPList]
** AJAX List of AJAX unsigned integer
** (Ensembl Genetic Variation Variation Feature identifier) objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorRetrieveAllIdentifiers(
    EnsPGvvariationfeatureadaptor gvvfa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!gvvfa)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("variation_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(
        ensGvvariationfeatureadaptorGetBaseadaptor(gvvfa),
        table,
        (AjPStr) NULL,
        identifiers);

    ajStrDel(&table);

    return result;
}




/* @datasection [EnsPGvvariationset] Ensembl Genetic Variation Variation Set **
**
** @nam2rule Gvvariationset Functions for manipulating
** Ensembl Genetic Variation Variation Set objects
**
** @cc Bio::EnsEMBL::Variation::VariationSet
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation Set
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation Set. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                              Variation Set
** @argrule Ini gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
** Variation Set Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ini description [AjPStr] Description
** @argrule Ini shortname [AjPStr] Short name
** @argrule Ref gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @valrule * [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationsetNewCpy **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [EnsPGvvariationset] Ensembl Genetic Variation Variation Set or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationset ensGvvariationsetNewCpy(const EnsPGvvariationset gvvs)
{
    EnsPGvvariationset pthis = NULL;

    if (!gvvs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvvs->Identifier;
    pthis->Adaptor    = gvvs->Adaptor;

    if (gvvs->Name)
        pthis->Name = ajStrNewRef(gvvs->Name);

    if (gvvs->Description)
        pthis->Description = ajStrNewRef(gvvs->Description);

    if (gvvs->Shortname)
        pthis->Shortname = ajStrNewRef(gvvs->Shortname);

    return pthis;
}




/* @func ensGvvariationsetNewIni **********************************************
**
** Constructor for an Ensembl Genetic Variation Variation Set with initial
** values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [uN] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::VariationSet::new
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [u] shortname [AjPStr] Short name
**
** @return [EnsPGvvariationset] Ensembl Genetic Variation Variation Set or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationset ensGvvariationsetNewIni(EnsPGvvariationsetadaptor gvvsa,
                                           ajuint identifier,
                                           AjPStr name,
                                           AjPStr description,
                                           AjPStr shortname)
{
    EnsPGvvariationset gvvs = NULL;

    if (!name)
        return NULL;

    if (!description)
        return NULL;

    AJNEW0(gvvs);

    gvvs->Use        = 1U;
    gvvs->Identifier = identifier;
    gvvs->Adaptor    = gvvsa;

    if (name)
        gvvs->Name = ajStrNewS(name);

    if (description)
        gvvs->Description = ajStrNewS(description);

    if (shortname)
        gvvs->Shortname = ajStrNewS(shortname);

    return gvvs;
}




/* @func ensGvvariationsetNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
**
** @return [EnsPGvvariationset] Ensembl Genetic Variation Variation Set or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationset ensGvvariationsetNewRef(EnsPGvvariationset gvvs)
{
    if (!gvvs)
        return NULL;

    gvvs->Use++;

    return gvvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Set
**
** @argrule * Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationsetDel *************************************************
**
** Default destructor for an Ensembl Genetic Variation Variation Set.
**
** @param [d] Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvvariationsetDel(EnsPGvvariationset *Pgvvs)
{
    EnsPGvvariationset pthis = NULL;

    if (!Pgvvs)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvvariationsetDel"))
    {
        ajDebug("ensGvvariationsetDel\n"
                "  *Pgvvs %p\n",
                *Pgvvs);

        ensGvvariationsetTrace(*Pgvvs, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvvs) || --pthis->Use)
    {
        *Pgvvs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->Shortname);

    ajMemFree((void **) Pgvvs);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Get Return Genetic Variation Variation Set attribute(s)
** @nam4rule Adaptor
** Return the Ensembl Genetic Variation Variation Set Adaptor
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Description Return the description
** @nam4rule Shortname Return the short name
**
** @argrule * gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @valrule Adaptor [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Shortname [AjPStr] Short name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationsetGetAdaptor ******************************************
**
** Get the Ensembl Genetic Variation Variation Set Adaptor member of an
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationsetadaptor ensGvvariationsetGetAdaptor(
    const EnsPGvvariationset gvvs)
{
    return (gvvs) ? gvvs->Adaptor : NULL;
}




/* @func ensGvvariationsetGetDescription **************************************
**
** Get the description member of an Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [AjPStr] Description or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationsetGetDescription(
    const EnsPGvvariationset gvvs)
{
    return (gvvs) ? gvvs->Description : NULL;
}




/* @func ensGvvariationsetGetIdentifier ***************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvvariationsetGetIdentifier(
    const EnsPGvvariationset gvvs)
{
    return (gvvs) ? gvvs->Identifier : 0U;
}




/* @func ensGvvariationsetGetName *********************************************
**
** Get the name member of an Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [AjPStr] Name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationsetGetName(
    const EnsPGvvariationset gvvs)
{
    return (gvvs) ? gvvs->Name : NULL;
}




/* @func ensGvvariationsetGetShortname ****************************************
**
** Get the short name member of an Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [AjPStr] Short name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvvariationsetGetShortname(
    const EnsPGvvariationset gvvs)
{
    return (gvvs) ? gvvs->Shortname : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Set Set one member of a Genetic Variation Variation Set
** @nam4rule Adaptor
** Set the Ensembl Genetic Variation Variation Set Adaptor
** @nam4rule Description Set the description
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Shortname Set the short name
**
** @argrule * gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set object
** @argrule Adaptor gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @argrule Description description [AjPStr] Description
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Shortname shortname [AjPStr] Short name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationsetSetAdaptor ******************************************
**
** Set the Ensembl Genetic Variation Variation Set Adaptor member of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetSetAdaptor(EnsPGvvariationset gvvs,
                                   EnsPGvvariationsetadaptor gvvsa)
{
    if (!gvvs)
        return ajFalse;

    gvvs->Adaptor = gvvsa;

    return ajTrue;
}




/* @func ensGvvariationsetSetDescription **************************************
**
** Set the description member of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetSetDescription(EnsPGvvariationset gvvs,
                                       AjPStr description)
{
    if (!gvvs)
        return ajFalse;

    ajStrDel(&gvvs->Description);

    if (description)
        gvvs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvvariationsetSetIdentifier ***************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetSetIdentifier(EnsPGvvariationset gvvs,
                                      ajuint identifier)
{
    if (!gvvs)
        return ajFalse;

    gvvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationsetSetName *********************************************
**
** Set the name member of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetSetName(EnsPGvvariationset gvvs,
                                AjPStr name)
{
    if (!gvvs)
        return ajFalse;

    ajStrDel(&gvvs->Name);

    if (name)
        gvvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationsetSetShortname ****************************************
**
** Set the short name member of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [u] shortname [AjPStr] Short name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetSetShortname(EnsPGvvariationset gvvs,
                                     AjPStr shortname)
{
    if (!gvvs)
        return ajFalse;

    ajStrDel(&gvvs->Shortname);

    if (shortname)
        gvvs->Shortname = ajStrNewRef(shortname);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Trace Report Ensembl Genetic Variation Variation Set members
**                 to debug file
**
** @argrule Trace gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationsetTrace ***********************************************
**
** Trace an Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetTrace(const EnsPGvvariationset gvvs,
                              ajuint level)
{
    AjPStr indent = NULL;

    if (!gvvs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvvariationsetTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Description '%S'\n"
            "%S  Shortname '%S'\n",
            indent, gvvs,
            indent, gvvs->Use,
            indent, gvvs->Identifier,
            indent, gvvs->Adaptor,
            indent, gvvs->Name,
            indent, gvvs->Description,
            indent, gvvs->Shortname);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Calculate
** Calculate Ensembl Genetic Variation Variation Set information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationsetCalculateMemsize ************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvvariationsetCalculateMemsize(const EnsPGvvariationset gvvs)
{
    size_t size = 0;

    if (!gvvs)
        return 0;

    size += sizeof (EnsOGvvariationset);

    if (gvvs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvs->Name);
    }

    if (gvvs->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvs->Description);
    }

    if (gvvs->Shortname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvs->Shortname);
    }

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching information from an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation Set information
** @nam4rule All Fetch all objects
** @nam5rule Gvvariations Fetch all Ensembl Genetic Variation Variation objects
** @nam5rule Sub Fetch sub-Ensembl Genetic Variation Variation Set objects
** @nam5rule Super Fetch super-Ensembl Genetic Variation Variation Set objects
**
** @argrule * gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @argrule Sub immediate [AjBool] Only immediate
** @argrule Super immediate [AjBool] Only immediate
** @argrule * gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationsetFetchAllGvvariations ********************************
**
** Recursively fetch all Ensembl Genetic Variation Variation Set objects, which
** belong to this Ensembl Genetic Variation Variation Set and all subsets.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::VariationSet::get_all_Variations
** @param [u] gvvs [EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [u] gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetFetchAllGvvariations(
    EnsPGvvariationset gvvs,
    AjPList gvvss)
{
    if (!gvvs)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    return ensGvvariationadaptorFetchAllbyGvvariationset(
        ensRegistryGetGvvariationadaptor(
            ensGvvariationsetadaptorGetDatabaseadaptor(gvvs->Adaptor)),
        gvvs,
        gvvss);
}




/* @func ensGvvariationsetFetchAllSub *****************************************
**
** Recursively fetch all Ensembl Genetic Variation Variation Set objects, which
** are subsets of this Ensembl Genetic Variation Variation Set.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::VariationSet::get_all_sub_VariationSets
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct subsets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all subsets
** @param [u] gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetFetchAllSub(EnsPGvvariationset gvvs,
                                    AjBool immediate,
                                    AjPList gvvss)
{
    if (!gvvs)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    return ensGvvariationsetadaptorFetchAllbySuper(
        gvvs->Adaptor,
        gvvs,
        immediate,
        gvvss);
}




/* @func ensGvvariationsetFetchAllSuper ***************************************
**
** Recursively fetch all Ensembl Genetic Variation Variation Set objects, which
** are supersets of this Ensembl Genetic Variation Variation Set.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::VariationSet::get_all_sub_VariationSets
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct supersets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all supersets
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetFetchAllSuper(EnsPGvvariationset gvvs,
                                      AjBool immediate,
                                      AjPList gvvss)
{
    if (!gvvs)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    return ensGvvariationsetadaptorFetchAllbySub(
        gvvs->Adaptor,
        gvvs,
        immediate,
        gvvss);
}




/* @datasection [EnsPGvvariationsetadaptor] Ensembl Genetic Variation Variation
** Set Adaptor
**
** @nam2rule Gvvariationsetadaptor Functions for manipulating
** Ensembl Genetic Variation Variation Set Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic gvvariationsetadaptorFetchAllbyStatement ***********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Genetic Variation Variation Set
** objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvvariationsetadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvss)
{
    ajuint identifier  = 0U;
    ajuint attributeid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr description = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvattribute        gva  = NULL;
    EnsPGvattributeadaptor gvaa = NULL;

    EnsPGvvariationset        gvvs  = NULL;
    EnsPGvvariationsetadaptor gvvsa = NULL;

    if (ajDebugTest("gvvariationsetadaptorFetchAllbyStatement"))
        ajDebug("gvvariationsetadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvvss %p\n",
                ba,
                statement,
                am,
                slice,
                gvvss);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvaa  = ensRegistryGetGvattributeadaptor(dba);
    gvvsa = ensRegistryGetGvvariationsetadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0U;
        name        = ajStrNew();
        description = ajStrNew();
        attributeid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToUint(sqlr, &attributeid);

        ensGvattributeadaptorFetchByIdentifier(gvaa, attributeid, &gva);

        gvvs = ensGvvariationsetNewIni(gvvsa,
                                       identifier,
                                       name,
                                       description,
                                       ensGvattributeGetValue(gva));

        ajListPushAppend(gvvss, (void *) gvvs);

        ensGvattributeDel(&gva);

        ajStrDel(&name);
        ajStrDel(&description);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation Set
** Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation Set Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariationsetadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationsetadaptorNew ******************************************
**
** Default constructor for an Ensembl Genetic Variation Variation Set Adaptor.
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
** @see ensRegistryGetGvvariationsetadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvvariationsetadaptor ensGvvariationsetadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensBaseadaptorNew(
        dba,
        gvvariationsetadaptorKTablenames,
        gvvariationsetadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &gvvariationsetadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Variation Set Adaptor object.
**
** @fdata [EnsPGvvariationsetadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Set Adaptor
**
** @argrule * Pgvvsa [EnsPGvvariationsetadaptor*]
** Ensembl Genetic Variation Variation Set Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationsetadaptorDel ******************************************
**
** Default destructor for an Ensembl Genetic Variation Variation Set Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvvsa [EnsPGvvariationsetadaptor*]
** Ensembl Genetic Variation Variation Set Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvvariationsetadaptorDel(EnsPGvvariationsetadaptor *Pgvvsa)
{
    ensBaseadaptorDel(Pgvvsa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Variation Set Adaptor object.
**
** @fdata [EnsPGvvariationsetadaptor]
**
** @nam3rule Get
** Return Ensembl Genetic Variation Variation Set Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationsetadaptorGetBaseadaptor *******************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Variation Set Adaptor.
**
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvvariationsetadaptorGetBaseadaptor(
    EnsPGvvariationsetadaptor gvvsa)
{
    return gvvsa;
}




/* @func ensGvvariationsetadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Variation Set Adaptor.
**
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvvariationsetadaptorGetDatabaseadaptor(
    EnsPGvvariationsetadaptor gvvsa)
{
    return ensBaseadaptorGetDatabaseadaptor(
        ensGvvariationsetadaptorGetBaseadaptor(gvvsa));
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Variation Set objects
** from an Ensembl SQL database.
**
** @fdata [EnsPGvvariationsetadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation Set object(s)
** @nam4rule All   Fetch all Ensembl Genetic Variation Variation Set objects
** @nam5rule Toplevel Fetch all top-level Ensembl Genetic Variation Variation
** Set objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Variation Set objects
**                 matching a criterion
** @nam5rule Gvvariation Fetch all by an Ensembl Genetic Variation Variation
** @nam5rule Sub   Fetch all by a sub-Ensembl Genetic Variation Variation Set
** @nam5rule Super Fetch all by a super-Ensembl Genetic Variation Variation Set
** @nam4rule By    Fetch one Ensembl Genetic Variation Variation Set object
**                 matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
** @nam5rule Name       Fetch by a name
** @nam5rule Shortname  Fetch by a short name
**
** @argrule * gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @argrule All gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
** @argrule Gvvariation gvv [const EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule AllbySub gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @argrule AllbySub immediate [AjBool] only direct sub-sets
** @argrule AllbySuper gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @argrule AllbySuper immediate [AjBool] only direct super-sets
** @argrule Allby gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [const AjPStr] Name
** @argrule Shortname shortname [const AjPStr] Short name
** @argrule By Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationsetadaptorFetchAllToplevel *****************************
**
** Fetch all top-level Ensembl Genetic Variation Variation Set objects, which
** are not sub-sets of any other Ensembl Genetic Variation Variation Set.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::
**     fetch_all_top_VariationSets
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllToplevel(
    EnsPGvvariationsetadaptor gvvsa,
    AjPList gvvss)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvvss)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    constraint = ajStrNewC(
        "NOT EXISTS "
        "("
        "SELECT "
        "* "
        "FROM "
        "variation_set_structure "
        "WHERE "
        "variation_set_structure.variation_set_sub = "
        "variation_set.variation_set_id "
        ")");

    result = ensBaseadaptorFetchAllbyConstraint(
        ensGvvariationsetadaptorGetBaseadaptor(gvvsa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvss);

    ajStrDel(&constraint);

    return result;
}




/* @funcstatic listGvvariationsetCompareIdentifierAscending *******************
**
** AJAX List comparison function to sort Ensembl Genetic Variation Variation
** Set objects by identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Genetic Variation Variation Set 1
** @param [r] item2 [const void*] Ensembl Genetic Variation Variation Set 2
** @see ajListSort
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listGvvariationsetCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPGvvariationset gvvs1 = *(EnsOGvvariationset *const *) item1;
    EnsPGvvariationset gvvs2 = *(EnsOGvvariationset *const *) item2;

    if (ajDebugTest("gvvariationsetlistCompare"))
        ajDebug("gvvariationsetlistCompare\n"
                "  gvvs1 %p\n"
                "  gvvs2 %p\n",
                gvvs1,
                gvvs2);

    if (gvvs1->Identifier < gvvs2->Identifier)
        result = -1;

    if (gvvs1->Identifier > gvvs2->Identifier)
        result = +1;

    return result;
}




/* @funcstatic listGvvariationsetDelete ***************************************
**
** ajListSortUnique "itemdel" function to delete Ensembl Genetic Variation
** Variation Set objects that are redundant.
**
** @param [r] Pitem [void**] Ensembl Genetic Variation Variation Set address
** @param [r] cl [void*] Standard, passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void listGvvariationsetDelete(void **Pitem, void *cl)
{
    if (!Pitem)
        return;

    (void) cl;

    ensGvvariationsetDel((EnsPGvvariationset *) Pitem);

    return;
}




/* @func ensGvvariationsetadaptorFetchAllbyGvvariation ************************
**
** Fetch an Ensembl Genetic Variation Variation Set via an
** Ensembl Genetic Variation Variation.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::fetch_by_Variation
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllbyGvvariation(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariation gvv,
    AjPList gvvss)
{
    const char *const *txtcolumns = NULL;

    register ajuint i = 0U;

    AjBool result = AJFALSE;

    AjPList variationsets = NULL;

    AjPStr columns = NULL;
    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvvariationset gvvs = NULL;

    if (!gvvsa)
        return ajFalse;

    if (!gvv)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    ba = ensGvvariationsetadaptorGetBaseadaptor(gvvsa);

    txtcolumns = ensBaseadaptorGetColumnnames(ba);

    columns = ajStrNew();

    for (i = 0U; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "variation_set, "
        "variation_set_variation "
        "WHERE "
        "variation_set.variation_set_id = "
        "variation_set_variation.variation_set_id "
        "AND "
        "variation_set_variation.variation_id = %u",
        columns,
        ensGvvariationGetIdentifier(gvv));

    ajStrDel(&columns);

    variationsets = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        variationsets);

    ajStrDel(&statement);

    /*
    ** Fetch all supersets of the returned Ensembl Genetic Variation
    ** Variation Set objects as well. Since an Ensembl Genetic Variation
    ** Variation may occur at several places in a hierarchy, which will cause
    ** duplicated data sort the AJAX List uniquely.
    */

    while (ajListPop(variationsets, (void **) &gvvs))
    {
        ensGvvariationsetadaptorFetchAllbySub(gvvsa,
                                              gvvs,
                                              ajFalse,
                                              gvvss);

        /*
        ** Move the current Ensembl Genetic Variation Variation Set onto the
        ** AJAX List of subsets, too.
        */

        ajListPushAppend(gvvss, (void *) gvvs);
    }

    ajListFree(&variationsets);

    ajListSortUnique(gvvss,
                     &listGvvariationsetCompareIdentifierAscending,
                     &listGvvariationsetDelete);

    return result;
}




/* @func ensGvvariationsetadaptorFetchAllbySub ********************************
**
** Fetch all Ensembl Genetic Variation Variation Set objects, by a subset of an
** Ensembl Genetic Variation Variation Set.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::
**     fetch_all_by_sub_VariationSet
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct supersets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all supersets
** @param [u] gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllbySub(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariationset gvvs,
    AjBool immediate,
    AjPList gvvss)
{
    ajuint *Pidentifier = NULL;

    ajuint vssid = 0U;

    AjPList variationsets = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable gvvstable = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationset variationset = NULL;

    if (!gvvsa)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    variationsets = ajListNew();

    /*
    ** Initialise an AJAX Table of AJAX unsigned integer key and
    ** Ensembl Genetic Variation Variation Set value data.
    ** The AJAX Table is cleared by moving Ensembl Genetic Variation
    ** Variation Set objects onto an AJAX List.
    */

    gvvstable = ajTableuintNew(0U);

    dba = ensGvvariationsetadaptorGetDatabaseadaptor(gvvsa);

    /*
    ** First, get all Ensembl Genetic Variation Variation Set objects that are
    ** direct supersets of this one.
    */

    statement = ajFmtStr(
        "SELECT "
        "variation_set_structure.variation_set_super "
        "FROM "
        "variation_set_structure "
        "WHERE "
        "variation_set_structure.variation_set_sub = %u",
        gvvs->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        vssid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &vssid);

        if (!ajTableFetchmodV(gvvstable, (const void *) &vssid))
        {
            ensGvvariationsetadaptorFetchByIdentifier(gvvsa,
                                                      vssid,
                                                      &variationset);

            if (!variationset)
                continue;

            AJNEW0(Pidentifier);

            *Pidentifier = vssid;

            ajTablePut(gvvstable, (void *) Pidentifier, (void *) variationset);

            if (immediate)
                continue;

            ensGvvariationsetadaptorFetchAllbySub(gvvsa,
                                                  variationset,
                                                  immediate,
                                                  variationsets);

            while (ajListPop(variationsets, (void **) &variationset))
            {
                if (!variationset)
                    continue;

                vssid = ensGvvariationsetGetIdentifier(variationset);

                if (!ajTableFetchmodV(gvvstable, (const void *) &vssid))
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = ensGvvariationsetGetIdentifier(
                        variationset);

                    ajTablePut(gvvstable,
                               (void *) Pidentifier,
                               (void *) ensGvvariationsetNewRef(variationset));
                }

                ensGvvariationsetDel(&variationset);
            }
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    ajListFree(&variationsets);

    /*
    ** Move all Ensembl Genetic Variation Variation Set objects from the
    ** AJAX Table onto the AJAX List.
    */

    ensTableuintToList(gvvstable, gvvss);

    ajTableFree(&gvvstable);

    return ajTrue;
}




/* @func ensGvvariationsetadaptorFetchAllbySuper ******************************
**
** Fetch all Ensembl Genetic Variation Variation Set objects, by a superset of
** an Ensembl Genetic Variation Variation Set.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set objects before deleting the
** AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::
**     fetch_all_by_super_VariationSet
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct subsets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all supersets
** @param [u] gvvss [AjPList]
** AJAX List of Ensembl Genetic Variation Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllbySuper(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariationset gvvs,
    AjBool immediate,
    AjPList gvvss)
{
    ajuint *Pidentifier = NULL;

    ajuint vssid = 0U;

    AjPList variationsets = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable gvvstable = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationset variationset = NULL;

    if (!gvvsa)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    if (!gvvss)
        return ajFalse;

    variationsets = ajListNew();

    /*
    ** Initialise an AJAX Table of AJAX unsigned integer key and
    ** Ensembl Genetic Variation Variation Set value data.
    ** The AJAX Table is cleared by moving Ensembl Genetic Variation
    ** Variation Set objects onto an AJAX List.
    */

    gvvstable = ajTableuintNew(0U);

    dba = ensGvvariationsetadaptorGetDatabaseadaptor(gvvsa);

    /*
    ** First, get all Ensembl Genetic Variation Variation Set objects that are
    ** direct supersets of this one.
    */

    statement = ajFmtStr(
        "SELECT "
        "variation_set_structure.variation_set_sub "
        "FROM "
        "variation_set_structure "
        "WHERE "
        "variation_set_structure.variation_set_super = %u",
        gvvs->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        vssid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &vssid);

        if (!ajTableFetchmodV(gvvstable, (const void *) &vssid))
        {
            ensGvvariationsetadaptorFetchByIdentifier(gvvsa,
                                                      vssid,
                                                      &variationset);

            if (!variationset)
                continue;

            AJNEW0(Pidentifier);

            *Pidentifier = vssid;

            ajTablePut(gvvstable, (void *) Pidentifier, (void *) variationset);

            if (immediate)
                continue;

            ensGvvariationsetadaptorFetchAllbySub(gvvsa,
                                                  variationset,
                                                  immediate,
                                                  variationsets);

            while (ajListPop(variationsets, (void **) &variationset))
            {
                if (!variationset)
                    continue;

                vssid = ensGvvariationsetGetIdentifier(variationset);

                if (!ajTableFetchmodV(gvvstable, (const void *) &vssid))
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = ensGvvariationsetGetIdentifier(
                        variationset);

                    ajTablePut(gvvstable,
                               (void *) Pidentifier,
                               (void *) ensGvvariationsetNewRef(variationset));
                }

                ensGvvariationsetDel(&variationset);
            }
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    ajListFree(&variationsets);

    /*
    ** Move all Ensembl Genetic Variation Variation Set objects from the
    ** AJAX Table onto the AJAX List.
    */

    ensTableuintToList(gvvstable, gvvss);

    ajTableFree(&gvvstable);

    return ajTrue;
}




/* @func ensGvvariationsetadaptorFetchByIdentifier ****************************
**
** Fetch an Ensembl Genetic Variation Variation Set via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::fetch_by_dbID
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchByIdentifier(
    EnsPGvvariationsetadaptor gvvsa,
    ajuint identifier,
    EnsPGvvariationset *Pgvvs)
{
    return ensBaseadaptorFetchByIdentifier(
        ensGvvariationsetadaptorGetBaseadaptor(gvvsa),
        identifier,
        (void **) Pgvvs);
}




/* @func ensGvvariationsetadaptorFetchByName **********************************
**
** Fetch an Ensembl Genetic Variation Variation Set via its name.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::fetch_by_name
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] name [const AjPStr] Name
** @param [wP] Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchByName(
    EnsPGvvariationsetadaptor gvvsa,
    const AjPStr name,
    EnsPGvvariationset *Pgvvs)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPList gvvss = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvvariationset gvvs = NULL;

    if (!gvvsa)
        return ajFalse;

    if (!(name && ajStrGetLen(name)))
        return ajFalse;

    if (!Pgvvs)
        return ajFalse;

    *Pgvvs = NULL;

    ba = ensGvvariationsetadaptorGetBaseadaptor(gvvsa);

    ensBaseadaptorEscapeC(ba, &txtname, name);

    constraint = ajFmtStr("variation_set.name = '%s'", txtname);

    ajCharDel(&txtname);

    gvvss = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvvss);

    if (ajListGetLength(gvvss) > 1)
        ajWarn("ensGvvariationsetadaptorFetchByName got more "
               "than one Ensembl Genetic Variation Variation Set for "
               "name '%S'.\n", name);

    ajListPop(gvvss, (void **) Pgvvs);

    while (ajListPop(gvvss, (void **) &gvvs))
        ensGvvariationsetDel(&gvvs);

    ajListFree(&gvvss);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvvariationsetadaptorFetchByShortname *****************************
**
** Fetch an Ensembl Genetic Variation Variation Set via its short name.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::fetch_by_short_name
** @param [u] gvvsa [EnsPGvvariationsetadaptor]
** Ensembl Genetic Variation Variation Set Adaptor
** @param [r] shortname [const AjPStr] Short name
** @param [wP] Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchByShortname(
    EnsPGvvariationsetadaptor gvvsa,
    const AjPStr shortname,
    EnsPGvvariationset *Pgvvs)
{
    AjBool result = AJFALSE;

    AjPList list = NULL;

    AjPStr gvattcode  = NULL;
    AjPStr constraint = NULL;

    EnsPGvattribute gva = NULL;

    EnsPGvvariationset gvvs = NULL;

    if (!gvvsa)
        return ajFalse;

    if (!(shortname && ajStrGetLen(shortname)))
        return ajFalse;

    if (!Pgvvs)
        return ajFalse;

    *Pgvvs = NULL;

    list = ajListNew();

    /* NOTE: _short_name_attrib_type_code has not been implmented. */
    gvattcode = ajStrNewC("short_name");

    ensGvattributeadaptorFetchAllbyCode(
        ensRegistryGetGvattributeadaptor(
            ensGvvariationsetadaptorGetDatabaseadaptor(gvvsa)),
        gvattcode,
        shortname,
        list);

    ajStrDel(&gvattcode);

    ajListPeekFirst(list, (void **) &gva);

    if (!gva)
    {
        while (ajListPop(list, (void **) &gva))
            ensGvattributeDel(&gva);

        ajListFree(&list);

        return ajTrue;
    }

    constraint = ajFmtStr("variation_set.short_name_attrib_id = %u",
                          ensGvattributeGetIdentifier(gva));

    while (ajListPop(list, (void **) &gva))
        ensGvattributeDel(&gva);

    result = ensBaseadaptorFetchAllbyConstraint(
        ensGvvariationsetadaptorGetBaseadaptor(gvvsa),
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        list);

    if (ajListGetLength(list) > 1)
        ajWarn("ensGvvariationsetadaptorFetchByName got more "
               "than one Ensembl Genetic Variation Variation Set for "
               "shortname '%S'.\n", shortname);

    ajListPop(list, (void **) Pgvvs);

    while (ajListPop(list, (void **) &gvvs))
        ensGvvariationsetDel(&gvvs);

    ajListFree(&list);

    ajStrDel(&constraint);

    return result;
}




#if AJFALSE
/* FIXME: Remove? */
AjBool ensGvvariationsetadaptorRetrieveAllGvvariationidentifiers(
    EnsPGvvariationsetadaptor gvvsa,
    EnsPGvvariationset gvvs)
{
    AjPList gvvss = NULL;

    AjPStr csv          = NULL;
    AjPStr   constraint = NULL;
    AjPStr fvconstraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvdatabaseadaptor gvdba = NULL;

    EnsPGvvariationset       lgvvs  = NULL;
    EnsPGvvariationsetadaptor gvvsa = NULL;

    if (!gvvsa)
        return ajFalse;

    if (!gvvs)
        return ajFalse;

    dba = ensGvvariationsetadaptorGetDatabaseadaptor(gvvsa);

    gvdba = ensRegistryGetGvdatabaseadaptor(dba);
    gvvsa = ensRegistryGetGvvariationsetadaptor(dba);

    gvvss = ajListNew();

    csv = ajStrNew();

    ensGvvariationsetadaptorFetchAllbySuper(gvvsa, gvvs, ajFalse, gvvss);

    while (ajListPop(gvvss, (void **) &lgvvs))
    {
        ajFmtPrintAppS(csv, "%u, ", ensGvvariationsetGetIdentifier(lgvvs));

        ensGvvariationsetDel(&lgvvs);
    }
    /* FIXME: Get variation set identifier objects. */

    ensGvdatabaseadaptorFailedvariationsconstraint(gvdba,
                                                   (const AjPStr) NULL,
                                                   &fvconstraint);

    statement = ajFmtStr(
        "SELECT DISTINCT "
        "variation_set_variation.variation_id "
        "FROM "
        "variation_set_variation vsv "
        "LEFT JOIN "
        "failed_variation fv "
        "ON "
        "("
        "failed_variation.variation_id = "
        "variation_set_variation.variation_id"
        ")"
        "WHERE "
        "variation_set_variation.variation_set_id IN (%S) "
        "AND %S",
        csv,
        fvconstraint);

    ajStrDel(&csv);
    ajStrDel(&fvconstraint);

    return ajTrue;
}
#endif /* AJFALSE */

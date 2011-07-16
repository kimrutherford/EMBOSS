/* @source Ensembl Genetic Variation Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:57:09 $ by $Author: mks $
** @version $Revision: 1.10 $
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

#include "ensgene.h"
#include "ensgvallele.h"
#include "ensgvattribute.h"
#include "ensgvdatabaseadaptor.h"
#include "ensgvpopulation.h"
#include "ensgvsource.h"
#include "ensgvsynonym.h"
#include "ensgvvariation.h"
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

/* @conststatic gvvariationClass **********************************************
**
** The Ensembl Genetic Variation Variation class element is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvvariationClass and the
** "variation.class_so_id" field.
**
** #attr [static const char* const*] gvvariationClass
** ##
******************************************************************************/

static const char* const gvvariationClass[] =
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
    (const char*) NULL
};




/* @conststatic gvvariationValidation *****************************************
**
** The Ensembl Genetic Variation Variation validation state element is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvvariationValidation and the
** 'variation.validation_status' field.
**
** #attr [static const char* const*] gvvariationValidation
** ##
******************************************************************************/

static const char* const gvvariationValidation[] =
{
    "",
    "cluster",
    "freq",
    "submitter",
    "doublehit",
    "hapmap",
    "1000genome",
    "failed",
    "precious",
    (const char*) NULL
};




/* @conststatic gvvariationfeatureadaptorTables *******************************
**
** Array of Ensembl Genetic Variation Variation Feature Adaptor
** SQL table names
**
******************************************************************************/

static const char* const gvvariationfeatureadaptorTables[] =
{
    "variation_feature",
    "source",
    "failed_variation",
    (const char*) NULL
};




/* @conststatic gvvariationfeatureadaptorColumns ******************************
**
** Array of Ensembl Genetic Variation Variation Feature Adaptor
** SQL column names
**
******************************************************************************/

static const char* const gvvariationfeatureadaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic gvvariationfeatureadaptorLeftjoin *****************************
**
** Array of Ensembl Genetic Variation Feature Adaptor SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin gvvariationfeatureadaptorLeftjoin[] =
{
    {
        "failed_variation",
        "variation_feature.variation_id = failed_variation.variation_id"
    },
    {(const char*) NULL, (const char*) NULL}
};




/* @conststatic gvvariationfeatureadaptorDefaultcondition *********************
**
** Array of Ensembl Genetic Variation Variation Feature Adaptor
** SQL default condition
**
******************************************************************************/

static const char* gvvariationfeatureadaptorDefaultcondition =
    "variation_feature.source_id = source.source_id";




/* @conststatic gvvariationsetadaptorTables ***********************************
**
** Array of Ensembl Genetic Variation Variation Set Adaptor
** SQL table names
**
******************************************************************************/

static const char* const gvvariationsetadaptorTables[] =
{
    "variation_set",
    (const char*) NULL
};




/* @conststatic gvvariationsetadaptorColumns **********************************
**
** Array of Ensembl Genetic Variation Variation Set Adaptor
** SQL column names
**
******************************************************************************/

static const char* const gvvariationsetadaptorColumns[] =
{
    "variation_set.variation_set_id",
    "variation_set.name",
    "variation_set.description",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void gvvariationClearGenes(void** key,
                                  void** value,
                                  void* cl);

static void tableGvvariationClear(void** key,
                                  void** value,
                                  void* cl);

static AjBool gvvariationadaptorFetchAllbyStatement(
    EnsPGvvariationadaptor gvva,
    const AjPStr statement,
    AjPList gvvs);

static int listGvvariationCompareIdentifier(const void* P1, const void* P2);

static void listGvvariationDelete(void** PP1, void* cl);

static void tableGvvariationadaptorFaileddescriptionsKeyDel(void** key);

static void tableGvvariationadaptorFaileddescriptions1ValueDel(void** value);

static void tableGvvariationadaptorFaileddescriptions2ValueDel(void** value);

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
    AjPStr* Psequence);

static int listGvvariationfeatureCompareStartAscending(const void* P1,
                                                       const void* P2);

static int listGvvariationfeatureCompareStartDescending(const void* P1,
                                                        const void* P2);

static AjBool gvvariationfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvfs);

static void* gvvariationfeatureadaptorCacheReference(void* value);

static void gvvariationfeatureadaptorCacheDelete(void** value);

static size_t gvvariationfeatureadaptorCacheSize(const void* value);

static EnsPFeature gvvariationfeatureadaptorGetFeature(const void* value);

static int listGvvariationsetCompareIdentifier(const void* P1, const void* P2);

static void listGvvariationsetDelete(void** PP1, void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.55
** @cc CVS Tag: branch-ensembl-62
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
** @argrule Ini classgva [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
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
** @argrule Ini somatic [AjBool] Somatic or germline flag
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
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewCpy(const EnsPGvvariation gvv)
{
    AjIList iter = NULL;

    AjPStr description = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    EnsPGvvariation pthis = NULL;

    if(!gvv)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1;
    pthis->Identifier = gvv->Identifier;
    pthis->Adaptor    = gvv->Adaptor;
    pthis->Class      = ensGvattributeNewRef(gvv->Class);
    pthis->Gvsource   = ensGvsourceNewRef(gvv->Gvsource);

    if(gvv->Name)
        pthis->Name = ajStrNewRef(gvv->Name);

    if(gvv->Ancestralallele)
        pthis->Ancestralallele = ajStrNewRef(gvv->Ancestralallele);

    /*
    ** NOTE: Copy the AJAX List of
    ** Ensembl Genetic Variation Allele objects.
    ** TODO: The Perl implementation uses a private _add_Allele method to
    ** weaken the link from the Allele object back to this Variation object.
    */

    if(gvv->Gvalleles)
    {
        pthis->Gvalleles = ajListNew();

        iter = ajListIterNew(gvv->Gvalleles);

        while(!ajListIterDone(iter))
        {
            gva = (EnsPGvallele) ajListIterGet(iter);

            ajListPushAppend(pthis->Gvalleles,
                             (void*) ensGvalleleNewRef(gva));
        }

        ajListIterDel(&iter);
    }

    /*
    ** NOTE: Copy the AJAX List of
    ** Ensembl Genetic Variation Synonym objects.
    */

    if(gvv->Gvsynonyms)
    {
        pthis->Gvsynonyms = ajListNew();

        iter = ajListIterNew(gvv->Gvsynonyms);

        while(!ajListIterDone(iter))
        {
            gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

            ajListPushAppend(pthis->Gvsynonyms,
                             (void*) ensGvsynonymNewRef(gvsynonym));
        }

        ajListIterDel(&iter);
    }

    /* NOTE: Copy the AJAX List of AJAX String (failed description) objects. */

    if(gvv->Faileddescriptions)
    {
        pthis->Faileddescriptions = ajListstrNew();

        iter = ajListIterNew(gvv->Faileddescriptions);

        while(!ajListIterDone(iter))
        {
            description = ajListstrIterGet(iter);

            if(description)
                ajListstrPushAppend(pthis->Faileddescriptions,
                                    ajStrNewS(description));
        }

        ajListIterDel(&iter);
    }

    if(gvv->Moleculetype)
        pthis->Moleculetype = ajStrNewRef(gvv->Moleculetype);

    if(gvv->FlankFive)
        pthis->FlankFive = ajStrNewRef(gvv->FlankFive);

    if(gvv->FlankThree)
        pthis->FlankThree = ajStrNewRef(gvv->FlankThree);

    pthis->FlankExists = gvv->FlankExists;
    pthis->Somatic     = gvv->Somatic;
    pthis->Validations = gvv->Validations;

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
** @param [u] classgva [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
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
** @param [r] somatic [AjBool] Somatic or germline flag
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewIni(EnsPGvvariationadaptor gvva,
                                     ajuint identifier,
                                     EnsPGvattribute classgva,
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
                                     AjBool somatic)
{
    AjIList iter = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvsource)
        return NULL;

    if(!name)
        return NULL;

    AJNEW0(gvv);

    gvv->Use        = 1;
    gvv->Identifier = identifier;
    gvv->Adaptor    = gvva;
    gvv->Class      = ensGvattributeNewRef(classgva);
    gvv->Gvsource   = ensGvsourceNewRef(gvsource);

    if(name)
        gvv->Name = ajStrNewRef(name);

    if(ancestralallele)
        gvv->Ancestralallele = ajStrNewRef(ancestralallele);

    /* Copy the AJAX List of Ensembl Genetic Variation Allele objects. */

    if(gvalleles)
    {
        gvv->Gvalleles = ajListNew();

        iter = ajListIterNew(gvalleles);

        while(!ajListIterDone(iter))
        {
            gva = (EnsPGvallele) ajListIterGet(iter);

            ajListPushAppend(gvv->Gvalleles,
                             (void*) ensGvalleleNewRef(gva));
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX List of Ensembl Genetic Variation Synonym objects. */

    if(gvsynonyms)
    {
        gvv->Gvsynonyms = ajListNew();

        iter = ajListIterNew(gvsynonyms);

        while(!ajListIterDone(iter))
        {
            gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

            ajListPushAppend(gvv->Gvsynonyms,
                             (void*) ensGvsynonymNewRef(gvsynonym));
        }

        ajListIterDel(&iter);
    }

    if(moltype)
        gvv->Moleculetype = ajStrNewRef(moltype);

    if(flankfive)
        gvv->FlankFive = ajStrNewRef(flankfive);

    if(flankthree)
        gvv->FlankThree = ajStrNewRef(flankthree);

    gvv->FlankExists = flankexists;
    gvv->Somatic     = somatic;
    gvv->Validations = ensGvvariationValidationsFromSet(validationstates);

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
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewRef(EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    gvv->Use++;

    return gvv;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Variation object
**
** @argrule * Pgvv [EnsPGvvariation*] Ensembl Genetic Variation Variation
**                                    object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationDel ****************************************************
**
** Default destructor for an Ensembl Genetic Variation Variation.
**
** @param [d] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation Variation
**                                    object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationDel(EnsPGvvariation* Pgvv)
{
    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    EnsPGvvariation pthis = NULL;

    if(!Pgvv)
        return;

    if(!*Pgvv)
        return;

    if(ajDebugTest("ensGvvariationDel"))
    {
        ajDebug("ensGvvariationDel\n"
                "  *Pgvv %p\n",
                *Pgvv);

        ensGvvariationTrace(*Pgvv, 1);
    }

    pthis = *Pgvv;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvv = NULL;

        return;
    }

    ensGvattributeDel(&pthis->Class);

    ensGvsourceDel(&pthis->Gvsource);

    ajStrDel(&pthis->Name);

    ajStrDel(&pthis->Ancestralallele);

    /* Clear the AJAX List of Ensembl Genetic Variation Allele objects. */

    while(ajListPop(pthis->Gvalleles, (void**) &gva))
        ensGvalleleDel(&gva);

    ajListFree(&pthis->Gvalleles);

    /* Clear the AJAX List of Ensembl Genetic Variation Synonym objects. */

    while(ajListPop(pthis->Gvsynonyms, (void**) &gvsynonym))
        ensGvsynonymDel(&gvsynonym);

    ajListFree(&pthis->Gvsynonyms);

    /* Clear the AJAX List of AJAX String (failed description) objects. */

    ajListstrFreeData(&pthis->Faileddescriptions);

    ajStrDel(&pthis->Moleculetype);
    ajStrDel(&pthis->FlankFive);
    ajStrDel(&pthis->FlankThree);

    AJFREE(pthis);

    *Pgvv = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Get Return Genetic Variation Variation attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Variation Adaptor
** @nam4rule Ancestralallele Return the ancestral allele
** @nam4rule Class Return the Class Ensembl Genetic Variation Attribute
** @nam4rule Flank Return flanking sequence elements
** @nam5rule Exists Return the flanking sequence exists
** @nam5rule Five Return the five-prime flanking sequence
** @nam5rule Three Return the three-prime flanking sequence
** @nam4rule Gvalleles Return Ensembl Genetic Variation Allele objects
** @nam4rule Gvsource Return the Ensembl Genetic Variation Source
** @nam4rule Gvsynonyms Return Ensembl Genetic Variation Synonms
** @nam4rule Identifier Return the SQL database-internal identifier
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
** @valrule FlankFive [AjPStr] Five-prime flanking sequence or NULL
** @valrule FlankThree [AjPStr] Three-prime flanking sequence or NULL
** @valrule FlankExists [AjBool] Flanking sequence exists or ajFalse
** @valrule Gvalleles [const AjPList] Ensembl Genetic Variation Allele objects
** or NULL
** @valrule Gvsource [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @valrule Gvsynonyms [const AjPList] Ensembl Genetic Variation Synonym
** objects or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Moleculetype [AjPStr] Molecule type or NULL
** @valrule Name [AjPStr] Name or NULL
** @valrule Somatic [AjBool] Somatic or germline flag or ajFalse
** @valrule Validations [ajuint] Validations bit field or 0
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationGetAdaptor *********************************************
**
** Get the Ensembl Genetic Variation Variation Adaptor element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                  Variation Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationadaptor ensGvvariationGetAdaptor(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Adaptor;
}




/* @func ensGvvariationGetAncestralallele *************************************
**
** Get the ancestral allele element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::ancestral_allele
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Ancestral allele or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationGetAncestralallele(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Ancestralallele;
}




/* @func ensGvvariationGetClass ***********************************************
**
** Get the class Ensembl Genetic Variation Attribute element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::var_class
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvattribute] Class Ensembl Genetic Variation Attribute or NULL
** @@
** FIXME: The Perl Bio::EnsEMBL::Variation::Variation::var_class method is not
** a simple accessor method, it rather assembles the class from
** Ensembl Genetic Variation Allele objects, but depends on
** Bio::EnsEMBL::Variation::Utils::Sequence::SO_variation_class.
******************************************************************************/

EnsPGvattribute ensGvvariationGetClass(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Class;
}




/* @func ensGvvariationGetFlankExists *****************************************
**
** Get the flanking sequence exists element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::flank_flag
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] Flanking sequence exists or ajFalse
** @@
******************************************************************************/

AjBool ensGvvariationGetFlankExists(const EnsPGvvariation gvv)
{
    if(!gvv)
        return ajFalse;

    return gvv->FlankExists;
}




/* @func ensGvvariationGetGvalleles *******************************************
**
** Get all Ensembl Genetic Variation Allele objects of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_Alleles
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [const AjPList] AJAX List of Ensembl Genetic Variation Allele
**                         objects or NULL
** @@
******************************************************************************/

const AjPList ensGvvariationGetGvalleles(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Gvalleles;
}




/* @func ensGvvariationGetGvsource ********************************************
**
** Get the Ensembl Genetic Variation Source element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::source
** @cc Bio::EnsEMBL::Variation::Variation::source_description
** @cc Bio::EnsEMBL::Variation::Variation::source_url
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @@
******************************************************************************/

EnsPGvsource ensGvvariationGetGvsource(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Gvsource;
}




/* @func ensGvvariationGetGvsynonyms ******************************************
**
** Get the AJAX List of Ensembl Genetic Variation Synonym objects element
** of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_synonyms
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @see ensGvvariationFetchAllGvsynonyms
** @see ensGvvariationFetchAllGvsources
**
** @return [const AjPList] AJAX List of Ensembl Genetic Variation Synonym
** objects or NULL
** @@
******************************************************************************/

const AjPList ensGvvariationGetGvsynonyms(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Gvsynonyms;
}




/* @func ensGvvariationGetIdentifier ******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvvariationGetIdentifier(const EnsPGvvariation gvv)
{
    if(!gvv)
        return 0;

    return gvv->Identifier;
}




/* @func ensGvvariationGetMoleculetype ****************************************
**
** Get the molecule type element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::moltype
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Molecule type or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationGetMoleculetype(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Moleculetype;
}




/* @func ensGvvariationGetName ************************************************
**
** Get the name element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::name
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationGetName(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Name;
}




/* @func ensGvvariationGetSomatic *********************************************
**
** Get the somatic or germline element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] Somatic or germline flag or ajFalse
** @@
******************************************************************************/

AjBool ensGvvariationGetSomatic(const EnsPGvvariation gvv)
{
    if(!gvv)
        return ajFalse;

    return gvv->Somatic;
}




/* @func ensGvvariationGetValidations *****************************************
**
** Get the validations bit field of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_validation_states
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] Validations bit field or 0
** @@
******************************************************************************/

ajuint ensGvvariationGetValidations(const EnsPGvvariation gvv)
{
    if(!gvv)
        return 0;

    return gvv->Validations;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Genetic Variation Variation
** object, which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Load Return Ensembl Genetic Variation Variation attribute(s)
** loaded on demand
** @nam4rule All Returns all Ensembl Genetic Variation Variation attributes
** loaded on demand
** @nam5rule Faileddescriptions Returns an AJAX List of AJAX String
** (failed description) objects
** @nam4rule Flank Return a flanking sequence
** @nam5rule Five Return the five-prime flanking sequence
** @nam5rule Three Return the three-prime flanking sequence
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @valrule Faileddescriptions [const AjPList]
** AJAX List of AJAX String (failed description) objects
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
** @return [const AjPList] AJAX List of AJAX String (failed description)
** objects or NULL
** @@
******************************************************************************/

const AjPList ensGvvariationLoadAllFaileddescriptions(EnsPGvvariation gvv)
{
    if(!gvv)
        return ajFalse;

    if(gvv->Faileddescriptions)
        return gvv->Faileddescriptions;

    if(!gvv->Adaptor)
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




/* @func ensGvvariationLoadFlankFive ******************************************
**
** Get the five-prime flank element of an Ensembl Genetic Variation Variation.
**
** This is not a simple accessor function, it will fetch the five-prime
** flanking sequence from the Ensembl Genetic Variation database in case it is
** not defined.
**
** @cc Bio::EnsEMBL::Variation::Variation::five_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Five-prime flank or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationLoadFlankFive(EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    if(gvv->FlankFive)
        return gvv->FlankFive;

    if(gvv->Adaptor)
        ensGvvariationadaptorRetrieveFlank(gvv->Adaptor,
                                           gvv->Identifier,
                                           &gvv->FlankFive,
                                           &gvv->FlankThree);

    return gvv->FlankFive;
}




/* @func ensGvvariationLoadFlankThree *****************************************
**
** Get the three-prime flank element of an Ensembl Genetic Variation Variation.
**
** This is not a simple accessor function, it will fetch the three-prime
** flanking sequence from the Ensembl Genetic Variation database in case it is
** not defined.
**
** @cc Bio::EnsEMBL::Variation::Variation::three_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Three-prime flank or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationLoadFlankThree(EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    if(gvv->FlankThree)
        return gvv->FlankThree;

    if(gvv->Adaptor)
        ensGvvariationadaptorRetrieveFlank(gvv->Adaptor,
                                           gvv->Identifier,
                                           &gvv->FlankFive,
                                           &gvv->FlankThree);

    return gvv->FlankThree;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Set Set one element of a Genetic Variation Variation
** @nam4rule Adaptor Set the Ensembl Genetic Variation Variation Adaptor
** @nam4rule Ancestralallele Set the ancestral allele
** @nam4rule Class Set the
** Class Ensembl Genetic Variation Attribute
** @nam4rule Flank Set flanking sequence elements
** @nam5rule Exists Set the flanking sequence exists
** @nam5rule Five Set the five-prime flank
** @nam5rule Three Set the three-prime flank
** @nam4rule Gvsource Set the Ensembl Genetic Variation Source
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Moleculetype Set the molecule type
** @nam4rule Name Set the name
** @nam4rule Somatic Set the somatic or germline flag
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation object
** @argrule Adaptor gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
** @argrule Ancestralallele ancestralallele [AjPStr] Ancestral allele
** @argrule Class classgva [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
** @argrule FlankExists flankexists [AjBool] Flank
** @argrule FlankFive flankfive [AjPStr] Five-prime flank
** @argrule FlankThree flankthree [AjPStr] Three-prime flank
** @argrule Gvsource gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
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
** Set the Ensembl Genetic Variation Variation Adaptor element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetAdaptor(EnsPGvvariation gvv,
                                EnsPGvvariationadaptor gvva)
{
    if(!gvv)
        return ajFalse;

    gvv->Adaptor = gvva;

    return ajTrue;
}




/* @func ensGvvariationSetAncestralallele *************************************
**
** Set the ancestral allele element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::ancestral_allele
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] ancestralallele [AjPStr] Ancestral allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetAncestralallele(EnsPGvvariation gvv,
                                        AjPStr ancestralallele)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->Ancestralallele);

    if(ancestralallele)
        gvv->Ancestralallele = ajStrNewRef(ancestralallele);

    return ajTrue;
}




/* @func ensGvvariationSetClass ***********************************************
**
** Set the class Ensembl Genetic Variation Attribute element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::var_class
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] classgva [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetClass(EnsPGvvariation gvv,
                              EnsPGvattribute classgva)
{
    if(!gvv)
        return ajFalse;

    ensGvattributeDel(&gvv->Class);

    gvv->Class = ensGvattributeNewRef(classgva);

    return ajTrue;
}




/* @func ensGvvariationSetFlankExists *****************************************
**
** Set the flanking sequence exists element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::flank_flag
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] flankexists [AjBool] Flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetFlankExists(EnsPGvvariation gvv,
                                    AjBool flankexists)
{
    if(!gvv)
        return ajFalse;

    gvv->FlankExists = flankexists;

    return ajTrue;
}




/* @func ensGvvariationSetFlankFive *******************************************
**
** Set the five-prime flank element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::five_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] flankfive [AjPStr] Five-prime flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetFlankFive(EnsPGvvariation gvv,
                                  AjPStr flankfive)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->FlankFive);

    if(flankfive)
        gvv->FlankFive = ajStrNewRef(flankfive);

    return ajTrue;
}




/* @func ensGvvariationSetFlankThree ******************************************
**
** Set the three-prime flank element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::three_prime_flanking_seq
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] flankthree [AjPStr] Three-prime flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetFlankThree(EnsPGvvariation gvv,
                                   AjPStr flankthree)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->FlankThree);

    if(flankthree)
        gvv->FlankThree = ajStrNewRef(flankthree);

    return ajTrue;
}




/* @func ensGvvariationSetGvsource ********************************************
**
** Set the Ensembl Genetic Variation Source element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::source
** @cc Bio::EnsEMBL::Variation::Variation::source_description
** @cc Bio::EnsEMBL::Variation::Variation::source_url
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetGvsource(EnsPGvvariation gvv,
                                 EnsPGvsource gvs)
{
    if(!gvv)
        return ajFalse;

    ensGvsourceDel(&gvv->Gvsource);

    gvv->Gvsource = ensGvsourceNewRef(gvs);

    return ajTrue;
}




/* @func ensGvvariationSetIdentifier ******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetIdentifier(EnsPGvvariation gvv,
                                   ajuint identifier)
{
    if(!gvv)
        return ajFalse;

    gvv->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationSetMoleculetype ****************************************
**
** Set the molecule type element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::moltype
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] moltype [AjPStr] Molecule type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetMoleculetype(EnsPGvvariation gvv,
                                     AjPStr moltype)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->Moleculetype);

    if(moltype)
        gvv->Moleculetype = ajStrNewRef(moltype);

    return ajTrue;
}




/* @func ensGvvariationSetName ************************************************
**
** Set the name element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::name
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetName(EnsPGvvariation gvv,
                             AjPStr name)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->Name);

    if(name)
        gvv->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationSetSomatic *********************************************
**
** Set the somatic or germline element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] somatic [AjBool] Somatic or germline flag
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetSomatic(EnsPGvvariation gvv,
                                AjBool somatic)
{
    if(!gvv)
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
** @nam3rule Trace Report Ensembl Genetic Variation Variation elements
**                 to debug file
**
** @argrule Trace gvv [const EnsPGvvariation] Ensembl Genetic Variation
**                                            Variation
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
** @@
******************************************************************************/

AjBool ensGvvariationTrace(const EnsPGvvariation gvv, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr states = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvs = NULL;

    if(!gvv)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvvariationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Class %p"
            "%S  Gvsource %p\n"
            "%S  Name '%S'\n"
            "%S  Ancestralallele '%S'\n"
            "%S  Gvalleles %p\n"
            "%S  Gvsynonyms %p\n"
            "%S  Moleculetype '%S'\n"
            "%S  FlankFive '%S'\n"
            "%S  FlankThree '%S'\n"
            "%S  FlankExists '%B'\n"
            "%S  Somatic '%B'\n"
            "%S  Validations %u\n",
            indent, gvv,
            indent, gvv->Use,
            indent, gvv->Identifier,
            indent, gvv->Adaptor,
            indent, gvv->Class,
            indent, gvv->Gvsource,
            indent, gvv->Name,
            indent, gvv->Ancestralallele,
            indent, gvv->Gvalleles,
            indent, gvv->Gvsynonyms,
            indent, gvv->Moleculetype,
            indent, gvv->FlankFive,
            indent, gvv->FlankThree,
            indent, gvv->FlankExists,
            indent, gvv->Somatic,
            indent, gvv->Validations);

    ensGvattributeTrace(gvv->Class, level + 1);

    ensGvsourceTrace(gvv->Gvsource, level + 1);

    states = ajStrNew();

    ensGvvariationValidationsToSet(gvv->Validations, &states);

    ajDebug("%S  Validation states SQL set: '%S'\n", indent, states);

    ajStrDel(&states);

    /* Trace the AJAX List of Ensembl Genetic Variation Allele objects. */

    ajDebug("%S  Ensembl Genetic Variation Alleles:\n", indent);

    iter = ajListIterNew(gvv->Gvalleles);

    while(!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        ensGvalleleTrace(gva, level + 1);
    }

    ajListIterDel(&iter);

    /* Trace the AJAX List of Ensembl Genetic Variation Synonym objects. */

    ajDebug("%S  Ensembl Genetic Variation Synonyms:\n", indent);

    iter = ajListIterNew(gvv->Gvsynonyms);

    while(!ajListIterDone(iter))
    {
        gvs = (EnsPGvsynonym) ajListIterGet(iter);

        ensGvsynonymTrace(gvs, level + 1);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Variation values
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
** @@
******************************************************************************/

size_t ensGvvariationCalculateMemsize(const EnsPGvvariation gvv)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    if(!gvv)
        return 0;

    size += sizeof (EnsOGvvariation);

    size += ensGvattributeCalculateMemsize(gvv->Class);

    size += ensGvsourceCalculateMemsize(gvv->Gvsource);

    if(gvv->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->Name);
    }

    if(gvv->Ancestralallele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->Ancestralallele);
    }

    /* Summarise the AJAX List of Ensembl Genetic Variation Allele objects. */

    iter = ajListIterNewread(gvv->Gvalleles);

    while(!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        size += ensGvalleleCalculateMemsize(gva);
    }

    ajListIterDel(&iter);

    /* Summarise the AJAX List of Ensembl Genetic Variation Synonym objects. */

    iter = ajListIterNewread(gvv->Gvsynonyms);

    while(!ajListIterDone(iter))
    {
        gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

        size += ensGvsynonymCalculateMemsize(gvsynonym);
    }

    ajListIterDel(&iter);

    if(gvv->Moleculetype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->Moleculetype);
    }

    if(gvv->FlankFive)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->FlankFive);
    }

    if(gvv->FlankThree)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvv->FlankThree);
    }

    return size;
}




/* @section element addition **************************************************
**
** Functions for adding elements to an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Add Add one object to an Ensembl Genetic Variation Variation
** @nam4rule Faileddescription Add a failed description
** @nam4rule Gvallele Add an Ensembl Genetic Variation Allele
** @nam4rule Gvsynonym Add an Ensembl Genetic Variation Synonym
** @nam4rule Validation Add an Ensembl Genetic Variation Variation Validation
** enumeration
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation object
** @argrule Faileddescription description [AjPStr] Failed description
** @argrule Gvallele gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Gvsynonym gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @argrule Validation gvvv [EnsEGvvariationValidation] Ensembl Genetic
** Variation Validation enumeration
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
** @@
******************************************************************************/

AjBool ensGvvariationAddFaileddescription(EnsPGvvariation gvv,
                                          AjPStr description)
{
    if(!gvv)
        return ajFalse;

    if(!description)
        return ajFalse;

    if(!gvv->Faileddescriptions)
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
** @@
******************************************************************************/

AjBool ensGvvariationAddGvallele(EnsPGvvariation gvv,
                                 EnsPGvallele gva)
{
    if(!gvv)
        return ajFalse;

    if(!gva)
        return ajFalse;

    if(!gvv->Gvalleles)
        gvv->Gvalleles = ajListNew();

    ajListPushAppend(gvv->Gvalleles, (void*) ensGvalleleNewRef(gva));

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
** @@
******************************************************************************/

AjBool ensGvvariationAddGvsynonym(EnsPGvvariation gvv,
                                  EnsPGvsynonym gvs)
{
    if(!gvv)
        return ajFalse;

    if(!gvs)
        return ajFalse;

    if(!gvv->Gvsynonyms)
        gvv->Gvsynonyms = ajListNew();

    ajListPushAppend(gvv->Gvsynonyms, (void*) ensGvsynonymNewRef(gvs));

    return ajTrue;
}




/* @func ensGvvariationAddValidation ******************************************
**
** Add a validation state to an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::add_validation_state
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvvv [EnsEGvvariationValidation] Ensembl Genetic Variation
**                                             Variation Validation enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationAddValidation(EnsPGvvariation gvv,
                                   EnsEGvvariationValidation gvvv)
{
    if(!gvv)
        return ajFalse;

    gvv->Validations |= (1 << gvvv);

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation values
** @nam4rule All Fetch all objects
** @nam5rule Genes Fetch all Ensembl Gene objects
** @nam5rule Gvsources Fetch all Ensembl Genetic Variation Source objects
** @nam5rule Gvsynonyms Fetch all Ensembl Genetic Variation Synonym objects
** @nam5rule Gvvariationfeatures Fetch all Ensembl Genetic Variation Variation
** Feature objects
**
** @argrule AllGenes gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @argrule AllGenes flank [ajint] Overlap between Gene and Variation Feature
**                                 objects, defaults to 5000 base pairs
** @argrule AllGenes genes [AjPList] AJAX List of Ensembl Gene objects
** @argrule AllGvsources gvv [const EnsPGvvariation] Ensembl Genetic Variation
** Variation
** @argrule AllGvsources gvss [AjPList] AJAX List of Ensembl Genetic Variation
** Source objects
** @argrule AllGvsynonyms gvv [const EnsPGvvariation] Ensembl Genetic
** Variation Variation
** @argrule AllGvsynonyms gvsourcename [const AjPStr] Ensembl Genetic
** Variation Source name
** @argrule AllGvsynonyms gvss [AjPList] AJAX List of Ensembl Genetic
** Variation Synonym objects
** @argrule AllGvvariationfeatures gvv [EnsPGvvariation] Ensembl Genetic
** Variation Variation
** @argrule AllGvvariationfeatures gvvfs [AjPList] AJAX List of
** Ensembl Genetic Variation Variation Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic gvvariationClearGenes ******************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of Ensembl Gene
** objects used in ensGvvariationFetchAllGenes. This function deletes the
** AJAX unsigned identifier key data and moves the Ensembl Gene value data
** onto the AJAX List.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Gene value data address
** @param [u] cl [void*] AJAX List, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvvariationClearGenes(void** key,
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

    if(!cl)
        return;

    AJFREE(*key);

    ajListPushAppend(cl, value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensGvvariationFetchAllGenes ******************************************
**
** Fetch all Ensembl Gene objects where an Ensembl Genetic Variation Variation
** has a consequence.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_Genes
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] flank [ajint] Overlap between Gene and Variation Feature
**                          objects, defaults to 5000 base pairs
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGenes(EnsPGvvariation gvv,
                                   ajint flank,
                                   AjPList genes)
{
    ajuint* Pidentifier = NULL;

    ajuint identifier = 0;

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

    EnsPSlice        slice  = NULL;
    EnsPSlice     newslice  = NULL;
    EnsPSliceadaptor slicea = NULL;

    if(!gvv)
        return ajFalse;

    if(!flank)
        flank = 5000;

    if(!genes)
        return ajFalse;

    if(!gvv->Adaptor)
        return ajTrue;

    genetable = ensTableuintNewLen(0);

    gvdba = ensGvdatabaseadaptorGetDatabaseadaptor(gvv->Adaptor);

    csdba = ensRegistryGetDatabaseadaptor(
        ensEDatabaseadaptorGroupCore,
        ensDatabaseadaptorGetSpecies(gvdba));

    slicea = ensRegistryGetSliceadaptor(csdba);

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

    while(ajListPop(gvvfs, (void**) &gvvf))
    {
        /* Fetch an expanded Feature Slice. */

        vfeature = ensGvvariationfeatureGetFeature(gvvf);

        ensSliceadaptorFetchByFeature(slicea, vfeature, flank, &slice);

        /* Fetch all Ensembl Gene objects on the new Ensembl Slice. */

        ensGeneadaptorFetchAllbySlice(genea,
                                      newslice,
                                      (AjPStr) NULL,
                                      (AjPStr) NULL,
                                      (AjPStr) NULL,
                                      ajFalse,
                                      genelist);

        while(ajListPop(genelist, (void**) &gene))
        {
            if((ensFeatureGetSeqregionStart(vfeature)
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

                if(ajTableFetchmodV(genetable,
                                    (const void*) &identifier) == NULL)
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = ensGeneGetIdentifier(gene);

                    ajTablePut(genetable,
                               (void*) Pidentifier,
                               (void*) ensGeneNewRef(gene));
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

    ajTableMapDel(genetable, gvvariationClearGenes, genes);

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
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Source
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!gvv)
        return ajFalse;

    if(!gvss)
        return ajFalse;

    iter1 = ajListIterNew(gvv->Gvsynonyms);
    iter2 = ajListIterNew(gvss);

    while(!ajListIterDone(iter1))
    {
        gvs1 = (EnsPGvsource) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        found = ajFalse;

        while(!ajListIterDone(iter2))
        {
            gvs2 = (EnsPGvsource) ajListIterGet(iter2);

            if(ensGvsourceMatch(gvs1, gvs2))
            {
                found = ajTrue;

                break;
            }
        }

        if(!found)
            ajListPushAppend(gvss, (void*) ensGvsourceNewRef(gvs1));
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
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Synonym
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGvsynonyms(const EnsPGvvariation gvv,
                                        const AjPStr gvsourcename,
                                        AjPList gvss)
{
    AjIList iter = NULL;

    EnsPGvsource gvsource = NULL;

    EnsPGvsynonym gvsynonym = NULL;

    if(!gvv)
        return ajFalse;

    if(!gvss)
        return ajFalse;

    iter = ajListIterNew(gvv->Gvsynonyms);

    while(!ajListIterDone(iter))
    {
        gvsynonym = (EnsPGvsynonym) ajListIterGet(iter);

        if(gvsourcename && ajStrGetLen(gvsourcename))
        {
            gvsource = ensGvsynonymGetGvsource(gvsynonym);

            if(ajStrMatchS(ensGvsourceGetName(gvsource), gvsourcename))
                ajListPushAppend(gvss, (void*) ensGvsynonymNewRef(gvsynonym));
        }
        else
            ajListPushAppend(gvss, (void*) ensGvsynonymNewRef(gvsynonym));
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
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllGvvariationfeatures(EnsPGvvariation gvv,
                                                 AjPList gvvfs)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    if(!gvv)
        return ajFalse;

    if(!gvvfs)
        return ajFalse;

    if(!gvv->Adaptor)
    {
        ajDebug("ensGvvariationFetchAllGvvariationfeatures got an "
                "Ensembl Genetic Variation Variation without an "
                "Ensembl Genetic Variation Variation Adaptor attached.\n");

        return ajTrue;
    }

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvv->Adaptor);

    gvvfa = ensRegistryGetGvvariationfeatureadaptor(dba);

    return ensGvvariationfeatureadaptorFetchAllbyGvvariation(gvvfa,
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
** @nam3rule Has Check whether an Ensembl Genetic Variation Variation has
** a certain property
** @nam4rule Failedgvalleles Check for failed Ensembl Genetic Variation Allele
** objects
** @nam3rule Is Check whether an Ensembl Genetic Variation Variation represents
** a certain property
** @nam4rule Failed Check whether an
** Ensembl Genetic Variation Variation has been failed
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
** @param [u] Presult [AjBool*] ajTrue, if at least one
** Ensembl Genetic Variation Allele has been failed
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationHasFailedgvalleles(EnsPGvvariation gvv, AjBool* Presult)
{
    AjBool result = AJFALSE;

    AjIList iter = NULL;

    EnsPGvallele gva = NULL;

    if(!gvv)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    *Presult = ajFalse;

    if(!gvv->Gvalleles)
        return ajTrue;

    iter = ajListIterNewread(gvv->Gvalleles);

    while(!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        ensGvalleleIsFailed(gva, &result);

        if(result == ajTrue)
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
** @param [u] Presult [AjBool*] ajTrue, if an
** Ensembl Genetic Variation Variation has been failed
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationIsFailed(EnsPGvvariation gvv, AjBool* Presult)
{
    const AjPList descriptions = NULL;

    if(!gvv)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    descriptions = ensGvvariationLoadAllFaileddescriptions(gvv);

    if(ajListGetLength(descriptions) > 0)
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
** @valrule * [EnsEGvvariationClass] Ensembl Genetic Variation
** Variation Class enumeration or ensEGvvariationClassNULL
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
** @return [EnsEGvvariationClass] Ensembl Genetic Variation
** Variation Class enumeration or ensEGvvariationClassNULL
** @@
******************************************************************************/

EnsEGvvariationClass ensGvvariationClassFromStr(
    const AjPStr class)
{
    register EnsEGvvariationClass i =
        ensEGvvariationClassNULL;

    EnsEGvvariationClass gvvc =
        ensEGvvariationClassNULL;

    for(i = ensEGvvariationClassNULL;
        gvvariationClass[i];
        i++)
        if(ajStrMatchC(class, gvvariationClass[i]))
            gvvc = i;

    if(!gvvc)
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
** @argrule To gvvc [EnsEGvvariationClass] Ensembl Genetic Variation
** Variation Class enumeration
**
** @valrule Char [const char*] Ensembl Genetic Variation Variation Class
** C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvvariationClassToChar ********************************************
**
** Convert an Ensembl Genetic Variation Variation Class enumeration
** into a C-type (char*) string.
**
** @param [u] gvvc [EnsEGvvariationClass] Ensembl Genetic Variation
** Variation Class enumeration
**
** @return [const char*] Ensembl Genetic Variation Variation Class
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvvariationClassToChar(
    EnsEGvvariationClass gvvc)
{
    register EnsEGvvariationClass i =
        ensEGvvariationClassNULL;

    for(i = ensEGvvariationClassNULL;
        gvvariationClass[i] && (i < gvvc);
        i++);

    if(!gvvariationClass[i])
        ajDebug("ensGvvariationClassToChar encountered an "
                "out of boundary error on Ensembl "
                "Genetic Variation Variation Class enumeration %d.\n",
                gvvc);

    return gvvariationClass[i];
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
** @valrule * [EnsEGvvariationValidation] Ensembl Genetic Variation
** Variation Validation enumeration or ensEGvvariationValidationNULL
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
** @return [EnsEGvvariationValidation] Ensembl Genetic Variation
** Variation Validation enumeration or ensEGvvariationValidationNULL
** @@
******************************************************************************/

EnsEGvvariationValidation ensGvvariationValidationFromStr(
    const AjPStr validation)
{
    register EnsEGvvariationValidation i =
        ensEGvvariationValidationNULL;

    EnsEGvvariationValidation gvvv =
        ensEGvvariationValidationNULL;

    for(i = ensEGvvariationValidationNULL;
        gvvariationValidation[i];
        i++)
        if(ajStrMatchC(validation, gvvariationValidation[i]))
            gvvv = i;

    if(!gvvv)
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
** @argrule To gvvv [EnsEGvvariationValidation] Ensembl Genetic Variation
** Variation Validation enumeration
**
** @valrule Char [const char*] Ensembl Genetic Variation Variation Validation
** C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvvariationValidationToChar ***************************************
**
** Convert an Ensembl Genetic Variation Variation Validation enumeration
** into a C-type (char*) string.
**
** @param [u] gvvv [EnsEGvvariationValidation] Ensembl Genetic Variation
** Variation Validation enumeration
**
** @return [const char*] Ensembl Genetic Variation Variation Validation
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvvariationValidationToChar(
    EnsEGvvariationValidation gvvv)
{
    register EnsEGvvariationValidation i =
        ensEGvvariationValidationNULL;

    for(i = ensEGvvariationValidationNULL;
        gvvariationValidation[i] && (i < gvvv);
        i++);

    if(!gvvariationValidation[i])
        ajDebug("ensGvvariationValidationToChar encountered an "
                "out of boundary error on Ensembl "
                "Genetic Variation Variation Validation enumeration %d.\n",
                gvvv);

    return gvvariationValidation[i];
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
** @valrule * [ajuint] Ensembl Genetic Variation Variation Validation bit field
** or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationValidationsFromSet *************************************
**
** Convert an AJAX String representing a comma-separared SQL set of
** validation states into an Ensembl Genetic Variation Variation Validation
** (AJAX unsigned integer) bit field.
**
** @param [r] gvvvset [const AjPStr] SQL set
**
** @return [ajuint] Ensembl Genetic Variation Variation Validation bit field
** or 0
** @@
******************************************************************************/

ajuint ensGvvariationValidationsFromSet(const AjPStr gvvvset)
{
    ajuint gvvvbf = 0;

    AjPStr gvvvstring = NULL;

    AjPStrTok token = NULL;

    if(!gvvvset)
        return 0;

    /* Split the comma-separated list of validation states. */

    token = ajStrTokenNewC(gvvvset, ",");

    gvvvstring = ajStrNew();

    while(ajStrTokenNextParse(&token, &gvvvstring))
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
** @argrule To gvvvbf [ajuint] Ensembl Genetic Variation Variation Validation
** bit field
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
** @param [r] gvvvbf [ajuint] Ensembl Genetic Variation Variation Validation
** bit field
** @param [w] Pgvvvset [AjPStr*] SQL set
**
** @return [AjBool] True on success, false on failure
** @@
******************************************************************************/

AjBool ensGvvariationValidationsToSet(ajuint gvvvbf, AjPStr* Pgvvvset)
{
    register ajuint i = 0;

    if(*Pgvvvset)
        ajStrAssignClear(Pgvvvset);
    else
        *Pgvvvset = ajStrNew();

    for(i = 1; gvvariationValidation[i]; i++)
        if(gvvvbf & (1 << i))
        {
            ajStrAppendC(Pgvvvset, gvvariationValidation[i]);
            ajStrAppendC(Pgvvvset, ",");
        }

    /* Remove the last comma if one exists. */

    if(ajStrGetLen(*Pgvvvset) > 0)
        ajStrCutEnd(Pgvvvset, 1);

    return ajTrue;
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
** @nam3rule Gvvariation AJAX Table of AJAX unsigned integer key data and
**                       Ensembl Genetic Variation Variation value data
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




/* @funcstatic tableGvvariationClear ******************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Variation value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Genetic Variation Variation address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableGvvariationClear(void** key,
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

    ensGvvariationDel((EnsPGvvariation*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableGvvariationClear *********************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Variation value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableGvvariationClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableGvvariationClear, NULL);

    return ajTrue;
}




/* @func ensTableGvvariationDelete ********************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Genetic Variation Variation value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableGvvariationDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableGvvariationClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
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
** @cc CVS Revision: 1.78
** @cc CVS Tag: branch-ensembl-62
**
** NOTE: Since the "source" SQL table contains only a small number of records
** linked to a potentially large number of records in other SQL tables, the
** contents have been modelled as independent objects. The EnsPGvsourceadaptor
** caches all EnsPGvsource entries, which should help reduce memory
** requirements. Consequently, neither the "variation" nor "variation_synonym"
** SQL tables need joining to the "source" table.
******************************************************************************/




/* @funcstatic gvvariationadaptorFetchAllbyStatement **************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::_objs_from_sth
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation
**                           Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvvariationadaptorFetchAllbyStatement(
    EnsPGvvariationadaptor gvva,
    const AjPStr statement,
    AjPList gvvs)
{
    float gvafrequency = 0.0F;

    ajuint gvvidentifier     = 0;
    ajuint gvvidentifierold  = 0;
    ajuint gvasubidentifier  = 0;
    ajuint gvvattributeid    = 0;
    ajuint gvvsourceid       = 0;
    ajuint gvaidentifier     = 0;
    ajuint gvaidentifierold  = 0;
    ajuint gvacount          = 0;
    ajuint gvagvsampleid     = 0;
    ajuint gvvsidentifier    = 0;
    ajuint gvvssubidentifier = 0;
    ajuint gvvssourceid      = 0;

    ajuint* Pidentifier = NULL;

    AjBool gvvflank   = AJFALSE;
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

    AjPTable gvpopulations = NULL;
    AjPTable gvsynonyms    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvallele        gvallele  = NULL;
    EnsPGvalleleadaptor gvallelea = NULL;

    EnsPGvattribute        gvattribute  = NULL;
    EnsPGvattributeadaptor gvattributea = NULL;

    EnsPGvpopulation        gvp  = NULL;
    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvvariation gvv = NULL;

    EnsPGvsource        gvsource  = NULL;
    EnsPGvsourceadaptor gvsourcea = NULL;

    EnsPGvsynonym        gvsynonym  = NULL;
    EnsPGvsynonymadaptor gvsynonyma = NULL;

    if(!gvva)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    gvpopulations = ensTableuintNewLen(0);
    gvsynonyms    = ensTableuintNewLen(0);

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

    gvallelea    = ensRegistryGetGvalleleadaptor(dba);
    gvattributea = ensRegistryGetGvattributeadaptor(dba);
    gvpa         = ensRegistryGetGvpopulationadaptor(dba);
    gvsourcea    = ensRegistryGetGvsourceadaptor(dba);
    gvsynonyma   = ensRegistryGetGvsynonymadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        /* "variation" SQL table */
        gvvidentifier     = 0;
        gvvsourceid       = 0;
        gvvname           = ajStrNew();
        gvvvalidation     = ajStrNew();
        gvvancestral      = ajStrNew();
        gvvattributeid    = 0;
        gvvsomatic        = ajFalse;
        /* "allele" SQL table */
        gvaidentifier     = 0;
        gvasubidentifier  = 0;
        gvaallele         = ajStrNew();
        gvafrequency      = 0.0F;
        gvacount          = 0;
        gvagvsampleid     = 0;
        /* "variation_synonym" SQL table */
        gvvsidentifier    = 0;
        gvvssubidentifier = 0;
        gvvsourceid       = 0;
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
        ajSqlcolumnToUint(sqlr, &gvvattributeid);
        ajSqlcolumnToBool(sqlr, &gvvsomatic);
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

        if(gvvidentifier != gvvidentifierold)
        {
            ensGvattributeadaptorFetchByIdentifier(gvattributea,
                                                   gvvattributeid,
                                                   &gvattribute);

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
                                       gvattribute,
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
                                       gvvsomatic);

            ajListPushAppend(gvvs, (void*) gvv);

            ensGvattributeDel(&gvattribute);

            ensGvsourceDel(&gvsource);

            gvvidentifierold = gvvidentifier;
        }

        if(gvaidentifier != gvaidentifierold)
        {
            /* Fetch the Ensembl Genetic Variation Population. */

            if(gvagvsampleid)
            {
                gvp = (EnsPGvpopulation) ajTableFetchmodV(
                    gvpopulations,
                    (const void*) &gvagvsampleid);

                if(!gvp)
                {
                    ensGvpopulationadaptorFetchByIdentifier(gvpa,
                                                            gvagvsampleid,
                                                            &gvp);

                    if(gvp)
                    {
                        AJNEW0(Pidentifier);

                        *Pidentifier = ensGvpopulationGetIdentifier(gvp);

                        ajTablePut(gvpopulations,
                                   (void*) Pidentifier,
                                   (void*) gvp);
                    }
                }
            }
            else
                gvp = (EnsPGvpopulation) NULL;

            if(gvaidentifier)
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
                gvallele = (EnsPGvallele) NULL;
        }

        if(gvvsidentifier)
        {
            gvsynonym = (EnsPGvsynonym) ajTableFetchmodV(
                gvsynonyms,
                (const void*) &gvvsidentifier);

            if(!gvsynonym)
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

                if(gvsynonym)
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = gvvsidentifier;

                    ajTablePut(gvsynonyms,
                               (void*) Pidentifier,
                               (void*) gvsynonym);
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

    ensTableGvpopulationDelete(&gvpopulations);
    ensTableGvsynonymDelete(&gvsynonyms);

    return ajTrue;
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
** @nam4rule Allby Fetch all Ensembl Genetic Variation Variation objects
**                      matching a criterion
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
** @nam4rule By             Fetch one Ensembl Genetic Variation Variation
**                          object matching a criterion
** @nam5rule Identifier     Fetch by an SQL database-internal identifier
** @nam5rule Name           Fetch by a name
** @nam5rule Subidentifier  Fetch by a Sub-SNP identifier
**
** @argrule * gvva [EnsPGvvariationadaptor]
** Ensembl Genetic Variation Variation Adaptor
** @argrule All gvss [AjPList] AJAX List of
** Ensembl Genetic Variation Source objects
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
** @argrule By Pgvv [EnsPGvvariation*]
** Ensembl Genetic Variation Variation address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




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
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation Variation
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyGvpopulation(
    EnsPGvvariationadaptor gvva,
    const EnsPGvpopulation gvp,
    AjPList gvvs)
{
    AjPStr constraint = NULL;
    AjPStr statement  = NULL;

    if(!gvva)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    */

    ensGvdatabaseadaptorFailedvariationsconstraint(gvva,
                                                   (const AjPStr) NULL,
                                                   &constraint);

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
        "(variation, allele) "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "WHERE "
        "variation.variation_id = allele.variation_id "
        "AND "
        "allele.sample_id = %u "
        "AND %S",
        ensGvpopulationGetIdentifier(gvp),
        constraint);

    gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

    ajStrDel(&statement);
    ajStrDel(&constraint);

    return ajTrue;
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
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [r] frequency [float] Lower minor allele frequency (MAF) threshold
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation Variation
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyGvpopulationfrequency(
    EnsPGvvariationadaptor gvva,
    const EnsPGvpopulation gvp,
    float frequency,
    AjPList gvvs)
{
    AjPStr statement = NULL;

    if(!gvva)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    /* Adjust the frequency if given a percentage. */

    if(frequency > 1.0F)
        frequency /= 100.0F;

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
        "(variation, allele) "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "WHERE "
        "variation.variation_id = allele.variation_id "
        "AND "
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

    gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic listGvvariationCompareIdentifier *******************************
**
** AJAX List comparison function to sort Ensembl Genetic Variation Variation
** objects by identifier in ascending order.
**
** @param [r] P1 [const void*] Ensembl Genetic Variation Variation 1
** @param [r] P2 [const void*] Ensembl Genetic Variation Variation 2
** @see ajListSort
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listGvvariationCompareIdentifier(const void* P1, const void* P2)
{
    int result = 0;

    const EnsPGvvariation gvv1 = NULL;
    const EnsPGvvariation gvv2 = NULL;

    gvv1 = *(EnsPGvvariation const*) P1;
    gvv2 = *(EnsPGvvariation const*) P2;

    if(ajDebugTest("gvvariationlistCompare"))
        ajDebug("gvvariationlistCompare\n"
                "  gvv1 %p\n"
                "  gvv2 %p\n",
                gvv1,
                gvv2);

    if(gvv1->Identifier < gvv2->Identifier)
        result = -1;

    if(gvv1->Identifier > gvv2->Identifier)
        result = +1;

    return result;
}




/* @funcstatic listGvvariationDelete ******************************************
**
** ajListSortUnique nodedelete function to delete Ensembl Genetic Variation
** Variation objects that are redundant.
**
** @param [r] PP1 [void**] Ensembl Genetic Variation Variation address 1
** @param [r] cl [void*] Standard, passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
** @@
******************************************************************************/

static void listGvvariationDelete(void** PP1, void* cl)
{
    if(!PP1)
        return;

    (void) cl;

    ensGvvariationDel((EnsPGvvariation*) PP1);

    return;
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

    if(!gvva)
        return ajFalse;

    if(!gvst)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

    gvsa = ensRegistryGetGvsourceadaptor(dba);

    gvss = ajListNew();

    ensGvsourceadaptorFetchAllbyGvsourceType(gvsa, gvst, gvss);

    while(ajListPop(gvss, (void*) &gvs))
    {
        ensGvvariationadaptorFetchAllbySource(
            gvva,
            ensGvsourceGetName(gvs),
            primary,
            gvvs);

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

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
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [u] gvvset [EnsPGvvariationset] Ensembl Genetic Variation
**                                        Variation Set
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation Variation
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!gvva)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    if(!gvvset->Adaptor)
        return ajFalse;

    if(!gvvset->Identifier)
        return ajFalse;

    variationsets = ajListNew();

    /*
    ** First, get all immediate subsets of the specified Ensembl Genetic
    ** Variation Variation Set and get their Ensembl Genetic Variation
    ** Variation objects.
    */

    ensGvvariationsetFetchAllSub(gvvset, ajTrue, variationsets);

    while(ajListPop(variationsets, (void**) &variationset))
    {
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

    gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

    ajStrDel(&statement);

    ajListSortUnique(gvvs,
                     listGvvariationCompareIdentifier,
                     listGvvariationDelete);

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
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [u] gvvs [AjPTable] AJAX Table of AJAX unsigned integer (identifier)
**                            key data and Ensembl Genetic Variation
**                            Variation value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyIdentifiers(
    EnsPGvvariationadaptor gvva,
    AjPTable gvvs)
{
    void** keyarray = NULL;

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    */

    const char* template =
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
        "variation.variation_id IN (%S)";

    register ajuint i = 0;

    ajuint identifier = 0;

    ajuint* Pidentifier = NULL;

    AjPList lgvvs = NULL;

    AjPStr statement = NULL;
    AjPStr csv       = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    lgvvs = ajListNew();

    csv = ajStrNew();

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifiers configured in the Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    ajTableToarrayKeys(gvvs, &keyarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint*) keyarray[i]));

        /* Run the statement if the maximum chunk size is exceed. */

        if(((i + 1) % ensBaseadaptorMaximumIdentifiers) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            statement = ajFmtStr(template, csv);

            gvvariationadaptorFetchAllbyStatement(gvva, statement, lgvvs);

            ajStrDel(&statement);

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
    {
        statement = ajFmtStr(template, csv);

        gvvariationadaptorFetchAllbyStatement(gvva, statement, lgvvs);

        ajStrDel(&statement);
    }

    ajStrDel(&csv);

    /*
    ** Move Ensembl Genetic Variation Variation objects from the AJAX List
    ** to the AJAX Table.
    */

    while(ajListPop(lgvvs, (void**) &gvv))
    {
        identifier = ensGvvariationGetIdentifier(gvv);

        if(ajTableMatchV(gvvs, (const void*) &identifier))
            ajTablePut(gvvs, (void*) &identifier, (void*) gvv);
        else
        {
            /*
            ** This should not happen, because the keys should have been in
            ** the AJAX Table in the first place.
            */

            AJNEW0(Pidentifier);

            *Pidentifier = ensGvvariationGetIdentifier(gvv);

            ajTablePut(gvvs, (void*) Pidentifier, (void*) gvv);
        }
    }

    ajListFree(&lgvvs);

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
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                                Variation Adaptor
** @param [u] gvvs [AjPTable] AJAX Table of
** AJAX String (name) key data and
** Ensembl Genetic Variation Variation value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbyNames(
    EnsPGvvariationadaptor gvva,
    AjPTable gvvs)
{
    void** keyarray = NULL;

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    */

    const char* template1 =
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

    const char* template2 =
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

    register ajuint i = 0;
    register ajuint j = 0;

    AjPList lgvvs = NULL;

    AjPStr name      = NULL;
    AjPStr statement = NULL;
    AjPStr csv       = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    lgvvs = ajListNew();

    csv = ajStrNew();

    /*
    ** MySQL is faster and we ensure that we do not exceed the maximum
    ** query size by splitting large queries into smaller queries of
    ** up to 200 names.
    */

    ajTableToarrayKeys(gvvs, &keyarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajFmtPrintAppS(&csv, "'%S', ", (AjPStr) keyarray[i]);

        /*
        ** Run the statement if we exceed the maximum chunk size.
        ** Assume that average name strings are 4 times longer than
        ** integer strings.
        */

        if(((i + 1) % (ensBaseadaptorMaximumIdentifiers / 4)) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            statement = ajFmtStr(template1, csv);

            gvvariationadaptorFetchAllbyStatement(gvva, statement, lgvvs);

            ajStrDel(&statement);

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
    {
        statement = ajFmtStr(template1, csv);

        gvvariationadaptorFetchAllbyStatement(gvva, statement, lgvvs);

        ajStrDel(&statement);
    }

    /*
    ** Move Ensembl Genetic Variation Variation objects from the AJAX List
    ** to the AJAX Table.
    */

    while(ajListPop(lgvvs, (void**) &gvv))
    {
        name = ensGvvariationGetName(gvv);

        if(ajTableMatchV(gvvs, (const void*) name))
            ajTablePut(gvvs, (void*) name, (void*) gvv);
        else
        {
            /*
            ** This should not happen, because the keys should have been in
            ** the AJAX Table in the first place.
            */

            ajTablePut(gvvs,
                       (void*) ajStrNewS(ensGvvariationGetName(gvv)),
                       (void*) gvv);
        }
    }

    /* Check, which names yielded no result abobve and search for synonyms. */

    ajTableToarrayKeys(gvvs, &keyarray);

    for(i = 0; keyarray[i]; i++)
    {
        gvv = (EnsPGvvariation) ajTableFetchmodV(gvvs,
                                                 (const void*) keyarray[i]);

        if(gvv == NULL)
        {
            ajFmtPrintAppS(&csv, "'%S', ", (AjPStr) keyarray[i]);

            j++;
        }

        /*
        ** Run the statement if we exceed the maximum chunk size.
        ** Assume that average name strings are 4 times longer than
        ** integer strings.
        */

        if(((j + 1) % (ensBaseadaptorMaximumIdentifiers / 4)) == 0)
        {
            /* Remove the last comma and space. */

            ajStrCutEnd(&csv, 2);

            statement = ajFmtStr(template2, csv);

            gvvariationadaptorFetchAllbyStatement(gvva, statement, lgvvs);

            ajStrDel(&statement);

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    /* Run the final statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    if(ajStrGetLen(csv))
    {
        statement = ajFmtStr(template2, csv);

        gvvariationadaptorFetchAllbyStatement(gvva, statement, lgvvs);

        ajStrDel(&statement);
    }

    /*
    ** Move Ensembl Genetic Variation Variation objects from the AJAX List
    ** to the AJAX Table based on a synonym.
    */

    while(ajListPop(lgvvs, (void**) &gvv))
    {
        name = ensGvvariationGetName(gvv);

        if(ajTableMatchV(gvvs, (const void*) name))
            ajTablePut(gvvs, (void*) name, (void*) gvv);
        else
        {
            /*
            ** This should not happen, because the keys should have been in
            ** the AJAX Table in the first place.
            */

            ajTablePut(gvvs,
                       (void*) ajStrNewS(ensGvvariationGetName(gvv)),
                       (void*) gvv);
        }
    }

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
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] source [const AjPStr] Source
** @param [r] primary [AjBool] Primary
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation Variation
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllbySource(
    EnsPGvvariationadaptor gvva,
    const AjPStr source,
    AjBool primary,
    AjPList gvvs)
{
    char* txtsource = NULL;

    AjPStr constraint = NULL;
    AjPStr statement  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvva)
        return ajFalse;

    if(!source)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

    ensDatabaseadaptorEscapeC(dba, &txtsource, source);

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    */

    ensGvdatabaseadaptorFailedvariationsconstraint(gvva,
                                                   (const AjPStr) NULL,
                                                   &constraint);

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
        "(variation, source) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = "
        "allele.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "WHERE "
        "variation.source_id = "
        "source.source_id "
        "AND "
        "source.name = '%s' "
        "AND %S",
        txtsource,
        constraint);

    gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

    ajStrDel(&statement);

    if(!primary)
    {
        /*
        ** We need to include variation_synonym as well,
        ** where the variation was merged with NCBI dbSNP.
        ** NOTE: Values are extracted from variation_synonym1,
        ** while variation_synonym2 is included in the condition.
        */

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
            "variation_synonym1.variation_synonym_id, "
            "variation_synonym1.subsnp_id, "
            "variation_synonym1.source_id, "
            "variation_synonym1.name, "
            "variation_synonym1.moltype, "
            "0 " /* "flanking_sequence" flag */
            "FROM "
            "("
            "variation, "
            "source, "
            "variation_synonym variation_synonym1"
            ") "
            "LEFT JOIN "
            "allele "
            "ON "
            "variation.variation_id = "
            "allele.variation_id "
            "WHERE "
            "variation.variation_id = "
            "variation_synonym1.variation_id "
            "AND "
            "variation_synonym1.source_id = "
            "source.source_id "
            "AND "
            "source.name = '%s' "
            "AND %S "
            "ORDER BY "
            "variation.variation_id",
            txtsource,
            constraint);

        /* need to merge both lists, trying to avoid duplicates. */

        gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

        ajStrDel(&statement);
    }

    ajCharDel(&txtsource);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Genetic Variation Variation via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_dbID
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation
**                                     Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchByIdentifier(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    EnsPGvvariation* Pgvv)
{
    AjPList variations = NULL;

    AjPStr statement = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvv)
        return ajFalse;

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    ** NOTE: The "flanking_sequence" SQL table is joined here and in
    ** ensGvvariationadaptorFetchByName so that the flank_flag can be
    ** set where the flanking sequence differs from the reference. It is just
    ** set to 0 when fetching Ensembl Genetic Variation Variation objects by
    ** other methods since otherwise the join takes too long.
    */

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
        "("
        "flanking_sequence.up_seq IS NOT NULL "
        "OR "
        "flanking_sequence.down_seq IS NOT NULL"
        ") "
        "FROM "
        "(variation) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = allele.variation_id "
        "LEFT JOIN "
        "flanking_sequence "
        "ON "
        "variation.variation_id = flanking_sequence.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "WHERE "
        "variation.variation_id = %u",
        identifier);

    variations = ajListNew();

    gvvariationadaptorFetchAllbyStatement(gvva, statement, variations);

    if(ajListGetLength(variations) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier go more than one "
               "Ensembl Genetic Variation Variation for identifier %u.",
               identifier);

    ajListPop(variations, (void**) Pgvv);

    while(ajListPop(variations, (void**) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&variations);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchByName *************************************
**
** Fetch an Ensembl Genetic Variation Variation via its name.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_name
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] source [const AjPStr] Source
** @param [wP] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation
**                                     Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchByName(
    EnsPGvvariationadaptor gvva,
    const AjPStr name,
    const AjPStr source,
    EnsPGvvariation* Pgvv)
{
    char* txtname   = NULL;
    char* txtsource = NULL;

    AjPList gvvs = NULL;

    AjPStr extracondition = NULL;
    AjPStr extrafrom      = NULL;
    AjPStr statement      = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Pgvv)
        return ajFalse;

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

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

    ensDatabaseadaptorEscapeC(dba, &txtname, name);

    if(source && ajStrGetLen(source))
    {
        extrafrom = ajStrNewC(", source");

        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        extracondition = ajFmtStr(" AND variation.source_id = source.source_id"
                                  " AND source.name = '%s'", txtsource);

        ajCharDel(&txtsource);
    }
    else
    {
        extrafrom = ajStrNew();
        extracondition = ajStrNew();
    }

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
        "("
        "flanking_sequence.up_seq IS NOT NULL "
        "OR "
        "flanking_sequence.down_seq IS NOT NULL"
        ") "
        "FROM "
        "(variation%S) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = allele.variation_id "
        "LEFT JOIN "
        "flanking_sequence "
        "ON "
        "variation.variation_id = flanking_sequence.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = failed_variation.variation_id "
        "LEFT JOIN "
        "WHERE "
        "variation.name = '%s'%S "
        "ORDER BY "
        "allele.allele_id",
        extrafrom,
        txtname,
        extracondition);

    gvvs = ajListNew();

    gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

    ajStrDel(&statement);

    if(ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier go more than one "
               "Ensembl Genetic Variation Variation for name '%S' and "
               "source '%S'.", name, source);

    if(ajListGetLength(gvvs) == 0)
    {
        /*
        ** NOTE: Values are extracted from variation_synonym1,
        ** while variation_synonym2 is included in the condition.
        */

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
            "variation_synonym1.variation_synonym_id, "
            "variation_synonym1.subsnp_id, "
            "variation_synonym1.source_id, "
            "variation_synonym1.name, "
            "variation_synonym1.moltype, "
            "("
            "flanking_sequence.up_seq IS NOT NULL "
            "OR "
            "flanking_sequence.down_seq IS NOT NULL"
            ") "
            "FROM "
            "("
            "variation, "
            "allele, "
            "variation_synonym variation_synonym1, "
            "variation_synonym variation_synonym2, "
            "flanking_sequence"
            "%S) "
            "WHERE "
            "variation.variation_id = allele.variation_id "
            "AND "
            "variation.variation_id = "
            "variation_synonym1.variation_id "
            "AND "
            "variation.variation_id = "
            "variation_synonym2.variation_id "
            "AND "
            "variation.variation_id = flanking_sequence.variation_id "
            "AND "
            "variation_synonym2.name = '%s'%S "
            "ORDER BY "
            "allele.allele_id",
            extrafrom,
            txtname,
            extracondition);

        /*
        ** FIXME: This second query does not use a LEFT JOIN for
        ** the "flanking_sequence" SQL table.
        ** Report this to the Variation team.
        */

        gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

        ajStrDel(&statement);
    }

    if(ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier go more than one "
               "Ensembl Genetic Variation Variation for name '%S' and "
               "source '%S'.", name, source);

    ajListPop(gvvs, (void**) Pgvv);

    while(ajListPop(gvvs, (void**) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&gvvs);

    ajCharDel(&txtname);

    ajStrDel(&extrafrom);
    ajStrDel(&extracondition);

    /*
    ** If the query yielded no result and the name looks like a
    ** Sub-SNP identifier, query again.
    */

    if((*Pgvv == NULL) && (ajStrPrefixC(name, "ss") == ajTrue))
        return ensGvvariationadaptorFetchBySubidentifier(gvva, name, Pgvv);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchBySubidentifier ****************************
**
** Fetch an Ensembl Genetic Variation Variation via its sub-identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_subsnp_id
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] name [const AjPStr] Name
** @param [wP] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation
**                                     Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchBySubidentifier(
    EnsPGvvariationadaptor gvva,
    const AjPStr name,
    EnsPGvvariation* Pgvv)
{
    char* txtname   = NULL;

    AjPList gvvs = NULL;

    AjPStr subidentifier = NULL;
    AjPStr statement     = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Pgvv)
        return ajFalse;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

    if(ajStrPrefixC(name, "ss"))
    {
        subidentifier = ajStrNew();
        ajStrAssignSubS(&subidentifier, name, 0, 2);
        ensDatabaseadaptorEscapeC(dba, &txtname, subidentifier);
        ajStrDel(&subidentifier);
    }
    else
        ensDatabaseadaptorEscapeC(dba, &txtname, name);

    /*
    ** NOTE: Because information from the "source" SQL table is
    ** modelled as independent EnsPGvsource object, neither
    ** "variation" nor "variation_synonym" need joining.
    ** NOTE: The "flanking_sequence" SQL table is joined here and in
    ** ensGvvariationadaptorFetchByIdentifier so that a flank_flag can be
    ** set where the flanking sequence differs from the reference. It is just
    ** set to 0 when fetching Ensembl Genetic Variation Variation objects by
    ** other methods since otherwise the join takes too long.
    */

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
        "("
        "flanking_sequence.up_seq IS NOT NULL "
        "OR "
        "flanking_sequence.down_seq IS NOT NULL"
        ")"
        "FROM "
        "(variation) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = "
        "allele.variation_id "
        "LEFT JOIN "
        "flanking_sequence "
        "ON "
        "variation.variation_id = "
        "flanking_sequence.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "LEFT JOIN "
        "WHERE "
        "allele.subsnp_id = '%s' "
        "ORDER BY "
        "allele.allele_id",
        txtname);

    ajCharDel(&txtname);

    gvvs = ajListNew();

    gvvariationadaptorFetchAllbyStatement(gvva, statement, gvvs);

    ajListPop(gvvs, (void**) Pgvv);

    while(ajListPop(gvvs, (void**) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&gvvs);

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




/* @funcstatic tableGvvariationadaptorFaileddescriptionsKeyDel ****************
**
** An ajTableNewFunctionLen keydel function to delete
** AJAX unsigned integer key data of an AJAX Table.
**
** @see ajTableNewFunctionLen
** @see ajTableClearDelete
** @param [u] key [void**] AJAX unsigned integer key data
**
** @return [void]
** @@
******************************************************************************/

static void tableGvvariationadaptorFaileddescriptionsKeyDel(void** key)
{
    if(!key)
        return;

    if(!*key)
        return;

    AJFREE(*key);

    *key = NULL;

    return;
}




/* @funcstatic tableGvvariationadaptorFaileddescriptions1ValueDel *************
**
** An ajTableNewFunctionLen valdel function to delete
** AJAX Table value data of an AJAX Table.
**
** @see ajTableNewFunctionLen
** @see ajTableClearDelete
** @param [u] value [void**] AJAX Table value data
**
** @return [void]
** @@
******************************************************************************/

static void tableGvvariationadaptorFaileddescriptions1ValueDel(void** value)
{
    if(!value)
        return;

    if(!*value)
        return;

    ajTableClearDelete((AjPTable) *value);

    ajTableFree((AjPTable*) value);

    *value = NULL;

    return;
}




/* @funcstatic tableGvvariationadaptorFaileddescriptions2ValueDel *************
**
** An ajTableNewFunctionLen valdel function to delete
** AJAX String value data of an AJAX Table.
**
** @see ajTableNewFunctionLen
** @see ajTableClearDelete
** @param [u] value [void**] AJAX String value data
**
** @return [void]
** @@
******************************************************************************/

static void tableGvvariationadaptorFaileddescriptions2ValueDel(void** value)
{
    if(!value)
        return;

    if(!*value)
        return;

    ajStrDel((AjPStr*) value);

    *value = NULL;

    return;
}




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
** @@
******************************************************************************/

static AjBool gvvariationadaptorRetrieveAllFaileddescriptions(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    const AjPStr constraint,
    AjPTable fvs)
{
    ajuint gvvidentifier = 0;
    ajuint subsnpid      = 0;

    ajuint* Pgvvidentifier = NULL;
    ajuint* Psubsnpid      = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr description = NULL;
    AjPStr statement   = NULL;

    AjPTable table = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvva)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!fvs)
        return ajFalse;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

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

    if((constraint != NULL) && (ajStrGetLen(constraint) > 0))
    {
        ajStrAppendC(&statement, " AND ");
        ajStrAppendS(&statement, constraint);
    }

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        gvvidentifier = 0;
        subsnpid      = 0;
        description   = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gvvidentifier);
        ajSqlcolumnToUint(sqlr, &subsnpid);
        ajSqlcolumnToStr(sqlr, &description);

        table = (AjPTable) ajTableFetchmodV(fvs, (const void*) &gvvidentifier);

        if(!table)
        {
            AJNEW0(Pgvvidentifier);
            *Pgvvidentifier = gvvidentifier;
            table = ajTableNewFunctionLen(
                0,
                ensTableuintCmp,
                ensTableuintHash,
                tableGvvariationadaptorFaileddescriptionsKeyDel,
                tableGvvariationadaptorFaileddescriptions2ValueDel);
            ajTablePut(fvs, (void*) Pgvvidentifier, (void*) table);
        }

        if(!ajTableMatchV(table, (const void*) &subsnpid))
        {
            AJNEW0(Psubsnpid);
            *Psubsnpid = subsnpid;
            ajTablePut(table, (void*) Psubsnpid, (void*) description);
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
** @@
******************************************************************************/

AjBool ensGvvariationadaptorRetrieveAllFaileddescriptions(
    EnsPGvvariationadaptor gvva,
    ajuint identifier,
    AjPList fvs)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    register ajuint i = 0;

    AjPTable fvstable1 = NULL;
    AjPTable fvstable2 = NULL;
    AjPTable strtable  = NULL;

    if(!gvva)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!fvs)
        return ajFalse;

    fvstable1 = ajTableNewFunctionLen(
        0,
        ensTableuintCmp,
        ensTableuintHash,
        tableGvvariationadaptorFaileddescriptionsKeyDel,
        tableGvvariationadaptorFaileddescriptions1ValueDel);

    gvvariationadaptorRetrieveAllFaileddescriptions(
        gvva,
        identifier,
        (AjPStr) NULL,
        fvstable1);

    fvstable2 = (AjPTable) ajTableFetchmodV(fvstable1,
                                            (const void*) &identifier);

    if(!fvstable2)
        return ajFalse;

    strtable = ajTablestrNew(0);

    ajTableToarrayValues(fvstable2, &valarray);

    for(i = 0; valarray[i]; i++)
        if(!ajTableMatchV(strtable, (const void*) valarray[i]))
            ajTablePut(strtable,
                       (void*) ajStrNewS((AjPStr) valarray[i]),
                       NULL);

    AJFREE(valarray);

    ajTableToarrayKeys(strtable, &keyarray);

    for(i = 0; keyarray[i]; i++)
        ajListPushAppend(fvs, (void*) keyarray[i]);

    ajTableClearDelete(fvstable1);

    ajTableFree(&fvstable1);
    ajTableFree(&strtable);

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
** @@
******************************************************************************/

static AjBool gvvariationadaptorRetrieveFlankFromCore(
    EnsPGvvariationadaptor gvva,
    ajuint srid,
    ajint srstart,
    ajint srend,
    ajint srstrand,
    AjPStr* Psequence)
{
    EnsPDatabaseadaptor dba = NULL;
    EnsPDatabaseadaptor rsa = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!gvva)
        return ajFalse;

    if(!srid)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

    rsa = ensRegistryGetReferenceadaptor(dba);

    sla = ensRegistryGetSliceadaptor(rsa);

    if(sla)
    {
        ensSliceadaptorFetchBySeqregionIdentifier(sla, srid, 0, 0, 0, &slice);

        if(!slice)
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
** @@
******************************************************************************/

AjBool ensGvvariationadaptorRetrieveFlank(EnsPGvvariationadaptor gvva,
                                          ajuint identifier,
                                          AjPStr* Pfiveseq,
                                          AjPStr* Pthreeseq)
{
    ajint srstrand = 0;

    ajuint srid     = 0;
    ajuint usrstart = 0;
    ajuint usrend   = 0;
    ajuint dsrstart = 0;
    ajuint dsrend   = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr useq      = NULL;
    AjPStr dseq      = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvva)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pfiveseq)
        return ajFalse;

    if(!Pthreeseq)
        return ajFalse;

    dba = ensGvdatabaseadaptorGetDatabaseadaptor(gvva);

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

    while(!ajSqlrowiterDone(sqli))
    {
        srid     = 0;
        srstrand = 0;
        useq     = ajStrNew();
        dseq     = ajStrNew();
        usrstart = 0;
        usrend   = 0;
        dsrstart = 0;
        dsrend   = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToStr(sqlr, &useq);
        ajSqlcolumnToStr(sqlr, &dseq);
        ajSqlcolumnToUint(sqlr, &usrstart);
        ajSqlcolumnToUint(sqlr, &usrend);
        ajSqlcolumnToUint(sqlr, &dsrstart);
        ajSqlcolumnToUint(sqlr, &dsrend);

        if(!ajStrGetLen(useq))
        {
            if(srid)
                gvvariationadaptorRetrieveFlankFromCore(gvva,
                                                        srid,
                                                        usrstart,
                                                        usrend,
                                                        srstrand,
                                                        &useq);
            else
                ajWarn("ensGvvariationadaptorRetrieveFlank "
                       "could not get "
                       "Ensembl Sequence Region identifier for "
                       "Ensembl Genetic Variation Variation %u.",
                       identifier);
        }

        ajStrAssignS(Pfiveseq, useq);

        if(!ajStrGetLen(dseq))
        {
            if(srid)
                gvvariationadaptorRetrieveFlankFromCore(gvva,
                                                        srid,
                                                        dsrstart,
                                                        dsrend,
                                                        srstrand,
                                                        &dseq);
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
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewCpy(
    const EnsPGvvariationfeature gvvf)
{
    EnsPGvvariationfeature pthis = NULL;

    if(!gvvf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gvvf->Identifier;

    pthis->Adaptor = gvvf->Adaptor;

    pthis->Feature = ensFeatureNewRef(gvvf->Feature);

    pthis->Gvsource = ensGvsourceNewRef(gvvf->Gvsource);

    pthis->Gvvariation = ensGvvariationNewRef(gvvf->Gvvariation);

    if(gvvf->Allele)
        pthis->Allele = ajStrNewRef(gvvf->Allele);

    if(gvvf->Name)
        pthis->Name = ajStrNewRef(gvvf->Name);

    if(gvvf->Validationcode)
        pthis->Validationcode = ajStrNewRef(gvvf->Validationcode);

    if(gvvf->Consequencetype)
        pthis->Consequencetype = ajStrNewRef(gvvf->Consequencetype);

    pthis->Mapweight = gvvf->Mapweight;

    pthis->Gvvariationidentifier = gvvf->Gvvariationidentifier;

    pthis->GvvariationClass = gvvf->GvvariationClass;

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
** @return [EnsPGvvariationfeature] Ensembl Genetic Variation Variation Feature
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

    if(!feature)
        return NULL;

    if(!gvvidentifier)
        return NULL;

    AJNEW0(gvvf);

    gvvf->Use = 1;

    gvvf->Identifier = identifier;

    gvvf->Adaptor = gvvfa;

    gvvf->Feature = ensFeatureNewRef(feature);

    gvvf->Gvsource = ensGvsourceNewRef(gvsource);

    gvvf->Gvvariationidentifier = gvvidentifier;

    if(allele)
        gvvf->Allele = ajStrNewRef(allele);

    if(name)
        gvvf->Name = ajStrNewRef(name);

    if(validation)
        gvvf->Validationcode = ajStrNewRef(validation);

    if(consequence)
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

    if(!feature)
        return NULL;

    if(!gvv)
        return NULL;

    AJNEW0(gvvf);

    gvvf->Use = 1;

    gvvf->Identifier = identifier;

    gvvf->Adaptor = gvvfa;

    gvvf->Feature = ensFeatureNewRef(feature);

    gvvf->Gvsource = ensGvsourceNewRef(gvsource);

    gvvf->Gvvariation = ensGvvariationNewRef(gvv);

    if(allele)
        gvvf->Allele = ajStrNewRef(allele);

    if(name)
        gvvf->Name = ajStrNewRef(name);

    if(validation)
        gvvf->Validationcode = ajStrNewRef(validation);

    if(consequence)
        gvvf->Consequencetype = ajStrNewRef(consequence);

    gvvf->Mapweight = mapweight;

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
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewRef(
    EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    gvvf->Use++;

    return gvvf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Feature object
**
** @argrule * Pgvvf [EnsPGvvariationfeature*]
** Ensembl Genetic Variation Variation Feature object address
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
** Ensembl Genetic Variation Variation Feature object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationfeatureDel(EnsPGvvariationfeature* Pgvvf)
{
    EnsPGvvariationfeature pthis = NULL;

    if(!Pgvvf)
        return;

    if(!*Pgvvf)
        return;

    if(ajDebugTest("ensGvvariationfeatureDel"))
    {
        ajDebug("ensGvvariationfeatureDel\n"
                "  *Pgvvf %p\n",
                *Pgvvf);

        ensGvvariationfeatureTrace(*Pgvvf, 1);
    }

    pthis = *Pgvvf;

    pthis->Use--;

    if(pthis->Use)
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

    AJFREE(pthis);

    *Pgvvf = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
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
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Mapweight [ajuint] Map weight or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Validationcode [AjPStr] Validation code or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureGetAdaptor **************************************
**
** Get the Ensembl Genetic Variation Variation Feature Adaptor element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                         Variation Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationfeatureadaptor ensGvvariationfeatureGetAdaptor(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Adaptor;
}




/* @func ensGvvariationfeatureGetAllele ***************************************
**
** Get the allele element of an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Allele or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetAllele(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Allele;
}




/* @func ensGvvariationfeatureGetConsequencetype ******************************
**
** Get the consequence type element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Consequence type or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetConsequencetype(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Consequencetype;
}




/* @func ensGvvariationfeatureGetFeature **************************************
**
** Get the Ensembl Feature element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensGvvariationfeatureGetFeature(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Feature;
}




/* @func ensGvvariationfeatureGetGvsource *************************************
**
** Get the Ensembl Genetic Variation Source element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @@
******************************************************************************/

EnsPGvsource ensGvvariationfeatureGetGvsource(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Gvsource;
}




/* @func ensGvvariationfeatureGetGvvariationclass *****************************
**
** Get the Ensembl Genetic Variation Variation Class enumeration element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsEGvvariationClass] Ensembl Genetic Variation Variation Class
** enumeration or ensEGvvariationClassNULL
** @@
******************************************************************************/

EnsEGvvariationClass ensGvvariationfeatureGetGvvariationclass(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return ensEGvvariationClassNULL;

    return gvvf->GvvariationClass;
}




/* @func ensGvvariationfeatureGetIdentifier ***********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetIdentifier(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return 0;

    return gvvf->Identifier;
}




/* @func ensGvvariationfeatureGetMapweight ************************************
**
** Get the map weight element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] Map weight or 0
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetMapweight(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return 0;

    return gvvf->Mapweight;
}




/* @func ensGvvariationfeatureGetName *****************************************
**
** Get the name element of an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetName(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Name;
}




/* @func ensGvvariationfeatureGetValidationcode *******************************
**
** Get the validation code element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Validation code or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetValidationcode(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Validationcode;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Genetic Variation Variation
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
** Get the Ensembl Genetic Variation Variation element of an
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
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationfeatureLoadGvvariation(
    EnsPGvvariationfeature gvvf)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationadaptor gvva = NULL;

    if(!gvvf)
        return NULL;

    if(gvvf->Gvvariation)
        return gvvf->Gvvariation;

    if(gvvf->Adaptor)
    {
        dba = ensGvvariationfeatureadaptorGetDatabaseadaptor(gvvf->Adaptor);

        gvva = ensRegistryGetGvvariationadaptor(dba);

        ensGvvariationadaptorFetchByIdentifier(gvva,
                                               gvvf->Gvvariationidentifier,
                                               &gvvf->Gvvariation);
    }

    return gvvf->Gvvariation;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Set Set one element of a Genetic Variation Variation Feature
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
** Set the Ensembl Genetic Variation Variation Feature Adaptor element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetAdaptor(EnsPGvvariationfeature gvvf,
                                       EnsPGvvariationfeatureadaptor gvvfa)
{
    if(!gvvf)
        return ajFalse;

    gvvf->Adaptor = gvvfa;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetAllele ***************************************
**
** Set the allele element of an Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] allele [AjPStr] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetAllele(EnsPGvvariationfeature gvvf,
                                      AjPStr allele)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Allele);

    if(allele)
        gvvf->Allele = ajStrNewRef(allele);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetConsequencetype ******************************
**
** Set the consequence type element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] consequencetype [AjPStr] Consequence type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetConsequencetype(EnsPGvvariationfeature gvvf,
                                               AjPStr consequencetype)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Consequencetype);

    if(consequencetype)
        gvvf->Consequencetype = ajStrNewRef(consequencetype);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetFeature **************************************
**
** Set the Ensembl Feature element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetFeature(EnsPGvvariationfeature gvvf,
                                       EnsPFeature feature)
{
    if(!gvvf)
        return ajFalse;

    ensFeatureDel(&gvvf->Feature);

    gvvf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvsource *************************************
**
** Set the Ensembl Genetic Variation Source element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvsource(EnsPGvvariationfeature gvvf,
                                        EnsPGvsource gvsource)
{
    if(!gvvf)
        return ajFalse;

    ensGvsourceDel(&gvvf->Gvsource);

    gvvf->Gvsource = ensGvsourceNewRef(gvsource);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvvariation **********************************
**
** Set the Ensembl Genetic Variation Variation element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvvariation(EnsPGvvariationfeature gvvf,
                                           EnsPGvvariation gvv)
{
    if(!gvvf)
        return ajFalse;

    ensGvvariationDel(&gvvf->Gvvariation);

    gvvf->Gvvariation = ensGvvariationNewRef(gvv);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvvariationclass *****************************
**
** Set the Ensembl Genetic Variation Variation Class enumeration element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvvc [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvvariationclass(EnsPGvvariationfeature gvvf,
                                                EnsEGvvariationClass gvvc)
{
    if(!gvvf)
        return ajFalse;

    gvvf->GvvariationClass = gvvc;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetIdentifier ***********************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetIdentifier(EnsPGvvariationfeature gvvf,
                                          ajuint identifier)
{
    if(!gvvf)
        return ajFalse;

    gvvf->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetMapweight ************************************
**
** Set the map weight element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] mapweight [ajuint] Map weight
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetMapweight(EnsPGvvariationfeature gvvf,
                                         ajuint mapweight)
{
    if(!gvvf)
        return ajFalse;

    gvvf->Mapweight = mapweight;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetName *****************************************
**
** Set the name element of an Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetName(EnsPGvvariationfeature gvvf,
                                    AjPStr name)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Name);

    if(name)
        gvvf->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetValidationcode *******************************
**
** Set the validation code element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] validationcode [AjPStr] Validation code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetValidationcode(EnsPGvvariationfeature gvvf,
                                              AjPStr validationcode)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Validationcode);

    if(validationcode)
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
** @nam3rule Trace Report Ensembl Genetic Variation Variation Feature elements
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureTrace(const EnsPGvvariationfeature gvvf,
                                  ajuint level)
{
    AjPStr indent = NULL;

    if(!gvvf)
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
** Functions for calculating values of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Variation Feature
** values
** @nam4rule Length Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
** Variation Feature
**
** @valrule Length [ajuint] Length or 0
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
** @return [ajuint] Length or 0
** @@
******************************************************************************/

ajuint ensGvvariationfeatureCalculateLength(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return 0;

    return ensFeatureGetEnd(gvvf->Feature)
        - ensFeatureGetStart(gvvf->Feature)
        + 1;
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
** @@
******************************************************************************/

size_t ensGvvariationfeatureCalculateMemsize(
    const EnsPGvvariationfeature gvvf)
{
    size_t size = 0;

    if(!gvvf)
        return 0;

    size += sizeof (EnsOGvvariationfeature);

    size += ensFeatureCalculateMemsize(gvvf->Feature);

    size += ensGvsourceCalculateMemsize(gvvf->Gvsource);

    size += ensGvvariationCalculateMemsize(gvvf->Gvvariation);

    if(gvvf->Allele)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Allele);
    }

    if(gvvf->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Name);
    }

    if(gvvf->Validationcode)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Validationcode);
    }

    if(gvvf->Consequencetype)
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
** Get the somatic element of an
** Ensembl Genetic Variation Variation Feature.
**
** @cc Bio::EnsEMBL::Variation::VariationFeature::somatic
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjBool] Somatic attribute or ajFalse
** @@
******************************************************************************/

AjBool ensGvvariationfeatureGetSomatic(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return ajFalse;

    return ensGvsourceGetSomatic(gvvf->Gvsource);
}




/* @func ensGvvariationfeatureGetSourcename ***********************************
**
** Get the name element of an Ensembl Genetic Variation Source associated
** with an Ensembl Genetic Variation Variation Feature.
**
** @cc Bio::EnsEMBL::Variation::VariationFeature::source
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Ensembl Genetic Variation Source name or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetSourcename(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return ajFalse;

    return ensGvsourceGetName(gvvf->Gvsource);
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureIsReference(EnsPGvvariationfeature gvvf,
                                        AjBool* Presult)
{
    AjBool nonreference = AJFALSE;
    AjBool result = AJFALSE;

    EnsPSlice slice = NULL;

    if(!gvvf)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    slice = ensFeatureGetSlice(gvvf->Feature);

    result = ensSliceIsNonreference(slice, &nonreference);

    if(nonreference == ajTrue)
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




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Gvvariationfeature Functions for manipulating AJAX List objects
** of Ensembl Genetic Variation Variation Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                                    Variation Feature objects
** @argrule Descending gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                                     Variation Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listGvvariationfeatureCompareStartAscending ********************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by Ensembl Feature start element in ascending order.
**
** @param [r] P1 [const void*] Ensembl Genetic Variation
**                             Variation Feature address 1
** @param [r] P2 [const void*] Ensembl Genetic Variation
**                             Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listGvvariationfeatureCompareStartAscending(const void* P1,
                                                       const void* P2)
{
    const EnsPGvvariationfeature gvvf1 = NULL;
    const EnsPGvvariationfeature gvvf2 = NULL;

    gvvf1 = *(EnsPGvvariationfeature const*) P1;
    gvvf2 = *(EnsPGvvariationfeature const*) P2;

    if(ajDebugTest("listGvvariationfeatureCompareStartAscending"))
        ajDebug("listGvvariationfeatureCompareStartAscending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(gvvf1 && (!gvvf2))
        return -1;

    if((!gvvf1) && (!gvvf2))
        return 0;

    if((!gvvf1) && gvvf2)
        return +1;

    return ensFeatureCompareStartAscending(gvvf1->Feature, gvvf2->Feature);
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
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortStartAscending(AjPList gvvfs)
{
    if(!gvvfs)
        return ajFalse;

    ajListSort(gvvfs, listGvvariationfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listGvvariationfeatureCompareStartDescending *******************
**
** AJAX List of Ensembl Genetic Variation Variation Feature objects comparison
** function to sort by Ensembl Feature start element in descending order.
**
** @param [r] P1 [const void*] Ensembl Genetic Variation
**                             Variation Feature address 1
** @param [r] P2 [const void*] Ensembl Genetic Variation
**                             Variation Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listGvvariationfeatureCompareStartDescending(const void* P1,
                                                        const void* P2)
{
    const EnsPGvvariationfeature gvvf1 = NULL;
    const EnsPGvvariationfeature gvvf2 = NULL;

    gvvf1 = *(EnsPGvvariationfeature const*) P1;
    gvvf2 = *(EnsPGvvariationfeature const*) P2;

    if(ajDebugTest("listGvvariationfeatureCompareStartDescending"))
        ajDebug("listGvvariationfeatureCompareStartDescending\n"
                "  gvvf1 %p\n"
                "  gvvf2 %p\n",
                gvvf1,
                gvvf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(gvvf1 && (!gvvf2))
        return -1;

    if((!gvvf1) && (!gvvf2))
        return 0;

    if((!gvvf1) && gvvf2)
        return +1;

    return ensFeatureCompareStartDescending(gvvf1->Feature, gvvf2->Feature);
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
** @@
******************************************************************************/

AjBool ensListGvvariationfeatureSortStartDescending(AjPList gvvfs)
{
    if(!gvvfs)
        return ajFalse;

    ajListSort(gvvfs, listGvvariationfeatureCompareStartDescending);

    return ajTrue;
}




/*
** TODO: The following methods are not implemented:
**   * get_all_TranscriptVariations
**   * add_TranscriptVariation
**   * display_consequence
**   * add_consequence_type
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvfs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvvariationfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvfs)
{
    ajint mapweight = 0;

    ajuint lastgvvfid    = 0;
    ajuint identifier    = 0;
    ajuint gvvidentifier = 0;
    ajuint sourceid  = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr allele      = NULL;
    AjPStr name        = NULL;
    AjPStr validation  = NULL;
    AjPStr consequence = NULL;
    AjPStr class       = NULL;

    EnsEGvvariationClass gvvc = ensEGvvariationClassNULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPGvsource        gvsource  = NULL;
    EnsPGvsourceadaptor gvsourcea = NULL;

    EnsPGvvariationfeature        gvvf  = NULL;
    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("gvvariationfeatureadaptorFetchAllbyStatement"))
        ajDebug("gvvariationfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvvfs %p\n",
                dba,
                statement,
                am,
                slice,
                gvvfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvvfs)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(dba);

    gvvfa = ensRegistryGetGvvariationfeatureadaptor(dba);

    gvsourcea = ensRegistryGetGvsourceadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        /* Skip multiple rows because of the left join to failed_variation. */

        if(lastgvvfid == identifier)
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

        /* Need to get the internal Ensembl Sequence Region identifier. */

        srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa, srid);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Feature objects, the range
        ** needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("exonadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("exonadaptorFetchAllbyStatement got a "
                    "Sequence Region end coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srend, INT_MAX);

        slstrand = srstrand;

        /* Fetch a Slice spanning the entire Sequence Region. */

        ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);

        /*
        ** Increase the reference counter of the Ensembl Assembly Mapper if
        ** one has been specified, otherwise fetch it from the database if a
        ** destination Slice has been specified.
        */

        if(am)
            am = ensAssemblymapperNewRef(am);
        else if(slice && (!ensCoordsystemMatch(
                              ensSliceGetCoordsystemObject(slice),
                              ensSliceGetCoordsystemObject(srslice))))
            ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice, &am);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper is defined at this point.
        */

        if(am)
        {
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

            ajListPop(mrs, (void**) &mr);

            /*
            ** Skip Feature objects that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&allele);
                ajStrDel(&name);
                ajStrDel(&validation);
                ajStrDel(&consequence);
                ajStrDel(&class);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                ensMapperresultDel(&mr);

                continue;
            }

            srid     = ensMapperresultGetObjectidentifier(mr);
            slstart  = ensMapperresultGetCoordinateStart(mr);
            slend    = ensMapperresultGetCoordinateEnd(mr);
            slstrand = ensMapperresultGetCoordinateStrand(mr);

            /*
            ** Delete the Sequence Region Slice and fetch a Slice in the
            ** Coordinate System we just mapped to.
            */

            ensSliceDel(&srslice);

            ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                      srid,
                                                      0,
                                                      0,
                                                      0,
                                                      &srslice);

            ensMapperresultDel(&mr);
        }

        /*
        ** Convert Sequence Region Slice coordinates to destination Slice
        ** coordinates, if a destination Slice has been provided.
        */

        if(slice)
        {
            /* Check that the length of the Slice is within range. */

            if(ensSliceCalculateLength(slice) <= INT_MAX)
                sllength = (ajint) ensSliceCalculateLength(slice);
            else
                ajFatal("exonadaptorFetchAllbyStatement got a "
                        "Slice, which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceCalculateLength(slice), INT_MAX);

            /*
            ** Nothing needs to be done if the destination Slice starts at 1
            ** and is on the forward strand.
            */

            if((ensSliceGetStart(slice) != 1) ||
               (ensSliceGetStrand(slice) < 0))
            {
                if(ensSliceGetStrand(slice) >= 0)
                {
                    slstart = slstart - ensSliceGetStart(slice) + 1;
                    slend   = slend   - ensSliceGetStart(slice) + 1;
                }
                else
                {
                    slend     = ensSliceGetEnd(slice) - slstart + 1;
                    slstart   = ensSliceGetEnd(slice) - slend   + 1;
                    slstrand *= -1;
                }
            }

            /*
            ** Throw away Feature objects off the end of the requested Slice or
            ** on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&allele);
                ajStrDel(&name);
                ajStrDel(&validation);
                ajStrDel(&consequence);
                ajStrDel(&class);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        /*
        ** Finally, create a new
        ** Ensembl Genetic Variation Variation Feature.
        */

        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

        ensGvsourceadaptorFetchByIdentifier(gvsourcea, sourceid, &gvsource);

        /* Set the Ensembl Genetic Variation Variation Class. */

        gvvc = ensGvvariationClassFromStr(class);

        if(!gvvc)
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

        ajListPushAppend(gvvfs, (void*) gvvf);

        ensGvsourceDel(&gvsource);

        ensFeatureDel(&feature);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);

        ajStrDel(&allele);
        ajStrDel(&name);
        ajStrDel(&validation);
        ajStrDel(&consequence);
        ajStrDel(&class);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic gvvariationfeatureadaptorCacheReference ************************
**
** Wrapper function to reference an
** Ensembl Genetic Variation Variation Feature from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Genetic Variation Variation Feature
**
** @return [void*] Ensembl Genetic Variation Variation Feature or NULL
** @@
******************************************************************************/

static void* gvvariationfeatureadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensGvvariationfeatureNewRef(
        (EnsPGvvariationfeature) value);
}




/* @funcstatic gvvariationfeatureadaptorCacheDelete ***************************
**
** Wrapper function to delete an
** Ensembl Genetic Variation Variation Feature from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Genetic Variation Variation Feature
**                           address
**
** @return [void]
** @@
******************************************************************************/

static void gvvariationfeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensGvvariationfeatureDel((EnsPGvvariationfeature*) value);

    return;
}




/* @funcstatic gvvariationfeatureadaptorCacheSize *****************************
**
** Wrapper function to determine the memory size of an
** Ensembl Genetic Variation Variation Feature from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Genetic Variation Variation Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t gvvariationfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensGvvariationfeatureCalculateMemsize(
        (const EnsPGvvariationfeature) value);
}




/* @funcstatic gvvariationfeatureadaptorGetFeature ****************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Genetic Variation Variation Feature
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Genetic Variation Variation Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature gvvariationfeatureadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensGvvariationfeatureGetFeature(
        (const EnsPGvvariationfeature) value);
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
** @valrule * [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                            Variation Feature Adaptor or NULL
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
** @return [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                         Variation Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationfeatureadaptor ensGvvariationfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPFeatureadaptor fa = NULL;

    EnsPGvvariationfeatureadaptor gvvfa = NULL;

    if(!dba)
        return NULL;

    fa = ensFeatureadaptorNew(
        dba,
        gvvariationfeatureadaptorTables,
        gvvariationfeatureadaptorColumns,
        gvvariationfeatureadaptorLeftjoin,
        gvvariationfeatureadaptorDefaultcondition,
        (const char*) NULL,
        gvvariationfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        gvvariationfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        gvvariationfeatureadaptorCacheDelete,
        gvvariationfeatureadaptorCacheSize,
        gvvariationfeatureadaptorGetFeature,
        "Ensembl Genetic Variation Variation Feature");

    if(!fa)
        return NULL;

    AJNEW0(gvvfa);

    gvvfa->Adaptor        = ensRegistryGetGvdatabaseadaptor(dba);
    gvvfa->Featureadaptor = fa;

    return gvvfa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Variation Feature Adaptor
** object.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Feature Adaptor object
**
** @argrule * Pgvvfa [EnsPGvvariationfeatureadaptor*]
** Ensembl Genetic Variation Variation Feature Adaptor object address
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
** Ensembl Genetic Variation Variation Feature Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationfeatureadaptorDel(EnsPGvvariationfeatureadaptor* Pgvvfa)
{
    EnsPGvvariationfeatureadaptor pthis = NULL;

    if(!Pgvvfa)
        return;

    if(!*Pgvvfa)
        return;

    pthis = *Pgvvfa;

    ensFeatureadaptorDel(&pthis->Featureadaptor);

    AJFREE(pthis);

    *Pgvvfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Variation Feature Adaptor object.
**
** @fdata [EnsPGvvariationfeatureadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Variation Feature Adaptor
** attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
** @nam4rule Gvdatabaseadaptor
** Return the Ensembl Genetic Variation Database Adaptor
**
** @argrule * gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
** @valrule Gvdatabaseadaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureadaptorGetDatabaseadaptor ***********************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvvariationfeatureadaptorGetDatabaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    if(!gvvfa)
        return NULL;

    return ensGvdatabaseadaptorGetDatabaseadaptor(gvvfa->Adaptor);
}




/* @func ensGvvariationfeatureadaptorGetFeatureadaptor ************************
**
** Get the Ensembl Feature Adaptor element of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPFeatureadaptor ensGvvariationfeatureadaptorGetFeatureadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    if(!gvvfa)
        return NULL;

    return gvvfa->Featureadaptor;
}




/* @func ensGvvariationfeatureadaptorGetGvdatabaseadaptor *********************
**
** Get the Ensembl Genetic Variation Database Adaptor element of an
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @param [r] gvvfa [const EnsPGvvariationfeatureadaptor]
** Ensembl Genetic Variation Variation Feature Adaptor
**
** @return [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor or NULL
** @@
******************************************************************************/

EnsPGvdatabaseadaptor ensGvvariationfeatureadaptorGetGvdatabaseadaptor(
    const EnsPGvvariationfeatureadaptor gvvfa)
{
    if(!gvvfa)
        return NULL;

    return gvvfa->Adaptor;
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbyFrequency(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvpopulation gvp,
    float frequency,
    EnsPSlice slice,
    AjPList gvvfs)
{
    const char* const* txtcolumns = NULL;

    register ajuint i = 0;

    AjPStr columns       = NULL;
    AjPStr fvsconstraint = NULL;
    AjPStr statement     = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvvfa)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!gvvfs)
        return ajFalse;

    /* Adjust frequency if given a percentage. */

    if(frequency > 1.0F)
        frequency /= 100.0F;

    /* Build the column expression. */

    ba = ensFeatureadaptorGetBaseadaptor(gvvfa->Featureadaptor);

    txtcolumns = ensBaseadaptorGetColumns(ba);

    columns = ajStrNew();

    for(i = 0; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /* Build the final SQL statement. */

    ensGvdatabaseadaptorFailedvariationsconstraint(gvvfa->Adaptor,
                                                   (const AjPStr) NULL,
                                                   &fvsconstraint);

    dba = ensFeatureadaptorGetDatabaseadaptor(gvvfa->Featureadaptor);

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

    gvvariationfeatureadaptorFetchAllbyStatement(dba,
                                                 statement,
                                                 (EnsPAssemblymapper) NULL,
                                                 (EnsPSlice) NULL,
                                                 gvvfs);

    ajStrDel(&columns);
    ajStrDel(&statement);

    return ajTrue;
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbyGvpopulation(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvpopulation gvp,
    EnsPSlice slice,
    AjPList gvvfs)
{
    const char* const* txtcolumns = NULL;

    register ajuint i = 0;

    AjPStr columns   = NULL;
    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvvfa)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!gvvfs)
        return ajFalse;

    /* Build the column expression. */

    ba = ensFeatureadaptorGetBaseadaptor(gvvfa->Featureadaptor);

    txtcolumns = ensBaseadaptorGetColumns(ba);

    columns = ajStrNew();

    for(i = 0; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /* Build the final SQL statement. */

    dba = ensFeatureadaptorGetDatabaseadaptor(gvvfa->Featureadaptor);

    statement = ajFmtStr(
        "SELECT "
        "%S "
        "FROM "
        "variation_feature vf, "
        "source s, "
        "allele a "
        "WHERE "
        "variation_feature.source_id = s.source_id "
        "AND "
        "variation_feature.variation_id = a.variation_id "
        "AND "
        "a.sample_id = %u "
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

    gvvariationfeatureadaptorFetchAllbyStatement(dba,
                                                 statement,
                                                 (EnsPAssemblymapper) NULL,
                                                 (EnsPSlice) NULL,
                                                 gvvfs);

    ajStrDel(&columns);
    ajStrDel(&statement);

    return ajTrue;
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbyGvvariation(
    EnsPGvvariationfeatureadaptor gvvfa,
    const EnsPGvvariation gvv,
    AjPList gvvfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!gvvfa)
        return ajFalse;

    if(!gvv)
        return ajFalse;

    if(!gvvfs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(gvvfa->Featureadaptor);

    constraint = ajFmtStr("variation_feature.variation_id = %u",
                          ensGvvariationGetIdentifier(gvv));

    result = ensBaseadaptorFetchAllbyConstraint(ba,
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
    const char* const* txtcolumns = NULL;

    char* txtstring = NULL;

    register ajuint i = 0;

    AjPStr columns       = NULL;
    AjPStr constraint    = NULL;
    AjPStr fvsconstraint = NULL;
    AjPStr statement     = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    ensGvdatabaseadaptorFailedvariationsconstraint(gvvfa->Adaptor,
                                                   (const AjPStr) NULL,
                                                   &fvsconstraint);

    constraint = ajFmtStr("vsource.somatic = %d AND %S",
                          somatic,
                          fvsconstraint);

    ajStrDel(&fvsconstraint);

    if(vsource && ajStrGetLen(vsource))
    {
        ensFeatureadaptorEscapeC(gvvfa->Featureadaptor, &txtstring, vsource);

        ajFmtPrintAppS(&constraint, " AND vsource.name = '%s'", txtstring);

        ajCharDel(&txtstring);
    }

    if(psource && ajStrGetLen(psource))
    {
        ensFeatureadaptorEscapeC(gvvfa->Featureadaptor,
                                 &txtstring,
                                 psource);

        ajFmtPrintAppS(&constraint, " AND psource.name = '%s'", txtstring);

        ajCharDel(&txtstring);
    }

    if(annotation && ajStrGetLen(annotation))
    {
        ensFeatureadaptorEscapeC(gvvfa->Featureadaptor,
                                 &txtstring,
                                 annotation);

        if(ajStrIsNum(annotation))
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

    ba = ensFeatureadaptorGetBaseadaptor(gvvfa->Featureadaptor);

    txtcolumns = ensBaseadaptorGetColumns(ba);

    columns = ajStrNew();

    for(i = 0; txtcolumns[i]; i++)
        ajFmtPrintAppS(&columns, "%s, ", txtcolumns[i]);

    /* Remove last comma and space from the column expression. */

    ajStrCutEnd(&columns, 2);

    /* Build the final SQL statement. */

    dba = ensFeatureadaptorGetDatabaseadaptor(gvvfa->Featureadaptor);

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

    gvvariationfeatureadaptorFetchAllbyStatement(dba,
                                                 statement,
                                                 (EnsPAssemblymapper) NULL,
                                                 (EnsPSlice) NULL,
                                                 gvvfs);

    ajStrDel(&columns);
    ajStrDel(&constraint);
    ajStrDel(&statement);

    return ajTrue;
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

    if(!gvvfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!gvvfs)
        return ajFalse;

    ensGvdatabaseadaptorFailedvariationsconstraint(gvvfa->Adaptor,
                                                   (const AjPStr) NULL,
                                                   &fvsconstraint);

    /* By default, filter out somatic mutations. */

    if(constraint && ajStrGetLen(constraint))
        newconstraint = ajFmtStr("%S AND source.somatic = %d AND %S",
                                 constraint,
                                 somatic,
                                 fvsconstraint);
    else
        newconstraint = ajFmtStr("source.somatic = %d AND %S",
                                 somatic,
                                 fvsconstraint);

    result = ensFeatureadaptorFetchAllbySlice(gvvfa->Featureadaptor,
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchAllbySliceGenotyped(
    EnsPGvvariationfeatureadaptor gvvfa,
    EnsPSlice slice,
    AjPList gvvfs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!gvvfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!gvvfs)
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
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorFetchByIdentifier(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPGvvariationfeature* Pgvvf)
{
    EnsPBaseadaptor ba = NULL;

    if(!gvvfa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvvf)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(gvvfa->Featureadaptor);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void**) Pgvvf);
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
** @nam5rule Identifiers Retrieve all AJAX unsigned integer identifiers
**
** @argrule * gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
** Variation Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
** integers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureadaptorRetrieveAllIdentifiers *******************
**
** Fetch all SQL database-internal identifiers of Ensembl Genetic Variation
** Variation Feature objects.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationFeatureAdaptor::list_dbIDs
** @param [u] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureadaptorRetrieveAllIdentifiers(
    EnsPGvvariationfeatureadaptor gvvfa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!gvvfa)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(gvvfa->Featureadaptor);

    table = ajStrNewC("variation_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
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
** @cc CVS Revision: 1.6
** @cc CVS Tag: branch-ensembl-61
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
** @argrule Ref gvvs [EnsPGvvariationset] Ensembl Genetic Variation
**                                        Variation Set
**
** @valrule * [EnsPGvvariationset] Ensembl Genetic Variation Variation Set or
** NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationsetNewCpy **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
**
** @return [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
**                              or NULL
** @@
******************************************************************************/

EnsPGvvariationset ensGvvariationsetNewCpy(const EnsPGvvariationset gvvs)
{
    EnsPGvvariationset pthis = NULL;

    if(!gvvs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gvvs->Identifier;

    pthis->Adaptor = gvvs->Adaptor;

    if(gvvs->Name)
        pthis->Name = ajStrNewRef(gvvs->Name);

    if(gvvs->Description)
        pthis->Description = ajStrNewRef(gvvs->Description);

    return pthis;
}




/* @func ensGvvariationsetNewIni **********************************************
**
** Constructor for an Ensembl Genetic Variation Variation Set with initial
** values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [uN] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                               Variation Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::VariationSet::new
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
**
** @return [EnsPGvvariationset] Ensembl Genetic Variation Variation Set or NULL
** @@
******************************************************************************/

EnsPGvvariationset ensGvvariationsetNewIni(EnsPGvvariationsetadaptor gvvsa,
                                           ajuint identifier,
                                           AjPStr name,
                                           AjPStr description)
{
    EnsPGvvariationset gvvs = NULL;

    if(!name)
        return NULL;

    if(!description)
        return NULL;

    AJNEW0(gvvs);

    gvvs->Use = 1;

    gvvs->Identifier = identifier;

    gvvs->Adaptor = gvvsa;

    if(name)
        gvvs->Name = ajStrNewS(name);

    if(description)
        gvvs->Description = ajStrNewS(description);

    return gvvs;
}




/* @func ensGvvariationsetNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
**
** @return [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
** @@
******************************************************************************/

EnsPGvvariationset ensGvvariationsetNewRef(EnsPGvvariationset gvvs)
{
    if(!gvvs)
        return NULL;

    gvvs->Use++;

    return gvvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Set object
**
** @argrule * Pgvvs [EnsPGvvariationset*]
** Ensembl Genetic Variation Variation Set object address
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
** Ensembl Genetic Variation Variation Set object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationsetDel(EnsPGvvariationset* Pgvvs)
{
    EnsPGvvariationset pthis = NULL;

    if(!Pgvvs)
        return;

    if(!*Pgvvs)
        return;

    if(ajDebugTest("ensGvvariationsetDel"))
    {
        ajDebug("ensGvvariationsetDel\n"
                "  *Pgvvs %p\n",
                *Pgvvs);

        ensGvvariationsetTrace(*Pgvvs, 1);
    }

    pthis = *Pgvvs;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvvs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pgvvs = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Get Return Genetic Variation Variation Set attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation
**                   Variation Set Adaptor
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Description Return the description
**
** @argrule * gvvs [const EnsPGvvariationset]
** Ensembl Genetic Variation Variation Set
**
** @valrule Adaptor [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationsetGetAdaptor ******************************************
**
** Get the Ensembl Genetic Variation Variation Set Adaptor element of an
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
**
** @return [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                     Variation Set Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationsetadaptor ensGvvariationsetGetAdaptor(
    const EnsPGvvariationset gvvs)
{
    if(!gvvs)
        return NULL;

    return gvvs->Adaptor;
}




/* @func ensGvvariationsetGetDescription **************************************
**
** Get the description element of an Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationsetGetDescription(
    const EnsPGvvariationset gvvs)
{
    if(!gvvs)
        return NULL;

    return gvvs->Description;
}




/* @func ensGvvariationsetGetIdentifier ***************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvvariationsetGetIdentifier(
    const EnsPGvvariationset gvvs)
{
    if(!gvvs)
        return 0;

    return gvvs->Identifier;
}




/* @func ensGvvariationsetGetName *********************************************
**
** Get the name element of an Ensembl Genetic Variation Variation Set.
**
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensGvvariationsetGetName(
    const EnsPGvvariationset gvvs)
{
    if(!gvvs)
        return NULL;

    return gvvs->Name;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Set Set one element of a Genetic Variation Variation Set
** @nam4rule Adaptor Set the Ensembl Genetic Variation
**                   Variation Set Adaptor
** @nam4rule Description Set the description
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
**
** @argrule * gvvs [EnsPGvvariationset] Ensembl Genetic Variation
**                                      Variation Set object
** @argrule Adaptor gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
** Variation Set Adaptor
** @argrule Description description [AjPStr] Description
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationsetSetAdaptor ******************************************
**
** Set the Ensembl Genetic Variation Variation Set Adaptor element of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation
**                                      Variation Set
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetSetAdaptor(EnsPGvvariationset gvvs,
                                   EnsPGvvariationsetadaptor gvvsa)
{
    if(!gvvs)
        return ajFalse;

    gvvs->Adaptor = gvvsa;

    return ajTrue;
}




/* @func ensGvvariationsetSetDescription **************************************
**
** Set the description element of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation
**                                      Variation Set
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetSetDescription(EnsPGvvariationset gvvs,
                                       AjPStr description)
{
    if(!gvvs)
        return ajFalse;

    ajStrDel(&gvvs->Description);

    if(description)
        gvvs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvvariationsetSetIdentifier ***************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation
**                                      Variation Set
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetSetIdentifier(EnsPGvvariationset gvvs,
                                      ajuint identifier)
{
    if(!gvvs)
        return ajFalse;

    gvvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationsetSetName *********************************************
**
** Set the name element of an
** Ensembl Genetic Variation Variation Set.
**
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation
**                                      Variation Set
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetSetName(EnsPGvvariationset gvvs,
                                AjPStr name)
{
    if(!gvvs)
        return ajFalse;

    ajStrDel(&gvvs->Name);

    if(name)
        gvvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Trace Report Ensembl Genetic Variation Variation Set elements
**                 to debug file
**
** @argrule Trace gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                                Variation Set
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
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetTrace(const EnsPGvvariationset gvvs,
                              ajuint level)
{
    AjPStr indent = NULL;

    if(!gvvs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvvariationsetTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Description '%S'\n",
            indent, gvvs,
            indent, gvvs->Use,
            indent, gvvs->Identifier,
            indent, gvvs->Adaptor,
            indent, gvvs->Name,
            indent, gvvs->Description);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Variation Set
** values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
** Variation Set
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
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensGvvariationsetCalculateMemsize(const EnsPGvvariationset gvvs)
{
    size_t size = 0;

    if(!gvvs)
        return 0;

    size += sizeof (EnsOGvvariationset);

    if(gvvs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvs->Name);
    }

    if(gvvs->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvvs->Description);
    }

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an
** Ensembl Genetic Variation Variation Set object.
**
** @fdata [EnsPGvvariationset]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Variation Set values
** @nam4rule All Fetch all objects
** @nam5rule Gvvariations Fetch all Ensembl Genetic Variation Variation objects
** @nam5rule Sub Fetch sub-Ensembl Genetic Variation Variation Set objects
** @nam5rule Super Fetch super-Ensembl Genetic Variation Variation Set objects
**
** @argrule * gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation
** Set
** @argrule Sub immediate [AjBool] Only immediate
** @argrule Super immediate [AjBool] Only immediate
** @argrule * gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Set objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationsetFetchAllGvvariations ********************************
**
** Recursively fetch all Ensembl Genetic Variation Variation Sets, which
** belong to this Ensembl Genetic Variation Variation Set and all subsets.
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::VariationSet::get_all_Variations
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetFetchAllGvvariations(
    EnsPGvvariationset gvvs,
    AjPList gvvss)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationadaptor gvva = NULL;

    if(!gvvs)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    if(!gvvs->Adaptor)
        return ajTrue;

    dba = ensBaseadaptorGetDatabaseadaptor(gvvs->Adaptor);

    gvva = ensRegistryGetGvvariationadaptor(dba);

    return ensGvvariationadaptorFetchAllbyGvvariationset(gvva, gvvs, gvvss);
}




/* @func ensGvvariationsetFetchAllSub *****************************************
**
** Recursively fetch all Ensembl Genetic Variation Variation Sets, which are
** subsets of this Ensembl Genetic Variation Variation Set.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Sets before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::VariationSet::get_all_sub_VariationSets
** @param [u] gvvs [EnsPGvvariationset] Ensembl Genetic Variation Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct subsets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all subsets
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetFetchAllSub(EnsPGvvariationset gvvs,
                                    AjBool immediate,
                                    AjPList gvvss)
{
    if(!gvvs)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    if(gvvs->Adaptor)
        return ajTrue;

    return ensGvvariationsetadaptorFetchAllbySuper(gvvs->Adaptor,
                                                   gvvs,
                                                   immediate,
                                                   gvvss);
}




/* @func ensGvvariationsetFetchAllSuper ***************************************
**
** Recursively fetch all Ensembl Genetic Variation Variation Sets, which are
** supersets of this Ensembl Genetic Variation Variation Set.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Sets before deleting the AJAX List.
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
** @@
******************************************************************************/

AjBool ensGvvariationsetFetchAllSuper(EnsPGvvariationset gvvs,
                                      AjBool immediate,
                                      AjPList gvvss)
{
    if(!gvvs)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    if(gvvs->Adaptor)
        return ajTrue;

    return ensGvvariationsetadaptorFetchAllbySub(gvvs->Adaptor,
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
** @cc CVS Revision: 1.5
** @cc CVS Tag: branch-ensembl-61
**
******************************************************************************/




/* @funcstatic gvvariationsetadaptorFetchAllbyStatement ***********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Genetic Variation Variation Set
** objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::_objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Sets
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvvariationsetadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvvss)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr description = NULL;

    EnsPGvvariationset        gvvs  = NULL;
    EnsPGvvariationsetadaptor gvvsa = NULL;

    if(ajDebugTest("gvvariationsetadaptorFetchAllbyStatement"))
        ajDebug("gvvariationsetadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvvss %p\n",
                dba,
                statement,
                am,
                slice,
                gvvss);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    gvvsa = ensRegistryGetGvvariationsetadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        name        = ajStrNew();
        description = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &description);

        gvvs = ensGvvariationsetNewIni(gvvsa,
                                       identifier,
                                       name,
                                       description);

        ajListPushAppend(gvvss, (void*) gvvs);

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
** @valrule * [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                        Variation Set Adaptor or NULL
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
** @return [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                     Variation Set Adaptor or NULL
** @@
******************************************************************************/

EnsPGvvariationsetadaptor ensGvvariationsetadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    if(ajDebugTest("ensGvvariationsetadaptorNew"))
        ajDebug("ensGvvariationsetadaptorNew\n"
                "  dba %p\n",
                dba);

    return ensBaseadaptorNew(
        dba,
        gvvariationsetadaptorTables,
        gvvariationsetadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        gvvariationsetadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Variation Set Adaptor
** object.
**
** @fdata [EnsPGvvariationsetadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Variation Set Adaptor object
**
** @argrule * Pgvvsa [EnsPGvvariationsetadaptor*]
** Ensembl Genetic Variation Variation Set Adaptor object address
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
** Ensembl Genetic Variation Variation Set Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationsetadaptorDel(EnsPGvvariationsetadaptor* Pgvvsa)
{
    if(!Pgvvsa)
        return;

    if(!*Pgvvsa)
        return;

    ensBaseadaptorDel(Pgvvsa);

    *Pgvvsa = NULL;

    return;
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
**
** @argrule * gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @argrule All gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                              Variation Set objects
** @argrule Gvvariation gvv [const EnsPGvvariation] Ensembl Genetic Variation
** Variation
** @argrule AllbySub gvvs [const EnsPGvvariationset] Ensembl Genetic
** Variation Variation Set
** @argrule AllbySub immediate [AjBool] only direct sub-sets
** @argrule AllbySuper gvvs [const EnsPGvvariationset] Ensembl Genetic
** Variation Variation Set
** @argrule AllbySuper immediate [AjBool] only direct super-sets
** @argrule Allby gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                                Variation Set objects
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [const AjPStr] Name
** @argrule By Pgvvs [EnsPGvvariationset*] Ensembl Genetic Variation
**                                         Variation Set address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationsetadaptorFetchAllToplevel *****************************
**
** Fetch all top-level Ensembl Genetic Variation Variation Sets, which
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
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllToplevel(
    EnsPGvvariationsetadaptor gvvsa,
    AjPList gvvss)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!gvvss)
        return ajFalse;

    if(!gvvss)
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

    result = ensBaseadaptorFetchAllbyConstraint(gvvsa,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                gvvss);

    ajStrDel(&constraint);

    return result;
}




/* @funcstatic listGvvariationsetCompareIdentifier ****************************
**
** AJAX List comparison function to sort Ensembl Genetic Variation Variation
** Set objects by identifier in ascending order.
**
** @param [r] P1 [const void*] Ensembl Genetic Variation Variation Set 1
** @param [r] P2 [const void*] Ensembl Genetic Variation Variation Set 2
** @see ajListSort
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listGvvariationsetCompareIdentifier(const void* P1, const void* P2)
{
    int result = 0;

    const EnsPGvvariationset gvvs1 = NULL;
    const EnsPGvvariationset gvvs2 = NULL;

    gvvs1 = *(EnsPGvvariationset const*) P1;
    gvvs2 = *(EnsPGvvariationset const*) P2;

    if(ajDebugTest("gvvariationsetlistCompare"))
        ajDebug("gvvariationsetlistCompare\n"
                "  gvvs1 %p\n"
                "  gvvs2 %p\n",
                gvvs1,
                gvvs2);

    if(gvvs1->Identifier < gvvs2->Identifier)
        result = -1;

    if(gvvs1->Identifier > gvvs2->Identifier)
        result = +1;

    return result;
}




/* @funcstatic listGvvariationsetDelete ***************************************
**
** ajListSortUnique nodedelete function to delete Ensembl Genetic Variation
** Variation Set objects that are redundant.
**
** @param [r] PP1 [void**] Ensembl Genetic Variation Variation Set address 1
** @param [r] cl [void*] Standard, passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
** @@
******************************************************************************/

static void listGvvariationsetDelete(void** PP1, void* cl)
{
    if(!PP1)
        return;

    (void) cl;

    ensGvvariationsetDel((EnsPGvvariationset*) PP1);

    return;
}




/* @funcstatic gvvariationsetadaptorGvvariationsetMove ************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of Ensembl Genetic
** Variation Variation Sets used in ensGvvariationsetadaptorFetchAllbySub.
** This function deletes the AJAX unsigned identifier key data and moves the
** Ensembl Genetic Variation Variation Set value data onto the
** AJAX List.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Gene value data address
** @param [u] cl [void*] AJAX List, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvvariationsetadaptorGvvariationsetMove(void** key,
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

    if(!cl)
        return;

    AJFREE(*key);

    ajListPushAppend(cl, value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensGvvariationsetadaptorFetchAllbyGvvariation ************************
**
** Fetch an Ensembl Genetic Variation Variation Set via an
** Ensembl Genetic Variation Variation.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Sets before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::fetch_by_Variation
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Sets
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllbyGvvariation(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariation gvv,
    AjPList gvvss)
{
    const char* const* txtcolumns = NULL;

    register ajuint i = 0;

    AjBool result = AJFALSE;

    AjPList variationsets = NULL;

    AjPStr columns = NULL;
    AjPStr statement = NULL;

    EnsPGvvariationset gvvs = NULL;

    if(!gvvsa)
        return ajFalse;

    if(!gvv)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    txtcolumns = ensBaseadaptorGetColumns(gvvsa);

    columns = ajStrNew();

    for(i = 0; txtcolumns[i]; i++)
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

    result = ensBaseadaptorFetchAllbyConstraint(gvvsa,
                                                statement,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                variationsets);

    ajStrDel(&statement);

    /*
    ** Fetch all supersets of the returned Ensembl Genetic Variation
    ** Variation Sets as well. Since an Ensembl Genetic Variation Variation may
    ** occur at several places in a hierarchy, which will cause duplicated
    ** data sort the AJAX List uniquely.
    */

    while(ajListPop(variationsets, (void**) &gvvs))
    {
        ensGvvariationsetadaptorFetchAllbySub(gvvsa,
                                              gvvs,
                                              ajFalse,
                                              gvvss);

        /*
        ** Move the current Ensembl Genetic Variation Variation Set onto the
        ** AJAX List of subsets, too.
        */

        ajListPushAppend(gvvss, (void*) gvvs);
    }

    ajListFree(&variationsets);

    ajListSortUnique(gvvss,
                     listGvvariationsetCompareIdentifier,
                     listGvvariationsetDelete);

    return result;
}




/* @func ensGvvariationsetadaptorFetchAllbySub ********************************
**
** Fetch all Ensembl Genetic Variation Variation Sets, by a subset of an
** Ensembl Genetic Variation Variation Set.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Sets before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::
**     fetch_all_by_sub_VariationSet
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct supersets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all supersets
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllbySub(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariationset gvvs,
    AjBool immediate,
    AjPList gvvss)
{
    ajuint* Pidentifier = NULL;

    ajuint vssid = 0;

    AjPList variationsets = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable gvvstable = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationset variationset = NULL;

    if(!gvvsa)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    variationsets = ajListNew();

    gvvstable = ensTableuintNewLen(0);

    dba = ensBaseadaptorGetDatabaseadaptor(gvvsa);

    /*
    ** First, get all Ensembl Genetic Variation Variation Sets that are
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

    while(!ajSqlrowiterDone(sqli))
    {
        vssid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &vssid);

        if(!ajTableFetchmodV(gvvstable, (const void*) &vssid))
        {
            ensGvvariationsetadaptorFetchByIdentifier(gvvsa,
                                                      vssid,
                                                      &variationset);

            if(!variationset)
                continue;

            AJNEW0(Pidentifier);

            *Pidentifier = vssid;

            ajTablePut(gvvstable, (void*) Pidentifier, (void*) variationset);

            if(immediate)
                continue;

            ensGvvariationsetadaptorFetchAllbySub(gvvsa,
                                                  variationset,
                                                  immediate,
                                                  variationsets);

            while(ajListPop(variationsets, (void**) &variationset))
            {
                if(!variationset)
                    continue;

                vssid = ensGvvariationsetGetIdentifier(variationset);

                if(!ajTableFetchmodV(gvvstable, (const void*) &vssid))
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = ensGvvariationsetGetIdentifier(
                        variationset);

                    ajTablePut(gvvstable,
                               (void*) Pidentifier,
                               (void*) ensGvvariationsetNewRef(variationset));
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
    ** Move all Ensembl Genetic Variation Variation Sets from the
    ** AJAX Table onto the AJAX List.
    */

    ajTableMapDel(gvvstable, gvvariationsetadaptorGvvariationsetMove, gvvss);

    ajTableFree(&gvvstable);

    return ajTrue;
}




/* @func ensGvvariationsetadaptorFetchAllbySuper ******************************
**
** Fetch all Ensembl Genetic Variation Variation Sets, by a superset of an
** Ensembl Genetic Variation Variation Set.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Sets before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::
**     fetch_all_by_super_VariationSet
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @param [r] gvvs [const EnsPGvvariationset] Ensembl Genetic Variation
**                                            Variation Set
** @param [r] immediate [AjBool] ajTrue:  only the direct subsets of this
**                                        variation set will be fetched
**                               ajFalse: recursively fetch all supersets
** @param [u] gvvss [AjPList] AJAX List of Ensembl Genetic Variation
**                            Variation Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchAllbySuper(
    EnsPGvvariationsetadaptor gvvsa,
    const EnsPGvvariationset gvvs,
    AjBool immediate,
    AjPList gvvss)
{
    ajuint* Pidentifier = NULL;

    ajuint vssid = 0;

    AjPList variationsets = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    AjPTable gvvstable = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvvariationset variationset = NULL;

    if(!gvvsa)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    if(!gvvss)
        return ajFalse;

    variationsets = ajListNew();

    gvvstable = ensTableuintNewLen(0);

    dba = ensBaseadaptorGetDatabaseadaptor(gvvsa);

    /*
    ** First, get all Ensembl Genetic Variation Variation Sets that are
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

    while(!ajSqlrowiterDone(sqli))
    {
        vssid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &vssid);

        if(!ajTableFetchmodV(gvvstable, (const void*) &vssid))
        {
            ensGvvariationsetadaptorFetchByIdentifier(gvvsa,
                                                      vssid,
                                                      &variationset);

            if(!variationset)
                continue;

            AJNEW0(Pidentifier);

            *Pidentifier = vssid;

            ajTablePut(gvvstable, (void*) Pidentifier, (void*) variationset);

            if(immediate)
                continue;

            ensGvvariationsetadaptorFetchAllbySub(gvvsa,
                                                  variationset,
                                                  immediate,
                                                  variationsets);

            while(ajListPop(variationsets, (void**) &variationset))
            {
                if(!variationset)
                    continue;

                vssid = ensGvvariationsetGetIdentifier(variationset);

                if(!ajTableFetchmodV(gvvstable, (const void*) &vssid))
                {
                    AJNEW0(Pidentifier);

                    *Pidentifier = ensGvvariationsetGetIdentifier(
                        variationset);

                    ajTablePut(gvvstable,
                               (void*) Pidentifier,
                               (void*) ensGvvariationsetNewRef(variationset));
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
    ** Move all Ensembl Genetic Variation Variation Sets from the
    ** AJAX Table onto the AJAX List.
    */

    ajTableMapDel(gvvstable, gvvariationsetadaptorGvvariationsetMove, gvvss);

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
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvvs [EnsPGvvariationset*] Ensembl Genetic Variation
**                                         Variation Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchByIdentifier(
    EnsPGvvariationsetadaptor gvvsa,
    ajuint identifier,
    EnsPGvvariationset* Pgvvs)
{
    if(!gvvsa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvvs)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(gvvsa, identifier, (void**) Pgvvs);
}




/* @func ensGvvariationsetadaptorFetchByName **********************************
**
** Fetch an Ensembl Genetic Variation Variation Set via its name.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation Set.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationSetAdaptor::fetch_by_name
** @param [u] gvvsa [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                              Variation Set Adaptor
** @param [r] name [const AjPStr] Name
** @param [wP] Pgvvs [EnsPGvvariationset*] Ensembl Genetic Variation
**                                         Variation Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationsetadaptorFetchByName(
    EnsPGvvariationsetadaptor gvvsa,
    const AjPStr name,
    EnsPGvvariationset* Pgvvs)
{
    char* txtname = NULL;

    AjPList gvvss = NULL;

    AjPStr constraint = NULL;

    EnsPGvvariationset gvvs = NULL;

    if(!gvvsa)
        return ajFalse;

    if(!(name && ajStrGetLen(name)))
        return ajFalse;

    if(!Pgvvs)
        return ajFalse;

    ensBaseadaptorEscapeC(gvvsa, &txtname, name);

    constraint = ajFmtStr("variation_set.name = %s", txtname);

    ajCharDel(&txtname);

    gvvss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvvsa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvvss);

    if(ajListGetLength(gvvss) > 1)
        ajWarn("ensGvvariationsetadaptorFetchByName got more "
               "than one Ensembl Genetic Variation Variation Set for "
               "name '%S'.\n", name);

    ajListPop(gvvss, (void**) Pgvvs);

    while(ajListPop(gvvss, (void**) &gvvs))
        ensGvvariationsetDel(&gvvs);

    ajListFree(&gvvss);

    ajStrDel(&constraint);

    return ajTrue;
}

/* @source enssimple **********************************************************
**
** Ensembl Simple Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.22 $
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

#include "enssimple.h"




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

/* @conststatic simplefeatureadaptorKTables ***********************************
**
** Array of Ensembl Simple Feature Adaptor SQL table names
**
******************************************************************************/

static const char *const simplefeatureadaptorKTables[] =
{
    "simple_feature",
    (const char *) NULL
};




/* @conststatic simplefeatureadaptorKColumns **********************************
**
** Array of Ensembl Simple Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const simplefeatureadaptorKColumns[] =
{
    "simple_feature.simple_feature_id",
    "simple_feature.seq_region_id",
    "simple_feature.seq_region_start",
    "simple_feature.seq_region_end",
    "simple_feature.seq_region_strand",
    "simple_feature.analysis_id",
    "simple_feature.label",
    "simple_feature.score",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listSimplefeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listSimplefeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listSimplefeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listSimplefeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listSimplefeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool simplefeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList sfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enssimple *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSimplefeature] Ensembl Simple Feature ********************
**
** @nam2rule Simplefeature Functions for manipulating
** Ensembl Simple Feature objects
**
** @cc Bio::EnsEMBL::SimpleFeature
** @cc CVS Revision: 1.14
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Simple Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Simple Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy sf [const EnsPSimplefeature] Ensembl Simple Feature
** @argrule Ini sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini label [AjPStr] Display label
** @argrule Ini score [double] Score
** @argrule Ref sf [EnsPSimplefeature] Ensembl Simple Feature
**
** @valrule * [EnsPSimplefeature] Ensembl Simple Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensSimplefeatureNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeature] Ensembl Simple Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewCpy(const EnsPSimplefeature sf)
{
    EnsPSimplefeature pthis = NULL;

    if (!sf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = sf->Identifier;
    pthis->Adaptor    = sf->Adaptor;
    pthis->Feature    = ensFeatureNewRef(sf->Feature);

    if (sf->Displaylabel)
        pthis->Displaylabel = ajStrNewRef(sf->Displaylabel);

    pthis->Score = sf->Score;

    return pthis;
}




/* @func ensSimplefeatureNewIni ***********************************************
**
** Constructor of an Ensembl Simple Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::SimpleFeature::new
** @param [u] label [AjPStr] Display label
** @param [r] score [double] Score
**
** @return [EnsPSimplefeature] Ensembl Simple Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewIni(EnsPSimplefeatureadaptor sfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         AjPStr label,
                                         double score)
{
    EnsPSimplefeature sf = NULL;

    if (!feature)
        return NULL;

    AJNEW0(sf);

    sf->Use = 1U;

    sf->Identifier = identifier;
    sf->Adaptor    = sfa;
    sf->Feature    = ensFeatureNewRef(feature);

    if (label)
        sf->Displaylabel = ajStrNewRef(label);

    sf->Score = score;

    return sf;
}




/* @func ensSimplefeatureNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeature] Ensembl Simple Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewRef(EnsPSimplefeature sf)
{
    if (!sf)
        return NULL;

    sf->Use++;

    return sf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Del Destroy (free) an Ensembl Simple Feature
**
** @argrule * Psf [EnsPSimplefeature*] Ensembl Simple Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSimplefeatureDel **************************************************
**
** Default destructor for an Ensembl Simple Feature.
**
** @param [d] Psf [EnsPSimplefeature*] Ensembl Simple Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensSimplefeatureDel(EnsPSimplefeature *Psf)
{
    EnsPSimplefeature pthis = NULL;

    if (!Psf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensSimplefeatureDel"))
    {
        ajDebug("ensSimplefeatureDel\n"
                "  *Psf %p\n",
                *Psf);

        ensSimplefeatureTrace(*Psf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Psf)
        return;

    pthis = *Psf;

    pthis->Use--;

    if (pthis->Use)
    {
        *Psf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->Displaylabel);

    AJFREE(pthis);

    *Psf = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Get Return Simple Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Simple Feature Adaptor
** @nam4rule Displaylabel Return the display label
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Score Return the score
**
** @argrule * sf [const EnsPSimplefeature] Simple Feature
**
** @valrule Adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Displaylabel [AjPStr] Display label or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Score [double] Score or 0.0
**
** @fcategory use
******************************************************************************/




/* @func ensSimplefeatureGetAdaptor *******************************************
**
** Get the Ensembl Simple Feature Adaptor member of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensSimplefeatureGetAdaptor(
    const EnsPSimplefeature sf)
{
    return (sf) ? sf->Adaptor : NULL;
}




/* @func ensSimplefeatureGetDisplaylabel **************************************
**
** Get the display label member of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::display_label
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [AjPStr] Display label or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensSimplefeatureGetDisplaylabel(
    const EnsPSimplefeature sf)
{
    return (sf) ? sf->Displaylabel : NULL;
}




/* @func ensSimplefeatureGetFeature *******************************************
**
** Get the Ensembl Feature member of an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensSimplefeatureGetFeature(
    const EnsPSimplefeature sf)
{
    return (sf) ? sf->Feature : NULL;
}




/* @func ensSimplefeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensSimplefeatureGetIdentifier(
    const EnsPSimplefeature sf)
{
    return (sf) ? sf->Identifier : 0U;
}




/* @func ensSimplefeatureGetScore *********************************************
**
** Get the score member of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::score
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [double] Score or 0.0
**
** @release 6.2.0
** @@
******************************************************************************/

double ensSimplefeatureGetScore(
    const EnsPSimplefeature sf)
{
    return (sf) ? sf->Score : 0.0;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Set Set one member of a Simple Feature
** @nam4rule Adaptor Set the Ensembl Simple Feature Adaptor
** @nam4rule Displaylabel Set the display label
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Score Set the score
**
** @argrule * sf [EnsPSimplefeature] Ensembl Simple Feature object
** @argrule Adaptor sfa [EnsPSimplefeatureadaptor] Ensembl Simple
**                                                 Feature Adaptor
** @argrule Displaylabel label [AjPStr] Display label
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Score score [double] Score
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSimplefeatureSetAdaptor *******************************************
**
** Set the Ensembl Simple Feature Adaptor member of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSimplefeatureSetAdaptor(EnsPSimplefeature sf,
                                  EnsPSimplefeatureadaptor sfa)
{
    if (!sf)
        return ajFalse;

    sf->Adaptor = sfa;

    return ajTrue;
}




/* @func ensSimplefeatureSetDisplaylabel **************************************
**
** Set the display label member of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::display_label
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] label [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSimplefeatureSetDisplaylabel(EnsPSimplefeature sf, AjPStr label)
{
    if (!sf)
        return ajFalse;

    ajStrDel(&sf->Displaylabel);

    sf->Displaylabel = ajStrNewRef(label);

    return ajTrue;
}




/* @func ensSimplefeatureSetFeature *******************************************
**
** Set the Ensembl Feature member of an Ensembl Simple Feature.
**
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSimplefeatureSetFeature(EnsPSimplefeature sf, EnsPFeature feature)
{
    if (!sf)
        return ajFalse;

    ensFeatureDel(&sf->Feature);

    sf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensSimplefeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSimplefeatureSetIdentifier(EnsPSimplefeature sf, ajuint identifier)
{
    if (!sf)
        return ajFalse;

    sf->Identifier = identifier;

    return ajTrue;
}




/* @func ensSimplefeatureSetScore *********************************************
**
** Set the score member of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::score
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSimplefeatureSetScore(EnsPSimplefeature sf, double score)
{
    if (!sf)
        return ajFalse;

    sf->Score = score;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Trace Report Ensembl Simple Feature members to debug file
**
** @argrule Trace sf [const EnsPSimplefeature] Ensembl Simple Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSimplefeatureTrace ************************************************
**
** Trace an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensSimplefeatureTrace(const EnsPSimplefeature sf, ajuint level)
{
    AjPStr indent = NULL;

    if (!sf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSimplefeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Displaylabel '%S'\n"
            "%S  Score %f\n",
            indent, sf,
            indent, sf->Use,
            indent, sf->Identifier,
            indent, sf->Adaptor,
            indent, sf->Feature,
            indent, sf->Displaylabel,
            indent, sf->Score);

    ensFeatureTrace(sf->Feature, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Calculate Calculate Ensembl Simple Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensSimplefeatureCalculateMemsize *************************************
**
** Calculate the memory size in bytes of an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensSimplefeatureCalculateMemsize(const EnsPSimplefeature sf)
{
    size_t size = 0;

    if (!sf)
        return 0;

    size += sizeof (EnsOSimplefeature);

    size += ensFeatureCalculateMemsize(sf->Feature);

    if (sf->Displaylabel)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(sf->Displaylabel);
    }

    return size;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listSimplefeatureCompareEndAscending ***************************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** Ensembl Feature end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Simple Feature address 1
** @param [r] item2 [const void*] Ensembl Simple Feature address 2
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

static int listSimplefeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPSimplefeature sf1 = *(EnsOSimplefeature *const *) item1;
    EnsPSimplefeature sf2 = *(EnsOSimplefeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSimplefeatureCompareEndAscending"))
    {
        ajDebug("listSimplefeatureCompareEndAscending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

        ensSimplefeatureTrace(sf1, 1);
        ensSimplefeatureTrace(sf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (sf1 && (!sf2))
        return -1;

    if ((!sf1) && (!sf2))
        return 0;

    if ((!sf1) && sf2)
        return +1;

    return ensFeatureCompareEndAscending(sf1->Feature, sf2->Feature);
}




/* @funcstatic listSimplefeatureCompareEndDescending **************************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** Ensembl Feature end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Simple Feature address 1
** @param [r] item2 [const void*] Ensembl Simple Feature address 2
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

static int listSimplefeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPSimplefeature sf1 = *(EnsOSimplefeature *const *) item1;
    EnsPSimplefeature sf2 = *(EnsOSimplefeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSimplefeatureCompareEndDescending"))
    {
        ajDebug("listSimplefeatureCompareEndDescending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

        ensSimplefeatureTrace(sf1, 1);
        ensSimplefeatureTrace(sf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (sf1 && (!sf2))
        return -1;

    if ((!sf1) && (!sf2))
        return 0;

    if ((!sf1) && sf2)
        return +1;

    return ensFeatureCompareEndDescending(sf1->Feature, sf2->Feature);
}




/* @funcstatic listSimplefeatureCompareIdentifierAscending ********************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** identifier member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Simple Feature address 1
** @param [r] item2 [const void*] Ensembl Simple Feature address 2
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

static int listSimplefeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPSimplefeature sf1 = *(EnsOSimplefeature *const *) item1;
    EnsPSimplefeature sf2 = *(EnsOSimplefeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSimplefeatureCompareIdentifierAscending"))
    {
        ajDebug("listSimplefeatureCompareIdentifierAscending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

        ensSimplefeatureTrace(sf1, 1);
        ensSimplefeatureTrace(sf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (sf1 && (!sf2))
        return -1;

    if ((!sf1) && (!sf2))
        return 0;

    if ((!sf1) && sf2)
        return +1;

    if (sf1->Identifier < sf2->Identifier)
        return -1;

    if (sf1->Identifier > sf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listSimplefeatureCompareStartAscending *************************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Simple Feature address 1
** @param [r] item2 [const void*] Ensembl Simple Feature address 2
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

static int listSimplefeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPSimplefeature sf1 = *(EnsOSimplefeature *const *) item1;
    EnsPSimplefeature sf2 = *(EnsOSimplefeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSimplefeatureCompareStartAscending"))
    {
        ajDebug("listSimplefeatureCompareStartAscending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

        ensSimplefeatureTrace(sf1, 1);
        ensSimplefeatureTrace(sf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (sf1 && (!sf2))
        return -1;

    if ((!sf1) && (!sf2))
        return 0;

    if ((!sf1) && sf2)
        return +1;

    return ensFeatureCompareStartAscending(sf1->Feature, sf2->Feature);
}




/* @funcstatic listSimplefeatureCompareStartDescending ************************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Simple Feature address 1
** @param [r] item2 [const void*] Ensembl Simple Feature address 2
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

static int listSimplefeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPSimplefeature sf1 = *(EnsOSimplefeature *const *) item1;
    EnsPSimplefeature sf2 = *(EnsOSimplefeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSimplefeatureCompareStartDescending"))
    {
        ajDebug("listSimplefeatureCompareStartDescending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

        ensSimplefeatureTrace(sf1, 1);
        ensSimplefeatureTrace(sf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (sf1 && (!sf2))
        return -1;

    if ((!sf1) && (!sf2))
        return 0;

    if ((!sf1) && sf2)
        return +1;

    return ensFeatureCompareStartDescending(sf1->Feature, sf2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Simplefeature Functions for manipulating AJAX List objects of
** Ensembl Simple Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature start member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending sfs [AjPList]  AJAX List of
**                                   Ensembl Simple Feature objects
** @argrule Descending sfs [AjPList] AJAX List of
**                                   Ensembl Simple Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListSimplefeatureSortEndAscending *********************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** Ensembl Feature end coordinate in ascending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortEndAscending(AjPList sfs)
{
    if (!sfs)
        return ajFalse;

    ajListSortTwoThree(sfs,
                       &listSimplefeatureCompareEndAscending,
                       &listSimplefeatureCompareStartAscending,
                       &listSimplefeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListSimplefeatureSortEndDescending ********************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** Ensembl Feature end coordinate in descending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortEndDescending(AjPList sfs)
{
    if (!sfs)
        return ajFalse;

    ajListSortTwoThree(sfs,
                       &listSimplefeatureCompareEndDescending,
                       &listSimplefeatureCompareStartDescending,
                       &listSimplefeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListSimplefeatureSortIdentifierAscending **************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** identifier member in ascending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortIdentifierAscending(AjPList sfs)
{
    if (!sfs)
        return ajFalse;

    ajListSort(sfs, &listSimplefeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListSimplefeatureSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortStartAscending(AjPList sfs)
{
    if (!sfs)
        return ajFalse;

    ajListSortTwoThree(sfs,
                       &listSimplefeatureCompareStartAscending,
                       &listSimplefeatureCompareEndAscending,
                       &listSimplefeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListSimplefeatureSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortStartDescending(AjPList sfs)
{
    if (!sfs)
        return ajFalse;

    ajListSortTwoThree(sfs,
                       &listSimplefeatureCompareStartDescending,
                       &listSimplefeatureCompareEndDescending,
                       &listSimplefeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor *****
**
** @nam2rule Simplefeatureadaptor Functions for manipulating
** Ensembl Simple Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor
** @cc CVS Revision: 1.38
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic simplefeatureadaptorFetchAllbyStatement ************************
**
** Fetch all Ensembl Simple Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool simplefeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList sfs)
{
    double score = 0;

    ajuint identifier = 0U;
    ajuint analysisid = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr label = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPSimplefeature sf         = NULL;
    EnsPSimplefeatureadaptor sfa = NULL;

    if (ajDebugTest("simplefeatureadaptorFetchAllbyStatement"))
        ajDebug("simplefeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  sfs %p\n",
                ba,
                statement,
                am,
                slice,
                sfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!sfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    sfa = ensRegistryGetSimplefeatureadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        srstrand   = 0;
        label      = ajStrNew();
        score      = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &label);
        ajSqlcolumnToDouble(sqlr, &score);

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &feature);

        if (!feature)
        {
            ajStrDel(&label);

            continue;
        }

        /* Finally, create a new Ensembl Simple Feature. */

        sf = ensSimplefeatureNewIni(sfa, identifier, feature, label, score);

        ajListPushAppend(sfs, (void *) sf);

        ensFeatureDel(&feature);

        ajStrDel(&label);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Simple Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Simple Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSimplefeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensSimplefeatureadaptorNew *******************************************
**
** Default constructor for an Ensembl Simple Feature Adaptor.
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
** @see ensRegistryGetSimplefeatureadaptor
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensSimplefeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        simplefeatureadaptorKTables,
        simplefeatureadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &simplefeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensSimplefeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensSimplefeatureDel,
        (size_t (*)(const void *)) &ensSimplefeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensSimplefeatureGetFeature,
        "Simple Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Simple Feature Adaptor object.
**
** @fdata [EnsPSimplefeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Simple Feature Adaptor
**
** @argrule * Psfa [EnsPSimplefeatureadaptor*]
** Ensembl Simple Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSimplefeatureadaptorDel *******************************************
**
** Default destructor for an Ensembl Simple Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Psfa [EnsPSimplefeatureadaptor*]
** Ensembl Simple Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensSimplefeatureadaptorDel(EnsPSimplefeatureadaptor *Psfa)
{
    ensFeatureadaptorDel(Psfa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Simple Feature Adaptor object.
**
** @fdata [EnsPSimplefeatureadaptor]
**
** @nam3rule Get Return Ensembl Simple Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSimplefeatureadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Simple Feature Adaptor.
**
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSimplefeatureadaptorGetDatabaseadaptor(
    EnsPSimplefeatureadaptor sfa)
{
    return ensFeatureadaptorGetDatabaseadaptor(sfa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Simple Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPSimplefeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Simple Feature object(s)
** @nam4rule All   Fetch all Ensembl Simple Feature objects
** @nam4rule Allby Fetch all Ensembl Simple Feature objects
**                 matching a criterion
** @nam5rule Analysisname Fetch all by an Ensembl Analysis name
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam5rule Slicescore Fetch all by an Ensembl Slice and score
** @nam4rule By    Fetch one Ensembl Simple Feature object
**                 matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @argrule All aefs [AjPList] AJAX List of Ensembl Simple Feature objects
** @argrule AllbyAnalysisname anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyAnalysisname sfs [AjPList]
** AJAX List of Ensembl Simple Feature objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice sfs [AjPList]
** AJAX List of Ensembl Simple Feature objects
** @argrule AllbySlicescore slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicescore score [double] Score
** @argrule AllbySlicescore anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicescore sfs [AjPList]
** AJAX List of Ensembl Simple Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Psf [EnsPSimplefeature*]
** Ensembl Simple Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSimplefeatureadaptorFetchAllbyAnalysisname ************************
**
** Fetch all Ensembl Simple Feature objects via an Ensembl Analysis name.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_logic_name
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllbyAnalysisname(
    EnsPSimplefeatureadaptor sfa,
    const AjPStr anname,
    AjPList sfs)
{
    if (!sfa)
        return ajFalse;

    if (!anname)
        return ajFalse;

    if (!sfs)
        return ajFalse;

    return ensFeatureadaptorFetchAllbyAnalysisname(sfa, anname, sfs);
}




/* @func ensSimplefeatureadaptorFetchAllbySlice *******************************
**
** Fetch all Ensembl Simple Feature objects on an Ensembl Slice.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllbySlice(EnsPSimplefeatureadaptor sfa,
                                              EnsPSlice slice,
                                              const AjPStr anname,
                                              AjPList sfs)
{
    if (!sfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!sfs)
        return ajFalse;

    return ensFeatureadaptorFetchAllbySlice(sfa,
                                            slice,
                                            (const AjPStr) NULL,
                                            anname,
                                            sfs);
}




/* @func ensSimplefeatureadaptorFetchAllbySlicescore **************************
**
** Fetch all Ensembl Simple Feature objects on an Ensembl Slice
** above a threshold score.
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureAdaptor::fetch_all_by_Slice
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] score [double] Score
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllbySlicescore(
    EnsPSimplefeatureadaptor sfa,
    EnsPSlice slice,
    double score,
    const AjPStr anname,
    AjPList sfs)
{
    if (!sfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!sfs)
        return ajFalse;

    return ensFeatureadaptorFetchAllbySlicescore(sfa,
                                                 slice,
                                                 score,
                                                 anname,
                                                 sfs);
}

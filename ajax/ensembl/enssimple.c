/* @source Ensembl Simple Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.5 $
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

#include "enssimple.h"




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

/* @conststatic simplefeatureadaptorTables ************************************
**
** Array of Ensembl Simple Feature Adaptor SQL table names
**
******************************************************************************/

static const char* const simplefeatureadaptorTables[] =
{
    "simple_feature",
    (const char*) NULL
};




/* @conststatic simplefeatureadaptorColumns ***********************************
**
** Array of Ensembl Simple Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const simplefeatureadaptorColumns[] =
{
    "simple_feature.simple_feature_id",
    "simple_feature.seq_region_id",
    "simple_feature.seq_region_start",
    "simple_feature.seq_region_end",
    "simple_feature.seq_region_strand",
    "simple_feature.analysis_id",
    "simple_feature.label",
    "simple_feature.score",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static int listSimplefeatureCompareStartAscending(const void* P1,
                                                  const void* P2);

static int listSimplefeatureCompareStartDescending(const void* P1,
                                                   const void* P2);

static AjBool simplefeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList sfs);

static void* simplefeatureadaptorCacheReference(void* value);

static void simplefeatureadaptorCacheDelete(void** value);

static size_t simplefeatureadaptorCacheSize(const void* value);

static EnsPFeature simplefeatureadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-62
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
** @valrule * [EnsPSimplefeature] Ensembl Simple Feature
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
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewCpy(const EnsPSimplefeature sf)
{
    EnsPSimplefeature pthis = NULL;

    if(!sf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = sf->Identifier;
    pthis->Adaptor    = sf->Adaptor;
    pthis->Feature    = ensFeatureNewRef(sf->Feature);

    if(sf->Displaylabel)
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
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewIni(EnsPSimplefeatureadaptor sfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         AjPStr label,
                                         double score)
{
    EnsPSimplefeature sf = NULL;

    if(!feature)
        return NULL;

    AJNEW0(sf);

    sf->Use = 1;

    sf->Identifier = identifier;
    sf->Adaptor    = sfa;
    sf->Feature    = ensFeatureNewRef(feature);

    if(label)
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
** @@
******************************************************************************/

EnsPSimplefeature ensSimplefeatureNewRef(EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;

    sf->Use++;

    return sf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Del Destroy (free) an Ensembl Simple Feature object
**
** @argrule * Psf [EnsPSimplefeature*] Ensembl Simple Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSimplefeatureDel **************************************************
**
** Default destructor for an Ensembl Simple Feature.
**
** @param [d] Psf [EnsPSimplefeature*] Ensembl Simple Feature object address
**
** @return [void]
** @@
******************************************************************************/

void ensSimplefeatureDel(EnsPSimplefeature* Psf)
{
    EnsPSimplefeature pthis = NULL;

    if(!Psf)
        return;

    if(!*Psf)
        return;

    pthis = *Psf;

    pthis->Use--;

    if(pthis->Use)
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




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Simple Feature object.
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
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Score [double] Score or 0.0
**
** @fcategory use
******************************************************************************/




/* @func ensSimplefeatureGetAdaptor *******************************************
**
** Get the Ensembl Simple Feature Adaptor element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensSimplefeatureGetAdaptor(
    const EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;

    return sf->Adaptor;
}




/* @func ensSimplefeatureGetDisplaylabel **************************************
**
** Get the display label element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::display_label
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [AjPStr] Display label or NULL
** @@
******************************************************************************/

AjPStr ensSimplefeatureGetDisplaylabel(
    const EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;

    return sf->Displaylabel;
}




/* @func ensSimplefeatureGetFeature *******************************************
**
** Get the Ensembl Feature element of an Ensembl Simple Feature.
**
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensSimplefeatureGetFeature(
    const EnsPSimplefeature sf)
{
    if(!sf)
        return NULL;

    return sf->Feature;
}




/* @func ensSimplefeatureGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensSimplefeatureGetIdentifier(
    const EnsPSimplefeature sf)
{
    if(!sf)
        return 0;

    return sf->Identifier;
}




/* @func ensSimplefeatureGetScore *********************************************
**
** Get the score element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::score
** @param [r] sf [const EnsPSimplefeature] Ensembl Simple Feature
**
** @return [double] Score or 0.0
** @@
******************************************************************************/

double ensSimplefeatureGetScore(
    const EnsPSimplefeature sf)
{
    if(!sf)
        return 0.0;

    return sf->Score;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Simple Feature object.
**
** @fdata [EnsPSimplefeature]
**
** @nam3rule Set Set one element of a Simple Feature
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
** Set the Ensembl Simple Feature Adaptor element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetAdaptor(EnsPSimplefeature sf,
                                  EnsPSimplefeatureadaptor sfa)
{
    if(!sf)
        return ajFalse;

    sf->Adaptor = sfa;

    return ajTrue;
}




/* @func ensSimplefeatureSetDisplaylabel **************************************
**
** Set the display label element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::display_label
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] label [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetDisplaylabel(EnsPSimplefeature sf, AjPStr label)
{
    if(!sf)
        return ajFalse;

    ajStrDel(&sf->Displaylabel);

    sf->Displaylabel = ajStrNewRef(label);

    return ajTrue;
}




/* @func ensSimplefeatureSetFeature *******************************************
**
** Set the Ensembl Feature element of an Ensembl Simple Feature.
**
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetFeature(EnsPSimplefeature sf, EnsPFeature feature)
{
    if(!sf)
        return ajFalse;

    ensFeatureDel(&sf->Feature);

    sf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensSimplefeatureSetIdentifier ****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetIdentifier(EnsPSimplefeature sf, ajuint identifier)
{
    if(!sf)
        return ajFalse;

    sf->Identifier = identifier;

    return ajTrue;
}




/* @func ensSimplefeatureSetScore *********************************************
**
** Set the score element of an Ensembl Simple Feature.
**
** @cc Bio::EnsEMBL::SimpleFeature::score
** @param [u] sf [EnsPSimplefeature] Ensembl Simple Feature
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSimplefeatureSetScore(EnsPSimplefeature sf, double score)
{
    if(!sf)
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
** @nam3rule Trace Report Ensembl Simple Feature elements to debug file
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
** @@
******************************************************************************/

AjBool ensSimplefeatureTrace(const EnsPSimplefeature sf, ajuint level)
{
    AjPStr indent = NULL;

    if(!sf)
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
** @@
******************************************************************************/

size_t ensSimplefeatureCalculateMemsize(const EnsPSimplefeature sf)
{
    size_t size = 0;

    if(!sf)
        return 0;

    size += sizeof (EnsOSimplefeature);

    size += ensFeatureCalculateMemsize(sf->Feature);

    if(sf->Displaylabel)
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




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Simplefeature Functions for manipulating AJAX List objects of
** Ensembl Simple Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
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




/* @funcstatic listSimplefeatureCompareStartAscending *************************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Simple Feature address 1
** @param [r] P2 [const void*] Ensembl Simple Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listSimplefeatureCompareStartAscending(const void* P1,
                                                  const void* P2)
{
    EnsPSimplefeature sf1 = NULL;
    EnsPSimplefeature sf2 = NULL;

    sf1 = *(EnsPSimplefeature const*) P1;
    sf2 = *(EnsPSimplefeature const*) P2;

    if(ajDebugTest("listSimplefeatureCompareStartAscending"))
    {
        ajDebug("listSimplefeatureCompareStartAscending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

        ensSimplefeatureTrace(sf1, 1);
        ensSimplefeatureTrace(sf2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(sf1 && (!sf2))
        return -1;

    if((!sf1) && (!sf2))
        return 0;

    if((!sf1) && sf2)
        return +1;

    return ensFeatureCompareStartAscending(sf1->Feature, sf2->Feature);
}




/* @func ensListSimplefeatureSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortStartAscending(AjPList sfs)
{
    if(!sfs)
        return ajFalse;

    ajListSort(sfs, listSimplefeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listSimplefeatureCompareStartDescending ************************
**
** AJAX List of Ensembl Simple Feature objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Simple Feature address 1
** @param [r] P2 [const void*] Ensembl Simple Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listSimplefeatureCompareStartDescending(const void* P1,
                                                   const void* P2)
{
    const EnsPSimplefeature sf1 = NULL;
    const EnsPSimplefeature sf2 = NULL;

    sf1 = *(EnsPSimplefeature const*) P1;
    sf2 = *(EnsPSimplefeature const*) P2;

    if(ajDebugTest("listSimplefeatureCompareStartDescending"))
        ajDebug("listSimplefeatureCompareStartDescending\n"
                "  sf1 %p\n"
                "  sf2 %p\n",
                sf1,
                sf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(sf1 && (!sf2))
        return -1;

    if((!sf1) && (!sf2))
        return 0;

    if((!sf1) && sf2)
        return +1;

    return ensFeatureCompareStartDescending(sf1->Feature, sf2->Feature);
}




/* @func ensListSimplefeatureSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Simple Feature objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListSimplefeatureSortStartDescending(AjPList sfs)
{
    if(!sfs)
        return ajFalse;

    ajListSort(sfs, listSimplefeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor *****
**
** @nam2rule Simplefeatureadaptor Functions for manipulating
** Ensembl Simple Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor
** @cc CVS Revision: 1.36
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic simplefeatureadaptorFetchAllbyStatement ************************
**
** Fetch all Ensembl Simple Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::SimpleFeatureAdaptor::_objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] sfs [AjPList] AJAX List of Ensembl Simple Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool simplefeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList sfs)
{
    double score = 0;

    ajint srstrand = 0;

    ajuint identifier = 0;
    ajuint analysisid = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr label = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPSimplefeature sf         = NULL;
    EnsPSimplefeatureadaptor sfa = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("simplefeatureadaptorFetchAllbyStatement"))
        ajDebug("simplefeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  sfs %p\n",
                dba,
                statement,
                am,
                slice,
                sfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!sfs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    sfa = ensRegistryGetSimplefeatureadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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
            ajFatal("simplefeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("simplefeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region end coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srend, INT_MAX);

        slstrand = srstrand;

        /* Fetch a Slice spanning the entire Sequence Region. */

        ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);

        /*
        ** Obtain an Ensembl Assembly Mapper if none was defined, but a
        ** destination Slice was.
        */

        if(am)
            am = ensAssemblymapperNewRef(am);
        else if(slice && (!ensCoordsystemMatch(
                              ensSliceGetCoordsystemObject(slice),
                              ensSliceGetCoordsystemObject(srslice))))
            ensAssemblymapperadaptorFetchBySlices(ama, slice, srslice, &am);

        /*
        ** Remap the Feature coordinates to another Ensembl Coordinate System
        ** if an Ensembl Assembly Mapper was provided.
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
            ** Skip Ensembl Feature objects that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&label);

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
                ajFatal("simplefeatureadaptorFetchAllbyStatement got a Slice, "
                        "which length (%u) exceeds the "
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

                    slend = slend - ensSliceGetStart(slice) + 1;
                }
                else
                {
                    slend = ensSliceGetEnd(slice) - slstart + 1;

                    slstart = ensSliceGetEnd(slice) - slend + 1;

                    slstrand *= -1;
                }
            }

            /*
            ** Throw away Feature objects off the end of the requested
            ** Slice or on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Load the next Feature but destroy first! */

                ajStrDel(&label);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        /* Finally, create a new Ensembl Simple Feature. */

        feature = ensFeatureNewIniS(analysis,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

        sf = ensSimplefeatureNewIni(sfa, identifier, feature, label, score);

        ajListPushAppend(sfs, (void*) sf);

        ensFeatureDel(&feature);

        ensAnalysisDel(&analysis);

        ensAssemblymapperDel(&am);

        ensSliceDel(&srslice);

        ajStrDel(&label);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic simplefeatureadaptorCacheReference *****************************
**
** Wrapper function to reference an Ensembl Simple Feature
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Simple Feature
**
** @return [void*] Ensembl Simple Feature or NULL
** @@
******************************************************************************/

static void* simplefeatureadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensSimplefeatureNewRef((EnsPSimplefeature) value);
}




/* @funcstatic simplefeatureadaptorCacheDelete ********************************
**
** Wrapper function to delete an Ensembl Simple Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Simple Feature address
**
** @return [void]
** @@
******************************************************************************/

static void simplefeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensSimplefeatureDel((EnsPSimplefeature*) value);

    return;
}




/* @funcstatic simplefeatureadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Simple Feature
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Simple Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t simplefeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensSimplefeatureCalculateMemsize((const EnsPSimplefeature) value);
}




/* @funcstatic simplefeatureadaptorGetFeature *********************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Simple Feature
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Simple Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature simplefeatureadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensSimplefeatureGetFeature((const EnsPSimplefeature) value);
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
** @valrule * [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
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
** @@
******************************************************************************/

EnsPSimplefeatureadaptor ensSimplefeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        simplefeatureadaptorTables,
        simplefeatureadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        simplefeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        simplefeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        simplefeatureadaptorCacheDelete,
        simplefeatureadaptorCacheSize,
        simplefeatureadaptorGetFeature,
        "Simple Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Simple Feature Adaptor object.
**
** @fdata [EnsPSimplefeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Simple Feature Adaptor object
**
** @argrule * Psfa [EnsPSimplefeatureadaptor*] Ensembl Simple Feature Adaptor
**                                             object address
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
** @param [d] Psfa [EnsPSimplefeatureadaptor*] Ensembl Simple Feature Adaptor
**                                             object address
**
** @return [void]
** @@
******************************************************************************/

void ensSimplefeatureadaptorDel(EnsPSimplefeatureadaptor* Psfa)
{
    if(!Psfa)
        return;

    if(!*Psfa)
        return;

    ensFeatureadaptorDel(Psfa);

    *Psfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
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
** Get the Ensembl Database Adaptor element of an
** Ensembl Simple Feature Adaptor.
**
** @param [u] sfa [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSimplefeatureadaptorGetDatabaseadaptor(
    EnsPSimplefeatureadaptor sfa)
{
    if(!sfa)
        return NULL;

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
** @argrule AllbyAnalysisname sfs [AjPList] AJAX List of Ensembl Simple
**                                          Feature objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice sfs [AjPList] AJAX List of Ensembl Simple
**                                   Feature objects
** @argrule AllbySlicescore slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicescore score [double] Score
** @argrule AllbySlicescore anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicescore sfs [AjPList] AJAX List of Ensembl Simple
**                                        Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Psf [EnsPSimplefeature*] Ensembl Simple Feature
**                                                address
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
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllbyAnalysisname(
    EnsPSimplefeatureadaptor sfa,
    const AjPStr anname,
    AjPList sfs)
{
    if(!sfa)
        return ajFalse;

    if(!anname)
        return ajFalse;

    if(!sfs)
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
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllbySlice(EnsPSimplefeatureadaptor sfa,
                                              EnsPSlice slice,
                                              const AjPStr anname,
                                              AjPList sfs)
{
    if(!sfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!sfs)
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
** @@
******************************************************************************/

AjBool ensSimplefeatureadaptorFetchAllbySlicescore(
    EnsPSimplefeatureadaptor sfa,
    EnsPSlice slice,
    double score,
    const AjPStr anname,
    AjPList sfs)
{
    if(!sfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!sfs)
        return ajFalse;

    return ensFeatureadaptorFetchAllbySlicescore(sfa,
                                                 slice,
                                                 score,
                                                 anname,
                                                 sfs);
}

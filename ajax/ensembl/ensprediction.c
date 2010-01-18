/******************************************************************************
** @source Ensembl Prediction functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
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

#include "ensprediction.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPAnalysisadaptor
ensRegistryGetAnalysisadaptor(EnsPDatabaseadaptor dba);

extern EnsPAssemblymapperadaptor
ensRegistryGetAssemblymapperadaptor(EnsPDatabaseadaptor dba);

extern EnsPCoordsystemadaptor
ensRegistryGetCoordsystemadaptor(EnsPDatabaseadaptor dba);

extern EnsPPredictionexonadaptor
ensRegistryGetPredictionexonadaptor(EnsPDatabaseadaptor dba);

extern EnsPPredictiontranscriptadaptor
ensRegistryGetPredictiontranscriptadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static AjBool predictionExonadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                 const AjPStr statement,
                                                 EnsPAssemblymapper am,
                                                 EnsPSlice slice,
                                                 AjPList pes);

static void *predictionExonadaptorCacheReference(void *value);

static void predictionExonadaptorCacheDelete(void **value);

static ajuint predictionExonadaptorCacheSize(const void *value);

static EnsPFeature predictionExonadaptorGetFeature(const void *value);

static AjBool predictionTranscriptadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                       const AjPStr statement,
                                                       EnsPAssemblymapper am,
                                                       EnsPSlice slice,
                                                       AjPList pts);

static void *predictionTranscriptadaptorCacheReference(void *value);

static void predictionTranscriptadaptorCacheDelete(void **value);

static ajuint predictionTranscriptadaptorCacheSize(const void *value);

static EnsPFeature predictionTranscriptadaptorGetFeature(const void *value);

    


/* @filesection ensprediction *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPPredictionexon] Prediction Exon **************************
**
** Functions for manipulating Ensembl Prediction Exon objects
**
** @cc Bio::EnsEMBL::Predictionexon CVS Revision: 1.6
**
** @nam2rule Predictionexon
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Exon by pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Exon. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictionexon]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPPredictionexon] Ensembl Prediction Exon
** @argrule Ref object [EnsPPredictionexon] Ensembl Prediction Exon
**
** @valrule * [EnsPPredictionexon] Ensembl Prediction Exon
**
** @fcategory new
******************************************************************************/




/* @func ensPredictionexonNew *************************************************
**
** Default Ensembl Prediction Exon constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPPredictionexonadaptor] Ensembl Prediction
**                                                Exon Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Predictionexon::new
** @param [r] sphase [ajint] Start phase of Translation
** @param [r] score [double] Score
** @param [r] pvalue [double] P-value
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonNew(EnsPPredictionexonadaptor adaptor,
                                        ajuint identifier,
                                        EnsPFeature feature,
                                        ajint sphase,
                                        double score,
                                        double pvalue)
{
    EnsPPredictionexon pe = NULL;
    
    if(!feature)
        return NULL;
    
    AJNEW0(pe);
    
    pe->Use = 1;
    
    pe->Identifier = identifier;
    
    pe->Adaptor = adaptor;
    
    pe->Feature = ensFeatureNewRef(feature);
    
    pe->StartPhase = sphase;
    
    pe->Score = score;
    
    pe->Pvalue = pvalue;
    
    return pe;
}




/* @func ensPredictionexonNewObj **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonNewObj(const EnsPPredictionexon object)
{
    EnsPPredictionexon pe = NULL;
    
    AJNEW0(pe);
    
    pe->Use = 1;
    
    pe->Identifier = object->Identifier;
    
    pe->Adaptor = object->Adaptor;
    
    pe->Feature = ensFeatureNewRef(object->Feature);
    
    pe->StartPhase = object->StartPhase;
    
    pe->Score = object->Score;
    
    pe->Pvalue = object->Pvalue;
    
    return pe;
}




/* @func ensPredictionexonNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonNewRef(EnsPPredictionexon pe)
{
    if(!pe)
	return NULL;
    
    pe->Use++;
    
    return pe;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Prediction Exon.
**
** @fdata [EnsPPredictionexon]
** @fnote None
**
** @nam3rule Del Destroy (free) a Prediction Exon object
**
** @argrule * Ppe [EnsPPredictionexon*] Ensembl Prediction Exon object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictionexonDel *************************************************
**
** Default destructor for an Ensembl Prediction Exon.
**
** @param [d] Ppe [EnsPPredictionexon*] Ensembl Prediction Exon address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictionexonDel(EnsPPredictionexon *Ppe)
{
    EnsPPredictionexon pthis = NULL;
    
    if(!Ppe)
        return;
    
    if(!*Ppe)
        return;

    pthis = *Ppe;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Ppe = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    AJFREE(pthis);

    *Ppe = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
** @fnote None
**
** @nam3rule Get Return Prediction Exon attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Prediction Exon Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetStartPhase Return the start phase
** @nam4rule GetScore Return the score
** @nam4rule GetPvalue Return the p-value
**
** @argrule * pe [const EnsPPredictionexon] Prediction Exon
**
** @valrule Adaptor [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule StartPhase [ajint] Start phase
** @valrule Score [double] Score
** @valrule Pvalue [double] P-value
**
** @fcategory use
******************************************************************************/




/* @func ensPredictionexonGetAdaptor ******************************************
**
** Get the Ensembl Prediction Exon Adaptor element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictionexonadaptor ensPredictionexonGetAdaptor(
    const EnsPPredictionexon pe)
{
    if(!pe)
	return NULL;
    
    return pe->Adaptor;
}




/* @func ensPredictionexonGetIdentifier ***************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensPredictionexonGetIdentifier(const EnsPPredictionexon pe)
{
    if(!pe)
	return 0;
    
    return pe->Identifier;
}




/* @func ensPredictionexonGetFeature ******************************************
**
** Get the Ensembl Feature element of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensPredictionexonGetFeature(const EnsPPredictionexon pe)
{
    if(!pe)
	return NULL;
    
    return pe->Feature;
}




/* @func ensPredictionexonGetStartPhase ***************************************
**
** Get the start phase element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Predictionexon::phase
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajint] Start phase
** @@
**
** Get or set the phase of the Exon, which tells the translation machinery,
** which makes a peptide from the DNA, where to start.
**
** The Ensembl phase convention can be thought of as "the number of bases of
** the first codon which are on the previous exon". It is therefore 0, 1 or 2
** or -1 if the exon is non-coding. In ASCII art, with alternate codons
** represented by '###' and '+++':
**
**       Previous Exon   Intron   This Exon
**    ...-------------            -------------...
**
**    5'                    Phase                3'
**    ...#+++###+++###          0 +++###+++###+...
**    ...+++###+++###+          1 ++###+++###++...
**    ...++###+++###++          2 +###+++###+++...
**
** Here is another explanation from Ewan:
**
** Phase means the place where the intron lands inside the codon - 0 between
** codons, 1 between the 1st and second base, 2 between the second and 3rd
** base. Exons therefore have a start phase and an end phase, but introns have
** just one phase.
******************************************************************************/

ajint ensPredictionexonGetStartPhase(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0;
    
    return pe->StartPhase;
}




/* @func ensPredictionexonGetScore ********************************************
**
** Get the score element of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [double] Score
** @@
******************************************************************************/

double ensPredictionexonGetScore(const EnsPPredictionexon pe)
{
    if(!pe)
	return 0.0;
    
    return pe->Score;
}




/* @func ensPredictionexonGetPvalue *******************************************
**
** Get the p-value element of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [double] P-value
** @@
******************************************************************************/

double ensPredictionexonGetPvalue(const EnsPPredictionexon pe)
{
    if(!pe)
	return 0.0;
    
    return pe->Pvalue;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
** @fnote None
**
** @nam3rule Set Set one element of a Prediction Exon
** @nam4rule SetAdaptor Set the Ensembl Prediction Exon Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetStartPhase Set the start phase
** @nam4rule SetScore Set the score
** @nam4rule SetPvalue Set the p-value
**
** @argrule * pe [EnsPPredictionexon] Ensembl Prediction Exon object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensPrediction<ExonSetAdaptor *****************************************
**
** Set the Ensembl Prediction Exon Adaptor element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] adaptor [EnsPPredictionexonadaptor] Ensembl Prediction
**                                                Exon Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetAdaptor(EnsPPredictionexon pe,
                                   EnsPPredictionexonadaptor adaptor)
{
    if(!pe)
        return ajFalse;
    
    pe->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensPredictionexonSetIdentifier ***************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetIdentifier(EnsPPredictionexon pe, ajuint identifier)
{
    if(!pe)
        return ajFalse;
    
    pe->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensPredictionexonSetFeature ******************************************
**
** Set the Ensembl Feature element of an Ensembl Prediction Exon.
**
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetFeature(EnsPPredictionexon pe, EnsPFeature feature)
{
#if AJFALSE
    AjIList iter = NULL;
    
    EnsPBasealignfeature baf  = NULL;
    EnsPBasealignfeature nbaf = NULL;
    
    EnsPSlice eslice = NULL;
#endif

    /*
     ajDebug("ensPredictionexonSetFeature\n"
	     "  pe %p\n"
	     "  feature %p\n",
	     pe,
	     feature);
     
     ensPredictionexonTrace(pe, 1);
     
     ensFeatureTrace(feature, 1);
     */
    
    if(!pe)
	return ajFalse;
    
    if(!feature)
	return ajFalse;
    
    /* Replace the current Feature. */
    
    if(pe->Feature)
	ensFeatureDel(&(pe->Feature));
    
    pe->Feature = ensFeatureNewRef(feature);
    
#if AJFALSE
    
    /* TODO: Resolve this! */
    
    /* Clear the sequence cache. */
    
    ajStrDel(&(pe->SequenceCache));
    
    /* Clear the Exon coordinate cache. */
    
    exonCoordinatesClear(exon);
    
    /*
     ** FIXME: Setting a Feature, which may be based on another Slice requires
     ** transferring of Supporting Features to the new Feature Slice.
     ** The Perl API does not implement this, it only deletes the Sequence
     ** cache, but it should.
     */
    
    /* Transfer Base Align Features onto the new Feature Slice. */
    
    if(!exon->Supportingfeatures)
	return ajTrue;
    
    eslice = ensFeatureGetSlice(exon->Feature);
    
    iter = ajListIterNew(exon->Supportingfeatures);
    
    while(!ajListIterDone(iter))
    {
	baf = (EnsPBasealignfeature) ajListIterGet(iter);
	
	ajListIterRemove(iter);
	
	nbaf = ensBasealignfeatureTransfer(baf, eslice);
	
	if(!nbaf)
	{
	    ajDebug("ensExonSetFeature could not transfer Base Align Feature "
		    "onto new Ensembl Feature Slice.");
	    
	    ensBasealignfeatureTrace(baf, 1);
	}
	
	ajListIterInsert(iter, (void *) nbaf);
	
	/* Advance the AJAX List Iterator after the insert. */
	
	(void) ajListIterGet(iter);
	
	ensBasealignfeatureDel(&baf);
    }
    
    ajListIterDel(&iter);
    
#endif
    
    return ajTrue;
}




/* @func ensPredictionexonSetStartPhase ***************************************
**
** Set the start phase element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Predictionexon::phase
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] sphase [ajint] Start phase
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetStartPhase(EnsPPredictionexon pe, ajint sphase)
{
    if(!pe)
        return ajFalse;
    
    pe->StartPhase = sphase;
    
    return ajTrue;
}




/* @func ensPredictionexonSetScore ********************************************
**
** Set the score element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Predictionexon::score
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] score [double] Score
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetScore(EnsPPredictionexon pe, double score)
{
    if(!pe)
        return ajFalse;
    
    pe->Score = score;
    
    return ajTrue;
}




/* @func ensPredictionexonSetPvalue *******************************************
**
** Set the p-value element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Predictionexon::p_value
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] pvalue [double] P-value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonSetPvalue(EnsPPredictionexon pe, double pvalue)
{
    if(!pe)
        return ajFalse;
    
    pe->Pvalue = pvalue;
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Prediction Exon object.
**
** @fdata [EnsPPredictionexon]
** @nam3rule Trace Report Ensembl Prediction Exon elements to debug file
**
** @argrule Trace pe [const EnsPPredictionexon] Ensembl Prediction Exon
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictionexonTrace ***********************************************
**
** Trace an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonTrace(const EnsPPredictionexon pe, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!pe)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensPredictionexonTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Feature %p\n"
	    "%S  StartPhase %d\n"
	    "%S  Score %f\n"
	    "%S  P-value %f\n",
	    indent, pe,
	    indent, pe->Use,
	    indent, pe->Identifier,
	    indent, pe->Adaptor,
	    indent, pe->Feature,
	    indent, pe->StartPhase,
	    indent, pe->Score,
	    indent, pe->Pvalue);
    
    ensFeatureTrace(pe->Feature, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensPredictionexonGetMemSize ******************************************
**
** Get the memory size in bytes of an Ensembl Prediction Exon.
**
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensPredictionexonGetMemSize(const EnsPPredictionexon pe)
{
    ajuint size = 0;
    
    if(!pe)
	return 0;
    
    size += (ajuint) sizeof (EnsOPredictionexon);
    
    size += ensFeatureGetMemSize(pe->Feature);
    
    return size;
}




/* @func ensPredictionexonGetEndPhase *****************************************
**
** Get the end phase element of an Ensembl Prediction Exon.
**
** @cc Bio::EnsEMBL::Predictionexon::end_phase
** @param [r] pe [const EnsPPredictionexon] Ensembl Prediction Exon
**
** @return [ajint] End phase
** @@
******************************************************************************/

ajint ensPredictionexonGetEndPhase(const EnsPPredictionexon pe)
{
    if(!pe)
        return 0;
    
    return (pe->StartPhase + ensFeatureGetLength(pe->Feature)) % 3;
}




/* @func ensPredictionexonTransform *******************************************
**
** Transform an Ensembl Prediction Exon into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Predictionexon::transform
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon in the defined
**                              Ensembl Coordinate System or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonTransform(EnsPPredictionexon pe,
                                              const AjPStr csname,
                                              const AjPStr csversion)
{
    EnsPPredictionexon npe = NULL;
    EnsPFeature nfeature   = NULL;
    
    if(!pe)
	return NULL;
    
    if(!csname)
	return NULL;
    
    if(!csversion)
	return NULL;
    
    nfeature = ensFeatureTransform(pe->Feature, csname, csversion);
    
    if(!nfeature)
	return NULL;
    
    npe = ensPredictionexonNewObj(pe);
    
    ensPredictionexonSetFeature(npe, nfeature);
    
    /*
    ** NOTE: The Exon-internal supporting features have already been
    ** transfered to the new Slice and the sequence cache has already been
    ** cleared by the ensExonSetFeature function.
    */
    
    /*
    ** FIXME: Clear the SequenceCache, which does not exist yet, in
    ** ensPredictionexonSetFeature.
    */
    
    return npe;
}




/* @func ensPredictionexonTransfer ********************************************
**
** Transfer an Ensembl Prediction Exon onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Predictionexon::transfer
** @param [u] pe [EnsPPredictionexon] Ensembl Prediction Exon
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPPredictionexon] Ensembl Prediction Exon on the defined
**                              Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPPredictionexon ensPredictionexonTransfer(EnsPPredictionexon pe,
                                             EnsPSlice slice)
{
    EnsPPredictionexon npe = NULL;
    EnsPFeature newfeature = NULL;
    
    if(!pe)
	return NULL;
    
    if(!slice)
	return NULL;
    
    newfeature = ensFeatureTransfer(pe->Feature, slice);
    
    if(!newfeature)
	return NULL;
    
    npe = ensPredictionexonNewObj(pe);
    
    ensPredictionexonSetFeature(npe, newfeature);
    
    ensFeatureDel(&newfeature);
    
    return npe;
}




/* @datasection [EnsPPredictionexonadaptor] Prediction Exon Adaptor ***********
**
** Functions for manipulating Ensembl Prediction Exon Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Predictionexonadaptor CVS Revision: 1.16
**
** @nam2rule Predictionexonadaptor
**
******************************************************************************/

static const char *predictionExonadaptorTables[] =
{
    "prediction_exon",
    NULL
};




static const char *predictionExonadaptorColumns[] =
{
    "prediction_exon.prediction_exon_id",
    "prediction_exon.seq_region_id",
    "prediction_exon.seq_region_start",
    "prediction_exon.seq_region_end",
    "prediction_exon.seq_region_strand",
    "prediction_exon.start_phase",
    "prediction_exon.score",
    "prediction_exon.p_value",
    NULL
};




static EnsOBaseadaptorLeftJoin predictionExonadaptorLeftJoin[] =
{
    {NULL, NULL}
};




static const char *predictionExonadaptorDefaultCondition = NULL;

static const char *predictionExonadaptorFinalCondition =
"ORDER BY "
"prediction_exon.prediction_transcript_id, prediction_exon.exon_rank";




/* @funcstatic predictionExonadaptorFetchAllBySQL *****************************
**
** Fetch all Ensembl Prediction Exon objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] pes [AjPList] AJAX List of Ensembl Prediction Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool predictionExonadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                 const AjPStr statement,
                                                 EnsPAssemblymapper am,
                                                 EnsPSlice slice,
                                                 AjPList pes)
{
    double score  = 0.0;
    double pvalue = 0.0;
    
    ajint sphase = 0;
    /* ajint ephase = 0; */ /* unused */
    
    ajuint identifier = 0;
    
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
    
    EnsPAssemblymapperadaptor ama = NULL;
    
    EnsPCoordsystemadaptor csa = NULL;
    
    EnsPPredictionexon pe         = NULL;
    EnsPPredictionexonadaptor pea = NULL;
    
    EnsPFeature feature = NULL;
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
    /*
     ajDebug("predictionExonadaptorFetchAllBySQL\n"
	     "  dba %p\n"
	     "  statement %p\n"
	     "  am %p\n"
	     "  slice %p\n"
	     "  pes %p\n",
	     dba,
	     statement,
	     am,
	     slice,
	     pes);
     */
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!pes)
	return ajFalse;
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    pea = ensRegistryGetPredictionexonadaptor(dba);
    
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
	sphase     = 0;
	score      = 0.0;
	pvalue     = 0.0;
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToInt(sqlr, &sphase);
	ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToDouble(sqlr, &pvalue);
	
	/* Need to get the internal Ensembl Sequence Region identifier. */
	
	srid = ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);
	
	/*
	** Since the Ensembl SQL schema defines Sequence Region start and end
	** coordinates as unsigned integers for all Features, the range needs
	** checking.
	*/
	
	if(srstart <= INT_MAX)
	    slstart = (ajint) srstart;
	else
	    ajFatal("predictionExonadaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("predictionExonadaptorFetchAllBySQL got a "
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
	
	if((!am) &&
	    slice &&
	    (!ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
				   ensSliceGetCoordsystem(srslice))))
	    am = ensAssemblymapperadaptorFetchByCoordsystems(
                ama,
                ensSliceGetCoordsystem(slice),
                ensSliceGetCoordsystem(srslice));
	
	/*
        ** FIXME: Should we have an ensAsemblyMapperadaptorFetchBySlices
        ** function?
        */
	
	/*
	** Remap the Feature coordinates to another Ensembl Coordinate System
	** if an Ensembl Assembly Mapper is defined at this point.
	*/
	
	if(am)
	{
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(srslice),
				     slstart,
				     slend,
				     slstrand,
				     mrs);
	    
	    /*
	    ** The ensAssemblymapperFastMap function returns at best one
	    ** Ensembl Mapper Result.
	    */
	    
	    ajListPop(mrs, (void **) &mr);
	    
	    /*
	    ** Skip Features that map to gaps or
	    ** Coordinate System boundaries.
	    */
	    
	    if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
	    {
		/* Load the next Feature but destroy first! */
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		ensMapperresultDel(&mr);
		
		continue;
	    }
	    
	    srid = ensMapperresultGetObjectIdentifier(mr);
	    
	    slstart = ensMapperresultGetStart(mr);
	    
	    slend = ensMapperresultGetEnd(mr);
	    
	    slstrand = ensMapperresultGetStrand(mr);
	    
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
	    
	    if(ensSliceGetLength(slice) <= INT_MAX)
		sllength = (ajint) ensSliceGetLength(slice);
	    else
		ajFatal("exonAdaptorFetchAllBySQL got a Slice, "
			"which length (%u) exceeds the "
			"maximum integer limit (%d).",
			ensSliceGetLength(slice), INT_MAX);
	    
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
	    ** Throw away Features off the end of the requested Slice or on
	    ** any other than the requested Slice.
	    */
	    
	    if((slend < 1) ||
		(slstart > sllength) ||
		(srid != ensSliceGetSeqregionIdentifier(slice)))
	    {
		/* Load the next Feature but destroy first! */
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		continue;
	    }
	    
	    /* Delete the Sequence Region Slice and set the requested Slice. */
	    
	    ensSliceDel(&srslice);
	    
	    srslice = ensSliceNewRef(slice);
	}
	
	/* Finally, create a new Ensembl Exon. */
	
	feature = ensFeatureNewS((EnsPAnalysis) NULL,
				 srslice,
				 slstart,
				 slend,
				 slstrand);
	
	pe = ensPredictionexonNew(pea,
				  identifier,
				  feature,
				  sphase,
				  score,
				  pvalue);
	
	ajListPushAppend(pes, (void *) pe);
	
	ensFeatureDel(&feature);
	
	ensAssemblymapperDel(&am);
	
	ensSliceDel(&srslice);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @funcstatic predictionExonadaptorCacheReference ****************************
**
** Wrapper function to reference an Ensembl Prediction Exon from an
** Ensembl Cache.
**
** @param [r] value [void *] Ensembl Prediction Exon
**
** @return [void *] Ensembl Prediction Exon or NULL
** @@
******************************************************************************/

static void *predictionExonadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensPredictionexonNewRef((EnsPPredictionexon) value);
}




/* @funcstatic predictionExonadaptorCacheDelete *******************************
**
** Wrapper function to delete an Ensembl Prediction Exon from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Prediction Exon address
**
** @return [void]
** @@
******************************************************************************/

static void predictionExonadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensPredictionexonDel((EnsPPredictionexon *) value);
    
    return;
}




/* @funcstatic predictionExonadaptorCacheSize *********************************
**
** Wrapper function to determine the memory size of an Ensembl Prediction Exon
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Prediction Exon
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

static ajuint predictionExonadaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensPredictionexonGetMemSize((const EnsPPredictionexon) value);
}




/* @funcstatic predictionExonadaptorGetFeature ********************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Prediction Exon
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Prediction Exon
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature predictionExonadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensPredictionexonGetFeature((const EnsPPredictionexon) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Exon Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Exon Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictionexonadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPPredictionexonadaptor] Ensembl Prediction
**                                                 Exon Adaptor
** @argrule Ref object [EnsPPredictionexonadaptor] Ensembl Prediction
**                                                 Exon Adaptor
**
** @valrule * [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensPredictionexonadaptorNew ******************************************
**
** Default Ensembl Prediction Exon Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictionexonadaptor ensPredictionexonadaptorNew(EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(dba,
				predictionExonadaptorTables,
				predictionExonadaptorColumns,
				predictionExonadaptorLeftJoin,
				predictionExonadaptorDefaultCondition,
				predictionExonadaptorFinalCondition,
				predictionExonadaptorFetchAllBySQL,
                                 /* Fread */
				(void * (*)(const void *key)) NULL,
				predictionExonadaptorCacheReference,
                                 /* Fwrite */
				(AjBool (*)(const void* value)) NULL,
				predictionExonadaptorCacheDelete,
				predictionExonadaptorCacheSize,
				predictionExonadaptorGetFeature,
				"Predictionexon");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Prediction Exon Adaptor.
**
** @fdata [EnsPPredictionexonadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Prediction Exon Adaptor object
**
** @argrule * Padaptor [EnsPPredictionexonadaptor*] Ensembl Prediction
**                                                  Exon Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictionexonadaptorDel ******************************************
**
** Default destructor for an Ensembl Prediction Exon Adaptor.
**
** @param [d] Padaptor [EnsPPredictionexonadaptor*] Ensembl Prediction
**                                                  Exon Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictionexonadaptorDel(EnsPPredictionexonadaptor *Padaptor)
{
    if(!Padaptor)
	return;
    
    ensFeatureadaptorDel(Padaptor);
    
    return;
}




/* @func ensPredictionexonadaptorFetchAllByPredictiontranscript ***************
**
** Fetch all Ensembl Prediction Exons via an Ensembl Prediction Transcript.
**
** The caller is responsible for deleting the Ensembl Prediction Exons before
** deleting the AJAX List.
**
** @param [r] pea [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] pes [AjPList] AJAX List of Ensembl Prediction Exons
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictionexonadaptorFetchAllByPredictiontranscript(
    EnsPPredictionexonadaptor pea,
    const EnsPPredictiontranscript pt,
    AjPList pes)
{
    AjIList iter = NULL;
    
    AjPStr constraint = NULL;
    
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPPredictionexon pe = NULL;
    
    EnsPFeature efeature = NULL;
    EnsPFeature tfeature = NULL;
    
    EnsPSlice eslice = NULL;
    EnsPSlice tslice = NULL;

    EnsPSliceadaptor sa = NULL;
    
    /*
     ajDebug("ensPredictionexonadaptorFetchAllByPredictiontranscript\n"
	     "  pea %p\n"
	     "  pt %p\n"
	     "  pes %p\n",
	     pea,
	     pt,
	     pes);
     
     ensPredictiontranscriptTrace(pt, 1);
     */
    
    if(!pea)
	return ajFalse;
    
    if(!pt)
	return ajFalse;
    
    if(!pes)
	return ajFalse;
    
    tfeature = ensPredictiontranscriptGetFeature(pt);
    
    tslice = ensFeatureGetSlice(tfeature);
    
    if(!tslice)
    {
	ajDebug("ensPredictionexonadaptorFetchAllByPredictiontranscript "
		"cannot fetch Prediction Exons for Prediction Transcript "
		"without Slice.\n");
	
	return ajFalse;
    }
    
    dba = ensFeatureadaptorGetDatabaseadaptor(pea);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    /*
    ** Get a Slice that spans just this Prediction Transcript to place
    ** Prediction Exons on them.
    */
    
    ensSliceadaptorFetchByFeature(sa, tfeature, 0, &eslice);
    
    constraint =
	ajFmtStr("prediction_exon.prediction_transcript_id = %u",
		 ensPredictiontranscriptGetIdentifier(pt));
    
    ensFeatureadaptorFetchAllBySliceConstraint(pea,
					       eslice,
					       constraint,
					       (const AjPStr) NULL,
					       pes);
    
    /* Remap Exon coordinates if neccessary. */
    
    /*
    ** FIXME: What is the point of retrieving Exons first on a Slice that
    ** spans the Transcript only and then map them to the Slice the Transcript
    ** is on? Could this have something to do with haplotypes or PAR regions?
    */
    
    if(!ensSliceMatch(eslice, tslice))
    {
	iter = ajListIterNew(pes);
	
	while(!ajListIterDone(iter))
	{
	    pe = (EnsPPredictionexon) ajListIterGet(iter);
	    
	    efeature = ensFeatureTransfer(pe->Feature, tslice);
	    
	    ensPredictionexonSetFeature(pe, efeature);
	    
	    ensFeatureDel(&efeature);
	}
	
	ajListIterDel(&iter);	
    }
    
    ajStrDel(&constraint);
    
    ensSliceDel(&eslice);
    
    return ajTrue;
}




/* @datasection [EnsPPredictiontranscript] Prediction Transcript **************
**
** Functions for manipulating Ensembl Prediction Transcript objects
**
** @cc Bio::EnsEMBL::Predictiontranscript CVS Revision: 1.43
**
** @nam2rule Predictiontranscript
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Transcript by pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Transcript. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictiontranscript]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @argrule Ref object [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @valrule * [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @fcategory new
******************************************************************************/




/* @func ensPredictiontranscriptNew *******************************************
**
** Default Ensembl Prediction Transcript constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                      Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Predictiontranscript::new
** @param [u] label [AjPStr] Display label
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptNew(
    EnsPPredictiontranscriptadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr label)
{
    EnsPPredictiontranscript pt = NULL;
    
    if(!feature)
        return NULL;
    
    AJNEW0(pt);
    
    pt->Use = 1;
    
    pt->Identifier = identifier;
    
    pt->Adaptor = adaptor;
    
    pt->Feature = ensFeatureNewRef(feature);
    
    if(label)
	pt->DisplayLabel = ajStrNewRef(label);
    
    return pt;
}




/* @func ensPredictiontranscriptNewObj ****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPPredictiontranscript] Ensembl Prediction
**                                                    Transcript
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptNewObj(
    const EnsPPredictiontranscript object)
{
    AjIList iter = NULL;
    
    EnsPPredictionexon pe = NULL;
    
    EnsPPredictiontranscript pt = NULL;
    
    if(!object)
	return NULL;
	
    AJNEW0(pt);
    
    pt->Use = 1;
    
    pt->Identifier = object->Identifier;
    
    pt->Adaptor = object->Adaptor;
    
    pt->Feature = ensFeatureNewRef(object->Feature);
    
    if(object->DisplayLabel)
	pt->DisplayLabel = ajStrNewRef(object->DisplayLabel);
    
    /* Copy the AJAX List of Ensembl Prediction Exons. */
    
    if(object->Predictionexons && ajListGetLength(object->Predictionexons))
    {
	pt->Predictionexons = ajListNew();
	
	iter = ajListIterNew(object->Predictionexons);
	
	while(!ajListIterDone(iter))
	{
	    pe = (EnsPPredictionexon) ajListIterGet(iter);
	    
	    ajListPushAppend(pt->Predictionexons,
			     (void *) ensPredictionexonNewRef(pe));
	}
	
	ajListIterDel(&iter);
    }
    else
	pt->Predictionexons = NULL;
    
    return pt;
}




/* @func ensPredictiontranscriptNewRef ****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPPredictiontranscript] Ensembl Prediction Transcript or NULL
** @@
******************************************************************************/

EnsPPredictiontranscript ensPredictiontranscriptNewRef(
    EnsPPredictiontranscript pt)
{
    if(!pt)
	return NULL;
    
    pt->Use++;
    
    return pt;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Prediction Transcript.
**
** @fdata [EnsPPredictiontranscript]
** @fnote None
**
** @nam3rule Del Destroy (free) a Prediction Transcript object
**
** @argrule * Ppt [EnsPPredictiontranscript*] Ensembl Prediction Transcript
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictiontranscriptDel *******************************************
**
** Default destructor for an Ensembl Prediction Transcript.
**
** @param [d] Ppt [EnsPPredictiontranscript*] Ensembl Prediction Transcript
**                                            address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictiontranscriptDel(EnsPPredictiontranscript *Ppt)
{
    EnsPPredictionexon pe = NULL;
    EnsPPredictiontranscript pthis = NULL;
    
    if(!Ppt)
        return;
    
    if(!*Ppt)
        return;

    pthis = *Ppt;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Ppt = NULL;
	
	return;
    }
    
    ensFeatureDel(&pthis->Feature);
    
    ajStrDel(&pthis->DisplayLabel);
    
    /* Clear and delete the AJAX List of Ensembl Prediction Exons. */
    
    while(ajListPop(pthis->Predictionexons, (void **) &pe))
	ensPredictionexonDel(&pe);
    
    ajListFree(&pthis->Predictionexons);
    
    AJFREE(pthis);

    *Ppt = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
** @fnote None
**
** @nam3rule Get Return Prediction Transcript attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Prediction Transcript Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetDisplayLabel Return the display label
**
** @argrule * pt [const EnsPPredictiontranscript] Prediction Transcript
**
** @valrule Adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                    Transcript Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule DisplayLabel [AjPStr] Display label
**
** @fcategory use
******************************************************************************/




/* @func ensPredictiontranscriptGetAdaptor ************************************
**
** Get the Ensembl Prediction Transcript Adaptor element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                           Transcript Adaptor
** @@
******************************************************************************/

EnsPPredictiontranscriptadaptor ensPredictiontranscriptGetAdaptor(
    const EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;
    
    return pt->Adaptor;
}




/* @func ensPredictiontranscriptGetIdentifier *********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensPredictiontranscriptGetIdentifier(const EnsPPredictiontranscript pt)
{
    if(!pt)
        return 0;
    
    return pt->Identifier;
}




/* @func ensPredictiontranscriptGetFeature ************************************
**
** Get the Ensembl Feature element of an Ensembl Prediction Transcript.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensPredictiontranscriptGetFeature(const EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;
    
    return pt->Feature;
}




/* @func ensPredictiontranscriptGetDisplayLabel *******************************
**
** Get the display label element of an Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Predictiontranscript::display_label
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [AjPStr] Display label
** @@
******************************************************************************/

AjPStr ensPredictiontranscriptGetDisplayLabel(const EnsPPredictiontranscript pt)
{
    if(!pt)
        return NULL;
    
    return pt->DisplayLabel;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
** @fnote None
**
** @nam3rule Set Set one element of a Prediction Transcript
** @nam4rule SetAdaptor Set the Ensembl Prediction Transcript Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetDisplayLabel Set the display label
**
** @argrule * pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**                                          object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensPredictiontranscriptSetAdaptor ************************************
**
** Set the Ensembl Prediction Transcript Adaptor element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [r] adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                      Transcript Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetAdaptor(
    EnsPPredictiontranscript pt,
    EnsPPredictiontranscriptadaptor adaptor)
{
    if(!pt)
        return ajFalse;
    
    pt->Adaptor = adaptor;
    
    return ajTrue;
}




/* @func ensPredictiontranscriptSetIdentifier *********************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetIdentifier(EnsPPredictiontranscript pt,
                                            ajuint identifier)
{
    if(!pt)
        return ajFalse;
    
    pt->Identifier = identifier;
    
    return ajTrue;
}




/* @func ensPredictiontranscriptSetFeature ************************************
**
** Set the Ensembl Feature element of an Ensembl Prediction Transcript.
**
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetFeature(EnsPPredictiontranscript pt,
                                         EnsPFeature feature)
{
    if(!pt)
        return ajFalse;
    
    ensFeatureDel(&pt->Feature);
    
    pt->Feature = ensFeatureNewRef(feature);
    
    return ajTrue;
}




/* @func ensPredictiontranscriptSetDisplayLabel *******************************
**
** Set the display label element of an Ensembl Prediction Transcript.
**
** @cc Bio::EnsEMBL::Predictiontranscript::display_label
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [u] label [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptSetDisplayLabel(EnsPPredictiontranscript pt,
                                              AjPStr label)
{
    if(!pt)
        return ajFalse;
    
    ajStrDel(&pt->DisplayLabel);
    
    pt->DisplayLabel = ajStrNewRef(label);
    
    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Prediction Transcript object.
**
** @fdata [EnsPPredictiontranscript]
** @nam3rule Trace Report Ensembl Prediction Transcript elements to debug file
**
** @argrule Trace pt [const EnsPPredictiontranscript] Ensembl Prediction
**                                                    Transcript
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensPredictiontranscriptTrace *****************************************
**
** Trace an Ensembl Prediction Transcript.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptTrace(const EnsPPredictiontranscript pt,
                                    ajuint level)
{
    AjIList iter = NULL;
    
    AjPStr indent = NULL;
    
    EnsPPredictionexon pe = NULL;
    
    if(!pt)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensPredictiontranscriptTrace %p\n"
	    "%S  Use %u\n"
	    "%S  Identifier %u\n"
	    "%S  Adaptor %p\n"
	    "%S  Feature %p\n"
	    "%S  DisplayLabel '%S'\n"
	    "%S  Predictionexons %p\n",
	    indent, pt,
	    indent, pt->Use,
	    indent, pt->Identifier,
	    indent, pt->Adaptor,
	    indent, pt->Feature,
	    indent, pt->DisplayLabel,
	    indent, pt->Predictionexons);
    
    ensFeatureTrace(pt->Feature, level + 1);
    
    /* Trace the AJAX List of Ensembl Prediction Exons. */
    
    if(pt->Predictionexons)
    {
	ajDebug("%S    AJAX List %p of Ensembl Prediction Exons\n",
		indent, pt->Predictionexons);
	
	iter = ajListIterNewread(pt->Predictionexons);
	
	while(!ajListIterDone(iter))
	{
	    pe = (EnsPPredictionexon) ajListIterGet(iter);
	    
	    ensPredictionexonTrace(pe, level + 2);
	}
	
	ajListIterDel(&iter);
    }
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensPredictiontranscriptGetMemSize ************************************
**
** Get the memory size in bytes of an Ensembl Prediction Transcript.
**
** @param [r] pt [const EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensPredictiontranscriptGetMemSize(const EnsPPredictiontranscript pt)
{
    ajuint size = 0;
    
    AjIList iter = NULL;
    
    EnsPPredictionexon pe = NULL;
    
    if(!pt)
	return 0;
    
    size += (ajuint) sizeof (EnsOPredictiontranscript);
    
    size += ensFeatureGetMemSize(pt->Feature);
    
    if(pt->DisplayLabel)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(pt->DisplayLabel);
    }
    
    /* Summarise the AJAX List of Ensembl Prediction Exons. */
    
    if(pt->Predictionexons)
    {
	size += (ajuint) sizeof (AjOList);
	
	iter = ajListIterNewread(pt->Predictionexons);
	
	while(!ajListIterDone(iter))
	{
	    pe = (EnsPPredictionexon) ajListIterGet(iter);
	    
	    size += ensPredictionexonGetMemSize(pe);
	}
	
	ajListIterDel(&iter);
    }
    
    return size;
}




/* @func ensPredictiontranscriptGetPredictionexons ****************************
**
** Get all Ensembl Prediction Exons of an Ensembl Prediction Transcript.
**
** This is not a simple accessor function, since it will attempt fetching the
** Prediction Exons from the Ensembl Core database associated with the
** Prediction Transcript Adaptor.
**
** @cc Bio::EnsEMBL::Predictiontranscript::get_all_Exons
** @param [u] pt [EnsPPredictiontranscript] Ensembl Prediction Transcript
**
** @return [const AjPList] AJAX List of Ensembl Prediction Exons
** @@
******************************************************************************/

const AjPList ensPredictiontranscriptGetExons(EnsPPredictiontranscript pt)
{
    EnsPDatabaseadaptor dba = NULL;
    
    EnsPPredictionexonadaptor pea = NULL;
    
    if(!pt)
	return NULL;
    
    if(pt->Predictionexons)
	return pt->Predictionexons;
    
    if(!pt->Adaptor)
    {
	ajDebug("ensPredictiontranscriptGetPredictionexons cannot fetch "
		"Ensembl Prediction Exons for a Prediction Transcript without "
		"a Prediction Transcript Adaptor.\n");
	
	return NULL;
    }
    
    dba = ensFeatureadaptorGetDatabaseadaptor(pt->Adaptor);
    
    pea = ensRegistryGetPredictionexonadaptor(dba);
    
    pt->Predictionexons = ajListNew();
    
    ensPredictionexonadaptorFetchAllByPredictiontranscript(pea,
							   pt,
							   pt->Predictionexons);
    
    return pt->Predictionexons;
}




/* TODO: Some Prediction Transcript methods are missing! */

/* @datasection [EnsPPredictiontranscriptadaptor] Prediction Transcript Adaptor
**
** Functions for manipulating Ensembl Prediction Transcript Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Predictiontranscriptadaptor CVS Revision: 1.48
**
** @nam2rule Predictiontranscriptadaptor
**
******************************************************************************/

static const char *predictionTranscriptadaptorTables[] =
{
    "prediction_transcript",
    NULL
};




static const char *predictionTranscriptadaptorColumns[] =
{
    "prediction_transcript.prediction_transcript_id",
    "prediction_transcript.seq_region_id",
    "prediction_transcript.seq_region_start",
    "prediction_transcript.seq_region_end",
    "prediction_transcript.seq_region_strand",
    "prediction_transcript.analysis_id",
    "prediction_transcript.display_label",
    NULL
};




static EnsOBaseadaptorLeftJoin predictionTranscriptadaptorLeftJoin[] =
{
    {NULL, NULL}
};




static const char *predictionTranscriptadaptorDefaultCondition = NULL;

static const char *predictionTranscriptadaptorFinalCondition = NULL;




/* @funcstatic predictionTranscriptadaptorFetchAllBySQL ***********************
**
** Fetch all Ensembl Prediction Transcript objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] pts [AjPList] AJAX List of Ensembl Prediction Transcripts
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool predictionTranscriptadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                                       const AjPStr statement,
                                                       EnsPAssemblymapper am,
                                                       EnsPSlice slice,
                                                       AjPList pts)
{
    ajuint identifier = 0;
    ajuint analysisid = 0;
    
    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;
    
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
    
    EnsPPredictiontranscript pt = NULL;
    EnsPPredictiontranscriptadaptor pta = NULL;
    
    EnsPMapperresult mr = NULL;
    
    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;
    
    /*
     ajDebug("predictionTranscriptadaptorFetchAllBySQL\n"
	     "  dba %p\n"
	     "  statement %p\n"
	     "  am %p\n"
	     "  slice %p\n"
	     "  pts %p\n",
	     dba,
	     statement,
	     am,
	     slice,
	     pts);
     */
    
    if(!dba)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!pts)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    pta = ensRegistryGetPredictiontranscriptadaptor(dba);
    
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
	analysisid = 0;
	label      = ajStrNew();
	
	sqlr = ajSqlrowiterGet(sqli);
	
	ajSqlcolumnToUint(sqlr, &identifier);
	ajSqlcolumnToUint(sqlr, &srid);
	ajSqlcolumnToUint(sqlr, &srstart);
	ajSqlcolumnToUint(sqlr, &srend);
	ajSqlcolumnToInt(sqlr, &srstrand);
	ajSqlcolumnToUint(sqlr, &analysisid);
	ajSqlcolumnToStr(sqlr, &label);
	
	/* Need to get the internal Ensembl Sequence Region identifier. */
	
	srid = ensCoordsystemadaptorGetInternalSeqregionIdentifier(csa, srid);
	
	/*
	** Since the Ensembl SQL schema defines Sequence Region start and end
	** coordinates as unsigned integers for all Features, the range needs
	** checking.
	*/
	
	if(srstart <= INT_MAX)
	    slstart = (ajint) srstart;
	else
	    ajFatal("predictionTranscriptadaptorFetchAllBySQL got a "
		    "Sequence Region start coordinate (%u) outside the "
		    "maximum integer limit (%d).",
		    srstart, INT_MAX);
	
	if(srend <= INT_MAX)
	    slend = (ajint) srend;
	else
	    ajFatal("predictionTranscriptadaptorFetchAllBySQL got a "
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
	
	if((!am) &&
	    slice &&
	    (!ensCoordsystemMatch(ensSliceGetCoordsystem(slice),
				   ensSliceGetCoordsystem(srslice))))
	    am =
		ensAssemblymapperadaptorFetchByCoordsystems(
                    ama,
                    ensSliceGetCoordsystem(slice),
                    ensSliceGetCoordsystem(srslice));
	
	/*
	** Remap the Feature coordinates to another Ensembl Coordinate System
	** if an Ensembl Assembly Mapper is defined at this point.
	*/
	
	if(am)
	{
	    ensAssemblymapperFastMap(am,
				     ensSliceGetSeqregion(srslice),
				     slstart,
				     slend,
				     slstrand,
				     mrs);
	    
	    /*
	    ** The ensAssemblymapperFastMap function returns at best one
	    ** Ensembl Mapper Result.
	    */
	    
	    ajListPop(mrs, (void **) &mr);
	    
	    /*
	    ** Skip Features that map to gaps or
	    ** Coordinate System boundaries.
	    */
	    
	    if(ensMapperresultGetType(mr) != ensEMapperresultCoordinate)
	    {
		/* Load the next Feature but destroy first! */
		
		ajStrDel(&label);
		
		ensSliceDel(&srslice);
		
		ensAssemblymapperDel(&am);
		
		ensMapperresultDel(&mr);
		
		continue;
	    }
	    
	    srid = ensMapperresultGetObjectIdentifier(mr);
	    
	    slstart = ensMapperresultGetStart(mr);
	    
	    slend = ensMapperresultGetEnd(mr);
	    
	    slstrand = ensMapperresultGetStrand(mr);
	    
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
	    
	    if(ensSliceGetLength(slice) <= INT_MAX)
		sllength = (ajint) ensSliceGetLength(slice);
	    else
		ajFatal("transcriptAdaptorFetchAllBySQL got a Slice, "
			"which length (%u) exceeds the "
			"maximum integer limit (%d).",
			ensSliceGetLength(slice), INT_MAX);
	    
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
	    ** Throw away Features off the end of the requested Slice or on
	    ** any other than the requested Slice.
	    */
	    
	    if((slend < 1) ||
		(slstart > sllength) ||
		(srid != ensSliceGetSeqregionIdentifier(slice)))
	    {
		/* Next feature but destroy first! */
		
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
	
	/* Finally, create a new Ensembl Prediction Transcript. */
	
	feature = ensFeatureNewS(analysis,
				 srslice,
				 slstart,
				 slend,
				 slstrand);
	
	pt = ensPredictiontranscriptNew(pta,
					identifier,
					feature,
					label);
	
	ajListPushAppend(pts, (void *) pt);	
	
	ensFeatureDel(&feature);
	
	ajStrDel(&label);
	
	ensAnalysisDel(&analysis);
	
	ensSliceDel(&srslice);
	
	ensAssemblymapperDel(&am);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    ajListFree(&mrs);
    
    return ajTrue;
}




/* @funcstatic predictionTranscriptadaptorCacheReference **********************
**
** Wrapper function to reference an Ensembl Prediction Transcript
** from an Ensembl Cache.
**
** @param [r] value [void *] Ensembl Prediction Transcript
**
** @return [void *] Ensembl Transcript or NULL
** @@
******************************************************************************/

static void *predictionTranscriptadaptorCacheReference(void *value)
{
    if(!value)
	return NULL;
    
    return (void *) ensPredictiontranscriptNewRef((EnsPPredictiontranscript)
						  value);
}




/* @funcstatic predictionTranscriptadaptorCacheDelete *************************
**
** Wrapper function to delete an Ensembl Prediction Transcript
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Prediction Transcript address
**
** @return [void]
** @@
******************************************************************************/

static void predictionTranscriptadaptorCacheDelete(void **value)
{
    if(!value)
	return;
    
    ensPredictiontranscriptDel((EnsPPredictiontranscript *) value);
    
    return;
}




/* @funcstatic predictionTranscriptadaptorCacheSize ***************************
**
** Wrapper function to determine the memory size of an
** Ensembl Prediction Transcript via an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Prediction Transcript
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

static ajuint predictionTranscriptadaptorCacheSize(const void *value)
{
    if(!value)
	return 0;
    
    return ensPredictiontranscriptGetMemSize((const EnsPPredictiontranscript)
					     value);
}




/* @funcstatic predictionTranscriptadaptorGetFeature **************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Prediction Transcript from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Prediction Transcript
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature predictionTranscriptadaptorGetFeature(const void *value)
{
    if(!value)
	return NULL;
    
    return ensPredictiontranscriptGetFeature((const EnsPPredictiontranscript)
					     value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Prediction Transcript Adaptor by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Prediction Transcript Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPPredictiontranscriptadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                       Transcript Adaptor
** @argrule Ref object [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                       Transcript Adaptor
**
** @valrule * [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                              Transcript Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensPredictiontranscriptadaptorNew ************************************
**
** Default Ensembl Prediction Transcript Adaptor constructor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                           Transcript Adaptor or NULL
** @@
******************************************************************************/

EnsPPredictiontranscriptadaptor ensPredictiontranscriptadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
	return NULL;
    
    return
	ensFeatureadaptorNew(dba,
			     predictionTranscriptadaptorTables,
			     predictionTranscriptadaptorColumns,
			     predictionTranscriptadaptorLeftJoin,
			     predictionTranscriptadaptorDefaultCondition,
			     predictionTranscriptadaptorFinalCondition,
			     predictionTranscriptadaptorFetchAllBySQL,
			     (void* (*)(const void* key)) NULL, /* Fread */
			     predictionTranscriptadaptorCacheReference,
			     (AjBool (*)(const void* value)) NULL, /* Fwrite */
			     predictionTranscriptadaptorCacheDelete,
			     predictionTranscriptadaptorCacheSize,
			     predictionTranscriptadaptorGetFeature,
			     "Predictiontranscript");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Prediction Transcript Adaptor.
**
** @fdata [EnsPPredictiontranscriptadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Prediction Transcript Adaptor
**               object.
**
** @argrule * Padaptor [EnsPPredictiontranscriptadaptor*] Ensembl Prediction
**                                                        Transcript Adaptor
**                                                        object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensPredictiontranscriptadaptorDel ************************************
**
** Default destructor for an Ensembl Prediction Transcript Adaptor.
**
** @param [d] Padaptor [EnsPPredictiontranscriptadaptor*] Ensembl Prediction
**                                                        Transcript Adaptor
**                                                        address
**
** @return [void]
** @@
******************************************************************************/

void ensPredictiontranscriptadaptorDel(
    EnsPPredictiontranscriptadaptor *Padaptor)
{
    if(!Padaptor)
	return;
    
    ensFeatureadaptorDel(Padaptor);
    
    return;
}




/* @func ensPredictiontranscriptadaptorFetchByIdentifier **********************
**
** Fetch an Ensembl Prediction Transcript via its SQL database-internal
** identifier.
** The caller is responsible for deleting the Ensembl Prediction Transcript.
**
** @param [r] adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                      Transcript Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ppt [EnsPPredictiontranscript*] Ensembl Prediction
**                                             Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptadaptorFetchByIdentifier(
    EnsPPredictiontranscriptadaptor adaptor,
    ajuint identifier,
    EnsPPredictiontranscript *Ppt)
{
    EnsPBaseadaptor ba = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!identifier)
	return ajFalse;
    
    if(!Ppt)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor);
    
    *Ppt = (EnsPPredictiontranscript)
	ensBaseadaptorFetchByIdentifier(ba, identifier);
    
    return ajTrue;
}




/* @func ensPredictiontranscriptadaptorFetchByStableIdentifier ****************
**
** Fetch an Ensembl Prediction Transcript via its stable identifier and
** version. This method is called FetchByStableIdentifier for polymorphism with
** the Ensembl Transcript Adaptor. Prediction Transcript display labels are
** not necessarily stable in that the same identifier may be reused for a
** completely different Prediction Transcript in a subsequent database release.
**
** The caller is responsible for deleting the Ensembl Prediction Transcript.
**
** @param [r] adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                      Transcript Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [wP] Ppt [EnsPPredictiontranscript*] Ensembl Prediction
**                                             Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensPredictiontranscriptadaptorFetchByStableIdentifier(
    EnsPPredictiontranscriptadaptor adaptor,
    const AjPStr stableid,
    EnsPPredictiontranscript *Ppt)
{
    char *txtstableid = NULL;
    
    AjPList pts = NULL;
    
    AjPStr constraint = NULL;
    
    EnsPBaseadaptor ba = NULL;
    
    EnsPPredictiontranscript pt = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!stableid)
	return ajFalse;
    
    if(!Ppt)
	return ajFalse;
    
    ba = ensFeatureadaptorGetBaseadaptor(adaptor);
    
    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);
    
    constraint =
	ajFmtStr("prediction_transcript.display_label = '%s'", txtstableid);
    
    ajCharDel(&txtstableid);
    
    pts = ajListNew();
    
    ensBaseadaptorGenericFetch(ba,
			       constraint,
			       (EnsPAssemblymapper) NULL,
			       (EnsPSlice) NULL,
			       pts);
    
    if(ajListGetLength(pts) > 1)
	ajDebug("ensPredictiontranscriptadaptorFetchByStableIdentifier got "
		"more than one Prediction Transcript for stable identifier "
		"'%S'.\n",
		stableid);
    
    ajListPop(pts, (void **) Ppt);
    
    while(ajListPop(pts, (void **) &pt))
	ensPredictiontranscriptDel(&pt);
    
    ajListFree(&pts);
    
    ajStrDel(&constraint);
    
    return ajTrue;
}

/* @source enstranslation *****************************************************
**
** Ensembl Translation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.65 $
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

#include "ensdatabaseentry.h"
#include "ensexon.h"
#include "enssequenceedit.h"
#include "enstable.h"
#include "enstranscript.h"
#include "enstranslation.h"




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

/* @conststatic translationKProteinfeatureDomainName **************************
**
** Ensembl characterises conserved protein domains from EBI InterPro member
** databases by running algorithms individually. The following Ensembl Analysis
** names are associated with InterPro member databases.
**
******************************************************************************/

static const char *translationKProteinfeatureDomainName[] =
{
    "pfscan",
    "scanprosite",
    "superfamily",
    "pfam",
    "smart",
    "tigrfam",
    "pirsf",
    "prints",
    (const char *) NULL
};




/* @conststatic translationKSequenceeditCode **********************************
**
** Ensembl Sequence Edit objects for Ensembl Translation objects are a sub-set
** of Ensembl Attributes that provide information about post-translational
** modifications of the Translation sequence. Ensembl Attribute objects with
** the following codes are Ensembl Sequence Edit objects on the
** Translation-level.
**
** initial_met:     Set the first amino acid to methionine
** _selenocysteine: Start and end position of a selenocysteine
** amino_acid_sub:  Amino acid substitution mainly for FlyBase
**
******************************************************************************/

static const char *translationKSequenceeditCode[] =
{
    "initial_met",
    "_selenocysteine",
    "amino_acid_sub",
    (const char *) NULL
};




/* @conststatic translationadaptorKTables *************************************
**
** Array of Ensembl Translation Adaptor SQL table names
**
******************************************************************************/

static const char *translationadaptorKTables[] =
{
    "translation",
    (const char *) NULL
};




/* @conststatic translationadaptorKColumns ************************************
**
** Array of Ensembl Translation Adaptor SQL column names
**
******************************************************************************/

static const char *translationadaptorKColumns[] =
{
    "translation.translation_id",
    "translation.transcript_id",
    "translation.seq_start",
    "translation.start_exon_id",
    "translation.seq_end",
    "translation.end_exon_id",
    "translation.stable_id",
    "translation.version",
    "translation.created_date",
    "translation.modified_date",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */

/* #varstatic translationGCache ***********************************************
**
** Upon each instantiation, AJAX Translation objects require a data file
** (EGC.codon table number) to be read. To avoid unnecessary file system
** operations, AJAX Translation objects are kept in a cache.
** AJAX Translation objects returned by ensTranslationCacheGet can
** be used directly and must not be deleted by the caller. Calling ensExit
** will call ensTranslationExit and in turn free all AJAX Translation objects
** from the cache.
**
******************************************************************************/

static AjPTable translationGCache = NULL;




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool translationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList translations);

static AjBool translationadaptorFetchAllbyIdentifiers(
    EnsPTranslationadaptor tla,
    const AjPStr csv,
    const AjPTable canonicalmap,
    AjPTable transcripts);

static AjBool translationadaptorRetrieveAllCanonicalidentifiers(
    EnsPTranslationadaptor tla,
    const AjPStr csv,
    AjPTable canonicalmap);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enstranslation ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
** @nam2rule Translation Functions for manipulating
** Ensembl Translation objects
**
******************************************************************************/




/* @datasection [none] Internals **********************************************
**
** Functions to control Ensembl Translation internals
**
******************************************************************************/




/* @section Initialise ********************************************************
**
** @fdata [none]
**
** @nam3rule Init Initialise Ensembl Translation internals
**
** @valrule * [void]
**
** @fcategory internals
******************************************************************************/




/* @func ensTranslationInit ***************************************************
**
** Initialises Ensembl Translation internals.
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensTranslationInit(void)
{
    translationGCache = ajTableuintNew(0);

    ajTableSetDestroyvalue(translationGCache, (void (*)(void **)) &ajTrnDel);

    return;
}




/* @section Exit **************************************************************
**
** @fdata [none]
**
** @nam3rule Exit Clear and free Ensembl Translation internals
**
** @valrule * [void]
**
** @fcategory internals
******************************************************************************/




/* @func ensTranslationExit ***************************************************
**
** Clear and free Ensembl Translation internals.
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensTranslationExit(void)
{
    if (!translationGCache)
        return;

    ajTableDel(&translationGCache);

    return;
}




/* @section Cache *************************************************************
**
** @fdata [none]
**
** @nam3rule Cache Functions for manipulating the Ensembl Translation Cache
** @nam4rule Get   Get an AJAX Translation from the cache
**
** @argrule Get codontable [ajint] Codon table number
**
** @valrule Get [const AjPTrn] AJAX Translation or NULL
**
** @fcategory internals
******************************************************************************/




/* @func ensTranslationCacheGet ***********************************************
**
** Get an AJAX Translation from the cache.
**
** Upon each instantiation, AJAX Translation objects require a data file
** (EGC.codon table number) to be read. To avoid unnecessary file system
** operations, AJAX Translation objects are kept in a cache.
** AJAX Translation objects returned by ensTranslationCacheGet can
** be used directly and must not be deleted by the caller. Calling ensExit
** will call ensTranslationExit and in turn free all AJAX Translation objects
** from the cache.
**
** @param [r] codontable [ajint] Codon table number
**
** @return [const AjPTrn] AJAX Translation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPTrn ensTranslationCacheGet(ajint codontable)
{
    ajint *Pcodontable = NULL;

    AjPTrn translation = NULL;

    translation = (AjPTrn) ajTableFetchmodV(translationGCache,
                                            (const void *) &codontable);

    if (translation)
        return translation;

    translation = ajTrnNewI(codontable);

    if (translation)
    {
        AJNEW0(Pcodontable);

        *Pcodontable = codontable;

        ajTablePut(translationGCache,
                   (void *) Pcodontable,
                   (void *) translation);
    }

    return translation;
}




/* @datasection [EnsPTranslation] Ensembl Translation *************************
**
** @nam2rule Translation Functions for manipulating
** Ensembl Translation objects
**
** @cc Bio::EnsEMBL::Translation
** @cc CVS Revision: 1.94
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Translation by pointer.
** It is the responsibility of the user to first destroy any previous
** Translation. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPTranslation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy translation [const EnsPTranslation] Ensembl Translation
** @argrule Ini tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini startexon [EnsPExon] Ensembl Exon in which the start coordinate
** @argrule Ini endexon [EnsPExon] Ensembl Exon in which the end coordinate
** @argrule Ini start [ajuint] Start coordinate relative to the start Exon
** @argrule Ini end [ajuint] Start coordinate relative to the end Exon
** @argrule Ini sequence [AjPStr] Translation sequence
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Version
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ref translation [EnsPTranslation] Ensembl Translation
**
** @valrule * [EnsPTranslation] Ensembl Translation or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensTranslationNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [EnsPTranslation] Ensembl Translation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPTranslation ensTranslationNewCpy(const EnsPTranslation translation)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPProteinfeature pf = NULL;

    EnsPTranslation pthis = NULL;

    if (!translation)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = translation->Identifier;
    pthis->Adaptor    = translation->Adaptor;
    pthis->Startexon  = ensExonNewRef(translation->Startexon);
    pthis->Endexon    = ensExonNewRef(translation->Endexon);
    pthis->Start      = translation->Start;
    pthis->End        = translation->End;

    if (translation->DateCreation)
        pthis->DateCreation = ajStrNewRef(translation->DateCreation);

    if (translation->DateModification)
        pthis->DateModification = ajStrNewRef(translation->DateModification);

    if (translation->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(translation->Stableidentifier);

    pthis->Version = translation->Version;

    /* Copy the List of Ensembl Attributes. */

    if (translation->Attributes && ajListGetLength(translation->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(translation->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void *) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* Copy the List of Ensembl Database Entry objects. */

    if (translation->Databaseentries
        && ajListGetLength(translation->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(translation->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ajListPushAppend(pthis->Databaseentries,
                             (void *) ensDatabaseentryNewRef(dbe));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Databaseentries = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Protein Feature objects. */

    if (translation->Proteinfeatures
        && ajListGetLength(translation->Proteinfeatures))
    {
        pthis->Proteinfeatures = ajListNew();

        iter = ajListIterNew(translation->Proteinfeatures);

        while (!ajListIterDone(iter))
        {
            pf = (EnsPProteinfeature) ajListIterGet(iter);

            ajListPushAppend(pthis->Proteinfeatures,
                             (void *) ensProteinfeatureNewRef(pf));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Proteinfeatures = NULL;

    if (translation->Sequence)
        pthis->Sequence = ajStrNewRef(translation->Sequence);

    pthis->TranscriptStart = translation->TranscriptStart;
    pthis->TranscriptEnd   = translation->TranscriptEnd;
    pthis->SliceStart      = translation->SliceStart;
    pthis->SliceEnd        = translation->SliceEnd;

    return pthis;
}




/* @func ensTranslationNewIni *************************************************
**
** Constructor for an Ensembl Translation with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Translation::new
** @param [u] startexon [EnsPExon]
** Ensembl Exon in which the start coordinate is annotated
** @param [u] endexon [EnsPExon]
** Ensembl Exon in which the end coordinate is annotated
** @param [r] start [ajuint] Start coordinate relative to the start Exon
** @param [r] end [ajuint] Start coordinate relative to the end Exon
** @param [u] sequence [AjPStr] Translation sequence
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
**
** @return [EnsPTranslation] Ensembl Translation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPTranslation ensTranslationNewIni(EnsPTranslationadaptor tla,
                                     ajuint identifier,
                                     EnsPExon startexon,
                                     EnsPExon endexon,
                                     ajuint start,
                                     ajuint end,
                                     AjPStr sequence,
                                     AjPStr stableid,
                                     ajuint version,
                                     AjPStr cdate,
                                     AjPStr mdate)
{
    EnsPTranslation translation = NULL;

    if (!startexon)
        return NULL;

    if (!endexon)
        return NULL;

    AJNEW0(translation);

    translation->Use = 1U;

    translation->Identifier = identifier;

    translation->Adaptor = tla;

    translation->Startexon = ensExonNewRef(startexon);

    translation->Endexon = ensExonNewRef(endexon);

    translation->Start = start;

    translation->End = end;

    if (stableid)
        translation->Stableidentifier = ajStrNewRef(stableid);

    if (cdate)
        translation->DateCreation = ajStrNewRef(cdate);

    if (mdate)
        translation->DateModification = ajStrNewRef(mdate);

    translation->Version = version;

    translation->Attributes = NULL;

    translation->Databaseentries = NULL;

    translation->Proteinfeatures = NULL;

    if (sequence)
        translation->Sequence = ajStrNewRef(sequence);

    translation->TranscriptStart = 0;

    translation->TranscriptEnd = 0;

    translation->SliceStart = 0;

    translation->SliceEnd = 0;

    return translation;
}




/* @func ensTranslationNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [EnsPTranslation] Ensembl Translation or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranslation ensTranslationNewRef(EnsPTranslation translation)
{
    if (!translation)
        return NULL;

    translation->Use++;

    return translation;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Del Destroy (free) an Ensembl Translation
**
** @argrule * Ptranslation [EnsPTranslation*] Ensembl Translation address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTranslationDel ****************************************************
**
** Default destructor for an Ensembl Translation.
**
** @param [d] Ptranslation [EnsPTranslation*] Ensembl Translation address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensTranslationDel(EnsPTranslation *Ptranslation)
{
    EnsPTranslation pthis = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPProteinfeature pf = NULL;

    if (!Ptranslation)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensTranslationDel"))
    {
        ajDebug("ensTranslationDel\n"
                "  *Ptranslation %p\n",
                *Ptranslation);

        ensTranslationTrace(*Ptranslation, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Ptranslation)
        return;

    pthis = *Ptranslation;

    pthis->Use--;

    if (pthis->Use)
    {
        *Ptranslation = NULL;

        return;
    }

    ensExonDel(&pthis->Startexon);
    ensExonDel(&pthis->Endexon);

    ajStrDel(&pthis->Stableidentifier);
    ajStrDel(&pthis->DateCreation);
    ajStrDel(&pthis->DateModification);

    /* Clear and delete the AJAX List of Ensembl Attributes. */

    while (ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while (ajListPop(pthis->Databaseentries, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Protein Feature objects. */

    while (ajListPop(pthis->Proteinfeatures, (void **) &pf))
        ensProteinfeatureDel(&pf);

    ajListFree(&pthis->Proteinfeatures);

    ajStrDel(&pthis->Sequence);

    AJFREE(pthis);

    *Ptranslation = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Get Return Translation attribute(s)
** @nam4rule Adaptor Return the Ensembl Translation Adaptor
** @nam4rule Date Return a date
** @nam5rule DateCreation Return the creation date
** @nam5rule DateModification Return the modification date
** @nam4rule End Return the end coordinate
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Startexon Return the start Exon
** @nam4rule Endexon Return the end Exon
** @nam4rule Start Return the start coordinate
** @nam4rule Stableidentifier Return the stable identifier
** @nam4rule Version Return the version
** @nam4rule Attributes Return all Ensembl Attributes
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Proteinfeatures Return all Ensembl Protein Feature objects
**
** @argrule * translation [const EnsPTranslation] Translation
**
** @valrule Adaptor [EnsPTranslationadaptor]
** Ensembl Translation Adaptor or NULL
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Creation date or NULL
** @valrule End [ajuint] End coordinate or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Startexon [EnsPExon] Start Exon or NULL
** @valrule Endexon [EnsPExon] End Exon or NULL
** @valrule Start [ajuint] Start coordinate or 0U
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Version [ajuint] Version or 0U
** @valrule GetAttributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule GetDatabaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule GetProteinfeatures [const AjPList]
** AJAX List of Ensembl Protein Feature objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensTranslationGetAdaptor *********************************************
**
** Get the Ensembl Translation Adaptor member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [EnsPTranslationadaptor] Ensembl Translation Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranslationadaptor ensTranslationGetAdaptor(
    const EnsPTranslation translation)
{
    return (translation) ? translation->Adaptor : NULL;
}




/* @func ensTranslationGetDateCreation ****************************************
**
** Get the creation date member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::created_date
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranslationGetDateCreation(const EnsPTranslation translation)
{
    return (translation) ? translation->DateCreation : NULL;
}




/* @func ensTranslationGetDateModification ************************************
**
** Get the modification date member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::modified_date
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [AjPStr] Modification date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranslationGetDateModification(const EnsPTranslation translation)
{
    return (translation) ? translation->DateModification : NULL;
}




/* @func ensTranslationGetEnd *************************************************
**
** Get the end member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::end
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Translation end coordinate, which is a position within
**                  the end Exon member or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensTranslationGetEnd(const EnsPTranslation translation)
{
    return (translation) ? translation->End : 0U;
}




/* @func ensTranslationGetEndexon *********************************************
**
** Get the end Ensembl Exon member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::end_Exon
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [EnsPExon] End Ensembl Exon or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExon ensTranslationGetEndexon(const EnsPTranslation translation)
{
    return (translation) ? translation->Endexon : NULL;
}




/* @func ensTranslationGetIdentifier ******************************************
**
** Get the SQL database-internal identifier member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensTranslationGetIdentifier(const EnsPTranslation translation)
{
    return (translation) ? translation->Identifier : 0U;
}




/* @func ensTranslationGetStableidentifier ************************************
**
** Get the stable identifier member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::stable_id
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [AjPStr] Stable identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensTranslationGetStableidentifier(const EnsPTranslation translation)
{
    return (translation) ? translation->Stableidentifier : NULL;
}




/* @func ensTranslationGetStart ***********************************************
**
** Get the start member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::start
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Translation start coordinate, which is a position within
**                  the start Exon member or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensTranslationGetStart(const EnsPTranslation translation)
{
    return (translation) ? translation->Start : 0U;
}




/* @func ensTranslationGetStartexon *******************************************
**
** Get the start Ensembl Exon member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::start_Exon
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [EnsPExon] Start Ensembl Exon or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPExon ensTranslationGetStartexon(const EnsPTranslation translation)
{
    return (translation) ? translation->Startexon : NULL;
}




/* @func ensTranslationGetVersion *********************************************
**
** Get the version member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::version
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [ajuint] Version or 0U
**
** @release 6.3.0
** @@
******************************************************************************/

ajuint ensTranslationGetVersion(const EnsPTranslation translation)
{
    return (translation) ? translation->Version : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Translation object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Load Return Ensembl Translation attribute(s) loaded on demand
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Proteinfeatures Return all Ensembl Protein Feature objects
**
** @argrule * translation [EnsPTranslation] Ensembl Translation
**
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule Proteinfeatures [const AjPList]
** AJAX List of Ensembl Protein Feature objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensTranslationLoadAttributes *****************************************
**
** Get all Ensembl Attribute objects of an Ensembl Translation.
**
** This is not a simple accessor function, it will fetch Ensembl Attribute
** objects from the Ensembl SQL database in case the AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Translation::get_all_Attributes
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranslationLoadAttributes(EnsPTranslation translation)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (ajDebugTest("ensTranslationLoadAttributes"))
        ajDebug("ensTranslationLoadAttributes\n"
                "  translation %p\n",
                translation);

    if (!translation)
        return NULL;

    if (translation->Attributes)
        return translation->Attributes;
    else
        translation->Attributes = ajListNew();

    if (!translation->Adaptor)
    {
        ajDebug("ensTranslationLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an Ensembl Translation "
                "without an Ensembl Translation Adaptor.\n");

        return NULL;
    }

    dba = ensTranslationadaptorGetDatabaseadaptor(translation->Adaptor);

    if (!dba)
    {
        ajDebug("ensTranslationLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an Ensembl Translation "
                "without an Ensembl Database Adaptor set in the "
                "Ensembl Translation Adaptor.\n");

        return NULL;
    }

    ata = ensRegistryGetAttributeadaptor(dba);

    ensAttributeadaptorFetchAllbyTranslation(ata,
                                             translation,
                                             (const AjPStr) NULL,
                                             translation->Attributes);

    return translation->Attributes;
}




/* @func ensTranslationLoadDatabaseentries ************************************
**
** Get all Ensembl Database Entry objects of an Ensembl Translation.
**
** This is not a simple accessor function, it will fetch
** Ensembl Database Entry objects from the Ensembl SQL database in case the
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Translation::get_all_DBEntries
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [const AjPList] AJAX List of Ensembl Database Entry objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranslationLoadDatabaseentries(EnsPTranslation translation)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentryadaptor dbea = NULL;

    if (!translation)
        return NULL;

    if (translation->Databaseentries)
        return translation->Databaseentries;
    else
        translation->Databaseentries = ajListNew();

    if (!translation->Adaptor)
    {
        ajDebug("ensTranslationLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Translation "
                "without an Ensembl Translation Adaptor.\n");

        return NULL;
    }

    dba = ensTranslationadaptorGetDatabaseadaptor(translation->Adaptor);

    if (!dba)
    {
        ajDebug("ensTranslationLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Translation "
                "without an Ensembl Database Adaptor set in the "
                "Ensembl Translation Adaptor.\n");

        return NULL;
    }

    dbea = ensRegistryGetDatabaseentryadaptor(dba);

    objtype = ajStrNewC("Translation");

    ensDatabaseentryadaptorFetchAllbyObject(dbea,
                                            translation->Identifier,
                                            objtype,
                                            (AjPStr) NULL,
                                            ensEExternaldatabaseTypeNULL,
                                            translation->Databaseentries);

    ajStrDel(&objtype);

    return translation->Databaseentries;
}




/* @func ensTranslationLoadProteinfeatures ************************************
**
** Get all Ensembl Protein Feature objects of an Ensembl Translation.
**
** This is not a simple accessor function, it will fetch
** Ensembl Protein Feature objects from the Ensembl SQL database in case the
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Translation::get_all_Proteinfeatures
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [const AjPList] AJAX List of Ensembl Protein Feature objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensTranslationLoadProteinfeatures(EnsPTranslation translation)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPProteinfeatureadaptor pfa = NULL;

    if (!translation)
        return NULL;

    if (translation->Proteinfeatures)
        return translation->Proteinfeatures;
    else
        translation->Proteinfeatures = ajListNew();

    if (!translation->Adaptor)
    {
        ajDebug("ensTranslationLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Translation "
                "without an Ensembl Translation Adaptor.\n");

        return NULL;
    }

    dba = ensTranslationadaptorGetDatabaseadaptor(translation->Adaptor);

    if (!dba)
    {
        ajDebug("ensTranslationLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Translation "
                "without an Ensembl Database Adaptor set in the "
                "Ensembl Translation Adaptor.\n");

        return NULL;
    }

    pfa = ensRegistryGetProteinfeatureadaptor(dba);

    ensProteinfeatureadaptorFetchAllbyTranslationidentifier(
        pfa,
        translation->Identifier,
        translation->Proteinfeatures);

    return translation->Proteinfeatures;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Set Set one member of a Translation
** @nam4rule Adaptor Set the Ensembl Translation Adaptor
** @nam4rule Date Set a date
** @nam5rule DateCreation Set the creation date
** @nam5rule DateModification Set the modification date
** @nam4rule End Set the end coordinate
** @nam4rule Endexon Set the end Exon
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Stableidentifier Set the stable identifier
** @nam4rule Start Set the start coordinate
** @nam4rule Startexon Set the start Exon
** @nam4rule Version Set the version
**
** @argrule * translation [EnsPTranslation] Ensembl Translation object
** @argrule Adaptor tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule DateModification mdate [AjPStr] Modification date
** @argrule End end [ajuint] End coordinate
** @argrule Endexon exon [EnsPExon] Ensembl Exon
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Stableidentifier stableid [AjPStr] Stable identifier
** @argrule Start start [ajuint] Start coordinate
** @argrule Startexon exon [EnsPExon] Ensembl Exon
** @argrule Version version [ajuint] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensTranslationSetAdaptor *********************************************
**
** Set the Ensembl Translation Adaptor member of an Ensembl Translation.
**
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationSetAdaptor(EnsPTranslation translation,
                                EnsPTranslationadaptor tla)
{
    if (!translation)
        return ajFalse;

    translation->Adaptor = tla;

    return ajTrue;
}




/* @func ensTranslationSetDateCreation ****************************************
**
** Set the creation date member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::created_date
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [uE] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationSetDateCreation(EnsPTranslation translation,
                                     AjPStr cdate)
{
    if (ajDebugTest("ensTranslationSetDateCreation"))
    {
        ajDebug("ensTranslationSetDateCreation\n"
                "  translation %p\n"
                "  cdate '%S'\n",
                translation,
                cdate);

        ensTranslationTrace(translation, 1);
    }

    if (!translation)
        return ajFalse;

    ajStrDel(&translation->DateCreation);

    if (cdate)
        translation->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensTranslationSetDateModification ************************************
**
** Set the modification date member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::created_date
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [uE] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationSetDateModification(EnsPTranslation translation,
                                         AjPStr mdate)
{
    if (ajDebugTest("ensTranslationSetDateModification"))
    {
        ajDebug("ensTranslationSetDateModification\n"
                "  translation %p\n"
                "  mdate '%S'\n",
                translation,
                mdate);

        ensTranslationTrace(translation, 1);
    }

    if (!translation)
        return ajFalse;

    ajStrDel(&translation->DateModification);

    if (mdate)
        translation->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensTranslationSetEnd *************************************************
**
** Set the end member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::end
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] end [ajuint] End coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationSetEnd(EnsPTranslation translation,
                            ajuint end)
{
    if (ajDebugTest("ensTranslationSetEnd"))
    {
        ajDebug("ensTranslationSetEnd\n"
                "  translation %p\n"
                "  end %u\n",
                translation,
                end);

        ensTranslationTrace(translation, 1);
    }

    if (!translation)
        return ajFalse;

    translation->End = end;

    /* Clear internal caches that depend on Exon coordinates. */

    ajStrDel(&translation->Sequence);

    translation->TranscriptStart = 0;
    translation->TranscriptEnd   = 0;
    translation->SliceStart      = 0;
    translation->SliceEnd        = 0;

    return ajTrue;
}




/* @func ensTranslationSetEndexon *********************************************
**
** Set the end Ensembl Exon member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::end_Exon
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationSetEndexon(EnsPTranslation translation,
                                EnsPExon exon)
{
    if (ajDebugTest("ensTranslationSetEndexon"))
    {
        ajDebug("ensTranslationSetEndexon\n"
                "  translation %p\n"
                "  exon %p\n",
                translation,
                exon);

        ensTranslationTrace(translation, 1);

        ensExonTrace(exon, 1);
    }

    if (!translation)
        return ajFalse;

    if (!exon)
        return ajFalse;

    ensExonDel(&translation->Endexon);

    translation->Endexon = ensExonNewRef(exon);

    /* Clear internal caches that depend on Exon coordinates. */

    ajStrDel(&translation->Sequence);

    translation->TranscriptStart = 0;
    translation->TranscriptEnd   = 0;
    translation->SliceStart      = 0;
    translation->SliceEnd        = 0;

    return ajTrue;
}




/* @func ensTranslationSetIdentifier ******************************************
**
** Set the SQL database-internal identifier member of an Ensembl Translation.
**
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationSetIdentifier(EnsPTranslation translation,
                                   ajuint identifier)
{
    if (!translation)
        return ajFalse;

    translation->Identifier = identifier;

    return ajTrue;
}




/* @func ensTranslationSetStableidentifier ************************************
**
** Set the stable identifier member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::stable_id
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [uE] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationSetStableidentifier(EnsPTranslation translation,
                                         AjPStr stableid)
{
    if (ajDebugTest("ensTranslationSetStableidentifier"))
    {
        ajDebug("ensTranslationSetStableidentifier\n"
                "  translation %p\n"
                "  stableid '%S'\n",
                translation,
                stableid);

        ensTranslationTrace(translation, 1);
    }

    if (!translation)
        return ajFalse;

    ajStrDel(&translation->Stableidentifier);

    if (stableid)
        translation->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensTranslationSetStart ***********************************************
**
** Set the start member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::start
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] start [ajuint] Start coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationSetStart(EnsPTranslation translation,
                              ajuint start)
{
    if (ajDebugTest("ensTranslationSetStart"))
    {
        ajDebug("ensTranslationSetStart\n"
                "  translation %p\n"
                "  start %u\n",
                translation,
                start);

        ensTranslationTrace(translation, 1);
    }

    if (!translation)
        return ajFalse;

    translation->Start = start;

    /* Clear internal caches that depend on Exon coordinates. */

    ajStrDel(&translation->Sequence);

    translation->TranscriptStart = 0;
    translation->TranscriptEnd   = 0;
    translation->SliceStart      = 0;
    translation->SliceEnd        = 0;

    return ajTrue;
}




/* @func ensTranslationSetStartexon *******************************************
**
** Set the start Ensembl Exon member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::start_Exon
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] exon [EnsPExon] Ensembl Exon
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationSetStartexon(EnsPTranslation translation,
                                  EnsPExon exon)
{
    if (ajDebugTest("ensTranslationSetStartexon"))
    {
        ajDebug("ensTranslationSetStartexon\n"
                "  translation %p\n"
                "  exon %p\n",
                translation,
                exon);

        ensTranslationTrace(translation, 1);

        ensExonTrace(exon, 1);
    }

    if (!translation)
        return ajFalse;

    if (!exon)
        return ajFalse;

    ensExonDel(&translation->Startexon);

    translation->Startexon = ensExonNewRef(exon);

    /* Clear internal caches that depend on Exon coordinates. */

    ajStrDel(&translation->Sequence);

    translation->TranscriptStart = 0;
    translation->TranscriptEnd   = 0;
    translation->SliceStart      = 0;
    translation->SliceEnd        = 0;

    return ajTrue;
}




/* @func ensTranslationSetVersion *********************************************
**
** Set the version member of an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::version
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationSetVersion(EnsPTranslation translation,
                                ajuint version)
{
    if (ajDebugTest("ensTranslationSetVersion"))
    {
        ajDebug("ensTranslationSetVersion\n"
                "  translation %p\n"
                "  version %u\n",
                translation,
                version);

        ensTranslationTrace(translation, 1);
    }

    if (!translation)
        return ajFalse;

    translation->Version = version;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Add Add one object to an Ensembl Translation
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Databaseentry Add an Ensembl Database Entry
** @nam4rule Proteinfeature Add an Ensembl Protein Feature
**
** @argrule * translation [EnsPTranslation] Ensembl Translation object
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Databaseentry dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Proteinfeature pf [EnsPProteinfeature] Ensembl Protein Feature
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensTranslationAddAttribute *******************************************
**
** Add an Ensembl Attribute to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::add_Attributes
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
** Adding an Ensembl Attribute also clears the sequence cache.
******************************************************************************/

AjBool ensTranslationAddAttribute(EnsPTranslation translation,
                                  EnsPAttribute attribute)
{
    if (!translation)
        return ajFalse;

    if (!attribute)
        return ajFalse;

    if (!translation->Attributes)
        translation->Attributes = ajListNew();

    ajListPushAppend(translation->Attributes,
                     (void *) ensAttributeNewRef(attribute));

    /* Clear the sequence cache. */

    if (translation->Sequence)
        ajStrDel(&translation->Sequence);

    return ajTrue;
}




/* @func ensTranslationAddDatabaseentry ***************************************
**
** Add an Ensembl Database Entry to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::add_DBEntry
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationAddDatabaseentry(EnsPTranslation translation,
                                      EnsPDatabaseentry dbe)
{
    if (!translation)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!translation->Databaseentries)
        translation->Databaseentries = ajListNew();

    ajListPushAppend(translation->Databaseentries,
                     (void *) ensDatabaseentryNewRef(dbe));

    return ajTrue;
}




/* @func ensTranslationAddProteinfeature **************************************
**
** Add an Ensembl Protein Feature to an Ensembl Translation.
**
** @cc Bio::EnsEMBL::Translation::add_Proteinfeature
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationAddProteinfeature(EnsPTranslation translation,
                                       EnsPProteinfeature pf)
{
    if (!translation)
        return ajFalse;

    if (!pf)
        return ajFalse;

    if (!translation->Proteinfeatures)
        translation->Proteinfeatures = ajListNew();

    ajListPushAppend(translation->Proteinfeatures,
                     (void *) ensProteinfeatureNewRef(pf));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Trace Report Ensembl Translation members to debug file
**
** @argrule Trace translation [const EnsPTranslation] Ensembl Translation
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensTranslationTrace **************************************************
**
** Trace an Ensembl Translation.
**
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationTrace(const EnsPTranslation translation, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPProteinfeature pf = NULL;

    if (!translation)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTranslationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Startexon %p\n"
            "%S  Endexon %p\n"
            "%S  Start %u\n"
            "%S  End %u\n"
            "%S  Stableidentifier '%S'\n"
            "%S  DateCreation '%S'\n"
            "%S  DateModification '%S'\n"
            "%S  Version %u\n"
            "%S  Attributes %p\n"
            "%S  Databaseentries %p\n"
            "%S  Proteinfeatures %p\n"
            "%S  Sequence %p\n"
            "%S  TranscriptStart %u\n"
            "%S  TranscriptEnd %u\n"
            "%S  SliceStart %u\n"
            "%S  SliceEnd %u\n",
            indent, translation,
            indent, translation->Use,
            indent, translation->Identifier,
            indent, translation->Adaptor,
            indent, translation->Startexon,
            indent, translation->Endexon,
            indent, translation->Start,
            indent, translation->End,
            indent, translation->Stableidentifier,
            indent, translation->DateCreation,
            indent, translation->DateModification,
            indent, translation->Version,
            indent, translation->Attributes,
            indent, translation->Databaseentries,
            indent, translation->Proteinfeatures,
            indent, translation->Sequence,
            indent, translation->TranscriptStart,
            indent, translation->TranscriptEnd,
            indent, translation->SliceStart,
            indent, translation->SliceEnd);

    ensExonTrace(translation->Startexon, level + 1);

    ensExonTrace(translation->Endexon, level + 1);

    /* Trace the AJAX List of Ensembl Attributes. */

    if (translation->Attributes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Attributes\n",
                indent, translation->Attributes);

        iter = ajListIterNewread(translation->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if (translation->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entry objects\n",
                indent, translation->Attributes);

        iter = ajListIterNewread(translation->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Protein Feature objects. */

    if (translation->Proteinfeatures)
    {
        ajDebug("%S    AJAX List %p of Ensembl Protein Feature objects:\n",
                indent, translation->Proteinfeatures);

        iter = ajListIterNewread(translation->Proteinfeatures);

        while (!ajListIterDone(iter))
        {
            pf = (EnsPProteinfeature) ajListIterGet(iter);

            ensProteinfeatureTrace(pf, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Calculate Calculate Ensembl Translation values
** @nam4rule Length  Calculate the length
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Slice   Calculate Ensembl Translation coordinates relative to
**                   an Ensembl Slice
** @nam5rule End    Calculate the Ensembl Translation end coordinate
** @nam5rule Start  Calculate the Ensembl Translation start coordinate
** @nam4rule Transcript Calculate Ensembl Transcript coordinates relative to
**                      an Ensembl Transcript
** @nam5rule End    Calculate the Ensembl Transcript end coordinate
** @nam5rule Start  Calculate the Ensembl Transcript start coordinate
**
** @argrule Length translation [EnsPTranslation] Ensembl Translation
** @argrule Memsize translation [const EnsPTranslation] Ensembl Translation
** @argrule SliceEnd translation [EnsPTranslation] Ensembl Translation
** @argrule SliceStart translation [EnsPTranslation] Ensembl Translation
** @argrule TranscriptEnd translation [EnsPTranslation] Ensembl Translation
** @argrule TranscriptStart translation [EnsPTranslation] Ensembl Translation
**
** @valrule Length [ajuint] Translation (protein) length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule SliceEnd [ajint] End coordinate or 0
** @valrule SliceStart [ajint] Start coordinate or 0
** @valrule TranscriptEnd [ajuint] End coordinate or 0U
** @valrule TranscriptStart [ajuint] Start coordinate or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensTranslationCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an Ensembl Translation.
**
** @param [r] translation [const EnsPTranslation] Ensembl Translation
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensTranslationCalculateMemsize(const EnsPTranslation translation)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPProteinfeature pf = NULL;

    if (!translation)
        return 0;

    size += sizeof (EnsOTranslation);

    size += ensExonCalculateMemsize(translation->Startexon);
    size += ensExonCalculateMemsize(translation->Endexon);

    if (translation->Sequence)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(translation->Sequence);
    }

    if (translation->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(translation->Stableidentifier);
    }

    if (translation->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(translation->DateCreation);
    }

    if (translation->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(translation->DateModification);
    }

    /* Summarise the AJAX List of Ensembl Attributes. */

    if (translation->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(translation->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if (translation->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(translation->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Protein Feature objects. */

    if (translation->Proteinfeatures)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(translation->Proteinfeatures);

        while (!ajListIterDone(iter))
        {
            pf = (EnsPProteinfeature) ajListIterGet(iter);

            size += ensProteinfeatureCalculateMemsize(pf);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @func ensTranslationCalculateSliceEnd **************************************
**
** Calculate the end position of an Ensembl Translation in
** Ensembl Slice coordinates.
**
** @cc Bio::EnsEMBL::Translation::genomic_end
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajint] Translation end coordinate relative to the Slice
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensTranslationCalculateSliceEnd(EnsPTranslation translation)
{
    EnsPFeature feature = NULL;

    if (!translation)
        return 0;

    if (translation->SliceEnd)
        return translation->SliceEnd;

    feature = ensExonGetFeature(translation->Endexon);

    if (ensFeatureGetStrand(feature) >= 0)
    {
        feature = ensExonGetFeature(translation->Endexon);

        translation->SliceEnd = ensFeatureGetStart(feature)
            + (translation->End - 1);
    }
    else
    {
        feature = ensExonGetFeature(translation->Startexon);

        translation->SliceEnd = ensFeatureGetEnd(feature)
            - (translation->Start - 1);
    }

    return translation->SliceEnd;
}




/* @func ensTranslationCalculateSliceStart ************************************
**
** Calculate the start position of an Ensembl Translation in
** Ensembl Slice coordinates.
**
** @cc Bio::EnsEMBL::Translation::genomic_start
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajint] Translation start coordinate relative to the Slice
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensTranslationCalculateSliceStart(EnsPTranslation translation)
{
    EnsPFeature feature = NULL;

    if (!translation)
        return 0;

    if (translation->SliceStart)
        return translation->SliceStart;

    feature = ensExonGetFeature(translation->Startexon);

    if (ensFeatureGetStrand(feature) >= 0)
    {
        feature = ensExonGetFeature(translation->Startexon);

        translation->SliceStart = ensFeatureGetStart(feature)
            + (translation->Start - 1);
    }
    else
    {
        feature = ensExonGetFeature(translation->Endexon);

        translation->SliceStart = ensFeatureGetEnd(feature)
            - (translation->End - 1);
    }

    return translation->SliceStart;
}




/* @func ensTranslationCalculateTranscriptEnd *********************************
**
** Calculate the end coordinate of an Ensembl Translation in
** Ensembl Transcript coordinates.
**
** @cc Bio::EnsEMBL::Translation::cdna_end
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajuint]
** Translation end coordinate relative to the Transcript or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensTranslationCalculateTranscriptEnd(EnsPTranslation translation)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    if (!translation)
        return 0U;

    if (translation->TranscriptEnd)
        return translation->TranscriptEnd;

    if (!translation->Adaptor)
        return 0U;

    dba = ensTranslationadaptorGetDatabaseadaptor(translation->Adaptor);

    tca = ensRegistryGetTranscriptadaptor(dba);

    ensTranscriptadaptorFetchByTranslationidentifier(tca,
                                                     translation->Identifier,
                                                     &transcript);

    if (!transcript)
    {
        ajDebug("ensTranslationCalculateTranscriptEnd could not fetch an "
                "Ensembl Transcript for Ensembl Translation %p.\n",
                translation);

        ensTranslationTrace(translation, 1);

        return 0U;
    }

    translation->TranscriptStart =
        ensExonCalculateTranscriptCodingStart(translation->Startexon,
                                              transcript,
                                              translation);

    translation->TranscriptEnd =
        ensExonCalculateTranscriptCodingEnd(translation->Endexon,
                                            transcript,
                                            translation);

    ensTranscriptDel(&transcript);

    return translation->TranscriptEnd;
}




/* @func ensTranslationCalculateTranscriptStart *******************************
**
** Calculate the start coordinate of an Ensembl Translation in
** Ensembl Transcript coordinates.
**
** @cc Bio::EnsEMBL::Translation::cdna_start
** @param [u] translation [EnsPTranslation] Ensembl Translation
**
** @return [ajuint]
** Translation start coordinate relative to the Transcript or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensTranslationCalculateTranscriptStart(EnsPTranslation translation)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    if (!translation)
        return 0U;

    if (translation->TranscriptStart)
        return translation->TranscriptStart;

    if (!translation->Adaptor)
        return 0U;

    dba = ensTranslationadaptorGetDatabaseadaptor(translation->Adaptor);

    tca = ensRegistryGetTranscriptadaptor(dba);

    ensTranscriptadaptorFetchByTranslationidentifier(tca,
                                                     translation->Identifier,
                                                     &transcript);

    if (!transcript)
    {
        ajDebug("ensTranslationCalculateTranscriptStart could not fetch an "
                "Ensembl Transcript for Ensembl Translation %p.\n",
                translation);

        ensTranslationTrace(translation, 1);

        return 0U;
    }

    translation->TranscriptStart =
        ensExonCalculateTranscriptCodingStart(translation->Startexon,
                                              transcript,
                                              translation);

    translation->TranscriptEnd =
        ensExonCalculateTranscriptCodingEnd(translation->Endexon,
                                            transcript,
                                            translation);

    ensTranscriptDel(&transcript);

    return translation->TranscriptStart;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Translation object.
**
** @fdata [EnsPTranslation]
**
** @nam3rule Fetch Fetch Ensembl Translation values
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Databaseentries Fetch all Ensembl Database Entry objects
** @nam5rule Domains Fetch all Ensembl Protein Feature objectss from the
** EBI InterPro conserved protein domain database
** @nam5rule Proteinfeatures Fetch all Ensembl Protein Feature objects
** @nam5rule Sequenceedits Fetch all Ensembl Sequence Edit objects
** @nam4rule Displayidentifier Fetch the display identifier
** @nam4rule Sequence Fetch the sequence
** @nam5rule Seq Fetch as AJAX Sequence object
** @nam5rule Str Fetch as AJAX String object
**
** @argrule AllAttributes translation [EnsPTranslation] Ensembl Translation
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllDatabaseentries translation [EnsPTranslation]
** Ensembl Translation
** @argrule AllDatabaseentries name [const AjPStr]
** Ensembl External Database name
** @argrule AllDatabaseentries type [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllDatabaseentries dbes [AjPList]
** AJAX List of Ensembl Database Entry objects
** @argrule AllProteinfeatures translation [EnsPTranslation]
** Ensembl Translation
** @argrule AllDomains translation [EnsPTranslation] Ensembl Translation
** @argrule AllDomains pfs [AjPList]
** AJAX List of Ensembl Protein Feature objects
** @argrule AllProteinfeatures name [const AjPStr] Ensembl Analysis name
** @argrule AllProteinfeatures pfs [AjPList]
** AJAX List of Ensembl Protein Feature objects
** @argrule AllSequenceedits translation [EnsPTranslation] Ensembl Translation
** @argrule AllSequenceedits ses [AjPList]
** AJAX List of Ensembl Sequence Edit objects
** @argrule Displayidentifier translation [const EnsPTranslation]
** Ensembl Translation
** @argrule Displayidentifier Pidentifier [AjPStr*] AJAX String address
** @argrule Seq translation [EnsPTranslation] Ensembl Translation
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence address
** @argrule Str translation [EnsPTranslation] Ensembl Translation
** @argrule Str Psequence [AjPStr*] AJAX String address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensTranslationFetchAllAttributes *************************************
**
** Fetch all Ensembl Attribute obkects of an Ensembl Translation.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Translation::get_all_Attributes
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationFetchAllAttributes(EnsPTranslation translation,
                                        const AjPStr code,
                                        AjPList attributes)
{
    AjBool match = AJFALSE;

    const AjPList list = NULL;
    AjIList iter       = NULL;

    EnsPAttribute attribute = NULL;

    if (ajDebugTest("ensTranslationFetchAllAttributes"))
        ajDebug("ensTranslationFetchAllAttributes\n"
                "  translation %p\n"
                "  code '%S'\n"
                "  attributes %p\n",
                translation,
                code,
                attributes);

    if (!translation)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    list = ensTranslationLoadAttributes(translation);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if (code)
        {
            if (ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if (match)
            ajListPushAppend(attributes,
                             (void *) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranslationFetchAllDatabaseentries ********************************
**
** Fetch all Ensembl Database Entry objects of an Ensembl Translation.
**
** The caller is responsible for deleting the Ensembl Database Entry objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Translation::get_all_DBEntries
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [u] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationFetchAllDatabaseentries(EnsPTranslation translation,
                                             const AjPStr name,
                                             EnsEExternaldatabaseType type,
                                             AjPList dbes)
{
    AjBool namematch = AJFALSE;
    AjBool typematch = AJFALSE;

    const AjPList list = NULL;
    AjIList iter       = NULL;

    EnsPDatabaseentry dbe = NULL;

    if (!translation)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    list = ensTranslationLoadDatabaseentries(translation);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        dbe = (EnsPDatabaseentry) ajListIterGet(iter);

        if (name)
        {
            if (ajStrMatchCaseS(name, ensDatabaseentryGetDbName(dbe)))
                namematch = ajTrue;
            else
                namematch = ajFalse;
        }
        else
            namematch = ajTrue;

        if (type)
        {
            if (type == ensDatabaseentryGetType(dbe))
                typematch = ajTrue;
            else
                typematch = ajFalse;
        }
        else
            typematch = ajTrue;

        if (namematch && typematch)
            ajListPushAppend(dbes, (void *) ensDatabaseentryNewRef(dbe));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranslationFetchAllDomains ****************************************
**
** Fetch all Ensembl Protein Feature objects of an Ensembl Translation that
** represent entries of the EBI InterPro conserved domain database.
**
** The corresponding Ensembl Analysis names for Protein Feature objects have
** to be defined in the
** static const char *translationKProteinfeatureDomainName[] array.
**
** The caller is responsible for deleting the Ensembl Protein Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Translation::get_all_DomainFeatures
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] pfs [AjPList] AJAX List of Ensembl Protein Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationFetchAllDomains(EnsPTranslation translation,
                                     AjPList pfs)
{
    register ajuint i = 0U;

    AjPStr name = NULL;

    if (!translation)
        return ajFalse;

    if (!pfs)
        return ajFalse;

    name = ajStrNew();

    for (i = 0U; translationKProteinfeatureDomainName[i]; i++)
    {
        ajStrAssignC(&name, translationKProteinfeatureDomainName[i]);

        ensTranslationFetchAllProteinfeatures(translation, name, pfs);
    }

    ajStrDel(&name);

    return ajTrue;
}




/* @func ensTranslationFetchAllProteinfeatures ********************************
**
** Fetch all Ensembl Protein Feature objects of an Ensembl Translation.
**
** The caller is responsible for deleting the Ensembl Protein Feature objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Translation::get_all_Proteinfeatures
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [r] name [const AjPStr] Ensembl Analysis name
** @param [u] pfs [AjPList] AJAX List of Ensembl Protein Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationFetchAllProteinfeatures(EnsPTranslation translation,
                                             const AjPStr name,
                                             AjPList pfs)
{
    AjBool match = AJFALSE;

    const AjPList list = NULL;

    AjIList iter = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPFeature feature = NULL;

    EnsPFeaturepair fp = NULL;

    EnsPProteinfeature pf = NULL;

    if (!translation)
        return ajFalse;

    if (!pfs)
        return ajFalse;

    list = ensTranslationLoadProteinfeatures(translation);

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        pf = (EnsPProteinfeature) ajListIterGet(iter);

        if (name)
        {
            fp = ensProteinfeatureGetFeaturepair(pf);

            feature = ensFeaturepairGetSourceFeature(fp);

            analysis = ensFeatureGetAnalysis(feature);

            if (ajStrMatchCaseS(name, ensAnalysisGetName(analysis)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if (match)
            ajListPushAppend(pfs, (void *) ensProteinfeatureNewRef(pf));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensTranslationFetchAllSequenceedits **********************************
**
** Fetch all Ensembl Sequence Edit objects for an Ensembl Transcript.
**
** The corresponding Ensembl Attribute codes have to be defined in the
** static const char *translationKSequenceeditCode[] array.
**
** The caller is responsible for deleting the Ensembl Sequence Edit objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Translation::get_all_SeqEdits
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [u] ses [AjPList] AJAX List of Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationFetchAllSequenceedits(EnsPTranslation translation,
                                           AjPList ses)
{
    register ajuint i = 0U;

    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute at = NULL;

    EnsPSequenceedit se = NULL;

    if (!translation)
        return ajFalse;

    if (!ses)
        return ajFalse;

    code = ajStrNew();

    attributes = ajListNew();

    for (i = 0U; translationKSequenceeditCode[i]; i++)
    {
        ajStrAssignC(&code, translationKSequenceeditCode[i]);

        ensTranslationFetchAllAttributes(translation, code, attributes);
    }

    while (ajListPop(attributes, (void **) &at))
    {
        se = ensSequenceeditNewAttribute(at);

        ajListPushAppend(ses, (void *) se);

        ensAttributeDel(&at);
    }

    ajListFree(&attributes);

    ajStrDel(&code);

    return ajTrue;
}




/* @func ensTranslationFetchDisplayidentifier *********************************
**
** Fetch the display identifier of an Ensembl Translation.
**
** This will return the stable identifier, the SQL database-internal identifier
** or the Translation memory address in this order.
**
** The caller is responsible for deleting the AJAX String object.
**
** @cc Bio::EnsEMBL::Translation:display_id
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [wP] Pidentifier [AjPStr*] Display identifier address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationFetchDisplayidentifier(const EnsPTranslation translation,
                                            AjPStr *Pidentifier)
{
    if (!translation)
        return ajFalse;

    if (!Pidentifier)
        return ajFalse;

    if (translation->Stableidentifier &&
        ajStrGetLen(translation->Stableidentifier))
    {
        if (*Pidentifier)
            ajStrAssignS(Pidentifier, translation->Stableidentifier);
        else
            *Pidentifier = ajStrNewS(translation->Stableidentifier);
    }
    else if (translation->Identifier)
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier,
                                       "%u", translation->Identifier);
        else
            *Pidentifier = ajFmtStr("%u", translation->Identifier);
    }
    else
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier, "%p", translation);
        else
            *Pidentifier = ajFmtStr("%p", translation);
    }

    return ajTrue;
}




/* @func ensTranslationFetchSequenceSeq ***************************************
**
** Fetch the sequence of an Ensembl Translation as AJAX Sequence.
**
** Returns the sequence passed into the constructor or fetches the
** Ensembl Transcript from the database and returns the sequence based on
** ensTranscriptFetchSequenceTranslationStr. Generally, it is more efficient
** to fetch the sequence via ensTranscriptFetchSequenceTranslationStr.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @cc Bio::EnsEMBL::Translation:seq
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationFetchSequenceSeq(EnsPTranslation translation,
                                      AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!translation)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    ensTranslationFetchDisplayidentifier(translation, &name);
    ensTranslationFetchSequenceStr(translation, &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
        *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetProt(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensTranslationFetchSequenceStr ***************************************
**
** Fetch the sequence of an Ensembl Translation as AJAX String.
**
** Returns the sequence passed into the constructor or fetches the
** Ensembl Transcript from the database and returns the sequence based on
** ensTranscriptFetchSequenceTranslationStr. Generally, it is more efficient
** to fetch the sequence via ensTranscriptFetchSequenceTranslationStr.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Translation::seq
** @cc Bio::EnsEMBL::Translation::modify_translation
** @param [u] translation [EnsPTranslation] Ensembl Translation
** @param [w] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
** NOTE: In this implementation, the Ensembl Translation object does not cache
** the translated sequence. Ensembl Exon sequences are cached, as are
** Transcript and Translation Attributes so that the sequence can be
** recalculated quickly.
******************************************************************************/

AjBool ensTranslationFetchSequenceStr(EnsPTranslation translation,
                                      AjPStr *Psequence)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    if (!translation)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNew();

    if (translation->Sequence)
        ajStrAssignS(Psequence, translation->Sequence);

    dba = ensTranslationadaptorGetDatabaseadaptor(translation->Adaptor);

    tca = ensRegistryGetTranscriptadaptor(dba);

    ensTranscriptadaptorFetchByTranslationidentifier(tca,
                                                     translation->Identifier,
                                                     &transcript);

    if (!transcript)
    {
        ajDebug("ensTranslationFetchSequenceStr could not fetch a Transcript "
                "for Translation with identifier %u from the database.\n",
                translation->Identifier);

        return ajFalse;
    }

    ensTranscriptFetchSequenceTranslationStr(transcript,
                                             translation,
                                             Psequence);

    ensTranscriptDel(&transcript);

    return ajTrue;
}




/* @datasection [EnsPTranslationadaptor] Ensembl Translation Adaptor **********
**
** @nam2rule Translationadaptor Functions for manipulating
** Ensembl Translation Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor
** @cc CVS Revision: 1.64
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic translationadaptorFetchAllbyStatement **************************
**
** Fetch all Ensembl Translation objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] translations [AjPList] AJAX List of Ensembl Translation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool translationadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList translations)
{
    ajuint identifier   = 0U;
    ajuint transcriptid = 0U;
    ajuint startexonid  = 0U;
    ajuint start        = 0U;
    ajuint endexonid    = 0U;
    ajuint end          = 0U;
    ajuint version      = 0U;

    AjIList iter = NULL;
    const AjPList exons = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr stableid = NULL;
    AjPStr cdate    = NULL;
    AjPStr mdate    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon      = NULL;
    EnsPExon startexon = NULL;
    EnsPExon endexon   = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    EnsPTranslation translation = NULL;
    EnsPTranslationadaptor tla = NULL;

    if (ajDebugTest("translationadaptorFetchAllbyStatement"))
        ajDebug("translationadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  translations %p\n",
                ba,
                statement,
                am,
                slice,
                translations);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!translations)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    tca = ensRegistryGetTranscriptadaptor(dba);
    tla = ensRegistryGetTranslationadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier   = 0;
        transcriptid = 0;
        start        = 0;
        startexonid  = 0;
        end          = 0;
        endexonid    = 0;
        stableid     = ajStrNew();
        version      = 0;
        cdate        = ajStrNew();
        mdate        = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &transcriptid);
        ajSqlcolumnToUint(sqlr, &start);
        ajSqlcolumnToUint(sqlr, &startexonid);
        ajSqlcolumnToUint(sqlr, &end);
        ajSqlcolumnToUint(sqlr, &endexonid);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);

        ensTranscriptadaptorFetchByIdentifier(tca, transcriptid, &transcript);

        if (!transcript)
            ajFatal("translationadaptorFetchAllbyStatement could not get "
                    "Ensembl Transcript for identifier %u.\n", transcriptid);

        exons = ensTranscriptLoadExons(transcript);

        iter = ajListIterNewread(exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            if (ensExonGetIdentifier(exon) == startexonid)
                startexon = exon;

            if (ensExonGetIdentifier(exon) == endexonid)
                endexon = exon;
        }

        ajListIterDel(&iter);

        if (!startexon)
            ajFatal("translationadaptorFetchAllbyStatement could not get "
                    "start Exon for transcript_id %u",
                    ensTranscriptGetIdentifier(transcript));

        if (!endexon)
            ajFatal("translationadaptorFetchAllbyStatement could not get "
                    "end Exon for transcript_id %u",
                    ensTranscriptGetIdentifier(transcript));

        translation = ensTranslationNewIni(tla,
                                           identifier,
                                           startexon,
                                           endexon,
                                           start,
                                           end,
                                           (AjPStr) NULL,
                                           stableid,
                                           version,
                                           cdate,
                                           mdate);

        ajListPushAppend(translations, (void *) translation);

        ensTranscriptDel(&transcript);

        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Translation Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Translation Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPTranslationadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPTranslationadaptor] Ensembl Translation Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensTranslationadaptorNew *********************************************
**
** Default constructor for an Ensembl Translation Adaptor.
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
** @see ensRegistryGetTranslationadaptor
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPTranslationadaptor] Ensembl Translation Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPTranslationadaptor ensTranslationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        translationadaptorKTables,
        translationadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &translationadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Translation Adaptor object.
**
** @fdata [EnsPTranslationadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Translation Adaptor
**
** @argrule * Ptla [EnsPTranslationadaptor*]
** Ensembl Translation Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTranslationadaptorDel *********************************************
**
** Default destructor for an Ensembl Translation Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Ptla [EnsPTranslationadaptor*]
** Ensembl Translation Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensTranslationadaptorDel(EnsPTranslationadaptor *Ptla)
{
    ensBaseadaptorDel(Ptla);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Translation Adaptor object.
**
** @fdata [EnsPTranslationadaptor]
**
** @nam3rule Get Return Ensembl Translation Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensTranslationadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Translation Adaptor.
**
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensTranslationadaptorGetDatabaseadaptor(
    EnsPTranslationadaptor tla)
{
    return ensBaseadaptorGetDatabaseadaptor(tla);
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Translation objects from an
** Ensembl SQL database.
**
** @fdata [EnsPTranslationadaptor]
**
** @nam3rule Fetch Retrieve Ensembl Translation object(s)
** @nam4rule All Retrieve all Ensembl Translation objects
** @nam4rule Allby Retrieve all Ensembl Translation objects matching a
** criterion
** @nam5rule Externalname  Fetch all by an external name
** @nam5rule Transcript Fetch all by an Ensembl Transcript
** @nam5rule Transcripts Fetch all by Ensembl Transcript objects
** @nam6rule List Fetch all by an AJAX List of Ensembl Transcript objects
** @nam6rule Table Fetch all by an AJAX Table of Ensembl Transcript objects
** @nam5rule Stableidentifier Fetch all by a stable Ensembl Exon identifier
** @nam4rule By Fetch one Ensembl Exon object matching a criterion
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by stable Ensembl Exon identifier
**
** @argrule * tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @argrule All translations [AjPList] AJAX List of Ensembl Translation objects
** @argrule AllbyExternalname name [const AjPStr] Ensembl Database Entry name
** @argrule AllbyExternalname dbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbyExternalname translations [AjPList]
** AJAX List of Ensembl Translation objects
** @argrule AllbyTranscript transcript [EnsPTranscript] Ensembl Transcript
** @argrule AllbyTranscriptsList transcripts [AjPList]
** AJAX List of Ensembl Transcript objects
** @argrule AllbyTranscriptsTable transcripts [AjPTable]
** AJAX Table of Ensembl Transcript objects
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule AllbyStableidentifier translations [AjPList]
** AJAX List of Ensembl Translation objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByDisplaylabel Ptranslation [EnsPTranslation*]
** Ensembl Translation address
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Ptranslation [EnsPTranslation*]
** Ensembl Translation address
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule ByStableidentifier Ptranslation [EnsPTranslation*]
** Ensembl Translation address
** @argrule ByTranscript transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensTranslationadaptorFetchAllbyExternalname **************************
**
** Fetch all Ensembl Translation objects via an Ensembl Database Entry name and
** Ensembl External Database name.
** The caller is responsible for deleting the Ensembl Translation objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::fetch_all_by_external_name
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [r] name [const AjPStr] Ensembl Database Entry name
** @param [r] dbname [const AjPStr] Ensembl External Database name
** @param [u] translations [AjPList] AJAX List of Ensembl Translations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchAllbyExternalname(
    EnsPTranslationadaptor tla,
    const AjPStr name,
    const AjPStr dbname,
    AjPList translations)
{
    ajuint *Pidentifier = NULL;

    AjPList idlist = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentryadaptor dbea = NULL;

    EnsPTranslation translation = NULL;

    if (!tla)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!dbname)
        return ajFalse;

    if (!translations)
        return ajFalse;

    dba = ensTranslationadaptorGetDatabaseadaptor(tla);

    dbea = ensRegistryGetDatabaseentryadaptor(dba);

    idlist = ajListNew();

    ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternalname(
        dbea,
        name,
        dbname,
        idlist);

    while (ajListPop(idlist, (void **) &Pidentifier))
    {
        ensTranslationadaptorFetchByIdentifier(tla,
                                               *Pidentifier,
                                               &translation);

        ajListPushAppend(translations, (void *) translation);

        AJFREE(Pidentifier);
    }

    ajListFree(&idlist);

    return ajTrue;
}




/* @func ensTranslationadaptorFetchAllbyTranscript ****************************
**
** Fetch all alternative Ensembl Translation objects via an Ensembl Transcript.
**
** Alternative Ensembl Translation are set and retained in the
** Ensembl Transcript so that they become accessible via
** ensTranscriptLoadAlternativetranslations.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::
** fetch_all_alternative_by_Transcript
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchAllbyTranscript(EnsPTranslationadaptor tla,
                                                 EnsPTranscript transcript)
{
    ajuint identifier  = 0U;
    ajuint startexonid = 0U;
    ajuint start       = 0U;
    ajuint endexonid   = 0U;
    ajuint end         = 0U;
    ajuint version     = 0U;

    AjBool debug = AJFALSE;

    AjIList iter        = NULL;
    const AjPList exons = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr stableid  = NULL;
    AjPStr cdate     = NULL;
    AjPStr mdate     = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon      = NULL;
    EnsPExon startexon = NULL;
    EnsPExon endexon   = NULL;

    EnsPTranslation translation = NULL;

    debug = ajDebugTest("ensTranslationadaptorFetchAllbyTranscript");

    if (debug)
        ajDebug("ensTranslationadaptorFetchAllbyTranscript\n"
                "  tla %p\n"
                "  transcript %p\n",
                tla,
                transcript);

    if (!tla)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(tla);

    statement = ajFmtStr(
        "SELECT "
        "translation.translation_id, "
        "translation.seq_start, "
        "translation.start_exon_id, "
        "translation.seq_end, "
        "translation.end_exon_id, "
        "translation.stable_id, "
        "translation.version, "
        "translation.created_date, "
        "translation.modified_date "
        "FROM "
        "(translation) "
        "JOIN "
        "transcript "
        "ON "
        "(translation.transcript_id = transcript.transcript_id) "
        "WHERE "
        "translation.transcript_id = %u "
        "AND "
        "translation.translation_id != transcript.canonical_translation_id",
        ensTranscriptGetIdentifier(transcript));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        start = 0;
        startexonid = 0;
        end = 0;
        endexonid = 0;
        stableid = ajStrNew();
        version = 0;
        cdate = ajStrNew();
        mdate = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &start);
        ajSqlcolumnToUint(sqlr, &startexonid);
        ajSqlcolumnToUint(sqlr, &end);
        ajSqlcolumnToUint(sqlr, &endexonid);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);

        exons = ensTranscriptLoadExons(transcript);

        iter = ajListIterNewread(exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            if (ensExonGetIdentifier(exon) == startexonid)
                startexon = exon;

            if (ensExonGetIdentifier(exon) == endexonid)
                endexon = exon;
        }

        ajListIterDel(&iter);

        if (!startexon)
            ajFatal("ensTranslationadaptorFetchAllbyTranscript could not get "
                    "start Exon for Transcript with identifier %u.",
                    ensTranscriptGetIdentifier(transcript));

        if (!endexon)
            ajFatal("ensTranslationadaptorFetchAllbyTranscript could not get "
                    "end Exon for Transcript with identifier %u.",
                    ensTranscriptGetIdentifier(transcript));

        translation = ensTranslationNewIni(tla,
                                           identifier,
                                           startexon,
                                           endexon,
                                           start,
                                           end,
                                           (AjPStr) NULL,
                                           stableid,
                                           version,
                                           cdate,
                                           mdate);

        ensTranscriptAddAlternativetranslation(transcript, translation);

        ensTranslationDel(&translation);

        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic translationadaptorFetchAllbyIdentifiers ************************
**
** Helper function for the generic function to fetch Ensembl Translation
** objects by an AJAX List of Ensembl Transcript objects.
**
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [r] csv [const AjPStr] Comma-separated list of SQL database-internal
**                               identifiers used in an IN comparison function
**                               in a SQL SELECT statement
** @param [r] canonicalmap [const AjPTable] AJAX Table of
** AJAX unsigned integer (Ensembl Transcript identifier) key data and
** AJAX unsigned integer (Ensembl Translation) value data
** @param [u] transcripts [AjPTable] AJAX Table of
** AJAX unsigned integer (Ensembl Transcript identifier) key data and
** Ensembl Transcript value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool translationadaptorFetchAllbyIdentifiers(
    EnsPTranslationadaptor tla,
    const AjPStr csv,
    const AjPTable canonicalmap,
    AjPTable transcripts)
{
    ajuint identifier   = 0U;
    ajuint transcriptid = 0U;
    ajuint startexonid  = 0U;
    ajuint start        = 0U;
    ajuint endexonid    = 0U;
    ajuint end          = 0U;
    ajuint version      = 0U;

    const ajuint *Pcanonical = NULL;

    AjIList iter        = NULL;
    const AjPList exons = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr stableid  = NULL;
    AjPStr cdate     = NULL;
    AjPStr mdate     = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon      = NULL;
    EnsPExon startexon = NULL;
    EnsPExon endexon   = NULL;

    EnsPTranscript transcript = NULL;

    EnsPTranslation translation = NULL;

    if (!tla)
        return ajFalse;

    if (!(csv && ajStrGetLen(csv)))
        return ajFalse;

    if (!canonicalmap)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(tla);

    statement = ajFmtStr(
        "SELECT "
        "translation.translation_id, "
        "translation.transcript_id, "
        "translation.seq_start, "
        "translation.start_exon_id, "
        "translation.seq_end, "
        "translation.end_exon_id, "
        "translation.stable_id, "
        "translation.version, "
        "translation.created_date, "
        "translation.modified_date "
        "FROM "
        "translation "
        "WHERE "
        "translation.transcript_id IN (%S)",
        csv);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        transcriptid = 0;
        start = 0;
        startexonid = 0;
        end = 0;
        endexonid = 0;
        stableid = ajStrNew();
        version = 0;
        cdate = ajStrNew();
        mdate = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &transcriptid);
        ajSqlcolumnToUint(sqlr, &start);
        ajSqlcolumnToUint(sqlr, &startexonid);
        ajSqlcolumnToUint(sqlr, &end);
        ajSqlcolumnToUint(sqlr, &endexonid);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);

        transcript = (EnsPTranscript) ajTableFetchmodV(
            transcripts,
            (const void *) &transcriptid);

        if (!transcript)
        {
            ajDebug("translationadaptorFetchAllbyIdentifiers could not get "
                    "Ensembl Transcript with identifier %u from the "
                    "AJAX Table of Ensembl Transcript objects.\n");

            ajStrDel(&stableid);
            ajStrDel(&cdate);
            ajStrDel(&mdate);

            continue;
        }

        exons = ensTranscriptLoadExons(transcript);

        iter = ajListIterNewread(exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            if (ensExonGetIdentifier(exon) == startexonid)
                startexon = exon;

            if (ensExonGetIdentifier(exon) == endexonid)
                endexon = exon;
        }

        ajListIterDel(&iter);

        if (!startexon)
            ajFatal("ensTranslationadaptorFetchByTranscript could not get "
                    "start Exon for Transcript with identifier %u.",
                    ensTranscriptGetIdentifier(transcript));

        if (!endexon)
            ajFatal("ensTranslationadaptorFetchByTranscript could not get "
                    "end Exon for Transcript with identifier %u.",
                    ensTranscriptGetIdentifier(transcript));

        translation = ensTranslationNewIni(tla,
                                           identifier,
                                           startexon,
                                           endexon,
                                           start,
                                           end,
                                           (AjPStr) NULL,
                                           stableid,
                                           version,
                                           cdate,
                                           mdate);

        Pcanonical = (const ajuint *) ajTableFetchV(
            canonicalmap,
            (const void *) &transcriptid);

        if (Pcanonical && (*Pcanonical == identifier))
            ensTranscriptSetTranslation(transcript, translation);

        ensTranslationDel(&translation);

        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic translationadaptorRetrieveAllCanonicalidentifiers **************
**
** Helper function for the generic function to retrieve
** Ensembl Transcript identifier to canonical Ensembl Translation identifier
** mapping.
**
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [r] csv [const AjPStr] Comma-separated list of SQL database-internal
**                               identifiers used in an IN comparison function
**                               in a SQL SELECT statement
** @param [u] canonicalmap [AjPTable] AJAX Table of
** AJAX unsigned integer (Ensembl Transcript identifier) key data and
** AJAX unsigned integer (Ensembl Translation) value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool translationadaptorRetrieveAllCanonicalidentifiers(
    EnsPTranslationadaptor tla,
    const AjPStr csv,
    AjPTable canonicalmap)
{
    ajuint trid = 0U;
    ajuint tlid = 0U;

    ajuint *Ptlid = NULL;
    ajuint *Ptrid = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!tla)
        return ajFalse;

    if (!csv)
        return ajFalse;

    if (!canonicalmap)
        return ajFalse;

    dba = ensTranslationadaptorGetDatabaseadaptor(tla);

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id, "
        "transcript.canonical_translation_id "
        "FROM "
        "transcript "
        "WHERE "
        "transcript.transcript_id IN (%S)",
        csv);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0U;
        tlid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
        ajSqlcolumnToUint(sqlr, &tlid);

        if (!ajTableMatchV(canonicalmap, (const void *) &trid))
        {
            AJNEW0(Ptrid);

            *Ptrid = trid;

            AJNEW0(Ptlid);

            *Ptlid = tlid;

            ajTablePut(canonicalmap, (void *) Ptrid, (void *) Ptlid);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensTranslationadaptorFetchAllbyTranscriptsList ***********************
**
** Fetch all Ensembl Translation objects via an
** AJAX List of Ensembl Transcript objects.
**
** The the Ensembl Translation objects are set and retained in the
** Ensembl Transcript objects so that they become accessible via
** ensTranscriptLoadTranslation.
**
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchAllbyTranscriptsList(
    EnsPTranslationadaptor tla,
    AjPList transcripts)
{
    ajuint *Pidentifier = NULL;

    AjIList iter = NULL;

    AjPTable table = NULL;

    EnsPTranscript newtranscript = NULL;
    EnsPTranscript oldtranscript = NULL;

    if (!tla)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    /*
    ** Create an AJAX Table of
    ** AJAX unsigned integer key data and
    ** Ensembl Transcript value data.
    */

    table = ajTableuintNew(ajListGetLength(transcripts));

    ajTableSetDestroyvalue(table, (void (*)(void **)) &ensTranscriptDel);

    iter = ajListIterNew(transcripts);

    while (!ajListIterDone(iter))
    {
        newtranscript = (EnsPTranscript) ajListIterGet(iter);

        if (!newtranscript)
            continue;

        AJNEW0(Pidentifier);

        *Pidentifier = ensTranscriptGetIdentifier(newtranscript);

        oldtranscript = (EnsPTranscript) ajTableFetchmodV(
            table,
            (const void *) Pidentifier);

        if (oldtranscript)
        {
            ajDebug("ensTranslationadaptorFetchAllbyTranscriptsList got "
                    "Ensembl Transcript objects with "
                    "identical identifiers:\n");

            ensTranscriptTrace(newtranscript, 1);
            ensTranscriptTrace(oldtranscript, 1);

            AJFREE(Pidentifier);

            continue;
        }

        ajTablePut(table,
                   (void *) Pidentifier,
                   (void *) ensTranscriptNewRef(newtranscript));
    }

    ajListIterDel(&iter);

    ensTranslationadaptorFetchAllbyTranscriptsTable(tla, table);

    ajTableDel(&table);

    return ajTrue;
}




/* @func ensTranslationadaptorFetchAllbyTranscriptsTable **********************
**
** Fetch all Ensembl Translation objects via an AJAX Table of
** Ensembl Transcript objects indexed by their identifiers.
**
** The the Ensembl Translation objects are set and retained in the
** Ensembl Transcript objects so that they become accessible via
** ensTranscriptLoadTranslation.
**
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [u] transcripts [AjPTable] AJAX Table of Ensembl Transcript objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchAllbyTranscriptsTable(
    EnsPTranslationadaptor tla,
    AjPTable transcripts)
{
    void **keyarray = NULL;

    register ajuint i = 0U;

    AjPStr csv = NULL;

    AjPTable canonicalmap = NULL;

    if (!tla)
        return ajFalse;

    if (!transcripts)
        return ajFalse;

    csv = ajStrNew();

    canonicalmap = ajTableuintNew(ajTableGetSize(transcripts));

    ajTableSetDestroyvalue(canonicalmap, (void (*)(void **)) ajMemFree);

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifiers configured in the Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    ajTableToarrayKeys(transcripts, &keyarray);

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
                translationadaptorRetrieveAllCanonicalidentifiers(
                    tla,
                    csv,
                    canonicalmap);

                translationadaptorFetchAllbyIdentifiers(
                    tla,
                    csv,
                    canonicalmap,
                    transcripts);
            }

            ajStrAssignClear(&csv);
        }
    }

    AJFREE(keyarray);

    ajStrDel(&csv);

    ajTableDel(&canonicalmap);

    return ajTrue;
}




/* @func ensTranslationadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Translation via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::fetch_by_dbID
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ptranslation [EnsPTranslation*] Ensembl Translation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchByIdentifier(EnsPTranslationadaptor tla,
                                              ajuint identifier,
                                              EnsPTranslation *Ptranslation)
{
    if (!tla)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Ptranslation)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(tla,
                                           identifier,
                                           (void **) Ptranslation);
}




/* @func ensTranslationadaptorFetchByStableidentifier *************************
**
** Fetch an Ensembl Translation via its stable identifier and version.
** In case a version is not specified, the current Ensembl Translation
** will be returned.
** The caller is responsible for deleting the Ensembl Translation.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::fetch_by_stable_id
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Ptranslation [EnsPTranslation*] Ensembl Translation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchByStableidentifier(
    EnsPTranslationadaptor tla,
    const AjPStr stableid,
    ajuint version,
    EnsPTranslation *Ptranslation)
{
    char *txtstableid = NULL;

    AjBool result = AJFALSE;

    AjPList translations = NULL;

    AjPStr constraint = NULL;

    EnsPTranslation translation = NULL;

    if (!tla)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Ptranslation)
        return ajFalse;

    ensBaseadaptorEscapeC(tla, &txtstableid, stableid);

    if (version)
        constraint = ajFmtStr(
            "translation.stable_id = '%s' "
            "AND "
            "translation.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "translation.stable_id = '%s'",
            txtstableid);

    ajCharDel(&txtstableid);

    translations = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(tla,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                translations);

    if (ajListGetLength(translations) > 1)
        ajDebug("ensTranslationadaptorFetchByStableidentifier got more than "
                "one Translation for stable identifier '%S' and version %u.\n",
                stableid, version);

    ajListPop(translations, (void **) Ptranslation);

    while (ajListPop(translations, (void **) &translation))
        ensTranslationDel(&translation);

    ajListFree(&translations);

    ajStrDel(&constraint);

    return result;
}




/* @func ensTranslationadaptorFetchByTranscript *******************************
**
** Fetch an Ensembl Translation via an Ensembl Transcript.
**
** The Ensembl Translation is set and retained in the Ensembl Transcript
** so that it becomes accessible via ensTranscriptLoadTranslation.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::fetch_by_Transcript
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorFetchByTranscript(EnsPTranslationadaptor tla,
                                              EnsPTranscript transcript)
{
    ajuint identifier  = 0U;
    ajuint startexonid = 0U;
    ajuint start       = 0U;
    ajuint endexonid   = 0U;
    ajuint end         = 0U;
    ajuint version     = 0U;

    ajulong rows = 0UL;

    AjBool debug = AJFALSE;

    AjIList iter        = NULL;
    const AjPList exons = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr stableid  = NULL;
    AjPStr cdate     = NULL;
    AjPStr mdate     = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon exon      = NULL;
    EnsPExon startexon = NULL;
    EnsPExon endexon   = NULL;

    EnsPTranslation translation = NULL;

    debug = ajDebugTest("ensTranslationadaptorFetchByTranscript");

    if (debug)
        ajDebug("ensTranslationadaptorFetchByTranscript\n"
                "  tla %p\n"
                "  transcript %p\n",
                tla,
                transcript);

    if (!tla)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(tla);

    statement = ajFmtStr(
        "SELECT "
        "translation.translation_id, "
        "translation.seq_start, "
        "translation.start_exon_id, "
        "translation.seq_end, "
        "translation.end_exon_id, "
        "translation.stable_id, "
        "translation.version, "
        "translation.created_date, "
        "translation.modified_date "
        "FROM "
        "(translation) "
        "JOIN "
        "transcript "
        "ON "
        "(translation.translation_id = transcript.canonical_translation_id) "
        "WHERE "
        "translation.transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    /* Check first, how many rows were returned. */

    rows = ajSqlstatementGetSelectedrows(sqls);

    if (rows == 0)
    {
        if (debug)
        {
            ajDebug("ensTranslationadaptorFetchByTranscript could not get "
                    "an Ensembl Translation for Ensembl Transcript %u.\n",
                    ensTranscriptGetIdentifier(transcript));

            ensTranscriptTrace(transcript, 1);
        }
    }
    else if (rows > 1)
    {
        if (debug)
        {
            ajDebug("ensTranslationadaptorFetchByTranscript got more than one "
                    "canonical Ensembl Translation for Ensembl Transcript "
                    "%u.\n", ensTranscriptGetIdentifier(transcript));

            ensTranscriptTrace(transcript, 1);
        }

        ensDatabaseadaptorSqlstatementDel(dba, &sqls);

        ajStrDel(&statement);

        return ajFalse;
    }

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        start = 0;
        startexonid = 0;
        end = 0;
        endexonid = 0;
        stableid = ajStrNew();
        version = 0;
        cdate = ajStrNew();
        mdate = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &start);
        ajSqlcolumnToUint(sqlr, &startexonid);
        ajSqlcolumnToUint(sqlr, &end);
        ajSqlcolumnToUint(sqlr, &endexonid);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);

        exons = ensTranscriptLoadExons(transcript);

        iter = ajListIterNewread(exons);

        while (!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            if (ensExonGetIdentifier(exon) == startexonid)
                startexon = exon;

            if (ensExonGetIdentifier(exon) == endexonid)
                endexon = exon;
        }

        ajListIterDel(&iter);

        if (!startexon)
            ajFatal("ensTranslationadaptorFetchByTranscript could not get "
                    "start Exon for Transcript with identifier %u.",
                    ensTranscriptGetIdentifier(transcript));

        if (!endexon)
            ajFatal("ensTranslationadaptorFetchByTranscript could not get "
                    "end Exon for Transcript with identifier %u.",
                    ensTranscriptGetIdentifier(transcript));

        translation = ensTranslationNewIni(tla,
                                           identifier,
                                           startexon,
                                           endexon,
                                           start,
                                           end,
                                           (AjPStr) NULL,
                                           stableid,
                                           version,
                                           cdate,
                                           mdate);

        ensTranscriptSetTranslation(transcript, translation);

        ensTranslationDel(&translation);

        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Translation objects
** from an Ensembl SQL database.
**
** @fdata [EnsPTranslationadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Translation-releated object(s)
** @nam4rule All Retrieve all Ensembl Translation-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
** @nam5rule Stableidentifiers Fetch all stable Ensembl Translation identifiers
**
** @argrule * tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
** @argrule AllStableidentifiers identifiers [AjPList] AJAX List of AJAX String
**                                              stable Ensembl Translation
**                                              identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensTranslationadaptorRetrieveAllIdentifiers **************************
**
** Retrieve all SQL database-internal identifiers of
** Ensembl Translation objects.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::list_dbIDs
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorRetrieveAllIdentifiers(
    EnsPTranslationadaptor tla,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!tla)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("translation");

    result = ensBaseadaptorRetrieveAllIdentifiers(tla,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensTranslationadaptorRetrieveAllStableidentifiers ********************
**
** Retrieve all stable identifiers of Ensembl Translation objects.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::TranslationAdaptor::list_stable_ids
** @param [u] tla [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTranslationadaptorRetrieveAllStableidentifiers(
    EnsPTranslationadaptor tla,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table   = NULL;
    AjPStr primary = NULL;

    if (!tla)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table   = ajStrNewC("translation");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(tla,
                                              table,
                                              primary,
                                              identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}

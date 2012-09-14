/* @source ensgene ************************************************************
**
** Ensembl Gene functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.72 $
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

#include "ensattribute.h"
#include "ensdatabaseentry.h"
#include "ensexon.h"
#include "ensexternaldatabase.h"
#include "ensgene.h"
#include "ensprojectionsegment.h"
#include "enstable.h"
#include "enstranscript.h"




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

/* @conststatic geneKStatus ***************************************************
**
** The Ensembl Gene status member is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsEGeneStatus.
**
******************************************************************************/

static const char *const geneKStatus[] =
{
    "",
    "KNOWN",
    "NOVEL",
    "PUTATIVE",
    "PREDICTED",
    "KNOWN_BY_PROJECTION",
    "UNKNOWN",
    (const char *) NULL
};




/* @conststatic geneadaptorKTables ********************************************
**
** Array of Ensembl Gene Adaptor SQL table names
**
******************************************************************************/

static const char *const geneadaptorKTables[] =
{
    "gene",
    "xref",
    (const char *) NULL
};




/* @conststatic geneadaptorKColumns *******************************************
**
** Array of Ensembl Gene Adaptor SQL column names
**
******************************************************************************/

static const char *const geneadaptorKColumns[] =
{
    "gene.gene_id",
    "gene.seq_region_id",
    "gene.seq_region_start",
    "gene.seq_region_end",
    "gene.seq_region_strand",
    "gene.analysis_id",
    "gene.display_xref_id",
    "gene.description",
    "gene.source",
    "gene.biotype",
    "gene.status",
    "gene.is_current",
    "gene.canonical_transcript_id",
    "gene.canonical_annotation",
    "gene.stable_id",
    "gene.version",
    "gene.created_date",
    "gene.modified_date",
    "xref.external_db_id",
    "xref.dbprimary_acc",
    "xref.display_label",
    "xref.version",
    "xref.description",
    "xref.info_type",
    "xref.info_text",
    (const char *) NULL
};




/* @conststatic geneadaptorKLeftjoin ******************************************
**
** Array of Ensembl Gene Adaptor SQL left join conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin geneadaptorKLeftjoin[] =
{
    {"xref", "gene.display_xref_id = xref.xref_id"},
    {(const char *) NULL, (const char *) NULL}
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static EnsPGene geneNewCpyFeatures(EnsPGene gene);

static int listExonCompareAddressAscending(
    const void *item1,
    const void *item2);

static void listExonDelete(void **Pitem, void *cl);

static int listGeneCompareEndAscending(
    const void *item1,
    const void *item2);

static int listGeneCompareEndDescending(
    const void *item1,
    const void *item2);

static int listGeneCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listGeneCompareStartAscending(
    const void *item1,
    const void *item2);

static int listGeneCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool geneadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList genes);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgene *******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGene] Ensembl Gene ***************************************
**
** @nam2rule Gene Functions for manipulating Ensembl Gene objects
**
** @cc Bio::EnsEMBL::Gene
** @cc CVS Revision: 1.180
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Gene by pointer.
** It is the responsibility of the user to first destroy any previous
** Gene. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGene]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gene [const EnsPGene] Ensembl Gene
** @argrule Ini ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini displaydbe [EnsPDatabaseentry] Ensembl display Database Entry
** @argrule Ini description [AjPStr] Description
** @argrule Ini source [AjPStr] Source
** @argrule Ini biotype [AjPStr] Biotype
** @argrule Ini status [EnsEGeneStatus] Status
** @argrule Ini current [AjBool] Current attribute
** @argrule Ini cantrcid [ajuint] Canonical Ensembl Transcript identifier
** @argrule Ini canann [AjPStr] Canonical annotation
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Version
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ini transcripts [AjPList] AJAX List of Ensembl Transcript objects
** @argrule Ref gene [EnsPGene] Ensembl Gene
**
** @valrule * [EnsPGene] Ensembl Gene or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGeneNewCpy ********************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPGene] Ensembl Gene or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGene ensGeneNewCpy(const EnsPGene gene)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene pthis = NULL;

    EnsPTranscript transcript = NULL;

    if (!gene)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gene->Identifier;
    pthis->Adaptor    = gene->Adaptor;
    pthis->Feature    = ensFeatureNewRef(gene->Feature);

    pthis->Displayreference = ensDatabaseentryNewRef(gene->Displayreference);

    if (gene->Description)
        pthis->Description = ajStrNewRef(gene->Description);

    if (gene->Source)
        pthis->Source = ajStrNewRef(gene->Source);

    if (gene->Biotype)
        pthis->Biotype = ajStrNewRef(gene->Biotype);

    pthis->Status  = gene->Status;
    pthis->Current = gene->Current;

    if (gene->Canonicalannotation)
        pthis->Canonicalannotation = ajStrNewRef(gene->Canonicalannotation);

    pthis->Canonicaltranscriptidentifier =
        gene->Canonicaltranscriptidentifier;

    pthis->Version = gene->Version;

    if (gene->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(gene->Stableidentifier);

    if (gene->DateCreation)
        pthis->DateCreation = ajStrNewRef(gene->DateCreation);

    if (gene->DateModification)
        pthis->DateModification = ajStrNewRef(gene->DateModification);

    /* NOTE: Copy the AJAX List of Ensembl Attribute objects. */

    if (gene->Attributes && ajListGetLength(gene->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(gene->Attributes);

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

    /* NOTE: Copy the AJAX List of Ensembl Database Entry objects. */

    if (gene->Databaseentries && ajListGetLength(gene->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(gene->Databaseentries);

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

    /* NOTE: Copy the AJAX List of Ensembl Transcript objects. */

    if (gene->Transcripts && ajListGetLength(gene->Transcripts))
    {
        pthis->Transcripts = ajListNew();

        iter = ajListIterNew(gene->Transcripts);

        while (!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            ajListPushAppend(pthis->Transcripts,
                             (void *) ensTranscriptNewRef(transcript));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Transcripts = NULL;

    return pthis;
}




/* @func ensGeneNewIni ********************************************************
**
** Constructor of an Ensembl Gene with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Gene::new
** @param [u] displaydbe [EnsPDatabaseentry] Ensembl display Database Entry
** @param [u] description [AjPStr] Description
** @param [u] source [AjPStr] Source
** @param [u] biotype [AjPStr] Biotype
** @param [u] status [EnsEGeneStatus] Status
** @param [r] current [AjBool] Current attribute
** @param [r] cantrcid [ajuint] Canonical Ensembl Transcript identifier
** @param [u] canann [AjPStr] Canonical annotation
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [EnsPGene] Ensembl Gene or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGene ensGeneNewIni(EnsPGeneadaptor ga,
                       ajuint identifier,
                       EnsPFeature feature,
                       EnsPDatabaseentry displaydbe,
                       AjPStr description,
                       AjPStr source,
                       AjPStr biotype,
                       EnsEGeneStatus status,
                       AjBool current,
                       ajuint cantrcid,
                       AjPStr canann,
                       AjPStr stableid,
                       ajuint version,
                       AjPStr cdate,
                       AjPStr mdate,
                       AjPList transcripts)
{
    AjIList iter = NULL;

    EnsPGene gene = NULL;

    EnsPTranscript transcript = NULL;

    if (ajDebugTest("ensGeneNew"))
    {
        ajDebug("ensGeneNew\n"
                "  ga %p\n"
                "  identifier %u\n"
                "  feature %p\n"
                "  displaydbe %p\n"
                "  description '%S'\n"
                "  source '%S'\n"
                "  biotype '%S'\n"
                "  status %d\n"
                "  current %b\n"
                "  cantrcid %u\n"
                "  canann %p\n"
                "  stableid '%S'\n"
                "  version %u\n"
                "  cdate '%S'\n"
                "  mdate '%S'\n"
                "  transcripts %p\n",
                ga,
                identifier,
                feature,
                displaydbe,
                description,
                source,
                biotype,
                status,
                current,
                cantrcid,
                canann,
                stableid,
                version,
                cdate,
                mdate,
                transcripts);

        ensFeatureTrace(feature, 1);

        ensDatabaseentryTrace(displaydbe, 1);
    }

    if (!feature)
        return NULL;

    AJNEW0(gene);

    gene->Use        = 1U;
    gene->Identifier = identifier;
    gene->Adaptor    = ga;
    gene->Feature    = ensFeatureNewRef(feature);

    gene->Displayreference = ensDatabaseentryNewRef(displaydbe);

    if (description)
        gene->Description = ajStrNewRef(description);

    if (source)
        gene->Source = ajStrNewRef(source);

    if (biotype)
        gene->Biotype = ajStrNewRef(biotype);

    gene->Status  = status;
    gene->Current = current;

    if (canann)
        gene->Canonicalannotation = ajStrNewRef(canann);

    gene->Canonicaltranscriptidentifier = cantrcid;
    gene->Version = version;

    if (stableid)
        gene->Stableidentifier = ajStrNewRef(stableid);

    if (cdate)
        gene->DateCreation = ajStrNewRef(cdate);

    if (mdate)
        gene->DateModification = ajStrNewRef(mdate);

    gene->Attributes = NULL;

    gene->Databaseentries = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Transcript objects. */

    if (transcripts && ajListGetLength(transcripts))
    {
        gene->Transcripts = ajListNew();

        iter = ajListIterNew(transcripts);

        while (!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            ajListPushAppend(gene->Transcripts,
                             (void *) ensTranscriptNewRef(transcript));
        }

        ajListIterDel(&iter);
    }
    else
        gene->Transcripts = NULL;

    return gene;
}




/* @func ensGeneNewRef ********************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [EnsPGene] Ensembl Gene or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGene ensGeneNewRef(EnsPGene gene)
{
    if (!gene)
        return NULL;

    gene->Use++;

    return gene;
}




/* @funcstatic geneNewCpyFeatures *********************************************
**
** Returns a new copy of an Ensembl Gene, but in addition to the shallow
** copy provided by ensGeneNewCpy, also copies all Ensembl Gene-
** internal Ensembl Objects based on the Ensembl Feature class. This is useful
** in preparation of ensGeneTransform and ensGeneTransfer, which
** return an independent Ensembl Gene object and therefore, require
** independent mapping of all internal Feature objects to the new
** Ensembl Coordinate System or Ensembl Slice.
**
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [EnsPGene] Ensembl Gene or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

static EnsPGene geneNewCpyFeatures(EnsPGene gene)
{
    AjIList iter = NULL;

    EnsPGene newgene = NULL;

    EnsPTranscript newtranscript = NULL;
    EnsPTranscript oldtranscript = NULL;

    if (!gene)
        return NULL;

    newgene = ensGeneNewCpy(gene);

    if (!newgene)
        return NULL;

    /* Copy the AJAX List of Ensembl Transcript objects. */

    if (newgene->Transcripts)
    {
        iter = ajListIterNew(newgene->Transcripts);

        while (!ajListIterDone(iter))
        {
            oldtranscript = (EnsPTranscript) ajListIterGet(iter);

            ajListIterRemove(iter);

            newtranscript = ensTranscriptNewCpy(oldtranscript);

            ajListIterInsert(iter, (void *) newtranscript);

            /* Advance the AJAX List Iterator after the insert. */

            (void) ajListIterGet(iter);

            ensTranscriptDel(&oldtranscript);
        }

        ajListIterDel(&iter);
    }

    return newgene;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Del Destroy (free) an Ensembl Gene
**
** @argrule * Pgene [EnsPGene*] Ensembl Gene address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGeneDel ***********************************************************
**
** Default destructor for an Ensembl Gene.
**
** @param [d] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGeneDel(EnsPGene *Pgene)
{
    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPTranscript transcript = NULL;

    EnsPGene pthis = NULL;

    if (!Pgene)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGeneDel"))
    {
        ajDebug("ensGeneDel\n"
                "  *Pgene %p\n",
                *Pgene);

        ensGeneTrace(*Pgene, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgene)
        return;

    pthis = *Pgene;

    pthis->Use--;

    if ((*Pgene)->Use)
    {
        *Pgene = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensDatabaseentryDel(&pthis->Displayreference);

    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->Source);
    ajStrDel(&pthis->Biotype);
    ajStrDel(&pthis->Canonicalannotation);
    ajStrDel(&pthis->Stableidentifier);
    ajStrDel(&pthis->DateCreation);
    ajStrDel(&pthis->DateModification);

    /* Clear and delete the AJAX List of Ensembl Attribute objects. */

    while (ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while (ajListPop(pthis->Databaseentries, (void **) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Transcript objects. */

    while (ajListPop(pthis->Transcripts, (void **) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&pthis->Transcripts);

    AJFREE(pthis);

    *Pgene = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Get Return Gene attribute(s)
** @nam4rule Adaptor Return the Ensembl Gene Adaptor
** @nam4rule Biotype Return the biological type
** @nam4rule Canonicalannotation Return the canonical annotation
** @nam4rule Canonicaltranscriptidentifier
** Return the canonical Ensembl Transcript identifier
** @nam4rule Current Return the current attribute
** @nam4rule Date Return a date
** @nam5rule DateCreation Return the creation date
** @nam5rule DateModification Return the modification date
** @nam4rule Description Return the description
** @nam4rule Displayreference Return the display External Reference
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Source Return the source
** @nam4rule Stableidentifier Return the stable identifier
** @nam4rule Status Return the status
** @nam4rule Version Return the version
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Transcripts Return all Ensembl Transcript objects
**
** @argrule * gene [const EnsPGene] Gene
**
** @valrule Adaptor [EnsPGeneadaptor] Ensembl Gene Adaptor or NULL
** @valrule Biotype [AjPStr] Biological type or NULL
** @valrule Canonicaltranscriptidentifier [ajuint]
** Canonical Ensembl Transcript identifier or 0U
** @valrule Canonicalannotation [AjPStr] Canonical annotation or NULL
** @valrule Current [AjBool] Current flag or ajFalse
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule DateModification [AjPStr] Modification date or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayreference [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Source [AjPStr] Source or NULL
** @valrule Stableidentifier [AjPStr] Stable identifier or NULL
** @valrule Status [EnsEGeneStatus] Status or ensEGeneStatusNULL
** @valrule Version [ajuint] Version or 0U
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule Transcripts [const AjPList]
** AJAX List of Ensembl Transcript objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGeneGetAdaptor ****************************************************
**
** Get the Ensembl Gene Adaptor member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPGeneadaptor] Ensembl Gene Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGeneadaptor ensGeneGetAdaptor(const EnsPGene gene)
{
    return (gene) ? gene->Adaptor : NULL;
}




/* @func ensGeneGetBiotype ****************************************************
**
** Get the biological type member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::biotype
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Biological type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGeneGetBiotype(const EnsPGene gene)
{
    return (gene) ? gene->Biotype : NULL;
}




/* @func ensGeneGetCanonicalannotation ****************************************
**
** Get the canonical annotation member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_annotation
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Canonical annotation or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGeneGetCanonicalannotation(const EnsPGene gene)
{
    return (gene) ? gene->Canonicalannotation : NULL;
}




/* @func ensGeneGetCanonicaltranscriptidentifier ******************************
**
** Get the canonical Ensembl Transcript identifier member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_transcript
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [ajuint] Canonical Ensembl Transcript identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGeneGetCanonicaltranscriptidentifier(const EnsPGene gene)
{
    return (gene) ? gene->Canonicaltranscriptidentifier : 0U;
}




/* @func ensGeneGetCurrent ****************************************************
**
** Get the current attribute of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::is_current
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjBool] Current attribute or ajFalse
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneGetCurrent(const EnsPGene gene)
{
    return (gene) ? gene->Current : ajFalse;
}




/* @func ensGeneGetDateCreation ***********************************************
**
** Get the creation date member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::created_date
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGeneGetDateCreation(const EnsPGene gene)
{
    return (gene) ? gene->DateCreation : NULL;
}




/* @func ensGeneGetDateModification *******************************************
**
** Get the modification date member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::modified_date
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Modification date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGeneGetDateModification(const EnsPGene gene)
{
    return (gene) ? gene->DateModification : NULL;
}




/* @func ensGeneGetDescription ************************************************
**
** Get the description member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::description
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGeneGetDescription(const EnsPGene gene)
{
    return (gene) ? gene->Description : NULL;
}




/* @func ensGeneGetDisplayreference *******************************************
**
** Get the display reference member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::display_xref
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseentry ensGeneGetDisplayreference(const EnsPGene gene)
{
    return (gene) ? gene->Displayreference : NULL;
}




/* @func ensGeneGetFeature ****************************************************
**
** Get the Ensembl Feature member of an Ensembl Gene.
**
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensGeneGetFeature(const EnsPGene gene)
{
    return (gene) ? gene->Feature : NULL;
}




/* @func ensGeneGetIdentifier *************************************************
**
** Get the SQL database-internal identifier member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGeneGetIdentifier(const EnsPGene gene)
{
    return (gene) ? gene->Identifier : 0U;
}




/* @func ensGeneGetSource *****************************************************
**
** Get the source member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::source
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Source or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGeneGetSource(const EnsPGene gene)
{
    return (gene) ? gene->Source : NULL;
}




/* @func ensGeneGetStableidentifier *******************************************
**
** Get the stable identifier member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::stable_id
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Stable identifier or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGeneGetStableidentifier(const EnsPGene gene)
{
    return (gene) ? gene->Stableidentifier : NULL;
}




/* @func ensGeneGetStatus *****************************************************
**
** Get the status member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::status
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsEGeneStatus] Status or ensEGeneStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEGeneStatus ensGeneGetStatus(const EnsPGene gene)
{
    return (gene) ? gene->Status : ensEGeneStatusNULL;
}




/* @func ensGeneGetVersion ****************************************************
**
** Get the version member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::version
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [ajuint] Version or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGeneGetVersion(const EnsPGene gene)
{
    return (gene) ? gene->Version : 0U;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Gene object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPGene]
**
** @nam3rule Load Return Ensembl Gene attribute(s) loaded on demand
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Databaseentries Return all Ensembl Database Entry objects
** @nam4rule Transcripts Return all Ensembl Transcript objects
**
** @argrule * gene [EnsPGene] Ensembl Gene
**
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects or NULL
** @valrule Transcripts [const AjPList]
** AJAX List of Ensembl Transcript objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGeneLoadAttributes ************************************************
**
** Load all Ensembl Attribute objects of an Ensembl Gene.
**
** This is not a simple accessor function, it will fetch
** Ensembl Attribute objects from an Ensembl SQL database in case the
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Gene::get_all_Attributes
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGeneLoadAttributes(EnsPGene gene)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gene)
        return NULL;

    if (gene->Attributes)
        return gene->Attributes;

    if (!gene->Adaptor)
    {
        ajDebug("ensGeneLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an Ensembl Gene without an "
                "Ensembl Gene Adaptor.\n");

        return NULL;
    }

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    if (!dba)
    {
        ajDebug("ensGeneLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an Ensembl Gene without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Gene Adaptor.\n");

        return NULL;
    }

    ata = ensRegistryGetAttributeadaptor(dba);

    gene->Attributes = ajListNew();

    ensAttributeadaptorFetchAllbyGene(ata,
                                      gene,
                                      (AjPStr) NULL,
                                      gene->Attributes);

    return gene->Attributes;
}




/* @func ensGeneLoadDatabaseentries *******************************************
**
** Load all Ensembl Database Entry objects of an Ensembl Gene.
**
** This is not a simple accessor function, it will fetch
** Ensembl Database Entry objects from an Ensembl SQL database in case the
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Gene::get_all_DBEntries
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [const AjPList] AJAX List of Ensembl Database Entry objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGeneLoadDatabaseentries(EnsPGene gene)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;
    EnsPDatabaseentryadaptor dbea = NULL;

    if (!gene)
        return NULL;

    if (gene->Databaseentries)
        return gene->Databaseentries;

    if (!gene->Adaptor)
    {
        ajDebug("ensGeneLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Gene without "
                "an Ensembl Gene Adaptor.\n");

        return NULL;
    }

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    if (!dba)
    {
        ajDebug("ensGeneLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Gene without "
                "an Ensembl Database Adaptor set in the "
                "Ensembl Gene Adaptor.\n");

        return NULL;
    }

    dbea = ensRegistryGetDatabaseentryadaptor(dba);

    objtype = ajStrNewC("Gene");

    gene->Databaseentries = ajListNew();

    ensDatabaseentryadaptorFetchAllbyObject(dbea,
                                            gene->Identifier,
                                            objtype,
                                            (AjPStr) NULL,
                                            ensEExternaldatabaseTypeNULL,
                                            gene->Databaseentries);

    ajStrDel(&objtype);

    return gene->Databaseentries;
}




/* @func ensGeneLoadTranscripts ***********************************************
**
** Load all Ensembl Transcript objects of an Ensembl Gene.
**
** This is not a simple accessor function, it will fetch
** Ensembl Transcript objects from an Ensembl SQL database in case the
** AJAX List is not defined.
**
** @cc Bio::EnsEMBL::Gene::get_all_Transcripts
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [const AjPList] AJAX List of Ensembl Transcript objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensGeneLoadTranscripts(EnsPGene gene)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscriptadaptor tca = NULL;

    if (!gene)
        return NULL;

    if (gene->Transcripts)
        return gene->Transcripts;

    if (!gene->Adaptor)
    {
        ajDebug("ensGeneLoadTranscripts cannot fetch "
                "Ensembl Transcript objects for an Ensembl Gene without an "
                "Ensembl Gene Adaptor.\n");

        return NULL;
    }

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    tca = ensRegistryGetTranscriptadaptor(dba);

    gene->Transcripts = ajListNew();

    ensTranscriptadaptorFetchAllbyGene(tca, gene, gene->Transcripts);

    return gene->Transcripts;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Set Set one member of a Gene
** @nam4rule Adaptor Set the Ensembl Gene Adaptor
** @nam4rule Biotype Set the biological type
** @nam4rule Canonicalannotation Set the canonical annotation
** @nam4rule Canonicaltranscriptidentifier
** Set the canonical Ensembl Transcript identifier
** @nam4rule Current Set the current attribute
** @nam4rule Date Set a date
** @nam5rule DateCreation Set the creation date
** @nam5rule DateModification Set the modification date
** @nam4rule Description Set the description
** @nam4rule Displayreference Set the Ensembl Database Entry
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Source Set the source
** @nam4rule Stableidentifier Set the stable identifier
** @nam4rule Status Set the status
** @nam4rule Version Set the version
**
** @argrule * gene [EnsPGene] Ensembl Gene object
** @argrule Adaptor ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @argrule Biotype biotype [AjPStr] Biological type
** @argrule Canonicalannotation canann [AjPStr] Canonical annotation
** @argrule Canonicaltranscriptidentifier cantrcid [ajuint]
** Canonical Ensembl Transcript identifier
** @argrule Current current [AjBool] Current attribute
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule DateModification mdate [AjPStr] Modification date
** @argrule Description description [AjPStr] Description
** @argrule Displayreference dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Source source [AjPStr] Source
** @argrule Stableidentifier stableid [AjPStr] Stable identifier
** @argrule Status status [EnsEGeneStatus] Status
** @argrule Version version [ajuint] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGeneSetAdaptor ****************************************************
**
** Set the Ensembl Gene Adaptor member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetAdaptor(EnsPGene gene, EnsPGeneadaptor ga)
{
    if (!gene)
        return ajFalse;

    gene->Adaptor = ga;

    return ajTrue;
}




/* @func ensGeneSetBiotype ****************************************************
**
** Set the biological type member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::biotype
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] biotype [AjPStr] Biological type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneSetBiotype(EnsPGene gene, AjPStr biotype)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->Biotype);

    gene->Biotype = ajStrNewRef(biotype);

    return ajTrue;
}




/* @func ensGeneSetCanonicalannotation ****************************************
**
** Set the canonical annotation member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_annotation
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] canann [AjPStr] Canonical annotation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneSetCanonicalannotation(EnsPGene gene, AjPStr canann)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->Canonicalannotation);

    gene->Canonicalannotation = ajStrNewRef(canann);

    return ajTrue;
}




/* @func ensGeneSetCanonicaltranscriptidentifier ******************************
**
** Set the canonical Ensembl Transcript identifier member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_transcript
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] cantrcid [ajuint] Canonical Ensembl Transcript identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: In this implementation, the Ensembl Gene only holds the identifier
** for the canonical Ensembl Transcript and not the object itself.
******************************************************************************/

AjBool ensGeneSetCanonicaltranscriptidentifier(EnsPGene gene, ajuint cantrcid)
{
    AjBool match = AJFALSE;

    AjIList iter = NULL;

    EnsPTranscript transcript = NULL;

    if (!gene)
        return ajFalse;

    /*
    ** If Ensembl Transcript objects are available, check if the
    ** identifier matches one. The ensGeneLoadTranscripts function
    ** is no longer called explicitly.
    */

    if (gene->Transcripts)
    {
        iter = ajListIterNewread(gene->Transcripts);

        while (!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            if (ensTranscriptGetIdentifier(transcript) ==
                gene->Canonicaltranscriptidentifier)
                ensTranscriptSetCanonical(transcript, ajFalse);

            if (ensTranscriptGetIdentifier(transcript) == cantrcid)
            {
                ensTranscriptSetCanonical(transcript, ajTrue);
                match = ajTrue;
                break;
            }
        }

        ajListIterDel(&iter);
    }

    if (cantrcid == 0U)
        match = ajTrue;

    if (match == ajTrue)
        gene->Canonicaltranscriptidentifier = cantrcid;

    return match;
}




/* @func ensGeneSetCurrent ****************************************************
**
** Set the current attribute of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::is_current
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] current [AjBool] Current attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetCurrent(EnsPGene gene, AjBool current)
{
    if (!gene)
        return ajFalse;

    gene->Current = current;

    return ajTrue;
}




/* @func ensGeneSetDateCreation ***********************************************
**
** Set the creation date member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::created_date
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneSetDateCreation(EnsPGene gene, AjPStr cdate)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->DateCreation);

    gene->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensGeneSetDateModification *******************************************
**
** Set the modification date member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::modified_date
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneSetDateModification(EnsPGene gene, AjPStr mdate)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->DateModification);

    gene->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensGeneSetDescription ************************************************
**
** Set the description member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::description
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetDescription(EnsPGene gene, AjPStr description)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->Description);

    gene->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGeneSetDisplayreference *******************************************
**
** Set the Ensembl Database Entry member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::display_xref
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneSetDisplayreference(EnsPGene gene, EnsPDatabaseentry dbe)
{
    if (!gene)
        return ajFalse;

    ensDatabaseentryDel(&gene->Displayreference);

    gene->Displayreference = ensDatabaseentryNewRef(dbe);

    return ajTrue;
}




/* @func ensGeneSetFeature ****************************************************
**
** Set the Ensembl Feature member of an Ensembl Gene.
**
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetFeature(EnsPGene gene, EnsPFeature feature)
{
    AjIList iter = NULL;

    EnsPSlice slice = NULL;

    EnsPTranscript oldtranscript = NULL;
    EnsPTranscript newtranscript = NULL;

    if (ajDebugTest("ensGeneSetFeature"))
        ajDebug("ensGeneSetFeature\n"
                "  gene %p\n"
                "  feature %p\n",
                gene,
                feature);

    if (!gene)
        return ajFalse;

    if (!feature)
        return ajFalse;

    /* Replace the current Feature. */

    if (gene->Feature)
        ensFeatureDel(&gene->Feature);

    gene->Feature = ensFeatureNewRef(feature);

    slice = ensFeatureGetSlice(gene->Feature);

    /* Transfer the Ensembl Transcript objects onto the new Ensembl Slice. */

    iter = ajListIterNew(gene->Transcripts);

    while (!ajListIterDone(iter))
    {
        oldtranscript = (EnsPTranscript) ajListIterGet(iter);

        ajListIterRemove(iter);

        newtranscript = ensTranscriptTransfer(oldtranscript, slice);

        if (!newtranscript)
        {
            ajDebug("ensGeneSetFeature could not transfer "
                    "Transcript onto new Ensembl Feature Slice.");

            ensTranscriptTrace(oldtranscript, 1);
        }

        ajListIterInsert(iter, (void *) newtranscript);

        /* Advance the AJAX List Iterator after the insert. */

        (void) ajListIterGet(iter);

        ensTranscriptDel(&oldtranscript);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensGeneSetIdentifier *************************************************
**
** Set the SQL database-internal identifier member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetIdentifier(EnsPGene gene, ajuint identifier)
{
    if (!gene)
        return ajFalse;

    gene->Identifier = identifier;

    return ajTrue;
}




/* @func ensGeneSetSource *****************************************************
**
** Set the source member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::source
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] source [AjPStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetSource(EnsPGene gene, AjPStr source)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->Source);

    gene->Source = ajStrNewRef(source);

    return ajTrue;
}




/* @func ensGeneSetStableidentifier *******************************************
**
** Set the stable identifier member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::stable_id
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneSetStableidentifier(EnsPGene gene, AjPStr stableid)
{
    if (!gene)
        return ajFalse;

    ajStrDel(&gene->Stableidentifier);

    gene->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensGeneSetStatus *****************************************************
**
** Set the status member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::status
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] status [EnsEGeneStatus] Status
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetStatus(EnsPGene gene, EnsEGeneStatus status)
{
    if (!gene)
        return ajFalse;

    gene->Status = status;

    return ajTrue;
}




/* @func ensGeneSetVersion ****************************************************
**
** Set the version member of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::version
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneSetVersion(EnsPGene gene, ajuint version)
{
    if (!gene)
        return ajFalse;

    gene->Version = version;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Add Add one object to an Ensembl Gene
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Databaseentry Add an Ensembl Database Entry
** @nam4rule Transcript Add an Ensembl Transcript
**
** @argrule * gene [EnsPGene] Ensembl Gene object
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Databaseentry dbe [EnsPDatabaseentry] Ensembl Database Entry
** @argrule Transcript transcript [EnsPTranscript] Ensembl Transcript
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGeneAddAttribute **************************************************
**
** Add an Ensembl Attribute to an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::add_Attributes
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneAddAttribute(EnsPGene gene, EnsPAttribute attribute)
{
    if (!gene)
        return ajFalse;

    if (!attribute)
        return ajFalse;

    if (!gene->Attributes)
        gene->Attributes = ajListNew();

    ajListPushAppend(gene->Attributes, (void *) ensAttributeNewRef(attribute));

    return ajTrue;
}




/* @func ensGeneAddDatabaseentry **********************************************
**
** Add an Ensembl Database Entry to an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::add_DBEntry
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneAddDatabaseentry(EnsPGene gene, EnsPDatabaseentry dbe)
{
    if (!gene)
        return ajFalse;

    if (!dbe)
        return ajFalse;

    if (!gene->Databaseentries)
        gene->Databaseentries = ajListNew();

    ajListPushAppend(gene->Databaseentries,
                     (void *) ensDatabaseentryNewRef(dbe));

    return ajTrue;
}




/* @func ensGeneAddTranscript *************************************************
**
** Add an Ensembl Transcript to an Ensembl Gene.
** This will also re-calculate gene coordinates.
**
** @cc Bio::EnsEMBL::Gene::add_Transcript
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneAddTranscript(EnsPGene gene, EnsPTranscript transcript)
{
    if (!gene)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!gene->Transcripts)
        gene->Transcripts = ajListNew();

    ajListPushAppend(gene->Transcripts,
                     (void *) ensTranscriptNewRef(transcript));

    ensGeneCalculateCoordinates(gene);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Trace Report Ensembl Gene members to debug file
**
** @argrule Trace gene [const EnsPGene] Ensembl Gene
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGeneTrace *********************************************************
**
** Trace an Ensembl Gene.
**
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneTrace(const EnsPGene gene, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPTranscript transcript = NULL;

    if (!gene)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGeneTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Displayreference %p\n"
            "%S  Description '%S'\n"
            "%S  Source '%S'\n"
            "%S  Biotype '%S'\n"
            "%S  Status %d\n"
            "%S  Current '%B'\n"
            "%S  Canonicalannotation '%S'\n"
            "%S  Canonicaltranscriptidentifier %u\n"
            "%S  Stableidentifier '%S'\n"
            "%S  DateCreation '%S'\n"
            "%S  DateModification '%S'\n"
            "%S  Version %u\n"
            "%S  Attributes %p\n"
            "%S  Databaseentries %p\n"
            "%S  Transcripts %p\n",
            indent, gene,
            indent, gene->Use,
            indent, gene->Identifier,
            indent, gene->Adaptor,
            indent, gene->Feature,
            indent, gene->Displayreference,
            indent, gene->Description,
            indent, gene->Source,
            indent, gene->Biotype,
            indent, gene->Status,
            indent, gene->Current,
            indent, gene->Canonicalannotation,
            indent, gene->Canonicaltranscriptidentifier,
            indent, gene->Stableidentifier,
            indent, gene->DateCreation,
            indent, gene->DateModification,
            indent, gene->Version,
            indent, gene->Attributes,
            indent, gene->Databaseentries,
            indent, gene->Transcripts);

    ensFeatureTrace(gene->Feature, level + 1);

    ensDatabaseentryTrace(gene->Displayreference, level + 1);

    /* Trace the AJAX List of Ensembl Attribute objects. */

    if (gene->Attributes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Attribute objects\n",
                indent, gene->Attributes);

        iter = ajListIterNewread(gene->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if (gene->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entry objects\n",
                indent, gene->Databaseentries);

        iter = ajListIterNewread(gene->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Transcript objects. */

    if (gene->Transcripts)
    {
        ajDebug("%S    AJAX List %p of Ensembl Transcript objects\n",
                indent, gene->Transcripts);

        iter = ajListIterNewread(gene->Transcripts);

        while (!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            ensTranscriptTrace(transcript, level + 2);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Calculate Calculate Ensembl Gene values
** @nam4rule Coordinates Calculate coordinates
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Coordinates gene [EnsPGene] Ensembl Gene
** @argrule Memsize gene [const EnsPGene] Ensembl Gene
**
** @valrule Coordinates [AjBool] ajTrue upon success, ajFalse otherwise
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGeneCalculateCoordinates ******************************************
**
** Calculate coordinates of an Ensembl Gene.
** This function should be called, whenever a Transcript of this Gene has
** been changed.
**
** @cc Bio::EnsEMBL::Gene::recalculate_coordinates
** @param [u] gene [EnsPGene] Ensembl Gene
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneCalculateCoordinates(EnsPGene gene)
{
    ajint start = 0;
    ajint end = 0;
    ajint strand = 0;

    AjBool transsplicing = AJFALSE;

    AjIList iter = NULL;
    const AjPList list = NULL;

    EnsPFeature gfeature = NULL;
    EnsPFeature tfeature = NULL;

    EnsPSlice slice = NULL;

    EnsPTranscript transcript = NULL;

    if (!gene)
        return ajFalse;

    list = ensGeneLoadTranscripts(gene);

    if (!ajListGetLength(list))
        return ajTrue;

    /* Get the first Transcript with coordinates. */

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        /* Skip missing Ensembl Transcript objects. */

        if (!transcript)
            continue;

        tfeature = ensTranscriptGetFeature(transcript);

        /* Skip un-mapped Ensembl Transcript objects. */

        if (!ensFeatureGetStart(tfeature))
            continue;

        slice = ensFeatureGetSlice(tfeature);

        start = ensFeatureGetStart(tfeature);

        end = ensFeatureGetEnd(tfeature);

        strand = ensFeatureGetStrand(tfeature);

        break;
    }

    ajListIterDel(&iter);

    /* Start loop after the first Transcript with coordinates. */

    iter = ajListIterNewread(list);

    while (!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        /* Skip missing Ensembl Transcript objects. */

        if (!transcript)
            continue;

        tfeature = ensTranscriptGetFeature(transcript);

        /* Skip un-mapped Ensembl Transcript objects. */

        if (!ensFeatureGetStart(tfeature))
            continue;

        if (!ensSliceMatch(ensFeatureGetSlice(tfeature), slice))
            ajFatal("ensGeneCalculateCoordinates got "
                    "Ensembl Transcript objects of one Ensembl Gene on "
                    "different Ensembl Slice objects.\n");

        start = (ensFeatureGetStart(tfeature) < start) ?
            ensFeatureGetStart(tfeature) :
            start;

        end = (ensFeatureGetEnd(tfeature) > end) ?
            ensFeatureGetEnd(tfeature) :
            end;

        if (ensFeatureGetStrand(tfeature) != strand)
            transsplicing = ajTrue;
    }

    ajListIterDel(&iter);

    if (transsplicing)
        ajWarn("ensGeneCalculateCoordinates got an Ensembl Gene suggesting "
               "a trans-splicing event.\n");

    gfeature = ensGeneGetFeature(gene);

    ensFeatureSetStart(gfeature, start);

    ensFeatureSetEnd(gfeature, end);

    ensFeatureSetStrand(gfeature, strand);

    ensFeatureSetSlice(gfeature, slice);

    /* Clear internal values that depend on Transcript coordinates. None! */

    return ajTrue;
}




/* @func ensGeneCalculateMemsize **********************************************
**
** Calculate the memory size in bytes of an Ensembl Gene.
**
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGeneCalculateMemsize(const EnsPGene gene)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPTranscript transcript = NULL;

    if (!gene)
        return 0;

    size += sizeof (EnsOGene);

    size += ensFeatureCalculateMemsize(gene->Feature);

    size += ensDatabaseentryCalculateMemsize(gene->Displayreference);

    if (gene->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Description);
    }

    if (gene->Source)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Source);
    }

    if (gene->Biotype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Biotype);
    }

    if (gene->Canonicalannotation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Canonicalannotation);
    }

    if (gene->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Stableidentifier);
    }

    if (gene->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->DateCreation);
    }

    if (gene->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->DateModification);
    }

    /* Summarise the AJAX List of Ensembl Attribute objects. */

    if (gene->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(gene->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if (gene->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(gene->Databaseentries);

        while (!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Transcript objects. */

    if (gene->Transcripts)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(gene->Transcripts);

        while (!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            size += ensTranscriptCalculateMemsize(transcript);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Fetch Fetch Ensembl Gene values
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Databaseentries Fetch all Ensembl Database Entry objects
** @nam5rule Exons Fetch all Ensembl Exon objects
** @nam4rule Canonicaltranscript Fetch the canonical Ensembl Transcript
** @nam4rule Displayidentifier Fetch the display identifier
**
** @argrule AllAttributes gene [EnsPGene] Ensembl Gene
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllDatabaseentries gene [EnsPGene] Ensembl Gene
** @argrule AllDatabaseentries name [const AjPStr]
** Ensembl External Database name
** @argrule AllDatabaseentries type [EnsEExternaldatabaseType]
** Ensembl External Database type
** @argrule AllDatabaseentries dbes [AjPList]
** AJAX List of Ensembl Database Entry objects
** @argrule AllExons gene [EnsPGene] Ensembl Gene
** @argrule AllExons exons [AjPList] AJAX List of Ensembl Exon objects
** @argrule Canonicaltranscript gene [EnsPGene] Ensembl Gene
** @argrule Canonicaltranscript Ptranscript [EnsPTranscript*]
** Ensembl Transcript address
** @argrule Displayidentifier gene [const EnsPGene] Ensembl Gene
** @argrule Displayidentifier Pidentifier [AjPStr*] AJAX String address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGeneFetchAllAttributes ********************************************
**
** Fetch all Ensembl Attribute objects of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::get_all_Attributes
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneFetchAllAttributes(EnsPGene gene,
                                 const AjPStr code,
                                 AjPList attributes)
{
    AjBool match = AJFALSE;

    const AjPList list = NULL;
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    if (!gene)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    list = ensGeneLoadAttributes(gene);

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




/* @func ensGeneFetchAllDatabaseentries ***************************************
**
** Fetch all Ensembl Database Entry objects of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::get_all_DBEntries
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] name [const AjPStr] Ensembl External Database name
** @param [u] type [EnsEExternaldatabaseType] Ensembl External Database type
** @param [u] dbes [AjPList] AJAX List of Ensembl Database Entry objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneFetchAllDatabaseentries(EnsPGene gene,
                                      const AjPStr name,
                                      EnsEExternaldatabaseType type,
                                      AjPList dbes)
{
    AjBool namematch = AJFALSE;
    AjBool typematch = AJFALSE;

    const AjPList list = NULL;
    AjIList iter       = NULL;

    EnsPDatabaseentry dbe = NULL;

    if (!gene)
        return ajFalse;

    if (!dbes)
        return ajFalse;

    list = ensGeneLoadDatabaseentries(gene);

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




/* @funcstatic listExonCompareAddressAscending ********************************
**
** Comparison function to sort Ensembl Exon objects via their address in
** ascending order. For identical Exon objects at separate addresses, which are
** matched via ensExonMatch 0 is returned.
**
** @param [r] item1 [const void*] Ensembl Exon 1
** @param [r] item2 [const void*] Ensembl Exon 2
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.2.0
** @@
******************************************************************************/

static int listExonCompareAddressAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPExon exon1 = *(EnsOExon *const *) item1;
    EnsPExon exon2 = *(EnsOExon *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listExonCompareAddressAscending"))
        ajDebug("listExonCompareAddressAscending\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    if (ensExonSimilarity(exon1, exon2) == ajTrue)
        return 0;

    if (exon1 < exon2)
        result = -1;

    if (exon1 > exon2)
        result = +1;

    return result;
}




/* @funcstatic listExonDelete *************************************************
**
** ajListSortUnique "itemdel" function to delete Ensembl Exon objects that
** are redundant.
**
** @param [r] Pitem [void**] Ensembl Exon address
** @param [r] cl [void*] Standard, passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void listExonDelete(void **Pitem, void *cl)
{
    if (!Pitem)
        return;

    (void) cl;

    ensExonDel((EnsPExon *) Pitem);

    return;
}




/* @func ensGeneFetchAllExons *************************************************
**
** Fetch all Ensembl Exon objects of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::get_all_Exons
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] exons [AjPList] AJAX List of Ensembl Exon objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneFetchAllExons(EnsPGene gene, AjPList exons)
{
    const AjPList elist = NULL;
    const AjPList tlist = NULL;

    AjIList eiter = NULL;
    AjIList titer = NULL;

    EnsPExon exon = NULL;

    EnsPTranscript transcript = NULL;

    if (!gene)
        return ajFalse;

    if (!exons)
        return ajFalse;

    tlist = ensGeneLoadTranscripts(gene);

    titer = ajListIterNewread(tlist);

    while (!ajListIterDone(titer))
    {
        transcript = (EnsPTranscript) ajListIterGet(titer);

        elist = ensTranscriptLoadExons(transcript);

        eiter = ajListIterNewread(elist);

        while (!ajListIterDone(eiter))
        {
            exon = (EnsPExon) ajListIterGet(eiter);

            ajListPushAppend(exons, (void *) ensExonNewRef(exon));
        }

        ajListIterDel(&eiter);
    }

    ajListIterDel(&titer);

    ajListSortUnique(exons, &listExonCompareAddressAscending, &listExonDelete);

    return ajTrue;
}




/* @func ensGeneFetchCanonicaltranscript **************************************
**
** Fetch the canonical Ensembl Transcript of an Ensembl Gene.
**
** The caller is repsonsible for deleting the Ensembl Transcript.
**
** @cc Bio::EnsEMBL::Gene::canonical_transcript
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [wP] Ptranscript [EnsPTranscript*] Ensembl Transcript address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneFetchCanonicaltranscript(EnsPGene gene,
                                       EnsPTranscript *Ptranscript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscriptadaptor tca = NULL;

    if (!gene)
        return ajFalse;

    if (!Ptranscript)
        return ajFalse;

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    tca = ensRegistryGetTranscriptadaptor(dba);

    ensTranscriptadaptorFetchByIdentifier(
        tca,
        gene->Canonicaltranscriptidentifier,
        Ptranscript);

    return ajTrue;
}




/* @func ensGeneFetchDisplayidentifier ****************************************
**
** Fetch the display identifier of an Ensembl Gene.
** This will return the stable identifier, the SQL database-internal identifier
** or the Gene memory address in this descending priority.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Gene::display_id
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [wP] Pidentifier [AjPStr*] AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGeneFetchDisplayidentifier(const EnsPGene gene,
                                     AjPStr *Pidentifier)
{
    if (!gene)
        return ajFalse;

    if (!Pidentifier)
        return ajFalse;

    if (gene->Stableidentifier &&
        ajStrGetLen(gene->Stableidentifier))
    {
        if (*Pidentifier)
            ajStrAssignS(Pidentifier, gene->Stableidentifier);
        else
            *Pidentifier = ajStrNewS(gene->Stableidentifier);
    }
    else if (gene->Identifier)
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier, "%u", gene->Identifier);
        else
            *Pidentifier = ajFmtStr("%u", gene->Identifier);
    }
    else
    {
        if (*Pidentifier)
            *Pidentifier = ajFmtPrintS(Pidentifier, "%p", gene);
        else
            *Pidentifier = ajFmtStr("%p", gene);
    }

    return ajTrue;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Gene objects
**
** @fdata [EnsPGene]
**
** @nam3rule Match      Test Ensembl Gene objects for identity
** @nam3rule Overlap    Test Ensembl Gene objects for overlap
** @nam3rule Similarity Test Ensembl Gene objects for similarity
**
** @argrule * gene1 [EnsPGene] Ensembl Gene
** @argrule * gene2 [EnsPGene] Ensembl Gene
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensGeneSimilarity ****************************************************
**
** Test Ensembl Gene objects for similarity.
**
** @cc Bio::EnsEMBL::Gene::equals
** @param [u] gene1 [EnsPGene] Ensembl Gene
** @param [u] gene2 [EnsPGene] Ensembl Gene
**
** @return [AjBool] ajTrue if the Ensembl Gene objects are similar
**
** @release 6.4.0
** @@
** NOTE: This function is similar to Bio::EnsEMBL::Gene::equals, but not
** completely identical. The Bio::EnsEMBL::Transcript objects are not sorted
** before comparison, as this would interfere with the internal AJAX List and
** their order in the Ensembl Gene.
******************************************************************************/

AjBool ensGeneSimilarity(EnsPGene gene1, EnsPGene gene2)
{
    AjBool mismatch = AJFALSE;
    AjBool similarity = AJFALSE;

    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    const AjPList transcripts1 = NULL;
    const AjPList transcripts2 = NULL;

    EnsPTranscript transcript1 = NULL;
    EnsPTranscript transcript2 = NULL;

    if (!gene1)
        return ajFalse;

    if (!gene2)
        return ajFalse;

    if (gene1 == gene2)
        return ajTrue;

    if (!ensFeatureSimilarity(gene1->Feature, gene2->Feature))
        return ajFalse;

    if (!ajStrMatchCaseS(gene1->Biotype, gene2->Biotype))
        return ajFalse;

    if ((gene1->Stableidentifier && gene2->Stableidentifier) &&
        (!ajStrMatchCaseS(gene1->Stableidentifier,
                          gene2->Stableidentifier)))
        return ajFalse;

    transcripts1 = ensGeneLoadTranscripts(gene1);
    transcripts2 = ensGeneLoadTranscripts(gene2);

    if (ajListGetLength(transcripts1) != ajListGetLength(transcripts2))
        return ajFalse;

    iter1 = ajListIterNewread(transcripts1);
    iter2 = ajListIterNewread(transcripts2);

    while (!ajListIterDone(iter1))
    {
        transcript1 = (EnsPTranscript) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        similarity = ajFalse;

        while (!ajListIterDone(iter2))
        {
            transcript2 = (EnsPTranscript) ajListIterGet(iter2);

            if (ensTranscriptSimilarity(transcript1, transcript2))
            {
                similarity = ajTrue;
                break;
            }
        }

        if (similarity == ajFalse)
        {
            mismatch = ajTrue;
            break;
        }
    }

    ajListIterDel(&iter1);
    ajListIterDel(&iter2);

    if (mismatch == ajTrue)
        return ajFalse;

    return ajTrue;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Gene objects between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPGene]
**
** @nam3rule Transfer Transfer an Ensembl Gene
** @nam3rule Transform Transform an Ensembl Gene
**
** @argrule * gene [EnsPGene] Ensembl Gene
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
**
** @valrule * [EnsPGene] Ensembl Gene or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGeneTransfer ******************************************************
**
** Transfer an Ensembl Gene onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Gene::transfer
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPGene] Ensembl Gene or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGene ensGeneTransfer(EnsPGene gene, EnsPSlice slice)
{
    EnsPFeature newfeature = NULL;

    EnsPGene newgene = NULL;

    if (ajDebugTest("ensGeneTransfer"))
        ajDebug("ensGeneTransfer\n"
                "  gene %p\n"
                "  slice %p\n",
                gene,
                slice);

    if (!gene)
        return NULL;

    if (!slice)
        return NULL;

    newfeature = ensFeatureTransfer(gene->Feature, slice);

    if (!newfeature)
    {
        ajDebug("ensGeneTransfer could not transfer the Ensembl Feature %p "
                "onto the new Slice %p.\n", gene->Feature, slice);

        ensFeatureTrace(gene->Feature, 1);

        ensSliceTrace(slice, 1);

        return NULL;
    }

    newgene = geneNewCpyFeatures(gene);

    ensGeneSetFeature(newgene, newfeature);

    ensFeatureDel(&newfeature);

    return newgene;
}




/* @func ensGeneTransform *****************************************************
**
** Transform an Ensembl Gene into another Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Gene::transform
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPGene] Ensembl Gene or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGene ensGeneTransform(EnsPGene gene,
                          const AjPStr csname,
                          const AjPStr csversion)
{
    ajint failures = 0;
    ajint minstart = INT_MAX;
    ajint maxend   = INT_MIN;

    ajuint psslength = 0U;

    AjIList iter = NULL;
    AjPList pss  = NULL;

    const AjPList transcripts = NULL;

    EnsPFeature newgf = NULL;
    EnsPFeature newtf = NULL;

    EnsPGene newgene = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPTranscript newtranscript = NULL;
    EnsPTranscript oldtranscript = NULL;

    if (!gene)
        return NULL;

    if (!csname)
        return NULL;

    if (ajStrGetLen(csname) == 0U)
        return NULL;

    newgf = ensFeatureTransform(gene->Feature,
                                csname,
                                csversion,
                                (EnsPSlice) NULL);

    if (!newgf)
    {
        /*
        ** Check if this Gene projects at all to the requested
        ** Coordinate System.
        */

        pss = ajListNew();

        ensFeatureProject(gene->Feature, csname, csversion, pss);

        psslength = (ajuint) ajListGetLength(pss);

        while (ajListPop(pss, (void **) &ps))
            ensProjectionsegmentDel(&ps);

        ajListFree(&pss);

        if (psslength == 0U)
            return NULL;
    }

    /* All Ensembl Transcript and Ensembl Exon objects need to be loaded. */

    transcripts = ensGeneLoadTranscripts(gene);

    iter = ajListIterNewread(transcripts);

    while (!ajListIterDone(iter))
    {
        oldtranscript = (EnsPTranscript) ajListIterGet(iter);

        ensTranscriptLoadExons(oldtranscript);
    }

    ajListIterDel(&iter);

    /*
    ** Copy the Ensembl Gene object and also its internal
    ** Ensembl Feature class-based members.
    */

    newgene = geneNewCpyFeatures(gene);

    if (newgene->Transcripts)
    {
        iter = ajListIterNew(newgene->Transcripts);

        while (!ajListIterDone(iter))
        {
            oldtranscript = (EnsPTranscript) ajListIterGet(iter);

            ajListIterRemove(iter);

            newtranscript = ensTranscriptTransform(oldtranscript,
                                                   csname,
                                                   csversion);

            if (newtranscript)
            {
                ajListIterInsert(iter, (void *) newtranscript);

                /* Advance the AJAX List Iterator after the insert. */

                (void) ajListIterGet(iter);
            }
            else
            {
                failures++;

                continue;
            }

            if (!newgf)
            {
                newtf = ensTranscriptGetFeature(newtranscript);

                minstart = (ensFeatureGetStart(newtf) < minstart)
                    ? ensFeatureGetStart(newtf) : minstart;

                maxend   = (ensFeatureGetEnd(newtf) > maxend)
                    ? ensFeatureGetEnd(newtf) : maxend;
            }

            ensTranscriptDel(&oldtranscript);
        }

        ajListIterDel(&iter);
    }

    if (failures)
    {
        ensFeatureDel(&newgf);

        ensGeneDel(&newgene);

        return NULL;
    }

    if (!newgf)
    {
        ajListPeekFirst(newgene->Transcripts, (void **) &newtranscript);

        newtf = ensTranscriptGetFeature(newtranscript);

        newgf = ensFeatureNewIniS(ensFeatureGetAnalysis(gene->Feature),
                                  ensFeatureGetSlice(newtf),
                                  minstart,
                                  maxend,
                                  ensFeatureGetStrand(newtf));
    }

    /*
    ** Set the Feature directly, since ensGeneSetFeature transfers all
    ** internal Ensembl Objects based on the Feature class onto the new
    ** Ensembl Feature Slice, which duplicates the work already done here.
    */

    ensFeatureDel(&newgene->Feature);

    newgene->Feature = newgf;

    /* Clear internal values that depend on Transcript coordinates. */

    return newgene;
}




/* @section check *************************************************************
**
** Check Ensembl Gene objects.
**
** @fdata [EnsPGene]
**
** @nam3rule Check Check Ensembl Gene object(s)
** @nam4rule Reference Check whether an Ensembl Gene object is the reference
**
** @argrule * gene [const EnsPGene] Ensembl Gene
** @argrule Reference Presult [AjBool*] Result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGeneCheckReference ************************************************
**
** Check, whether an Ensembl Gene is a reference or alternative allele.
**
** @cc Bio::EnsEMBL::Gene::is_reference
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] Presult [AjBool*] Result
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneCheckReference(const EnsPGene gene, AjBool *Presult)
{
    if (!gene)
        return ajFalse;

    if (!gene->Adaptor)
        return ajFalse;

    return ensGeneadaptorCheckReference(gene->Adaptor, gene, Presult);
}




/* @datasection [EnsEGeneStatus] Ensembl Gene Status **************************
**
** @nam2rule Gene Functions for manipulating
** Ensembl Gene objects
** @nam3rule GeneStatus Functions for manipulating
** Ensembl Gene Status enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Gene Status enumeration.
**
** @fdata [EnsEGeneStatus]
**
** @nam4rule From Ensembl Gene Status query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  status  [const AjPStr] Status string
**
** @valrule * [EnsEGeneStatus] Ensembl Gene Status enumeration or
**                             ensEGeneStatusNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGeneStatusFromStr *************************************************
**
** Convert an AJAX String into an Ensembl Gene Status enumeration.
**
** @param [r] status [const AjPStr] Status string
**
** @return [EnsEGeneStatus] Ensembl Gene Status enumeration or
**                          ensEGeneStatusNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEGeneStatus ensGeneStatusFromStr(const AjPStr status)
{
    register EnsEGeneStatus i = ensEGeneStatusNULL;

    EnsEGeneStatus estatus = ensEGeneStatusNULL;

    for (i = ensEGeneStatusNULL;
         geneKStatus[i];
         i++)
        if (ajStrMatchC(status, geneKStatus[i]))
            estatus = i;

    if (!estatus)
        ajDebug("ensGeneStatusFromStr encountered "
                "unexpected string '%S'.\n", status);

    return estatus;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Gene Status enumeration.
**
** @fdata [EnsEGeneStatus]
**
** @nam4rule To   Return Ensembl Gene Status enumeration
** @nam5rule Char Return C character string value
**
** @argrule To status [EnsEGeneStatus] Ensembl Gene Status enumeration
**
** @valrule Char [const char*] Ensembl Gene Status C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGeneStatusToChar **************************************************
**
** Convert an Ensembl Gene Status enumeration into a C-type (char *) string.
**
** @param [u] status [EnsEGeneStatus] Ensembl Gene Status enumeration
**
** @return [const char*] Ensembl Gene Status C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensGeneStatusToChar(EnsEGeneStatus status)
{
    register EnsEGeneStatus i = ensEGeneStatusNULL;

    for (i = ensEGeneStatusNULL;
         geneKStatus[i] && (i < status);
         i++);

    if (!geneKStatus[i])
        ajDebug("ensGeneStatusToChar encountered an "
                "out of boundary error on "
                "Ensembl Gene Status enumeration %d.\n",
                status);

    return geneKStatus[i];
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listGeneCompareEndAscending ************************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** Ensembl Feature end member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Gene address 1
** @param [r] item2 [const void*] Ensembl Gene address 2
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

static int listGeneCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPGene gene1 = *(EnsOGene *const *) item1;
    EnsPGene gene2 = *(EnsOGene *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGeneCompareEndAscending"))
        ajDebug("listGeneCompareEndAscending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gene1 && (!gene2))
        return -1;

    if ((!gene1) && (!gene2))
        return 0;

    if ((!gene1) && gene2)
        return +1;

    return ensFeatureCompareEndAscending(gene1->Feature, gene2->Feature);
}




/* @funcstatic listGeneCompareEndDescending ***********************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** Ensembl Feature end member in descending order.
**
** @param [r] item1 [const void*] Ensembl Gene address 1
** @param [r] item2 [const void*] Ensembl Gene address 2
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

static int listGeneCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPGene gene1 = *(EnsOGene *const *) item1;
    EnsPGene gene2 = *(EnsOGene *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGeneCompareEndDescending"))
        ajDebug("listGeneCompareEndDescending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gene1 && (!gene2))
        return -1;

    if ((!gene1) && (!gene2))
        return 0;

    if ((!gene1) && gene2)
        return +1;

    return ensFeatureCompareEndDescending(gene1->Feature, gene2->Feature);
}




/* @funcstatic listGeneCompareIdentifierAscending *****************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** identifier member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Gene address 1
** @param [r] item2 [const void*] Ensembl Gene address 2
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

static int listGeneCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPGene gene1 = *(EnsOGene *const *) item1;
    EnsPGene gene2 = *(EnsOGene *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGeneCompareIdentifierAscending"))
        ajDebug("listGeneCompareIdentifierAscending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gene1 && (!gene2))
        return -1;

    if ((!gene1) && (!gene2))
        return 0;

    if ((!gene1) && gene2)
        return +1;

    if (gene1->Identifier < gene2->Identifier)
        return -1;

    if (gene1->Identifier > gene2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listGeneCompareStartAscending **********************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** Ensembl Feature start member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Gene address 1
** @param [r] item2 [const void*] Ensembl Gene address 2
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

static int listGeneCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPGene gene1 = *(EnsOGene *const *) item1;
    EnsPGene gene2 = *(EnsOGene *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGeneCompareStartAscending"))
        ajDebug("listGeneCompareStartAscending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gene1 && (!gene2))
        return -1;

    if ((!gene1) && (!gene2))
        return 0;

    if ((!gene1) && gene2)
        return +1;

    return ensFeatureCompareStartAscending(gene1->Feature, gene2->Feature);
}




/* @funcstatic listGeneCompareStartDescending *********************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** Ensembl Feature start member in descending order.
**
** @param [r] item1 [const void*] Ensembl Gene address 1
** @param [r] item2 [const void*] Ensembl Gene address 2
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

static int listGeneCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPGene gene1 = *(EnsOGene *const *) item1;
    EnsPGene gene2 = *(EnsOGene *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGeneCompareStartDescending"))
        ajDebug("listGeneCompareStartDescending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gene1 && (!gene2))
        return -1;

    if ((!gene1) && (!gene2))
        return 0;

    if ((!gene1) && gene2)
        return +1;

    return ensFeatureCompareStartDescending(gene1->Feature, gene2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Gene Functions for manipulating AJAX List objects of
** Ensembl Gene objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * genes [AjPList] AJAX List of Ensembl Gene objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListGeneSortEndAscending ******************************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** Ensembl Feature end member in ascending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGeneSortEndAscending(AjPList genes)
{
    if (!genes)
        return ajFalse;

    ajListSortTwoThree(genes,
                       &listGeneCompareEndAscending,
                       &listGeneCompareStartAscending,
                       &listGeneCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGeneSortEndDescending *****************************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** Ensembl Feature end member in descending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGeneSortEndDescending(AjPList genes)
{
    if (!genes)
        return ajFalse;

    ajListSortTwoThree(genes,
                       &listGeneCompareEndDescending,
                       &listGeneCompareStartDescending,
                       &listGeneCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGeneSortIdentifierAscending ***********************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** identifier member in ascending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGeneSortIdentifierAscending(AjPList genes)
{
    if (!genes)
        return ajFalse;

    ajListSort(genes, &listGeneCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGeneSortStartAscending ****************************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** Ensembl Feature start member in ascending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGeneSortStartAscending(AjPList genes)
{
    if (!genes)
        return ajFalse;

    ajListSortTwoThree(genes,
                       &listGeneCompareStartAscending,
                       &listGeneCompareEndAscending,
                       &listGeneCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListGeneSortStartDescending ***************************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** Ensembl Feature start member in descending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListGeneSortStartDescending(AjPList genes)
{
    if (!genes)
        return ajFalse;

    ajListSortTwoThree(genes,
                       &listGeneCompareStartDescending,
                       &listGeneCompareEndDescending,
                       &listGeneCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [AjPSeq] AJAX Sequence ****************************************
**
** @nam2rule Sequence Functions for manipulating AJAX Sequence objects
**
******************************************************************************/




/* @section add ***************************************************************
**
** Functions for manipulating AJAX Sequence objects.
**
** @fdata [AjPSeq]
**
** @nam3rule Add Add to an AJAX Sequence
** @nam4rule Feature Add an AJAX Feature
** @nam5rule Gene Convert an Ensembl Gene into an AJAX Feature
**
** @argrule * seq [AjPSeq] AJAX Sequence
** @argrule Gene gene [EnsPGene] Ensembl Gene
** @argrule Gene Pfeature [AjPFeature*] AJAX Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceAddFeatureGene ********************************************
**
** Convert an Ensembl Gene into an AJAX Feature and add it to the
** AJAX Feature Table of an AJAX Sequence. Also recursively convert and add
** all Ensembl Transcript and Ensembl Exon objects.
**
** @param [u] seq [AjPSeq] AJAX Sequence
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [wP] Pfeature [AjPFeature*] AJAX Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensSequenceAddFeatureGene(AjPSeq seq,
                                 EnsPGene gene,
                                 AjPFeature *Pfeature)
{
    AjPFeature feature = NULL;

    AjIList iter = NULL;
    const AjPList transcripts = NULL;

    AjPStr label = NULL;
    AjPStr type  = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene newgene = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPTranscript transcript = NULL;

    if (!seq)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!Pfeature)
        return ajFalse;

    *Pfeature = NULL;

    /*
    ** NOTE: An AJAX Sequence accessor function returning the
    ** AJAX Feature Table in modifiable form is missing.
    */

    if (!seq->Fttable)
        return ajFalse;

    /*
    ** Get the Ensembl Slice covering the AJAX Sequence object and transfer
    ** the Ensembl Gene object onto it.
    */

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    sla = ensRegistryGetSliceadaptor(dba);

    ensSliceadaptorFetchByName(sla, ajSeqGetNameS(seq), &slice);

    if (!slice)
    {
        ajDebug("ensSequenceAddFeatureGene could not fetch an "
                "Ensembl Slice for AJAX Sequence name '%S'.\n",
                ajSeqGetNameS(seq));

        return ajFalse;
    }

    newgene = ensGeneTransfer(gene, slice);

    if (!newgene)
    {
        ajDebug("ensSequenceAddFeatureGene could not transfer "
                "Ensembl Gene %p onto "
                "Ensembl Slice %p.\n", gene, slice);

        ensGeneTrace(gene, 1);
        ensSliceTrace(slice, 1);

        ensSliceDel(&slice);

        return ajFalse;
    }

    /* Convert the Ensembl Gene into a parent AJAX Feature. */

    type = ajStrNewC("gene");

    *Pfeature = ajFeatNewNucFlags(
        seq->Fttable,
        ensAnalysisGetName(ensFeatureGetAnalysis(newgene->Feature)),
        type,
        ensFeatureGetStart(newgene->Feature),
        ensFeatureGetEnd(newgene->Feature),
        0.0F,
        ensFeatureCalculateStrand(newgene->Feature),
        0, /* Frame */
        0, /* Exon number */
        0, /* Start 2 */
        0, /* End 2 */
        (AjPStr) NULL, /* Remote Identifier */
        (AjPStr) NULL, /* Label */
        0);

    ensGeneFetchDisplayidentifier(gene, &label);

    ajFeatTagAddCS(*Pfeature, "standard_name", label);

    dbe = ensGeneGetDisplayreference(gene);

    if (dbe)
        ajFeatTagAddCS(*Pfeature,
                       "gene",
                       ensDatabaseentryGetDisplayidentifier(dbe));

    /* Add Ensembl Transcript objects as sub-AJAX Feature objects. */

    transcripts = ensGeneLoadTranscripts(newgene);

    iter = ajListIterNewread(transcripts);

    while (!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        ensSequenceAddFeatureTranscript(seq, transcript, &feature);
    }

    ajListIterDel(&iter);

    ajStrDel(&label);
    ajStrDel(&type);

    ensGeneDel(&newgene);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @datasection [EnsPGeneadaptor] Ensembl Gene Adaptor ************************
**
** @nam2rule Geneadaptor Functions for manipulating
** Ensembl Gene Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor
** @cc CVS Revision: 1.198
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic geneadaptorFetchAllbyStatement *********************************
**
** Fetch all Ensembl Gene objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** NOTE: In this implementation, the Ensembl Gene Adaptor does not LEFT JOIN
** the external_db table. Due to the rather small number of Ensembl External
** Database objects, potentially linked to a large number of Ensembl External
** Reference objects, all External Database objects are cached in the adaptor.
******************************************************************************/

static AjBool geneadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList genes)
{
    ajuint identifier = 0U;
    ajuint analysisid = 0U;
    ajuint erid       = 0U;
    ajuint edbid      = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    ajuint cantrcid = 0U;
    ajuint version  = 0U;

    AjBool current = AJFALSE;

    EnsEGeneStatus estatus =
        ensEGeneStatusNULL;

    EnsEExternalreferenceInfotype erit = ensEExternalreferenceInfotypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr description   = NULL;
    AjPStr source        = NULL;
    AjPStr biotype       = NULL;
    AjPStr status        = NULL;
    AjPStr canann        = NULL;
    AjPStr stableid      = NULL;
    AjPStr cdate         = NULL;
    AjPStr mdate         = NULL;
    AjPStr erprimaryid   = NULL;
    AjPStr erdisplayid   = NULL;
    AjPStr erversion     = NULL;
    AjPStr erdescription = NULL;
    AjPStr erinfotype    = NULL;
    AjPStr erinfotext    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPFeature feature = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    if (ajDebugTest("geneadaptorFetchAllbyStatement"))
        ajDebug("geneadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  genes %p\n",
                ba,
                statement,
                am,
                slice,
                genes);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!genes)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);
    ga   = ensRegistryGetGeneadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier    = 0U;
        srid          = 0U;
        srstart       = 0U;
        srend         = 0U;
        srstrand      = 0;
        analysisid    = 0U;
        erid          = 0U;
        description   = ajStrNew();
        source        = ajStrNew();
        biotype       = ajStrNew();
        status        = ajStrNew();
        current       = ajFalse;
        cantrcid      = 0U;
        canann        = ajStrNew();
        stableid      = ajStrNew();
        version       = 0U;
        cdate         = ajStrNew();
        mdate         = ajStrNew();
        edbid         = 0U;
        erprimaryid   = ajStrNew();
        erdisplayid   = ajStrNew();
        erversion     = ajStrNew();
        erdescription = ajStrNew();
        erinfotype    = ajStrNew();
        erinfotext    = ajStrNew();

        estatus = ensEGeneStatusNULL;
        erit    = ensEExternalreferenceInfotypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToUint(sqlr, &erid);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &source);
        ajSqlcolumnToStr(sqlr, &biotype);
        ajSqlcolumnToStr(sqlr, &status);
        ajSqlcolumnToBool(sqlr, &current);
        ajSqlcolumnToUint(sqlr, &cantrcid);
        ajSqlcolumnToStr(sqlr, &canann);
        ajSqlcolumnToStr(sqlr, &stableid);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &mdate);
        ajSqlcolumnToUint(sqlr, &edbid);
        ajSqlcolumnToStr(sqlr, &erprimaryid);
        ajSqlcolumnToStr(sqlr, &erdisplayid);
        ajSqlcolumnToStr(sqlr, &erversion);
        ajSqlcolumnToStr(sqlr, &erdescription);
        ajSqlcolumnToStr(sqlr, &erinfotype);
        ajSqlcolumnToStr(sqlr, &erinfotext);

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
            ajStrDel(&description);
            ajStrDel(&source);
            ajStrDel(&biotype);
            ajStrDel(&status);
            ajStrDel(&canann);
            ajStrDel(&stableid);
            ajStrDel(&cdate);
            ajStrDel(&mdate);
            ajStrDel(&erprimaryid);
            ajStrDel(&erdisplayid);
            ajStrDel(&erversion);
            ajStrDel(&erdescription);
            ajStrDel(&erinfotype);
            ajStrDel(&erinfotext);

            continue;
        }

        if (erid)
        {
            ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

            erit = ensExternalreferenceInfotypeFromStr(erinfotype);

            if (!erit)
                ajDebug("geneadaptorFetchAllbyStatement encountered "
                        "unexpected string '%S' in the "
                        "'xref.infotype' field.\n", erinfotype);

            dbe = ensDatabaseentryNewIni((EnsPDatabaseentryadaptor) NULL,
                                         erid,
                                         (EnsPAnalysis) NULL,
                                         edb,
                                         erprimaryid,
                                         erdisplayid,
                                         erversion,
                                         erdescription,
                                         (AjPStr) NULL,
                                         erinfotext,
                                         erit,
                                         ensEExternalreferenceObjecttypeGene,
                                         identifier);

            ensExternaldatabaseDel(&edb);
        }

        /* Set the Gene status. */

        estatus = ensGeneStatusFromStr(status);

        if (!estatus)
            ajDebug("geneadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'gene.status' field.\n", status);

        /* Finally, create a new Ensembl Gene. */

        gene = ensGeneNewIni(ga,
                             identifier,
                             feature,
                             dbe,
                             description,
                             source,
                             biotype,
                             estatus,
                             current,
                             cantrcid,
                             canann,
                             stableid,
                             version,
                             cdate,
                             mdate,
                             (AjPList) NULL);

        ajListPushAppend(genes, (void *) gene);

        ensDatabaseentryDel(&dbe);

        ensFeatureDel(&feature);

        ajStrDel(&description);
        ajStrDel(&source);
        ajStrDel(&biotype);
        ajStrDel(&status);
        ajStrDel(&canann);
        ajStrDel(&stableid);
        ajStrDel(&cdate);
        ajStrDel(&mdate);
        ajStrDel(&erprimaryid);
        ajStrDel(&erdisplayid);
        ajStrDel(&erversion);
        ajStrDel(&erdescription);
        ajStrDel(&erinfotype);
        ajStrDel(&erinfotext);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Gene Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Gene Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGeneadaptor] Ensembl Gene Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGeneadaptorNew ****************************************************
**
** Default constructor for an Ensembl Gene Adaptor.
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
** @see ensRegistryGetGeneadaptor
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGeneadaptor] Ensembl Gene Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGeneadaptor ensGeneadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPGeneadaptor ga = NULL;

    if (!dba)
        return NULL;

    AJNEW0(ga);

    ga->Adaptor = ensFeatureadaptorNew(
        dba,
        geneadaptorKTables,
        geneadaptorKColumns,
        geneadaptorKLeftjoin,
        (const char *) NULL,
        (const char *) NULL,
        &geneadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensGeneNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensGeneDel,
        (size_t (*)(const void *)) &ensGeneCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensGeneGetFeature,
        "Gene");

    return ga;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Gene Adaptor object.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Gene Adaptor
**
** @argrule * Pga [EnsPGeneadaptor*] Ensembl Gene Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGeneadaptorDel ****************************************************
**
** Default destructor for an Ensembl Gene Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pga [EnsPGeneadaptor*] Ensembl Gene Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGeneadaptorDel(EnsPGeneadaptor *Pga)
{
    EnsPGeneadaptor pthis = NULL;

    if (!Pga)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGeneadaptorDel"))
        ajDebug("ensGeneadaptorDel\n"
                "  *Pga %p\n",
                *Pga);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pga)
        return;

    pthis = *Pga;

    ensFeatureadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pga = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Gene Adaptor object.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule Get Return Ensembl Gene Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * ga [const EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @valrule Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensGeneadaptorGetDatabaseadaptor *************************************
**
** Get the Ensembl Database Adaptor member of the
** Ensembl Feature Adaptor member of an Ensembl Gene Adaptor.
**
** @param [r] ga [const EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGeneadaptorGetDatabaseadaptor(const EnsPGeneadaptor ga)
{
    return (ga) ? ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor) : NULL;
}




/* @func ensGeneadaptorGetFeatureadaptor **************************************
**
** Get the Ensembl Feature Adaptor member of an Ensembl Gene Adaptor.
**
** @param [r] ga [const EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensGeneadaptorGetFeatureadaptor(const EnsPGeneadaptor ga)
{
    return (ga) ? ga->Adaptor : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Gene objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule Fetch Fetch Ensembl Gene object(s)
** @nam4rule All Fetch all Ensembl Gene objects
** @nam4rule Allby Fetch all Ensembl Gene objects matching a criterion
** @nam5rule Biotype Fetch all by biological type
** @nam5rule Slice   Fetch all by an Ensembl Slice
** @nam5rule Stableidentifier Fetch all by a stable Ensembl Gene identifier
** @nam4rule By Fetch one Ensembl Gene object matching a criterion
** @nam5rule Displaylabel
** Fetch by a display label
** @nam5rule Exonidentifier
** Fetch by an Ensembl Exon identifier
** @nam5rule Exonstableidentifier
** Fetch by an Ensembl Exon stable identifier
** @nam5rule Identifier
** Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier
** Fetch by an Ensembl Gene stable identifier
** @nam5rule Transcriptidentifier
** Fetch by an Ensembl Transcript identifier
** @nam5rule Transcriptstableidentifier
** Fetch by an Ensembl Transcript stable identifier
** @nam5rule Translationidentifier
** Fetch by an Ensembl Translation identifier
** @nam5rule Translationstableidentifier
** Fetch by an Ensembl Translation stable identifier
**
** @argrule * ga [EnsPGeneadaptor]
** Ensembl Gene Adaptor
** @argrule All genes [AjPList]
** AJAX List of Ensembl Gene objects
** @argrule AllbyBiotype biotype [const AjPStr]
** Biotype
** @argrule AllbySlice slice [EnsPSlice]
** Ensembl Slice
** @argrule AllbySlice anname [const AjPStr]
** Ensembl Analysis name
** @argrule AllbySlice source [const AjPStr]
** Source name
** @argrule AllbySlice biotype [const AjPStr]
** Biotype name
** @argrule AllbySlice loadtranscripts [AjBool]
** Load Ensembl Transcript objects
** @argrule AllbyStableidentifier stableid [const AjPStr]
** Ensembl Gene stable identifier
** @argrule Allby genes [AjPList]
** AJAX List of Ensembl Gene objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByExonidentifier identifier [ajuint]
** Ensembl Exon identifier
** @argrule ByExonstableidentifier stableid [const AjPStr]
** Ensembl Exon stable identifier
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByStableidentifier stableid [const AjPStr]
** Ensembl Gene stable identifier
** @argrule ByStableidentifier version [ajuint]
** Version
** @argrule ByTranscriptidentifier identifier [ajuint]
** Ensembl Transcript identifier
** @argrule ByTranscriptstableidentifier stableid [const AjPStr]
** Ensembl Transcript stable identifier
** @argrule ByTranslationidentifier identifier [ajuint]
** Ensembl Translation identifier
** @argrule ByTranslationstableidentifier stableid [const AjPStr]
** Ensembl Translation stable identifier
** @argrule By Pgene [EnsPGene*] Ensembl Gene address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGeneadaptorFetchAll ***********************************************
**
** Fetch all Ensembl Gene objects.
**
** The caller is responsible for deleting the Ensembl Gene objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_all
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAll(
    EnsPGeneadaptor ga,
    AjPList genes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ga)
        return ajFalse;

    if (!genes)
        return ajFalse;

    constraint = ajStrNewC(
        "gene.biotype != 'LRG_gene' "
        "AND "
        "gene.is_current = 1");

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
                                                constraint,
                                                (EnsPAssemblymapper) NULL,
                                                (EnsPSlice) NULL,
                                                genes);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGeneadaptorFetchAllbyBiotype **************************************
**
** Fetch all Ensembl Gene objects via a biotype.
**
** The caller is responsible for deleting the Ensembl Gene objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_all_by_biotype
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] biotype [const AjPStr] Biotype
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAllbyBiotype(
    EnsPGeneadaptor ga,
    const AjPStr biotype,
    AjPList genes)
{
    char *txtbiotype = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ga)
        return ajFalse;

    if (!biotype)
        return ajFalse;

    if (!genes)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtbiotype, biotype);

    constraint = ajFmtStr(
        "gene.is_current = 1 "
        "AND "
        "gene.biotype = '%s'",
        txtbiotype);

    ajCharDel(&txtbiotype);

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       genes);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGeneadaptorFetchAllbySlice ****************************************
**
** Fetch all Ensembl Genes via an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Genes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_all_by_Slice
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] source [const AjPStr] Source name
** @param [r] biotype [const AjPStr] Biotype name
** @param [r] loadtranscripts [AjBool] Load Ensembl Transcript objects
** @param [u] genes [AjPList] AJAX List of Ensembl Genes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAllbySlice(
    EnsPGeneadaptor ga,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr source,
    const AjPStr biotype,
    AjBool loadtranscripts,
    AjPList genes)
{
    void **keyarray = NULL;

    char *txtsource  = NULL;
    char *txtbiotype = NULL;

    ajint start = INT_MAX;
    ajint end   = INT_MIN;

    register ajuint i = 0U;

    ajuint gnid = 0U;
    ajuint trid = 0U;

    ajuint *Pidentifier = NULL;

    AjIList iter        = NULL;
    AjPList transcripts = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr constraint = NULL;
    AjPStr csv        = NULL;
    AjPStr statement  = NULL;

    AjPTable gntable = NULL;
    AjPTable trtable = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPGene gene = NULL;

    EnsPSlice newslice  = NULL;
    EnsPSliceadaptor sa = NULL;

    EnsPTranscript oldtranscript = NULL;
    EnsPTranscript newtranscript = NULL;
    EnsPTranscriptadaptor tca    = NULL;

    if (!ga)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!genes)
        return ajFalse;

    dba = ensGeneadaptorGetDatabaseadaptor(ga);

    constraint = ajStrNewC("gene.is_current = 1");

    if (source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        ajFmtPrintAppS(&constraint, " AND gene.source = '%s'", txtsource);

        ajCharDel(&txtsource);
    }

    if (biotype && ajStrGetLen(biotype))
    {
        ensDatabaseadaptorEscapeC(dba, &txtbiotype, biotype);

        ajFmtPrintAppS(&constraint, " AND gene.biotype = '%s'", txtbiotype);

        ajCharDel(&txtbiotype);
    }

    ensFeatureadaptorFetchAllbySlice(ga->Adaptor,
                                     slice,
                                     constraint,
                                     anname,
                                     genes);

    ajStrDel(&constraint);

    /* If there are less than two genes, still do lazy-loading. */

    if ((!loadtranscripts) || (ajListGetLength(genes) == 0U))
        return ajTrue;

    /*
    ** Preload all Ensembl Transcript objects now, instead of lazy loading
    ** later, which is faster than one query per Ensembl Transcript.
    ** First check if Ensembl Transcript objects are already preloaded.
    ** TODO: This should check all Ensembl Transcript objects.
    */

    ajListPeekFirst(genes, (void **) &gene);

    if (gene->Transcripts)
        return ajTrue;

    tca = ensRegistryGetTranscriptadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    /* Get the extent of the region spanned by Ensembl Transcript objects. */

    csv = ajStrNew();

    gntable = ajTableuintNew(ajListGetLength(genes));
    trtable = ajTableuintNew(ajListGetLength(genes));

    ajTableSetDestroyvalue(gntable, (void (*)(void **)) &ensGeneDel);
    ajTableSetDestroyvalue(trtable, (void (*)(void **)) &ensGeneDel);

    iter = ajListIterNew(genes);

    while (!ajListIterDone(iter))
    {
        gene = (EnsPGene) ajListIterGet(iter);

        gnid = ensGeneGetIdentifier(gene);

        feature = ensGeneGetFeature(gene);

        start = (ensFeatureGetSeqregionStart(feature) < start) ?
            ensFeatureGetSeqregionStart(feature) : start;

        end = (ensFeatureGetSeqregionEnd(feature) > end) ?
            ensFeatureGetSeqregionEnd(feature) : end;

        ajFmtPrintAppS(&csv, "%u, ", gnid);

        /*
        ** Put all Ensembl Genes into an AJAX Table indexed by their
        ** identifier.
        */

        if (ajTableMatchV(gntable, (const void *) &gnid))
            ajDebug("ensGeneadaptorFetchAllbySlice got duplicate "
                    "Ensembl Gene with identifier %u.\n", gnid);
        else
        {
            AJNEW0(Pidentifier);

            *Pidentifier = gnid;

            ajTablePut(gntable,
                       (void *) Pidentifier,
                       (void *) ensGeneNewRef(gene));
        }
    }

    ajListIterDel(&iter);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    if ((start >= ensSliceGetStart(slice)) && (end <= ensSliceGetEnd(slice)))
        newslice = ensSliceNewRef(slice);
    else
        ensSliceadaptorFetchBySlice(sa,
                                    slice,
                                    start,
                                    end,
                                    ensSliceGetStrand(slice),
                                    &newslice);

    /* Associate Transcript identifiers with Genes. */

    statement = ajFmtStr(
        "SELECT "
        "transcript.transcript_id, "
        "transcript.gene_id "
        "FROM "
        "transcript "
        "WHERE "
        "transcript.gene_id IN (%S)",
        csv);

    ajStrAssignClear(&csv);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        trid = 0U;
        gnid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
        ajSqlcolumnToUint(sqlr, &gnid);

        if (ajTableMatchV(trtable, (const void *) &trid))
        {
            ajDebug("ensGeneadaptorFetchAllbySlice got duplicate Transcript "
                    "with identifier %u for Gene with identifier %u.\n",
                    trid, gnid);

            continue;
        }

        gene = (EnsPGene) ajTableFetchmodV(gntable, (const void *) &gnid);

        if (gene)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = trid;

            ajTablePut(trtable,
                       (void *) Pidentifier,
                       (void *) ensGeneNewRef(gene));
        }
        else
            ajDebug("ensGeneadaptorFetchAllbySlice could not get Gene with "
                    "identifier %u for Transcript with identifier %u.\n",
                    gnid, trid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    /* Get all Transcript identifiers as comma-separated values. */

    ajTableToarrayKeys(trtable, &keyarray);

    for (i = 0U; keyarray[i]; i++)
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint *) keyarray[i]));

    AJFREE(keyarray);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    constraint = ajFmtStr("transcript.transcript_id IN (%S)", csv);

    ajStrDel(&csv);

    transcripts = ajListNew();

    ensTranscriptadaptorFetchAllbySlice(tca,
                                        newslice,
                                        anname,
                                        constraint,
                                        ajTrue,
                                        transcripts);

    ajStrDel(&constraint);

    /*
    ** Transfer Ensembl Transcript objects onto the Gene Slice, and add them
    ** to Gene objects.
    */

    while (ajListPop(transcripts, (void **) &oldtranscript))
    {
        newtranscript = ensTranscriptTransfer(oldtranscript, newslice);

        if (!newtranscript)
            ajFatal("ensGeneAdaptorFetchAllbySlice could not transfer "
                    "Transcript onto new Slice.\n");

        trid = ensTranscriptGetIdentifier(newtranscript);

        gene = (EnsPGene) ajTableFetchmodV(trtable, (const void *) &trid);

        ensGeneAddTranscript(gene, newtranscript);

        ensTranscriptDel(&newtranscript);
        ensTranscriptDel(&oldtranscript);
    }

    ajListFree(&transcripts);

    /*
    ** Clear and delete the AJAX Table objects of
    ** AJAX unsigned integer key (Gene and Transcript identifier) and
    ** Ensembl Gene value data.
    */

    ajTableDel(&gntable);
    ajTableDel(&trtable);

    ensSliceDel(&newslice);

    return ajTrue;
}




/* @func ensGeneadaptorFetchAllbyStableidentifier *****************************
**
** Fetch all Ensembl Genes of all version via an
** Ensembl Gene stable identifier.
**
** The caller is responsible for deleting the Ensembl Genes before deleting
** the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_all_versions_by_stable_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] stableid [const AjPStr] Ensembl Gene stable identifier
** @param [u] genes [AjPList] AJAX List of Ensembl Genes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAllbyStableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    AjPList genes)
{
    char *txtstableid = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ga)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!genes)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("gene.stable_id = '%s'", txtstableid);

    ajCharDel(&txtstableid);

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       genes);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGeneadaptorFetchByDisplaylabel ************************************
**
** Fetch an Ensembl Gene via its display label.
**
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_display_label
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] label [const AjPStr] Display label
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByDisplaylabel(
    EnsPGeneadaptor ga,
    const AjPStr label,
    EnsPGene *Pgene)
{
    char *txtlabel = NULL;

    AjPList genes = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGene gene = NULL;

    if (!ga)
        return ajFalse;

    if (!label)
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtlabel, label);

    constraint = ajFmtStr(
        "gene.is_current = 1 "
        "AND "
        "xref.display_label = '%s'",
        txtlabel);

    ajCharDel(&txtlabel);

    genes = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       genes);

    if (ajListGetLength(genes) > 1)
        ajDebug("ensGeneadaptorFetchByDisplaylabel got more than one "
                "Gene for display label '%S'.\n", label);

    ajListPop(genes, (void **) Pgene);

    while (ajListPop(genes, (void **) &gene))
        ensGeneDel(&gene);

    ajListFree(&genes);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGeneadaptorFetchByExonidentifier **********************************
**
** Fetch an Ensembl Gene via an Ensembl Exon identifier.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_exon_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] identifier [ajuint] Ensembl Exon identifier
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByExonidentifier(
    EnsPGeneadaptor ga,
    ajuint identifier,
    EnsPGene *Pgene)
{
    ajuint gid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ga)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor);

    statement = ajFmtStr(
        "SELECT "
        "transcript.gene_id "
        "FROM "
        "transcript, "
        "exon_transcript, "
        "exon "
        "WHERE "
        "transcript.transcript_id = exon_transcript.transcript_id "
        "AND "
        "exon_transcript.exon_id = exon.exon_id "
        "AND "
        "exon.exon_id = %u "
        "AND "
        "exon.is_current = 1",
        identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!gid)
    {
        ajDebug("ensGeneadaptorFetchByExonidentifier "
                "could not get Gene identifier for Exon "
                "identifier '%S'.\n", identifier);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    return ensBaseadaptorFetchByIdentifier(ba, gid, (void **) Pgene);
}




/* @func ensGeneadaptorFetchByExonstableidentifier ****************************
**
** Fetch an Ensembl Gene via an Ensembl Exon stable identifier.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_exon_stable_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] stableid [const AjPStr] Ensembl Exon stable identifier
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByExonstableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    EnsPGene *Pgene)
{
    char *txtstableid = NULL;

    ajuint gid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ga)
        return ajFalse;

    if (!stableid && !ajStrGetLen(stableid))
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.gene_id "
        "FROM "
        "transcript, "
        "exon_transcript, "
        "exon "
        "WHERE "
        "transcript.transcript_id = exon_transcript.transcript_id "
        "AND "
        "exon_transcript.exon_id = exon.exon_id "
        "AND "
        "exon.stable_id = '%s' "
        "AND "
        "exon.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!gid)
    {
        ajDebug("ensGeneadaptorFetchByExonstableidentifier "
                "could not get Gene identifier for Exon "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    return ensBaseadaptorFetchByIdentifier(ba, gid, (void **) Pgene);
}




/* @func ensGeneadaptorFetchByIdentifier **************************************
**
** Fetch an Ensembl Gene via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor::fetch_by_dbID
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByIdentifier(
    EnsPGeneadaptor ga,
    ajuint identifier,
    EnsPGene *Pgene)
{
    EnsPBaseadaptor ba = NULL;

    EnsPCache cache = NULL;

    if (!ga)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    cache = ensFeatureadaptorGetCache(ga->Adaptor);

    ensCacheFetch(cache, (void *) &identifier, (void **) Pgene);

    if (*Pgene)
        return ajTrue;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorFetchByIdentifier(ba, identifier, (void **) Pgene);

    ensCacheStore(cache, (void *) &identifier, (void **) Pgene);

    return ajTrue;
}




/* @func ensGeneadaptorFetchByStableidentifier ********************************
**
** Fetch an Ensembl Gene via its stable identifier and version.
** In case a particular version is not specified,
** the current Ensembl Gene will be returned.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_stable_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByStableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    ajuint version,
    EnsPGene *Pgene)
{
    char *txtstableid = NULL;

    AjPList genes = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGene gene = NULL;

    if (!ga)
        return ajFalse;

    if (!stableid)
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if (version)
        constraint = ajFmtStr(
            "gene.stable_id = '%s' "
            "AND "
            "gene.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "gene.stable_id = '%s' "
            "AND "
            "gene.is_current = 1",
            txtstableid);

    ajCharDel(&txtstableid);

    genes = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       genes);

    if (ajListGetLength(genes) > 1)
        ajDebug("ensGeneadaptorFetchByStableidentifier got more than one "
                "Gene for stable identifier '%S' and version %u.\n",
                stableid,
                version);

    ajListPop(genes, (void **) Pgene);

    while (ajListPop(genes, (void **) &gene))
        ensGeneDel(&gene);

    ajListFree(&genes);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGeneadaptorFetchByTranscriptidentifier ****************************
**
** Fetch an Ensembl Gene via an Ensembl Transcript identifier.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_transcript_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] identifier [ajuint] Ensembl Transcript identifier
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByTranscriptidentifier(
    EnsPGeneadaptor ga,
    ajuint identifier,
    EnsPGene *Pgene)
{
    ajuint gid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ga)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor);

    statement = ajFmtStr(
        "SELECT "
        "transcript.gene_id "
        "FROM "
        "transcript "
        "WHERE "
        "transcript.transcript_id = %u",
        identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!gid)
    {
        ajDebug("ensGeneadaptorFetchByTranscriptidentifier "
                "could not get Gene identifier for Transcript "
                "identifier %u.\n", identifier);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    return ensBaseadaptorFetchByIdentifier(ba, gid, (void **) Pgene);
}




/* @func ensGeneadaptorFetchByTranscriptstableidentifier **********************
**
** Fetch an Ensembl Gene via an Ensembl Transcript stable identifier.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_transcript_stable_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] stableid [const AjPStr] Ensembl Transcript stable identifier
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByTranscriptstableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    EnsPGene *Pgene)
{
    char *txtstableid = NULL;

    ajuint gid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ga)
        return ajFalse;

    if (!stableid && !ajStrGetLen(stableid))
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.gene_id "
        "FROM"
        "transcript "
        "WHERE "
        "transcript.stable_id = '%s' "
        "AND "
        "transcript.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!gid)
    {
        ajDebug("ensGeneadaptorFetchByTranscriptstableidentifier "
                "could not get Gene identifier for Transcript "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    return ensBaseadaptorFetchByIdentifier(ba, gid, (void **) Pgene);
}




/* @func ensGeneadaptorFetchByTranslationstableidentifier *********************
**
** Fetch an Ensembl Gene via an Ensembl Translation stable identifier.
** The caller is responsible for deleting the Ensembl Gene.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_by_translation_stable_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] stableid [const AjPStr] Ensembl Translation stable identifier
** @param [wP] Pgene [EnsPGene*] Ensembl Gene address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByTranslationstableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    EnsPGene *Pgene)
{
    char *txtstableid = NULL;

    ajuint gid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ga)
        return ajFalse;

    if (!stableid && !ajStrGetLen(stableid))
        return ajFalse;

    if (!Pgene)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    statement = ajFmtStr(
        "SELECT "
        "transcript.gene_id "
        "FROM "
        "transcript, "
        "translation"
        "WHERE "
        "translation.stable_id = '%s' "
        "AND "
        "transcript.transcript_id = translation.transcript_id "
        "AND "
        "transcript.is_current = 1",
        txtstableid);

    ajCharDel(&txtstableid);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        gid = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &gid);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    if (!gid)
    {
        ajDebug("ensGeneadaptorFetchByTranslationstableidentifier "
                "could not get Gene identifier for Translation "
                "stable identifier '%S'.\n", stableid);

        return ajFalse;
    }

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    return ensBaseadaptorFetchByIdentifier(ba, gid, (void **) Pgene);
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Gene objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Gene-releated object(s)
** @nam4rule All Retrieve all Ensembl Gene-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
** @nam5rule Stableidentifiers Fetch all stable Ensembl Gene identifiers
**
** @argrule * ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
** @argrule AllStableidentifiers identifiers [AjPList] AJAX List of AJAX String
**                                              stable Ensembl Exon identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGeneadaptorRetrieveAllIdentifiers *********************************
**
** Retrieve all SQL database-internal identifiers of Ensembl Genes.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::list_dbIDs
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorRetrieveAllIdentifiers(
    EnsPGeneadaptor ga,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ga)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    table = ajStrNewC("gene");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}




/* @func ensGeneadaptorRetrieveAllStableidentifiers ***************************
**
** Retrieve all stable identifiers of Ensembl Genes.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::list_stable_ids
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorRetrieveAllStableidentifiers(
    EnsPGeneadaptor ga,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr primary = NULL;
    AjPStr table   = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!ga)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    table   = ajStrNewC("gene");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(ba,
                                              table,
                                              primary,
                                              identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}




/* @section check *************************************************************
**
** Check Ensembl Gene objects.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule Check Check Ensembl Gene object(s)
** @nam4rule Reference Check whether an Ensembl Gene object is the reference
**
** @argrule * ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @argrule * gene [const EnsPGene] Ensembl Gene
** @argrule Reference Presult [AjBool*] Result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGeneadaptorCheckReference *****************************************
**
** Check, whether an Ensembl Gene is a reference or alternative allele.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::is_ref
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [u] Presult [AjBool*] Result
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGeneadaptorCheckReference(
    EnsPGeneadaptor ga,
    const EnsPGene gene,
    AjBool *Presult)
{
    ajuint count = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!ga)
        return ajFalse;

    if (!gene)
        return ajFalse;

    if (!Presult)
        return ajFalse;

    *Presult = ajTrue;

    dba = ensGeneadaptorGetDatabaseadaptor(ga);

    statement = ajFmtStr(
        "SELECT "
        "COUNT(1) "
        "FROM "
        "alt_allele "
        "WHERE "
        "alt_allele.gene_id = %u "
        "AND "
        "alt_allele.is_ref = 0",
        ensGeneGetIdentifier(gene));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        count = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &count);

        if (count == 1)
            *Presult = ajFalse;
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

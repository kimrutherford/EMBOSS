/* @source Ensembl Gene functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.53 $
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

#include "ensattribute.h"
#include "ensdatabaseentry.h"
#include "ensexon.h"
#include "ensexternaldatabase.h"
#include "ensgene.h"
#include "ensprojectionsegment.h"
#include "enstable.h"
#include "enstranscript.h"




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

/* @conststatic geneStatus ****************************************************
**
** The Ensembl Gene status element is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to EnsEGeneStatus.
**
******************************************************************************/

static const char* const geneStatus[] =
{
    "",
    "KNOWN",
    "NOVEL",
    "PUTATIVE",
    "PREDICTED",
    "KNOWN_BY_PROJECTION",
    "UNKNOWN",
    (const char*) NULL
};




/* @conststatic geneadaptorTables *********************************************
**
** Array of Ensembl Gene Adaptor SQL table names
**
******************************************************************************/

static const char* const geneadaptorTables[] =
{
    "gene",
    "gene_stable_id",
    "xref",
    (const char*) NULL
};




/* @conststatic geneadaptorColumns ********************************************
**
** Array of Ensembl Gene Adaptor SQL column names
**
******************************************************************************/

static const char* const geneadaptorColumns[] =
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
    "gene_stable_id.stable_id",
    "gene_stable_id.version",
    "gene_stable_id.created_date",
    "gene_stable_id.modified_date",
    "xref.external_db_id",
    "xref.dbprimary_acc",
    "xref.display_label",
    "xref.version",
    "xref.description",
    "xref.info_type",
    "xref.info_text",
    (const char*) NULL
};




/* @conststatic geneadaptorLeftjoin *******************************************
**
** Array of Ensembl Gene Adaptor SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin geneadaptorLeftjoin[] =
{
    {"gene_stable_id", "gene.gene_id = gene_stable_id.gene_id"},
    {"xref", "gene.display_xref_id = xref.xref_id"},
    {(const char*) NULL, (const char*) NULL}
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static EnsPGene geneNewCpyFeatures(EnsPGene gene);

static int geneCompareExon(const void* P1, const void* P2);

static void geneDeleteExon(void** PP1, void* cl);

static int listGeneCompareStartAscending(const void* P1, const void* P2);

static int listGeneCompareStartDescending(const void* P1, const void* P2);

static void tableGeneClear(void** key,
                           void** value,
                           void* cl);

static AjBool geneadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList genes);

static void* geneadaptorCacheReference(void* value);

static void geneadaptorCacheDelete(void** value);

static size_t geneadaptorCacheSize(const void* value);

static EnsPFeature geneadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.166
** @cc CVS Tag: branch-ensembl-62
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
** @argrule Ini current [AjBool] Current
** @argrule Ini cantrcid [ajuint] Canonical Ensembl Transcript identfier
** @argrule Ini canann [AjPStr] Canonical annotation
** @argrule Ini stableid [AjPStr] Stable identifier
** @argrule Ini version [ajuint] Version
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini mdate [AjPStr] Modification date
** @argrule Ini transcripts [AjPList] AJAX List of Ensembl Transcript objects
** @argrule Ref gene [EnsPGene] Ensembl Gene
**
** @valrule * [EnsPGene] Ensembl Gene
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
** @@
******************************************************************************/

EnsPGene ensGeneNewCpy(const EnsPGene gene)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPGene pthis = NULL;

    EnsPTranscript transcript = NULL;

    if(!gene)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gene->Identifier;
    pthis->Adaptor    = gene->Adaptor;
    pthis->Feature    = ensFeatureNewRef(gene->Feature);

    pthis->Displayreference = ensDatabaseentryNewRef(gene->Displayreference);

    if(gene->Description)
        pthis->Description = ajStrNewRef(gene->Description);

    if(gene->Source)
        pthis->Source = ajStrNewRef(gene->Source);

    if(gene->Biotype)
        pthis->Biotype = ajStrNewRef(gene->Biotype);

    pthis->Status  = gene->Status;
    pthis->Current = gene->Current;

    if(gene->Canonicalannotation)
        pthis->Canonicalannotation = ajStrNewRef(gene->Canonicalannotation);

    pthis->Canonicaltranscriptidentifier =
        gene->Canonicaltranscriptidentifier;

    pthis->Version = gene->Version;

    if(gene->Stableidentifier)
        pthis->Stableidentifier = ajStrNewRef(gene->Stableidentifier);

    if(gene->DateCreation)
        pthis->DateCreation = ajStrNewRef(gene->DateCreation);

    if(gene->DateModification)
        pthis->DateModification = ajStrNewRef(gene->DateModification);

    /* NOTE: Copy the AJAX List of Ensembl Attribute objects. */

    if(gene->Attributes && ajListGetLength(gene->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(gene->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void*) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Database Entry objects. */

    if(gene->Databaseentries && ajListGetLength(gene->Databaseentries))
    {
        pthis->Databaseentries = ajListNew();

        iter = ajListIterNew(gene->Databaseentries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ajListPushAppend(pthis->Databaseentries,
                             (void*) ensDatabaseentryNewRef(dbe));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Databaseentries = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Transcript objects. */

    if(gene->Transcripts && ajListGetLength(gene->Transcripts))
    {
        pthis->Transcripts = ajListNew();

        iter = ajListIterNew(gene->Transcripts);

        while(!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            ajListPushAppend(pthis->Transcripts,
                             (void*) ensTranscriptNewRef(transcript));
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
** @param [r] current [AjBool] Current
** @param [r] cantrcid [ajuint] Canonical Ensembl Transcript identfier
** @param [u] canann [AjPStr] Canonical annotation
** @param [u] stableid [AjPStr] Stable identifier
** @param [r] version [ajuint] Version
** @param [u] cdate [AjPStr] Creation date
** @param [u] mdate [AjPStr] Modification date
** @param [u] transcripts [AjPList] AJAX List of Ensembl Transcript objects
**
** @return [EnsPGene] Ensembl Gene or NULL
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

    if(ajDebugTest("ensGeneNew"))
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

    if(!feature)
        return NULL;

    AJNEW0(gene);

    gene->Use        = 1;
    gene->Identifier = identifier;
    gene->Adaptor    = ga;
    gene->Feature    = ensFeatureNewRef(feature);

    gene->Displayreference = ensDatabaseentryNewRef(displaydbe);

    if(description)
        gene->Description = ajStrNewRef(description);

    if(source)
        gene->Source = ajStrNewRef(source);

    if(biotype)
        gene->Biotype = ajStrNewRef(biotype);

    gene->Status  = status;
    gene->Current = current;

    if(canann)
        gene->Canonicalannotation = ajStrNewRef(canann);

    gene->Canonicaltranscriptidentifier = cantrcid;
    gene->Version = version;

    if(stableid)
        gene->Stableidentifier = ajStrNewRef(stableid);

    if(cdate)
        gene->DateCreation = ajStrNewRef(cdate);

    if(mdate)
        gene->DateModification = ajStrNewRef(mdate);

    gene->Attributes = NULL;

    gene->Databaseentries = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Transcript objects. */

    if(transcripts && ajListGetLength(transcripts))
    {
        gene->Transcripts = ajListNew();

        iter = ajListIterNew(transcripts);

        while(!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            ajListPushAppend(gene->Transcripts,
                             (void*) ensTranscriptNewRef(transcript));
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
** @@
******************************************************************************/

EnsPGene ensGeneNewRef(EnsPGene gene)
{
    if(!gene)
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
** @@
******************************************************************************/

static EnsPGene geneNewCpyFeatures(EnsPGene gene)
{
    AjIList iter = NULL;

    EnsPGene newgene = NULL;

    EnsPTranscript newtranscript = NULL;
    EnsPTranscript oldtranscript = NULL;

    if(!gene)
        return NULL;

    newgene = ensGeneNewCpy(gene);

    if(!newgene)
        return NULL;

    /* Copy the AJAX List of Ensembl Transcript objects. */

    if(newgene->Transcripts)
    {
        iter = ajListIterNew(newgene->Transcripts);

        while(!ajListIterDone(iter))
        {
            oldtranscript = (EnsPTranscript) ajListIterGet(iter);

            ajListIterRemove(iter);

            newtranscript = ensTranscriptNewCpy(oldtranscript);

            ajListIterInsert(iter, (void*) newtranscript);

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
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Del Destroy (free) an Ensembl Gene object
**
** @argrule * Pgene [EnsPGene*] Ensembl Gene object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGeneDel ***********************************************************
**
** Default destructor for an Ensembl Gene.
**
** @param [d] Pgene [EnsPGene*] Ensembl Gene object address
**
** @return [void]
** @@
******************************************************************************/

void ensGeneDel(EnsPGene* Pgene)
{
    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPTranscript transcript = NULL;

    EnsPGene pthis = NULL;

    if(!Pgene)
        return;

    if(!*Pgene)
        return;

    if(ajDebugTest("ensGeneDel"))
    {
        ajDebug("ensGeneDel\n"
                "  *Pgene %p\n",
                *Pgene);

        ensGeneTrace(*Pgene, 1);
    }

    pthis = *Pgene;

    pthis->Use--;

    if((*Pgene)->Use)
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

    while(ajListPop(pthis->Attributes, (void**) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Database Entry objects. */

    while(ajListPop(pthis->Databaseentries, (void**) &dbe))
        ensDatabaseentryDel(&dbe);

    ajListFree(&pthis->Databaseentries);

    /* Clear and delete the AJAX List of Ensembl Transcript objects. */

    while(ajListPop(pthis->Transcripts, (void**) &transcript))
        ensTranscriptDel(&transcript);

    ajListFree(&pthis->Transcripts);

    AJFREE(pthis);

    *Pgene = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Get Return Gene attribute(s)
** @nam4rule Adaptor Return the Ensembl Gene Adaptor
** @nam4rule Biotype Return the biological type
** @nam4rule Canonicalannotation Return the canonical annotation
** @nam4rule Canonicaltranscriptidentifier Return the canonical
**                                         Ensembl Transcript identifier
** @nam4rule Current Return the current element
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
** @valrule Adaptor [EnsPGeneadaptor] Ensembl Gene Adaptor
** @valrule Biotype [AjPStr] Biological type
** @valrule Canonicaltranscriptidentifier [ajuint] Canonical Ensembl
**                                                 Transcript identifier
** @valrule Canonicalannotation [AjPStr] Canonical annotation
** @valrule Current [AjBool] Current element
** @valrule DateCreation [AjPStr] Creation date
** @valrule DateModification [AjPStr] Modification date
** @valrule Description [AjPStr] Description
** @valrule Displayreference [EnsPDatabaseentry] Ensembl Database Entry
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Source [AjPStr] Source
** @valrule Stableidentifier [AjPStr] Stable identifier
** @valrule Status [EnsEGeneStatus] Status
** @valrule Version [ajuint] Version
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects
** @valrule Databaseentries [const AjPList]
** AJAX List of Ensembl Database Entry objects
** @valrule Transcripts [const AjPList]
** AJAX List of Ensembl Transcript objects
**
** @fcategory use
******************************************************************************/




/* @func ensGeneGetAdaptor ****************************************************
**
** Get the Ensembl Gene Adaptor element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPGeneadaptor] Ensembl Gene Adaptor or NULL
** @@
******************************************************************************/

EnsPGeneadaptor ensGeneGetAdaptor(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Adaptor;
}




/* @func ensGeneGetBiotype ****************************************************
**
** Get the biological type element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::biotype
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Biological type or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetBiotype(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Biotype;
}




/* @func ensGeneGetCanonicalannotation ****************************************
**
** Get the canonical annotation element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_annotation
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Canonical annotation or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetCanonicalannotation(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Canonicalannotation;
}




/* @func ensGeneGetCanonicaltranscriptidentifier ******************************
**
** Get the canonical Ensembl Transcript identifier element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_transcript
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [ajuint] Canonical Ensembl Transcript identifier or 0
** @@
******************************************************************************/

ajuint ensGeneGetCanonicaltranscriptidentifier(const EnsPGene gene)
{
    if(!gene)
        return 0;

    return gene->Canonicaltranscriptidentifier;
}




/* @func ensGeneGetCurrent ****************************************************
**
** Get the current element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::is_current
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjBool] ajTrue if this Gene reflects the current state of
**                  annotation
** @@
******************************************************************************/

AjBool ensGeneGetCurrent(const EnsPGene gene)
{
    if(!gene)
        return ajFalse;

    return gene->Current;
}




/* @func ensGeneGetDateCreation ***********************************************
**
** Get the creation date element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::created_date
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Creation date or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetDateCreation(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->DateCreation;
}




/* @func ensGeneGetDateModification *******************************************
**
** Get the modification date element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::modified_date
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Modification date or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetDateModification(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->DateModification;
}




/* @func ensGeneGetDescription ************************************************
**
** Get the description element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::description
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetDescription(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Description;
}




/* @func ensGeneGetDisplayreference *******************************************
**
** Get the display reference element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::display_xref
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPDatabaseentry] Ensembl Database Entry or NULL
** @@
******************************************************************************/

EnsPDatabaseentry ensGeneGetDisplayreference(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Displayreference;
}




/* @func ensGeneGetFeature ****************************************************
**
** Get the Ensembl Feature element of an Ensembl Gene.
**
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensGeneGetFeature(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Feature;
}




/* @func ensGeneGetIdentifier *************************************************
**
** Get the SQL database-internal identifier element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGeneGetIdentifier(const EnsPGene gene)
{
    if(!gene)
        return 0;

    return gene->Identifier;
}




/* @func ensGeneGetSource *****************************************************
**
** Get the source element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::source
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Source or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetSource(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Source;
}




/* @func ensGeneGetStableidentifier *******************************************
**
** Get the stable identifier element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::stable_id
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [AjPStr] Stable identifier or NULL
** @@
******************************************************************************/

AjPStr ensGeneGetStableidentifier(const EnsPGene gene)
{
    if(!gene)
        return NULL;

    return gene->Stableidentifier;
}




/* @func ensGeneGetStatus *****************************************************
**
** Get the status element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::status
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [EnsEGeneStatus] Status or ensEGeneStatusNULL
** @@
******************************************************************************/

EnsEGeneStatus ensGeneGetStatus(const EnsPGene gene)
{
    if(!gene)
        return ensEGeneStatusNULL;

    return gene->Status;
}




/* @func ensGeneGetVersion ****************************************************
**
** Get the version element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::version
** @param [r] gene [const EnsPGene] Ensembl Gene
**
** @return [ajuint] Version or 0
** @@
******************************************************************************/

ajuint ensGeneGetVersion(const EnsPGene gene)
{
    if(!gene)
        return 0;

    return gene->Version;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Gene object,
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
** @@
******************************************************************************/

const AjPList ensGeneLoadAttributes(EnsPGene gene)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gene)
        return NULL;

    if(gene->Attributes)
        return gene->Attributes;

    if(!gene->Adaptor)
    {
        ajDebug("ensGeneLoadAttributes cannot fetch "
                "Ensembl Attribute objects for an Ensembl Gene without an "
                "Ensembl Gene Adaptor.\n");

        return NULL;
    }

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    if(!dba)
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
** @@
******************************************************************************/

const AjPList ensGeneLoadDatabaseentries(EnsPGene gene)
{
    AjPStr objtype = NULL;

    EnsPDatabaseadaptor dba = NULL;
    EnsPDatabaseentryadaptor dbea = NULL;

    if(!gene)
        return NULL;

    if(gene->Databaseentries)
        return gene->Databaseentries;

    if(!gene->Adaptor)
    {
        ajDebug("ensGeneLoadDatabaseentries cannot fetch "
                "Ensembl Database Entry objects for an Ensembl Gene without "
                "an Ensembl Gene Adaptor.\n");

        return NULL;
    }

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    if(!dba)
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
** @@
******************************************************************************/

const AjPList ensGeneLoadTranscripts(EnsPGene gene)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscriptadaptor tca = NULL;

    if(!gene)
        return NULL;

    if(gene->Transcripts)
        return gene->Transcripts;

    if(!gene->Adaptor)
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




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Set Set one element of a Gene
** @nam4rule Adaptor Set the Ensembl Gene Adaptor
** @nam4rule Biotype Set the biological type
** @nam4rule Canonicalannotation Set the canonical annotation
** @nam4rule Canonicaltranscriptidentifier Set the canonical
**                                         Ensembl Transcript identifier
** @nam4rule Current Set the current element
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
**                                     Canonical Ensembl Transcript identifier
** @argrule Current current [AjBool] Current
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
** Set the Ensembl Gene Adaptor element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetAdaptor(EnsPGene gene, EnsPGeneadaptor ga)
{
    if(!gene)
        return ajFalse;

    gene->Adaptor = ga;

    return ajTrue;
}




/* @func ensGeneSetBiotype ****************************************************
**
** Set the biological type element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::biotype
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] biotype [AjPStr] Biological type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetBiotype(EnsPGene gene, AjPStr biotype)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->Biotype);

    gene->Biotype = ajStrNewRef(biotype);

    return ajTrue;
}




/* @func ensGeneSetCanonicalannotation ****************************************
**
** Set the canonical annotation element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_annotation
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] canann [AjPStr] Canonical annotation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetCanonicalannotation(EnsPGene gene, AjPStr canann)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->Canonicalannotation);

    gene->Canonicalannotation = ajStrNewRef(canann);

    return ajTrue;
}




/* @func ensGeneSetCanonicaltranscriptidentifier ******************************
**
** Set the canonical Ensembl Transcript identifier element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::canonical_transcript
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] cantrcid [ajuint] Canonical Ensembl Transcript identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: In this implementation, the Ensembl Gene only holds the identifier
** for the canonical Ensembl Transcript and not the object itself.
******************************************************************************/

AjBool ensGeneSetCanonicaltranscriptidentifier(EnsPGene gene, ajuint cantrcid)
{
    AjBool match = AJFALSE;

    AjIList iter = NULL;

    const AjPList transcripts = NULL;

    EnsPTranscript transcript = NULL;

    if(!gene)
        return ajFalse;

    if(cantrcid == 0)
    {
        gene->Canonicaltranscriptidentifier = cantrcid;

        return ajTrue;
    }

    transcripts = ensGeneLoadTranscripts(gene);

    iter = ajListIterNewread(transcripts);

    while(!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        if(ensTranscriptGetIdentifier(transcript) == cantrcid)
        {
            match = ajTrue;
            break;
        }
    }

    ajListIterDel(&iter);

    if(match == ajTrue)
        gene->Canonicaltranscriptidentifier = cantrcid;

    return match;
}




/* @func ensGeneSetCurrent ****************************************************
**
** Set the current element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::is_current
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] current [AjBool] Current
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetCurrent(EnsPGene gene, AjBool current)
{
    if(!gene)
        return ajFalse;

    gene->Current = current;

    return ajTrue;
}




/* @func ensGeneSetDateCreation ***********************************************
**
** Set the creation date element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::created_date
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetDateCreation(EnsPGene gene, AjPStr cdate)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->DateCreation);

    gene->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensGeneSetDateModification *******************************************
**
** Set the modification date element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::modified_date
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] mdate [AjPStr] Modification date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetDateModification(EnsPGene gene, AjPStr mdate)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->DateModification);

    gene->DateModification = ajStrNewRef(mdate);

    return ajTrue;
}




/* @func ensGeneSetDescription ************************************************
**
** Set the description element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::description
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetDescription(EnsPGene gene, AjPStr description)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->Description);

    gene->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGeneSetDisplayreference *******************************************
**
** Set the Ensembl Database Entry element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::display_xref
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] dbe [EnsPDatabaseentry] Ensembl Database Entry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetDisplayreference(EnsPGene gene, EnsPDatabaseentry dbe)
{
    if(!gene)
        return ajFalse;

    ensDatabaseentryDel(&gene->Displayreference);

    gene->Displayreference = ensDatabaseentryNewRef(dbe);

    return ajTrue;
}




/* @func ensGeneSetFeature ****************************************************
**
** Set the Ensembl Feature element of an Ensembl Gene.
**
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetFeature(EnsPGene gene, EnsPFeature feature)
{
    AjIList iter = NULL;

    EnsPSlice slice = NULL;

    EnsPTranscript oldtranscript = NULL;
    EnsPTranscript newtranscript = NULL;

    if(ajDebugTest("ensGeneSetFeature"))
        ajDebug("ensGeneSetFeature\n"
                "  gene %p\n"
                "  feature %p\n",
                gene,
                feature);

    if(!gene)
        return ajFalse;

    if(!feature)
        return ajFalse;

    /* Replace the current Feature. */

    if(gene->Feature)
        ensFeatureDel(&gene->Feature);

    gene->Feature = ensFeatureNewRef(feature);

    slice = ensFeatureGetSlice(gene->Feature);

    /* Transfer the Ensembl Transcript objects onto the new Ensembl Slice. */

    iter = ajListIterNew(gene->Transcripts);

    while(!ajListIterDone(iter))
    {
        oldtranscript = (EnsPTranscript) ajListIterGet(iter);

        ajListIterRemove(iter);

        newtranscript = ensTranscriptTransfer(oldtranscript, slice);

        if(!newtranscript)
        {
            ajDebug("ensGeneSetFeature could not transfer "
                    "Transcript onto new Ensembl Feature Slice.");

            ensTranscriptTrace(oldtranscript, 1);
        }

        ajListIterInsert(iter, (void*) newtranscript);

        /* Advance the AJAX List Iterator after the insert. */

        (void) ajListIterGet(iter);

        ensTranscriptDel(&oldtranscript);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensGeneSetIdentifier *************************************************
**
** Set the SQL database-internal identifier element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetIdentifier(EnsPGene gene, ajuint identifier)
{
    if(!gene)
        return ajFalse;

    gene->Identifier = identifier;

    return ajTrue;
}




/* @func ensGeneSetSource *****************************************************
**
** Set the source element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::source
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] source [AjPStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetSource(EnsPGene gene, AjPStr source)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->Source);

    gene->Source = ajStrNewRef(source);

    return ajTrue;
}




/* @func ensGeneSetStableidentifier *******************************************
**
** Set the stable identifier element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::stable_id
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] stableid [AjPStr] Stable identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetStableidentifier(EnsPGene gene, AjPStr stableid)
{
    if(!gene)
        return ajFalse;

    ajStrDel(&gene->Stableidentifier);

    gene->Stableidentifier = ajStrNewRef(stableid);

    return ajTrue;
}




/* @func ensGeneSetStatus *****************************************************
**
** Set the status element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::status
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [u] status [EnsEGeneStatus] Status
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetStatus(EnsPGene gene, EnsEGeneStatus status)
{
    if(!gene)
        return ajFalse;

    gene->Status = status;

    return ajTrue;
}




/* @func ensGeneSetVersion ****************************************************
**
** Set the version element of an Ensembl Gene.
**
** @cc Bio::EnsEMBL::Gene::version
** @param [u] gene [EnsPGene] Ensembl Gene
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneSetVersion(EnsPGene gene, ajuint version)
{
    if(!gene)
        return ajFalse;

    gene->Version = version;

    return ajTrue;
}




/* @section element addition **************************************************
**
** Functions for adding elements to an Ensembl Gene object.
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
** @@
******************************************************************************/

AjBool ensGeneAddAttribute(EnsPGene gene, EnsPAttribute attribute)
{
    if(!gene)
        return ajFalse;

    if(!attribute)
        return ajFalse;

    if(!gene->Attributes)
        gene->Attributes = ajListNew();

    ajListPushAppend(gene->Attributes, (void*) ensAttributeNewRef(attribute));

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
** @@
******************************************************************************/

AjBool ensGeneAddDatabaseentry(EnsPGene gene, EnsPDatabaseentry dbe)
{
    if(!gene)
        return ajFalse;

    if(!dbe)
        return ajFalse;

    if(!gene->Databaseentries)
        gene->Databaseentries = ajListNew();

    ajListPushAppend(gene->Databaseentries,
                     (void*) ensDatabaseentryNewRef(dbe));

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
** @@
******************************************************************************/

AjBool ensGeneAddTranscript(EnsPGene gene, EnsPTranscript transcript)
{
    if(!gene)
        return ajFalse;

    if(!transcript)
        return ajFalse;

    if(!gene->Transcripts)
        gene->Transcripts = ajListNew();

    ajListPushAppend(gene->Transcripts,
                     (void*) ensTranscriptNewRef(transcript));

    ensGeneCalculateCoordinates(gene);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Gene object.
**
** @fdata [EnsPGene]
**
** @nam3rule Trace Report Ensembl Gene elements to debug file
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
** @@
******************************************************************************/

AjBool ensGeneTrace(const EnsPGene gene, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPTranscript transcript = NULL;

    if(!gene)
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

    if(gene->Attributes)
    {
        ajDebug("%S    AJAX List %p of Ensembl Attribute objects\n",
                indent, gene->Attributes);

        iter = ajListIterNewread(gene->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ensAttributeTrace(attribute, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Database Entry objects. */

    if(gene->Databaseentries)
    {
        ajDebug("%S    AJAX List %p of Ensembl Database Entry objects\n",
                indent, gene->Databaseentries);

        iter = ajListIterNewread(gene->Databaseentries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            ensDatabaseentryTrace(dbe, level + 2);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of Ensembl Transcript objects. */

    if(gene->Transcripts)
    {
        ajDebug("%S    AJAX List %p of Ensembl Transcript objects\n",
                indent, gene->Transcripts);

        iter = ajListIterNewread(gene->Transcripts);

        while(!ajListIterDone(iter))
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

    if(!gene)
        return ajFalse;

    list = ensGeneLoadTranscripts(gene);

    if(!ajListGetLength(list))
        return ajTrue;

    /* Get the first Transcript with coordinates. */

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        /* Skip missing Ensembl Transcript objects. */

        if(!transcript)
            continue;

        tfeature = ensTranscriptGetFeature(transcript);

        /* Skip un-mapped Ensembl Transcript objects. */

        if(!ensFeatureGetStart(tfeature))
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

    while(!ajListIterDone(iter))
    {
        transcript = (EnsPTranscript) ajListIterGet(iter);

        /* Skip missing Ensembl Transcript objects. */

        if(!transcript)
            continue;

        tfeature = ensTranscriptGetFeature(transcript);

        /* Skip un-mapped Ensembl Transcript objects. */

        if(!ensFeatureGetStart(tfeature))
            continue;

        if(!ensSliceMatch(ensFeatureGetSlice(tfeature), slice))
            ajFatal("ensGeneCalculateCoordinates got "
                    "Ensembl Transcript objects of one Ensembl Gene on "
                    "different Ensembl Slice objects.\n");

        start = (ensFeatureGetStart(tfeature) < start) ?
            ensFeatureGetStart(tfeature) :
            start;

        end = (ensFeatureGetEnd(tfeature) > end) ?
            ensFeatureGetEnd(tfeature) :
            end;

        if(ensFeatureGetStrand(tfeature) != strand)
            transsplicing = ajTrue;
    }

    ajListIterDel(&iter);

    if(transsplicing)
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
** @@
******************************************************************************/

size_t ensGeneCalculateMemsize(const EnsPGene gene)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPTranscript transcript = NULL;

    if(!gene)
        return 0;

    size += sizeof (EnsOGene);

    size += ensFeatureCalculateMemsize(gene->Feature);

    size += ensDatabaseentryCalculateMemsize(gene->Displayreference);

    if(gene->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Description);
    }

    if(gene->Source)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Source);
    }

    if(gene->Biotype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Biotype);
    }

    if(gene->Canonicalannotation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Canonicalannotation);
    }

    if(gene->Stableidentifier)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->Stableidentifier);
    }

    if(gene->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->DateCreation);
    }

    if(gene->DateModification)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gene->DateModification);
    }

    /* Summarise the AJAX List of Ensembl Attribute objects. */

    if(gene->Attributes)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(gene->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Database Entry objects. */

    if(gene->Databaseentries)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(gene->Databaseentries);

        while(!ajListIterDone(iter))
        {
            dbe = (EnsPDatabaseentry) ajListIterGet(iter);

            size += ensDatabaseentryCalculateMemsize(dbe);
        }

        ajListIterDel(&iter);
    }

    /* Summarise the AJAX List of Ensembl Transcript objects. */

    if(gene->Transcripts)
    {
        size += sizeof (AjOList);

        iter = ajListIterNewread(gene->Transcripts);

        while(!ajListIterDone(iter))
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

    if(!gene)
        return ajFalse;

    if(!attributes)
        return ajFalse;

    list = ensGeneLoadAttributes(gene);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if(code)
        {
            if(ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if(match)
            ajListPushAppend(attributes,
                             (void*) ensAttributeNewRef(attribute));
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

    if(!gene)
        return ajFalse;

    if(!dbes)
        return ajFalse;

    list = ensGeneLoadDatabaseentries(gene);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        dbe = (EnsPDatabaseentry) ajListIterGet(iter);

        if(name)
        {
            if(ajStrMatchCaseS(name, ensDatabaseentryGetDbName(dbe)))
                namematch = ajTrue;
            else
                namematch = ajFalse;
        }
        else
            namematch = ajTrue;

        if(type)
        {
            if(type == ensDatabaseentryGetType(dbe))
                typematch = ajTrue;
            else
                typematch = ajFalse;
        }
        else
            typematch = ajTrue;

        if(namematch && typematch)
            ajListPushAppend(dbes, (void*) ensDatabaseentryNewRef(dbe));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic geneCompareExon ************************************************
**
** Comparison function to sort Ensembl Exon objects via their address in
** ascending order. For identical Exon objects at separate addresses, which are
** matched via ensExonMatch 0 is returned.
**
** @param [r] P1 [const void*] Ensembl Exon 1
** @param [r] P2 [const void*] Ensembl Exon 2
** @see ajListSortUnique
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int geneCompareExon(const void* P1, const void* P2)
{
    int result = 0;

    const EnsPExon exon1 = NULL;
    const EnsPExon exon2 = NULL;

    exon1 = *(EnsPExon const*) P1;
    exon2 = *(EnsPExon const*) P2;

    if(ajDebugTest("geneCompareExon"))
        ajDebug("geneCompareExon\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);

    if(ensExonSimilarity(exon1, exon2) == ajTrue)
        return 0;

    if(exon1 < exon2)
        result = -1;

    if(exon1 > exon2)
        result = +1;

    return result;
}




/* @funcstatic geneDeleteExon *************************************************
**
** ajListSortUnique nodedelete function to delete Ensembl Exon objects that
** are redundant.
**
** @param [r] PP1 [void**] Ensembl Exon address 1
** @param [r] cl [void*] Standard, passed in from ajListSortUnique
** @see ajListSortUnique
**
** @return [void]
** @@
******************************************************************************/

static void geneDeleteExon(void** PP1, void* cl)
{
    if(!PP1)
        return;

    (void) cl;

    ensExonDel((EnsPExon*) PP1);

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

    if(!gene)
        return ajFalse;

    if(!exons)
        return ajFalse;

    tlist = ensGeneLoadTranscripts(gene);

    titer = ajListIterNewread(tlist);

    while(!ajListIterDone(titer))
    {
        transcript = (EnsPTranscript) ajListIterGet(titer);

        elist = ensTranscriptLoadExons(transcript);

        eiter = ajListIterNewread(elist);

        while(!ajListIterDone(eiter))
        {
            exon = (EnsPExon) ajListIterGet(eiter);

            ajListPushAppend(exons, (void*) ensExonNewRef(exon));
        }

        ajListIterDel(&eiter);
    }

    ajListIterDel(&titer);

    ajListSortUnique(exons, geneCompareExon, geneDeleteExon);

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
** @@
******************************************************************************/

AjBool ensGeneFetchCanonicaltranscript(EnsPGene gene,
                                       EnsPTranscript* Ptranscript)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPTranscriptadaptor tca = NULL;

    if(!gene)
        return ajFalse;

    if(!Ptranscript)
        return ajFalse;

    dba = ensGeneadaptorGetDatabaseadaptor(gene->Adaptor);

    tca = ensRegistryGetTranscriptadaptor(dba);

    ensTranscriptadaptorFetchByIdentifier(
        tca,
        gene->Canonicaltranscriptidentifier,
        Ptranscript);

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

    if(!gene1)
        return ajFalse;

    if(!gene2)
        return ajFalse;

    if(gene1 == gene2)
        return ajTrue;

    if(!ensFeatureSimilarity(gene1->Feature, gene2->Feature))
        return ajFalse;

    if(!ajStrMatchCaseS(gene1->Biotype, gene2->Biotype))
        return ajFalse;

    if((gene1->Stableidentifier && gene2->Stableidentifier) &&
       (!ajStrMatchCaseS(gene1->Stableidentifier,
                         gene2->Stableidentifier)))
        return ajFalse;

    transcripts1 = ensGeneLoadTranscripts(gene1);
    transcripts2 = ensGeneLoadTranscripts(gene2);

    if(ajListGetLength(transcripts1) != ajListGetLength(transcripts2))
        return ajFalse;

    iter1 = ajListIterNewread(transcripts1);
    iter2 = ajListIterNewread(transcripts2);

    while(!ajListIterDone(iter1))
    {
        transcript1 = (EnsPTranscript) ajListIterGet(iter1);

        ajListIterRewind(iter2);

        similarity = ajFalse;

        while(!ajListIterDone(iter2))
        {
            transcript2 = (EnsPTranscript) ajListIterGet(iter2);

            if(ensTranscriptSimilarity(transcript1, transcript2))
            {
                similarity = ajTrue;
                break;
            }
        }

        if(similarity == ajFalse)
        {
            mismatch = ajTrue;
            break;
        }
    }

    ajListIterDel(&iter1);
    ajListIterDel(&iter2);

    if(mismatch == ajTrue)
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
** @@
******************************************************************************/

EnsPGene ensGeneTransfer(EnsPGene gene, EnsPSlice slice)
{
    EnsPFeature newfeature = NULL;

    EnsPGene newgene = NULL;

    if(ajDebugTest("ensGeneTransfer"))
        ajDebug("ensGeneTransfer\n"
                "  gene %p\n"
                "  slice %p\n",
                gene,
                slice);

    if(!gene)
        return NULL;

    if(!slice)
        return NULL;

    newfeature = ensFeatureTransfer(gene->Feature, slice);

    if(!newfeature)
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
** @@
******************************************************************************/

EnsPGene ensGeneTransform(EnsPGene gene,
                          const AjPStr csname,
                          const AjPStr csversion)
{
    ajint failures = 0;
    ajint minstart = INT_MAX;
    ajint maxend   = INT_MIN;

    ajuint psslength = 0;

    AjIList iter = NULL;
    AjPList pss  = NULL;

    EnsPFeature newgf = NULL;
    EnsPFeature newtf = NULL;

    EnsPGene newgene = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPTranscript newtranscript = NULL;
    EnsPTranscript oldtranscript = NULL;

    if(!gene)
        return NULL;

    if(!csname)
        return NULL;

    if(ajStrGetLen(csname) == 0)
        return NULL;

    newgf = ensFeatureTransform(gene->Feature,
                                csname,
                                csversion,
                                (EnsPSlice) NULL);

    if(!newgf)
    {
        /*
        ** Check if this Gene projects at all to the requested
        ** Coordinate System.
        */

        pss = ajListNew();

        ensFeatureProject(gene->Feature, csname, csversion, pss);

        psslength = ajListGetLength(pss);

        while(ajListPop(pss, (void**) &ps))
            ensProjectionsegmentDel(&ps);

        ajListFree(&pss);

        if(psslength == 0)
            return NULL;
        else
            ensGeneLoadTranscripts(gene);
    }

    /*
    ** Copy the Ensembl Gene object and also its internal
    ** Ensembl Feature class-based members.
    */

    newgene = geneNewCpyFeatures(gene);

    if(newgene->Transcripts)
    {
        iter = ajListIterNew(newgene->Transcripts);

        while(!ajListIterDone(iter))
        {
            oldtranscript = (EnsPTranscript) ajListIterGet(iter);

            ajListIterRemove(iter);

            newtranscript = ensTranscriptTransform(oldtranscript,
                                                   csname,
                                                   csversion);

            if(newtranscript)
            {
                ajListIterInsert(iter, (void*) newtranscript);

                /* Advance the AJAX List Iterator after the insert. */

                (void) ajListIterGet(iter);
            }
            else
            {
                failures++;

                continue;
            }

            if(!newgf)
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

    if(failures)
    {
        ensFeatureDel(&newgf);

        ensGeneDel(&newgene);

        return NULL;
    }

    if(!newgf)
    {
        ajListPeekFirst(newgene->Transcripts, (void**) &newtranscript);

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
** @@
******************************************************************************/

EnsEGeneStatus ensGeneStatusFromStr(const AjPStr status)
{
    register EnsEGeneStatus i = ensEGeneStatusNULL;

    EnsEGeneStatus estatus = ensEGeneStatusNULL;

    for(i = ensEGeneStatusNULL;
        geneStatus[i];
        i++)
        if(ajStrMatchC(status, geneStatus[i]))
            estatus = i;

    if(!estatus)
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
** @valrule Char [const char*] Ensembl Gene Status C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGeneStatusToChar **************************************************
**
** Convert an Ensembl Gene Status enumeration into a C-type (char*) string.
**
** @param [u] status [EnsEGeneStatus] Ensembl Gene Status enumeration
**
** @return [const char*] Ensembl Gene Status C-type (char*) string
** @@
******************************************************************************/

const char* ensGeneStatusToChar(EnsEGeneStatus status)
{
    register EnsEGeneStatus i = ensEGeneStatusNULL;

    for(i = ensEGeneStatusNULL;
        geneStatus[i] && (i < status);
        i++);

    if(!geneStatus[i])
        ajDebug("ensGeneStatusToChar encountered an "
                "out of boundary error on "
                "Ensembl Gene Status enumeration %d.\n",
                status);

    return geneStatus[i];
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
** @nam3rule Gene Functions for manipulating AJAX List objects of
** Ensembl Gene objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending genes [AjPList] AJAX List of Ensembl Gene objects
** @argrule Descending genes [AjPList] AJAX List of Ensembl Gene objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listGeneCompareStartAscending **********************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** Ensembl Feature start element in ascending order.
**
** @param [r] P1 [const void*] Ensembl Gene address 1
** @param [r] P2 [const void*] Ensembl Gene address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listGeneCompareStartAscending(const void* P1, const void* P2)
{
    const EnsPGene gene1 = NULL;
    const EnsPGene gene2 = NULL;

    gene1 = *(EnsPGene const*) P1;
    gene2 = *(EnsPGene const*) P2;

    if(ajDebugTest("listGeneCompareStartAscending"))
        ajDebug("listGeneCompareStartAscending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);

    /* Sort empty values towards the end of the AJAX List. */

    if(gene1 && (!gene2))
        return -1;

    if((!gene1) && (!gene2))
        return 0;

    if((!gene1) && gene2)
        return +1;

    return ensFeatureCompareStartAscending(gene1->Feature, gene2->Feature);
}




/* @func ensListGeneSortStartAscending ****************************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** Ensembl Feature start element in ascending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListGeneSortStartAscending(AjPList genes)
{
    if(!genes)
        return ajFalse;

    ajListSort(genes, listGeneCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listGeneCompareStartDescending *********************************
**
** AJAX List of Ensembl Gene objects comparison function to sort by
** Ensembl Feature start element in descending order.
**
** @param [r] P1 [const void*] Ensembl Gene address 1
** @param [r] P2 [const void*] Ensembl Gene address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listGeneCompareStartDescending(const void* P1, const void* P2)
{
    const EnsPGene gene1 = NULL;
    const EnsPGene gene2 = NULL;

    gene1 = *(EnsPGene const*) P1;
    gene2 = *(EnsPGene const*) P2;

    if(ajDebugTest("listGeneCompareStartDescending"))
        ajDebug("listGeneCompareStartDescending\n"
                "  gene1 %p\n"
                "  gene2 %p\n",
                gene1,
                gene2);

    /* Sort empty values towards the end of the AJAX List. */

    if(gene1 && (!gene2))
        return -1;

    if((!gene1) && (!gene2))
        return 0;

    if((!gene1) && gene2)
        return +1;

    return ensFeatureCompareStartDescending(gene1->Feature, gene2->Feature);
}




/* @func ensListGeneSortStartDescending ***************************************
**
** Sort an AJAX List of Ensembl Gene objects by their
** Ensembl Feature start element in descending order.
**
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListGeneSortStartDescending(AjPList genes)
{
    if(!genes)
        return ajFalse;

    ajListSort(genes, listGeneCompareStartDescending);

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
** @nam3rule Gene AJAX Table of AJAX unsigned integer key data and
**                Ensembl Gene value data
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




/* @funcstatic tableGeneClear *************************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Gene value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] Ensembl Gene address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableGeneClear(void** key,
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

    ensGeneDel((EnsPGene*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableGeneClear ****************************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Gene value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableGeneClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableGeneClear, NULL);

    return ajTrue;
}




/* @func ensTableGeneDelete **************************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** Ensembl Gene value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableGeneDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableGeneClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [EnsPGeneadaptor] Ensembl Gene Adaptor ************************
**
** @nam2rule Geneadaptor Functions for manipulating
** Ensembl Gene Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor
** @cc CVS Revision: 1.188
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic geneadaptorFetchAllbyStatement *********************************
**
** Fetch all Ensembl Gene objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::_objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] genes [AjPList] AJAX List of Ensembl Gene objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: In this implementation, the Ensembl Gene Adaptor does not LEFT JOIN
** the external_db table. Due to the rather small number of Ensembl External
** Database objects, potentially linked to a large number of Ensembl External
** Reference objects, all External Database objects are cached in the adaptor.
******************************************************************************/

static AjBool geneadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList genes)
{
    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint erid       = 0;
    ajuint edbid      = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;
    ajint tmpstart = 0;

    ajuint cantrcid = 0;
    ajuint version  = 0;

    AjBool circular     = AJFALSE;
    AjBool current = AJFALSE;

    EnsEGeneStatus estatus =
        ensEGeneStatusNULL;

    EnsEExternalreferenceInfotype erit = ensEExternalreferenceInfotypeNULL;

    AjPList mrs = NULL;

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

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseentry dbe = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPFeature feature = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("geneadaptorFetchAllbyStatement"))
        ajDebug("geneadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  genes %p\n",
                dba,
                statement,
                am,
                slice,
                genes);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!genes)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    ga = ensRegistryGetGeneadaptor(dba);

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
        analysisid    = 0;
        erid          = 0;
        description   = ajStrNew();
        source        = ajStrNew();
        biotype       = ajStrNew();
        status        = ajStrNew();
        current       = ajFalse;
        cantrcid      = 0;
        canann        = ajStrNew();
        stableid      = ajStrNew();
        version       = 0;
        cdate         = ajStrNew();
        mdate         = ajStrNew();
        edbid         = 0;
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

        /* Need to get the internal Ensembl Sequence Region identifier. */

        srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa, srid);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Ensembl Feature objects,
        ** the range needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("geneadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("geneadaptorFetchAllbyStatement got a "
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
            ** Skip Ensembl Feature objects that map to gaps or
            ** Coordinate System boundaries.
            */

            if(ensMapperresultGetType(mr) != ensEMapperresultTypeCoordinate)
            {
                /* Load the next Feature but destroy first! */

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
                ajFatal("geneadaptorFetchAllbyStatement got a Slice, "
                        "which length (%u) exceeds the "
                        "maximum integer limit (%d).",
                        ensSliceCalculateLength(slice), INT_MAX);

            ensSliceIsCircular(slice, &circular);

            if(ensSliceGetStrand(slice) >= 0)
            {
                /* On the positive strand ... */

                slstart = slstart - ensSliceGetStart(slice) + 1;
                slend   = slend   - ensSliceGetStart(slice) + 1;

                if(circular == ajTrue)
                {
                    if(slstart > slend)
                    {
                        /* A Feature overlapping the chromsome origin. */

                        /* Region in the beginning of the chromosome. */
                        if(slend > ensSliceGetStart(slice))
                            slstart -= sllength;

                        if(slend < 0)
                            slend += sllength;
                    }
                    else
                    {
                        if((ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                           && (slend < 0))
                        {
                            /*
                            ** A region overlapping the chromosome origin
                            ** and a Feature, which is at the beginning of
                            ** the chromosome.
                            */

                            slstart += sllength;
                            slend   += sllength;
                        }
                    }
                }
            }
            else
            {
                /* On the negative strand ... */

                if((circular == ajTrue) && (slstart > slend))
                {
                    /* Handle circular chromosomes. */

                    if(ensSliceGetStart(slice) > ensSliceGetEnd(slice))
                    {
                        tmpstart = slstart;
                        slstart = ensSliceGetEnd(slice) - slend + 1;
                        slend   = ensSliceGetEnd(slice) + sllength
                            - tmpstart + 1;
                    }
                    else
                    {

                        if(slend > ensSliceGetStart(slice))
                        {
                            /*
                            ** Looking at the region in the beginning of the
                            ** chromosome.
                            */

                            slstart = ensSliceGetEnd(slice) - slend + 1;
                            slend   = slend - sllength - slstart + 1;
                        }
                        else
                        {
                            tmpstart = slstart;
                            slstart  = ensSliceGetEnd(slice) - slend
                                - sllength + 1;
                            slend    = slend - tmpstart + 1;
                        }
                    }
                }
                else
                {
                    /* Non-circular Ensembl Slice objects... */

                    slend   = ensSliceGetEnd(slice) - slstart + 1;
                    slstart = ensSliceGetEnd(slice) - slend   + 1;
                }

                slstrand *= -1;
            }

            /*
            ** Throw away Ensembl Feature objects off the end of the requested
            ** Slice or on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Next feature but destroy first! */

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

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        if(erid)
        {
            ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

            erit = ensExternalreferenceInfotypeFromStr(erinfotype);

            if(!erit)
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

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        /* Set the Gene status. */

        estatus = ensGeneStatusFromStr(status);

        if(!estatus)
            ajDebug("geneadaptorFetchAllbyStatement encountered "
                    "unexpected string '%S' in the "
                    "'gene.status' field.\n", status);

        /* Finally, create a new Ensembl Gene. */

        feature = ensFeatureNewIniS(analysis,
                                    srslice,
                                    slstart,
                                    slend,
                                    slstrand);

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

        ensFeatureDel(&feature);

        ajListPushAppend(genes, (void*) gene);

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

        ensAnalysisDel(&analysis);

        ensDatabaseentryDel(&dbe);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    return ajTrue;
}




/* @funcstatic geneadaptorCacheReference **************************************
**
** Wrapper function to reference an Ensembl Gene from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Gene
**
** @return [void*] Ensembl Gene or NULL
** @@
******************************************************************************/

static void* geneadaptorCacheReference(void*value)
{
    if(!value)
        return NULL;

    return (void*) ensGeneNewRef((EnsPGene) value);
}




/* @funcstatic geneadaptorCacheDelete *****************************************
**
** Wrapper function to delete an Ensembl Gene from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Gene address
**
** @return [void]
** @@
******************************************************************************/

static void geneadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensGeneDel((EnsPGene*) value);

    return;
}




/* @funcstatic geneadaptorCacheSize *******************************************
**
** Wrapper function to determine the memory size of an Ensembl Gene
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Gene
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t geneadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensGeneCalculateMemsize((const EnsPGene) value);
}




/* @funcstatic geneadaptorGetFeature ******************************************
**
** Wrapper function to get the Ensembl Feature of an Ensembl Gene
** from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Gene
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature geneadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensGeneGetFeature((const EnsPGene) value);
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
** @valrule * [EnsPGeneadaptor] Ensembl Gene Adaptor
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
** @@
******************************************************************************/

EnsPGeneadaptor ensGeneadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPGeneadaptor ga = NULL;

    if(!dba)
        return NULL;

    AJNEW0(ga);

    ga->Adaptor = ensFeatureadaptorNew(
        dba,
        geneadaptorTables,
        geneadaptorColumns,
        geneadaptorLeftjoin,
        (const char*) NULL,
        (const char*) NULL,
        geneadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        geneadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        geneadaptorCacheDelete,
        geneadaptorCacheSize,
        geneadaptorGetFeature,
        "Gene");

    return ga;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Gene Adaptor object.
**
** @fdata [EnsPGeneadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Gene Adaptor object
**
** @argrule * Pga [EnsPGeneadaptor*] Ensembl Gene Adaptor object address
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
** @param [d] Pga [EnsPGeneadaptor*] Ensembl Gene Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGeneadaptorDel(EnsPGeneadaptor* Pga)
{
    EnsPGeneadaptor pthis = NULL;

    if(!Pga)
        return;

    if(!*Pga)
        return;

    pthis = *Pga;

    ensFeatureadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pga = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Gene Adaptor object.
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
** Get the Ensembl Database Adaptor element of the
** Ensembl Feature Adaptor element of an Ensembl Gene Adaptor.
**
** @param [r] ga [const EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGeneadaptorGetDatabaseadaptor(const EnsPGeneadaptor ga)
{
    if(!ga)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(ga->Adaptor);
}




/* @func ensGeneadaptorGetFeatureadaptor **************************************
**
** Get the Ensembl Feature Adaptor element of an Ensembl Gene Adaptor.
**
** @param [r] ga [const EnsPGeneadaptor] Ensembl Gene Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

EnsPFeatureadaptor ensGeneadaptorGetFeatureadaptor(const EnsPGeneadaptor ga)
{
    if(!ga)
        return NULL;

    return ga->Adaptor;
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
** @nam5rule Displaylabel Fetch by display label
** @nam5rule Identifier Fetch by SQL database-internal identifier
** @nam5rule Stableidentifier Fetch by a stable Ensembl Gene identifier
**
** @argrule * ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @argrule All genes [AjPList] AJAX List of Ensembl Gene objects
** @argrule AllbyBiotype biotype [const AjPStr] Biotype
** @argrule AllbyBiotype genes [AjPList] AJAX List of Ensembl Gene objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlice source [const AjPStr] Source name
** @argrule AllbySlice biotype [const AjPStr] Biotype name
** @argrule AllbySlice loadtranscripts [AjBool] Load Ensembl Transcript objects
** @argrule AllbySlice genes [AjPList] AJAX List of Ensembl Gene objects
** @argrule AllbyStableidentifier stableid [const AjPStr] Stable identifier
** @argrule AllbyStableidentifier genes [AjPList]
** AJAX List of Ensembl Gene objects
** @argrule ByDisplaylabel label [const AjPStr] Display label
** @argrule ByDisplaylabel Pgene [EnsPGene*] Ensembl Gene address
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pgene [EnsPGene*] Ensembl Gene address
** @argrule ByStableidentifier stableid [const AjPStr] Stable identifier
** @argrule ByStableidentifier version [ajuint] Version
** @argrule ByStableidentifier Pgene [EnsPGene*] Ensembl Gene address
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
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAll(EnsPGeneadaptor ga,
                              AjPList genes)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ga)
        return ajFalse;

    if(!genes)
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
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAllbyBiotype(EnsPGeneadaptor ga,
                                       const AjPStr biotype,
                                       AjPList genes)
{
    char* txtbiotype = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ga)
        return ajFalse;

    if(!biotype)
        return ajFalse;

    if(!genes)
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
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAllbySlice(EnsPGeneadaptor ga,
                                     EnsPSlice slice,
                                     const AjPStr anname,
                                     const AjPStr source,
                                     const AjPStr biotype,
                                     AjBool loadtranscripts,
                                     AjPList genes)
{
    void** keyarray = NULL;

    char* txtsource  = NULL;
    char* txtbiotype = NULL;

    ajint start = INT_MAX;
    ajint end   = INT_MIN;

    register ajuint i = 0;

    ajuint gnid = 0;
    ajuint trid = 0;

    ajuint* Pidentifier = NULL;

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

    if(!ga)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!genes)
        return ajFalse;

    dba = ensGeneadaptorGetDatabaseadaptor(ga);

    constraint = ajStrNewC("gene.is_current = 1");

    if(source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        ajFmtPrintAppS(&constraint, " AND gene.source = '%s'", txtsource);

        ajCharDel(&txtsource);
    }

    if(biotype && ajStrGetLen(biotype))
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

    if((!loadtranscripts) || (ajListGetLength(genes) == 0))
        return ajTrue;

    /*
    ** Preload all Ensembl Transcript objects now, instead of lazy loading
    ** later, which is faster than one query per Ensembl Transcript.
    ** First check if Ensembl Transcript objects are already preloaded.
    ** TODO: This should check all Ensembl Transcript objects.
    */

    ajListPeekFirst(genes, (void**) &gene);

    if(gene->Transcripts)
        return ajTrue;

    tca = ensRegistryGetTranscriptadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    /* Get the extent of the region spanned by Ensembl Transcript objects. */

    csv = ajStrNew();

    gntable = ensTableuintNewLen(ajListGetLength(genes));
    trtable = ensTableuintNewLen(ajListGetLength(genes));

    iter = ajListIterNew(genes);

    while(!ajListIterDone(iter))
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

        if(ajTableMatchV(gntable, (const void*) &gnid))
            ajDebug("ensGeneadaptorFetchAllbySlice got duplicate "
                    "Ensembl Gene with identifier %u.\n", gnid);
        else
        {
            AJNEW0(Pidentifier);

            *Pidentifier = gnid;

            ajTablePut(gntable,
                       (void*) Pidentifier,
                       (void*) ensGeneNewRef(gene));
        }
    }

    ajListIterDel(&iter);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    if((start >= ensSliceGetStart(slice)) && (end <= ensSliceGetEnd(slice)))
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

    while(!ajSqlrowiterDone(sqli))
    {
        trid = 0;
        gnid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &trid);
        ajSqlcolumnToUint(sqlr, &gnid);

        if(ajTableMatchV(trtable, (const void*) &trid))
        {
            ajDebug("ensGeneadaptorFetchAllbySlice got duplicate Transcript "
                    "with identifier %u for Gene with identifier %u.\n",
                    trid, gnid);

            continue;
        }

        gene = (EnsPGene) ajTableFetchmodV(gntable, (const void*) &gnid);

        if(gene)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = trid;

            ajTablePut(trtable,
                       (void*) Pidentifier,
                       (void*) ensGeneNewRef(gene));
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

    for(i = 0; keyarray[i]; i++)
        ajFmtPrintAppS(&csv, "%u, ", *((ajuint*) keyarray[i]));

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

    while(ajListPop(transcripts, (void**) &oldtranscript))
    {
        newtranscript = ensTranscriptTransfer(oldtranscript, newslice);

        if(!newtranscript)
            ajFatal("ensGeneAdaptorFetchAllbySlice could not transfer "
                    "Transcript onto new Slice.\n");

        trid = ensTranscriptGetIdentifier(newtranscript);

        gene = (EnsPGene) ajTableFetchmodV(trtable, (const void*) &trid);

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

    ensTableGeneDelete(&gntable);
    ensTableGeneDelete(&trtable);

    ensSliceDel(&newslice);

    return ajTrue;
}




/* @func ensGeneadaptorFetchAllbyStableidentifier *****************************
**
** Fetch all Ensembl Genes of all version via a stable identifier.
**
** The caller is responsible for deleting the Ensembl Genes before deleting
** the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::fetch_all_versions_by_stable_id
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [r] stableid [const AjPStr] Stable identifier
** @param [u] genes [AjPList] AJAX List of Ensembl Genes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchAllbyStableidentifier(EnsPGeneadaptor ga,
                                                const AjPStr stableid,
                                                AjPList genes)
{
    char* txtstableid = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ga)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!genes)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    constraint = ajFmtStr("gene_stable_id.stable_id = '%s'", txtstableid);

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
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByDisplaylabel(EnsPGeneadaptor ga,
                                         const AjPStr label,
                                         EnsPGene* Pgene)
{
    char* txtlabel = NULL;

    AjPList genes = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGene gene = NULL;

    if(!ga)
        return ajFalse;

    if(!label)
        return ajFalse;

    if(!Pgene)
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

    if(ajListGetLength(genes) > 1)
        ajDebug("ensGeneadaptorFetchByDisplaylabel got more than one "
                "Gene for display label '%S'.\n", label);

    ajListPop(genes, (void**) Pgene);

    while(ajListPop(genes, (void**) &gene))
        ensGeneDel(&gene);

    ajListFree(&genes);

    ajStrDel(&constraint);

    return ajTrue;
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
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByIdentifier(EnsPGeneadaptor ga,
                                       ajuint identifier,
                                       EnsPGene* Pgene)
{
    EnsPBaseadaptor ba = NULL;

    EnsPCache cache = NULL;

    if(!ga)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgene)
        return ajFalse;

    cache = ensFeatureadaptorGetCache(ga->Adaptor);

    ensCacheFetch(cache, (void*) &identifier, (void**) Pgene);

    if(*Pgene)
        return ajTrue;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorFetchByIdentifier(ba, identifier, (void**) Pgene);

    ensCacheStore(cache, (void*) &identifier, (void**) Pgene);

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
** @@
******************************************************************************/

AjBool ensGeneadaptorFetchByStableidentifier(EnsPGeneadaptor ga,
                                             const AjPStr stableid,
                                             ajuint version,
                                             EnsPGene* Pgene)
{
    char* txtstableid = NULL;

    AjPList genes = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGene gene = NULL;

    if(!ga)
        return ajFalse;

    if(!stableid)
        return ajFalse;

    if(!Pgene)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    ensBaseadaptorEscapeC(ba, &txtstableid, stableid);

    if(version)
        constraint = ajFmtStr(
            "gene_stable_id.stable_id = '%s' "
            "AND "
            "gene_stable_id.version = %u",
            txtstableid,
            version);
    else
        constraint = ajFmtStr(
            "gene_stable_id.stable_id = '%s' "
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

    if(ajListGetLength(genes) > 1)
        ajDebug("ensGeneadaptorFetchByStableidentifier got more than one "
                "Gene for stable identifier '%S' and version %u.\n",
                stableid,
                version);

    ajListPop(genes, (void**) Pgene);

    while(ajListPop(genes, (void**) &gene))
        ensGeneDel(&gene);

    ajListFree(&genes);

    ajStrDel(&constraint);

    return ajTrue;
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
** Fetch all SQL database-internal identifiers of Ensembl Genes.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::list_dbIDs
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneadaptorRetrieveAllIdentifiers(EnsPGeneadaptor ga,
                                            AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ga)
        return ajFalse;

    if(!identifiers)
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
** Fetch all stable identifiers of Ensembl Genes.
**
** The caller is responsible for deleting the AJAX String objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::GeneAdaptor::list_stable_ids
** @param [u] ga [EnsPGeneadaptor] Ensembl Gene Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGeneadaptorRetrieveAllStableidentifiers(EnsPGeneadaptor ga,
                                                  AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table   = NULL;
    AjPStr primary = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!ga)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(ga->Adaptor);

    table   = ajStrNewC("gene_stable_id");
    primary = ajStrNewC("stable_id");

    result = ensBaseadaptorRetrieveAllStrings(ba, table, primary, identifiers);

    ajStrDel(&table);
    ajStrDel(&primary);

    return result;
}

/* @source ensalign ***********************************************************
**
** Ensembl Alignment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.24 $
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

#include "ensalign.h"
#include "ensexon.h"
#include "ensexternaldatabase.h"
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

/* @conststatic dnaalignfeatureadaptorKTablenames *****************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL table names
**
******************************************************************************/

static const char *const dnaalignfeatureadaptorKTablenames[] =
{
    "dna_align_feature",
    (const char *) NULL
};




/* @conststatic dnaalignfeatureadaptorKColumnnames ****************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const dnaalignfeatureadaptorKColumnnames[] =
{
    "dna_align_feature.dna_align_feature_id",
    "dna_align_feature.seq_region_id",
    "dna_align_feature.seq_region_start",
    "dna_align_feature.seq_region_end",
    "dna_align_feature.seq_region_strand",
    "dna_align_feature.hit_start",
    "dna_align_feature.hit_end",
    "dna_align_feature.hit_strand",
    "dna_align_feature.hit_name",
    "dna_align_feature.analysis_id",
    "dna_align_feature.cigar_line",
    "dna_align_feature.score",
    "dna_align_feature.evalue",
    "dna_align_feature.perc_ident",
    "dna_align_feature.external_db_id",
    "dna_align_feature.hcoverage",
    "dna_align_feature.external_data",
    "dna_align_feature.pair_dna_align_feature_id",
    (const char *) NULL
};




/* @conststatic proteinalignfeatureadaptorKTablenames *************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL table names
**
******************************************************************************/

static const char *const proteinalignfeatureadaptorKTablenames[] =
{
    "protein_align_feature",
    (const char *) NULL
};




/* @conststatic proteinalignfeatureadaptorKColumnnames ************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const proteinalignfeatureadaptorKColumnnames[] =
{
    "protein_align_feature.protein_align_feature_id",
    "protein_align_feature.seq_region_id",
    "protein_align_feature.seq_region_start",
    "protein_align_feature.seq_region_end",
    "protein_align_feature.seq_region_strand",
    "protein_align_feature.hit_start",
    "protein_align_feature.hit_end",
    "protein_align_feature.hit_name",
    "protein_align_feature.analysis_id",
    "protein_align_feature.cigar_line",
    "protein_align_feature.score",
    "protein_align_feature.evalue",
    "protein_align_feature.perc_ident",
    "protein_align_feature.external_db_id",
    "protein_align_feature.hcoverage",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool basealignfeatureParseFeaturepairs(EnsPBasealignfeature baf,
                                                AjPList objects);

static AjBool basealignfeatureParseCigar(const EnsPBasealignfeature baf,
                                         AjPList fps);

static int listBasealignfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listBasealignfeatureCompareSourceEndAscending(
    const void *item1,
    const void *item2);

static int listBasealignfeatureCompareSourceEndDescending(
    const void *item1,
    const void *item2);

static int listBasealignfeatureCompareSourceStartAscending(
    const void *item1,
    const void *item2);

static int listBasealignfeatureCompareSourceStartDescending(
    const void *item1,
    const void *item2);

static AjBool dnaalignfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs);

static AjBool proteinalignfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensalign ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPBasealignfeature] Ensembl Base Align Feature *************
**
** @nam2rule Basealignfeature Functions for manipulating
** Ensembl Base Align Feature objects
**
** @cc Bio::EnsEMBL::BaseAlignFeature
** @cc CVS Revision: 1.65
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::DnaDnaAlignFeature
** @cc CVS Revision: 1.27
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::DnaPepAlignFeature
** @cc CVS Revision: 1.18
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::PepDnaAlignFeature
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic basealignfeatureParseFeaturepairs ******************************
**
** Parse Ensembl Base Align Feature objects from an AJAX List of
** Ensembl Objects based on Ensembl Feature Pair objects.
** This function creates an internal CIGAR string and sets internal alignment
** coordinates.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] objects [AjPList]
** AJAX List of Ensembl Objects based on Ensembl Feature Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool basealignfeatureParseFeaturepairs(EnsPBasealignfeature baf,
                                                AjPList objects)
{
    void *Pobject = NULL;

    ajint srcstart = 0;
    ajint trgstart = 0;

    ajint srcend = 0;
    ajint trgend = 0;

    ajint srcgap = 0;
    ajint trggap = 0;

    ajint srclength = 0;
    ajint trglength = 0;

    ajint srcpos = 0; /* where last feature q part ended */
    ajint trgpos = 0; /* where last feature s part ended */

    ajint match = 0;

    ajuint srcunit = 0U;
    ajuint trgunit = 0U;

    AjBool insertion = AJFALSE;
    AjBool warning   = AJFALSE;

    AjIList iter = NULL;
    AjPList fps  = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeature csf = NULL; /* Current source Feature */
    EnsPFeature ctf = NULL; /* Current target Feature */
    EnsPFeature fsf = NULL; /* First source Feature */
    EnsPFeature ftf = NULL; /* First target Feature */
    EnsPFeature lsf = NULL; /* Last source Feature */
    EnsPFeature ltf = NULL; /* Last target Feature */

    EnsPFeaturepair cfp = NULL; /* Current Feature Pair */
    EnsPFeaturepair ffp = NULL; /* First Feature Pair */
    EnsPFeaturepair lfp = NULL; /* Last Feature Pair */

    if (!baf)
        return ajFalse;

    if (!objects)
        return ajFalse;

    if (!ajListGetLength(objects))
        return ajTrue;

    srcunit = ensBasealignfeatureCalculateUnitSource(baf);
    trgunit = ensBasealignfeatureCalculateUnitTarget(baf);

    if (baf->Cigar)
        ajStrAssignClear(&baf->Cigar);
    else
        baf->Cigar = ajStrNew();

    /* Extract Ensembl Feature Pair objects from Ensembl Objects. */

    iter = ajListIterNew(objects);

    while (!ajListIterDone(iter))
    {
        Pobject = ajListIterGet(iter);

        ajListPushAppend(fps,
                         (void *) (*baf->FobjectGetFeaturepair) (Pobject));
    }

    ajListIterDel(&iter);

    /*
    ** Sort the AJAX List of Ensembl Feature Pair objects on their source
    ** Ensembl Feature start member in ascending order on the positive strand
    ** and descending order on the negative strand.
    */

    ajListPeekFirst(fps, (void **) &ffp);

    fsf = ensFeaturepairGetSourceFeature(ffp);

    if (ensFeatureGetStrand(fsf) >= 0)
        ensListFeaturepairSortSourceStartAscending(fps);
    else
        ensListFeaturepairSortSourceStartDescending(fps);

    /*
    ** Use strandedness info of source and target to make sure both sets of
    ** start and end coordinates are oriented the right way around.
    */

    ajListPeekFirst(fps, (void **) &ffp);
    ajListPeekLast(fps, (void **) &lfp);

    fsf = ensFeaturepairGetSourceFeature(ffp);
    ftf = ensFeaturepairGetTargetFeature(ffp);
    lsf = ensFeaturepairGetSourceFeature(lfp);
    ltf = ensFeaturepairGetTargetFeature(lfp);

    if (ensFeatureGetStrand(fsf) >= 0)
    {
        srcstart = ensFeatureGetStart(fsf);
        srcend   = ensFeatureGetEnd(lsf);
    }
    else
    {
        srcstart = ensFeatureGetStart(lsf);
        srcend   = ensFeatureGetEnd(fsf);
    }

    if (ensFeatureGetStrand(ftf) >= 0)
    {
        trgstart = ensFeatureGetStart(ftf);
        trgend   = ensFeatureGetEnd(ltf);
    }
    else
    {
        trgstart = ensFeatureGetStart(ltf);
        trgend   = ensFeatureGetEnd(ftf);
    }

    /*
    ** Loop through each portion of the alignment (Feature Pair) and
    ** construct a CIGAR string.
    */

    iter = ajListIterNew(fps);

    while (!ajListIterDone(iter))
    {
        cfp = (EnsPFeaturepair) ajListIterGet(iter);

        csf = ensFeaturepairGetSourceFeature(cfp);
        ctf = ensFeaturepairGetTargetFeature(cfp);

        /* Sanity checks */

        if (ensFeatureGetSlice(csf) &&
            ensFeatureGetSlice(fsf) &&
            (!ensSliceMatch(ensFeatureGetSlice(csf),
                            ensFeatureGetSlice(fsf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source Feature Slice members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeatureGetSequencename(csf) &&
            ensFeatureGetSequencename(fsf) &&
            (!ajStrMatchS(ensFeatureGetSequencename(csf),
                          ensFeatureGetSequencename(fsf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source Feature sequence name members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeatureGetStrand(csf) != ensFeatureGetStrand(fsf))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source Feature strand members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeatureGetSlice(ctf) &&
            ensFeatureGetSlice(ftf) &&
            (!ensSliceMatch(ensFeatureGetSlice(ctf),
                            ensFeatureGetSlice(ftf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target Feature Slice members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeatureGetSequencename(ctf) &&
            ensFeatureGetSequencename(ftf) &&
            (!ajStrMatchCaseS(ensFeatureGetSequencename(ctf),
                              ensFeatureGetSequencename(ftf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target Feature sequence name members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeatureGetStrand(ctf) != ensFeatureGetStrand(ftf))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target Feature strand members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeaturepairGetSourceSpecies(cfp) &&
            ensFeaturepairGetSourceSpecies(ffp) &&
            (!ajStrMatchS(ensFeaturepairGetSourceSpecies(cfp),
                          ensFeaturepairGetSourceSpecies(ffp))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source species members between the "
                    "current and first Ensembl Feature Pair.\n");

        if (ensFeaturepairGetTargetSpecies(cfp) &&
            ensFeaturepairGetTargetSpecies(ffp) &&
            (!ajStrMatchS(ensFeaturepairGetTargetSpecies(cfp),
                          ensFeaturepairGetTargetSpecies(ffp))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target species members between the "
                    "current and first Ensembl Feature Pair.\n");

        /*
        ** NOTE: The score, percent identity and P-values are not tested for
        ** equality as this is unreliable for float and double types.
        **
        ** More sanity checking
        */

        if (srcpos)
        {
            if (ensFeatureGetStrand(csf) >= 0)
            {
                if (ensFeatureGetStart(csf) < srcpos)
                    ajFatal("Inconsistent coordinates in "
                            "Ensembl Feature Pair List (forward strand).\n"
                            "Start (%d) of current Feature Pair should be "
                            "greater than previous Feature Pair end (%d).\n",
                            ensFeatureGetStart(csf), srcpos);
            }
            else
            {
                if (ensFeatureGetEnd(csf) > srcpos)
                    ajFatal("Inconsistent coordinates in "
                            "Ensembl Feature Pair List (reverse strand).\n"
                            "End (%d) of current Feature Pair should be "
                            "less than previous Feature Pair start (%d).\n",
                            ensFeatureGetEnd(csf), srcpos);
            }
        }

        srclength = ensFeatureCalculateLength(csf);
        trglength = ensFeatureCalculateLength(ctf);

        /*
        ** using multiplication to avoid rounding errors, hence the
        ** switch from source to target for the ratios
        */

        /* Yet more sanity checking */

        if (srcunit > trgunit)
        {
            /*
            ** I am going to make the assumption here that this situation
            ** will only occur with DnaPepAlignFeatures, this may not be true
            */

            if ((srclength / srcunit) != (trglength * trgunit))
                ajFatal("Feature Pair lengths not comparable "
                        "Lengths: %d %d "
                        "Ratios: %u %u.\n",
                        srclength, trglength,
                        srcunit, trgunit);
        }
        else
        {
            if ((srclength * trgunit) != (trglength * srcunit))
                ajFatal("Feature Pair lengths not comparable "
                        "Lengths: %d %d "
                        "Ratios: %u %u.\n",
                        srclength, trglength,
                        srcunit, trgunit);
        }

        /*
        ** Check to see if there is an I type (insertion) gap:
        ** If there is a space between the end of the last source sequence
        ** alignment and the start of this one, then this is an insertion
        */

        insertion = ajFalse;

        if (ensFeatureGetStrand(fsf) >= 0)
        {
            if (srcpos && (ensFeatureGetStart(csf) > (srcpos + 1)))
            {
                /* there is an insertion */

                insertion = ajTrue;

                srcgap = ensFeatureGetStart(csf) - srcpos - 1;

                /* no need for a number if gap length is 1 */

                if (srcgap == 1)
                    ajStrAppendK(&baf->Cigar, 'I');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dI", srcgap);
            }

            /* shift our position in the source sequence alignment */

            srcpos = ensFeatureGetEnd(csf);
        }
        else
        {
            if (srcpos && ((ensFeatureGetEnd(csf) + 1) < srcpos))
            {
                /* there is an insertion */

                insertion = ajTrue;

                srcgap = srcpos - ensFeatureGetEnd(csf) - 1;

                /* no need for a number if gap length is 1 */

                if (srcgap == 1)
                    ajStrAppendK(&baf->Cigar, 'I');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dI", srcgap);
            }

            /* shift our position in the source sequence alignment */

            srcpos = ensFeatureGetStart(csf);
        }

        /*
        ** Check to see if there is a D type (deletion) gap
        ** There is a deletion gap if there is a space between the end of the
        ** last portion of the hit sequence alignment and this one
        */

        if (ensFeatureGetStrand(ctf) >= 0)
        {
            if (trgpos && (ensFeatureGetStart(ctf) > (trgpos + 1)))
            {
                /* there is a deletion */

                srcgap = ensFeatureGetStart(ctf) - trgpos - 1;

                trggap = (ajint) (srcgap * srcunit / trgunit + 0.5);

                /* no need for a number if gap length is 1 */

                if (trggap == 1)
                    ajStrAppendK(&baf->Cigar, 'D');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dD", trggap);

                /* sanity check, should not be an insertion and deletion */

                if (insertion)
                {
                    if (!warning)
                    {
                        ajWarn("Should not be an deletion and insertion on "
                               "the same alignment region. "
                               "CIGAR line '%S'\n", baf->Cigar);

                        warning = ajTrue;
                    }
                }
            }

            /* shift our position in the target sequence alignment */

            trgpos = ensFeatureGetEnd(ctf);
        }
        else
        {
            if (trgpos && ((ensFeatureGetEnd(ctf) + 1) < trgpos))
            {
                /* there is a deletion */

                srcgap = trgpos - ensFeatureGetEnd(ctf) - 1;

                trggap = (ajint) (srcgap * srcunit / trgunit + 0.5);

                /* no need for a number if gap length is 1 */

                if (trggap == 1)
                    ajStrAppendK(&baf->Cigar, 'D');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dD", trggap);

                /* sanity check,  should not be an insertion and deletion */

                if (insertion)
                {
                    if (!warning)
                    {
                        ajWarn("Should not be an deletion and insertion on "
                               "the same alignment region. "
                               "target position %d "
                               "target end %d "
                               "CIGAR line '%S'\n",
                               trgpos,
                               ensFeatureGetEnd(ctf),
                               baf->Cigar);

                        warning = ajTrue;
                    }
                }
            }

            /* shift our position in the target sequence alignment */

            trgpos = ensFeatureGetStart(ctf);
        }

        match = ensFeatureCalculateLength(csf);

        if (match == 1)
            ajStrAppendK(&baf->Cigar, 'M');
        else
            ajFmtPrintAppS(&baf->Cigar, "%dM", match);
    }

    /* Replace the Feature Pair in the Base Align Feature. */

    ensFeaturepairDel(&baf->Featurepair);

    /* Clone the source Feature and set the new coordinates. */

    srcfeature = ensFeatureNewCpy(ffp->SourceFeature);

    ensFeatureSetStart(srcfeature, srcstart);
    ensFeatureSetEnd(srcfeature, srcend);

    /* Clone the target Feature and set the new coordinates. */

    trgfeature = ensFeatureNewCpy(ffp->TargetFeature);

    ensFeatureSetStart(trgfeature, trgstart);
    ensFeatureSetEnd(trgfeature, trgend);

    /*
    ** Clone the Ensembl Feature Pair and set new Ensembl Feature objects for
    ** the source and target.
    */

    baf->Featurepair = ensFeaturepairNewCpy(ffp);

    ensFeaturepairSetSourceFeature(baf->Featurepair, srcfeature);
    ensFeaturepairSetTargetFeature(baf->Featurepair, trgfeature);

    /* Delete the cloned Ensembl Feature objects for the source and target. */

    ensFeatureDel(&srcfeature);
    ensFeatureDel(&trgfeature);

    return ajTrue;
}




/* @funcstatic basealignfeatureParseCigar *************************************
**
** Convert an Ensembl Base Align Feature CIGAR line member into an
** AJAX List of Ensembl Feature Pair objects.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool basealignfeatureParseCigar(const EnsPBasealignfeature baf,
                                         AjPList fps)
{
    ajint fpsrcstart = 0;
    ajint fptrgstart = 0;

    ajint srcstart = 0;
    ajint trgstart = 0;

    ajint srcend = 0;
    ajint trgend = 0;

    ajint tlength = 0;
    ajint mlength = 0;

    ajuint srcunit = 0U;
    ajuint trgunit = 0U;

    const AjPStr token = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    if (!baf)
        return ajFalse;

    if (!fps)
        return ajFalse;

    if ((!baf->Cigar) && (!ajStrGetLen(baf->Cigar)))
        ajFatal("basealignfeatureParseCigar no CIGAR string defined!\n");

    if (!baf->Featurepair)
        ajFatal("basealignfeatureParseCigar got an Ensembl Base Align Feature "
                "without an Ensembl Feature Pair.\n");

    if (ensFeaturepairGetSourceFeature(baf->Featurepair) == NULL)
        ajFatal("basealignfeatureParseCigar got an Ensembl Base Align Feature "
                "without a source Ensembl Feature in the "
                "Ensembl Feature Pair.\n");

    if (ensFeaturepairGetTargetFeature(baf->Featurepair) == NULL)
        ajFatal("basealignfeatureParseCigar got an Ensembl Base Align Feature "
                "without a target Ensembl Feature in the "
                "Ensembl Feature Pair.\n");

    srcunit = ensBasealignfeatureCalculateUnitSource(baf);
    trgunit = ensBasealignfeatureCalculateUnitTarget(baf);

    if (ensFeaturepairGetSourceStrand(baf->Featurepair) > 0)
        fpsrcstart = ensFeaturepairGetSourceStart(baf->Featurepair);
    else
        fpsrcstart = ensFeaturepairGetSourceEnd(baf->Featurepair);

    if (ensFeaturepairGetTargetStrand(baf->Featurepair) > 0)
        fptrgstart = ensFeaturepairGetTargetStart(baf->Featurepair);
    else
        fptrgstart = ensFeaturepairGetTargetEnd(baf->Featurepair);

    /* Construct ungapped blocks as Feature Pair objects for each MATCH. */

    while ((token = ajStrParseC(baf->Cigar, "MDI")))
    {
        ajStrToInt(token, &tlength);

        tlength = (tlength) ? tlength : 1;

        if ((srcunit == 1) && (trgunit == 3))
            mlength = tlength * 3;
        else if ((srcunit == 3) && (trgunit == 1))
            mlength = tlength / 3;
        else if ((srcunit == 1) && (trgunit == 1))
            mlength = tlength;
        else
            ajFatal("basealignfeatureParseCigar got "
                    "Base Align Feature source unit %u "
                    "Base Align Feature target unit %u, "
                    "but currently only 1 or 3 are allowed.\n",
                    srcunit,
                    trgunit);

        if (ajStrMatchC(token, "M"))
        {
            /* MATCH */

            if (ensFeaturepairGetSourceStrand(baf->Featurepair) > 0)
            {
                srcstart = fpsrcstart;
                srcend   = fpsrcstart + tlength - 1;

                fpsrcstart = srcend + 1;
            }
            else
            {
                srcend   = fpsrcstart;
                srcstart = fpsrcstart - tlength + 1;

                fpsrcstart = srcstart - 1;
            }

            if (ensFeaturepairGetTargetStrand(baf->Featurepair) > 0)
            {
                trgstart = fptrgstart;
                trgend   = fptrgstart + mlength - 1;

                fptrgstart = trgend + 1;
            }
            else
            {
                trgend   = fptrgstart;
                trgstart = fptrgstart - mlength + 1;

                fptrgstart = trgstart - 1;
            }

            /* Clone the source Feature and set the new coordinates. */

            srcfeature = ensFeatureNewCpy(
                ensFeaturepairGetSourceFeature(baf->Featurepair));

            ensFeatureSetStart(srcfeature, srcstart);
            ensFeatureSetEnd(srcfeature, srcend);

            /* Clone the target Feature and set the new coordinates. */

            trgfeature = ensFeatureNewCpy(
                ensFeaturepairGetTargetFeature(baf->Featurepair));

            ensFeatureSetStart(trgfeature, trgstart);
            ensFeatureSetEnd(trgfeature, trgend);

            /*
            ** Clone the Ensembl Feature Pair and set the new
            ** Ensembl Feature objects for the source and target.
            */

            fp = ensFeaturepairNewCpy(baf->Featurepair);

            ensFeaturepairSetSourceFeature(baf->Featurepair, srcfeature);
            ensFeaturepairSetTargetFeature(baf->Featurepair, trgfeature);

            /* Delete the Ensembl Feature objects for the source and target. */

            ensFeatureDel(&srcfeature);
            ensFeatureDel(&trgfeature);

            ajListPushAppend(fps, (void *) fp);
        }
        else if (ajStrMatchC(token, "I"))
        {
            /* INSERT */

            if (ensFeaturepairGetSourceStrand(baf->Featurepair) > 0)
                fpsrcstart += tlength;
            else
                fpsrcstart -= tlength;
        }

        else if (ajStrMatchC(token, "D"))
        {
            /* DELETION */

            if (ensFeaturepairGetTargetStrand(baf->Featurepair) > 0)
                fptrgstart += mlength;
            else
                fptrgstart -= mlength;
        }
        else
            ajFatal("basealignfeatureParseCigar "
                    "illegal CIGAR line token '%S'\n",
                    token);
    }

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Base Align Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Base Align Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Fps Constructor from Ensembl Feature Pair objects
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @suffix D Constructor for a Base Align Feature of type 'DNA'
** @suffix P Constructor for a Base Align Feature of type 'Protein'
**
** @argrule Cpy baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Fps fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule Fps cigar [AjPStr] CIGAR line
** @argrule Fps fps [AjPList] AJAX List of Ensembl Feature Pair objects
** @argrule Fps type [EnsEBasealignfeatureType]
** Ensembl Base Align Feature Type enumeration
** @argrule Fps pair [ajuint] Pair DNA Align Feature identifier
** @argrule IniD dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @argrule IniD identifier [ajuint] SQL database-internal identifier
** @argrule IniD fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule IniD cigar [AjPStr] CIGAR line
** @argrule IniD pair [ajuint] Pair DNA Align Feature identifier
** @argrule IniP pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @argrule IniP identifier [ajuint] SQL database-internal identifier
** @argrule IniP fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule IniP cigar [AjPStr] CIGAR line
** @argrule Ref baf [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @valrule * [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensBasealignfeatureNewCpy ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewCpy(
    const EnsPBasealignfeature baf)
{
    EnsPBasealignfeature pthis = NULL;

    if (!baf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier                 = baf->Identifier;
    pthis->Dnaalignfeatureadaptor     = baf->Dnaalignfeatureadaptor;
    pthis->Proteinalignfeatureadaptor = baf->Proteinalignfeatureadaptor;
    pthis->Featurepair                = ensFeaturepairNewRef(baf->Featurepair);
    pthis->FobjectGetFeaturepair      = baf->FobjectGetFeaturepair;

    if (baf->Cigar)
        pthis->Cigar = ajStrNewRef(baf->Cigar);

    pthis->Type = baf->Type;

    pthis->Alignmentlength = baf->Alignmentlength;

    pthis->Pairdnaalignfeatureidentifier = baf->Pairdnaalignfeatureidentifier;

    return pthis;
}




/* @func ensBasealignfeatureNewFps ********************************************
**
** Constructor for an Ensembl Base Align Feature with
** Ensembl Feature Pair objects.
**
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
** @param [u] type [EnsEBasealignfeatureType]
** Ensembl Base Align Feature Type enumeration
** @param [r] pair [ajuint] Pair DNA Align Feature identifier
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewFps(
    EnsPFeaturepair fp,
    AjPStr cigar,
    AjPList fps,
    EnsEBasealignfeatureType type,
    ajuint pair)
{
    EnsPBasealignfeature baf = NULL;

    if (!fp)
        return NULL;

    if ((type < ensEBasealignfeatureTypeDNA) ||
        (type > ensEBasealignfeatureTypeProtein))
    {
        ajDebug("ensBasealignfeatureNewFps got illegal type (%d).\n",
                type);

        return NULL;
    }

    AJNEW0(baf);

    baf->Use = 1U;

    baf->Featurepair           = ensFeaturepairNewRef(fp);
    baf->FobjectGetFeaturepair = (EnsPFeaturepair (*) (const void*))
        &ensBasealignfeatureGetFeaturepair;

    if (cigar && ajStrGetLen(cigar) && fps && ajListGetLength(fps))
    {
        ajDebug("ensBasealignfeatureNewFps requires a CIGAR line "
                "or an AJAX List of Ensembl Feature Pair objects, "
                "not both.\n");

        AJFREE(baf);

        return NULL;
    }
    else if (cigar && ajStrGetLen(cigar))
        baf->Cigar = ajStrNewRef(cigar);
    else if (fps && ajListGetLength(fps))
        basealignfeatureParseFeaturepairs(baf, fps);
    else
        ajDebug("ensBasealignfeatureNewFps requires either a CIGAR line or "
                "an AJAX List of Ensembl Feature Pair objects.\n");

    baf->Type = type;

    baf->Pairdnaalignfeatureidentifier = pair;

    return baf;
}




/* @func ensBasealignfeatureNewIniD *******************************************
**
** Constructor for an Ensembl Base Align Feature of type "DNA"
** with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
** @param [r] pair [ajuint] Pair DNA Align Feature identifier
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewIniD(
    EnsPDnaalignfeatureadaptor dafa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar,
    ajuint pair)
{
    EnsPBasealignfeature baf = NULL;

    if (!fp)
        return NULL;

    if (!cigar)
        return NULL;

    AJNEW0(baf);

    baf->Use = 1U;

    baf->Identifier = identifier;

    baf->Dnaalignfeatureadaptor = dafa;

    baf->Featurepair = ensFeaturepairNewRef(fp);

    baf->FobjectGetFeaturepair = (EnsPFeaturepair (*) (const void*))
        &ensBasealignfeatureGetFeaturepair;

    if (cigar)
        baf->Cigar = ajStrNewRef(cigar);

    baf->Type = ensEBasealignfeatureTypeDNA;

    baf->Pairdnaalignfeatureidentifier = pair;

    return baf;
}




/* @func ensBasealignfeatureNewIniP *******************************************
**
** Constructor for an Ensembl Base Align Feature of type "Protein"
** with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewIniP(
    EnsPProteinalignfeatureadaptor pafa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar)
{
    EnsPBasealignfeature baf = NULL;

    if (!fp)
        return NULL;

    if (!cigar)
        return NULL;

    AJNEW0(baf);

    baf->Use = 1U;

    baf->Identifier = identifier;

    baf->Proteinalignfeatureadaptor = pafa;

    baf->Featurepair = ensFeaturepairNewRef(fp);

    baf->FobjectGetFeaturepair = (EnsPFeaturepair (*) (const void*))
        &ensBasealignfeatureGetFeaturepair;

    if (cigar)
        baf->Cigar = ajStrNewRef(cigar);

    baf->Type = ensEBasealignfeatureTypeProtein;

    baf->Pairdnaalignfeatureidentifier = 0;

    return baf;
}




/* @func ensBasealignfeatureNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewRef(
    EnsPBasealignfeature baf)
{
    if (!baf)
        return NULL;

    baf->Use++;

    return baf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Base Align Feature
**
** @argrule * Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensBasealignfeatureDel ***********************************************
**
** Default destructor for an Ensembl Base Align Feature.
**
** @param [d] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensBasealignfeatureDel(
    EnsPBasealignfeature *Pbaf)
{
    EnsPBasealignfeature pthis = NULL;

    if (!Pbaf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensBasealignfeatureDel"))
    {
        ajDebug("ensBasealignfeatureDel\n"
                "  *Pbaf %p\n",
                *Pbaf);

        ensBasealignfeatureTrace(*Pbaf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pbaf) || --pthis->Use)
    {
        *Pbaf = NULL;

        return;
    }

    ensFeaturepairDel(&pthis->Featurepair);

    ajStrDel(&pthis->Cigar);

    ajMemFree((void **) Pbaf);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Get Return Base Align Feature attribute(s)
** @nam4rule Cigar Return the CIGAR line
** @nam4rule Dnaalignfeatureadaptor
** Return the Ensembl DNA Align Feature Adaptor
** @nam4rule Featurepair Return the Ensembl Feature Pair
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Pairdnaalignfeatureidentifier
** Return the pair DNA Align Feature identifier
** @nam4rule Proteinalignfeatureadaptor
** Return the Ensembl Protein Align Feature Adaptor
** @nam4rule Type Return the type
**
** @argrule * baf [const EnsPBasealignfeature] Base Align Feature
**
** @valrule Cigar [AjPStr] CIGAR line or NULL
** @valrule Dnaalignfeatureadaptor [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
** @valrule Featurepair [EnsPFeaturepair] Ensembl Feature Pair or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Pairdnaalignfeatureidentifier [ajuint]
** Pair Ensembl DNA Align Feature identifier or 0
** @valrule Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
** @valrule Type [EnsEBasealignfeatureType]
** Ensembl Base Align Feature Type enumeration or ensEBasealignfeatureTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensBasealignfeatureGetCigar ******************************************
**
** Get the CIGAR line member of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [AjPStr] CIGAR line or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensBasealignfeatureGetCigar(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Cigar : NULL;
}




/* @func ensBasealignfeatureGetDnaalignfeatureadaptor *************************
**
** Get the Ensembl DNA Align Feature Adaptor member of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDnaalignfeatureadaptor ensBasealignfeatureGetDnaalignfeatureadaptor(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Dnaalignfeatureadaptor : NULL;
}




/* @func ensBasealignfeatureGetFeaturepair ************************************
**
** Get the Ensembl Feature Pair member of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeaturepair ensBasealignfeatureGetFeaturepair(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Featurepair : NULL;
}




/* @func ensBasealignfeatureGetIdentifier *************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetIdentifier(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Identifier : 0U;
}




/* @func ensBasealignfeatureGetPairdnaalignfeatureidentifier ******************
**
** Get the pair Ensembl DNA Align Feature identifier member of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::DnaDnaAlignFeature::pair_dna_align_feature_id
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint]
** Pair Ensembl DNA Align Feature identifier or 0U
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetPairdnaalignfeatureidentifier(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Pairdnaalignfeatureidentifier : 0U;
}




/* @func ensBasealignfeatureGetProteinalignfeatureadaptor *********************
**
** Get the Ensembl Protein Align Feature Adaptor member of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensBasealignfeatureGetProteinalignfeatureadaptor(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Proteinalignfeatureadaptor : NULL;
}




/* @func ensBasealignfeatureGetType *******************************************
**
** Get the Base Align Feature Type enumeration member of an
** Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsEBasealignfeatureType]
** Ensembl Base Align Feature Type enumeration or ensEBasealignfeatureTypeNULL
** @@
******************************************************************************/

EnsEBasealignfeatureType ensBasealignfeatureGetType(
    const EnsPBasealignfeature baf)
{
    return (baf) ? baf->Type : ensEBasealignfeatureTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Set Set one member of a Base Align Feature
** @nam4rule Featurepair Set the Ensembl Feature Pair
**
** @argrule * baf [EnsPBasealignfeature] Ensembl Base Align Feature object
** @argrule Featurepair fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensBasealignfeatureSetFeaturepair ************************************
**
** Set the Ensembl Feature Pair member of an Ensembl Base Align Feature.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBasealignfeatureSetFeaturepair(EnsPBasealignfeature baf,
                                         EnsPFeaturepair fp)
{
    if (ajDebugTest("ensBasealignfeatureSetFeaturepair"))
    {
        ajDebug("ensBasealignfeatureSetFeaturepair\n"
                "  baf %p\n"
                "  fp %p\n",
                baf,
                fp);

        ensBasealignfeatureTrace(baf, 1);

        ensFeaturepairTrace(fp, 1);
    }

    if (!baf)
        return ajFalse;

    if (!fp)
        return ajFalse;

    /* Replace the current Feature Pair. */

    ensFeaturepairDel(&baf->Featurepair);

    baf->Featurepair = ensFeaturepairNewRef(fp);

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Base Align Feature convenience functions
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Feature Get the (source) Ensembl Feature
**
** @argrule * baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @valrule Feature [EnsPFeature] Ensembl Feature
**
** @fcategory use
******************************************************************************/




/* @func ensBasealignfeatureGetFeature ****************************************
**
** Get the (source) Ensembl Feature member of an Ensembl Feature Pair member
** of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPFeature] Ensembl Feature
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensBasealignfeatureGetFeature(const EnsPBasealignfeature baf)
{
    if (!baf)
        return NULL;

    return ensFeaturepairGetSourceFeature(baf->Featurepair);
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Calculate Calculate Ensembl Base Align Feature information
** @nam4rule Adaptor Calculate the adaptor
** @nam4rule Alignmentlength Calculate the alignment length
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Unit Calculate an alignment unit
** @nam5rule Source Calculate the source unit
** @nam5rule Target Calculate the target unit
**
** @argrule Adaptor baf [const EnsPBasealignfeature]
** Ensembl Base Align Feature
** @argrule Alignmentlength baf [EnsPBasealignfeature]
** Ensembl Base Align Feature
** @argrule Memsize baf [const EnsPBasealignfeature]
** Ensembl Base Align Feature
** @argrule UnitSource baf [const EnsPBasealignfeature]
** Ensembl Base Align Feature
** @argrule UnitTarget baf [const EnsPBasealignfeature]
** Ensembl Base Align Feature
**
** @valrule Adaptor [void*]
** Ensembl DNA or Protein Align Feature Adaptor or NULL
** @valrule Alignmentlength [ajuint] Alignment length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule UnitSource [ajuint] Alignment unit for the source part or 0U
** @valrule UnitTarget [ajuint] Alignment unit for the target part or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensBasealignfeatureCalculateAdaptor **********************************
**
** Calculate the Adaptor member of an Ensembl Base Align Feature.
** This will return an EnsPDnaalignfeatureadaptor for an Ensembl Base Align
** Feature of type ensEBasealignfeatureTypeDNA and an
** EnsPProteinalignfeatureadaptor for a feature of type
** ensEBasealignfeatureTypeProtein.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [void*] Ensembl DNA or Protein Align Feature Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

void* ensBasealignfeatureCalculateAdaptor(const EnsPBasealignfeature baf)
{
    void *Padaptor = NULL;

    if (!baf)
        return NULL;

    switch (baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            Padaptor = (void *) baf->Dnaalignfeatureadaptor;

            break;

        case ensEBasealignfeatureTypeProtein:

            Padaptor = (void *) baf->Proteinalignfeatureadaptor;

            break;

        default:

            ajWarn("ensBasealignfeatureCalculateAdaptor got an "
                   "Ensembl Base Align Feature with unexpected type %d.",
                   baf->Type);
    }

    return Padaptor;
}




/* @func ensBasealignfeatureCalculateAlignmentlength **************************
**
** Calculate the alignment length member of an Ensembl Base Align Feature.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Alignment length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensBasealignfeatureCalculateAlignmentlength(EnsPBasealignfeature baf)
{
    ajint tlength = 0;

    const AjPStr token = NULL;

    if (!baf)
        return 0U;

    if (!baf->Alignmentlength && baf->Cigar)
    {
        while ((token = ajStrParseC(baf->Cigar, "MDI")))
        {
            ajStrToInt(token, &tlength);

            tlength = tlength ? tlength : 1;

            baf->Alignmentlength += tlength;
        }
    }

    return baf->Alignmentlength;
}




/* @func ensBasealignfeatureCalculateMemsize **********************************
**
** Get the memory size in bytes of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensBasealignfeatureCalculateMemsize(const EnsPBasealignfeature baf)
{
    size_t size = 0;

    if (!baf)
        return 0;

    size += sizeof (EnsOBasealignfeature);

    size += ensFeaturepairCalculateMemsize(baf->Featurepair);

    if (baf->Cigar)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(baf->Cigar);
    }

    return size;
}




/* @func ensBasealignfeatureCalculateUnitSource *******************************
**
** Get the alignment unit for the source part of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Alignment unit for the source part or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensBasealignfeatureCalculateUnitSource(const EnsPBasealignfeature baf)
{
    ajuint unit = 0U;

    if (!baf)
        return 0U;

    switch (baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            unit = 1U;

            break;

        case ensEBasealignfeatureTypeProtein:

            unit = 3U;

            break;

        default:

            ajWarn("ensBasealignfeatureCalculateUnitSource got an "
                   "Ensembl Base Align Feature with unexpected type %d.",
                   baf->Type);
    }

    return unit;
}




/* @func ensBasealignfeatureCalculateUnitTarget *******************************
**
** Get the alignment unit for the target part of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Alignment unit for the target part or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensBasealignfeatureCalculateUnitTarget(const EnsPBasealignfeature baf)
{
    ajuint unit = 0U;

    if (!baf)
        return 0U;

    switch (baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            unit = 1U;

            break;

        case ensEBasealignfeatureTypeProtein:

            unit = 1U;

            break;

        default:

            ajWarn("ensBasealignfeatureCalculateUnitTarget got an "
                   "Ensembl Base Align Feature with unexpected type %d.",
                   baf->Type);
    }

    return unit;
}




/* @section map ***************************************************************
**
** Functions for mapping Ensembl Base Align Feature objects between
** Ensembl Coordinate System objects.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Transfer Transfer an Ensembl Base Align Feature
** @nam3rule Transform Transform an Ensembl Base Align Feature
**
** @argrule * baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr]
** Ensembl Coordinate System version
**
** @valrule * [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @fcategory misc
******************************************************************************/




/* @func ensBasealignfeatureTransfer ******************************************
**
** Transfer an Ensembl Base Align Feature onto another Ensembl Slice.
**
** @cc Bio::EnsEMBL::Feature::transfer
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] slice [EnsPSlice] Ensembl Slice
** @see ensFeatureTransfer
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureTransfer(
    EnsPBasealignfeature baf,
    EnsPSlice slice)
{
    EnsPBasealignfeature newbaf = NULL;

    EnsPFeaturepair newfp = NULL;

    if (!baf)
        return NULL;

    if (!slice)
        return NULL;

    newfp = ensFeaturepairTransfer(baf->Featurepair, slice);

    if (!newfp)
        return NULL;

    newbaf = ensBasealignfeatureNewCpy(baf);

    ensBasealignfeatureSetFeaturepair(newbaf, newfp);

    ensFeaturepairDel(&newfp);

    return newbaf;
}




/* @func ensBasealignfeatureTransform *****************************************
**
** Transform an Ensembl Base Align Feature into another
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Feature::transform
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @see ensFeatureTransform
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureTransform(
    EnsPBasealignfeature baf,
    const AjPStr csname,
    const AjPStr csversion)
{
    EnsPFeaturepair nfp = NULL;

    EnsPBasealignfeature nbaf = NULL;

    if (!baf)
        return NULL;

    if (!csname)
        return NULL;

    if (!csversion)
        return NULL;

    nfp = ensFeaturepairTransform(baf->Featurepair, csname, csversion);

    if (!nfp)
        return NULL;

    nbaf = ensBasealignfeatureNewCpy(baf);

    ensBasealignfeatureSetFeaturepair(nbaf, nfp);

    ensFeaturepairDel(&nfp);

    return nbaf;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Trace Report Ensembl Base Align Feature members to debug file
**
** @argrule Trace baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensBasealignfeatureTrace *********************************************
**
** Trace an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBasealignfeatureTrace(const EnsPBasealignfeature baf, ajuint level)
{
    AjPStr indent = NULL;

    if (!baf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("ensBasealignfeatureTrace %p\n"
            "%S  Dnaalignfeatureadaptor %p\n"
            "%S  Proteinalignfeatureadaptor %p\n"
            "%S  Identifier %u\n"
            "%S  Featurepair %p\n"
            "%S  Cigar '%S'\n"
            "%S  FobjectGetFeaturepair %p\n"
            "%S  Type %d\n"
            "%S  Alignmentlength %u\n"
            "%S  Pairdnaalignfeatureidentifier %u\n"
            "%S  Use %u\n",
            indent, baf,
            indent, baf->Dnaalignfeatureadaptor,
            indent, baf->Proteinalignfeatureadaptor,
            indent, baf->Identifier,
            indent, baf->Featurepair,
            indent, baf->Cigar,
            indent, baf->FobjectGetFeaturepair,
            indent, baf->Type,
            indent, baf->Alignmentlength,
            indent, baf->Pairdnaalignfeatureidentifier,
            indent, baf->Use);

    ensFeaturepairTrace(baf->Featurepair, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an
** Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Fetch Fetch objects from an Ensembl Base Align Feature
** @nam4rule All Fetch all objects from an Ensembl Base Apign Feature
** @nam5rule Featurepairs Fetch all Ensembl Feature Pair objects
**
** @argrule * baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Featurepairs fps [AjPList]
** AJAX List of Ensembl Feature Pair objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensBasealignfeatureFetchAllFeaturepairs ******************************
**
** Fetch all (ungapped) Ensembl Feature Pair objects of an
** Ensembl Base Align Feature.
**
** The caller is responsible for deleting the Ensembl Feature Pair objects
** before deleting the AJAX List.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensBasealignfeatureFetchAllFeaturepairs(const EnsPBasealignfeature baf,
                                               AjPList fps)
{
    return basealignfeatureParseCigar(baf, fps);
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listBasealignfeatureCompareIdentifierAscending *****************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Base Align Feature address 1
** @param [r] item2 [const void*] Ensembl Base Align Feature address 2
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

static int listBasealignfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPBasealignfeature baf1 = *(EnsOBasealignfeature *const *) item1;
    EnsPBasealignfeature baf2 = *(EnsOBasealignfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listBasealignfeatureCompareIdentifierAscending"))
    {
        ajDebug("listBasealignfeatureCompareIdentifierAscending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (baf1 && (!baf2))
        return -1;

    if ((!baf1) && (!baf2))
        return 0;

    if ((!baf1) && baf2)
        return +1;

    if (baf1->Identifier < baf2->Identifier)
        return -1;

    if (baf1->Identifier > baf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listBasealignfeatureCompareSourceEndAscending ******************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by source Ensembl Feature end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Base Align Feature address 1
** @param [r] item2 [const void*] Ensembl Base Align Feature address 2
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

static int listBasealignfeatureCompareSourceEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPBasealignfeature baf1 = *(EnsOBasealignfeature *const *) item1;
    EnsPBasealignfeature baf2 = *(EnsOBasealignfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listBasealignfeatureCompareSourceEndAscending"))
    {
        ajDebug("listBasealignfeatureCompareSourceEndAscending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (baf1 && (!baf2))
        return -1;

    if ((!baf1) && (!baf2))
        return 0;

    if ((!baf1) && baf2)
        return +1;

    return ensFeaturepairCompareSourceEndAscending(
        baf1->Featurepair,
        baf2->Featurepair);
}




/* @funcstatic listBasealignfeatureCompareSourceEndDescending *****************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by source Ensembl Feature end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Base Align Feature address 1
** @param [r] item2 [const void*] Ensembl Base Align Feature address 2
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

static int listBasealignfeatureCompareSourceEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPBasealignfeature baf1 = *(EnsOBasealignfeature *const *) item1;
    EnsPBasealignfeature baf2 = *(EnsOBasealignfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listBasealignfeatureCompareSourceEndDescending"))
    {
        ajDebug("listBasealignfeatureCompareSourceEndDescending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (baf1 && (!baf2))
        return -1;

    if ((!baf1) && (!baf2))
        return 0;

    if ((!baf1) && baf2)
        return +1;

    return ensFeaturepairCompareSourceEndDescending(
        baf1->Featurepair,
        baf2->Featurepair);
}




/* @funcstatic listBasealignfeatureCompareSourceStartAscending ****************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by source Ensembl Feature start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Base Align Feature address 1
** @param [r] item2 [const void*] Ensembl Base Align Feature address 2
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

static int listBasealignfeatureCompareSourceStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPBasealignfeature baf1 = *(EnsOBasealignfeature *const *) item1;
    EnsPBasealignfeature baf2 = *(EnsOBasealignfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listBasealignfeatureCompareSourceStartAscending"))
    {
        ajDebug("listBasealignfeatureCompareSourceStartAscending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (baf1 && (!baf2))
        return -1;

    if ((!baf1) && (!baf2))
        return 0;

    if ((!baf1) && baf2)
        return +1;

    return ensFeaturepairCompareSourceStartAscending(
        baf1->Featurepair,
        baf2->Featurepair);
}




/* @funcstatic listBasealignfeatureCompareSourceStartDescending ***************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by source Ensembl Feature start coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Base Align Feature address 1
** @param [r] item2 [const void*] Ensembl Base Align Feature address 2
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

static int listBasealignfeatureCompareSourceStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPBasealignfeature baf1 = *(EnsOBasealignfeature *const *) item1;
    EnsPBasealignfeature baf2 = *(EnsOBasealignfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listBasealignfeatureCompareSourceStartDescending"))
    {
        ajDebug("listBasealignfeatureCompareSourceStartDescending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (baf1 && (!baf2))
        return -1;

    if ((!baf1) && (!baf2))
        return 0;

    if ((!baf1) && baf2)
        return +1;

    return ensFeaturepairCompareSourceStartDescending(
        baf1->Featurepair,
        baf2->Featurepair);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Basealignfeature Functions for manipulating AJAX List objects of
** Ensembl Base Align Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule Source     Sort by source Ensembl Feature member
** @nam5rule Target     Sort by target Ensembl feature member
** @nam6rule End        Sort by Ensembl Feature end member
** @nam6rule Start      Sort by Ensembl Feature start member
** @nam7rule Ascending  Sort in ascending order
** @nam7rule Descending Sort in descending order
**
** @argrule * bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListBasealignfeatureSortSourceEndAscending ************************
**
** Sort Ensembl Base Align Feature objects by the source Ensembl Feature
** end member in ascending order.
**
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListBasealignfeatureSortSourceEndAscending(AjPList bafs)
{
    if (!bafs)
        return ajFalse;

    ajListSortTwoThree(bafs,
                       &listBasealignfeatureCompareSourceEndAscending,
                       &listBasealignfeatureCompareSourceStartAscending,
                       &listBasealignfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListBasealignfeatureSortSourceEndDescending ***********************
**
** Sort Ensembl Base Align Feature objects by the source Ensembl Feature
** start member in descending order.
**
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListBasealignfeatureSortSourceEndDescending(AjPList bafs)
{
    if (!bafs)
        return ajFalse;

    ajListSortTwoThree(bafs,
                       &listBasealignfeatureCompareSourceEndDescending,
                       &listBasealignfeatureCompareSourceStartDescending,
                       &listBasealignfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListBasealignfeatureSortSourceStartAscending **********************
**
** Sort Ensembl Base Align Feature objects by the source Ensembl Feature
** start member in ascending order.
**
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListBasealignfeatureSortSourceStartAscending(AjPList bafs)
{
    if (!bafs)
        return ajFalse;

    ajListSortTwoThree(bafs,
                       &listBasealignfeatureCompareSourceStartAscending,
                       &listBasealignfeatureCompareSourceEndAscending,
                       &listBasealignfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListBasealignfeatureSortSourceStartDescending *********************
**
** Sort Ensembl Base Align Feature objects by the source Ensembl Feature
** start member in descending order.
**
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListBasealignfeatureSortSourceStartDescending(AjPList bafs)
{
    if (!bafs)
        return ajFalse;

    ajListSortTwoThree(bafs,
                       &listBasealignfeatureCompareSourceStartDescending,
                       &listBasealignfeatureCompareSourceEndDescending,
                       &listBasealignfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPDnaalignfeatureadaptor] Ensembl DNA Align Feature Adaptor
**
** @nam2rule Dnaalignfeatureadaptor Functions for manipulating
** Ensembl DNA Align Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseAlignFeatureAdaptor
** @cc CVS Revision: 1.36
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::DBSQL::DnaAlignFeatureAdaptor
** @cc CVS Revision: 1.77
** @cc CVS Tag: branch-ensembl-68
**
** NOTE: The Ensembl External Database Adaptor has an internal cache of all
** Ensembl External Database objects. Therefore, the same set of objects can be
** simply fetched by their SQL identifier and the "external_db" table does not
** need joining.
**
******************************************************************************/




/* @funcstatic dnaalignfeatureadaptorFetchAllbyStatement **********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Base Align Feature objects.
**
** The caller is responsible for deleting the Ensembl Base Align Feature
** objects before deleting the AJAX List.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool dnaalignfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs)
{
    double evalue = 0.0;
    double score  = 0.0;

    float identity    = 0.0F;
    float hitcoverage = 0.0F;

    ajuint identifier = 0U;
    ajuint srid       = 0U;
    ajuint srstart    = 0U;
    ajuint srend      = 0U;
    ajint  srstrand   = 0;
    ajuint hitstart   = 0U;
    ajuint hitend     = 0U;
    ajint  hitstrand  = 0;
    ajuint analysisid = 0U;
    ajuint edbid      = 0U;
    ajuint pairdafid  = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr hitname = NULL;
    AjPStr cigar   = NULL;
    AjPStr extra   = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDnaalignfeatureadaptor dafa = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    if (ajDebugTest("dnaalignfeatureadaptorFetchAllbyStatement"))
        ajDebug("dnaalignfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  bafs %p\n",
                ba,
                statement,
                am,
                slice,
                bafs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);
    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0U;
        srid        = 0U;
        srstart     = 0U;
        srend       = 0U;
        srstrand    = 0;
        hitstart    = 0U;
        hitend      = 0U;
        hitstrand   = 0;
        hitname     = ajStrNew();
        analysisid  = 0U;
        cigar       = ajStrNew();
        score       = 0.0;
        evalue      = 0.0;
        identity    = 0.0F;
        edbid       = 0U;
        hitcoverage = 0.0F;
        extra       = ajStrNew();
        pairdafid   = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &hitstart);
        ajSqlcolumnToUint(sqlr, &hitend);
        ajSqlcolumnToInt(sqlr, &hitstrand);
        ajSqlcolumnToStr(sqlr, &hitname);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &cigar);
        ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToDouble(sqlr, &evalue);
        ajSqlcolumnToFloat(sqlr, &identity);
        ajSqlcolumnToUint(sqlr, &edbid);
        ajSqlcolumnToFloat(sqlr, &hitcoverage);
        ajSqlcolumnToStr(sqlr, &extra);
        ajSqlcolumnToUint(sqlr, &pairdafid);

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &srcfeature);

        if (!srcfeature)
        {
            ajStrDel(&hitname);
            ajStrDel(&cigar);

            continue;
        }

        ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

        trgfeature = ensFeatureNewIniN((EnsPAnalysis) NULL,
                                       hitname,
                                       hitstart,
                                       hitend,
                                       hitstrand);

        fp = ensFeaturepairNewIni(srcfeature,
                                  trgfeature,
                                  edb,
                                  extra,
                                  (AjPStr) NULL, /* srcspecies */
                                  (AjPStr) NULL, /* trgspecies */
                                  0U, /* groupid */
                                  0U, /* levelid */
                                  evalue,
                                  score,
                                  0.0F, /* srccoverage */
                                  hitcoverage,
                                  identity);

        /*
        ** Finally, create a Base Align Feature object of type
        ** ensEBasealignfeatureTypeDNA.
        */

        baf = ensBasealignfeatureNewIniD(dafa,
                                         identifier,
                                         fp,
                                         cigar,
                                         pairdafid);

        ajListPushAppend(bafs, (void *) baf);

        ensFeaturepairDel(&fp);

        ensFeatureDel(&srcfeature);
        ensFeatureDel(&trgfeature);

        ensExternaldatabaseDel(&edb);

        ajStrDel(&hitname);
        ajStrDel(&cigar);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl DNA Align Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** DNA Align Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensDnaalignfeatureadaptorNew *****************************************
**
** Default constructor for an Ensembl DNA Align Feature Adaptor.
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
** @see ensRegistryGetDnaalignfeatureadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDnaalignfeatureadaptor ensDnaalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        dnaalignfeatureadaptorKTablenames,
        dnaalignfeatureadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &dnaalignfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensBasealignfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensBasealignfeatureDel,
        (size_t (*)(const void *)) &ensBasealignfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensBasealignfeatureGetFeature,
        "DNA Align Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl DNA Align Feature Adaptor object.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl DNA Align Feature Adaptor
**
** @argrule * Pdafa [EnsPDnaalignfeatureadaptor*]
** Ensembl DNA Align Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensDnaalignfeatureadaptorDel *****************************************
**
** Default destructor for an Ensembl DNA Align Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pdafa [EnsPDnaalignfeatureadaptor*]
** Ensembl DNA Align Feature Adaptor address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensDnaalignfeatureadaptorDel(
    EnsPDnaalignfeatureadaptor *Pdafa)
{
    ensFeatureadaptorDel(Pdafa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl DNA Align Feature Adaptor object.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Get Return Ensembl DNA Align Feature Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDnaalignfeatureadaptorGetBaseadaptor ******************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl DNA Align Feature Adaptor.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensDnaalignfeatureadaptorGetBaseadaptor(
    EnsPDnaalignfeatureadaptor dafa)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensDnaalignfeatureadaptorGetFeatureadaptor(dafa));
}




/* @func ensDnaalignfeatureadaptorGetDatabaseadaptor **************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl DNA Align Feature Adaptor.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDnaalignfeatureadaptorGetDatabaseadaptor(
    EnsPDnaalignfeatureadaptor dafa)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensDnaalignfeatureadaptorGetFeatureadaptor(dafa));
}




/* @func ensDnaalignfeatureadaptorGetFeatureadaptor ***************************
**
** Get the Ensembl Feature Adaptor member of an
** Ensembl DNA Align Feature Adaptor.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensDnaalignfeatureadaptorGetFeatureadaptor(
    EnsPDnaalignfeatureadaptor dafa)
{
    return dafa;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Base Align Feature objects (type dna)
** from an Ensembl SQL database.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Fetch
** Fetch Ensembl Base Align Feature object(s)
** @nam4rule All
** Fetch all Ensembl Base Align Feature objects
** @nam4rule Allby
** Fetch all Ensembl Base Align Feature objects matching a criterion
** @nam5rule Hitname Fetch all by hit name
** @nam5rule Hitunversioned Fetch all by unversioned hit name
** @nam5rule Slicecoverage Fetch all by an Ensembl Slice and sequence coverage
** @nam5rule Sliceexternaldatabasename
** Fetch all by an Ensembl Slice and Ensembl External Database name
** @nam5rule Sliceidentity Fetch all by an Ensembl Slice and sequence identity
** @nam4rule By
** Fetch one Ensembl Base Align Feature object matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @argrule AllbyHitname hitname [const AjPStr] Hit (target) sequence name
** @argrule AllbyHitname anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitname bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbyHitunversioned hitname [const AjPStr]
** Hit (target) sequence name
** @argrule AllbyHitunversioned anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitunversioned bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbySlicecoverage slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicecoverage coverage [float]
** Alignment target coverage threshold
** @argrule AllbySlicecoverage anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicecoverage bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbySliceexternaldatabasename slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceexternaldatabasename edbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbySliceexternaldatabasename anname [const AjPStr]
** Ensembl Analysis name
** @argrule AllbySliceexternaldatabasename bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbySliceidentity slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceidentity identity [float] Alignment identity threshold
** @argrule AllbySliceidentity anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySliceidentity bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pbaf [EnsPBasealignfeature*]
** Ensembl Base Align Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDnaalignfeatureadaptorFetchAllbyHitname ***************************
**
** Fetch Ensembl DNA Align Feature objects via a hit name.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbyHitname(
    EnsPDnaalignfeatureadaptor dafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txthitname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!dafa)
        return ajFalse;

    if (!hitname)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    ba = ensDnaalignfeatureadaptorGetBaseadaptor(dafa);

    ensBaseadaptorEscapeC(ba, &txthitname, hitname);

    constraint = ajFmtStr("dna_align_feature.hit_name = '%s'", txthitname);

    ajCharDel(&txthitname);

    /* Add the Ensembl Analysis name constraint. */

    if (anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(
            ensDnaalignfeatureadaptorGetFeatureadaptor(dafa),
            &constraint,
            anname);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDnaalignfeatureadaptorFetchAllbyHitunversioned ********************
**
** Fetch Ensembl DNA Align Feature objects via an unversioned hit name.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbyHitunversioned(
    EnsPDnaalignfeatureadaptor dafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr modhitname = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!dafa)
        return ajFalse;

    if (!hitname)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    ba = ensDnaalignfeatureadaptorGetBaseadaptor(dafa);

    ensBaseadaptorEscapeS(ba, &modhitname, hitname);

    /* Escape expensive SQL wilcard characters. */
    ajStrExchangeCC(&modhitname, "_", "\\_");

    constraint = ajFmtStr("dna_align_feature.hit_name LIKE '%S.%%'",
                          modhitname);

    ajStrDel(&modhitname);

    /* Add the Ensembl Analysis name constraint. */

    if (anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(
            ensDnaalignfeatureadaptorGetFeatureadaptor(dafa),
            &constraint,
            anname);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDnaalignfeatureadaptorFetchAllbySlicecoverage *********************
**
** Fetch Ensembl DNA Align Feature objects via an Ensembl Slice and
** an alignment target coverage.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] coverage [float] Alignment coverage threshold
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbySlicecoverage(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dafa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (coverage > 0.0F)
        constraint = ajFmtStr("dna_align_feature.hcoverage > %f", coverage);

    result = ensFeatureadaptorFetchAllbySlice(
        ensDnaalignfeatureadaptorGetFeatureadaptor(dafa),
        slice,
        constraint,
        anname,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDnaalignfeatureadaptorFetchAllbySliceexternaldatabasename *********
**
** Fetch Ensembl DNA Align Feature objects via an Ensembl Slice and
** an Ensembl External Database name.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] edbname [const AjPStr] Ensembl External Database name
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbySliceexternaldatabasename(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPFeatureadaptor fa = NULL;

    if (!dafa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!edbname)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    fa = ensDnaalignfeatureadaptorGetFeatureadaptor(dafa);

    ensFeatureadaptorEscapeC(fa, &txtname, edbname);

    constraint = ajFmtStr("external_db.db_name = %s", txtname);

    ajCharDel(&txtname);

    result = ensFeatureadaptorFetchAllbySlice(
        fa,
        slice,
        constraint,
        anname,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDnaalignfeatureadaptorFetchAllbySliceidentity *********************
**
** Fetch Ensembl DNA Align Feature objects via an Ensembl Slice and
** an alignment identity threshold.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] identity [float] Alignment identity threshold
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbySliceidentity(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!dafa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (identity > 0.0F)
        constraint = ajFmtStr("dna_align_feature.perc_ident > %f", identity);

    result = ensFeatureadaptorFetchAllbySlice(
        ensDnaalignfeatureadaptorGetFeatureadaptor(dafa),
        slice,
        constraint,
        anname,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensDnaalignfeatureadaptorFetchByIdentifier ***************************
**
** Fetch an Ensembl DNA Align Feature via its SQL database-internal identifier.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchByIdentifier(
    EnsPDnaalignfeatureadaptor dafa,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf)
{
    return ensBaseadaptorFetchByIdentifier(
        ensDnaalignfeatureadaptorGetBaseadaptor(dafa),
        identifier,
        (void **) Pbaf);
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl DNA Align Feature
** objects from an Ensembl SQL database.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl DNA Align Feature-releated object(s)
** @nam4rule All Retrieve all Ensembl DNA Align Feature-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
**
** @argrule * dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDnaalignfeatureadaptorRetrieveAllIdentifiers **********************
**
** Retrieve all SQL database-internal identifier objects of
** Ensembl DNA Align Feature objects.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor
** @param [u] identifiers [AjPList]
** AJAX List of AJAX unsigned integer objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorRetrieveAllIdentifiers(
    EnsPDnaalignfeatureadaptor dafa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!dafa)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("dna_align_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(
        ensDnaalignfeatureadaptorGetBaseadaptor(dafa),
        table,
        (AjPStr) NULL,
        identifiers);

    ajStrDel(&table);

    return result;
}




/* @datasection [EnsPProteinalignfeatureadaptor] Ensembl Protein Align Feature
** Adaptor
**
** @nam2rule Proteinalignfeatureadaptor Functions for manipulating
** Ensembl Protein Align Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseAlignFeatureAdaptor
** @cc CVS Revision: 1.36
** @cc CVS Tag: branch-ensembl-68
**
** @cc Bio::EnsEMBL::BBSQL::ProteinAlignFeatureAdaptor
** @cc CVS Revision: 1.52
** @cc CVS Tag: branch-ensembl-68
**
** NOTE: The Ensembl External Database Adaptor has an internal cache of all
** Ensembl External Database objects. Therefore, the same set of objects can be
** simply fetched by their SQL identifier and the "external_db" table does not
** need joining.
**
******************************************************************************/




/* @funcstatic proteinalignfeatureadaptorFetchAllbyStatement ******************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Protein Align Feature objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool proteinalignfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs)
{
    double evalue = 0.0;
    double score  = 0.0;

    float identity    = 0.0F;
    float hitcoverage = 0.0F;

    ajuint identifier = 0U;
    ajuint srid       = 0U;
    ajuint srstart    = 0U;
    ajuint srend      = 0U;
    ajint  srstrand   = 0;
    ajuint hitstart   = 0U;
    ajuint hitend     = 0U;
    ajuint analysisid = 0U;
    ajuint edbid      = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr hitname = NULL;
    AjPStr cigar   = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if (ajDebugTest("proteinalignfeatureadaptorFetchAllbyStatement"))
        ajDebug("proteinalignfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  bafs %p\n",
                ba,
                statement,
                am,
                slice,
                bafs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);
    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0U;
        srid        = 0U;
        srstart     = 0U;
        srend       = 0U;
        srstrand    = 0;
        hitstart    = 0U;
        hitend      = 0U;
        hitname     = ajStrNew();
        analysisid  = 0U;
        cigar       = ajStrNew();
        score       = 0.0;
        evalue      = 0.0;
        identity    = 0.0F;
        edbid       = 0U;
        hitcoverage = 0.0F;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &hitstart);
        ajSqlcolumnToUint(sqlr, &hitend);
        ajSqlcolumnToStr(sqlr, &hitname);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &cigar);
        ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToDouble(sqlr, &evalue);
        ajSqlcolumnToFloat(sqlr, &identity);
        ajSqlcolumnToUint(sqlr, &edbid);
        ajSqlcolumnToFloat(sqlr, &hitcoverage);

        ensBaseadaptorRetrieveFeature(ba,
                                      analysisid,
                                      srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      am,
                                      slice,
                                      &srcfeature);

        if (!srcfeature)
        {
            ajStrDel(&hitname);
            ajStrDel(&cigar);

            continue;
        }

        ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

        trgfeature = ensFeatureNewIniN((EnsPAnalysis) NULL,
                                       hitname,
                                       hitstart,
                                       hitend,
                                       1);

        fp = ensFeaturepairNewIni(srcfeature,
                                  trgfeature,
                                  edb,
                                  (AjPStr) NULL, /* extra data */
                                  (AjPStr) NULL, /* srcspecies */
                                  (AjPStr) NULL, /* trgspecies */
                                  0U, /* groupid */
                                  0U, /* levelid */
                                  evalue,
                                  score,
                                  0.0F, /* srccoverage */
                                  0.0F, /* trgcoverage */
                                  identity);

        /*
        ** Finally, create a Base Align Feature object of type
        ** ensEBasealignfeatureTypeProtein.
        */

        baf = ensBasealignfeatureNewIniP(pafa, identifier, fp, cigar);

        ajListPushAppend(bafs, (void *) baf);

        ensFeaturepairDel(&fp);

        ensFeatureDel(&srcfeature);
        ensFeatureDel(&trgfeature);

        ensExternaldatabaseDel(&edb);

        ajStrDel(&hitname);
        ajStrDel(&cigar);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Protein Align Feature Adaptor by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Protein Align Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensProteinalignfeatureadaptorNew *************************************
**
** Default constructor for an Ensembl Protein Align Feature Adaptor.
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
** @see ensRegistryGetProteinalignfeatureadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensProteinalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensFeatureadaptorNew(
        dba,
        proteinalignfeatureadaptorKTablenames,
        proteinalignfeatureadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &proteinalignfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensBasealignfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensBasealignfeatureDel,
        (size_t (*)(const void *)) &ensBasealignfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensBasealignfeatureGetFeature,
        "Protein Align Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Protein Align Feature Adaptor object.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Protein Align Feature Adaptor
**
** @argrule * Ppafa [EnsPProteinalignfeatureadaptor*]
** Ensembl Protein Align Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProteinalignfeatureadaptorDel *************************************
**
** Default destructor for an Ensembl Protein Align Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Ppafa [EnsPProteinalignfeatureadaptor*]
** Ensembl Protein Align Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensProteinalignfeatureadaptorDel(
    EnsPProteinalignfeatureadaptor *Ppafa)
{
    ensFeatureadaptorDel(Ppafa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Protein Align Feature Adaptor object.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Get Return Ensembl Protein Align Feature Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
** @nam4rule Featureadaptor Return the Ensembl Feature Adaptor
**
** @argrule * pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
** @valrule Featureadaptor [EnsPFeatureadaptor]
** Ensembl Feature Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensProteinalignfeatureadaptorGetBaseadaptor **************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Protein Align Feature Adaptor.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensProteinalignfeatureadaptorGetBaseadaptor(
    EnsPProteinalignfeatureadaptor pafa)
{
    return ensFeatureadaptorGetBaseadaptor(
        ensProteinalignfeatureadaptorGetFeatureadaptor(pafa));
}




/* @func ensProteinalignfeatureadaptorGetDatabaseadaptor **********************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Protein Align Feature Adaptor.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensProteinalignfeatureadaptorGetDatabaseadaptor(
    EnsPProteinalignfeatureadaptor pafa)
{
    return ensFeatureadaptorGetDatabaseadaptor(
        ensProteinalignfeatureadaptorGetFeatureadaptor(pafa));
}




/* @func ensProteinalignfeatureadaptorGetFeatureadaptor ***********************
**
** Get the Ensembl Feature Adaptor member of an
** Ensembl Protein Align Feature Adaptor.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
**
** @return [EnsPFeatureadaptor] Ensembl Feature Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPFeatureadaptor ensProteinalignfeatureadaptorGetFeatureadaptor(
    EnsPProteinalignfeatureadaptor pafa)
{
    return pafa;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Base Align Feature objects (type protein)
** from an Ensembl SQL database.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Fetch
** Fetch Ensembl Base Align Feature object(s)
** @nam4rule All
** Fetch all Ensembl Base Align Feature objects
** @nam4rule Allby
** Fetch all Ensembl Base Align Feature objects matching a criterion
** @nam5rule Hitname Fetch all by hit name
** @nam5rule Hitunversioned Fetch all by unversioned hit name
** @nam5rule Slicecoverage Fetch all by an Ensembl Slice and sequence coverage
** @nam5rule Sliceexternaldatabasename
** Fetch all by an Ensembl Slice and Ensembl External Database name
** @nam5rule Sliceidentity Fetch all by an Ensembl Slice and sequence identity
** @nam4rule By
** Fetch one Ensembl Base Align Feature object matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @argrule AllbyHitname hitname [const AjPStr] Hit (target) sequence name
** @argrule AllbyHitname anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitname bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbyHitunversioned hitname [const AjPStr]
** Hit (target) sequence name
** @argrule AllbyHitunversioned anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitunversioned bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbySlicecoverage slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicecoverage coverage [float]
** Alignment target coverage threshold
** @argrule AllbySlicecoverage anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicecoverage bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule AllbySliceexternaldatabasename slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceexternaldatabasename edbname [const AjPStr]
** Ensembl External Database name
** @argrule AllbySliceexternaldatabasename anname [const AjPStr]
** Ensembl Analysis name
** @argrule AllbySliceexternaldatabasename bafs [AjPList] AJAX List of
** Ensembl Base Align Feature objects
** @argrule AllbySliceidentity slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceidentity identity [float] Alignment identity threshold
** @argrule AllbySliceidentity anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySliceidentity bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pbaf [EnsPBasealignfeature*]
** Ensembl Base Align Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensProteinalignfeatureadaptorFetchAllbyHitname ***********************
**
** Fetch Ensembl Base Align Feature objects (type protein) via a hit name.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbyHitname(
    EnsPProteinalignfeatureadaptor pafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txthitname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!pafa)
        return ajFalse;

    if (!hitname)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    ba = ensProteinalignfeatureadaptorGetBaseadaptor(pafa);

    ensBaseadaptorEscapeC(ba, &txthitname, hitname);

    constraint = ajFmtStr("protein_align_feature.hit_name = '%s'", txthitname);

    ajCharDel(&txthitname);

    /* Add the Ensembl Analysis name constraint. */

    if (anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(
            ensProteinalignfeatureadaptorGetFeatureadaptor(pafa),
            &constraint,
            anname);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensProteinalignfeatureadaptorFetchAllbyHitunversioned ****************
**
** Fetch all Ensembl Base Align Feature objects (type protein) via an
** unversioned hit name.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbyHitunversioned(
    EnsPProteinalignfeatureadaptor pafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;
    AjPStr modhitname = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!pafa)
        return ajFalse;

    if (!hitname)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    ba = ensProteinalignfeatureadaptorGetBaseadaptor(pafa);

    ensBaseadaptorEscapeS(ba, &modhitname, hitname);

    /* Escape expensive SQL wilcard characters. */
    ajStrExchangeCC(&modhitname, "_", "\\_");

    constraint = ajFmtStr(
        "protein_align_feature.hit_name LIKE '%S.%%'",
        modhitname);

    ajStrDel(&modhitname);

    /* Add the Ensembl Analysis name constraint. */

    if (anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(
            ensProteinalignfeatureadaptorGetFeatureadaptor(pafa),
            &constraint,
            anname);

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensProteinalignfeatureadaptorFetchAllbySlicecoverage *****************
**
** Fetch Ensembl Base Align Feature objects (type protein) via an
** Ensembl Slice and an alignment target coverage.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] coverage [float] Alignment target coverage threshold
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbySlicecoverage(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!pafa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (coverage > 0.0F)
        constraint = ajFmtStr(
            "protein_align_feature.hcoverage > %f",
            coverage);

    result = ensFeatureadaptorFetchAllbySlice(
        ensProteinalignfeatureadaptorGetFeatureadaptor(pafa),
        slice,
        constraint,
        anname,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensProteinalignfeatureadaptorFetchAllbySliceexternaldatabasename *****
**
** Fetch Ensembl Base Align Feature objects (type protein) via an
** Ensembl Slice and an Ensembl External Database name.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] edbname [const AjPStr] Ensembl External Database name
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbySliceexternaldatabasename(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPFeatureadaptor fa = NULL;

    if (!pafa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!edbname)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    fa = ensProteinalignfeatureadaptorGetFeatureadaptor(pafa);

    ensFeatureadaptorEscapeC(fa, &txtname, edbname);

    constraint = ajFmtStr("external_db.db_name = %s", txtname);

    ajCharDel(&txtname);

    result = ensFeatureadaptorFetchAllbySlice(
        fa,
        slice,
        constraint,
        anname,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensProteinalignfeatureadaptorFetchAllbySliceidentity *****************
**
** Fetch Ensembl Base Align Feature objects (type protein) via an
** Ensembl Slice and an alignment identity threshold.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] identity [float] Alignment identity threshold
** @param [rN] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbySliceidentity(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!pafa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (identity > 0.0F)
        constraint = ajFmtStr(
            "protein_align_feature.perc_ident > %f",
            identity);

    result = ensFeatureadaptorFetchAllbySlice(
        ensProteinalignfeatureadaptorGetFeatureadaptor(pafa),
        slice,
        constraint,
        anname,
        bafs);

    ajStrDel(&constraint);

    return result;
}




/* @func ensProteinalignfeatureadaptorFetchByIdentifier ***********************
**
** Fetch an Ensembl Base Align Feature (type protein) via its
** SQL database-internal identifier.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchByIdentifier(
    EnsPProteinalignfeatureadaptor pafa,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf)
{
    return ensBaseadaptorFetchByIdentifier(
        ensProteinalignfeatureadaptorGetBaseadaptor(pafa),
        identifier,
        (void **) Pbaf);
}




/* @section accessory object retrieval ****************************************
**
** Functions for retrieving objects releated to Ensembl Base Align Feature
** objects (type protein) from an Ensembl SQL database.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Base Align Feature-releated object(s)
** @nam4rule All Retrieve all Ensembl Base Align Feature-releated objects
** @nam5rule Identifiers Retrieve all SQL database-internal identifier objects
**
** @argrule * pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensProteinalignfeatureadaptorRetrieveAllIdentifiers ******************
**
** Retrieve all SQL database-internal identifier objects of
** Ensembl Base Align Feature objects.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integer objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorRetrieveAllIdentifiers(
    EnsPProteinalignfeatureadaptor pafa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    if (!pafa)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    table = ajStrNewC("protein_align_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(
        ensProteinalignfeatureadaptorGetBaseadaptor(pafa),
        table,
        (AjPStr) NULL,
        identifiers);

    ajStrDel(&table);

    return result;
}




/* @datasection [EnsPSupportingfeatureadaptor]
** Ensembl Supporting Feature Adaptor
**
** @nam2rule Supportingfeatureadaptor Functions for manipulating
** Ensembl Supporting Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SupportingFeatureAdaptor
** @cc CVS Revision: 1.22
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Supporting Feature Adaptor object.
**
** @fdata [EnsPSupportingfeatureadaptor]
**
** @nam3rule Get Return Ensembl Supporting Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * sfa [EnsPSupportingfeatureadaptor]
** Ensembl Supporting Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSupportingfeatureadaptorGetDatabaseadaptor ************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Supporting Feature Adaptor.
**
** @param [u] sfa [EnsPSupportingfeatureadaptor]
** Ensembl Supporting Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSupportingfeatureadaptorGetDatabaseadaptor(
    EnsPSupportingfeatureadaptor sfa)
{
    return sfa;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Base Align Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPSupportingfeatureadaptor]
**
** @nam3rule Fetch      Fetch Ensembl Base Align Feature object(s)
** @nam4rule FetchAll   Fetch all Ensembl Base Align Feature objects
** @nam4rule FetchAllby Fetch all Ensembl Base Align Feature objects
**                      matching a criterion
** @nam5rule Exon       Fetch all Ensembl Base Align Feature objects
**                      by an Ensembl Exon
** @nam5rule Transcript Fetch all Ensembl Base Align Feature objects
**                      by an Ensembl Transcript
** @nam4rule FetchBy    Fetch one Ensembl Base Align Feature object
**                      matching a criterion
**
** @argrule * sfa [EnsPSupportingfeatureadaptor]
** Ensembl Supporting Feature Adaptor
** @argrule Exon exon [EnsPExon] Ensembl Exon
** @argrule Transcript transcript [EnsPTranscript] Ensembl Transcript
** @argrule Allby bafs [AjPList]
** AJAX List of Ensembl Base Align Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSupportingfeatureadaptorFetchAllbyExon ****************************
**
** Fetch all supporting Ensembl Base Align Feature objects via an Ensembl Exon.
**
** @param [u] sfa [EnsPSupportingfeatureadaptor]
** Ensembl Supporting Feature Adaptor
** @param [u] exon [EnsPExon] Ensembl Exon
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSupportingfeatureadaptorFetchAllbyExon(
    EnsPSupportingfeatureadaptor sfa,
    EnsPExon exon,
    AjPList bafs)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr type      = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature efeature = NULL;
    EnsPFeature nfeature = NULL;
    EnsPFeature ofeature = NULL;

    EnsPSlice eslice = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDnaalignfeatureadaptor dafa = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if (!sfa)
        return ajFalse;

    if (!exon)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (ensExonGetIdentifier(exon) == 0)
    {
        ajDebug("ensSupportingfeatureadaptorFetchAllbyExon cannot get "
                "supporting Ensembl Base Align Feature objects for an "
                "Ensembl Exon without an identifier.\n");

        return ajFalse;
    }

    dba = ensSupportingfeatureadaptorGetDatabaseadaptor(sfa);

    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);
    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    efeature = ensExonGetFeature(exon);

    eslice = ensFeatureGetSlice(efeature);

    statement = ajFmtStr(
        "SELECT "
        "supporting_feature.feature_type, "
        "supporting_feature.feature_id "
        "FROM "
        "supporting_feature "
        "WHERE "
        "supporting_feature.exon_id = %u",
        ensExonGetIdentifier(exon));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        type       = ajStrNew();
        identifier = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &identifier);

        if (ajStrMatchC(type, "dna_align_feature"))
            ensDnaalignfeatureadaptorFetchByIdentifier(
                dafa,
                identifier,
                &baf);
        else if (ajStrMatchC(type, "protein_align_feature"))
            ensProteinalignfeatureadaptorFetchByIdentifier(
                pafa,
                identifier,
                &baf);
        else
            ajWarn("ensSupportingfeatureadaptorFetchAllbyExon got "
                   "unexpected value in supporting_feature.feature_type "
                   "'%S'.\n", type);

        if (baf)
        {
            ofeature = ensFeaturepairGetSourceFeature(baf->Featurepair);

            nfeature = ensFeatureTransfer(ofeature, eslice);

            ensFeaturepairSetSourceFeature(baf->Featurepair, nfeature);

            ensFeatureDel(&nfeature);

            ajListPushAppend(bafs, (void *) baf);
        }
        else
            ajDebug("ensSupportingfeatureadaptorFetchAllbyExon could not "
                    "retrieve Supporting feature of type '%S' and "
                    "identifier %u from database.\n", type, identifier);

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensSupportingfeatureadaptorFetchAllbyTranscript **********************
**
** Fetch Ensembl Supporting Feature objects via an Ensembl Transcript.
**
** @param [u] sfa [EnsPSupportingfeatureadaptor]
** Ensembl Supporting Feature Adaptor
** @param [u] transcript [EnsPTranscript] Ensembl Transcript
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSupportingfeatureadaptorFetchAllbyTranscript(
    EnsPSupportingfeatureadaptor sfa,
    EnsPTranscript transcript,
    AjPList bafs)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr type      = NULL;

    EnsPFeature tfeature = NULL;
    EnsPFeature nfeature = NULL;
    EnsPFeature ofeature = NULL;

    EnsPSlice tslice = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDnaalignfeatureadaptor dafa = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if (!sfa)
        return ajFalse;

    if (!transcript)
        return ajFalse;

    if (!bafs)
        return ajFalse;

    if (!ensTranscriptGetIdentifier(transcript))
    {
        ajDebug("ensSupportingfeatureadaptorFetchAllbyTranscript cannot get "
                "Ensembl Supporting Feature objects for an Ensembl Transcript "
                "without an identifier.\n");

        return ajFalse;
    }

    dba = ensSupportingfeatureadaptorGetDatabaseadaptor(sfa);

    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);
    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    tfeature = ensTranscriptGetFeature(transcript);

    tslice = ensFeatureGetSlice(tfeature);

    statement = ajFmtStr(
        "SELECT "
        "transcript_supporting_feature.feature_type, "
        "transcript_supporting_feature.feature_id "
        "FROM "
        "transcript_supporting_feature "
        "WHERE "
        "transcript_supporting_feature.transcript_id = %u",
        ensTranscriptGetIdentifier(transcript));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        type       = ajStrNew();
        identifier = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &identifier);

        if (ajStrMatchC(type, "dna_align_feature"))
            ensDnaalignfeatureadaptorFetchByIdentifier(
                dafa,
                identifier,
                &baf);
        else if (ajStrMatchC(type, "protein_align_feature"))
            ensProteinalignfeatureadaptorFetchByIdentifier(
                pafa,
                identifier,
                &baf);
        else
            ajWarn("ensSupportingfeatureadaptorFetchAllbyTranscript got "
                   "unexpected value in "
                   "transcript_supporting_feature.feature_type '%S'.\n", type);

        if (baf)
        {
            ofeature = ensFeaturepairGetSourceFeature(baf->Featurepair);

            nfeature = ensFeatureTransfer(ofeature, tslice);

            ensFeaturepairSetSourceFeature(baf->Featurepair, nfeature);

            ensFeatureDel(&nfeature);

            ajListPushAppend(bafs, (void *) baf);
        }
        else
            ajDebug("ensSupportingfeatureadaptorFetchAllbyTranscript could "
                    "not fetch Supporting feature of type '%S' and "
                    "identifier %u from database.\n", type, identifier);

        ajStrDel(&type);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

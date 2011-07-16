/* @source Ensembl Alignment functions
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

#include "ensalign.h"
#include "ensexternaldatabase.h"




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

/* @conststatic dnaalignfeatureadaptorTables **********************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL table names
**
******************************************************************************/

static const char* const dnaalignfeatureadaptorTables[] =
{
    "dna_align_feature",
    (const char*) NULL
};




/* @conststatic dnaalignfeatureadaptorColumns *********************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const dnaalignfeatureadaptorColumns[] =
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
    (const char*) NULL
};




/* @conststatic proteinalignfeatureadaptorTables ******************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL table names
**
******************************************************************************/

static const char* const proteinalignfeatureadaptorTables[] =
{
    "protein_align_feature",
    (const char*) NULL
};




/* @conststatic proteinalignfeatureadaptorColumns *****************************
**
** Array of Ensembl DNA Align Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const proteinalignfeatureadaptorColumns[] =
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
    "protein_align_feature.external_data",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool basealignfeatureParseFeaturepairs(EnsPBasealignfeature baf,
                                                AjPList objects);

static AjBool basealignfeatureParseCigar(const EnsPBasealignfeature baf,
                                         AjPList fps);

static int listBasealignfeatureCompareSourceStartAscending(const void* P1,
                                                           const void* P2);

static int listBasealignfeatureCompareSourceStartDescending(const void* P1,
                                                            const void* P2);

static void* basealignfeatureadaptorCacheReference(void* value);

static void basealignfeatureadaptorCacheDelete(void** value);

static size_t basealignfeatureadaptorCacheSize(const void* value);

static EnsPFeature basealignfeatureadaptorGetFeature(const void* value);

static AjBool dnaalignfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs);

static AjBool proteinalignfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.62
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::DnaDnaAlignFeature
** @cc CVS Revision: 1.25
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::DnaPepAlignFeature
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::PepDnaAlignFeature
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-62
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
** @param [u] objects [AjPList] AJAX List of Ensembl Objects based on
**                              Ensembl Feature Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** FIXME: The Perl API assigns to $start1 $start2 and $ori but never uses
** these values.
******************************************************************************/

static AjBool basealignfeatureParseFeaturepairs(EnsPBasealignfeature baf,
                                                AjPList objects)
{
    void* Pobject = NULL;

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

    ajuint srcunit = 0;
    ajuint trgunit = 0;

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

    EnsPFeaturepair cfp = NULL; /* Current Featuire Pair */
    EnsPFeaturepair ffp = NULL; /* First Feature Pair */
    EnsPFeaturepair lfp = NULL; /* Last Feature Pair */

    if(!baf)
        return ajFalse;

    if(!objects)
        return ajFalse;

    if(!ajListGetLength(objects))
        return ajTrue;

    srcunit = ensBasealignfeatureCalculateUnitSource(baf);
    trgunit = ensBasealignfeatureCalculateUnitTarget(baf);

    if(baf->Cigar)
        ajStrAssignClear(&baf->Cigar);
    else
        baf->Cigar = ajStrNew();

    /* Extract Ensembl Feature Pair objects from Ensembl Objects. */

    iter = ajListIterNew(objects);

    while(!ajListIterDone(iter))
    {
        Pobject = ajListIterGet(iter);

        /* FIXME: This function pointer is not defined! */
        ajListPushAppend(fps, (void*) baf->GetFeaturepair(Pobject));
    }

    ajListIterDel(&iter);

    /*
    ** Sort the AJAX List of Ensembl Feature Pair objects on their source
    ** Ensembl Feature start element in ascending order on the positive strand
    ** and descending order on the negative strand.
    */

    ajListPeekFirst(fps, (void**) &ffp);

    fsf = ensFeaturepairGetSourceFeature(ffp);

    if(ensFeatureGetStrand(fsf) >= 0)
        ensListFeaturepairSortSourceStartAscending(fps);
    else
        ensListFeaturepairSortSourceStartDescending(fps);

    /*
    ** Use strandedness info of source and target to make sure both sets of
    ** start and end coordinates are oriented the right way around.
    */

    ajListPeekFirst(fps, (void**) &ffp);
    ajListPeekLast(fps, (void**) &lfp);

    fsf = ensFeaturepairGetSourceFeature(ffp);
    ftf = ensFeaturepairGetTargetFeature(ffp);
    lsf = ensFeaturepairGetSourceFeature(lfp);
    ltf = ensFeaturepairGetTargetFeature(lfp);

    if(ensFeatureGetStrand(fsf) >= 0)
    {
        srcstart = ensFeatureGetStart(fsf);
        srcend   = ensFeatureGetEnd(lsf);
    }
    else
    {
        srcstart = ensFeatureGetStart(lsf);
        srcend   = ensFeatureGetEnd(fsf);
    }

    if(ensFeatureGetStrand(ftf) >= 0)
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

    while(!ajListIterDone(iter))
    {
        cfp = (EnsPFeaturepair) ajListIterGet(iter);

        csf = ensFeaturepairGetSourceFeature(cfp);
        ctf = ensFeaturepairGetTargetFeature(cfp);

        /* Sanity checks */

        if(ensFeatureGetSlice(csf) &&
           ensFeatureGetSlice(fsf) &&
           (!ensSliceMatch(ensFeatureGetSlice(csf),
                           ensFeatureGetSlice(fsf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source Feature Slice elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeatureGetSequencename(csf) &&
           ensFeatureGetSequencename(fsf) &&
           (!ajStrMatchS(ensFeatureGetSequencename(csf),
                         ensFeatureGetSequencename(fsf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source Feature sequence name elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeatureGetStrand(csf) != ensFeatureGetStrand(fsf))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source Feature strand elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeatureGetSlice(ctf) &&
           ensFeatureGetSlice(ftf) &&
           (!ensSliceMatch(ensFeatureGetSlice(ctf),
                           ensFeatureGetSlice(ftf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target Feature Slice elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeatureGetSequencename(ctf) &&
           ensFeatureGetSequencename(ftf) &&
           (!ajStrMatchCaseS(ensFeatureGetSequencename(ctf),
                             ensFeatureGetSequencename(ftf))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target Feature sequence name elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeatureGetStrand(ctf) != ensFeatureGetStrand(ftf))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target Feature strand elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeaturepairGetSourceSpecies(cfp) &&
           ensFeaturepairGetSourceSpecies(ffp) &&
           (!ajStrMatchS(ensFeaturepairGetSourceSpecies(cfp),
                         ensFeaturepairGetSourceSpecies(ffp))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "source species elements between the "
                    "current and first Ensembl Feature Pair.\n");

        if(ensFeaturepairGetTargetSpecies(cfp) &&
           ensFeaturepairGetTargetSpecies(ffp) &&
           (!ajStrMatchS(ensFeaturepairGetTargetSpecies(cfp),
                         ensFeaturepairGetTargetSpecies(ffp))))
            ajFatal("basealignfeatureParseFeaturepairs found inconsistent "
                    "target species elements between the "
                    "current and first Ensembl Feature Pair.\n");

        /*
        ** NOTE: The score, percent identity and P-values are not tested for
        ** equality as this is unreliable for float and double types.
        **
        ** More sanity checking
        */

        if(srcpos)
        {
            if(ensFeatureGetStrand(csf) >= 0)
            {
                if(ensFeatureGetStart(csf) < srcpos)
                    ajFatal("Inconsistent coordinates in "
                            "Ensembl Feature Pair List (forward strand).\n"
                            "Start (%d) of current Feature Pair should be "
                            "greater than previous Feature Pair end (%d).\n",
                            ensFeatureGetStart(csf), srcpos);
            }
            else
            {
                if(ensFeatureGetEnd(csf) > srcpos)
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

        if(srcunit > trgunit)
        {
            /*
            ** I am going to make the assumption here that this situation
            ** will only occur with DnaPepAlignFeatures, this may not be true
            */

            if((srclength / srcunit) != (trglength * trgunit))
                ajFatal("Feature Pair lengths not comparable "
                        "Lengths: %d %d "
                        "Ratios: %d %d.\n",
                        srclength, trglength,
                        srcunit, trgunit);
        }
        else
        {
            if((srclength * trgunit) != (trglength * srcunit))
                ajFatal("Feature Pair lengths not comparable "
                        "Lengths: %d %d "
                        "Ratios: %d %d.\n",
                        srclength, trglength,
                        srcunit, trgunit);
        }

        /*
        ** Check to see if there is an I type (insertion) gap:
        ** If there is a space between the end of the last source sequence
        ** alignment and the start of this one, then this is an insertion
        */

        insertion = ajFalse;

        if(ensFeatureGetStrand(fsf) >= 0)
        {
            if(srcpos && (ensFeatureGetStart(csf) > (srcpos + 1)))
            {
                /* there is an insertion */

                insertion = ajTrue;

                srcgap = ensFeatureGetStart(csf) - srcpos - 1;

                /* no need for a number if gap length is 1 */

                if(srcgap == 1)
                    ajStrAppendK(&baf->Cigar, 'I');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dI", srcgap);
            }

            /* shift our position in the source sequence alignment */

            srcpos = ensFeatureGetEnd(csf);
        }
        else
        {
            if(srcpos && ((ensFeatureGetEnd(csf) + 1) < srcpos))
            {
                /* there is an insertion */

                insertion = ajTrue;

                srcgap = srcpos - ensFeatureGetEnd(csf) - 1;

                /* no need for a number if gap length is 1 */

                if(srcgap == 1)
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

        if(ensFeatureGetStrand(ctf) >= 0)
        {
            if(trgpos && (ensFeatureGetStart(ctf) > (trgpos + 1)))
            {
                /* there is a deletion */

                srcgap = ensFeatureGetStart(ctf) - trgpos - 1;

                trggap = (ajint) (srcgap * srcunit / trgunit + 0.5);

                /* no need for a number if gap length is 1 */

                if(trggap == 1)
                    ajStrAppendK(&baf->Cigar, 'D');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dD", trggap);

                /* sanity check, should not be an insertion and deletion */

                if(insertion)
                {
                    if(!warning)
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
            if(trgpos && ((ensFeatureGetEnd(ctf) + 1) < trgpos))
            {
                /* there is a deletion */

                srcgap = trgpos - ensFeatureGetEnd(ctf) - 1;

                trggap = (ajint) (srcgap * srcunit / trgunit + 0.5);

                /* no need for a number if gap length is 1 */

                if(trggap == 1)
                    ajStrAppendK(&baf->Cigar, 'D');
                else
                    ajFmtPrintAppS(&baf->Cigar, "%dD", trggap);

                /* sanity check,  should not be an insertion and deletion */

                if(insertion)
                {
                    if(!warning)
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

        if(match == 1)
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
** Convert an Ensembl Base Align Feature CIGAR line element into an
** AJAX List of Ensembl Feature Pair objects.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fps [AjPList] AJAX List of Ensembl Feature Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    ajuint srcunit = 0;
    ajuint trgunit = 0;

    const AjPStr token = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    if(!baf)
        return ajFalse;

    if((!baf->Cigar) && (!ajStrGetLen(baf->Cigar)))
        ajFatal("basealignfeatureParseCigar no CIGAR string defined!\n");

    if(!baf->Featurepair)
        ajFatal("basealignfeatureParseCigar no Feature Pair defined in the "
                "Base Align Feature!\n");

    if(!baf->Featurepair->SourceFeature)
        ajFatal("basealignfeatureParseCigar no Feature defined in the "
                "Feature Pair of the Base Align Feature!\n");

    srcunit = ensBasealignfeatureCalculateUnitSource(baf);
    trgunit = ensBasealignfeatureCalculateUnitTarget(baf);

    if(baf->Featurepair->SourceFeature->Strand > 0)
        fpsrcstart = baf->Featurepair->SourceFeature->Start;
    else
        fpsrcstart = baf->Featurepair->SourceFeature->End;

    if(baf->Featurepair->TargetFeature->Strand > 0)
        fptrgstart = baf->Featurepair->TargetFeature->Start;
    else
        fptrgstart = baf->Featurepair->TargetFeature->End;

    /* Construct ungapped blocks as Feature Pair objects for each MATCH. */

    while((token = ajStrParseC(baf->Cigar, "MDI")))
    {
        ajStrToInt(token, &tlength);

        tlength = (tlength) ? tlength : 1;

        if((srcunit == 1) && (trgunit == 3))
            mlength = tlength * 3;
        else if((srcunit == 3) && (trgunit == 1))
            mlength = tlength / 3;
        else if((srcunit == 1) && (trgunit == 1))
            mlength = tlength;
        else
            ajFatal("basealignfeatureParseCigar got "
                    "Base Align Feature source unit %d "
                    "Base Align Feature target unit %d, "
                    "but currently only 1 or 3 are allowed.\n",
                    srcunit,
                    trgunit);

        if(ajStrMatchC(token, "M"))
        {
            /* MATCH */

            if(baf->Featurepair->SourceFeature->Strand > 0)
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

            if(baf->Featurepair->TargetFeature->Strand > 0)
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

            srcfeature = ensFeatureNewCpy(baf->Featurepair->SourceFeature);

            ensFeatureSetStart(srcfeature, srcstart);
            ensFeatureSetEnd(srcfeature, srcend);

            /* Clone the target Feature and set the new coordinates. */

            trgfeature = ensFeatureNewCpy(baf->Featurepair->TargetFeature);

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

            ajListPushAppend(fps, (void*) fp);
        }
        else if(ajStrMatchC(token, "I"))
        {
            /* INSERT */

            if(baf->Featurepair->SourceFeature->Strand > 0)
                fpsrcstart += tlength;
            else
                fpsrcstart -= tlength;
        }

        else if(ajStrMatchC(token, "D"))
        {
            /* DELETION */

            if(baf->Featurepair->TargetFeature->Strand > 0)
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
** @argrule Fps type [EnsEBasealignfeatureType] Ensembl Base Align Feature type
** @argrule Fps pair [ajuint] Pair DNA Align Feature identifier
** @argrule IniD dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                                 Feature Adaptor
** @argrule IniD identifier [ajuint] SQL database-internal identifier
** @argrule IniD fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule IniD cigar [AjPStr] CIGAR line
** @argrule IniD pair [ajuint] Pair DNA Align Feature identifier
** @argrule IniP pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                     Feature Adaptor
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
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewCpy(
    const EnsPBasealignfeature baf)
{
    EnsPBasealignfeature pthis = NULL;

    if(!baf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier                 = baf->Identifier;
    pthis->Dnaalignfeatureadaptor     = baf->Dnaalignfeatureadaptor;
    pthis->Proteinalignfeatureadaptor = baf->Proteinalignfeatureadaptor;

    pthis->Featurepair = ensFeaturepairNewRef(baf->Featurepair);

    if(baf->Cigar)
        pthis->Cigar = ajStrNewRef(baf->Cigar);

    pthis->Type = baf->Type;

    pthis->Alignmentlength = baf->Alignmentlength;

    pthis->PairDnaalignfeatureIdentifier = baf->PairDnaalignfeatureIdentifier;

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
** @param [u] type [EnsEBasealignfeatureType] Ensembl Base Align Feature Type
** enumeration
** @param [r] pair [ajuint] Pair DNA Align Feature identifier
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
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

    if(!fp)
        return NULL;

    if((type < ensEBasealignfeatureTypeDNA) ||
       (type > ensEBasealignfeatureTypeProtein))
    {
        ajDebug("ensBasealignfeatureNewFps got illegal type (%d).\n",
                type);

        return NULL;
    }

    AJNEW0(baf);

    baf->Use = 1;

    baf->Featurepair = ensFeaturepairNewRef(fp);

    if(cigar && ajStrGetLen(cigar) && fps && ajListGetLength(fps))
    {
        ajDebug("ensBasealignfeatureNewFps requires a CIGAR line "
                "or an AJAX List of Ensembl Feature Pair objects, "
                "not both.\n");

        AJFREE(baf);

        return NULL;
    }
    else if(cigar && ajStrGetLen(cigar))
        baf->Cigar = ajStrNewRef(cigar);
    else if(fps && ajListGetLength(fps))
        basealignfeatureParseFeaturepairs(baf, fps);
    else
        ajDebug("ensBasealignfeatureNewFps requires either a CIGAR line or "
                "an AJAX List of Ensembl Feature Pair objects.\n");

    baf->Type = type;

    baf->PairDnaalignfeatureIdentifier = pair;

    return baf;
}




/* @func ensBasealignfeatureNewIniD *******************************************
**
** Constructor for an Ensembl Base Align Feature of type "DNA"
** with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
** @param [r] pair [ajuint] Pair DNA Align Feature identifier
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
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

    if(!fp)
        return NULL;

    if(!cigar)
        return NULL;

    AJNEW0(baf);

    baf->Use = 1;

    baf->Identifier = identifier;

    baf->Dnaalignfeatureadaptor = dafa;

    baf->Featurepair = ensFeaturepairNewRef(fp);

    if(cigar)
        baf->Cigar = ajStrNewRef(cigar);

    baf->Type = ensEBasealignfeatureTypeDNA;

    baf->PairDnaalignfeatureIdentifier = pair;

    return baf;
}




/* @func ensBasealignfeatureNewIniP *******************************************
**
** Constructor for an Ensembl Base Align Feature of type "Protein"
** with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::BaseAlignFeature
** @param [u] cigar [AjPStr] CIGAR line
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewIniP(
    EnsPProteinalignfeatureadaptor pafa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar)
{
    EnsPBasealignfeature baf = NULL;

    if(!fp)
        return NULL;

    if(!cigar)
        return NULL;

    AJNEW0(baf);

    baf->Use = 1;

    baf->Identifier = identifier;

    baf->Proteinalignfeatureadaptor = pafa;

    baf->Featurepair = ensFeaturepairNewRef(fp);

    if(cigar)
        baf->Cigar = ajStrNewRef(cigar);

    baf->Type = ensEBasealignfeatureTypeProtein;

    baf->PairDnaalignfeatureIdentifier = 0;

    return baf;
}




/* @func ensBasealignfeatureNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPBasealignfeature] Ensembl Base Align Feature
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureNewRef(EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;

    baf->Use++;

    return baf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Base Align Feature object
**
** @argrule * Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature
**                                         object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensBasealignfeatureDel ***********************************************
**
** Default destructor for an Ensembl Base Align Feature.
**
** @param [d] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature
**                                         object address
**
** @return [void]
** @@
******************************************************************************/

void ensBasealignfeatureDel(EnsPBasealignfeature* Pbaf)
{
    EnsPBasealignfeature pthis = NULL;

    if(!Pbaf)
        return;

    if(!*Pbaf)
        return;

    pthis = *Pbaf;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pbaf = NULL;

        return;
    }

    ensFeaturepairDel(&pthis->Featurepair);

    AJFREE(pthis);

    *Pbaf = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Get Return Base Align Feature attribute(s)
** @nam4rule Cigar Return the CIGAR line
** @nam4rule Dnaalignfeatureadaptor
**           Return the Ensembl DNA Align Feature Adaptor
** @nam4rule Featurepair Return the Ensembl Feature Pair
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Proteinalignfeatureadaptor
**           Return the Ensembl Protein Align Feature Adaptor
** @nam4rule Type Return the type
** @nam4rule PairDnaalignfeatureIdentifier Return the pair DNA Align Feature
**                                         identifier
**
** @argrule * baf [const EnsPBasealignfeature] Base Align Feature
**
** @valrule Cigar [AjPStr] CIGAR line or NULL
** @valrule Dnaalignfeatureadaptor [EnsPDnaalignfeatureadaptor]
** Ensembl DNA Align Feature Adaptor or NULL
** @valrule Featurepair [EnsPFeaturepair] Ensembl Feature Pair or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
** Ensembl Protein Align Feature Adaptor or NULL
** @valrule Type [EnsEBasealignfeatureType] Type or
** ensEBasealignfeatureTypeNULL
** @valrule PairDnaalignfeatureIdentifier [ajuint] Pair DNA Align Feature
**                                                 identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensBasealignfeatureGetCigar ******************************************
**
** Get the CIGAR line element of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [AjPStr] CIGAR line or NULL
** @@
******************************************************************************/

AjPStr ensBasealignfeatureGetCigar(
    const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;

    return baf->Cigar;
}




/* @func ensBasealignfeatureGetDnaalignfeatureadaptor *************************
**
** Get the DNA Align Feature Adaptor element of an Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPDnaalignfeatureadaptor] Ensembl DNA Align Feature Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPDnaalignfeatureadaptor ensBasealignfeatureGetDnaalignfeatureadaptor(
    const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;

    return baf->Dnaalignfeatureadaptor;
}




/* @func ensBasealignfeatureGetFeaturepair ************************************
**
** Get the Ensembl Feature Pair element of an Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
** @@
******************************************************************************/

EnsPFeaturepair ensBasealignfeatureGetFeaturepair(
    const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;

    return baf->Featurepair;
}




/* @func ensBasealignfeatureGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Base Align Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensBasealignfeatureGetIdentifier(const EnsPBasealignfeature baf)
{
    if(!baf)
        return 0;

    return baf->Identifier;
}




/* @func ensBasealignfeatureGetProteinalignfeatureadaptor *********************
**
** Get the Protein Align Feature Adaptor element of an
** Ensembl Base Align Feature.
**
** Sets the alignment length if it not yet defined.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                          Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensBasealignfeatureGetProteinalignfeatureadaptor(
    const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;

    return baf->Proteinalignfeatureadaptor;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Set Set one element of a Base Align Feature
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
** Set the Ensembl Feature Pair element of an Ensembl Base Align Feature.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensBasealignfeatureSetFeaturepair(EnsPBasealignfeature baf,
                                         EnsPFeaturepair fp)
{
    if(ajDebugTest("ensBasealignfeatureSetFeaturepair"))
    {
        ajDebug("ensBasealignfeatureSetFeaturepair\n"
                "  baf %p\n"
                "  fp %p\n",
                baf,
                fp);

        ensBasealignfeatureTrace(baf, 1);

        ensFeaturepairTrace(fp, 1);
    }

    if(!baf)
        return ajFalse;

    if(!fp)
        return ajFalse;

    /* Replace the current Feature Pair. */

    if(baf->Featurepair)
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
** @nam4rule Feature Get the source Ensembl Feature
**
** @argrule * baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @valrule Feature [EnsPFeature] Ensembl Feature
**
** @fcategory use
******************************************************************************/




/* @func ensBasealignfeatureGetFeature ****************************************
**
** Get the Ensembl Feature element of an Ensembl Feature Pair element of an
** Ensembl Base Align Feature.
**
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensBasealignfeatureGetFeature(const EnsPBasealignfeature baf)
{
    if(!baf)
        return NULL;

    return ensFeaturepairGetSourceFeature(baf->Featurepair);
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Base Align Feature object.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Calculate       Calculate Ensembl Base Align Feature values
** @nam4rule Adaptor         Calculate the adaptor
** @nam4rule Alignmentlength Calculate the alignment length
** @nam4rule Memsize         Calculate the memory size in bytes
** @nam4rule Unit            Calculate an alignment unit
** @nam5rule Source          Calculate the source unit
** @nam5rule Target          Calculate the target unit
**
** @argrule Adaptor baf [const EnsPBasealignfeature] Ensembl Base
**                                                   Align Feature
** @argrule Alignmentlength baf [EnsPBasealignfeature] Ensembl Base
**                                                     Align Feature
** @argrule Memsize baf [const EnsPBasealignfeature] Ensembl Base
**                                                   Align Feature
** @argrule UnitSource baf [const EnsPBasealignfeature] Ensembl Base
**                                                      Align Feature
** @argrule UnitTarget baf [const EnsPBasealignfeature] Ensembl Base
**                                                      Align Feature
**
** @valrule Adaptor [void*] Adaptor
** @valrule Alignmentlength [ajuint] Alignment length
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule UnitSource [ajuint] Alignment unit for the source part
** @valrule UnitTarget [ajuint] Alignment unit for the target part
**
** @fcategory misc
******************************************************************************/




/* @func ensBasealignfeatureCalculateAdaptor **********************************
**
** Calculate the Adaptor element of an Ensembl Base Align Feature.
** This will return an EnsPDnaalignfeatureadaptor for an Ensembl Base Align
** Feature of type ensEBasealignfeatureTypeDNA and an
** EnsPProteinalignfeatureadaptor for a feature of type
** ensEBasealignfeatureTypeProtein.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] baf [const EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [void*] Ensembl DNA or Protein Align Feature Adaptor
** @@
******************************************************************************/

void* ensBasealignfeatureCalculateAdaptor(const EnsPBasealignfeature baf)
{
    void* Padaptor = NULL;

    if(!baf)
        return NULL;

    switch(baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            Padaptor = (void*) baf->Dnaalignfeatureadaptor;

            break;

        case ensEBasealignfeatureTypeProtein:

            Padaptor = (void*) baf->Proteinalignfeatureadaptor;

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
** Calculate the alignment length element of an Ensembl Base Align Feature.
**
** @param [u] baf [EnsPBasealignfeature] Ensembl Base Align Feature
**
** @return [ajuint] Alignment length
** @@
** FIXME: The alignment length could already be calculated when the CIGAR line
** is parsed the first time in basealignfeatureParseCigar.
******************************************************************************/

ajuint ensBasealignfeatureCalculateAlignmentlength(EnsPBasealignfeature baf)
{
    ajint tlength = 0;

    const AjPStr token = NULL;

    if(!baf)
        return 0;

    if(!baf->Alignmentlength && baf->Cigar)
    {

        while((token = ajStrParseC(baf->Cigar, "MDI")))
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
** @@
******************************************************************************/

size_t ensBasealignfeatureCalculateMemsize(const EnsPBasealignfeature baf)
{
    size_t size = 0;

    if(!baf)
        return 0;

    size += sizeof (EnsOBasealignfeature);

    size += ensFeaturepairCalculateMemsize(baf->Featurepair);

    if(baf->Cigar)
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
** @return [ajuint] Alignment unit for the source part
** @@
******************************************************************************/

ajuint ensBasealignfeatureCalculateUnitSource(const EnsPBasealignfeature baf)
{
    ajuint unit = 0;

    if(!baf)
        return 0;

    switch(baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            unit = 1;

            break;

        case ensEBasealignfeatureTypeProtein:

            unit = 3;

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
** @return [ajuint] Alignment unit for the target part
** @@
******************************************************************************/

ajuint ensBasealignfeatureCalculateUnitTarget(const EnsPBasealignfeature baf)
{
    ajuint unit = 0;

    if(!baf)
        return 0;

    switch(baf->Type)
    {
        case ensEBasealignfeatureTypeDNA:

            unit = 1;

            break;

        case ensEBasealignfeatureTypeProtein:

            unit = 1;

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
** Ensembl Coordinate Systems.
**
** @fdata [EnsPBasealignfeature]
**
** @nam3rule Transfer Transfer an Ensembl Base Align Feature
** @nam3rule Transform Transform an Ensembl Base Align Feature
**
** @argrule * baf [EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Transfer slice [EnsPSlice] Ensembl Slice
** @argrule Transform csname [const AjPStr] Ensembl Coordinate System name
** @argrule Transform csversion [const AjPStr] Ensembl Coordinate System
**                                             version
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
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureTransfer(
    EnsPBasealignfeature baf,
    EnsPSlice slice)
{
    EnsPBasealignfeature newbaf = NULL;

    EnsPFeaturepair newfp = NULL;

    if(!baf)
        return NULL;

    if(!slice)
        return NULL;

    newfp = ensFeaturepairTransfer(baf->Featurepair, slice);

    if(!newfp)
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
** @@
******************************************************************************/

EnsPBasealignfeature ensBasealignfeatureTransform(
    EnsPBasealignfeature baf,
    const AjPStr csname,
    const AjPStr csversion)
{
    EnsPFeaturepair nfp = NULL;

    EnsPBasealignfeature nbaf = NULL;

    if(!baf)
        return NULL;

    if(!csname)
        return NULL;

    if(!csversion)
        return NULL;

    nfp = ensFeaturepairTransform(baf->Featurepair, csname, csversion);

    if(!nfp)
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
** @nam3rule Trace Report Ensembl Base Align Feature elements to debug file
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
** @@
******************************************************************************/

AjBool ensBasealignfeatureTrace(const EnsPBasealignfeature baf, ajuint level)
{
    AjPStr indent = NULL;

    if(!baf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("ensBasealignfeatureTrace %p\n"
            "%S  Dnaalignfeatureadaptor %p\n"
            "%S  Proteinalignfeatureadaptor %p\n"
            "%S  Identifier %u\n"
            "%S  Featurepair %p\n"
            "%S  Cigar '%S'\n"
            "%S  (*GetFeaturepair) %p\n"
            "%S  Type %d\n"
            "%S  Alignmentlength %u\n"
            "%S  PairDnaalignfeatureIdentifier %u\n"
            "%S  Use %u\n",
            indent, baf,
            indent, baf->Dnaalignfeatureadaptor,
            indent, baf->Proteinalignfeatureadaptor,
            indent, baf->Identifier,
            indent, baf->Featurepair,
            indent, baf->Cigar,
            indent, baf->GetFeaturepair,
            indent, baf->Type,
            indent, baf->Alignmentlength,
            indent, baf->PairDnaalignfeatureIdentifier,
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
** @nam3rule Fetch Fetch object from an Ensembl Base Align Feature
** @nam4rule All
** @nam5rule Featurepairs Fetch all Ensembl Feature Pair objects
**
** @argrule * baf [const EnsPBasealignfeature] Ensembl Base Align Feature
** @argrule Featurepairs fps [AjPList] AJAX List of
**                                     Ensembl Feature Pair objects
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
** @@
******************************************************************************/

AjBool ensBasealignfeatureFetchAllFeaturepairs(const EnsPBasealignfeature baf,
                                               AjPList fps)
{
    if(!baf)
        return ajFalse;

    if(!fps)
        return ajFalse;

    return basealignfeatureParseCigar(baf, fps);
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
** @nam3rule Basealignfeature Functions for manipulating AJAX List objects of
** Ensembl Base Align Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Source Sort by source Ensembl Feature element
** @nam6rule Start Sort by Ensembl Feature start element
** @nam7rule Ascending  Sort in ascending order
** @nam7rule Descending Sort in descending order
**
** @argrule Ascending bafs [AjPList]  AJAX List of
**                                    Ensembl Base Align Feature objects
** @argrule Descending bafs [AjPList] AJAX List of
**                                    Ensembl Base Align Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listBasealignfeatureCompareSourceStartAscending ****************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by source Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Base Align Feature address 1
** @param [r] P2 [const void*] Ensembl Base Align Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listBasealignfeatureCompareSourceStartAscending(const void* P1,
                                                           const void* P2)
{
    EnsPBasealignfeature baf1 = NULL;
    EnsPBasealignfeature baf2 = NULL;

    baf1 = *(EnsPBasealignfeature const*) P1;
    baf2 = *(EnsPBasealignfeature const*) P2;

    if(ajDebugTest("listBasealignfeatureCompareSourceStartAscending"))
    {
        ajDebug("listBasealignfeatureCompareSourceStartAscending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(baf1 && (!baf2))
        return -1;

    if((!baf1) && (!baf2))
        return 0;

    if((!baf1) && baf2)
        return +1;

    return ensFeaturepairCompareSourceStartAscending(
        baf1->Featurepair,
        baf2->Featurepair);
}




/* @func ensListBasealignfeatureSortSourceStartAscending **********************
**
** Sort Ensembl Base Align Feature objects by the source Ensembl Feature
** in start element in ascending order.
**
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListBasealignfeatureSortSourceStartAscending(AjPList bafs)
{
    if(!bafs)
        return ajFalse;

    ajListSort(bafs, listBasealignfeatureCompareSourceStartAscending);

    return ajTrue;
}




/* @funcstatic listBasealignfeatureCompareSourceStartDescending ***************
**
** AJAX List of Ensembl Base Align Feature objects comparison function to
** sort by source Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Base Align Feature address 1
** @param [r] P2 [const void*] Ensembl Base Align Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listBasealignfeatureCompareSourceStartDescending(const void* P1,
                                                            const void* P2)
{
    EnsPBasealignfeature baf1 = NULL;
    EnsPBasealignfeature baf2 = NULL;

    baf1 = *(EnsPBasealignfeature const*) P1;
    baf2 = *(EnsPBasealignfeature const*) P2;

    if(ajDebugTest("listBasealignfeatureCompareSourceStartDescending"))
    {
        ajDebug("listBasealignfeatureCompareSourceStartDescending\n"
                "  baf1 %p\n"
                "  baf2 %p\n",
                baf1,
                baf2);

        ensBasealignfeatureTrace(baf1, 1);
        ensBasealignfeatureTrace(baf2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(baf1 && (!baf2))
        return -1;

    if((!baf1) && (!baf2))
        return 0;

    if((!baf1) && baf2)
        return +1;

    return ensFeaturepairCompareSourceStartDescending(
        baf1->Featurepair,
        baf2->Featurepair);
}




/* @func ensListBasealignfeatureSortSourceStartDescending *********************
**
** Sort Ensembl Base Align Feature objects by the source Ensembl Feature
** start element in descending order.
**
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListBasealignfeatureSortSourceStartDescending(AjPList bafs)
{
    if(!bafs)
        return ajFalse;

    ajListSort(bafs, listBasealignfeatureCompareSourceStartDescending);

    return ajTrue;
}




/* @funcstatic basealignfeatureadaptorCacheReference **************************
**
** Wrapper function to reference an Ensembl Base Align Feature
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Base Align Feature
**
** @return [void*] Ensembl Base Align Feature or NULL
** @@
******************************************************************************/

static void* basealignfeatureadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensBasealignfeatureNewRef((EnsPBasealignfeature) value);
}




/* @funcstatic basealignfeatureadaptorCacheDelete *****************************
**
** Wrapper function to delete an Ensembl Base Align Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Base Align Feature address
**
** @return [void]
** @@
******************************************************************************/

static void basealignfeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensBasealignfeatureDel((EnsPBasealignfeature*) value);

    return;
}




/* @funcstatic basealignfeatureadaptorCacheSize *******************************
**
** Wrapper function to determine the memory size of an
** Ensembl Base Align Feature from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Base Align Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t basealignfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensBasealignfeatureCalculateMemsize(
        (const EnsPBasealignfeature) value);
}




/* @funcstatic basealignfeatureadaptorGetFeature ******************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Base Align Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Base Align Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature basealignfeatureadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensBasealignfeatureGetFeature((const EnsPBasealignfeature) value);
}




/* @datasection [EnsPDnaalignfeatureadaptor] Ensembl DNA Align Feature Adaptor
**
** @nam2rule Dnaalignfeatureadaptor Functions for manipulating
** Ensembl DNA Align Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseAlignFeatureAdaptor
** @cc CVS Revision: 1.34
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::DBSQL::DnaAlignFeatureAdaptor
** @cc CVS Revision: 1.75
** @cc CVS Tag: branch-ensembl-62
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool dnaalignfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs)
{
    double evalue = 0.0;
    double score  = 0.0;

    float identity    = 0.0F;
    float hitcoverage = 0.0F;

    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint edbid      = 0;
    ajuint pair       = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint hitstart = 0;
    ajuint hitend   = 0;
    ajint hitstrand = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr hitname = NULL;
    AjPStr cigar   = NULL;
    AjPStr extra   = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPDnaalignfeatureadaptor dafa = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("dnaalignfeatureadaptorFetchAllbyStatement"))
        ajDebug("dnaalignfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  bafs %p\n",
                dba,
                statement,
                am,
                slice,
                bafs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    sa = (slice) ? ensSliceGetAdaptor(slice) : ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        srid        = 0;
        srstart     = 0;
        srend       = 0;
        srstrand    = 0;
        hitstart    = 0;
        hitend      = 0;
        hitstrand   = 0;
        hitname     = ajStrNew();
        analysisid  = 0;
        cigar       = ajStrNew();
        score       = 0.0;
        evalue      = 0.0;
        identity    = 0.0F;
        edbid       = 0;
        hitcoverage = 0.0F;
        extra       = ajStrNew();
        pair        = 0;

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
        ajSqlcolumnToUint(sqlr, &pair);

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Feature objects, the range
        ** needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("dnaalignfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("dnaalignfeatureadaptorFetchAllbyStatement got a "
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
        ** if an Ensembl Mapper was provided.
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

                ajStrDel(&hitname);

                ajStrDel(&cigar);

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
                ajFatal("dnaalignfeatureadaptorFetchAllbyStatement got a "
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
            ** Throw away Feature objects off the end of the requested Slice
            ** or on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Next feature but destroy first! */

                ajStrDel(&hitname);

                ajStrDel(&cigar);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

        srcfeature = ensFeatureNewIniS(analysis,
                                       srslice,
                                       slstart,
                                       slend,
                                       slstrand);

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
                                  0, /* groupid */
                                  0, /* levelid */
                                  evalue,
                                  score,
                                  0, /* srccoverage */
                                  hitcoverage,
                                  identity);

        /*
        ** Finally, create a Base Align Feature object of type
        ** ensEBasealignfeatureTypeDNA.
        */

        baf = ensBasealignfeatureNewIniD(dafa, identifier, fp, cigar, pair);

        ajListPushAppend(bafs, (void*) baf);

        ensFeaturepairDel(&fp);

        ensFeatureDel(&srcfeature);

        ensFeatureDel(&trgfeature);

        ensExternaldatabaseDel(&edb);

        ensAnalysisDel(&analysis);

        ajStrDel(&hitname);

        ajStrDel(&cigar);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

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
** @valrule * [EnsPDnaalignfeatureadaptor] Ensembl DNA Align Feature Adaptor
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
** @return [EnsPDnaalignfeatureadaptor] Ensembl DNA Align Feature Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPDnaalignfeatureadaptor ensDnaalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        dnaalignfeatureadaptorTables,
        dnaalignfeatureadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        dnaalignfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        basealignfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        basealignfeatureadaptorCacheDelete,
        basealignfeatureadaptorCacheSize,
        basealignfeatureadaptorGetFeature,
        "DNA Align Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl DNA Align Feature Adaptor object.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl DNA Align Feature Adaptor object
**
** @argrule * Pdafa [EnsPDnaalignfeatureadaptor*]
** Ensembl DNA Align Feature Adaptor object address
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
** Ensembl DNA Align Feature Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensDnaalignfeatureadaptorDel(EnsPDnaalignfeatureadaptor* Pdafa)
{
    if(!Pdafa)
        return;

    if(!*Pdafa)
        return;

    ensFeatureadaptorDel(Pdafa);

    *Pdafa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl DNA Align Feature Adaptor object.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Get Return Ensembl DNA Align Feature Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor or
** NULL
**
** @fcategory use
******************************************************************************/




/* @func ensDnaalignfeatureadaptorGetDatabaseadaptor **************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl DNA Align Feature Adaptor.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensDnaalignfeatureadaptorGetDatabaseadaptor(
    EnsPDnaalignfeatureadaptor dafa)
{
    if(!dafa)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(dafa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Base Align Feature objects (type dna)
** from an Ensembl SQL database.
**
** @fdata [EnsPDnaalignfeatureadaptor]
**
** @nam3rule Fetch   Fetch Ensembl Base Align Feature object(s)
** @nam4rule All     Fetch all Ensembl Base Align Feature objects
** @nam4rule Allby   Fetch all Ensembl Base Align Feature objects
**                   matching a criterion
** @nam5rule Hitname Fetch all by hit name
** @nam5rule Hitunversioned Fetch all by unversioned hit name
** @nam5rule Slicecoverage Fetch all by an Ensembl Slice and sequence coverage
** @nam5rule Sliceexternaldatabasename Fetch all by an Ensembl Slice and
**                                     Ensembl External Database name
** @nam5rule Sliceidentity Fetch all by an Ensembl Slice and sequence identity
** @nam4rule By    Fetch one Ensembl Base Align Feature object
**                 matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @argrule AllbyHitname hitname [const AjPStr] Hit (target) sequence name
** @argrule AllbyHitname anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitname bafs [AjPList] AJAX List of Ensembl Base Align
**                                      Feature objects
** @argrule AllbyHitunversioned hitname [const AjPStr] Hit (target)
**                                                     sequence name
** @argrule AllbyHitunversioned anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitunversioned bafs [AjPList] AJAX List of Ensembl Base
**                                             Align Feature objects
** @argrule AllbySlicecoverage slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicecoverage coverage [float] Alignment target
**                                              coverage threshold
** @argrule AllbySlicecoverage anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicecoverage bafs [AjPList]  AJAX List of Ensembl Base
**                                             Align Feature objects
** @argrule AllbySliceexternaldatabasename slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceexternaldatabasename edbname [const AjPStr]
**                                         Ensembl External Database name
** @argrule AllbySliceexternaldatabasename anname [const AjPStr]
**                                         Ensembl Analysis name
** @argrule AllbySliceexternaldatabasename bafs [AjPList] AJAX List of
**                                          Ensembl Base Align Feature objects
** @argrule AllbySliceidentity slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceidentity identity [float] Alignment identity threshold
** @argrule AllbySliceidentity anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySliceidentity bafs [AjPList] AJAX List of Ensembl Base
**                                            Align Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pbaf [EnsPBasealignfeature*] Ensembl Base Align
**                                                    Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDnaalignfeatureadaptorFetchAllbyHitname ***************************
**
** Fetch Ensembl DNA Align Feature objects via a hit name.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbyHitname(
    EnsPDnaalignfeatureadaptor dafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char* txthitname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!dafa)
        return ajFalse;

    if(!hitname)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dafa);

    ensBaseadaptorEscapeC(ba, &txthitname, hitname);

    constraint = ajFmtStr("dna_align_feature.hit_name = '%s'", txthitname);

    ajCharDel(&txthitname);

    /* Add the Ensembl Analysis name constraint. */

    if(anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(dafa,
                                                      &constraint,
                                                      anname);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
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
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbyHitunversioned(
    EnsPDnaalignfeatureadaptor dafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char* txthitname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!dafa)
        return ajFalse;

    if(!hitname)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dafa);

    ensBaseadaptorEscapeC(ba, &txthitname, hitname);

    constraint = ajFmtStr("dna_align_feature.hit_name LIKE '%s.%'",
                          txthitname);

    ajCharDel(&txthitname);

    /* Add the Ensembl Analysis name constraint. */

    if(anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(dafa,
                                                      &constraint,
                                                      anname);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
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
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] coverage [float] Alignment coverage threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!dafa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(coverage > 0.0F)
        constraint = ajFmtStr("dna_align_feature.hcoverage > %f", coverage);

    result = ensFeatureadaptorFetchAllbySlice(dafa,
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
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] edbname [const AjPStr] Ensembl External Database name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchAllbySliceexternaldatabasename(
    EnsPDnaalignfeatureadaptor dafa,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs)
{
    char* txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!dafa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!edbname)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    ensFeatureadaptorEscapeC(dafa, &txtname, edbname);

    constraint = ajFmtStr("external_db.db_name = %s", txtname);

    ajCharDel(&txtname);

    result = ensFeatureadaptorFetchAllbySlice(dafa,
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
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] identity [float] Alignment identity threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!dafa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(identity > 0.0F)
        constraint = ajFmtStr("dna_align_feature.perc_ident > %f", identity);

    result = ensFeatureadaptorFetchAllbySlice(dafa,
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
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorFetchByIdentifier(
    EnsPDnaalignfeatureadaptor dafa,
    ajuint identifier,
    EnsPBasealignfeature* Pbaf)
{
    EnsPBaseadaptor ba = NULL;

    if(!dafa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pbaf)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dafa);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void**) Pbaf);
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
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
**
** @argrule * dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensDnaalignfeatureadaptorRetrieveAllIdentifiers **********************
**
** Retrieve all SQL database-internal identifiers of
** Ensembl DNA Align Feature objects.
**
** @param [u] dafa [EnsPDnaalignfeatureadaptor] Ensembl DNA Align
**                                              Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned
**                                  integer identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensDnaalignfeatureadaptorRetrieveAllIdentifiers(
    EnsPDnaalignfeatureadaptor dafa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!dafa)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(dafa);

    table = ajStrNewC("dna_align_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
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
** @cc CVS Revision: 1.34
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::BBSQL::ProteinAlignFeatureAdaptor
** @cc CVS Revision: 1.60
** @cc CVS Tag: branch-ensembl-62
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool proteinalignfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList bafs)
{
    double evalue = 0.0;
    double score  = 0.0;

    float identity    = 0.0F;
    float hitcoverage = 0.0F;

    ajuint identifier = 0;
    ajuint analysisid = 0;
    ajuint edbid      = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint hitstart = 0;
    ajuint hitend   = 0;
    ajint srstrand  = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr hitname = NULL;
    AjPStr cigar   = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPBasealignfeature baf = NULL;

    EnsPExternaldatabase edb         = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    EnsPMapperresult mr = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("proteinalignfeatureadaptorFetchAllbyStatement"))
        ajDebug("proteinalignfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  bafs %p\n",
                dba,
                statement,
                am,
                slice,
                bafs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    edba = ensRegistryGetExternaldatabaseadaptor(dba);

    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        srid        = 0;
        srstart     = 0;
        srend       = 0;
        srstrand    = 0;
        hitstart    = 0;
        hitend      = 0;
        hitname     = ajStrNew();
        analysisid  = 0;
        cigar       = ajStrNew();
        score       = 0.0;
        evalue      = 0.0;
        identity    = 0.0F;
        edbid       = 0;
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

        /*
        ** Since the Ensembl SQL schema defines Sequence Region start and end
        ** coordinates as unsigned integers for all Feature objects, the range
        ** needs checking.
        */

        if(srstart <= INT_MAX)
            slstart = (ajint) srstart;
        else
            ajFatal("proteinalignfeatureadaptorFetchAllbyStatement got a "
                    "Sequence Region start coordinate (%u) outside the "
                    "maximum integer limit (%d).",
                    srstart, INT_MAX);

        if(srend <= INT_MAX)
            slend = (ajint) srend;
        else
            ajFatal("proteinalignfeatureadaptorFetchAllbyStatement got a "
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
        ** if an Ensembl Mapper was provided.
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

                ajStrDel(&hitname);

                ajStrDel(&cigar);

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
                ajFatal("proteinalignfeatureadaptorFetchAllbyStatement got a "
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
            ** Throw away Feature objects off the end of the requested Slice
            ** or on any other than the requested Slice.
            */

            if((slend < 1) ||
               (slstart > sllength) ||
               (srid != ensSliceGetSeqregionIdentifier(slice)))
            {
                /* Next feature but destroy first! */

                ajStrDel(&hitname);

                ajStrDel(&cigar);

                ensSliceDel(&srslice);

                ensAssemblymapperDel(&am);

                continue;
            }

            /* Delete the Sequence Region Slice and set the requested Slice. */

            ensSliceDel(&srslice);

            srslice = ensSliceNewRef(slice);
        }

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

        srcfeature = ensFeatureNewIniS(analysis,
                                       srslice,
                                       slstart,
                                       slend,
                                       slstrand);

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
                                  0, /* groupid */
                                  0, /* levelid */
                                  evalue,
                                  score,
                                  0, /* srccoverage */
                                  0, /* trgcoverage */
                                  identity);

        /*
        ** Finally, create a Base Align Feature object of type
        ** ensEBasealignfeatureTypeProtein.
        */

        baf = ensBasealignfeatureNewIniP(pafa, identifier, fp, cigar);

        ajListPushAppend(bafs, (void*) baf);

        ensFeaturepairDel(&fp);

        ensFeatureDel(&srcfeature);
        ensFeatureDel(&trgfeature);

        ensExternaldatabaseDel(&edb);

        ensAnalysisDel(&analysis);

        ajStrDel(&hitname);

        ajStrDel(&cigar);

        ensSliceDel(&srslice);

        ensAssemblymapperDel(&am);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

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
** @valrule * [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                             Feature Adaptor
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
** @return [EnsPProteinalignfeatureadaptor] Ensembl Protein Align Feature
**                                          Adaptor or NULL
** @@
******************************************************************************/

EnsPProteinalignfeatureadaptor ensProteinalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        proteinalignfeatureadaptorTables,
        proteinalignfeatureadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        proteinalignfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        basealignfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        basealignfeatureadaptorCacheDelete,
        basealignfeatureadaptorCacheSize,
        basealignfeatureadaptorGetFeature,
        "Protein Align Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Protein Align Feature Adaptor object.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Protein Align Feature Adaptor object
**
** @argrule * Ppafa [EnsPProteinalignfeatureadaptor*]
** Ensembl Protein Align Feature Adaptor object address
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
** Ensembl Protein Align Feature Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensProteinalignfeatureadaptorDel(EnsPProteinalignfeatureadaptor* Ppafa)
{
    if(!Ppafa)
        return;

    if(!*Ppafa)
        return;

    ensFeatureadaptorDel(Ppafa);

    *Ppafa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Protein Align Feature Adaptor object.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Get Return Ensembl Protein Align Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensProteinalignfeatureadaptorGetDatabaseadaptor **********************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Protein Align Feature Adaptor.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensProteinalignfeatureadaptorGetDatabaseadaptor(
    EnsPProteinalignfeatureadaptor pafa)
{
    if(!pafa)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(pafa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Base Align Feature objects (type protein)
** from an Ensembl SQL database.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Base Align Feature object(s)
** @nam4rule All   Fetch all Ensembl Base Align Feature objects
** @nam4rule Allby Fetch all Ensembl Base Align Feature objects
**                      matching a criterion
** @nam5rule Hitname Fetch all by hit name
** @nam5rule Hitunversioned Fetch all by unversioned hit name
** @nam5rule Slicecoverage Fetch all by an Ensembl Slice and sequence coverage
** @nam5rule Sliceexternaldatabasename Fetch all by an Ensembl Slice and
**                                     Ensembl External Database name
** @nam5rule Sliceidentity Fetch all by an Ensembl Slice and sequence identity
** @nam4rule By    Fetch one Ensembl Base Align Feature object
**                 matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @argrule AllbyHitname hitname [const AjPStr] Hit (target) sequence name
** @argrule AllbyHitname anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitname bafs [AjPList] AJAX List of Ensembl Base Align
**                                      Feature objects
** @argrule AllbyHitunversioned hitname [const AjPStr] Hit (target)
**                                                     sequence name
** @argrule AllbyHitunversioned anname [const AjPStr] Ensembl Analysis name
** @argrule AllbyHitunversioned bafs [AjPList] AJAX List of Ensembl Base
**                                             Align Feature objects
** @argrule AllbySlicecoverage slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicecoverage coverage [float] Alignment target
**                                              coverage threshold
** @argrule AllbySlicecoverage anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySlicecoverage bafs [AjPList]  AJAX List of Ensembl Base
**                                             Align Feature objects
** @argrule AllbySliceexternaldatabasename slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceexternaldatabasename edbname [const AjPStr]
**                                         Ensembl External Database name
** @argrule AllbySliceexternaldatabasename anname [const AjPStr]
**                                         Ensembl Analysis name
** @argrule AllbySliceexternaldatabasename bafs [AjPList] AJAX List of
**                                          Ensembl Base Align Feature objects
** @argrule AllbySliceidentity slice [EnsPSlice] Ensembl Slice
** @argrule AllbySliceidentity identity [float] Alignment identity threshold
** @argrule AllbySliceidentity anname [const AjPStr] Ensembl Analysis name
** @argrule AllbySliceidentity bafs [AjPList] AJAX List of Ensembl Base
**                                            Align Feature objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pbaf [EnsPBasealignfeature*] Ensembl Base Align
**                                                    Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensProteinalignfeatureadaptorFetchAllbyHitname ***********************
**
** Fetch Ensembl Base Align Feature objects (type protein) via a hit name.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbyHitname(
    EnsPProteinalignfeatureadaptor pafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char* txthitname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!pafa)
        return ajFalse;

    if(!hitname)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(pafa);

    ensBaseadaptorEscapeC(ba, &txthitname, hitname);

    constraint = ajFmtStr("protein_align_feature.hit_name = '%s'", txthitname);

    ajCharDel(&txthitname);

    /* Add the Ensembl Analysis name constraint. */

    if(anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(pafa,
                                                      &constraint,
                                                      anname);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
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
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [r] hitname [const AjPStr] Hit (target) sequence name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbyHitunversioned(
    EnsPProteinalignfeatureadaptor pafa,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs)
{
    char* txthitname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!pafa)
        return ajFalse;

    if(!hitname)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(pafa);

    ensBaseadaptorEscapeC(ba, &txthitname, hitname);

    constraint = ajFmtStr("protein_align_feature.hit_name LIKE '%s.%'",
                          txthitname);

    ajCharDel(&txthitname);

    /* Add the Ensembl Analysis name constraint. */

    if(anname && ajStrGetLen(anname))
        ensFeatureadaptorConstraintAppendAnalysisname(pafa,
                                                      &constraint,
                                                      anname);

    result = ensBaseadaptorFetchAllbyConstraint(ba,
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
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] coverage [float] Alignment target coverage threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!pafa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(coverage > 0.0F)
        constraint = ajFmtStr("protein_align_feature.hcoverage > %f",
                              coverage);

    result = ensFeatureadaptorFetchAllbySlice(pafa,
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
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] edbname [const AjPStr] Ensembl External Database name
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchAllbySliceexternaldatabasename(
    EnsPProteinalignfeatureadaptor pafa,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs)
{
    char* txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!pafa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!edbname)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    ensFeatureadaptorEscapeC(pafa, &txtname, edbname);

    constraint = ajFmtStr("external_db.db_name = %s", txtname);

    ajCharDel(&txtname);

    result = ensFeatureadaptorFetchAllbySlice(pafa,
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
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] identity [float] Alignment identity threshold
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [u] bafs [AjPList] AJAX List of Ensembl Base Align Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!pafa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!bafs)
        return ajFalse;

    if(identity > 0.0F)
        constraint = ajFmtStr("protein_align_feature.perc_ident > %f",
                              identity);

    result = ensFeatureadaptorFetchAllbySlice(pafa,
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
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pbaf [EnsPBasealignfeature*] Ensembl Base Align Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorFetchByIdentifier(
    EnsPProteinalignfeatureadaptor pafa,
    ajuint identifier,
    EnsPBasealignfeature* Pbaf)
{
    EnsPBaseadaptor ba = NULL;

    if(!pafa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pbaf)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(pafa);

    return ensBaseadaptorFetchByIdentifier(ba, identifier, (void**) Pbaf);
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Base Align Feature
** objects (type protein) from an Ensembl SQL database.
**
** @fdata [EnsPProteinalignfeatureadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Base Align Feature-releated object(s)
** @nam4rule All Retrieve all Ensembl Base Align Feature-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
**
** @argrule * pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensProteinalignfeatureadaptorRetrieveAllIdentifiers ******************
**
** Fetch all SQL database-internal identifiers of
** Ensembl Base Align Feature objects.
**
** @param [u] pafa [EnsPProteinalignfeatureadaptor] Ensembl Protein Align
**                                                  Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of ajuint identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensProteinalignfeatureadaptorRetrieveAllIdentifiers(
    EnsPProteinalignfeatureadaptor pafa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!pafa)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(pafa);

    table = ajStrNewC("protein_align_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}

/******************************************************************************
** @source Ensembl Intron functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.4 $
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

#include "ensintron.h"
#include "ensexon.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensintron *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library.
**
******************************************************************************/




/* @datasection [EnsPIntron] Intron *******************************************
**
** Functions for manipulating Ensembl Intron objects
**
** Bio::EnsEMBL::Intron CVS Revision: 1.13
**
** @nam2rule Intron
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Intron by pointer.
** It is the responsibility of the user to first destroy any previous
** Intron. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPIntron]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewExons Constructor with two flanking exons
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the use counter
**
** @argrule Obj object [const EnsPIntron] Ensembl Intron
** @argrule Ref intron [EnsPIntron] Ensembl Intron
** @argrule Exons exon1 [EnsPExon] Ensembl Exon 1
** @argrule Exons exon2 [EnsPExon] Ensembl Exon 2
**
** @valrule * [EnsPIntron] Ensembl Intron
**
** @fcategory new
******************************************************************************/




/* @func ensIntronNewExons ****************************************************
**
** Default constructor for an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::new
** @param [u] exon1 [EnsPExon] Ensembl Exon 1
** @param [u] exon2 [EnsPExon] Ensembl Exon 2
**
** @return [EnsPIntron] Ensembl Intron
** @@
******************************************************************************/

EnsPIntron ensIntronNewExons(EnsPExon exon1, EnsPExon exon2)
{
    ajint strand = 0;

    ajuint start = 0;
    ajuint end   = 0;

    AjPStr seqname1 = NULL;
    AjPStr seqname2 = NULL;

    EnsPFeature feature  = NULL;
    EnsPFeature feature1 = NULL;
    EnsPFeature feature2 = NULL;

    EnsPIntron intron = NULL;

    EnsPSlice slice1 = NULL;
    EnsPSlice slice2 = NULL;

    if(ajDebugTest("ensIntronNewExons"))
    {
        ajDebug("ensIntronNewExons\n"
                "  exon1 %p\n"
                "  exon2 %p\n",
                exon1,
                exon2);

        ensExonTrace(exon1, 1);
        ensExonTrace(exon2, 1);
    }

    if(!exon1)
        return NULL;

    if(!exon2)
        return NULL;

    feature1 = ensExonGetFeature(exon1);
    feature2 = ensExonGetFeature(exon2);

    slice1 = ensFeatureGetSlice(feature1);
    slice2 = ensFeatureGetSlice(feature2);

    seqname1 = ensFeatureGetSequenceName(feature1);
    seqname2 = ensFeatureGetSequenceName(feature2);

    /* Both Exons have to be on the same Slice or sequence name. */

    if(!((slice1 && slice2) || (seqname1 && seqname2)))
    {
        ajDebug("ensIntronNewExons got Exons on Slice and sequence names.\n");

        return NULL;
    }

    if(slice1 && slice2 && (!ensSliceMatch(slice1, slice2)))
    {
        ajDebug("ensIntronNewExons got Exons on different Slices.\n");

        return NULL;
    }

    if(seqname1 && seqname2 && (!ajStrMatchCaseS(seqname1, seqname2)))
    {
        ajDebug("ensIntronNewExons got Exons on different sequence names.\n");

        return NULL;
    }

    if(ensFeatureGetStrand(feature1) != ensFeatureGetStrand(feature2))
    {
        ajDebug("ensIntronNewExons got Exons on different strands.\n");

        return NULL;
    }

    if(ensFeatureGetStrand(feature1) >= 0)
    {
        start = ensFeatureGetEnd(feature1)   + 1;
        end   = ensFeatureGetStart(feature2) - 1;
    }
    else
    {
        start = ensFeatureGetEnd(feature2)   + 1;
        end   = ensFeatureGetStart(feature1) - 1;
    }

    if(start > (end + 1))
    {
        ajDebug("ensIntronNewExons requires that the start coordinate %u "
                "is less than the end coordinate %u + 1 ", start, end);

        return NULL;
    }

    strand = ensFeatureGetStrand(feature1);

    if(slice1)
        feature = ensFeatureNewS((EnsPAnalysis) NULL,
                                 slice1,
                                 start,
                                 end,
                                 strand);

    if(seqname1)
        feature = ensFeatureNewN((EnsPAnalysis) NULL,
                                 seqname1,
                                 start,
                                 end,
                                 strand);

    if(feature)
    {
        AJNEW0(intron);

        intron->Feature = feature;

        intron->PreviousExon = ensExonNewRef(exon1);

        intron->NextExon = ensExonNewRef(exon2);

        intron->Use = 1;
    }
    else
        ajDebug("ensIntronNewExons could not create an Ensembl Feature.\n");

    return intron;
}




/* @func ensIntronNewObj ******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPIntron] Ensembl Intron
**
** @return [EnsPIntron] Ensembl Intron or NULL
** @@
******************************************************************************/

EnsPIntron ensIntronNewObj(const EnsPIntron object)
{
    EnsPIntron intron = NULL;

    if(!object)
        return NULL;

    AJNEW0(intron);

    intron->Feature = ensFeatureNewRef(object->Feature);

    intron->PreviousExon = ensExonNewRef(object->PreviousExon);

    intron->NextExon = ensExonNewRef(object->NextExon);

    intron->Use = 1;

    return intron;
}




/* @func ensIntronNewRef ******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] intron [EnsPIntron] Ensembl Intron
**
** @return [EnsPIntron] Ensembl Intron
** @@
******************************************************************************/

EnsPIntron ensIntronNewRef(EnsPIntron intron)
{
    if(!intron)
        return NULL;

    intron->Use++;

    return intron;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Introns.
**
** @fdata [EnsPIntron]
** @fnote None.
**
** @nam3rule Del Destroy (free) an Intron object
**
** @argrule * Pintron [EnsPIntron*] Intron object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensIntronDel *********************************************************
**
** Default destructor for an Ensembl Intron.
**
** @param [d] Pintron [EnsPIntron*] Ensembl Intron address
**
** @return [void]
** @@
******************************************************************************/

void ensIntronDel(EnsPIntron *Pintron)
{
    EnsPIntron pthis = NULL;

    if(!Pintron)
        return;

    if(!*Pintron)
        return;

    if(ajDebugTest("ensIntronDel"))
    {
        ajDebug("ensIntronDel\n"
                "  *Pintron %p\n",
                *Pintron);

        ensIntronTrace(*Pintron, 1);
    }

    pthis = *Pintron;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pintron = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensExonDel(&pthis->PreviousExon);
    ensExonDel(&pthis->NextExon);

    AJFREE(pthis);

    *Pintron = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
** @fnote None.
**
** @nam3rule Get Return Intron attribute(s)
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetPreviousexon Return the previous Ensembl Exon
** @nam4rule GetNextexon Return the next Ensembl Exon
**
** @argrule * intron [const EnsPIntron] Intron
**
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule Nextexon [EnsPExon] Ensembl Exon
** @valrule Previousexon [EnsPExon] Ensembl Exon
**
** @fcategory use
******************************************************************************/




/* @func ensIntronGetFeature **************************************************
**
** Get the Ensembl Feature element of an Ensembl Intron.
**
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPFeature] Ensembl Feature.
** @@
******************************************************************************/

EnsPFeature ensIntronGetFeature(const EnsPIntron intron)
{
    if(!intron)
        return NULL;

    return intron->Feature;
}




/* @func ensIntronGetNextexon *************************************************
**
** Get the next Ensembl Exon element of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::next_Exon
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPExon] Ensembl Exon
** @@
******************************************************************************/

EnsPExon ensIntronGetNextexon(const EnsPIntron intron)
{
    if(!intron)
        return NULL;

    return intron->NextExon;
}




/* @func ensIntronGetPreviousexon *********************************************
**
** Get the previous Ensembl Exon element of an Ensembl Intron.
**
** @cc Bio::EnsEMBL::Intron::prev_Exon
** @param [r] intron [const EnsPIntron] Ensembl Intron
**
** @return [EnsPExon] Ensembl Exon
** @@
******************************************************************************/

EnsPExon ensIntronGetPreviousexon(const EnsPIntron intron)
{
    if(!intron)
        return NULL;

    return intron->PreviousExon;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Intron object.
**
** @fdata [EnsPIntron]
** @nam3rule Trace Report Intron elements to debug file
**
** @argrule Trace intron [const EnsPIntron] Ensembl Intron
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensIntronTrace *******************************************************
**
** Trace an Ensembl Intron.
**
** @param [r] intron [const EnsPIntron] Ensembl Intron
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensIntronTrace(const EnsPIntron intron, ajuint level)
{
    AjPStr indent = NULL;

    if(!intron)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensIntronTrace %p\n"
            "%S  Feature %p\n"
            "%S  PreviousExon %p\n"
            "%S  NextExon %p\n"
            "%S  Use %u\n",
            indent, intron,
            indent, intron->Feature,
            indent, intron->PreviousExon,
            indent, intron->NextExon,
            indent, intron->Use);

    ensFeatureTrace(intron->Feature, level + 1);

    ensExonTrace(intron->PreviousExon, level + 1);

    ensExonTrace(intron->NextExon, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}

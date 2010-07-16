/******************************************************************************
** @source Ensembl Slice functions
**
** An Ensembl Slice object represents a region of a genome. It can be used to
** retrieve sequence or features from an area of interest.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.19 $
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

#include "ensslice.h"
#include "ensattribute.h"
#include "enssequence.h"
#include "enssequenceedit.h"
#include "ensrepeat.h"
#include "enstranslation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* sliceSequenceEditCode ******************************************************
**
** Ensembl Sequence Edits for Ensembl Slices are a sub-set of
** Ensembl Attributes that provide information about modifications of the
** Slice sequence. Attributes with the following codes are Sequence Edits on
** the Slice-level.
**
** _rna_edit: General sequence edit
**
******************************************************************************/

static const char *sliceSequenceEditCode[] =
{
    "_rna_edit",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static EnsPProjectionsegment sliceConstrainToSeqregion(EnsPSlice slice);

static void* sliceadaptorCacheReference(void *value);

static void sliceadaptorCacheDelete(void **value);

static ajulong sliceadaptorCacheSize(const void* value);




/* @filesection ensslice ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSlice] Slice *********************************************
**
** Functions for manipulating Ensembl Slice objects
**
** @cc Bio::EnsEMBL::Slice CVS Revision: 1.239
**
** @nam2rule Slice
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Slice by pointer.
** It is the responsibility of the user to first destroy any previous
** Slice. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSlice]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPSlice] Ensembl Slice
** @argrule Ref object [EnsPSlice] Ensembl Slice
**
** @valrule * [EnsPSlice] Ensembl Slice
**
** @fcategory new
******************************************************************************/




/* @func ensSliceNewS *********************************************************
**
** Construct a new Ensembl Slice with sequence information.
**
** @cc Bio::EnsEMBL::Slice::new
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajint] Ensembl Sequence Region start
** @param [r] end [ajint] Ensembl Sequence Region end
** @param [r] strand [ajint] Ensembl Sequence Region strand
** @param [u] sequence [AjPStr] Sequence (optional)
**
** @return [EnsPSlice] Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPSlice ensSliceNewS(EnsPSliceadaptor adaptor,
                       EnsPSeqregion sr,
                       ajint start,
                       ajint end,
                       ajint strand,
                       AjPStr sequence)
{
    EnsPSlice slice = NULL;

    if(!sr)
    {
        ajDebug("ensSliceNewS requires an Ensembl Sequence Region.\n");

        return NULL;
    }

    if(ensCoordsystemIsTopLevel(ensSeqregionGetCoordsystem(sr)))
    {
        ajDebug("ensSliceNewS cannot create a Slice on a Sequence Region "
                "with a top-level Coordinate System.\n");

        return NULL;
    }

    if(!start)
        start = 1;

    if(!end)
        end = ensSeqregionGetLength(sr);

    if(start > (end + 1))
    {
        ajDebug("ensSliceNewS requires that the start coordinate %d is "
                "less than or equal to the end coordinate %d + 1.\n",
                start, end);

        return NULL;
    }

    if(!strand)
        strand = 1;

    if((strand != 1) && (strand != -1))
    {
        ajDebug("ensSliceNewS requires a strand of either 1 or -1 "
                "not %d.\n", strand);

        return NULL;
    }

    if(sequence &&
       (ajStrGetLen(sequence) != (ajuint) ensSeqregionGetLength(sr)))
    {
        ajDebug("ensSliceNewS requires that the Sequence Region length %d "
                "matches the length of the Sequence string %u.\n",
                ensSeqregionGetLength(sr), ajStrGetLen(sequence));

        return NULL;
    }

    AJNEW0(slice);

    slice->Adaptor = adaptor;

    slice->Seqregion = ensSeqregionNewRef(sr);

    if(sequence)
        slice->Sequence = ajStrNewRef(sequence);

    slice->Start = start;

    slice->End = end;

    slice->Strand = strand;

    slice->Use = 1;

    return slice;
}




/* @func ensSliceNew **********************************************************
**
** Default constructor for an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::new
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajint] Ensembl Sequence Region start
** @param [r] end [ajint] Ensembl Sequence Region end
** @param [r] strand [ajint] Ensembl Sequence Region strand
**
** @return [EnsPSlice] Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPSlice ensSliceNew(EnsPSliceadaptor adaptor,
                      EnsPSeqregion sr,
                      ajint start,
                      ajint end,
                      ajint strand)
{
    EnsPSlice slice = NULL;

    if(!sr)
    {
        ajDebug("ensSliceNew requires an Ensembl Sequence Region.\n");

        return NULL;
    }

    if(ensCoordsystemIsTopLevel(ensSeqregionGetCoordsystem(sr)))
    {
        ajDebug("ensSliceNew cannot create a Slice on a Sequence Region "
                "with a top-level Coordinate System.\n");

        return NULL;
    }

    if(!start)
        start = 1;

    if(!end)
        end = ensSeqregionGetLength(sr);

    if(start > (end + 1))
    {
        ajDebug("ensSliceNew requires that the start coordinate %d is "
                "less than or equal to the end coordinate %d + 1.\n",
                start, end);

        return NULL;
    }

    if(!strand)
        strand = 1;

    if((strand != 1) && (strand != -1))
    {
        ajDebug("ensSliceNew requires a strand of either 1 or -1 "
                "not %d.\n", strand);

        return NULL;
    }

    AJNEW0(slice);

    slice->Adaptor = adaptor;

    slice->Seqregion = ensSeqregionNewRef(sr);

    slice->Sequence = (AjPStr) NULL;

    slice->Start = start;

    slice->End = end;

    slice->Strand = strand;

    slice->Use = 1;

    return slice;
}




/* @func ensSliceNewObj *******************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [EnsPSlice] Ensembl Slice
**
** @return [EnsPSlice] Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPSlice ensSliceNewObj(EnsPSlice object)
{
    EnsPSlice slice = NULL;

    if(!object)
        return NULL;

    AJNEW0(slice);

    slice->Adaptor = object->Adaptor;

    slice->Seqregion = ensSeqregionNewRef(object->Seqregion);

    if(object->Sequence)
        slice->Sequence = ajStrNewRef(object->Sequence);

    slice->Start = object->Start;

    slice->End = object->End;

    slice->Strand = object->Strand;

    slice->Use = 1;

    return slice;
}




/* @func ensSliceNewRef *******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsPSlice] Ensembl Slice or NULL
** @@
******************************************************************************/

EnsPSlice ensSliceNewRef(EnsPSlice slice)
{
    if(!slice)
        return NULL;

    slice->Use++;

    return slice;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Slice.
**
** @fdata [EnsPSlice]
** @fnote None
**
** @nam3rule Del Destroy (free) a Slice object
**
** @argrule * Pslice [EnsPSlice*] Slice object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSliceDel **********************************************************
**
** Default Ensembl Slice destructor.
**
** @param [d] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [void]
** @@
******************************************************************************/

void ensSliceDel(EnsPSlice *Pslice)
{
    EnsPSlice pthis = NULL;

    if(!Pslice)
        return;

    if(!*Pslice)
        return;

    if(ajDebugTest("ensSliceDel"))
    {
        ajDebug("ensSliceDel\n"
                "  *Pslice %p\n",
                *Pslice);

        ensSliceTrace(*Pslice, 1);
    }

    pthis = *Pslice;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pslice = NULL;

        return;
    }

    ensSeqregionDel(&pthis->Seqregion);

    ajStrDel(&pthis->Sequence);

    AJFREE(pthis);

    *Pslice = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
** @fnote None
**
** @nam3rule Get Return Slice attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Slice Adaptor
** @nam4rule GetSeqregion Return the Ensembl Sequence Region
** @nam4rule GetSequence Return the sequence
** @nam4rule GetStart Return the start
** @nam4rule GetEnd Return the end
** @nam4rule GetStrand Return the strand
**
** @argrule * slice [const EnsPSlice] Slice
**
** @valrule Adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @valrule Seqregion [EnsPSeqregion] Ensembl Sequence Region
** @valrule Sequence [AjPStr] Sequence
** @valrule Start [ajint] Start
** @valrule End [ajint] End
** @valrule Strand [ajint] Strand
**
** @fcategory use
******************************************************************************/




/* @func ensSliceGetAdaptor ***************************************************
**
** Get the Ensembl Slice Adaptor element an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::adaptor
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPSliceadaptor] Ensembl Slice Adaptor
** @@
******************************************************************************/

EnsPSliceadaptor ensSliceGetAdaptor(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    return slice->Adaptor;
}




/* @func ensSliceGetSeqregion *************************************************
**
** Get the Ensembl Sequence Region element of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPSeqregion] Ensembl Sequence Region
** @@
******************************************************************************/

EnsPSeqregion ensSliceGetSeqregion(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    return slice->Seqregion;
}




/* @func ensSliceGetSequence **************************************************
**
** Get the sequence element of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Sequence
** @@
******************************************************************************/

const AjPStr ensSliceGetSequence(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    return slice->Sequence;
}




/* @func ensSliceGetStart *****************************************************
**
** Get the start coordinate element of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::start
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Start coordinate
** @@
******************************************************************************/

ajint ensSliceGetStart(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return slice->Start;
}




/* @func ensSliceGetEnd *******************************************************
**
** Get the end coordinate element of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::end
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] End coordinate
** @@
******************************************************************************/

ajint ensSliceGetEnd(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return slice->End;
}




/* @func ensSliceGetStrand ****************************************************
**
** Get the strand element of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::strand
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Strand information
** @@
******************************************************************************/

ajint ensSliceGetStrand(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return slice->Strand;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
** @fnote None
**
** @nam3rule Set Set one element of a Slice
** @nam4rule SetAdaptor Set the Ensembl Slice Adaptor
** @nam4rule SetSequence Set the sequence
**
** @argrule * slice [EnsPSlice] Ensembl Slice object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSliceSetAdaptor ***************************************************
**
** Set the Ensembl Slice Adaptor element of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceSetAdaptor(EnsPSlice slice, EnsPSliceadaptor adaptor)
{
    if(!slice)
        return ajFalse;

    slice->Adaptor = adaptor;

    return ajTrue;
}




/* @func ensSliceSetSequence **************************************************
**
** Set the sequence element of an Ensembl Slice.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] sequence [AjPStr] Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceSetSequence(EnsPSlice slice, AjPStr sequence)
{
    if(!slice)
        return ajFalse;

    ajStrDel(&slice->Sequence);

    if(sequence)
    {
        if(ajStrGetLen(sequence) == ensSliceGetLength(slice))
            slice->Sequence = ajStrNewRef(sequence);
        else
            ajFatal("ensSliceSetSequence got sequence of length %u, "
                    "which does not match the length of the Slice %u.\n",
                    ajStrGetLen(sequence),
                    ensSliceGetLength(slice));
    }

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Slice object.
**
** @fdata [EnsPSlice]
** @nam3rule Trace Report Ensembl Slice elements to debug file
**
** @argrule Trace slice [const EnsPSlice] Ensembl Slice
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSliceTrace ********************************************************
**
** Trace an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceTrace(const EnsPSlice slice, ajuint level)
{
    AjPStr indent = NULL;

    if(!slice)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSliceTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Seqregion %p\n"
            "%S  Sequence %p\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Strand %d\n"
            "%S  Use %u\n",
            indent, slice,
            indent, slice->Adaptor,
            indent, slice->Seqregion,
            indent, slice->Sequence,
            indent, slice->Start,
            indent, slice->End,
            indent, slice->Strand,
            indent, slice->Use);

    ensSeqregionTrace(slice->Seqregion, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensSliceGetSeqregionIdentifier ***************************************
**
** Get the identifier element of the Ensembl Sequence Region element of an
** Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajuint] Ensembl Sequence Region identifier
** @@
******************************************************************************/

ajuint ensSliceGetSeqregionIdentifier(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return ensSeqregionGetIdentifier(slice->Seqregion);
}




/* @func ensSliceGetSeqregionName *********************************************
**
** Get the name element of the Ensembl Sequence Region element of an
** Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Ensembl Sequence Region name
** @@
******************************************************************************/

const AjPStr ensSliceGetSeqregionName(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    return ensSeqregionGetName(slice->Seqregion);
}




/* @func ensSliceGetSeqregionLength *******************************************
**
** Get the length element of the Ensembl Sequence Region element of an
** Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Ensembl Sequence Region length
** @@
******************************************************************************/

ajint ensSliceGetSeqregionLength(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return ensSeqregionGetLength(slice->Seqregion);
}




/* @func ensSliceGetCoordsystem ***********************************************
**
** Get the Ensembl Coordinate System element of the
** Ensembl Sequence Region element of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::coord_system
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [EnsPCoordsystem] Ensembl Coordinate System
** @@
******************************************************************************/

EnsPCoordsystem ensSliceGetCoordsystem(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    if(!slice->Seqregion)
    {
        ajDebug("ensSliceGetCoordsystem cannot get the Coordinate System of "
                "a Slice without a Sequence Region.\n");

        return NULL;
    }

    return ensSeqregionGetCoordsystem(slice->Seqregion);
}




/* @func ensSliceGetCoordsystemName *******************************************
**
** Get the name element of the Ensembl Coordinate System element of the
** Ensembl Sequence Region element of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::coord_system_name
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Ensembl Coordinate System name
** @@
******************************************************************************/

const AjPStr ensSliceGetCoordsystemName(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    if(!slice->Seqregion)
    {
        ajDebug("ensSliceGetCoordsystemName cannot get the Coordinate System "
                "name of a Slice without a Sequence Region.\n");

        return NULL;
    }

    return ensCoordsystemGetName(ensSeqregionGetCoordsystem(slice->Seqregion));
}




/* @func ensSliceGetCoordsystemVersion ****************************************
**
** Get the version element of the Ensembl Coordinate System element of the
** Ensembl Sequence Region element of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [const AjPStr] Ensembl Coordinate System version
** @@
******************************************************************************/

const AjPStr ensSliceGetCoordsystemVersion(const EnsPSlice slice)
{
    if(!slice)
        return NULL;

    if(!slice->Seqregion)
    {
        ajDebug("ensSliceGetCoordsystemVersion cannot get the "
                "Coordinate System version for a Slice without a "
                "Sequence Region.\n");

        return NULL;
    }

    return ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(
                                        slice->Seqregion));
}




/* @func ensSliceGetCentrePoint ***********************************************
**
** Get the centre point coordinate of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::centrepoint
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajint] Centre point coordinate or 0
** @@
******************************************************************************/

ajint ensSliceGetCentrePoint(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return (slice->Start + slice->End) / 2;
}




/* @func ensSliceGetLength ****************************************************
**
** Get the length of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::length
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajuint] Ensembl Slice length
** @@
******************************************************************************/

ajuint ensSliceGetLength(const EnsPSlice slice)
{
    if(!slice)
        return 0;

    return (ajuint) (slice->End - slice->Start + 1);
}




/* @func ensSliceGetMemsize ***************************************************
**
** Get the memory size in bytes of an Ensembl Slice.
**
** @param [r] slice [const EnsPSlice] Ensembl Slice
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensSliceGetMemsize(const EnsPSlice slice)
{
    ajulong size = 0;

    if(!slice)
        return 0;

    size += sizeof (EnsOSlice);

    size += ensSeqregionGetMemsize(slice->Seqregion);

    if(slice->Sequence)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(slice->Sequence);
    }

    return size;
}




/* @func ensSliceGetTranslation ***********************************************
**
** Get an AJAX Translation for an Ensembl Slice.
**
** The AJAX Translation will match the codon table defined as an
** Ensembl Attribute of code 'codon_table' associated with an Ensembl Slice.
** If no Attribute is associated with this Slice, an AJAX Translation based on
** codon table 0, the standard code with translation start at AUG only,
** will be returned.
**
** @param [r] slice [EnsPSlice] Ensembl Slice
**
** @return [const AjPTrn] AJAX Translation or NULL
** @@
******************************************************************************/

const AjPTrn ensSliceGetTranslation(EnsPSlice slice)
{
    ajuint codontable = 0;

    AjPList attributes = NULL;

    AjPStr code  = NULL;
    AjPStr value = NULL;

    EnsPAttribute attribute = NULL;

    if(!slice)
        return NULL;

    code = ajStrNewC("codon_table");

    attributes = ajListNew();

    ensSliceFetchAllAttributes(slice, code, attributes);

    ajStrDel(&code);

    while(ajListPop(attributes, (void **) &attribute))
    {
        value = ensAttributeGetValue(attribute);

        if(value && ajStrGetLen(value))
        {
            if(!ajStrToUint(value, &codontable))
                ajWarn("ensSliceGetTranslation Could not parse "
                       "Ensembl Attribute value '%S' into an "
                       "unsigned integer value.",
                       value);
        }
        else
        {
            ajDebug("ensSliceGetTranslation got Ensembl Attribute %p with an "
                    "empty value.",
                    attribute);

            ensAttributeTrace(attribute, 1);
        }

        ensAttributeDel(&attribute);
    }

    ajListFree(&attributes);

    return ensTranslationCacheGetTranslation(codontable);
}




/* @func ensSliceFetchName ****************************************************
**
** Fetch the name of an Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::name
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [wP] Pname [AjPStr*] Name String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchName(const EnsPSlice slice, AjPStr* Pname)
{
    EnsPCoordsystem cs = NULL;

    if(!slice)
        return ajFalse;

    if(!Pname)
        return ajFalse;

    if(*Pname)
        ajStrDel(Pname);

    cs = ensSeqregionGetCoordsystem(slice->Seqregion);

    *Pname = ajFmtStr("%S:%S:%S:%d:%d:%d",
                      ensCoordsystemGetName(cs),
                      ensCoordsystemGetVersion(cs),
                      ensSeqregionGetName(slice->Seqregion),
                      slice->Start,
                      slice->End,
                      slice->Strand);

    return ajTrue;
}




/* @func ensSliceCompareIdentifierAscending ***********************************
**
** Comparison function to sort Ensembl Slices by their Ensembl Sequence Region
** identifier in ascending order.
**
** Ensembl Slices without Ensembl Sequence Region objects sort towards the
** end of the AJAX List.
**
** @param [r] slice1 [const EnsPSlice] Ensembl Slice 1
** @param [r] slice2 [const EnsPSlice] Ensembl Slice 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

int ensSliceCompareIdentifierAscending(const EnsPSlice slice1,
                                       const EnsPSlice slice2)
{
    int value = 0;

    ajuint srid1 = 0;
    ajuint srid2 = 0;

    /* Sort empty values towards the end of the AJAX List. */

    if(slice1 && (!slice2))
        return -1;

    if((!slice1) && (!slice2))
        return 0;

    if((!slice1) && slice2)
        return +1;

    srid1 = ensSliceGetSeqregionIdentifier(slice1);
    srid2 = ensSliceGetSeqregionIdentifier(slice2);

    if(srid1 < srid2)
        value = -1;

    if(srid1 > srid2)
        value = +1;

    return value;
}




/* @func ensSliceMatch ********************************************************
**
** Test for matching two Ensembl Slices.
**
** @param [r] slice1 [const EnsPSlice] First Ensembl Slice
** @param [r] slice2 [const EnsPSlice] Second Ensembl Slice
**
** @return [AjBool] ajTrue if the Slices are equal
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, the Coordinate System, the Sequence Region, as well as the Slice
** start, end and strand elements are compared. In case the Slice has a
** sequence String attached, it is compared as well.
******************************************************************************/

AjBool ensSliceMatch(const EnsPSlice slice1, const EnsPSlice slice2)
{
    if(ajDebugTest("ensSliceMatch"))
    {
        ajDebug("ensSliceMatch\n"
                "  slice1 %p\n"
                "  slice2 %p\n",
                slice1,
                slice2);

        ensSliceTrace(slice1, 1);
        ensSliceTrace(slice2, 1);
    }

    if(!slice1)
        return ajFalse;

    if(!slice2)
        return ajFalse;

    /* Try a direct pointer comparison first. */

    if(slice1 == slice2)
        return ajTrue;

    if(!ensSeqregionMatch(slice1->Seqregion, slice2->Seqregion))
        return ajFalse;

    if(slice1->Start != slice2->Start)
        return ajFalse;

    if(slice1->End != slice2->End)
        return ajFalse;

    if(slice1->Strand != slice2->Strand)
        return ajFalse;

    if((slice1->Sequence || slice2->Sequence) &&
       (!ajStrMatchS(slice1->Sequence, slice2->Sequence)))
        return ajFalse;

    return ajTrue;
}




/* @func ensSliceIsTopLevel ***************************************************
**
** Test whether an Ensembl Slice is based on an Ensembl Sequence Region, which
** has an Ensembl Attribute of code 'top-level' set.
**
** @cc Bio::EnsEMBL::Slice::is_toplevel
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] Presult [AjBool*] ajTrue if the Ensembl Sequence Region has an
**                              Ensembl Attribute of code 'top-level' set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceIsTopLevel(EnsPSlice slice, AjBool* Presult)
{
    if(!slice)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    if(!slice->Seqregion)
    {
        ajDebug("ensSliceIsTopLevel requires the Ensembl Slice to have an "
                "Ensembl Sequence region attached.\n");

        return ajFalse;
    }

    return ensSeqregionIsTopLevel(slice->Seqregion, Presult);
}




/* @func ensSliceFetchSequenceSeq *********************************************
**
** Fetch the sequence of an Ensembl Slice as AJAX Sequence.
**
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchSequenceSeq(EnsPSlice slice, AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!slice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    /* Fetch the Slice name. */

    name = ajStrNew();

    ensSliceFetchName(slice, &name);

    /* Fetch the Slice sequence. */

    sequence = ajStrNew();

    ensSliceFetchSequenceStr(slice, &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensSliceFetchSequenceStr *********************************************
**
** Fetch the sequence of an Ensembl Slice as AJAX String.
**
** @cc Bio::EnsEMBL::Slice::seq
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPStr*] Sequence String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchSequenceStr(EnsPSlice slice, AjPStr *Psequence)
{
    EnsPDatabaseadaptor dba = NULL;

    EnsPSequenceadaptor sa = NULL;

    if(!slice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(ensSliceGetLength(slice) + 1);

    /* Special case for "in-between" (insertion) coordinates. */

    if(slice->Start == (slice->End + 1))
        return ajTrue;

    if(slice->Sequence)
    {
        /*
        ** Since the Slice has sequence attached, check whether its
        ** Slice length matches its sequence length.
        */

        if(ajStrGetLen(slice->Sequence) != ensSliceGetLength(slice))
            ajFatal("ensSliceFetchSequenceStr got a Slice, "
                    "which sequence length %u does not match its length %u.\n",
                    ajStrGetLen(slice->Sequence),
                    ensSliceGetLength(slice));

        ajStrAssignS(Psequence, slice->Sequence);
    }
    else if(slice->Adaptor)
    {
        /*
        ** Since the Slice has a Slice Adaptor attached, it is possible to
        ** retrieve the sequence from the database.
        */

        dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

        sa = ensRegistryGetSequenceadaptor(dba);

        ensSequenceadaptorFetchStrBySlice(sa, slice, Psequence);
    }
    else
    {
        /*
        ** The Slice has no sequence and no Slice Adaptor attached,
        ** so just return Ns.
        */

        ajStrAppendCountK(Psequence, 'N', ensSliceGetLength(slice));
    }

    if(ajStrGetLen(*Psequence) != ensSliceGetLength(slice))
        ajWarn("ensSliceFetchSequenceStr got sequence of length %u "
               "for Ensembl Slice of length %u.\n",
               ajStrGetLen(*Psequence),
               ensSliceGetLength(slice));

    return ajTrue;
}




/* @func ensSliceFetchSubSequenceSeq ******************************************
**
** Fetch a sub-sequence of an Ensembl Slice as AJAX Sequence.
**
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPSeq*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchSubSequenceSeq(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPSeq* Psequence)
{
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!slice)
        return ajFalse;

    if(!strand)
        strand = 1;

    if(!Psequence)
        return ajFalse;

    /*
    ** Construct the Slice name, but convert relative Slice coordinates into
    ** absolute Sequence Region coordinates.
    */

    if(slice->Strand >= 0)
    {
        srstart = slice->Start + start - 1;
        srend   = slice->Start + end   - 1;
    }
    else
    {
        srstart = slice->End - end   + 1;
        srend   = slice->End - start + 1;
    }

    srstrand = slice->Strand * strand;

    name = ajFmtStr("%S:%S:%S:%d:%d:%d",
                    ensSliceGetCoordsystemName(slice),
                    ensSliceGetCoordsystemVersion(slice),
                    ensSliceGetSeqregionName(slice),
                    srstart,
                    srend,
                    srstrand);

    /* Fetch the Slice sequence. */

    sequence = ajStrNew();

    ensSliceFetchSubSequenceStr(slice, start, end, strand, &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensSliceFetchSubSequenceStr ******************************************
**
** Fetch a sub-sequence of an Ensembl Slice as AJAX String
** in releative coordinates.
**
** @cc Bio::EnsEMBL::Slice::subseq
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchSubSequenceStr(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPStr *Psequence)
{
    ajint length = 0;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSequenceadaptor sa = NULL;

    if(ajDebugTest("ensSliceFetchSubSequenceStr"))
        ajDebug("ensSliceFetchSubSequenceStr\n"
                "  slice %p\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  Psequence %p\n",
                slice,
                start,
                end,
                strand,
                Psequence);

    if(!slice)
        return ajFalse;

    if(start > (end + 1))
    {
        ajDebug("ensSliceFetchSubSequenceStr requires the start coordinate %d "
                "to be less than the end coordinate %d + 1.\n",
                start, end);

        return ajFalse;
    }

    if(!strand)
        strand = 1;

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes((ajuint) (end - start + 1));

    /* For "in-between" (insertion) coordinates return an empty string. */

    if(start == (end + 1))
        return ajTrue;

    if(slice->Sequence)
    {
        /*
        ** Since the Slice has sequence attached, check whether
        ** Slice length and sequence length match.
        */

        if(ajStrGetLen(slice->Sequence) != ensSliceGetLength(slice))
            ajFatal("ensSliceFetchSubSequenceStr got a Slice, "
                    "which sequence length (%u) does not match its "
                    "length (%u).\n",
                    ajStrGetLen(slice->Sequence),
                    ensSliceGetLength(slice));

        /* Relative Slice coordinates range from 1 to length. */

        /* Check for a gap at the beginning and pad it with Ns. */

        if(start < 1)
        {
            ajStrAppendCountK(Psequence, 'N', 1 - start);

            start = 1;
        }

        ajStrAppendSubS(Psequence,
                        slice->Sequence,
                        start - 1,
                        end - start + 1);

        /* Check that the Slice is within signed integer range. */

        if(ensSliceGetLength(slice) <= INT_MAX)
            length = (ajint) ensSliceGetLength(slice);
        else
            ajFatal("ensSliceFetchSubSequenceStr got an "
                    "Ensembl Slice, which length (%u) exceeds the "
                    "maximum integer limit (%d).\n",
                    ensSliceGetLength(slice), INT_MAX);

        /* Check for a gap at the end and pad it again with Ns. */

        if(end > length)
            ajStrAppendCountK(Psequence, 'N', (ajuint) (end - length));

        if(strand < 0)
            ajSeqstrReverse(Psequence);
    }
    else if(slice->Adaptor)
    {
        /*
        ** Since the Slice has a Slice Adaptor attached, it is possible to
        ** retrieve the sequence from the database.
        */

        dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

        sa = ensRegistryGetSequenceadaptor(dba);

        ensSequenceadaptorFetchSubStrBySlice(sa,
                                             slice,
                                             start,
                                             end,
                                             strand,
                                             Psequence);
    }
    else
    {
        /*
        ** The Slice has no sequence and no Slice Adaptor attached,
        ** so just return Ns.
        */

        ajStrAppendCountK(Psequence, 'N', (ajuint) (end - start + 1));
    }

    if(ajStrGetLen(*Psequence) != (ajuint) (end - start + 1))
        ajWarn("ensSliceFetchSubSequenceStr got sequence of length %u "
               "for region of length %u.\n",
               ajStrGetLen(*Psequence),
               (ajuint) (end - start + 1));

    return ajTrue;
}




/* @func ensSliceFetchInvertedSlice *******************************************
**
** Fetch an inverted Slice from an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::invert
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
** @see ensSliceadaptorFetchBySlice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: Currently, this function fetches the expanded Slice via a
** Slice Adaptor, if one is set for this Slice. The Slice Adaptor registers
** Slices in the internal cache, which minimises memory requirements.
** However, registering many Slices, the LRU cache may drop important Slices.
******************************************************************************/

AjBool ensSliceFetchInvertedSlice(EnsPSlice slice, EnsPSlice *Pslice)
{
    AjPStr sequence = NULL;

    if(!slice)
        return ajFalse;

    if(!Pslice)
        return ajFalse;

    /*
    ** Retrieve the inverted Slice via the Slice Adaptor if one is attached.
    ** This will automatically register the new Slice in the
    ** Slice Adaptor-internal cache and will also take care of
    ** an eventual Slice-internal sequence.
    ** Use relative Slice coordinates, which range from 1 to length.
    */

    if(slice->Adaptor)
    {
        ensSliceadaptorFetchBySlice(slice->Adaptor,
                                    slice,
                                    1,
                                    (ajint) ensSliceGetLength(slice),
                                    -1,
                                    Pslice);
        return ajTrue;
    }

    if(slice->Sequence)
    {
        /* Reverse (and complement) the sequence if one has been defined. */

        sequence = ajStrNewS(slice->Sequence);

        ajSeqstrReverse(&sequence);

        *Pslice = ensSliceNewS(slice->Adaptor,
                               slice->Seqregion,
                               slice->Start,
                               slice->End,
                               slice->Strand * -1,
                               sequence);

        ajStrDel(&sequence);
    }
    else
        *Pslice = ensSliceNew(slice->Adaptor,
                              slice->Seqregion,
                              slice->Start,
                              slice->End,
                              slice->Strand * -1);

    return ajTrue;
}




/* @func ensSliceFetchSubSlice ************************************************
**
** Fetch a Sub-Slice from an Ensembl Slice.
**
** If a Slice is requested, which lies outside the boundaries of this Slice,
** this function will return NULL. This means that the behaviour will be
** consistent whether or not the Slice is attached to the database
** (i.e. if there is attached sequence to the Slice).
** Alternatively, the ensSliceFetchExpandedSlice or
** ensSliceAdaptorFetchByRegion functions could be used instead.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::sub_Slice
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
** @see ensSliceadaptorFetchBySlice
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: Currently, this function fetches the expanded Slice via a
** Slice Adaptor, if one is set for this Slice. The Slice Adaptor registers
** Slices in the internal cache, which minimises memory requirements.
** However, registering many Slices, the LRU cache may drop important Slices.
******************************************************************************/

AjBool ensSliceFetchSubSlice(EnsPSlice slice,
                             ajint start,
                             ajint end,
                             ajint strand,
                             EnsPSlice *Pslice)
{
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    AjPStr sequence = NULL;

    if(!slice)
    {
        ajDebug("ensSliceGetSubSlice requires an Ensembl Slice.\n");

        return ajFalse;
    }

    if((start < 1) || (start > slice->End))
    {
        ajDebug("ensSliceGetSubSlice requires the start coordinate %d "
                "to be greater than or equal to 1 and less than or equal to "
                "the end coordinate %d of the Ensembl Slice.\n",
                start, slice->End);

        return ajFalse;
    }

    if((end < start) || (end > slice->End))
    {
        ajDebug("ensSliceGetSubSlice requires the end coordinate %d "
                "to be greater than or equal to the start coordinate %d "
                "and to be less than or equal to the "
                "end coordinate %d of the Ensembl Slice.\n",
                end, start, slice->End);

        return ajFalse;
    }

    if(!strand)
        strand = 1;

    /*
    ** Retrieve the Sub-Slice via the Slice Adaptor if one is attached.
    ** This will automatically register the new Slice in the
    ** Slice Adaptor-internal cache and will also take care of
    ** an eventual Slice-internal sequence.
    */

    if(slice->Adaptor)
    {
        ensSliceadaptorFetchBySlice(slice->Adaptor,
                                    slice,
                                    start,
                                    end,
                                    strand,
                                    Pslice);

        return ajTrue;
    }

    /* Transform relative into absolute Slice coordinates. */

    if(slice->Strand >= 0)
    {
        srstart = slice->Start + start - 1;
        srend   = slice->Start + end   - 1;
    }
    else
    {
        srstart = slice->End - end   + 1;
        srend   = slice->End - start + 1;
    }

    srstrand = slice->Strand * strand;

    if(slice->Sequence)
    {
        sequence = ajStrNewRes(end - start + 1);

        ensSliceFetchSubSequenceStr(slice,
                                    start,
                                    end,
                                    strand,
                                    &sequence);

        *Pslice = ensSliceNewS(slice->Adaptor,
                               slice->Seqregion,
                               srstart,
                               srend,
                               srstrand,
                               sequence);

        ajStrDel(&sequence);
    }
    else
        *Pslice = ensSliceNew(slice->Adaptor,
                              slice->Seqregion,
                              srstart,
                              srend,
                              srstrand);

    return ajTrue;
}




/* @func ensSliceFetchExpandedSlice *******************************************
**
** Fetch an expanded copy of an Ensembl Slice, which remains unchanged.
** The start and end are moved outwards from the center of the Slice if
** positive values are provided and moved inwards if negative values are
** provided. A Slice may not be contracted below 1 base pair but may grow
** to be arbitrarily large.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @cc Bio::EnsEMBL::Slice::expand
** @param [u] slice [const EnsPSlice] Ensembl Slice
** @param [r] five [ajint] Number of bases to expand the 5' region
**            Positive values expand the Slice, negative values contract it
** @param [r] three [ajint] Number of bases to expand the 3' region
**            Positive values expand the Slice, negative values contract it
** @param [r] force [AjBool] Force Slice contraction, even when shifts for
**                          'five' and 'three' overlap. In that case 'five' and
**                          'three' will be set to a maximum possible number
**                           resulting in a Slice, which would have only 2 base
**                           pairs.
** @param [w] Pfshift [ajint*] Maximum possible 5' shift when 'force' is set
** @param [w] Ptshift [ajint*] Maximum possible 3' shift when 'force' is set
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: Currently, this function fetches the expanded Slice via a
** Slice Adaptor, if one is set for this Slice. The Slice Adaptor registers
** Slices in the internal cache, which minimises memory requirements.
** However, registering many Slices, the LRU cache may drop important Slices.
******************************************************************************/

AjBool ensSliceFetchExpandedSlice(const EnsPSlice slice,
                                  ajint five,
                                  ajint three,
                                  AjBool force,
                                  ajint *Pfshift,
                                  ajint *Ptshift,
                                  EnsPSlice *Pslice)
{
    ajint sshift = 0;
    ajint eshift = 0;

    ajuint srid   = 0;
    ajint srstart = 0;
    ajint srend   = 0;

    if(ajDebugTest("ensSliceFetchExpandedSlice"))
        ajDebug("ensSliceFetchExpandedSlice\n"
                "  slice %p\n"
                "  five %d\n"
                "  three %d\n"
                "  force '%B'\n"
                "  Pfshift %p\n"
                "  Ptshift %p\n"
                "  Pslice %p\n",
                slice,
                five,
                three,
                force,
                Pfshift,
                Ptshift,
                Pslice);

    if(!slice)
        return ajFalse;

    if(!Pfshift)
        return ajFalse;

    if(!Ptshift)
        return ajFalse;

    if(!Pslice)
        return ajFalse;

    *Pslice = (EnsPSlice) NULL;

    if(slice->Sequence)
    {
        ajDebug("ensSliceFetchExpandedSlice cannot expand an Ensembl Slice "
                "with an attached sequence.\n");

        return ajFalse;
    }

    if(slice->Strand > 0)
    {
        sshift = five;
        eshift = three;
    }
    else
    {
        sshift = three;
        eshift = five;
    }

    srid = ensSeqregionGetIdentifier(slice->Seqregion);

    srstart = slice->Start - sshift;
    srend   = slice->End   + eshift;

    if(srstart > srend)
    {
        if(force)
        {
            /* Apply the maximal possible shift, if force is set. */

            if(sshift < 0)
            {
                /*
                ** If we are contracting the Slice from the start,
                ** move the start just before the end.
                */

                srstart = srend - 1;

                sshift = slice->Start - srstart;
            }
            else
            {
                /*
                ** If the Slice still has a negative length,
                ** try to move the end.
                */

                if(eshift < 0)
                {
                    srend = srstart + 1;

                    eshift = srend - slice->End;
                }

                *Pfshift = (slice->Strand >= 0) ? eshift : sshift;
                *Ptshift = (slice->Strand >= 0) ? sshift : eshift;
            }
        }

        if(srstart > srend)
        {
            /* If the Slice still has a negative length, return NULL. */

            ajDebug("ensSliceFetchExpandedSlice requires the Slice start %d "
                    "to be less than the Slice end %d coordinate.\n",
                    srstart, srend);

            return ajFalse;
        }
    }

    if(slice->Adaptor)
        ensSliceadaptorFetchBySeqregionIdentifier(slice->Adaptor,
                                                  srid,
                                                  srstart,
                                                  srend,
                                                  slice->Strand,
                                                  Pslice);
    else
        *Pslice = ensSliceNew(slice->Adaptor,
                              slice->Seqregion,
                              srstart,
                              srend,
                              slice->Strand);

    return ajTrue;
}




/* @funcstatic sliceConstrainToSeqregion **************************************
**
** Constrain an Ensembl Slice to its Ensembl Sequence Region element.
**
** The caller is responsible for deleting the Ensembl Projection Segment.
**
** @cc Bio::EnsEMBL::Slice::_constrain_to_region
** @param [r] slice [EnsPSlice] Ensembl Slice
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment or NULL
** @@
******************************************************************************/

static EnsPProjectionsegment sliceConstrainToSeqregion(EnsPSlice slice)
{
    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice nslice = NULL;

    if(!slice)
    {
        ajDebug("sliceConstrainToSeqregion requires an Ensembl Slice.\n");

        return NULL;
    }

    /* Return NULL, if this Slice does not overlap with its Sequence Region. */

    if((slice->Start > ensSeqregionGetLength(slice->Seqregion)) ||
       (slice->End < 1))
        return NULL;

    /*
    ** If the Slice has negative coordinates or coordinates exceeding the
    ** length of the Sequence Region the Slice needs shrinking to the
    ** defined Sequence Region.
    */

    if(slice->Start < 1)
        five = slice->Start - 1;

    if(slice->End > ensSeqregionGetLength(slice->Seqregion))
        three = ensSeqregionGetLength(slice->Seqregion) - slice->End;

    if(five || three)
        ensSliceFetchExpandedSlice(slice,
                                   five,
                                   three,
                                   ajFalse,
                                   &fshift,
                                   &tshift,
                                   &nslice);
    else
        nslice = ensSliceNewRef(slice);

    ps = ensProjectionsegmentNew(1 - five,
                                 ensSliceGetLength(slice) + three,
                                 nslice);

    ensSliceDel(&nslice);

    return ps;
}




/* @func ensSliceProject ******************************************************
**
** Project an Ensembl Slice onto another Ensembl Coordinate System.
** Projecting this Slice onto a Coordinate System that the Slice is assembled
** from is analogous to retrieving a tiling path. This method may also be used
** to project up-wards to a higher-level Ensembl Coordinate System.
**
** The caller is responsible for deleting the Ensembl Projection Segments
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::project
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [rN] csversion [const AjPStr] Ensembl Coordinate System version
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceProject(EnsPSlice slice,
                       const AjPStr csname,
                       const AjPStr csversion,
                       AjPList pss)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint length = 0;

    AjPList nrmpss = NULL;
    AjPList mrs    = NULL;

    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystem srccs      = NULL;
    EnsPCoordsystem trgcs      = NULL;
    EnsPCoordsystem nrmcs      = NULL;
    EnsPCoordsystem mrcs       = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment nrmps = NULL;
    EnsPProjectionsegment ps    = NULL;

    EnsPMapperresult mr = NULL;

    const EnsPSlice nrmslc = NULL;
    EnsPSlice newslc       = NULL;

    if(ajDebugTest("ensSliceProject"))
    {
        ajDebug("ensSliceProject\n"
                "  slice %p\n"
                "  csname '%S'\n"
                "  cvsversion '%S'\n"
                "  pss %p\n",
                slice,
                csname,
                csversion,
                pss);

        ensSliceTrace(slice, 1);
    }

    if(!slice)
    {
        ajDebug("ensSliceProject requires an Ensembl Slice.\n");

        return ajFalse;
    }

    if(!csname)
    {
        ajDebug("ensSliceProject requires an "
                "Ensembl Coordinate System name.\n");

        return ajFalse;
    }

    /*
    ** A Coordinate System version is not strictly required,
    ** since ensCoordsystemadaptorFetchByName does not require one.
    */

    if(!pss)
    {
        ajDebug("ensSliceProject requires an AJAX List of "
                "Ensembl Projection Segments.\n");

        return ajFalse;
    }

    if(!slice->Adaptor)
    {
        ajDebug("ensSliceProject requires that an Ensembl Slice Adaptor has "
                "been set in the Ensembl Slice.\n");

        return ajFalse;
    }

    if(!slice->Seqregion)
    {
        ajDebug("ensSliceProject requires that an Ensembl Sequence Region "
                "has been set in the Ensembl Slice.\n");

        return ajFalse;
    }

    dba = ensSliceadaptorGetDatabaseadaptor(slice->Adaptor);

    ama = ensRegistryGetAssemblymapperadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    /*
    ** Get the source Coordinate System, which is the
    ** Ensembl Coordinate System element of the
    ** Ensembl Sequence Region element of this Ensembl Slice.
    */

    srccs = ensSeqregionGetCoordsystem(slice->Seqregion);

    if(!srccs)
    {
        ajDebug("ensSliceProject requires that an Ensembl Coordinate System "
                "element has been set in the Ensembl Sequence Region element "
                "of the Ensembl Slice.\n");

        return ajFalse;
    }

    /* Fetch the target Coordinate System. */

    ensCoordsystemadaptorFetchByName(csa, csname, csversion, &trgcs);

    if(!trgcs)
    {
        ajDebug("ensSliceProject cannot project to an unknown "
                "Ensembl Coordinate System '%S:%S'.\n", csname, csversion);

        return ajFalse;
    }

    /*
    ** No mapping is needed if the requested Ensembl Coordinate System is the
    ** one this Ensembl Slice is based upon, but we do need to check if some
    ** of the Slice is outside of defined regions.
    */

    if(ensCoordsystemMatch(srccs, trgcs))
    {
        ajListPushAppend(pss, (void *) sliceConstrainToSeqregion(slice));

        ensCoordsystemDel(&trgcs);

        return ajTrue;
    }

    start = 1;

    /*
    ** Decompose this Slice into its symlinked components, which allows
    ** handling of haplotypes (HAPs) and pseudo-autosomal region (PARs).
    */

    nrmpss = ajListNew();

    ensSliceadaptorFetchNormalisedSliceProjection(slice->Adaptor,
                                                  slice,
                                                  nrmpss);

    while(ajListPop(nrmpss, (void **) &nrmps))
    {
        nrmslc = ensProjectionsegmentGetTrgSlice(nrmps);

        nrmcs = ensSeqregionGetCoordsystem(nrmslc->Seqregion);

        am = ensAssemblymapperadaptorFetchByCoordsystems(ama, nrmcs, trgcs);

        mrs = ajListNew();

        if(am)
            ensAssemblymapperMap(am,
                                 nrmslc->Seqregion,
                                 nrmslc->Start,
                                 nrmslc->End,
                                 nrmslc->Strand,
                                 mrs);
        else
        {
            ajDebug("ensSliceProject got no Assemblymapper -> gap\n");

            mr = MENSMAPPERGAPNEW(nrmslc->Start, nrmslc->End, 0);

            ajListPushAppend(mrs, (void *) mr);
        }

        ensAssemblymapperDel(&am);

        /* Construct a projection from the mapping results and return it. */

        while(ajListPop(mrs, (void **) &mr))
        {
            length = ensMapperresultGetLength(mr);

            /* Skip gaps. */

            if(ensMapperresultGetType(mr) == ensEMapperresultCoordinate)
            {
                mrcs = ensMapperresultGetCoordsystem(mr);

                /*
                ** If the normalised projection just ended up mapping to the
                ** same Coordinate System we were already in then we should
                ** just return the original region. This can happen for
                ** example, if we were on a PAR region on Y, which referred
                ** to X and a projection to 'toplevel' was requested.
                */

                if(ensCoordsystemMatch(mrcs, nrmcs))
                {
                    /* Trim off regions, which are not defined. */

                    ajListPushAppend(pss, (void *)
                                     sliceConstrainToSeqregion(slice));

                    /*
                    ** Delete this Ensembl Mapper Result and the rest of the
                    ** Ensembl Mapper Results including the AJAX List.
                    */

                    ensMapperresultDel(&mr);

                    while(ajListPop(mrs, (void **) &mr))
                        ensMapperresultDel(&mr);

                    ajListFree(&mrs);

                    /*
                    ** Delete this normalised Projection Segment and the rest
                    ** of the normalised Projection Segments including the
                    ** AJAX List.
                    */

                    ensProjectionsegmentDel(&nrmps);

                    while(ajListPop(nrmpss, (void **) &nrmps))
                        ensProjectionsegmentDel(&nrmps);

                    ajListFree(&nrmpss);

                    /* Finally, delete the target Coordinate System. */

                    ensCoordsystemDel(&trgcs);

                    return ajTrue;
                }
                else
                {
                    /* Create a Slice for the target Coordinate System. */

                    ensSliceadaptorFetchBySeqregionIdentifier(
                        slice->Adaptor,
                        ensMapperresultGetObjectIdentifier(mr),
                        ensMapperresultGetStart(mr),
                        ensMapperresultGetEnd(mr),
                        ensMapperresultGetStrand(mr),
                        &newslc);

                    end = start + length - 1;

                    ps = ensProjectionsegmentNew(start, end, newslc);

                    ajListPushAppend(pss, (void *) ps);

                    ensSliceDel(&newslc);
                }
            }

            start += length;

            ensMapperresultDel(&mr);
        }

        ajListFree(&mrs);

        ensProjectionsegmentDel(&nrmps);
    }

    ajListFree(&nrmpss);

    ensCoordsystemDel(&trgcs);

    return ajTrue;
}




/* @func ensSliceProjectToSlice ***********************************************
**
** Project an Ensembl Slice onto another Ensembl Slice.
** Needed for cases where multiple assembly mappings exist and a specific
** mapping is specified.
**
** The caller is responsible for deleting the Ensembl Projection Segments
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::project_to_slice
** @param [u] srcslice [EnsPSlice] Ensembl Slice
** @param [u] trgslice [EnsPSlice] Target Ensembl Slice
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceProjectToSlice(EnsPSlice srcslice,
                              EnsPSlice trgslice,
                              AjPList pss)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint length = 0;

    AjPList nrmpss = NULL;
    AjPList mrs    = NULL;

    EnsPAssemblymapper am         = NULL;
    EnsPAssemblymapperadaptor ama = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem trgcs = NULL;
    EnsPCoordsystem nrmcs = NULL;
    /*
    ** FIXME: Apparently not required.
    EnsPCoordsystem mrcs  = NULL;
    */

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment nrmps = NULL;
    EnsPProjectionsegment ps    = NULL;

    EnsPMapperresult mr = NULL;

    const EnsPSlice nrmslc = NULL;
    EnsPSlice newslc       = NULL;

    if(!srcslice)
        return ajFalse;

    if(!trgslice)
        return ajFalse;

    dba = ensSliceadaptorGetDatabaseadaptor(srcslice->Adaptor);

    ama = ensRegistryGetAssemblymapperadaptor(dba);

    /*
    ** Get the source Coordinate System, which is the
    ** Ensembl Coordinate System element of the
    ** Ensembl Sequence Region element of this Ensembl Slice.
    */

    srccs = ensSeqregionGetCoordsystem(srcslice->Seqregion);

    if(!srccs)
    {
        ajDebug("ensSliceProjectToSlice requires that an "
                "Ensembl Coordinate System element has been set in the "
                "Ensembl Sequence Region element of the source "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    /*
    ** Get the target Coordinate System, which is the
    ** Ensembl Coordinate System element of the
    ** Ensembl Sequence Region element of this Ensembl Slice.
    */

    trgcs = ensSeqregionGetCoordsystem(trgslice->Seqregion);

    if(!trgcs)
    {
        ajDebug("ensSliceProjectToSlice requires that an "
                "Ensembl Coordinate System element has been set in the "
                "Ensembl Sequence Region element of the target "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    /*
    ** Decompose this Slice into its symlinked components, which allows
    ** handling of haplotypes (HAPs) and pseudo-autosomal region (PARs).
    */

    nrmpss = ajListNew();

    ensSliceadaptorFetchNormalisedSliceProjection(srcslice->Adaptor,
                                                  srcslice,
                                                  nrmpss);

    while(ajListPop(nrmpss, (void **) &nrmps))
    {
        nrmslc = ensProjectionsegmentGetTrgSlice(nrmps);

        nrmcs = ensSeqregionGetCoordsystem(nrmslc->Seqregion);

        am = ensAssemblymapperadaptorFetchByCoordsystems(ama, nrmcs, trgcs);

        mrs = ajListNew();

        if(am)
            ensAssemblymapperMapToSeqregion(am,
                                            nrmslc->Seqregion,
                                            nrmslc->Start,
                                            nrmslc->End,
                                            nrmslc->Strand,
                                            trgslice->Seqregion,
                                            mrs);
        else
        {
            ajDebug("ensSliceProjectToSlice got no Assemblymapper -> gap\n");

            mr = MENSMAPPERGAPNEW(nrmslc->Start, nrmslc->End, 0);

            ajListPushAppend(mrs, (void *) mr);
        }

        ensAssemblymapperDel(&am);

        /* Construct a projection from the mapping results and return it. */

        while(ajListPop(mrs, (void **) &mr))
        {
            length = ensMapperresultGetLength(mr);

            /* Skip gaps. */

            if(ensMapperresultGetType(mr) == ensEMapperresultCoordinate)
            {
                /*
                ** AJB: The variable mrcs, set below, is not used
                ** elsewhere.
                ** FIXME: Needs fixing appropriately.
                ** mrcs = ensMapperresultGetCoordsystem(mr);
                */

                /* Create a Slice for the target Coordinate System. */

                ensSliceadaptorFetchBySeqregionIdentifier(
                    srcslice->Adaptor,
                    ensMapperresultGetObjectIdentifier(mr),
                    ensMapperresultGetStart(mr),
                    ensMapperresultGetEnd(mr),
                    ensMapperresultGetStrand(mr),
                    &newslc);

                end = start + length - 1;

                ps = ensProjectionsegmentNew(start, end, newslc);

                ajListPushAppend(pss, (void *) ps);

                ensSliceDel(&newslc);
            }

            start += length;

            ensMapperresultDel(&mr);
        }

        ajListFree(&mrs);

        ensProjectionsegmentDel(&nrmps);
    }

    ajListFree(&nrmpss);

    ensCoordsystemDel(&trgcs);

    /*
    ** Delete the Ensembl Assembly Mapper Adaptor cache,
    ** as the next mapping may include a different set.
    */

    ensAssemblymapperadaptorCacheClear(ama);

    return ajTrue;
}




/* @func ensSliceFetchAllAttributes *******************************************
**
** Fetch all Ensembl Attributes for an Ensembl Slice.
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Slice::get_all_Attributes
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [rN] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchAllAttributes(EnsPSlice slice,
                                  const AjPStr code,
                                  AjPList attributes)
{
    if(!slice)
        return ajFalse;

    if(!attributes)
        return ajFalse;

    if(!slice->Seqregion)
    {
        ajDebug("ensSliceFetchAllAttributes cannot fetch Ensembl Attributes "
                "for a Slice without a Sequence Region.\n");

        return ajFalse;
    }

    ensSeqregionFetchAllAttributes(slice->Seqregion, code, attributes);

    return ajTrue;
}




/* @func ensSliceFetchAllRepeatfeatures ***************************************
**
** Fetch all Ensembl Repeat Features on an Ensembl Slice.
** The caller is responsible for deleting the Ensembl Repeat Fetures before
** deleting the AJAX List.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] anname [const AjPStr] Ensembl Analysis name
** @param [r] rctype [AjPStr] Ensembl Repeat Consensus type
** @param [r] rcclass [AjPStr] Ensembl Repeat Consensus class
** @param [r] rcname [AjPStr] Ensembl Repeat Consensus name
** @param [u] rfs [AjPList] AJAX List of Ensembl Repeat Features
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceFetchAllRepeatfeatures(EnsPSlice slice,
                                      const AjPStr anname,
                                      AjPStr rctype,
                                      AjPStr rcclass,
                                      AjPStr rcname,
                                      AjPList rfs)
{
    EnsPRepeatfeatureadaptor rfa = NULL;

    if(ajDebugTest("ensSliceFetchAllRepeatfeatures"))
        ajDebug("ensSliceFetchAllRepeatfeatures\n"
                "  slice %p\n"
                "  anname '%S'\n"
                "  rctype '%S'\n"
                "  rcclass '%S'\n"
                "  rcname '%S'\n"
                "  rfs %p\n",
                slice,
                anname,
                rctype,
                rcclass,
                rcname,
                rfs);

    if(!slice)
        return ajFalse;

    if(!slice->Adaptor)
    {
        ajDebug("ensSliceFetchAllRepeatfeatures cannot get Repeat Features "
                "without a Slice Adaptor attached to the Slice.\n");

        return ajFalse;
    }

    rfa = ensRegistryGetRepeatfeatureadaptor(slice->Adaptor->Adaptor);

    return ensRepeatfeatureadaptorFetchAllBySlice(
        rfa,
        slice,
        anname,
        rctype,
        rcclass,
        rcname,
        rfs);
}




/* @func ensSliceFetchAllSequenceEdits ****************************************
**
** Fetch all Ensembl Sequence Edits of an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Sequence Edits before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::SequenceAdaptor::_rna_edit
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] ses [AjPList] AJAX List of Ensembl Sequence Edits
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** Ensembl Sequence Edits are Ensembl Attributes, which codes have to be
** defined in the static const char *sliceSequenceEditCode array.
******************************************************************************/

AjBool ensSliceFetchAllSequenceEdits(EnsPSlice slice,
                                     AjPList ses)
{
    register ajuint i = 0;

    AjPList attributes = NULL;

    AjPStr code = NULL;

    EnsPAttribute at = NULL;

    EnsPSequenceEdit se = NULL;

    if(!slice)
        return ajFalse;

    if(!ses)
        return ajFalse;

    code = ajStrNew();

    attributes = ajListNew();

    for(i = 0; sliceSequenceEditCode[i]; i++)
    {
        ajStrAssignC(&code, sliceSequenceEditCode[i]);

        ensSliceFetchAllAttributes(slice, code, attributes);
    }

    while(ajListPop(attributes, (void **) &at))
    {
        se = ensSequenceEditNewA(at);

        ajListPushAppend(ses, (void *) se);

        ensAttributeDel(&at);
    }

    ajListFree(&attributes);

    ajStrDel(&code);

    return ajTrue;
}




/* @datasection [EnsPSliceadaptor] Slice Adaptor ******************************
**
** Functions for manipulating Ensembl Slice Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SliceAdaptor CVS Revision: 1.104
**
** @nam2rule Sliceadaptor
**
******************************************************************************/

/* 1 << 26 = 64 MiB */

static ajuint sliceadaptorCacheMaxBytes = 1 << 26;

/* 1 << 16 = 64 Ki  */

static ajuint sliceadaptorCacheMaxCount = 1 << 16;

static ajuint sliceadaptorCacheMaxSize = 0;




/* @funcstatic sliceadaptorCacheReference *************************************
**
** Wrapper function to reference an Ensembl Slice
** from an Ensembl Cache.
**
** @param [u] value [void*] Ensembl Slice
**
** @return [void*] Ensembl Slice or NULL
** @@
******************************************************************************/

static void* sliceadaptorCacheReference(void *value)
{
    if(!value)
        return NULL;

    return (void *) ensSliceNewRef((EnsPSlice) value);
}




/* @funcstatic sliceadaptorCacheDelete ****************************************
**
** Wrapper function to delete (or de-reference) an Ensembl Slice
** from an Ensembl Cache.
**
** @param [u] value [void**] Ensembl Slice address
**
** @return [void]
** @@
** When deleting from the Cache, this function also removes and deletes the
** Ensembl Slice from the name cache, which is based on a
** conventional AJAX Table.
******************************************************************************/

static void sliceadaptorCacheDelete(void **value)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjPStr newkey = NULL;
    AjPStr oldkey = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPSlice oldslice = NULL;
    EnsPSlice *Pslice  = NULL;

    if(!value)
        return;

    /*
    ** Synchronise the deletion of this Sequence Region from the
    ** identifier cache, which is based on an Ensembl (LRU) Cache,
    ** with the name cache, based on a conventional AJAX Table,
    ** both in the Sequence Adaptor.
    */

    Pslice = (EnsPSlice *) value;

    if(!*Pslice)
        return;

    if((*Pslice)->Adaptor && (*Pslice)->Adaptor->CacheByName)
    {
        cs = ensSeqregionGetCoordsystem((*Pslice)->Seqregion);

        start  = (*Pslice)->Start;
        end    = (*Pslice)->End;
        strand = (*Pslice)->Strand;

        if((start == 1) &&
           (end == ensSeqregionGetLength((*Pslice)->Seqregion)) &&
           (strand == 1))
        {
            start  = 0;
            end    = 0;
            strand = 0;
        }

        /* Remove from the name cache. */

        newkey = ajFmtStr("%S:%S:%S:%d:%d:%d",
                          ensCoordsystemGetName(cs),
                          ensCoordsystemGetVersion(cs),
                          ensSeqregionGetName((*Pslice)->Seqregion),
                          start, end, strand);

        oldslice = (EnsPSlice)
            ajTableRemoveKey((*Pslice)->Adaptor->CacheByName,
                             (const void *) newkey,
                             (void **) &oldkey);

        /* Delete the old and new key data. */

        ajStrDel(&oldkey);
        ajStrDel(&newkey);

        /* Delete the value data. */

        ensSliceDel(&oldslice);
    }

    ensSliceDel(Pslice);

    return;
}




/* @funcstatic sliceadaptorCacheSize ******************************************
**
** Wrapper function to determine the memory size of an Ensembl Slice
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Slice
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

static ajulong sliceadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensSliceGetMemsize((const EnsPSlice) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Slice Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Slice Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSliceadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPSliceadaptor] Ensembl Slice Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensSliceadaptorNew ***************************************************
**
** Default constructor for an Ensembl Slice Adaptor.
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
** @see ensRegistryGetSliceadaptor
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSliceadaptor] Ensembl Slice Adaptor or NULL
** @@
******************************************************************************/

EnsPSliceadaptor ensSliceadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPSliceadaptor adaptor = NULL;

    if(!dba)
        return NULL;

    AJNEW0(adaptor);

    adaptor->Adaptor = dba;

    adaptor->CacheByIdentifier = ensCacheNew(
        ensECacheTypeAlphaNumeric,
        sliceadaptorCacheMaxBytes,
        sliceadaptorCacheMaxCount,
        sliceadaptorCacheMaxSize,
        sliceadaptorCacheReference,
        sliceadaptorCacheDelete,
        sliceadaptorCacheSize,
        (void* (*)(const void* key)) NULL,
        (AjBool (*)(const void* value)) NULL,
        ajFalse,
        "Slice");

    adaptor->CacheByName = ajTablestrNewLen(0);

    return adaptor;
}




/* @func ensSliceadaptorDel ***************************************************
**
** Default destructor for an Ensembl Slice Adaptor.
** This function also clears the internal Sequence Region caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Padaptor [EnsPSliceadaptor*] Ensembl Slice Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensSliceadaptorDel(EnsPSliceadaptor *Padaptor)
{
    EnsPSliceadaptor pthis = NULL;

    if(!Padaptor)
        return;

    if(!*Padaptor)
        return;

    if(ajDebugTest("ensSliceadaptorDel"))
        ajDebug("ensSliceadaptorDel\n"
                "  *Padaptor %p\n",
                *Padaptor);

    pthis = *Padaptor;

    /*
    ** Clear the identifier cache, which is based on an Ensembl LRU Cache.
    ** Clearing the Ensembl LRU Cache automatically clears the name cache
    ** via sliceadaptorCacheDelete.
    */

    ensCacheDel(&pthis->CacheByIdentifier);

    ajTableFree(&pthis->CacheByName);

    AJFREE(pthis);

    *Padaptor = NULL;

    return;
}




/* @func ensSliceadaptorGetDatabaseadaptor ************************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Slice Adaptor.
**
** @param [r] adaptor [const EnsPSliceadaptor] Ensembl Slice Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSliceadaptorGetDatabaseadaptor(
    const EnsPSliceadaptor adaptor)
{
    if(!adaptor)
        return NULL;

    return adaptor->Adaptor;
}




/* @func ensSliceadaptorCacheInsert *******************************************
**
** Insert an Ensembl Slice into the Slice Adaptor-internal cache.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceadaptorCacheInsert(EnsPSliceadaptor adaptor, EnsPSlice* Pslice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjPStr ikey = NULL;
    AjPStr nkey = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPSlice slice1 = NULL;
    EnsPSlice slice2 = NULL;

    if(!adaptor)
        return ajFalse;

    if(!Pslice)
        return ajFalse;

    if(!*Pslice)
        return ajFalse;

    if(ajDebugTest("ensSliceadaptorCacheInsert"))
    {
        ajDebug("ensSliceadaptorCacheInsert\n"
                "  adaptor %p\n"
                "  *Pslice %p\n",
                adaptor,
                *Pslice);

        ensSliceTrace(*Pslice, 1);
    }

    cs = ensSeqregionGetCoordsystem((*Pslice)->Seqregion);

    start  = (*Pslice)->Start;
    end    = (*Pslice)->End;
    strand = (*Pslice)->Strand;

    /* For Slices that cover entire Sequence Regions zero all coordinates. */

    if((start == 1) &&
       (end == ensSeqregionGetLength((*Pslice)->Seqregion)) &&
       (strand == 1))
    {
        start  = 0;
        end    = 0;
        strand = 0;
    }

    /* Search the identifier cache. */

    ikey = ajFmtStr("%u:%d:%d:%d",
                    ensSeqregionGetIdentifier((*Pslice)->Seqregion),
                    start, end, strand);

    slice1 = (EnsPSlice) ensCacheFetch(adaptor->CacheByIdentifier,
                                       (void *) ikey);

    /* Search the name cache. */

    nkey = ajFmtStr("%S:%S:%S:%d:%d:%d",
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs),
                    ensSeqregionGetName((*Pslice)->Seqregion),
                    start, end, strand);

    slice2 = (EnsPSlice) ajTableFetch(adaptor->CacheByName,
                                      (const void *) nkey);

    if((!slice1) && (!slice2))
    {
        /*
        ** None of the caches returned an identical Ensembl Slice so add this
        ** one to both caches. The Ensembl LRU Cache automatically references
        ** the Ensembl Slice via the sliceadaptorCacheReference function,
        ** while the AJAX Table-based cache needs manual referencing.
        */

        ensCacheStore(adaptor->CacheByIdentifier,
                      (void *) ikey,
                      (void **) Pslice);

        ajTablePut(adaptor->CacheByName,
                   (void *) ajStrNewS(nkey),
                   (void *) ensSliceNewRef(*Pslice));
    }

    if(slice1 && slice2 && (slice1 == slice2))
    {
        /*
        ** Both caches returned the same Ensembl Slice so delete it and
        ** return a pointer to the one already in the cache.
        */

        ensSliceDel(Pslice);

        *Pslice = ensSliceNewRef(slice2);
    }

    if(slice1 && slice2 && (slice1 != slice2))
        ajDebug("ensSliceadaptorCacheInsert detected Slices in the "
                "identifier '%S' and name '%S' cache with "
                "different addresses (%p and %p).\n",
                ikey, nkey, slice1, slice2);

    if(slice1 && (!slice2))
        ajDebug("ensSliceadaptorCacheInsert detected a Slice in "
                "the identifier, but not in the name cache.\n");

    if((!slice1) && slice2)
        ajDebug("ensSliceadaptorCacheInsert detected a Slice in "
                "the name, but not in the identifier cache.\n");

    ensSliceDel(&slice1);

    ajStrDel(&ikey);
    ajStrDel(&nkey);

    return ajTrue;
}




/* @func ensSliceadaptorFetchBySeqregionIdentifier ****************************
**
** Fetch an Ensembl Slice by an Ensembl Sequence Region identifier.
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] srid [ajuint] Ensembl Sequence Region identifier
** @param [r] srstart [ajint] Start coordinate
** @param [r] srend [ajint] End coordinate
** @param [r] srstrand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchBySeqregionIdentifier(EnsPSliceadaptor adaptor,
                                                 ajuint srid,
                                                 ajint srstart,
                                                 ajint srend,
                                                 ajint srstrand,
                                                 EnsPSlice *Pslice)
{
    AjPStr key = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensSliceadaptorFetchBySeqregionIdentifier"))
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier\n"
                "  adaptor %p\n"
                "  srid %u\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  Pslice %p\n",
                adaptor,
                srid,
                srstart,
                srend,
                srstrand,
                Pslice);

    if(!adaptor)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if(!srid)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier requires an "
                "Ensembl Sequence Region identifier.\n");

        return ajFalse;
    }

    if(!Pslice)
        return ajFalse;

    /* Check the cache first. */

    key = ajFmtStr("%u:%d:%d:%d", srid, srstart, srend, srstrand);

    *Pslice = (EnsPSlice)
        ensCacheFetch(adaptor->CacheByIdentifier, (void *) key);

    ajStrDel(&key);

    if(*Pslice)
        return ajTrue;

    sra = ensRegistryGetSeqregionadaptor(adaptor->Adaptor);

    ensSeqregionadaptorFetchByIdentifier(sra, srid, &sr);

    if(!sr)
    {
        ajDebug("ensSliceadaptorFetchBySeqregionIdentifier could not load an "
                "Ensembl Sequence Region for identifier %d.\n", srid);

        return ajTrue;
    }

    if(!srstart)
        srstart = 1;

    if(!srend)
        srend = ensSeqregionGetLength(sr);

    if(srstart > (srend + 1))
        ajFatal("ensSliceadaptorFetchBySeqregionIdentifier requires the Slice "
                "start %d to be less than or equal to the end %d + 1.",
                srstart, srend);

    if(!srstrand)
        srstrand = 1;

    *Pslice = ensSliceNew(adaptor, sr, srstart, srend, srstrand);

    ensSliceadaptorCacheInsert(adaptor, Pslice);

    ensSeqregionDel(&sr);

    return ajTrue;
}




/* @func ensSliceadaptorFetchByRegion *****************************************
**
** Fetch an Ensembl Slice by region information. At a minimum the name of an
** Ensembl Sequence Region must be provided. If no Coordinate System name is
** provided then a Slice of the highest ranked Coordinate System with a
** matching Sequence Region name will be returned.
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [rN] csname [const AjPStr] Ensembl CoordinateSystem name
** @param [rN] csversion [const AjPStr] Ensembl CoordinateSystem version
** @param [r] srname [const AjPStr] Ensembl Sequence Region name
** @param [r] srstart [ajint] Start coordinate
** @param [r] srend [ajint] End coordinate
** @param [r] srstrand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
**
** NOTE: The Ensembl Core Perl API performs some fuzzy matching, if no exact
** match for the provided Sequence Region name can be found. This allows clones
** to be fetched even when their sequence version is not known.
** For example ensSliceadaptorFetchByRegion(..., 'clone', 'AC008066', ...)
** will retrieve the Sequence Region with name 'AC008066.4'.
**
** The fuzzy matching can be turned off by setting the $no_fuzz argument to a
** true value.
**
** This has not been implemented here.
******************************************************************************/

AjBool ensSliceadaptorFetchByRegion(EnsPSliceadaptor adaptor,
                                    const AjPStr csname,
                                    const AjPStr csversion,
                                    const AjPStr srname,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    EnsPSlice *Pslice)
{
    AjPStr key = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensSliceadaptorFetchByRegion"))
        ajDebug("ensSliceadaptorFetchByRegion\n"
                "  adaptor %p\n"
                "  csname '%S'\n"
                "  csversion '%S'\n"
                "  srname '%S'\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  Pslice %p\n",
                adaptor,
                csname,
                csversion,
                srname,
                srstart,
                srend,
                srstrand,
                Pslice);

    if(!adaptor)
    {
        ajDebug("ensSliceadaptorFetchByRegion requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if(!(srname && ajStrGetLen(srname)))
    {
        ajDebug("ensSliceadaptorFetchByRegion requires an "
                "Ensembl Sequence Region name.\n");

        return ajFalse;
    }

    if(!Pslice)
        return ajFalse;

    /*
    ** Initially, search the cache, which can only return an Ensembl Slice,
    ** which is associated with a name and version of a regular Ensembl
    ** Coordinate System. For requests specifying the top-level Coordinate
    ** System or no Coordinate System at all the database needs to be queried
    ** for the Sequence Region associated with the Coordinate System of the
    ** highest rank. However, all Slices will be inserted into the cache with
    ** their true Coordinate System, keeping at least the memory requirements
    ** minimal.
    */

    key = ajFmtStr("%S:%S:%S:%d:%d:%d",
                   csname,
                   csversion,
                   srname,
                   srstart,
                   srend,
                   srstrand);

    *Pslice = (EnsPSlice) ajTableFetch(adaptor->CacheByName,
                                       (const void *) key);

    ajStrDel(&key);

    if(*Pslice)
    {
        /*
        ** For any object returned by the AJAX Table the reference counter
        ** needs to be incremented manually.
        */

        ensSliceNewRef(*Pslice);

        return ajTrue;
    }

    /* Load the Ensembl Coordinate System if a name has been provided. */

    if(csname && ajStrGetLen(csname))
    {
        csa = ensRegistryGetCoordsystemadaptor(adaptor->Adaptor);

        ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);

        if(!cs)
        {
            ajDebug("ensSliceadaptorFetchByRegion could not load an Ensembl "
                    "Coordinate System for name '%S' and version '%S'.\n",
                    csname, csversion);

            return ajTrue;
        }
    }

    /* Load the Ensembl Sequence Region. */

    sra = ensRegistryGetSeqregionadaptor(adaptor->Adaptor);

    ensSeqregionadaptorFetchByName(sra, cs, srname, &sr);

    if(!sr)
    {
        ajDebug("ensSliceadaptorFetchByRegion could not load an Ensembl "
                "Sequence Region for name '%S' and Coordinate System "
                "identifier %d.\n", srname, ensCoordsystemGetIdentifier(cs));

        ensCoordsystemTrace(cs, 1);

        ensCoordsystemDel(&cs);

        return ajTrue;
    }

    if(!srstart)
        srstart = 1;

    if(!srend)
        srend = ensSeqregionGetLength(sr);

    if(srstart > (srend + 1))
        ajFatal("ensSliceadaptorFetchByRegion requires the Slice start %d "
                "to be less than or equal to the end coordinate %d + 1.",
                srstart, srend);

    if(!srstrand)
        srstrand = 1;

    *Pslice = ensSliceNew(adaptor, sr, srstart, srend, srstrand);

    ensSliceadaptorCacheInsert(adaptor, Pslice);

    ensCoordsystemDel(&cs);

    ensSeqregionDel(&sr);

    return ajTrue;
}




/* @func ensSliceadaptorFetchByName *******************************************
**
** Fetch an Ensembl Slice by name, which consists of the following
** colon-separated fields:
**   Coordinate System name,
**   Coordinate System version,
**   Sequence Region start,
**   Sequence Region end and
**   Sequence Region strand
**
** csname:csversion:srname:srstart:srend:srstrand
**
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] name [const AjPStr] Ensembl Slice name
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchByName(EnsPSliceadaptor adaptor,
                                  const AjPStr name,
                                  EnsPSlice *Pslice)
{
    ajint start  = 0;
    ajint end    = 0;
    ajint strand = 0;

    AjBool debug = AJFALSE;
    AjBool error = AJFALSE;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr srstart   = NULL;
    AjPStr srend     = NULL;
    AjPStr srstrand  = NULL;

    AjPStrTok token = NULL;

    debug = ajDebugTest("ensSliceadaptorFetchByName");

    if(debug)
        ajDebug("ensSliceadaptorFetchByName\n"
                "  adaptor %p\n"
                "  name '%S'\n"
                "  Pslice %p\n",
                adaptor,
                name,
                Pslice);

    if(!adaptor)
        return ajFalse;

    if(!(name && ajStrGetLen(name)))
        return ajFalse;

    if(!Pslice)
        return ajFalse;

    /* Check the Slice Adaptor-internal cache for the name, first. */

    *Pslice = (EnsPSlice) ajTableFetch(adaptor->CacheByName,
                                       (const void *) name);

    if(*Pslice)
    {
        /*
        ** For any object returned by the AJAX Table the reference counter
        ** needs to be incremented manually.
        */

        ensSliceNewRef(*Pslice);

        return ajTrue;
    }

    token = ajStrTokenNewC(name, ":");

    csname    = ajStrNew();
    csversion = ajStrNew();
    srname    = ajStrNew();
    srstart   = ajStrNew();
    srend     = ajStrNew();
    srstrand  = ajStrNew();

    if(ajStrTokenNextParseNoskip(&token, &csname) &&
       ajStrTokenNextParseNoskip(&token, &csversion) &&
       ajStrTokenNextParseNoskip(&token, &srname) &&
       ajStrTokenNextParseNoskip(&token, &srstart) &&
       ajStrTokenNextParseNoskip(&token, &srend) &&
       ajStrTokenNextParseNoskip(&token, &srstrand))
    {
        /*
        ** Convert AJAX Strings into AJAX integers.
        ** Empty AJAX Strings are not converted and default to 0, to
        ** discriminate them from out of range and other errors reported by
        ** ajStrToInt.
        */

        if(ajStrGetLen(srstart))
            if(!ajStrToInt(srstart, &start))
            {
                ajWarn("ensSliceadaptorFetchByName could not convert '%S' "
                       "into an integer.",
                       srstart);

                error = ajTrue;
            }

        if(ajStrGetLen(srend))
            if(!ajStrToInt(srend, &end))
            {
                ajWarn("ensSliceadaptorFetchByName could not convert '%S' "
                       "into an integer.",
                       srend);

                error = ajTrue;
            }

        if(ajStrGetLen(srstrand))
            if(!ajStrToInt(srstrand, &strand))
            {
                ajWarn("ensSliceadaptorFetchByName could not convert '%S' "
                       "into an integer.",
                       srstrand);

                error = ajTrue;
            }

        if(debug)
            ajDebug("ensSliceadaptorFetchByName parsed name '%S' into:\n"
                    "  csname    '%S'\n"
                    "  csversion '%S'\n"
                    "  srname    '%S'\n"
                    "  srstart   '%S' %d\n"
                    "  srend     '%S' %d\n"
                    "  srstrand  '%S' %d\n",
                    name,
                    csname,
                    csversion,
                    srname,
                    srstart,  start,
                    srend,    end,
                    srstrand, strand);

        if(!error)
            ensSliceadaptorFetchByRegion(adaptor,
                                         csname,
                                         csversion,
                                         srname,
                                         start,
                                         end,
                                         strand,
                                         Pslice);
    }
    else
        ajDebug("ensSliceadaptorFetchByName got a malformed Ensembl Slice "
                "name '%S', should be something like "
                "chromosome:NCBI36:X:1000000:2000000:1\n"
                "  csname    '%S'\n"
                "  csversion '%S'\n"
                "  srname    '%S'\n"
                "  srstart   '%S'\n"
                "  srend     '%S'\n"
                "  srstrand  '%S'\n",
                name,
                csname,
                csversion,
                srname,
                srstart,
                srend,
                srstrand);

    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    ajStrDel(&srstart);
    ajStrDel(&srend);
    ajStrDel(&srstrand);

    ajStrTokenDel(&token);

    return ajTrue;
}




/* @func ensSliceadaptorFetchBySlice ******************************************
**
** Fetch an Ensembl Slice by an Ensembl Slice in relative coordinates.
**
** This function is experimental, but the advantage of going through the
** Slice Adaptor would be that a (Sub-)Slice would be registered in the
** Slice Adaptor-internal cache, which in turn reduces memory consumption.
** The caller is responsible for deleting the Ensembl Slice.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchBySlice(EnsPSliceadaptor adaptor,
                                   EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   EnsPSlice *Pslice)
{
    ajuint srid    = 0;
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    if(!adaptor)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(start > (end + 1))
    {
        ajDebug("ensSliceadaptorFetchBySlice requires the start coordinate %d "
                "to be less than the end coordinate %d + 1.\n",
                start, end);

        return ajFalse;
    }

    if(!strand)
        strand = 1;

    if(!Pslice)
        return ajFalse;

    /* Transform relative into absolute Sequence Region coordinates. */

    /*
    **
    **           s           e
    **     SS     \    +1   /     SE
    **  1    \     |-------|     /    length
    **   \    |------Slice------|    /
    **    |--------Seqregion--------|
    **        |------Slice------|
    **       /     |-------|     \
    **     SS     /    -1   \     SE
    **           e           s
    **
    */

    srid = ensSeqregionGetIdentifier(slice->Seqregion);

    if(slice->Strand >= 0)
    {
        srstart = slice->Start + start - 1;
        srend   = slice->Start + end   - 1;
    }
    else
    {
        srstart = slice->End - end   + 1;
        srend   = slice->End - start + 1;
    }

    srstrand = slice->Strand * strand;

    ensSliceadaptorFetchBySeqregionIdentifier(adaptor,
                                              srid,
                                              srstart,
                                              srend,
                                              srstrand,
                                              Pslice);

    if(!*Pslice)
        return ajFalse;

    /* Adjust the sequence in case the Slice has one associated. */

    if(slice->Sequence)
        ensSliceFetchSubSequenceStr(slice,
                                    start,
                                    end,
                                    strand,
                                    &((*Pslice)->Sequence));

    return ajTrue;
}




/* @func ensSliceadaptorFetchByFeature ****************************************
**
** Retrieve an Ensembl Slice around an Ensembl Feature.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] feature [const EnsPFeature] Ensembl Feature
** @param [r] flank [ajint] Flanking region in base pair coordinates
** @param [wP] Pslice [EnsPSlice*] Ensembl Slice address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** All this really does is return a resized version of the Slice that the
** Feature is already on. Note that Slices returned from this function are
** always on the forward strand of the Sequence Region regardless of the
** strandedness of the Feature passed in.
******************************************************************************/

AjBool ensSliceadaptorFetchByFeature(EnsPSliceadaptor adaptor,
                                     const EnsPFeature feature,
                                     ajint flank,
                                     EnsPSlice *Pslice)
{
    ajuint srid   = 0;
    ajint srstart = 0;
    ajint srend   = 0;

    EnsPSlice slice = NULL;

    if(ajDebugTest("ensSliceadaptorFetchByFeature"))
    {
        ajDebug("ensSliceadaptorFetchByFeature\n"
                "  adaptor %p\n"
                "  feature %p\n"
                "  flank %d\n"
                "  Pslice %p\n",
                adaptor,
                feature,
                flank,
                Pslice);

        ensFeatureTrace(feature, 1);
    }

    if(!adaptor)
    {
        ajDebug("ensSliceadaptorFetchByFeature requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if(!feature)
    {
        ajDebug("ensSliceadaptorFetchByFeature requires an "
                "Ensembl Feature.\n");

        return ajFalse;
    }

    if(!Pslice)
        return ajFalse;

    slice = ensFeatureGetSlice(feature);

    if(!slice)
    {
        ajDebug("ensSliceadaptorFetchByFeature requires an "
                "Ensembl Slice attached to the Ensembl Feature.\n");

        return ajFalse;
    }

    srid = ensSliceGetSeqregionIdentifier(slice);

    /* Convert the Feature Slice coordinates to Sequence Region coordinates. */

    if(slice->Strand >= 0)
    {
        srstart = slice->Start + ensFeatureGetStart(feature) - 1;
        srend   = slice->Start + ensFeatureGetEnd(feature)   - 1;
    }
    else
    {
        srstart = slice->End - ensFeatureGetEnd(feature)   + 1;
        srend   = slice->End - ensFeatureGetStart(feature) + 1;
    }

    ensSliceadaptorFetchBySeqregionIdentifier(adaptor,
                                              srid,
                                              srstart - flank,
                                              srend   + flank,
                                              1,
                                              Pslice);

    return ajTrue;
}




/* @func ensSliceadaptorFetchNormalisedSliceProjection ************************
**
** Fetch a normalised projection for an Ensembl Slice.
** This function returns an AJAX List of Ensembl Projection Segments where
** symbolic links exist for the given Ensembl Slice.
** The caller is resposible for deleting the Ensembl Projection Segments before
** deleting the AJAX List.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] pss [AjPList] AJAX List of Ensembl Projection Segments
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSliceadaptorFetchNormalisedSliceProjection(EnsPSliceadaptor adaptor,
                                                     EnsPSlice slice,
                                                     AjPList pss)
{
    ajuint srid   = 0;
    ajuint maxlen = 0;
    ajuint diff   = 0;
    ajuint start  = 1;

    AjBool debug = AJFALSE;

    AjPList aes  = NULL;
    AjPList haps = NULL;
    AjPList pars = NULL;
    AjPList syms = NULL;
    AjPList mrs  = NULL;

    AjPStr source = NULL;
    AjPStr target = NULL;

    EnsPAssemblyexception ae         = NULL;
    EnsPAssemblyexceptionadaptor aea = NULL;

    EnsPMapper mapper = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice = NULL;
    EnsPSlice exslice = NULL;

    debug = ajDebugTest("ensSliceadaptorFetchNormalisedSliceProjection");

    if(debug)
    {
        ajDebug("ensSliceadaptorFetchNormalisedSliceProjection\n"
                "  adaptor %p\n"
                "  slice %p\n"
                "  pss %p\n",
                adaptor,
                slice,
                pss);

        ensSliceTrace(slice, 1);
    }

    if(!adaptor)
    {
        ajDebug("ensSliceadaptorFetchNormalisedSliceProjection requires an "
                "Ensembl Slice Adaptor.\n");

        return ajFalse;
    }

    if(!slice)
    {
        ajDebug("ensSliceadaptorFetchNormalisedSliceProjection requires an "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    if(!pss)
    {
        ajDebug("ensSliceadaptorFetchNormalisedSliceProjection requires an "
                "AJAX List of Ensembl Projection Segments.\n");

        return ajFalse;
    }

    aea = ensRegistryGetAssemblyexceptionadaptor(adaptor->Adaptor);

    srid = ensSliceGetSeqregionIdentifier(slice);

    haps = ajListNew();
    pars = ajListNew();
    aes  = ajListNew();

    ensAssemblyexceptionadaptorFetchAllBySeqregionIdentifier(aea, srid, aes);

    while(ajListPop(aes, (void **) &ae))
    {
        /*
        ** Select all overlapping pseudo-autosomal regions (PARs) and all
        ** haplotype regions (HAPs) if any.
        */

        switch(ensAssemblyexceptionGetType(ae))
        {
            case ensEAssemblyexceptionTypePAR:

                if(((ajint) ensAssemblyexceptionGetSeqregionStart(ae)
                    <= slice->End) &&
                   ((ajint) ensAssemblyexceptionGetSeqregionEnd(ae)
                    >= slice->Start))
                    ajListPushAppend(pars, (void *) ae);
                else
                    ensAssemblyexceptionDel(&ae);

                break;

            case ensEAssemblyexceptionTypeHAP:

                ajListPushAppend(haps, (void *) ae);

                break;

            default:

                ajWarn("ensSliceadaptorFetchNormalisedSliceProjection got "
                       "unexpected Assembly Exception type %d.\n",
                       ensAssemblyexceptionGetType(ae));

                ensAssemblyexceptionDel(&ae);
        }
    }

    ajListFree(&aes);

    if((!ajListGetLength(haps)) && (!ajListGetLength(pars)))
    {
        /*
        ** There are no haplotypes and no pseudo-autosomal regions,
        ** so return just a Projection Segment for this Slice.
        */

        if(debug)
            ajDebug("ensSliceadaptorFetchNormalisedSliceProjection\n"
                    "  found no Assembly Exceptions overlapping this "
                    "Slice.\n");

        ajListFree(&haps);
        ajListFree(&pars);

        ajListPushAppend(
            pss,
            (void *) ensProjectionsegmentNew(
                1,
                ensSliceGetLength(slice),
                slice));

        return ajTrue;
    }

    if(ajListGetLength(haps) > 1)
        ajFatal("ensSliceadaptorFetchNormalisedSliceProjection does not "
                "support more than one haplotypes yet.");

    syms = ajListNew();

    if(ajListGetLength(haps) == 1)
    {
        ajListPop(haps, (void **) &ae);

        ensSliceadaptorFetchBySeqregionIdentifier(
            adaptor,
            srid,
            0,
            0,
            0,
            &srslice);

        ensSliceadaptorFetchBySeqregionIdentifier(
            adaptor,
            ensAssemblyexceptionGetExcRegionIdentifier(ae),
            0,
            0,
            0,
            &exslice);

        /*
        ** The lengths of haplotype and reference sequences in the database
        ** may be different. We want to use the maximum possible length for
        ** the mapping between the two Coordinate Systems.
        */

        maxlen = (ensSliceGetLength(srslice) >= ensSliceGetLength(exslice)) ?
            ensSliceGetLength(srslice) : ensSliceGetLength(exslice);

        /*
        ** The inserted Sequence Region can differ in length, but mapped
        ** sections need to be of same lengths.
        */

        diff = ensAssemblyexceptionGetExcRegionEnd(ae) -
            ensAssemblyexceptionGetSeqregionEnd(ae);

        /*
        ** We want the region of the haplotype inverted, which means that we
        ** want the two regions of the Slice that are not covered by the
        ** haplotype as Projection Segments.
        **
        ** Haplotype:                   *******
        ** Slice:                 -------------------
        ** Projection Segments:   ++++++       ++++++
        */

        ajListPushAppend(
            syms,
            (void *) ensAssemblyexceptionNew(
                (EnsPAssemblyexceptionadaptor) NULL,
                0,
                srid,
                1,
                ensAssemblyexceptionGetSeqregionStart(ae) - 1,
                ensAssemblyexceptionGetExcRegionIdentifier(ae),
                1,
                ensAssemblyexceptionGetExcRegionStart(ae) - 1,
                1,
                ensEAssemblyexceptionTypeNULL));

        ajListPushAppend(
            syms,
            (void *) ensAssemblyexceptionNew(
                (EnsPAssemblyexceptionadaptor) NULL,
                0,
                srid,
                ensAssemblyexceptionGetSeqregionEnd(ae) + 1,
                maxlen - diff,
                ensAssemblyexceptionGetExcRegionIdentifier(ae),
                ensAssemblyexceptionGetExcRegionEnd(ae) + 1,
                maxlen,
                1,
                ensEAssemblyexceptionTypeNULL));

        ensSliceDel(&srslice);
        ensSliceDel(&exslice);

        ensAssemblyexceptionDel(&ae);
    }

    ajListFree(&haps);

    /*
    ** The ajListPushlist function adds all nodes from the second AJAX List to
    ** the first and deletes the second.
    */

    ajListPushlist(syms, &pars);

    source = ajStrNewC("sym");
    target = ajStrNewC("org");

    mapper = ensMapperNew(source,
                          target,
                          ensSliceGetCoordsystem(slice),
                          ensSliceGetCoordsystem(slice));

    while(ajListPop(syms, (void **) &ae))
    {
        ensMapperAddCoordinates(mapper,
                                srid,
                                ensAssemblyexceptionGetSeqregionStart(ae),
                                ensAssemblyexceptionGetSeqregionEnd(ae),
                                1,
                                ensAssemblyexceptionGetExcRegionIdentifier(ae),
                                ensAssemblyexceptionGetExcRegionStart(ae),
                                ensAssemblyexceptionGetExcRegionEnd(ae));

        ensAssemblyexceptionDel(&ae);
    }

    ajListFree(&syms);

    mrs = ajListNew();

    ensMapperMapCoordinates(mapper,
                            srid,
                            slice->Start,
                            slice->End,
                            slice->Strand,
                            source,
                            mrs);

    ajStrDel(&source);
    ajStrDel(&target);

    while(ajListPop(mrs, (void **) &mr))
    {
        switch(ensMapperresultGetType(mr))
        {
            case ensEMapperresultCoordinate:

                if(debug)
                    ajDebug("ensSliceadaptorFetchNormalisedSliceProjection\n"
                            "  Ensembl Mapper Result coordinate "
                            "%u:%d:%d:%d\n",
                            ensMapperresultGetObjectIdentifier(mr),
                            ensMapperresultGetStart(mr),
                            ensMapperresultGetEnd(mr),
                            ensMapperresultGetStrand(mr));

                ensSliceadaptorFetchBySeqregionIdentifier(
                    adaptor,
                    ensMapperresultGetObjectIdentifier(mr),
                    ensMapperresultGetStart(mr),
                    ensMapperresultGetEnd(mr),
                    ensMapperresultGetStrand(mr),
                    &exslice);

                ajListPushAppend(
                    pss,
                    (void *) ensProjectionsegmentNew(
                        start,
                        start + ensMapperresultGetLength(mr) - 1,
                        exslice));

                ensSliceDel(&exslice);

                break;

            case ensEMapperresultGap:

                if(debug)
                    ajDebug("ensSliceadaptorFetchNormalisedSliceProjection\n"
                            "  Ensembl Mapper Result gap %d:%d\n",
                            ensMapperresultGetGapStart(mr),
                            ensMapperresultGetGapEnd(mr));

                exslice = ensSliceNew(adaptor,
                                      slice->Seqregion,
                                      ensMapperresultGetGapStart(mr),
                                      ensMapperresultGetGapEnd(mr),
                                      slice->Strand);

                ajListPushAppend(
                    pss,
                    (void *) ensProjectionsegmentNew(
                        start,
                        start + ensMapperresultGetGapLength(mr) - 1,
                        exslice));

                ensSliceDel(&exslice);

                break;

            default:

                ajWarn("ensSliceadaptorFetchNormalisedSliceProjection got "
                       "an unexpected Ensembl Mapper Result type %d.\n",
                       ensMapperresultGetType(mr));
        }

        start += ensMapperresultGetLength(mr);

        ensMapperresultDel(&mr);
    }

    ajListFree(&mrs);

    ensMapperDel(&mapper);

    return ajTrue;
}




/* @func ensSliceadaptorFetchAll **********************************************
**
** Fetch all Ensembl Slices representing Sequence Regions of a given
** Coordinate System.
**
** @param [u] adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @param [r] csname [const AjPStr] Ensembl Coordinate System name
** @param [r] csversion [const AjPStr] Ensembl Coordinate System version
** @param [r] nonreference [AjBool] Include non-reference Sequence Regions
** @param [r] duplicates [AjBool] Include duplicate Sequence Regions
** @param [u] slices [AjPList] An AJAX List of Ensembl Slices
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** The Coordinate System name may be the name of an actual Coordinate System
** or an alias like 'seqlevel' or 'toplevel'.
** If the Coordinate System name provided is 'toplevel', all non-redundant,
** top-level Slices are returned. The Coordinate System version will be
** ignored in that case.
** Non-reference Slices reflect alternative haplotype assemblies.
** Examples for human NCBI36 would be c5_H2, c6_COX, c6_QBL and c22_H2.
** If the duplicate option is not set and a pseudo-autosomal region (PAR) is
** at the beginning of the Sequence Region, then the resulting Slice will not
** start at position 1, so that coordinates retrieved from this Slice might
** not be what was expected.
******************************************************************************/

AjBool ensSliceadaptorFetchAll(EnsPSliceadaptor adaptor,
                               const AjPStr csname,
                               const AjPStr csversion,
                               AjBool nonreference,
                               AjBool duplicates,
                               AjPList slices)
{
    AjBool import = AJFALSE;
    AjBool result = AJFALSE;

    AjPList aes = NULL;
    AjPList pss = NULL;
    AjPList srs = NULL;

    EnsPAssemblyexception ae         = NULL;
    EnsPAssemblyexceptionadaptor aea = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    EnsPSlice srslice = NULL;
    EnsPSlice psslice = NULL;

    if(!adaptor)
        return ajFalse;

    if(!csname)
        return ajFalse;

    if(!csversion)
        return ajFalse;

    if(!slices)
        return ajFalse;

    if(!duplicates)
        aea = ensRegistryGetAssemblyexceptionadaptor(adaptor->Adaptor);

    csa = ensRegistryGetCoordsystemadaptor(adaptor->Adaptor);

    sra = ensRegistryGetSeqregionadaptor(adaptor->Adaptor);

    ensCoordsystemadaptorFetchByName(csa, csname, csversion, &cs);

    if(!cs)
    {
        ajWarn("ensSliceadaptorFetchAll could not retrieve Coordinate System "
               "for name '%S' and version '%S'.\n", csname, csversion);

        return ajTrue;
    }

    /* Retrieve the Sequence Regions from the database. */

    srs = ajListNew();

    ensSeqregionadaptorFetchAllByCoordsystem(sra, cs, srs);

    while(ajListPop(srs, (void **) &sr))
    {
        if(nonreference)
            import = ajTrue;
        else
        {
            if(!ensSeqregionIsNonReference(sr, &result))
            {
                ajDebug("ensSliceadaptorFetchAll could not call "
                        "ensSeqRegionIsNonReference successfully.\n");

                ensSeqregionTrace(sr, 1);

                ensSeqregionDel(&sr);

                continue;
            }

            import = !result;
        }

        if(import)
        {
            /* Create a new Slice spanning the entire Sequence Region. */

            srslice = ensSliceNew(adaptor,
                                  sr,
                                  1,
                                  ensSeqregionGetLength(sr),
                                  1);

            if(duplicates)
            {
                /*
                ** Duplicate regions are allowed, so no further checks are
                ** required.
                */

                ajListPushAppend(slices, (void *) srslice);
            }
            else
            {
                /*
                ** Test if this Slice *could* have a duplicate region by
                ** checking for an Assembly Exception for the
                ** Ensembl Sequence Region element of this Ensembl Slice.
                */

                aes = ajListNew();

                ensAssemblyexceptionadaptorFetchAllBySeqregionIdentifier(
                    aea,
                    ensSeqregionGetIdentifier(sr),
                    aes);

                if(ajListGetLength(aes))
                {
                    /*
                    ** This Slice may have duplicate regions, so de-reference
                    ** symlinked assembly regions and remove any Slices,
                    ** which have a symlink, because these are duplicates.
                    ** Replace them with any symlinked Slices based on the
                    ** same Sequence Region and Coordinate System as the
                    ** original Slice.
                    */

                    pss = ajListNew();

                    ensSliceadaptorFetchNormalisedSliceProjection(adaptor,
                                                                  srslice,
                                                                  pss);

                    while(ajListPop(pss, (void **) &ps))
                    {
                        psslice = ensProjectionsegmentGetTrgSlice(ps);

                        if(ensSeqregionMatch(psslice->Seqregion,
                                             srslice->Seqregion))
                            ajListPushAppend(slices,
                                             (void *) ensSliceNewRef(psslice));

                        ensProjectionsegmentDel(&ps);
                    }

                    ajListFree(&pss);

                    ensSliceDel(&srslice);
                }
                else
                {
                    /* No duplicates for this Slice. */

                    ajListPushAppend(slices, (void *) srslice);
                }

                while(ajListPop(aes, (void **) &ae))
                    ensAssemblyexceptionDel(&ae);

                ajListFree(&aes);
            }
        }

        ensSeqregionDel(&sr);
    }

    ajListFree(&srs);

    ensCoordsystemDel(&cs);

    return ajTrue;
}




/* @datasection [EnsPRepeatmaskedslice] Repeat-Masked Slice *******************
**
** Functions for manipulating Ensembl Repeat-Masked Slice objects
**
** @cc Bio::EnsEMBL::RepeatMaskedSlice CVS Revision: 1.13
**
** @nam2rule Repeatmaskedslice
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Repeat-Masked Slice by pointer.
** It is the responsibility of the user to first destroy any previous
** Repeat-Masked Slice. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPRepeatmaskedslice]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
** @argrule Ref object [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @valrule * [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @fcategory new
******************************************************************************/




/* @func ensRepeatmaskedsliceNew **********************************************
**
** Default constructor for an Ensembl Repeat-Masked Slice.
**
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [uN] annames [AjPList] AJAX List of Ensembl Analysis name
**                               AJAX Strings
** @param [uN] masking [AjPTable] AJAX Table to override masking types
**
** @return [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
** @@
******************************************************************************/

EnsPRepeatmaskedslice ensRepeatmaskedsliceNew(EnsPSlice slice,
                                              AjPList annames,
                                              AjPTable masking)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    EnsERepeatMaskType *Pmsktyp = NULL;

    AjIList iter = NULL;

    AjPStr key = NULL;

    EnsPRepeatmaskedslice rmslice = NULL;

    if(ajDebugTest("ensRepeatmaskedsliceNew"))
    {
        ajDebug("ensRepeatmaskedsliceNew\n"
                "  slice %p\n"
                "  annames %p\n"
                "  masking %p\n",
                slice,
                annames,
                masking);

        ensSliceTrace(slice, 1);
    }

    if(!slice)
        return NULL;

    AJNEW0(rmslice);

    rmslice->Slice = ensSliceNewRef(slice);

    /* Copy the AJAX List of Ensembl Analysis name AJAX Strings. */

    if(annames)
    {
        rmslice->AnalysisNames = ajListstrNew();

        iter = ajListIterNew(annames);

        while(!ajListIterDone(iter))
        {
            key = (AjPStr) ajListIterGet(iter);

            if(key && ajStrGetLen(key))
                ajListPushAppend(rmslice->AnalysisNames,
                                 (void *) ajStrNewS(key));
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX Table of AJAX String key and AJAX Enum masking types. */

    if(masking)
    {
        rmslice->Masking = ajTablestrNewLen(0);

        ajTableToarrayKeysValues(masking, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
        {
            key = ajStrNewS((AjPStr) keyarray[i]);

            AJNEW0(Pmsktyp);

            *Pmsktyp = *((EnsERepeatMaskType *) valarray[i]);

            ajTablePut(rmslice->Masking, (void *) key, (void *) Pmsktyp);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    rmslice->Use = 1;

    return rmslice;
}




/* @func ensRepeatmaskedsliceNewObj *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @return [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
** @@
******************************************************************************/

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewObj(
    EnsPRepeatmaskedslice object)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    EnsERepeatMaskType *Pmsktyp = NULL;

    AjIList iter = NULL;

    AjPStr key = NULL;

    EnsPRepeatmaskedslice rmslice = NULL;

    if(!object)
        return NULL;

    AJNEW0(rmslice);

    rmslice->Slice = ensSliceNewRef(object->Slice);

    /* Copy the AJAX List of Ensembl Analysis name AJAX Strings. */

    if(object->AnalysisNames)
    {
        rmslice->AnalysisNames = ajListstrNew();

        iter = ajListIterNew(object->AnalysisNames);

        while(!ajListIterDone(iter))
        {
            key = (AjPStr) ajListIterGet(iter);

            if(key && ajStrGetLen(key))
                ajListPushAppend(rmslice->AnalysisNames, ajStrNewS(key));
        }

        ajListIterDel(&iter);
    }

    /* Copy the AJAX Table of AJAX String key and AJAX Enum masking types. */

    if(object->Masking)
    {
        rmslice->Masking = ajTablestrNewLen(0);

        ajTableToarrayKeysValues(object->Masking, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
        {
            key = ajStrNewS((AjPStr) keyarray[i]);

            AJNEW0(Pmsktyp);

            *Pmsktyp = *((EnsERepeatMaskType *) valarray[i]);

            ajTablePut(rmslice->Masking, (void *) key, (void *) Pmsktyp);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    rmslice->Use = 1;

    return rmslice;
}




/* @func ensRepeatmaskedsliceNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
**
** @return [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice or NULL
** @@
******************************************************************************/

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewRef(
    EnsPRepeatmaskedslice rmslice)
{
    if(!rmslice)
        return NULL;

    rmslice->Use++;

    return rmslice;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Repeat-Masked Slice.
**
** @fdata [EnsPRepeatmaskedslice]
** @fnote None
**
** @nam3rule Del Destroy (free) a Repeat-Masked Slice object
**
** @argrule * Prmslice [EnsPRepeatmaskedslice*] Repeat-Masked Slice object
**                                              address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic repeatmaskedsliceClearMasking **********************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Repeat-Masked Slice-
** internal AJAX Table. This function deletes the AJAX String key data and the
** AJAX Enum value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX Enum value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void repeatmaskedsliceClearMasking(void **key,
                                          void **value,
                                          void *cl)
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

    ajStrDel((AjPStr*) key);

    AJFREE(*value);

    return;
}




/* @func ensRepeatmaskedsliceDel **********************************************
**
** Default Ensembl Repeat-Masked Slice destructor.
**
** @param [d] Prmslice [EnsPRepeatmaskedslice*] Ensembl Repeat-Masked Slice
**                                              address
**
** @return [void]
** @@
******************************************************************************/

void ensRepeatmaskedsliceDel(EnsPRepeatmaskedslice *Prmslice)
{
    EnsPRepeatmaskedslice pthis = NULL;

    if(!Prmslice)
        return;

    if(!*Prmslice)
        return;

    pthis = *Prmslice;

    pthis->Use--;

    if(pthis->Use)
    {
        *Prmslice = NULL;

        return;
    }

    ensSliceDel(&pthis->Slice);

    /* Clear and delete the AJAX List of Ensembl Analysis name AJAX Strings. */

    ajListstrFreeData(&pthis->AnalysisNames);

    /* Clear and delete the AJAX Table. */

    if(pthis->Masking)
    {
        ajTableMapDel(pthis->Masking, repeatmaskedsliceClearMasking, NULL);

        ajTableFree(&pthis->Masking);
    }

    AJFREE(pthis);

    *Prmslice = NULL;

    return;
}




/* @func ensRepeatmaskedsliceFetchSequenceSeq *********************************
**
** Fetch the (masked) sequence of an Ensembl Repeat-Masked Slice as
** AJAX Sequence.
**
** @param [u] rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
** @param [r] mtype [EnsERepeatMaskType] Repeat masking type
** @param [wP] Psequence [AjPSeq*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensRepeatmaskedsliceFetchSequenceSeq(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(!rmslice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    name = ajStrNew();

    ensSliceFetchName(rmslice->Slice, &name);

    sequence = ajStrNew();

    ensRepeatmaskedsliceFetchSequenceStr(rmslice, mtype, &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&sequence);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensRepeatmaskedsliceFetchSequenceStr *********************************
**
** Fetch the (masked) sequence for an Ensembl Repeat-Masked Slice as
** AJAX String.
**
** @cc Bio::EnsEMBL::RepeatMaskedSlice::seq
** @cc Bio::EnsEMBL::Slice::_mask_features
** @param [u] rmslice [EnsPRepeatmaskedslice] Ensembl Repeat-Masked Slice
** @param [r] mtype [EnsERepeatMaskType] Repeat masking type
** @param [wP] Psequence [AjPStr*] Sequence String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: The Bio::EnsEMBL::RepeatMaskedSlice::subseq method has not been
** implemented, because the Ensembl Base Adaptor has no global Ensembl Feature
** cache. Handling of polymorphic Bio::EnsEMBL::Feature subclasses is simple in
** object-oriented Perl, but quite complex in ANSI C. The Perl method
** retrieves Bio::EnsEMBL::RepeatFeature objects for larger blocks
** (2^18 or 256 Ki base pairs) for more efficient caching of subsequent
** requests.
** However, the performance hit should be minimal, as the relational database
** management system (RDBMS) caches the underlying queries. It is also possible
** to fetch the (masked) sequence for the whole Repeat-Masked Slice and
** subsequently extract the sub sequence with correspoinding AJAX functions.
** Therefore, the Bio::EnsEMBL::RepeatMaskedSlice::subseq method offers
** only small benefits over Bio::EnsEMBL::RepeatMaskedSlice::seq.
** As a consequence, the private Bio::EnsEMBL::Slice::_mask_features method
** has been merged into this function to simplify parameter checking if both,
** sequence and Repeat Features are fetched from the same Slice.
******************************************************************************/

AjBool ensRepeatmaskedsliceFetchSequenceStr(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPStr *Psequence)
{
    ajint start = 0;
    ajint end   = 0;

    EnsERepeatMaskType msktyp   = ensERepeatMaskTypeNULL;
    EnsERepeatMaskType *Pmsktyp = NULL;

    AjIList iter = NULL;
    AjPList rfs  = NULL;

    AjPStr key = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPRepeatconsensus rc = NULL;

    EnsPRepeatfeature rf         = NULL;
    EnsPRepeatfeatureadaptor rfa = NULL;

    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("ensRepeatmaskedsliceFetchSequenceStr"))
        ajDebug("ensRepeatmaskedsliceFetchSequenceStr\n"
                "  rmslice %p\n"
                "  *Psequence %p\n"
                "  mtype %d\n",
                rmslice,
                *Psequence,
                mtype);

    if(!rmslice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    if((mtype <= ensERepeatMaskTypeNULL) || (mtype > ensERepeatMaskTypeHard))
    {
        ajDebug("ensRepeatmaskedsliceFetchSequenceStr got unsupported "
                "masking type %d\n", mtype);

        return ajFalse;
    }

    if(!rmslice->Slice)
    {
        ajDebug("ensRepeatmaskedsliceFetchSequenceStr got an "
                "Ensembl Repeat-Masked Slice without an "
                "Ensembl Slice.\n");

        return ajFalse;
    }

    ensSliceFetchSequenceStr(rmslice->Slice, Psequence);

    /* Sanity check */

    if(ensSliceGetLength(rmslice->Slice) != ajStrGetLen(*Psequence))
        ajWarn("ensRepeatmaskedsliceFetchSequenceStr got Slice of length %u, "
               "but DNA sequence of length %u.\n",
               ensSliceGetLength(rmslice->Slice),
               ajStrGetLen(*Psequence));

    sa = ensSliceGetAdaptor(rmslice->Slice);

    dba = ensSliceadaptorGetDatabaseadaptor(sa);

    rfa = ensRegistryGetRepeatfeatureadaptor(dba);

    rfs = ajListNew();

    if(rmslice->AnalysisNames)
    {
        iter = ajListIterNew(rmslice->AnalysisNames);

        while(!ajListIterDone(iter))
        {
            key = (AjPStr) ajListIterGet(iter);

            ensRepeatfeatureadaptorFetchAllBySlice(rfa,
                                                   rmslice->Slice,
                                                   key,
                                                   (AjPStr) NULL,
                                                   (AjPStr) NULL,
                                                   (AjPStr) NULL,
                                                   rfs);
        }

        ajListIterDel(&iter);
    }
    else
        ensRepeatfeatureadaptorFetchAllBySlice(rfa,
                                               rmslice->Slice,
                                               (AjPStr) NULL,
                                               (AjPStr) NULL,
                                               (AjPStr) NULL,
                                               (AjPStr) NULL,
                                               rfs);

    while(ajListPop(rfs, (void **) &rf))
    {
        /*
        ** NOTE: Since this function merges methods from the RepeatMaskedSlice
        ** and Slice classes the Repeat Features are automatically fetched
        ** from the same Slice and do not need transforming back onto the
        ** Slice that has been passed into the second method. It is also not
        ** necessary to check whether the Features overlap with the Slice.
        **
        ** For the AJAX String masking functions, the start and end coordinates
        ** of Repeat Features partly outside the Slice need to be trimmed.
        */

        feature = ensRepeatfeatureGetFeature(rf);

        start = (ensFeatureGetStart(feature) >= 1) ?
            ensFeatureGetStart(feature) :
            1;

        end = (ensFeatureGetEnd(feature) <=
               (ajint) ensSliceGetLength(rmslice->Slice)) ?
            ensFeatureGetEnd(feature) :
            (ajint) ensSliceGetLength(rmslice->Slice);

        if(rmslice->Masking)
        {
            rc = ensRepeatfeatureGetRepeatconsensus(rf);

            /* Get the masking type for the Repeat Consensus type. */

            key = ajFmtStr("repeat_type_%S", ensRepeatconsensusGetType(rc));

            Pmsktyp = (EnsERepeatMaskType *) ajTableFetch(rmslice->Masking,
                                                          (const void *) key);

            if(Pmsktyp)
                msktyp = *Pmsktyp;

            ajStrDel(&key);

            /* Get the masking type for the Repeat Consensus class. */

            key = ajFmtStr("repeat_class_%S", ensRepeatconsensusGetClass(rc));

            Pmsktyp = (EnsERepeatMaskType *) ajTableFetch(rmslice->Masking,
                                                          (const void *) key);

            if(Pmsktyp)
                msktyp = *Pmsktyp;

            ajStrDel(&key);

            /* Get the masking type for the Repeat Consensus name. */

            key = ajFmtStr("repeat_name_%S", ensRepeatconsensusGetName(rc));

            Pmsktyp = (EnsERepeatMaskType *) ajTableFetch(rmslice->Masking,
                                                          (const void *) key);

            if(Pmsktyp)
                msktyp = *Pmsktyp;

            ajStrDel(&key);
        }

        /* Set the default masking type, if no other type has been defined. */

        if(!msktyp)
            msktyp = mtype;

        /* Mask the DNA sequence. */

        switch(msktyp)
        {
            case ensERepeatMaskTypeNULL:

                break;

            case ensERepeatMaskTypeSoft:

                ajStrFmtLowerSub(Psequence, start, end);

                break;

            case ensERepeatMaskTypeHard:

                ajStrMaskRange(Psequence, start, end, 'N');

                break;

            default:

                ajDebug("ensRepeatmaskedsliceFetchSequenceStr got unsupported "
                        "masking type %d\n", msktyp);
        }

        ensRepeatfeatureDel(&rf);
    }

    ajListFree(&rfs);

    return ajTrue;
}

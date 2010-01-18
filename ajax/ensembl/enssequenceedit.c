/******************************************************************************
** @source Ensembl Sequence Edit functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.1 $
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

#include "enssequenceedit.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection enssequenceedit ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSequenceEdit] Sequence Edit ******************************
**
** Functions for manipulating Ensembl Sequence Edit objects
**
** @cc Bio::EnsEMBL::SeqEdit CVS Revision: 1.3
**
** @nam2rule Sequence Edit
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Edit by pointer.
** It is the responsibility of the user to first destroy any previous
** Sequence Edit. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSequenceEdit]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
** @nam4rule A Constructor wit Ensembl Attribute object
**
** @argrule Obj object [EnsPSequenceEdit] Ensembl Sequence Edit
** @argrule Ref object [EnsPSequenceEdit] Ensembl Sequence Edit
**
** @valrule * [EnsPSequenceEdit] Ensembl Sequence Edit
**
** @fcategory new
******************************************************************************/




/* @func ensSequenceEditNew ***************************************************
**
** Default Ensembl Sequence Edit constructor.
**
** @cc Bio::EnsEMBL:SeqEdit::new
** @param [u] code [AjPStr] Code
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [u] altseq [AjPStr] Alternative sequence
** @param [r] start [ajuint] Start coordinate
** @param [r] end [ajuint] End coordinate
**
** @return [EnsPSequenceEdit] Ensembl Sequence Edit or NULL
** @@
******************************************************************************/

EnsPSequenceEdit ensSequenceEditNew(AjPStr code,
                                    AjPStr name,
                                    AjPStr description,
                                    AjPStr altseq,
                                    ajuint start,
                                    ajuint end)
{
    AjPStr value = NULL;
    
    EnsPSequenceEdit se = NULL;
    
    if(!altseq)
    {
	ajDebug("ensSequenceEditNew requires an alternate sequence.\n");
	
	return NULL;
    }
    
    if(start > (end + 1))
    {
	ajDebug("ensSequenceEditNew start %d must be less than or equal to "
		"end %d + 1.\n", start, end);
	
	return NULL;
    }
    
    if(start < 1)
    {
	ajDebug("ensSequenceEditNew start %d must be greater than or "
		"equal to 1.\n", start);
	
	return NULL;
    }
    
    value = ajFmtStr("%u %u %S", start, end, altseq);
    
    AJNEW0(se);
    
    se->Attribute = ensAttributeNew(code, name, description, value);
    
    if(altseq)
	se->AltSeq = ajStrNewRef(altseq);
    else
	se->AltSeq = ajStrNew();
    
    se->Start = start;
    
    se->End = end;
    
    se->Use = 1;
    
    ajStrDel(&value);
    
    return se;
}




/* @func ensSequenceEditNewA **************************************************
**
** Default Ensembl Sequence Edit constructor based on an Ensembl Attribute.
**
** @cc Bio::EnsEMBL:SeqEdit::new
** @param [r] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [EnsPSequenceEdit] Ensembl Sequence Edit or NULL
** @@
******************************************************************************/

EnsPSequenceEdit ensSequenceEditNewA(EnsPAttribute attribute)
{
    ajint start = 0;
    ajint end   = 0;
    
    AjPStr altseq = NULL;
    
    EnsPSequenceEdit se = NULL;
    
    if(!attribute)
	return NULL;
    
    altseq = ajStrNew();
    
    ajFmtScanS(attribute->Value, "%d %d %S", &start, &end, &altseq);
    
    if(start > (end + 1))
    {
	ajDebug("ensSequenceEditNewA start %d must be less than or equal to "
		"end %d + 1 in Ensembl Attribute value element '%S'.\n",
		start, end, attribute->Value);
	
	return NULL;
    }
    
    if(start < 1)
    {
	ajDebug("ensSequenceEditNewA start %d in Ensembl Attribute value "
		"element '%S' must be greater than or equal to 1.\n",
		start, attribute->Value);
	
	return NULL;
    }
    
    if(end < 0)
    {
	ajDebug("ensSequenceEditNewA end %d in Ensembl Attribute value "
		"element '%S' must be greater than or equal to 0.\n",
		end, attribute->Value);
	
	return NULL;
    }
    
    AJNEW0(se);
    
    se->Attribute = ensAttributeNewRef(attribute);
    
    if(altseq)
	se->AltSeq = ajStrNewRef(altseq);
    else
	se->AltSeq = ajStrNew();
    
    se->Start = start;
    
    se->End = end;
    
    se->Use = 1;
    
    ajStrDel(&altseq);
    
    return se;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Sequence Edits.
**
** @fdata [EnsPSequenceEdit]
** @fnote None
**
** @nam3rule Del Destroy (free) a Sequence Edit object
**
** @argrule * Pse [EnsPSequenceEdit*] Sequence Edit object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSequenceEditDel ***************************************************
**
** Default destructor for an Ensembl Sequence Edit.
**
** @param [d] Pse [EnsPSequenceEdit*] Ensembl Sequence Edit address
**
** @return [void]
** @@
******************************************************************************/

void ensSequenceEditDel(EnsPSequenceEdit *Pse)
{
    EnsPSequenceEdit pthis = NULL;
    
    if(!Pse)
	return;
    
    if(!*Pse)
	return;

    pthis = *Pse;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pse = NULL;
	
	return;
    }
    
    ensAttributeDel(&pthis->Attribute);
    
    ajStrDel(&pthis->AltSeq);
    
    AJFREE(pthis);

    *Pse = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Sequence Edit object.
**
** @fdata [EnsPSequenceEdit]
** @fnote None
**
** @nam3rule Get Return Sequence Edit attribute(s)
** @nam4rule GetAttribute Return the Ensembl Attribute
** @nam4rule GetAltSeq Return the alternate sequence
** @nam4rule GetStart Return the description
** @nam4rule GetEnd Return the value
**
** @argrule * se [const EnsPSequenceEdit] Sequence Edit
**
** @valrule Attribute [EnsPAttribute] Ensembl Attribute
** @valrule AltSeq [AjPStr] Alternate sequence
** @valrule Start [ajuint] Start coordinate
** @valrule End [ajuint] End coordinate
**
** @fcategory use
******************************************************************************/




/* @func ensSequenceEditGetAttribute ******************************************
**
** Get the Ensembl Attribute element of an Ensembl Sequence Edit.
**
** @cc Bio::EnsEMBL:SeqEdit::get_Attribute
** @param [r] se [const EnsPSequenceEdit] Ensembl Sequence Edit
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
** @@
******************************************************************************/

EnsPAttribute ensSequenceEditGetAttribute(const EnsPSequenceEdit se)
{
    if(!se)
        return NULL;
    
    return se->Attribute;
}




/* @func ensSequenceEditGetAltSeq *********************************************
**
** Get the alternative sequence element of an Ensembl Sequence Edit.
**
** @cc Bio::EnsEMBL:SeqEdit::alt_seq
** @param [r] se [const EnsPSequenceEdit] Ensembl Sequence Edit
**
** @return [AjPStr] Alternative sequence
** @@
******************************************************************************/

AjPStr ensSequenceEditGetAltSeq(const EnsPSequenceEdit se)
{
    if(!se)
        return NULL;
    
    return se->AltSeq;
}




/* @func ensSequenceEditGetStart **********************************************
**
** Get the start coordinate element of an Ensembl Sequence Edit.
**
** @cc Bio::EnsEMBL:SeqEdit::start
** @param [r] se [const EnsPSequenceEdit] Ensembl Sequence Edit
**
** @return [ajuint] Start coordinate
** @@
******************************************************************************/

ajuint ensSequenceEditGetStart(const EnsPSequenceEdit se)
{
    if(!se)
        return 0;
    
    return se->Start;
}




/* @func ensSequenceEditGetEnd ************************************************
**
** Get the end coordinate element of an Ensembl Sequence Edit.
**
** @cc Bio::EnsEMBL:SeqEdit::end
** @param [r] se [const EnsPSequenceEdit] Ensembl Sequence Edit
**
** @return [ajuint] End coordinate
** @@
******************************************************************************/

ajuint ensSequenceEditGetEnd(const EnsPSequenceEdit se)
{
    if(!se)
        return 0;
    
    return se->End;
}




/* @func ensSequenceEditGetLengthDifference ***********************************
**
** Get the length difference an Ensembl Sequence Edit would cause.
**
** @cc Bio::EnsEMBL:SeqEdit::length_diff
** @param [r] se [EnsPSequenceEdit] Ensembl Sequence Edit
**
** @return [ajuint] Length difference or 0
** @@
******************************************************************************/

ajuint ensSequenceEditGetLengthDifference(EnsPSequenceEdit se)
{
    if(!se)
	return 0;
    
    return (ajStrGetLen(se->AltSeq) - (se->End - se->Start + 1));
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Sequence Edit object.
**
** @fdata [EnsPSequenceEdit]
** @nam3rule Trace Report Ensembl Sequence Edit elements to debug file
**
** @argrule Trace attribute [const EnsPSequenceEdit] Ensembl Sequence Edit
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceEditTrace *************************************************
**
** Trace an Ensembl Sequence Edit.
**
** @param [r] se [const EnsPSequenceEdit] Ensembl Sequence Edit
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceEditTrace(const EnsPSequenceEdit se, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!se)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensSequenceEditTrace %p\n"
	    "%S  Attribute %p\n"
	    "%S  AltSeq '%S'\n"
	    "%S  Start %u\n"
	    "%S  End %u\n"
	    "%S  Use %u\n",
	    indent, se,
	    indent, se->Attribute,
	    indent, se->AltSeq,
	    indent, se->Start,
	    indent, se->End,
	    indent, se->Use);
    
    ensAttributeTrace(se->Attribute, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensSequenceEditApplyEdit *********************************************
**
** Apply an Ensembl Sequence Edit to a sequence AJAX String.
**
** @cc Bio::EnsEMBL:SeqEdit::apply_edit
** @param [r] se [EnsPSequenceEdit] Ensembl Sequence Edit
** @param [r] PSequence [AjPStr*] Sequence AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceEditApplyEdit(EnsPSequenceEdit se, AjPStr* Psequence)
{
    if(!se)
	return ajFalse;
    
    if(!Psequence)
	return ajFalse;
    
    if(!*Psequence)
	return ajFalse;
    
    if(se->Start > ajStrGetLen(*Psequence))
    {
	ajDebug("ensSequenceEditApplyEdit start position %u beyond "
		"sequence length %u.\n",
		se->Start,
		ajStrGetLen(*Psequence));
	
	return ajFalse;
    }
    
    ajStrCutRange(Psequence, se->Start - 1, se->End - 1);
    
    ajStrInsertS(Psequence, se->Start - 1, se->AltSeq);
    
    return ajTrue;
}




/* @func ensSequenceEditCompareStartAscending *********************************
**
** Comparison function to sort Ensembl Sequence Edits by their
** Start Coordinates in ascending order.
**
** @param [r] P1 [const void*] Ensembl Sequence Edit address 1
** @param [r] P2 [const void*] Ensembl Sequence Edit address 2
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

int ensSequenceEditCompareStartAscending(const void* P1, const void* P2)
{
    int value = 0;
    
    const EnsPSequenceEdit se1 = NULL;
    const EnsPSequenceEdit se2 = NULL;
    
    se1 = *(EnsPSequenceEdit const *) P1;
    
    se2 = *(EnsPSequenceEdit const *) P2;
    
    if(!se1)
    {
	ajDebug("ensSequenceEditCompareStartAscending got empty se1.\n");
	
	return 0;
    }
    
    if(!se2)
    {
	ajDebug("ensSequenceEditCompareStartAscending got empty se2.\n");
	
	return 0;
    }
    
    /*
     ajDebug("ensSequenceEditCompareStartAscending\n"
	     "  se1 %p\n"
	     "  se2 %p\n",
	     se1,
	     se2);
     
     ensSequenceEditTrace(se1, 1);
     
     ensSequenceEditTrace(se2, 1);
     */
    
    if(se1->Start < se2->Start)
        value = -1;
    
    if(se1->Start == se2->Start)
        value = 0;
    
    if(se1->Start > se2->Start)
        value = +1;
    
    return value;
}




/* @func ensSequenceEditCompareStartDescending ********************************
**
** Comparison function to sort Ensembl Sequence Edits by their
** Start Coordinates in descending order.
**
** @param [r] P1 [const void*] Ensembl Sequence Edit address 1
** @param [r] P2 [const void*] Ensembl Sequence Edit address 2
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

int ensSequenceEditCompareStartDescending(const void* P1, const void* P2)
{
    int value = 0;
    
    const EnsPSequenceEdit se1 = NULL;
    const EnsPSequenceEdit se2 = NULL;
    
    se1 = *(EnsPSequenceEdit const *) P1;
    
    se2 = *(EnsPSequenceEdit const *) P2;
    
    if(!se1)
    {
	ajDebug("ensSequenceEditCompareStartDescending got empty se1.\n");
	
	return 0;
    }
    
    if(!se2)
    {
	ajDebug("ensSequenceEditCompareStartDescending got empty se2.\n");
	
	return 0;
    }
    
    /*
     ajDebug("ensSequenceEditCompareStartDescending\n"
	     "  se1 %p\n"
	     "  se2 %p\n",
	     se1,
	     se2);
     
     ensSequenceEditTrace(se1, 1);
     
     ensSequenceEditTrace(se2, 1);
     */
    
    if(se1->Start < se2->Start)
        value = +1;
    
    if(se1->Start == se2->Start)
        value = 0;
    
    if(se1->Start > se2->Start)
        value = -1;
    
    return value;
}

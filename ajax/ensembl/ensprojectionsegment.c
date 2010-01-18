/******************************************************************************
** @source Ensembl Projection Segment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.2 $
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

#include "ensprojectionsegment.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensprojectionsegment ******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPProjectionsegment] Projection Segment ********************
**
** Functions for manipulating Ensembl Projection Segment objects
**
** @nam2rule Projectionsegment
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Projection Segment by pointer.
** It is the responsibility of the user to first destroy any previous
** Projection Segment. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProjectionsegment]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPProjectionsegment] Ensembl Projection Segment
** @argrule Ref object [EnsPProjectionsegment] Ensembl Projection Segment
**
** @valrule * [EnsPProjectionsegment] Ensembl Projection Segment
**
** @fcategory new
******************************************************************************/




/* @func ensProjectionsegmentNew **********************************************
**
** Default constructor for an Ensembl Projection Segment.
**
** @param [r] srcstart [ajuint] Source start coordinate.
** @param [r] srcend [ajuint] Source end coordinate.
** @param [r] trgslice [EnsPSlice] Target Ensembl Slice.
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment or NULL.
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNew(ajuint srcstart, ajuint srcend,
                                              EnsPSlice trgslice)
{
    EnsPProjectionsegment ps = NULL;
    
    /*
     ajDebug("ensProjectionsegmentNew\n"
	     "  srcstart %u\n"
	     "  srcend %u\n"
	     "  trgslice %p\n",
	     srcstart,
	     srcend,
	     trgslice);
     
     ensSliceTrace(trgslice, 1);
     */
    
    if(!trgslice)
        return NULL;
    
    AJNEW0(ps);
    
    ps->SrcStart = srcstart;
    
    ps->SrcEnd = srcend;
    
    ps->TrgSlice = ensSliceNewRef(trgslice);
    
    ps->Use = 1;
    
    return ps;
}




/* @func ensProjectionsegmentNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ps [EnsPProjectionsegment] Ensembl Projection Segment.
**
** @return [EnsPProjectionsegment] Ensembl Projection Segment.
** @@
******************************************************************************/

EnsPProjectionsegment ensProjectionsegmentNewRef(EnsPProjectionsegment ps)
{
    if(!ps)
	return NULL;
    
    ps->Use++;
    
    return ps;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Projection Segments.
**
** @fdata [EnsPProjectionsegment]
** @fnote None
**
** @nam3rule Del Destroy (free) an Projection Segment object
**
** @argrule * Pps [EnsPProjectionsegment*] Projection Segment object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProjectionsegmentDel **********************************************
**
** Default Ensembl Projection Segment destructor.
**
** @param [d] Pps [EnsPProjectionsegment*] Ensembl Projection Segment address.
**
** @return [void]
** @@
******************************************************************************/

void ensProjectionsegmentDel(EnsPProjectionsegment *Pps)
{
    EnsPProjectionsegment pthis = NULL;
    
    if(!Pps)
        return;
    
    if(!*Pps)
        return;

    pthis = *Pps;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pps = NULL;
	
	return;
    }
    
    ensSliceDel(&pthis->TrgSlice);
    
    AJFREE(pthis);

    *Pps = NULL;
    
    return;
}




/* @func ensProjectionsegmentGetSrcStart **************************************
**
** Get the source start element of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment.
**
** @return [ajuint] Source start coordinate.
** @@
******************************************************************************/

ajuint ensProjectionsegmentGetSrcStart(const EnsPProjectionsegment ps)
{
    if(!ps)
        return 0;
    
    return ps->SrcStart;
}




/* @func ensProjectionsegmentGetSrcEnd ****************************************
**
** Get the source end element of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment.
**
** @return [ajuint] Source end coordinate.
** @@
******************************************************************************/

ajuint ensProjectionsegmentGetSrcEnd(const EnsPProjectionsegment ps)
{
    if(!ps)
        return 0;
    
    return ps->SrcEnd;
}




/* @func ensProjectionsegmentGetTrgSlice **************************************
**
** Get the target Ensembl Slice element of an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment.
**
** @return [EnsPSlice] Target Ensembl Slice.
** @@
******************************************************************************/

EnsPSlice ensProjectionsegmentGetTrgSlice(const EnsPProjectionsegment ps)
{
    if(!ps)
        return NULL;
    
    return ps->TrgSlice;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Projection Segment object.
**
** @fdata [EnsPProjectionsegment]
** @nam3rule Trace Report Ensembl Projection Segment elements to debug file
**
** @argrule Trace ps [const EnsPProjectionsegment] Ensembl Projection Segment
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensProjectionsegmentTrace ********************************************
**
** Trace an Ensembl Projection Segment.
**
** @param [r] ps [const EnsPProjectionsegment] Ensembl Projection Segment.
** @param [r] level [ajuint] Indentation level.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensProjectionsegmentTrace(const EnsPProjectionsegment ps, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!ps)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensProjectionsegmentTrace %p\n"
	    "%S  SrcStart %d\n"
	    "%S  SrcEnd %d\n"
	    "%S  TrgSlice %p\n"
	    "%S  Use %u\n",
	    indent, ps,
	    indent, ps->SrcStart,
	    indent, ps->SrcEnd,
	    indent, ps->TrgSlice,
	    indent, ps->Use);
    
    ensSliceTrace(ps->TrgSlice, level + 1);
    
    ajStrDel(&indent);
    
    return ajTrue;
}

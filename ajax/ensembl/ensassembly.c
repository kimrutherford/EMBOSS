/******************************************************************************
** @source Ensembl Assembly functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.3 $
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

#include "ensassembly.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensassembly ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/



/* @datasection [EnsPAssembly] Assembly ***************************************
**
** Functions for manipulating Ensembl Assembly objects
**
** @nam2rule Assembly
**
******************************************************************************/



/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAssembly]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [const EnsPAssembly] Ensembl Assembly
** @argrule Ref object [EnsPAssembly] Ensembl Assembly
**
** @valrule * [EnsPAssembly] Ensembl Assembly
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyNew *******************************************************
**
** Default constructor for an Ensembl Assembly.
**
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [r] asmstart [ajuint] Assembled Sequence Region start coordinate
** @param [r] asmend [ajuint] Assembled Sequence Region end coordinate
** @param [r] cmpsrid [ajuint] Component Ensembl Sequence Region identifier
** @param [r] cmpstart [ajuint] Component Sequence Region start coordinate
** @param [r] cmpend [ajuint] Component Sequence Region end coordinate
** @param [r] orientation [ajint] Relative orientation
**
** @return [EnsPAssembly] Ensembl Assembly or NULL
** @@
******************************************************************************/

EnsPAssembly ensAssemblyNew(ajuint asmsrid,
                            ajuint asmstart,
                            ajuint asmend,
                            ajuint cmpsrid,
                            ajuint cmpstart,
                            ajuint cmpend,
                            ajint orientation)
{
    EnsPAssembly assembly = NULL;
    
    if(!asmsrid)
	return NULL;
    
    if(!cmpsrid)
	return NULL;
    
    AJNEW0(assembly);
    
    assembly->Use = 1;
    
    assembly->AssembledSeqregionId = asmsrid;
    assembly->AssembledStart       = asmstart;
    assembly->AssembledEnd         = asmend;
    assembly->ComponentSeqregionId = cmpsrid;
    assembly->ComponentStart       = cmpstart;
    assembly->ComponentEnd         = cmpend;
    assembly->Orientation          = orientation;
    
    return assembly;
}




/* @func ensAssemblyNewObj ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPAssembly] Ensembl Assembly
**
** @return [EnsPAssembly] Ensembl Assembly or NULL
** @@
******************************************************************************/

EnsPAssembly ensAssemblyNewObj(const EnsPAssembly object)
{
    EnsPAssembly assembly = NULL;
    
    if(!object)
	return NULL;
    
    AJNEW0(assembly);
    
    assembly->Use = 1;
    
    assembly->AssembledSeqregionId = object->AssembledSeqregionId;
    assembly->AssembledStart       = object->AssembledStart;
    assembly->AssembledEnd         = object->AssembledEnd;
    assembly->ComponentSeqregionId = object->ComponentSeqregionId;
    assembly->ComponentStart       = object->ComponentStart;
    assembly->ComponentEnd         = object->ComponentEnd;
    assembly->Orientation          = object->Orientation;
    
    return assembly;
}




/* @func ensAssemblyNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] assembly [EnsPAssembly] Ensembl Assembly
**
** @return [EnsPAssembly] Ensembl Assembly or NULL
** @@
******************************************************************************/

EnsPAssembly ensAssemblyNewRef(EnsPAssembly assembly)
{
    if(!assembly)
	return NULL;
    
    assembly->Use++;
    
    return assembly;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Assembly.
**
** @fdata [EnsPAssembly]
** @fnote None
**
** @nam3rule Del Destroy (free) an Assembly object
**
** @argrule * Passembly [EnsPAssembly*] Assembly object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblyDel *******************************************************
**
** Default destructor for an Ensembl Assembly.
**
** @param [d] Passembly [EnsPAssembly*] Ensembl Assembly address
**
** @return [void]
** @@
******************************************************************************/

void ensAssemblyDel(EnsPAssembly* Passembly)
{
    EnsPAssembly pthis = NULL;
    
    if(!Passembly)
	return;
    
    if(!*Passembly)
	return;

    pthis = *Passembly;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Passembly = NULL;
	
	return;
    }
    
    AJFREE(*Passembly);
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Assembly object.
**
** @fdata [EnsPAssembly]
** @fnote None
**
** @nam3rule Get Return Assembly attribute(s)
** @nam4rule GetAssembledSeqregionId Return the assembled
**                                   Ensembl Sequence Region identifier
** @nam4rule GetAssembledStart Return the assembled
**                             Ensembl Sequence Region start
** @nam4rule GetAssembledEnd Return the assembled
**                           Ensembl Sequence Region end
** @nam4rule GetComponentSeqregionId Return the component
**                                   Ensembl Sequence Region identifier
** @nam4rule GetComponentStart Return the component
**                             Ensembl Sequence Region start
** @nam4rule GetComponentEnd Return the component
**                           Ensembl Sequence Region end
** @nam4rule GetOrientation Return the relative orientation of assembled
**                          and component Ensembl Sequence Regions
**
** @argrule * assembly [const EnsPAssembly] Ensembl Assembly
**
** @valrule AssembledSeqregionId [ajuint] Ensembl Analysis Adaptor
** @valrule AssembledStart [ajuint] SQL database-internal identifier
** @valrule AssembledEnd [ajuint] Creation date
** @valrule ComponentSeqregionId [ajuint] Name
** @valrule ComponentStart [ajuint] Database name
** @valrule ComponentEnd [ajuint] Database version
** @valrule Orientation [ajint] Database file
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyGetAssembledSeqregionId ***********************************
**
** Get the assembled Ensembl Sequence Region identifier element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Assembled Ensembl Sequence Region identifier
** @@
******************************************************************************/

ajuint ensAssemblyGetAssembledSeqregionId(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->AssembledSeqregionId;
}




/* @func ensAssemblyGetAssembledStart *****************************************
**
** Get the assembled Ensembl Sequence Region start element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Assembled Ensembl Sequence Region start
** @@
******************************************************************************/

ajuint ensAssemblyGetAssembledStart(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->AssembledStart;
}




/* @func ensAssemblyGetAssembledEnd *******************************************
**
** Get the assembled Ensembl Sequence Region end element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Assembled Ensembl Sequence Region end
** @@
******************************************************************************/

ajuint ensAssemblyGetAssembledEnd(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->AssembledEnd;
}




/* @func ensAssemblyGetComponentSeqregionId ***********************************
**
** Get the component Ensembl Sequence Region identifier element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Component Ensembl Sequence Region identifier
** @@
******************************************************************************/

ajuint ensAssemblyGetComponentSeqregionId(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->ComponentSeqregionId;
}




/* @func ensComponentGetAssembledStart ****************************************
**
** Get the component Ensembl Sequence Region start element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Component Ensembl Sequence Region start
** @@
******************************************************************************/

ajuint ensAssemblyGetComponentStart(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->ComponentStart;
}




/* @func ensAssemblyGetComponentEnd *******************************************
**
** Get the component Ensembl Sequence Region end element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Component Ensembl Sequence Region end
** @@
******************************************************************************/

ajuint ensAssemblyGetComponentEnd(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->ComponentEnd;
}




/* @func ensAssemblyGetOrientation ********************************************
**
** Get the releative orientation of assembled and component
** Ensembl Sequence Regions element of an Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajint] Orientation
** @@
******************************************************************************/

ajint ensAssemblyGetOrientation(const EnsPAssembly assembly)
{
    if(!assembly)
	return 0;
    
    return assembly->Orientation;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Assembly object.
**
** @fdata [EnsPAssembly]
** @nam3rule Trace Report Ensembl Assembly elements to debug file
**
** @argrule Trace assembly [const EnsPAssembly] Ensembl Assembly
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyTrace *****************************************************
**
** Trace an Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyTrace(const EnsPAssembly assembly, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!assembly)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensAssemblyTrace %p\n"
	    "%S  Use %u\n"
	    "%S  AssembledSeqregionId %u\n"
	    "%S  AssembledStart %u\n"
	    "%S  AssembledEnd %u\n"
	    "%S  ComponentSeqregionId %u\n"
	    "%S  ComponentStart %u\n"
	    "%S  ComponentEnd %u\n"
	    "%S  Orientation %d\n",
	    indent, assembly,
	    indent, assembly->Use,
	    indent, assembly->AssembledSeqregionId,
	    indent, assembly->AssembledStart,
	    indent, assembly->AssembledEnd,
	    indent, assembly->ComponentSeqregionId,
	    indent, assembly->ComponentStart,
	    indent, assembly->ComponentEnd,
	    indent, assembly->Orientation);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @func ensAssemblyGetMemSize ************************************************
**
** Get the memory size in bytes of an Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensAssemblyGetMemSize(const EnsPAssembly assembly)
{
    ajuint size = 0;
    
    if(!assembly)
	return 0;
    
    size += (ajuint) sizeof (EnsOAssembly);
    
    return size;
}

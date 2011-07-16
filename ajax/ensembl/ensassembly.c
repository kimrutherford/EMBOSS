/* @source Ensembl Assembly functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/02/26 11:57:21 $ by $Author: mks $
** @version $Revision: 1.18 $
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




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensassembly ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPAssembly] Ensembl Assembly *******************************
**
** @nam2rule Assembly Functions for manipulating Ensembl Assembly objects
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy assembly [const EnsPAssembly] Ensembl Assembly
** @argrule Ini asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @argrule Ini asmstart [ajuint] Assembled Sequence Region start coordinate
** @argrule Ini asmend [ajuint] Assembled Sequence Region end coordinate
** @argrule Ini cmpsrid [ajuint] Component Ensembl Sequence Region identifier
** @argrule Ini cmpstart [ajuint] Component Sequence Region start coordinate
** @argrule Ini cmpend [ajuint] Component Sequence Region end coordinate
** @argrule Ini orientation [ajint] Relative orientation
** @argrule Ref assembly [EnsPAssembly] Ensembl Assembly
**
** @valrule * [EnsPAssembly] Ensembl Assembly or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [EnsPAssembly] Ensembl Assembly or NULL
** @@
******************************************************************************/

EnsPAssembly ensAssemblyNewCpy(const EnsPAssembly assembly)
{
    EnsPAssembly pthis = NULL;

    if(!assembly)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->AssembledIdentifier = assembly->AssembledIdentifier;
    pthis->AssembledStart      = assembly->AssembledStart;
    pthis->AssembledEnd        = assembly->AssembledEnd;
    pthis->ComponentIdentifier = assembly->ComponentIdentifier;
    pthis->ComponentStart      = assembly->ComponentStart;
    pthis->ComponentEnd        = assembly->ComponentEnd;
    pthis->Orientation         = assembly->Orientation;

    return pthis;
}




/* @func ensAssemblyNewIni ****************************************************
**
** Constructor for an Ensembl Assembly with initial values.
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

EnsPAssembly ensAssemblyNewIni(ajuint asmsrid,
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

    assembly->AssembledIdentifier = asmsrid;
    assembly->AssembledStart      = asmstart;
    assembly->AssembledEnd        = asmend;
    assembly->ComponentIdentifier = cmpsrid;
    assembly->ComponentStart      = cmpstart;
    assembly->ComponentEnd        = cmpend;
    assembly->Orientation         = orientation;

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
** memory allocated for an Ensembl Assembly object.
**
** @fdata [EnsPAssembly]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly object
**
** @argrule * Passembly [EnsPAssembly*] Ensembl Assembly object address
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

    AJFREE(pthis);

    *Passembly = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Assembly object.
**
** @fdata [EnsPAssembly]
**
** @nam3rule Get Return Assembly attribute(s)
** @nam4rule Assembled Return assembled Sequence Region members
** @nam5rule Identifier Return the assembled
** Ensembl Sequence Region identifier
** @nam5rule Start Return the assembled
** Ensembl Sequence Region start
** @nam5rule End Return the assembled
** Ensembl Sequence Region end
** @nam4rule Component Return component Seuqence Region members
** @nam4rule Identifier Return the component
** Ensembl Sequence Region identifier
** @nam4rule Start Return the component
** Ensembl Sequence Region start
** @nam4rule End Return the component
** Ensembl Sequence Region end
** @nam4rule Orientation Return the relative orientation of assembled
** and component Ensembl Sequence Regions
**
** @argrule * assembly [const EnsPAssembly] Ensembl Assembly
**
** @valrule Identifier [ajuint] Ensembl Sequence Region identifier or 0
** @valrule Start [ajuint] Start or 0
** @valrule End [ajuint] End or 0
** @valrule Orientation [ajint] Orientation or 0
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyGetAssembledEnd *******************************************
**
** Get the assembled Ensembl Sequence Region end element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Assembled Ensembl Sequence Region end or 0
** @@
******************************************************************************/

ajuint ensAssemblyGetAssembledEnd(const EnsPAssembly assembly)
{
    if(!assembly)
        return 0;

    return assembly->AssembledEnd;
}




/* @func ensAssemblyGetAssembledIdentifier ************************************
**
** Get the assembled Ensembl Sequence Region identifier element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Assembled Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensAssemblyGetAssembledIdentifier(const EnsPAssembly assembly)
{
    if(!assembly)
        return 0;

    return assembly->AssembledIdentifier;
}




/* @func ensAssemblyGetAssembledStart *****************************************
**
** Get the assembled Ensembl Sequence Region start element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Assembled Ensembl Sequence Region start or 0
** @@
******************************************************************************/

ajuint ensAssemblyGetAssembledStart(const EnsPAssembly assembly)
{
    if(!assembly)
        return 0;

    return assembly->AssembledStart;
}




/* @func ensAssemblyGetComponentEnd *******************************************
**
** Get the component Ensembl Sequence Region end element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Component Ensembl Sequence Region end or 0
** @@
******************************************************************************/

ajuint ensAssemblyGetComponentEnd(const EnsPAssembly assembly)
{
    if(!assembly)
        return 0;

    return assembly->ComponentEnd;
}




/* @func ensAssemblyGetComponentIdentifier ************************************
**
** Get the component Ensembl Sequence Region identifier element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Component Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensAssemblyGetComponentIdentifier(const EnsPAssembly assembly)
{
    if(!assembly)
        return 0;

    return assembly->ComponentIdentifier;
}




/* @func ensAssemblyGetComponentStart *****************************************
**
** Get the component Ensembl Sequence Region start element of an
** Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajuint] Component Ensembl Sequence Region start or 0
** @@
******************************************************************************/

ajuint ensAssemblyGetComponentStart(const EnsPAssembly assembly)
{
    if(!assembly)
        return 0;

    return assembly->ComponentStart;
}




/* @func ensAssemblyGetOrientation ********************************************
**
** Get the releative orientation of assembled and component
** Ensembl Sequence Regions element of an Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [ajint] Orientation or 0
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
**
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
            "%S  AssembledIdentifier %u\n"
            "%S  AssembledStart %u\n"
            "%S  AssembledEnd %u\n"
            "%S  ComponentIdentifier %u\n"
            "%S  ComponentStart %u\n"
            "%S  ComponentEnd %u\n"
            "%S  Orientation %d\n",
            indent, assembly,
            indent, assembly->Use,
            indent, assembly->AssembledIdentifier,
            indent, assembly->AssembledStart,
            indent, assembly->AssembledEnd,
            indent, assembly->ComponentIdentifier,
            indent, assembly->ComponentStart,
            indent, assembly->ComponentEnd,
            indent, assembly->Orientation);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Assembly object.
**
** @fdata [EnsPAssembly]
**
** @nam3rule Calculate Calculate Ensembl Assembly values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * assembly [const EnsPAssembly] Ensembl Assembly
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyCalculateMemsize ******************************************
**
** Calculate the memory size in bytes of an Ensembl Assembly.
**
** @param [r] assembly [const EnsPAssembly] Ensembl Assembly
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensAssemblyCalculateMemsize(const EnsPAssembly assembly)
{
    size_t size = 0;

    if(!assembly)
        return 0;

    size += sizeof (EnsOAssembly);

    return size;
}

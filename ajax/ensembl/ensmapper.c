/******************************************************************************
** @source Ensembl Mapper functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.15 $
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

#include "ensmapper.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static int mapperpairCompareSourceStartAscending(const void* P1,
                                                 const void* P2);

static int mapperpairCompareTargetStartAscending(const void* P1,
                                                 const void* P2);

static AjBool mapperMergePairs(EnsPMapper mapper);

static AjBool mapperSort(EnsPMapper mapper);

static AjBool mapperMapInsert(EnsPMapper mapper,
                              ajuint oid,
                              ajint start,
                              ajint end,
                              ajint strand,
                              const AjPStr type,
                              AjBool fastmap,
                              AjPList mrs);




/* @filesection ensmapper *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMapperunit] Mapper Unit **********************************
**
** Functions for manipulating Ensembl Mapper Unit objects
**
** Bio::EnsEMBL::Mapper::Unit CVS Revision: 1.9
**
** @nam2rule Mapperunit
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Mapper Unit by pointer.
** It is the responsibility of the user to first destroy any previous
** Mapper Unit. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMapperunit]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMapperunit] Ensembl Mapper Unit
** @argrule Ref object [EnsPMapperunit] Ensembl Mapper Unit
**
** @valrule * [EnsPMapperunit] Ensembl Mapper Unit
**
** @fcategory new
******************************************************************************/




/* @func ensMapperunitNew *****************************************************
**
** Default constructor for an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::new
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
**
** @return [EnsPMapperunit] Ensembl Mapper Unit or NULL
** @@
******************************************************************************/

EnsPMapperunit ensMapperunitNew(ajuint oid, ajint start, ajint end)
{
    EnsPMapperunit mu = NULL;

    if(!oid)
        return NULL;

    AJNEW0(mu);

    mu->ObjectIdentifier = oid;

    mu->Start = start;
    mu->End   = end;
    mu->Use   = 1;

    return mu;
}




/* @func ensMapperunitNewObj **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [EnsPMapperunit] Ensembl Mapper Unit or NULL
** @@
******************************************************************************/

EnsPMapperunit ensMapperunitNewObj(const EnsPMapperunit object)
{
    EnsPMapperunit mu = NULL;

    if(!object)
        return NULL;

    AJNEW0(mu);

    mu->ObjectIdentifier = object->ObjectIdentifier;

    mu->Start = object->Start;
    mu->End   = object->End;
    mu->Use   = 1;

    return mu;
}




/* @func ensMapperunitNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
**
** @return [EnsPMapperunit] Ensembl Mapper Unit or NULL
** @@
******************************************************************************/

EnsPMapperunit ensMapperunitNewRef(EnsPMapperunit mu)
{
    if(!mu)
        return NULL;

    mu->Use++;

    return mu;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Mapper Unit.
**
** @fdata [EnsPMapperunit]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Unit object
**
** @argrule * Pmu [EnsPMapperunit*] Ensembl Mapper Unit object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperunitDel *****************************************************
**
** Default Ensembl Mapper Unit destructor.
**
** @param [d] Pmu [EnsPMapperunit*] Ensembl Mapper Unit address
**
** @return [void]
** @@
******************************************************************************/

void ensMapperunitDel(EnsPMapperunit *Pmu)
{
    EnsPMapperunit pthis = NULL;

    if(!Pmu)
        return;

    if(!*Pmu)
        return;

    pthis = *Pmu;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmu = NULL;

        return;
    }

    AJFREE(pthis);

    *Pmu = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
** @fnote None
**
** @nam3rule Get Return Ensembl Mapper Unit attribute(s)
** @nam4rule GetObjectIdentifier Return the Ensembl Object identifier
** @nam4rule GetStart Return the start coordinate
** @nam4rule GetEnd Return the end coordinate
**
** @argrule * mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @valrule ObjectIdentifier [ajuint] Ensembl Object identifier
** @valrule Start [ajint] Start coordinate
** @valrule End [ajint] End coordinate
**
** @fcategory use
******************************************************************************/




/* @func ensMapperunitGetObjectIdentifier *************************************
**
** Get the Ensembl Object identifier element of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::id
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajuint] Ensembl Object identifier
** @@
******************************************************************************/

ajuint ensMapperunitGetObjectIdentifier(const EnsPMapperunit mu)
{
    if(!mu)
        return 0;

    return mu->ObjectIdentifier;
}




/* @func ensMapperunitGetStart ************************************************
**
** Get the start coordinate element of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::start
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajint] Start coordinate
** @@
******************************************************************************/

ajint ensMapperunitGetStart(const EnsPMapperunit mu)
{
    if(!mu)
        return 0;

    return mu->Start;
}




/* @func ensMapperunitGetEnd **************************************************
**
** Get the end coordinate element of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::end
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajint] End coordinate
** @@
******************************************************************************/

ajint ensMapperunitGetEnd(const EnsPMapperunit mu)
{
    if(!mu)
        return 0;

    return mu->End;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Mapper Unit
** @nam4rule SetObjectIdentifier Set the Ensembl Object identifier
** @nam4rule SetStart Set the start coordinate
** @nam4rule SetEnd Set the end coordinate
**
** @argrule * mu [EnsPMapperunit] Ensembl Mapper Unit object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMapperunitSetObjectIdentifier *************************************
**
** Set the Ensembl Object identifier element of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::id
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
** @param [r] oid [ajuint] Ensembl Object identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperunitSetObjectIdentifier(EnsPMapperunit mu, ajuint oid)
{
    if(!mu)
        return ajFalse;

    if(!oid)
        return ajFalse;

    mu->ObjectIdentifier = oid;

    return ajTrue;
}




/* @func ensMapperunitSetStart ************************************************
**
** Set the start coordinate element of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::start
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
** @param [r] start [ajint] Start coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperunitSetStart(EnsPMapperunit mu, ajint start)
{
    if(!mu)
        return ajFalse;

    mu->Start = start;

    return ajTrue;
}




/* @func ensMapperunitSetEnd **************************************************
**
** Set the end coordinate element of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::end
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
** @param [r] end [ajint] End coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperunitSetEnd(EnsPMapperunit mu, ajint end)
{
    if(!mu)
        return ajFalse;

    mu->End = end;

    return ajTrue;
}




/* @func ensMapperunitGetMemsize **********************************************
**
** Get the memory size in bytes of an Ensembl Mapper Unit.
**
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensMapperunitGetMemsize(const EnsPMapperunit mu)
{
    ajulong size = 0;

    if(!mu)
        return 0;

    size += sizeof (EnsOMapperunit);

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
** @nam3rule Trace Report Ensembl Mapper Unit elements to debug file
**
** @argrule Trace mu [const EnsPMapperunit] Ensembl Mapper Unit
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperunitTrace ***************************************************
**
** Trace an Ensembl Mapper Unit.
**
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperunitTrace(const EnsPMapperunit mu, ajuint level)
{
    AjPStr indent = NULL;

    if(!mu)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperunitTrace %p\n"
            "%S  ObjectIdentifier %u\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Use %u\n",
            indent, mu,
            indent, mu->ObjectIdentifier,
            indent, mu->Start,
            indent, mu->End,
            indent, mu->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPMapperpair] Mapper Pair **********************************
**
** Functions for manipulating Ensembl Mapper Pair objects
**
** Bio::EnsEMBL::Mapper::IndelPair CVS Revision: 1.5
** Bio::EnsEMBL::Mapper::Pair CVS Revision: 1.9
**
** @nam2rule Mapperpair
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Mapper Pair by pointer.
** It is the responsibility of the user to first destroy any previous
** Mapper Pair. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMapperpair]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMapperpair] Ensembl Mapper Pair
** @argrule Ref object [EnsPMapperpair] Ensembl Mapper Pair
**
** @valrule * [EnsPMapperpair] Ensembl Mapper Pair
**
** @fcategory new
******************************************************************************/




/* @func ensMapperpairNew *****************************************************
**
** Default constructor for an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::new
** @param [u] source [EnsPMapperunit] Source Ensembl Mapper Unit
** @param [u] target [EnsPMapperunit] Target Ensembl Mapper Unit
** @param [r] ori [ajint] Relative orientation of the Ensembl Mapper Units
** @cc Bio::EnsEMBL::Mapper::IndelPair::new
** @param [r] indel [AjBool] Insertion-deletion attribute
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNew(EnsPMapperunit source,
                                EnsPMapperunit target,
                                ajint ori,
                                AjBool indel)
{
    EnsPMapperpair mp = NULL;

    if(!source)
        return NULL;

    if(!target)
        return NULL;

    AJNEW0(mp);

    mp->Source = ensMapperunitNewRef(source);

    mp->Target = ensMapperunitNewRef(target);

    mp->Orientation = ori;

    mp->InsertionDeletion = indel;

    mp->Use = 1;

    return mp;
}




/* @func ensMapperpairNewC ****************************************************
**
** Construct an Ensembl Mapper Pair from components of Ensembl Mapper Units.
**
** @param [r] srcoid [ajuint] Source Ensembl Object identifier
** @param [r] srcstart [ajint] Source Start coordinate
** @param [r] srcend [ajint] Source End coordinate
** @param [r] trgoid [ajuint] Target Ensembl Object identifier
** @param [r] trgstart [ajint] Target Start coordinate
** @param [r] trgend [ajint] Target End coordinate
** @param [r] ori [ajint] Relative orientation of the Ensembl Mapper Units
** @param [r] indel [AjBool] Insertion-deletion attribute
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewC(ajuint srcoid,
                                 ajint srcstart,
                                 ajint srcend,
                                 ajuint trgoid,
                                 ajint trgstart,
                                 ajint trgend,
                                 ajint ori,
                                 AjBool indel)
{
    EnsPMapperunit source = NULL;
    EnsPMapperunit target = NULL;

    EnsPMapperpair mp = NULL;

    if(!srcoid)
        return NULL;

    if(!trgoid)
        return NULL;

    source = ensMapperunitNew(srcoid, srcstart, srcend);

    target = ensMapperunitNew(trgoid, trgstart, trgend);

    mp = ensMapperpairNew(source, target, ori, indel);

    ensMapperunitDel(&source);

    ensMapperunitDel(&target);

    return mp;
}




/* @func ensMapperpairNewObj **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewObj(const EnsPMapperpair object)
{
    EnsPMapperpair mp = NULL;

    if(!object)
        return NULL;

    AJNEW0(mp);

    mp->Source = ensMapperunitNewObj(object->Source);
    mp->Target = ensMapperunitNewObj(object->Target);

    mp->Orientation = object->Orientation;

    mp->InsertionDeletion = object->InsertionDeletion;

    mp->Use = 1;

    return mp;
}




/* @func ensMapperpairNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count
**
** @param [u] mp [EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewRef(EnsPMapperpair mp)
{
    if(!mp)
        return NULL;

    mp->Use++;

    return mp;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Mapper Pair.
**
** @fdata [EnsPMapperpair]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Pair object
**
** @argrule * Pmp [EnsPMapperpair*] Mapper Pair object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperpairDel *****************************************************
**
** Default Ensembl Mapper Pair destructor.
**
** @param [d] Pmp [EnsPMapperpair*] Ensembl Mapper Pair address
**
** @return [void]
** @@
******************************************************************************/

void ensMapperpairDel(EnsPMapperpair *Pmp)
{
    EnsPMapperpair pthis = NULL;

    if(!Pmp)
        return;

    if(!*Pmp)
        return;

    pthis = *Pmp;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmp = NULL;

        return;
    }

    ensMapperunitDel(&pthis->Source);

    ensMapperunitDel(&pthis->Target);

    AJFREE(pthis);

    *Pmp = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Mapper Pair object.
**
** @fdata [EnsPMapperpair]
** @fnote None
**
** @nam3rule Get Return Ensembl Mapper Pair attribute(s)
** @nam4rule GetSource Return the source Ensembl Mapper Unit
** @nam4rule GetTarget Return the target Ensembl Mapper Unit
** @nam4rule GetOrientation Return the orientation
** @nam4rule GetInsertionDeletion Return the 'Insertion Deletion' element
**
** @argrule * mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @valrule Source [EnsPMapperunit] Ensembl Mapper Unit
** @valrule Target [EnsPMapperunit] Ensembl Mapper Unit
** @valrule Orientation [ajint] Orientation
** @valrule InsertionDeletion [AjBool] 'Insertion Deletion' element
**
** @fcategory use
******************************************************************************/




/* @func ensMapperpairGetSource ***********************************************
**
** Get the source Ensembl Mapper Unit element of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::from
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperunit] Source Ensembl Mapper Unit
** @@
******************************************************************************/

EnsPMapperunit ensMapperpairGetSource(const EnsPMapperpair mp)
{
    if(!mp)
        return NULL;

    return mp->Source;
}




/* @func ensMapperpairGetTarget ***********************************************
**
** Get the Target Ensembl Mapper Unit element of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::to
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperunit] Target Ensembl Mapper Unit
** @@
******************************************************************************/

EnsPMapperunit ensMapperpairGetTarget(const EnsPMapperpair mp)
{
    if(!mp)
        return NULL;

    return mp->Target;
}




/* @func ensMapperpairGetOrientation ******************************************
**
** Get the relative orientation element of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::ori
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [ajint] Relative orientation
** @@
******************************************************************************/

ajint ensMapperpairGetOrientation(const EnsPMapperpair mp)
{
    if(!mp)
        return 0;

    return mp->Orientation;
}




/* @func ensMapperpairGetInsertionDeletion ************************************
**
** Test whether this Ensembl Mapper Pair represents an insertion-deletion.
**
** @cc Bio::EnsEMBL::Mapper::IndelPair::???
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [AjBool] ajTrue if this Mapper Pair represents an
**                  insertion-deletion
** @@
******************************************************************************/

AjBool ensMapperpairGetInsertionDeletion(const EnsPMapperpair mp)
{
    if(!mp)
        return ajFalse;

    return mp->InsertionDeletion;
}




/* @func ensMapperpairGetUnit *************************************************
**
** Get the Ensembl Mapper Unit element of an Ensembl Mapper Pair via an
** Ensembl Mapper Unit Type enumeration.
**
** @cc Bio::EnsEMBL::Mapper::Pair::from
** @cc Bio::EnsEMBL::Mapper::Pair::to
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
** @param [r] type [EnsEMapperunitType] Ensembl Mapper Unit Type
**
** @return [EnsPMapperunit] Source or Target Ensembl Mapper Unit or NULL
** @@
******************************************************************************/

EnsPMapperunit ensMapperpairGetUnit(const EnsPMapperpair mp,
                                    EnsEMapperunitType type)
{
    if(!mp)
        return NULL;

    if(!type)
        return NULL;

    switch(type)
    {
        case ensEMapperunitTypeSource:
            return mp->Source;
        case ensEMapperunitTypeTarget:
            return mp->Target;
        default:
            ajWarn("ensMapperpairGetUnit got unexpected type %d.\n", type);
    }

    return NULL;
}




/* @func ensMapperpairGetMemsize **********************************************
**
** Get the memory size in bytes of an Ensembl Mapper Pair.
**
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensMapperpairGetMemsize(const EnsPMapperpair mp)
{
    ajulong size = 0;

    if(!mp)
        return 0;

    size += sizeof (EnsOMapperpair);

    size += ensMapperunitGetMemsize(mp->Source);
    size += ensMapperunitGetMemsize(mp->Target);

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Pair object.
**
** @fdata [EnsPMapperpair]
** @nam3rule Trace Report Ensembl Mapper Pair elements to debug file
**
** @argrule Trace mp [const EnsPMapperpair] Ensembl Mapper Pair
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperpairTrace ***************************************************
**
** Trace an Ensembl Mapper Pair.
**
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperpairTrace(const EnsPMapperpair mp, ajuint level)
{
    AjPStr indent = NULL;

    if(!mp)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperpairTrace %p\n"
            "%S  Source %p\n"
            "%S  Target %p\n"
            "%S  Orientation %d\n"
            "%S  InsertionDeletion '%B'\n"
            "%S  Use %u\n",
            indent, mp,
            indent, mp->Source,
            indent, mp->Target,
            indent, mp->Orientation,
            indent, mp->InsertionDeletion,
            indent, mp->Use);

    ensMapperunitTrace(mp->Source, level + 1);
    ensMapperunitTrace(mp->Target, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic mapperpairCompareSourceStartAscending **************************
**
** Comparison function to sort Ensembl Mapper Pairs by the start coordinate of
** their source Ensembl Mapper Unit in ascending order.
**
** @param [r] P1 [const void*] Ensembl Mapper Pair address 1
** @param [r] P2 [const void*] Ensembl Mapper Pair address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int mapperpairCompareSourceStartAscending(const void* P1,
                                                 const void* P2)
{
    int value = 0;

    const EnsPMapperpair mp1 = NULL;
    const EnsPMapperpair mp2 = NULL;

    mp1 = *(EnsPMapperpair const *) P1;
    mp2 = *(EnsPMapperpair const *) P2;

    if(ajDebugTest("mapperpairCompareSourceStartAscending"))
    {
        ajDebug("mapperpairCompareSourceStartAscending\n"
                "  mp1 %p\n"
                "  mp2 %p\n",
                mp1,
                mp2);

        ensMapperpairTrace(mp1, 1);
        ensMapperpairTrace(mp2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(mp1 && (!mp2))
        return -1;

    if((!mp1) && (!mp2))
        return 0;

    if((!mp1) && mp2)
        return +1;

    /*
    ** Sort Ensembl Mapper Pairs with empty source Ensembl Mapper Units
    ** towards the end of the AJAX List.
    */

    if(mp1->Source && (!mp2->Source))
        return -1;

    if((!mp1->Source) && (!mp2->Source))
        return 0;

    if((!mp1->Source) && mp2->Source)
        return +1;

    /* Evaluate the Start coordinates of source Ensembl Mapper Units. */

    if(mp1->Source->Start < mp2->Source->Start)
        value = -1;

    if(mp1->Source->Start > mp2->Source->Start)
        value = +1;

    return value;
}




/* @func ensMapperpairSortBySourceStartAscending ******************************
**
** Sort Ensembl Mapper Pairs by their source Ensembl Mapper Unit
** start coordinate in ascending order.
**
** @param [u] mps [AjPList] AJAX List of Ensembl Mapper Pairs
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperpairSortBySourceStartAscending(AjPList mps)
{
    if(!mps)
        return ajFalse;

    ajListSort(mps, mapperpairCompareSourceStartAscending);

    return ajTrue;
}




/* @funcstatic mapperpairCompareTargetStartAscending **************************
**
** Comparison function to sort Ensembl Mapper Pairs by the start coordinate of
** their target Ensembl Mapper Unit in ascending order.
**
** @param [r] P1 [const void*] Ensembl Mapper Pair address 1
** @param [r] P2 [const void*] Ensembl Mapper Pair address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int mapperpairCompareTargetStartAscending(const void* P1,
                                                 const void* P2)
{
    int value = 0;

    const EnsPMapperpair mp1 = NULL;
    const EnsPMapperpair mp2 = NULL;

    mp1 = *(EnsPMapperpair const *) P1;
    mp2 = *(EnsPMapperpair const *) P2;

    if(ajDebugTest("mapperpairCompareTargetStartAscending"))
    {
        ajDebug("mapperpairCompareTargetStartAscending\n"
                "  mp1 %p\n"
                "  mp2 %p\n",
                mp1,
                mp2);

        ensMapperpairTrace(mp1, 1);
        ensMapperpairTrace(mp2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(mp1 && (!mp2))
        return -1;

    if((!mp1) && (!mp2))
        return 0;

    if((!mp1) && mp2)
        return +1;

    /*
    ** Sort Ensembl Mapper Pairs with empty target Ensembl Mapper Units
    ** towards the end of the AJAX List.
    */

    if(mp1->Target && (!mp2->Target))
        return -1;

    if((!mp1->Target) && (!mp2->Target))
        return 0;

    if((!mp1->Target) && mp2->Target)
        return +1;

    /* Evaluate the Start coordinates of target Ensembl Mapper Units. */

    if(mp1->Target->Start < mp2->Target->Start)
        value = -1;

    if(mp1->Target->Start > mp2->Target->Start)
        value = +1;

    return value;
}




/* @func ensMapperpairSortByTargetStartAscending ******************************
**
** Sort Ensembl Mapper Pairs by their target Ensembl Mapper Unit
** start coordinate in ascending order.
**
** @param [u] mps [AjPList] AJAX List of Ensembl Mapper Pairs
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperpairSortByTargetStartAscending(AjPList mps)
{
    if(!mps)
        return ajFalse;

    ajListSort(mps, mapperpairCompareTargetStartAscending);

    return ajTrue;
}




/* @datasection [EnsPMapperresult] Mapper Result ******************************
**
** Functions for manipulating Ensembl Mapper Result objects
**
** Bio::EnsEMBL::Mapper::Coordinate CVS Revision: 1.13
** Bio::EnsEMBL::Mapper::Gap CVS Revision: 1.11
** Bio::EnsEMBL::Mapper::IndelCoordinate CVS Revision: 1.6
**
** @nam2rule Mapperresult
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Mapper Result by pointer.
** It is the responsibility of the user to first destroy any previous
** Mapper Result. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMapperresult]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMapperresult] Ensembl Mapper Result
** @argrule Ref object [EnsPMapperresult] Ensembl Mapper Result
**
** @valrule * [EnsPMapperresult] Ensembl Mapper Result
**
** @fcategory new
******************************************************************************/




/* @func ensMapperresultNew ***************************************************
**
** Default constructor for an Ensembl Mapper Result.
**
** @param [r] type [EnsEMapperresultType] Type
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start
** @param [r] end [ajint] End
** @param [r] strand [ajint] Strand
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] gapstart [ajint] Gap start
** @param [r] gapend [ajint] Gap end
** @param [r] rank [ajuint] Rank
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNew(EnsEMapperresultType type,
                                    ajuint oid,
                                    ajint start,
                                    ajint end,
                                    ajint strand,
                                    EnsPCoordsystem cs,
                                    ajint gapstart,
                                    ajint gapend,
                                    ajuint rank)
{
    EnsPMapperresult mr = NULL;

    if(!type)
        return NULL;

    if((type == ensEMapperresultCoordinate) ||
       (type == ensEMapperresultInDel))
    {
        if(!oid)
            return NULL;

        if(!cs)
            return NULL;
    }

    AJNEW0(mr);

    mr->Type = type;

    mr->ObjectIdentifier = oid;

    mr->Start = start;

    mr->End = end;

    mr->Strand = strand;

    mr->Coordsystem = ensCoordsystemNewRef(cs);

    mr->GapStart = gapstart;

    mr->GapEnd = gapend;

    mr->Rank = rank;

    mr->Use = 1;

    return mr;
}




/* @func ensMapperresultNewObj ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewObj(const EnsPMapperresult object)
{
    EnsPMapperresult mr = NULL;

    if(!object)
        return NULL;

    AJNEW0(mr);

    mr->Type = object->Type;

    mr->ObjectIdentifier = object->ObjectIdentifier;

    mr->Start = object->Start;

    mr->End = object->End;

    mr->Strand = object->Strand;

    mr->Coordsystem = ensCoordsystemNewRef(object->Coordsystem);

    mr->GapStart = object->GapStart;

    mr->GapEnd = object->GapEnd;

    mr->Rank = object->Rank;

    mr->Use = 1;

    return mr;
}




/* @func ensMapperresultNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewRef(EnsPMapperresult mr)
{
    if(!mr)
        return NULL;

    mr->Use++;

    return mr;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Mapper Result.
**
** @fdata [EnsPMapperresult]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Result object
**
** @argrule * Pmr [EnsPMapperresult*] Mapper Result object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperresultDel ***************************************************
**
** Default Ensembl Mapper Result destructor.
**
** @param [d] Pmr [EnsPMapperresult*] Ensembl Mapper Result address
**
** @return [void]
** @@
******************************************************************************/

void ensMapperresultDel(EnsPMapperresult *Pmr)
{
    EnsPMapperresult pthis = NULL;

    if(!Pmr)
        return;

    if(!*Pmr)
        return;

    pthis = *Pmr;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmr = NULL;

        return;
    }

    ensCoordsystemDel(&pthis->Coordsystem);

    AJFREE(pthis);

    *Pmr = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
** @fnote None
**
** @nam3rule Get Return Ensembl Mapper Result attribute(s)
** @nam4rule GetType Return the type
** @nam4rule GetObjectIdentifier Return the Ensembl Object identifier
** @nam4rule GetStart Return the start
** @nam4rule GetEnd Return the end
** @nam4rule GetStrand Return the strand
** @nam4rule GetCoordsystem Return the Ensembl Coordinate System
** @nam4rule GetGapStart Return the gap start
** @nam4rule GetGapEnd Return the gap end
** @nam4rule GetRank Return the rank
**
** @argrule * mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @valrule Type [EnsEMapperresultType] Type
** @valrule ObjectIdentifier [ajuint] Ensembl Object identifier
** @valrule Start [ajint] Start
** @valrule End [ajint] End
** @valrule Strand [ajint] Strand
** @valrule Coordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule GapStart [ajint] Gap start
** @valrule GapEnd [ajint] Gap end
** @valrule Rank [ajuint] Rank
**
** @fcategory use
******************************************************************************/




/* @func ensMapperresultGetType ***********************************************
**
** Get the type element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsEMapperresultType] Type or ensEMapperresultNULL
** @@
******************************************************************************/

EnsEMapperresultType ensMapperresultGetType(const EnsPMapperresult mr)
{
    if(!mr)
        return ensEMapperresultNULL;

    return mr->Type;
}




/* @func ensMapperresultGetObjectIdentifier ***********************************
**
** Get the Ensembl Object identifier element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Ensembl Object identifier
** @@
******************************************************************************/

ajuint ensMapperresultGetObjectIdentifier(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->ObjectIdentifier;
}




/* @func ensMapperresultGetStart **********************************************
**
** Get the start element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Start
** @@
******************************************************************************/

ajint ensMapperresultGetStart(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->Start;
}




/* @func ensMapperresultGetEnd ************************************************
**
** Get the end element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] End
** @@
******************************************************************************/

ajint ensMapperresultGetEnd(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->End;
}




/* @func ensMapperresultGetStrand *********************************************
**
** Get the strand element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Strand
** @@
******************************************************************************/

ajint ensMapperresultGetStrand(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->Strand;
}




/* @func ensMapperresultGetCoordsystem ****************************************
**
** Get the Ensembl Coordinate System element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsPCoordsystem] Ensembl Coordinate System
** @@
******************************************************************************/

EnsPCoordsystem ensMapperresultGetCoordsystem(const EnsPMapperresult mr)
{
    if(!mr)
        return NULL;

    return mr->Coordsystem;
}




/* @func ensMapperresultGetGapStart *******************************************
**
** Get the gap start element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Ensembl Mapper Result Gap Start Coordinate
** @@
******************************************************************************/

ajint ensMapperresultGetGapStart(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->GapStart;
}




/* @func ensMapperresultGetGapEnd *********************************************
**
** Get the gap end element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Ensembl Mapper Result Gap End Coordinate
** @@
******************************************************************************/

ajint ensMapperresultGetGapEnd(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->GapEnd;
}




/* @func ensMapperresultGetRank ***********************************************
**
** Get the rank element of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Rank
** @@
******************************************************************************/

ajint ensMapperresultGetRank(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return mr->Rank;
}




/* @func ensMapperresultGetCoordinateLength ***********************************
**
** Get the coordinate (end - start + 1) length of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Coordinate length
** @@
******************************************************************************/

ajuint ensMapperresultGetCoordinateLength(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return (mr->End - mr->Start + 1);
}




/* @func ensMapperresultGetGapLength ******************************************
**
** Get the gap (gapend - gapstart + 1) length of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Gap length
** @@
******************************************************************************/

ajuint ensMapperresultGetGapLength(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    return (mr->GapEnd - mr->GapStart + 1);
}




/* @func ensMapperresultGetLength *********************************************
**
** Get the length of an Ensembl Mapper Result.
** This is the coordinate length for Mapper Results of type
** ensEMapperresultCoordinate and ensEMapperresultInDel and the gap length
** for Mapper Resuls of type ensEMapperresultGap.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Ensembl Mapper Result length
** @@
******************************************************************************/

ajuint ensMapperresultGetLength(const EnsPMapperresult mr)
{
    if(!mr)
        return 0;

    switch(mr->Type)
    {
        case ensEMapperresultCoordinate:
            return ensMapperresultGetCoordinateLength(mr);
        case ensEMapperresultGap:
            return ensMapperresultGetGapLength(mr);
        case ensEMapperresultInDel:
            return ensMapperresultGetCoordinateLength(mr);
        default:
            ajWarn("ensMapperresultGetLength got unexpected "
                   "Ensembl Mapper Result type %d.\n", mr->Type);
    }

    return 0;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
** @nam3rule Trace Report Ensembl Mapper Result elements to debug file
**
** @argrule Trace mr [const EnsPMapperresult] Ensembl Mapper Result
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperresultTrace *************************************************
**
** Trace an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperresultTrace(const EnsPMapperresult mr, ajuint level)
{
    AjPStr indent = NULL;

    if(!mr)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperresultTrace %p\n"
            "%S  Type %d\n"
            "%S  ObjectIdentifier %u\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Strand %d\n"
            "%S  Coordsystem %p\n"
            "%S  GapStart %d\n"
            "%S  GapEnd %d\n"
            "%S  Rank %u\n"
            "%S  Use %u\n",
            indent, mr,
            indent, mr->Type,
            indent, mr->ObjectIdentifier,
            indent, mr->Start,
            indent, mr->End,
            indent, mr->Strand,
            indent, mr->Coordsystem,
            indent, mr->GapStart,
            indent, mr->GapEnd,
            indent, mr->Rank,
            indent, mr->Use);

    ensCoordsystemTrace(mr->Coordsystem, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPMapperrange] Mapper Range ********************************
**
** Functions for manipulating Ensembl Mapper Range objects
**
** @nam2rule Mapperrange
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Mapper Range by pointer.
** It is the responsibility of the user to first destroy any previous
** Mapper Range. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMapperrange]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMapperrange] Ensembl Mapper Range
** @argrule Ref object [EnsPMapperrange] Ensembl Mapper Range
**
** @valrule * [EnsPMapperrange] Ensembl Mapper Range
**
** @fcategory new
******************************************************************************/




/* @func ensMapperrangeNew ****************************************************
**
** Default constructor for an Ensembl Mapper Range.
**
** @param [r] start [ajint] Start
** @param [r] end [ajint] End
**
** @return [EnsPMapperrange] Ensembl Mapper Range
** @@
******************************************************************************/

EnsPMapperrange ensMapperrangeNew(ajint start, ajint end)
{
    EnsPMapperrange mr = NULL;

    AJNEW0(mr);

    mr->Start = start;
    mr->End   = end;
    mr->Use   = 1;

    return mr;
}




/* @func ensMapperrangeNewObj *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [EnsPMapperrange] Ensembl Mapper Range or NULL
** @@
******************************************************************************/

EnsPMapperrange ensMapperrangeNewObj(const EnsPMapperrange object)
{
    EnsPMapperrange mr = NULL;

    if(!object)
        return NULL;

    AJNEW0(mr);

    mr->Start = object->Start;
    mr->End   = object->End;
    mr->Use   = 1;

    return mr;
}




/* @func ensMapperrangeNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mr [EnsPMapperrange] Ensembl Mapper Range
**
** @return [EnsPMapperrange] Ensembl Mapper Range or NULL
** @@
******************************************************************************/

EnsPMapperrange ensMapperrangeNewRef(EnsPMapperrange mr)
{
    if(!mr)
        return NULL;

    mr->Use++;

    return mr;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Mapper Range.
**
** @fdata [EnsPMapperrange]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Range object
**
** @argrule * Pmr [EnsPMapperrange*] Ensembl Mapper Range object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperrangeDel ****************************************************
**
** Default Ensembl Mapper Range destructor.
**
** @param [d] Pmr [EnsPMapperrange*] Ensembl Mapper Range address
**
** @return [void]
** @@
******************************************************************************/

void ensMapperrangeDel(EnsPMapperrange *Pmr)
{
    EnsPMapperrange pthis = NULL;

    if(!Pmr)
        return;

    if(!*Pmr)
        return;

    if(ajDebugTest("ensMapperrangeDel"))
        ajDebug("ensMapperrangeDel\n"
                "  *Pmr %p\n",
                *Pmr);

    pthis = *Pmr;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmr = NULL;

        return;
    }

    AJFREE(pthis);

    *Pmr = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Mapper Range object.
**
** @fdata [EnsPMapperrange]
** @fnote None
**
** @nam3rule Get Return Ensembl Mapper Range attribute(s)
** @nam4rule GetStart Return the start
** @nam4rule GetEnd Return the end
**
** @argrule * mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @valrule Start [ajint] Start
** @valrule End [ajint] End
**
** @fcategory use
******************************************************************************/




/* @func ensMapperrangeGetStart ***********************************************
**
** Get the start element of an Ensembl Mapper Range.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [ajint] Start
** @@
******************************************************************************/

ajint ensMapperrangeGetStart(const EnsPMapperrange mr)
{
    if(!mr)
        return 0;

    return mr->Start;
}




/* @func ensMapperrangeGetEnd *************************************************
**
** Get the end element of an Ensembl Mapper Range.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [ajint] End
** @@
******************************************************************************/

ajint ensMapperrangeGetEnd(const EnsPMapperrange mr)
{
    if(!mr)
        return 0;

    return mr->End;
}




/* @datasection [EnsPMapperrangeregistry] Mapper Range Registry ***************
**
** Functions for manipulating Ensembl Mapper Range Registry objects
**
** Bio::EnsEMBL::Mapper::RangeRegistry CVS Revision: 1.13
**
** @nam2rule Mapperrangeregistry
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Mapper Range Registry by pointer.
** It is the responsibility of the user to first destroy any previous
** Mapper Range Registry. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMapperrangeregistry]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
** @argrule Ref object [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @valrule * [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @fcategory new
******************************************************************************/




/* @func ensMapperrangeregistryNew ********************************************
**
** Default constructor for an Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::new
** @return [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
** @@
** The Ensembl Mapper Range Registry maintains an internal list of registered
** regions and is used to quickly ascertain if and what regions of a provided
** range need registration. It is implemented as a first-level AJAX Table with
** Ensembl Object identifers as keys and second-level AJAX Lists of
** Ensembl Mapper Ranges as values.
******************************************************************************/

EnsPMapperrangeregistry ensMapperrangeregistryNew(void)
{
    EnsPMapperrangeregistry mrr = NULL;

    if(ajDebugTest("ensMapperrangeregistryNew"))
        ajDebug("ensMapperrangeregistryNew\n");

    AJNEW0(mrr);

    mrr->Registry =
        ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    mrr->Use = 1;

    return mrr;
}




/* @func ensMapperrangeregistryNewRef *****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @return [EnsPMapperrangeregistry] Ensembl Mapper Range Registry or NULL
** @@
******************************************************************************/

EnsPMapperrangeregistry ensMapperrangeregistryNewRef(
    EnsPMapperrangeregistry mrr)
{
    if(!mrr)
        return NULL;

    mrr->Use++;

    return mrr;
}




/* @func ensMapperrangeregistryClear ******************************************
**
** Clear an Ensembl Mapper Range Registry.
** This function clears the unsigned integer key data and the AJAX List value
** data from the first-level AJAX Table, as well as the Ensembl Mapper Ranges
** from the second-level AJAX List.
**
** @param [u] mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperrangeregistryClear(EnsPMapperrangeregistry mrr)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    EnsPMapperrange mr = NULL;

    if(ajDebugTest("ensMapperrangeregistryClear"))
        ajDebug("ensMapperrangeregistryClear\n"
                "  mrr %p\n",
                mrr);

    if(!mrr)
        return ajFalse;

    /* Clear the first-level AJAX Table. */

    ajTableToarrayKeysValues(mrr->Registry, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajTableRemove(mrr->Registry, (const void *) keyarray[i]);

        /* Delete unsigned integer key data. */

        AJFREE(keyarray[i]);

        /* Clear and delete the second-level AJAX List value data. */

        while(ajListPop((AjPList) valarray[i], (void **) &mr))
            ensMapperrangeDel(&mr);

        ajListFree((AjPList *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Mapper Range Registry.
**
** @fdata [EnsPMapperrangeregistry]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Range Registry object
**
** @argrule * Pmrr [EnsPMapperrangeregistry*] Ensembl Mapper Range Registry
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperrangeregistryDel ********************************************
**
** Default Ensembl Mapper Range Registry destructor.
**
** @param [d] Pmrr [EnsPMapperrangeregistry*] Ensembl Mapper
**                                            Range Registry address
**
** @return [void]
** @@
******************************************************************************/

void ensMapperrangeregistryDel(EnsPMapperrangeregistry *Pmrr)
{
    EnsPMapperrangeregistry pthis = NULL;

    if(!Pmrr)
        return;

    if(!*Pmrr)
        return;

    if(ajDebugTest("ensMapperrangeregistryDel"))
        ajDebug("ensMapperrangeregistryDel\n"
                "  *Pmrr %p\n",
                *Pmrr);

    pthis = *Pmrr;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmrr = NULL;

        return;
    }

    ensMapperrangeregistryClear(pthis);

    ajTableFree(&pthis->Registry);

    AJFREE(pthis);

    *Pmrr = NULL;

    return;
}




/* @func ensMapperrangeregistryCheckAndRegister *******************************
**
** Check and register Ensembl Mapper Ranges in the
** Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::check_and_register
** @param [u] mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] chkstart [ajint] Start coordinate of the region to be checked
** @param [r] chkend [ajint] End coordinate of the region to be checked
** @param [r] regstart [ajint] Start coordinate of the region to register
** @param [r] regend [ajint] End coordinate of the region to register
** @param [uN] ranges [AjPList] AJAX List of Ensembl Mapper Ranges
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
**
** Checks the Ensembl Mapper Range Registry to see if the entire range denoted
** by (oid:chkstart-chkend) is already registered. If it already is, an empty
** AJAX List is returned. If it is not, then the range specified by regstart
** and regend is registered and an AJAX List of Ensembl Mapper Regions that
** were required to completely register this region is returned.
**
** If regstart and regend are not defined, they default to chkstart and chkend,
** respectively.
**
** The reason, there is just a single function to do both the checking and
** registering is to reduce the overhead. Much of the work to check if a range
** is registered is the same as registering a region around that range.
******************************************************************************/

AjBool ensMapperrangeregistryCheckAndRegister(EnsPMapperrangeregistry mrr,
                                              ajuint oid,
                                              ajint chkstart,
                                              ajint chkend,
                                              ajint regstart,
                                              ajint regend,
                                              AjPList ranges)
{
    register ajuint i = 0;

    ajuint idxstart  = 0;
    ajuint idxmid    = 0;
    ajuint idxend    = 0;
    ajuint idxlength = 0;

    ajuint idxregstart = 0;
    ajuint idxregend   = 0;
    ajuint idxregpos   = 0;

    ajint gapstart = 0;
    ajint gapend   = 0;
    ajint newstart = 0;
    ajint newend   = 0;

    ajuint *Poid = NULL;

    AjBool idxregstartset = AJFALSE;
    AjBool idxregposset   = AJFALSE;

    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPMapperrange range = NULL;
    EnsPMapperrange gap   = NULL;

    if(ajDebugTest("ensMapperrangeregistryCheckAndRegister"))
        ajDebug("ensMapperrangeregistryCheckAndRegister\n"
                "  mrr %p\n"
                "  oid %u\n"
                "  chkstart %d\n"
                "  chkend %d\n"
                "  regstart %d (%#x)\n"
                "  regend %d (%#x)\n"
                "  ranges %p\n",
                mrr,
                oid,
                chkstart,
                chkend,
                regstart, regstart,
                regend, regend,
                ranges);

    if(!mrr)
    {
        ajDebug("ensMapperrangeregistryCheckAndRegister requires an "
                "Ensembl Range Registry.\n");

        return ajFalse;
    }

    if(!oid)
        ajWarn("ensMapperrangeregistryCheckAndRegister did not get a valid "
               "Ensembl Object identifier.\n");

    if(chkstart > chkend)
    {
        ajWarn("ensMapperrangeregistryCheckAndRegister requires the start "
               "coordinate (%d) to be less than or equal the end "
               "coordinate (%d) for the region to be checked.\n",
               chkstart, chkend);

        return ajFalse;
    }

    if((!regstart) && (!regend))
    {
        regstart = chkstart;

        regend = chkend;
    }

    if(regstart > regend)
    {
        ajWarn("ensMapperrangeregistryCheckAndRegister requires the start "
               "coordinate (%d) to be less than or equal the end "
               "coordinate (%d) for the region to be registered.\n",
               regstart, regend);

        return ajFalse;
    }

    if(regstart > chkstart)
    {
        ajWarn("ensMapperrangeregistryCheckAndRegister requires the start "
               "coordinate (%d) of the region to be registered to be less "
               "than or equal the start coordinate (%d) of the "
               "region to be checked.\n",
               regstart, chkstart);

        return ajFalse;
    }

    if(regend < chkend)
    {
        ajWarn("ensMapperrangeregistryCheckAndRegister requires the end "
               "coordinate (%d) of the region to be registered to be less "
               "than or equal the end coordinate (%d) of the "
               "region to be checked.\n",
               regend, chkend);

        return ajFalse;
    }

    if(!mrr->Registry)
        ajFatal("ensMapperrangeregistryCheckAndRegister got a "
                "Mapper Range Registry without a valid AJAX Table.\n");

    list = (AjPList) ajTableFetch(mrr->Registry, (const void *) &oid);

    if(!list)
    {
        AJNEW0(Poid);

        *Poid = oid;

        list = ajListNew();

        ajTablePut(mrr->Registry, (void *) Poid, (void *) list);
    }

    idxlength = ajListGetLength(list);

    if(!idxlength)
    {
        /*
        ** This is the first request for this Ensembl Object identifier,
        ** return a gap Mapper Range for the entire range and register it as
        ** seen.
        */

        range = ensMapperrangeNew(regstart, regend);

        ajListPushAppend(list, (void *) range);

        if(ranges)
            ajListPushAppend(ranges, (void *) ensMapperrangeNewRef(range));

        return ajTrue;
    }

    /*
    ** Loop through the List of existing Ensembl Mapper Ranges recording any
    ** "gaps" where the existing Mapper Ranges do not cover part of the
    ** requested range.
    */

    idxstart = 0;
    idxend   = idxlength - 1;

    /*
    ** Binary search the relevant Ensembl Mapper Ranges,
    ** which helps if the AJAX List of Ensembl Mapper Ranges is long.
    */

    while((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &range);

        if(range->End < regstart)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    gapstart = regstart;

    for(i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &range);

        /*
        ** No work needs to be done at all if we find a Mapper Range that
        ** entirely overlaps the requested region.
        */

        if((range->Start <= chkstart) && (range->End >= chkend))
            return ajTrue;

        /* Find adjacent or overlapping regions already registered. */

        if((range->Start <= (regend + 1)) && (range->End >= (regstart - 1)))
        {
            if(!idxregstartset)
            {
                idxregstartset = ajTrue;

                idxregstart = i;
            }

            idxregend = i;
        }

        /* Find previously un-registered regions (gaps). */

        if(range->Start > regstart)
        {
            gapend = (regend < range->Start) ? regend : (range->Start - 1);

            if(ranges)
            {
                gap = ensMapperrangeNew(gapstart, gapend);

                ajListPushAppend(ranges, (void *) gap);
            }
        }

        gapstart = (regstart > range->End) ? regstart : (range->End + 1);

        if((range->End >= regend) && (!idxregposset))
        {
            idxregposset = ajTrue;

            idxregpos = i;

            break;
        }
    }

    /* Do we have to define another gap? */

    if((gapstart <= regend) && ranges)
    {
        gap = ensMapperrangeNew(gapstart, regend);

        ajListPushAppend(ranges, (void *) gap);
    }

    /* Merge the new Ensembl Mapper Range into the AJAX List of registered. */

    if(idxregstartset)
    {
        /* Adjacent or overlapping regions have been found. */

        ajListPeekNumber(list, idxregstart, (void **) &range);

        newstart = (regstart < range->Start) ? regstart : range->Start;

        ajListPeekNumber(list, idxregend, (void **) &range);

        newend = (regend > range->End) ? regend : range->End;

        iter = ajListIterNew(list);

        /* Position the AJAX List Iterator at idxregstart. */

        for(i = 0; i < idxregstart; i++)
            range = (EnsPMapperrange) ajListIterGet(iter);

        /*
        ** Now, remove idxregend - idxregstart + 1 elements
        ** from the AJAX List.
        */

        for(i = 0; i < (idxregend - idxregstart + 1); i++)
        {
            range = (EnsPMapperrange) ajListIterGet(iter);

            ajListIterRemove(iter);

            ensMapperrangeDel(&range);
        }

        /* Finally, insert a new Mapper Range at the current position. */

        range = ensMapperrangeNew(newstart, newend);

        ajListIterInsert(iter, (void *) range);

        ajListIterDel(&iter);
    }
    else if(idxregposset)
    {
        iter = ajListIterNew(list);

        /* Position the AJAX List Iterator at idxregpos. */

        for(i = 0; i < idxregpos; i++)
            range = (EnsPMapperrange) ajListIterGet(iter);

        /* Insert a new Mapper Range at this position. */

        range = ensMapperrangeNew(regstart, regend);

        ajListIterInsert(iter, (void *) range);

        ajListIterDel(&iter);
    }
    else
    {
        range = ensMapperrangeNew(regstart, regend);

        ajListPushAppend(list, (void *) range);
    }

    return ajTrue;
}




/* @func ensMapperrangeregistryGetRanges **************************************
**
** Get Ensembl Mapper Ranges from the Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::get_ranges
** @param [r] mrr [const EnsPMapperrangeregistry] Ensembl Mapper
**                                                Range Registry
** @param [r] oid [ajuint] Ensembl Object identifier
**
** @return [const AjPList] AJAX List of Ensembl Mapper Ranges or NULL
** @@
******************************************************************************/

const AjPList ensMapperrangeregistryGetRanges(const EnsPMapperrangeregistry mrr,
                                              ajuint oid)
{
    if(!mrr)
        return NULL;

    if(!oid)
        return NULL;

    return (AjPList) ajTableFetch(mrr->Registry, (const void *) &oid);
}




/* @func ensMapperrangeregistryOverlapSize ************************************
**
** Get the overlap size of Ensembl Mapper Ranges in the
** Ensembl Mapper Range Registry. Finds out how many bases in the given range
** are already registered in an Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::overlap_size
** @param [u] mrr [const EnsPMapperrangeregistry] Ensembl Mapper
**                                                Range Registry
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
**
** @return [ajuint] Overlap size or 0
** @@
******************************************************************************/

ajuint ensMapperrangeregistryOverlapSize(const EnsPMapperrangeregistry mrr,
                                         ajuint oid,
                                         ajint start,
                                         ajint end)
{
    register ajuint i = 0;

    ajuint idxstart  = 0;
    ajuint idxmid    = 0;
    ajuint idxend    = 0;
    ajuint idxlength = 0;

    ajint mrstart = 0;
    ajint mrend   = 0;

    ajuint overlap = 0;

    AjPList list = NULL;

    EnsPMapperrange range = NULL;

    if(!mrr)
        return 0;

    if(start > end)
        return 0;

    list = (AjPList) ajTableFetch(mrr->Registry, (const void *) &oid);

    if(!list)
        return 0;

    idxlength = ajListGetLength(list);

    if(!idxlength)
        return 0;

    idxstart = 0;
    idxend   = idxlength - 1;

    /*
    ** Binary search the relevant Ensembl Mapper Ranges,
    ** which helps if the AJAX List is long.
    */

    while((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &range);

        if(range->End < start)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    for(i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &range);

        /*
        ** Check, wheher the loop has already overrun.
        ** If that was the case, there are no more interesting Mapper Ranges.
        */

        if(range->Start > start)
            break;

        /*
        ** No work needs to be done at all if we find a Mapper Range that
        ** entirely overlaps the requested region.
        */

        if((range->Start <= start) && (range->End >= end))
        {
            overlap = (ajuint) (end - start + 1);

            break;
        }

        mrstart = (start < range->Start) ? range->Start : start;

        mrend = (end < range->End) ? end : range->End;

        if((mrend - mrstart) >= 0)
            overlap += (ajuint) (mrend - mrstart + 1);
    }

    return overlap;
}




/* @datasection [EnsPMapper] Mapper *******************************************
**
** Functions for manipulating Ensembl Mapper objects
**
** @cc Bio::EnsEMBL::Mapper CVS Revision: 1.48
**
** @nam2rule Mapper
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Mapper by pointer.
** It is the responsibility of the user to first destroy any previous
** Mapper. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMapper]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPMapper] Ensembl Mapper
** @argrule Ref object [EnsPMapper] Ensembl Mapper
**
** @valrule * [EnsPMapper] Ensembl Mapper
**
** @fcategory new
******************************************************************************/




/* @func ensMapperNew *********************************************************
**
** Default constructor for an Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::new
** @param [r] srctype [AjPStr] Source type
** @param [r] trgtype [AjPStr] Target type
** @param [u] srccs [EnsPCoordsystem] Source Ensembl Coordinate System
** @param [u] trgcs [EnsPCoordsystem] Target Ensembl Coordinate System
**
** @return [EnsPMapper] Ensembl Mapper or NULL
** @@
******************************************************************************/

EnsPMapper ensMapperNew(AjPStr srctype,
                        AjPStr trgtype,
                        EnsPCoordsystem srccs,
                        EnsPCoordsystem trgcs)
{
    AjPTable table = NULL;

    EnsPMapper mapper = NULL;

    if(ajDebugTest("ensMapperNew"))
    {
        ajDebug("ensMapperNew\n"
                "  srctype '%S'\n"
                "  trgtype '%S'\n"
                "  srccs %p\n"
                "  trgcs %p\n",
                srctype,
                trgtype,
                srccs,
                trgcs);

        ensCoordsystemTrace(srccs, 1);
        ensCoordsystemTrace(trgcs, 1);
    }

    if(!srctype)
        return NULL;

    if(!trgtype)
        return NULL;

    if(!srccs)
        return NULL;

    if(!trgcs)
        return NULL;

    AJNEW0(mapper);

    mapper->SourceType = ajStrNewRef(srctype);

    mapper->TargetType = ajStrNewRef(trgtype);

    mapper->SourceCoordsystem = ensCoordsystemNewRef(srccs);

    mapper->TargetCoordsystem = ensCoordsystemNewRef(trgcs);

    mapper->Pairs = ajTablestrNewCaseLen(0);

    mapper->PairCount = 0;

    mapper->IsSorted = ajFalse;

    mapper->Use = 1;

    /*
    ** Initialise second-level AJAX Tables with Ensembl Object identifier
    ** keys and put them into the first-level AJAX Table indexed on
    ** source and target types.
    */

    table = ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    ajTablePut(mapper->Pairs, (void *) ajStrNewS(srctype), (void *) table);

    table = ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    ajTablePut(mapper->Pairs, (void *) ajStrNewS(trgtype), (void *) table);

    return mapper;
}




/* @func ensMapperNewRef ******************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mapper [EnsPMapper] Ensembl Mapper
**
** @return [EnsPMapper] Ensembl Mapper or NULL
** @@
******************************************************************************/

EnsPMapper ensMapperNewRef(EnsPMapper mapper)
{
    if(!mapper)
        return NULL;

    mapper->Use++;

    return mapper;
}




/* @func ensMapperClear *******************************************************
**
** Clear an Ensembl Mapper.
**
** This function does not delete or modify first-level AJAX Tables with
** Ensembl Mapper type AJAX Strings as keys and second-level AJAX Tables as
** values. The second-level AJAX Tables with Object identifiers as keys and
** third-level AJAX Lists of Ensembl Mapper Pairs are cleared.
** The third-level AJAX Lists of Ensembl Mapper Pairs are cleared and deleted,
** as are the Ensembl Mapper Pair list elements.
**
** @cc Bio::EnsEMBL::Mapper::flush
** @param [u] mapper [EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperClear(EnsPMapper mapper)
{
    void **keyarray1 = NULL;
    void **valarray1 = NULL;
    void **keyarray2 = NULL;
    void **valarray2 = NULL;

    register ajuint i = 0;
    register ajuint j = 0;

    EnsPMapperpair mp = NULL;

    if(ajDebugTest("ensMapperClear"))
    {
        ajDebug("ensMapperClear\n"
                "  mapper %p\n",
                mapper);

        ensMapperTrace(mapper, 1);
    }

    if(!mapper)
        return ajFalse;

    /*
    ** Clear the first-level AJAX Table with Ensembl Mapper type AJAX Strings
    ** of source and target as keys and AJAX Tables as values into an array.
    ** This first-level AJAX Table is not modified here, but deleted later by
    ** the ensMapperDel function.
    */

    ajTableToarrayKeysValues(mapper->Pairs, &keyarray1, &valarray1);

    for(i = 0; keyarray1[i]; i++)
    {
        /*
        ** Clear the second-level AJAX Tables with Ensembl Object identifiers
        ** as keys and AJAX Lists as values.
        */

        ajTableToarrayKeysValues((AjPTable) valarray1[i],
                                 &keyarray2, &valarray2);

        for(j = 0; keyarray2[j]; j++)
        {
            ajTableRemove((AjPTable) valarray1[i],
                          (const void *) keyarray2[j]);

            AJFREE(keyarray2[j]);

            /*
            ** Clear the third-level AJAX Lists of Ensembl Mapper Pairs and
            ** delete the Ensembl Mapper Pair list elements.
            */

            while(ajListPop((AjPList) valarray2[j], (void **) &mp))
                ensMapperpairDel(&mp);

            /* Delete the third-level AJAX Lists */

            ajListFree((AjPList *) &valarray2[j]);
        }

        AJFREE(keyarray2);
        AJFREE(valarray2);
    }

    AJFREE(keyarray1);
    AJFREE(valarray1);

    mapper->PairCount = 0;
    mapper->IsSorted  = ajFalse;

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Mapper.
**
** @fdata [EnsPMapper]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Mapper object
**
** @argrule * Pmapper [EnsPMapper*] Ensembl Mapper object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperDel *********************************************************
**
** Default destructor for an Ensembl Mapper.
**
** This function clears and deletes first-level AJAX Tables with Ensembl Mapper
** type AJAX Strings as keys and second-level AJAX Tables as values.
** The second-level AJAX Tables with Object identifiers as keys and
** third-level AJAX Lists of Ensembl Mapper Pairs are also cleared and deleted.
** The third-level AJAX Lists of Ensembl Mapper Pairs are cleared and deleted,
** as are the Ensembl Mapper Pair list elements.
**
** @param [d] Pmapper [EnsPMapper*] Ensembl Mapper address
**
** @return [void]
** @@
******************************************************************************/

void ensMapperDel(EnsPMapper *Pmapper)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    EnsPMapper pthis = NULL;

    register ajuint i = 0;

    if(!Pmapper)
        return;

    if(!*Pmapper)
        return;

    pthis = *Pmapper;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmapper = NULL;

        return;
    }

    ensMapperClear(pthis);

    ajStrDel(&pthis->SourceType);

    ajStrDel(&pthis->TargetType);

    ensCoordsystemDel(&pthis->SourceCoordsystem);

    ensCoordsystemDel(&pthis->TargetCoordsystem);

    ajTableToarrayKeysValues(pthis->Pairs, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        /*
        ** Clear the first-level AJAX Tables with source and target
        ** Ensembl Mapper types as keys and second-level AJAX Tables as values.
        */

        ajStrDel((AjPStr *) &keyarray[i]);

        ajTableFree((AjPTable *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    /* Delete the first-level AJAX Table. */

    ajTableFree(&pthis->Pairs);

    AJFREE(pthis);

    *Pmapper = NULL;

    return;
}




/* @func ensMapperGetSourceType ***********************************************
**
** Get the source type element of an Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::from
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [AjPStr] Source type
** @@
******************************************************************************/

AjPStr ensMapperGetSourceType(const EnsPMapper mapper)
{
    if(!mapper)
        return NULL;

    return mapper->SourceType;
}




/* @func ensMapperGetTargetType ***********************************************
**
** Get the target type element of an Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::to
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [AjPStr] Target type
** @@
******************************************************************************/

AjPStr ensMapperGetTargetType(const EnsPMapper mapper)
{
    if(!mapper)
        return NULL;

    return mapper->TargetType;
}




/* @func ensMapperGetPairCount ************************************************
**
** Get the number of Ensembl Mapper Pairs in an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [ajuint] Number of Ensembl Mapper Pairs in an Ensembl Mapper
** @@
******************************************************************************/

ajuint ensMapperGetPairCount(const EnsPMapper mapper)
{
    if(!mapper)
        return 0;

    return mapper->PairCount;
}




/* @func ensMapperIsSorted ****************************************************
**
** Test whether the Ensembl Mapper Pairs are sorted in the Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::_is_sorted
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue if the Mapper Pairs are sorted
** @@
******************************************************************************/

AjBool ensMapperIsSorted(const EnsPMapper mapper)
{
    if(!mapper)
        return ajFalse;

    return mapper->IsSorted;
}




/* @funcstatic mapperMergePairs ***********************************************
**
** Merge adjacent Ensembl Mapper Pairs in an Ensembl Mapper into one.
**
** @cc Bio::EnsEMBL::Mapper::_merge_pairs
** @param [r] mapper [EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool mapperMergePairs(EnsPMapper mapper)
{
    void **valarray = NULL;

    register ajuint i = 0;

    AjBool debug = AJFALSE;

    AjIList srciter = NULL;
    AjIList trgiter = NULL;

    AjPList srclist = NULL;
    AjPList trglist = NULL;

    AjPTable srctable = NULL;
    AjPTable trgtable = NULL;

    EnsPMapperpair delpair  = NULL;
    EnsPMapperpair srcpair  = NULL;
    EnsPMapperpair trgpair1 = NULL;
    EnsPMapperpair trgpair2 = NULL;

    debug = ajDebugTest("mapperMergePairs");

    if(debug)
    {
        ajDebug("mapperMergePairs\n"
                "  mapper %p\n",
                mapper);

        ensMapperTrace(mapper, 1);
    }

    if(!mapper)
        return ajFalse;

    trgtable = (AjPTable)
        ajTableFetch(mapper->Pairs, (const void *) mapper->TargetType);

    ajTableToarrayValues(trgtable, &valarray);

    for(i = 0; valarray[i]; i++)
    {
        trglist = (AjPList) valarray[i];

        trgiter = ajListIterNew(trglist);

        while(!ajListIterDone(trgiter))
        {
            trgpair1 = (EnsPMapperpair) ajListIterGet(trgiter);
            trgpair2 = (EnsPMapperpair) ajListIterGet(trgiter);

            /*
            ** If target Mapper Pair 2 is not defined, there are no more
            ** Mapper Pairs to compare.
            */

            if((!trgpair1) || (!trgpair2))
                break;

            /* Do not merge insertion or deletion Mapper Pairs. */

            if(trgpair1->InsertionDeletion || trgpair2->InsertionDeletion)
                continue;

            assert(trgpair1->Source);
            assert(trgpair1->Target);

            assert(trgpair2->Source);
            assert(trgpair2->Target);

            /* Merge overlapping Mapper Pairs. */

            if((trgpair1->Source->ObjectIdentifier ==
                trgpair2->Source->ObjectIdentifier) &&
               (trgpair1->Target->Start == trgpair2->Target->Start))
                delpair = trgpair2;
            /* Merge adjacent Mapper Pairs. */
            else if((trgpair1->Source->ObjectIdentifier ==
                     trgpair2->Source->ObjectIdentifier) &&
                    (trgpair1->Orientation == trgpair2->Orientation) &&
                    (trgpair1->Target->End == (trgpair2->Target->Start - 1)))
            {
                if(trgpair1->Orientation >= 0)
                {
                    /* Check for a potential parallel merge. */

                    if(trgpair1->Source->End == (trgpair2->Source->Start - 1))
                    {
                        if(debug)
                        {
                            ajDebug("mapperMergePairs merged %p with %p "
                                    "in parallel orientation.\n",
                                    trgpair1, trgpair2);

                            ensMapperpairTrace(trgpair1, 1);
                            ensMapperpairTrace(trgpair2, 1);
                        }

                        /* Merge in parallel orientation. */

                        trgpair1->Source->End = trgpair2->Source->End;
                        trgpair1->Target->End = trgpair2->Target->End;

                        delpair = trgpair2;
                    }
                }
                else
                {
                    /* Check for a potential anti-parallel merge. */

                    if(trgpair1->Source->Start == (trgpair2->Source->End + 1))
                    {
                        if(debug)
                        {
                            ajDebug("mapperMergePairs merged %p with %p "
                                    "in anti-parallel orientation.\n",
                                    trgpair1, trgpair2);

                            ensMapperpairTrace(trgpair1, 1);
                            ensMapperpairTrace(trgpair2, 1);
                        }

                        /* Merge in anti-parallel orientation. */

                        trgpair1->Source->Start = trgpair2->Source->Start;
                        trgpair1->Target->End = trgpair2->Target->End;

                        delpair = trgpair2;
                    }
                }
            }

            /* Remove the redundant Mapper Pair also from the source Table. */

            if(delpair)
            {
                ajListIterRemove(trgiter);

                srctable = (AjPTable)
                    ajTableFetch(mapper->Pairs,
                                 (const void *) mapper->SourceType);

                srclist = (AjPList)
                    ajTableFetch(srctable,
                                 (const void *)
                                 &delpair->Source->ObjectIdentifier);

                srciter = ajListIterNew(srclist);

                while(!ajListIterDone(srciter))
                {
                    srcpair = (EnsPMapperpair) ajListIterGet(srciter);

                    if(srcpair == delpair)
                    {
                        ajListIterRemove(srciter);

                        ensMapperpairDel(&srcpair);
                    }
                }

                ajListIterDel(&srciter);

                ensMapperpairDel(&delpair);
            }
        }

        ajListIterDel(&trgiter);

        mapper->PairCount = ajListGetLength(trglist);
    }

    AJFREE(valarray);

    return ajTrue;
}




/* @funcstatic mapperSort *****************************************************
**
** Sort Ensembl Mapper Pairs in an Ensembl Mapper.
** @cc Bio::EnsEMBL::Mapper::_sort
**
** @param [r] mapper [EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool mapperSort(EnsPMapper mapper)
{
    void **valarray = NULL;

    register ajuint i = 0;

    AjPTable table = NULL;

    if(ajDebugTest("mapperSort"))
    {
        ajDebug("mapperSort\n"
                "  mapper %p\n",
                mapper);

        ensMapperTrace(mapper, 1);
    }

    if(!mapper)
        return ajFalse;

    table = (AjPTable)
        ajTableFetch(mapper->Pairs, (const void *) mapper->SourceType);

    ajTableToarrayValues(table, &valarray);

    for(i = 0; valarray[i]; i++)
        ensMapperpairSortBySourceStartAscending((AjPList) valarray[i]);

    AJFREE(valarray);

    table = (AjPTable)
        ajTableFetch(mapper->Pairs, (const void *) mapper->TargetType);

    ajTableToarrayValues(table, &valarray);

    for(i = 0; valarray[i]; i++)
        ensMapperpairSortByTargetStartAscending((AjPList) valarray[i]);

    AJFREE(valarray);

    mapperMergePairs(mapper);

    mapper->IsSorted = ajTrue;

    return ajTrue;
}




/* @func ensMapperAddCoordinates **********************************************
**
** Store details of mapping between a source and a target region.
**
** @cc Bio::EnsEMBL::Mapper::add_map_coordinates
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] srcoid [ajuint] Source Object identifier
** @param [r] srcstart [ajint] Source start coordinate
** @param [r] srcend [ajint] Source end coordinate
** @param [r] ori [ajint] Orientation
** @param [r] trgoid [ajuint] Target Object identifier
** @param [r] trgstart [ajint] Target start coordinate
** @param [r] trgend [ajint] Target end coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperAddCoordinates(EnsPMapper mapper,
                               ajuint srcoid,
                               ajint srcstart,
                               ajint srcend,
                               ajint ori,
                               ajuint trgoid,
                               ajint trgstart,
                               ajint trgend)
{
    AjBool value = AJFALSE;

    EnsPMapperpair mp = NULL;

    if(ajDebugTest("ensMapperAddCoordinates"))
        ajDebug("ensMapperAddCoordinates\n"
                "  mapper %p\n"
                "  srcoid %u\n"
                "  srcstart %d\n"
                "  srcend %d\n"
                "  ori %d\n"
                "  trgoid %u\n"
                "  trgstart %d\n"
                "  trgend %d\n",
                mapper,
                srcoid,
                srcstart,
                srcend,
                ori,
                trgoid,
                trgstart,
                trgend);

    if(!mapper)
        return ajFalse;

    if(!srcoid)
        return ajFalse;

    if(!trgoid)
        return ajFalse;

    if((srcend - srcstart) != (trgend - trgstart))
        ajFatal("ensMapperAddCoordinates cannot deal with mis-lengthed "
                "mappings so far.\n");

    mp = ensMapperpairNewC(srcoid, srcstart, srcend,
                           trgoid, trgstart, trgend,
                           ori, ajFalse);

    value = ensMapperAddMapperpair(mapper, mp);

    ensMapperpairDel(&mp);

    return value;
}




/* @func ensMapperAddMapperunits **********************************************
**
** Insert Ensembl Mapper Units into an Ensembl Mapper.
**
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [u] srcmu [EnsPMapperunit] Source Ensembl Mapper Unit
** @param [u] trgmu [EnsPMapperunit] Target Ensembl Mapper Unit
** @param [r] ori [ajint] Relative orientation of the Ensembl Mapper Units
** @param [r] indel [AjBool] Insertion-deletion attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperAddMapperunits(EnsPMapper mapper,
                               EnsPMapperunit srcmu,
                               EnsPMapperunit trgmu,
                               ajint ori,
                               AjBool indel)
{
    AjBool value = AJFALSE;

    EnsPMapperpair mp = NULL;

    if(!mapper)
        return ajFalse;

    if(!srcmu)
        return ajFalse;

    if(!trgmu)
        return ajFalse;

    mp = ensMapperpairNew(srcmu, trgmu, ori, indel);

    value = ensMapperAddMapperpair(mapper, mp);

    ensMapperpairDel(&mp);

    return value;
}




/* @func ensMapperAddMapperpair ***********************************************
**
** Insert an Ensembl Mapper Pair into an Ensembl Mapper.
**
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [u] mp [EnsPMapperpair] Ensembl Mapper Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperAddMapperpair(EnsPMapper mapper, EnsPMapperpair mp)
{
    ajuint *Poid = NULL;

    AjPList list = NULL;

    AjPTable table = NULL;

    if(ajDebugTest("ensMapperAddMapperpair"))
    {
        ajDebug("ensMapperAddMapperpair\n"
                "  mapper %p\n"
                "  mp %p\n",
                mapper,
                mp);

        ensMapperpairTrace(mp, 1);
    }

    if(!mapper)
        return ajFalse;

    if(!mp)
        return ajFalse;

    if(!mp->Source)
        ajFatal("ensMapperAddMapperpair requires a Mapper Pair with a "
                "Source Mapper Unit.\n");

    if(!mp->Target)
        ajFatal("ensMapperAddMapperpair requires a Mapper Pair with a "
                "Target Mapper Unit.\n");

    /*
    ** Check for the matching Mapper Unit lengths if this is not a Mapper Pair
    ** reflecting an insertion or deletion.
    */

    if((!mp->InsertionDeletion) &&
       ((mp->Source->End - mp->Source->Start) !=
        (mp->Target->End - mp->Target->Start)))
        ajFatal("ensMapperAddMapperpair cannot deal with mis-lengthed "
                "mappings so far.\n");

    /*
    ** Insert the Mapper Pair into the source branch of the Mapper.
    **
    ** Search the first-level AJAX Table of Ensembl Mapper type AJAX Strings
    ** for the second-level AJAX Table of Ensembl Object Identifers.
    */

    table = (AjPTable) ajTableFetch(mapper->Pairs,
                                    (const void *) mapper->SourceType);

    if(table)
    {
        /*
        ** Search the second-level AJAX Table of Ensembl Object identifiers
        ** for the third-level AJAX List of Ensembl Mapper Pairs.
        */

        list = (AjPList) ajTableFetch(
            table,
            (const void *) &mp->Source->ObjectIdentifier);

        if(!list)
        {
            AJNEW0(Poid);

            *Poid = mp->Source->ObjectIdentifier;

            list = ajListNew();

            ajTablePut(table, (void *) Poid, (void *) list);
        }

        ajListPushAppend(list, (void *) ensMapperpairNewRef(mp));
    }
    else
        ajFatal("ensMapperAddMapperpair first-level AJAX Table for "
                "Ensembl Mapper source type '%S' has not been initialised.",
                mapper->SourceType);

    /*
    ** Insert the Mapper Pair into the target branch of the Mapper.
    **
    ** Search the first-level AJAX Table of Ensembl Mapper type AJAX Strings
    ** for the second-level AJAX Table of Ensembl Object Identifers.
    */

    table = (AjPTable) ajTableFetch(mapper->Pairs,
                                    (const void *) mapper->TargetType);

    if(table)
    {
        /*
        ** Search the second-level AJAX Table of Ensembl Object identifiers
        ** for the third-level AJAX List of Ensembl Mapper Pairs.
        */

        list = (AjPList) ajTableFetch(
            table,
            (const void *) &mp->Target->ObjectIdentifier);

        if(!list)
        {
            AJNEW0(Poid);

            *Poid = mp->Target->ObjectIdentifier;

            list = ajListNew();

            ajTablePut(table, (void *) Poid, (void *) list);
        }

        ajListPushAppend(list, (void *) ensMapperpairNewRef(mp));
    }
    else
        ajFatal("ensMapperAddMapperpair first-level AJAX Table for "
                "Ensembl Mapper target type '%S' has not been initialised.",
                mapper->TargetType);

    mapper->PairCount++;

    mapper->IsSorted = ajFalse;

    return ajTrue;
}




/* @func ensMapperListPairs ***************************************************
**
** List Ensembl Mapper Pairs in an Ensembl Mapper.
** The caller is responsible for deleting the Ensembl Mapper Pairs before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Mapper::list_pairs
** @param [r] mapper [EnsPMapper] Ensembl Mapper
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] type [const AjPStr] Ensembl Mapper type
** @param [u] mps [AjPList] AJAX List of Ensembl Mapper Pairs
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperListPairs(EnsPMapper mapper,
                          ajuint oid,
                          ajint start,
                          ajint end,
                          const AjPStr type,
                          AjPList mps)
{
    EnsEMapperunitType mutype = ensEMapperunitTypeNULL;

    AjPList list = NULL;
    AjIList iter = NULL;

    AjPTable table = NULL;

    EnsPMapperpair mp = NULL;

    EnsPMapperunit mu = NULL;

    if(!mapper)
        return ajFalse;

    if(!oid)
        return ajFalse;

    if(!type)
        return ajFalse;

    if(!mapper->IsSorted)
        mapperSort(mapper);

    if(start > end)
        ajFatal("ensMapperListPairs start (%d) is greater than end (%d) "
                "for Ensembl Object identifier %u.\n",
                start, end, oid);

    if(ajStrMatchCaseS(mapper->SourceType, type))
        mutype = ensEMapperunitTypeSource;
    else if(ajStrMatchCaseS(mapper->TargetType, type))
        mutype = ensEMapperunitTypeTarget;
    else
        ajFatal("ensMapperListPairs type '%S' is neither the source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type, mapper->SourceType, mapper->TargetType);

    table = (AjPTable) ajTableFetch(mapper->Pairs, (const void *) type);

    if(!table)
        ajFatal("ensMapperListPairs first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.",
                type);

    list = (AjPList) ajTableFetch(table, (const void *) &oid);

    if(!list)
        return ajTrue;

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        mp = (EnsPMapperpair) ajListIterGet(iter);

        if((!start) && (!end))
        {
            ajListPushAppend(mps, (void *) ensMapperpairNewRef(mp));

            continue;
        }

        mu = ensMapperpairGetUnit(mp, mutype);

        if(mu->End < start)
            continue;

        if(mu->Start > end)
            break;

        ajListPushAppend(mps, (void *) ensMapperpairNewRef(mp));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic mapperMapInsert ************************************************
**
** Internal function to handle the special mapping case for inserts, where by
** Ensembl convention (start == end + 1). This function will be called
** automatically by the map function so there is no reason to call it directly.
** The caller is responsible for deleting the Ensembl Mapper Results before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Mapper::map_insert
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [r] type [const AjPStr] Ensembl Mapper source type
** @param [r] fastmap [AjBool] Fast-mapping attribute
** @param [u] mrs [AjPList] AJAX list of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool mapperMapInsert(EnsPMapper mapper,
                              ajuint oid,
                              ajint start,
                              ajint end,
                              ajint strand,
                              const AjPStr type,
                              AjBool fastmap,
                              AjPList mrs)
{
    ajint srcstart = 0;
    ajint srcend   = 0;

    AjPList coords = NULL;

    EnsPMapperresult mr = NULL;

    if(!mapper)
        return ajFalse;

    if(!oid)
        return ajFalse;

    if(ajDebugTest("mapperMapInsert"))
        ajDebug("mapperMapInsert\n"
                "  mapper %p\n"
                "  oid %u\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  type '%S'\n"
                "  fastmap '%B'\n"
                "  mrs %p\n",
                mapper,
                oid,
                start,
                end,
                strand,
                type,
                fastmap,
                mrs);

    srcstart = end;
    srcend   = start;

    coords = ajListNew();

    ensMapperMapCoordinates(mapper, oid, srcstart, srcend, strand,
                            type, coords);

    if(ajListGetLength(coords) == 1)
    {
        ajListPop(coords, (void **) &mr);

        /*
        ** Swap start and end to convert back into an insert where
        ** (start == end + 1)
        */

        srcstart = mr->End;
        srcend   = mr->Start;

        mr->Start = srcstart;
        mr->End   = srcend;

        ajListPushAppend(mrs, (void *) ensMapperresultNewRef(mr));

        ensMapperresultDel(&mr);
    }
    else
    {
        if(ajListGetLength(coords) != 2)
            ajFatal("mapperMapInsert got %d Ensembl Mapper Pairs "
                    "but expected only two.\n",
                    ajListGetLength(coords));

        /* Adjust coordinates and remove gaps. */

        if(strand < 0)
            ajListReverse(coords);

        ajListPop(coords, (void **) &mr);

        if(mr->Type == ensEMapperresultCoordinate)
        {
            /* The insert is after the first coordinate. */

            if((mr->Strand * strand) < 0)
                mr->End--;
            else
                mr->Start++;

            ajListPushAppend(mrs, (void *) ensMapperresultNewRef(mr));
        }

        ensMapperresultDel(&mr);

        ajListPop(coords, (void **) &mr);

        if(mr->Type == ensEMapperresultCoordinate)
        {
            /* The insert is before the second coordinate. */

            if((mr->Strand * strand) < 0)
                mr->Start++;
            else
                mr->End++;

            if(strand < 0)
                ajListPush(mrs, (void *) ensMapperresultNewRef(mr));
            else
                ajListPushAppend(mrs, (void *) ensMapperresultNewRef(mr));
        }

        ensMapperresultDel(&mr);
    }

    if(fastmap && (ajListGetLength(mrs) != 1))
        while(ajListPop(mrs, (void **) &mr))
            ensMapperresultDel(&mr);

    ajListFree(&coords);

    return ajTrue;
}




/* @func ensMapperMapCoordinates **********************************************
**
** Map Coordinates.
**
** The caller is responsible for deleting the Ensembl Mapper Results before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Mapper::map_coordinates
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [r] type [const AjPStr] Ensembl Mapper type to map from
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperMapCoordinates(EnsPMapper mapper,
                               ajuint oid,
                               ajint start,
                               ajint end,
                               ajint strand,
                               const AjPStr type,
                               AjPList mrs)
{
    register ajuint i = 0;

    ajuint idxstart  = 0;
    ajuint idxmid    = 0;
    ajuint idxend    = 0;
    ajuint idxlength = 0;
    ajuint rank      = 0;

    ajint srcstart = 0;
    ajint srcend   = 0;

    ajuint trgoid  = 0;
    ajint trgstart = 0;
    ajint trgend   = 0;

    AjBool debug = AJFALSE;

    EnsEMapperunitType srctype = ensEMapperunitTypeNULL;
    EnsEMapperunitType trgtype = ensEMapperunitTypeNULL;

    AjPList list = NULL;

    AjPTable table = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPMapperpair mp     = NULL;
    EnsPMapperpair lastmp = NULL;

    EnsPMapperresult mr = NULL;

    EnsPMapperunit srcmu = NULL;
    EnsPMapperunit trgmu = NULL;

    debug = ajDebugTest("ensMapperMapCoordinates");

    if(debug)
        ajDebug("ensMapperMapCoordinates\n"
                "  mapper %p\n"
                "  oid %u\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  type '%S'\n"
                "  mrs %p\n",
                mapper,
                oid,
                start,
                end,
                strand,
                type,
                mrs);

    if(!mapper)
        return ajFalse;

    if(!oid)
        return ajFalse;

    if(!type)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    if(start == (end + 1))
        return mapperMapInsert(mapper, oid, start, end, strand, type,
                               ajFalse, mrs);

    if(!mapper->IsSorted)
        mapperSort(mapper);

    if(ajStrMatchCaseS(mapper->SourceType, type))
    {
        srctype = ensEMapperunitTypeSource;
        trgtype = ensEMapperunitTypeTarget;

        cs = mapper->TargetCoordsystem;
    }
    else if(ajStrMatchCaseS(mapper->TargetType, type))
    {
        srctype = ensEMapperunitTypeTarget;
        trgtype = ensEMapperunitTypeSource;

        cs = mapper->SourceCoordsystem;
    }
    else
        ajFatal("ensMapperMapCoordinates type '%S' is neither the "
                "source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type,
                mapper->SourceType,
                mapper->TargetType);

    table = (AjPTable) ajTableFetch(mapper->Pairs, (const void *) type);

    if(!table)
        ajFatal("ensMapperMapCoordinates first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.\n",
                type);

    list = (AjPList) ajTableFetch(table, (const void *) &oid);

    if(!(list && (idxlength = ajListGetLength(list))))
    {
        /*
        ** If an Ensembl Object identifier is not associated with an
        ** AJAX List of Ensembl Mapper Pairs or the AJAX List is empty,
        ** the whole region is just one big gap.
        */

        if(debug)
            ajDebug("ensMapperMapCoordinates could not find an AJAX List for "
                    "Ensembl Object identifier %u or the List is empty "
                    "--> one big gap!\n",
                    oid);

        mr = MENSMAPPERGAPNEW(start, end, 0);

        ajListPushAppend(mrs, (void *) mr);

        return ajTrue;
    }

    /*
    ** Binary search the relevant Ensembl Mapper Pairs,
    ** which helps if the AJAX List of Ensembl Mapper Pairs is long.
    */

    idxstart = 0;
    idxend   = idxlength - 1;

    while((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &mp);

        srcmu = ensMapperpairGetUnit(mp, srctype);

        if(srcmu->End < start)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    srcstart = start;
    srcend   = end;

    for(i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &mp);

        srcmu = ensMapperpairGetUnit(mp, srctype);
        trgmu = ensMapperpairGetUnit(mp, trgtype);

        if(debug)
            ajDebug("ensMapperMapCoordinates coordinates "
                    "%u:%d:%d:%d %d:%d srcMU %u:%d:%d\n",
                    oid,
                    start,
                    end,
                    strand,
                    srcstart,
                    srcend,
                    srcmu->ObjectIdentifier,
                    srcmu->Start,
                    srcmu->End);

        if(srcmu->Start < start)
        {
            srcstart = start;

            rank++;
        }

        /*
        ** Check for cases where the source Mapper Unit maps to more than one
        ** location.
        ** If the target object identifier changes, and the source start
        ** coordinate is less than the current start coordinate, this source
        ** Mapper Unit maps to more than one location on a target Mapper Unit.
        ** Reset the current source start position to the original start.
        */

        if(trgoid && (trgoid != trgmu->ObjectIdentifier))
        {
            if(srcmu->Start < start)
                srcstart = start;
        }
        else
            trgoid = trgmu->ObjectIdentifier;

        /* In case the loop hasn't even reached the start, move on. */

        if(srcmu->End < start)
            continue;

        /* In case the loop has over-run, break. */

        if(srcmu->Start > srcend)
            break;

        if(srcmu->Start > srcstart)
        {
            /* A gap has been detected. */

            mr = MENSMAPPERGAPNEW(srcstart, srcmu->Start - 1, rank);

            ajListPushAppend(mrs, (void *) mr);

            srcstart = srcmu->Start;
        }

        if(mp->InsertionDeletion)
        {
            /*
            ** If the Mapper Pair represents an insertion or deletion,
            ** create a Mapper Result of type insertion or deletion.
            */

            mr = MENSMAPPERINDELNEW(trgmu->ObjectIdentifier,
                                    trgmu->Start,
                                    trgmu->End,
                                    mp->Orientation * strand,
                                    cs,
                                    srcstart,
                                    (srcmu->End < srcend) ?
                                    srcmu->End : srcend);
        }
        else
        {
            /* The start is somewhere inside the region. */

            if(mp->Orientation >= 0)
                trgstart = trgmu->Start + (srcstart - srcmu->Start);
            else
                trgend = trgmu->End - (srcstart - srcmu->Start);

            /*
            ** Either we are enveloping this map or not. If yes, then the end
            ** point (self perspective) is determined solely by target.
            ** If not we need to adjust.
            */

            if(srcend > srcmu->End)
            {
                /* enveloped */

                if(mp->Orientation >= 0)
                    trgend = trgmu->End;
                else
                    trgstart = trgmu->Start;
            }
            else
            {
                /* The end needs to be adjusted. */

                if(mp->Orientation >= 0)
                    trgend = trgmu->Start + (srcend - srcmu->Start);
                else
                    trgstart = trgmu->End - (srcend - srcmu->Start);
            }

            mr = MENSMAPPERCOORDINATENEW(trgmu->ObjectIdentifier,
                                         trgstart,
                                         trgend,
                                         mp->Orientation * strand,
                                         cs,
                                         rank);
        }

        ajListPushAppend(mrs, (void *) mr);

        lastmp = mp;

        srcstart = srcmu->End + 1;
    }

    if(lastmp)
    {
        /*
        ** Previously, a Mapper Pair has been found,
        ** check for a gap inbetween.
        */

        srcmu = ensMapperpairGetUnit(lastmp, srctype);

        if(srcmu->End < srcend)
        {
            /* A gap at the end has been detected. */

            mr = MENSMAPPERGAPNEW(srcmu->End + 1, srcend, rank);

            ajListPushAppend(mrs, (void *) mr);
        }
    }
    else
    {
        /* Since no Mapper Pair has been found the entire region is a gap. */

        mr = MENSMAPPERGAPNEW(srcstart, srcend, 0);

        ajListPushAppend(mrs, (void *) mr);
    }

    if(strand < 0)
        ajListReverse(mrs);

    return ajTrue;
}




/* @func ensMapperFastMap *****************************************************
**
** Inferior mapping function, which will only perform ungapped,
** unsplit mapping.
**
** The caller is responsible for deleting the Ensembl Mapper Results before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Mapper::fastmap
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [r] type [const AjPStr] Ensembl Mapper type to map from
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperFastMap(EnsPMapper mapper,
                        ajuint oid,
                        ajint start,
                        ajint end,
                        ajint strand,
                        const AjPStr type,
                        AjPList mrs)
{
    AjBool debug = AJFALSE;

    EnsEMapperunitType srctype = ensEMapperunitTypeNULL;
    EnsEMapperunitType trgtype = ensEMapperunitTypeNULL;

    AjIList iter = NULL;
    AjPList list = NULL;

    AjPTable table = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPMapperpair mp = NULL;

    EnsPMapperresult mr = NULL;

    EnsPMapperunit srcmu = NULL;
    EnsPMapperunit trgmu = NULL;

    debug = ajDebugTest("ensMapperFastMap");

    if(debug)
        ajDebug("ensMapperFastMap\n"
                "  mapper %p\n"
                "  oid %u\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  type '%S'\n"
                "  mrs %p\n",
                mapper,
                oid,
                start,
                end,
                strand,
                type,
                mrs);

    if(!mapper)
        return ajFalse;

    if(!oid)
        return ajFalse;

    if(!type)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    if(start == (end + 1))
        return mapperMapInsert(mapper, oid, start, end, strand, type,
                               ajTrue, mrs);

    if(!mapper->IsSorted)
        mapperSort(mapper);

    if(ajStrMatchCaseS(mapper->SourceType, type))
    {
        srctype = ensEMapperunitTypeSource;
        trgtype = ensEMapperunitTypeTarget;

        cs = mapper->TargetCoordsystem;
    }
    else if(ajStrMatchCaseS(mapper->TargetType, type))
    {
        srctype = ensEMapperunitTypeTarget;
        trgtype = ensEMapperunitTypeSource;

        cs = mapper->SourceCoordsystem;
    }
    else
        ajFatal("ensMapperFastMap type '%S' is neither the "
                "source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type,
                mapper->SourceType,
                mapper->TargetType);

    table = (AjPTable) ajTableFetch(mapper->Pairs, (const void *) type);

    if(!table)
        ajFatal("ensMapperFastMap first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.\n",
                type);

    list = (AjPList) ajTableFetch(table, (const void *) &oid);

    if(!(list && ajListGetLength(list)))
    {
        /*
        ** If an Ensembl Object identifier is not associated with an
        ** AJAX List of Ensembl Mapper Pairs or the AJAX List is empty,
        ** the whole region is just one big gap. The ensMapperFastMap
        ** function returns no Ensembl Mapper Result.
        */

        if(debug)
            ajDebug("ensMapperFastMap could not find an AJAX List for "
                    "Ensembl Object identifier %u or the List is empty "
                    "--> one big gap!\n",
                    oid);

        return ajTrue;
    }

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        mp = (EnsPMapperpair) ajListIterGet(iter);

        srcmu = ensMapperpairGetUnit(mp, srctype);
        trgmu = ensMapperpairGetUnit(mp, trgtype);

        /* Only super easy mapping is done! */

        if((start < srcmu->Start) || (end > srcmu->End))
            continue;

        if(mp->Orientation >= 0)
            mr = MENSMAPPERCOORDINATENEW(trgmu->ObjectIdentifier,
                                         trgmu->Start + (start - srcmu->Start),
                                         trgmu->Start + (end - srcmu->Start),
                                         +strand,
                                         cs,
                                         0);
        else
            mr = MENSMAPPERCOORDINATENEW(trgmu->ObjectIdentifier,
                                         trgmu->End - (end - srcmu->Start),
                                         trgmu->End - (start - srcmu->Start),
                                         -strand,
                                         cs,
                                         0);

        ajListPushAppend(mrs, (void *) mr);

        break;
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensMapperAddInDelCoordinates *****************************************
**
** Store details of mapping between a source and a target region.
**
** @cc Bio::EnsEMBL::Mapper::add_indel_coordinates
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] srcoid [ajuint] Source Object identifier
** @param [r] srcstart [ajint] Source start coordinate
** @param [r] srcend [ajint] Source end coordinate
** @param [r] ori [ajint] Orientation
** @param [r] trgoid [ajuint] Target Object identifier
** @param [r] trgstart [ajint] Target start coordinate
** @param [r] trgend [ajint] Target end coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperAddInDelCoordinates(EnsPMapper mapper,
                                    ajuint srcoid,
                                    ajint srcstart,
                                    ajint srcend,
                                    ajint ori,
                                    ajuint trgoid,
                                    ajint trgstart,
                                    ajint trgend)
{
    AjBool value = AJFALSE;

    EnsPMapperpair mp = NULL;

    if(!mapper)
        return ajFalse;

    if(!srcoid)
        return ajFalse;

    if(!trgoid)
        return ajFalse;

    mp = ensMapperpairNewC(srcoid, srcstart, srcend,
                           trgoid, trgstart, trgend,
                           ori, ajTrue);

    value = ensMapperAddMapperpair(mapper, mp);

    ensMapperpairDel(&mp);

    return value;
}




/* @func ensMapperMapInDel ****************************************************
**
** Map InDel Coordinates.
** The caller is responsible for deleting the Ensembl Mapper Results before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Mapper::map_indel
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [r] type [const AjPStr] Ensembl Mapper type to map from
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperMapInDel(EnsPMapper mapper,
                         ajuint oid,
                         ajint start,
                         ajint end,
                         ajint strand,
                         const AjPStr type,
                         AjPList mrs)
{
    register ajuint i = 0;

    ajuint idxstart  = 0;
    ajuint idxend    = 0;
    ajuint idxmid    = 0;
    ajuint idxlength = 0;

    AjBool debug = AJFALSE;

    EnsEMapperunitType srctype = ensEMapperunitTypeNULL;
    EnsEMapperunitType trgtype = ensEMapperunitTypeNULL;

    AjPList list = NULL;

    AjPTable table = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPMapperpair mp = NULL;

    EnsPMapperresult mr = NULL;

    EnsPMapperunit srcmu = NULL;
    EnsPMapperunit trgmu = NULL;

    debug = ajDebugTest("ensMapperMapInDel");

    if(debug)
        ajDebug("ensMapperMapInDel\n"
                "  mapper %p\n"
                "  oid %u\n"
                "  start %u\n"
                "  end %u\n"
                "  strand %d\n"
                "  type '%S'"
                "  mrs %p\n",
                mapper,
                oid,
                start,
                end,
                strand,
                type,
                mrs);

    if(!mapper)
        return ajFalse;

    if(!oid)
        return ajFalse;

    if(!type)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    if(!mapper->IsSorted)
        mapperSort(mapper);

    if(ajStrMatchCaseS(mapper->SourceType, type))
    {
        srctype = ensEMapperunitTypeSource;
        trgtype = ensEMapperunitTypeTarget;

        cs = mapper->TargetCoordsystem;
    }
    else if(ajStrMatchCaseS(mapper->TargetType, type))
    {
        srctype = ensEMapperunitTypeTarget;
        trgtype = ensEMapperunitTypeSource;

        cs = mapper->SourceCoordsystem;
    }
    else
        ajFatal("ensMapperMapInDel type '%S' is neither the "
                "source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type,
                mapper->SourceType,
                mapper->TargetType);

    table = (AjPTable) ajTableFetch(mapper->Pairs, (const void *) type);

    if(!table)
        ajFatal("ensMapperMapInDel first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.",
                type);

    list = (AjPList) ajTableFetch(table, (const void *) &oid);

    if(!(list && (idxlength = ajListGetLength(list))))
    {
        /*
        ** If an Ensembl Object identifier is not associated with an
        ** AJAX List of Ensembl Mapper Pairs or the AJAX List is empty,
        ** the whole region is just one big gap.
        */

        if(debug)
            ajDebug("ensMapperMapInDel could not find an AJAX List for "
                    "Ensembl Object identifier %u or the List is empty\n",
                    oid);

        return ajTrue;
    }

    /*
    ** Binary search the relevant Ensembl Mapper Pairs,
    ** which helps if the AJAX List of Ensembl Mapper Pairs is long.
    */

    idxstart = 0;
    idxend   = idxlength - 1;

    while((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &mp);

        srcmu = ensMapperpairGetUnit(mp, srctype);

        /*
        ** NOTE: The ensMapperMapCoordinates function checks for source
        ** Mapper Unit end less than start. if(srcmu->End < start)
        ** This function has already swapped the start and end coordinates
        ** for the insertion-deletion and checks for source Mapper Unit end
        ** less than *or equal* the start. Since the coordinates have not
        ** been swapped here, this becomes if(srcmu->End <= end)
        */

        if(srcmu->End <= end)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    for(i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &mp);

        trgmu = ensMapperpairGetUnit(mp, trgtype);

        if(mp->InsertionDeletion)
        {
            mr = MENSMAPPERCOORDINATENEW(trgmu->ObjectIdentifier,
                                         trgmu->Start,
                                         trgmu->End,
                                         mp->Orientation * strand,
                                         cs,
                                         0);

            ajListPushAppend(mrs, (void *) mr);

            break;
        }
    }

    return ajTrue;
}




/* @func ensMapperAddMapper ***************************************************
**
** Transfer all Ensembl Mapper Pairs from the second into the first
** Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::add_Mapper
** @param [u] mapper1 [EnsPMapper] First Ensembl Mapper
** @param [u] mapper2 [EnsPMapper] Second Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperAddMapper(EnsPMapper mapper1, EnsPMapper mapper2)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    ajuint srccounter = 0;
    ajuint trgcounter = 0;

    AjPList list1 = NULL;
    AjPList list2 = NULL;

    AjPTable table1 = NULL;
    AjPTable table2 = NULL;

    EnsPMapperpair mp = NULL;

    if(!mapper1)
        return ajFalse;

    if(!mapper2)
        return ajFalse;

    if(!ajStrMatchCaseS(mapper1->SourceType, mapper2->SourceType) ||
       !ajStrMatchCaseS(mapper1->TargetType, mapper2->TargetType))
        ajFatal("ensMapperAddMapper trying to add Ensembl Mapper of "
                "incompatible type ('%S:%S' vs '%S:%S').",
                mapper1->SourceType,
                mapper1->TargetType,
                mapper2->SourceType,
                mapper2->TargetType);

    /*
    ** Get the first-level AJAX Tables for the source type of both
    ** Ensembl Mappers.
    */

    table1 = (AjPTable)
        ajTableFetch(mapper1->Pairs, (const void *) mapper1->SourceType);

    if(!table1)
        ajFatal("ensMapperAddMapper first-level AJAX Table for first "
                "Ensembl Mapper source type '%S' not initialised.",
                mapper1->SourceType);

    table2 = (AjPTable)
        ajTableFetch(mapper2->Pairs, (const void *) mapper2->SourceType);

    if(!table2)
        ajFatal("ensMapperAddMapper first-level AJAX Table for second "
                "Ensembl Mapper source type '%S' not initialised.",
                mapper2->SourceType);

    /*
    ** Convert the second-level AJAX Table with Ensembl Object
    ** identifiers as key data and AJAX List value data for the second
    ** Ensembl Mapper.
    */

    ajTableToarrayKeysValues(table2, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        /*
        ** Get the third-level AJAX Lists for corresponding
        ** Ensembl Object identifiers.
        */

        list1 = (AjPList) ajTableFetch(table1, (const void *) &keyarray[i]);

        list2 = (AjPList) valarray[i];

        while(ajListPop(list2, (void **) &mp))
        {
            ajListPushAppend(list1, (void *) mp);

            srccounter++;
        }

        /*
        ** Remove the entry from the AJAX Table and free the
        ** Object identifier and the AJAX List.
        */

        ajTableRemove(table2, (const void *) keyarray[i]);

        AJFREE(keyarray[i]);

        ajListFree(&list2);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    /*
    ** Get the first-level AJAX Tables for the target type of both
    ** Ensembl Mappers.
    */

    table1 = (AjPTable)
        ajTableFetch(mapper1->Pairs, (const void *) mapper1->TargetType);

    if(!table1)
        ajFatal("ensMapperAddMapper first-level AJAX Table for first "
                "Ensembl Mapper target type '%S' not initialised.",
                mapper1->TargetType);

    table2 = (AjPTable)
        ajTableFetch(mapper2->Pairs, (const void *) mapper2->TargetType);

    if(!table2)
        ajFatal("ensMapperAddMapper first-level AJAX Table for second "
                "Ensembl Mapper target type '%S' not initialised.",
                mapper2->TargetType);

    /*
    ** Convert the second-level AJAX Table with Ensembl Object identifiers
    ** as key data and AJAX List value data for the second Ensembl Mapper.
    */

    ajTableToarrayKeysValues(table2, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        /*
        ** Get the third-level AJAX Lists for corresponding
        ** Ensembl Object identifiers.
        */

        list1 = (AjPList) ajTableFetch(table1, (const void *) &keyarray[i]);

        list2 = (AjPList) valarray[i];

        while(ajListPop(list2, (void **) &mp))
        {
            ajListPushAppend(list1, (void *) mp);

            trgcounter++;
        }

        /*
        ** Remove the entry from the AJAX Table and free the
        ** Ensembl Object identifier and the AJAX List.
        */

        ajTableRemove(table2, (const void *) keyarray[i]);

        AJFREE(keyarray[i]);

        ajListFree(&list2);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    if(srccounter == trgcounter)
        mapper1->PairCount += srccounter;
    else
        ajFatal("ensMapperAddMapper Ensembl Mapper Pairs for source (%u) and "
                "target (%u) types do not match in the second Ensembl Mapper.",
                srccounter, trgcounter);

    mapper1->IsSorted = ajFalse;

    return ajTrue;
}




/* @func ensMapperGetMemsize **************************************************
**
** Get the memory size in bytes of an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensMapperGetMemsize(const EnsPMapper mapper)
{
    void **keyarray1 = NULL;
    void **valarray1 = NULL;

    void **valarray2 = NULL;

    register ajuint i = 0;
    register ajuint j = 0;

    ajulong size = 0;

    AjIList iter = NULL;

    EnsPMapperpair mp = NULL;

    if(!mapper)
        return 0;

    size += sizeof (EnsOMapper);

    if(mapper->SourceType)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mapper->SourceType);
    }

    if(mapper->TargetType)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mapper->TargetType);
    }

    size += ensCoordsystemGetMemsize(mapper->SourceCoordsystem);
    size += ensCoordsystemGetMemsize(mapper->TargetCoordsystem);

    /* Level 0 data (AjOTable). */

    size += sizeof (AjOTable);

    ajTableToarrayKeysValues(mapper->Pairs, &keyarray1, &valarray1);

    for(i = 0; valarray1[i]; i++)
    {
        /* Level 1 key data (AjOStr). */

        size += sizeof (AjOStr);

        size += ajStrGetRes((AjPStr) keyarray1[i]);

        /* Level 1 value data (AjOTable). */

        size += sizeof (AjOTable);

        ajTableToarrayValues(valarray1[i], &valarray2);

        for(j = 0; valarray2[j]; j++)
        {
            /* Level 2 key data (ajuint). */

            size += sizeof (ajuint);

            /* Level 2 value data (AjOList). */

            size += sizeof (AjOList);

            iter = ajListIterNew((AjPList) valarray2[j]);

            while(!ajListIterDone(iter))
            {
                /* Level 3 data (EnsOMapperpair). */

                mp = (EnsPMapperpair) ajListIterGet(iter);

                size += ensMapperpairGetMemsize(mp);
            }

            ajListIterDel(&iter);
        }

        AJFREE(valarray2);
    }

    AJFREE(keyarray1);
    AJFREE(valarray1);

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper object.
**
** @fdata [EnsPMapper]
** @nam3rule Trace Report Ensembl Mapper elements to debug file
**
** @argrule Trace mapper [const EnsPMapper] Ensembl Mapper
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperTrace *******************************************************
**
** Trace an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMapperTrace(const EnsPMapper mapper, ajuint level)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    ajuint *Poid = NULL;

    AjIList iter = NULL;

    AjPStr indent = NULL;

    AjPTable table = NULL;

    EnsPMapperpair mp = NULL;

    if(!mapper)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperTrace %p\n"
            "%S  SourceType '%S'\n"
            "%S  TargetType '%S'\n"
            "%S  SourceCoordsystem %p\n"
            "%S  TargetCoordsystem %p\n"
            "%S  Pairs %p\n"
            "%S  PairCount %u\n"
            "%S  IsSorted '%B'\n"
            "%S  Use %u\n",
            indent, mapper,
            indent, mapper->SourceType,
            indent, mapper->TargetType,
            indent, mapper->SourceCoordsystem,
            indent, mapper->TargetCoordsystem,
            indent, mapper->Pairs,
            indent, mapper->PairCount,
            indent, mapper->IsSorted,
            indent, mapper->Use);

    ensCoordsystemTrace(mapper->SourceCoordsystem, level + 1);

    ensCoordsystemTrace(mapper->TargetCoordsystem, level + 1);

    /* Trace the AJAX Table for the SourceType. */

    table = (AjPTable)
        ajTableFetch(mapper->Pairs, (const void *) mapper->SourceType);

    ajDebug("%S  AJAX Table %p for SourceType '%S'\n",
            indent, table, mapper->SourceType);

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

    for(i = 0; valarray[i]; i++)
    {
        Poid = (ajuint *) keyarray[i];

        ajDebug("%S    AJAX List %p for Object identifier %u\n",
                indent, valarray[i], *Poid);

        iter = ajListIterNew((AjPList) valarray[i]);

        while(!ajListIterDone(iter))
        {
            mp = (EnsPMapperpair) ajListIterGet(iter);

            ensMapperpairTrace(mp, level + 3);
        }

        ajListIterDel(&iter);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    /* Trace the AJAX Table for the TargetType. */

    table = (AjPTable)
        ajTableFetch(mapper->Pairs, (const void *) mapper->TargetType);

    ajDebug("%S  AJAX Table %p for TargetType '%S'\n",
            indent, table, mapper->TargetType);

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

    for(i = 0; valarray[i]; i++)
    {
        Poid = (ajuint *) keyarray[i];

        ajDebug("%S    AJAX List %p for Object identifier %u\n",
                indent, valarray[i], *Poid);

        iter = ajListIterNew((AjPList) valarray[i]);

        while(!ajListIterDone(iter))
        {
            mp = (EnsPMapperpair) ajListIterGet(iter);

            ensMapperpairTrace(mp, level + 3);
        }

        ajListIterDel(&iter);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajStrDel(&indent);

    return ajTrue;
}

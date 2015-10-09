/* @source ensmapper **********************************************************
**
** Ensembl Mapper functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.50 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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

#include "ensmapper.h"
#include "enstable.h"




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




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listMapperpairCompareSourceStartAscending(
    const void *item1,
    const void *item2);

static int listMapperpairCompareTargetStartAscending(
    const void *item1,
    const void *item2);

static void mapperrangeregistryListMapperrangeValdel(void **Pvalue);

static void mapperListMapperpairValdel(void **Pvalue);

static void mapperMapperpairsClear(const void *key,
                                   void **Pvalue,
                                   void *cl);

static AjBool mapperMapperpairsMerge(EnsPMapper mapper);

static AjBool mapperMapperpairsSort(EnsPMapper mapper);

static AjBool mapperMapInsert(EnsPMapper mapper,
                              ajuint oid,
                              ajint start,
                              ajint end,
                              ajint strand,
                              const AjPStr type,
                              AjBool fastmap,
                              AjPList mrs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensmapper *****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMapperunit] Ensembl Mapper Unit **************************
**
** @nam2rule Mapperunit Functions for manipulating Ensembl Mapper Unit objects
**
** @cc Bio::EnsEMBL::Mapper::Unit
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-66
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mu [const EnsPMapperunit] Ensembl Mapper Unit
** @argrule Ini oid [ajuint] Ensembl Object identifier
** @argrule Ini start [ajint] Start coordinate
** @argrule Ini end [ajint] End coordinate
** @argrule Ref mu [EnsPMapperunit] Ensembl Mapper Unit
**
** @valrule * [EnsPMapperunit] Ensembl Mapper Unit or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMapperunitNewCpy **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [EnsPMapperunit] Ensembl Mapper Unit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperunit ensMapperunitNewCpy(const EnsPMapperunit mu)
{
    EnsPMapperunit pthis = NULL;

    if (!mu)
        return NULL;

    AJNEW0(pthis);

    pthis->Objectidentifier = mu->Objectidentifier;

    pthis->Start = mu->Start;
    pthis->End   = mu->End;
    pthis->Use   = 1U;

    return pthis;
}




/* @func ensMapperunitNewIni **************************************************
**
** Constructor for an Ensembl Mapper Unit with initial values.
**
** @cc Bio::EnsEMBL::Mapper::Unit::new
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
**
** @return [EnsPMapperunit] Ensembl Mapper Unit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperunit ensMapperunitNewIni(ajuint oid, ajint start, ajint end)
{
    EnsPMapperunit mu = NULL;

    if (!oid)
        return NULL;

    AJNEW0(mu);

    mu->Objectidentifier = oid;

    mu->Start = start;
    mu->End   = end;
    mu->Use   = 1U;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperunit ensMapperunitNewRef(EnsPMapperunit mu)
{
    if (!mu)
        return NULL;

    mu->Use++;

    return mu;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Unit
**
** @argrule * Pmu [EnsPMapperunit*] Ensembl Mapper Unit address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperunitDel *****************************************************
**
** Default destructor for an Ensembl Mapper Unit.
**
** @param [d] Pmu [EnsPMapperunit*] Ensembl Mapper Unit address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMapperunitDel(EnsPMapperunit *Pmu)
{
    EnsPMapperunit pthis = NULL;

    if (!Pmu)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMapperunitDel"))
    {
        ajDebug("ensMapperunitDel\n"
                "  *Pmu %p\n",
                *Pmu);

        ensMapperunitTrace(*Pmu, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmu)
        return;

    pthis = *Pmu;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmu = NULL;

        return;
    }

    AJFREE(pthis);

    *Pmu = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
**
** @nam3rule Get Return Ensembl Mapper Unit attribute(s)
** @nam4rule End Return the end coordinate
** @nam4rule Objectidentifier Return the Ensembl Object identifier
** @nam4rule Start Return the start coordinate
**
** @argrule * mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @valrule End [ajint] End coordinate or 0
** @valrule Objectidentifier [ajuint] Ensembl Object identifier or 0U
** @valrule Start [ajint] Start coordinate or 0
**
** @fcategory use
******************************************************************************/




/* @func ensMapperunitGetEnd **************************************************
**
** Get the end coordinate member of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::end
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajint] End coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperunitGetEnd(const EnsPMapperunit mu)
{
    return (mu) ? mu->End : 0;
}




/* @func ensMapperunitGetObjectidentifier *************************************
**
** Get the Ensembl Object identifier member of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::id
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajuint] Ensembl Object identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperunitGetObjectidentifier(const EnsPMapperunit mu)
{
    return (mu) ? mu->Objectidentifier : 0U;
}




/* @func ensMapperunitGetStart ************************************************
**
** Get the start coordinate member of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::start
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [ajint] Start coordinate or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperunitGetStart(const EnsPMapperunit mu)
{
    return (mu) ? mu->Start : 0;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
**
** @nam3rule Set Set one member of an Ensembl Mapper Unit
** @nam4rule End Set the end coordinate
** @nam4rule Objectidentifier Set the Ensembl Object identifier
** @nam4rule Start Set the start coordinate
**
** @argrule * mu [EnsPMapperunit] Ensembl Mapper Unit object
** @argrule End end [ajint] End coordinate
** @argrule Objectidentifier oid [ajuint] Ensembl Object identifier
** @argrule Start start [ajint] Start coordinate
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMapperunitSetEnd **************************************************
**
** Set the end coordinate member of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::end
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
** @param [r] end [ajint] End coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperunitSetEnd(EnsPMapperunit mu, ajint end)
{
    if (!mu)
        return ajFalse;

    mu->End = end;

    return ajTrue;
}




/* @func ensMapperunitSetObjectidentifier *************************************
**
** Set the Ensembl Object identifier member of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::id
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
** @param [r] oid [ajuint] Ensembl Object identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperunitSetObjectidentifier(EnsPMapperunit mu, ajuint oid)
{
    if (!mu)
        return ajFalse;

    if (!oid)
        return ajFalse;

    mu->Objectidentifier = oid;

    return ajTrue;
}




/* @func ensMapperunitSetStart ************************************************
**
** Set the start coordinate member of an Ensembl Mapper Unit.
**
** @cc Bio::EnsEMBL::Mapper::Unit::start
** @param [u] mu [EnsPMapperunit] Ensembl Mapper Unit
** @param [r] start [ajint] Start coordinate
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperunitSetStart(EnsPMapperunit mu, ajint start)
{
    if (!mu)
        return ajFalse;

    mu->Start = start;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
**
** @nam3rule Trace Report Ensembl Mapper Unit members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperunitTrace(const EnsPMapperunit mu, ajuint level)
{
    AjPStr indent = NULL;

    if (!mu)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperunitTrace %p\n"
            "%S  Objectidentifier %u\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Use %u\n",
            indent, mu,
            indent, mu->Objectidentifier,
            indent, mu->Start,
            indent, mu->End,
            indent, mu->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Mapper Unit object.
**
** @fdata [EnsPMapperunit]
**
** @nam3rule Calculate Calculate Ensembl Mapper Unit values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperunitCalculateMemsize ****************************************
**
** Calculate the memory size in bytes of an Ensembl Mapper Unit.
**
** @param [r] mu [const EnsPMapperunit] Ensembl Mapper Unit
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMapperunitCalculateMemsize(const EnsPMapperunit mu)
{
    size_t size = 0;

    if (!mu)
        return 0;

    size += sizeof (EnsOMapperunit);

    return size;
}




/* @datasection [EnsPMapperpair] Ensembl Mapper Pair **************************
**
** @nam2rule Mapperpair Functions for manipulating Ensembl Mapper Pair objects
**
** @cc Bio::EnsEMBL::Mapper::IndelPair
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-66
**
** @cc Bio::EnsEMBL::Mapper::Pair
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
** @nam4rule Unit Constructor with Ensembl Mapper Unit objects
**
** @argrule Cpy mp [const EnsPMapperpair] Ensembl Mapper Pair
** @argrule Ini srcoid [ajuint] Source Ensembl Object identifier
** @argrule Ini srcstart [ajint] Source Start coordinate
** @argrule Ini srcend [ajint] Source End coordinate
** @argrule Ini trgoid [ajuint] Target Ensembl Object identifier
** @argrule Ini trgstart [ajint] Target Start coordinate
** @argrule Ini trgend [ajint] Target End coordinate
** @argrule Ini ori [ajint]
** Relative orientation of the Ensembl Mapper Unit objects
** @argrule ensMapperpairNewIni indel [AjBool] Insertion-deletion attribute
** @argrule Unit source [EnsPMapperunit] Source Ensembl Mapper Unit
** @argrule Unit target [EnsPMapperunit] Target Ensembl Mapper Unit
** @argrule Unit ori [ajint]
** Relative orientation of the Ensembl Mapper Unit objects
** @argrule ensMapperpairNewUnit indel [AjBool] Insertion-deletion attribute
** @argrule Ref mp [EnsPMapperpair] Ensembl Mapper Pair
**
** @valrule * [EnsPMapperpair] Ensembl Mapper Pair or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMapperpairNewCpy **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewCpy(const EnsPMapperpair mp)
{
    EnsPMapperpair pthis = NULL;

    if (!mp)
        return NULL;

    AJNEW0(pthis);

    pthis->Source      = ensMapperunitNewCpy(mp->Source);
    pthis->Target      = ensMapperunitNewCpy(mp->Target);
    pthis->Orientation = mp->Orientation;
    pthis->Indel       = mp->Indel;
    pthis->Use         = 1U;

    return pthis;
}




/* @func ensMapperpairNewIni **************************************************
**
** Construct an Ensembl Mapper Pair from components of Ensembl Mapper Unit
** objects.
**
** @param [r] srcoid [ajuint] Source Ensembl Object identifier
** @param [r] srcstart [ajint] Source Start coordinate
** @param [r] srcend [ajint] Source End coordinate
** @param [r] trgoid [ajuint] Target Ensembl Object identifier
** @param [r] trgstart [ajint] Target Start coordinate
** @param [r] trgend [ajint] Target End coordinate
** @param [r] ori [ajint]
** Relative orientation of the Ensembl Mapper Unit objects
** @param [r] indel [AjBool] Insertion-deletion attribute
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewIni(ajuint srcoid,
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

    if (!srcoid)
        return NULL;

    if (!trgoid)
        return NULL;

    source = ensMapperunitNewIni(srcoid, srcstart, srcend);
    target = ensMapperunitNewIni(trgoid, trgstart, trgend);

    mp = ensMapperpairNewUnit(source, target, ori, indel);

    ensMapperunitDel(&source);
    ensMapperunitDel(&target);

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewRef(EnsPMapperpair mp)
{
    if (!mp)
        return NULL;

    mp->Use++;

    return mp;
}




/* @func ensMapperpairNewUnit *************************************************
**
** Constructor for an Ensembl Mapper Pair with Ensembl Mapper Unit objects.
**
** @cc Bio::EnsEMBL::Mapper::Pair::new
** @param [u] source [EnsPMapperunit] Source Ensembl Mapper Unit
** @param [u] target [EnsPMapperunit] Target Ensembl Mapper Unit
** @param [r] ori [ajint]
** Relative orientation of the Ensembl Mapper Unit objects
** @cc Bio::EnsEMBL::Mapper::IndelPair::new
** @param [r] indel [AjBool] Insertion-deletion attribute
**
** @return [EnsPMapperpair] Ensembl Mapper Pair or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperpair ensMapperpairNewUnit(EnsPMapperunit source,
                                    EnsPMapperunit target,
                                    ajint ori,
                                    AjBool indel)
{
    EnsPMapperpair mp = NULL;

    if (!source)
        return NULL;

    if (!target)
        return NULL;

    AJNEW0(mp);

    mp->Source      = ensMapperunitNewRef(source);
    mp->Target      = ensMapperunitNewRef(target);
    mp->Orientation = ori;
    mp->Indel       = indel;
    mp->Use         = 1U;

    return mp;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Mapper Pair object.
**
** @fdata [EnsPMapperpair]
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Pair
**
** @argrule * Pmp [EnsPMapperpair*] Ensembl Mapper Pair address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperpairDel *****************************************************
**
** Default destructor for an Ensembl Mapper Pair.
**
** @param [d] Pmp [EnsPMapperpair*] Ensembl Mapper Pair address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMapperpairDel(EnsPMapperpair *Pmp)
{
    EnsPMapperpair pthis = NULL;

    if (!Pmp)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMapperpairDel"))
    {
        ajDebug("ensMapperpairDel\n"
                "  *Pmp %p\n",
                *Pmp);

        ensMapperpairTrace(*Pmp, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmp)
        return;

    pthis = *Pmp;

    pthis->Use--;

    if (pthis->Use)
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




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Mapper Pair object.
**
** @fdata [EnsPMapperpair]
**
** @nam3rule Get Return Ensembl Mapper Pair attribute(s)
** @nam4rule Indel Return the insertion or deletion member
** @nam4rule Orientation Return the orientation
** @nam4rule Source Return the source Ensembl Mapper Unit
** @nam4rule Target Return the target Ensembl Mapper Unit
**
** @argrule * mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @valrule Indel [AjBool] Insertion-deletion attribute
** @valrule Orientation [ajint] Orientation or 0
** @valrule Source [EnsPMapperunit] Ensembl Mapper Unit or NULL
** @valrule Target [EnsPMapperunit] Ensembl Mapper Unit or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMapperpairGetIndel ************************************************
**
** Return the insertion or deletion member of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::IndelPair::???
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [AjBool] ajTrue if this is an insertion or deletion
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperpairGetIndel(const EnsPMapperpair mp)
{
    return (mp) ? mp->Indel : ajFalse;
}




/* @func ensMapperpairGetOrientation ******************************************
**
** Get the relative orientation member of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::ori
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [ajint] Relative orientation or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperpairGetOrientation(const EnsPMapperpair mp)
{
    return (mp) ? mp->Orientation : 0;
}




/* @func ensMapperpairGetSource ***********************************************
**
** Get the source Ensembl Mapper Unit member of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::from
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperunit] Source Ensembl Mapper Unit or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperunit ensMapperpairGetSource(const EnsPMapperpair mp)
{
    return (mp) ? mp->Source : NULL;
}




/* @func ensMapperpairGetTarget ***********************************************
**
** Get the Target Ensembl Mapper Unit member of an Ensembl Mapper Pair.
**
** @cc Bio::EnsEMBL::Mapper::Pair::to
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [EnsPMapperunit] Target Ensembl Mapper Unit or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperunit ensMapperpairGetTarget(const EnsPMapperpair mp)
{
    return (mp) ? mp->Target : NULL;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Pair object.
**
** @fdata [EnsPMapperpair]
**
** @nam3rule Trace Report Ensembl Mapper Pair members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperpairTrace(const EnsPMapperpair mp, ajuint level)
{
    AjPStr indent = NULL;

    if (!mp)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperpairTrace %p\n"
            "%S  Source %p\n"
            "%S  Target %p\n"
            "%S  Orientation %d\n"
            "%S  Indel '%B'\n"
            "%S  Use %u\n",
            indent, mp,
            indent, mp->Source,
            indent, mp->Target,
            indent, mp->Orientation,
            indent, mp->Indel,
            indent, mp->Use);

    ensMapperunitTrace(mp->Source, level + 1);
    ensMapperunitTrace(mp->Target, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Mapper Pair object.
**
** @fdata [EnsPMapperpair]
**
** @nam3rule Calculate Calculate Ensembl Mapper Pair values
** @nam4rule Mapperunit Calculate the Ensembl Mapper Unit
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mp [const EnsPMapperpair] Ensembl Mapper Pair
** @argrule Mapperunit type [EnsEMapperunitType] Ensembl Mapper Unit Type
**
** @valrule Mapperunit [EnsPMapperunit] Ensembl Mapper Unit or NULL
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperpairCalculateMapperunit *************************************
**
** Calculate the Ensembl Mapper Unit member of an Ensembl Mapper Pair via an
** Ensembl Mapper Unit Type enumeration.
**
** @cc Bio::EnsEMBL::Mapper::Pair::from
** @cc Bio::EnsEMBL::Mapper::Pair::to
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
** @param [u] type [EnsEMapperunitType] Ensembl Mapper Unit Type
**
** @return [EnsPMapperunit] Source or Target Ensembl Mapper Unit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperunit ensMapperpairCalculateMapperunit(const EnsPMapperpair mp,
                                                EnsEMapperunitType type)
{
    if (!mp)
        return NULL;

    if (!type)
        return NULL;

    switch (type)
    {
        case ensEMapperunitTypeSource:
            return mp->Source;
        case ensEMapperunitTypeTarget:
            return mp->Target;
        default:
            ajWarn("ensMapperpairCalculateMapperunit got unexpected "
                   "Ensembl Mapper Unit Type enumeration %d.\n", type);
    }

    return NULL;
}




/* @func ensMapperpairCalculateMemsize ****************************************
**
** Calculate the memory size in bytes of an Ensembl Mapper Pair.
**
** @param [r] mp [const EnsPMapperpair] Ensembl Mapper Pair
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMapperpairCalculateMemsize(const EnsPMapperpair mp)
{
    size_t size = 0;

    if (!mp)
        return 0;

    size += sizeof (EnsOMapperpair);

    size += ensMapperunitCalculateMemsize(mp->Source);
    size += ensMapperunitCalculateMemsize(mp->Target);

    return size;
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
** @nam3rule Mapperpair Functions for manipulating AJAX List objects of
** Ensembl Mapper Pair objects
** @nam4rule Sort Sort functions
** @nam5rule Source Sort by source Ensembl Mapper Unit
** @nam5rule Target Sort by target Ensembl Mapper Unit
** @nam6rule Start Sort by Ensembl Feature start member
** @nam7rule Ascending  Sort in ascending order
** @nam7rule Descending Sort in descending order
**
** @argrule Ascending mps [AjPList] AJAX List of Ensembl Mapper Pair objects
** @argrule Descending mps [AjPList] AJAX List of Ensembl Mapper Pair objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listMapperpairCompareSourceStartAscending **********************
**
** AJAX List of Ensembl Mapper Pair objects comparison function to sort by
** source Ensembl Mapper Unit start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Mapper Pair address 1
** @param [r] item2 [const void*] Ensembl Mapper Pair address 2
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

static int listMapperpairCompareSourceStartAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPMapperpair mp1 = *(EnsOMapperpair *const *) item1;
    EnsPMapperpair mp2 = *(EnsOMapperpair *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMapperpairCompareSourceStartAscending"))
    {
        ajDebug("listMapperpairCompareSourceStartAscending\n"
                "  mp1 %p\n"
                "  mp2 %p\n",
                mp1,
                mp2);

        ensMapperpairTrace(mp1, 1);
        ensMapperpairTrace(mp2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mp1 && (!mp2))
        return -1;

    if ((!mp1) && (!mp2))
        return 0;

    if ((!mp1) && mp2)
        return +1;

    /*
    ** Sort Ensembl Mapper Pair objects with empty source Ensembl Mapper Unit
    ** objects towards the end of the AJAX List.
    */

    if (mp1->Source && (!mp2->Source))
        return -1;

    if ((!mp1->Source) && (!mp2->Source))
        return 0;

    if ((!mp1->Source) && mp2->Source)
        return +1;

    /* Evaluate the Start coordinates of source Ensembl Mapper Unit objects. */

    if (mp1->Source->Start < mp2->Source->Start)
        result = -1;

    if (mp1->Source->Start > mp2->Source->Start)
        result = +1;

    return result;
}




/* @func ensListMapperpairSortSourceStartAscending ****************************
**
** Sort an AJAX List of Ensembl Mapper Pair objects by their source
** Ensembl Mapper Unit start member in ascending order.
**
** @param [u] mps [AjPList] AJAX List of Ensembl Mapper Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMapperpairSortSourceStartAscending(AjPList mps)
{
    if (!mps)
        return ajFalse;

    ajListSort(mps, &listMapperpairCompareSourceStartAscending);

    return ajTrue;
}




/* @funcstatic listMapperpairCompareTargetStartAscending **********************
**
** AJAX List of Ensembl Mapper Pair objects comparison function to sort by
** target Ensembl Mapper Unit start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Mapper Pair address 1
** @param [r] item2 [const void*] Ensembl Mapper Pair address 2
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

static int listMapperpairCompareTargetStartAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPMapperpair mp1 = *(EnsOMapperpair *const *) item1;
    EnsPMapperpair mp2 = *(EnsOMapperpair *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMapperpairCompareTargetStartAscending"))
    {
        ajDebug("listMapperpairCompareTargetStartAscending\n"
                "  mp1 %p\n"
                "  mp2 %p\n",
                mp1,
                mp2);

        ensMapperpairTrace(mp1, 1);
        ensMapperpairTrace(mp2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mp1 && (!mp2))
        return -1;

    if ((!mp1) && (!mp2))
        return 0;

    if ((!mp1) && mp2)
        return +1;

    /*
    ** Sort Ensembl Mapper Pair objects with empty target Ensembl Mapper Unit
    ** objects towards the end of the AJAX List.
    */

    if (mp1->Target && (!mp2->Target))
        return -1;

    if ((!mp1->Target) && (!mp2->Target))
        return 0;

    if ((!mp1->Target) && mp2->Target)
        return +1;

    /* Evaluate the Start coordinates of target Ensembl Mapper Unit objects. */

    if (mp1->Target->Start < mp2->Target->Start)
        result = -1;

    if (mp1->Target->Start > mp2->Target->Start)
        result = +1;

    return result;
}




/* @func ensListMapperpairSortTargetStartAscending ****************************
**
** Sort an AJAX List of Ensembl Mapper Pair objects by their target
** Ensembl Mapper Unit start member in ascending order.
**
** @param [u] mps [AjPList] AJAX List of Ensembl Mapper Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMapperpairSortTargetStartAscending(AjPList mps)
{
    if (!mps)
        return ajFalse;

    ajListSort(mps, &listMapperpairCompareTargetStartAscending);

    return ajTrue;
}




/* @datasection [EnsPMapperresult] Ensembl Mapper Result **********************
**
** @nam2rule Mapperresult Functions for manipulating
** Ensembl Mapper Result objects
**
** @cc Bio::EnsEMBL::Mapper::Coordinate
** @cc CVS Revision: 1.16
** @cc CVS Tag: branch-ensembl-66
**
** @cc Bio::EnsEMBL::Mapper::Gap
** @cc CVS Revision: 1.14
** @cc CVS Tag: branch-ensembl-66
**
** @cc Bio::EnsEMBL::Mapper::IndelCoordinate
** @cc CVS Revision: 1.9
** @cc CVS Tag: branch-ensembl-66
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
**
** @nam3rule New Constructor
** @nam4rule Coordinate Constructor for type ensEMapperresultTypeCoordinate
** @nam4rule Cpy Constructor with existing object
** @nam4rule Gap Constructor for type ensEMapperresultTypeGap
** @nam4rule Indel Constructor for type ensEMapperresultTypeInDel
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Coordinate oid [ajuint] Ensembl Object identifier
** @argrule Coordinate crdstart [ajint] Start
** @argrule Coordinate crdend [ajint] End
** @argrule Coordinate crdstrand [ajint] Strand
** @argrule Coordinate cs [EnsPCoordsystem] Ensembl Coordinate System
** @argrule Coordinate rank [ajuint] Rank
** @argrule Cpy mr [const EnsPMapperresult] Ensembl Mapper Result
** @argrule Gap gapstart [ajint] Gap start
** @argrule Gap gapend [ajint] Gap end
** @argrule Gap rank [ajuint] Rank
** @argrule Indel oid [ajuint] Ensembl Object identifier
** @argrule Indel crdstart [ajint] Start
** @argrule Indel crdend [ajint] End
** @argrule Indel crdstrand [ajint] Strand
** @argrule Indel cs [EnsPCoordsystem] Ensembl Coordinate System
** @argrule Indel gapstart [ajint] Gap start
** @argrule Indel gapend [ajint] Gap end
** @argrule Indel rank [ajuint] Rank
** @argrule Ini type [EnsEMapperresultType] Type
** @argrule Ini oid [ajuint] Ensembl Object identifier
** @argrule Ini crdstart [ajint] Start
** @argrule Ini crdend [ajint] End
** @argrule Ini crdstrand [ajint] Strand
** @argrule Ini cs [EnsPCoordsystem] Ensembl Coordinate System
** @argrule Ini gapstart [ajint] Gap start
** @argrule Ini gapend [ajint] Gap end
** @argrule Ini rank [ajuint] Rank
** @argrule Ref mr [EnsPMapperresult] Ensembl Mapper Result
**
** @valrule * [EnsPMapperresult] Ensembl Mapper Result or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMapperresultNewCoordinate *****************************************
**
** Constructor for an Ensembl Mapper Result of type
** ensEMapperresultTypeCoordinate.
**
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] crdstart [ajint] Start
** @param [r] crdend [ajint] End
** @param [r] crdstrand [ajint] Strand
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] rank [ajuint] Rank
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewCoordinate(ajuint oid,
                                              ajint crdstart,
                                              ajint crdend,
                                              ajint crdstrand,
                                              EnsPCoordsystem cs,
                                              ajuint rank)
{
    EnsPMapperresult mr = NULL;

    if (!oid)
        return NULL;

    if (!cs)
        return NULL;

    AJNEW0(mr);

    mr->Type             = ensEMapperresultTypeCoordinate;
    mr->Objectidentifier = oid;
    mr->CoordinateStart  = crdstart;
    mr->CoordinateEnd    = crdend;
    mr->CoordinateStrand = crdstrand;
    mr->Coordsystem      = ensCoordsystemNewRef(cs);
    mr->GapStart         = 0;
    mr->GapEnd           = 0;
    mr->Rank             = rank;
    mr->Use              = 1U;

    return mr;
}




/* @func ensMapperresultNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewCpy(const EnsPMapperresult mr)
{
    EnsPMapperresult pthis = NULL;

    if (!mr)
        return NULL;

    AJNEW0(pthis);

    pthis->Type             = mr->Type;
    pthis->Objectidentifier = mr->Objectidentifier;
    pthis->CoordinateStart  = mr->CoordinateStart;
    pthis->CoordinateEnd    = mr->CoordinateEnd;
    pthis->CoordinateStrand = mr->CoordinateStrand;
    pthis->Coordsystem      = ensCoordsystemNewRef(mr->Coordsystem);
    pthis->GapStart         = mr->GapStart;
    pthis->GapEnd           = mr->GapEnd;
    pthis->Rank             = mr->Rank;
    pthis->Use              = 1U;

    return pthis;
}




/* @func ensMapperresultNewGap ************************************************
**
** Constructor for an Ensembl Mapper Result of type ensEMapperresultTypeGap.
**
** @param [r] gapstart [ajint] Gap start
** @param [r] gapend [ajint] Gap end
** @param [r] rank [ajuint] Rank
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewGap(ajint gapstart,
                                       ajint gapend,
                                       ajuint rank)
{
    EnsPMapperresult mr = NULL;

    AJNEW0(mr);

    mr->Type             = ensEMapperresultTypeGap;
    mr->Objectidentifier = 0;
    mr->CoordinateStart  = 0;
    mr->CoordinateEnd    = 0;
    mr->CoordinateStrand = 0;
    mr->Coordsystem      = (EnsPCoordsystem) NULL;
    mr->GapStart         = gapstart;
    mr->GapEnd           = gapend;
    mr->Rank             = rank;
    mr->Use              = 1U;

    return mr;
}




/* @func ensMapperresultNewIndel **********************************************
**
** Constructor for an Ensembl Mapper Result of type
** ensEMapperresultTypeInDel.
**
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] crdstart [ajint] Start
** @param [r] crdend [ajint] End
** @param [r] crdstrand [ajint] Strand
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] gapstart [ajint] Gap start
** @param [r] gapend [ajint] Gap end
** @param [r] rank [ajuint] Rank
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewIndel(ajuint oid,
                                         ajint crdstart,
                                         ajint crdend,
                                         ajint crdstrand,
                                         EnsPCoordsystem cs,
                                         ajint gapstart,
                                         ajint gapend,
                                         ajuint rank)
{
    EnsPMapperresult mr = NULL;

    AJNEW0(mr);

    if (!oid)
        return NULL;

    if (!cs)
        return NULL;

    mr->Type             = ensEMapperresultTypeInDel;
    mr->Objectidentifier = oid;
    mr->CoordinateStart  = crdstart;
    mr->CoordinateEnd    = crdend;
    mr->CoordinateStrand = crdstrand;
    mr->Coordsystem      = ensCoordsystemNewRef(cs);
    mr->GapStart         = gapstart;
    mr->GapEnd           = gapend;
    mr->Rank             = rank;
    mr->Use              = 1U;

    return mr;
}




/* @func ensMapperresultNewIni ************************************************
**
** Constructor for an Ensembl Mapper Result with initial values.
**
** @param [u] type [EnsEMapperresultType] Type
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] crdstart [ajint] Start
** @param [r] crdend [ajint] End
** @param [r] crdstrand [ajint] Strand
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] gapstart [ajint] Gap start
** @param [r] gapend [ajint] Gap end
** @param [r] rank [ajuint] Rank
**
** @return [EnsPMapperresult] Ensembl Mapper Result or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewIni(EnsEMapperresultType type,
                                       ajuint oid,
                                       ajint crdstart,
                                       ajint crdend,
                                       ajint crdstrand,
                                       EnsPCoordsystem cs,
                                       ajint gapstart,
                                       ajint gapend,
                                       ajuint rank)
{
    EnsPMapperresult mr = NULL;

    if (!type)
        return NULL;

    if ((type == ensEMapperresultTypeCoordinate) ||
        (type == ensEMapperresultTypeInDel))
    {
        if (!oid)
            return NULL;

        if (!cs)
            return NULL;
    }

    AJNEW0(mr);

    mr->Type             = type;
    mr->Objectidentifier = oid;
    mr->CoordinateStart  = crdstart;
    mr->CoordinateEnd    = crdend;
    mr->CoordinateStrand = crdstrand;
    mr->Coordsystem      = ensCoordsystemNewRef(cs);
    mr->GapStart         = gapstart;
    mr->GapEnd           = gapend;
    mr->Rank             = rank;
    mr->Use              = 1U;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperresult ensMapperresultNewRef(EnsPMapperresult mr)
{
    if (!mr)
        return NULL;

    mr->Use++;

    return mr;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Result
**
** @argrule * Pmr [EnsPMapperresult*] Ensembl Mapper Result address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperresultDel ***************************************************
**
** Default destructor for an Ensembl Mapper Result.
**
** @param [d] Pmr [EnsPMapperresult*] Ensembl Mapper Result address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMapperresultDel(EnsPMapperresult *Pmr)
{
    EnsPMapperresult pthis = NULL;

    if (!Pmr)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMapperresultDel"))
    {
        ajDebug("ensMapperresultDel\n"
                "  *Pmr %p\n",
                *Pmr);

        ensMapperresultTrace(*Pmr, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmr)
        return;

    pthis = *Pmr;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmr = NULL;

        return;
    }

    ensCoordsystemDel(&pthis->Coordsystem);

    AJFREE(pthis);

    *Pmr = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
**
** @nam3rule Get Return Ensembl Mapper Result attribute(s)
** @nam4rule Coordinate Return coordinate members
** @nam5rule End Return the coordinate end
** @nam5rule Start Return the coordinate start
** @nam5rule Strand Return the coordinate strand
** @nam4rule Coordsystem Return the Ensembl Coordinate System
** @nam4rule Gap Return gap members
** @nam5rule End Return the gap end
** @nam5rule Start Return the gap start
** @nam4rule Objectidentifier Return the Ensembl Object identifier
** @nam4rule Rank Return the rank
** @nam4rule Type Return the type
**
** @argrule * mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @valrule Coordsystem [EnsPCoordsystem] Ensembl Coordinate System or NULL
** @valrule CoordinateEnd [ajint] End or 0
** @valrule CoordinateStart [ajint] Coordinate start or 0
** @valrule CoordinateStrand [ajint] Coordinate strand or 0
** @valrule GapEnd [ajint] Gap end or 0
** @valrule GapStart [ajint] Gap start or 0
** @valrule Objectidentifier [ajuint] Ensembl Object identifier or 0U
** @valrule Rank [ajuint] Rank or 0U
** @valrule Type [EnsEMapperresultType] Type or ensEMapperresultTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensMapperresultGetCoordinateEnd **************************************
**
** Get the coordinate end member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::end
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Coordinate end or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensMapperresultGetCoordinateEnd(const EnsPMapperresult mr)
{
    return (mr) ? mr->CoordinateEnd : 0;
}




/* @func ensMapperresultGetCoordinateStart ************************************
**
** Get the coordinate start member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::start
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Coordinate start or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensMapperresultGetCoordinateStart(const EnsPMapperresult mr)
{
    return (mr) ? mr->CoordinateStart : 0;
}




/* @func ensMapperresultGetCoordinateStrand ***********************************
**
** Get the coordinate strand member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::strand
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Coordinate strand or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensMapperresultGetCoordinateStrand(const EnsPMapperresult mr)
{
    return (mr) ? mr->CoordinateStrand : 0;
}




/* @func ensMapperresultGetCoordsystem ****************************************
**
** Get the Ensembl Coordinate System member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::coord_system
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPCoordsystem ensMapperresultGetCoordsystem(const EnsPMapperresult mr)
{
    return (mr) ? mr->Coordsystem : NULL;
}




/* @func ensMapperresultGetGapEnd *********************************************
**
** Get the gap end member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Gap::end
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Gap end or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperresultGetGapEnd(const EnsPMapperresult mr)
{
    return (mr) ? mr->GapEnd : 0;
}




/* @func ensMapperresultGetGapStart *******************************************
**
** Get the gap start member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Gap::start
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajint] Gap start or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperresultGetGapStart(const EnsPMapperresult mr)
{
    return (mr) ? mr->GapStart : 0;
}




/* @func ensMapperresultGetObjectidentifier ***********************************
**
** Get the Ensembl Object identifier member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::id
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Ensembl Object identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperresultGetObjectidentifier(const EnsPMapperresult mr)
{
    return (mr) ? mr->Objectidentifier : 0U;
}




/* @func ensMapperresultGetRank ***********************************************
**
** Get the rank member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::rank
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Rank or 0U
**
** @release 6.3.0
** @@
******************************************************************************/

ajuint ensMapperresultGetRank(const EnsPMapperresult mr)
{
    return (mr) ? mr->Rank : 0U;
}




/* @func ensMapperresultGetType ***********************************************
**
** Get the type member of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [EnsEMapperresultType] Type or ensEMapperresultTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEMapperresultType ensMapperresultGetType(const EnsPMapperresult mr)
{
    return (mr) ? mr->Type : ensEMapperresultTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
**
** @nam3rule Set Set one member of an Ensembl Mapper Result
** @nam4rule Coordinate Set a coordinate
** @nam4rule Gap Set a gap
** @nam5rule End Set the end
** @nam5rule Start Set the start
** @nam5rule Strand Set the strand
** @nam4rule Coordsystem Set the Ensembl Coordinate System
** @nam4rule Objectidentifier Set the Ensembl Object identifier
** @nam4rule Rank Set the rank
** @nam4rule Type Set the type
**
** @argrule * mr [EnsPMapperresult] Ensembl Mapper Result object
** @argrule CoordinateEnd crdend [ajint] End
** @argrule CoordinateStart crdstart [ajint] Start
** @argrule CoordinateStrand crdstrand [ajint] Strand
** @argrule Coordsystem cs [EnsPCoordsystem] Ensembl Coordinate System
** @argrule GapEnd gapend [ajint] End
** @argrule GapStart gapstart [ajint] Start
** @argrule Objectidentifier oid [ajuint] Ensembl Object identifier
** @argrule Rank rank [ajuint] Rank
** @argrule Type mrt [EnsEMapperresultType]
** Ensembl Mapper Result Type enumeration
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMapperresultSetCoordinateEnd **************************************
**
** Set the coordinate end member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::end
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] crdend [ajint] End
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetCoordinateEnd(EnsPMapperresult mr,
                                       ajint crdend)
{
    if (!mr)
        return ajFalse;

    mr->CoordinateEnd = crdend;

    return ajTrue;
}




/* @func ensMapperresultSetCoordinateStart ************************************
**
** Set the coordinate start member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::start
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] crdstart [ajint] Start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetCoordinateStart(EnsPMapperresult mr,
                                         ajint crdstart)
{
    if (!mr)
        return ajFalse;

    mr->CoordinateStart = crdstart;

    return ajTrue;
}




/* @func ensMapperresultSetCoordinateStrand ***********************************
**
** Set the coordinate strand member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::strand
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] crdstrand [ajint] Strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetCoordinateStrand(EnsPMapperresult mr,
                                          ajint crdstrand)
{
    if (!mr)
        return ajFalse;

    mr->CoordinateStrand = crdstrand;

    return ajTrue;
}




/* @func ensMapperresultSetCoordsystem ****************************************
**
** Set the Ensembl Coordinate System member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::coord_system
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetCoordsystem(EnsPMapperresult mr,
                                     EnsPCoordsystem cs)
{
    if (!mr)
        return ajFalse;

    ensCoordsystemDel(&mr->Coordsystem);

    mr->Coordsystem = ensCoordsystemNewRef(cs);

    return ajTrue;
}




/* @func ensMapperresultSetGapEnd *********************************************
**
** Set the gap end member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Gap::end
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] gapend [ajint] End
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetGapEnd(EnsPMapperresult mr,
                                ajint gapend)
{
    if (!mr)
        return ajFalse;

    mr->GapEnd = gapend;

    return ajTrue;
}




/* @func ensMapperresultSetGapStart *******************************************
**
** Set the gap start member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Gap::start
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] gapstart [ajint] Start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetGapStart(EnsPMapperresult mr,
                                  ajint gapstart)
{
    if (!mr)
        return ajFalse;

    mr->GapStart = gapstart;

    return ajTrue;
}




/* @func ensMapperresultSetObjectidentifier ***********************************
**
** Set the Ensembl Object identifier member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::id
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] oid [ajuint] Ensembl Object identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetObjectidentifier(EnsPMapperresult mr,
                                          ajuint oid)
{
    if (!mr)
        return ajFalse;

    mr->Objectidentifier = oid;

    return ajTrue;
}




/* @func ensMapperresultSetRank ***********************************************
**
** Set the rank member of an Ensembl Mapper Result.
**
** @cc Bio::EnsEMBL::Mapper::Coordinate::rank
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [r] rank [ajuint] Rank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetRank(EnsPMapperresult mr,
                              ajuint rank)
{
    if (!mr)
        return ajFalse;

    mr->Rank = rank;

    return ajTrue;
}




/* @func ensMapperresultSetType ***********************************************
**
** Set the type member of an Ensembl Mapper Result.
**
** @param [u] mr [EnsPMapperresult] Ensembl Mapper Result
** @param [u] mrt [EnsEMapperresultType]
** Ensembl Mapper Result Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperresultSetType(EnsPMapperresult mr,
                              EnsEMapperresultType mrt)
{
    if (!mr)
        return ajFalse;

    mr->Type = mrt;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
**
** @nam3rule Trace Report Ensembl Mapper Result members to debug file
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperresultTrace(const EnsPMapperresult mr, ajuint level)
{
    AjPStr indent = NULL;

    if (!mr)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperresultTrace %p\n"
            "%S  Type %d\n"
            "%S  Objectidentifier %u\n"
            "%S  CoordinateStart %d\n"
            "%S  CoordinateEnd %d\n"
            "%S  CoordinateStrand %d\n"
            "%S  Coordsystem %p\n"
            "%S  GapStart %d\n"
            "%S  GapEnd %d\n"
            "%S  Rank %u\n"
            "%S  Use %u\n",
            indent, mr,
            indent, mr->Type,
            indent, mr->Objectidentifier,
            indent, mr->CoordinateStart,
            indent, mr->CoordinateEnd,
            indent, mr->CoordinateStrand,
            indent, mr->Coordsystem,
            indent, mr->GapStart,
            indent, mr->GapEnd,
            indent, mr->Rank,
            indent, mr->Use);

    ensCoordsystemTrace(mr->Coordsystem, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Mapper Result object.
**
** @fdata [EnsPMapperresult]
**
** @nam3rule Calculate Calculate Ensembl Mapper Result values
** @nam4rule Length Calculate the length
** @nam5rule Coordinate Ensembl Mapper Result Type
** ensEMapperresultTypeCoordinate
** @nam5rule Gap Ensembl Mapper Result Type ensEMapperresultTypeGap
** @nam5rule Result Ensembl Mapper Result Type
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @valrule Length [ajuint] Length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperresultCalculateLengthCoordinate *****************************
**
** Calculate the length of an Ensembl Mapper Result of
** Ensembl Mapper Result Type ensEMapperresultTypeCoordinate.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Coordinate length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperresultCalculateLengthCoordinate(const EnsPMapperresult mr)
{
    if (!mr)
        return 0U;

    return mr->CoordinateEnd - mr->CoordinateStart + 1U;
}




/* @func ensMapperresultCalculateLengthGap ************************************
**
** Calculate the length gap of an Ensembl Mapper Result of
** Ensembl Mapper Result Type ensEMapperresultTypeGap.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Gap length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperresultCalculateLengthGap(const EnsPMapperresult mr)
{
    if (!mr)
        return 0U;

    return mr->GapEnd - mr->GapStart + 1U;
}




/* @func ensMapperresultCalculateLengthResult *********************************
**
** Get the length of an Ensembl Mapper Result.
** This is the coordinate length for Mapper Results of type
** ensEMapperresultTypeCoordinate and ensEMapperresultTypeInDel and the
** gap length for Mapper Resuls of type ensEMapperresultTypeGap.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [ajuint] Ensembl Mapper Result length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperresultCalculateLengthResult(const EnsPMapperresult mr)
{
    if (!mr)
        return 0U;

    switch (mr->Type)
    {
        case ensEMapperresultTypeCoordinate:
            return ensMapperresultCalculateLengthCoordinate(mr);
        case ensEMapperresultTypeGap:
            return ensMapperresultCalculateLengthGap(mr);
        case ensEMapperresultTypeInDel:
            return ensMapperresultCalculateLengthCoordinate(mr);
        default:
            ajWarn("ensMapperresultCalculateLengthResult got unexpected "
                   "Ensembl Mapper Result type %d.\n", mr->Type);
    }

    return 0U;
}




/* @func ensMapperresultCalculateMemsize **************************************
**
** Calculate the memory size in bytes of an Ensembl Mapper Result.
**
** @param [r] mr [const EnsPMapperresult] Ensembl Mapper Result
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMapperresultCalculateMemsize(const EnsPMapperresult mr)
{
    size_t size = 0;

    if (!mr)
        return 0;

    size += sizeof (EnsOMapperresult);

    return size;
}




/* @datasection [EnsPMapperrange] Ensembl Mapper Range ************************
**
** @nam2rule Mapperrange Functions for manipulating
** Ensembl Mapper Range objects
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mr [const EnsPMapperrange] Ensembl Mapper Range
** @argrule Ini start [ajint] Start
** @argrule Ini end [ajint] End
** @argrule Ref mr [EnsPMapperrange] Ensembl Mapper Range
**
** @valrule * [EnsPMapperrange] Ensembl Mapper Range or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMapperrangeNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [EnsPMapperrange] Ensembl Mapper Range or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperrange ensMapperrangeNewCpy(const EnsPMapperrange mr)
{
    EnsPMapperrange pthis = NULL;

    if (!mr)
        return NULL;

    AJNEW0(pthis);

    pthis->Start = mr->Start;
    pthis->End   = mr->End;
    pthis->Use   = 1U;

    return pthis;
}




/* @func ensMapperrangeNewIni *************************************************
**
** Constructor for an Ensembl Mapper Range with initial values.
**
** @param [r] start [ajint] Start
** @param [r] end [ajint] End
**
** @return [EnsPMapperrange] Ensembl Mapper Range or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapperrange ensMapperrangeNewIni(ajint start, ajint end)
{
    EnsPMapperrange mr = NULL;

    AJNEW0(mr);

    mr->Start = start;
    mr->End   = end;
    mr->Use   = 1U;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperrange ensMapperrangeNewRef(EnsPMapperrange mr)
{
    if (!mr)
        return NULL;

    mr->Use++;

    return mr;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Mapper Range object.
**
** @fdata [EnsPMapperrange]
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Range
**
** @argrule * Pmr [EnsPMapperrange*] Ensembl Mapper Range address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperrangeDel ****************************************************
**
** Default destructor for an Ensembl Mapper Range.
**
** @param [d] Pmr [EnsPMapperrange*] Ensembl Mapper Range address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMapperrangeDel(EnsPMapperrange *Pmr)
{
    EnsPMapperrange pthis = NULL;

    if (!Pmr)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMapperrangeDel"))
    {
        ajDebug("ensMapperrangeDel\n"
                "  *Pmr %p\n",
                *Pmr);

        ensMapperrangeTrace(*Pmr, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmr)
        return;

    pthis = *Pmr;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmr = NULL;

        return;
    }

    AJFREE(pthis);

    *Pmr = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Mapper Range object.
**
** @fdata [EnsPMapperrange]
**
** @nam3rule Get Return Ensembl Mapper Range attribute(s)
** @nam4rule GetEnd Return the end
** @nam4rule GetStart Return the start
**
** @argrule * mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @valrule End [ajint] End or 0
** @valrule Start [ajint] Start or 0
**
** @fcategory use
******************************************************************************/




/* @func ensMapperrangeGetEnd *************************************************
**
** Get the end member of an Ensembl Mapper Range.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [ajint] End or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperrangeGetEnd(const EnsPMapperrange mr)
{
    return (mr) ? mr->End : 0;
}




/* @func ensMapperrangeGetStart ***********************************************
**
** Get the start member of an Ensembl Mapper Range.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [ajint] Start or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensMapperrangeGetStart(const EnsPMapperrange mr)
{
    return (mr) ? mr->Start : 0;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper Range object.
**
** @fdata [EnsPMapperrange]
**
** @nam3rule Trace Report Ensembl Mapper Range members to debug file
**
** @argrule Trace mr [const EnsPMapperrange] Ensembl Mapper Range
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperrangeTrace **************************************************
**
** Trace an Ensembl Mapper Range.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperrangeTrace(const EnsPMapperrange mr, ajuint level)
{
    AjPStr indent = NULL;

    if (!mr)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperrangeTrace %p\n"
            "%S  Start %d\n"
            "%S  End %d\n"
            "%S  Use %u\n",
            indent, mr,
            indent, mr->Start,
            indent, mr->End,
            indent, mr->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Mapper Range object.
**
** @fdata [EnsPMapperrange]
**
** @nam3rule Calculate Calculate Ensembl Mapper Range values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperrangeCalculateMemsize ***************************************
**
** Calculate the memory size in bytes of an Ensembl Mapper Range.
**
** @param [r] mr [const EnsPMapperrange] Ensembl Mapper Range
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMapperrangeCalculateMemsize(const EnsPMapperrange mr)
{
    size_t size = 0;

    if (!mr)
        return 0;

    size += sizeof (EnsOMapperrange);

    return size;
}




/* @datasection [EnsPMapperrangeregistry] Ensembl Mapper Range Registry *******
**
** @nam2rule Mapperrangeregistry Functions for manipulating
** Ensembl Mapper Range Registry objects
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry
** @cc CVS Revision: 1.18
** @cc CVS Tag: branch-ensembl-66
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mrr [const EnsPMapperrangeregistry]
** Ensembl Mapper Range Registry
** @argrule Ref mrr [EnsPMapperrangeregistry]
** Ensembl Mapper Range Registry
**
** @valrule * [EnsPMapperrangeregistry] Ensembl Mapper Range Registry or NULL
**
** @fcategory new
******************************************************************************/




/* @funcstatic mapperrangeregistryListMapperrangeValdel ***********************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl Mapper Range objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void mapperrangeregistryListMapperrangeValdel(void **Pvalue)
{
    EnsPMapperrange mr = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &mr))
        ensMapperrangeDel(&mr);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @func ensMapperrangeregistryNew ********************************************
**
** Default constructor for an Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::new
** @return [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @release 6.2.0
** @@
** The Ensembl Mapper Range Registry maintains an internal list of registered
** regions and is used to quickly ascertain if and what regions of a provided
** range need registration. It is implemented as a first-level AJAX Table with
** Ensembl Object identifers as keys and second-level AJAX Lists of
** Ensembl Mapper Range objects as values.
******************************************************************************/

EnsPMapperrangeregistry ensMapperrangeregistryNew(void)
{
    EnsPMapperrangeregistry mrr = NULL;

    if (ajDebugTest("ensMapperrangeregistryNew"))
        ajDebug("ensMapperrangeregistryNew\n");

    AJNEW0(mrr);

    mrr->Registry = ajTableuintNew(0);

    ajTableSetDestroyvalue(
        mrr->Registry,
        (void (*)(void **)) &mapperrangeregistryListMapperrangeValdel);

    mrr->Use = 1U;

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapperrangeregistry ensMapperrangeregistryNewRef(
    EnsPMapperrangeregistry mrr)
{
    if (!mrr)
        return NULL;

    mrr->Use++;

    return mrr;
}




/* @section clear *************************************************************
**
** Clear all internal data structures and frees the
** memory allocated for Ensembl Mapper Range Registry internals.
**
** @fdata [EnsPMapperrangeregistry]
**
** @nam3rule Clear Clear (free) an Ensembl Mapper Range Registry object
**
** @argrule * mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperrangeregistryClear ******************************************
**
** Clear an Ensembl Mapper Range Registry.
** This function clears the unsigned integer key data and the AJAX List value
** data from the first-level AJAX Table, as well as the Ensembl Mapper Range
** objects from the second-level AJAX List.
**
** @param [u] mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperrangeregistryClear(EnsPMapperrangeregistry mrr)
{
    if (ajDebugTest("ensMapperrangeregistryClear"))
        ajDebug("ensMapperrangeregistryClear\n"
                "  mrr %p\n",
                mrr);

    if (!mrr)
        return ajFalse;

    ajTableClearDelete(mrr->Registry);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Mapper Range Registry object.
**
** @fdata [EnsPMapperrangeregistry]
**
** @nam3rule Del Destroy (free) an Ensembl Mapper Range Registry
**
** @argrule * Pmrr [EnsPMapperrangeregistry*]
** Ensembl Mapper Range Registry address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperrangeregistryDel ********************************************
**
** Default destructor for an Ensembl Mapper Range Registry.
**
** @param [d] Pmrr [EnsPMapperrangeregistry*]
** Ensembl Mapper Range Registry address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMapperrangeregistryDel(EnsPMapperrangeregistry *Pmrr)
{
    EnsPMapperrangeregistry pthis = NULL;

    if (!Pmrr)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMapperrangeregistryDel"))
        ajDebug("ensMapperrangeregistryDel\n"
                "  *Pmrr %p\n",
                *Pmrr);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmrr)
        return;

    pthis = *Pmrr;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmrr = NULL;

        return;
    }

    ajTableDel(&pthis->Registry);

    AJFREE(pthis);

    *Pmrr = NULL;

    return;
}




/* @section check *************************************************************
**
** Check, whether coordinates have been registered before.
**
** @fdata [EnsPMapperrangeregistry]
**
** @nam3rule Check Check a coordinates
**
** @argrule * mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
** @argrule Check oid [ajuint] Ensembl Object identifier
** @argrule Check chkstart [ajint] Start coordinate of the region to be checked
** @argrule Check chkend [ajint] End coordinate of the region to be checked
** @argrule Check regstart [ajint] Start coordinate of the region to register
** @argrule Check regend [ajint] End coordinate of the region to register
** @argrule Check ranges [AjPList] AJAX List of Ensembl Mapper Range objects
**
** @valrule * [AjBool] ajTrue if already registered, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMapperrangeregistryCheck ******************************************
**
** Check and register Ensembl Mapper Range objects in the
** Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::check_and_register
** @param [u] mrr [EnsPMapperrangeregistry] Ensembl Mapper Range Registry
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] chkstart [ajint] Start coordinate of the region to be checked
** @param [r] chkend [ajint] End coordinate of the region to be checked
** @param [r] regstart [ajint] Start coordinate of the region to register
** @param [r] regend [ajint] End coordinate of the region to register
** @param [uN] ranges [AjPList] AJAX List of Ensembl Mapper Range objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
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

AjBool ensMapperrangeregistryCheck(EnsPMapperrangeregistry mrr,
                                   ajuint oid,
                                   ajint chkstart,
                                   ajint chkend,
                                   ajint regstart,
                                   ajint regend,
                                   AjPList ranges)
{
    register ajuint i = 0U;

    ajuint idxstart  = 0U;
    ajuint idxmid    = 0U;
    ajuint idxend    = 0U;
    ajuint idxlength = 0U;

    ajuint idxregstart = 0U;
    ajuint idxregend   = 0U;
    ajuint idxregpos   = 0U;

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

    if (ajDebugTest("ensMapperrangeregistryCheck"))
        ajDebug("ensMapperrangeregistryCheck\n"
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

    if (!mrr)
    {
        ajDebug("ensMapperrangeregistryCheck requires an "
                "Ensembl Range Registry.\n");

        return ajFalse;
    }

    if (!oid)
        ajWarn("ensMapperrangeregistryCheck did not get a valid "
               "Ensembl Object identifier.\n");

    /*
    ** NOTE: Not applicable for circular genomes.
    if (chkstart > chkend)
    {
    ajWarn("ensMapperrangeregistryCheck requires the start "
    "coordinate (%d) to be less than or equal the end "
    "coordinate (%d) for the region to be checked.\n",
    chkstart, chkend);

    return ajFalse;
    }
    */

    if ((!regstart) && (!regend))
    {
        regstart = chkstart;

        regend = chkend;
    }

    /*
    ** NOTE: Not applicable for circular genomes.
    if (regstart > regend)
    {
    ajWarn("ensMapperrangeregistryCheck requires the start "
    "coordinate (%d) to be less than or equal the end "
    "coordinate (%d) for the region to be registered.\n",
    regstart, regend);

    return ajFalse;
    }
    */

    if (regstart > chkstart)
    {
        ajWarn("ensMapperrangeregistryCheck requires the start "
               "coordinate (%d) of the region to be registered to be less "
               "than or equal the start coordinate (%d) of the "
               "region to be checked.\n",
               regstart, chkstart);

        return ajFalse;
    }

    if (regend < chkend)
    {
        ajWarn("ensMapperrangeregistryCheck requires the end "
               "coordinate (%d) of the region to be registered to be less "
               "than or equal the end coordinate (%d) of the "
               "region to be checked.\n",
               regend, chkend);

        return ajFalse;
    }

    if (!mrr->Registry)
        ajFatal("ensMapperrangeregistryCheck got a "
                "Mapper Range Registry without a valid AJAX Table.\n");

    list = (AjPList) ajTableFetchmodV(mrr->Registry, (const void *) &oid);

    if (!list)
    {
        AJNEW0(Poid);

        *Poid = oid;

        list = ajListNew();

        ajTablePut(mrr->Registry, (void *) Poid, (void *) list);
    }

    idxlength = (ajuint) ajListGetLength(list);

    if (!idxlength)
    {
        /*
        ** This is the first request for this Ensembl Object identifier,
        ** return a gap Mapper Range for the entire range and register it as
        ** seen.
        */

        range = ensMapperrangeNewIni(regstart, regend);

        ajListPushAppend(list, (void *) range);

        if (ranges)
            ajListPushAppend(ranges, (void *) ensMapperrangeNewRef(range));

        return ajTrue;
    }

    /*
    ** Loop through the AJAX List of existing Ensembl Mapper Range objects
    ** recording any "gaps" where the existing Mapper Range objects do not
    ** cover part of the requested range.
    */

    idxstart = 0U;
    idxend   = idxlength - 1U;

    /*
    ** Binary search the relevant Ensembl Mapper Range objects,
    ** which helps if the AJAX List of Ensembl Mapper Range objects is long.
    */

    while ((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &range);

        if (range->End < regstart)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    gapstart = regstart;

    for (i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &range);

        /*
        ** No work needs to be done at all if we find a Mapper Range that
        ** entirely overlaps the requested region.
        */

        if ((range->Start <= chkstart) && (range->End >= chkend))
            return ajTrue;

        /* Find adjacent or overlapping regions already registered. */

        if ((range->Start <= (regend + 1)) && (range->End >= (regstart - 1)))
        {
            if (!idxregstartset)
            {
                idxregstartset = ajTrue;

                idxregstart = i;
            }

            idxregend = i;
        }

        /* Find previously un-registered regions (gaps). */

        if (range->Start > regstart)
        {
            gapend = (regend < range->Start) ? regend : (range->Start - 1);

            if (ranges)
            {
                gap = ensMapperrangeNewIni(gapstart, gapend);

                ajListPushAppend(ranges, (void *) gap);
            }
        }

        gapstart = (regstart > range->End) ? regstart : (range->End + 1);

        if ((range->End >= regend) && (!idxregposset))
        {
            idxregposset = ajTrue;

            idxregpos = i;

            break;
        }
    }

    /* Do we have to define another gap? */

    if ((gapstart <= regend) && ranges)
    {
        gap = ensMapperrangeNewIni(gapstart, regend);

        ajListPushAppend(ranges, (void *) gap);
    }

    /* Merge the new Ensembl Mapper Range into the AJAX List of registered. */

    if (idxregstartset)
    {
        /* Adjacent or overlapping regions have been found. */

        ajListPeekNumber(list, idxregstart, (void **) &range);

        newstart = (regstart < range->Start) ? regstart : range->Start;

        ajListPeekNumber(list, idxregend, (void **) &range);

        newend = (regend > range->End) ? regend : range->End;

        iter = ajListIterNew(list);

        /* Position the AJAX List Iterator at idxregstart. */

        for (i = 0U; i < idxregstart; i++)
            range = (EnsPMapperrange) ajListIterGet(iter);

        /*
        ** Now, remove idxregend - idxregstart + 1 elements
        ** from the AJAX List.
        */

        for (i = 0U; i < (idxregend - idxregstart + 1); i++)
        {
            range = (EnsPMapperrange) ajListIterGet(iter);

            ajListIterRemove(iter);

            ensMapperrangeDel(&range);
        }

        /* Finally, insert a new Mapper Range at the current position. */

        range = ensMapperrangeNewIni(newstart, newend);

        ajListIterInsert(iter, (void *) range);

        ajListIterDel(&iter);
    }
    else if (idxregposset)
    {
        iter = ajListIterNew(list);

        /* Position the AJAX List Iterator at idxregpos. */

        for (i = 0U; i < idxregpos; i++)
            range = (EnsPMapperrange) ajListIterGet(iter);

        /* Insert a new Mapper Range at this position. */

        range = ensMapperrangeNewIni(regstart, regend);

        ajListIterInsert(iter, (void *) range);

        ajListIterDel(&iter);
    }
    else
    {
        range = ensMapperrangeNewIni(regstart, regend);

        ajListPushAppend(list, (void *) range);
    }

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Mapper Range Registry object.
**
** @fdata [EnsPMapperrangeregistry]
**
** @nam3rule Calculate Calculate Ensembl Mapper Range Registry values
** @nam4rule Mapperranges
** Calculate an AJAX List of Ensembl Mapper Range objects
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Overlap Calculate the overlap size
**
** @argrule * mrr [const EnsPMapperrangeregistry] Ensembl Mapper Range Registry
** @argrule Mapperranges oid [ajuint] Ensembl Object identifier
** @argrule Overlap oid [ajuint] Ensembl Object identifier
** @argrule Overlap start [ajint] Start coordinate
** @argrule Overlap end [ajint] End coordinate
**
** @valrule Mapperranges [const AjPList]
** AJAX List or Ensembl Mapper Range objects or NULL
** @valrule Memsize [size_t] Memory size in bytes or 0
** @valrule Overlap [ajuint] Overlap size or 0U
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperrangeregistryCalculateMapperranges **************************
**
** Calculate an AJAX List of Ensembl Mapper Range objects.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::get_ranges
** @param [r] mrr [const EnsPMapperrangeregistry] Ensembl Mapper
**                                                Range Registry
** @param [r] oid [ajuint] Ensembl Object identifier
**
** @return [const AjPList] AJAX List of Ensembl Mapper Range objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensMapperrangeregistryCalculateMapperranges(
    const EnsPMapperrangeregistry mrr,
    ajuint oid)
{
    if (!mrr)
        return NULL;

    if (!oid)
        return NULL;

    return (AjPList) ajTableFetchmodV(mrr->Registry, (const void *) &oid);
}




/* @func ensMapperrangeregistryCalculateOverlap *******************************
**
** Calculate the overlap size of Ensembl Mapper Range objects in the
** Ensembl Mapper Range Registry. Finds out how many bases in the given range
** are already registered in an Ensembl Mapper Range Registry.
**
** @cc Bio::EnsEMBL::Mapper::RangeRegistry::overlap_size
** @param [r] mrr [const EnsPMapperrangeregistry] Ensembl Mapper
**                                                Range Registry
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
**
** @return [ajuint] Overlap size or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperrangeregistryCalculateOverlap(
    const EnsPMapperrangeregistry mrr,
    ajuint oid,
    ajint start,
    ajint end)
{
    register ajuint i = 0U;

    ajuint idxstart  = 0U;
    ajuint idxmid    = 0U;
    ajuint idxend    = 0U;
    ajuint idxlength = 0U;

    ajint mrstart = 0;
    ajint mrend   = 0;

    ajuint overlap = 0U;

    AjPList list = NULL;

    EnsPMapperrange range = NULL;

    if (!mrr)
        return 0U;

    if (start > end)
        return 0U;

    list = (AjPList) ajTableFetchmodV(mrr->Registry, (const void *) &oid);

    if (!list)
        return 0U;

    idxlength = (ajuint) ajListGetLength(list);

    if (!idxlength)
        return 0U;

    idxstart = 0U;
    idxend   = idxlength - 1U;

    /*
    ** Binary search the relevant Ensembl Mapper Range objects,
    ** which helps if the AJAX List is long.
    */

    while ((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &range);

        if (range->End < start)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    for (i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &range);

        /*
        ** Check, wheher the loop has already overrun.
        ** If that was the case, there are no more interesting Mapper Range
        ** objects.
        */

        if (range->Start > start)
            break;

        /*
        ** No work needs to be done at all if we find a Mapper Range that
        ** entirely overlaps the requested region.
        */

        if ((range->Start <= start) && (range->End >= end))
        {
            overlap = (ajuint) (end - start + 1);

            break;
        }

        mrstart = (start < range->Start) ? range->Start : start;

        mrend = (end < range->End) ? end : range->End;

        if ((mrend - mrstart) >= 0)
            overlap += (ajuint) (mrend - mrstart + 1);
    }

    return overlap;
}




/* @datasection [EnsPMapper] Ensembl Mapper ***********************************
**
** @nam2rule Mapper Functions for manipulating Ensembl Mapper objects
**
** @cc Bio::EnsEMBL::Mapper
** @cc CVS Revision: 1.60
** @cc CVS Tag: branch-ensembl-66
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mapper [const EnsPMapper] Ensembl Mapper
** @argrule Ini srctype [AjPStr] Source type
** @argrule Ini trgtype [AjPStr] Target type
** @argrule Ini srccs [EnsPCoordsystem] Source Ensembl Coordinate System
** @argrule Ini trgcs [EnsPCoordsystem] Target Ensembl Coordinate System
** @argrule Ref mapper [EnsPMapper] Ensembl Mapper
**
** @valrule * [EnsPMapper] Ensembl Mapper or NULL
**
** @fcategory new
******************************************************************************/




/* @funcstatic mapperListMapperpairValdel *************************************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl Mapper Pair objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void mapperListMapperpairValdel(void **Pvalue)
{
    EnsPMapperpair mp = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &mp))
        ensMapperpairDel(&mp);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @func ensMapperNewIni ******************************************************
**
** Constructor for an Ensembl Mapper with initial values.
**
** @cc Bio::EnsEMBL::Mapper::new
** @param [u] srctype [AjPStr] Source type
** @param [u] trgtype [AjPStr] Target type
** @param [u] srccs [EnsPCoordsystem] Source Ensembl Coordinate System
** @param [u] trgcs [EnsPCoordsystem] Target Ensembl Coordinate System
**
** @return [EnsPMapper] Ensembl Mapper or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMapper ensMapperNewIni(AjPStr srctype,
                           AjPStr trgtype,
                           EnsPCoordsystem srccs,
                           EnsPCoordsystem trgcs)
{
    AjPTable table = NULL;

    EnsPMapper mapper = NULL;

    if (ajDebugTest("ensMapperNewIni"))
    {
        ajDebug("ensMapperNewIni\n"
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

    if (!srctype)
        return NULL;

    if (!trgtype)
        return NULL;

    if (!srccs)
        return NULL;

    if (!trgcs)
        return NULL;

    AJNEW0(mapper);

    mapper->TypeSource        = ajStrNewRef(srctype);
    mapper->TypeTarget        = ajStrNewRef(trgtype);
    mapper->CoordsystemSource = ensCoordsystemNewRef(srccs);
    mapper->CoordsystemTarget = ensCoordsystemNewRef(trgcs);
    mapper->Mapperpairs       = ajTablestrNew(0);
    mapper->Count             = 0;
    mapper->Sorted            = ajFalse;
    mapper->Use               = 1U;

    ajTableSetDestroyvalue(
        mapper->Mapperpairs,
        (void (*)(void **)) &ajTableDel);

    /*
    ** Initialise second-level AJAX Table objects with Ensembl Object
    ** identifier keys and put them into the first-level AJAX Table indexed on
    ** source and target types.
    */

    table = ajTableuintNew(0);

    ajTableSetDestroyvalue(
        table,
        (void (*)(void **)) &mapperListMapperpairValdel);

    ajTablePut(mapper->Mapperpairs, (void *) ajStrNewS(srctype), (void *) table);

    table = ajTableuintNew(0);

    ajTableSetDestroyvalue(
        table,
        (void (*)(void **)) &mapperListMapperpairValdel);

    ajTablePut(mapper->Mapperpairs, (void *) ajStrNewS(trgtype), (void *) table);

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
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMapper ensMapperNewRef(EnsPMapper mapper)
{
    if (!mapper)
        return NULL;

    mapper->Use++;

    return mapper;
}




/* @section clear *************************************************************
**
** Clear all internal data structures and frees the
** memory allocated for Ensembl Mapper internals.
**
** @fdata [EnsPMapper]
**
** @nam3rule Clear Clear (free) an Ensembl Mapper object
**
** @argrule * mapper [EnsPMapper] Ensembl Mapper
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory delete
******************************************************************************/




/* @funcstatic mapperMapperpairsClear *****************************************
**
** An ajTableMap "apply" function to process an AJAX Table of
** AJAX String object key data and
** AJAX Table object value data.
** This function also clears and deletes the secondary AJAX Table objects via
** mapperClearL2.
**
** @param [r] key [const void*] AJAX String address
** @param [u] Pvalue [void**] AJAX Table address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMap
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void mapperMapperpairsClear(const void *key,
                                   void **Pvalue,
                                   void *cl)
{
    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    (void) cl;

    (void) key;

    ajTableClearDelete(*((AjPTable *) Pvalue));

    return;
}




/* @func ensMapperClear *******************************************************
**
** Clear an Ensembl Mapper.
**
** This function processes the first-level AJAX Table of
** AJAX String object (Ensembl Mapper type) key data and
** second-level AJAX Table object value data.
** The second-level AJAX Table of
** AJAX unsigned integer (Ensembl Object identifier) key data and
** third-level AJAX List object valude data are cleared.
** The third-level AJAX List objects of Ensembl Mapper Pair objects are cleared
** and deleted, as are the Ensembl Mapper Pair objects.
**
** @cc Bio::EnsEMBL::Mapper::flush
** @param [u] mapper [EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperClear(EnsPMapper mapper)
{
    if (ajDebugTest("ensMapperClear"))
    {
        ajDebug("ensMapperClear\n"
                "  mapper %p\n",
                mapper);

        ensMapperTrace(mapper, 1);
    }

    if (!mapper)
        return ajFalse;

    ajTableMap(mapper->Mapperpairs, &mapperMapperpairsClear, NULL);

    mapper->Count = 0;
    mapper->Sorted = ajFalse;

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Mapper object.
**
** @fdata [EnsPMapper]
**
** @nam3rule Del Destroy (free) an Ensembl Mapper
**
** @argrule * Pmapper [EnsPMapper*] Ensembl Mapper address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMapperDel *********************************************************
**
** Default destructor for an Ensembl Mapper.
**
** This function clears and deletes the first-level AJAX Table of
** AJAX String object (Ensembl Mapper type) key data and
** second-level AJAX Table object value data.
** The second-level AJAX Table of
** AJAX unsigned integer (Ensembl Object identifier) key data and
** third-level AJAX List object value data are also cleared.
** The third-level AJAX List objects of Ensembl Mapper Pair objects are cleared
** and deleted, as are the Ensembl Mapper Pair objects.
**
** @param [d] Pmapper [EnsPMapper*] Ensembl Mapper address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMapperDel(EnsPMapper *Pmapper)
{
    EnsPMapper pthis = NULL;

    if (!Pmapper)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMapperDel"))
    {
        ajDebug("ensMapperDel\n"
                "  *Pmapper %p\n",
                *Pmapper);

        ensMapperTrace(*Pmapper, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmapper)
        return;

    pthis = *Pmapper;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmapper = NULL;

        return;
    }

    ajTableDel(&pthis->Mapperpairs);

    ajStrDel(&pthis->TypeSource);
    ajStrDel(&pthis->TypeTarget);

    ensCoordsystemDel(&pthis->CoordsystemSource);
    ensCoordsystemDel(&pthis->CoordsystemTarget);

    AJFREE(pthis);

    *Pmapper = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Mapper object.
**
** @fdata [EnsPMapper]
**
** @nam3rule Get Return Ensembl Mapper attribute(s)
** @nam4rule Coordsystem Return an Ensembl Coordinate System
** @nam5rule Source Return the source Ensembl Coordinate System
** @nam5rule Target Return the target Ensembl Coordinate System
** @nam4rule Count Return the number of Ensembl Mapper Pair objects
** @nam4rule Sorted Return the sorted attribute
** @nam4rule Type Return an Ensembl Mapper Type
** @nam5rule Source Return the source type
** @nam5rule Target Return the target type
**
** @argrule * mapper [const EnsPMapper] Ensembl Mapper
**
** @valrule CoordsystemSource [EnsPCoordsystem] Source Ensembl Coordinate
** System or NULL
** @valrule CoordsystemTarget [EnsPCoordsystem] Target Ensembl Coordinate
** System or NULL
** @valrule Count [ajuint] Number of Ensembl Mapper Pair objects or 0U
** @valrule Sorted [AjBool] ajTrue if the Mapper Pair objects are sorted
** @valrule TypeSource [AjPStr] Source type or NULL
** @valrule TypeTarget [AjPStr] Target type or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMapperGetCoordsystemSource ****************************************
**
** Get the source Ensembl Coordinate System member of an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [EnsPCoordsystem] Source Ensembl Coordinate System or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPCoordsystem ensMapperGetCoordsystemSource(const EnsPMapper mapper)
{
    return (mapper) ? mapper->CoordsystemSource : NULL;
}




/* @func ensMapperGetCoordsystemTarget ****************************************
**
** Get the target Ensembl Coordinate System member of an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [EnsPCoordsystem] Target Ensembl Coordinate System or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPCoordsystem ensMapperGetCoordsystemTarget(const EnsPMapper mapper)
{
    return (mapper) ? mapper->CoordsystemTarget : NULL;
}




/* @func ensMapperGetCount ****************************************************
**
** Get the number of Ensembl Mapper Pair objects in an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [ajuint] Number of Ensembl Mapper Pair objects or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensMapperGetCount(const EnsPMapper mapper)
{
    return (mapper) ? mapper->Count : 0U;
}




/* @func ensMapperGetSorted ***************************************************
**
** Return the sorted member of an Ensembl Mapper.
**
** This member indicates whether Ensembl Mapper Pair objects are sorted
** in the Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::_is_sorted
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue if the Mapper Pair objects are sorted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperGetSorted(const EnsPMapper mapper)
{
    return (mapper) ? mapper->Sorted : ajFalse;
}




/* @func ensMapperGetTypeSource ***********************************************
**
** Get the source type member of an Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::from
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [AjPStr] Source type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensMapperGetTypeSource(const EnsPMapper mapper)
{
    return (mapper) ? mapper->TypeSource : NULL;
}




/* @func ensMapperGetTypeTarget ***********************************************
**
** Get the target type member of an Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::to
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [AjPStr] Target type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensMapperGetTypeTarget(const EnsPMapper mapper)
{
    return (mapper) ? mapper->TypeTarget : NULL;
}




/* @funcstatic mapperMapperpairsMerge *****************************************
**
** Merge adjacent Ensembl Mapper Pair objects in an Ensembl Mapper into one.
**
** @cc Bio::EnsEMBL::Mapper::_merge_pairs
** @param [u] mapper [EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool mapperMapperpairsMerge(EnsPMapper mapper)
{
    void **valarray = NULL;

    register ajuint i = 0U;

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

    debug = ajDebugTest("mapperMapperpairsMerge");

    if (debug)
    {
        ajDebug("mapperMapperpairsMerge\n"
                "  mapper %p\n",
                mapper);

        ensMapperTrace(mapper, 1);
    }

    if (!mapper)
        return ajFalse;

    trgtable = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                           mapper->TypeTarget);

    ajTableToarrayValues(trgtable, &valarray);

    for (i = 0U; valarray[i]; i++)
    {
        trglist = (AjPList) valarray[i];

        trgiter = ajListIterNew(trglist);

        while (!ajListIterDone(trgiter))
        {
            trgpair1 = (EnsPMapperpair) ajListIterGet(trgiter);
            trgpair2 = (EnsPMapperpair) ajListIterGet(trgiter);

            /*
            ** If target Mapper Pair 2 is not defined, there are no more
            ** Mapper Pair objects to compare.
            */

            if ((!trgpair1) || (!trgpair2))
                break;

            /* Do not merge insertion or deletion Mapper Pair objects. */

            if (trgpair1->Indel || trgpair2->Indel)
                continue;

            assert(trgpair1->Source);
            assert(trgpair1->Target);

            assert(trgpair2->Source);
            assert(trgpair2->Target);

            /* Merge overlapping Mapper Pair objects. */

            if ((trgpair1->Source->Objectidentifier ==
                 trgpair2->Source->Objectidentifier) &&
                (trgpair1->Target->Start == trgpair2->Target->Start))
                delpair = trgpair2;
            /* Merge adjacent Mapper Pair objects. */
            else if ((trgpair1->Source->Objectidentifier ==
                      trgpair2->Source->Objectidentifier) &&
                     (trgpair1->Orientation == trgpair2->Orientation) &&
                     (trgpair1->Target->End == (trgpair2->Target->Start - 1)))
            {
                if (trgpair1->Orientation >= 0)
                {
                    /* Check for a potential parallel merge. */

                    if (trgpair1->Source->End == (trgpair2->Source->Start - 1))
                    {
                        if (debug)
                        {
                            ajDebug("mapperMapperpairsMerge merged %p with %p "
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

                    if (trgpair1->Source->Start == (trgpair2->Source->End + 1))
                    {
                        if (debug)
                        {
                            ajDebug("mapperMapperpairsMerge merged %p with %p "
                                    "in anti-parallel orientation.\n",
                                    trgpair1, trgpair2);

                            ensMapperpairTrace(trgpair1, 1);
                            ensMapperpairTrace(trgpair2, 1);
                        }

                        /* Merge in anti-parallel orientation. */

                        trgpair1->Source->Start = trgpair2->Source->Start;
                        trgpair1->Target->End   = trgpair2->Target->End;

                        delpair = trgpair2;
                    }
                }
            }

            /* Remove the redundant Mapper Pair also from the source Table. */

            if (delpair)
            {
                ajListIterRemove(trgiter);

                srctable = (AjPTable) ajTableFetchmodS(
                    mapper->Mapperpairs,
                    mapper->TypeSource);

                srclist = (AjPList) ajTableFetchmodV(
                    srctable,
                    (const void *) &delpair->Source->Objectidentifier);

                srciter = ajListIterNew(srclist);

                while (!ajListIterDone(srciter))
                {
                    srcpair = (EnsPMapperpair) ajListIterGet(srciter);

                    if (srcpair == delpair)
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

        mapper->Count = (ajuint) ajListGetLength(trglist);
    }

    AJFREE(valarray);

    return ajTrue;
}




/* @funcstatic mapperMapperpairsSort ******************************************
**
** Sort Ensembl Mapper Pair objects in an Ensembl Mapper.
** @cc Bio::EnsEMBL::Mapper::_sort
**
** @param [u] mapper [EnsPMapper] Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool mapperMapperpairsSort(EnsPMapper mapper)
{
    void **valarray = NULL;

    register ajuint i = 0U;

    AjPTable table = NULL;

    if (ajDebugTest("mapperMapperpairsSort"))
    {
        ajDebug("mapperMapperpairsSort\n"
                "  mapper %p\n",
                mapper);

        ensMapperTrace(mapper, 1);
    }

    if (!mapper)
        return ajFalse;

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                        mapper->TypeSource);

    ajTableToarrayValues(table, &valarray);

    for (i = 0U; valarray[i]; i++)
        ensListMapperpairSortSourceStartAscending((AjPList) valarray[i]);

    AJFREE(valarray);

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                        mapper->TypeTarget);

    ajTableToarrayValues(table, &valarray);

    for (i = 0U; valarray[i]; i++)
        ensListMapperpairSortTargetStartAscending((AjPList) valarray[i]);

    AJFREE(valarray);

    mapperMapperpairsMerge(mapper);

    mapper->Sorted = ajTrue;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Mapper object.
**
** @fdata [EnsPMapper]
**
** @nam3rule Add Add one object to an Ensembl Mapper
** @nam4rule Coordinates Add coordinates
** @nam4rule Indel       Add insertion-deletion coordinates
** @nam4rule Mapperpair  Add an Ensembl Mapper Pair
** @nam4rule Mappers     Add an Ensembl Mapper
** @nam4rule Mapperunits Add an Ensembl Mapper Unit pair
**
** @argrule Coordinates mapper [EnsPMapper] Ensembl Mapper
** @argrule Coordinates srcoid [ajuint] Source Object identifier
** @argrule Coordinates srcstart [ajint] Source start coordinate
** @argrule Coordinates srcend [ajint] Source end coordinate
** @argrule Coordinates ori [ajint] Orientation
** @argrule Coordinates trgoid [ajuint] Target Object identifier
** @argrule Coordinates trgstart [ajint] Target start coordinate
** @argrule Coordinates trgend [ajint] Target end coordinate
** @argrule Indel mapper [EnsPMapper] Ensembl Mapper
** @argrule Indel srcoid [ajuint] Source Object identifier
** @argrule Indel srcstart [ajint] Source start coordinate
** @argrule Indel srcend [ajint] Source end coordinate
** @argrule Indel ori [ajint] Orientation
** @argrule Indel trgoid [ajuint] Target Object identifier
** @argrule Indel trgstart [ajint] Target start coordinate
** @argrule Indel trgend [ajint] Target end coordinate
** @argrule Mapperpair mapper [EnsPMapper] Ensembl Mapper
** @argrule Mapperpair mp [EnsPMapperpair] Ensembl Mapper Pair
** @argrule Mappers mapper1 [EnsPMapper] Ensembl Mapper
** @argrule Mappers mapper2 [EnsPMapper] Ensembl Mapper
** @argrule Mapperunits mapper [EnsPMapper] Ensembl Mapper
** @argrule Mapperunits srcmu [EnsPMapperunit] Source Ensembl Mapper Unit
** @argrule Mapperunits trgmu [EnsPMapperunit] Target Ensembl Mapper Unit
** @argrule Mapperunits ori [ajint] Relative orientation of the Ensembl Mapper
** Unit objects
** @argrule Mapperunits indel [AjBool] Insertion-deletion attribute
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




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
**
** @release 6.2.0
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
    AjBool result = AJFALSE;

    EnsPMapperpair mp = NULL;

    if (ajDebugTest("ensMapperAddCoordinates"))
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

    if (!mapper)
        return ajFalse;

    if (!srcoid)
        return ajFalse;

    if (!trgoid)
        return ajFalse;

    if ((srcend - srcstart) != (trgend - trgstart))
        ajFatal("ensMapperAddCoordinates cannot deal with mis-lengthed "
                "mappings so far.\n");

    mp = ensMapperpairNewIni(srcoid, srcstart, srcend,
                             trgoid, trgstart, trgend,
                             ori, ajFalse);

    result = ensMapperAddMapperpair(mapper, mp);

    ensMapperpairDel(&mp);

    return result;
}




/* @func ensMapperAddIndel ****************************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperAddIndel(EnsPMapper mapper,
                         ajuint srcoid,
                         ajint srcstart,
                         ajint srcend,
                         ajint ori,
                         ajuint trgoid,
                         ajint trgstart,
                         ajint trgend)
{
    AjBool result = AJFALSE;

    EnsPMapperpair mp = NULL;

    if (!mapper)
        return ajFalse;

    if (!srcoid)
        return ajFalse;

    if (!trgoid)
        return ajFalse;

    mp = ensMapperpairNewIni(srcoid, srcstart, srcend,
                             trgoid, trgstart, trgend,
                             ori, ajTrue);

    result = ensMapperAddMapperpair(mapper, mp);

    ensMapperpairDel(&mp);

    return result;
}




/* @func ensMapperAddMapperpair ***********************************************
**
** Insert an Ensembl Mapper Pair into an Ensembl Mapper.
**
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [u] mp [EnsPMapperpair] Ensembl Mapper Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperAddMapperpair(EnsPMapper mapper, EnsPMapperpair mp)
{
    ajuint *Poid = NULL;

    AjPList list = NULL;

    AjPTable table = NULL;

    if (ajDebugTest("ensMapperAddMapperpair"))
    {
        ajDebug("ensMapperAddMapperpair\n"
                "  mapper %p\n"
                "  mp %p\n",
                mapper,
                mp);

        ensMapperpairTrace(mp, 1);
    }

    if (!mapper)
        return ajFalse;

    if (!mp)
        return ajFalse;

    if (!mp->Source)
        ajFatal("ensMapperAddMapperpair requires a Mapper Pair with a "
                "Source Mapper Unit.\n");

    if (!mp->Target)
        ajFatal("ensMapperAddMapperpair requires a Mapper Pair with a "
                "Target Mapper Unit.\n");

    /*
    ** Check for the matching Mapper Unit lengths if this is not a Mapper Pair
    ** reflecting an insertion or deletion.
    */

    if ((!mp->Indel) &&
        ((mp->Source->End - mp->Source->Start) !=
         (mp->Target->End - mp->Target->Start)))
        ajFatal("ensMapperAddMapperpair cannot deal with mis-lengthed "
                "mappings so far.\n");

    /*
    ** Insert the Mapper Pair into the source branch of the Mapper.
    **
    ** Search the first-level AJAX Table of (Ensembl Mapper type) AJAX String
    ** objects for the second-level AJAX Table of Ensembl Object identifers.
    */

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                        mapper->TypeSource);

    if (table)
    {
        /*
        ** Search the second-level AJAX Table of Ensembl Object identifiers
        ** for the third-level AJAX List of Ensembl Mapper Pair objects.
        */

        list = (AjPList) ajTableFetchmodV(
            table,
            (const void *) &mp->Source->Objectidentifier);

        if (!list)
        {
            AJNEW0(Poid);

            *Poid = mp->Source->Objectidentifier;

            list = ajListNew();

            ajTablePut(table, (void *) Poid, (void *) list);
        }

        ajListPushAppend(list, (void *) ensMapperpairNewRef(mp));
    }
    else
        ajFatal("ensMapperAddMapperpair first-level AJAX Table for "
                "Ensembl Mapper source type '%S' has not been initialised.",
                mapper->TypeSource);

    /*
    ** Insert the Mapper Pair into the target branch of the Mapper.
    **
    ** Search the first-level AJAX Table of (Ensembl Mapper type) AJAX String
    ** objects for the second-level AJAX Table of Ensembl Object identifers.
    */

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                        mapper->TypeTarget);

    if (table)
    {
        /*
        ** Search the second-level AJAX Table of Ensembl Object identifiers
        ** for the third-level AJAX List of Ensembl Mapper Pair objects.
        */

        list = (AjPList) ajTableFetchmodV(
            table,
            (const void *) &mp->Target->Objectidentifier);

        if (!list)
        {
            AJNEW0(Poid);

            *Poid = mp->Target->Objectidentifier;

            list = ajListNew();

            ajTablePut(table, (void *) Poid, (void *) list);
        }

        ajListPushAppend(list, (void *) ensMapperpairNewRef(mp));
    }
    else
        ajFatal("ensMapperAddMapperpair first-level AJAX Table for "
                "Ensembl Mapper target type '%S' has not been initialised.",
                mapper->TypeTarget);

    mapper->Count++;

    mapper->Sorted = ajFalse;

    return ajTrue;
}




/* @func ensMapperAddMappers **************************************************
**
** Transfer all Ensembl Mapper Pair objects from the second into the first
** Ensembl Mapper.
**
** @cc Bio::EnsEMBL::Mapper::add_Mapper
** @param [u] mapper1 [EnsPMapper] First Ensembl Mapper
** @param [u] mapper2 [EnsPMapper] Second Ensembl Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperAddMappers(EnsPMapper mapper1, EnsPMapper mapper2)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    ajuint srccounter = 0U;
    ajuint trgcounter = 0U;

    AjPList list1 = NULL;
    AjPList list2 = NULL;

    AjPTable table1 = NULL;
    AjPTable table2 = NULL;

    EnsPMapperpair mp = NULL;

    if (!mapper1)
        return ajFalse;

    if (!mapper2)
        return ajFalse;

    if (!ajStrMatchCaseS(mapper1->TypeSource, mapper2->TypeSource) ||
        !ajStrMatchCaseS(mapper1->TypeTarget, mapper2->TypeTarget))
        ajFatal("ensMapperAddMappers trying to add Ensembl Mapper of "
                "incompatible type ('%S:%S' vs '%S:%S').",
                mapper1->TypeSource,
                mapper1->TypeTarget,
                mapper2->TypeSource,
                mapper2->TypeTarget);

    /*
    ** Get the first-level AJAX Table objects for the source type of both
    ** Ensembl Mapper objects.
    */

    table1 = (AjPTable) ajTableFetchmodS(mapper1->Mapperpairs,
                                         mapper1->TypeSource);

    if (!table1)
        ajFatal("ensMapperAddMappers first-level AJAX Table for first "
                "Ensembl Mapper source type '%S' not initialised.",
                mapper1->TypeSource);

    table2 = (AjPTable) ajTableFetchmodS(mapper2->Mapperpairs,
                                         mapper2->TypeSource);

    if (!table2)
        ajFatal("ensMapperAddMappers first-level AJAX Table for second "
                "Ensembl Mapper source type '%S' not initialised.",
                mapper2->TypeSource);

    /*
    ** Convert the second-level AJAX Table with Ensembl Object
    ** identifiers as key data and AJAX List value data for the second
    ** Ensembl Mapper.
    */

    ajTableToarrayKeysValues(table2, &keyarray, &valarray);

    for (i = 0U; keyarray[i]; i++)
    {
        /*
        ** Get the third-level AJAX Lists for corresponding
        ** Ensembl Object identifiers.
        */

        list1 = (AjPList) ajTableFetchmodV(table1, (const void *) &keyarray[i]);

        list2 = (AjPList) valarray[i];

        while (ajListPop(list2, (void **) &mp))
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
    ** Get the first-level AJAX Table objects for the target type of both
    ** Ensembl Mapper objects.
    */

    table1 = (AjPTable) ajTableFetchmodS(mapper1->Mapperpairs,
                                         mapper1->TypeTarget);

    if (!table1)
        ajFatal("ensMapperAddMappers first-level AJAX Table for first "
                "Ensembl Mapper target type '%S' not initialised.",
                mapper1->TypeTarget);

    table2 = (AjPTable) ajTableFetchmodS(mapper2->Mapperpairs,
                                         mapper2->TypeTarget);

    if (!table2)
        ajFatal("ensMapperAddMappers first-level AJAX Table for second "
                "Ensembl Mapper target type '%S' not initialised.",
                mapper2->TypeTarget);

    /*
    ** Convert the second-level AJAX Table with Ensembl Object identifiers
    ** as key data and AJAX List value data for the second Ensembl Mapper.
    */

    ajTableToarrayKeysValues(table2, &keyarray, &valarray);

    for (i = 0U; keyarray[i]; i++)
    {
        /*
        ** Get the third-level AJAX Lists for corresponding
        ** Ensembl Object identifiers.
        */

        list1 = (AjPList) ajTableFetchmodV(table1, (const void *) &keyarray[i]);

        list2 = (AjPList) valarray[i];

        while (ajListPop(list2, (void **) &mp))
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

    if (srccounter == trgcounter)
        mapper1->Count += srccounter;
    else
        ajFatal("ensMapperAddMappers Numbers of Ensembl Mapper Pair objects "
                "for source (%u) and target (%u) types do not match in the "
                "second Ensembl Mapper.",
                srccounter, trgcounter);

    mapper1->Sorted = ajFalse;

    return ajTrue;
}




/* @func ensMapperAddMapperunits **********************************************
**
** Insert Ensembl Mapper Unit objects into an Ensembl Mapper.
**
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [u] srcmu [EnsPMapperunit] Source Ensembl Mapper Unit
** @param [u] trgmu [EnsPMapperunit] Target Ensembl Mapper Unit
** @param [r] ori [ajint] Relative orientation of the Ensembl Mapper Unit
**                        objects
** @param [r] indel [AjBool] Insertion-deletion attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperAddMapperunits(EnsPMapper mapper,
                               EnsPMapperunit srcmu,
                               EnsPMapperunit trgmu,
                               ajint ori,
                               AjBool indel)
{
    AjBool result = AJFALSE;

    EnsPMapperpair mp = NULL;

    if (!mapper)
        return ajFalse;

    if (!srcmu)
        return ajFalse;

    if (!trgmu)
        return ajFalse;

    mp = ensMapperpairNewUnit(srcmu, trgmu, ori, indel);

    result = ensMapperAddMapperpair(mapper, mp);

    ensMapperpairDel(&mp);

    return result;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Mapper object.
**
** @fdata [EnsPMapper]
**
** @nam3rule List List Ensembl Mapper Pair objects
** @nam3rule Trace Report Ensembl Mapper members to debug file
**
** @argrule List mapper [EnsPMapper] Ensembl Mapper
** @argrule List oid [ajuint] Ensembl Object identifier
** @argrule List start [ajint] Start coordinate
** @argrule List end [ajint] End coordinate
** @argrule List type [const AjPStr] Ensembl Mapper type
** @argrule List mps [AjPList] AJAX List of Ensembl Mapper Pair objects
** @argrule Trace mapper [const EnsPMapper] Ensembl Mapper
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperList ********************************************************
**
** List Ensembl Mapper Pair objects in an Ensembl Mapper.
** The caller is responsible for deleting the Ensembl Mapper Pair objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Mapper::list_pairs
** @param [u] mapper [EnsPMapper] Ensembl Mapper
** @param [r] oid [ajuint] Ensembl Object identifier
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] type [const AjPStr] Ensembl Mapper type
** @param [u] mps [AjPList] AJAX List of Ensembl Mapper Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperList(EnsPMapper mapper,
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

    if (!mapper)
        return ajFalse;

    if (!oid)
        return ajFalse;

    if (!type)
        return ajFalse;

    if (!mapper->Sorted)
        mapperMapperpairsSort(mapper);

    if (start > end)
        ajFatal("ensMapperList start (%d) is greater than end (%d) "
                "for Ensembl Object identifier %u.\n",
                start, end, oid);

    if (ajStrMatchCaseS(mapper->TypeSource, type))
        mutype = ensEMapperunitTypeSource;
    else if (ajStrMatchCaseS(mapper->TypeTarget, type))
        mutype = ensEMapperunitTypeTarget;
    else
        ajFatal("ensMapperList type '%S' is neither the source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type, mapper->TypeSource, mapper->TypeTarget);

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs, type);

    if (!table)
        ajFatal("ensMapperList first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.",
                type);

    list = (AjPList) ajTableFetchmodV(table, (const void *) &oid);

    if (!list)
        return ajTrue;

    iter = ajListIterNew(list);

    while (!ajListIterDone(iter))
    {
        mp = (EnsPMapperpair) ajListIterGet(iter);

        if ((!start) && (!end))
        {
            ajListPushAppend(mps, (void *) ensMapperpairNewRef(mp));

            continue;
        }

        mu = ensMapperpairCalculateMapperunit(mp, mutype);

        if (mu->End < start)
            continue;

        if (mu->Start > end)
            break;

        ajListPushAppend(mps, (void *) ensMapperpairNewRef(mp));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensMapperTrace *******************************************************
**
** Trace an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMapperTrace(const EnsPMapper mapper, ajuint level)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    ajuint *Poid = NULL;

    AjIList iter = NULL;

    AjPStr indent = NULL;

    AjPTable table = NULL;

    EnsPMapperpair mp = NULL;

    if (!mapper)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMapperTrace %p\n"
            "%S  TypeSource '%S'\n"
            "%S  TypeTarget '%S'\n"
            "%S  CoordsystemSource %p\n"
            "%S  CoordsystemTarget %p\n"
            "%S  Mapperpairs %p\n"
            "%S  Count %u\n"
            "%S  Sorted '%B'\n"
            "%S  Use %u\n",
            indent, mapper,
            indent, mapper->TypeSource,
            indent, mapper->TypeTarget,
            indent, mapper->CoordsystemSource,
            indent, mapper->CoordsystemTarget,
            indent, mapper->Mapperpairs,
            indent, mapper->Count,
            indent, mapper->Sorted,
            indent, mapper->Use);

    ensCoordsystemTrace(mapper->CoordsystemSource, level + 1);

    ensCoordsystemTrace(mapper->CoordsystemTarget, level + 1);

    /* Trace the AJAX Table for the TypeSource. */

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                        mapper->TypeSource);

    ajDebug("%S  AJAX Table %p for TypeSource '%S'\n",
            indent, table, mapper->TypeSource);

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

    for (i = 0U; valarray[i]; i++)
    {
        Poid = (ajuint *) keyarray[i];

        ajDebug("%S    AJAX List %p for Object identifier %u\n",
                indent, valarray[i], *Poid);

        iter = ajListIterNew((AjPList) valarray[i]);

        while (!ajListIterDone(iter))
        {
            mp = (EnsPMapperpair) ajListIterGet(iter);

            ensMapperpairTrace(mp, level + 3);
        }

        ajListIterDel(&iter);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    /* Trace the AJAX Table for the TypeTarget. */

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs,
                                        mapper->TypeTarget);

    ajDebug("%S  AJAX Table %p for TypeTarget '%S'\n",
            indent, table, mapper->TypeTarget);

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

    for (i = 0U; valarray[i]; i++)
    {
        Poid = (ajuint *) keyarray[i];

        ajDebug("%S    AJAX List %p for Object identifier %u\n",
                indent, valarray[i], *Poid);

        iter = ajListIterNew((AjPList) valarray[i]);

        while (!ajListIterDone(iter))
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




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Mapper object.
**
** @fdata [EnsPMapper]
**
** @nam3rule Calculate Calculate Ensembl Mapper values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mapper [const EnsPMapper] Ensembl Mapper
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMapperCalculateMemsize ********************************************
**
** Calculate the memory size in bytes of an Ensembl Mapper.
**
** @param [r] mapper [const EnsPMapper] Ensembl Mapper
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMapperCalculateMemsize(const EnsPMapper mapper)
{
    void **keyarray1 = NULL;
    void **valarray1 = NULL;

    void **valarray2 = NULL;

    register ajuint i = 0U;
    register ajuint j = 0U;

    size_t size = 0;

    AjIList iter = NULL;

    EnsPMapperpair mp = NULL;

    if (!mapper)
        return 0;

    size += sizeof (EnsOMapper);

    if (mapper->TypeSource)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mapper->TypeSource);
    }

    if (mapper->TypeTarget)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mapper->TypeTarget);
    }

    size += ensCoordsystemCalculateMemsize(mapper->CoordsystemSource);
    size += ensCoordsystemCalculateMemsize(mapper->CoordsystemTarget);

    /* Level 0 data (AjOTable). */

    size += sizeof (AjOTable);

    ajTableToarrayKeysValues(mapper->Mapperpairs, &keyarray1, &valarray1);

    for (i = 0U; valarray1[i]; i++)
    {
        /* Level 1 key data (AjOStr). */

        size += sizeof (AjOStr);

        size += ajStrGetRes((AjPStr) keyarray1[i]);

        /* Level 1 value data (AjOTable). */

        size += sizeof (AjOTable);

        ajTableToarrayValues(valarray1[i], &valarray2);

        for (j = 0U; valarray2[j]; j++)
        {
            /* Level 2 key data (ajuint). */

            size += sizeof (ajuint);

            /* Level 2 value data (AjOList). */

            size += sizeof (AjOList);

            iter = ajListIterNew((AjPList) valarray2[j]);

            while (!ajListIterDone(iter))
            {
                /* Level 3 data (EnsOMapperpair). */

                mp = (EnsPMapperpair) ajListIterGet(iter);

                size += ensMapperpairCalculateMemsize(mp);
            }

            ajListIterDel(&iter);
        }

        AJFREE(valarray2);
    }

    AJFREE(keyarray1);
    AJFREE(valarray1);

    return size;
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
**
** @release 6.2.0
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

    if (!mapper)
        return ajFalse;

    if (!oid)
        return ajFalse;

    if (ajDebugTest("mapperMapInsert"))
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

    ensMapperMap(mapper, oid, srcstart, srcend, strand,
                 type, coords);

    if (ajListGetLength(coords) == 1)
    {
        ajListPop(coords, (void **) &mr);

        /*
        ** Swap start and end to convert back into an insert where
        ** (start == end + 1)
        */

        srcstart = mr->CoordinateEnd;
        srcend   = mr->CoordinateStart;

        mr->CoordinateStart = srcstart;
        mr->CoordinateEnd   = srcend;

        ajListPushAppend(mrs, (void *) ensMapperresultNewRef(mr));

        ensMapperresultDel(&mr);
    }
    else
    {
        if (ajListGetLength(coords) != 2)
            ajFatal("mapperMapInsert got %d Ensembl Mapper Pair objects "
                    "but expected only two.\n",
                    ajListGetLength(coords));

        /* Adjust coordinates and remove gaps. */

        if (strand < 0)
            ajListReverse(coords);

        ajListPop(coords, (void **) &mr);

        if (mr->Type == ensEMapperresultTypeCoordinate)
        {
            /* The insert is after the first coordinate. */

            if ((mr->CoordinateStrand * strand) < 0)
                mr->CoordinateEnd--;
            else
                mr->CoordinateStart++;

            ajListPushAppend(mrs, (void *) ensMapperresultNewRef(mr));
        }

        ensMapperresultDel(&mr);

        ajListPop(coords, (void **) &mr);

        if (mr->Type == ensEMapperresultTypeCoordinate)
        {
            /* The insert is before the second coordinate. */

            if ((mr->CoordinateStrand * strand) < 0)
                mr->CoordinateStart++;
            else
                mr->CoordinateEnd++;

            if (strand < 0)
                ajListPush(mrs, (void *) ensMapperresultNewRef(mr));
            else
                ajListPushAppend(mrs, (void *) ensMapperresultNewRef(mr));
        }

        ensMapperresultDel(&mr);
    }

    if (fastmap && (ajListGetLength(mrs) != 1))
        while (ajListPop(mrs, (void **) &mr))
            ensMapperresultDel(&mr);

    ajListFree(&coords);

    return ajTrue;
}




/* @section map ***************************************************************
**
** Map coordinates between Ensembl Coordinate Systems.
**
** @fdata [EnsPMapper]
**
** @nam3rule Fastmap Fast map coordinates
** @nam3rule Map Map coordinates
** @nam3rule Mapindel Map insertion-deletion coordinates
**
** @argrule * mapper [EnsPMapper] Ensembl Mapper
** @argrule * oid [ajuint] Ensembl Object Identifier
** @argrule * start [ajint] Start coordinate
** @argrule * end [ajint] End coordinate
** @argrule * strand [ajint] Strand information
** @argrule * type [const AjPStr] Ensembl Mapper type to map from
** @argrule * mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMapperFastmap *****************************************************
**
** Inferior mapping function, which will only perform ungapped,
** unsplit mapping.
**
** The caller is responsible for deleting the Ensembl Mapper Result objects
** before deleting the AJAX List.
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperFastmap(EnsPMapper mapper,
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

    debug = ajDebugTest("ensMapperFastmap");

    if (debug)
        ajDebug("ensMapperFastmap\n"
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

    if (!mapper)
        return ajFalse;

    if (!oid)
        return ajFalse;

    if (!type)
        return ajFalse;

    if (!mrs)
        return ajFalse;

    if (start == (end + 1))
        return mapperMapInsert(mapper, oid, start, end, strand, type,
                               ajTrue, mrs);

    if (!mapper->Sorted)
        mapperMapperpairsSort(mapper);

    if (ajStrMatchCaseS(mapper->TypeSource, type))
    {
        srctype = ensEMapperunitTypeSource;
        trgtype = ensEMapperunitTypeTarget;

        cs = mapper->CoordsystemTarget;
    }
    else if (ajStrMatchCaseS(mapper->TypeTarget, type))
    {
        srctype = ensEMapperunitTypeTarget;
        trgtype = ensEMapperunitTypeSource;

        cs = mapper->CoordsystemSource;
    }
    else
        ajFatal("ensMapperFastmap type '%S' is neither the "
                "source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type,
                mapper->TypeSource,
                mapper->TypeTarget);

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs, type);

    if (!table)
        ajFatal("ensMapperFastmap first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.\n",
                type);

    list = (AjPList) ajTableFetchmodV(table, (const void *) &oid);

    if (!(list && ajListGetLength(list)))
    {
        /*
        ** If an Ensembl Object identifier is not associated with an
        ** AJAX List of Ensembl Mapper Pair objects or the AJAX List is empty,
        ** the whole region is just one big gap. The ensMapperFastmap
        ** function returns no Ensembl Mapper Result.
        */

        if (debug)
            ajDebug("ensMapperFastmap could not find an AJAX List for "
                    "Ensembl Object identifier %u or the List is empty "
                    "--> one big gap!\n",
                    oid);

        return ajTrue;
    }

    iter = ajListIterNew(list);

    while (!ajListIterDone(iter))
    {
        mp = (EnsPMapperpair) ajListIterGet(iter);

        srcmu = ensMapperpairCalculateMapperunit(mp, srctype);
        trgmu = ensMapperpairCalculateMapperunit(mp, trgtype);

        /* Only super easy mapping is done! */

        if ((start < srcmu->Start) || (end > srcmu->End))
            continue;

        if (mp->Orientation >= 0)
            mr = ensMapperresultNewCoordinate(
                trgmu->Objectidentifier,
                trgmu->Start + (start - srcmu->Start),
                trgmu->Start + (end   - srcmu->Start),
                +strand,
                cs,
                0);
        else
            mr = ensMapperresultNewCoordinate(
                trgmu->Objectidentifier,
                trgmu->End - (end   - srcmu->Start),
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




/* @func ensMapperMap *********************************************************
**
** Map coordinates.
**
** The caller is responsible for deleting the Ensembl Mapper Result obects
** before deleting the AJAX List.
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperMap(EnsPMapper mapper,
                    ajuint oid,
                    ajint start,
                    ajint end,
                    ajint strand,
                    const AjPStr type,
                    AjPList mrs)
{
    register ajuint i = 0U;

    ajuint idxstart  = 0U;
    ajuint idxmid    = 0U;
    ajuint idxend    = 0U;
    ajuint idxlength = 0U;
    ajuint rank      = 0U;

    ajint srcstart = 0;
    ajint srcend   = 0;

    ajuint trgoid  = 0U;
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

    debug = ajDebugTest("ensMapperMap");

    if (debug)
        ajDebug("ensMapperMap\n"
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

    if (!mapper)
        return ajFalse;

    if (!oid)
        return ajFalse;

    if (!type)
        return ajFalse;

    if (!mrs)
        return ajFalse;

    if (start == (end + 1))
        return mapperMapInsert(mapper, oid, start, end, strand, type,
                               ajFalse, mrs);

    if (!mapper->Sorted)
        mapperMapperpairsSort(mapper);

    if (ajStrMatchCaseS(mapper->TypeSource, type))
    {
        srctype = ensEMapperunitTypeSource;
        trgtype = ensEMapperunitTypeTarget;

        cs = mapper->CoordsystemTarget;
    }
    else if (ajStrMatchCaseS(mapper->TypeTarget, type))
    {
        srctype = ensEMapperunitTypeTarget;
        trgtype = ensEMapperunitTypeSource;

        cs = mapper->CoordsystemSource;
    }
    else
        ajFatal("ensMapperMap type '%S' is neither the "
                "source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type,
                mapper->TypeSource,
                mapper->TypeTarget);

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs, type);

    if (!table)
        ajFatal("ensMapperMap first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.\n",
                type);

    list = (AjPList) ajTableFetchmodV(table, (const void *) &oid);

    if (!(list && (idxlength = (ajuint) ajListGetLength(list))))
    {
        /*
        ** If an Ensembl Object identifier is not associated with an
        ** AJAX List of Ensembl Mapper Pair objects or the AJAX List is empty,
        ** the whole region is just one big gap.
        */

        if (debug)
            ajDebug("ensMapperMap could not find an AJAX List for "
                    "Ensembl Object identifier %u or the List is empty "
                    "--> one big gap!\n",
                    oid);

        mr = ensMapperresultNewGap(start, end, 0);

        ajListPushAppend(mrs, (void *) mr);

        return ajTrue;
    }

    /*
    ** Binary search the relevant Ensembl Mapper Pair objects,
    ** which helps if the AJAX List of Ensembl Mapper Pair objects is long.
    */

    idxstart = 0U;
    idxend   = idxlength - 1U;

    while ((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &mp);

        srcmu = ensMapperpairCalculateMapperunit(mp, srctype);

        if (srcmu->End < start)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    srcstart = start;
    srcend   = end;

    for (i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &mp);

        srcmu = ensMapperpairCalculateMapperunit(mp, srctype);
        trgmu = ensMapperpairCalculateMapperunit(mp, trgtype);

        if (debug)
            ajDebug("ensMapperMap coordinates "
                    "%u:%d:%d:%d %d:%d srcMU %u:%d:%d\n",
                    oid,
                    start,
                    end,
                    strand,
                    srcstart,
                    srcend,
                    srcmu->Objectidentifier,
                    srcmu->Start,
                    srcmu->End);

        /*
        ** NOTE: This breaks the haplotype projection and was therefore removed
        ** in CVS revision 1.49
        **
        if (srcmu->Start < start)
        {
        srcstart = start;

        rank++;
        }
        */

        /*
        ** Check for cases where the source Mapper Unit maps to more than one
        ** location.
        ** If the target object identifier changes, and the source start
        ** coordinate is less than the current start coordinate, this source
        ** Mapper Unit maps to more than one location on a target Mapper Unit.
        ** Reset the current source start position to the original start.
        */

        if (trgoid && (trgoid != trgmu->Objectidentifier))
        {
            if (srcmu->Start < start)
                srcstart = start;
        }
        else
            trgoid = trgmu->Objectidentifier;

        /* In case the loop hasn't even reached the start, move on. */

        if (srcmu->End < start)
            continue;

        /* In case the loop has over-run, break. */

        if (srcmu->Start > srcend)
            break;

        if (srcmu->Start > srcstart)
        {
            /* A gap has been detected. */

            mr = ensMapperresultNewGap(srcstart, srcmu->Start - 1, rank);

            ajListPushAppend(mrs, (void *) mr);

            srcstart = srcmu->Start;
        }

        if (mp->Indel)
        {
            /*
            ** If the Mapper Pair represents an insertion or deletion,
            ** create a Mapper Result of type insertion or deletion.
            */

            mr = ensMapperresultNewIndel(trgmu->Objectidentifier,
                                         trgmu->Start,
                                         trgmu->End,
                                         mp->Orientation * strand,
                                         cs,
                                         srcstart,
                                         (srcmu->End < srcend) ?
                                         srcmu->End : srcend,
                                         rank);
        }
        else
        {
            /* The start is somewhere inside the region. */

            if (mp->Orientation >= 0)
                trgstart = trgmu->Start + (srcstart - srcmu->Start);
            else
                trgend = trgmu->End - (srcstart - srcmu->Start);

            /*
            ** Either we are enveloping this map or not. If yes, then the end
            ** point (self perspective) is determined solely by target.
            ** If not we need to adjust.
            */

            if (srcend > srcmu->End)
            {
                /* enveloped */

                if (mp->Orientation >= 0)
                    trgend = trgmu->End;
                else
                    trgstart = trgmu->Start;
            }
            else
            {
                /* The end needs to be adjusted. */

                if (mp->Orientation >= 0)
                    trgend = trgmu->Start + (srcend - srcmu->Start);
                else
                    trgstart = trgmu->End - (srcend - srcmu->Start);
            }

            mr = ensMapperresultNewCoordinate(trgmu->Objectidentifier,
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

    if (lastmp)
    {
        /*
        ** Previously, a Mapper Pair has been found,
        ** check for a gap inbetween.
        */

        srcmu = ensMapperpairCalculateMapperunit(lastmp, srctype);

        if (srcmu->End < srcend)
        {
            /* A gap at the end has been detected. */

            mr = ensMapperresultNewGap(srcmu->End + 1, srcend, rank);

            ajListPushAppend(mrs, (void *) mr);
        }
    }
    else
    {
        /* Since no Mapper Pair has been found the entire region is a gap. */

        mr = ensMapperresultNewGap(srcstart, srcend, 0);

        ajListPushAppend(mrs, (void *) mr);
    }

    if (strand < 0)
        ajListReverse(mrs);

    return ajTrue;
}




/* @func ensMapperMapindel ****************************************************
**
** Map insertion-deletion coordinates.
**
** The caller is responsible for deleting the Ensembl Mapper Result objects
** before deleting the AJAX List.
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMapperMapindel(EnsPMapper mapper,
                         ajuint oid,
                         ajint start,
                         ajint end,
                         ajint strand,
                         const AjPStr type,
                         AjPList mrs)
{
    register ajuint i = 0U;

    ajuint idxstart  = 0U;
    ajuint idxend    = 0U;
    ajuint idxmid    = 0U;
    ajuint idxlength = 0U;

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

    debug = ajDebugTest("ensMapperMapindel");

    if (debug)
        ajDebug("ensMapperMapindel\n"
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

    if (!mapper)
        return ajFalse;

    if (!oid)
        return ajFalse;

    if (!type)
        return ajFalse;

    if (!mrs)
        return ajFalse;

    if (!mapper->Sorted)
        mapperMapperpairsSort(mapper);

    if (ajStrMatchCaseS(mapper->TypeSource, type))
    {
        srctype = ensEMapperunitTypeSource;
        trgtype = ensEMapperunitTypeTarget;

        cs = mapper->CoordsystemTarget;
    }
    else if (ajStrMatchCaseS(mapper->TypeTarget, type))
    {
        srctype = ensEMapperunitTypeTarget;
        trgtype = ensEMapperunitTypeSource;

        cs = mapper->CoordsystemSource;
    }
    else
        ajFatal("ensMapperMapindel type '%S' is neither the "
                "source '%S' nor "
                "target '%S' type of the Ensembl Mapper.\n",
                type,
                mapper->TypeSource,
                mapper->TypeTarget);

    table = (AjPTable) ajTableFetchmodS(mapper->Mapperpairs, type);

    if (!table)
        ajFatal("ensMapperMapindel first-level AJAX Table for "
                "Ensembl Mapper type '%S' has not been initialised.",
                type);

    list = (AjPList) ajTableFetchmodV(table, (const void *) &oid);

    if (!(list && (idxlength = (ajuint) ajListGetLength(list))))
    {
        /*
        ** If an Ensembl Object identifier is not associated with an
        ** AJAX List of Ensembl Mapper Pair objects or the AJAX List is empty,
        ** the whole region is just one big gap.
        */

        if (debug)
            ajDebug("ensMapperMapindel could not find an AJAX List for "
                    "Ensembl Object identifier %u or the List is empty\n",
                    oid);

        return ajTrue;
    }

    /*
    ** Binary search the relevant Ensembl Mapper Pair objects,
    ** which helps if the AJAX List of Ensembl Mapper Pair objects is long.
    */

    idxstart = 0U;
    idxend   = idxlength - 1U;

    while ((idxend - idxstart) > 1)
    {
        idxmid = (idxstart + idxend) >> 1;

        ajListPeekNumber(list, idxmid, (void **) &mp);

        srcmu = ensMapperpairCalculateMapperunit(mp, srctype);

        /*
        ** NOTE: The ensMapperMap function checks for source
        ** Mapper Unit end less than start. if (srcmu->End < start)
        ** This function has already swapped the start and end coordinates
        ** for the insertion-deletion and checks for source Mapper Unit end
        ** less than *or equal* the start. Since the coordinates have not
        ** been swapped here, this becomes if (srcmu->End <= end)
        */

        if (srcmu->End <= end)
            idxstart = idxmid;
        else
            idxend = idxmid;
    }

    for (i = idxstart; i < idxlength; i++)
    {
        ajListPeekNumber(list, i, (void **) &mp);

        trgmu = ensMapperpairCalculateMapperunit(mp, trgtype);

        if (mp->Indel)
        {
            mr = ensMapperresultNewCoordinate(trgmu->Objectidentifier,
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

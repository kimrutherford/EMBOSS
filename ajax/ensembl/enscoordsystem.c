/* @source enscoordsystem *****************************************************
**
** Ensembl Coordinate System functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.49 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#include "enscoordsystem.h"
#include "ensmetainformation.h"
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

static int listCoordsystemCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listCoordsystemCompareIdentifierDescending(
    const void *item1,
    const void *item2);

static int listCoordsystemCompareRankAscending(
    const void *item1,
    const void *item2);

static int listCoordsystemCompareRankDescending(
    const void *item1,
    const void *item2);

static AjBool coordsystemadaptorFetchAllbyStatement(
    EnsPCoordsystemadaptor csa,
    const AjPStr statement,
    AjPList css);

static AjBool coordsystemadaptorCacheInit(EnsPCoordsystemadaptor csa);

static AjBool coordsystemadaptorMappingpathInit(EnsPCoordsystemadaptor csa);

static AjBool coordsystemadaptorSeqregionMapInit(EnsPCoordsystemadaptor csa);

static void coordsystemadaptorMappingpathValdel(void **Pvalue);

static void coordsystemadaptorFetchAll(const void *key,
                                       void **Pvalue,
                                       void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enscoordsystem ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library.
**
******************************************************************************/




/* @datasection [EnsPCoordsystem] Ensembl Coordinate System *******************
**
** @nam2rule Coordsystem Functions for manipulating
** Ensembl Coordinate System objects
**
** @cc Bio::EnsEMBL::CoordSystem
** @cc CVS Revision: 1.13
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Coordinate System by pointer.
** It is the responsibility of the user to first destroy any previous
** Coordinate System. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the use counter
**
** @argrule Cpy cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule Ini csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ini version [AjPStr] Version
** @argrule Ini rank [ajuint] Rank
** @argrule Ini dflt [AjBool] Default attribute
** @argrule Ini toplevel [AjBool] Top-level attrbute
** @argrule Ini seqlevel [AjBool] Sequence-level attribute
** @argrule Ref cs [EnsPCoordsystem] Ensembl Coordinate System
**
** @valrule * [EnsPCoordsystem] Ensembl Coordinate System or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensCoordsystemNewCpy *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPCoordsystem ensCoordsystemNewCpy(const EnsPCoordsystem cs)
{
    EnsPCoordsystem pthis = NULL;

    if (!cs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = cs->Identifier;

    pthis->Adaptor = cs->Adaptor;

    if (cs->Name)
        pthis->Name = ajStrNewRef(cs->Name);

    /*
    ** Although Coordinate System versions are optional, the AJAX String
    ** should always be defined, since Ensembl Slice names are depending
    ** on it.
    */

    if (cs->Version)
        pthis->Version = ajStrNewRef(cs->Version);
    else
        pthis->Version = ajStrNew();

    pthis->Default = cs->Default;

    pthis->SequenceLevel = cs->SequenceLevel;

    pthis->Toplevel = cs->Toplevel;

    pthis->Rank = cs->Rank;

    return pthis;
}




/* @func ensCoordsystemNewIni *************************************************
**
** Constructor for an Ensembl Coordinate System with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::CoordSystem::new
** @param [u] name [AjPStr] Name
** @param [uN] version [AjPStr] Version
** @param [r] rank [ajuint] Rank
** @param [r] dflt [AjBool] Default attribute
** @param [r] toplevel [AjBool] Top-level attrbute
** @param [r] seqlevel [AjBool] Sequence-level attribute
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
**
** @release 6.4.0
** @@
** Many Ensembl Coordinate Systems do not have a concept of a version for the
** entire Coordinate System, although they may have a per-sequence version.
** The 'chromosome' Coordinate System usually has a version - i.e. the
** assembly build version - but the clonal Coordinate System does not, despite
** having individual sequence versions. In the case where a Coordinate System
** does not have a version an empty string is used instead.
**
** The highest-level Coordinate System (e.g. chromosome) should have rank 1,
** the second-highest level Coordinate System (e.g. clone) should have rank 2
** and so on.
**
** Top-level should only be set for creating an artificial top-level Coordinate
** System of name 'toplevel'.
******************************************************************************/

EnsPCoordsystem ensCoordsystemNewIni(
    EnsPCoordsystemadaptor csa,
    ajuint identifier,
    AjPStr name,
    AjPStr version,
    ajuint rank,
    AjBool dflt,
    AjBool toplevel,
    AjBool seqlevel)
{
    EnsPCoordsystem cs = NULL;

    if (ajDebugTest("ensCoordsystemNewIni"))
        ajDebug("ensCoordsystemNewIni\n"
                "  csa %p\n"
                "  identifier %u\n"
                "  name '%S'\n"
                "  version '%S'\n"
                "  rank %u\n"
                "  dflt %B\n"
                "  toplevel %B\n"
                "  seqlevel %B\n",
                csa,
                identifier,
                name,
                version,
                rank,
                dflt,
                toplevel,
                seqlevel);

    if (toplevel)
    {
        if (name && ajStrGetLen(name))
        {
            if (!ajStrMatchCaseC(name, "toplevel"))
            {
                ajWarn("ensCoordsystemNewIni name parameter must be 'toplevel' "
                       "if the top-level parameter is set.\n");

                return NULL;
            }
        }

        if (rank > 0)
        {
            ajWarn("ensCoordsystemNewIni rank parameter must be 0 "
                   "if the top-level parameter is set.\n");

            return NULL;
        }

        if (seqlevel == ajTrue)
        {
            ajWarn("ensCoordsystemNewIni sequence-level parameter must not "
                   "be set if the top-level parameter is set.\n");

            return NULL;
        }

        if (dflt == ajTrue)
        {
            ajWarn("ensCoordsystemNewIni default parameter must not be set "
                   "if the top-level parameter is set.\n");

            return NULL;
        }
    }
    else
    {
        if (name && ajStrGetLen(name))
        {
            if (ajStrMatchCaseC(name, "toplevel"))
            {
                ajWarn("ensCoordsystemNewIni name parameter cannot be "
                       "'toplevel' for non-top-level Coordinate Systems.\n");

                return NULL;
            }
        }
        else
        {
            ajWarn("ensCoordsystemNewIni name parameter must be provided for "
                   "non-top-level Coordinate Systems.\n");

            return NULL;
        }

        if (rank == 0)
        {
            ajWarn("ensCoordsystemNewIni rank parameter must be non-zero "
                   "for non-top-level Coordinate Systems.\n");

            return NULL;
        }
    }

    AJNEW0(cs);

    cs->Use = 1U;

    cs->Identifier = identifier;

    cs->Adaptor = csa;

    if (toplevel)
        cs->Name = ajStrNewC("toplevel");
    else if (name)
        cs->Name = ajStrNewRef(name);

    /*
    ** Although Coordinate System versions are optional, the AJAX String
    ** should always be defined, since Ensembl Slice names are depending
    ** on it.
    */

    if (version)
        cs->Version = ajStrNewRef(version);
    else
        cs->Version = ajStrNew();

    cs->Rank = rank;

    cs->SequenceLevel = seqlevel;

    cs->Toplevel = toplevel;

    cs->Default = dflt;

    return cs;
}




/* @func ensCoordsystemNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPCoordsystem ensCoordsystemNewRef(EnsPCoordsystem cs)
{
    if (ajDebugTest("ensCoordsystemNewRef"))
    {
        ajDebug("ensCoordsystemNewRef\n"
                "  cs %p\n",
                cs);

        ensCoordsystemTrace(cs, 1);
    }

    if (!cs)
        return NULL;

    cs->Use++;

    return cs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule Del Destroy (free) an Ensembl Coordinate System
**
** @argrule * Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensCoordsystemDel ****************************************************
**
** Default destructor for an Ensembl Coordinate System.
**
** @param [d] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensCoordsystemDel(EnsPCoordsystem *Pcs)
{
    EnsPCoordsystem pthis = NULL;

    if (!Pcs)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensCoordsystemDel"))
    {
        ajDebug("ensCoordsystemDel\n"
                "  *Pcs %p\n",
                *Pcs);

        ensCoordsystemTrace(*Pcs, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pcs)
        return;

    pthis = *Pcs;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pcs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);

    ajStrDel(&pthis->Version);

    AJFREE(pthis);

    *Pcs = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule Get Return Coordinate System attribute(s)
** @nam4rule Adaptor Return the Ensembl Coordinate System Adaptor
** @nam4rule Default Return the default attribute
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Rank Return the rank
** @nam4rule Seqlevel Return the sequence-level attribute
** @nam4rule Toplevel Return the top-level attribute
** @nam4rule Version Return the version
**
** @argrule * cs [const EnsPCoordsystem] Coordinate System
**
** @valrule Adaptor [EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor or NULL
** @valrule Default [AjBool] Default attribute or ajFalse
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [const AjPStr] Name or NULL
** @valrule Rank [ajuint] Rank or 0U
** @valrule Seqlevel [AjBool] Sequence-level attribute or ajFalse
** @valrule Toplevel [AjBool] Top-level attribute or ajFalse
** @valrule Version [const AjPStr] Version or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensCoordsystemGetAdaptor *********************************************
**
** Get the Ensembl Coordinate System Adaptor member of an
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPCoordsystemadaptor ensCoordsystemGetAdaptor(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Adaptor : NULL;
}




/* @func ensCoordsystemGetDefault *********************************************
**
** Get the default member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::CoordSystem::is_default
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the Coordinate System version defines the
**                  default of all Coordinate Systems with the same name.
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemGetDefault(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Default : ajFalse;
}




/* @func ensCoordsystemGetIdentifier ******************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensCoordsystemGetIdentifier(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Identifier : 0U;
}




/* @func ensCoordsystemGetName ************************************************
**
** Get the name member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::CoordSystem::name
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [const AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensCoordsystemGetName(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Name : NULL;
}




/* @func ensCoordsystemGetRank ************************************************
**
** Get the rank member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::CoordSystem::rank
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [ajuint] Rank or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensCoordsystemGetRank(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Rank : 0U;
}




/* @func ensCoordsystemGetSeqlevel ********************************************
**
** Get the sequence-level member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::CoordSystem::is_sequence_level
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the Coordinate System defines the sequence-level
** or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensCoordsystemGetSeqlevel(const EnsPCoordsystem cs)
{
    return (cs) ? cs->SequenceLevel : ajFalse;
}




/* @func ensCoordsystemGetToplevel ********************************************
**
** Get the top-level member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::CoordSystem::is_top_level
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the coordinate system defines the top-level
** or ajFalse
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensCoordsystemGetToplevel(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Toplevel : ajFalse;
}




/* @func ensCoordsystemGetVersion *********************************************
**
** Get the version member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::CoordSystem::version
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [const AjPStr] Version or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ensCoordsystemGetVersion(const EnsPCoordsystem cs)
{
    return (cs) ? cs->Version : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule Set Set one member of a Coordinate System
** @nam4rule Adaptor Set the Ensembl Coordinate System Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * cs [EnsPCoordsystem] Ensembl Coordinate System object
** @argrule Adaptor csa [EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensCoordsystemSetAdaptor *********************************************
**
** Set the Object Adaptor member of an Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [u] csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemSetAdaptor(EnsPCoordsystem cs,
                                EnsPCoordsystemadaptor csa)
{
    if (!cs)
        return ajFalse;

    cs->Adaptor = csa;

    return ajTrue;
}




/* @func ensCoordsystemSetIdentifier ******************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemSetIdentifier(EnsPCoordsystem cs, ajuint identifier)
{
    if (!cs)
        return ajFalse;

    cs->Identifier = identifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule Trace Report Ensembl Coordinate System members to debug file
**
** @argrule Trace cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensCoordsystemTrace **************************************************
**
** Trace an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemTrace(const EnsPCoordsystem cs, ajuint level)
{
    AjPStr indent = NULL;

    if (!cs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensCoordsystemTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Version '%S'\n"
            "%S  SequenceLevel '%B'\n"
            "%S  Toplevel '%B'\n"
            "%S  Default '%B'\n"
            "%S  Rank %u\n",
            indent, cs,
            indent, cs->Use,
            indent, cs->Identifier,
            indent, cs->Adaptor,
            indent, cs->Name,
            indent, cs->Version,
            indent, cs->SequenceLevel,
            indent, cs->Toplevel,
            indent, cs->Default,
            indent, cs->Rank);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule Calculate Calculate Ensembl Coordinate System values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensCoordsystemCalculateMemsize ***************************************
**
** Get the memory size in bytes of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensCoordsystemCalculateMemsize(const EnsPCoordsystem cs)
{
    size_t size = 0;

    if (!cs)
        return 0;

    size += sizeof (EnsOCoordsystem);

    if (cs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(cs->Name);
    }

    if (cs->Version)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(cs->Version);
    }

    return size;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Coordinate System objects.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule  Match Compare two Ensembl Coordinate System objects
**
** @argrule * cs1 [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule * cs2 [const EnsPCoordsystem] Ensembl Coordinate System
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensCoordsystemMatch **************************************************
**
** Test for matching two Ensembl Coordinate Systems.
**
** @cc Bio::EnsEMBL::CoordSystem::equals
** @param [r] cs1 [const EnsPCoordsystem] First Ensembl Coordinate System
** @param [r] cs2 [const EnsPCoordsystem] Second Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the Ensembl Coordinate System objects are equal
**
** @release 6.2.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name and version members
** is performed.
******************************************************************************/

AjBool ensCoordsystemMatch(const EnsPCoordsystem cs1,
                           const EnsPCoordsystem cs2)
{
    if (ajDebugTest("ensCoordsystemMatch"))
    {
        ajDebug("ensCoordsystemMatch\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                cs1,
                cs2);

        ensCoordsystemTrace(cs1, 1);
        ensCoordsystemTrace(cs2, 1);
    }

    if (!cs1)
        return ajFalse;

    if (!cs2)
        return ajFalse;

    if (cs1 == cs2)
        return ajTrue;

    if (cs1->Identifier != cs2->Identifier)
        return ajFalse;

    if (!ajStrMatchCaseS(cs1->Name, cs2->Name))
        return ajFalse;

    if (!ajStrMatchCaseS(cs1->Version, cs2->Version))
        return ajFalse;

    if (cs1->Default != cs2->Default)
        return ajFalse;

    if (cs1->SequenceLevel != cs2->SequenceLevel)
        return ajFalse;

    if (cs1->Toplevel != cs2->Toplevel)
        return ajFalse;

    if (cs1->Rank != cs2->Rank)
        return ajFalse;

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Coordinate System convenience functions
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Species Get the Ensembl Database Adaptor species
**
** @argrule * cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @valrule Species [AjPStr] Ensembl Database Adaptor species or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensCoordsystemGetSpecies *********************************************
**
** Get the species member of the Ensembl Database Adaptor the
** Ensembl Coordinate System Adaptor of an Ensembl Coordinate System is
** based on.
**
** @cc Bio::EnsEMBL::CoordSystem::species
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjPStr] Ensembl Database Adaptor species or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensCoordsystemGetSpecies(const EnsPCoordsystem cs)
{
    if (!cs)
        return NULL;

    if (!cs->Adaptor)
        return NULL;

    return ensDatabaseadaptorGetSpecies(cs->Adaptor->Adaptor);
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listCoordsystemCompareIdentifierAscending **********************
**
** AJAX List of Ensembl Coordinate System objects comparison function to
** sort by Ensembl Coordinate System identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Coordinate System address 1
** @param [r] item2 [const void*] Ensembl Coordinate System address 2
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

static int listCoordsystemCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPCoordsystem cs1 = *(EnsOCoordsystem *const *) item1;
    EnsPCoordsystem cs2 = *(EnsOCoordsystem *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listCoordsystemCompareIdentifierAscending"))
        ajDebug("listCoordsystemCompareIdentifierAscending\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                cs1,
                cs2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (cs1 && (!cs2))
        return -1;

    if ((!cs1) && (!cs2))
        return 0;

    if ((!cs1) && cs2)
        return +1;

    if (cs1->Identifier < cs2->Identifier)
        return -1;

    if (cs1->Identifier > cs2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listCoordsystemCompareIdentifierDescending *********************
**
** AJAX List of Ensembl Coordinate System objects comparison function to
** sort by Ensembl Coordinate System identifier in descending order.
**
** @param [r] item1 [const void*] Ensembl Coordinate System address 1
** @param [r] item2 [const void*] Ensembl Coordinate System address 2
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

static int listCoordsystemCompareIdentifierDescending(
    const void *item1,
    const void *item2)
{
    EnsPCoordsystem cs1 = *(EnsOCoordsystem *const *) item1;
    EnsPCoordsystem cs2 = *(EnsOCoordsystem *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listCoordsystemCompareIdentifierDescending"))
        ajDebug("listCoordsystemCompareIdentifierDescending\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                cs1,
                cs2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (cs1 && (!cs2))
        return -1;

    if ((!cs1) && (!cs2))
        return 0;

    if ((!cs1) && cs2)
        return +1;

    if (cs1->Identifier > cs2->Identifier)
        return -1;

    if (cs1->Identifier < cs2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listCoordsystemCompareRankAscending ****************************
**
** AJAX List of Ensembl Coordinate System objects comparison function to
** sort by Ensembl Coordinate System rank in ascending order.
**
** @param [r] item1 [const void*] Ensembl Coordinate System address 1
** @param [r] item2 [const void*] Ensembl Coordinate System address 2
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

static int listCoordsystemCompareRankAscending(
    const void *item1,
    const void *item2)
{
    EnsPCoordsystem cs1 = *(EnsOCoordsystem *const *) item1;
    EnsPCoordsystem cs2 = *(EnsOCoordsystem *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listCoordsystemCompareRankAscending"))
        ajDebug("listCoordsystemCompareRankAscending\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                cs1,
                cs2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (cs1 && (!cs2))
        return -1;

    if ((!cs1) && (!cs2))
        return 0;

    if ((!cs1) && cs2)
        return +1;

    if (cs1->Rank < cs2->Rank)
        return -1;

    if (cs1->Rank > cs2->Rank)
        return +1;

    return 0;
}




/* @funcstatic listCoordsystemCompareRankDescending ***************************
**
** AJAX List of Ensembl Coordinate System objects comparison function to
** sort by Ensembl Coordinate System rank in descending order.
**
** @param [r] item1 [const void*] Ensembl Coordinate System address 1
** @param [r] item2 [const void*] Ensembl Coordinate System address 2
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

static int listCoordsystemCompareRankDescending(
    const void *item1,
    const void *item2)
{
    EnsPCoordsystem cs1 = *(EnsOCoordsystem *const *) item1;
    EnsPCoordsystem cs2 = *(EnsOCoordsystem *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listCoordsystemCompareRankDescending"))
        ajDebug("listCoordsystemCompareRankDescending\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                cs1,
                cs2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (cs1 && (!cs2))
        return -1;

    if ((!cs1) && (!cs2))
        return 0;

    if ((!cs1) && cs2)
        return +1;

    if (cs1->Rank > cs2->Rank)
        return -1;

    if (cs1->Rank < cs2->Rank)
        return +1;

    return 0;
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Coordsystem Functions for manipulating AJAX List objects of
** Ensembl Coordinate System objects
** @nam4rule Sort Sort functions
** @nam5rule Identifier Sort by Ensembl Coordinate System identifier member
** @nam5rule Name Sort by Ensembl Coordinate System name member
** @nam5rule Rank Sort by Ensembl Coordinate System rank member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
** @nam4rule Trace Trace Ensembl Coordinate System objects
**
** @argrule Sort css [AjPList]
** AJAX List of Ensembl Coordinate System objects
** @argrule Trace css [const AjPList]
** AJAX List of Ensembl Coordinate System objects
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListCoordsystemSortIdentifierAscending ****************************
**
** Sort an AJAX List of Ensembl Coordinate System objects by their
** Ensembl Coordinate System identifier in ascending order.
**
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate System objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListCoordsystemSortIdentifierAscending(AjPList css)
{
    if (!css)
        return ajFalse;

    ajListSort(css, &listCoordsystemCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListCoordsystemSortIdentifierDescending ***************************
**
** Sort an AJAX List of Ensembl Coordinate System objects by their
** Ensembl Coordinate System identifier in descending order.
**
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate System objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListCoordsystemSortIdentifierDescending(AjPList css)
{
    if (!css)
        return ajFalse;

    ajListSort(css, &listCoordsystemCompareIdentifierDescending);

    return ajTrue;
}




/* @func ensListCoordsystemSortRankAscending **********************************
**
** Sort an AJAX List of Ensembl Coordinate System objects by their
** Ensembl Coordinate System rank in ascending order.
**
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate System objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListCoordsystemSortRankAscending(AjPList css)
{
    if (!css)
        return ajFalse;

    ajListSort(css, &listCoordsystemCompareRankAscending);

    return ajTrue;
}




/* @func ensListCoordsystemSortRankDescending *********************************
**
** Sort an AJAX List of Ensembl Coordinate System objects by their
** Ensembl Coordinate System rank in descending order.
**
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate System objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListCoordsystemSortRankDescending(AjPList css)
{
    if (!css)
        return ajFalse;

    ajListSort(css, &listCoordsystemCompareRankDescending);

    return ajTrue;
}




/* @func ensListCoordsystemTrace **********************************************
**
** Trace an Ensembl Coordinate System mapping path.
**
** @param [r] css [const AjPList] AJAX List of
** Ensembl Coordinate System objects
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListCoordsystemTrace(const AjPList css, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPCoordsystem cs = NULL;

    if (!css)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensListCoordsystemTrace %p\n"
            "%S  length %u\n",
            indent, css,
            indent, ajListGetLength(css));

    iter = ajListIterNewread(css);

    while (!ajListIterDone(iter))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iter);

        if (cs)
            ensCoordsystemTrace(cs, level + 1);
        else
            ajDebug("%S  <nul>\n", indent);
    }

    ajListIterDel(&iter);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor ****
**
** @nam2rule Coordsystemadaptor Functions for manipulating
** Ensembl Coordinate System Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor
** @cc CVS Revision: 1.29
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/





/* @funcstatic coordsystemadaptorFetchAllbyStatement **************************
**
** Run a SQL statement against an Ensembl Coordinate System Adaptor and
** consolidate the results into an AJAX List of Ensembl Coordinate System
** objects.
**
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @param [u] csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool coordsystemadaptorFetchAllbyStatement(
    EnsPCoordsystemadaptor csa,
    const AjPStr statement,
    AjPList css)
{
    ajuint identifier = 0U;
    ajuint rank       = 0U;

    AjBool dflt     = AJFALSE;
    AjBool seqlevel = AJFALSE;
    AjBool toplevel = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr version   = NULL;
    AjPStr attribute = NULL;
    AjPStr value     = NULL;

    AjPStrTok attrtoken = NULL;

    EnsPCoordsystem cs = NULL;

    if (!csa)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!css)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(csa->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name       = ajStrNew();
        version    = ajStrNew();
        rank       = 0;
        attribute  = ajStrNew();

        value = ajStrNew();

        dflt     = ajFalse;
        toplevel = ajFalse;
        seqlevel = ajFalse;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &version);
        ajSqlcolumnToUint(sqlr, &rank);
        ajSqlcolumnToStr(sqlr, &attribute);

        attrtoken = ajStrTokenNewC(attribute, ",");

        while (ajStrTokenNextParse(&attrtoken, &value))
        {
            if (ajStrMatchCaseC(value, "default_version"))
                dflt = ajTrue;

            if (ajStrMatchCaseC(value, "sequence_level"))
                seqlevel = ajTrue;
        }

        ajStrTokenDel(&attrtoken);

        cs = ensCoordsystemNewIni(csa,
                                  identifier,
                                  name,
                                  version,
                                  rank,
                                  dflt,
                                  toplevel,
                                  seqlevel);

        ajListPushAppend(css, (void *) cs);

        ajStrDel(&name);
        ajStrDel(&version);
        ajStrDel(&attribute);
        ajStrDel(&value);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(csa->Adaptor, &sqls);

    return ajTrue;
}




/* @funcstatic coordsystemadaptorCacheInit ************************************
**
** Initialise the internal Coordinate System cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool coordsystemadaptorCacheInit(EnsPCoordsystemadaptor csa)
{
    ajuint *Pidentifier = NULL;
    ajuint *Prank       = NULL;

    AjPList css = NULL;

    AjPStr statement = NULL;

    AjPTable versions = NULL;

    EnsPCoordsystem cs     = NULL;
    EnsPCoordsystem cstemp = NULL;

    if (!csa)
        return ajFalse;

    if (!csa->CacheByIdentifier)
    {
        csa->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(csa->CacheByIdentifier,
                               (void (*)(void **)) &ensCoordsystemDel);
    }

    if (!csa->CacheByName)
    {
        csa->CacheByName = ajTablestrNew(0);

        ajTableSetDestroyvalue(csa->CacheByName,
                               (void (*)(void **)) &ajTableDel);
    }

    if (!csa->CacheByRank)
    {
        csa->CacheByRank = ajTableuintNew(0);

        ajTableSetDestroyvalue(csa->CacheByRank,
                               (void (*)(void **)) &ensCoordsystemDel);
    }

    if (!csa->CacheByDefault)
    {
        csa->CacheByDefault = ajTableuintNew(0);

        ajTableSetDestroyvalue(csa->CacheByDefault,
                               (void (*)(void **)) &ensCoordsystemDel);
    }

    statement = ajFmtStr(
        "SELECT "
        "coord_system.coord_system_id, "
        "coord_system.name, "
        "coord_system.version, "
        "coord_system.rank, "
        "coord_system.attrib "
        "FROM "
        "coord_system "
        "WHERE "
        "coord_system.species_id = %u",
        ensDatabaseadaptorGetIdentifier(csa->Adaptor));

    css = ajListNew();

    coordsystemadaptorFetchAllbyStatement(csa, statement, css);

    while (ajListPop(css, (void **) &cs))
    {
        /* Sequence-level cache */

        if (cs->SequenceLevel)
            csa->Seqlevel = ensCoordsystemNewRef(cs);

        /* Identifier cache */

        AJNEW0(Pidentifier);

        *Pidentifier = cs->Identifier;

        cstemp = (EnsPCoordsystem)
            ajTablePut(csa->CacheByIdentifier,
                       (void *) Pidentifier,
                       (void *) ensCoordsystemNewRef(cs));

        if (cstemp)
        {
            ajWarn("coordsystemadaptorCacheInit got more than one "
                   "Ensembl Coordinate System with (PRIMARY KEY) identifier "
                   "%u.\n",
                   cstemp->Identifier);

            ensCoordsystemDel(&cstemp);
        }

        /* Name and Version cache */

        /*
        ** For each Coordinate System of a particular name one or more
        ** versions are supported. Thus, Ensembl Coordinate System objects
        ** are cached in two levels of AJAX Table objects.
        **
        ** First-level (Name) AJAX Table: An AJAX Table storing
        ** (Ensembl Coordinate System name) AJAX String objects as key data
        ** and second-level AJAX Table objects as value data.
        **
        ** Second-level (Version) AJAX Table: An AJAX Table storing
        ** (Ensembl Coordinate System version) AJAX String objects as key data
        ** and Ensembl Coordinate System objects as value data.
        */

        versions = (AjPTable) ajTableFetchmodS(csa->CacheByName, cs->Name);

        if (!versions)
        {
            /*
            ** Create a new AJAX Table of AJAX String (version) key and
            ** Ensembl Coordinate System value data.
            */

            versions = ajTablestrNew(0);

            ajTableSetDestroyvalue(versions,
                                   (void (*)(void **)) &ensCoordsystemDel);

            ajTablePut(csa->CacheByName,
                       (void *) ajStrNewS(cs->Name),
                       (void *) versions);
        }

        cstemp = (EnsPCoordsystem)
            ajTablePut(versions,
                       (void *) ajStrNewS(cs->Version),
                       (void *) ensCoordsystemNewRef(cs));

        if (cstemp)
        {
            ajWarn("coordsystemadaptorCacheInit got more than one "
                   "Ensembl Coordinate System with (UNIQUE) name '%S' and "
                   "version '%S' with identifiers %u and %u.\n",
                   cstemp->Name,
                   cstemp->Version,
                   cstemp->Identifier,
                   cs->Identifier);

            ensCoordsystemDel(&cstemp);
        }

        /* Cache by Rank */

        AJNEW0(Prank);

        *Prank = cs->Rank;

        cstemp = (EnsPCoordsystem)
            ajTablePut(csa->CacheByRank,
                       (void *) Prank,
                       (void *) ensCoordsystemNewRef(cs));

        if (cstemp)
        {
            ajWarn("coordsystemadaptorCacheInit got more than one "
                   "Ensembl Coordinate System with (UNIQUE) rank %u and "
                   "identifiers %u and %u.\n",
                   cstemp->Rank,
                   cstemp->Identifier,
                   cs->Identifier);

            ensCoordsystemDel(&cstemp);
        }

        /* Defaults cache */

        /*
        ** Ensembl supports one or more default Coordinate System objects.
        ** Ensembl Coordinate System objects are stored in an AJAX Table
        ** with (Ensembl Coordinate System identifier) AJAX unsigned integer
        ** key data and Ensembl Coordinate System value data.
        */

        if (cs->Default)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = cs->Identifier;

            cstemp = (EnsPCoordsystem)
                ajTablePut(csa->CacheByDefault,
                           (void *) Pidentifier,
                           (void *) ensCoordsystemNewRef(cs));

            if (cstemp)
            {
                ajWarn("coordsystemadaptorCacheInit got more than one "
                       "Ensembl Coordinate System with (PRIMARY KEY) "
                       "identifier %u.\n",
                       cstemp->Identifier);

                ensCoordsystemDel(&cstemp);
            }
        }

        /*
        ** All caches keep internal references to the
        ** Ensembl Coordinate System objects.
        */

        ensCoordsystemDel(&cs);
    }

    ajListFree(&css);

    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic coordsystemadaptorMappingpathInit ******************************
**
** Initialise the internal Coordinate System mapping path cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool coordsystemadaptorMappingpathInit(EnsPCoordsystemadaptor csa)
{
    AjBool debug = AJFALSE;

    AjIList iter    = NULL;
    AjPList cskeys  = NULL;
    AjPList css     = NULL;
    AjPList mis     = NULL;
    AjPList mappath = NULL;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr cskey     = NULL;
    AjPStr cs1key    = NULL;
    AjPStr cs2key    = NULL;
    AjPStr mapkey    = NULL;
    AjPStr metakey   = NULL;
    AjPStr metaval   = NULL;

    AjPStrTok cstoken   = NULL;
    AjPStrTok pathtoken = NULL;

    EnsPCoordsystem cs  = NULL;
    EnsPCoordsystem cs1 = NULL;
    EnsPCoordsystem cs2 = NULL;

    EnsPMetainformation mi         = NULL;
    EnsPMetainformationadaptor mia = NULL;

    debug = ajDebugTest("coordsystemadaptorMappingpathInit");

    if (debug)
        ajDebug("coordsystemadaptorMappingpathInit\n"
                "  csa %p\n",
                csa);

    if (!csa)
        return ajFalse;

    if (!csa->MappingPaths)
    {
        csa->MappingPaths = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            csa->MappingPaths,
            (void (*)(void **)) &coordsystemadaptorMappingpathValdel);
    }

    cskeys = ajListNew();

    /* Read 'assembly.mapping' keys from the Ensembl Core 'meta' table. */

    mia = ensRegistryGetMetainformationadaptor(csa->Adaptor);

    metakey = ajStrNewC("assembly.mapping");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(mia, metakey, mis);

    while (ajListPop(mis, (void **) &mi))
    {
        metaval = ensMetainformationGetValue(mi);

        if (debug)
            ajDebug("coordsystemadaptorMappingpathInit processing "
                    "Ensembl Meta Information value '%S'.\n",
                    metaval);

        /*
        ** Split 'assembly.mapping' Meta Information values on '#' or '|'
        ** characters into Coordinate System name:version keys.
        */

        pathtoken = ajStrTokenNewC(metaval, "#|");

        cskey = ajStrNew();

        while (ajStrTokenNextParse(&pathtoken, &cskey))
            ajListPushAppend(cskeys, (void *) ajStrNewS(cskey));

        ajStrDel(&cskey);

        ajStrTokenDel(&pathtoken);

        if (ajListGetLength(cskeys) < 2)
            ajWarn("coordsystemadaptorMappingpathInit got the incorrectly "
                   "formatted 'assembly.mapping' value '%S' from the "
                   "Ensembl Core 'meta' table.",
                   metaval);
        else
        {
            /*
            ** Split Coordinate System keys into names and versions and
            ** fetch the corresponding Coordinate Systems from the database.
            */

            css = ajListNew();

            iter = ajListIterNew(cskeys);

            while (!ajListIterDone(iter))
            {
                cskey = (AjPStr) ajListIterGet(iter);

                cstoken = ajStrTokenNewC(cskey, ":");

                csname    = ajStrNew();
                csversion = ajStrNew();

                ajStrTokenNextParse(&cstoken, &csname);
                ajStrTokenNextParse(&cstoken, &csversion);

                ensCoordsystemadaptorFetchByName(csa,
                                                 csname,
                                                 csversion,
                                                 &cs);

                if (cs)
                    ajListPushAppend(css, (void *) cs);
                else
                    ajWarn("coordsystemadaptorMappingpathInit could not load "
                           "an Ensembl Coordinate System for name '%S' and "
                           "version '%S', as specified in the "
                           "Ensembl Core 'meta' table by '%S'.",
                           csname, csversion, metaval);

                ajStrDel(&csname);
                ajStrDel(&csversion);

                ajStrTokenDel(&cstoken);
            }

            ajListIterDel(&iter);

            /*
            ** Test the 'assembly.mapping' Meta-Information value for
            ** '#' characters. A '#' delimiter indicates a special case,
            ** where multiple parts of a 'component' region map to the
            ** same part of an 'assembled' region. As this looks like the
            ** 'long' mapping, we just make the mapping path a bit longer.
            */

            if ((ajStrFindC(metaval, "#") >= 0) && (ajListGetLength(css) == 2))
            {
                /*
                ** Insert an empty middle node into the mapping path
                ** i.e. the AJAX List of Ensembl Coordinate Systems.
                */

                iter = ajListIterNew(css);

                (void) ajListIterGet(iter);

                ajListIterInsert(iter, NULL);

                ajListIterDel(&iter);

                if (debug)
                    ajDebug("coordsystemadaptorMappingpathInit "
                            "elongated mapping path '%S'.\n",
                            metaval);
            }

            /*
            ** Take the first and last Coordinate Systems from the AJAX List
            ** and generate name:version Coordinate System keys, before
            ** a name1:version1|name2:version2 map key.
            */

            ajListPeekFirst(css, (void **) &cs1);
            ajListPeekLast(css, (void **) &cs2);

            if (debug)
            {
                ajDebug("coordsystemadaptorMappingpathInit cs1 %p\n", cs1);

                ensCoordsystemTrace(cs1, 1);

                ajDebug("coordsystemadaptorMappingpathInit cs2 %p\n", cs2);

                ensCoordsystemTrace(cs2, 1);
            }

            if (cs1 && cs2)
            {
                cs1key = ajFmtStr("%S:%S", cs1->Name, cs1->Version);
                cs2key = ajFmtStr("%S:%S", cs2->Name, cs2->Version);
                mapkey = ajFmtStr("%S|%S", cs1key, cs2key);

                if (debug)
                    ajDebug("coordsystemadaptorMappingpathInit mapkey '%S'\n",
                            mapkey);

                /* Does a mapping path already exist? */

                mappath = (AjPList) ajTableFetchmodS(csa->MappingPaths,
                                                     mapkey);

                if (mappath)
                {
                    /* A similar map path exists already. */

                    ajDebug("coordsystemadaptorMappingpathInit got multiple "
                            "mapping paths between Ensembl Coordinate Systems "
                            "'%S' and '%S' and chooses the shorter mapping "
                            "path arbitrarily.\n",
                            cs1key, cs2key);

                    if (ajListGetLength(css) < ajListGetLength(mappath))
                    {
                        /*
                        ** The current map path is shorter than the stored map
                        ** path. Replace the stored List with the current List
                        ** and delete the (longer) stored List. The Table key
                        ** String remains in place.
                        */

                        mappath = (AjPList) ajTablePut(
                            csa->MappingPaths,
                            (void *) ajStrNewS(mapkey),
                            (void *) css);

                        ajDebug("coordsystemadaptorMappingpathInit "
                                "deleted the longer, stored mapping path!\n");

                        while (ajListPop(mappath, (void **) &cs))
                            ensCoordsystemDel(&cs);

                        ajListFree(&mappath);
                    }
                    else
                    {
                        /*
                        ** The current mapping path is longer than the stored
                        ** mapping path. Delete this (longer) mapping path.
                        */

                        ajDebug("coordsystemadaptorMappingpathInit "
                                "deleted the longer, current mapping path!\n");

                        while (ajListPop(css, (void **) &cs))
                            ensCoordsystemDel(&cs);

                        ajListFree(&css);
                    }
                }
                else
                {
                    /* No similar mappath exists so store the new mappath. */

                    ajTablePut(csa->MappingPaths,
                               (void *) ajStrNewS(mapkey),
                               (void *) css);

                    if (debug)
                        ajDebug("coordsystemadaptorMappingpathInit "
                                "added a new mapping path '%S'.\n",
                                mapkey);
                }

                ajStrDel(&cs1key);
                ajStrDel(&cs2key);
                ajStrDel(&mapkey);
            }
            else
                ajWarn("coordsystemadaptorMappingpathInit requires that both, "
                       "first and last Ensembl Coordinate Systems of a "
                       "mapping path are defined. "
                       "See Ensembl Core 'meta.meta_value' '%S'.",
                       metaval);
        }

        /*
        ** Clear the AJAX List of Coordinate System keys, but keep the
        ** AJAX List between Ensembl Meta Information entries.
        */

        while (ajListPop(cskeys, (void **) &cskey))
            ajStrDel(&cskey);

        ensMetainformationDel(&mi);
    }

    ajListFree(&mis);
    ajListFree(&cskeys);

    ajStrDel(&metakey);

    return ajTrue;
}




/* @funcstatic coordsystemadaptorSeqregionMapInit *****************************
**
** Initialise the Ensembl Coordinate System Adaptor-internal
** Ensembl Sequence Region mapping cache. This allows mapping of internal to
** external Sequence Region identifiers and vice versa.
**
** @param [u] csa [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool coordsystemadaptorSeqregionMapInit(EnsPCoordsystemadaptor csa)
{
    ajuint internal = 0U;
    ajuint external = 0U;

    ajuint *Pinternal = NULL;
    ajuint *Pexternal = NULL;
    ajuint *Pprevious = NULL;

    AjBool debug = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr build = NULL;
    AjPStr statement = NULL;

    debug = ajDebugTest("coordsystemadaptorSeqregionMapInit");

    if (debug)
        ajDebug("coordsystemadaptorSeqregionMapInit\n"
                "  csa %p\n",
                csa);

    if (!csa)
        return ajFalse;

    if (csa->ExternalToInternal)
        return ajFalse;
    else
    {
        csa->ExternalToInternal = ajTableuintNew(0);

        ajTableSetDestroyvalue(csa->ExternalToInternal, &ajMemFree);
    }

    if (csa->InternalToExternal)
        return ajFalse;
    else
    {
        csa->InternalToExternal = ajTableuintNew(0);

        ajTableSetDestroyvalue(csa->InternalToExternal, &ajMemFree);
    }

    build = ajStrNew();

    ensDatabaseadaptorFetchSchemabuild(csa->Adaptor, &build);

    if (debug)
        ajDebug("coordsystemadaptorSeqregionMapInit got build '%S'.\n", build);

    /* Get the relations for the current database. */

    statement = ajFmtStr(
        "SELECT "
        "seq_region_mapping.internal_seq_region_id, "
        "seq_region_mapping.external_seq_region_id "
        "FROM "
        "mapping_set, "
        "seq_region_mapping, "
        "seq_region, "
        "coord_system "
        "WHERE "
        "mapping_set.schema_build = '%S' "
        "AND "
        "mapping_set.mapping_set_id = seq_region_mapping.mapping_set_id "
        "AND "
        "seq_region_mapping.internal_seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = coord_system.coord_system_id "
        "AND "
        "coord_system.species_id = %u",
        build,
        ensDatabaseadaptorGetIdentifier(csa->Adaptor));

    ajStrDel(&build);

    sqls = ensDatabaseadaptorSqlstatementNew(csa->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        internal = 0U;
        external = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &internal);
        ajSqlcolumnToUint(sqlr, &external);

        /* Internal to external identifier mapping. */

        Pprevious = (ajuint *) ajTableFetchmodV(csa->InternalToExternal,
                                                (const void *) &internal);

        if (Pprevious)
            ajDebug("coordsystemadaptorSeqregionMapInit got duplicate "
                    "internal Sequence Region identifier:\n"
                    "%u -> %u\n"
                    "%u -> %u\n",
                    internal, *Pprevious,
                    internal, external);
        else
        {
            AJNEW0(Pinternal);
            AJNEW0(Pexternal);

            *Pinternal = internal;
            *Pexternal = external;

            ajTablePut(csa->InternalToExternal,
                       (void *) Pinternal,
                       (void *) Pexternal);
        }

        /* External to internal identifier mapping. */

        Pprevious = (ajuint *) ajTableFetchmodV(csa->ExternalToInternal,
                                                (const void *) &external);

        if (Pprevious)
            ajDebug("coordsystemadaptorSeqregionMapInit got duplicate "
                    "external Sequence Region identifier:\n"
                    "%u -> %u\n"
                    "%u -> %u\n",
                    external, *Pprevious,
                    external, internal);
        else
        {
            AJNEW0(Pinternal);
            AJNEW0(Pexternal);

            *Pinternal = internal;
            *Pexternal = external;

            ajTablePut(csa->ExternalToInternal,
                       (void *) Pexternal,
                       (void *) Pinternal);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(csa->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Coordinate System Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Coordinate System Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPCoordsystemadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensCoordsystemadaptorNew *********************************************
**
** Default constructor for an Ensembl Coordinate System Adaptor.
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
** @see ensRegistryGetCoordsystemadaptor
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPCoordsystemadaptor ensCoordsystemadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPCoordsystemadaptor csa = NULL;

    if (!dba)
        return NULL;

    AJNEW0(csa);

    csa->Adaptor = dba;

    coordsystemadaptorCacheInit(csa);

    coordsystemadaptorMappingpathInit(csa);

    coordsystemadaptorSeqregionMapInit(csa);

    /*
    ** Create a Pseudo-Coordinate System 'toplevel' and cache it so that only
    ** one of these is created for each database.
    */

    csa->Toplevel =
        ensCoordsystemNewIni(csa, 0, NULL, NULL, 0, ajFalse, ajTrue, ajFalse);

    return csa;
}




/* @funcstatic coordsystemadaptorMappingpathValdel ****************************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl Coordinate System objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void coordsystemadaptorMappingpathValdel(void **Pvalue)
{
    EnsPCoordsystem cs = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &cs))
        ensCoordsystemDel(&cs);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Coordinate System Adaptor object.
**
** @fdata [EnsPCoordsystemadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Coordinate System Adaptor
**
** @argrule * Pcsa [EnsPCoordsystemadaptor*]
** Ensembl Coordinate System Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensCoordsystemadaptorDel *********************************************
**
** Default destructor for an Ensembl Coordinate System Adaptor.
**
** This function also clears the internal Ensembl Coordinate System and
** mapping path caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pcsa [EnsPCoordsystemadaptor*]
** Ensembl Coordinate System Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensCoordsystemadaptorDel(EnsPCoordsystemadaptor *Pcsa)
{
    EnsPCoordsystemadaptor pthis = NULL;

    if (!Pcsa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensCoordsystemadaptorDel"))
        ajDebug("ensCoordsystemadaptorDel\n"
                "  *Pcsa %p\n",
                *Pcsa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pcsa)
        return;

    pthis = *Pcsa;

    ajTableDel(&pthis->CacheByIdentifier);
    ajTableDel(&pthis->CacheByName);
    ajTableDel(&pthis->CacheByRank);
    ajTableDel(&pthis->CacheByDefault);
    ajTableDel(&pthis->MappingPaths);
    ajTableDel(&pthis->ExternalToInternal);
    ajTableDel(&pthis->InternalToExternal);

    ensCoordsystemDel(&pthis->Seqlevel);
    ensCoordsystemDel(&pthis->Toplevel);

    AJFREE(pthis);

    *Pcsa = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Coordinate System Adaptor
** object.
**
** @fdata [EnsPCoordsystemadaptor]
**
** @nam3rule Get Return Coordinate System Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * csa [const EnsPCoordsystemadaptor] Coordinate System Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensCoordsystemadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Coordinate System Adaptor.
**
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensCoordsystemadaptorGetDatabaseadaptor(
    const EnsPCoordsystemadaptor csa)
{
    return (csa) ? csa->Adaptor : NULL;
}




/* @section object fetching ***************************************************
**
** Functions for fetching Ensembl Coordinate System objects from an
** Ensembl SQL database.
**
** @fdata [EnsPCoordsystemadaptor]
**
** @nam3rule Fetch Fetch Ensembl Coordinate System object(s)
** @nam4rule All   Fetch all Ensembl Coordinate System objects
** @nam4rule Allby Fetch all Ensembl Coordinate System objects
**                 matching a criterion
** @nam5rule Name  Fetch all by a name
** @nam4rule By    Fetch one Ensembl Coordinate System object
**                 matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
** @nam5rule Name  Fetch by a name
** @nam5rule Rank  Fetch by a rank
** @nam4rule Seqlevel Fetch the sequence-level Ensembl Coordinate System
** @nam4rule Toplevel Fetch the top-level Ensembl Coordinate System
**
** @argrule * csa [const EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor
** @argrule All css [AjPList] AJAX List of Ensembl Coordinate System objects
** @argrule AllbyName name [const AjPStr] Name
** @argrule Allby css [AjPList] AJAX List of Ensembl Coordinate System objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pcs [EnsPCoordsystem*] Ensembl Coordinate System
** address
** @argrule ByName name [const AjPStr] Name
** @argrule ByName version [const AjPStr] Version
** @argrule ByName Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
** @argrule ByRank rank [ajuint] Rank
** @argrule ByRank Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
** @argrule Seqlevel Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
** @argrule Toplevel Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic coordsystemadaptorFetchAll *************************************
**
** An ajTableMap "apply" function to return all Ensembl Coordinate Systems from
** an Ensembl Coordinate System Adaptor-internal cache.
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] Pvalue [void**] Ensembl Coordinate System value data address
** @param [u] cl [void*] AJAX List of Ensembl Coordinate System objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void coordsystemadaptorFetchAll(const void *key,
                                       void **Pvalue,
                                       void *cl)
{
    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    ajListPushAppend((AjPList) cl, (void *)
                     ensCoordsystemNewRef(*((EnsPCoordsystem *) Pvalue)));

    return;
}




/* @func ensCoordsystemadaptorFetchAll ****************************************
**
** Fetch all Ensembl Coordinate Systems in the order of ascending rank.
**
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_all
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchAll(const EnsPCoordsystemadaptor csa,
                                     AjPList css)
{
    if (!csa)
        return ajFalse;

    if (!css)
        return ajFalse;

    ajTableMap(csa->CacheByIdentifier,
               &coordsystemadaptorFetchAll,
               (void *) css);

    ensListCoordsystemSortRankAscending(css);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchAllbyName **********************************
**
** Fetch Ensembl Coordinate Systems of all versions for a name.
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_all_by_name
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [r] name [const AjPStr] Name
** @param [u] css [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchAllbyName(
    const EnsPCoordsystemadaptor csa,
    const AjPStr name,
    AjPList css)
{
    AjPTable versions = NULL;

    EnsPCoordsystem cs = NULL;

    if (!csa)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!css)
        return ajFalse;

    if (ajStrMatchCaseC(name, "seqlevel"))
    {
        ensCoordsystemadaptorFetchSeqlevel(csa, &cs);

        ajListPushAppend(css, (void *) cs);

        return ajTrue;
    }

    if (ajStrMatchCaseC(name, "toplevel"))
    {
        ensCoordsystemadaptorFetchToplevel(csa, &cs);

        ajListPushAppend(css, (void *) cs);

        return ajTrue;
    }

    versions = (AjPTable) ajTableFetchmodS(csa->CacheByName, name);

    if (versions)
        ajTableMap(versions, &coordsystemadaptorFetchAll, (void *) css);

    ensListCoordsystemSortRankAscending(css);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Coordinate System by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_by_dbID
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchByIdentifier(
    const EnsPCoordsystemadaptor csa,
    ajuint identifier,
    EnsPCoordsystem *Pcs)
{
    if (!csa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pcs)
        return ajFalse;

    *Pcs = (EnsPCoordsystem) ajTableFetchmodV(csa->CacheByIdentifier,
                                              (const void *) &identifier);

    ensCoordsystemNewRef(*Pcs);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchByName *************************************
**
** Fetch an Ensembl Coordinate System by name and version.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_by_name
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] version [const AjPStr] Version
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchByName(
    const EnsPCoordsystemadaptor csa,
    const AjPStr name,
    const AjPStr version,
    EnsPCoordsystem *Pcs)
{
    void **valarray = NULL;

    register ajuint i = 0U;

    AjPTable versions = NULL;

    EnsPCoordsystem cs = NULL;

    if (ajDebugTest("ensCoordsystemadaptorFetchByName"))
        ajDebug("ensCoordsystemadaptorFetchByName\n"
                "  csa %p\n"
                "  name '%S'\n"
                "  version '%S'\n"
                "  Pcs %p\n",
                csa,
                name,
                version,
                Pcs);

    if (!csa)
        return ajFalse;

    if (!name)
        return ajFalse;

    if (!Pcs)
        return ajFalse;

    *Pcs = (EnsPCoordsystem) NULL;

    if (ajStrMatchCaseC(name, "seqlevel"))
        return ensCoordsystemadaptorFetchSeqlevel(csa, Pcs);

    if (ajStrMatchCaseC(name, "toplevel"))
        return ensCoordsystemadaptorFetchToplevel(csa, Pcs);

    versions = (AjPTable) ajTableFetchmodS(csa->CacheByName, name);

    if (versions)
    {
        if (version && ajStrGetLen(version))
            *Pcs = (EnsPCoordsystem) ajTableFetchmodS(versions, version);
        else
        {
            /*
            ** If no version has been specified search for the default
            ** Ensembl Coordinate System of this name.
            */

            ajTableToarrayValues(versions, &valarray);

            for (i = 0U; valarray[i]; i++)
            {
                cs = (EnsPCoordsystem) valarray[i];

                if (cs->Default)
                {
                    *Pcs = cs;

                    break;
                }
            }

            AJFREE(valarray);
        }

        ensCoordsystemNewRef(*Pcs);

        return ajTrue;
    }

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchByRank *************************************
**
** Fetch an Ensembl Coordinate System by its rank.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_by_rank
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [r] rank [ajuint] Rank
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchByRank(
    const EnsPCoordsystemadaptor csa,
    ajuint rank,
    EnsPCoordsystem *Pcs)
{
    if (!csa)
        return ajFalse;

    if (!rank)
        return ensCoordsystemadaptorFetchToplevel(csa, Pcs);

    if (!Pcs)
        return ajFalse;

    *Pcs = (EnsPCoordsystem) ajTableFetchmodV(csa->CacheByRank,
                                              (const void *) &rank);

    ensCoordsystemNewRef(*Pcs);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchSeqlevel ***********************************
**
** Fetch the sequence-level Ensembl Coordinate System.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_sequence_level
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchSeqlevel(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem *Pcs)
{
    if (!csa)
        return ajFalse;

    if (!Pcs)
        return ajFalse;

    *Pcs = ensCoordsystemNewRef(csa->Seqlevel);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchToplevel ***********************************
**
** Fetch the top-level Ensembl Coordinate System.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::fetch_top_level
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchToplevel(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem *Pcs)
{
    if (!csa)
        return ajFalse;

    if (!Pcs)
        return ajFalse;

    *Pcs = ensCoordsystemNewRef(csa->Toplevel);

    return ajTrue;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Coordinate System Adaptor
** object.
**
** @fdata [EnsPCoordsystemadaptor]
**
** @nam3rule Get Return Coordinate System Adaptor attribute(s)
** @nam4rule Mappingpath Return the Ensembl Coordinate System mapping path
** @nam4rule Seqregionidentifier Return an Ensembl Sequence Region identifier
** @nam5rule External Return the external identifier
** @nam5rule Internal Return the internal identifier
**
** @argrule * csa [const EnsPCoordsystemadaptor] Coordinate System Adaptor
** @argrule Mappingpath cs1 [EnsPCoordsystem] First Ensembl Coordinate System
** @argrule Mappingpath cs2 [EnsPCoordsystem] Second Ensembl Coordinate System
** @argrule External srid [ajuint] Internal Ensembl Sequence Region identifier
** @argrule Internal srid [ajuint] External Ensembl Sequence Region identifier
**
** @valrule Mappingpath [const AjPList] AJAX List of
** Ensembl Coordinate System objects or NULL
** @valrule External [ajuint] External Ensembl Sequence Region identifier or 0U
** @valrule Internal [ajuint] Internal Ensembl Sequence Region identifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensCoordsystemadaptorGetMappingpath **********************************
**
** Fetch a mapping path between two Ensembl Coordinate Systems.
**
** @cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor::get_mapping_path
** @param [r] csa [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                               System Adaptor
** @param [u] cs1 [EnsPCoordsystem] First Ensembl Coordinate System
** @param [u] cs2 [EnsPCoordsystem] Second Ensembl Coordinate System
**
** @return [const AjPList] AJAX List of Ensembl Coordinate Systems or NULL
**
** @release 6.4.0
** @@
** Given two Coordinate Systems this will return a mapping path between them
** if one has been defined. Allowed Mapping paths are explicitly defined in the
** 'meta' table. The following is an example:
**
** mysql> select * from meta where meta_key = 'assembly.mapping';
**
**   +---------+------------------+--------------------------------------+
**   | meta_id | meta_key         | meta_value                           |
**   +---------+------------------+--------------------------------------+
**   |      20 | assembly.mapping | chromosome:NCBI34|contig             |
**   |      21 | assembly.mapping | clone|contig                         |
**   |      22 | assembly.mapping | supercontig|contig                   |
**   |      23 | assembly.mapping | chromosome:NCBI34|contig|clone       |
**   |      24 | assembly.mapping | chromosome:NCBI34|contig|supercontig |
**   |      25 | assembly.mapping | supercontig|contig|clone             |
**   +---------+------------------+--------------------------------------+
**
** For a one-step mapping path to be valid there needs to be a relationship
** between the two Coordinate Systems defined in the assembly table. Two step
** mapping paths work by building on the one-step mapping paths which are
** already defined.
**
** The first coordinate system in a one step mapping path must be the assembled
** coordinate system and the second must be the component.
**
** A '#' delimiter indicates a special case where multiple parts of a
** 'component' region map to the same part of an 'assembled' region.
** In those cases, the mapping path is elongated by an empty middle node.
******************************************************************************/

const AjPList ensCoordsystemadaptorGetMappingpath(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    ajint match = 0;

    AjBool debug = AJFALSE;

    AjPList mappath = NULL;
    AjPList midpath = NULL;
    AjPList tmppath = NULL;

    AjPStr cs1key = NULL;
    AjPStr cs2key = NULL;
    AjPStr midkey = NULL;
    AjPStr mapkey = NULL;

    AjPStr cs1str = NULL;
    AjPStr cs2str = NULL;
    AjPStr midstr = NULL;

    AjPTable midcs1 = NULL;
    AjPTable midcs2 = NULL;

    EnsPCoordsystem csp0 = NULL;
    EnsPCoordsystem csp1 = NULL;
    EnsPCoordsystem midcs = NULL;

    debug = ajDebugTest("ensCoordsystemadaptorGetMappingpath");

    if (debug)
    {
        ajDebug("ensCoordsystemadaptorGetMappingpath\n"
                "  csa %p\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                csa,
                cs1,
                cs2);

        ensCoordsystemTrace(cs1, 1);
        ensCoordsystemTrace(cs2, 1);
    }

    if (!csa)
        return NULL;

    if (!cs1)
        return NULL;

    if (!cs2)
        return NULL;

    cs1key = ajFmtStr("%S:%S", cs1->Name, cs1->Version);
    cs2key = ajFmtStr("%S:%S", cs2->Name, cs2->Version);

    /* Lookup path for cs1key|cs2key. */

    mapkey = ajFmtStr("%S|%S", cs1key, cs2key);

    mappath = (AjPList) ajTableFetchmodV(csa->MappingPaths,
                                         (const void *) mapkey);

    ajStrDel(&mapkey);

    if (mappath && ajListGetLength(mappath))
    {
        ajStrDel(&cs1key);
        ajStrDel(&cs2key);

        return mappath;
    }

    /* Lookup path for cs2key|cs1key. */

    mapkey = ajFmtStr("%S|%S", cs2key, cs1key);

    mappath = (AjPList) ajTableFetchmodV(csa->MappingPaths,
                                         (const void *) mapkey);

    ajStrDel(&mapkey);

    if (mappath && ajListGetLength(mappath))
    {
        ajStrDel(&cs1key);
        ajStrDel(&cs2key);

        return mappath;
    }

    /*
    ** Still no success. Search for a chained mapping path involving two
    ** coordinate system pairs with a shared middle coordinate system.
    */

    if (debug)
        ajDebug("ensCoordsystemadaptorGetMappingpath got no explicit mapping "
                "path between Ensembl Coordinate Systems '%S' and '%S'.\n",
                cs1key, cs2key);

    /*
    ** Initialise temporary AJAX Table objects of AJAX String key and
    ** AJAX List of Ensembl Coordinate System objects value data.
    */

    midcs1 = ajTablestrNew(0);
    midcs2 = ajTablestrNew(0);

    ajTableSetDestroyvalue(
        midcs1,
        (void (*)(void **)) &coordsystemadaptorMappingpathValdel);
    ajTableSetDestroyvalue(
        midcs2,
        (void (*)(void **)) &coordsystemadaptorMappingpathValdel);

    /*
    ** Iterate over all mapping paths stored in the
    ** Coordinate System Adaptor.
    */

    ajTableToarrayKeysValues(csa->MappingPaths, &keyarray, &valarray);

    for (i = 0U; keyarray[i]; i++)
    {
        tmppath = (AjPList) valarray[i];

        /* Consider only paths of two components. */

        if (ajListGetLength(tmppath) != 2)
            continue;

        ajListPeekNumber(tmppath, 0, (void **) &csp0);
        ajListPeekNumber(tmppath, 1, (void **) &csp1);

        match = -1;

        if (ensCoordsystemMatch(csp0, cs1))
            match = 1;

        if (ensCoordsystemMatch(csp1, cs1))
            match = 0;

        if (match >= 0)
        {
            ajListPeekNumber(tmppath, (ajuint) match, (void **) &midcs);

            midkey = ajFmtStr("%S:%S", midcs->Name, midcs->Version);

            if (ajTableMatchS(midcs2, midkey))
            {
                mapkey = ajFmtStr("%S|%S", cs1key, cs2key);

                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs1));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs2));

                ajTablePut(csa->MappingPaths,
                           (void *) mapkey,
                           (void *) midpath);

                mappath = midpath;

                /*
                ** Inform the user that an explicit assembly.mapping entry is
                ** missing from the Ensembl Core meta table.
                */

                cs1str = (ajStrGetLen(cs1->Version)) ?
                    ajFmtStr("%S:%S", cs1->Name, cs1->Version) :
                    ajStrNewS(cs1->Name);

                midstr = (ajStrGetLen(midcs->Version)) ?
                    ajFmtStr("%S:%S", midcs->Name, midcs->Version) :
                    ajStrNewS(midcs->Name);

                cs2str = (ajStrGetLen(cs2->Version)) ?
                    ajFmtStr("%S:%S", cs2->Name, cs2->Version) :
                    ajStrNewS(cs2->Name);

                ajWarn("ensCoordsystemadaptorGetMappingpath uses an implicit"
                       "mapping path between Ensembl Coordinate Systems "
                       "'%S' and '%S'.\n"
                       "An explicit 'assembly.mapping' entry should be "
                       "added to the Ensembl Core 'meta' table.\n"
                       "Example: '%S|%S|%S'\n",
                       cs1str, cs2str,
                       cs1str, midstr, cs2str);

                ajStrDel(&cs1str);
                ajStrDel(&cs2str);
                ajStrDel(&midstr);
                ajStrDel(&midkey);

                break;
            }
            else
            {
                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajTablePut(midcs1,
                           (void *) ajStrNewS(midkey),
                           (void *) midpath);
            }

            ajStrDel(&midkey);
        }

        match = -1;

        if (ensCoordsystemMatch(csp0, cs2))
            match = 1;

        if (ensCoordsystemMatch(csp1, cs2))
            match = 0;

        if (match >= 0)
        {
            ajListPeekNumber(tmppath, (ajuint) match, (void **) &midcs);

            midkey = ajFmtStr("%S:%S", midcs->Name, midcs->Version);

            if (ajTableMatchS(midcs1, midkey))
            {
                mapkey = ajFmtStr("%S|%S", cs2key, cs1key);

                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs2));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs1));

                ajTablePut(csa->MappingPaths,
                           (void *) mapkey,
                           (void *) midpath);

                mappath = midpath;

                /*
                ** Inform the user that an explicit assembly.mapping entry is
                ** missing from the Ensembl Core meta table.
                */

                cs1str = (ajStrGetLen(cs1->Version)) ?
                    ajFmtStr("%S:%S", cs1->Name, cs1->Version) :
                    ajStrNewS(cs1->Name);

                midstr = (ajStrGetLen(midcs->Version)) ?
                    ajFmtStr("%S:%S", midcs->Name, midcs->Version) :
                    ajStrNewS(midcs->Name);

                cs2str = (ajStrGetLen(cs2->Version)) ?
                    ajFmtStr("%S:%S", cs2->Name, cs2->Version) :
                    ajStrNewS(cs2->Name);

                ajWarn("ensCoordsystemadaptorGetMappingpath uses an implicit "
                       "mapping path between Ensembl Coordinate Systems "
                       "'%S' and '%S'.\n"
                       "An explicit 'assembly.mapping' entry should be "
                       "added to the Ensembl Core 'meta' table.\n"
                       "Example: '%S|%S|%S'\n",
                       cs1str, cs2str,
                       cs1str, midstr, cs2str);

                ajStrDel(&cs1str);
                ajStrDel(&cs2str);
                ajStrDel(&midstr);
                ajStrDel(&midkey);

                break;
            }
            else
            {
                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajTablePut(midcs2,
                           (void *) ajStrNewS(midkey),
                           (void *) midpath);
            }

            ajStrDel(&midkey);
        }
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableDel(&midcs1);
    ajTableDel(&midcs2);

    ajStrDel(&cs1key);
    ajStrDel(&cs2key);

    return mappath;
}




/* @func ensCoordsystemadaptorGetSeqregionidentifierExternal ******************
**
** Get the external Ensembl Sequence Region identifier for an
** internal Ensembl Sequence Region identifier.
** If the external Ensembl Sequence Region is not present in the mapping table,
** the identifier for the internal one will be returned.
**
** @param [r] csa [const EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor
** @param [r] srid [ajuint] Internal Ensembl Sequence Region identifier
**
** @return [ajuint] External Ensembl Sequence Region identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensCoordsystemadaptorGetSeqregionidentifierExternal(
    const EnsPCoordsystemadaptor csa,
    ajuint srid)
{
    ajuint *Pidentifier = NULL;

    if (!csa)
        return 0U;

    if (!srid)
        return 0U;

    Pidentifier = (ajuint *) ajTableFetchmodV(csa->InternalToExternal,
                                              (const void *) &srid);

    return (Pidentifier) ? *Pidentifier : srid;
}




/* @func ensCoordsystemadaptorGetSeqregionidentifierInternal ******************
**
** Get the internal Ensembl Sequence Region identifier for an
** external Ensembl Sequence Region identifier.
** If the internal Ensembl Sequence Region is not present in the mapping table,
** the identifier for the external one will be returned.
**
** @param [r] csa [const EnsPCoordsystemadaptor]
** Ensembl Coordinate System Adaptor
** @param [r] srid [ajuint] External Ensembl Sequence Region identifier
**
** @return [ajuint] Internal Ensembl Sequence Region identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensCoordsystemadaptorGetSeqregionidentifierInternal(
    const EnsPCoordsystemadaptor csa,
    ajuint srid)
{
    ajuint *Pidentifier = NULL;

    if (!csa)
        return 0U;

    if (!srid)
        return 0U;

    Pidentifier = (ajuint *) ajTableFetchmodV(csa->ExternalToInternal,
                                              (const void *) &srid);

    return (Pidentifier) ? *Pidentifier : srid;
}

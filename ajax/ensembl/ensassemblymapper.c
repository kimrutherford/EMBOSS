/* @source Ensembl Assembly Mapper functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.34 $
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

#include "ensassemblymapper.h"
#include "ensseqregion.h"
#include "ensslice.h"
#include "enstable.h"




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

/* @conststatic assemblymapperMaximum *****************************************
**
** The maximum number of Ensembl Mapper Pair objects an Ensembl Assembly Mapper
** is allowed to register. 2 ^ 11 (2 ki) corresponds to about 2 x 10 ^ 3
** objects.
**
******************************************************************************/

static const ajuint assemblymapperMaximum = 1 << 11;




/* @conststatic assemblymapperChunkFactor *************************************
**
** The Assembly Mapper registers Sequence Region objects in chunks of
** 2 ^ 20 (1 Mi), which corresponds to approximately 1 x 10 ^ 6 or 1 M
** base pairs.
**
******************************************************************************/

static const ajuint assemblymapperChunkFactor = 20;




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void genericassemblymapperClear(void** key, void** value, void* cl);

static AjBool assemblymapperadaptorHasMultipleMappings(
    const EnsPAssemblymapperadaptor ama,
    ajuint srid);

static AjBool assemblymapperadaptorMultipleMappingsInit(
    EnsPAssemblymapperadaptor ama);

static AjBool assemblymapperadaptorMappingPath(const AjPList mappath);

static AjBool assemblymapperadaptorBuildCombinedMapper(
    EnsPAssemblymapperadaptor ama,
    AjPList ranges,
    EnsPMapper srcmidmapper,
    EnsPMapper trgmidmapper,
    EnsPMapper srctrgmapper,
    const AjPStr srctype);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensassemblymapper *********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper ***
**
** @nam2rule Genericassemblymapper Functions for manipulating
** Ensembl Generic Assembly Mapper objects
**
** @cc Bio::EnsEMBL::AssemblyMapper
** @cc CVS Revision: 1.46
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Generic Assembly Mapper by pointer.
** It is the responsibility of the user to first destroy any previous
** Generic Assembly Mapper. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gam [const EnsPGenericassemblymapper] Ensembl Generic Assembly
** Mapper
** @argrule Ini ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @argrule Ini asmcs [EnsPCoordsystem] Assembled Ensembl Coordinate System
** @argrule Ini cmpcs [EnsPCoordsystem] Component Ensembl Coordinate System
** @argrule Ref gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @valrule * [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGenericassemblymapperNewIni ***************************************
**
** Constructor for an Ensembl Generic Assembly Mapper with initial values.
**
** @cc Bio::EnsEMBL::AssemblyMapper::new
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] asmcs [EnsPCoordsystem] Assembled Ensembl Coordinate System
** @param [u] cmpcs [EnsPCoordsystem] Component Ensembl Coordinate System
**
** @return [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPGenericassemblymapper ensGenericassemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem asmcs,
    EnsPCoordsystem cmpcs)
{
    AjPStr srcname = NULL;
    AjPStr trgname = NULL;

    EnsPGenericassemblymapper gam = NULL;

    if(ajDebugTest("ensGenericassemblymapperNewIni"))
    {
        ajDebug("ensGenericassemblymapperNewIni\n"
                "  ama %p\n"
                "  asmcs %p\n"
                "  cmpcs %p\n",
                ama,
                asmcs,
                cmpcs);

        ensCoordsystemTrace(asmcs, 1);
        ensCoordsystemTrace(cmpcs, 1);
    }

    if(!ama)
        return NULL;

    if(!asmcs)
        return NULL;

    if(!cmpcs)
        return NULL;

    srcname = ajStrNewC("assembled");
    trgname = ajStrNewC("component");

    AJNEW0(gam);

    gam->Adaptor              = ama;
    gam->CoordsystemAssembled = ensCoordsystemNewRef(asmcs);
    gam->CoordsystemComponent = ensCoordsystemNewRef(cmpcs);
    gam->RegisterAssembled    = ensTableuintNewLen(0);
    gam->RegisterComponent    = ensTableuintNewLen(0);
    gam->Mapper               = ensMapperNewIni(srcname,
                                                trgname,
                                                asmcs,
                                                cmpcs);
    gam->Maximum              = assemblymapperMaximum;
    gam->Use                  = 1;

    ajStrDel(&srcname);
    ajStrDel(&trgname);

    return gam;
}




/* @func ensGenericassemblymapperNewRef ***************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @return [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPGenericassemblymapper ensGenericassemblymapperNewRef(
    EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    gam->Use++;

    return gam;
}




/* @section clear *************************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Generic Assembly Mapper.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Clear Clear internals of an Ensembl Generic Assembly Mapper object
**
** @argrule * gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic genericassemblymapperClear *************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data (Ensembl Sequence Region identifier) and
** AJAX Table value data. This function also clears and deletes the
** second-level AJAX Table objects of AJAX unsigned integer key data
** (Ensembl Sequence Region chunk identifier) and
** AJAX Boolean value data.
**
** @param [u] key [void**] Void address
** @param [u] value [void**] Void address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void genericassemblymapperClear(void** key, void** value, void* cl)
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

    AJFREE(*key);

    ensTableDelete((AjPTable*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensGenericassemblymapperClear ****************************************
**
** Clear an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::flush
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperClear(EnsPGenericassemblymapper gam)
{
    if(ajDebugTest("ensGenericassemblymapperClear"))
        ajDebug("ensGenericassemblymapperClear\n"
                "  gam %p\n",
                gam);

    if(!gam)
        return ajFalse;

    ajTableMapDel(gam->RegisterAssembled, genericassemblymapperClear, NULL);

    ensTableClear(gam->RegisterComponent);

    ensMapperClear(gam->Mapper);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Generic Assembly Mapper object.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Del Destroy (free) an Ensembl Generic Assembly Mapper object
**
** @argrule * Pgam [EnsPGenericassemblymapper*] Ensembl Generic Assembly Mapper
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGenericassemblymapperDel ******************************************
**
** Default destructor for an Ensembl Generic Assembly Mapper.
**
** @param [d] Pgam [EnsPGenericassemblymapper*] Ensembl Generic Assembly Mapper
**                                              object address
**
** @return [void]
** @@
******************************************************************************/

void ensGenericassemblymapperDel(EnsPGenericassemblymapper* Pgam)
{
    EnsPGenericassemblymapper pthis = NULL;

    if(!Pgam)
        return;

    if(!*Pgam)
        return;

    if(ajDebugTest("ensGenericassemblymapperDel"))
        ajDebug("ensGenericassemblymapperDel\n"
                "  *Pgam %p\n",
                *Pgam);

    pthis = *Pgam;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgam = NULL;

        return;
    }

    ensGenericassemblymapperClear(pthis);

    ensCoordsystemDel(&pthis->CoordsystemAssembled);
    ensCoordsystemDel(&pthis->CoordsystemComponent);

    ajTableFree(&pthis->RegisterAssembled);
    ajTableFree(&pthis->RegisterComponent);

    ensMapperDel(&pthis->Mapper);

    AJFREE(pthis);

    *Pgam = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Generic Assembly Mapper object.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Get Return Generic Assembly Mapper attribute(s)
** @nam4rule Adaptor Return the Ensembl Assembly Mapper Adaptor
** @nam4rule Coordsystem Return an Ensembl Coordinate System
** @nam5rule Assembled Return the assembled Ensembl Coordinate System
** @nam5rule Component Return the component Ensembl Coordinate System
** @nam4rule Mapper Return the Ensembl Mapper
** @nam4rule Maximum Return the maximum number of Ensembl Mapper Pair objects
**
** @argrule * gam [const EnsPGenericassemblymapper] Generic Assembly Mapper
**
** @valrule Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @valrule Assembled [const EnsPCoordsystem] Ensembl Coordinate System
** @valrule Component [const EnsPCoordsystem] Ensembl Coordinate System
** @valrule Mapper [EnsPMapper] Ensembl Mapper
** @valrule Maximum [ajuint] Maximum number of Ensembl Mapper Pair objects or 0
**
** @fcategory use
******************************************************************************/




/* @func ensGenericassemblymapperGetAdaptor ***********************************
**
** Get the Ensembl Assembly Mapper Adaptor element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::adaptor
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
** Assembly Mapper
**
** @return [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblymapperadaptor ensGenericassemblymapperGetAdaptor(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    return gam->Adaptor;
}




/* @func ensGenericassemblymapperGetCoordsystemAssembled **********************
**
** Get the assembled Ensembl Coordinate System element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::assembled_CoordSystem
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [const EnsPCoordsystem] Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensGenericassemblymapperGetCoordsystemAssembled(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    return gam->CoordsystemAssembled;
}




/* @func ensGenericassemblymapperGetCoordsystemComponent **********************
**
** Get the component Ensembl Coordinate System element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::component_CoordSystem
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [const EnsPCoordsystem] Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensGenericassemblymapperGetCoordsystemComponent(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    return gam->CoordsystemComponent;
}




/* @func ensGenericassemblymapperGetMapper ************************************
**
** Get the Ensembl Mapper element of an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::mapper
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [EnsPMapper] Ensembl Mapper or NULL
** @@
******************************************************************************/

EnsPMapper ensGenericassemblymapperGetMapper(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    return gam->Mapper;
}




/* @func ensGenericassemblymapperGetMaximum ***********************************
**
** Get the maximum number of Ensembl Mapper Pair objects element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::max_pair_count
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [ajuint] Maximum number of Ensembl Mapper Pair objects or 0
** @@
******************************************************************************/

ajuint ensGenericassemblymapperGetMaximum(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return 0;

    return gam->Maximum;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Generic Assembly Mapper object.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Set Set one element of a Generic Assembly Mapper
** @nam4rule Adaptor Set the Ensembl Assembly Mapper Adaptor
** @nam4rule Maximum Set the maximum number of Ensembl Mapper Pair objects
**
** @argrule * gam [EnsPGenericassemblymapper] Ensembl Generic Assembly
**                                            Mapper object
** @argrule Adaptor ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper
** Adaptor
** @argrule Maximum maximum [ajuint] Maximum number of Ensembl Mapper Pair
** objects
**
** @valrule * [AjBool]
**
** @fcategory modify
******************************************************************************/




/* @func ensGenericassemblymapperSetAdaptor ***********************************
**
** Set the Adaptor element of an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperSetAdaptor(EnsPGenericassemblymapper gam,
                                          EnsPAssemblymapperadaptor ama)
{
    if(!gam)
        return ajFalse;

    if(!ama)
        return ajFalse;

    gam->Adaptor = ama;

    return ajTrue;
}




/* @func ensGenericassemblymapperSetMaximum ***********************************
**
** Set the maximum number of Ensembl Mapper Pair objects element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::max_pair_count
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] maximum [ajuint] Maximum number of Ensembl Mapper Pair objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This sets the number of Ensembl Mapper Pair objects allowed in the internal
** cache. This can be used to override the default value assemblymapperMaximum
** to tune the performance and memory usage for certain scenarios.
** A higher value means a bigger cache and thus higher memory requirements.
******************************************************************************/

AjBool ensGenericassemblymapperSetMaximum(EnsPGenericassemblymapper gam,
                                          ajuint maximum)
{
    if(!gam)
        return ajFalse;

    gam->Maximum = maximum;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Generic Assembly Mapper object.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Trace Report Ensembl Generic Assembly Mapper elements to
** debug file.
**
** @argrule Trace gam [const EnsPGenericassemblymapper] Ensembl Generic
** Assembly Mapper
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGenericassemblymapperTrace ****************************************
**
** Trace an Ensembl Generic Assembly Mapper.
**
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic Assembly
** Mapper
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperTrace(const EnsPGenericassemblymapper gam,
                                     ajuint level)
{
    AjPStr indent = NULL;

    if(!gam)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGenericassemblymapperTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  CoordsystemAssembled %p\n"
            "%S  CoordsystemComponent %p\n"
            "%S  RegisterAssembled %p\n"
            "%S  RegisterComponent %p\n"
            "%S  Mapper %p\n"
            "%S  Maximum %u\n"
            "%S  Use %u\n",
            indent, gam,
            indent, gam->Adaptor,
            indent, gam->CoordsystemAssembled,
            indent, gam->CoordsystemComponent,
            indent, gam->RegisterAssembled,
            indent, gam->RegisterComponent,
            indent, gam->Mapper,
            indent, gam->Maximum,
            indent, gam->Use);

    ensCoordsystemTrace(gam->CoordsystemAssembled, level + 1);
    ensCoordsystemTrace(gam->CoordsystemComponent, level + 1);

    ensMapperTrace(gam->Mapper, level +1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Generic Assembly Mapper convenience functions
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Count Get the number of Ensembl Mapper Pair objects in the
** associated Ensembl Mapper
**
** @argrule * gam [const EnsPGenericassemblymapper] Ensembl Generic Assembly
** Mapper
**
** @valrule Count [ajuint] Number of Ensembl Mapper Pair objects or 0
**
** @fcategory use
******************************************************************************/




/* @func ensGenericassemblymapperGetCount *************************************
**
** Get the number of Ensembl Mapper Pair objects stored in the Ensembl Mapper
** that is associated with an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::size
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [ajuint] Number of Ensembl Mapper Pair objects or 0
** @@
******************************************************************************/

ajuint ensGenericassemblymapperGetCount(const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return 0;

    return ensMapperGetCount(gam->Mapper);
}




/* @section register **********************************************************
**
** Register Ensembl Sequence Region identifiers in an
** Ensembl Generic Assembly Mapper.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Register  Register Ensembl Sequence Region identifiers
** @nam4rule All       Register all Ensembl Sequence Region identifiers
** @nam4rule Assembled Register an assembled Ensembl Sequence Region identifier
** @nam4rule Component Register a component Ensembl Sequence Rgion identifier
**
** @argrule * gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @argrule Assembled asmsrid [ajuint] Assembled Ensembl Sequence Region
** identifier
** @argrule Assembled chunkid [ajint] Chunk identifier
** @argrule Component cmpsrid [ajuint] Component Ensembl Sequence Region
** identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGenericassemblymapperRegisterAll **********************************
**
** Register all mappings between two Ensembl Coordinate Systems in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::register_all
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperRegisterAll(EnsPGenericassemblymapper gam)
{
    if(!gam)
        return ajFalse;

    return ensAssemblymapperadaptorRegisterGenericAll(gam->Adaptor, gam);
}




/* @func ensGenericassemblymapperRegisterAssembled ****************************
**
** Register an assembled Ensembl Sequence Region identifier in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::register_assembled
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [rE] chunkid [ajint] Chunk identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperRegisterAssembled(EnsPGenericassemblymapper gam,
                                                 ajuint asmsrid,
                                                 ajint chunkid)
{
    ajint* Pchunkid = NULL;

    ajuint* Pasmsrid = NULL;

    AjBool* Pflag  = NULL;

    AjPTable table = NULL;

    if(ajDebugTest("ensGenericassemblymapperRegisterAssembled"))
        ajDebug("ensGenericassemblymapperRegisterAssembled\n"
                "  gam %p\n"
                "  asmsrid %u\n"
                "  chunkid %d\n",
                gam,
                asmsrid,
                chunkid);

    if(!gam)
        return ajFalse;

    if(!asmsrid)
        return ajFalse;

    table = (AjPTable) ajTableFetchmodV(gam->RegisterAssembled,
                                        (const void*) &asmsrid);

    if(!table)
    {
        AJNEW0(Pasmsrid);

        *Pasmsrid = asmsrid;

        table = ensTableuintNewLen(0);

        ajTablePut(gam->RegisterAssembled, (void*) Pasmsrid, (void*) table);
    }

    if(ajTableFetchmodV(table, (const void*) &chunkid))
        return ajTrue;
    else
    {
        AJNEW0(Pchunkid);

        *Pchunkid = chunkid;

        AJNEW0(Pflag);

        *Pflag = ajTrue;

        ajTablePut(table, (void*) Pchunkid, (void*) Pflag);
    }

    return ajTrue;
}




/* @func ensGenericassemblymapperRegisterComponent ****************************
**
** Register a component Ensembl Sequence Region identifier in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::register_component
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] cmpsrid [ajuint] Component Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperRegisterComponent(EnsPGenericassemblymapper gam,
                                                 ajuint cmpsrid)
{
    ajuint* Pcmpsrid = NULL;

    AjBool* Pold = NULL;
    AjBool* Pflag = NULL;

    if(ajDebugTest("ensGenericassemblymapperRegisterComponent"))
        ajDebug("ensGenericassemblymapperRegisterComponent\n"
                "  gam %p\n"
                "  cmpsrid %d\n",
                gam,
                cmpsrid);

    if(!gam)
        return ajFalse;

    if(!cmpsrid)
        return ajFalse;

    Pold = (AjBool*) ajTableFetchmodV(gam->RegisterComponent,
                                      (const void*) &cmpsrid);

    if(Pold)
        return ajTrue;
    else
    {
        AJNEW0(Pcmpsrid);

        *Pcmpsrid = cmpsrid;

        AJNEW0(Pflag);

        *Pflag = ajTrue;

        ajTablePut(gam->RegisterComponent, (void*) Pcmpsrid, (void*) Pflag);
    }

    return ajTrue;
}




/* @section check *************************************************************
**
** Check, whether an Ensembl Sequence Region identifier has been registered.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Check Check an Ensembl Sequence Region identifier
** @nam4rule Assembled Check an assembled Ensembl Sequence Region identifier
** @nam4rule Component Check a component Ensembl Sequence Rgion identifier
**
** @argrule * gam [const EnsPGenericassemblymapper] Ensembl Generic Assembly
** Mapper
** @argrule Assembled asmsrid [ajuint] Assembled Ensembl Sequence Region
** identifier
** @argrule Assembled chunkid [ajint] Chunk identifier
** @argrule Component cmpsrid [ajuint] Component Ensembl Sequence Region
** identifier
**
** @valrule * [AjBool] ajTrue if already registered, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGenericassemblymapperCheckAssembled *******************************
**
** Check whether an assembled Ensembl Sequence Region identifier has been
** registered in an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::have_registered_assembled
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [rE] chunkid [ajint] Chunk identifier
**
** @return [AjBool] ajTrue if already registered, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperCheckAssembled(
    const EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajint chunkid)
{
    AjPTable table = NULL;

    if(ajDebugTest("ensGenericassemblymapperCheckAssembled"))
        ajDebug("ensGenericassemblymapperCheckAssembled\n"
                "  gam %p\n"
                "  asmsrid %u\n"
                "  chunkid %d\n",
                gam,
                asmsrid,
                chunkid);

    if(!gam)
        return ajFalse;

    if(!asmsrid)
        return ajFalse;

    /*
    ** Get the second-level AJAX Table keyed on chunk identifers from the
    ** first-level AJAX Table keyed on Ensembl Sequence Region identifiers.
    */

    table = (AjPTable) ajTableFetchmodV(gam->RegisterAssembled,
                                        (const void*) &asmsrid);

    if(table && ajTableFetchmodV(table, (const void*) &chunkid))
        return ajTrue;

    return ajFalse;
}




/* @func ensGenericassemblymapperCheckComponent *******************************
**
** Check whether a component Ensembl Sequence Region identifier has been
** registered in an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::have_registered_component
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
** @param [r] cmpsrid [ajuint] Component Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue if already registered, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperCheckComponent(
    const EnsPGenericassemblymapper gam,
    ajuint cmpsrid)
{
    if(ajDebugTest("ensGenericassemblymapperCheckComponent"))
        ajDebug("ensGenericassemblymapperCheckComponent\n"
                "  gam %p\n"
                "  cmpsrid %u\n",
                gam,
                cmpsrid);

    if(!gam)
        return ajFalse;

    if(!cmpsrid)
        return ajFalse;

    if(ajTableFetchmodV(gam->RegisterComponent, (const void*) &cmpsrid))
        return ajTrue;

    return ajFalse;
}




/* @section map ***************************************************************
**
** Map coordinates based on Ensembl Sequence Region identifiers between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPGenericassemblymapper]
**
** @nam3rule Fastmap Fast map coordinates
** @nam3rule Map Map coordinates
**
** @argrule * gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @argrule Fastmap sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Fastmap srstart [ajint] Sequence Region Start coordinate
** @argrule Fastmap srend [ajint] Sequence Region End coordinate
** @argrule Fastmap srstrand [ajint] Sequence Region Strand information
** @argrule Fastmap mrs [AjPList] AJAX List of Ensembl Mapper Result objects
** @argrule Map sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Map srstart [ajint] Sequence Region Start coordinate
** @argrule Map srend [ajint] Sequence Region End coordinate
** @argrule Map srstrand [ajint] Sequence Region Strand information
** @argrule Map mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGenericassemblymapperFastmap **************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Generic Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::AssemblyMapper::fastmap
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperFastmap(EnsPGenericassemblymapper gam,
                                       const EnsPSeqregion sr,
                                       ajint srstart,
                                       ajint srend,
                                       ajint srstrand,
                                       AjPList mrs)
{
    ajuint srid = 0;

    AjPStr type = NULL;

    if(ajDebugTest("ensGenericassemblymapperFastmap"))
    {
        ajDebug("ensGenericassemblymapperFastmap\n"
                "  gam %p\n"
                "  sr %p\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  mrs %p\n",
                gam,
                sr,
                srstart,
                srend,
                srstrand,
                mrs);

        ensSeqregionTrace(sr, 1);
    }

    if(!gam)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    srid = ensSeqregionGetIdentifier(sr);

    if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                           gam->CoordsystemComponent))
    {
        if(!ensGenericassemblymapperCheckComponent(gam, srid))
            ensAssemblymapperadaptorRegisterGenericComponent(gam->Adaptor,
                                                             gam,
                                                             srid);

        type = ajStrNewC("component");
    }
    else if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                                gam->CoordsystemAssembled))
    {
        ensAssemblymapperadaptorRegisterGenericAssembled(gam->Adaptor,
                                                         gam,
                                                         srid,
                                                         srstart,
                                                         srend);

        type = ajStrNewC("assembled");
    }
    else
        ajFatal("ensGenericassemblymapperFastmap got an "
                "Ensembl Sequence Region, which Coordinate System '%S:%S' "
                "is neiter the assembled nor the component Coordinate System "
                "of this Ensembl Generic Assembly Mapper.\n",
                ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
                ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)));

    ensMapperFastmap(gam->Mapper,
                     srid,
                     srstart,
                     srend,
                     srstrand,
                     type,
                     mrs);

    ajStrDel(&type);

    return ajTrue;
}




/* @func ensGenericassemblymapperMap ******************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Generic Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::AssemblyMapper::map
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperMap(EnsPGenericassemblymapper gam,
                                   const EnsPSeqregion sr,
                                   ajint srstart,
                                   ajint srend,
                                   ajint srstrand,
                                   AjPList mrs)
{
    ajuint srid = 0;

    AjPStr type = NULL;

    if(ajDebugTest("ensGenericassemblymapperMap"))
    {
        ajDebug("ensGenericassemblymapperMap\n"
                "  gam %p\n"
                "  sr %p\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  mrs %p\n",
                gam,
                sr,
                srstart,
                srend,
                srstrand,
                mrs);

        ensGenericassemblymapperTrace(gam, 1);

        ensSeqregionTrace(sr, 1);
    }

    if(!gam)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    srid = ensSeqregionGetIdentifier(sr);

    if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                           gam->CoordsystemComponent))
    {
        if(!ensGenericassemblymapperCheckComponent(gam, srid))
            ensAssemblymapperadaptorRegisterGenericComponent(gam->Adaptor,
                                                             gam,
                                                             srid);

        type = ajStrNewC("component");
    }
    else if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                                gam->CoordsystemAssembled))
    {
        ensAssemblymapperadaptorRegisterGenericAssembled(gam->Adaptor,
                                                         gam,
                                                         srid,
                                                         srstart,
                                                         srend);

        type = ajStrNewC("assembled");
    }
    else
        ajFatal("ensGenericassemblymapperMap got an Ensembl Sequence Region, "
                "which Coordinate System '%S:%S' "
                "is neiter the assembled nor the component Coordinate System "
                "of this Ensembl Generic Assembly Mapper.",
                ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
                ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)));

    ensMapperMap(gam->Mapper,
                 srid,
                 srstart,
                 srend,
                 srstrand,
                 type,
                 mrs);

    ajStrDel(&type);

    return ajTrue;
}




/* @datasection [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper ***
**
** @nam2rule Chainedassemblymapper Functions for manipulating
** Ensembl Chained Assembly Mapper objects
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper
** @cc CVS Revision: 1.18
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Chained Assembly Mapper by pointer.
** It is the responsibility of the user to first destroy any previous
** Chained Assembly Mapper. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy cam [const EnsPChainedassemblymapper] Ensembl Chained Assembly
** Mapper
** @argrule Ini ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @argrule Ini srccs [EnsPCoordsystem] Source Ensembl Coordinate System
** @argrule Ini midcs [EnsPCoordsystem] Middle Ensembl Coordinate System
** @argrule Ini trgcs [EnsPCoordsystem] Target Ensembl Coordinate System
** @argrule Ref cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @valrule * [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensChainedassemblymapperNewIni ***************************************
**
** Constructor for an Ensembl Chained Assembly Mapper with initial values.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::new
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] srccs [EnsPCoordsystem] Source Ensembl Coordinate System
** @param [u] midcs [EnsPCoordsystem] Middle Ensembl Coordinate System
** @param [u] trgcs [EnsPCoordsystem] Target Ensembl Coordinate System
**
** @return [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPChainedassemblymapper ensChainedassemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem srccs,
    EnsPCoordsystem midcs,
    EnsPCoordsystem trgcs)
{
    AjBool debug = AJFALSE;

    AjPStr srctype = NULL;
    AjPStr midtype = NULL;
    AjPStr trgtype = NULL;

    EnsPChainedassemblymapper cam = NULL;

    debug = ajDebugTest("ensChainedassemblymapperNewIni");

    if(debug)
    {
        ajDebug("ensChainedassemblymapperNewIni\n"
                "  ama %p\n"
                "  srccs %p\n"
                "  midcs %p\n"
                "  trgcs %p\n",
                ama,
                srccs,
                midcs,
                trgcs);

        ensCoordsystemTrace(srccs, 1);
        ensCoordsystemTrace(midcs, 1);
        ensCoordsystemTrace(trgcs, 1);
    }

    if(!ama)
        return NULL;

    if(!srccs)
        return NULL;

    if(!midcs && debug)
        ajDebug("ensChainedassemblymapperNewIni got no middle "
                "Ensembl Coordinate System. "
                "Multiple parts of a component map to more than one "
                "assembled part.\n");

    if(!trgcs)
        return NULL;

    srctype = ajStrNewC("source");
    midtype = ajStrNewC("middle");
    trgtype = ajStrNewC("target");

    AJNEW0(cam);

    cam->Adaptor = ama;

    cam->CoordsystemSource  = ensCoordsystemNewRef(srccs);
    cam->CoordsystemMiddle  = ensCoordsystemNewRef(midcs);
    cam->CoordsystemTarget  = ensCoordsystemNewRef(trgcs);
    cam->MapperSourceMiddle = ensMapperNewIni(srctype, midtype, srccs, midcs);
    cam->MapperTargetMiddle = ensMapperNewIni(trgtype, midtype, trgcs, midcs);
    cam->MapperSourceTarget = ensMapperNewIni(srctype, trgtype, srccs, trgcs);
    cam->RegistrySource     = ensMapperrangeregistryNew();
    cam->RegistryTarget     = ensMapperrangeregistryNew();
    cam->Maximum            = assemblymapperMaximum;

    cam->Use = 1;

    ajStrDel(&srctype);
    ajStrDel(&midtype);
    ajStrDel(&trgtype);

    return cam;
}




/* @func ensChainedassemblymapperNewRef ***************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @return [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPChainedassemblymapper ensChainedassemblymapperNewRef(
    EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    cam->Use++;

    return cam;
}




/* @section clear *************************************************************
**
** Clear internals of an Ensembl Chained Assembly Mapper.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Clear Clear internals of an Ensembl Chained Assembly Mapper
**
** @argrule * cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory delete
******************************************************************************/




/* @func ensChainedassemblymapperClear ****************************************
**
** Clear all Ensembl Mapper objects and Ensembl Mapper Range Registry objects
** of an Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::flush
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensChainedassemblymapperClear(EnsPChainedassemblymapper cam)
{
    if(ajDebugTest("ensChainedassemblymapperClear"))
        ajDebug("ensChainedassemblymapperClear\n"
                "  cam %p\n",
                cam);

    if(!cam)
        return ajFalse;

    ensMapperClear(cam->MapperSourceMiddle);
    ensMapperClear(cam->MapperTargetMiddle);
    ensMapperClear(cam->MapperSourceTarget);

    ensMapperrangeregistryClear(cam->RegistrySource);
    ensMapperrangeregistryClear(cam->RegistryTarget);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Chained Assembly Mapper object.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Del Destroy (free) an Ensembl Chained Assembly Mapper object
**
** @argrule * Pcam [EnsPChainedassemblymapper*] Ensembl Chained Assembly Mapper
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensChainedassemblymapperDel ******************************************
**
** Default destructor for an Ensembl Chained Assembly Mapper.
**
** @param [d] Pcam [EnsPChainedassemblymapper*]
** Ensembl Chained Assembly Mapper object address
**
** @return [void]
** @@
******************************************************************************/

void ensChainedassemblymapperDel(EnsPChainedassemblymapper* Pcam)
{
    EnsPChainedassemblymapper pthis = NULL;

    if(!Pcam)
        return;

    if(!*Pcam)
        return;

    if(ajDebugTest("ensChainedassemblymapperDel"))
        ajDebug("ensChainedassemblymapperDel\n"
                "  *Pcam %p\n",
                *Pcam);

    pthis = *Pcam;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pcam = NULL;

        return;
    }

    ensCoordsystemDel(&pthis->CoordsystemSource);
    ensCoordsystemDel(&pthis->CoordsystemMiddle);
    ensCoordsystemDel(&pthis->CoordsystemTarget);

    ensMapperDel(&pthis->MapperSourceMiddle);
    ensMapperDel(&pthis->MapperTargetMiddle);
    ensMapperDel(&pthis->MapperSourceTarget);

    ensMapperrangeregistryDel(&pthis->RegistrySource);
    ensMapperrangeregistryDel(&pthis->RegistryTarget);

    AJFREE(pthis);

    *Pcam = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Chained Assembly Mapper object.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Get Return Chained Assembly Mapper attribute(s)
** @nam4rule Adaptor Return the Ensembl Assembly Mapper Adaptor
** @nam4rule Coordsystem Return an Ensembl Coordinate System
** @nam5rule Source Return the source Ensembl Coordinate System
** @nam5rule Middle Return the middle Ensembl Coordinate System
** @nam5rule Target Return the target Ensembl Coordinate System
** @nam4rule Mapper Return an Ensembl Mapper
** @nam5rule Source Return a source Ensembl Mapper
** @nam6rule Middle Return the source to middle Ensembl Mapper
** @nam6rule Target Return the source to target Ensembl Mapper
** @nam5rule Target Return a target Ensembl Mapper
** @nam6rule Middle Return the target to middle Ensembl Mapper
** @nam4rule Maximum Return the maximum Ensembl Mapper Pair count
** @nam4rule Registry Return an Ensembl Range Registry
** @nam5rule Source Return the source Ensembl Range Registry
** @nam5rule Target Return the target Ensembl Range Registry
**
** @argrule * cam [const EnsPChainedassemblymapper] Ensembl Chained Assembly
** Mapper
**
** @valrule Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @valrule CoordsystemSource [const EnsPCoordsystem] Ensembl Coordinate System
** @valrule CoordsystemMiddle [const EnsPCoordsystem] Ensembl Coordinate System
** @valrule CoordsystemTarget [const EnsPCoordsystem] Ensembl Coordinate System
** @valrule MapperSourceMiddle [EnsPMapper] Ensembl Mapper
** @valrule MapperSourceTarget [EnsPMapper] Ensembl Mapper
** @valrule MapperTargetMiddle [EnsPMapper] Ensembl Mapper
** @valrule Maximum [ajuint] Maximum number of Ensembl Mapper Pair objects or 0
** @valrule RegistrySource [EnsPMapperrangeregistry] Ensembl Range Registry
** @valrule RegistryTarget [EnsPMapperrangeregistry] Ensembl Range Registry
**
** @fcategory use
******************************************************************************/




/* @func ensChainedassemblymapperGetAdaptor ***********************************
**
** Get the Ensembl Assembly Mapper Adaptor element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::adaptor
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblymapperadaptor ensChainedassemblymapperGetAdaptor(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->Adaptor;
}




/* @func ensChainedassemblymapperGetCoordsystemMiddle *************************
**
** Get the middle Coordinate System element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::middle_CoordSystem
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [const EnsPCoordsystem] Middle Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensChainedassemblymapperGetCoordsystemMiddle(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->CoordsystemMiddle;
}




/* @func ensChainedassemblymapperGetCoordsystemSource *************************
**
** Get the source Coordinate System element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::first_CoordSystem
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [const EnsPCoordsystem] Source Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensChainedassemblymapperGetCoordsystemSource(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->CoordsystemSource;
}




/* @func ensChainedassemblymapperGetCoordsystemTarget *************************
**
** Get the target Coordinate System element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::last_CoordSystem
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [const EnsPCoordsystem] Target Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensChainedassemblymapperGetCoordsystemTarget(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->CoordsystemTarget;
}




/* @func ensChainedassemblymapperGetMapperSourceMiddle ************************
**
** Get the source to middle Ensembl Mapper element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::first_middle_mapper
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [EnsPMapper] Source to middle Ensembl Mapper or NULL
** @@
******************************************************************************/

EnsPMapper ensChainedassemblymapperGetMapperSourceMiddle(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->MapperSourceMiddle;
}




/* @func ensChainedassemblymapperGetMapperSourceTarget ************************
**
** Get the source to target Ensembl Mapper element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::first_last_mapper
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [EnsPMapper] Source to target Ensembl Mapper or NULL
** @@
******************************************************************************/

EnsPMapper ensChainedassemblymapperGetMapperSourceTarget(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->MapperSourceTarget;
}




/* @func ensChainedassemblymapperGetMapperTargetMiddle ************************
**
** Get the target to middle Ensembl Mapper element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::last_middle_mapper
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [EnsPMapper] Target to middle Ensembl Mapper or NULL
** @@
******************************************************************************/

EnsPMapper ensChainedassemblymapperGetMapperTargetMiddle(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->MapperTargetMiddle;
}




/* @func ensChainedassemblymapperGetMaximum ***********************************
**
** Get the maximum Ensembl Mapper Pair count element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::max_pair_count
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [ajuint] Maximum Ensembl Mapper Pair count or 0
** @@
******************************************************************************/

ajuint ensChainedassemblymapperGetMaximum(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return 0;

    return cam->Maximum;
}




/* @func ensChainedassemblymapperGetRegistrySource ****************************
**
** Get the source Ensembl Mapper Range Registry element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::first_registry
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [EnsPMapperrangeregistry] Source Ensembl Mapper Range Registry
**                                   or NULL.
** @@
******************************************************************************/

EnsPMapperrangeregistry ensChainedassemblymapperGetRegistrySource(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->RegistrySource;
}




/* @func ensChainedassemblymapperGetRegistryTarget ****************************
**
** Get the target Ensembl Mapper Range Registry element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::last_registry
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [EnsPMapperrangeregistry] Target Ensembl Mapper Range Registry
**                                   or NULL
** @@
******************************************************************************/

EnsPMapperrangeregistry ensChainedassemblymapperGetRegistryTarget(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->RegistryTarget;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Chained Assembly Mapper object.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Set Set one element of a Chained Assembly Mapper
** @nam4rule Adaptor Set the Ensembl Assembly Mapper
** @nam4rule Maximum Set the maximum Ensembl Mapper Pair count
**
** @argrule * cam [EnsPChainedassemblymapper] Ensembl Chained Assembly
**                                            Mapper object
** @argrule Adaptor ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper
** Adaptor
** @argrule Maximum maximum [ajuint] Maximum number of Ensembl Mapper Pair
** objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensChainedassemblymapperSetAdaptor ***********************************
**
** Set the Adaptor element of an Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensChainedassemblymapperSetAdaptor(EnsPChainedassemblymapper cam,
                                          EnsPAssemblymapperadaptor ama)
{
    if(!cam)
        return ajFalse;

    if(!ama)
        return ajFalse;

    cam->Adaptor = ama;

    return ajTrue;
}




/* @func ensChainedassemblymapperSetMaximum ***********************************
**
** Set the maximum number of Ensembl Mapper Pair objects element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::max_pair_count
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] maximum [ajuint] Maximum number of Ensembl Mapper Pair objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
** This sets the number of Ensembl Mapper Pair objects allowed in the internal
** cache. This can be used to override the default value assemblymapperMaximum
** to tune the performance and memory usage for certain scenarios.
** A higher value means a bigger cache and thus higher memory requirements.
******************************************************************************/

AjBool ensChainedassemblymapperSetMaximum(EnsPChainedassemblymapper cam,
                                          ajuint maximum)
{
    if(!cam)
        return ajFalse;

    if(!maximum)
        return ajFalse;

    cam->Maximum = maximum;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Chained Assembly Mapper object.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Trace Report Ensembl Chained Assembly Mapper elements to
** debug file.
**
** @argrule Trace cam [const EnsPChainedassemblymapper] Ensembl Chained
** Assembly Mapper
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensChainedassemblymapperTrace ****************************************
**
** Trace an Ensembl Chained Assembly Mapper.
**
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained Assembly
** Mapper
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensChainedassemblymapperTrace(const EnsPChainedassemblymapper cam,
                                     ajuint level)
{
    AjPStr indent = NULL;

    if(!cam)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensChainedassemblymapperTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  CoordsystemSource %p\n"
            "%S  CoordsystemMiddle %p\n"
            "%S  CoordsystemTarget %p\n"
            "%S  MapperSourceMiddle %p\n"
            "%S  MapperTargetMiddle %p\n"
            "%S  MapperSourceTarget %p\n"
            "%S  RegistrySource %p\n"
            "%S  RegistryTarget %p\n"
            "%S  Maximum %u\n"
            "%S  Use %u\n",
            indent, cam,
            indent, cam->Adaptor,
            indent, cam->CoordsystemSource,
            indent, cam->CoordsystemMiddle,
            indent, cam->CoordsystemTarget,
            indent, cam->MapperSourceMiddle,
            indent, cam->MapperTargetMiddle,
            indent, cam->MapperSourceTarget,
            indent, cam->RegistrySource,
            indent, cam->RegistryTarget,
            indent, cam->Maximum,
            indent, cam->Use);

    ensCoordsystemTrace(cam->CoordsystemSource, level + 1);
    ensCoordsystemTrace(cam->CoordsystemMiddle, level + 1);
    ensCoordsystemTrace(cam->CoordsystemTarget, level + 1);

    ensMapperTrace(cam->MapperSourceMiddle, level +1);
    ensMapperTrace(cam->MapperTargetMiddle, level +1);
    ensMapperTrace(cam->MapperSourceTarget, level +1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section convenience functions *********************************************
**
** Ensembl Chained Assembly Mapper convenience functions
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Count Get the number of Ensembl Mapper Pair objects in the
** associated Ensembl Mapper
**
** @argrule * cam [const EnsPChainedassemblymapper] Ensembl Chained Assembly
** Mapper
**
** @valrule Count [ajuint] Number of Ensembl Mapper Pair objects or 0
**
** @fcategory use
******************************************************************************/




/* @func ensChainedassemblymapperGetCount *************************************
**
** Get the number of Ensembl Mapper Pair objects in an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::size
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [ajuint] Number of Ensembl Mapper Pair objects or 0
** @@
******************************************************************************/

ajuint ensChainedassemblymapperGetCount(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return 0;

    return ensMapperGetCount(cam->MapperSourceMiddle) +
        ensMapperGetCount(cam->MapperTargetMiddle) +
        ensMapperGetCount(cam->MapperSourceTarget);
}




/* @section map ***************************************************************
**
** Map coordinates based on Ensembl Sequence Region identifiers between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Fastmap Fast map coordinates
** @nam3rule Map Map coordinates
**
** @argrule * cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @argrule Map sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Map srstart [ajint] Sequence Region Start coordinate
** @argrule Map srend [ajint] Sequence Region End coordinate
** @argrule Map srstrand [ajint] Sequence Region Strand information
** @argrule Map optsr [const EnsPSeqregion] Optional Ensembl Sequence Region
** @argrule Map fastmap [AjBool] Fast-mapping attribute
** @argrule Map mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensChainedassemblymapperMap ******************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Chained Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::map
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::fastmap
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [rE] optsr [const EnsPSeqregion] Optional Ensembl Sequence Region
** @param [r] fastmap [AjBool] Fast-mapping attribute
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensChainedassemblymapperMap(EnsPChainedassemblymapper cam,
                                   const EnsPSeqregion sr,
                                   ajint srstart,
                                   ajint srend,
                                   ajint srstrand,
                                   const EnsPSeqregion optsr,
                                   AjBool fastmap,
                                   AjPList mrs)
{
    ajint regstart = 0;
    ajint regend   = 0;

    ajuint srid    = 0;
    ajuint optsrid = 0;

    AjBool isinsert = AJFALSE;

    AjPList ranges = NULL;

    AjPStr type = NULL;

    EnsPMapperrange mr = NULL;
    EnsPMapperrangeregistry registry = NULL;

    if(ajDebugTest("ensChainedassemblymapperMap"))
    {
        ajDebug("ensChainedassemblymapperMap\n"
                "  cam %p\n"
                "  sr %p\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  optsr %p\n"
                "  fastmap '%B'\n"
                "  mrs %p\n",
                cam,
                sr,
                srstart,
                srend,
                srstrand,
                optsr,
                fastmap,
                mrs);

        ensSeqregionTrace(sr, 1);
        ensSeqregionTrace(optsr, 1);
    }

    if(!cam)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    isinsert = (srstart == (srend + 1));

    srid = ensSeqregionGetIdentifier(sr);

    optsrid = ensSeqregionGetIdentifier(optsr);

    if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                           cam->CoordsystemSource))
    {
        type = ajStrNewC("source");

        registry = cam->RegistrySource;
    }
    else if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                                cam->CoordsystemTarget))
    {
        type = ajStrNewC("target");

        registry = cam->RegistryTarget;
    }
    else
        ajFatal("ensChainedassemblymapperMap got an Ensembl Sequence Region, "
                "which Coordinate System '%S:%S' is "
                "neither the source nor the target Coordinate System of this "
                "Chained Assembly Mapper.",
                ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
                ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)));


    /*
    ** The minimum area we want to register if registration is necessary is
    ** about 1 Mb. Break requested ranges into chunks of 1 Mb and then
    ** register this larger region if we have a registry miss.
    **
    ** Use bitwise shift for fast and easy integer multiplication and
    ** division.
    */

    if(isinsert)
    {
        regstart = ((srend >> assemblymapperChunkFactor)
                    << assemblymapperChunkFactor);

        regend = (((srstart >> assemblymapperChunkFactor) + 1)
                  << assemblymapperChunkFactor) - 1 ;
    }
    else
    {
        regstart = ((srstart >> assemblymapperChunkFactor)
                    << assemblymapperChunkFactor);

        regend = (((srend >> assemblymapperChunkFactor) + 1)
                  << assemblymapperChunkFactor) - 1 ;
    }

    /*
    ** Get a list of Mapper Ranges in the requested region that have not been
    ** registered, and register them at the same time.
    */

    ranges = ajListNew();

    if(isinsert)
        ensMapperrangeregistryCheck(registry,
                                    srid,
                                    srend,
                                    srstart,
                                    regstart,
                                    regend,
                                    ranges);
    else
        ensMapperrangeregistryCheck(registry,
                                    srid,
                                    srstart,
                                    srend,
                                    regstart,
                                    regend,
                                    ranges);

    if(ajListGetLength(ranges))
    {
        if(ensChainedassemblymapperGetCount(cam) > cam->Maximum)
        {
            /*
            ** If the Chained Assembly Mapper contains already more
            ** Ensembl Mapper Pair objects than allowed, clear the internal
            ** cache and re-register the lot. The AJAX List of Ensembl Mapper
            ** Range objects needs clearing as well.
            */

            ensChainedassemblymapperClear(cam);

            while(ajListPop(ranges, (void**) &mr))
                ensMapperrangeDel(&mr);

            if(isinsert)
                ensMapperrangeregistryCheck(registry,
                                            srid,
                                            srend,
                                            srstart,
                                            regstart,
                                            regend,
                                            ranges);
            else
                ensMapperrangeregistryCheck(registry,
                                            srid,
                                            srstart,
                                            srend,
                                            regstart,
                                            regend,
                                            ranges);
        }

        ensAssemblymapperadaptorRegisterChainedRegion(cam->Adaptor,
                                                      cam,
                                                      type,
                                                      srid,
                                                      optsrid,
                                                      ranges);
    }

    if(fastmap)
        ensMapperFastmap(cam->MapperSourceTarget,
                         srid,
                         srstart,
                         srend,
                         srstrand,
                         type,
                         mrs);
    else
        ensMapperMap(cam->MapperSourceTarget,
                     srid,
                     srstart,
                     srend,
                     srstrand,
                     type,
                     mrs);

    /*
    ** Delete all Ensembl Mapper Ranges from the AJAX List before
    ** deleting the List.
    */

    while(ajListPop(ranges, (void**) &mr))
        ensMapperrangeDel(&mr);

    ajListFree(&ranges);

    ajStrDel(&type);

    return ajTrue;
}




/* @section register **********************************************************
**
** Register Ensembl Sequence Region identifiers in an
** Ensembl Chained Assembly Mapper.
**
** @fdata [EnsPChainedassemblymapper]
**
** @nam3rule Register Register Ensembl Sequence Region identifiers
** @nam4rule All       Register all Ensembl Sequence Region identifiers
**
** @argrule * cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensChainedassemblymapperRegisterAll **********************************
**
** Register all mappings between two Ensembl Coordinate Systems in an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::register_all
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensChainedassemblymapperRegisterAll(EnsPChainedassemblymapper cam)
{
    if(!cam)
        return ajFalse;

    return ensAssemblymapperadaptorRegisterChainedAll(cam->Adaptor, cam);
}




/* @datasection [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly Mapper
**
** @nam2rule Toplevelassemblymapper Functions for manipulating
** Ensembl Top-Level Assembly Mapper objects
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper
** @cc CVS Revision: 1.12
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Top-Level Assembly Mapper by pointer.
** It is the responsibility of the user to first destroy any previous
** Top-Level Assembly Mapper. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPToplevelassemblymapper]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy tlam [const EnsPToplevelassemblymapper] Ensembl Top-Level
** Assembly Mapper
** @argrule Ini ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @argrule Ini asmcs [EnsPCoordsystem] Top-level Ensembl Coordinate System
** @argrule Ini cmpcs [EnsPCoordsystem] Other Ensembl Coordinate System
** @argrule Ref tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
** Assembly Mapper
**
** @valrule * [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly Mapper
** or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensToplevelassemblymapperNewIni **************************************
**
** Constructor for an Ensembl Top-Level Assembly Mapper with initial values.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::new
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] asmcs [EnsPCoordsystem] Top-level Ensembl Coordinate System
** @param [u] cmpcs [EnsPCoordsystem] Other Ensembl Coordinate System
**
** @return [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly Mapper
**                                      or NULL
** @@
******************************************************************************/

EnsPToplevelassemblymapper ensToplevelassemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem asmcs,
    EnsPCoordsystem cmpcs)
{
    EnsPCoordsystemadaptor csa = NULL;

    EnsPToplevelassemblymapper tlam = NULL;

    if(!ama)
        return NULL;

    if(!asmcs)
        return NULL;

    if(!cmpcs)
        return NULL;

    if(!ensCoordsystemGetToplevel(asmcs))
        ajFatal("ensToplevelassemblymapperNewIni asmcs '%S:%S' should be a "
                "top-level Ensembl Coordinate System.",
                ensCoordsystemGetName(asmcs),
                ensCoordsystemGetVersion(asmcs));

    if(ensCoordsystemGetToplevel(cmpcs))
        ajFatal("ensToplevelassemblymapperNewIni cmpcs '%S:%S' should be no "
                "top-level Ensembl Coordinate System.",
                ensCoordsystemGetName(cmpcs),
                ensCoordsystemGetVersion(cmpcs));

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    AJNEW0(tlam);

    tlam->Adaptor = ama;

    tlam->Coordsystems = ajListNew();

    ensCoordsystemadaptorFetchAll(csa, tlam->Coordsystems);

    tlam->CoordsystemAssembled = ensCoordsystemNewRef(asmcs);
    tlam->CoordsystemComponent    = ensCoordsystemNewRef(cmpcs);

    tlam->Use = 1;

    return tlam;
}




/* @func ensToplevelassemblymapperNewRef **************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                              Assembly Mapper
**
** @return [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly Mapper
**                                      or NULL
** @@
******************************************************************************/

EnsPToplevelassemblymapper ensToplevelassemblymapperNewRef(
    EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    tlam->Use++;

    return tlam;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Top-Level Assembly Mapper object.
**
** @fdata [EnsPToplevelassemblymapper]
**
** @nam3rule Del Destroy (free) an Ensembl Top-Level Assembly Mapper object
**
** @argrule * Ptlam [EnsPToplevelassemblymapper*]
** Ensembl Top-Level Assembly Mapper object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensToplevelassemblymapperDel *****************************************
**
** Default destructor for an Ensembl Top-Level Assembly Mapper object.
**
** @param [d] Ptlam [EnsPToplevelassemblymapper*]
** Ensembl Top-Level Assembly Mapper object address
** @return [void]
** @@
******************************************************************************/

void ensToplevelassemblymapperDel(EnsPToplevelassemblymapper* Ptlam)
{
    EnsPCoordsystem cs = NULL;
    EnsPToplevelassemblymapper pthis = NULL;

    if(!Ptlam)
        return;

    if(!*Ptlam)
        return;

    pthis = *Ptlam;

    pthis->Use--;

    if(pthis->Use)
    {
        *Ptlam = NULL;

        return;
    }

    ensCoordsystemDel(&pthis->CoordsystemAssembled);
    ensCoordsystemDel(&pthis->CoordsystemComponent);

    /* Clear and delete the AJAX List of Ensembl Coordinate System objects. */

    while(ajListPop(pthis->Coordsystems, (void**) &cs))
        ensCoordsystemDel(&cs);

    ajListFree(&pthis->Coordsystems);

    AJFREE(pthis);

    *Ptlam = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Top-Level Assembly Mapper object.
**
** @fdata [EnsPToplevelassemblymapper]
**
** @nam3rule Get Return Top-Level Assembly Mapper attribute(s)
** @nam4rule Adaptor Return the Ensembl Assembly Mapper Adaptor
** @nam4rule Coordsystem Return an Ensembl Coordinate System
** @nam5rule Assembled Return the assembled Coordinate System
** @nam5rule Component Return the component Coordinate System
**
** @argrule * tlam [const EnsPToplevelassemblymapper] Top-Level Assembly Mapper
**
** @valrule Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @valrule Assembled [const EnsPCoordsystem] Ensembl Coordinate System
** @valrule Component [const EnsPCoordsystem] Ensembl Coordinate System
**
** @fcategory use
******************************************************************************/




/* @func ensToplevelassemblymapperGetAdaptor **********************************
**
** Get the Ensembl Assembly Mapper Adaptor element of an
** Ensembl Top-Level Assembly Mapper.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::adaptor
** @param [r] tlam [const EnsPToplevelassemblymapper] Ensembl Top-Level
**                                                    Assembly Mapper
**
** @return [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblymapperadaptor ensToplevelassemblymapperGetAdaptor(
    const EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    return tlam->Adaptor;
}




/* @func ensToplevelassemblymapperGetCoordsystemAssembled *********************
**
** Get the Assembled Ensembl Coordinate System element of an
** Ensembl Top-Level Assembly Mapper.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::assembled_CoordSystem
** @param [r] tlam [const EnsPToplevelassemblymapper] Ensembl Top-Level
**                                                    Assembly Mapper
**
** @return [const EnsPCoordsystem] Assembled Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensToplevelassemblymapperGetCoordsystemAssembled(
    const EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    return tlam->CoordsystemAssembled;
}




/* @func ensToplevelassemblymapperGetCoordsystemComponent *********************
**
** Get the component Ensembl Coordinate System element of an
** Ensembl Top-Level Assembly Mapper.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::component_CoordSystem
** @param [r] tlam [const EnsPToplevelassemblymapper] Ensembl Top-Level
**                                                    Assembly Mapper
**
** @return [const EnsPCoordsystem] Component Ensembl Coordinate System or NULL
** @@
******************************************************************************/

const EnsPCoordsystem ensToplevelassemblymapperGetCoordsystemComponent(
    const EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    return tlam->CoordsystemComponent;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Top-Level Assembly Mapper object.
**
** @fdata [EnsPToplevelassemblymapper]
**
** @nam3rule Set Set one element of a Top-Level Assembly Mapper
** @nam4rule Adaptor Set the Ensembl Assembly Mapper Adaptor
**
** @argrule * tlam [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly
**                                              Mapper object
** @argrule Adaptor ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper
** Adaptor
**
** @valrule * [AjBool] ajTrue on success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensToplevelassemblymapperSetAdaptor **********************************
**
** Set the Ensembl Assembly Mapper Adaptor element of an
** Ensembl Top-Level Assembly Mapper.
**
** @param [u] tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                              Assembly Mapper
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly
**                                            Mapper Adaptor
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensToplevelassemblymapperSetAdaptor(EnsPToplevelassemblymapper tlam,
                                           EnsPAssemblymapperadaptor ama)
{
    if(!tlam)
        return ajFalse;

    if(!ama)
        return ajFalse;

    tlam->Adaptor = ama;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Top-Level Assembly Mapper object.
**
** @fdata [EnsPToplevelassemblymapper]
**
** @nam3rule Trace Report Ensembl Top-Level Assembly Mapper elements to
** debug file.
**
** @argrule Trace tlam [const EnsPToplevelassemblymapper] Ensembl Top-Level
** Assembly Mapper
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensToplevelassemblymapperTrace ***************************************
**
** Trace an Ensembl Top-Level Assembly Mapper.
**
** @param [r] tlam [const EnsPToplevelassemblymapper] Ensembl Top-Level
** Assembly Mapper
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensToplevelassemblymapperTrace(const EnsPToplevelassemblymapper tlam,
                                      ajuint level)
{
    AjPStr indent = NULL;

    if(!tlam)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensToplevelassemblymapperTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Coordsystems %p\n"
            "%S  CoordsystemAssembled %p\n"
            "%S  CoordsystemComponent %p\n"
            "%S  Use %u\n",
            indent, tlam,
            indent, tlam->Adaptor,
            indent, tlam->Coordsystems,
            indent, tlam->CoordsystemAssembled,
            indent, tlam->CoordsystemComponent,
            indent, tlam->Use);

    ensCoordsystemTrace(tlam->CoordsystemAssembled, level + 1);
    ensCoordsystemTrace(tlam->CoordsystemComponent, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section map ***************************************************************
**
** Map coordinates based on Ensembl Sequence Region identifiers between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPToplevelassemblymapper]
**
** @nam3rule Fastmap Fast map coordinates
** @nam3rule Map Map coordinates
**
** @argrule * tlam [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly
** Mapper
** @argrule Map sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Map srstart [ajint] Sequence Region Start coordinate
** @argrule Map srend [ajint] Sequence Region End coordinate
** @argrule Map srstrand [ajint] Sequence Region Strand information
** @argrule Map fastmap [AjBool] Fast-mapping attribute
** @argrule Map mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensToplevelassemblymapperMap *****************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Top-Level Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::map
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::fastmap
** @param [u] tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                              Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Sequence Region
** @param [r] srstart [ajint] Sequence Region start coordinate
** @param [r] srend [ajint] Sequence Region end coordinate
** @param [r] srstrand [ajint] Sequence Region strand information
** @param [r] fastmap [AjBool] Fast-mapping attribute
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensToplevelassemblymapperMap(EnsPToplevelassemblymapper tlam,
                                    const EnsPSeqregion sr,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    AjBool fastmap,
                                    AjPList mrs)
{
    ajuint srid = 0;

    AjBool done = AJFALSE;

    AjIList iter = NULL;
    const AjPList mappath = NULL;

    EnsPAssemblymapper am = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPCoordsystem tstcs = NULL;
    const EnsPCoordsystemadaptor csa = NULL;

    EnsPMapperresult mr = NULL;

    if(ajDebugTest("ensToplevelassemblymapperMap"))
        ajDebug("ensToplevelassemblymapperMap\n"
                "  tlam %p\n"
                "  sr %p\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %d\n"
                "  fastmap '%B'\n"
                "  mrs %p\n",
                tlam,
                sr,
                srstart,
                srend,
                srstrand,
                fastmap,
                mrs);

    if(!tlam)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    /*
    ** NOTE: A test for top-level Coordinate Systems is not required,
    ** since Coordinate Systems are passed in linked to Sequence Regions,
    ** which are never associated with top-level Coordinate Systems.
    */

    if(!ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                            tlam->CoordsystemComponent))
        ajFatal("ensToplevelassemblymapperMap got an Ensembl Sequence Region, "
                "which Coordinate System '%S:%S' "
                "is neither the assembled nor the component Coordinate System "
                "of this Top-Level Assembly Mapper.",
                ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
                ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)));

    dba = ensAssemblymapperadaptorGetDatabaseadaptor(tlam->Adaptor);
    csa = ensRegistryGetCoordsystemadaptor(dba);

    srid = ensSeqregionGetIdentifier(sr);

    iter = ajListIterNew(tlam->Coordsystems);

    while(!ajListIterDone(iter))
    {
        tstcs = (EnsPCoordsystem) ajListIterGet(iter);

        if(ensCoordsystemGetRank(tstcs) >=
           ensCoordsystemGetRank(tlam->CoordsystemComponent))
            break;

        /* Check if a mapping path even exists to this Coordinate System. */

        mappath = ensCoordsystemadaptorGetMappingpath(
            csa,
            tstcs,
            tlam->CoordsystemComponent);

        if(ajListGetLength(mappath))
        {
            /*
            ** Try to map to this Coordinate System.
            ** If we get back any coordinates then it is our 'toplevel' that
            ** we were looking for.
            */

            ensAssemblymapperadaptorFetchByCoordsystems(
                tlam->Adaptor,
                tlam->CoordsystemComponent,
                tstcs,
                &am);

            if(fastmap)
            {
                ensAssemblymapperFastmap(am,
                                         sr,
                                         srstart,
                                         srend,
                                         srstrand,
                                         mrs);

                if(ajListGetLength(mrs))
                {
                    done = ajTrue;

                    break;
                }
            }
            else
            {
                ensAssemblymapperMapSeqregion(am,
                                              sr,
                                              srstart,
                                              srend,
                                              srstrand,
                                              mrs);

                if(ajListGetLength(mrs))
                {
                    if(ajListGetLength(mrs) > 1)
                    {
                        done = ajTrue;

                        break;
                    }

                    ajListPeekFirst(mrs, (void**) &mr);

                    if(ensMapperresultGetType(mr) != ensEMapperresultTypeGap)
                    {
                        done = ajTrue;

                        break;
                    }

                    while(ajListPop(mrs, (void**) &mr))
                        ensMapperresultDel(&mr);
                }
            }

            ensAssemblymapperDel(&am);
        }
    }

    ajListIterDel(&iter);

    if(done)
    {
        ensAssemblymapperDel(&am);

        return ajTrue;
    }

    /*
    ** The top-level coordinate system for the region requested *is* the
    ** requested region.
    */

    mr = ensMapperresultNewCoordinate(srid,
                                      srstart,
                                      srend,
                                      srstrand,
                                      tlam->CoordsystemComponent,
                                      0);

    ajListPushAppend(mrs, (void*) mr);

    return ajTrue;
}




/* @datasection [EnsPAssemblymapper] Ensembl Assembly Mapper ******************
**
** @nam2rule Assemblymapper Functions for manipulating
** Ensembl Assembly Mapper objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly Mapper by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly Mapper. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAssemblymapper]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy am [const EnsPAssemblymapper] Ensembl Assembly Mapper
** @argrule Ini ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @argrule Ini gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @argrule Ini cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @argrule Ini tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
** @argrule Ref am [EnsPAssemblymapper] Ensembl Assembly Mapper
**
** @valrule * [EnsPAssemblymapper] Ensembl Assembly Mapper
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblymapperNewIni **********************************************
**
** Constructor for an Ensembl Assembly Mapper with initial values.
**
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [u] tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                             Assembly Mapper
**
** @return [EnsPAssemblymapper] Ensembl Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPAssemblymapper ensAssemblymapperNewIni(EnsPAssemblymapperadaptor ama,
                                           EnsPGenericassemblymapper gam,
                                           EnsPChainedassemblymapper cam,
                                           EnsPToplevelassemblymapper tlam)
{
    EnsPAssemblymapper am = NULL;

    if(ajDebugTest("ensAssemblymapperNewIni"))
    {
        ajDebug("ensAssemblymapperNewIni\n"
                "  ama %p\n"
                "  gam %p\n"
                "  cam %p\n"
                "  tlam %p\n",
                ama,
                gam,
                cam,
                tlam);

        ensGenericassemblymapperTrace(gam, 1);

        ensChainedassemblymapperTrace(cam, 1);

        ensToplevelassemblymapperTrace(tlam, 1);
    }

    if(!ama)
        return NULL;

    if(!(gam || cam || tlam))
        return NULL;

    if(gam && (cam || tlam))
        ajFatal("ensAssemblymapperNewIni ");

    if(cam && (gam || tlam))
        ajFatal("ensAssemblymapperNewIni ");

    if(tlam && (gam || cam))
        ajFatal("ensAssemblymapperNewIni ");

    AJNEW0(am);

    am->Adaptor = ama;

    am->Generic  = ensGenericassemblymapperNewRef(gam);
    am->Chained  = ensChainedassemblymapperNewRef(cam);
    am->Toplevel = ensToplevelassemblymapperNewRef(tlam);

    am->Use = 1;

    return am;
}




/* @func ensAssemblymapperNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
**
** @return [EnsPAssemblymapper] Ensembl Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPAssemblymapper ensAssemblymapperNewRef(EnsPAssemblymapper am)
{
    if(!am)
        return NULL;

    am->Use++;

    return am;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Assembly Mapper object.
**
** @fdata [EnsPAssemblymapper]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Mapper object
**
** @argrule * Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblymapperDel *************************************************
**
** Default destructor for an Ensembl Assembly Mapper.
**
** @param [d] Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper object address
**
** @return [void]
** @@
******************************************************************************/

void ensAssemblymapperDel(EnsPAssemblymapper* Pam)
{
    EnsPAssemblymapper pthis = NULL;

    if(!Pam)
        return;

    if(!*Pam)
        return;

    if(ajDebugTest("ensAssemblymapperDel"))
        ajDebug("ensAssemblymapperDel\n"
                "  *Pam %p\n",
                *Pam);

    pthis = *Pam;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pam = NULL;

        return;
    }

    ensGenericassemblymapperDel(&pthis->Generic);

    ensChainedassemblymapperDel(&pthis->Chained);

    ensToplevelassemblymapperDel(&pthis->Toplevel);

    AJFREE(pthis);

    *Pam = NULL;

    return;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Assembly Mapper object.
**
** @fdata [EnsPAssemblymapper]
**
** @nam3rule Trace Report Ensembl Assembly Mapper elements to debug file.
**
** @argrule Trace am [const EnsPAssemblymapper] Ensembl Assembly Mapper
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblymapperTrace ***********************************************
**
** Trace an Ensembl Assembly Mapper.
**
** @param [r] am [const EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperTrace(const EnsPAssemblymapper am, ajuint level)
{
    AjPStr indent = NULL;

    if(!am)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensAssemblymapperTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Generic %p\n"
            "%S  Chained %p\n"
            "%S  Toplevel %p\n"
            "%S  Use %u\n",
            indent, am,
            indent, am->Adaptor,
            indent, am->Generic,
            indent, am->Chained,
            indent, am->Toplevel,
            indent, am->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section map ***************************************************************
**
** Map coordinates based on Ensembl Sequence Region identifiers between
** Ensembl Coordinate Systems.
**
** @fdata [EnsPAssemblymapper]
**
** @nam3rule Fastmap Fast map coordinates
** @nam3rule Map Map coordinates
** @nam4rule Seqregion Map an Ensembl Sequence Region
** @nam4rule Slice     Map an Ensembl Slice
** @nam4rule To        Special map function to map to a specified region
** @nam5rule Seqregion Map an Ensembl Sequence Region to a particular
** Ensembl Sequence Region
** @nam5rule Slice     Map an Ensembl Slice to a particular Ensembl Slice
**
** @argrule * am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @argrule Fastmap sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Fastmap srstart [ajint] Sequence Region Start coordinate
** @argrule Fastmap srend [ajint] Sequence Region End coordinate
** @argrule Fastmap srstrand [ajint] Sequence Region Strand information
** @argrule Fastmap mrs [AjPList] AJAX List of Ensembl Mapper Result objects
** @argrule MapSeqregion sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule MapSeqregion srstart [ajint] Sequence Region Start coordinate
** @argrule MapSeqregion srend [ajint] Sequence Region End coordinate
** @argrule MapSeqregion srstrand [ajint] Sequence Region Strand information
** @argrule MapSeqregion mrs [AjPList] AJAX List of Ensembl Mapper Result
** objects
** @argrule MapSlice slice [const EnsPSlice] Ensembl Slice
** @argrule MapSlice mrs [AjPList] AJAX List of Ensembl Mapper Result objects
** @argrule MapToSeqregion sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule MapToSeqregion srstart [ajint] Sequence Region start coordinate
** @argrule MapToSeqregion srend [ajint] Sequence Region end coordinate
** @argrule MapToSeqregion srstrand [ajint] Sequence Region strand information
** @argrule MapToSeqregion optsr [const EnsPSeqregion] Optional Ensembl
** Sequence Region
** @argrule MapToSeqregion mrs [AjPList] AJAX List of Ensembl Mapper Result
** objects
** @argrule MapToSlice slice [const EnsPSlice] Ensembl Slice
** @argrule MapToSlice optslice [const EnsPSlice] Optional Ensembl Slice
** @argrule MapToSlice mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblymapperFastmap *********************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperFastmap(EnsPAssemblymapper am,
                                const EnsPSeqregion sr,
                                ajint srstart,
                                ajint srend,
                                ajint srstrand,
                                AjPList mrs)
{
    if(!am)
        return ajFalse;

    if(am->Generic)
        ensGenericassemblymapperFastmap(am->Generic,
                                        sr,
                                        srstart,
                                        srend,
                                        srstrand,
                                        mrs);

    if(am->Chained)
        ensChainedassemblymapperMap(am->Chained,
                                    sr,
                                    srstart,
                                    srend,
                                    srstrand,
                                    (const EnsPSeqregion) NULL,
                                    ajTrue,
                                    mrs);

    if(am->Toplevel)
        ensToplevelassemblymapperMap(am->Toplevel,
                                     sr,
                                     srstart,
                                     srend,
                                     srstrand,
                                     ajTrue,
                                     mrs);

    return ajTrue;
}




/* @func ensAssemblymapperMapSeqregion ****************************************
**
** Transform coordinates of an Ensembl Sequence Region from one
** Ensembl Coordinate System to another by means of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region start coordinate
** @param [r] srend [ajint] Sequence Region end coordinate
** @param [r] srstrand [ajint] Sequence Region strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperMapSeqregion(EnsPAssemblymapper am,
                                     const EnsPSeqregion sr,
                                     ajint srstart,
                                     ajint srend,
                                     ajint srstrand,
                                     AjPList mrs)
{
    if(ajDebugTest("ensAssemblymapperMapSeqregion"))
    {
        ajDebug("ensAssemblymapperMapSeqregion\n"
                "  am %p\n"
                "  sr %p\n"
                "  srstart %d\n"
                "  srend %d\n"
                "  srstrand %p\n"
                "  mrs %p\n",
                am,
                sr,
                srstart,
                srend,
                srstrand,
                mrs);

        ensAssemblymapperTrace(am, 1);

        ensSeqregionTrace(sr, 1);
    }

    if(!am)
        return ajFalse;

    if(am->Generic)
        ensGenericassemblymapperMap(am->Generic,
                                    sr,
                                    srstart,
                                    srend,
                                    srstrand,
                                    mrs);

    if(am->Chained)
        ensChainedassemblymapperMap(am->Chained,
                                    sr,
                                    srstart,
                                    srend,
                                    srstrand,
                                    (const EnsPSeqregion) NULL,
                                    ajFalse,
                                    mrs);

    if(am->Toplevel)
        ensToplevelassemblymapperMap(am->Toplevel,
                                     sr,
                                     srstart,
                                     srend,
                                     srstrand,
                                     ajFalse,
                                     mrs);

    return ajTrue;
}




/* @func ensAssemblymapperMapSlice ********************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperMapSlice(EnsPAssemblymapper am,
                                 const EnsPSlice slice,
                                 AjPList mrs)
{
    if(!am)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    return ensAssemblymapperMapSeqregion(am,
                                         ensSliceGetSeqregion(slice),
                                         ensSliceGetStart(slice),
                                         ensSliceGetEnd(slice),
                                         ensSliceGetStrand(slice),
                                         mrs);
}




/* @func ensAssemblymapperMapToSeqregion **************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region start coordinate
** @param [r] srend [ajint] Sequence Region end coordinate
** @param [r] srstrand [ajint] Sequence Region strand information
** @param [rN] optsr [const EnsPSeqregion] Optional Ensembl Sequence Region
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperMapToSeqregion(EnsPAssemblymapper am,
                                       const EnsPSeqregion sr,
                                       ajint srstart,
                                       ajint srend,
                                       ajint srstrand,
                                       const EnsPSeqregion optsr,
                                       AjPList mrs)
{
    if(!am)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    if(am->Generic)
        ensGenericassemblymapperMap(am->Generic,
                                    sr,
                                    srstart,
                                    srend,
                                    srstrand,
                                    mrs);

    if(am->Chained)
        ensChainedassemblymapperMap(am->Chained,
                                    sr,
                                    srstart,
                                    srend,
                                    srstrand,
                                    optsr,
                                    ajFalse,
                                    mrs);

    if(am->Toplevel)
        ensToplevelassemblymapperMap(am->Toplevel,
                                     sr,
                                     srstart,
                                     srend,
                                     srstrand,
                                     ajFalse,
                                     mrs);

    return ajTrue;
}




/* @func ensAssemblymapperMapToSlice ******************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Result objects before deleting the AJAX List.
**
** @param [u] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [rN] optslice [const EnsPSlice] Optional Ensembl Slice
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Result objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperMapToSlice(EnsPAssemblymapper am,
                                   const EnsPSlice slice,
                                   const EnsPSlice optslice,
                                   AjPList mrs)
{
    if(!am)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!mrs)
        return ajFalse;

    return ensAssemblymapperMapToSeqregion(am,
                                           ensSliceGetSeqregion(slice),
                                           ensSliceGetStart(slice),
                                           ensSliceGetEnd(slice),
                                           ensSliceGetStrand(slice),
                                           ensSliceGetSeqregion(optslice),
                                           mrs);
}




/* @datasection [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor ***
**
** @nam2rule Assemblymapperadaptor Functions for manipulating
** Ensembl Assembly Mapper Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor
** @cc CVS Revision: 1.63
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic assemblymapperadaptorHasMultipleMappings ***********************
**
** Check whether an Ensembl Sequence Region maps to more than one location.
**
** @param [r] ama [const EnsPAssemblymapperadaptor] Ensembl Assembly Mapper
**                                                  Adaptor
** @param [r] srid [ajuint] Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblymapperadaptorHasMultipleMappings(
    const EnsPAssemblymapperadaptor ama,
    ajuint srid)
{
    if(!ama)
        return ajFalse;

    if(!srid)
        return ajFalse;

    if(!ama->MultipleMappings)
        ajFatal("assemblymapperadaptorHasMultipleMappings AJAX Table for "
                "multiple Sequence Region mappings has not been "
                "initialised!\n");

    if(ajTableFetchmodV(ama->MultipleMappings, (const void*) &srid))
        return ajTrue;

    return ajFalse;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly Mapper Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly Mapper Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAssemblymapperadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @fcategory new
******************************************************************************/




/* @funcstatic assemblymapperadaptorMultipleMappingsInit **********************
**
** Initialise Ensembl Assembly Mapper Adaptor-internal cache of
** Ensembl Sequence Regions that map to more than one location.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::
**     cache_seq_ids_with_mult_assemblys
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblymapperadaptorMultipleMappingsInit(
    EnsPAssemblymapperadaptor ama)
{
    ajuint identifier = 0;

    ajuint* Pidentifier = NULL;

    AjBool* Pbool = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(ajDebugTest("assemblymapperadaptorMultipleMappingsInit"))
        ajDebug("assemblymapperadaptorMultipleMappingsInit\n"
                "  ama %p\n",
                ama);

    if(!ama)
        return ajFalse;

    if(ama->MultipleMappings)
        return ajTrue;

    ama->MultipleMappings = ensTableuintNewLen(0);

    dba = ensAssemblymapperadaptorGetDatabaseadaptor(ama);

    statement = ajFmtStr(
        "SELECT "
        "seq_region_attrib.seq_region_id "
        "FROM "
        "seq_region_attrib, "
        "attrib_type, "
        "seq_region, "
        "coord_system "
        "WHERE "
        "seq_region_attrib.attrib_type_id = attrib_type.attrib_type_id "
        "AND "
        "attrib_type.code = 'MultAssem' "
        "AND "
        "seq_region_attrib.seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = coord_system.coord_system_id "
        "AND "
        "coord_system.species_id = %u",
        ensDatabaseadaptorGetIdentifier(dba));

    sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);

        AJNEW0(Pidentifier);

        *Pidentifier = identifier;

        AJNEW0(Pbool);

        *Pbool = ajTrue;

        if(ajTableFetchmodV(ama->MultipleMappings, (const void*) Pidentifier))
        {
            ajWarn("assemblymapperadaptorMultipleMappingsInit already "
                   "cached Ensembl Sequence region with identifier %u.\n",
                   *Pidentifier);

            AJFREE(Pidentifier);
            AJFREE(Pbool);
        }
        else
            ajTablePut(ama->MultipleMappings,
                       (void*) Pidentifier,
                       (void*) Pbool);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorNew ******************************************
**
** Default constructor for an Ensembl Assembly Mapper Adaptor.
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
** @see ensRegistryGetAssemblymapperadaptor
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblymapperadaptor ensAssemblymapperadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAssemblymapperadaptor ama = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorNew"))
        ajDebug("ensAssemblymapperadaptorNew\n"
                "  dba %p\n",
                dba);

    if(!dba)
        return NULL;

    AJNEW0(ama);

    ama->Adaptor = dba;

    ama->AsmMapperCache = ensTablestrNewLen(0);

    assemblymapperadaptorMultipleMappingsInit(ama);

    return ama;
}




/* @section clear *************************************************************
**
** Clear Ensembl Assembly Mapper Adaptor internals.
**
** @fdata [EnsPAssemblymapperadaptor]
**
** @nam3rule Clear Clear an Ensembl Assembly Mapper object
**
** @argrule * ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory delete
******************************************************************************/




/* @funcstatic assemblymapperadaptorClear *************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX String key data (Ensembl Coordinate System path) and
** Ensembl Assembly Mapper value data.
**
** @param [u] key [void**] AJAX String address
** @param [u] value [void**] Ensembl Assembly Mapper address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void assemblymapperadaptorClear(void** key,
                                       void** value,
                                       void* cl)
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

    ensAssemblymapperDel((EnsPAssemblymapper*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensAssemblymapperadaptorClear ****************************************
**
** Clears the Ensembl Assembly Mapper Adaptor-internal cache of
** Ensembl Assembly Mapper objects.
**
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorClear(EnsPAssemblymapperadaptor ama)
{
    if(ajDebugTest("ensAssemblymapperadaptorClear"))
        ajDebug("ensAssemblymapperadaptorClear\n"
                "  ama %p\n",
                ama);

    if(!ama)
        return ajFalse;

    /* Clear the Ensembl Assembly Mapper cache. */

    ajTableMapDel(ama->AsmMapperCache, assemblymapperadaptorClear, NULL);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Assembly Mapper Adaptor object.
**
** @fdata [EnsPAssemblymapperadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Mapper Adaptor object
**
** @argrule * Pama [EnsPAssemblymapperadaptor*] Ensembl Assembly Mapper Adaptor
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblymapperadaptorDel ******************************************
**
** Default destructor for an Ensembl Assembly Mapper Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pama [EnsPAssemblymapperadaptor*] Ensembl Assembly Mapper Adaptor
**                                              object address
**
** @return [void]
** @@
******************************************************************************/

void ensAssemblymapperadaptorDel(EnsPAssemblymapperadaptor* Pama)
{
    EnsPAssemblymapperadaptor pthis = NULL;

    if(!Pama)
        return;

    if(!*Pama)
        return;

    if(ajDebugTest("ensAssemblymapperadaptorDel"))
        ajDebug("ensAssemblymapperadaptorDel\n"
                "  *Pama %p\n",
                *Pama);

    pthis = *Pama;

    /* Clear the Assembly Mapper AJAX Table. */

    ensAssemblymapperadaptorClear(pthis);

    ajTableFree(&pthis->AsmMapperCache);

    /* Clear and delete the AJAX Table of multiple mappings. */

    ensTableDelete(&pthis->MultipleMappings);

    AJFREE(pthis);

    *Pama = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Assembly Mapper Adaptor object.
**
** @fdata [EnsPAssemblymapperadaptor]
**
** @nam3rule Get Return Assembly Mapper Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ama [const EnsPAssemblymapperadaptor] Ensembl Assembly
**                                                  Mapper Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblymapperadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Assembly Mapper Adaptor.
**
** @param [r] ama [const EnsPAssemblymapperadaptor] Ensembl Assembly
**                                                  Mapper Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAssemblymapperadaptorGetDatabaseadaptor(
    const EnsPAssemblymapperadaptor ama)
{
    if(!ama)
        return NULL;

    return ama->Adaptor;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Assembly Mapper objects from an
** Ensembl SQL database.
**
** @fdata [EnsPAssemblymapperadaptor]
**
** @nam3rule Fetch Fetch Ensembl Assembly Mapper object(s)
** @nam4rule All   Fetch all Ensembl Assembly Mapper objects
** @nam5rule Allby Fetch all Ensembl Assembly Mapper objects
**                 matching a criterion
** @nam4rule By    Fetch one Ensembl Assembly Mapper object
**                 matching a criterion
** @nam5rule Coordsystems Fetch by two Ensembl Coordinate System objects
** @nam5rule Slices Fetch by two Ensembl Slice objects
**
** @argrule * ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @argrule ByCoordsystems cs1 [EnsPCoordsystem] Ensembl Coordinate System
** @argrule ByCoordsystems cs2 [EnsPCoordsystem] Ensembl Coordinate System
** @argrule ByCoordsystems Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper
** address
** @argrule BySlices slice1 [EnsPSlice] Ensembl Slice
** @argrule BySlices slice2 [EnsPSlice] Ensembl Slice
** @argrule BySlices Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblymapperadaptorFetchByCoordsystems **************************
**
** Fetch an Ensembl Assembly Mapper via an Ensembl Coordinate System pair.
**
** The caller is responsible for deleting the Ensembl Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::fetch_by_CoordSystems
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cs1 [EnsPCoordsystem] Ensembl Coordinate System
** @param [u] cs2 [EnsPCoordsystem] Ensembl Coordinate System
** @param [wP] Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorFetchByCoordsystems(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2,
    EnsPAssemblymapper* Pam)
{
    AjBool debug = AJFALSE;

    const AjPList mappath = NULL;
    AjIList iter          = NULL;

    AjPStr keystr = NULL;

    EnsPChainedassemblymapper cam = NULL;

    EnsPGenericassemblymapper gam = NULL;

    EnsPToplevelassemblymapper tlam = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem midcs = NULL;
    EnsPCoordsystem trgcs = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    if(!ama)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires an "
                "Ensembl Assembly Mapper Adaptor.\n");

        return ajFalse;
    }

    if(!cs1)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires a "
                "primary Ensembl Coordinate System.\n");

        return ajFalse;
    }

    if(!cs2)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires a "
                "secondary Ensembl Coordinate System.\n");

        return ajFalse;
    }

    if(!Pam)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires an "
                "Ensembl Assembly Mapper object address.\n");

        return ajFalse;
    }

    debug = ajDebugTest("ensAssemblymapperadaptorFetchByCoordsystems");

    if(debug)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems\n"
                "  ama %p\n"
                "  cs1 %p\n"
                "  cs2 %p\n"
                "  *Pam %p\n",
                ama,
                cs1,
                cs2,
                *Pam);

        ensCoordsystemTrace(cs1, 1);
        ensCoordsystemTrace(cs2, 1);
    }

    *Pam = (EnsPAssemblymapper) NULL;

    /*
    ** NOTE: Coordinate mapping between Ensembl Sequence Region objects with
    ** multiple locations within the same Ensembl Coordinate System are now
    ** possible.
    */

    if(ensCoordsystemGetToplevel(cs1))
    {
        /* Ensembl Top-Level Assembly Mapper */

        tlam = ensToplevelassemblymapperNewIni(ama, cs1, cs2);

        *Pam = ensAssemblymapperNewIni(
            ama,
            (EnsPGenericassemblymapper) NULL,
            (EnsPChainedassemblymapper) NULL,
            tlam);

        ensToplevelassemblymapperDel(&tlam);

        return ajTrue;
    }

    if(ensCoordsystemGetToplevel(cs2))
    {
        /* Ensembl Top-Level Assembly Mapper */

        tlam = ensToplevelassemblymapperNewIni(ama, cs2, cs1);

        *Pam = ensAssemblymapperNewIni(
            ama,
            (EnsPGenericassemblymapper) NULL,
            (EnsPChainedassemblymapper) NULL,
            tlam);

        ensToplevelassemblymapperDel(&tlam);

        return ajTrue;
    }

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    mappath = ensCoordsystemadaptorGetMappingpath(csa, cs1, cs2);

    if(!ajListGetLength(mappath))
    {
        if(debug)
            ajDebug("ensAssemblymapperadaptorFetchByCoordsystems "
                    "got no mapping path between Coordinate Systems "
                    "'%S:%S' and '%S:%S'.\n",
                    ensCoordsystemGetName(cs1),
                    ensCoordsystemGetVersion(cs1),
                    ensCoordsystemGetName(cs2),
                    ensCoordsystemGetVersion(cs2));

        return ajTrue;
    }

    keystr = ajStrNew();

    iter = ajListIterNewread(mappath);

    while(!ajListIterDone(iter))
    {
        srccs = (EnsPCoordsystem) ajListIterGet(iter);

        ajFmtPrintAppS(&keystr, "%u:", ensCoordsystemGetIdentifier(srccs));
    }

    ajListIterDel(&iter);

    *Pam = (EnsPAssemblymapper) ajTableFetchmodV(ama->AsmMapperCache,
                                                 (const void*) keystr);

    if(*Pam)
        ensAssemblymapperNewRef(*Pam);
    else
    {
        if(ajListGetLength(mappath) == 1)
            ajFatal("ensAssemblymapperadaptorFetchByCoordsystems "
                    "got an incorrect mapping path from Ensembl Core 'meta' "
                    "table. Zero step mapping path defined between "
                    "Coordinate Systems '%S:%S' and '%S:%S'.",
                    ensCoordsystemGetName(cs1), ensCoordsystemGetVersion(cs1),
                    ensCoordsystemGetName(cs2), ensCoordsystemGetVersion(cs2));
        else if(ajListGetLength(mappath) == 2)
        {
            /* Ensembl Generic Assembly Mapper */

            ajListPeekNumber(mappath, 0, (void**) &srccs);
            ajListPeekNumber(mappath, 1, (void**) &trgcs);

            gam = ensGenericassemblymapperNewIni(ama, srccs, trgcs);

            *Pam = ensAssemblymapperNewIni(
                ama,
                gam,
                (EnsPChainedassemblymapper) NULL,
                (EnsPToplevelassemblymapper) NULL);

            ensGenericassemblymapperDel(&gam);

            /* Cache this Ensembl Generic Assembly Mapper. */

            ajTablePut(ama->AsmMapperCache,
                       (void*) ajStrNewS(keystr),
                       (void*) ensAssemblymapperNewRef(*Pam));

            /*
            ** If you want multiple pieces on two Ensembl Sequence Regions
            ** to map to each other you need to make an 'assembly.mapping'
            ** entry in the 'meta' table of the Ensembl Core database, that
            ** is seperated with a '#' character instead of a '|' character.
            */
        }
        else if(ajListGetLength(mappath) == 3)
        {
            /* Ensembl Chained Assembly Mapper */

            ajListPeekNumber(mappath, 0, (void**) &srccs);
            ajListPeekNumber(mappath, 1, (void**) &midcs);
            ajListPeekNumber(mappath, 2, (void**) &trgcs);

            cam = ensChainedassemblymapperNewIni(ama, srccs, midcs, trgcs);

            *Pam = ensAssemblymapperNewIni(
                ama,
                (EnsPGenericassemblymapper) NULL,
                cam,
                (EnsPToplevelassemblymapper) NULL);

            ensChainedassemblymapperDel(&cam);

            /* Cache this Ensembl Chained Assembly Mapper with forward key. */

            ajTablePut(ama->AsmMapperCache,
                       (void*) ajStrNewS(keystr),
                       (void*) ensAssemblymapperNewRef(*Pam));

            ajStrAssignClear(&keystr);

            /*
            ** Register this Chained Assembly Mapper also for
            ** Coordinate System identifiers in reverse order.
            */

            iter = ajListIterNewreadBack(mappath);

            while(!ajListIterDoneBack(iter))
            {
                srccs = (EnsPCoordsystem) ajListIterGetBack(iter);

                ajFmtPrintAppS(&keystr,
                               "%u:",
                               ensCoordsystemGetIdentifier(srccs));
            }

            ajListIterDel(&iter);

            /* Cache this Ensembl Chained Assembly Mapper with reverse key. */

            ajTablePut(ama->AsmMapperCache,
                       (void*) ajStrNewS(keystr),
                       (void*) ensAssemblymapperNewRef(*Pam));

            /*
            ** In multi-step mapping it is possible to get requests with the
            ** coordinate system ordering reversed since both mapping
            ** directions cache on both orderings just in case
            ** e.g.
            ** chromosome <-> contig <-> clone
            ** and
            ** clone <-> contig <-> chromosome
            */
        }
        else
            ajFatal("ensAssemblymapperadaptorFetchByCoordsystems "
                    "got incorrect mapping path of length %u defined "
                    "between Coordinate Systems '%S:%S' and '%S:%S'.",
                    ajListGetLength(mappath),
                    ensCoordsystemGetName(cs1),
                    ensCoordsystemGetVersion(cs1),
                    ensCoordsystemGetName(cs2),
                    ensCoordsystemGetVersion(cs2));
    }

    ajStrDel(&keystr);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorFetchBySlices ********************************
**
** Fetch an Ensembl Assembly Mapper via an Ensembl Slice pair.
**
** The caller is responsible for deleting the Ensembl Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::???
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] slice1 [EnsPSlice] Ensembl Slice
** @param [u] slice2 [EnsPSlice] Ensembl Slice
** @param [wP] Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorFetchBySlices(
    EnsPAssemblymapperadaptor ama,
    EnsPSlice slice1,
    EnsPSlice slice2,
    EnsPAssemblymapper* Pam)
{
    if(!slice1)
        return ajFalse;

    if(!slice2)
        return ajFalse;

    if(!Pam)
        return ajFalse;

    return ensAssemblymapperadaptorFetchByCoordsystems(
        ama,
        ensSliceGetCoordsystemObject(slice1),
        ensSliceGetCoordsystemObject(slice2),
        Pam);
}




/* @section register **********************************************************
**
** Register Ensembl Sequence Region identifiers in an
** Ensembl Generic Assembly Mapper via an Ensembl Assembly Mapper Adaptor.
**
** @fdata [EnsPAssemblymapperadaptor]
**
** @nam3rule Register  Register Ensembl Sequence Region identifiers
** @nam4rule Chained   Register in an Ensembl Chained Assembly Mapper
** @nam5rule All       Register all Ensembl Sequence Region identifiers
** @nam5rule Region    Register an Ensembl Sequenceregion identifier
** @nam5rule Special   Register source and target
** @nam4rule Generic   Register in an Ensembl Generic Assembly Mapper
** @nam5rule All       Register all Ensembl Sequence Region identifiers
** @nam5rule Assembled Register an assembled Ensembl Sequence Region identifier
** @nam5rule Component Register a component Ensembl Sequence Region identifier
**
** @argrule * ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @argrule Chained cam [EnsPChainedassemblymapper] Ensembl Chained Assembly
** Mapper
** @argrule ChainedRegion source [const AjPStr] Mapping type for the source
** @argrule ChainedRegion srcsrid [ajuint] Source Ensembl Sequence Region
** identifier
** @argrule ChainedRegion optsrid [ajuint] Optional Ensembl Sequence Region
** identifier
** @argrule ChainedRegion ranges [AjPList] AJAX List of Ensembl Mapper Range
** objects
** @argrule ChainedSpecial source [const AjPStr] Mapping type for the source
** @argrule ChainedSpecial srcsrid [ajuint] Source Ensembl Sequence Region
** identifier
** @argrule ChainedSpecial optsrid [ajuint] Optional Ensembl Sequence Region
** identifier
** @argrule ChainedSpecial ranges [AjPList] AJAX List of Ensembl Mapper Ranges
** @argrule Generic gam [EnsPGenericassemblymapper] Ensembl Generic Assembly
** Mapper
** @argrule GenericAssembled asmsrid [ajuint] Assembled Ensembl Sequence Region
** identifier
** @argrule GenericAssembled regstart [ajint] Start coordinate
** @argrule GenericAssembled regend [ajint] End ccordinate
** @argrule GenericComponent cmpsrid [ajuint] Component Ensembl Sequence Region
** identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic assemblymapperadaptorBuildCombinedMapper ***********************
**
** Build a combined Mapper after both halves of an
** Ensembl Chained Assembly Mapper have been loaded.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::_build_combined_mapper
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] ranges [AjPList] AJAX List of Ensembl Mapper Ranges
** @param [u] srcmidmapper [EnsPMapper] Ensembl Mapper for source to middle
**                                      Coordinate System mapping
** @param [u] trgmidmapper [EnsPMapper] Ensembl Mapper for target to middle
**                                      Coordinate System mapping
** @param [u] srctrgmapper [EnsPMapper] Ensembl Mapper for source to target
**                                      Coordinate System mapping,
**                                      the combined Mapper
** @param [r] srctype [const AjPStr] Source mapping type
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
** After both halves of an Ensembl Chained Mapper are loaded, this function
** maps all Ensembl Mapper Range objects in the AJAX List and loads the
** Ensembl Mapper Result objects into the combined (srctrgmapper)
** Ensembl Mapper.
******************************************************************************/

static AjBool assemblymapperadaptorBuildCombinedMapper(
    EnsPAssemblymapperadaptor ama,
    AjPList ranges,
    EnsPMapper srcmidmapper,
    EnsPMapper trgmidmapper,
    EnsPMapper srctrgmapper,
    const AjPStr srctype)
{
    ajint sum = 0;

    ajint totalstart = 0;
    ajint totalend   = 0;

    AjIList iter = NULL;
    AjPList mrs1 = NULL;
    AjPList mrs2 = NULL;

    AjPStr midtype = NULL;

    EnsPMapperresult mr1 = NULL;
    EnsPMapperresult mr2 = NULL;

    EnsPMapperunit mu = NULL;

    if(ajDebugTest("assemblymapperadaptorBuildCombinedMapper"))
    {
        ajDebug("assemblymapperadaptorBuildCombinedMapper\n"
                "  ama %p\n"
                "  ranges %p\n"
                "  srcmidmapper %p\n"
                "  trgmidmapper %p\n"
                "  srctrgmapper %p\n"
                "  srctype '%S'\n",
                ama,
                ranges,
                srcmidmapper,
                trgmidmapper,
                srctrgmapper,
                srctype);

        ensMapperTrace(srcmidmapper, 1);
        ensMapperTrace(trgmidmapper, 1);
        ensMapperTrace(srctrgmapper, 1);
    }

    if(!ama)
        return ajFalse;

    if(!ranges)
        return ajFalse;

    if(!srcmidmapper)
        return ajFalse;

    if(!trgmidmapper)
        return ajFalse;

    if(!srctrgmapper)
        return ajFalse;

    if(!srctype)
        return ajFalse;

    midtype = ajStrNewC("middle");

    iter = ajListIterNew(ranges);

    while(!ajListIterDone(iter))
    {
        mu = (EnsPMapperunit) ajListIterGet(iter);

        sum = 0;

        mrs1 = ajListNew();

        ensMapperMap(srcmidmapper,
                     ensMapperunitGetObjectidentifier(mu),
                     ensMapperunitGetStart(mu),
                     ensMapperunitGetEnd(mu),
                     1,
                     srctype,
                     mrs1);

        while(ajListPop(mrs1, (void**) &mr1))
        {
            if(ensMapperresultGetType(mr1) == ensEMapperresultTypeGap)
            {
                sum += ensMapperresultCalculateLengthGap(mr1);

                ensMapperresultDel(&mr1);

                continue;
            }

            /*
            ** Feed the Ensembl Mapper Result objects of the first
            ** Ensembl Mapper into the second.
            */

            mrs2 = ajListNew();

            ensMapperMap(trgmidmapper,
                         ensMapperresultGetObjectidentifier(mr1),
                         ensMapperresultGetCoordinateStart(mr1),
                         ensMapperresultGetCoordinateEnd(mr1),
                         ensMapperresultGetCoordinateStrand(mr1),
                         midtype,
                         mrs2);

            while(ajListPop(mrs2, (void**) &mr2))
            {
                if(ensMapperresultGetType(mr2) ==
                   ensEMapperresultTypeCoordinate)
                {
                    totalstart = ensMapperunitGetStart(mu) + sum;

                    totalend = totalstart +
                        ensMapperresultCalculateLengthCoordinate(mr2) - 1;

                    if(ajStrMatchC(srctype, "source"))
                        ensMapperAddCoordinates(
                            srctrgmapper,
                            ensMapperunitGetObjectidentifier(mu),
                            totalstart,
                            totalend,
                            ensMapperresultGetCoordinateStrand(mr2),
                            ensMapperresultGetObjectidentifier(mr2),
                            ensMapperresultGetCoordinateStart(mr2),
                            ensMapperresultGetCoordinateEnd(mr2));
                    else
                        ensMapperAddCoordinates(
                            srctrgmapper,
                            ensMapperresultGetObjectidentifier(mr2),
                            ensMapperresultGetCoordinateStart(mr2),
                            ensMapperresultGetCoordinateEnd(mr2),
                            ensMapperresultGetCoordinateStrand(mr2),
                            ensMapperunitGetObjectidentifier(mu),
                            totalstart,
                            totalend);
                }

                sum += ensMapperresultCalculateLengthResult(mr2);

                ensMapperresultDel(&mr2);
            }

            ajListFree(&mrs2);

            ensMapperresultDel(&mr1);
        }

        ajListFree(&mrs1);
    }

    ajListIterDel(&iter);

    ajStrDel(&midtype);

    return ajTrue;
}




/* @funcstatic assemblymapperadaptorMappingPath *******************************
**
** Check if an Assembly Mapper has an acceptable mapping path.
**
** Only two-step assembly mapping paths or three-step paths with an
** empty middle Coordinate System are acceptable.
**
** If the middle Coordinate System of multi-component mapping paths is
** NULL, we have a case where multiple parts of a 'component' map to
** more than one 'assembled' part, something like
** supercontig#contig#chromosome. See
** ensCoordsystemadaptorGetMappingpath for details. If the middle
** Coordinate System is defined we have a multi-component mapping
** path, which is not acceptable.
**
** @param [r] mappath [const AjPList] Mapping path, which is an AJAX List of
**                                    Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblymapperadaptorMappingPath(const AjPList mappath)
{
    ajuint length = 0;

    EnsPCoordsystem cs = NULL;

    if(!mappath)
        return ajFalse;

    length = ajListGetLength(mappath);

    if(length == 2)
        return ajTrue;

    if(length == 3)
    {
        ajListPeekNumber(mappath, 1, (void**) &cs);

        if(!cs)
            return ajTrue;
    }

    ajDebug("assemblymapperadaptorMappingPath got inacceptable "
            "assembly mapping path.\n");

    ensListCoordsystemTrace(mappath, 1);

    return ajFalse;
}




/* @func ensAssemblymapperadaptorRegisterChainedAll ***************************
**
** Registers all Ensembl Mapper Ranges in an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_all_chained
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterChainedAll(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam)
{
    ajint ori         = 0;

    ajuint srcsrid    = 0;
    ajint srcsrstart  = 0;
    ajint srcsrend    = 0;
    ajint srcsrlength = 0;

    ajuint midsrid    = 0;
    ajint midsrstart  = 0;
    ajint midsrend    = 0;
    ajint midsrlength = 0;

    ajuint trgsrid    = 0;
    ajint trgsrstart  = 0;
    ajint trgsrend    = 0;
    ajint trgsrlength = 0;

    const AjPList mappath = NULL;
    AjPList ranges = NULL;

    AjISqlrow sqli       = NULL;
    AjPSqlstatement sqls = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr srcsrname = NULL;
    AjPStr midsrname = NULL;
    AjPStr trgsrname = NULL;

    AjPStr sqlfmt    = NULL;
    AjPStr statement = NULL;
    AjPStr srctype   = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem midcs = NULL;
    EnsPCoordsystem trgcs = NULL;

    EnsPCoordsystem asmcs = NULL;
    EnsPCoordsystem cmpcs = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPMapper srcmidmapper = NULL;
    EnsPMapper trgmidmapper = NULL;
    EnsPMapper srctrgmapper = NULL;
    EnsPMapper mapper       = NULL;
    EnsPMapperunit mu       = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterChainedAll"))
        ajDebug("ensAssemblymapperadaptorRegisterChainedAll\n"
                "  ama %p\n"
                "  cam %p\n",
                ama,
                cam);

    if(!ama)
        return ajFalse;

    if(!cam)
        return ajFalse;

    srccs = cam->CoordsystemSource;
    midcs = cam->CoordsystemMiddle;
    trgcs = cam->CoordsystemTarget;

    srcmidmapper = cam->MapperSourceMiddle;
    trgmidmapper = cam->MapperTargetMiddle;
    srctrgmapper = cam->MapperSourceTarget;

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    sqlfmt = ajStrNewC(
        "SELECT "
        "assembly.cmp_start, "
        "assembly.cmp_end, "
        "assembly.cmp_seq_region_id, "
        "sr_cmp.name, "
        "sr_cmp.length, "
        "assembly.ori, "
        "assembly.asm_start, "
        "assembly.asm_end, "
        "assembly.asm_seq_region_id, "
        "sr_asm.name, "
        "sr_asm.length "
        "FROM "
        "assembly, seq_region sr_asm, seq_region sr_cmp "
        "WHERE "
        "sr_asm.seq_region_id = assembly.asm_seq_region_id "
        "AND "
        "sr_cmp.seq_region_id = assembly.cmp_seq_region_id "
        "AND "
        "sr_asm.coord_system_id = %u "
        "AND "
        "sr_cmp.coord_system_id = %u");

    if(midcs)
    {
        mappath = ensCoordsystemadaptorGetMappingpath(csa, srccs, midcs);

        mapper = srcmidmapper;
    }
    else
    {
        mappath = ensCoordsystemadaptorGetMappingpath(csa, srccs, trgcs);

        mapper = srctrgmapper;
    }

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblymapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChainedAll "
                "unexpected mapping between source and intermediate "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(srccs),
                ensCoordsystemGetVersion(srccs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ranges = ajListNew();

    ajListPeekFirst(mappath, (void**) &asmcs);

    ajListPeekLast(mappath, (void**) &cmpcs);

    statement = ajFmtStr(ajStrGetPtr(sqlfmt),
                         ensCoordsystemGetIdentifier(asmcs),
                         ensCoordsystemGetIdentifier(cmpcs));

    sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        srcsrstart  = 0;
        srcsrend    = 0;
        srcsrid     = 0;
        srcsrname   = ajStrNew();
        srcsrlength = 0;
        ori         = 0;
        midsrstart  = 0;
        midsrend    = 0;
        midsrid     = 0;
        midsrname   = ajStrNew();
        midsrlength = 0;

        sqlr = ajSqlrowiterGet(sqli);

        if(ensCoordsystemMatch(srccs, asmcs))
        {
            ajSqlcolumnToInt(sqlr, &midsrstart);
            ajSqlcolumnToInt(sqlr, &midsrend);
            ajSqlcolumnToUint(sqlr, &midsrid);
            ajSqlcolumnToStr(sqlr, &midsrname);
            ajSqlcolumnToInt(sqlr, &midsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &srcsrstart);
            ajSqlcolumnToInt(sqlr, &srcsrend);
            ajSqlcolumnToUint(sqlr, &srcsrid);
            ajSqlcolumnToStr(sqlr, &srcsrname);
            ajSqlcolumnToInt(sqlr, &srcsrlength);
        }
        else
        {
            ajSqlcolumnToInt(sqlr, &srcsrstart);
            ajSqlcolumnToInt(sqlr, &srcsrend);
            ajSqlcolumnToUint(sqlr, &srcsrid);
            ajSqlcolumnToStr(sqlr, &srcsrname);
            ajSqlcolumnToInt(sqlr, &srcsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &midsrstart);
            ajSqlcolumnToInt(sqlr, &midsrend);
            ajSqlcolumnToUint(sqlr, &midsrid);
            ajSqlcolumnToStr(sqlr, &midsrname);
            ajSqlcolumnToInt(sqlr, &midsrlength);
        }

        /*
        ** Load the results into the source <-> middle Mapper or the
        ** source <-> target Mapper in case the Chained Assembly Mapper
        ** is short.
        */

        ensMapperAddCoordinates(mapper,
                                srcsrid,
                                srcsrstart,
                                srcsrend,
                                ori,
                                midsrid,
                                midsrstart,
                                midsrend);

        /* Record Mapper Units for the source part. */

        mu = ensMapperunitNewIni(srcsrid, srcsrstart, srcsrend);

        ajListPushAppend(ranges, (void*) mu);

        /* Register the source Mapper Units in the source Mapper Registry. */

        ensMapperrangeregistryCheck(cam->RegistrySource,
                                    srcsrid,
                                    1,
                                    srcsrlength,
                                    1,
                                    srcsrlength,
                                    (AjPList) NULL);

        /*
        ** If the Chained Assembly Mapper is short, immediately register the
        ** middle Mapper Units in the target Mapper Registry.
        */

        if(!midcs)
            ensMapperrangeregistryCheck(cam->RegistryTarget,
                                        midsrid,
                                        midsrstart,
                                        midsrend,
                                        midsrstart,
                                        midsrend,
                                        (AjPList) NULL);

        /*
        ** Add the (new) source Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNewIni(sra, srcsrid, srccs, srcsrname, srcsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        /*
        ** Add the (new) middle Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNewIni(sra,
                                midsrid,
                                (midcs) ? midcs : trgcs,
                                midsrname,
                                midsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&srcsrname);
        ajStrDel(&trgsrname);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

    ajStrDel(&statement);

    if(!midcs)
    {
        /* That is all for the simple case. */

        while(ajListPop(ranges, (void**) &mu))
            ensMapperunitDel(&mu);

        ajListFree(&ranges);

        ajStrDel(&sqlfmt);

        return ajTrue;
    }

    mappath = ensCoordsystemadaptorGetMappingpath(csa, trgcs, midcs);

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblymapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChainedAll "
                "unexpected mapping between target and intermediate "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(trgcs),
                ensCoordsystemGetVersion(trgcs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void**) &asmcs);

    ajListPeekLast(mappath, (void**) &cmpcs);

    statement = ajFmtStr(ajStrGetPtr(sqlfmt),
                         ensCoordsystemGetIdentifier(asmcs),
                         ensCoordsystemGetIdentifier(cmpcs));

    sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        midsrstart  = 0;
        midsrend    = 0;
        midsrid     = 0;
        midsrname   = ajStrNew();
        midsrlength = 0;
        ori         = 0;
        trgsrstart  = 0;
        trgsrend    = 0;
        trgsrid     = 0;
        trgsrname   = ajStrNew();
        trgsrlength = 0;

        sqlr = ajSqlrowiterGet(sqli);

        if(ensCoordsystemMatch(midcs, asmcs))
        {
            ajSqlcolumnToInt(sqlr, &trgsrstart);
            ajSqlcolumnToInt(sqlr, &trgsrend);
            ajSqlcolumnToUint(sqlr, &trgsrid);
            ajSqlcolumnToStr(sqlr, &trgsrname);
            ajSqlcolumnToInt(sqlr, &trgsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &midsrstart);
            ajSqlcolumnToInt(sqlr, &midsrend);
            ajSqlcolumnToUint(sqlr, &midsrid);
            ajSqlcolumnToStr(sqlr, &midsrname);
            ajSqlcolumnToInt(sqlr, &midsrlength);
        }
        else
        {
            ajSqlcolumnToInt(sqlr, &midsrstart);
            ajSqlcolumnToInt(sqlr, &midsrend);
            ajSqlcolumnToUint(sqlr, &midsrid);
            ajSqlcolumnToStr(sqlr, &midsrname);
            ajSqlcolumnToInt(sqlr, &midsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &trgsrstart);
            ajSqlcolumnToInt(sqlr, &trgsrend);
            ajSqlcolumnToUint(sqlr, &trgsrid);
            ajSqlcolumnToStr(sqlr, &trgsrname);
            ajSqlcolumnToInt(sqlr, &trgsrlength);
        }

        /*
        ** Load the results into the target <-> middle Mapper of the
        ** Chained Assembly Mapper.
        */

        ensMapperAddCoordinates(trgmidmapper,
                                trgsrid,
                                trgsrstart,
                                trgsrend,
                                ori,
                                midsrid,
                                midsrstart,
                                midsrend);

        /*
        ** Register the target Mapper Unit in the
        ** target Mapper Range Registry.
        */

        ensMapperrangeregistryCheck(cam->RegistryTarget,
                                    trgsrid,
                                    1,
                                    trgsrlength,
                                    1,
                                    trgsrlength,
                                    (AjPList) NULL);

        /*
        ** Add the target Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNewIni(sra, trgsrid, trgcs, trgsrname, trgsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&srcsrname);
        ajStrDel(&trgsrname);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

    ajStrDel(&statement);

    srctype = ajStrNewC("source");

    assemblymapperadaptorBuildCombinedMapper(ama,
                                             ranges,
                                             srcmidmapper,
                                             trgmidmapper,
                                             srctrgmapper,
                                             srctype);

    ajStrDel(&srctype);

    while(ajListPop(ranges, (void**) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&ranges);

    ajStrDel(&sqlfmt);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterChainedRegion ************************
**
** Registers an AJAX List of previously unregistered Ensembl Mapper Range
** objects in an Ensembl Chained Assembly Mapper.
**
** Optionally, only those Ensembl Mapper Range objects can be registered,
** which map onto a particular Ensembl Sequence Region, if its identifier
** has been specified.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_chained
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] source [const AjPStr] Mapping type for the source
** @param [r] srcsrid [ajuint] Source Ensembl Sequence Region identifier
** @param [rE] optsrid [ajuint] Optional Ensembl Sequence Region identifier
** @param [u] ranges [AjPList] AJAX List of Ensembl Mapper Range objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterChainedRegion(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam,
    const AjPStr source,
    ajuint srcsrid,
    ajuint optsrid,
    AjPList ranges)
{
    ajint ori         = 0;

    ajint srcsrstart  = 0;
    ajint srcsrend    = 0;

    ajuint midsrid    = 0;
    ajint midsrstart  = 0;
    ajint midsrend    = 0;
    ajint midsrlength = 0;

    ajuint trgsrid    = 0;
    ajint trgsrstart  = 0;
    ajint trgsrend    = 0;
    ajint trgsrlength = 0;

    AjPList srcranges = NULL;
    AjPList midranges = NULL;

    AjIList liter = NULL;
    const AjPList mappath = NULL;

    AjISqlrow sqli       = NULL;
    AjPSqlstatement sqls = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr srctype = NULL;
    AjPStr midtype = NULL;
    AjPStr trgtype = NULL;

    AjPStr midsrname = NULL;
    AjPStr trgsrname = NULL;

    AjPStr asm2cmp   = NULL;
    AjPStr cmp2asm   = NULL;
    AjPStr sqlfmt    = NULL;
    AjPStr statement = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem midcs = NULL;
    EnsPCoordsystem trgcs = NULL;

    EnsPCoordsystem asmcs = NULL;
    EnsPCoordsystem cmpcs = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPMapper srcmidmapper = NULL;
    EnsPMapper srctrgmapper = NULL;
    EnsPMapper trgmidmapper = NULL;

    EnsPMapperrange mr = NULL;

    EnsPMapperrangeregistry srcregistry = NULL;
    EnsPMapperrangeregistry trgregistry = NULL;

    EnsPMapperunit mu = NULL;

    EnsPSeqregion sr = NULL;
    EnsPSeqregion optsr = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterChainedRegion"))
        ajDebug("ensAssemblymapperadaptorRegisterChainedRegion\n"
                "  ama %p\n"
                "  cam %p\n"
                "  source '%S'\n"
                "  srcsrid %u\n"
                "  optsrid %u\n"
                "  ranges %p\n",
                ama,
                cam,
                source,
                srcsrid,
                optsrid,
                ranges);

    if(!ama)
        return ajFalse;

    if(!cam)
        return ajFalse;

    if(!(source && ajStrGetLen(source)))
        return ajFalse;

    if(!srcsrid)
        return ajFalse;

    if(!ranges)
        return ajFalse;

    if(optsrid)
    {
        if(ensCoordsystemMatch(
               ensChainedassemblymapperGetCoordsystemSource(cam),
               ensChainedassemblymapperGetCoordsystemTarget(cam)))
            return ensAssemblymapperadaptorRegisterChainedSpecial(ama,
                                                                  cam,
                                                                  source,
                                                                  srcsrid,
                                                                  optsrid,
                                                                  ranges);
    }

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    if(ajStrMatchC(source, "source"))
    {
        srctype = ajStrNewC("source");

        srcmidmapper = cam->MapperSourceMiddle;

        srccs = cam->CoordsystemSource;

        srcregistry = cam->RegistrySource;

        trgtype = ajStrNewC("target");

        trgmidmapper = cam->MapperTargetMiddle;

        trgcs = cam->CoordsystemTarget;

        trgregistry = cam->RegistryTarget;
    }
    else if(ajStrMatchC(source, "target"))
    {
        srctype = ajStrNewC("target");

        srcmidmapper = cam->MapperTargetMiddle;

        srccs = cam->CoordsystemTarget;

        srcregistry = cam->RegistryTarget;

        trgtype = ajStrNewC("source");

        trgmidmapper = cam->MapperSourceMiddle;

        trgcs = cam->CoordsystemSource;

        trgregistry = cam->RegistrySource;
    }
    else
        ajFatal("ensAssemblymapperadaptorRegisterChainedRegion invalid "
                "'source' argument '%S' must be 'source' or 'target'.",
                source);

    midtype = ajStrNewC("middle");

    srctrgmapper = cam->MapperSourceTarget;

    midcs = cam->CoordsystemMiddle;

    /*
    ** Obtain the first half of the mappings and load them into the
    ** source <-> middle Mapper. Check for the simple case, where the
    ** Chained Assembly Mapper is short and requires only a one-step
    ** source <-> target Mapper.
    */

    if(midcs)
        mappath = ensCoordsystemadaptorGetMappingpath(csa, srccs, midcs);
    else
    {
        mappath = ensCoordsystemadaptorGetMappingpath(csa, srccs, trgcs);

        srcmidmapper = srctrgmapper;
    }

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblymapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChainedRegion "
                "unexpected mapping between source and middle "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(srccs),
                ensCoordsystemGetVersion(srccs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void**) &asmcs);

    ajListPeekLast(mappath, (void**) &cmpcs);

    asm2cmp = ajStrNewC(
        "SELECT "
        "assembly.cmp_start, "
        "assembly.cmp_end, "
        "assembly.cmp_seq_region_id, "
        "seq_region.name, "
        "seq_region.length, "
        "assembly.ori, "
        "assembly.asm_start, "
        "assembly.asm_end "
        "FROM "
        "assembly, "
        "seq_region "
        "WHERE "
        "assembly.asm_seq_region_id = %u "
        "AND "
        "%d <= assembly.asm_end "
        "AND "
        "%d >= assembly.asm_start "
        "AND "
        "assembly.cmp_seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = %u");

    cmp2asm = ajStrNewC(
        "SELECT "
        "assembly.asm_start, "
        "assembly.asm_end, "
        "assembly.asm_seq_region_id, "
        "seq_region.name, "
        "seq_region.length, "
        "assembly.ori, "
        "assembly.cmp_start, "
        "assembly.cmp_end "
        "FROM "
        "assembly, "
        "seq_region "
        "WHERE "
        "assembly.cmp_seq_region_id = %u "
        "AND "
        "%d <= assembly.cmp_end "
        "AND "
        "%d >= assembly.cmp_start "
        "AND "
        "assembly.asm_seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = %u");

    if(optsrid)
    {
        ensSeqregionadaptorFetchByIdentifier(sra, optsrid, &optsr);

        if(ensCoordsystemMatch(asmcs, ensSeqregionGetCoordsystem(optsr)))
            ajFmtPrintAppS(&cmp2asm,
                           " AND asm.asm_seq_region_id = %u",
                           ensSeqregionGetIdentifier(optsr));
        else if(ensCoordsystemMatch(cmpcs, ensSeqregionGetCoordsystem(optsr)))
            ajFmtPrintAppS(&asm2cmp,
                           " AND asm.cmp_seq_region_id = %u",
                           ensSeqregionGetIdentifier(optsr));
        else
            ajWarn("ensAssemblymapperadaptorRegisterChainedRegion got an "
                   "optional Sequence Region, which is neither linked to the "
                   "source nor target Coordinate System.");
    }

    sqlfmt = (ensCoordsystemMatch(srccs, asmcs)) ? asm2cmp : cmp2asm;

    /* Run an SQL statement for each previously unregistered Mapper Range. */

    srcranges = ajListNew();
    midranges = ajListNew();

    liter = ajListIterNew(ranges);

    while(!ajListIterDone(liter))
    {
        mr = (EnsPMapperrange) ajListIterGet(liter);

        statement = ajFmtStr(ajStrGetPtr(sqlfmt),
                             srcsrid,
                             ensMapperrangeGetStart(mr),
                             ensMapperrangeGetEnd(mr),
                             (midcs) ?
                             ensCoordsystemGetIdentifier(midcs) :
                             ensCoordsystemGetIdentifier(trgcs));

        sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

        sqli = ajSqlrowiterNew(sqls);

        while(!ajSqlrowiterDone(sqli))
        {
            midsrstart  = 0;
            midsrend    = 0;
            midsrid     = 0;
            midsrname   = ajStrNew();
            midsrlength = 0;
            ori         = 0;
            srcsrstart  = 0;
            srcsrend    = 0;

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToInt(sqlr, &midsrstart);
            ajSqlcolumnToInt(sqlr, &midsrend);
            ajSqlcolumnToUint(sqlr, &midsrid);
            ajSqlcolumnToStr(sqlr, &midsrname);
            ajSqlcolumnToInt(sqlr, &midsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &srcsrstart);
            ajSqlcolumnToInt(sqlr, &srcsrend);

            /*
            ** Load the results into the source <-> middle Mapper or the
            ** source <-> target Mapper in case the Chained Assembly Mapper
            ** is short.
            */

            if(midcs)
                ensMapperAddCoordinates(srcmidmapper,
                                        srcsrid,
                                        srcsrstart,
                                        srcsrend,
                                        ori,
                                        midsrid,
                                        midsrstart,
                                        midsrend);
            else
            {
                if(ajStrMatchC(source, "source"))
                    ensMapperAddCoordinates(srctrgmapper,
                                            srcsrid,
                                            srcsrstart,
                                            srcsrend,
                                            ori,
                                            midsrid,
                                            midsrstart,
                                            midsrend);
                else
                    ensMapperAddCoordinates(srctrgmapper,
                                            midsrid,
                                            midsrstart,
                                            midsrend,
                                            ori,
                                            srcsrid,
                                            srcsrstart,
                                            srcsrend);
            }

            /* Record Mapper Units for the source part. */

            mu = ensMapperunitNewIni(srcsrid, srcsrstart, srcsrend);

            ajListPushAppend(srcranges, (void*) mu);

            /* Record Mapper Units for the middle part. */

            mu = ensMapperunitNewIni(midsrid, midsrstart, midsrend);

            ajListPushAppend(midranges, (void*) mu);

            /*
            ** Add the (new) middle Ensembl Sequence Region into the
            ** Ensembl Sequence Region Adaptor cache.
            */

            sr = ensSeqregionNewIni(sra,
                                    midsrid,
                                    (midcs) ? midcs : trgcs,
                                    midsrname,
                                    midsrlength);

            ensSeqregionadaptorCacheInsert(sra, &sr);

            ensSeqregionDel(&sr);

            /*
            ** Register the source Mapper Units in the source Mapper Registry.
            ** The region we actually register may be smaller or larger than
            ** the region we originally intended to register.
            ** Register the intersection of the region so we do not end up
            ** doing extra work later.
            */

            if((srcsrstart < ensMapperrangeGetStart(mr)) ||
               (srcsrend > ensMapperrangeGetEnd(mr)))
                ensMapperrangeregistryCheck(srcregistry,
                                            srcsrid,
                                            srcsrstart,
                                            srcsrend,
                                            srcsrstart,
                                            srcsrend,
                                            (AjPList) NULL);

            ajStrDel(&midsrname);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

        ajStrDel(&statement);
    }

    ajListIterDel(&liter);

    /*
    ** In the one-step case, the middle Mapper Units are simply registered
    ** in the target Mapper Range Registry.
    */

    if(!midcs)
    {
        /* The source Mapper Units are no longer needed in this case. */

        while(ajListPop(srcranges, (void**) &mu))
            ensMapperunitDel(&mu);

        ajListFree(&srcranges);

        /*
        ** Register the middle Mapper Units in the
        ** target Mapper Range Registry
        ** and that's it for the simple case ...
        */

        while(ajListPop(midranges, (void**) &mu))
        {
            ensMapperrangeregistryCheck(
                trgregistry,
                ensMapperunitGetObjectidentifier(mu),
                ensMapperunitGetStart(mu),
                ensMapperunitGetEnd(mu),
                ensMapperunitGetStart(mu),
                ensMapperunitGetEnd(mu),
                (AjPList) NULL);

            ensMapperunitDel(&mu);
        }

        ajListFree(&midranges);

        ajStrDel(&srctype);
        ajStrDel(&midtype);
        ajStrDel(&trgtype);
        ajStrDel(&asm2cmp);
        ajStrDel(&cmp2asm);

        return ajTrue;
    }

    /*
    ** For the second half of the mapping an SQL statement for each middle
    ** Mapper Unit is performed. The results are loaded into the
    ** middle <-> target Mapper.
    ** But first, ascertain which is the component and which is actually the
    ** assembled Coordinate System.
    */

    mappath = ensCoordsystemadaptorGetMappingpath(csa, midcs, trgcs);

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblymapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChainedRegion "
                "unexpected mapping between intermediate and target "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ensCoordsystemGetName(trgcs),
                ensCoordsystemGetVersion(trgcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void**) &asmcs);

    ajListPeekLast(mappath, (void**) &cmpcs);

    sqlfmt = (ensCoordsystemMatch(midcs, asmcs)) ? asm2cmp : cmp2asm;

    while(ajListPop(midranges, (void**) &mu))
    {
        statement = ajFmtStr(ajStrGetPtr(sqlfmt),
                             ensMapperunitGetObjectidentifier(mu),
                             ensMapperunitGetStart(mu),
                             ensMapperunitGetEnd(mu),
                             ensCoordsystemGetIdentifier(trgcs));

        sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

        sqli = ajSqlrowiterNew(sqls);

        while(!ajSqlrowiterDone(sqli))
        {
            trgsrstart  = 0;
            trgsrend    = 0;
            trgsrid     = 0;
            trgsrname   = ajStrNew();
            trgsrlength = 0;
            ori         = 0;
            midsrstart  = 0;
            midsrend    = 0;

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToInt(sqlr, &trgsrstart);
            ajSqlcolumnToInt(sqlr, &trgsrend);
            ajSqlcolumnToUint(sqlr, &trgsrid);
            ajSqlcolumnToStr(sqlr, &trgsrname);
            ajSqlcolumnToInt(sqlr, &trgsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &midsrstart);
            ajSqlcolumnToInt(sqlr, &midsrend);

            /* Load the results into the target <-> middle Mapper. */

            ensMapperAddCoordinates(trgmidmapper,
                                    trgsrid,
                                    trgsrstart,
                                    trgsrend,
                                    ori,
                                    ensMapperunitGetObjectidentifier(mu),
                                    midsrstart,
                                    midsrend);

            /*
            ** Add the (new) target Ensembl Sequence Region into the
            ** Ensembl Sequence Region Adaptor cache.
            */

            sr = ensSeqregionNewIni(sra,
                                    trgsrid,
                                    trgcs,
                                    trgsrname,
                                    trgsrlength);

            ensSeqregionadaptorCacheInsert(sra, &sr);

            ensSeqregionDel(&sr);

            /*
            ** Register the target Mapper Unit in the
            ** target Mapper Range Registry.
            */

            ensMapperrangeregistryCheck(trgregistry,
                                        trgsrid,
                                        trgsrstart,
                                        trgsrend,
                                        trgsrstart,
                                        trgsrend,
                                        (AjPList) NULL);

            ajStrDel(&trgsrname);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

        ajStrDel(&statement);

        ensMapperunitDel(&mu);
    }

    ajListFree(&midranges);

    /*
    ** Now that both halves are loaded do stepwise mapping using both of
    ** the loaded Ensembl Mapper objects to load the final source <-> target
    ** Ensembl Mapper.
    */

    assemblymapperadaptorBuildCombinedMapper(ama,
                                             srcranges,
                                             srcmidmapper,
                                             trgmidmapper,
                                             srctrgmapper,
                                             srctype);

    while(ajListPop(srcranges, (void**) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&srcranges);

    ajStrDel(&srctype);
    ajStrDel(&midtype);
    ajStrDel(&trgtype);
    ajStrDel(&asm2cmp);
    ajStrDel(&cmp2asm);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterChainedSpecial ***********************
**
** Registers an AJAX List of previously unregistered Ensembl Mapper Ranges in
** an Ensembl Chained Assembly Mapper.
**
** Optionally, only those Ensembl Mapper Ranges can be registered, which map
** onto a particular Ensembl Sequence Region, if its identifier was specified.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_chained_special
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] source [const AjPStr] Mapping type for the source
** @param [r] srcsrid [ajuint] Source Ensembl Sequence Region identifier
** @param [rE] optsrid [ajuint] Optional Ensembl Sequence Region identifier
** @param [u] ranges [AjPList] AJAX List of Ensembl Mapper Range objects
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterChainedSpecial(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam,
    const AjPStr source,
    ajuint srcsrid,
    ajuint optsrid,
    AjPList ranges)
{
    register ajint i  = 0;
    ajint ori         = 0;

    ajint srcsrstart  = 0;
    ajint srcsrend    = 0;

    ajuint midsrid    = 0;
    ajint midsrstart  = 0;
    ajint midsrend    = 0;
    ajint midsrlength = 0;

    AjBool found = AJFALSE;

    AjPList srcranges = NULL;
    AjPList midranges = NULL;

    AjIList liter         = NULL;
    const AjPList mappath = NULL;

    AjISqlrow sqli       = NULL;
    AjPSqlstatement sqls = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr srctype = NULL;
    AjPStr midtype = NULL;
    AjPStr trgtype = NULL;

    AjPStr midsrname = NULL;

    AjPStr asm2cmp   = NULL;
    AjPStr sqlfmt    = NULL;
    AjPStr statement = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem midcs = NULL;
    EnsPCoordsystem trgcs = NULL;

    EnsPCoordsystem asmcs = NULL;
    EnsPCoordsystem cmpcs = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPMapper srcmidmapper = NULL;
    EnsPMapper srctrgmapper = NULL;

    EnsPMapperrange mr = NULL;
    EnsPMapperrangeregistry srcregistry = NULL;
    EnsPMapperrangeregistry trgregistry = NULL;

    EnsPMapperunit mu = NULL;

    EnsPSeqregion sr = NULL;
    EnsPSeqregion optsr = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterChainedSpecial"))
        ajDebug("ensAssemblymapperadaptorRegisterChainedSpecial\n"
                "  ama %p\n"
                "  cam %p\n"
                "  source '%S'\n"
                "  srcsrid %u\n"
                "  optsrid %u\n"
                "  ranges %p\n",
                ama,
                cam,
                source,
                srcsrid,
                optsrid,
                ranges);

    if(!ama)
        return ajFalse;

    if(!cam)
        return ajFalse;

    if((!source) && (!ajStrGetLen(source)))
        return ajFalse;

    if(!srcsrid)
        return ajFalse;

    if(!ranges)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    if(ajStrMatchC(source, "source"))
    {
        srctype = ajStrNewC("source");

        srcmidmapper = cam->MapperSourceMiddle;

        srccs = cam->CoordsystemSource;

        srcregistry = cam->RegistrySource;

        trgtype = ajStrNewC("target");

        trgcs = cam->CoordsystemTarget;

        trgregistry = cam->RegistryTarget;
    }
    else if(ajStrMatchC(source, "target"))
    {
        srctype = ajStrNewC("target");

        srcmidmapper = cam->MapperTargetMiddle;

        srccs = cam->CoordsystemTarget;

        srcregistry = cam->RegistryTarget;

        trgtype = ajStrNewC("source");

        trgcs = cam->CoordsystemSource;

        trgregistry = cam->RegistrySource;
    }
    else
        ajFatal("ensAssemblymapperadaptorRegisterChainedSpecial invalid "
                "'source' argument '%S' must be 'source' or 'target'.",
                source);

    midtype = ajStrNewC("middle");

    srctrgmapper = cam->MapperSourceTarget;

    midcs = cam->CoordsystemMiddle;

    /*
    ** Obtain the first half of the mappings and load them into the
    ** source <-> middle Mapper. Check for the simple case, where the
    ** Chained Assembly Mapper is short and requires only a one-step
    ** source <-> target Mapper.
    */

    if(midcs)
        mappath = ensCoordsystemadaptorGetMappingpath(csa, srccs, midcs);
    else
    {
        mappath = ensCoordsystemadaptorGetMappingpath(csa, srccs, trgcs);

        srcmidmapper = srctrgmapper;
    }

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblymapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChainedSpecial "
                "unexpected mapping between start and intermediate "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(srccs),
                ensCoordsystemGetVersion(srccs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void**) &asmcs);

    ajListPeekLast(mappath, (void**) &cmpcs);

    asm2cmp = ajStrNewC(
        "SELECT "
        "assembly.cmp_start, "
        "assembly.cmp_end, "
        "assembly.cmp_seq_region_id, "
        "seq_region.name, "
        "seq_region.length, "
        "assembly.ori, "
        "assembly.asm_start, "
        "assembly.asm_end "
        "FROM "
        "assembly, "
        "seq_region "
        "WHERE "
        "assembly.asm_seq_region_id = %u "
        "AND "
        "%d <= assembly.asm_end "
        "AND "
        "%d >= assembly.asm_start "
        "AND "
        "assembly.cmp_seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = %u "
        "AND "
        "assembly.cmp_seq_region_id = %u");

    if(optsrid)
        ensSeqregionadaptorFetchByIdentifier(sra, optsrid, &optsr);

    sqlfmt = asm2cmp;

    /* Run an SQL statement for each previously unregistered Mapper Range. */

    srcranges = ajListNew();

    midranges = ajListNew();

    for(i = 0; i <= 1; i++)
    {

        liter = ajListIterNew(ranges);

        while(!ajListIterDone(liter))
        {
            mr = (EnsPMapperrange) ajListIterGet(liter);

            statement = ajFmtStr(
                ajStrGetPtr(sqlfmt),
                (i) ? optsrid : srcsrid,
                ensMapperrangeGetStart(mr),
                ensMapperrangeGetEnd(mr),
                ensCoordsystemGetIdentifier(ensSeqregionGetCoordsystem(optsr)),
                (i) ? srcsrid : optsrid);

            sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

            sqli = ajSqlrowiterNew(sqls);

            while(!ajSqlrowiterDone(sqli))
            {
                found = ajTrue;

                midsrstart  = 0;
                midsrend    = 0;
                midsrid     = 0;
                midsrname   = ajStrNew();
                midsrlength = 0;
                ori         = 0;
                srcsrstart  = 0;
                srcsrend    = 0;

                sqlr = ajSqlrowiterGet(sqli);

                ajSqlcolumnToInt(sqlr, &midsrstart);
                ajSqlcolumnToInt(sqlr, &midsrend);
                ajSqlcolumnToUint(sqlr, &midsrid);
                ajSqlcolumnToStr(sqlr, &midsrname);
                ajSqlcolumnToInt(sqlr, &midsrlength);
                ajSqlcolumnToInt(sqlr, &ori);
                ajSqlcolumnToInt(sqlr, &srcsrstart);
                ajSqlcolumnToInt(sqlr, &srcsrend);

                /*
                ** Load the results into the source <-> middle Mapper or the
                ** source <-> target Mapper in case the
                ** Chained Assembly Mapper is short.
                */

                if(midcs)
                    ensMapperAddCoordinates(srcmidmapper,
                                            (i) ? optsrid : srcsrid,
                                            srcsrstart,
                                            srcsrend,
                                            ori,
                                            midsrid,
                                            midsrstart,
                                            midsrend);
                else
                {
                    if(ajStrMatchC(source, "source"))
                    {
                        if(i)
                            ensMapperAddCoordinates(srctrgmapper,
                                                    (i) ? optsrid : srcsrid,
                                                    srcsrstart,
                                                    srcsrend,
                                                    ori,
                                                    midsrid,
                                                    midsrstart,
                                                    midsrend);
                        else
                            ensMapperAddCoordinates(srctrgmapper,
                                                    midsrid,
                                                    midsrstart,
                                                    midsrend,
                                                    ori,
                                                    (i) ? optsrid : srcsrid,
                                                    srcsrstart,
                                                    srcsrend);
                    }
                    else
                    {
                        if(i)
                            ensMapperAddCoordinates(srctrgmapper,
                                                    midsrid,
                                                    midsrstart,
                                                    midsrend,
                                                    ori,
                                                    (i) ? optsrid : srcsrid,
                                                    srcsrstart,
                                                    srcsrend);
                        else
                            ensMapperAddCoordinates(srctrgmapper,
                                                    (i) ? optsrid : srcsrid,
                                                    srcsrstart,
                                                    srcsrend,
                                                    ori,
                                                    midsrid,
                                                    midsrstart,
                                                    midsrend);
                    }
                }

                /* Record Mapper Units for the source part. */

                mu = ensMapperunitNewIni((i) ? optsrid : srcsrid,
                                         srcsrstart,
                                         srcsrend);

                ajListPushAppend(srcranges, (void*) mu);

                /* Record Mapper Units for the middle part. */

                mu = ensMapperunitNewIni(midsrid, midsrstart, midsrend);

                ajListPushAppend(midranges, (void*) mu);

                /*
                ** Add the (new) middle Ensembl Sequence Region into the
                ** Ensembl Sequence Region Adaptor cache.
                */

                sr = ensSeqregionNewIni(sra,
                                        midsrid,
                                        (midcs) ? midcs : trgcs,
                                        midsrname,
                                        midsrlength);

                ensSeqregionadaptorCacheInsert(sra, &sr);

                ensSeqregionDel(&sr);

                /*
                ** Register the source Mapper Units in the source
                ** Mapper Registry. The region we actually register may be
                ** smaller or larger than the region we originally intended
                ** to register. Register the intersection of the region so we
                ** do not end up doing extra work later.
                */

                if((srcsrstart < ensMapperrangeGetStart(mr)) ||
                   (srcsrend > ensMapperrangeGetEnd(mr)))
                    ensMapperrangeregistryCheck(
                        srcregistry,
                        (i) ? optsrid : srcsrid,
                        srcsrstart,
                        srcsrend,
                        srcsrstart,
                        srcsrend,
                        (AjPList) NULL);

                ajStrDel(&midsrname);
            }

            ajSqlrowiterDel(&sqli);

            ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

            ajStrDel(&statement);
        }

        ajListIterDel(&liter);
    }

    /*
    ** In the one-step case, the middle Mapper Units are simply registered
    ** in the target Mapper Range Registry.
    */

    if(found && (!midcs))
    {
        /* The source Mapper Units are no longer needed in this case. */

        while(ajListPop(srcranges, (void**) &mu))
            ensMapperunitDel(&mu);

        ajListFree(&srcranges);

        /*
        ** Register the middle Mapper Units in the
        ** target Mapper Range Registry
        ** and that's it for the simple case ...
        */

        while(ajListPop(midranges, (void**) &mu))
        {
            ensMapperrangeregistryCheck(
                trgregistry,
                ensMapperunitGetObjectidentifier(mu),
                ensMapperunitGetStart(mu),
                ensMapperunitGetEnd(mu),
                ensMapperunitGetStart(mu),
                ensMapperunitGetEnd(mu),
                (AjPList) NULL);

            ensMapperunitDel(&mu);
        }

        ajListFree(&midranges);

        ajStrDel(&srctype);
        ajStrDel(&midtype);
        ajStrDel(&trgtype);
        ajStrDel(&asm2cmp);

        return ajTrue;
    }

    while(ajListPop(srcranges, (void**) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&srcranges);

    while(ajListPop(midranges, (void**) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&midranges);

    ajStrDel(&srctype);
    ajStrDel(&midtype);
    ajStrDel(&trgtype);
    ajStrDel(&asm2cmp);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterGenericAll ***************************
**
** Registers all component Ensembl Sequence Regions in an
** Ensembl Generic Assembly Mapper.
**
** This function registers the entire set of mappings between two
** Ensembl Coordinate Systems in an Ensembl Assembly Mapper.
** This will use a lot of memory but will be much more efficient when doing a
** lot of mapping which is spread over the entire genome.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_all
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @return [AjBool] ajTrue on success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterGenericAll(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam)
{
    register ajint i  = 0;

    ajint ori         = 0;
    ajint chunkend    = 0;

    ajuint asmsrid    = 0;
    ajint asmsrstart  = 0;
    ajint asmsrend    = 0;
    ajint asmsrlength = 0;

    ajuint cmpsrid    = 0;
    ajint cmpsrstart  = 0;
    ajint cmpsrend    = 0;
    ajint cmpsrlength = 0;

    AjISqlrow sqli       = NULL;
    AjPSqlstatement sqls = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr asmsrname = NULL;
    AjPStr cmpsrname = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterGenericAll"))
        ajDebug("ensAssemblymapperadaptorRegisterGenericAll\n"
                "  ama %p\n"
                "  gam %p\n",
                ama,
                gam);

    dba = ensAssemblymapperadaptorGetDatabaseadaptor(gam->Adaptor);

    sra = ensRegistryGetSeqregionadaptor(dba);

    statement = ajFmtStr(
        "SELECT "
        "assembly.cmp_start, "
        "assembly.cmp_end, "
        "assembly.cmp_seq_region_id, "
        "cmp_sr.name, "
        "cmp_sr.length, "
        "assembly.ori, "
        "assembly.asm_start, "
        "assembly.asm_end, "
        "assembly.asm_seq_region_id, "
        "asm_sr.name, "
        "asm_sr.length "
        "FROM "
        "assembly, "
        "seq_region asm_sr, "
        "seq_region cmp_sr "
        "WHERE "
        "assembly.cmp_seq_region_id = cmp_sr.seq_region_id "
        "AND "
        "assembly.asm_seq_region_id = asm_sr.seq_region_id "
        "AND "
        "cmp_sr.coord_system_id = %u "
        "AND "
        "asm_sr.coord_system_id = %u",
        ensCoordsystemGetIdentifier(gam->CoordsystemComponent),
        ensCoordsystemGetIdentifier(gam->CoordsystemAssembled));

    sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        cmpsrstart  = 0;
        cmpsrend    = 0;
        cmpsrid     = 0;
        cmpsrname   = ajStrNew();
        cmpsrlength = 0;
        ori         = 0;
        asmsrstart  = 0;
        asmsrend    = 0;
        asmsrid     = 0;
        asmsrname   = ajStrNew();
        asmsrlength = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToInt(sqlr, &cmpsrstart);
        ajSqlcolumnToInt(sqlr, &cmpsrend);
        ajSqlcolumnToUint(sqlr, &cmpsrid);
        ajSqlcolumnToStr(sqlr, &cmpsrname);
        ajSqlcolumnToInt(sqlr, &cmpsrlength);
        ajSqlcolumnToInt(sqlr, &ori);
        ajSqlcolumnToInt(sqlr, &asmsrstart);
        ajSqlcolumnToInt(sqlr, &asmsrend);
        ajSqlcolumnToUint(sqlr, &asmsrid);
        ajSqlcolumnToStr(sqlr, &asmsrname);
        ajSqlcolumnToInt(sqlr, &asmsrlength);

        /* Register the component Sequence Region. */

        ensGenericassemblymapperRegisterComponent(gam, cmpsrid);

        ensMapperAddCoordinates(gam->Mapper,
                                asmsrid,
                                asmsrstart,
                                asmsrend,
                                ori,
                                cmpsrid,
                                cmpsrstart,
                                cmpsrend);

        /*
        ** Register all chunks from the start to the end of the
        ** assembled Sequence Region.
        */

        chunkend = asmsrlength >> assemblymapperChunkFactor;

        for(i = 0; i <= chunkend; i++)
            ensGenericassemblymapperRegisterAssembled(gam, asmsrid, i);

        /*
        ** Add the component Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNewIni(sra,
                                cmpsrid,
                                gam->CoordsystemComponent,
                                cmpsrname,
                                cmpsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        /*
        ** Add the assembled Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNewIni(sra,
                                asmsrid,
                                gam->CoordsystemAssembled,
                                asmsrname,
                                asmsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&asmsrname);
        ajStrDel(&cmpsrname);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterGenericAssembled *********************
**
** Register an assembled Ensembl Sequence Region identifier in an
** Ensembl Generic Assembly Mapper via an Ensembl Assembly Mapper Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_assembled
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [r] regstart [ajint] Start coordinate
** @param [r] regend [ajint] End ccordinate
**
** @return [AjBool] ajTrue on success, ajFalse otherwise.
** @@
** Declares an assembled Ensembl Sequence Region to the
** Ensembl Assembly Mapper.
** This extracts the relevant data from the 'assembly' table and stores it in
** the Ensembl Mapper internal to the Ensembl Assembly Mapper.
** It therefore must be called before any mapping is attempted on that region.
** Otherwise only gaps will be returned. Note that the Ensembl Assembly Mapper
** automatically calls this function when the need arises.
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterGenericAssembled(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajint regstart,
    ajint regend)
{
    register ajint i = 0;

    ajint ori  = 0;
    ajint temp = 0;

    ajint chunkstart = 0;
    ajint chunkend   = 0;

    ajint chunkregionstart = INT_MIN;
    ajint chunkregionend   = INT_MIN;

    ajint asmsrstart = 0;
    ajint asmsrend   = 0;

    ajuint cmpsrid    = 0;
    ajint cmpsrstart  = 0;
    ajint cmpsrend    = 0;
    ajint cmpsrlength = 0;

    AjPList chunkregions = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr cmpsrname = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    EnsPMapperrange chunkregion = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterGenericAssembled"))
        ajDebug("ensAssemblymapperadaptorRegisterGenericAssembled\n"
                "  ama %p\n"
                "  gam %p\n"
                "  asmsrid %u\n"
                "  regstart %d\n"
                "  regend %d\n",
                ama,
                gam,
                asmsrid,
                regstart,
                regend);

    if(!ama)
        return ajFalse;

    if(!gam)
        return ajFalse;

    if(!asmsrid)
        return ajFalse;

    /*
    ** Split up the region to be registered into fixed chunks, allowing
    ** for keeping track of regions that have already been registered.
    ** This works under the assumption that if a small region is requested
    ** it is likely that other requests will be made in the vicinity.
    ** The minimum size registered is the chunksize (2^chunkfactor).
    ** Determine the span of chunks via bitwise shift right operatin,
    ** as a fast and easy integer division.
    ** NOTE: Right bit shift operations on signed integers should give
    ** correct on all "two's complement" machines.
    */

    chunkregions = ajListNew();

    chunkstart = regstart >> assemblymapperChunkFactor;
    chunkend   = regend   >> assemblymapperChunkFactor;

    /*
    ** Inserts have start == end + 1, on boundary condition start chunk
    ** could be less than end chunk.
    */

    if(regstart == (regend + 1))
    {
        temp = chunkstart;

        chunkstart = chunkend;

        chunkend = temp;
    }

    /* Find regions of continuous unregistered chunks. */

    for(i = chunkstart; i <= chunkend; i++)
    {
        if(ensGenericassemblymapperCheckAssembled(gam, asmsrid, i))
        {
            if(chunkregionstart != INT_MIN)
            {
                /* This is the end of an unregistered region. */

                chunkregion = ensMapperrangeNewIni(
                    chunkregionstart << assemblymapperChunkFactor,
                    ((chunkregionend + 1) << assemblymapperChunkFactor) - 1);

                ajListPushAppend(chunkregions, (void*) chunkregion);

                chunkregionstart = INT_MIN;
                chunkregionend   = INT_MIN;
            }
        }
        else
        {
            if(chunkregionstart == INT_MIN)
                chunkregionstart = i;

            chunkregionend = i + 1;

            ensGenericassemblymapperRegisterAssembled(gam, asmsrid, i);
        }
    }

    /* The last part may have been an unregistered region too. */

    if(chunkregionstart != INT_MIN)
    {
        chunkregion = ensMapperrangeNewIni(
            chunkregionstart << assemblymapperChunkFactor,
            ((chunkregionend + 1) << assemblymapperChunkFactor) - 1);

        ajListPushAppend(chunkregions, (void*) chunkregion);
    }

    if(!ajListGetLength(chunkregions))
    {
        ajListFree(&chunkregions);

        return ajTrue;
    }

    /* Keep the Mapper to a reasonable size. */

    if(ensGenericassemblymapperGetCount(gam) > gam->Maximum)
    {
        ensGenericassemblymapperClear(gam);

        while(ajListPop(chunkregions, (void**) &chunkregion))
            ensMapperrangeDel(&chunkregion);

        /* After clearing the cache, everything needs to be re-registered. */

        chunkregion = ensMapperrangeNewIni(
            chunkstart << assemblymapperChunkFactor,
            ((chunkend + 1) << assemblymapperChunkFactor) - 1);

        ajListPushAppend(chunkregions, (void*) chunkregion);

        for(i = chunkstart; i <= chunkend; i++)
            ensGenericassemblymapperRegisterAssembled(gam, asmsrid, i);
    }

    /*
    ** Retrieve the description of how the assembled region is made from
    ** component regions for each of the continuous blocks of unregistered,
    ** chunked regions.
    */

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    while(ajListPop(chunkregions, (void**) &chunkregion))
    {
        statement = ajFmtStr(
            "SELECT "
            "assembly.cmp_start, "
            "assembly.cmp_end, "
            "assembly.cmp_seq_region_id, "
            "seq_region.name, "
            "seq_region.length, "
            "assembly.ori, "
            "assembly.asm_start, "
            "assembly.asm_end "
            "FROM "
            "assembly, "
            "seq_region "
            "WHERE "
            "assembly.asm_seq_region_id = %u "
            "AND "
            "assembly.asm_end >= %d "
            "AND "
            "assembly.asm_start <= %d "
            "AND "
            "assembly.cmp_seq_region_id = seq_region.seq_region_id "
            "AND "
            "seq_region.coord_system_id = %u",
            asmsrid,
            ensMapperrangeGetStart(chunkregion),
            ensMapperrangeGetEnd(chunkregion),
            ensCoordsystemGetIdentifier(gam->CoordsystemComponent));

        sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

        sqli = ajSqlrowiterNew(sqls);

        while(!ajSqlrowiterDone(sqli))
        {
            cmpsrstart = 0;
            cmpsrend = 0;
            cmpsrid = 0;
            cmpsrname = ajStrNew();
            cmpsrlength = 0;
            ori = 0;
            asmsrstart = 0;
            asmsrend = 0;

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToInt(sqlr, &cmpsrstart);
            ajSqlcolumnToInt(sqlr, &cmpsrend);
            ajSqlcolumnToUint(sqlr, &cmpsrid);
            ajSqlcolumnToStr(sqlr, &cmpsrname);
            ajSqlcolumnToInt(sqlr, &cmpsrlength);
            ajSqlcolumnToInt(sqlr, &ori);
            ajSqlcolumnToInt(sqlr, &asmsrstart);
            ajSqlcolumnToInt(sqlr, &asmsrend);

            /*
            ** Only load unregistered Ensembl Sequence Regions of the
            ** Ensembl Mapper and Sequence Regions that map to
            ** multiple locations.
            */

            if(!(ensGenericassemblymapperCheckComponent(gam, cmpsrid)
                 &&
                 (!assemblymapperadaptorHasMultipleMappings(ama, cmpsrid))))
            {
                ensGenericassemblymapperRegisterComponent(gam, cmpsrid);

                ensMapperAddCoordinates(gam->Mapper,
                                        asmsrid,
                                        asmsrstart,
                                        asmsrend,
                                        ori,
                                        cmpsrid,
                                        cmpsrstart,
                                        cmpsrend);

                /*
                ** Add the component Ensembl Sequence Region into the
                ** Ensembl Sequence Region Adaptor cache.
                */

                sr = ensSeqregionNewIni(sra,
                                        cmpsrid,
                                        gam->CoordsystemComponent,
                                        cmpsrname,
                                        cmpsrlength);

                ensSeqregionadaptorCacheInsert(sra, &sr);

                ensSeqregionDel(&sr);
            }

            ajStrDel(&cmpsrname);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

        ajStrDel(&statement);

        ensMapperrangeDel(&chunkregion);
    }

    ajListFree(&chunkregions);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterGenericComponent *********************
**
** Register a component Ensembl Sequence Region identifier in an
** Ensembl Generic Assembly Mapper via an Esnembl Assembly Mapper Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_component
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] cmpsrid [ajuint] Component Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue on success, ajFalse otherwise.
** @@
** Declares a component Ensembl Sequence Region to the Ensembl Assembly Mapper.
** This extracts the relevant data from the 'assembly' table and stores it in
** the Ensembl Mapper internal to the Ensembl Assembly Mapper.
** It therefore must be called before any mapping is attempted on that region.
** Otherwise only gaps will be returned. Note that the Ensembl Assembly Mapper
** automatically calls this function when the need arises.
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterGenericComponent(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    ajuint cmpsrid)
{
    ajuint asmsrid    = 0;
    ajint asmsrstart  = 0;
    ajint asmsrend    = 0;
    ajint asmsrlength = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr asmsrname = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterGenericComponent"))
        ajDebug("ensAssemblymapperadaptorRegisterGenericComponent\n"
                "  ama %p\n"
                "  gam %p\n"
                "  cmpsrid %u\n",
                ama,
                gam,
                cmpsrid);

    if(!ama)
        return ajFalse;

    if(!gam)
        return ajFalse;

    if(!cmpsrid)
        return ajFalse;

    if(ensGenericassemblymapperCheckComponent(gam, cmpsrid) &&
       (!assemblymapperadaptorHasMultipleMappings(ama, cmpsrid)))
        return ajTrue;

    /*
    ** Determine what part of the assembled Sequence Region this component
    ** region makes up.
    */

    statement = ajFmtStr(
        "SELECT "
        "assembly.asm_start, "
        "assembly.asm_end, "
        "assembly.asm_seq_region_id, "
        "seq_region.name, "
        "seq_region.length "
        "FROM "
        "assembly, seq_region "
        "WHERE "
        "assembly.cmp_seq_region_id = %u "
        "AND "
        "assembly.asm_seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = %u",
        cmpsrid,
        ensCoordsystemGetIdentifier(gam->CoordsystemAssembled));

    sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

    if(!ajSqlstatementGetSelectedrows(sqls))
    {
        /* This component is not used in the assembled part, i.e. a gap. */

        ensGenericassemblymapperRegisterComponent(gam, cmpsrid);

        ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

        ajStrDel(&statement);

        return ajTrue;
    }

    if(ajSqlstatementGetSelectedrows(sqls) > 1)
        ajFatal("ensAssemblymapperadaptorRegisterGenericComponent "
                "multiple assembled Sequence Regions for single "
                "component Sequence Region with identifier %u.\n"
                "Remember that multiple mappings require the '#' operator "
                "in the Ensembl Core meta.meta_value entries with "
                "meta.meta_key = 'assembly.mapping'.\n",
                cmpsrid);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        asmsrstart  = 0;
        asmsrend    = 0;
        asmsrid     = 0;
        asmsrname   = ajStrNew();
        asmsrlength = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToInt(sqlr, &asmsrstart);
        ajSqlcolumnToInt(sqlr, &asmsrend);
        ajSqlcolumnToUint(sqlr, &asmsrid);
        ajSqlcolumnToStr(sqlr, &asmsrname);
        ajSqlcolumnToInt(sqlr, &asmsrlength);

        /*
        ** Register the corresponding assembled region. This allows us to
        ** register things in assembled chunks, which allows us to:
        ** (1) Keep track of what assembled regions are registered
        ** (2) Use locality of reference; if they want something in same
        **     general region it will already be registered.
        */

        ensAssemblymapperadaptorRegisterGenericAssembled(ama,
                                                         gam,
                                                         asmsrid,
                                                         asmsrstart,
                                                         asmsrend);

        /*
        ** Add the assembled Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNewIni(sra,
                                asmsrid,
                                gam->CoordsystemAssembled,
                                asmsrname,
                                asmsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&asmsrname);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(ama->Adaptor, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}

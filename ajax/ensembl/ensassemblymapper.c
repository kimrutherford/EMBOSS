/******************************************************************************
** @source Ensembl Assembly Mapper functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.6 $
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
#include "ensslice.h"
#include "ensseqregion.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

const ajuint assemblyMapperMaxPairCount = 1000;

/*
** The Assembly Mapper registers Sequence Regions in chunks of 2 ^ 20,
** which corresponds to approximately 10 ^ 6 or 1 M bps.
*/

const ajuint assemblyMapperChunkFactor = 20;




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPCoordsystemadaptor
ensRegistryGetCoordsystemadaptor(EnsPDatabaseadaptor dba);

extern EnsPSeqregionadaptor
ensRegistryGetSeqregionadaptor(EnsPDatabaseadaptor dba);

static AjBool assemblyMapperadaptorHasMultipleMappings(
    const EnsPAssemblymapperadaptor ama,
    ajuint srid);

static AjBool assemblyMapperadaptorMultipleMappingsInit(
    EnsPAssemblymapperadaptor ama);

static AjBool assemblyMapperadaptorMultipleMappingsExit(
    EnsPAssemblymapperadaptor ama);

static AjBool assemblyMapperadaptorMappingPath(const AjPList mappath);

static AjBool assemblyMapperadaptorBuildCombinedMapper(
    EnsPAssemblymapperadaptor ama,
    AjPList ranges,
    EnsPMapper srcmidmapper,
    EnsPMapper trgmidmapper,
    EnsPMapper srctrgmapper,
    const AjPStr srctype);




/* @filesection ensassemblymapper *********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGenericassemblymapper] Generic Assembly Mapper ***********
**
** Functions for manipulating Ensembl Generic Assembly Mapper objects
**
** @cc Bio::EnsEMBL::AssemblyMapper CVS Revision: 1.42
**
** @nam2rule Genericassemblymapper
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGenericassemblymapper] Ensembl Generic
**                                                 Assembly Mapper
** @argrule Ref object [EnsPGenericassemblymapper] Ensembl Generic
**                                                 Assembly Mapper
**
** @valrule * [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @fcategory new
******************************************************************************/




/* @func ensGenericassemblymapperNew ******************************************
**
** Default constructor for an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::new
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] asmcs [EnsPCoordsystem] Assembled Ensembl Coordinate System
** @param [u] cmpcs [EnsPCoordsystem] Component Ensembl Coordinate System
**
** @return [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPGenericassemblymapper ensGenericassemblymapperNew(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem asmcs,
    EnsPCoordsystem cmpcs)
{
    AjPStr srcname = NULL;
    AjPStr trgname = NULL;

    EnsPGenericassemblymapper gam = NULL;

    if(ajDebugTest("ensGenericassemblymapperNew"))
    {
        ajDebug("ensGenericassemblymapperNew\n"
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

    gam->Adaptor        = ama;
    gam->AsmCoordsystem = ensCoordsystemNewRef(asmcs);
    gam->CmpCoordsystem = ensCoordsystemNewRef(cmpcs);

    gam->AsmRegister =
        ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    gam->CmpRegister =
        ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    gam->Mapper       = ensMapperNew(srcname, trgname, asmcs, cmpcs);
    gam->MaxPairCount = assemblyMapperMaxPairCount;

    gam->Use = 1;

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




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Generic Assembly Mapper.
**
** @fdata [EnsPGenericassemblymapper]
** @fnote None
**
** @nam3rule Del Destroy (free) an EnsPGenericassemblymapper object
**
** @argrule * Pgam [EnsPGenericassemblymapper*] Generic Assembly Mapper
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




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
    void **keyarray1 = NULL;
    void **valarray1 = NULL;
    void **keyarray2 = NULL;
    void **valarray2 = NULL;

    register ajint i = 0;
    register ajint j = 0;

    if(ajDebugTest("ensGenericassemblymapperClear"))
        ajDebug("ensGenericassemblymapperClear\n"
                "  gam %p\n",
                gam);

    if(!gam)
        return ajFalse;

    ajTableToarrayKeysValues(gam->AsmRegister, &keyarray1, &valarray1);

    /*
    ** Free the Assembled Sequence Region identifiers and the second-level
    ** AJAX Tables from the first-level Table. The first-level Table is not
    ** deleted here but in the ensGenericassemblymapperDel function.
    */

    for(i = 0; keyarray1[i]; i++)
    {
        ajTableRemove(gam->AsmRegister, (const void *) keyarray1[i]);
        AJFREE(keyarray1[i]);
        ajTableToarrayKeysValues((AjPTable) valarray1[i],
                                 &keyarray2, &valarray2);

        /*
        ** Free the Assembled Sequence Region chunk identifiers and the
        ** flags from the second-level Table.
        */

        for(j = 0; keyarray2[j]; j++)
        {
            AJFREE(keyarray2[j]);
            AJFREE(valarray2[j]);
        }

        AJFREE(keyarray2);
        AJFREE(valarray2);

        /* Free the second-level AJAX Tables. */

        ajTableFree((AjPTable *) &valarray1[i]);
    }

    AJFREE(keyarray1);
    AJFREE(valarray1);

    ajTableToarrayKeysValues(gam->CmpRegister, &keyarray1, &valarray1);

    /* Free the Component Sequence Region identifiers and the AjBool flags. */

    for(i = 0; keyarray1[i]; i++)
    {
        ajTableRemove(gam->CmpRegister, (const void *) keyarray1[i]);

        AJFREE(keyarray1[i]);
        AJFREE(valarray1[i]);
    }

    AJFREE(keyarray1);
    AJFREE(valarray1);

    ensMapperClear(gam->Mapper);

    return ajTrue;
}




/* @func ensGenericassemblymapperDel ******************************************
**
** Default destructor for an Ensembl Generic Assembly Mapper.
**
** @param [d] Pgam [EnsPGenericassemblymapper*] Ensembl Generic Assembly Mapper
**                                              address
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

    ensCoordsystemDel(&pthis->AsmCoordsystem);

    ensCoordsystemDel(&pthis->CmpCoordsystem);

    ajTableFree(&pthis->AsmRegister);

    ajTableFree(&pthis->CmpRegister);

    ensMapperDel(&pthis->Mapper);

    AJFREE(*Pgam);

    *Pgam = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Generic Assembly Mapper object.
**
** @fdata [EnsPGenericassemblymapper]
** @fnote None
**
** @nam3rule Get Return Generic Assembly Mapper attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Assembly Mapper Adaptor
** @nam4rule GetAssembledCoordsystem Return the assembled Coordinate System
** @nam4rule GetComponentCoordsystem Return the component Coordinate System
** @nam4rule GetMapper Return the Ensembl Mapper
** @nam4rule GetMaxPairCount Return the maximum Ensembl Mapper Pair count
**
** @argrule * gam [const EnsPGenericassemblymapper] Generic Assembly Mapper
**
** @valrule Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @valrule AssembledCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule ComponentCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule Mapper [EnsPMapper] Ensembl Mapper
** @valrule MaxPairCount [ajuint] Maximum Ensembl Mapper Pair count
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
**                                                  Assembly Mapper
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




/* @func ensGenericassemblymapperGetAssembledCoordinateSystem *****************
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

const EnsPCoordsystem ensGenericassemblymapperGetAssembledCoordsystem(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    return gam->AsmCoordsystem;
}




/* @func ensAssemblyGenericMapperGetComponentCoordinateSystem *****************
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

const EnsPCoordsystem ensGenericassemblymapperGetComponentCoordsystem(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return NULL;

    return gam->CmpCoordsystem;
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




/* @func ensGenericassemblymapperGetMaxPairCount ******************************
**
** Get the maximum Ensembl Mapper Pair count element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::max_pair_count
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [ajuint] Maximum Ensembl Mapper Pair count or 0
** @@
******************************************************************************/

ajuint ensGenericassemblymapperGetMaxPairCount(
    const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return 0;

    return gam->MaxPairCount;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Generic Assembly Mapper object.
**
** @fdata [EnsPGenericassemblymapper]
** @fnote None
**
** @nam3rule Set Set one element of a Generic Assembly Mapper
** @nam4rule SetAdaptor Set the Ensembl Assembly Mapper
** @nam4rule SetMaxPairCount Set the maximum Ensembl Mapper Pair count
**
** @argrule * gam [EnsPGenericassemblymapper] Ensembl Generic Assembly
**                                            Mapper object
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
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
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




/* @func ensGenericassemblymapperSetMaxPairCount ******************************
**
** Set the maximum Ensembl Mapper Pair count element of an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::max_pair_count
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] count [ajuint] Maximum Ensembl Mapper Pair count
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperSetMaxPairCount(EnsPGenericassemblymapper gam,
                                               ajuint count)
{
    if(!gam)
        return ajFalse;

    gam->MaxPairCount = count;

    return ajTrue;
}




/* @func ensGenericassemblymapperGetPairCount *********************************
**
** Get the number of Ensembl Mapper Pairs stored in the Ensembl Mapper that is
** associated with an Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::size
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
**
** @return [ajuint] Number of Ensembl Mapper Pairs or 0
** @@
******************************************************************************/

ajuint ensGenericassemblymapperGetPairCount(const EnsPGenericassemblymapper gam)
{
    if(!gam)
        return 0;

    return ensMapperGetPairCount(gam->Mapper);
}




/* @func ensGenericassemblymapperRegisterAssembled ****************************
**
** Register an Assembled Ensembl Sequence Region in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::register_assembled
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [r] chunkid [ajuint] Chunk identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperRegisterAssembled(EnsPGenericassemblymapper gam,
                                                 ajuint asmsrid,
                                                 ajuint chunkid)
{
    ajuint *Pchunkid = NULL;
    ajuint *Pasmsrid = NULL;

    AjBool *Pflag  = NULL;

    AjPTable table = NULL;

    if(ajDebugTest("ensGenericassemblymapperRegisterAssembled"))
        ajDebug("ensGenericassemblymapperRegisterAssembled\n"
                "  gam %p\n"
                "  asmsrid %u\n"
                "  chunkid %u\n",
                gam,
                asmsrid,
                chunkid);

    if(!gam)
        return ajFalse;

    if(!asmsrid)
        return ajFalse;

    table = (AjPTable) ajTableFetch(gam->AsmRegister, (const void *) &asmsrid);

    if(!table)
    {
        AJNEW0(Pasmsrid);

        *Pasmsrid = asmsrid;

        table = ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

        ajTablePut(gam->AsmRegister, (void *) Pasmsrid, (void *) table);
    }

    if(ajTableFetch(table, (const void *) &chunkid))
        return ajTrue;
    else
    {
        AJNEW0(Pchunkid);

        *Pchunkid = chunkid;

        AJNEW0(Pflag);

        *Pflag = ajTrue;

        ajTablePut(table, (void *) Pchunkid, (void *) Pflag);
    }

    return ajTrue;
}




/* @func ensGenericassemblymapperRegisterComponent ****************************
**
** Register a Component Ensembl Sequence Region in an
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
    ajuint *Pcmpsrid = NULL;

    AjBool *Pold = NULL;
    AjBool *Pflag = NULL;

    if(ajDebugTest("ensGenericassemblymapperRegisterComponent"))
        ajDebug("ensGenericassemblymapperRegisterComponent\n"
                "  gam %p\n"
                "  cmpsrid %u\n",
                gam,
                cmpsrid);

    if(!gam)
        return ajFalse;

    if(!cmpsrid)
        return ajFalse;

    Pold = (AjBool *) ajTableFetch(gam->CmpRegister, (const void *) &cmpsrid);

    if(Pold)
        return ajTrue;
    else
    {
        AJNEW0(Pcmpsrid);

        *Pcmpsrid = cmpsrid;

        AJNEW0(Pflag);

        *Pflag = ajTrue;

        ajTablePut(gam->CmpRegister, (void *) Pcmpsrid, (void *) Pflag);
    }

    return ajTrue;
}




/* @func ensGenericassemblymapperHaveRegisteredAssembled **********************
**
** Test whether an Ensembl Generic Assembly Mapper has an
** Ensembl Sequence Region in the cache of assembled sequence regions.
**
** @cc Bio::EnsEMBL::AssemblyMapper::have_registered_assembled
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [r] chunkid [ajuint] Sequence Region chunk identifier
**
** @return [AjBool] ajTrue if the Sequence Region chunk is already registered
**                  in the assembled cache, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperHaveRegisteredAssembled(
    const EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajuint chunkid)
{
    AjPTable table = NULL;

    if(ajDebugTest("ensGenericassemblymapperHaveRegisteredAssembled"))
        ajDebug("ensGenericassemblymapperHaveRegisteredAssembled\n"
                "  gam %p\n"
                "  asmsrid %u\n"
                "  chunkid %u\n",
                gam,
                asmsrid,
                chunkid);

    if(!gam)
        return ajFalse;

    if(!asmsrid)
        return ajFalse;

    /*
    ** Get the second-level AJAX table keyed on chunk identifers from the
    ** first-level table keyed on Ensembl Sequence Region identifiers.
    */

    table = (AjPTable) ajTableFetch(gam->AsmRegister, (const void *) &asmsrid);

    if(table)
    {
        if(ajTableFetch(table, (const void *) &chunkid))
            return ajTrue;
        else
            return ajFalse;
    }
    else
        return ajFalse;
}




/* @func ensGenericassemblymapperHaveRegisteredComponent **********************
**
** Test whether an Ensembl Enseric Assembly Mapper has an
** Ensembl Sequence Region in the cache of component sequence regions.
**
** @cc Bio::EnsEMBL::AssemblyMapper::have_registered_component
** @param [r] gam [const EnsPGenericassemblymapper] Ensembl Generic
**                                                  Assembly Mapper
** @param [r] cmpsrid [ajuint] Component Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue if the Sequence Region chunk is already registered
**                  in the component cache, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperHaveRegisteredComponent(
    const EnsPGenericassemblymapper gam,
    ajuint cmpsrid)
{
    if(ajDebugTest("ensGenericassemblymapperHaveRegisteredComponent"))
        ajDebug("ensGenericassemblymapperHaveRegisteredComponent\n"
                "  gam %p\n"
                "  cmpsrid %u\n",
                gam,
                cmpsrid);

    if(!gam)
        return ajFalse;

    if(!cmpsrid)
        return ajFalse;

    if(ajTableFetch(gam->CmpRegister, (const void *) &cmpsrid))
        return ajTrue;

    return ajFalse;
}




/* @func ensGenericassemblymapperMap ******************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Generic Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::AssemblyMapper::map
** @param [r] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
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

    AjBool *Pflag = NULL;
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
                           gam->CmpCoordsystem))
    {
        Pflag = (AjBool *)
            ajTableFetch(gam->CmpRegister, (const void *) &srid);

        if(!*Pflag)
            ensAssemblymapperadaptorRegisterComponent(gam->Adaptor,
                                                      gam,
                                                      srid);

        type = ajStrNewC("component");
    }
    else if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                                gam->AsmCoordsystem))
    {
        ensAssemblymapperadaptorRegisterAssembled(gam->Adaptor,
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

    ensMapperMapCoordinates(gam->Mapper,
                            srid,
                            srstart,
                            srend,
                            srstrand,
                            type,
                            mrs);

    ajStrDel(&type);

    return ajTrue;
}




/* @func ensChainedassemblymapperFastMap **************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Generic Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::AssemblyMapper::fastmap
** @param [r] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperFastMap(EnsPGenericassemblymapper gam,
                                       const EnsPSeqregion sr,
                                       ajint srstart,
                                       ajint srend,
                                       ajint srstrand,
                                       AjPList mrs)
{
    ajuint srid = 0;

    AjBool *Pflag = NULL;

    AjPStr type = NULL;

    if(ajDebugTest("ensGenericassemblymapperFastMap"))
    {
        ajDebug("ensGenericassemblymapperFastMap\n"
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
                           gam->CmpCoordsystem))
    {
        Pflag = (AjBool *)
            ajTableFetch(gam->CmpRegister, (const void *) &srid);

        if(!*Pflag)
            ensAssemblymapperadaptorRegisterComponent(gam->Adaptor, gam, srid);

        type = ajStrNewC("component");
    }
    else if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                                gam->AsmCoordsystem))
    {
        /*
        ** TODO: The following note from the Perl API is probably not relevant
        ** for the C implementation.
        **
        ** This can be probably be sped up some by only calling registered
        ** assembled if needed.
        */

        ensAssemblymapperadaptorRegisterAssembled(gam->Adaptor,
                                                  gam,
                                                  srid,
                                                  srstart,
                                                  srend);

        type = ajStrNewC("assembled");
    }
    else
        ajFatal("ensGenericassemblymapperFastMap got an "
                "Ensembl Sequence Region, which Coordinate System '%S:%S' "
                "is neiter the assembled nor the component Coordinate System "
                "of this Ensembl Generic Assembly Mapper.\n",
                ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
                ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)));

    ensMapperFastMap(gam->Mapper,
                     srid,
                     srstart,
                     srend,
                     srstrand,
                     type,
                     mrs);

    ajStrDel(&type);

    return ajTrue;
}




/* @func ensGenericassemblymapperRegisterAll **********************************
**
** Register all mappings between two Ensembl Coordinate Systems in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::AssemblyMapper::register_all
** @param [r] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGenericassemblymapperRegisterAll(EnsPGenericassemblymapper gam)
{
    if(!gam)
        return ajFalse;

    return ensAssemblymapperadaptorRegisterAll(gam->Adaptor, gam);
}




/* @datasection [EnsPChainedassemblymapper] Chained Assembly Mapper ***********
**
** Functions for manipulating Ensembl Chained Assembly Mapper objects
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper CVS Revision: 1.14
**
** @nam2rule Chainedassemblymapper
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPChainedassemblymapper] Ensembl Chained
**                                                 Assembly Mapper
** @argrule Ref object [EnsPChainedassemblymapper] Ensembl Chained
**                                                 Assembly Mapper
**
** @valrule * [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @fcategory new
******************************************************************************/




/* @func ensChainedassemblymapperNew ******************************************
**
** Default constructor for an Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::new
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] srccs [EnsPCoordsystem] Source Ensembl Coordinate System
** @param [r] midcs [EnsPCoordsystem] Middle Ensembl Coordinate System
** @param [r] trgcs [EnsPCoordsystem] Target Ensembl Coordinate System
**
** @return [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPChainedassemblymapper ensChainedassemblymapperNew(
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

    debug = ajDebugTest("ensChainedassemblymapperNew");

    if(debug)
    {
        ajDebug("ensChainedassemblymapperNew\n"
                "  ama %p\n"
                "  srccs %p\n"
                "  midcs %p\n"
                "  trgcs %p\n",
                ama,
                midcs,
                trgcs,
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
        ajDebug("ensChainedassemblymapperNew got no middle Coordinate System. "
                "Multiple parts of a component map to more than one "
                "assembled part.\n");

    if(!trgcs)
        return NULL;

    srctype = ajStrNewC("source");
    midtype = ajStrNewC("middle");
    trgtype = ajStrNewC("target");

    AJNEW0(cam);

    cam->Adaptor = ama;

    cam->SourceCoordsystem  = ensCoordsystemNewRef(srccs);
    cam->MiddleCoordsystem  = ensCoordsystemNewRef(midcs);
    cam->TargetCoordsystem  = ensCoordsystemNewRef(trgcs);
    cam->SourceMiddleMapper = ensMapperNew(srctype, midtype, srccs, midcs);
    cam->TargetMiddleMapper = ensMapperNew(trgtype, midtype, trgcs, midcs);
    cam->SourceTargetMapper = ensMapperNew(srctype, trgtype, srccs, trgcs);
    cam->SourceRegistry     = ensMapperrangeregistryNew();
    cam->TargetRegistry     = ensMapperrangeregistryNew();
    cam->MaxPairCount       = assemblyMapperMaxPairCount;

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
** @param [u] gam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
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




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Chained Assembly Mapper.
**
** @fdata [EnsPChainedassemblymapper]
** @fnote None
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




/* @func ensChainedassemblymapperClear ****************************************
**
** Clear all Ensembl Mappers and Ensembl Mapper Range Registries in an
** Ensembl Chained Assembly Mapper.
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

    ensMapperClear(cam->SourceMiddleMapper);
    ensMapperClear(cam->TargetMiddleMapper);
    ensMapperClear(cam->SourceTargetMapper);

    ensMapperrangeregistryClear(cam->SourceRegistry);
    ensMapperrangeregistryClear(cam->TargetRegistry);

    return ajTrue;
}




/* @func ensChainedassemblymapperDel ******************************************
**
** Default destructor for an Ensembl Chained Assembly Mapper.
**
** @param [d] Pcam [EnsPChainedassemblymapper*] Ensembl Chained
**                                              Assembly Mapper
**
** @return [void]
** @@
******************************************************************************/

void ensChainedassemblymapperDel(EnsPChainedassemblymapper *Pcam)
{
    EnsPChainedassemblymapper pthis = NULL;

    if(!Pcam)
        return;

    if(!*Pcam)
        return;

    pthis = *Pcam;

    if(ajDebugTest("ensChainedassemblymapperDel"))
        ajDebug("ensChainedassemblymapperDel\n"
                "  *Pcam %p\n",
                *Pcam);

    pthis->Use--;

    if(pthis->Use)
    {
        *Pcam = NULL;

        return;
    }

    ensCoordsystemDel(&pthis->SourceCoordsystem);

    ensCoordsystemDel(&pthis->MiddleCoordsystem);

    ensCoordsystemDel(&pthis->TargetCoordsystem);

    ensMapperDel(&pthis->SourceMiddleMapper);

    ensMapperDel(&pthis->TargetMiddleMapper);

    ensMapperDel(&pthis->SourceTargetMapper);

    ensMapperrangeregistryDel(&pthis->SourceRegistry);

    ensMapperrangeregistryDel(&pthis->TargetRegistry);

    AJFREE(*Pcam);

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Chained Assembly Mapper object.
**
** @fdata [EnsPChainedassemblymapper]
** @fnote None
**
** @nam3rule Get Return Chained Assembly Mapper attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Assembly Mapper Adaptor
** @nam4rule GetSourceCoordsystem Return the source Ensembl Coordinate System
** @nam4rule GetMiddleCoordsystem Return the middle Ensembl Coordinate System
** @nam4rule GetTargetCoordsystem Return the target Ensembl Coordinate System
** @nam4rule GetSourceMiddleMapper Return the source <-> middle Ensembl Mapper
** @nam4rule GetTargetMiddleMapper Return the target <-> middle Ensembl Mapper
** @nam4rule GetSourceTargetMapper Return the source <-> target Ensembl Mapper
** @nam4rule GetSourceRegistry Return the source Ensembl Range Registry
** @nam4rule GetTargetRegistry Return the target Ensembl Range Registry
** @nam4rule GetMaxPairCount Return the maximum Ensembl Mapper Pair count
**
** @argrule * gam [const EnsPGenericassemblymapper] Generic Assembly Mapper
**
** @valrule Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @valrule SourceCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule MiddleCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule TargetCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule SourceMiddleMapper [EnsPMapper] Ensembl Mapper
** @valrule TargetMiddleMapper [EnsPMapper] Ensembl Mapper
** @valrule SourceTargetMapper [EnsPMapper] Ensembl Mapper
** @valrule SourceRegistry [EnsPMapperrangeregistry] Ensembl Range Registry
** @valrule TargetRegistry [EnsPMapperrangeregistry] Ensembl Range Registry
** @valrule MaxPairCount [ajuint] Maximum Ensembl Mapper Pair count
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




/* @func ensChainedassemblymapperGetSourceCoordsystem *************************
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

const EnsPCoordsystem ensChainedassemblymapperGetSourceCoordsystem(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->SourceCoordsystem;
}




/* @func ensChainedassemblymapperGetMiddleCoordsystem *************************
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

const EnsPCoordsystem ensChainedassemblymapperGetMiddleCoordsystem(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->MiddleCoordsystem;
}




/* @func ensChainedassemblymapperGetTargetCoordsystem *************************
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

const EnsPCoordsystem ensChainedassemblymapperGetTargetCoordsystem(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->TargetCoordsystem;
}




/* @func ensChainedassemblymapperGetSourceMiddleMapper ************************
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

EnsPMapper ensChainedassemblymapperGetSourceMiddleMapper(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->SourceMiddleMapper;
}




/* @func ensChainedassemblymapperGetTargetMiddleMapper ************************
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

EnsPMapper ensChainedassemblymapperGetTargetMiddleMapper(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->TargetMiddleMapper;
}




/* @func ensChainedassemblymapperGetSourceTargetMapper ************************
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

EnsPMapper ensChainedassemblymapperGetSourceTargetMapper(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->SourceTargetMapper;
}




/* @func ensChainedassemblymapperGetSourceRegistry ****************************
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

EnsPMapperrangeregistry ensChainedassemblymapperGetSourceRegistry(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->SourceRegistry;
}




/* @func ensChainedassemblymapperGetTargetRegistry ****************************
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

EnsPMapperrangeregistry ensChainedassemblymapperGetTargetRegistry(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return NULL;

    return cam->TargetRegistry;
}




/* @func ensChainedassemblymapperGetMaxPairCount ******************************
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

ajuint ensChainedassemblymapperGetMaxPairCount(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return 0;

    return cam->MaxPairCount;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Chained Assembly Mapper object.
**
** @fdata [EnsPChainedassemblymapper]
** @fnote None
**
** @nam3rule Set Set one element of a Chained Assembly Mapper
** @nam4rule SetAdaptor Set the Ensembl Assembly Mapper
** @nam4rule SetMaxPairCount Set the maximum Ensembl Mapper Pair count
**
** @argrule * cam [EnsPChainedassemblymapper] Ensembl Chained Assembly
**                                            Mapper object
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
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
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




/* @func ensChainedassemblymapperSetMaxPairCount ******************************
**
** Set the maximum Ensembl Mapper Pair count element of an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::max_pair_count
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] count [ajuint] Maximum Ensembl Mapper Pair count
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
** This sets the number of Ensembl Mapper Pairs allowed in the internal cache.
** This can be used to override the default value (6000) to tune the
** performance and memory usage for certain scenarios.
** A higher value means a bigger cache and thus higher memory requirements.
******************************************************************************/

AjBool ensChainedassemblymapperSetMaxPairCount(EnsPChainedassemblymapper cam,
                                               ajuint count)
{
    if(!cam)
        return ajFalse;

    if(!count)
        return ajFalse;

    cam->MaxPairCount = count;

    return ajTrue;
}




/* @func ensChainedassemblymapperGetPairCount *********************************
**
** Get the number of Ensembl Mapper Pairs in an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::size
** @param [r] cam [const EnsPChainedassemblymapper] Ensembl Chained
**                                                  Assembly Mapper
**
** @return [ajuint] Number of Ensembl Mapper Pairs or 0
** @@
******************************************************************************/

ajuint ensChainedassemblymapperGetPairCount(
    const EnsPChainedassemblymapper cam)
{
    if(!cam)
        return 0;

    return (ensMapperGetPairCount(cam->SourceMiddleMapper) +
            ensMapperGetPairCount(cam->TargetMiddleMapper) +
            ensMapperGetPairCount(cam->SourceTargetMapper));
}




/* @func ensChainedassemblymapperMap ******************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Chained Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::map
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::fastmap
** @param [r] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [r] optsr [const EnsPSeqregion] Optional Ensembl Sequence Region
** @param [r] fastmap [AjBool] Fast-mapping attribute
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
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
                           cam->SourceCoordsystem))
    {
        type = ajStrNewC("source");

        registry = cam->SourceRegistry;
    }
    else if(ensCoordsystemMatch(ensSeqregionGetCoordsystem(sr),
                                cam->TargetCoordsystem))
    {
        type = ajStrNewC("target");

        registry = cam->TargetRegistry;
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
        regstart = ((srend >> assemblyMapperChunkFactor)
                    << assemblyMapperChunkFactor);

        regend = (((srstart >> assemblyMapperChunkFactor) + 1)
                  << assemblyMapperChunkFactor) - 1 ;
    }
    else
    {
        regstart = ((srstart >> assemblyMapperChunkFactor)
                    << assemblyMapperChunkFactor);

        regend = (((srend >> assemblyMapperChunkFactor) + 1)
                  << assemblyMapperChunkFactor) - 1 ;
    }

    /*
    ** Get a list of Mapper Ranges in the requested region that have not been
    ** registered, and register them at the same time.
    */

    ranges = ajListNew();

    if(isinsert)
        ensMapperrangeregistryCheckAndRegister(registry,
                                               srid,
                                               srend,
                                               srstart,
                                               regstart,
                                               regend,
                                               ranges);
    else
        ensMapperrangeregistryCheckAndRegister(registry,
                                               srid,
                                               srstart,
                                               srend,
                                               regstart,
                                               regend,
                                               ranges);

    if(ajListGetLength(ranges))
    {
        if(ensChainedassemblymapperGetPairCount(cam) > cam->MaxPairCount)
        {
            /*
            ** If the Chained Assembly Mapper contains already more
            ** Ensembl Mapper Pairs than allowed, clear the internal cache and
            ** re-register the lot. The AJAX List of Ensembl Mapper Ranges
            ** needs clearing as well.
            */

            ensChainedassemblymapperClear(cam);

            while(ajListPop(ranges, (void **) &mr))
                ensMapperrangeDel(&mr);

            if(isinsert)
                ensMapperrangeregistryCheckAndRegister(registry,
                                                       srid,
                                                       srend,
                                                       srstart,
                                                       regstart,
                                                       regend,
                                                       ranges);
            else
                ensMapperrangeregistryCheckAndRegister(registry,
                                                       srid,
                                                       srstart,
                                                       srend,
                                                       regstart,
                                                       regend,
                                                       ranges);
        }

        ensAssemblymapperadaptorRegisterChained(cam->Adaptor,
                                                cam,
                                                type,
                                                srid,
                                                optsrid,
                                                ranges);
    }

    if(fastmap)
        ensMapperFastMap(cam->SourceTargetMapper,
                         srid,
                         srstart,
                         srend,
                         srstrand,
                         type,
                         mrs);
    else
        ensMapperMapCoordinates(cam->SourceTargetMapper,
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

    while(ajListPop(ranges, (void **) &mr))
        ensMapperrangeDel(&mr);

    ajListFree(&ranges);

    ajStrDel(&type);

    return ajTrue;
}




/* @func ensChainedassemblymapperRegisterAll **********************************
**
** Register all mappings between two Ensembl Coordinate Systems in an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::ChainedAssemblyMapper::register_all
** @param [r] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensChainedassemblymapperRegisterAll(EnsPChainedassemblymapper cam)
{
    if(!cam)
        return ajFalse;

    return ensAssemblymapperadaptorRegisterAllChained(cam->Adaptor, cam);
}




/* @datasection [EnsPToplevelassemblymapper] Top-Level Assembly Mapper ********
**
** Functions for manipulating Ensembl Top-Level Assembly Mapper objects
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper CVS Revision: 1.7
**
** @nam2rule Toplevelassemblymapper
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                                  Assembly Mapper
** @argrule Ref object [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                                  Assembly Mapper
**
** @valrule * [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly Mapper
**
** @fcategory new
******************************************************************************/




/* @func ensToplevelassemblymapperNew *****************************************
**
** Default constructor for an Ensembl TopLevel Assembly Mapper.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::new
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cs1 [EnsPCoordsystem] Top-level Ensembl Coordinate System
** @param [u] cs2 [EnsPCoordsystem] Other Ensembl Coordinate System
**
** @return [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly Mapper
**                                      or NULL
** @@
******************************************************************************/

EnsPToplevelassemblymapper ensToplevelassemblymapperNew(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2)
{
    EnsPCoordsystemadaptor csa = NULL;

    EnsPToplevelassemblymapper tlam = NULL;

    if(!ama)
        return NULL;

    if(!cs1)
        return NULL;

    if(!cs2)
        return NULL;

    if(!ensCoordsystemIsTopLevel(cs1))
        ajFatal("ensTopLevelMapperNew cs1 '%S:%S' should be a top-level "
                "Coordinate System.",
                ensCoordsystemGetName(cs1),
                ensCoordsystemGetVersion(cs1));

    if(ensCoordsystemIsTopLevel(cs2))
        ajFatal("ensTopLevelMapperNew cs2 '%S:%S' should be no top-level "
                "Coordinate System.",
                ensCoordsystemGetName(cs2),
                ensCoordsystemGetVersion(cs2));

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    AJNEW0(tlam);

    tlam->Adaptor = ama;

    tlam->Coordsystems = ajListNew();

    ensCoordsystemadaptorFetchAll(csa, tlam->Coordsystems);

    tlam->TopLevelCoordsystem = ensCoordsystemNewRef(cs1);
    tlam->OtherCoordsystem    = ensCoordsystemNewRef(cs2);

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
** memory allocated for the Ensembl Top-Level Assembly Mapper.
**
** @fdata [EnsPToplevelassemblymapper]
** @fnote None
**
** @nam3rule Del Destroy (free) an EnsPToplevelassemblymapper object
**
** @argrule * Ptlam [EnsPToplevelassemblymapper*] Top-Level Assembly Mapper
**                                                object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensToplevelassemblymapperDel *****************************************
**
** Default destructor for an Ensembl TopLevel Assembly Mapper.
**
** @param [d] Ptlam [EnsPToplevelassemblymapper*]  Ensembl Top-Level Assembly
**                                                 Mapper address
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

    ensCoordsystemDel(&pthis->TopLevelCoordsystem);

    ensCoordsystemDel(&pthis->OtherCoordsystem);

    /*
    ** Delete the Ensembl Coordinate System list elements before deleting
    ** the AJAX List.
    */

    while(ajListPop(pthis->Coordsystems, (void **) &cs))
        ensCoordsystemDel(&cs);

    ajListFree(&pthis->Coordsystems);

    AJFREE(*Ptlam);

    *Ptlam = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Top-Level Assembly Mapper object.
**
** @fdata [EnsPToplevelassemblymapper]
** @fnote None
**
** @nam3rule Get Return Top-Level Assembly Mapper attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Assembly Mapper Adaptor
** @nam4rule GetAssembledCoordsystem Return the assembled Coordinate System
** @nam4rule GetComponentCoordsystem Return the component Coordinate System
**
** @argrule * tlam [const EnsPToplevelassemblymapper] Top-Level Assembly Mapper
**
** @valrule Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @valrule AssembledCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule ComponentCoordsystem [EnsPCoordsystem] Ensembl Coordinate System
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
** @return [const EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**                                           or NULL
** @@
******************************************************************************/

const EnsPAssemblymapperadaptor ensToplevelassemblymapperGetAdaptor(
    const EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    return tlam->Adaptor;
}




/* @func ensToplevelassemblymapperGetAssembledCoordsystem *********************
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

const EnsPCoordsystem ensToplevelassemblymapperGetAssembledCoordsystem(
    const EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    return tlam->TopLevelCoordsystem;
}




/* @func ensToplevelassemblymapperGetComponentCoordsystem *********************
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

const EnsPCoordsystem ensToplevelassemblymapperGetComponentCoordsystem(
    const EnsPToplevelassemblymapper tlam)
{
    if(!tlam)
        return NULL;

    return tlam->OtherCoordsystem;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Top-Level Assembly Mapper object.
**
** @fdata [EnsPToplevelassemblymapper]
** @fnote None
**
** @nam3rule Set Set one element of a Top-Level Assembly Mapper
** @nam4rule SetAdaptor Set the Ensembl Assembly Mapper
**
** @argrule * tlam [EnsPToplevelassemblymapper] Ensembl Top-Level Assembly
**                                              Mapper object
**
** @valrule * [AjBool]
**
** @fcategory modify
******************************************************************************/




/* @func ensToplevelassemblymapperSetAdaptor **********************************
**
** Set the Ensembl Assembly Mapper Adaptor element of an
** Ensembl Top-Level Assembly Mapper.
**
** @param [u] cam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                             Assembly Mapper
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly
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




/* @func ensToplevelassemblymapperMap *****************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Top-Level Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::map
** @cc Bio::EnsEMBL::TopLevelAssemblyMapper::fastmap
** @param [r] tlam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                              Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Sequence Region
** @param [r] srstart [ajint] Sequence Region start coordinate
** @param [r] srend [ajint] Sequence Region end coordinate
** @param [r] srstrand [ajint] Sequence Region strand information
** @param [r] fastmap [AjBool] Fast-mapping attribute
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
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
                "  fastmap %B"
                "  mrs %p",
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
                            tlam->OtherCoordsystem))
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
           ensCoordsystemGetRank(tlam->OtherCoordsystem))
            break;

        /* Check if a mapping path even exists to this Coordinate System. */

        mappath = ensCoordsystemadaptorGetMappingPath(
            csa,
            tstcs,
            tlam->OtherCoordsystem);

        if(ajListGetLength(mappath))
        {
            /*
            ** Try to map to this Coordinate System.
            ** If we get back any coordinates then it is our 'toplevel' that
            ** we were looking for.
            */

            am = ensAssemblymapperadaptorFetchByCoordsystems(
                tlam->Adaptor,
                tlam->OtherCoordsystem,
                tstcs);

            if(fastmap)
            {
                ensAssemblymapperFastMap(am,
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
                ensAssemblymapperMap(am,
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

                    ajListPeekFirst(mrs, (void **) &mr);

                    if(ensMapperresultGetType(mr) != ensEMapperresultGap)
                    {
                        done = ajTrue;

                        break;
                    }

                    while(ajListPop(mrs, (void **) &mr))
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

    mr = MENSMAPPERCOORDINATENEW(srid,
                                 srstart,
                                 srend,
                                 srstrand,
                                 tlam->OtherCoordsystem);

    ajListPushAppend(mrs, (void *) mr);

    return ajTrue;
}




/* @datasection [EnsPAssemblymapper] Assembly Mapper **************************
**
** Functions for manipulating Ensembl Assembly Mapper objects
**
** @nam2rule Assemblymapper
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPAssemblymapper] Ensembl Assembly Mapper
** @argrule Ref object [EnsPAssemblymapper] Ensembl Assembly Mapper
**
** @valrule * [EnsPAssemblymapper] Ensembl Assembly Mapper
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblymapperNew *************************************************
**
** Default constructor for an Ensembl Assembly Mapper.
**
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] gam [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                             Assembly Mapper
**
** @return [EnsPAssemblymapper] Ensembl Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPAssemblymapper ensAssemblymapperNew(EnsPAssemblymapperadaptor ama,
                                        EnsPGenericassemblymapper gam,
                                        EnsPChainedassemblymapper cam,
                                        EnsPToplevelassemblymapper tlam)
{
    EnsPAssemblymapper am = NULL;

    if(!ama)
        return NULL;

    if(!(gam || cam || tlam))
        return NULL;

    if(gam && (cam || tlam))
        ajFatal("ensAssemblymapperNew ");

    if(cam && (gam || tlam))
        ajFatal("ensAssemblymapperNew ");

    if(tlam && (gam || cam))
        ajFatal("ensAssemblymapperNew ");

    AJNEW0(am);

    am->Adaptor = ama;

    am->Generic  = ensGenericassemblymapperNewRef(gam);
    am->Chained  = ensChainedassemblymapperNewRef(cam);
    am->TopLevel = ensToplevelassemblymapperNewRef(tlam);

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
** memory allocated for the Ensembl Assembly Mapper.
**
** @fdata [EnsPAssemblymapper]
** @fnote None
**
** @nam3rule Del Destroy (free) an EnsPAssemblymapper object
**
** @argrule * Pam [EnsPAssemblymapper*] Assembly Mapper object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblymapperDel *************************************************
**
** Default destructor for an Ensembl Assembly Mapper.
**
** @param [d] Pam [EnsPAssemblymapper*] Ensembl Assembly Mapper address
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

    pthis = *Pam;

    if(ajDebugTest("ensAssemblymapperDel"))
        ajDebug("ensAssemblymapperDel\n"
                "  *Pam %p\n",
                *Pam);

    pthis->Use--;

    if(pthis->Use)
    {
        *Pam = NULL;

        return;
    }

    ensGenericassemblymapperDel(&pthis->Generic);

    ensChainedassemblymapperDel(&pthis->Chained);

    ensToplevelassemblymapperDel(&pthis->TopLevel);

    AJFREE(*Pam);

    *Pam = NULL;

    return;
}




/* @func ensAssemblymapperMap *************************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @param [r] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region start coordinate
** @param [r] srend [ajint] Sequence Region end coordinate
** @param [r] srstrand [ajint] Sequence Region strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperMap(EnsPAssemblymapper am,
                            const EnsPSeqregion sr,
                            ajint srstart,
                            ajint srend,
                            ajint srstrand,
                            AjPList mrs)
{
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

    if(am->TopLevel)
        ensToplevelassemblymapperMap(am->TopLevel,
                                     sr,
                                     srstart,
                                     srend,
                                     srstrand,
                                     ajFalse,
                                     mrs);

    return ajTrue;
}




/* @func ensAssemblymapperFastMap *********************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @param [r] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region Start coordinate
** @param [r] srend [ajint] Sequence Region End coordinate
** @param [r] srstrand [ajint] Sequence Region Strand information
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperFastMap(EnsPAssemblymapper am,
                                const EnsPSeqregion sr,
                                ajint srstart,
                                ajint srend,
                                ajint srstrand,
                                AjPList mrs)
{
    if(!am)
        return ajFalse;

    if(am->Generic)
        ensGenericassemblymapperFastMap(am->Generic,
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

    if(am->TopLevel)
        ensToplevelassemblymapperMap(am->TopLevel,
                                     sr,
                                     srstart,
                                     srend,
                                     srstrand,
                                     ajTrue,
                                     mrs);

    return ajTrue;
}




/* @func ensAssemblymapperMapToSeqregion **************************************
**
** Transform coordinates from one Ensembl Coordinate System to another by means
** of an Ensembl Assembly Mapper.
**
** The caller is responsible for deleting the
** Ensembl Mapper Results before deleting the AJAX List.
**
** @param [r] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] srstart [ajint] Sequence Region start coordinate
** @param [r] srend [ajint] Sequence Region end coordinate
** @param [r] srstrand [ajint] Sequence Region strand information
** @param [r] optsr [const EnsPSeqregion] Optional Ensembl Sequence Region
** @param [u] mrs [AjPList] AJAX List of Ensembl Mapper Results
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

    if(am->TopLevel)
        ensToplevelassemblymapperMap(am->TopLevel,
                                     sr,
                                     srstart,
                                     srend,
                                     srstrand,
                                     ajFalse,
                                     mrs);

    return ajTrue;
}




/* @datasection [EnsPAssemblymapperadaptor] Assembly Mapper Adaptor ***********
**
** Functions for manipulating Ensembl Assembly Mapper Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor CVS Revision: 1.57
**
** @nam2rule Assemblymapperadaptor
**
******************************************************************************/




/* @funcstatic assemblyMapperadaptorHasMultipleMappings ***********************
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

static AjBool assemblyMapperadaptorHasMultipleMappings(
    const EnsPAssemblymapperadaptor ama,
    ajuint srid)
{
    if(!ama)
        return ajFalse;

    if(!srid)
        return ajFalse;

    if(!ama->MultipleMappings)
        ajFatal("assemblyMapperadaptorHasMultipleMappings AJAX Table for "
                "multiple Sequence Region mappings has not been "
                "initialised!\n");

    if(ajTableFetch(ama->MultipleMappings, (const void *) &srid))
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPAssemblymapperadaptor] Ensembl Assembly
**                                                 Mapper Adaptor
** @argrule Ref object [EnsPAssemblymapperadaptor] Ensembl Assembly
**                                                 Mapper Adaptor
**
** @valrule * [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @fcategory new
******************************************************************************/




/* @funcstatic assemblyMapperadaptorMultipleMappingsInit **********************
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

static AjBool assemblyMapperadaptorMultipleMappingsInit(
    EnsPAssemblymapperadaptor ama)
{
    ajuint identifier = 0;

    ajuint *Pidentifier = NULL;

    AjBool *Pbool = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(ajDebugTest("assemblyMapperadaptorMultipleMappingsInit"))
        ajDebug("assemblyMapperadaptorMultipleMappingsInit\n"
                "  ama %p\n",
                ama);

    if(!ama)
        return ajFalse;

    if(ama->MultipleMappings)
        return ajTrue;

    ama->MultipleMappings = MENSTABLEUINTNEW(0);

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

        if(ajTableFetch(ama->MultipleMappings, (const void *) Pidentifier))
        {
            ajWarn("assemblyMapperadaptorMultipleMappingsInit already "
                   "cached Ensembl Sequence region with identifier %u.\n",
                   *Pidentifier);

            AJFREE(Pidentifier);
            AJFREE(Pbool);
        }
        else
            ajTablePut(ama->MultipleMappings,
                       (void *) Pidentifier,
                       (void *) Pbool);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorNew ******************************************
**
** Default constructor for an Ensembl Assembly Mapper Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor or NULL
** @@
******************************************************************************/

EnsPAssemblymapperadaptor ensAssemblymapperadaptorNew(EnsPDatabaseadaptor dba)
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

    ama->AsmMapperCache = ajTablestrNewLen(0);

    assemblyMapperadaptorMultipleMappingsInit(ama);

    return ama;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Assembly Mapper Adaptor.
**
** @fdata [EnsPAssemblymapperadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an EnsPAssemblymapperadaptor object
**
** @argrule * Pama [EnsPAssemblymapperadaptor*] Assembly Mapper Adaptor
**                                              object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic assemblyMapperadaptorMultipleMappingsExit **********************
**
** Clears the Ensembl Assembly Mapper Adaptor-internal cache of
** Ensembl Sequence Regions that map to more than one location.
**
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyMapperadaptorMultipleMappingsExit(
    EnsPAssemblymapperadaptor ama)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajint i = 0;

    if(ajDebugTest("assemblyMapperadaptorMultipleMappingsExit"))
        ajDebug("ensAsemblyMapperadaptorMultipleMappingsExit\n"
                "  ama %p\n",
                ama);

    if(!ama)
        return ajFalse;

    /* Clear the Multiple Mappings AJAX Table. */

    if(ama->MultipleMappings)
    {
        ajTableToarrayKeysValues(ama->MultipleMappings, &keyarray, &valarray);

        /* Free the ajuint key and the AjBool value data. */

        for(i = 0; keyarray[i]; i++)
        {
            AJFREE(keyarray[i]);
            AJFREE(valarray[i]);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    return ajTrue;
}




/* @func ensAssemblymapperadaptorCacheClear ***********************************
**
** Clears the Ensembl Assembly Mapper Adaptor-internal cache of
** Ensembl Assembly Mappers.
**
** @param [u] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorCacheClear(EnsPAssemblymapperadaptor ama)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajint i = 0;

    if(ajDebugTest("ensAssemblymapperadaptorCacheClear"))
        ajDebug("ensAssemblymapperadaptorCacheClear\n"
                "  ama %p\n",
                ama);

    if(!ama)
        return ajFalse;

    /* Delete the Assembly Mapper Cache. */

    if(ama->AsmMapperCache)
    {
        ajTableToarrayKeysValues(ama->AsmMapperCache, &keyarray, &valarray);

        /* Free the AJAX String key and Ensembl Assembly Mapper value data. */

        for(i = 0; keyarray[i]; i++)
        {
            ajTableRemove(ama->MultipleMappings, (const void *) keyarray[i]);

            ajStrDel((AjPStr *) &keyarray[i]);

            ensAssemblymapperDel((EnsPAssemblymapper *) &valarray[i]);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    return ajTrue;
}




/* @func ensAssemblymapperadaptorDel ******************************************
**
** Default destructor for an Ensembl Assembly Mapper Adaptor.
**
** @param [d] Pama [EnsPAssemblymapperadaptor*] Ensembl Assembly Mapper
**                                              Adaptor address
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

    ensAssemblymapperadaptorCacheClear(pthis);

    ajTableFree(&pthis->AsmMapperCache);

    /* Clear the Multiple Mappings AJAX Table. */

    assemblyMapperadaptorMultipleMappingsExit(pthis);

    ajTableFree(&pthis->MultipleMappings);

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
** @fnote None
**
** @nam3rule Get Return Assembly Mapper Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * ama [const EnsPAssemblymapperadaptor] Ensembl Assembly
**                                                  Mapper Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
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
** Functions for retrieving Ensembl Assembly Mapper objects from an
** Ensembl Core database.
**
** @fdata [EnsPAssemblymapperadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Assembly Mapper object(s)
** @nam4rule FetchAll Retrieve all Ensembl Assembly Mapper objects
** @nam5rule FetchAllBy Retrieve all Ensembl Assembly Mapper objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Assembly Mapper object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPAssemblymapperadaptor] Ensembl Assembly
**                                                      Mapper Adaptor
** @argrule FetchAll amlist [AjPList] AJAX List of Ensembl Assembly Mappers
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
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] cs1 [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] cs2 [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPAssemblymapper] Ensembl Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPAssemblymapper ensAssemblymapperadaptorFetchByCoordsystems(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2)
{
    const AjPList mappath = NULL;
    AjIList iter          = NULL;

    AjPStr keystr = NULL;

    EnsPAssemblymapper am = NULL;

    EnsPChainedassemblymapper cam = NULL;

    EnsPGenericassemblymapper gam = NULL;

    EnsPToplevelassemblymapper tlam = NULL;

    EnsPCoordsystem srccs = NULL;
    EnsPCoordsystem midcs = NULL;
    EnsPCoordsystem trgcs = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorFetchByCoordsystems"))
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems\n"
                "  ama %p\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                ama,
                cs1,
                cs2);

        ensCoordsystemTrace(cs1, 1);
        ensCoordsystemTrace(cs2, 1);
    }

    if(!ama)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires an "
                "Ensembl Assembly Mapper Adaptor.\n");

        return NULL;
    }

    if(!cs1)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires "
                "Ensembl Coordinate System one.\n");

        return NULL;
    }

    if(!cs2)
    {
        ajDebug("ensAssemblymapperadaptorFetchByCoordsystems requires "
                "Ensembl Coordinate System two.\n");

        return NULL;
    }

    /*
    ** NOTE: Coordinate mapping between Sequence Regions with multiple
    ** locations within the same Coordinate System are now possible.
    */

    if(ensCoordsystemIsTopLevel(cs1))
    {
        tlam = ensToplevelassemblymapperNew(ama, cs1, cs2);

        am = ensAssemblymapperNew(ama,
                                  (EnsPGenericassemblymapper) NULL,
                                  (EnsPChainedassemblymapper) NULL,
                                  tlam);

        ensToplevelassemblymapperDel(&tlam);

        return am;
    }

    if(ensCoordsystemIsTopLevel(cs2))
    {
        tlam = ensToplevelassemblymapperNew(ama, cs2, cs1);

        am = ensAssemblymapperNew(ama,
                                  (EnsPGenericassemblymapper) NULL,
                                  (EnsPChainedassemblymapper) NULL,
                                  tlam);

        ensToplevelassemblymapperDel(&tlam);

        return am;
    }

    csa = ensRegistryGetCoordsystemadaptor(ama->Adaptor);

    mappath = ensCoordsystemadaptorGetMappingPath(csa, cs1, cs2);

    if(!ajListGetLength(mappath))
    {
        ajWarn("ensAssemblymapperadaptorFetchByCoordsystems "
               "got no mapping path between Coordinate Systems "
               "'%S:%S' and '%S:%S'.\n",
               ensCoordsystemGetName(cs1),
               ensCoordsystemGetVersion(cs1),
               ensCoordsystemGetName(cs2),
               ensCoordsystemGetVersion(cs2));

        return NULL;
    }

    keystr = ajStrNew();

    iter = ajListIterNewread(mappath);

    while(!ajListIterDone(iter))
    {
        srccs = (EnsPCoordsystem) ajListIterGet(iter);

        ajFmtPrintAppS(&keystr, "%u:", ensCoordsystemGetIdentifier(srccs));
    }

    ajListIterDel(&iter);

    am = (EnsPAssemblymapper)
        ajTableFetch(ama->AsmMapperCache, (const void *) keystr);

    if(!am)
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
            ajListPeekNumber(mappath, 0, (void **) &srccs);
            ajListPeekNumber(mappath, 1, (void **) &trgcs);

            gam = ensGenericassemblymapperNew(ama, srccs, trgcs);

            am = ensAssemblymapperNew(ama,
                                      gam,
                                      (EnsPChainedassemblymapper) NULL,
                                      (EnsPToplevelassemblymapper) NULL);

            /*
            ** The Assembly Mapper reference counter is incremented with
            ** the return statement.
            */

            ajTablePut(ama->AsmMapperCache,
                       (void *) ajStrNewS(keystr),
                       (void *) am);

            /*
            ** If you want multiple pieces on two Ensembl Sequence Regions
            ** to map to each other you need to make an 'assembly.mapping'
            ** entry in the 'meta' table of the Ensembl Core database, that
            ** is seperated with a '#' character instead of a '|' character.
            */

            ensGenericassemblymapperDel(&gam);
        }
        else if(ajListGetLength(mappath) == 3)
        {
            ajListPeekNumber(mappath, 0, (void **) &srccs);
            ajListPeekNumber(mappath, 1, (void **) &midcs);
            ajListPeekNumber(mappath, 2, (void **) &trgcs);

            cam = ensChainedassemblymapperNew(ama, srccs, midcs, trgcs);

            am = ensAssemblymapperNew(ama,
                                      (EnsPGenericassemblymapper) NULL,
                                      cam,
                                      (EnsPToplevelassemblymapper) NULL);

            /*
            ** The Assembly Mapper reference counter is incremented with
            ** the return statement.
            */

            ajTablePut(ama->AsmMapperCache,
                       (void *) ajStrNewS(keystr),
                       (void *) am);

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

            /*
            ** Registering this Chained Assembly Mapper twice requires
            ** increasing its reference counter.
            */

            ajTablePut(ama->AsmMapperCache,
                       (void *) ajStrNewS(keystr),
                       (void *) ensAssemblymapperNewRef(am));

            /*
            ** In multi-step mapping it is possible to get requests with the
            ** coordinate system ordering reversed since both mapping
            ** directions cache on both orderings just in case
            ** e.g.
            ** chromosome <-> contig <-> clone
            ** and
            ** clone <-> contig <-> chromosome
            */

            ensChainedassemblymapperDel(&cam);
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

    return ensAssemblymapperNewRef(am);
}




/* @func ensAssemblymapperadaptorFetchBySlices ********************************
**
** Fetch an Ensembl Assembly Mapper via an Ensembl Slice pair.
**
** The caller is responsible for deleting the Ensembl Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::???
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] slice1 [EnsPSlice] Ensembl Slice
** @param [r] slice2 [EnsPSlice] Ensembl Slice
**
** @return [EnsPAssemblymapper] Ensembl Assembly Mapper or NULL
** @@
******************************************************************************/

EnsPAssemblymapper ensAssemblymapperadaptorFetchBySlices(
    EnsPAssemblymapperadaptor ama,
    EnsPSlice slice1,
    EnsPSlice slice2)
{
    if(!slice1)
        return NULL;

    if(!slice2)
        return NULL;

    return ensAssemblymapperadaptorFetchByCoordsystems(
        ama,
        ensSliceGetCoordsystem(slice1),
        ensSliceGetCoordsystem(slice2));
}




/* @func ensAssemblymapperadaptorSeqregionIdsToNames **************************
**
** Converts database-internal Ensembl Sequence Region identifiers to their
** names.
**
** The caller is responsible for deleting the
** AJAX Strings before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::seq_ids_to_regions
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::_seq_region_id_to_name
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] identifiers [const AjPList] AJAX List of AJAX unsigned integers
**                                        of Ensembl Sequence Region
**                                        identifiers
** @param [u] names [AjPList] AJAX List of AJAX Strings of
**                            Ensembl Sequence Region names
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
**
** FIXME: This function is obsolete in the EMBOSS implementation, since the
** Ensembl Sequence Region records its identifier internally.
** TODO: Suggest this to the Ensembl Core team.
******************************************************************************/

AjBool ensAssemblymapperadaptorSeqregionIdsToNames(
    EnsPAssemblymapperadaptor ama,
    const AjPList identifiers,
    AjPList names)
{
    ajuint *Pid = NULL;

    AjIList iter = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(!ama)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    if(!names)
        return ajFalse;

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    iter = ajListIterNewread(identifiers);

    while(!ajListIterDone(iter))
    {
        Pid = (ajuint *) ajListIterGet(iter);

        ensSeqregionadaptorFetchByIdentifier(sra, *Pid, &sr);

        if(sr)
            ajListPushAppend(names,
                             (void *) ajStrNewS(ensSeqregionGetName(sr)));
        else
            ajWarn("ensAssemblymapperadaptorSeqregionIdsToNames could not "
                   "resolve Sequence Region identifier %u to a "
                   "Sequence Region.\n",
                   *Pid);

        ensSeqregionDel(&sr);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorSeqregionNamesToIds **************************
**
** Converts Ensembl Sequence Region names to their internal database
** identifiers.
**
** The caller is responsible for deleting the
** AJAX unsigned integers before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::seq_regions_to_ids
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::_seq_region_name_to_id
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] names [const AjPList] AJAX List of AJAX Strings of
**                                  Ensembl Sequence Region names
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers of
**                                  Ensembl Sequence Region identifiers
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
**
** FIXME: This function is obsolete in the EMBOSS implementation, since the
** Ensembl Sequence Region records its identifier internally.
** TODO: Suggest this to the Ensembl Core team.
******************************************************************************/

AjBool ensAssemblymapperadaptorSeqregionNamesToIds(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs,
    const AjPList names,
    AjPList identifiers)
{
    ajuint *Pid  = NULL;

    AjIList iter = NULL;

    AjPStr srname = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(!ama)
        return ajFalse;

    if(!cs)
        return ajFalse;

    if(!names)
        return ajFalse;

    if(!identifiers)
        return ajFalse;

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    iter = ajListIterNewread(names);

    while(!ajListIterDone(iter))
    {
        srname = (AjPStr) ajListIterGet(iter);

        ensSeqregionadaptorFetchByName(sra, cs, srname, &sr);

        if(sr)
        {
            AJNEW0(Pid);

            *Pid = ensSeqregionGetIdentifier(sr);

            ajListPushAppend(identifiers, (void *) Pid);
        }
        else
            ajWarn("ensAssemblymapperadaptorSeqregionNamesToIds could not "
                   "resolve Sequence Region name '%S' and "
                   "Coordinate System identifier %u to a valid "
                   "Sequence Region.\n",
                   srname,
                   ensCoordsystemGetIdentifier(cs));

        ensSeqregionDel(&sr);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterAssembled ****************************
**
** Registers an assembled Ensembl Sequence Region in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_assembled
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @param [r] asmsrid [ajuint] Assembled Ensembl Sequence Region identifier
** @param [r] regstart [ajint] Start coordinate of the assembled
**                             Ensembl Sequence Region
** @param [r] regend [ajint] End ccordinate of the assembled
**                           Ensembl Sequence Region
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

AjBool ensAssemblymapperadaptorRegisterAssembled(
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

    ajint chunkregionstart = 0;
    ajint chunkregionend   = 0;

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

    if(ajDebugTest("ensAssemblymapperadaptorRegisterAssembled"))
        ajDebug("ensAssemblymapperadaptorRegisterAssembled\n"
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
    ** Split up the region to be registered into fixed chunks.
    ** This allows us to keep track of regions that have already been
    ** registered and also works under the assumption that if a small region
    ** is requested it is likely that other requests will be made in the
    ** vicinity (the minimum size registered the chunksize (2^chunkfactor)).
    */

    chunkregions = ajListNew();

    /*
    ** Determine the span of chunks.
    ** A bitwise shift right is a fast and easy integer division.
    */

    /*
    ** FIXME: regstart and regend are signed. Does bit shifting or a division
    ** make sense in this case?
    ** Shifting of negative integers is implementation (i.e. compiler)
    ** dependent.
    */

    chunkstart = regstart >> assemblyMapperChunkFactor;

    chunkend = regend >> assemblyMapperChunkFactor;

    /*
    ** Inserts have start == end + 1, on boundary condition start chunk
    ** could be less than end chunk.
    */

    if(regstart == (regend + 1))
    {
        temp = regstart;

        regstart = regend;

        regend = temp;
    }

    /* Find regions of continuous unregistered chunks. */

    for(i = chunkstart; i <= chunkend; i++)
    {
        if(ensGenericassemblymapperHaveRegisteredAssembled(gam, asmsrid, i))
        {
            if(chunkregionstart >= 0)
            {
                /* This is the end of an unregistered region. */

                chunkregion = ensMapperrangeNew(
                    chunkregionstart << assemblyMapperChunkFactor,
                    ((chunkregionend + 1) << assemblyMapperChunkFactor) - 1);

                ajListPushAppend(chunkregions, (void *) chunkregion);

                chunkregionstart = -1;

                chunkregionend = -1;
            }
        }
        else
        {
            if(chunkregionstart < 0)
                chunkregionstart = i;

            chunkregionend = i + 1;

            ensGenericassemblymapperRegisterAssembled(gam, asmsrid, i);
        }
    }

    /* The last part may have been an unregistered region too. */

    if(chunkregionstart >= 0)
    {
        chunkregion = ensMapperrangeNew(
            chunkregionstart << assemblyMapperChunkFactor,
            ((chunkregionend + 1) << assemblyMapperChunkFactor) - 1);

        ajListPushAppend(chunkregions, (void *) chunkregion);
    }

    if(!ajListGetLength(chunkregions))
        return ajTrue;

    /* Keep the Mapper to a reasonable size. */

    if(ensGenericassemblymapperGetPairCount(gam) > gam->MaxPairCount)
    {
        ensGenericassemblymapperClear(gam);

        while(ajListPop(chunkregions, (void **) &chunkregion))
            ensMapperrangeDel(&chunkregion);

        /* After clearing the cache, everything needs to be re-registered. */

        chunkregion = ensMapperrangeNew(
            chunkstart << assemblyMapperChunkFactor,
            ((chunkend + 1) << assemblyMapperChunkFactor) - 1);

        ajListPushAppend(chunkregions, (void *) chunkregion);

        for(i = chunkstart; i <= chunkend; i++)
            ensGenericassemblymapperRegisterAssembled(gam, asmsrid, i);
    }

    /*
    ** Retrieve the description of how the assembled region is made from
    ** component regions for each of the continuous blocks of unregistered,
    ** chunked regions.
    */

    sra = ensRegistryGetSeqregionadaptor(ama->Adaptor);

    while(ajListPop(chunkregions, (void **) &chunkregion))
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
            ensCoordsystemGetIdentifier(gam->CmpCoordsystem));

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

            if(!(ensGenericassemblymapperHaveRegisteredComponent(gam, cmpsrid)
                 &&
                 (!assemblyMapperadaptorHasMultipleMappings(ama, cmpsrid))))
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

                sr = ensSeqregionNew(sra,
                                     cmpsrid,
                                     gam->CmpCoordsystem,
                                     cmpsrname,
                                     cmpsrlength);

                ensSeqregionadaptorCacheInsert(sra, &sr);

                ensSeqregionDel(&sr);
            }

            ajStrDel(&cmpsrname);
        }

        ajSqlrowiterDel(&sqli);

        ajSqlstatementDel(&sqls);

        ajStrDel(&statement);

        ensMapperrangeDel(&chunkregion);
    }

    ajListFree(&chunkregions);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterComponent ****************************
**
** Registers a component Ensembl Sequence Region in an
** Ensembl Generic Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_component
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
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

AjBool ensAssemblymapperadaptorRegisterComponent(
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

    if(ajDebugTest("ensAssemblymapperadaptorRegisterComponent"))
        ajDebug("ensAssemblymapperadaptorRegisterComponent\n"
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

    if(ensGenericassemblymapperHaveRegisteredComponent(gam, cmpsrid) &&
       (!assemblyMapperadaptorHasMultipleMappings(ama, cmpsrid)))
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
        ensCoordsystemGetIdentifier(gam->AsmCoordsystem));

    sqls = ensDatabaseadaptorSqlstatementNew(ama->Adaptor, statement);

    if(!ajSqlstatementGetSelectedrows(sqls))
    {
        ensGenericassemblymapperRegisterComponent(gam, cmpsrid);

        ajSqlstatementDel(&sqls);

        ajStrDel(&statement);

        return ajTrue;
    }

    if(ajSqlstatementGetSelectedrows(sqls) > 1)
        ajFatal("ensAssemblymapperadaptorRegisterComponent "
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
        ** register things in assembled chunks which allows us to:
        ** (1) Keep track of what assembled regions are registered
        ** (2) Use locality of reference
        ** if they want something in same general region it will already
        ** be registered.
        */

        ensAssemblymapperadaptorRegisterAssembled(ama,
                                                  gam,
                                                  asmsrid,
                                                  asmsrstart,
                                                  asmsrend);

        /*
        ** Add the assembled Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNew(sra,
                             asmsrid,
                             gam->AsmCoordsystem,
                             asmsrname,
                             asmsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&asmsrname);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterAll **********************************
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
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] gam [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
**
** @return [AjBool] ajTrue on success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterAll(EnsPAssemblymapperadaptor ama,
                                           EnsPGenericassemblymapper gam)
{
    register ajint i  = 0;

    ajint ori         = 0;
    ajint endchunk    = 0;

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

    AjPStr sqlfmt    = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensAssemblymapperadaptorRegisterAll"))
        ajDebug("ensAssemblymapperadaptorRegisterAll\n"
                "  ama %p\n"
                "  gam %p\n",
                ama,
                gam);

    dba = ensAssemblymapperadaptorGetDatabaseadaptor(gam->Adaptor);

    sra = ensRegistryGetSeqregionadaptor(dba);

    sqlfmt = ajStrNewC(
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
        "asm_sr.coord_system_id = %u");

    statement = ajFmtStr(ajStrGetPtr(sqlfmt),
                         ensCoordsystemGetIdentifier(gam->CmpCoordsystem),
                         ensCoordsystemGetIdentifier(gam->AsmCoordsystem));

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

        endchunk = asmsrlength >> assemblyMapperChunkFactor;

        for(i = 0; i <= endchunk; i++)
            ensGenericassemblymapperRegisterAssembled(gam, asmsrid, i);

        /*
        ** Add the component Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNew(sra,
                             cmpsrid,
                             gam->CmpCoordsystem,
                             cmpsrname,
                             cmpsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        /*
        ** Add the assembled Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNew(sra,
                             asmsrid,
                             gam->AsmCoordsystem,
                             asmsrname,
                             asmsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&asmsrname);
        ajStrDel(&cmpsrname);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    ajStrDel(&sqlfmt);

    return ajTrue;
}




/* @funcstatic assemblyMapperadaptorBuildCombinedMapper ***********************
**
** Build a combined Mapper after both halves of an
** Ensembl Chained Assembly Mapper have been loaded.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::_build_combined_mapper
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [r] ranges [AjPList] AJAX List of Ensembl Mapper Ranges
** @param [r] srcmidmapper [EnsPMapper] Ensembl Mapper for source to middle
**                                      Coordinate System mapping
** @param [r] trgmidmapper [EnsPMapper] Ensembl Mapper for target to middle
**                                      Coordinate System mapping
** @param [u] srctrgmapper [EnsPMapper] Ensembl Mapper for source to target
**                                      Coordinate System mapping,
**                                      the combined Mapper
** @param [r] srctype [AjPStr] Source mapping type
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
** After both halves of an Ensembl Chained Mapper are loaded this function
** maps all Ensembl Mapper Ranges in the AJAX List and loads the
** Ensembl Mapper Results into the combined (srctrgmapper) Ensembl Mapper.
******************************************************************************/

static AjBool assemblyMapperadaptorBuildCombinedMapper(
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

    if(ajDebugTest("assemblyMapperadaptorBuildCombinedMapper"))
    {
        ajDebug("assemblyMapperadaptorBuildCombinedMapper\n"
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

        ensMapperMapCoordinates(srcmidmapper,
                                ensMapperunitGetObjectIdentifier(mu),
                                ensMapperunitGetStart(mu),
                                ensMapperunitGetEnd(mu),
                                1,
                                srctype,
                                mrs1);

        while(ajListPop(mrs1, (void **) &mr1))
        {
            if(ensMapperresultGetType(mr1) == ensEMapperresultGap)
            {
                sum += ensMapperresultGetGapLength(mr1);

                ensMapperresultDel(&mr1);

                continue;
            }

            /* Feed the results of the first Ensembl Mapper into the second. */

            mrs2 = ajListNew();

            ensMapperMapCoordinates(trgmidmapper,
                                    ensMapperresultGetObjectIdentifier(mr1),
                                    ensMapperresultGetStart(mr1),
                                    ensMapperresultGetEnd(mr1),
                                    ensMapperresultGetStrand(mr1),
                                    midtype,
                                    mrs2);

            while(ajListPop(mrs2, (void **) &mr2))
            {
                if(ensMapperresultGetType(mr2) == ensEMapperresultCoordinate)
                {
                    totalstart = ensMapperunitGetStart(mu) + sum;

                    totalend = totalstart +
                        ensMapperresultGetCoordinateLength(mr2) - 1;

                    if(ajStrMatchC(srctype, "source"))
                        ensMapperAddCoordinates(
                            srctrgmapper,
                            ensMapperunitGetObjectIdentifier(mu),
                            totalstart,
                            totalend,
                            ensMapperresultGetStrand(mr2),
                            ensMapperresultGetObjectIdentifier(mr2),
                            ensMapperresultGetStart(mr2),
                            ensMapperresultGetEnd(mr2));
                    else
                        ensMapperAddCoordinates(
                            srctrgmapper,
                            ensMapperresultGetObjectIdentifier(mr2),
                            ensMapperresultGetStart(mr2),
                            ensMapperresultGetEnd(mr2),
                            ensMapperresultGetStrand(mr2),
                            ensMapperunitGetObjectIdentifier(mu),
                            totalstart,
                            totalend);
                }

                sum += ensMapperresultGetLength(mr2);

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




/* @funcstatic assemblyMapperadaptorMappingPath *******************************
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
** ensCoordsystemadaptorGetMappingPath for details. If the middle
** Coordinate System is defined we have a multi-component mapping
** path, which is not acceptable.
**
** @param [r] mappath [const AjPList] Mapping path, which is an AJAX List of
**                                    Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyMapperadaptorMappingPath(const AjPList mappath)
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
        ajListPeekNumber(mappath, 1, (void **) &cs);

        if(!cs)
            return ajTrue;
    }

    ajDebug("assemblyMapperadaptorMappingPath got inacceptable "
            "assembly mapping path.\n");

    ensCoordsystemMappingPathTrace(mappath, 1);

    return ajFalse;
}




/* @func ensAssemblymapperadaptorRegisterChained ******************************
**
** Registers an AJAX List of previously unregistered Ensembl Mapper Ranges in
** an Ensembl Chained Assembly Mapper.
**
** Optionally, only those Ensembl Mapper Ranges can be registered, which map
** onto a particular Ensembl Sequence Region, if its identifier was specified.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_chained
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] source [const AjPStr] Mapping type for the source
** @param [r] srcsrid [ajuint] Source Ensembl Sequence Region identifier
** @param [r] optsrid [ajuint] Optional Ensembl Sequence Region identifier
** @param [r] ranges [AjPList] AJAX List of Ensembl Mapper Ranges
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterChained(EnsPAssemblymapperadaptor ama,
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

    if(ajDebugTest("ensAssemblymapperadaptorRegisterChained"))
        ajDebug("ensAssemblymapperadaptorRegisterChained\n"
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
               ensChainedassemblymapperGetSourceCoordsystem(cam),
               ensChainedassemblymapperGetTargetCoordsystem(cam)))
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

        srcmidmapper = cam->SourceMiddleMapper;

        srccs = cam->SourceCoordsystem;

        srcregistry = cam->SourceRegistry;

        trgtype = ajStrNewC("target");

        trgmidmapper = cam->TargetMiddleMapper;

        trgcs = cam->TargetCoordsystem;

        trgregistry = cam->TargetRegistry;
    }
    else if(ajStrMatchC(source, "target"))
    {
        srctype = ajStrNewC("target");

        srcmidmapper = cam->TargetMiddleMapper;

        srccs = cam->TargetCoordsystem;

        srcregistry = cam->TargetRegistry;

        trgtype = ajStrNewC("source");

        trgmidmapper = cam->SourceMiddleMapper;

        trgcs = cam->SourceCoordsystem;

        trgregistry = cam->SourceRegistry;
    }
    else
        ajFatal("ensAssemblymapperadaptorRegisterChained invalid 'source' "
                "argument '%S' must be 'source' or 'target'.",
                source);

    midtype = ajStrNewC("middle");

    srctrgmapper = cam->SourceTargetMapper;

    midcs = cam->MiddleCoordsystem;

    /*
    ** Obtain the first half of the mappings and load them into the
    ** source <-> middle Mapper. Check for the simple case, where the
    ** Chained Assembly Mapper is short and requires only a one-step
    ** source <-> target Mapper.
    */

    if(midcs)
        mappath = ensCoordsystemadaptorGetMappingPath(csa, srccs, midcs);
    else
    {
        mappath = ensCoordsystemadaptorGetMappingPath(csa, srccs, trgcs);

        srcmidmapper = srctrgmapper;
    }

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblyMapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChained "
                "unexpected mapping between source and middle "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(srccs),
                ensCoordsystemGetVersion(srccs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void **) &asmcs);

    ajListPeekLast(mappath, (void **) &cmpcs);

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
        {
            ajFmtPrintAppS(&cmp2asm,
                           " AND asm.asm_seq_region_id = %u",
                           ensSeqregionGetIdentifier(optsr));
        }
        else if(ensCoordsystemMatch(cmpcs, ensSeqregionGetCoordsystem(optsr)))
        {
            ajFmtPrintAppS(&asm2cmp,
                           " AND asm.cmp_seq_region_id = %u",
                           ensSeqregionGetIdentifier(optsr));
        }
        else
            ajWarn("ensAssemblymapperadaptorRegisterChained got an optional "
                   "Sequence Region, which is neither linked to the "
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

            mu = ensMapperunitNew(srcsrid, srcsrstart, srcsrend);

            ajListPushAppend(srcranges, (void *) mu);

            /* Record Mapper Units for the middle part. */

            mu = ensMapperunitNew(midsrid, midsrstart, midsrend);

            ajListPushAppend(midranges, (void *) mu);

            /*
            ** Add the (new) middle Ensembl Sequence Region into the
            ** Ensembl Sequence Region Adaptor cache.
            */

            sr = ensSeqregionNew(sra,
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
                ensMapperrangeregistryCheckAndRegister(srcregistry,
                                                       srcsrid,
                                                       srcsrstart,
                                                       srcsrend,
                                                       srcsrstart,
                                                       srcsrend,
                                                       (AjPList) NULL);

            ajStrDel(&midsrname);
        }

        ajSqlrowiterDel(&sqli);

        ajSqlstatementDel(&sqls);

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

        while(ajListPop(srcranges, (void **) &mu))
            ensMapperunitDel(&mu);

        ajListFree(&srcranges);

        /*
        ** Register the middle Mapper Units in the
        ** target Mapper Range Registry
        ** and that's it for the simple case ...
        */

        while(ajListPop(midranges, (void **) &mu))
        {
            ensMapperrangeregistryCheckAndRegister(
                trgregistry,
                ensMapperunitGetObjectIdentifier(mu),
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

    mappath = ensCoordsystemadaptorGetMappingPath(csa, midcs, trgcs);

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblyMapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChained "
                "unexpected mapping between intermediate and target "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ensCoordsystemGetName(trgcs),
                ensCoordsystemGetVersion(trgcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void **) &asmcs);

    ajListPeekLast(mappath, (void **) &cmpcs);

    sqlfmt = (ensCoordsystemMatch(midcs, asmcs)) ? asm2cmp : cmp2asm;

    while(ajListPop(midranges, (void **) &mu))
    {
        statement = ajFmtStr(ajStrGetPtr(sqlfmt),
                             ensMapperunitGetObjectIdentifier(mu),
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
                                    ensMapperunitGetObjectIdentifier(mu),
                                    midsrstart,
                                    midsrend);

            /*
            ** Add the (new) target Ensembl Sequence Region into the
            ** Ensembl Sequence Region Adaptor cache.
            */

            sr = ensSeqregionNew(sra,
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

            ensMapperrangeregistryCheckAndRegister(trgregistry,
                                                   trgsrid,
                                                   trgsrstart,
                                                   trgsrend,
                                                   trgsrstart,
                                                   trgsrend,
                                                   (AjPList) NULL);

            ajStrDel(&trgsrname);
        }

        ajSqlrowiterDel(&sqli);

        ajSqlstatementDel(&sqls);

        ajStrDel(&statement);

        ensMapperunitDel(&mu);
    }

    ajListFree(&midranges);

    /*
    ** Now that both halves are loaded do stepwise mapping using both of
    ** the loaded Mappers to load the final source <-> target Mapper.
    */

    assemblyMapperadaptorBuildCombinedMapper(ama,
                                             srcranges,
                                             srcmidmapper,
                                             trgmidmapper,
                                             srctrgmapper,
                                             srctype);

    while(ajListPop(srcranges, (void **) &mu))
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
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @param [r] source [const AjPStr] Mapping type for the source
** @param [r] srcsrid [ajuint] Ensembl Sequence Region identifier
** @param [rE] optsrid [ajuint] Optional Ensembl Sequence Region identifier
** @param [r] ranges [AjPList] AJAX List of Ensembl Mapper Ranges
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
    EnsPMapper trgmidmapper = NULL;

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

        srcmidmapper = cam->SourceMiddleMapper;

        srccs = cam->SourceCoordsystem;

        srcregistry = cam->SourceRegistry;

        trgtype = ajStrNewC("target");

        trgmidmapper = cam->TargetMiddleMapper;

        trgcs = cam->TargetCoordsystem;

        trgregistry = cam->TargetRegistry;
    }
    else if(ajStrMatchC(source, "target"))
    {
        srctype = ajStrNewC("target");

        srcmidmapper = cam->TargetMiddleMapper;

        srccs = cam->TargetCoordsystem;

        srcregistry = cam->TargetRegistry;

        trgtype = ajStrNewC("source");

        trgmidmapper = cam->SourceMiddleMapper;

        trgcs = cam->SourceCoordsystem;

        trgregistry = cam->SourceRegistry;
    }
    else
        ajFatal("ensAssemblymapperadaptorRegisterChainedSpecial invalid "
                "'source' argument '%S' must be 'source' or 'target'.",
                source);

    midtype = ajStrNewC("middle");

    srctrgmapper = cam->SourceTargetMapper;

    midcs = cam->MiddleCoordsystem;

    /*
    ** Obtain the first half of the mappings and load them into the
    ** source <-> middle Mapper. Check for the simple case, where the
    ** Chained Assembly Mapper is short and requires only a one-step
    ** source <-> target Mapper.
    */

    if(midcs)
        mappath = ensCoordsystemadaptorGetMappingPath(csa, srccs, midcs);
    else
    {
        mappath = ensCoordsystemadaptorGetMappingPath(csa, srccs, trgcs);

        srcmidmapper = srctrgmapper;
    }

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblyMapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterChainedSpecial "
                "unexpected mapping between start and intermediate "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(srccs),
                ensCoordsystemGetVersion(srccs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void **) &asmcs);

    ajListPeekLast(mappath, (void **) &cmpcs);

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

                mu = ensMapperunitNew((i) ? optsrid : srcsrid,
                                      srcsrstart,
                                      srcsrend);

                ajListPushAppend(srcranges, (void *) mu);

                /* Record Mapper Units for the middle part. */

                mu = ensMapperunitNew(midsrid, midsrstart, midsrend);

                ajListPushAppend(midranges, (void *) mu);

                /*
                ** Add the (new) middle Ensembl Sequence Region into the
                ** Ensembl Sequence Region Adaptor cache.
                */

                sr = ensSeqregionNew(sra,
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
                    ensMapperrangeregistryCheckAndRegister(
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

            ajSqlstatementDel(&sqls);

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

        while(ajListPop(srcranges, (void **) &mu))
            ensMapperunitDel(&mu);

        ajListFree(&srcranges);

        /*
        ** Register the middle Mapper Units in the
        ** target Mapper Range Registry
        ** and that's it for the simple case ...
        */

        while(ajListPop(midranges, (void **) &mu))
        {
            ensMapperrangeregistryCheckAndRegister(
                trgregistry,
                ensMapperunitGetObjectIdentifier(mu),
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

    while(ajListPop(srcranges, (void **) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&srcranges);

    while(ajListPop(midranges, (void **) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&midranges);

    ajStrDel(&srctype);
    ajStrDel(&midtype);
    ajStrDel(&trgtype);
    ajStrDel(&asm2cmp);

    return ajTrue;
}




/* @func ensAssemblymapperadaptorRegisterAllChained ***************************
**
** Registers all Ensembl Mapper Ranges in an
** Ensembl Chained Assembly Mapper.
**
** @cc Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor::register_all_chained
** @param [r] ama [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @param [u] cam [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
**
** @return [AjBool] ajTrue on success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblymapperadaptorRegisterAllChained(
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

    if(ajDebugTest("ensAssemblymapperadaptorRegisterAllChained"))
        ajDebug("ensAssemblymapperadaptorRegisterAllChained\n"
                "  ama %p\n"
                "  cam %p\n",
                ama,
                cam);

    if(!ama)
        return ajFalse;

    if(!cam)
        return ajFalse;

    srccs = cam->SourceCoordsystem;
    midcs = cam->MiddleCoordsystem;
    trgcs = cam->TargetCoordsystem;

    srcmidmapper = cam->SourceMiddleMapper;
    trgmidmapper = cam->TargetMiddleMapper;
    srctrgmapper = cam->SourceTargetMapper;

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
        mappath = ensCoordsystemadaptorGetMappingPath(csa, srccs, midcs);

        mapper = srcmidmapper;
    }
    else
    {
        mappath = ensCoordsystemadaptorGetMappingPath(csa, srccs, trgcs);

        mapper = srctrgmapper;
    }

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblyMapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterAllChained "
                "unexpected mapping between source and intermediate "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(srccs),
                ensCoordsystemGetVersion(srccs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ranges = ajListNew();

    ajListPeekFirst(mappath, (void **) &asmcs);

    ajListPeekLast(mappath, (void **) &cmpcs);

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

        mu = ensMapperunitNew(srcsrid, srcsrstart, srcsrend);

        ajListPushAppend(ranges, (void *) mu);

        /* Register the source Mapper Units in the source Mapper Registry. */

        ensMapperrangeregistryCheckAndRegister(cam->SourceRegistry,
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
            ensMapperrangeregistryCheckAndRegister(cam->TargetRegistry,
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

        sr = ensSeqregionNew(sra, srcsrid, srccs, srcsrname, srcsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        /*
        ** Add the (new) middle Ensembl Sequence Region into the
        ** Ensembl Sequence Region Adaptor cache.
        */

        sr = ensSeqregionNew(sra,
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

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    if(!midcs)
    {
        /* That is all for the simple case. */

        while(ajListPop(ranges, (void **) &mu))
            ensMapperunitDel(&mu);

        ajListFree(&ranges);

        ajStrDel(&sqlfmt);

        return ajTrue;
    }

    mappath = ensCoordsystemadaptorGetMappingPath(csa, trgcs, midcs);

    /* Test for an acceptable two- or multi-component mapping path. */

    if(!assemblyMapperadaptorMappingPath(mappath))
        ajFatal("ensAssemblymapperadaptorRegisterAllChained "
                "unexpected mapping between target and intermediate "
                "Coordinate Systems '%S:%S' and '%S:%S'. "
                "Expected path length 2, but got length %d.",
                ensCoordsystemGetName(trgcs),
                ensCoordsystemGetVersion(trgcs),
                ensCoordsystemGetName(midcs),
                ensCoordsystemGetVersion(midcs),
                ajListGetLength(mappath));

    ajListPeekFirst(mappath, (void **) &asmcs);

    ajListPeekLast(mappath, (void **) &cmpcs);

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

        ensMapperrangeregistryCheckAndRegister(cam->TargetRegistry,
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

        sr = ensSeqregionNew(sra, trgsrid, trgcs, trgsrname, trgsrlength);

        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);

        ajStrDel(&srcsrname);
        ajStrDel(&trgsrname);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    srctype = ajStrNewC("source");

    assemblyMapperadaptorBuildCombinedMapper(ama,
                                             ranges,
                                             srcmidmapper,
                                             trgmidmapper,
                                             srctrgmapper,
                                             srctype);

    ajStrDel(&srctype);

    while(ajListPop(ranges, (void **) &mu))
        ensMapperunitDel(&mu);

    ajListFree(&ranges);

    ajStrDel(&sqlfmt);

    return ajTrue;
}

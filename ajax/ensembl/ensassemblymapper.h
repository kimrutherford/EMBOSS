/* @include ensassemblymapper *************************************************
**
** Ensembl Assembly Mapper functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.18 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:03:15 $ by $Author: mks $
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

#ifndef ENSASSEMBLYMAPPER_H
#define ENSASSEMBLYMAPPER_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Generic Assembly Mapper */

EnsPGenericassemblymapper ensGenericassemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem asmcs,
    EnsPCoordsystem cmpcs);

EnsPGenericassemblymapper ensGenericassemblymapperNewRef(
    EnsPGenericassemblymapper gam);

AjBool ensGenericassemblymapperClear(EnsPGenericassemblymapper gam);

void ensGenericassemblymapperDel(EnsPGenericassemblymapper *Pgam);

EnsPAssemblymapperadaptor ensGenericassemblymapperGetAdaptor(
    const EnsPGenericassemblymapper gam);

const EnsPCoordsystem ensGenericassemblymapperGetCoordsystemAssembled(
    const EnsPGenericassemblymapper gam);

const EnsPCoordsystem ensGenericassemblymapperGetCoordsystemComponent(
    const EnsPGenericassemblymapper gam);

EnsPMapper ensGenericassemblymapperGetMapper(
    const EnsPGenericassemblymapper gam);

ajuint ensGenericassemblymapperGetMaximum(
    const EnsPGenericassemblymapper gam);

AjBool ensGenericassemblymapperSetAdaptor(EnsPGenericassemblymapper gam,
                                          EnsPAssemblymapperadaptor ama);

AjBool ensGenericassemblymapperSetMaximum(EnsPGenericassemblymapper gam,
                                          ajuint maximum);

AjBool ensGenericassemblymapperTrace(const EnsPGenericassemblymapper gam,
                                     ajuint level);

ajuint ensGenericassemblymapperGetCount(
    const EnsPGenericassemblymapper gam);

AjBool ensGenericassemblymapperRegisterAll(EnsPGenericassemblymapper gam);

AjBool ensGenericassemblymapperRegisterAssembled(EnsPGenericassemblymapper gam,
                                                 ajuint asmsrid,
                                                 ajint chunkid);

AjBool ensGenericassemblymapperRegisterComponent(EnsPGenericassemblymapper gam,
                                                 ajuint cmpsrid);

AjBool ensGenericassemblymapperCheckAssembled(
    const EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajint chunkid);

AjBool ensGenericassemblymapperCheckComponent(
    const EnsPGenericassemblymapper gam,
    ajuint cmpsrid);

AjBool ensGenericassemblymapperMap(EnsPGenericassemblymapper gam,
                                   const EnsPSeqregion sr,
                                   ajint srstart,
                                   ajint srend,
                                   ajint srstrand,
                                   AjBool fastmap,
                                   AjPList mrs);

/* Ensembl Chained Assembly Mapper */

EnsPChainedassemblymapper ensChainedassemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem srccs,
    EnsPCoordsystem midcs,
    EnsPCoordsystem trgcs);

EnsPChainedassemblymapper ensChainedassemblymapperNewRef(
    EnsPChainedassemblymapper cam);

void ensChainedassemblymapperDel(EnsPChainedassemblymapper *Pcam);

AjBool ensChainedassemblymapperClear(EnsPChainedassemblymapper cam);

EnsPAssemblymapperadaptor ensChainedassemblymapperGetAdaptor(
    const EnsPChainedassemblymapper cam);

const EnsPCoordsystem ensChainedassemblymapperGetCoordsystemMiddle(
    const EnsPChainedassemblymapper cam);

const EnsPCoordsystem ensChainedassemblymapperGetCoordsystemSource(
    const EnsPChainedassemblymapper cam);

const EnsPCoordsystem ensChainedassemblymapperGetCoordsystemTarget(
    const EnsPChainedassemblymapper cam);

EnsPMapper ensChainedassemblymapperGetMapperSourceMiddle(
    const EnsPChainedassemblymapper cam);

EnsPMapper ensChainedassemblymapperGetMapperSourceTarget(
    const EnsPChainedassemblymapper cam);

EnsPMapper ensChainedassemblymapperGetMapperTargetMiddle(
    const EnsPChainedassemblymapper cam);

ajuint ensChainedassemblymapperGetMaximum(
    const EnsPChainedassemblymapper cam);

EnsPMapperrangeregistry ensChainedassemblymapperGetRegistrySource(
    const EnsPChainedassemblymapper cam);

EnsPMapperrangeregistry ensChainedassemblymapperGetRegistryTarget(
    const EnsPChainedassemblymapper cam);

AjBool ensChainedassemblymapperSetAdaptor(EnsPChainedassemblymapper cam,
                                          EnsPAssemblymapperadaptor ama);

AjBool ensChainedassemblymapperSetMaximum(EnsPChainedassemblymapper cam,
                                          ajuint maximum);

AjBool ensChainedassemblymapperTrace(const EnsPChainedassemblymapper cam,
                                     ajuint level);
ajuint ensChainedassemblymapperGetCount(
    const EnsPChainedassemblymapper cam);

AjBool ensChainedassemblymapperMap(EnsPChainedassemblymapper cam,
                                   const EnsPSeqregion sr,
                                   ajint srstart,
                                   ajint srend,
                                   ajint srstrand,
                                   const EnsPSeqregion optsr,
                                   AjBool fastmap,
                                   AjPList mrs);

AjBool ensChainedassemblymapperRegisterAll(EnsPChainedassemblymapper cam);

/* Ensembl Top-level Assembly Mapper */

EnsPToplevelassemblymapper ensToplevelassemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem asmcs,
    EnsPCoordsystem cmpcs);

EnsPToplevelassemblymapper ensToplevelassemblymapperNewRef(
    EnsPToplevelassemblymapper tlam);

void ensToplevelassemblymapperDel(
    EnsPToplevelassemblymapper *Ptlam);

EnsPAssemblymapperadaptor ensToplevelassemblymapperGetAdaptor(
    const EnsPToplevelassemblymapper tlam);

const EnsPCoordsystem ensToplevelassemblymapperGetCoordsystemAssembled(
    const EnsPToplevelassemblymapper tlam);

const EnsPCoordsystem ensToplevelassemblymapperGetCoordsystemComponent(
    const EnsPToplevelassemblymapper tlam);

AjBool ensToplevelassemblymapperSetAdaptor(
    EnsPToplevelassemblymapper tlam,
    EnsPAssemblymapperadaptor ama);

AjBool ensToplevelassemblymapperTrace(const EnsPToplevelassemblymapper tlam,
                                      ajuint level);

AjBool ensToplevelassemblymapperMap(EnsPToplevelassemblymapper tlam,
                                    const EnsPSeqregion sr,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    AjBool fastmap,
                                    AjPList mrs);

/* Ensembl Assembly Mapper */

EnsPAssemblymapper ensAssemblymapperNewIni(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    EnsPChainedassemblymapper cam,
    EnsPToplevelassemblymapper tlam);

EnsPAssemblymapper ensAssemblymapperNewRef(
    EnsPAssemblymapper am);

void ensAssemblymapperDel(
    EnsPAssemblymapper *Pam);

AjBool ensAssemblymapperTrace(
    const EnsPAssemblymapper am,
    ajuint level);

AjBool ensAssemblymapperMapSeqregion(
    EnsPAssemblymapper am,
    const EnsPSeqregion sr,
    ajint srstart,
    ajint srend,
    ajint srstrand,
    AjBool fastmap,
    AjPList mrs);

AjBool ensAssemblymapperMapSlice(
    EnsPAssemblymapper am,
    const EnsPSlice slice,
    AjBool fastmap,
    AjPList mrs);

AjBool ensAssemblymapperMapToSeqregion(
    EnsPAssemblymapper am,
    const EnsPSeqregion sr,
    ajint srstart,
    ajint srend,
    ajint srstrand,
    const EnsPSeqregion optsr,
    AjBool fastmap,
    AjPList mrs);

AjBool ensAssemblymapperMapToSlice(
    EnsPAssemblymapper am,
    const EnsPSlice slice,
    const EnsPSlice optslice,
    AjBool fastmap,
    AjPList mrs);

/* Ensembl Assembly Mapper Adaptor */

EnsPAssemblymapperadaptor ensRegistryGetAssemblymapperadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblymapperadaptor ensAssemblymapperadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensAssemblymapperadaptorClear(
    EnsPAssemblymapperadaptor ama);

void ensAssemblymapperadaptorDel(
    EnsPAssemblymapperadaptor *Pama);

EnsPDatabaseadaptor ensAssemblymapperadaptorGetDatabaseadaptor(
    EnsPAssemblymapperadaptor ama);

AjBool ensAssemblymapperadaptorFetchByCoordsystems(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2,
    EnsPAssemblymapper *Pam);

AjBool ensAssemblymapperadaptorFetchBySlices(
    EnsPAssemblymapperadaptor ama,
    EnsPSlice slice1,
    EnsPSlice slice2,
    EnsPAssemblymapper *Pam);

AjBool ensAssemblymapperadaptorRegisterChainedAll(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam);

AjBool ensAssemblymapperadaptorRegisterChainedRegion(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam,
    const AjPStr source,
    ajuint srcsrid,
    ajuint optsrid,
    AjPList ranges);

AjBool ensAssemblymapperadaptorRegisterChainedSpecial(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam,
    const AjPStr source,
    ajuint srcsrid,
    ajuint optsrid,
    AjPList ranges);

AjBool ensAssemblymapperadaptorRegisterGenericAll(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam);

AjBool ensAssemblymapperadaptorRegisterGenericAssembled(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajint regstart,
    ajint regend);

AjBool ensAssemblymapperadaptorRegisterGenericComponent(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    ajuint cmpsrid);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSASSEMBLYMAPPER_H */

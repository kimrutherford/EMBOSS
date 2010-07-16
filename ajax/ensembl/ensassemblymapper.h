#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensassemblymapper_h
#define ensassemblymapper_h

#include "ensassembly.h"
#include "ensdata.h"
#include "enstable.h"




/*
** Prototype definitions
*/

/* Ensembl Generic Assembly Mapper */

EnsPGenericassemblymapper ensGenericassemblymapperNew(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem asmcs,
    EnsPCoordsystem cmpcs);

EnsPGenericassemblymapper ensGenericassemblymapperNewRef(
    EnsPGenericassemblymapper gam);

AjBool ensGenericassemblymapperClear(EnsPGenericassemblymapper gam);

void ensGenericassemblymapperDel(EnsPGenericassemblymapper* Pgam);

EnsPAssemblymapperadaptor ensGenericassemblymapperGetAdaptor(
    const EnsPGenericassemblymapper gam);

const EnsPCoordsystem ensGenericassemblymapperGetAssembledCoordsystem(
    const EnsPGenericassemblymapper gam);

const EnsPCoordsystem ensGenericassemblymapperGetComponentCoordsystem(
    const EnsPGenericassemblymapper gam);

EnsPMapper ensGenericassemblymapperGetMapper(
    const EnsPGenericassemblymapper gam);

ajuint ensGenericassemblymapperGetMaxPairCount(
    const EnsPGenericassemblymapper gam);

ajuint ensGenericassemblymapperGetPairCount(
    const EnsPGenericassemblymapper gam);

AjBool ensGenericassemblymapperSetAdaptor(EnsPGenericassemblymapper gam,
                                          EnsPAssemblymapperadaptor ama);

AjBool ensGenericassemblymapperSetMaxPairCount(EnsPGenericassemblymapper gam,
                                               ajuint count);

AjBool ensGenericassemblymapperRegisterAssembled(EnsPGenericassemblymapper gam,
                                                 ajuint asmsrid,
                                                 ajint chunkid);

AjBool ensGenericassemblymapperRegisterComponent(EnsPGenericassemblymapper gam,
                                                 ajuint cmpsrid);

AjBool ensGenericassemblymapperHaveRegisteredAssembled(
    const EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajint chunkid);

AjBool ensGenericassemblymapperHaveRegisteredComponent(
    const EnsPGenericassemblymapper gam,
    ajuint cmpsrid);

AjBool ensGenericassemblymapperMap(EnsPGenericassemblymapper gam,
                                   const EnsPSeqregion sr,
                                   ajint srstart,
                                   ajint srend,
                                   ajint srstrand,
                                   AjPList mrs);

AjBool ensGenericassemblymapperFastMap(EnsPGenericassemblymapper gam,
                                       const EnsPSeqregion sr,
                                       ajint srstart,
                                       ajint srend,
                                       ajint srstrand,
                                       AjPList mrs);

AjBool ensGenericassemblymapperRegisterAll(EnsPGenericassemblymapper gam);

/* Ensembl Chained Assembly Mapper */

EnsPChainedassemblymapper ensChainedassemblymapperNew(
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

const EnsPCoordsystem ensChainedassemblymapperGetSourceCoordsystem(
    const EnsPChainedassemblymapper cam);

const EnsPCoordsystem ensChainedassemblymapperGetMiddleCoordsystem(
    const EnsPChainedassemblymapper cam);

const EnsPCoordsystem ensChainedassemblymapperGetTargetCoordsystem(
    const EnsPChainedassemblymapper cam);

EnsPMapper ensChainedassemblymapperGetSourceMiddleMapper(
    const EnsPChainedassemblymapper cam);

EnsPMapper ensChainedassemblymapperGetTargetMiddleMapper(
    const EnsPChainedassemblymapper cam);

EnsPMapper ensChainedassemblymapperGetSourceTargetMapper(
    const EnsPChainedassemblymapper cam);

EnsPMapperrangeregistry ensChainedassemblymapperGetSourceRegistry(
    const EnsPChainedassemblymapper cam);

EnsPMapperrangeregistry ensChainedassemblymapperGetTargetRegistry(
    const EnsPChainedassemblymapper cam);

ajuint ensChainedassemblymapperGetMaxPairCount(
    const EnsPChainedassemblymapper cam);

AjBool ensChainedassemblymapperSetAdaptor(EnsPChainedassemblymapper cam,
                                          EnsPAssemblymapperadaptor ama);

AjBool ensChainedassemblymapperSetMaxPairCount(EnsPChainedassemblymapper cam,
                                               ajuint count);

ajuint ensChainedassemblymapperGetPairCount(
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

EnsPToplevelassemblymapper ensToplevelassemblymapperNew(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2);

EnsPToplevelassemblymapper ensToplevelassemblymapperNewRef(
    EnsPToplevelassemblymapper tlam);

void ensToplevelassemblymapperDel(EnsPToplevelassemblymapper* Ptlam);

const EnsPAssemblymapperadaptor ensToplevelassemblymapperGetAdaptor(
    const EnsPToplevelassemblymapper tlam);

const EnsPCoordsystem ensToplevelassemblymapperGetAssembledCoordsystem(
    const EnsPToplevelassemblymapper tlam);

const EnsPCoordsystem ensToplevelassemblymapperGetComponentCoordsystem(
    const EnsPToplevelassemblymapper tlam);

AjBool ensToplevelassemblymapperSetAdaptor(
    EnsPToplevelassemblymapper tlam,
    EnsPAssemblymapperadaptor ama);

AjBool ensToplevelassemblymapperMap(EnsPToplevelassemblymapper tlam,
                                    const EnsPSeqregion sr,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    AjBool fastmap,
                                    AjPList mrs);

/* Ensembl Assembly Mapper */

EnsPAssemblymapper ensAssemblymapperNew(EnsPAssemblymapperadaptor ama,
                                        EnsPGenericassemblymapper gam,
                                        EnsPChainedassemblymapper cam,
                                        EnsPToplevelassemblymapper tlam);

EnsPAssemblymapper ensAssemblymapperNewRef(EnsPAssemblymapper am);

void ensAssemblymapperDel(EnsPAssemblymapper* Pam);

AjBool ensAssemblymapperMap(EnsPAssemblymapper am,
                            const EnsPSeqregion sr,
                            ajint srstart,
                            ajint srend,
                            ajint srstrand,
                            AjPList mrs);

AjBool ensAssemblymapperFastMap(EnsPAssemblymapper am,
                                const EnsPSeqregion sr,
                                ajint srstart,
                                ajint srend,
                                ajint srstrand,
                                AjPList mrs);

AjBool ensAssemblymapperMapToSeqregion(EnsPAssemblymapper am,
                                       const EnsPSeqregion sr,
                                       ajint srstart,
                                       ajint srend,
                                       ajint srstrand,
                                       const EnsPSeqregion optsr,
                                       AjPList mrs);

/* Ensembl Assembly Mapper Adaptor */

EnsPAssemblymapperadaptor ensRegistryGetAssemblymapperadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblymapperadaptor ensAssemblymapperadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensAssemblymapperadaptorCacheClear(EnsPAssemblymapperadaptor ama);

void ensAssemblymapperadaptorDel(EnsPAssemblymapperadaptor* Pama);

EnsPDatabaseadaptor ensAssemblymapperadaptorGetDatabaseadaptor(
    const EnsPAssemblymapperadaptor ama);

EnsPAssemblymapper ensAssemblymapperadaptorFetchByCoordsystems(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2);

EnsPAssemblymapper ensAssemblymapperadaptorFetchBySlices(
    EnsPAssemblymapperadaptor ama,
    EnsPSlice slice1,
    EnsPSlice slice2);

AjBool ensAssemblymapperadaptorSeqregionIdsToNames(
    EnsPAssemblymapperadaptor ama,
    const AjPList ids,
    AjPList names);

AjBool ensAssemblymapperadaptorSeqregionNamesToIds(
    EnsPAssemblymapperadaptor ama,
    EnsPCoordsystem cs,
    const AjPList names,
    AjPList identifiers);

AjBool ensAssemblymapperadaptorRegisterAssembled(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    ajuint asmsrid,
    ajint regstart,
    ajint regend);

AjBool ensAssemblymapperadaptorRegisterComponent(
    EnsPAssemblymapperadaptor ama,
    EnsPGenericassemblymapper gam,
    ajuint cmpsrid);

AjBool ensAssemblymapperadaptorRegisterAll(EnsPAssemblymapperadaptor ama,
                                           EnsPGenericassemblymapper gam);

AjBool ensAssemblymapperadaptorRegisterChained(
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

AjBool ensAssemblymapperadaptorRegisterAllChained(
    EnsPAssemblymapperadaptor ama,
    EnsPChainedassemblymapper cam);

/*
** End of prototype definitions
*/

#define MENSCHAINEDASSEMBLYMAPPERADAPTORGETASSEMBLEDCOORDSYSTEM(cam)    \
ensChainedassemblymapperGetSourceCoordsystem(cam)

#define MENSCHAINEDASSEMBLYMAPPERADAPTORGETCOMPONENTCOORDSYSTEM(cam)    \
ensChainedassemblymapperGetTargetCoordsystem(cam)

#define MENSCHAINEDASSEMBLYMAPPERGETMAPPER(cam)         \
ensChainedassemblymapperGetSourceTargetMapper(cam)

#endif /* ensassemblymapper_h */

#ifdef __cplusplus
}
#endif

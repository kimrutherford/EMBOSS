
#ifndef ENSSLICE_H
#define ENSSLICE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensseqregion.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsERepeatMaskType **************************************************
**
** Ensembl Repeat Mask Type enumeration.
**
******************************************************************************/

typedef enum EnsORepeatMaskType
{
    ensERepeatMaskTypeNULL,
    ensERepeatMaskTypeNone,
    ensERepeatMaskTypeSoft,
    ensERepeatMaskTypeHard
} EnsERepeatMaskType;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPRepeatmaskedslice ************************************************
**
** Ensembl Repeat-Masked Slice.
**
** Holds information about a masked genome sequence slice.
**
** @alias EnsSRepeatmaskedslice
** @alias EnsORepeatmaskedslice
**
** @attr Slice [EnsPSlice] Ensembl Slice.
** @attr Analysisnames [AjPList] AJAX List of AJAX String objects
**                               (Ensembl Analysis names)
** @attr Masking [AjPTable] AJAX Table of Repeat Consensus types, classes or
**                          names and sequence masking types
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSRepeatmaskedslice
{
    EnsPSlice Slice;
    AjPList Analysisnames;
    AjPTable Masking;
    ajuint Use;
    ajuint Padding;
} EnsORepeatmaskedslice;

#define EnsPRepeatmaskedslice EnsORepeatmaskedslice*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Slice */

EnsPSlice ensSliceNewCpy(const EnsPSlice slice);

EnsPSlice ensSliceNewIni(EnsPSliceadaptor sla,
                         EnsPSeqregion sr,
                         ajint start,
                         ajint end,
                         ajint strand);

EnsPSlice ensSliceNewRef(EnsPSlice slice);

EnsPSlice ensSliceNewSeq(EnsPSliceadaptor sla,
                         EnsPSeqregion sr,
                         ajint start,
                         ajint end,
                         ajint strand,
                         AjPStr sequence);

void ensSliceDel(EnsPSlice* Pslice);

EnsPSliceadaptor ensSliceGetAdaptor(const EnsPSlice slice);

ajint ensSliceGetEnd(const EnsPSlice slice);

EnsPSeqregion ensSliceGetSeqregion(const EnsPSlice slice);

const AjPStr ensSliceGetSequence(const EnsPSlice slice);

ajint ensSliceGetStart(const EnsPSlice slice);

ajint ensSliceGetStrand(const EnsPSlice slice);

EnsESliceTopology ensSliceLoadTopology(EnsPSlice slice);

AjBool ensSliceSetAdaptor(EnsPSlice slice, EnsPSliceadaptor sla);

AjBool ensSliceSetSequence(EnsPSlice slice, AjPStr sequence);

AjBool ensSliceSetTopology(EnsPSlice slice, EnsESliceTopology sltp);

AjBool ensSliceTrace(const EnsPSlice slice, ajuint level);

const AjPStr ensSliceGetCoordsystemName(const EnsPSlice slice);

EnsPCoordsystem ensSliceGetCoordsystemObject(const EnsPSlice slice);

const AjPStr ensSliceGetCoordsystemVersion(const EnsPSlice slice);

ajuint ensSliceGetSeqregionIdentifier(const EnsPSlice slice);

ajint ensSliceGetSeqregionLength(const EnsPSlice slice);

const AjPStr ensSliceGetSeqregionName(const EnsPSlice slice);

const AjPTrn ensSliceGetTranslation(EnsPSlice slice);

ajint ensSliceCalculateCentrepoint(EnsPSlice slice);

ajuint ensSliceCalculateLength(EnsPSlice slice);

size_t ensSliceCalculateMemsize(const EnsPSlice slice);

ajuint ensSliceCalculateRegion(EnsPSlice slice, ajint start, ajint end);

AjBool ensSliceFetchAllAttributes(EnsPSlice slice,
                                  const AjPStr code,
                                  AjPList attributes);

AjBool ensSliceFetchAllRepeatfeatures(EnsPSlice slice,
                                      const AjPStr anname,
                                      const AjPStr rctype,
                                      const AjPStr rcclass,
                                      const AjPStr rcname,
                                      AjPList rfs);

AjBool ensSliceFetchAllSequenceedits(EnsPSlice slice,
                                     AjPList ses);

AjBool ensSliceFetchName(const EnsPSlice slice, AjPStr* Pname);

AjBool ensSliceFetchSequenceAllSeq(EnsPSlice slice, AjPSeq* Psequence);

AjBool ensSliceFetchSequenceAllStr(EnsPSlice slice, AjPStr* Psequence);

AjBool ensSliceFetchSequenceSubSeq(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPSeq* Psequence);

AjBool ensSliceFetchSequenceSubStr(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPStr* Psequence);

AjBool ensSliceFetchSliceinverted(EnsPSlice slice,
                                  EnsPSlice* Pslice);

AjBool ensSliceFetchSlicesub(EnsPSlice slice,
                             ajint start,
                             ajint end,
                             ajint strand,
                             EnsPSlice* Pslice);

AjBool ensSliceFetchSliceexpanded(EnsPSlice slice,
                                  ajint five,
                                  ajint three,
                                  AjBool force,
                                  ajint* Pfshift,
                                  ajint* Ptshift,
                                  EnsPSlice* Pslice);

int ensSliceCompareIdentifierAscending(const EnsPSlice slice1,
                                       const EnsPSlice slice2);

AjBool ensSliceMatch(const EnsPSlice slice1, const EnsPSlice slice2);

AjBool ensSliceSimilarity(const EnsPSlice slice1, const EnsPSlice slice2);

AjBool ensSliceIsCircular(EnsPSlice slice, AjBool* Presult);

AjBool ensSliceIsNonreference(EnsPSlice slice, AjBool* Presult);

AjBool ensSliceIsToplevel(EnsPSlice slice, AjBool* Presult);

AjBool ensSliceProject(EnsPSlice slice,
                       const AjPStr csname,
                       const AjPStr csversion,
                       AjPList pss);

AjBool ensSliceProjectslice(EnsPSlice srcslice,
                            EnsPSlice trgslice,
                            AjPList pss);

EnsESliceType ensSliceTypeFromSeqregion(EnsPSeqregion sr);

EnsESliceType ensSliceTypeFromStr(const AjPStr type);

const char* ensSliceTypeToChar(EnsESliceType ste);

AjBool ensListSliceRemoveDuplications(AjPList slices);

/* Ensembl Slice Adaptor */

EnsPSliceadaptor ensRegistryGetSliceadaptor(
    EnsPDatabaseadaptor dba);

EnsPSliceadaptor ensSliceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSliceadaptorDel(EnsPSliceadaptor* Psla);

EnsPDatabaseadaptor ensSliceadaptorGetDatabaseadaptor(
    EnsPSliceadaptor sla);

AjBool ensSliceadaptorCacheInsert(EnsPSliceadaptor sla, EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchAll(EnsPSliceadaptor sla,
                               const AjPStr csname,
                               const AjPStr csversion,
                               AjBool nonreference,
                               AjBool duplicates,
                               AjBool lrg,
                               AjPList slices);

AjBool ensSliceadaptorFetchAllbyRegionunique(EnsPSliceadaptor sla,
                                             const AjPStr csname,
                                             const AjPStr csversion,
                                             const AjPStr srname,
                                             ajint srstart,
                                             ajint srend,
                                             ajint srstrand,
                                             AjPList slices);

AjBool ensSliceadaptorFetchByFeature(EnsPSliceadaptor sla,
                                     const EnsPFeature feature,
                                     ajint flank,
                                     EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchByMapperresult(EnsPSliceadaptor sla,
                                          const EnsPMapperresult mr,
                                          EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchByName(EnsPSliceadaptor sla,
                                  const AjPStr name,
                                  EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchByRegion(EnsPSliceadaptor sla,
                                    const AjPStr csname,
                                    const AjPStr csversion,
                                    const AjPStr srname,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchBySeqregionIdentifier(EnsPSliceadaptor sla,
                                                 ajuint srid,
                                                 ajint srstart,
                                                 ajint srend,
                                                 ajint srstrand,
                                                 EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchBySeqregionName(EnsPSliceadaptor sla,
                                           const AjPStr csname,
                                           const AjPStr csversion,
                                           const AjPStr srname,
                                           EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchBySlice(EnsPSliceadaptor sla,
                                   EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   EnsPSlice* Pslice);

AjBool ensSliceadaptorRetrieveNormalisedprojection(EnsPSliceadaptor sla,
                                                   EnsPSlice slice,
                                                   AjPList pss);

/* Ensembl Repeat-Masked Slice */

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewCpy(
    const EnsPRepeatmaskedslice rmslice);

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewIni(EnsPSlice slice,
                                                 AjPList annames,
                                                 AjPTable masking);

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewRef(
    EnsPRepeatmaskedslice rmslice);

void ensRepeatmaskedsliceDel(EnsPRepeatmaskedslice* Prmslice);

AjBool ensRepeatmaskedsliceFetchSequenceSeq(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPSeq* Psequence);

AjBool ensRepeatmaskedsliceFetchSequenceStr(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPStr* Psequence);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSLICE_H */

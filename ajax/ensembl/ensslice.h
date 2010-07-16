#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensslice_h
#define ensslice_h

#include "ensassemblyexception.h"
#include "ensassemblymapper.h"
#include "enscache.h"
#include "ensmapper.h"
#include "ensseqregion.h"
#include "enstable.h"




/* EnsERepeatMaskType *********************************************************
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
** @attr AnalysisNames [AjPList] AJAX List of Ensembl Analysis name
**                               AJAX Strings
** @attr Masking [AjPTable] AJAX Table of Repeat Consensus types, classes or
**                          names and sequence masking types
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSRepeatmaskedslice
{
    EnsPSlice Slice;
    AjPList AnalysisNames;
    AjPTable Masking;
    ajuint Use;
    ajuint Padding;
} EnsORepeatmaskedslice;

#define EnsPRepeatmaskedslice EnsORepeatmaskedslice*




#include "ensprojectionsegment.h"




/*
** Prototype definitions
*/

/* Ensembl Slice */

EnsPSlice ensSliceNewS(EnsPSliceadaptor adaptor,
                       EnsPSeqregion sr,
                       ajint start,
                       ajint end,
                       ajint strand,
                       AjPStr sequence);

EnsPSlice ensSliceNew(EnsPSliceadaptor adaptor,
                      EnsPSeqregion sr,
                      ajint start,
                      ajint end,
                      ajint strand);

EnsPSlice ensSliceNewObj(EnsPSlice object);

EnsPSlice ensSliceNewRef(EnsPSlice slice);

void ensSliceDel(EnsPSlice* Pslice);

EnsPSliceadaptor ensSliceGetAdaptor(const EnsPSlice slice);

EnsPSeqregion ensSliceGetSeqregion(const EnsPSlice slice);

const AjPStr ensSliceGetSequence(const EnsPSlice slice);

ajint ensSliceGetStart(const EnsPSlice slice);

ajint ensSliceGetEnd(const EnsPSlice slice);

ajint ensSliceGetStrand(const EnsPSlice slice);

AjBool ensSliceSetAdaptor(EnsPSlice slice, EnsPSliceadaptor adaptor);

AjBool ensSliceSetSequence(EnsPSlice slice, AjPStr sequence);

AjBool ensSliceTrace(const EnsPSlice slice, ajuint level);

ajuint ensSliceGetSeqregionIdentifier(const EnsPSlice slice);

const AjPStr ensSliceGetSeqregionName(const EnsPSlice slice);

ajint ensSliceGetSeqregionLength(const EnsPSlice slice);

EnsPCoordsystem ensSliceGetCoordsystem(const EnsPSlice slice);

const AjPStr ensSliceGetCoordsystemName(const EnsPSlice slice);

const AjPStr ensSliceGetCoordsystemVersion(const EnsPSlice slice);

ajint ensSliceGetCentrePoint(const EnsPSlice slice);

ajuint ensSliceGetLength(const EnsPSlice slice);

ajulong ensSliceGetMemsize(const EnsPSlice slice);

const AjPTrn ensSliceGetTranslation(EnsPSlice slice);

AjBool ensSliceFetchName(const EnsPSlice slice, AjPStr* Pname);

int ensSliceCompareIdentifierAscending(const EnsPSlice slice1,
                                       const EnsPSlice slice2);

AjBool ensSliceMatch(const EnsPSlice slice1, const EnsPSlice slice2);

AjBool ensSliceIsTopLevel(EnsPSlice slice, AjBool* Presult);

AjBool ensSliceFetchSequenceSeq(EnsPSlice slice, AjPSeq *Psequence);

AjBool ensSliceFetchSequenceStr(EnsPSlice slice, AjPStr *Psequence);

AjBool ensSliceFetchSubSequenceSeq(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPSeq *Psequence);

AjBool ensSliceFetchSubSequenceStr(EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   AjPStr *Psequence);

AjBool ensSliceFetchInvertedSlice(EnsPSlice slice, EnsPSlice *Pslice);

AjBool ensSliceFetchSubSlice(EnsPSlice slice,
                             ajint start,
                             ajint end,
                             ajint strand,
                             EnsPSlice *Pslice);

AjBool ensSliceFetchExpandedSlice(const EnsPSlice slice,
                                  ajint five,
                                  ajint three,
                                  AjBool force,
                                  ajint *Pfshift,
                                  ajint *Ptshift,
                                  EnsPSlice *Pslice);

AjBool ensSliceProject(EnsPSlice slice,
                       const AjPStr csname,
                       const AjPStr csversion,
                       AjPList pss);

AjBool ensSliceProjectToSlice(EnsPSlice srcslice,
                              EnsPSlice trgslice,
                              AjPList pss);

AjBool ensSliceFetchAllAttributes(EnsPSlice slice,
                                  const AjPStr code,
                                  AjPList attributes);

AjBool ensSliceFetchAllRepeatfeatures(EnsPSlice slice,
                                      const AjPStr anname,
                                      AjPStr rctype,
                                      AjPStr rcclass,
                                      AjPStr rcname,
                                      AjPList rfs);

AjBool ensSliceFetchAllSequenceEdits(EnsPSlice slice,
                                     AjPList ses);

/* Ensembl Slice Adaptor */

EnsPSliceadaptor ensRegistryGetSliceadaptor(
    EnsPDatabaseadaptor dba);

EnsPSliceadaptor ensSliceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSliceadaptorDel(EnsPSliceadaptor* Padaptor);

EnsPDatabaseadaptor ensSliceadaptorGetDatabaseadaptor(
    const EnsPSliceadaptor adaptor);

AjBool ensSliceadaptorCacheInsert(EnsPSliceadaptor adaptor, EnsPSlice* Pslice);

AjBool ensSliceadaptorFetchBySeqregionIdentifier(EnsPSliceadaptor adaptor,
                                                 ajuint srid,
                                                 ajint srstart,
                                                 ajint srend,
                                                 ajint srstrand,
                                                 EnsPSlice *Pslice);

AjBool ensSliceadaptorFetchByRegion(EnsPSliceadaptor adaptor,
                                    const AjPStr csname,
                                    const AjPStr csversion,
                                    const AjPStr srname,
                                    ajint srstart,
                                    ajint srend,
                                    ajint srstrand,
                                    EnsPSlice *Pslice);

AjBool ensSliceadaptorFetchByName(EnsPSliceadaptor adaptor,
                                  const AjPStr name,
                                  EnsPSlice *Pslice);

AjBool ensSliceadaptorFetchBySlice(EnsPSliceadaptor adaptor,
                                   EnsPSlice slice,
                                   ajint start,
                                   ajint end,
                                   ajint strand,
                                   EnsPSlice *Pslice);

AjBool ensSliceadaptorFetchByFeature(EnsPSliceadaptor adaptor,
                                     const EnsPFeature feature,
                                     ajint flank,
                                     EnsPSlice *Pslice);

AjBool ensSliceadaptorFetchNormalisedSliceProjection(EnsPSliceadaptor adaptor,
                                                     EnsPSlice slice,
                                                     AjPList pss);

AjBool ensSliceadaptorFetchAll(EnsPSliceadaptor adaptor,
                               const AjPStr csname,
                               const AjPStr csversion,
                               AjBool nonreference,
                               AjBool duplicates,
                               AjPList slices);

/* Ensembl Repeat-Masked Slice */

EnsPRepeatmaskedslice ensRepeatmaskedsliceNew(EnsPSlice slice,
                                              AjPList annames,
                                              AjPTable masking);

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewObj(
    EnsPRepeatmaskedslice object);

EnsPRepeatmaskedslice ensRepeatmaskedsliceNewRef(
    EnsPRepeatmaskedslice rmslice);

void ensRepeatmaskedsliceDel(EnsPRepeatmaskedslice* Prmslice);

AjBool ensRepeatmaskedsliceFetchSequenceSeq(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPSeq *Psequence);

AjBool ensRepeatmaskedsliceFetchSequenceStr(EnsPRepeatmaskedslice rmslice,
                                            EnsERepeatMaskType mtype,
                                            AjPStr *Psequence);

/*
** End of prototype definitions
*/




#endif /* ensslice_h */

#ifdef __cplusplus
}
#endif

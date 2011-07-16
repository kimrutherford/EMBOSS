
#ifndef ENSSEQUENCE_H
#define ENSSEQUENCE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensslice.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPSequenceadaptor **************************************************
**
** Ensembl Sequence Adaptor.
**
** @alias EnsSSequenceadaptor
** @alias EnsOSequenceadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @attr Cache [EnsPCache] Ensembl LRU Cache.
** @@
******************************************************************************/

typedef struct EnsSSequenceadaptor
{
    EnsPDatabaseadaptor Adaptor;
    EnsPCache Cache;
} EnsOSequenceadaptor;

#define EnsPSequenceadaptor EnsOSequenceadaptor*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Sequence Adaptor */

EnsPSequenceadaptor ensRegistryGetSequenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPSequenceadaptor ensSequenceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSequenceadaptorDel(EnsPSequenceadaptor* Psa);

AjBool ensSequenceadaptorFetchSeqregionAllSeq(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              AjPSeq* Psequence);

AjBool ensSequenceadaptorFetchSeqregionAllStr(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              AjPStr* Psequence);

AjBool ensSequenceadaptorFetchSeqregionSubSeq(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPSeq* Psequence);

AjBool ensSequenceadaptorFetchSeqregionSubStr(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPStr* Psequence);

AjBool ensSequenceadaptorFetchSliceAllSeq(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          AjPSeq* Psequence);

AjBool ensSequenceadaptorFetchSliceAllStr(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          AjPStr* Psequence);

AjBool ensSequenceadaptorFetchSliceSubSeq(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPSeq* Psequence);

AjBool ensSequenceadaptorFetchSliceSubStr(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPStr* Psequence);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSEQUENCE_H */

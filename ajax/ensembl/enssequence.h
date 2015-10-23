/* @include enssequence *******************************************************
**
** Ensembl Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.15 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:28:48 $ by $Author: mks $
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

#ifndef ENSSEQUENCE_H
#define ENSSEQUENCE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensslice.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

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




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Sequence Adaptor */

EnsPSequenceadaptor ensRegistryGetSequenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPSequenceadaptor ensSequenceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSequenceadaptorDel(EnsPSequenceadaptor *Psqa);

AjBool ensSequenceadaptorCacheClear(EnsPSequenceadaptor sqa);

EnsPDatabaseadaptor ensSequenceadaptorGetDatabaseadaptor(
    EnsPSequenceadaptor sqa);

AjBool ensSequenceadaptorFetchSeqregionAllSeq(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              AjPSeq *Psequence);

AjBool ensSequenceadaptorFetchSeqregionAllStr(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              AjPStr *Psequence);

AjBool ensSequenceadaptorFetchSeqregionSubSeq(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPSeq *Psequence);

AjBool ensSequenceadaptorFetchSeqregionSubStr(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPStr *Psequence);

AjBool ensSequenceadaptorFetchSliceAllSeq(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          AjPSeq *Psequence);

AjBool ensSequenceadaptorFetchSliceAllStr(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          AjPStr *Psequence);

AjBool ensSequenceadaptorFetchSliceSubSeq(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPSeq *Psequence);

AjBool ensSequenceadaptorFetchSliceSubStr(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPStr *Psequence);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSEQUENCE_H */

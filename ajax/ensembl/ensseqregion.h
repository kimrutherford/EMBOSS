/* @include ensseqregion ******************************************************
**
** Ensembl Sequence Region functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.23 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:27:31 $ by $Author: mks $
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

#ifndef ENSSEQREGION_H
#define ENSSEQREGION_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enscoordsystem.h"
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

/* Ensembl Sequence Region */

EnsPSeqregion ensSeqregionNewCpy(EnsPSeqregion sr);

EnsPSeqregion ensSeqregionNewIni(EnsPSeqregionadaptor sra,
                                 ajuint identifier,
                                 EnsPCoordsystem cs,
                                 AjPStr name,
                                 ajint length);

EnsPSeqregion ensSeqregionNewRef(EnsPSeqregion sr);

void ensSeqregionDel(EnsPSeqregion *Psr);

EnsPSeqregionadaptor ensSeqregionGetAdaptor(const EnsPSeqregion sr);

EnsPCoordsystem ensSeqregionGetCoordsystem(const EnsPSeqregion sr);

ajuint ensSeqregionGetIdentifier(const EnsPSeqregion sr);

ajint ensSeqregionGetLength(const EnsPSeqregion sr);

AjPStr ensSeqregionGetName(const EnsPSeqregion sr);

const AjPList ensSeqregionLoadAttributes(EnsPSeqregion sr);

const AjPList ensSeqregionLoadSeqregionsynonyms(EnsPSeqregion sr);

AjBool ensSeqregionSetAdaptor(EnsPSeqregion sr, EnsPSeqregionadaptor sra);

AjBool ensSeqregionSetCoordsystem(EnsPSeqregion sr, EnsPCoordsystem cs);

AjBool ensSeqregionSetIdentifier(EnsPSeqregion sr, ajuint identifier);

AjBool ensSeqregionSetLength(EnsPSeqregion sr, ajint length);

AjBool ensSeqregionSetName(EnsPSeqregion sr, AjPStr name);

AjBool ensSeqregionAddAttribute(EnsPSeqregion sr, EnsPAttribute attribute);

AjBool ensSeqregionAddSeqregionsynonym(EnsPSeqregion sr,
                                       EnsPSeqregionsynonym srs);

AjBool ensSeqregionTrace(const EnsPSeqregion sr, ajuint level);

size_t ensSeqregionCalculateMemsize(const EnsPSeqregion sr);

AjBool ensSeqregionMatch(const EnsPSeqregion sr1, const EnsPSeqregion sr2);

AjBool ensSeqregionFetchAllAttributes(EnsPSeqregion sr,
                                      const AjPStr code,
                                      AjPList attributes);

AjBool ensSeqregionIsLocusreferencegenomic(EnsPSeqregion sr, AjBool *Presult);

AjBool ensSeqregionIsNonreference(EnsPSeqregion sr, AjBool *Presult);

AjBool ensSeqregionIsToplevel(EnsPSeqregion sr, AjBool *Presult);

AjBool ensListSeqregionSortIdentifierAscending(AjPList srs);

AjBool ensListSeqregionSortIdentifierDescending(AjPList srs);

AjBool ensListSeqregionSortNameAscending(AjPList srs);

AjBool ensListSeqregionSortNameDescending(AjPList srs);

/* Ensembl Sequence Region Adaptor */

EnsPSeqregionadaptor ensRegistryGetSeqregionadaptor(
    EnsPDatabaseadaptor dba);

EnsPSeqregionadaptor ensSeqregionadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSeqregionadaptorDel(EnsPSeqregionadaptor *Psra);

EnsPDatabaseadaptor ensSeqregionadaptorGetDatabaseadaptor(
    EnsPSeqregionadaptor sra);

const AjPList ensSeqregionadaptorLoadLocusreferencegenomic(
    EnsPSeqregionadaptor sra);

const AjPList ensSeqregionadaptorLoadNonreferences(
    EnsPSeqregionadaptor sra);

AjBool ensSeqregionadaptorCacheInsert(EnsPSeqregionadaptor sra,
                                      EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorCacheRemove(EnsPSeqregionadaptor sra,
                                      const EnsPSeqregion sr);

AjBool ensSeqregionadaptorFetchAllbyAttributecodevalue(
    EnsPSeqregionadaptor sra,
    const AjPStr code,
    const AjPStr value,
    AjPList srs);

AjBool ensSeqregionadaptorFetchAllbyCoordsystem(EnsPSeqregionadaptor sra,
                                                const EnsPCoordsystem cs,
                                                AjPList srs);

AjBool ensSeqregionadaptorFetchByIdentifier(EnsPSeqregionadaptor sra,
                                            ajuint identifier,
                                            EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorFetchByName(EnsPSeqregionadaptor sra,
                                      const EnsPCoordsystem cs,
                                      const AjPStr name,
                                      EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorFetchByNamefuzzy(EnsPSeqregionadaptor sra,
                                           const EnsPCoordsystem cs,
                                           const AjPStr name,
                                           EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorFetchBySynonym(EnsPSeqregionadaptor sra,
                                         const AjPStr synonym,
                                         EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorIsLocusreferencegenomic(EnsPSeqregionadaptor sra,
                                                  const EnsPSeqregion sr,
                                                  AjBool *Presult);

AjBool ensSeqregionadaptorIsNonreference(EnsPSeqregionadaptor sra,
                                         const EnsPSeqregion sr,
                                         AjBool *Presult);

/* Ensembl Sequence Region Synonym */

EnsPSeqregionsynonym ensSeqregionsynonymNewCpy(EnsPSeqregionsynonym srs);

EnsPSeqregionsynonym ensSeqregionsynonymNewIni(
    EnsPSeqregionsynonymadaptor srsa,
    ajuint identifier,
    EnsPExternaldatabase edb,
    AjPStr name,
    ajuint srid);

EnsPSeqregionsynonym ensSeqregionsynonymNewRef(EnsPSeqregionsynonym srs);

void ensSeqregionsynonymDel(EnsPSeqregionsynonym *Psrs);

EnsPSeqregionsynonymadaptor ensSeqregionsynonymGetAdaptor(
    const EnsPSeqregionsynonym srs);

EnsPExternaldatabase ensSeqregionsynonymGetExternaldatabase(
    const EnsPSeqregionsynonym srs);

ajuint ensSeqregionsynonymGetIdentifier(
    const EnsPSeqregionsynonym srs);

AjPStr ensSeqregionsynonymGetName(
    const EnsPSeqregionsynonym srs);

ajuint ensSeqregionsynonymGetSeqregionidentifier(
    const EnsPSeqregionsynonym srs);

AjBool ensSeqregionsynonymSetAdaptor(EnsPSeqregionsynonym srs,
                                     EnsPSeqregionsynonymadaptor srsa);

AjBool ensSeqregionsynonymSetExternaldatabase(EnsPSeqregionsynonym srs,
                                              EnsPExternaldatabase edb);

AjBool ensSeqregionsynonymSetIdentifier(EnsPSeqregionsynonym srs,
                                        ajuint identifier);

AjBool ensSeqregionsynonymSetName(EnsPSeqregionsynonym srs,
                                  AjPStr name);

AjBool ensSeqregionsynonymSetSeqregionidentifier(EnsPSeqregionsynonym srs,
                                                 ajuint srid);

AjBool ensSeqregionsynonymTrace(const EnsPSeqregionsynonym srs, ajuint level);

size_t ensSeqregionsynonymCalculateMemsize(const EnsPSeqregionsynonym srs);

/* Ensembl Sequence Region Synonym Adaptor */

EnsPSeqregionsynonymadaptor ensRegistryGetSeqregionsynonymadaptor(
    EnsPDatabaseadaptor dba);

EnsPSeqregionsynonymadaptor ensSeqregionsynonymadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSeqregionsynonymadaptorDel(EnsPSeqregionsynonymadaptor *Psrsa);

EnsPBaseadaptor ensSeqregionsynonymadaptorGetBaseadaptor(
    EnsPSeqregionsynonymadaptor srsa);

EnsPDatabaseadaptor ensSeqregionsynonymadaptorGetDatabaseadaptor(
    EnsPSeqregionsynonymadaptor srsa);

AjBool ensSeqregionsynonymadaptorFetchAllbySeqregion(
    EnsPSeqregionsynonymadaptor srsa,
    const EnsPSeqregion sr,
    AjPList srss);

AjBool ensSeqregionsynonymadaptorFetchByIdentifier(
    EnsPSeqregionsynonymadaptor srsa,
    ajuint identifier,
    EnsPSeqregionsynonym *Psrs);

AjBool ensSeqregionsynonymadaptorFetchBySynonym(
    EnsPSeqregionsynonymadaptor srsa,
    const AjPStr synonym,
    EnsPSeqregionsynonym *Psrs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSEQREGION_H */

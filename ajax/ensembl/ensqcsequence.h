/* @include ensqcsequence *****************************************************
**
** Ensembl Quality Check Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.17 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:24 $ by $Author: mks $
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

#ifndef ENSQCSEQUENCE_H
#define ENSQCSEQUENCE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensqcdatabase.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPQcsequenceadaptor ************************************************
**
** Ensembl Quality Check Sequence Adaptor
**
** @alias EnsSQcsequenceadaptor
** @alias EnsOQcsequenceadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
** TODO: Should this also have a CacheByAccessionVersion???
******************************************************************************/

typedef struct EnsSQcsequenceadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOQcsequenceadaptor;

#define EnsPQcsequenceadaptor EnsOQcsequenceadaptor*




/* @data EnsPQcsequence *******************************************************
**
** Ensembl Quality Check Sequence
**
** @alias EnsSQcsequence
** @alias EnsOQcsequence
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcsequenceadaptor] Ensembl Sequence Adaptor
** @cc Bio::EnsEMBL::QC::Sequence
** @cc 'sequence' SQL table
** @attr Qcdatabase [EnsPQcdatabase] Ensembl Quality Check Database
** @attr Name [AjPStr] Name
** @attr Accession [AjPStr] Accession
** @attr Description [AjPStr] Description
** @attr Type [AjPStr] Type
** @attr Version [ajuint] Version
** @attr Length [ajuint] Length
** @attr CdsStart [ajuint] Coding sequence start
** @attr CdsEnd [ajuint] Coding sequence end
** @attr CdsStrand [ajint] Coding sequence strand
** @attr Polya [ajuint] Poly A+ tail length
** @@
******************************************************************************/

typedef struct EnsSQcsequence
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcsequenceadaptor Adaptor;
    EnsPQcdatabase Qcdatabase;
    AjPStr Name;
    AjPStr Accession;
    AjPStr Description;
    AjPStr Type;
    ajuint Version;
    ajuint Length;
    ajuint CdsStart;
    ajuint CdsEnd;
    ajint CdsStrand;
    ajuint Polya;
} EnsOQcsequence;

#define EnsPQcsequence EnsOQcsequence*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Quality Check Sequence */

EnsPQcsequence ensQcsequenceNewCpy(const EnsPQcsequence qcs);

EnsPQcsequence ensQcsequenceNewIni(EnsPQcsequenceadaptor adaptor,
                                   ajuint identifier,
                                   EnsPQcdatabase qcdb,
                                   AjPStr name,
                                   AjPStr accession,
                                   ajuint version,
                                   AjPStr type,
                                   ajuint length,
                                   ajuint cdsstart,
                                   ajuint cdsend,
                                   ajint cdsstrand,
                                   ajuint polya,
                                   AjPStr description);

EnsPQcsequence ensQcsequenceNewRef(EnsPQcsequence qcs);

void ensQcsequenceDel(EnsPQcsequence *Pqcs);

AjPStr ensQcsequenceGetAccession(const EnsPQcsequence qcs);

EnsPQcsequenceadaptor ensQcsequenceGetAdaptor(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetCdsEnd(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetCdsStart(const EnsPQcsequence qcs);

ajint ensQcsequenceGetCdsStrand(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetDescription(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetIdentifier(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetLength(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetName(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetPolya(const EnsPQcsequence qcs);

EnsPQcdatabase ensQcsequenceGetQcdatabase(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetType(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetVersion(const EnsPQcsequence qcs);

AjBool ensQcsequenceSetAdaptor(EnsPQcsequence qcs, EnsPQcsequenceadaptor qcsa);

AjBool ensQcsequenceSetIdentifier(EnsPQcsequence qcs, ajuint identifier);

AjBool ensQcsequenceSetQcdatabase(EnsPQcsequence qcs, EnsPQcdatabase qcdb);

AjBool ensQcsequenceSetName(EnsPQcsequence qcs, AjPStr name);

AjBool ensQcsequenceSetAccession(EnsPQcsequence qcs, AjPStr accession);

AjBool ensQcsequenceSetVersion(EnsPQcsequence qcs, ajuint version);

AjBool ensQcsequenceSetType(EnsPQcsequence qcs, AjPStr type);

AjBool ensQcsequenceSetLength(EnsPQcsequence qcs, ajuint length);

AjBool ensQcsequenceSetCdsStart(EnsPQcsequence qcs, ajuint cdsstart);

AjBool ensQcsequenceSetCdsEnd(EnsPQcsequence qcs, ajuint cdsend);

AjBool ensQcsequenceSetCdsStrand(EnsPQcsequence qcs, ajint cdsstrand);

AjBool ensQcsequenceSetPolya(EnsPQcsequence qcs, ajuint polya);

AjBool ensQcsequenceSetDescription(EnsPQcsequence qcs, AjPStr description);

AjBool ensQcsequenceTrace(const EnsPQcsequence qcs, ajuint level);

size_t ensQcsequenceCalculateMemsize(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetQcdatabaseIdentifier(const EnsPQcsequence qcs);

AjBool ensQcsequenceMatch(const EnsPQcsequence qcs1,
                          const EnsPQcsequence qcs2);

AjBool ensQcsequenceFetchAnchorExternal(const EnsPQcsequence qcs,
                                        AjBool htmlid,
                                        AjPStr *Pstr);

AjBool ensQcsequenceFetchAnchorInternal(const EnsPQcsequence qcs,
                                        AjPStr *Pstr);

AjBool ensQcsequenceFetchUrlExternal(const EnsPQcsequence qcs, AjPStr *Pstr);

AjBool ensHtmlEncodeEntities(AjPStr *Pstr);

AjBool ensHtmlEncodeSgmlid(AjPStr *Pstr);

/* Ensembl Quality Check Sequence Adaptor */

EnsPQcsequenceadaptor ensRegistryGetQcsequenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcsequenceadaptor ensQcsequenceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcsequenceadaptorDel(EnsPQcsequenceadaptor *Pqcsa);

EnsPBaseadaptor ensQcsequenceadaptorGetBaseadaptor(
    EnsPQcsequenceadaptor qcsa);

EnsPDatabaseadaptor ensQcsequenceadaptorGetDatabaseadaptor(
    EnsPQcsequenceadaptor qcsa);

AjBool ensQcsequenceadaptorFetchByAccession(EnsPQcsequenceadaptor qcsa,
                                            ajuint qcdbid,
                                            const AjPStr accession,
                                            ajuint version,
                                            EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchByIdentifier(EnsPQcsequenceadaptor adaptor,
                                             ajuint identifier,
                                             EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchByName(EnsPQcsequenceadaptor qcsa,
                                       ajuint qcdbid,
                                       const AjPStr name,
                                       EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchAllbyQcdatabase(EnsPQcsequenceadaptor qcsa,
                                                const EnsPQcdatabase qcdb,
                                                AjPList qcss);

AjBool ensQcsequenceadaptorStore(EnsPQcsequenceadaptor qcsa,
                                 EnsPQcsequence qcs);

AjBool ensQcsequenceadaptorDelete(EnsPQcsequenceadaptor qcsa,
                                  EnsPQcsequence qcs);

AjBool ensQcsequenceadaptorUpdate(EnsPQcsequenceadaptor qcsa,
                                  const EnsPQcsequence qcs);


/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSQCSEQUENCE_H */

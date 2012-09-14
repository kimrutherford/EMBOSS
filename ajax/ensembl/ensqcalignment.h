/* @include ensqcalignment ****************************************************
**
** Ensembl Quality Check Alignment functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.19 $
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

#ifndef ENSQCALIGNMENT_H
#define ENSQCALIGNMENT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensanalysis.h"
#include "ensqcsequence.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPQcalignmentadaptor **********************************************
**
** Ensembl Quality Check Alignment Adaptor
**
******************************************************************************/

#define EnsPQcalignmentadaptor EnsPBaseadaptor




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPQcalignment ******************************************************
**
** Ensembl Quality Check Alignment
**
** @alias EnsSQcalignment
** @alias EnsOQcalignment
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcalignmentadaptor] Ensembl Alignment Adaptor
** @cc Bio::EnsEMBL::QC::Alignment
** @cc 'alignment' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr QuerySequence [EnsPQcsequence] Query Ensembl Quality Check Sequence
** @attr TargetSequence [EnsPQcsequence] Target Ensembl Quality Check Sequence
** @attr Vulgar [AjPStr] Vulgar line
** @attr QueryStart [ajuint] Query start
** @attr QueryEnd [ajuint] Query end
** @attr QueryStrand [ajint] Query strand
** @attr TargetStart [ajuint] Target start
** @attr TargetEnd [ajuint] Target end
** @attr TargetStrand [ajint] Target strand
** @attr Splicestrand [ajint] Splice strand
** @attr Coverage [ajuint] Coverage score
** @attr Score [double] Score
** @attr Identity [float] Identity
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSQcalignment
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcalignmentadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsPQcsequence QuerySequence;
    EnsPQcsequence TargetSequence;
    AjPStr Vulgar;
    ajuint QueryStart;
    ajuint QueryEnd;
    ajint QueryStrand;
    ajuint TargetStart;
    ajuint TargetEnd;
    ajint TargetStrand;
    ajint Splicestrand;
    ajuint Coverage;
    double Score;
    float Identity;
    char Padding[4];
} EnsOQcalignment;

#define EnsPQcalignment EnsOQcalignment*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Quality Check Alignment */

EnsPQcalignment ensQcalignmentNewCpy(const EnsPQcalignment qca);

EnsPQcalignment ensQcalignmentNewIni(EnsPQcalignmentadaptor qcaa,
                                     ajuint identifier,
                                     EnsPAnalysis analysis,
                                     EnsPQcsequence qsequence,
                                     ajuint qstart,
                                     ajuint qend,
                                     ajint qstrand,
                                     EnsPQcsequence tsequence,
                                     ajuint tstart,
                                     ajuint tend,
                                     ajint tstrand,
                                     ajint sstrand,
                                     ajuint coverage,
                                     double score,
                                     float identity,
                                     AjPStr vulgar);

EnsPQcalignment ensQcalignmentNewRef(EnsPQcalignment qca);

void ensQcalignmentDel(EnsPQcalignment *Pqca);

EnsPQcalignmentadaptor ensQcalignmentGetAdaptor(const EnsPQcalignment qca);

EnsPAnalysis ensQcalignmentGetAnalysis(const EnsPQcalignment qca);

ajuint ensQcalignmentGetCoverage(const EnsPQcalignment qca);

ajuint ensQcalignmentGetIdentifier(const EnsPQcalignment qca);

float ensQcalignmentGetIdentity(const EnsPQcalignment qca);

ajuint ensQcalignmentGetQueryEnd(const EnsPQcalignment qca);

EnsPQcsequence ensQcalignmentGetQuerySequence(const EnsPQcalignment qca);

ajuint ensQcalignmentGetQueryStart(const EnsPQcalignment qca);

ajint ensQcalignmentGetQueryStrand(const EnsPQcalignment qca);

double ensQcalignmentGetScore(const EnsPQcalignment qca);

ajint ensQcalignmentGetSplicestrand(const EnsPQcalignment qca);

ajuint ensQcalignmentGetTargetEnd(const EnsPQcalignment qca);

EnsPQcsequence ensQcalignmentGetTargetSequence(const EnsPQcalignment qca);

ajuint ensQcalignmentGetTargetStart(const EnsPQcalignment qca);

ajint ensQcalignmentGetTargetStrand(const EnsPQcalignment qca);

AjPStr ensQcalignmentGetVulgar(const EnsPQcalignment qca);

AjBool ensQcalignmentSetAdaptor(EnsPQcalignment qca,
                                EnsPQcalignmentadaptor qcaa);

AjBool ensQcalignmentSetAnalysis(EnsPQcalignment qca,
                                 EnsPAnalysis analysis);

AjBool ensQcalignmentSetCoverage(EnsPQcalignment qca,
                                 ajuint coverage);

AjBool ensQcalignmentSetIdentifier(EnsPQcalignment qca,
                                   ajuint identifier);

AjBool ensQcalignmentSetIdentity(EnsPQcalignment qca,
                                 float identity);

AjBool ensQcalignmentSetQueryEnd(EnsPQcalignment qca,
                                 ajuint qend);

AjBool ensQcalignmentSetQuerySequence(EnsPQcalignment qca,
                                      EnsPQcsequence qsequence);

AjBool ensQcalignmentSetQueryStart(EnsPQcalignment qca,
                                   ajuint qstart);

AjBool ensQcalignmentSetQueryStrand(EnsPQcalignment qca,
                                    ajint qstrand);

AjBool ensQcalignmentSetScore(EnsPQcalignment qca,
                              double score);

AjBool ensQcalignmentSetSplicestrand(EnsPQcalignment qca,
                                     ajint sstrand);

AjBool ensQcalignmentSetTargetEnd(EnsPQcalignment qca,
                                  ajuint tend);

AjBool ensQcalignmentSetTargetSequence(EnsPQcalignment qca,
                                       EnsPQcsequence qcs);

AjBool ensQcalignmentSetTargetStart(EnsPQcalignment qca,
                                    ajuint tstart);

AjBool ensQcalignmentSetTargetStrand(EnsPQcalignment qca,
                                     ajint tstrand);

AjBool ensQcalignmentSetVulgar(EnsPQcalignment qca,
                               AjPStr vulgar);

AjBool ensQcalignmentTrace(const EnsPQcalignment qca, ajuint level);

size_t ensQcalignmentCalculateMemsize(const EnsPQcalignment qca);

AjBool ensQcalignmentCalculateQueryCoordinates(
    const EnsPQcalignment qca,
    ajint *Pstart,
    ajint *Pend,
    ajint *Pstrand,
    ajuint *Plength);

AjBool ensQcalignmentCalculateQueryCoverageDnaDna(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateQueryCoverageDnaGenome(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateQueryCoverageProteinGenome(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateQueryCoverageProteinProtein(
    EnsPQcalignment qca,
    float identity);

AjBool ensQcalignmentCalculateQueryCoverageQueryQuery(
    const EnsPQcalignment qca1,
    const EnsPQcalignment qca2,
    ajuint *Pscore);

AjBool ensQcalignmentCalculateQueryCoverageQueryTarget(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateTargetCoordinates(const EnsPQcalignment qca,
                                                ajint *Pstart,
                                                ajint *Pend,
                                                ajint *Pstrand,
                                                ajuint *Plength);

AjBool ensQcalignmentCheckTargetOverlap(const EnsPQcalignment qca1,
                                        const EnsPQcalignment qca2);

AjBool ensQcalignmentReport(const EnsPQcalignment qca,
                            AjPStr *Pstr,
                            AjBool internalquery,
                            AjBool internaltarget);

/* Ensembl Quality Check Alignment Adaptor */

EnsPQcalignmentadaptor ensRegistryGetQcalignmentadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcalignmentadaptor ensQcalignmentadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcalignmentadaptorDel(EnsPQcalignmentadaptor *Pqcaa);

EnsPBaseadaptor ensQcalignmentadaptorGetBaseadaptor(
    EnsPQcalignmentadaptor qcaa);

EnsPDatabaseadaptor ensQcalignmentadaptorGetDatabaseadaptor(
    EnsPQcalignmentadaptor qcaa);

AjBool ensQcalignmentadaptorFetchAllbyLocationTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence tsequence,
    ajuint tstart,
    ajuint tend,
    AjPList qcas);

AjBool ensQcalignmentadaptorFetchAllbyQcdatabasePair(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcas);


AjBool ensQcalignmentadaptorFetchAllbyQcdatabaseQuery(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcas);

AjBool ensQcalignmentadaptorFetchAllbyQcdatabaseTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcas);

AjBool ensQcalignmentadaptorFetchByIdentifier(
    EnsPQcalignmentadaptor qcaa,
    ajuint identifier,
    EnsPQcalignment *Pqca);

AjBool ensQcalignmentadaptorFetchAllbyCoverage(
    EnsPQcalignmentadaptor qcaa,
    ajuint lower,
    ajuint upper,
    AjPList qcas);

AjBool ensQcalignmentadaptorStore(EnsPQcalignmentadaptor qcaa,
                                  EnsPQcalignment qca);

AjBool ensQcalignmentadaptorUpdate(EnsPQcalignmentadaptor qcaa,
                                   const EnsPQcalignment qca);

AjBool ensQcalignmentadaptorDelete(EnsPQcalignmentadaptor qcaa,
                                   EnsPQcalignment qca);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSQCALIGNMENT_H */

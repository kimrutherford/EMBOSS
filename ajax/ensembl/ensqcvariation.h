/* @include ensqcvariation ****************************************************
**
** Ensembl Quality Check Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.7 $
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

#ifndef ENSQCVARIATION_H
#define ENSQCVARIATION_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensanalysis.h"
#include "ensqcalignment.h"
#include "ensqcsequence.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPQcvariationadaptor **********************************************
**
** Ensembl Quality Check Variation Adaptor
**
******************************************************************************/

#define EnsPQcvariationadaptor EnsPBaseadaptor




/* @enum EnsEQcvariationClass ************************************************
**
** Ensembl Quality Check Variation Class enumeration
**
** @value ensEQcvariationClassNULL Null
** @value ensEQcvariationClassNone None
** @value ensEQcvariationClassSimple Simple
** @value ensEQcvariationClassSplice Splice
** @value ensEQcvariationClassExon Exon
** @@
******************************************************************************/

typedef enum EnsOQcvariationClass
{
    ensEQcvariationClassNULL,
    ensEQcvariationClassNone,
    ensEQcvariationClassSimple,
    ensEQcvariationClassSplice,
    ensEQcvariationClassExon
} EnsEQcvariationClass;




/* @enum EnsEQcvariationState *************************************************
**
** Ensembl Quality Check Variation State enumeration
**
** @value ensEQcvariationStateNULL Null
** @value ensEQcvariationStateNone None
** @value ensEQcvariationStateMatch Match
** @value ensEQcvariationStateGap Gap
** @value ensEQcvariationStateFrameshift Frameshift
** @value ensEQcvariationState5ss 5-prime Splice Site
** @value ensEQcvariationState3ss 3-prime Splice Site
** @value EnsEQcvariationStateSplit Split
** @@
******************************************************************************/

typedef enum EnsOQcvariationState
{
    ensEQcvariationStateNULL,
    ensEQcvariationStateNone,
    ensEQcvariationStateMatch,
    ensEQcvariationStateGap,
    ensEQcvariationStateFrameshift,
    ensEQcvariationState5ss,
    ensEQcvariationState3ss,
    EnsEQcvariationStateSplit
} EnsEQcvariationState;




/* @enum EnsEQcvariationType **************************************************
**
** Ensembl Quality Check Variation Type enumeration
**
** @value ensEQcvariationTypeNULL Null
** @value ensEQcvariationTypeNone None
** @value ensEQcvariationTypeSingle Single
** @value ensEQcvariationTypeMulti Multi
** @@
******************************************************************************/

typedef enum EnsOQcvariationType
{
    ensEQcvariationTypeNULL,
    ensEQcvariationTypeNone,
    ensEQcvariationTypeSingle,
    ensEQcvariationTypeMulti
} EnsEQcvariationType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPQcvariation ******************************************************
**
** Ensembl Quality Check Variation
**
** @alias EnsSQcvariation
** @alias EnsOQcvariation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcvariationadaptor]
** Ensembl Quality Check Variation Adaptor
** @cc Bio::EnsEMBL::QC::Variation
** @cc 'variation' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr Qcalignment [EnsPQcalignment] Ensembl Quality Check Alignment
** @attr QuerySequence [EnsPQcsequence] Query Ensembl Quality Check Sequence
** @attr QueryStart [ajuint] Query start
** @attr QueryEnd [ajuint] Query end
** @attr QueryString [AjPStr] Query string
** @attr TargetSequence [EnsPQcsequence] Target Ensembl Quality Check Sequence
** @attr TargetStart [ajuint] Target start
** @attr TargetEnd [ajuint] Target end
** @attr TargetString [AjPStr] Target string
** @attr Class [EnsEQcvariationClass] Class
** @attr Type [EnsEQcvariationType] Type
** @attr State [EnsEQcvariationState] State
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSQcvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcvariationadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsPQcalignment Qcalignment;
    EnsPQcsequence QuerySequence;
    ajuint QueryStart;
    ajuint QueryEnd;
    AjPStr QueryString;
    EnsPQcsequence TargetSequence;
    ajuint TargetStart;
    ajuint TargetEnd;
    AjPStr TargetString;
    EnsEQcvariationClass Class;
    EnsEQcvariationType Type;
    EnsEQcvariationState State;
    char Padding[4];
} EnsOQcvariation;

#define EnsPQcvariation EnsOQcvariation*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Quality Check Variation */

EnsPQcvariation ensQcvariationNewCpy(const EnsPQcvariation qcv);

EnsPQcvariation ensQcvariationNewIni(EnsPQcvariationadaptor adaptor,
                                     ajuint identifier,
                                     EnsPQcalignment qca,
                                     EnsPAnalysis analysis,
                                     EnsPQcsequence qsequence,
                                     ajuint qstart,
                                     ajuint qend,
                                     AjPStr qstring,
                                     EnsPQcsequence tsequence,
                                     ajuint tstart,
                                     ajuint tend,
                                     AjPStr tstring,
                                     EnsEQcvariationClass class,
                                     EnsEQcvariationType type,
                                     EnsEQcvariationState state);

EnsPQcvariation ensQcvariationNewRef(EnsPQcvariation qcv);

void ensQcvariationDel(EnsPQcvariation *Pqcv);

EnsPQcvariationadaptor ensQcvariationGetAdaptor(const EnsPQcvariation qcv);

EnsPAnalysis ensQcvariationGetAnalysis(const EnsPQcvariation qcv);

EnsEQcvariationClass ensQcvariationGetClass(const EnsPQcvariation qcv);

ajuint ensQcvariationGetIdentifier(const EnsPQcvariation qcv);

EnsPQcalignment ensQcvariationGetQcalignment(const EnsPQcvariation qcv);

ajuint ensQcvariationGetQueryEnd(const EnsPQcvariation qcv);

EnsPQcsequence ensQcvariationGetQuerySequence(const EnsPQcvariation qcv);

ajuint ensQcvariationGetQueryStart(const EnsPQcvariation qcv);

AjPStr ensQcvariationGetQueryString(const EnsPQcvariation qcv);

EnsEQcvariationState ensQcvariationGetState(const EnsPQcvariation qcv);

ajuint ensQcvariationGetTargetEnd(const EnsPQcvariation qcv);

EnsPQcsequence ensQcvariationGetTargetSequence(const EnsPQcvariation qcv);

ajuint ensQcvariationGetTargetStart(const EnsPQcvariation qcv);

AjPStr ensQcvariationGetTargetString(const EnsPQcvariation qcv);

EnsEQcvariationType ensQcvariationGetType(const EnsPQcvariation qcv);

AjBool ensQcvariationSetAdaptor(EnsPQcvariation qcv,
                                EnsPQcvariationadaptor qcva);

AjBool ensQcvariationSetAnalysis(EnsPQcvariation qcv,
                                 EnsPAnalysis analysis);

AjBool ensQcvariationSetClass(EnsPQcvariation qcv,
                              EnsEQcvariationClass class);

AjBool ensQcvariationSetIdentifier(EnsPQcvariation qcv,
                                   ajuint identifier);

AjBool ensQcvariationSetQcalignment(EnsPQcvariation qcv,
                                    EnsPQcalignment qca);

AjBool ensQcvariationSetQueryEnd(EnsPQcvariation qcv,
                                 ajuint qend);

AjBool ensQcvariationSetQuerySequence(EnsPQcvariation qcv,
                                      EnsPQcsequence qsequence);

AjBool ensQcvariationSetQueryStart(EnsPQcvariation qcv,
                                   ajuint qstart);

AjBool ensQcvariationSetQueryString(EnsPQcvariation qcv,
                                    AjPStr qstring);

AjBool ensQcvariationSetState(EnsPQcvariation qcv,
                              EnsEQcvariationState state);

AjBool ensQcvariationSetTargetEnd(EnsPQcvariation qcv,
                                  ajuint tend);

AjBool ensQcvariationSetTargetSequence(EnsPQcvariation qcv,
                                       EnsPQcsequence tsequence);

AjBool ensQcvariationSetTargetStart(EnsPQcvariation qcv,
                                    ajuint tstart);

AjBool ensQcvariationSetTargetString(EnsPQcvariation qcv,
                                     AjPStr tstring);

AjBool ensQcvariationSetType(EnsPQcvariation qcv,
                             EnsEQcvariationType type);

AjBool ensQcvariationTrace(const EnsPQcvariation qcv, ajuint level);

size_t ensQcvariationCalculateMemsize(const EnsPQcvariation qcv);

EnsEQcvariationClass ensQcvariationClassFromStr(const AjPStr vclass);

const char *ensQcvariationClassToChar(EnsEQcvariationClass qcvc);

EnsEQcvariationState ensQcvariationStateFromStr(const AjPStr state);

const char *ensQcvariationStateToChar(EnsEQcvariationState qcvs);

EnsEQcvariationType ensQcvariationTypeFromStr(const AjPStr type);

const char *ensQcvariationTypeToChar(EnsEQcvariationType qcvt);

/* Ensembl Quality Check Variation Adaptor */

EnsPQcvariationadaptor ensRegistryGetQcvariationadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcvariationadaptor ensQcvariationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcvariationadaptorDel(EnsPQcvariationadaptor *Pqcva);

EnsPBaseadaptor ensQcvariationadaptorGetBaseadaptor(
    EnsPQcvariationadaptor qcva);

EnsPDatabaseadaptor ensQcvariationadaptorGetDatabaseadaptor(
    EnsPQcvariationadaptor qcva);

AjBool ensQcvariationadaptorFetchAllbyQcalignment(
    EnsPQcvariationadaptor qcva,
    const EnsPQcalignment qca,
    AjPList qcvs);

AjBool ensQcvariationadaptorFetchAllbyQcdatabasePair(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcvs);

AjBool ensQcvariationadaptorFetchAllbyQcdatabaseQuery(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcvs);

AjBool ensQcvariationadaptorFetchAllbyQcdatabaseTarget(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcvs);

AjBool ensQcvariationadaptorFetchByIdentifier(
    EnsPQcvariationadaptor adaptor,
    ajuint identifier,
    EnsPQcvariation *Pqcvf);

AjBool ensQcvariationadaptorDelete(EnsPQcvariationadaptor qcva,
                                   EnsPQcvariation qcv);

AjBool ensQcvariationadaptorStore(EnsPQcvariationadaptor qcva,
                                  EnsPQcvariation qcv);

AjBool ensQcvariationadaptorUpdate(EnsPQcvariationadaptor qcva,
                                   const EnsPQcvariation qcv);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSQCVARIATION_H */

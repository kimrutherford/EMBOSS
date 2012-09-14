/* @include ensditag **********************************************************
**
** Ensembl Ditag functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.23 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#ifndef ENSDITAG_H
#define ENSDITAG_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPDitagadaptor ****************************************************
**
** Ensembl Ditag Adaptor.
** Defined as an alias in EnsPBaseadaptor.
**
** #alias EnsPBaseadaptor
**
** #cc Bio::EnsEMBL::Map::DBSQL::DitagAdaptor
** ##
******************************************************************************/

#define EnsPDitagadaptor EnsPBaseadaptor




/* @const EnsPDitagfeatureadaptor *********************************************
**
** Ensembl Ditag Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor
** ##
******************************************************************************/

#define EnsPDitagfeatureadaptor EnsPFeatureadaptor




/* @enum EnsEDitagfeatureSide *************************************************
**
** Ensembl Ditag Feature Side enumeration
**
** @value ensEDitagfeatureSideNULL Null
** @value ensEDitagfeatureSideLeft Left side
** @value ensEDitagfeatureSideRight Right side
** @value ensEDitagfeatureSideFull Full
** @@
******************************************************************************/

typedef enum EnsODitagfeatureSide
{
    ensEDitagfeatureSideNULL,
    ensEDitagfeatureSideLeft,
    ensEDitagfeatureSideRight,
    ensEDitagfeatureSideFull
} EnsEDitagfeatureSide;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPDitag ************************************************************
**
** Ensembl Ditag
**
** @alias EnsSDitag
** @alias EnsODitag
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDitagadaptor] Ensembl Ditag Adaptor
** @cc Bio::EnsEMBL::Map::Ditag
** @attr Name [AjPStr] Name
** @attr Type [AjPStr] Source
** @attr Sequence [AjPStr] Sequence
** @attr Count [ajuint] Count
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDitag
{
    ajuint Use;
    ajuint Identifier;
    EnsPDitagadaptor Adaptor;
    AjPStr Name;
    AjPStr Type;
    AjPStr Sequence;
    ajuint Count;
    ajuint Padding;
} EnsODitag;

#define EnsPDitag EnsODitag*




/* @data EnsPDitagfeature *****************************************************
**
** Ensembl Ditag Feature
**
** @alias EnsSDitagfeature
** @alias EnsODitagfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Map::DitagFeature
** @attr Ditag [EnsPDitag] Ditag
** @attr Cigar [AjPStr] CIGAR line
** @attr Side [EnsEDitagfeatureSide] Side
** @attr TargetStart [ajint] Target start
** @attr TargetEnd [ajint] Target end
** @attr TargetStrand [ajint] Target strand
** @attr Pairidentifier [ajuint] Pair identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDitagfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPDitagfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDitag Ditag;
    AjPStr Cigar;
    EnsEDitagfeatureSide Side;
    ajint TargetStart;
    ajint TargetEnd;
    ajint TargetStrand;
    ajuint Pairidentifier;
    ajuint Padding;
} EnsODitagfeature;

#define EnsPDitagfeature EnsODitagfeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Ditag */

EnsPDitag ensDitagNewCpy(const EnsPDitag dt);

EnsPDitag ensDitagNewIni(EnsPDitagadaptor dta,
                         ajuint identifier,
                         AjPStr name,
                         AjPStr type,
                         AjPStr sequence,
                         ajuint count);

EnsPDitag ensDitagNewRef(EnsPDitag dt);

void ensDitagDel(EnsPDitag *Pdt);

EnsPDitagadaptor ensDitagGetAdaptor(const EnsPDitag dt);

ajuint ensDitagGetCount(const EnsPDitag dt);

ajuint ensDitagGetIdentifier(const EnsPDitag dt);

AjPStr ensDitagGetName(const EnsPDitag dt);

AjPStr ensDitagGetSequence(const EnsPDitag dt);

AjPStr ensDitagGetType(const EnsPDitag dt);

AjBool ensDitagSetAdaptor(EnsPDitag dt, EnsPDitagadaptor dta);

AjBool ensDitagSetCount(EnsPDitag dt, ajuint count);

AjBool ensDitagSetIdentifier(EnsPDitag dt, ajuint identifier);

AjBool ensDitagSetName(EnsPDitag dt, AjPStr name);

AjBool ensDitagSetSequence(EnsPDitag dt, AjPStr sequence);

AjBool ensDitagSetType(EnsPDitag dt, AjPStr type);

AjBool ensDitagTrace(const EnsPDitag dt, ajuint level);

size_t ensDitagCalculateMemsize(const EnsPDitag dt);

/* Ensembl Ditag Adaptor */

EnsPDitagadaptor ensRegistryGetDitagadaptor(
    EnsPDatabaseadaptor dba);

EnsPDitagadaptor ensDitagadaptorNew(EnsPDatabaseadaptor dba);

void ensDitagadaptorDel(EnsPDitagadaptor *Pdta);

EnsPDatabaseadaptor ensDitagadaptorGetDatabaseadaptor(EnsPDitagadaptor dta);

AjBool ensDitagadaptorFetchAll(EnsPDitagadaptor dta,
                               const AjPStr name,
                               const AjPStr type,
                               AjPList dts);

AjBool ensDitagadaptorFetchAllbyIdentifiers(EnsPDitagadaptor dta,
                                            AjPTable dts);

AjBool ensDitagadaptorFetchAllbyName(EnsPDitagadaptor dta,
                                     const AjPStr name,
                                     AjPList dts);

AjBool ensDitagadaptorFetchAllbyType(EnsPDitagadaptor dta,
                                     const AjPStr type,
                                     AjPList dts);

AjBool ensDitagadaptorFetchByIdentifier(EnsPDitagadaptor dta,
                                        ajuint identifier,
                                        EnsPDitag *Pdt);

/* Ensembl Ditag Feature */

EnsPDitagfeature ensDitagfeatureNewCpy(const EnsPDitagfeature dtf);

EnsPDitagfeature ensDitagfeatureNewIni(EnsPDitagfeatureadaptor dtfa,
                                       ajuint identifier,
                                       EnsPFeature feature,
                                       EnsPDitag dt,
                                       AjPStr cigar,
                                       EnsEDitagfeatureSide side,
                                       ajint tstart,
                                       ajint tend,
                                       ajint tstrand,
                                       ajuint pairid);

EnsPDitagfeature ensDitagfeatureNewRef(EnsPDitagfeature dtf);

void ensDitagfeatureDel(EnsPDitagfeature *Pdtf);

EnsPDitagfeatureadaptor ensDitagfeatureGetAdaptor(const EnsPDitagfeature dtf);

AjPStr ensDitagfeatureGetCigar(const EnsPDitagfeature dtf);

EnsPDitag ensDitagfeatureGetDitag(const EnsPDitagfeature dtf);

EnsPFeature ensDitagfeatureGetFeature(const EnsPDitagfeature dtf);

ajuint ensDitagfeatureGetIdentifier(const EnsPDitagfeature dtf);

ajuint ensDitagfeatureGetPairidentifier(const EnsPDitagfeature dtf);

EnsEDitagfeatureSide ensDitagfeatureGetSide(const EnsPDitagfeature dtf);

ajint ensDitagfeatureGetTargetEnd(const EnsPDitagfeature dtf);

ajint ensDitagfeatureGetTargetStart(const EnsPDitagfeature dtf);

ajint ensDitagfeatureGetTargetStrand(const EnsPDitagfeature dtf);

AjBool ensDitagfeatureSetAdaptor(EnsPDitagfeature dtf,
                                 EnsPDitagfeatureadaptor dtfa);

AjBool ensDitagfeatureSetIdentifier(EnsPDitagfeature dtf, ajuint identifier);

AjBool ensDitagfeatureSetFeature(EnsPDitagfeature dtf, EnsPFeature feature);

AjBool ensDitagfeatureSetDitag(EnsPDitagfeature dtf, EnsPDitag dt);

AjBool ensDitagfeatureSetCigar(EnsPDitagfeature dtf, AjPStr cigar);

AjBool ensDitagfeatureSetSide(EnsPDitagfeature dtf, EnsEDitagfeatureSide side);

AjBool ensDitagfeatureSetTargetStart(EnsPDitagfeature dtf, ajint tstart);

AjBool ensDitagfeatureSetTargetEnd(EnsPDitagfeature dtf, ajint tend);

AjBool ensDitagfeatureSetTargetStrand(EnsPDitagfeature dtf, ajint tstrand);

AjBool ensDitagfeatureSetPairidentifier(EnsPDitagfeature dtf, ajuint pairid);

AjBool ensDitagfeatureTrace(const EnsPDitagfeature dtf, ajuint level);

size_t ensDitagfeatureCalculateMemsize(const EnsPDitagfeature dtf);

EnsEDitagfeatureSide ensDitagfeatureSideFromStr(const AjPStr side);

const char *ensDitagfeatureSideToChar(const EnsEDitagfeatureSide dtfs);

/* AJAX List of Ensembl Ditag Feature objects */

AjBool ensListDitagfeatureSortEndAscending(AjPList dtfs);

AjBool ensListDitagfeatureSortEndDescending(AjPList dtfs);

AjBool ensListDitagfeatureSortIdentifierAscending(AjPList dtfs);

AjBool ensListDitagfeatureSortStartAscending(AjPList dtfs);

AjBool ensListDitagfeatureSortStartDescending(AjPList dtfs);

/* Ensembl Ditag Feature Adaptor */

EnsPDitagfeatureadaptor ensRegistryGetDitagfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPDitagfeatureadaptor ensDitagfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDitagfeatureadaptorDel(EnsPDitagfeatureadaptor *Pdtfa);

EnsPDatabaseadaptor ensDitagfeatureadaptorGetDatabaseadaptor(
    EnsPDitagfeatureadaptor dtfa);

AjBool ensDitagfeatureadaptorFetchAllbyDitag(
    EnsPDitagfeatureadaptor dtfa,
    const EnsPDitag dt,
    AjPList dtfs);

AjBool ensDitagfeatureadaptorFetchAllbySlice(
    EnsPDitagfeatureadaptor dtfa,
    EnsPSlice slice,
    const AjPStr type,
    const AjPStr anname,
    AjPList dtfs);

AjBool ensDitagfeatureadaptorFetchAllbyType(
    EnsPDitagfeatureadaptor dtfa,
    const AjPStr type,
    AjPList dtfs);

AjBool ensDitagfeatureadaptorFetchByIdentifier(
    EnsPDitagfeatureadaptor dtfa,
    ajuint identifier,
    EnsPDitagfeature *Pdtf);

AjBool ensDitagfeatureadaptorRetrieveAllIdentifiers(
    EnsPDitagfeatureadaptor dtfa,
    AjPList identifiers);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSDITAG_H */

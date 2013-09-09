/* @include ensassemblyexception **********************************************
**
** Ensembl Assembly Exception functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.25 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 10:26:10 $ by $Author: mks $
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

#ifndef ENSASSEMBLYEXCEPTION_H
#define ENSASSEMBLYEXCEPTION_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdatabaseadaptor.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @enum EnsEAssemblyexceptionType ********************************************
**
** Ensembl Assembly Exception Type enumeration
**
** @value ensEAssemblyexceptionTypeNULL Null
** @value ensEAssemblyexceptionTypeHAP Haplotype Region
** @value ensEAssemblyexceptionTypePAR Pseudo-Autosomal Region
** @value ensEAssemblyexceptionTypePatchFix Sequence Patch Region
** @value ensEAssemblyexceptionTypePatchNovel Novel Patch Region
** @value ensEAssemblyexceptionTypeHAPRef Haplotype Reference Region
** @value ensEAssemblyexceptionTypePatchFixRef Sequence Patch Reference Region
** @value ensEAssemblyexceptionTypePatchNovelRef Novel Patch Reference Region
** @@
******************************************************************************/

typedef enum EnsOAssemblyexceptionType
{
    ensEAssemblyexceptionTypeNULL,
    ensEAssemblyexceptionTypeHAP,
    ensEAssemblyexceptionTypePAR,
    ensEAssemblyexceptionTypePatchFix,
    ensEAssemblyexceptionTypePatchNovel,
    ensEAssemblyexceptionTypeHAPRef,
    ensEAssemblyexceptionTypePatchFixRef,
    ensEAssemblyexceptionTypePatchNovelRef
} EnsEAssemblyexceptionType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPAssemblyexceptionadaptor *****************************************
**
** Ensembl Assembly Exception Adaptor.
**
** @alias EnsSAssemblyexceptionadaptor
** @alias EnsOAssemblyexceptionadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByReferenceSeqregion [AjPTable]
** AJAX Table of AJAX unsigned integer (Ensembl Sequence Region identifier) key
** and Ensembl Assembly Exception value data
** @@
******************************************************************************/

typedef struct EnsSAssemblyexceptionadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByReferenceSeqregion;
} EnsOAssemblyexceptionadaptor;

#define EnsPAssemblyexceptionadaptor EnsOAssemblyexceptionadaptor*




/* @data EnsPAssemblyexception ************************************************
**
** Ensembl Assembly Exception.
**
** An Ensembl Assembly Exception object encapsulates information about
** exceptions in the reference assembly path. Currently, the Ensembl system
** models two types of assembly exceptions.
**
** Haplotype (HAPs) assemblies represent alternative assembly paths of regions
** of increased genetic variablility.
** Pseudo-autosomal regions (PARs) are paired stretches at either tip of
** (mammalian) sex chromosomes where frequent chrossing over events occur.
** Due to the frequent exchange of genetic material those sequences represent
** exact copies of each other.
**
** @alias EnsSAssemblyexception
** @alias EnsOAssemblyexception
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                              Exception Adaptor
** @cc Bio::EnsEMBL::AssemblyException
** @attr ReferenceSeqregion [ajuint] Ensembl Sequence Region identifier
** @attr ReferenceStart [ajuint] Ensembl Sequence Region start
** @attr ReferenceEnd [ajuint] Ensembl Sequence Region end
** @attr ExceptionSeqregion [ajuint] Ensembl Sequence Region identifier
** @attr ExceptionStart [ajuint] Ensembl Sequence Region start
** @attr ExceptionEnd [ajuint] Ensembl Sequence Region end
** @attr Orientation [ajint] Orientation
** @attr Type [EnsEAssemblyexceptionType] Type
** @@
******************************************************************************/

typedef struct EnsSAssemblyexception
{
    ajuint Use;
    ajuint Identifier;
    EnsPAssemblyexceptionadaptor Adaptor;
    ajuint ReferenceSeqregion;
    ajuint ReferenceStart;
    ajuint ReferenceEnd;
    ajuint ExceptionSeqregion;
    ajuint ExceptionStart;
    ajuint ExceptionEnd;
    ajint Orientation;
    EnsEAssemblyexceptionType Type;
} EnsOAssemblyexception;

#define EnsPAssemblyexception EnsOAssemblyexception*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Assembly Exception */

EnsPAssemblyexception ensAssemblyexceptionNewCpy(
    const EnsPAssemblyexception ae);

EnsPAssemblyexception ensAssemblyexceptionNewIni(
    EnsPAssemblyexceptionadaptor aea,
    ajuint identifier,
    ajuint srid,
    ajuint srstart,
    ajuint srend,
    ajuint erid,
    ajuint erstart,
    ajuint erend,
    ajint ori,
    EnsEAssemblyexceptionType aet);

EnsPAssemblyexception ensAssemblyexceptionNewRef(
    EnsPAssemblyexception ae);

void ensAssemblyexceptionDel(
    EnsPAssemblyexception *Pae);

EnsPAssemblyexceptionadaptor ensAssemblyexceptionGetAdaptor(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExceptionEnd(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExceptionSeqregion(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExceptionStart(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetIdentifier(
    const EnsPAssemblyexception ae);

ajint ensAssemblyexceptionGetOrientation(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetReferenceEnd(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetReferenceSeqregion(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetReferenceStart(
    const EnsPAssemblyexception ae);

EnsEAssemblyexceptionType ensAssemblyexceptionGetType(
    const EnsPAssemblyexception ae);

AjBool ensAssemblyexceptionSetAdaptor(
    EnsPAssemblyexception ae,
    EnsPAssemblyexceptionadaptor aea);

AjBool ensAssemblyexceptionSetExceptionEnd(
    EnsPAssemblyexception ae,
    ajuint erend);

AjBool ensAssemblyexceptionSetExceptionSeqregion(
    EnsPAssemblyexception ae,
    ajuint erid);

AjBool ensAssemblyexceptionSetExceptionStart(
    EnsPAssemblyexception ae,
    ajuint erstart);

AjBool ensAssemblyexceptionSetIdentifier(
    EnsPAssemblyexception ae,
    ajuint identifier);

AjBool ensAssemblyexceptionSetOrientation(
    EnsPAssemblyexception ae,
    ajint orientation);

AjBool ensAssemblyexceptionSetReferenceEnd(
    EnsPAssemblyexception ae,
    ajuint srend);

AjBool ensAssemblyexceptionSetReferenceSeqregion(
    EnsPAssemblyexception ae,
    ajuint srid);

AjBool ensAssemblyexceptionSetReferenceStart(
    EnsPAssemblyexception ae,
    ajuint srstart);

AjBool ensAssemblyexceptionSetType(
    EnsPAssemblyexception ae,
    EnsEAssemblyexceptionType type);

AjBool ensAssemblyexceptionTrace(
    const EnsPAssemblyexception ae,
    ajuint level);

ajuint ensAssemblyexceptionCalculateExceptionLength(
    const EnsPAssemblyexception ae);

size_t ensAssemblyexceptionCalculateMemsize(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionCalculateReferenceLength(
    const EnsPAssemblyexception ae);

EnsEAssemblyexceptionType ensAssemblyexceptionTypeFromStr(
    const AjPStr type);

const char *ensAssemblyexceptionTypeToChar(
    const EnsEAssemblyexceptionType aet);

/* AJAX List of Ensembl Assembly Excepion objects */

AjBool ensListAssemblyexceptionSortReferenceEndAscending(AjPList aes);

AjBool ensListAssemblyexceptionSortReferenceEndDescending(AjPList aes);

/* Ensembl Assembly Exception Adaptor */

EnsPAssemblyexceptionadaptor ensRegistryGetAssemblyexceptionadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblyexceptionadaptor ensAssemblyexceptionadaptorNew(
    EnsPDatabaseadaptor dba);

EnsPDatabaseadaptor ensAssemblyexceptionadaptorGetDatabaseadaptor(
    EnsPAssemblyexceptionadaptor aea);

void ensAssemblyexceptionadaptorDel(
    EnsPAssemblyexceptionadaptor *Paea);

AjBool ensAssemblyexceptionadaptorFetchAll(
    const EnsPAssemblyexceptionadaptor aea,
    AjPList aes);

AjBool ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion(
    const EnsPAssemblyexceptionadaptor aea,
    ajuint srid,
    AjPList aes);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSASSEMBLYEXCEPTION_H */

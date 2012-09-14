/* @include ensstorable *******************************************************
**
** Ensembl Storable functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.15 $
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

#ifndef ENSSTORABLE_H
#define ENSSTORABLE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdatabaseconnection.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @enum EnsEStorableType *****************************************************
**
** Ensembl Storable Type enumeration
**
** @value ensEStorableTypeNULL Null
** @value ensEStorableTypeAnalysis Analysis
** @value ensEStorableTypeRepeatconsensus Repeat consensus
** @@
******************************************************************************/

typedef enum EnsOStorableType
{
    ensEStorableTypeNULL,
    ensEStorableTypeAnalysis,
    ensEStorableTypeRepeatconsensus
} EnsEStorableType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPStorable *********************************************************
**
** Ensembl Storable.
**
** @alias EnsSStorable
** @alias EnsOStorable
**
** @attr Use [ajuint] Use counter
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [void*] Ensembl Object Adaptor
** @attr Type [EnsEStorableType] Ensembl Storable Object Type
** @attr Padding [ajuint] Padding to alignment boundary
**
** @@
******************************************************************************/

typedef struct EnsSStorable
{
    ajuint Use;
    ajuint Identifier;
    void *Adaptor;
    EnsEStorableType Type;
    ajuint Padding;
} EnsOStorable;

#define EnsPStorable EnsOStorable*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

EnsPStorable ensStorableNewCpy(const EnsPStorable storable);

EnsPStorable ensStorableNewIni(EnsEStorableType type,
                               ajuint identifier,
                               void *adaptor);

EnsPStorable ensStorableNewRef(EnsPStorable storable);

void ensStorableDel(EnsPStorable *Pstorable);

void *ensStorableGetAdaptor(const EnsPStorable storable);

ajuint ensStorableGetIdentifier(const EnsPStorable storable);

EnsEStorableType ensStorableGetType(const EnsPStorable storable);

AjBool ensStorableSetIdentifier(EnsPStorable storable, ajuint identifier);

AjBool ensStorableSetAdaptor(EnsPStorable storable, void *adaptor);

AjBool ensStorableIsStored(const EnsPStorable storable,
                           const EnsPDatabaseconnection dbc);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSTORABLE_H */

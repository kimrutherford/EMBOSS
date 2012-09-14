/* @include ensutility ********************************************************
**
** Ensembl Utility functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.11 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/09/29 15:45:05 $ by $Author: mks $
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

#ifndef ENSUTILITY_H
#define ENSUTILITY_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajax.h"

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

void ensInit(void);

void ensExit(void);

AjBool ensTraceQuery(const AjPQuery qry, ajuint level);

AjBool ensTraceSeqin(const AjPSeqin seqin, ajuint level);

AjBool ensTraceSeqdesc(const AjPSeqDesc seqdesc, ajuint level);

AjBool ensTraceSeq(const AjPSeq seq, ajuint level);

AjBool ensTraceTextin(const AjPTextin textin, ajuint level);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSUTILITY_H */

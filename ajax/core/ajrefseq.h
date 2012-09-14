/* @include ajrefseq **********************************************************
**
** AJAX reference sequence functions
**
** These functions control all aspects of AJAX reference sequence
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.4 $
** @modified Oct 5 pmr First version
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#ifndef AJREFSEQ_H
#define AJREFSEQ_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajrefseqdata.h"

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

AjPRefseq    ajRefseqNew(void);
void         ajRefseqDel(AjPRefseq *Prefseq);
void         ajRefseqClear(AjPRefseq refseq);
const AjPStr ajRefseqGetDb(const AjPRefseq refseq);
const AjPStr ajRefseqGetId(const AjPRefseq refseq);
const char*  ajRefseqGetQryC(const AjPRefseq refseq);
const AjPStr ajRefseqGetQryS(const AjPRefseq refseq);
void         ajRefseqExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJREFSEQ_H */

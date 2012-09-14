/* @include embpatlist ********************************************************
**
** Functions for patternlist handling.
**
** @author Copyright (C) 2004 Henrikki Almusa, Medicel Oy,Finland
** @version $Revision: 1.6 $
** @modified $Date: 2011/10/18 14:24:25 $ by $Author: rice $
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

#ifndef EMBPATLIST_H
#define EMBPATLIST_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajpat.h"
#include "ajseqdata.h"
#include "ajfeat.h"

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

void   embPatlistSeqSearch(AjPFeattable ftable, const AjPSeq seq,
                           AjPPatlistSeq plist, AjBool reverse);
void   embPatlistSeqSearchAll(AjPFeattable ftable, const AjPSeq seq,
			      AjPPatlistSeq plist, AjBool reverse);
void   embPatlistRegexSearch(AjPFeattable ftable, const AjPSeq seq,
                             AjPPatlistRegex plist, AjBool reverse);
void   embPatlistRegexSearchAll(AjPFeattable ftable, const AjPSeq seq,
                                AjPPatlistRegex plist, AjBool reverse);
void   embPatternRegexSearch(AjPFeattable ftable, const AjPSeq seq,
                             const AjPPatternRegex pat, AjBool reverse);
void   embPatternRegexSearchAll(AjPFeattable ftable, const AjPSeq seq,
                                const AjPPatternRegex pat, AjBool reverse);
void   embPatternSeqSearch(AjPFeattable ftable, const AjPSeq seq,
                           const AjPPatternSeq pat, AjBool reverse);
void   embPatternSeqSearchAll(AjPFeattable ftable, const AjPSeq seq,
			      const AjPPatternSeq pat, AjBool reverse);
AjBool embPatternSeqCompile(AjPPatternSeq pat);

void   embPatlistExit(void);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBPATLIST_H */


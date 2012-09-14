/* @include ajseqtype *********************************************************
**
** AJAX seqtype functions
**
** @author Copyright (C) 2002 Peter Rice
** @version $Revision: 1.18 $
** @modified 2002-2011 Peter Rice
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#ifndef AJSEQTYPE_H
#define AJSEQTYPE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajseqdata.h"

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

void         ajSeqGap(AjPSeq thys, char gapc, char padc);
void         ajSeqGapLen(AjPSeq thys, char gapc, char padc, ajint ilen);
void         ajSeqGapS(AjPStr* seq, char gapc);
void         ajSeqPrintType(AjPFile outf, AjBool full);
void         ajSeqSetNuc(AjPSeq thys);
void         ajSeqSetProt(AjPSeq thys);
void         ajSeqsetSetNuc(AjPSeqset thys);
void         ajSeqsetSetProt(AjPSeqset thys);

char         ajSeqTypeAnyprotS(const AjPStr pthys);
char         ajSeqTypeDnaS(const AjPStr pthys);
void         ajSeqTypeExit(void);
char         ajSeqTypeGapanyS(const AjPStr pthys);
char         ajSeqTypeGapdnaS(const AjPStr pthys);
char         ajSeqTypeGapnucS(const AjPStr pthys);
char         ajSeqTypeGaprnaS(const AjPStr pthys);
char         ajSeqTypeNucS(const AjPStr pthys);
char         ajSeqTypeProtS(const AjPStr pthys);
AjBool       ajSeqTypeSummary(const AjPStr type_name,
                              AjPStr* Ptype, AjBool* gaps);
char         ajSeqTypeRnaS(const AjPStr pthys);

AjBool       ajSeqTypeIsAny(const AjPStr type_name);
AjBool       ajSeqTypeIsNuc(const AjPStr type_name);
AjBool       ajSeqTypeIsProt(const AjPStr type_name);


void         ajSeqType(AjPSeq thys);
AjBool       ajSeqTypeCheckIn(AjPSeq thys, const AjPSeqin seqin);
AjBool       ajSeqTypeCheckS(AjPStr* pthys, const AjPStr type_name);

void         ajSeqTypeUnused(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJSEQTYPE_H */

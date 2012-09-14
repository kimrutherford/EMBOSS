/* @include ajseqabi **********************************************************
**
** AJAX ABI format sequence processing functions
**
** These functions control all aspects of AJAX ABI file processing
**
** @author Copyright (C) 2000 Peter Rice
** @version $Revision: 1.7 $
** @modified 2000-2011 Peter Rice
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

#ifndef AJSEQABI_H
#define AJSEQABI_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajarr.h"
#include "ajstr.h"
#include "ajfile.h"

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

ajint      ajSeqABIGetBaseOffset(AjPFile fp);
void       ajSeqABIGetBasePosition(AjPFile fp,ajlong numBases,
                                   AjPShort* basePositions);
ajint      ajSeqABIGetBasePosOffset(AjPFile fp);
float      ajSeqABIGetBaseSpace(AjPFile fp);
ajint      ajSeqABIGetConfidOffset(AjPFile fp);
void       ajSeqABIGetData(AjPFile fp,const ajlong *Offset,ajlong numPoints,
                           AjPInt2d trace);
ajint      ajSeqABIGetFWO(AjPFile fp);
ajint      ajSeqABIGetNBase(AjPFile fp);
ajint      ajSeqABIGetNData(AjPFile fp);
ajint      ajSeqABIGetPrimerOffset(AjPFile fp);
ajint      ajSeqABIGetPrimerPosition(AjPFile fp);
void       ajSeqABIGetSignal(AjPFile fp,ajlong fwo_,
                             ajshort *sigC,ajshort *sigA,
                             ajshort *sigG,ajshort *sigT);
AjBool     ajSeqABIGetTraceOffset(AjPFile fp, ajlong *Offset);
AjBool     ajSeqABIMachineName(AjPFile fp,AjPStr *machine);
AjBool     ajSeqABIReadConfid(AjPFile fp,ajlong pconO,ajlong numBases,
                              float *Pqual);
AjBool     ajSeqABIReadSeq(AjPFile fp,ajlong baseO,ajlong numBases,
                           AjPStr* nseq);
AjBool     ajSeqABISampleName(AjPFile fp, AjPStr *sample);
AjBool     ajSeqABITest(AjPFile fp);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJSEQABI_H */

/* @include ajrange ***********************************************************
**
** AJAX range functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.24 $
** @modified Aug 21 ajb First version
** @modified 7 Sept 1999 GWW - String range edit functions added
** @modified 5 Nov 1999 GWW - store text after pairs of numbers
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

#ifndef AJRANGE_H
#define AJRANGE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajseqdata.h"
#include "ajlist.h"
#include "ajtranslate.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPRange *************************************************************
**
** Data structure for AJAX sequence range specifications
**
** @alias AjORange
** @alias AjSRange
**
** @attr start [ajuint*] From positions
** @attr end [ajuint*] End positions
** @attr text [AjPStr*] Associated text for each range
** @attr n [ajuint] Number of ranges
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSRange
{
    ajuint *start;
    ajuint *end;
    AjPStr *text;
    ajuint n;
    char Padding[4];
} AjORange;

#define AjPRange AjORange*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjBool    ajRangeSetOffset(AjPRange thys, ajuint begin);
AjBool    ajRangeElementSet(AjPRange thys, ajuint element,
                            ajuint start, ajuint end);
AjBool    ajRangeIsWhole(const AjPRange thys, const AjPSeq s);
void      ajRangeDel(AjPRange *thys);
AjPRange  ajRangeNewI(ajuint n);
AjPRange  ajRangeNewFilename(const AjPStr name);
AjPRange  ajRangeNewFilenameLimits(const AjPStr name,
                                   ajuint imin, ajuint imax,
                                   ajuint minsize, ajuint size);
AjPRange  ajRangeNewRange(const AjPRange src);
AjPRange  ajRangeNewString(const AjPStr s);
AjPRange  ajRangeNewStringLimits(const AjPStr str, ajuint imin, ajuint imax,
                                 ajuint minsize, ajuint size);
ajuint    ajRangeGetSize(const AjPRange thys);
AjBool    ajRangeIsOrdered(const AjPRange thys);
ajuint    ajRangeCountOverlaps(const AjPRange thys, ajuint pos, ajuint length);
ajuint    ajRangeElementTypeOverlap(const AjPRange thys, ajuint element,
                                    ajuint pos, ajuint length);
AjBool    ajRangeSeqExtract(const AjPRange thys, AjPSeq seq);
AjPSeq    ajRangeSeqExtractPep(const AjPRange thys, AjPSeq seq,
                               const AjPTrn trntable, ajint frame);
AjBool    ajRangeSeqExtractList(const AjPRange thys,
                                const AjPSeq seq, AjPList outliststr);
AjBool    ajRangeSeqMask(const AjPRange thys,
                         const AjPStr maskchar, AjPSeq seq);
AjBool    ajRangeSeqStuff(const AjPRange thys, AjPSeq seq);
AjBool    ajRangeSeqStuffPep(const AjPRange thys, AjPSeq seq,
                             ajint frame);
AjBool    ajRangeSeqToLower(const AjPRange thys, AjPSeq seq);
AjBool    ajRangeStrExtract(const AjPRange thys,
                            const AjPStr instr, AjPStr *outstr);
AjBool    ajRangeStrExtractList(const AjPRange thys,
                                const AjPStr instr, AjPList outliststr);
AjBool    ajRangeStrMask(const AjPRange thys,
                         const AjPStr maskchar, AjPStr *str);
AjBool    ajRangeStrStuff(const AjPRange thys,
                          const AjPStr instr, AjPStr *outstr);
AjBool    ajRangeStrToLower(const AjPRange thys, AjPStr *str);
AjBool    ajRangeElementGetText(const AjPRange thys, ajuint element,
                                AjPStr * text);
AjBool    ajRangeElementGetValues(const AjPRange thys, ajuint element,
                                  ajuint *start, ajuint *end);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjPRange  ajRangeCopy(const AjPRange src);
__deprecated AjPRange  ajRangeGet(const AjPStr s);
__deprecated AjPRange  ajRangeGetLimits(const AjPStr str,
                                        ajuint imin, ajuint imax,
                                        ajuint minsize, ajuint size);
__deprecated AjPRange  ajRangeFile(const AjPStr name);
__deprecated AjPRange  ajRangeFileLimits(const AjPStr name,
                                         ajuint imin, ajuint imax,
                                         ajuint minsize, ajuint size);
__deprecated ajuint    ajRangeNumber(const AjPRange thys);
__deprecated AjBool    ajRangeValues(const AjPRange thys, ajuint element,
                                     ajuint *start, ajuint *end);
__deprecated AjBool    ajRangeBegin(AjPRange thys, ajuint begin);
__deprecated AjBool    ajRangeChange(AjPRange thys, ajuint element,
                                     ajuint start, ajuint end);
__deprecated AjBool    ajRangeText(const AjPRange thys, ajuint element,
                                   AjPStr * text);
__deprecated ajuint    ajRangeOverlaps(const AjPRange thys,
                                       ajuint pos, ajuint length);
__deprecated ajuint    ajRangeOverlapSingle(ajuint start, ajuint end,
                                            ajuint pos, ajuint length);
__deprecated AjBool    ajRangeOrdered(const AjPRange thys);
__deprecated AjBool    ajRangeDefault(const AjPRange thys, const AjPSeq s);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJRANGE_H */
